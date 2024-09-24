#include "main.h"
#include "./fileview.h"
#include "./fileview_internal.h"
#include <vector>
#include <deque>
#include "../nu/threadname.h"
#include <api/service/waServiceFactory.h>

#include "../playlist/api_playlistmanager.h"
#include "../playlist/ifc_playlistloadercallback.h"
#include "../Agave/Metadata/api_metadata.h"

#include <shlwapi.h>
#include <strsafe.h>
#include <algorithm>

typedef struct _METASEARCHKEY
{
	INT pathId;
	LPCWSTR pszFileName;
} METASEARCHKEY;

typedef struct _PATHID
{
	INT		id;
	LPWSTR pszPath;
} PATHID;

typedef struct _METAINFO
{
	INT				pathId;
	LPWSTR			pszFileName;
	FILETIME			lastWriteTime;
	DWORD			fileSizeLow;
	DWORD			fileSizeHigh;
	BOOL			bBusy;
	FILEMETARECORD	*pFileMeta;
} METAINFO;

typedef std::vector<PATHID>		PATHLIST;
typedef std::vector<METAINFO*>	METARECORS;

typedef struct _METADB
{
	UINT		ref;
	PATHLIST	pPath;
	METARECORS	pRec;
	size_t		lastPathIndex;
	INT			maxPathId;
	HANDLE		hDiscoverThread;
	HANDLE		hDiscoverWake;
	HANDLE		hDiscoverKill;
} METADB;

#define METATHREAD_KILL		0
#define METATHREAD_WAKE		1

typedef void (CALLBACK *DISCOVERCALLBACK)(LPCWSTR /*pszFileName*/, ULONG_PTR /*param*/);

typedef struct _DISCOVERJOB
{
	METAINFO			*pMeta;
	HANDLE				hCaller;
	DISCOVERCALLBACK	fnCallback;
	ULONG_PTR			param;
	UINT				fileType;
	WCHAR				szFileName[2*MAX_PATH];
} DISCOVERJOB;

typedef std::deque<DISCOVERJOB*> DISCOVERDEQUE;

static DISCOVERDEQUE discoverJobs; 
static api_metadata *apiMetaData = NULL;
static api_playlistmanager *apiPlaylistManager = NULL;
static METADB metaDb = { 0, };
static CRITICAL_SECTION g_cs_discover;

static BOOL MetaDiscovery_InitializeThread(METADB *pMetaDb);
static void MetaDiscovery_KillThread(METADB *pMetaDb);
static BOOL MetaDiscovery_ScheduleJob(HANDLE hWakeEvent, LPCWSTR pszPath, METAINFO *pMeta, UINT fileType, DISCOVERCALLBACK fnCallback, ULONG_PTR param);

class PlMetaLoader : public ifc_playlistloadercallback
{
public:
	PlMetaLoader( PLAYLISTMETA *plm )
	{
		this->plm = plm; plm->nCount = 0; plm->nLength = 0;
	}
	~PlMetaLoader()
	{};


	void OnFile( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info )
	{
		if ( plm->nCount < sizeof( plm->szEntries ) / sizeof( plm->szEntries[ 0 ] ) )
		{
			PLENTRY *pe = &plm->szEntries[ plm->nCount ];

			if ( title && *title )
				pe->pszTitle = _wcsdup( title );
			else if ( filename && *filename )
			{
				pe->pszTitle = _wcsdup( filename );
			}
			else
				pe->pszTitle = NULL;

			pe->nLength = lengthInMS / 1000;
		}

		plm->nCount++;
		plm->nLength += lengthInMS;
	}

	void OnPlaylistInfo( const wchar_t *playlistName, size_t numEntries, ifc_plentryinfo *info )
	{
		plm->pszTitle = ( playlistName && *playlistName ) ? _wcsdup( playlistName ) : NULL;
	}

	const wchar_t *GetBasePath()
	{
		return L".";
	}

protected:
	RECVS_DISPATCH;

private:
	PLAYLISTMETA *plm;
};

