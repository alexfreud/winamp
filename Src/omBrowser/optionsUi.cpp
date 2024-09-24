#include "main.h"
#include "./options.h"
#include "./resource.h"

#include "./obj_ombrowser.h"
#include "./ifc_omconfig.h"
#include "./ifc_omtoolbarconfig.h"
#include "./ifc_omstatusbarconfig.h"

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <strsafe.h>

#define TOOLBAR_TOPDOCK			0
#define TOOLBAR_BOTTOMDOCK		1

#define BOOL2HRESULT(__result) ((FALSE != (__result)) ? S_OK : S_FALSE)

static INT_PTR CALLBACK OptionsUI_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND CALLBACK OptionsUI_CreatePage(HWND hParent, UINT style)
{
	return Plugin_CreateDialogParam(MAKEINTRESOURCE(IDD_OPTIONS_UI), hParent, OptionsUI_DialogProc, 0L);
}

static INT OptionsUI_GetLocationIndex(HWND hwnd, UINT locationId)
{
	HWND hControl = GetDlgItem(hwnd, IDC_TOOLBAR_LOCATION);
	if (NULL == hControl) return CB_ERR;

	INT count = (INT)SendMessage(hControl, CB_GETCOUNT, 0, 0L);
	for (INT i = 0; i < count; i++)
	{
		if (locationId == (UINT)SendMessage(hControl, CB_GETITEMDATA, i, 0L))
			return i;
	}
	return CB_ERR;
}

static void OptionsUI_UpdateToolbarLocation(HWND hwnd, HRESULT bottomLocation)
{
	HWND hControl = GetDlgItem(hwnd, IDC_TOOLBAR_LOCATION);
	if (NULL == hControl) return;

	if (FAILED(bottomLocation))
	{
		EnableWindow(hControl, FALSE);
	}
	else
	{
		INT iIndex = OptionsUI_GetLocationIndex(hwnd,  (S_OK == bottomLocation) ? TOOLBAR_BOTTOMDOCK : TOOLBAR_TOPDOCK);
		if (CB_ERR !=  iIndex) 
			SendMessage(hControl, CB_SETCURSEL, iIndex, 0L);
		
		EnableWindow(hControl, TRUE);
	}
}

static void OptionsUI_UpdateToolbarAutoHide(HWND hwnd, HRESULT autoHide)
{
	Options_SetCheckbox(hwnd, IDC_TOOLBAR_AUTOHIDE, autoHide);
}

static void OptionsUI_UpdateToolbarTabStop(HWND hwnd, HRESULT tabStop)
{
	Options_SetCheckbox(hwnd, IDC_TOOLBAR_TABSTOP, tabStop);
}

static void OptionsUI_UpdateToolbarForceAddress(HWND hwnd, HRESULT enabled)
{
	Options_SetCheckbox(hwnd, IDC_TOOLBAR_FORCEADDRESS, enabled);
}

static void OptionsUI_UpdateToolbarFancyAddress(HWND hwnd, HRESULT enabled)
{
	Options_SetCheckbox(hwnd, IDC_TOOLBAR_FANCYADDRESS, enabled);
}

static void OptionsUI_UpdateStatusbarEnabled(HWND hwnd, HRESULT enabled)
{
	Options_SetCheckbox(hwnd, IDC_STATUSBAR_ENABLED, enabled);
}

