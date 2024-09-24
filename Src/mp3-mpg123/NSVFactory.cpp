#include "NSVFactory.h"
#include "nsvmain.h"
#include "api__mp3-mpg123.h"
#include "../nsv/nsvlib.h"

IAudioDecoder *NSVFactory::CreateAudioDecoder(FOURCC format, IAudioOutput **output)
{
	switch (format)
	{
		case NSV_MAKETYPE('M', 'P', '3', ' '):
		{
			MP3_Decoder *dec;
			WASABI_API_MEMMGR->New(&dec);
			return dec;
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
