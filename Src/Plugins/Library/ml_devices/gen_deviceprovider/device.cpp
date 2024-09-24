#include "main.h"
#include "./device.h"

#include <strsafe.h>

Device::Device() 
	: ref(1), name(NULL), type(NULL), connection(NULL), displayName(NULL), 
	 totalSpace(0), usedSpace(0), attached(FALSE), hidden(FALSE), 
	 connected(FALSE), activity(NULL), model(NULL), status(NULL)
{
	
	InitializeCriticalSection(&lock);

	if (NULL == WASABI_API_DEVICES || 
		FAILED(WASABI_API_DEVICES->CreateDeviceEventManager(&eventManager)))
	{
		eventManager = NULL;
	}

	if (NULL == WASABI_API_DEVICES ||
		FAILED(WASABI_API_DEVICES->CreateIconStore(&iconStore)))
	{
		iconStore = NULL;
	}

	if (NULL == WASABI_API_DEVICES ||
		FAILED(WASABI_API_DEVICES->CreateSupportedCommandStore(&commands)))
	{
		commands = NULL;
	}

}

Device::~Device()
{
	DeviceActivity *activityCopy;

	Lock();

	if (NULL != activity)
	{
		activity->SetUser(NULL);
		activityCopy = activity;
		activityCopy->AddRef();
	}		
	else
		activityCopy = NULL;

	AnsiString_Free(name);
	AnsiString_Free(type);
	AnsiString_Free(connection);
	String_Free(displayName);
	String_Free(model);
	String_Free(status);
	
	if (NULL != commands)
		commands->Release();

	if (NULL != iconStore)
		iconStore->Release();
	
	if (NULL != eventManager)
		eventManager->Release();
	
	Unlock();

	if (NULL != activityCopy)
	{
		
		activityCopy->Stop();
		activityCopy->Release();
	}

	DeleteCriticalSection(&lock);
}

HRESULT Device::CreateInstance(const char *name, const char *type, const char *connection, Device**instance)
{
	Device *self;

	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;
	
	self = new Device();
	if (NULL == self) 
		return E_OUTOFMEMORY;

	self->name = AnsiString_Duplicate(name);
	self->type = AnsiString_Duplicate(type);
	self->connection = AnsiString_Duplicate(connection);
	
	*instance = self;
	return S_OK;
}

size_t Device::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t Device::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int Device::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_Device))
		*object = static_cast<ifc_device*>(this);
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

void Device::Lock()
{
	EnterCriticalSection(&lock);
}

void Device::Unlock()
{
	LeaveCriticalSection(&lock);
}

const char *Device::GetName()
{
	return name;
}

const char *Device::GetType()
{
	return type;
}

const char *Device::GetConnection()
{
	return connection;
}

HRESULT Device::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
{
	if (NULL == buffer)
		return E_POINTER;

	if (NULL == iconStore)
		return E_UNEXPECTED;

	return iconStore->Get(buffer, bufferSize, width, height);
}

HRESULT Device::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	HRESULT hr;

	if (NULL == buffer)
		return E_POINTER;

	Lock();

	if (0 == String_CopyTo(buffer, displayName, bufferSize) &&
		FALSE == IS_STRING_EMPTY(displayName))
	{
		hr = E_FAIL;
	}
	else
		hr = S_OK;

	Unlock();

	return hr;
}

BOOL Device::GetHidden()
{
	return hidden;
}

HRESULT Device::GetTotalSpace(uint64_t *size)
{
	if (NULL == size)
		return E_POINTER;

	Lock();

	*size = totalSpace;

	Unlock();

	return S_OK;
}

HRESULT Device::GetUsedSpace(uint64_t *size)
{
	if (NULL == size)
		return E_POINTER;

	Lock();

	*size = usedSpace;

	Unlock();

	return S_OK;
}

BOOL Device::GetAttached()
{
	return attached;
}

HRESULT Device::Attach(HWND hostWindow)
{
	HRESULT hr;

	Lock();

	if (FALSE != attached)
		hr = S_FALSE;
	else
	{
		attached = TRUE;
		hr = S_OK;
	}
	
	Unlock();

	if (S_OK == hr && NULL != eventManager)
		eventManager->Notify_AttachmentChanged(this, attached);

	return hr;
}

