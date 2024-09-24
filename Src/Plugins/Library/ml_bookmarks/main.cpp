//#define PLUGIN_NAME "Nullsoft Bookmarks"
#define PLUGIN_VERSION L"1.27"

#include "Main.h"
#include "../nu/AutoWide.h"
#include <strsafe.h>
#include "../../General/gen_ml/menu.h"
#include "../../General/gen_ml/ml_ipc_0313.h"

static int Init();
static void Quit();

extern "C" winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"nullsoft(ml_bookmarks.dll)",
	Init,
	Quit,
	bm_pluginMessageProc,
	0,
	0,
	0,
};

int bookmark_treeItem = 0;
HMENU g_context_menus, g_context_menus2;
HCURSOR hDragNDropCursor;
C_Config *g_config;
WNDPROC waProc=0;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
api_stats *AGAVE_API_STATS = 0;
api_application *WASABI_API_APP = 0;

static DWORD WINAPI wa_newWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_WA_IPC && lParam == IPC_ADDBOOKMARK && wParam && wParam != 666)
	{
		bookmark_notifyAdd(AutoWide((char*)wParam));
	}
	else if (msg == WM_WA_IPC && lParam == IPC_ADDBOOKMARKW && wParam && wParam != 666)
	{
		bookmark_notifyAdd((wchar_t*)wParam);
	}
	else if ((msg == WM_COMMAND || msg == WM_SYSCOMMAND) && LOWORD(wParam) == WINAMP_EDIT_BOOKMARKS)
	{
		mediaLibrary.ShowMediaLibrary();
		mediaLibrary.SwitchToPluginView(bookmark_treeItem);
		return 0;
	}

	if (waProc)
		return (DWORD)CallWindowProcW(waProc, hwnd, msg, wParam, lParam);
	else
		return (DWORD)DefWindowProc(hwnd, msg, wParam, lParam);
}

int Init()
{
	waProc = (WNDPROC)SetWindowLongPtrW( plugin.hwndWinampParent, GWLP_WNDPROC, (LONG_PTR)wa_newWndProc );

	mediaLibrary.library  = plugin.hwndLibraryParent;
	mediaLibrary.winamp   = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;

	waServiceFactory *sf = plugin.service->service_getServiceByGuid( languageApiGUID );
	if ( sf )
		WASABI_API_LNG = reinterpret_cast<api_language *>( sf->getInterface() );

	sf = plugin.service->service_getServiceByGuid( AnonymousStatsGUID );
	if ( sf )
		AGAVE_API_STATS = reinterpret_cast<api_stats *>( sf->getInterface() );

	sf = plugin.service->service_getServiceByGuid( applicationApiServiceGuid );
	if ( sf )
		WASABI_API_APP = reinterpret_cast<api_application *>( sf->getInterface() );

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG( plugin.hDllInstance, MlBookmarkLangGUID );

	static wchar_t szDescription[ 256 ];
	StringCchPrintfW( szDescription, ARRAYSIZE( szDescription ), WASABI_API_LNGSTRINGW( IDS_NULLSOFT_BOOKMARKS ), PLUGIN_VERSION );
	plugin.description = (char *)szDescription;

	wchar_t inifile[ MAX_PATH ] = { 0 };
	mediaLibrary.BuildPath( L"Plugins", inifile, MAX_PATH );
	CreateDirectoryW( inifile, NULL );

	mediaLibrary.BuildPath( L"Plugins\\gen_ml.ini", inifile, MAX_PATH );
	g_config = new C_Config( inifile );

	g_context_menus  = WASABI_API_LOADMENU( IDR_MENU1 );
	g_context_menus2 = WASABI_API_LOADMENU( IDR_MENU1 );
	hDragNDropCursor = LoadCursor( GetModuleHandle( L"gen_ml.dll" ), MAKEINTRESOURCE( ML_IDC_DRAGDROP ) );

	NAVINSERTSTRUCT nis = { 0 };
	nis.item.cbSize         = sizeof( NAVITEM );
	nis.item.pszText        = WASABI_API_LNGSTRINGW( IDS_BOOKMARKS );
	nis.item.pszInvariant   = L"Bookmarks";
	nis.item.mask           = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL;
	nis.item.iSelectedImage = nis.item.iImage = mediaLibrary.AddTreeImageBmp( IDB_TREEITEM_BOOKMARKS );

	// map to item id (will probably have to change but is a quick port to support invariant item naming)
	NAVITEM nvItem = { sizeof( NAVITEM ),0,NIMF_ITEMID, };
	nvItem.hItem = MLNavCtrl_InsertItem( plugin.hwndLibraryParent, &nis );
	MLNavItem_GetInfo( plugin.hwndLibraryParent, &nvItem );
	bookmark_treeItem = nvItem.id;

	return 0;
}

void Quit()
{
	delete g_config;
}

extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &plugin;
}