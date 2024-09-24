#include "main.h"
#include "./provider.h"

typedef struct DeviceProviderThreadParam
{
	api_devicemanager *manager;
	DeviceProvider *provider;
	HANDLE readyEvent;
	
}DeviceProviderThreadParam;

DeviceProvider::DeviceProvider()
	: ref(1), discoveryThread(NULL), cancelEvent(NULL)
{
	wchar_t buffer[MAX_PATH * 2];
	HINSTANCE module;

	TestSuiteLoader loader;
	
	InitializeCriticalSection(&lock);
	
	module = Plugin_GetInstance();
	if (0 == GetModuleFileName(module, buffer, ARRAYSIZE(buffer)) ||
		FALSE == PathRemoveFileSpec(buffer))
	{
		buffer[0] = L'\0';
	}
	PathAppend(buffer, L"testprovider.xml");
	loader.Load(buffer, &testSuite);
}

DeviceProvider::~DeviceProvider()
{
	CancelDiscovery();
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

DWORD DeviceProvider::DiscoveryThread(api_devicemanager *manager)
{
	DWORD waitResult;
	DWORD deviceCount;
	DWORD threadId;
	DWORD sleepTime;
	LARGE_INTEGER perfCounter;

	deviceCount = 0;

	threadId = GetCurrentThreadId();
//	aTRACE_FMT("[test provider] device discovery started (0x%X).\r\n", threadId);
	
	manager->SetProviderActive(this, TRUE);

	for(;;)
	{
		if (FALSE != QueryPerformanceCounter(&perfCounter))
			srand(perfCounter.LowPart);
		else
			srand(GetTickCount());

		sleepTime = (DWORD)((double)rand()/(RAND_MAX) * 0);
		
		waitResult = WaitForSingleObject(cancelEvent, sleepTime);
		if (WAIT_OBJECT_0 == waitResult)
			break;
		else if (WAIT_TIMEOUT == waitResult)
		{
			Device *device;

			deviceCount++;
//			aTRACE_FMT("[test provider] creating new device[%d] (0x%X).\r\n", deviceCount, threadId);
			
			device = testSuite.GetRandomDevice();
			if (NULL != device)
			{
				device->Connect();
				if (0 == manager->DeviceRegister((ifc_device**)&device, 1))
					device->Disconnect();
			}

			if (4 == deviceCount)
				break;
		}
		else
		{
//			aTRACE_FMT("[test provider] error (0x%X).\r\n", threadId);
			break;
		}
	}

	EnterCriticalSection(&lock);

	if (NULL != discoveryThread)
	{
		CloseHandle(discoveryThread);
		discoveryThread = NULL;
	}

	if (NULL != cancelEvent)
	{
		CloseHandle(cancelEvent);
		cancelEvent = NULL;
	}

//	aTRACE_FMT("[test provider] device discovery finished (0x%X).\r\n", threadId);
	LeaveCriticalSection(&lock);

	manager->SetProviderActive(this, FALSE);

	return 0;
}

static DWORD CALLBACK DeviceProvider_DiscoveryThreadStarter(void *user)
{
	DeviceProviderThreadParam *param;
	DeviceProvider *provider;
	api_devicemanager *manager;
	DWORD result;
	
	param = (DeviceProviderThreadParam*)user;
	manager = param->manager;
	provider = param->provider;
	
	if (NULL != manager)
		manager->AddRef();
	
	if (NULL != param->readyEvent)
		SetEvent(param->readyEvent);

	if (NULL == manager)
		return -1;
	
	if (NULL != provider)
		result = provider->DiscoveryThread(manager);
	else
		result = -2;
	
	manager->Release();

	return result;
}

HRESULT DeviceProvider::BeginDiscovery(api_devicemanager *manager)
{
	HRESULT hr;
	
	if (NULL == manager)
		return E_INVALIDARG;

	EnterCriticalSection(&lock);

	if (NULL != discoveryThread)
		hr = E_PENDING;
	else
	{
		hr = S_OK;
		if (NULL == cancelEvent)
		{
			cancelEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (NULL == cancelEvent)
				hr = E_FAIL;
		}

		if (SUCCEEDED(hr))
		{			
			DeviceProviderThreadParam param;
			
			param.provider = this;
			param.manager = manager;
			param.readyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			if (NULL == param.readyEvent)
				hr = E_FAIL;
			else
			{
				DWORD threadId;
				
				discoveryThread = CreateThread(NULL, 0, DeviceProvider_DiscoveryThreadStarter, &param, 0, &threadId);
				if (NULL == discoveryThread)
					hr = E_FAIL;
				else
					WaitForSingleObject(param.readyEvent, INFINITE);
				
				CloseHandle(param.readyEvent);
			}
		}

		if (FAILED(hr))
			CancelDiscovery();
	}

	LeaveCriticalSection(&lock);

	return hr;
}

HRESULT DeviceProvider::CancelDiscovery()
{
	HRESULT hr;
	HANDLE threadHandle, eventHandle;
	
	EnterCriticalSection(&lock);

	threadHandle = discoveryThread;
	eventHandle = cancelEvent;

	discoveryThread = NULL;
	cancelEvent = NULL;

	LeaveCriticalSection(&lock);

	if (NULL != threadHandle)
	{
		if (NULL != eventHandle)
			SetEvent(eventHandle);

		WaitForSingleObject(threadHandle, INFINITE);
		CloseHandle(threadHandle);
		hr = S_OK;
	}
	else
		hr = S_FALSE;
	
	if (NULL != eventHandle)
		CloseHandle(eventHandle);

	return hr;
}

HRESULT DeviceProvider::GetActive()
{
	HRESULT hr;

	EnterCriticalSection(&lock);
	
	hr = (NULL != discoveryThread) ? S_OK : S_FALSE;

	LeaveCriticalSection(&lock);

	return hr;
}

HRESULT DeviceProvider::Register(api_devicemanager *manager)
{
	HRESULT hr;

	if (NULL == manager)
		return E_POINTER;

	hr = manager->RegisterProvider(this);
	if (SUCCEEDED(hr))
	{
		testSuite.RegisterCommands(manager);
		testSuite.RegisterTypes(manager);
		testSuite.RegisterConnections(manager);
		testSuite.RegisterDevices(manager);
	}

	return hr;
}

HRESULT DeviceProvider::Unregister(api_devicemanager *manager)
{
	HRESULT hr;

	if (NULL == manager)
		return E_POINTER;

	hr = manager->UnregisterProvider(this);
	testSuite.UnregisterTypes(manager);
	testSuite.UnregisterConnections(manager);
	testSuite.UnregisterCommands(manager);
	testSuite.UnregisterDevices(manager);
	
	return hr;
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