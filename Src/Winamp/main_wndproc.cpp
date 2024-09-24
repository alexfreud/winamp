/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "main.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h" 
#include "../nu/AutoWideFn.h" 
#include "./api.h" 
#include "resource.h"
#include "wintheme.h"

// WM_CREATE handler
LRESULT Main_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	//	srand(GetTickCount());
	hMainWindow = hwnd;
	DisableVistaPreview();

	HACCEL hAccel = LoadAcceleratorsW(language_pack_instance, MAKEINTRESOURCEW(IDR_ACCELERATOR_GLOBAL));
	if (!hAccel && language_pack_instance != hMainInstance) hAccel = LoadAcceleratorsW(hMainInstance, MAKEINTRESOURCEW(IDR_ACCELERATOR_GLOBAL));
	if (hAccel) WASABI_API_APP->app_addAccelerators(hwnd, &hAccel, 1, TRANSLATE_MODE_GLOBAL);

	hAccel = LoadAcceleratorsW(language_pack_instance, MAKEINTRESOURCEW(IDR_ACCELERATOR_MAIN));
	if (!hAccel && language_pack_instance != hMainInstance) hAccel = LoadAcceleratorsW(hMainInstance, MAKEINTRESOURCEW(IDR_ACCELERATOR_MAIN));
	if (hAccel) WASABI_API_APP->app_addAccelerators(hwnd, &hAccel, 1, TRANSLATE_MODE_NORMAL);

	WASABI_API_APP->app_registerGlobalWindow(hwnd);

	return 1;
} // Main_OnCreate()

// Message sent from system notification area
LRESULT Main_OnWASystray(HWND hwnd, int id)
{
	POINT p;
	switch (id)
	{
	case WM_LBUTTONUP:
		if (GetAsyncKeyState(VK_CONTROL) & (1 << 15))
			SendMessageW(hwnd, WM_COMMAND, WINAMP_FILE_LOC, 0);
		else if (GetAsyncKeyState(VK_SHIFT) & (1 << 15))
			SendMessageW(hwnd, WM_COMMAND, WINAMP_FILE_PLAY, 0);
		else if (!IsWindowVisible(hwnd))
		{
			ShowWindow(hwnd, SW_RESTORE);
			ShowWindow(hwnd, SW_SHOW);
		}
		else SetForegroundWindow(hwnd);

		break;
	case WM_RBUTTONUP:
		GetCursorPos(&p);
		SetForegroundWindow(hwnd);
		//DoTrackPopup(main_menu, TPM_RIGHTBUTTON, p.x, p.y, hwnd);
		TrackPopupMenu(main_menu, TPM_RIGHTBUTTON, p.x, p.y, 0, hwnd, NULL);
		break;
	}
	return 1;
}


// WM_SIZE handler, to detect minizations.
LRESULT Main_OnSize(HWND hwnd, UINT state, int cx, int cy)
{

	if (state == SIZE_MINIMIZED)
	{
		int q = GetAsyncKeyState(VK_SHIFT) >> 15;
		if (config_taskbar || q || minimize_hack_winamp) ShowWindow(hwnd, SW_HIDE);
		if (q || minimize_hack_winamp)
		{
			systray_minimize(caption);
			config_minimized = 2;
		}
		else config_minimized = 1;
		if (config_pe_open && hPLWindow){
			HWND parent = GetParent(GetParent(hPLWindow));
			if(IsWindow(parent) && IsWindowVisible(hPLWindow)){}
			else ShowWindow(hPLWindow,SW_HIDE);
		}
		//if (config_mb_open&&hMBWindow) ShowWindow(hMBWindow,SW_HIDE);
		if (config_eq_open && hEQWindow) ShowWindow(hEQWindow,SW_HIDE);
		minimize_hack_winamp = 0;
	}
	else if (state == SIZE_RESTORED)
	{

		if (config_minimized == 2 && !(config_taskbar == 1 || config_taskbar == 2))
		{
			systray_restore();
		}
		if (g_taskbar_dirty)
		{
			g_taskbar_dirty = 0;
			set_taskbar();
		}
		if (!IsWindowVisible(hwnd) && !deferring_show)	ShowWindow(hwnd, SW_SHOWNA);
		if (config_minimized) set_aot(0);
		if (!config_mw_open) MoveOffscreen(hwnd);
		config_minimized = 0;
		SendMessageW(hwnd, WM_TIMER, UPDATE_DISPLAY_TIMER + 4, 0);
		if (!deferring_show) SetForegroundWindow(hwnd);
		if (config_pe_open && hPLWindow) ShowWindow(hPLWindow, SW_SHOWNA);
		//if (config_mb_open&&hMBWindow) ShowWindow(hMBWindow,SW_SHOWNA);
		if (config_eq_open && hEQWindow) ShowWindow(hEQWindow,SW_SHOWNA);
		if (config_video_open && hVideoWindow) ShowWindow(hVideoWindow, SW_SHOWNA);
		set_aot(0);
//		Browser_toggleVisible(1);
	}
	return 1;
}


