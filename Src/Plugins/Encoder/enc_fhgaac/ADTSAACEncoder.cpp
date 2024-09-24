#include "ADTSAACEncoder.h"
#include "mp4FastAAClib.h"
#include <malloc.h>
#include "config.h"
#include "../nsutil/pcm.h"

ADTSAACEncoder *ADTSAACEncoder::CreateDecoder(const AACConfiguration *cfg, int nch, int srate, int bps)
{
	MPEG4ENC_ERROR err;

	MPEG4ENC_SETUP setup;
	setup.aot =	AACConfig_GetAOT(cfg);
	setup.nBitRate = AACConfig_GetBitrate(cfg, nch); 
	setup.bitrateMode = AACConfig_GetBitrateMode(cfg);
	setup.quality = MP4_QUAL_HIGH;
	setup.chMode = AACConfig_GetChannelMode(cfg, nch);
	setup.sbrSignaling = MP4_SBRSIG_IMPLICIT;
	setup.nSampleRateIn = srate;
	setup.transportFormat = MP4_TT_ADTS;
	setup.nGranuleLength = MP4_GRANULE_1024;
	setup.metadataMode = MP4_METADATA_NONE;

	HANDLE_MPEG4ENC_ENCODER encoder=0;
	err = MPEG4ENC_Configure(&encoder, &setup);
	if (err != MPEG4ENC_NO_ERROR)
	{
		return 0;
	}

	unsigned int first_samples;
	err = MPEG4ENC_Open(&encoder, &first_samples);
	if (err != MPEG4ENC_NO_ERROR)
	{
		
		MPEG4ENC_Close(&encoder);
		return 0;
	}
		float *sample_buffer = (float *)malloc(first_samples * sizeof(float));
	if (!sample_buffer)
	{
		MPEG4ENC_Close(&encoder);
		return 0;
	}

	ADTSAACEncoder *fhg_enc = new ADTSAACEncoder(encoder, &setup, nch, srate, bps, sample_buffer, first_samples);
	if (!fhg_enc)
	{
		free(sample_buffer);
		MPEG4ENC_Close(&encoder);
	}

	AACConfig_GetToolString(&setup, fhg_enc->tool, sizeof(fhg_enc->tool));

	return fhg_enc;
}

ADTSAACEncoder::ADTSAACEncoder(HANDLE_MPEG4ENC_ENCODER encoder, const MPEG4ENC_SETUP *setup, int nch, int srate, int bps, float *sample_buffer, unsigned int next_samples)
: encoder(encoder), channels(nch), sample_rate(srate), bits_per_sample(bps), sample_buffer(sample_buffer), next_samples(next_samples)
{
	MPEG4ENC_INFO info;
	MPEG4ENC_GetInfo(encoder, &info);
	samples_per_frame = info.nSamplesFrame[0];

	finishing=false;
}

ADTSAACEncoder::~ADTSAACEncoder()
{
	free(sample_buffer);
	MPEG4ENC_Close(&encoder);
}

int ADTSAACEncoder::Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail)
{
	if (!in_avail && !finishing)
		return 0;

	size_t num_samples = in_avail / (bits_per_sample/8);
	num_samples = min(num_samples, next_samples);

	nsutil_pcm_IntToFloat_Interleaved(sample_buffer, in, bits_per_sample, num_samples);

	int samples_consumed=0;
	int out_used = 0;
	MPEG4ENC_AUINFO *info;
	MPEG4ENC_ERROR err = MPEG4ENC_Encode(encoder, sample_buffer, num_samples, &samples_consumed, &next_samples,	(unsigned char * const)out, out_avail, &out_used,	&info);
	if (err != MPEG4ENC_NO_ERROR)
	{
		out_used = 0; /* in case it didn't get set */
	}
	else if (err != MPEG4ENC_NO_ERROR)
	{
		return 0;
	}
	if (!finishing)
	{
		*in_used = samples_consumed * (bits_per_sample/8);
	}

	return out_used;
}

void ADTSAACEncoder::PrepareToFinish()
{
	finishing=true;
}

void ADTSAACEncoder::Finish(const wchar_t *filename)
{
	/* TODO: 
	MPEG4ENC_INFO info;
	MPEG4ENC_GetInfo(encoder, &info);
	if (!resampling)
		mp4_writer.WriteGaps(info.nDelay, decodable_samples-total_samples-info.nDelay, total_samples);

	mp4_writer.WriteTool(tool);
			*/

}
