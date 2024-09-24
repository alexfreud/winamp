#include "main.h"
#include "./resource.h"
#include "./api.h"
#include "./language.h"
#include "./jsapi2_svc_apicreator.h"
#include "./commandLink.h"


#include <strsafe.h>


#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#ifdef WIN64
	#define MSGRESULT(__hwnd, __result) { SetWindowLongPtrW((__hwnd), DWLP_MSGRESULT, ((LONGX86)(LONG_PTR)(__result))); return TRUE; }
#else
	#define MSGRESULT(__hwnd, __result) { SetWindowLongPtrW((__hwnd), DWL_MSGRESULT, ((LONGX86)(LONG_PTR)(__result))); return TRUE; }
#endif

#ifdef __cplusplus
  #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) ::SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#else
 #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#endif // __cplusplus

#define SENDWAIPC(__ipcMsgId, __param) SENDMSG(hMainWindow, WM_WA_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))


#ifndef OIC_WARNING
#define OIC_WARNING          32515
#endif

#define SECDLG_PROP		L"SecurityPromptProp"

#ifndef IDC_HELPLINK
#define IDC_HELPLINK	10000
#endif 

typedef struct __SECDLGCREATEPARAM
{
	HWND hCenter;
	LPCWSTR pszCaption;
	LPCWSTR pszTitle;
	LPCWSTR pszMessage;
	UINT flags;
} SECDLGCREATEPARAM;

typedef struct  __SECDLG
{
	HFONT	titleFont;
	SIZE	minSize;
	SIZE	maxSize;
} SECDLG;

#define GetDialog(__hwnd) ((SECDLG*)GetPropW((__hwnd), SECDLG_PROP))

static INT_PTR CALLBACK SecurityPrompt_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


static HWND SecurityPrompt_GetPlayerWindow()
{
	return (NULL != g_dialog_box_parent) ? g_dialog_box_parent : hMainWindow;
}

INT_PTR JSAPI2_SecurityPrompt(HWND hParent, LPCWSTR pszCaption, LPCWSTR pszTitle, LPCWSTR pszMessage, UINT flags)
{	
	SECDLGCREATEPARAM param;
	ZeroMemory(&param, sizeof(SECDLGCREATEPARAM));
	param.hCenter = hParent;
	param.flags = flags;
	param.pszCaption = pszCaption;
	param.pszTitle = pszTitle;
	param.pszMessage  = pszMessage;
	

	if (NULL == hParent)
		hParent = SecurityPrompt_GetPlayerWindow();

	return LPDialogBoxParamW(IDD_JSAPI2_AUTHORIZATION2, hParent, SecurityPrompt_DialogProc, (LPARAM)&param);
}

static BOOL SecurityPrompt_GetCenterRect(HWND hCenter, RECT *centerRect, BOOL fUseMonitor)
{
	if (NULL == centerRect) 
		return FALSE;

	HWND hDesktop = GetDesktopWindow();
	HWND hTest = hCenter;
	while(NULL != hTest)
	{
		DWORD windowStyle = GetWindowLongPtrW(hTest, GWL_STYLE);
		if (WS_VISIBLE != ((WS_VISIBLE | WS_MINIMIZE) & windowStyle))
		{
			hTest = NULL;
			hCenter = NULL;
		}
		else
		{
			hTest = GetParent(hTest);
		}
	}
	
	if (FALSE != fUseMonitor || NULL == hCenter || !GetWindowRect(hCenter, centerRect))
	{ 
		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);
		if (NULL == hCenter)
		{
			hCenter = SecurityPrompt_GetPlayerWindow();
			if (NULL != hCenter)
			{
				DWORD windowStyle = GetWindowLongPtrW(hCenter, GWL_STYLE);
				if (WS_VISIBLE != ((WS_VISIBLE | WS_MINIMIZE) & windowStyle))
					hCenter = NULL;
			}
		}
		
		HMONITOR hMonitor = MonitorFromWindow(hCenter, 
						(NULL != hCenter) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY);
		

		if (NULL != hMonitor && GetMonitorInfo(hMonitor, &mi))
		{
			CopyRect(centerRect, &mi.rcWork);
		}
		else
		{
			if (NULL == hDesktop || !GetWindowRect(hDesktop, centerRect))
				return FALSE;
		}
	}
	
	return TRUE;
}

