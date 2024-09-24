#include "./popupPasscode.h"
#include "./common.h"
#include "./loginNotifier.h"
#include "./dataCredentials.h"
#include "./editboxExtender.h"

#include "../resource.h"
#include "../api.h"
#include "../api_auth.h"

#include <windows.h>
#include <commctrl.h>
#include <strsafe.h>

#define MAX_PASSCODE_LENGTH			6
#define MIN_PASSCODE_LENGTH			6

#define TIMER_UPDATETITLE_ID		14
#define TIMER_UPDATETITLE_DELAY		50

static HRESULT CALLBACK LoginPopupPasscode_CreateInstance(HWND hwnd, LPARAM param, LoginPopup **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == hwnd) return E_INVALIDARG;

	*instance = new LoginPopupPasscode(hwnd);
	if (NULL == instance) return E_OUTOFMEMORY;

	return S_OK;
}

LoginPopupPasscode::LoginPopupPasscode(HWND hwnd)
	: LoginPopup(hwnd, -1, MAKEINTRESOURCE(IDS_POPUP_PASSCODE_TITLE)), 
	loginData(NULL), message(NULL), messageType(-1)
{
}

LoginPopupPasscode::~LoginPopupPasscode()
{
	if (NULL != loginData)
		loginData->Release();

	LoginBox_FreeString(message);
}

HWND LoginPopupPasscode::CreatePopup(HWND hParent, LoginData *loginData)
{
	LoginDataCredentials *credentials;
	if (NULL == loginData || 
		FAILED(loginData->QueryInterface(IID_LoginDataCredentials, (void**)&credentials)))
	{
		return NULL;
	}

	HWND hwnd = LoginPopup::CreatePopup(MAKEINTRESOURCE(IDD_POPUP_PASSCODE), hParent, (LPARAM)credentials, LoginPopupPasscode_CreateInstance);
	credentials->Release();
	return hwnd;
}

void LoginPopupPasscode::UpdateLayout(BOOL fRedraw)
{	
	LoginPopup::UpdateLayout(fRedraw);

	RECT rect;
	if (FALSE == GetInfoRect(&rect)) return;

	const INT szButtons[] = { IDOK, };
	const INT szControls[] = { IDC_PASSCODE_TITLE, IDC_PASSCODE_EDIT, IDC_PASSCODE_HINT, };

	HDWP hdwp = BeginDeferWindowPos(ARRAYSIZE(szControls) + ARRAYSIZE(szButtons));
	if (NULL == hdwp) return;

	hdwp = LayoutButtons(hdwp, szButtons, ARRAYSIZE(szButtons), fRedraw, NULL);

	LONG top = rect.top;
	RECT controlRect;

	for(INT i = 0; i < ARRAYSIZE(szControls); i++)
	{
		HWND hControl  = GetDlgItem(hwnd, szControls[i]);
		if (NULL == hControl || FALSE == GetWindowRect(hControl, &controlRect))
			continue;

		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&controlRect, 2);

		hdwp = DeferWindowPos(hdwp, hControl, NULL, 
					rect.left, top, rect.right - rect.left, controlRect.bottom - controlRect.top, 
					SWP_NOACTIVATE | SWP_NOZORDER);

		if (NULL == hdwp)
			return;

		top += controlRect.bottom - controlRect.top;

		switch(szControls[i])
		{
			case IDC_PASSCODE_TITLE:	top += 2; break;
		}
		
	}



	EndDeferWindowPos(hdwp);
}


void LoginPopupPasscode::EndDialog(INT_PTR code)
{
	if (IDOK == code)
	{
		code = -1;
		if (NULL != loginData)
		{
			HWND hEdit = GetDlgItem(hwnd, IDC_PASSCODE_EDIT);
			if (NULL != hEdit)
			{
				WCHAR szBuffer[64] = {0};
				GetWindowText(hEdit, szBuffer, ARRAYSIZE(szBuffer));
				if(SUCCEEDED(loginData->SetPasscode(szBuffer)))
					code = IDOK;
			}
		}
	}

	NPPNRESULT result;
	result.exitCode = code;
	result.loginData = loginData;
	if (NULL != result.loginData)
		result.loginData->AddRef();

	SendNotification(NPPN_RESULT, (NMHDR*)&result);

	if (NULL != result.loginData)
		result.loginData->Release();

	LoginPopup::EndDialog(code);
}

