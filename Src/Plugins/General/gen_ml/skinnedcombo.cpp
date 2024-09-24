#include "./skinnedcombo.h"
#include "./skinnedheader.h"
#include "../winamp/wa_dlg.h"
#include "./skinning.h"
#include "../nu/trace.h"
#include <windowsx.h>
#include <strsafe.h>

#define COMBO_TEXT_MAX	512

SkinnedCombobox::SkinnedCombobox(void) : SkinnedWnd(FALSE), activeBorder(TRUE)
{
}

SkinnedCombobox::~SkinnedCombobox(void)
{
}

BOOL SkinnedCombobox::Attach(HWND hwndCombo)
{
	if(!SkinnedWnd::Attach(hwndCombo)) return FALSE;
	SetType(SKINNEDWND_TYPE_COMBOBOX);
	activeBorder = TRUE;
	COMBOBOXINFO cbi;
	ZeroMemory(&cbi, sizeof(COMBOBOXINFO));
	cbi.cbSize =  sizeof(COMBOBOXINFO);
	if (GetComboBoxInfo(hwnd, &cbi))
	{
		if (NULL != cbi.hwndItem) SkinWindowEx(cbi.hwndItem, SKINNEDWND_TYPE_EDIT, style);
		if (NULL != cbi.hwndList) 
		{
			SkinWindowEx(cbi.hwndList, SKINNEDWND_TYPE_LISTBOX, style);
		}
	}
	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	return TRUE;
}

BOOL SkinnedCombobox::SetStyle(UINT newStyle, BOOL bRedraw)
{
	BOOL result = __super::SetStyle(newStyle, bRedraw);
	if (hwnd)
	{
		COMBOBOXINFO cbi;
		ZeroMemory(&cbi, sizeof(COMBOBOXINFO));
		cbi.cbSize =  sizeof(COMBOBOXINFO);
		activeBorder = (0 == (SWCBS_TOOLBAR & style));
		if (GetComboBoxInfo(hwnd, &cbi))
		{
			if (NULL != cbi.hwndItem) MLSkinnedWnd_SetStyle(cbi.hwndItem, style);
			if (NULL != cbi.hwndList) MLSkinnedWnd_SetStyle(cbi.hwndList, style);
		}
	}
	return result;
}

BOOL SkinnedCombobox::IsButtonDown(DWORD windowStyle)
{
	if(GetAsyncKeyState((GetSystemMetrics(SM_SWAPBUTTON)) ? VK_RBUTTON : VK_LBUTTON) & 0x8000)
	{
		if (CBS_DROPDOWNLIST == (0x0F & windowStyle)) 
		{
			POINT pt;
			RECT rc;

			if (hwnd == GetFocus() && GetClientRect(hwnd, &rc))
			{
				GetCursorPos(&pt);
				MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
				return PtInRect(&rc, pt);
			}
			return FALSE;
		}

		COMBOBOXINFO cbi;
		ZeroMemory(&cbi, sizeof(COMBOBOXINFO));
		cbi.cbSize =  sizeof(COMBOBOXINFO);

		if (GetComboBoxInfo(hwnd, &cbi))
		{
			//check if in arrow down area
			POINT pt;
			GetCursorPos(&pt);
			MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
			return PtInRect(&cbi.rcButton, pt);
		}
	}
	return FALSE;
}

