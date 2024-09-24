#pragma once
#include <bfc/dispatch.h>

class NOVTABLE ifc_aviaudiodecoder : public Dispatchable
{
protected:
	ifc_aviaudiodecoder() {}
	~ifc_aviaudiodecoder() {}
public:
	enum
	{
		AVI_SUCCESS = 0,
		AVI_NEED_MORE_INPUT = -1,
		AVI_FAILURE=1,
		AVI_RESYNC=2,
	};
	int OutputFrameSize(uint32_t *frame_size);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat); // can return an error code for "havn't locked to stream yet"
	// many AVI files arbitrarily divide the data stream (e.g. an MP3 frame might span two chunks).  Others put ALL the audio into ONE chunk. awesome.
	// for this reason, you are given double pointers to the data buffer and a pointer to the data size
	// and you are expected to update it on return
	// if inputBufferBytes != 0, you will be called again with the same data (return AVI_SUCCESS, though)
	// if you were unable to decode because of bitstream errors, return AVI_RESYNC so in_avi can try to correct timing
	int DecodeChunk(uint16_t type, const void **inputBuffer, uint32_t *inputBufferBytes, void *outputBuffer, uint32_t *outputBufferBytes);
	void Flush();
	void Close(); // self-destructs
	void EndOfStream(); // no more input, output anything you have buffered
	DISPATCH_CODES
	{
		OUTPUT_FRAME_SIZE = 0,
			GET_OUTPUT_PROPERTIES = 1,
			DECODE_CHUNK = 2,
			FLUSH = 3,
			CLOSE = 4,
			END_OF_STREAM = 5,
	};
};

inline int ifc_aviaudiodecoder::OutputFrameSize(uint32_t *frame_size)
{
	return _call(OUTPUT_FRAME_SIZE, (int)AVI_FAILURE, frame_size);
}

inline int ifc_aviaudiodecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	return _call(GET_OUTPUT_PROPERTIES, (int)AVI_FAILURE, sampleRate, channels, bitsPerSample, isFloat);
}

inline int ifc_aviaudiodecoder::DecodeChunk(uint16_t type, const void **inputBuffer, uint32_t *inputBufferBytes, void *outputBuffer, uint32_t *outputBufferBytes)
{
	return _call(DECODE_CHUNK, (int)AVI_FAILURE, type, inputBuffer, inputBufferBytes, outputBuffer, outputBufferBytes);
}

inline void ifc_aviaudiodecoder::Flush()
{
	_voidcall(FLUSH);
}

inline void ifc_aviaudiodecoder::Close()
{
	_voidcall(CLOSE);
}

inline void ifc_aviaudiodecoder::EndOfStream()
{
	_voidcall(END_OF_STREAM);
}
