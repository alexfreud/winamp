#define PLUGIN_NAME "Nullsoft Portable Music Player Support"
#define PLUGIN_VERSION L"2.25"

#include "main.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "..\..\General\gen_ml/ml.h"
#include "..\..\General\gen_ml/itemlist.h"
#include "../winamp/wa_ipc.h"
#include "nu/ns_wc.h"
#include "..\..\General\gen_hotkeys/wa_hotkeys.h"
#include "resource1.h"
#include "pmp.h"
#include "DeviceView.h"
#include "pluginloader.h"
#include "nu/AutoWide.h"
#include "api__ml_pmp.h"
#include "transcoder_imp.h"
#include <api/service/waservicefactory.h>
#include "config.h"
#include "tataki/export.h"
#include "nu/ServiceWatcher.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include "mt19937ar.h"
#include "./local_menu.h"
#include "pmpdevice.h"
#include "IconStore.h"
#include "../replicant/nx/nxstring.h"
#include <strsafe.h>
#include "../nu/MediaLibraryInterface.h"
#include <vector>

#define MAINTREEID 5002
#define PROGRESSTIMERID 1

static int init();
static void quit();
static INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

LRESULT CALLBACK DeviceMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK pmp_devices_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
extern void UpdateDevicesListView(bool softupdate);
INT_PTR CALLBACK global_config_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK config_dlgproc_plugins(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
void UpdateTransfersListView(bool softUpdate, CopyInst * item=NULL);

static void CALLBACK ProgressTimerTickCb(HWND hwnd, UINT uMsg, UINT_PTR eventId, unsigned long elapsed);

extern "C" winampMediaLibraryPlugin plugin = 
{
	MLHDR_VER,
	"nullsoft(ml_pmp.dll)",
	init,
	quit,
	PluginMessageProc,
	0,
	0,
	0,
};

C_ItemList devices;
C_ItemList loadingDevices;
extern HNAVITEM cloudQueueTreeItem;
static HNAVITEM navigationRoot = NULL;
static ATOM	viewAtom = 0;
int groupBtn = 1, customAllowed = 0, enqueuedef = 0;
extern HWND hwndMediaView;
extern DeviceView * currentViewedDevice;
HMENU m_context_menus = NULL, m_context_menus2 = NULL;
int prefsPageLoaded = 0, profile = 0;
prefsDlgRecW prefsPage;
prefsDlgRecW pluginsPrefsPage;
C_Config * global_config;
C_Config * gen_mlconfig;
UINT genhotkeys_add_ipc;
HANDLE hMainThread;
UINT_PTR mainTreeHandle;
extern HINSTANCE cloud_hinst;
extern int IPC_GET_CLOUD_HINST, IPC_LIBRARY_PLAYLISTS_REFRESH;
void deviceConnected(Device * dev);
void deviceDisconnected(Device * dev);
void deviceLoading(pmpDeviceLoading * load);
void deviceNameChanged(Device * dev);

//extern CRITICAL_SECTION listTransfersLock;
CRITICAL_SECTION csenumdrives;

api_playlists       *AGAVE_API_PLAYLISTS       = 0;
api_playlistmanager *AGAVE_API_PLAYLISTMANAGER = 0;
api_mldb            *AGAVE_API_MLDB            = 0;
api_memmgr          *WASABI_API_MEMMGR         = 0;
api_syscb           *WASABI_API_SYSCB          = 0;
api_application     *WASABI_API_APP            = 0;
api_podcasts        *AGAVE_API_PODCASTS        = 0;
api_albumart        *AGAVE_API_ALBUMART        = 0;
api_stats           *AGAVE_API_STATS           = 0;
api_threadpool      *WASABI_API_THREADPOOL     = 0;
api_devicemanager   *AGAVE_API_DEVICEMANAGER   = 0;
api_metadata        *AGAVE_API_METADATA        = 0;
// wasabi based services for localisation support
api_language        *WASABI_API_LNG            = 0;
HINSTANCE            WASABI_API_LNG_HINST      = 0;
HINSTANCE            WASABI_API_ORIG_HINST     = 0;

static const GUID pngLoaderGUID = 
	{ 0x5e04fb28, 0x53f5, 0x4032, { 0xbd, 0x29, 0x3, 0x2b, 0x87, 0xec, 0x37, 0x25 } };

static svc_imageLoader *wasabiPngLoader = NULL;

HWND mainMessageWindow = 0;
static bool classRegistered=0;
HWND CreateDummyWindow()
{
	if (!classRegistered)
	{
		WNDCLASSW wc = {0, };

		wc.style         = 0;
		wc.lpfnWndProc   = DeviceMsgProc;
		wc.hInstance     = plugin.hDllInstance;
		wc.hIcon         = 0;
		wc.hCursor       = NULL;
		wc.lpszClassName = L"ml_pmp_window";

		if (!RegisterClassW(&wc))
			return 0;

		classRegistered = true;
	}
	HWND dummy = CreateWindowW(L"ml_pmp_window", L"ml_pmp_window", 0, 0, 0, 0, 0, NULL, NULL, plugin.hDllInstance, NULL);

	return dummy;
}

genHotkeysAddStruct hksync     = { 0, HKF_UNICODE_NAME, WM_USER,     0, 0, "ml_pmp_sync",     0 };
genHotkeysAddStruct hkautofill = { 0, HKF_UNICODE_NAME, WM_USER + 1, 0, 0, "ml_pmp_autofill", 0 };
genHotkeysAddStruct hkeject    = { 0, HKF_UNICODE_NAME, WM_USER + 2, 0, 0, "ml_pmp_eject",    0 };

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

HNAVITEM NavigationItem_Find(HNAVITEM root, const wchar_t *name, BOOL allow_root = 0)
{
	NAVCTRLFINDPARAMS find;
	HNAVITEM item;

	if (NULL == name)
		return NULL;

	if (NULL == plugin.hwndLibraryParent)
		return NULL;

	find.pszName = (wchar_t*)name;
	find.cchLength = -1;
	find.compFlags = NICF_INVARIANT;
	find.fFullNameSearch = FALSE;

	item = MLNavCtrl_FindItemByName(plugin.hwndLibraryParent, &find);
	if (NULL == item)
		return NULL;

	if (!allow_root)
	{
		// if allowed then we can look for root level items which
		// is really for getting 'cloud' devices to another group
		if (NULL != root && 
			root != MLNavItem_GetParent(plugin.hwndLibraryParent, item))
		{
			item = NULL;
		}
	}

	return item;
}

HNAVITEM GetNavigationRoot(BOOL forceCreate)
{
	if (NULL == navigationRoot && 
		FALSE !=  forceCreate)
	{
		NAVINSERTSTRUCT nis = {0};
		wchar_t buffer[512] = {0};

		WASABI_API_LNGSTRINGW_BUF(IDS_PORTABLES, buffer, ARRAYSIZE(buffer));

		nis.hParent = NULL;
		nis.hInsertAfter = NCI_LAST;
		nis.item.cbSize = sizeof(NAVITEM);
		nis.item.mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_STYLE | NIMF_ITEMID | NIMF_PARAM;
		nis.item.id = MAINTREEID;
		nis.item.pszInvariant = L"Portables";
		nis.item.style = NIS_HASCHILDREN;
		nis.item.pszText = buffer;
		nis.item.lParam = -1;

		navigationRoot = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis);
		if (NULL != navigationRoot)
		{
			SetTimer(mainMessageWindow, PROGRESSTIMERID, 250, ProgressTimerTickCb);
		}
	}

	return navigationRoot;
}


