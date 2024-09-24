#include "main.h"
#include "./skinnedtooltip.h"
#include "./skinning.h"
#include "../winamp/wa_dlg.h"
#include "./colors.h"
#include <strsafe.h>

SkinnedToolTip::SkinnedToolTip(void) 
	: SkinnedWnd(FALSE), skinCursor(NULL), buffer(NULL), bufferSizeCch(0)
{
}

SkinnedToolTip::~SkinnedToolTip(void)
{
	if (NULL != buffer)
		free(buffer);
}

BOOL SkinnedToolTip::Attach(HWND hToolTip)
{
	if(!__super::Attach(hToolTip)) return FALSE;
	
	SetType(SKINNEDWND_TYPE_TOOLTIP);
	return TRUE;
}

HPEN SkinnedToolTip::GetBorderPen(void)
{
	return (HPEN)MlStockObjects_Get(TOOLTIPBORDER_PEN);
}

void SkinnedToolTip::OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw)
{
	if (0 != (SWS_USESKINCOLORS & style))
	{
		HRESULT hr;
		COLORREF rgbText, rgbBk;

		hr = MLGetSkinColor(MLSO_TOOLTIP, TTP_BACKGROUND, MBS_NORMAL, &rgbBk);
		
		if (SUCCEEDED(hr))
			hr = MLGetSkinColor(MLSO_TOOLTIP, TTP_TEXT, MBS_NORMAL, &rgbText);
		
		if (SUCCEEDED(hr))
		{			
			if (rgbBk != (COLORREF)CallPrevWndProc(TTM_GETTIPBKCOLOR, 0, 0L)) 
				CallPrevWndProc(TTM_SETTIPBKCOLOR, rgbBk, 0L);

			if (rgbText != (COLORREF)CallPrevWndProc(TTM_GETTIPTEXTCOLOR, 0, 0L)) 
				CallPrevWndProc(TTM_SETTIPTEXTCOLOR, rgbText, 0L);
		}
	}

	skinCursor = (0 != (SWS_USESKINCURSORS & style)) ? 
					(HCURSOR)SENDWAIPC(plugin.hwndParent, IPC_GETSKINCURSORS, WACURSOR_NORMAL) : NULL;

	HFONT hfOld = (HFONT)CallPrevWndProc(WM_GETFONT, 0, 0L);

	__super::OnSkinChanged(bNotifyChildren, bRedraw);

	if (hfOld != (HFONT)CallPrevWndProc(WM_GETFONT, 0, 0L))
		CallPrevWndProc(TTM_UPDATE, 0, 0L);
}

void SkinnedToolTip::OnPaint()
{
	BOOL defaultPaint = TRUE;
	DWORD windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (0 == ((WS_BORDER | WS_DLGFRAME | WS_THICKFRAME) & windowStyle))
	{
		DWORD windowExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
		if (0 == ((WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME) & windowExStyle))
			defaultPaint = FALSE;
	}

	if (FALSE != defaultPaint)
	{
		CallPrevWndProc(WM_PAINT, 0, 0L);
		return;
	}
	 
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	if (NULL == hdc) return;

	COLORREF rgbBk;
	rgbBk = (COLORREF)CallPrevWndProc(TTM_GETTIPBKCOLOR, 0, 0L);
	SetBkColor(hdc, rgbBk);

	RECT rc, rcText;
	GetClientRect(hwnd, &rc);
	
	if (FALSE != ps.fErase)
	{
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, NULL, 0, NULL);
		DrawBorder(hdc, &rc, BORDER_FLAT, GetBorderPen());
	}

	unsigned int textLength = (unsigned int)CallPrevWndProc(WM_GETTEXTLENGTH, 0, 0L);
	if (0 != textLength)
	{
		if (textLength >= bufferSizeCch)
		{
			if (NULL != buffer)
				free(buffer);
			bufferSizeCch = textLength + 1;
			buffer = (wchar_t*)calloc(bufferSizeCch, sizeof(wchar_t));
			if (NULL == buffer)
				bufferSizeCch = 0;
		}

		if (NULL != buffer)
			textLength = (long)CallPrevWndProc(WM_GETTEXT, (WPARAM)bufferSizeCch, (LPARAM)buffer);
		else
			textLength = 0;

		COLORREF rgbFg = (COLORREF)CallPrevWndProc(TTM_GETTIPTEXTCOLOR, 0, 0L);

		SetRectEmpty(&rcText);
		CallPrevWndProc(TTM_GETMARGIN, 0, (LPARAM)&rcText);
		
		if (rcText.left < 2)
			rcText.left = 2;
		
		if (rcText.right < 2)
			rcText.right = 2;

		if (rcText.bottom < 1)
			rcText.bottom = 1;

		if (rcText.top < 1)
			rcText.top = 1;

		rcText.left = rc.left + rcText.left;
		rcText.top = rc.top + rcText.top;
		rcText.right = rc.right - rcText.right;
		rcText.bottom = rc.bottom - rcText.bottom;
		
		HFONT textFont = (HFONT)CallPrevWndProc(WM_GETFONT, 0, 0L);
		if (NULL == textFont) 
		textFont = (HFONT)MlStockObjects_Get(DEFAULT_FONT);

		HFONT textFontOld = (HFONT)SelectObject(hdc, textFont);
		COLORREF rgbFgOld = SetTextColor(hdc, rgbFg);
		
		unsigned int textFormat;

		textFormat = DT_TOP | DT_LEFT | DT_WORDBREAK;
		if (0 != (TTS_NOPREFIX & windowStyle))
			textFormat |= DT_NOPREFIX;
				

		DrawTextW(hdc, buffer, textLength, &rcText, textFormat);
		
		SelectObject(hdc, textFontOld);
		if (rgbFg != rgbFgOld)
			SetTextColor(hdc, rgbFgOld);
	}

	EndPaint(hwnd, &ps);
}

LRESULT SkinnedToolTip::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_ERASEBKGND:
			return 0;
		case WM_PAINT:
			OnPaint();
			return 0;
		case WM_WINDOWPOSCHANGED: 
			if (0 != (SWP_SHOWWINDOW & ((WINDOWPOS*)lParam)->flags))
				SkinChanged(FALSE, TRUE);
			break;
		case WM_SHOWWINDOW: 
			if (0 != wParam)
				SkinChanged(FALSE, TRUE);
			break;
		case WM_SETCURSOR:
			if (NULL != skinCursor)
			{
				if (skinCursor != GetCursor())
					SetCursor(skinCursor);
				return TRUE;
			}
			break;
	}
	return __super::WindowProc(uMsg, wParam, lParam);
}