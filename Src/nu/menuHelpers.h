#ifndef NULLOSFT_MENUHELPERS_HEADER
#define NULLOSFT_MENUHELPERS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
INT MenuHelper_CopyMenuEx(HMENU hDest, INT iDstStart, HMENU hSource, INT iSrcStart, INT iSrcCount);
INT MenuHelper_CopyMenu(HMENU hDest, INT iDstStart, HMENU hSource);
HMENU MenuHelper_DuplcateMenu(HMENU hMenu);

INT MenuHelper_InsertSeparator(HMENU hMenu, INT iPos);
void MenuHelper_RemoveRange(HMENU hMenu, UINT min, UINT max);
void MenuHelper_EnableGroup(HMENU hMenu, UINT *pszItems, INT cchItems, BOOL fByPos, BOOL bEnable);
BOOL MenuHelper_GetMenuItemPos(HMENU hMenu, UINT itemId, INT *pPos);

#endif // NULLOSFT_MENUHELPERS_HEADER