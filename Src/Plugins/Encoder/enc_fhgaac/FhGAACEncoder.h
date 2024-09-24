#pragma once
#include "MP4Writer.h"
#include "mp4FastAAClib.h"
#include "config.h"
#include "encoder_common.h"

class FhGAACEncoder : public EncoderCommon
{
public:
	static FhGAACEncoder *CreateDecoder(const AACConfiguration *cfg, int nch, int srate, int bps);
	FhGAACEncoder(HANDLE_MPEG4ENC_ENCODER encoder, const MPEG4ENC_SETUP *setup, int nch, int srate, int bps, float *sample_buffer, unsigned int next_samples);
	~FhGAACEncoder();
	int Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail); 
	void PrepareToFinish();
	void Finish(const wchar_t *filename);
private:
	HANDLE_MPEG4ENC_ENCODER encoder;
	unsigned int channels;
	unsigned int sample_rate;
	unsigned int bits_per_sample;
	unsigned int next_samples;
	unsigned int samples_per_frame;
	float *sample_buffer;
	MP4Writer mp4_writer;
	uint64_t total_samples;
	uint64_t decodable_samples;
	bool finishing;
	bool resampling;
};