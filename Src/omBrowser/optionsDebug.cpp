#include "main.h"
#include "./options.h"
#include "./resource.h"

#include "./obj_ombrowser.h"
#include "./ifc_omconfig.h"
#include "./ifc_omdebugconfig.h"

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <strsafe.h>


#define INVERTHRESULT(__result)\
	((S_OK == (__result)) ? S_FALSE : ((S_FALSE == (__result)) ? S_OK : (__result)))

#define BOOL2HRESULT(__result)\
	((FALSE != (__result)) ? S_OK : S_FALSE)

static INT_PTR CALLBACK OptionsDebug_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


HWND CALLBACK OptionsDebug_CreatePage(HWND hParent, UINT style)
{
	if (0 == (BOSTYLE_SHOWDEBUG & style)) 
		return NULL;

	return Plugin_CreateDialogParam(MAKEINTRESOURCE(IDD_OPTIONS_DEBUG), hParent, OptionsDebug_DialogProc, 0L);
}

static void OptionsDebug_UpdateFilterMenu(HWND hwnd, HRESULT enable)
{
	Options_SetCheckbox(hwnd, IDC_FILTERMENU, INVERTHRESULT(enable));
}

static void OptionsDebug_UpdateShowError(HWND hwnd, HRESULT enable)
{
	Options_SetCheckbox(hwnd, IDC_SHOWERROR, enable);
}

static void OptionsDebug_UpdateShowDebugger(HWND hwnd, HRESULT enable)
{
	Options_SetCheckbox(hwnd, IDC_SHOWDEBUGGER, enable);
}


static INT_PTR OptionsDebug_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM param)
{	
	WCHAR szBuffer[512] = {0};
	Plugin_LoadString(IDS_OPTIONS_DEBUG, szBuffer, ARRAYSIZE(szBuffer));
	SetWindowText(hwnd, szBuffer);

	HWND hParent = GetParent(hwnd);

	obj_ombrowser *browserManager;
	if (FALSE == SendMessage(hParent, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
		browserManager = NULL;

	ifc_omdebugconfig *debugConfig;

	if (NULL != browserManager && 
		SUCCEEDED(browserManager->GetConfig(&IFC_OmDebugConfig, (void**)&debugConfig)))
	{
		OptionsDebug_UpdateFilterMenu(hwnd, debugConfig->GetMenuFilterEnabled());
		OptionsDebug_UpdateShowError(hwnd, debugConfig->GetScriptErrorEnabled());
		OptionsDebug_UpdateShowDebugger(hwnd, debugConfig->GetScriptDebuggerEnabled());
		debugConfig->Release();
	}
	else
	{
		OptionsDebug_UpdateFilterMenu(hwnd, E_FAIL);
		OptionsDebug_UpdateShowError(hwnd, E_FAIL);
		OptionsDebug_UpdateShowDebugger(hwnd, E_FAIL);
	}

	return 0;
}

static void OptionsDebug_OnDestroy(HWND hwnd)
{
}

static void OptionsDebug_OnFilterMenu(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_FILTERMENU);
	if (NULL == hControl) return;

	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return;

	BOOL checked = (BST_CHECKED == (UINT)SendMessage(hControl, BM_GETCHECK, 0, 0L));

	obj_ombrowser *browserManager;
	if (FALSE != SendMessage(hParent, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
	{
		ifc_omdebugconfig *debugConfig;
		if (SUCCEEDED(browserManager->GetConfig(&IFC_OmDebugConfig, (void**)&debugConfig)))
		{
			debugConfig->EnableMenuFilter(!checked);
			debugConfig->Release();
		}
		browserManager->Release();
	}
}

static void OptionsDebug_OnShowError(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_SHOWERROR);
	if (NULL == hControl) return;

	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return;

	BOOL checked = (BST_CHECKED == (UINT)SendMessage(hControl, BM_GETCHECK, 0, 0L));

	obj_ombrowser *browserManager;
	if (FALSE != SendMessage(hParent, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
	{
		ifc_omdebugconfig *debugConfig;
		if (SUCCEEDED(browserManager->GetConfig(&IFC_OmDebugConfig, (void**)&debugConfig)))
		{
			debugConfig->EnableScriptError(checked);
			debugConfig->Release();
		}
		browserManager->Release();
	}
}

static void OptionsDebug_OnShowDebugger(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_SHOWDEBUGGER);
	if (NULL == hControl) return;

	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return;

	BOOL checked = (BST_CHECKED == (UINT)SendMessage(hControl, BM_GETCHECK, 0, 0L));

	obj_ombrowser *browserManager;
	if (FALSE != SendMessage(hParent, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
	{
		ifc_omdebugconfig *debugConfig;
		if (SUCCEEDED(browserManager->GetConfig(&IFC_OmDebugConfig, (void**)&debugConfig)))
		{
			debugConfig->EnableScriptDebugger(checked);
			debugConfig->Release();
		}
		browserManager->Release();
	}
}

static void OptionsDebug_OnConfigChanged(HWND hwnd, BOMCONFIGCHANGED *configData)
{
	if (NULL == configData || NULL == configData->configUid ||
		FALSE == IsEqualIID(IFC_OmDebugConfig, *configData->configUid))
	{
		return;
	}

	
	switch(configData->valueId)
	{
		case CFGID_DEBUG_FILTERMENU:			OptionsDebug_UpdateFilterMenu(hwnd, BOOL2HRESULT(configData->value)); break;
		case CFGID_DEBUG_SCRIPTERROR:		OptionsDebug_UpdateShowError(hwnd, BOOL2HRESULT(configData->value)); break;
		case CFGID_DEBUG_SCRIPTDEBUGGER:	OptionsDebug_UpdateShowDebugger(hwnd, BOOL2HRESULT(configData->value)); break;
	}
	
}
static void OptionsDebug_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND hControl)
{
	switch(commandId)
	{		
		case IDC_FILTERMENU:	
			if (BN_CLICKED == eventId) 
				OptionsDebug_OnFilterMenu(hwnd); 
			break;
		case IDC_SHOWERROR:	
			if (BN_CLICKED == eventId) 
				OptionsDebug_OnShowError(hwnd); 
			break;
		case IDC_SHOWDEBUGGER:
			if (BN_CLICKED == eventId) 
				OptionsDebug_OnShowDebugger(hwnd); 
			break;
	}

}



static INT_PTR CALLBACK OptionsDebug_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return OptionsDebug_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:				OptionsDebug_OnDestroy(hwnd); return 0;
		case WM_COMMAND:				OptionsDebug_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
		
		case BOM_CONFIGCHANGED:		OptionsDebug_OnConfigChanged(hwnd, (BOMCONFIGCHANGED*)lParam); return TRUE;
	}
	return 0;
}