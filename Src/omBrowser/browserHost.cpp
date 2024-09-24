#include "main.h"
#include "./browser.h"
#include "./browserHost.h"
//#include "./browserPopup.h"
#include "./graphics.h"

//#include "../winamp/wa_dlg.h"
#include "../Plugins/General/gen_ml/colors.h"

#include "./browserThread.h"

#include "../winamp/IWasabiDispatchable.h"
#include "../winamp/JSAPI_Info.h"

#include "./service.h"
//#include "./menu.h"

#include "./obj_ombrowser.h"
#include "./ifc_skinhelper.h"
#include "./ifc_skinnedbrowser.h"
#include "./ifc_omservice.h"
#include "./ifc_travelloghelper.h"
#include "./ifc_wasabihelper.h"
#include <exdispid.h>
#include <strsafe.h>

#define NWC_OMBROWSERHOST		L"Nullsoft_omBrowserHost"

typedef struct __BROWSERHOSTCREATEPARAM
{
	HWND hParent;
	INT controlId;
	UINT fStyle;
	INT x;
	INT y;
	INT cx;
	INT cy;
	HINSTANCE hInstance;
	HACCEL hAccel;
	obj_ombrowser *browserManager;
} BROWSERHOSTCREATEPARAM;

typedef struct __BROWSERHOST
{
	Browser *container;
	HACCEL hAccel;
	obj_ombrowser *browserManager;
} BROWSERHOST;


#define BHT_CONTAINERDOWNLOAD			27
#define BHT_ACTIVATECONTAINER_DELAY		15
#define BHT_DEACTIVATECONTAINER_DELAY	75

#define GetHost(__hwnd) ((BROWSERHOST*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))

static LRESULT CALLBACK BrowserHost_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static void CALLBACK BrowserHost_OnDocumentReady(Browser *browser, IDispatch *pDispatch, VARIANT *URL);
static void CALLBACK BrowserHost_OnNavigateComplete(Browser *browser, IDispatch *pDispatch, VARIANT *URL);
static void CALLBACK BrowserHost_OnContainerDestroy(Browser *browser);
static void CALLBACK BrowserHost_OnDownloadBegin(Browser *browser);
static void CALLBACK BrowserHost_OnDownloadComplete(Browser *browser);
static void CALLBACK BrowserHost_OnCommandStateChange(Browser *browser, INT commandId, BOOL fEnabled);
static void CALLBACK BrowserHost_OnStatusChange(Browser *browser, LPCWSTR pszText);
static void CALLBACK BrowserHost_OnTitleChange(Browser *browser, LPCWSTR pszText);
static void CALLBACK BrowserHost_OnSecureLockIconChange(Browser *browser);
static void CALLBACK BrowserHost_OnCreatePopup(Browser *browser, IDispatch **ppDisp, VARIANT_BOOL *Cancel);
static void CALLBACK BrowserHost_OnVisibleChange(Browser *browser, VARIANT_BOOL fVisible);
static void CALLBACK BrowserHost_OnSetResizable(Browser *browser, VARIANT_BOOL fAllow);
static void CALLBACK BrowserHost_OnSetFullScreen(Browser *browser, VARIANT_BOOL fAllow);
static void CALLBACK BrowserHost_OnWindowClosing(Browser *browser, VARIANT_BOOL fIsChild, VARIANT_BOOL *fCancel);
static void CALLBACK BrowserHost_OnShowUiElenent(Browser *browser, UINT elementId, VARIANT_BOOL fShow);
static void CALLBACK BrowserHost_OnClientToHost(Browser *browser, LONG *cx, LONG *cy);
static void CALLBACK BrowserHost_OnSetWindowPos(Browser *browser, UINT Flags, LONG x, LONG y, LONG cx, LONG cy);
static void CALLBACK BrowserHost_OnFocusChange(Browser *browser, VARIANT_BOOL *fAllow);
static void CALLBACK BrowserHost_OnClosePopup(Browser *browser);

static HRESULT CALLBACK BrowserHost_GetOmService(Browser *browser, ifc_omservice **ppService);
static LRESULT CALLBACK BrowserHost_RedirectKey(Browser *browser, MSG *pMsg);

static BOOL BrowserHost_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc = {0};
	if (GetClassInfo(hInstance, NWC_OMBROWSERHOST, &wc)) return TRUE;

	ZeroMemory(&wc, sizeof(WNDCLASS));
	wc.hInstance = hInstance;
	wc.lpszClassName = NWC_OMBROWSERHOST;
	wc.lpfnWndProc = BrowserHost_WindowProc;
	wc.style = CS_DBLCLKS;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.cbWndExtra = sizeof(BROWSERHOST*);

	return ( 0 != RegisterClassW(&wc));
}

static HWND CALLBACK BrowserHost_CreateHostWindow(ULONG_PTR user)
{
	BROWSERHOSTCREATEPARAM *param = (BROWSERHOSTCREATEPARAM*)user;
	if (NULL == param) return NULL;

    return CreateWindowEx(WS_EX_NOPARENTNOTIFY | WS_EX_CONTROLPARENT, 
						  NWC_OMBROWSERHOST, NULL,
						  WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | (0x00000FFF & param->fStyle),
						  param->x, param->y, param->cx, param->cy,
						  param->hParent, (HMENU)(INT_PTR)param->controlId,
						  param->hInstance, param);
}

static BOOL CALLBACK BrowserHost_KeyFilter(HWND hwnd, MSG *pMsg)
{	
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL == host) return FALSE;

	UINT vKey = (UINT)pMsg->wParam;
	switch(vKey)
	{
		case VK_CONTROL:
		case VK_MENU:
		case VK_SHIFT:
			return TRUE;
	}

	if (NULL != host->hAccel && (hwnd == pMsg->hwnd || IsChild(hwnd, pMsg->hwnd)))
	{
		HWND hParent = GetParent(hwnd);
		if (NULL != hParent && 0 != TranslateAccelerator(hParent, host->hAccel, pMsg))
			return TRUE;
	}

	return (NULL != host->container &&
			FALSE != host->container->TranslateKey(pMsg));
}

