#include "main.h"
#include "./deviceManager.h"
#include "./deviceEventManager.h"

static void
DeviceManager_ReleaseDispatchables(Dispatchable** buffer, size_t count)
{
	if(NULL == buffer)
		return;

	while(count--)
	{
		Dispatchable *object = buffer[count];
		if (NULL != object)
			object->Release();
	}
}

DeviceManager::DeviceManager() 
	: ref(1), 
	#pragma warning(push)
	#pragma warning(disable:4355)
	typeStore(ObjectAddedCallback, ObjectRemovedCallback, this),
	connectionStore(ObjectAddedCallback, ObjectRemovedCallback, this),
	commandStore(ObjectAddedCallback, ObjectRemovedCallback, this),
	deviceStore(ObjectAddedCallback, ObjectRemovedCallback, this)
	#pragma warning(pop)
{	
	InitializeCriticalSection(&eventLock);
	InitializeCriticalSection(&providerLock);
}


DeviceManager::~DeviceManager()
{
	EnterCriticalSection(&providerLock);
	DeviceManager_ReleaseDispatchables((Dispatchable**)(providerList.size() ? &providerList.at(0) : nullptr), providerList.size());
	providerList.clear();
	LeaveCriticalSection(&providerLock);
	DeleteCriticalSection(&providerLock);

	EnterCriticalSection(&eventLock);
	DeviceManager_ReleaseDispatchables((Dispatchable**)(eventList.size() ? &eventList.at(0) : nullptr), eventList.size());
	eventList.clear();
	LeaveCriticalSection(&eventLock);
	DeleteCriticalSection(&eventLock);
}

HRESULT DeviceManager::CreateInstance(DeviceManager **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = new DeviceManager();

	if (NULL == *instance) 
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t DeviceManager::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceManager::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceManager::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, DeviceManagerGUID))
		*object = static_cast<api_devicemanager*>(this);
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

size_t DeviceManager::TypeRegister(ifc_devicetype **types, size_t count)
{
	return typeStore.AddRange((ifc_deviceobject**)types, count);
}

size_t DeviceManager::TypeRegisterIndirect(const char **names, size_t count, DeviceTypeCreator callback, void *user)
{
	return typeStore.AddIndirect(names, count, (DeviceObjectCreator)callback, user);
}

HRESULT DeviceManager::TypeUnregister(const char *name)
{
	return typeStore.Remove(name);
}

HRESULT DeviceManager::TypeFind(const char *name, ifc_devicetype **type)
{
	return typeStore.Find(name, (ifc_deviceobject**)type);
}

HRESULT DeviceManager::TypeEnumerate(ifc_deviceobjectenum **enumerator)
{
	return typeStore.Enumerate((DeviceObjectEnum**)enumerator);
}

size_t DeviceManager::ConnectionRegister(ifc_deviceconnection **connections, size_t count)
{
	
	return connectionStore.AddRange((ifc_deviceobject**)connections, count);
}

size_t DeviceManager::ConnectionRegisterIndirect(const char **names, size_t count, DeviceConnectionCreator callback, void *user)
{
	return connectionStore.AddIndirect(names, count, (DeviceObjectCreator)callback, user);
}

HRESULT DeviceManager::ConnectionUnregister(const char *name)
{
	return connectionStore.Remove(name);
}

HRESULT DeviceManager::ConnectionFind(const char *name, ifc_deviceconnection **connection)
{
	return connectionStore.Find(name, (ifc_deviceobject**)connection);
}

HRESULT DeviceManager::ConnectionEnumerate(ifc_deviceobjectenum **enumerator)
{
	return connectionStore.Enumerate((DeviceObjectEnum**)enumerator);
}

size_t DeviceManager::CommandRegister(ifc_devicecommand **commands, size_t count)
{
	return commandStore.AddRange((ifc_deviceobject**)commands, count);
}

size_t DeviceManager::CommandRegisterIndirect(const char **names, size_t count, DeviceCommandCreator callback, void *user)
{
	return commandStore.AddIndirect(names, count, (DeviceObjectCreator)callback, user);
}

HRESULT DeviceManager::CommandUnregister(const char *name)
{
	return commandStore.Remove(name);
}

HRESULT DeviceManager::CommandFind(const char *name, ifc_devicecommand **command)
{
	return commandStore.Find(name, (ifc_deviceobject**)command);
}

