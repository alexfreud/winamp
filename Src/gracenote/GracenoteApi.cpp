#include "GracenoteApi.h"
#include "api.h"
#include "../winamp/api_decodefile.h"
#include <bfc/error.h>
#include <limits.h>
#include <shlwapi.h>
#include <strsafe.h>

GracenoteApi::GracenoteApi()
{
	cddbInitialized = false;
	playlistInitialized = false;
	pCDDBControl=0;
}

GracenoteApi::~GracenoteApi()
{
}

void GracenoteApi::Close()
{
	if (pCDDBControl)
	{
		pCDDBControl->Shutdown();
		pCDDBControl->Release();
		pCDDBControl=0;
	}
}

static void SetProxy(const wchar_t *proxy, ICddbOptions *options)
{
	wchar_t user[1024]=L"";
	wchar_t pass[1024]=L"";
	wchar_t server[1024]=L"";
	int port=80;
	if (!_wcsnicmp(proxy,L"https:",6))
		port = 443;

	const wchar_t *skip = wcsstr(proxy, L"://");
	if (skip)
		proxy = skip+3;

	skip = wcsstr(proxy, L"@");
	if (skip)
	{
		const wchar_t *delimiter = wcsstr(proxy, L":");
		if (delimiter < skip) // make sure there's really a password (and we didn't end up finding the port number)
		{
			StringCchCopyNW(user, 1024, proxy, delimiter-proxy);
			StringCchCopyNW(pass, 1024, delimiter+1, skip-(delimiter+1));
			proxy=skip+1;
		}
		else
		{
			StringCchCopyNW(user, 1024, proxy, skip-proxy);
			proxy=skip+1;
		}
	}

	skip = wcsstr(proxy, L":"); // look for port
	if (skip)
	{
		StringCchCopyNW(server, 1024, proxy, skip-proxy);
		port = _wtoi(skip+1);
	}
	else
		StringCchCopyW(server, 1024, proxy);

	if (server[0])
		options->put_ProxyServer(server);
	if (port)
		options->put_ProxyServerPort(port);
	if (user[0])
		options->put_ProxyUserName(user);
	if (pass[0])
		options->put_ProxyPassword(pass);
}

// {C0A565DC-0CFE-405a-A27C-468B0C8A3A5C}
static const GUID internetConfigGroupGUID =
  { 0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c } };