// Sent by decode thread to let the main program know it's done
LRESULT Main_OnWAMPEGEOF(HWND hwnd)
{
	return PlayQueue_OnEOF();
}

// Drag&Drop handler
LRESULT Main_OnDropFiles(HWND hwnd, HDROP hdrop)
{
	wchar_t temp[MAX_PATH] = {0};
	int s = (GetAsyncKeyState(VK_SHIFT) & (1 << 15)) ? 0 : 1;
	int y = DragQueryFileW(hdrop, 0xffffffff, temp, MAX_PATH);
	if (s)
	{
		PlayList_delete();
		PlayList_randpos(0);
	}
	for (int x = 0; x < y; x ++)
	{
		DragQueryFileW(hdrop, x, temp, MAX_PATH);

		PlayList_appendthing(temp, 0, 0);
	}
	if (s)
		BeginPlayback();

	plEditRefresh();

	return 1;
}

LRESULT Main_OnCopyData(HWND sendingHWND, COPYDATASTRUCT *cds)
{
	switch (cds->dwData)
	{
		case IPC_SETSKIN:
			if (cds->lpData && lstrlenA((char *)cds->lpData))
			{
				AutoWide dataW((char*)cds->lpData);
				if (_wcsicmp(config_skin, dataW))
				{
					StringCchCopyW(config_skin, MAX_PATH, dataW);
				}
				SendMessageW(hMainWindow, WM_COMMAND, WINAMP_REFRESHSKIN, 0);
			}
			return TRUE;
		case IPC_SETSKINW:
			if (cds->lpData && lstrlenW((wchar_t*)cds->lpData))
			{
				if (_wcsicmp(config_skin, (wchar_t*)cds->lpData))
				{
					StringCchCopyW(config_skin, MAX_PATH, (wchar_t*)cds->lpData);
				}
				SendMessageW(hMainWindow, WM_COMMAND, WINAMP_REFRESHSKIN, 0);
			}
			return TRUE;
		case IPC_CHDIR:
			SetCurrentDirectoryA((char *) cds->lpData);
			return TRUE;
		case IPC_PLAYFILE:
			{
				char *filename = (char *)cds->lpData;
				PlayList_appendthing(AutoWideFn(filename), 0, 0);
				plEditRefresh();
			}
			return TRUE;
		case IPC_PLAYFILEW:
			{
				wchar_t *filename = (wchar_t *)cds->lpData;
				PlayList_appendthing(filename, 0, 0);
				plEditRefresh();
			}
			return TRUE;
		case IPC_GETMODULENAME:
			{
				char b[512] = {0}, b2[512] = {0};
				GetModuleFileNameA(hMainInstance, b, sizeof(b));
				GetShortPathNameA(b, b2, 511);
				GetShortPathNameA((char *)cds->lpData, b, 511);
				stat_isit = _stricmp(b, b2);
			}
			return TRUE;
		case IPC_ADDBOOKMARK:
			{
				wchar_t* file = AutoWide((char *)cds->lpData);
				Bookmark_additem(file, PlayList_gettitle(file, 1));
			}
			return TRUE;
		case IPC_ADDBOOKMARKW:
			{
				Bookmark_additem((wchar_t *)cds->lpData, PlayList_gettitle((const wchar_t *)cds->lpData, 1));
			}
			return TRUE;
		case IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE:
			{
				wchar_t *filename = (wchar_t *)cds->lpData;
				wchar_t *tagname = filename + wcslen(filename) + 1;

				extendedFileInfoStructW info;
				info.filename = filename;
				info.metadata = tagname;
				wchar_t ret[1024]=L"";
				info.ret = ret;
				info.retlen = 1024;
				
				if (0 == SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE))
					ret[0] = L'\0';

				COPYDATASTRUCT answer;
				answer.lpData = info.ret;
				answer.cbData = sizeof(ret);
				answer.dwData = IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE;
				return SendMessageW(sendingHWND, WM_COPYDATA, (WPARAM)hMainWindow, (LPARAM)&answer);
			}
		case IPC_GET_PLAYING_FILENAME:
			{
				COPYDATASTRUCT answer;
				answer.lpData = FileName;
				answer.cbData = (DWORD)(sizeof(wchar_t) * (wcslen(FileName) + 1));
				answer.dwData = IPC_GET_PLAYING_FILENAME;
				return SendMessageW(sendingHWND, WM_COPYDATA, (WPARAM)hMainWindow, (LPARAM)&answer);
			}
		case IPC_OPEN_URL:
			{
				myOpenURL(hMainWindow, (wchar_t *)cds->lpData);
				return TRUE;
			}
		case IPC_HANDLE_URI:
			{
				HandleFilename((wchar_t *)cds->lpData);
				return TRUE;
			}
	}
	return FALSE;
}