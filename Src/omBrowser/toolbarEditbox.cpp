#define OEMRESOURCE

#include "main.h"
#include "./toolbarEditbox.h"
#include "./menu.h"
#include "./ifc_skinhelper.h"

#include "../Plugins/General/gen_ml/ml_ipc_0313.h"

#include <richedit.h>
#include <strsafe.h>

#define NTEF_USERFLAGSMASK	0x00FFFFFF
#define NTEF_UNICODE		0x01000000
#define NTEF_CLICKEDONCE	0x02000000
#define NTEF_SHOWCURSOR		0x04000000
#define NTEF_KEYDOWN		0x08000000
#define NTEF_MENULOOP		0x10000000
#define NTEF_SHOWCARET		0x20000000

typedef struct __TOOLBAREDITBOX
{
	WNDPROC originalProc;
	UINT	flags;
	ToolbarEditboxHost *host;
	DWORD	dblclkTime;
} TOOLBAREDITBOX;

static ATOM TOOLBAREDITBOX_PROP = 0;
static HHOOK mouseHook = NULL;

#define GetEditbox(__hwnd) ((TOOLBAREDITBOX*)GetProp((__hwnd), MAKEINTATOM(TOOLBAREDITBOX_PROP)))

static LRESULT CALLBACK ToolbarEditbox_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT CALLBACK ToolbarEditbox_WordBreakProc(LPWSTR pszText, INT iCurrent, INT cchLen, INT code);
static INT CALLBACK ToolbarEditbox_WordBreakProc2(LPWSTR pszText, INT iCurrent, INT cchLen, INT code);

static void CALLBACK ToolbarEditbox_Uninitialize()
{
	if (0 != TOOLBAREDITBOX_PROP)
	{
		GlobalDeleteAtom(TOOLBAREDITBOX_PROP);
		TOOLBAREDITBOX_PROP = 0;
	}
}


BOOL ToolbarEditbox_AttachWindow(HWND hEditbox, ToolbarEditboxHost *host)
{
	if (!IsWindow(hEditbox)) 
		return  FALSE;

	if (0 == TOOLBAREDITBOX_PROP)
	{
		 TOOLBAREDITBOX_PROP = GlobalAddAtom(L"NullsoftToolbarEditBox");
		 if (0 == TOOLBAREDITBOX_PROP) return  FALSE;
		 Plugin_RegisterUnloadCallback(ToolbarEditbox_Uninitialize);
	}
	
	TOOLBAREDITBOX *editbox = (TOOLBAREDITBOX*)GetProp(hEditbox, MAKEINTATOM(TOOLBAREDITBOX_PROP));
	if (NULL != editbox) return TRUE;
	
	editbox = (TOOLBAREDITBOX*)calloc(1, sizeof(TOOLBAREDITBOX));
	if (NULL == editbox) return FALSE;


	ifc_skinhelper *skinHelper = NULL;
	if (SUCCEEDED(Plugin_GetSkinHelper(&skinHelper)))
	{
	//	skinHelper->SkinControl(hEditbox, SKINNEDWND_TYPE_EDIT, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWES_BOTTOM);
		skinHelper->Release();
	}

	ZeroMemory(editbox, sizeof(TOOLBAREDITBOX));

	if (IsWindowUnicode(hEditbox))
		editbox->flags |= NTEF_UNICODE;

	editbox->host = host;

	editbox->originalProc = (WNDPROC)(LONG_PTR)((0 != (NTEF_UNICODE & editbox->flags)) ? 
						SetWindowLongPtrW(hEditbox, GWLP_WNDPROC, (LONGX86)(LONG_PTR)ToolbarEditbox_WindowProc) : 
						SetWindowLongPtrA(hEditbox, GWLP_WNDPROC, (LONGX86)(LONG_PTR)ToolbarEditbox_WindowProc));

	if (NULL == editbox->originalProc || !SetProp(hEditbox, MAKEINTATOM(TOOLBAREDITBOX_PROP), editbox))
	{
		if (NULL != editbox->originalProc)
		{
			if (0 != (NTEF_UNICODE & editbox->flags))
				SetWindowLongPtrW(hEditbox, GWLP_WNDPROC, (LONGX86)(LONG_PTR)editbox->originalProc);
			else
				SetWindowLongPtrA(hEditbox, GWLP_WNDPROC, (LONGX86)(LONG_PTR)editbox->originalProc);
		}
			
		free(editbox);
		return FALSE;
	}
	SendMessage(hEditbox, EM_SETWORDBREAKPROC, 0, (LPARAM)ToolbarEditbox_WordBreakProc);
	return TRUE;
}

