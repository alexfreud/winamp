#include "main.h"
#include "./deviceObjectStore.h"
#include <algorithm>

template <typename T>
struct GenericComparator
{
	typedef const char* (T::*GETTER)();
	GETTER m_getterFunc;
	const char *m_data;
	GenericComparator(GETTER getterFunc, const char *data)
	{
		m_getterFunc = getterFunc;
		m_data = data;
	}
	bool operator()(const T& obj)
	{
		return strcmp(((obj).*m_getterFunc)(), m_data) == 0;
	}
};

static ifc_deviceobject * 
DeviceObjectStore_FindLocation(const char *name, std::vector<ifc_deviceobject*> &list)
{
	if (FALSE != IS_STRING_EMPTY(name))
		return NULL;

	//size_t name_length;
	//name_length = lstrlenA(name) * sizeof(char);
	//
	//return (ifc_deviceobject*)bsearch_s(name, buffer, length, 
	//									sizeof(ifc_deviceobject*),
	//									DeviceObjectStore_FindComparer, 
	//									(void*)name_length);

	//auto it = std::find_if(list.begin(), list.begin(), GenericComparator<ifc_deviceobject>(&ifc_deviceobject::GetName, name));

	const auto it = std::find_if(list.begin(), list.end(),
		[&](ifc_deviceobject* upT) -> bool
		{
			return strcmp(upT->GetName(), name) == 0;
		}
	);
	if (it != list.end())
	{
		return *it;
	}
	return nullptr;
}

int DeviceObjectComparer(const char* arg1, const char* arg2)
{
	return stricmp(arg1, arg2);
}


// Created for std::sort
static bool DeviceObjectStore_SortComparer_V2(const void* element1, const void* element2)
{
	//return DeviceObjectStore_SortComparer(element1, element2) < 0;
	const char* name1 = (((ifc_deviceobject*)element1))->GetName();
	const char* name2 = (((ifc_deviceobject*)element2))->GetName();
	return DeviceObjectComparer(name1, name2) < 0;
}

static ifc_deviceobject *
DeviceObjectStore_FindUnsortedObject(const char *name, ifc_deviceobject **objects, size_t count)
{
	size_t index;
	size_t length;

	if (0 == count)
		return NULL;
	
	length = lstrlenA(name) * sizeof(char);
	
	for(index = 0; index < count; index++)
	{
		//if (0 == DeviceObjectStore_NameCompare(name, length, objects[index]->GetName()))
		if(0 == DeviceObjectComparer(name, objects[index]->GetName()))
			return objects[index];
	}

	return NULL;
}


DeviceObjectStore::DeviceObjectStore(DeviceObjectCallback addCallback, 
									DeviceObjectCallback _removeCallback, void *callbackData)
{
	this->addCallback = addCallback;
	this->removeCallback = _removeCallback;
	this->callbackData = callbackData;

	InitializeCriticalSection(&lock);
}

DeviceObjectStore::~DeviceObjectStore()
{
	RemoveAll();
	DeleteCriticalSection(&lock);
}

void DeviceObjectStore::Lock()
{
	EnterCriticalSection(&lock);
}

void DeviceObjectStore::Unlock()
{
	LeaveCriticalSection(&lock);
}

CRITICAL_SECTION *DeviceObjectStore::GetLock()
{
	return &lock;
}

HRESULT DeviceObjectStore::Add(ifc_deviceobject *object)
{
	const char *name;
	
	if (NULL == object)
		return E_POINTER;

	name = object->GetName();

	if (NULL == name || '\0' == *name)
		return E_INVALIDARG;

	return (1 == AddRange(&object, 1)) ? S_OK : S_FALSE;
}

size_t DeviceObjectStore::AddRange(ifc_deviceobject **objects, size_t count)
{
	const char *name;
	size_t index, registered, added;
	ifc_deviceobject *object;

	
	if (NULL == objects || 0 == count)
		return 0;
		
	Lock();

	added = 0;
	registered = list.size();
	
	for(index = 0; index < count; index++)
	{
		object = objects[index];
		if (NULL != object)
		{
			name = object->GetName();
			
			if (NULL != name && 
				'\0' != *name && 
				NULL == DeviceObjectStore_FindLocation(name, list))
				//&& NULL == DeviceObjectStore_FindUnsortedObject(name, buffer + registered, added))
			{
				list.push_back(object);
				object->AddRef();

				if (NULL != addCallback)
					this->addCallback(this, object, callbackData);

				added++;
			}
		}
	}
	
	if (0 != added)
	{
		//qsort(list.first(), list.size(), 
		//		sizeof(ifc_deviceobject**), 
		//		DeviceObjectStore_SortComparer);

		std::sort(list.begin(), list.end(), DeviceObjectStore_SortComparer_V2);
	}

	Unlock();

	return added;
}