#define CBCLASS PlMetaLoader
START_DISPATCH;
VCB( IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile )
VCB( IFC_PLAYLISTLOADERCALLBACK_ONPLAYLISTINFO, OnPlaylistInfo )
CB( IFC_PLAYLISTLOADERCALLBACK_GETBASEPATH, GetBasePath )
END_DISPATCH;


void FileViewMeta_InitializeStorage(HWND hView)
{
	if (0 == metaDb.ref)
	{
		InitializeCriticalSection(&g_cs_discover);
		if (WASABI_API_SVC)
		{
			waServiceFactory *factory;

			factory = WASABI_API_SVC->service_getServiceByGuid(api_metadataGUID);
			if (factory) apiMetaData = (api_metadata*) factory->getInterface();
			else apiMetaData = NULL;

			factory = WASABI_API_SVC->service_getServiceByGuid(api_playlistmanagerGUID);
			if (factory) apiPlaylistManager = (api_playlistmanager*) factory->getInterface();
			else apiPlaylistManager = NULL;
		}

		

	}
	metaDb.ref++;
}

static void FileViewMeta_FreeAudioMeta(AUDIOMETA *pMeta)
{
	if (pMeta->pszAlbum)	{ free(pMeta->pszAlbum); pMeta->pszAlbum = NULL; }
	if (pMeta->pszArtist) { free(pMeta->pszArtist); pMeta->pszArtist = NULL; }
	if (pMeta->pszTitle) { free(pMeta->pszTitle); pMeta->pszTitle = NULL; }
	if (pMeta->pszAlbumArtist) { free(pMeta->pszAlbumArtist); pMeta->pszAlbumArtist = NULL; }
	if (pMeta->pszComment) { free(pMeta->pszComment); pMeta->pszComment = NULL; }
	if (pMeta->pszComposer) { free(pMeta->pszComposer); pMeta->pszComposer = NULL; }
	if (pMeta->pszGenre) { free(pMeta->pszGenre); pMeta->pszGenre = NULL; }
	if (pMeta->pszPublisher) { free(pMeta->pszPublisher); pMeta->pszPublisher = NULL; }
}

static void FileViewMeta_FreeVideoMeta(VIDEOMETA *pMeta)
{
	if (pMeta->pszAlbum)	{ free(pMeta->pszAlbum); pMeta->pszAlbum = NULL; }
	if (pMeta->pszArtist) { free(pMeta->pszArtist); pMeta->pszArtist = NULL; }
	if (pMeta->pszTitle) { free(pMeta->pszTitle); pMeta->pszTitle = NULL; }
	if (pMeta->pszAlbumArtist) { free(pMeta->pszAlbumArtist); pMeta->pszAlbumArtist = NULL; }
	if (pMeta->pszComment) { free(pMeta->pszComment); pMeta->pszComment = NULL; }
	if (pMeta->pszComposer) { free(pMeta->pszComposer); pMeta->pszComposer = NULL; }
	if (pMeta->pszGenre) { free(pMeta->pszGenre); pMeta->pszGenre = NULL; }
	if (pMeta->pszPublisher) { free(pMeta->pszPublisher); pMeta->pszPublisher = NULL; }
}

static void FileViewMeta_FreePlaylistMeta(PLAYLISTMETA *pMeta)
{
	if (!pMeta) return;
	if (pMeta->pszTitle) { free(pMeta->pszTitle); pMeta->pszTitle = NULL; }
	for (int i = 0; i < sizeof(pMeta->szEntries)/sizeof(pMeta->szEntries[0]); i++) 
	{
		if (pMeta->szEntries[i].pszTitle) { free(pMeta->szEntries[i].pszTitle); pMeta->szEntries[i].pszTitle = NULL; }
	}
	if (pMeta->pszTitle) { free(pMeta->pszTitle); pMeta->pszTitle = NULL; }
}

static void FileViewMeta_FreeFileMeta(FILEMETARECORD *pFileMeta)
{
	if (pFileMeta)
	{
		switch(pFileMeta->type)
		{
			case METATYPE_AUDIO:
				FileViewMeta_FreeAudioMeta(&pFileMeta->audio); break;
			case METATYPE_VIDEO:
				FileViewMeta_FreeVideoMeta(&pFileMeta->video); break;
			case METATYPE_PLAYLIST:
				FileViewMeta_FreePlaylistMeta(&pFileMeta->playlist); break;
		}
		
		free(pFileMeta);
	}
}

