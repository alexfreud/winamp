#ifndef NULLSOFT_AUTH_LOGIN_PAGE_INFO_HEADER
#define NULLSOFT_AUTH_LOGIN_PAGE_INFO_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginPage.h"

#define NLPIM_FIRST			(NLPM_PAGEFIRST	+ 0)

#define NLPIM_SETMESSAGE	(NLPIM_FIRST + 0)	//wParam - not used, lParam = (LPARAM)(LPCWSTR)pszMessage; Return - no return value.
#define LoginPageInfo_SetMessage(/*HWND*/ __hwnd, /*LPCWSTR*/ __pszMessage)\
		(SNDMSG((__hwnd), NLPIM_SETMESSAGE, 0, (LPARAM)(__pszMessage)))

class LoginPageInfo: public LoginPage
{
protected:
	LoginPageInfo(HWND hwnd, HWND hLoginbox);
	~LoginPageInfo();

public:
	static HWND CreatePage(HWND hLoginbox, HWND hParent);
	
protected:
	void UpdateLayout(BOOL fRedraw);

	BOOL OnInitDialog(HWND hFocus, LPARAM param);

	void OnSetMessage(LPCWSTR pszMessage);
	
	INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);


private:
	friend static HRESULT CALLBACK LoginPageInfo_CreateInstance(HWND hwnd, HWND hLoginbox, LoginPage **instance);


};

#endif //NULLSOFT_AUTH_LOGIN_PAGE_INFO_HEADER