#include "./pageCredentials.h"
#include "./dataCredentials.h"
#include "./common.h"
#include "./loginGui.h"
#include "../../winamp/commandLink.h"
#include "../resource.h"

#include <commctrl.h>


static HRESULT CALLBACK LoginPageCredentials_CreateInstance(HWND hwnd, HWND hLoginbox, LoginPage **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == hwnd || NULL == hLoginbox) return E_INVALIDARG;

	*instance = new LoginPageCredentials(hwnd, hLoginbox);
	if (NULL == instance) return E_OUTOFMEMORY;

	return S_OK;
}

LoginPageCredentials::LoginPageCredentials(HWND hwnd, HWND hLoginbox)
	: LoginPage(hwnd, hLoginbox), accountRecoverUrl(NULL), accountCreateUrl(NULL)
{

}

LoginPageCredentials::~LoginPageCredentials()
{
	LoginBox_FreeString(accountRecoverUrl);
	LoginBox_FreeString(accountCreateUrl);
}

HWND LoginPageCredentials::CreatePage(HWND hLoginbox, HWND hParent)
{
	return LoginPage::CreatePage(hLoginbox, MAKEINTRESOURCE(IDD_PAGE_CREDENTIALS), 
						hParent, NULL, LoginPageCredentials_CreateInstance);
}