HWND BrowserHost_CreateWindow(obj_ombrowser *browserManager, HWND hParent, UINT fStyle, INT x, INT y, INT cx, INT cy, INT controlId, HACCEL hAccel)
{
	if (FALSE == BrowserHost_RegisterClass(Plugin_GetInstance()))
		return NULL;

	BROWSERHOSTCREATEPARAM param = {0};

	param.hParent = hParent;
	param.controlId = controlId;
	param.hInstance = Plugin_GetInstance();
	param.x = x;
	param.y = y;
	param.cx = cx;
	param.cy = cy;
	param.fStyle = fStyle;
	param.hAccel = hAccel;
	param.browserManager = browserManager;

	HWND hWinamp = NULL;
	if (FAILED(Plugin_GetWinampWnd(&hWinamp)))
		hWinamp = NULL;

	HWND hHost = NULL;
	HANDLE hThread = BrowserThread_Create(hWinamp, BrowserHost_CreateHostWindow, (ULONG_PTR)&param, BrowserHost_KeyFilter, &hHost, NULL);
	if (NULL != hThread)
		CloseHandle(hThread);
	
	return hHost;
}

static LRESULT BrowserHost_SendBrowserNotification(Browser *browser, INT code, NMHDR *pnmh)
{
	HWND hHost = (NULL != browser) ? browser->GetParentHWND() : NULL;
	if (NULL == hHost || NULL == pnmh)
		return 0;

	HWND hParent = GetParent(hHost);
	if (NULL == hParent)
		return 0;

	pnmh->code = code;
	pnmh->hwndFrom = hHost;
	pnmh->idFrom = GetDlgCtrlID(hHost);
	
	return SendMessage(hParent, WM_NOTIFY, (WPARAM)pnmh->idFrom, (LPARAM)pnmh);
}

static LRESULT BrowserHost_SendNotification(HWND hwnd, NMHDR *phdr)
{
	HWND hParent = GetParent(hwnd);
	if (NULL == hParent || NULL == phdr || !IsWindow(hParent))
		return 0;

	return SendMessage(hParent, WM_NOTIFY, (WPARAM)phdr->idFrom, (LPARAM)phdr);
}

static void BrowserHost_UpdateLayout(HWND hwnd, BOOL fRedraw)
{
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL == host) return;

	RECT clientRect;
	if (!GetClientRect(hwnd, &clientRect))
		return;

	if (NULL != host->container)
	{
		host->container->SetLocation(clientRect.left, clientRect.top, 
									 clientRect.right - clientRect.left,
									 clientRect.bottom - clientRect.top);
	}
}

static COLORREF BrwoserHost_GetBackColor(HWND hwnd)
{
	COLORREF rgb;	

	ifc_skinnedbrowser *skinnedBrowser = NULL;
	if (SUCCEEDED(Plugin_GetBrowserSkin(&skinnedBrowser)) && skinnedBrowser != NULL)
	{
		rgb = skinnedBrowser->GetBackColor();
		skinnedBrowser->Release();
	}
	else
		rgb = GetSysColor(COLOR_WINDOW);
	
	return rgb;
}

static void BrowserHost_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{	
	SetBkColor(hdc, BrwoserHost_GetBackColor(hwnd));
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prcPaint, NULL, 0, NULL);
}

static LRESULT BrowserHost_OnCreate(HWND hwnd, CREATESTRUCT *pcs)
{		
	BROWSERHOST *host= (BROWSERHOST*)calloc(1, sizeof(BROWSERHOST));
	if (NULL != host)
	{
		SetLastError(ERROR_SUCCESS);
		if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)host) && ERROR_SUCCESS != GetLastError())
		{
			free(host);
			host = NULL;
		}
	}

	if (NULL == host)
	{
		DestroyWindow(hwnd);
		return -1;
	}

	BROWSERHOSTCREATEPARAM *param = (BROWSERHOSTCREATEPARAM*)pcs->lpCreateParams;
	host->hAccel = param->hAccel;
	host->browserManager = param->browserManager;
	if (NULL != host->browserManager)
		host->browserManager->AddRef();

	return 0;
}

static void BrowserHost_OnDestroy(HWND hwnd)
{
	BROWSERHOST *host = GetHost(hwnd);
	/*if (NULL != host->container)
	{
	//	SendMessage(hwnd, BTM_RELEASECONTAINER, 0, 0L);
	//	BrowserThread_SetFlags(BHTF_BEGINDESTROY, BHTF_BEGINDESTROY, FALSE);
	}*/

	SetWindowLongPtr(hwnd, 0, 0L);

	if (NULL != host)
	{
		if (NULL != host->browserManager)
		{
			host->browserManager->Release();
		}
		free(host);
	}

#ifdef _DEBUG
	aTRACE_FMT("[%d] %S: host window destroyed\r\n", GetCurrentThreadId(), OMBROWSER_NAME);
#endif // _DEBUG
}

static void BrowserHost_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps = {0};
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right)
			BrowserHost_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}	
}

static void BrowserHost_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	BrowserHost_Paint(hwnd, hdc, &clientRect, 0 != (PRF_ERASEBKGND & options));
}

static void BrowserHost_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags))
		return;
	BrowserHost_UpdateLayout(hwnd, 0 == (SWP_NOREDRAW & pwp->flags));
}

static void BrowserHost_OnCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
}