static void SecurityPrompt_CenterDialog(HWND hwnd, HWND hCenter)
{
	if (NULL == hwnd)
		return;

	RECT centerRect, windowRect;
	if (!GetWindowRect(hwnd, &windowRect) || 
		!SecurityPrompt_GetCenterRect(hCenter, &centerRect, FALSE))
	{
		return;
	}
	
	windowRect.left = centerRect.left + ((centerRect.right - centerRect.left) - (windowRect.right - windowRect.left))/2;
	windowRect.top = centerRect.top + ((centerRect.bottom - centerRect.top) - (windowRect.bottom - windowRect.top))/2;

	SetWindowPos(hwnd, NULL, windowRect.left, windowRect.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

static void SecurityPrompt_CalculateMinMax(HWND hwnd, HWND hCenter)
{
	SECDLG *psd = GetDialog(hwnd);
	if (NULL == hwnd || NULL == psd) return;
	
	RECT centerRect, windowRect;
	if (GetWindowRect(hwnd, &windowRect))
	{
		psd->minSize.cx = windowRect.right - windowRect.left;
		psd->minSize.cy = windowRect.bottom - windowRect.top;
	}
	if (SecurityPrompt_GetCenterRect(hCenter, &centerRect, TRUE))
	{
		InflateRect(&centerRect, -16, -16);
		psd->maxSize.cx = centerRect.right - centerRect.left;
		psd->maxSize.cy = centerRect.bottom - centerRect.top;

		if (psd->maxSize.cx > 360) 
			psd->maxSize.cx = 360;
	}
}

static void SecurityPrompt_LoadIcon(HWND hwnd, HINSTANCE hInstance, LPCWSTR pszIcon)
{
	HWND hControl = GetDlgItem(hwnd, IDC_MESSAGEICON);
	if (NULL == hControl) return;
	
	SendMessageW(hControl, WM_SETREDRAW, FALSE, 0L);

	HICON hIcon = (HICON)LoadImageW(hInstance, pszIcon, IMAGE_ICON, 	0, 0, LR_DEFAULTCOLOR | LR_SHARED | LR_DEFAULTSIZE);
	
	HICON hPrevious = (HICON)SendMessageW(hControl, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
	if (NULL != hPrevious)
		DeleteObject(hPrevious);

	if (NULL != hIcon)
	{
		hPrevious = (HICON)SendMessageW(hControl, STM_GETIMAGE, IMAGE_ICON, 0L);
		if (hPrevious != hIcon)
			DeleteObject(hIcon);
	}

	SendMessageW(hControl, WM_SETREDRAW, TRUE, 0L);

	if (0 != ShowWindow(hControl,  (NULL != hIcon) ? SW_SHOWNA : SW_HIDE))
		InvalidateRect(hControl, NULL, TRUE);

}
static BOOL SecurityPrompt_GetWindowTextSize(HWND hwnd, SIZE *pSize, BOOL fMultiline, LONG lineWidth)
{
	if (NULL == pSize) return FALSE;
	
	INT cchLen = GetWindowTextLengthW(hwnd);
	if (0 == cchLen) 
	{
		ZeroMemory(pSize, sizeof(SIZE));
		return TRUE;
	}
	cchLen++;
	LPWSTR pszText = NULL;
	WCHAR szBuffer[1024] = {0};
	if (cchLen > ARRAYSIZE(szBuffer))
	{
		pszText = (LPWSTR)calloc(cchLen, sizeof(WCHAR));
		if (NULL == pszText) return FALSE;
	}
	else
	{
		pszText = szBuffer;
		cchLen = ARRAYSIZE(szBuffer);
	}

	BOOL resultOk = FALSE;
	cchLen = GetWindowTextW(hwnd, pszText, cchLen);
	if (0 != cchLen)
	{
		HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL != hdc)
		{
			HFONT hf = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0L);
			HFONT originalFont = (HFONT)SelectObject(hdc, hf);

			if (FALSE == fMultiline)
			{
				resultOk = GetTextExtentPoint32W(hdc, pszText, cchLen, pSize);
			}
			else
			{
				RECT textRect;
				SetRect(&textRect, 0, 0, lineWidth, 0);
				INT h = DrawTextW(hdc, pszText, cchLen, &textRect, DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP | DT_WORDBREAK);
				resultOk = (0 != h);
				if (resultOk)
				{
					pSize->cx = textRect.right - textRect.left;
					pSize->cy = textRect.bottom - textRect.top;
				}
					
			}
				
			SelectObject(hdc, originalFont);
			ReleaseDC(hwnd, hdc);
		}
	}
	

	if (pszText != szBuffer)
		free(pszText);

	return resultOk;
}

static LONG SecurityPrompt_CalculateFooterHeight(HWND hwnd, LONG marginCY)
{
	RECT rect;
	HWND hControl;
	
	
	LONG height = 0;

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_BUTTON_ALLOW)) &&
		0 != (WS_VISIBLE & GetWindowLongPtrW(hControl, GWL_STYLE)) &&
		GetWindowRect(hControl, &rect))
	{
		height += (rect.bottom - rect.top);
	}

	
	if (0 == height &&
		NULL != (hControl = GetDlgItem(hwnd, IDC_BUTTON_DENY)) &&
		0 != (WS_VISIBLE & GetWindowLongPtrW(hControl, GWL_STYLE)) &&
		GetWindowRect(hControl, &rect))
	{
		height += (rect.bottom - rect.top);
	}

	INT szCtrl[] = {IDC_APPLYTOALL, IDC_SEPARATOR, IDC_HELPTEXT, };
	for (INT i = 0; i < ARRAYSIZE(szCtrl); i++)
	{
		if (NULL != (hControl = GetDlgItem(hwnd, szCtrl[i])) &&
			0 != (WS_VISIBLE & GetWindowLongPtrW(hControl, GWL_STYLE)) &&
			GetWindowRect(hControl, &rect))
		{
			LONG h = (rect.bottom - rect.top);
			if (h > 0) 
			{
				if (height > 0) height += marginCY;
				height += h;
			}
		}
	}

	if (0 != height)
		height += 2*marginCY;

	return height;
}

