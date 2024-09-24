#include "avi_mp3_decoder.h"
#include "../nsutil/pcm.h"
#include <assert.h>

int AVIDecoder::CreateAudioDecoder(const nsavi::AVIH *avi_header, 
																	 const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, 
																	 unsigned int preferred_bits, unsigned int max_channels, bool floating_point, 
																	 ifc_aviaudiodecoder **decoder)
{
	nsavi::mp3_format *waveformat = (nsavi::mp3_format *)stream_format;

	if (waveformat->format.format == nsavi::audio_format_mp3
		||waveformat->format.format == nsavi::audio_format_mp2)
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

		*decoder = new AVIMP3Decoder(ctx, preferred_bits, max_channels, floating_point);
		return CREATEDECODER_SUCCESS;
	}

	return CREATEDECODER_NOT_MINE;

}

#define CBCLASS AVIDecoder
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS

#define FHG_DELAY 529
AVIMP3Decoder::AVIMP3Decoder(mpg123_handle *mp3, unsigned int bps, unsigned max_channels, bool floating_point)
: mp3(mp3), bits(bps?bps:16), max_channels(max_channels?max_channels:2), floating_point(floating_point), channels(0)
{
	mpg123_open_feed(mp3);
	pregap = FHG_DELAY;
	channels = 0;
	decode_buffer=0;
	decode_buffer_length=0;
}

AVIMP3Decoder::~AVIMP3Decoder() 
{
	if (mp3) {
		mpg123_delete(mp3);
		mp3 = 0;
	}
	free(decode_buffer);
}

int AVIMP3Decoder::OutputFrameSize(size_t *frame_size)
{
	long sample_rate = 44100;
	int channels = 2;
	int encoding = 0;
	if (mpg123_getformat(mp3, &sample_rate, &channels, &encoding) == MPG123_OK) {
		this->channels = channels;
	}

	*frame_size = (bits/8) * channels * mpg123_spf(mp3);
#if 0 // TODO?
	if (mp3->GetStreamInfo()->GetLayer() == 1)
		*frame_size *= 3;
#endif
	return AVI_SUCCESS;
}

int AVIMP3Decoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *_channels, unsigned int *bitsPerSample, bool *isFloat)
{
	long sample_rate = 44100;
	int channels = 2;
	int encoding = 0;
	if (mpg123_getformat(mp3, &sample_rate, &channels, &encoding) == MPG123_OK) {
		this->channels = channels;
		*sampleRate = sample_rate;
		*_channels = channels;
		*bitsPerSample = bits;
		*isFloat = floating_point;
		return AVI_SUCCESS;
	}
	else
	{
		return AVI_FAILURE;
	}
}

int AVIMP3Decoder::DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	if (!mp3)
		return AVI_FAILURE;

	if (inputBufferBytes) {
		const uint8_t *mp3_buffer = (const uint8_t *)*inputBuffer;
		int err = mpg123_feed(mp3, mp3_buffer, *inputBufferBytes);
		if (err == MPG123_OK) {
			mp3_buffer += *inputBufferBytes;
			*inputBuffer = (void *)mp3_buffer;
			*inputBufferBytes = 0;
		}
	}

	size_t output_buffer_len = *outputBufferBytes;
	*outputBufferBytes = 0;
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
					return AVI_FAILURE;
				}
			}
		}

		// get the decoded data out
		// TODO: if floating_point is true and pregap is 0, decode straight into outputBuffer
		size_t pcm_buf_used=0;
		int err = mpg123_read(mp3, (unsigned char *)decode_buffer, decode_buffer_length, &pcm_buf_used);

		if (err == MPG123_NEED_MORE && pcm_buf_used == 0) {
			if (*outputBufferBytes == 0) {
				return AVI_NEED_MORE_INPUT;
			} else {
				return AVI_SUCCESS;
			}
		} else if (err == MPG123_NEW_FORMAT) {
			assert(pcm_buf_used == 0);
			continue;
		} else if (err == MPG123_OK || pcm_buf_used) {
			if (!channels) {
				long sample_rate = 44100;
				int channels = 2;
				int encoding = 0;
				if (mpg123_getformat(mp3, &sample_rate, &channels, &encoding) == MPG123_OK) {
					this->channels = channels;
				} else {
					return AVI_FAILURE;
				}
			}
			// deal with pregap
			size_t numSamples = pcm_buf_used / sizeof(float);
			size_t offset = min(numSamples, pregap * channels);
			numSamples -= offset;
			pregap -= (int)offset / channels;
	
			float *pcm_buf = decode_buffer + offset;
			if (numSamples * bits / 8 > output_buffer_len) {
				return AVI_SUCCESS;
			}
			// convert to destination sample format
			if (floating_point)
			{
				memcpy(outputBuffer, pcm_buf, numSamples*bits/8);
			}
			else
			{
				nsutil_pcm_FloatToInt_Interleaved_Gain(outputBuffer, pcm_buf, bits, numSamples, 1.0);
			}

			*outputBufferBytes = *outputBufferBytes + numSamples * bits / 8;
			outputBuffer = (char *)outputBuffer + numSamples * bits / 8;
			output_buffer_len -= numSamples * bits / 8;
			//return AVI_SUCCESS;
		} else {
			assert(pcm_buf_used == 0);
			return AVI_FAILURE;
		}
	}
	return AVI_SUCCESS;
}

void AVIMP3Decoder::Flush()
{
	mpg123_open_feed(mp3);
	pregap = FHG_DELAY;
}

void AVIMP3Decoder::Close()
{
	if (mp3) {
		mpg123_delete(mp3);
		mp3 = 0;
	}
	
	delete this;
}

#define CBCLASS AVIMP3Decoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS