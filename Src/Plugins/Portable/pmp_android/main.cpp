#include "../../Library/ml_pmp/pmp.h"
#include "../Winamp/wa_ipc.h"
#include <vector>
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../nu/Alias.h"

#include <api/service/waServiceFactory.h>

#include "api.h"
#include "resource.h"
#include "androiddevice.h"
#include "deviceprovider.h"

#include <devguid.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>

#define PLUGIN_VERSION L"1.72"
static int Init();
static void Quit();
static bool doRegisterForDevNotification(void);
static intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3);

extern PMPDevicePlugin plugin = {PMPHDR_VER,0,Init,Quit,MessageProc};

// start-android
static const wchar_t *winampini;
static std::vector<wchar_t*> blacklist;
bool loading_devices[26] = {0,};

static HDEVNOTIFY hDevNotify;

std::vector<AndroidDevice*> devices;
HWND config;

static DeviceProvider *deviceProvider = NULL;
static UINT_PTR rescanTimer = 0;

static void blacklistLoad() {
	wchar_t keyname[64] = {0};
	int l = GetPrivateProfileIntW(L"pmp_android", L"blacklistnum", 0, winampini);
	for(int i=l>100?l-100:0; i<l; i++) {
		wchar_t buf[100] = {0};
		StringCchPrintfW(keyname, 64, L"blacklist-%d", i);
		GetPrivateProfileStringW(L"pmp_android", keyname, L"", buf, 100, winampini);
		if(buf[0]) 
		{
			blacklist.push_back(_wcsdup(buf));
		}
	}
}

static void blacklistSave() {
	wchar_t buf[64] = {0};
	StringCchPrintfW(buf, 64, L"%d", blacklist.size());
	WritePrivateProfileStringW(L"pmp_android", L"blacklistnum", buf, winampini);
	for(size_t i=0; i<blacklist.size(); i++) 
	{
		StringCchPrintfW(buf, 64, L"blacklist-%d", i);
		WritePrivateProfileStringW(L"pmp_android", buf, (const wchar_t*)blacklist.at(i), winampini);
	}
}

static wchar_t *makeBlacklistString(wchar_t drive) {
	wchar_t path[4]={drive,L":\\"};
	wchar_t name[100]=L"";
	wchar_t buf[FIELD_LENGTH]=L"";
	DWORD serial=0;

	UINT olderrmode=SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	GetVolumeInformation(path,name,100,&serial,NULL,NULL,NULL,0);
	
	if(serial) 
	{
		StringCchPrintf(buf, FIELD_LENGTH, L"s:%d",serial);
		SetErrorMode(olderrmode);
		return _wcsdup(buf);
	}

	{
		ULARGE_INTEGER tfree={0,}, total={0,}, freeb={0,};
		GetDiskFreeSpaceEx(path,  &tfree, &total, &freeb);
		StringCchPrintf(buf, FIELD_LENGTH, L"n:%s,%d,%d", name, total.HighPart, total.LowPart);
		SetErrorMode(olderrmode);
		return _wcsdup(buf);
	}
}

static bool blacklistCheck(wchar_t drive) 
{
	wchar_t *s = makeBlacklistString(drive);
	if (s)
	{
		for(size_t i=0; i<blacklist.size(); i++)
		{
			if(!wcscmp(s,(wchar_t*)blacklist.at(i))) 
			{ 
				free(s); 
				return true; 
			}
		}
		free(s);
	}
	return false;
}

static bool blacklistAdd(const wchar_t drive)
{
	wchar_t *s = makeBlacklistString(drive);
	if (s)
	{
		for(size_t i=0; i<blacklist.size(); i++)
		{
			if(!wcscmp(s,(wchar_t*)blacklist.at(i))) 
			{ 
				free(s); 
				return false; 
			}
		}
		blacklist.push_back(s);
	}
	return true;
}

static BOOL 
Device_IsiPod(const wchar_t drive)
{
	const wchar_t test[] = {drive, L":\\iPod_Control"};
	WIN32_FIND_DATAW findData;
	HANDLE file = FindFirstFileW(test, &findData);
	if (INVALID_HANDLE_VALUE != file)
	{
		FindClose(file);
		if (0 != (FILE_ATTRIBUTE_DIRECTORY & findData.dwFileAttributes))
			return TRUE;
	}
	return FALSE;
}

static BOOL 
Device_IsAndroid(const wchar_t drive)
{
	const wchar_t test[] = {drive, L":\\Android"};
	WIN32_FIND_DATAW findData;
	HANDLE file = FindFirstFileW(test, &findData);
	if (INVALID_HANDLE_VALUE != file)
	{
		FindClose(file);
		if (0 != (FILE_ATTRIBUTE_DIRECTORY & findData.dwFileAttributes))
			return TRUE;
	}
	return FALSE;
}