void SkinnedCombobox::DrawButton(HDC hdc, RECT *prcButton, BOOL bPressed, BOOL bActive)
{
	COLORREF rgbBkOld, rgbBk;

	rgbBk = WADlg_getColor(WADLG_LISTHEADER_BGCOLOR);
	rgbBkOld = SetBkColor(hdc, rgbBk);
	ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, prcButton, NULL, 0, 0);

	if (bActive)
	{
		HPEN pen, penOld;
		pen = (HPEN)MlStockObjects_Get((bPressed) ? HEADERBOTTOM_PEN : HEADERTOP_PEN);
		penOld = (HPEN)SelectObject(hdc, pen);

		MoveToEx(hdc, prcButton->left, prcButton->top, NULL);
		LineTo(hdc, prcButton->right, prcButton->top);
		MoveToEx(hdc, prcButton->left, prcButton->top, NULL);
		LineTo(hdc, prcButton->left, prcButton->bottom);

		if (!bPressed)
		{
			SelectObject(hdc, penOld);
			pen = (HPEN)MlStockObjects_Get(HEADERBOTTOM_PEN);
			penOld = (HPEN)SelectObject(hdc, pen);
		}

		MoveToEx(hdc, prcButton->right - 1, prcButton->top, NULL);
		LineTo(hdc, prcButton->right - 1, prcButton->bottom);
		MoveToEx(hdc, prcButton->right - 1, prcButton->bottom - 1, NULL);
		LineTo(hdc, prcButton->left - 1, prcButton->bottom - 1);

		if (!bPressed)
		{
			SelectObject(hdc, penOld);

			pen = (HPEN)MlStockObjects_Get(HEADERMIDDLE_PEN);
			penOld = (HPEN)SelectObject(hdc, pen);

			MoveToEx(hdc, prcButton->right - 2, prcButton->top + 1, NULL);
			LineTo(hdc, prcButton->right - 2, prcButton->bottom - 2);
			MoveToEx(hdc, prcButton->right - 2, prcButton->bottom - 2, NULL);
			LineTo(hdc, prcButton->left, prcButton->bottom - 2);
		}
		SelectObject(hdc, penOld);
	}

	RECT r;
	DWORD arrowSize = SkinnedHeader::GetSortArrowSize();

	SetRect(&r, 0, 0, GET_X_LPARAM(arrowSize), GET_Y_LPARAM(arrowSize));

	OffsetRect(&r, 
		prcButton->left + (prcButton->right - prcButton->left - r.right)/2  + (prcButton->right - prcButton->left - r.right)%2, 
		prcButton->top + (prcButton->bottom - prcButton->top - r.bottom)/2);
	if (bPressed) OffsetRect(&r, 1, 1);
	if (r.left > (prcButton->left + 1) && r.top > (prcButton->top + 1))
	{
		SkinnedHeader::DrawSortArrow(hdc, &r, rgbBk, WADlg_getColor(WADLG_LISTHEADER_FONTCOLOR), FALSE);
	}

	SetBkColor(hdc, rgbBkOld);
}

