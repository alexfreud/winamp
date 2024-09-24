#ifndef NULLSOFT_AUTH_LOGINCOMMAND_HEADER
#define NULLSOFT_AUTH_LOGINCOMMAND_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./loginResult.h"

class LoginData;
class LoginCredentials;

class __declspec(novtable) LoginCommand
{
protected:
	LoginCommand() {}
	~LoginCommand() {}

public:
	virtual ULONG AddRef() = 0;
	virtual ULONG Release() = 0;

	virtual HRESULT GetType(GUID *commandUid) = 0;

	virtual HRESULT SetParameter(LPCWSTR pszKey, LPCWSTR pszValue) = 0;
	virtual HRESULT IsValid() = 0;
	virtual HRESULT IsIdentical(LoginCommand *test) = 0;

	virtual HRESULT BeginLogin(LoginData *data, LoginResult::Callback callback, void *user, LoginResult **result) = 0;
	virtual HRESULT EndLogin(LoginResult *result, INT *authCode, LoginCredentials **credentials) = 0;
	virtual HRESULT RequestAbort(LoginResult *result, BOOL drop) = 0;

};

#endif //NULLSOFT_AUTH_LOGINCOMMAND_HEADER