#include "main.h"
#include "./browserView.h"
#include "./browserUiInternal.h"
#include "./graphics.h"
#include "./resource.h"
#include "./browser.h"
#include "./browserHost.h"
#include "./browserPopup.h"
#include "./toolbar.h"
#include "./statusbar.h"
#include "./curtain.h"
#include "../winamp/wa_dlg.h"

#include "../Plugins/General/gen_ml/colors.h"

#include "./ifc_omservice.h"
#include "./obj_ombrowser.h"
#include "./ifc_wasabihelper.h"
#include "./ifc_skinhelper.h"
#include "./ifc_skinnedbrowser.h"
#include "./ifc_omserviceeventmngr.h"
#include "./ifc_omserviceevent.h"
#include "./ifc_ombrowserwndmngr.h"
#include "./ifc_ombrowsereventmngr.h"

#include "./ifc_omconfig.h"
#include "./ifc_omtoolbarconfig.h"
#include "./ifc_omstatusbarconfig.h"

#include "./browserUiHook.h"

#include <shlwapi.h>
#include <strsafe.h>


typedef struct __BROWSERVIEWCREATEPARAM
{
	obj_ombrowser	*browserManager;
	ifc_omservice	*service;
	LPCWSTR		redirectUrl;
} BROWSERVIEWCREATEPARAM;

typedef struct __BROWSERVIEW
{
	UINT		extendedStyle;
	obj_ombrowser	*browserManager;
	ifc_omservice	*service;
	LPWSTR		storedUrl;
	BSTR		storedData;
	HRGN		updateRegion;
	POINT		regionOffset;
	COLORREF	rgbBack;
	BrowserUiHook *browserHook;
} BROWSERVIEW;

#define IDC_BROWSER		0x1000
#define IDC_TOOLBAR		0x1001
#define IDC_STATUSBAR	0x1002

static BOOL BrowserView_RegisterClass(HINSTANCE hInstance);
static LRESULT CALLBACK BrowserView_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define GetView(__hwnd) ((BROWSERVIEW*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))

HWND BrowserView_Create(obj_ombrowser *browserManager, ifc_omservice *service, HWND hParent, LPCWSTR redirectUrl, UINT style)
{
	HINSTANCE hInstance = Plugin_GetInstance();
	if (!BrowserView_RegisterClass(hInstance) ||
		!Toolbar_RegisterClass(hInstance) ||
		!Statusbar_RegisterClass(hInstance))
	{
		return NULL;
	}

	BROWSERVIEWCREATEPARAM createParam;
	ZeroMemory(&createParam, sizeof(BROWSERVIEWCREATEPARAM));

	createParam.browserManager = browserManager;
	createParam.service = service;
	createParam.redirectUrl = redirectUrl;

	HWND hwnd = CreateWindowEx(WS_EX_NOPARENTNOTIFY | WS_EX_CONTROLPARENT, 
		NWC_OMBROWSERVIEW, NULL, 
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | style, 
		0, 0, 0, 0, hParent, NULL, hInstance, &createParam);

	return hwnd;
}

static void BrowserView_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{
	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view) return;

	SetBkColor(hdc, view->rgbBack);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prcPaint, NULL, 0, NULL);
}


static BOOL BrowserView_PreformNavigation(HWND hwnd)
{
	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view) return FALSE;

	if (NULL != view->storedUrl)
	{ 
		LPWSTR url = view->storedUrl;
		view->storedUrl = NULL;
		BrowserView_Navigate(hwnd, url, TRUE);
		Plugin_FreeResString(url);
	}
	else
	{
		BrowserView_NavigateHome(hwnd, TRUE);
	}
	return TRUE;
}

static HWND BrowserView_CreateToolbar(HWND hwnd)
{
	HWND hToolbar = GetDlgItem(hwnd, IDC_TOOLBAR);
	if (NULL != hToolbar) return hToolbar;

	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view) return NULL;

	UINT fStyle = GetWindowStyle(hwnd);

	UINT toolbarStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBS_LOCKUPDATE;
	if (0 == (NBCS_NOTOOLBAR & fStyle)) 
		toolbarStyle |= WS_VISIBLE;

	ifc_omtoolbarconfig *toolbarConfig;
	if (NULL != view->browserManager && SUCCEEDED(view->browserManager->GetConfig(&IFC_OmToolbarConfig, (void**)&toolbarConfig)))
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

	Toolbar_AutoPopulate(hToolbar, view->service, populateStyle);

	toolbarStyle = GetWindowStyle(hToolbar);
	if (0 != (TBS_LOCKUPDATE & toolbarStyle))
		Toolbar_LockUpdate(hToolbar, FALSE);

	return hToolbar;
}

