#include "ogg_theora_decoder.h"
#include <string.h>

ifc_oggdecoder *OggDecoderFactory::CreateDecoder(const ogg_packet *packet)
{
	if (packet && packet->packet && packet->bytes >= 42)
	{
		if (!memcmp(packet->packet + 1, "theora", 6))
			return new OggTheoraDecoder(packet);
	}
	return 0;
}

#define CBCLASS OggDecoderFactory
START_DISPATCH;
CB(DISP_CREATEDECODER, CreateDecoder)
END_DISPATCH;
#undef CBCLASS



OggTheoraDecoder::OggTheoraDecoder(const ogg_packet *packet)
{
}

#define CBCLASS OggTheoraDecoder
START_DISPATCH;
END_DISPATCH;
#undef CBCLASS
