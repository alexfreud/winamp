#include "main.h"
#include "./commandLink.h"
#include "./api.h"
//#include "./guiObjects.h"

#include <windows.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <strsafe.h>

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86


#define MARGIN_LEFT		2
#define MARGIN_TOP		0
#define MARGIN_RIGHT		2
#define MARGIN_BOTTOM	1

typedef struct __COMMANDLINK
{
	COLORREF	rgbBk;
	COLORREF	rgbText;
	COLORREF	rgbTextVisited;
	COLORREF	rgbTextHighlight;
	HCURSOR		cursorHot;
	HFONT		textFont;
	UINT		state;
	RECT		margins;
} COMMANDLINK;

#define GetCommandLink(__hwnd) ((COMMANDLINK*)(LONG_PTR)(LONGX86)GetWindowLongPtrW((__hwnd), 0))

static LRESULT CALLBACK CommandLink_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


EXTERN_C BOOL CommandLink_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW  wc;
	ATOM klassAtom;

	if (GetClassInfoExW(hInstance, NWC_COMMANDLINKW, &wc)) 
		return TRUE;

	ZeroMemory(&wc, sizeof(WNDCLASSEXW));

	wc.cbSize        = sizeof(WNDCLASSEXW);
	wc.hInstance     = hInstance;
	wc.lpszClassName = NWC_COMMANDLINKW;
	wc.lpfnWndProc   = CommandLink_WindowProc;
	wc.style         = CS_GLOBALCLASS;
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.cbWndExtra    = sizeof(COMMANDLINK*);
	
	klassAtom = RegisterClassExW(&wc);
	if (0 == klassAtom)
		return FALSE;

	if (NULL != WASABI_API_APP)
		WASABI_API_APP->DirectMouseWheel_RegisterSkipClass(klassAtom);

	return TRUE;
}


static COLORREF CommandLink_BlendColors(COLORREF rgbTop, COLORREF rgbBottom, INT alpha)
{
	if (alpha > 254) return rgbTop;
	if (alpha < 0) return rgbBottom;

	INT k = (((255 - alpha)*255 + 127)/255);
	
	return RGB( (GetRValue(rgbTop)*alpha + k*GetRValue(rgbBottom) + 127)/255, 
				(GetGValue(rgbTop)*alpha + k*GetGValue(rgbBottom) + 127)/255, 
				(GetBValue(rgbTop)*alpha + k*GetBValue(rgbBottom) + 127)/255);
}

static HBRUSH CommandLink_GetPaintColors(HWND hwnd, HDC hdc, COLORREF *rgbBkOut, COLORREF *rgbFgOut)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	if (NULL == link) return NULL;
	
	COLORREF rgbBk, rgbFg;
	UINT windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
	BOOL defaultColors = (0 != (CLS_DEFAULTCOLORS & windowStyle));

	rgbBk = (defaultColors) ? GetSysColor(COLOR_3DFACE) : link->rgbBk;
	if (NULL != rgbBkOut) *rgbBkOut = rgbBk;

	if (0 != (WS_DISABLED & windowStyle))
		rgbFg = GetSysColor(COLOR_GRAYTEXT);
	else
	{	
		if (0 != (CLIS_VISITED & link->state))
			rgbFg = (defaultColors) ? RGB(85, 26, 139) : link->rgbTextVisited;
		else
			rgbFg = (defaultColors) ? GetSysColor(COLOR_HOTLIGHT) : link->rgbText;

		if (0 != ((CLIS_HOT | CLIS_PRESSED) & link->state))
		{
			if (0 == (CLS_HIGHLIGHTCOLOR & windowStyle))
				rgbFg = CommandLink_BlendColors(rgbFg, rgbBk, 160);
			else
				rgbFg = link->rgbTextHighlight;
		}
	}

	if (NULL != rgbFgOut)
		*rgbFgOut = rgbFg;
	

	HBRUSH backBrush = NULL;
	if (FALSE != defaultColors)
	{
		HWND hParent = GetParent(hwnd);
		if (NULL != hParent)
		{
			COLORREF backBk = SetBkColor(hdc, rgbBk);
			COLORREF backFg = SetBkColor(hdc, rgbFg);
			backBrush = (HBRUSH)SendMessageW(hParent, WM_CTLCOLORSTATIC, (WPARAM)hdc, (LPARAM)hwnd);
			rgbBk = SetBkColor(hdc, backBk);
			SetBkColor(hdc, backFg);
		}
	}
	return backBrush;
}