static BOOL
Device_IsSizeOk(const wchar_t drive)
{
	const wchar_t test[] = {drive, L":\\"};
	ULARGE_INTEGER total;
	
	if (0 == GetDiskFreeSpaceExW(test, NULL, &total, NULL) || 
			total.HighPart == 0 && total.LowPart == 0) 
	{
		return FALSE;
	}

	return TRUE;
}

static BOOL
Device_IsOkToConnect(const wchar_t drive)
{
	const wchar_t test[] = {drive, TEXT(":\\Winamp\\")TAG_CACHE};
	wchar_t title[128] = {0};
	wchar_t message[1024] = {0};
	int result;

	if (FALSE != PathFileExistsW(test)) 
		return TRUE;
		
	StringCbPrintfW(message, sizeof(message), WASABI_API_LNGSTRINGW(IDS_REMOVEABLE_DRIVE_DETECTED),
			towupper(drive));

	WASABI_API_LNGSTRINGW_BUF(IDS_WINAMP_PMP_SUPPORT,title,ARRAYSIZE(title));

	result = MessageBoxW(NULL, message, title, 
							MB_YESNO | MB_SETFOREGROUND | MB_TOPMOST |  
							MB_ICONINFORMATION);
	
	if(IDNO == result)
	{
		return FALSE;
	}

	return TRUE;
}


static Nullsoft::Utility::LockGuard connect_guard;

static int ThreadFunc_Load(HANDLE handle, void *user_data, intptr_t id)
{
	wchar_t drive = (wchar_t)id;

	if (FALSE == Device_IsOkToConnect(drive))
	{
		Nullsoft::Utility::AutoLock connect_lock(connect_guard);
		blacklistAdd(drive);
		blacklistSave();
	}
	else
	{
		pmpDeviceLoading load;
		Device * d = new AndroidDevice(drive,&load);
	}

	loading_devices[drive-'A'] = false;
	deviceProvider->DecrementActivity();

	return 0;
}

//#include <WinIoCtl.h>

void connectDrive(wchar_t drive, bool checkSize=true, bool checkBlacklist=true) 
{
	Nullsoft::Utility::AutoLock connect_lock(connect_guard);
		// capitalize
	if (drive >= 'a' && drive <= 'z')
		drive = drive - 32;
	
	// reject invalid drive letters
	if (drive < 'A' || drive > 'Z')
		return;

	if(checkBlacklist && blacklistCheck(drive)) 
		return;

	// if device is taken already ignore
	for (std::vector<AndroidDevice*>::const_iterator e = devices.begin(); e != devices.end(); e++) 
	{
		if ((*e)->drive == drive) 
			return;
	}
	
	if (loading_devices[drive-'A'])
		return;

	loading_devices[drive-'A'] = true;
	
	if(FALSE == checkSize || FALSE != Device_IsSizeOk(drive))  
	{
		if (FALSE != Device_IsAndroid(drive) &&
			FALSE == Device_IsiPod(drive))
		{

			deviceProvider->IncrementActivity();
			if (NULL != WASABI_API_THREADPOOL &&
				0 == WASABI_API_THREADPOOL->RunFunction(0, ThreadFunc_Load, 0, 
												(int)drive, api_threadpool::FLAG_LONG_EXECUTION))
			{
				return;
			}
			deviceProvider->DecrementActivity();
		}
	}

	loading_devices[drive-'A'] = false;
}

static void autoDetectCallback(wchar_t drive,UINT type) 
{
	if(type == DRIVE_REMOVABLE)
	{
		connectDrive(drive, true, true);
	}
}


// end-android
static int Init() 
{
	WasabiInit();

	// start-android
	winampini = (const wchar_t*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETINIFILEW);
	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,PmpAndroidLangGUID);
	// end-android

	static wchar_t szDescription[256];
	StringCchPrintfW(szDescription, ARRAYSIZE(szDescription),
					 WASABI_API_LNGSTRINGW(IDS_NULLSOFT_ANDROID_DEVICE_PLUGIN), PLUGIN_VERSION);
	plugin.description = szDescription;

	/** load up the backlist */
	blacklistLoad();

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

static void Quit() 
{
	if (NULL != deviceProvider)
	{
		deviceProvider->Unregister();
		deviceProvider->Release();
		deviceProvider = NULL;
	}

	WasabiQuit();
	UnregisterDeviceNotification(hDevNotify);

	AndroidDevice::CloseDatabase();
}

static wchar_t FirstDriveFromMask(ULONG *unitmask) {
	char i;
	ULONG adj = 0x1, mask = *unitmask;
	for(i=0; i<26; ++i) {
		if(mask & 0x1) {
			*unitmask -= adj;
			break;
		}
		adj = adj << 1;
		mask = mask >> 1;
	}
	return (i+L'A');
}

