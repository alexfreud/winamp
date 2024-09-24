#pragma once
#include "../Plugins/Input/in_avi/ifc_avivideodecoder.h"
#include "../Plugins/Input/in_avi/svc_avidecoder.h"


// {C5EC74D7-BE87-457c-BADA-0AA403F53822}
static const GUID avi_bitmap_guid = 
{ 0xc5ec74d7, 0xbe87, 0x457c, { 0xba, 0xda, 0xa, 0xa4, 0x3, 0xf5, 0x38, 0x22 } };

class AVIDecoderCreator : public svc_avidecoder
{
public:
	static const char *getServiceName() { return "Bitmap AVI Decoder"; }
	static GUID getServiceGuid() { return avi_bitmap_guid; } 
	int CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder);
protected:
	RECVS_DISPATCH;
};
