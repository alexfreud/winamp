#ifndef NULLSOFT_WINAMP_OMSTORAGE_HELPER_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omstoragehelper.h"

class StorageHelper : public ifc_omstoragehelper
{
protected:
	StorageHelper();
	~StorageHelper();

public:
	static HRESULT CreateInstance(StorageHelper **instance);
public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omstoragehelper */
	HRESULT CreateHandler(const wchar_t *key, HandlerProc proc, ifc_omstoragehandler **handler);
	HRESULT CreateEnumerator(const TemplateRecord *recordList, size_t recordCount, ifc_omstoragehandlerenum **enumerator);

protected:
	size_t ref;

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_WINAMP_OMSTORAGE_HELPER_HEADER