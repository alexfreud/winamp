#ifndef NULLSOFT_WINAMP_HOOK_HEADER
#define NULLSOFT_WINAMP_HOOK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <map>

class ifc_winamphook;

class WinampHook
{
protected:
	WinampHook(HWND hwndWinamp);
	~WinampHook();

public:
	static HRESULT CreateInstance(HWND hwndWinamp, WinampHook **instance);

public:
	ULONG AddRef();
	ULONG Release();

	HRESULT RegisterCallback(ifc_winamphook *callback, UINT *cookie);
	HRESULT UnregisterCallback(UINT cookie);

	HWND GetWinamp();
	LRESULT CallPrevWinampProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CallDefWinampProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	HRESULT AttachToWinamp();
	HRESULT DetachFromWinamp();
	LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnWinampDestroy();
	LRESULT OnWinampIPC(UINT commandId, WPARAM param);
	void OnWinampCommand(UINT commandId, UINT controlId, HWND hControl);
	void OnSysColorChange();

protected:
	typedef enum
	{
		flagUnicode = 0x00000001,
	} Flags;

	typedef std::map<UINT, ifc_winamphook*> CallbackMap;

	friend static LRESULT CALLBACK WinampWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	ULONG ref;
	HWND hwnd;
	WNDPROC originalProc;
	UINT flags;
	UINT lastCookie;
	CallbackMap callbackMap;
};

#endif //NULLSOFT_WINAMP_HOOK_HEADER