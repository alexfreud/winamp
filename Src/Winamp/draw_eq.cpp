/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "draw.h"
#include "plush/plush.h"
#include "WinampAttributes.h"
#include "resource.h"
#include "WADrawDC.h"

HDC eqMainDC, eqMainDC2;
int enable_eq_windowshade_button;
HBITMAP eqMainBM = NULL, eqMainBM2 = NULL, eqExBM = NULL, eqOldmainBM2 = NULL, eqOldmainBM = NULL;

extern "C" int eq_init = 0;
void draw_eq_init()
{
	EnterCriticalSection(&g_srcdccs);
	if (eq_init)
		draw_eq_kill();
	
	eq_init=1;
	HDC screenHdc = draw_GetWindowDC(hMainWindow);

	eqMainDC = CreateCompatibleDC(screenHdc);
	eqMainDC2 = CreateCompatibleDC(screenHdc);

	enable_eq_windowshade_button = 2;

	// attempt to use the ISO eq image (if present)
	if(config_eq_frequencies!=EQ_FREQUENCIES_WINAMP)
		eqMainBM = draw_LBitmap(NULL, L"eqmain_iso.bmp");
	// otherwise we revert to the normal eq image
	if (!eqMainBM)
		eqMainBM = draw_LBitmap(NULL, L"eqmain.bmp");
	if (eqMainBM)
		enable_eq_windowshade_button = 0;
	// and if that fails then we revert to the built in classic skin resources
	else
		eqMainBM = draw_LBitmap(MAKEINTRESOURCE((config_eq_frequencies==EQ_FREQUENCIES_WINAMP)?IDB_EQMAIN:IDB_EQMAIN_ISO), NULL);
	eqOldmainBM = (HBITMAP)SelectObject(eqMainDC, eqMainBM);

	eqExBM = draw_LBitmap(NULL, L"eq_ex.bmp");
	if (!eqExBM)
	{
		if (!skin_directory[0]) 
			enable_eq_windowshade_button = 1;
		eqExBM = draw_LBitmap(MAKEINTRESOURCE(IDB_EQEX), NULL);
	}
	else 
		enable_eq_windowshade_button = 1;

	draw_ReleaseDC(hMainWindow, screenHdc);
		
	int x;
	draw_eq_slid(0,config_preamp,0);
	for (x = 1; x <= 10; x ++)
		draw_eq_slid(x,eq_tab[x-1],0);
	draw_eq_graphthingy();
	draw_eq_onauto(config_use_eq, config_autoload_eq, 0,0);
	draw_eq_tbar(GetForegroundWindow()==hEQWindow?1:(config_hilite?0:1));
	draw_eq_presets(0);
	
	LeaveCriticalSection(&g_srcdccs);
}

void draw_eq_kill()
{
	if (!eq_init)
		return ;
	EnterCriticalSection(&g_srcdccs);
	SelectObject(eqMainDC, eqOldmainBM);
	DeleteObject(eqMainBM);
	eqMainBM = NULL;

	if (eqMainBM2)
	{
		SelectObject(eqMainDC2, eqOldmainBM2);
		DeleteObject(eqMainBM2);
		eqMainBM2 = NULL;
	}
	
	DeleteDC(eqMainDC);
	DeleteDC(eqMainDC2);

	if (eqExBM)
		DeleteObject(eqExBM); 
	eqExBM=NULL;
	LeaveCriticalSection(&g_srcdccs);
}

static void update_area_eq(int x1, int y1, int w, int h)
{
	if (updateen && hEQWindow)
	{
		WADrawDC tDC(hEQWindow);
		if (tDC && hEQWindow)
		{
			do_palmode(tDC);
			if (!(config_dsize && config_eqdsize))
			{
				BitBlt(tDC, x1, y1, w, h, eqMainDC, x1, y1, SRCCOPY);
				if (eqMainBM2)
				{
					SelectObject(eqMainDC2, eqOldmainBM2);
					DeleteObject(eqMainBM2);
					eqMainBM2 = NULL;
				}
			}
			else
			{
				if (!eqMainBM2)
				{
					eqMainBM2 = CreateCompatibleBitmap(mainDC, WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2);
					eqOldmainBM2 = (HBITMAP)SelectObject(eqMainDC2, eqMainBM2);
					x1 = y1 = 0;
					w = WINDOW_WIDTH;
					h = WINDOW_HEIGHT;
				}
				StretchBlt(eqMainDC2, x1*2, y1*2, w*2, h*2, eqMainDC, x1, y1, w, h, SRCCOPY);
				BitBlt(tDC, x1*2, y1*2, w*2, h*2, eqMainDC2, x1*2, y1*2, SRCCOPY);
			}
		}
	}
}

