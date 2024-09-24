// HotKey.h: interface for the CHotKey class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HOTKEY_H__82523B98_0E49_4B2A_8206_A4262D29C3C8__INCLUDED_)
#define AFX_HOTKEY_H__82523B98_0E49_4B2A_8206_A4262D29C3C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gen_hotkeys.h"

struct HOTKEY_DATA
{
	DWORD dwHotKey;
	int iCommand;
	wchar_t *szCommand;
};

struct HOTKEY
{
	HOTKEY_DATA hkd;
	ATOM atom;
	BOOL failed;
};

int RegisterHotkey(HOTKEY *hk);
void UnregisterHotkey(HOTKEY *hk);
UINT GetModKeys(DWORD dwHotKey);

#endif // !defined(AFX_HOTKEY_H__82523B98_0E49_4B2A_8206_A4262D29C3C8__INCLUDED_)