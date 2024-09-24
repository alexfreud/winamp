#define OEMRESOURCE

#include "./addressEditbox.h"
#include "./common.h"

#include <richedit.h>
#include <strsafe.h>

#define NAEF_USERFLAGSMASK	0x00FFFFFF
#define NAEF_UNICODE		0x01000000


typedef struct __ADDRESSEDITBOX
{
	WNDPROC originalProc;
	UINT	flags;
	DWORD	dblclkTime;	LPWSTR	rollbackText;
} ADDRESSEDITBOX;

#define ADDRESSEDITBOX_PROP		L"NullsoftAddressEditbox"

#define GetEditbox(__hwnd) ((ADDRESSEDITBOX*)GetProp((__hwnd), ADDRESSEDITBOX_PROP))

static LRESULT CALLBACK AddressEditbox_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT CALLBACK AddressEditbox_WordBreakProc(LPWSTR pszText, INT iCurrent, INT cchLen, INT code);
static INT CALLBACK AddressEditbox_WordBreakProc2(LPWSTR pszText, INT iCurrent, INT cchLen, INT code);

BOOL AddressEditbox_AttachWindow(HWND hEditbox)
{
	if (!IsWindow(hEditbox)) 
		return  FALSE;

	ADDRESSEDITBOX *editbox = (ADDRESSEDITBOX*)GetProp(hEditbox, ADDRESSEDITBOX_PROP);
	if (NULL != editbox) return TRUE;
	
	editbox = (ADDRESSEDITBOX*)calloc(1, sizeof(ADDRESSEDITBOX));
	if (NULL == editbox) return FALSE;


	ZeroMemory(editbox, sizeof(ADDRESSEDITBOX));

	if (IsWindowUnicode(hEditbox))
		editbox->flags |= NAEF_UNICODE;

	editbox->originalProc = (WNDPROC)(LONG_PTR)((0 != (NAEF_UNICODE & editbox->flags)) ? 
						SetWindowLongPtrW(hEditbox, GWLP_WNDPROC, (LONGX86)(LONG_PTR)AddressEditbox_WindowProc) : 
						SetWindowLongPtrA(hEditbox, GWLP_WNDPROC, (LONGX86)(LONG_PTR)AddressEditbox_WindowProc));

	if (NULL == editbox->originalProc || !SetProp(hEditbox, ADDRESSEDITBOX_PROP, editbox))
	{
		if (NULL != editbox->originalProc)
		{
			if (0 != (NAEF_UNICODE & editbox->flags))
				SetWindowLongPtrW(hEditbox, GWLP_WNDPROC, (LONGX86)(LONG_PTR)editbox->originalProc);
			else
				SetWindowLongPtrA(hEditbox, GWLP_WNDPROC, (LONGX86)(LONG_PTR)editbox->originalProc);
		}
			
		free(editbox);
		return FALSE;
	}
	SendMessage(hEditbox, EM_SETWORDBREAKPROC, 0, (LPARAM)AddressEditbox_WordBreakProc);


	if (FAILED(LoginBox_GetWindowText(hEditbox, &editbox->rollbackText, NULL)))
		editbox->rollbackText = NULL;

	return TRUE;
}


static void AddressEditbox_Detach(HWND hwnd)
{	
	ADDRESSEDITBOX *editbox = GetEditbox(hwnd);
	RemoveProp(hwnd, ADDRESSEDITBOX_PROP);

	if (NULL == editbox) 
		return;

	if (NULL != editbox->originalProc)
	{
		if (0 != (NAEF_UNICODE & editbox->flags))
			SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)editbox->originalProc);
		else
			SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)editbox->originalProc);
	}

	free(editbox);
}


