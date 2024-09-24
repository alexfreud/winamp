#include "api__ml_history.h"
#include "main.h"
#include "..\..\General\gen_ml/config.h"
#include "HistoryAPIFactory.h"
#include "JSAPI2_Creator.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include "../nu/AutoCharFn.h"
#include <strsafe.h>

#define LOCAL_WRITE_VER L"2.03"

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
api_explorerfindfile *WASABI_API_EXPLORERFINDFILE = 0;
JSAPI2::api_security *AGAVE_API_JSAPI2_SECURITY = 0;
api_application *WASABI_API_APP=0;

HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

static HistoryAPIFactory historyAPIFactory;
static JSAPI2Factory jsapi2Factory;

static int Init();
static void Quit();
static INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

prefsDlgRecW preferences = {0};
wchar_t g_tableDir[MAX_PATH] = {0};

extern "C" winampMediaLibraryPlugin plugin =
{
    MLHDR_VER,
    "nullsoft(ml_history.dll)",
    Init,
    Quit,
    MessageProc,
    0,
    0,
    0,
};

// Delay load library control << begin >>
#include <delayimp.h>
#pragma comment(lib, "delayimp")

bool nde_error = false;

FARPROC WINAPI FailHook(unsigned dliNotify, PDelayLoadInfo  pdli) 
{
	nde_error = true;
	return 0;
}
/*
extern "C"
{
	PfnDliHook __pfnDliFailureHook2 = FailHook;
}
*/
// Delay load library control << end >>

int Init()
{
	InitializeCriticalSection(&g_db_cs);

	g_db = NULL;
	g_table = NULL;
	
	mediaLibrary.library  = plugin.hwndLibraryParent;
	mediaLibrary.winamp   = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;

	wchar_t configName[ MAX_PATH ] = { 0 };
	const wchar_t *dir = mediaLibrary.GetIniDirectoryW();

	if ( (INT_PTR)( dir ) < 65536 )
		return ML_INIT_FAILURE;


	PathCombineW( g_tableDir, dir, L"Plugins" );
	PathCombineW( configName, g_tableDir, L"gen_ml.ini" );

	g_config = new C_Config( configName );

	CreateDirectoryW( g_tableDir, NULL );
	PathCombineW( g_tableDir, g_tableDir, L"ml" );
	CreateDirectoryW( g_tableDir, NULL );

	// loader so that we can get the localisation service api for use

	waServiceFactory *sf = plugin.service->service_getServiceByGuid( languageApiGUID );
	if ( sf )
		WASABI_API_LNG = reinterpret_cast<api_language *>( sf->getInterface() );

	sf = plugin.service->service_getServiceByGuid( JSAPI2::api_securityGUID );
	if ( sf )
		AGAVE_API_JSAPI2_SECURITY = reinterpret_cast<JSAPI2::api_security *>( sf->getInterface() );
	
	sf = plugin.service->service_getServiceByGuid( applicationApiServiceGuid );
	if ( sf )
		WASABI_API_APP = reinterpret_cast<api_application *>( sf->getInterface() );

	sf = plugin.service->service_getServiceByGuid( ExplorerFindFileApiGUID );
	if ( sf )
		WASABI_API_EXPLORERFINDFILE = reinterpret_cast<api_explorerfindfile *>( sf->getInterface() );

	plugin.service->service_register( &historyAPIFactory );
	plugin.service->service_register( &jsapi2Factory );

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG( plugin.hDllInstance, MlHistoryLangGUID );

	g_context_menus2 = WASABI_API_LOADMENUW(IDR_CONTEXTMENUS);

	static wchar_t szDescription[256];
	StringCchPrintfW( szDescription, ARRAYSIZE( szDescription ), WASABI_API_LNGSTRINGW( IDS_NULLSOFT_HISTORY ), LOCAL_WRITE_VER );


	plugin.description = (char*)szDescription;

	static wchar_t preferencesName[64] = {0};
	preferences.hInst  = WASABI_API_LNG_HINST;
	preferences.dlgID  = IDD_PREFS;
	preferences.proc   = (void *)PrefsProc;
	preferences.name   = WASABI_API_LNGSTRINGW_BUF( IDS_HISTORY, preferencesName, 64 );
	preferences.where = 6; // media library

	SENDWAIPC( plugin.hwndWinampParent, IPC_ADD_PREFS_DLGW, &preferences );
	
	if ( !history_init() )
		return ML_INIT_FAILURE;

	return ML_INIT_SUCCESS;
}

void Quit()
{
	plugin.service->service_deregister(&historyAPIFactory);
	plugin.service->service_deregister(&jsapi2Factory);
	history_quit();
	delete g_config;
	DeleteCriticalSection(&g_db_cs);
	waServiceFactory *sf = plugin.service->service_getServiceByGuid(languageApiGUID);
	if (sf) sf->releaseInterface(WASABI_API_LNG);

	sf = plugin.service->service_getServiceByGuid(JSAPI2::api_securityGUID);
	if (sf) sf->releaseInterface(AGAVE_API_JSAPI2_SECURITY);
}

static INT_PTR History_OnContextMenu(INT_PTR param1, HWND hHost, POINTS pts)
{
	HNAVITEM hItem = (HNAVITEM)param1;
	HNAVITEM myItem = MLNavCtrl_FindItemById(plugin.hwndLibraryParent, ml_history_tree);
	if (hItem != myItem) 
		return FALSE;

	POINT pt;
	POINTSTOPOINT(pt, pts);
	if (-1 == pt.x || -1 == pt.y)
	{
		NAVITEMGETRECT itemRect;
		itemRect.fItem = FALSE;
		itemRect.hItem = hItem;
		if (MLNavItem_GetRect(plugin.hwndLibraryParent, &itemRect))
		{
			MapWindowPoints(hHost, HWND_DESKTOP, (POINT*)&itemRect.rc, 2);
			pt.x = itemRect.rc.left + 2;
			pt.y = itemRect.rc.top + 2;
		}
	}


	HMENU hMenu = WASABI_API_LOADMENUW(IDR_CONTEXTMENUS);
	HMENU subMenu = (NULL != hMenu) ? GetSubMenu(hMenu, 1) : NULL;
	if (NULL != subMenu)
	{

		INT r = Menu_TrackPopup(plugin.hwndLibraryParent, subMenu,
								TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | 
								TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, 
								pt.x, pt.y, hHost, NULL);

		switch(r)
		{
			case ID_NAVIGATION_PREFERENCES:
				SENDWAIPC(plugin.hwndWinampParent, IPC_OPENPREFSTOPAGE, &preferences);
				break;
			case ID_NAVIGATION_HELP:
				SENDWAIPC(plugin.hwndWinampParent, IPC_OPEN_URL, L"https://help.winamp.com/hc/articles/8105304048660-The-Winamp-Media-Library");
				break;
		}
	}

	if (NULL != hMenu)
		DestroyMenu(hMenu);
	
	return TRUE;
}

