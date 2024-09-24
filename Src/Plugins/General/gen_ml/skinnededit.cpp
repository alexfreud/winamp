#include "./skinnededit.h"
#include "../winamp/wa_dlg.h"
#include "./skinning.h"
#include "./stockobjects.h"
#include "./skinnedmenu.h"
#include "../nu/trace.h"
#include <windowsx.h>
#include <strsafe.h>


#define EDIT_TEXT_MAX		65536
static WCHAR *pszTextGlobal = NULL;

static int caretPos = -1;
static BOOL updateCaret = FALSE;

#define GET_CHAR_X(__hwnd, __index) (GET_X_LPARAM(CallPrevWndProc(EM_POSFROMCHAR, (WPARAM)(__index), 0L)))

#define MLSEM_FIRST			(WM_APP + 0x2FFE)
#define MLSEM_ENABLEREDRAW	(MLSEM_FIRST - 0)

SkinnedEdit::SkinnedEdit(void) : SkinnedWnd(FALSE), firstVisible(0), lastVisible(0),
								 firstSelected(0), lastSelected(0), maxCharWidth(0),
								 mouseWParam(0), mouseLParam(0), cx(0), cy(0)
{
	if (NULL == pszTextGlobal) 
		pszTextGlobal = (LPWSTR)calloc(EDIT_TEXT_MAX, sizeof(WCHAR));
}

SkinnedEdit::~SkinnedEdit(void)
{
}

BOOL SkinnedEdit::Attach(HWND hwndEdit)
{
	if(!SkinnedWnd::Attach(hwndEdit)) return FALSE;
	SetType(SKINNEDWND_TYPE_EDIT);
	FontChanged();
	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	return TRUE;
}

void SkinnedEdit::EraseBckGnd(HDC hdc, RECT *prc, RECT *prcText, BOOL fEraseAll, HBRUSH hBrush)
{		
	HRGN rgn = CreateRectRgnIndirect(prc);
	if (!IsRectEmpty(prcText))
	{
		HRGN rgn2 = CreateRectRgnIndirect(prcText);
		if (NULL != rgn2)
		{
			CombineRgn(rgn, rgn, rgn2, RGN_DIFF);

		/*	if (FALSE != fEraseAll)
			{	
				SetRectRgn(rgn2, prc->left, prc->top, prc->right, prcText->top);
				CombineRgn(rgn, rgn, rgn2, RGN_DIFF);

				SetRectRgn(rgn2, prc->left, prcText->bottom, prc->right, prc->bottom);
				CombineRgn(rgn, rgn, rgn2, RGN_DIFF);
			}*/
			DeleteObject(rgn2);
		}
	}
	
	FillRgn(hdc, rgn, hBrush);
	DeleteObject(rgn);
}


void SkinnedEdit::DrawText(HDC hdc, RECT *prc, RECT *prcText, LPCWSTR pszText, INT cchText)
{

	if ((lastSelected != firstSelected) && 
		(lastSelected  > firstVisible) && 
		(firstSelected < lastVisible) &&  
		((hwnd == GetFocus()) || (ES_NOHIDESEL & GetWindowLongPtrW(hwnd, GWL_STYLE))))
	{
		RECT rt;
		int lim, limSel;
		LPCWSTR pszTextOut;
		pszTextOut = pszText + firstVisible;
		lim = firstSelected - firstVisible;
		CopyRect(&rt, prcText);
		if (lim > 0) 
		{			
		//	DrawTextW(hdc, pszTextOut, lim, prcText,DT_TOP | DT_EDITCONTROL | DT_NOPREFIX | DT_NOCLIP); 
			ExtTextOutW(hdc, rt.left, rt.top, 0, &rt, pszTextOut, lim, NULL);
			pszTextOut += lim;
		}
				
		limSel = min(lastSelected, lastVisible) - (int)(pszTextOut - pszText);
		lim = lastVisible - lastSelected;
		if(lim > 0)
		{
			rt.left = GET_CHAR_X(hwnd, lastSelected);
			// DrawTextW(hdc, pszTextOut + limSel, lim, &rt, DT_TOP | DT_EDITCONTROL | DT_NOPREFIX | DT_NOCLIP); 
			ExtTextOutW(hdc, rt.left, rt.top, 0, &rt, pszTextOut + limSel, lim, NULL);
		}					
			
		SetTextColor(hdc, WADlg_getColor(WADLG_SELBAR_FGCOLOR));
		SetBkColor(hdc, WADlg_getColor(WADLG_SELBAR_BGCOLOR));
		
		if (lim > 0) rt.right = rt.left - 1;
		rt.left = GET_CHAR_X(hwnd, max(firstSelected, firstVisible));
	//	DrawTextW(hdc, pszTextOut, limSel, &rt, DT_TOP | DT_EDITCONTROL | DT_NOPREFIX | DT_NOCLIP ); 
		if (rt.right > rt.left) ExtTextOutW(hdc, rt.left, rt.top, ETO_CLIPPED, &rt, pszTextOut, limSel, NULL);
	}
	else
	{
	 //  DrawTextW(hdc, pszText + firstVisible, cchText - firstVisible - (cchText - lastVisible), prcText, DT_TOP | DT_NOPREFIX | DT_NOCLIP); 
		ExtTextOutW(hdc, prcText->left, prcText->top, 0, prcText, 
						pszText+ firstVisible, cchText - firstVisible - (cchText - lastVisible), NULL);
	}
}

