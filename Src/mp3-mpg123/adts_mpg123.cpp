#include "ADTS_MPG123.h"
#include "../winamp/wa_ipc.h"
#include <math.h>
#include "../nsutil/pcm.h"
#include <assert.h>

ADTS_MPG123::ADTS_MPG123() : decoder(0), gain(1.f)
{
	decoderDelay = 529;
	
	bitsPerSample = 0;
	allowRG = false;
	useFloat = false;
	channels = 0;
	sampleRate = 0;
	decode_buffer = 0;
	decode_buffer_length = 0;
}

ADTS_MPG123::~ADTS_MPG123()
{
	if (decoder) {
		mpg123_delete(decoder);
		decoder = 0;
	}
	free(decode_buffer);
}

int ADTS_MPG123::Initialize(bool forceMono, bool reverseStereo, bool allowSurround, int maxBits, bool _allowRG, bool _useFloat, bool _useCRC) 
{
	allowRG = _allowRG;
	useFloat = _useFloat;
	int downmix = 0;
	if (reverseStereo)
		downmix = 2;
	else
		downmix = forceMono ? 1 : 0;
	bitsPerSample = maxBits;

	decoder = mpg123_new(NULL, NULL);
	long flags = MPG123_QUIET|MPG123_FORCE_FLOAT|MPG123_SKIP_ID3V2|MPG123_IGNORE_STREAMLENGTH|MPG123_IGNORE_INFOFRAME;
	if (forceMono) {
		flags |= MPG123_FORCE_MONO;
	}
	mpg123_param(decoder, MPG123_FLAGS, flags, 0);
	mpg123_param(decoder, MPG123_RVA, MPG123_RVA_OFF, 0);

	return 0;
}

bool ADTS_MPG123::Open(ifc_mpeg_stream_reader *file)
{
	mpg123_open_feed(decoder);
	if (allowRG) {
		gain = file->MPEGStream_Gain();
	}
	return true;
}

void ADTS_MPG123::Close()
{
	if (decoder) {
		mpg123_delete(decoder);
		decoder = 0;
	}
}

bool ADTS_MPG123::_UpdateProperties()
{
	if (decoder && (!channels || !sampleRate)) {
		long sample_rate = 44100;
		int channels = 2;
		int encoding = 0;
		if (mpg123_getformat(decoder, &sample_rate, &channels, &encoding) == MPG123_OK) {
			this->channels = channels;
			this->sampleRate = sample_rate;
		}
	}

	return channels && sampleRate;
}

void ADTS_MPG123::GetOutputParameters(size_t *numBits, int *numChannels, int *sampleRate)
{
	_UpdateProperties();
	*sampleRate = this->sampleRate;
	*numChannels = this->channels;
	*numBits = bitsPerSample;
}

void ADTS_MPG123::CalculateFrameSize(int *frameSize)
{
	_UpdateProperties();
	*frameSize = (int)(bitsPerSample/8) * channels * mpg123_spf(decoder);
#if 0 // TODO?
	if (decoder->GetStreamInfo()->GetLayer() == 1)
		*frameSize *= 3;
#endif
}

void ADTS_MPG123::Flush(ifc_mpeg_stream_reader *file)
{
	mpg123_open_feed(decoder);
}

size_t ADTS_MPG123::GetCurrentBitrate()
{
	mpg123_frameinfo frameInfo;
	if (mpg123_info(decoder, &frameInfo) == MPG123_OK) {
		return frameInfo.bitrate;
	} else {
		return 0;
	}
}

size_t ADTS_MPG123::GetDecoderDelay()
{
	return decoderDelay;
}

static void Decimate(const float *input, void *output, size_t numSamples, size_t *outputWritten, int bitsPerSample, bool useFloat, float gain)
{
	if (!useFloat)
	{
		// TODO seen a few crashes reported where 'output' is 0
		nsutil_pcm_FloatToInt_Interleaved_Gain(output, input, bitsPerSample, numSamples, gain);
	}
	else if (gain != 1.f)
	{
		float *data = (float *)output;
		for (size_t i=0;i<numSamples;i++)
			data[i]*=gain;
		//data[i]=input[i]*gain;
	}

	*outputWritten = numSamples * (bitsPerSample / 8);
}

