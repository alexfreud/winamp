#include <precomp.h>
#ifdef WIN32
#include <windows.h>
#include <shellapi.h>
#endif
#include "systray.h"
#include <bfc/assert.h>
#include <bfc/wasabi_std.h>

Systray::Systray(HWND wnd, int uid, int msg, HICON smallicon)
{
	id = uid;
	hwnd = wnd;
	message = msg;
	icon = smallicon;
	/*int r = */addIcon();
	// always asserts with desktop with no systray support (litestep, WINE, etc...)
	//  ASSERT(r == TRUE);
}

Systray::~Systray()
{
	/*int r = */deleteIcon();
	// always asserts with desktop with no systray support (litestep, WINE, etc...)
	//  ASSERT(r == TRUE);
}

void Systray::setTip(const wchar_t *_tip)
{
	tip = _tip;
	tip.trunc(64);
	if (!tip.isempty())
	{
		/*int r = */setTip();
		// always asserts with desktop with no systray support (litestep, WINE, etc...)
		//    ASSERT(r == TRUE);
	}
}

bool Systray::addIcon()
{
#ifdef WIN32
	NOTIFYICONDATAW tnid = {0};
	tnid.cbSize = sizeof(NOTIFYICONDATAW);
	tnid.uID = id;
	tnid.hWnd = hwnd;
	tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnid.uCallbackMessage = message;
	tnid.hIcon = icon;
	if (tip)
		WCSCPYN(tnid.szTip, tip, sizeof(tnid.szTip)/sizeof(wchar_t));

	return !!Shell_NotifyIconW(NIM_ADD, &tnid);
#else
	DebugString("portme Systray::addIcon\n");
	return 1;
#endif
}

bool Systray::setTip()
{
#ifdef WIN32
	NOTIFYICONDATAW tnid = {0};
	tnid.cbSize = sizeof(NOTIFYICONDATAW);
	tnid.uFlags = NIF_TIP;
	tnid.uID = id;
	tnid.hWnd = hwnd;
	if (tip)
		WCSCPYN(tnid.szTip, tip, sizeof(tnid.szTip)/sizeof(wchar_t));

	return !!Shell_NotifyIconW(NIM_MODIFY, &tnid);
#else
	DebugString("portme Systray::setTip\n");
	return 1;
#endif
}


bool Systray::deleteIcon()
{
#ifdef WIN32
	NOTIFYICONDATA tnid = {0};
	tnid.cbSize = sizeof(NOTIFYICONDATAW);
	tnid.hWnd = hwnd;
	tnid.uID = id;

	return !!Shell_NotifyIcon(NIM_DELETE, &tnid);
#else
	DebugString("portme Systray::deleteIcon\n");
	return 1;
#endif
}