static void ToolbarEditbox_PatchCursor()
{	
	ShowCursor(FALSE);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	ShowCursor(TRUE);

	CURSORINFO cursorInfo;
	cursorInfo.cbSize = sizeof(CURSORINFO);
	if (GetCursorInfo(&cursorInfo) && 
		0 != (CURSOR_SHOWING & cursorInfo.flags))
	{
		POINT pt;
		GetCursorPos(&pt);
		HWND hTarget= WindowFromPoint(pt);
		if (NULL != hTarget)
		{
			UINT hitTest = (UINT)SendMessage(hTarget, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));
			UINT uMsg = (HTCLIENT == hitTest) ? WM_MOUSEMOVE : WM_NCMOUSEMOVE;
			SendMessage(hTarget, WM_SETCURSOR,  (WPARAM)hTarget, MAKELPARAM(hitTest, uMsg));
		}
	}
}


static void ToolbarEditbox_Detach(HWND hwnd)
{	
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	RemoveProp(hwnd, MAKEINTATOM(TOOLBAREDITBOX_PROP));

	ToolbarEditbox_PatchCursor();

	if (NULL != editbox && 0 != (NTEF_SHOWCURSOR & editbox->flags))
	{
		ShowCursor(TRUE);
		editbox->flags &= ~NTEF_SHOWCURSOR;
	}

	if (NULL != mouseHook)
	{
		UnhookWindowsHookEx(mouseHook);
		mouseHook = NULL;
	}

	if (NULL == editbox) 
		return;

	if (NULL != editbox->originalProc)
	{
		if (0 != (NTEF_UNICODE & editbox->flags))
			SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)editbox->originalProc);
		else
			SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)editbox->originalProc);
	}

	free(editbox);
}


static LRESULT ToolbarEditbox_CallOrigWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);

	if (NULL == editbox || NULL == editbox->originalProc)
	{
		return (0 != (NTEF_UNICODE & editbox->flags)) ? 
				DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
				DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}

	return (0 != (NTEF_UNICODE & editbox->flags)) ? 
			CallWindowProcW(editbox->originalProc, hwnd, uMsg, wParam, lParam) : 
			CallWindowProcA(editbox->originalProc, hwnd, uMsg, wParam, lParam);
}

static void ToolbarEditbox_ResetText(HWND hwnd)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox && NULL != editbox->host)
		editbox->host->EditboxResetText(hwnd);
}

static void ToolbarEditbox_NavigateNextCtrl(HWND hwnd, BOOL fForward)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox && NULL != editbox->host)
		editbox->host->EditboxNavigateNextCtrl(hwnd, fForward);
}

static void ToolbarEditbox_AcceptText(HWND hwnd)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox && NULL != editbox->host)
		editbox->host->EditboxAcceptText(hwnd);
}

static BOOL ToolbarEditbox_IsDelimiterChar(WCHAR testChar)
{
	WORD info;
	if (FALSE == GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, &testChar, 1, &info))
		return 0;

	return (0 != ((C1_SPACE | C1_PUNCT | C1_CNTRL | C1_BLANK) & info));
}

static INT ToolbarEditbox_FindLeft(LPCWSTR pszText, INT iCurrent, INT cchLen)
{
	if (iCurrent <= 0) 
		return 0;

	LPCWSTR pszCursor = &pszText[iCurrent];
	BOOL charDelim = ToolbarEditbox_IsDelimiterChar(*pszCursor);
	
	for(;;)
	{
		pszCursor = CharPrev(pszText, pszCursor);
		if (charDelim != ToolbarEditbox_IsDelimiterChar(*pszCursor))
			return (INT)(INT_PTR)(CharNext(pszCursor) - pszText);
		
		if (pszCursor == pszText)
			break;
	}
	return 0;
}

static INT ToolbarEditbox_FindRight(LPCWSTR pszText, INT iCurrent, INT cchLen)
{
	if (iCurrent >= cchLen)
		return cchLen;

	LPCWSTR pszEnd = &pszText[cchLen];
	LPCWSTR pszCursor = &pszText[iCurrent];

	if (iCurrent > 0)
		pszCursor = CharNext(pszCursor);
	
	BOOL charDelim = ToolbarEditbox_IsDelimiterChar(*pszCursor);

	for(;;)
	{
		pszCursor = CharNext(pszCursor);
		if (pszCursor >= pszEnd) 
			break;

		if (charDelim != ToolbarEditbox_IsDelimiterChar(*pszCursor))
			return (INT)(INT_PTR)(pszCursor - pszText);
	}
	return cchLen;
}

