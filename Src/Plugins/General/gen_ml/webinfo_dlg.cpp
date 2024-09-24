#include "main.h"
#include "api__gen_ml.h"
#include "./resource.h"
#include "./webinfo_obj.h"
#include "../nu/mtbrowser.h"
#include "./ml_ipc_0313.h"
#include "./stockobjects.h"
#include "../nu/CGlobalAtom.h"
#include <strsafe.h>

#define BROWSER_FORCEQUITDELAY	5000
#define BROWSER_TERMINATEDELAY	3000

#define BROWSER_CACHEINTERVAL	3000

#define TIMER_BROWSERFORCEQUIT_ID		1975
#define TIMER_BROWSERCACHE_ID			1976
#define TIMER_BROWSERLOADINGNOTICE_ID	1977

#define TIMER_BROWSERLOADINGNOTICE_DELAY		80

#define WEBINFOWND_CLASSW	L"WAWEBINFOWND"
static CGlobalAtom BROWSERWND_PROPW(L"BWPROP");

#define WM_BROWSERNOTIFY		(WM_USER  + 0xF000)

#define IDC_LBL_INFO		0x100
#define IDC_BROWSER			0x1000


typedef struct _BROWSERVIEW
{
    MTBROWSER	browser;
	BOOL		bBrowserReady;
	BOOL		bDestroying;
	UINT		uMsgQuery;
	LPWSTR		pszCachedFileName;
} BROWSERVIEW;


static HWND hwndCached = NULL;
static LRESULT WINAPI WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void CALLBACK Window_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

#define GetBrowserView(_hwnd) ((BROWSERVIEW*)GetPropW(_hwnd, BROWSERWND_PROPW))


// public function
HWND CreateWebInfoWindow(HWND hwndParent, UINT uMsgQuery, INT x, INT y, INT cx, INT cy, INT ctrlId)
{
	HWND hwnd;
	BROWSERVIEW *pbv;
	WNDCLASSW	wc;

	if (hwndCached && IsWindow(hwndCached))
	{
		pbv = GetBrowserView(hwndCached);
		if (pbv)
		{			
			KillTimer(hwndCached, TIMER_BROWSERCACHE_ID);
			hwnd = hwndCached;
			hwndCached = NULL;
			
			pbv->uMsgQuery = uMsgQuery;
			pbv->bDestroying = FALSE;
			
			pbv->pszCachedFileName = NULL;
			ShowWindow(hwnd, SW_HIDE);
			SetParent(hwnd, hwndParent);
			SetWindowPos(hwnd, HWND_TOP, x, y, cx, cy, SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
			SetWindowLongPtrW(hwnd, GWLP_ID, ctrlId);
			return hwnd;
		}
	}
		
	if (!GetClassInfoW(plugin.hDllInstance, WEBINFOWND_CLASSW, &wc))
	{
		ZeroMemory(&wc, sizeof(WNDCLASSW));
		wc.hInstance		= plugin.hDllInstance;
		wc.lpszClassName	= WEBINFOWND_CLASSW;
		wc.lpfnWndProc		= WindowProc;
		wc.style			= CS_DBLCLKS;
		if (!RegisterClassW(&wc)) return NULL;
	}

	pbv = (BROWSERVIEW*)calloc(1, sizeof(BROWSERVIEW));
	if (!pbv) return NULL;
	pbv->uMsgQuery = uMsgQuery;

	hwnd = CreateWindowExW(WS_EX_CONTROLPARENT, WEBINFOWND_CLASSW, L"", 
								DS_CONTROL | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
								x, y, cx, cy, hwndParent, (HMENU)(INT_PTR)ctrlId, plugin.hDllInstance, (LPVOID)pbv);
	if (!hwnd) free(pbv);
	else
	{
		HFONT font;
		HWND hwndCtrl;
		
		hwndCtrl = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"STATIC", L"",
							WS_CHILD | SS_OWNERDRAW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
							0, 0, 1, 1, hwnd, (HMENU)IDC_LBL_INFO, NULL, 0);
		
		font = (HFONT)MlStockObjects_Get(DEFAULT_FONT);
		SendMessageW(hwnd, WM_SETFONT, (WPARAM)font, FALSE);
		if (hwndCtrl) SendMessageW(hwndCtrl, WM_SETFONT, (WPARAM)font, FALSE);
	}

	return hwnd;
}


static void CALLBACK APC_NavigateToPage(HTMLContainer2 *pContainer, VARIANTARG *pArgs, INT cArgs, LPARAM *pResult)
{
	WebFileInfo *pWebInfo;
	pWebInfo = reinterpret_cast<WebFileInfo*>(pContainer);
	pWebInfo->NavigateToPage();
}