static void FileViewMeta_FreeMetaInfo(METAINFO *pMeta)
{
	if (pMeta)
	{
		if (pMeta->pszFileName) { free(pMeta->pszFileName); pMeta->pszFileName = NULL; }
		FileViewMeta_FreeFileMeta(pMeta->pFileMeta);
	}
}

void FileViewMeta_TruncateQueue(size_t max)
{
	EnterCriticalSection(&g_cs_discover);
	while(discoverJobs.size() > max)
	{
		DISCOVERJOB *pdj = discoverJobs.front();
		if (pdj)
		{
			if (pdj->pMeta) 
			{
				ZeroMemory(&pdj->pMeta->lastWriteTime, sizeof(FILETIME));
				pdj->pMeta->fileSizeLow = 0;
				pdj->pMeta->bBusy = FALSE;
				if (pdj->hCaller) CloseHandle(pdj->hCaller);
			}
			free(pdj);
		}
		
		discoverJobs.pop_front();
	}
	LeaveCriticalSection(&g_cs_discover);
}

void FileViewMeta_ReleaseStorage(HWND hView)
{
	if (0 == metaDb.ref) return;
	
	metaDb.ref--;
	if (0 == metaDb.ref)
	{
		FileViewMeta_TruncateQueue(0);
		MetaDiscovery_KillThread(&metaDb);
		if (WASABI_API_SVC)
		{
			if(apiMetaData)
			{
				waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(api_metadataGUID);
				if (factory) factory->releaseInterface(apiMetaData);
			}
			if(apiPlaylistManager)
			{
				waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(api_playlistmanagerGUID);
				if (factory) factory->releaseInterface(apiPlaylistManager);
			}
		}

		while(metaDb.pPath.size() > 0)
		{
			if (metaDb.pPath.back().pszPath) free(metaDb.pPath.back().pszPath);
			metaDb.pPath.pop_back();
		}
		while(metaDb.pRec.size() > 0)
		{
			FileViewMeta_FreeMetaInfo(metaDb.pRec.back());
			metaDb.pRec.pop_back();
		}
		metaDb.maxPathId = 0;
		metaDb.lastPathIndex = 0;
		DeleteCriticalSection(&g_cs_discover);
	}
}

__inline static int __cdecl FileViewMeta_SortPathId(const void *elem1, const void *elem2)
{
	return (CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, 
			((PATHID*)elem1)->pszPath, -1, ((PATHID*)elem2)->pszPath, -1) - 2);
}

__inline static int __cdecl FileViewMeta_SearhPath(const void *elem1, const void *elem2)
{
	return (CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, 
			(LPCWSTR)elem1, -1, ((PATHID*)elem2)->pszPath, -1) - 2);
}

__inline static int __cdecl FileViewMeta_SortMetaInfo(const void *elem1, const void *elem2)
{
	METAINFO *pmi1 = ((METAINFO*)elem1);
	METAINFO *pmi2 = ((METAINFO*)elem2);
	if (pmi1->pathId != pmi2->pathId) return (pmi1->pathId - pmi2->pathId);
	return (CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, pmi1->pszFileName, -1, pmi2->pszFileName, -1) - 2);
}

__inline static int __cdecl FileViewMeta_SortMetaInfo_V2(const void* elem1, const void* elem2)
{
	return FileViewMeta_SortMetaInfo(elem1, elem2) < 0;
}
__inline static int __cdecl FileViewMeta_SearhMetaInfo(const void *elem1, const void *elem2)
{
	METASEARCHKEY *pKey = (METASEARCHKEY*)elem1;
	if (pKey->pathId != (((METAINFO*)elem2))->pathId) 
		return (pKey->pathId - (((METAINFO*)elem2))->pathId);
	return (CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, 
			pKey->pszFileName, -1, (((METAINFO*)elem2))->pszFileName, -1) - 2);
}

