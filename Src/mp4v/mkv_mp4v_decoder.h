#pragma once
#include "../Plugins/Input/in_mkv/ifc_mkvvideodecoder.h"
#include "../Plugins/Input/in_mkv/svc_mkvdecoder.h"
#include "MFTDecoder.h"

// {E63A1285-DD51-4b2d-9847-F1C2A5638951}
static const GUID mkv_mp4v_guid = 
{ 0xe63a1285, 0xdd51, 0x4b2d, { 0x98, 0x47, 0xf1, 0xc2, 0xa5, 0x63, 0x89, 0x51 } };


class MKVDecoderCreator : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "MPEG-4 Part 2 MKV Decoder"; }
	static GUID getServiceGuid() { return mkv_mp4v_guid; } 
	int CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder);
protected:
	RECVS_DISPATCH;
};


class MKVMP4V : public ifc_mkvvideodecoder
{
public:
	MKVMP4V(MFTDecoder *decoder, const nsmkv::VideoData *video_data);
~MKVMP4V();

	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio);
	int DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp);
	void Flush();
	int GetPicture(void **data, void **decoder_data, uint64_t *timestamp); 
	void FreePicture(void *data, void *decoder_data);
	void HurryUp(int state);
	void Close();
private:
	MFTDecoder *decoder;
	const nsmkv::VideoData *video_data;

protected: 
	RECVS_DISPATCH;
};