static HWND BrowserView_CreateStatusbar(HWND hwnd)
{
	HWND hStatusbar = GetDlgItem(hwnd, IDC_STATUSBAR);
	if (NULL != hStatusbar) return hStatusbar;

	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view) return NULL;

	UINT fStyle = GetWindowStyle(hwnd);

	UINT statusbarStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_DISABLED;
	if (0 == (NBCS_NOSTATUSBAR & fStyle))
	{
		ifc_omstatusbarconfig *statusbarConfig;
		if (NULL != view->browserManager && SUCCEEDED(view->browserManager->GetConfig(&IFC_OmStatusbarConfig, (void**)&statusbarConfig)))
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

static LRESULT BrowserView_ShowHistory(HWND hwnd, BOOL fButtonPressd)
{
	HWND hBrowser = GetDlgItem(hwnd, IDC_BROWSER);
	if (NULL == hBrowser) return FALSE;

	UINT flags = TPM_LEFTALIGN;
	POINT pt = {0, 0};

	BOOL fUseHost = TRUE;;
	HWND hToolbar = GetDlgItem(hwnd, IDC_TOOLBAR);
	DWORD toolbarStyle = (NULL != hToolbar) ? GetWindowStyle(hToolbar) : 0;

	if (NULL != hToolbar && 0 != (WS_VISIBLE & toolbarStyle))
	{
		fUseHost = FALSE;
		RECT toolbarRect;
		GetWindowRect(hToolbar, &toolbarRect);
		pt.x = toolbarRect.left + 2;
		if (0 != (TBS_BOTTOMDOCK & toolbarStyle))
		{
			pt.y = toolbarRect.top + 1;
			flags |= TPM_VERNEGANIMATION | TPM_BOTTOMALIGN;
		}
		else
		{
			pt.y = toolbarRect.bottom - 1;
			flags |= TPM_VERPOSANIMATION | TPM_TOPALIGN;
		}
	}

	if (FALSE != fUseHost)
	{
		RECT windowRect;
		GetClientRect(hwnd, &windowRect);
		pt.x = windowRect.left;
		pt.y = windowRect.top;
		MapWindowPoints(hwnd, HWND_DESKTOP, &pt, 1);
		flags |= TPM_VERPOSANIMATION | TPM_TOPALIGN;
	}

	if (NULL != hToolbar && 0 != (TBS_AUTOHIDE & toolbarStyle))
		SetWindowLongPtr(hToolbar, GWL_STYLE, toolbarStyle & ~TBS_AUTOHIDE);

	HWND hStatusbar = GetDlgItem(hwnd, IDC_STATUSBAR);
	DWORD statusStyle = (NULL != hStatusbar) ? GetWindowStyle(hStatusbar) : 0;
	if (NULL != hStatusbar && 0 == (WS_DISABLED & statusStyle))
		SetWindowLongPtr(hStatusbar, GWL_STYLE, WS_DISABLED | statusStyle);

	LRESULT  result = SendMessage(hBrowser, NBHM_SHOWHISTORYPOPUP, (WPARAM)flags, MAKELPARAM(pt.x, pt.y));

	if (NULL != hToolbar && 0 != (TBS_AUTOHIDE & toolbarStyle))
	{
		SetWindowLongPtr(hToolbar, GWL_STYLE, toolbarStyle);
		TRACKMOUSEEVENT tm;
		tm.cbSize = sizeof(TRACKMOUSEEVENT);
		tm.dwFlags = TME_LEAVE;
		tm.hwndTrack = hToolbar;
		TrackMouseEvent(&tm);
	}

	if (NULL != hStatusbar && 0 == (WS_DISABLED & statusStyle))
		SetWindowLongPtr(hStatusbar, GWL_STYLE, statusStyle);

	return result;
}

static BOOL BrowserView_SetStatusText(HWND hwnd, LPCWSTR pszText)
{
	HWND hStatusbar = GetDlgItem(hwnd, IDC_STATUSBAR);
	if (NULL == hStatusbar) return FALSE;

	Statusbar_Update(hStatusbar, pszText);
	return TRUE;
}

static BOOL BrowserView_SetStatusTextRes(HWND hwnd, LPCWSTR pszText)
{
	if (NULL != pszText && IS_INTRESOURCE(pszText))
	{
		WCHAR szBuffer[512] = {0};
		pszText = Plugin_LoadString((INT)(INT_PTR)pszText, szBuffer, ARRAYSIZE(szBuffer));
	}
	return BrowserView_SetStatusText(hwnd, pszText);
}

static void BrowserView_RegisterUiHook(HWND hwnd)
{
	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view || NULL != view->browserHook) return;

	if (FAILED(BrowserUiHook::CreateInstance(hwnd, FALSE, &view->browserHook)))
		return;

	view->browserHook->Register(view->browserManager, view->service);
}

