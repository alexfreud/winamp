#include "main.h"
#include <nde/nde_c.h>
#include "../nu/threadname.h"
#include "AlbumArtContainer.h"
#include <tataki/bitmap/bitmap.h>
#include "MD5.h"
#include "../replicant/nu/AutoLock.h"
#include <strsafe.h>
#include <map>
#include <deque>

static bool GetArtFromCache(const wchar_t *filename, int size, ARGB32 **bits);
static bool SetArtCache(const wchar_t *filename, int size, const ARGB32 *bits, uint8_t hash[16]);
static void CloseArtCache();
static HANDLE artWake=0;
static HANDLE mainThread=0;

struct CreateCacheParameters 
{
	AlbumArtContainer *container;
	int w;
	int h;
	SkinBitmap *cache;
	AlbumArtContainer::CacheStatus status;
};

static std::deque<CreateCacheParameters*> artQueue;
static nu::LockGuard queueGuard;

static void Adjust(int bmpw, int bmph, int &x, int &y, int &w, int &h)
{
	// maintain 'square' stretching
	double aspX = (double)(w)/(double)bmpw;
	double aspY = (double)(h)/(double)bmph;
	double asp = min(aspX, aspY);
	int newW = (int)(bmpw*asp);
	int newH = (int)(bmph*asp);
	x = (w - newW)/2;
	y = (h - newH)/2;
	w = newW;
	h = newH;
}

static void CALLBACK CreateCacheCallbackAPC(ULONG_PTR parameter)
{
	CreateCacheParameters *parameters = (CreateCacheParameters *)parameter;
	parameters->container->SetCache(parameters->cache, parameters->status);
	parameters->container->Release();
	WASABI_API_MEMMGR->sysFree(parameters);
}

static void CreateCache(CreateCacheParameters *parameters)
{
	// let's hope this doesn't overflow the stack
	const wchar_t *filename = parameters->container->filename;

	if ((unsigned int)(ULONG_PTR)filename < 65536)
	{
		parameters->cache = 0;
		parameters->status = AlbumArtContainer::CACHE_NOTFOUND;
		QueueUserAPC(CreateCacheCallbackAPC, mainThread, (ULONG_PTR)parameters);
		return;
	}

	int w = parameters->w, h = parameters->h;

	ARGB32 *cacheBits = 0;
	if (GetArtFromCache(filename, w, &cacheBits))
	{
		SkinBitmap *cache = new SkinBitmap(cacheBits, w, h, true);
		parameters->cache = cache;
		parameters->status = AlbumArtContainer::CACHE_CACHED;
	}
	else
	{
		int artsize = w;
		int bmp_w = 0, bmp_h = 0;
		ARGB32 *bits = 0;

		if (AGAVE_API_ALBUMART && AGAVE_API_ALBUMART->GetAlbumArt(filename, L"Cover", &bmp_w, &bmp_h, &bits) == ALBUMART_SUCCESS)
		{
			uint8_t hash[16] = {0};
			MD5_CTX hashCtx;
			MD5Init(&hashCtx);
			MD5Update(&hashCtx, (uint8_t *)bits, bmp_w*bmp_h*sizeof(ARGB32));
			MD5Final(hash, &hashCtx);

			BltCanvas canvas(w,h);
			HQSkinBitmap temp(bits, bmp_w,bmp_h); // wrap into a SkinBitmap (no copying involved)
			int x = 0, y = 0;
			Adjust(bmp_w, bmp_h, x,y,w,h);
			temp.stretch(&canvas,x,y,w,h);
			SkinBitmap *cache = new SkinBitmap(&canvas);
			parameters->cache = cache;
			SetArtCache(filename, artsize, (ARGB32 *)canvas.getBits(), hash);
			WASABI_API_MEMMGR->sysFree(bits);
			parameters->status = AlbumArtContainer::CACHE_CACHED;
		}
		else
		{
			parameters->cache = 0;
			parameters->status = AlbumArtContainer::CACHE_NOTFOUND;
		}
	}
	QueueUserAPC(CreateCacheCallbackAPC, mainThread, (ULONG_PTR)parameters);
	return;
}

