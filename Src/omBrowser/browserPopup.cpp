#include "main.h"
#include "./browser.h"
#include "./browserUiInternal.h"
#include "./browserPopup.h"
#include "./browserHost.h"
#include "./browserThread.h"
#include "./graphics.h"
#include "./resource.h"
#include "./toolbar.h"
#include "./statusbar.h"
#include "./curtain.h"
#include "../winamp/skinWindowIPC.h"
#include "../Plugins/General/gen_ml/colors.h"

#include "./obj_ombrowser.h"
#include "./ifc_omservice.h"
#include "./ifc_omserviceeventmngr.h"
#include "./ifc_omserviceevent.h"
#include "./ifc_ombrowserwndmngr.h"
#include "./ifc_ombrowsereventmngr.h"

#include "./ifc_omconfig.h"
#include "./ifc_omtoolbarconfig.h"
#include "./ifc_omstatusbarconfig.h"

#include "./ifc_wasabihelper.h"
#include "./ifc_skinhelper.h"
#include "./ifc_skinnedbrowser.h"

#include "./browserUiHook.h"

#ifdef _DEBUG
#pragma warning( push )
#pragma warning( disable : 4244 )
#endif

#include <api/wnd/api_window.h>

#ifdef _DEBUG
#pragma warning( pop )
#endif


#include "../winamp/wa_dlg.h"

#include <exdispid.h>
#include <strsafe.h>

#define IDC_BROWSER		0x1000
#define IDC_TOOLBAR		0x1001
#define IDC_STATUSBAR	0x1002

#define BPT_ACTIVATEFRAME			27
#define BPT_ACTIVATEFRAME_DELAY		300

#define OSWNDHOST_REQUEST_IDEAL_SIZE (WM_USER + 2048)

typedef struct __BROWSERPOPUPCREATEPARAM
{
	obj_ombrowser	*browserManager;
	ifc_omservice	*service;
	WPARAM			callbackParam;
	DISPATCHAPC		callback;
	HWND			hOwner;
} BROWSERPOPUPCREATEPARAM;


typedef struct __POPUPRESTORE
{
	UINT	style;
	UINT	exStyle;
    RECT	rect;
	UINT	embedStyle;
	UINT	embedFlags;
	GUID	embedGuid;
	HWND	hOwner; // NULL - if DlgParent
} POPUPRESTORE;

#define BPF_LOCKRESIZE			0x00000001
#define BPF_FORCEDTOPMOST		0x00000002
#define BPF_MODECHANGELOCK		0x00000004

typedef struct __BROWSERPOPUP
{

	UINT		extendedStyle;
	UINT		flags;
	obj_ombrowser	*browserManager;
	ifc_omservice	*service;
	LPWSTR		storedUrl;
	BSTR		storedData;
	HWND			lastFocus;
	BrowserUiHook *browserHook;
	WPARAM		callbackParam;
	DISPATCHAPC	callback;
	COLORREF	rgbBack;
	POPUPRESTORE *restore;
	HWND		hOwner;
} BROWSERPOPUP;

#define FULLSCREEN_STYLE_FILTER		(WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_POPUP)
#define FULLSCREEN_EXSTYLE_FILTER	(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME | WS_EX_TOOLWINDOW)


#define GetPopup(__hwnd) ((BROWSERPOPUP*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))

static LRESULT CALLBACK BrowserPopup_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


static BOOL BrowserPopup_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;
	if (GetClassInfo(hInstance, NWC_OMBROWSERPOPUP, &wc)) return TRUE;

	ZeroMemory(&wc, sizeof(WNDCLASS));

	wc.hInstance		= hInstance;
	wc.lpszClassName	= NWC_OMBROWSERPOPUP;
	wc.lpfnWndProc	= BrowserPopup_WindowProc;
	wc.style			= CS_DBLCLKS;
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.cbWndExtra	= sizeof(BROWSERPOPUP*);
	
	if (0 == RegisterClassW(&wc))
		return FALSE;
	

	return TRUE;
}


HWND BrowserPopup_Create(obj_ombrowser *browserManager, ifc_omservice *service, UINT fStyle, INT x, INT y, INT cx, INT cy, HWND hOwner, DISPATCHAPC callback, ULONG_PTR callbackParam)
{
	if (FALSE == BrowserPopup_RegisterClass(Plugin_GetInstance()))
		return NULL;

	Toolbar_RegisterClass(Plugin_GetInstance());
	Statusbar_RegisterClass(Plugin_GetInstance());
	
	if (NULL == hOwner && FAILED(Plugin_GetWinampWnd(&hOwner)))
		hOwner = NULL;

	BROWSERPOPUPCREATEPARAM param;
	ZeroMemory(&param, sizeof(BROWSERPOPUPCREATEPARAM));

	param.browserManager = browserManager;
	param.service = service;
	param.callback = callback;
	param.callbackParam = callbackParam;
	param.hOwner = hOwner;

	HWND hPopup = CreateWindowEx(WS_EX_WINDOWEDGE, 
						NWC_OMBROWSERPOPUP, OMBROWSER_NAME, 
						WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | (0x0000FFFFF & fStyle), 
						x, y, cx, cy,
						hOwner, NULL, 
						Plugin_GetInstance(), &param);

	return hPopup;
}


static HWND BrowserPopup_GetFrame(HWND hwnd)
{
	HWND hFrame = hwnd;
	while (NULL != hFrame && 
		0 != (WS_CHILD & GetWindowLongPtr(hFrame, GWL_STYLE)))
	{
		hFrame = GetAncestor(hFrame, GA_PARENT);
	}
	return hFrame;
}

static void BrowserPopup_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return;

	SetBkColor(hdc, popup->rgbBack);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prcPaint, NULL, 0, NULL);
}

static void BrowserPopup_UpdateTitle(HWND hwnd, LPCWSTR pszTitle)
{
    if (NULL == hwnd) return;

	WCHAR szBuffer[256] = {0};
	LPWSTR cursor = szBuffer;
	size_t remaining = ARRAYSIZE(szBuffer);

	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL != popup)
	{
		if (NULL != popup->service)
		{
			WCHAR szName[128] = {0};
			if (SUCCEEDED(popup->service->GetName(szName, ARRAYSIZE(szName))) && L'\0' != szName[0])
				StringCchCopyEx(cursor, remaining, szName, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
		}

		HWND hFrame = (SENDWAIPC(hwnd, IPC_SKINWINDOW_GETEMBEDNUMS, 0) ? NULL : BrowserPopup_GetFrame(hwnd));
		if (NULL != pszTitle && hFrame != hwnd) // for now we can't update title in classic skin
		{
			if (cursor != szBuffer)
				StringCchCopyEx(cursor, remaining, L": ", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
			StringCchCopyEx(cursor, remaining, pszTitle, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
		}
		else
		{
		/*	if (NULL != popup->container)
			{
				IWebBrowser2 *pWeb2;
				if (SUCCEEDED(popup->container->GetIWebBrowser2(&pWeb2)))
				{
					BSTR bstrTitle;
					if (SUCCEEDED(pWeb2->get_LocationName(&bstrTitle)) && NULL != bstrTitle)
					{
						StringCchCopyEx(cursor, remaining, bstrTitle, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
						SysFreeString(bstrTitle);
					}
					pWeb2->Release();
				}
			}*/
		}
	}
	
	if (cursor == szBuffer)
		StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), OMBROWSER_NAME);
	
	SetWindowText(hwnd, szBuffer);
}

static HWND BrowserPopup_CreateToolbar(HWND hwnd)
{
	HWND hToolbar = GetDlgItem(hwnd, IDC_TOOLBAR);
	if (NULL != hToolbar) return hToolbar;

	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return NULL;
		
	UINT fStyle = GetWindowStyle(hwnd);

	UINT toolbarStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBS_LOCKUPDATE | TBS_SHOWADDRESS;
	if (0 == (NBCS_NOTOOLBAR & fStyle)) 
		toolbarStyle |= WS_VISIBLE;

	ifc_omtoolbarconfig *toolbarConfig;
	if (NULL != popup->browserManager && SUCCEEDED(popup->browserManager->GetConfig(&IFC_OmToolbarConfig, (void**)&toolbarConfig)))
	{
		if (S_OK == toolbarConfig->GetBottomDockEnabled())
			toolbarStyle |= TBS_BOTTOMDOCK;
		if (S_OK == toolbarConfig->GetAutoHideEnabled())
			toolbarStyle |= TBS_AUTOHIDE;
		if (S_OK == toolbarConfig->GetTabStopEnabled())
			toolbarStyle |= TBS_TABSTOP;
		if (S_OK == toolbarConfig->GetForceAddressbarEnabled())
			toolbarStyle |= TBS_FORCEADDRESS;
		if (S_OK == toolbarConfig->GetFancyAddressbarEnabled())
			toolbarStyle |= TBS_FANCYADDRESS;
		
		toolbarConfig->Release();
	}
	
	HINSTANCE hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

	hToolbar = CreateWindowEx(WS_EX_NOPARENTNOTIFY, 
		NWC_ONLINEMEDIATOOLBAR, NULL, toolbarStyle, 
		0, 0, 0, 0, hwnd, (HMENU)IDC_TOOLBAR, hInstance, NULL);

	if (NULL == hToolbar)
		return NULL;

	UINT populateStyle = TBPF_NORMAL | TBPF_READONLYADDRESS;
	if (0 != (NBCS_NOSERVICECOMMANDS & fStyle)) 
		populateStyle |= TBPF_NOSERVICECOMMANDS;

	Toolbar_AutoPopulate(hToolbar, popup->service, populateStyle);

	toolbarStyle = GetWindowStyle(hToolbar);
	if (0 != (TBS_LOCKUPDATE & toolbarStyle))
		Toolbar_LockUpdate(hToolbar, FALSE);
	
	return hToolbar;
}

