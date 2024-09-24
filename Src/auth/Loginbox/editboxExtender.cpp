#include "./editboxExtender.h"
#include "./common.h"

#include <commctrl.h>

#define NAEF_USERFLAGSMASK	0x00FFFFFF
#define NAEF_UNICODE		0x01000000

typedef struct __EDITBOXEXTENDER
{
	WNDPROC originalProc;
	UINT	flags;
	DWORD	dblclkTime;
	LPWSTR	rollbackText;
} EDITBOXEXTENDER;

#define EDITBOXEXTENDER_PROP		L"NullsoftEditboxExtender"

#define GetEditbox(__hwnd) ((EDITBOXEXTENDER*)GetProp((__hwnd), EDITBOXEXTENDER_PROP))

static LRESULT CALLBACK EditboxExtender_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL EditboxExtender_AttachWindow(HWND hEditbox)
{
	if (!IsWindow(hEditbox)) 
		return  FALSE;

	EDITBOXEXTENDER *editbox = (EDITBOXEXTENDER*)GetProp(hEditbox, EDITBOXEXTENDER_PROP);
	if (NULL != editbox) return TRUE;
	
	editbox = (EDITBOXEXTENDER*)calloc(1, sizeof(EDITBOXEXTENDER));
	if (NULL == editbox) return FALSE;

	if (IsWindowUnicode(hEditbox))
		editbox->flags |= NAEF_UNICODE;

	editbox->originalProc = (WNDPROC)(LONG_PTR)((0 != (NAEF_UNICODE & editbox->flags)) ? 
						SetWindowLongPtrW(hEditbox, GWLP_WNDPROC, (LONGX86)(LONG_PTR)EditboxExtender_WindowProc) : 
						SetWindowLongPtrA(hEditbox, GWLP_WNDPROC, (LONGX86)(LONG_PTR)EditboxExtender_WindowProc));

	if (NULL == editbox->originalProc || !SetProp(hEditbox, EDITBOXEXTENDER_PROP, editbox))
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
	return TRUE;
}


