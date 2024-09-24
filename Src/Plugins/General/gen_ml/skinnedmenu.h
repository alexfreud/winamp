#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_MENU_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_MENU_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./skinnedmenuthreadinfo.h"

class SkinnedMenu
{
public:
	SkinnedMenu();
	virtual ~SkinnedMenu(void);

public:
	static HWND WindowFromHandle(HMENU menu);

public:
	virtual BOOL InitializeHook(HWND hwndOwner, UINT skinStyle, HMLIMGLST hmlil, INT width, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam);
	virtual BOOL TrackMenuPopupEx(HMENU hmenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, UINT skinStyle,
					HMLIMGLST hmlil, INT width, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam);

protected:
	virtual BOOL AttachToHwnd(HWND hwndMenu);

protected:
	friend class SkinnedMenuThreadInfo;

protected:
	HWND		hwndOwner;
	UINT		skinStyle;
	HMLIMGLST	hmlil;
	INT			width;
	MENUCUSTOMIZEPROC customProc;
	ULONG_PTR customParam;
	SkinnedMenuThreadInfo *threadInfo;
};


#endif // NULLOSFT_MEDIALIBRARY_SKINNED_MENU_HEADER