#include "playlist.h"
#include <shlwapi.h>
#include "main.h"
#include "api__ml_plg.h"
#include "../nu/MediaLibraryInterface.h"
#include "impl_playlist.h"
//#import	"../gracenote/CDDBControlWinamp.dll" no_namespace, named_guids, raw_interfaces_only
#include "../gracenote/cddbcontrolwinamp.tlh"
#include "../winamp/ipc_pe.h"
#include "resource.h"

#include <strsafe.h>

//extern Playlist currentPlaylist;
ICddbPlaylist25Mgr *playlistMgr;
ICddbMLDBManager *mldbMgr;

Playlist currentPlaylist;
Playlist seedPlaylist;

class PlaylistEventHandler : public DPlaylist2Events
{
	STDMETHODIMP STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObject)
	{
		if (!ppvObject)
			return E_POINTER;

		else if (IsEqualIID(riid, __uuidof(DPlaylist2Events)))
			*ppvObject = (DPlaylist2Events *)this;
		else if (IsEqualIID(riid, IID_IDispatch))
			*ppvObject = (IDispatch *)this;
		else if (IsEqualIID(riid, IID_IUnknown))
			*ppvObject = this;
		else
		{
			*ppvObject = NULL;
			return E_NOINTERFACE;
		}

		AddRef();
		return S_OK;
	}

	ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return 1;
	}

	ULONG STDMETHODCALLTYPE Release(void)
	{
		return 0;
	}

	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
	{
		switch (dispid)
		{
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;

		}
		return DISP_E_MEMBERNOTFOUND;
	}

	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
	{
		*rgdispid = DISPID_UNKNOWN;
		return DISP_E_UNKNOWNNAME;
	}

	HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(unsigned int FAR * pctinfo)
	{
		return E_NOTIMPL;
	}

};

static IConnectionPoint *GetConnectionPoint(IUnknown *punk, REFIID riid)
{
	if (!punk)
		return 0;

	IConnectionPointContainer *pcpc;
	IConnectionPoint *pcp = 0;

	HRESULT hr = punk->QueryInterface(IID_IConnectionPointContainer, (void **) & pcpc);
	if (SUCCEEDED(hr))
	{
		pcpc->FindConnectionPoint(riid, &pcp);
		pcpc->Release();
	}
	return pcp;
}


static PlaylistEventHandler events;
static DWORD m_dwCookie = 0;
//static DWORD m_dwCookie_MldbMgr = 0;
bool SetupPlaylistSDK()
{
	if (!playlistMgr)
	{
		//playlistMgr = AGAVE_API_GRACENOTE->GetPlaylistManager();
		//AGAVE_API_GRACENOTE->GetPlaylistManagerWithMLDBManager(&playlistMgr, &mldbMgr);
		AGAVE_API_GRACENOTE->GetPlaylistManager(&playlistMgr, &mldbMgr);

		IConnectionPoint *icp = GetConnectionPoint(playlistMgr, DIID_DPlaylist2Events);
		if (icp)
		{
			icp->Advise(static_cast<IDispatch *>(&events), &m_dwCookie);
			icp->Release();
		}
	}

	return !!playlistMgr;
}

void ShutdownPlaylistSDK()
{
	if (playlistMgr)
	{
		if (mldbMgr)
		{
			mldbMgr->Detach(playlistMgr);	// Detach the mldb manager from the playlist manager if it is not null
		}

		IConnectionPoint *icp = GetConnectionPoint(playlistMgr, DIID_DPlaylist2Events);
		if (icp)
		{
			icp->Unadvise(m_dwCookie);
			icp->Release();
		}

		if (mldbMgr)
			mldbMgr->Release();		// Release the mldb manager if its not null

		playlistMgr->Shutdown();
		playlistMgr->Release();
	}
	mldbMgr=0;
	playlistMgr=0;
}

// Caller must cleanup the BSTR
BSTR SetAndCreatePath(/*wchar_t *path_to_create,*/ const wchar_t *node)
{
	wchar_t path_to_create[MAX_PATH] = {0};
	
	BSTR bPath = 0;
	
	PathCombineW(path_to_create, WASABI_API_APP->path_getUserSettingsPath(), L"Plugins");
	CreateDirectoryW(path_to_create, 0);
	PathAppendW(path_to_create, node);
	CreateDirectoryW(path_to_create, 0);

	bPath = SysAllocString(path_to_create);
	return bPath;
	// modified path as return value
}

