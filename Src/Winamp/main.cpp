/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename:  main.cpp
 ** Project: Winamp
 ** Description: Winamp initialization code
 ** Author: Justin Frankel
 ** Created: April 1997
 **/

#include "main.h"
#include <windowsx.h>

#include "../Agave/Language/lang.h"
#include <stdarg.h>
#include "vis.h"
#include "fft.h"
#include "gen.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "menuv5.h"
#include "../Plugins/General/gen_ml/ml.h"
#include "wa_dlg.h"
#include "strutil.h"
#include "./setup/setupfactory.h"
#include "./commandLink.h"
#include "AppRefCount.h"
#include <unknwn.h>
#include <shlwapi.h>
#include <shobjidl.h>

#include "WAT/WAT.h"

#ifndef WM_DWMSENDICONICTHUMBNAIL
#define WM_DWMSENDICONICTHUMBNAIL 0x0323
#endif
#include "Agave/Language/api_language.h"

#ifndef WM_DWMSENDICONICLIVEPREVIEWBITMAP
#define WM_DWMSENDICONICLIVEPREVIEWBITMAP 0x0326
#endif

#ifndef THBN_CLICKED
#define THBN_CLICKED 0x1800
#endif

typedef HRESULT( WINAPI *CHANGEWINDOWMESSAGEFILTER )( UINT message, DWORD dwFlag );
static HMODULE user32Lib = 0;
static CHANGEWINDOWMESSAGEFILTER changeWMFilter;
static BOOL changeWMLoadTried = FALSE;

//#define BENSKI_TEST_WM_PRINTCLIENT
static UINT WM_TASKBARCREATED;
static UINT WM_TASKBARBUTTONCREATED;

LARGE_INTEGER freq;
UINT g_scrollMsg;
UINT songChangeBroadcastMessage = 0;
int g_noreg;
int disable_skin_borders = 0;
int no_notify_play = 0;
int last_no_notify_play = 0;
int main_delta_carryover = 0;
int g_restartonquit = 0;
char g_audiocdletter[ 4 ] = { 0 };
int g_audiocdletters = 0;
const char app_name[] = "Winamp", app_version[] = APP_VERSION, app_version_string[] = APP_VERSION_STRING; // application name and version strings
int g_fullstop;
char *app_date = __DATE__;
int g_stopaftercur;
int is_install;
HWND hTooltipWindow, hEQTooltipWindow, hVideoTooltipWindow, hPLTooltipWindow;
HWND hMainWindow = NULL;			// main window
HWND hEQWindow, hPLWindow, /*hMBWindow, */hVideoWindow, hExternalVisWindow = NULL;

HWND g_dialog_box_parent = NULL; // used by IPC_SETDIALOGBOXPARENT (FG, 5/19/03)
HINSTANCE language_pack_instance;
HINSTANCE hMainInstance;	// program instance
HANDLE hMainThread; // main thread handle
DWORD mainThreadId; // main thread ID
HMENU main_menu = 0, top_menu = 0, g_submenus_bookmarks1 = 0,
g_submenus_bookmarks2 = 0, g_submenus_skins1 = 0,
g_submenus_skins2 = 0, g_submenus_vis = 0,
g_submenus_options = 0, g_submenus_lang = 0,
g_submenus_play = 0;

int     g_submenus_lang_id = 0;
int     g_video_numaudiotracks = 1;
int     g_video_curaudiotrack = 0;

int     bStartPlaying = 0;
int     paused = 0;
int     playing = 0;
wchar_t caption[ CAPTION_SIZE ] = { 0 };   // current program caption
wchar_t FileName[ FILENAME_SIZE ] = { 0 };   // current file name
wchar_t FileTitle[ FILETITLE_SIZE ] = { 0 };   // current file title
wchar_t FileTitleNum[ FILETITLE_SIZE ] = { 0 };   // current file title + track position
int     eggstat = 0;       // used for easter eggs
int     g_srate, g_brate, g_nch, g_srate_exact;
int     last_brate = -1;
int     g_need_titleupd = 0;
int     g_need_infoupd = 0;
int     g_SkinTop, g_BookmarkTop, g_LangTop;
int     g_mm_optionsbase_adj = 0;       //used by IPC_ADJUST_OPTIONSMENUPOS
int     g_mm_ffwindowsbase_adj = 0;       //used by IPC_ADJUST_FFWINDOWSMENUPOS
int     g_mm_ffoptionsbase_adj = 0;       //used by IPC_ADJUST_FFOPTIONSMENUPOS
int     g_has_video_plugin = 0;
int     g_no_video_loaded = 0;       //filled in by in_init

char    playlist_custom_font[ 128 ] = { 0 };
wchar_t playlist_custom_fontW[ 128 ] = { 0 };
int     config_custom_plfont = 1;
int     disable_skin_cursors = 0;
int     vis_fullscreen = 0;

struct ITaskbarList3 *pTaskbar3 = NULL;

static LRESULT Main_OnSysCommand( HWND hwnd, UINT cmd, int x, int y );

HWND find_otherwinamp( wchar_t * );

#undef HANDLE_WM_NCACTIVATE
#define HANDLE_WM_NCACTIVATE(hwnd, wParam, lParam, fn) \
	(LRESULT)(DWORD)(BOOL)(fn)((hwnd), (BOOL)(wParam), (HWND)(lParam), 0L)

int stat_isit = 1; // used for faster version checkig
wchar_t szAppName[ 64 ] = { 0 }; //	window class name, generated on the fly.

EXTERN_C BOOL eggTyping = FALSE;
static char eggstr[] = "NULLSOFT";

UINT USER_CONSENT_EVENT_ID = 123456;

int g_exit_disabled = 0;
int g_safeMode = 0;
HANDLE g_hEventRunning;
int bNoHwndOther = 0;

static void CreateEQPresets()
{
	if ( !PathFileExistsW( EQDIR1 ) )
	{
		int x;
		struct
		{
			char *s;
			unsigned char tab[ 10 ];
		}
		eqsets[] =
		{
			{"Classical", {31, 31, 31, 31, 31, 31, 44, 44, 44, 48}},
			{"Club", {31, 31, 26, 22, 22, 22, 26, 31, 31, 31}},
			{"Dance", {16, 20, 28, 32, 32, 42, 44, 44, 32, 32}},
			{"Flat", {31, 31, 31, 31, 31, 31, 31, 31, 31, 31}},
			{"Laptop speakers/headphones", {24, 14, 23, 38, 36, 29, 24, 16, 11, 8}},
			{"Large hall", {15, 15, 22, 22, 31, 40, 40, 40, 31, 31}},
			{"Party", {20, 20, 31, 31, 31, 31, 31, 31, 20, 20}},
			{"Pop", {35, 24, 20, 19, 23, 34, 36, 36, 35, 35}},
			{"Reggae", {31, 31, 33, 42, 31, 21, 21, 31, 31, 31}},
			{"Rock", {19, 24, 41, 45, 38, 25, 17, 14, 14, 14}},
			{"Soft", {24, 29, 34, 36, 34, 25, 18, 16, 14, 12}},
			{"Ska", {36, 40, 39, 33, 25, 22, 17, 16, 14, 16}},
			{"Full Bass", {16, 16, 16, 22, 29, 39, 46, 49, 50, 50}},
			{"Soft Rock", {25, 25, 28, 33, 39, 41, 38, 33, 27, 17}},
			{"Full Treble", {48, 48, 48, 39, 27, 14, 6, 6, 6, 4}},
			{"Full Bass & Treble", {20, 22, 31, 44, 40, 29, 18, 14, 12, 12}},
			{"Live", {40, 31, 25, 23, 22, 22, 25, 27, 27, 28}},
			{"Techno", {19, 22, 31, 41, 40, 31, 19, 16, 16, 17}},
		};

		for ( x = 0; x < sizeof( eqsets ) / sizeof( eqsets[ 0 ] ); x++ )
			writeEQfile_init( EQDIR1, eqsets[ x ].s, eqsets[ x ].tab );
	}
}

void BuildAppName()
{
	StringCchCopyW( szAppName, 64, L"Winamp v1.x" );
	StringCchPrintfW( caption, CAPTION_SIZE, L"%S %S", app_name, app_version_string );
}

static void CALLBACK DisplayUserConsentMessageBox( HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD time )
{
	KillTimer( hMainWindow, USER_CONSENT_EVENT_ID );
	if ( config_user_consent_join_channels != -1 )
	{
		return;
	}

	wchar_t titleStr[ 32 ] = { 0 };
	int msgboxID = MessageBoxW(
		NULL,
		WASABI_API_LNGSTRINGW( IDS_RC_CHANNEL_MESSAGE ),
		WASABI_API_LNGSTRINGW_BUF( IDS_RC_CHANNEL_TITLE, titleStr, 32 ),
		MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON1
	);

	switch ( msgboxID )
	{
		case IDYES:
			config_user_consent_join_channels = 1;
			config_newverchk_rc = 1;
			config_newverchk = 1;
			break;
		case IDNO:
			config_user_consent_join_channels = 0;
			config_newverchk_rc = 0;
			config_newverchk = 1;
			break;
	}
}

