#include "main.h"
#include "./api__ml_online.h"
#include "./config.h"
#include "./navigation.h"
#include "./resource.h"
#include "./preferences.h"
#include "./serviceHelper.h"

#include "../../General/gen_ml/ml.h"
#include "../../General/gen_ml/ml_ipc_0313.h"

#include "../nu/MediaLibraryInterface.h"
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoWide.h"
#include <vector>
#include "../nu/nonewthrow.c"
#include "../nu/ConfigCOM.h"

#include "BufferCache.h"
#include "OMCOM.h"
#include "JNetCom.h" // for buffer_map

#include <shlwapi.h>
#include <strsafe.h>


static int Plugin_Init();
static void Plugin_Quit();
static INT_PTR Plugin_MessageProc(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3);

static Navigation *navigation = NULL;
static std::vector<PLUGINUNLOADCALLBACK> *unloadCallbacks = NULL;

C_Config *g_config=NULL;

extern "C"  winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"nullsoft(ml_online.dll)",
    Plugin_Init,
    Plugin_Quit,
    Plugin_MessageProc,
    0,
    0,
    0,
};


HINSTANCE Plugin_GetInstance(void)
{
	return plugin.hDllInstance;
}

HWND Plugin_GetWinamp(void)
{
	return plugin.hwndWinampParent;
}

HWND Plugin_GetLibrary(void)
{
	return plugin.hwndLibraryParent;
}

HRESULT Plugin_GetNavigation(Navigation **instance)
{
	if(NULL == instance) return E_POINTER;

	if (NULL == navigation) 
	{
		*instance = NULL;
		return E_UNEXPECTED;
	}

	*instance = navigation;
	navigation->AddRef();

	return S_OK;
}

typedef struct __PLUGINTIMERREC
{
	UINT_PTR id;
	PLUGINTIMERPROC callback;
	ULONG_PTR data;
} PLUGINTIMERREC;

typedef std::vector<PLUGINTIMERREC> PluginTimerList;

static void CALLBACK Plugin_TimerProcDispath(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD elapsedMs)
{
	HWND hLibrary = Plugin_GetLibrary();
	if (NULL != hLibrary && FALSE != IsWindow(hLibrary))
	{
		PluginTimerList *list = (PluginTimerList*)GetProp(hLibrary, L"OnlineMediaTimerData");
		if (NULL != list)
		{
			size_t index = list->size();
			while(index--)
			{
				PLUGINTIMERREC *rec = &list->at(index);
				if (rec->id == eventId)
				{
					rec->callback(eventId, elapsedMs, rec->data);
					return;
				}
			}
		}
	}

	KillTimer(hwnd, eventId);
}

UINT_PTR Plugin_SetTimer(UINT elapseMs, PLUGINTIMERPROC callback, ULONG_PTR data)
{
	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary || FALSE == IsWindow(hLibrary))
		return 0;
		
	if (GetCurrentThreadId() != GetWindowThreadProcessId(hLibrary, NULL))
		return 0;

	if (NULL == callback)
		return 0;

	PluginTimerList *list = (PluginTimerList*)GetProp(hLibrary, L"OnlineMediaTimerData");
	if (NULL == list)
	{
		list = new PluginTimerList();
		if (NULL == list) return 0;
		if (0 == SetProp(hLibrary, L"OnlineMediaTimerData", list))
		{
			delete(list);
			return 0;
		}
	}

	PLUGINTIMERREC rec;
	rec.data = data;
	rec.callback = callback;
	rec.id = SetTimer(NULL, NULL, elapseMs, Plugin_TimerProcDispath);
	if (0 == rec.id)
	{
		if (0 == list->size())
		{
			RemoveProp(hLibrary, L"OnlineMediaTimerData");
			delete(list);
		}
		return 0;
	}

	list->push_back(rec);
	return rec.id;
}

void Plugin_KillTimer(UINT_PTR eventId)
{
	KillTimer(NULL, eventId);

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary || FALSE == IsWindow(hLibrary))
		return;
	
	PluginTimerList *list = (PluginTimerList*)GetProp(hLibrary, L"OnlineMediaTimerData");
	if (NULL == list) return;
		
	size_t index = list->size();
	while(index--)
	{
		if (list->at(index).id == eventId)
		{
			list->erase(list->begin() + index);
			break;
		}
	}

	if (0 == list->size())
	{
		RemoveProp(hLibrary, L"OnlineMediaTimerData");
		delete(list);
	}
}

