#ifndef NULLSOFT_PODCAST_PLUGIN_LAYOUT_HEADER
#define NULLSOFT_PODCAST_PLUGIN_LAYOUT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef struct __LAYOUTITEM
{ 
	HWND hwnd;
	LONG x;
	LONG y;
	LONG cx;
	LONG cy;
	UINT flags;
	RECT rect;
} LAYOUTITEM;


#define LI_GET_R(__li) ((__li).x + (__li).cx)
#define LI_GET_B(__li) ((__li).y + (__li).cy)
#define LI_EXPAND_W(__li, __delta) { (__li).cx += (__delta); }
#define LI_EXPAND_H(__li, __delta) { (__li).cy += (__delta); }
#define LI_SHIFT_L(__li, __delta) { (__li).x += (__delta); }
#define LI_SHIFT_T(__li, __delta) { { (__li).y += (__delta); }
#define LI_SET_L(__li, __val) { (__li).x = (__val); }
#define LI_SET_T(__li, __val) { (__li).y = (__val); }
#define LI_SET_W(__li, __val) { (__li).cx = (__val); }
#define LI_SET_H(__li, __val) { (__li).cy = (__val); }
#define LI_SET_R(__li, __val) { (__li).cx = ((__val) - (__li).x); }
#define LI_SET_B(__li, __val) { (__li).cy = ((__val) - (__li).y); }

BOOL Layout_Initialize(HWND hwnd, const INT *itemList, INT itemCount, LAYOUTITEM *layout);
BOOL Layout_SetVisibilityEx(const RECT *rect, const INT *indexList, INT indexCount, LAYOUTITEM *layout);
BOOL Layout_SetVisibility(const RECT *rect, LAYOUTITEM *layout, INT layoutCount);
BOOL Layout_Perform(HWND hwnd, LAYOUTITEM *layout, INT layoutCount, BOOL fRedraw);

BOOL Layout_GetValidRgn(HRGN validRgn, POINTS parrentOffset, const RECT *validRect, LAYOUTITEM *layout, INT layoutCount);


#endif //NULLSOFT_PODCAST_PLUGIN_LAYOUT_HEADER