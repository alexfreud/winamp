#pragma once
#include "../Plugins/Input/in_avi/ifc_avivideodecoder.h"
#include "../nsavi/avi_header.h"

class AVIRLE : public ifc_avivideodecoder
{
public:
	AVIRLE(void *video_frame, nsavi::video_format *stream_format, size_t video_frame_size);
	static AVIRLE *CreateDecoder(nsavi::video_format *stream_format);
	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip);
	int DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes);
	void Flush();
	void Close();
	int GetPicture(void **data, void **decoder_data); 
	int GetPalette(RGB32 **palette);
private:
	nsavi::video_format *stream_format;
	size_t video_frame_size;
	uint8_t *video_frame;
	bool video_outputted;
	RGBQUAD palette[256];
	bool palette_retrieved;
protected: 
	RECVS_DISPATCH;
};