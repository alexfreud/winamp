#include "HotKeyCtl.h"
#include "gen_hotkeys.h"

struct HKWND_DATA
{
	WNDPROC lpfnEditWndProc;
	DWORD dwScanCode;
	WORD wMod;
	DWORD dwHotKey;
};

LRESULT CALLBACK HotKeyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#if(_WIN32_WINNT < 0x0500)
#define VK_BROWSER_BACK        0xA6
#define VK_BROWSER_FORWARD     0xA7
#define VK_BROWSER_REFRESH     0xA8
#define VK_BROWSER_STOP        0xA9
#define VK_BROWSER_SEARCH      0xAA
#define VK_BROWSER_FAVORITES   0xAB
#define VK_BROWSER_HOME        0xAC

#define VK_VOLUME_MUTE         0xAD
#define VK_VOLUME_DOWN         0xAE
#define VK_VOLUME_UP           0xAF
#define VK_MEDIA_NEXT_TRACK    0xB0
#define VK_MEDIA_PREV_TRACK    0xB1
#define VK_MEDIA_STOP          0xB2
#define VK_MEDIA_PLAY_PAUSE    0xB3
#define VK_LAUNCH_MAIL         0xB4
#define VK_LAUNCH_MEDIA_SELECT 0xB5
#define VK_LAUNCH_APP1         0xB6
#define VK_LAUNCH_APP2         0xB7
#endif

#ifndef VK_SLEEP
#define VK_SLEEP 0x5F
#endif

UINT wmHKCtlSet;
UINT wmHKCtlGet;

int SubclassEditBox(HWND hwEdit)
{
	if (!IsWindow(hwEdit))
		return 0;

	if (!wmHKCtlSet)
		wmHKCtlSet = RegisterWindowMessage("gen_hotkeys HotKeyCTL set");

	if (!wmHKCtlGet)
		wmHKCtlGet = RegisterWindowMessage("gen_hotkeys HotKeyCTL get");

	HKWND_DATA *hkwnd_data = new HKWND_DATA; 
	if (!hkwnd_data)
		return 0;

	memset(hkwnd_data, 0,  sizeof(HKWND_DATA));

	hkwnd_data->lpfnEditWndProc = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(hwEdit, GWLP_WNDPROC, (LONGX86)(LONG_PTR)HotKeyWndProc);

	SetWindowLongPtr(hwEdit, GWLP_USERDATA, (LONGX86)(LONG_PTR) hkwnd_data);

	return 1;
}