HRESULT Device::Detach(HWND hostWindow)
{
	HRESULT hr;

	Lock();

	if (FALSE == attached)
		hr = S_FALSE;
	else
	{
		attached = FALSE;
		hr = S_OK;
	}

	Unlock();

	if (S_OK == hr && NULL != eventManager)
		eventManager->Notify_AttachmentChanged(this, attached);
	
	return hr;
}

HRESULT Device::EnumerateCommands(ifc_devicesupportedcommandenum **enumerator, DeviceCommandContext context)
{
	if (NULL == commands)
		return E_UNEXPECTED;

	return commands->Enumerate(enumerator);
}

HRESULT Device::SendCommand(const char *command, HWND hostWindow, ULONG_PTR param)
{
	const wchar_t *commandName;
	wchar_t message[1024];

	if (NULL == command)
		return E_POINTER;

	if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, command, -1, "sync", -1))
	{
		StartSyncActivity(hostWindow);
		return S_OK;
	}
	else if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, command, -1, "eject", -1))
		commandName = L"Eject";
	else if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, command, -1, "detach", -1))
	{
		Detach(hostWindow);
		return S_OK;
	}
	else if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, command, -1, "settings", -1))
		commandName = L"Settings";
	else
		return E_NOTIMPL;
	
	StringCchPrintf(message, ARRAYSIZE(message), L"%s command received", commandName);
	MessageBox(hostWindow, message, L"Device command test", MB_OK | MB_ICONINFORMATION);


	return S_OK;
}

HRESULT Device::GetCommandFlags(const char *command, DeviceCommandFlags *flags)
{
	if (NULL == commands)
		return E_UNEXPECTED;
	
	return commands->GetFlags(command, flags);
}

HRESULT Device::GetActivity(ifc_deviceactivity **activityOut)
{
	HRESULT hr;

	if (NULL == activityOut)
		return E_POINTER;
	
	Lock();

	*activityOut = activity;
	
	if (NULL != activity)
	{
		activity->AddRef();
		hr = S_OK;
	}
	else
		hr = S_FALSE;

	Unlock();

	return hr;
}

HRESULT Device::Advise(ifc_deviceevent *handler)
{
	if (NULL == eventManager)
		return E_UNEXPECTED;
	
	return eventManager->Advise(handler);
}

HRESULT Device::Unadvise(ifc_deviceevent *handler)
{
	if (NULL == eventManager)
		return E_UNEXPECTED;
	
	return eventManager->Unadvise(handler);
}

HWND Device::CreateView(HWND parentWindow)
{
	return DeviceView_CreateWindow(parentWindow, this);
}

void Device::SetNavigationItem(void *navigationItem)
{
}

HRESULT Device::GetModel(wchar_t *buffer, size_t bufferSize)
{
	HRESULT hr;

	if (NULL == buffer)
		return E_POINTER;

	Lock();

	if (0 == String_CopyTo(buffer, model, bufferSize) &&
		FALSE == IS_STRING_EMPTY(model))
	{
		hr = E_FAIL;
	}
	else
		hr = S_OK;

	Unlock();

	return hr;
}
HRESULT Device::GetStatus(wchar_t *buffer, size_t bufferSize)
{
	HRESULT hr;

	if (NULL == buffer)
		return E_POINTER;

	Lock();

	if (0 == String_CopyTo(buffer, status, bufferSize) &&
		FALSE == IS_STRING_EMPTY(status))
	{
		hr = E_FAIL;
	}
	else
		hr = S_OK;

	Unlock();

	return hr;
}
HRESULT Device::SetConnection(const char *con)
{
	Lock();
	
	AnsiString_Free(connection);
	connection = AnsiString_Duplicate(con);

	Unlock();

	return S_OK;
}

HRESULT Device::SetDisplayName(const wchar_t *name)
{
	HRESULT hr;

	Lock();

	if (NULL == name && NULL == displayName)
		hr = S_FALSE;
	else
	{
		if (NULL != displayName && 
			CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, 0, name, -1, displayName, -1))
		{
			hr = S_FALSE;
		}
		else
		{
			wchar_t *string;

			string = String_Duplicate(name);
			if (NULL == string && NULL != name)
				hr = E_FAIL;
			else
			{
				String_Free(displayName);
				displayName = string;

				if (NULL != eventManager)
					eventManager->Notify_DisplayNameChanged(this, displayName);

				hr = S_OK;
			}
		}
	}

	Unlock();

	return hr;
}

HRESULT Device::SetTotalSpace(uint64_t size)
{
	Lock();

	totalSpace = size;

	if (NULL != eventManager)
		eventManager->Notify_TotalSpaceChanged(this, totalSpace);

	Unlock();

	return S_OK;
}

