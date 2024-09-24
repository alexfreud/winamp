#include "MP4AACDecoder.h"
#include <Mferror.h>
#include <Mfapi.h>
#include "../external_dependencies/libmp4v2/mp4.h"
#include "util.h"
#include "../nsutil/pcm.h"

MP4AACDecoder::MP4AACDecoder()
{
	isFloat = false;
	gain=1.0f;
	channels = 0;
}

MP4AACDecoder::~MP4AACDecoder()
{
}

int MP4AACDecoder::OpenMP4(MP4FileHandle mp4_file, MP4TrackId mp4_track, size_t output_bits, size_t maxChannels, bool useFloat)
{
	HRESULT hr;
	unsigned char *buffer;
	uint32_t buffer_size;

	if (useFloat) {
		this->bitsPerSample = 32;
	} else if (output_bits) {
		this->bitsPerSample = (unsigned int)output_bits;
	} else {
		this->bitsPerSample = 16;
	}

	this->isFloat = useFloat;

	if (MP4GetTrackESConfiguration(mp4_file, mp4_track, (uint8_t **)&buffer, &buffer_size) && buffer) {
		hr = decoder.Open(buffer, buffer_size);
		if (SUCCEEDED(hr)) {
			uint32_t local_sample_rate, local_channels;
			hr = decoder.GetOutputProperties(&local_sample_rate, &local_channels);
			if (SUCCEEDED(hr)) {
				this->channels = local_channels;
				return MP4_SUCCESS;
			} 
		}
	}
	return MP4_FAILURE;	
}

void MP4AACDecoder::Close()
{
}

void MP4AACDecoder::Flush()
{
	decoder.Flush();
}

int MP4AACDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *_bitsPerSample)
{
	bool dummy;
	return GetOutputPropertiesEx(sampleRate, channels, _bitsPerSample, &dummy);
}

int MP4AACDecoder::GetOutputPropertiesEx(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *useFloat)
{
	HRESULT hr;
	UINT32 local_sample_rate, local_channels;

	hr = decoder.GetOutputProperties(&local_sample_rate, &local_channels);
	if (FAILED(hr)) {
		return MP4_FAILURE;
	}

	*sampleRate = local_sample_rate;
	*channels = local_channels;
	*bitsPerSample = this->bitsPerSample;
	*useFloat = this->isFloat;

	return MP4_SUCCESS;
}

int MP4AACDecoder::DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	HRESULT hr;

	hr = decoder.Feed(inputBuffer, inputBufferBytes);
	if (FAILED(hr)) {
		return MP4_FAILURE;
	}

	hr = decoder.Decode(outputBuffer, outputBufferBytes, this->bitsPerSample, this->isFloat, this->gain);
	if (FAILED(hr)) {
		return MP4_FAILURE;
	}
	
	return MP4_SUCCESS;
}

int MP4AACDecoder::OutputFrameSize(size_t *frameSize)
{
	if (channels == 0) {
		return MP4_FAILURE;
	}

	size_t local_frame_size;
	if (FAILED(decoder.OutputBlockSizeSamples(&local_frame_size))) {
		return MP4_FAILURE;
	}
	*frameSize = local_frame_size / channels;

	return MP4_SUCCESS;
}

int MP4AACDecoder::CanHandleCodec(const char *codecName)
{
	return !strcmp(codecName, "mp4a");
}

int MP4AACDecoder::CanHandleType(uint8_t type)
{
	switch (type)
	{
	case MP4_TYPE_MPEG4_AUDIO:
		return 1;
	case MP4_TYPE_MPEG2_AAC_LC_AUDIO:
		return 1;
	default:
		return 0;
	}
}

int MP4AACDecoder::CanHandleMPEG4Type(uint8_t type)
{
	switch (type)
	{
	case MP4_MPEG4_TYPE_AAC_LC_AUDIO:
	case MP4_MPEG4_TYPE_AAC_HE_AUDIO:
	case MP4_MPEG4_TYPE_PARAMETRIC_STEREO:
		return 1;
	default:
		return 0;
	}
}

void MP4AACDecoder::EndOfStream()
{
	decoder.Feed(0, 0);
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS MP4AACDecoder
START_DISPATCH;
CB(MPEG4_AUDIO_OPENMP4, OpenMP4)
#if 0
CB(MPEG4_AUDIO_BITRATE, GetCurrentBitrate)
#endif

CB(MPEG4_AUDIO_FRAMESIZE, OutputFrameSize)
CB(MPEG4_AUDIO_OUTPUTINFO, GetOutputProperties)
CB(MPEG4_AUDIO_OUTPUTINFO_EX, GetOutputPropertiesEx)
CB(MPEG4_AUDIO_DECODE, DecodeSample)
VCB(MPEG4_AUDIO_FLUSH, Flush)
VCB(MPEG4_AUDIO_CLOSE, Close)
CB(MPEG4_AUDIO_HANDLES_CODEC, CanHandleCodec)
CB(MPEG4_AUDIO_HANDLES_TYPE, CanHandleType)
CB(MPEG4_AUDIO_HANDLES_MPEG4_TYPE, CanHandleMPEG4Type)
CB(MPEG4_AUDIO_SET_GAIN, SetGain)
END_DISPATCH;