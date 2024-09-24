#include "main.h"
#include "./storageHandler.h"

StorageHandler::StorageHandler(LPCWSTR pszKey, ifc_omstoragehelper::HandlerProc handlerProc, UINT flags)
: ref(1), key(NULL), handler(handlerProc), flags(flags)
{
	if (0 != (flagCopyKey & flags))
		key = Plugin_CopyString(pszKey);
	else
		key = (LPWSTR)pszKey;
}

StorageHandler::~StorageHandler()
{
	if (0 != (flagCopyKey & flags))
		Plugin_FreeString(key);
}

HRESULT StorageHandler::CreateInstance(LPCWSTR pszKey, ifc_omstoragehelper::HandlerProc handlerProc, UINT flags, StorageHandler **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	if (NULL == pszKey || L'\0' == pszKey || NULL == handlerProc) 
	{
		*instance = NULL;
		return E_INVALIDARG;
	}

	*instance = new StorageHandler(pszKey, handlerProc, flags);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t StorageHandler::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t StorageHandler::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int StorageHandler::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_OmStorageHandler))
		*object = static_cast<ifc_omstoragehandler*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT StorageHandler::GetKey(LPCWSTR *ppKey)
{
	if (NULL == ppKey) return E_POINTER;
	*ppKey = key;
	return S_OK;
}

void StorageHandler::Invoke(ifc_omservice *service, LPCWSTR pszKey, LPCWSTR pszValue)
{
	handler(service, pszKey, pszValue);
}

#define CBCLASS StorageHandler
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETKEY, GetKey)
VCB(API_INVOKE, Invoke)
END_DISPATCH;
#undef CBCLASS