static LRESULT AddressEditbox_CallOrigWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ADDRESSEDITBOX *editbox = GetEditbox(hwnd);

	if (NULL == editbox || NULL == editbox->originalProc)
	{
		return (0 != (NAEF_UNICODE & editbox->flags)) ? 
				DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
				DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}

	return (0 != (NAEF_UNICODE & editbox->flags)) ? 
			CallWindowProcW(editbox->originalProc, hwnd, uMsg, wParam, lParam) : 
			CallWindowProcA(editbox->originalProc, hwnd, uMsg, wParam, lParam);
}

static void AddressEditbox_SelectReplacementBlock(HWND hwnd, LPCWSTR pszText)
{
	INT begin(-1), end(-1);

	if (NULL != pszText)
	{
		LPCWSTR cursor = pszText;
		while(L'\0' != *cursor)
		{
			if (-1 == begin)
			{
				if (REPLACE_MARKER_BEGIN == *cursor)
					begin = (INT)(INT_PTR)(cursor - pszText);
			}
			else if (REPLACE_MARKER_END == *cursor)
			{
				end = (INT)(INT_PTR)(cursor - pszText) + 1;
				break;
			}
			cursor = CharNext(cursor);
		}
		if (-1 == begin || -1 == end)
		{
			begin = (INT)(INT_PTR)(cursor - pszText);
			end = begin + 1;
		}
	}
	
	SendMessage(hwnd, EM_SETSEL, begin, end);
}
static void AddressEditbox_ResetText(HWND hwnd)
{
	ADDRESSEDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox)
	{
		AddressEditbox_CallOrigWindowProc(hwnd, WM_SETTEXT, 0, (LPARAM)editbox->rollbackText);
		AddressEditbox_SelectReplacementBlock(hwnd, editbox->rollbackText);
	}
}

static BOOL AddressEditbox_IsDelimiterChar(WCHAR testChar)
{
	WORD info;
	if (FALSE == GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, &testChar, 1, &info))
		return 0;

	if (0 != ((C1_SPACE | C1_PUNCT | C1_CNTRL | C1_BLANK) & info) && 
		REPLACE_MARKER_BEGIN != testChar && REPLACE_MARKER_END != testChar)
	{
		return TRUE;
	}

	return FALSE;
}

static INT AddressEditbox_FindLeft(LPCWSTR pszText, INT iCurrent, INT cchLen)
{
	if (iCurrent <= 0) 
		return 0;

	LPCWSTR pszCursor = &pszText[iCurrent];
	BOOL charDelim = AddressEditbox_IsDelimiterChar(*pszCursor);
	
	for(;;)
	{
		pszCursor = CharPrev(pszText, pszCursor);
		if (charDelim != AddressEditbox_IsDelimiterChar(*pszCursor))
			return (INT)(INT_PTR)(CharNext(pszCursor) - pszText);
		
		if (pszCursor == pszText)
			break;
	}
	return 0;
}

static INT AddressEditbox_FindRight(LPCWSTR pszText, INT iCurrent, INT cchLen)
{
	if (iCurrent >= cchLen)
		return cchLen;

	LPCWSTR pszEnd = &pszText[cchLen];
	LPCWSTR pszCursor = &pszText[iCurrent];

	if (iCurrent > 0)
		pszCursor = CharNext(pszCursor);
	
	BOOL charDelim = AddressEditbox_IsDelimiterChar(*pszCursor);

	for(;;)
	{
		pszCursor = CharNext(pszCursor);
		if (pszCursor >= pszEnd) 
			break;

		if (charDelim != AddressEditbox_IsDelimiterChar(*pszCursor))
			return (INT)(INT_PTR)(pszCursor - pszText);
	}
	return cchLen;
}