static INT FileViewMeta_GetPathId(LPCWSTR pszPath)
{
	if (!pszPath || L'\0' == pszPath) return 0;

	//PATHID *psr;
	PATHLIST::iterator psr;
	if (metaDb.lastPathIndex < metaDb.pPath.size() &&
		CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, pszPath, -1,
			metaDb.pPath.at(metaDb.lastPathIndex).pszPath, -1))
	{
		//psr = &metaDb.pPath.at(metaDb.lastPathIndex);
		psr = metaDb.pPath.begin() + metaDb.lastPathIndex;
	}
	else
	{
		//psr = (PATHID*)bsearch(pszPath, metaDb.pPath.begin(), metaDb.pPath.size(), sizeof(PATHID), FileViewMeta_SearhPath);
		psr = std::find_if(metaDb.pPath.begin(), metaDb.pPath.end(),
			[&](PATHID &path) -> bool
			{
				return FileViewMeta_SearhPath(pszPath, &path) == 0;
			}
		);
	}

	//if (!psr) 
	if(psr == metaDb.pPath.end())
	{
		PATHID pid;
		pid.id = metaDb.maxPathId+1;
		pid.pszPath = _wcsdup(pszPath);
		metaDb.pPath.push_back(pid);
		metaDb.maxPathId++;
		//psr = &pid;
		psr = metaDb.pPath.end() - 1;
		
		//qsort(metaDb.pPath.begin(), metaDb.pPath.size(), sizeof(PATHID), FileViewMeta_SortPathId);
		std::sort(metaDb.pPath.begin(), metaDb.pPath.end(),
			[&](const PATHID& lhs, const PATHID& rhs) -> bool
			{
				return FileViewMeta_SortPathId(&lhs, &rhs) == CSTR_LESS_THAN;
			}
		);
	}
	metaDb.lastPathIndex = (size_t)(ULONG_PTR)(psr - metaDb.pPath.begin());	
	return psr->id;
}

static METAINFO *FileViewMeta_GetMeta(LPCWSTR pszPath, LPCWSTR pszFileName, LPCWSTR pszExt, BOOL bCreateIfNotFound)
{
	WCHAR szBuffer[MAX_PATH] = {0};
	METASEARCHKEY key;
	key.pathId = FileViewMeta_GetPathId(pszPath);
	key.pszFileName = pszFileName;

	if (pszExt == (pszFileName + lstrlenW(pszFileName) + 1))
	{
		StringCchCopyW(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), pszFileName);
		if (pszExt) 
		{
			StringCchCatW(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), L".");
			StringCchCatW(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), pszExt);
		}
		key.pszFileName = szBuffer;
	}

	//METAINFO **pr = (METAINFO**)bsearch(&key, metaDb.pRec.at(0), metaDb.pRec.size(), sizeof(METAINFO*), FileViewMeta_SearhMetaInfo);
	auto it = std::find_if(metaDb.pRec.begin(), metaDb.pRec.end(),
		[&](const METAINFO* info) -> bool
		{
			return FileViewMeta_SearhMetaInfo(&key, info) == 0;
		}
	);
	if (it != metaDb.pRec.end())
	{
		return *it;
	}

	//if (pr) 
	//	return *pr;

	if (!bCreateIfNotFound) 
		return NULL;

	METAINFO *pInfo = (METAINFO*)calloc(1, sizeof(METAINFO));
	if (pInfo)
	{
		pInfo->pathId = key.pathId;
		pInfo->pszFileName = _wcsdup(key.pszFileName);
		metaDb.pRec.push_back(pInfo);
		//qsort(metaDb.pRec.first(), metaDb.pRec.size(), sizeof(METAINFO*), FileViewMeta_SortMetaInfo);
		std::sort(metaDb.pRec.begin(), metaDb.pRec.end(), FileViewMeta_SortMetaInfo_V2);
	}
	return pInfo;
}

// sets part and parts to -1 or 0 on fail/missing (e.g. parts will be -1 on "1", but 0 on "1/")
static void ParseIntSlashInt(wchar_t *string, int *part, int *parts)
{
	*part = -1;
	*parts = -1;

	if (string && string[0])
	{
		*part = _wtoi(string);
		while (*string && *string != '/')
		{
			string++;
		}
		if (*string == '/')
		{
			string++;
			*parts = _wtoi(string);
		}
	}
}