static INT ToolbarEditbox_FindWordLeft(LPCWSTR pszText, INT iCurrent, INT cchLen, BOOL fRightCtrl)
{
	if (iCurrent < 2)
		return 0;

	LPCWSTR pszCursor = &pszText[iCurrent];

	if (FALSE == fRightCtrl) 
		pszCursor = CharPrev(pszText, pszCursor);

	BOOL prevCharDelim = ToolbarEditbox_IsDelimiterChar(*pszCursor);
	for(;;)
	{
		pszCursor = CharPrev(pszText, pszCursor);
		if (TRUE == ToolbarEditbox_IsDelimiterChar(*pszCursor))
		{
			if (FALSE == prevCharDelim)
				return (INT)(INT_PTR)(CharNext(pszCursor) - pszText);

			prevCharDelim = TRUE;
		}
		else
			prevCharDelim = FALSE;

		if (pszCursor == pszText)
			break;
	}
	return 0;
}

static INT ToolbarEditbox_FindWordRight(LPCWSTR pszText, INT iCurrent, INT cchLen)
{
	if ( iCurrent >= (cchLen - 1))
		return cchLen;

	LPCWSTR pszEnd = &pszText[cchLen];
	LPCWSTR pszCursor = &pszText[iCurrent];

	BOOL prevCharDelim = ToolbarEditbox_IsDelimiterChar(*pszCursor);

	for(;;)
	{
		pszCursor = CharNext(pszCursor);
		if (pszCursor >= pszEnd) 
			break;

		if (prevCharDelim != ToolbarEditbox_IsDelimiterChar(*pszCursor))
		{
			prevCharDelim = TRUE;
			return (INT)(INT_PTR)(pszCursor - pszText);
		}
		else
			prevCharDelim = FALSE;

	}
	return cchLen;
}

static INT CALLBACK ToolbarEditbox_WordBreakProc(LPWSTR pszText, INT iCurrent, INT cchLen, INT code)
{
	switch(code)
	{
		case WB_ISDELIMITER:	return (iCurrent < 0) ? 0 : ((iCurrent > cchLen) ? (cchLen + 1) : ToolbarEditbox_IsDelimiterChar(pszText[iCurrent]));
		case WB_LEFT:			return ToolbarEditbox_FindLeft(pszText, iCurrent, cchLen);
		case WB_RIGHT:			return ToolbarEditbox_FindRight(pszText, iCurrent, cchLen);
		case WB_MOVEWORDLEFT:	return ToolbarEditbox_FindWordLeft(pszText, iCurrent, cchLen, FALSE);
		case WB_MOVEWORDRIGHT:	return ToolbarEditbox_FindWordRight(pszText, iCurrent, cchLen);
	}
	return 0;
}

static INT CALLBACK ToolbarEditbox_WordBreakProcOverrideLeft(LPWSTR pszText, INT iCurrent, INT cchLen, INT code)
{
	switch(code)
	{
		case WB_LEFT:	return ToolbarEditbox_FindWordLeft(pszText, iCurrent, cchLen, FALSE);
		case WB_RIGHT:	return ToolbarEditbox_FindWordRight(pszText, iCurrent, cchLen);
	}
	return ToolbarEditbox_WordBreakProc(pszText, iCurrent, cchLen, code);
}

static INT CALLBACK ToolbarEditbox_WordBreakProcOverrideRight(LPWSTR pszText, INT iCurrent, INT cchLen, INT code)
{
	switch(code)
	{
		case WB_LEFT:	return ToolbarEditbox_FindWordLeft(pszText, iCurrent, cchLen, TRUE);
		case WB_RIGHT:	return ToolbarEditbox_FindWordRight(pszText, iCurrent, cchLen);
	}
	return ToolbarEditbox_WordBreakProc(pszText, iCurrent, cchLen, code);
}


static void ToolbarEditbox_OnDestroy(HWND hwnd)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox && NULL != editbox->host)
		editbox->host->EditboxDestroyed(hwnd);

	WNDPROC originalProc = (editbox ? editbox->originalProc : NULL);
	BOOL fUnicode = (0 != (NTEF_UNICODE & editbox->flags));

	ToolbarEditbox_Detach(hwnd);

	if (NULL != originalProc)
	{
		if (FALSE != fUnicode) 
			CallWindowProcW(originalProc, hwnd, WM_DESTROY, 0, 0L);
		else
			CallWindowProcA(originalProc, hwnd, WM_DESTROY, 0, 0L);
	}
	
}

static void ToolbarEditbox_OnSetFocus(HWND hwnd, HWND hFocus)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox) 
	{
		editbox->flags &= ~NTEF_CLICKEDONCE;
	}
	ToolbarEditbox_CallOrigWindowProc(hwnd, WM_SETFOCUS, (WPARAM)hFocus, 0L);
}

