#pragma once
#include <mftransform.h>
#include <bfc/platform/types.h>

// generic API for use by all decoder flavors (MP4, MKV, etc)

class MFTDecoder
{
public:
	MFTDecoder();
	~MFTDecoder();
	HRESULT Open(const void *asc, size_t asc_bytes);
	HRESULT Open();
	void Flush();
	HRESULT GetOutputProperties(uint32_t *sampleRate, uint32_t *channels);
	HRESULT Feed(const void *buffer, size_t bufferBytes);
	HRESULT Decode(void *outputBuffer, size_t *outputBufferBytes, unsigned int bitsPerSample, bool useFloat, double gain);
	HRESULT OutputBlockSizeSamples(size_t *frameSize);
	bool AcceptingInput();
private:
	IMFTransform *decoder;
	IMFMediaBuffer *output_buffer;
	IMFSample *output_sample;
};