static BOOL CommandLink_GetTextRect(HDC hdc, LPCWSTR pszText, INT cchText, const RECT *prcClient, RECT *prcTextOut, const RECT *margins)
{
	if (NULL == prcClient || NULL == prcTextOut)
		return FALSE;

	SIZE textSize;
	if (0 == cchText || !GetTextExtentPoint32W(hdc, pszText, cchText, &textSize))
		return SetRectEmpty(prcTextOut);
		
	prcTextOut->left = ((prcClient->right - prcClient->left) - textSize.cx) / 2;
	if (prcTextOut->left < (prcClient->left + margins->left)) 
		prcTextOut->left = prcClient->left + margins->left;
	
	prcTextOut->top = ((prcClient->bottom - prcClient->top) - textSize.cy) / 2;
	if (prcTextOut->top < (prcClient->top + margins->top)) 
		prcTextOut->top = prcClient->top + margins->top;

	prcTextOut->right = prcTextOut->left + textSize.cx;
	if (prcTextOut->right > (prcClient->right - margins->right))
		prcTextOut->right = prcClient->right - margins->right;

	prcTextOut->bottom = prcTextOut->top + textSize.cy;
	if (prcTextOut->bottom > (prcClient->bottom - margins->bottom))
		prcTextOut->bottom = prcClient->bottom - margins->bottom;
	
	return TRUE;
}

static BOOL CommandLink_GetIdealSizeReal(HWND hwnd, SIZE *sizeOut)
{
	if (NULL == sizeOut)
		return FALSE;
	
	ZeroMemory(sizeOut, sizeof(SIZE));

	COMMANDLINK *link = GetCommandLink(hwnd);
	if (NULL == link) return FALSE;

	WCHAR szText[256] = {0};
	INT cchText = GetWindowTextW(hwnd, szText, ARRAYSIZE(szText));
	
	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_WINDOW | DCX_NORESETATTRS);
	if (NULL == hdc) return FALSE;

	HFONT originalFont = (HFONT)SelectObject(hdc, link->textFont);
	BOOL resultOk;

	if (0 == cchText)
	{
		TEXTMETRICW tm;
		resultOk = GetTextMetricsW(hdc, &tm);
		if (resultOk) sizeOut->cy = tm.tmHeight;
	}
	else
		resultOk = GetTextExtentPoint32W(hdc, szText, cchText, sizeOut);
	
	if (originalFont != link->textFont) SelectObject(hdc, originalFont);
	ReleaseDC(hwnd, hdc);

	if (resultOk)
	{
		sizeOut->cx += (link->margins.left + link->margins.right);
		sizeOut->cy += (link->margins.top + link->margins.bottom);
	}
	return resultOk;
}

static BOOL CommandLink_SetStateEx(HWND hwnd, UINT newState, UINT stateMask, BOOL fRedraw)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	if (NULL == link) return FALSE;
	
	UINT oldState = link->state;
	link->state = (link->state & ~stateMask) | (newState & stateMask); 

	if (oldState != link->state && fRedraw)
		InvalidateRect(hwnd, NULL, TRUE);
	
	return TRUE;
}