static void ToolbarEditbox_OnKillFocus(HWND hwnd, HWND hFocus)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox && NULL != editbox->host )
	{
		if (FALSE != editbox->host->EditboxKillFocus(hwnd, hFocus))
			return;
	}

	ToolbarEditbox_CallOrigWindowProc(hwnd, WM_KILLFOCUS, (WPARAM)hFocus, 0L);
}

static LRESULT ToolbarEditbox_OnGetDlgCode(HWND hwnd, INT vKey, MSG* pMsg)
{
	LRESULT result = ToolbarEditbox_CallOrigWindowProc(hwnd, WM_GETDLGCODE, (WPARAM)vKey, (LPARAM)pMsg);
	result |= DLGC_WANTALLKEYS;
	result &= ~DLGC_HASSETSEL;
	return result;
}

		
static LRESULT ToolbarEditbox_OnSetCursor(HWND hwnd, HWND hOwner, INT hitTest, INT messageId)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (HTCLIENT == hitTest && 
		NULL != editbox && 0 == (NTEF_MENULOOP & editbox->flags))
	{
		HCURSOR hCursor = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_IBEAM), IMAGE_CURSOR, 
							0, 0, LR_DEFAULTCOLOR | LR_SHARED | LR_DEFAULTSIZE);
		if (NULL != hCursor)
		{
			SetCursor(hCursor);
			return TRUE;
		}
	}
	
	HWND hParent = GetParent(hwnd);
	if (NULL != hParent && 
		FALSE != (BOOL)SendMessage(hParent, WM_SETCURSOR, (WPARAM)hOwner, MAKELPARAM(hitTest, messageId)))
	{
		return TRUE;
	}

	return ToolbarEditbox_CallOrigWindowProc(hwnd, WM_SETCURSOR, (WPARAM)hOwner, MAKELPARAM(hitTest, messageId));
}

static void ToolbarEditbox_OnContextMenu(HWND hwnd, HWND hTarget, POINTS pts)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox) editbox->flags |= NTEF_MENULOOP;
//	SendMessage(hwnd, WM_SETCURSOR, (WPARAM)hwnd, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
	ToolbarEditbox_CallOrigWindowProc(hwnd, WM_CONTEXTMENU, (WPARAM)hTarget, *((LPARAM*)&pts));
	if (NULL != editbox) editbox->flags &= ~NTEF_MENULOOP;
}

static void ToolbarEditbox_OnLButtonDown(HWND hwnd, UINT vKey, POINTS pts)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox)
	{
		if (0 != (NTEF_SHOWCARET & editbox->flags))
		{
			editbox->flags &= ~NTEF_SHOWCARET;
			ShowCaret(hwnd);
		}

		
		DWORD clickTime = GetTickCount();
		if (clickTime >= editbox->dblclkTime && clickTime <= (editbox->dblclkTime + GetDoubleClickTime()))
		{
			SendMessage(hwnd, NTEBM_SELECTALL, 0, 0L);
			return;
		}

		UINT start, end;
		SendMessage(hwnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
		if (start != end)
		{
			editbox->flags |= NTEF_CLICKEDONCE;
		}
	}

	ToolbarEditbox_CallOrigWindowProc(hwnd, WM_LBUTTONDOWN, (WPARAM)vKey, *((LPARAM*)&pts));
}

static void ToolbarEditbox_OnLButtonUp(HWND hwnd, UINT vKey, POINTS pts)
{
	ToolbarEditbox_CallOrigWindowProc(hwnd, WM_LBUTTONUP, (WPARAM)vKey, *((LPARAM*)&pts));

	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox && 0 == (NTEF_CLICKEDONCE & editbox->flags) && hwnd == GetFocus())
	{
		editbox->flags |= NTEF_CLICKEDONCE;
		UINT start, end;
		SendMessage(hwnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
		if (start == end)
		{
			SendMessage(hwnd, NTEBM_SELECTALL, 0, 0L);
		}
	}
}

static void ToolbarEditbox_OnLButtonDblClk(HWND hwnd, UINT vKey, POINTS pts)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL == editbox) return;
	
	DWORD clickTime = GetTickCount();
	if (clickTime >= editbox->dblclkTime && clickTime <= (editbox->dblclkTime + 2*GetDoubleClickTime()))
	{
		INT r = (INT)SendMessage(hwnd, EM_CHARFROMPOS, 0, *(LPARAM*)&pts); 
		r = LOWORD(r);
		SendMessage(hwnd, EM_SETSEL, (WPARAM)r, (LPARAM)r);
		editbox->dblclkTime = 0;
	}
	else
	{
		editbox->dblclkTime = clickTime;
	}

	INT f, l;
	SendMessage(hwnd, EM_GETSEL, (WPARAM)&f, (LPARAM)&l);
	if (f != l) return;
	
		
	ToolbarEditbox_CallOrigWindowProc(hwnd, WM_LBUTTONDBLCLK, (WPARAM)vKey, *((LPARAM*)&pts));

}

