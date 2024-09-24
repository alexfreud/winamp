#pragma once
#include "../Plugins/Input/in_avi/ifc_avivideodecoder.h"
#include "../Plugins/Input/in_avi/svc_avidecoder.h"
#include "../nsavi/avi_header.h"
#include "loader_jpg.h"
extern "C"
{
#undef FAR
#include "jpeglib.h"
};

// {F54E12B2-1B6B-48ac-B65D-F33B690A4F81}
static const GUID avi_jpeg_guid = 
{ 0xf54e12b2, 0x1b6b, 0x48ac, { 0xb6, 0x5d, 0xf3, 0x3b, 0x69, 0xa, 0x4f, 0x81 } };


class AVIDecoderCreator : public svc_avidecoder
{
public:
	static const char *getServiceName() { return "JPEG AVI Decoder"; }
	static GUID getServiceGuid() { return avi_jpeg_guid; } 
	int CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class AVIMJPEG : public ifc_avivideodecoder
{
public:
	AVIMJPEG(nsavi::video_format *stream_format);
	static AVIMJPEG *CreateDecoder(nsavi::video_format *stream_format);
	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip);
	int DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes);
	void Flush();
	void Close();
	int GetPicture(void **data, void **decoder_data); 
	void FreePicture(void *data, void *decoder_data);

private:
	nsavi::video_format *stream_format;
	JpgLoad *jpegLoader; 
	int width, height;
	int image_size;
	void *decoded_image; // UYVY
	bool image_outputted;
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
protected: 
	RECVS_DISPATCH;
};