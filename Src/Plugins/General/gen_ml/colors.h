#ifndef NULLOSFT_MEDIALIBRARY_COLORS_HEADER
#define NULLOSFT_MEDIALIBRARY_COLORS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

//******************************* Window
#define MLSO_WINDOW			0

#define WP_BACKGROUND		0
#define WBS_NORMAL			0
#define WBS_HILITED			1

#define WP_TEXT				1
#define WP_FRAME				2

//******************************* Menu
#define MLSO_MENU			2

#define MP_BACKGROUND		0
#define	MBS_NORMAL			0
#define MBS_SELECTED			1
#define MBS_SELECTEDFRAME	2

#define MP_TEXT				1
#define MTS_NORMAL			0
#define MTS_SELECTED			1
#define MTS_DISABLED			2

#define MP_SEPARATOR			2
#define MP_FRAME				3

//******************************* ToolTip
#define MLSO_TOOLTIP			3

#define TTP_BACKGROUND		0
#define TTP_TEXT			1
#define TTP_FRAME			2

//******************************* omBrowser
#define MLSO_OMBROWSER		5

#define BP_BACKGROUND		0

#define BP_TEXT				1
#define BTS_NORMAL			0
#define BTS_DISABLED		1

#define BP_LINK				2
#define BLS_NORMAL			0
#define BLS_ACTIVE			1
#define BLS_VISITED			2
#define BLS_HOVER			3

void ResetColors(BOOL fImmediate);
HRESULT MLGetSkinColor(UINT uObject, UINT uPart, UINT uState, COLORREF *pColor);

INT GetColorDistance(COLORREF rgb1, COLORREF rgb2);
COLORREF GetDarkerColor(COLORREF rgb1, COLORREF rgb2);
COLORREF BlendColors(COLORREF rgbTop, COLORREF rgbBottom, INT alpha);

#endif //NULLOSFT_MEDIALIBRARY_COLORS_HEADER