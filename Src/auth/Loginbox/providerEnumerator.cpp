#include "./common.h"
#include "./providerEnumerator.h"
#include "./loginProvider.h"

LoginProviderEnumerator::LoginProviderEnumerator()
: ref(1), cursor(0), list(NULL), size(0)
{
}

LoginProviderEnumerator::~LoginProviderEnumerator()
{
	if (NULL != list)
	{
		size_t index = size;
		while(index--)
		{
			LoginProvider *provider = list[index];
			if (NULL != provider) provider->Release();
		}

		free(list);
	}
}

ULONG LoginProviderEnumerator::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG LoginProviderEnumerator::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT LoginProviderEnumerator::CreateInstance(LoginProvider **list, size_t size, LoginProviderEnumerator **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == list) return E_INVALIDARG;

	LoginProviderEnumerator *enumerator = new LoginProviderEnumerator();
	if(NULL == enumerator) 
		return E_OUTOFMEMORY;

	if (0 != size)
	{
		enumerator->list = (LoginProvider**)calloc(size, sizeof(LoginProvider*));
		if (NULL == enumerator->list)
		{
			enumerator->Release();
			return E_OUTOFMEMORY;
		}

		enumerator->size = size;
		CopyMemory(enumerator->list, list, size * sizeof(LoginProvider*));
	}

	*instance = enumerator;
	return S_OK;
}

HRESULT LoginProviderEnumerator::Next(ULONG listSize, LoginProvider **elementList, ULONG *elementCount)
{
	if (NULL == elementList || 0 == listSize) return E_INVALIDARG;
	if (cursor >= size)
	{
		if (NULL != elementCount) *elementCount = 0;
		return S_FALSE;
	}

	ULONG count = 0;
	
	for (;cursor < size && count < listSize; cursor++)
	{
		LoginProvider *provider = list[cursor];
		if (NULL != provider)
		{
			provider->AddRef();
			elementList[count] = provider;
			count++;
		}
	}

	if (NULL != elementCount) *elementCount = count;

	return (count == listSize) ? S_OK : S_FALSE;
}


HRESULT LoginProviderEnumerator::Reset(void)
{
	cursor = 0;
	return S_OK;
}

HRESULT LoginProviderEnumerator::Skip(ULONG elementCount)
{
	cursor += elementCount;
	if (cursor >= size)
	{
		cursor = (size - 1);
		return S_FALSE;
	}
	return S_OK;

}