static void GetEditColors(DWORD windowStyle, BOOL bEnabled, COLORREF *prgbText, COLORREF *prgbTextBk)
{
	COLORREF fg, bg;
	bg = WADlg_getColor((ES_READONLY & windowStyle) ? WADLG_WNDBG : WADLG_ITEMBG);
	fg = WADlg_getColor((ES_READONLY & windowStyle) ? WADLG_WNDFG : WADLG_ITEMFG);
	if(!bEnabled)
	{		
		fg = RGB((GetRValue(fg)+GetRValue(bg))/2,
						(GetGValue(fg)+GetGValue(bg))/2,
						(GetBValue(fg)+GetBValue(bg))/2);
	}
	if (prgbText) *prgbText = fg;
	if (prgbTextBk) *prgbTextBk = bg;
}


void SkinnedEdit::OnPaint()
{
	HDC hdc;
	int cchText;

	PAINTSTRUCT ps;
	RECT rc, rt;
	HFONT hFont, hFontOld;
	DWORD margins, ws;
	TEXTMETRICW tm;
	
		
	cchText = (INT)CallPrevWndProc(WM_GETTEXTLENGTH, 0, 0);
	if (cchText) CallPrevWndProc(WM_GETTEXT, (WPARAM)EDIT_TEXT_MAX, (LPARAM)pszTextGlobal);
	
	hFont = (HFONT)CallPrevWndProc(WM_GETFONT, 0, 0L);
	if (!hFont) hFont = (HFONT)MlStockObjects_Get(DEFAULT_FONT);

	hdc = GetDCEx(hwnd, NULL, DCX_PARENTCLIP | DCX_CACHE | DCX_CLIPSIBLINGS |
				DCX_INTERSECTUPDATE | DCX_VALIDATE);
	if (NULL == hdc) return;

	hFontOld = (hFont) ? (HFONT)SelectObject(hdc, hFont) : NULL;
	GetTextMetricsW(hdc, &tm);
		

	GetClientRect(hwnd, &rc);
	CopyRect(&rt, &rc);
		
	if (SWES_BOTTOM & style) 
	{	
		if ((rt.bottom - tm.tmHeight) > rt.top) rt.top = rt.bottom - tm.tmHeight;
	}
	else if (SWES_VCENTER & style)
	{
		INT t = rc.top + ((rc.bottom - rc.top) - tm.tmHeight)/2;
		if (t > rc.top) rt.top = t;
	}

	ws = GetWindowLongPtrW(hwnd, GWL_STYLE);
		

	margins = (DWORD)CallPrevWndProc(EM_GETMARGINS, 0, 0L);

	
	if (cchText)
	{
		int x;

		CallPrevWndProc(EM_GETSEL, (WPARAM)&firstSelected, (LPARAM)&lastSelected);
		lastVisible = (int)(INT_PTR)CallPrevWndProc(EM_CHARFROMPOS, 0, MAKELPARAM(rc.right - (GET_Y_LPARAM(margins)), 0));
		if ( -1 == lastVisible) lastVisible = cchText;
		firstVisible =(INT)(INT_PTR)CallPrevWndProc(EM_CHARFROMPOS, 0, MAKELPARAM(rc.left + GET_X_LPARAM(margins), 0));
		if (cchText == firstVisible) firstVisible = 0;
		else if (firstVisible > 0) firstVisible++;
		while (12000 < (x = GET_CHAR_X(hwnd, firstVisible)))  firstVisible++;
		rt.left = x;
		if (firstVisible > 0 && rt.left > rc.left + GET_X_LPARAM(margins) + 1)
		{ // we can try to display one more
			int t = GET_CHAR_X(hwnd, firstVisible -1);
			if (t != -1 && t < rc.right) { rt.left = t; firstVisible--; }
		}
	
		if (lastVisible)
		{
			if (lastVisible < cchText -1)
			{
				rt.right = GET_CHAR_X(hwnd, lastVisible);
			}
			else
			{
				INT i = lastVisible - ((cchText == lastVisible && cchText) ? 1 : 0);
				ABC abc;
				if (GetCharABCWidthsW(hdc, pszTextGlobal[i], pszTextGlobal[i], &abc))
					rt.right = abc.abcA + abc.abcB + abc.abcC;
				else
					GetCharWidth32W(hdc, pszTextGlobal[i], pszTextGlobal[i], (INT*)&rt.right);
				rt.right += GET_CHAR_X(hwnd, i);
			}
		}
		else rt.right = rt.left;
		
		if (rt.top + tm.tmHeight < rt.bottom) rt.bottom = rt.top + tm.tmHeight;
		if (rt.right > rc.right - GET_Y_LPARAM(margins)) rt.right = rc.right - GET_Y_LPARAM(margins);
	}
	else 
	{
		firstVisible = 0;
		lastVisible = 0;
		rt.left += GET_X_LPARAM(margins);
		rt.right -= GET_Y_LPARAM(margins);
		if (ES_CENTER & ws) 
		{ 
			rt.left += (rt.right - rt.left)/2;
			rt.right = rt.left;
		}
		else if (ES_RIGHT & ws) rt.left = rt.right;
		else rt.right = rt.left;
	}


	if (FALSE != updateCaret)
	{
		updateCaret = FALSE;
	//	ShowCaret(hwnd);

		if (hwnd == GetFocus())
		{
			INT x;
			if (caretPos >= lastVisible) x = rt.right;
			else if (caretPos <= firstVisible) x = rt.left;
			else x = GET_CHAR_X(hwnd, caretPos);
			
			if (x < rc.left) 
				x = rc.left;
			else 
			{
				INT caretWidth = GetSystemMetrics(SM_CXBORDER);
				if (x + caretWidth > rc.right)
					x = rc.right - caretWidth;
			}
			SetCaretPos(x, rt.top);
		}
	}

	if (hFontOld) SelectObject(hdc, hFontOld);
	ReleaseDC(hwnd, hdc);

	hdc = BeginPaint(hwnd, &ps);
	if (NULL == hdc) return;

	hFontOld = (hFont) ? (HFONT)SelectObject(hdc, hFont) : NULL;
		
	HBRUSH brushBk = NULL;
	BOOL overrideColors = FALSE;

	HWND hParent = GetParent(hwnd);
	if (NULL != hParent)
	{
		UINT uMsg = (0 == ((ES_READONLY | WS_DISABLED) & ws)) ? WM_CTLCOLOREDIT : WM_CTLCOLORSTATIC;
		brushBk = (HBRUSH)SendMessage(hParent, uMsg, (WPARAM)hdc, (LPARAM)hwnd);
		
		HBRUSH stockBursh = GetSysColorBrush( (0 == ((ES_READONLY | WS_DISABLED) & ws)) ? COLOR_WINDOW : COLOR_3DFACE);
		if (NULL == brushBk || stockBursh == brushBk)
			overrideColors = TRUE;
	}

	if (FALSE != overrideColors)
	{
		COLORREF rgbText, rgbTextBk;
		GetEditColors(ws, IsWindowEnabled(hwnd), &rgbText, &rgbTextBk);

		SetBkColor(hdc, rgbTextBk);
		SetTextColor(hdc, rgbText);
		brushBk = (HBRUSH)MlStockObjects_Get((ES_READONLY & ws) ? WNDBCK_BRUSH : ITEMBCK_BRUSH);
	}
			
	
		
	
	IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
	EraseBckGnd(hdc, &rc, NULL, ps.fErase, brushBk);
	if (cchText) 
	{		
		IntersectClipRect(hdc, rc.left + GET_X_LPARAM(margins), rt.top, rc.right - GET_Y_LPARAM(margins), rt.bottom);
		DrawText(hdc, &rc, &rt, pszTextGlobal, cchText);
	}
		
	if (hFontOld) SelectObject(hdc, hFontOld);
	EndPaint(hwnd, &ps);
}

