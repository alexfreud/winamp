#include "./pageEmpty.h"
#include "./loginBox.h"
#include "./common.h"
#include "../resource.h"


static HRESULT CALLBACK LoginPageEmpty_CreateInstance(HWND hwnd, HWND hLoginbox, LoginPage **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == hwnd || NULL == hLoginbox) return E_INVALIDARG;

	*instance = new LoginPageEmpty(hwnd, hLoginbox);
	if (NULL == instance) return E_OUTOFMEMORY;

	return S_OK;
}

LoginPageEmpty::LoginPageEmpty(HWND hwnd, HWND hLoginbox)
	: LoginPage(hwnd, hLoginbox)
{
}

LoginPageEmpty::~LoginPageEmpty()
{
}

HWND LoginPageEmpty::CreatePage(HWND hLoginbox, HWND hParent)
{
	return LoginPage::CreatePage(hLoginbox, MAKEINTRESOURCE(IDD_PAGE_EMPTY), 
						hParent, NULL, LoginPageEmpty_CreateInstance);
}

BOOL LoginPageEmpty::OnInitDialog(HWND hFocus, LPARAM param)
{
	if (NULL != hLoginbox)
	{
		BOOL updateActive = LoginBox_GetUpdateState(hLoginbox);
		EnableCheckButton(FALSE == updateActive);
	}

	LoginPage::OnInitDialog(hFocus, param);
	return FALSE;
}

void LoginPageEmpty::OnCommand(UINT commandId, UINT eventType, HWND hControl)
{
	switch(commandId)
	{
		case IDC_CHECKNOW:
			if (BN_CLICKED == eventType && NULL != hLoginbox)
				LoginBox_UpdateProviders(hLoginbox, TRUE);
			break;
	}
	LoginPage::OnCommand(commandId, eventType, hControl);
}

void LoginPageEmpty::UpdateLayout(BOOL fRedraw)
{
	LoginPage::UpdateLayout(fRedraw);

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	RECT offsetRect;
	SetRect(&offsetRect, 2, 4, 4, 22);
	MapDialogRect(hwnd, &offsetRect);

	clientRect.left += offsetRect.left;
	clientRect.top += offsetRect.top;
	clientRect.right -= offsetRect.right;
	clientRect.bottom -= offsetRect.bottom;

	UINT flags = SWP_NOACTIVATE | SWP_NOZORDER;
	if (FALSE == fRedraw) flags |= SWP_NOREDRAW;

	const INT szControls[] = { IDC_MESSAGE, IDC_CHECKNOW, IDC_CHECK_STATUS};
	HDWP hdwp = BeginDeferWindowPos(ARRAYSIZE(szControls));

	RECT rect;
	LONG clientTop = clientRect.top;
	LONG left, top = clientRect.top;
	for (INT i = 0; i < ARRAYSIZE(szControls); i++)
	{
		HWND hControl = GetDlgItem(hwnd, szControls[i]);
		if (NULL == hControl || FALSE == GetWindowRect(hControl, &rect)) continue;
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);

		switch(szControls[i])
		{
			case IDC_MESSAGE:
				top = clientRect.top + ((clientRect.bottom - clientRect.top) - (rect.bottom - rect.top))/2;
				left = clientRect.left + ((clientRect.right - clientRect.left) - (rect.right - rect.left))/2;
				OffsetRect(&rect, left - rect.left, top - rect.top);
				clientTop = rect.bottom;
				break;
			case IDC_CHECKNOW:
				top = clientTop +  2*offsetRect.top;
				left = clientRect.left + ((clientRect.right - clientRect.left) - (rect.right - rect.left))/2;
				OffsetRect(&rect, left - rect.left, top - rect.top);
				break;
			case IDC_CHECK_STATUS:
				top = clientTop + offsetRect.top;
				left = clientRect.left + ((clientRect.right - clientRect.left) - (rect.right - rect.left))/2;
				OffsetRect(&rect, left - rect.left, top - rect.top);
				break;
		}
		
		hdwp = DeferWindowPos(hdwp, hControl, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, flags);
		if (NULL == hdwp) break;
	}

	if (NULL != hdwp)
		EndDeferWindowPos(hdwp);

}

HBRUSH LoginPageEmpty::OnGetStaticColor(HDC hdc, HWND hControl)
{
	INT_PTR controlId = (INT_PTR)GetWindowLongPtr(hControl, GWLP_ID);
	switch(controlId)
	{
		case IDC_MESSAGE:
		case IDC_CHECK_STATUS:
			SetTextColor(hdc, rgbSecondaryText);
			SetBkColor(hdc, rgbBack);
			return hbrBack;
	}

	return LoginPage::OnGetStaticColor(hdc, hControl);
}

BOOL LoginPageEmpty::OnGetLoginData(LoginData **ppLoginData)
{
	return FALSE;
}

void LoginPageEmpty::OnUpdateStateChange(BOOL updateActive)
{
	EnableCheckButton(FALSE == updateActive);
}

void LoginPageEmpty::EnableCheckButton(BOOL fEnable)
{
	HWND hButton = GetDlgItem(hwnd, IDC_CHECKNOW);
	HWND hStatus = GetDlgItem(hwnd, IDC_CHECK_STATUS);

	if (FALSE != fEnable)
	{
		if (NULL != hStatus)
			ShowWindow(hStatus, SW_HIDE);
		
		if (NULL != hButton)
		{
			EnableWindow(hButton, TRUE);
			ShowWindow(hButton, SW_SHOWNA);
		}
		
	}
	else
	{
		if (NULL  != hButton)
		{
			if (NULL != hStatus)
				ShowWindow(hButton, SW_HIDE);
			EnableWindow(hButton, FALSE);
		}
		if (NULL != hStatus)
			ShowWindow(hStatus, SW_SHOWNA);
	}
}

