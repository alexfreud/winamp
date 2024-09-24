#ifndef NULLSOFT_AUTH_LOGINCOMMAND_WEBAUTH_HEADER
#define NULLSOFT_AUTH_LOGINCOMMAND_WEBAUTH_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginCommand.h"

// {48F006E2-EC11-4171-833B-A9CD14F6D727}
static const GUID LCUID_WEBAUTH = 
{ 0x48f006e2, 0xec11, 0x4171, { 0x83, 0x3b, 0xa9, 0xcd, 0x14, 0xf6, 0xd7, 0x27 } };

class LoginCommandWebAuth :	public LoginCommand								
{
protected:
	LoginCommandWebAuth();
	~LoginCommandWebAuth();

public:
	static HRESULT CreateInstance(LoginCommandWebAuth **instance);

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
	LPWSTR targetUrl;
};

#endif //NULLSOFT_AUTH_LOGINCOMMAND_WEBAUTH_HEADER