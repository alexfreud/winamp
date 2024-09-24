#ifndef NULLSOFT_AUTH_LOGIN_PAGE_ADDRESS_HEADER
#define NULLSOFT_AUTH_LOGIN_PAGE_ADDRESS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginPage.h"

#define NLPAM_FIRST			(NLPM_PAGEFIRST	+ 0)

#define NLPAM_SETADDRESS		(NLPAM_FIRST + 0)	//wParam = (WPARAM)(BOOL)replaceUsername, lParam = (LPARAM)(LPCWSTR)pszAddress; Return - TRUE on succeess.
#define LoginPageAddress_SetAddress(/*HWND*/ __hwnd, /*LPCWSTR*/ __address, /*BOOL*/ __replaceUsername)\
		((BOOL)SNDMSG((__hwnd), NLPAM_SETADDRESS, (WPARAM)(__replaceUsername), (LPARAM)(__address)))

#define NLPAM_SETADDRESSTITLE	(NLPAM_FIRST + 1)	//wParam - not used, lParam = (LPARAM)(LPCWSTR)pszAddressTitle; Return - TRUE on succeess.
#define LoginPageAddress_SetAddressTitle(/*HWND*/ __hwnd, /*LPCWSTR*/ __addressTitle)\
		((BOOL)SNDMSG((__hwnd), NLPAM_SETADDRESSTITLE, 0, (LPARAM)(__addressTitle)))

#define NLPAM_SETMESSAGE		(NLPAM_FIRST + 2)	//wParam - not used, lParam = (LPARAM)(LPCWSTR)pszMessage; Return - TRUE on succeess.
#define LoginPageAddress_SetMessage(/*HWND*/ __hwnd, /*LPCWSTR*/ __pszMessage)\
		((BOOL)SNDMSG((__hwnd), NLPAM_SETMESSAGE, 0, (LPARAM)(__pszMessage)))


class LoginPageAddress : public LoginPage
{
protected:
	LoginPageAddress(HWND hwnd, HWND hLoginbox);
	~LoginPageAddress();

public:
	static HWND CreatePage(HWND hLoginbox, HWND hParent);
	
protected:
	void UpdateLayout(BOOL fRedraw);

	BOOL OnInitDialog(HWND hFocus, LPARAM param);
	void OnDestroy();

	BOOL OnGetLoginData(LoginData **ppLoginData);
	BOOL OnSetUsername(LPCWSTR pszUsername);
	
	HBRUSH OnGetStaticColor(HDC hdc, HWND hControl);

	BOOL OnSetAddress(LPCWSTR pszAddress, BOOL replaceUsername);
	BOOL OnSetAddressTitle(LPCWSTR pszTitle);
	BOOL OnSetMessage(LPCWSTR pszMessage);
	
	INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);


private:
	friend static HRESULT CALLBACK LoginPageAddress_CreateInstance(HWND hwnd, HWND hLoginbox, LoginPage **instance);


};

#endif //NULLSOFT_AUTH_LOGIN_PAGE_ADDRESS_HEADER