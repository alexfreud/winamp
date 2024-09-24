#ifndef NULLSOFT_VLB_OBJ_VLBDECODER_H
#define NULLSOFT_VLB_OBJ_VLBDECODER_H
#include "aacdecoderapi.h"
#include <bfc/dispatch.h>

class obj_vlbDecoder : public Dispatchable
{
protected:
	obj_vlbDecoder() {}
	~obj_vlbDecoder() {}
public:
	int Open(DataIOControl *paacInput);
	void Close();
  long Synchronize(AACStreamParameters *paacStreamParameters);
  long DecodeFrame(AACStreamParameters *paacStreamParameters);
	void Flush();
	size_t Read(void *buffer, size_t bufferlen);

	enum
	{
		OBJ_VLBDECODER_OPEN = 0,
		OBJ_VLBDECODER_CLOSE = 1,
		OBJ_VLBDECODER_SYNCHRONIZE = 2,
		OBJ_VLBDECODER_DECODEFRAME = 3,
		OBJ_VLBDECODER_FLUSH = 4,
		OBJ_VLBDECODER_READ = 5,
	};
};


inline int obj_vlbDecoder::Open(DataIOControl *paacInput)
{
	return _call(OBJ_VLBDECODER_OPEN, (int)0, paacInput);
}

inline void obj_vlbDecoder::Close()
{
	_voidcall(OBJ_VLBDECODER_CLOSE);
}

inline long obj_vlbDecoder::Synchronize(AACStreamParameters *paacStreamParameters)
{
	return _call(OBJ_VLBDECODER_SYNCHRONIZE, (long)ERR_SYNC_ERROR, paacStreamParameters);
}

inline long obj_vlbDecoder::DecodeFrame(AACStreamParameters *paacStreamParameters)
{
	return _call(OBJ_VLBDECODER_DECODEFRAME, (long)ERR_INVALID_BITSTREAM, paacStreamParameters);
}

inline void obj_vlbDecoder::Flush()
{
		_voidcall(OBJ_VLBDECODER_FLUSH);
}

inline size_t obj_vlbDecoder::Read(void *buffer, size_t bufferlen)
{
		return _call(OBJ_VLBDECODER_READ, (size_t)0, buffer, bufferlen);
}

// {A9F0E37C-69BA-48d7-BFEB-1C85A9F5A2FA}
static const GUID obj_vlbDecoderGUID = 
{ 0xa9f0e37c, 0x69ba, 0x48d7, { 0xbf, 0xeb, 0x1c, 0x85, 0xa9, 0xf5, 0xa2, 0xfa } };

#endif