// IMPORTANT: Make sure to call this on the gracenote dedicated thread.
int RestoreGracenoteMLDB(void)
{
	long restoreFlags = PL_MLDB_RESTORE_BASE | PL_MLDB_RESTORE_INDEX;
	//wchar_t backupPath[MAX_PATH] = {0};
	BSTR bDataPath = SetAndCreatePath(GRACENOTE_DB_BASE_PATH);
	BSTR bBackupPath = SetAndCreatePath(GRACENOTE_DB_BACKUP_PATH);

	// Restore the db files.
	mldbMgr->RestoreDBFiles(restoreFlags, bDataPath, bBackupPath);

	SysFreeString(bDataPath);
	SysFreeString(bBackupPath);
	return NErr_Success;
}

// Backs up the gracenote MLDB so that it can be restored on corruption
// IMPORTANT: Make sure to call this on the gracenote dedicated thread.
int BackupGracenoteMLDB(void)
{
	long backupFlags = PL_MLDB_BACKUP_BASE | PL_MLDB_BACKUP_INDEX;
	//wchar_t backupPath[MAX_PATH] = {0};
	BSTR bDataPath = SetAndCreatePath(GRACENOTE_DB_BASE_PATH);
	BSTR bBackupPath = SetAndCreatePath(GRACENOTE_DB_BACKUP_PATH);

	// Backup the db files.
	mldbMgr->BackupDBFiles(backupFlags, bDataPath, bBackupPath);

	SysFreeString(bDataPath);
	SysFreeString(bBackupPath);
	return NErr_Success;
}

/*BOOL DeleteGracenoteFile(char *filename)
{
	BOOL result;
	char path[MAX_PATH] = {0};
	//PathCombineA(path,mediaLibrary.GetIniDirectory(),"Plugins\\Gracenote");
	PathCombineA(path,"C:\\Users\\bigg\\AppData\\Roaming\\Winamp\\","Plugins\\Gracenote");
	PathAppendA(path, filename);
	result = DeleteFileA(path);
	return result;
}*/

void CheckForResetError(HRESULT error)
{
	if (error != S_OK)
	{
		MessageBoxW(0, WASABI_API_LNGSTRINGW(IDS_ERROR_RESET), (LPWSTR)plugin.description, MB_OK| MB_ICONERROR);
	}
}

void CheckForShutdownError(HRESULT error)
{
	if (error != S_OK)
	{
		MessageBoxW(plugin.hwndWinampParent, WASABI_API_LNGSTRINGW(IDS_CANNOT_SHUT_DOWN), (LPWSTR)plugin.description, MB_OK| MB_ICONERROR);
	}
}

// Deprecated: Currently not used, remove at some point
/*
INT_PTR CALLBACK ResettingProcedure(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
			//ShowWindow(hwndDlg,SW_HIDE);
			ShowWindow(hwndDlg, SW_SHOW);
			return 0;
			break;
		case WM_QUIT:
			EndDialog(hwndDlg,0);
			return 0;
			break;
	}
	return 0;
}*/

// IMPORTANT: Make sure to call this on the gracenote dedicated thread.
int DeleteGracenoteMLDB(bool silent)
{
	long deleteFlags = PL_MLDB_DELETE_INDEX | PL_MLDB_DELETE_BASE | PL_MLDB_DELETE_OTHER | PL_MLDB_DELETE_BACKUPS;
	//long deleteFlags = PL_MLDB_DELETE_INDEX | PL_MLDB_DELETE_BASE | PL_MLDB_DELETE_OTHER; // | PL_MLDB_DELETE_BACKUPS;
	BSTR bDataPath = SetAndCreatePath(GRACENOTE_DB_BASE_PATH);
		
	HRESULT error = 0;

	if (playlistMgr)
		error = playlistMgr->Shutdown();

	if (!silent)
		CheckForShutdownError(error);

	// Spawn the working window
	//HWND hwndResetWorking = WASABI_API_CREATEDIALOG(IDD_NAG, plugin.hwndWinampParent, ResettingProcedure);
	if (mldbMgr)
		error = mldbMgr->DeleteDBFiles(deleteFlags, bDataPath);
	//SendMessage(hwndResetWorking, WM_QUIT, 0, 0);

	if (!silent)
		CheckForResetError(error);

	SysFreeString(bDataPath);
	return NErr_Success;		// NOT the HRESULT so that non zero values return as false
}