// creates (but does not show) main window
int CreateMainWindow()
{
	if ( !IsWindow( hMainWindow ) )
	{
		WNDCLASSW wcW = { 0 };

		wcW.style = CS_DBLCLKS;
		wcW.lpfnWndProc = Main_WndProc;
		wcW.hInstance = hMainInstance;
		wcW.hIcon = LoadIconW( hMainInstance, MAKEINTRESOURCE( ICON_XP ) );
		wcW.hCursor = NULL;
		wcW.lpszClassName = szAppName;

		if ( !RegisterClassW( &wcW ) )
			return 0;

		if ( !CreateWindowExW( WS_EX_ACCEPTFILES, szAppName, L"Winamp", WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CAPTION,
			 config_wx, config_wy, 0, 0,        // WM_CREATE will size it
			 NULL, NULL, hMainInstance, NULL ) )
		{
			return 0;
		}
	}
	return 1;
}

wchar_t *getGUIDstr( const GUID guid, wchar_t *target )
{
	StringCchPrintfW( target, 40, L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\0",
					  (int)guid.Data1, (int)guid.Data2, (int)guid.Data3,
					  (int)guid.Data4[ 0 ], (int)guid.Data4[ 1 ], (int)guid.Data4[ 2 ], (int)guid.Data4[ 3 ],
					  (int)guid.Data4[ 4 ], (int)guid.Data4[ 5 ], (int)guid.Data4[ 6 ], (int)guid.Data4[ 7 ] );
	return target;
}

