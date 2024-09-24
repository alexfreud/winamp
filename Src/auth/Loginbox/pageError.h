#ifndef NULLSOFT_AUTH_LOGIN_PAGE_ERROR_HEADER
#define NULLSOFT_AUTH_LOGIN_PAGE_ERROR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginPage.h"

class LoginPageError : public LoginPage
{
protected:
	LoginPageError(HWND hwnd, HWND hLoginbox);
	~LoginPageError();

public:
	static HWND CreatePage(HWND hLoginbox, HWND hParent);
	
protected:
	void UpdateLayout(BOOL fRedraw);

	HBRUSH OnGetStaticColor(HDC hdc, HWND hControl);
	BOOL OnGetLoginData(LoginData **ppLoginData);
		
private:
	friend static HRESULT CALLBACK LoginPageError_CreateInstance(HWND hwnd, HWND hLoginbox, LoginPage **instance);


};

#endif //NULLSOFT_AUTH_LOGIN_PAGE_ERROR_HEADER