static HNAVITEM GetNavigationItemFromMessage(int msg, INT_PTR param)
{
	return (msg < ML_MSG_NAVIGATION_FIRST) ? 
			MLNavCtrl_FindItemById(plugin.hwndLibraryParent, param) : 
			(HNAVITEM)param;
}

void Menu_ConvertRatingMenuStar(HMENU menu, UINT menu_id)
{
MENUITEMINFOW mi = {sizeof(mi), MIIM_DATA | MIIM_TYPE, MFT_STRING};
wchar_t rateBuf[32], *rateStr = rateBuf;
	mi.dwTypeData = rateBuf;
	mi.cch = 32;
	if(GetMenuItemInfoW(menu, menu_id, FALSE, &mi))
	{
		while(rateStr && *rateStr)
		{
			if(*rateStr == L'*') *rateStr = L'\u2605';
			rateStr=CharNextW(rateStr);
		}
		SetMenuItemInfoW(menu, menu_id, FALSE, &mi);
	}
}

static ServiceWatcher serviceWatcher;

static int init()
{
	// if there are no pmp_*.dll then no reason to load
	if (!testForDevPlugins())
		return ML_INIT_FAILURE;

	genrand_int31 = (int (*)())SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_RANDFUNC);

	if (0 == viewAtom)
	{
		viewAtom = GlobalAddAtomW(L"WinampPortableMediaView");
		if (0 == viewAtom)
			return 2;
	}

	TranscoderImp::init();
	InitializeCriticalSection(&csenumdrives);
	
	Tataki::Init(plugin.service);
	ServiceBuild( AGAVE_API_PLAYLISTS,       api_playlistsGUID );
	ServiceBuild( AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID );
	ServiceBuild( WASABI_API_SYSCB,          syscbApiServiceGuid );
	ServiceBuild( WASABI_API_APP,            applicationApiServiceGuid );
	ServiceBuild( AGAVE_API_STATS,           AnonymousStatsGUID );
	ServiceBuild( WASABI_API_THREADPOOL,     ThreadPoolGUID );
	ServiceBuild( AGAVE_API_DEVICEMANAGER,   DeviceManagerGUID );
	ServiceBuild( AGAVE_API_ALBUMART,        albumArtGUID );
	ServiceBuild( AGAVE_API_METADATA,        api_metadataGUID );

	// loader so that we can get the localisation service api for use
	ServiceBuild( WASABI_API_LNG,            languageApiGUID );
	ServiceBuild( WASABI_API_MEMMGR,         memMgrApiServiceGuid );

	// no guarantee that AGAVE_API_MLDB will be available yet, so we'll start a watcher for it
	serviceWatcher.WatchWith( plugin.service );
	serviceWatcher.WatchFor( &AGAVE_API_MLDB,     mldbApiGuid );
	serviceWatcher.WatchFor( &AGAVE_API_PODCASTS, api_podcastsGUID );

	WASABI_API_SYSCB->syscb_registerCallback( &serviceWatcher );


	mediaLibrary.library  = plugin.hwndLibraryParent;
	mediaLibrary.winamp   = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;

	mediaLibrary.GetIniDirectory();
	mediaLibrary.GetIniDirectoryW();

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,MlPMPLangGUID);
	
	static wchar_t szDescription[256];
	StringCbPrintfW(szDescription, ARRAYSIZE(szDescription), WASABI_API_LNGSTRINGW(IDS_NULLSOFT_PMP_SUPPORT), PLUGIN_VERSION);
	plugin.description = (char*)szDescription;

	hMainThread      = GetCurrentThread();
	//InitializeCriticalSection(&listTransfersLock);
	global_config    = new C_Config( (wchar_t *)mediaLibrary.GetWinampIniW() );
	profile          = global_config->ReadInt( L"profile", 0, L"Winamp" );

	gen_mlconfig     = new C_Config( (wchar_t *)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETMLINIFILEW ), L"gen_ml_config" );

	m_context_menus  = WASABI_API_LOADMENU( IDR_CONTEXTMENUS );
	m_context_menus2 = WASABI_API_LOADMENU( IDR_CONTEXTMENUS );


	HMENU rate_hmenu = GetSubMenu(GetSubMenu(m_context_menus,0),7);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_5);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_4);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_3);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_2);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_1);

	rate_hmenu = GetSubMenu(GetSubMenu(m_context_menus,1),4);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_5);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_4);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_3);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_2);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_1);

	rate_hmenu = GetSubMenu(GetSubMenu(m_context_menus,2),7);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_5);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_4);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_3);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_2);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_1);

	rate_hmenu = GetSubMenu(GetSubMenu(m_context_menus,7),5);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_5);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_4);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_3);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_2);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATE_1);

	//subclass winamp window
	mainMessageWindow = CreateDummyWindow();

	prefsPage.hInst = WASABI_API_LNG_HINST;
	prefsPage.dlgID = IDD_CONFIG_GLOBAL;
	prefsPage.name  = _wcsdup( WASABI_API_LNGSTRINGW( IDS_PORTABLES ) );
	prefsPage.where = 6;
	prefsPage.proc  = global_config_dlgproc;

	pluginsPrefsPage.hInst = WASABI_API_LNG_HINST;
	pluginsPrefsPage.dlgID = IDD_CONFIG_PLUGINS;
	pluginsPrefsPage.name  = prefsPage.name;
	pluginsPrefsPage.where = 1;
	pluginsPrefsPage.proc  = config_dlgproc_plugins;

	// only insert the portables page if we've actually loaded a pmp_*.dll
	int count = 0;
	if(loadDevPlugins(&count) && count > 0)
	{
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(intptr_t)&pluginsPrefsPage,IPC_ADD_PREFS_DLGW);
	}
	else if (!count)
	{
		// and if there are none, then cleanup and also notify ml_devices.dll to
		// shut-down as no need for it to keep running if there's not going to be
		// anything else running (as unlikely we'd have use for it without ml_pmp
		quit();

		winampMediaLibraryPlugin *(*gp)();
		gp = (winampMediaLibraryPlugin * (__cdecl *)(void))GetProcAddress(GetModuleHandleW(L"ml_devices.dll"), "winampGetMediaLibraryPlugin");
		if (gp)
		{
			winampMediaLibraryPlugin *mlplugin = gp();
			if (mlplugin && (mlplugin->version >= MLHDR_VER_OLD && mlplugin->version <= MLHDR_VER))
			{
				mlplugin->quit();
			}
		}

		return ML_INIT_FAILURE;
	}

	// we've got pmp_*.dll to load, so lets start the fun...
	Devices_Init();
	if (AGAVE_API_DEVICEMANAGER)
	{
		SetTimer(mainMessageWindow, PROGRESSTIMERID, 250, ProgressTimerTickCb);
	}
	else if (!global_config->ReadInt(L"HideRoot",0))
	{
		GetNavigationRoot(TRUE);
	}

	SetTimer(mainMessageWindow,COMMITTIMERID,5000,NULL);

	IPC_LIBRARY_PLAYLISTS_REFRESH = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"ml_playlist_refresh", IPC_REGISTER_WINAMP_IPCMESSAGE);
	IPC_GET_CLOUD_HINST = (INT)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloud", IPC_REGISTER_WINAMP_IPCMESSAGE);
	cloud_hinst = (HINSTANCE)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_HINST);

	// set up hotkeys...
	genhotkeys_add_ipc = SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&"GenHotkeysAdd",IPC_REGISTER_WINAMP_IPCMESSAGE);
	hksync.wnd = hkautofill.wnd = hkeject.wnd = mainMessageWindow;
	hksync.name = (char*)_wcsdup(WASABI_API_LNGSTRINGW(IDS_GHK_SYNC_PORTABLE_DEVICE));
	PostMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&hksync,genhotkeys_add_ipc);
	hkautofill.name = (char*)_wcsdup(WASABI_API_LNGSTRINGW(IDS_GHK_AUTOFILL_PORTABLE_DEVICE));
	PostMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&hkautofill,genhotkeys_add_ipc);
	hkeject.name = (char*)_wcsdup(WASABI_API_LNGSTRINGW(IDS_GHK_EJECT_PORTABLE_DEVICE));
	PostMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&hkeject,genhotkeys_add_ipc);
	return ML_INIT_SUCCESS;
}

