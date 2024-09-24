#pragma once

#include <unknwn.h>

class WinampFactory: public IClassFactory 
{
public:
	WinampFactory();
  virtual ~WinampFactory();
	HRESULT __stdcall QueryInterface(REFIID riid, void ** ppAny);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	HRESULT __stdcall CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void ** ppAny);
  HRESULT __stdcall LockServer(BOOL fLock);
};


