//#define PLUGIN_NAME "Nullsoft iPod Plug-in"
#define PLUGIN_VERSION L"0.91"

#include "iPodDevice.h"
#include <Dbt.h>
#include "..\..\General\gen_ml/itemlist.h"
#include <api/service/waservicefactory.h>
#include "api.h"
#include "../nu/autoLock.h"
#include "deviceprovider.h"
#include <tataki/export.h>
#include <shlwapi.h>

int init();
void quit();
intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3);

extern PMPDevicePlugin plugin = {PMPHDR_VER,0,init,quit,MessageProc};

static DWORD WINAPI ThreadFunc_DeviceChange(LPVOID lpParam);

bool g_detectAll=false;

std::vector<iPodDevice*> iPods;

api_config *AGAVE_API_CONFIG=0;
api_memmgr *WASABI_API_MEMMGR=0;
api_albumart *AGAVE_API_ALBUMART=0;
api_threadpool *WASABI_API_THREADPOOL=0;
api_application *WASABI_API_APP=0;
api_devicemanager *AGAVE_API_DEVICEMANAGER = 0;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;

HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

static Nullsoft::Utility::LockGuard connect_guard;
static DeviceProvider *deviceProvider = NULL;
static bool loading_devices[26] = {0,};

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

static bool createBlankDatabase(wchar_t drive) {
	wchar_t ipodtest[]={drive,L":\\iPod_Control\\iTunes\\iTunesDB"};
	iPod_mhbd db;
	BYTE data[4096] = {0};
	long l = db.write(data,sizeof(data));
	if(l <= 0) return false;
	FILE* fh=_wfopen(ipodtest,L"wb");
	if(!fh) return false;
	if(fwrite(data,l,1,fh))
	{
		wchar_t ipodtest2[]={drive,L":\\iPod_Control\\iTunes\\firsttime"};
		_wunlink(ipodtest2);
	}
	fclose(fh);
	wchar_t music[] = {drive,L":\\iPod_Control\\Music"};
	CreateDirectoryW(music,NULL);
	return true;
}


bool ConnectDrive(wchar_t drive, bool connect) 
{
	bool result;
	size_t index;
	iPodDevice *device;

	Nullsoft::Utility::AutoLock connect_lock(connect_guard);

  	// capitalize
	if (drive >= L'a' && drive <= L'z')
		drive = drive - 32;
	
	// reject invalid drive letters
	if (drive < L'A' || drive > L'Z')
		return false;

	if (false != loading_devices[drive-'A'])
		return false;

	
	loading_devices[drive-'A'] = true;

	if (false != connect)
	{
		result = true;

		char ipodtest[]= {(char)drive,":\\iPod_Control\\iTunes\\iTunesDB"};
		if (GetFileAttributes(ipodtest) == INVALID_FILE_ATTRIBUTES)
		{
			char ipodtest2[]={(char)drive,":\\iPod_Control\\iTunes\\firsttime"};
			if (GetFileAttributes(ipodtest2) == INVALID_FILE_ATTRIBUTES ||
				false == createBlankDatabase(drive))
			{
				result =  false;
			}
		}
		
		if (false != result)
		{
			index = iPods.size();
			while(index--)
			{
				device = iPods[index];
				if(device->drive == drive) 
					break;
			}

			if((size_t)-1 == index) 
			{
				iPodDevice *d = new iPodDevice((char)drive);
			}
			else 
				result = false;
		}
	}
	else
	{
		result = false;

		index = iPods.size();
		while(index--)
		{
			device = iPods.at(index);
			if (device->drive == drive)
			{
				SendNotifyMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)device,PMP_IPC_DEVICEDISCONNECTED);
				iPods.erase(iPods.begin() + index);
				delete device;
				result = true;
				break;
			}
		} 
	}

	loading_devices[drive-'A'] = false;

	return result;
}


static void autoDetectCallback(wchar_t driveW, UINT type) 
{
	if(false == g_detectAll && DRIVE_REMOVABLE != type) 
		return;

	ConnectDrive(driveW, true);
}

