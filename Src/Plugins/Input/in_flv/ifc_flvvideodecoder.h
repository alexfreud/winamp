#pragma once

enum
{
	FLV_VIDEO_SUCCESS = 0,
	FLV_VIDEO_FAILURE = 1,
};

class ifc_flvvideodecoder : public Dispatchable
{
protected:
	ifc_flvvideodecoder() {}
	~ifc_flvvideodecoder() {}
public:
	int GetOutputFormat(int *x, int *y, int *color_format);
	int DecodeSample(const void *inputBuffer, size_t inputBufferBytes, int32_t timestamp);
	void Flush();
	void Close();
	int GetPicture(void **data, void **decoder_data, uint64_t *timestamp); 
	void FreePicture(void *data, void *decoder_data);
	int Ready(); // returns 1 for ready [default], 0 for not ready.  Some codecs in FLV use the first packet for decoder config data.  return 1 from this once you've gotten it
	DISPATCH_CODES
	{
		FLV_VIDEO_GETOUTPUTFORMAT = 0,
			FLV_VIDEO_DECODE = 1,
			FLV_VIDEO_FLUSH = 2,
			FLV_VIDEO_CLOSE = 3,
			FLV_VIDEO_GET_PICTURE = 4,
			FLV_VIDEO_FREE_PICTURE = 5,
			FLV_VIDEO_READY = 6,
	};
};

inline int ifc_flvvideodecoder::GetOutputFormat(int *x, int *y, int *color_format)
{
	return _call(FLV_VIDEO_GETOUTPUTFORMAT, (int)FLV_VIDEO_FAILURE, x, y, color_format);
}

inline int ifc_flvvideodecoder::DecodeSample(const void *inputBuffer, size_t inputBufferBytes, int32_t timestamp)
{
	return _call(FLV_VIDEO_DECODE, (int)FLV_VIDEO_FAILURE, inputBuffer, inputBufferBytes, timestamp);
}

inline void ifc_flvvideodecoder::Flush()
{
	_voidcall(FLV_VIDEO_FLUSH);
}

inline void ifc_flvvideodecoder::Close()
{
	_voidcall(FLV_VIDEO_CLOSE);
}

inline int ifc_flvvideodecoder::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
{
	return _call(FLV_VIDEO_GET_PICTURE, (int)FLV_VIDEO_FAILURE, data, decoder_data, timestamp);
}

inline void ifc_flvvideodecoder::FreePicture(void *data, void *decoder_data)
{
	_voidcall(FLV_VIDEO_FREE_PICTURE, data, decoder_data);
}

inline int ifc_flvvideodecoder::Ready()
{
	return _call(FLV_VIDEO_READY, (int)1); // default to true so that decoders that don't implement won't block in_flv from seeking
}
