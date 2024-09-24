/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "wa_dlg.h"
#include "../nu/AutoChar.h"
#include "resource.h"

extern "C" extern int g_skinloadedmanually;

int Main_OnGetText(wchar_t *text, int sizeCch)
{
	size_t rem;
	StringCchCopyExW(text, sizeCch, caption, 0, &rem, 0);
	return (int)sizeCch-rem;

}
// evil 256 color mode windows palette handling
int Main_OnQueryNewPalette(HWND hwnd)
{
	if (draw_hpal) { // hPal is NULL if we're in hicolor
		HDC hdc = GetWindowDC(hwnd);
		SelectPalette(hdc,draw_hpal,FALSE);
		RealizePalette(hdc);
		InvalidateRect(hwnd,NULL,FALSE);
		ReleaseDC(hwnd,hdc);
	}
	return 1;
}

// more 256 color windows palette handling
int Main_OnPaletteChanged(HWND hwnd, HWND hwndPaletteChange)
{
	if (draw_hpal)
	{
		HDC hdc = GetWindowDC(hwnd);
		SelectPalette(hdc,draw_hpal,FALSE);
		RealizePalette(hdc);
		UpdateColors(hdc);
		ReleaseDC(hwnd,hdc);
	}
	return 1;
}

// displaychange handling. This reinitializes bitmaps, to use optimal storage format
// depending on mode (DIB or DDB)
int Main_OnDisplayChange(HWND hwnd)
{
	int t=0;
	if (g_skinloadedmanually)
	{
		Skin_Load();
	}

	if (g_skinmissinggenff)
	{
		wchar_t msg[512] = {0};
		StringCchPrintfW(msg, 512, getStringW(IDS_NO_MODERN_SKIN_SUPPORT, NULL, 0), config_skin);
		MessageBoxW(NULL, msg, getStringW(IDS_SKIN_LOAD_ERROR, NULL, 0), MB_ICONWARNING | MB_OK | MB_TOPMOST);
	}

	draw_setnoupdate(1);
	draw_init();
	draw_clutterbar(0);
	draw_shuffle(config_shuffle,0);
	draw_eject(0);
	draw_eqplbut(config_eq_open,0,config_pe_open,0);
	draw_repeat(config_repeat,0);
	draw_buttonbar(-1);
	draw_volumebar(config_volume,0);
	draw_panbar(config_pan,0);
	draw_songname(L"",&t,0);
	draw_playicon(playing?paused?4:1:2);
	draw_tbar(config_hilite?(GetForegroundWindow() == hMainWindow?1:0):1, config_windowshade,eggstat);

	draw_monostereo(-1);
	if (playing)
	{
		draw_bitmixrate(-1,-1);
	}

	draw_setnoupdate(0);
	draw_songname(FileTitle,&ui_songposition,playing?in_getlength():PlayList_getcurrentlength());

	// tell other windows
	if (IsWindow(hVideoWindow))
		SendMessageW(hVideoWindow, WM_DISPLAYCHANGE,0,0);
	if (IsWindow(hEQWindow))
		SendMessageW(hEQWindow, WM_DISPLAYCHANGE,0,0);
	if (IsWindow(hPLWindow))
		SendMessageW(hPLWindow, WM_DISPLAYCHANGE,0,0);
	/*
	if (IsWindow(hMBWindow))
		SendMessageW(hVideoWindow, WM_DISPLAYCHANGE,0,0);
	*/
	if (IsWindow(hExternalVisWindow))
		SendMessageW(hExternalVisWindow, WM_DISPLAYCHANGE,0,0);

	SetTimer(hMainWindow, 101, 250, NULL); // set_aot(-1);

	InvalidateRect(hwnd,NULL,FALSE);
	//UpdateWindow(hwnd);

	EnterCriticalSection(&embedcs);
	{
		embedWindowState *p =embedwndlist;  
		while (p)
		{
			PostMessageW(p->me,WM_DISPLAYCHANGE,0,0);
			p=p->link;
		}
	}

	LeaveCriticalSection(&embedcs);

	PostMessageW(hMainWindow, WM_WA_IPC, 0, IPC_SKIN_CHANGED);
	return 1;
}