HRESULT DeviceManager::CommandEnumerate(ifc_deviceobjectenum **enumerator)
{
	return commandStore.Enumerate((DeviceObjectEnum**)enumerator);
}

size_t DeviceManager::DeviceRegister(ifc_device **devices, size_t count)
{
	return deviceStore.AddRange((ifc_deviceobject**)devices, count);
}

HRESULT DeviceManager::DeviceUnregister(const char *name)
{
	return deviceStore.Remove(name);
}

HRESULT DeviceManager::DeviceFind(const char *name, ifc_device **device)
{
	return deviceStore.Find(name, (ifc_deviceobject**)device);
}

HRESULT DeviceManager::DeviceEnumerate(ifc_deviceobjectenum **enumerator)
{
	return deviceStore.Enumerate((DeviceObjectEnum**)enumerator);
}


HRESULT DeviceManager::IsDiscoveryActive()
{
	return (FALSE != discoveryMonitor.IsActive()) ? S_OK : S_FALSE;
}

HRESULT DeviceManager::BeginDiscovery()
{
	size_t index, started;

	EnterCriticalSection(&providerLock);
	
	started = 0;
	index = providerList.size();

	while(index--)
	{	
		if (providerList[index] && SUCCEEDED(providerList[index]->BeginDiscovery((api_devicemanager*)this)))
			started++;
	}
		
	LeaveCriticalSection(&providerLock);

	return (0 != started) ? S_OK : E_FAIL;
}

HRESULT DeviceManager::CancelDiscovery()
{
	size_t index;

	EnterCriticalSection(&providerLock);
	
	index = providerList.size();

	while(index--)
	{	
		providerList[index]->CancelDiscovery();
	}
		
	LeaveCriticalSection(&providerLock);

	return S_OK;
}

HRESULT DeviceManager::SetProviderActive(ifc_deviceprovider *provider, BOOL activeState)
{
	
	if (FALSE != activeState)
	{
		if (FALSE != discoveryMonitor.Register(provider))
			EventDiscoveryStarted();
	}
	else
	{
		if (FALSE != discoveryMonitor.Unregister(provider))
			EventDiscoveryFinished();
	}
		
	return S_OK;
}

HRESULT DeviceManager::RegisterProvider(ifc_deviceprovider *provider)
{
	HRESULT hr;
	size_t index;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = S_OK;

	EnterCriticalSection(&providerLock);
	
	index = providerList.size();
	while(index--)
	{	
		if (provider == providerList[index])
		{
			hr = S_FALSE;
			break;
		}
	}

	if(S_OK == hr)
	{
		providerList.push_back(provider);
		provider->AddRef();

		SetProviderActive(provider, (S_OK == provider->GetActive()));
	}
	
	LeaveCriticalSection(&providerLock);
		
	return hr;
}

HRESULT DeviceManager::UnregisterProvider(ifc_deviceprovider *provider)
{
	HRESULT hr;
	size_t index;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = S_FALSE;

	EnterCriticalSection(&providerLock);
	
	index = providerList.size();
	while(index--)
	{	
		if (provider == providerList[index])
		{
			ifc_deviceprovider *object;
			object = providerList[index];

			SetProviderActive(object, (S_OK == object->GetActive()));

			providerList.erase(providerList.begin() + index);
			object->Release();

			hr = S_OK;
			break;
		}
	}
	
	LeaveCriticalSection(&providerLock);

	return hr;
}

HRESULT DeviceManager::Advise(ifc_devicemanagerevent *handler)
{
	HRESULT hr;
	size_t index;

	if (NULL == handler)
		return E_INVALIDARG;

	hr = S_OK;

	EnterCriticalSection(&eventLock);
	
	index = eventList.size();
	while(index--)
	{	
		if (handler == eventList[index])
		{
			hr = S_FALSE;
			break;
		}
	}

	if(S_OK == hr)
	{
		eventList.push_back(handler);
		handler->AddRef();
	}
	
	LeaveCriticalSection(&eventLock);

	return hr;
}

HRESULT DeviceManager::Unadvise(ifc_devicemanagerevent *handler)
{
	HRESULT hr;
	size_t index;

	if (NULL == handler)
		return E_INVALIDARG;

	hr = S_FALSE;

	EnterCriticalSection(&eventLock);
	
	index = eventList.size();
	while(index--)
	{	
		if (handler == eventList[index])
		{
			ifc_devicemanagerevent *object;
			object = eventList[index];

			eventList.erase(eventList.begin() + index);
			object->Release();

			hr = S_OK;
			break;
		}
	}
	
	LeaveCriticalSection(&eventLock);

	return hr;
}