void HotKeySetText(HWND hwHK, HKWND_DATA *hkwnd_data)
{
	if (!IsWindow(hwHK) || !hkwnd_data)
		return;

	wchar_t szKeyName[1024] = L"";
	wchar_t *p = szKeyName;
	DWORD dwSize = sizeof(szKeyName);
	DWORD dwLen = 0;
	WORD wMod;

	if (hkwnd_data->dwHotKey)
		wMod = HIBYTE(hkwnd_data->dwHotKey);
	else
		wMod = hkwnd_data->wMod;

	if(wMod & HOTKEYF_WIN) {
		// GetKeyNameText gives us Left/Right Windows but RegisterHotKey doesn't seperate the two
		//GetKeyNameText(MAKELPARAM(0, MapVirtualKey(VK_LWIN, 0)) | (1 << 24), p, dwSize);
		// so we use just "Winkey" to avoid confusion
		WASABI_API_LNGSTRINGW_BUF(IDS_GHK_WINKEY_STR,p,dwSize);
		dwLen = lstrlenW(szKeyName);
		StringCchCatW(p, dwSize, L" + ");
		dwSize -= dwLen + 3;
		p = szKeyName + dwLen + 3;
	}
	if(wMod & HOTKEYF_CONTROL) {
		GetKeyNameTextW(MAKELONG(0, MapVirtualKey(VK_CONTROL, 0)), p, dwSize);
		dwLen = lstrlenW(szKeyName);
		StringCchCatW(p, dwSize, L" + ");
		dwSize -= dwLen + 3;
		p = szKeyName + dwLen + 3;
	}
	if(wMod & HOTKEYF_SHIFT) {
		GetKeyNameTextW(MAKELONG(0, MapVirtualKey(VK_SHIFT, 0)), p, dwSize);
		dwLen = lstrlenW(szKeyName);
		StringCchCatW(p, dwSize, L" + ");
		dwSize -= dwLen + 3;
		p = szKeyName + dwLen + 3;
	}
	if(wMod & HOTKEYF_ALT) {
		GetKeyNameTextW(MAKELONG(0, MapVirtualKey(VK_MENU, 0)), p, dwSize);
		dwLen = lstrlenW(szKeyName);
		StringCchCatW(p, dwSize, L" + ");
		dwSize -= dwLen + 3;
		p = szKeyName + dwLen + 3;
	}

	if(hkwnd_data->dwHotKey)
	{
		switch (LOBYTE(hkwnd_data->dwHotKey))
		{
			case VK_BROWSER_BACK:
				WASABI_API_LNGSTRINGW_BUF(IDS_BROWSER_BACK,p,dwSize);
			break;
			case VK_BROWSER_FORWARD:
				WASABI_API_LNGSTRINGW_BUF(IDS_BROWSER_FORWARD,p,dwSize);
			break;
			case VK_BROWSER_REFRESH:
				WASABI_API_LNGSTRINGW_BUF(IDS_BROWSER_REFRESH,p,dwSize);
			break;
			case VK_BROWSER_STOP:
				WASABI_API_LNGSTRINGW_BUF(IDS_BROWSER_STOP,p,dwSize);
			break;
			case VK_BROWSER_SEARCH:
				WASABI_API_LNGSTRINGW_BUF(IDS_BROWSER_SEARCH,p,dwSize);
			break;
			case VK_BROWSER_FAVORITES:
				WASABI_API_LNGSTRINGW_BUF(IDS_BROWSER_FAVOURITES,p,dwSize);
			break;
			case VK_BROWSER_HOME:
				WASABI_API_LNGSTRINGW_BUF(IDS_BROWSER_HOME,p,dwSize);
			break;
			case VK_VOLUME_MUTE:
				WASABI_API_LNGSTRINGW_BUF(IDS_VOLUME_MUTE,p,dwSize);
			break;
			case VK_VOLUME_DOWN:
				WASABI_API_LNGSTRINGW_BUF(IDS_VOLUME_DOWN,p,dwSize);
			break;
			case VK_VOLUME_UP:
				WASABI_API_LNGSTRINGW_BUF(IDS_VOLUME_UP,p,dwSize);
			break;
			case VK_MEDIA_NEXT_TRACK:
				WASABI_API_LNGSTRINGW_BUF(IDS_NEXT_TRACK,p,dwSize);
			break;
			case VK_MEDIA_PREV_TRACK:
				WASABI_API_LNGSTRINGW_BUF(IDS_PREVIOUS_TRACK,p,dwSize);
			break;
			case VK_MEDIA_STOP:
				WASABI_API_LNGSTRINGW_BUF(IDS_STOP,p,dwSize);
			break;
			case VK_MEDIA_PLAY_PAUSE:
				WASABI_API_LNGSTRINGW_BUF(IDS_PLAY_PAUSE,p,dwSize);
			break;
			case VK_LAUNCH_MAIL:
				WASABI_API_LNGSTRINGW_BUF(IDS_LAUNCH_MAIL,p,dwSize);
			break;
			case VK_LAUNCH_MEDIA_SELECT:
				WASABI_API_LNGSTRINGW_BUF(IDS_LAUNCH_MEDIA_SELECT,p,dwSize);
			break;
			case VK_LAUNCH_APP1:
				WASABI_API_LNGSTRINGW_BUF(IDS_LAUCH_APP1,p,dwSize);
			break;
			case VK_LAUNCH_APP2:
				WASABI_API_LNGSTRINGW_BUF(IDS_LAUCH_APP2,p,dwSize);
			break;
			case VK_SLEEP:
				WASABI_API_LNGSTRINGW_BUF(IDS_SLEEP,p,dwSize);
			break;
			case VK_PAUSE:
				WASABI_API_LNGSTRINGW_BUF(IDS_PAUSE,p,dwSize);
			break;
			default:
				GetKeyNameTextW(hkwnd_data->dwScanCode, p, dwSize);
				if (!*p)
					StringCchPrintfW(p, 1024, L"0x%X", hkwnd_data->dwScanCode);
			break;
		}
	}
	else
		szKeyName[dwLen] = 0;

	SetWindowTextW(hwHK, szKeyName);

	SendMessageW(hwHK, EM_SETSEL, lstrlenW(szKeyName), lstrlenW(szKeyName));
}

