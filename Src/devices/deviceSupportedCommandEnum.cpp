#include "main.h"
#include "./deviceSupportedCommandEnum.h"
#include <new>

DeviceSupportedCommandEnum::DeviceSupportedCommandEnum() 
	: ref(1), commands(NULL), count(0), cursor(0)
{
}

DeviceSupportedCommandEnum::~DeviceSupportedCommandEnum()
{
	if (NULL != commands)
	{
		while(count--)
			commands[count]->Release();
	}
}

HRESULT DeviceSupportedCommandEnum::CreateInstance(ifc_devicesupportedcommand **commands, size_t count, 
													DeviceSupportedCommandEnum **instance)
{
	size_t index, size;
	void *storage;
	ifc_devicesupportedcommand *c;
	DeviceSupportedCommandEnum *enumerator;
	

	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	size = sizeof(DeviceSupportedCommandEnum) + (sizeof(ifc_devicesupportedcommand**) * count);
	storage = calloc(1, size);
	if (NULL == storage)
		return E_OUTOFMEMORY;
	
	enumerator = new(storage) DeviceSupportedCommandEnum();
	if (NULL == enumerator)
	{
		free(storage);
		return E_FAIL;
	}

	enumerator->commands = (ifc_devicesupportedcommand**)(((BYTE*)enumerator) + sizeof(DeviceSupportedCommandEnum));
	
	for (index = 0; index < count; index++)
	{
		c = commands[index];
		if (NULL != c)
		{
			enumerator->commands[enumerator->count] = c;
			c->AddRef();
			enumerator->count++;
		}
	}

	*instance = enumerator;
	return S_OK;
}



size_t DeviceSupportedCommandEnum::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceSupportedCommandEnum::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceSupportedCommandEnum::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DeviceSupportedCommandEnum))
		*object = static_cast<ifc_devicesupportedcommandenum*>(this);
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

HRESULT DeviceSupportedCommandEnum::Next(ifc_devicesupportedcommand **objects, size_t bufferMax, size_t *fetched)
{
	size_t available, copied, index;
	ifc_devicesupportedcommand **source;

	if (NULL == objects)
		return E_POINTER;
	
	if (0 == bufferMax) 
		return E_INVALIDARG;

	if (cursor >= count)
	{
		if (NULL != fetched) 
			*fetched = 0;

		return S_FALSE;
	}

	available = count - cursor;
	copied = ((available > bufferMax) ? bufferMax : available);
	
	source = commands + cursor;
	CopyMemory(objects, source, copied * sizeof(ifc_devicesupportedcommand*));
    
	for(index = 0; index < copied; index++)
		objects[index]->AddRef();
	
	cursor += copied;

	if (NULL != fetched) 
		*fetched = copied;

	return (bufferMax == copied) ? S_OK : S_FALSE;
}

HRESULT DeviceSupportedCommandEnum::Reset(void)
{
	cursor = 0;
	return S_OK;
}

HRESULT DeviceSupportedCommandEnum::Skip(size_t count)
{
	cursor += count;
	if (cursor > this->count)
		cursor = this->count;
	
	return (cursor < this->count) ? S_OK : S_FALSE;
}


HRESULT DeviceSupportedCommandEnum::GetCount(size_t *count)
{
	if (NULL == count)
		return E_POINTER;
	
	*count = this->count;

	return S_OK;
}

#define CBCLASS DeviceSupportedCommandEnum
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