HRESULT Device::SetUsedSpace(uint64_t size)
{
	Lock();

	usedSpace = size;

	if (NULL != eventManager)
		eventManager->Notify_UsedSpaceChanged(this, usedSpace);

	Unlock();

	return S_OK;
}

HRESULT Device::SetModel(const wchar_t *deviceModel)
{
	HRESULT hr;

	Lock();

	if (NULL == deviceModel && NULL == model)
		hr = S_FALSE;
	else
	{
		if (NULL != model && 
			CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, 0, deviceModel, -1, model, -1))
		{
			hr = S_FALSE;
		}
		else
		{
			wchar_t *string;

			string = String_Duplicate(deviceModel);
			if (NULL == string && NULL != deviceModel)
				hr = E_FAIL;
			else
			{
				String_Free(model);
				model = string;

				if (NULL != eventManager)
					eventManager->Notify_ModelChanged(this, model);

				hr = S_OK;
			}
		}
	}

	Unlock();

	return hr;
}

HRESULT Device::SetStatus(const wchar_t *deviceStatus)
{
	HRESULT hr;

	Lock();

	if (NULL == deviceStatus && NULL == status)
		hr = S_FALSE;
	else
	{
		if (NULL != status && 
			CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, 0, deviceStatus, -1, status, -1))
		{
			hr = S_FALSE;
		}
		else
		{
			wchar_t *string;

			string = String_Duplicate(deviceStatus);
			if (NULL == string && NULL != deviceStatus)
				hr = E_FAIL;
			else
			{
				String_Free(status);
				status = string;

				if (NULL != eventManager)
					eventManager->Notify_StatusChanged(this, status);

				hr = S_OK;
			}
		}
	}

	Unlock();

	return hr;
}

HRESULT Device::AddIcon(const wchar_t *path, unsigned int width, unsigned int height)
{
	HRESULT hr;

	if (NULL == iconStore)
		return E_UNEXPECTED;
	
	hr = iconStore->Add(path, width, height, TRUE);

	if (SUCCEEDED(hr))
	{
		if (NULL != eventManager)
			eventManager->Notify_IconChanged(this);
	}

	return hr;
}

HRESULT Device::EnumerateIcons(ifc_deviceiconstore::EnumeratorCallback callback, void *user)
{
	if (NULL == iconStore)
		return E_UNEXPECTED;
	
	return iconStore->Enumerate(callback, user);
}


HRESULT Device::RemoveIcon(unsigned int width, unsigned int height)
{
	HRESULT hr;

	if (NULL == iconStore)
		return E_UNEXPECTED;
	
	hr = iconStore->Remove(width, height);

	if (SUCCEEDED(hr))
	{
		if (NULL != eventManager)
			eventManager->Notify_IconChanged(this);
	}

	return hr;
}


HRESULT Device::SetHidden(BOOL hiddenState)
{
	HRESULT hr;

	Lock();

	if (hidden == (FALSE != hiddenState))
		hr = S_FALSE;
	else
	{
		hidden = (FALSE != hiddenState);
		hr = S_OK;
	}

	Unlock();

	if (S_OK == hr && NULL != eventManager)
		eventManager->Notify_VisibilityChanged(this, TRUE); 

	return hr;
}

HRESULT Device::IsConnected()
{
	HRESULT hr;

	Lock();
	hr = (FALSE != connected) ? S_OK : S_FALSE;
	Unlock();

	return hr;
}

HRESULT Device::Connect()
{
	HRESULT hr;

	Lock();

	if (FALSE != connected)
		hr = S_FALSE;
	else
	{
		connected = TRUE;
		hr = S_OK;
	}

	Unlock();

	return hr;
}

HRESULT Device::Disconnect()
{
	HRESULT hr;

	Lock();

	if (FALSE == connected)
		hr = S_FALSE;
	else
	{	
		connected = FALSE;
		hr = S_OK;
	}

	Unlock();

	return hr;
}

HRESULT Device::CopyTo(Device *target)
{
	if (NULL == target)
		return E_POINTER;

	Lock();

	target->SetDisplayName(displayName);
	
	if (NULL != target->iconStore)
		target->iconStore->Release();

	if (NULL == iconStore || FAILED(iconStore->Clone(&target->iconStore)))
		target->iconStore = NULL;

	target->usedSpace = usedSpace;
	target->totalSpace = totalSpace;
	target->hidden = hidden;
	target->attached = attached;
	target->connected = connected;
	
	if (NULL != target->commands)
		target->commands->Release();

	if (NULL == commands || FAILED(commands->Clone(&target->commands, TRUE)))
		target->commands = NULL;

	Unlock();

	return S_OK;
}