static void CALLBACK APC_InvokeFileInfo(HTMLContainer2 *pContainer, VARIANTARG *pArgs, INT cArgs, LPARAM *pResult)
{
	WebFileInfo *pWebInfo;
	pWebInfo = reinterpret_cast<WebFileInfo*>(pContainer);
	*pResult = (pWebInfo && 1 == cArgs) ? pWebInfo->InvokeFileInfo(pArgs[0].bstrVal) : E_INVALIDARG;
}

static void CALLBACK APC_DisplayMessage(HTMLContainer2 *pContainer, VARIANTARG *pArgs, INT cArgs, LPARAM *pResult)
{
	WebFileInfo *pWebInfo;
	pWebInfo = reinterpret_cast<WebFileInfo*>(pContainer);
	*pResult = (pWebInfo && 1 == cArgs) ? pWebInfo->DisplayMessage(pArgs[0].bstrVal, FALSE) : E_INVALIDARG;
}

static void CALLBACK APC_GetHostHWND(HTMLContainer2 *pContainer, VARIANTARG *pArgs, INT cArgs, LPARAM *pResult)
{
	if (1 == cArgs)
	{		
		*((HWND*)pArgs[0].byref) = pContainer->GetHostHWND();
		*pResult = S_OK;
	}
	else *pResult = E_INVALIDARG;
}

static void CALLBACK APC_RegisterCursor(HTMLContainer2 *pContainer, VARIANTARG *pArgs, INT cArgs, LPARAM *pResult)
{
	if (1 == cArgs) 
	{				
		HCURSOR hCur = (pArgs[0].byref) ? CopyCursor((HCURSOR)pArgs[0].byref) : NULL;
		*pResult = pContainer->RegisterBrowserCursor(/*OCR_NORMAL*/32512, hCur);
	}
	else *pResult = E_INVALIDARG;
}

static void CALLBACK APC_UpdateColors(HTMLContainer2 *pContainer, VARIANTARG *pArgs, INT cArgs, LPARAM *pResult)
{
	WebFileInfo *pWebInfo;
	pWebInfo = reinterpret_cast<WebFileInfo*>(pContainer);
	if (pWebInfo) pWebInfo->UpdateColors();
}

static void DisplayInfoText(HWND hwndView, LPCWSTR pszText)
{
	HWND hwndCtrl;
	hwndCtrl = GetDlgItem(hwndView, IDC_LBL_INFO);
	if (!hwndCtrl) return;
	if (pszText)
	{
		SetWindowTextW(hwndCtrl, pszText);
		if (!IsWindowVisible(hwndCtrl))
		{
			RECT rc;
			GetClientRect(hwndView, &rc);
			SetWindowPos(hwndCtrl, NULL, 8, 8, rc.right - 16, rc.bottom - 16, SWP_NOACTIVATE | SWP_NOZORDER);
			ShowWindow(hwndCtrl, SW_SHOWNORMAL);
		}
	}
	else ShowWindow(hwndCtrl, SW_HIDE);
}

static LRESULT Window_OnCreate(HWND hwnd, CREATESTRUCT *pcs)
{
	BROWSERVIEW *pbv;
	
	pbv =(BROWSERVIEW*)pcs->lpCreateParams;
	if (pbv)
	{
		SetPropW(hwnd, BROWSERWND_PROPW, (HANDLE)pbv);
		pbv->bBrowserReady = FALSE;
		MTBrowser_Init(&pbv->browser);
	}
	return 0;
}

static void Window_OnDestroy(HWND hwnd)
{
	BROWSERVIEW *pbv;
	pbv = GetBrowserView(hwnd);
	RemovePropW(hwnd, BROWSERWND_PROPW);
	KillTimer(hwnd, TIMER_BROWSERFORCEQUIT_ID);

	if (pbv) 
	{
		pbv->bBrowserReady = FALSE;
		
		MTBrowser_Kill(&pbv->browser, BROWSER_TERMINATEDELAY);
		MTBrowser_Clear(&pbv->browser);
	}
	free(pbv);

	CoFreeUnusedLibraries();

}
static BOOL UpdateCursors(BROWSERVIEW *pbv)
{
	HCURSOR hCur;
	HAPC hAPC;
	VARIANTARG *pArgs;

	if (!pbv) return FALSE;
	
	hCur = (HCURSOR)SendMessageW(plugin.hwndParent, WM_WA_IPC, WACURSOR_NORMAL, IPC_GETSKINCURSORS);
				
	hAPC = MTBrowser_InitializeAPC(&pbv->browser, 1, 0, APC_RegisterCursor, &pArgs);
	if (!hAPC) return FALSE;
	
	pArgs[0].vt	= VT_BYREF;
	V_BYREF(&pArgs[0]) = hCur;

	return MTBrowser_CallAPC(hAPC);
}

