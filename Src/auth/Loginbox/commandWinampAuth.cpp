#include "./commandWinampAuth.h"
#include "./resultWinampAuth.h"


LoginCommandWinampAuth::LoginCommandWinampAuth() 
	: ref(1)
{	
}

LoginCommandWinampAuth::~LoginCommandWinampAuth()
{

}

HRESULT LoginCommandWinampAuth::CreateInstance(LoginCommandWinampAuth **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new LoginCommandWinampAuth();
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

ULONG LoginCommandWinampAuth::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG LoginCommandWinampAuth::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT LoginCommandWinampAuth::GetType(GUID *commandUid)
{
	if (NULL == commandUid) return E_INVALIDARG;
	*commandUid = LCUID_WINAMPAUTH;
	return S_OK;
}

HRESULT LoginCommandWinampAuth::SetParameter(LPCWSTR pszKey, LPCWSTR pszValue)
{
	return E_NOTIMPL;
}

HRESULT LoginCommandWinampAuth::IsValid()
{
	return S_OK;
}

HRESULT LoginCommandWinampAuth::IsIdentical(LoginCommand *test)
{
	if (NULL == test)
		return E_INVALIDARG;

	GUID typeId;
	if (FAILED(test->GetType(&typeId)) || FALSE == IsEqualGUID(LCUID_WINAMPAUTH, typeId))
		return S_FALSE;

	return S_OK;
}


HRESULT LoginCommandWinampAuth::BeginLogin(LoginData *data, LoginResult::Callback callback, void *user, LoginResult **result)
{
	LoginResultWinampAuth *winampAuth;
	HRESULT hr = LoginResultWinampAuth::CreateInstance(data, callback, user, &winampAuth);
	if (SUCCEEDED(hr))
	{
		if (NULL != result)
			*result = winampAuth;
		else
			winampAuth->Release();
	}
	else
	{
		if (NULL != result) 
			*result = NULL;
	}

	return hr;
}

HRESULT LoginCommandWinampAuth::EndLogin(LoginResult *result, INT *authCode, LoginCredentials **credentials)
{
	if (NULL == result)
		return E_INVALIDARG;

	HRESULT hr = result->IsCompleted();
	if (S_OK != hr)
	{
		HANDLE completed;
		hr = result->GetWaitHandle(&completed);
		if (SUCCEEDED(hr))
		{
			WaitForSingleObjectEx(completed, INFINITE, TRUE);
			CloseHandle(completed);
		}
	}
	
	if (SUCCEEDED(hr))
	{
		LoginResultWinampAuth *winampAuth;
		hr = result->QueryInterface(LCUID_WINAMPAUTH, (void**)&winampAuth);
		if(SUCCEEDED(hr))
		{
			hr = winampAuth->GetResult(authCode, credentials);
			winampAuth->Release();
		}
	}

	return hr;
}

HRESULT LoginCommandWinampAuth::RequestAbort(LoginResult *result, BOOL drop)
{
	if (NULL == result) return E_INVALIDARG;
	return result->RequestAbort(drop);
}