BOOL parseMessageCommands( HWND hwnd_other, int bCommand, int bCmdParam )
{
	if ( LOWORD( bCommand ) ) // attempt to send the action to it
	{
		if ( HIWORD( bCommand ) == 1 )
		{
			SendMessageW( hwnd_other, WM_WA_IPC, bCmdParam, MAKELPARAM( LOWORD( bCommand ), 0 ) );
			return TRUE;
		}
		else if ( HIWORD( bCommand ) == 2 )
		{
			// these need some additional processing which is easier to do once we're loaded
			if ( LOWORD( bCommand ) == EQ_PANLEFT || LOWORD( bCommand ) == EQ_PANRIGHT ||
				 LOWORD( bCommand ) == IPC_SETPANNING )
			{
				int pan = IPC_GETPANNING( hwnd_other );
				if ( LOWORD( bCommand ) == EQ_PANLEFT ) pan -= 12;
				else if ( LOWORD( bCommand ) == EQ_PANRIGHT ) pan += 12;
				else pan = bCmdParam;

				if ( pan < -127 ) pan = -127;
				if ( pan > 127 ) pan = 127;

				SendMessageW( hwnd_other, WM_WA_IPC, pan, IPC_SETPANNING );
			}
			else if ( LOWORD( bCommand ) == IPC_ISPLAYING )
			{
				int command = SendMessageW( hwnd_other, WM_WA_IPC, 0, IPC_ISPLAYING );
				switch ( command )
				{
					case 1:		// playing so need to pause
						command = WINAMP_BUTTON3;
						break;
					default:	// stopped so start playing
						command = WINAMP_BUTTON2;
						break;
				}
				SendMessageW( hwnd_other, WM_COMMAND, command, 0 );
			}
			return TRUE;
		}
		else
		{
			SendMessageW( hwnd_other, WM_COMMAND, bCommand, 0 );

			if ( LOWORD( bCommand ) == WINAMP_JUMPFILE )
			{
				// TODO need to make this locale independant...
				HWND jumpWnd = FindWindowW( NULL, L"Jump to file" );
				if ( IsWindow( jumpWnd ) )
				{
					SetForegroundWindow( jumpWnd );
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}

static int PassToOtherWinamp( wchar_t *lpszCmdParam, HWND hwnd_other, int bAdd, int bBookmark, int bHandle, int bCommand, int bCmdParam )
{
	// if we have command line params, pass to other winamp window
	if ( lpszCmdParam && *lpszCmdParam )
	{
		int skinExit = 0;
		int bC = 0;
		HANDLE hSem = NULL;
		HINSTANCE existingWLZ = 0, templng = 0;
		DWORD_PTR vn = 0;

		// check if we're using a language pack with the already open winamp process
		// and if so then we're going to use the winamp.lng from it on the messagebox
		// only if we had a success and the other winamp returned the correct value
		// within the timeout period (can't be having it lock up so revert if needed)
		if ( SendMessageTimeout( hwnd_other, WM_WA_IPC, 1, IPC_GETLANGUAGEPACKINSTANCE, SMTO_NORMAL, 5000, &vn ) && !vn )
		{
			DWORD processid = 0;
			HANDLE hwaProcess = NULL;
			SIZE_T bread = 0;
			wchar_t lng_path_copy[ MAX_PATH ] = { 0 }, dirmask[ MAX_PATH ] = { 0 }, gs[ 40 ] = { 0 };
			WIN32_FIND_DATAW d = { 0 };

			GetWindowThreadProcessId( hwnd_other, &processid );
			hwaProcess = OpenProcess( PROCESS_VM_READ, FALSE, processid );
			ReadProcessMemory( hwaProcess, (wchar_t *)SendMessageW( hwnd_other, WM_WA_IPC, 3, IPC_GETLANGUAGEPACKINSTANCE ), lng_path_copy, MAX_PATH, &bread );
			CloseHandle( hwaProcess );

			getGUIDstr( WinampLangGUID, gs );
			PathCombineW( dirmask, lng_path_copy, L"*.lng" );
			HANDLE h = FindFirstFileW( dirmask, &d );
			if ( h != INVALID_HANDLE_VALUE )
			{
				do
				{
					PathCombineW( dirmask, lng_path_copy, d.cFileName );
					templng = LoadLibraryExW( dirmask, NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE | LOAD_LIBRARY_AS_DATAFILE );
					if ( !templng ) templng = LoadLibraryW( dirmask );
					if ( templng )
					{
						wchar_t s[ 39 ] = { 0 };
						if ( LoadStringW( templng, LANG_DLL_GUID_STRING_ID, s, 39 ) )
						{
							if ( !_wcsnicmp( gs, s, 38 ) )
							{
								existingWLZ = Lang_FakeWinampLangHInst( templng );
							}
							else
								FreeLibrary( templng );
						}
						else
							FreeLibrary( templng );
					}
				} while ( FindNextFileW( h, &d ) );
				FindClose( h );
			}
		}

		lpszCmdParam = CheckSkin( lpszCmdParam, hwnd_other, &skinExit );
		if ( skinExit )
		{
			// restore the language pack settings now that we've done the override and clean up as needed
			if ( existingWLZ )
			{
				Lang_FakeWinampLangHInst( existingWLZ );
				FreeLibrary( templng );
			}
			return TRUE;
		}

		skinExit = 0;
		lpszCmdParam = CheckLang( lpszCmdParam, hwnd_other, &skinExit );
		// restore the language pack settings now that we've done the override and clean up as needed
		if ( existingWLZ )
		{
			Lang_FakeWinampLangHInst( existingWLZ );
			FreeLibrary( templng );
		}

		if ( skinExit )
			return TRUE;

		hSem = CreateSemaphoreA( 0, 0, 65535, "WinampExplorerHack1" );

		if ( hSem && GetLastError() != ERROR_ALREADY_EXISTS )
		{
			bC = 1;
			if ( !bAdd && !bBookmark && !bHandle )
			{
				SendMessageW( hwnd_other, WM_WA_IPC, 0, IPC_DELETE_INT );
			}
		}
		if ( hSem )
		{
			ReleaseSemaphore( hSem, 1, NULL );
			if ( bBookmark )
			{
				static wchar_t tmp[ MAX_PATH ];
				StringCchPrintfW( tmp, MAX_PATH, L"/BOOKMARK %s", lpszCmdParam );
				lpszCmdParam = tmp;
			}
			else if ( bHandle )
			{
				static wchar_t tmp[ MAX_PATH ];
				StringCchPrintfW( tmp, MAX_PATH, L"/HANDLE %s", lpszCmdParam );
				lpszCmdParam = tmp;
			}
			parseCmdLine( lpszCmdParam, hwnd_other );

			WaitForSingleObject( hSem, 5000 );
			if ( bC )
			{
				int n = 500;
				if ( !bAdd && !bBookmark && !bHandle ) SendMessageW( hwnd_other, WM_WA_IPC, 0, IPC_STARTPLAY_INT );
				Sleep( 200 );
				for ( ;;)
				{
					if ( WaitForSingleObject( hSem, 100 ) == WAIT_TIMEOUT )
					{
						if ( WaitForSingleObject( hSem, 900 ) == WAIT_TIMEOUT )
						{
							break;
						}
						else
						{
							ReleaseSemaphore( hSem, 1, NULL );
							n--;
						}
					}
					else
					{
						ReleaseSemaphore( hSem, 1, NULL );
						Sleep( 100 );
						n--;
					}
				}
			}
			CloseHandle( hSem );
		}
	}
	else
	{
		if ( !parseMessageCommands( hwnd_other, bCommand, bCmdParam ) )
		{
			ShowWindow( hwnd_other, SW_RESTORE );
			SetForegroundWindow( hwnd_other );
		}
	}
	return TRUE;
}

DWORD CALLBACK MainThread( LPVOID param );
extern wchar_t vidoutbuf_save[ 1024 ];
static LPWSTR lpszCmdParam = 0;
static int bAdd = 0, bBookmark = 0,
bHandle = 0, bCommand = 0,
bCmdParam = 0, bAllowCompat = 0;

void ShowSafeModeMessage( int mode )
{
	if ( g_safeMode && ( g_safeMode != 3 ) )
	{
		wchar_t title[ 256 ] = { 0 }, message[ 512 ] = { 0 };
		MSGBOXPARAMSW msgbx = { sizeof( MSGBOXPARAMSW ),0 };
		if ( !mode )
		{
			msgbx.lpszText = getStringW( ( g_safeMode == 2 ? IDS_SAFE_MODE_ALL : IDS_SAFE_MODE_NORMAL ), message, 512 );
			msgbx.lpszCaption = getStringW( IDS_START_SAFE_MODE, title, 256 );
		}
		else
		{
			msgbx.lpszText = getStringW( IDS_FAILED_SAFE_MODE_MSG, message, 512 );
			msgbx.lpszCaption = getStringW( IDS_FAILED_SAFE_MODE, title, 256 );
		}
		msgbx.lpszIcon = MAKEINTRESOURCEW( 102 );
		msgbx.hInstance = hMainInstance;
		msgbx.dwStyle = MB_USERICON;
		MessageBoxIndirectW( &msgbx );
	}
}

#ifdef BETA
time_t inline get_compile_time( char const *time )
{
	char s_month[ 5 ] = { 0 };
	int day = 0, year = 0;
	struct tm t = { 0 };
	static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

	sscanf( time, "%s %d %d", s_month, &day, &year );

	t.tm_mon = ( ( strstr( month_names, s_month ) - month_names ) / 3 );
	t.tm_mday = day;
	t.tm_year = year - 1900;
	t.tm_isdst = -1;

	return mktime( &t );
}
#endif

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR unused /*lpszCmdParam*/, int nCmdShow )
{
	INITCOMMONCONTROLSEX icex = { sizeof( icex ), ICC_WIN95_CLASSES | ICC_DATE_CLASSES };
	InitCommonControlsEx( &icex );
	QueryPerformanceFrequency( &freq );

#if 0
#ifdef BETA
	// gives ~4 weeks from a build compile to when it'll show this (should be enough time)
	time_t now = time( 0 ), compile = get_compile_time( app_date );
	struct tm *tn = localtime( &now );
	tn->tm_sec = tn->tm_min = tn->tm_hour = 0;
	now = mktime( tn );

	if ( ( now - compile ) >= 2678400 )
	{
		/* Skip the executable name in the commandline */
		/* and check for /UNREG which we will allow so */
		/* an expired beta can be uninstalled properly */
		lpszCmdParam = GetCommandLineW();
		lpszCmdParam = FindNextCommand( lpszCmdParam );
		ParseParametersExpired( lpszCmdParam );

		MSGBOXPARAMSW msgbx = {
			sizeof( MSGBOXPARAMSW ),
			0,
			GetModuleHandle( NULL ),
			L"This beta version of Winamp is now over 4 weeks old.\n\n"
			L"Please update to the latest Winamp version available.",
			L"Winamp Beta Expired",
			MB_USERICON,
			MAKEINTRESOURCEW( 102 ),
			0, 0, 0
		};
		MessageBoxIndirectW( &msgbx );
		ShellExecuteW( NULL, L"open", L"http://www.winamp.com/media-player", NULL, NULL, SW_SHOWNORMAL );
		return 0;
	}
#endif
#endif

	DWORD threadId = 0, res = 0;
	HANDLE mainThread = nullptr;
	int cmdShow = nCmdShow;

	/*void *refCounter = */InitAppRefCounterObject( GetCurrentThreadId() );
	//SHSetInstanceExplorer((IUnknown *)refCounter);

	SetErrorMode( SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS );

	// don't load from working directory !!!!!
	// requires XP SP3+
	SetDllDirectoryW( L"" );

	GetModuleFileNameW( NULL, SHAREDDIR, MAX_PATH );
	PathRemoveFileSpecW( SHAREDDIR );
	PathAppendW( SHAREDDIR, L"Shared" );
	// load supporting dlls from the Winamp\Shared directory
	SetDllDirectoryW( SHAREDDIR );

	hMainInstance = hInstance;

	/* Skip the executable name in the commandline */
	lpszCmdParam = GetCommandLineW();
	lpszCmdParam = FindNextCommand( lpszCmdParam );

	BuildAppName();

	init_config();
	LoadPathsIni();
	lpszCmdParam = ParseParameters( lpszCmdParam, &bAdd, &bBookmark, &bHandle, &cmdShow, &bCommand, &bCmdParam, &bAllowCompat );

	setup_config();

	// ensure we've got things set up as needed for safe mode being always on so we
	// don't incorrectly prompt about entering it from using IPC_RESTARTSAFEWINAMP.
	int mode = _r_i( "allowcompat", 0 );
	if ( mode ) bAllowCompat = 1;

	if ( !bAllowCompat && read_compatmode() )
	{
		MSGBOXPARAMSW msgbx = {
			sizeof( MSGBOXPARAMSW ),
			0,
			GetModuleHandle( NULL ),
			L"Winamp appears to have been started with Windows program compatibility mode enabled.\n\n"
			L"This is not a recommended way to run Winamp as it can often cause problems with how Winamp works e.g. causing it to randomly crash."
			L"\n\n\nAre you sure you want to continue to run Winamp like this?\n\n\n"
			L"If you choose 'No', you can disable this by right-clicking winamp.exe, choosing 'Properties' and selecting the 'Compatibility' tab, "
			L"followed by unchecking 'Run this program in compatibility mode for:' and run Winamp again.",
			L"Winamp",
			MB_USERICON | MB_YESNO | MB_DEFBUTTON2,
			MAKEINTRESOURCEW( 102 ),
			0, 0, 0
		};
		if ( MessageBoxIndirectW( &msgbx ) == IDNO )
		{
			return 0;
		}
	}


	CoInitializeEx( 0, COINIT_MULTITHREADED );

	// ensure we've got things set up as needed for safe mode being always on so we
	// don't incorrectly prompt about entering it from using IPC_RESTARTSAFEWINAMP.
	mode = _r_i( "safemode", 0 );
	if ( mode ) g_safeMode = 3;

	if ( 0 == ( 128 & is_install ) )
	{
		HWND hwnd_other = find_otherwinamp( lpszCmdParam );
		if ( IsWindow( hwnd_other ) )
		{
			// unable to start safe mode so inform the user
			ShowSafeModeMessage( 1 );
			int x = PassToOtherWinamp( lpszCmdParam, hwnd_other, bAdd, bBookmark, bHandle, bCommand, bCmdParam );
			CoUninitialize();
			return x;
		}
	}

	// unable to start safe mode so inform the user
	ShowSafeModeMessage( 0 );

	mainThread = CreateThread( 0, 0, MainThread, (LPVOID)cmdShow, 0, &threadId );

	while ( !AppRefCount_CanQuit() )
	{
		DWORD dwStatus = MsgWaitForMultipleObjectsEx( 1, &mainThread, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE );
		if ( dwStatus == WAIT_OBJECT_0 + 1 )
		{
			MSG msg;
			while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
			{
				if ( msg.message == WM_QUIT )
					return msg.wParam;
				DispatchMessage( &msg );
			}
		}
		else if ( dwStatus == WAIT_OBJECT_0 )
		{
			GetExitCodeThread( mainThread, &res );
			CloseHandle( mainThread );
			AppRefCount_Release();
		}
	}
	return 0;
}

static BOOL LoadWMFilter()
{
	if ( !changeWMLoadTried )
	{
		user32Lib = LoadLibraryA( "user32.dll" );
		if ( user32Lib )
			changeWMFilter = (CHANGEWINDOWMESSAGEFILTER)GetProcAddress( user32Lib, "ChangeWindowMessageFilter" );

		changeWMLoadTried = TRUE;
	}

	return user32Lib && changeWMFilter;
}

VOID CALLBACK PrefsShowProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
	KillTimer( hwnd, idEvent );

	if ( IsWindow( prefs_hwnd ) )
		SetForegroundWindow( prefs_hwnd );
}

void load_gen_crasher()
{
	// this will load gen_crasher.dll as applicable (included in beta builds but not release mode by default)
	// with extra checks to ensure it's 'valid' (as can be) so we could drop it into release builds as needed
	wchar_t crasherDll[ MAX_PATH ] = { 0 };
	PathCombineW( crasherDll, PLUGINDIR, L"gen_crasher.dll" );
	HMODULE hm = LoadLibraryW( crasherDll );
	if ( hm )
	{
		int( __cdecl * StartHandler )( wchar_t *iniPath ) = NULL;
		*(FARPROC *)&StartHandler = GetProcAddress( hm, "StartHandler" );
		if ( StartHandler )
		{
			wchar_t iniPath[ MAX_PATH ] = { 0 };
			if ( SUCCEEDED( StringCchPrintfW( iniPath, MAX_PATH, L"%s\\Plugins", CONFIGDIR ) ) )
			{
				winampGeneralPurposePluginGetter pr = (winampGeneralPurposePluginGetter)GetProcAddress( hm, "winampGetGeneralPurposePlugin" );
				if ( pr )
				{
					winampGeneralPurposePlugin *plugin = pr();
					if ( plugin && plugin->version == GPPHDR_VER_U )
					{
						char desc[ 128 ] = { 0 };
						lstrcpynA( desc, plugin->description, sizeof( desc ) );
						if ( desc[ 0 ] && !memcmp( desc, "nullsoft(", 9 ) )
						{
							char *p = strrchr( desc, ')' );
							if ( p )
							{
								*p = 0;
								if ( !_wcsicmp( L"gen_crasher.dll", AutoWide( desc + 9 ) ) )
								{
									StartHandler( iniPath );
								}
								else
									FreeLibrary( hm );
							}
							else
								FreeLibrary( hm );
						}
						else
							FreeLibrary( hm );
					}
					else
						FreeLibrary( hm );
				}
				else
					FreeLibrary( hm );
			}
			else
				FreeLibrary( hm );
		}
		else
			FreeLibrary( hm );
	}
}

DWORD CALLBACK MainThread( LPVOID param )
{
	language_pack_instance = hMainInstance;

	playlistStr[ 0 ] = 0;
	playlistStr[ 18 ] = 0; // keep the last byte null terminated (and don't overwrite) so we can be smoewhat thread-safe (may have junk data, but it won't read outside the array)

	vidoutbuf_save[ 0 ] = 0;
	vidoutbuf_save[ 1023 ] = 0; // keep the last byte null terminated (and don't overwrite) so we can be smoewhat thread-safe (may have junk data, but it won't read outside the array)

	mainThreadId = GetCurrentThreadId();
	DuplicateHandle( GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, 0, FALSE, DUPLICATE_SAME_ACCESS );

	InitializeCriticalSection( &embedcs );

	CoInitialize( 0 );
	Wasabi_Load();
	plstring_init();
	/*Browser_Create();*/


	// check if it's gen_crasher.dll that's being removed and skip loading it so uninstall will work
	bool skip_crasher = false;
	wchar_t buf[ 1024 ] = { 0 };
	_r_sW( "remove_genplug", buf, 1024 );
	if ( buf[ 0 ] )
		skip_crasher = !wcsicmp( PathFindFileNameW( buf ), L"gen_crasher.dll" );

	if ( !skip_crasher )
		load_gen_crasher();


	if ( ( 128 & is_install ) || ( !g_noreg && GetPrivateProfileIntW( L"WinampReg", L"NeedReg", 1, INI_FILE ) ) )
	{
		is_install = 128; //  nothing else
		Setup_RegisterService();
	}

	draw_firstinit();

	WM_TASKBARCREATED = RegisterWindowMessageA( "TaskbarCreated" );
	g_scrollMsg = RegisterWindowMessageA( "MSWHEEL_ROLLMSG" );
	WM_TASKBARBUTTONCREATED = RegisterWindowMessageW( L"TaskbarButtonCreated" );

	if ( LoadWMFilter() )
	{
		changeWMFilter( WM_TASKBARBUTTONCREATED, 1/*MSGFLT_ADD*/ );
		changeWMFilter( WM_DWMSENDICONICTHUMBNAIL, 1/*MSGFLT_ADD*/ );
		changeWMFilter( WM_DWMSENDICONICLIVEPREVIEWBITMAP, 1/*MSGFLT_ADD*/ );
		changeWMFilter( WM_COMMAND, 1/*MSGFLT_ADD*/ );  //for thumbnail toolbar buttons
	}

	CommandLink_RegisterClass( hMainInstance );

	Skin_CleanupAfterCrash();
	Lang_CleanupAfterCrash();

	{
		int langExit = 0;
		lpszCmdParam = CheckLang( lpszCmdParam, 0, &langExit );
		if ( langExit )
		{
			Lang_EndLangSupport();
			Lang_CleanupZip();
			return TRUE;
		}
	}

	if ( !g_safeMode )
	{
		language_pack_instance = Lang_InitLangSupport( hMainInstance, WinampLangGUID );
		Lang_FollowUserDecimalLocale();
	}

	if ( bBookmark )
	{
		w5s_init();

		if ( !in_init() )
		{
			w5s_deinit();
			Wasabi_Unload();
			RemoveRegistrar();
			CoUninitialize();
			ExitProcess( 0 );
		}

		Bookmark_AddCommandline( lpszCmdParam );
		in_deinit();
		w5s_deinit();
		Wasabi_Unload();
		RemoveRegistrar();
		CoUninitialize();
		ExitProcess( 0 );
	}

	{
		int skinExit = 0;
		lpszCmdParam = CheckSkin( lpszCmdParam, 0, &skinExit );
		if ( skinExit )
		{
			return TRUE;
		}
	}

	{
		// remove general purpose plug-in (if set)
		wchar_t buf[ 1024 ] = { 0 };
		_r_sW( "remove_genplug", buf, 1024 );
		if ( buf[ 0 ] )
		{
			IFileTypeRegistrar *registrar = 0;
			if ( GetRegistrar( &registrar, true ) == 0 && registrar )
			{
				registrar->DeleteItem( buf );
				// if gen_crasher is requested to remove, also remove reporter.exe
				if ( skip_crasher )
				{
					GetModuleFileNameW( NULL, buf, MAX_PATH );
					PathRemoveFileSpecW( buf );
					PathCombineW( buf, buf, L"reporter.exe" );
					registrar->DeleteItem( buf );
				}
				registrar->Release();
			}

			_w_s( "remove_genplug", 0 );
		}
	}

	fft_init();
	SpectralAnalyzer_Create();
	JSAPI1_Initialize();
	stats_init();

	w5s_init();


	if ( !in_init() )
	{
		w5s_deinit();
		Wasabi_Unload();
		RemoveRegistrar();
		CoUninitialize();
		ExitProcess( 0 );
	}

	out_init();
	vis_init();

	if ( lpszCmdParam && *lpszCmdParam && !bAdd && !bHandle )
		config_read( 1 );
	else
		config_read( 0 );

	CreateEQPresets();

	if ( is_install )
		DoInstall( is_install );

	reg_associated_filetypes( 0 );

	if ( config_splash ) splashDlg( SPLASH_DELAY );	// display splash screen if desired

	PlayList_getcurrent( FileName, FileTitle, FileTitleNum ); // update filename and filetitle if a list was loaded

	songChangeBroadcastMessage = RegisterWindowMessageW( L"WinampSongChange" );

	if ( !InitApplication( hMainInstance ) )
	{
		LPMessageBox( NULL, IDS_ERRORINIT, IDS_ERROR, MB_OK );
		return FALSE;
	}

	if ( !InitInstance( hMainInstance, (int)param ) )
	{
		LPMessageBox( NULL, IDS_ERRORINIT, IDS_ERROR, MB_OK );
		return FALSE;
	}

	if ( !bHandle )
	{
		if ( *lpszCmdParam ) // if command line parameters, parse them
		{
			parseCmdLine( lpszCmdParam, 0 );
			plEditRefresh();
			{
				if ( config_shuffle )
					PlayList_randpos( -BIGINT );

				if ( !bAdd )
					bStartPlaying = 1;
			}
		}
		else // otherwise, we're using our loaded playlist
		{
			if ( config_shuffle ) PlayList_randpos( -BIGINT );
			PlayList_getcurrent( FileName, FileTitle, FileTitleNum );
		}
	}

	//SetCurrentDirectoryW(config_cwd);

	plEditSelect( PlayList_getPosition() );

	Ole_initDragDrop();

	if ( !( GetAsyncKeyState( VK_RCONTROL ) & 0x8000 ) || !( GetAsyncKeyState( VK_LCONTROL ) & 0x8000 ) )
	{
		load_genplugins(); // load general purpose plugins

		if ( !Skin_Check_Modern_Support() )
		{
			wchar_t msg[ 512 ] = { 0 };
			StringCchPrintfW( msg, 512, getStringW( IDS_NO_MODERN_SKIN_SUPPORT, NULL, 0 ), config_skin );
			MessageBoxW( NULL, msg, getStringW( IDS_SKIN_LOAD_ERROR, NULL, 0 ), MB_ICONWARNING | MB_OK | MB_TOPMOST );
		}
	}

	//disable video menu if no video plugins are present or configured as disabled
	if ( !g_has_video_plugin )
	{
		RemoveMenu( main_menu, WINAMP_OPTIONS_VIDEO, MF_BYCOMMAND );
		RemoveMenu( GetSubMenu( v5_top_menu, 3 ), WINAMP_OPTIONS_VIDEO, MF_BYCOMMAND );
		g_mm_optionsbase_adj -= 1;
	}

	set_aot( 0 ); // in case our gen plugins did anything fun
	set_priority();

	{
		int v = _r_i( "show_prefs", 0 );
		if ( v != 0 )
		{
			if ( v > 0 ) prefs_last_page = v;
			_w_i( "show_prefs", 0 );
			PostMessageW( hMainWindow, WM_COMMAND, WINAMP_OPTIONS_PREFS, 0 );
			SetTimer( hMainWindow, 969, 1, PrefsShowProc );
		}
	}

	if ( bStartPlaying )
	{
		PlayList_getcurrent( FileName, FileTitle, FileTitleNum );
		SendMessageW( hMainWindow, WM_COMMAND, WINAMP_BUTTON2, 0 );
		//SendMessageW(hMainWindow,WM_WA_IPC,0,IPC_STARTPLAY);
		draw_paint( NULL );
	}

	WADlg_init( hMainWindow );

	SetTimer( hMainWindow, USER_CONSENT_EVENT_ID, 5000, DisplayUserConsentMessageBox );

#ifdef BENSKI_TEST_WM_PRINTCLIENT
	SetTimer( hMainWindow, 9999, 10000, 0 );
#endif

	if ( bHandle && *lpszCmdParam )
		PostMessageW( hMainWindow, WM_WA_IPC, (WPARAM)lpszCmdParam, IPC_HANDLE_URI );

	parseMessageCommands( hMainWindow, bCommand, bCmdParam );

	WPARAM exitParam = WinampMessageLoop();
	JSAPI1_Uninitialize();

	unload_genplugins();
	w5s_deinit();

	stats_save();
	SpectralAnalyzer_Destroy();

	/*Browser_Destroy();*/
	//RaiseException(0x0000DEAD,0,0,0);

	Ole_uninitDragDrop();

	if ( g_restartonquit )
	{
		char buf[ MAX_PATH ] = "\"";
		STARTUPINFO si = { sizeof( si ), };
		PROCESS_INFORMATION pi;
		GetModuleFileNameA( NULL, buf + 1, sizeof( buf ) - 1 );
		StringCchCatA( buf, MAX_PATH, "\"" );

		if ( g_restartonquit == 2 )
			StringCchCatA( buf, MAX_PATH, " /SAFE=1" );

		CreateProcessA( NULL, buf, NULL, NULL, FALSE, 0, NULL, NULL, (LPSTARTUPINFOA)&si, &pi );
	}

	RemoveRegistrar();
	Wasabi_Unload();
	CoUninitialize();
	return exitParam;
} // WinMain

void MoveOffscreen( HWND hwnd )
{
	RECT r;
	GetWindowRect( hwnd, &r );
	SetWindowPos( hwnd, 0, r.left, OFFSCREEN_Y_POS, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER );
}

int g_showcode, deferring_show = 0;

#include "../Plugins/General/gen_ml/ml_ipc.h"

extern librarySendToMenuStruct mainSendTo = { 0 };

// used to delay / filter out quick IPC_SETDIALOGBOXPARENT messages
// to try to prevent the aero-peek buttons failing / part loading
#define AEROPEEKLOAD 0xC0DE+5
VOID CALLBACK TaskButtonCreated( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
	KillTimer( hwnd, idEvent );
	OnTaskbarButtonCreated( TRUE );
}

void UpdateAudioCDMenus( HMENU hmenu )
{
	wchar_t buf[ 32 ] = { 0 };
	if ( IsMenu( hmenu ) && in_get_extended_fileinfoW( L"cda://", L"ext_cdda", buf, ARRAYSIZE( buf ) ) && buf[ 0 ] == L'1' )
	{
		for ( int i = 0; i < 5; i++ )
		{
			DeleteMenu( hmenu, ID_MAIN_PLAY_AUDIOCD + i, MF_BYCOMMAND );
		}

		MENUITEMINFOW i = { sizeof( i ), MIIM_TYPE | MIIM_DATA | MIIM_ID, MFT_STRING, };
		i.wID = ID_MAIN_PLAY_AUDIOCD;

		DWORD drives = GetLogicalDrives();
		g_audiocdletters = 0;

		int need_sep = 1;
		for ( int drivemask = 0; drivemask < 32; drivemask++ )
		{
			if ( drives & ( 1 << drivemask ) )
			{
				int old_error_mode = SetErrorMode( SEM_FAILCRITICALERRORS );
				wchar_t str[ 64 ] = { 0 }, tmp[ 64 ] = { 0 }, vol_buf[ 40 ] = { 0 }, empty[ 64 ] = { 0 };
				DWORD system_flags = 0, max_file_len = 0;
				char c = ( 'A' + drivemask );
				StringCchPrintfW( str, 64, L"%c:\\", c );

				if ( GetDriveTypeW( str ) == DRIVE_CDROM )
				{
					if ( need_sep )
					{
						MENUITEMINFO i2 = { sizeof( i2 ), MIIM_TYPE | MIIM_ID, MFT_SEPARATOR, };
						i2.wID = ID_MAIN_PLAY_AUDIOCD_SEP;
						InsertMenuItem( hmenu, 4, TRUE, &i2 );
						need_sep = 0;
					}

					GetVolumeInformationW( str, vol_buf, ARRAYSIZE( vol_buf ), 0, &max_file_len, &system_flags, 0, 0 );
					SetErrorMode( old_error_mode );

					StringCchPrintfW( str, 256, getStringW( IDS_AUDIO_CD, tmp, 64 ), c, ( vol_buf[ 0 ] ? vol_buf : getStringW( IDS_EMPTY, empty, 64 ) ) );

					g_audiocdletter[ g_audiocdletters ] = c;
					i.dwTypeData = str;
					i.cch = (UINT)wcslen( str );
					InsertMenuItemW( hmenu, 4 + g_audiocdletters, TRUE, &i );
					i.wID++;

					g_audiocdletters++;
					if ( g_audiocdletters == 4 )
						break;
				}
			}
		}
	}
}

// Main Winamp window procedure
// we use message crackers when available, write our own for the ones that aren't
LRESULT CALLBACK Main_WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == g_scrollMsg )
	{
		wParam <<= 16;
		uMsg = WM_MOUSEWHEEL;
	}

	if ( uMsg == WM_TASKBARCREATED )
	{
		if ( systray_intray )
		{
			systray_restore();
			systray_minimize( caption );
		}
		return 0;
	}

	if ( uMsg == WM_TASKBARBUTTONCREATED )
	{
		OnTaskbarButtonCreated( TRUE );
		KillTimer( hMainWindow, AEROPEEKLOAD );
		SetTimer( hMainWindow, AEROPEEKLOAD, 250, TaskButtonCreated );
		return 0;
	}

	if ( IsDirectMouseWheelMessage( uMsg ) != FALSE )
	{
		SendMessageW( hwnd, WM_MOUSEWHEEL, wParam, lParam );
		return TRUE;
	}

	switch ( uMsg )
	{
		case WM_INITMENUPOPUP:
		{
			HMENU hMenu = (HMENU)wParam;
			if ( wParam && hMenu == mainSendTo.build_hMenu && mainSendTo.mode == 1 )
			{
				int IPC_LIBRARY_SENDTOMENU = wa_register_ipc( ( WPARAM ) & "LibrarySendToMenu" );
				if ( IPC_LIBRARY_SENDTOMENU > 65536 && SendMessageW( hMainWindow, WM_WA_IPC, (WPARAM)&mainSendTo, IPC_LIBRARY_SENDTOMENU ) == 0xffffffff )
					mainSendTo.mode = 2;
			}

			if ( config_usecursors && !disable_skin_cursors )
			{
				if ( Skin_Cursors[ 2 ] )
					SetCursor( Skin_Cursors[ 2 ] );
				else
					SetCursor( LoadCursorW( NULL, IDC_ARROW ) );
			}
			else
				SetCursor( LoadCursorW( NULL, IDC_ARROW ) );

			if ( hMenu == main_menu )
			{
				MENUITEMINFOW l_menu_info = { sizeof( l_menu_info ), };
				l_menu_info.fMask = MIIM_TYPE;
				l_menu_info.fType = MFT_STRING;
				l_menu_info.dwTypeData = getStringW( IDS_WINAMP_MENUITEM, NULL, 0 );
				l_menu_info.cch = (UINT)wcslen( l_menu_info.dwTypeData );
				SetMenuItemInfoW( main_menu, 0, TRUE, &l_menu_info );
				EnableMenuItem( main_menu, WINAMP_FILE_QUIT, MF_BYCOMMAND | ( g_exit_disabled ? MF_GRAYED : MF_ENABLED ) );
			}
			else if ( hMenu == g_submenus_play )
			{
				UpdateAudioCDMenus( g_submenus_play );
			}
			else if ( hMenu == g_submenus_bookmarks1 || hMenu == g_submenus_bookmarks2 )
			{
				MENUITEMINFOW l_menu_info = { sizeof( l_menu_info ), };
				FILE *fp = 0;
				int a = 34768;
				int offs = 3;
				int count = GetMenuItemCount( hMenu ) + 1;
				if ( hMenu != g_submenus_bookmarks1 )
					offs = 0;

				l_menu_info.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID;
				l_menu_info.fType = MFT_STRING;
				l_menu_info.wID = 34768;

				// this will remove the "(no bookmarks)" item from Main menu->Play->Bookmsrk
				// if it still exists -> removed as of 5.55 since we handle the menu better.
				if ( !offs ) RemoveMenu( hMenu, ID_MAIN_PLAY_BOOKMARK_NONE, MF_BYCOMMAND );

				// remove all of the items we might have added - do by command for certainty
				while ( count )
				{
					if ( !RemoveMenu( hMenu, a++, MF_BYCOMMAND ) ) break;
					count--;
				}

				fp = _wfopen( BOOKMARKFILE8, L"rt" );
				if ( fp )
				{
					while ( 1 )
					{
						char ft[ 4096 ] = { 0 }, fn[ MAX_PATH ] = { 0 };
						fgets( fn, MAX_PATH, fp );
						if ( feof( fp ) ) break;
						fgets( ft, 4096, fp );
						if ( feof( fp ) ) break;
						if ( ft[ 0 ] && fn[ 0 ] )
						{
							if ( fn[ lstrlenA( fn ) - 1 ] == '\n' ) fn[ lstrlenA( fn ) - 1 ] = 0;
							if ( ft[ lstrlenA( ft ) - 1 ] == '\n' ) ft[ lstrlenA( ft ) - 1 ] = 0;
							if ( ft[ 0 ] && fn[ 0 ] )
							{
								l_menu_info.dwTypeData = AutoWideDup( ft, CP_UTF8 );
								l_menu_info.cch = lstrlenW( l_menu_info.dwTypeData );
								RemoveMenu( hMenu, l_menu_info.wID, MF_BYCOMMAND );
								InsertMenuItemW( hMenu, l_menu_info.wID + offs - 34768, TRUE, &l_menu_info );
								l_menu_info.wID++;
							}
						}
					}
					fclose( fp );
				}

				g_BookmarkTop = l_menu_info.wID;

				// put in a place holder item if there were no read bookmarks
				if ( g_BookmarkTop == 34768 )
				{
					l_menu_info.dwTypeData = getStringW( IDS_NO_BOOKMARKS, NULL, 0 );
					l_menu_info.cch = lstrlenW( l_menu_info.dwTypeData );
					InsertMenuItemW( hMenu, l_menu_info.wID + offs - 34768, TRUE, &l_menu_info );
					EnableMenuItem( hMenu, l_menu_info.wID, MF_BYCOMMAND | MF_GRAYED );
				}
			}
			else if ( hMenu == g_submenus_skins1 || hMenu == g_submenus_skins2 )
			{
				MENUITEMINFOW l_menu_info = { sizeof( l_menu_info ), };
				HANDLE h;
				WIN32_FIND_DATAW d = { 0 };
				wchar_t dirmask[ MAX_PATH ] = { 0 };

				l_menu_info.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID | MIIM_STATE;
				l_menu_info.fType = MFT_STRING;
				l_menu_info.wID = 32768;
				PathCombineW( dirmask, SKINDIR, L"*" );

				if ( !config_skin[ 0 ] )
					CheckMenuItem( hMenu, 32767, MF_CHECKED );
				else
					CheckMenuItem( hMenu, 32767, MF_UNCHECKED );

				h = FindFirstFileW( dirmask, &d );
				if ( h != INVALID_HANDLE_VALUE )
				{
					int a = 0, mod_got = 0, bento_got = 0, checked = 0;
					do
					{
						if ( !wcscmp( d.cFileName, L"." ) || !wcscmp( d.cFileName, L".." ) ) continue;
						if ( ( d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ||
							 !_wcsicmp( extensionW( d.cFileName ), L"zip" ) ||
							 !_wcsicmp( extensionW( d.cFileName ), L"wal" ) ||
							 !_wcsicmp( extensionW( d.cFileName ), L"wsz" ) )
						{
							if ( !_wcsicmp( config_skin, d.cFileName ) )
							{
								l_menu_info.fState = MFS_CHECKED | MFS_ENABLED;
								checked = 1;
							}
							else
							{
								l_menu_info.fState = MFS_UNCHECKED | MFS_ENABLED;
							}

							if ( d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
								l_menu_info.dwItemData = 0;
							else
							{
								if ( !_wcsicmp( extensionW( d.cFileName ), L"zip" ) )
									l_menu_info.dwItemData = 1;
								else if ( !_wcsicmp( extensionW( d.cFileName ), L"wal" ) )
									l_menu_info.dwItemData = 4;
								else
									l_menu_info.dwItemData = 2;
								extensionW( d.cFileName )[ -1 ] = 0;
							}

							l_menu_info.dwTypeData = d.cFileName;
							l_menu_info.cch = (UINT)wcslen( d.cFileName );
							if ( !a )
								if ( !RemoveMenu( hMenu, l_menu_info.wID + 4 - 32768, MF_BYPOSITION ) )
									a = 1;

							if ( !l_menu_info.dwItemData && !_wcsicmp( d.cFileName, MODERN_SKIN_NAME ) )
							{
								mod_got = 1;
								InsertMenuItemW( hMenu, 4, TRUE, &l_menu_info );
							}
							else if ( !_wcsicmp( d.cFileName, BENTO_SKIN_NAME ) )
							{
								// place below classic + modern (if it exists)
								bento_got = 1;
								InsertMenuItemW( hMenu, 4 + mod_got, TRUE, &l_menu_info );
							}
							else if ( !_wcsicmp( d.cFileName, BIG_BENTO_SKIN_NAME ) )
							{
								// place below classic + modern + normal bento (if it exists)
								InsertMenuItemW( hMenu, 4 + mod_got + bento_got, TRUE, &l_menu_info );
							}
							else
								InsertMenuItemW( hMenu, l_menu_info.wID + 4 - 32768, TRUE, &l_menu_info );

							l_menu_info.wID++;
						}
					} while ( l_menu_info.wID < 34700 && FindNextFileW( h, &d ) );
					FindClose( h );
					g_SkinTop = l_menu_info.wID;
					while ( !a ) if ( !RemoveMenu( hMenu, l_menu_info.wID++ + 4 - 32768, MF_BYPOSITION ) ) a = 1;
					if ( !checked ) CheckMenuItem( hMenu, 32767, MF_CHECKED );
				}
			}
			else if ( ( hMenu == g_submenus_lang ) && config_wlz_menu )
			{
				MENUITEMINFOW l_menu_info = { sizeof( l_menu_info ), };
				HANDLE h;
				WIN32_FIND_DATAW d = { 0 };
				wchar_t dirmask[ MAX_PATH ] = { 0 };

				l_menu_info.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID | MIIM_STATE;
				l_menu_info.fType = MFT_STRING;
				l_menu_info.wID = 34700;
				PathCombineW( dirmask, LANGDIR, L"*" );

				l_menu_info.dwTypeData = L"English (US)";
				l_menu_info.cch = (UINT)wcslen( l_menu_info.dwTypeData );
				InsertMenuItemW( hMenu, l_menu_info.wID - 34700, TRUE, &l_menu_info );
				l_menu_info.wID++;

				CheckMenuItem( hMenu, 34700, ( !config_langpack[ 0 ] ? MF_CHECKED : MF_UNCHECKED ) );

				h = FindFirstFileW( dirmask, &d );
				if ( h != INVALID_HANDLE_VALUE )
				{
					int a = 0, checked = 0;
					do
					{
						if ( !wcscmp( d.cFileName, L"." ) || !wcscmp( d.cFileName, L".." ) ) continue;

						if ( ( d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ||
							 !_wcsicmp( extensionW( d.cFileName ), L"zip" ) ||
							 !_wcsicmp( extensionW( d.cFileName ), L"wlz" ) )
						{

							if ( !_wcsicmp( config_langpack, d.cFileName ) )
							{
								l_menu_info.fState = MFS_CHECKED | MFS_ENABLED;
								checked = 1;
							}
							else
							{
								l_menu_info.fState = MFS_UNCHECKED | MFS_ENABLED;
							}

							if ( d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
							{
								wchar_t check[ MAX_PATH ] = { 0 };
								PathCombineW( check, LANGDIR, d.cFileName );
								PathCombineW( check, check, L"winamp.lng" );

								if ( !PathFileExistsW( check ) )
									continue;

								l_menu_info.dwItemData = 0;
							}
							else
							{
								if ( !_wcsicmp( extensionW( d.cFileName ), L"zip" ) )
									l_menu_info.dwItemData = 1;
								else
									l_menu_info.dwItemData = 2;

								extensionW( d.cFileName )[ -1 ] = 0;
							}

							l_menu_info.dwTypeData = d.cFileName;
							l_menu_info.cch = (UINT)wcslen( d.cFileName );

							if ( !a )
								if ( !RemoveMenu( hMenu, l_menu_info.wID - 34700, MF_BYPOSITION ) )
									a = 1;

							InsertMenuItemW( hMenu, l_menu_info.wID - 34700, TRUE, &l_menu_info );
							l_menu_info.wID++;
						}
					} while ( l_menu_info.wID < 34800 && FindNextFileW( h, &d ) );

					FindClose( h );
					g_LangTop = l_menu_info.wID;
					while ( !a )
						if ( !RemoveMenu( hMenu, l_menu_info.wID++ - 34700, MF_BYPOSITION ) )
							a = 1;

					if ( !checked )
						CheckMenuItem( hMenu, 34700, MF_CHECKED );
				}
			}
			break;
		}
		case WM_DISPLAYCHANGE:
			Main_OnDisplayChange( hwnd );
			break;
		case WM_WA_SYSTRAY:
			return ( Main_OnWASystray( hwnd, (int)LOWORD( lParam ) ) ? 0 : -1L );
		case WM_WA_MPEG_EOF:
			return ( Main_OnWAMPEGEOF( hwnd ) ? 0 : -1L ); // sent by decode thread
		case WM_WA_IPC:
			return ( Main_OnIPC( hwnd, lParam, (int)(DWORD)wParam ) );

			HANDLE_MSG( hwnd, WM_COMMAND, Main_OnCommand );
			HANDLE_MSG( hwnd, WM_SYSCOMMAND, Main_OnSysCommand );
			HANDLE_MSG( hwnd, WM_CREATE, Main_OnCreate );
			HANDLE_MSG( hwnd, WM_QUERYNEWPALETTE, Main_OnQueryNewPalette );
			HANDLE_MSG( hwnd, WM_PALETTECHANGED, Main_OnPaletteChanged );
			HANDLE_MSG( hwnd, WM_SIZE, Main_OnSize );
			HANDLE_MSG( hwnd, WM_DROPFILES, Main_OnDropFiles );
			HANDLE_MSG( hwnd, WM_TIMER, Main_OnTimer );
			HANDLE_MSG( hwnd, WM_PAINT, draw_paint );
		case WM_PRINTCLIENT:
			draw_printclient( (HDC)wParam, lParam );
			return 0;

			HANDLE_MSG( hwnd, WM_RBUTTONUP, Main_OnRButtonUp );
			HANDLE_MSG( hwnd, WM_LBUTTONDBLCLK, Main_OnLButtonDblClk );
			HANDLE_MSG( hwnd, WM_LBUTTONUP, Main_OnLButtonUp );
			HANDLE_MSG( hwnd, WM_LBUTTONDOWN, Main_OnLButtonDown );
			HANDLE_MSG( hwnd, WM_MOUSEMOVE, Main_OnMouseMove );
		case WM_CONTEXTMENU:
			SendMessageW( hwnd, WM_COMMAND, WINAMP_MAINMENU, 0 );
			return 0;
		case WM_CAPTURECHANGED:
			return Main_OnCaptureChanged( (HWND)lParam );

			HANDLE_MSG( hwnd, WM_DESTROY, Main_OnDestroy );
			HANDLE_MSG( hwnd, WM_CLOSE, Main_OnClose );
		case WM_NCACTIVATE:
		{
			LRESULT result = (LRESULT)(DWORD)(BOOL)(Main_OnNCActivate)( ( hwnd ), (BOOL)( wParam ), (HWND)( lParam ), 0L );
			if ( IsIconic( hwnd ) )
				break;
			else
				return result;
		}
		case WM_NCHITTEST:
			if ( IsIconic( hwnd ) )
				break;
			return (LRESULT)(DWORD)(UINT)(Main_OnNCHitTest)( ( hwnd ), (int)(short)LOWORD( lParam ), (int)(short)HIWORD( lParam ) );
		case WM_NCCALCSIZE:
			if ( IsIconic( hwnd ) )
				break;
			return (LRESULT)(DWORD)(UINT)Main_OnNCCalcSize( hwnd, (BOOL)wParam, (NCCALCSIZE_PARAMS *)lParam );

			HANDLE_MSG( hwnd, WM_ENDSESSION, Main_OnEndSession );
		case WM_QUERYENDSESSION:
			return !!SendMessageW( hwnd, WM_WA_IPC, 0, IPC_HOOK_OKTOQUIT );
		case WM_KEYDOWN:
		{
			static int pos;
			TCHAR buf[ 2 ] = { (TCHAR)wParam, 0 };
			CharUpperBuff( buf, 1 );

			if ( buf[ 0 ] == eggstr[ pos ] )
			{
				eggTyping = TRUE;
				if ( !eggstr[ ++pos ] )
				{
					eggTyping = FALSE;
					eggstat = !eggstat;
					pos = 0;
					draw_tbar( 1, config_windowshade, eggstat );
				}
			}
			else
				pos = 0;

			break;
		}
		case WM_NCPAINT:
			return 0;
		case WM_COPYDATA:
			return Main_OnCopyData( (HWND)wParam, (COPYDATASTRUCT *)lParam );
		case WM_GETTEXT:
			return Main_OnGetText( (wchar_t *)lParam, (int)wParam );
		case WM_NOTIFY:
		{
			LPTOOLTIPTEXTW tt = (LPTOOLTIPTEXTW)lParam;
			if ( tt->hdr.hwndFrom = hTooltipWindow )
			{
				switch ( tt->hdr.code )
				{
					case TTN_SHOW:
						SetWindowPos( tt->hdr.hwndFrom, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE );
						break;
					case TTN_NEEDTEXTW:
					{
						LPTOOLTIPTEXTW tt = (LPTOOLTIPTEXTW)lParam;
						wchar_t booga[ 81 ] = { 0 };
						GetWindowTextW( hwnd, booga, 79 );
						booga[ 79 ] = 0;
						StringCchCopyW( tt->szText, 80, booga );
						tt->lpszText = tt->szText;
						return 0;
					}
				}
			}
			break;
		}
		case WM_SETFOCUS:
			if ( !config_mw_open )
			{
				if ( config_pe_open )
					SetForegroundWindow( hPLWindow );
				else if ( config_eq_open )
					SetForegroundWindow( hEQWindow );
				// else if (config_mb_open) SetForegroundWindow(hMBWindow);
				else if ( config_video_open )
					SetForegroundWindow( hVideoWindow );
				else
				{
					EnterCriticalSection( &embedcs );
					{
						embedWindowState *p = embedwndlist;
						while ( p )
						{
							if ( IsWindowVisible( p->me ) )
							{
								SetForegroundWindow( p->me );
								break;
							}

							p = p->link;
						}
					}

					LeaveCriticalSection( &embedcs );
				}
			}

			break;
		case WM_KILLFOCUS:
		{
			if ( !config_mw_open )
			{
				if ( config_pe_open )
					SetForegroundWindow( hPLWindow );
				else if ( config_eq_open )
					SetForegroundWindow( hEQWindow );
				// else if (config_mb_open) SetForegroundWindow(hMBWindow);
				else if ( config_video_open )
					SetForegroundWindow( hVideoWindow );
				else
				{
					EnterCriticalSection( &embedcs );
					{
						embedWindowState *p = embedwndlist;
						while ( p )
						{
							if ( IsWindowVisible( p->me ) )
							{
								SetForegroundWindow( p->me );
								break;
							}

							p = p->link;
						}
					}

					LeaveCriticalSection( &embedcs );
				}
			}


			break;
		}
		case WM_MOUSEWHEEL:
		{
			// because some people don't like it
			if ( config_nomwheel )
				break;

			int zDelta = GET_WHEEL_DELTA_WPARAM( wParam ), dLines;
			// if the delta changes then ignore prior carryover
			// hopefully this will go with the expected action.
			if ( zDelta < 0 && main_delta_carryover > 0 || zDelta > 0 && main_delta_carryover < 0 )
				main_delta_carryover = 0;
			// otherwise add on the carryover from the prior message
			else
				zDelta += main_delta_carryover;

			if ( 0 == ( MK_MBUTTON & LOWORD( wParam ) ) )
				zDelta *= 2;

			dLines = zDelta / WHEEL_DELTA;
			main_delta_carryover = zDelta - dLines * WHEEL_DELTA;

			if ( 0 != dLines )
			{
				zDelta = ( dLines > 0 ) ? dLines : -dLines;

				if ( 0 != ( MK_MBUTTON & LOWORD( wParam ) ) )
				{
					if ( dLines >= 0 )
						dLines = WINAMP_FFWD5S;
					else
						dLines = WINAMP_REW5S;
				}
				else
				{
					if ( dLines >= 0 )
						dLines = WINAMP_VOLUMEUP;
					else
						dLines = WINAMP_VOLUMEDOWN;
				}

				while ( zDelta-- )
				{
					SendMessageW( hwnd, WM_COMMAND, dLines, 0 );
				}
			}
			break;
		}
		case WM_DWMSENDICONICTHUMBNAIL:
		{
			int x = HIWORD( lParam );
			int y = LOWORD( lParam );
			OnIconicThumbnail( x, y );
		}
		break;
#if 0
		case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
		{
			//MessageBoxA(NULL, "winamp/live", "winamp/live", MB_OK);
			OnThumbnailPreview();
		}
		break;
#endif
		case WM_MOVE:
#if 0
			if ( (int)LOWORD( lParam ) < 32768 && (int)HIWORD( lParam ) < 32768 )
			{
				if (/*(int)LOWORD(lParam) != 3000 && */(int)HIWORD( lParam ) != OFFSCREEN_Y_POS )
				{
					if ( (int)LOWORD( lParam ) != config_wx ||
						 (int)HIWORD( lParam ) != config_wy )
						if ( config_keeponscreen & 1 )
						{
							config_wx = (int)LOWORD( lParam );
							config_wy = (int)HIWORD( lParam );
							set_aot( 1 );
						}
				}
			}
#endif
			break;
		case WM_SETCURSOR:

			switch ( HIWORD( lParam ) )
			{
				case WM_LBUTTONDOWN:
				case WM_RBUTTONDOWN:
				case WM_MBUTTONDOWN:
				case WM_XBUTTONDOWN:
					DisabledWindow_OnMouseClick( hwnd );
					break;
			}
			if ( config_usecursors && !disable_skin_cursors )
			{
				if ( (HWND)wParam == hMainWindow && HIWORD( lParam ) == WM_MOUSEMOVE )
					ui_handlecursor();

				return TRUE;
			}
			else
				SetCursor( LoadCursorW( NULL, IDC_ARROW ) );

			break;
	}

	return ( DefWindowProcW( hwnd, uMsg, wParam, lParam ) );
}

static LRESULT Main_OnSysCommand( HWND hwnd, UINT cmd, int x, int y )
{
	//  char buf[512];
	//  wsprintf(buf,"got WM_SYSCOMMAND %08x\n",cmd);
	//  OutputDebugString(buf);
	// video
	if ( ( ( cmd & 0xfff0 ) == SC_SCREENSAVE || ( cmd & 0xfff0 ) == SC_MONITORPOWER ) && config_video_noss && video_isVideoPlaying() )
	{
		return -1;
	}

	if ( !Main_OnCommand( hwnd, cmd, (HWND)x, (UINT)y ) )
		FORWARD_WM_SYSCOMMAND( hwnd, cmd, x, y, DefWindowProcW );
	return 1;
}

static LRESULT WINAPI browseCheckBoxProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			if ( !( config_rofiob & 2 ) )
				CheckDlgButton( hwndDlg, IDC_CHECK1, BST_CHECKED );

			break;
		}
		case WM_COMMAND:
		{
			if ( LOWORD( wParam ) == IDC_CHECK1 )
			{
				config_rofiob &= ~2;
				if ( !IsDlgButtonChecked( hwndDlg, IDC_CHECK1 ) )
					config_rofiob |= 2;

				config_write( 0 );
			}

			break;
		}
	}

	return 0;
}

BOOL CALLBACK browseEnumProc( HWND hwnd, LPARAM lParam )
{
	char cl[ 32 ] = { 0 };
	GetClassNameA( hwnd, cl, ARRAYSIZE( cl ) );
	if ( !lstrcmpiA( cl, WC_TREEVIEWA ) )
	{
		PostMessageW( hwnd, TVM_ENSUREVISIBLE, 0, (LPARAM)TreeView_GetSelection( hwnd ) );
		return FALSE;
	}

	return TRUE;
}

int CALLBACK WINAPI BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
	switch ( uMsg )
	{
		case BFFM_INITIALIZED:
		{
			SetWindowTextW( hwnd, getStringW( ( !lpData ? IDS_OPENDIR : IDS_ADD_FOLDER ), NULL, 0 ) );
			SendMessageW( hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)config_cwd );

			HWND h = FindWindowExW( hwnd, NULL, NULL, L"__foo" );
			if ( h )
				ShowWindow( h, SW_HIDE );

			h = LPCreateDialogW( IDD_BROWSE_RECDLG, hwnd, browseCheckBoxProc );

			SetWindowPos( h, 0, 4, 4, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
			ShowWindow( h, SW_SHOWNA );

			// this is not nice but it fixes the selection not working correctly on all OSes
			EnumChildWindows( hwnd, browseEnumProc, 0 );
		}
	}

	return 0;
}

