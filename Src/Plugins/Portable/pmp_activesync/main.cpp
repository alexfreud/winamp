//#define PLUGIN_NAME "Nullsoft ActiveSync Plug-in"
#define PLUGIN_VERSION L"0.25"

#include "ASDevice.h"

int init();
void quit();
intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3);

extern PMPDevicePlugin plugin = {PMPHDR_VER,0,init,quit,MessageProc};

static HANDLE killEvent=0, hThread=0;
static DWORD WINAPI ThreadFunc(LPVOID lpParam);

IRAPIDesktop *pIRapiDesktop = NULL;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

std::vector<ASDevice*> devices;

std::vector<ejectedDevice*> ejected;
static RAPIDEVICEID lastDevId;


class MyRAPISink : public IRAPISink {
public:
	virtual HRESULT STDMETHODCALLTYPE OnDeviceConnected(IRAPIDevice *pIDevice) {
		RAPI_DEVICEINFO devInfo;
		if(!SUCCEEDED(pIDevice->GetDeviceInfo(&devInfo))) return S_OK;
		SysFreeString(devInfo.bstrName);
		SysFreeString(devInfo.bstrPlatform);
		
		EnterCriticalSection(&cs);

		lastDevId = devInfo.DeviceId;
		for(unsigned int i=0; i<ejected.size(); i++) {
			if(devInfo.DeviceId == ejected[i]->id) {
				ejected[i]->marked=true;
				LeaveCriticalSection(&cs);
				return S_OK;
			}
		}

		for(unsigned int i=0; i<devices.size(); i++) {
			if(devInfo.DeviceId == devices[i]->devInfo.DeviceId || devices[i]->pIDevice == pIDevice) {
				LeaveCriticalSection(&cs);
				return S_OK;
			}
		}

		IRAPISession *pISession = NULL;
		if (SUCCEEDED(pIDevice->CreateSession(&pISession))) {
			if (SUCCEEDED(pISession->CeRapiInit())) devices.push_back(new ASDevice(pIDevice,pISession));
			else pISession->Release();
		}

		LeaveCriticalSection(&cs);
		return S_OK;
	}
	
	virtual HRESULT STDMETHODCALLTYPE OnDeviceDisconnected(IRAPIDevice *pIDevice) {
		EnterCriticalSection(&cs);
		RAPI_DEVICEINFO devInfo;
		if(!SUCCEEDED(pIDevice->GetDeviceInfo(&devInfo))) return S_OK;
		for(unsigned int i=0; i<devices.size(); i++) {
			if(devInfo.DeviceId == devices[i]->devInfo.DeviceId || devices[i]->pIDevice == pIDevice) {
				SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)devices[i],PMP_IPC_DEVICEDISCONNECTED);
				delete devices[i];
			}
		}
		SysFreeString(devInfo.bstrName);
		SysFreeString(devInfo.bstrPlatform);
		LeaveCriticalSection(&cs);
		return S_OK;
	}
	
	DWORD RAPISinkContext;
	CRITICAL_SECTION cs;
	ULONG refs;
  MyRAPISink() {refs=1; InitializeCriticalSection(&cs);}
	~MyRAPISink() { DeleteCriticalSection(&cs); }
  #define IMPLEMENTS(ifc) if (riid == IID_ ## ifc) { ++refs; *ppvObject = static_cast<ifc *>(this); return S_OK; }
  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject) {
    IMPLEMENTS(IRAPISink);
    IMPLEMENTS(IUnknown);
    *ppvObject = NULL; 
	  return E_NOINTERFACE;
  }
  virtual ULONG STDMETHODCALLTYPE AddRef() { return ++refs; }
  virtual ULONG STDMETHODCALLTYPE Release() { int x = --refs; if(!x) delete this; return x; }
  #undef IMPLEMENTS
};

MyRAPISink *pMyRapiSink=NULL;