static void Plugin_UninitializeTimer()
{
	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary || FALSE == IsWindow(hLibrary))
		return;
	
	PluginTimerList *list = (PluginTimerList*)GetProp(hLibrary, L"OnlineMediaTimerData");
	RemoveProp(hLibrary, L"OnlineMediaTimerData");
	if (NULL == list) return;
		
	size_t index = list->size();
	while(index--)
	{
		KillTimer(NULL, list->at(index).id);
	}

	delete(list);
}


wchar_t g_w_cachedir[2048] = {0};
int     winampVersion=0;

OMCOM omCOM;

URLMap urlMap;
MetadataMap metadataMap;
Nullsoft::Utility::LockGuard urlMapGuard;

void LoadCacheItem( wchar_t *path )
{
	FILECACHETYPE cachefile = {0};
	unsigned long size = 0;
	HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return;
	ReadFile(hFile, &cachefile, sizeof(FILECACHETYPE),&size, NULL);
	if ( size == sizeof(FILECACHETYPE ))
	{
		size = 0;
		time_t now = time(NULL);
		//read the header, validate
		if ( cachefile.version == FILECACHEVERSION )
		{
			if ( now < cachefile.expires )
			{
				char *url = (char *)calloc((size_t)cachefile.urllen, sizeof(char));
				if (url)
				{
					size = 0;
					ReadFile(hFile, url, (DWORD)cachefile.urllen, &size, NULL);
					if ( cachefile.urllen == size ) // we read it ok!
					{
						char tempbuf[16384] = {0};
						Buffer_GrowBuf *newbuffer = new Buffer_GrowBuf;
						INT64 readin=0;
						newbuffer->expire_time = (time_t)cachefile.expires;
						while ( readin != cachefile.datalen )
						{
							DWORD toread=(DWORD)cachefile.datalen - (DWORD)readin;
							if ( toread > 16384 ) toread=16384;
							size = 0;
							int success = ReadFile(hFile, &tempbuf, toread , &size, NULL);
							if ( success )
							{
								if ( size ) 
								{
									newbuffer->add(tempbuf,(int)size);
									readin += size;
								}
							}
							else
							{ 
								break;
							}
						}
						if ( readin != cachefile.datalen )
						{
							delete newbuffer;
						}
						else
						{
							buffer_map[(wchar_t *)AutoWide(url)]=newbuffer;
						}
					}
					else
					{
						free(url);
					}
				}
			}
		}
	}
	CloseHandle(hFile);
	DeleteFile(path);
}

void LoadCache()
{
	WIN32_FIND_DATA FindFileData = {0};
	HANDLE hFind;
	wchar_t searchs[2048] = {0};

	buffer_map.clear();

	StringCchPrintf(searchs, 2048, L"%s\\*.w5x",g_w_cachedir);
	hFind = FindFirstFile(searchs, &FindFileData);
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do 
		{
			wchar_t activefile[2048] = {0};
			StringCchPrintf(activefile, 2048, L"%s\\%s",g_w_cachedir,FindFileData.cFileName);
			LoadCacheItem(activefile);
		} while ( FindNextFile(hFind, &FindFileData) );
		FindClose(hFind);
	}
}

void SaveCache()
{
	BufferMap::iterator buffer_it;
	DWORD start=0xABBACAFE;
	for(buffer_it = buffer_map.begin();buffer_it != buffer_map.end(); buffer_it++)
	{
		time_t now = time(NULL);
		if ( buffer_it->second->expire_time > now )
		{
			wchar_t filename[2048] = {0};
			FILECACHETYPE cachefile;
			HANDLE hFile;
			INT64 size=0;
			memset((void *)&cachefile,0,sizeof(FILECACHETYPE));        
			cachefile.version = FILECACHEVERSION;
			cachefile.expires = buffer_it->second->expire_time;
			AutoChar charUrl(buffer_it->first.c_str());
			cachefile.urllen = strlen(charUrl)+1;
			cachefile.datalen = buffer_it->second->getlen()+1;

			StringCchPrintf(filename, 2048, L"%s\\%08X.w5x",g_w_cachedir,start++);
			hFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS , FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE) 
			{
				WriteFile(hFile, &cachefile, sizeof(FILECACHETYPE),(LPDWORD)&size,NULL);
				if ( size == sizeof(FILECACHETYPE) )
				{
					char blank[2]="\0";
					size = 0; WriteFile(hFile, (char *)charUrl ,(DWORD)cachefile.urllen, (LPDWORD)&size, NULL);
					size = 0; WriteFile(hFile, buffer_it->second->get() , (DWORD)buffer_it->second->getlen(), (LPDWORD)&size, NULL);
					size = 0; WriteFile(hFile, blank , 1, (LPDWORD)&size, NULL);
				}
				else
				{
					CloseHandle(hFile);
					hFile=NULL;
					DeleteFile(filename);
				}
			}
			if (hFile)
			{
				CloseHandle(hFile);
				hFile=NULL;
			}
		}
	}
}

