// used to decode an MPEG-1 audio object in an MPEG-4 ISO Media file
#include "mp3_in_mp4.h"
#include "api__mp3-mpg123.h"
#include "../nsutil/pcm.h"

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID = 
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

#define FHG_DELAY 529
MPEG4_MP3::MPEG4_MP3()
{
	channels = 0;
	gain = 1;
	floatingPoint = false;
	decoder = 0;
	sample_rate = 0;
	bits = 16;
	pregap = FHG_DELAY;
}

MPEG4_MP3::~MPEG4_MP3()
{
	if (decoder) {
		mpg123_delete(decoder);
		decoder = 0;
	}
}

int MPEG4_MP3::OpenEx(size_t _bits, size_t _maxChannels, bool useFloat)
{
	_bits = bits;
	floatingPoint = useFloat;
	if (floatingPoint)
		bits = 32;
	else
		bits = (int)_bits;

	decoder = mpg123_new(NULL, NULL);
	long flags = MPG123_QUIET|MPG123_FORCE_FLOAT|MPG123_SKIP_ID3V2|MPG123_IGNORE_STREAMLENGTH|MPG123_IGNORE_INFOFRAME;
	if (_maxChannels == 1) {
		flags |= MPG123_FORCE_MONO;
	}
	mpg123_param(decoder, MPG123_FLAGS, flags, 0);
	mpg123_param(decoder, MPG123_RVA, MPG123_RVA_OFF, 0);
	mpg123_open_feed(decoder);
	return MP4_SUCCESS;
}

const char *MPEG4_MP3::GetCodecInfoString()
{
	return 0;
}

int MPEG4_MP3::CanHandleCodec(const char *codecName)
{
	if (!lstrcmpA(codecName, "mp4a"))
		return 1;
	else
		return 0;
}

int MPEG4_MP3::CanHandleType(unsigned __int8 type)
{
	switch(type)
	{
	case MP4_MPEG4_LAYER3_AUDIO:
	case MP4_MPEG4_LAYER2_AUDIO:
	case MP4_MPEG4_LAYER1_AUDIO:
	case MP4_TYPE_MPEG1_AUDIO:
	case MP4_TYPE_MPEG2_AUDIO:
	//case MP4_TYPE_MPEG4_AUDIO:
		return 1;
	default:
		return 0;
	}
}

int MPEG4_MP3::DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
		if (!decoder)
		return MP4_FAILURE;

	*outputBufferBytes = 0;
	mpg123_feed(decoder, (unsigned char *)inputBuffer, inputBufferBytes);

	for (;;) {
		// get the decoded data out
		size_t pcm_buf_used=0;
		float decodeBuf[1152*2];
		int err = mpg123_read(decoder, (unsigned char *)decodeBuf, sizeof(decodeBuf), &pcm_buf_used);

		if (pcm_buf_used) {
			if (!_UpdateProperties()) {
				return MP4_FAILURE;
			}
			// deal with pregap
			int numSamples = (int)pcm_buf_used / sizeof(float);
			int offset = min(numSamples, pregap * channels);
			numSamples -= offset;
			pregap -= offset / channels;
			float *pcm_buf = decodeBuf + offset;

			// convert to destination sample format
			
			nsutil_pcm_FloatToInt_Interleaved(outputBuffer, pcm_buf, bits, numSamples);

			*outputBufferBytes += numSamples * bits / 8;
			outputBuffer = (char *)outputBuffer + numSamples * bits / 8;

			return MP4_SUCCESS;
		} else if (err == MPG123_NEED_MORE) {
			*outputBufferBytes = 0;
			return MP4_NEED_MORE_INPUT;
		} else if (err == MPG123_NEW_FORMAT) {
			continue;
		} else if (err == MPG123_OK) {
			continue;
		}
		else
			return MP4_FAILURE;
	}
	return MP4_SUCCESS;
}

bool MPEG4_MP3::_UpdateProperties()
{
	if (decoder && (!channels || !sample_rate)) {
		long sample_rate = 44100;
		int channels = 2;
		int encoding = 0;
		if (mpg123_getformat(decoder, &sample_rate, &channels, &encoding) == MPG123_OK) {
			this->channels = channels;
			this->sample_rate = sample_rate;
		}
	}

	return channels && sample_rate;
}
int MPEG4_MP3::GetOutputPropertiesEx(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	if (_UpdateProperties()) {
		*sampleRate = this->sample_rate;
		*channels = this->channels;
		*bitsPerSample = bits;
		*isFloat = floatingPoint;
		return MP4_SUCCESS;
	} else {
		return MP4_FAILURE;
	}
}

int MPEG4_MP3::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample)
{
	bool dummy;
	return GetOutputPropertiesEx(sampleRate, channels, bitsPerSample, &dummy);
}

void MPEG4_MP3::Close()
{
	if (decoder) {
		mpg123_delete(decoder);
		decoder = 0;
	}
}

void MPEG4_MP3::Flush()
{
	mpg123_open_feed(decoder);
	pregap = FHG_DELAY;
}

int MPEG4_MP3::SetGain(float _gain)
{
	gain = _gain;
	return MP4_SUCCESS;
}

int MPEG4_MP3::GetCurrentBitrate(unsigned int *bitrate)
{
	mpg123_frameinfo frameInfo;
	if (mpg123_info(decoder, &frameInfo) == MPG123_OK) {
		*bitrate = frameInfo.bitrate;
		return MP4_SUCCESS;
	} else {
		return MP4_FAILURE;
	}
}

int MPEG4_MP3::OutputFrameSize(size_t *frameSize)
{
	if (_UpdateProperties()) {
		*frameSize = (bits/8) * channels * mpg123_spf(decoder);
		return MP4_SUCCESS;
	} else {
		return MP4_FAILURE;
	}	
}

int MPEG4_MP3::CanHandleMPEG4Type(unsigned __int8 type)
{
	switch (type)
	{
	case MP4_MPEG4_LAYER1_AUDIO:
	case MP4_MPEG4_LAYER2_AUDIO:
	case MP4_MPEG4_LAYER3_AUDIO:
		return 1;
	default:
		return 0;
	}
}

#define CBCLASS MPEG4_MP3
START_DISPATCH;
CB(MPEG4_AUDIO_OPEN_EX, OpenEx)
CB(MPEG4_AUDIO_CODEC_INFO_STRING, GetCodecInfoString)
CB(MPEG4_AUDIO_BITRATE, GetCurrentBitrate)
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
