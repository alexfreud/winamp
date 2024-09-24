#pragma once
#include "../Plugins/Input/in_mkv/ifc_mkvvideodecoder.h"
#include "../Plugins/Input/in_mkv/svc_mkvdecoder.h"
#include "../winamp/wa_ipc.h"

// {0071721C-7B4D-4766-B165-E42BC080DA74}
static const GUID mkv_flv1_guid = 
{ 0x71721c, 0x7b4d, 0x4766, { 0xb1, 0x65, 0xe4, 0x2b, 0xc0, 0x80, 0xda, 0x74 } };


class MKVDecoderCreator : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "FLV1 MKV Decoder"; }
	static GUID getServiceGuid() { return mkv_flv1_guid; } 
	int CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder);
protected:
	RECVS_DISPATCH;
};


class MKVFLV1 : public ifc_mkvvideodecoder
{
	public:
	MKVFLV1(void *decoder);
	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio);
	int DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp);
	void Flush();
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