int InitializeMLDBManager(void)
{
	// Other initializations for the MLDB manager can go here
	///BackupGracenoteMLDB();

	return NErr_Success;
}

static void ConfigureGeneratorPrefs(ICddbPLMoreLikeThisCfg *config)
{
	// ToDo: (BigG) Consider using some of the different algorithms
	// GNPL_MORELIKETHIS_ALG_20 (Playlist SDK 2.0) 
	// GNPL_MORELIKETHIS_ALG_25 (Playlist SDK 2.5) 
	// GNPL_MORELIKETHIS_ALG_DSP_1 (Playlist SDK 2.6) 
	// GNPL_MORELIKETHIS_ALG_DSP_25 (Playlist SDK 2.6)

	config->put_Algorithm(GNPL_MORELIKETHIS_ALG_DEFAULT);
	if(!multipleAlbums) config->put_MaxPerAlbum(1);
	if(!multipleArtists) config->put_MaxPerArtist(1);
	//config->put_TrackLimit(plLength);
	config->put_TrackLimit(0);		// Dont put a limit on gracenote (return all tracks)
}

//void playPlaylist(Playlist &pl, bool enqueue=false, int startplaybackat=0/*-1 for don't start playback*/, const wchar_t *seedfn=NULL, int useSeed=FALSE)
void playPlaylist(Playlist &pl, bool enqueue=false, int startplaybackat=0/*-1 for don't start playback*/, int useSeed=FALSE)
{
	extern winampMediaLibraryPlugin plugin;

	const size_t number_of_seeds = seedPlaylist.GetNumItems();
	wchar_t seedFilename[MAX_PATH] = {0};
	seedFilename[0] = 0;

	if(!enqueue)
		mediaLibrary.ClearPlaylist();

	// Enqueue the seed tracks first
	if(useSeed)
	{
		for (size_t i = 0; i < number_of_seeds; i++)
		{
			seedPlaylist.GetItem(i, seedFilename, MAX_PATH);			// Get the playlist filename

			enqueueFileWithMetaStructW s={seedFilename,NULL,PathFindExtensionW(seedFilename),-1};
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
		}
	}

	size_t listLength = pl.GetNumItems();
	for(size_t i=0; i<listLength; i++)
	{
		wchar_t filename[MAX_PATH] = {0};
		pl.GetItem(i,filename,MAX_PATH);

		//if(seedfn && !_wcsicmp(seedfn,filename))			// Not really sure this is necessary... just making sure that the same file doesnt get added twice
		//	continue;
		enqueueFileWithMetaStructW s={filename,NULL,PathFindExtensionW(filename),-1};
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
	}

	if(!enqueue && startplaybackat != -1)
	{ //play item startplaybackat
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,startplaybackat,IPC_SETPLAYLISTPOS);
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0); //stop
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40045,0); //play
	}
}


// Tests wether an item can be added to a playlist and still stay under the current target
bool PlaylistIsFull(Playlist *playlist, /*uint64_t*/ unsigned int currentItemLength, /*uint64_t*/ unsigned int currentItemSize)
{
#define POWER_2_TO_20 1048576	// 1024 * 1024
	uint64_t measurement = 0;
	
	// See what type we care about items, size, or length
	switch (plLengthType)
	{
	case PL_ITEMS:				// Check for number of items
		{
			measurement = currentPlaylist.GetNumItems() + ( (useSeed) ? seedPlaylist.GetNumItems() : 0 );		// Add the data from the seed track if we are using it
		
		if ( (measurement + 1) > plItems )			// See if an extra item will put us over.
			return true;
		else
			return false;
		}
		break;

	case PL_MINUTES:			// Check for minutes used
		{
		measurement = currentPlaylist.GetPlaylistLengthMilliseconds() + ( (useSeed) ? seedPlaylist.GetPlaylistLengthMilliseconds() : 0 );

		if ( (measurement + (currentItemLength) ) > (plMinutes * 60 * 1000) )
			return true;
		else
			return false;
		}
		break;
	case PL_MEGABYTES:			// Check for megabytes used
		{
		measurement = currentPlaylist.GetPlaylistSizeBytes() + ( (useSeed) ? seedPlaylist.GetPlaylistSizeBytes() : 0 );

		if ( (measurement + (uint64_t)currentItemSize) > ((uint64_t)plMegabytes * POWER_2_TO_20) )
			return true;
		else
			return false;
		}
		break;
	}
	
	return true;
}