static HWND BrowserPopup_CreateStatusbar(HWND hwnd)
{	
	HWND hStatusbar = GetDlgItem(hwnd, IDC_STATUSBAR);
	if (NULL != hStatusbar) return hStatusbar;

	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return NULL;

	UINT fStyle = GetWindowStyle(hwnd);

	UINT statusbarStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_DISABLED;
	if (0 == (NBCS_NOSTATUSBAR & fStyle))
	{
		ifc_omstatusbarconfig *statusbarConfig;
		if (NULL != popup->browserManager && SUCCEEDED(popup->browserManager->GetConfig(&IFC_OmStatusbarConfig, (void**)&statusbarConfig)))
		{
			if (S_OK == statusbarConfig->GetEnabled())
				statusbarStyle &= ~WS_DISABLED;
			statusbarConfig->Release();
		}
	}

	HINSTANCE hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
	
	hStatusbar = CreateWindowEx(WS_EX_NOPARENTNOTIFY, 
		NWC_ONLINEMEDIASTATUSBAR, NULL, statusbarStyle, 
		0, 0, 0, 0, hwnd, (HMENU)IDC_STATUSBAR, hInstance, NULL);

	return hStatusbar;
}

static BOOL BrowserPopup_PushRect(HWND hwnd, const RECT *rectIn, BOOL fOnlyIfExist)
{
	if (NULL == rectIn) return FALSE;

	RECT *rect = (RECT*)GetProp(hwnd, TEXT("omBrowserPopupRect"));
	if (FALSE != fOnlyIfExist && NULL == rect)
		return FALSE;

	if (NULL == rect)
	{
		rect = (RECT*)calloc(1, sizeof(RECT));
		if (NULL == rect || FALSE == SetProp(hwnd, TEXT("omBrowserPopupRect"), rect))
		{
			if (NULL != rect) free(rect);
			return FALSE;
		}
	}
	return CopyRect(rect, rectIn);
}

static BOOL BrwoserPopup_PushClientRect(HWND hwnd, BOOL fOnlyIfExists)
{
	RECT clientRect;
	if (!GetClientRect(hwnd, &clientRect))
		return FALSE;
	
	MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&clientRect, 2);
	return BrowserPopup_PushRect(hwnd, &clientRect, fOnlyIfExists);
}

static BOOL BrowserPopup_PopRect(HWND hwnd, RECT *rectOut, BOOL fNoRemove)
{
	RECT *rect = (RECT*)GetProp(hwnd, TEXT("omBrowserPopupRect"));
	if (FALSE == fNoRemove)
	{
		RemoveProp(hwnd, TEXT("omBrowserPopupRect"));
	}

	if (NULL == rect)
		return FALSE;
	
	if (NULL != rectOut)
		CopyRect(rectOut, rect);
	
	if (FALSE == fNoRemove)
		free(rect);
	
	return TRUE;
}

static BOOL BrowserPopup_ClientToFrame(HWND hwnd, RECT *rect)
{
	if (NULL == rect) return FALSE;
	RECT frameRect, clientRect;

	ifc_window *wasabiWnd = (ifc_window*)SENDWAIPC(hwnd, IPC_SKINWINDOW_GETWASABIWND, 0);
	if (NULL != wasabiWnd)
	{
		ifc_window *wasabiParent = wasabiWnd->getDesktopParent();
		if (NULL == wasabiParent) wasabiParent = wasabiWnd;
		
		if (!wasabiParent->getWindowRect(&frameRect))
			return FALSE;

		wasabiWnd->getClientRect(&clientRect);
		MapWindowPoints(wasabiWnd->gethWnd(), HWND_DESKTOP, (POINT*)&clientRect, 2);
	}
	else
	{
		HWND hFrame =  BrowserPopup_GetFrame(hwnd);
		if (!GetWindowRect(hFrame, &frameRect) || !GetClientRect(hwnd, &clientRect))
			return FALSE;
		MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&clientRect, 2);
	}
	
	rect->left += (frameRect.left - clientRect.left);
	rect->top += (frameRect.top - clientRect.top);
	rect->right += (frameRect.right - clientRect.right);
	rect->bottom += (frameRect.bottom - clientRect.bottom);

	return TRUE;
}

static INT CALLBACK BrowserPopup_FFCallback(embedWindowState *windowState, INT eventId, LPARAM param)
{
	switch(eventId)
	{
		case FFC_CREATEEMBED:
			if(NULL != windowState && NULL != param)
			{
				RECT rect;
				if (BrowserPopup_PopRect(windowState->me, &rect, FALSE))
				{
					BrowserPopup_ClientToFrame(windowState->me, &rect);
					BrowserPopup_SetFramePos(windowState->me, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 
								SWP_NOACTIVATE | SWP_NOZORDER);
				}
			}
			break;
		case FFC_DESTROYEMBED:
			if(NULL != windowState && NULL != param)
			{
				ifc_window *window = (ifc_window*)param;
				if (NULL != window)
				{
					RECT clientRect;
					window->getClientRect(&clientRect);
					MapWindowPoints(window->gethWnd(), HWND_DESKTOP, (POINT*)&clientRect, 2);
					BrowserPopup_PushRect(windowState->me, &clientRect, FALSE);
				}
			}
			break;
	}
	return 0;
}


static BOOL BrowserPopup_SkinWindow(HWND hwnd, const GUID *windowGuid)
{
	GUID windowId;

	if (NULL == windowGuid || IsEqualGUID(*windowGuid, GUID_NULL))
	{
		static ULONG counter = 0L;
		windowId = SkinClass_BrowserPopup;
		windowId.Data1 += counter;
		counter++;
	}
	else
		CopyMemory(&windowId, windowGuid, sizeof(GUID));

	BrwoserPopup_PushClientRect(hwnd, FALSE);

	ifc_skinhelper *skinHelper;
	HRESULT hr = Plugin_GetSkinHelper(&skinHelper);
	if (SUCCEEDED(hr))
	{
		hr = skinHelper->SkinWindow(hwnd, &windowId, SWF_NOWINDOWMENU, BrowserPopup_FFCallback);
		if (SUCCEEDED(hr))
		{
			SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		}

		skinHelper->Release();
	}

	return SUCCEEDED(hr);
}

static BOOL BrowserPopup_SwitchToFullscreen(HWND hwnd)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return FALSE;

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&clientRect, 2);

	HMONITOR hMonitor = MonitorFromRect(&clientRect, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi;
	ZeroMemory(&mi, sizeof(MONITORINFO));
	mi.cbSize = sizeof(MONITORINFO);
	if (!GetMonitorInfo(hMonitor, &mi)) return FALSE;

	POPUPRESTORE *restore = popup->restore;
	if (NULL == restore)
	{
		restore = (POPUPRESTORE*)calloc(1, sizeof(POPUPRESTORE));
		if (NULL == restore) return FALSE;
		popup->restore = restore;
	}

	popup->flags |= BPF_MODECHANGELOCK;

	CopyRect(&restore->rect, &clientRect);

	restore->embedStyle = (UINT)SENDWAIPC(hwnd, IPC_SKINWINDOW_GETEXSTYLE, 0);
	restore->embedFlags = (UINT)SENDWAIPC(hwnd, IPC_SKINWINDOW_GETEMBEDFLAGS, 0);
	if (FALSE == SENDWAIPC(hwnd, IPC_SKINWINDOW_GETEMBEDFLAGS, &restore->embedGuid))
		ZeroMemory(&restore->embedGuid, sizeof(GUID));

	HWND hFrame = BrowserPopup_GetFrame(hwnd);
	BOOL fTopmost = (NULL != hFrame && 0 != (WS_EX_TOPMOST & GetWindowStyleEx(hFrame)));

	SENDWAIPC(hwnd, IPC_SKINWINDOW_UNSKIN, 0);

	restore->hOwner = (HWND)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_HWNDPARENT);

	HWND hWinamp;
	if (FAILED(Plugin_GetWinampWnd(&hWinamp)))
		hWinamp = NULL;

	HWND hDlgParent = (HWND)SENDWAIPC(hWinamp, IPC_GETDIALOGBOXPARENT, 0);
	if (NULL != hDlgParent && restore->hOwner == hDlgParent)
		restore->hOwner = NULL;

	HWND hDesktop = GetDesktopWindow();
	SetWindowLongPtr(hwnd, GWLP_HWNDPARENT, (LONGX86)(LONG_PTR)hDesktop);

	DWORD style = GetWindowStyle(hwnd);
	restore->style = (FULLSCREEN_STYLE_FILTER & style);
	SetWindowLongPtr(hwnd, GWL_STYLE, style & ~FULLSCREEN_STYLE_FILTER);

	style = GetWindowStyleEx(hwnd);
	restore->exStyle = (FULLSCREEN_EXSTYLE_FILTER & style);
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, style & ~FULLSCREEN_EXSTYLE_FILTER);

	if (FALSE == fTopmost)
	{
		SetWindowPos(hwnd, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top, 
			mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOACTIVATE | SWP_FRAMECHANGED);

		SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW);
	}
	else
	{
		SetWindowPos(hwnd, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_FRAMECHANGED);
	}

	popup->flags &= ~BPF_MODECHANGELOCK;
	return TRUE;
}

