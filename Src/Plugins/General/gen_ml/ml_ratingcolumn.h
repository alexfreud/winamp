#ifndef NULLOSFT_MEDIALIBRARY_RATING_COLUMN_HEADER
#define NULLOSFT_MEDIALIBRARY_RATING_COLUMN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif
#include <windows.h>

// Styles
#define RCS_DEFAULT_I				0xFFFFFFFF		// use default gen_ml style
// layout
#define RCS_ALLIGN_LEFT_I           0x00000000
#define RCS_ALLIGN_CENTER_I         0x00000001
#define RCS_ALLIGN_RIGHT_I          0x00000002
// showepmty
#define RCS_SHOWEMPTY_NEVER_I       0x00000000
#define RCS_SHOWEMPTY_NORMAL_I      0x00000010
#define RCS_SHOWEMPTY_HOT_I         0x00000020
#define RCS_SHOWEMPTY_ANIMATION_I   0x00000040
#define RCS_SHOWEMPTY_ALWAYS_I      0x00000070
#define RCS_SHOWINACTIVE_HOT_I      0x00000080
// traking (when)
#define RCS_TRACK_NEVER_I           0x00000000
#define RCS_TRACK_WNDFOCUSED_I      0x00000100
#define RCS_TRACK_ANCESTORACITVE_I  0x00000200
#define RCS_TRACK_PROCESSACTIVE_I   0x00000400
#define RCS_TRACK_ALWAYS_I          0x00000800
// traking (what)
#define RCS_TRACKITEM_ALL_I         0x00000000
#define RCS_TRACKITEM_SELECTED_I    0x00100000
#define RCS_TRACKITEM_FOCUSED_I     0x00200000

#define RCS_BLOCKCLICK_I            0x01000000
#define RCS_BLOCKUNRATECLICK_I      0x02000000
#define RCS_BLOCKDRAG_I             0x04000000

#define RCS_SIZE_ALLOWDECREASE_I    0x10000000
#define RCS_SIZE_ALLOWINCREASE_I    0x20000000

#define RATING_DEFAULT_STYLE        (RCS_ALLIGN_LEFT_I | \
                                    RCS_SHOWEMPTY_HOT_I | \
                                    RCS_SHOWEMPTY_ANIMATION_I | \
                                    RCS_TRACK_PROCESSACTIVE_I | \
                                    RCS_TRACKITEM_ALL_I | \
                                    /*RCS_BLOCKUNRATECLICK_I |*/ \
                                    0)

typedef struct _RATINGCOLUMNPAINT_I
{
	HWND        hwndList;	// hwnd of the listview
	HDC         hdc;		// hdc
	UINT        iItem;		// item index
	UINT        iSubItem;	// subitem index
	INT         value;		// database rating value
	RECT        *prcItem;	// whole item rect (plvcd->nmcd.rc)
	RECT        *prcView;	// client area size (you can get it at CDDS_PREPAINT in plvcd->nmcd.rc)
	COLORREF    rgbBk;		// color to use as background (plvcd->clrTextBk)
	COLORREF    rgbFg;		// color to use as foreground (plvcd->clrText)
	UINT        fStyle;		// style to use RCS_XXX
} RATINGCOLUMNPAINT_I;

typedef struct _RATINGCOLUMN_I
{
	HWND    hwndList;
	UINT    iItem;
	UINT    iSubItem;
	INT     value;
	POINT   ptAction;		// 
	BOOL    bRedrawNow;		// You want list to be redrawn immediatly
	BOOL    bCanceled;		// Used with EndDrag - i
	UINT    fStyle;			// RCS_XXX
} RATINGCOLUMN_I;

BOOL MLRatingColumnI_Initialize(void); // call it before any other. You can call it any time something changed
BOOL MLRatingColumnI_Update(void);		// call this when any skin / font changed happend
BOOL MLRatingColumnI_Paint(RATINGCOLUMNPAINT_I *pRCPaint);
BOOL MLRatingColumnI_Click(RATINGCOLUMN_I *pRating); // Set: hwndList, ptAction, bRedrawNow, fStyle. // Returns  TRUE if rating was clicked. Sets: iItem, iSubItem, value.
void MLRatingColumnI_Track(RATINGCOLUMN_I *pRating); // Set: hwndList, iItem, iSubItem, value, ptAction, bRedrawNow, fStyle. No Return.
BOOL MLRatingColumnI_BeginDrag(RATINGCOLUMN_I *pRating);	// Set: hwndList, iItem, iSubItem, value, fStyle.  Return TRUE if handled.
BOOL MLRatingColumnI_Drag(POINT pt);						// Return TRUE if handled.
BOOL MLRatingColumnI_EndDrag(RATINGCOLUMN_I *pRating);	// Set: ptAction, bCanceled, bRedrawNow. Return TRUE if handled. Sets: iItem, value.
void MLRatingColumnI_Animate(HWND hwndList, UINT iItem, UINT durationMs);
void MLRatingColumnI_CancelTracking(BOOL bRedrawNow);
INT MLRatingColumnI_GetMinWidth(void);
INT MLRatingColumnI_GetWidth(INT width, UINT fStyle); // will return you new width according to the policies
LPCWSTR MLRatingColumnI_FillBackString(LPWSTR pszText, INT cchTextMax, INT nColumnWidth, UINT fStyle);

typedef BOOL (CALLBACK *ONRATINGTWEAKAPLLY)(UINT /*newStyle*/, BOOL /*bClosing*/);
HWND MLRatingColumnI_TweakDialog(HWND hwndParent, UINT fStyle, ONRATINGTWEAKAPLLY fnApply, BOOL bVisible);
#endif // NULLOSFT_MEDIALIBRARY_RATING_COLUMN_HEADER