int init() {
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	HRESULT hr = CoCreateInstance(CLSID_RAPI,NULL,CLSCTX_INPROC_SERVER,IID_IRAPIDesktop,(void**)&pIRapiDesktop);
	if(!SUCCEEDED(hr) || !pIRapiDesktop) return -1; // no activesync on this computer!

	// loader so that we can get the localisation service api for use
	waServiceFactory *sf = plugin.service->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,PmpACTIVESYNCLangGUID);

	static wchar_t szDescription[256];
	swprintf(szDescription, ARRAYSIZE(szDescription),
			 WASABI_API_LNGSTRINGW(IDS_NULLSOFT_ACTIVESYNC_PLUGIN), PLUGIN_VERSION);
	plugin.description = szDescription;

	killEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	DWORD dwThreadId; 
	hThread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, &dwThreadId);

	return 0;
}

static void enumDevices() {
	// find all the currently connected devices
	IRAPIEnumDevices* pIRapiEnumDevices = NULL;
	HRESULT hr = pIRapiDesktop->EnumDevices(&pIRapiEnumDevices);

	for (unsigned int i = 0; i < ejected.size(); i++) ejected[i]->marked = false;

	while (SUCCEEDED(hr) && pIRapiEnumDevices) {
		IRAPIDevice* pIRapiDevice = NULL;
		hr = pIRapiEnumDevices->Next(&pIRapiDevice);
		if (SUCCEEDED(hr) && pIRapiDevice) {
			pMyRapiSink->OnDeviceConnected(pIRapiDevice);
			pIRapiDevice->Release();
		}
		else {
			pIRapiEnumDevices->Release();
			pIRapiEnumDevices = NULL;
		}
	}

	//for (unsigned int i = 0; i < ejected.size(); i++)
	//{
	//	if (!ejected[i]->marked)
	//	{
	//		free(ejected[i]);
	//		ejected.eraseindex(i);
	//	}
	//}
	auto it = ejected.begin();
	while (it != ejected.end())
	{
		ejectedDevice* dev = *it;
		if (!dev->marked)
		{
			free(dev);
			it = ejected.erase(it);
		}
		else
		{
			it++;
		}
	}
}

static void init2() {
	// set up device connection/disconnection notifications
	pMyRapiSink = new MyRAPISink();
	pIRapiDesktop->Advise(pMyRapiSink,(DWORD_PTR*)&pMyRapiSink->RAPISinkContext);

	// find currently attached devices
	enumDevices();
}

void quit() {
	SetEvent(killEvent);
	if (hThread) {
		for(;;) {
			int val = WaitForSingleObjectEx(hThread,15000,TRUE);
			if(val == WAIT_OBJECT_0) { CloseHandle(hThread); break; }
			else if(val == WAIT_TIMEOUT) { TerminateThread(hThread, 0); break; }
			else continue;
		}
	}
	CloseHandle(killEvent);

	pIRapiDesktop->UnAdvise(pMyRapiSink->RAPISinkContext);
	pIRapiDesktop->Release();
	pMyRapiSink->Release();
  CoUninitialize();
	for(unsigned int i=0; i<ejected.size(); i++) free(ejected[i]);
}

static DWORD WINAPI ThreadFunc(LPVOID lpParam) {
	CoInitializeEx(0,COINIT_MULTITHREADED);
	init2();
	while (WaitForSingleObjectEx(killEvent,5000,TRUE) != WAIT_OBJECT_0) {
		// FUCKO: For some reason I'm not getting the device connected notifications, so lets just enum for devices on a regular basis.
		enumDevices();
	}
	CoUninitialize();
  return 0;
}

intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3) {
	if (msg == PMP_NO_CONFIG)
		return TRUE;

	return FALSE;
}

extern "C" {
	__declspec( dllexport ) PMPDevicePlugin * winampGetPMPDevicePlugin(){return &plugin;}
	__declspec( dllexport ) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
		int i = (int)devices.size();
		while(i-- > 0) devices[i]->Close();
		return PMP_PLUGIN_UNINSTALL_NOW;
	}
};