static LRESULT BrowserView_OnCreate(HWND hwnd, CREATESTRUCT *pcs)
{
	BROWSERVIEWCREATEPARAM *createParam = (BROWSERVIEWCREATEPARAM*)pcs->lpCreateParams;

	BROWSERVIEW *view = (BROWSERVIEW*)calloc(1, sizeof(BROWSERVIEW));
	if (NULL != view)
	{
		SetLastError(ERROR_SUCCESS);
		if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)view) && ERROR_SUCCESS != GetLastError())
		{
			free(view);
			view = NULL;
		}
	}

	if (NULL == view)
	{
		DestroyWindow(hwnd);
		return -1;
	}
	
	if (NULL != createParam)
	{
		view->browserManager = createParam->browserManager;
		if (NULL != view->browserManager)
			view->browserManager->AddRef();

		view->service = createParam->service;
		if (NULL != view->service)
			view->service->AddRef();

		if (NULL != createParam->redirectUrl)
			view->storedUrl = Plugin_DuplicateResString(createParam->redirectUrl);
	}

	BrowserView_RegisterUiHook(hwnd);

	BrowserView_CreateToolbar(hwnd);
	BrowserView_CreateStatusbar(hwnd);
	
	UINT hostStyle = 0;
	if (0 != (NBCS_DISABLECONTEXTMENU & pcs->style)) hostStyle |= NBHS_DISABLECONTEXTMENU;
	if (0 != (NBCS_DIALOGMODE & pcs->style)) hostStyle |= NBHS_DIALOGMODE;
	if (0 != (NBCS_DISABLEHOSTCSS & pcs->style)) 
		hostStyle |= NBHS_DISABLEHOSTCSS;

	HACCEL hAccel = BrowserControl_GetAccelTable(ACCELTABLE_VIEW);

	HWND hHost = BrowserHost_CreateWindow(view->browserManager, hwnd, hostStyle, 0, 0, 0, 0, IDC_BROWSER, hAccel);
		
	HWND hControl; 
	if (NULL != (hControl = GetDlgItem(hwnd, IDC_TOOLBAR)))
	{
		Toolbar_SetBrowserHost(hControl, hHost);
	}

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_STATUSBAR)))
	{
		Statusbar_SetBrowserHost(hControl, hHost);
		Statusbar_SetActive(hControl, (0 == (WS_DISABLED & GetWindowLongPtr(hControl, GWL_STYLE))));
	}

	if (NULL != hAccel)
	{
		ifc_wasabihelper *wasabiHelper;
		if (SUCCEEDED(Plugin_GetWasabiHelper(&wasabiHelper)))
		{
			api_application *app;
			if (SUCCEEDED(wasabiHelper->GetApplicationApi(&app)))
			{ 			
				app->app_addAccelerators(hwnd, &hAccel, 1, TRANSLATE_MODE_CHILD);

				app->Release();
			}
			wasabiHelper->Release();
		}
	}
		
	if(NULL != view->browserManager)
	{		
		ifc_ombrowserwndmngr *windowManager;
		if (SUCCEEDED(view->browserManager->QueryInterface(IFC_OmBrowserWindowManager, (void**)&windowManager)))
		{
			windowManager->RegisterWindow(hwnd, &WTID_BrowserView);
			windowManager->Release();
		}

		ifc_ombrowsereventmngr *eventManager;
		if (SUCCEEDED(view->browserManager->QueryInterface(IFC_OmBrowserEventManager, (void**)&eventManager)))
		{
			eventManager->Signal_WindowCreate(hwnd, &WTID_BrowserView);
			eventManager->Release();
		}
	}
	return 0;
}