static HRESULT BrowserHost_AssociateDispatch(HWND hwnd, IDispatch *pDispatch)
{
	if (NULL == pDispatch)
		return E_INVALIDARG;

	IWasabiDispatchable *pWasabi = NULL;
	HRESULT hr = pDispatch->QueryInterface(IID_IWasabiDispatchable, (void**)&pWasabi);
	if (SUCCEEDED(hr) && pWasabi != NULL)
	{
		JSAPI::ifc_info *pInfo = NULL;
		hr = pWasabi->QueryDispatchable(JSAPI::IID_JSAPI_ifc_info, (Dispatchable**)&pInfo);
		if (SUCCEEDED(hr) && pInfo != NULL)
		{
			HWND hParent = GetParent(hwnd);
			if (NULL == hParent) hParent = hwnd;

			pInfo->SetHWND(hParent);

			WCHAR szBuffer[512] = {0};
			BROWSERHOST *host = GetHost(hwnd);
			if (NULL != host && NULL != host->container)
			{
				ifc_omservice *service = NULL;
				if (SUCCEEDED(BrowserHost_GetOmService(host->container, &service)) && service != NULL)
				{
					if (FAILED(service->GetName(szBuffer, ARRAYSIZE(szBuffer))))
						szBuffer[0]= L'\0';
					service->Release();
				}
			}

			pInfo->SetName(szBuffer);
			pInfo->Release();
		}
		pWasabi->Release();
	}
	return hr;
}

static void BrowserHost_OnContainerInit(HWND winampWindow, HWND hwnd)
{
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL == host) return;

	if (NULL != host->container) return; // we're alredy running

	DWORD windowStyle = GetWindowStyle(hwnd);

	HTMLContainer2_Initialize();

	Browser *container = Browser::CreateInstance(host->browserManager, winampWindow, hwnd);
	host->container = container;
	if (NULL != container)
	{
		ifc_omservice *service = NULL;
		if (SUCCEEDED(BrowserHost_GetOmService(container, &service)) && service != NULL)
		{
			IDispatch *pDispatch = NULL;
			if (SUCCEEDED(service->GetExternal(&pDispatch)) && NULL != pDispatch)
			{
				BrowserHost_AssociateDispatch(hwnd, pDispatch);
				container->SetExternal(pDispatch);
				pDispatch->Release();
			}
			service->Release();
		}

		container->EventDocumentReady = BrowserHost_OnDocumentReady;
		container->EventNavigateComplete = BrowserHost_OnNavigateComplete;
		container->EventContainerDestroyed = BrowserHost_OnContainerDestroy;
		container->EventDownloadBegin = BrowserHost_OnDownloadBegin;
		container->EventDownloadComplete = BrowserHost_OnDownloadComplete;
		container->EventCommandStateChange = BrowserHost_OnCommandStateChange;
		container->EventStatusChange = BrowserHost_OnStatusChange;
		container->EventTitleChange = BrowserHost_OnTitleChange;
		container->EventSecureLockIconChange = BrowserHost_OnSecureLockIconChange;
		container->EventCreatePopup = BrowserHost_OnCreatePopup;
		container->EventFocusChange = BrowserHost_OnFocusChange;
		container->EventWindowClosing = BrowserHost_OnWindowClosing;

		if (0 != (NBHS_POPUP & windowStyle))
		{
			container->EventVisible = BrowserHost_OnVisibleChange;
			container->EventSetResizable = BrowserHost_OnSetResizable;
			container->EventSetFullscreen = BrowserHost_OnSetFullScreen;
			container->EventShowUiElement = BrowserHost_OnShowUiElenent;
			container->EventClientToHost = BrowserHost_OnClientToHost;
			container->EventSetWindowPos = BrowserHost_OnSetWindowPos;
			container->EventClosePopup = BrowserHost_OnClosePopup;
		}

		container->CallbackGetOmService = BrowserHost_GetOmService;
		container->CallbackRedirectKey = BrowserHost_RedirectKey;

		UINT uiFlags = 0;
		if (0 != (NBHS_DISABLECONTEXTMENU & windowStyle)) uiFlags |= Browser::flagUiDisableContextMenu;
		if (0 != (NBHS_DIALOGMODE & windowStyle)) uiFlags |= Browser::flagUiDialogMode;
		if (0 != (NBHS_DISABLEHOSTCSS & windowStyle)) uiFlags |= Browser::flagUiDisableHostCss;

		if (0 != uiFlags)
			container->SetUiFlags(uiFlags, uiFlags);

		HRESULT hr = container->Initialize(0 != (NBHS_POPUP & windowStyle));
		if (SUCCEEDED(hr) && 0 == (NBHS_SCRIPTMODE & windowStyle))
		{
			// we need to do this to allow script error disabling
			 hr = container->NavigateToName(NULL, 0);
		}

		if (FAILED(hr) || 0 != (NBHS_SCRIPTMODE & windowStyle))
		{
			if (0 == (NBHS_BROWSERREADY & windowStyle))
			{
				SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | NBHS_BROWSERREADY);
			
				NMHDR hdr;
				BrowserHost_SendBrowserNotification(container, NBHN_READY, &hdr);
			}
		}
	}

	HWND hParent = GetParent(hwnd);
	if (NULL != hParent)
	{	
		DWORD threadMine = 0, threadParent = 0;
		threadMine = GetCurrentThreadId();
		threadParent = GetWindowThreadProcessId(hParent, NULL);
		if (0 != threadMine && threadMine != threadParent)
		{
			AttachThreadInput(threadMine, threadParent, TRUE);

			HKL hkl = GetKeyboardLayout(threadParent);
			if (NULL != hkl) 
				ActivateKeyboardLayout(hkl, 0);
		}
	}
}