static int ArtLoadThread(HANDLE handle, void *user_data, intptr_t id)
{
	// TODO?	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);

	queueGuard.Lock();
	if (artQueue.empty())
	{
		queueGuard.Unlock();
		return 0;
	}

	CreateCacheParameters *parameters = artQueue.front();
	artQueue.pop_front();
	queueGuard.Unlock();
	CreateCache(parameters);
	return 0;
}

enum
{
	ARTHASH_FILENAME = 0,
	ARTHASH_HASH = 1,

	ARTTABLE_HASH = 0,
	ARTTABLE_ARGB32 = 1,
};

typedef std::map<int, nde_table_t> ArtCache;
static ArtCache artcache;
static nde_database_t art_db = 0;
static nde_table_t artHashes = 0;

static ThreadID *artThread=0;
static void InitArtThread()
{
	if (!artThread)
	{
		artWake = CreateSemaphore(0, 0, 65536, 0);
		mainThread = WASABI_API_APP->main_getMainThreadHandle();
		artThread = WASABI_API_THREADPOOL->ReserveThread(0);
		WASABI_API_THREADPOOL->AddHandle(artThread, artWake, ArtLoadThread, 0, 0, 0);
	}
}

static int ArtKillThreadPoolFunc(HANDLE handle, void *user_data, intptr_t id)
{
	WASABI_API_THREADPOOL->RemoveHandle(artThread, artWake);
	CloseArtCache();
	SetEvent((HANDLE)user_data);
	return 0;
}

void KillArtThread()
{
	if (artThread)
	{
		HANDLE artKill = CreateEvent(0, FALSE, FALSE, 0);
		WASABI_API_THREADPOOL->RunFunction(artThread, ArtKillThreadPoolFunc, (void *)artKill, 0, 0);
		WaitForSingleObject(artKill, INFINITE);
		CloseHandle(mainThread); mainThread = 0;
		CloseHandle(artKill); artKill = 0;
		CloseHandle(artWake); artWake = 0;
		WASABI_API_THREADPOOL->ReleaseThread(artThread);
	}
}

void MigrateArtCache()
{
	int size[] = {0, 60, 90, 120, 180};

	for (int i = 0; i < sizeof(size)/sizeof(size[0]); i++)
	{
		if (!size[i])
		{
			const wchar_t *inidir = WASABI_API_APP->path_getUserSettingsPath();
			art_db = NDE_CreateDatabase(plugin.hDllInstance);
			wchar_t tableName[MAX_PATH] = {0}, oldTableName[MAX_PATH] = {0}, indexName[MAX_PATH] = {0}, oldIndexName[MAX_PATH] = {0};
			PathCombineW(oldIndexName, inidir, L"Plugins\\ml");
			PathCombineW(indexName, inidir, L"Plugins\\ml\\art");
			CreateDirectoryW(indexName, NULL);
			PathCombineW(tableName, indexName, L"art.dat");
			PathCombineW(oldTableName, oldIndexName, L"art.dat");
			PathAppendW(indexName, L"art.idx");
			PathAppendW(oldIndexName, L"art.idx");

			// migrate files to their own 'art' sub-folder
			if (PathFileExistsW(oldIndexName) && !PathFileExistsW(indexName))
			{
				MoveFileW(oldIndexName, indexName);
				MoveFileW(oldTableName, tableName);
			}
		}
		else if (size[i])
		{
			const wchar_t *inidir = WASABI_API_APP->path_getUserSettingsPath();
			wchar_t temp[MAX_PATH] = {0}, tableName[MAX_PATH] = {0}, oldTableName[MAX_PATH] = {0}, indexName[MAX_PATH] = {0}, oldIndexName[MAX_PATH] = {0};
			PathCombineW(temp, inidir, L"Plugins\\ml");
			StringCchPrintfW(tableName, MAX_PATH, L"%s\\art\\art_%d.dat", temp, size[i]);
			StringCchPrintfW(oldTableName, MAX_PATH, L"%s\\art_%d.dat", temp, size[i]);
			StringCchPrintfW(indexName, MAX_PATH, L"%s\\art\\art_%d.idx", temp, size[i]);
			StringCchPrintfW(oldIndexName, MAX_PATH, L"%s\\art_%d.idx", temp, size[i]);

			// migrate files to their own 'art' sub-folder
			if (PathFileExistsW(oldIndexName) && !PathFileExistsW(indexName))
			{
				MoveFileW(oldIndexName, indexName);
				MoveFileW(oldTableName, tableName);
			}
		}
	}
}

