#include "./loginPopup.h"
#include "../api.h"
#include "../../nu/Vectors.h"
#include "./loginNotifier.h"
#include "./common.h"

typedef Vector<HWND> WindowList;

typedef struct __THREADPOPUPDATA
{
	HHOOK hHook;
	WindowList windowList;
} THREADPOPUPDATA;

static size_t threadStorage = TLS_OUT_OF_INDEXES;

typedef struct __LOGINPOPUPCREATEPARAM
{
	LoginPopup::Creator fnCreator;
	LPARAM  lParam;
} LOGINPOPUPCREATEPARAM;

#define IDC_NOTIFIER	10001

//#define COLOR_TITLE			COLOR_3DLIGHT
//#define COLOR_TITLETEXT		COLOR_WINDOWTEXT
#define COLOR_CLIENT		COLOR_3DFACE
#define COLOR_CLIENTTEXT	COLOR_WINDOWTEXT

LoginPopup::LoginPopup(HWND hwnd, UINT popupType, LPCWSTR pszTitle)
	: alertType(-1), alertMessage(NULL)
{
	buttonHeight = buttonSpace = 0;
	this->hwnd = hwnd;

	if (NULL != pszTitle)
		SetTitle(popupType, pszTitle);
}

LoginPopup::~LoginPopup()
{
	RegisterPopup(hwnd, FALSE);

	if (FALSE == IS_INTRESOURCE(alertMessage))
		LoginBox_FreeString(alertMessage);
}

HWND LoginPopup::CreatePopup(LPCWSTR pszTemplate, HWND hParent, LPARAM param, Creator fnCreator)
{
	if (NULL == hParent || NULL == pszTemplate || NULL == fnCreator)
		return NULL;

	LOGINPOPUPCREATEPARAM createParam;
	createParam.fnCreator = fnCreator;
	createParam.lParam = param;
	
	return WASABI_API_CREATEDIALOGPARAMW((INT)(INT_PTR)pszTemplate, hParent, LoginPopup_DialogProc, (LPARAM)&createParam);
}

BOOL LoginPopup::RegisterPopup(HWND hwnd, BOOL fRegister)
{
	if (NULL == hwnd || GetWindowThreadProcessId(hwnd, NULL) != GetCurrentThreadId())
		return FALSE;

	THREADPOPUPDATA *data = NULL;
	if (TLS_OUT_OF_INDEXES == threadStorage)
	{
		if (NULL == WASABI_API_APP)
			return FALSE;
		
		threadStorage = WASABI_API_APP->AllocateThreadStorage();
		if (TLS_OUT_OF_INDEXES == threadStorage)
			return FALSE;
	}
	else
	{
		data = (THREADPOPUPDATA*)WASABI_API_APP->GetThreadStorage(threadStorage);
	}

	if (NULL == data)
	{
		data = new THREADPOPUPDATA();
		if (NULL == data) return FALSE;
		data->hHook = SetWindowsHookEx(WH_MSGFILTER, LoginPopup_MessageFilter, NULL, GetCurrentThreadId());
		if (NULL == data->hHook) 
		{
			delete data;
			return FALSE;
		}
		WASABI_API_APP->SetThreadStorage(threadStorage, data);
	}

	size_t index = data->windowList.size();
	while(index--)
	{
		if (hwnd == data->windowList[index])
		{
			if (FALSE == fRegister)
			{
				data->windowList.eraseAt(index);
				if (0 == data->windowList.size())
				{
					if (NULL != data->hHook)
						UnhookWindowsHookEx(data->hHook);
					WASABI_API_APP->SetThreadStorage(threadStorage, NULL);
					delete data;
				}
			}
			return TRUE;
		}
	}
	
	if (FALSE != fRegister)
	{
		data->windowList.push_back(hwnd);
		return TRUE;
	}

	return FALSE;
}

