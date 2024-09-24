#include "main.h"
#include "./options.h"
#include "./optionsHook.h"
#include "./resource.h"

#include "./ifc_omconfig.h"
#include "./ifc_ombrowserconfig.h"
#include "./ifc_omtoolbarconfig.h"
#include "./ifc_omstatusbarconfig.h"

#include "../winamp/wa_ipc.h"

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <strsafe.h>

HWND CALLBACK OptionsUI_CreatePage(HWND hParent, UINT style);
HWND CALLBACK OptionsDebug_CreatePage(HWND hParent, UINT style);
HWND CALLBACK OptionsInfo_CreatePage(HWND hParent, UINT style);

static const OPTIONSPAGECREATOR szPageList[] = 
{
	OptionsUI_CreatePage,
	OptionsDebug_CreatePage,
	OptionsInfo_CreatePage,
};

typedef struct __OPTIONSDATA
{
	obj_ombrowser *browserManager;
	BROWSEROPTIONSCALLBACK callback;
	ULONG_PTR	user;
	UINT		configCookie;
	UINT		style;
} OPTIONSDATA;

#define OPTIONS_PROP		L"OptionsDataProp"
#define GetOptions(__hwnd)	((OPTIONSDATA*)GetProp(__hwnd, OPTIONS_PROP))

#define IDC_ACTIVEPAGE		10001

static INT_PTR  CALLBACK OptionsFrame_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HRESULT BrowserOptions_ShowDialog(obj_ombrowser *browserManager, HWND hOwner, UINT style, BROWSEROPTIONSCALLBACK callback, ULONG_PTR user)
{
	OPTIONSDATA optionsData = {0};
	optionsData.browserManager = browserManager;
	optionsData.callback = callback;
	optionsData.user = user;
	optionsData.style = style;

	INT_PTR result = Plugin_DialogBoxParam(MAKEINTRESOURCE(IDD_BROWSER_OPTIONS), hOwner, 
										   OptionsFrame_DialogProc, (LPARAM)&optionsData);

	return (0 == result) ? S_OK : E_FAIL;
}

BOOL Options_SetCheckbox(HWND hwnd, UINT controlId, HRESULT checkedState)
{
	HWND hControl = GetDlgItem(hwnd, controlId);
	if (NULL == hControl) return FALSE;

	if (FAILED(checkedState))
	{
		EnableWindow(hControl, FALSE);
	}
	else
	{
		WPARAM wParam = (S_OK == checkedState) ? BST_CHECKED : BST_UNCHECKED;
		SendMessage(hControl, BM_SETCHECK, wParam, 0L);
		EnableWindow(hControl, TRUE);
	}

	return TRUE;
}

static void OptionsFrame_NotifyTabSelected(HWND hwnd)
{
	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame) return;

	NMHDR nmh = {0};
	nmh.code = TCN_SELCHANGE;
	nmh.hwndFrom = hFrame;
	nmh.idFrom = GetDlgCtrlID(hFrame);
	SendNotifyMessage(hwnd, WM_NOTIFY, (WPARAM)nmh.idFrom, (LPARAM)&nmh);
}

static void OptionsFrame_InitializePages(HWND hwnd, UINT style)
{
	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame) return;

	HWND hWinamp = NULL;
	if (FAILED(Plugin_GetWinampWnd(&hWinamp))) 
		hWinamp = NULL;

	INT pageCount = 0;
	WCHAR szBuffer[256] = {0};
	TCITEM tabItem = {0};

	tabItem.mask = TCIF_TEXT | TCIF_PARAM;

	for (size_t i = 0; i < ARRAYSIZE(szPageList); i++)
	{	
		HWND hPage = szPageList[i](hwnd, style);
		if (NULL == hPage) continue;

		if (NULL != hWinamp && 0 == SENDWAIPC(hWinamp, IPC_USE_UXTHEME_FUNC, IPC_ISWINTHEMEPRESENT))
			SENDWAIPC(hWinamp, IPC_USE_UXTHEME_FUNC, hPage);

		if (0 == GetWindowText(hPage, szBuffer, ARRAYSIZE(szBuffer)))
			StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), L"Page %d", pageCount + 1);

		tabItem.lParam = (LPARAM)hPage;
		tabItem.pszText = szBuffer;
		tabItem.iImage = 0;

		if (-1 != TabCtrl_InsertItem(hFrame, pageCount, &tabItem))
		{
			pageCount++;
		}
	}

	TabCtrl_SetCurSel(hFrame, 0);
}

