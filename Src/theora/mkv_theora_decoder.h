#pragma once
#include "../Plugins/Input/in_mkv/ifc_mkvvideodecoder.h"
#include "../Plugins/Input/in_mkv/svc_mkvdecoder.h"
#include <theora/theoradec.h>
#include "../Winamp/wa_ipc.h" // for YV12_PLANES

// {96E5EC72-FE8A-4e9f-B964-D16DA34D2FC9}
static const GUID mkv_theora_guid = 
{ 0x96e5ec72, 0xfe8a, 0x4e9f, { 0xb9, 0x64, 0xd1, 0x6d, 0xa3, 0x4d, 0x2f, 0xc9 } };

class MKVDecoder : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "Theora MKV Decoder"; }
	static GUID getServiceGuid() { return mkv_theora_guid; } 
	int CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder);
protected:
	RECVS_DISPATCH;
};


class MKVTheora : public ifc_mkvvideodecoder
{
public:
	friend class MKVDecoder;
	MKVTheora(const nsmkv::VideoData *video_data);

	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio);
	int DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp);
	void Flush();
	int GetPicture(void **data, void **decoder_data, uint64_t *timestamp); 
	void FreePicture(void *data, void *decoder_data);
	void HurryUp(int state);
	void Close();
private:
	th_info info;
	th_comment comment;
	th_setup_info *setup;
	th_dec_ctx *decoder;
	ogg_int64_t packet_number;
	const nsmkv::VideoData *video_data;
	bool frame_ready;
	uint64_t last_timestamp;
	YV12_PLANES planes;
	bool flushing;
protected: 
	RECVS_DISPATCH;
};