//------------------------------------------------------------------------
//
// iTunes XML Import/Export Plugin
// Copyright © 2003-2014 Winamp SA
//
//------------------------------------------------------------------------
//#define PLUGIN_NAME "Nullsoft Database Import/Export"
#define PLUGIN_VERSION L"2.65"

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <stdio.h>
#include "api__ml_impex.h"
#include "../../General/gen_ml/ml.h"
#include "../winamp/wa_ipc.h"
#include "resource.h"
#include <bfc/string/url.h>
#include "itunesxmlwrite.h"
#include "importer.h"
#include "ImporterAPI.h"
#include "../nu/Singleton.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"

static ImporterAPI importAPI;
static SingletonServiceFactory<api_itunes_importer, ImporterAPI> importerFactory;
// -----------------------------------------------------------------------

#define ID_IMPORT         35443
#define ID_EXPORT         35444
#define ID_CVTPLAYLISTS   35445
#define ID_IMPORT_ITUNES  35446

#define ID_FILE_ADDTOLIBRARY            40344
#define ID_DOSHITMENU_ADDNEWVIEW        40030
#define IDM_LIBRARY_CONFIG              40050
#define ID_DOSHITMENU_ADDNEWPLAYLIST    40031
#define WINAMP_MANAGEPLAYLISTS          40385

// -----------------------------------------------------------------------

api_application *WASABI_API_APP = 0;
api_playlistmanager *AGAVE_API_PLAYLISTMANAGER = 0;
api_playlists *AGAVE_API_PLAYLISTS = 0;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

wchar_t* GetFilterListString(void) {	// "iTunes XML Library\0*.xml\0\0"
	static wchar_t filterString[128] = {0};
	wchar_t* end = 0;
	StringCchCopyEx(filterString, 128, WASABI_API_LNGSTRINGW(IDS_ITUNES_XML_LIBRARY), &end, 0, 0);
	StringCchCopyEx(end+1, 128, L"*.xml", 0, 0, 0);
	return filterString;
}

// -----------------------------------------------------------------------

static LRESULT WINAPI ml_newParentWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static LRESULT WINAPI ml_newMlWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

static WNDPROC ml_oldParentWndProc;
static WNDPROC ml_oldMlWndProc;
static HWND export_wnd, hwnd_winamp, mlWnd;
extern winampMediaLibraryPlugin plugin;
HMENU mlMenu=NULL;
void exportDatabase();
void importDatabase();

// -----------------------------------------------------------------------
// dummy plugin message procs, we just ignore everything
// -----------------------------------------------------------------------
static INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	if (message_type == ML_MSG_NO_CONFIG)
		return TRUE;

	return FALSE;
}

// -----------------------------------------------------------------------
// plugin, exported to gen_ml
// -----------------------------------------------------------------------
static int init();
static void quit();

extern "C" winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"nullsoft(ml_impex.dll)",
	init,
	quit,
	PluginMessageProc,
	0,
	0,
	0,
};

// -----------------------------------------------------------------------
// export
// -----------------------------------------------------------------------
extern "C" {
	__declspec( dllexport ) winampMediaLibraryPlugin * winampGetMediaLibraryPlugin()
	{
		return &plugin;
	}
};

// -----------------------------------------------------------------------
// returns the position of a command within an HMENU
// -----------------------------------------------------------------------
int getMenuItemPos(HMENU menu, UINT command) {
	for (int i=0;i<256;i++) {
		MENUITEMINFO mii={sizeof(mii),MIIM_ID,};
		if (!GetMenuItemInfo(menu, i, TRUE, &mii)) break;
		if (mii.wID == command) return i;
	}
	return -1;
}