bool quitting=false;
static void quit() 
{
	// trigger transfer kill
	int i = devices.GetSize();
	if (i > 0)
	{
		while(i-- > 0)
		{
			DeviceView * device = ((DeviceView*)devices.Get(i));
			if (device) device->threadKillswitch = 1;
		}
	}

	quitting = true;
	KillTimer(mainMessageWindow, COMMITTIMERID);
	DeviceMsgProc(NULL, WM_TIMER, COMMITTIMERID, 0);

	i = devices.GetSize();
	if (i > 0)
	{
		while(i-- > 0)
		{
			DeviceView * device = ((DeviceView*)devices.Get(i));
			if (device) device->dev->Close();
		}
	}
	unloadDevPlugins();

	stopServer();
	HWND f = mainMessageWindow;
	mainMessageWindow = 0;
	DestroyWindow(f);
	delete global_config;

	WASABI_API_SYSCB->syscb_deregisterCallback(&serviceWatcher);
	serviceWatcher.Clear();

	ServiceRelease( AGAVE_API_PLAYLISTS,       api_playlistsGUID );
	ServiceRelease( AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID );
	ServiceRelease( WASABI_API_SYSCB,          syscbApiServiceGuid );
	ServiceRelease( WASABI_API_APP,            applicationApiServiceGuid );
	ServiceRelease( WASABI_API_LNG,            languageApiGUID );
	ServiceRelease( WASABI_API_MEMMGR,         memMgrApiServiceGuid );
	ServiceRelease( AGAVE_API_MLDB,            mldbApiGuid );
	ServiceRelease( AGAVE_API_PODCASTS,        api_podcastsGUID );
	ServiceRelease( AGAVE_API_STATS,           AnonymousStatsGUID );
	ServiceRelease( AGAVE_API_DEVICEMANAGER,   DeviceManagerGUID );
	ServiceRelease( AGAVE_API_ALBUMART,        albumArtGUID );
	ServiceRelease( AGAVE_API_METADATA,        api_metadataGUID );

	if (NULL != wasabiPngLoader)
	{
		ServiceRelease(wasabiPngLoader, pngLoaderGUID);
		wasabiPngLoader = NULL;
	}

	DeleteCriticalSection(&csenumdrives);
	TranscoderImp::quit();

	delete gen_mlconfig;
	Tataki::Quit();	
	ServiceRelease(WASABI_API_THREADPOOL, ThreadPoolGUID);

	if (0 != viewAtom)
	{
		GlobalDeleteAtom(viewAtom);
		viewAtom = 0;
	}
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

static int spc_GetState(spc *s)
{
	EnterCriticalSection(&s->lock);
	int state = s->state;
	LeaveCriticalSection(&s->lock);
	return state;
}

// p must be of type void (CALLBACK *)(ULONG_PTR dwParam). Returns 0 for success.
int SynchronousProcedureCall(void * p, ULONG_PTR dwParam) {
	if(!p) return 1;
	spc * s = (spc*)calloc(1, sizeof(spc));
	InitializeCriticalSection(&s->lock);
	s->param = dwParam;
	s->proc = p;
	s->state = 0;
#if 1 // _WIN32_WINNT >= 0x0400
	if(!QueueUserAPC(spc_caller,hMainThread,(ULONG_PTR)s)) { DeleteCriticalSection(&s->lock); free(s); return 1; } //failed
#else
	if(!mainMessageWindow) { DeleteCriticalSection(&s->lock); free(s); return 1; } else PostMessage(mainMessageWindow,WM_USER+3,(WPARAM)spc_caller,(LPARAM)s);
#endif
	int i=0;
	while (spc_GetState(s) != 1)
	{
		if (SleepEx(10,TRUE) == 0)
			i++;
		if(i >= 100) 
		{
			EnterCriticalSection(&s->lock);
			s->state = -1;
			LeaveCriticalSection(&s->lock);
			return 1;
		}
	}
	DeleteCriticalSection(&s->lock);
	free(s);
	return 0;
}

static VOID CALLBACK getTranscoder(ULONG_PTR dwParam)
{
	C_Config * conf = global_config;
	for(int i=0; i<devices.GetSize(); i++)
	{
		DeviceView * d = (DeviceView *)devices.Get(i);
		if(d->dev == *(Device **)dwParam) { conf = d->config; break; }
	}
	Transcoder ** t = (Transcoder**)dwParam;
	*t = new TranscoderImp(plugin.hwndWinampParent,plugin.hDllInstance,conf, *(Device **)dwParam);
}

static void CALLBACK ProgressTimerTickCb(HWND hwnd, UINT uMsg, UINT_PTR eventId, unsigned long elapsed)
{
	wchar_t *buf = (wchar_t*)calloc(sizeof(wchar_t), 256);
	NAVITEM itemInfo = {0};
	HNAVITEM rootItem = GetNavigationRoot(FALSE);
	if (!AGAVE_API_DEVICEMANAGER && NULL == rootItem)
	{
		KillTimer(hwnd, eventId);
		if (buf) free(buf);
		return;
	}

	// TODO need to get this working for a merged instance...
	int num = 0,total = 0, percent = 0;
	for(int i=0; i<devices.GetSize(); i++)
	{
		DeviceView * dev = (DeviceView*)devices.Get(i);
		LinkedQueue * txQueue = getTransferQueue(dev);
		LinkedQueue * finishedTX = getFinishedTransferQueue(dev);
		int txProgress = getTransferProgress(dev);
		int size = (txQueue ? txQueue->GetSize() : 0);
		int device_num = (100 * size) - txProgress;
		num += device_num;
		int device_total = 100 * size + 100 * (finishedTX ? finishedTX->GetSize() : 0);
		total += device_total;
		percent = (0 != device_total) ? (((device_total - device_num) * 100) / device_total) : -1;
		// TODO need to do something to handle it sticking on 100%
		if (dev->queueTreeItem)
		{
			itemInfo.cbSize = sizeof(itemInfo);
			itemInfo.hItem = dev->queueTreeItem;
			itemInfo.mask = NIMF_PARAM;

			if (MLNavItem_GetInfo(plugin.hwndLibraryParent, &itemInfo) && itemInfo.lParam != percent)
			{
				if (-1 == percent)
					WASABI_API_LNGSTRINGW_BUF(IDS_TRANSFERS,buf, 256);
				else
					StringCbPrintfW(buf, 256, WASABI_API_LNGSTRINGW(IDS_TRANSFERS_PERCENT), percent); 

				itemInfo.mask |= NIMF_TEXT;
				itemInfo.pszText = buf;
				itemInfo.lParam = percent;
				itemInfo.cchTextMax = -1;

				MLNavItem_SetInfo(plugin.hwndLibraryParent, &itemInfo);
			}
		}

		dev->UpdateActivityState();
	}

	if (!rootItem)
	{
		if (buf) free(buf);
		return;
	}

	percent = (0 != total) ? (((total-num)*100)/total) : -1;

	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.hItem = rootItem;
	itemInfo.mask = NIMF_PARAM;

	if (FALSE == MLNavItem_GetInfo(plugin.hwndLibraryParent, &itemInfo))
	{
		if (buf) free(buf);
		return;
	}

	if (itemInfo.lParam != percent)
	{
		if (-1 == percent)
			WASABI_API_LNGSTRINGW_BUF(IDS_PORTABLES,buf,256);
		else
			StringCbPrintfW(buf, sizeof(buf), WASABI_API_LNGSTRINGW(IDS_PORTABLES_PERCENT),percent);
	
		itemInfo.mask |= NIMF_TEXT;
		itemInfo.pszText = buf;
		itemInfo.lParam = percent;
		itemInfo.cchTextMax = -1;

		MLNavItem_SetInfo(plugin.hwndLibraryParent, &itemInfo);
	}
	if (buf) free(buf);
}

extern LRESULT CALLBACK TranscodeMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void EnumDrives(ENUM_DRIVES_CALLBACK callback)
{
	DWORD drives=GetLogicalDrives();
	wchar_t drivestr[4] = L"X:\\";
	for(int i=2;i<25;i++) if(drives&(1<<i))
	{
		EnterCriticalSection(&csenumdrives);
		UINT olderrmode=SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
		drivestr[0] = L'A'+i;
		callback(drivestr[0],GetDriveTypeW(drivestr));
		SetErrorMode(olderrmode);
		LeaveCriticalSection(&csenumdrives);
	}
}

LRESULT CALLBACK DeviceMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_WA_IPC && hwnd != mainMessageWindow) return TranscodeMsgProc(hwnd,uMsg,wParam,lParam);
		switch(uMsg)
		{
			case WM_USER:   // hotkey: sync
				if(devices.GetSize() > 0)
				{
					if (!((DeviceView *)devices.Get(0))->isCloudDevice)
						((DeviceView *)devices.Get(0))->Sync();
					else
						((DeviceView *)devices.Get(0))->CloudSync();
				}
				break;
			case WM_USER+1: // hotkey: autofill
				if(devices.GetSize() > 0)
					((DeviceView *)devices.Get(0))->Autofill();
				break;
			case WM_USER+2: // hotkey: eject
				if(devices.GetSize() > 0)
					((DeviceView *)devices.Get(0))->Eject();
				break;
			case WM_USER+3:
				{
					void (CALLBACK *p)(ULONG_PTR dwParam);
					p =  (void (CALLBACK *)(ULONG_PTR dwParam))wParam;
					p((ULONG_PTR)lParam);
				}
				break;
			case WM_USER+4: // refreshes cloud views (re-checks if we've moved away)
				if (IsWindow(hwndMediaView) && currentViewedDevice && currentViewedDevice->isCloudDevice)
				{
					PostMessage(plugin.hwndLibraryParent, WM_USER + 30, 0, 0);
				}
				break;
			case WM_USER+5:	// removes cloud devices from a cloud sources logout...
			{
				for (int i = devices.GetSize() - 1; i > -1; --i)
				{
					DeviceView * dev = (DeviceView *)devices.Get(i);
					if (dev->isCloudDevice)
						dev->dev->Close();
				}
				break;
			}
			case WM_PMP_IPC:
			{
				switch(lParam)
				{
					case PMP_IPC_DEVICECONNECTED:
						deviceConnected((Device*)wParam);
						break;
					case PMP_IPC_DEVICEDISCONNECTED:
						deviceDisconnected((Device*)wParam);
						break;
					case PMP_IPC_DEVICELOADING:
						deviceLoading((pmpDeviceLoading *)wParam);
						break;
					case PMP_IPC_DEVICENAMECHANGED:
						deviceNameChanged((Device*)wParam);
						break;
					case PMP_IPC_DEVICECLOUDTRANSFER:
					{
						int ret = 0;							
						cloudDeviceTransfer * transfer = (cloudDeviceTransfer *)wParam;
						if (transfer)
						{
							for (int i = 0; i < devices.GetSize(); i++)
							{
								DeviceView * dev = (DeviceView *)devices.Get(i);
								if (dev->dev->extraActions(DEVICE_IS_CLOUD_TX_DEVICE, (intptr_t)transfer->device_token, 0, 0))
								{
									ret = dev->TransferFromML(ML_TYPE_FILENAMESW, (void *)transfer->filenames, 0, 1);
								}
							}
						}
						return ret;
					}
					case PMP_IPC_GETCLOUDTRANSFERS:
					{
						typedef std::vector<wchar_t*> CloudFiles;
						CloudFiles *pending = (CloudFiles *)wParam;
						if (pending)
						{
							cloudTransferQueue.lock();
							// TODO de-dupe incase going to multiple devices...
							for(int i = 0; i < cloudTransferQueue.GetSize(); i++)
							{
								pending->push_back(((CopyInst *)cloudTransferQueue.Get(i))->sourceFile);
							}
							cloudTransferQueue.unlock();

							return pending->size();
						}
						return 0;
					}
					case PMP_IPC_GET_TRANSCODER:
					{
						void * t = (void*)wParam;
						getTranscoder((ULONG_PTR)&t);
						if (t == (void*)wParam) return 0;
						return (LRESULT)t;
					}
					case PMP_IPC_RELEASE_TRANSCODER:
						delete ((TranscoderImp *)wParam);
						break;
					case PMP_IPC_ENUM_ACTIVE_DRIVES:
						{
							if (wParam == 0)
								return (LRESULT)EnumDrives;
							else
								EnumDrives((ENUM_DRIVES_CALLBACK)wParam);
						}
						break;
					case PMP_IPC_GET_INI_FILE:
						for (int i = 0; i < devices.GetSize(); i++)
						{
							DeviceView * dev = (DeviceView *)devices.Get(i);
							if(dev->dev == (Device*)wParam) return (intptr_t)dev->config->GetIniFile();
						}
						break;
					case PMP_IPC_GET_PREFS_VIEW:
					{
						pmpDevicePrefsView *view = (pmpDevicePrefsView *)wParam;
						if (view)
						{
							for (int i = 0; i < devices.GetSize(); i++)
							{
								DeviceView * dev = (DeviceView *)devices.Get(i);
								if (!lstrcmpA(dev->GetName(), view->dev_name))
								{
									return (LRESULT)OnSelChanged(view->parent, view->parent, dev);
								}
							}
						}
						return 0;
					}
				}
			}
		break;
	case WM_DEVICECHANGE:
		{
			int r = wmDeviceChange(wParam,lParam);
			if (r) return r;
		}
		break;
	case WM_TIMER:
		switch(wParam)
		{
			case COMMITTIMERID:
				{
					for(int i=0; i<devices.GetSize(); i++)
					{
						DeviceView * d = (DeviceView *)devices.Get(i);
						LinkedQueue * txQueue = getTransferQueue(d);
						if (txQueue)
						{
							if(d->commitNeeded && !txQueue->GetSize())
							{
								d->commitNeeded = false;
								d->dev->commitChanges();
							}
						}
					}
				}
				break;
		}
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void UpdateLoadingCaption(wchar_t * caption, void * context)
{
	NAVITEM itemInfo = {0};

	itemInfo.hItem = (HNAVITEM)context;
	if (NULL == itemInfo.hItem)
		return;

	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.pszText = caption;
	itemInfo.cchTextMax = -1;
	itemInfo.mask = NIMF_TEXT;

	MLNavItem_SetInfo(plugin.hwndLibraryParent, &itemInfo);
}

void deviceLoading(pmpDeviceLoading * load)
{
	if (!AGAVE_API_DEVICEMANAGER)
	{
		wchar_t buffer[256] = {0};
		NAVINSERTSTRUCT nis = {0};
		MLTREEIMAGE devIcon;

		devIcon.resourceId = IDR_DEVICE_ICON;
		devIcon.hinst = plugin.hDllInstance;
		if(load->dev)
			load->dev->extraActions(DEVICE_SET_ICON,(intptr_t)&devIcon,0,0);

		nis.hParent = GetNavigationRoot(TRUE);
		nis.hInsertAfter = NCI_LAST;
		nis.item.cbSize = sizeof(NAVITEM);
		nis.item.mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_STYLE | NIMF_IMAGE | NIMF_IMAGESEL;
		nis.item.pszText = WASABI_API_LNGSTRINGW_BUF(IDS_LOADING, buffer, ARRAYSIZE(buffer));
		nis.item.pszInvariant = L"device_loading";
		nis.item.style = NIS_ITALIC;
		nis.item.iImage = icon_store.GetResourceIcon(devIcon.hinst, (LPCWSTR)MAKEINTRESOURCE(devIcon.resourceId));
		nis.item.iSelectedImage = nis.item.iImage;
		load->context = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis);
		load->UpdateCaption = UpdateLoadingCaption;
		loadingDevices.Add(load);
	}
}

