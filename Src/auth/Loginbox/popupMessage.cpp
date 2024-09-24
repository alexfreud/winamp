#include "./popupMessage.h"
#include "./loginNotifier.h"
#include "./common.h"

#include "../resource.h"
#include "../api.h"

#include <windows.h>
#include <strsafe.h>

typedef struct __MESSAGECREATEPARAM
{
	UINT type;
	LPCWSTR title;
	LPCWSTR message;
	LoginPopupMessage::ResultCallback callback;
	LPARAM param;
} MESSAGECREATEPARAM;

typedef struct __MESSAGEBUTTON
{
	INT		id;
	LPCWSTR pTitle;
	BOOL	fGroup;
	BOOL	fDisabled;
	BOOL	fDefault;
} MESSAGEBUTTON;

const static MESSAGEBUTTON szTypeContinue[] =
{
	{ IDOK, MAKEINTRESOURCE(IDS_BUTTON_CONTINUE), TRUE, FALSE, TRUE, },
};

const static MESSAGEBUTTON szTypeYesNo[] =
{
	{ IDYES, MAKEINTRESOURCE(IDS_BUTTON_YES), TRUE, FALSE, TRUE, },
	{ IDNO, MAKEINTRESOURCE(IDS_BUTTON_NO), FALSE, FALSE, FALSE, },
};

static HRESULT CALLBACK LoginPopupMessage_CreateInstance(HWND hwnd, LPARAM param, LoginPopup **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == hwnd) return E_INVALIDARG;

	*instance = new LoginPopupMessage(hwnd);

	if (NULL == instance) return E_OUTOFMEMORY;

	return S_OK;
}

LoginPopupMessage::LoginPopupMessage(HWND hwnd)
	: LoginPopup(hwnd, NLNTYPE_INFORMATION, NULL), callback(NULL), param(0L),
	buttonsCount(0)
{
	memset(szButtons, 0, sizeof(szButtons));
}

LoginPopupMessage::~LoginPopupMessage()
{
}

HWND LoginPopupMessage::CreatePopup(HWND hParent, LPCWSTR pszTitle, LPCWSTR pszMessage, UINT uType, ResultCallback callback, LPARAM param)
{
	switch(typeMask & uType)
	{
		case typeContinue:
		case typeYesNo:
			break;
		default:
			return NULL;
	}

	MESSAGECREATEPARAM createParam;
	createParam.type = uType;
	createParam.title = pszTitle;
	createParam.message = pszMessage;
	createParam.callback = callback;
	createParam.param = param;

	return LoginPopup::CreatePopup(MAKEINTRESOURCE(IDD_POPUP_MESSAGE), hParent, 
				(LPARAM)&createParam, LoginPopupMessage_CreateInstance);
}

void LoginPopupMessage::UpdateLayout(BOOL fRedraw)
{
	LoginPopup::UpdateLayout(fRedraw);

	RECT rect;
	if (FALSE == GetInfoRect(&rect)) return;
		
	HDWP hdwp = BeginDeferWindowPos(1 + buttonsCount);
	if (NULL == hdwp) return;

	UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
	if (FALSE == fRedraw) flags |= SWP_NOREDRAW;
	
	HWND hMessage = GetDlgItem(hwnd, IDC_MESSAGE);
	if (NULL != hMessage)
	{
		hdwp = DeferWindowPos(hdwp, hMessage, NULL, 
				rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, flags);

		if (NULL == hdwp) return;
	}

	if (NULL != buttonsCount)
		hdwp = LayoutButtons(hdwp, szButtons, buttonsCount, fRedraw, NULL);

	EndDeferWindowPos(hdwp);
}


void LoginPopupMessage::EndDialog(INT_PTR code)
{
	ResultCallback callbackCopy = callback;
	LPARAM paramCopy = param;


	NLPNRESULT result;
	result.exitCode = code;
	SendNotification(NLPN_RESULT, (NMHDR*)&result);

	if (NULL != callbackCopy)
		callbackCopy(hwnd, code, paramCopy);

	LoginPopup::EndDialog(code);
}

