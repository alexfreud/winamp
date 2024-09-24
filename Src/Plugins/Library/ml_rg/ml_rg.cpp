#include "Main.h"
#include "../nu/AutoWideFn.h"
#include "api__ml_rg.h"
//#include <api/service/waservicefactory.h>
#include "RGFactory.h"
#include "../playlist/ifc_playlistloadercallback.h"
#include <strsafe.h>

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

int uninstalling = 0;
RGFactory rgFactory;
static DWORD ml_rg_config_ipc;
static BOOL ml_rg_open_prefs;

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

static int Init();
static void Quit();
static INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

#define PLUG_VER L"1.29"
extern "C" winampMediaLibraryPlugin plugin =
{
    MLHDR_VER,
    "nullsoft(ml_rg.dll)",
    Init,
    Quit,
    PluginMessageProc,
    0,
    0,
    0,
};

api_decodefile      *AGAVE_API_DECODE          = 0;
api_application     *WASABI_API_APP            = 0;
api_playlistmanager *AGAVE_API_PLAYLISTMANAGER = 0;
api_stats           *AGAVE_API_STATS           = 0;

char *iniFile = 0;
int Init()
{
	waServiceFactory *sf = 0;


	ServiceBuild( AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID );
	ServiceBuild( AGAVE_API_DECODE,          decodeFileGUID );
	ServiceBuild( WASABI_API_APP,            applicationApiServiceGuid );
	ServiceBuild( AGAVE_API_STATS,           AnonymousStatsGUID );

	plugin.service->service_register( &rgFactory );

	// loader so that we can get the localisation service api for use
	ServiceBuild( WASABI_API_LNG, languageApiGUID );

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG( plugin.hDllInstance, MlReplayGainLangGUID );

	static wchar_t szDescription[ 256 ];
	StringCchPrintfW( szDescription, ARRAYSIZE( szDescription ), WASABI_API_LNGSTRINGW( IDS_NULLSOFT_REPLAY_GAIN_ANALYZER ), PLUG_VER );

	plugin.description = (char *)szDescription;

	ml_rg_config_ipc = (DWORD)SendMessageA( plugin.hwndWinampParent, WM_WA_IPC, ( WPARAM ) & "ml_rg_config", IPC_REGISTER_WINAMP_IPCMESSAGE );

	iniFile               = (char *)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETINIFILE );
	config_ask            = GetPrivateProfileIntA( "ml_rg", "config_ask", config_ask, iniFile );
	config_ask_each_album = GetPrivateProfileIntA( "ml_rg", "config_ask_each_album", config_ask_each_album, iniFile );


	return ML_INIT_SUCCESS;
}

void Quit()
{
	ServiceRelease(decodeFile, decodeFileGUID);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID);
	ServiceRelease(AGAVE_API_STATS, AnonymousStatsGUID);
	plugin.service->service_deregister(&rgFactory);
}

void WorkQueue::Add(const wchar_t *filename)
{
	wchar_t album[512] = L"";

	// if the user wants to ignore tracks already scanned, check and skip appropriately
	if (config_ignore_gained_album &&	GetFileInfo(filename, L"replaygain_album_gain", album, 256) && album[0] != 0)
		return; 


	GetFileInfo(filename, L"album", album, 256);
	if (album[0])
	{
		RGWorkFile workFile;
		lstrcpynW(workFile.filename, filename, MAX_PATH);
		albums[album].push_back(workFile);
	}
	else
	{
		RGWorkFile workFile;
		lstrcpynW(workFile.filename, filename, MAX_PATH);
		unclassified.push_back(workFile);
	}
	totalFiles++;
}

void WriteAlbum(WorkQueue::RGWorkQueue &workQueue)
{
	for (WorkQueue::RGWorkQueue::iterator itr = workQueue.begin();itr != workQueue.end();itr++)
	{
		if (itr->track_gain[0])
			SetFileInfo(itr->filename, L"replaygain_track_gain", itr->track_gain);

		if (itr->track_peak[0])
			SetFileInfo(itr->filename, L"replaygain_track_peak", itr->track_peak);

		if (itr->album_gain[0])
			SetFileInfo(itr->filename, L"replaygain_album_gain", itr->album_gain);

		if (itr->album_peak[0])
			SetFileInfo(itr->filename, L"replaygain_album_peak", itr->album_peak);

		WriteFileInfo();
		if (AGAVE_API_STATS)
			AGAVE_API_STATS->IncrementStat(api_stats::REPLAYGAIN_COUNT);

		TagUpdated(itr->filename);
	}
}

