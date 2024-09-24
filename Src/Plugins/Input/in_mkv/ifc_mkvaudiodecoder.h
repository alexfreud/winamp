#pragma once
#include <bfc/dispatch.h>

class NOVTABLE ifc_mkvaudiodecoder : public Dispatchable
{
protected:
	ifc_mkvaudiodecoder() {}
	~ifc_mkvaudiodecoder() {}
public:
	enum
	{
		MKV_SUCCESS = 0,
		MKV_NEED_MORE_INPUT = -1,
		MKV_FAILURE=1,
	};
	int OutputFrameSize(size_t *frame_size);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat); // can return an error code for "havn't locked to stream yet"
	int DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes);
	void Flush();
	void Close(); // self-destructs
	void EndOfStream(); // no more input, output anything you have buffered
	DISPATCH_CODES
	{
		OUTPUT_FRAME_SIZE = 0,
			GET_OUTPUT_PROPERTIES = 1,
			DECODE_BLOCK = 2,
			FLUSH = 3,
			CLOSE = 4,
			END_OF_STREAM = 5,
	};
};

inline int ifc_mkvaudiodecoder::OutputFrameSize(size_t *frame_size)
{
	return _call(OUTPUT_FRAME_SIZE, (int)MKV_FAILURE, frame_size);
}

inline int ifc_mkvaudiodecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	return _call(GET_OUTPUT_PROPERTIES, (int)MKV_FAILURE, sampleRate, channels, bitsPerSample, isFloat);
}

inline int ifc_mkvaudiodecoder::DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	return _call(DECODE_BLOCK, (int)MKV_FAILURE, inputBuffer, inputBufferBytes, outputBuffer, outputBufferBytes);
}

inline void ifc_mkvaudiodecoder::Flush()
{
	_voidcall(FLUSH);
}

inline void ifc_mkvaudiodecoder::Close()
{
	_voidcall(CLOSE);
}

inline void ifc_mkvaudiodecoder::EndOfStream()
{
	_voidcall(END_OF_STREAM);
}
