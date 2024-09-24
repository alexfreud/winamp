#ifndef NULLSOFT_MEIDALIBRARY_IMAGE_FILTER_HEADER
#define NULLSOFT_MEIDALIBRARY_IMAGE_FILTER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

typedef LPVOID HMLIMGFLTRMNGR;

typedef BOOL (CALLBACK *MLIMAGEFILTERPROC)(LPBYTE /*pData*/, LONG /*cx*/, LONG /*cy*/, INT /*bpp*/, COLORREF /*rgbBk*/, COLORREF /*rgbFg*/, INT_PTR /*imageTag*/, LPARAM /*lParam*/);

typedef struct _MLIMAGEFILTERINFO_I
{
	UINT				mask;
	GUID				uid;
	MLIMAGEFILTERPROC	fnProc;
	LPARAM				lParam;
	UINT				fFlags;
	LPWSTR				pszTitle;
	INT					cchTitleMax;

} MLIMAGEFILTERINFO_I;

// flags
#define MLIFF_IGNORE_BKCOLOR_I	0x0001
#define MLIFF_IGNORE_FGCOLOR_I	0x0002

// mask 
#define MLIFF_TITLE_I	0x0001
#define MLIFF_PARAM_I	0x0002
#define MLIFF_FLAGS_I	0x0004
#define MLIFF_PROC_I		0x0008


HMLIMGFLTRMNGR MLImageFilterI_CreateManager(INT cInitial, INT cGrow);
BOOL MLImageFilterI_DestroyManager(HMLIMGFLTRMNGR hmlifMngr);

BOOL MLImageFilterI_Register(HMLIMGFLTRMNGR hmlifMngr, MLIMAGEFILTERINFO_I *pmlif);  
BOOL MLImageFilterI_Unregister(HMLIMGFLTRMNGR hmlifMngr, REFGUID filterUID); 
BOOL MLImageFilterI_GetInfo(HMLIMGFLTRMNGR hmlifMngr, MLIMAGEFILTERINFO_I *pmlif);
BOOL MLImageFilterI_ApplyEx(HMLIMGFLTRMNGR hmlifMngr, const GUID *filterUID, LPBYTE pData, LONG cx, LONG cy, INT bpp, COLORREF rgbBk, COLORREF rgbFg, INT_PTR imageTag);
BOOL MLImageFilterI_Apply(HMLIMGFLTRMNGR hmlifMngr, const GUID *filterUID, HBITMAP hbmp, COLORREF rgbBk, COLORREF rgbFg, INT_PTR imageTag);


#endif //NULLSOFT_MEIDALIBRARY_IMAGE_FILTER_HEADER