static INT AddressEditbox_FindWordLeft(LPCWSTR pszText, INT iCurrent, INT cchLen, BOOL fRightCtrl)
{
	if (iCurrent < 2)
		return 0;

	LPCWSTR pszCursor = &pszText[iCurrent];

	if (FALSE == fRightCtrl) 
		pszCursor = CharPrev(pszText, pszCursor);

	BOOL prevCharDelim = AddressEditbox_IsDelimiterChar(*pszCursor);
	for(;;)
	{
		pszCursor = CharPrev(pszText, pszCursor);
		if (TRUE == AddressEditbox_IsDelimiterChar(*pszCursor))
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

static INT AddressEditbox_FindWordRight(LPCWSTR pszText, INT iCurrent, INT cchLen)
{
	if ( iCurrent >= (cchLen - 1))
		return cchLen;

	LPCWSTR pszEnd = &pszText[cchLen];
	LPCWSTR pszCursor = &pszText[iCurrent];

	BOOL prevCharDelim = AddressEditbox_IsDelimiterChar(*pszCursor);

	for(;;)
	{
		pszCursor = CharNext(pszCursor);
		if (pszCursor >= pszEnd) 
			break;

		if (prevCharDelim != AddressEditbox_IsDelimiterChar(*pszCursor))
		{
			prevCharDelim = TRUE;
			return (INT)(INT_PTR)(pszCursor - pszText);
		}
		else
			prevCharDelim = FALSE;

	}
	return cchLen;
}

static INT CALLBACK AddressEditbox_WordBreakProc(LPWSTR pszText, INT iCurrent, INT cchLen, INT code)
{
	switch(code)
	{
		case WB_ISDELIMITER:	return (iCurrent < 0) ? 0 : ((iCurrent > cchLen) ? (cchLen + 1) : AddressEditbox_IsDelimiterChar(pszText[iCurrent]));
		case WB_LEFT:			return AddressEditbox_FindLeft(pszText, iCurrent, cchLen);
		case WB_RIGHT:			return AddressEditbox_FindRight(pszText, iCurrent, cchLen);
		case WB_MOVEWORDLEFT:	return AddressEditbox_FindWordLeft(pszText, iCurrent, cchLen, FALSE);
		case WB_MOVEWORDRIGHT:	return AddressEditbox_FindWordRight(pszText, iCurrent, cchLen);
	}
	return 0;
}

static INT CALLBACK AddressEditbox_WordBreakProcOverrideLeft(LPWSTR pszText, INT iCurrent, INT cchLen, INT code)
{
	switch(code)
	{
		case WB_LEFT:	return AddressEditbox_FindWordLeft(pszText, iCurrent, cchLen, FALSE);
		case WB_RIGHT:	return AddressEditbox_FindWordRight(pszText, iCurrent, cchLen);
	}
	return AddressEditbox_WordBreakProc(pszText, iCurrent, cchLen, code);
}

static INT CALLBACK AddressEditbox_WordBreakProcOverrideRight(LPWSTR pszText, INT iCurrent, INT cchLen, INT code)
{
	switch(code)
	{
		case WB_LEFT:	return AddressEditbox_FindWordLeft(pszText, iCurrent, cchLen, TRUE);
		case WB_RIGHT:	return AddressEditbox_FindWordRight(pszText, iCurrent, cchLen);
	}
	return AddressEditbox_WordBreakProc(pszText, iCurrent, cchLen, code);
}


static void AddressEditbox_OnDestroy(HWND hwnd)
{
	ADDRESSEDITBOX *editbox = GetEditbox(hwnd);
	
	WNDPROC originalProc = editbox->originalProc;
	BOOL fUnicode = (0 != (NAEF_UNICODE & editbox->flags));

	AddressEditbox_Detach(hwnd);

	if (NULL != originalProc)
	{
		if (FALSE != fUnicode) 
			CallWindowProcW(originalProc, hwnd, WM_DESTROY, 0, 0L);
		else
			CallWindowProcA(originalProc, hwnd, WM_DESTROY, 0, 0L);
	}
	
}

static LRESULT AddressEditbox_OnGetDlgCode(HWND hwnd, INT vKey, MSG* pMsg)
{
	LRESULT result = AddressEditbox_CallOrigWindowProc(hwnd, WM_GETDLGCODE, (WPARAM)vKey, (LPARAM)pMsg);
	
	switch(vKey)
	{
		case VK_ESCAPE:
			return result |= DLGC_WANTALLKEYS;
	}
		
	result &= ~DLGC_HASSETSEL;
	return result;
}

		
static void AddressEditbox_OnLButtonDown(HWND hwnd, UINT vKey, POINTS pts)
{
	ADDRESSEDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox)
	{
		DWORD clickTime = GetTickCount();
		if (clickTime >= editbox->dblclkTime && clickTime <= (editbox->dblclkTime + GetDoubleClickTime()))
		{
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			return;
		}
	}
	AddressEditbox_CallOrigWindowProc(hwnd, WM_LBUTTONDOWN, (WPARAM)vKey, *((LPARAM*)&pts));
}

static void AddressEditbox_OnLButtonDblClk(HWND hwnd, UINT vKey, POINTS pts)
{
	ADDRESSEDITBOX *editbox = GetEditbox(hwnd);
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
	
		
	AddressEditbox_CallOrigWindowProc(hwnd, WM_LBUTTONDBLCLK, (WPARAM)vKey, *((LPARAM*)&pts));

}


static void AddressEditbox_DeleteWord(HWND hwnd, UINT vKey, UINT state)
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
static void AddressEditbox_OnKeyDown(HWND hwnd, UINT vKey, UINT state)
{	
	EDITWORDBREAKPROC fnOrigBreak = NULL;
	if(0 != (0x8000 & GetAsyncKeyState(VK_CONTROL)))
	{
		switch(vKey)
		{
			case VK_LEFT:
			case VK_RIGHT:
				fnOrigBreak = (EDITWORDBREAKPROC)SendMessage(hwnd, EM_GETWORDBREAKPROC, 0, 0L);
				if (AddressEditbox_WordBreakProc == fnOrigBreak)
				{
					EDITWORDBREAKPROC fnOverride = (VK_LEFT == vKey) ?
						AddressEditbox_WordBreakProcOverrideLeft : AddressEditbox_WordBreakProcOverrideRight;
					SendMessage(hwnd, EM_SETWORDBREAKPROC, 0, (LPARAM)fnOverride);
				}
				break;
			case VK_DELETE:
				AddressEditbox_DeleteWord(hwnd, VK_RIGHT, state);
				return;
			case VK_BACK:
				AddressEditbox_DeleteWord(hwnd, VK_LEFT, state);
				return;
		}
	}

	AddressEditbox_CallOrigWindowProc(hwnd, WM_KEYDOWN, (WPARAM)vKey, (LPARAM)state);

	if (NULL != fnOrigBreak)
		SendMessage(hwnd, EM_SETWORDBREAKPROC, 0, (LPARAM)fnOrigBreak);
}

static void AddressEditbox_OnChar(HWND hwnd, UINT vKey, UINT state)
{
	if (0 != (0x8000 & GetAsyncKeyState(VK_CONTROL)))
	{
		UINT scanCode = (HIWORD(state) & 0x00FF);
		vKey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK);
	}
	
	switch(vKey)
	{
		case VK_ESCAPE:		AddressEditbox_ResetText(hwnd); return;
	}


	AddressEditbox_CallOrigWindowProc(hwnd, WM_CHAR, (WPARAM)vKey, (LPARAM)state);
}



