#include "main.h"
#include "./storageEnum.h"
#include "./ifc_omstorage.h"

OmStorageEnumerator::OmStorageEnumerator(ifc_omstorage **storageList, size_t storageSize, const GUID *type, UINT capabilities)
	: ref(1), index(0), list(storageList), size(storageSize)
{
	fType = (NULL != type) ? *type : GUID_NULL;
	fCapabilities = capabilities;
}

OmStorageEnumerator::~OmStorageEnumerator()
{
}

HRESULT OmStorageEnumerator::CreateInstance(ifc_omstorage **storageList, size_t storageSize, const GUID *type, UINT capabilities, OmStorageEnumerator **instance)
{
	if (NULL == instance) return E_POINTER;
	
	*instance = new OmStorageEnumerator(storageList, storageSize, type, capabilities);
	if (NULL == *instance) return E_OUTOFMEMORY;
	
	return S_OK;
}

size_t OmStorageEnumerator::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmStorageEnumerator::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int OmStorageEnumerator::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_OmStorageEnumerator))
		*object = static_cast<ifc_omstorageenumerator*>(this);
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

HRESULT OmStorageEnumerator::Next(ULONG listSize, ifc_omstorage **elementList, ULONG *elementCount)
{
	if (NULL == elementList || 0 == listSize) return E_INVALIDARG;
	if (index >= size)
	{
		if (NULL != elementCount) *elementCount = 0;
		return S_FALSE;
	}

	ULONG count = 0;
	ifc_omstorage *storage = NULL;
	GUID storageId = GUID_NULL;

	for (;index < size && count < listSize; index++)
	{
		storage = list[index];
		if (NULL != storage && 
			SUCCEEDED(storage->GetType(&storageId)) && IsEqualGUID(storageId, fType) &&
			(0 == fCapabilities || fCapabilities == (fCapabilities & storage->GetCapabilities())))
		{
			elementList[count] = storage;
			storage->AddRef();
			count++;
		}
	}

	if (NULL != elementCount) *elementCount = count;

	return (count == listSize) ? S_OK : S_FALSE;
}

HRESULT OmStorageEnumerator::Reset(void)
{
	index = 0;
	return S_OK;
}

HRESULT OmStorageEnumerator::Skip(ULONG elementCount)
{
	index += elementCount;
	if (index >= size)
	{
		index = (size - 1);
		return S_FALSE;
	}
	return S_OK;
}

#define CBCLASS OmStorageEnumerator
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_NEXT, Next)
CB(API_RESET, Reset)
CB(API_SKIP, Skip)
END_DISPATCH;
#undef CBCLASS