#include "./loginData.h"
#include "./loginBox.h"
#include "./loginProvider.h"
#include "./loginStatus.h"

#include "../api.h"

LoginData::LoginData(const GUID *pRealm, HWND hPage, HWND hLoginbox)
	: ref(1), hPage(hPage), hLoginbox(hLoginbox), provider(NULL), status(NULL), statusCookie(-1)
{
	if (NULL != pRealm)
	{
		realm = *pRealm;
	}
	else
	{
		if (NULL == hLoginbox || FALSE == LoginBox_GetRealm(hLoginbox, &realm))
			realm = GUID_NULL;
	}

	if (FALSE == LoginBox_GetActiveProvider(hLoginbox, &provider))
		provider = NULL;

	LoginBox_GetStatus(hLoginbox, &status);
}

LoginData::~LoginData()
{
	if (NULL != provider)
		provider->Release();

	if (NULL != status)
	{
		if (-1 != statusCookie)
			status->Remove(statusCookie);

		status->Release();
	}
}

HRESULT LoginData::CreateInstance(const GUID *pRealm, HWND hPage, HWND hLoginbox, LoginData **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == hPage || NULL == hLoginbox) return E_INVALIDARG;
	*instance = new LoginData(pRealm, hPage, hLoginbox);
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

ULONG LoginData::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG LoginData::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT LoginData::QueryInterface(REFIID riid, void** ppObject)
{
	if (NULL == ppObject) 
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_LoginData))
		*ppObject = static_cast<LoginData*>(this);
	else
	{
		*ppObject = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *ppObject)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HWND LoginData::GetLoginbox()
{
	return hLoginbox;
}

HWND LoginData::GetPage()
{
	return hPage;
}

HRESULT LoginData::GetRealm(GUID *pRealm)
{
	if (NULL == pRealm) return E_POINTER;
	*pRealm = realm;
	return S_OK;
}

HRESULT LoginData::GetProvider(LoginProvider **ppProvider)
{
	if (NULL == ppProvider) return E_POINTER;
	*ppProvider = provider;
	if (NULL != provider)
		provider->AddRef();
	return S_OK;
}

HRESULT LoginData::GetStatus(LoginStatus **ppStatus)
{
	if (NULL == ppStatus) return E_POINTER;
	*ppStatus = status;
	if (NULL != status)
		status->AddRef();
	return S_OK;
}
HRESULT LoginData::SetStatus(LPCWSTR pszStatus)
{
	if (NULL == status)
		return E_FAIL;

	BSTR bstrText;
	if (NULL == pszStatus || FALSE == IS_INTRESOURCE(pszStatus))
		bstrText = SysAllocString(pszStatus);
	else
	{
		WCHAR szBuffer[256] = {0};
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszStatus, szBuffer, ARRAYSIZE(szBuffer));
		bstrText = SysAllocString(szBuffer);
	}

	if (-1 == statusCookie)
	{
		statusCookie = status->Add(bstrText);
		if (-1 == statusCookie) 
			return E_FAIL;
	}
	else
	{
		if (FALSE == status->Set(statusCookie, bstrText))
			return E_FAIL;
	}

	return S_OK;
}