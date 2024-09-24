#include "main.h"
#include "./deviceEventManager.h"

DeviceEventManager::DeviceEventManager()
	: ref(1)
{
	InitializeCriticalSection(&lock);
}

DeviceEventManager::~DeviceEventManager()
{
	Lock();

	size_t index = handlerList.size();
	while(index--)
	{
		ifc_deviceevent *handler = handlerList[index];
		handler->Release();
	}

	Unlock();

	DeleteCriticalSection(&lock);
}

HRESULT DeviceEventManager::CreateInstance(DeviceEventManager **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = new DeviceEventManager();

	if (NULL == *instance) 
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t DeviceEventManager::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceEventManager::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceEventManager::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DeviceEventManager))
		*object = static_cast<ifc_deviceeventmanager*>(this);
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

void DeviceEventManager::Lock()
{
	EnterCriticalSection(&lock);
}

void DeviceEventManager::Unlock()
{
	LeaveCriticalSection(&lock);
}

HRESULT DeviceEventManager::Advise(ifc_deviceevent *handler)
{
	HRESULT hr;
	size_t index;

	Lock();

	hr = S_OK;
	index = handlerList.size();

	while(index--)
	{
		if (handler == handlerList[index])
		{
			hr = E_FAIL;
			break;
		}
	}
	
	if (SUCCEEDED(hr))
	{
		handlerList.push_back(handler);
		handler->AddRef();
	}

	Unlock();

	return hr;
}

HRESULT DeviceEventManager::Unadvise(ifc_deviceevent *handler)
{
	Lock();

	HRESULT hr = S_FALSE;
	size_t index = handlerList.size();

	while(index--)
	{
		if (handler == handlerList[index])
		{
			handlerList.erase(handlerList.begin() + index);
			handler->Release();
			hr = S_OK;
			break;
		}
	}
	Unlock();

	return hr;
}

void DeviceEventManager::Notify_IconChanged(ifc_device *device)
{
	Lock();

	size_t index, count = handlerList.size();
	for(index = 0; index < count;index++)
	{
		ifc_deviceevent *handler = handlerList[index];
		handler->IconChanged(device);
	}
	Unlock();
}

void DeviceEventManager::Notify_DisplayNameChanged(ifc_device *device, const wchar_t *displayName)
{
	Lock();
	size_t index, count = handlerList.size();
	for(index = 0; index < count;index++)
	{
		ifc_deviceevent *handler = handlerList[index];
		handler->DisplayNameChanged(device, displayName);
	}
	Unlock();
}

void DeviceEventManager::Notify_AttachmentChanged(ifc_device *device, BOOL attached)
{
	Lock();
	size_t index, count = handlerList.size();
	for(index = 0; index < count;index++)
	{
		ifc_deviceevent *handler = handlerList[index];
		handler->AttachmentChanged(device, attached);
	}
	Unlock();
}

void DeviceEventManager::Notify_VisibilityChanged(ifc_device *device, BOOL visible)
{
	Lock();
	size_t index, count = handlerList.size();
	for(index = 0; index < count;index++)
	{
		ifc_deviceevent *handler = handlerList[index];
		handler->VisibilityChanged(device, visible);
	}
	Unlock();
}

void DeviceEventManager::Notify_TotalSpaceChanged(ifc_device *device, uint64_t space)
{
	Lock();
	size_t index, count = handlerList.size();
	for(index = 0; index < count;index++)
	{
		ifc_deviceevent *handler = handlerList[index];
		handler->TotalSpaceChanged(device, space);
	}
	Unlock();
}

void DeviceEventManager::Notify_UsedSpaceChanged(ifc_device *device, uint64_t space)
{
	Lock();
	size_t index, count = handlerList.size();
	for(index = 0; index < count;index++)
	{
		ifc_deviceevent *handler = handlerList[index];
		handler->UsedSpaceChanged(device, space);
	}
	Unlock();
}

void DeviceEventManager::Notfiy_CommandChanged(ifc_device *device)
{
	Lock();
	size_t index, count = handlerList.size();
	for(index = 0; index < count;index++)
	{
		ifc_deviceevent *handler = handlerList[index];
		handler->CommandChanged(device);
	}
	Unlock();
}

void DeviceEventManager::Notify_ActivityStarted(ifc_device *device, ifc_deviceactivity *activity)
{
	Lock();
	size_t index, count = handlerList.size();
	for(index = 0; index < count;index++)
	{
		ifc_deviceevent *handler = handlerList[index];
		handler->ActivityStarted(device, activity);
	}
	Unlock();
}

void DeviceEventManager::Notify_ActivityFinished(ifc_device *device, ifc_deviceactivity *activity)
{
	Lock();
	size_t index, count = handlerList.size();
	for(index = 0; index < count;index++)
	{
		ifc_deviceevent *handler = handlerList[index];
		handler->ActivityFinished(device, activity);
	}
	Unlock();
}

void DeviceEventManager::Notify_ActivityChanged(ifc_device *device, ifc_deviceactivity *activity)
{
	Lock();
	size_t index, count = handlerList.size();
	for(index = 0; index < count;index++)
	{
		ifc_deviceevent *handler = handlerList[index];
		handler->ActivityChanged(device, activity);
	}
	Unlock();
}

void DeviceEventManager::Notify_ModelChanged(ifc_device *device, const wchar_t *model)
{
	Lock();
	size_t index, count = handlerList.size();
	for(index = 0; index < count;index++)
	{
		ifc_deviceevent *handler = handlerList[index];
		handler->ModelChanged(device, model);
	}
	Unlock();
}

void DeviceEventManager::Notify_StatusChanged(ifc_device *device, const wchar_t *status)
{
	Lock();
	size_t index, count = handlerList.size();
	for(index = 0; index < count;index++)
	{
		ifc_deviceevent *handler = handlerList[index];
		handler->StatusChanged(device, status);
	}
	Unlock();
}


#define CBCLASS DeviceEventManager
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_ADVISE, Advise)
CB(API_UNADVISE, Unadvise)
VCB(API_NOTIFY_ICONCHANGED, Notify_IconChanged)
VCB(API_NOTIFY_DISPLAYNAMECHANGED, Notify_DisplayNameChanged)
VCB(API_NOTIFY_ATTACHMENTCHANGED, Notify_AttachmentChanged)
VCB(API_NOTIFY_VISIBILITYCHANGED, Notify_VisibilityChanged)
VCB(API_NOTIFY_TOTALSPACECHANGED, Notify_TotalSpaceChanged)
VCB(API_NOTIFY_USEDSPACECHANGED, Notify_UsedSpaceChanged)
VCB(API_NOTIFY_COMMANDCHANGED, Notfiy_CommandChanged)
VCB(API_NOTIFY_ACTIVITYSTARTED, Notify_ActivityStarted)
VCB(API_NOTIFY_ACTIVITYFINISHED, Notify_ActivityFinished)
VCB(API_NOTIFY_ACTIVITYCHANGED, Notify_ActivityChanged)
VCB(API_NOTIFY_MODELCHANGED, Notify_ModelChanged)
VCB(API_NOTIFY_STATUSCHANGED, Notify_StatusChanged)
END_DISPATCH;
#undef CBCLASS
