#define PLUGIN_VER L"2.08"

#include "main.h"
#include "tagger.h"
#include "../nu/MediaLibraryInterface.h"

api_language   *WASABI_API_LNG = 0;
HINSTANCE       WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
api_gracenote  *AGAVE_API_GRACENOTE = 0;
api_decodefile *AGAVE_API_DECODE = 0;
api_mldb       *AGAVE_API_MLDB = 0;

static int Init();
static void Quit();

extern "C" winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"nullsoft(ml_autotag.dll)",
	Init,
	Quit,
	PluginMessageProc,
	0,
	0,
	0,
};

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

const char *INI_FILE=0;

int Init()
{
	mediaLibrary.library  = plugin.hwndLibraryParent;
	mediaLibrary.winamp   = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;

	INI_FILE = mediaLibrary.GetWinampIni();

	ServiceBuild( AGAVE_API_GRACENOTE, gracenoteApiGUID );
	ServiceBuild( AGAVE_API_DECODE,    decodeFileGUID );
	ServiceBuild( WASABI_API_LNG,      languageApiGUID );
	ServiceBuild( AGAVE_API_MLDB,      mldbApiGuid );
	
	if ( !AGAVE_API_GRACENOTE || !AGAVE_API_DECODE || !WASABI_API_LNG )
		return ML_INIT_FAILURE; // error!

	WASABI_API_START_LANG( plugin.hDllInstance, MlAutoTagLangGUID );

	static wchar_t szDescription[256];
	swprintf( szDescription, ARRAYSIZE( szDescription ), WASABI_API_LNGSTRINGW( IDS_PLUGINNAME ), PLUGIN_VER );
	plugin.description = (char*)szDescription;


	return ML_INIT_SUCCESS;
}

void Quit()
{
}

void LookupTracks(std::vector<TagItem*> &list, HWND parent=0)
{
	if(!AGAVE_API_MLDB)
	{
		ServiceBuild(AGAVE_API_MLDB, mldbApiGuid);
		if(!AGAVE_API_MLDB) return;
	}

	if(!parent) parent = plugin.hwndLibraryParent;
	ICDDBMusicIDManager3 *musicid = AGAVE_API_GRACENOTE?AGAVE_API_GRACENOTE->GetMusicID():NULL;
	if(!musicid)
	{
		wchar_t title[32] = {0};
		MessageBoxW(parent,WASABI_API_LNGSTRINGW(IDS_GRACENOTE_TOOLS_NOT_INSTALLED),
					WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,title, 32),0);
		return;
	}

	//parent = 0; // non-blocking...
	// as we're not parenting directly, then we have a look at the aot state and fiddle it
	// -> not ideal but at least the dialog will show on top so it can be seen initially
	HWND dialog = WASABI_API_CREATEDIALOGPARAMW(IDD_AUTOTAGGER,parent,autotagger_dlgproc,(LPARAM)new Tagger(list, musicid));
	if(GetWindowLongPtr(plugin.hwndWinampParent,GWL_EXSTYLE) & WS_EX_TOPMOST)
		SetWindowPos(dialog, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
}

static bool IsInternetAvailable()
{
	return !!SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_INETAVAILABLE);
}

INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	if (message_type == ML_MSG_NO_CONFIG)
	{
		return TRUE;
	}
	else if (message_type == ML_MSG_ONSENDTOBUILD)
	{
		if ((param1 == ML_TYPE_ITEMRECORDLIST || param1 == ML_TYPE_FILENAMES ||
			 param1 == ML_TYPE_STREAMNAMES || param1 == ML_TYPE_ITEMRECORDLISTW ||
			 param1 == ML_TYPE_FILENAMESW || param1 == ML_TYPE_STREAMNAMESW) &&
			 IsInternetAvailable())
		{
			mediaLibrary.AddToSendTo(WASABI_API_LNGSTRINGW(IDS_AUTOTAG), param2, (INT_PTR)PluginMessageProc);
		}
	}
	else if (message_type == ML_MSG_ONSENDTOSELECT)
	{
		if (param3 != (INT_PTR)PluginMessageProc) return 0;

		INT_PTR type = param1;
		INT_PTR data = param2;

		if (data)
		{
			if (type == ML_TYPE_ITEMRECORDLISTW)
			{
				std::vector<TagItem*> list;
				itemRecordListW *p = (itemRecordListW*)data;
				for (int x = 0; x < p->Size; x ++)
				{
					list.push_back(new TagItem(p->Items[x].filename));
				}
				LookupTracks(list);

				//list.deleteAll();
				for (auto obj : list)
				{
					delete obj;
				}
				list.clear();
				return 1;
			}
			else if (type == ML_TYPE_ITEMRECORDLIST)
			{
				std::vector<TagItem*> list;
				itemRecordList *p = (itemRecordList*)data;
				for (int x = 0; x < p->Size; x ++)
				{
					list.push_back(new TagItem(p->Items[x].filename));
				}
				LookupTracks(list);

				//list.deleteAll();
				for (auto obj : list)
				{
					delete obj;
				}
				list.clear();

				return 1;
			}
			else if (type == ML_TYPE_FILENAMESW || type == ML_TYPE_STREAMNAMESW)
			{
				std::vector<TagItem*> list;
				wchar_t *p = (wchar_t*)data;
				while (p && *p)
				{
					list.push_back(new TagItem(p));
					p += wcslen(p) + 1;
				}
				LookupTracks(list);

				//list.deleteAll();
				for (auto obj : list)
				{
					delete obj;
				}
				list.clear();

				return 1;
			}
			else if (type == ML_TYPE_FILENAMES || type == ML_TYPE_STREAMNAMES)
			{
				std::vector<TagItem*> list;
				char *p = (char*)data;
				while (p && *p)
				{
					list.push_back(new TagItem(p));
					p += strlen(p) + 1;
				}
				LookupTracks(list);

				//list.deleteAll();
				for (auto obj : list)
				{
					delete obj;
				}
				list.clear();

				return 1;
			}
		}
	}
	return 0;
}

extern "C"
{
	__declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
	{
		return &plugin;
	}

	__declspec(dllexport) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
		// allow an on-the-fly removal (since we've got to be with a compatible client build)
		return ML_PLUGIN_UNINSTALL_NOW;
	}
}