static void BrowserView_OnDestroy(HWND hwnd)
{
	BROWSERVIEW *view = GetView(hwnd);
	SetWindowLongPtr(hwnd, 0, 0L);
	
	HWND hHost = GetDlgItem(hwnd, IDC_BROWSER);
	if (NULL != hHost)
	{
		DWORD hostStyle = GetWindowStyle(hHost);
		if (0 != (WS_VISIBLE & hostStyle))
			SetWindowLongPtr(hHost, GWL_STYLE, hostStyle & ~WS_VISIBLE);
		
		HWND hWinamp;
		if (SUCCEEDED(Plugin_GetWinampWnd(&hWinamp)))
		{
			SetWindowLongPtr(hHost, GWLP_HWNDPARENT, (LONGX86)(LONG_PTR)hWinamp);
		}
		
		if (NULL == view->browserManager || 
			S_OK == view->browserManager->IsFinishing() || 
			0 == PostMessage(hHost, NBHM_DESTROY, 0, 0L))
		{
			DWORD_PTR result;
			SendMessageTimeout(hHost, NBHM_DESTROY, TRUE, 0L, SMTO_NOTIMEOUTIFNOTHUNG | SMTO_BLOCK, 500, &result);
		}
	}
	else
	{
		aTRACE_LINE("browser control not created");
	}
	
	if (NULL != view)
	{	
		view->extendedStyle &= ~(NBCS_EX_BROWSERREADY | NBCS_EX_NAVCOMPLETED );
		view->extendedStyle |= NBCS_EX_BLOCKNAVIGATION;
			
		if (NULL != view->browserHook)
		{
			view->browserHook->Unregister(view->browserManager, view->service);
			view->browserHook->Release();
		}
		
		if (NULL != view->service)
		{
			view->service->Release();
			view->service = NULL;
		}

		Plugin_FreeResString(view->storedUrl);
		view->storedUrl = NULL;

		SysFreeString(view->storedData);
		view->storedData = NULL;

		if (NULL != view->browserManager)
		{
			ifc_ombrowserwndmngr *windowManager;
			if (SUCCEEDED(view->browserManager->QueryInterface(IFC_OmBrowserWindowManager, (void**)&windowManager)))
			{
				windowManager->UnregisterWindow(hwnd);
				windowManager->Release();
			}

			ifc_ombrowsereventmngr *eventManager;
			if (SUCCEEDED(view->browserManager->QueryInterface(IFC_OmBrowserEventManager, (void**)&eventManager)))
			{
				eventManager->Signal_WindowClose(hwnd, &WTID_BrowserPopup);
				eventManager->Release();
			}
	
			view->browserManager->Release();
			view->browserManager = NULL;
		}
		free(view);
	}

	ifc_wasabihelper *wasabiHelper;
	if (SUCCEEDED(Plugin_GetWasabiHelper(&wasabiHelper)))
	{
		api_application *app;
		if (SUCCEEDED(wasabiHelper->GetApplicationApi(&app)))
		{ 
			app->app_removeAccelerators(hwnd);
			app->Release();
		}
		wasabiHelper->Release();
	}
}

static void BrowserView_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right)
			BrowserView_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void BrowserView_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
		BrowserView_Paint(hwnd, hdc, &clientRect, 0 != (PRF_ERASEBKGND & options));
}

static void BrowserView_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags))
		return;

	BROWSERVIEW *view = GetView(hwnd);
	if (NULL != view)
	{

		BrowserControl_UpdateLayout(hwnd, 0 == (SWP_NOREDRAW & pwp->flags), 
			0 != (SWP_FRAMECHANGED & pwp->flags), view->updateRegion, &view->regionOffset);
	}
}



static void BrowserView_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND hControl)
{
	switch(commandId)
	{		
		case IDC_TOOLBAR:	BrowserControl_ProcessToolbarCommand(hwnd, eventId); break;
		case IDC_STATUSBAR:	BrowserControl_ProcessStatusbarCommand(hwnd, eventId); break;
		default:			BrowserControl_ProcessCommonCommand(hwnd, commandId); break;
	}


}

static void BrowserView_OnUpdateSkin(HWND hwnd)
{
	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view) return;
	
	ifc_skinnedbrowser *skinnedBrowser;
	if (SUCCEEDED(Plugin_GetBrowserSkin(&skinnedBrowser)))
	{
		view->rgbBack = skinnedBrowser->GetBackColor();
		skinnedBrowser->Release();
	}
	else
		view->rgbBack = GetSysColor(COLOR_WINDOW);
	
	HWND hControl;
	if (NULL != (hControl = GetDlgItem(hwnd, IDC_BROWSER))) 
		PostMessage(hControl, NBHM_UPDATESKIN, 0, 0L);

	if (NULL != (hControl = BrowserControl_GetOperationWidget(hwnd)))
		Curtain_UpdateSkin(hControl, FALSE);

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_STATUSBAR))) 
		Statusbar_UpdateSkin(hControl, FALSE);
		
	if (NULL != (hControl = GetDlgItem(hwnd, IDC_TOOLBAR))) 
		Toolbar_UpdateSkin(hControl, FALSE);
	
	BrowserControl_UpdateLayout(hwnd, TRUE, TRUE, NULL, NULL);
}

