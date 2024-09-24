#include "./skinnedlistbox.h"
#include "../winamp/wa_dlg.h"
#include "./skinning.h"
#include "../nu/trace.h"
#include <strsafe.h>

SkinnedListbox::SkinnedListbox(void) : SkinnedScrollWnd(FALSE)
{
}

SkinnedListbox::~SkinnedListbox(void)
{
}

BOOL SkinnedListbox::Attach(HWND hwndListbox)
{
	DWORD ws;

	if(!SkinnedScrollWnd::Attach(hwndListbox)) return FALSE;
	
	ws = GetWindowLongPtrW(hwnd, GWL_STYLE);

	SetType(SKINNEDWND_TYPE_LISTBOX);
	SetMode((LBS_COMBOBOX & ws) ? SCROLLMODE_COMBOLBOX : SCROLLMODE_STANDARD);

	if (0 == (LBS_COMBOBOX & ws))
	{
		HWND hwndParent = GetParent(hwndListbox);
		if (hwndParent) SkinWindow(hwndParent, SWS_NORMAL); 
	}

	if (LBS_DISABLENOSCROLL & ws) DisableNoScroll(TRUE);
	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	return TRUE;
}

HPEN SkinnedListbox::GetBorderPen(void)
{
	if (LBS_COMBOBOX & GetWindowLongPtrW(hwnd, GWL_STYLE))
	{
		return (HPEN)MlStockObjects_Get(MENUBORDER_PEN);
	}
	else return __super::GetBorderPen();
}

void SkinnedListbox::OnDrawItem(DRAWITEMSTRUCT *pdis)
{
	static wchar_t szText[512];	
	if (pdis->itemID == -1) return;
	INT cchLen = (INT)SendMessageW(pdis->hwndItem, LB_GETTEXT, pdis->itemID, (LPARAM)szText);
	DrawItem(pdis, szText, cchLen);
}

void SkinnedListbox::DrawItem(DRAWITEMSTRUCT *pdis, LPCWSTR pszText, INT cchText)
{
	RECT ri;
	COLORREF rgbTextOld, rgbBkOld;

	if (pdis->itemID == -1) return;

	CopyRect(&ri, &pdis->rcItem);

	if (ODA_FOCUS == pdis->itemAction)
	{
		if (0 == (0x0200/*ODS_NOFOCUSRECT*/ & pdis->itemState))
		{
			rgbTextOld = SetTextColor(pdis->hDC, 0x000000);
			rgbBkOld = SetBkColor(pdis->hDC, 0xFFFFFF);
			DrawFocusRect(pdis->hDC, &ri);
			SetTextColor(pdis->hDC, rgbTextOld);
			SetBkColor(pdis->hDC, rgbBkOld);
		}
		return;
	}

	if (ODS_SELECTED & pdis->itemState)
	{
		BOOL bActive = (pdis->hwndItem == GetFocus());
		if (!bActive && (LBS_COMBOBOX & GetWindowLongPtrW(pdis->hwndItem, GWL_STYLE))) bActive = TRUE;

		rgbTextOld = SetTextColor(pdis->hDC, WADlg_getColor((bActive) ? WADLG_SELBAR_FGCOLOR : WADLG_INACT_SELBAR_FGCOLOR));
		rgbBkOld = SetBkColor(pdis->hDC, WADlg_getColor((bActive) ? WADLG_SELBAR_BGCOLOR : WADLG_INACT_SELBAR_BGCOLOR));
	}
	else
	{
		rgbTextOld = GetTextColor(pdis->hDC);
		rgbBkOld = GetBkColor(pdis->hDC);
	}

	if (cchText> 0)
	{
		//InflateRect(&ri, -4, -1);
		//int mode = SetBkMode(pdis->hDC, TRANSPARENT);

		//DrawTextW(pdis->hDC, pszText, cchText, &ri, DT_NOPREFIX | DT_VCENTER | DT_NOCLIP);
		//if (mode != TRANSPARENT) SetBkMode(pdis->hDC, mode);
		ExtTextOutW(pdis->hDC, ri.left + 4, ri.top + 1, ETO_OPAQUE, &ri, pszText, cchText, NULL);
	}
	else if ((ODA_SELECT | ODA_DRAWENTIRE) & pdis->itemAction || (ODS_SELECTED & pdis->itemState)) 
	{
		ExtTextOutW(pdis->hDC, 0, 0, ETO_OPAQUE, &ri, L"", 0, NULL);
	}

	if (ODS_SELECTED & pdis->itemState)
	{
		SetTextColor(pdis->hDC, rgbTextOld);
		SetBkColor(pdis->hDC, rgbBkOld);
	}
}

void SkinnedListbox::MeasureItem(HWND hwnd, LPCWSTR pszText, INT cchText, UINT *pWidth, UINT *pHeight)
{
	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	HFONT hf, hfo;

	hf = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0L);
	if (NULL == hf) hf = (HFONT) MlStockObjects_Get(DEFAULT_FONT);
	hfo = (HFONT)SelectObject(hdc, hf);

	SIZE sz;
	if (!GetTextExtentPoint32W(hdc, pszText, cchText, &sz))
		ZeroMemory(&sz, sizeof(SIZE));

	if (pWidth) *pWidth = sz.cx;
	if (pHeight) *pHeight= sz.cy;

	SelectObject(hdc, hfo);
	ReleaseDC(hwnd, hdc);
}

