#pragma once
#include "../Plugins/Input/in_flv/svc_flvdecoder.h"
#include "../Plugins/Input/in_flv/FLVVideoHeader.h"
#include "../Plugins/Input/in_flv/ifc_flvvideodecoder.h"
#include "duck_dxl.h"
#include "../nsv/dec_if.h"
// {8FFD7807-26F0-44ef-9B6E-BEFDD6B5779A}
static const GUID vp6_flv_guid = 
{ 0x8ffd7807, 0x26f0, 0x44ef, { 0x9b, 0x6e, 0xbe, 0xfd, 0xd6, 0xb5, 0x77, 0x9a } };

class FLVDecoderCreator : public svc_flvdecoder
{
public:
	static const char *getServiceName() { return "VP6 FLV Decoder"; }
	static GUID getServiceGuid() { return vp6_flv_guid; } 
	int CreateVideoDecoder(int format_type, int width, int height, ifc_flvvideodecoder **decoder);
	int HandlesVideo(int format_type);
protected:
	RECVS_DISPATCH;
};

class FLVVP6 : public ifc_flvvideodecoder
{
public:
	FLVVP6(DXL_XIMAGE_HANDLE xim);
	int GetOutputFormat(int *x, int *y, int *color_format);
	int DecodeSample(const void *inputBuffer, size_t inputBufferBytes, int32_t timestamp);
	void Close();
	int GetPicture(void **data, void **decoder_data, uint64_t *timestamp);
private:
	DXL_XIMAGE_HANDLE xim;
	YV12_PLANES vidbufdec;
	int decoded;
protected:
	RECVS_DISPATCH;
};