#include "./skinnedstatic.h"
#include "../winamp/wa_dlg.h"
#include "./skinning.h"
#include <strsafe.h>

#define MARGIN_TOP		2
#define MARGIN_BOTTOM	2
#define MARGIN_LEFT		2
#define MARGIN_RIGHT	2
	
SkinnedStatic::SkinnedStatic(void) : SkinnedWnd(FALSE)
{
}

SkinnedStatic::~SkinnedStatic(void)
{
}

BOOL SkinnedStatic::Attach(HWND hwndStatic)
{
	if(!SkinnedWnd::Attach(hwndStatic)) return FALSE;
	SetType(SKINNEDWND_TYPE_STATIC);
	
	HWND hwndParent = GetParent(hwndStatic);
	if (hwndParent) SkinWindow(hwndParent, SWS_NORMAL); 

	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	return TRUE;
}

LRESULT SkinnedStatic::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (SWS_USESKINCOLORS & style)
	{
		switch(uMsg)
		{
			case REFLECTED_CTLCOLORSTATIC:
				{
					COLORREF rgbText, rgbTextBk;
					rgbText = WADlg_getColor(WADLG_WNDFG);
					rgbTextBk = WADlg_getColor(WADLG_WNDBG);

					if(!IsWindowEnabled(hwnd))
					{		
						rgbText = RGB((GetRValue(rgbText)+GetRValue(rgbTextBk))/2,
									(GetGValue(rgbText)+GetGValue(rgbTextBk))/2,
									(GetBValue(rgbText)+GetBValue(rgbTextBk))/2);
					}
	
					SetBkColor((HDC)wParam, rgbTextBk);
					SetTextColor((HDC)wParam, rgbText);
				}
				((REFLECTPARAM*)lParam)->result = (LRESULT)MlStockObjects_Get(WNDBCK_BRUSH);
				return TRUE;
		}
	}
	return __super::WindowProc(uMsg, wParam, lParam);
}

LRESULT SkinnedStatic::GetIdealSize(LPCWSTR pszText)
{
	INT cchText;
	SIZE szButton;
	szButton.cx = 0;
	szButton.cy = 0;

	cchText = (pszText) ? lstrlenW(pszText) : (INT)SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0L);

	{
		HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE);
		if (hdc)
		{
			wchar_t szText[STATIC_TEXT_MAX] = {0};
			if (NULL == pszText) 
			{
				SendMessageW(hwnd, WM_GETTEXT, (WPARAM)STATIC_TEXT_MAX, (LPARAM)szText);
				pszText = szText;
			}

			HFONT hFont = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0L);
			if (NULL == hFont) hFont = (HFONT)MlStockObjects_Get(DEFAULT_FONT);
			HFONT hfo = (NULL != hFont) ? (HFONT)SelectObject(hdc, hFont) : NULL;

			if (0 != cchText)
			{
				RECT rt;
				SetRect(&rt, 0, 0, 0, 0);
				if (FALSE == DrawTextW(hdc, pszText, cchText, &rt, DT_CALCRECT | DT_SINGLELINE))
				{
					szButton.cx = 0;
					szButton.cy = 0;
				}
				else
				{
					szButton.cx = rt.right - rt.left;
					szButton.cy = rt.bottom - rt.top;
				}
			}
			else
			{
				TEXTMETRIC metrics;

				szButton.cx = 0;
				if (FALSE == GetTextMetrics(hdc, &metrics))
					szButton.cy = 0;
				else 
					szButton.cy = metrics.tmHeight;
			}

			if (0 != szButton.cy)
				szButton.cy += (MARGIN_TOP + MARGIN_BOTTOM);

			if (0 != szButton.cx)
				szButton.cx += (MARGIN_LEFT + MARGIN_RIGHT) + 2;

			if (NULL != hfo) 
				SelectObject(hdc, hfo);

			ReleaseDC(hwnd, hdc);
		}
	}

	return MAKELPARAM(szButton.cx, szButton.cy);
}

BOOL SkinnedStatic::OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult)
{
	switch(msg)
	{
		case ML_IPC_SKINNEDSTATIC_GETIDEALSIZE:
			*pResult = GetIdealSize((LPCWSTR)param);
			return TRUE;
	}
	return __super::OnMediaLibraryIPC(msg, param, pResult);
}