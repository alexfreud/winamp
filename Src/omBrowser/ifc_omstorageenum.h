#ifndef NULLSOFT_WINAMP_OMSTORAGE_ENUMERATOR_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_ENUMERATOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {31BF2BEF-4362-4277-A4E1-0589336BC55C}
static const GUID IFC_OmStorageEnumerator = 
{ 0x31bf2bef, 0x4362, 0x4277, { 0xa4, 0xe1, 0x5, 0x89, 0x33, 0x6b, 0xc5, 0x5c } };

#include <bfc/dispatch.h>
class ifc_omstorage;

class __declspec(novtable) ifc_omstorageenumerator : public Dispatchable
{
protected:
	ifc_omstorageenumerator() {}
	~ifc_omstorageenumerator() {}

public:
	HRESULT Next(unsigned long listSize, ifc_omstorage **elementList, unsigned long *elementCount);
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

inline HRESULT ifc_omstorageenumerator::Next(unsigned long listSize, ifc_omstorage **elementList, unsigned long *elementCount)
{
	return _call(API_NEXT, (HRESULT)E_NOTIMPL, listSize, elementList, elementCount);
}

inline HRESULT ifc_omstorageenumerator::Reset(void)
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_omstorageenumerator::Skip(unsigned long elementCount)
{
	return _call(API_SKIP, (HRESULT)E_NOTIMPL, elementCount);
}

#endif //NULLSOFT_WINAMP_OMSTORAGE_ENUMERATOR_INTERFACE_HEADER