void SkinnedEdit::OnSkinUpdated(BOOL bNotifyChildren, BOOL bRedraw)
{
	__super::OnSkinUpdated(bNotifyChildren, bRedraw);

	if (SWS_USESKINFONT & style)
	{
		CallPrevWndProc(EM_SETMARGINS, (WPARAM)(EC_LEFTMARGIN | EC_RIGHTMARGIN), MAKELPARAM(EC_USEFONTINFO, EC_USEFONTINFO));
	}
}




BOOL SkinnedEdit::GetSelection(SELECTION *selection, INT cchText, const RECT *clientRect)
{
	if (NULL == selection) return FALSE;
	if (0 == cchText)
	{
		selection->first = 0;
		selection->last = 0;
		selection->leftX = clientRect->left;
		selection->rightX = clientRect->left;
		return TRUE;
	}

	CallPrevWndProc(EM_GETSEL, (WPARAM)&selection->first, (LPARAM)&selection->last);

	selection->leftX = GET_CHAR_X(hwnd, selection->first);

	if (-1 == selection->last || cchText == selection->last)
	{
		selection->last = cchText;
		selection->rightX = GET_CHAR_X(hwnd, cchText - 1) + maxCharWidth;
	}
	else
	{	
		selection->rightX = GET_CHAR_X(hwnd, selection->last);
	}
	
	return TRUE;
}

