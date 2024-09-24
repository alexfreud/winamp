#ifndef NULLSOFT_AUTH_LOGIN_PAGE_EMPTY_HEADER
#define NULLSOFT_AUTH_LOGIN_PAGE_EMPTY_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginPage.h"

class LoginPageEmpty: public LoginPage
{
protected:
	LoginPageEmpty(HWND hwnd, HWND hLoginbox);
	~LoginPageEmpty();

public:
	static HWND CreatePage(HWND hLoginbox, HWND hParent);
	
protected:
	void UpdateLayout(BOOL fRedraw);
	void EnableCheckButton(BOOL fEnable);

	BOOL OnInitDialog(HWND hFocus, LPARAM param);
	void OnCommand(UINT commandId, UINT eventType, HWND hControl);
	
	BOOL OnGetLoginData(LoginData **ppLoginData);
	void OnUpdateStateChange(BOOL updateActive);

	HBRUSH OnGetStaticColor(HDC hdc, HWND hControl);
	
private:
	friend static HRESULT CALLBACK LoginPageEmpty_CreateInstance(HWND hwnd, HWND hLoginbox, LoginPage **instance);


};

#endif //NULLSOFT_AUTH_LOGIN_PAGE_EMPTY_HEADER