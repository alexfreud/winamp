#define PLUGIN_VERSION L"3.36"
#include "main.h"
#include "resource.h"
#include "api__ml_local.h"
#include "..\..\General\gen_ml/config.h"
#include <commctrl.h>
#include ".\ml_local.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include "../replicant/nu/AutoChar.h"
#include <api/service/waServiceFactory.h>
#include "../playlist/api_playlistmanager.h"
#include "mldbApiFactory.h"
#include "../nu/ServiceWatcher.h"
#include "LocalMediaCOM.h"
#include <tataki/export.h>
#include <strsafe.h>
#if 0
// disabled since not building cloud dlls
#include "../ml_cloud/CloudCallback.h"
#endif

static ServiceWatcher serviceWatcher;
static LocalMediaCOM localMediaCOM;
MLDBAPIFactory mldbApiFactory;
mlAddTreeItemStruct newTree;

#if 0
// disabled since not building cloud dlls
static class CloudCallbacks : public CloudCallback
{
	void OnCloudUploadStart(const wchar_t *filename) {
		SendMessage(m_curview_hwnd, WM_APP + 5, 0, (LPARAM)filename);
	}

	void OnCloudUploadDone(const wchar_t *filename, int code) {
		SendMessage(m_curview_hwnd, WM_APP + 5, MAKEWPARAM(code, 1), (LPARAM)filename);
	}
} cloudCallback;
#endif

LRESULT ML_IPC_MENUFUCKER_BUILD = -1, ML_IPC_MENUFUCKER_RESULT = -1;
int IPC_CLOUD_ENABLED = -1;

int CreateView(int treeItem, HWND parent);

static int Init();
static void Quit();
static INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

void SaveAll();

prefsDlgRecW preferences;

static wchar_t preferencesName[64] = {0};

int winampVersion = 0;
int substantives = 0;
int play_enq_rnd_alt = 0;

// Delay load library control << begin >>
#include <delayimp.h>
#pragma comment(lib, "delayimp")

bool nde_error = false;

FARPROC WINAPI FailHook(unsigned dliNotify, DelayLoadInfo *dli)
{
	nde_error = true;
	return 0;
}

/*
extern "C"
{
	PfnDliHook __pfnDliFailureHook2 = FailHook;
}
// Delay load library control << end >>
*/

#define CBCLASS PLCallBackW
START_DISPATCH;
VCB(IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile)
END_DISPATCH;
#undef CBCLASS

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

extern WORD waMenuID;
DEFINE_EXTERNAL_SERVICE(api_application,         WASABI_API_APP);
DEFINE_EXTERNAL_SERVICE(api_explorerfindfile,    WASABI_API_EXPLORERFINDFILE);
DEFINE_EXTERNAL_SERVICE(api_language,            WASABI_API_LNG);
DEFINE_EXTERNAL_SERVICE(api_syscb,               WASABI_API_SYSCB);
DEFINE_EXTERNAL_SERVICE(api_memmgr,              WASABI_API_MEMMGR);
DEFINE_EXTERNAL_SERVICE(api_albumart,            AGAVE_API_ALBUMART);
DEFINE_EXTERNAL_SERVICE(api_metadata,            AGAVE_API_METADATA);
DEFINE_EXTERNAL_SERVICE(api_playlistmanager,     AGAVE_API_PLAYLISTMANAGER);
DEFINE_EXTERNAL_SERVICE(api_itunes_importer,     AGAVE_API_ITUNES_IMPORTER);
DEFINE_EXTERNAL_SERVICE(api_playlist_generator,  AGAVE_API_PLAYLIST_GENERATOR);
DEFINE_EXTERNAL_SERVICE(api_threadpool,          WASABI_API_THREADPOOL);
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