void finishDeviceLoading(Device * dev)
{
	for(int i=0; i < loadingDevices.GetSize(); i++)
	{
		pmpDeviceLoading * l = (pmpDeviceLoading *)loadingDevices.Get(i);
		if(l->dev == dev) {
			loadingDevices.Del(i);
			HNAVITEM treeItem = (HNAVITEM)l->context;
			MLNavCtrl_DeleteItem(plugin.hwndLibraryParent,treeItem); 
			return;
		}
	}
}

void deviceConnected(Device * dev)
{
	if (!AGAVE_API_DEVICEMANAGER)
		GetNavigationRoot(TRUE);

	Device * oldDev = dev;

	finishDeviceLoading(oldDev);
	if(!devices.GetSize()) startServer();

	DeviceView *pmp_device = new DeviceView(dev);
	devices.Add(pmp_device);
	UpdateDevicesListView(false);
}

void deviceDisconnected(Device * dev)
{
	finishDeviceLoading(dev);
	int cloud_count = 0;
	for(int i = 0; i < devices.GetSize(); i++) 
	{
		DeviceView * d = (DeviceView *)devices.Get(i);
		if(d->dev == dev)
		{
			d->threadKillswitch = 1;
			d->transferContext.WaitForKill();
			devices.Del(i);
			d->Unregister();
			d->Release();
		}
		else
		{
			// to keep the 'cloud library' node on the correct expanded state
			// we need to adjust the cloud sources count due to 'all sources'
			if (_strnicmp(d->GetName(), "all_sources", 11) && d->isCloudDevice)
			{
				cloud_count += 1;
			}
		}
	}

	// if we're only showing a single device, then we can just disable everything else
	if (cloud_count == 0)
	{
		HNAVITEM cloud_transfers = NavigationItem_Find(0, L"cloud_transfers", TRUE);
		if (cloud_transfers != NULL)
		{
			MLNavCtrl_DeleteItem(plugin.hwndLibraryParent, cloud_transfers);
			cloudQueueTreeItem = NULL;
		}

		cloud_transfers = NavigationItem_Find(0, L"cloud_add_sources", TRUE);
		if (cloud_transfers != NULL)
		{
			MLNavCtrl_DeleteItem(plugin.hwndLibraryParent, cloud_transfers);
		}

		cloud_transfers = NavigationItem_Find(0, L"cloud_byos", TRUE);
		if (cloud_transfers != NULL)
		{
			MLNavCtrl_DeleteItem(plugin.hwndLibraryParent, cloud_transfers);
		}

		HNAVITEM cloud = NavigationItem_Find(0, L"cloud_sources", TRUE);
		if (cloud != NULL)
		{
			NAVITEM item = {0};
			item.cbSize = sizeof(NAVITEM);
			item.mask = NIMF_IMAGE | NIMF_IMAGESEL;
			item.hItem = cloud;
			item.iImage = item.iSelectedImage = mediaLibrary.AddTreeImageBmp(IDB_TREEITEM_CLOUD);
			MLNavItem_SetInfo(plugin.hwndLibraryParent, &item);
		}
	}

	UpdateDevicesListView(false);
	if(!devices.GetSize() && !loadingDevices.GetSize() && global_config->ReadInt(L"HideRoot",0))
	{
	    HNAVITEM rootItem = GetNavigationRoot(FALSE);
		if (NULL != rootItem)
			MLNavCtrl_DeleteItem(plugin.hwndLibraryParent, rootItem);
	}
	if(!devices.GetSize()) stopServer();
}

