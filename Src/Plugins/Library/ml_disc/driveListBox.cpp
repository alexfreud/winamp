#include "main.h"
#include ".\DriveListBox.h"
#include "resource.h"
#include "..\..\General\gen_ml\graphics.h"
#include <strsafe.h>

DriveListBox::DriveListBox(int controlId)
{
	m_hwnd		= NULL;
	m_parentHwnd= NULL;
	hInstance	= NULL;
	bmpNormal	= NULL;
	bmpSelected = NULL;
	driveResId	= NULL; 
	bgndResId	= NULL;

	this->controlId = controlId;

	SetRect(&rcItem, 0,0,0,68); // hardcoded height
	
	clrNormalBG  = RGB(0,0,0);
	clrSelected1 = RGB(0,0,0);
	clrSelected2 = RGB(255,255,255);
	clrTextSel	 = RGB(255,255,255);
	clrTextNorm  = RGB(0,255,0);
}

DriveListBox::~DriveListBox(void)
{
	DestroyImages();
}

void DriveListBox::DestroyImages(void)
{
	if (bmpNormal) DeleteObject(bmpNormal);
	bmpNormal = NULL;
	
	if (bmpSelected) DeleteObject(bmpSelected);
	bmpSelected = NULL;
}

void DriveListBox::SetColors(COLORREF clrNormalBG, COLORREF clrSelected1, COLORREF clrSelected2, COLORREF clrTextSel, COLORREF clrTextNorm)
{	
	this->clrNormalBG = clrNormalBG;
	this->clrSelected1	= clrSelected1;
	this->clrSelected2	= clrSelected2;
	this->clrTextSel	= clrTextSel;
	this->clrTextNorm	= clrTextNorm;
	ReloadImages();
}

void DriveListBox::SetImages(HINSTANCE hInstance, int bgndResId, int driveResId)
{
	this->hInstance = hInstance;
	this->bgndResId = bgndResId;
	this->driveResId = driveResId;
	ReloadImages();
}

HWND DriveListBox::GetHWND(void)
{
	return m_hwnd;
}

void DriveListBox::ReloadImages(void)
{
	DestroyImages();
	if (!hInstance) return;
	
	HBITMAP bmpBck = NULL, bmpDrive = NULL;
	if (bgndResId) 
	{
		bmpBck = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(bgndResId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
		if (bmpBck) bmpBck = PatchBitmapColors24(bmpBck, clrSelected1, clrSelected2, Filter1);
	}
	if (driveResId)
	{
		bmpDrive = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(driveResId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	}
	
	CreateBitmaps(bmpBck, bmpDrive);
	if (bmpBck) DeleteObject(bmpBck);
	if (bmpDrive) DeleteObject(bmpDrive);
}

void DriveListBox::CreateBitmaps(HBITMAP bmpBck, HBITMAP bmpDrive)
{
	if (rcItem.right == 0 || rcItem.bottom == 0) return;

	HBITMAP bmpDriveMask = NULL;
	if (bmpDrive) bmpDriveMask = CreateBitmapMask(bmpDrive, RGB(255, 0, 255));

	HDC hdc = GetDC(m_hwnd);
	HBITMAP bmp;	
	
	for (int i = 0; i < 2; i++)
	{
		bmp = CreateCompatibleBitmap(hdc, rcItem.right, rcItem.bottom);
		HDC memDstDC = CreateCompatibleDC (hdc);
		HDC memSrcDC = CreateCompatibleDC (hdc);
		HBITMAP obmp1 = (HBITMAP)SelectObject(memDstDC, bmp);
		HBITMAP obmp2 = (HBITMAP)SelectObject(memSrcDC, bmpBck);
		
		if (i == 0 )
		{
			for (int i = 0; i < rcItem.right; i++)
			{
				BitBlt(memDstDC, i, 0, 2, rcItem.bottom, memSrcDC, 0, 0, SRCCOPY); 
			}
		}
		else
		{
			HBRUSH hb = CreateSolidBrush(clrNormalBG);
			FillRect(memDstDC, &rcItem, hb);
			DeleteObject(hb);
		}
	
		BITMAP bm;
		GetObject(bmpDrive, sizeof(BITMAP), &bm);
		RECT r1 = {	
						max(2, (rcItem.right - bm.bmWidth) / 2), 
						max(2, (rcItem.bottom - 16 - bm.bmHeight) / 2),
						min(rcItem.right - 4, bm.bmWidth), 
						min(rcItem.bottom -18, bm.bmHeight) 
					};
		SelectObject(memSrcDC, bmpDriveMask);
		BitBlt(memDstDC, r1.left, r1.top, r1.right, r1.bottom, memSrcDC, 0,0, SRCAND);

		SelectObject(memSrcDC, bmpDrive);
   		BitBlt(memDstDC, r1.left, r1.top, r1.right, r1.bottom, memSrcDC, 0,0, 	SRCPAINT);
		
		SelectObject(memDstDC, obmp1);
		SelectObject(memSrcDC, obmp2);

		DeleteDC(memDstDC);
		DeleteDC(memSrcDC);
		if (i == 0) bmpSelected = bmp;
		else bmpNormal = bmp;
	}

	ReleaseDC(m_hwnd, hdc); 
	DeleteObject(bmpDriveMask);
}

void DriveListBox::Init(HWND hwnd)
{	
	m_hwnd = hwnd;
	m_parentHwnd = GetParent(hwnd);
}

int DriveListBox::HandleMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DRAWITEM:
			if (wParam == (WPARAM)controlId) DrawItem((DRAWITEMSTRUCT *)lParam);
			break;
		case WM_MEASUREITEM:
			if (wParam == (WPARAM)controlId && MeasureItem((LPMEASUREITEMSTRUCT)lParam)) 	return 1;
			break;
	
		case WM_CTLCOLORLISTBOX:
			SetBkColor((HDC)wParam, clrNormalBG);
			return NULL;
	}
	return FALSE;
}

