#include "ImporterAPI.h"
#include "../xml/obj_xml.h"
#include "importer.h"
#include "resource.h"
#include "api__ml_impex.h"
#include "../plist/loader.h"
#include <api/service/waservicefactory.h>
#include <shlobj.h>

extern winampMediaLibraryPlugin plugin;

int Load(const wchar_t *filename, obj_xml *parser)
{
	if (!parser)
		return 1; // no sense in continuing if there's no parser available

	HANDLE file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return 1;

	while (true)
	{
		char data[1024] = {0};
		DWORD bytesRead = 0;
		if (ReadFile(file, data, 1024, &bytesRead, NULL) && bytesRead)
		{
			parser->xmlreader_feed(data, bytesRead);
		}
		else
			break;
	}

	CloseHandle(file);
	parser->xmlreader_feed(0, 0);
	return 0;
}

static bool GetiTunesPreferencesPath(wchar_t *path, size_t path_cch)
{
	wchar_t appdata[MAX_PATH] = {0};
	SHGetSpecialFolderPathW(NULL, appdata, CSIDL_APPDATA, FALSE);
	StringCchPrintf(path, path_cch, L"%s\\Apple Computer\\iTunes\\iTunesPrefs.xml", appdata);
	return true;
}

static bool GetiTunesLibraryPath(wchar_t *path, size_t path_cch)
{
	wchar_t itunes_pref[MAX_PATH] = {0};
	if (GetiTunesPreferencesPath(itunes_pref, MAX_PATH))
	{
		plistLoader it;

		obj_xml *parser=0;
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(obj_xmlGUID);
		if (factory)
			parser = (obj_xml *)factory->getInterface();

		if (parser)
		{
			// load the XML, this creates an iTunes DB in memory, and returns the	root key			
			parser->xmlreader_open();
			parser->xmlreader_registerCallback(L"plist\f*", &it);
			Load(itunes_pref, parser);
			parser->xmlreader_unregisterCallback(&it);
			parser->xmlreader_close();
			plistKey *root_key = &it;
			plistData *root_dict = root_key->getData();
			if (root_dict)
			{
				plistKey *prefs_key = ((plistDict*)root_dict)->getKey(L"User Preferences");
				if (prefs_key)
				{
					plistData *prefs_dict= prefs_key->getData();
					if (prefs_dict)
					{
						plistKey *location_key = ((plistDict*)prefs_dict)->getKey(L"iTunes Library XML Location:1");
						if (location_key)
						{
							plistData *location_data = location_key->getData();
							if (location_data)
							{
								plistRaw *location_data_raw = (plistRaw *)location_data;
								if (location_data_raw)
								{
									int size;
									const wchar_t *mem = (const wchar_t *)location_data_raw->getMem(&size);
									if (mem)
									{
										memcpy(path, mem, size);
										path[size/2]=0;
										return true;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return false;

}

// -----------------------------------------------------------------------
// import status window proc
// -----------------------------------------------------------------------
static BOOL CALLBACK import_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			// show import progress controls
			ShowWindow(GetDlgItem(hwndDlg, IDC_PROCESSING_STATE), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_PROGRESS_PERCENT), SW_SHOWNORMAL);
			ShowWindow(GetDlgItem(hwndDlg, IDC_TRACKS), SW_SHOWNORMAL);

			SetWindowText(hwndDlg,WASABI_API_LNGSTRINGW(IDS_IMPORTING_DATABASE));
			SetDlgItemText(hwndDlg, IDC_PROCESSING_STATE, WASABI_API_LNGSTRINGW(IDS_LOADING_XML));
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
				if (progress[0] == -666)
				{
					KillTimer(hwndDlg, 666);
					EndDialog(hwndDlg, 0);
				}
				else
				{
					// display progress
					SetDlgItemText(hwndDlg, IDC_TRACKS, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_TRACKS_IMPORTED_X), progress[0]));
					SendDlgItemMessage(hwndDlg, IDC_PROGRESS_PERCENT, PBM_SETPOS, (int)((double)progress[0]/progress[1]*100.0), 0);
				}
			}
			break;

		case WM_COMMAND:
		{
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

static DWORD CALLBACK ImportThread(LPVOID param)
{
	WASABI_API_DIALOGBOXPARAMW(IDD_INFODIALOG, NULL, import_dlgproc, (LPARAM)param);
	return 0;
}

int ImporterAPI::ImportFromFile(HWND parent, const wchar_t *library_file)
{
	// create an XML parser
	obj_xml *parser=0;
	waServiceFactory *factory = plugin.service->service_getServiceByGuid(obj_xmlGUID);
	if (factory)
		parser = (obj_xml *)factory->getInterface();

	if (parser)
	{
		// create status window
		int progress[2] = {0};
		DWORD threadId = 0;
		HANDLE importThread = CreateThread(0, 0, ImportThread, progress, 0, &threadId);

		// create an iTunes XML library reader
		plistLoader it;

		// load the XML, this creates an iTunes DB in memory, and returns the root key
		parser->xmlreader_open();
		parser->xmlreader_registerCallback(L"plist\f*", &it);
		Load(library_file, parser);
		parser->xmlreader_unregisterCallback(&it);
		parser->xmlreader_close();
		plistKey *root_key = &it;

		// we start at the root key
		if (root_key) {
			// the root key contains a dictionary
			plistData *root_dict = root_key->getData();
			if (root_dict && root_dict->getType() == PLISTDATA_DICT) {
				// that dictionary contains a number of keys, one of which contains a dictionary of tracks
				plistKey *tracks_key = ((plistDict*)root_dict)->getKey(L"Tracks");
				plistData *tracks_dict = tracks_key?tracks_key->getData():0;
				if (tracks_dict && tracks_dict->getType() == PLISTDATA_DICT) {
					// we have the tracks dictionary ...
					plistDict *tracks = (plistDict *)tracks_dict;
					progress[1]=tracks?tracks->getNumKeys():0;
					// ... now enumerate tracks
					for (int i=0;i<progress[1];i++) {
						// each track is a key in the tracks dictionary, and contains a dictionary of properties
						plistKey *track_key = tracks->enumKey(i);
						plistData *track_dict = track_key->getData();
						// prepare an item record
						itemRecordW ir; 
						MEMZERO(&ir, sizeof(ir));
						ir.year = ir.track = ir.length = -1;
						ir.lastplay = -1;
						ir.type = 0; // this makes it an Audio file (unless otherwise specified
						if (track_dict->getType() == PLISTDATA_DICT) {
							// we have the track's dictionary of properties...
							plistDict *track = (plistDict *)track_dict;
							int tn = track->getNumKeys();
							// ... now enumerate the properties
							for (int j=0;j<tn;j++) {
								plistKey *prop = track->enumKey(j);
								Importer_AddKeyToItemRecord(prop, ir);
							}
							// add or update the file
							SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&ir, ML_IPC_DB_ADDORUPDATEITEMW);
							// free the record
							freeRecord(&ir);
							// show progress
							++progress[0];
						}
					}
				}
			}
		}

		//done
		progress[0]=-666;
		if (importThread)
		{
			WaitForSingleObject(importThread, INFINITE);
			CloseHandle(importThread);
		}

		// tell gen_ml we modified the db
		SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, 0, ML_IPC_DB_SYNCDB);

		factory->releaseInterface(parser);
	}
	else
		return DISPATCH_FAILURE;
	return DISPATCH_SUCCESS;
}

bool ImporterAPI::iTunesExists()
{
	wchar_t itunes_path[MAX_PATH] = {0};
	if (GetiTunesPreferencesPath(itunes_path, MAX_PATH))
	{
		return GetFileAttributesW(itunes_path) != INVALID_FILE_ATTRIBUTES;
	}
	return false;
}

int ImporterAPI::ImportFromiTunes(HWND parent)
{
	wchar_t itunes_path[MAX_PATH] = {0};
	if (GetiTunesLibraryPath(itunes_path, MAX_PATH))
	{
		return ImportFromFile(parent, itunes_path);
	}
	return DISPATCH_FAILURE;	
}

int ImportPlaylists(HWND parent, const wchar_t *library_file);
int ImporterAPI::ImportPlaylistsFromiTunes(HWND parent)
{
	wchar_t itunes_path[MAX_PATH] = {0};
	if (GetiTunesLibraryPath(itunes_path, MAX_PATH))
	{
		return ImportPlaylists(parent, itunes_path);
	}
	return DISPATCH_FAILURE;	
}

#define CBCLASS ImporterAPI
START_DISPATCH;
CB(API_ITUNES_IMPORTER_ITUNESEXISTS, iTunesExists)
CB(API_ITUNES_IMPORTER_IMPORTFROMFILE, ImportFromFile)
CB(API_ITUNES_IMPORTER_IMPORTFROMITUNES, ImportFromiTunes)
CB(API_ITUNES_IMPORTER_IMPORTPLAYLISTSFROMITUNES, ImportPlaylistsFromiTunes)
END_DISPATCH;
#undef CBCLASS