static void Window_OnDisplayChange(HWND hwnd, INT dpi, INT resX, INT resY)
{
	HWND hwndCtrl;
	BROWSERVIEW *pbv;
	
	pbv = GetBrowserView(hwnd);
	if (pbv)
	{
		HAPC hAPC;

		UpdateCursors(pbv);

		hAPC = MTBrowser_InitializeAPC(&pbv->browser, 0, 0, APC_UpdateColors, NULL);
		if (hAPC) MTBrowser_CallAPC(hAPC);
	}
	InvalidateRect(hwnd, NULL, TRUE);
	hwndCtrl = GetDlgItem(hwnd, IDC_LBL_INFO);
	if (hwndCtrl) InvalidateRect(hwndCtrl, NULL, TRUE);
}

static void Window_OnShowWindow(HWND hwnd, BOOL bVisible, INT nStatus)
{
	BROWSERVIEW *pbv;

	pbv = GetBrowserView(hwnd);
	if (pbv && !pbv->browser.hThread && bVisible)
	{
		WebFileInfo *pWebInfo;
		IDispatch *pDispWA;
		pDispWA = (IDispatch *)SendMessageW(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_DISPATCH_OBJECT);
		if (pDispWA == (IDispatch*)1) pDispWA = NULL;
		
		pWebInfo = CreateWebFileInfo(hwnd, pDispWA);
		if (NULL != pWebInfo)
		{			
			SetTimer(hwnd, TIMER_BROWSERLOADINGNOTICE_ID, TIMER_BROWSERLOADINGNOTICE_DELAY, Window_TimerProc);
			MTBrowser_Start(&pbv->browser, pWebInfo, WM_BROWSERNOTIFY);
		}
		else
		{
			DisplayInfoText(hwnd, WASABI_API_LNGSTRINGW(IDS_WEBINFO_NAVIGATE_ERROR));
		}

		if (NULL != pDispWA)
			pDispWA->Release();
		
	}

	if(bVisible && pbv && pbv->uMsgQuery)
	{
		HWND hwndParent;
		hwndParent = GetParent(hwnd);
		if (hwndParent) PostMessageW(hwndParent, pbv->uMsgQuery, 0, 0L);
	}
}

static LRESULT Window_OnEraseBkGnd(HWND hwnd, HDC hdc)
{
	if (hdc)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		SetBkColor(hdc, WADlg_getColor(WADLG_ITEMBG));
		ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rc, L"", 0, 0);
	}
	return 1;
}

static void Window_OnShowInfo(HWND hwnd, WEBINFOSHOW *pShowParam)
{
	HAPC hAPC;
	VARIANTARG *pArgs;
	BROWSERVIEW *pbv = GetBrowserView(hwnd);
	
	if (!IsWindowVisible(hwnd) || !pbv || !pbv->bBrowserReady || !pShowParam) 
	{
		if (pbv && pbv->pszCachedFileName) 
		{
			free(pbv->pszCachedFileName);
			pbv->pszCachedFileName = NULL;

		}
		return;
	}

	if (pShowParam->pszFileName)
	{	
		if (0 == (WISF_FORCE & pShowParam->fFlags) && pbv->pszCachedFileName &&
				0 == lstrcmpW(pbv->pszCachedFileName, pShowParam->pszFileName)) return;
		else
		{
			if (pbv->pszCachedFileName) free(pbv->pszCachedFileName);
			pbv->pszCachedFileName = _wcsdup(pShowParam->pszFileName);
		}
	}
	else
	{
		if (0 == (WISF_FORCE & pShowParam->fFlags) && !pbv->pszCachedFileName) return;
		if (pbv->pszCachedFileName)
		{
			free(pbv->pszCachedFileName);
			pbv->pszCachedFileName = NULL;
		}
	}
	
	hAPC = MTBrowser_InitializeAPC(&pbv->browser, 1, 0, (WISF_MESSAGE & pShowParam->fFlags) ? APC_DisplayMessage : APC_InvokeFileInfo , &pArgs);
	if (hAPC)
	{
		pArgs[0].vt = VT_BSTR;
		pArgs[0].bstrVal = SysAllocString(pShowParam->pszFileName);
		MTBrowser_CallAPC(hAPC);
	}
}

