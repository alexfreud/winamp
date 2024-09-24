#include "main.h"
#include "./options.h"
#include "./resource.h"

#include "./obj_ombrowser.h"
#include "./ifc_omconfig.h"
#include "./ifc_ombrowserclass.h"
#include "./ieversion.h"

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>


static INT_PTR CALLBACK OptionsInfo_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND CALLBACK OptionsInfo_CreatePage(HWND hParent, UINT style)
{
	return Plugin_CreateDialogParam(MAKEINTRESOURCE(IDD_OPTIONS_INFO), hParent, OptionsInfo_DialogProc, 0L);
}

static void OptionsInfo_SetClassName(HWND hwnd, obj_ombrowser *browserManager)
{
	HWND hControl = GetDlgItem(hwnd, IDC_CLASS);
	if (NULL == hControl) return;
	
	WCHAR szBuffer[512] = {0};

	BOOL valueInvalid = TRUE;
	ifc_ombrowserclass *browserClass;
	if (NULL != browserManager && SUCCEEDED(browserManager->GetClass(&browserClass)))
	{
		if (SUCCEEDED(browserClass->GetName(szBuffer, ARRAYSIZE(szBuffer))))
			valueInvalid = FALSE;
		browserClass->Release();
	}
	
	if (FALSE != valueInvalid)
		Plugin_LoadString(IDS_UNKNOWN, szBuffer, ARRAYSIZE(szBuffer));

	SetWindowText(hControl, szBuffer);
}


static void OptionsInfo_SetConfigPath(HWND hwnd, obj_ombrowser *browserManager)
{
	HWND hControl = GetDlgItem(hwnd, IDC_CONFIGPATH);
	if (NULL == hControl) return;
	
	WCHAR szBuffer[512] = {0};

	BOOL valueInvalid = TRUE;
	ifc_omconfig *browserConfig;
	if (NULL != browserManager && SUCCEEDED(browserManager->GetConfig(NULL, (void**)&browserConfig)))
	{
		if (SUCCEEDED(browserConfig->GetPath(szBuffer, ARRAYSIZE(szBuffer))))
			valueInvalid = FALSE;
		browserConfig->Release();
	}
	
	if (FALSE != valueInvalid)
		Plugin_LoadString(IDS_UNKNOWN, szBuffer, ARRAYSIZE(szBuffer));

	SetWindowText(hControl, szBuffer);
}


static void OptionsInfo_SetIEVersion(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_MSIEVERSION);
	if (NULL == hControl) return;

	WCHAR szBuffer[128] = {0};
	if (FAILED(MSIE_GetVersionString(szBuffer, ARRAYSIZE(szBuffer))))
		Plugin_LoadString(IDS_UNKNOWN, szBuffer, ARRAYSIZE(szBuffer));

	SetWindowText(hControl, szBuffer);
}

static void OptionsInfo_SetVersion(HWND hwnd, obj_ombrowser *browserManager)
{
	HWND hControl = GetDlgItem(hwnd, IDC_VERSION);
	if (NULL == hControl) return;
	
	WCHAR szBuffer[32] = {0};
	INT major, minor;
	if (NULL == browserManager || 
		FAILED(browserManager->GetVersion(&major, &minor)) ||
		FAILED(StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), L"%d.%d", major, minor)))
	{
		Plugin_LoadString(IDS_UNKNOWN, szBuffer, ARRAYSIZE(szBuffer));
	}
	
	SetWindowText(hControl, szBuffer);
}

static void OptionsInfo_SetClientId(HWND hwnd, obj_ombrowser *browserManager)
{
	HWND hControl = GetDlgItem(hwnd, IDC_CLIENTID);
	if (NULL == hControl) return;
	
	WCHAR szBuffer[256] = {0};
	if (NULL == browserManager || FAILED(browserManager->GetClientId(szBuffer, ARRAYSIZE(szBuffer))))
		Plugin_LoadString(IDS_UNKNOWN, szBuffer, ARRAYSIZE(szBuffer));
	
	SetWindowText(hControl, szBuffer);
}

static void OptionsInfo_SetSessionId(HWND hwnd, obj_ombrowser *browserManager)
{
	HWND hControl = GetDlgItem(hwnd, IDC_SESSIONID);
	if (NULL == hControl) return;
	
	WCHAR szBuffer[256] = {0};
	if (NULL == browserManager || FAILED(browserManager->GetSessionId(szBuffer, ARRAYSIZE(szBuffer))))
		Plugin_LoadString(IDS_UNKNOWN, szBuffer, ARRAYSIZE(szBuffer));
	
	SetWindowText(hControl, szBuffer);
}

static void OptionsInfo_SetTitleFont(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_TITLE);
	if (NULL == hControl) return;

	LOGFONT lfTitle;
	HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
	INT fontHeight;
	fontHeight = (NULL != hFont && 0 != GetObject(hFont, sizeof(lfTitle), &lfTitle)) ? lfTitle.lfHeight : -11;
	fontHeight += (fontHeight > 0) ? 2 : -2;
		
	ZeroMemory(&lfTitle, sizeof(lfTitle));
	lfTitle.lfHeight = fontHeight;
	lfTitle.lfWeight = FW_NORMAL;
	lfTitle.lfItalic = FALSE;
	lfTitle.lfUnderline = FALSE;
	lfTitle.lfStrikeOut = FALSE;
	lfTitle.lfCharSet = DEFAULT_CHARSET;
	lfTitle.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lfTitle.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lfTitle.lfQuality = CLEARTYPE_QUALITY;
	lfTitle.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	StringCchCopy(lfTitle.lfFaceName, ARRAYSIZE(lfTitle.lfFaceName), L"Arial");
	
	HFONT titleFont = CreateFontIndirect(&lfTitle);
	if (NULL == titleFont) return;

	SendMessage(hControl, WM_SETFONT, (WPARAM)titleFont, 0);
}

static INT_PTR OptionsInfo_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM param)
{	
	WCHAR szBuffer[512] = {0};
	Plugin_LoadString(IDS_OPTIONS_INFO, szBuffer, ARRAYSIZE(szBuffer));
	SetWindowText(hwnd, szBuffer);

	HWND hParent = GetParent(hwnd);
	
	obj_ombrowser *browserManager;
	if (FALSE == SendMessage(hParent, BOM_GETBROWSER, 0, (LPARAM)&browserManager))
		browserManager = NULL;
	
	OptionsInfo_SetVersion(hwnd, browserManager);
	OptionsInfo_SetClassName(hwnd, browserManager);
	OptionsInfo_SetClientId(hwnd, browserManager);
	OptionsInfo_SetSessionId(hwnd, browserManager);
	OptionsInfo_SetConfigPath(hwnd, browserManager);
	OptionsInfo_SetIEVersion(hwnd);

	if (NULL != browserManager)	
		browserManager->Release();

	OptionsInfo_SetTitleFont(hwnd);

	return 0;
}

static void OptionsInfo_OnDestroy(HWND hwnd)
{
}

static INT_PTR OptionsInfo_OnColorStatic(HWND hwnd, HDC hdc, HWND hControl)
{
	UINT controlId = (UINT)GetWindowLongPtr(hControl, GWLP_ID);
	if (((UINT)-1) == controlId)
		SetTextColor(hdc, GetSysColor(COLOR_HOTLIGHT));
	return 0;
}

static INT_PTR CALLBACK OptionsInfo_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return OptionsInfo_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:			OptionsInfo_OnDestroy(hwnd); return 0;
		case WM_CTLCOLORSTATIC:		return OptionsInfo_OnColorStatic(hwnd, (HDC)wParam, (HWND)lParam);
	}
	return 0;
}