void LoginPageCredentials::UpdateLayout(BOOL fRedraw)
{
	LoginPage::UpdateLayout(fRedraw);
	
	RECT pageRect;
	if (FALSE == GetPageRect(&pageRect))
		return;

	UINT flags = SWP_NOACTIVATE | SWP_NOZORDER;
	if (FALSE == fRedraw) flags |= SWP_NOREDRAW;

	const INT szControls[] = { IDC_USERNAME_LABEL, IDC_USERNAME, IDC_PASSWORD_LABEL, IDC_PASSWORD};
	const INT szLinks[] = { IDC_CREATE_ACCOUNT, IDC_RECOVER_ACCOUNT };

	INT baseunitX, baseunitY;
	if (FALSE == LoginBox_GetWindowBaseUnits(hwnd, &baseunitX, &baseunitY))
	{
		baseunitX = 6;
		baseunitY = 13;
	}

	HDWP hdwp = BeginDeferWindowPos(ARRAYSIZE(szControls) + ARRAYSIZE(szLinks));
	
	HWND hControl;
	INT cx, cy, x, y;
		
	INT editWidth = 0;
	INT column1Width = 0;
	INT column2Width = 0;
	INT columnSpace = MulDiv(7, baseunitX, 4);
	SIZE sizeLink;
	RECT linkMargins;

	// column 1 width
	for (INT i = 0; i < ARRAYSIZE(szControls); i++)
	{
		hControl = GetDlgItem(hwnd, szControls[i]);
		if (NULL == hControl) continue;
		
		switch(szControls[i])
		{
			case IDC_USERNAME_LABEL:
			case IDC_PASSWORD_LABEL:
				if (FALSE != (LoginBox_GetWindowTextSize(hControl, pageRect.right - pageRect.left, &cx, &cy)) &&
					cx > column1Width)
				{
					column1Width = cx;
				}
				break;
			case IDC_USERNAME:
			case IDC_PASSWORD:
				if (0 == editWidth)
				{
					HDC hdc = GetDCEx(hControl, NULL, DCX_CACHE | DCX_NORESETATTRS);
					if (NULL != hdc)
					{
						HFONT fontControl = (HFONT)SendMessage(hControl, WM_GETFONT, 0, 0L);
						HFONT fontOrig = (HFONT)SelectObject(hdc, fontControl);
						
						editWidth = LoginBox_GetAveStrWidth(hdc, 28);
			
						SelectObject(hdc, fontOrig);
						ReleaseDC(hControl, hdc);
					}
					if (0 != editWidth)
					{
						LRESULT editMargins = SendMessage(hControl, EM_GETMARGINS, 0,0L);
						editWidth += (LOWORD(editMargins) + HIWORD(editMargins));
					}
				}
				if (editWidth > column1Width) 
					column1Width = editWidth;
				break;
		}
	}
	// column 2 width	
	for (INT i = 0; i < ARRAYSIZE(szLinks); i++)
	{
		hControl = GetDlgItem(hwnd, szLinks[i]);
		if (NULL != hControl && 
			0 != (WS_VISIBLE & GetWindowStyle(hControl)) &&
			FALSE != CommandLink_GetIdealSize(hControl, &sizeLink))
			
		{
			if (FALSE != CommandLink_GetMargins(hControl, &linkMargins))
				sizeLink.cx -= linkMargins.right;

			if (column2Width < sizeLink.cx)
				column2Width = sizeLink.cx;
		}
	}

	
	BOOL multiColumnLayout;
	if (column2Width > 0 && 
		(column1Width + columnSpace + column2Width) <= (pageRect.right - pageRect.left))
	{
		multiColumnLayout = TRUE;
	}
	else
	{
		multiColumnLayout = FALSE;
	}

	INT nextTop = 0;

	if (column2Width != 0)
	{
		if(FALSE == multiColumnLayout)
			nextTop = pageRect.top - GetTitleSpacing()/2;
		else
		{
			nextTop = pageRect.top;	
			HWND hLabel = GetDlgItem(hwnd, IDC_USERNAME_LABEL);
			INT offsetY = (NULL != hLabel) ? LoginBox_GetWindowTextHeight(hLabel, 0) : 0;
			if (0 == offsetY) offsetY = baseunitY;
			nextTop += (offsetY - 1);
		}

		for (INT i = 0; i < ARRAYSIZE(szLinks); i++)
		{
			hControl = GetDlgItem(hwnd, szLinks[i]);
			if (NULL == hControl || 0 == (WS_VISIBLE & GetWindowStyle(hControl))) 
				continue;

			if (FALSE == CommandLink_GetIdealSize(hControl, &sizeLink))
				ZeroMemory(&sizeLink, sizeof(SIZE));

			x = pageRect.right - sizeLink.cx;
			if (x < pageRect.left) x = pageRect.left;

			if (FALSE != CommandLink_GetMargins(hControl, &linkMargins))
				x += linkMargins.right;

			if (i == 0)
				nextTop -= linkMargins.top;

			hdwp = DeferWindowPos(hdwp, hControl, NULL, x, nextTop, sizeLink.cx, sizeLink.cy, flags);
			if (NULL == hdwp) break;

			nextTop += sizeLink.cy;

			if (i != (ARRAYSIZE(szLinks) -1))
				nextTop += MulDiv(1, baseunitY, 8);
		}
	}

	if (FALSE != multiColumnLayout || 0 == column2Width)
		nextTop = pageRect.top;
	else
		nextTop += MulDiv(2, baseunitY, 8);

	for (INT i = 0; i < ARRAYSIZE(szControls); i++)
	{
		hControl = GetDlgItem(hwnd, szControls[i]);
		if (NULL == hControl) continue;
		
		switch(szControls[i])
		{
			case IDC_USERNAME_LABEL:
			case IDC_PASSWORD_LABEL:
				x = pageRect.left;
				y = nextTop;
				LoginBox_GetWindowTextSize(hControl, pageRect.right - pageRect.left, &cx, &cy);
				nextTop += (cy + MulDiv(2, baseunitY, 8));
				if (cx > column1Width)
					cx = column1Width;
				break;
			case IDC_USERNAME:
			case IDC_PASSWORD:
				x = pageRect.left;
				y = nextTop;
				cy = LoginBox_GetWindowTextHeight(hControl, 0);
				{
					RECT r1, r2;
					GetWindowRect(hControl, &r1);
					SendMessage(hControl, EM_GETRECT, 0, (LPARAM)&r2);
					INT t = (r1.bottom - r1.top) - (r2.bottom - r2.top);
					cy += t;
				}
				nextTop += (cy + MulDiv(4, baseunitY, 8));
			
				cx = column1Width;
				if ((x + cx) > pageRect.right)
					cx = pageRect.right - x;

				break;

		}
		
		hdwp = DeferWindowPos(hdwp, hControl, NULL, x, y, cx, cy, flags);
		if (NULL == hdwp) break;
	}

	if (NULL != hdwp)
		EndDeferWindowPos(hdwp);
}