static bool InitArtCache(int size)
{
	if (!art_db)
	{
		const wchar_t *inidir = WASABI_API_APP->path_getUserSettingsPath();
		art_db = NDE_CreateDatabase(plugin.hDllInstance);
		wchar_t tableName[MAX_PATH] = {0}, indexName[MAX_PATH] = {0};
		PathCombineW(indexName, inidir, L"Plugins\\ml\\art");
		PathCombineW(tableName, indexName, L"art.dat");
		PathAppendW(indexName, L"art.idx");

		artHashes = NDE_Database_OpenTable(art_db, tableName, indexName, NDE_OPEN_ALWAYS, NDE_CACHE);
		NDE_Table_NewColumnW(artHashes, ARTHASH_FILENAME, DB_FIELDNAME_filename, FIELD_FILENAME);
		NDE_Table_NewColumnW(artHashes, ARTHASH_HASH, L"hash", FIELD_INT128);
		NDE_Table_PostColumns(artHashes);
		NDE_Table_AddIndexByIDW( artHashes, ARTHASH_FILENAME, DB_FIELDNAME_filename );
	}

	if (size && !artcache[size])
	{
		const wchar_t *inidir = WASABI_API_APP->path_getUserSettingsPath();
		wchar_t temp[MAX_PATH] = {0}, tableName[MAX_PATH] = {0}, indexName[MAX_PATH] = {0};
		PathCombineW(temp, inidir, L"Plugins\\ml");
		StringCchPrintfW(tableName, MAX_PATH, L"%s\\art\\art_%d.dat", temp, size);
		StringCchPrintfW(indexName, MAX_PATH, L"%s\\art\\art_%d.idx", temp, size);

		nde_table_t artTable = NDE_Database_OpenTable(art_db, tableName, indexName, NDE_OPEN_ALWAYS, NDE_NOCACHE);
		NDE_Table_NewColumnW(artTable, ARTTABLE_HASH, L"hash", FIELD_INT128);
		NDE_Table_NewColumnW(artTable, ARTTABLE_ARGB32, L"art", FIELD_BINARY32);
		NDE_Table_PostColumns(artTable);
		NDE_Table_AddIndexByIDW(artTable, ARTTABLE_HASH, L"hash");
		artcache[size] = artTable;
	}
	else
		return artHashes != 0;

	return artcache[size] != 0;
}
static void CloseArtHashes()
{
	__try
	{
		if (artHashes)
		{
			NDE_Database_CloseTable(art_db, artHashes);
		}
		artHashes = 0;

		NDE_DestroyDatabase(art_db);
		art_db = 0;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		artHashes = 0;
		art_db = 0;
	}
}
static void CloseArtCache()
{
	ArtCache::iterator itr;
	for (itr = artcache.begin(); itr != artcache.end(); itr++)
	{
		if (itr->second)
			NDE_Database_CloseTable(art_db, itr->second);
		itr->second = 0;
	}
	artcache.clear();

	CloseArtHashes();
}
/*
@param size = art dimensions.  e.g. size==120 is for 120x120 album art
@param bits better be allocated to size*size*sizeof(ARGB32) or you're in for a world of hurt
*/
static bool GetArtFromCache(const wchar_t *filename, int size, ARGB32 **bits)
{
	if (InitArtCache(size))
	{
		nde_scanner_t hashscanner = NDE_Table_CreateScanner(artHashes);
		if (NDE_Scanner_LocateFilename(hashscanner, ARTHASH_FILENAME, FIRST_RECORD, filename))
		{
			nde_field_t field = NDE_Scanner_GetFieldByID(hashscanner, ARTHASH_HASH);
			if (field)
			{
				nde_scanner_t artscanner = NDE_Table_CreateScanner(artcache[size]);
				if (NDE_Scanner_LocateField(artscanner, ARTTABLE_HASH, FIRST_RECORD, field))
				{
					nde_field_t field = NDE_Scanner_GetFieldByID(artscanner, ARTTABLE_ARGB32);
					if (field)
					{
						size_t len = 0;
						void *data = NDE_BinaryField_GetData(field, &len);
						if (data && len == size*size*sizeof(ARGB32))
						{
							*bits = (ARGB32 *)WASABI_API_MEMMGR->sysMalloc(len);
							memcpy(*bits, data, len);
							NDE_Table_DestroyScanner(artcache[size], artscanner);
							NDE_Table_DestroyScanner(artHashes, hashscanner);
							return true;
						}
					}
				}
				NDE_Table_DestroyScanner(artcache[size], artscanner);
			}
		}
		NDE_Table_DestroyScanner(artHashes, hashscanner);
	}
	return false;
}

