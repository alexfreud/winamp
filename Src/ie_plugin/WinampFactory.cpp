#include "WinampFactory.h"
#include "Winamp.h"

//FileTypeRegistrar registrar;

WinampFactory::WinampFactory()
{
}

WinampFactory::~WinampFactory()
{
}

ULONG WinampFactory::AddRef()
{
	return 10;
}

ULONG WinampFactory::Release()
{
	return 10;
}

HRESULT WinampFactory::QueryInterface(REFIID riid, void ** ppAny)
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

HRESULT WinampFactory::LockServer(BOOL fLock)
{
	return S_OK;
}

HRESULT WinampFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void **ppAny)
{
	if(pUnkOuter != NULL)
	{
		return CLASS_E_NOAGGREGATION;
	}

	Winamp *winamp = new Winamp;
	HRESULT hr = winamp->QueryInterface(riid, ppAny);
	if (FAILED(hr))
		delete winamp;
	return hr;


}