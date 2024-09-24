#include "main.h"
#include ".\infoBox.h"


MLInfoBox::MLInfoBox(void)
{
	oldWndProc	= NULL;
	m_hwnd = NULL;
	bodyBrush = NULL;
	headerBrush = NULL;
	headerText[0] = 0;

	SetColors(RGB(0,0,0), RGB(255,255,255), RGB(0,60,0));
	
	SetRect(&rcBody, 0,0,0,0);
	
	drawHeader = TRUE;
	SetRect(&rcHeader, 0,0,0,20); // default height

	headerFont = NULL;

}
MLInfoBox::~MLInfoBox(void)
{
	SetWindowLong(m_hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)oldWndProc);
	oldWndProc	= NULL;

	if (headerBrush) DeleteObject(headerBrush);
	headerBrush = NULL;
	
	if (bodyBrush) DeleteObject(bodyBrush);
	bodyBrush = NULL;

	if (headerFont) DeleteObject(headerFont);
	headerFont = NULL;

}

void MLInfoBox::SetColors(COLORREF bodyBG, COLORREF headerFG, COLORREF headerBG)
{
	this->bodyBG = bodyBG;
	this->headerFG = headerFG;
	this->headerBG = headerBG;

	if (headerBrush) DeleteObject(headerBrush);
	headerBrush = NULL;
	headerBrush = CreateSolidBrush(headerBG);
	
	if (bodyBrush) DeleteObject(bodyBrush);
	bodyBrush = NULL;
	bodyBrush = CreateSolidBrush(bodyBG);

}

void MLInfoBox::Init(HWND hwnd)
{
	m_hwnd = hwnd;
	
	HDC hdc = GetDC(hwnd);
	long lfHeight;
    lfHeight = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	headerFont = CreateFontW(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Arial");
	ReleaseDC(hwnd, hdc);

	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONGX86)(LONG_PTR)this);
	oldWndProc= (WNDPROC)(LONG_PTR)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)newWndProc);
	RECT rc;
	GetWindowRect(hwnd,  &rc);
	SetSize(rc.right - rc.left, rc.bottom - rc.top);
}

void MLInfoBox::SetSize(int cx, int cy)
{
	int offset = 0;
	if (drawHeader) 				
	{
		SetRect(&rcHeader, 0,0, cx, rcHeader.bottom);
		offset = rcHeader.bottom;
	}
	SetRect(&rcBody, 0, offset, cx, cy);
}
LRESULT CALLBACK MLInfoBox::newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	MLInfoBox *box = (MLInfoBox*)(LONG_PTR)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	
	switch(uMsg)
	{
		case WM_SIZE:
			if (SIZE_MINIMIZED != wParam)
			{
				box->SetSize(LOWORD(lParam), HIWORD(lParam));
			}
			break;
		case WM_ERASEBKGND:
			{
				HDC hdc = GetDC(hwndDlg);
				SetTextColor(hdc, box->headerFG);
				SetBkColor(hdc, box->headerBG);
				RECT  txtRect;
				SetRect(&txtRect, box->rcHeader.left + 8, box->rcHeader.top + 2, box->rcHeader.right -2, box->rcHeader.bottom -2);
				HFONT oldFont = (HFONT)SelectObject(hdc, box->headerFont);
				GetWindowTextW(hwndDlg, box->headerText, CAPTION_LENGTH);
				DrawTextW(hdc, box->headerText, -1, &txtRect, DT_VCENTER | DT_LEFT | DT_SINGLELINE);
				SelectObject(hdc, oldFont);
				ReleaseDC(hwndDlg, hdc);
			}
			return TRUE;


			break;
		case WM_PAINT:
			{
				PAINTSTRUCT pt;
				HDC hdc = BeginPaint(hwndDlg, &pt);
				RECT drawRect ;
				if(box->drawHeader && IntersectRect(&drawRect, &box->rcHeader, &pt.rcPaint))
				{
					FillRect(hdc, &drawRect, box->headerBrush);
	
					SetTextColor(hdc, box->headerFG);
					SetBkColor(hdc, box->headerBG);
					SetRect(&drawRect, box->rcHeader.left + 8, box->rcHeader.top + 2, box->rcHeader.right -2, box->rcHeader.bottom -2);
					HFONT oldFont = (HFONT)SelectObject(hdc, box->headerFont);
					GetWindowTextW(hwndDlg, box->headerText, CAPTION_LENGTH);
					DrawTextW(hdc, box->headerText, -1, &drawRect, DT_VCENTER | DT_LEFT | DT_SINGLELINE);
					SelectObject(hdc, oldFont);
					ValidateRect(hwndDlg, &drawRect);	
				}

	
				if(IntersectRect(&drawRect, &box->rcBody, &pt.rcPaint))
				{
					FillRect(hdc, &drawRect, box->bodyBrush);
					ValidateRect(hwndDlg, &drawRect);	
				}
						
				EndPaint(hwndDlg, &pt);
			}
			break;
	}

	return CallWindowProc(box->oldWndProc, hwndDlg, uMsg, wParam, lParam);
}

	