int ADTS_MPG123::_Read(ifc_mpeg_stream_reader *_file)
{
	int err;
	do {
		unsigned char buffer[4096];
		size_t bytes = 0;
		_file->MPEGStream_Read(buffer, sizeof(buffer), &bytes);
		err = mpg123_feed(decoder, buffer, bytes);
	} while (err == MPG123_NEED_MORE);
	switch(err) {
		case MPG123_OK:
			return adts::SUCCESS;
		case MPG123_NEED_MORE:
			if (_file->MPEGStream_EOF()) {
				return adts::ENDOFFILE;
			} else {
				return adts::NEEDMOREDATA;
			}
		default:
			return adts::FAILURE;
	}
}

/*
notes for mp3 surround implementations
need to check the first two frames for ancillary data
store first valid in temp
store second valid frame in delay line
decimate first valid into output buffer
ancillary data is stored one frame behind, so PCM data decoded from mp3 frame n combines with anc data from frame n+1
*/
int ADTS_MPG123::Sync(ifc_mpeg_stream_reader *_file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate)
{
	if (outputWritten) {
		*outputWritten = 0;
	}

	int err = _Read(_file);
	if (err == adts::SUCCESS) {
		err = mpg123_read(decoder, 0, 0, 0);
		if (err == MPG123_NEED_MORE) {
			return adts::NEEDMOREDATA;
		}
		if (err != MPG123_NEW_FORMAT
			&& err != MPG123_OK) {
				return adts::FAILURE;
		}
		_UpdateProperties();
		mpg123_frameinfo frameInfo;
		err = mpg123_info(decoder, &frameInfo);
		if (err == MPG123_OK) {
			*bitrate = frameInfo.bitrate;
			return adts::SUCCESS;
		} else {
			return adts::NEEDMOREDATA;
		}
	} else {
		return err;
	}
}

int ADTS_MPG123::Decode(ifc_mpeg_stream_reader *_file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate, size_t *endCut)
{
	bool retried=false;
	for (;;) {
		if (!decode_buffer) {
			int channels = 2;
			long sample_rate;
			int encoding;
			if (mpg123_getformat(decoder, &sample_rate, &channels, &encoding) == MPG123_OK) {
				this->channels = channels;
				decode_buffer_length = sizeof(float) * channels * mpg123_spf(decoder);
				decode_buffer = (float *)malloc(decode_buffer_length);
				if (!decode_buffer) {
					return adts::FAILURE;
				}
			}
		}

		float *flData=useFloat?(float *)output:decode_buffer;
		size_t flDataSize=useFloat?outputSize:decode_buffer_length;
		size_t done=0;
		int err = mpg123_read(decoder, (unsigned char *)flData, flDataSize, &done);
		switch(err) {
				case MPG123_OK:
					{
						Decimate(flData, output, done / sizeof(float), outputWritten, (int)bitsPerSample, useFloat, (float)gain);
						mpg123_frameinfo frameInfo;
						if (mpg123_info(decoder, &frameInfo) == MPG123_OK) {
							*bitrate = frameInfo.bitrate;
						}
					}
					return adts::SUCCESS;
				case MPG123_NEW_FORMAT:
					assert(done == 0);
					return adts::SUCCESS;
				case MPG123_NEED_MORE:
					if (done) {
						Decimate(flData, output, done / sizeof(float), outputWritten, (int)bitsPerSample, useFloat, (float)gain);
						mpg123_frameinfo frameInfo;
						if (mpg123_info(decoder, &frameInfo) == MPG123_OK) {
							*bitrate = frameInfo.bitrate;
						}
						return adts::SUCCESS;
					} else if (_file->MPEGStream_EOF()) {
						return adts::ENDOFFILE;
					} else {
						if (retried) {
							return adts::NEEDMOREDATA;
						} else {
							retried=true;
							err = _Read(_file);
							if (err != adts::SUCCESS) {
								return err;
							} else {
								break;
							}
						}
					}
				default:
					assert(done == 0);
					return adts::FAILURE;
		}
	}
}

int ADTS_MPG123::GetLayer()
{
	if (decoder) {
		mpg123_frameinfo frameInfo;
		mpg123_info(decoder, &frameInfo);
		return frameInfo.layer;
	} else {
		return 0;
	}
}

void ADTS_MPG123::Release()
{
	delete this;
}