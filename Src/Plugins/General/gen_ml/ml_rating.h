#ifndef NULLOSFT_MEDIALIBRARY_RATING_HEADER
#define NULLOSFT_MEDIALIBRARY_RATING_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include "./ml_imagelist.h"

/// Rating_Draw Styles
#define RDS_SHOWEMPTY_I     0x00000001      // Draw elements that not set.
#define RDS_OPAQUE_I        0x00000002      // Fill rest of the rectangle with rgbBk.
#define RDS_HOT_I           0x00000004      // Draw elements as "hot".
#define RDS_LEFT_I          0x00000000      // Aligns elements to the left.
#define RDS_TOP_I           0x00000000      // Justifies elements to the top of the rectangle.
#define RDS_RIGHT_I         0x00000010      // Aligns elements to the right.
#define RDS_BOTTOM_I        0x00000020      // Justifies elements to the bottom of the rectangle.
#define RDS_HCENTER_I       0x00000040      // Centers elements horizontally in the rectangle.
#define RDS_VCENTER_I       0x00000080      // Centers elements horizontally in the rectangle.
#define RDS_INACTIVE_HOT_I  0x00000100      // Draw elements as "hot" when inactive

#define RDS_NORMAL_I        (RDS_SHOWEMPTY_I | RDS_OPAQUE_I | RDS_LEFT | RDS_TOP)

// Rating_HitTest hitFlags
#define RHT_NOWHERE_I       0x0001
#define RHT_ONVALUE_I       0x0002
#define RHT_ONVALUEABOVE_I  0x0004
#define RHT_ONVALUEBELOW_I  0x0008
#define RHT_TOLEFT_I        0x0100
#define RHT_TORIGHT_I       0x0200

// Draws Rating based on RATINGDRAWPARAMS
BOOL MLRatingI_Draw(HDC	hdc, INT maxValue, INT value, INT trackingVal, HMLIMGLST hmlil, INT index, RECT *prc, UINT fStyle);
// HIWORD - hitFlags LOWORD - index if any
LONG MLRatingI_HitTest(POINT pt, INT maxValue, HMLIMGLST hmlil, RECT *prc, UINT fStyle);
BOOL MLRatingI_CalcMinRect(INT maxValue, HMLIMGLST hmlil, RECT *prc);

#endif //NULLOSFT_MEDIALIBRARY_RATING_HEADER