LRESULT sendMlIpc( int msg, WPARAM param )
{
	static LRESULT IPC_GETMLWINDOW;
	static HWND mlwnd;

	if ( !IPC_GETMLWINDOW ) IPC_GETMLWINDOW = wa_register_ipc( ( WPARAM ) & "LibraryGetWnd" );
	if ( !mlwnd || (int)mlwnd == 1 )
		mlwnd = (HWND)SendMessageW( hMainWindow, WM_WA_IPC, 0, IPC_GETMLWINDOW );

	if ( param == 0 && msg == 0 ) return (LRESULT)mlwnd;

	if ( IsWindow( mlwnd ) )
		return SendMessageW( mlwnd, WM_ML_IPC, param, msg );

	return 0;
}

void tealike_crappy_code( unsigned long v[ 2 ], unsigned long k[ 4 ] )
{
	unsigned long y = v[ 0 ], z = v[ 1 ], sum = 0,    /* set up */
		delta = 0x9e3779b9UL, n = 32;  /* key schedule constant*/

	while ( n-- > 0 )
	{
		/* basic cycle start */
		sum += delta;
		y += ( ( z << 4 ) + k[ 0 ] ) ^ ( z + sum ) ^ ( ( z >> 5 ) + k[ 1 ] );
		z += ( ( y << 4 ) + k[ 2 ] ) ^ ( y + sum ) ^ ( ( y >> 5 ) + k[ 3 ] );   /* end cycle */
	}
	v[ 0 ] = y; v[ 1 ] = z;
}

