#include ".\banner.h"
#include "..\gen_ml\graphics.h"


MLBanner::MLBanner(void)
{
	bmpBck		= NULL;
	bmpLogo		= NULL;
	bmpLogoMask	= NULL;
	bmpBanner   = NULL;

	oldWndProc	= NULL;

	color1 = RGB(0,0,0);
	color2 = RGB(255,255,255);

	hInstance = NULL;
	logoResId = 0; 
	bgndResId = 0;
	m_hwnd = 0;

	SetRect(&rcBanner, 0,0,0,0);
}
MLBanner::~MLBanner(void)
{
	DestroyImages();
	SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)oldWndProc);
	oldWndProc	= NULL;
}

void MLBanner::SetColors(int color1, int color2)
{
	this->color1 = color1;
	this->color2 = color2;
	ReloadImages();
}

void MLBanner::SetImages(HINSTANCE hInstance, int bgndResId, int logoResId)
{
	this->hInstance = hInstance;
	this->logoResId = logoResId;
	this->bgndResId = bgndResId;
	ReloadImages();
}

void MLBanner::ReloadImages(void)
{
	DestroyImages();
	if (hInstance)
	{
		if (bgndResId) 
		{
			bmpBck = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(bgndResId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
			if (bmpBck) bmpBck = PatchBitmapColors24(bmpBck, color1, color2, Filter1);
		}
		if (logoResId)
		{
			bmpLogo = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(logoResId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
			if (bmpLogo) 
			{
				bmpLogoMask = CreateBitmapMask(bmpLogo, 1,1);
			}
		}
	}

}

void MLBanner::DestroyImages(void)
{
	if (bmpBck) DeleteObject(bmpBck);
	bmpBck = NULL;
	
	if (bmpLogo) DeleteObject(bmpLogo);
	bmpLogo = NULL;

	if (bmpLogoMask) DeleteObject(bmpLogoMask);
	bmpLogoMask = NULL;

	if (bmpBanner) DeleteObject(bmpBanner);
	bmpBanner = NULL;
}



void MLBanner::UpdateBunnerBmp(void)
{
	if (bmpBanner) DeleteObject(bmpBanner);
	
	HDC hdc = GetDC(m_hwnd);
	
	bmpBanner = CreateCompatibleBitmap(hdc, rcBanner.right, rcBanner.bottom);
	HDC memDstDC = CreateCompatibleDC (hdc);
	HDC memSrcDC = CreateCompatibleDC (hdc);
    SelectObject(memDstDC, bmpBanner);
	SelectObject(memSrcDC, bmpBck);

	for (int i = 0; i < rcBanner.right; i++)
	{
		BitBlt(memDstDC,  
        i,0, 
        1, rcBanner.bottom, 
        memSrcDC, 
        0,0, 
       SRCCOPY); 
	   
	}
	
	BITMAP bm;
    GetObject(bmpLogo, sizeof(BITMAP), &bm);

	SelectObject(memSrcDC, bmpLogoMask);
	BitBlt(memDstDC,  
		6, 
		max(2, (rcBanner.bottom - bm.bmHeight) / 2),
		min(rcBanner.right - 4, bm.bmWidth), 
		min(rcBanner.bottom - 2, bm.bmHeight), 
		memSrcDC, 
		0,0, 
		SRCAND);

    SelectObject(memSrcDC, bmpLogo);
   	BitBlt(memDstDC,  
		6, 
		max(2, (rcBanner.bottom - bm.bmHeight) / 2),
		min(rcBanner.right - 4, bm.bmWidth), 
		min(rcBanner.bottom - 2, bm.bmHeight), 
		memSrcDC, 
		0,0, 
		SRCPAINT);

	ReleaseDC(m_hwnd, hdc); 
	DeleteDC(memDstDC);
	DeleteDC(memSrcDC);

}

void MLBanner::Init(HWND hwnd)
{
	m_hwnd = hwnd;
	SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)this);
	oldWndProc= (WNDPROC) SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)newWndProc);
	UpdateBunnerBmp();
}

INT_PTR CALLBACK MLBanner::newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	MLBanner *banner = (MLBanner*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	
	switch(uMsg)
	{
		case WM_SIZE:
			if (SIZE_MINIMIZED != wParam)
			{
				SetRect(&banner->rcBanner, 0,0,LOWORD(lParam),HIWORD(lParam));
				banner->UpdateBunnerBmp();
			}
			break;
		case WM_ERASEBKGND:
			{
				HDC hdc = GetDC(hwndDlg);
				if (banner->bmpBanner)
				{
					HDC memSrcDC = CreateCompatibleDC (hdc);
					SelectObject(memSrcDC, banner->bmpBanner);
					StretchBlt(	hdc,  
								banner->rcBanner.left,
								banner->rcBanner.top,
								banner->rcBanner.right - banner->rcBanner.left, 
								banner->rcBanner.bottom - banner->rcBanner.top, 
								memSrcDC, 
								banner->rcBanner.left,
								banner->rcBanner.top,
								banner->rcBanner.right - banner->rcBanner.left, 
								banner->rcBanner.bottom - banner->rcBanner.top, 
								SRCCOPY); 
					DeleteDC(memSrcDC);
				}
				ReleaseDC(hwndDlg, hdc);
			}
			return TRUE;
		case WM_PAINT:
			{
				PAINTSTRUCT pt;
				HDC hdc = BeginPaint(hwndDlg, &pt);
				if (!banner->bmpBanner)
				{
					SetRect(&banner->rcBanner, 0,0,pt.rcPaint.right - pt.rcPaint.left, pt.rcPaint.bottom - pt.rcPaint.top);
					banner->UpdateBunnerBmp();
				}
				if (banner->bmpBanner)
				{
					HDC memSrcDC = CreateCompatibleDC (hdc);
					SelectObject(memSrcDC, banner->bmpBanner);
					StretchBlt(	hdc,  
								pt.rcPaint.left,
								pt.rcPaint.top,
								pt.rcPaint.right - pt.rcPaint.left, 
								pt.rcPaint.bottom - pt.rcPaint.top, 
								memSrcDC, 
								pt.rcPaint.left,
								pt.rcPaint.top,
								pt.rcPaint.right - pt.rcPaint.left, 
								pt.rcPaint.bottom - pt.rcPaint.top, 
								SRCCOPY); 
					DeleteDC(memSrcDC);
					ValidateRect(hwndDlg, &pt.rcPaint);
				}
				EndPaint(hwndDlg, &pt);
			}
			break;
	}

	return CallWindowProc(banner->oldWndProc, hwndDlg, uMsg, wParam, lParam);
}

	