#ifndef NULLSOFT_WINAMP_OMBROWSER_MENU_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_MENU_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class ifc_menucustomizer;

#define MENU_RATING			2
#define MENU_TOOLBAR		3
#define MENU_ADDRESSBAR		4

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

BOOL Menu_TrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm);
HANDLE Menu_InitializeHook(HWND hwnd, ifc_menucustomizer *customizer);
HRESULT Menu_RemoveHook(HANDLE menuHook);

#endif //NULLSOFT_WINAMP_OMBROWSER_MENU_HEADER