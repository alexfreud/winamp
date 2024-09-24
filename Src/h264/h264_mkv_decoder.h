#pragma once
#include "../Plugins/Input/in_mkv/ifc_mkvvideodecoder.h"
#include "../Plugins/Input/in_mkv/svc_mkvdecoder.h"
#include "MFTDecoder.h"
#include <vector>

// {8CC583E7-46DC-4736-A7CA-1159A70ADCAE}
static const GUID mkv_h264_guid = 
{ 0x8cc583e7, 0x46dc, 0x4736, { 0xa7, 0xca, 0x11, 0x59, 0xa7, 0xa, 0xdc, 0xae } };

class MKVDecoderCreator : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "H.264 MKV Decoder"; }
	static GUID getServiceGuid() { return mkv_h264_guid; } 
	int CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder);
protected:
	RECVS_DISPATCH;
};


class MKVH264 : public ifc_mkvvideodecoder
{
public:
	MKVH264(MFTDecoder *ctx, uint8_t nalu_size_minus_one, const nsmkv::VideoData *video_data);
	~MKVH264();

	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio);
	int DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp);
	void Flush();
	void Close();
	int GetPicture(void **data, void **decoder_data, uint64_t *timestamp); 
	void FreePicture(void *data, void *decoder_data);
	void EndOfStream();
	void HurryUp(int state);
private:
	MFTDecoder *decoder;
	UINT width, height;
	uint8_t nalu_size;
	const nsmkv::VideoData *video_data;
	std::vector<nullsoft_h264_frame_data> buffered_frames;
protected: 
	RECVS_DISPATCH;
};