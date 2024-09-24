#include "main.h"
#include "DownloadsDialog.h"
#include "DownloadsParse.h"
#include "Preferences.h"
#include "XMLWriter.h"
#include "..\..\General\gen_ml/ml.h"
#include "Defaults.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"

#include "api__ml_downloads.h"
#include "Downloaded.h"
#include "DownloadStatus.h"
#include <api/service/waServiceFactory.h>

#include <strsafe.h>

wchar_t *ml_cfg = 0,
		xmlFileName[1024] = {0};

ATOM VIEWPROP = 0;

api_downloadManager *WAC_API_DOWNLOADMANAGER = 0;
DownloadViewCallback *downloadViewCallback = 0;

static int Init();
static void Quit();
static INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

DWORD threadStorage=TLS_OUT_OF_INDEXES;
extern "C" winampMediaLibraryPlugin plugin =
    {
        MLHDR_VER,
        "nullsoft(ml_downloads.dll)",
        Init,
        Quit,
        MessageProc,
        0,
        0,
        0,
    };

//static prefsDlgRecW preferences;
//static wchar_t preferencesName[64] = {0};
int downloads_treeItem = 0,
	podcast_parent = 0,
	no_auto_hide = 0;
HANDLE hMainThread;

HCURSOR hDragNDropCursor;
int winampVersion = 0, dirty = 0;

api_application *applicationApi = NULL;
api_explorerfindfile *WASABI_API_EXPLORERFINDFILE = 0;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

int icons[4] = {-1, -1, -1, -1};
int activeIcon = 0;
int totalIcons = 4;
static Nullsoft::Utility::LockGuard navigationLock;

// used to get the downloads view parented to the podcast view
#define CSTR_INVARIANT MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)
#define NAVITEM_PREFIX L"podcast_svc_"
#define SERVICE_PODCAST 720

enum
{
    DOWNLOADS_TIMER_NAVNODE = 35784,
};

HNAVITEM Navigation_FindService(UINT serviceId)
{
	HWND hLibrary = plugin.hwndLibraryParent;
	INT cchPrefix = ARRAYSIZE(NAVITEM_PREFIX) - 1;

	WCHAR szBuffer[256] = {0};
	NAVITEM itemInfo = {0};
	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.mask = NIMF_TEXTINVARIANT;
	itemInfo.cchInvariantMax = ARRAYSIZE(szBuffer);
	itemInfo.pszInvariant = szBuffer;
	itemInfo.hItem = MLNavCtrl_GetFirst(hLibrary);

	while(NULL != itemInfo.hItem)
	{
		if (FALSE != MLNavItem_GetInfo(hLibrary, &itemInfo) && 
		CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, itemInfo.pszInvariant, cchPrefix, 
						NAVITEM_PREFIX, cchPrefix))
		{
			return itemInfo.hItem;
		}
		itemInfo.hItem = MLNavItem_GetNext(hLibrary, itemInfo.hItem);
	}
	return NULL;
}

void Navigation_Update_Icon(void)
{
	Nullsoft::Utility::AutoLock navlock(navigationLock);

	HNAVITEM hItem = NULL;
	if(downloads_treeItem)
	{
		hItem = MLNavCtrl_FindItemById(plugin.hwndLibraryParent,downloads_treeItem);
	}

	if (hItem)
	{
		NAVITEM item;
		item.cbSize = sizeof(NAVITEM);
		item.hItem = hItem;
		item.iSelectedImage = item.iImage = icons[activeIcon];
		activeIcon = (activeIcon + 1) % totalIcons;
		item.mask = NIMF_IMAGE | NIMF_IMAGESEL;
		MLNavItem_SetInfo(plugin.hwndLibraryParent, &item);
	}
}

