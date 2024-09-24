/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include <stdio.h>
#include "resource.h"
#include "draw.h"
#include "WADrawDC.h"

//#define DEBUG_DRAW
// time to fix reloading of main.bmp and just save a copy to restore with

COLORREF mfont_bgcolor=RGB(0,0,0), mfont_fgcolor=RGB(0,255,0);
int pe_fontheight=8;
int mfont_height=6;
HFONT font=0, mfont=0, shadefont=0, osdFontText=0;
HBRUSH selbrush, normbrush, mfont_bgbrush;

volatile int draw_initted;
HDC mainDC, bmDC, specDC,mainDC2;
HBITMAP mainBM_save, mainBM,shufflerepeatBM,
			   fontBM, specBM, oldMainBM, oldSpecBM;
extern int sa_kill;
static int palmode;
HPALETTE draw_hpal=0;

#ifdef DEBUG_DRAW
static DWORD main_thread_id;
#endif

CRITICAL_SECTION g_mainwndcs, g_srcdccs;

void draw_firstinit()
{
	InitializeCriticalSection(&g_srcdccs);
	InitializeCriticalSection(&g_mainwndcs);
#ifdef DEBUG_DRAW
	main_thread_id=GetCurrentThreadId();
#endif
}

void draw_finalquit()
{
	DeleteCriticalSection(&g_mainwndcs);
	DeleteCriticalSection(&g_srcdccs);
}

HDC draw_GetWindowDC(HWND hwnd)
{
#ifdef DRAW_DEBUG
	if (!hwnd)
	{
		MessageBox(NULL,"GWDC: hwnd=0","DRAW_DEBUG",0);
	}
#endif
	HDC hdc;
	EnterCriticalSection(&g_mainwndcs);
	hdc = GetWindowDC(hwnd);
#ifdef DRAW_DEBUG
	if (!hdc)
	{
		MessageBox(NULL,"GWDC: hdc=0","DRAW_DEBUG",0);
	}
#endif
  return hdc;
}

int draw_ReleaseDC(HWND hwnd, HDC hdc)
{
	int t=ReleaseDC(hwnd,hdc);
#ifdef DRAW_DEBUG
	if (!hwnd)
	{
		MessageBox(NULL,"RDC: hwnd=0","DRAW_DEBUG",0);
	}
	if (!hdc)
	{
		MessageBox(NULL,"RDC: hdc=0","DRAW_DEBUG",0);
	}
#endif
	LeaveCriticalSection(&g_mainwndcs);
	return t;
}

HBITMAP draw_LBitmap(LPCTSTR bmname, const wchar_t *filename)
{
	if (skin_directory[0] && filename) {
		HBITMAP bm;
		wchar_t bitmapfilename[MAX_PATH] = {0};
		PathCombineW(bitmapfilename, skin_directory, filename);
		bm = (HBITMAP)LoadImageW(hMainInstance, bitmapfilename, IMAGE_BITMAP, 0, 0, (palmode?LR_CREATEDIBSECTION:0)|LR_LOADFROMFILE);
		if (bm) return bm;
	}
	if (bmname) return (HBITMAP)LoadImage(hMainInstance, bmname, IMAGE_BITMAP, 0, 0, (palmode?LR_CREATEDIBSECTION:0));
	else return 0;
}

void do_palmode(HDC hdc)
{
	if (palmode)
	{
		SelectPalette(hdc,draw_hpal,FALSE);
		RealizePalette(hdc);
	}
}

int updateen=1;
void update_area(int x1, int y1, int w, int h);

void _setSrcBM(HBITMAP hbm
#ifdef DEBUG_DRAW
                      , char *a
#endif
                      )
{
	static HBITMAP old;
#ifdef DEBUG_DRAW
	if (!hbm && a)
	{
		char s[156] = {0};
		wsprintf(s,"Invalid bitmap: %s",a);
		DebugBreak();
		MessageBox(NULL,s,"DRAW_DEBUG error",MB_OK);
	}
	if (main_thread_id != GetCurrentThreadId()) 
		DebugBreak();//MessageBox(NULL,"Not in mainthread","DRAW_DEBUG error",MB_OK);
	if (hbm && old) 
	{
		char s[156] = {0};
		StringCchPrintf(s,156,"Tried to set bitmap when bitmap already set: %s",a);
		DebugBreak();
		MessageBox(NULL,s,"DRAW_DEBUG error",MB_OK);
	}
	if (!hbm && !old) 
	{
		DebugBreak();
		MessageBox(NULL,"Tried to unset bitmap when bitmap not set","DRAW_DEBUG error",MB_OK);
	}
#endif
	if (hbm) 
	{
		EnterCriticalSection(&g_srcdccs);
		old = (HBITMAP)SelectObject(bmDC,hbm);
	}
	else 
	{ 
		SelectObject(bmDC,old); old=0; 
		LeaveCriticalSection(&g_srcdccs);
	}
}

