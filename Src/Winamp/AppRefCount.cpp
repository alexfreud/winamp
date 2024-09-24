#include "AppRefCount.h"

AppRefCount appRefCount;
AppRefCount::AppRefCount()
{
	refCount = 1;
	m_dwThread = 0;
}

STDMETHODIMP AppRefCount::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG AppRefCount::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}

ULONG AppRefCount::Release(void)
{
    LONG lRef = InterlockedDecrement(&refCount);
    if (lRef == 0) PostThreadMessage(m_dwThread, WM_NULL, 0, 0);
    return lRef;
}

int AppRefCount_CanQuit()
{
	return appRefCount.refCount == 0;
}

void *InitAppRefCounterObject(DWORD threadId)
{
	appRefCount.m_dwThread = threadId;
	return (IUnknown *)&appRefCount;
}

void AppRefCount_Release()
{
	appRefCount.Release();
}