#include "./loginCredentials.h"
#include "./common.h"

#include "../api_auth.h"


LoginCredentials::LoginCredentials(const GUID *pRealm, LPCWSTR pszName, LPCSTR pszSession, LPCSTR pszToken, __time64_t tExpire)
:	ref(1), username(NULL), sessionKey(NULL), token(NULL)
{
	realm = (NULL == pRealm) ? GUID_NULL : *pRealm;
	username = LoginBox_CopyString(pszName);
	sessionKey = LoginBox_CopyAnsiString(pszSession);
	token = LoginBox_CopyAnsiString(pszToken);
	expire = tExpire;
}

LoginCredentials::~LoginCredentials()
{
	LoginBox_FreeStringSecure(username);
	LoginBox_FreeAnsiStringSecure(sessionKey);
	LoginBox_FreeAnsiStringSecure(token);
}

HRESULT LoginCredentials::CreateInstance(const GUID *pRealm, LPCWSTR pszName, LPCSTR pszSession, LPCSTR pszToken, __time64_t tExpire, LoginCredentials **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new LoginCredentials(pRealm, pszName, pszSession, pszToken, tExpire);
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

HRESULT LoginCredentials::CreateFromAuth(api_auth *authApi, const GUID *pRealm, LoginCredentials **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	if (NULL == authApi) return E_INVALIDARG;
	
	const size_t sessionKeyMax(8192), tokenMax(8192), usernameMax(8192);
	LPSTR sessionKey = LoginBox_MallocAnsiString(sessionKeyMax);
	LPSTR token = LoginBox_MallocAnsiString(tokenMax);
	LPWSTR username = LoginBox_MallocString(usernameMax);
	__time64_t expire;

	HRESULT hr;

	if (NULL == sessionKey || NULL == token || NULL == username)
		hr = E_OUTOFMEMORY;
	else
	{
		INT result = authApi->GetCredentials((NULL != pRealm) ? *pRealm : GUID_NULL, sessionKey, sessionKeyMax, token, tokenMax, username, usernameMax, &expire);
		if (AUTH_SUCCESS == result)
		{
			hr = CreateInstance(pRealm, username, sessionKey, token, expire, instance);
		}
		else
		{
			hr = E_FAIL;
		}
	}
	
	LoginBox_FreeAnsiStringSecure(sessionKey);
	LoginBox_FreeAnsiStringSecure(token);
	LoginBox_FreeStringSecure(username);

	return hr;
}

UINT LoginCredentials::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

UINT LoginCredentials::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

GUID LoginCredentials::GetRealm()
{
	return realm;
}

__time64_t LoginCredentials::GetExpiration()
{
	return expire;
}

LPCWSTR LoginCredentials::GetUsername()
{
	return username;
}
LPCSTR LoginCredentials::GetSessionKey()
{
	return sessionKey;
}

LPCSTR LoginCredentials::GetToken()
{
	return token;
}