LRESULT SkinnedEdit::OverrideDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;
			
	UINT windowStyle = GetWindowStyle(hwnd);
	if (0 == (WS_VISIBLE & windowStyle))
	{
		result = __super::WindowProc(uMsg, wParam, lParam);
		return result;
	}

	SELECTION selectionOrig;

	RECT ri;
	GetClientRect(hwnd, &ri);
	
	INT lengthOrig = (INT)CallPrevWndProc(WM_GETTEXTLENGTH, 0, 0L);
	INT startXOrig = (lengthOrig > 0) ? GET_CHAR_X(hwnd, 0) : 0;
	GetSelection(&selectionOrig, lengthOrig, &ri);

	SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~ WS_VISIBLE);

	result = __super::WindowProc(uMsg, wParam, lParam);
	
	windowStyle = GetWindowStyle(hwnd);
	if (0 == (WS_VISIBLE & windowStyle))
	{
		windowStyle |= WS_VISIBLE;
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);
	}
	
	if (hwnd == GetFocus())
	{

		INT length = (INT)CallPrevWndProc(WM_GETTEXTLENGTH, 0, 0L);

		SELECTION selection;
		GetSelection(&selection, length, &ri);
		
		if (selectionOrig.first != selection.first || selectionOrig.last != selection.last || lengthOrig != length ) 
		{	
			caretPos = (selectionOrig.last != selection.last) ? selection.last : selection.first;
			if( FALSE == updateCaret)
				updateCaret = TRUE;
			
			if (0 != (WS_VISIBLE & windowStyle))
			{						
				INT startX = (lengthOrig > 0) ? GET_CHAR_X(hwnd, 0) : 0;
				if (lengthOrig != length || startXOrig != startX) 
				{	
					RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_VALIDATE | RDW_INTERNALPAINT | RDW_NOERASE | RDW_ERASENOW | RDW_UPDATENOW | RDW_NOCHILDREN);
				}
				else
				{
					//RECT ri;
					GetClientRect(hwnd, &ri);

					if (selectionOrig.first != selectionOrig.last)
					{
						if (selectionOrig.rightX > selection.leftX && selectionOrig.rightX < selection.rightX)
							selectionOrig.rightX--;

						if (selectionOrig.leftX > selection.leftX && selectionOrig.leftX < selection.rightX)
							selectionOrig.leftX++;
					}

				//	aTRACE_FMT("redraw: (%d, %d) - (%d, %d)\r\n",selectionOrig.leftX, selectionOrig.rightX, selection.leftX, selection.rightX); 
					HRGN rgn1 = CreateRectRgn(selectionOrig.leftX, ri.top, selectionOrig.rightX, ri.bottom);
					HRGN rgn2 = CreateRectRgn(selection.leftX, ri.top, selection.rightX, ri.bottom);
					CombineRgn(rgn1, rgn1, rgn2, (selectionOrig.first != selectionOrig.last) ? RGN_XOR : RGN_OR);

					POINT caretPt;
					if (FALSE != GetCaretPos(&caretPt))
					{
						INT caretWidth = GetSystemMetrics(SM_CXBORDER);
						if (0 == caretWidth) caretWidth = 1;
						SetRectRgn(rgn2, caretPt.x, ri.top, caretPt.x + caretWidth, ri.bottom);
						CombineRgn(rgn1, rgn1, rgn2, RGN_OR);
					}

					RedrawWindow(hwnd, NULL, rgn1, RDW_INVALIDATE | RDW_NOERASE | RDW_ERASENOW | RDW_UPDATENOW);

					INT scrollCX = 0;
					if (selectionOrig.first == selection.first && selectionOrig.leftX != selection.leftX)
					{
						scrollCX = selectionOrig.leftX - selection.leftX;
					}

					if (selectionOrig.last == selection.last && selectionOrig.rightX != selection.rightX)
					{
						scrollCX = selectionOrig.rightX - selection.rightX;
					}
					if (0 != scrollCX)
					{
						if (scrollCX > 0)
							aTRACE_LINE("move on left side");
						else
							aTRACE_LINE("move on right side");
					}
					DeleteObject(rgn1);
					DeleteObject(rgn2);
				}
			}
		}
	}

	return result;
}