ICDDBControl2 *GracenoteApi::GetCDDB()
{
	Nullsoft::Utility::AutoLock lock(cddbGuard);
	if (!cddbInitialized)
	{
		CoCreateInstance(__uuidof(CDDBNSWinampControl), 0, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&pCDDBControl);
		if (pCDDBControl == NULL)
			return 0;

#if 1 // TODO: benski> put back in once we can match winamp lang pack to a gracenote language ID
		// translate if necessary
		if (WASABI_API_LNG)
		{
			const wchar_t *langFolder = WASABI_API_LNG->GetLanguageFolder();
			if (langFolder)
			{
				WIN32_FIND_DATAW find;
				wchar_t mask[MAX_PATH] = {0}, maskLoc[16] = {0};

				// attempt to get the language code to help better guess the CDDB dll needed
				if(WASABI_API_LNG->GetLanguageIdentifier(LANG_IDENT_STR))
					StringCchPrintfW(maskLoc, 16, L"CddbLang%s.dll", WASABI_API_LNG->GetLanguageIdentifier(LANG_LANG_CODE));

				PathCombineW(mask, langFolder, maskLoc);
				HANDLE hFind = FindFirstFileW(mask, &find);

				// if the the guess provides nothing then scan for any valid dll (not ideal)
				if (hFind == INVALID_HANDLE_VALUE)
				{
					PathCombineW(mask, langFolder, L"CddbLang*.dll");
					hFind = FindFirstFileW(mask, &find);
				}

				if (hFind != INVALID_HANDLE_VALUE)
				{
					ICddbUIPtr ui;
					ui.CreateInstance(__uuidof(CddbNSWinampUI));
					if (ui)
					{
						long val = 0;
						wchar_t cddb_lang_fn[MAX_PATH] = {0};
						PathCombineW(cddb_lang_fn, langFolder, find.cFileName);
						// TODO: benski> gracenote wants a "language ID" but we don't have a good way of knowing it :(
						ui->SetUILanguage(0, cddb_lang_fn, &val);
						// TODO: benski> also need to set ICddbOptions language ID
					}
				}
				FindClose(hFind);
			}
		}
#endif

		// winamp browser id
		//HRESULT hr = pCDDBControl->SetClientInfo(L"7944448", L"F8DE207FBA826F136FF2C7EFE0AAB181", L"1", L"regstring");
		//wa5's id

		const wchar_t *appVersion = WASABI_API_APP->main_getVersionNumString();

		/* development client ID */
		//HRESULT hr = pCDDBControl->SetClientInfo(L"3714048", L"B49286AE14F73CCD73C23B371A56DB00", const_cast<BSTR>(appVersion), L"regstring");

		/* Beta Client ID */
		//HRESULT hr = pCDDBControl->SetClientInfo(L"8337664", L"A222F2FA8B3E047291DFDBF465FD3C95", const_cast<BSTR>(appVersion), L"regstring");

		/* Production Client ID */
		BSTR appVersionBSTR = SysAllocString(appVersion);
		HRESULT hr = pCDDBControl->SetClientInfo(L"4896768", L"C1519CAE91489E405BCA93531837F2BE", appVersionBSTR, L"regstring");
		SysFreeString(appVersionBSTR);

		if (FAILED(hr))
			return 0;

		long flags = CACHE_UPDATE_FUZZY | CACHE_DONT_WRITE_ANY | CACHE_NO_LOOKUP_MEDIA; //CACHE_SUBMIT_ALL | CACHE_SUBMIT_OFFLINE | CACHE_SUBMIT_NEW | CACHE_NO_LOOKUP_MEDIA;

		// set cache path for cddb control
		ICddbOptionsPtr pOptions;
		hr = pCDDBControl->GetOptions(&pOptions);
		if (SUCCEEDED(hr))
		{
			wchar_t dataPath[MAX_PATH] = {0};
			PathCombineW(dataPath, WASABI_API_APP->path_getUserSettingsPath(), L"Plugins");
			CreateDirectoryW(dataPath, 0);
			PathAppendW(dataPath, L"Gracenote");
			CreateDirectoryW(dataPath, 0);

			hr = pOptions->put_LocalCachePath(dataPath);

			// initial cache flags

			//BOOL bOnline = SendMessage(line.hMainWindow, WM_USER, 0, 242);
			//if (!bOnline)
			//flags |= CACHE_DONT_CONNECT;

			// other needed settings
			hr = pOptions->put_ProgressEvents(FALSE);
			//hr = pOptions->put_LocalCacheFlags(CACHE_DONT_CREATE | CACHE_UPDATE_FUZZY | CACHE_SUBMIT_ALL);
			hr = pOptions->put_LocalCacheFlags(flags);
			hr = pOptions->put_LocalCacheSize(128 * 1024); // 128 megabyte limit on local cache size
			hr = pOptions->put_LocalCacheTimeout(5 * 365); // 5 years (e.g. when Gracenote contract runs out)
			hr = pOptions->put_TestSubmitMode(FALSE);
			//hr = pOptions->put_TestSubmitMode(TRUE); //CT> for BETA cycle...

			// this is supposed to turn off the spinning logo in the upper-left-hand corner
			hr = pOptions->put_ResourceModule(-1);

			// get n set proxy settings
			const wchar_t *proxy = AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", L"");
			if (proxy && proxy[0])
				SetProxy(proxy, pOptions);

			// save settings
			hr = pCDDBControl->SetOptions(pOptions);
		}

		hr = pCDDBControl->Initialize(0/*(long)line.hMainWindow*/, (CDDBCacheFlags)flags);

		// checks for user registration
		long pVal=0;

		// this must be called when control is first initialized
		// this will load existing registration into control
		hr = pCDDBControl->IsRegistered(FALSE, &pVal);

		// if not reg'd, bring up reg UI (param1 = TRUE)
		if (!pVal)
		{
			// do headless registration
			ICddbUserInfoPtr pUser;

			hr = pCDDBControl->GetUserInfo(&pUser);
			if (pUser != NULL)
			{
				do
				{
					wchar_t strdata[129] = {0};
					size_t size = sizeof(strdata)/sizeof(*strdata);
					wchar_t *str = strdata;

					GUID uid = GUID_NULL;
					int x;
					unsigned char *p;
					CoCreateGuid(&uid);
					p = (unsigned char *) & uid;
					//lstrcpynW(str, L"WA2_", 129);
					StringCchCopyExW(str, size, L"WA5_", &str, &size, 0);
					for (x = 0; x < sizeof(uid); x ++)
					{
						StringCchPrintfExW(str, size, &str, &size, 0, L"%02X", p[x]);
						//wsprintfW(str + wcslen(str), L"%02X", p[x]);
					}

					// user name will have to be unique per install
					hr = pUser->put_UserHandle(strdata);
					hr = pUser->put_Password(strdata);

					hr = pCDDBControl->SetUserInfo(pUser);
				}
				while (hr == CDDBSVCHandleUsed);
			}

			// this is just to check again that the user is now registered
			hr = pCDDBControl->IsRegistered(FALSE, &pVal);
		}

		cddbInitialized = true;
	}

	if (pCDDBControl)
		pCDDBControl->AddRef();

	return pCDDBControl;
}