static void BrowserView_OnBrowserReady(HWND hwnd)
{
	ReplyMessage(0);

	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view) return;

	view->extendedStyle |= NBCS_EX_BROWSERREADY;
		
	BrowserControl_UpdateLayout(hwnd, FALSE, FALSE, NULL, NULL);
	BrowserView_PreformNavigation(hwnd);

	HWND hToolbar = BrowserControl_GetToolbar(hwnd);
	if (NULL != hToolbar) 
		Toolbar_EnableItem(hToolbar, TOOLITEM_ADDRESSBAR, TRUE);
}

static void BrowserView_OnDocumentReady(HWND hwnd)
{
	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view) return;
	
	if (0 == (NBCS_EX_NAVCOMPLETED & view->extendedStyle))
	{		
		view->extendedStyle |= NBCS_EX_NAVCOMPLETED;
		if (NULL != view->storedData)
		{
			BSTR documentData = view->storedData;
			view->storedData = NULL;
			if (FALSE == BrowserView_WriteDocument(hwnd, documentData, FALSE))
				SysFreeString(documentData);
		}
		
		HWND hHost = GetDlgItem(hwnd, IDC_BROWSER);
		if (NULL != hHost && 
			0 == (WS_VISIBLE & GetWindowStyle(hHost)))
		{	
			ShowWindowAsync(hHost, SW_SHOWNA);
		}
		
	}
}

static void BrowserView_OnBrowserActive(HWND hwnd, BOOL fActive)
{
	ReplyMessage(0);

	HWND hToolbar = GetDlgItem(hwnd, IDC_TOOLBAR);
	if (NULL != hToolbar)
	{
		Toolbar_EnableItem(hToolbar, TOOLITEM_DOWNLOADPROGRESS, fActive);
	}

	HWND hStatusbar = GetDlgItem(hwnd, IDC_STATUSBAR);
	if (NULL != hStatusbar )
	{
		if (FALSE != fActive && (0 != (WS_DISABLED & GetWindowLongPtr(hStatusbar, GWL_STYLE))))
			fActive = FALSE;
		Statusbar_SetActive(hStatusbar, fActive);
	}
}

static void BrowserView_OnCommandStateChange(HWND hwnd, UINT commandId, BOOL fEnable)
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
static void BrowserView_OnStatusChange(HWND hwnd, LPCWSTR pszText)
{
	WCHAR szBuffer[512] = {0};
	if (NULL == pszText || L'\0' == *pszText)
		szBuffer[0] = L'\0';
	else
		StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), pszText);

	ReplyMessage(0);

	BrowserView_SetStatusText(hwnd, szBuffer);
}



static void BrowserView_OnSecureIconChange(HWND hwnd, UINT iconId)
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

static BOOL BrowserView_OnGetOmService(HWND hwnd, void **serviceInstance)
{
	if (NULL == serviceInstance)
		return FALSE;

	*serviceInstance = NULL;

	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view || NULL == view->service) return FALSE;

	*serviceInstance = view->service;
	view->service->AddRef();
	return TRUE;
}