void getViewport(RECT *r, HWND wnd, int full, RECT *sr)
{
	POINT *p = NULL;
	if (p || sr || wnd)
	{
		HMONITOR hm = NULL;
		if (sr)
			hm = MonitorFromRect(sr, MONITOR_DEFAULTTONEAREST);
		else if (wnd)
			hm = MonitorFromWindow(wnd, MONITOR_DEFAULTTONEAREST);
		else if (p)
			hm = MonitorFromPoint(*p, MONITOR_DEFAULTTONEAREST);

		if (hm)
		{
			MONITORINFOEXW mi;
			memset(&mi, 0, sizeof(mi));
			mi.cbSize = sizeof(mi);

			if (GetMonitorInfoW(hm, &mi))
			{
				if (!full)
					*r = mi.rcWork;
				else
					*r = mi.rcMonitor;

				return ;
			}
		}
	}
	if (full)
	{ // this might be borked =)
		r->top = r->left = 0;
		r->right = GetSystemMetrics(SM_CXSCREEN);
		r->bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		SystemParametersInfoW(SPI_GETWORKAREA, 0, r, 0);
	}
}

BOOL windowOffScreen(HWND hwnd, POINT pt)
{
	RECT r = {0}, wnd = {0}, sr = {0};
	GetWindowRect(hwnd, &wnd);
	sr.left = pt.x;
	sr.top = pt.y;
	sr.right = sr.left + (wnd.right - wnd.left);
	sr.bottom = sr.top + (wnd.bottom - wnd.top);
	getViewport(&r, hwnd, 0, &sr);
	return !PtInRect(&r, pt);
}