int sse_flag;
int Init()
{
#ifdef _M_IX86
	int flags_edx;
	_asm 
	{
		mov eax, 1
			cpuid
			mov flags_edx, edx
	}

	sse_flag = flags_edx & 0x02000000;
#else
	sse_flag=1; // always supported on amd64
#endif
	InitializeCriticalSection(&g_db_cs);

	waMenuID = (WORD)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_REGISTER_LOWORD_COMMAND);

	Tataki::Init(plugin.service);

	plugin.service->service_register(&mldbApiFactory);

	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(WASABI_API_EXPLORERFINDFILE, ExplorerFindFileApiGUID);
	ServiceBuild(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceBuild(AGAVE_API_METADATA, api_metadataGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceBuild(WASABI_API_SYSCB, syscbApiServiceGuid);
	ServiceBuild(AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID);
	ServiceBuild(WASABI_API_THREADPOOL, ThreadPoolGUID);
	//ServiceBuild(AGAVE_API_PLAYLIST_GENERATOR, api_playlist_generator::getServiceGuid());

	serviceWatcher.WatchWith(plugin.service);
	serviceWatcher.WatchFor(&AGAVE_API_ITUNES_IMPORTER, api_itunes_importer::getServiceGuid());
	serviceWatcher.WatchFor(&AGAVE_API_PLAYLIST_GENERATOR, api_playlist_generator::getServiceGuid());
	WASABI_API_SYSCB->syscb_registerCallback(&serviceWatcher);
	#if 0
	// disabled since not building cloud dlls
	WASABI_API_SYSCB->syscb_registerCallback(&cloudCallback);
	#endif

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,MlLocalLangGUID);

	wchar_t buf[2] = {0};
	if(LoadString(WASABI_API_LNG_HINST,IDS_SUBSTANTIVES,buf,2)){
		substantives = 1;
	}

	// this is used to load alternative play/enqueue random strings where
	// the default implementation will cause pluralisation issue eg de-de
	buf[0] = 0;
	if(LoadString(WASABI_API_LNG_HINST,IDS_PLAY_ENQ_RND_ALTERNATIVE,buf,2)){
		play_enq_rnd_alt = 1;
	}

	static wchar_t szDescription[256];
	StringCchPrintfW(szDescription, ARRAYSIZE(szDescription),
					 WASABI_API_LNGSTRINGW(IDS_NULLSOFT_LOCAL_MEDIA), PLUGIN_VERSION);
	plugin.description = (char*)szDescription;

	ML_IPC_MENUFUCKER_BUILD = (int)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"menufucker_build", IPC_REGISTER_WINAMP_IPCMESSAGE);
	ML_IPC_MENUFUCKER_RESULT = (int)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"menufucker_result", IPC_REGISTER_WINAMP_IPCMESSAGE);
	IPC_CLOUD_ENABLED = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloudEnabled", IPC_REGISTER_WINAMP_IPCMESSAGE);

	mediaLibrary.library = plugin.hwndLibraryParent;
	mediaLibrary.winamp = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;
	winampVersion = mediaLibrary.GetWinampVersion();

	mediaLibrary.AddDispatch(L"LocalMedia", &localMediaCOM);

	// this may look unused, but we want to get this here since mediaLibrary will cache the inidir
	// and then we don't run into weird multithreaded SendMessage issues
	mediaLibrary.GetIniDirectory();
	mediaLibrary.GetIniDirectoryW();

	preferences.hInst = WASABI_API_LNG_HINST;
	preferences.dlgID = IDD_PREFSFR;
	preferences.proc = (void *)PrefsProc;
	preferences.name = WASABI_API_LNGSTRINGW_BUF(IDS_LOCAL_MEDIA,preferencesName,64);
	preferences.where = 6; // 0;
	mediaLibrary.AddPreferences(preferences);

	mediaLibrary.AddTreeImage(IDB_TREEITEM_AUDIO, TREE_IMAGE_LOCAL_AUDIO, (BMPFILTERPROC)FILTER_DEFAULT1);
	mediaLibrary.AddTreeImage(IDB_TREEITEM_MOSTPLAYED, TREE_IMAGE_LOCAL_MOSTPLAYED, (BMPFILTERPROC)FILTER_DEFAULT1);
	mediaLibrary.AddTreeImage(IDB_TREEITEM_NEVERPLAYED, TREE_IMAGE_LOCAL_NEVERPLAYED, (BMPFILTERPROC)FILTER_DEFAULT1);
	mediaLibrary.AddTreeImage(IDB_TREEITEM_RECENTLYPLAYED, TREE_IMAGE_LOCAL_RECENTLYPLAYED, (BMPFILTERPROC)FILTER_DEFAULT1);
	mediaLibrary.AddTreeImage(IDB_TREEITEM_RECENTLYADDED, TREE_IMAGE_LOCAL_RECENTLYADDED, (BMPFILTERPROC)FILTER_DEFAULT1);
	mediaLibrary.AddTreeImage(IDB_TREEITEM_TOPRATED, TREE_IMAGE_LOCAL_TOPRATED, (BMPFILTERPROC)FILTER_DEFAULT1);
	mediaLibrary.AddTreeImage(IDB_TREEITEM_VIDEO, TREE_IMAGE_LOCAL_VIDEO, (BMPFILTERPROC)FILTER_DEFAULT1);
	mediaLibrary.AddTreeImage(IDB_TREEITEM_PODCASTS, TREE_IMAGE_LOCAL_PODCASTS, (BMPFILTERPROC)FILTER_DEFAULT1);
	mediaLibrary.AddTreeImage(IDB_TREEITEM_RECENTLYMODIFIED, TREE_IMAGE_LOCAL_RECENTLYMODIFIED, (BMPFILTERPROC)FILTER_DEFAULT1);

	int ret = init();
	if (ret) return ret;

	NAVINSERTSTRUCT nis = {0};
	nis.item.cbSize = sizeof(NAVITEM);
	nis.item.pszText = WASABI_API_LNGSTRINGW(IDS_LOCAL_MEDIA);
	nis.item.pszInvariant = L"Local Media";
	nis.item.style = NIS_HASCHILDREN;
	nis.item.id = 1000;  // benski> use the old ID for backwards compatability
	nis.item.mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_STYLE | NIMF_ITEMID;

	// map to item id (will probably have to change but is a quick port to support invariant item naming)
	NAVITEM nvItem = {sizeof(NAVITEM),0,NIMF_ITEMID,};
	nvItem.hItem = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis);
	MLNavItem_GetInfo(plugin.hwndLibraryParent, &nvItem);
	m_query_tree = nvItem.id;

	loadQueryTree();

	m_query_mode = 0;
	m_query_metafile = L"default.vmd";

	return ret;
}