// command line parsing, for IPC or normal modes
// goes to a lot of trouble to look for "'s.

HWND find_otherwinamp( wchar_t *cmdline )
{
	int y = 0;
	wchar_t buf[ MAX_PATH ] = { 0 };

	StringCchPrintfW( buf, MAX_PATH, L"%s_%x_CLASS", szAppName, APP_VERSION_NUM );
again:
	g_hEventRunning = CreateEventW( 0, 1, 0, buf );
	if ( g_hEventRunning && GetLastError() == ERROR_ALREADY_EXISTS )
	{
		int x;
		CloseHandle( g_hEventRunning );
		g_hEventRunning = 0;
		// check for window for 4s, then give up
		if ( !bNoHwndOther && ( !config_minst || *cmdline ) ) for ( x = 0; x < 40; x++ )
		{
			HWND lhwnd = NULL;
			int failed = 0;
			while ( ( lhwnd = FindWindowExW( NULL, lhwnd, szAppName, NULL ) ) )
			{
				DWORD_PTR vn = 0; //APP_VERSION_NUM
				if ( lhwnd == hMainWindow )
					continue;
				if ( !SendMessageTimeout( lhwnd, WM_WA_IPC, 0, IPC_GETVERSION, SMTO_NORMAL, 5000, &vn ) )
				{
					failed = 1;
				}
				else if ( vn == APP_VERSION_NUM ) return lhwnd;
			}
			if ( failed ) return NULL; // no valid winamp windows, but one that fucked up

			Sleep( 100 );
		}
		if ( y++ < 20 ) goto again;
	}
	return NULL;
}