#if 0
/// This is the deprecated version of GetPlaylistManager that is no longer used without the MLDB manager
ICddbPlaylist25Mgr *GracenoteApi::GetPlaylistManager()
{
	ICddbPlaylist25Mgr *playlistMgr;

	ICDDBControl2 *cddb = GetCDDB();
	if (!cddb)
		return 0;

	CoCreateInstance(__uuidof(CddbNSWinampPlaylist2Mgr), 0, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&playlistMgr);
	if (playlistMgr)
	{
		playlistMgr->AddRef();
		wchar_t dataPath[MAX_PATH] = {0};
		PathCombineW(dataPath, WASABI_API_APP->path_getUserSettingsPath(), L"Plugins");
		CreateDirectoryW(dataPath, 0);
		PathAppendW(dataPath, L"Gracenote");
		CreateDirectoryW(dataPath, 0);

		if (SUCCEEDED(playlistMgr->Initialize(cddb, dataPath)))
		{
			playlistMgr->DownloadCorrelates(0);
			playlistInitialized = true;
		}
		else
		{
			playlistMgr->Release();
			playlistMgr=0;
		}
	}
	cddb->Release();
	return playlistMgr;
}
#endif

/// Caller is responsible for freeing the returned BSTR !!!
BSTR SetAndCreatePath(const wchar_t *node)
{
	wchar_t path_to_create[MAX_PATH] = {0};
	
	BSTR bPath = 0;
	
	PathCombineW(path_to_create, WASABI_API_APP->path_getUserSettingsPath(), L"Plugins");
	CreateDirectoryW(path_to_create, 0);
	PathAppendW(path_to_create, node);
	CreateDirectoryW(path_to_create, 0);

	bPath = SysAllocString(path_to_create);
	// modified path as return value
	return bPath;
}

