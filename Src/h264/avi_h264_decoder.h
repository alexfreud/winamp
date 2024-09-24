#pragma once
#include "../Plugins/Input/in_avi/ifc_avivideodecoder.h"
#include "../Plugins/Input/in_avi/svc_avidecoder.h"
#include "MFTDecoder.h"
#include <vector>

// {AFA1BB51-F41B-4522-9251-25A8DF923DBE}
static const GUID avi_h264_guid = 
{ 0xafa1bb51, 0xf41b, 0x4522, { 0x92, 0x51, 0x25, 0xa8, 0xdf, 0x92, 0x3d, 0xbe } };


class AVIDecoderCreator : public svc_avidecoder
{
public:
	static const char *getServiceName() { return "H.264 AVI Decoder"; }
	static GUID getServiceGuid() { return avi_h264_guid; } 
	int CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class AVIH264 : public ifc_avivideodecoder
{
public:
	AVIH264(MFTDecoder *ctx, const nsavi::STRH *stream_header);
	~AVIH264();

	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip);
	int DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes);
	void Flush();
	void Close();
	int GetPicture(void **data, void **decoder_data); 
	void FreePicture(void *data, void *decoder_data);
	void EndOfStream();
	void HurryUp(int state);
private:
	MFTDecoder *decoder;
	const nsavi::STRH *stream_header;
	std::vector<nullsoft_h264_frame_data> buffered_frames;
protected: 
	RECVS_DISPATCH;
};