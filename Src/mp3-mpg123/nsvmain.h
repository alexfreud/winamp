#pragma once
#include <mpg123.h>
#include "../nsv/dec_if.h"

class MP3_Decoder : public IAudioDecoder
{
public:
	MP3_Decoder();
	~MP3_Decoder() { };
	int decode(void *in, int in_len,
	           void *out, int *out_len,
	           unsigned int out_fmt[8]);
	void flush();
private:
	mpg123_handle *decoder;
	float pcm_buf[1152*2*2];
	size_t pcm_buf_used;
	int pcm_offs;
	int fused;
};