static void BrowserHost_OnContainerRelease(HWND hwnd)
{
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL == host) return;

	Browser *container = host->container;
	host->container = NULL;
	if (NULL == container) return;

	container->EventDocumentReady = NULL;
	container->EventNavigateComplete = NULL;
	container->EventDownloadBegin = NULL;
	container->EventDownloadComplete = NULL;
	container->EventCommandStateChange = NULL;
	container->EventStatusChange = NULL;
	container->EventTitleChange = NULL;
    container->EventSecureLockIconChange = NULL;
	container->EventCreatePopup = NULL;
	container->EventVisible = NULL;
	container->EventSetResizable = NULL;
	container->EventSetFullscreen = NULL;
	container->EventWindowClosing = NULL;
	container->EventShowUiElement = NULL;
	container->EventClientToHost = NULL;
	container->EventSetWindowPos = NULL;
	container->EventFocusChange = NULL;
	container->CallbackGetOmService = NULL;
	container->CallbackRedirectKey = NULL;

	if (SUCCEEDED(container->SendCommand(Browser::commandStop)) &&
		SUCCEEDED(container->NavigateToName(NULL, navNoHistory | navNoWriteToCache)))
	{
		IWebBrowser2 *pWeb2 = NULL;
		if (SUCCEEDED(container->GetIWebBrowser2(&pWeb2)) && pWeb2 != NULL)
		{
			ReplyMessage(0);
			BrowserThread_WaitNavigateComplete(pWeb2, 30000);
			pWeb2->Release();
		}
	}
	else
	{
	}

	container->Finish();
	container->Release();

	HWND hParent = GetParent(hwnd);
	if (NULL != hParent)
	{
		DWORD threadMine = 0, threadParent = 0;
		threadMine = GetCurrentThreadId();
		threadParent = GetWindowThreadProcessId(hParent, NULL);
		if (0 != threadMine && threadMine != threadParent)
		{
			AttachThreadInput(threadMine, threadParent, FALSE);
		}
	}
}

static LRESULT BrowserHost_OnNiceDestroy(HWND hwnd, BOOL fImmediate)
{
	DWORD windowStyle = GetWindowStyle(hwnd);
	if (0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

	BROWSERHOST *host = GetHost(hwnd);
	if (NULL != host && NULL != host->container && FALSE == fImmediate) 
	{
		HWND hWinamp = NULL;
		if (FAILED(Plugin_GetWinampWnd(&hWinamp)))
			hWinamp = NULL;

		if (hWinamp != GetParent(hwnd))
		{
			SetParent(hwnd, hWinamp);
			SetWindowLongPtr(hwnd, GWLP_HWNDPARENT, (LONGX86)(LONG_PTR)hWinamp);
		}

		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);

		if (FALSE != BrowserThread_PostDestroy(hwnd))
			return TRUE;
	}

	BrowserThread_SetFlags(BHTF_BEGINDESTROY, BHTF_BEGINDESTROY, FALSE);
	BrowserHost_OnContainerRelease(hwnd);
	return (0 != DestroyWindow(hwnd));
}

static void BrowserHost_OnContainerCommand(HWND hwnd, INT commandId)
{
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL == host) return;

	if (NULL == host->container) return;
	host->container->SendCommand(commandId);
}

static void BrowserHost_OnUpdateSkin(HWND hwnd)
{
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL == host) return;

	if (NULL != host->container)
	{
		//HWND hWinamp;
		//if (NULL == host->browserManager || FAILED(host->browserManager->GetWinampWindow(&hWinamp)))
		//	hWinamp = NULL;

		//HCURSOR cursor = (HCURSOR)SENDWAIPC(hWinamp, IPC_GETSKINCURSORS, WACURSOR_NORMAL);
		//if (NULL != cursor)
		//{
		//	host->container->RegisterBrowserCursor(/*OCR_NORMAL*/32512, CopyCursor(cursor));
		//}
	}
	
	UINT windowStyle = GetWindowStyle(hwnd);
	if (NBHS_BROWSERREADY == ((NBHS_BROWSERREADY | NBHS_BROWSERACTIVE) & windowStyle))
	{
		PostMessage(hwnd, NBHM_CONTAINERCOMMAND, (WPARAM)Browser::commandRefresh, 0L);
	}
}

static void BrowserHost_OnNavigate(HWND hwnd, BSTR url, UINT flags)
{
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL != host && 
		NULL != host->container &&
		FALSE == BrowserThread_IsQuiting()) 
	{
		host->container->NavigateToName(url, flags);
	}
	if (NULL != url)
	{
		SysFreeString(url);
	}
}

static void BrowserHost_OnEnableContainerUpdate(HWND hwnd, BOOL fEnable, BOOL fRedraw)
{
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL == host || NULL == host->container) return;

	HWND hContainer = host->container->GetHostHWND();
	if (NULL == hContainer) return;

	SendMessage(hContainer, WM_SETREDRAW, fEnable, 0L);
	if (fEnable && fRedraw)
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_NOERASE | RDW_NOFRAME | RDW_NOINTERNALPAINT | RDW_VALIDATE | RDW_ERASENOW );
}

static void BrowserHost_OnQueueAPC(HWND hwnd, PAPCFUNC pfnApc, ULONG_PTR param)
{
	if (NULL == pfnApc) return;
	pfnApc(param);
}

static void BrowserHost_OnShowHistoryPopup(HWND hwnd, UINT fuFlags, POINTS pts)
{
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL != host && NULL != host->container)
	{
		ifc_travelloghelper *travelLog;
		if (SUCCEEDED(host->container->GetTravelLog(&travelLog)))
		{
			travelLog->ShowPopup(fuFlags, pts.x, pts.y, hwnd, NULL);
			travelLog->Release();
		}		
	}
}

