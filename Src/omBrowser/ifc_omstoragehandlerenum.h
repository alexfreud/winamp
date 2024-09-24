#ifndef NULLSOFT_WINAMP_OMSTORAGE_HANDLER_ENUMERATOR_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_HANDLER_ENUMERATOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {412877D7-0482-423d-87B9-975137588F6E}
static const GUID IFC_OmStorageHandlerEnum = 
{ 0x412877d7, 0x482, 0x423d, { 0x87, 0xb9, 0x97, 0x51, 0x37, 0x58, 0x8f, 0x6e } };

#include <bfc/dispatch.h>
class ifc_omstoragehandler;

class __declspec(novtable) ifc_omstoragehandlerenum : public Dispatchable
{
protected:
	ifc_omstoragehandlerenum() {}
	~ifc_omstoragehandlerenum() {}

public:
	HRESULT Next(unsigned long listSize, ifc_omstoragehandler **elementList, unsigned long *elementCount);
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

inline HRESULT ifc_omstoragehandlerenum::Next(unsigned long listSize, ifc_omstoragehandler **elementList, unsigned long *elementCount)
{
	return _call(API_NEXT, (HRESULT)E_NOTIMPL, listSize, elementList, elementCount);
}

inline HRESULT ifc_omstoragehandlerenum::Reset(void)
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_omstoragehandlerenum::Skip(unsigned long elementCount)
{
	return _call(API_SKIP, (HRESULT)E_NOTIMPL, elementCount);
}

#endif //NULLSOFT_WINAMP_OMSTORAGE_HANDLER_ENUMERATOR_INTERFACE_HEADER