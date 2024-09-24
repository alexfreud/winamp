#ifndef NULLSOFT_ELEVATORFACTORY_H
#define NULLSOFT_ELEVATORFACTORY_H

#include <unknwn.h>

class ElevatorFactory : public IClassFactory 
{
public:
	ElevatorFactory();
  virtual ~ElevatorFactory();
	HRESULT __stdcall QueryInterface(REFIID riid, void ** ppAny);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	HRESULT __stdcall CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void ** ppAny);
  HRESULT __stdcall LockServer(BOOL fLock);
};


#endif