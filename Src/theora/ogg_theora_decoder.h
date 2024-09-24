#pragma once
#include "../in_ogg/svc_oggdecoder.h"

// {6018D413-172D-417a-929A-72ADC2B6E387}
static const GUID ogg_theora_guid = 
{ 0x6018d413, 0x172d, 0x417a, { 0x92, 0x9a, 0x72, 0xad, 0xc2, 0xb6, 0xe3, 0x87 } };


class OggDecoderFactory : public svc_oggdecoder
{
public:
	static const char *getServiceName() { return "Ogg Theora Decoder"; }
	static GUID getServiceGuid() { return ogg_theora_guid; }

	ifc_oggdecoder *CreateDecoder(const ogg_packet *packet);
protected:
	RECVS_DISPATCH;
};

class OggTheoraDecoder : public ifc_oggdecoder
{
public:
	OggTheoraDecoder(const ogg_packet *packet);
protected:
	RECVS_DISPATCH;
};