void SkinnedListbox::OnSkinUpdated(BOOL bNotifyChildren, BOOL bRedraw)
{
	__super::OnSkinUpdated(bNotifyChildren, bRedraw);

	if (SWS_USESKINFONT & style)
	{
		if (0 == (LBS_OWNERDRAWVARIABLE & GetWindowLongPtrW(hwnd, GWL_STYLE)))
		{
			HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
			HFONT hf, hfo;
			TEXTMETRIC tm;
			hf = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0L);
			if (NULL == hf) hf = (HFONT)MlStockObjects_Get(DEFAULT_FONT);
			hfo = (HFONT)SelectObject(hdc, hf);
			GetTextMetrics(hdc, &tm);
			SendMessageW(hwnd, LB_SETITEMHEIGHT, 0, tm.tmHeight + 2);
			SelectObject(hdc, hfo);
			ReleaseDC(hwnd, hdc);
		}
	}
}

void SkinnedListbox::OnPrint(HDC hdc, UINT options)
{
	if ((PRF_CHECKVISIBLE & options) && !IsWindowVisible(hwnd)) return;

	if (LBS_COMBOBOX & GetWindowLongPtrW(hwnd, GWL_STYLE))
	{
		__super::OnPrint(hdc, options & ~PRF_CLIENT);
		if (((PRF_CLIENT | PRF_ERASEBKGND) & options))
		{
			OffsetViewportOrgEx(hdc, 1, 1, NULL);
			SendMessageW(hwnd, WM_PRINTCLIENT, (WPARAM)hdc, (LPARAM)((PRF_CLIENT | PRF_ERASEBKGND) & options));
			OffsetViewportOrgEx(hdc, -1, -1, NULL);
		}
		return;
	}
	__super::OnPrint(hdc, options);
}

LRESULT SkinnedListbox::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (SWS_USESKINCOLORS & style)
	{
		switch(uMsg)
		{
			case WM_ERASEBKGND: 
				if (LBS_OWNERDRAWFIXED & GetWindowLongPtrW(hwnd, GWL_STYLE))
				{
					RECT rc;
					GetClientRect(hwnd, &rc);
					HBRUSH hb = (HBRUSH)MlStockObjects_Get(ITEMBCK_BRUSH);
					INT count = GetListBoxInfo(hwnd);

					if (count > 0)
					{
						RECT ri;
						if (LB_ERR != SendMessageW(hwnd, LB_GETITEMRECT, count -1, (LPARAM)&ri))
						{
							__super::WindowProc(uMsg, 0, lParam);
							if (ri.bottom < rc.bottom)
							{
								rc.top = ri.bottom;
								FillRect((HDC)wParam, &rc, hb);
							}
							if (ri.right < rc.right)
							{
								rc.top = 0;
								rc.left = ri.right;
								FillRect((HDC)wParam, &rc, hb);
							}
							return 1;
						}
					}
					else
					{
						__super::WindowProc(uMsg, 0, lParam);
						FillRect((HDC)wParam, &rc, hb);
						return 1;
					}
				}
				break;
			case WM_SETFOCUS:
			case WM_KILLFOCUS:
				InvalidateRect(hwnd, NULL, TRUE);
				UpdateWindow(hwnd);
				break;

			case REFLECTED_DRAWITEM: 
				if ((LBS_OWNERDRAWFIXED | LBS_HASSTRINGS) == 
				((LBS_OWNERDRAWFIXED | LBS_NODATA | LBS_HASSTRINGS) & GetWindowLongPtrW(hwnd, GWL_STYLE)))
				{
					OnDrawItem((DRAWITEMSTRUCT*)((REFLECTPARAM*)lParam)->lParam);
					((REFLECTPARAM*)lParam)->result = TRUE;
					return TRUE;
				}
				return FALSE;

			case REFLECTED_CTLCOLORLISTBOX:
				{
					COLORREF rgbText, rgbTextBk;
					rgbText = WADlg_getColor(WADLG_ITEMFG);
					rgbTextBk = WADlg_getColor(WADLG_ITEMBG);

					if(!IsWindowEnabled(hwnd))
					{
						rgbText = RGB((GetRValue(rgbText)+GetRValue(rgbTextBk))/2,
									(GetGValue(rgbText)+GetGValue(rgbTextBk))/2,
									(GetBValue(rgbText)+GetBValue(rgbTextBk))/2);
					}

					SetBkColor((HDC)wParam, rgbTextBk);
					SetTextColor((HDC)wParam, rgbText);
				}
				((REFLECTPARAM*)lParam)->result = (LRESULT)MlStockObjects_Get(ITEMBCK_BRUSH);
				return TRUE;
		}
	}
	return __super::WindowProc(uMsg, wParam, lParam);
}