static void EditboxExtender_Detach(HWND hwnd)
{	
	EDITBOXEXTENDER *editbox = GetEditbox(hwnd);
	RemoveProp(hwnd, EDITBOXEXTENDER_PROP);

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


static LRESULT EditboxExtender_CallOrigWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	EDITBOXEXTENDER *editbox = GetEditbox(hwnd);

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

static void EditboxExtender_OnDestroy(HWND hwnd)
{
	EDITBOXEXTENDER *editbox = GetEditbox(hwnd);
	
	WNDPROC originalProc = editbox->originalProc;
	BOOL fUnicode = (0 != (NAEF_UNICODE & editbox->flags));

	EditboxExtender_Detach(hwnd);

	if (NULL != originalProc)
	{
		if (FALSE != fUnicode) 
			CallWindowProcW(originalProc, hwnd, WM_DESTROY, 0, 0L);
		else
			CallWindowProcA(originalProc, hwnd, WM_DESTROY, 0, 0L);
	}
}

static LRESULT EditboxExtender_OnGetDlgCode(HWND hwnd, INT vKey, MSG* pMsg)
{
	LRESULT result = EditboxExtender_CallOrigWindowProc(hwnd, WM_GETDLGCODE, (WPARAM)vKey, (LPARAM)pMsg);
	return result;
}

static void EditboxExtender_OnLButtonDown(HWND hwnd, UINT vKey, POINTS pts)
{
	EDITBOXEXTENDER *editbox = GetEditbox(hwnd);
	if (NULL != editbox)
	{
		DWORD clickTime = GetTickCount();
		if (clickTime >= editbox->dblclkTime && clickTime <= (editbox->dblclkTime + GetDoubleClickTime()))
		{
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			return;
		}
	}
	EditboxExtender_CallOrigWindowProc(hwnd, WM_LBUTTONDOWN, (WPARAM)vKey, *((LPARAM*)&pts));
}

static void EditboxExtender_OnLButtonDblClk(HWND hwnd, UINT vKey, POINTS pts)
{
	EDITBOXEXTENDER *editbox = GetEditbox(hwnd);
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
	
		
	EditboxExtender_CallOrigWindowProc(hwnd, WM_LBUTTONDBLCLK, (WPARAM)vKey, *((LPARAM*)&pts));

}

static void EditboxExtender_OnChar(HWND hwnd, UINT vKey, UINT state)
{	
	if (0 == (0x8000 & GetAsyncKeyState(VK_CONTROL)) && 0 == (0x8000 & GetAsyncKeyState(VK_MENU)))
	{
		HWND hParent = GetAncestor(hwnd, GA_PARENT);
		if (NULL != hParent)
		{
			NMCHAR nmh;
			nmh.hdr.hwndFrom = hwnd;
			nmh.hdr.idFrom = (INT)(INT_PTR)GetWindowLongPtr(hwnd, GWLP_ID);
			nmh.hdr.code = NM_CHAR;
			nmh.ch = vKey;
			nmh.dwItemNext = 0;
			nmh.dwItemPrev = 0;
			if (FALSE != (BOOL)SendMessage(hParent, WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh))
				return;
		}
	}
	EditboxExtender_CallOrigWindowProc(hwnd, WM_CHAR, (WPARAM)vKey, (LPARAM)state);
}

static void EditboxExtender_OnKeyDown(HWND hwnd, UINT vKey, UINT state)
{	
	HWND hParent = GetAncestor(hwnd, GA_PARENT);
	if (NULL != hParent)
	{
		NMKEY nmh;
		nmh.hdr.hwndFrom = hwnd;
		nmh.hdr.idFrom = (INT)(INT_PTR)GetWindowLongPtr(hwnd, GWLP_ID);
		nmh.hdr.code = NM_KEYDOWN;
		nmh.nVKey = vKey;
		nmh.uFlags = HIWORD(state);
		if (FALSE != (BOOL)SendMessage(hParent, WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh))
			return;
	}

	EditboxExtender_CallOrigWindowProc(hwnd, WM_KEYDOWN, (WPARAM)vKey, (LPARAM)state);
}

static void EditboxExtender_PasteText(HWND hwnd, LPCWSTR pszClipboard)
{	
	HWND hParent = GetAncestor(hwnd, GA_PARENT);
	if (NULL != hParent)
	{
		EENMPASTE nmh;
		nmh.hdr.hwndFrom = hwnd;
		nmh.hdr.idFrom = (INT)(INT_PTR)GetWindowLongPtr(hwnd, GWLP_ID);
		nmh.hdr.code = EENM_PASTE;
		nmh.text = pszClipboard;
		if (FALSE != (BOOL)SendMessage(hParent, WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh))
			return;
	}

	SendMessage(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)pszClipboard);
}

static void EditboxExtender_OnPaste(HWND hwnd)
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
			EditboxExtender_PasteText(hwnd, pClipboard);
			
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

				EditboxExtender_PasteText(hwnd, pClipboard);
		
				LoginBox_FreeString(pClipboard);
      			GlobalUnlock(stgm.hGlobal);
				ReleaseStgMedium(&stgm);
			}
		}
  		pObject->Release();
	}
}

static LRESULT CALLBACK EditboxExtender_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DESTROY:		EditboxExtender_OnDestroy(hwnd); return 0;
		case WM_GETDLGCODE:		return EditboxExtender_OnGetDlgCode(hwnd, (INT)wParam, (MSG*)lParam);
		case WM_LBUTTONDOWN:	EditboxExtender_OnLButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_LBUTTONDBLCLK:	EditboxExtender_OnLButtonDblClk(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_CHAR:			EditboxExtender_OnChar(hwnd, (UINT)wParam, (UINT)lParam); return 0;
		case WM_KEYDOWN:		EditboxExtender_OnKeyDown(hwnd, (UINT)wParam, (UINT)lParam); return 0;
		case WM_PASTE:			EditboxExtender_OnPaste(hwnd); return 1;
	}
	
	return EditboxExtender_CallOrigWindowProc(hwnd, uMsg, wParam, lParam);
}