// -----------------------------------------------------------------------
// entry point, gen_ml is starting up and we've just been loaded
// -----------------------------------------------------------------------
int init() 
{
	waServiceFactory *sf = plugin.service->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	sf = plugin.service->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf) WASABI_API_APP = reinterpret_cast<api_application*>(sf->getInterface());

	sf = plugin.service->service_getServiceByGuid(api_playlistmanagerGUID);
	if (sf) AGAVE_API_PLAYLISTMANAGER = reinterpret_cast<api_playlistmanager*>(sf->getInterface());

	sf = plugin.service->service_getServiceByGuid(api_playlistsGUID);
	if (sf) AGAVE_API_PLAYLISTS = reinterpret_cast<api_playlists*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,MlImpexLangGUID);

	importerFactory.Register(plugin.service, &importAPI);

	static wchar_t szDescription[256];
	StringCchPrintf(szDescription, ARRAYSIZE(szDescription),
					WASABI_API_LNGSTRINGW(IDS_ML_IMPEX_DESC), PLUGIN_VERSION);
	plugin.description = (char*)szDescription;

	HWND w = plugin.hwndWinampParent;
	while (GetParent(w) != NULL) w = GetParent(w);
	hwnd_winamp = w;

	ml_oldParentWndProc = (WNDPROC)SetWindowLongPtrW(plugin.hwndWinampParent, GWLP_WNDPROC, (LONG_PTR)ml_newParentWndProc);

	mlMenu = (HMENU)SendMessage(hwnd_winamp, WM_WA_IPC, 9, IPC_GET_HMENU);
	int IPC_GETMLWINDOW=(int)SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)&"LibraryGetWnd",IPC_REGISTER_WINAMP_IPCMESSAGE);
	mlWnd = (HWND)SendMessage(hwnd_winamp, WM_WA_IPC, -1, IPC_GETMLWINDOW);

	ml_oldMlWndProc = (WNDPROC)SetWindowLongPtrW(mlWnd, GWLP_WNDPROC, (LONG_PTR)ml_newMlWndProc);

	int p = getMenuItemPos(mlMenu, ID_FILE_ADDTOLIBRARY);
	MENUITEMINFO mii={sizeof(mii),MIIM_ID|MIIM_TYPE, MFT_SEPARATOR, };
	InsertMenuItem(mlMenu, ++p, TRUE, &mii);

	if (importAPI.iTunesExists())
	{
		MENUITEMINFO mii2={sizeof(mii2),MIIM_ID|MIIM_STATE|MIIM_TYPE, MFT_STRING, MFS_ENABLED, ID_IMPORT_ITUNES, NULL, NULL, NULL, NULL, WASABI_API_LNGSTRINGW(IDS_IMPORT_ITUNES_DB), 0};
		InsertMenuItem(mlMenu, ++p, TRUE, &mii2);
		MENUITEMINFO mii5={sizeof(mii5),MIIM_ID|MIIM_STATE|MIIM_TYPE, MFT_STRING, MFS_ENABLED, ID_CVTPLAYLISTS, NULL, NULL, NULL, NULL, WASABI_API_LNGSTRINGW(IDS_IMPORT_ITUNES_PL), 0};
		InsertMenuItem(mlMenu, ++p, TRUE, &mii5);
	}
	MENUITEMINFO mii3={sizeof(mii),MIIM_ID|MIIM_STATE|MIIM_TYPE, MFT_STRING, MFS_ENABLED, ID_IMPORT, NULL, NULL, NULL, NULL, WASABI_API_LNGSTRINGW(IDS_IMPORT_DATABASE), 0};
	InsertMenuItem(mlMenu, ++p, TRUE, &mii3);
	MENUITEMINFO mii4={sizeof(mii),MIIM_ID|MIIM_STATE|MIIM_TYPE, MFT_STRING, MFS_ENABLED, ID_EXPORT, NULL, NULL, NULL, NULL, WASABI_API_LNGSTRINGW(IDS_EXPORT_DATABASE), 0};
	InsertMenuItem(mlMenu, ++p, TRUE, &mii4);

	int IPC_GET_ML_HMENU = (int)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"LibraryGetHmenu", IPC_REGISTER_WINAMP_IPCMESSAGE);
	HMENU context_menu = (HMENU) SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_ML_HMENU);
	if (context_menu)
	{
		HMENU hmenuPopup = GetSubMenu(context_menu, 0);
		if (hmenuPopup)
		{
			int p = getMenuItemPos(hmenuPopup, WINAMP_MANAGEPLAYLISTS);
			if (getMenuItemPos(hmenuPopup, IDM_LIBRARY_CONFIG) != -1) // sanity check
			{
				bool end_separator=true;
				if (p != -1)
				{
					MENUITEMINFO mii={sizeof(mii),MIIM_ID|MIIM_TYPE, MFT_SEPARATOR, };
					InsertMenuItem(hmenuPopup, ++p, TRUE, &mii);
					end_separator=false;
				}
				if (importAPI.iTunesExists())
				{
					MENUITEMINFO mii2={sizeof(mii2),MIIM_ID|MIIM_STATE|MIIM_TYPE, MFT_STRING, MFS_ENABLED, ID_IMPORT_ITUNES, NULL, NULL, NULL, NULL, WASABI_API_LNGSTRINGW(IDS_IMPORT_ITUNES_DB), 0};
					InsertMenuItem(hmenuPopup, ++p, TRUE, &mii2);
					MENUITEMINFO mii5={sizeof(mii5),MIIM_ID|MIIM_STATE|MIIM_TYPE, MFT_STRING, MFS_ENABLED, ID_CVTPLAYLISTS, NULL, NULL, NULL, NULL, WASABI_API_LNGSTRINGW(IDS_IMPORT_ITUNES_PL), 0};
					InsertMenuItem(hmenuPopup, ++p, TRUE, &mii5);
				}
				MENUITEMINFO mii3={sizeof(mii3),MIIM_ID|MIIM_STATE|MIIM_TYPE, MFT_STRING, MFS_ENABLED, ID_IMPORT, NULL, NULL, NULL, NULL, WASABI_API_LNGSTRINGW(IDS_IMPORT_DATABASE), 0};
				InsertMenuItem(hmenuPopup, ++p, TRUE, &mii3);
				MENUITEMINFO mii4={sizeof(mii4),MIIM_ID|MIIM_STATE|MIIM_TYPE, MFT_STRING, MFS_ENABLED, ID_EXPORT, NULL, NULL, NULL, NULL, WASABI_API_LNGSTRINGW(IDS_EXPORT_DATABASE), 0};
				InsertMenuItem(hmenuPopup, ++p, TRUE, &mii4);
				if (end_separator)
				{
					MENUITEMINFO mii={sizeof(mii),MIIM_ID|MIIM_TYPE, MFT_SEPARATOR, };
					InsertMenuItem(hmenuPopup, ++p, TRUE, &mii);
				}
			}
		}
	}

	return ML_INIT_SUCCESS;
}

