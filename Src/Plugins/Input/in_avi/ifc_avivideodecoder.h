#pragma once
#include <bfc/dispatch.h>
#include <bfc/platform/types.h>

class NOVTABLE ifc_avivideodecoder : public Dispatchable
{
protected:
	ifc_avivideodecoder() {}
	~ifc_avivideodecoder() {}
public:
	enum
	{
		AVI_SUCCESS = 0,
		AVI_NEED_MORE_INPUT = -1,
		AVI_FAILURE=1,
	};
	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip);
	int DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes);
	void Flush();
	void Close();
	int GetPicture(void **data, void **decoder_data); 
	void FreePicture(void *data, void *decoder_data);
	void EndOfStream(); // signal to the decoder that the video bitstream is over - flush any buffered frames
	void HurryUp(int state); // 1 = hurry up (drop unnecessary frames), 0 = revert to normal
	int GetPalette(RGB32 **palette);
	DISPATCH_CODES
	{
		GET_OUTPUT_PROPERTIES = 0,
			DECODE_CHUNK = 1,
			FLUSH = 2,
			CLOSE = 3,
			GET_PICTURE = 4,
			FREE_PICTURE = 5,
			END_OF_STREAM = 6,
			HURRY_UP = 7,
			GET_PALETTE = 8,
	};
};

inline int ifc_avivideodecoder::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip)
{
	return _call(GET_OUTPUT_PROPERTIES, (int)AVI_FAILURE, x, y, color_format, aspect_ratio, flip);
}
inline int ifc_avivideodecoder::DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes)
{
	return _call(DECODE_CHUNK, (int)AVI_FAILURE, type, inputBuffer, inputBufferBytes);
}
inline void ifc_avivideodecoder::Flush()
{
	_voidcall(FLUSH);
}
inline void ifc_avivideodecoder::Close()
{
	_voidcall(CLOSE);
}
inline int ifc_avivideodecoder::GetPicture(void **data, void **decoder_data)
{
	return _call(GET_PICTURE, (int)AVI_FAILURE, data, decoder_data);
}
inline void ifc_avivideodecoder::FreePicture(void *data, void *decoder_data)
{
	_voidcall(FREE_PICTURE, data, decoder_data);
}
inline void ifc_avivideodecoder::EndOfStream()
{
	_voidcall(END_OF_STREAM);
}

inline void ifc_avivideodecoder::HurryUp(int state)
{
	_voidcall(HURRY_UP, state);
}

inline int ifc_avivideodecoder::GetPalette(RGB32 **palette)
{
	return _call(GET_PALETTE, (int)AVI_FAILURE, palette);
}