void SkinnedEdit::OnWindowPosChanged(WINDOWPOS *pwp)
{	
	
	HRGN updateRgn = CreateRectRgn(0, 0, 0, 0);

	UINT windowStyle = GetWindowStyle(hwnd);
	if (0 != (WS_VISIBLE & windowStyle))
	{
		if (NULL != updateRgn)
		{
			GetUpdateRgn(hwnd, updateRgn, FALSE);
		}
	
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~ WS_VISIBLE);
	}

	__super::WindowProc(WM_WINDOWPOSCHANGED, 0, (LPARAM)pwp);

	if (0 != (WS_VISIBLE & windowStyle))
	{
		windowStyle = GetWindowStyle(hwnd);
		if (0 == (WS_VISIBLE & windowStyle))
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
	}
	
	RECT rc;
	GetClientRect(hwnd, &rc);
	
	if( FALSE == updateCaret)
	{
		updateCaret = TRUE;
//		HideCaret(hwnd);
	}

	if (0 == ((SWP_NOSIZE | SWP_NOREDRAW) & pwp->flags))
	{
		HRGN rgn = NULL;
		if (rc.right - rc.left != cx)
		{
			cx -= GET_Y_LPARAM((DWORD)CallPrevWndProc(EM_GETMARGINS, 0, 0L));
			cx -= maxCharWidth;
			
			LONG l = rc.left;
			rc.left = cx;
			
			if (NULL != rgn) SetRectRgn(rgn, rc.left, rc.top, rc.right, rc.bottom);
			else rgn = CreateRectRgnIndirect(&rc);

			rc.left = l;

		}
		if (rc.bottom - rc.top != cy)
		{	
			LONG t = rc.top;
			rc.top = cy;
			
			if (NULL != rgn) SetRectRgn(rgn, rc.left, rc.top, rc.right, rc.bottom);
			else rgn = CreateRectRgnIndirect(&rc);
			CombineRgn(updateRgn, updateRgn, rgn, RGN_OR);

			rc.top = t;
		}

		if (0 == (SWP_NOREDRAW & pwp->flags))
		{
			if (NULL != updateRgn)
				InvalidateRgn(hwnd, updateRgn, FALSE);
		}

		if (NULL != rgn)
			DeleteObject(rgn);
		
	}
	
	cx = rc.right - rc.left;
	cy = rc.bottom - rc.top;

	if (NULL != updateRgn)
		DeleteObject(updateRgn);
	
}
void SkinnedEdit::FontChanged()
{
	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL != hdc)
	{			
		HFONT  font, fontOrig;
		font = (HFONT)CallPrevWndProc(WM_GETFONT, 0, 0L);
		if (NULL != font) 
			fontOrig = (HFONT)SelectObject(hdc, font);
		
		TEXTMETRICW tm;
		maxCharWidth = (FALSE != GetTextMetricsW(hdc, &tm)) ? tm.tmMaxCharWidth : 0;
			
		if (NULL != font)
			SelectObject(hdc, fontOrig);
		
		ReleaseDC(hwnd, hdc);
	}
}