// -----------------------------------------------------------------------
// entry point, gen_ml is shutting down and we are being unloaded
// -----------------------------------------------------------------------
void quit() {
	if (IsWindow(plugin.hwndWinampParent)) SetWindowLongPtrW(plugin.hwndWinampParent, GWLP_WNDPROC, (LONG_PTR)ml_oldParentWndProc);
	if (IsWindow(mlWnd)) SetWindowLongPtrW(mlWnd, GWLP_WNDPROC, (LONG_PTR)ml_oldMlWndProc);
	importerFactory.Deregister(plugin.service);
}

void handleMenuItem(int wID) {
	switch (wID) {
		case ID_EXPORT:
			exportDatabase();
			break;
		case ID_IMPORT:
			importDatabase();
			break;
		case ID_IMPORT_ITUNES:
			{
				importAPI.ImportFromiTunes(plugin.hwndLibraryParent);
			}
			break;
		case ID_CVTPLAYLISTS:
			{
				importAPI.ImportPlaylistsFromiTunes(plugin.hwndLibraryParent);
			}
			break;
	}
}

void setDialogIcon(HWND hwndDlg)
{
	static HICON wa_icy;
	if (wa_icy)
	{
		wa_icy = (HICON)LoadImage(GetModuleHandle(L"winamp.exe"),
								  MAKEINTRESOURCE(102), IMAGE_ICON,
								  GetSystemMetrics(SM_CXSMICON),
								  GetSystemMetrics(SM_CYSMICON),
								  LR_SHARED | LR_LOADTRANSPARENT | LR_CREATEDIBSECTION);
	}
	SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)wa_icy);
}

