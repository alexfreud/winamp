#ifndef NULLSOFT_AUTH_LOGINCOMMAND_WINAMPAUTH_HEADER
#define NULLSOFT_AUTH_LOGINCOMMAND_WINAMPAUTH_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginCommand.h"

// {36B883C8-5400-4d43-89EA-96B1CBEFC605}
static const GUID LCUID_WINAMPAUTH = 
{ 0x36b883c8, 0x5400, 0x4d43, { 0x89, 0xea, 0x96, 0xb1, 0xcb, 0xef, 0xc6, 0x5 } };


class LoginCommandWinampAuth :	public LoginCommand								
{
protected:
	LoginCommandWinampAuth();
	~LoginCommandWinampAuth();

public:
	static HRESULT CreateInstance(LoginCommandWinampAuth **instance);

public:
	ULONG AddRef();
	ULONG Release();

	HRESULT GetType(GUID *commandUid);

	HRESULT SetParameter(LPCWSTR pszKey, LPCWSTR pszValue);
	HRESULT IsValid();
	HRESULT IsIdentical(LoginCommand *test);


	HRESULT BeginLogin(LoginData *data, LoginResult::Callback callback, void *user, LoginResult **result);
	HRESULT EndLogin(LoginResult *result, INT *authCode, LoginCredentials **credentials);
	HRESULT RequestAbort(LoginResult *result, BOOL drop);

protected:
	ULONG ref;

};

#endif //NULLSOFT_AUTH_LOGINCOMMAND_WINAMPAUTH_HEADER