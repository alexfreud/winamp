#ifndef NULLOSFT_MEDIALIBRARY_IMAGELIST_HEADER
#define NULLOSFT_MEDIALIBRARY_IMAGELIST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


#include <windows.h>		
#include <commctrl.h>	
#include "./ml_imageloader.h"		// image loading
#include "./ml_imagefilter.h"		// image loading

typedef LPVOID HMLIMGLST;

#define MAX_ALLOWED_CACHE_SIZE			10
#define MAX_ALLOWED_LIST_SIZE			400


// create falgs
#define MLILC_COLOR24_I		0x00000018	// Use a 24-bit DIB section.
#define MLILC_COLOR32_I		0x00000020	// Use a 32-bit DIB section.
#define MLILC_MASK_I			0x00000001	// Use a mask.

HMLIMGLST MLImageListI_Create(INT cx, INT cy, UINT flags, INT cInitial, INT cGrow, INT cCacheSize, HMLIMGFLTRMNGR hmlifManager);
BOOL MLImageListI_Destroy(HMLIMGLST hmlil);

HIMAGELIST MLImageListI_GetRealList(HMLIMGLST hmlil);
INT MLImageListI_GetRealIndex(HMLIMGLST hmlil, INT index, COLORREF rgbBk, COLORREF rgbFg);


INT MLImageListI_Add(HMLIMGLST hmlil, MLIMAGESOURCE_I *pImageSource, REFGUID filterUID, INT_PTR nTag);
BOOL MLImageListI_Replace(HMLIMGLST hmlil, INT index, MLIMAGESOURCE_I *pImageSource, REFGUID filterUID, INT_PTR nTag);
BOOL MLImageListI_Remove(HMLIMGLST hmlil, INT index);


BOOL MLImageListI_GetImageSize(HMLIMGLST hmlil, INT *cx, INT *cy);
INT MLImageListI_GetImageCount(HMLIMGLST hmlil);
INT MLImageListI_GetIndexFromTag(HMLIMGLST hmlil, INT_PTR nTag);
BOOL MLImageListI_GetTagFromIndex(HMLIMGLST hmlil, INT index, INT_PTR *nTag);
BOOL MLImageListI_CheckItemExist(HMLIMGLST hmlil, INT index);


#endif //NULLOSFT_MEDIALIBRARY_IMAGELIST_HEADER