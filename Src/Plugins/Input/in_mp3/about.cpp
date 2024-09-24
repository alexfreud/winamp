#include <windows.h>
#include "main.h"
#if 1
BOOL CALLBACK AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)) EndDialog(hwndDlg,0);
	return 0;
}

#else
#include <commctrl.h>

#include ".\graphics\image.h"
#include ".\graphics\imagefilters.h"
#include <strsafe.h>

 

#define random( min, max ) (( rand() % (int)((( max ) + 1 ) - ( min ))) + ( min ))

HBITMAP LoadImageFromResource(INT_PTR handle);

#define LEGAL_COUNT 4
wchar_t *legal[] = {L"Copyright (C) 1998-2006 - Nullsoft, Inc.",
					L"MPEG Layer-3 audio compression technology licensed by Fraunhofer IIS and THOMSON multimedia.",
					L"VLB decoding copyright 1998-2002 by Dolby Laboratories, Inc.  All rights reserved.",
					L"AAC && aacPlus decoding copyright 1998-2006 by Coding Technologies, Inc.  All rights reserved."};

HFONT fntPlugin, fntC, fntLegal;
HBRUSH brhDlgBG;

#define IMAGES_COUNT			6
#define IMAGES_INDEX_MY		0
#define IMAGES_INDEX_WA		1
#define IMAGES_INDEX_CT		2
#define IMAGES_INDEX_IIS		3
#define IMAGES_INDEX_ID3V2	4
#define IMAGES_INDEX_MP3S	5

RECT rectLogo[IMAGES_COUNT];
MLImage *imgLogo[IMAGES_COUNT] = {NULL, NULL, NULL, NULL, NULL, NULL};
MLImage *imgLlama = NULL, *imgLlamaOrig = NULL, *imgMy = NULL;
RECT rcLlama;

int idxSelected;

wchar_t *url[IMAGES_COUNT - 1] = {	L"http://winamp.com/",
									L"http://www.codingtechnologies.com/index.htm",
									L"http://www.iis.fraunhofer.de/index.html",
									L"http://www.id3.org/",
									L"http://www.iis.fraunhofer.de/amm/download/mp3surround/index.html"};

HWND hwndTT;
wchar_t strTT[] = L"click here to visit this website";

#define TIMER_ID_WATERRENDER		1980
#define TIMER_ID_WATERPULSE		1978
#define TIMER_DELAY_WATERRENDER	48
#define TIMER_DELAY_WATERPULSE	12000
#define TIMER_ID_LLAMAFADE		1987
#define TIMER_DELAY_LLAMAFADE	200

#define TIMER_ID_LOADDATA		1959
#define TIMER_DELAY_LOADDATA		10
MLImage *tmpImage = NULL;

MLImageFilterWater *fltrWater = NULL;
HCURSOR curHand = NULL;

void about_OnInit(HWND hwndDlg);
void about_OnDestroy(HWND hwndDlg);
void about_OnMouseDown(HWND hwndDlg, int cx, int cy);
void about_OnMouseMove(HWND hwndDlg, int cx, int cy);
void about_OnDraw(HWND hwndDlg);
void timer_OnWaterPulse(HWND hwndDlg);
void timer_OnLlamaFade(HWND hwndDlg);
void timer_OnLoadData(HWND hwndDlg);