static void BrowserHost_OnGetDispatchApc(HWND hwnd, DISPATCHAPC callback, ULONG_PTR param)
{
	if (NULL == callback) return;
	
	IDispatch *pDisp = NULL;
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL != host && NULL != host->container)
	{
		IWebBrowser2 *pWeb2 = NULL;
		if (SUCCEEDED(host->container->GetIWebBrowser2(&pWeb2)) && pWeb2 != NULL)
		{
			pWeb2->get_Application(&pDisp);
			pWeb2->Release();
		}
	}

	callback(pDisp, param);

	if (NULL != pDisp)
		pDisp->Release();
}

static void BrowserHost_OnActivate(HWND hwnd)
{
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL != host && NULL !=  host->container)
	{
		host->container->SetFocus(TRUE);
	}
}

static void BrowserHost_OnQueryTitle(HWND hwnd)
{
	BSTR bstrName = NULL;
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL != host && NULL != host->container)
	{
		IWebBrowser2 *pWeb2 = NULL;
		if (SUCCEEDED(host->container->GetIWebBrowser2(&pWeb2)) && pWeb2 != NULL)
		{
			if (FAILED(pWeb2->get_LocationName(&bstrName)))
				bstrName = NULL;
			pWeb2->Release();
		}
	}
	
	BHNTEXTCHANGE textChange = {0};
	textChange.hdr.code = NBHN_TITLECHANGE;
	textChange.hdr.hwndFrom = hwnd;
	textChange.hdr.idFrom = GetDlgCtrlID(hwnd);
	textChange.pszText = bstrName;

	BrowserHost_SendNotification(hwnd, (NMHDR*)&textChange);

	SysFreeString(bstrName);
}

static void BrowserHost_OnToggleFullscreen(HWND hwnd)
{
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL == host || NULL == host->container)
		return;

	host->container->ToggleFullscreen();
}

static void BrowserHost_OnWriteDocument(HWND hwnd, BSTR documentData)
{
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL == host || 
		NULL == host->container || 
		FAILED(host->container->WriteDocument(documentData)))
	{
		SysFreeString(documentData);
	}
}

static void BrowserHost_OnEnableWindow(HWND hwnd, BOOL fEnable)
{
	EnableWindow(hwnd, fEnable);
}

static void BrowserHost_OnUpdateExternal(HWND hwnd)
{
	BROWSERHOST *host = GetHost(hwnd);
	if (NULL == host) return;

	if (NULL != host->container)
	{
		ifc_omservice *service = NULL;
		if (SUCCEEDED(BrowserHost_GetOmService(host->container, &service)) && service != NULL)
		{
			IDispatch *pDispatch = NULL;
			if (SUCCEEDED(service->GetExternal(&pDispatch)) && NULL != pDispatch)
			{
				BrowserHost_AssociateDispatch(hwnd, pDispatch);
				host->container->SetExternal(pDispatch);
				pDispatch->Release();
			}
			service->Release();
		}
	}
}

static LRESULT CALLBACK BrowserHost_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:
			return BrowserHost_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:
			BrowserHost_OnDestroy(hwnd);
			break;
		case WM_ERASEBKGND:
			return 0;
		case WM_PAINT:
			BrowserHost_OnPaint(hwnd);
			return 0;
		case WM_PRINTCLIENT:
			BrowserHost_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam);
			return 0;
		case WM_WINDOWPOSCHANGED:
			BrowserHost_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam);
			return 0;
		case WM_COMMAND:
			BrowserHost_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			break;
		case BTM_INITCONTAINER:
			BrowserHost_OnContainerInit((HWND)wParam, hwnd);
			return 0;
		case BTM_RELEASECONTAINER:
			BrowserHost_OnContainerRelease(hwnd);
			return 0;
		case NBHM_DESTROY:
			return BrowserHost_OnNiceDestroy(hwnd, (BOOL)wParam);
		case NBHM_CONTAINERCOMMAND:
			BrowserHost_OnContainerCommand(hwnd, (INT)wParam);
			return 0;
		case NBHM_UPDATESKIN:
			BrowserHost_OnUpdateSkin(hwnd);
			return 0;
		case NBHM_NAVIGATE:
			BrowserHost_OnNavigate(hwnd, (BSTR)lParam, (UINT)wParam);
			return 0;	
		case NBHM_ENABLECONTAINERUPDATE:
			BrowserHost_OnEnableContainerUpdate(hwnd, (BOOL)wParam, (BOOL)lParam);
			return 0;	
		case NBHM_QUEUEAPC:
			BrowserHost_OnQueueAPC(hwnd, (PAPCFUNC)lParam, (ULONG_PTR)wParam);
			return 0;
		case NBHM_SHOWHISTORYPOPUP:
			BrowserHost_OnShowHistoryPopup(hwnd, (UINT)wParam, MAKEPOINTS(lParam));
			return 0;
		case NBHM_GETDISPATCHAPC:
			BrowserHost_OnGetDispatchApc(hwnd, (DISPATCHAPC)lParam, (ULONG_PTR)wParam);
			return 0;
		case NBHM_ACTIVATE:
			BrowserHost_OnActivate(hwnd);
			return 0;
		case NBHM_QUERYTITLE:
			BrowserHost_OnQueryTitle(hwnd); 
			return 0;
		case NBHM_TOGGLEFULLSCREEN:
			BrowserHost_OnToggleFullscreen(hwnd);
			return 0;
		case NBHM_WRITEDOCUMENT:
			BrowserHost_OnWriteDocument(hwnd, (BSTR)lParam);
			return 0;
		case NBHM_ENABLEWINDOW:
			BrowserHost_OnEnableWindow(hwnd, (BOOL)lParam);
			return 0;
		case NBHM_UPDATEEXTERNAL:
			BrowserHost_OnUpdateExternal(hwnd);
			return 0;
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static DWORD BrowserHost_GetBrowserFlags(Browser *browser)
{
	if (NULL == browser) return 0;
	HWND hHost = browser->GetParentHWND();
	if (NULL == hHost) return 0;

	DWORD windowStyle = GetWindowStyle(hHost);
	return (NBHS_BROWSERMASK & windowStyle);
}