static LRESULT BrowserView_OnCreatePopup(HWND hwnd, DISPATCHAPC callback, ULONG_PTR param)
{
	ReplyMessage(TRUE);

	HWND hPopup = NULL;

	UINT windowStyle = GetWindowStyle(hwnd);
	if (0 == (NBCS_BLOCKPOPUP & windowStyle))
	{
		BROWSERVIEW *view = GetView(hwnd);
		if (NULL != view)
		{
			RECT windowRect;
			GetWindowRect(hwnd, &windowRect);
			
			UINT popupStyle = NBCS_NOSERVICECOMMANDS | NBCS_DISABLEHOSTCSS;
			popupStyle |= (NBCS_POPUPOWNER  & GetWindowStyle(hwnd));
			
			HWND hOwner = NULL;
			if (0 != (NBCS_POPUPOWNER & popupStyle))
				hOwner = hwnd;
						
			hPopup = BrowserPopup_Create(view->browserManager, view->service, popupStyle, 
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

static void BrowserView_OnAllowFocusChange(HWND hwnd, BOOL *fAllow)
{
	*fAllow = FALSE;
}

static void BrowserView_OnNavigateComplete(HWND hwnd, IDispatch *pDispath, VARIANT *URL, BOOL fTopFrame)
{	
	if (FALSE == fTopFrame) 
		return;

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

static void BrowserView_OnTitleChange(HWND hwnd, LPCWSTR pszText)
{
	WCHAR szBuffer[256] = {0};
	if (NULL == pszText || L'\0' == *pszText) szBuffer[0] = L'\0';
	else StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), pszText);

	ReplyMessage(0);
	
	HWND hToolbar = GetDlgItem(hwnd, IDC_TOOLBAR);
	if (NULL != hToolbar)
	{
		INT itemId = Toolbar_FindItem(hToolbar, TOOLITEM_ADDRESSBAR);
		if (ITEM_ERR != itemId)
			Toolbar_SetItemDescription(hToolbar, MAKEINTRESOURCE(itemId),  szBuffer); 
	}
}

static void BrowserView_OnBrowserClosing(HWND hwnd, BOOL isChild, BOOL *fCancel)
{	
	*fCancel = TRUE;
	ReplyMessage(0);
}

static LRESULT BrowserView_OnBrowserNotify(HWND hwnd, NMHDR *pnmh)
{
	switch(pnmh->code)
	{
		case NBHN_READY:
			BrowserView_OnBrowserReady(hwnd);
			break;
		case NBHN_DOCUMENTREADY:
			BrowserView_OnDocumentReady(hwnd);
			break;
		case NBHN_BROWSERACTIVE:
			BrowserView_OnBrowserActive(hwnd, ((BHNACTIVE*)pnmh)->fActive);
			break;
		case NBHN_COMMANDSTATECHANGE:
			BrowserView_OnCommandStateChange(hwnd, ((BHNCMDSTATE*)pnmh)->commandId, ((BHNCMDSTATE*)pnmh)->fEnabled);
			break;
		case NBHN_STATUSCHANGE:
			BrowserView_OnStatusChange(hwnd, ((BHNTEXTCHANGE*)pnmh)->pszText);
			break;
		case NBHN_SECUREICONCHANGE:
			BrowserView_OnSecureIconChange(hwnd, ((BHNSECUREICON*)pnmh)->iconId);
			break;
		case NBHN_GETOMSERVICE:
			return BrowserView_OnGetOmService(hwnd, &((BHNSERVICE*)pnmh)->instance);
		case NBHN_CREATEPOPUP:
			return BrowserView_OnCreatePopup(hwnd, ((BHNCREATEPOPUP*)pnmh)->callback, ((BHNCREATEPOPUP*)pnmh)->param);
		case NBHN_FOCUSCHANGE:
			BrowserView_OnAllowFocusChange(hwnd, &((BHNFOCUSCHANGE*)pnmh)->fAllow);
			break;
		case NBHN_NAVIGATECOMPLETE:
			BrowserView_OnNavigateComplete(hwnd, ((BHNNAVCOMPLETE*)pnmh)->pDispatch, ((BHNNAVCOMPLETE*)pnmh)->URL, ((BHNNAVCOMPLETE*)pnmh)->fTopFrame);
			break;
		case NBHN_TITLECHANGE:
			BrowserView_OnTitleChange(hwnd, ((BHNTEXTCHANGE*)pnmh)->pszText);
			break;
		case NBHN_CLOSING:
			BrowserView_OnBrowserClosing(hwnd, ((BHNCLOSING*)pnmh)->isChild, &((BHNCLOSING*)pnmh)->cancel);
			break;
	}
	return 0;
}

static LRESULT BrowserView_OnNotify(HWND hwnd, INT controlId, NMHDR *pnmh)
{
	switch(controlId)
	{
	case IDC_BROWSER:
		return BrowserView_OnBrowserNotify(hwnd, pnmh);
	}

	return 0;
}



static LRESULT BrowserView_OnNavigate(HWND hwnd, LPCWSTR navigateUrl, BOOL fScheduleBlocked)
{
	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view) return FALSE;

	UINT windowStyle = GetWindowStyle(hwnd);

	if (NULL != view->browserHook)
		view->browserHook->CheckBlockedState(view->service);

	Plugin_FreeResString(view->storedUrl);
	view->storedUrl = NULL;
		
	if (NBCS_EX_BROWSERREADY != ((NBCS_EX_BROWSERREADY | NBCS_EX_BLOCKNAVIGATION) & view->extendedStyle))
	{		
		if (FALSE == fScheduleBlocked) 
			return FALSE;
		
		view->storedUrl = Plugin_DuplicateResString(navigateUrl);
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
				BrowserView_SetStatusText(hwnd, NULL);
				url = SysAllocString(L"about:blank");
				break;

			case NAVIGATE_HOME:
				if (0 == (NBCS_NOSERVICECOMMANDS & windowStyle) && NULL != view->service)
				{
					WCHAR szBuffer[8192] = {0};
					if (SUCCEEDED(view->service->GetUrl(szBuffer, ARRAYSIZE(szBuffer))))
					{
						BrowserView_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_HOME_DESCRIPTION));
						url = SysAllocString(szBuffer);
					}

					if (SUCCEEDED(view->service->GetName(szBuffer, ARRAYSIZE(szBuffer))))
						pszDescription = Plugin_CopyString(szBuffer);
				}
				
				break;
			case NAVIGATE_BACK:
				BrowserView_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_BACK_DESCRIPTION));
				return PostMessage(hHost, NBHM_CONTAINERCOMMAND, (WPARAM)Browser::commandBack, 0L);
			case NAVIGATE_FORWARD:
				BrowserView_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_FORWARD_DESCRIPTION));
				return PostMessage(hHost, NBHM_CONTAINERCOMMAND, (WPARAM)Browser::commandForward, 0L);
			case NAVIGATE_STOP:
				BrowserView_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_STOP_DESCRIPTION));
				return PostMessage(hHost, NBHM_CONTAINERCOMMAND, (WPARAM)Browser::commandStop, 0L);
			case NAVIGATE_REFRESH:
				BrowserView_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_REFRESH_DESCRIPTION));
				return PostMessage(hHost, NBHM_CONTAINERCOMMAND, (WPARAM)Browser::commandRefresh, 0L);
			case NAVIGATE_REFRESH_COMPLETELY:
				BrowserView_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_REFRESH_DESCRIPTION));
				return PostMessage(hHost, NBHM_CONTAINERCOMMAND, (WPARAM)Browser::commandRefreshCompletely, 0L);
		}
	}
	else
	{
		BrowserView_SetStatusTextRes(hwnd, MAKEINTRESOURCE(IDS_NAVIGATING));
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

static LRESULT BrowserView_OnWriteDocument(HWND hwnd, BSTR documentData, BOOL fScheduleBlocked)
{
	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view) return FALSE;

	SysFreeString(view->storedData);
	view->storedData = NULL;

	Plugin_FreeResString(view->storedUrl);
	view->storedUrl = NULL;
	
	if (0 == (NBCS_EX_NAVCOMPLETED & view->extendedStyle))
	{		
		if (FALSE == fScheduleBlocked) 
			return FALSE;
		view->storedData = documentData;
		return TRUE;
	}

	HWND hHost = GetDlgItem(hwnd, IDC_BROWSER);
	if (NULL == hHost) return FALSE;

	return PostMessage(hHost, NBHM_WRITEDOCUMENT, 0, (LPARAM)documentData);
}

