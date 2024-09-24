#ifndef NULLOSFT_MEDIALIBRARY_FILEVIEW_CONTROL_HEADER
#define NULLOSFT_MEDIALIBRARY_FILEVIEW_CONTROL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

HWND FileView_CreateDialog(HWND hwndParent, UINT fStyle, HWND hwndInsertAfter, INT x, INT y, INT cx, INT cy);


#ifdef __cplusplus
}
#endif



#endif // NULLOSFT_MEDIALIBRARY_FILEVIEW_CONTROL_HEADER