static void CALLBACK BrowserHost_OnDocumentReady(Browser *browser, IDispatch *pDispatch, VARIANT *URL)
{
	HWND hHost = browser->GetParentHWND();
	if (NULL == hHost) return;

	DWORD windowStyle = GetWindowStyle(hHost);
	if (0 == (NBHS_BROWSERREADY & windowStyle))
	{
		SetWindowLongPtr(hHost, GWL_STYLE, windowStyle | NBHS_BROWSERREADY);

		NMHDR hdr = {0};
		BrowserHost_SendBrowserNotification(browser, NBHN_READY, &hdr);
	}
	else
	{
		BHNNAVCOMPLETE document = {0};
		document.pDispatch = pDispatch;
		document.URL = URL;
		document.fTopFrame = TRUE;
		BrowserHost_SendBrowserNotification(browser, NBHN_DOCUMENTREADY, (NMHDR*)&document);
	}
}

static void CALLBACK BrowserHost_OnNavigateComplete(Browser *browser, IDispatch *pDispatch, VARIANT *URL)
{
	HWND hHost = browser->GetParentHWND();
	if (NULL == hHost) return;
	
	DWORD windowStyle = GetWindowStyle(hHost);
	if (0 == (NBHS_BROWSERREADY & windowStyle))
		return;

	IWebBrowser2 *pWeb1 = NULL, *pWeb2 = NULL;

	if (FAILED(browser->GetIWebBrowser2(&pWeb1)))
		pWeb1 = NULL;

	if (NULL == pDispatch || FAILED(pDispatch->QueryInterface(IID_IWebBrowser2, (void**)&pWeb2)))
		pWeb2 = NULL;

	if (NULL != pWeb1) pWeb1->Release();
	if (NULL != pWeb2) pWeb2->Release();

	BHNNAVCOMPLETE document = {0};
	document.pDispatch = pDispatch;
	document.URL = URL;
	document.fTopFrame = (NULL != pWeb1 && pWeb1 == pWeb2);
	BrowserHost_SendBrowserNotification(browser, NBHN_NAVIGATECOMPLETE, (NMHDR*)&document);
}

static void CALLBACK BrowserHost_OnContainerDestroy(Browser *browser)
{
	BrowserThread_SetFlags(BHTF_BEGINDESTROY, BHTF_BEGINDESTROY, FALSE);
	PostMessage(NULL, WM_QUIT, 0, 0L);
}

static void BrowserHost_NotifyContainerActive(HWND hwnd, BOOL fActive, UINT_PTR timerId)
{
	if (0 != timerId) 
		KillTimer(hwnd, timerId);

	DWORD windowStyle = GetWindowStyle(hwnd);
	if ((FALSE == fActive) != (0 == (NBHS_BROWSERACTIVE & windowStyle)))
		return;

	if (0 == (NBHS_BROWSERREADY & windowStyle))
		fActive = FALSE;

	BHNACTIVE active = {0};
	active.hdr.code = NBHN_BROWSERACTIVE;
	active.hdr.hwndFrom = hwnd;
	active.hdr.idFrom = GetDlgCtrlID(hwnd);
	active.fActive = fActive;
	BrowserHost_SendNotification(hwnd, (NMHDR*)&active);

	BROWSERHOST *host = GetHost(hwnd);
	INT commandState = 0;
	if (NULL != host && 
		NULL != host->container &&
		SUCCEEDED(host->container->QueryCommandState(Browser::commandStop, &commandState)))
	{
		BOOL fEnable = (0 != (Browser::commandStateEnabled & commandState));
		BrowserHost_OnCommandStateChange(host->container, Browser::commandStop, fEnable);
	}
}

static void CALLBACK BrowserHost_ActivateContainerTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD time)
{
	BrowserHost_NotifyContainerActive(hwnd, TRUE, eventId);
}

static void CALLBACK BrowserHost_DeactivateContainerTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD time)
{
	BrowserHost_NotifyContainerActive(hwnd, FALSE, eventId);
}

static void CALLBACK BrowserHost_OnDownloadBegin(Browser *browser)
{
	HWND hHost = browser->GetParentHWND();
	if (NULL == hHost) return;

	DWORD windowStyle = GetWindowStyle(hHost);
	if (NBHS_BROWSERREADY == ((NBHS_BROWSERACTIVE | NBHS_BROWSERREADY) & windowStyle))
	{
		SetWindowLongPtr(hHost, GWL_STYLE, windowStyle | NBHS_BROWSERACTIVE);
		SetTimer(hHost, BHT_CONTAINERDOWNLOAD, BHT_ACTIVATECONTAINER_DELAY, BrowserHost_ActivateContainerTimer);
	}
}

static void CALLBACK BrowserHost_OnDownloadComplete(Browser *browser)
{
	HWND hHost = browser->GetParentHWND();
	if (NULL == hHost) return;

	DWORD windowStyle = GetWindowStyle(hHost);
	if (0 != (NBHS_BROWSERACTIVE & windowStyle))
	{
		SetWindowLongPtr(hHost, GWL_STYLE, windowStyle & ~NBHS_BROWSERACTIVE);
		SetTimer(hHost, BHT_CONTAINERDOWNLOAD, BHT_DEACTIVATECONTAINER_DELAY, BrowserHost_DeactivateContainerTimer);
	}
}