static void BrowserView_OnSetUpdateRegion(HWND  hwnd, HRGN updateRegion, POINTS regionOffset)
{
	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view) return;

	view->updateRegion = updateRegion;
	
	
	view->regionOffset.x = regionOffset.x;
	view->regionOffset.y = regionOffset.y;
}

static LRESULT BrowserView_OnGetService(HWND hwnd, ifc_omservice **serviceOut)
{
	if (NULL == serviceOut) return FALSE;

	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view || NULL == view->service)
	{
		*serviceOut = NULL;
		return FALSE;
	}

	*serviceOut = view->service;
	(*serviceOut)->AddRef();
	return TRUE;
}


static LRESULT BrowserView_OnGetBrowserObject(HWND hwnd, obj_ombrowser **browserOut)
{
	if (NULL == browserOut) return FALSE;

	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view || NULL == view->browserManager)
	{
		*browserOut = NULL;
		return FALSE;
	}

	*browserOut = view->browserManager;
	(*browserOut)->AddRef();
	return TRUE;
}
static LRESULT BrowserView_OnGetToolbar(HWND hwnd)
{
	if (0 != (NBCS_NOTOOLBAR & GetWindowStyle(hwnd)))
		return 0;

	return (LRESULT)GetDlgItem(hwnd, IDC_TOOLBAR);
}

static LRESULT BrowserView_OnGetStatusbar(HWND hwnd)
{
	if (0 != (NBCS_NOSTATUSBAR & GetWindowStyle(hwnd)))
		return 0;

	return (LRESULT)GetDlgItem(hwnd, IDC_STATUSBAR);
}

static LRESULT BrowserView_OnGetHost(HWND hwnd)
{
	return (LRESULT)GetDlgItem(hwnd, IDC_BROWSER);
}