static void CommandLink_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	if (NULL == link) return;

	RECT clientRect, partRect;
	if (!GetClientRect(hwnd, &clientRect)) return;

	DWORD windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);

	COLORREF rgbBk, rgbFg;
	HBRUSH backBrush = CommandLink_GetPaintColors(hwnd, hdc, &rgbBk, &rgbFg);

	COLORREF originalBk = SetBkColor(hdc, rgbBk);
	COLORREF originalFg = SetTextColor(hdc, rgbFg);

	HFONT originalFont = (HFONT)SelectObject(hdc, link->textFont);

	WCHAR szText[256] = {0};
	INT cchText = GetWindowTextW(hwnd, szText, ARRAYSIZE(szText));

	RECT textRect;
	CommandLink_GetTextRect(hdc, szText, cchText, &clientRect, &textRect, &link->margins);

	if (fErase)
	{
		if(NULL != backBrush)
		{
			FillRect(hdc, prcPaint, backBrush);
		}
		else
		{
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prcPaint, NULL, 0, NULL);
		}
	}

	if (0 != ((CLIS_HOT | CLIS_PRESSED) & link->state) ||
		0 != (CLS_ALWAYSUNDERLINE & windowStyle))
	{		
		TEXTMETRIC tm;
		if (GetTextMetrics(hdc, &tm))
		{
			CopyRect(&partRect, &textRect);
			partRect.top = partRect.top + tm.tmAscent + 1;
			partRect.bottom = partRect.top + 1;
				
			COLORREF originalColor = SetBkColor(hdc, rgbFg);
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &partRect, NULL, 0, NULL);
			if (originalColor != rgbFg) SetBkColor(hdc, originalColor);
		}
	}

	if (0 != cchText)
	{	
		INT originalMode = SetBkMode(hdc, TRANSPARENT);
		UINT originalAlign = SetTextAlign(hdc, TA_LEFT | TA_TOP);
		ExtTextOutW(hdc, textRect.left, textRect.top, ETO_CLIPPED, &textRect, szText, cchText, NULL);
		if (TRANSPARENT != originalMode) SetBkMode(hdc, originalMode);
		if ((TA_LEFT | TA_TOP) != originalAlign) SetTextAlign(hdc, originalAlign);
	}

	if (CLIS_FOCUSED == ((CLIS_FOCUSED | CLIS_HIDEFOCUS) & link->state))
	{
		SetBkColor(hdc, 0x00FFFFFF);
		SetTextColor(hdc, 0x000000000);
		DrawFocusRect(hdc, &clientRect);
	}

	SelectObject(hdc, originalFont);
	if (originalBk != rgbBk) SetBkColor(hdc, originalBk);
	if (originalFg != rgbFg) SetTextColor(hdc, originalFg);

}

static void CommandLink_NotifyParent(HWND hwnd, UINT notificationId)
{
	NMHDR nmhdr;
	nmhdr.code = notificationId;
	nmhdr.hwndFrom = hwnd;
	nmhdr.idFrom = GetDlgCtrlID(hwnd);

	HWND hParent = GetParent(hwnd);
	if (NULL != hParent)
	{
		SendMessageW(hParent, WM_NOTIFY, (WPARAM)nmhdr.idFrom, (LPARAM)&nmhdr);
	}
}

static void CommandLink_Click(HWND hwnd)
{
	CommandLink_NotifyParent(hwnd, NM_CLICK);
	
	DWORD windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
	if (0 != (CLS_TRACKVISITED & windowStyle))
		CommandLink_SetStateEx(hwnd, CLIS_VISITED, CLIS_VISITED, TRUE);

}

static void CommandLink_QueryUiState(HWND hwnd)
{
	UINT uiState = (UINT)DefWindowProcW(hwnd, WM_QUERYUISTATE, 0, 0L);
	CommandLink_SetStateEx(hwnd, (0 != (UISF_HIDEFOCUS & uiState)) ? CLIS_HIDEFOCUS : 0, CLIS_HIDEFOCUS, TRUE);
}
static LRESULT CommandLink_OnCreateWindow(HWND hwnd, CREATESTRUCT *pcs)
{	
	UNREFERENCED_PARAMETER(pcs);

	COMMANDLINK *link;

	link = (COMMANDLINK*)calloc(1, sizeof(COMMANDLINK));
	if (NULL == link)
	{
		DestroyWindow(hwnd);
		return -1;
	}

	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtrW(hwnd, 0, (LONGX86)(LONG_PTR)link) && ERROR_SUCCESS != GetLastError())
	{
		free(link);
		DestroyWindow(hwnd);
		return -1;
	}

	link->rgbBk = GetSysColor(COLOR_3DFACE);
	link->rgbText = GetSysColor(COLOR_HOTLIGHT);
	link->rgbTextVisited = (0x00FFFFFF & (~link->rgbText));
	link->rgbTextHighlight = GetSysColor(COLOR_HOTLIGHT);
	link->cursorHot = LoadCursor(NULL, IDC_HAND);
	SetRect(&link->margins, MARGIN_LEFT, MARGIN_TOP, MARGIN_RIGHT, MARGIN_BOTTOM);

	CommandLink_QueryUiState(hwnd);
	return FALSE;
}

