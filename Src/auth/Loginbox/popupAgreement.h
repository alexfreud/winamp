#ifndef NULLSOFT_AUTH_LOGINPOPUP_AGREEMENT_HEADER
#define NULLSOFT_AUTH_LOGINPOPUP_AGREEMENT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginPopup.h"

class LoginProvider;

class LoginPopupAgreement  : public LoginPopup
{
protected:
	LoginPopupAgreement(HWND hwnd);
	~LoginPopupAgreement();

public:
	static HWND CreatePopup(HWND hParent, LoginProvider *provider);

protected:
	void UpdateLayout(BOOL fRedraw);
	void EndDialog(INT_PTR code);
	void UpdateMargins();

	BOOL CreateProviderLinks(LPCWSTR pszProvider, LPCWSTR pszTos, LPCWSTR pszPrivacy, INT groupId, HWND hwndInsertAfter);
	HDWP LayoutProviderLinks(HDWP hdwp, INT groupId, HWND hwndInsertAfter, INT x, INT y, UINT flags, SIZE *size); // pass hdwp = NULL to get ideal size
	

	void OnLinkClicked(HWND hLink);
	HBRUSH OnGetStaticColor(HDC hdc, HWND hControl);
	LRESULT OnNotify(UINT controlId, const NMHDR *pnmh);
	void OnParentNotify(UINT eventId, UINT wParam, LPARAM lParam);
	BOOL OnUpdateWindowPos(const RECT* clientRect, RECT *rectOut);
	BOOL OnInitDialog(HWND hFocus, LPARAM param);
	void OnDestroy();

protected:
	LONG marginLinkLeft;
	LONG marginLinkFirst;
	LONG marginLinkNext;

private:
	friend static HRESULT CALLBACK LoginPopupAgreement_CreateInstance(HWND hwnd, LPARAM param, LoginPopup **instance);
};


#endif //NULLSOFT_AUTH_LOGINPOPUP_AGREEMENT_HEADER