static void SecurityPrompt_UpdateLayout(HWND hwnd, BOOL fRedraw)
{
	SECDLG *psd = GetDialog(hwnd);
	if (NULL == hwnd || NULL == psd) return;

	HWND hControl;
	RECT rect;
	LONG marginCX, marginCY;
	SIZE clientSize, iconSize, titleSize, messageSize;
	ZeroMemory(&clientSize, sizeof(SIZE));

	SetRect(&rect, 6, 6, 6, 6);
	if (MapDialogRect(hwnd, &rect))
	{
		marginCX = rect.left;
		marginCY = rect.top;
	}
	else
	{
		marginCX = 12;
		marginCY = 12;
	}

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_MESSAGEICON)) && 
		0 != (WS_VISIBLE & GetWindowLongPtrW(hControl, GWL_STYLE)) &&
		GetWindowRect(hControl, &rect))
	{
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);
		iconSize.cx = rect.right - rect.left;
		iconSize.cy = rect.bottom - rect.top;
	}
	else
	{
		ZeroMemory(&iconSize, sizeof(SIZE));
	}

	if (NULL == (hControl = GetDlgItem(hwnd, IDC_TITLE)) ||
		0 == (WS_VISIBLE & GetWindowLongPtrW(hControl, GWL_STYLE)) ||
		FALSE == SecurityPrompt_GetWindowTextSize(hControl, &titleSize, FALSE, 0))
	{
		ZeroMemory(&titleSize, sizeof(SIZE));
	}

	if (NULL == (hControl = GetDlgItem(hwnd, IDC_MESSAGE)) ||
		0 == (WS_VISIBLE & GetWindowLongPtrW(hControl, GWL_STYLE)) ||
		FALSE == SecurityPrompt_GetWindowTextSize(hControl, &messageSize, TRUE, psd->maxSize.cx - (iconSize.cx + marginCX * 3)))
	{
		ZeroMemory(&messageSize, sizeof(SIZE));
	}
			
	clientSize.cx = iconSize.cx;
	if (messageSize.cx > 0)
	{
		clientSize.cx += messageSize.cx;
		if (iconSize.cx > 0) clientSize.cx += marginCX;
	}
	if (titleSize.cx > clientSize.cx)
		titleSize.cx = clientSize.cx;
	clientSize.cx += 2*marginCX;
	
	clientSize.cy = titleSize.cy;
	
	LONG h1 = messageSize.cy;
	if (h1 > 0) h1 += 2 * marginCY;

	LONG h2 = iconSize.cy;
	if (h2 > 0) h2 += marginCY;

	clientSize.cy += (h1 > h2) ? h1 : h2;
	if (clientSize.cy > 0) 
	{
		if (clientSize.cy != titleSize.cy && 0 != titleSize.cy)
			clientSize.cy += marginCY;
	}
	clientSize.cy += SecurityPrompt_CalculateFooterHeight(hwnd, marginCY);


	DWORD windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
	DWORD windowExStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);

	SetRect(&rect, 0, 0, clientSize.cx, clientSize.cy);
	if (AdjustWindowRectEx(&rect, windowStyle, FALSE, windowExStyle))
	{
		clientSize.cx = rect.right - rect.left;
		clientSize.cy = rect.bottom - rect.top;
	}
	if (clientSize.cx < psd->minSize.cx) clientSize.cx = psd->minSize.cx;
	if (clientSize.cy < psd->minSize.cy) clientSize.cy = psd->minSize.cy;

	
	SetWindowPos(hwnd, NULL, 0, 0, clientSize.cx, clientSize.cy, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

	RECT clientRect;
	if (!GetClientRect(hwnd, &clientRect))
		SetRectEmpty(&clientRect);
	else
		InflateRect(&clientRect, -marginCX, -marginCY);
		

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_TITLE)))
	{
		SetWindowPos(hControl, NULL, clientRect.left, clientRect.top, clientRect.right - clientRect.left, titleSize.cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_MESSAGEICON)))
	{
		LONG y = clientRect.top;
		if (titleSize.cy > 0) y += (titleSize.cy + marginCY);
		SetWindowPos(hControl, NULL, clientRect.left, y, iconSize.cx, iconSize.cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_MESSAGE)))
	{
		LONG x = clientRect.left;
		if (iconSize.cx > 0) x += (iconSize.cx + marginCX);
		LONG y = clientRect.top;
		if (titleSize.cy > 0) y += (titleSize.cy + marginCY);
		SetWindowPos(hControl, NULL, x, y, clientRect.right - x, messageSize.cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}


	HWND hHelp;
	hHelp = GetDlgItem(hwnd, IDC_HELPTEXT);
	if (NULL == hHelp || !GetWindowRect(hHelp, &rect))
		SetRectEmpty(&rect);
	else
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);

	SIZE linkSize;
	LONG bottomLine = clientRect.bottom;
	if (NULL != (hControl = GetDlgItem(hwnd, IDC_HELPLINK)) && 
		CommandLink_GetIdealSize(hControl, &linkSize))
	{
		RECT margins;

		if (!CommandLink_GetMargins(hControl, &margins))
			SetRectEmpty(&margins);
		
		if (linkSize.cy > 0)
		{
			bottomLine -= (linkSize.cy - margins.bottom);
		}

		SetWindowPos(hControl, NULL, clientRect.left - margins.left, bottomLine, linkSize.cx, linkSize.cy, SWP_NOACTIVATE | SWP_NOZORDER);
		if (NULL != hHelp)
		{
			LONG x = clientRect.left + linkSize.cx - margins.right;
			LONG cy = rect.bottom - rect.top;
			SetWindowPos(hHelp, NULL, x, clientRect.bottom - cy, clientRect.right - x, cy, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_SEPARATOR)) && 
		0 != (WS_VISIBLE & GetWindowLongPtrW(hControl, GWL_STYLE)) &&
		GetWindowRect(hControl, &rect))
	{
		bottomLine -= (marginCY + (rect.bottom - rect.top));
		SetWindowPos(hControl, NULL, 0, bottomLine, clientRect.right - clientRect.left + 2*marginCX + 2, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_APPLYTOALL)) && 
		0 != (WS_VISIBLE & GetWindowLongPtrW(hControl, GWL_STYLE)) &&
		GetWindowRect(hControl, &rect))
	{
		bottomLine -= (marginCY + (rect.bottom - rect.top));
		SetWindowPos(hControl, NULL, clientRect.left, bottomLine, clientRect.right - clientRect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	LONG buttonRight = clientRect.right;
	if (NULL != (hControl = GetDlgItem(hwnd, IDC_BUTTON_DENY)) && 
		0 != (WS_VISIBLE & GetWindowLongPtrW(hControl, GWL_STYLE)) &&
		GetWindowRect(hControl, &rect))
	{
		
		SetWindowPos(hControl, NULL, buttonRight - (rect.right - rect.left), bottomLine - (marginCY + (rect.bottom - rect.top)), 
						rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOACTIVATE);
		if (rect.right > rect.left)
		{
			buttonRight -= ((rect.right - rect.left) + marginCX);
		}
	}

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_BUTTON_ALLOW)) && 
		0 != (WS_VISIBLE & GetWindowLongPtrW(hControl, GWL_STYLE)) &&
		GetWindowRect(hControl, &rect))
	{		
		SetWindowPos(hControl, NULL, buttonRight - (rect.right - rect.left), bottomLine - (marginCY + (rect.bottom - rect.top)), 
						rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

static HFONT SecurityPrompt_GetTitleFont(HWND hwnd)
{
	SECDLG *psd = GetDialog(hwnd);
	if (NULL == psd) 
		return (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0L);
		
	if (NULL != psd->titleFont)
		return psd->titleFont;

			
	LOGFONTW lf;
	ZeroMemory(&lf, sizeof(LOGFONTW));

	HFONT hf = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0L);

	if (NULL != hf)
		GetObjectW(hf, sizeof(LOGFONTW), &lf);
		
	if (L'\0' == lf.lfFaceName[0])
	{
		if (!SystemParametersInfoW(SPI_GETICONTITLELOGFONT, sizeof(LOGFONTW), &lf, 0) ||
			FAILED(StringCchCopyW(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), L"MS Shell Dlg 2")))
		{
			lf.lfFaceName[0] = L'\0';
		}
	}
	
	if (L'\0' != lf.lfFaceName[0])
	{
		lf.lfWeight = FW_SEMIBOLD;
		psd->titleFont = CreateFontIndirectW(&lf);
	}

	return psd->titleFont;
}