HRESULT DeviceManager::CreateDeviceEventManager(ifc_deviceeventmanager **eventManager)
{
	return DeviceEventManager::CreateInstance(
				reinterpret_cast<DeviceEventManager**>(eventManager));
}

HRESULT DeviceManager::CreateSupportedCommandStore(ifc_devicesupportedcommandstore **store)
{
	return DeviceSupportedCommandStore::CreateInstance(
				reinterpret_cast<DeviceSupportedCommandStore**>(store));
}

HRESULT DeviceManager::CreateSupportedCommandEnum(ifc_devicesupportedcommand **commands, size_t count, ifc_devicesupportedcommandenum **enumerator)
{
	return DeviceSupportedCommandEnum::CreateInstance(commands, count, 
				reinterpret_cast<DeviceSupportedCommandEnum**>(enumerator));
}

HRESULT DeviceManager::CreateIconStore(ifc_deviceiconstore **store)
{
	return DeviceIconStore::CreateInstance(
				reinterpret_cast<DeviceIconStore**>(store));
}

HRESULT DeviceManager::CreateType(const char *name, ifc_devicetype **type)
{
	return DeviceType::CreateInstance(name, reinterpret_cast<DeviceType**>(type));
}

HRESULT DeviceManager::CreateCommand(const char *name, ifc_devicecommand **command)
{
	return DeviceCommand::CreateInstance(name, reinterpret_cast<DeviceCommand**>(command));

}

HRESULT DeviceManager::CreateConnection(const char *name, ifc_deviceconnection **connection)
{
	return DeviceConnection::CreateInstance(name, reinterpret_cast<DeviceConnection**>(connection));
}

void
DeviceManager::ObjectAddedCallback(DeviceObjectStore *store, ifc_deviceobject *object, void *userData)
{
	DeviceManager *manager;
	manager = (DeviceManager*)userData;

	if (NULL == manager)
		return;
	
	if (store == &manager->typeStore)
		manager->EventTypeAdded((ifc_devicetype*)object);
	else if (store == &manager->connectionStore)
		manager->EventConnectionAdded((ifc_deviceconnection*)object);
	else if (store == &manager->commandStore)
		manager->EventCommandAdded((ifc_devicecommand*)object);
	else if (store == &manager->deviceStore)
		manager->EventDeviceAdded((ifc_device*)object);
}

void 
DeviceManager::ObjectRemovedCallback(DeviceObjectStore *store, ifc_deviceobject *object, void *userData)
{
	DeviceManager *manager;
	manager = (DeviceManager*)userData;

	if (NULL == manager)
		return;
	
	if (store == &manager->typeStore)
		manager->EventTypeRemoved((ifc_devicetype*)object);
	else if (store == &manager->connectionStore)
		manager->EventConnectionRemoved((ifc_deviceconnection*)object);
	else if (store == &manager->commandStore)
		manager->EventCommandRemoved((ifc_devicecommand*)object);
	else if (store == &manager->deviceStore)
		manager->EventDeviceRemoved((ifc_device*)object);
}


void DeviceManager::EventTypeAdded(ifc_devicetype *type)
{
	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while(index--)
	{
		ifc_devicemanagerevent *handler = eventList[index];
		handler->TypeAdded((api_devicemanager*)this, type);
	}

	LeaveCriticalSection(&eventLock);
}

void DeviceManager::EventTypeRemoved(ifc_devicetype *type)
{
	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while(index--)
	{
		ifc_devicemanagerevent *handler = eventList[index];
		handler->TypeRemoved((api_devicemanager*)this, type);
	}

	LeaveCriticalSection(&eventLock);
}

void DeviceManager::EventConnectionAdded(ifc_deviceconnection *connection)
{
	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while(index--)
	{
		ifc_devicemanagerevent *handler = eventList[index];
		handler->ConnectionAdded((api_devicemanager*)this, connection);
	}

	LeaveCriticalSection(&eventLock);
}

void DeviceManager::EventConnectionRemoved(ifc_deviceconnection *connection)
{
	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while(index--)
	{
		ifc_devicemanagerevent *handler = eventList[index];
		handler->ConnectionRemoved((api_devicemanager*)this, connection);
	}
	
	LeaveCriticalSection(&eventLock);
}