static void CommandLink_OnDestroy(HWND hwnd)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	SetWindowLongPtrW(hwnd, 0, 0L);
	if (!link) return;
	
	free(link);
}

static void CommandLink_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right)
			CommandLink_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void CommandLink_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	CommandLink_Paint(hwnd, hdc, &clientRect, 0 != (PRF_ERASEBKGND & options));
}

static void CommandLink_OnSetFocus(HWND hwnd, HWND lostFocus)
{
	UNREFERENCED_PARAMETER(lostFocus);
	CommandLink_SetStateEx(hwnd, CLIS_FOCUSED, CLIS_FOCUSED, TRUE);
	CommandLink_NotifyParent(hwnd, NM_SETFOCUS);
}

static void CommandLink_OnKillFocus(HWND hwnd, HWND receiveFocus)
{
	UNREFERENCED_PARAMETER(receiveFocus);
	CommandLink_SetStateEx(hwnd, 0, CLIS_FOCUSED, TRUE);
	CommandLink_NotifyParent(hwnd, NM_KILLFOCUS);
}

static void CommandLink_OnMouseMove(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	UNREFERENCED_PARAMETER(mouseFlags);
	UNREFERENCED_PARAMETER(pts);

	UINT windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);

	COMMANDLINK *link = GetCommandLink(hwnd);
	if (NULL != link)
	{
		if (0 == (CLIS_HOT & link->state) && 0 != (CLS_HOTTRACK & windowStyle))
		{
			link->state |= CLIS_HOT;
			InvalidateRect(hwnd, NULL, FALSE);

			TRACKMOUSEEVENT tm;
			tm.cbSize = sizeof(TRACKMOUSEEVENT);
			tm.dwFlags = TME_LEAVE;
			tm.hwndTrack = hwnd;
			TrackMouseEvent(&tm);
		}
	}
}

static void CommandLink_OnMouseLeave(HWND hwnd)
{
	CommandLink_SetStateEx(hwnd, 0, CLIS_HOT, TRUE);
}

static void CommandLink_OnLButtonDown(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	UNREFERENCED_PARAMETER(mouseFlags);
	UNREFERENCED_PARAMETER(pts);

	if (IsWindowEnabled(hwnd) && hwnd != GetFocus())
		SetFocus(hwnd);
	
	CommandLink_SetStateEx(hwnd, CLIS_PRESSED, CLIS_PRESSED, TRUE);
	if (hwnd != GetCapture())
		SetCapture(hwnd);

}

static void CommandLink_OnLButtonUp(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	UNREFERENCED_PARAMETER(mouseFlags);

	RECT clientRect;
	POINT pt;
	POINTSTOPOINT(pt, pts);

	COMMANDLINK *link = GetCommandLink(hwnd);
	if (NULL != link && 0 != (CLIS_PRESSED & link->state))
	{
		if (GetClientRect(hwnd, &clientRect) && PtInRect(&clientRect, pt))
			CommandLink_Click(hwnd);
		CommandLink_SetStateEx(hwnd, 0, CLIS_PRESSED, TRUE);
	}

	if (hwnd == GetCapture())
		SetCapture(NULL);
}

static void CommandLink_OnKeyDown(HWND hwnd, UINT virtualKey, UINT keyFlags)
{
	UNREFERENCED_PARAMETER(hwnd);
	UNREFERENCED_PARAMETER(virtualKey);
	UNREFERENCED_PARAMETER(keyFlags);
	
}

static void CommandLink_OnKeyUp(HWND hwnd, UINT virtualKey, UINT keyFlags)
{
	UNREFERENCED_PARAMETER(keyFlags);

	switch(virtualKey)
	{
		case VK_SPACE:
			CommandLink_Click(hwnd);
			break;
	}

}