// -----------------------------------------------------------------------
// library HWND subclass
// -----------------------------------------------------------------------
static LRESULT WINAPI ml_newParentWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg) 
	{
		case WM_COMMAND:
		{
			int wID = LOWORD(wParam);
			handleMenuItem(wID);
		}
		break;
	}
	return CallWindowProcW(ml_oldParentWndProc,hwndDlg,uMsg,wParam,lParam);
}

// -----------------------------------------------------------------------
static LRESULT WINAPI ml_newMlWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg) 
	{
		case WM_COMMAND:
		{
			int wID = LOWORD(wParam);
			handleMenuItem(wID);
		}
		break;
	}
	return CallWindowProcW(ml_oldMlWndProc,hwndDlg,uMsg,wParam,lParam);
}

// {55334B63-68D5-4389-A209-797F123E207F}
static const GUID ML_IMPEX = 
{ 0x55334b63, 0x68d5, 0x4389, { 0xa2, 0x9, 0x79, 0x7f, 0x12, 0x3e, 0x20, 0x7f } };

//------------------------------------------------------------------------
// pick an input file
//------------------------------------------------------------------------
static int pickFile(HWND hwndDlg, StringW *file) 
{
	wchar_t oldCurPath[MAX_PATH] = {0};
	GetCurrentDirectoryW(MAX_PATH, oldCurPath);

	OPENFILENAME l={sizeof(l),};
	wchar_t *temp;
	const int len=256*1024-128;

	temp = (wchar_t *)GlobalAlloc(GPTR,len*sizeof(*temp));
	l.hwndOwner = hwndDlg;
	extern wchar_t* GetFilterListString(void);
	l.lpstrFilter = GetFilterListString();//L"iTunes XML Library\0*.xml\0\0";	// IDS_ITUNES_XML_LIBRARY
	l.lpstrFile = temp;
	l.nMaxFile = len-1;
	l.lpstrTitle = WASABI_API_LNGSTRINGW(IDS_IMPORT_DATABASE);
	l.lpstrDefExt = L"";
	l.lpstrInitialDir = WASABI_API_APP->path_getWorkingPath();;
	l.Flags = OFN_HIDEREADONLY|OFN_EXPLORER;
	if(GetOpenFileName(&l))
	{
		wchar_t newCurPath[MAX_PATH] = {0};
		GetCurrentDirectoryW(MAX_PATH, newCurPath);
		WASABI_API_APP->path_setWorkingPath(newCurPath);
		SetCurrentDirectoryW(oldCurPath);
		*file = temp;
		return 1;
	}
	SetCurrentDirectoryW(oldCurPath);
	return 0;
}

// -----------------------------------------------------------------------
// import an iTunes XML library into gen_ml
// -----------------------------------------------------------------------
void importDatabase() 
{
	StringW file;
	// pick an inputfile
	if (pickFile(plugin.hwndLibraryParent, &file)) 
	{
		importAPI.ImportFromFile(plugin.hwndLibraryParent, file.getValue());
		// TODO ideally should only do this if the current view is from ml_local
		PostMessage(plugin.hwndLibraryParent, WM_USER + 30, 0, 0);
	}
}

// -----------------------------------------------------------------------
int only_itunes = -1; // ask

// -----------------------------------------------------------------------
// ask the user if we should also write files unsupported by iTunes
// -----------------------------------------------------------------------
static BOOL CALLBACK exporttype_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			only_itunes = 0;
			setDialogIcon(hwndDlg);

			CheckDlgButton(hwndDlg, IDC_RADIO_ALLFILES, TRUE);
			CheckDlgButton(hwndDlg, IDC_RADIO_ONLYSUPPORTED, FALSE);
			SetForegroundWindow(hwndDlg);
			return 0;
		}

		case WM_COMMAND: {
			int wID = LOWORD(wParam);
			switch (wID) {
				case IDOK:
				case IDCANCEL:
					EndDialog(hwndDlg, wID);
					SetForegroundWindow(export_wnd);
				break;
				case IDC_RADIO_ONLYSUPPORTED:
					only_itunes = 1;
				break;
				case IDC_RADIO_ALLFILES:
					only_itunes = 0;
				break;
			}
		}
		return 0;
	}
	return 0;
}