BOOL Navigation_Update(void)
{
	int activeDownloads = 0;
	int historyDownloads = 0;
	{
		Nullsoft::Utility::AutoLock historylock(downloadedFiles.downloadedLock);
		historyDownloads = (int)downloadedFiles.downloadList.size();
	}

	{
		Nullsoft::Utility::AutoLock statuslock(downloadStatus.statusLock);		
		activeDownloads = (int)downloadStatus.downloads.size();
	}

	Nullsoft::Utility::AutoLock navlock(navigationLock);
	HNAVITEM hItem = NULL;
	if (downloads_treeItem)
	{
		hItem = MLNavCtrl_FindItemById(plugin.hwndLibraryParent,downloads_treeItem);
	}

	NAVINSERTSTRUCT nis = {0};
	nis.item.cbSize = sizeof(NAVITEM);
	nis.item.pszText = WASABI_API_LNGSTRINGW(IDS_DOWNLOADS);
	nis.item.iSelectedImage = nis.item.iImage = icons[0];

	WCHAR szName[256] = {0};
	if (activeDownloads && SUCCEEDED(StringCchPrintf(szName, ARRAYSIZE(szName), L"%s (%u)", WASABI_API_LNGSTRINGW(IDS_DOWNLOADS), activeDownloads)))
		nis.item.pszText = szName;

	if (activeDownloads > 0 || historyDownloads > 0 || no_auto_hide == 2)
	{
		if (!hItem)
		{
			nis.item.pszInvariant = NAVITEM_UNIQUESTR;
			nis.item.mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL;
			if(podcast_parent) nis.hParent = Navigation_FindService(SERVICE_PODCAST);
			NAVITEM nvItem = {sizeof(NAVITEM),0,NIMF_ITEMID,};

			nvItem.hItem = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis);

			MLNavItem_GetInfo(plugin.hwndLibraryParent, &nvItem);
			downloads_treeItem = nvItem.id;
		}
		else
		{
			nis.item.hItem = hItem;
			nis.item.mask = NIMF_TEXT;
			MLNavItem_SetInfo(plugin.hwndLibraryParent, &nis.item);
		}
	}
	else if (hItem)
	{
		nis.item.hItem = hItem;
		nis.item.mask = NIMF_TEXT | NIMF_IMAGE | NIMF_IMAGESEL;
		MLNavItem_SetInfo(plugin.hwndLibraryParent, &nis.item);
	}

	return TRUE;
}

void CALLBACK Downloads_Nav_Timer(HWND hwnd, UINT uMsg, UINT_PTR eventId, ULONG elapsed)
{
	switch (eventId)
	{
		case DOWNLOADS_TIMER_NAVNODE:
			if (downloadStatus.CurrentlyDownloading())
					Navigation_Update_Icon();
			break;
	}
}


int Init()
{
	hMainThread      = GetCurrentThread();
	hDragNDropCursor = LoadCursor( GetModuleHandle( L"gen_ml.dll" ), MAKEINTRESOURCE( ML_IDC_DRAGDROP ) );
	threadStorage    = TlsAlloc();

	if ( 0 == VIEWPROP )
	{
		VIEWPROP = GlobalAddAtom( L"Nullsoft_DownloadsView" );
		if ( VIEWPROP == 0 )
			return ML_INIT_FAILURE;
	}

	winampVersion = (int)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETVERSION );

	waServiceFactory *sf = plugin.service->service_getServiceByGuid( DownloadManagerGUID );
	if ( !sf ) // no sense in continuing
		return ML_INIT_FAILURE;
	else
		WAC_API_DOWNLOADMANAGER = reinterpret_cast<api_downloadManager *>( sf->getInterface() );

	// loader so that we can get the localisation service api for use
	sf = plugin.service->service_getServiceByGuid( languageApiGUID );
	if ( sf )
		WASABI_API_LNG = reinterpret_cast<api_language*>( sf->getInterface() );

	sf = plugin.service->service_getServiceByGuid( applicationApiServiceGuid );
	if ( sf )
		WASABI_API_APP = reinterpret_cast<api_application*>( sf->getInterface() );

	sf = plugin.service->service_getServiceByGuid( ExplorerFindFileApiGUID );
	if ( sf )
		WASABI_API_EXPLORERFINDFILE = reinterpret_cast<api_explorerfindfile*>( sf->getInterface() );

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG( plugin.hDllInstance, MlDownloadsLangGUID );

	static wchar_t szDescription[ 256 ];
	StringCchPrintf( szDescription, ARRAYSIZE( szDescription ), WASABI_API_LNGSTRINGW( IDS_PLUGIN_NAME ), PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR );
	plugin.description = (char*)szDescription;

	mediaLibrary.library  = plugin.hwndLibraryParent;
	mediaLibrary.winamp   = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;

	BuildDefaults( plugin.hwndWinampParent );

	mediaLibrary.BuildPath( L"Plugins\\ml\\downloads.xml", xmlFileName, 1024 );
	if ( PathFileExists( xmlFileName ) )
	{
		DownloadsParse downloader;
		downloader.DownloadFile( xmlFileName );
	}
	else
	{
		wchar_t xmlRssFileName[ 1024 ] = { 0 };
		mediaLibrary.BuildPath( L"Plugins\\ml\\feeds\\rss.xml", xmlRssFileName, 1024 );
		{
			DownloadsParse rssDownloader;
			rssDownloader.DownloadFile( xmlRssFileName );
		}
	}

	icons[ 0 ] = mediaLibrary.AddTreeImageBmp( IDB_TREEITEM_DOWNLOADS );
	icons[ 1 ] = mediaLibrary.AddTreeImageBmp( IDB_TREEITEM_DOWNLOADS1 );
	icons[ 2 ] = mediaLibrary.AddTreeImageBmp( IDB_TREEITEM_DOWNLOADS2 );
	icons[ 3 ] = mediaLibrary.AddTreeImageBmp( IDB_TREEITEM_DOWNLOADS3 );

	ml_cfg = (wchar_t *)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETMLINIFILEW );

	podcast_parent = ( !!GetPrivateProfileInt( L"gen_ml_config", L"podcast_parent", 0, ml_cfg ) );
	no_auto_hide   = GetPrivateProfileInt( L"gen_ml_config", L"no_auto_hide", 0, ml_cfg );

	Navigation_Update();

	downloadViewCallback = new DownloadViewCallback();
	WAC_API_DOWNLOADMANAGER->RegisterStatusCallback( downloadViewCallback );

	SetTimer( plugin.hwndLibraryParent, DOWNLOADS_TIMER_NAVNODE, 1000, Downloads_Nav_Timer );

	return ML_INIT_SUCCESS;
}