void deviceNameChanged(Device * dev)
{
	finishDeviceLoading(dev);
	for(int i=0; i < devices.GetSize(); i++) 
	{
		DeviceView * d = (DeviceView *)devices.Get(i);
		if(d->dev == dev)
		{
			wchar_t *buffer = (wchar_t*)calloc(sizeof(wchar_t),128);
			if (buffer)
			{
				dev->getPlaylistName(0, buffer, 128);
				UpdateLoadingCaption(buffer, d->treeItem);
				d->SetDisplayName(buffer, 1);
				free(buffer);
			}
			break;
		}
	}
}

svc_imageLoader *GetPngLoaderService()
{
	if (NULL == wasabiPngLoader)
	{
		if (NULL == WASABI_API_MEMMGR)
			return NULL;

		ServiceBuild(wasabiPngLoader, pngLoaderGUID);
	}

	return wasabiPngLoader;
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

BOOL
CenterWindow(HWND window, HWND centerWindow)
{
	RECT centerRect, windowRect;
	long x, y;

	if (NULL == window ||
		FALSE == GetWindowRect(window, &windowRect))
	{
		return FALSE;
	}

	if (CENTER_OVER_ML_VIEW == centerWindow)
	{
		centerWindow = (HWND)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_GETCURRENTVIEW, 0);
		if (NULL == centerWindow || FALSE == IsWindowVisible(centerWindow))
			centerWindow = CENTER_OVER_ML;
	}

	if (CENTER_OVER_ML == centerWindow)
	{
		centerWindow = plugin.hwndLibraryParent;
		if (NULL == centerWindow || FALSE == IsWindowVisible(centerWindow))
			centerWindow = CENTER_OVER_WINAMP;
	}
	
	if (CENTER_OVER_WINAMP == centerWindow)
	{
		centerWindow = (HWND)SENDWAIPC(plugin.hwndWinampParent, IPC_GETDIALOGBOXPARENT, 0);
		if (FALSE == IsChild(centerWindow, plugin.hwndLibraryParent))
			centerWindow = NULL;
	}
		
	if (NULL == centerWindow || 
		FALSE == IsWindowVisible(centerWindow) ||
		FALSE == GetWindowRect(centerWindow, &centerRect))
	{
		HMONITOR monitor;
		MONITORINFO monitorInfo;

		monitor = MonitorFromWindow(plugin.hwndWinampParent, MONITOR_DEFAULTTONEAREST);
		
		monitorInfo.cbSize = sizeof(monitorInfo);

		if (NULL == monitor ||
			FALSE == GetMonitorInfo(monitor, &monitorInfo) ||
			FALSE == CopyRect(&centerRect, &monitorInfo.rcWork))
		{
			CopyRect(&centerRect, &windowRect);
		}
	}

	x = centerRect.left + ((centerRect.right - centerRect.left)- (windowRect.right - windowRect.left))/2;
	y = centerRect.top + ((centerRect.bottom - centerRect.top)- (windowRect.bottom - windowRect.top))/2;

	if (x == windowRect.left && y == windowRect.top)
		return FALSE;

	return SetWindowPos(window, NULL, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}
ATOM
GetViewAtom()
{
	return viewAtom;
}

void *
GetViewData(HWND hwnd)
{
	return GetPropW(hwnd, (const wchar_t*)MAKEINTATOM(viewAtom));
}

BOOL
SetViewData(HWND hwnd, void *data)
{
	return SetPropW(hwnd, (const wchar_t*)MAKEINTATOM(viewAtom), data);
}

void *
RemoveViewData(HWND hwnd)
{
	return RemovePropW(hwnd, (const wchar_t*)MAKEINTATOM(viewAtom));
}

static void URL_GetParameter(const wchar_t *param_start, char *dest, size_t dest_len)
{
	if (!param_start)
	{
		dest[0]=0;
		return;
	}

	while (param_start && *param_start++ != '=');

	while (param_start && *param_start && dest_len > 1 && *param_start != L'&')
	{
		if (*param_start == '+') 
    {
      param_start++;
      *dest++=' ';
    }
	  else if (*param_start == L'%' && param_start[1] != L'%' && param_start[1])
	  {
		  int a=0;
		  int b=0;
		  for ( b = 0; b < 2; b ++)
		  {
			  int r=param_start[1+b];
			  if (r>='0'&&r<='9') r-='0';
			  else if (r>='a'&&r<='z') r-='a'-10;
			  else if (r>='A'&&r<='Z') r-='A'-10;
			  else break;
			  a*=16;
			  a+=r;
		  }
		  if (b < 2) 
			{
				*dest++=(char)*param_start++;
			}
		  else 
			{
				*dest++=a; 
				param_start += 3;}
	  }
	  else
		{
			*dest++=(char)*param_start++;
		}
	  dest_len--;
	}
	*dest = 0;
}

// this only works for one-character parameters
static const wchar_t *GetParameterStart(const wchar_t *url, wchar_t param)
{
	wchar_t lookup[4] = { L'&', param, L'=', 0 };
	lookup[1] = param;
	const wchar_t *val = wcsstr(url, lookup);
	if (!val)
	{
		lookup[0] = L'?';
		val = wcsstr(url, lookup);
	}
	return val;
}
extern int serverPort;
static INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	int i;
	if (message_type == ML_IPC_HOOKEXTINFOW)
	{
		extendedFileInfoStructW *hookMetadata = (extendedFileInfoStructW *)param1;
		if (hookMetadata->filename && !wcsncmp(hookMetadata->filename, L"http://127.0.0.1:", 17) && _wtoi(hookMetadata->filename + 17) == serverPort)
		{
			if (!_wcsicmp(hookMetadata->metadata, L"artist"))
			{
				char metadata[1024] = {0};
				URL_GetParameter(GetParameterStart(hookMetadata->filename, 'a'), metadata, 1024);
				MultiByteToWideCharSZ(CP_UTF8, 0, metadata, -1, hookMetadata->ret, hookMetadata->retlen);
				return 1;
			}
			else if (!_wcsicmp(hookMetadata->metadata, L"album"))
			{
				char metadata[1024] = {0};
				URL_GetParameter(GetParameterStart(hookMetadata->filename, 'l'), metadata, 1024);
				MultiByteToWideCharSZ(CP_UTF8, 0, metadata, -1, hookMetadata->ret, hookMetadata->retlen);
				return 1;
			}
			else if (!_wcsicmp(hookMetadata->metadata, L"title"))
			{
				char metadata[1024] = {0};
				URL_GetParameter(GetParameterStart(hookMetadata->filename, 't'), metadata, 1024);
				MultiByteToWideCharSZ(CP_UTF8, 0, metadata, -1, hookMetadata->ret, hookMetadata->retlen);
				return 1;
			}
		}
	}

	for(i=0; i < devices.GetSize(); i++)
	{
		DeviceView *deviceView;
		deviceView = (DeviceView *)devices.Get(i);
		if (NULL != deviceView)
		{
			INT_PTR a= deviceView->MessageProc(message_type,param1,param2,param3); 
			if(0 != a) 
				return a;
		}
	}

	
	if (message_type >= ML_MSG_TREE_BEGIN && 
		message_type <= ML_MSG_TREE_END)
	{
		HNAVITEM item, rootItem;
		item = GetNavigationItemFromMessage(message_type, param1);
		rootItem = GetNavigationRoot(FALSE);
		
		if(message_type == ML_MSG_TREE_ONCREATEVIEW)
		{
			for(i=0; i < loadingDevices.GetSize(); i++)
			{
				pmpDeviceLoading * l = (pmpDeviceLoading *)loadingDevices.Get(i);
				if(NULL != l->context)
				{					
					if (((HNAVITEM)l->context) == item)
					{
						HNAVITEM parentItem;
						parentItem = MLNavItem_GetParent(plugin.hwndLibraryParent, item);
						if (NULL == parentItem)
							parentItem = rootItem;

						PostMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)parentItem, ML_IPC_NAVITEM_SELECT);
						return 0;
					}
				}
			}
		}

		if(NULL != item && item == rootItem) 
		{
			switch (message_type)
			{
				case ML_MSG_TREE_ONCREATEVIEW:
					return (INT_PTR)WASABI_API_CREATEDIALOGW(IDD_VIEW_PMP_DEVICES,(HWND)param2,pmp_devices_dlgproc);
				case ML_MSG_TREE_ONCLICK:
					switch(param2)
					{
						case ML_ACTION_RCLICK:
							{
								HMENU menu = GetSubMenu(m_context_menus,6);
								int hideRoot = global_config->ReadInt(L"HideRoot",0);
								CheckMenuItem(menu,ID_MAINTREEROOT_AUTOHIDEROOT,hideRoot?MF_CHECKED:MF_UNCHECKED);
								POINT p;
								GetCursorPos(&p);
								int r = Menu_TrackSkinnedPopup(menu,TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY,p.x,p.y,plugin.hwndLibraryParent,NULL);
								switch(r)
								{
									case ID_MAINTREEROOT_AUTOHIDEROOT:
										hideRoot = hideRoot?0:1;
										global_config->WriteInt(L"HideRoot",hideRoot);
										if(hideRoot && devices.GetSize() == 0)
										{
											MLNavCtrl_DeleteItem(plugin.hwndLibraryParent, rootItem);
										}
										break;
									case ID_MAINTREEROOT_PREFERENCES:
										SENDWAIPC(plugin.hwndWinampParent, IPC_OPENPREFSTOPAGE, &pluginsPrefsPage);
										break;
									case ID_MAINTREEROOT_HELP:
										SENDWAIPC(plugin.hwndWinampParent, IPC_OPEN_URL, L"https://help.winamp.com/hc/articles/8106455294612-Winamp-Portables-Guide");
										break;
								}
							}
							break;
						case ML_ACTION_DBLCLICK:
							break;
						case ML_ACTION_ENTER:
							break;
					}
					break;
				case ML_MSG_TREE_ONDROPTARGET:
					break;
				case ML_MSG_TREE_ONDRAG:
					break;
				case ML_MSG_TREE_ONDROP:
					break;
				case ML_MSG_NAVIGATION_ONDELETE:
					navigationRoot = NULL;
					KillTimer(mainMessageWindow, PROGRESSTIMERID);
					return TRUE;
			}
		}
	}
	else if (message_type == ML_MSG_NO_CONFIG)
	{
		if(prefsPage._id == 0)
			return TRUE;
	}
	else if (message_type == ML_MSG_CONFIG)
	{
		if(prefsPage._id == 0) return 0;
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, prefsPage._id, IPC_OPENPREFSTOPAGE);
	}
	else if (message_type == ML_MSG_NOTOKTOQUIT)
	{
		// see if we have any transfers in progress and if so then prompt on what to do...
		bool transfers = false;
		for (int i = 0; i < devices.GetSize(); i++)
		{
			DeviceView * d = (DeviceView *)devices.Get(i);
			LinkedQueue * txQueue = getTransferQueue(d);
			if(txQueue && txQueue->GetSize() > 0)
			{
				transfers = true;
				break;
			}
		}

		if (transfers)
		{
			wchar_t titleStr[32] = {0};
			if (MessageBoxW(plugin.hwndLibraryParent, WASABI_API_LNGSTRINGW(IDS_CANCEL_TRANSFERS_AND_QUIT),
				WASABI_API_LNGSTRINGW_BUF(IDS_CONFIRM_QUIT,titleStr,32), MB_YESNO | MB_ICONQUESTION) == IDNO)
				return TRUE;
		}
		return FALSE;
	}
	else if(message_type == ML_MSG_ONSENDTOBUILD)
	{
		if (param1 == ML_TYPE_ITEMRECORDLIST || param1 == ML_TYPE_ITEMRECORDLISTW ||
			param1 == ML_TYPE_FILENAMES || param1 == ML_TYPE_FILENAMESW ||
			param1 == ML_TYPE_PLAYLISTS || param1 == ML_TYPE_PLAYLIST)
		{
			if (gen_mlconfig->ReadInt(L"pmp_send_to", DEFAULT_PMP_SEND_TO))
			{
				for(int m = 0, mode = 0; m < 2; m++, mode++)
				{
					int added = 0;
					for(i = 0; i < devices.GetSize(); i++)
					{
						DeviceView *deviceView;
						deviceView = (DeviceView *)devices.Get(i);
						if (NULL != deviceView)
						{
							if (deviceView->dev->extraActions(DEVICE_SENDTO_UNSUPPORTED, 0, 0, 0) == 0)
							{
								wchar_t buffer[128] = {0};
								deviceView->dev->getPlaylistName(0, buffer, 128);
								if (buffer[0])
								{
									// TODO - this is to block true playlists from appearing on the sendto
									//		for cloud playlists handling - remove this when we can do more
									//		than just uploading the playlist blob without the actual files
									if (deviceView->isCloudDevice && param3 == ML_TYPE_PLAYLIST) continue;

									if (!deviceView->isCloudDevice == mode)
									{
										if (!added)
										{
											mediaLibrary.BranchSendTo(param2);
											added = 1;
										}
										mediaLibrary.AddToBranchSendTo(buffer, param2, reinterpret_cast<INT_PTR>(deviceView));
									}
								}
							}
						}
					}

					if (added)
					{
						mediaLibrary.EndBranchSendTo(WASABI_API_LNGSTRINGW((!m ? IDS_SENDTO_CLOUD : IDS_SENDTO_DEVICES)), param2);
					}
				}
			}
		}
	}

	else if (message_type == ML_MSG_VIEW_PLAY_ENQUEUE_CHANGE)
	{
		enqueuedef = param1;
		groupBtn = param2;
		if (IsWindow(hwndMediaView))
			PostMessage(hwndMediaView, WM_APP + 104, param1, param2);
		return 0;
	}

	return 0;
}