static void CommandLink_OnEnable(HWND hwnd, BOOL fEnabled)
{
	UNREFERENCED_PARAMETER(fEnabled);
    InvalidateRect(hwnd, NULL, TRUE);
}


static void CommandLink_OnUpdateUiState(HWND hwnd, UINT actionId, UINT stateId)
{
	DefWindowProcW(hwnd, WM_UPDATEUISTATE, MAKEWPARAM(actionId, stateId), 0L);
	CommandLink_QueryUiState(hwnd);
}

static void CommandLink_OnSetFont(HWND hwnd, HFONT newFont, BOOL fRedraw)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	if (NULL != link) link->textFont = newFont;

	if (FALSE != fRedraw)
		InvalidateRect(hwnd, NULL, TRUE);
}

static LRESULT CommandLink_OnGetFont(HWND hwnd)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	return (LRESULT)((NULL != link) ? link->textFont : NULL);
}

static LRESULT CommandLink_OnSetCursor(HWND hwnd, HWND cursorWindow, UINT hitTest, UINT uMsg)
{	
	UNREFERENCED_PARAMETER(cursorWindow);
	UNREFERENCED_PARAMETER(hitTest);
	UNREFERENCED_PARAMETER(uMsg);

	if (IsWindowEnabled(hwnd))
	{
		COMMANDLINK *link = GetCommandLink(hwnd);
		if (NULL != link && NULL != link->cursorHot)
		{
			SetCursor(link->cursorHot);
			return TRUE;
		}
	}
	return FALSE;
}

static LRESULT CommandLink_OnGetIdealHeight(HWND hwnd)
{
	SIZE windowSize;
	return (CommandLink_GetIdealSizeReal(hwnd, &windowSize)) ? windowSize.cy : 0;
}

static LRESULT CommandLink_OnGetIdealSize(HWND hwnd, SIZE *sizeOut)
{
	return CommandLink_GetIdealSizeReal(hwnd, sizeOut);
}

static void CommandLink_OnResetVisited(HWND hwnd)
{
	CommandLink_SetStateEx(hwnd, 0, CLIS_VISITED, TRUE);
}


static BOOL CommandLink_OnGetMargins(HWND hwnd, RECT *prc)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	if (NULL == prc || NULL == link) return FALSE;
	return CopyRect(prc, &link->margins);
}

static BOOL CommandLink_OnSetMargins(HWND hwnd, const RECT *prc)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	if (NULL == prc || NULL == link) return FALSE;
	return CopyRect(&link->margins, prc);
}

static BOOL CommandLink_OnSetBackColor(HWND hwnd, COLORREF rgb)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	if (NULL == link) return FALSE;
	
	link->rgbBk = rgb;
	return TRUE;
}

static BOOL CommandLink_OnGetBackColor(HWND hwnd)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	return (NULL != link) ? link->rgbBk : RGB(255, 0, 255);
}

static BOOL CommandLink_OnSetTextColor(HWND hwnd, COLORREF rgb)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	if (NULL == link) return FALSE;
	
	link->rgbText = rgb;
	return TRUE;
}

static BOOL CommandLink_OnGetTextColor(HWND hwnd)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	return (NULL != link) ? link->rgbText : RGB(255, 0, 255);
}

static BOOL CommandLink_OnSetVisitedColor(HWND hwnd, COLORREF rgb)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	if (NULL == link) return FALSE;
	
	link->rgbTextVisited = rgb;
	return TRUE;
}

static BOOL CommandLink_OnGetVisitedColor(HWND hwnd)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	return (NULL != link) ? link->rgbTextVisited : RGB(255, 0, 255);
}

static BOOL CommandLink_OnSetHighlightColor(HWND hwnd, COLORREF rgb)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	if (NULL == link) return FALSE;
	
	link->rgbTextHighlight = rgb;
	return TRUE;
}

static BOOL CommandLink_OnGetHighlightColor(HWND hwnd)
{
	COMMANDLINK *link = GetCommandLink(hwnd);
	return (NULL != link) ? link->rgbTextHighlight : RGB(255, 0, 255);
}