static int GetNumberOfDrivesFromMask(ULONG unitmask) {
	int count = 0;
	for(int i=0; i<26; ++i)
	{
		if(unitmask & 0x1)
			count++;

		unitmask = unitmask >> 1;
	}
	return count;
}

static void CALLBACK RescanOnTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent,  DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	if (idEvent == rescanTimer)
		rescanTimer = 0;

	if (NULL == deviceProvider || 
		FAILED(deviceProvider->BeginDiscovery(AGAVE_API_DEVICEMANAGER)))
	{
		PostMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)autoDetectCallback,PMP_IPC_ENUM_ACTIVE_DRIVES);
	}
}


int wmDeviceChange(WPARAM wParam, LPARAM lParam) 
{
	UINT olderrmode=SetErrorMode(SEM_FAILCRITICALERRORS);
	if(wParam==DBT_DEVICEARRIVAL  || wParam==DBT_DEVICEREMOVECOMPLETE)
	{ // something has been inserted or removed
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
		if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
		{ // its a volume
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
			if((!(lpdbv->dbcv_flags & DBTF_MEDIA) && !(lpdbv->dbcv_flags & DBTF_NET))) 
			{ // its not a network drive or a CD/floppy, game on!
				ULONG dbcv_unitmask = lpdbv->dbcv_unitmask;

				// see just how many drives have been flagged on the action
				// eg one android drive could have multiple partitions that we handle
				int count = GetNumberOfDrivesFromMask(dbcv_unitmask);
				for(int j = 0; j < count; j++)
				{
					wchar_t drive = FirstDriveFromMask(&dbcv_unitmask);
					if((wParam == DBT_DEVICEARRIVAL) && !blacklistCheck(drive))
					{ // connected
						connectDrive(drive);
						//send a message as if the user just selected a drive from the combo box, this way the fields are refreshed to the correct device's settings
						SendMessage(config, WM_COMMAND,MAKEWPARAM(IDC_DRIVESELECT,CBN_SELCHANGE),0); 
					}
					else
					{ // removal
						for(size_t i=0; i < devices.size(); i++) 
						{
							AndroidDevice * d = (AndroidDevice*)devices.at(i);
							if(d->drive == drive)
							{
								devices.erase(devices.begin() + i);
								if(config) SendMessage(config,WM_USER,0,0); //refresh fields 
								if(config) SendMessage(config,WM_COMMAND, MAKEWPARAM(IDC_DRIVESELECT,CBN_SELCHANGE),0); //update to correct device change as if the user had clicked on the combo box themself
								SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)d,PMP_IPC_DEVICEDISCONNECTED);
								delete d;
								i--;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		rescanTimer = SetTimer(NULL, rescanTimer, 10000, RescanOnTimer);
	}
	SetErrorMode(olderrmode);
	return 0;
}

static int IsDriveConnectedToPMP(wchar_t drive) 
{
	for(size_t i = 0; i < devices.size(); i++) 
	{
		if(((AndroidDevice*)devices.at(i))->drive == drive) 
		{
			return 1;
		}
	}
	return 0;
}

static INT_PTR CALLBACK config_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	switch(uMsg) 
	{
		case WM_INITDIALOG:
			{
				for(wchar_t d=L'A'; d<='Z'; d++) 
				{
					wchar_t drive[3] = {d,L':',0}, drv[4] = {d,L':','\\',0};
					UINT uDriveType = GetDriveType(drv);
					if(uDriveType == DRIVE_REMOVABLE || uDriveType == DRIVE_CDROM || uDriveType == DRIVE_FIXED) {
						int position = (int)SendDlgItemMessageW(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_ADDSTRING,0,(LPARAM)drive);
						SendDlgItemMessage(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_SETITEMDATA,position,d);
					}
				}
				SendDlgItemMessage(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_SETCURSEL,0,0);
				SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO_MANUALCONNECT,CBN_SELCHANGE),0);
			}
			break;
		case WM_CLOSE:
			EndDialog(hwndDlg,0);
			break;
        case WM_COMMAND:
			switch(LOWORD(wParam)) {
			    case IDC_COMBO_MANUALCONNECT: 
				{
					if(HIWORD(wParam)==CBN_SELCHANGE) {
						int indx = (int)SendDlgItemMessageW(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETCURSEL,0,0);
						wchar_t drive = (wchar_t)SendDlgItemMessage(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETITEMDATA,indx,0);
						if(indx >= 0)
						{
							int connected = IsDriveConnectedToPMP(drive), isblacklisted = blacklistCheck(drive);
							EnableWindow(GetDlgItem(hwndDlg, IDC_MANUALCONNECT), !connected && !isblacklisted);
							EnableWindow(GetDlgItem(hwndDlg, IDC_MANUALDISCONNECT), connected);
							EnableWindow(GetDlgItem(hwndDlg, IDC_MANUALBLACKLIST), TRUE);
							SetDlgItemText(hwndDlg, IDC_MANUALBLACKLIST, WASABI_API_LNGSTRINGW(isblacklisted ? IDS_UNBLACKLIST_DRIVE : IDS_BLACKLIST_DRIVE));
						}
					}
				}
					break;
				case IDC_MANUALCONNECT:
				{
					char titleStr[32] = {0};
					if(MessageBoxA(hwndDlg, WASABI_API_LNGSTRING(IDS_MANUAL_CONNECT_PROMPT),
								   WASABI_API_LNGSTRING_BUF(IDS_WARNING,titleStr,32), MB_YESNO) == IDYES)
					{
						int indx = (int)SendDlgItemMessageW(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETCURSEL,0,0);
						wchar_t drive = (wchar_t)SendDlgItemMessage(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETITEMDATA,indx,0);
						if(drive >= L'A' && drive <= L'Z') {
							wchar_t *bl = makeBlacklistString(drive);
							if (bl)
							{
								for(size_t i=0; i<blacklist.size(); i++)
								{
									if(!wcscmp(bl,(wchar_t*)blacklist.at(i)))
									{
										free(blacklist.at(i));
										blacklist.erase(blacklist.begin() + i);
										break;
									}
								}
								free(bl);
							}
							connectDrive(drive,false);
							// should do a better check here incase of failure, etc
							EnableWindow(GetDlgItem(hwndDlg, IDC_MANUALCONNECT), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_MANUALDISCONNECT), TRUE);
						}
					}
				}
					break;
				case IDC_MANUALDISCONNECT:
				{
					int indx = (int)SendDlgItemMessageW(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETCURSEL,0,0);
					wchar_t drive = (wchar_t)SendDlgItemMessage(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETITEMDATA,indx,0);
					if(drive >= L'A' && drive <= L'Z') 
					{
						for(size_t i=0; i < devices.size(); i++) 
						{
							AndroidDevice * d = (AndroidDevice*)devices.at(i);
							if(d->drive == drive)
							{
								devices.erase(devices.begin() + i);
								if(config) SendMessage(config,WM_USER,0,0); //refresh fields 
								if(config) SendMessage(config,WM_COMMAND, MAKEWPARAM(IDC_DRIVESELECT,CBN_SELCHANGE),0); //update to correct device change as if the user had clicked on the combo box themself
								SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)d,PMP_IPC_DEVICEDISCONNECTED);
								SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO_MANUALCONNECT,CBN_SELCHANGE),0);
								delete d;
								i--;
							}
						}
					}
				}
					break;
				case IDC_MANUALBLACKLIST:
				{
					int indx = (int)SendDlgItemMessageW(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETCURSEL,0,0);
					wchar_t drive = (wchar_t)SendDlgItemMessage(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETITEMDATA,indx,0);
					if(drive >= L'A' && drive <= L'Z') {
						wchar_t *bl = makeBlacklistString(drive);
						if (bl)
						{
							if(!blacklistCheck(drive)) {
								blacklist.push_back(bl);
								// see if we've got a connected drive and prompt to remove it or wait till restart
								if(IsDriveConnectedToPMP(drive)) {
									wchar_t title[96] = {0};
									GetWindowText(hwndDlg, title, 96);
									if(MessageBox(hwndDlg,WASABI_API_LNGSTRINGW(IDS_DRIVE_CONNECTED_DISCONNECT_Q),title,MB_YESNO)==IDYES){
										SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_MANUALDISCONNECT,0),0);
									}
								}
							}
							else 
							{
								for(size_t i=0; i < blacklist.size(); i++)
								{
									if(!wcscmp(bl,(wchar_t*)blacklist.at(i)))
									{
										free(blacklist.at(i));
										blacklist.erase(blacklist.begin() + i);
										break;
									}
								}
								free(bl);
							}
						}
						SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO_MANUALCONNECT,CBN_SELCHANGE),0);
					}
				}
					break;
				case IDOK:
				case IDCANCEL:
					blacklistSave();
					EndDialog(hwndDlg,0);
					break;
			}
			break;
	}
	return 0;
}


static intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3) 
{
	switch(msg) {
		case PMP_DEVICECHANGE:
			return wmDeviceChange(param1,param2);
		case PMP_CONFIG:
			WASABI_API_DIALOGBOXW(IDD_CONFIG_GLOBAL,(HWND)param1,config_dialogProc);
			return 1;
	}
	return 0;
}

extern "C" 	__declspec(dllexport) PMPDevicePlugin *winampGetPMPDevicePlugin()
{
	return &plugin;
}