static bool SetArtCache(const wchar_t *filename, int size, const ARGB32 *bits, uint8_t hash[16])
{
	if (InitArtCache(size))
	{
		nde_scanner_t hashscanner = NDE_Table_CreateScanner(artHashes);
		if (!NDE_Scanner_LocateFilename(hashscanner, ARTHASH_FILENAME, FIRST_RECORD, filename))
		{
			NDE_Scanner_New(hashscanner);
			db_setFieldStringW(hashscanner, ARTHASH_FILENAME, filename);
		}
		else
			NDE_Scanner_Edit(hashscanner);

		nde_field_t field = NDE_Scanner_GetFieldByID(hashscanner, ARTHASH_HASH);
		if (!field)
			field = NDE_Scanner_NewFieldByID(hashscanner, ARTHASH_HASH);
		NDE_Int128Field_SetValue(field, hash);

		NDE_Scanner_Post(hashscanner);

		nde_scanner_t artscanner = NDE_Table_CreateScanner(artcache[size]);
		if (!NDE_Scanner_LocateField(artscanner, ARTTABLE_HASH, FIRST_RECORD, field))
		{
			NDE_Scanner_New(artscanner);
			field = NDE_Scanner_NewFieldByID(artscanner, ARTTABLE_HASH);
			NDE_Int128Field_SetValue(field, hash);
			field = NDE_Scanner_NewFieldByID(artscanner, ARTTABLE_ARGB32);
			// TODO when size is possibly zero, this is causing a crash in nde.dll
			if (size > 0) NDE_BinaryField_SetData(field, bits, size*size*sizeof(ARGB32));
			NDE_Scanner_Post(artscanner);
			NDE_Table_DestroyScanner(artcache[size], artscanner);
			NDE_Table_DestroyScanner(artHashes, hashscanner);
			NDE_Table_Sync(artHashes);
			return true;
		}
		NDE_Table_DestroyScanner(artcache[size], artscanner);
		NDE_Table_DestroyScanner(artHashes, hashscanner);
		NDE_Table_Sync(artHashes);
	}

	return false;
}

size_t maxCache = 65536/*100*/;
void HintCacheSize(int _cachesize)
{
	maxCache = _cachesize;
}

void CreateCache(AlbumArtContainer *container, int w, int h)
{
	InitArtThread();
	assert(artThread);
	container->AddRef();
	CreateCacheParameters *parameters = (CreateCacheParameters *)WASABI_API_MEMMGR->sysMalloc(sizeof(CreateCacheParameters));
	parameters->container = container;
	parameters->w = w;
	parameters->h = h;
	parameters->status = AlbumArtContainer::CACHE_UNKNOWN;
	nu::AutoLock lock(queueGuard);
	if (artQueue.size() > maxCache)
	{
		CreateCacheParameters *kill = artQueue.back();
		artQueue.pop_back();
		kill->cache = 0;
		kill->status = AlbumArtContainer::CACHE_UNKNOWN;
		QueueUserAPC(CreateCacheCallbackAPC, mainThread, (ULONG_PTR)kill);
	}
	artQueue.push_front(parameters);
	ReleaseSemaphore(artWake, 1, 0);
}