int init()
{
	wchar_t mlipod[MAX_PATH] = {0};
	PathCombineW(mlipod, (LPCWSTR)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETPLUGINDIRECTORYW), L"ml_ipod.dll");
	FILE * f = _wfopen(mlipod, L"rb");
	if (f) {
		fclose(f);
		return -1;
	}

	Tataki::Init(plugin.service);

	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceBuild(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceBuild(WASABI_API_THREADPOOL, ThreadPoolGUID);
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(AGAVE_API_DEVICEMANAGER, DeviceManagerGUID);

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,PmpIPODLangGUID);

	static wchar_t szDescription[256];
	swprintf(szDescription, ARRAYSIZE(szDescription),
			 WASABI_API_LNGSTRINGW(IDS_NULLSOFT_IPOD_PLUGIN), PLUGIN_VERSION);
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

	/* Our device shows up as a normal drive */
	if (NULL == deviceProvider || 
		FAILED(deviceProvider->BeginDiscovery(AGAVE_API_DEVICEMANAGER)))
	{
		SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)autoDetectCallback,PMP_IPC_ENUM_ACTIVE_DRIVES);
	}

	return 0;
}

void quit()
{
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceRelease(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceRelease(WASABI_API_THREADPOOL, ThreadPoolGUID);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(AGAVE_API_DEVICEMANAGER, DeviceManagerGUID);

	Tataki::Quit();
}

static char FirstDriveFromMask(ULONG unitmask) {
  char i;
  for(i=0; i<26; ++i) {
    if(unitmask & 0x1) break;
    unitmask = unitmask >> 1;
  }
  return (i+'A');
}


int wmDeviceChange(WPARAM wParam, LPARAM lParam) 
{  
	if(wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE)
	{ 
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
		if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME) 
		{ // its a volume
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
			if(0 == ((DBTF_MEDIA | DBTF_NET) & lpdbv->dbcv_flags) || g_detectAll) 
			{ // its not a network drive or a CD/floppy, game on!
				unsigned long unitmask = lpdbv->dbcv_unitmask;
				for (int i = 0; i < 26; i++)
				{
					if (0 != (0x1 & unitmask))
					{
						 int p = (int)('A' + i);
						 if (DBT_DEVICEARRIVAL == wParam)
							 p += 0x10000;

						 ThreadFunc_DeviceChange((void*)(intptr_t)p);

						 unitmask = unitmask >> 1;
						 if (0 == unitmask)
							 break;
					}
					else
						unitmask = unitmask >> 1;
				}
			}
		}
	}
	return 0;
}


static DWORD WINAPI ThreadFunc_DeviceChange(LPVOID lpParam) 
{
	int p = (int)lpParam;
	bool connect = p > 0x10000;
	
	if(connect) 
		p -= 0x10000;
	
	char drive = (char)p;
	if(drive == 0) 
		return 0;
 
	if(connect) 
	{ // something plugged in
		ConnectDrive(drive, connect);
	}
	else 
	{ //something removed
		size_t index;

		index = iPods.size();
		while(index--)
		{
			iPodDevice *device = iPods.at(index);
			if (device->drive == drive)
			{
				SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)device,PMP_IPC_DEVICEDISCONNECTED);
				iPods.erase(iPods.begin() + index);
				delete device;
				break;
			}
		} 
	}
	return 0;
}


intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3) {
	switch(msg) {
		case PMP_DEVICECHANGE:
			return wmDeviceChange(param1,param2);
		case PMP_NO_CONFIG:
			return TRUE;
		case PMP_CONFIG:
			return 0;
	}
	return 0;
}

extern "C" {
	__declspec( dllexport ) PMPDevicePlugin * winampGetPMPDevicePlugin(){return &plugin;}
	__declspec( dllexport ) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
		int i = iPods.size();
		while(i-- > 0) iPods[i]->Close();
		return PMP_PLUGIN_UNINSTALL_NOW;
	}
};