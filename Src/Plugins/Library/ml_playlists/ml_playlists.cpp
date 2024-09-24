//#define PLUGIN_NAME "Nullsoft Playlists"
#define PLUGIN_VERSION L"1.78"

#include "main.h"
#include "../../General/gen_ml/ml_ipc_0313.h"
#include "api__ml_playlists.h"
#include "replicant/nu/AutoChar.h"
#include "PlaylistsCOM.h"
#include "api/service/waservicefactory.h"
#include "PlaylistsCB.h"
#include "playlist/plstring.h"

#define PLAYLIST_IMAGE_INDEX		201
#define PLAYLIST_CLOUD_IMAGE_INDEX	202

PlaylistsCOM playlistsCOM;
HMENU wa_playlists_cmdmenu = NULL;
HMENU wa_play_menu = NULL;
INT_PTR playlistsTreeId = 0, playlistsCloudTreeId = 3002;
HNAVITEM playlistItem = 0;
wchar_t g_path[ MAX_PATH ] = { 0 };
HMENU g_context_menus = 0, g_context_menus2 = 0, g_context_menus3 = 0;
int Init();
void Quit();
int( *warand )( void ) = 0;
INT_PTR sendToIgnoreID = 0;
int IPC_LIBRARY_PLAYLISTS_REFRESH = -1, IPC_CLOUD_ENABLED = -1;

extern "C" winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"nullsoft(ml_playlists.dll)",
	Init,
	Quit,
	pluginMessageProc,
	0,
	0,
	0,
};

HCURSOR hDragNDropCursor;
C_Config *g_config = 0;
int imgPL = 0, imgCloudPL = 0;
api_playlistmanager  *AGAVE_API_PLAYLISTMANAGER   = 0;
api_application      *WASABI_API_APP              = 0;
api_playlists        *AGAVE_API_PLAYLISTS         = 0;
api_downloadManager  *WAC_API_DOWNLOADMANAGER     = 0;
api_syscb            *WASABI_API_SYSCB            = 0;
api_explorerfindfile *WASABI_API_EXPLORERFINDFILE = 0;
PlaylistsCB playlistsCB;
// wasabi based services for localisation support
api_language         *WASABI_API_LNG              = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;


template <class api_t>
api_t *GetService( GUID serviceGUID )
{
	waServiceFactory *sf = plugin.service->service_getServiceByGuid( serviceGUID );
	if ( sf )
		return (api_t *)sf->getInterface();
	else
		return 0;
}

inline void ReleaseService( GUID serviceGUID, void *service )
{
	waServiceFactory *sf = plugin.service->service_getServiceByGuid( serviceGUID );
	if ( sf )
		sf->releaseInterface( service );
}