void Quit()
{
	KillTimer( plugin.hwndLibraryParent, DOWNLOADS_TIMER_NAVNODE );

	// If there are still files downloading, cancel download to remove incomplete downloaded files
	while ( downloadStatus.CurrentlyDownloading() ) 
    { 
           Nullsoft::Utility::AutoLock lock(downloadStatus.statusLock); 
           DownloadToken dltoken = downloadStatus.downloads.begin()->first; 
           WAC_API_DOWNLOADMANAGER->CancelDownload(dltoken); 
    }

	if (dirty)
	{
		SaveDownloads( xmlFileName, downloadedFiles );
		dirty = 0;
	}

	WAC_API_DOWNLOADMANAGER->UnregisterStatusCallback( downloadViewCallback );
	downloadViewCallback->Release();

	waServiceFactory *sf = plugin.service->service_getServiceByGuid( applicationApiServiceGuid );
	if ( sf != NULL )
		sf->releaseInterface(WASABI_API_APP);

	sf = plugin.service->service_getServiceByGuid(ExplorerFindFileApiGUID);
	if ( sf != NULL )
		sf->releaseInterface(WASABI_API_EXPLORERFINDFILE);

	sf = plugin.service->service_getServiceByGuid(DownloadManagerGUID);
	if ( sf != NULL )
		sf->releaseInterface(WAC_API_DOWNLOADMANAGER);

	if ( VIEWPROP != 0 )
	{
		GlobalDeleteAtom(VIEWPROP);
		VIEWPROP = 0;
	}
}

HWND CALLBACK DownloadDialog_Create( HWND hParent );

INT_PTR MessageProc( int msg, INT_PTR param1, INT_PTR param2, INT_PTR param3 )
{
	switch ( msg )
	{
		case ML_MSG_DOWNLOADS_VIEW_LOADED:
			return TRUE;

		case ML_MSG_DOWNLOADS_VIEW_POSITION:
		{
			// toggle the position of the node based on the preferences settings
			if ( param2 )
			{
				podcast_parent = (int)param1;
				if ( downloads_treeItem )
				{
					HNAVITEM hItem = MLNavCtrl_FindItemById( plugin.hwndLibraryParent, downloads_treeItem );
					if (hItem)
					{
						if ( MLNavCtrl_DeleteItem( plugin.hwndLibraryParent, hItem ) )
						{
							downloads_treeItem = 0;
						}
					}
				}
			}

			Navigation_Update();
			return 0;
		}

		case ML_MSG_TREE_ONCREATEVIEW:
			return ( param1 == downloads_treeItem ) ? (INT_PTR)DownloadDialog_Create( (HWND)param2 ) : 0;

		case ML_MSG_NO_CONFIG:
			return TRUE;

		case ML_MSG_NOTOKTOQUIT:
			if (downloadStatus.CurrentlyDownloading())
			{
				wchar_t titleStr[32] = {0};
				if ( MessageBox( plugin.hwndLibraryParent, WASABI_API_LNGSTRINGW( IDS_CANCEL_DOWNLOADS_AND_QUIT ), WASABI_API_LNGSTRINGW_BUF( IDS_CONFIRM_QUIT, titleStr, 32 ), MB_YESNO | MB_ICONQUESTION ) == IDNO )
					return TRUE;
			}
			return FALSE;

		case ML_MSG_WRITE_CONFIG:
			if ( dirty )
			{
				SaveDownloads( xmlFileName, downloadedFiles );
				dirty = 0;
			}
			break;

		case ML_MSG_VIEW_PLAY_ENQUEUE_CHANGE:
			enqueuedef = (int)param1;
			groupBtn   = (int)param2;
			PostMessage( downloads_window, WM_APP + 104, param1, param2 );
			return 0;
	}

	return 0;
}


extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &plugin;
}