BOOL LoginPopup::EnumeratePopups(HWND hHost, Enumerator callback, LPARAM param)
{
	if (NULL == callback || 
		NULL == hHost || GetWindowThreadProcessId(hHost, NULL) != GetCurrentThreadId())
		return FALSE;

	THREADPOPUPDATA *data = (TLS_OUT_OF_INDEXES != threadStorage && NULL != WASABI_API_APP) ? 
							(THREADPOPUPDATA *)WASABI_API_APP->GetThreadStorage(threadStorage) : NULL;
	if (NULL == data)
		return FALSE;

	size_t index = data->windowList.size();
	while(index--)
	{
		HWND hPopup = data->windowList[index];
		if (IsChild(hHost, hPopup) && FALSE == callback(hPopup, param))
		return FALSE;
	}

	return TRUE;
}

BOOL LoginPopup::AnyPopup(HWND hHost)
{
	if (NULL == hHost || GetWindowThreadProcessId(hHost, NULL) != GetCurrentThreadId())
		return FALSE;

	THREADPOPUPDATA *data = (TLS_OUT_OF_INDEXES != threadStorage && NULL != WASABI_API_APP) ? 
							(THREADPOPUPDATA *)WASABI_API_APP->GetThreadStorage(threadStorage) : NULL;
	if (NULL == data)
		return FALSE;

	size_t index = data->windowList.size();
	while(index--)
	{
		if (IsChild(hHost, data->windowList[index]))
			return TRUE;
	}

	return FALSE;
}

void LoginPopup::UpdateLayout(BOOL fRedraw)
{
	RECT rect;
	GetClientRect(hwnd, &rect);

	HWND hNotifier;
	hNotifier = GetDlgItem(hwnd, IDC_NOTIFIER);
	if (NULL != hNotifier)
	{
		INT height = LoginNotifier_GetIdealHeight(hNotifier);
		SetWindowPos(hNotifier, NULL, rect.left, rect.top, rect.right - rect.left, height, 
				SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

void LoginPopup::Paint(HDC hdc, const RECT *prcPaint, BOOL fErase)
{
	if (FALSE != fErase)
	{		
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);
		
		COLORREF rgbOrig, rgbPart;
		rgbOrig = GetBkColor(hdc);

		if (buttonHeight > 0)
		{
			RECT buttonRect;
			SetRect(&buttonRect, 
				clientRect.left, clientRect.bottom - (2* clientMargins.bottom + buttonHeight), 
				clientRect.right, clientRect.bottom);
				
			clientRect.bottom = buttonRect.top;

			buttonRect.top++;
			rgbPart = GetSysColor(COLOR_3DFACE);
			SetBkColor(hdc, rgbPart);
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &buttonRect, NULL, 0, NULL);

			buttonRect.bottom = buttonRect.top;
			buttonRect.top--;

			rgbPart = GetSysColor(COLOR_3DLIGHT);
			SetBkColor(hdc, rgbPart);
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &buttonRect, NULL, 0, NULL);
		}

		rgbPart = GetSysColor(COLOR_CLIENT);
		SetBkColor(hdc, rgbPart);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &clientRect, NULL, 0, NULL);

		SetBkColor(hdc, rgbOrig);
	}
}

void LoginPopup::EndDialog(INT_PTR code)
{
	DestroyWindow(hwnd);
}

void LoginPopup::UpdateMargins()
{
	SetRect(&clientMargins, 8, 6, 8, 6);
	MapDialogRect(hwnd, &clientMargins);

	SetRect(&infoMargins, 6, 8, 6, 8);
	MapDialogRect(hwnd, &infoMargins);

	RECT rect;
	SetRect(&rect, 4, 15, 0, 0);
	MapDialogRect(hwnd, &rect);
	buttonHeight = rect.top;
	buttonSpace = rect.left;
}

void LoginPopup::SetTitle(UINT type, LPCWSTR title)
{
	popupType = type;
	
	
	if (NULL == title || FALSE == IS_INTRESOURCE(title))
		SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)title);
	else
	{
		WCHAR szBuffer[256] = {0};
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)title, szBuffer, ARRAYSIZE(szBuffer));
		SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)szBuffer);
	}
}

