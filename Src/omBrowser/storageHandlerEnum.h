#ifndef NULLSOFT_ONLINEMEDIA_PLUGIN_STORAGE_HANDLER_ENUMERATOR_HEADER
#define NULLSOFT_ONLINEMEDIA_PLUGIN_STORAGE_HANDLER_ENUMERATOR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omstoragehandlerenum.h"
#include "./ifc_omstoragehelper.h"
#include <vector>

class StorageHandlerEnum : public ifc_omstoragehandlerenum
{
protected:
	StorageHandlerEnum();
	~StorageHandlerEnum();

public:
	static HRESULT CreateInstance(StorageHandlerEnum **instance);
	static HRESULT CreateFromTemplate(const ifc_omstoragehelper::TemplateRecord *recordList, size_t recordCount, StorageHandlerEnum **instance);
	
public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omstoragehandlerenum */
	HRESULT Next(ULONG listSize, ifc_omstoragehandler **elementList, ULONG *elementCount);
	HRESULT Reset(void);
	HRESULT Skip(ULONG elementCount);

public:
	HRESULT AddHandler(ifc_omstoragehandler *handler);

protected:
	typedef std::vector<ifc_omstoragehandler*> HandlerList;

protected:
	ULONG ref;
	HandlerList handlerList;
	size_t index;

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_ONLINEMEDIA_PLUGIN_STORAGE_HANDLER_ENUMERATOR_HEADER