static BOOL AddressEditbox_RemoveBadChars(LPCWSTR pszText, LPWSTR *bufferOut)
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
	buffer = LoginBox_MallocString(cchText + 1);
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

static LRESULT AddressEditbox_OnSetText(HWND hwnd, LPCWSTR pszText)
{
	LPWSTR buffer;
	if (FALSE == AddressEditbox_RemoveBadChars(pszText, &buffer))
		buffer = NULL;
	else
		pszText = buffer;
	
	LRESULT result =  AddressEditbox_CallOrigWindowProc(hwnd, WM_SETTEXT, 0, (LPARAM)pszText);
	
	ADDRESSEDITBOX *editbox = GetEditbox(hwnd);
	if (NULL != editbox)
	{
		LoginBox_FreeString(editbox->rollbackText);
		editbox->rollbackText = LoginBox_CopyString(pszText);
		AddressEditbox_SelectReplacementBlock(hwnd, pszText);
	}
	
	if (NULL != buffer)
		LoginBox_FreeString(buffer);

	return result;
}

static LRESULT AddressEditbox_OnReplaceSel(HWND hwnd, BOOL fUndo, LPCWSTR pszText)
{
	LPWSTR buffer;
	if (FALSE == AddressEditbox_RemoveBadChars(pszText, &buffer))
		buffer = NULL;
	else
		pszText = buffer;
	
	LRESULT result =  AddressEditbox_CallOrigWindowProc(hwnd, EM_REPLACESEL, (WPARAM)fUndo, (LPARAM)pszText);
	
	if (NULL != buffer)
		LoginBox_FreeString(buffer);

	return result;
}