#define READFILEINFO(__fileName, __tag, __result, __pszBuffer, __cchBuffer)\
	(apiMetaData->GetExtendedFileInfo((__fileName), (__tag), (__pszBuffer), (__cchBuffer)) && L'\0' != *(__pszBuffer))

static void FileViewMeta_ReadAudioMeta(LPCWSTR pszFullPath, AUDIOMETA *pa)
{
	#define GETFILEINFO_STR(__tag, __result) { szBuffer[0] = L'\0';\
		if (READFILEINFO(pszFullPath, __tag, __result, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0])))\
		{(__result) = _wcsdup(szBuffer); }}
	#define GETFILEINFO_INT(__tag, __result) { szBuffer[0] = L'\0';\
		if (READFILEINFO(pszFullPath, __tag, __result, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0])))\
		{(__result) = _wtoi(szBuffer); }}
	#define GETFILEINFO_INTINT(__tag, __result1, __result2) { szBuffer[0] = L'\0';\
		if (READFILEINFO(pszFullPath, __tag, __result, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0])))\
		{ParseIntSlashInt(szBuffer, (__result1), (__result2)); }}

	if (AGAVE_API_MLDB)
	{
		itemRecordW *record = AGAVE_API_MLDB->GetFile(pszFullPath);
		if (record)
		{
			if (record->artist)		pa->pszArtist = _wcsdup(record->artist);
			if (record->album)		pa->pszAlbum = _wcsdup(record->album);
			if (record->title)		pa->pszTitle = _wcsdup(record->title);
			if (record->albumartist)	pa->pszAlbumArtist = _wcsdup(record->albumartist);
			if (record->comment)		pa->pszComment = _wcsdup(record->comment);
			if (record->composer)	pa->pszComposer = _wcsdup(record->composer);
			if (record->genre)		pa->pszGenre = _wcsdup(record->genre);
			if (record->publisher)	pa->pszPublisher = _wcsdup(record->publisher);
			pa->nLength		= record->length;
			pa->nBitrate	= record->bitrate;
			pa->nYear		= record->year;
			pa->nDiscNum		= record->disc;
			pa->nDiscCount	= record->discs;
			pa->nTrackNum	= record->track;
			pa->nTrackCount	= record->tracks;
			pa->nSource		= METADATA_SOURCE_MLDB;
			AGAVE_API_MLDB->FreeRecord(record);
			return;
		}
	}

	WCHAR szBuffer[2048] = {0};

	GETFILEINFO_STR(L"artist",		pa->pszArtist);
	GETFILEINFO_STR(L"album",		pa->pszAlbum);
	GETFILEINFO_STR(L"title",		pa->pszTitle);
	GETFILEINFO_STR(L"albumartist",	pa->pszAlbumArtist);
	GETFILEINFO_STR(L"comment",		pa->pszComment);
	GETFILEINFO_STR(L"composer",		pa->pszComposer);
	GETFILEINFO_STR(L"genre",		pa->pszGenre);
	GETFILEINFO_STR(L"publisher",	pa->pszPublisher);

	GETFILEINFO_INT(L"bitrate",	pa->nBitrate);
	GETFILEINFO_INT(L"year",	pa->nYear);
	GETFILEINFO_INT(L"length",	pa->nLength);
	pa->nLength = pa->nLength/1000;

	GETFILEINFO_INTINT(L"disc",	&pa->nDiscNum, &pa->nDiscCount);
	GETFILEINFO_INTINT(L"track",&pa->nTrackNum, &pa->nTrackCount);

	pa->nSource = METADATA_SOURCE_FILEINFO;
}

static void FileViewMeta_ReadPlaylistMeta(LPCWSTR pszFullPath, PLAYLISTMETA *plm)
{
	if (!apiPlaylistManager) return;
	PlMetaLoader plLoaderCb(plm);
	apiPlaylistManager->Load(pszFullPath, &plLoaderCb);
	plm->nLength = plm->nLength/1000;
}