BOOL LoginPopupPasscode::Validate()
{
	BOOL validatedOk = TRUE;

	HWND hEdit = GetDlgItem(hwnd, IDC_PASSCODE_EDIT);
	if (NULL == hEdit)
	{
		SetAlert(NLNTYPE_ERROR, MAKEINTRESOURCE(IDS_ERR_UNEXPECTED));
		validatedOk = FALSE;
	}
	else
	{
		INT cchText = (INT)SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0L);
		if (cchText < MIN_PASSCODE_LENGTH || cchText > MAX_PASSCODE_LENGTH)
		{
			WCHAR szBuffer[256] = {0}, szTemplate[256] = {0};
			WASABI_API_LNGSTRINGW_BUF(IDS_ERR_PASSCODE_BADLENGTH_TEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
			StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szTemplate, MAX_PASSCODE_LENGTH);
			SetAlert(NLNTYPE_ERROR, szBuffer);
			validatedOk = FALSE;
		}
		else
		{
			WCHAR szPasscode[MAX_PASSCODE_LENGTH + 1] = {0};
			WORD szType[MAX_PASSCODE_LENGTH] = {0};
			SendMessage(hEdit, WM_GETTEXT, (WPARAM)ARRAYSIZE(szPasscode), (LPARAM)szPasscode);
			if (FALSE != GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, szPasscode, cchText, szType))
			{
				for(INT i = 0; i < cchText; i++)
				{
					if (0 == (C1_DIGIT & szType[i]))
					{
						SetAlert(NLNTYPE_ERROR, MAKEINTRESOURCE(IDS_ERR_PASSCODE_ONLYDIGITS));
						validatedOk = FALSE;
						break;
					}
				}
			}
			else
			{
				SetAlert(NLNTYPE_ERROR, MAKEINTRESOURCE(IDS_ERR_UNEXPECTED));
				validatedOk = FALSE;
			}
		}
	}
	
	UpdateTitle(TRUE);
	return validatedOk;
}

BOOL LoginPopupPasscode::OnInitDialog(HWND hFocus, LPARAM param)
{
	loginData = (LoginDataCredentials*)param;
	if (NULL != loginData)
		loginData->AddRef();

	if (NULL != loginData)
	{
		LPCWSTR pcode = loginData->GetPasscode();
		if (NULL != pcode && L'\0' != *pcode)
		{
			SetAlert(NLNTYPE_ERROR, MAKEINTRESOURCE(IDS_ERR_PASSCODE_INVALID));
			UpdateTitle(FALSE);
		}
	}

	HWND hEdit = GetDlgItem(hwnd, IDC_PASSCODE_EDIT);
	if (NULL != hEdit)
	{
		EditboxExtender_AttachWindow(hEdit);
	}
		
	HWND hHint = GetDlgItem(hwnd, IDC_PASSCODE_HINT);
	if (NULL != hHint)
	{
		WCHAR szBuffer[128], szTemplate[128] = {0};
		WASABI_API_LNGSTRINGW_BUF(IDS_PASSCODE_EDIT_HINT, szTemplate, ARRAYSIZE(szTemplate));
		StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szTemplate, MAX_PASSCODE_LENGTH);
		SendMessage(hHint, WM_SETTEXT, 0, (LPARAM)szBuffer);
	}
	
	return LoginPopup::OnInitDialog(hFocus, param);
}

void LoginPopupPasscode::OnCommand(UINT commandId, UINT eventType, HWND hControl)
{
	if (IDOK == commandId && FALSE == Validate())
		return;

	LoginPopup::OnCommand(commandId, eventType, hControl);
}

LRESULT LoginPopupPasscode::OnNotify(UINT controlId, const NMHDR *pnmh)
{
	switch(controlId)
	{
		case IDC_PASSCODE_EDIT:
			switch(pnmh->code)
			{
				case NM_CHAR:	
					return OnEditboxChar(pnmh->hwndFrom, ((NMCHAR*)pnmh)->ch);
				case NM_KEYDOWN:	
					return OnEditboxKey(pnmh->hwndFrom, ((NMKEY*)pnmh)->nVKey, ((NMKEY*)pnmh)->uFlags);
				case EENM_PASTE:
					return OnEditboxPaste(pnmh->hwndFrom, ((EENMPASTE*)pnmh)->text);
			}
			return 0;
	}
	return LoginPopup::OnNotify(controlId, pnmh);
}

HBRUSH LoginPopupPasscode::OnGetStaticColor(HDC hdc, HWND hControl)
{
	HBRUSH hb = LoginPopup::OnGetStaticColor(hdc, hControl);

	INT controlId = (NULL != hControl) ? (INT)GetWindowLongPtr(hControl, GWLP_ID) : 0;
	switch(controlId)
	{
		case IDC_PASSCODE_HINT:
			SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
			break;
	}

	return hb;
}