void initConfigCache()
{
	wchar_t iniFileName[2048] = {0};
	mediaLibrary.BuildPath(L"Plugins\\ml", iniFileName, 2048);
	CreateDirectory(iniFileName, NULL);
	mediaLibrary.BuildPath(L"Plugins\\ml\\cache", g_w_cachedir, 2048);
	CreateDirectory(g_w_cachedir, NULL);
	mediaLibrary.BuildPath(L"Plugins\\ml\\ml_online.ini", iniFileName, 2048);
	AutoChar charFn(iniFileName);
	g_config = new C_Config(AutoChar(iniFileName));

	int x = g_config->ReadInt("maxbandwidth", MAXBANDWIDTH );
	g_config->WriteInt("maxbandwidth",x);

	x = g_config->ReadInt("minbandwidth",1);
	g_config->WriteInt("minbandwidth",x);

	LoadCache();
}

static void Plugin_ExecuteOpenOnce()
{
	CHAR szBuffer[128] = {0};
	INT cchLen = Config_ReadStr("Navigation", "openOnce", NULL, szBuffer, ARRAYSIZE(szBuffer));
	if (0 != cchLen)
	{
		UINT serviceId;
		if (FALSE != StrToIntExA(szBuffer, STIF_SUPPORT_HEX, (INT*)&serviceId))
		{
			
			cchLen = Config_ReadStr("Navigation", "openOnceMode", NULL, szBuffer, ARRAYSIZE(szBuffer));
			UINT showMode;
			if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "popup", -1, szBuffer, cchLen))
				showMode = SHOWMODE_POPUP;
			else if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "ensureVisible", -1, szBuffer, cchLen))
				showMode = SHOWMODE_ENSUREVISIBLE;
			else
				showMode = SHOWMODE_NORMAL;

			ServiceHelper_ShowService(serviceId, showMode);
		}
		
		Config_WriteStr("Navigation", "openOnce", NULL);
		Config_WriteStr("Navigation", "openOnceMode", NULL);
	}
}

