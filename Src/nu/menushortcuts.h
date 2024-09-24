#ifndef NULLOSFT_MEDIALIBRARY_MENU_SHORTCUTS_HEADER
#define NULLOSFT_MEDIALIBRARY_MENU_SHORTCUTS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

#define MSF_REPLACE			0x0000
#define MSF_APPEND			0x0001
#define MSF_WALKSUBMENU		0x0100

BOOL AppendMenuShortcuts(HMENU hMenu, HACCEL *phAccel, INT count, UINT uMode);
BOOL AppendMenuShortcutsEx(HMENU hMenu, ACCEL *pszAccel, INT count, UINT uMode);
BOOL AppendShortcutText(LPWSTR pszItemText, INT cchTextMax, WORD wID, HACCEL *phAccel, INT count, UINT uMode);
BOOL AppendShortcutTextEx(LPWSTR pszItemText, INT cchTextMax, WORD wID, ACCEL *pszAccel, INT cchAccel, UINT uMode);


#endif // NULLOSFT_MEDIALIBRARY_MENU_SHORTCUTS_HEADER