void WriteTracks(WorkQueue::RGWorkQueue &workQueue)
{
	for (WorkQueue::RGWorkQueue::iterator itr = workQueue.begin();itr != workQueue.end();itr++)
	{
		if (itr->track_gain[0])
			SetFileInfo(itr->filename, L"replaygain_track_gain", itr->track_gain);

		if (itr->track_peak[0])
			SetFileInfo(itr->filename, L"replaygain_track_peak", itr->track_peak);

		WriteFileInfo();
		if (AGAVE_API_STATS)
			AGAVE_API_STATS->IncrementStat(api_stats::REPLAYGAIN_COUNT);

		TagUpdated(itr->filename);
	}
}

void CopyAlbumData(WorkQueue::RGWorkQueue &workQueue, const wchar_t *album_gain, const wchar_t *album_peak)
{
	for (WorkQueue::RGWorkQueue::iterator itr = workQueue.begin();itr != workQueue.end();itr++)
	{
		if (itr->track_gain && itr->track_gain[0]) // if there's no track gain, it's because there was an error!
		{
			if (album_gain && album_gain[0])
				lstrcpynW(itr->album_gain, album_gain, 64);

			if (album_peak && album_peak[0])
				lstrcpynW(itr->album_peak, album_peak, 64);
		}
	}
}

void WorkQueue::Calculate(ProgressCallback *callback, int *killSwitch)
{
	void *context = CreateRG();
	StartRG(context);

	float albumPeak = 0;
	for (RGWorkQueue::iterator itr = unclassified.begin();itr != unclassified.end();itr++)
	{
		if (*killSwitch) {DestroyRG(context); return ;}
		CalculateRG(context, itr->filename, itr->track_gain, itr->track_peak, callback, killSwitch, albumPeak);
		callback->FileFinished();
	}
	if (*killSwitch) {DestroyRG(context); return ;}
	callback->AlbumFinished(&unclassified);

	for (AlbumMap::iterator mapItr = albums.begin();mapItr != albums.end();mapItr++)
	{
		albumPeak = 0;
		StartRG(context);
		for (RGWorkQueue::iterator itr = mapItr->second.begin();itr != mapItr->second.end();itr++)
		{
			if (*killSwitch) {DestroyRG(context); return ;}
			CalculateRG(context, itr->filename, itr->track_gain, itr->track_peak, callback, killSwitch, albumPeak);
			callback->FileFinished();
		}
		wchar_t album_gain[64] = L"", album_peak[64] = L"";
		CalculateAlbumRG(context, album_gain, album_peak, albumPeak);
		CopyAlbumData(mapItr->second, album_gain, album_peak);
		if (*killSwitch) {DestroyRG(context); return ;}
		callback->AlbumFinished(&(mapItr->second));
	}
	DestroyRG(context);
}

class WorkQueuePlaylistLoader : public ifc_playlistloadercallback
{
public:
	WorkQueuePlaylistLoader(WorkQueue *_queue);
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info);

protected:
	WorkQueue *queue;
	RECVS_DISPATCH;
};

WorkQueuePlaylistLoader::WorkQueuePlaylistLoader(WorkQueue *_queue)
{
	queue = _queue;
}

void WorkQueuePlaylistLoader::OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
{
	queue->Add(filename);
}

#define CBCLASS WorkQueuePlaylistLoader
START_DISPATCH;
VCB(IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile)
END_DISPATCH;
#undef CBCLASS

INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	if (message_type == ML_MSG_ONSENDTOBUILD)
	{
		if (param1 == ML_TYPE_ITEMRECORDLISTW || param1 == ML_TYPE_ITEMRECORDLIST ||
			param1 == ML_TYPE_FILENAMES || param1 == ML_TYPE_STREAMNAMES ||
			param1 == ML_TYPE_FILENAMESW || param1 == ML_TYPE_STREAMNAMESW ||
			(AGAVE_API_PLAYLISTMANAGER && (param1 == ML_TYPE_PLAYLIST || param1 == ML_TYPE_PLAYLISTS)))
		{
			wchar_t description[512] = {0};
			WASABI_API_LNGSTRINGW_BUF(IDS_CALCULATE_REPLAY_GAIN, description, 512);

			mlAddToSendToStructW s = {0};
			s.context = param2;
			s.desc = description;
			s.user32 = (INT_PTR)PluginMessageProc;
			SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&s, ML_IPC_ADDTOSENDTOW);
		}
	}
	else if (message_type == ML_MSG_ONSENDTOSELECT)
	{
		if (param3 != (INT_PTR)PluginMessageProc) return 0;

		INT_PTR type = param1;
		INT_PTR data = param2;

		if (data)
		{
			WorkQueue workQueue;
			if (type == ML_TYPE_ITEMRECORDLIST)
			{
				itemRecordList *p = (itemRecordList*)data;
				for (int x = 0; x < p->Size; x ++)
				{
					workQueue.Add(AutoWideFn(p->Items[x].filename));
				}

				DoProgress(workQueue);
				return 1;
			}
			else if (type == ML_TYPE_ITEMRECORDLISTW)
			{
				itemRecordListW *p = (itemRecordListW*)data;
				for (int x = 0; x < p->Size; x ++)
				{
					workQueue.Add(p->Items[x].filename);
				}

				DoProgress(workQueue);
				return 1;
			}
			else if (type == ML_TYPE_FILENAMES || type == ML_TYPE_STREAMNAMES)
			{
				char *p = (char*)data;
				while (p && *p)
				{
					workQueue.Add(AutoWideFn(p));
					p += strlen(p) + 1;
				}
				DoProgress(workQueue);
				return 1;
			}
			else if (type == ML_TYPE_FILENAMESW || type == ML_TYPE_STREAMNAMESW)
			{
				wchar_t *p = (wchar_t*)data;
				while (p && *p)
				{
					workQueue.Add(p);
					p += wcslen(p) + 1;
				}
				DoProgress(workQueue);
				return 1;
			}
			else if (type == ML_TYPE_PLAYLIST)
			{
				mlPlaylist *playlist = (mlPlaylist *)param2;
				WorkQueuePlaylistLoader loader(&workQueue);
				AGAVE_API_PLAYLISTMANAGER->Load(playlist->filename, &loader);
				DoProgress(workQueue);
				return 1;
			}
			else if (type == ML_TYPE_PLAYLISTS)
			{
				mlPlaylist **playlists = (mlPlaylist **)param2;
				WorkQueuePlaylistLoader loader(&workQueue);
				while (playlists && *playlists)
				{
					mlPlaylist *playlist = *playlists;
					AGAVE_API_PLAYLISTMANAGER->Load(playlist->filename, &loader);
					playlists++;
				}
				DoProgress(workQueue);
				return 1;
			}
		}
	}
	else if (message_type == ML_MSG_CONFIG)
	{
		ml_rg_open_prefs = TRUE;
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 42, IPC_OPENPREFSTOPAGE);
		ml_rg_open_prefs = FALSE;
		return 1;
	}
	// this will be sent if we've opened the playback->replay gain prefs page
	// so that we can enable the embedded controls and return the RGConfig proc
	else if (message_type == ml_rg_config_ipc)
	{
		// sanity check by winamp.exe to make sure that we're valid
		if(!param1)
		{
			return 1;
		}
		// return the config dialog proceedure
		else if(param1 == 1)
		{
			return (INT_PTR)RGConfig;
		}
		// queried when the playback prefs page is opened to see if we [ml_rg] caused it
		else if(param1 == 2)
		{
			if(ml_rg_open_prefs)
			{
				return TRUE;
			}
		}
	}
	return 0;
}

extern "C" {
	__declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
	{
		return &plugin;
	}

	__declspec( dllexport ) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
		uninstalling = 1;

		// prompt to remove our settings with default as no (just incase)
		if(MessageBoxW(hwndDlg,WASABI_API_LNGSTRINGW(IDS_DO_YOU_ALSO_WANT_TO_REMOVE_SETTINGS),
					   (wchar_t*)plugin.description,MB_YESNO|MB_DEFBUTTON2) == IDYES)
		{
			WritePrivateProfileStringA("ml_rg", 0, 0, iniFile);
		}

		// also attempt to remove the ReplayGainAnalysis.dll so everything is kept cleaner
		wchar_t path[MAX_PATH] = {0};
		PathCombineW(path, (wchar_t*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETSHAREDDLLDIRECTORYW), L"ReplayGainAnalysis.dll");

		// if we get a handle then try to lower the handle count so we can delete
		HINSTANCE rgLib = GetModuleHandleW(path);
		if(rgLib) {
			FreeLibrary(rgLib);
		}
		DeleteFileW(path);

		// allow an on-the-fly removal (since we've got to be with a compatible client build)
		return ML_PLUGIN_UNINSTALL_NOW;
	}
};