static void Window_OnWindowPosChanging(HWND hwnd, WINDOWPOS *pwp)
{
	if (0 == (SWP_NOSIZE & pwp->flags))
	{
		BROWSERVIEW *pbv;
		pbv = GetBrowserView(hwnd);
		if (pbv && pbv->bBrowserReady)
		{
			RECT rc;
			HWND hBrowser = GetDlgItem(hwnd, IDC_BROWSER);
			if (NULL != hBrowser && FALSE != GetClientRect(hBrowser, &rc) &&
				(rc.right != pwp->cx || rc.bottom != pwp->cy))
			{
				SetWindowPos(hBrowser, NULL, 0, 0, pwp->cx, pwp->cy, 
						SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS | ((SWP_NOREDRAW | SWP_NOCOPYBITS) & pwp->flags));
			}
		}
	}
}

static void Window_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (0 == (SWP_NOSIZE & pwp->flags))
	{
		HWND hwndCtrl;
		hwndCtrl = GetDlgItem(hwnd, IDC_LBL_INFO);
		if (hwndCtrl && WS_VISIBLE == (WS_VISIBLE & GetWindowLongPtrW(hwnd, GWL_STYLE)))
		{
			SetWindowPos(hwndCtrl, NULL, 0, 0, pwp->cx - 16, pwp->cy - 16, 
										SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);
			if (0 == (SWP_NOREDRAW & pwp->flags)) InvalidateRect(hwndCtrl, NULL, FALSE);
		}
		if (0 == (SWP_NOREDRAW & pwp->flags))
		{
			HWND hBrowser = GetDlgItem(hwnd, IDC_BROWSER);
			if (NULL == hBrowser || 0 == (WS_VISIBLE & GetWindowLongPtr(hBrowser, GWL_STYLE))) 
				InvalidateRect(hwnd, NULL, TRUE);
		}
	}
}

static void DrawInfoLabel(DRAWITEMSTRUCT *pds)
{
	wchar_t szText[128] = {0};
	SetBkColor(pds->hDC, WADlg_getColor(WADLG_ITEMBG));

	INT len = GetWindowTextW(pds->hwndItem, szText, sizeof(szText)/sizeof(wchar_t));
	ExtTextOutW(pds->hDC, 0, 0, ETO_OPAQUE, &pds->rcItem, L"", 0, 0);
	if(len)
	{
		RECT rt;
		INT height, x, y;
		CopyRect(&rt, &pds->rcItem);

		height = DrawTextW(pds->hDC, szText, len, &rt, DT_CALCRECT | DT_CENTER | DT_WORDBREAK);
		SetTextColor(pds->hDC, WADlg_getColor(WADLG_ITEMFG));
		
		y = pds->rcItem.top + ((pds->rcItem.bottom - pds->rcItem.top) - height)/2;
		if (y < pds->rcItem.top) y = pds->rcItem.top;
		x = pds->rcItem.left + ((pds->rcItem.right - pds->rcItem.left) - (rt.right - rt.left))/2;
		
		SetRect(&rt, x, y, x + rt.right- rt.left, y + rt.bottom - rt.top);

		DrawTextW(pds->hDC, szText, len, &rt, DT_CENTER | DT_WORDBREAK);
	}
}

static LRESULT Window_OnDrawItem(HWND hwnd, INT ctrlID, DRAWITEMSTRUCT *pds)
{
	switch(ctrlID)
	{
		case IDC_LBL_INFO: DrawInfoLabel(pds); return 1;
	}
	return 0;
}

static void Browser_OnDocumentComplete(HWND hwndView, BROWSERVIEW *pbv, BOOL bDocReady)
{
	if (bDocReady)
	{
		RECT rc;
		
		KillTimer(hwndView, TIMER_BROWSERLOADINGNOTICE_ID);
		DisplayInfoText(hwndView, NULL); // hide 

		GetClientRect(hwndView, &rc);
		MTBrowser_SetLocationAPC(&pbv->browser, &rc);
	}
}

static void Browser_OnDestroyed(HWND hwndView, BROWSERVIEW *pbv)
{
	if (pbv->bDestroying) DestroyWindow(hwndView);
	else {} // browser died for some reason
}

static void Browser_OnReady(HWND hwndView, BROWSERVIEW *pbv)
{
	if (NULL == pbv) return;
	pbv->bBrowserReady = TRUE;
	
	
	HWND hBrowser = (NULL != pbv->browser.pContainer) ? pbv->browser.pContainer->GetHostHWND() : NULL;
	if (NULL != hBrowser) 
		SetWindowLongPtr(hBrowser, GWLP_ID, IDC_BROWSER);

	UpdateCursors(pbv);
	HWND hParent = GetParent(hwndView);
	if (NULL != hParent && 0 != pbv->uMsgQuery) PostMessageW(hParent, pbv->uMsgQuery, 0, 0L);
}