static void ToolbarEditbox_OnMouseMove(HWND hwnd, UINT vKey, POINTS pts)
{
	ToolbarEditbox_CallOrigWindowProc(hwnd, WM_MOUSEMOVE, (WPARAM)vKey, *((LPARAM*)&pts));

	if (0 != (MK_LBUTTON & vKey))
	{
		TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
		if (NULL != editbox && 0 == (NTEF_SHOWCARET & editbox->flags) && 
			hwnd == GetFocus())
		{
			UINT start, end;
			SendMessage(hwnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
			if (start != end)
			{
				if (FALSE != HideCaret(hwnd))
					editbox->flags |= NTEF_SHOWCARET;
			}
		}
	}
}

static LRESULT CALLBACK ToolbarEditbox_MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	ToolbarEditbox_PatchCursor();

	TOOLBAREDITBOX *editbox = GetEditbox(GetFocus());
	if (NULL != editbox && 0 != (NTEF_SHOWCURSOR & editbox->flags))
	{
		ShowCursor(TRUE);
		editbox->flags &= ~NTEF_SHOWCURSOR;
	}

	LRESULT result = CallNextHookEx(mouseHook, nCode, wParam, lParam);
	UnhookWindowsHookEx(mouseHook);
	mouseHook = NULL;
	return result;
}

static void ToolbarEditbox_DeleteWord(HWND hwnd, UINT vKey, UINT state)
{
	BOOL resetVisible = FALSE;
	INT first, last;
	SendMessage(hwnd, EM_GETSEL, (WPARAM)&first, (LPARAM)&last);
	if (first == last)
	{
		UINT windowStyle = GetWindowStyle(hwnd);
		if (0 != (WS_VISIBLE & windowStyle))
		{
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);
			resetVisible = TRUE;
		}

		SendMessage(hwnd, WM_KEYDOWN, (WPARAM)vKey, (LPARAM)state);
		INT newFirst, newLast;
		SendMessage(hwnd, EM_GETSEL, (WPARAM)&newFirst, (LPARAM)&newLast);
		if (newFirst != first || newLast != last)
			SendMessage(hwnd, EM_SETSEL, (WPARAM)first, (LPARAM)newLast);
	}

	SendMessage(hwnd, EM_REPLACESEL, TRUE, NULL); 
	if (FALSE != resetVisible)
	{
		UINT windowStyle = GetWindowStyle(hwnd);
		if (0 == (WS_VISIBLE & windowStyle))
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);

		InvalidateRect(hwnd, NULL, FALSE);
	}

}
static void ToolbarEditbox_OnKeyDown(HWND hwnd, UINT vKey, UINT state)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox) 
	{
		if (0 != (NTEF_SHOWCARET & editbox->flags))
		{
			editbox->flags &= ~NTEF_SHOWCARET;
			ShowCaret(hwnd);
		}

		editbox->flags |= NTEF_KEYDOWN;
		
	}
	

	if (NULL == editbox || NULL == editbox->host ||
		FALSE == editbox->host->EditboxKeyDown(hwnd, vKey, state))
	{
		EDITWORDBREAKPROC fnOrigBreak = NULL;
		if(0 != (0x8000 & GetAsyncKeyState(VK_CONTROL)))
		{
			switch(vKey)
			{
				case VK_LEFT:
				case VK_RIGHT:
					fnOrigBreak = (EDITWORDBREAKPROC)SendMessage(hwnd, EM_GETWORDBREAKPROC, 0, 0L);
					if (ToolbarEditbox_WordBreakProc == fnOrigBreak)
					{
						EDITWORDBREAKPROC fnOverride = (VK_LEFT == vKey) ?
							ToolbarEditbox_WordBreakProcOverrideLeft : ToolbarEditbox_WordBreakProcOverrideRight;
						SendMessage(hwnd, EM_SETWORDBREAKPROC, 0, (LPARAM)fnOverride);
					}
					break;
				case VK_DELETE:
					ToolbarEditbox_DeleteWord(hwnd, VK_RIGHT, state);
					return;
				case VK_BACK:
					ToolbarEditbox_DeleteWord(hwnd, VK_LEFT, state);
					return;

				
			}
		}

		ToolbarEditbox_CallOrigWindowProc(hwnd, WM_KEYDOWN, (WPARAM)vKey, (LPARAM)state);

		if (NULL != fnOrigBreak)
			SendMessage(hwnd, EM_SETWORDBREAKPROC, 0, (LPARAM)fnOrigBreak);
	}
	
	if (('0' <= vKey && vKey <= '9') ||
		('A' <= vKey && vKey <= 'Z') ||
		(VK_NUMPAD0 <= vKey && vKey <= VK_DIVIDE) ||
		(VK_OEM_1 <= vKey && vKey <= VK_OEM_3) ||
		(VK_OEM_4 <= vKey && vKey <= VK_OEM_8) ||
		VK_OEM_NEC_EQUAL == vKey || 
		VK_OEM_102 == vKey)
	{
		if (0 == (0x8000 & GetAsyncKeyState(VK_CONTROL)) && 
			0 == (0x8000 & GetAsyncKeyState(VK_MENU)))
		{
			PostMessage(hwnd, NTEBM_UPDATECURSOR, 0, 0L);
		}
	}	
	
}

