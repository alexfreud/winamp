#ifndef NULLSOFT_ONLINEMEDIA_PLUGIN_STORAGE_HANDLER_HEADER
#define NULLSOFT_ONLINEMEDIA_PLUGIN_STORAGE_HANDLER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omstoragehandler.h"
#include "./ifc_omstoragehelper.h"
class ifc_omservice;

class StorageHandler : public ifc_omstoragehandler
{
public:
	typedef enum
	{
		flagCopyKey = 0x00000001,
	} Flags;

protected:
	StorageHandler(LPCWSTR pszKey, ifc_omstoragehelper::HandlerProc handlerProc, UINT flags);
	~StorageHandler();

public:
	static HRESULT CreateInstance(LPCWSTR pszKey, ifc_omstoragehelper::HandlerProc handlerProc, UINT flags, StorageHandler **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omstoragehandler */
	HRESULT GetKey(LPCWSTR *ppKey);
	void Invoke(ifc_omservice *service, LPCWSTR pszKey, LPCWSTR pszValue);

protected:
	ULONG ref;
	LPWSTR key;
	ifc_omstoragehelper::HandlerProc handler;
	UINT flags;

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_ONLINEMEDIA_PLUGIN_STORAGE_HANDLER_HEADER