static LRESULT BrowserView_OnAppCommand(HWND hwnd, HWND hTarget, INT commandId, INT deviceId, INT keys)
{
	return BrowserControl_ProcessAppCommand(hwnd, commandId);
}
static LRESULT BrowserView_OnNavigateStoredUrl(HWND hwnd)
{
	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view || NULL == view->storedUrl) 
		return FALSE;

	LPWSTR url = view->storedUrl;
	view->storedUrl = NULL;
	BOOL result = BrowserView_Navigate(hwnd, url, TRUE);
	Plugin_FreeResString(url);
	return result;
}

static LRESULT BrowserView_OnSetExtendedStyle(HWND hwnd, UINT extMask, UINT extStyle)
{
	BROWSERVIEW *view = GetView(hwnd);
	if (NULL == view) return 0;

	UINT currentStyle = view->extendedStyle;
	view->extendedStyle = (currentStyle & ~extMask) | (extStyle & extMask);
	return currentStyle;
}

static LRESULT BrowserView_OnGetExtendedStyle(HWND hwnd)
{
	BROWSERVIEW *view = GetView(hwnd);
	return (NULL != view) ? view->extendedStyle : 0;
}

static LRESULT CALLBACK BrowserView_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE:				return BrowserView_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:				BrowserView_OnDestroy(hwnd); break;
		case WM_ERASEBKGND:			return 0;
		case WM_PAINT:				BrowserView_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:			BrowserView_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_WINDOWPOSCHANGED:	BrowserView_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_COMMAND:				BrowserView_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case WM_NOTIFY:				return BrowserView_OnNotify(hwnd, (INT)wParam, (NMHDR*)lParam);
		case WM_APPCOMMAND:			return BrowserView_OnAppCommand(hwnd, (HWND)wParam, GET_APPCOMMAND_LPARAM(lParam), GET_DEVICE_LPARAM(lParam), GET_KEYSTATE_LPARAM(lParam));

		case NBCM_GETTOOLBAR:			return BrowserView_OnGetToolbar(hwnd);
		case NBCM_GETSTATUSBAR:		return BrowserView_OnGetStatusbar(hwnd);
		case NBCM_GETHOST:			return BrowserView_OnGetHost(hwnd);
		case NBCM_UPDATESKIN:			BrowserView_OnUpdateSkin(hwnd); return 0;
		case NBCM_GETSERVICE:			return BrowserView_OnGetService(hwnd, (ifc_omservice**)lParam);
		case NBCM_NAVIGATE:			return BrowserView_OnNavigate(hwnd, (LPCWSTR)lParam, (BOOL)wParam);
		case NBCM_WRITEDOCUMENT:		return BrowserView_OnWriteDocument(hwnd, (BSTR)lParam, (BOOL)wParam);
		case NBCM_GETBROWSEROBJECT:	return BrowserView_OnGetBrowserObject(hwnd, (obj_ombrowser**)lParam);
		case NBCM_SHOWOPERATION:	return BrowserControl_OnShowOperation(hwnd, (OPERATIONINFO*)lParam);
		case NBCM_NAVSTOREDURL:		return BrowserView_OnNavigateStoredUrl(hwnd);
		case NBCM_BLOCK:			BrowserControl_SetBlockedState(hwnd, (BOOL)lParam); return 0;
		case NBCM_SETEXTSTYLE:		return BrowserView_OnSetExtendedStyle(hwnd, (UINT)wParam, (UINT)lParam);
		case NBCM_GETEXTSTYLE:		return BrowserView_OnGetExtendedStyle(hwnd);

		// gen_ml flickerless drawing
		case WM_USER + 0x200:		return 1;
		case WM_USER + 0x201:		BrowserView_OnSetUpdateRegion(hwnd, (HRGN)lParam, MAKEPOINTS(wParam)); return 0;
	}

	if (FALSE != Plugin_IsDirectMouseWheelMessage(uMsg))
	{
		SendMessage(hwnd, WM_MOUSEWHEEL, wParam, lParam);
		return TRUE;
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static BOOL BrowserView_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;
	if (GetClassInfo(hInstance, NWC_OMBROWSERVIEW, &wc)) return TRUE;

	ZeroMemory(&wc, sizeof(WNDCLASS));

	wc.hInstance		= hInstance;
	wc.lpszClassName	= NWC_OMBROWSERVIEW;
	wc.lpfnWndProc	= BrowserView_WindowProc;
	wc.style			= CS_DBLCLKS;
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.cbWndExtra	= sizeof(BROWSERVIEW*);

	return ( 0 != RegisterClassW(&wc));
}