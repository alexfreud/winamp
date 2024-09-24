#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_MENU_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_MENU_HEADER


#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../General/gen_ml/ml_ipc_0313.h"

class OmService;

#define OMMENU_SERVICECONTEXT	0
#define OMMENU_GALERYCONTEXT	1
#define OMMENU_RATING			2
#define OMMENU_TOOLBAR			3

#define MCF_VIEW			0x00000001
#define MCF_NAVIGATION		0x00000004
#define MCF_VIEWACTIVE		0x00000008

#define MCF_RATINGMASK	0xF0000000
#define MCF_RATING1		0x10000000
#define MCF_RATING2		0x20000000
#define MCF_RATING3		0x30000000
#define MCF_RATING4		0x40000000
#define MCF_RATING5		0x50000000

#define RATINGTOMCF(__rating) ((0x0F & (__rating)) << 24)
#define RATINGFROMMCF(__mcf) (0x0F & ((__mcf) >> 24))

HMENU Menu_GetMenu(INT menuKind, UINT flags);
void Menu_ReleaseMenu(HMENU hMenu, INT menuKind);

HMENU Menu_FindRatingMenu(HMENU hMenu);
BOOL Menu_SetRatingValue(HMENU ratingMenu, INT ratingValue);
INT DoTrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y,  HWND hwnd, LPTPMPARAMS lptpm);

#endif //NULLOSFT_ONLINEMEDIA_PLUGIN_MENU_HEADER