//#define PLUGIN_NAME "Nullsoft PlaysForSure Plug-in"
#define PLUGIN_VERSION L"0.99.1"

//#define _WIN32_WINNT 0x0400
#define _WIN32_DCOM
#define INITGUID 
#include "P4SDevice.h"
#include "..\..\General\gen_ml/itemlist.h"
#include "key-sub-523.c"    // This key is the authentication key.
#include "mswmdm.h"
#include "deviceprovider.h"
#include "../nu/threadpool/api_threadpool.h"
//#include "key.c"

C_ItemList devices;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
api_memmgr *memoryManager=0;
api_albumart *AGAVE_API_ALBUMART=0;
api_threadpool *WASABI_API_THREADPOOL = 0;
api_devicemanager *AGAVE_API_DEVICEMANAGER = 0;

int init();
void quit();
intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3);

PMPDevicePlugin plugin = {PMPHDR_VER,0,init,quit,MessageProc};

void checkForDevices(BOOL *killSwitch);
void gotDevice(IWMDMDevice* pIDevice);

CRITICAL_SECTION csTransfers;

static DeviceProvider *deviceProvider = NULL;

static IWMDeviceManager3* pIdvMgr=NULL;
static DWORD ConnectionNotificationCookie = 0;
CSecureChannelClient SAC;
IWMDRMDeviceApp * DRMDeviceApp = NULL;

HANDLE killEvent = 0;

static int ThreadInitFunc(HANDLE handle, void *user_data, intptr_t param);
static int ThreadQuitFunc(HANDLE handle, void *user_data, intptr_t param);

HANDLE hWinampThread=NULL;

class MyNotification : public IWMDMNotification {
public:
  virtual HRESULT STDMETHODCALLTYPE WMDMMessage(DWORD dwMessageType, LPCWSTR  pwszCanonicalName) {
    switch(dwMessageType) {
    case WMDM_MSG_DEVICE_ARRIVAL: // WMDM device has been plugged in
      //OutputDebugString(L"Device arrived (IWMN)");
		if (NULL == deviceProvider ||
			FAILED(deviceProvider->BeginDiscovery(AGAVE_API_DEVICEMANAGER)))
		{
			checkForDevices(NULL);
		}
		break;
    case WMDM_MSG_DEVICE_REMOVAL: // WMDM device has been removed
      //OutputDebugString(L"Device removed (IWMN)");
      for(int i=0; i<devices.GetSize(); i++) {
        wchar_t devName[256] = {0};
        ((P4SDevice *)devices.Get(i))->WMDevice->GetCanonicalName(devName,256);
        if(wcscmp(pwszCanonicalName,devName) == 0) {
          ((P4SDevice *)devices.Get(i))->Close();
        }
      }
      break;
		case WMDM_MSG_MEDIA_ARRIVAL: // Media has been inserted in WMDM device 
			WMDMMessage(WMDM_MSG_DEVICE_REMOVAL,pwszCanonicalName);
			WMDMMessage(WMDM_MSG_DEVICE_ARRIVAL,pwszCanonicalName);
			break;
		case WMDM_MSG_MEDIA_REMOVAL: // Media is removed from WMDM device
			WMDMMessage(WMDM_MSG_DEVICE_REMOVAL,pwszCanonicalName);
			WMDMMessage(WMDM_MSG_DEVICE_ARRIVAL,pwszCanonicalName);
			break;
    }
    return S_OK;
  }
  // COM shit
  ULONG refs;
  MyNotification() {refs=1;}
  #define IMPLEMENTS(ifc) if (riid == IID_ ## ifc) { ++refs; *ppvObject = static_cast<ifc *>(this); return S_OK; }
  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject) {
    IMPLEMENTS(IWMDMNotification);
    IMPLEMENTS(IUnknown);
    *ppvObject = NULL; 
	  return E_NOINTERFACE;
  }
  virtual ULONG STDMETHODCALLTYPE AddRef() { return ++refs; }
  virtual ULONG STDMETHODCALLTYPE Release() { int x = --refs; if(!x) delete this; return x; }
  #undef IMPLEMENTS
};

IWMDMNotification * myNotify=NULL;