void SetRects(int cx, int cy)
{
	int i, xl, yl;
	
	i = IMAGES_INDEX_MY; xl = (cx - imgLogo[i]->GetWidth())/2; yl = 64; 
	if (imgLogo[i]) SetRect(&rectLogo[i], xl, yl, xl + imgLogo[i]->GetWidth(), yl + imgLogo[i]->GetHeight());
	xl = 2; yl = 2; i = IMAGES_INDEX_WA; 
	if (imgLogo[i]) SetRect(&rectLogo[i], xl, yl, xl + imgLogo[i]->GetWidth(), yl + imgLogo[i]->GetHeight());
	xl = 2; yl = cy - 88; i = IMAGES_INDEX_CT; 
	if (imgLogo[i]) SetRect(&rectLogo[i], xl, yl, xl + imgLogo[i]->GetWidth(), yl + imgLogo[i]->GetHeight());
	xl = rectLogo[i].right; i = IMAGES_INDEX_IIS; 
	if (imgLogo[i]) SetRect(&rectLogo[i], xl, yl, xl + imgLogo[i]->GetWidth(), yl + imgLogo[i]->GetHeight());
	xl = rectLogo[i].right;  i = IMAGES_INDEX_ID3V2; 
	if (imgLogo[i]) SetRect(&rectLogo[i], xl, yl, xl + imgLogo[i]->GetWidth(), yl + imgLogo[i]->GetHeight());
	xl = rectLogo[i].right; i = IMAGES_INDEX_MP3S; 
	if (imgLogo[i]) SetRect(&rectLogo[i], xl, yl, xl + imgLogo[i]->GetWidth(), yl + imgLogo[i]->GetHeight());

	if(imgLlama) SetRect(&rcLlama, 2, 1, imgLlama->GetWidth() + 2, imgLlama->GetHeight() + 1);
}
void CreateToolTipWnd(HWND hwndDlg)
{
    INITCOMMONCONTROLSEX iccex; 
	iccex.dwICC = ICC_WIN95_CLASSES;
    iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCommonControlsEx(&iccex);
    
    hwndTT = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndDlg, NULL, mod.hDllInstance, NULL);

    SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void UpdateToolTips(HWND hwndDlg)
{
	TOOLINFOW ti;
	unsigned int uid = 0;
	
	// delete tools
	int ttCount = SendMessage(hwndTT, TTM_GETTOOLCOUNT, 0, 0);
	for(int i = 0; i < ttCount; i++)
	{
		if (SendMessageW(hwndTT, TTM_ENUMTOOLSW, (WPARAM)i, (LPARAM)&ti)) SendMessageW(hwndTT, TTM_DELTOOLW, 0, (LPARAM)&ti);
	}
	
	/// add tools
    
	for (int i = 1; i < IMAGES_COUNT -1; i++)
	{
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = hwndDlg;
		ti.hinst = mod.hDllInstance;
		ti.uId = uid;
		ti.lpszText = strTT;
		ti.rect = rectLogo[i];
		SendMessageW(hwndTT, TTM_ADDTOOLW, 0, (LPARAM) (LPTOOLINFO) &ti);
	}
}
BOOL CALLBACK AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
			about_OnInit(hwndDlg);
			break;
		case WM_DESTROY:
			about_OnDestroy(hwndDlg);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hwndDlg,0);
			}
			break;
		case WM_SIZE:
			if (wParam != SIZE_MINIMIZED)
			{
				SetRects(LOWORD(lParam), HIWORD(lParam));
				UpdateToolTips(hwndDlg);
			}
		case WM_MOUSEMOVE:
			about_OnMouseMove(hwndDlg, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_LBUTTONDOWN:
			about_OnMouseDown(hwndDlg, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_PAINT:
			about_OnDraw(hwndDlg);
			break;
		case WM_TIMER:
			switch(wParam)
			{
				case TIMER_ID_WATERRENDER:
					if (idxSelected != -1 && fltrWater)
					{
						fltrWater->Render(tmpImage, imgLogo[idxSelected]);
						InvalidateRect(hwndDlg, &rectLogo[idxSelected], FALSE);
					}
					break;
				case TIMER_ID_WATERPULSE:
					timer_OnWaterPulse(hwndDlg);
					break;
				case TIMER_ID_LLAMAFADE:
					timer_OnLlamaFade(hwndDlg);
					break;
				case TIMER_ID_LOADDATA:
					timer_OnLoadData(hwndDlg);
					break;
			}
			break;
		case WM_CTLCOLORDLG:
            return (BOOL)brhDlgBG;
			break;
	}
	return 0;
}
void about_OnInit(HWND hwndDlg)
{
	HDC hdc = GetDC(hwndDlg);
	fntPlugin = CreateFontW(-MulDiv(20, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Microsoft Sans Serif");
	fntC = CreateFontW(-MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, 0, TRUE, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, 0, L"Arial");
	fntLegal = CreateFontW(-MulDiv(6, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, 0, L"Microsoft Sans Serif");
	ReleaseDC(hwndDlg, hdc);

	brhDlgBG = CreateSolidBrush(RGB(255,255,255));

	SetTimer(hwndDlg, TIMER_ID_LOADDATA, TIMER_DELAY_LOADDATA, NULL);
}

void about_OnDestroy(HWND hwndDlg)
{
	for (int i = 0; i < IMAGES_COUNT; i++)
	{
		if (imgLogo[i]) delete(imgLogo[i]);
		imgLogo[i] = NULL;
	}

	if (imgLlama) delete(imgLlama);
	imgLlama = NULL;

	if (imgLlamaOrig) delete(imgLlamaOrig);
	imgLlamaOrig = NULL;
	
	if (imgMy) delete(imgMy);
	imgMy = NULL;
	
	if (fntPlugin) DeleteObject(fntPlugin);
	if (fntC) DeleteObject(fntC);
	if (fntLegal) DeleteObject(fntLegal);
	fntPlugin = NULL;
	fntC = NULL;
	fntLegal = NULL;

	if (brhDlgBG) DeleteObject(brhDlgBG);
	brhDlgBG = NULL;

	if (fltrWater) delete (fltrWater);
	fltrWater = NULL;
	
	if (tmpImage) delete(tmpImage);
	tmpImage = NULL;

	if (curHand) DestroyCursor(curHand);
	curHand = NULL;
}

void about_OnMouseDown(HWND hwndDlg, int cx, int cy)
{

	if (idxSelected == -1) return;
	
	HCURSOR curWait = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(32514/*OCR_WAIT*/), IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR);
	SetCursor(curWait);
	ShellExecuteW(hwndDlg, L"open", url[idxSelected -1], NULL, L"c:\\", SW_SHOW);
	SetCursor(curHand);
	DestroyCursor(curWait);
}
void about_OnMouseMove(HWND hwndDlg, int cx, int cy)
{
	POINT pt = {cx, cy};
	int idxNew = -1;
	for (int i = 1; i < IMAGES_COUNT - 1; i ++)
	{
		if (PtInRect(&rectLogo[i], pt))
		{ 
			if (!curHand)
			{
				curHand = (HCURSOR)LoadImage(mod.hDllInstance, MAKEINTRESOURCE(IDC_CUR_HAND), IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR);
			}
			SetCursor(curHand);
			idxNew = i;
		}
	}
	if (idxNew != idxSelected)
	{
		// stop animation
		KillTimer(hwndDlg, TIMER_ID_WATERPULSE);
		KillTimer(hwndDlg, TIMER_ID_WATERRENDER);
		
		if (idxSelected != -1) // redraw previously animated
		{
			InvalidateRect(hwndDlg, &rectLogo[idxSelected], FALSE);
		}
		// set new one
		idxSelected = idxNew;
		if (fltrWater) delete(fltrWater);
		fltrWater = NULL;
		if (tmpImage) delete(tmpImage);
		tmpImage = NULL;
		if (idxSelected != -1 && idxSelected != IMAGES_INDEX_WA) SetTimer(hwndDlg, TIMER_ID_WATERPULSE, 30, NULL); // start delay
	} 
}
void about_OnDraw(HWND hwndDlg)
{
	PAINTSTRUCT ps;
	HDC  hdc;
	hdc = BeginPaint(hwndDlg, &ps);
	RECT rc, ri;
	GetClientRect(hwndDlg, &rc);
	
	// Draw Llama
	RECT rl;
	SetRect(&rl, rcLlama.left, rcLlama.top, rcLlama.right, rcLlama.bottom);
	if (imgLlama && IntersectRect(&ri, &rl, &ps.rcPaint)) 
	{
		HRGN hrgn = CreateRectRgn(rcLlama.left, rcLlama.top, rcLlama.right, rcLlama.bottom);
		
		HRGN hrgn1 = CreateRectRgn(rectLogo[IMAGES_INDEX_MY].left,
									rectLogo[IMAGES_INDEX_MY].top,
									rectLogo[IMAGES_INDEX_MY].right,
									rectLogo[IMAGES_INDEX_MY].bottom);
		CombineRgn(hrgn, hrgn, hrgn1, RGN_DIFF);
		DeleteObject(hrgn1);
		hrgn1 = CreateRectRgn(rectLogo[IMAGES_INDEX_WA].left,
									rectLogo[IMAGES_INDEX_WA].top,
									rectLogo[IMAGES_INDEX_WA].right,
									rectLogo[IMAGES_INDEX_WA].bottom);
		CombineRgn(hrgn, hrgn, hrgn1, RGN_DIFF);
		SelectClipRgn(hdc, hrgn);
		DeleteObject(hrgn);
		DeleteObject(hrgn1);

		imgLlama->Draw(hdc, rl.left, rl.top);
		SelectClipRgn(hdc, NULL);
	}
	
	MLImage *img;
	for (int i = 0; i < IMAGES_COUNT -1; i++)
	{
		
		if (IntersectRect(&ri, &rectLogo[i], &ps.rcPaint))		
		{
			if (idxSelected == i && tmpImage)
			{
				img = tmpImage;
				HRGN hrgn = CreateRectRgn(rectLogo[i].left + 3, rectLogo[i].top + 3 , rectLogo[i].right - 3, rectLogo[i].bottom - 3);
				SelectClipRgn(hdc, hrgn);
				DeleteObject(hrgn);
			}
			else img = imgLogo[i];
			if (img == NULL || imgLlama == NULL) continue;
			if (i == IMAGES_INDEX_MY)
			{ // blend Llama First
				MLImageFilter_Blend1(imgMy, img, 0, 0,
									imgLlama->GetWidth() - (rectLogo[i].left - rcLlama.left), img->GetHeight(), imgLlama,
									rectLogo[i].left - rcLlama.left, rectLogo[i].top - rcLlama.top, RGB(255,255,255));
				img = imgMy;
			}
			img->Draw(hdc, rectLogo[i].left, rectLogo[i].top);
			SelectClipRgn(hdc, NULL);
		}
	}
	
	RECT rt;
	SetBkMode(hdc, TRANSPARENT);
	rc.left += 6;
	rc.bottom -= 2;
	rc.right -= 4;

	HFONT oldF = NULL;

	SetRect(&rt, rc.right - 192, rc.top,  rc.right, rc.top + 16);  
	if (IntersectRect(&ri, &rt, &ps.rcPaint))
	{
		/// Nullsoft (c)
		HFONT oldF = (HFONT)SelectObject(hdc, fntC);
		SetTextColor(hdc, RGB(100,100,200));
		DrawTextW(hdc, legal[0], -1, &rt, DT_LEFT | DT_SINGLELINE);
	}
	
	SetRect(&rt, rc.left - 2, rc.bottom - 33,  rc.right, rc.bottom);  
	if (IntersectRect(&ri, &rt, &ps.rcPaint))
	{
		/// Separator
		MoveToEx(hdc, rt.left, rt.top, NULL);
		HPEN lp = CreatePen(PS_SOLID,1, RGB(190, 190, 190));
		HPEN op = (HPEN)SelectObject(hdc, lp);
		LineTo(hdc, rt.right, rt.top);
		SelectObject(hdc, lp);
		DeleteObject(lp);
		
		/// Legal...
		oldF  = (oldF != NULL) ? oldF : (HFONT)SelectObject(hdc, fntLegal);
		SetTextColor(hdc, RGB(0,0,0));
		for(int i = 1; i < LEGAL_COUNT; i++)
		{
			int y = rc.bottom - (LEGAL_COUNT - i)*10;
			SetRect(&rt, rc.left, y,  rc.right, y +10);  
			if (IntersectRect(&ri, &rt, &ps.rcPaint))
				DrawTextW(hdc, legal[i], -1, &rt, DT_LEFT | DT_SINGLELINE);
		}
	}
	if (oldF != NULL) SelectObject(hdc, oldF);
	EndPaint(hwndDlg, &ps);
}
void timer_OnLoadData(HWND hwndDlg)
{
	static step = 0;
	KillTimer(hwndDlg, TIMER_ID_LOADDATA);
	RECT rc; 
	GetClientRect(hwndDlg, &rc);
	for (int i = 0; i < IMAGES_COUNT; i++)
	{
		imgLogo[i] = new MLImage(LoadImageFromResource, TRUE);
		imgLogo[i]->Load();
	}

	imgMy = new MLImage();
	MLImage::Copy(imgMy, imgLogo[IMAGES_INDEX_MY]);
		
	imgLlamaOrig = new MLImage(LoadImageFromResource, TRUE);
	imgLlamaOrig->Load();
	imgLlama = new MLImage();
	MLImage::Copy(imgLlama, imgLlamaOrig);
	CreateToolTipWnd(hwndDlg);
    UpdateToolTips(hwndDlg);
	idxSelected = -1;
	SetRects(rc.right - rc.left, rc.bottom - rc.top);
	InvalidateRect(hwndDlg, &rc, FALSE);
//	SetTimer(hwndDlg, TIMER_ID_LLAMAFADE, TIMER_DELAY_LLAMAFADE, NULL);

}
void timer_OnWaterPulse(HWND hwndDlg)
{
	if (idxSelected == -1) return;
	
	BOOL startRender = (!fltrWater);
	
	if(!fltrWater)
	{	
		KillTimer(hwndDlg, TIMER_ID_WATERPULSE); // stop timer  - will change to slower interval
		fltrWater = new MLImageFilterWater();
		fltrWater->CreateFor(imgLogo[idxSelected]);
		tmpImage = new MLImage(imgLogo[idxSelected]->GetWidth(), imgLogo[idxSelected]->GetHeight());
	}
	
	int ow = imgLogo[idxSelected]->GetWidth();
	int oh = imgLogo[idxSelected]->GetHeight();
						
	for (int i = 0; i < oh*ow/1024; i++)
	{
		fltrWater->SineBlob(-1, -1,  random(4,min(oh,ow)), 600, 0);
	}
	
	if (startRender)
	{
		SetTimer(hwndDlg, TIMER_ID_WATERRENDER, TIMER_DELAY_WATERRENDER, NULL);
		SetTimer(hwndDlg, TIMER_ID_WATERPULSE, TIMER_DELAY_WATERPULSE, NULL);
	}
}
void timer_OnLlamaFade(HWND hwndDlg)
{
	static int c = -2;
	static int step = 2;
	c = c + step;
	if (c > 30 && step > 0) {step = 0 - step; c = 30;}
	else if (c < 0 && step < 0) { step = 0 - step; c = 0;	}
	MLImageFilter_Fader3(imgLlama, imgLlamaOrig, c);
	InvalidateRect(hwndDlg, &rcLlama, FALSE);
}

HBITMAP LoadImageFromResource(INT_PTR handle)
{
	int id = 0;
	BOOL incSize;
	MLImage *img = (MLImage*)handle;

	if (img == imgLogo[IMAGES_INDEX_WA]) {id = IDB_LOGO_WAPLUGINS; incSize = FALSE;}
	else if (img == imgLogo[IMAGES_INDEX_IIS]) {id = IDB_LOGO_IIS; incSize = TRUE;}
	else if (img == imgLogo[IMAGES_INDEX_CT]) {id = IDB_LOGO_CODETECH; incSize = TRUE;}
	else if (img == imgLogo[IMAGES_INDEX_ID3V2]) {id = IDB_LOGO_ID3V2; incSize = TRUE;}
	else if (img == imgLogo[IMAGES_INDEX_MP3S]) {id = IDB_LOGO_MP3SURROUND; incSize = FALSE;} // because we don't use it for now
	else if (img == imgLogo[IMAGES_INDEX_MY]) {id = IDB_LOGO_MY; incSize = FALSE;}
	else if (img == imgLlamaOrig) {id = IDB_LLAMA; incSize = FALSE;}
	if (id == 0) return NULL;

	HBITMAP hbmp = LoadBitmap(mod.hDllInstance, MAKEINTRESOURCE(id));
	if (hbmp == NULL) return hbmp;

	if (incSize)  
	{// we want to add additional white border
		HDC hdcWnd = GetWindowDC(NULL);
		HDC hdcSrc = CreateCompatibleDC(hdcWnd);
		HDC hdcDest = CreateCompatibleDC(hdcWnd);
		BITMAP bmp;
		GetObject(hbmp, sizeof(BITMAP), &bmp);
		int extraW = 40;
		int extraH = 16;
		HBITMAP hbmpNew = CreateBitmap(bmp.bmWidth + extraW, bmp.bmHeight + extraH, bmp.bmPlanes, bmp.bmBitsPixel, NULL);
		
		SelectObject(hdcSrc, hbmp);
		SelectObject(hdcDest, hbmpNew);

		// fill borders
		HBRUSH br = CreateSolidBrush(RGB(255,255,255));
		RECT rc;
		SetRect(&rc, 0,0, bmp.bmWidth + extraW, extraH/2);
		FillRect(hdcDest, &rc, br);
		SetRect(&rc, 0, bmp.bmHeight + extraH/2, bmp.bmWidth + extraW, bmp.bmHeight + extraH);
		FillRect(hdcDest, &rc, br);
		SetRect(&rc, 0, extraH/2, extraW/2, bmp.bmHeight + extraH/2);
		FillRect(hdcDest, &rc, br);
		SetRect(&rc, bmp.bmWidth + extraW/2, extraH/2, bmp.bmWidth + extraW, bmp.bmHeight + extraH/2);
		FillRect(hdcDest, &rc, br);
		// copy original
		BitBlt(hdcDest, extraW/2, extraH/2, bmp.bmWidth, bmp.bmHeight, hdcSrc, 0,0, SRCCOPY);

		DeleteObject(br);
		DeleteObject(hbmp);
		DeleteDC(hdcSrc);
		DeleteDC(hdcDest);
		ReleaseDC(NULL, hdcWnd);

		hbmp = hbmpNew;
	}
	if (img == imgLogo[IMAGES_INDEX_MY])
	{ // need to add vesion number
		HDC hdcWnd = GetWindowDC(NULL);
		HDC hdcSrc = CreateCompatibleDC(hdcWnd);
		HDC hdcDest = CreateCompatibleDC(hdcWnd);

		HFONT fnt = CreateFontW(-MulDiv(90, GetDeviceCaps(hdcDest, LOGPIXELSY), 72), 22, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, 0, L"Agency FB");
		SelectObject(hdcDest, fnt);

		char *ver =  mod.description  + lstrlen(mod.description);
		while(ver != mod.description && *ver != ' ')
		{
			ver = CharPrevA(mod.description, ver);
		}
		if (*ver == ' ') ver++;
		
		RECT rc;
		SetRect(&rc, 0, 0, 0, 0);
		DrawText(hdcDest, ver, -1,  &rc, DT_CALCRECT | DT_RIGHT |DT_SINGLELINE);

		int extraW = rc.right + 6;
		int extraH = 16;

		BITMAP bmp;
		GetObject(hbmp, sizeof(BITMAP), &bmp);
		HBITMAP hbmpNew = CreateBitmap(bmp.bmWidth + extraW, bmp.bmHeight + extraH, bmp.bmPlanes, bmp.bmBitsPixel, NULL);
		
		SelectObject(hdcSrc, hbmp);
		SelectObject(hdcDest, hbmpNew);

		// fill new area
		HBRUSH br = CreateSolidBrush(RGB(255,255,255));
	
		SetRect(&rc, bmp.bmWidth,0, bmp.bmWidth + extraW, bmp.bmHeight + extraH);
		FillRect(hdcDest, &rc, br);
		SetRect(&rc, 0,0, bmp.bmWidth, extraH);
		FillRect(hdcDest, &rc, br);
		// copy original
		BitBlt(hdcDest, 0, extraH, bmp.bmWidth, bmp.bmHeight, hdcSrc, 0,0, SRCCOPY);
		// draw number
		
		SetTextColor(hdcDest, RGB(80, 60, 10));
		SetBkMode(hdcDest, TRANSPARENT);
		SetRect(&rc, bmp.bmWidth + 6, rc.top -= 22, bmp.bmWidth + extraW, bmp.bmHeight + extraH);
		DrawText(hdcDest, ver, -1,  &rc, DT_RIGHT |DT_SINGLELINE);
		DeleteObject(fnt);
		DeleteObject(br);
		DeleteObject(hbmp);
		DeleteDC(hdcSrc);
		DeleteDC(hdcDest);
		ReleaseDC(NULL, hdcWnd);

		hbmp = hbmpNew;
		
	}


	return  hbmp;
	
}

#endif