static void ToolbarEditbox_OnKeyUp(HWND hwnd, UINT vKey, UINT state)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL == editbox || NULL == editbox->host ||
		FALSE == editbox->host->EditboxKeyUp(hwnd, vKey, state))
	{
		ToolbarEditbox_CallOrigWindowProc(hwnd, WM_KEYUP, (WPARAM)vKey, (LPARAM)state);
	}
	
	if (NULL != editbox) 
		editbox->flags &= ~NTEF_KEYDOWN;

	if (NULL == mouseHook)
	{
		mouseHook = SetWindowsHookEx(WH_MOUSE, ToolbarEditbox_MouseHook, NULL, GetCurrentThreadId());
	}

}

static void ToolbarEditbox_OnChar(HWND hwnd, UINT vKey, UINT state)
{	
	BOOL fControl = FALSE;
	if (0 != (0x8000 & GetAsyncKeyState(VK_CONTROL)))
	{
		fControl = TRUE;
		UINT scanCode = (HIWORD(state) & 0x00FF);
		vKey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK);
	}

	switch(vKey)
	{
		case VK_ESCAPE:		ToolbarEditbox_ResetText(hwnd); return;
		case VK_TAB:		ToolbarEditbox_NavigateNextCtrl(hwnd, 0 == (0x8000 & GetAsyncKeyState(VK_SHIFT))); return;
		case VK_RETURN:		ToolbarEditbox_AcceptText(hwnd); return;
	}

	if (FALSE != fControl)
	{
		switch(vKey)
		{
			case 'A':	SendMessage(hwnd, NTEBM_SELECTALL, 0, 0L); return;
			case 'Z':	SendMessage(hwnd, EM_UNDO, 0, 0L); return;
			case 'X':	SendMessage(hwnd, WM_CUT, 0, 0L); return;
			case 'C':	SendMessage(hwnd, WM_COPY, 0, 0L); return;
			case 'V':	SendMessage(hwnd, WM_PASTE, 0, 0L); return;
		}
		return;
	}
		

	

	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox && NULL != editbox->host && 
		FALSE != editbox->host->EditboxPreviewChar(hwnd, vKey, state))
	{
		return;
	}

	ToolbarEditbox_CallOrigWindowProc(hwnd, WM_CHAR, (WPARAM)vKey, (LPARAM)state);
}


static void ToolbarEditbox_OnUpdateCursor(HWND hwnd)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox && 0 != (NTEF_KEYDOWN & editbox->flags))
	{	
		if (NULL != mouseHook)
		{
			UnhookWindowsHookEx(mouseHook);
			mouseHook = NULL;
		}

		if (0 == (NTEF_SHOWCURSOR & editbox->flags))
		{
			CURSORINFO cursorInfo;
			cursorInfo.cbSize = sizeof(CURSORINFO);
			if (GetCursorInfo(&cursorInfo) && 0 != (CURSOR_SHOWING & cursorInfo.flags))
			{
				ShowCursor(FALSE);
				editbox->flags |= NTEF_SHOWCURSOR;
			}
		}
	}
}

static void ToolbarEditbox_OnEnterMenuLoop(HWND hwnd, BOOL fContext)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox) editbox->flags |= NTEF_MENULOOP;

	ToolbarEditbox_CallOrigWindowProc(hwnd, WM_ENTERMENULOOP, (WPARAM)fContext, 0L);
}

static void ToolbarEditbox_OnExitMenuLoop(HWND hwnd, BOOL fContext)
{
	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox) editbox->flags &= ~NTEF_MENULOOP;

	ToolbarEditbox_CallOrigWindowProc(hwnd, WM_EXITMENULOOP, (WPARAM)fContext, 0L);
}

