#pragma once
#include "../Plugins/Input/in_avi/ifc_avivideodecoder.h"
#include "../nsavi/avi_header.h"
#include "zlib.h"


class AVITSCC : public ifc_avivideodecoder
{
public:
	AVITSCC(void *video_frame, size_t video_frame_size, void *data, size_t data_len, nsavi::video_format *stream_format);
	static AVITSCC *CreateDecoder(nsavi::video_format *stream_format);
	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip);
	int DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes);
	void Flush();
	void Close();
	int GetPicture(void **data, void **decoder_data); 
private:
	nsavi::video_format *stream_format;
	uint8_t *video_frame;
	bool video_outputted;
	z_stream zlib_stream;
	uint8_t *data;
	size_t data_len;
	size_t video_frame_size;
protected: 
	RECVS_DISPATCH;
};