BOOL LoginPageCredentials::OnInitDialog(HWND hFocus, LPARAM param)
{
	HWND hControl;
	const INT szLinks[] = { IDC_CREATE_ACCOUNT, IDC_RECOVER_ACCOUNT };
	for (INT i = 0; i < ARRAYSIZE(szLinks); i++)
	{
		hControl = GetDlgItem(hwnd, szLinks[i]);
		if (NULL == hControl) continue;
		
		RECT linkMargins;
		SetRect(&linkMargins, 2, 0, 2, 2);
		MapDialogRect(hwnd, &linkMargins);
		CommandLink_SetMargins(hControl, &linkMargins);
	}

	HFONT fontLabel = NULL, fontEdit = NULL;
	LoginGuiObject *loginGui;
	if (SUCCEEDED(LoginGuiObject::QueryInstance(&loginGui)))
	{
		fontLabel = loginGui->GetTextFont();
		fontEdit = loginGui->GetEditorFont();
		loginGui->Release();
	}

	if (NULL != fontLabel)
	{
		const INT szLabels[] = { IDC_USERNAME_LABEL, IDC_PASSWORD_LABEL};
		for (INT i = 0; i < ARRAYSIZE(szLabels); i++)
		{
			hControl = GetDlgItem(hwnd, szLabels[i]);
			if (NULL != hControl)
				SendMessage(hControl, WM_SETFONT, (WPARAM)fontLabel, 0L);
		}
	}

	if (NULL != fontEdit)
	{
		const INT szEdits[] = { IDC_USERNAME, IDC_PASSWORD};
		for (INT i = 0; i < ARRAYSIZE(szEdits); i++)
		{
			hControl = GetDlgItem(hwnd, szEdits[i]);
			if (NULL != hControl)
				SendMessage(hControl, WM_SETFONT, (WPARAM)fontEdit, 0L);
		}
	}


	LoginPage::OnInitDialog(hFocus, param);
	return FALSE;
}

BOOL LoginPageCredentials::OnNotify(UINT controlId, const NMHDR *pnmh)
{
	switch(controlId)
	{
		case IDC_CREATE_ACCOUNT:
			if (pnmh->code == NM_CLICK && NULL != accountCreateUrl && L'\0' != *accountCreateUrl)
				LoginBox_OpenUrl(hwnd, accountCreateUrl, TRUE);
			return TRUE;

		case IDC_RECOVER_ACCOUNT:
			if (pnmh->code == NM_CLICK && NULL != accountRecoverUrl && L'\0' != *accountRecoverUrl)
				LoginBox_OpenUrl(hwnd, accountRecoverUrl, TRUE);
			return TRUE;
	}
	return LoginPage::OnNotify(controlId, pnmh);
}
BOOL LoginPageCredentials::OnGetLoginData(LoginData **ppLoginData)
{
	if (NULL == ppLoginData)
		return FALSE;

	HWND hEdit;
	LPWSTR username, password;

	hEdit  = GetDlgItem(hwnd, IDC_USERNAME);
	if (NULL == hEdit || FAILED(LoginBox_GetWindowText(hEdit, &username, NULL)))
		username = NULL;

	hEdit  = GetDlgItem(hwnd, IDC_PASSWORD);
	if (NULL == hEdit || FAILED(LoginBox_GetWindowText(hEdit, &password, NULL)))
		password = NULL;

	HRESULT hr = LoginDataCredentials::CreateInstance(NULL, hwnd, hLoginbox, 
					username, password, (LoginDataCredentials**)ppLoginData);

	LoginBox_FreeStringSecure(username);
	LoginBox_FreeStringSecure(password);

	return SUCCEEDED(hr);
}