// -----------------------------------------------------------------------
// export status window proc
// -----------------------------------------------------------------------
static BOOL CALLBACK export_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			export_wnd = hwndDlg;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			// init it
			ShowWindow(GetDlgItem(hwndDlg, IDC_PROGRESS_PERCENT), SW_SHOWNORMAL);
			ShowWindow(GetDlgItem(hwndDlg, IDC_TRACKS), SW_SHOWNORMAL);

			SetWindowText(hwndDlg,WASABI_API_LNGSTRINGW(IDS_EXPORTING_DATABASE));
			SendMessage(GetDlgItem(hwndDlg, IDC_PROGRESS_PERCENT), PBM_SETRANGE, 0, MAKELPARAM(0, 100));
			SendDlgItemMessage(hwndDlg, IDC_PROGRESS_PERCENT, PBM_SETPOS, 0, 0);

			setDialogIcon(hwndDlg);

			SetTimer(hwndDlg, 666, 250, 0);
			SetForegroundWindow(hwndDlg);
			return 0;
		}

		case WM_TIMER:
			if (wParam == 666)
			{
				int *progress = (int *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if (progress[0] == -333)
				{
					// show "saving xml"
					ShowWindow(GetDlgItem(hwndDlg, IDC_PROGRESS_PERCENT), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_TRACKS), SW_HIDE);
					SetDlgItemText(hwndDlg,IDC_PROCESSING_STATE,WASABI_API_LNGSTRINGW(IDS_WRITINGING_XML));
					ShowWindow(GetDlgItem(hwndDlg, IDC_PROCESSING_STATE), SW_SHOWNORMAL);
				}
				else if (progress[0] == -666)
				{
					KillTimer(hwndDlg, 666);
					EndDialog(hwndDlg, 0);
				}
				else
				{
					// display progress
					SendDlgItemMessage(hwndDlg, IDC_PROGRESS_PERCENT, PBM_SETPOS, (int)((double)progress[0] / progress[1] * 100.0), 0);
					SetDlgItemText(hwndDlg, IDC_TRACKS, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_TRACKS_EXPORTED_X), progress[0]));
				}
			}
			break;

		case WM_COMMAND: {
			int wID = LOWORD(wParam);
			switch (wID) {
				case IDOK:
				case IDCANCEL:
					EndDialog(hwndDlg, wID);
				break;
			}
		}
		return 0;
	}
	return 0;
}

// -----------------------------------------------------------------------
// returns 1 if str ends with e
// -----------------------------------------------------------------------
int endsIn(const wchar_t *str, const wchar_t *e) {
	return !_wcsicmp(str+wcslen(str)-wcslen(e), e);
}

static DWORD CALLBACK ExportThread(LPVOID param)
{
	WASABI_API_DIALOGBOXPARAMW(IDD_INFODIALOG, NULL, export_dlgproc, (LPARAM)param);
	return 0;
}

// -----------------------------------------------------------------------
// exports an iTunes XML library from gen_ml's database
// -----------------------------------------------------------------------
void exportDatabase() {
	// create an iTunes XML library writer
	iTunesXmlWrite w;

	// pick an output file
	if (w.pickFile(plugin.hwndLibraryParent)) {
		// if a file unsupported by iTunes is about to be exported, ask confirmation
		only_itunes = -1;

		// create status window
		int progress[2] = {0};
		DWORD threadId = 0;
		HANDLE exportThread = CreateThread(0, 0, ExportThread, progress, 0, &threadId);

		// create an iTunes DB in memory, start with a root key
		plistKey *rootKey = new plistKey(L"root");
		// add a dictionary to it
		plistDict *rootDict = new plistDict();
		rootKey->setData(rootDict);

		// add a few useless fields
		rootDict->addKey(new plistKey(L"Major Version", new plistInteger(1)));
		rootDict->addKey(new plistKey(L"Minor Version", new plistInteger(1)));
		rootDict->addKey(new plistKey(L"Application Version", new plistString(L"7.6.1"))); // we pretend to be iTunes 7.6.1

		// create the Tracks key and its dictionary of tracks
		plistDict *dict_tracks = new plistDict();
		rootDict->addKey(new plistKey(L"Tracks", dict_tracks));

		// run an empty query, so we get all items in the db
		mlQueryStructW query;
		query.query = L"";
		query.max_results = 0;
		query.results.Size = 0;
		query.results.Items = NULL;
		query.results.Alloc = 0;

		SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&query, ML_IPC_DB_RUNQUERYW);

		// my, what a big number of items...
		progress[1] = query.results.Size;

		// ... enumerate them
		for (int x = 0; x < progress[1]; x ++) {
			// take the xth item
			itemRecordW ir = query.results.Items[x];

			// check if it's supported by iTunes, if we've already answered the question, use last answer
			if ((only_itunes > 0 || only_itunes < 0) && (!endsIn(ir.filename, L".mp3") && !endsIn(ir.filename, L".m4a") && !endsIn(ir.filename, L".wav") && !endsIn(ir.filename, L".aiff"))) {
				if (only_itunes < 0) {
					// prompt user, should we export files unsupported by iTunes ?
					WASABI_API_DIALOGBOXW(IDD_EXPORTTYPE, plugin.hwndLibraryParent, exporttype_dlgproc);
				}
				// if not, continue with the next item
				if (only_itunes) continue;
			}

			// create a track key, its name is the number of the track (not counting skipped tracks)
			plistKey *key_track = new plistKey(StringPrintfW(L"%d", progress[0]));
			dict_tracks->addKey(key_track);

			// give it a dictionary to hold its properties
			plistDict *dict_track = new plistDict();
			key_track->setData(dict_track);

			// create the properties as needed
			dict_track->addKey(new plistKey(L"Track ID", new plistInteger(progress[0])));
			if (ir.title) dict_track->addKey(new plistKey(L"Name", new plistString(ir.title)));
			if (ir.artist) dict_track->addKey(new plistKey(L"Artist", new plistString(ir.artist)));
			if (ir.albumartist) dict_track->addKey(new plistKey(L"Album Artist", new plistString(ir.albumartist)));
			if (ir.album) dict_track->addKey(new plistKey(L"Album", new plistString(ir.album)));
			if (ir.genre) dict_track->addKey(new plistKey(L"Genre", new plistString(ir.genre)));
			if (ir.comment) dict_track->addKey(new plistKey(L"Comments", new plistString(ir.comment)));
			dict_track->addKey(new plistKey(L"Kind", new plistString(L"MPEG audio file")));

			// changed in 5.64 to use the 'realsize' if it's available, otherwise map to filesize scaled to bytes (stored as kb otherwise)
			const wchar_t *realsize = getRecordExtendedItem(&ir, L"realsize");
			if (realsize) dict_track->addKey(new plistKey(L"Size", new plistInteger(_wtoi64(realsize))));
			else if (ir.filesize > 0) dict_track->addKey(new plistKey(L"Size", new plistInteger(ir.filesize * 1024)));

			if (ir.length >= 0) dict_track->addKey(new plistKey(L"Total Time", new plistInteger(ir.length * 1000)));
			if (ir.track >= 0) dict_track->addKey(new plistKey(L"Track Number", new plistInteger(ir.track)));
			if (ir.year >= 0) dict_track->addKey(new plistKey(L"Year", new plistInteger(ir.year)));
			if (ir.filetime> 0) dict_track->addKey(new plistKey(L"Date Modified", new plistDate((time_t)ir.filetime)));
			if (ir.lastupd> 0) dict_track->addKey(new plistKey(L"Date Added", new plistDate((time_t)ir.lastupd)));
			//if (ir.lastplay> 0) dict_track->addKey(new plistKey(L"Play Date", new plistInteger((time_t)ir.lastplay)));
			if (ir.lastplay > 0) dict_track->addKey(new plistKey(L"Play Date UTC", new plistDate((time_t)ir.lastplay)));
			if (ir.bitrate> 0) dict_track->addKey(new plistKey(L"Bit Rate", new plistInteger(ir.bitrate)));
			if (ir.playcount> 0) dict_track->addKey(new plistKey(L"Play Count", new plistInteger(ir.playcount)));
			if (ir.rating> 0) dict_track->addKey(new plistKey(L"Rating", new plistInteger(ir.rating * 20)));
			if (ir.composer) dict_track->addKey(new plistKey(L"Composer", new plistString(ir.composer)));
			if (ir.publisher) dict_track->addKey(new plistKey(L"Publisher", new plistString(ir.publisher)));
			if (ir.type == 1) dict_track->addKey(new plistKey(L"Has Video", new plistInteger(1)));
			if (ir.disc> 0) dict_track->addKey(new plistKey(L"Disc Number", new plistInteger(ir.disc)));
			if (ir.discs> 0) dict_track->addKey(new plistKey(L"Disc Count", new plistInteger(ir.discs)));
			if (ir.tracks > 0) dict_track->addKey(new plistKey(L"Track Count", new plistInteger(ir.tracks)));
			if (ir.bpm > 0) dict_track->addKey(new plistKey(L"BPM", new plistInteger(ir.bpm)));
			const wchar_t *category = getRecordExtendedItem(&ir, L"category");
			if (category) dict_track->addKey(new plistKey(L"Grouping", new plistString(category)));
			const wchar_t *producer = getRecordExtendedItem(&ir, L"producer");
			if (producer) dict_track->addKey(new plistKey(L"Producer", new plistString(producer)));
			const wchar_t *director = getRecordExtendedItem(&ir, L"director");
			if (director) dict_track->addKey(new plistKey(L"Director", new plistString(director)));
			const wchar_t *width = getRecordExtendedItem(&ir, L"width");
			if (width) dict_track->addKey(new plistKey(L"Video Width", new plistInteger(_wtoi64(width))));
			const wchar_t *height = getRecordExtendedItem(&ir, L"height");
			if (height) dict_track->addKey(new plistKey(L"Video Height", new plistInteger(_wtoi64(height))));
			// convert the filename's backslashes to slashes 
			StringW s = ir.filename;

			if (WCSNICMP(s, L"http://", 7)) 
			{
				wchar_t *val = s.getNonConstVal();
				// TODO: we could do this with less malloc usage
				AutoChar utf8(val, CP_UTF8);
				String encoded = (const char *)utf8;
				Url::encode(encoded, 0, URLENCODE_EXCLUDEALPHANUM|URLENCODE_EXCLUDESLASH, Url::URLENCODE_STYLE_PERCENT);

				s = StringPrintfW(L"%s%s", ITUNES_FILENAME_HEADER, AutoWide(encoded, CP_UTF8));
			}

			// done
			dict_track->addKey(new plistKey(L"Location", new plistString(s)));
			dict_track->addKey(new plistKey(L"File Folder Count", new plistInteger(-1)));
			dict_track->addKey(new plistKey(L"Library Folder Count", new plistInteger(-1)));

			// we have one more item in our exported db
			progress[0]++;
		}

		// show "saving xml"
		progress[0]=-333;

		// free query results
		SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&query, ML_IPC_DB_FREEQUERYRESULTSW);

		// save the xml
		w.saveXml(rootKey);

		// done
		progress[0]=-666;
		if (exportThread)
		{
			WaitForSingleObject(exportThread, INFINITE);
			CloseHandle(exportThread);
		}

		// destroy the db, this deletes all the children too
		delete rootKey;
	}
}

//------------------------------------------------------------------------