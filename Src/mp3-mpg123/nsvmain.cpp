#include "../nsv/nsvlib.h"
#include "../nsv/dec_if.h"
#include "nsvmain.h"
#include "../nsutil/pcm.h"

MP3_Decoder::MP3_Decoder()
{
	decoder = mpg123_new(NULL, NULL);
	long flags = MPG123_QUIET|MPG123_FORCE_FLOAT|MPG123_SKIP_ID3V2|MPG123_IGNORE_STREAMLENGTH|MPG123_IGNORE_INFOFRAME;
	mpg123_param(decoder, MPG123_FLAGS, flags, 0);
	mpg123_param(decoder, MPG123_RVA, MPG123_RVA_OFF, 0);
	memset(pcm_buf, 0, sizeof(pcm_buf)); 
	mpg123_open_feed(decoder);
	fused = 0; pcm_buf_used = 0; pcm_offs = 0; 
}

int MP3_Decoder::decode(void *in, int in_len,
                        void *out, int *out_len,
                        unsigned int out_fmt[8])
{
	int rval = 1;
	if (fused < in_len)
	{
		int l = 4096;
		if (l > in_len - fused) l = in_len - fused;
		if (l) mpg123_feed(decoder, (unsigned char *)in + fused, l);
		fused += l;
	}

	if (!pcm_buf_used)
	{
		mpg123_read(decoder, (unsigned char *)pcm_buf, sizeof(pcm_buf), &pcm_buf_used);
		
		pcm_offs = 0;
	}

	if (pcm_buf_used)
	{
		size_t numSamples = *out_len / 2;
		if (numSamples > (pcm_buf_used/sizeof(float)))
			numSamples = pcm_buf_used/sizeof(float);
			nsutil_pcm_FloatToInt_Interleaved(out, pcm_buf+pcm_offs, 16, numSamples);
		pcm_buf_used -= numSamples*sizeof(float);
		pcm_offs += (int)numSamples;
		*out_len = 2*(int)numSamples;
	}
	else
	{
		if (fused >= in_len) { fused = 0; rval = 0; }
		*out_len = 0;
	}
	mpg123_frameinfo frameInfo;
	if (mpg123_info(decoder, &frameInfo) == MPG123_OK) {
		int nch = (frameInfo.mode == MPG123_M_MONO)?1:2;
		int srate = frameInfo.rate;
		out_fmt[0] = (nch && srate) ? NSV_MAKETYPE('P', 'C', 'M', ' ') : 0;
		out_fmt[1] = srate;
		out_fmt[2] = nch;
		out_fmt[3] = (nch && srate) ? 16 : 0;
		out_fmt[4] = frameInfo.bitrate;
	}
	return rval;
}

void MP3_Decoder::flush() 
{ 
	fused = 0; 
	pcm_buf_used = 0;
	pcm_offs = 0;
	mpg123_open_feed(decoder);
}

extern "C"
{
	__declspec(dllexport) IAudioDecoder *CreateAudioDecoder(unsigned int fmt, IAudioOutput **output)
	{
		switch (fmt)
		{
		case NSV_MAKETYPE('M', 'P', '3', ' '):
			return new MP3_Decoder;

		default:
			return NULL;
		}
	}

	__declspec(dllexport) void DeleteAudioDecoder(IAudioDecoder *decoder)
	{
		if (decoder)
			delete decoder;

	}
}
