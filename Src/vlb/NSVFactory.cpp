#include "NSVFactory.h"
#include "nsv_vlb.h"
#include "api__vlb.h"
#include "../nsv/nsvlib.h"

IAudioDecoder *NSVFactory::CreateAudioDecoder(FOURCC format, IAudioOutput **output)
{
	switch (format)
	{
	case NSV_MAKETYPE('V', 'L', 'B', ' '):
		{
			VLB_Decoder *dec;
			WASABI_API_MEMMGR->New(&dec);
			return  dec;
		}

	default:
		return 0;
	}
}


#define CBCLASS NSVFactory
START_DISPATCH;
CB(SVC_NSVFACTORY_CREATEAUDIODECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS