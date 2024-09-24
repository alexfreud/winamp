#define PLUGIN_VER L"1.81"
#define FORCED_REBUILD_VERSION 1	// When changing this be sure to update 'forcedRebuildVersion' logic if no breaking changes are introduced in the new version

#include "api__ml_plg.h"
#include "../../General/gen_ml/ml.h"
#include "resource.h"
#include "main.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/ipc_pe.h"
#include "../nu/MediaLibraryInterface.h"
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"
#include <api/service/waservicefactory.h>
#include "playlist.h"
#include "../Agave/Language/api_language.h"
#include "resource.h"
//#include "mldbcallbacks.h"
#include "../../General/gen_ml/menufucker.h"
#include "../nu/ServiceWatcher.h"
//#include "api_playlist_generator.h"
#include "../nu/Singleton.h"
#include "PlaylistGeneratorApi.h"


//#include "wacmldbcallbacks.h"
#include <strsafe.h> // make sure this always gets #include'd last

// For the playlist generator API
static PlaylistGeneratorAPI playlistGeneratorAPI;
static SingletonServiceFactory<api_playlist_generator, PlaylistGeneratorAPI> playlistGeneratorFactory;


api_threadpool *WASABI_API_THREADPOOL = 0;
api_application *WASABI_API_APP = 0;
api_playlistmanager *AGAVE_API_PLAYLISTMGR = 0;
api_language *WASABI_API_LNG = 0;
api_config *AGAVE_API_CONFIG=0;
api_gracenote *AGAVE_API_GRACENOTE=0;
api_decodefile *AGAVE_API_DECODE=0;
api_metadata *AGAVE_API_METADATA=0;
api_mldb *AGAVE_API_MLDB = 0;
api_playlists *AGAVE_API_PLAYLISTS = 0;
api_stats *AGAVE_API_STATS = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
api_syscb *WASABI_API_SYSCB=0;

class MLDBWatcher : public ServiceWatcherSingle
{
public:
	void OnDeregister()
	{
		StopScan();
	}
};

static MLDBWatcher mldbWatcher;

extern winampMediaLibraryPlugin plugin;

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
void ServiceRelease(api_T *&api_t, GUID factoryGUID_t)
{
	if (plugin.service && api_t)
	{
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

static HWND winampPlaylist;

static int ML_IPC_MENUFUCKER_BUILD;
static int ML_IPC_MENUFUCKER_RESULT;

bool pluginEnabled = true;
int scanMode = 0;						// 0 = not inited, 1 = on start, 2 = on use
int plLengthType = PL_ITEMS;			// 0 = not inited, 1 = items (# of), 2 = length (minutes), 3 = size (kilobytes)
//int plLength = 20;					// Length of playlist, used for either # of items or for minutes target of the playlist
int plItems = 20;						// Number of desired items in the playlist
int plMinutes = 60;						// Number of desired minutes in the playlist
int plMegabytes = 650;					// Size of the desired playlist in kilobytes
int multipleArtists = FALSE;			// Generate tracks from the same artists in a single playlist
int multipleAlbums = FALSE;				// Generate tracks from the same albums in a single playlist
int useSeed = TRUE;						// Put the seed track into the generated playlist
int useMLQuery = FALSE;					// Use a custom query against the media library database to post process the results
wchar_t mlQuery[MAX_ML_QUERY_SIZE] = {0};	// Storage for the custom query
int forcedRebuildVersion = 0;			// Stores the ID of a forced rebuild when upgrading, 0 is never reset, 1 reset with ml_plg rewrite, 2+ and on are reserved for future scenarios

ThreadID *plg_thread=0;					// Thread ID for the single gracenote thread that we always make API calls on.
bool reset_db_flag = false;				// Flag that gets set whenever the DB needs to be reset before a scan.
bool run_full_scan_flag = true;			// Flag that gets set whenever there are media library changes so step 4 (pass 2) can be rerun for any changed files
volatile bool run_pass2_flag = false;	// Flag that gets set whenever there are media library changes so step 4 (pass 2) can be rerun for any changed files


void WriteIntToIni(const char *key, const int value)
{
	char buf[32] = {0};
	_itoa(value, buf, 10);
	WritePrivateProfileStringA("ml_plg", key, buf, mediaLibrary.GetWinampIni());
}

// BE CAREFULL! Using this could potentially internationalize floats on some versions of windows eg. '1,6' instead of '1.6'
void WriteFloatToIni(const char *key, const float value)
{
	char buf[32] = {0};
	StringCchPrintfA(buf, 32, "%.2f", value);
	WritePrivateProfileStringA("ml_plg", key, buf, mediaLibrary.GetWinampIni());
}

int Init()
{
	mediaLibrary.library = plugin.hwndLibraryParent;
	mediaLibrary.winamp = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;

	ServiceBuild(WASABI_API_SYSCB, syscbApiServiceGuid);
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(AGAVE_API_PLAYLISTMGR, api_playlistmanagerGUID);
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(AGAVE_API_DECODE, decodeFileGUID);
	ServiceBuild(AGAVE_API_GRACENOTE, gracenoteApiGUID);
	ServiceBuild(AGAVE_API_METADATA, api_metadataGUID);
	ServiceBuild(AGAVE_API_PLAYLISTS, api_playlistsGUID);
	ServiceBuild(AGAVE_API_STATS, AnonymousStatsGUID);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(WASABI_API_THREADPOOL, ThreadPoolGUID);

	playlistGeneratorFactory.Register(plugin.service, &playlistGeneratorAPI);

	if (WASABI_API_THREADPOOL)
		plg_thread = WASABI_API_THREADPOOL->ReserveThread(api_threadpool::FLAG_REQUIRE_COM_STA);

	if (!plg_thread)
		return ML_INIT_FAILURE; // if we weren't able to get a thread from the threadpool, bail out

	// no guarantee that AGAVE_API_MLDB will be available yet, so we'll start a watcher for it
	mldbWatcher.WatchWith(plugin.service);
	mldbWatcher.WatchFor(&AGAVE_API_MLDB, mldbApiGuid);
	WASABI_API_SYSCB->syscb_registerCallback(&mldbWatcher);

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,MlPlgLangGUID);

	ML_IPC_MENUFUCKER_BUILD = (int)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"menufucker_build", IPC_REGISTER_WINAMP_IPCMESSAGE);
	ML_IPC_MENUFUCKER_RESULT = (int)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"menufucker_result", IPC_REGISTER_WINAMP_IPCMESSAGE);

	winampPlaylist = (HWND)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,IPC_GETWND_PE,IPC_GETWND);

	static wchar_t szDescription[256];
	StringCchPrintf(szDescription, ARRAYSIZE(szDescription),
					WASABI_API_LNGSTRINGW(IDS_NULLSOFT_PLAYLIST_GENERATOR), PLUGIN_VER);
	plugin.description = (char*)szDescription;

	// Load variables from winamp.ini
	scanMode = GetPrivateProfileInt(L"ml_plg", L"scanmode", 0, mediaLibrary.GetWinampIniW());
	pluginEnabled = GetPrivateProfileInt(L"ml_plg", L"enable", 1, mediaLibrary.GetWinampIniW())!=0;
	multipleArtists = GetPrivateProfileInt(L"ml_plg", L"multipleArtists", multipleArtists, mediaLibrary.GetWinampIniW());
	multipleAlbums = GetPrivateProfileInt(L"ml_plg", L"multipleAlbums", multipleAlbums, mediaLibrary.GetWinampIniW());
	
	plLengthType = GetPrivateProfileInt(L"ml_plg", L"plLengthType", plLengthType, mediaLibrary.GetWinampIniW());
	plItems = GetPrivateProfileInt(L"ml_plg", L"plItems", plItems, mediaLibrary.GetWinampIniW());
	plMinutes = GetPrivateProfileInt(L"ml_plg", L"plMinutes", plMinutes, mediaLibrary.GetWinampIniW());
	plMegabytes = GetPrivateProfileInt(L"ml_plg", L"plMegabytes", plMegabytes, mediaLibrary.GetWinampIniW());
	useSeed = GetPrivateProfileInt(L"ml_plg", L"useSeed", useSeed, mediaLibrary.GetWinampIniW());
	useMLQuery = GetPrivateProfileInt(L"ml_plg", L"useMLQuery", useMLQuery, mediaLibrary.GetWinampIniW());
		
	char temp[MAX_ML_QUERY_SIZE] = {0};
	GetPrivateProfileStringA("ml_plg", "mlQuery", DEFAULT_ML_QUERY, temp, sizeof(temp), mediaLibrary.GetWinampIni());
	MultiByteToWideCharSZ(CP_UTF8, 0, temp, -1, mlQuery, sizeof(mlQuery)/sizeof(mlQuery[0]));
	//GetPrivateProfileStringA("ml_plg", "forcedRebuildVersion", "", temp, sizeof(temp), mediaLibrary.GetWinampIni());
	//forcedRebuildVersion = (float)atof(temp);
	forcedRebuildVersion = GetPrivateProfileIntA("ml_plg","forcedRebuildVersion", forcedRebuildVersion, mediaLibrary.GetWinampIni());
	
	// Here we check if the person is upgrading from the old ml_plg, if that value is less than our current version then we need to force a rebuild
	if (forcedRebuildVersion < FORCED_REBUILD_VERSION/*atof(PLUGIN_VER)*/)		// NOTE: Hard code this to a version if no breaking changes were made
	{																			// Otherwise there will be a forced reset every time version is incremented
		reset_db_flag = true;
		//ResetDBOnThread(true);
	}
	forcedRebuildVersion = FORCED_REBUILD_VERSION;								//(float)atof(PLUGIN_VER);

	if(scanMode == 1)									// If scanmode is set to rescan on winamp launch
		WASABI_API_CREATEDIALOGPARAMW(IDD_NAG, plugin.hwndWinampParent, BGScanProcedure, 1); // 1 means silent!

	return ML_INIT_SUCCESS;
}

void Quit()
{
	StopScan();
	HANDLE wait_event = CreateEvent(NULL, FALSE, FALSE, 0);
	WASABI_API_THREADPOOL->RunFunction(plg_thread, ShutdownScanner, (void *)wait_event, 0, api_threadpool::FLAG_REQUIRE_COM_STA);
	WaitForSingleObject(wait_event, INFINITE);
	
	mldbWatcher.StopWatching();
	WASABI_API_THREADPOOL->ReleaseThread(plg_thread);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(AGAVE_API_PLAYLISTMGR, api_playlistmanagerGUID);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(AGAVE_API_DECODE, decodeFileGUID);
	ServiceRelease(AGAVE_API_GRACENOTE, gracenoteApiGUID);
	ServiceRelease(AGAVE_API_METADATA, api_metadataGUID);
	ServiceRelease(AGAVE_API_MLDB, mldbApiGuid);
	ServiceRelease(AGAVE_API_PLAYLISTS, api_playlistsGUID);
	ServiceRelease(AGAVE_API_STATS, AnonymousStatsGUID);
	ServiceRelease(WASABI_API_THREADPOOL, ThreadPoolGUID);

	playlistGeneratorFactory.Deregister(plugin.service);
	//WASABI_API_SYSCB->syscb_deregisterCallback(&IDscanner);
}

static void FixAmps(wchar_t *str, size_t len)
{
	size_t realSize = 0;
	size_t extra = 0;
	wchar_t *itr = str;
	while (itr && *itr)
	{
		if (*itr == L'&')
			extra++;
		itr++;
		realSize++;
	}

	extra = min(len - (realSize + 1), extra);

	while (extra)
	{
		str[extra+realSize] = str[realSize];
		if (str[realSize] == L'&')
		{
			extra--;
			str[extra+realSize] = L'&';
		}
		realSize--;
	}
}

static void FixStrForMenu(wchar_t *str, size_t len)
{
	FixAmps(str,len);
}

// Triggered once some seed tracks are selected and added by the user
HWND SongsSelected(void)
{
	// I know this function is a one-liner but it may not be the case forever
	//WASABI_API_CREATEDIALOG(IDD_GENERATE, GetDesktopWindow(), GenerateProcedure);
	return WASABI_API_CREATEDIALOGW(IDD_GENERATE, plugin.hwndLibraryParent, GenerateProcedure);
}

// Display the warning message that the current file is not in ML so it cannot be used as a seed track
void NotInMLWarning(const wchar_t *filename)
{
	wchar_t message[MAX_PATH + 256] = {0};
	StringCchPrintfW(message, MAX_PATH + 256, WASABI_API_LNGSTRINGW(IDS_CANT_USE_SEED), filename);
	MessageBoxW(plugin.hwndLibraryParent, message, (LPWSTR)plugin.description, MB_OK| MB_ICONINFORMATION);
}