bool MatchesQuery(const wchar_t *filename, const wchar_t *user_query)
{
	// Get an item that mathces both the filename and the query
	itemRecordW *result = AGAVE_API_MLDB->GetFileIf(filename, user_query);
	
	if (result)
	{
		AGAVE_API_MLDB->FreeRecord(result);									// Clean up the records list since we are done using it		
		return true;
	}
	else
	{
		AGAVE_API_MLDB->FreeRecord(result);									// Clean up the records list since we are done using it		
		return false;
	}
}


// Callback for getting a tag for gracenote library items
static wchar_t * TitleTagFuncGracenote(const wchar_t * tag, void * p)
{ //return 0 if not found, -1 for empty tag
	//tagItem * s = (tagItem *)p;
	//wchar_t *filename = (wchar_t *)p;
	ICddbPL2Result *gracenoteResult = (ICddbPL2Result *)p;

	BSTR tag_data;
	
	if (!_wcsicmp(tag, L"artist"))			gracenoteResult->GetArtist(&tag_data)/*wsprintf(buf,L"%s",L"artist")*/;
	else if (!_wcsicmp(tag, L"album"))		gracenoteResult->GetAlbum(&tag_data);
	else if (!_wcsicmp(tag, L"title"))		gracenoteResult->GetTitle(&tag_data);
	else if (!_wcsicmp(tag, L"filename"))	gracenoteResult->GetFilename(&tag_data);
	else
		return 0;

	//else if (!_wcsicmp(tag, L"genre"))			-;
	//else if (!_wcsicmp(tag, L"year"))			-;
	//else if (!_wcsicmp(tag, L"tracknumber"))	-;
	//else if (!_wcsicmp(tag, L"discnumber"))		-;
	//else if (!_wcsicmp(tag, L"bitrate"))		-;
	
	//else if (!_wcsicmp(tag, L"albumartist"))	-;
	//else if (!_wcsicmp(tag, L"composer"))		-;
	//else if (!_wcsicmp(tag, L"publisher"))		-;

	return tag_data;
}

