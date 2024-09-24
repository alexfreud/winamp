#include "./main.h"
#include "./serviceList.h"

#include "./ifc_omservice.h"

OmServiceList::OmServiceList()
	: ref(1), cursor(0)
{
}

OmServiceList::~OmServiceList()
{
	size_t index = list.size();
	while(index--)
	{
		list[index]->Release();
	}
}

HRESULT OmServiceList::CreateInstance(OmServiceList **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new OmServiceList();
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

size_t OmServiceList::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmServiceList::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

int OmServiceList::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_OmServiceEnum))
		*object = static_cast<ifc_omserviceenum*>(this);
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

HRESULT OmServiceList::Next(unsigned long listSize, ifc_omservice **elementList, unsigned long *elementCount)
{
	if (NULL == elementList || 0 == listSize) return E_INVALIDARG;

	size_t size = list.size();
	
	if (cursor >= size)
	{
		if (NULL != elementCount) *elementCount = 0;
		return S_FALSE;
	}

	size_t available = size - cursor;
	size_t count = ((available > listSize) ? listSize : available);

	ifc_omservice **source = &list.at(0) + cursor;
	CopyMemory(elementList, source, count * sizeof(ifc_omservice*));
    for(size_t i = 0; i < count; i++)
	{
		elementList[i]->AddRef();
	}

	cursor += count;

	if (NULL != elementCount) 
		*elementCount = (ULONG)count;

	return (count == listSize) ? S_OK : S_FALSE;
}

HRESULT OmServiceList::Reset(void)
{
	cursor = 0;
	return S_OK;
}

HRESULT OmServiceList::Skip(unsigned long elementCount)
{
	size_t size = list.size();
	cursor += elementCount;
	if (cursor > size)
		cursor = size;
	
	return (cursor < size) ? S_OK : S_FALSE;
}

HRESULT OmServiceList::Add(ifc_omservice *service)
{
	if(NULL == service) return E_INVALIDARG;
	list.push_back(service);
	service->AddRef();
	return S_OK;
}

HRESULT OmServiceList::Remove(size_t index)
{
	if (index >= list.size()) 
		return E_FAIL;
	ifc_omservice *temp = list[index];
	list.erase(list.begin() + index);
	temp->Release();

	return S_OK;
}

HRESULT OmServiceList::Clear()
{
	size_t index = list.size();
	while(index--)
	{
		list[index]->Release();
	}
	list.clear();
	return S_OK;
}

#define CBCLASS OmServiceList
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_NEXT, Next)
CB(API_RESET, Reset)
CB(API_SKIP, Skip)
END_DISPATCH;
#undef CBCLASS