void MultipleInstancesWarning(void)
{
	MessageBoxW(plugin.hwndLibraryParent, WASABI_API_LNGSTRINGW(IDS_THERE_CAN_BE_ONLY_ONE), (LPWSTR)plugin.description, MB_OK | MB_ICONINFORMATION);
}

// Add seed tracks from main media library view
int AddSeedTracks(menufucker_t *mf)
{
	const int count = mf->extinf.mediaview.items->Size;
	int position = ListView_GetNextItem(mf->extinf.mediaview.list, -1, LVNI_SELECTED);		// Start the search from -1 so that we dont ignore the 0th selection

	while (position >= 0 && position < count)
	{
		wchar_t winamp_title[MAX_TITLE_SIZE] = {0};
		itemRecordW *item = &mf->extinf.mediaview.items->Items[position];
		if (item)
		{
			GetTitleFormattingML(item->filename, item, winamp_title, MAX_TITLE_SIZE);
			seedPlaylist.AppendWithInfo(item->filename, winamp_title, item->length * 1000, item->filesize * 1024);
			AGAVE_API_MLDB->FreeRecord(item);
		}
		position = ListView_GetNextItem(mf->extinf.mediaview.list, position, LVNI_SELECTED);
	}

	return true;
}

// Add seed tracks from a media library playlist
int AddSeedTracksMlPlaylist(menufucker_t *mf)
{
	int position = ListView_GetNextItem(mf->extinf.mlplaylist.list, -1, LVNI_SELECTED);		// Start the search from -1 so that we dont ignore the 0th selection

	while (position >= 0)
	{
		wchar_t filename[MAX_PATH] = {0};
		mf->extinf.mlplaylist.pl->GetItem(position, filename, MAX_PATH);
		itemRecordW *item = AGAVE_API_MLDB->GetFile(filename);
		if (item)
		{
			wchar_t winamp_title[MAX_TITLE_SIZE] = {0};
			GetTitleFormattingML(item->filename, item, winamp_title, MAX_TITLE_SIZE);
			seedPlaylist.AppendWithInfo(item->filename, winamp_title, item->length * 1000, item->filesize * 1024);
			AGAVE_API_MLDB->FreeRecord(item);
		}
		position = ListView_GetNextItem(mf->extinf.mlplaylist.list, position, LVNI_SELECTED);
	}

	return true;
}

// Add tracks from the winamp playlist
int AddSeedTracksPlaylist(menufucker_t *mf, int first_selection)
{
	bool isSuccess = true;

	int position = first_selection;
	while (position >= 0)
	{
		wchar_t winamp_title[MAX_TITLE_SIZE] = {0};
		fileinfoW inf={0};
		inf.index = position;

		SendMessage(winampPlaylist,WM_WA_IPC,IPC_PE_GETINDEXINFOW_INPROC,(LPARAM)&inf);
		itemRecordW *item = AGAVE_API_MLDB->GetFile(inf.file);

		if (item)
		{
			GetTitleFormattingML(inf.file, item, winamp_title, MAX_TITLE_SIZE);
			seedPlaylist.AppendWithInfo(item->filename, winamp_title, item->length * 1000, item->filesize * 1024);
			AGAVE_API_MLDB->FreeRecord(item);
		}
		else
		{
			NotInMLWarning(inf.file);			// Popup to warn that its not in the ML
		}

		position = (int)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, position, IPC_PLAYLIST_GET_NEXT_SELECTED);
	}

	if (seedPlaylist.GetNumItems() == 0)
		isSuccess = false;

	return isSuccess;
}

// Add a single seed track from the now playing song ticker
int AddSeedTrack(const wchar_t *filename)
{
	bool isSuccess = true;

	if (filename)
	{
		wchar_t winamp_title[MAX_TITLE_SIZE] = {0};
		itemRecordW *item = AGAVE_API_MLDB->GetFile(filename);
		if (item)
		{
			GetTitleFormattingML(filename, item, winamp_title, MAX_TITLE_SIZE);
			seedPlaylist.AppendWithInfo(filename, winamp_title, item->length * 1000, item->filesize * 1024);
			AGAVE_API_MLDB->FreeRecord(item);
		}
		else
		{
			NotInMLWarning(filename);			// Popup to warn that its not in the ML
		}
	}
	else
	{
		NotInMLWarning(filename);			// Popup to warn that its not in the ML
	}

	if (seedPlaylist.GetNumItems() == 0)
		isSuccess = false;

	return isSuccess;
}