static BOOL BrowserPopup_Restore(HWND hwnd)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup || NULL == popup->restore) return FALSE;

	POPUPRESTORE *restore = popup->restore;
	popup->restore = NULL;
	popup->flags |= BPF_MODECHANGELOCK;

	ShowWindow(hwnd, SW_HIDE);

	if (NULL == restore->hOwner)
	{
		HWND hWinamp;
		if (SUCCEEDED(Plugin_GetWinampWnd(&hWinamp)))
		{
			restore->hOwner = (HWND)SENDWAIPC(hWinamp, IPC_GETDIALOGBOXPARENT, 0);
		}
	}

	SetWindowLongPtr(hwnd, GWLP_HWNDPARENT, (LONGX86)(LONG_PTR)restore->hOwner);
	
	DWORD style;
	style = GetWindowStyleEx(hwnd);
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, style | restore->exStyle);

	style = GetWindowStyle(hwnd);
	SetWindowLongPtr(hwnd, GWL_STYLE, style | restore->style);

	BrowserPopup_SkinWindow(hwnd, &restore->embedGuid);
	SENDWAIPC(hwnd, IPC_SKINWINDOW_SETEXSTYLE, restore->embedStyle);
	SENDWAIPC(hwnd, IPC_SKINWINDOW_SETEMBEDFLAGS, restore->embedFlags);

	BrowserPopup_PushRect(hwnd, &restore->rect, TRUE);
	BrowserPopup_ClientToFrame(hwnd, &restore->rect);
	SetWindowPos(hwnd, NULL, restore->rect.left, restore->rect.top, 
			restore->rect.right - restore->rect.left, restore->rect.bottom - restore->rect.top, SWP_NOACTIVATE | SWP_NOZORDER);
	
	popup->flags &= ~BPF_MODECHANGELOCK;

	ShowWindow(hwnd, SW_SHOWNA);
	BrowserPopup_ActivateFrame(hwnd);
	
	free(restore);
	return TRUE;
}

static void BrowserPopup_CloseWindow(HWND hwnd)
{
	POINT pt;
	GetCursorPos(&pt);
	SendMessage(hwnd, WM_SYSCOMMAND, (WPARAM)SC_CLOSE, MAKELPARAM(pt.x, pt.y));
}

static BOOL BrowserPopup_SetStatusText(HWND hwnd, LPCWSTR pszText)
{
	HWND hStatusbar = GetDlgItem(hwnd, IDC_STATUSBAR);
	if (NULL == hStatusbar) return FALSE;
	
	Statusbar_Update(hStatusbar, pszText);
	return TRUE;
}

static BOOL BrowserPopup_SetStatusTextRes(HWND hwnd, LPCWSTR pszText)
{
	if (NULL != pszText && IS_INTRESOURCE(pszText))
	{
		WCHAR szBuffer[512] = {0};
		pszText = Plugin_LoadString((INT)(INT_PTR)pszText, szBuffer, ARRAYSIZE(szBuffer));
	}
	return BrowserPopup_SetStatusText(hwnd, pszText);
}

static BOOL BrowserPopup_ToggleFullscreen(HWND hwnd)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return FALSE;

	HWND hBrowser = GetDlgItem(hwnd, IDC_BROWSER);
	if (NULL == hBrowser) return FALSE;
			
	UINT embedFlags = 0;
	if (0 == (NBCS_EX_FULLSCREEN & popup->extendedStyle))
	{
		UINT windowStyle = GetWindowStyle(hwnd);
		if (0 != (NBCS_DISABLEFULLSCREEN & windowStyle))
			return FALSE;
		embedFlags = (UINT)SENDWAIPC(hwnd, IPC_SKINWINDOW_GETEMBEDFLAGS, 0);
	}
	else
	{
		BROWSERPOPUP *popup = GetPopup(hwnd);
		if (NULL != popup && NULL != popup->restore)
			embedFlags = popup->restore->embedFlags;
	}

	if (0 != (SWF_NORESIZE & embedFlags))
		return FALSE;
	
	return PostMessage(hBrowser, NBHM_TOGGLEFULLSCREEN, 0, 0L);
}

static void BrowserPopup_RegisterUiHook(HWND hwnd)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup || NULL != popup->browserHook) return;
	
	if (FAILED(BrowserUiHook::CreateInstance(hwnd, TRUE, &popup->browserHook)))
		return;
	
	popup->browserHook->Register(popup->browserManager, popup->service);
}

static LRESULT BrowserPopup_OnCreate(HWND hwnd, CREATESTRUCT *pcs)
{		
	BROWSERPOPUPCREATEPARAM *createParam = (BROWSERPOPUPCREATEPARAM*)pcs->lpCreateParams;
	BROWSERPOPUP *popup = (BROWSERPOPUP*)calloc(1, sizeof(BROWSERPOPUP));

	if (NULL != popup)
	{
		SetLastError(ERROR_SUCCESS);
		if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)popup) && ERROR_SUCCESS != GetLastError())
		{
			free(popup);
			popup = NULL;
		}
	}

	if (NULL == popup)
	{
		DestroyWindow(hwnd);
		return -1;
	}

	if (NULL != createParam)
	{
		popup->browserManager = createParam->browserManager;
		if (NULL != popup->browserManager)
			popup->browserManager->AddRef();
			
		popup->service = createParam->service;
		if (NULL != popup->service)
			popup->service->AddRef();
		
		popup->callback = createParam->callback;
		popup->callbackParam = createParam->callbackParam;
		popup->hOwner = createParam->hOwner;
	}

	BrowserPopup_RegisterUiHook(hwnd);

	SendMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0L);

	HWND hToolbar = BrowserPopup_CreateToolbar(hwnd);
	HWND hStatusbar = BrowserPopup_CreateStatusbar(hwnd);
	
	RECT clientRect;
    GetClientRect(hwnd, &clientRect);

	UINT hostStyle = NBHS_POPUP;
	if (0 != (NBCS_DISABLECONTEXTMENU & pcs->style)) hostStyle |= NBHS_DISABLECONTEXTMENU;
	if (0 != (NBCS_DIALOGMODE & pcs->style)) hostStyle |= NBHS_DIALOGMODE;
	if (0 != (NBCS_DISABLEHOSTCSS & pcs->style)) hostStyle |= NBHS_DISABLEHOSTCSS;
	
	HACCEL hAccel = BrowserControl_GetAccelTable(ACCELTABLE_POPUP);

	HWND hHost = BrowserHost_CreateWindow(popup->browserManager, hwnd, hostStyle, clientRect.left, clientRect.top, 
					clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, IDC_BROWSER, hAccel);
	
	if (NULL == hHost)
	{
		DestroyWindow(hwnd);
		return -1;
	}		

	if (NULL != hToolbar)
	{
		Toolbar_SetBrowserHost(hToolbar, hHost);
	}

	if (NULL != hStatusbar)
	{
		Statusbar_SetBrowserHost(hStatusbar, hHost);
		Statusbar_SetActive(hStatusbar, (0 == (WS_DISABLED & GetWindowLongPtr(hStatusbar, GWL_STYLE))));
	}

	ifc_wasabihelper *wasabiHelper;
	if (SUCCEEDED(Plugin_GetWasabiHelper(&wasabiHelper)))
	{
		api_application *app;
		if (SUCCEEDED(wasabiHelper->GetApplicationApi(&app)))
		{ 
			app->app_registerGlobalWindow(hwnd);
			
			if (NULL != hAccel)
				app->app_addAccelerators(hwnd, &hAccel, 1, TRANSLATE_MODE_CHILD);

			app->Release();
		}
		wasabiHelper->Release();
	}

	BrowserPopup_UpdateTitle(hwnd, NULL);
	BrowserPopup_SkinWindow(hwnd, NULL);

	if (NULL != popup->browserManager)
	{
		ifc_ombrowserwndmngr *windowManager;
		if (SUCCEEDED(popup->browserManager->QueryInterface(IFC_OmBrowserWindowManager, (void**)&windowManager)))
		{
			windowManager->RegisterWindow(hwnd, &WTID_BrowserPopup);
			windowManager->Release();
		}

		ifc_ombrowsereventmngr *eventManager;
		if (SUCCEEDED(popup->browserManager->QueryInterface(IFC_OmBrowserEventManager, (void**)&eventManager)))
		{
			eventManager->Signal_WindowCreate(hwnd, &WTID_BrowserPopup);
			eventManager->Release();
		}
	}
	return 0;
}

static void BrowserPopup_OnDestroy(HWND hwnd)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	SetWindowLongPtr(hwnd, 0, 0L);

	HWND hBrowser = GetDlgItem(hwnd, IDC_BROWSER);
	if (NULL != hBrowser)
	{
		DWORD browserStyle = GetWindowStyle(hBrowser);
		if (0 != (WS_VISIBLE & browserStyle))
			SetWindowLongPtr(hBrowser, GWL_STYLE, browserStyle & ~WS_VISIBLE);
		
		HWND hWinamp = NULL;
		if (Plugin_GetWinampWnd(&hWinamp))
			hWinamp = NULL;

		SetWindowLongPtr(hBrowser, GWLP_HWNDPARENT, (LONGX86)(LONG_PTR)hWinamp);

		if (NULL == popup || NULL == popup->browserManager ||
			S_OK == popup->browserManager->IsFinishing() ||
			0 == PostMessage(hBrowser, NBHM_DESTROY, 0, 0L))
		{
			DWORD_PTR result;
			SendMessageTimeout(hBrowser, NBHM_DESTROY, TRUE, 0L, SMTO_NOTIMEOUTIFNOTHUNG | SMTO_BLOCK, 500, &result);

		}
	}

	BrowserPopup_PopRect(hwnd, NULL, FALSE);

	if (NULL != popup)
	{
		popup->extendedStyle &= ~(NBCS_EX_BROWSERREADY | NBCS_EX_NAVCOMPLETED);
		popup->extendedStyle |= NBCS_EX_BLOCKNAVIGATION; 

		if (NULL != popup->browserHook)
		{
			popup->browserHook->Unregister(popup->browserManager, popup->service);
			popup->browserHook->Release();
		}

		if (NULL != popup->service)
		{
			popup->service->Release();
			popup->service = NULL;
		}

		if (NULL != popup->restore)
		{
			free(popup->restore);
		}

		Plugin_FreeResString(popup->storedUrl);
		popup->storedUrl = NULL;

		SysFreeString(popup->storedData);
		popup->storedData = NULL;

		if (NULL != popup->browserManager)
		{
			ifc_ombrowserwndmngr *windowManager;
			if (SUCCEEDED(popup->browserManager->QueryInterface(IFC_OmBrowserWindowManager, (void**)&windowManager)))
			{
				windowManager->UnregisterWindow(hwnd);
				windowManager->Release();
			}

			ifc_ombrowsereventmngr *eventManager = NULL;
			if (SUCCEEDED(popup->browserManager->QueryInterface(IFC_OmBrowserEventManager, (void**)&eventManager)))
			{
				eventManager->Signal_WindowClose(hwnd, &WTID_BrowserPopup);
				eventManager->Release();
			}

			popup->browserManager->Release();
			popup->browserManager = NULL;
		}

		free(popup);
	}

	ifc_wasabihelper *wasabiHelper = NULL;
	if (SUCCEEDED(Plugin_GetWasabiHelper(&wasabiHelper)))
	{
		api_application *app;
		if (SUCCEEDED(wasabiHelper->GetApplicationApi(&app)))
		{ 
			app->app_unregisterGlobalWindow(hwnd);
			app->app_removeAccelerators(hwnd);
			app->Release();
		}
		wasabiHelper->Release();
	}
}