LRESULT	LoginPopupPasscode::OnEditboxKey(HWND hEdit, UINT vKey, UINT flags)
{
	if (-1 != alertType)
	{
		RemoveAlert();
		SetTimer(hwnd, TIMER_UPDATETITLE_ID, TIMER_UPDATETITLE_DELAY, NULL);
	}
	return 0L;
}

LRESULT LoginPopupPasscode::OnEditboxChar(HWND hEdit, UINT ch)
{	
	if (VK_BACK == ch)
		return FALSE;
	
	BOOL filtered = FALSE;

	INT selBegin, selEnd;
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&selBegin, (LPARAM)&selEnd);

	INT cchText = (INT)SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0L);
	if (selBegin == selEnd && cchText >= MAX_PASSCODE_LENGTH)
	{
		WCHAR szBuffer[256] = {0}, szTemplate[256] = {0};
		WASABI_API_LNGSTRINGW_BUF(IDS_ERR_PASSCODE_BADLENGTH_TEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
		StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szTemplate, MAX_PASSCODE_LENGTH);
		SetAlert(NLNTYPE_WARNING, szBuffer);
		KillTimer(hwnd, TIMER_UPDATETITLE_ID);
		UpdateTitle(TRUE);
		filtered = TRUE;
	}
	else
	{
		WORD charType;
		if (FALSE != GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, (WCHAR*)&ch, 1, &charType))
		{
			if (0 == (C1_DIGIT & charType))
			{		
				SetAlert(NLNTYPE_WARNING, MAKEINTRESOURCE(IDS_ERR_PASSCODE_ONLYDIGITS));
				KillTimer(hwnd, TIMER_UPDATETITLE_ID);
				UpdateTitle(TRUE);
				filtered = TRUE;
			}
		}
	}

	return filtered;
}

LRESULT LoginPopupPasscode::OnEditboxPaste(HWND hEdit, LPCWSTR pszText)
{
	if (NULL == pszText)
		return TRUE;

	INT cchText = lstrlen(pszText);
	if (0 == cchText) return TRUE;

	WCHAR szBuffer[MAX_PASSCODE_LENGTH + 1] = {0};
	INT  iBuffer = 0;
	
	WORD charType;

	BOOL validatedOk = TRUE;

	
	INT maxSize  = MAX_PASSCODE_LENGTH - (INT)SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0L);
	if (maxSize < MAX_PASSCODE_LENGTH)
	{
		INT selBegin, selEnd;
		SendMessage(hEdit, EM_GETSEL, (WPARAM)&selBegin, (LPARAM)&selEnd);
		selEnd -= selBegin;
		if (selEnd < 0) selEnd = -selBegin;
		maxSize += selEnd;
	}
	for (INT i =0; i <cchText; i++)
	{
		if (FALSE == GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, &pszText[i], 1, &charType))
		{
			SetAlert(NLNTYPE_WARNING, MAKEINTRESOURCE(IDS_ERR_UNEXPECTED));
			validatedOk = FALSE;
			break;
		}
		else if (0 == (C1_DIGIT & charType))
		{		
			if (0 == ((C1_SPACE | C1_CNTRL | C1_BLANK) & charType))
			{
				SetAlert(NLNTYPE_WARNING, MAKEINTRESOURCE(IDS_ERR_PASSCODE_ONLYDIGITS));
				validatedOk = FALSE;
				break;
			}
		}
		else
		{
			if (iBuffer > maxSize)
			{
				WCHAR szBuffer[256], szTemplate[256] = {0};
				WASABI_API_LNGSTRINGW_BUF(IDS_ERR_PASSCODE_BADLENGTH_TEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
				StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szTemplate, MAX_PASSCODE_LENGTH);
				SetAlert(NLNTYPE_WARNING, szBuffer);
				validatedOk = FALSE;
				break;
			}
			else
			{
				szBuffer[iBuffer] = pszText[i];
				iBuffer++;
			}
		}
	}

	if(FALSE != validatedOk)
	{
		szBuffer[iBuffer] = L'\0';
		SendMessage(hEdit, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szBuffer);
	}
	else
	{
		UpdateTitle(TRUE);
	}

	return TRUE;
}

INT_PTR LoginPopupPasscode::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_TIMER:
			switch(wParam)
			{
				case TIMER_UPDATETITLE_ID:
					KillTimer(hwnd, wParam);
					UpdateTitle((-1 != alertType));
					return TRUE;
			}
			break;
	}
	return LoginPopup::DialogProc(uMsg, wParam, lParam);
}
