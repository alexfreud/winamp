#ifndef NULLSOFT_WINAMP_OMSTORAGE_ASYNC_ENUMERATOR_WRAPPER_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_ASYNC_ENUMERATOR_WRAPPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_omstorageasync.h"

class ifc_omservicehost;
class ifc_omserviceenum;
class ifc_omservice;
class OmServiceList;

class EnumAsyncWrapper : public ifc_omstorageasync
{
protected:
	EnumAsyncWrapper(ifc_omserviceenum *enumerator);
	~EnumAsyncWrapper();

public:
	static HRESULT CreateInstance(ifc_omserviceenum *enumerator, EnumAsyncWrapper **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);
	
	/* ifc_omstorageasync */
	HRESULT GetState(UINT *state);
	HRESULT GetWaitHandle(HANDLE *handle);
	HRESULT GetData(void **data);
	
public:
	HRESULT SetData(void *data);
	HRESULT SetCallback(AsyncCallback callback);
	HRESULT GetCallback(AsyncCallback *callback);
	HRESULT RequestAbort(BOOL fDrop);

	HRESULT BeginEnumerate();
	HRESULT Enumerate();

	HRESULT GetResultCode();
	HRESULT GetServiceList(ifc_omserviceenum **list);

protected:
	size_t ref;
	ifc_omserviceenum *enumerator;
	AsyncCallback userCallback;
	void *userData;
	HANDLE completed;
	UINT state;
	HRESULT resultCode;
	CRITICAL_SECTION lock;
	OmServiceList *serviceList;

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_WINAMP_OMSTORAGE_ASYNC_ENUMERATOR_WRAPPER_HEADER