#ifndef NULLSOFT_WINAMP_OMSTORAGE_ENUMERATOR_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_ENUMERATOR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omstorageenum.h"

class OmStorageEnumerator : public ifc_omstorageenumerator
{
protected:
	OmStorageEnumerator(ifc_omstorage **storageList, size_t storageSize, const GUID *type, UINT capabilities);
	~OmStorageEnumerator();

public:
	static HRESULT CreateInstance(ifc_omstorage **storageList, size_t storageSize, const GUID *type, UINT capabilities, OmStorageEnumerator **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omstorage */
	HRESULT Next(ULONG listSize, ifc_omstorage **elementList, ULONG *elementCount);
	HRESULT Reset(void);
	HRESULT Skip(ULONG elementCount);
	
protected:
	ULONG ref;
	size_t index;
	ifc_omstorage **list;
	size_t size;
	GUID fType;
	UINT fCapabilities;

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_WINAMP_OMSTORAGE_ENUMERATOR_HEADER