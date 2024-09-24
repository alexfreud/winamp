#include <stddef.h>
#include "main.h"
#include "mpeg4audio.h"
#include "api__in_mp4.h"
#include <api/service/waservicefactory.h>
#include <assert.h>
#include "../nu/GaplessRingBuffer.h"
#include "virtualIO.h"
struct ExtendedRead
{
	ExtendedRead() : mp4(0), mp4track(0), sampleId(1),
		samples(0),	audio(0), audioFactory(0), frameSize(0), reader(0),
		sample_rate(0), timescale(0), max_sample_size(0), sample_buffer(0), decode_buffer(0)
	{}
	~ExtendedRead()
	{
		if (mp4)	MP4Close(mp4); mp4 = 0;
		if (reader) DestroyUnicodeReader(reader); reader=0;
		if (audio)
		{
			audio->Close();
			audioFactory->releaseInterface(audio);
		}
		audioFactory = 0;
		audio = 0;
		free(sample_buffer);
		sample_buffer=0;
		free(decode_buffer);
	}

	bool Open(const wchar_t *fn, int *size, int *bps, int *nch, int *srate, bool useFloat);
	MP4AudioDecoder *audio;
	MP4FileHandle mp4;
	MP4TrackId mp4track;
	MP4SampleId sampleId, samples;
	GaplessRingBuffer ringBuffer;
	void *reader;
	waServiceFactory *audioFactory;
	size_t frameSize;
	unsigned int sample_rate;
	uint32_t timescale;
	uint32_t max_sample_size;
	void *sample_buffer;
	uint8_t *decode_buffer;
};

