#include "./setupDetails.h"
#include "../common.h"
#include "../resource.h"
#include "../api__ml_online.h"

#include "../../winamp/commandLink.h"
#include "./setupGroup.h"

#include <commctrl.h>
#include <strsafe.h>

#ifndef IDC_HELPLINK
#define IDC_HELPLINK	10000
#endif 

struct GROUPDETAILSCREATEPARAM
{
	GROUPDETAILSCREATEPARAM() : group(NULL), name(NULL) {}

	SetupGroup *group;
	LPCWSTR name;
};

struct GROUPDETAILS
{
	GROUPDETAILS() : group(NULL), name(NULL), fontTitle(NULL) {}

	SetupGroup *group;
	LPWSTR name;
	HFONT fontTitle;
};

#define GetDetails(__hwnd) ((GROUPDETAILS*)GetPropW((__hwnd), MAKEINTATOM(DETAILS_PROP)))

static INT_PTR WINAPI GroupDetails_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND SetupDetails_CreateGroupView(HWND hParent, LPCWSTR pszName, SetupGroup *group)
{
	GROUPDETAILSCREATEPARAM param;
	param.group = group;
	param.name = pszName;
	return WASABI_API_CREATEDIALOGPARAMW(IDD_SETUP_GROUPDETAILS, hParent, GroupDetails_DialogProc, (LPARAM)&param);
}

static void GroupDetails_SetTitle(HWND hwnd, SetupGroup *group)
{
	HWND hTitle = GetDlgItem(hwnd, IDC_TITLE);
	if (NULL == hTitle) return;

	WCHAR szBuffer[128] = {0};
	if (NULL == group || 
		FAILED(group->GetLongName(szBuffer, ARRAYSIZE(szBuffer))))
	{
		szBuffer[0] = L'\0';
	}
	
	GROUPDETAILS *details = GetDetails(hwnd);
	if (NULL != details && NULL == details->fontTitle)
	{
		HFONT dialogFont =  (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
		LOGFONT lf;
		if (0 != GetObject(dialogFont, sizeof(LOGFONT), &lf))
		{
			StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), L"Arial Bold");
			lf.lfWidth = 0;
			lf.lfWeight = FW_DONTCARE;
			lf.lfHeight += (lf.lfHeight < 0) ? -2 : +2;
			lf.lfQuality = 5/*ANTIALIASED_QUALITY*/;
			details->fontTitle = CreateFontIndirect(&lf);
		}

		if (NULL != details->fontTitle)
		{
			SendMessage(hTitle, WM_SETFONT, (WPARAM)details->fontTitle, 0L);
		}
	}

	SetWindowText(hTitle, szBuffer);
	InvalidateRect(hTitle, NULL, TRUE);
}

static void GroupDetails_SetDescription(HWND hwnd, SetupGroup *group)
{
	HWND hDescription = GetDlgItem(hwnd, IDC_DESCRIPTION);
	if (NULL == hDescription) return;

	WCHAR szBuffer[4096] = {0};
	if (NULL == group || 
		FAILED(group->GetDescription(szBuffer, ARRAYSIZE(szBuffer))))
	{
		szBuffer[0] = L'\0';
	}
	
	SetupDetails_SetDescription(hDescription, szBuffer);
}

static BOOL GroupDetails_ShowHelp(HWND hwnd)
{
	INT result = (INT)(INT_PTR)ShellExecuteW(hwnd, L"open", 
								L"https://help.winamp.com/hc/articles/8112753225364-Online-Services-Security", 
								NULL, NULL, SW_SHOWNORMAL);
	return (result > 32);
}

static void GroupDetails_UpdateLayout(HWND hwnd, BOOL fRedraw)
{
	RECT clientRect, rect;
	if(FALSE == GetClientRect(hwnd, &clientRect))
		return;


	UINT commonFlags = SWP_NOACTIVATE | SWP_NOZORDER;
	if (FALSE != fRedraw) commonFlags |= SWP_NOREDRAW;

	LONG bottomLine = clientRect.bottom;

	HWND hControl;
	SIZE linkSize;
	RECT linkMargins;

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_HELPLINK)) && 
		CommandLink_GetIdealSize(hControl, &linkSize))
	{		
		if (!CommandLink_GetMargins(hControl, &linkMargins))
			SetRectEmpty(&linkMargins);
		
		if (linkSize.cy > 0)
			bottomLine -= linkSize.cy;

		SetWindowPos(hControl, NULL, clientRect.left + 4, bottomLine, linkSize.cx, linkSize.cy, commonFlags);
	}
	else
	{
		ZeroMemory(&linkSize, sizeof(linkSize));
		SetRectEmpty(&linkMargins);
	}

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_HELPTEXT)))
	{
		LONG x = clientRect.left + 4 + linkSize.cx/* - linkMargins.right*/;
		LONG y = 0;

		HDC hdc =  GetDCEx(hControl, NULL, DCX_CACHE | DCX_WINDOW | DCX_NORESETATTRS);
		if (NULL != hdc)
		{
			HFONT font = (HFONT)SendMessage(hControl, WM_GETFONT, 0, 0L);
			HFONT originalFont = (HFONT)SelectObject(hdc, font);

			TEXTMETRICW tm;
			if (GetTextMetricsW(hdc, &tm))
				y = tm.tmHeight;

			SelectObject(hdc, originalFont);
			ReleaseDC(hControl, hdc);
		}
		
		SetWindowPos(hControl, NULL, x, clientRect.bottom - linkMargins.bottom - y, clientRect.right - x, y, commonFlags);
	}
	
	if (NULL != (hControl = GetDlgItem(hwnd, IDC_DESCRIPTION)) &&
		FALSE != GetWindowRect(hControl, &rect))
	{
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);
		SetWindowPos(hControl, NULL, rect.left, rect.right, rect.right - rect.left, bottomLine - rect.top, 
			commonFlags | SWP_NOMOVE);
	}
}