/// This has superceded the old GetPlaylistManager
/// Returns both a playlist manager in 'playlistMgr' and an mldb manager in 'mldbMgr'
//int GracenoteApi::GetPlaylistManagerWithMLDBManager(ICddbPlaylist25Mgr **playlistMgr, ICddbMLDBManager **mldbMgr)
int GracenoteApi::GetPlaylistManager(ICddbPlaylist25Mgr **playlistMgr, ICddbMLDBManager **mldbMgr)
{
	ICddbPlaylist25Mgr *playlistMgrCreated;
	ICddbMLDBManager *mldbMgrCreated;

	ICDDBControl2 *cddb = GetCDDB();
	if (!cddb)
		return 0;

	// Create the mldb manager
	CoCreateInstance(__uuidof(CddbMLDBManager), 0, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&mldbMgrCreated);
	if (mldbMgrCreated)
	{
		//PL_MLDB_FLAGS_ZERO  0x00000000 Used for initialization. 
		//PL_MLDB_CHECK_BASE 0x00000001 Causes CheckDB to verify the consistency of the MLDB data file. 
		//PL_MLDB_CHECK_INDEX 0x00000002 Causes CheckDB to verify the consistency of the MLDB index file. 
		//PL_MLDB_CHECK_DEEP 0x00000004 Causes CheckDB to perform a thorough check of the MLDB file(s). 
		//PL_MLDB_CHECK_DEFAULT 0x00000007 Default operation of CheckDB. 
		//PL_MLDB_CLEAR_INIT_FLAG  0x00000010 Causes ModifyInitFlag to remove the file indicating Playlist is initialized. 
		//PL_MLDB_SET_INIT_FLAG 0x00000020 Causes ModifyInitFlag to create the file indicating Playlist is initialized. 
		//PL_MLDB_BACKUP_BASE 0x00000100 Causes BackupDB to create a backup copy of the MLDB data file. 
		//PL_MLDB_BACKUP_INDEX 0x00000200 Causes BackupDB to create a backup copy of the MLDB index file. 
		//PL_MLDB_RESTORE_BASE 0x00000400 Causes RestoreDB to restore the MLDB data file from the backup copy. 
		//PL_MLDB_RESTORE_INDEX 0x00000800 Causes RestoreDB to restore the MLDB index file from the backup copy. 
		//PL_MLDB_DELETE_INDEX 0x00001000 Causes DeleteDBFiles to remove the MLDB index file. 
		//PL_MLDB_DELETE_BASE 0x00002000 Causes DeleteDBFiles to remove the MLDB data file. 
		//PL_MLDB_DELETE_BACKUPS 0x00004000  Causes DeleteDBFiles to remove the MLDBbackup files. 
		//PL_MLDB_DELETE_OTHER 0x00008000 Causes DeleteDBFiles to remove the other (non-MLDB) files used by Playlist. 
		//PL_MLDB_AUTO_REINDEX  0x00010000 When specified in SetOptions, will cause theindex file to be automatically rebuilt atinitialization if it is corrupt. 
		//PL_MLDB_AUTO_BACKUP  0x00020000 When specified in SetOptions, will cause the MLDB files to be automatically backed up at shutdown (if they have been modified). PL_MLDB_AUTO_MANAGE_INIT_FLAG 0x00040000 When specified in SetOptions, will cause the “init file” to be managed automatically (created at initialization, deleted at shut down).
		//PL_MLDB_AUTO_CHECK_IF_INIT_SET 0x00080000 When specified in SetOptions, will cause the MLDB files to be check at initialization if the “init file” exists (meaning shut down wasn’t called). 
		//PL_MLDB_AUTO_CHECK_AT_INIT 0x00100000 When specified in SetOptions, will cause the MLDB files to be checked always at initialization. 
		//PL_MLDB_AUTO_DEFAULT 0x000C0000 The default automatic behavior if no flags are specified with SetOptions. 
		//PL_MLDB_DEVICE_MLDB_42 0x01000000 Enable Gracenote Device SDK 4.2 compatibility for MLDB, list, and correlates files
		
		//long autoFlags = PL_MLDB_AUTO_DEFAULT;
		//long autoFlags = PL_MLDB_AUTO_REINDEX | PL_MLDB_AUTO_BACKUP | PL_MLDB_AUTO_MANAGE_INIT_FLAG | PL_MLDB_AUTO_CHECK_IF_INIT_SET | PL_MLDB_AUTO_CHECK_AT_INIT;
		long autoFlags = PL_MLDB_AUTO_REINDEX | PL_MLDB_AUTO_BACKUP | PL_MLDB_AUTO_MANAGE_INIT_FLAG | PL_MLDB_AUTO_CHECK_IF_INIT_SET;
		BSTR bDataPath = SetAndCreatePath(L"Gracenote");
		BSTR bBackupPath = SetAndCreatePath(L"Gracenote/Backup");
		
		mldbMgrCreated->AddRef();
		mldbMgrCreated->SetOptions(autoFlags, bBackupPath);


		CoCreateInstance(__uuidof(CddbNSWinampPlaylist2Mgr), 0, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&playlistMgrCreated);
		if (playlistMgrCreated)
		{
			playlistMgrCreated->AddRef();

			

			mldbMgrCreated->Attach(playlistMgrCreated);		// Attach the MLDB manager to the playlistMgr

			if (SUCCEEDED(playlistMgrCreated->Initialize(cddb, bDataPath)))
			{
				playlistMgrCreated->DownloadCorrelates(0);
				playlistInitialized = true;
			}
			else
			{
				playlistMgrCreated->Release();
				playlistMgrCreated=0;
			}
		}

		SysFreeString(bDataPath);
		SysFreeString(bBackupPath);
	}
	cddb->Release();
	
	*mldbMgr = mldbMgrCreated;
	*playlistMgr = playlistMgrCreated;
	
	if (mldbMgr && playlistMgr)
		return NErr_Success;
	else
		return NErr_FailedCreate;
}

