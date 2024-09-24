#include "main.h"
#include "./storageHandlerEnum.h"
#include "./storageHandler.h"

StorageHandlerEnum::StorageHandlerEnum()
	: ref(1), index(0)
{
}

StorageHandlerEnum::~StorageHandlerEnum()
{
	size_t index = handlerList.size();
	while(index--)
	{
		handlerList[index]->Release();
	}
}

HRESULT StorageHandlerEnum::CreateInstance(StorageHandlerEnum **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new StorageHandlerEnum();
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

HRESULT StorageHandlerEnum::CreateFromTemplate(const ifc_omstoragehelper::TemplateRecord *recordList, size_t recordCount, StorageHandlerEnum **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new StorageHandlerEnum();
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	if (NULL != recordList)
	{
		StorageHandler *handler = NULL;
		for (size_t i = 0; i < recordCount; i++)
		{
			const ifc_omstoragehelper::TemplateRecord *record = &recordList[i];
			if (NULL != record && 
				SUCCEEDED(StorageHandler::CreateInstance(record->key, record->handler, StorageHandler::flagCopyKey, &handler)))
			{
				if (FAILED((*instance)->AddHandler(handler)))
				{
					handler->Release();
				}
			}
		}
	}

	return S_OK;
}

size_t StorageHandlerEnum::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t StorageHandlerEnum::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int StorageHandlerEnum::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_OmStorageHandlerEnum))
		*object = static_cast<ifc_omstoragehandlerenum*>(this);
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

HRESULT StorageHandlerEnum::Next(ULONG listSize, ifc_omstoragehandler **elementList, ULONG *elementCount)
{
	if (NULL == elementList || 0 == listSize) return E_INVALIDARG;
	size_t size = handlerList.size();

	if (index >= size)
	{
		if (NULL != elementCount) *elementCount = 0;
		return S_FALSE;
	}

	ULONG count = 0;
	ifc_omstoragehandler *handler = NULL;

	for (;index < size && count < listSize; index++)
	{
		handler = handlerList[index];
		elementList[count] = handler;
		handler->AddRef();
		count++;
	}

	if (NULL != elementCount) 
		*elementCount = count;

	return (count == listSize) ? S_OK : S_FALSE;
}

HRESULT StorageHandlerEnum::Reset(void)
{
	index = 0;
	return S_OK;
}

HRESULT StorageHandlerEnum::Skip(ULONG elementCount)
{
	index += elementCount;
	if (index >= handlerList.size())
	{
		index = (handlerList.size() - 1);
		return S_FALSE;
	}
	return S_OK;
}

HRESULT StorageHandlerEnum::AddHandler(ifc_omstoragehandler *handler)
{
	if (NULL == handler) 
		return E_INVALIDARG;
	
	handlerList.push_back(handler);

	return S_OK;
}

#define CBCLASS StorageHandlerEnum
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_NEXT, Next)
CB(API_RESET, Reset)
CB(API_SKIP, Skip)
END_DISPATCH;
#undef CBCLASS