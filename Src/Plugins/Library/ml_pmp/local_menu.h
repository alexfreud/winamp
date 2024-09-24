#ifndef NULLOSFT_PORTABLES_PLUGIN_MENU_HEADER
#define NULLOSFT_PORTABLES_PLUGIN_MENU_HEADER


#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

INT Menu_TrackSkinnedPopup(HMENU hMenu, UINT fuFlags, INT x, INT y,  HWND hwnd, LPTPMPARAMS lptpm);
BOOL Menu_SetRatingValue(HMENU ratingMenu, INT ratingValue);
HMENU Menu_FindRatingMenu(HMENU hMenu, BOOL fUseMarker);

#endif //NULLOSFT_PORTABLES_PLUGIN_MENU_HEADER