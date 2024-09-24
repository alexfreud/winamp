#pragma once
#include "../Plugins/Input/in_avi/ifc_avivideodecoder.h"
#include "../Plugins/Input/in_avi/svc_avidecoder.h"
#include "duck_dxl.h"
#include "../nsv/dec_if.h"

// {51E8C046-6170-49ab-B690-1EDB58A2B76D}
static const GUID avi_vp6_guid = 
{ 0x51e8c046, 0x6170, 0x49ab, { 0xb6, 0x90, 0x1e, 0xdb, 0x58, 0xa2, 0xb7, 0x6d } };


class AVIDecoderCreator : public svc_avidecoder
{
public:
	static const char *getServiceName() { return "VP6 AVI Decoder"; }
	static GUID getServiceGuid() { return avi_vp6_guid; } 
	int CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class AVIVP6 : public ifc_avivideodecoder
{
public:
	AVIVP6(DXL_XIMAGE_HANDLE xim);

	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip);
	int DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes);
	void Flush();
	int GetPicture(void **data, void **decoder_data); 
	void FreePicture(void *data, void *decoder_data);
	void HurryUp(int state);
private:
	DXL_XIMAGE_HANDLE xim;
	YV12_PLANES vidbufdec;
	int decoded;

protected: 
	RECVS_DISPATCH;
};