static BOOL ToolbarEditbox_RemoveBadChars(LPCWSTR pszText, LPWSTR *bufferOut)
{
	LPWSTR buffer = NULL;
	if (NULL == pszText) return FALSE;
	
	const WCHAR szBadChars[] = { L'\r', L'\n', L'\t', L'\0'};
	BOOL fDetected = FALSE;

	for (LPCWSTR p = pszText; L'\0' != *p && FALSE == fDetected; p++)
	{
		for (LPCWSTR b = szBadChars; L'\0' != *b; b++)
		{
			if (*p == *b)
			{
				fDetected = TRUE;
				break;
			}
		}
	}

	if (FALSE == fDetected)
		return FALSE;

	if (NULL == bufferOut)
		return TRUE;
		
	INT cchText = lstrlen(pszText);
	buffer = Plugin_MallocString(cchText + 1);
	if (NULL == buffer) return FALSE;
	
	LPCWSTR s = pszText;
	LPWSTR d = buffer;
	LPCWSTR b;
	for(;;)
	{
		for (b = szBadChars; L'\0' != *b && *s != *b; b++);
		if(L'\0' != *b)
		{
			if (L'\t' == *b)
			{
				*d = L' ';
				d++;
			}
		}
		else
		{
			*d = *s;
			d++;
		}
		
		if (L'\0' == *s)
			break;
		
		s++;
		
	}

	*bufferOut = buffer;
	return TRUE;
}

static LRESULT ToolbarEditbox_OnSetText(HWND hwnd, LPCWSTR pszText)
{
	LPWSTR buffer;
	if (FALSE == ToolbarEditbox_RemoveBadChars(pszText, &buffer))
		buffer = NULL;
	else
		pszText = buffer;
	
	LRESULT result =  ToolbarEditbox_CallOrigWindowProc(hwnd, WM_SETTEXT, 0, (LPARAM)pszText);
	
	if (NULL != buffer)
		Plugin_FreeString(buffer);

	return result;
}

static LRESULT ToolbarEditbox_OnReplaceSel(HWND hwnd, BOOL fUndo, LPCWSTR pszText)
{
	LPWSTR buffer;
	if (FALSE == ToolbarEditbox_RemoveBadChars(pszText, &buffer))
		buffer = NULL;
	else
		pszText = buffer;
	
	LRESULT result =  ToolbarEditbox_CallOrigWindowProc(hwnd, EM_REPLACESEL, (WPARAM)fUndo, (LPARAM)pszText);
	
	if (NULL != buffer)
		Plugin_FreeString(buffer);

	return result;
}

static void ToolbarEditbox_ReplaceText(HWND hwnd, LPCWSTR pszText, BOOL fUndo, BOOL fScrollCaret)
{
	UINT windowStyle = GetWindowStyle(hwnd);
	if (0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);
				
	SendMessage(hwnd, EM_REPLACESEL, (WPARAM)fUndo, (LPARAM)pszText);
	if (FALSE != fScrollCaret)
	{
		INT f, l;
		SendMessage(hwnd, EM_GETSEL, (WPARAM)&f, (LPARAM)&l);
		SendMessage(hwnd, EM_SETSEL, (WPARAM)f, (LPARAM)l);
	}

	if (0 != (WS_VISIBLE & windowStyle))
	{
		windowStyle = GetWindowStyle(hwnd);
		if (0 == (WS_VISIBLE & windowStyle))
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
		InvalidateRect(hwnd, NULL, FALSE);
	}
}

static void ToolbarEditbox_OnPaste(HWND hwnd)
{
	IDataObject *pObject;
	HRESULT hr = OleGetClipboard(&pObject);
	if (SUCCEEDED(hr))
	{		
		FORMATETC fmt = {CF_UNICODETEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		STGMEDIUM stgm; 
		hr = pObject->GetData(&fmt, &stgm);
		if(S_OK == hr) 
		{
			LPCWSTR pClipboard = (LPCWSTR)GlobalLock(stgm.hGlobal);
			ToolbarEditbox_ReplaceText(hwnd, pClipboard, TRUE, TRUE);
      		GlobalUnlock(stgm.hGlobal);
			ReleaseStgMedium(&stgm);

		}
		else
		{
			fmt.cfFormat = CF_TEXT;
			hr = pObject->GetData(&fmt, &stgm);
			if(S_OK == hr) 
			{
				LPCSTR pClipboardAnsi = (LPCSTR)GlobalLock(stgm.hGlobal);
				LPWSTR pClipboard = Plugin_MultiByteToWideChar(CP_ACP, 0, pClipboardAnsi, -1);
				ToolbarEditbox_ReplaceText(hwnd, pClipboard, TRUE, TRUE);
				Plugin_FreeString(pClipboard);
      			GlobalUnlock(stgm.hGlobal);
				ReleaseStgMedium(&stgm);
			}
		}
  		pObject->Release();
	}
}


static void ToolbarEditbox_OnSelectAll(HWND hwnd)
{
	UINT windowStyle = GetWindowStyle(hwnd);

	INT f, l, tl;
	tl = GetWindowTextLength(hwnd);
	SendMessage(hwnd, EM_GETSEL, (WPARAM)&f, (LPARAM)&l);
	if (f == 0 && tl == l) 
		return;

	if (0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

	RECT rc;
	GetWindowRect(hwnd, &rc);
	SetWindowPos(hwnd, NULL, 0, 0, 0xFFFFF, rc.bottom - rc.top, 
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOCOPYBITS);
	
	SendMessage(hwnd, EM_SETSEL, 0, -1);

	SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, 
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOCOPYBITS);

	if (0 != (WS_VISIBLE & windowStyle))
	{
		UINT windowStyle = GetWindowStyle(hwnd);
		if (0 == (WS_VISIBLE & windowStyle))
		{
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
			InvalidateRect(hwnd, NULL, FALSE);
		}
	}

	TOOLBAREDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox && 
		0 == (NTEF_SHOWCARET & editbox->flags) &&
		FALSE != HideCaret(hwnd))
	{
		editbox->flags |= NTEF_SHOWCARET;
	}
}



