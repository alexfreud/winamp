// HotKey.cpp: implementation of the CHotKey class.
//
//////////////////////////////////////////////////////////////////////

#include "gen_hotkeys.h"
#include "HotKey.h"

//////////////////////////////////////////////////////////////////////
// Registration / Unregstration
//////////////////////////////////////////////////////////////////////

int RegisterHotkey(HOTKEY *hk)
{
	if (!hk) return 1;

	wchar_t atomName[1024] = {0};
	bool unicode = 0;
	char* name = GetCommandName(hk->hkd.iCommand, &unicode);
	StringCchPrintfW(atomName, 1024, (unicode?L"%s %X %X %X":L"%hs %X %X %X"), name, hk->hkd.dwHotKey, hk->hkd.iCommand, GetTickCount());
	hk->atom = GlobalAddAtomW(atomName);
	if (!hk->atom)
		return 1;

	return !RegisterHotKey(psPlugin.hwndParent, hk->atom, GetModKeys(hk->hkd.dwHotKey), LOBYTE(hk->hkd.dwHotKey));
}

void UnregisterHotkey(HOTKEY *hk)
{
	if (!hk || !hk->atom)
		return;

	UnregisterHotKey(psPlugin.hwndParent, hk->atom);
	GlobalDeleteAtom(hk->atom);
	hk->atom = NULL;
}

//////////////////////////////////////////////////////////////////////
// Set / Get
//////////////////////////////////////////////////////////////////////

UINT GetModKeys(DWORD dwHotKey)
{
	UINT result = 0;
	WORD wHotKey = HIBYTE(dwHotKey);

	if (wHotKey & HOTKEYF_ALT)
		result |= MOD_ALT;
	if (wHotKey & HOTKEYF_CONTROL)
		result |= MOD_CONTROL;
	if (wHotKey & HOTKEYF_WIN)
		result |= MOD_WIN;
	if (wHotKey & HOTKEYF_SHIFT)
		result |= MOD_SHIFT;

	return result;
}