void LoginPageCredentials::OnSetAccountRecoverUrl(LPCWSTR pszUrl)
{
	LoginBox_FreeString(accountRecoverUrl);
	accountRecoverUrl = LoginBox_CopyString(pszUrl);

	HWND hLink = GetDlgItem(hwnd, IDC_RECOVER_ACCOUNT);
	if (NULL != hLink) 
	{
		INT showWindow = (NULL != accountRecoverUrl && S_OK == IsValidURL(NULL, accountRecoverUrl, 0)) ? SW_SHOWNA : SW_HIDE;
		ShowWindow(hLink, showWindow);
	}
}

void LoginPageCredentials::OnSetAccountCreateUrl(LPCWSTR pszUrl)
{
	LoginBox_FreeString(accountCreateUrl);
	accountCreateUrl = LoginBox_CopyString(pszUrl);

	HWND hLink = GetDlgItem(hwnd, IDC_CREATE_ACCOUNT);
	if (NULL != hLink) 
	{
		INT showWindow = (NULL != accountCreateUrl && S_OK == IsValidURL(NULL, accountCreateUrl, 0)) ? SW_SHOWNA : SW_HIDE;
		ShowWindow(hLink, showWindow);
	}
}

void LoginPageCredentials::OnSetUsernameLabel(LPCWSTR pszLabel)
{
	SetLabelText(IDC_USERNAME_LABEL, pszLabel);
}

void LoginPageCredentials::OnSetPasswordLabel(LPCWSTR pszLabel)
{
	SetLabelText(IDC_PASSWORD_LABEL, pszLabel);
}

BOOL LoginPageCredentials::OnSetUsername(LPCWSTR pszUsername)
{
	HWND hEdit = GetDlgItem(hwnd, IDC_USERNAME);
	if (NULL == hEdit) return FALSE;

	BOOL result = (BOOL)SNDMSG(hEdit, WM_SETTEXT, 0, (LPARAM)pszUsername);
	if (FALSE != result)
	{
		SNDMSG(hEdit, EM_SETSEL, 0, -1);
	}
	return result;
}

BOOL LoginPageCredentials::OnSetPassword(LPCWSTR pszPassword)
{
	HWND hEdit = GetDlgItem(hwnd, IDC_PASSWORD);
	if (NULL == hEdit) return FALSE;

	BOOL result = (BOOL)SNDMSG(hEdit, WM_SETTEXT, 0, (LPARAM)pszPassword);
	if (FALSE != result)
	{
		SNDMSG(hEdit, EM_SETSEL, 0, -1);
	}
	return result;
}

HWND LoginPageCredentials::OnGetFirstItem()
{
	HWND hEdit = GetDlgItem(hwnd, IDC_USERNAME);
	if (NULL != hEdit && (WS_VISIBLE == ((WS_VISIBLE | WS_DISABLED) & GetWindowStyle(hEdit))))
		return hEdit;

	return LoginPage::OnGetFirstItem();
}


INT_PTR LoginPageCredentials::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case NLPCM_SETACCOUNTRECOVERURL:	OnSetAccountRecoverUrl((LPCWSTR)lParam); return TRUE;
		case NLPCM_SETACCOUNTCREATEURL:		OnSetAccountCreateUrl((LPCWSTR)lParam); return TRUE;
		case NLPCM_SETUSERNAMELABEL:		OnSetUsernameLabel((LPCWSTR)lParam); return TRUE;
		case NLPCM_SETPASSWORDLABEL:		OnSetPasswordLabel((LPCWSTR)lParam); return TRUE;
	}
	return __super::DialogProc(uMsg, wParam, lParam);
}