static INT_PTR GroupDetails_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM lParam)
{
	GROUPDETAILSCREATEPARAM *param = (GROUPDETAILSCREATEPARAM*)lParam;

	GROUPDETAILS *details = (GROUPDETAILS*)calloc(1, sizeof(GROUPDETAILS));
	if (NULL == details) return FALSE;

	if (!SetProp(hwnd, MAKEINTATOM(DETAILS_PROP), details))
		return FALSE;

	if (NULL != param)
	{
		if (NULL != param->group)
		{
			details->group = param->group;
			details->group->AddRef();
		}

		details->name = Plugin_CopyString(param->name);
	}

	HINSTANCE winampInstance = (NULL != WASABI_API_APP) ? WASABI_API_APP->main_gethInstance() : NULL;
	if (NULL != winampInstance)
	{
		WCHAR szBuffer[256] = {0};
		WASABI_API_LNGSTRINGW_BUF(IDS_CLICKHERE, szBuffer, ARRAYSIZE(szBuffer));
		HWND hLink = CreateWindowExW(WS_EX_NOPARENTNOTIFY | WS_EX_TRANSPARENT, NWC_COMMANDLINKW, szBuffer, 
						WS_VISIBLE | WS_CHILD | WS_TABSTOP | CLS_ALWAYSUNDERLINE | CLS_DEFAULTCOLORS /* | CLS_HOTTRACK */, 
						0, 0, 0, 0, hwnd, (HMENU)IDC_HELPLINK, winampInstance, NULL);

		if (NULL != hLink)
		{
			SendMessageW(hLink, WM_SETFONT, (WPARAM)SendMessageW(hwnd, WM_GETFONT, 0, 0L), 0L);
		}
	}

	GroupDetails_UpdateLayout(hwnd, FALSE);

	if (NULL != param)
	{
		GroupDetails_SetTitle(hwnd, param->group);
		GroupDetails_SetDescription(hwnd, param->group);
	}
		
	return FALSE;
}

static void GroupDetails_OnDestroy(HWND hwnd)
{
	GROUPDETAILS *details = GetDetails(hwnd);
	RemoveProp(hwnd, MAKEINTATOM(DETAILS_PROP));
	
	if (NULL != details)
	{
		if (NULL != details->group)
			details->group->Release();
		if (NULL != details->fontTitle)
			DeleteObject(details->fontTitle);

		Plugin_FreeString(details->name);
	}
}


static INT_PTR GroupDetails_OnDialogColor(HWND hwnd, HDC hdc, HWND hControl)
{
	HWND hParent = GetAncestor(hwnd, GA_PARENT);
	if (NULL != hParent && hParent != hwnd)
		return (INT_PTR)SendMessage(hParent, WM_CTLCOLORDLG, (WPARAM)hdc, (LPARAM)hControl);
	return 0;
}


static INT_PTR GroupDetails_OnStaticColor(HWND hwnd, HDC hdc, HWND hControl)
{
	HWND hParent = GetAncestor(hwnd, GA_PARENT);
	if (NULL != hParent && hParent != hwnd)
		return (INT_PTR)SendMessage(hParent, WM_CTLCOLORSTATIC, (WPARAM)hdc, (LPARAM)hControl);
	return 0;
}

static LRESULT GroupDetails_OnNotify(HWND hwnd, INT controlId, NMHDR *pnmh)
{
	switch(controlId)
	{	
		case IDC_HELPLINK:
			if (NM_CLICK == pnmh->code)
				 GroupDetails_ShowHelp(hwnd);
			return TRUE;
	}
	return 0;
}

static BOOL GroupDetails_OnGetUniqueName(HWND hwnd, LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) 
		return FALSE;

	GROUPDETAILS *details = GetDetails(hwnd);
	return SUCCEEDED(StringCchCopy(pszBuffer, cchBufferMax, 
		(NULL != details && NULL != details->name) ? details->name : L""));
}
static INT_PTR WINAPI GroupDetails_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:		return GroupDetails_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:			GroupDetails_OnDestroy(hwnd); break;
		case WM_CTLCOLORDLG:		return GroupDetails_OnDialogColor(hwnd, (HDC)wParam, (HWND)lParam);
		case WM_CTLCOLORSTATIC: return GroupDetails_OnStaticColor(hwnd, (HDC)wParam, (HWND)lParam);
		case WM_NOTIFY:			MSGRESULT(hwnd, GroupDetails_OnNotify(hwnd, (INT)wParam, (NMHDR*)lParam));

		case NSDM_GETUNIQUENAME: MSGRESULT(hwnd, GroupDetails_OnGetUniqueName(hwnd, (LPWSTR)lParam, (UINT)wParam));
	}
	return 0;
}