HRESULT Device::SetIconBase(const wchar_t *path)
{
	if (NULL == iconStore)
		return E_UNEXPECTED;

	return iconStore->SetBasePath(path);
}

HRESULT Device::AddCommand(const char *command, DeviceCommandFlags flags)
{
	if (NULL == commands)
		return E_UNEXPECTED;

	return commands->Add(command, flags);
}

HRESULT Device::RemoveCommand(const char *command)
{
	if (NULL == commands)
		return E_UNEXPECTED;

	return commands->Remove(command);

}

HRESULT Device::SetCommandFlags(const char *command, DeviceCommandFlags mask, DeviceCommandFlags flags)
{
	if (NULL == commands)
		return E_UNEXPECTED;

	return commands->SetFlags(command, mask, flags);
}

void Device::ActivityStartedCb(DeviceActivity *activity)
{
	Device *device;
	
	if(FAILED(activity->GetUser((void**)&device)) || NULL == device)
		return;
	
	if (NULL != device->eventManager)
		device->eventManager->Notify_ActivityStarted(device, activity);
}

void Device::ActivityFinishedCb(DeviceActivity *activity)
{
	Device *device;
		
	if(FAILED(activity->GetUser((void**)&device)) || NULL == device)
		return;

	device->Lock();
	
	if (activity == device->activity)
		device->activity = NULL;
	

	device->Unlock();

	if (NULL != device->eventManager)
		device->eventManager->Notify_ActivityFinished(device, activity);

	activity->Release();
}

void Device::ActivityProgressCb(DeviceActivity *activity, unsigned int progress, unsigned int duration)
{
	Device *device;
	uint64_t space;
	
	if(FAILED(activity->GetUser((void**)&device)) || NULL == device)
		return;
	
	device->Lock();
	
	space = device->usedSpace;
	space++;
	if (space > device->totalSpace)
		space = 0;

	device->Unlock();

	device->SetUsedSpace(space);
	
	if (NULL != device->eventManager)
		device->eventManager->Notify_ActivityChanged(device, activity);

}

HRESULT Device::StartSyncActivity(HWND hostWindow)
{
	HRESULT hr;

	Lock();
	
	if (NULL != activity)
		hr = E_PENDING;
	else
	{
		hr = DeviceActivity::CreateInstance(DeviceActivityFlag_Cancelable | DeviceActivityFlag_SupportProgress, 
								ActivityStartedCb, ActivityFinishedCb, ActivityProgressCb, 
								this, &activity);
		
		if (SUCCEEDED(hr))
		{
			activity->SetDisplayName(L"Synchronizing...");

			activity->SetStatus(L"Performing synchronization...");
			hr = activity->Start(60000, 20);
			if (FAILED(hr))
			{
				activity->Release();
				activity = NULL;
			}
		}
	}
	Unlock();
	return S_OK;
}

#define CBCLASS Device
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETNAME, GetName)
CB(API_GETTYPE, GetType)
CB(API_GETCONNECTION, GetConnection)
CB(API_GETICON, GetIcon)
CB(API_GETDISPLAYNAME, GetDisplayName)
CB(API_GETHIDDEN, GetHidden)
CB(API_GETTOTALSPACE, GetTotalSpace)
CB(API_GETUSEDSPACE, GetUsedSpace)
CB(API_GETATTACHED, GetAttached)
CB(API_ATTACH, Attach)
CB(API_DETACH, Detach)
CB(API_ENUMERATECOMMANDS, EnumerateCommands)
CB(API_SENDCOMMAND, SendCommand)
CB(API_GETCOMMANDFLAGS, GetCommandFlags)
CB(API_GETACTIVITY, GetActivity)
CB(API_ADVISE, Advise)
CB(API_UNADVISE, Unadvise)
CB(API_CREATEVIEW, CreateView)
VCB(API_SETNAVIGATIONITEM, SetNavigationItem)
CB(API_SETDISPLAYNAME, SetDisplayName)
CB(API_GETMODEL, GetModel)
CB(API_GETSTATUS, GetStatus)
END_DISPATCH;
#undef CBCLASS