void draw_eq_presets(int pressed)
{
	int top = 18, left = 217;
	int w = 44;
	int h = 12;
	BitBlt(eqMainDC, left, top, w, h, eqMainDC, 224, pressed ? 176 : 164, SRCCOPY);
	update_area_eq(left, top, w, h);
}

void draw_eq_tbar(int active)
{
	int l = active ? 134 : 149;
	if (!eq_init) return ;
	if (config_eq_ws)
	{
		int xo = 63;
		int p = 94;
		int r;
		int xx = config_volume * 3 / 256;
		setSrcBM(eqExBM);
		BitBlt(eqMainDC, 0, 0, WINDOW_WIDTH, 14, bmDC, 0, active ? 0 : 15, SRCCOPY);
		r = xo + (p * config_volume) / 255;
		BitBlt(eqMainDC, r - 2, 4, 3, 7, bmDC, xx*3 + 1, 30, SRCCOPY);
		r = 166 + (39 * (config_pan + 128)) / 255;
		xx = (config_pan + 128) * 3 / 256;
		BitBlt(eqMainDC, r - 2, 4, 3, 7, bmDC, xx*3 + 11, 30, SRCCOPY);
		unsetSrcBM();
	}
	else
	{
		BitBlt(eqMainDC, 0, 0, WINDOW_WIDTH, 14, eqMainDC, 0, l, SRCCOPY);
	}
	update_area_eq(0, 0, WINDOW_WIDTH, 14);
}

void draw_eq_slid(int which, int pos, int pressed) // left to right, 0-64
{
	int top = 38, h = 63;
	int num_pos = 63 - 11;
	int w = 14;
	int xp;
	int n = 0;
	if (!which)
		xp = 21;
	else xp = 78 + (96 - 78) * (which - 1);
	if (!eq_init) return ;
	n = 27 - ((pos * 28) / 64);
	if (n < 14)
		BitBlt(eqMainDC, xp, top, w, h, eqMainDC, 13 + n*15, 164, SRCCOPY);
	else
		BitBlt(eqMainDC, xp, top, w, h, eqMainDC, 13 + (n - 14)*15, 229, SRCCOPY);
	BitBlt(eqMainDC, xp + 1, top + h - 12 - ((63 - pos)*num_pos) / 64, 11, 11, eqMainDC, 0, pressed ? 176 : 164, SRCCOPY);
	update_area_eq(xp, top, w, h);
}

void draw_eq_onauto(int on, int autoon, int onpressed, int autopressed)
{
	int top = 18, left = 14;
	int w1 = 25, w2 = 33;
	int h = 12;
	BitBlt(eqMainDC, left, top, w1, h, eqMainDC, 10 + (onpressed ? 118 : 0) + (on ? 59 : 0), 119, SRCCOPY);
	BitBlt(eqMainDC, left + w1, top, w2, h, eqMainDC, 35 + (autopressed ? 118 : 0) + (autoon ? 59 : 0), 119, SRCCOPY);
	update_area_eq(left, top, w1 + w2, h);
}

