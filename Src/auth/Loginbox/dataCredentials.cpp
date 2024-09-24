#include "./dataCredentials.h"
#include "./common.h"

#include <strsafe.h>

LoginDataCredentials::LoginDataCredentials(const GUID *pRealm, HWND hPage, HWND hLoginbox, LPCWSTR pszUsername, LPCWSTR pszPassword)
	: LoginData(pRealm, hPage, hLoginbox), username(NULL), password(NULL), context(NULL), passcode(NULL)
{
	username = LoginBox_CopyString(pszUsername);
	password = LoginBox_CopyString(pszPassword);
}

LoginDataCredentials::~LoginDataCredentials()
{	
	LoginBox_FreeStringSecure(username);
	LoginBox_FreeStringSecure(password);
	LoginBox_FreeStringSecure(passcode);
	LoginBox_FreeAnsiStringSecure(context);
}

HRESULT LoginDataCredentials::CreateInstance(const GUID *pRealm, HWND hPage, HWND hLoginbox, LPCWSTR pszUsername, LPCWSTR pszPassword, LoginDataCredentials **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == hPage || NULL == hLoginbox) return E_INVALIDARG;
	*instance = new LoginDataCredentials(pRealm, hPage, hLoginbox, pszUsername, pszPassword);
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

HRESULT LoginDataCredentials::QueryInterface(REFIID riid, void** ppObject)
{
	if (NULL == ppObject) 
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_LoginDataCredentials))
	{
		*ppObject = static_cast<LoginDataCredentials*>(this);
		if (NULL == *ppObject) return E_UNEXPECTED;
		AddRef();
		return S_OK;
	}
	
	return __super::QueryInterface(riid, ppObject);
}

LPCWSTR LoginDataCredentials::GetUsername()
{
	return username;
}

LPCWSTR LoginDataCredentials::GetPassword()
{
	return password;
}

HRESULT LoginDataCredentials::SetContext(LPCSTR pszContext)
{
	LoginBox_FreeAnsiStringSecure(context);
	if (NULL == pszContext)
	{
		context = NULL;
	}
	else
	{
		context = LoginBox_CopyAnsiString(pszContext);
		if (NULL == context) 
			return E_OUTOFMEMORY;
	}

	return S_OK;
}

LPCSTR LoginDataCredentials::GetContext()
{
	return context;
}
	
HRESULT LoginDataCredentials::SetPasscode(LPCWSTR pszPasscode)
{
	LoginBox_FreeStringSecure(passcode);
	if (NULL == pszPasscode)
	{
		passcode = NULL;
	}
	else
	{
		passcode = LoginBox_CopyString(pszPasscode);
		if (NULL == passcode) 
			return E_OUTOFMEMORY;
	}

	return S_OK;
}

LPCWSTR LoginDataCredentials::GetPasscode()
{
	return passcode;
}