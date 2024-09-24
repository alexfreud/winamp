#include "main.h"
#include "./deviceHandler.h"

DeviceHandler::DeviceHandler() 
	: ref(1), relayWindow(NULL)
{
}

DeviceHandler::~DeviceHandler()
{
}

HRESULT DeviceHandler::CreateInstance(DeviceHandler **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = new DeviceHandler();

	if (NULL == *instance) 
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t DeviceHandler::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceHandler::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceHandler::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DeviceEvent))
		*object = static_cast<ifc_deviceevent*>(this);
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


void DeviceHandler::IconChanged(ifc_device *device)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceIconChanged);
}

void DeviceHandler::DisplayNameChanged(ifc_device *device, const wchar_t *displayName)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceDisplayNameChanged);
}

void DeviceHandler::AttachmentChanged(ifc_device *device, BOOL attached)
{
	if (NULL != relayWindow)
	{
		if (FALSE != attached)
		{
			EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceAttached);
		}
		else
		{
			EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceDetached);
		}
	}
}

void DeviceHandler::VisibilityChanged(ifc_device *device, BOOL visible)
{
	if (NULL != relayWindow)
	{
		if (FALSE == visible)
		{
			EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceHidden);
		}
		else
		{
			EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceShown);
		}
	}
}

void DeviceHandler::TotalSpaceChanged(ifc_device *device, size_t space)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceTotalSpaceChanged);
}

void DeviceHandler::UsedSpaceChanged(ifc_device *device, size_t space)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceUsedSpaceChanged);
}

void DeviceHandler::CommandChanged(ifc_device *device)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceCommandChanged);
}


void DeviceHandler::ActivityStarted(ifc_device *device, ifc_deviceactivity *activity)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceActivityStarted);
}

void DeviceHandler::ActivityFinished(ifc_device *device, ifc_deviceactivity *activity)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceActivityFinished);
}

void DeviceHandler::ActivityChanged(ifc_device *device, ifc_deviceactivity *activity)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceActivityChanged);
}

void DeviceHandler::ModelChanged(ifc_device *device, const wchar_t *model)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceModelChanged);
}

void DeviceHandler::StatusChanged(ifc_device *device, const wchar_t *status)
{
	if (NULL != relayWindow)
		EVENTRELAY_NOTIFY_DEVICE(relayWindow, device, Event_DeviceStatusChanged);
}

HRESULT DeviceHandler::SetRelayWindow(HWND hwnd)
{
	relayWindow = hwnd;
	return S_OK;
}

HRESULT DeviceHandler::Advise(ifc_device *device)
{
	HRESULT hr;

	if (NULL == device)
		return E_INVALIDARG;

	hr = device->Advise(this);
	if (FAILED(hr))
		return hr;
	
	return hr;
}

HRESULT DeviceHandler::Unadvise(ifc_device *device)
{
	HRESULT hr;

	if (NULL == device)
		return E_INVALIDARG;

	hr = device->Unadvise(this);
	if (FAILED(hr))
		return hr;

	return hr;
}

#define CBCLASS DeviceHandler
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
VCB(API_ICONCHANGED, IconChanged)
VCB(API_DISPLAYNAMECHANGED, DisplayNameChanged)
VCB(API_ATTACHMENTCHANGED, AttachmentChanged)
VCB(API_VISIBILITYCHANGED, VisibilityChanged)
VCB(API_TOTALSPACECHANGED, TotalSpaceChanged)
VCB(API_USEDSPACECHANGED, UsedSpaceChanged)
VCB(API_COMMANDCHANGED, CommandChanged)
VCB(API_ACTIVITYSTARTED, ActivityStarted)
VCB(API_ACTIVITYFINISHED, ActivityFinished)
VCB(API_ACTIVITYCHANGED, ActivityChanged)
VCB(API_MODELCHANGED, ModelChanged)
VCB(API_STATUSCHANGED, StatusChanged)
END_DISPATCH;
#undef CBCLASS