#include "main.h"
#include "./deviceSupportedCommandStore.h"

DeviceSupportedCommandStore::DeviceSupportedCommandStore()
	: ref(1)
{
	InitializeCriticalSection(&lock);
}

DeviceSupportedCommandStore::~DeviceSupportedCommandStore()
{
	RemoveAll();
	DeleteCriticalSection(&lock);
}

HRESULT DeviceSupportedCommandStore::CreateInstance(DeviceSupportedCommandStore **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = new DeviceSupportedCommandStore();

	if (NULL == *instance) 
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t DeviceSupportedCommandStore::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceSupportedCommandStore::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceSupportedCommandStore::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DeviceSupportedCommandStore))
		*object = static_cast<ifc_devicesupportedcommandstore*>(this);
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

void DeviceSupportedCommandStore::Lock()
{
	EnterCriticalSection(&lock);
}

void DeviceSupportedCommandStore::Unlock()
{
	LeaveCriticalSection(&lock);
}

int 
DeviceSupportedCommandStore::SortCallback(const void *element1, const void *element2)
{
	DeviceSupportedCommand *command1, *command2;

	command1 = *(DeviceSupportedCommand**)element1;
	command2 = *(DeviceSupportedCommand**)element2;
	
	return CompareStringA(CSTR_INVARIANT, 0, command1->GetName(), -1, command2->GetName(), -1) - 2;
}

int 
DeviceSupportedCommandStore::SearchCallback(const void *key, const void *element)
{
	const char *name;
	DeviceSupportedCommand *command;

	name = (const char*)key;
	command = *(DeviceSupportedCommand**)element;
	
	return CompareStringA(CSTR_INVARIANT, 0, name, -1, command->GetName(), -1) - 2;
}

DeviceSupportedCommand *DeviceSupportedCommandStore::Find(const char *name, size_t *indexOut)
{
	DeviceSupportedCommand *command;
	int length;
	size_t index, count;

	if (NULL == name || '\0' == *name)
		return NULL;
		
	length = lstrlenA(name);

	count = commandList.size();
	for(index = 0; index < count; index++)
	{
		command = commandList[index];
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, name, length, command->GetName(), -1))
			break;
	}
	
	if (count == index)
		return NULL;

	if (NULL != indexOut)
		*indexOut = index;
	
	return command;

}


HRESULT DeviceSupportedCommandStore::Add(const char *name, DeviceCommandFlags flags)
{
	HRESULT hr;
	DeviceSupportedCommand *command;
	

	hr = DeviceSupportedCommand::CreateInstance(name, &command);
	if (FAILED(hr))
		return hr;

	Lock();

	if (NULL == Find(name, NULL))
	{
		command->SetFlags(flags, flags);
		commandList.push_back(command);
	}
	else
	{
		command->Release();
		hr = S_FALSE;
	}
	
	Unlock();

	return hr;
}

HRESULT DeviceSupportedCommandStore::Remove(const char *name)
{
	HRESULT hr;
	size_t index;
	DeviceSupportedCommand *command;

	Lock();

	command = Find(name, &index);
	if (NULL != command)
	{
		commandList.erase(commandList.begin() + index);
		command->Release();
		hr = S_OK;
	}
	else
	{
		hr = S_FALSE;
	}
	
	Unlock();

	return hr;
}

HRESULT DeviceSupportedCommandStore::RemoveAll()
{
	Lock();

	size_t index = commandList.size();
	while(index--)
	{
		DeviceSupportedCommand *command = commandList[index];
		command->Release();
	}

	commandList.clear();

	Unlock();

	return S_OK;
}

HRESULT DeviceSupportedCommandStore::GetFlags(const char *name, DeviceCommandFlags *flagsOut)
{
	HRESULT hr;
	DeviceSupportedCommand *command;

	if (NULL == flagsOut)
		return E_POINTER;

	Lock();

	command = Find(name, NULL);
	hr = (NULL != command) ? command->GetFlags(flagsOut) : E_FAIL;		
	
	Unlock();

	return hr;
}

HRESULT DeviceSupportedCommandStore::SetFlags(const char *name, DeviceCommandFlags mask, DeviceCommandFlags value)
{
	HRESULT hr;
	DeviceSupportedCommand *command;

	Lock();

	command = Find(name, NULL);
	hr = (NULL != command) ? command->SetFlags(mask, value) : E_FAIL;		
	
	Unlock();

	return hr;
}

HRESULT DeviceSupportedCommandStore::Get(const char *name, ifc_devicesupportedcommand **command)
{
	HRESULT hr;

	if (NULL == command)
		return E_POINTER;

	Lock();

	*command = Find(name, NULL);
	if (NULL != *command)
	{
		(*command)->AddRef();
		hr = S_OK;
	}
	else
		hr = E_FAIL;

	Unlock();

	return hr;
}

HRESULT DeviceSupportedCommandStore::GetActive(ifc_devicesupportedcommand **command)
{
	return E_NOTIMPL;
}

HRESULT DeviceSupportedCommandStore::Enumerate(ifc_devicesupportedcommandenum **enumerator)
{
	HRESULT hr;
	
	Lock();
	hr = DeviceSupportedCommandEnum::CreateInstance(
				reinterpret_cast<ifc_devicesupportedcommand**>(commandList.size() ? &commandList.at(0) : nullptr), 
				commandList.size(), 
				reinterpret_cast<DeviceSupportedCommandEnum**>(enumerator));
		

	Unlock();

	return hr;
}

HRESULT DeviceSupportedCommandStore::Clone(ifc_devicesupportedcommandstore **instance, BOOL fullCopy)
{
	HRESULT hr;
	DeviceSupportedCommandStore *clone;
	DeviceSupportedCommand *command;

	if (NULL == instance)
		return E_POINTER;

	Lock();
		
	hr = CreateInstance(&clone);
	if (SUCCEEDED(hr))
	{
		size_t index, count;
		
		count = commandList.size();
		
		for(index = 0; index < count; index++)
		{
			command = commandList[index];
			if (FALSE != fullCopy)
			{
				DeviceSupportedCommand *commandClone;
					
				hr = command->Clone(&commandClone);
				if(SUCCEEDED(hr))
					command = commandClone;
				else
					break;
			}
			else
				command->AddRef();
			
			clone->commandList.push_back(command);
		}
	}

	Unlock();

	if (FAILED(hr) && NULL != clone)
	{
		clone->Release();
		*instance = NULL;
	}
	else
		*instance = clone;

	return hr;
}

#define CBCLASS DeviceSupportedCommandStore
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_ADD, Add)
CB(API_REMOVE, Remove)
CB(API_REMOVEALL, RemoveAll)
CB(API_GETFLAGS, GetFlags)
CB(API_SETFLAGS, SetFlags)
CB(API_GET, Get)
CB(API_GETACTIVE, GetActive)
CB(API_ENUMERATE, Enumerate)
CB(API_CLONE, Clone)
END_DISPATCH;
#undef CBCLASS