void LoginPopup::UpdateTitle(BOOL playBeep)
{
	HWND hNotifier;
	hNotifier = GetDlgItem(hwnd, IDC_NOTIFIER);
	if (NULL == hNotifier) return;

	WCHAR szBuffer[512] = {0};
	LPCWSTR text;
	UINT type;

	if (-1 != alertType && NULL != alertMessage)
	{
		type = alertType;
		text = (IS_INTRESOURCE(alertMessage)) ?
			WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)alertMessage, szBuffer, ARRAYSIZE(szBuffer)) :
			alertMessage;
	}
	else
	{		
		type = popupType;
		SendMessage(hwnd, WM_GETTEXT, (WPARAM)ARRAYSIZE(szBuffer), (LPARAM)szBuffer);
		text = szBuffer;
	}

	LoginNotifier_Notify(hNotifier, type, text);

	UINT windowStyle = GetWindowStyle(hNotifier);
	if (0 == (WS_VISIBLE & windowStyle))
	{
		ShowWindow(hNotifier, SW_SHOWNA);
	}

	if (FALSE != playBeep)
		LoginNotifier_PlayBeep(hNotifier);
}

void LoginPopup::SetAlert(UINT type, LPCWSTR message)
{
	alertType = type;
	
	if (FALSE == IS_INTRESOURCE(alertMessage))
		LoginBox_FreeString(alertMessage);

	if (NULL == message)
	{
		alertMessage = NULL;
		return;
	}

	if (IS_INTRESOURCE(message))
		alertMessage = (LPWSTR)message;
	else
		alertMessage = LoginBox_CopyString(message);
}

void LoginPopup::RemoveAlert()
{
	SetAlert(-1, NULL);
}

LRESULT LoginPopup::SendNotification(UINT code, NMHDR *pnmh)
{
	if (NULL == pnmh) return 0L;

	HWND hParent = GetAncestor(hwnd, GA_PARENT);
	if(NULL == hParent) return 0L;

	pnmh->code = code;
	pnmh->hwndFrom = hwnd;
	pnmh->idFrom = (UINT_PTR)GetWindowLongPtr(hwnd, GWLP_ID);
	
	return SendMessage(hParent, WM_NOTIFY, (WPARAM)pnmh->idFrom, (LPARAM)pnmh);
}


BOOL LoginPopup::GetInfoRect(RECT *rect)
{
	if (NULL == rect)
		return FALSE;

	LONG notifierHeight = 0;
	HWND hNotifier = GetDlgItem(hwnd, IDC_NOTIFIER);
	if (NULL != hNotifier && 0 != (WS_VISIBLE & GetWindowStyle(hNotifier)) &&
		FALSE != GetWindowRect(hNotifier, rect))
	{
		notifierHeight = rect->bottom - rect->top;
		if (notifierHeight < 0) notifierHeight = 0;
	}

	if (FALSE == GetClientRect(hwnd, rect))
		return FALSE;

	rect->left += (clientMargins.left + infoMargins.left);
	rect->right -= (clientMargins.right + infoMargins.right);


	if (0 != notifierHeight)
		rect->top += (notifierHeight + infoMargins.top);
	else
		rect->top += (clientMargins.top + infoMargins.top);

	if (buttonHeight > 0)
		rect->bottom -= (2 * clientMargins.bottom + buttonHeight + infoMargins.bottom);
	else
		rect->bottom -= (clientMargins.bottom + infoMargins.bottom);

	if (rect->right < rect->left) rect->right = rect->left;
	if (rect->bottom < rect->top) rect->bottom = rect->top;

	return TRUE;
}

