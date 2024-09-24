#include "NSVAACDecoder.h"

#include <assert.h>
#include "api.h"
#include "../nsv/nsvlib.h"
#include "api.h"
#include "../nsv/nsvlib.h"
#include "../nsv/dec_if.h"
#include <string.h>
#include <bfc/platform/export.h>
#include "NSVAACDecoder.h"
#include <bfc/error.h>
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

NSVAACDecoder *NSVAACDecoder::CreateDecoder()
{
	NSVAACDecoder *decoder=0;
	WASABI_API_MEMMGR->New(&decoder);
	if (!decoder)
	{
		return 0;
	}
	decoder->Initialize();
	return decoder;
}

NSVAACDecoder::NSVAACDecoder()
{
	source_position=0;
	out_left=0;
	in_position=0;
}

NSVAACDecoder::~NSVAACDecoder()
{

}

void NSVAACDecoder::Initialize()
{
	decoder.Open();
}

void NSVAACDecoder::flush()
{
	decoder.Flush();
}

// returns -1 on error, 0 on success (done with data in 'in'), 1 on success
// but to pass 'in' again next time around.
int NSVAACDecoder::decode(void *in, int in_len, void *out, int *out_len, unsigned int out_fmt[8])
{
	HRESULT hr;
	size_t local_out_len = *out_len;
	if (SUCCEEDED(decoder.Decode(out, &local_out_len, 16, false, 1.0))) {
	*out_len = (int)local_out_len;
	if (local_out_len) {
			uint32_t local_sample_rate, local_channels;
	if (SUCCEEDED(decoder.GetOutputProperties(&local_sample_rate, &local_channels))) {

		out_fmt[0] = NSV_MAKETYPE('P', 'C', 'M', ' ');
		out_fmt[1] = local_sample_rate;
		out_fmt[2] = local_channels;
		out_fmt[3] = 16;
	}
		return 1;
	}
	}

	hr = decoder.Feed(in, in_len);
	if (FAILED(hr)) {
		hr=hr;
	}


	local_out_len = *out_len;
	if (SUCCEEDED(decoder.Decode(out, &local_out_len, 16, false, 1.0))) {
	*out_len = (int)local_out_len;
		if (local_out_len) {
			uint32_t local_sample_rate, local_channels;
	if (SUCCEEDED(decoder.GetOutputProperties(&local_sample_rate, &local_channels))) {

		out_fmt[0] = NSV_MAKETYPE('P', 'C', 'M', ' ');
		out_fmt[1] = local_sample_rate;
		out_fmt[2] = local_channels;
		out_fmt[3] = 16;
	}
		}
	} else {
		*out_len = 0;
	}
	return 0;
}



IAudioDecoder *NSVDecoder::CreateAudioDecoder(FOURCC format, IAudioOutput **output)
{
	switch (format)
	{
	case NSV_MAKETYPE('A', 'A', 'C', ' ') :							
	case NSV_MAKETYPE('A', 'A', 'C', 'P'):
	case NSV_MAKETYPE('A', 'P', 'L', ' '):
		{
			NSVAACDecoder *dec = NSVAACDecoder::CreateDecoder();
			return dec;
		}

	default:
		return 0;
	}
}


#define CBCLASS NSVDecoder
START_DISPATCH;
CB(SVC_NSVFACTORY_CREATEAUDIODECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS
