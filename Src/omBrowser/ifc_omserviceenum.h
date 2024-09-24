#ifndef NULLSOFT_WINAMP_OMSERVICE_ENUMERATOR_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_ENUMERATOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {27E67F83-2E2B-4b5b-A029-2CDDFF28BD41}
static const GUID IFC_OmServiceEnum = 
{ 0x27e67f83, 0x2e2b, 0x4b5b, { 0xa0, 0x29, 0x2c, 0xdd, 0xff, 0x28, 0xbd, 0x41 } };


#include <bfc/dispatch.h>
class ifc_omservice;

class __declspec(novtable) ifc_omserviceenum : public Dispatchable
{

protected:
	ifc_omserviceenum() {}
	~ifc_omserviceenum() {}

public:
	HRESULT Next(unsigned long listSize, ifc_omservice **elementList, unsigned long *elementCount);
	HRESULT Reset(void);
	HRESULT Skip(unsigned long elementCount);

public:
	DISPATCH_CODES
	{
		API_NEXT = 10,
		API_RESET = 20,
		API_SKIP = 30,
	};
};

inline HRESULT ifc_omserviceenum::Next(unsigned long listSize, ifc_omservice **elementList, unsigned long *elementCount)
{
	return _call(API_NEXT, (HRESULT)E_NOTIMPL, listSize, elementList, elementCount);
}

inline HRESULT ifc_omserviceenum::Reset(void)
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_omserviceenum::Skip(unsigned long elementCount)
{
	return _call(API_SKIP, (HRESULT)E_NOTIMPL, elementCount);
}

#endif //NULLSOFT_WINAMP_OMSERVICE_ENUMERATOR_INTERFACE_HEADER