BOOL LoginPopup::CalculateWindowRect(LONG infoWidth, LONG infoHeight, const INT *buttonList, UINT buttonCount, BOOL includeTitle, RECT *rect)
{
	if (NULL == rect)
		return FALSE;
	
	if (infoWidth < 110) infoWidth = 110;
	if (infoHeight < 32) infoHeight = 32;

	LONG minWidth = 0;

	LONG notifierHeight = 0;
	
	HWND hNotifier = GetDlgItem(hwnd, IDC_NOTIFIER);
	if (NULL != hNotifier && 0 != (WS_VISIBLE & GetWindowStyle(hNotifier)) &&
		FALSE != GetWindowRect(hNotifier, rect))
	{
		notifierHeight = rect->bottom - rect->top;
		if (notifierHeight < 0) notifierHeight = 0;
		
		if (FALSE != includeTitle)
		{
			SIZE size;
			if (FALSE != LoginNotifier_GetIdealSize(hNotifier, &size))
				minWidth = size.cx + clientMargins.right;
		}
	}

	if (NULL != buttonList && buttonCount > 0 && 
		((HDWP)TRUE) == LayoutButtons(NULL, buttonList, buttonCount, FALSE, rect))
	{
		LONG buttonWidth = (rect->right - rect->left) + clientMargins.left + clientMargins.right;
		if (buttonWidth > minWidth)
			minWidth = buttonWidth;
	}

	rect->left = 0;
	rect->top = 0;
	rect->right = rect->left + infoWidth;
	rect->bottom = rect->top + infoHeight;

	rect->right += (clientMargins.left + infoMargins.left + clientMargins.right + infoMargins.right);
	if ((rect->right - rect->left) < minWidth)
		rect->right = rect->left + minWidth;
	
	rect->bottom += (0 != notifierHeight) ?
		(notifierHeight + infoMargins.top) : (clientMargins.top + infoMargins.top);

	rect->bottom += (buttonHeight > 0) ?
		(2 * clientMargins.bottom + buttonHeight + infoMargins.bottom) : (clientMargins.bottom + infoMargins.bottom);

	return TRUE;

}

HDWP LoginPopup::LayoutButtons(HDWP hdwp, const INT *buttonList, UINT buttonCount, BOOL redraw, RECT *rectOut)
{
	RECT rect;
	GetClientRect(hwnd, &rect);
	rect.left += clientMargins.left;
	rect.top += clientMargins.top;
	rect.right -= clientMargins.right;
	rect.bottom -= clientMargins.bottom;

	return LoginBox_LayoutButtonBar(hdwp, hwnd, buttonList, buttonCount, &rect, 
				buttonHeight, buttonSpace, redraw, rectOut);
}

BOOL LoginPopup::GetTextSize(HWND hText, LONG width, SIZE *size)
{
	if (NULL == hText) return FALSE;
		
	WCHAR szBuffer[4096] = {0};
	INT cchLen = (INT)SendMessage(hText, WM_GETTEXT, ARRAYSIZE(szBuffer), (LPARAM)szBuffer);
	if (0 == cchLen)
	{
		size->cx = 0;
		size->cy = 0;
		return TRUE;
	}
	
	HDC hdc = GetDCEx(hText, NULL, DCX_CACHE | DCX_WINDOW | DCX_NORESETATTRS);
	if (NULL == hdc)
		return FALSE;

	HFONT font = (HFONT)SendMessage(hText, WM_GETFONT, 0, 0L);
	HFONT fontOrig = (HFONT)SelectObject(hdc, font);
	
	BOOL resultOk;
	RECT rect;
	SetRect(&rect, 0, 0, width, 0);
	resultOk = DrawText(hdc, szBuffer, cchLen, &rect, 
		DT_CALCRECT | DT_EXTERNALLEADING | DT_LEFT | DT_NOPREFIX | DT_WORDBREAK);
		
	if(FALSE != resultOk)
	{
		size->cx = (rect.right - rect.left);
		size->cy = (rect.bottom - rect.top);
	}

	SelectObject(hdc, fontOrig);
	ReleaseDC(hText, hdc);
	
	return resultOk;
}

BOOL LoginPopup::OnInitDialog(HWND hFocus, LPARAM param)
{
	RegisterPopup(hwnd, TRUE);

	RECT rect;
	if (FALSE == GetWindowRect(hwnd, &rect))
		SetRectEmpty(&rect);

	idealSize.cx = rect.right - rect.left;
	idealSize.cy = rect.bottom - rect.top;

	
	HWND hNotifier = LoginNotifier_CreateWindow(0, WS_CHILD | WS_CLIPSIBLINGS, 0, 0, 0, 0, hwnd, IDC_NOTIFIER);
	if (NULL != hNotifier) 
	{
		HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
		if (NULL != hFont)
			SendMessage(hNotifier, WM_SETFONT, (WPARAM)hFont, 0L);

#ifdef COLOR_TITLE
		LoginNotifier_SetBkColor(hNotifier, GetSysColor(COLOR_TITLE));
#endif //COLOR_TITLE

#ifdef COLOR_TITLETEXT
		LoginNotifier_SetTextColor(hNotifier, GetSysColor(COLOR_TITLETEXT));
#endif //COLOR_TITLETEXT

		UpdateTitle(FALSE);
	}

	UpdateMargins();
	UpdateLayout(FALSE);
	
	PostMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0L);
	return FALSE;
}