void WriteSettingsToIni(HWND hwndDlg)
{
	/*char buf[32] = {0};
	
	StringCchPrintfA(buf, 32, "%d", plLengthType);
	WritePrivateProfileStringA("ml_plg","plLengthType",buf,mediaLibrary.GetWinampIni());*/

	WriteIntToIni("plLengthType", plLengthType);
	WriteIntToIni("plItems", plItems);
	WriteIntToIni("plMinutes", plMinutes);
	WriteIntToIni("plMegabytes", plMegabytes);
	WriteIntToIni("forcedRebuildVersion", forcedRebuildVersion);
	//WriteFloatToIni("forcedRebuildVersion", forcedRebuildVersion);

	WriteIntToIni("multipleArtists", multipleArtists);
	WriteIntToIni("multipleAlbums", multipleAlbums);
	WriteIntToIni("useSeed", useSeed);
	WriteIntToIni("useMLQuery", useMLQuery);
	
	/*multipleArtists = IsDlgButtonChecked(hwndDlg,IDC_CHECK_MULTIPLE_ARTISTS);
	WritePrivateProfileStringA("ml_plg","multipleArtists",multipleArtists?"1":"0",mediaLibrary.GetWinampIni());

	multipleAlbums = IsDlgButtonChecked(hwndDlg,IDC_CHECK_MULTIPLE_ALBUMS);
	WritePrivateProfileStringA("ml_plg","multipleAlbums",multipleAlbums?"1":"0",mediaLibrary.GetWinampIni());

	useSeed = IsDlgButtonChecked(hwndDlg,IDC_CHECK_USE_SEED);
	WritePrivateProfileStringA("ml_plg","useSeed",useSeed?"1":"0",mediaLibrary.GetWinampIni());

	useMLQuery = IsDlgButtonChecked(hwndDlg,IDC_CHECK_ML_QUERY);
	WritePrivateProfileStringA("ml_plg","useMLQuery", useMLQuery ? "1" : "0",mediaLibrary.GetWinampIni());*/

	//WritePrivateProfileStringW(L"ml_plg",L"mlQuery", mlQuery ,mediaLibrary.GetWinampIniW());
	WritePrivateProfileStringA("ml_plg", "mlQuery", AutoChar(mlQuery, CP_UTF8), mediaLibrary.GetWinampIni());
}

static bool IsInternetAvailable()
{
	return !!SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_INETAVAILABLE);
}

INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	static int mymenuid=0;

	if(message_type == ML_IPC_MENUFUCKER_BUILD && pluginEnabled && IsInternetAvailable())
	{
		menufucker_t* mf = (menufucker_t*)param1;
		wchar_t str[100] = {0}, str2[64] = {0};
		MENUITEMINFOW mii =
		{
			sizeof(MENUITEMINFOW),
			MIIM_TYPE | MIIM_ID,
			MFT_STRING,
			MFS_ENABLED,
			(UINT)mf->nextidx,
			0
		};

		mymenuid = mf->nextidx;
		mf->nextidx++;

		if(mf->type == MENU_MEDIAVIEW)
		{
			int n = ListView_GetSelectionMark(mf->extinf.mediaview.list);
			if(n == -1)
			{
				mymenuid=0;
				return 0;
			}

			itemRecordW * ice = &mf->extinf.mediaview.items->Items[n];
			if(!ice->title || !ice->title[0])
			{
				mymenuid=0;
				return 0;
			}

			int len = lstrlenW(ice->title);
			if (len > 39)
			{
				StringCchPrintfW(str2, 40, L"%.36s...", ice->title);
				StringCchPrintfW(str, 100, WASABI_API_LNGSTRINGW(IDS_PLAY_TRACKS_SIMILAR_TO), str2);
			}
			else
			{
				StringCchPrintfW(str, 100, WASABI_API_LNGSTRINGW(IDS_PLAY_TRACKS_SIMILAR_TO), ice->title);
			}

			FixStrForMenu(str,100);
			mii.dwTypeData = str;
			mii.cch = (UINT)wcslen(str);

			if(!InsertMenuItem(mf->menu,0xdeadbeef,FALSE,&mii))
			{
				InsertMenuItem(mf->menu,40012,FALSE,&mii);
				mii.wID = 0xdeadbeef;
				mii.fType = MFT_SEPARATOR;
				InsertMenuItem(mf->menu,40012,FALSE,&mii);
			}
		}
		else if(mf->type == MENU_MLPLAYLIST)
		{
			int n = ListView_GetSelectionMark(mf->extinf.mlplaylist.list);
			if(n == -1)
			{
				mymenuid=0;
				return 0;
			}
			wchar_t filename[MAX_PATH] = {0}, title[75] = {0};
			mf->extinf.mlplaylist.pl->GetItem(n,filename,MAX_PATH);
			AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"title", title, 75);
			if(!title[0])
			{
				mymenuid=0;
				return 0;
			}

			int len = lstrlenW(title);
			if (len > 39)
			{
				StringCchPrintfW(str2, 40, L"%.36s...", title);
				StringCchPrintfW(str, 100, WASABI_API_LNGSTRINGW(IDS_PLAY_TRACKS_SIMILAR_TO), str2);
			}
			else
			{
				StringCchPrintfW(str, 100, WASABI_API_LNGSTRINGW(IDS_PLAY_TRACKS_SIMILAR_TO), title);
			}

			FixStrForMenu(str,100);
			mii.dwTypeData = str;
			mii.cch = (UINT)wcslen(str);
			InsertMenuItem(mf->menu,3,TRUE,&mii);
			mii.wID = 0xdeadc0de;
			mii.fType = MFT_SEPARATOR;
			InsertMenuItem(mf->menu,3,TRUE,&mii);
		}
		else if(mf->type == MENU_PLAYLIST)
		{
			int n = (int)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,-1,IPC_PLAYLIST_GET_NEXT_SELECTED);
			if(n == -1) {
				mymenuid=0;
				return 0;
			}

			fileinfoW inf={0};
			inf.index = n;
			SendMessage(winampPlaylist,WM_WA_IPC,IPC_PE_GETINDEXINFOW_INPROC,(LPARAM)&inf);
			wchar_t title[75] = {0};
			AGAVE_API_METADATA->GetExtendedFileInfo(inf.file, L"title", title, 75);
			if(!title[0])
			{
				mymenuid=0;
				return 0;
			}


			int len = lstrlenW(title);
			if (len > 39)
			{
				StringCchPrintfW(str2, 40, L"%.36s...", title);
				StringCchPrintfW(str, 100, WASABI_API_LNGSTRINGW(IDS_PLAY_TRACKS_SIMILAR_TO), str2);
			}
			else
			{
				StringCchPrintfW(str, 100, WASABI_API_LNGSTRINGW(IDS_PLAY_TRACKS_SIMILAR_TO), title);
			}

			FixStrForMenu(str,100);
			mii.dwTypeData = str;
			mii.cch = (UINT)wcslen(str);

			InsertMenuItem(mf->menu,40470/*40208*/,FALSE,&mii);
			mii.wID = 0xdeadc0de;
			mii.fType = MFT_SEPARATOR;
			InsertMenuItem(mf->menu,40470/*40208*/,FALSE,&mii);
		}
		else if (mf->type == MENU_SONGTICKER)
		{
			wchar_t * file = (wchar_t*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GET_PLAYING_FILENAME);
			wchar_t title[75] = {0};
			AGAVE_API_METADATA->GetExtendedFileInfo(file, L"title", title, 75);
			if(!title[0])
			{
				mymenuid=0;
				return 0;
			}

			int len = lstrlenW(title);
			if (len > 39)
			{
				StringCchPrintfW(str2, 40, L"%.36s...", title);
				StringCchPrintfW(str, 100, WASABI_API_LNGSTRINGW(IDS_PLAY_TRACKS_SIMILAR_TO), str2);
			}
			else
			{
				StringCchPrintfW(str, 100, WASABI_API_LNGSTRINGW(IDS_PLAY_TRACKS_SIMILAR_TO), title);
			}

			FixStrForMenu(str,100);
			mii.dwTypeData = str;
			mii.cch = (UINT)wcslen(str);

			InsertMenuItem(mf->menu,0,TRUE,&mii);
		}
	}
	else if(message_type == ML_IPC_MENUFUCKER_RESULT && mymenuid != 0 && pluginEnabled)
	{
		menufucker_t* mf = (menufucker_t*)param1;
		DeleteMenu(mf->menu,mymenuid,MF_BYCOMMAND);
		if(mf->type == MENU_PLAYLIST || mf->type == MENU_MLPLAYLIST) DeleteMenu(mf->menu,0xdeadc0de,MF_BYCOMMAND);

		if(param2 == mymenuid && mymenuid != 0)
		{
			if(mf->type == MENU_MEDIAVIEW)			// Main Media Library View
			{
				int n = ListView_GetSelectionMark(mf->extinf.mediaview.list);
				if(n == -1)
				{
					mymenuid=0;
					return 0;
				}

				if (hwndDlgCurrent)					// Warn if trying to open two seperate playlist generators
					MultipleInstancesWarning();
				else
				{
					if (AddSeedTracks(mf))		// Make sure that we added the seed tracks successfully
						SongsSelected();
				}
			}
			else if(mf->type == MENU_MLPLAYLIST)	// Media library playlist view
			{
				// Check to see if anything is selected
				int n = ListView_GetSelectionMark(mf->extinf.mlplaylist.list);
				if(n == -1)
				{
					mymenuid=0;
					return 0;
				}

				if (hwndDlgCurrent)					// Warn if trying to open two seperate playlist generators
					MultipleInstancesWarning();
				else
				{
					if (AddSeedTracksMlPlaylist(mf))	// Make sure that we added the seed tracks successfully
						SongsSelected();
				}
			}
			else if(mf->type == MENU_PLAYLIST)		// Main window playlist
			{
				// Check to see if anything is selected
				int n = (int)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,-1,IPC_PLAYLIST_GET_NEXT_SELECTED);
				if(n == -1)
				{
					mymenuid=0;
					return 0;
				}

				if (hwndDlgCurrent)					// Warn if trying to open two seperate playlist generators
					MultipleInstancesWarning();
				else
				{
					if (AddSeedTracksPlaylist(mf, n))	// Make sure that we added the seed tracks successfully
						SongsSelected();
				}
			}
			else if(mf->type == MENU_SONGTICKER)	// Current playing track in the song ticker
			{
				wchar_t * file = (wchar_t*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GET_PLAYING_FILENAME);
				if (file)
				{
					if (hwndDlgCurrent)				// Warn if trying to open two seperate playlist generators
						MultipleInstancesWarning();
					else
					{
						if (AddSeedTrack(file))	// Make sure that we added the seed tracks successfully
							SongsSelected();
					}
				}
			}
		}
		mymenuid=0;
	}
	else switch (message_type)
	{
	case ML_MSG_CONFIG:
	{
		HWND parent = (HWND)param1;
		WASABI_API_DIALOGBOXW(IDD_PREFS, parent, PrefsProcedure);
		return TRUE;
	}
	break;
	}
	return 0;
}

extern "C" winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"nullsoft(ml_plg.dll)", // name filled in later
	Init,
	Quit,
	MessageProc,
	0,
	0,
	0,
};

extern "C"
{
	__declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
	{
		return &plugin;
	}

	__declspec( dllexport ) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
		// prompt to remove our settings with default as no (just incase)
		static wchar_t title[256];
		StringCchPrintf(title, ARRAYSIZE(title),
						WASABI_API_LNGSTRINGW(IDS_NULLSOFT_PLAYLIST_GENERATOR), PLUGIN_VER);

		if(MessageBoxW(hwndDlg,WASABI_API_LNGSTRINGW(IDS_DO_YOU_ALSO_WANT_TO_REMOVE_SETTINGS),
					   title,MB_YESNO|MB_DEFBUTTON2) == IDYES)
		{
			WritePrivateProfileStringW(L"ml_plg",0,0,mediaLibrary.GetWinampIniW());
		}

		// allow an on-the-fly removal (since we've got to be with a compatible client build)
		return ML_PLUGIN_UNINSTALL_NOW;
	}
};