LRESULT CALLBACK HotKeyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HKWND_DATA *hkwnd_data = (HKWND_DATA *)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	HKWND_DATA hkwnd_data_local;

	if (uMsg == wmHKCtlSet)
	{
		int extflag = 0;
		if (HIBYTE(wParam) & HOTKEYF_EXT)
			extflag = (1 << 24);

		hkwnd_data->dwHotKey = (unsigned long)wParam;
		hkwnd_data->dwScanCode = MAKELPARAM(0, MapVirtualKey(LOBYTE(wParam), 0)) | extflag;
		//hkwnd_data->wMod = HIBYTE(wParam);
		HotKeySetText(hwnd, hkwnd_data);
		return 0;
	}
	else if (uMsg == wmHKCtlGet)
	{
		return hkwnd_data->dwHotKey;
	}

	switch (uMsg)
	{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			hkwnd_data->dwHotKey = 0;

			switch (wParam)
			{
				case VK_SHIFT:
					hkwnd_data->wMod |= HOTKEYF_SHIFT;
				break;
				case VK_CONTROL:
					hkwnd_data->wMod |= HOTKEYF_CONTROL;
				break;
				case VK_MENU:
					hkwnd_data->wMod |= HOTKEYF_ALT;
				break;
				case VK_LWIN:
				case VK_RWIN:
					hkwnd_data->wMod |= HOTKEYF_WIN;
				break;
				default:
					hkwnd_data->dwScanCode = (unsigned long)lParam;
					if (lParam & (1 << 24)) // extended bit
						hkwnd_data->wMod |= HOTKEYF_EXT;
					else
						hkwnd_data->wMod &= ~HOTKEYF_EXT;
					hkwnd_data->dwHotKey = MAKEWORD(wParam, hkwnd_data->wMod);
				break;
			}

			HotKeySetText(hwnd, hkwnd_data);
			return 1;
		}

		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			switch (wParam)
			{
				case VK_SHIFT:
					hkwnd_data->wMod &= ~HOTKEYF_SHIFT;
				break;
				case VK_CONTROL:
					hkwnd_data->wMod &= ~HOTKEYF_CONTROL;
				break;
				case VK_MENU:
					hkwnd_data->wMod &= ~HOTKEYF_ALT;
				break;
				case VK_LWIN:
				case VK_RWIN:
					hkwnd_data->wMod &= ~HOTKEYF_WIN;
				break;
			}

			HotKeySetText(hwnd, hkwnd_data);
			return 1;
		}

		case WM_CHAR:
		case WM_PASTE:
			return 0;

		case WM_DESTROY:
			if (hkwnd_data)
			{
				SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR) hkwnd_data->lpfnEditWndProc);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
				hkwnd_data_local = *hkwnd_data;
				delete hkwnd_data;
				hkwnd_data = &hkwnd_data_local;
			}
		break;
	}

	return CallWindowProc(hkwnd_data->lpfnEditWndProc, hwnd, uMsg, wParam, lParam);
}