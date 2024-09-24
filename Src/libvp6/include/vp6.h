#pragma once
#include "duck_dxl.h"
#ifdef __cplusplus
extern "C" {
#endif

int vp60_decompress(DXL_XIMAGE_HANDLE src);
void vp60_SetParameter(DXL_XIMAGE_HANDLE src, int Command, uintptr_t Parameter);
int vp60_getWH(DXL_XIMAGE_HANDLE src, int *w, int *h);

#ifdef __cplusplus
}
#endif