#include "./pageError.h"
#include "./loginBox.h"
#include "./common.h"
#include "../resource.h"


static HRESULT CALLBACK LoginPageError_CreateInstance(HWND hwnd, HWND hLoginbox, LoginPage **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == hwnd || NULL == hLoginbox) return E_INVALIDARG;

	*instance = new LoginPageError(hwnd, hLoginbox);
	if (NULL == instance) return E_OUTOFMEMORY;

	return S_OK;
}

LoginPageError::LoginPageError(HWND hwnd, HWND hLoginbox)
	: LoginPage(hwnd, hLoginbox)
{
}

LoginPageError::~LoginPageError()
{
}

HWND LoginPageError::CreatePage(HWND hLoginbox, HWND hParent)
{
	return LoginPage::CreatePage(hLoginbox, MAKEINTRESOURCE(IDD_PAGE_ERROR), 
						hParent, NULL, LoginPageError_CreateInstance);
}

void LoginPageError::UpdateLayout(BOOL fRedraw)
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

	const INT szControls[] = { IDC_MESSAGE, };
	HDWP hdwp = BeginDeferWindowPos(ARRAYSIZE(szControls));

	RECT rect;
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
				break;
		}

		hdwp = DeferWindowPos(hdwp, hControl, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, flags);
		if (NULL == hdwp) break;
	}

	if (NULL != hdwp)
		EndDeferWindowPos(hdwp);

}

HBRUSH LoginPageError::OnGetStaticColor(HDC hdc, HWND hControl)
{
	INT_PTR controlId = (INT_PTR)GetWindowLongPtr(hControl, GWLP_ID);
	switch(controlId)
	{
		case IDC_MESSAGE:
			SetTextColor(hdc, rgbSecondaryText);
			SetBkColor(hdc, rgbBack);
			return hbrBack;
	}

	return LoginPage::OnGetStaticColor(hdc, hControl);
}

BOOL LoginPageError::OnGetLoginData(LoginData **ppLoginData)
{
	return FALSE;
}