void LoginPopup::OnDestroy()
{	
}

void LoginPopup::OnWindowPosChanged(const WINDOWPOS *pwp)
{
	if (SWP_NOSIZE != ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) 
		UpdateLayout(0 == (SWP_NOREDRAW & pwp->flags));
}

void LoginPopup::OnCommand(UINT commandId, UINT eventType, HWND hControl)
{
	switch(commandId)
	{
		case IDOK:
		case IDCANCEL:
			EndDialog(commandId);
			break;
	}
}

LRESULT LoginPopup::OnNotify(UINT controlId, const NMHDR *pnmh)
{
	return FALSE;
}

void LoginPopup::OnPaint()
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right)
			Paint(ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

void LoginPopup::OnPrintClient(HDC hdc, UINT options)
{
	if (0 != (PRF_CLIENT & options))
	{
		RECT clientRect;
		if (GetClientRect(hwnd, &clientRect))
			Paint(hdc, &clientRect, TRUE);
	}
}

HBRUSH LoginPopup::OnGetStaticColor(HDC hdc, HWND hControl)
{	
	HBRUSH hb = (HBRUSH)GetSysColorBrush(COLOR_CLIENT);
	SetTextColor(hdc, GetSysColor(COLOR_CLIENTTEXT));
	SetBkColor(hdc, GetSysColor(COLOR_CLIENT));
	return hb;
}

void LoginPopup::OnSetFont(HFONT font, BOOL redraw)
{
	DefDlgProc(hwnd, WM_SETFONT, (WPARAM)font, MAKELPARAM(redraw, 0));
	UpdateMargins();
}

void LoginPopup::OnParentNotify(UINT eventId, UINT wParam, LPARAM lParam)
{
}

BOOL LoginPopup::OnUpdateWindowPos(const RECT* clientRect, RECT *rectOut)
{
	if (NULL == clientRect || NULL == rectOut) 
		return FALSE;

	LONG width = idealSize.cx;
	LONG height = idealSize.cy;
	rectOut->left = clientRect->left + ((clientRect->right - clientRect->left) - width)/2;
	rectOut->top = clientRect->top+ ((clientRect->bottom - clientRect->top) - height)/2;
	rectOut->right = rectOut->left + width;
	rectOut->bottom = rectOut->top + height;

	return TRUE;
}

void LoginPopup::OnPlayBeep()
{
	LoginBox_MessageBeep(MB_ICONASTERISK);
}

INT_PTR LoginPopup::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:			return OnInitDialog((HWND)wParam, lParam);
		case WM_DESTROY:			OnDestroy(); return TRUE;
		case WM_NOTIFY:				MSGRESULT(hwnd, OnNotify((INT)wParam, (NMHDR*)lParam));
		case WM_COMMAND:			OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
		case WM_WINDOWPOSCHANGED:	OnWindowPosChanged((WINDOWPOS*)lParam); return TRUE;
		case WM_SIZE:				return TRUE;
		case WM_SETFONT:			OnSetFont((HFONT)wParam, (BOOL)LOWORD(lParam)); return TRUE;
		case WM_ERASEBKGND:			MSGRESULT(hwnd, 0);
		case WM_PAINT:				OnPaint(); return TRUE;
		case WM_PRINTCLIENT:		OnPrintClient((HDC)wParam, (UINT)lParam); return TRUE;
		case WM_CTLCOLORSTATIC:		return (INT_PTR)OnGetStaticColor((HDC)wParam, (HWND)lParam);
		case WM_PARENTNOTIFY:		OnParentNotify(LOWORD(wParam), HIWORD(wParam), lParam); return TRUE;
		
		case NLPOPUP_UPDATEWNDPOS:	MSGRESULT(hwnd, OnUpdateWindowPos((const RECT*)wParam, (RECT*)lParam));
		case NLPOPUP_PLAYBEEP:		OnPlayBeep(); return TRUE;
	}
	return FALSE;
}