size_t DeviceObjectStore::AddIndirect(const char **names, size_t count, DeviceObjectCreator callback, void *user)
{
	size_t index, registered, added;
	ifc_deviceobject *object;

	if (NULL == names || 0 == count || NULL == callback)
		return 0;

	Lock();

	added = 0;
	registered = list.size();
	

	for(index = 0; index < count; index++)
	{
		const char *name = names[index];

		if (NULL != name && 
			'\0' != *name && 
			NULL == DeviceObjectStore_FindLocation(name, list) )
			//&& NULL == DeviceObjectStore_FindUnsortedObject(name, buffer + registered, added))
		{
			object = callback(name, user);
			if (NULL != object)
			{
				list.push_back(object);
				
				if (NULL != addCallback)
					this->addCallback(this, object, callbackData);

				added++;
			}
		}
	}
	
	if (0 != added)
	{
		//qsort(list.first(), list.size(), 
		//		sizeof(ifc_deviceobject**), 
		//		DeviceObjectStore_SortComparer);

		std::sort(list.begin(), list.end(), DeviceObjectStore_SortComparer_V2);
	}

	Unlock();

	return added;
}

HRESULT DeviceObjectStore::Remove(const char *name)
{
	HRESULT hr = hr = S_FALSE;
	
	if (NULL == name || '\0' == *name)
		return E_INVALIDARG;

	Lock();

	//object_ptr = DeviceObjectStore_FindLocation(name, list);
	//if (NULL != object_ptr)
	//{
	//	hr = S_OK;

	//	ifc_deviceobject *object = *object_ptr;

	//	size_t index = (size_t)(object_ptr - buffer);
	//	list.erase(list.begin() + index);

	//	if (NULL != removeCallback)
	//		removeCallback(this, object, callbackData);

	//	object->Release();
	//}
	//else
	//{
	//	hr = S_FALSE;
	//}
	
	const auto it = std::find_if(list.begin(), list.end(),
		[&](ifc_deviceobject* upT) -> bool
		{
			return strcmp(upT->GetName(), name) == 0;
		}
	);

	if (it != list.end())
	{
		ifc_deviceobject* object = *it;
		list.erase(it);
		if (NULL != removeCallback)
		{
			removeCallback(this, object, callbackData);
		}
		object->Release();
		hr = S_OK;
	}
	Unlock();

	return hr;
}

void DeviceObjectStore::RemoveAll()
{
	Lock();

	size_t index = list.size();
	while(index--)
	{
		ifc_deviceobject *object = list[index];
		if (NULL != removeCallback)
			removeCallback(this, object, callbackData);

		object->Release();
	}

	list.clear();
	Unlock();
}

HRESULT DeviceObjectStore::Find(const char *name, ifc_deviceobject **object)
{
	HRESULT hr;
	ifc_deviceobject *object_ptr;

	if (NULL == name || '\0' == *name)
		return E_INVALIDARG;

	if (NULL == object)
		return E_POINTER;

	Lock();

	object_ptr = DeviceObjectStore_FindLocation(name, list);
	
	if (NULL != object_ptr)
	{
		if (NULL != object)
		{
			*object = object_ptr;
			(*object)->AddRef();
		}
		hr = S_OK;
	}
	else
	{
		if (NULL != object)
			*object = NULL;
		hr = S_FALSE;
	}
	
	Unlock();

	return hr;
}

HRESULT DeviceObjectStore::Enumerate(DeviceObjectEnum **enumerator)
{
	HRESULT hr;

	if (NULL == enumerator)
		return E_POINTER;

	Lock();

	hr = DeviceObjectEnum::CreateInstance(list.size() ? &list.at(0) : nullptr, list.size(), enumerator);
		
	Unlock();

	return hr;
}
