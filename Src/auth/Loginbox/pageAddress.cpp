#include "./pageAddress.h"
#include "./dataAddress.h"
#include "./common.h"
#include "./addressEncoder.h"
#include "./addressEditbox.h"
#include "./loginGui.h"

#include "../resource.h"

#include <shlwapi.h>
#include <wininet.h>



static HRESULT CALLBACK LoginPageAddress_CreateInstance(HWND hwnd, HWND hLoginbox, LoginPage **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == hwnd || NULL == hLoginbox) return E_INVALIDARG;

	*instance = new LoginPageAddress(hwnd, hLoginbox);
	if (NULL == instance) return E_OUTOFMEMORY;

	return S_OK;
}

LoginPageAddress::LoginPageAddress(HWND hwnd, HWND hLoginbox)
	: LoginPage(hwnd, hLoginbox)
{
	HWND hEdit = GetDlgItem(hwnd, IDC_ADDRESS_EDIT);
	if (NULL != hEdit)
		AddressEditbox_AttachWindow(hEdit);

}

LoginPageAddress::~LoginPageAddress()
{
}

HWND LoginPageAddress::CreatePage(HWND hLoginbox, HWND hParent)
{
	return LoginPage::CreatePage(hLoginbox, MAKEINTRESOURCE(IDD_PAGE_ADDRESS), 
						hParent, NULL, LoginPageAddress_CreateInstance);
}

BOOL LoginPageAddress::OnInitDialog(HWND hFocus, LPARAM param)
{
	HFONT fontEdit = NULL, fontLabel = NULL;
	LoginGuiObject *loginGui;
	if (SUCCEEDED(LoginGuiObject::QueryInstance(&loginGui)))
	{
		fontLabel = loginGui->GetTextFont();
		fontEdit = loginGui->GetEditorFont();
		loginGui->Release();
	}

	if (NULL != fontLabel)
	{
		HWND hLabel = GetDlgItem(hwnd, IDC_ADDRESS_LABEL);
		if (NULL != hLabel)
			SendMessage(hLabel, WM_SETFONT, (WPARAM)fontLabel, 0L);

		hLabel = GetDlgItem(hwnd, IDC_MESSAGE);
		if (NULL != hLabel)
			SendMessage(hLabel, WM_SETFONT, (WPARAM)fontLabel, 0L);
	}

	if (NULL != fontEdit)
	{
		HWND hEdit = GetDlgItem(hwnd, IDC_ADDRESS_EDIT);
		if (NULL != hEdit)
			SendMessage(hEdit, WM_SETFONT, (WPARAM)fontEdit, 0L);
	}

	LoginPage::OnInitDialog(hFocus, param);
	return FALSE;
}

void LoginPageAddress::OnDestroy()
{
	LoginPage::OnDestroy();
}

void LoginPageAddress::UpdateLayout(BOOL fRedraw)
{
	LoginPage::UpdateLayout(fRedraw);

	RECT pageRect;
	GetPageRect(&pageRect);
	
	UINT flags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW;
	
	const INT szControls[] = { IDC_ADDRESS_LABEL, IDC_ADDRESS_EDIT, IDC_MESSAGE, };
	HDWP hdwp = BeginDeferWindowPos(ARRAYSIZE(szControls));

	INT baseunitX, baseunitY;
	if (FALSE == LoginBox_GetWindowBaseUnits(hwnd, &baseunitX, &baseunitY))
	{
		baseunitY = 13;
	}

	HRGN invalidRegion = CreateRectRgn(0, 0, 0, 0);
	HRGN tempRegion = CreateRectRgn(0, 0, 0, 0);
	HWND hControl;

	INT nextTop = pageRect.top;
	INT x, y, cx, cy;

	RECT rect;

	INT maxWidth = pageRect.right - pageRect.left;
	hControl = GetDlgItem(hwnd, IDC_ADDRESS_EDIT);
	if (NULL != hControl)
	{
		HDC hdc = GetDCEx(hControl, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL != hdc)
		{
			HFONT fontControl = (HFONT)SendMessage(hControl, WM_GETFONT, 0, 0L);
			HFONT fontOrig = (HFONT)SelectObject(hdc, fontControl);
			
			maxWidth = LoginBox_GetAveStrWidth(hdc, 80);
			if(maxWidth > (pageRect.right - pageRect.left))
				maxWidth = pageRect.right - pageRect.left;

			SelectObject(hdc, fontOrig);
			ReleaseDC(hControl, hdc);
		}
	}

	for (INT i = 0; i < ARRAYSIZE(szControls); i++)
	{
		hControl = GetDlgItem(hwnd, szControls[i]);
		if (NULL == hControl || FALSE == GetWindowRect(hControl, &rect)) continue;
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);
		x = rect.left;
		y = rect.top;
		cx = rect.right - rect.left;
		cy = rect.bottom - rect.top;
		switch(szControls[i])
		{
			case IDC_ADDRESS_LABEL:
			case IDC_MESSAGE:
				x = pageRect.left;
				y = nextTop;
				LoginBox_GetWindowTextSize(hControl, maxWidth, &cx, &cy);
				nextTop += (cy + MulDiv(2, baseunitY, 8));
				break;
			case IDC_ADDRESS_EDIT:
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
				cx = maxWidth;
				nextTop += (cy + MulDiv(7, baseunitY, 8));
				break;
		}

		if (rect.left != x || rect.top != y ||
			(rect.right - rect.left) != cx || (rect.bottom - rect.top) != cy)
		{
			hdwp = DeferWindowPos(hdwp, hControl, NULL, x, y, cx, cy, flags);
			if (NULL == hdwp) break;

			if (FALSE != fRedraw)
			{
				SetRectRgn(tempRegion, rect.left, rect.top, rect.right, rect.bottom);
				CombineRgn(invalidRegion, invalidRegion, tempRegion, RGN_OR);

				SetRectRgn(tempRegion, x, y , x + cx, y + cy);
				CombineRgn(invalidRegion, invalidRegion, tempRegion, RGN_OR);
			}
		}
	}

	if (NULL != hdwp)
		EndDeferWindowPos(hdwp);

	if (FALSE != fRedraw)
	{
		RedrawWindow(hwnd, NULL, invalidRegion, 
			RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN | RDW_VALIDATE);
	}

	if (NULL != invalidRegion)
		DeleteObject(invalidRegion);
	if (NULL != tempRegion)
		DeleteObject(tempRegion);
}

