#ifndef NULLSOFT_AUTH_LOGIN_CREDENTIALS_HEADER
#define NULLSOFT_AUTH_LOGIN_CREDENTIALS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class api_auth;

class LoginCredentials
{
protected:
	LoginCredentials(const GUID *pRealm, LPCWSTR pszName, LPCSTR pszSession, LPCSTR pszToken, __time64_t tExpire);
	~LoginCredentials();

public: 
	static HRESULT CreateInstance(const GUID *pRealm, LPCWSTR pszName, LPCSTR pszSession, LPCSTR pszToken, __time64_t tExpire, LoginCredentials **instance);
	static HRESULT CreateFromAuth(api_auth *authApi, const GUID *pRealm, LoginCredentials **instance);

public:
	UINT AddRef();
	UINT Release();
	
	GUID GetRealm();
	LPCWSTR GetUsername();
	LPCSTR GetSessionKey();
	LPCSTR GetToken();
	__time64_t GetExpiration();

private:
	ULONG ref;
	GUID	realm;
	LPWSTR	username;
	LPSTR	sessionKey;
	LPSTR	token;
	__time64_t expire;

};

#endif //NULLSOFT_AUTH_LOGIN_CREDENTIALS_HEADER