/// Dont really have to use this, get the MLDB manager when creating the playlist manager
ICddbMLDBManager *GracenoteApi::GetMLDBManager()
{
	ICddbMLDBManager *mldbMgr;

	ICDDBControl2 *cddb = GetCDDB();
	if (!cddb)
		return 0;

	CoCreateInstance(__uuidof(CddbMLDBManager), 0, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&mldbMgr);
	if (mldbMgr)
	{
		mldbMgr->AddRef();
	}
	cddb->Release();
	return mldbMgr;
}

HRESULT GracenoteApi::CreateFingerprint(ICDDBMusicIDManager *musicID, api_decodefile *decodeApi, ICddbFileInfo *info, const wchar_t *filename, long *killswitch)
{
	if (!musicID || !decodeApi)
		return E_FAIL;

	ICddbMusicIDFingerprinterPtr fingerprinter;
	musicID->CreateFingerprinter(NULL, &fingerprinter);

	AudioParameters parameters;
	parameters.bitsPerSample = 16;
	parameters.channels = 2;
	parameters.sampleRate = 44100;
	ifc_audiostream *decoder = decodeApi->OpenAudioBackground(filename, &parameters);
	if (decoder)
	{
		HRESULT hr = fingerprinter->BeginAudioStream((long)parameters.sampleRate, (long)parameters.bitsPerSample, (long)parameters.channels);
		char data[65536] = {0};
		size_t decodeSize;
		int decode_killswitch=0, decode_error;
		while (decodeSize = decoder->ReadAudio((void *)data, sizeof(data), &decode_killswitch, &decode_error))
		{
			if (decodeSize > LONG_MAX) // I _really_ doubt this is going to happen, but just in case, since we cast down to a long
				break;

			if (*killswitch)
				break;
			hr = fingerprinter->WriteAudioData(data, (long)decodeSize);
			if (hr == CDDBMusicID_FPAcquired)
				break;
		}
		ICddbMusicIDFingerprintPtr fingerprint;
		fingerprinter->EndAudioStream(&fingerprint);
		decodeApi->CloseAudio(decoder);

		hr=info->put_Fingerprint(fingerprint);
		return S_OK;
	}
	return E_FAIL;
}

ICDDBMusicIDManager3 *GracenoteApi::GetMusicID()
{
	ICDDBControl2 *cddb = GetCDDB();
	if (!cddb)
		return 0;
	ICDDBMusicIDManager3 *musicID;
	CoCreateInstance(__uuidof(CDDBNSWinampMusicIDManager), 0, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&musicID);
	if (musicID)
		musicID->Initialize(cddb);
	cddb->Release();
	return musicID;
}

#define CBCLASS GracenoteApi
START_DISPATCH;
CB(API_GRACENOTE_GETCDDB, GetCDDB)
CB(API_GRACENOTE_GETMUSICID, GetMusicID)
CB(API_GRACENOTE_GETPLAYLISTMGR, GetPlaylistManager)
//CB(API_GRACENOTE_GETPLAYLISTMGRWITHMLDBMGR, GetPlaylistManagerWithMLDBManager)
CB(API_GRACENOTE_GETMLDBMGR, GetMLDBManager)
CB(API_GRACENOTE_CREATEFINGERPRINT, CreateFingerprint)
END_DISPATCH;
#undef CBCLASS