static void BrowserPopup_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right)
			BrowserPopup_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void BrowserPopup_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	BrowserPopup_Paint(hwnd, hdc, &clientRect, 0 != (PRF_ERASEBKGND & options));
}

static void BrowserPopup_OnWindowPosChanging(HWND hwnd, WINDOWPOS *pwp)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL != popup && 0 != (BPF_LOCKRESIZE & popup->flags))
	{
		HWND hWinamp = NULL;
		if (FAILED(Plugin_GetWinampWnd(&hWinamp))) 
			hWinamp = NULL;

		if (NULL != hWinamp && hWinamp == (HWND)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_HWNDPARENT))
		{
			RECT rect;
			if (BrowserPopup_PopRect(hwnd, &rect, TRUE))
			{
				BrowserPopup_ClientToFrame(hwnd, &rect);
				pwp->x = rect.left;
				pwp->y = rect.top;
				pwp->cx = rect.right - rect.left;
				pwp->cy = rect.bottom - rect.top;
			}
		}
	}
}

static void BrowserPopup_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags))
		return;

	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL != popup)
	{
		BrowserControl_UpdateLayout(hwnd, 0 == (SWP_NOREDRAW & pwp->flags), 
			0 != (SWP_FRAMECHANGED & pwp->flags), NULL, NULL);
	}
}

static void BrowserPopup_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND hControl)
{
	switch(commandId)
	{		
		case IDC_TOOLBAR:	BrowserControl_ProcessToolbarCommand(hwnd, eventId); break;
		case IDC_STATUSBAR:	BrowserControl_ProcessStatusbarCommand(hwnd, eventId); break;
		default:						
			if (FALSE == BrowserControl_ProcessCommonCommand(hwnd, commandId))
			{
				switch(commandId)
				{
					case ID_WINDOW_CLOSE:			BrowserPopup_CloseWindow(hwnd); break;
					case ID_WINDOW_FULLSCREEN:		BrowserPopup_ToggleFullscreen(hwnd); break;
				}
			}
			break;
	}
}

static LRESULT BrowserPopup_OnAppCommand(HWND hwnd, HWND hTarget, INT commandId, INT deviceId, INT keys)
{
	return BrowserControl_ProcessAppCommand(hwnd, commandId);
}

static void BrowserPopup_OnBrowserReady(HWND hwnd)
{
	ReplyMessage(0);

	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return;

	popup->extendedStyle |= NBCS_EX_BROWSERREADY;
	
	BrowserControl_UpdateLayout(hwnd, FALSE, FALSE, NULL, NULL);
	
	HWND hToolbar = BrowserControl_GetToolbar(hwnd);
	if (NULL != hToolbar) 
		Toolbar_EnableItem(hToolbar, TOOLITEM_ADDRESSBAR, TRUE);

    if (NULL != popup->callback)
	{
		HWND hBrowser = GetDlgItem(hwnd, IDC_BROWSER);
		if (NULL == hBrowser || 
			FALSE == PostMessage(hBrowser, NBHM_GETDISPATCHAPC, (WPARAM)popup->callbackParam, (LPARAM)popup->callback))
		{
			// sucks
			popup->callback(NULL, popup->callbackParam);
		}

		popup->callback = NULL;
		return;
	}

	if (NULL != popup->storedUrl)
	{ 		
		LPWSTR pszUrl = popup->storedUrl;
		popup->storedUrl = NULL;
		BrowserPopup_Navigate(hwnd, pszUrl, TRUE);
		Plugin_FreeResString(pszUrl);
	}
	else
	{
		BrowserPopup_NavigateHome(hwnd, TRUE);
	}
}



static void CALLBACK BrowserPopup_ActivateTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD time)
{
	KillTimer(hwnd, eventId);
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return;

	HWND hFrame = BrowserPopup_GetFrame(hwnd);
	if (NULL != hFrame)
	{
		DWORD frameStyle = GetWindowStyleEx(hFrame);
		
		if (0 != (WS_EX_TOPMOST & frameStyle) && 0 != (BPF_FORCEDTOPMOST & popup->flags))
		{
			popup->flags &= ~BPF_FORCEDTOPMOST;
			SetWindowPos(hFrame, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE| SWP_NOMOVE | SWP_NOOWNERZORDER);
		}
		
		SetWindowPos(hFrame, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE| SWP_NOMOVE | SWP_NOOWNERZORDER);
	}
	
	PostMessage(hwnd, NBPM_ACTIVATEFRAME, 0, 0L);	

	HWND hBrowser = GetDlgItem(hwnd, IDC_BROWSER);
	if (NULL != hBrowser)
		PostMessage(hBrowser, NBHM_ACTIVATE, 0, 0L);
}

static void BrowserPopup_OnNavigateComplete(HWND hwnd, IDispatch *pDispath, VARIANT *URL, BOOL fTopFrame)
{	
	if (FALSE == fTopFrame) 
		return;

	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return;
	
	popup->extendedStyle |= NBCS_EX_NAVIGATEDONCE;
	
	LPWSTR pszUrl = NULL;
	if (NULL != URL && VT_BSTR == URL->vt && NULL != URL->bstrVal)
		pszUrl = Plugin_CopyString(URL->bstrVal);
	
	ReplyMessage(0);

	if (NULL != pszUrl)
	{
		HWND hToolbar = GetDlgItem(hwnd, IDC_TOOLBAR);
		if (NULL != hToolbar)
		{
			INT itemId = Toolbar_FindItem(hToolbar, TOOLITEM_ADDRESSBAR);
			if (ITEM_ERR != itemId)
				Toolbar_SetItemString(hToolbar, MAKEINTRESOURCE(itemId),  pszUrl); 
		}
		Plugin_FreeString(pszUrl);
	}
}

static void BrowserPopup_OnDocumentReady(HWND hwnd, IDispatch *pDispath, VARIANT *URL, BOOL fTopFrame)
{
	ReplyMessage(0);

	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return;
	
	if (0 == (NBCS_EX_NAVCOMPLETED & popup->extendedStyle))
	{
		popup->extendedStyle |= NBCS_EX_NAVCOMPLETED;
		
		BROWSERPOPUP *popup = GetPopup(hwnd);
		if (NULL != popup && NULL != popup->storedData)
		{
			BrowserPopup_WriteDocument(hwnd, popup->storedData, FALSE);
		}

		if(0 == (NBCS_EX_SCRIPTMODE & popup->extendedStyle))
		{
			HWND hHost = GetDlgItem(hwnd, IDC_BROWSER);
			if (NULL != hHost && 
				0 == (WS_VISIBLE & GetWindowLongPtr(hHost, GWL_STYLE)))
			{	
				ShowWindowAsync(hHost, SW_SHOWNA);
			}
		}

	}

	
}

static void BrowserPopup_OnBrowserActive(HWND hwnd, BOOL fActive)
{
	ReplyMessage(0);

	HWND hToolbar = GetDlgItem(hwnd, IDC_TOOLBAR);
	if (NULL != hToolbar)
	{
		Toolbar_EnableItem(hToolbar, TOOLITEM_DOWNLOADPROGRESS, fActive);
	}

	HWND hStatusbar = GetDlgItem(hwnd, IDC_STATUSBAR);
	if (NULL != hStatusbar)
	{
		if (FALSE != fActive && (0 != (WS_DISABLED & GetWindowLongPtr(hStatusbar, GWL_STYLE))))
			fActive = FALSE;
		Statusbar_SetActive(hStatusbar, fActive);
	}
}

static void BrowserPopup_OnCommandStateChange(HWND hwnd, UINT commandId, BOOL fEnable)
{
	ReplyMessage(0);

	HWND hToolbar = GetDlgItem(hwnd, IDC_TOOLBAR);
	if (NULL != hToolbar)
	{
		switch(commandId)
		{
			case Browser::commandBack:
				Toolbar_EnableItem(hToolbar, TOOLITEM_BUTTON_BACK, fEnable); 
				Toolbar_EnableItem(hToolbar, TOOLITEM_BUTTON_HISTORY, (FALSE != fEnable) ? 
									TRUE : 
									(0 == Toolbar_GetItemStyle(hToolbar, TOOLITEM_BUTTON_FORWARD, TBIS_DISABLED))); 
				break;

			case Browser::commandForward:
				Toolbar_EnableItem(hToolbar, TOOLITEM_BUTTON_FORWARD, fEnable); 
				Toolbar_EnableItem(hToolbar, TOOLITEM_BUTTON_HISTORY, (FALSE != fEnable) ? 
									TRUE : 
									(0 == Toolbar_GetItemStyle(hToolbar, TOOLITEM_BUTTON_BACK, TBIS_DISABLED))); 
				break;
			case Browser::commandStop:		Toolbar_EnableItem(hToolbar, TOOLITEM_BUTTON_STOP, fEnable); break;
			case Browser::commandRefresh:
				Toolbar_EnableItem(hToolbar, TOOLITEM_BUTTON_REFRESH, fEnable); 
				Toolbar_EnableItem(hToolbar, TOOLITEM_BUTTON_HOME, fEnable); 
				break;
		}
	}

}

