#pragma once
#include <Mftransform.h>
#include <bfc/platform/types.h>
#include "../Winamp/wa_ipc.h"

struct nullsoft_h264_frame_data
{
	void *data;
	void *decoder_data;
	uint64_t local_timestamp;
};

class MFTDecoder
{
public:
	MFTDecoder();
	~MFTDecoder();
	
	HRESULT Open();
	HRESULT Feed(const void *data, size_t data_size, uint64_t timestamp_hundred_nanos);
	HRESULT FeedRaw(const void *data, size_t data_size, uint64_t timestamp_hundred_nanos);
	HRESULT GetFrame(IMFMediaBuffer **output_buffer, uint64_t *hundrednanos);
	HRESULT GetFrame(YV12_PLANES **data, void **decoder_data, uint64_t *mft_timestamp);
	HRESULT FreeFrame(YV12_PLANES *data, void *decoder_data);
	HRESULT GetOutputFormat(UINT *width, UINT *height, bool *flip, double *aspect);
	HRESULT Flush();
	HRESULT Drain();

	HRESULT GetVideoDisplayArea(IMFMediaType* pType, MFVideoArea* pArea);

	IMFTransform *decoder;
	LONG stride;
	UINT32 width, height;

private:
	MFVideoArea MakeArea(float x, float y, DWORD width, DWORD height);
	MFOffset MakeOffset(float v);
};