void Quit()
{
	serviceWatcher.StopWatching();
	serviceWatcher.Clear();

	// deregister this first, otherwise people might try to use it after we shut down the database!
	plugin.service->service_deregister(&mldbApiFactory);

	UnhookPlaylistEditor();

	Scan_Kill();

	closeDb();

	delete(g_view_metaconf);
	g_view_metaconf = 0;

	delete g_config;
	g_config = NULL;

	KillArtThread();

	for (QueryList::iterator i = m_query_list.begin();i != m_query_list.end();i++)
	{
		queryItem *item = i->second;
		if (item)
		{
			free(item->metafn);
			free(item->name);
			free(item->query);
		}
		free(item);
	}
	m_query_list.clear();
	DeleteCriticalSection(&g_db_cs);

	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceRelease(AGAVE_API_METADATA, api_metadataGUID);
	ServiceRelease(WASABI_API_SYSCB, syscbApiServiceGuid);
	ServiceRelease(AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID);
	ServiceRelease(AGAVE_API_ITUNES_IMPORTER, api_itunes_importer::getServiceGuid());
	ServiceRelease(WASABI_API_THREADPOOL, ThreadPoolGUID);
	ServiceRelease(AGAVE_API_PLAYLIST_GENERATOR, api_playlist_generator::getServiceGuid());

	Tataki::Quit();
}

INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	switch (message_type)
	{
	case ML_MSG_TREE_ONCREATEVIEW:       // param1 = param of tree item, param2 is HWND of parent. return HWND if it is us
		if (param1 == m_query_tree || m_query_list[param1])
			return (INT_PTR)onTreeViewSelectChange((HWND)param2);
		else
			return 0;

	case ML_MSG_NAVIGATION_CONTEXTMENU:
	{
		HNAVITEM hItem = (HNAVITEM)param1;
		HNAVITEM myItem = MLNavCtrl_FindItemById(plugin.hwndLibraryParent, m_query_tree);
		if (hItem == myItem)
		{
			queriesContextMenu(param1, (HWND)param2, MAKEPOINTS(param3));
			return 1;
		}
		else
		{
			NAVITEM nvItem = {sizeof(NAVITEM),hItem,NIMF_ITEMID,};
			MLNavItem_GetInfo(plugin.hwndLibraryParent, &nvItem);
			if (m_query_list[nvItem.id])
			{
				view_queryContextMenu(param1, (HWND)param2, MAKEPOINTS(param3), nvItem.id);
				return 1;
			}
		}
		return 0;
	}

	case ML_MSG_TREE_ONCLICK:
		if (param1 == m_query_tree)
			return OnLocalMediaClick(param2, (HWND)param3);
		else if (m_query_list[param1])
			return OnLocalMediaItemClick(param2, param1, (HWND)param3);
		else
			return 0;

	case ML_MSG_TREE_ONDRAG:
		if (m_query_list[param1])
		{
			int *type = reinterpret_cast<int *>(param3);
			*type = ML_TYPE_ITEMRECORDLIST;
			return 1;
		}
		return 0;

	case ML_MSG_TREE_ONDROP:
		if (param3 != NULL && param1 != NULL)  // here we go - finishing moving view
		{
			if (m_query_list[param1])
			{
				if (param3 == m_query_tree || m_query_list[param3])
				{
					QueryList::iterator src = m_query_list.find(param1);
					mediaLibrary.RemoveTreeItem(src->first);
					MLTREEITEMW srcItem = {sizeof(MLTREEITEMW), };

					srcItem.title = src->second->name;
					srcItem.hasChildren = 0;
					srcItem.parentId = m_query_tree;
					srcItem.id = param3;
					srcItem.imageIndex = src->second->imgIndex;
					mediaLibrary.InsertTreeItem(srcItem);

					auto item = src->second;
					m_query_list.erase(param1);
					m_query_list.insert({ srcItem.id, item });

					saveQueryTree();
					mediaLibrary.SelectTreeItem(srcItem.id);
					return 1;
				}
			}
		}
		else if (m_query_list[param1])
		{
			mlDropItemStruct m = {0};
			m.type = ML_TYPE_ITEMRECORDLISTW;
			m.p = *(POINT *)param2;
			m.flags = ML_HANDLEDRAG_FLAG_NOCURSOR | ML_HANDLEDRAG_FLAG_NAME;

			// build an itemRecordList
			queryItem *item = m_query_list[param1];
			wchar_t configDir[MAX_PATH] = {0};
			PathCombineW(configDir, g_viewsDir, item->metafn);
			C_Config viewconf(configDir);

			EnterCriticalSection(&g_db_cs);
			nde_scanner_t s = NDE_Table_CreateScanner(g_table);
			NDE_Scanner_Query(s, item->query);
			itemRecordListW obj = {0, };
			saveQueryToListW(&viewconf, s, &obj, 0, 0, (resultsniff_funcW)-1);
			NDE_Table_DestroyScanner(g_table, s);
			LeaveCriticalSection(&g_db_cs);
			m.data = (void *) & obj;
			AutoChar whatsThisNameUsedForAnyway(item->name);
			m.name = whatsThisNameUsedForAnyway;

			pluginHandleIpcMessage(ML_IPC_HANDLEDROP, (WPARAM)&m);
			if (m.result < 1)
			{
				m.result = 0;
				m.type = ML_TYPE_ITEMRECORDLIST;
				itemRecordList objA={0,};
				convertRecordList(&objA, &obj);
				m.data = (void*)&objA;
				pluginHandleIpcMessage(ML_IPC_HANDLEDRAG, (WPARAM)&m);
				freeRecordList(&objA);
			}
			freeRecordList(&obj);
		}
		return 0;

	case ML_MSG_CONFIG:
		mediaLibrary.GoToPreferences(preferences._id);
		return TRUE;

	case ML_MSG_VIEW_PLAY_ENQUEUE_CHANGE:
		enqueuedef = param1;
		groupBtn = param2;
		PostMessage(m_curview_hwnd, WM_APP + 104, param1, param2);
		return 0;

	case ML_MSG_ONSENDTOBUILD:
		if (param1 == ML_TYPE_ITEMRECORDLISTW || param1 == ML_TYPE_ITEMRECORDLIST ||
		    param1 == ML_TYPE_FILENAMES  || param1 == ML_TYPE_FILENAMESW)
		{
			if (!myMenu) mediaLibrary.AddToSendTo(WASABI_API_LNGSTRINGW(IDS_ADD_TO_LOCAL_MEDIA), param2, (INT_PTR)MessageProc);
		}
		break;

	case ML_MSG_ONSENDTOSELECT:
	case ML_MSG_TREE_ONDROPTARGET:      // return -1 if not allowed, 1 if allowed, or 0 if not our tree item
		// set with droptarget defaults =)
		INT_PTR type, data;

		if (message_type == ML_MSG_ONSENDTOSELECT)
		{
			if (param3 != (INT_PTR)MessageProc) return 0;

			type = param1;
			data = param2;
		}
		else
		{
			if (param1 != m_query_tree && !m_query_list[param1]) return 0;

			type = param2;
			data = param3;

			if (!data)
			{
				return (type ==  ML_TYPE_ITEMRECORDLISTW || type ==  ML_TYPE_ITEMRECORDLIST ||
				        type == ML_TYPE_FILENAMES || type == ML_TYPE_FILENAMESW ||
				        type == ML_TYPE_PLAYLIST) ? 1 : -1;
			}
		}

		if (data)
		{
			if (type == ML_TYPE_ITEMRECORDLIST)
			{
				itemRecordList *p = (itemRecordList*)data;
				for (int x = 0; x < p->Size; x ++)
					mediaLibrary.AddToMediaLibrary(p->Items[x].filename);

				return 1;
			}
			else if (type == ML_TYPE_ITEMRECORDLISTW)
			{
				itemRecordListW *p = (itemRecordListW*)data;
				for (int x = 0; x < p->Size; x ++)
					mediaLibrary.AddToMediaLibrary(p->Items[x].filename);

				return 1;
			}
			else if (type == ML_TYPE_PLAYLIST)
			{
				mlPlaylist * pl = (mlPlaylist *)data;
				PLCallBackW plCB;
				if (AGAVE_API_PLAYLISTMANAGER && PLAYLISTMANAGER_SUCCESS != AGAVE_API_PLAYLISTMANAGER->Load(pl->filename, &plCB))
				{
					mediaLibrary.AddToMediaLibrary(pl->filename);
				}
				return 1;
			}
			else if (type == ML_TYPE_FILENAMES)
			{
				const char *p = (const char*)data;
				while (p && *p)
				{
					PLCallBackW plCB;
					if (AGAVE_API_PLAYLISTMANAGER && PLAYLISTMANAGER_SUCCESS != AGAVE_API_PLAYLISTMANAGER->Load(AutoWide(p), &plCB))
					{
						mediaLibrary.AddToMediaLibrary(p);
					}
					p += strlen(p) + 1;
				}
				return 1;
			}
			else if (type == ML_TYPE_FILENAMESW)
			{
				const wchar_t *p = (const wchar_t*)data;
				while (p && *p)
				{
					PLCallBackW plCB;
					if (AGAVE_API_PLAYLISTMANAGER && PLAYLISTMANAGER_SUCCESS != AGAVE_API_PLAYLISTMANAGER->Load(p, &plCB))
					{
						mediaLibrary.AddToMediaLibrary(p);
					}
					p += wcslen(p) + 1;
				}
				return 1;
			}
		}
		break;

	case ML_MSG_TREE_ONKEYDOWN:
	{
		NMTVKEYDOWN *p = (NMTVKEYDOWN*)param2;
		int ctrl = (GetAsyncKeyState(VK_CONTROL)&0x8000);
		int shift = (GetAsyncKeyState(VK_SHIFT)&0x8000);

		if (p->wVKey == VK_INSERT && !shift)
		{
			if (!ctrl)
				addNewQuery(plugin.hwndLibraryParent);
			else
				if (!g_bgscan_scanning) SendMessage(plugin.hwndLibraryParent, WM_USER + 575, 0xffff00dd, 0);

			PostMessageW(plugin.hwndLibraryParent, WM_NEXTDLGCTL, (WPARAM)param3, (LPARAM)TRUE);
			return 1;
		}
		else if (m_query_list[param1])
		{
			if (p->wVKey == VK_F2 && !shift && !ctrl)
			{
				queryEditItem(param1);
			}
			else if (p->wVKey == VK_DELETE && !shift && !ctrl)
			{
				queryDeleteItem(plugin.hwndLibraryParent, param1);
			}
			else
				break;

			PostMessageW(plugin.hwndLibraryParent, WM_NEXTDLGCTL, (WPARAM)param3, (LPARAM)TRUE);
			return 1;
		}
		return 0;
	}

	case ML_IPC_HOOKEXTINFO:
		if (IPC_HookExtInfo(param1)) return 1;
		break;

	case ML_IPC_HOOKEXTINFOW:
		if (IPC_HookExtInfoW(param1)) return 1;
		break;

	case ML_IPC_HOOKTITLEW:
		if (IPC_HookTitleInfo(param1)) return 1;
		break;

	case ML_MSG_PLAYING_FILE:
		if (param1) onStartPlayFileTrack((const wchar_t *)param1, false);
		break;
	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" winampMediaLibraryPlugin plugin = 
{ 
	MLHDR_VER, 
	"nullsoft(ml_local.dll)", 
	Init, 
	Quit, 
	MessageProc, 
	0, 
	0, 
	0,
};

extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &plugin;
}