BOOL LoginPageAddress::OnGetLoginData(LoginData **ppLoginData)
{
	if (NULL == ppLoginData)
		return FALSE;

	HWND hEdit;
	LPWSTR address;

	hEdit  = GetDlgItem(hwnd, IDC_ADDRESS_EDIT);
	if (NULL == hEdit || FAILED(LoginBox_GetWindowText(hEdit, &address, NULL)))
		address = NULL;
	else
	{
		size_t encodedMax = lstrlen(address)*2;
		LPWSTR addressEncoded = LoginBox_MallocString(encodedMax);

		for(;;)
		{
			if (FALSE == InternetCanonicalizeUrl(address, addressEncoded, (DWORD*)&encodedMax, ICU_ENCODE_PERCENT | ICU_NO_META))
			{
				if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
				{
					LoginBox_FreeString(addressEncoded);
					addressEncoded = LoginBox_MallocString(encodedMax);
					if (NULL != addressEncoded)
						continue;
				}
			}
			else
			{
				LoginBox_FreeString(address);
				address = LoginBox_CopyString(addressEncoded);
			}
			break;
		}

		HRESULT hr;
		for(;;)
		{
			hr = AddressEncoder_EncodeString(address, addressEncoded, &encodedMax, ICU_BROWSER_MODE);
			if (ENC_E_INSUFFICIENT_BUFFER == hr)
			{
				LoginBox_FreeString(addressEncoded);
				addressEncoded = LoginBox_MallocString(encodedMax);
				if (NULL == addressEncoded)
					hr = E_OUTOFMEMORY;
				else
					continue;
			}
			break;
		} 

		if (SUCCEEDED(hr))
		{
			LoginBox_FreeString(address);
			address = addressEncoded;
			addressEncoded = NULL;
		}
		else
			LoginBox_FreeString(addressEncoded);
	}

	HRESULT hr = LoginDataAddress::CreateInstance(NULL, hwnd, hLoginbox, 
					address, (LoginDataAddress**)ppLoginData);

	LoginBox_FreeString(address);
	return SUCCEEDED(hr);
}

BOOL LoginPageAddress::OnSetUsername(LPCWSTR pszUsername)
{
	HWND hEdit = GetDlgItem(hwnd, IDC_ADDRESS_EDIT);
	if (NULL == hEdit) return FALSE;

	if (NULL == pszUsername || L'\0' == *pszUsername)
		return FALSE;

	INT cchLen = lstrlen(pszUsername);

	INT f, l;
	SNDMSG(hEdit, EM_GETSEL, (WPARAM)&f, (LPARAM)&l);
	if (f == l) return FALSE;
	
	SNDMSG(hEdit, EM_REPLACESEL, FALSE, (LPARAM)pszUsername);
	SNDMSG(hEdit, EM_SETSEL, (WPARAM)f, (LPARAM)f + cchLen);

	return TRUE;

}
HBRUSH LoginPageAddress::OnGetStaticColor(HDC hdc, HWND hControl)
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

BOOL LoginPageAddress::OnSetAddress(LPCWSTR pszAddress, BOOL replaceUsername)
{
	HWND hEdit = GetDlgItem(hwnd, IDC_ADDRESS_EDIT);
	if (NULL == hEdit) return FALSE;
	
	BOOL succeeded;
	LPWSTR addressDecoded;

	if (SUCCEEDED(AddressEncoder_DecodeString(pszAddress, &addressDecoded)))
	{
		succeeded = (BOOL)SNDMSG(hEdit, WM_SETTEXT, 0, (LPARAM)addressDecoded);
		LoginBox_FreeString(addressDecoded);
	}
	else
		succeeded = (BOOL)SNDMSG(hEdit, WM_SETTEXT, 0, (LPARAM)pszAddress);


	return succeeded;
}

BOOL LoginPageAddress::OnSetAddressTitle(LPCWSTR pszTitle)
{
	return SetLabelText(IDC_ADDRESS_LABEL, pszTitle);
}

BOOL LoginPageAddress::OnSetMessage(LPCWSTR pszMessage)
{
	HWND hLabel = GetDlgItem(hwnd, IDC_MESSAGE);
	if (NULL == hLabel) return FALSE;
	
	return SetWindowText(hLabel, pszMessage);
}

INT_PTR LoginPageAddress::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case NLPAM_SETADDRESS:		MSGRESULT(hwnd, OnSetAddress((LPCWSTR)lParam, (BOOL)wParam));
		case NLPAM_SETADDRESSTITLE:	MSGRESULT(hwnd, OnSetAddressTitle((LPCWSTR)lParam));
		case NLPAM_SETMESSAGE:		MSGRESULT(hwnd, OnSetMessage((LPCWSTR)lParam));

	}
	return __super::DialogProc(uMsg, wParam, lParam);
}