void draw_eq_graphthingy()
{
	int top = 17, left = 86;
	int src_top = 294;
	int w = 113, h = 19;
	float keys[12] = {0};
	pl_Spline spline = {keys, 1, 12, 0.0f, 0.0f, 0.1f};
	BitBlt(eqMainDC, left, top, w, h, eqMainDC, 0, src_top, SRCCOPY);
	BitBlt(eqMainDC, left, top - 1 + h - (int)(config_preamp*19.0f / 64.0f), w, 1, eqMainDC, 0, 314, SRCCOPY);
	{
		int x;
		int last_p = -1;
		for (x = 0; x < 10; x ++)
			keys[x + 1] = eq_tab[x] * 19.0f / 64.0f;
		keys[0] = keys[1];
		keys[11] = keys[10];

		for (x = 0; x < 109; x ++)
		{
			float p;
			int this_p;
			int lin_offs = 115;
			plSplineGetPoint(&spline, 1.0f + x / 12.0f, &p);
			this_p = (int)p;
			if (this_p < 0) this_p = 0;
			if (this_p > 18) this_p = 18;
			if (last_p == -1 || this_p == last_p)
				BitBlt(eqMainDC, left + 2 + x, top + this_p, 1, 1, eqMainDC, lin_offs, src_top + this_p, SRCCOPY);
			else
			{
				if (this_p < last_p)
					BitBlt(eqMainDC, left + 2 + x, top + this_p, 1, last_p - this_p + 1, eqMainDC, lin_offs, src_top + this_p, SRCCOPY);
				else if (this_p > last_p)
					BitBlt(eqMainDC, left + 2 + x, top + last_p, 1, this_p - last_p + 1, eqMainDC, lin_offs, src_top + last_p, SRCCOPY);
			}
			last_p = this_p;
		}
	}
	update_area_eq(left, top, w, h);
}

void draw_eq_tbutton(int b3, int wsb)
{
	setSrcBM(eqExBM);
	if (config_eq_ws)
	{
		if (wsb)
			BitBlt(eqMainDC, 254, 3, 9, 9, bmDC, 1, 47, SRCCOPY);
		else
			BitBlt(eqMainDC, 254, 3, 9, 9, bmDC, 254, 3, SRCCOPY);
		BitBlt(eqMainDC, 264, 3, 9, 9, bmDC, 11, 38 + b3*9, SRCCOPY);
	}
	else
	{
		if (wsb && enable_eq_windowshade_button)
			BitBlt(eqMainDC, 254, 3, 9, 9, bmDC, 1, 38, SRCCOPY);
		else
			BitBlt(eqMainDC, 254, 3, 9, 9, eqMainDC, 254, 137, SRCCOPY);
		BitBlt(eqMainDC, 264, 3, 9, 9, eqMainDC, 0, 116 + b3*9, SRCCOPY);
	}
	unsetSrcBM();
	update_area_eq(253, 3, 20, 9);
}

static void draw_paintDC_eq(HDC screenHdc, const RECT &r)
{
	int dsize = (config_dsize && config_eqdsize);

	do_palmode(screenHdc);

	if (!dsize)
	{
		BitBlt(screenHdc, r.left, r.top, r.right - r.left, r.bottom - r.top, eqMainDC, r.left, r.top, SRCCOPY);
		if (eqMainBM2)
		{
			SelectObject(eqMainDC2, eqOldmainBM2);
			DeleteObject(eqMainBM2);
			eqMainBM2 = NULL;
		}
	}
	else
	{
		if (!eqMainBM2)
		{
			eqMainBM2 = CreateCompatibleBitmap(mainDC, WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2);
			eqOldmainBM2 = (HBITMAP)SelectObject(eqMainDC2, eqMainBM2);
			StretchBlt(eqMainDC2, 0, 0, WINDOW_WIDTH*2, WINDOW_HEIGHT*2, eqMainDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SRCCOPY);
		}
		BitBlt(screenHdc, r.left, r.top, r.right - r.left, r.bottom - r.top, eqMainDC2, r.left, r.top, SRCCOPY);
	}

}

void draw_printclient_eq(HDC hdc, LPARAM /*drawingOptions*/)
{
	RECT r;
	GetClientRect(hEQWindow,&r);
	draw_paintDC_eq(hdc, r);
}

void draw_paint_eq(HWND hwnd)
{
	if (hwnd || hEQWindow)
	{
		HDC screenHdc;
		PAINTSTRUCT ps;
		RECT r;

		if (!eq_init) return ;

		if (hwnd)
		{
			GetUpdateRect(hwnd, &ps.rcPaint, 0);
			EnterCriticalSection(&g_mainwndcs);
			screenHdc = BeginPaint(hwnd, &ps);
			memcpy(&r, &ps.rcPaint, sizeof(r));
		}
		else
		{
			screenHdc = draw_GetWindowDC(hEQWindow);
			GetClientRect(hEQWindow, &r);
		}

		draw_paintDC_eq(screenHdc, r);
		
		if (hwnd)
		{
			EndPaint(hwnd, &ps);
			LeaveCriticalSection(&g_mainwndcs);
		}
		else
			draw_ReleaseDC(hEQWindow, screenHdc);
	}
}