static FILEMETARECORD* FileViewMeta_ReadFileMeta(LPCWSTR pszFullPath, UINT uType)
{
	FILEMETARECORD *pmr = (FILEMETARECORD*)calloc(1, sizeof(FILEMETARECORD));
	if (!pmr) return NULL;
	switch(uType)
	{
		case FVFT_AUDIO:
			pmr->type = METATYPE_AUDIO;
			FileViewMeta_ReadAudioMeta(pszFullPath, &pmr->audio);
			break;
		case FVFT_PLAYLIST:
			pmr->type = METATYPE_PLAYLIST;
			FileViewMeta_ReadPlaylistMeta(pszFullPath, &pmr->playlist);
			break;
	}

	return pmr;
}

FILEMETARECORD *FileViewMeta_GetFromCache(LPCWSTR pszPath, FILERECORD *pfr)
{
	if ((FVFT_AUDIO != pfr->fileType && FVFT_VIDEO != pfr->fileType) || !pszPath) return NULL;

	METAINFO *pMeta = FileViewMeta_GetMeta(pszPath, pfr->Info.cFileName, &pfr->Info.cFileName[pfr->extOffset], FALSE);
	if (pMeta && !pMeta->bBusy && pMeta->fileSizeLow == pfr->Info.nFileSizeLow && 
		pMeta->fileSizeHigh == pfr->Info.nFileSizeHigh && 0 == CompareFileTime(&pMeta->lastWriteTime, &pfr->Info.ftLastWriteTime))
	{
		return pMeta->pFileMeta;
	}
	return NULL;
}

BOOL FileViewMeta_Discover(LPCWSTR pszPath, FILERECORD *pfr, DISCOVERCALLBACK fnCallback, ULONG_PTR param, INT queueMax)
{
	if (!apiMetaData) return FALSE;

	if ((FVFT_AUDIO != pfr->fileType && 
		FVFT_VIDEO != pfr->fileType &&
		FVFT_PLAYLIST != pfr->fileType) || !pszPath) return FALSE;

	METAINFO *pMeta = FileViewMeta_GetMeta(pszPath, pfr->Info.cFileName, &pfr->Info.cFileName[pfr->extOffset], TRUE);
	if (!pMeta || pMeta->bBusy) return FALSE;
	if (0 == CompareFileTime(&pMeta->lastWriteTime, &pfr->Info.ftLastWriteTime) &&
		pMeta->fileSizeLow == pfr->Info.nFileSizeLow && pMeta->fileSizeHigh == pfr->Info.nFileSizeHigh)
	{
		pfr->pMeta = pMeta->pFileMeta;
		return TRUE;
	}

	FileViewMeta_FreeFileMeta(pMeta->pFileMeta);

	pMeta->lastWriteTime	= pfr->Info.ftLastWriteTime;
	pMeta->fileSizeLow		= pfr->Info.nFileSizeLow;
	pMeta->fileSizeHigh		= pfr->Info.nFileSizeHigh;
	pMeta->pFileMeta		= NULL;
	pMeta->bBusy			= TRUE;

	if (!fnCallback && !param)
	{
		WCHAR szFullPath[MAX_PATH] = {0};
		FileViewMeta_TruncateQueue(0);

		PathCombineW(szFullPath, pszPath, pMeta->pszFileName);
		pMeta->pFileMeta = FileViewMeta_ReadFileMeta(szFullPath, pfr->fileType);
		pMeta->bBusy = FALSE;
		pfr->pMeta = pMeta->pFileMeta;
		return TRUE;
	}

	if (queueMax > 0)
	{
		FileViewMeta_TruncateQueue(--queueMax);
	}
	if (!MetaDiscovery_InitializeThread(&metaDb) ||
		!MetaDiscovery_ScheduleJob(metaDb.hDiscoverWake, pszPath, pMeta, pfr->fileType, fnCallback, param))
	{
		ZeroMemory(&pMeta->lastWriteTime, sizeof(FILETIME));
		pMeta->fileSizeLow = 0;
		pMeta->bBusy = FALSE;
		return FALSE;
	}

	return FALSE;
}