void SkinnedEdit::OnSetFont(HFONT hFont, BOOL fRedraw)
{
	__super::WindowProc(WM_SETFONT, (WPARAM)hFont, (LPARAM)fRedraw);
	FontChanged();
}

LRESULT SkinnedEdit::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( 0 != (SWES_SELECTONCLICK & style))
	{
		switch(uMsg)
		{
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
				if (hwnd != GetFocus())
				{
					CallPrevWndProc(EM_SETSEL, 0, -1);
					SetFocus(hwnd);
					if (WM_LBUTTONDOWN == uMsg) return 0;
				}
				break;
		}
	}
	
	if (SWS_USESKINCOLORS & style)
	{
		UINT windowStyle = GetWindowStyle(hwnd);
		switch(uMsg)
		{
			case WM_CONTEXTMENU:
				if (IsSkinnedPopupEnabled(FALSE))
				{
					SkinnedMenu sm;
					if (sm.InitializeHook(hwnd, SMS_USESKINFONT, NULL, 0, NULL, 0L))
					{
						__super::WindowProc(uMsg, wParam, lParam);
						InvalidateRect(hwnd, NULL, TRUE);
						UpdateWindow(hwnd);
						return 0;
					}
				}
				break;
			case MLSEM_ENABLEREDRAW:
				windowStyle = GetWindowStyle(hwnd);
				if (0 == (WS_VISIBLE & windowStyle))
				{
					SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
				}
				return 0;
		}

		if (0 == (ES_MULTILINE & windowStyle))
		{
			switch(uMsg)
			{
				case WM_PAINT: OnPaint(); return 0;
				case WM_ERASEBKGND: return 0;
				case WM_SETFONT:	OnSetFont((HFONT)wParam, (BOOL)lParam); return 0;
				
				case WM_MOUSEMOVE:
					if (wParam == mouseWParam && lParam == mouseLParam) return 0;
					mouseWParam = wParam;
					mouseLParam = lParam;
					return (MK_LBUTTON & mouseWParam) ? OverrideDefault(uMsg, wParam, lParam) : 0;
				
				case WM_SETFOCUS:
					caretPos = -1;
					
				case WM_CAPTURECHANGED:
			
					if (0 != (WS_VISIBLE & windowStyle))
						SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

					__super::WindowProc(uMsg, wParam, lParam);
					
					if (0 != (WS_VISIBLE & windowStyle))
					{
						windowStyle = GetWindowStyle(hwnd);
						if (0 == (WS_VISIBLE & windowStyle))
						{
							SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
							InvalidateRect(hwnd, NULL, FALSE);
						}
					}

					if (FALSE == updateCaret)
					{
					//	HideCaret(hwnd);
						updateCaret = TRUE;
					}

					return 0;
					
	

				case WM_UPDATEUISTATE:
				case WM_KILLFOCUS: 
					
					if (0 != (WS_VISIBLE & windowStyle))
						SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

					__super::WindowProc(uMsg, wParam, lParam);
					
					if (0 != (WS_VISIBLE & windowStyle))
					{
						windowStyle = GetWindowStyle(hwnd);
						if (0 == (WS_VISIBLE & windowStyle))
						{
							SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
							RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
						}
					}
					return 0;

				
				case WM_WINDOWPOSCHANGED: 
					OnWindowPosChanged((WINDOWPOS*)lParam);
					return 0;
				case WM_SETTEXT:
					if (OverrideDefault(uMsg, wParam, lParam))
					{
						InvalidateRect(hwnd, NULL, TRUE);
						return TRUE;
					}	
					return FALSE;

				case EM_SETSEL:
					{					
						BOOL recoverRegion = FALSE;
						HRGN windowRegion = CreateRectRgn(0, 0, 0, 0);
						INT result = GetWindowRgn(hwnd, windowRegion);
						if (SIMPLEREGION != result && COMPLEXREGION != result)
						{
							DeleteObject(windowRegion);
							windowRegion = NULL;
						}
						
						if (0 != (WS_VISIBLE & windowStyle))
							SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);
						
						HRGN fakeRegion = CreateRectRgn(0, 0, 0, 0);
						if (0 != SetWindowRgn(hwnd, fakeRegion, FALSE))
							recoverRegion = TRUE;

						INT prevFirst, prevLast, currFirst, currLast;
						CallPrevWndProc(EM_GETSEL, (WPARAM)&prevFirst, (LPARAM)&prevLast);

						__super::WindowProc(uMsg, wParam, lParam);

						CallPrevWndProc(EM_GETSEL, (WPARAM)&currFirst, (LPARAM)&currLast);
						
						if (currFirst != prevFirst || currLast != prevLast)
						{
							if (currLast != prevLast) caretPos = currLast;
							else caretPos = currFirst;
							
							if (FALSE == updateCaret)
							{
//								HideCaret(hwnd);
								updateCaret = TRUE;
							}
						}
			                   
						if (FALSE != recoverRegion)
							SetWindowRgn(hwnd, windowRegion, FALSE);

						if (NULL != windowRegion)
						{
							DeleteObject(windowRegion);
							windowRegion = NULL;
						}

						if (0 != (WS_VISIBLE & windowStyle))
						{
							windowStyle = GetWindowStyle(hwnd);
							if (0 == (WS_VISIBLE & windowStyle))
							{								
								SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
								InvalidateRect(hwnd, NULL, FALSE);
							}
						}
					}
					return 0;
				case WM_LBUTTONUP:
					{
						if (0 != (WS_VISIBLE & windowStyle))
							SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

						__super::WindowProc(uMsg, wParam, lParam);
					
						if (0 != (WS_VISIBLE & windowStyle))
						{
							windowStyle = GetWindowStyle(hwnd);
							if (0 == (WS_VISIBLE & windowStyle))
								SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
							
						}
					}
					return 0;
				case WM_CHAR:
				case WM_KEYDOWN:
				case WM_CUT:
				//case WM_PASTE:
				case WM_CLEAR:
				case WM_UNDO:
				case EM_UNDO:
				case WM_LBUTTONDBLCLK:
				case WM_LBUTTONDOWN:
					return OverrideDefault(uMsg, wParam, lParam);
			}
		}
	}

	switch(uMsg)
	{
		case WM_SETCURSOR:
			if ((HWND)wParam == hwnd && HTCLIENT == LOWORD(lParam))
			{
				HCURSOR cursor;
				cursor = LoadCursor(NULL, IDC_IBEAM);
				if (NULL != cursor)
				{
					SetCursor(cursor);
					return TRUE;
				}
			}
			break;
	}

	return __super::WindowProc(uMsg, wParam, lParam);
}

