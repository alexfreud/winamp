#define PLUGIN_VERSION L"0.1"

#include <strsafe.h>

#include "Main.h"
#include "../nu/AutoWide.h"

#include "../../General/gen_ml/menu.h"
#include "../../General/gen_ml/ml_ipc_0313.h"

#include "../WAT/WAT.h"

static int  Init();
static void Quit();

extern "C" winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"nullsoft(ml_hotmixradio.dll)",
	Init,
	Quit,
	hotmixradio_pluginMessageProc,
	0,
	0,
	0,
};

int       hotmixradio_treeItem = 0;

HCURSOR   hDragNDropCursor;
C_Config *g_config    = 0;
WNDPROC   waProc      = 0;

// wasabi based services for localisation support
api_language    *WASABI_API_LNG        = 0;
HINSTANCE        WASABI_API_LNG_HINST  = 0;
HINSTANCE        WASABI_API_ORIG_HINST = 0;
api_stats       *AGAVE_API_STATS       = 0;
api_application *WASABI_API_APP        = 0;

static DWORD WINAPI wa_newWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if ( waProc )
		return (DWORD)CallWindowProcW( waProc, hwnd, msg, wParam, lParam );
	else
		return (DWORD)DefWindowProc( hwnd, msg, wParam, lParam );
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
	WASABI_API_START_LANG( plugin.hDllInstance, MlHotmixRadioLangGUID );

	static wchar_t szDescription[ 256 ];
	StringCchPrintfW( szDescription, ARRAYSIZE( szDescription ), WASABI_API_LNGSTRINGW( IDS_NULLSOFT_HOTMIXRADIO ), PLUGIN_VERSION );
	plugin.description = (char *)szDescription;

	wchar_t inifile[ MAX_PATH ] = { 0 };
	mediaLibrary.BuildPath( L"Plugins", inifile, MAX_PATH );
	CreateDirectoryW( inifile, NULL );

	mediaLibrary.BuildPath( L"Plugins\\gen_ml.ini", inifile, MAX_PATH );
	g_config = new C_Config( inifile );

	hDragNDropCursor = LoadCursor( GetModuleHandle( L"gen_ml.dll" ), MAKEINTRESOURCE( ML_IDC_DRAGDROP ) );

	NAVINSERTSTRUCT nis = { 0 };
	nis.item.cbSize         = sizeof( NAVITEM );
	nis.item.pszText        = WASABI_API_LNGSTRINGW( IDS_HOTMIXRADIO );
	nis.item.pszInvariant   = L"Hotmix Radio";
	nis.item.mask           = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL;
	nis.item.iSelectedImage = nis.item.iImage = mediaLibrary.AddTreeImageBmp( IDB_TREEITEM_HOTMIXRADIO );

	// map to item id (will probably have to change but is a quick port to support invariant item naming)
	NAVITEM nvItem = { sizeof( NAVITEM ),0,NIMF_ITEMID, };
	nvItem.hItem   = MLNavCtrl_InsertItem( plugin.hwndLibraryParent, &nis );
	MLNavItem_GetInfo( plugin.hwndLibraryParent, &nvItem );
	hotmixradio_treeItem = nvItem.id;


	return 0;
}

void Quit()
{
	if ( g_config != 0 )
		delete g_config;
}

extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &plugin;
}