static void CALLBACK BrowserHost_OnCommandStateChange(Browser *browser, INT commandId, BOOL fEnabled)
{
	if (Browser::commandStop == commandId && FALSE != fEnabled &&
		NBHS_BROWSERREADY == ((NBHS_BROWSERACTIVE | NBHS_BROWSERREADY) & BrowserHost_GetBrowserFlags(browser)))
	{				
		fEnabled = FALSE;
	}

	BHNCMDSTATE commandState = {0};
	commandState.commandId = commandId;
	commandState.fEnabled = fEnabled;
	BrowserHost_SendBrowserNotification(browser, NBHN_COMMANDSTATECHANGE, (NMHDR*)&commandState);
}

static void CALLBACK BrowserHost_OnStatusChange(Browser *browser, LPCWSTR pszText)
{	
	if (0 == (NBHS_BROWSERREADY & BrowserHost_GetBrowserFlags(browser)))
		return;

	BHNTEXTCHANGE textChange = {0};
	textChange.pszText = pszText;
	BrowserHost_SendBrowserNotification(browser, NBHN_STATUSCHANGE, (NMHDR*)&textChange);
}

static void CALLBACK BrowserHost_OnTitleChange(Browser *browser, LPCWSTR pszText)
{
	if (0 == (NBHS_BROWSERREADY & BrowserHost_GetBrowserFlags(browser)))
		return;

	BHNTEXTCHANGE textChange = {0};
	textChange.pszText = pszText;
	BrowserHost_SendBrowserNotification(browser, NBHN_TITLECHANGE, (NMHDR*)&textChange);
}

static void CALLBACK BrowserHost_OnSecureLockIconChange(Browser *browser)
{
	BHNSECUREICON icon = {0};
	icon.iconId = browser->GetSecueLockIcon();
	BrowserHost_SendBrowserNotification(browser, NBHN_SECUREICONCHANGE, (NMHDR*)&icon);
}

typedef struct __POPUPCALLBACKPARAM
{
	IDispatch **ppDisp;
	HANDLE readyEvent;
	HWND hHost;
} POPUPCALLBACKPARAM;

typedef struct __DISPATCHMARSHALPARAM
{
	LPSTREAM pStream;
	IDispatch *pDisp;
	HRESULT hr;
} DISPATCHMARSHALPARAM;

static void CALLBACK BrowserHost_DispatchMarshalApc(ULONG_PTR user)
{
	DISPATCHMARSHALPARAM *param = (DISPATCHMARSHALPARAM*)user;
	if (NULL == param) return;

	param->hr = CoGetInterfaceAndReleaseStream(param->pStream, IID_IDispatch, (void**)&param->pDisp);
	//if (SUCCEEDED(param->hr) && NULL != param->pDisp)
	//	param->pDisp->AddRef();
}

static void CALLBACK BrowserHost_DispatchCallbackApc(IDispatch *pDisp, ULONG_PTR user)
{
	POPUPCALLBACKPARAM *param = (POPUPCALLBACKPARAM*)user;
	if (NULL == param) return;

	if (NULL != pDisp)
	{
		LPSTREAM pStream = NULL;
		if (SUCCEEDED(CoMarshalInterThreadInterfaceInStream(IID_IDispatch, pDisp, &pStream)))
		{ // switch
			DISPATCHMARSHALPARAM marshal = {0};
			marshal.pStream = pStream;
			SendMessage(param->hHost, NBHM_QUEUEAPC, (WPARAM)&marshal, (LPARAM)BrowserHost_DispatchMarshalApc);
			if (SUCCEEDED(marshal.hr) && NULL != marshal.pDisp)
			{
				*param->ppDisp = marshal.pDisp;
			}
		}
	}

	if (NULL != param->readyEvent)
		SetEvent(param->readyEvent);
}

static HRESULT BrowserHost_GetSecurityApi(JSAPI2::api_security **securityApi)
{
	if (NULL == securityApi)
		return E_POINTER;

	*securityApi = NULL;

	ifc_wasabihelper *wasabiHelper = NULL;
	HRESULT hr = Plugin_GetWasabiHelper(&wasabiHelper);
	if (SUCCEEDED(hr) && wasabiHelper != NULL)
	{
		hr = wasabiHelper->GetSecurityApi(securityApi);
		wasabiHelper->Release();
	}
	return hr;
}

static BOOL BrowserHost_IsPopupAllowed(HWND hwnd, Browser *browser)
{
	ifc_omservice *service = NULL;
	if (FAILED(BrowserHost_GetOmService(browser, &service)) || NULL == service)
		return TRUE;

	BOOL fAllowed = TRUE;

	JSAPI::ifc_info *pInfo = NULL;
	WCHAR szKey[64] = {0};

	if (FAILED(StringCchPrintfW(szKey, ARRAYSIZE(szKey), L"%u", service->GetId())))
		szKey[0] = L'\0';

	IDispatch *pDispatch = NULL;
	if (SUCCEEDED(browser->GetExternal(&pDispatch)) && NULL != pDispatch)
	{
		IWasabiDispatchable *pWasabi = NULL;
		if (SUCCEEDED(pDispatch->QueryInterface(IID_IWasabiDispatchable, (void**)&pWasabi)) && pWasabi != NULL)
		{
			if (FAILED(pWasabi->QueryDispatchable(JSAPI::IID_JSAPI_ifc_info, (Dispatchable**)&pInfo)))
				pInfo = NULL;
			pWasabi->Release();
		}
		pDispatch->Release();
	}

	if (NULL != pInfo && L'\0' != szKey)
	{
		JSAPI2::api_security *security = NULL;
		if (SUCCEEDED(BrowserHost_GetSecurityApi(&security)) && security != NULL)
		{
			if (security->GetActionAuthorization(L"application", L"launchurl", szKey, pInfo, 
												 JSAPI2::api_security::ACTION_PROMPT) != JSAPI2::api_security::ACTION_ALLOWED)
			{
				fAllowed = FALSE;
			}
			security->Release();
		}
	}

	if (NULL != pInfo) 
		pInfo->Release();

	service->Release();
	return fAllowed;
}