// returns 0 if showing/hiding sould be aborted
int Ipc_WindowToggle( INT_PTR which, INT_PTR how )
{
	return SendMessageW( hMainWindow, WM_WA_IPC, which, how ? IPC_CB_ONSHOWWND : IPC_CB_ONHIDEWND );
}
//}

#ifdef BENSKI_TEST_WM_PRINTCLIENT
static void PrintWindow( HWND hWnd )
{
	HDC hDCMem = CreateCompatibleDC( NULL );
	HBITMAP hBmp = NULL;
	RECT rect;

	GetWindowRect( hWnd, &rect );
	{
		HDC hDC = GetDC( hWnd );
		hBmp = CreateCompatibleBitmap( hDC, rect.right - rect.left, rect.bottom - rect.top );
		ReleaseDC( hWnd, hDC );
	}
	{
		HGDIOBJ hOld = SelectObject( hDCMem, hBmp );
		SendMessageW( hWnd, WM_PRINT, (WPARAM)hDCMem, PRF_CHILDREN | PRF_CLIENT | PRF_ERASEBKGND | PRF_NONCLIENT | PRF_OWNED );

		SelectObject( hDCMem, hOld );
	}
	DeleteObject( hDCMem );

	OpenClipboard( hWnd );

	EmptyClipboard();
	SetClipboardData( CF_BITMAP, hBmp );
	CloseClipboard();
}
#endif