void FlushCache()
{
	nu::AutoLock lock(queueGuard);
	while (!artQueue.empty())
	{
		CreateCacheParameters *kill = artQueue.front();
		kill->container->SetCache(0, AlbumArtContainer::CACHE_UNKNOWN);
		artQueue.pop_front();
		WASABI_API_MEMMGR->sysFree(kill);
	}
}

void ResumeCache()
{
}

static int ClearFilenameCacheAPC(HANDLE handle, void *param, intptr_t id)
{
	wchar_t *filename = (wchar_t *)param;

	if (InitArtCache(0))
	{
		nde_scanner_t hashscanner = NDE_Table_CreateScanner(artHashes);
		if (NDE_Scanner_LocateNDEFilename(hashscanner, ARTHASH_FILENAME, FIRST_RECORD, filename))
		{
			nde_field_t field = NDE_Scanner_GetFieldByID(hashscanner, ARTHASH_HASH);
			if (field)
			{
				nde_scanner_t deleteAllScanner = NDE_Table_CreateScanner(artHashes);
				while (NDE_Scanner_LocateField(deleteAllScanner, ARTHASH_HASH, FIRST_RECORD, field))
				{
					NDE_Scanner_Delete(deleteAllScanner);
					NDE_Scanner_Post(deleteAllScanner);
				}
				NDE_Table_DestroyScanner(artHashes, deleteAllScanner);
				NDE_Table_Sync(artHashes);

				/* delete it from the art table as well, but we'll just 
				use the already-opened ones */
				for (ArtCache::iterator itr=artcache.begin();itr!=artcache.end();itr++)
				{
					nde_table_t table = itr->second;
					if (table)
					{
						nde_scanner_t s= NDE_Table_CreateScanner(table);
						while (NDE_Scanner_LocateField(s, ARTTABLE_HASH, FIRST_RECORD, field))
						{
							NDE_Scanner_Delete(s);
							NDE_Scanner_Post(s);
						}
						NDE_Table_DestroyScanner(table, s);
						NDE_Table_Sync(table);
					}
				}				
			}
		}
		NDE_Table_DestroyScanner(artHashes, hashscanner);
	}
	ndestring_release(filename);
	return 0;
}

static int DeleteDatabaseAPC(HANDLE handle, void *user_data, intptr_t id)
{
	CloseArtCache();

	wchar_t search_mask[MAX_PATH] = {0};
	StringCchPrintfW(search_mask, MAX_PATH, L"%s\\art_*.*", g_tableDir);

	wchar_t fn[MAX_PATH] = {0};
	StringCchPrintfW(fn, MAX_PATH, L"%s\\art.idx", g_tableDir);
	DeleteFileW(fn);
	StringCchPrintfW(fn, MAX_PATH, L"%s\\art.dat", g_tableDir);
	DeleteFileW(fn);	

	WIN32_FIND_DATAW findData;
	HANDLE hFind = FindFirstFileW(search_mask, &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!_wcsnicmp(findData.cFileName, L"art_", 4))
			{
				StringCchPrintfW(fn, MAX_PATH, L"%s\\%s", g_tableDir, findData.cFileName);
				DeleteFileW(fn);
			}
		}
		while (FindNextFileW(hFind, &findData));
		FindClose(hFind);
	}
	return 0;
}

void DumpArtCache()
{
	InitArtThread();
	assert(artThread);
	WASABI_API_THREADPOOL->RunFunction(artThread, DeleteDatabaseAPC, 0, 0, 0);
}

void ClearCache(const wchar_t *filename)
{
	InitArtThread();
	assert(artThread);
	WASABI_API_THREADPOOL->RunFunction(artThread, ClearFilenameCacheAPC, ndestring_wcsdup(filename), 0, 0);
}