static void BrowserPopup_OnStatusChange(HWND hwnd, LPCWSTR pszText)
{
	HWND hStatusbar = GetDlgItem(hwnd, IDC_STATUSBAR);
	if (NULL == hStatusbar) return;

	WCHAR szBuffer[512] = {0};
	if (NULL == pszText || L'\0' == *pszText) szBuffer[0] = L'\0';
	else StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), pszText);

	ReplyMessage(0);
	
	Statusbar_Update(hStatusbar, szBuffer);
}

static void BrowserPopup_OnTitleChange(HWND hwnd, LPCWSTR pszText)
{
	WCHAR szBuffer[256] = {0};
	if (NULL == pszText || L'\0' == *pszText) szBuffer[0] = L'\0';
	else StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), pszText);

	ReplyMessage(0);
	
	BrowserPopup_UpdateTitle(hwnd, szBuffer);

	HWND hToolbar = GetDlgItem(hwnd, IDC_TOOLBAR);
	if (NULL != hToolbar)
	{
		INT itemId = Toolbar_FindItem(hToolbar, TOOLITEM_ADDRESSBAR);
		if (ITEM_ERR != itemId)
			Toolbar_SetItemDescription(hToolbar, MAKEINTRESOURCE(itemId),  szBuffer); 
	}
}

static void BrowserPopup_OnSecureIconChange(HWND hwnd, UINT iconId)
{
	ReplyMessage(0);

	HWND hToolbar = GetDlgItem(hwnd, IDC_TOOLBAR);
	if (NULL != hToolbar)
	{
		INT itemId = Toolbar_FindItem(hToolbar, TOOLITEM_BUTTON_SECURECONNECTION);
		if (ITEM_ERR != itemId)
		{
			WCHAR szBuffer[256] = {0};

			if(FAILED(FormatEncryptionString(iconId, szBuffer, ARRAYSIZE(szBuffer))))
				szBuffer[0] = L'\0';

			Toolbar_ShowItem(hToolbar, MAKEINTRESOURCE(itemId), (secureLockIconUnsecure != iconId)); 
			Toolbar_SetItemDescription(hToolbar, MAKEINTRESOURCE(itemId), szBuffer);
		}
	}
}



static LRESULT BrowserPopup_OnCreatePopup(HWND hwnd, DISPATCHAPC callback, ULONG_PTR param)
{
	ReplyMessage(TRUE);

	HWND hPopup = NULL;
	
	UINT windowStyle = GetWindowStyle(hwnd);
	if (0 == (NBCS_BLOCKPOPUP & windowStyle))
	{
		BROWSERPOPUP *popup = GetPopup(hwnd);
		if (NULL != popup)
		{
			RECT windowRect;
			GetWindowRect(hwnd, &windowRect);
			
			UINT popupStyle = NBCS_NOSERVICECOMMANDS | NBCS_DISABLEHOSTCSS;
			popupStyle |= (NBCS_POPUPOWNER & GetWindowStyle(hwnd));
			
			HWND hOwner = NULL;
			if (0 != (NBCS_POPUPOWNER & popupStyle))
				hOwner = popup->hOwner;

			hPopup = BrowserPopup_Create(popup->browserManager, popup->service, popupStyle, 
						windowRect.left + 12, windowRect.top + 12, 640, 480, hOwner, callback, param);
		}
	}
	
	if (NULL == hPopup)
	{
		if (NULL != callback)
			callback(NULL, param);
		return FALSE;
	}

	BrowserControl_SetExtendedStyle(hPopup, NBCS_EX_SCRIPTMODE, NBCS_EX_SCRIPTMODE);
	BrowserPopup_UpdateSkin(hPopup, FALSE);
	return TRUE;
}
static void BrowserPopup_OnVisibleChange(HWND hwnd, BOOL fVisible)
{
	ReplyMessage(0);

	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return;

	HWND hFrame = BrowserPopup_GetFrame(hwnd);
	if (NULL == hFrame) hFrame = hwnd;

	if (FALSE != fVisible)
	{
		ShowWindow(hFrame, (0 == (WS_VISIBLE & GetWindowLongPtr(hFrame, GWL_STYLE))) ? SW_SHOWNOACTIVATE : SW_SHOW);
		
		HWND hWinamp;
		if (FAILED(Plugin_GetWinampWnd(&hWinamp))) 
			hWinamp = NULL;

		HWND hFrame = BrowserPopup_GetFrame(hwnd);
		HWND hDlgParent = (HWND)SENDWAIPC(hWinamp, IPC_GETDIALOGBOXPARENT, 0);
		if (NULL != hFrame && NULL != hDlgParent && hFrame != hDlgParent)
		{
			DWORD frameStyle = GetWindowStyleEx(hFrame);
			if (0 == (WS_EX_TOPMOST & frameStyle))
			{
				popup->flags |= BPF_FORCEDTOPMOST;
				SetWindowPos(hFrame, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE| SWP_NOMOVE | SWP_NOOWNERZORDER);
			}
			SetWindowPos(hFrame, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE| SWP_NOMOVE | SWP_NOOWNERZORDER);
		}

		HWND hBrowser = GetDlgItem(hwnd, IDC_BROWSER);
		if (NULL != hBrowser)
		{
			ShowWindowAsync(hBrowser, SW_SHOWNA);
		}

		SetTimer(hwnd, BPT_ACTIVATEFRAME, BPT_ACTIVATEFRAME_DELAY, BrowserPopup_ActivateTimer);
	}
	else
	{
		ShowWindow(hFrame, SW_HIDE);
	}
}

