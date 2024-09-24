#include "./pageInfo.h"
#include "./common.h"
#include "./loginGui.h"
#include "../resource.h"


static HRESULT CALLBACK LoginPageInfo_CreateInstance(HWND hwnd, HWND hLoginbox, LoginPage **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == hwnd || NULL == hLoginbox) return E_INVALIDARG;

	*instance = new LoginPageInfo(hwnd, hLoginbox);
	if (NULL == instance) return E_OUTOFMEMORY;

	return S_OK;
}

LoginPageInfo::LoginPageInfo(HWND hwnd, HWND hLoginbox)
	: LoginPage(hwnd, hLoginbox)
{
}

LoginPageInfo::~LoginPageInfo()
{
}

HWND LoginPageInfo::CreatePage(HWND hLoginbox, HWND hParent)
{
	return LoginPage::CreatePage(hLoginbox, MAKEINTRESOURCE(IDD_PAGE_INFO), 
						hParent, NULL, LoginPageInfo_CreateInstance);
}

void LoginPageInfo::UpdateLayout(BOOL fRedraw)
{
	LoginPage::UpdateLayout(fRedraw);

	RECT pageRect;
	if (FALSE == GetPageRect(&pageRect))
		return;

	
	HWND hMessage = GetDlgItem(hwnd, IDC_MESSAGE);
	if (NULL == hMessage) return;

	INT cx, cy;
	cx = pageRect.right - pageRect.left;

	HDC hdc = GetDCEx(hMessage, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL != hdc)
	{
		HFONT fontControl = (HFONT)SendMessage(hMessage, WM_GETFONT, 0, 0L);
		HFONT fontOrig = (HFONT)SelectObject(hdc, fontControl);
		
		INT maxWidth = LoginBox_GetAveStrWidth(hdc, 80);
		if (cx > maxWidth) cx = maxWidth;

		SelectObject(hdc, fontOrig);
		ReleaseDC(hMessage, hdc);
	}

	LoginBox_GetWindowTextSize(hMessage, cx, &cx, &cy);
	if(cy > (pageRect.bottom - pageRect.top))
		cy = pageRect.bottom - pageRect.top;

	RECT messageRect;
	GetWindowRect(hMessage, &messageRect);
	MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&messageRect, 2);
	if (messageRect.left != pageRect.left ||
		messageRect.top != pageRect.top ||
		(messageRect.right - messageRect.left) != cx ||
		(messageRect.bottom - messageRect.top) != cy)
	{
		
		SetWindowPos(hMessage, NULL, pageRect.left, pageRect.top, cx, cy, 
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
		
		if (FALSE != fRedraw)
		{
			HRGN rgn1 = CreateRectRgnIndirect(&messageRect);
			HRGN rgn2 = CreateRectRgn(pageRect.left, pageRect.top, pageRect.left + cx, pageRect.top + cy);
			CombineRgn(rgn1, rgn1, rgn2, RGN_OR);
			RedrawWindow(hwnd, NULL, rgn1, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ERASENOW | RDW_VALIDATE | RDW_ALLCHILDREN);
			DeleteObject(rgn1);
			DeleteObject(rgn2);
		}
	}

}

BOOL LoginPageInfo::OnInitDialog(HWND hFocus, LPARAM param)
{	
	HFONT fontText = NULL;
	LoginGuiObject *loginGui;
	if (SUCCEEDED(LoginGuiObject::QueryInstance(&loginGui)))
	{
		fontText = loginGui->GetTextFont();
		loginGui->Release();
	}

	if (NULL != fontText)
	{
		HWND hMessage = GetDlgItem(hwnd, IDC_MESSAGE);
		if (NULL != hMessage)
			SendMessage(hMessage, WM_SETFONT, (WPARAM)fontText, 0L);
	}
	
	LoginPage::OnInitDialog(hFocus, param);
	return FALSE;
}


void LoginPageInfo::OnSetMessage(LPCWSTR pszMessage)
{
	HWND hMessage = GetDlgItem(hwnd, IDC_MESSAGE);
	if (NULL != hMessage)
	{
		SetWindowText(hMessage, pszMessage);
		INT showWindow = (NULL != pszMessage && L'\0' != *pszMessage) ? SW_SHOWNA : SW_HIDE;
		ShowWindow(hMessage, showWindow);
	}
}

INT_PTR LoginPageInfo::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case NLPIM_SETMESSAGE:		OnSetMessage((LPCWSTR)lParam); return TRUE;
	}
	return __super::DialogProc(uMsg, wParam, lParam);
}