static BOOL LoginPopupMessage_CreateButtons(HWND hwnd, const MESSAGEBUTTON *buttonList, UINT *buttonsCount, INT *buttonIdList)
{
	if (NULL == hwnd || NULL == buttonList || NULL == buttonsCount || 0 == *buttonsCount)
		return FALSE;

	UINT count = *buttonsCount;
	*buttonsCount = 0;

	WCHAR szBuffer[256] = {0};
	RECT rect;
	SetRect(&rect, 50, 15, 0, 0);
	MapDialogRect(hwnd, &rect);

	LONG width = rect.left;
	LONG height = rect.top;

	HFONT font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);

	for (UINT i = 0; i < count; i++)
	{
		LPCWSTR title = buttonList[i].pTitle;
		if (NULL != title && FALSE != IS_INTRESOURCE(title))
		{
			WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)title, szBuffer, ARRAYSIZE(szBuffer));
			title = szBuffer;
		}

		UINT style = BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP;
		if (FALSE != buttonList[i].fGroup) style |= WS_GROUP;
		if (FALSE != buttonList[i].fDisabled) style |= WS_DISABLED;
		if (FALSE != buttonList[i].fDefault) style |= BS_DEFPUSHBUTTON;

		HWND hButton = CreateWindowEx(WS_EX_NOPARENTNOTIFY, L"Button", title, style, 
			0, 0, width, height, hwnd, (HMENU)(INT_PTR)buttonList[i].id, NULL, 0L);

		if (NULL != hButton)
		{
			if (NULL != font)
				SendMessage(hButton, WM_SETFONT, (WPARAM)font, 0L);

			if (NULL != buttonIdList)
				buttonIdList[*buttonsCount] = buttonList[i].id;
			
			(*buttonsCount)++;
		}

	}
	return TRUE;
}

BOOL LoginPopupMessage::OnInitDialog(HWND hFocus, LPARAM param)
{
	MESSAGECREATEPARAM *createParam = (MESSAGECREATEPARAM*)param;

	if (NULL != createParam)
	{
		callback = createParam->callback;
		param  = createParam->param;
		
		switch(iconMask & createParam->type)
		{
			case iconInfo:		popupType = NLNTYPE_INFORMATION; break;
			case iconWarning:	popupType = NLNTYPE_WARNING; break;
			case iconError:		popupType = NLNTYPE_ERROR; break;
		}
		SetTitle(popupType, createParam->title);

		switch(typeMask & createParam->type)
		{
			case typeContinue:	
				buttonsCount = ARRAYSIZE(szTypeContinue);
				LoginPopupMessage_CreateButtons(hwnd, szTypeContinue, &buttonsCount, szButtons);
				break;
			case typeYesNo:
				buttonsCount = ARRAYSIZE(szTypeYesNo);
				LoginPopupMessage_CreateButtons(hwnd, szTypeYesNo, &buttonsCount, szButtons);
				break;
		}

		HWND hMessage = GetDlgItem(hwnd, IDC_MESSAGE);
		if (NULL != hMessage)
		{
			if (NULL != createParam->message && FALSE != IS_INTRESOURCE(createParam->message))
			{
				WCHAR szBuffer[4096] = {0};
				WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)createParam->message, szBuffer, ARRAYSIZE(szBuffer));
				SendMessage(hMessage, WM_SETTEXT, 0, (LPARAM)szBuffer);
			}
			else
				SendMessage(hMessage, WM_SETTEXT, 0, (LPARAM)createParam->message);
		}
	}
	return LoginPopup::OnInitDialog(hFocus, param);
}

BOOL LoginPopupMessage::OnUpdateWindowPos(const RECT* clientRect, RECT *rectOut)
{
	if (NULL == clientRect || NULL == rectOut)
		return FALSE;

	SIZE size;
	
	LONG maxWidth = clientRect->right - clientRect->left - 
					(clientMargins.right + clientMargins.left) -
					(infoMargins.right + infoMargins.left);

	if (FALSE == GetTextSize(GetDlgItem(hwnd, IDC_MESSAGE), maxWidth, &size))
	{
		size.cx = 0;
		size.cy = 0;
	}
	
	if (FALSE == CalculateWindowRect(size.cx, size.cy, szButtons, buttonsCount, TRUE, rectOut))
		return FALSE;

	LONG ox = clientRect->left + ((clientRect->right - clientRect->left) - (rectOut->right - rectOut->left))/2;
	LONG oy = clientRect->top + ((clientRect->bottom - clientRect->top) - (rectOut->bottom - rectOut->top))/2;

	if (ox < clientRect->left) ox = clientRect->left;
	if (oy < clientRect->top) oy = clientRect->top;
	
	OffsetRect(rectOut, ox, oy);
	return TRUE;
}