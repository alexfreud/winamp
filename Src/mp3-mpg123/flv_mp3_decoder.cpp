#include "flv_mp3_decoder.h"
#include "../nsutil/pcm.h"

int FLVDecoderCreator::CreateAudioDecoder(int stereo, int bits, int sample_rate, int format_type, ifc_flvaudiodecoder **decoder)
{
	if (format_type == FLV::AUDIO_FORMAT_MP3 || format_type == FLV::AUDIO_FORMAT_MP3_8KHZ)
	{
		mpg123_handle *ctx = mpg123_new(NULL, NULL);
		if (!ctx)
			return CREATEDECODER_FAILURE;

		long flags = MPG123_QUIET|MPG123_FORCE_FLOAT|MPG123_SKIP_ID3V2|MPG123_IGNORE_STREAMLENGTH|MPG123_IGNORE_INFOFRAME;
		if (!stereo) {
			flags |= MPG123_FORCE_MONO;
		}
		mpg123_param(ctx, MPG123_FLAGS, flags, 0);
		mpg123_param(ctx, MPG123_RVA, MPG123_RVA_OFF, 0);

		*decoder = new FLVMP3(ctx);
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

int FLVDecoderCreator::HandlesAudio(int format_type)
{
	if (format_type == FLV::AUDIO_FORMAT_MP3 || format_type == FLV::AUDIO_FORMAT_MP3_8KHZ)
	{
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

#define CBCLASS FLVDecoderCreator
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
CB(HANDLES_AUDIO, HandlesAudio)
END_DISPATCH;
#undef CBCLASS

/* --- */
#define FHG_DELAY 529
FLVMP3::FLVMP3(mpg123_handle *mp3) : mp3(mp3)
{
	mpg123_open_feed(mp3);
	bits = 16;
	pregap = FHG_DELAY;
	max_channels = 2;
	channels = 0;
	decode_buffer = 0;
	decode_buffer_length = 0;
}

FLVMP3::~FLVMP3()
{
	if (mp3) {
		mpg123_delete(mp3);
		mp3 = 0;
	}
	free(decode_buffer);
}

int FLVMP3::GetOutputFormat(unsigned int *sample_rate, unsigned int *_channels, unsigned int *_bits)
{
	mpg123_frameinfo frameInfo;
	if (mpg123_info(mp3, &frameInfo) == MPG123_OK)
	{
		*sample_rate = frameInfo.rate;
		channels = (frameInfo.mode == MPG123_M_MONO)?1:2;
		*_channels = channels;
		*_bits = bits;
		return FLV_AUDIO_SUCCESS;
	}
	else
	{
		return FLV_AUDIO_FAILURE;
	}
}

int FLVMP3::DecodeSample(const void *input_buffer, size_t input_buffer_bytes, void *samples, size_t *samples_size_bytes, double *bitrate)
{
	if (!mp3)
		return FLV_AUDIO_FAILURE;

	mpg123_feed(mp3, (unsigned char *)input_buffer, input_buffer_bytes);

	*samples_size_bytes = 0;
	*bitrate = 0;

	for (;;) {
		if (!decode_buffer) {
			int channels = 2;
			long sample_rate;
			int encoding;
			if (mpg123_getformat(mp3, &sample_rate, &channels, &encoding) == MPG123_OK) {
				this->channels = channels;
				decode_buffer_length = sizeof(float) * channels * mpg123_spf(mp3);
				decode_buffer = (float *)malloc(decode_buffer_length);
				if (!decode_buffer) {
					return FLV_AUDIO_FAILURE;
				}
			}
		}

		// get the decoded data out
		size_t pcm_buf_used=0;
		int err = mpg123_read(mp3, (unsigned char *)decode_buffer, decode_buffer_length, &pcm_buf_used);

		if (pcm_buf_used) {
			if (!channels) {
				long sample_rate = 44100;
				int channels = 2;
				int encoding = 0;
				if (mpg123_getformat(mp3, &sample_rate, &channels, &encoding) == MPG123_OK) {
					this->channels = channels;
				} else {
					return FLV_AUDIO_FAILURE;
				}
			}

			// deal with pregap
			size_t numSamples = pcm_buf_used / sizeof(float);
			size_t offset = min(numSamples, pregap * channels);
			numSamples -= offset;
			pregap -= (int)offset / channels;
			float *pcm_buf = decode_buffer + offset;

			// convert to destination sample format
			
			nsutil_pcm_FloatToInt_Interleaved(samples, pcm_buf, bits, numSamples);

			*samples_size_bytes += numSamples * bits / 8;
			samples = (char *)samples + numSamples * bits / 8;

			mpg123_frameinfo frameInfo;
			if (mpg123_info(mp3, &frameInfo) == MPG123_OK) {
				*bitrate = frameInfo.bitrate;
			}
			return FLV_AUDIO_SUCCESS;
		} else if (err == MPG123_NEED_MORE) {
			*samples_size_bytes = 0;
			return FLV_AUDIO_NEEDS_MORE_INPUT;
		} else if (err == MPG123_NEW_FORMAT) {
			continue;
		} else if (err == MPG123_OK) {
			continue;
		}
		else
			return FLV_AUDIO_FAILURE;
	}
	return FLV_AUDIO_SUCCESS;
}

void FLVMP3::Flush()
{
	mpg123_open_feed(mp3);
	pregap = FHG_DELAY;
}

void FLVMP3::Close()
{
	if (mp3) {
		mpg123_delete(mp3);
		mp3 = 0;
	}
	delete this;
}

void FLVMP3::SetPreferences(unsigned int _max_channels, unsigned int preferred_bits)
{
	if (preferred_bits)
		bits = preferred_bits;

	if (max_channels > _max_channels)
		max_channels = _max_channels;

	long flags = MPG123_QUIET|MPG123_FORCE_FLOAT|MPG123_SKIP_ID3V2|MPG123_IGNORE_STREAMLENGTH|MPG123_IGNORE_INFOFRAME;
	if (max_channels == 1) {
		flags |= MPG123_FORCE_MONO;
	}
	mpg123_param(mp3, MPG123_FLAGS, flags, 0);
	mpg123_param(mp3, MPG123_RVA, MPG123_RVA_OFF, 0);

}

#define CBCLASS FLVMP3
START_DISPATCH;
CB(FLV_AUDIO_GETOUTPUTFORMAT, GetOutputFormat)
CB(FLV_AUDIO_DECODE, DecodeSample)
VCB(FLV_AUDIO_FLUSH, Flush)
VCB(FLV_AUDIO_CLOSE, Close)
VCB(FLV_AUDIO_SETPREFERENCES, SetPreferences)
END_DISPATCH;
#undef CBCLASS