static BOOL MetaDiscovery_ScheduleJob(HANDLE hWakeEvent, LPCWSTR pszPath, METAINFO *pMeta, UINT fileType, DISCOVERCALLBACK fnCallback, ULONG_PTR param)
{
	if (!pszPath || !pMeta  || !pMeta->pszFileName  || !fnCallback || !hWakeEvent)
	{
		if (pMeta)
		{
			ZeroMemory(&pMeta->lastWriteTime, sizeof(FILETIME));
			pMeta->fileSizeLow = 0;
			pMeta->bBusy = FALSE;
		}
		return FALSE;
	}

	DISCOVERJOB *pJob = (DISCOVERJOB*)calloc(1, sizeof(DISCOVERJOB));
	if (pJob)
	{
		pJob->pMeta = pMeta;

		HANDLE hp = GetCurrentProcess();
		if (DuplicateHandle(hp, GetCurrentThread(), hp, &pJob->hCaller, 0, FALSE, DUPLICATE_SAME_ACCESS))
		{
			pJob->fnCallback = fnCallback;
			pJob->param = param;
			pJob->fileType = fileType;
			PathCombineW(pJob->szFileName, pszPath, pMeta->pszFileName);
			EnterCriticalSection(&g_cs_discover);
			discoverJobs.push_front(pJob);
			LeaveCriticalSection(&g_cs_discover);
			if (SetEvent(hWakeEvent)) return TRUE;
		}
	}
	if (pJob)
	{
		if (pJob->hCaller) CloseHandle(pJob->hCaller);
		free(pJob);
	}
	if (pMeta) 
	{
		ZeroMemory(&pMeta->lastWriteTime, sizeof(FILETIME));
		pMeta->fileSizeLow = 0;
		pMeta->bBusy = FALSE;
	}

	return FALSE;
}

static void CALLBACK MetaDiscovery_ApcCallback(ULONG_PTR param)
{
	DISCOVERJOB *pJob = (DISCOVERJOB*)param;
	if (!pJob) return;
	if (pJob->pMeta) pJob->pMeta->bBusy = FALSE;
	if (pJob->fnCallback) pJob->fnCallback(pJob->szFileName, pJob->param);
	if (pJob->hCaller) CloseHandle(pJob->hCaller);
	free(pJob);
}

static void MetaDiscovery_ExecuteJob(DISCOVERJOB *pJob)
{
	pJob->pMeta->pFileMeta	= FileViewMeta_ReadFileMeta(pJob->szFileName, pJob->fileType);
	if (pJob->hCaller)
	{
		QueueUserAPC(MetaDiscovery_ApcCallback, pJob->hCaller, (ULONG_PTR)pJob);
		SleepEx(1, TRUE);
	}
	else MetaDiscovery_ApcCallback((ULONG_PTR)pJob);
}

static DWORD CALLBACK MetaDiscovery_ThreadProc(LPVOID param)
{
	METADB *pMetaDb = (METADB*)param;
	HANDLE hEvents[] = { pMetaDb->hDiscoverKill, pMetaDb->hDiscoverWake };
	SetThreadName(-1, "FileView meta discovery");
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
	SetEvent(hEvents[METATHREAD_WAKE]);

	for(;;)
	{
		switch (WaitForMultipleObjectsEx(2, hEvents, FALSE, INFINITE, TRUE))
		{
			case WAIT_OBJECT_0: // kill switch
				TRACE_FMT(TEXT("Kill Meta Discovery"));
				return 0;
			case WAIT_OBJECT_0+1: // job
				EnterCriticalSection(&g_cs_discover);
				if (discoverJobs.empty()) 
				{
					ResetEvent(hEvents[METATHREAD_WAKE]);
					LeaveCriticalSection(&g_cs_discover);
				}
				else
				{
					DISCOVERJOB *pj;
					pj = discoverJobs.front();
					discoverJobs.pop_front();
					LeaveCriticalSection(&g_cs_discover);
					MetaDiscovery_ExecuteJob(pj);
				}
				break;
		}
	}
	return 0;
}

