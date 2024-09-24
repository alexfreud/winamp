#include "./providerOperation.h"
#include "./loginProvider.h"
#include "./providerEnumerator.h"

LoginProviderOperation::LoginProviderOperation(LoginProvider *pSource, LoginProvider *pTarget, UINT uCode)
	: ref(1), source(pSource), target(pTarget), code(uCode)
{
	if (NULL != source) source->AddRef();
	if (NULL != target) target->AddRef();
}

LoginProviderOperation::~LoginProviderOperation()
{
	if (NULL != source) source->Release();
	if (NULL != target) target->Release();
}

HRESULT LoginProviderOperation::CreateDeleteOperation(LoginProvider *pRemove, LoginProviderOperation **instance)
{
	if (NULL == instance)
		return E_POINTER;

	if (NULL == pRemove)
	{
		*instance = NULL;
		return E_INVALIDARG;
	}

	*instance = new LoginProviderOperation(pRemove, NULL, operationDelete);
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

HRESULT LoginProviderOperation::CreateReplaceOperation(LoginProvider *pSource, LoginProvider *pTarget, LoginProviderOperation **instance)
{
	if (NULL == instance)
		return E_POINTER;

	if (NULL == pSource || NULL == pTarget)
	{
		*instance = NULL;
		return E_INVALIDARG;
	}

	*instance = new LoginProviderOperation(pSource, pTarget, operationReplace);
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

HRESULT LoginProviderOperation::CreateFromUpdate(LoginProvider *active, LoginProviderEnumerator *enumerator, LoginProviderOperation **instance)
{
	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == active || NULL == enumerator)
		return E_INVALIDARG;
		
	GUID testId, activeId(GUID_NULL);
	HRESULT hr = active->GetId(&activeId);
	if (FAILED(hr)) return hr;

	LoginProvider *test;
	BOOL providerFound = FALSE;
	enumerator->Reset();

	BOOL loop = TRUE;
	while(TRUE == loop && S_OK == enumerator->Next(1, &test, NULL))
	{
		if (FAILED(test->GetId(&testId)))
		{
			hr = E_FAIL;
			loop = FALSE;
		}
		else if(FALSE != IsEqualGUID(activeId, testId))
		{
			providerFound = TRUE;
			if (S_OK == test->IsIdentical(active))
				hr = S_FALSE;
			else
				hr = CreateReplaceOperation(active, test, instance);
			loop = FALSE;
		}

		test->Release();
	}
	
	if (SUCCEEDED(hr) && FALSE == providerFound)
		hr = CreateDeleteOperation(active, instance);

	return hr;
}

ULONG LoginProviderOperation::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG LoginProviderOperation::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

UINT LoginProviderOperation::GetCode()
{
	return code;
}
HRESULT LoginProviderOperation::GetSource(LoginProvider **provider)
{
	if (NULL == provider) return E_POINTER;
	*provider = source;
	
	if (NULL != source)
		source->AddRef();
	
	return S_OK;
}

HRESULT LoginProviderOperation::GetTarget(LoginProvider **provider)
{
	if (NULL == provider) return E_POINTER;
	*provider = target;
	
	if (NULL != target)
		target->AddRef();
	
	return S_OK;
}
