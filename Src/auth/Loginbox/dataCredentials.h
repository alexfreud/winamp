#ifndef NULLSOFT_AUTH_LOGINDATA_CREDENTIALS_HEADER
#define NULLSOFT_AUTH_LOGINDATA_CREDENTIALS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginData.h"

// {15D82B0E-A557-4497-808D-CB68F2C9C33A}
static const GUID IID_LoginDataCredentials = 
{ 0x15d82b0e, 0xa557, 0x4497, { 0x80, 0x8d, 0xcb, 0x68, 0xf2, 0xc9, 0xc3, 0x3a } };


class LoginDataCredentials : public LoginData
{

protected:
	LoginDataCredentials(const GUID *pRealm, HWND hPage, HWND hLoginbox, LPCWSTR pszUsername, LPCWSTR pszPassword);
	~LoginDataCredentials();

public: 
	static HRESULT CreateInstance(const GUID *pRealm, HWND hPage, HWND hLoginbox, LPCWSTR pszUsername, LPCWSTR pszPassword, LoginDataCredentials **instance);

public:
	virtual HRESULT QueryInterface(REFIID riid, void** ppObject);
	LPCWSTR GetUsername();
	LPCWSTR GetPassword();

	HRESULT SetContext(LPCSTR pszContext);
	LPCSTR GetContext();

	HRESULT SetPasscode(LPCWSTR pszPasscode);
	LPCWSTR GetPasscode();

protected:
	LPWSTR username;
	LPWSTR password;
	LPWSTR passcode;
	LPSTR context;
};

#endif //NULLSOFT_AUTH_LOGINDATA_CREDENTIALS_HEADER