static BOOL SecurityPrompt_ShowHelp(HWND hwnd)
{
	INT result = (INT)(INT_PTR)ShellExecuteW(hwnd, L"open", 
								L"https://help.winamp.com/hc/articles/8112753225364-Online-Services-Security", 
								NULL, NULL, SW_SHOWNORMAL);
	return (result > 32);
}

static INT_PTR SecurityPrompt_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM param)
{
	SECDLGCREATEPARAM *createParam = (SECDLGCREATEPARAM*)param;
	if (NULL == createParam) return 0;

	SECDLG *psd = (SECDLG*)calloc(1, sizeof(SECDLG));
	if (NULL != psd)
	{
		if (!SetPropW(hwnd, SECDLG_PROP, psd))
		{
			free(psd);
			psd = NULL;
		}
	}
	
	HMENU hMenu = GetSystemMenu(hwnd, FALSE);
	if (NULL != hMenu)
	{
		EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND|MF_DISABLED);
		DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
		DeleteMenu(hMenu, SC_SIZE, MF_BYCOMMAND);
	}


	SecurityPrompt_LoadIcon(hwnd, NULL, MAKEINTRESOURCEW(OIC_WARNING));

	WCHAR szBuffer[4096] = {0};
	LPCWSTR pszText;
	HWND hControl;
	
	SecurityPrompt_CalculateMinMax(hwnd, createParam->hCenter);
		
	if (NULL != (pszText = createParam->pszCaption) && IS_INTRESOURCE(pszText))
		pszText = getStringW((INT)(INT_PTR)pszText, szBuffer, ARRAYSIZE(szBuffer));

	if (NULL != pszText && L'\0' != *pszText)
		SetWindowTextW(hwnd, pszText);

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_TITLE)))
	{
		if (NULL != (pszText = createParam->pszTitle) && IS_INTRESOURCE(pszText))
			pszText = getStringW((INT)(INT_PTR)pszText, szBuffer, ARRAYSIZE(szBuffer));
		
		HFONT hf = SecurityPrompt_GetTitleFont(hwnd);
		if (NULL != hf) SendMessageW(hControl, WM_SETFONT, (WPARAM)hf, 0L);

		SetWindowTextW(hControl, pszText);
		ShowWindow(hControl, (NULL != pszText && L'\0' != *pszText) ? SW_SHOWNA : SW_HIDE);
	}
    
	if (NULL != (hControl = GetDlgItem(hwnd, IDC_MESSAGE)))
	{
		if (NULL != (pszText = createParam->pszMessage) && IS_INTRESOURCE(pszText))
			pszText = getStringW((INT)(INT_PTR)pszText, szBuffer, ARRAYSIZE(szBuffer));
		SetWindowTextW(hControl, pszText);
		ShowWindow(hControl, (NULL != pszText && L'\0' != *pszText) ? SW_SHOWNA : SW_HIDE);
	}

	getStringW(IDS_CLICKHERE, szBuffer, ARRAYSIZE(szBuffer));

	HWND hLink = CreateWindowExW(WS_EX_NOPARENTNOTIFY, NWC_COMMANDLINKW, szBuffer, 
					WS_VISIBLE | WS_CHILD | WS_TABSTOP | /*CLS_HOTTRACK | */CLS_DEFAULTCOLORS | CLS_ALWAYSUNDERLINE, 
					0, 0, 0, 0, hwnd, (HMENU)IDC_HELPLINK, hMainInstance, NULL);
	
	if (NULL != hLink)
	{
		if (NULL != (hControl = GetDlgItem(hwnd, IDC_APPLYTOALL)))
			SetWindowPos(hLink, hControl, 0, 0, 0, 0, SWP_NOACTIVATE |SWP_NOSIZE | SWP_NOMOVE);

		SendMessageW(hLink, WM_SETFONT, (WPARAM)SendMessageW(hwnd, WM_GETFONT, 0, 0L), 0L);
	}
	
	CheckDlgButton(hwnd, IDC_APPLYTOALL, 
					(0 != (JSAPI2::svc_apicreator::AUTHORIZATION_FLAG_ALWAYS_FOR_SERVICE &createParam->flags)) ? 
					BST_CHECKED : BST_UNCHECKED);
	
	SecurityPrompt_UpdateLayout(hwnd, FALSE);
	
	SecurityPrompt_CenterDialog(hwnd, createParam->hCenter);
	SendMessageW(hwnd, DM_REPOSITION, 0, 0L);

	DWORD dialogThread = GetWindowThreadProcessId(hwnd, NULL);
	DWORD parentThread = GetWindowThreadProcessId(GetParent(hwnd), NULL);
	if (dialogThread != parentThread)
	{
		AttachThreadInput(parentThread, dialogThread, TRUE);
		SetForegroundWindow(hwnd);
	}

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_BUTTON_DENY)) &&
		WS_VISIBLE == ((WS_VISIBLE | WS_DISABLED) & GetWindowLongPtrW(hControl, GWL_STYLE)) &&
		PostMessageW(hwnd, WM_NEXTDLGCTL, (WPARAM)hControl, TRUE))
	{
		return TRUE;
	}

	return FALSE;
}