static INT_PTR CALLBACK LoginPopup_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static ATOM LOGINPOPUP_PROP = 0;
	LoginPopup *popup = (LoginPopup*)GetProp(hwnd, MAKEINTATOM(LOGINPOPUP_PROP));

	if (NULL == popup) 
	{
		switch(uMsg)
		{
			case WM_INITDIALOG:
				if (0 == LOGINPOPUP_PROP)
				{
					LOGINPOPUP_PROP = GlobalAddAtomW(L"NullsoftLoginPopupProp");
					if (0 == LOGINPOPUP_PROP)
						return 0;
				}

				if (NULL != lParam)
				{
					LOGINPOPUPCREATEPARAM *create = (LOGINPOPUPCREATEPARAM*)lParam;
					lParam = create->lParam;
					if (SUCCEEDED(create->fnCreator(hwnd, lParam, &popup)))
					{
						if (FALSE == SetProp(hwnd, MAKEINTATOM(LOGINPOPUP_PROP), (HANDLE)popup))
						{
							delete(popup);
							popup = NULL;
						}
					}
				}

				if (NULL != popup)
					return popup->DialogProc(uMsg, wParam, lParam);

				break;
		}
		return 0;
	}

	INT_PTR result = popup->DialogProc(uMsg, wParam, lParam);

	if (WM_NCDESTROY == uMsg)
	{
		RemoveProp(hwnd, MAKEINTATOM(LOGINPOPUP_PROP));
		delete(popup);
	}

	return result;
}

static LRESULT CALLBACK LoginPopup_MessageFilter(INT code, WPARAM wParam, LPARAM lParam)
{
	THREADPOPUPDATA *data = (NULL != WASABI_API_APP && TLS_OUT_OF_INDEXES != threadStorage) ? 
								(THREADPOPUPDATA*)WASABI_API_APP->GetThreadStorage(threadStorage) : NULL;
	if (NULL == data)
		return 0;

	if (code >= 0)
	{
		MSG *pMsg = (MSG*)lParam;
		if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
		{
			if (L'C' == pMsg->wParam && 0 == (0x40000000 & pMsg->lParam) && 
				0 != (0x8000 & GetAsyncKeyState(VK_MENU)))
			{
				pMsg->wParam = VK_ESCAPE;
			}

			if ((VK_ESCAPE == pMsg->wParam || VK_RETURN == pMsg->wParam) &&
				0 == (0x40000000 & pMsg->lParam))
			{
				size_t index = data->windowList.size();
				while(index--)
				{
					HWND hPopup = data->windowList[index];
					if (IsChild(hPopup, pMsg->hwnd))
					{
						INT commandId;
						switch(pMsg->wParam)
						{
							case VK_ESCAPE:
								commandId = IDCANCEL;
								break;
							case VK_RETURN:
								if (0 != (DLGC_BUTTON & SendMessage(pMsg->hwnd, WM_GETDLGCODE, 0, 0L)) &&
									IsWindowVisible(pMsg->hwnd) && IsWindowEnabled(pMsg->hwnd))
								{
									commandId = (INT)(INT_PTR)GetWindowLongPtr(pMsg->hwnd, GWLP_ID);
								}
								else
								{
									commandId = (INT)(INT_PTR)SendMessage(hPopup, DM_GETDEFID, 0, 0L);
									if (DC_HASDEFID != HIWORD(commandId))
										commandId = IDOK;
								}
								break;
						}
						SendMessage(hPopup, WM_COMMAND, MAKEWPARAM(commandId, 0), (LPARAM)pMsg->hwnd);
						return 1;
					}
				}
			}

			// add  mnemonic support here (http://msdn.microsoft.com/en-us/library/ms644995%28VS.85%29.aspx)
			//HWND hPopup;
			//size_t index = data->windowList.size();
			//while(index--)
			//{	
			//	hPopup = data->windowList[index];
			//	if (pMsg->hwnd == hPopup || IsChild(hPopup, pMsg->hwnd))
			//	{
			//	
			//	}
			//}
		}
	}
	return CallNextHookEx(data->hHook, code, wParam, lParam);
}