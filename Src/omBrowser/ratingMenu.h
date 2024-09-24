#ifndef NULLSOFT_WINAMP_OMBROWSER_RATING_MENU_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_RATING_MENU_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

class ifc_menucustomizer;

BOOL RatingMenu_InitializeMenu(HMENU ratingMenu, INT ratingValue);
HMENU RatingMenu_FindMenu(HMENU hMenu);
BOOL RatingMenu_SetValue(HMENU ratingMenu, INT ratingValue);
HRESULT RatingMenu_GetCustomizer(HMENU hMenu, ifc_menucustomizer **customizer);

#include <wtypes.h>

#endif //NULLSOFT_WINAMP_OMBROWSER_RATING_MENU_HEADER