static void BrowserPopup_OnSetResizable(HWND hwnd, BOOL fEnabled)
{
	ReplyMessage(0);

	UINT flags = (UINT)SENDWAIPC(hwnd, IPC_SKINWINDOW_GETEMBEDFLAGS, 0);
	if ((0 != (SWF_NORESIZE & flags)) != (FALSE == fEnabled))
	{
		if (FALSE == fEnabled) flags |= SWF_NORESIZE;
		else flags &= ~EMBED_FLAGS_NORESIZE;
		SENDWAIPC(hwnd, IPC_SKINWINDOW_SETEMBEDFLAGS, (WPARAM)flags);

		HWND hFrame = BrowserPopup_GetFrame(hwnd);
		if (NULL != hFrame)
		{
			SetWindowPos(hFrame, NULL, 0, 0, 0, 0, 
				SWP_NOSIZE | SWP_NOMOVE| SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
	}
}

static void BrowserPopup_OnBrowserClosing(HWND hwnd, BOOL isChild, BOOL *fCancel)
{	
	// we telling ie to cancel in order to skip user prompt 
	*fCancel = TRUE;
	ReplyMessage(0);

	HWND hFrame = BrowserPopup_GetFrame(hwnd);
	if (NULL != hFrame)
	{
		ShowWindow(hFrame, SW_HIDE);
		DestroyWindow(hFrame);
	}
}

static void BrowserPopup_OnShowUiElement(HWND hwnd, UINT elementId, BOOL fShow)
{
	ReplyMessage(0);

	HWND hControl;
	UINT fStyle = GetWindowStyle(hwnd);
	switch(elementId)
	{
		case HTMLContainer2::uiToolbar:
			hControl = GetDlgItem(hwnd, IDC_TOOLBAR);
			if (NULL != hControl && 0 == (NBCS_NOTOOLBAR & fStyle))
			{
				ShowWindow(hControl, (FALSE != fShow) ? SW_SHOW : SW_HIDE);
				BrowserControl_UpdateLayout(hwnd, TRUE, FALSE, NULL, NULL);
			}
			break;

		case HTMLContainer2::uiStatusbar:
			hControl = GetDlgItem(hwnd, IDC_STATUSBAR);
			if (NULL != hControl && 0 == (NBCS_NOSTATUSBAR & fStyle))
			{
				EnableWindow(hControl, fShow);
				BrowserControl_UpdateLayout(hwnd, TRUE, FALSE, NULL, NULL);
			}
			break;
	}
}

static void BrowserPopup_OnClientToHost(HWND hwnd, LONG *cx, LONG *cy)
{
	RECT frameRect, browserRect;
	
	HWND hFrame =  BrowserPopup_GetFrame(hwnd);
	if (NULL == hFrame || !GetWindowRect(hFrame, &frameRect))
		return;
	
	HWND hBrowser = GetDlgItem(hwnd, IDC_BROWSER);
	if (NULL == hBrowser || !GetClientRect(hBrowser, &browserRect)) 
		return;
	
	*cx += ((frameRect.right - frameRect.left) - (browserRect.right - browserRect.left));
	*cy += ((frameRect.bottom - frameRect.top) - (browserRect.bottom - browserRect.top));
}

static void BrowserPopup_OnSetWindowPos(HWND hwnd, UINT flags, LONG x, LONG y, LONG cx, LONG cy)
{
	ReplyMessage(0);

	RECT frameRect;
	HWND hFrame = BrowserPopup_GetFrame(hwnd);
	if (NULL == hFrame || !GetWindowRect(hFrame, &frameRect)) return;

	UINT swpFlags = SWP_NOACTIVATE | SWP_NOZORDER;
	
	if (0 == ((HTMLContainer2::wndLeft | HTMLContainer2::wndTop) & flags))
	{
		swpFlags |= SWP_NOMOVE;
	}
	else
	{
		if (0 == (HTMLContainer2::wndLeft & flags)) x = frameRect.left;
		else if (0 != (HTMLContainer2::wndRelative & flags)) x += frameRect.left;

		if (0 == (HTMLContainer2::wndTop & flags)) y = frameRect.top;
		else if (0 != (HTMLContainer2::wndRelative & flags)) y += frameRect.top;
	}

	if (0 == ((HTMLContainer2::wndWidth | HTMLContainer2::wndHeight) & flags))
	{
		swpFlags |= SWP_NOSIZE;
	}
	else
	{
		if (0 == (HTMLContainer2::wndWidth & flags)) cx = (frameRect.right - frameRect.left);
		else if (0 != (HTMLContainer2::wndRelative & flags)) cx += (frameRect.right - frameRect.left);

		if (0 == (HTMLContainer2::wndHeight & flags)) cy = (frameRect.bottom - frameRect.top);
		else if (0 != (HTMLContainer2::wndRelative & flags)) cy += (frameRect.bottom - frameRect.top);
	}
	
	BrowserPopup_SetFramePos(hwnd, NULL, x, y, cx, cy, swpFlags);

}
static void BrowserPopup_OnAllowFocusChange(HWND hwnd, BOOL *fAllow)
{
	*fAllow = TRUE;
}

static void BrowserPopup_OnSetFullscreen(HWND hwnd, BOOL fEnable)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return;


	if (0 != (NBCS_EX_FULLSCREEN & popup->extendedStyle) == (FALSE != fEnable))
		return;

	if (FALSE == fEnable)
		popup->extendedStyle &= ~NBCS_EX_FULLSCREEN;
	else
	{
		UINT windowStyle = GetWindowStyle(hwnd);
		if (0 != (NBCS_DISABLEFULLSCREEN & windowStyle))
			return;

		popup->extendedStyle |= NBCS_EX_FULLSCREEN;
	}
	
	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

static void BrowserPopup_OnClosePopup(HWND hwnd)
{
	ReplyMessage(0);

	HWND hFrame = BrowserPopup_GetFrame(hwnd);
	if (NULL != hFrame)
	{
		ShowWindow(hFrame, SW_HIDE);
		DestroyWindow(hFrame);
	}
}

static LRESULT BrowserPopup_OnBrowserNotify(HWND hwnd, NMHDR *pnmh)
{
	switch(pnmh->code)
	{
		case NBHN_READY:
			BrowserPopup_OnBrowserReady(hwnd);
			break;
		case NBHN_NAVIGATECOMPLETE:
			BrowserPopup_OnNavigateComplete(hwnd, ((BHNNAVCOMPLETE*)pnmh)->pDispatch, ((BHNNAVCOMPLETE*)pnmh)->URL, ((BHNNAVCOMPLETE*)pnmh)->fTopFrame);
			break;
		case NBHN_DOCUMENTREADY:
			BrowserPopup_OnDocumentReady(hwnd, ((BHNNAVCOMPLETE*)pnmh)->pDispatch, ((BHNNAVCOMPLETE*)pnmh)->URL, ((BHNNAVCOMPLETE*)pnmh)->fTopFrame);
			break;
		case NBHN_BROWSERACTIVE:
			BrowserPopup_OnBrowserActive(hwnd, ((BHNACTIVE*)pnmh)->fActive);
			break;
		case NBHN_COMMANDSTATECHANGE:
			BrowserPopup_OnCommandStateChange(hwnd, ((BHNCMDSTATE*)pnmh)->commandId, ((BHNCMDSTATE*)pnmh)->fEnabled);
			break;
		case NBHN_STATUSCHANGE:
			BrowserPopup_OnStatusChange(hwnd, ((BHNTEXTCHANGE*)pnmh)->pszText);
			break;
		case NBHN_TITLECHANGE:
			BrowserPopup_OnTitleChange(hwnd, ((BHNTEXTCHANGE*)pnmh)->pszText);
			break;
		case NBHN_SECUREICONCHANGE:
			BrowserPopup_OnSecureIconChange(hwnd, ((BHNSECUREICON*)pnmh)->iconId);
			break;
		case NBHN_GETOMSERVICE:
			return BrowserPopup_GetService(hwnd, &((BHNSERVICE*)pnmh)->instance);
		case NBHN_CREATEPOPUP:
			return BrowserPopup_OnCreatePopup(hwnd, ((BHNCREATEPOPUP*)pnmh)->callback, ((BHNCREATEPOPUP*)pnmh)->param);
		case NBHN_VISIBLECHANGE:
			BrowserPopup_OnVisibleChange(hwnd, ((BHNVISIBLE*)pnmh)->fVisible);
			break;
		case NBHN_RESIZABLE:
			BrowserPopup_OnSetResizable(hwnd, ((BHNRESIZABLE*)pnmh)->fEnabled);
			break;
		case NBHN_CLOSING:
			BrowserPopup_OnBrowserClosing(hwnd, ((BHNCLOSING*)pnmh)->isChild, &((BHNCLOSING*)pnmh)->cancel);
			break;
		case NBHN_SHOWUI:
			BrowserPopup_OnShowUiElement(hwnd, ((BHNSHOWUI*)pnmh)->elementId, ((BHNSHOWUI*)pnmh)->fShow);
			break;
		case NBHN_CLIENTTOHOST:
			BrowserPopup_OnClientToHost(hwnd, &((BHNCLIENTTOHOST*)pnmh)->cx, &((BHNCLIENTTOHOST*)pnmh)->cy);
			break;
		case NBHN_SETWINDOWPOS:
			BrowserPopup_OnSetWindowPos(hwnd, ((BHNSETWINDOWPOS*)pnmh)->flags, ((BHNSETWINDOWPOS*)pnmh)->x,
					((BHNSETWINDOWPOS*)pnmh)->y, ((BHNSETWINDOWPOS*)pnmh)->cx, ((BHNSETWINDOWPOS*)pnmh)->cy);
			break;
		case NBHN_FOCUSCHANGE:
			BrowserPopup_OnAllowFocusChange(hwnd, &((BHNFOCUSCHANGE*)pnmh)->fAllow);
			break;
		case NBHN_FULLSCREEN:
			BrowserPopup_OnSetFullscreen(hwnd, ((BHNFULLSCREEN*)pnmh)->fEnable);
			break;
		case NBHN_CLOSEPOPUP:
			BrowserPopup_OnClosePopup(hwnd);
			break;

	}
	return 0;
}

static LRESULT BrowserPopup_OnNotify(HWND hwnd, INT controlId, NMHDR *pnmh)
{
	switch(controlId)
	{
	case IDC_BROWSER:
		return BrowserPopup_OnBrowserNotify(hwnd, pnmh);
	}

	return 0;
}

static void BrowserPopup_OnSetFocus(HWND hwnd, HWND hLost)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	HWND hChild, hTab;
	hChild = FindWindowEx(hwnd, NULL, NULL, NULL);
	hTab = NULL;
	
	if (NULL != popup)
	{
		while(popup->lastFocus && IsChild(hwnd, popup->lastFocus))
		{
			if (IsWindowEnabled(popup->lastFocus) && IsWindowVisible(popup->lastFocus) && 
				0 != (WS_TABSTOP & GetWindowLongPtrW(popup->lastFocus, GWL_STYLE)))
			{
				hTab = popup->lastFocus;
				break;
			}
			popup->lastFocus = GetParent(popup->lastFocus);
		}
	}

	if (NULL == hTab)
	{
		hTab = (hChild) ? GetNextDlgTabItem(hwnd, hChild, FALSE) : hwnd;
	}
	
	if (NULL != hTab && hwnd != hTab && 
		IsWindowEnabled(hTab) && 
		IsWindowVisible(hTab))
	{
		TCHAR szName[128] = {0};
		if (NULL != hChild && 
			GetClassName(hChild, szName, ARRAYSIZE(szName)) &&
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, szName, -1, TEXT("#32770"), -1))
		{
			if (IsWindowEnabled(hChild))
				PostMessage(hChild, WM_NEXTDLGCTL, (WPARAM)hTab, TRUE);
			else
				DefWindowProc(hwnd, WM_SETFOCUS, (WPARAM)hLost, 0L);
		}
		else 
		{
			SetFocus(hTab);
		}
		return;
	}
	
	DefWindowProc(hwnd, WM_SETFOCUS, (WPARAM)hLost, 0L);
}

static void BrowserPopup_OnActivate(HWND hwnd, UINT uActivate, HWND hwndOther, BOOL bMinimized)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return;

	POINT pt;
	
	switch(uActivate)
	{
		case WA_INACTIVE:
			popup->lastFocus = GetFocus();
			if (!IsChild(hwnd, popup->lastFocus))
				popup->lastFocus = NULL;
			break;
		
		case WA_CLICKACTIVE:
			if (GetCursorPos(&pt))
			{
				MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
				HWND hTarget = ChildWindowFromPointEx(hwnd, pt, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT);
				if (hTarget && hTarget != hwnd && hTarget != GetDlgItem(hwnd, IDC_TOOLBAR)) 
				{				
					
					popup->lastFocus = hTarget;
				}
			}
			break;
	}
}

static void BrowserPopup_OnStyleChanged(HWND hwnd, UINT nStyleType, STYLESTRUCT *pss)
{
	#define ISSTYLECHANGED(__style) ((__style) & pss->styleOld) != ((__style) & pss->styleNew)
	
	if (GWL_STYLE == nStyleType)
	{		
		if (ISSTYLECHANGED(WS_CHILD) && 0 != pss->styleNew && 0 != pss->styleOld)
		{	
			BROWSERPOPUP *popup = GetPopup(hwnd);
			if (NULL != popup && 0 == (BPF_MODECHANGELOCK & popup->flags))
			{
				popup->flags |= BPF_LOCKRESIZE;
				PostMessage(hwnd, NBPM_PARENTCHANGED, 0, 0L);
			}
		}
	}
}

static void BrowserPopup_OnGetMinMaxInfo(HWND hwnd, MINMAXINFO *minMax)
{
	minMax->ptMinTrackSize.x = 275;
	minMax->ptMinTrackSize.y = 116;
}

