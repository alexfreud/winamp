#pragma once
#include "../Plugins/Input/in_flv/svc_flvdecoder.h"
#include "../Plugins/Input/in_flv/FLVVideoHeader.h"
#include "../Plugins/Input/in_flv/ifc_flvvideodecoder.h"
#include "../Winamp/wa_ipc.h" // for YV12_PLANES

// {8E100A48-C03B-4453-A1FA-6B944FC23F6A}
static const GUID flv_h263_guid=
{ 0x8e100a48, 0xc03b, 0x4453, { 0xa1, 0xfa, 0x6b, 0x94, 0x4f, 0xc2, 0x3f, 0x6a } };

class FLVDecoderCreator : public svc_flvdecoder
{
public:
	static const char *getServiceName() { return "H.263 FLV Decoder"; }
		static GUID getServiceGuid() { return flv_h263_guid; } 
	int CreateVideoDecoder(int format_type, int width, int height, ifc_flvvideodecoder **decoder);
	int HandlesVideo(int format_type);
protected:
	RECVS_DISPATCH;
};

class FLVSorenson : public ifc_flvvideodecoder
{
public:
	FLVSorenson(void *decoder);
	int GetOutputFormat(int *x, int *y, int *color_format);
	int DecodeSample(const void *inputBuffer, size_t inputBufferBytes, int32_t timestamp);
	void Flush();
	void Close();
	int GetPicture(void **data, void **decoder_data, uint64_t *timestamp);
private:
	void *decoder;
	YV12_PLANES yv12;
	int32_t last_timestamp;
	int width, height;
	int decoded;
protected:
	RECVS_DISPATCH;
};