static void CALLBACK BrowserHost_OnCreatePopup(Browser *browser, IDispatch **ppDisp, VARIANT_BOOL *Cancel)
{
	HWND hHost = browser->GetParentHWND();

	if (FALSE == BrowserHost_IsPopupAllowed(hHost, browser))
	{
		if (NULL != Cancel) *Cancel = VARIANT_TRUE;
		return;
	}

	POPUPCALLBACKPARAM param = {0};
	param.hHost = browser->GetParentHWND();
	if (NULL == param.hHost) return;

	param.readyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	param.ppDisp = ppDisp;

	BHNCREATEPOPUP popup = {0};
	popup.callback = BrowserHost_DispatchCallbackApc;
	popup.param  = (ULONG_PTR)&param;

	if (FALSE != BrowserHost_SendBrowserNotification(browser, NBHN_CREATEPOPUP, (NMHDR*)&popup))
	{
		BrowserThread_ModalLoop(param.hHost, param.readyEvent, 30000);
	}

	if (NULL != param.readyEvent)
		CloseHandle(param.readyEvent);
}

static void CALLBACK BrowserHost_OnVisibleChange(Browser *browser, VARIANT_BOOL fVisible)
{
	BHNVISIBLE visible = {0};
	visible.fVisible = (VARIANT_FALSE != fVisible);
	BrowserHost_SendBrowserNotification(browser, NBHN_VISIBLECHANGE, (NMHDR*)&visible);
}

static void CALLBACK BrowserHost_OnSetResizable(Browser *browser, VARIANT_BOOL fAllow)
{
	BHNRESIZABLE resizable = {0};
	resizable.fEnabled = (VARIANT_FALSE != fAllow);
	BrowserHost_SendBrowserNotification(browser, NBHN_RESIZABLE, (NMHDR*)&resizable);
}

static void CALLBACK BrowserHost_OnSetFullScreen(Browser *browser, VARIANT_BOOL fEnable)
{
	BHNFULLSCREEN fullscreen = {0};
	fullscreen.fEnable = (VARIANT_FALSE != fEnable);
	BrowserHost_SendBrowserNotification(browser, NBHN_FULLSCREEN, (NMHDR*)&fullscreen);
}

static void CALLBACK BrowserHost_OnWindowClosing(Browser *browser, VARIANT_BOOL fIsChild, VARIANT_BOOL *fCancel)
{
	BHNCLOSING closing = {0};
	closing.isChild = (VARIANT_FALSE != fIsChild);
	closing.cancel = FALSE;

	BrowserHost_SendBrowserNotification(browser, NBHN_CLOSING, (NMHDR*)&closing);

	if (FALSE != closing.cancel && NULL != fCancel)
		*fCancel = VARIANT_TRUE;
}

static void CALLBACK BrowserHost_OnShowUiElenent(Browser *browser, UINT elementId, VARIANT_BOOL fShow)
{
	BHNSHOWUI ui = {0};
	ui.elementId = elementId;
	ui.fShow = fShow;

	BrowserHost_SendBrowserNotification(browser, NBHN_SHOWUI, (NMHDR*)&ui);
}

static void CALLBACK BrowserHost_OnClientToHost(Browser *browser, LONG *cx, LONG *cy)
{
	BHNCLIENTTOHOST convert = {0};
	convert.cx = *cx;
	convert.cy = *cy;
	BrowserHost_SendBrowserNotification(browser, NBHN_CLIENTTOHOST, (NMHDR*)&convert);

	*cx = convert.cx;
	*cy = convert.cy;
}

static void CALLBACK BrowserHost_OnSetWindowPos(Browser *browser, UINT flags, LONG x, LONG y, LONG cx, LONG cy)
{
	BHNSETWINDOWPOS windowPos = {0};
	windowPos.flags = flags;
	windowPos.x = x;
	windowPos.y = y;
	windowPos.cx = cx;
	windowPos.cy = cy;

	BrowserHost_SendBrowserNotification(browser, NBHN_SETWINDOWPOS, (NMHDR*)&windowPos);
}

static void CALLBACK BrowserHost_OnFocusChange(Browser *browser, VARIANT_BOOL *fAllow)
{
	BHNFOCUSCHANGE focus = {0};
	focus.fAllow = (VARIANT_FALSE != *fAllow);

	BrowserHost_SendBrowserNotification(browser, NBHN_FOCUSCHANGE, (NMHDR*)&focus);

	*fAllow = (FALSE != focus.fAllow) ? VARIANT_TRUE : VARIANT_FALSE;
}

static void CALLBACK BrowserHost_OnClosePopup(Browser *browser)
{
	NMHDR hdr = {0};
	BrowserHost_SendBrowserNotification(browser, NBHN_CLOSEPOPUP, &hdr);
	
}
static HRESULT CALLBACK BrowserHost_GetOmService(Browser *browser, ifc_omservice **ppService)
{
	if (NULL == ppService) 	return E_POINTER;

	BHNSERVICE service = {0};

	if (FALSE != BrowserHost_SendBrowserNotification(browser, NBHN_GETOMSERVICE, (NMHDR*)&service) && 
		NULL != service.instance)
	{
		*ppService = (ifc_omservice*)service.instance;
		return S_OK;
	}

	*ppService = NULL;
	return E_NOTIMPL;
}


static LRESULT CALLBACK BrowserHost_RedirectKey(Browser *browser, MSG *pMsg)
{
	HWND hHost = browser->GetParentHWND();
	HWND hParent = GetParent(hHost);

	if (NULL == hParent) 
	{
		BROWSERHOST *host = GetHost(hHost);

		if (NULL != host && FAILED(Plugin_GetWinampWnd(&hParent)))
			hParent = NULL;
	}

	return PostMessage(hParent, pMsg->message, pMsg->wParam, pMsg->lParam);
}