static LRESULT ToolbarEditbox_OnFindWordBreak(HWND hwnd, INT code, INT start)
{
	EDITWORDBREAKPROC fnBreak = (EDITWORDBREAKPROC)SendMessage(hwnd, EM_GETWORDBREAKPROC, 0, 0L);
	if (NULL == fnBreak) return 0;

	UINT cchText = GetWindowTextLength(hwnd);
	if (0 == cchText) return 0;

	LPWSTR pszText = Plugin_MallocString(cchText + 1);
	if (NULL == pszText) return 0;

	LRESULT result = 0;
	cchText = GetWindowText(hwnd, pszText, cchText + 1);
	if (0 != cchText)
	{
		result = fnBreak(pszText, start, cchText, code);
	}
	Plugin_FreeString(pszText);
	return result;
}

static LRESULT CALLBACK ToolbarEditbox_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DESTROY:		ToolbarEditbox_OnDestroy(hwnd); return 0;
		case WM_SETFOCUS:		ToolbarEditbox_OnSetFocus(hwnd, (HWND)wParam); return 0;
		case WM_KILLFOCUS:		ToolbarEditbox_OnKillFocus(hwnd, (HWND)wParam); return 0;
		case WM_GETDLGCODE:		return ToolbarEditbox_OnGetDlgCode(hwnd, (INT)wParam, (MSG*)lParam);
		case WM_SETCURSOR:		return ToolbarEditbox_OnSetCursor(hwnd, (HWND)wParam, (INT)LOWORD(lParam), (INT)HIWORD(lParam));
		case WM_CONTEXTMENU:	ToolbarEditbox_OnContextMenu(hwnd, (HWND)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_LBUTTONDOWN:	ToolbarEditbox_OnLButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_LBUTTONUP:		ToolbarEditbox_OnLButtonUp(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_LBUTTONDBLCLK:	ToolbarEditbox_OnLButtonDblClk(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_MOUSEMOVE:		ToolbarEditbox_OnMouseMove(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_KEYDOWN:		ToolbarEditbox_OnKeyDown(hwnd, (UINT)wParam, (UINT)lParam); return 0;
		case WM_KEYUP:			ToolbarEditbox_OnKeyUp(hwnd, (UINT)wParam, (UINT)lParam); return 0;
		case WM_CHAR:			ToolbarEditbox_OnChar(hwnd, (UINT)wParam, (UINT)lParam); return 0;
		case WM_ENTERMENULOOP:	ToolbarEditbox_OnEnterMenuLoop(hwnd, (BOOL)wParam); return 0;
		case WM_EXITMENULOOP:	ToolbarEditbox_OnExitMenuLoop(hwnd, (BOOL)wParam); return 0;
		case WM_SETTEXT:		return ToolbarEditbox_OnSetText(hwnd, (LPCWSTR)lParam);
		case WM_PASTE:			ToolbarEditbox_OnPaste(hwnd); return 1;
		case EM_REPLACESEL:		ToolbarEditbox_OnReplaceSel(hwnd, (BOOL)wParam, (LPCWSTR)lParam); return 0;
		case EM_FINDWORDBREAK:	return ToolbarEditbox_OnFindWordBreak(hwnd, (INT)wParam, (INT)lParam);
			
		case NTEBM_UPDATECURSOR: ToolbarEditbox_OnUpdateCursor(hwnd); return 0;
		case NTEBM_SELECTALL:	ToolbarEditbox_OnSelectAll(hwnd); return 0;
	}
	
	return ToolbarEditbox_CallOrigWindowProc(hwnd, uMsg, wParam, lParam);
}
