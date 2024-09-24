#include "main.h"
#include "./reflectmsg.h"
#include "./skinnedwnd.h"
#include "./skinnedmenu.h"
#include "./skinnedmenuthreadinfo.h"



BOOL CanReflect(UINT uMsg)
{
	switch(uMsg)
	{
		case WM_DRAWITEM:
		case WM_NOTIFY:	
		case WM_COMMAND:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC:
		case WM_MEASUREITEM:
			return TRUE;
	}
	return FALSE;
}

BOOL ReflectMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bDialog, LRESULT *pResult)
{
	HWND hwndReflect = NULL;

	switch(uMsg)
	{
		case WM_DRAWITEM:			
			hwndReflect = ((LPDRAWITEMSTRUCT)lParam)->hwndItem; 
			if (NULL != hwndReflect && ODT_MENU == ((LPDRAWITEMSTRUCT)lParam)->CtlType)
				hwndReflect = SkinnedMenu::WindowFromHandle((HMENU)hwndReflect);
			break;
		case WM_NOTIFY:				
			hwndReflect = ((LPNMHDR)lParam)->hwndFrom; 
			break;
		case WM_COMMAND:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC: 		
			hwndReflect =(HWND)lParam; 
			break;
		case WM_MEASUREITEM:			
			if (0 == wParam && ODT_MENU == ((MEASUREITEMSTRUCT*)lParam)->CtlType)
			{
				SkinnedMenuThreadInfo *threadInfo;
				if (S_OK == SkinnedMenuThreadInfo::GetInstance(FALSE, &threadInfo))
				{
					hwndReflect = SkinnedMenu::WindowFromHandle(threadInfo->GetActiveMeasureMenu());
					threadInfo->Release();
				}
			}
			break;
	}
	// make sure that hwndReflect is a valid hwnd otherwise it can incorrectly lose valid messages
	// when sent as custom version of normally valid ones ie faked wm_command messages where lparam is -1
	if (NULL != hwndReflect &&
		hwnd != hwndReflect &&
		IsWindow(hwndReflect))
	{
		REFLECTPARAM	 rParam;
		rParam.result = 0;
		rParam.lParam = lParam;
		rParam.hwndFrom = hwnd;
		
		if (REFLECTMESSAGE(hwndReflect, uMsg, wParam, (LPARAM)&rParam)) 
		{
			*pResult = rParam.result;
			if (bDialog)
			{
				switch(uMsg)
				{
					case WM_CTLCOLORBTN:
					case WM_CTLCOLOREDIT:
					case WM_CTLCOLORLISTBOX:
					case WM_CTLCOLORSCROLLBAR:
					case WM_CTLCOLORSTATIC: 	 
						return TRUE;
				}
				SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, (LONGX86)(LONG_PTR)*pResult);
				*pResult = TRUE;
			}
			return TRUE;
		}
	}
	*pResult = 0;
	return FALSE;
}


typedef struct __REFLECTORWND
{
	BOOL bUnicode;
	BOOL bDialog;
	WNDPROC windowProc;
}REFLECTORWND;

static ATOM	REFLECTOR = 0;

static LRESULT CALLBACK Reflector_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HRESULT InstallReflector(HWND hwnd)
{
	if (0 == REFLECTOR)
	{
		 REFLECTOR = GlobalAddAtomW(L"WAREFLECTOR");
		 if (0 == REFLECTOR) return E_UNEXPECTED;
	}

	if (NULL == hwnd || 	!IsWindow(hwnd))
		return E_INVALIDARG;

	if (NULL != GetPropW(hwnd, (LPCWSTR)MAKEINTATOM(REFLECTOR)) ||
		NULL != SkinnedWnd::GetFromHWND(hwnd))
		return S_FALSE;

	REFLECTORWND *prw = (REFLECTORWND*)calloc(1, sizeof(REFLECTORWND));
	if (NULL == prw)
		return E_OUTOFMEMORY;

	ZeroMemory(prw, sizeof(REFLECTORWND));
	
	prw->bUnicode = IsWindowUnicode(hwnd);
	prw->windowProc = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)Reflector_WindowProc);
	
	if (NULL == prw->windowProc ||
		!SetPropW(hwnd, (LPCWSTR)MAKEINTATOM(REFLECTOR), prw))
	{
		RemoveReflector(hwnd);
		return E_FAIL;
	}

	return S_OK;
}

BOOL RemoveReflector(HWND hwnd)
{
	if (!hwnd || !IsWindow(hwnd))
		return FALSE;
	
	REFLECTORWND *prw = (REFLECTORWND*)GetPropW(hwnd, (LPCWSTR)MAKEINTATOM(REFLECTOR));
	RemovePropW(hwnd, (LPCWSTR)MAKEINTATOM(REFLECTOR));
	
	if (NULL == prw)
		return FALSE;

	if (prw->windowProc)
		SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)prw->windowProc);
	
	free(prw);
	return TRUE;

}

static LRESULT CALLBACK Reflector_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	REFLECTORWND *prw = (REFLECTORWND*)GetPropW(hwnd, (LPCWSTR)MAKEINTATOM(REFLECTOR));
	if (NULL == prw || NULL == prw->windowProc)
	{
		return (IsWindowUnicode(hwnd)) ? 
			DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
			DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}
	
	if (WM_DESTROY == uMsg)
	{
		BOOL unicode = prw->bUnicode;
		WNDPROC windowProc = prw->windowProc;
		RemoveReflector(hwnd);
		return (unicode) ? 
			CallWindowProcW(windowProc, hwnd, uMsg, wParam, lParam) : 
			CallWindowProcA(windowProc, hwnd, uMsg, wParam, lParam);
	}

	LRESULT result;
	if (ReflectMessage(hwnd, uMsg, wParam, lParam, prw->bDialog, &result))
		return result;

	return (prw->bUnicode) ? 
		CallWindowProcW(prw->windowProc, hwnd, uMsg, wParam, lParam) : 
		CallWindowProcA(prw->windowProc, hwnd, uMsg, wParam, lParam);
}