static LRESULT BrowserPopup_OnNavigate(HWND hwnd, LPCWSTR navigateUrl, BOOL fScheduleBlocked)
{	
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return FALSE;

	UINT windowStyle = GetWindowStyle(hwnd);

	if (NULL != popup->browserHook)
		popup->browserHook->CheckBlockedState(popup->service);

	SysFreeString(popup->storedData);
	popup->storedData = NULL;

	Plugin_FreeResString(popup->storedUrl);
	popup->storedUrl = NULL;
		
	if (NBCS_EX_BROWSERREADY != ((NBCS_EX_BROWSERREADY | NBCS_EX_BLOCKNAVIGATION) & popup->extendedStyle))
	{		
		if (FALSE == fScheduleBlocked) 
			return FALSE;
		
		popup->storedUrl = Plugin_DuplicateResString(navigateUrl);
		return TRUE;
	}	

	HWND hHost = GetDlgItem(hwnd, IDC_BROWSER);
	if (NULL == hHost) return FALSE;

	LPWSTR pszDescription = NULL;
	BSTR url = NULL;
	if (IS_INTRESOURCE(navigateUrl))
	{
		switch((INT_PTR)navigateUrl)
		{
			case NAVIGATE_BLANK:
				BrowserPopup_SetStatusText(hwnd, NULL);
				url = SysAllocString(L"about:blank");
				break;

			case NAVIGATE_HOME:
				if (0 == (NBCS_NOSERVICECOMMANDS & windowStyle) && NULL != popup->service)
				{
					WCHAR szBuffer[8192] = {0};
					if (SUCCEEDED(popup->service->GetUrl(szBuffer, ARRAYSIZE(szBuffer))))
					{
						BrowserPopup_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_HOME_DESCRIPTION));
						url = SysAllocString(szBuffer);
					}

					if (SUCCEEDED(popup->service->GetName(szBuffer, ARRAYSIZE(szBuffer))))
						pszDescription = Plugin_CopyString(szBuffer);
				}
				break;
			case NAVIGATE_BACK:
				BrowserPopup_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_BACK_DESCRIPTION));
				return PostMessage(hHost, NBHM_CONTAINERCOMMAND, (WPARAM)Browser::commandBack, 0L);
			case NAVIGATE_FORWARD:
				BrowserPopup_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_FORWARD_DESCRIPTION));
				return PostMessage(hHost, NBHM_CONTAINERCOMMAND, (WPARAM)Browser::commandForward, 0L);
			case NAVIGATE_STOP:
				BrowserPopup_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_STOP_DESCRIPTION));
				return PostMessage(hHost, NBHM_CONTAINERCOMMAND, (WPARAM)Browser::commandStop, 0L);
			case NAVIGATE_REFRESH:
				BrowserPopup_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_REFRESH_DESCRIPTION));
				return PostMessage(hHost, NBHM_CONTAINERCOMMAND, (WPARAM)Browser::commandRefresh, 0L);
			case NAVIGATE_REFRESH_COMPLETELY:
				BrowserPopup_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_REFRESH_DESCRIPTION));
				return PostMessage(hHost, NBHM_CONTAINERCOMMAND, (WPARAM)Browser::commandRefreshCompletely, 0L);
		}
	}
	else
	{
		BrowserPopup_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_NAVIGATING));
		url = SysAllocString(navigateUrl);
	}

	if (NULL == url) 
		return FALSE;

	HWND hToolbar = GetDlgItem(hwnd, IDC_TOOLBAR);
	INT addressbarId = (NULL != hToolbar) ? Toolbar_FindItem(hToolbar, TOOLITEM_ADDRESSBAR) : ITEM_ERR;
	if (ITEM_ERR != addressbarId)
	{
		Toolbar_SetItemString(hToolbar, MAKEINTRESOURCE(addressbarId), url);
		Toolbar_SetItemDescription(hToolbar, MAKEINTRESOURCE(addressbarId), pszDescription); 
	}

	if (!PostMessage(hHost, NBHM_NAVIGATE, 0, (LPARAM)url))
	{
		SysFreeString(url);
		return FALSE;
	}

	Plugin_FreeString(pszDescription);

	return TRUE;
}

static LRESULT BrowserPopup_OnWriteDocument(HWND hwnd, BSTR documentData, BOOL fScheduleBlocked)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return FALSE;

	SysFreeString(popup->storedData);
	popup->storedData = NULL;

	Plugin_FreeResString(popup->storedUrl);
	popup->storedUrl = NULL;
	
	if (0 == (NBCS_EX_NAVCOMPLETED & popup->extendedStyle))
	{		
		if (FALSE == fScheduleBlocked) 
			return FALSE;
		popup->storedData = documentData;
		return TRUE;
	}

	HWND hHost = GetDlgItem(hwnd, IDC_BROWSER);
	if (NULL == hHost) return FALSE;

	return PostMessage(hHost, NBHM_WRITEDOCUMENT, 0, (LPARAM)documentData);
}

static void BrowserPopup_OnParentChanged(HWND hwnd)
{
	HWND hRoot = GetAncestor(hwnd, GA_ROOT);
	DWORD oldStyleEx, newStyleEx;

	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return;
	
	popup->flags &= ~BPF_LOCKRESIZE;

	HWND hWinamp;
	if (FAILED(Plugin_GetWinampWnd(&hWinamp))) 
		hWinamp = NULL;

	oldStyleEx = GetWindowStyleEx(hwnd);
	newStyleEx = oldStyleEx;
	
	if (hRoot != hwnd)
	{
		HWND hDlgParent = (HWND)SENDWAIPC(hWinamp, IPC_GETDIALOGBOXPARENT, 0);
		if (hRoot != hWinamp && hRoot != hDlgParent)
		{			
			if (NULL != hDlgParent && hWinamp == (HWND)(LONG_PTR)GetWindowLongPtr(hRoot, GWLP_HWNDPARENT))
			{
				SetWindowLongPtr(hRoot, GWLP_HWNDPARENT, (LONGX86)(LONG_PTR)hDlgParent);
			}
		}
		newStyleEx |= WS_EX_CONTROLPARENT;
	}
	else
	{
		newStyleEx &= ~WS_EX_CONTROLPARENT;
		

		if (NULL != hWinamp && hWinamp != (HWND)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_HWNDPARENT))
			SetWindowLongPtr(hwnd, GWLP_HWNDPARENT, (LONGX86)(LONG_PTR)hWinamp);

		RECT rect;
		if (BrowserPopup_PopRect(hwnd, &rect, FALSE))
		{
			BrowserPopup_ClientToFrame(hwnd, &rect);
			BrowserPopup_SetFramePos(hwnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
		}
		
	}
	
	if (newStyleEx != oldStyleEx)
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, newStyleEx);

	BrowserPopup_RefreshTitle(hwnd);
	SendMessage(hRoot, WM_UPDATEUISTATE, MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0L);
}


static void BrowserPopup_OnUpdateSkin(HWND hwnd, BOOL fRedraw)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return;

	DWORD windowStyle = GetWindowStyle(hwnd);

	if (0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, ~WS_VISIBLE & windowStyle);

	ifc_skinnedbrowser *skinnedBrowser;
	if (SUCCEEDED(Plugin_GetBrowserSkin(&skinnedBrowser)))
	{
		popup->rgbBack = skinnedBrowser->GetBackColor();
		skinnedBrowser->Release();
	}
	else
		popup->rgbBack = GetSysColor(COLOR_WINDOW);

		
	HWND hControl;

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_BROWSER))) 
		PostMessage(hControl, NBHM_UPDATESKIN, 0, 0L);
	
	if (NULL != (hControl = GetDlgItem(hwnd, IDC_STATUSBAR))) 
		Statusbar_UpdateSkin(hControl, FALSE);
		
	if (NULL != (hControl = GetDlgItem(hwnd, IDC_TOOLBAR))) 
		Toolbar_UpdateSkin(hControl, FALSE);

	if (NULL != (hControl = BrowserControl_GetOperationWidget(hwnd)))
		Curtain_UpdateSkin(hControl, FALSE);
	
	BrowserControl_UpdateLayout(hwnd, FALSE, TRUE, NULL, NULL);

	if (0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);
	
	if (FALSE != fRedraw)
	{
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_FRAME);
	}
}

static LRESULT BrowserPopup_OnSkinRefreshing(HWND hwnd)
{
	
	BrwoserPopup_PushClientRect(hwnd, FALSE);

	HWND hRoot = GetAncestor(hwnd, GA_ROOT);
	if (hRoot != hwnd)
	{
		HWND hWinamp;
		if (FAILED(Plugin_GetWinampWnd(&hWinamp))) 
			hWinamp = NULL;

		HWND hDlgParent = (HWND)SENDWAIPC(hWinamp, IPC_GETDIALOGBOXPARENT, 0);
		if (NULL != hDlgParent && hDlgParent == (HWND)(LONG_PTR)GetWindowLongPtr(hRoot, GWLP_HWNDPARENT))
		{
            SetWindowLongPtr(hRoot, GWLP_HWNDPARENT, (LONGX86)(LONG_PTR)hWinamp);
		}
	}
	return 0;
}

static void BrowserPopup_OnSkinRefreshed(HWND hwnd)
{
	BrowserPopup_PopRect(hwnd, NULL, FALSE);
	
	HWND hFrame = BrowserPopup_GetFrame(hwnd);
	if (hwnd != hFrame)
	{
		UINT state = (IsWindowVisible(hwnd) && IsWindowEnabled(hwnd) && (hFrame == GetActiveWindow())) ? WA_ACTIVE : WA_INACTIVE;
		SendMessage(hwnd, WM_ACTIVATE, state, 0L);
	}
}

