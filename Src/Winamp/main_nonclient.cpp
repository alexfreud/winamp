/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
UINT Main_OnNCHitTest(HWND hwnd, int x, int y)
{
	return HTCLIENT;
}

BOOL Main_OnNCActivate(HWND hwnd, BOOL fActive, HWND hwndActDeact, BOOL fMinimized)
{
	if (fActive == FALSE)
	{
		draw_tbar(config_hilite ? 0 : 1, config_windowshade, eggstat);
		if (config_windowshade) SendMessageW(hwnd, WM_TIMER, UPDATE_DISPLAY_TIMER + 4, 0);
	}
	else
	{
		static int i;
		if (i) draw_tbar(1, config_windowshade, eggstat);
		i = 1;
		if (config_windowshade) SendMessageW(hwnd, WM_TIMER, UPDATE_DISPLAY_TIMER + 4, 0);
	}

	return TRUE;
}

UINT Main_OnNCCalcSize(HWND hwnd, BOOL fCalcValidRects, NCCALCSIZE_PARAMS * lpcsp)
{
	/*if (fCalcValidRects)
	{}
	else
	{}*/
	return WVR_ALIGNTOP | WVR_ALIGNBOTTOM | WVR_ALIGNRIGHT | WVR_ALIGNLEFT;
}