static void SecurityPrompt_OnDestroy(HWND hwnd)
{
	SECDLG *psd = GetDialog(hwnd);
	RemovePropW(hwnd, SECDLG_PROP);

	DWORD dialogThread = GetWindowThreadProcessId(hwnd, NULL);
	DWORD parentThread = GetWindowThreadProcessId(GetParent(hwnd), NULL);
	if (dialogThread != parentThread)
	{
		AttachThreadInput(parentThread, dialogThread, FALSE);
	}

	if (NULL == psd) return;
	
	if (NULL != psd->titleFont)
		DeleteObject(psd->titleFont);
	
	free(psd);
}

static void SecurityPrompt_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND hControl)
{
	switch(commandId)
	{
		case IDC_BUTTON_ALLOW:
		case IDC_BUTTON_DENY:
			if(BN_CLICKED == eventId)
			{
				INT_PTR result = (IDC_BUTTON_ALLOW == commandId) ? 
					JSAPI2::svc_apicreator::AUTHORIZATION_ALLOW :
					JSAPI2::svc_apicreator::AUTHORIZATION_DENY;
		
				result |= (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_APPLYTOALL)) ?
						JSAPI2::svc_apicreator::AUTHORIZATION_FLAG_ALWAYS_FOR_SERVICE :
						JSAPI2::svc_apicreator::AUTHORIZATION_FLAG_ALWAYS;
			
				EndDialog(hwnd, result);
			}
			break;
	}
}

static LRESULT SecurityPrompt_OnNotify(HWND hwnd, INT controlId, NMHDR *pnmh)
{
	switch(controlId)
	{	
		case IDC_HELPLINK:
			if (NM_CLICK == pnmh->code)
				 SecurityPrompt_ShowHelp(hwnd);
			return TRUE;
	}
	return 0;
}
static INT_PTR CALLBACK SecurityPrompt_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG: return SecurityPrompt_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		SecurityPrompt_OnDestroy(hwnd); break;
		case WM_COMMAND:		SecurityPrompt_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case WM_NOTIFY:		MSGRESULT(hwnd, SecurityPrompt_OnNotify(hwnd, (INT)wParam, (NMHDR*)lParam));

	}
	
	
	return 0;
}