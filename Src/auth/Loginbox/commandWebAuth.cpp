#include "./commandWebAuth.h"
#include "./resultWebAuth.h"
#include "./common.h"

#include "../api.h"

#include "../../omBrowser/obj_ombrowser.h"
#include <api/service/waservicefactory.h>

LoginCommandWebAuth::LoginCommandWebAuth() 
	: ref(1), targetUrl(NULL)
{
	
}

LoginCommandWebAuth::~LoginCommandWebAuth()
{
	LoginBox_FreeString(targetUrl);
}

HRESULT LoginCommandWebAuth::CreateInstance(LoginCommandWebAuth **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new LoginCommandWebAuth();
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

ULONG LoginCommandWebAuth::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG LoginCommandWebAuth::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT LoginCommandWebAuth::GetType(GUID *commandUid)
{
	if (NULL == commandUid) return E_INVALIDARG;
	*commandUid = LCUID_WEBAUTH;
	return S_OK;
}

HRESULT LoginCommandWebAuth::SetParameter(LPCWSTR pszKey, LPCWSTR pszValue)
{
	if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, pszKey, -1, L"url", -1))
	{
		LoginBox_FreeString(targetUrl);
		targetUrl = LoginBox_CopyString(pszValue);
	}
	return S_OK;
}

HRESULT LoginCommandWebAuth::IsValid()
{
	if (NULL == targetUrl || L'\0' == *targetUrl)
		return S_FALSE;

	HRESULT hr = S_FALSE;
	
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(OBJ_OmBrowser);
	if (NULL != sf)
	{
		obj_ombrowser *browserMngr = (obj_ombrowser*)sf->getInterface();
		if (NULL != browserMngr) 
		{
			hr = S_OK;
			browserMngr->Release();
		}
		sf->Release();
	}
	return hr;
}

HRESULT LoginCommandWebAuth::IsIdentical(LoginCommand *test)
{
	if (NULL == test)
		return E_INVALIDARG;

	GUID typeId;
	if (FAILED(test->GetType(&typeId)) || FALSE == IsEqualGUID(LCUID_WEBAUTH, typeId))
		return S_FALSE;
	
	LoginCommandWebAuth *testWeb = (LoginCommandWebAuth*)test;

	if(S_OK != LoginBox_IsStrEqInvI(targetUrl, testWeb->targetUrl))
		return S_FALSE;
	
	return S_OK;
}

HRESULT LoginCommandWebAuth::BeginLogin(LoginData *data, LoginResult::Callback callback, void *user, LoginResult **result)
{
	HRESULT hr;
	LoginResultWebAuth *webAuth;

	hr = LoginResultWebAuth::CreateInstance(targetUrl, data, callback, user, &webAuth);
	
	if (SUCCEEDED(hr))
	{
		if (NULL != result)
			*result = webAuth;
		else
			webAuth->Release();
	}
	else
	{
		if (NULL != result) 
			*result = NULL;
	}

	return hr;
}

HRESULT LoginCommandWebAuth::EndLogin(LoginResult *result, INT *authCode, LoginCredentials **credentials)
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
		LoginResultWebAuth *webAuth;
		hr = result->QueryInterface(LCUID_WEBAUTH, (void**)&webAuth);
		if(SUCCEEDED(hr))
		{
			hr = webAuth->GetResult(authCode, credentials);
			webAuth->Release();
		}
	}

	return hr;
}

HRESULT LoginCommandWebAuth::RequestAbort(LoginResult *result, BOOL drop)
{
	if (NULL == result) return E_INVALIDARG;
	return result->RequestAbort(drop);
}
