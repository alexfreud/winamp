#ifndef NULLSOFT_WINAMP_OMSTORAGE_EXTENDER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_EXTENDER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {8D1915C0-7925-49f1-A851-C230C634B7EE}
static const GUID IFC_OmStorageExt = 
{ 0x8d1915c0, 0x7925, 0x49f1, { 0xa8, 0x51, 0xc2, 0x30, 0xc6, 0x34, 0xb7, 0xee } };

#include <bfc/dispatch.h>
class ifc_omstoragehandlerenum;
class ifc_omstorage;

class __declspec(novtable) ifc_omstorageext : public Dispatchable
{
protected:
	ifc_omstorageext() {}
	~ifc_omstorageext() {}

public:
	HRESULT Enumerate(const GUID *storageId, ifc_omstoragehandlerenum **enumerator);
	
public:
	DISPATCH_CODES
	{
		API_ENUMERATE = 10,
	};
};

inline HRESULT ifc_omstorageext::Enumerate(const GUID *storageId, ifc_omstoragehandlerenum **enumerator)
{
	return _call(API_ENUMERATE, (HRESULT)E_NOTIMPL, storageId, enumerator);
}

#endif //NULLSOFT_WINAMP_OMSTORAGE_EXTENDER_INTERFACE_HEADER