void DriveListBox::DrawItem(LPDRAWITEMSTRUCT di)
{
	if(di->CtlType == ODT_LISTBOX)
	{
		if (di->itemID == -1) return;
		
		RECT r;
		r=di->rcItem;

		if (!bmpSelected || !bmpNormal || ((r.right  - r.left) != rcItem.right))
		{
			SetRect(&rcItem, 0,0,r.right - r.left, rcItem.bottom);
			ReloadImages();
		}

		HBITMAP bmp;
		int color;
		if (di->itemState & ODS_SELECTED)
		{
			bmp = bmpSelected;
			color = clrTextSel;
		}
		else
		{
			bmp = bmpNormal;
			color = clrTextNorm;
		}

		RECT rc;
		GetClientRect(di->hwndItem, &rc);
	
		HRGN rgnW = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
	//	HRGN rgn = CreateRoundRectRgn(r.left +2, r.top + 2, r.right - 2, r.bottom -2, 10, 8);
		HRGN rgn = CreateRectRgn(r.left +2, r.top + 2, r.right - 2, r.bottom -2);
		
		CombineRgn(rgn, rgn, rgnW, RGN_AND);
		SelectClipRgn(di->hDC, rgn); 
		DeleteObject(rgn);
		DeleteObject(rgnW);
		
		if (bmp)
		{
			HDC hdcbmp = CreateCompatibleDC(di->hDC);
			HBITMAP obmp = (HBITMAP)SelectObject(hdcbmp,bmp);
			StretchBlt(di->hDC, 
						r.left,
						r.top, 
						rcItem.right,
						rcItem.bottom,
						hdcbmp,
						0,
						0,
						rcItem.right,
						rcItem.bottom,
						SRCCOPY);
			SelectObject(hdcbmp,obmp);
			DeleteDC(hdcbmp);
		}
		
		InflateRect(&r, -2, -2);
		
		if ( (di->itemState & ODS_SELECTED) && GetFocus() == di->hwndItem) 
		{
			DrawFocusRect(di->hDC, &r);
		}
			
		SetBkMode(di->hDC, TRANSPARENT);
		SetTextColor(di->hDC, color);
		
		RECT textRect = {r.left + 2, r.bottom - 20, r.right - 2, r.bottom - 6};

		wchar_t str[256] = {0};
		INT nType;
		nType = 0xFFFF & ((DWORD)di->itemData >> 16);
		StringCchPrintfW(str, 256, WASABI_API_LNGSTRINGW(IDS_X_DRIVE_X), 
						 (nType) ? Drive_GetTypeString(nType) : L"",
						 (CHAR)(0xFF & di->itemData));
		DrawTextW(di->hDC, str,-1,&textRect,DT_BOTTOM|DT_SINGLELINE|DT_CENTER);
	}
	return;
}

int DriveListBox::MeasureItem(LPMEASUREITEMSTRUCT mi)
{
	mi->itemHeight = rcItem.bottom;
	return 1;
}