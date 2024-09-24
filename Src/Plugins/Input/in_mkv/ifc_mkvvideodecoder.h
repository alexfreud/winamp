#pragma once
#include <bfc/dispatch.h>

class NOVTABLE ifc_mkvvideodecoder : public Dispatchable
{
protected:
	ifc_mkvvideodecoder() {}
	~ifc_mkvvideodecoder() {}
public:
	enum
	{
		MKV_SUCCESS = 0,
		MKV_NEED_MORE_INPUT = -1,
		MKV_FAILURE=1,
	};
	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio);
	int DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp);
	void Flush();
	void Close();
	int GetPicture(void **data, void **decoder_data, uint64_t *timestamp); 
	void FreePicture(void *data, void *decoder_data);
	void EndOfStream(); // signal to the decoder that the video bitstream is over - flush any buffered frames
	void HurryUp(int state); // 1 = hurry up (drop unnecessary frames), 0 = revert to normal
	DISPATCH_CODES
	{
		GET_OUTPUT_PROPERTIES = 0,
			DECODE_BLOCK = 1,
			FLUSH = 2,
			CLOSE = 3,
			GET_PICTURE = 4,
			FREE_PICTURE = 5,
			END_OF_STREAM = 6,
			HURRY_UP = 7,
	};
};

inline int ifc_mkvvideodecoder::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio)
{
	return _call(GET_OUTPUT_PROPERTIES, (int)MKV_FAILURE, x, y, color_format, aspect_ratio);
}
inline int ifc_mkvvideodecoder::DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp)
{
	return _call(DECODE_BLOCK, (int)MKV_FAILURE, inputBuffer, inputBufferBytes, timestamp);
}
inline void ifc_mkvvideodecoder::Flush()
{
	_voidcall(FLUSH);
}
inline void ifc_mkvvideodecoder::Close()
{
	_voidcall(CLOSE);
}
inline int ifc_mkvvideodecoder::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
{
	return _call(GET_PICTURE, (int)MKV_FAILURE, data, decoder_data, timestamp);
}
inline void ifc_mkvvideodecoder::FreePicture(void *data, void *decoder_data)
{
	_voidcall(FREE_PICTURE, data, decoder_data);
}
inline void ifc_mkvvideodecoder::EndOfStream()
{
	_voidcall(END_OF_STREAM);
}

inline void ifc_mkvvideodecoder::HurryUp(int state)
{
	_voidcall(HURRY_UP, state);
}