template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (plugin.service)
	{
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (plugin.service && api_t)
	{
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

BOOL 
QueueThreadFunction(api_threadpool::ThreadPoolFunc func, void *user, intptr_t id)
{
	BOOL result;
	ThreadID *thread;

	if (NULL == WASABI_API_THREADPOOL)
		return FALSE;

	thread = WASABI_API_THREADPOOL->ReserveThread(api_threadpool::FLAG_REQUIRE_COM_MT);
	if (NULL == thread)
		return FALSE;

	result = (0 == WASABI_API_THREADPOOL->RunFunction(thread, func, user, id, 
											api_threadpool::FLAG_REQUIRE_COM_MT));

	WASABI_API_THREADPOOL->ReleaseThread(thread);

	return result;
}

static int init2(BOOL runCheck) 
{
	HRESULT hr;
	IComponentAuthenticate* pICompAuth;
  
	DWORD dwNumProtCount;
	DWORD* pdwProt=NULL;  // This will always be SAC_PROTOCOL_V1.

	hr = CoCreateInstance(CLSID_MediaDevMgr, NULL, CLSCTX_ALL, IID_IComponentAuthenticate, (void **)&pICompAuth);
  
	// After getting IComponentAuthenticate, the authentication progression follows.
	if (SUCCEEDED(hr))
	{
    	hr = SAC.SetCertificate(SAC_CERT_V1, (BYTE*) abCert, sizeof(abCert), (BYTE*) abPVK, sizeof(abPVK));
		if (SUCCEEDED(hr))
		{
			// Set interface for Secure Authenticated Channel
			// operations. The return value of this function is void.

			SAC.SetInterface(pICompAuth);
			hr = pICompAuth->SACGetProtocols(&pdwProt, &dwNumProtCount);
			if (SUCCEEDED(hr))
				SAC.Authenticate(*pdwProt); //SAC_PROTOCOL_V1
			//if (SUCCEEDED(hr)) OutputDebugString(L"CSecureChannelClient.Authenticate succeeded\n");
		}
		if(pdwProt) 
			CoTaskMemFree(pdwProt);
		// After authentication has succeeded, call QueryInterface to 
		// get IID_IWMDeviceManager.
    
		 if (FAILED(pICompAuth->QueryInterface(IID_IWMDeviceManager3, (void**)&pIdvMgr)))
			pIdvMgr = NULL;

		pICompAuth->Release();
		pICompAuth = NULL;

		if (NULL != pIdvMgr)
		{
			pIdvMgr->SetDeviceEnumPreference(ALLOW_OUTOFBAND_NOTIFICATION);
			IConnectionPointContainer *pIcpc = NULL;
			if (SUCCEEDED(pIdvMgr->QueryInterface(IID_IConnectionPointContainer,(void**)&pIcpc)))
			{
				IConnectionPoint * pICP=NULL;
				if (SUCCEEDED(pIcpc->FindConnectionPoint(IID_IWMDMNotification,&pICP)))
				{
					myNotify = new MyNotification;
					
					if (FAILED(pICP->Advise(myNotify, &ConnectionNotificationCookie)))
						ConnectionNotificationCookie = 0;

					pICP->Release();
				}
				pIcpc->Release(); 
				pIcpc=0;
			}
		}
	}

	if (FALSE != runCheck)
		checkForDevices(NULL);

	return 0;
}

static void quit2()
{
	if(pIdvMgr) 
	{
		if (0 != ConnectionNotificationCookie)
		{
			IConnectionPointContainer *pIcpc;
			if (SUCCEEDED(pIdvMgr->QueryInterface(IID_IConnectionPointContainer,(void**)&pIcpc)))
			{
				IConnectionPoint * pICP;
				if (SUCCEEDED(pIcpc->FindConnectionPoint(IID_IWMDMNotification,&pICP)))
				{
					pICP->Unadvise(ConnectionNotificationCookie);
					ConnectionNotificationCookie = 0;
					pICP->Release();
				}
				pIcpc->Release();
			}
		}
		pIdvMgr->Release();
		pIdvMgr = NULL;
    }
  
	if(myNotify) 
	{
		myNotify->Release(); 
		myNotify=0;
	}

	if(DRMDeviceApp) 
	{
		DRMDeviceApp->Release(); 
		DRMDeviceApp=0;
	}
}


int init()
{
	CoInitialize(0);
	// check OS version
	OSVERSIONINFO osvi = {0};
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if(osvi.dwMajorVersion < 5) return -1;
	if(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion < 1) return -1;
	hWinampThread = GetCurrentThread();
	InitializeCriticalSection(&csTransfers);
  
	Tataki::Init(plugin.service);

	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceBuild(WASABI_API_THREADPOOL, ThreadPoolGUID);
	ServiceBuild(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceBuild(AGAVE_API_DEVICEMANAGER, DeviceManagerGUID);

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,PmpP4SLangGUID);

	static wchar_t szDescription[256];
	StringCchPrintfW(szDescription, ARRAYSIZE(szDescription),
					 WASABI_API_LNGSTRINGW(IDS_NULLSOFT_P4S_PLUGIN), PLUGIN_VERSION);
	plugin.description = szDescription;

	if (NULL != AGAVE_API_DEVICEMANAGER && 
		NULL == deviceProvider)
	{
		if (SUCCEEDED(DeviceProvider::CreateInstance(&deviceProvider)) &&
			FAILED(deviceProvider->Register(AGAVE_API_DEVICEMANAGER)))
		{
			deviceProvider->Release();
			deviceProvider = NULL;
		}
	}

	// set up thread
	killEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	if (FALSE == QueueThreadFunction(ThreadInitFunc, NULL, (NULL == deviceProvider)))
		init2(NULL == deviceProvider);

	if (NULL != deviceProvider &&
		FAILED(deviceProvider->BeginDiscovery(AGAVE_API_DEVICEMANAGER)))
	{
		checkForDevices(NULL);
	}
	return 0;
}

void quit() 
{
	SetEvent(killEvent);

	if (NULL != deviceProvider)
	{
		deviceProvider->Unregister();
		deviceProvider->Release();
		deviceProvider = NULL;
	}

	HANDLE doneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (FALSE != QueueThreadFunction(ThreadQuitFunc, doneEvent, 0))
		WaitForSingleObject(doneEvent, INFINITE);
	else
		quit2();

	CloseHandle(doneEvent);

	CloseHandle(killEvent);
	DeleteCriticalSection(&csTransfers);
	Tataki::Quit();

	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceRelease(WASABI_API_THREADPOOL, ThreadPoolGUID);
	ServiceRelease(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceRelease(AGAVE_API_DEVICEMANAGER, DeviceManagerGUID);

	CoUninitialize();
}



void newDeviceAvaliable(IWMDMDevice3* pIDevice) 
{
	HRESULT hr;
	DWORD dwTypeDev = 0;
	wchar_t buffer[1024] = {0};
	
	hr = pIDevice->GetType(&dwTypeDev);
	if (SUCCEEDED(hr))
	{
		if(0 == (WMDM_DEVICE_TYPE_PLAYBACK & dwTypeDev)) 
		{
			return;

			if (FAILED(pIDevice->GetName(buffer,ARRAYSIZE(buffer))))
				buffer[0] = L'\0';

			int l = wcslen(buffer);
			
			if(l > 5 && 
				buffer[l-5] == L' ' && 
				buffer[l-4] == L'(' && 
				buffer[l-2] == L':' && 
				buffer[l-1] == L')') 
			{
				return;
			}
		}

		if(0 == (WMDM_DEVICE_TYPE_STORAGE & dwTypeDev)) 
			return;
	}	 
	else 
		return;
 
	// make sure it's not just a simple mass storage device (we have a separate plugin for this!)
	PROPVARIANT protocol;  // VARIANTs can kiss my ass
	hr = pIDevice->GetProperty(g_wszWMDMDeviceProtocol,  &protocol);
	if (SUCCEEDED(hr))
	{
		bool isUsb = !!(*protocol.puuid == WMDM_DEVICE_PROTOCOL_MSC);
		PropVariantClear(&protocol);
		if (isUsb)
			return;
	}

	if (SUCCEEDED(pIDevice->GetCanonicalName(buffer, ARRAYSIZE(buffer))))
	{
		wchar_t buffer2[ARRAYSIZE(buffer)] = {0};

		for(int i=0; i < devices.GetSize(); i++) 
		{
			P4SDevice *device = (P4SDevice*)devices.Get(i);
			if (NULL != device && 
				NULL != device->WMDevice && 
				SUCCEEDED(device->WMDevice->GetCanonicalName(buffer2, ARRAYSIZE(buffer2))))
			{
				if(wcscmp(buffer, buffer2) == 0) 
					return;
			}
		}
	}
  
	if(!DRMDeviceApp)
		CoCreateInstance(CLSID_WMDRMDeviceApp, NULL, CLSCTX_ALL, IID_IWMDRMDeviceApp, (void **)&DRMDeviceApp);
  
	bool noMetadata;

	if (0 == (WMDM_DEVICE_TYPE_PLAYBACK & dwTypeDev))
	{
		noMetadata = true;
	}
	else if (SUCCEEDED(pIDevice->GetManufacturer(buffer, ARRAYSIZE(buffer))) &&
			 NULL != wcsstr(buffer, L"Nokia"))
	{
		noMetadata = true;
	}
	else
		noMetadata = false;

	P4SDevice *dev = new P4SDevice(pIDevice,noMetadata);

	//return dev;
}

void checkForDevices(BOOL *killSwitch) 
{
	HRESULT hr = S_OK;
	IWMDMEnumDevice* pIEnumDev;
	IWMDMDevice* pIDeviceOld;
	IWMDMDevice3* pIDevice = NULL;
	unsigned long fetched;
	if (!pIdvMgr)
		return;

	hr = pIdvMgr->EnumDevices2(&pIEnumDev); // Query for device enumerator.
	if (SUCCEEDED(hr))
	{
		// If no device is found, S_FALSE is returned, which is still
		// a success case.  Do not use the SUCCEEDED macro.
		while ((NULL == killSwitch || FALSE == *killSwitch) &&
				S_OK == pIEnumDev->Next(1, &pIDeviceOld, &fetched))
		{
			if(SUCCEEDED(pIDeviceOld->QueryInterface(IID_IWMDMDevice3,(void**)&pIDevice)))
			{
				newDeviceAvaliable(pIDevice);
				pIDevice->Release();
			}
			pIDeviceOld->Release();
		}
		// We should release this pointer but it appears to make winamp not shut down properly if we do.
		// This function will only be called a small number of times, so I propose that we don't free it
		// until this error can be better investigated. --will
		pIEnumDev->Release();
	}
}


static int ThreadInitFunc(HANDLE handle, void *user_data, intptr_t param)
{
	init2((BOOL)param);
	return 0;
}

static int ThreadQuitFunc(HANDLE handle, void *user_data, intptr_t param)
{
	quit2();
	
	if (NULL != user_data)
		SetEvent((HANDLE)user_data);

	return 0;
}


intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3) {
	switch(msg) {
		case PMP_DEVICECHANGE:
			return 0;
		case PMP_NO_CONFIG:
			return TRUE;
		case PMP_CONFIG:
			return 0;
	}
	return 0;
}

typedef struct { ULONG_PTR param; void * proc; int state; CRITICAL_SECTION lock;} spc ;

static VOID CALLBACK spc_caller(ULONG_PTR dwParam) {
  spc * s = (spc*)dwParam;
  if(!s) return;
  EnterCriticalSection(&s->lock);
  if(s->state == -1) { LeaveCriticalSection(&s->lock); DeleteCriticalSection(&s->lock); free(s); return; }
  s->state = 2;
  void (CALLBACK *p)(ULONG_PTR dwParam);
  p = (void (CALLBACK *)(ULONG_PTR dwParam))s->proc;
  if(p) p(s->param);
  s->state=1;
  LeaveCriticalSection(&s->lock); 
}

// p must be of type void (CALLBACK *)(ULONG_PTR dwParam). Returns 0 for success.
int SynchronousProcedureCall(void * p, ULONG_PTR dwParam) {
  if(!p) return 1;
  spc * s = (spc*)calloc(1, sizeof(spc));
  InitializeCriticalSection(&s->lock);
  s->param = dwParam;
  s->proc = p;
  s->state = 0;
  if(!QueueUserAPC(spc_caller,hWinampThread,(ULONG_PTR)s)) { DeleteCriticalSection(&s->lock); free(s); return 1; } //failed
  int i=0, state;
  do {
    SleepEx(10,true);
    EnterCriticalSection(&s->lock);
    state = s->state;
    if(i++ == 100) {s->state = -1; return 1;}
    LeaveCriticalSection(&s->lock);
  }
  while(state!=1);
  DeleteCriticalSection(&s->lock);
  free(s);
  return 0;
}


BOOL FormatResProtocol(const wchar_t *resourceName, const wchar_t *resourceType, wchar_t *buffer, size_t bufferMax)
{
	unsigned long filenameLength;

	if (NULL == resourceName)
		return FALSE;

	if (FAILED(StringCchCopyExW(buffer, bufferMax, L"res://", &buffer, &bufferMax, 0)))
		return FALSE;

	filenameLength = GetModuleFileNameW(plugin.hDllInstance, buffer, bufferMax);
	if (0 == filenameLength || bufferMax == filenameLength)
		return FALSE;

	buffer += filenameLength;
	bufferMax -= filenameLength;

	if (NULL != resourceType)
	{
		if (FALSE != IS_INTRESOURCE(resourceType))
		{
			if (FAILED(StringCchPrintfExW(buffer, bufferMax, &buffer, &bufferMax, 0, L"/#%d", (int)(INT_PTR)resourceType)))
				return FALSE;
		}
		else
		{
			if (FAILED(StringCchPrintfExW(buffer, bufferMax, &buffer, &bufferMax, 0, L"/%s", resourceType)))
				return FALSE;
		}
	}

	if (FALSE != IS_INTRESOURCE(resourceName))
	{
		if (FAILED(StringCchPrintfExW(buffer, bufferMax, &buffer, &bufferMax, 0, L"/#%d", (int)(INT_PTR)resourceName)))
			return FALSE;
	}
	else
	{
		if (FAILED(StringCchPrintfExW(buffer, bufferMax, &buffer, &bufferMax, 0, L"/%s", resourceName)))
			return FALSE;
	}

	return TRUE;
}

extern "C" {
	__declspec( dllexport ) PMPDevicePlugin * winampGetPMPDevicePlugin(){return &plugin;}
	__declspec( dllexport ) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
		int i = devices.GetSize();
		while(i-- > 0) ((Device*)devices.Get(i))->Close();
		return PMP_PLUGIN_UNINSTALL_NOW;
	}
};