#include "ElevatorFactory.h"
#include "FileTypeRegistrar.h"

FileTypeRegistrar registrar;

void Lock();
void UnLock();

ElevatorFactory::ElevatorFactory()
{
}

ElevatorFactory::~ElevatorFactory()
{
}

ULONG ElevatorFactory::AddRef()
{
	return 10;
}

ULONG ElevatorFactory::Release()
{
	return 10;
}

HRESULT ElevatorFactory::QueryInterface(REFIID riid, void ** ppAny)
{
	// IID_IUnknown is the REFIID of standard interface IUnknown
	if(riid == IID_IUnknown)
	{
		*ppAny = (IUnknown *)this;
	}
	else if(riid == IID_IClassFactory)
	{
		*ppAny = (IClassFactory *)this;
	}
	else
	{
		*ppAny = NULL;
		return E_NOINTERFACE;
	}

	((IUnknown *)(*ppAny))->AddRef();

	return S_OK;
}

HRESULT ElevatorFactory::LockServer(BOOL fLock)
{
	if (fLock)
		Lock();
	else
		UnLock();
	return S_OK;
}

HRESULT ElevatorFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void **ppAny)
{
	if(pUnkOuter != NULL)
	{
		return CLASS_E_NOAGGREGATION;
	}

	FileTypeRegistrar *pRegistrar = &registrar;
	return pRegistrar->QueryInterface(riid, ppAny);
}