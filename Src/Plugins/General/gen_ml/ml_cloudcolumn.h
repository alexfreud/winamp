#ifndef NULLOSFT_MEDIALIBRARY_CLOUD_COLUMN_HEADER
#define NULLOSFT_MEDIALIBRARY_CLOUD_COLUMN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif
#include <windows.h>

typedef struct _CLOUDCOLUMNPAINT_I
{
	HWND        hwndList;	// hwnd of the listview
	HDC         hdc;		// hdc
	UINT        iItem;		// item index
	UINT        iSubItem;	// subitem index
	INT         value;		// database cloud status (1=full,2=partial,3=unavail)
	RECT        *prcItem;	// whole item rect (plvcd->nmcd.rc)
	RECT        *prcView;	// client area size (you can get it at CDDS_PREPAINT in plvcd->nmcd.rc)
	COLORREF    rgbBk;		// color to use as background (plvcd->clrTextBk)
	COLORREF    rgbFg;		// color to use as foreground (plvcd->clrText)
} CLOUDCOLUMNPAINT_I;

typedef struct _CLOUDBACKTEXT_I
{
	LPWSTR	pszText;
	INT		cchTextMax;
	INT		nColumnWidth; // used if style is RCS_ALLIGN_CENTER or RCS_ALLIGN_RIGHT
} CLOUDBACKTEXT_I;

BOOL MLCloudColumnI_Initialize(void); // call it before any other. You can call it any time something changed
BOOL MLCloudColumnI_Paint(CLOUDCOLUMNPAINT_I *pRCPaint);
INT MLCloudColumnI_GetMinWidth(void);
INT MLCloudColumnI_GetWidth(INT width);

#endif // NULLOSFT_MEDIALIBRARY_CLOUD_COLUMN_HEADER