static INT_PTR OptionsFrame_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM param)
{
	HWND hWinamp = NULL;
	if (FAILED(Plugin_GetWinampWnd(&hWinamp))) 
		hWinamp = NULL;

	OPTIONSDATA *optionsData = (OPTIONSDATA*)param;
	SetProp(hwnd, OPTIONS_PROP, optionsData);

	OptionsConfigHook *configHook;
	if (SUCCEEDED(OptionsConfigHook::CreateInstance(hwnd, &configHook)))
	{
		ifc_omconfig *config;
		if (NULL != optionsData->browserManager && SUCCEEDED(optionsData->browserManager->GetConfig(NULL, (void**)&config)))
		{
			config->RegisterCallback(configHook, &optionsData->configCookie);
			config->Release();
		}
		configHook->Release();
	}

	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL != hFrame)
	{
		if (NULL != hWinamp && 0 == SENDWAIPC(hWinamp, IPC_USE_UXTHEME_FUNC, IPC_ISWINTHEMEPRESENT))
		SENDWAIPC(hWinamp, IPC_USE_UXTHEME_FUNC, hFrame);

		TabCtrl_SetMinTabWidth(hFrame, 60);
		OptionsFrame_InitializePages(hwnd, optionsData->style);
		OptionsFrame_NotifyTabSelected(hwnd);

		UINT style = GetWindowStyle(hFrame);
		style |= TCS_HOTTRACK | TCS_TABS | TCS_MULTILINE | TCS_RIGHTJUSTIFY;
		SetWindowLongPtr(hFrame, GWL_STYLE, style);
	}

	PostMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0L);

	if (0 != optionsData->callback)
		optionsData->callback(hwnd, BOCALLBACK_INIT, optionsData->user);

	// show config window and restore last position as applicable				
	obj_ombrowser *browserManager;
	if (FALSE != SendMessage(hwnd, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
	{
		ifc_ombrowserconfig *browserConfig;
		if (SUCCEEDED(browserManager->GetConfig(&IFC_OmBrowserConfig, (void**)&browserConfig)))
		{
			POINT pt = {-1, -1};

			pt.x = browserConfig->GetX();
			pt.y = browserConfig->GetY();
			browserConfig->Release();

			if (!windowOffScreen(hwnd, pt))
				SetWindowPos(hwnd, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
		}
		browserManager->Release();
	}

	return 0;
}

static void OptionsFrame_OnDestroy(HWND hwnd)
{
	obj_ombrowser *browserManager;
	if (FALSE != SendMessage(hwnd, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
	{
		ifc_ombrowserconfig *browserConfig;
		if (SUCCEEDED(browserManager->GetConfig(&IFC_OmBrowserConfig, (void**)&browserConfig)))
		{
			RECT rect = {0};
			GetWindowRect(hwnd, &rect);

			browserConfig->SetX(rect.left);
			browserConfig->SetY(rect.top);
			browserConfig->Release();
		}
		browserManager->Release();
	}

	OPTIONSDATA *optionsData = GetOptions(hwnd);
	RemoveProp(hwnd, OPTIONS_PROP);

	if (NULL == optionsData) 
		return;

	if (0 != optionsData->configCookie)
	{
		ifc_omconfig *config;
		if (NULL != optionsData->browserManager && SUCCEEDED(optionsData->browserManager->GetConfig(NULL, (void**)&config)))
		{
			config->UnregisterCallback(optionsData->configCookie);
			config->Release();
		}
		optionsData->configCookie = 0;
	}
}

static void OptionsFrame_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
}

static void OptionsFrame_OnTabSelected(HWND hwnd, HWND hFrame)
{
	if (NULL == hFrame) return;

	HWND hPageOld = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
	if (NULL != hPageOld)
		SetWindowLongPtr(hPageOld, GWLP_ID, 0);

	INT iSelected = TabCtrl_GetCurSel(hFrame);
	TCITEM tabItem;
	tabItem.mask = TCIF_PARAM;

	HWND hPage = (TRUE == TabCtrl_GetItem(hFrame, iSelected, &tabItem)) ? (HWND)tabItem.lParam : NULL;

	if (NULL != hPage)
	{
		SetWindowLongPtr(hPage, GWLP_ID, IDC_ACTIVEPAGE);

		RECT rcTab;
		GetClientRect(hFrame, &rcTab);
		MapWindowPoints(hFrame, hwnd, (POINT*)&rcTab, 2);
		TabCtrl_AdjustRect(hFrame, FALSE, &rcTab);

		SetWindowPos(hPage, HWND_TOP, rcTab.left, rcTab.top,
					 rcTab.right - rcTab.left, rcTab.bottom - rcTab.top,
					 SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW);
	}

	if (NULL != hPage)
	{
		ShowWindow(hPage, SW_SHOWNA);
	}

	if (NULL != hPageOld)
	{
		ShowWindow(hPageOld, SW_HIDE);
		if (NULL != hPage)
			RedrawWindow(hPage, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_NOINTERNALPAINT | RDW_NOERASE);
	}
}

static void OptionsFrame_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND hControl)
{
	switch(commandId)
	{
		case IDOK:
		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
	}
}

static LRESULT OptionsFrame_OnNotify(HWND hwnd, INT controlId, NMHDR *pnmh)
{
	switch(controlId)
	{
		case IDC_TABFRAME:
			switch(pnmh->code)
			{
				case TCN_SELCHANGE:
					OptionsFrame_OnTabSelected(hwnd, pnmh->hwndFrom);
					break;
			}
			break;
	}
	return 0;
}

static LRESULT OptionsFrame_OnGetBrowser(HWND hwnd, obj_ombrowser **ppBrowser)
{
	if (NULL == ppBrowser) return FALSE;

	OPTIONSDATA *optionsData = GetOptions(hwnd);
	if (NULL == optionsData || NULL == optionsData->browserManager)
		return FALSE;

	*ppBrowser = optionsData->browserManager;
	(*ppBrowser)->AddRef();
	return TRUE;
}

static void OptionsFrame_OnConfigChanged(HWND hwnd, BOMCONFIGCHANGED *configData)
{
	HWND hControl = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hControl) return;

	TCITEM item = {0};
	item.mask = TCIF_PARAM;

	INT count = (INT)SendMessage(hControl, TCM_GETITEMCOUNT, 0, 0L);
	for(INT i = 0; i < count; i++)
	{
		if (FALSE != (BOOL)SendMessage(hControl, TCM_GETITEM, (WPARAM)i, (LPARAM)&item) && 
			NULL != item.lParam)
		{
			SendMessage((HWND)item.lParam, BOM_CONFIGCHANGED, 0, (LPARAM)configData);
		}
	}
}

static INT_PTR CALLBACK OptionsFrame_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return OptionsFrame_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:			OptionsFrame_OnDestroy(hwnd); return 0;
		case WM_WINDOWPOSCHANGED:	OptionsFrame_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_COMMAND:			OptionsFrame_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
		case WM_NOTIFY:				MSGRESULT(hwnd, OptionsFrame_OnNotify(hwnd, (INT)wParam, (NMHDR*)lParam));
		case BOM_GETBROWSER:		MSGRESULT(hwnd, OptionsFrame_OnGetBrowser(hwnd, (obj_ombrowser**)lParam));
		case BOM_CONFIGCHANGED:		OptionsFrame_OnConfigChanged(hwnd, (BOMCONFIGCHANGED*)lParam); return TRUE;
	}
	return 0;
}