#ifndef NULLSOFT_F263_LIB_H
#define NULLSOFT_F263_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#define F263_OK 0
#define F263_ERROR_TOO_MUCH_DATA 2
#define F263_ERROR_NO_DATA 3

#include "../Winamp/wa_ipc.h"

void *F263_CreateDecoder();
int F263_DecodeFrame(void *context, void *frameData, size_t frameSize, YV12_PLANES *yv12, int *width, int *height, int *keyframe);
void F263_DestroyDecoder(void *context);

#ifdef __cplusplus
}
#endif

#endif