static void CALLBACK Window_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	BROWSERVIEW *pbv;
	switch(idEvent)
	{
		case TIMER_BROWSERFORCEQUIT_ID:
			KillTimer(hwnd, TIMER_BROWSERFORCEQUIT_ID);
			DestroyWindow(hwnd);
			break;
		case TIMER_BROWSERCACHE_ID:
			KillTimer(hwnd, TIMER_BROWSERCACHE_ID);
			pbv = GetBrowserView(hwnd);
			
			if (hwnd == hwndCached)  hwndCached = NULL;
						
			if (pbv && pbv->bDestroying)
			{
				pbv->bBrowserReady = FALSE;
				MTBrowser_QuitAPC(&pbv->browser);
				SetTimer(hwnd, TIMER_BROWSERFORCEQUIT_ID, BROWSER_FORCEQUITDELAY, Window_TimerProc);
			}
			else DestroyWindow(hwnd);
			break;
		case TIMER_BROWSERLOADINGNOTICE_ID:
			KillTimer(hwnd, TIMER_BROWSERLOADINGNOTICE_ID);
			DisplayInfoText(hwnd, WASABI_API_LNGSTRINGW(IDS_LOADING_IN_PROGRESS));
			break;
	}
}

static void Window_OnRelease(HWND hwnd)
{
	BROWSERVIEW *pbv;
	pbv = GetBrowserView(hwnd);
	if (pbv)
	{
		hwndCached = hwnd;
		pbv->bDestroying = TRUE;
		if (pbv->pszCachedFileName) 
		{
			free(pbv->pszCachedFileName);
			pbv->pszCachedFileName = NULL;
		}
		SetWindowPos(hwnd, HWND_BOTTOM, -30000, -30000, 1, 1, SWP_ASYNCWINDOWPOS | SWP_HIDEWINDOW | SWP_NOACTIVATE |
													SWP_NOREDRAW | SWP_DEFERERASE | SWP_NOCOPYBITS | SWP_NOSENDCHANGING);
		SetParent(hwnd, plugin.hwndParent);
		SetTimer(hwnd, TIMER_BROWSERCACHE_ID, BROWSER_CACHEINTERVAL, Window_TimerProc);
	}
	else DestroyWindow(hwnd);
}

static void Window_OnBrowserNotify(HWND hwnd, INT nCode, LPARAM lParam)
{
	BROWSERVIEW *pbv;
	pbv = GetBrowserView(hwnd);
	if (!pbv) return; 
	switch(nCode)
	{
		case MTBC_READY:				Browser_OnReady(hwnd, pbv); break;
		case MTBC_DESTROYED:			Browser_OnDestroyed(hwnd, pbv); break;
		case MTBC_DOCUMENTCOMPLETE:		Browser_OnDocumentComplete(hwnd, pbv, (BOOL)lParam); break;
	}
}

static LRESULT Window_OnMediaLibraryIPC(HWND hwndView, INT msg, INT_PTR param)
{
	switch(msg)
	{
		case ML_IPC_WEBINFO_RELEASE:		Window_OnRelease(hwndView); return TRUE;
		case ML_IPC_WEBINFO_SHOWINFO:	Window_OnShowInfo(hwndView, (WEBINFOSHOW*)param); return TRUE;
	}
	return 0;
}

static LRESULT WINAPI WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return (LRESULT)Window_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			Window_OnDestroy(hwnd); break;
		case WM_DISPLAYCHANGE:		Window_OnDisplayChange(hwnd, (INT)wParam, LOWORD(lParam), HIWORD(lParam)); break;
		case WM_SHOWWINDOW:			Window_OnShowWindow(hwnd, (BOOL)wParam, (INT)lParam); break;
		case WM_WINDOWPOSCHANGING:	Window_OnWindowPosChanging(hwnd, (WINDOWPOS*)lParam); return 0;	
		case WM_WINDOWPOSCHANGED:	Window_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;	
		case WM_ERASEBKGND:			return (LRESULT)Window_OnEraseBkGnd(hwnd, (HDC)wParam);
		case WM_DRAWITEM:			return Window_OnDrawItem(hwnd, (INT)wParam, (DRAWITEMSTRUCT*)lParam);
		case WM_BROWSERNOTIFY:		Window_OnBrowserNotify(hwnd, (INT)wParam, lParam); return 1;
		case WM_ML_IPC:
			return Window_OnMediaLibraryIPC(hwnd, (INT)lParam, (INT_PTR)wParam);	
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}