void SkinnedCombobox::OnPaint()
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rc;
	COMBOBOXINFO cbi;
	BOOL ctrlActive;

	if (!GetClientRect(hwnd, &rc) ||  rc.bottom <= rc.top || rc.right <= rc.left) return;

	hdc= BeginPaint(hwnd, &ps);
	if (NULL == hdc) return;

	ctrlActive = (activeBorder /*|| hwnd == GetFocus()*/);

	if (SWCBS_TOOLBAR & style)
	{
		DrawBorder(hdc, &rc, BORDER_FLAT, (HPEN)MlStockObjects_Get(WNDBCK_PEN));
		if (ctrlActive)
		{
			InflateRect(&rc, -1, -1);
			DrawBorder(hdc, &rc, BORDER_FLAT, __super::GetBorderPen());
		}
	}
	else DrawBorder(hdc, &rc, BORDER_FLAT, GetBorderPen());

	InflateRect(&rc, -1, -1);

	ZeroMemory(&cbi, sizeof(COMBOBOXINFO));
	cbi.cbSize =  sizeof(COMBOBOXINFO);
	if (GetComboBoxInfo(hwnd, &cbi))
	{
		RECT r;

		DWORD ws = GetWindowLongPtrW(hwnd, GWL_STYLE);

		SetBkMode(hdc, OPAQUE);
		SetBkColor(hdc, WADlg_getColor(WADLG_ITEMBG));

		SetRect(&r, rc.left, rc.top, cbi.rcItem.left, rc.bottom);
		if (r.left < r.right) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &r, L"", 0, NULL);
		SetRect(&r, cbi.rcItem.left, rc.top, cbi.rcItem.right, cbi.rcItem.top);
		if (r.top < r.bottom) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &r, L"", 0, NULL);
		SetRect(&r, cbi.rcItem.left, cbi.rcItem.bottom, cbi.rcItem.right, rc.bottom);
		if (r.top < r.bottom) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &r, L"", 0, NULL);

		if (cbi.rcButton.left != cbi.rcButton.right)
		{
			SetRect(&r, cbi.rcItem.right, rc.top, cbi.rcButton.left, rc.bottom);
			if (r.left < r.right) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &r, L"", 0, NULL);
			SetRect(&r, cbi.rcButton.right, rc.top, rc.right, rc.bottom);

			if (r.left < r.right) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &r, L"", 0, NULL);
			SetRect(&r, cbi.rcButton.left, rc.top, cbi.rcButton.right, cbi.rcButton.top);
			if (r.top < r.bottom) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &r, L"", 0, NULL);
			SetRect(&r, cbi.rcButton.left, cbi.rcButton.bottom, cbi.rcButton.right, rc.bottom);
			if (r.top < r.bottom) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &r, L"", 0, NULL);

			DrawButton(hdc, &cbi.rcButton, IsButtonDown(ws), ctrlActive);
		}
		else
		{
			SetRect(&r, cbi.rcItem.right, rc.top, rc.right, rc.bottom);
			if (r.left < r.right) ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &r, L"", 0, NULL);
		}

		if (CBS_DROPDOWNLIST == (0x0F & ws))
		{
			INT cchText = (INT)SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0);
			if (cchText)
			{
				HFONT hFont, hFontOld;
				wchar_t szText[COMBO_TEXT_MAX] = {0};

				SendMessageW(hwnd, WM_GETTEXT, (WPARAM)COMBO_TEXT_MAX, (LPARAM)szText);
				hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
				if (!hFont) hFont = (HFONT)MlStockObjects_Get(DEFAULT_FONT);
				hFontOld = (hFont) ? (HFONT)SelectObject(hdc, hFont) : NULL;

				COLORREF rgbText;
				BOOL bFocused = (hwnd == GetFocus() && !SendMessageW(hwnd, CB_GETDROPPEDSTATE, 0, 0L));
				if  (bFocused && 0 == (SWCBS_TOOLBAR & style))
				{
					rgbText = WADlg_getColor(WADLG_SELBAR_FGCOLOR);
					SetBkColor(hdc, WADlg_getColor(WADLG_SELBAR_BGCOLOR));
				}
				else rgbText = WADlg_getColor(WADLG_ITEMFG);
				if(!IsWindowEnabled(hwnd))
				{
					COLORREF rgbBack = GetBkColor(hdc);
					rgbText = RGB((GetRValue(rgbText)+GetRValue(rgbBack))/2,
						(GetGValue(rgbText)+GetGValue(rgbBack))/2,
						(GetBValue(rgbText)+GetBValue(rgbBack))/2);
				}
				SetTextColor(hdc, rgbText);
				ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &cbi.rcItem, L"", 0, NULL);
				if (bFocused && 
					0 == (0x01/*UISF_HIDEFOCUS*/ & uiState) &&
					0 == (SWCBS_TOOLBAR & style))
				{
					COLORREF fg, bk;
					fg = SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
					bk = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
					DrawFocusRect(hdc, &cbi.rcItem);
					SetTextColor(hdc, fg);
					SetBkColor(hdc, bk);
				}

				InflateRect(&cbi.rcItem, -1, -1);
				DrawTextW(hdc, szText, min(cchText,COMBO_TEXT_MAX), &cbi.rcItem, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
				if (hFontOld) SelectObject(hdc, hFontOld);
			}
			else 
			{
				ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &cbi.rcItem, L"", 0, NULL);
			}
		}
	}

	EndPaint(hwnd, &ps);
}

void SkinnedCombobox::OnSkinUpdated(BOOL bNotifyChildren, BOOL bRedraw)
{
	__super::OnSkinUpdated(bNotifyChildren, bRedraw);

	if (SWS_USESKINFONT & style)
	{
		if (0 == (CBS_OWNERDRAWVARIABLE & GetWindowLongPtrW(hwnd, GWL_STYLE)))
		{
			HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
			HFONT hf, hfo;
			TEXTMETRIC tm;
			hf = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0L);
			if (NULL == hf) hf = (HFONT)MlStockObjects_Get(DEFAULT_FONT);
			hfo = (HFONT)SelectObject(hdc, hf);
			GetTextMetrics(hdc, &tm);
			SendMessageW(hwnd, CB_SETITEMHEIGHT, -1, tm.tmHeight + 2);
			SendMessageW(hwnd, CB_SETITEMHEIGHT, 0, tm.tmHeight + 2);
			SelectObject(hdc, hfo);
			ReleaseDC(hwnd, hdc);

		}
	}
}
INT SkinnedCombobox::OnNcHitTest(POINTS pts)
{
	INT ht = __super::OnNcHitTest(pts);

	if (ht > 0 && !activeBorder && (SWCBS_TOOLBAR & style) && IsChild(GetActiveWindow(), hwnd))
	{
		TRACKMOUSEEVENT track;
		track.cbSize = sizeof(TRACKMOUSEEVENT);
		track.dwFlags = TME_LEAVE;
		track.hwndTrack = hwnd;
		TrackMouseEvent(&track);
		activeBorder = TRUE;
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);
	}
	return ht;
}