wchar_t prefsname[ 64 ] = { 0 };
extern WORD waMenuID;
int Init()
{
	waMenuID = (WORD)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_REGISTER_LOWORD_COMMAND );

	AGAVE_API_PLAYLISTMANAGER = GetService<api_playlistmanager>( api_playlistmanagerGUID );
	if ( !AGAVE_API_PLAYLISTMANAGER ) // no sense in continuing
		return ML_INIT_FAILURE;

	AGAVE_API_PLAYLISTS = GetService<api_playlists>( api_playlistsGUID );

	if ( !AGAVE_API_PLAYLISTS ) // no sense in continuing
		return ML_INIT_FAILURE;

	WAC_API_DOWNLOADMANAGER   = GetService<api_downloadManager>( DownloadManagerGUID );

	WASABI_API_APP              = GetService<api_application>( applicationApiServiceGuid );
	WASABI_API_SYSCB            = GetService<api_syscb>( syscbApiServiceGuid );

	WASABI_API_SYSCB->syscb_registerCallback( &playlistsCB );

	WASABI_API_EXPLORERFINDFILE = GetService<api_explorerfindfile>( ExplorerFindFileApiGUID );

	//	Hook(plugin.hwndWinampParent);
	warand = ( int( * )( void ) )SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_RANDFUNC );

	// need to get WASABI_API_APP first
	plstring_init();

	// loader so that we can get the localisation service api for use
	WASABI_API_LNG = GetService<api_language>( languageApiGUID );

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG( plugin.hDllInstance, MlPlaylistsLangGUID );

	static wchar_t szDescription[ 256 ];
	swprintf( szDescription, ARRAYSIZE( szDescription ), WASABI_API_LNGSTRINGW( IDS_NULLSOFT_PLAYLISTS ), PLUGIN_VERSION );
	plugin.description = (char *)szDescription;

	mediaLibrary.library  = plugin.hwndLibraryParent;
	mediaLibrary.winamp   = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;

	mediaLibrary.GetWinampIni(); // to prevent a SendMessage hang later

	mediaLibrary.AddDispatch( L"Playlists", &playlistsCOM );
	wchar_t inifile[ MAX_PATH ] = { 0 };
	mediaLibrary.BuildPath( L"Plugins", inifile, MAX_PATH );
	CreateDirectoryW( inifile, NULL );
	mediaLibrary.BuildPath( L"Plugins\\gen_ml.ini", inifile, MAX_PATH );
	g_config = new C_Config( inifile );
	enqueuedef = g_config->ReadInt( L"enqueuedef", 0 );

	// if m3udir has been changed (not the same as inidir) then
	// we attempt to use the same folder for our playlist files
	const wchar_t *m3udir = (wchar_t *)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETM3UDIRECTORYW );
	const wchar_t *inidir = mediaLibrary.GetIniDirectoryW();
	if ( !lstrcmpiW( m3udir, inidir ) )
		mediaLibrary.BuildPath( L"Plugins\\ml\\playlists", g_path, MAX_PATH );
	else
		lstrcpynW( g_path, m3udir, MAX_PATH );

	CreateDirectoryW( g_path, NULL );

	g_context_menus  = WASABI_API_LOADMENU( IDR_MENU1 );
	g_context_menus2 = WASABI_API_LOADMENU( IDR_MENU1 );	// this and next one are used for the combined buttons
	g_context_menus3 = WASABI_API_LOADMENU( IDR_MENU1 );	// so we don't have to mess around the translators etc

	HookMediaLibrary();

	hDragNDropCursor = LoadCursor( GetModuleHandle( L"gen_ml.dll" ), MAKEINTRESOURCE( ML_IDC_DRAGDROP ) );

	HMENU wa_plcontext_menu = GetSubMenu( (HMENU)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, -1, IPC_GET_HMENU ), 2 );
	if ( wa_plcontext_menu )
	{
		wa_playlists_cmdmenu = GetSubMenu( wa_plcontext_menu, 4 );
		if ( wa_playlists_cmdmenu )
		{
			MENUITEMINFO i = { sizeof( i ), MIIM_TYPE, MFT_SEPARATOR, 0, 0 };
			InsertMenuItem( wa_playlists_cmdmenu, 9, TRUE, &i );
			MENUITEMINFO j = { sizeof( i ), MIIM_ID | MIIM_STATE | MIIM_TYPE, MFT_STRING, 0, WINAMP_MANAGEPLAYLISTS };
			j.dwTypeData = WASABI_API_LNGSTRINGW( IDS_MANAGE_PLAYLISTS );
			InsertMenuItem( wa_playlists_cmdmenu, 10, TRUE, &j );
		}
	}

	IPC_CLOUD_ENABLED             = SendMessage( plugin.hwndWinampParent, WM_WA_IPC, ( WPARAM ) & "WinampCloudEnabled", IPC_REGISTER_WINAMP_IPCMESSAGE );
	IPC_LIBRARY_PLAYLISTS_REFRESH = SendMessage( plugin.hwndWinampParent, WM_WA_IPC, ( WPARAM ) & "ml_playlist_refresh", IPC_REGISTER_WINAMP_IPCMESSAGE );
	wa_play_menu                  = GetSubMenu( (HMENU)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_HMENU ), 2 );

	// lets extend menu that called on button press
	int   IPC_GET_ML_HMENU = (int)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, ( WPARAM ) & "LibraryGetHmenu", IPC_REGISTER_WINAMP_IPCMESSAGE );
	HMENU context_menu     = (HMENU)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_ML_HMENU );

	if ( context_menu )
	{
		HMENU btnMenu = GetSubMenu( context_menu, 0 );
		if ( btnMenu )
		{
			MENUITEMINFO mii = { sizeof( MENUITEMINFO ) };

			mii.fMask  = MIIM_FTYPE;
			mii.fType  = MFT_SEPARATOR;
			mii.fState = MFS_ENABLED;
			InsertMenuItem( btnMenu, 0, TRUE, &mii );

			mii.fMask = MIIM_TYPE | MIIM_ID;
			mii.fType = MFT_STRING;

			mii.dwTypeData = WASABI_API_LNGSTRINGW( IDS_MANAGE_PLAYLISTS );
			mii.cch        = (unsigned int)lstrlen( mii.dwTypeData );
			mii.wID        = WINAMP_MANAGEPLAYLISTS;
			InsertMenuItem( btnMenu, 0, TRUE, &mii );

			mii.dwTypeData = WASABI_API_LNGSTRINGW( IDS_NEW_PLAYLIST );
			mii.cch        = (unsigned int)lstrlen( mii.dwTypeData );
			mii.wID        = ID_DOSHITMENU_ADDNEWPLAYLIST;
			InsertMenuItem( btnMenu, 0, TRUE, &mii );
		}
	}

	imgPL = mediaLibrary.AddTreeImage( IDB_TREEITEM_PLAYLIST, PLAYLIST_IMAGE_INDEX, (BMPFILTERPROC)FILTER_DEFAULT1 );
	imgCloudPL = mediaLibrary.AddTreeImage( IDB_TREEITEM_CLOUD_PLAYLIST, PLAYLIST_CLOUD_IMAGE_INDEX, (BMPFILTERPROC)FILTER_DEFAULT1 );

	NAVINSERTSTRUCT nis = { 0 };
	nis.item.cbSize       = sizeof( NAVITEM );
	nis.item.pszText      = WASABI_API_LNGSTRINGW_BUF( IDS_PLAYLISTS, prefsname, 64 );
	nis.item.pszInvariant = L"Playlists";
	nis.item.id           = playlistsTreeId = 3001; // for backwards compatability
	nis.item.style        = NIS_HASCHILDREN;
	nis.item.mask         = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_ITEMID | NIMF_STYLE;

	playlistItem = MLNavCtrl_InsertItem( plugin.hwndLibraryParent, &nis );

	LoadPlaylists();

	Hook( plugin.hwndWinampParent );
	HookPlaylistEditor();

	return ML_INIT_SUCCESS;
}

void Quit()
{
	AGAVE_API_PLAYLISTS->Flush();
	WASABI_API_SYSCB->syscb_deregisterCallback( &playlistsCB );

	ReleaseService( api_playlistmanagerGUID,   AGAVE_API_PLAYLISTMANAGER );
	ReleaseService( api_playlistsGUID,         AGAVE_API_PLAYLISTS );
	ReleaseService( DownloadManagerGUID,       WAC_API_DOWNLOADMANAGER  );
	ReleaseService( applicationApiServiceGuid, WASABI_API_APP );
	ReleaseService( syscbApiServiceGuid,       WASABI_API_SYSCB );
	ReleaseService( ExplorerFindFileApiGUID,   WASABI_API_EXPLORERFINDFILE );

	UnhookPlaylistEditor();
	UnhookMediaLibrary();

	delete g_config;
}

extern "C" __declspec( dllexport ) winampMediaLibraryPlugin * winampGetMediaLibraryPlugin()
{
	return &plugin;
}