#ifndef NULLSOFT_AUTH_LOGIN_PAGE_CREDENTIALS_HEADER
#define NULLSOFT_AUTH_LOGIN_PAGE_CREDENTIALS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginPage.h"

#define NLPCM_FIRST			(NLPM_PAGEFIRST	+ 0)

#define NLPCM_SETACCOUNTRECOVERURL	(NLPCM_FIRST + 1)	//wParam - not used, lParam = (LPARAM)(LPCWSTR)pszAccountRecoverUrl; Return - no return value.
#define LoginPageCredentials_SetAccountRecoverUrl(/*HWND*/ __hwnd, /*LPCWSTR*/ __url)\
		(SNDMSG((__hwnd), NLPCM_SETACCOUNTRECOVERURL, 0, (LPARAM)(__url)))

#define NLPCM_SETACCOUNTCREATEURL	(NLPCM_FIRST + 2)	//wParam - not used, lParam = (LPARAM)(LPCWSTR)pszAccountCreateUrl; Return - no return value.
#define LoginPageCredentials_SetAccountCreateUrl(/*HWND*/ __hwnd, /*LPCWSTR*/ __url)\
		(SNDMSG((__hwnd), NLPCM_SETACCOUNTCREATEURL, 0, (LPARAM)(__url)))

#define NLPCM_SETUSERNAMELABEL		(NLPCM_FIRST + 3)	//wParam - not used, lParam = (LPARAM)(LPCWSTR)pszUsernameLabel; Return - no return value.
#define LoginPageCredentials_SetUsernameLabel(/*HWND*/ __hwnd, /*LPCWSTR*/ __label)\
		(SNDMSG((__hwnd), NLPCM_SETUSERNAMELABEL, 0, (LPARAM)(__label)))

#define NLPCM_SETPASSWORDLABEL		(NLPCM_FIRST + 4)	//wParam - not used, lParam = (LPARAM)(LPCWSTR)pszPasswordLabel; Return - no return value.
#define LoginPageCredentials_SetPasswordLabel(/*HWND*/ __hwnd, /*LPCWSTR*/ __label)\
		(SNDMSG((__hwnd), NLPCM_SETPASSWORDLABEL, 0, (LPARAM)(__label)))

class LoginPageCredentials : public LoginPage
{
protected:
	LoginPageCredentials(HWND hwnd, HWND hLoginbox);
	~LoginPageCredentials();

public:
	static HWND CreatePage(HWND hLoginbox,  HWND hParent);
	
protected:
	void UpdateLayout(BOOL fRedraw);

	BOOL OnInitDialog(HWND hFocus, LPARAM param);
	BOOL OnNotify(UINT controlId, const NMHDR *pnmh);

	BOOL OnGetLoginData(LoginData **ppLoginData);
	BOOL OnSetUsername(LPCWSTR pszUsername);
	BOOL OnSetPassword(LPCWSTR pszPassword);
	HWND OnGetFirstItem();

	void OnSetAccountRecoverUrl(LPCWSTR pszUrl);
	void OnSetAccountCreateUrl(LPCWSTR pszUrl);
	void OnSetUsernameLabel(LPCWSTR pszLabel);
	void OnSetPasswordLabel(LPCWSTR pszLabel);
	

	INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);


private:
	friend static HRESULT CALLBACK LoginPageCredentials_CreateInstance(HWND hwnd, HWND hLoginbox, LoginPage **instance);

protected:
	LPWSTR accountRecoverUrl;
	LPWSTR accountCreateUrl;

};

#endif //NULLSOFT_AUTH_LOGIN_PAGE_CREDENTIALS_HEADER