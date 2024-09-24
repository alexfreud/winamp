#include "main.h"
#include "./deviceManagerHandler.h"

DeviceManagerHandler::DeviceManagerHandler() 
	: ref(1), relayWindow(NULL)
{
}

DeviceManagerHandler::~DeviceManagerHandler()
{
}

HRESULT DeviceManagerHandler::CreateInstance(DeviceManagerHandler **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = new DeviceManagerHandler();

	if (NULL == *instance) 
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t DeviceManagerHandler::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceManagerHandler::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceManagerHandler::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DeviceManagerEvent))
		*object = static_cast<ifc_devicemanagerevent*>(this);
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

void DeviceManagerHandler::TypeAdded(api_devicemanager *manager, ifc_devicetype *type)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_TYPE(relayWindow, type, Event_TypeRegistered);
}

void DeviceManagerHandler::TypeRemoved(api_devicemanager *manager, ifc_devicetype *type)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_TYPE(relayWindow, type, Event_TypeUnregistered);
}

void DeviceManagerHandler::ConnectionAdded(api_devicemanager *manager, ifc_deviceconnection *connection)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_CONNECTION(relayWindow, connection, Event_ConnectionRegistered);
}

void DeviceManagerHandler::ConnectionRemoved(api_devicemanager *manager, ifc_deviceconnection *connection)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_CONNECTION(relayWindow, connection, Event_ConnectionUnregistered);
}

void DeviceManagerHandler::CommandAdded(api_devicemanager *manager, ifc_devicecommand *command)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_COMMAND(relayWindow, command, Event_CommandRegistered);
}

void DeviceManagerHandler::CommandRemoved(api_devicemanager *manager, ifc_devicecommand *command)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_COMMAND(relayWindow, command, Event_CommandUnregistered);
}

void DeviceManagerHandler::DeviceAdded(api_devicemanager *manager, ifc_device *device)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceAdded);
}

void DeviceManagerHandler::DeviceRemoved(api_devicemanager *manager, ifc_device *device)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceRemoved);
}

void DeviceManagerHandler::DiscoveryStarted(api_devicemanager *manager)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DISCOVERY(relayWindow, manager, Event_DiscoveryStarted);
}

void DeviceManagerHandler::DiscoveryFinished(api_devicemanager *manager)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DISCOVERY(relayWindow, manager, Event_DiscoveryFinished);
}

HRESULT DeviceManagerHandler::SetRelayWindow(HWND hwnd)
{
	relayWindow = hwnd;
	return S_OK;
}

HRESULT DeviceManagerHandler::Advise(api_devicemanager *manager)
{
	HRESULT hr;
	
	if (NULL == manager)
		return E_INVALIDARG;

	hr = manager->Advise(this);
	if (FAILED(hr))
		return hr;
	
	return hr;
}

HRESULT DeviceManagerHandler::Unadvise(api_devicemanager *manager)
{
	HRESULT hr;

	if (NULL == manager)
		return E_INVALIDARG;

	hr = manager->Unadvise(this);
	if (FAILED(hr))
		return hr;


	return hr;
}


#define CBCLASS DeviceManagerHandler
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
VCB(API_TYPEADDED, TypeAdded)
VCB(API_TYPEREMOVED, TypeRemoved)
VCB(API_CONNECTIONADDED, ConnectionAdded)
VCB(API_CONNECTIONREMOVED, ConnectionRemoved)
VCB(API_COMMANDADDED, CommandAdded)
VCB(API_COMMANDREMOVED, CommandRemoved)
VCB(API_DEVICEADDED, DeviceAdded)
VCB(API_DEVICEREMOVED, DeviceRemoved)
VCB(API_DISCOVERYSTARTED, DiscoveryStarted)
VCB(API_DISCOVERYFINISHED, DiscoveryFinished)
END_DISPATCH;
#undef CBCLASS