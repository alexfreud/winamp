/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "main.h"



HWND hFSMonitorWindow;
extern "C"
{
	int g_fsapp = 0;
	int g_restoreaot_timer_set = 0;
	int g_dropaot_timer_set = 0;
}
#define APPBAR_CALLBACK 	WM_USER + 1010
#define appbartag L"wa_fsmonitorclass"
LRESULT CALLBACK fsMonitorWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void BeginFullscreenAppMonitor()
{
	// this lets us receive appbar messages.
	// we're interested in ABN_FULLSCREENAPP so we can turn AOT off temporarily
	APPBARDATA abd;

	WNDCLASSW wc;
	if (!GetClassInfoW(hMainInstance, appbartag, &wc))
	{
		memset(&wc, 0, sizeof(wc));
		wc.lpfnWndProc = fsMonitorWndProc;
		wc.hInstance = hMainInstance;
		wc.lpszClassName = appbartag;
		wc.style = 0;

		RegisterClassW(&wc);
	}

	hFSMonitorWindow = CreateWindowExW(0, appbartag, L"", 0, 0, 0, 1, 1, NULL, NULL, hMainInstance, NULL);

	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = hFSMonitorWindow;
	abd.uCallbackMessage = APPBAR_CALLBACK;
	abd.uEdge = ABE_TOP;
	memset(&abd.rc, 0, sizeof(RECT));

	SHAppBarMessage(ABM_NEW, &abd);
}

void EndFullscreenAppMonitor()
{
	APPBARDATA abd;

	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = hFSMonitorWindow;

	SHAppBarMessage(ABM_REMOVE, &abd);

	if (IsWindow(hFSMonitorWindow))
		DestroyWindow(hFSMonitorWindow);
}


void OnFullscreenApp()
{
	// ignore this event if the window going fullscreen is a winamp window
	if (!is_fullscreen_video && !vis_fullscreen)
	{
		if (g_restoreaot_timer_set)
		{
			g_restoreaot_timer_set = 0;
			KillTimer(hMainWindow, 100);
		}
		else
		{
			g_dropaot_timer_set = 1;
			SetTimer(hMainWindow, 99, 250, NULL);
		}
	}
}

void OnCancelFullscreenApp()
{
	if (!is_fullscreen_video && !vis_fullscreen)
	{
		if (g_dropaot_timer_set)
		{
			KillTimer(hMainWindow, 99);
			g_dropaot_timer_set = 0;
		}
		else
		{
			SetTimer(hMainWindow, 100, 250, NULL);
			g_restoreaot_timer_set = 1;
		}
	}
}

LRESULT CALLBACK fsMonitorWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case APPBAR_CALLBACK:
		switch (wParam)
		{
		case ABN_FULLSCREENAPP:
			if (lParam && !g_fsapp)
			{
				g_fsapp = 1;
				OnFullscreenApp();
			}
			else if (!lParam && g_fsapp)
			{
				g_fsapp = 0;
				OnCancelFullscreenApp();
			}
		}
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



void dropAOT()
{
	if (!config_dropaotfs) return ;
	set_aot(0);
	if (config_aot)
	{
		SetWindowPos(hMainWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		SetWindowPos(hMainWindow, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}

void restoreAOT()
{
	if (!config_dropaotfs) return ;
	if (config_aot)
	{
		// normally, changing hMainWindow's ONTOP flag should be enough to change all the owned window' z-orders too,
		// but for some reason I cannot figure out, it does not work unless WA has been clicked away and back in focus.
		// if that hasn't been done, then reseting the flag on each window is necessary.
		// now for the fun part: the above is true for classic skin, but in modern skins, reseting the flag
		// on those windows actually prevents the player from coming back ONTOP! FUN!
		if (GetParent(hPLWindow) == NULL)
		{
			SetWindowPos(hPLWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
			SetWindowPos(hVideoWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
			SetWindowPos(hEQWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
		}
		SetWindowPos(hMainWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		SetWindowPos(hMainWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	set_aot(0);
}
