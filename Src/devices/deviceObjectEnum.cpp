#include "main.h"
#include "./deviceObjectEnum.h"
#include <new>

DeviceObjectEnum::DeviceObjectEnum() 
	: ref(1), buffer(NULL), size(0), cursor(0)
{
}

DeviceObjectEnum::~DeviceObjectEnum()
{
	if (NULL != buffer)
	{
		while(size--)
		{
			buffer[size]->Release();
		}
	}
}

HRESULT DeviceObjectEnum::CreateInstance(ifc_deviceobject **objects, 
										size_t count, 
										DeviceObjectEnum **instance)
{
	size_t index, size;
	void *storage;
	ifc_deviceobject *o;
	DeviceObjectEnum *enumerator;
	

	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	size = sizeof(DeviceObjectEnum) + (sizeof(ifc_deviceobject**) * count);
	storage = calloc(1, size);
	if (NULL == storage)
		return E_OUTOFMEMORY;
	
	enumerator = new(storage) DeviceObjectEnum();
	if (NULL == enumerator)
	{
		free(storage);
		return E_FAIL;
	}

	enumerator->buffer = (ifc_deviceobject**)(((BYTE*)enumerator) + sizeof(DeviceObjectEnum));
	
	for (index = 0; index < count; index++)
	{
		o = objects[index];
		if (NULL != o)
		{
			enumerator->buffer[enumerator->size] = o;
			o->AddRef();
			enumerator->size++;
		}
	}

	*instance = enumerator;
	return S_OK;
}



size_t DeviceObjectEnum::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceObjectEnum::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceObjectEnum::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DeviceObjectEnum))
		*object = static_cast<ifc_deviceobjectenum*>(this);
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

HRESULT DeviceObjectEnum::Next(ifc_deviceobject **objects, size_t bufferMax, size_t *fetched)
{
	size_t available, copied, index;
	ifc_deviceobject **source;

	if (NULL == objects)
		return E_POINTER;
	
	if (0 == bufferMax) 
		return E_INVALIDARG;

	if (cursor >= size)
	{
		if (NULL != fetched) 
			*fetched = 0;

		return S_FALSE;
	}

	available = size - cursor;
	copied = ((available > bufferMax) ? bufferMax : available);
	
	source = buffer + cursor;
	CopyMemory(objects, source, copied * sizeof(ifc_deviceobject*));
    
	for(index = 0; index < copied; index++)
		objects[index]->AddRef();
	
	cursor += copied;

	if (NULL != fetched) 
		*fetched = copied;

	return (bufferMax == copied) ? S_OK : S_FALSE;
}

HRESULT DeviceObjectEnum::Reset(void)
{
	cursor = 0;
	return S_OK;
}

HRESULT DeviceObjectEnum::Skip(size_t count)
{
	cursor += count;
	if (cursor > size)
		cursor = size;
	
	return (cursor < size) ? S_OK : S_FALSE;
}


HRESULT DeviceObjectEnum::GetCount(size_t *count)
{
	if (NULL == count)
		return E_POINTER;
	
	*count = size;

	return S_OK;
}

#define CBCLASS DeviceObjectEnum
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_NEXT, Next)
CB(API_RESET, Reset)
CB(API_SKIP, Skip)
CB(API_GETCOUNT, GetCount)
END_DISPATCH;
#undef CBCLASS