void SkinnedCombobox::OnMouseLeave(void)
{
	if (!activeBorder) return;

	COMBOBOXINFO cbi;
	ZeroMemory(&cbi, sizeof(COMBOBOXINFO));
	cbi.cbSize =  sizeof(COMBOBOXINFO);
	if (GetComboBoxInfo(hwnd, &cbi) && IsWindowVisible(cbi.hwndList)) 
		return;
	
	activeBorder = FALSE;
	if (SWCBS_TOOLBAR & style)
	{
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);
	}
}
LRESULT SkinnedCombobox::SilenceMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;
	UpdateWindow(hwnd);
	CallDefWndProc(WM_SETREDRAW, FALSE, 0L);
	result = __super::WindowProc(uMsg, wParam, lParam);
	CallDefWndProc(WM_SETREDRAW, TRUE, 0L);
	InvalidateRect(hwnd, NULL, TRUE);
	return result;
}

static HWND hwndPreviousFocus = NULL;

LRESULT SkinnedCombobox::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (SWCBS_TOOLBAR & style)
	{
		switch(uMsg)
		{
			case WM_SETFOCUS:
				hwndPreviousFocus = (HWND)wParam;
				break;
			case REFLECTED_COMMAND:
				switch(HIWORD(wParam))
				{
					case CBN_CLOSEUP:
						{
							if (NULL != hwndPreviousFocus  && hwnd == GetFocus())
							{
								do 
								{
									if (IsWindowVisible(hwndPreviousFocus) && IsWindowEnabled(hwndPreviousFocus))
									{
										SetFocus(hwndPreviousFocus);
										break;
									}
								} while (NULL != (hwndPreviousFocus = GetAncestor(hwndPreviousFocus, GA_PARENT)));
								
							}
							hwndPreviousFocus = NULL;
						}

						break;

				}
				break;
		}
	}
	if (SWS_USESKINCOLORS & style)
	{
		switch(uMsg)
		{
			case WM_PAINT: OnPaint(); return 0;
			case WM_ERASEBKGND: return 0;
			case WM_MOUSELEAVE: OnMouseLeave(); break;
			 
			case WM_KILLFOCUS:
			case WM_SETFOCUS:
			case WM_COMMAND:
			case WM_CAPTURECHANGED:			
			case 0x0128/*WM_UPDATEUISTATE*/:
				SilenceMessage(uMsg, wParam, lParam);
				return 0;
			case WM_LBUTTONUP:
				if (activeBorder && (SWCBS_TOOLBAR & style))
				{
					TRACKMOUSEEVENT track;
					track.cbSize = sizeof(TRACKMOUSEEVENT);
					track.dwFlags = TME_LEAVE;
					track.hwndTrack = hwnd;
					TrackMouseEvent(&track);

					POINT pt;
					pt.x = GET_X_LPARAM(lParam);
					pt.y = GET_Y_LPARAM(lParam);
					RECT rw;
					if (GetWindowRect(hwnd, &rw))
					{
						MapWindowPoints(hwnd, HWND_DESKTOP, &pt, 1);
						if (!PtInRect(&rw, pt))
						{
							INPUT pi[2];
							ZeroMemory(&pi[0], sizeof(INPUT));
							pi[0].type = INPUT_MOUSE;
							pi[0].mi.dx = pt.x;
							pi[0].mi.dy = pt.y;
							pi[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
							CopyMemory(&pi[1], &pi[0], sizeof(INPUT));
							pi[1].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP;
							SendInput(2, pi, sizeof(INPUT));
						}
					}

				}
				break;
			case WM_LBUTTONDOWN:
				if (!activeBorder && (SWCBS_TOOLBAR & style))
				{
					TRACKMOUSEEVENT track;
					track.cbSize = sizeof(TRACKMOUSEEVENT);
					track.dwFlags = TME_LEAVE;
					track.hwndTrack = hwnd;
					TrackMouseEvent(&track);
					activeBorder = TRUE;
				}
				break;
		}
	}
	return __super::WindowProc(uMsg, wParam, lParam);
}