static INT_PTR OptionsUI_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM param)
{
	WCHAR szBuffer[512] = {0};
	Plugin_LoadString(IDS_OPTIONS_UI, szBuffer, ARRAYSIZE(szBuffer));
	SetWindowText(hwnd, szBuffer);

	HWND hControl = GetDlgItem(hwnd, IDC_TOOLBAR_LOCATION);
	if (NULL != hControl)
	{
		INT iItem;
		
		Plugin_LoadString(IDS_TOOLBAR_TOPDOCK, szBuffer, ARRAYSIZE(szBuffer));
		iItem = (INT)(INT_PTR)SendMessage(hControl, CB_ADDSTRING, 0, (LPARAM)szBuffer);
		if (CB_ERR != iItem)  
			SendMessage(hControl, CB_SETITEMDATA, (WPARAM)iItem, (LPARAM)TOOLBAR_TOPDOCK);

		Plugin_LoadString(IDS_TOOLBAR_BOTTOMDOCK, szBuffer, ARRAYSIZE(szBuffer));
		iItem = (INT)(INT_PTR)SendMessage(hControl, CB_ADDSTRING, 0, (LPARAM)szBuffer);
		if (CB_ERR != iItem)  
			SendMessage(hControl, CB_SETITEMDATA, (WPARAM)iItem, (LPARAM)TOOLBAR_BOTTOMDOCK);
	}


	HWND hParent = GetParent(hwnd);

	obj_ombrowser *browserManager;
	if (FALSE == SendMessage(hParent, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
		browserManager = NULL;

	ifc_omtoolbarconfig *toolbarConfig;

	if (NULL != browserManager && 
		SUCCEEDED(browserManager->GetConfig(&IFC_OmToolbarConfig, (void**)&toolbarConfig)))
	{
		OptionsUI_UpdateToolbarLocation(hwnd, toolbarConfig->GetBottomDockEnabled());
		OptionsUI_UpdateToolbarAutoHide(hwnd, toolbarConfig->GetAutoHideEnabled());
		OptionsUI_UpdateToolbarTabStop(hwnd, toolbarConfig->GetTabStopEnabled());
		OptionsUI_UpdateToolbarForceAddress(hwnd, toolbarConfig->GetForceAddressbarEnabled());
		OptionsUI_UpdateToolbarFancyAddress(hwnd, toolbarConfig->GetFancyAddressbarEnabled());

		toolbarConfig->Release();
	}
	else
	{
		OptionsUI_UpdateToolbarLocation(hwnd, E_FAIL);
		OptionsUI_UpdateToolbarAutoHide(hwnd, E_FAIL);
		OptionsUI_UpdateToolbarTabStop(hwnd, E_FAIL);
	}

	ifc_omstatusbarconfig *statusbarConfig;

	if (NULL != browserManager && 
		SUCCEEDED(browserManager->GetConfig(&IFC_OmStatusbarConfig, (void**)&statusbarConfig)))
	{
		OptionsUI_UpdateStatusbarEnabled(hwnd, statusbarConfig->GetEnabled());
		statusbarConfig->Release();
	}
	else
	{
		OptionsUI_UpdateStatusbarEnabled(hwnd, E_FAIL);
	}
	
	if (NULL != browserManager)
		browserManager->Release();
	
	return 0;
}

static void OptionsUI_OnDestroy(HWND hwnd)
{
}

static void OptionsUI_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
}

static void OptionsUI_OnToolbarLocation(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_TOOLBAR_LOCATION);
	if (NULL == hControl) return;

	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return;

	INT iIndex = (INT)SendMessage(hControl, CB_GETCURSEL, 0, 0L);
	if (CB_ERR == iIndex) return;

	INT dockType = (INT)SendMessage(hControl, CB_GETITEMDATA, iIndex, 0L);
	if (TOOLBAR_TOPDOCK != dockType && TOOLBAR_BOTTOMDOCK != dockType)
		return;

	obj_ombrowser *browserManager;
	if (FALSE != SendMessage(hParent, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
	{
		ifc_omtoolbarconfig *toolbarConfig;
		if (SUCCEEDED(browserManager->GetConfig(&IFC_OmToolbarConfig, (void**)&toolbarConfig)))
		{
			toolbarConfig->EnableBottomDock(TOOLBAR_BOTTOMDOCK == dockType);
			toolbarConfig->Release();
		}
		browserManager->Release();
	}
}

static void OptionsUI_OnToolbarAutoHide(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_TOOLBAR_AUTOHIDE);
	if (NULL == hControl) return;

	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return;

	BOOL checked = (BST_CHECKED == (UINT)SendMessage(hControl, BM_GETCHECK, 0, 0L));

	obj_ombrowser *browserManager;
	if (FALSE != SendMessage(hParent, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
	{
		ifc_omtoolbarconfig *toolbarConfig;
		if (SUCCEEDED(browserManager->GetConfig(&IFC_OmToolbarConfig, (void**)&toolbarConfig)))
		{
			toolbarConfig->EnableAutoHide(checked);
			toolbarConfig->Release();
		}
		browserManager->Release();
	}
}

static void OptionsUI_OnToolbarTabstop(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_TOOLBAR_TABSTOP);
	if (NULL == hControl) return;

	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return;

	BOOL checked = (BST_CHECKED == (UINT)SendMessage(hControl, BM_GETCHECK, 0, 0L));

	obj_ombrowser *browserManager;
	if (FALSE != SendMessage(hParent, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
	{
		ifc_omtoolbarconfig *toolbarConfig;
		if (SUCCEEDED(browserManager->GetConfig(&IFC_OmToolbarConfig, (void**)&toolbarConfig)))
		{
			toolbarConfig->EnableTabStop(checked);
			toolbarConfig->Release();
		}
		browserManager->Release();
	}
}

static void OptionsUI_OnToolbarForceAddress(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_TOOLBAR_FORCEADDRESS);
	if (NULL == hControl) return;

	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return;

	BOOL checked = (BST_CHECKED == (UINT)SendMessage(hControl, BM_GETCHECK, 0, 0L));

	obj_ombrowser *browserManager;
	if (FALSE != SendMessage(hParent, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
	{
		ifc_omtoolbarconfig *toolbarConfig;
		if (SUCCEEDED(browserManager->GetConfig(&IFC_OmToolbarConfig, (void**)&toolbarConfig)))
		{
			toolbarConfig->EnableForceAddressbar(checked);
			toolbarConfig->Release();
		}
		browserManager->Release();
	}
}

static void OptionsUI_OnToolbarFancyAddress(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_TOOLBAR_FANCYADDRESS);
	if (NULL == hControl) return;

	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return;

	BOOL checked = (BST_CHECKED == (UINT)SendMessage(hControl, BM_GETCHECK, 0, 0L));

	obj_ombrowser *browserManager;
	if (FALSE != SendMessage(hParent, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
	{
		ifc_omtoolbarconfig *toolbarConfig;
		if (SUCCEEDED(browserManager->GetConfig(&IFC_OmToolbarConfig, (void**)&toolbarConfig)))
		{
			toolbarConfig->EnableFancyAddressbar(checked);
			toolbarConfig->Release();
		}
		browserManager->Release();
	}
}

static void OptionsUI_OnStatusbarEnabled(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_STATUSBAR_ENABLED);
	if (NULL == hControl) return;

	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return;

	BOOL checked = (BST_CHECKED == (UINT)SendMessage(hControl, BM_GETCHECK, 0, 0L));

	obj_ombrowser *browserManager;
	if (FALSE != SendMessage(hParent, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
	{
		ifc_omstatusbarconfig *statusbarConfig;
		if (SUCCEEDED(browserManager->GetConfig(&IFC_OmStatusbarConfig, (void**)&statusbarConfig)))
		{
			statusbarConfig->EnableStatusbar(checked);
			statusbarConfig->Release();
		}
		browserManager->Release();
	}
}

static void OptionsUI_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND hControl)
{
	switch(commandId)
	{
		case IDC_TOOLBAR_LOCATION:	
			if (CBN_SELCHANGE  == eventId) 
				OptionsUI_OnToolbarLocation(hwnd); 
			break;
		case IDC_TOOLBAR_AUTOHIDE:	
			if (BN_CLICKED == eventId) 
				OptionsUI_OnToolbarAutoHide(hwnd); 
			break;
		case IDC_TOOLBAR_TABSTOP:	
			if (BN_CLICKED == eventId) 
				OptionsUI_OnToolbarTabstop(hwnd); 
			break;
		case IDC_TOOLBAR_FORCEADDRESS:	
			if (BN_CLICKED == eventId) 
				OptionsUI_OnToolbarForceAddress(hwnd); 
			break;
		case IDC_TOOLBAR_FANCYADDRESS:	
			if (BN_CLICKED == eventId) 
				OptionsUI_OnToolbarFancyAddress(hwnd); 
			break;
		case IDC_STATUSBAR_ENABLED:
			if (BN_CLICKED == eventId) 
				OptionsUI_OnStatusbarEnabled(hwnd); 
			break;
	}
}

static void OptionsUI_OnConfigChanged(HWND hwnd, BOMCONFIGCHANGED *configData)
{
	if (NULL == configData || NULL == configData->configUid)
		return;

	if (IsEqualIID(IFC_OmToolbarConfig, *configData->configUid))
	{
		switch(configData->valueId)
		{
			case CFGID_TOOLBAR_BOTTOMDOCK:			OptionsUI_UpdateToolbarLocation(hwnd, BOOL2HRESULT(configData->value)); break;
			case CFGID_TOOLBAR_AUTOHIDE:			OptionsUI_UpdateToolbarAutoHide(hwnd, BOOL2HRESULT(configData->value)); break;
			case CFGID_TOOLBAR_TABSTOP:				OptionsUI_UpdateToolbarTabStop(hwnd, BOOL2HRESULT(configData->value)); break;
			case CFGID_TOOLBAR_FORCEADDRESS:		OptionsUI_UpdateToolbarForceAddress(hwnd, BOOL2HRESULT(configData->value)); break;
			case CFGID_TOOLBAR_FANCYADDRESS:		OptionsUI_UpdateToolbarFancyAddress(hwnd, BOOL2HRESULT(configData->value)); break;
		}
	}
	else if (IsEqualIID(IFC_OmStatusbarConfig, *configData->configUid))
	{
		switch(configData->valueId)
		{
			case CFGID_STATUSBAR_ENABLED:	OptionsUI_UpdateStatusbarEnabled(hwnd, BOOL2HRESULT(configData->value)); break; 
		}
	}
}


static INT_PTR CALLBACK OptionsUI_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return OptionsUI_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:				OptionsUI_OnDestroy(hwnd); return 0;
		case WM_WINDOWPOSCHANGED:	OptionsUI_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_COMMAND:				OptionsUI_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;

		case BOM_CONFIGCHANGED:		OptionsUI_OnConfigChanged(hwnd, (BOMCONFIGCHANGED*)lParam); return TRUE;
	}
	return 0;
}