static LRESULT CALLBACK CommandLink_WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_CREATE:
			return CommandLink_OnCreateWindow( hwnd, (CREATESTRUCT *) lParam );
		case WM_DESTROY:
			CommandLink_OnDestroy( hwnd );
			return 0;
		case WM_PAINT:
			CommandLink_OnPaint( hwnd );
			return 0;
		case WM_PRINTCLIENT:
			CommandLink_OnPrintClient( hwnd, (HDC) wParam, (UINT) lParam );
			return 0;
		case WM_ERASEBKGND:
			return 0;
		case WM_SETFONT:
			CommandLink_OnSetFont( hwnd, (HFONT) wParam, (BOOL) LOWORD( lParam ) );
			return 0;
		case WM_GETFONT:
			return CommandLink_OnGetFont( hwnd );
		case WM_SETFOCUS:
			CommandLink_OnSetFocus( hwnd, (HWND) wParam );
			return 0;
		case WM_KILLFOCUS:
			CommandLink_OnKillFocus( hwnd, (HWND) wParam );
			return 0;
		case WM_MOUSEMOVE:
			CommandLink_OnMouseMove( hwnd, (UINT) wParam, MAKEPOINTS( lParam ) );
			return 0;
		case WM_MOUSELEAVE:
			CommandLink_OnMouseLeave( hwnd );
			return 0;
		case WM_LBUTTONDOWN:
			CommandLink_OnLButtonDown( hwnd, (UINT) wParam, MAKEPOINTS( lParam ) );
			return 0;
		case WM_LBUTTONUP:
			CommandLink_OnLButtonUp( hwnd, (UINT) wParam, MAKEPOINTS( lParam ) );
			return 0;
		case WM_KEYDOWN:
			CommandLink_OnKeyDown( hwnd, (UINT) wParam, (UINT) lParam );
			return 0;
		case WM_KEYUP:
			CommandLink_OnKeyUp( hwnd, (UINT) wParam, (UINT) lParam );
			return 0;
		case WM_ENABLE:
			CommandLink_OnEnable( hwnd, (BOOL) wParam );
			return 0;
		case WM_UPDATEUISTATE:
			CommandLink_OnUpdateUiState( hwnd, LOWORD( wParam ), HIWORD( wParam ) );
			return 0;
		case WM_SETCURSOR:
			return CommandLink_OnSetCursor( hwnd, (HWND) wParam, LOWORD( lParam ), HIWORD( lParam ) );

		case CLM_GETIDEALHEIGHT:
			return CommandLink_OnGetIdealHeight( hwnd );
		case CLM_GETIDEALSIZE:
			return CommandLink_OnGetIdealSize( hwnd, (SIZE *) lParam );
		case CLM_RESETVISITED:
			CommandLink_OnResetVisited( hwnd );
			return 0;
		case CLM_GETMARGINS:
			return CommandLink_OnGetMargins( hwnd, (RECT *) lParam );
		case CLM_SETMARGINS:
			return CommandLink_OnSetMargins( hwnd, (const RECT *) lParam );
		case CLM_SETBACKCOLOR:
			return CommandLink_OnSetBackColor( hwnd, (COLORREF) lParam );
		case CLM_GETBACKCOLOR:
			return CommandLink_OnGetBackColor( hwnd );
		case CLM_SETTEXTCOLOR:
			return CommandLink_OnSetTextColor( hwnd, (COLORREF) lParam );
		case CLM_GETTEXTCOLOR:
			return CommandLink_OnGetTextColor( hwnd );
		case CLM_SETVISITEDCOLOR:
			return CommandLink_OnSetVisitedColor( hwnd, (COLORREF) lParam );
		case CLM_GETVISITEDCOLOR:
			return CommandLink_OnGetVisitedColor( hwnd );
		case CLM_SETHIGHLIGHTCOLOR:
			return CommandLink_OnSetHighlightColor( hwnd, (COLORREF) lParam );
		case CLM_GETHIGHLIGHTCOLOR:
			return CommandLink_OnGetHighlightColor( hwnd );
	}

	return DefWindowProcW( hwnd, uMsg, wParam, lParam );
}