static int Plugin_Init()
{
	if (FAILED(WasabiApi_Initialize(plugin.hDllInstance, plugin.service)))
		return 1;

	if (FAILED(WasabiApi_LoadDefaults()) || 
		NULL == OMBROWSERMNGR ||
		NULL == OMSERVICEMNGR ||
		NULL == OMUTILITY)
	{
		WasabiApi_Release();
		return 2;
	}

	ServiceHelper_Initialize();

	if (NULL != WASABI_API_LNG)
	{
		static wchar_t szDescription[256];
		StringCchPrintf(szDescription, ARRAYSIZE(szDescription),
						WASABI_API_LNGSTRINGW(IDS_PLUGIN_NAME),
						PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
		plugin.description = (char*)szDescription;
	}

	mediaLibrary.library  = plugin.hwndLibraryParent;
	mediaLibrary.winamp   = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;

	winampVersion = mediaLibrary.GetWinampVersion();

	omCOM.Publish();
	
	Preferences_Register();

	if (NULL == navigation)
	{
		if (FAILED(Navigation::CreateInstance(&navigation)))
		{
			navigation = NULL;

			if (NULL != unloadCallbacks)
			{
				size_t index = unloadCallbacks->size();
				while(index--)
					unloadCallbacks->at(index)();
				delete(unloadCallbacks);
			}

			Preferences_Unregister();
			WasabiApi_Release();
			return 3;
		}
	}

	initConfigCache();
		
	Plugin_ExecuteOpenOnce();
	return ML_INIT_SUCCESS;
}

static void Plugin_Quit()
{
	SaveCache();
	buffer_map.clear();

	Plugin_UninitializeTimer();

	if (NULL != navigation)
	{
		navigation->Finish();
		navigation->Release();
		navigation = NULL;
	}

	if (NULL != unloadCallbacks)
	{
		size_t index = unloadCallbacks->size();
		while(index--)
			unloadCallbacks->at(index)();
		delete(unloadCallbacks);
		unloadCallbacks = NULL;
	}

	Preferences_Unregister();

	WasabiApi_Release();
}

static INT_PTR TitleHook(waHookTitleStructW *hookTitle)
{
	if (NULL == hookTitle ||
		NULL == hookTitle->filename)
	{
		return 0;
	}

	Nullsoft::Utility::AutoLock lock(urlMapGuard);
	// this is kinda slow but AOL Videos is so underused anyway that this map won't fill up much
	URLMap::iterator itr;
	for (itr=urlMap.begin();itr!=urlMap.end();itr++)
	{
		if (!_wcsnicmp(hookTitle->filename, itr->url.c_str(), itr->url_wcslen))
		{
			if (NULL != hookTitle->title)
				StringCchCopy(hookTitle->title, 2048, itr->title.c_str());
			
			hookTitle->length = itr->length;
			return 1;
		}
	}

	return 0;
}

static INT_PTR MetadataHook(extendedFileInfoStructW *hookMetadata)
{
	if (NULL == hookMetadata ||
		NULL == hookMetadata->filename ||
		NULL == hookMetadata->metadata)
	{
		return 0;
	}

	Nullsoft::Utility::AutoLock lock(urlMapGuard);
	// this is kinda slow but AOL Videos is so underused anyway that this map won't fill up much
	MetadataMap::iterator itr;

	for (itr=metadataMap.begin();itr!=metadataMap.end();itr++)
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, hookMetadata->filename, -1, itr->url.c_str(), - 1) &&
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, hookMetadata->metadata, -1, itr->tag.c_str(), - 1))
		{
			StringCchCopy(hookMetadata->ret, hookMetadata->retlen, itr->metadata.c_str());
			return 1;
		}
	}
	return 0;
}


static INT_PTR Plugin_MessageProc(int msg, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	INT_PTR result = 0;
	if (NULL != navigation && 
		FALSE != navigation->ProcessMessage(msg, param1, param2, param3, &result))
	{
		return result;
	}
	
	switch (msg)
	{
 		case ML_IPC_HOOKTITLEW: 	return TitleHook((waHookTitleStructW *)param1);
		case ML_IPC_HOOKEXTINFOW:	return MetadataHook((extendedFileInfoStructW *)param1);
		case ML_MSG_CONFIG:			Preferences_Show(); return TRUE;
	}

	return FALSE;
}


void Plugin_RegisterUnloadCallback(PLUGINUNLOADCALLBACK callback)
{
	if (NULL == unloadCallbacks)
	{
		unloadCallbacks = new std::vector<PLUGINUNLOADCALLBACK>();
		if (NULL == unloadCallbacks)
			return;
	}
	unloadCallbacks->push_back(callback);
}


extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &plugin;
}

#if 0
extern "C" __declspec( dllexport ) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {

		// prompt to remove our settings with default as no (just incase)
		/*if(MessageBoxA(hwndDlg,"Do you also want to remove the saved settings for this plugin?",
					   plugin.description,MB_YESNO|MB_DEFBUTTON2) == IDYES)
		{
			WritePrivateProfileString("ml_rg", 0, 0, iniFile);
		}*/

		// also attempt to remove the ReplayGainAnalysis.dll so everything is kept cleaner
		/*char path[MAX_PATH] = {0};
		GetModuleFileName(hDllInst, path, MAX_PATH);
		PathRemoveFileSpec(path);
		PathAppend(path, "ReplayGainAnalysis.dll");
		// if we get a handle then try to lower the handle count so we can delete
		HINSTANCE rgLib = GetModuleHandle(path);
		if(rgLib)
			FreeLibrary(rgLib);
		DeleteFile(path);*/

		// allow an on-the-fly removal (since we've got to be with a compatible client build)
		return ML_PLUGIN_UNINSTALL_NOW;
	}
#endif