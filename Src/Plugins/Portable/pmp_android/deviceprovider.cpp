#include "api.h"
#include "./deviceprovider.h"
#include "../devices/api_devicemanager.h"

extern PMPDevicePlugin plugin;
void connectDrive(wchar_t drive, bool checkSize, bool checkBlacklist);

static size_t tlsIndex = (size_t)-1;

static BOOL 
DiscoveryProvider_RegisterCancelSwitch(BOOL *cancelSwitch)
{
	if ((size_t)-1 != tlsIndex && 
		NULL != WASABI_API_APP)
	{
		WASABI_API_APP->SetThreadStorage(tlsIndex, cancelSwitch);
		return TRUE;
	}

	return FALSE;
}

static BOOL 
DiscoveryProvider_GetCancelSwitchOn()
{
	if ((size_t)-1 != tlsIndex && 
		NULL != WASABI_API_APP)
	{
		BOOL *cancelSwitch = (BOOL*)WASABI_API_APP->GetThreadStorage(tlsIndex);
		if (NULL != cancelSwitch && 
			FALSE != *cancelSwitch)
		{
			return TRUE;
		}
	}
	return FALSE;
}

static void
DeviceProvider_DriverEnumCb(wchar_t drive, unsigned int type)
{
	if (DRIVE_REMOVABLE == type &&
		FALSE == DiscoveryProvider_GetCancelSwitchOn())
	{
		connectDrive(drive,true,true);
	}
}

DeviceProvider::DeviceProvider()
	: ref(1), activity(0), manager(NULL), readyEvent(NULL), cancelDiscovery(FALSE)
{
	InitializeCriticalSection(&lock);
	enumerator = (ENUMDRIVES)SendMessageW(plugin.hwndPortablesParent, 
								WM_PMP_IPC, 0, PMP_IPC_ENUM_ACTIVE_DRIVES);
}

DeviceProvider::~DeviceProvider()
{
	CancelDiscovery();

	if (NULL != readyEvent)
		CloseHandle(readyEvent);

	DeleteCriticalSection(&lock);
}

HRESULT DeviceProvider::CreateInstance(DeviceProvider **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = new DeviceProvider();

	if (NULL == *instance) 
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t DeviceProvider::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceProvider::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceProvider::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DeviceProvider))
		*object = static_cast<ifc_deviceprovider*>(this);
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

void DeviceProvider::Lock()
{
	EnterCriticalSection(&lock);
}

void DeviceProvider::Unlock()
{
	LeaveCriticalSection(&lock);
}

DWORD DeviceProvider::DiscoveryThread()
{
	IncrementActivity();

	if (NULL != enumerator && 
		FALSE == cancelDiscovery)
	{
		DiscoveryProvider_RegisterCancelSwitch(&cancelDiscovery);

		enumerator(DeviceProvider_DriverEnumCb);

		DiscoveryProvider_RegisterCancelSwitch(NULL);
	}

	DecrementActivity();

	Lock();
	
	if (NULL != readyEvent)
		SetEvent(readyEvent);

	Unlock();

	
	return 0;
}

static int DeviceProvider_DiscoveryThreadStarter(HANDLE handle, void *user, intptr_t id)
{
	DeviceProvider *self;
	DWORD result;
		
	self = (DeviceProvider*)user;
			
	if (NULL != self)
		result = self->DiscoveryThread();
	else
		result = -2;

	return result;
}

HRESULT DeviceProvider::BeginDiscovery(api_devicemanager *manager)
{
	HRESULT hr;
	
	if (NULL == enumerator)
		return E_UNEXPECTED;

	Lock();

	if (NULL != readyEvent && 
		WAIT_TIMEOUT == WaitForSingleObject(readyEvent, 0))
	{
		hr = E_PENDING;
	}
	else
	{
		hr = S_OK;
		
		if (NULL == readyEvent)
		{
			readyEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
			if (NULL == readyEvent)
				hr = E_FAIL;
		}

		if ((size_t)-1 == tlsIndex && 
			NULL != WASABI_API_APP)
		{
			tlsIndex = WASABI_API_APP->AllocateThreadStorage();
		}

		if (SUCCEEDED(hr))
		{
			
			cancelDiscovery = FALSE;
			ResetEvent(readyEvent);
			
			if (0 != WASABI_API_THREADPOOL->RunFunction(0, DeviceProvider_DiscoveryThreadStarter, 
									this, 0, api_threadpool::FLAG_LONG_EXECUTION))
			{

				SetEvent(readyEvent);
				hr = E_FAIL;
			}
		}
	}

	Unlock();

	return hr;
}

HRESULT DeviceProvider::CancelDiscovery()
{
	HRESULT hr;

	hr = S_FALSE;

	Lock();
	
	if (NULL != readyEvent)
	{
		cancelDiscovery = TRUE;
		if (WAIT_OBJECT_0 == WaitForSingleObject(readyEvent, 0))
			hr = S_OK;

		cancelDiscovery = FALSE;
	}

	Unlock();
	
	return hr;
}

HRESULT DeviceProvider::GetActive()
{
	HRESULT hr;

	Lock();
	
	if (0 != activity)
		hr = S_OK;
	else
		hr = S_FALSE;

	Unlock();

	return hr;
}

HRESULT DeviceProvider::Register(api_devicemanager *manager)
{
	HRESULT hr;

	if (NULL != this->manager)
		return E_UNEXPECTED;

	if (NULL == manager)
		return E_POINTER;

	hr = manager->RegisterProvider(this);
	if (SUCCEEDED(hr))
	{
		this->manager = manager;
		manager->AddRef();
	}
	return hr;
}

HRESULT DeviceProvider::Unregister()
{
	HRESULT hr;

	if (NULL == manager)
		return E_UNEXPECTED;

	hr = manager->UnregisterProvider(this);
	manager->Release();
	manager = NULL;
	return hr;
}

size_t DeviceProvider::IncrementActivity()
{
	size_t a;

	Lock();

	activity++;
	if (1 == activity && 
		NULL != manager)
	{
		manager->SetProviderActive(this, TRUE);
	}
	
	a = activity;
	
	Unlock();

	return a;
}

size_t DeviceProvider::DecrementActivity()
{
	size_t a;

	Lock();

	if (0 != activity)
	{
		activity--;
		if (0 == activity && 
			NULL != manager)
		{
			manager->SetProviderActive(this, FALSE);
		}
	}

	a = activity;
	
	Unlock();

	return a;
}

#define CBCLASS DeviceProvider
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_BEGINDISCOVERY, BeginDiscovery)
CB(API_CANCELDISCOVERY, CancelDiscovery)
CB(API_GETACTIVE, GetActive)
END_DISPATCH;
#undef CBCLASS