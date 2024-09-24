#include "NSVFactory.h"
#include "nsvdec.h"
#include "api.h"
#include "../nsv/nsvlib.h"

IVideoDecoder *NSVFactory::CreateVideoDecoder(int w, int h, double framerate, unsigned int fmt, int *flip)
{
	if (fmt == NSV_MAKETYPE('V','P','6','0') || fmt == NSV_MAKETYPE('V','P','6','1') || fmt == NSV_MAKETYPE('V','P','6','2')) 
	{
		*flip=1;
		void *mem = WASABI_API_MEMMGR->sysMalloc(sizeof(VP6_Decoder));
		VP6_Decoder *dec = new (mem) VP6_Decoder(w,h);
		return dec;
	}
	return NULL;
}

#define CBCLASS NSVFactory
START_DISPATCH;
CB(SVC_NSVFACTORY_CREATEVIDEODECODER, CreateVideoDecoder)
END_DISPATCH;
#undef CBCLASS