bool ExtendedRead::Open(const wchar_t *fn, int *size, int *bps, int *nch, int *srate, bool useFloat)
{
		unsigned __int32 pregap, postgap;
	int numBits = *bps;
	int numChannels = *nch;

	reader = CreateUnicodeReader(fn);
	if (!reader)
		return false;
	mp4 = MP4ReadEx(fn, reader, &UnicodeIO);
	if (!mp4) 
	{
		DestroyUnicodeReader(reader);
		return false;
	}
	mp4track = GetAudioTrack(mp4);
	if (mp4track == MP4_INVALID_TRACK_ID) return false;
	if (!CreateDecoder(mp4, mp4track, audio, audioFactory))
		return false;

	int result;
	result = audio->OpenMP4(mp4, mp4track, numBits, numChannels, useFloat);

	if (result != MP4_SUCCESS)
		return false;


	GetGaps(mp4, pregap, postgap);
	ConfigureDecoderASC(mp4, mp4track, audio);

	timescale = MP4GetTrackTimeScale(mp4, mp4track);

	samples = MP4GetTrackNumberOfSamples(mp4, mp4track);
	MP4SampleId sample = 0;

// some codecs require a frame or two to get decoded. so we'll go until GetOutputProperties is valid
	for (MP4SampleId sample = 1;sample <= samples; sample++)
	{
		int ret;
		if (useFloat)
		{
			bool verifyFloat = false;
			ret = audio->GetOutputPropertiesEx(&sample_rate, reinterpret_cast<unsigned int *>(nch), reinterpret_cast<unsigned int *>(bps), &verifyFloat);
			if (ret == MP4_SUCCESS && !verifyFloat)
				return false;
		}
		else
		{
			ret = audio->GetOutputProperties(&sample_rate, reinterpret_cast<unsigned int *>(nch), reinterpret_cast<unsigned int *>(bps));
		}
		if (ret == MP4_SUCCESS)
		{
			MP4Duration duration = MP4GetTrackDuration(mp4, mp4track);
			*srate = sample_rate;
			frameSize = (*nch) * (*bps / 8);
			size_t outputFrameSize;
			*size = duration * frameSize;
			if (audio->OutputFrameSize(&outputFrameSize) == MP4_SUCCESS)
			{
				
			}
			else
			{
				outputFrameSize = 65536; // err on the side of caution
			}

			decode_buffer = (uint8_t *)malloc(outputFrameSize*frameSize);
			ringBuffer.Initialize(outputFrameSize, *bps, *nch, pregap, postgap);

			max_sample_size = MP4GetTrackMaxSampleSize(mp4, mp4track);
			sample_buffer = malloc(max_sample_size);
			if (sample != 1) {
				audio->Flush();
			}
			return true;
		}

		unsigned char *buffer = NULL;
		unsigned __int32 buffer_size = 0;
		if (MP4ReadSample(mp4, mp4track, sample, (unsigned __int8 **)&buffer, &buffer_size))
		{
			unsigned char tempBuf[65536];
			size_t outSize = 65536;
			int err = audio->DecodeSample(buffer, buffer_size, tempBuf, &outSize);
			MP4Free(buffer);

			if (err != MP4_SUCCESS)
				continue;
		}
	}

	return false;
}
extern "C"
{
	//returns handle!=0 if successful, 0 if error
	//size will return the final nb of bytes written to the output, -1 if unknown
	__declspec( dllexport ) intptr_t winampGetExtendedRead_openW(const wchar_t *fn, int *size, int *bps, int *nch, int *srate)
	{
		ExtendedRead *ext = new ExtendedRead;
		if (ext->Open(fn, size, bps, nch, srate, false))
			return reinterpret_cast<intptr_t>(ext);

		delete ext;
		return 0;
	}

	//returns handle!=0 if successful, 0 if error
	//size will return the final nb of bytes written to the output, -1 if unknown
	__declspec( dllexport ) intptr_t winampGetExtendedRead_openW_float(const wchar_t *fn, int *size, int *bps, int *nch, int *srate)
	{
		ExtendedRead *ext = new ExtendedRead;
		if (ext->Open(fn, size, bps, nch, srate, true))
			return reinterpret_cast<intptr_t>(ext);

		delete ext;
		return 0;
	}

	//returns nb of bytes read. -1 if read error (like CD ejected). if (ret == 0), EOF is assumed
	__declspec( dllexport ) intptr_t winampGetExtendedRead_getData(intptr_t handle, char *dest, int len, volatile int *killswitch)
	{
		ExtendedRead *ext = (ExtendedRead *)handle;

		int bytesCopied = 0;
		int skip = 0;
		len -= (len % ext->frameSize); // round down to the nearest whole frame size
		while (len && !*killswitch)
		{	
			size_t copySize = ext->ringBuffer.Read(dest, len);
			len -= copySize;
			dest += copySize;
			bytesCopied += copySize;

			if (ext->ringBuffer.Empty())
			{
				size_t outSize = 0;
				MP4Duration offset=0,duration=INT_MAX;
				if (ext->sampleId <= ext->samples) {
					unsigned char *buffer = (unsigned char *)ext->sample_buffer;
					unsigned __int32 buffer_size = ext->max_sample_size;
					MP4ReadSample(ext->mp4, ext->mp4track, ext->sampleId++, (unsigned __int8 **) & buffer, &buffer_size, 0, &duration, &offset);

					ext->audio->DecodeSample(buffer, buffer_size, ext->decode_buffer, &outSize); // TODO error check
				} else {
#if 0 // TODO Drain decode
					ext->audio->DecodeSample(0, 0, decode_buffer, &outSize); // TODO Drain method?
#else 
#endif
					return bytesCopied;
				}

				// convert to the track timescale for purposes of duration/offset/gap stuff
				int outSamples = MulDiv(outSize, ext->timescale, ext->sample_rate * ext->frameSize);

				if (offset > 0)
					outSamples -= min(outSamples, offset);

				if (outSamples > duration)
					outSamples = duration;

				// convert back to sample rate timescale
				outSize = MulDiv(ext->sample_rate * ext->frameSize, outSamples, ext->timescale);
				ext->ringBuffer.Write(ext->decode_buffer+offset*ext->frameSize, outSize);
			}
		}
		return bytesCopied;
	}

	// return nonzero on success, zero on failure.
	__declspec( dllexport ) int winampGetExtendedRead_setTime(intptr_t handle, int millisecs)
	{
		ExtendedRead *ext = (ExtendedRead *)handle;

		MP4Duration duration = MP4ConvertToTrackDuration(ext->mp4, ext->mp4track, millisecs, MP4_MSECS_TIME_SCALE);
		if(duration == MP4_INVALID_DURATION) return 0;

		MP4SampleId newSampleId = MP4GetSampleIdFromTime(ext->mp4, ext->mp4track, duration);
		if(newSampleId > ext->samples) return 0;

		ext->sampleId = newSampleId;
		ext->audio->Flush();
		//		ext->bufferUsed=0;
		ext->ringBuffer.Reset();
		return 1;
	}

	__declspec( dllexport ) void winampGetExtendedRead_close(intptr_t handle)
	{
		ExtendedRead *ext = (ExtendedRead *)handle;
		delete ext;
	}
}