void History_StartTracking(const wchar_t *filename, bool resume)
{
	KillTimer(plugin.hwndWinampParent, 8082);
	if (!resume)
	{
		free(history_fn);
		history_fn = 0;
		history_fn_mode = 0;
		if (wcsstr(filename, L"://") && _wcsnicmp(filename, L"cda://", 6) && _wcsnicmp(filename, L"file://", 7))
		{
			history_fn_mode = 1;
		}
		history_fn = _wcsdup(filename);
	}
	
	int timer1 = -1, timer2 = -1, timer3 = -1;

	// wait for x seconds
	if(g_config->ReadInt(L"recent_wait_secs",0))
	{
		timer1 = g_config->ReadInt(L"recent_wait_secs_lim",5)*1000;
	}

	// wait for x percent of the song (approx to a second)
	if(g_config->ReadInt(L"recent_wait_percent",0))
	{
		basicFileInfoStructW bfiW = {0};
		bfiW.filename = history_fn;
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&bfiW, IPC_GET_BASIC_FILE_INFOW);
		if(bfiW.length > 0)
		{
			bfiW.length=bfiW.length*1000;
			timer2 = (bfiW.length*g_config->ReadInt(L"recent_wait_percent_lim",50))/100;
		}
	}

	// wait for the end of the item (within the last second of the track hopefully)
	if(g_config->ReadInt(L"recent_wait_end",0))
	{
		basicFileInfoStructW bfiW = {0};
		bfiW.filename = history_fn;
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&bfiW, IPC_GET_BASIC_FILE_INFOW);
		if(bfiW.length > 0)
		{
			timer3=(bfiW.length-1)*1000;
		}
	}

	// decide on which playback option will be the prefered duration (smallest wins)
	if(timer1 != -1 && timer2 != -1)
	{
		if(timer1 > timer2)
		{
			timer = timer2;
		}
		if(timer2 > timer1)
		{
			timer = timer1;
		}
	}
	else if(timer1 == -1 && timer2 != -1)
	{
		timer = timer2;
	}
	else if(timer2 == -1 && timer1 != -1)
	{
		timer = timer1;
	}

	// only track on end of file as very last method
	if((timer <= 0) && (timer3 > 0)){ timer = timer3; }
	
	// if no match or something went wrong then try to ensure the default timer value is used
	SetTimer(plugin.hwndWinampParent, 8082, ((timer > 0)? timer : 350), NULL);
}

INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	switch (message_type)
	{
		case ML_MSG_TREE_ONCREATEVIEW:     // param1 = param of tree item, param2 is HWND of parent. return HWND if it is us
			return (param1 == ml_history_tree) ?  (INT_PTR)onTreeViewSelectChange((HWND)param2) : 0; 
			
		case ML_MSG_CONFIG:
			mediaLibrary.GoToPreferences((int)preferences._id);
			return TRUE;

		case ML_MSG_VIEW_PLAY_ENQUEUE_CHANGE:
			enqueuedef = (int)param1;
			groupBtn = (int)param2;
			PostMessage(m_curview_hwnd, WM_APP + 104, param1, param2);
			return 0;

		case ML_MSG_NAVIGATION_CONTEXTMENU:
			return History_OnContextMenu(param1, (HWND)param2, MAKEPOINTS(param3));

		case ML_MSG_PLAYING_FILE:
			if (param1)
			{
				int resume = g_config->ReadInt(L"resumeplayback",0);
				if(resume)
				{
					int is_playing = (int)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_ISPLAYING);
					//int play_pos = SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETOUTPUTTIME);
					if(is_playing == 1/* && !(play_pos/1000 > 0)*/) //playing, look up last play offset and send seek message
					{
						wchar_t genre[256]={0};
						extendedFileInfoStructW efis={
							(wchar_t*)param1,
							L"genre",
							genre,
							ARRAYSIZE(genre),
						};
						SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efis,IPC_GET_EXTENDED_FILE_INFOW); 

						wchar_t ispodcast[8]={0};
						extendedFileInfoStructW efis1={
							(wchar_t*)param1,
							L"ispodcast",
							ispodcast,
							ARRAYSIZE(ispodcast),
						};
						SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efis1,IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE);

						if (resume == 2 || (ispodcast[0] && _wtoi(ispodcast) > 0) || (genre[0] && !_wcsicmp(genre, L"podcast")))
						{
							int offset = retrieve_offset((wchar_t*)param1);
							if (offset > 0 && (offset/1000 > 0)) PostMessage(plugin.hwndWinampParent,WM_WA_IPC,offset,IPC_JUMPTOTIME);
						}
					}
				}

				History_StartTracking((const wchar_t *)param1, false);
			}
			break;

			case ML_MSG_WRITE_CONFIG:
				if(param1)
				{
					closeDb();
					openDb();
				}
			break;
	}
	return 0;
}

extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &plugin;
}