// Callback for getting a tag for media library items
static wchar_t * TitleTagFuncML(const wchar_t * tag, void * p)
{ //return 0 if not found, -1 for empty tag
	itemRecordW *mlResult = (itemRecordW *)p;

	if (!mlResult)
		return 0;		// Return 0 because we dont have this ml object
	
	wchar_t buf[128] = {0};

	wchar_t *tag_data = 0;

	if (!_wcsicmp(tag, L"artist"))				tag_data = mlResult->artist;
	else if (!_wcsicmp(tag, L"album"))			tag_data = mlResult->album;
	else if (!_wcsicmp(tag, L"title"))			tag_data = mlResult->title;
	else if (!_wcsicmp(tag, L"filename"))		tag_data = mlResult->filename;
	else if (!_wcsicmp(tag, L"genre"))			tag_data = mlResult->genre;
	else if (!_wcsicmp(tag, L"year"))			if (mlResult->year > 0) { StringCchPrintfW(buf, 128, L"%04d", mlResult->year); tag_data = buf; }
	else if (!_wcsicmp(tag, L"tracknumber") || !_wcsicmp(tag, L"track"))	if (mlResult->track > 0) { StringCchPrintfW(buf, 128, L"%04d", mlResult->track); tag_data = buf; }
	else if (!_wcsicmp(tag, L"discnumber"))		if (mlResult->disc > 0) { StringCchPrintfW(buf, 128, L"%d", mlResult->disc); tag_data = buf; }
	else if (!_wcsicmp(tag, L"bitrate"))		if (mlResult->bitrate > 0) { StringCchPrintfW(buf, 128, L"%d", mlResult->bitrate); tag_data = buf; }
	else if (!_wcsicmp(tag, L"albumartist"))	tag_data = mlResult->albumartist;
	else if (!_wcsicmp(tag, L"composer"))		tag_data = mlResult->composer;
	else if (!_wcsicmp(tag, L"publisher"))		tag_data = mlResult->publisher;
	else if (!_wcsicmp(tag, L"bpm"))			if (mlResult->bpm > 0) { StringCchPrintfW(buf, 128, L"%d", mlResult->bpm); tag_data = buf; }
	else if (!_wcsicmp(tag, L"comment"))		tag_data = mlResult->comment;
	else if (!_wcsicmp(tag, L"discs"))			if (mlResult->discs > 0) { StringCchPrintfW(buf, 128, L"%d", mlResult->discs); tag_data = buf; }
	else if (!_wcsicmp(tag, L"filesize"))		if (mlResult->filesize > 0) { StringCchPrintfW(buf, 128, L"%d", mlResult->filesize); tag_data = buf; }
	//else if (!_wcsicmp(tag, L"filetime"))		tag_data = mlResult->filetime;
	else if (!_wcsicmp(tag, L"length"))			if (mlResult->length > 0) { StringCchPrintfW(buf, 128, L"%d", mlResult->length); tag_data = buf; }
	else if (!_wcsicmp(tag, L"playcount"))		if (mlResult->playcount > 0) { StringCchPrintfW(buf, 128, L"%d", mlResult->playcount); tag_data = buf; }
	else if (!_wcsicmp(tag, L"rating"))			if (mlResult->rating > 0) { StringCchPrintfW(buf, 128, L"%d", mlResult->rating); tag_data = buf; }
	else
		return 0;

	return _wcsdup(tag_data);
}

// Callback to free a tag in gracenote library
static void TitleTagFreeFuncGracenote(wchar_t *tag_data, void *p)
{
	if(tag_data)
		SysFreeString(tag_data);
}

// Callback to free a tag in media library
static void TitleTagFreeFuncML(wchar_t *tag_data, void *p)
{
	if(tag_data)
		free(tag_data);
}

// Retreive the title formatting for gracenote library
void GetTitleFormattingGracenote(const wchar_t *filename, ICddbPL2Result * gracenoteResult, wchar_t * buf, int len)
{
	waFormatTitleExtended fmt={ filename, 1, NULL, (void *)gracenoteResult, buf, len, TitleTagFuncGracenote, TitleTagFreeFuncGracenote };
	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&fmt, IPC_FORMAT_TITLE_EXTENDED);
}

// Retreive the title formatting for media library
void GetTitleFormattingML(const wchar_t *filename, itemRecordW *mlResult, wchar_t * buf, int len)
{
	waFormatTitleExtended fmt={ filename, 1, NULL, (void *)mlResult, buf, len, TitleTagFuncML, TitleTagFreeFuncML };
	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&fmt, IPC_FORMAT_TITLE_EXTENDED);
}

