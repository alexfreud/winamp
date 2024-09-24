#ifndef NULLOSFT_SKINNED_MENU_HEADER
#define NULLOSFT_SKINNED_MENU_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ml.h"
#include "./ml_ipc_0313.h"

#define Menu_TrackPopup(library, hMenu, fuFlags, x, y, hwnd, lptpm) \
		Menu_TrackPopupParam(library, hMenu, fuFlags, x, y, hwnd, lptpm, 0)

INT Menu_TrackPopupParam(HWND library, HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, ULONG_PTR param);

#endif //NULLOSFT_SKINNED_MENU_HEADER