static void AddressEditbox_ReplaceText(HWND hwnd, LPCWSTR pszText, BOOL fUndo, BOOL fScrollCaret)
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

static void AddressEditbox_OnPaste(HWND hwnd)
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
			AddressEditbox_ReplaceText(hwnd, pClipboard, TRUE, TRUE);
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
				LPWSTR pClipboard;
				if (FAILED(LoginBox_MultiByteToWideChar(CP_ACP, 0, pClipboardAnsi, -1, &pClipboard)))
					pClipboard = NULL;

				AddressEditbox_ReplaceText(hwnd, pClipboard, TRUE, TRUE);
				LoginBox_FreeString(pClipboard);
      			GlobalUnlock(stgm.hGlobal);
				ReleaseStgMedium(&stgm);
			}
		}
  		pObject->Release();
	}
}

static LRESULT AddressEditbox_OnFindWordBreak(HWND hwnd, INT code, INT start)
{
	EDITWORDBREAKPROC fnBreak = (EDITWORDBREAKPROC)SendMessage(hwnd, EM_GETWORDBREAKPROC, 0, 0L);
	if (NULL == fnBreak) return 0;

	UINT cchText = GetWindowTextLength(hwnd);
	if (0 == cchText) return 0;

	LPWSTR pszText = LoginBox_MallocString(cchText + 1);
	if (NULL == pszText) return 0;

	LRESULT result = 0;
	cchText = GetWindowText(hwnd, pszText, cchText + 1);
	if (0 != cchText)
	{
		result = fnBreak(pszText, start, cchText, code);
	}
	LoginBox_FreeString(pszText);
	return result;
}

static LRESULT CALLBACK AddressEditbox_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DESTROY:		AddressEditbox_OnDestroy(hwnd); return 0;
		case WM_GETDLGCODE:		return AddressEditbox_OnGetDlgCode(hwnd, (INT)wParam, (MSG*)lParam);
		case WM_LBUTTONDOWN:	AddressEditbox_OnLButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_LBUTTONDBLCLK:	AddressEditbox_OnLButtonDblClk(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_KEYDOWN:		AddressEditbox_OnKeyDown(hwnd, (UINT)wParam, (UINT)lParam); return 0;
		case WM_CHAR:			AddressEditbox_OnChar(hwnd, (UINT)wParam, (UINT)lParam); return 0;
		case WM_SETTEXT:		return AddressEditbox_OnSetText(hwnd, (LPCWSTR)lParam);
		case WM_PASTE:			AddressEditbox_OnPaste(hwnd); return 1;
		case EM_REPLACESEL:		AddressEditbox_OnReplaceSel(hwnd, (BOOL)wParam, (LPCWSTR)lParam); return 0;
		case EM_FINDWORDBREAK:	return AddressEditbox_OnFindWordBreak(hwnd, (INT)wParam, (INT)lParam);
	}
	
	return AddressEditbox_CallOrigWindowProc(hwnd, uMsg, wParam, lParam);
}
