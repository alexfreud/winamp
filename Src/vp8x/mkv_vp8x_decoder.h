#pragma once
#define VPX_CODEC_DISABLE_COMPAT 1

#include <vpx/vpx_decoder.h>

#include <vpx/vp8dx.h>

#include "../Plugins/Input/in_mkv/svc_mkvdecoder.h"
#include "../Plugins/Input/in_mkv/ifc_mkvvideodecoder.h"
#include "../Winamp/wa_ipc.h"
// {23D36C12-E1DF-461b-9616-969C73BD2785}
static const GUID mkv_vp8_guid =
{ 0x23d36c12, 0xe1df, 0x461b, { 0x96, 0x16, 0x96, 0x9c, 0x73, 0xbd, 0x27, 0x85 } };

class MKVDecoder : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "VP8 MKV Decoder"; }
	static GUID getServiceGuid() { return mkv_vp8_guid; } 
	int CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder);
protected:
	RECVS_DISPATCH;
};


class MKVVP8: public ifc_mkvvideodecoder
{
public:
	friend class MKVDecoder;
	MKVVP8(vpx_codec_ctx_t decoder, const nsmkv::VideoData *video_data);

	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio);
	int DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp);
	void Flush();
	int GetPicture(void **data, void **decoder_data, uint64_t *timestamp); 
	void FreePicture(void *data, void *decoder_data);
	void HurryUp(int state);
	void Close();
private:
	vpx_codec_ctx_t decoder;
	const nsmkv::VideoData *video_data;
	vpx_codec_iter_t frame_iterator;
	YV12_PLANES planes;
	bool flushing;
protected: 
	RECVS_DISPATCH;
};