void DeviceManager::EventCommandAdded(ifc_devicecommand *command)
{
	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while(index--)
	{
		ifc_devicemanagerevent *handler = eventList[index];
		handler->CommandAdded((api_devicemanager*)this, command);
	}

	LeaveCriticalSection(&eventLock);
}

void DeviceManager::EventCommandRemoved(ifc_devicecommand *command)
{
	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while(index--)
	{
		ifc_devicemanagerevent *handler = eventList[index];
		handler->CommandRemoved((api_devicemanager*)this, command);
	}

	LeaveCriticalSection(&eventLock);
}

void DeviceManager::EventDeviceAdded(ifc_device *device)
{
	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while(index--)
	{
		ifc_devicemanagerevent *handler = eventList[index];
		handler->DeviceAdded((api_devicemanager*)this, device);
	}

	LeaveCriticalSection(&eventLock);
}

void DeviceManager::EventDeviceRemoved(ifc_device *device)
{
	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while(index--)
	{
		ifc_devicemanagerevent *handler = eventList[index];
		handler->DeviceRemoved((api_devicemanager*)this, device);
	}

	LeaveCriticalSection(&eventLock);
}

void DeviceManager::EventDiscoveryStarted()
{
	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while(index--)
	{
		ifc_devicemanagerevent *handler = eventList[index];
		handler->DiscoveryStarted((api_devicemanager*)this);
	}
	
	LeaveCriticalSection(&eventLock);
}

void DeviceManager::EventDiscoveryFinished()
{
	EnterCriticalSection(&eventLock);

	size_t index = eventList.size();
	while(index--)
	{
		ifc_devicemanagerevent *handler = eventList[index];
		handler->DiscoveryFinished((api_devicemanager*)this);
	}
	
	LeaveCriticalSection(&eventLock);
}

#define CBCLASS DeviceManager
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_TYPEREGISTER, TypeRegister)
CB(API_TYPEREGISTERINDIRECT, TypeRegisterIndirect)
CB(API_TYPEUNREGISTER, TypeUnregister)
CB(API_TYPEFIND, TypeFind)
CB(API_TYPEENUMERATE, TypeEnumerate)
CB(API_CONNECTIONREGISTER, ConnectionRegister)
CB(API_CONNECTIONREGISTERINDIRECT, ConnectionRegisterIndirect)
CB(API_CONNECTIONUNREGISTER, ConnectionUnregister)
CB(API_CONNECTIONFIND, ConnectionFind)
CB(API_CONNECTIONENUMERATE, ConnectionEnumerate)
CB(API_COMMANDREGISTER, CommandRegister)
CB(API_COMMANDREGISTERINDIRECT, CommandRegisterIndirect)
CB(API_COMMANDUNREGISTER, CommandUnregister)
CB(API_COMMANDFIND, CommandFind)
CB(API_COMMANDENUMERATE, CommandEnumerate)
CB(API_DEVICEREGISTER, DeviceRegister)
CB(API_DEVICEUNREGISTER, DeviceUnregister)
CB(API_DEVICEFIND, DeviceFind)
CB(API_DEVICEENUMERATE, DeviceEnumerate)
CB(API_ISDISCOVERYACTIVE, IsDiscoveryActive)
CB(API_BEGINDISCOVERY, BeginDiscovery)
CB(API_CANCELDISCOVERY, CancelDiscovery)
CB(API_REGISTERPROVIDER, RegisterProvider)
CB(API_UNREGISTERPROVIDER, UnregisterProvider)
CB(API_SETPROVIDERACTIVE, SetProviderActive)
CB(API_ADVISE, Advise)
CB(API_UNADVISE, Unadvise)
CB(API_CREATEDEVICEEVENTMANAGER, CreateDeviceEventManager)
CB(API_CREATESUPPORTEDCOMMANDSTORE, CreateSupportedCommandStore)
CB(API_CREATESUPPORTEDCOMMANDENUM, CreateSupportedCommandEnum)
CB(API_CREATEICONSTORE, CreateIconStore)
CB(API_CREATETYPE, CreateType)
CB(API_CREATECOMMAND, CreateCommand)
CB(API_CREATECONNECTION, CreateConnection)
END_DISPATCH;
#undef CBCLASS