#include "main.h"
#include "./storageHelper.h"
#include "./storageHandler.h"
#include "./storageHandlerEnum.h"

StorageHelper::StorageHelper()
: ref(1)
{
}

StorageHelper::~StorageHelper()
{
}

HRESULT StorageHelper::CreateInstance(StorageHelper **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new StorageHelper();
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

size_t StorageHelper::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t StorageHelper::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int StorageHelper::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_OmStorageHelper))
		*object = static_cast<ifc_omstoragehelper*>(this);
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

HRESULT StorageHelper::CreateHandler(const wchar_t *key, HandlerProc proc, ifc_omstoragehandler **handler)
{
	return StorageHandler::CreateInstance(key, proc, StorageHandler::flagCopyKey, (StorageHandler**)handler);
}

HRESULT StorageHelper::CreateEnumerator(const TemplateRecord *recordList, size_t recordCount, ifc_omstoragehandlerenum **enumerator)
{
	return StorageHandlerEnum::CreateFromTemplate(recordList, recordCount, (StorageHandlerEnum**)enumerator);
}

#define CBCLASS StorageHelper
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_CREATEHANDLER, CreateHandler)
CB(API_CREATEENUMERATOR, CreateEnumerator)
END_DISPATCH;
#undef CBCLASS