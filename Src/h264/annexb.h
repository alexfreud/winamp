#pragma once
#include <bfc/platform/types.h>

#ifdef __cplusplus
extern "C" {
#endif
	enum
	{
		AnnexB_UnitAvailable = 0, // data was added succesfully and a new unit is available via GetUnit().  
		AnnexB_BufferFull = 1, // no start code found within the maximum unit length
		AnnexB_NeedMoreData = 2, // no unit ready yet, pass in the next data chunk
		AnnexB_Error = 3, // general error (out of memory, null pointer, etc)
	};

typedef void *h264_annexb_demuxer_t;
h264_annexb_demuxer_t AnnexB_Create(int size);
void AnnexB_Destroy(h264_annexb_demuxer_t demuxer);
int AnnexB_AddData(h264_annexb_demuxer_t demuxer, const void **data, size_t *data_len);
void AnnexB_EndOfStream(h264_annexb_demuxer_t demuxer);
int AnnexB_GetUnit(h264_annexb_demuxer_t demuxer, const void **data, size_t *data_len);

#ifdef __cplusplus
}
#endif