void draw_setnoupdate(int v)
{
	updateen=!v;
	if (!v)
		update_area(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
}

void draw_reinit_plfont(int update)
{
	EnterCriticalSection(&g_srcdccs);
	{
		HWND plw=hPLWindow;
		wchar_t font_name[MAX_PATH] = {0};
		int font_charset=DEFAULT_CHARSET;
		TEXTMETRIC tm;
		WADrawDC hdc(plw?plw:hMainWindow);
		HANDLE holdf;

		if (font) DeleteObject(font);
		if (mfont) DeleteObject(mfont);
		if (shadefont) DeleteObject(shadefont);
		if (osdFontText) DeleteObject(osdFontText);
		if (selbrush) DeleteObject(selbrush);
		if (mfont_bgbrush) DeleteObject(mfont_bgbrush);
		if (normbrush) DeleteObject(normbrush);
		mfont_bgbrush=selbrush=normbrush=0;
		font=0;
		mfont=0;
		shadefont=0;
		osdFontText = NULL;

		if (config_custom_plfont && *playlist_custom_fontW)
			StringCchCopyW(font_name, sizeof(font_name), playlist_custom_fontW);
		else
		{
			if (!Skin_PLFontW[0]) getStringW(IDS_PLFONT,font_name,sizeof(font_name)/sizeof(*font_name));
			else StringCbCopyW(font_name,sizeof(font_name), Skin_PLFontW);
		}

		// TODO: verify the existance of the font and fall back to Arial if it doesn't exist
		font_charset=atoi(getString(IDS_PLFONT_CHARSET,NULL,0));

		font=CreateFontW(ScaleY(-config_pe_fontsize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
						 font_charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY,
						 DEFAULT_PITCH | FF_DONTCARE, font_name);

		mfont=CreateFontW(-10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
						 font_charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
						 (config_dsize && !config_bifont && config_bifont_alt ? ANTIALIASED_QUALITY : DRAFT_QUALITY),
						 DEFAULT_PITCH | FF_DONTCARE, font_name);

		shadefont=CreateFontW(-8, 5, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
							  font_charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY,
							  DEFAULT_PITCH | FF_DONTCARE, font_name);

		osdFontText = CreateFontW(OSD_TEXT_SIZE, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
								  font_charset, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
								  ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, GetFontNameW());

		holdf=SelectObject(hdc,font);
		GetTextMetrics(hdc,&tm);
		if (!plw) SelectObject(hdc,holdf);
		pe_fontheight=tm.tmHeight;
		if (pe_fontheight < 1) pe_fontheight=1;

		holdf=SelectObject(hdc,mfont);
		GetTextMetrics(hdc,&tm);
		mfont_height=tm.tmHeight;
		SelectObject(hdc,holdf);

		{
			int ld=0,y;
			setSrcBM(fontBM);
			mfont_fgcolor=mfont_bgcolor=GetPixel(bmDC,150,4);
			for (y = 0; y < 6; y ++)
			{
				for (int x = 0; x < 20; x ++)
				{
					int d,a,b,c;
					COLORREF r=GetPixel(bmDC,x,y);
					a=(r&0xff)-(mfont_bgcolor&0xff);
					b=(((r&0xff00)>>8)-((mfont_bgcolor&0xff00)>>8));
					c=(((r&0xff0000)>>16)-((mfont_bgcolor&0xff0000)>>16));
					d=(a*a+b*b+c*c);
					if (d > ld) { ld=d; mfont_fgcolor=r; }
				}
			}
			unsetSrcBM();
			//mfont_fgcolor
		}

		{
			LOGBRUSH lb={BS_SOLID};
			lb.lbColor=GetNearestColor(hdc,Skin_PLColors[3]);
			selbrush = CreateBrushIndirect(&lb);
			lb.lbColor=GetNearestColor(hdc,Skin_PLColors[2]);
			normbrush = CreateBrushIndirect(&lb);
			lb.lbColor=mfont_bgcolor;
			mfont_bgbrush = CreateBrushIndirect(&lb);
		}
	}
	LeaveCriticalSection(&g_srcdccs);

	if (update) PostMessageW(hMainWindow, WM_WA_IPC, 0, IPC_CB_RESETFONT);
}

static struct
{
	BITMAPINFO bmi;
	RGBQUAD more_bm7iColors[256];
} bitmap;

static void CopyToMainBM()
{
  HDC hdc = CreateCompatibleDC(mainDC);
  HBITMAP oldbm = (HBITMAP)SelectObject(hdc,mainBM_save);
  BitBlt(mainDC,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,hdc,0,0,SRCCOPY);
  SelectObject(hdc,oldbm);
  DeleteDC(hdc);
}

void draw_init() 
{
	HDC screenHdc;
	EnterCriticalSection(&g_srcdccs);
	if (draw_initted) draw_kill();
	screenHdc = draw_GetWindowDC(hMainWindow);
	palmode = GetDeviceCaps(screenHdc,RASTERCAPS)&RC_PALETTE?1:0;
	mainDC = CreateCompatibleDC(screenHdc);
	mainDC2 = CreateCompatibleDC(screenHdc);

	bmDC = CreateCompatibleDC(screenHdc);
	specDC = CreateCompatibleDC(screenHdc);
	mainBM_save = draw_LBitmap(MAKEINTRESOURCE(IDB_MAINBITMAP),L"main.bmp");
	oldMainBM = (HBITMAP)SelectObject(mainDC,mainBM_save);
	mainBM = CreateCompatibleBitmap(mainDC,WINDOW_WIDTH,WINDOW_HEIGHT);
	SelectObject(mainDC,mainBM);
	CopyToMainBM();

	embedBM = draw_LBitmap(MAKEINTRESOURCE(IDB_EMBEDWND),L"gen.bmp");
	{
		COLORREF start;
		int x;
		int pos=0;
		setSrcBM(embedBM);
		if ((start = GetPixel(bmDC,0,90)) != CLR_INVALID)
		{

			for (x = 0; x < 26; x ++)
			{
				int cnt=0;
				while (GetPixel(bmDC,pos,90) == start) pos++;

				titlebar_font_offsets[x]=pos;
				for (;;)
				{
					COLORREF t=GetPixel(bmDC,pos,90);
					if (t == CLR_INVALID) break;
					pos++;
					if (t == start)
					{
						titlebar_font_widths[x]=cnt;
						break;
					}
					else cnt++;
				}
			}
		}

		if ((start = GetPixel(bmDC,0,74)) != CLR_INVALID)
		{
			pos = 0;
			for (x = 0; x < 12; x ++)
			{
				int cnt=0;
				while (GetPixel(bmDC,pos,74) == start) pos++;

				titlebar_font_num_offsets[x]=pos;
				for (;;)
				{
					COLORREF t=GetPixel(bmDC,pos,74);
					if (t == CLR_INVALID) break;
					pos++;
					if (t == start)
					{
						titlebar_font_num_widths[x]=cnt;
						break;
					}
					else cnt++;
				}
			}
		}
		unsetSrcBM();
	}

	cbuttonsBM = draw_LBitmap(MAKEINTRESOURCE(IDB_CBUTTONS),L"cbuttons.bmp");
	monostereoBM = draw_LBitmap(MAKEINTRESOURCE(IDB_MONOSTEREO),L"monoster.bmp");
	playpauseBM = draw_LBitmap(MAKEINTRESOURCE(IDB_PLAYPAUSE),L"playpaus.bmp");
	shufflerepeatBM = draw_LBitmap(MAKEINTRESOURCE(IDB_SHUFFLEREP),L"shufrep.bmp");
	numbersBM_ex = draw_LBitmap(NULL,L"nums_ex.bmp");
	if (!numbersBM_ex) numbersBM = draw_LBitmap(MAKEINTRESOURCE(IDB_NUMBERS1),L"numbers.bmp");
	else numbersBM=NULL;
	volBM = draw_LBitmap(MAKEINTRESOURCE(IDB_VOLBAR),L"volume.bmp");
	if (skin_directory[0])
		panBM = draw_LBitmap(NULL,L"balance.bmp");
	else 
		panBM = draw_LBitmap(MAKEINTRESOURCE(IDB_PANBAR),NULL);
	if (!panBM) panBM=volBM;
	fontBM = draw_LBitmap(MAKEINTRESOURCE(IDB_FONT1),L"text.bmp");
	posbarBM = draw_LBitmap(MAKEINTRESOURCE(IDB_POSBAR),L"posbar.bmp");
	tbBM = draw_LBitmap(MAKEINTRESOURCE(IDB_TB),L"titlebar.bmp");

	{
		int c;
		static unsigned char ppal2[] = { 
										0,0,0, // color 0 = black
										24,24,41, // color 1 = grey for dots
										239,49,16, // color 2 = top of spec
										206,41,16, // 3
										214,90,0, // 4
										214,102,0, // 5
										214,115,0, // 6
										198,123,8, // 7
										222,165,24, // 8
										214,181,33, // 9
										189,222,41, // 10
										148,222,33, // 11
										41,206,16, // 12
										50,190,16, // 13
										57,181,16, // 14
										49,156,8,  // 15
										41,148,0,  // 16
										24,132,8,   // 17
										255,255,255, // 18 = osc 1
										214,214,222, // 19 = osc 2 (slightly dimmer)
										181,189,189, // 20 = osc 3
										160,170,175,  // 21 = osc 4
										148,156,165,  // 22 = osc 4
										150, 150, 150, // 23 = analyzer peak
		};
		unsigned char ppal[sizeof(ppal2)];
		memcpy(ppal,ppal2,sizeof(ppal2));
		if (skin_directory[0])
		{
			FILE *fp;
			wchar_t bitmapfilename[MAX_PATH] = {0};
			PathCombineW(bitmapfilename, skin_directory, L"viscolor.txt");
			fp = _wfopen(bitmapfilename,L"rt");
			if (fp)
			{
				int x;
				for (x = 0; x < 24; x ++)
				{
					int t;
					char progdir[91],*p=progdir;
					fgets(progdir,90,fp);
					if (feof(fp)) break;
					for (t=0; t<3; t ++)
					{
						int b=0,s=0;
						while (p && (*p == ' ' || *p == ',' || *p == '\t')) p++;
						while (p && *p >= '0' && *p <= '9') {s=1;b=b*10+*p++-'0';}
						if (!s) { x=24; break; }
						ppal[x*3+t]=b;
					}
				}
				fclose(fp);
			}
		}
		for (c = 0; c < sizeof(ppal)/(3); c ++) {
			bitmap.bmi.bmiColors[c].rgbRed = ppal[c*3];
			bitmap.bmi.bmiColors[c].rgbGreen = ppal[c*3+1];
			bitmap.bmi.bmiColors[c].rgbBlue = ppal[c*3+2];
			bitmap.bmi.bmiColors[c].rgbReserved = 0;
		}
		bitmap.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmap.bmi.bmiHeader.biPlanes = 1;
		bitmap.bmi.bmiHeader.biBitCount = 8;
		bitmap.bmi.bmiHeader.biCompression = BI_RGB;
		bitmap.bmi.bmiHeader.biClrUsed = sizeof(ppal)/(3);
		bitmap.bmi.bmiHeader.biClrImportant = sizeof(ppal)/(3);
		bitmap.bmi.bmiHeader.biWidth = 76*2;
		bitmap.bmi.bmiHeader.biHeight = 16*2;
		bitmap.bmi.bmiHeader.biSizeImage = 76*16*4;
		specBM = CreateDIBSection(specDC,&bitmap.bmi,DIB_RGB_COLORS, (LPVOID*)&specData, NULL, 0);
		oldSpecBM = (HBITMAP)SelectObject(specDC,specBM);
		memset(specData,0,76*16*4);
	}

	if (palmode)
	{
		RGBQUAD rgb[256] = {0};
		int x;
		struct {
			LOGPALETTE lpal;
			PALETTEENTRY pal[256];
		} lPal;
		GetDIBColorTable(mainDC,0,256,rgb);
		lPal.lpal.palVersion = 0x300;
		lPal.lpal.palNumEntries = 256;
		for (x = 0; x < 256; x ++)
		{
			lPal.lpal.palPalEntry[x].peRed = rgb[x].rgbRed;
			lPal.lpal.palPalEntry[x].peGreen = rgb[x].rgbGreen;
			lPal.lpal.palPalEntry[x].peBlue = rgb[x].rgbBlue;
			lPal.lpal.palPalEntry[x].peFlags = 0;
		}
		
		draw_hpal = CreatePalette((LPLOGPALETTE)&lPal);
	}
	else draw_hpal = 0;
	draw_ReleaseDC(hMainWindow,screenHdc);

	draw_initted=1;

	draw_reinit_plfont(1);
	sa_kill = 0;
	draw_pe_init();
	draw_eq_init();
	draw_vw_init();
	LeaveCriticalSection(&g_srcdccs);
}

void draw_kill() 
{
	int old_sa_mode = sa_curmode;
	sa_setthread(-1);
	sa_kill = 1;
	while (sa_safe>0) 
		Sleep(50);
	sa_setthread(old_sa_mode);
	if (!draw_initted) return;
	EnterCriticalSection(&g_srcdccs);
	sa_safe=0;
	draw_initted=0;
	specData=0;
	DeleteObject(mainBM_save);
 	SelectObject(mainDC,oldMainBM);
	
	SelectObject(specDC,oldSpecBM);

	if (mainBM2)
	{
		SelectObject(mainDC2,oldmainBM2);
		DeleteObject(mainBM2);
		mainBM2=NULL;
	}

	DeleteObject(embedBM);

	DeleteDC(mainDC);
	DeleteDC(specDC);
	DeleteDC(mainDC2);
	DeleteDC(bmDC);

	DeleteObject(mainBM);
	DeleteObject(cbuttonsBM);
	DeleteObject(monostereoBM);
	DeleteObject(shufflerepeatBM);
	DeleteObject(playpauseBM);
	if (numbersBM) DeleteObject(numbersBM);
	if (numbersBM_ex) DeleteObject(numbersBM_ex);
	if (panBM != volBM) DeleteObject(panBM);

	DeleteObject(volBM);
	DeleteObject(fontBM);
	DeleteObject(posbarBM);
	DeleteObject(specBM);
	DeleteObject(tbBM);
	if (draw_hpal) DeleteObject(draw_hpal);
	draw_hpal = 0;
	if (font) DeleteObject(font);
	if (mfont) DeleteObject(mfont);
	if (shadefont) DeleteObject(shadefont);
	if (osdFontText) 	DeleteObject(osdFontText);
	if (selbrush) DeleteObject(selbrush);
	if (mfont_bgbrush) DeleteObject(mfont_bgbrush);
	if (normbrush) DeleteObject(normbrush);
	mfont_bgbrush=selbrush=normbrush=0;
	shadefont=mfont=font=0;
	LeaveCriticalSection(&g_srcdccs);
}

void draw_clear() 
{
	RECT r;
	if (!draw_initted) return;
	CopyToMainBM();
	draw_playicon(2);
	GetClientRect(hMainWindow,&r);
	draw_tbar(config_hilite?(GetForegroundWindow() == hMainWindow?1:0):1, config_windowshade,0);
	update_area(0,0,r.right,r.bottom);
}

void draw_clutterbar(int enable)
{
	int x,y;
	if (!draw_initted) return;
	if (config_ascb_new && !enable) enable=1;
	if (!enable)
	{
		x=8;
		y=0;
	}
	else if (enable == 1)
	{
		x=0;
		y=0;
	}
	else
	{
		y=44;
		x=(enable-2)*8;
	}
	setSrcBM(tbBM);
	BitBlt(mainDC,10,22,18-10,65-22,bmDC,304+x,y,SRCCOPY);
	if (enable != 3 && (config_ascb_new||enable))
	{
		if (config_aot)
		{
			BitBlt(mainDC,11,22+11,18-10-1,65-22-34-1,bmDC,312+1,44+11,SRCCOPY);
		}
		else
			BitBlt(mainDC,11,22+11,18-10-1,65-22-34-1,bmDC,304+1,11,SRCCOPY);
	}
	if (enable != 5 && (config_ascb_new||enable))
	{
		if (config_dsize)
		{
			BitBlt(mainDC,11,22+27,18-10-1,6,bmDC,328+1,44+27,SRCCOPY);
		}
		else
			BitBlt(mainDC,11,22+27,18-10-1,6,bmDC,304+1,27,SRCCOPY);
	}
	unsetSrcBM();
	update_area(10,22,8,65-22);
}

void update_area(int x1, int y1, int w, int h) 
{
	if (updateen && hMainWindow) 
	{
		WADrawDC tDC(hMainWindow);
		if (tDC)
		{ 
			do_palmode(tDC);
			if (!config_dsize) 
			{
				BitBlt(tDC,x1,y1,w,h,mainDC,x1,y1,SRCCOPY);
				if (mainBM2)
				{
					SelectObject(mainDC2,oldmainBM2);
					DeleteObject(mainBM2);
					mainBM2=NULL;
				}
			}
	  		else
			{
				if (!mainBM2)
				{
					mainBM2 = CreateCompatibleBitmap(mainDC,WINDOW_WIDTH*2,WINDOW_HEIGHT*2);
					oldmainBM2 = (HBITMAP)SelectObject(mainDC2,mainBM2);
					x1=y1=0;
					w=WINDOW_WIDTH;
					h=WINDOW_HEIGHT;
				}
				StretchBlt(mainDC2,x1*2,y1*2,w*2,h*2,mainDC,x1,y1,w,h,SRCCOPY);
				BitBlt(tDC,x1*2,y1*2,w*2,h*2,mainDC2,x1*2,y1*2,SRCCOPY);
			}
		}
	}
}

void getXYfromChar(wchar_t ic, int *x, int *y)
{
	int c,c2=0;
	switch (ic)
	{
		case L'°': ic = L'0'; break;
		case L'Ç': ic = L'C'; break;
		case L'ü': ic = L'u'; break;
		case L'è': case L'ë': case L'ê': case L'é': ic = L'e'; break;
		case L'á': case L'à': case L'â': ic = L'a'; break;
		case L'ç': ic = L'c'; break;
		case L'í': case L'ì': case L'î': case L'ï': ic = L'i'; break;
		case L'É': ic = L'E'; break;
		case L'æ': ic = L'a'; break;
		case L'Æ': ic = L'A'; break;
		case L'ó': case L'ò': case L'ô': ic = L'o'; break;
		case L'ú': case L'ù': case L'û': ic = L'u'; break;
		case L'ÿ': ic = L'y'; break;
		case L'Ü': ic = L'U'; break;
		case L'ƒ': ic = L'f'; break;
		case L'Ñ': case L'ñ': ic = L'n'; break;
		default: break;
	} // quick relocations
	if (ic <= L'Z' && ic >= L'A') c = (ic-'A');
	else if (ic <= L'z' && ic >= L'a') c = (ic-'a');
	else
	{
		c2 = 6;
		if (ic == L'\1') c=10;
		else if (ic == L'.') c = 11;
  		else if (ic <= L'9' && ic >= L'0') c = ic - '0';
  		else if (ic == L':') c = 12;
		else if (ic == L'(') c = 13;
  		else if (ic == L')') c = 14;
		else if (ic == L'-') c = 15;
		else if (ic == L'\'' || ic=='`') c = 16;
		else if (ic == L'!') c = 17;
		else if (ic == L'_') c = 18;
		else if (ic == L'+') c = 19;
		else if (ic == L'\\') c = 20;
		else if (ic == L'/') c = 21;
		else if (ic == L'[' || ic == L'{' || ic == L'<') c = 22;
		else if (ic == L']' || ic == L'}' || ic == L'>') c = 23;
		else if (ic == L'~' || ic == L'^') c = 24;
		else if (ic == L'&') c = 25;
		else if (ic == L'%') c = 26;
		else if (ic == L',') c = 27;
		else if (ic == L'=') c = 28;
		else if (ic == L'$') c = 29;
		else if (ic == L'#') c = 30;
		else 
		{
			c2=12;
			if (ic == L'Å' || ic == L'å') c = 0;
			else if (ic == L'Ö' || ic == L'ö') c = 1;
			else if (ic == L'Ä' || ic == L'ä') c = 2;
			else if (ic == L'?') c = 3;
			else if (ic == L'*') c = 4;
			else
			{
				c2 = 0;
				if (ic == L'"') c = 26;
				else if (ic == L'@') c = 27;
				else c = 30;
			}
		}
	}
	c*=5;
	*x=c;
	*y=c2;
}