extern "C" {
	__declspec( dllexport ) winampMediaLibraryPlugin * winampGetMediaLibraryPlugin()
	{
		return &plugin;
	}

	__declspec( dllexport ) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
		// cleanup the ml tree so the portables root isn't left

		HNAVITEM rootItem = GetNavigationRoot(FALSE);
		if(NULL != rootItem)
			MLNavCtrl_DeleteItem(plugin.hwndLibraryParent, rootItem);
		
		// prompt to remove our settings with default as no (just incase)
		wchar_t title[256] = {0};
		StringCbPrintfW(title, ARRAYSIZE(title), WASABI_API_LNGSTRINGW(IDS_NULLSOFT_PMP_SUPPORT), PLUGIN_VERSION);
		if(MessageBoxW(hwndDlg,WASABI_API_LNGSTRINGW(IDS_DO_YOU_ALSO_WANT_TO_REMOVE_SETTINGS),
					  title,MB_YESNO|MB_DEFBUTTON2) == IDYES)
		{
			global_config->WriteString(0,0);
		}

		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(intptr_t)&pluginsPrefsPage,IPC_REMOVE_PREFS_DLG);

		// allow an on-the-fly removal (since we've got to be with a compatible client build)
		return ML_PLUGIN_UNINSTALL_NOW;
	}
};