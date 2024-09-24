#include "mkv_mp3_decoder.h"
#include "../nsutil/pcm.h"

int MKVDecoder::CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels,bool floating_point, ifc_mkvaudiodecoder **decoder)
{
	if (!strcmp(codec_id, "A_MPEG/L3")
		|| !strcmp(codec_id, "A_MPEG/L2")
		|| !strcmp(codec_id, "A_MPEG/L1"))
	{
		mpg123_handle *ctx = mpg123_new(NULL, NULL);
		if (!ctx)
			return CREATEDECODER_FAILURE;

		long flags = MPG123_QUIET|MPG123_FORCE_FLOAT|MPG123_SKIP_ID3V2|MPG123_IGNORE_STREAMLENGTH|MPG123_IGNORE_INFOFRAME;
		if (max_channels == 1) {
			flags |= MPG123_FORCE_MONO;
		}
		mpg123_param(ctx, MPG123_FLAGS, flags, 0);
		mpg123_param(ctx, MPG123_RVA, MPG123_RVA_OFF, 0);

		*decoder = new MKVMP3Decoder(ctx, preferred_bits, max_channels, floating_point);
		return CREATEDECODER_SUCCESS;
	}

	return CREATEDECODER_NOT_MINE;
}

#define CBCLASS MKVDecoder
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS

#define FHG_DELAY 529
MKVMP3Decoder::MKVMP3Decoder(mpg123_handle *mp3, unsigned int bps, unsigned max_channels, bool floating_point)
: mp3(mp3), bits(bps?bps:16), max_channels(max_channels?max_channels:2), floating_point(floating_point)
{
	decode_buffer=0;
	decode_buffer_length=0;
	sample_rate = 0;
	channels = 0;
	pregap = FHG_DELAY;
	mpg123_open_feed(mp3);
}

MKVMP3Decoder::~MKVMP3Decoder()
{
	if (mp3) {
		mpg123_delete(mp3);
		mp3 = 0;
	}
	free(decode_buffer);
}

bool MKVMP3Decoder::_UpdateProperties()
{
	if (mp3 && (!channels || !sample_rate)) {
		long sample_rate = 44100;
		int channels = 2;
		int encoding = 0;
		if (mpg123_getformat(mp3, &sample_rate, &channels, &encoding) == MPG123_OK) {
			this->channels = channels;
			this->sample_rate = sample_rate;
		}
	}

	return channels && sample_rate;
}

int MKVMP3Decoder::OutputFrameSize(size_t *frame_size)
{
	if (_UpdateProperties()) {
		*frame_size = (bits/8) * channels * mpg123_spf(mp3);
		return MKV_SUCCESS;
	} else {
		return MKV_FAILURE;
	}
}

int MKVMP3Decoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	if (_UpdateProperties()) {
		*sampleRate = this->sample_rate;
		*channels = this->channels;
		*bitsPerSample = bits;
		*isFloat = floating_point;
		return MKV_SUCCESS;
	}
	else
	{
		return MKV_FAILURE;
	}
}

int MKVMP3Decoder::DecodeBlock(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	if (!mp3)
		return MKV_FAILURE;

	*outputBufferBytes = 0;
	mpg123_feed(mp3, (unsigned char *)inputBuffer, inputBufferBytes);

	for (;;) {
		if (!decode_buffer) {
			if (_UpdateProperties()) {
				decode_buffer_length = sizeof(float) * channels * mpg123_spf(mp3);
				decode_buffer = (float *)malloc(decode_buffer_length);
				if (!decode_buffer) {
					return MKV_FAILURE;
				}
			}
		}
		// get the decoded data out
		size_t pcm_buf_used=0;
		int err = mpg123_read(mp3, (unsigned char *)decode_buffer, decode_buffer_length, &pcm_buf_used);

		if (pcm_buf_used) {
			if (!_UpdateProperties()) {
				return MKV_FAILURE;
			}
			// deal with pregap
			size_t numSamples = pcm_buf_used / sizeof(float);
			size_t offset = min(numSamples, pregap * channels);
			numSamples -= offset;
			pregap -= (int)offset / channels;
			float *pcm_buf = decode_buffer + offset;

			// convert to destination sample format
			
			nsutil_pcm_FloatToInt_Interleaved(outputBuffer, pcm_buf, bits, numSamples);

			*outputBufferBytes += numSamples * bits / 8;
			outputBuffer = (char *)outputBuffer + numSamples * bits / 8;

			return MKV_SUCCESS;
		} else if (err == MPG123_NEED_MORE) {
			*outputBufferBytes = 0;
			return MKV_NEED_MORE_INPUT;
		} else if (err == MPG123_NEW_FORMAT) {
			continue;
		} else if (err == MPG123_OK) {
			continue;
		}
		else
			return MKV_FAILURE;
	}
	return MKV_SUCCESS;
}

void MKVMP3Decoder::Flush()
{
	mpg123_open_feed(mp3);
	pregap = FHG_DELAY;
}

void MKVMP3Decoder::Close()
{
	delete this;
}

#define CBCLASS MKVMP3Decoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS