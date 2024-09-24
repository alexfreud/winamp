#ifndef NULLOSFT_MEDIALIBRARY_MLDISC_COPYFILES_HEADER
#define NULLOSFT_MEDIALIBRARY_MLDISC_COPYFILES_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif


void MLDisc_InitializeCopyData();
void MLDisc_ReleaseCopyData();

// use CoTaskMemAlloc/CoTackMemFree to allocate buffers and each string. pszFSize can be NULL. if return TRUE do not free data.
BOOL MLDisc_CopyFiles(HWND hParent, LPWSTR *ppszFiles, ULONGLONG *pFSizes, INT count); 
BOOL MLDisc_IsDiscCopying(CHAR cLetter);
#ifdef __cplusplus
}
#endif



#endif // NULLOSFT_MEDIALIBRARY_MLDISC_COPYFILES_HEADER