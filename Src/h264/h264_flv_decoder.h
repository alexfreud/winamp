#pragma once
#include "../Plugins/Input/in_flv/svc_flvdecoder.h"
#include "../Plugins/Input/in_flv/FLVVideoHeader.h"
#include "../Plugins/Input/in_flv/ifc_flvvideodecoder.h"
#include "MFTDecoder.h"
#include <vector>

// {7BBC5D47-7E96-4e27-85DB-FF4190428CD0}
static const GUID flv_h264_guid = 
{ 0x7bbc5d47, 0x7e96, 0x4e27, { 0x85, 0xdb, 0xff, 0x41, 0x90, 0x42, 0x8c, 0xd0 } };

class FLVDecoderCreator : public svc_flvdecoder
{
public:
	static const char *getServiceName() { return "H.264 FLV Decoder"; }
	static GUID getServiceGuid() { return flv_h264_guid; } 
	int CreateVideoDecoder(int format_type, int width, int height, ifc_flvvideodecoder **decoder);
	int HandlesVideo(int format_type);
protected:
	RECVS_DISPATCH;
};

class FLVH264 : public ifc_flvvideodecoder
{
public:
	FLVH264(MFTDecoder *decoder);
	~FLVH264();
	int GetOutputFormat(int *x, int *y, int *color_format);
	int DecodeSample(const void *inputBuffer, size_t inputBufferBytes, int32_t timestamp);
	void Flush();
	void Close();
	int GetPicture(void **data, void **decoder_data, uint64_t *timestamp);
	void FreePicture(void *data, void *decoder_data);
	int Ready();
private:
	MFTDecoder *decoder;
	int sequence_headers_parsed;
	uint32_t nalu_size_bytes;
	std::vector<nullsoft_h264_frame_data> buffered_frames;
protected:
	RECVS_DISPATCH;
};