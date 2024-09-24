#include "main.h"
#include "./api__ml_online.h"
#include "./resource.h"

#include <windows.h>
#include <strsafe.h>

typedef struct __MESSAGEBOX
{
	HWND	hParent;
	LPCWSTR pszText;
	LPCWSTR pszCaption;
	UINT	uType;
	LPCWSTR	pszCheck;
	INT		*checked;
} MESSAGEBOX;

static INT_PTR CALLBACK OmMessageBox_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT OmMessageBox(HWND hParent, LPCWSTR pszText, LPCWSTR pszCaption, UINT uType, LPCWSTR pszCheck, INT *checked)
{
	MESSAGEBOX instance;
	instance.hParent = hParent;
	instance.pszText = pszText;
	instance.pszCaption = pszCaption;
	instance.uType = uType;
	instance.pszCheck = pszCheck;
	instance.checked = checked;

	return (INT)WASABI_API_DIALOGBOXPARAMW(IDD_MESSAGEBOX, hParent, OmMessageBox_DialogProc, (LPARAM)&instance);
}

static void OmMessgageBox_CenterWindow(HWND hwnd, HWND hCenter)
{
	if (NULL == hwnd || NULL == hCenter)
		return;

	RECT centerRect, windowRect;
	if (!GetWindowRect(hwnd, &windowRect) || !GetWindowRect(hCenter, &centerRect))
		return;
	windowRect.left = centerRect.left + ((centerRect.right - centerRect.left) - (windowRect.right - windowRect.left))/2;
	windowRect.top = centerRect.top + ((centerRect.bottom - centerRect.top) - (windowRect.bottom - windowRect.top))/2;

	SetWindowPos(hwnd, NULL, windowRect.left, windowRect.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

static BOOL OmMessageBox_GetTextBox(HWND hwnd, LPCWSTR pszText, INT cchText, SIZE *sizeText)
{
	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == hdc) return FALSE;
	
	HFONT font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
	HFONT fontOrig = (HFONT)SelectObject(hdc, font);

	if (cchText < 0)
		cchText = lstrlen(pszText);

	BOOL resultOk = GetTextExtentPoint32(hdc, pszText, cchText, sizeText);

	SelectObject(hdc, fontOrig);
	ReleaseDC(hwnd, hdc);

	return resultOk;
}
static HICON OmMessageBox_GetIcon(UINT flags)
{
	LPCWSTR iconName = NULL;
	switch(0x000000F0 & flags)
	{
		case MB_ICONHAND:		iconName = IDI_HAND; break;
		case MB_ICONQUESTION:	iconName = IDI_QUESTION; break;
		case MB_ICONEXCLAMATION:iconName = IDI_EXCLAMATION; break;
		case MB_ICONASTERISK:	iconName = IDI_ASTERISK; break;
	}
	return (NULL != iconName) ? LoadIcon(NULL, iconName) : NULL;
}

static BOOL OmMessageBox_GetIconSize(UINT flags, SIZE *iconSize)
{
	if (NULL == iconSize) return FALSE;

	iconSize->cx = 0;
	iconSize->cy = 0;

	HICON hIcon = OmMessageBox_GetIcon(flags);
	if (NULL == hIcon) 
		return TRUE;
	
	ICONINFO iconInfo;
	
	if (!GetIconInfo(hIcon, &iconInfo) || FALSE == iconInfo.fIcon) 
		return FALSE;
	
	BITMAP bm;
	if (NULL != iconInfo.hbmColor)
	{
		if (FALSE == GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bm))
			return FALSE;

		iconSize->cx = bm.bmWidth;
		iconSize->cy = bm.bmHeight;
	}
	else if (NULL != iconInfo.hbmMask)
	{
		if (FALSE == GetObject(iconInfo.hbmMask, sizeof(BITMAP), &bm))
			return FALSE;

		iconSize->cx = bm.bmWidth;
		iconSize->cy = bm.bmHeight/2;
	}

	return TRUE;
}

static BOOL OmMessageBox_GetButtonSize(HWND hwnd, SIZE *buttonSize)
{
	if (NULL == buttonSize)
		return FALSE;

	if (FALSE != SendMessage(hwnd, (0x1600 + 0x0001) /*BCM_GETIDEALSIZE*/, 0, (LPARAM)buttonSize))
		return TRUE;

	return FALSE;
}
static INT_PTR OmMessageBox_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM lParam)
{
	MESSAGEBOX *pmb = (MESSAGEBOX*)lParam;
	SetWindowText(hwnd, pmb->pszCaption);

	SIZE textSize;
	SIZE maxSize;
	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);

	HMONITOR hMonitor = MonitorFromWindow(pmb->hParent, MONITOR_DEFAULTTONEAREST);
	if (NULL != hMonitor &&
		GetMonitorInfo(hMonitor, &mi))
	{
		RECT rcFrame;
		SetRectEmpty(&rcFrame);
		AdjustWindowRectEx(&rcFrame, GetWindowStyle(hwnd), FALSE, GetWindowStyleEx(hwnd));
		maxSize.cx = mi.rcWork.right - mi.rcWork.left - (rcFrame.right - rcFrame.left) - 8*2;
		maxSize.cy = mi.rcWork.bottom - mi.rcWork.top - (rcFrame.bottom - rcFrame.top) - 8*2;
	}
	else
	{
		maxSize.cx = 1200;
		maxSize.cy = 800;
	}

	HWND hText = GetDlgItem(hwnd, IDC_TEXT);
	if (NULL != hText)
	{
		if (!OmMessageBox_GetTextBox(hText, pmb->pszText, -1, &textSize))
			ZeroMemory(&textSize, sizeof(SIZE));

		SetWindowPos(hText, NULL, 0, 0, textSize.cx, textSize.cy, SWP_NOACTIVATE | SWP_NOZORDER); 
	}
	else
		ZeroMemory(&textSize, sizeof(SIZE));



	OmMessgageBox_CenterWindow(hwnd, pmb->hParent);
	SendMessage(hwnd, DM_REPOSITION, 0, 0L);
	return FALSE;
}

static void OmMessageBox_OnDestroy(HWND hwnd)
{

}

static void OmMessageBox_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND hControl)
{
	switch(commandId)
	{
		case IDOK:
		case IDCANCEL:
			EndDialog(hwnd, commandId);
			break;
	}
}
static INT_PTR CALLBACK OmMessageBox_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:		return OmMessageBox_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		OmMessageBox_OnDestroy(hwnd); break;
		case WM_COMMAND:		OmMessageBox_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;		
	}
	return 0;
}