static LRESULT BrowserPopup_OnSetFramePos(HWND hwnd, WINDOWPOS *pwp)
{
	if (NULL == pwp) return FALSE;

	HWND hFrame;
		
	ifc_window *wasabiWnd = (ifc_window*)SENDWAIPC(hwnd, IPC_SKINWINDOW_GETWASABIWND, 0);
	if (NULL != wasabiWnd)
	{
		ifc_window *wasabiParent = wasabiWnd->getDesktopParent();
		if (NULL == wasabiParent) wasabiParent = wasabiWnd;
		
		hFrame = wasabiParent->gethWnd();
		if (NULL == hFrame) return FALSE;

		HWND hWinamp;
		if (FAILED(Plugin_GetWinampWnd(&hWinamp))) 
			hWinamp = NULL;

		HWND hDlgParent = (HWND)SENDWAIPC(hWinamp, IPC_GETDIALOGBOXPARENT, 0);
		if (hDlgParent == hFrame)
			return FALSE;  // do not change size/pos if we are part of the sui
		
		if (0 == (SWP_NOSIZE & pwp->flags))
		{
			RECT windowRect;
			wasabiParent->getWindowRect(&windowRect);
			if (pwp->cx != (windowRect.right - windowRect.left) || pwp->cy != (windowRect.bottom - windowRect.top))
			{
				if (0  == SendMessage(wasabiWnd->gethWnd(), OSWNDHOST_REQUEST_IDEAL_SIZE, pwp->cx, pwp->cy))
				{
					return FALSE; // hmm, 
				}
			}
			pwp->flags |= SWP_NOSIZE;
		}

		if (0 == (SWP_NOMOVE & pwp->flags))
		{
			RECT windowRect;
			wasabiParent->getWindowRect(&windowRect);
			wasabiParent->resize(pwp->x, pwp->y, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);
			pwp->flags |= SWP_NOMOVE;
		}
	}
	else
	{
		hFrame = BrowserPopup_GetFrame(hwnd);
		if (NULL == hFrame) return FALSE;
	}

	BOOL result = SetWindowPos(hFrame, pwp->hwndInsertAfter, pwp->x, pwp->y, pwp->cx, pwp->cy, pwp->flags);
	BrwoserPopup_PushClientRect(hwnd, TRUE);
	return result;
}



static void BrowserPopup_OnActivateFrame(HWND hwnd)
{
	ifc_window *wasabiWnd = (ifc_window*)SENDWAIPC(hwnd, IPC_SKINWINDOW_GETWASABIWND, 0);
	if (NULL != wasabiWnd)
	{
		wasabiWnd->activate();
	}
	else
	{
		HWND hFrame = BrowserPopup_GetFrame(hwnd);
		if (NULL == hFrame) hFrame = hwnd;

		BringWindowToTop(hFrame);
		SetActiveWindow(hFrame);
	}
}

static LRESULT BrowserPopup_OnGetService(HWND hwnd, ifc_omservice **serviceOut)
{
	if (NULL == serviceOut) return FALSE;
		
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup || NULL == popup->service)
	{
		*serviceOut = NULL;
		return FALSE;
	}

	*serviceOut = popup->service;
	(*serviceOut)->AddRef();
	return TRUE;
}

static LRESULT BrowserPopup_OnGetBrowserObject(HWND hwnd, obj_ombrowser **browserOut)
{
	if (NULL == browserOut) return FALSE;

	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup || NULL == popup->browserManager)
	{
		*browserOut = NULL;
		return FALSE;
	}

	*browserOut = popup->browserManager;
	(*browserOut)->AddRef();
	return TRUE;
}
static LRESULT BrowserPopup_OnRefreshTitle(HWND hwnd)
{
	HWND hBrowser = GetDlgItem(hwnd, IDC_BROWSER);
	if (NULL == hBrowser || FALSE == PostMessage(hBrowser, NBHM_QUERYTITLE, 0, 0L))
		BrowserPopup_UpdateTitle(hwnd, NULL);
	return TRUE;
}

static LRESULT BrowserPopup_OnGetToolbar(HWND hwnd)
{
	if (0 != (NBCS_NOTOOLBAR & GetWindowLongPtr(hwnd, GWL_STYLE)))
		return 0;

	HWND hToolbar = GetDlgItem(hwnd, IDC_TOOLBAR);
	return (LRESULT)hToolbar;
}

static LRESULT BrowserPopup_OnGetStatusbar(HWND hwnd)
{
	if (0 != (NBCS_NOSTATUSBAR & GetWindowLongPtr(hwnd, GWL_STYLE)))
		return 0;

	HWND hStatusbar = GetDlgItem(hwnd, IDC_STATUSBAR);
	return (LRESULT)hStatusbar;
}

static LRESULT BrowserPopup_OnGetHost(HWND hwnd)
{
	return (LRESULT)GetDlgItem(hwnd, IDC_BROWSER);
}

static LRESULT BrowserPopup_OnNavigateStoredUrl(HWND hwnd)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup || NULL == popup->storedUrl)
		return FALSE;

	LPWSTR url = popup->storedUrl;
	popup->storedUrl = NULL;
	BOOL result = BrowserPopup_Navigate(hwnd, url, TRUE);
	Plugin_FreeResString(url);
	return result;
}

static LRESULT BrowserPopup_OnSetExtendedStyle(HWND hwnd, UINT extMask, UINT extStyle)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	if (NULL == popup) return 0;

	UINT currentStyle = popup->extendedStyle;
	popup->extendedStyle = (currentStyle & ~extMask) | (extStyle & extMask);

	if ((NBCS_EX_SCRIPTMODE & currentStyle) != (NBCS_EX_SCRIPTMODE & popup->extendedStyle))
	{
		HWND hBrowser = GetDlgItem(hwnd, IDC_BROWSER);
		if (NULL != hBrowser)
		{
			UINT browserStyle = GetWindowStyle(hBrowser);
			
			if (0 != (NBCS_EX_SCRIPTMODE & popup->extendedStyle))
				browserStyle |= NBHS_SCRIPTMODE;
			else
				browserStyle &= ~ NBHS_SCRIPTMODE;

			SetWindowLongPtr(hBrowser, GWL_STYLE, browserStyle);
		}
	}
	
	if ((NBCS_EX_FULLSCREEN & currentStyle) != (NBCS_EX_FULLSCREEN & popup->extendedStyle))
	{
		if (0 != (NBCS_EX_FULLSCREEN & popup->extendedStyle))
			BrowserPopup_SwitchToFullscreen(hwnd);
		else
			BrowserPopup_Restore(hwnd);
	}

	return currentStyle;
}

static LRESULT BrowserPopup_OnGetExtendedStyle(HWND hwnd)
{
	BROWSERPOPUP *popup = GetPopup(hwnd);
	return (NULL != popup) ? popup->extendedStyle : 0;
}

static LRESULT CALLBACK BrowserPopup_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return BrowserPopup_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			BrowserPopup_OnDestroy(hwnd); break;
		case WM_ERASEBKGND:			return 0;
		case WM_PAINT:				BrowserPopup_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:		BrowserPopup_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_WINDOWPOSCHANGING:	BrowserPopup_OnWindowPosChanging(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_WINDOWPOSCHANGED:	BrowserPopup_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_COMMAND:			BrowserPopup_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case WM_APPCOMMAND:			return BrowserPopup_OnAppCommand(hwnd, (HWND)wParam, GET_APPCOMMAND_LPARAM(lParam), GET_DEVICE_LPARAM(lParam), GET_KEYSTATE_LPARAM(lParam));
		case WM_NOTIFY:				return BrowserPopup_OnNotify(hwnd, (INT)wParam, (NMHDR*)lParam);
		case WM_SETFOCUS:			BrowserPopup_OnSetFocus(hwnd, (HWND)wParam); return 0;
		case WM_ACTIVATE:			BrowserPopup_OnActivate(hwnd, LOWORD(wParam), (HWND)lParam, HIWORD(wParam)); break;
		case WM_STYLECHANGED:		BrowserPopup_OnStyleChanged(hwnd, (UINT)wParam, (STYLESTRUCT*)lParam); break;
		case WM_GETMINMAXINFO:		BrowserPopup_OnGetMinMaxInfo(hwnd, (MINMAXINFO*)lParam); return 0;

		case NBCM_GETTOOLBAR:		return BrowserPopup_OnGetToolbar(hwnd);
		case NBCM_GETSTATUSBAR:		return BrowserPopup_OnGetStatusbar(hwnd);
		case NBCM_GETHOST:			return BrowserPopup_OnGetHost(hwnd);
		case NBCM_GETSERVICE:		return BrowserPopup_OnGetService(hwnd, (ifc_omservice**)lParam); 
		case NBCM_UPDATESKIN:		BrowserPopup_OnUpdateSkin(hwnd, (BOOL)lParam); return 0;
		case NBCM_NAVIGATE:			return BrowserPopup_OnNavigate(hwnd, (LPCWSTR)lParam, (BOOL)wParam);
		case NBCM_WRITEDOCUMENT:	return BrowserPopup_OnWriteDocument(hwnd, (BSTR)lParam, (BOOL)wParam);
		case NBCM_GETBROWSEROBJECT:	return BrowserPopup_OnGetBrowserObject(hwnd, (obj_ombrowser**)lParam); 
		case NBCM_SHOWOPERATION:	return BrowserControl_OnShowOperation(hwnd, (OPERATIONINFO*)lParam);
		case NBCM_NAVSTOREDURL:		return BrowserPopup_OnNavigateStoredUrl(hwnd);
		case NBCM_BLOCK:			BrowserControl_SetBlockedState(hwnd, (BOOL)lParam); return 0;
		case NBCM_SETEXTSTYLE:		return BrowserPopup_OnSetExtendedStyle(hwnd, (UINT)wParam, (UINT)lParam);
		case NBCM_GETEXTSTYLE:		return BrowserPopup_OnGetExtendedStyle(hwnd);

		case NBPM_PARENTCHANGED:	BrowserPopup_OnParentChanged(hwnd); return 0;
		case NBPM_SKINREFRESHING:	return BrowserPopup_OnSkinRefreshing(hwnd);
		case NBPM_SKINREFRESHED:	BrowserPopup_OnSkinRefreshed(hwnd); return 0;
		case NBPM_SETFRAMEPOS:		return BrowserPopup_OnSetFramePos(hwnd, (WINDOWPOS*)lParam);
		case NBPM_ACTIVATEFRAME:	BrowserPopup_OnActivateFrame(hwnd); return 0;
		case NBPM_REFRESHTITLE:		return BrowserPopup_OnRefreshTitle(hwnd);
	}

	if (FALSE != Plugin_IsDirectMouseWheelMessage(uMsg))
	{
		SendMessage(hwnd, WM_MOUSEWHEEL, wParam, lParam);
		return TRUE;
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}