static int MoreLikeTheseSongsFunc(HANDLE handle, void *user_data, intptr_t id)
{	
	Playlist *pl = (Playlist *)user_data;
	const int number_of_files = (int)pl->GetNumItems();

	isGenerating = true;

	//Sleep(501);						// Wait for the update timer to finish its cycles, This is EVIL, keep it removed...
	SetMarqueeProgress(true);			// Turn the marquee off because we are actually generating the tracks
	SetDlgItemText(hwndDlgCurrent, IDC_STATIC_PROGRESS_STATE, WASABI_API_LNGSTRINGW(IDS_GENERATING));
	//EnableWindow (GetDlgItem(hwndDlgCurrent, IDC_BUTTON_REGENERATE), FALSE );
	SetButtonsEnabledState(false);		// Disable the buttons once we start generating here

	if (SetupPlaylistSDK())
	{
		HRESULT hr;

		ICddbPLMoreLikeThisCfgPtr cfg;
		cfg.CreateInstance(CLSID_CddbPLMoreLikeThisCfg);
		ConfigureGeneratorPrefs(cfg);

		ICddbPL2ResultListPtr results;
		wchar_t plFilename[MAX_PATH] = {0};

		if (number_of_files == 1)			// Call the regular morelikethis function if there is only the standard seed track
		{
			pl->GetItem(0, plFilename, MAX_PATH);
			BSTR i_dunno = SysAllocString(plFilename);
			hr=playlistMgr->MoreLikeThisSong(i_dunno, cfg, &results);
			SysFreeString(i_dunno);
		}
		else if (number_of_files > 1)			// We have more than 1 seed track
		{
			// Create the variant of an array of filenames for MoreLikeTheseSongs
			VARIANT fileList;
			VariantInit((VARIANTARG *)&fileList);
			SAFEARRAY *psa = SafeArrayCreateVector (VT_BSTR, 0, number_of_files);
			BSTR *data;
			SafeArrayAccessData(psa, (void **)&data);
			for (size_t i=0;i!=number_of_files;i++)
			{
				pl->GetItem(i, plFilename, MAX_PATH);
				data[i] = SysAllocString(plFilename);
			}
			SafeArrayUnaccessData(psa);
			V_VT(&fileList) = VT_ARRAY|VT_BSTR;
			V_ARRAY(&fileList) = psa;
			
			
			hr=playlistMgr->MoreLikeTheseSongs(fileList, cfg, &results);
		}
		else		// We dont have any seed tracks (this should not happen because we shouldnt get this far.
		{
			return 1;		// Failure
		}

		long count=-1;
		if (results && SUCCEEDED(hr=results->get_Count(&count)) && count)
		{
			currentPlaylist.Clear();
			for (long i=0;i<count;i++)
			{
				ICddbPL2Result *result;
				if (SUCCEEDED(hr=results->GetResult(i+1, &result)))
				{
					BSTR filename = 0;
					BSTR title = 0;
					
					unsigned int length = -1;
					unsigned int size = 0;

					result->GetFilename(&filename);
					result->GetTitle(&title);
					result->GetTracklength(&length);	// Gracenote is returning seconds here
					result->GetFilesize(&size);			// Gracenote took the size as kilobytes but it is returning it to us as bytes
					
					length *= 1000;						// Multiply length by 1000 to turn it into milliseconds from seconds

					// Get the winamp user formatted title.
					wchar_t winamp_title[512] = {0};
					GetTitleFormattingGracenote(filename, result, winamp_title, 512);
					
					// Only check for the query if the user wants to apply one
					if ( useMLQuery == TRUE && MatchesQuery(filename, mlQuery ) == false )
					{
						SysFreeString(filename);
						SysFreeString(title);
						result->Release();
						continue;
					}

					// Lets check for the playlist limit to see if we should add this track
					if ( !PlaylistIsFull(&currentPlaylist, length, size) )
						currentPlaylist.AppendWithInfo(filename, winamp_title, length, size);
					// DONT break here if we are full, keep going as we may find something that will fit into the playlist eventually

					SysFreeString(filename);
					SysFreeString(title);
					result->Release();
				}
			}
			/*char cfg[1024 + 32] = {0};
			char *dir = (char*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETINIDIRECTORY);
			PathCombineA(cfg, dir, "Plugins\\gen_ml.ini");
			int en = GetPrivateProfileIntA("gen_ml_config","enqueuedef",0,cfg);*/

			PopulateResults(&currentPlaylist);
		}
		else /*if (count == 0)*/
		{
			PopulateResults(0);			// Call populate with an empty playlist
			CantPopulateResults();		// Display warning about not being able to generate any tracks
		}
	}

	isGenerating = false;

	return 0;
}

void MoreLikeTheseSongs(Playlist *pl)
{
	// Capture the stats, we dont care if its successful or not, we only care about the try
	if (AGAVE_API_STATS)
		AGAVE_API_STATS->IncrementStat(api_stats::PLG_COUNT);

	// Call the the mor like this fuction on the gracenote reserved thread
	WASABI_API_THREADPOOL->RunFunction(plg_thread, MoreLikeTheseSongsFunc, pl, 0, api_threadpool::FLAG_REQUIRE_COM_STA);
}
