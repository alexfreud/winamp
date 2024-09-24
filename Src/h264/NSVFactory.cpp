#include "NSVFactory.h"
#include "nsv_h264_decoder.h"
#include "api__h264.h"
#include "../nsv/nsvlib.h"

IVideoDecoder *NSVFactory::CreateVideoDecoder(int w, int h, double framerate, unsigned int fmt, int *flip)
{
	if (fmt == NSV_MAKETYPE('H','2','6','4')) 
	{
		*flip=0;
		void *mem = WASABI_API_MEMMGR->sysMalloc(sizeof(H264_Decoder));
		H264_Decoder *dec = new (mem) H264_Decoder();
		return dec;
	}
	return NULL;
}

#define CBCLASS NSVFactory
START_DISPATCH;
CB(SVC_NSVFACTORY_CREATEVIDEODECODER, CreateVideoDecoder)
END_DISPATCH;
#undef CBCLASS