static BOOL MetaDiscovery_InitializeThread(METADB *pMetaDb)
{
	if (!pMetaDb) return FALSE;
	if (pMetaDb->hDiscoverThread) return TRUE;

	DWORD threadId;
	if (!pMetaDb->hDiscoverWake) pMetaDb->hDiscoverWake = CreateEvent(0, TRUE, FALSE, 0);
	if (!pMetaDb->hDiscoverKill) pMetaDb->hDiscoverKill = CreateEvent(0, TRUE, FALSE, 0);
	if (pMetaDb->hDiscoverKill && pMetaDb->hDiscoverWake)
	{
		pMetaDb->hDiscoverThread = CreateThread(NULL, 0, MetaDiscovery_ThreadProc, (LPVOID)pMetaDb, 0, &threadId);
		if (pMetaDb->hDiscoverThread) 
		{
			WaitForSingleObject(pMetaDb->hDiscoverWake, INFINITE);
			ResetEvent(pMetaDb->hDiscoverWake);
			return TRUE;
		}
	}

	MetaDiscovery_KillThread(pMetaDb);
	return FALSE;
}

static void MetaDiscovery_KillThread(METADB *pMetaDb)
{
	if (!pMetaDb) return;

	if (pMetaDb->hDiscoverThread)
	{
		if (pMetaDb->hDiscoverKill)
		{
			SetEvent(metaDb.hDiscoverKill);
			WaitForSingleObject(metaDb.hDiscoverThread, INFINITE);
		}
		CloseHandle(pMetaDb->hDiscoverThread); 
		pMetaDb->hDiscoverThread = NULL;
	}
	if (pMetaDb->hDiscoverKill) { CloseHandle(pMetaDb->hDiscoverKill); pMetaDb->hDiscoverKill = NULL; }
	if (pMetaDb->hDiscoverWake) { CloseHandle(pMetaDb->hDiscoverWake); pMetaDb->hDiscoverWake = NULL; }
}

static BOOL FileViewMeta_GetAudioString(AUDIOMETA *pam, UINT uMetaField, LPCWSTR *ppszOut)
{
	switch(uMetaField)
	{
		case MF_ARTIST:			*ppszOut = pam->pszArtist; return TRUE;
		case MF_ALBUM:			*ppszOut = pam->pszAlbum; return TRUE;
		case MF_TITLE:			*ppszOut = pam->pszTitle; return TRUE;
		case MF_GENRE:			*ppszOut = pam->pszGenre; return TRUE;
		case MF_COMMENT:			*ppszOut = pam->pszComment; return TRUE;
		case MF_PUBLISHER:		*ppszOut = pam->pszPublisher; return TRUE;
		case MF_COMPOSER:		*ppszOut = pam->pszComposer; return TRUE;
		case MF_ALBUMARTIST:		*ppszOut = pam->pszAlbumArtist; return TRUE;
	}
	return FALSE;
}

static BOOL FileViewMeta_GetAudioInt(AUDIOMETA *pam, UINT uMetaField, INT *pOut)
{
	switch(uMetaField)
	{
		case MF_BITRATE:			*pOut = pam->nBitrate; return TRUE;
		case MF_DISCCOUNT:		*pOut = pam->nDiscCount; return TRUE;
		case MF_DISCNUM:			*pOut = pam->nDiscNum; return TRUE;
		case MF_LENGTH:			*pOut = pam->nLength; return TRUE;
		case MF_SOURCE:			*pOut = pam->nSource; return TRUE;
		case MF_TRACKCOUNT:		*pOut = pam->nTrackCount; return TRUE;
		case MF_TRACKNUM:		*pOut = pam->nTrackNum; return TRUE;
		case MF_YEAR:			*pOut = pam->nYear; return TRUE;
	}
	return FALSE;
}

BOOL FileViewMeta_GetString(FILEMETARECORD *pMeta, UINT uMetaField, LPCWSTR *ppszOut)
{
	if (!pMeta) return FALSE;

	switch(pMeta->type)
	{
		case METATYPE_AUDIO: return FileViewMeta_GetAudioString(&pMeta->audio, uMetaField, ppszOut);
	}
	return FALSE;
}

BOOL FileViewMeta_GetInt(FILEMETARECORD *pMeta, UINT uMetaField, INT *pOut)
{
	if (!pMeta) return FALSE;

	switch(pMeta->type)
	{
		case METATYPE_AUDIO: return FileViewMeta_GetAudioInt(&pMeta->audio, uMetaField, pOut);
	}
	return FALSE;
}