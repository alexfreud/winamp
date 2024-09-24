#include "main.h"
#include "resource.h"
#include "api_mldb.h"
#include "../Winamp/strutil.h"

enum {
	STATUS_SEARCHING,
	STATUS_GETINFO,
	STATUS_DONE,
};

extern HWND g_bgrescan_status_hwnd;
extern nde_scanner_t m_media_scanner;

#define MAX_RECURSE_DEPTH 32
/* Event handles */
static HANDLE scan_killswitch=0;
static HANDLE scan_cancel=0;
static HANDLE scan_cancel_complete=0;
/* Thread handle */
static HANDLE scan_thread=0;
/* extension list */
static wchar_t *scan_extlist=0;

static void SyncTable()
{
	EnterCriticalSection(&g_db_cs);
	NDE_Table_Sync(g_table);
	g_table_dirty=0;
	LeaveCriticalSection(&g_db_cs);
}

static bool ScanCancelled(HANDLE *events, int count)
{
	// make sure no one cancelled us
	DWORD eventFired=WaitForMultipleObjectsEx(count, events, FALSE, 0, TRUE);
	if (eventFired >= WAIT_OBJECT_0 && eventFired < (WAIT_OBJECT_0+count))
		return true;

	return false;
}

static bool SupportedType(const wchar_t *ext)
{
	if (!scan_extlist) 
		scan_extlist=(wchar_t*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GET_EXTLISTW);

	// dunno how this would happen but should verify
	if (!scan_extlist || scan_extlist == (wchar_t *)1)
		return false;

	const wchar_t *a = scan_extlist;
	while (a && *a) 
	{
		if (CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE, a, -1, ext, -1) == CSTR_EQUAL) 
		{
			return true;
		}
		a+=wcslen(a)+1;
	}
	return false;
}

static void CountFolder(const wchar_t *folder, int recurse, HANDLE cancelswitch, volatile int *found)
{
	wchar_t filespec[MAX_PATH] = {0};
	PathCombineW(filespec, folder, L"*.*"); 

	WIN32_FIND_DATAW findData = {0};

	HANDLE h = FindFirstFileW(filespec, &findData);
	if (h != INVALID_HANDLE_VALUE)
	{
		if (IsWindow(g_bgrescan_status_hwnd))
		{
			wchar_t status[150+MAX_PATH] = {0};
			WASABI_API_LNGSTRINGW_BUF(IDS_SCANNING_DIR, status, 150);

			const wchar_t *p=folder+wcslen(folder);
			while (p > folder && *p != '\\') p--;
			p--;
			while (p >= folder && *p != '\\') p--;

			StringCbCatW(status, sizeof(status), ++p);

			SetWindowTextW(g_bgrescan_status_hwnd,status);
		}

		HANDLE events[2] = { scan_killswitch, cancelswitch};
		do
		{
			// make sure no one cancelled us
			if (ScanCancelled(events, 2))
				break;

			/* if it's a directory (And not either of the two special dirs */
			if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
				&& lstrcmpiW(findData.cFileName, L".") 
				&& lstrcmpiW(findData.cFileName, L".."))
			{
				if (recurse && recurse < MAX_RECURSE_DEPTH)
				{
					PathCombineW(filespec, folder, findData.cFileName);
					CountFolder(filespec, recurse+1, cancelswitch, found); // add 1 so we can verify recurse depth
				}
			}	

			if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				wchar_t *ext=extensionW(findData.cFileName);
				if (ext && ext[0] && SupportedType(ext))
				{
					PathCombineW(filespec, folder, findData.cFileName);
					if (IsWindow(g_bgrescan_status_hwnd))
					{
						wchar_t b[150+MAX_PATH] = {0};
						WASABI_API_LNGSTRINGW_BUF(IDS_SCANNING_FILE, b, 150);
						StringCbCatW(b, sizeof(b), filespec);
						SetWindowTextW(g_bgrescan_status_hwnd,b);
					}
					if (found)
						(*found)++;
				}
			}

		} while (FindNextFileW(h, &findData));
		FindClose(h);
	}
	else if (!(GetFileAttributesW(folder) & FILE_ATTRIBUTE_DIRECTORY))
	{
		if (IsWindow(g_bgrescan_status_hwnd))
		{
			wchar_t b[150+MAX_PATH] = {0};
			WASABI_API_LNGSTRINGW_BUF(IDS_SCANNING_FILE, b, 150);
			StringCbCatW(b, sizeof(b), folder);
			SetWindowTextW(g_bgrescan_status_hwnd,b);
		}
		if (found)
			(*found)++;
	}
}  

static void ScanFolder(const wchar_t *folder, int recurse, int metadata, int guessmode, HANDLE cancelswitch, volatile int *scanned)
{
	if ((unsigned long)folder < 65536) return;

	wchar_t filespec[MAX_PATH] = {0};
	wchar_t status[150+MAX_PATH] = {0};
	PathCombineW(filespec, folder, L"*.*"); 

	WIN32_FIND_DATAW findData = {0};

	HANDLE h = FindFirstFileW(filespec, &findData);
	if (h != INVALID_HANDLE_VALUE)
	{
		if (IsWindow(g_bgrescan_status_hwnd))
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_SCANNING_DIR, status, 150);

			const wchar_t *p=folder+wcslen(folder);
			while (p > folder && *p != '\\') p--;
			p--;
			while (p >= folder && *p != '\\') p--;

			StringCbCatW(status, sizeof(status), ++p);

			SetWindowTextW(g_bgrescan_status_hwnd,status);
		}

		HANDLE events[2] = { scan_killswitch, cancelswitch};
		do
		{
			// make sure no one cancelled us
			if (ScanCancelled(events, 2))
				break;

			/* if it's a directory (And not either of the two special dirs */
			if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
				&& lstrcmpiW(findData.cFileName, L".") 
				&& lstrcmpiW(findData.cFileName, L".."))
			{
				if (recurse && recurse < MAX_RECURSE_DEPTH)
				{
					PathCombineW(filespec, folder, findData.cFileName);
					ScanFolder(filespec, recurse+1, metadata, guessmode, cancelswitch, scanned); // add 1 so we can verify recurse depth
				}
			}

			if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				wchar_t *ext=extensionW(findData.cFileName);
				if (ext && ext[0] && SupportedType(ext))
				{
					PathCombineW(filespec, folder, findData.cFileName);
					if (IsWindow(g_bgrescan_status_hwnd))
					{
						WASABI_API_LNGSTRINGW_BUF(IDS_SCANNING_FILE, status, 150);
						StringCbCatW(status, sizeof(status), filespec);
						SetWindowTextW(g_bgrescan_status_hwnd,status);
					}
					addFileToDb(filespec, 0, metadata, guessmode);
					if (scanned)
						(*scanned)++;
				}
			}
		} while (FindNextFileW(h, &findData));
		FindClose(h);	
	}
	else if (!(GetFileAttributesW(folder) & FILE_ATTRIBUTE_DIRECTORY))
	{
		if (IsWindow(g_bgrescan_status_hwnd))
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_SCANNING_FILE, status, 150);
			StringCbCatW(status, sizeof(status), folder);
			SetWindowTextW(g_bgrescan_status_hwnd,status);
		}
		addFileToDb(folder, 0, metadata, guessmode);
		if (scanned)
			(*scanned)++;
	}
}

static DWORD CALLBACK ScanThreadProc(LPVOID param)
{
	/* sit and run APCs until we get signalled to die */
	HANDLE events[2] = { scan_killswitch, scan_cancel};
	int eventFired;
	do
	{
		eventFired=WaitForMultipleObjectsEx(2, events, FALSE, INFINITE, TRUE);
		switch(eventFired)
		{
		case WAIT_OBJECT_0+1: // cancel event
			ResetEvent(scan_cancel);
			SetEvent(scan_cancel_complete);
			break;
		}
	}
	while (eventFired != WAIT_OBJECT_0);

	if (scan_extlist && scan_extlist != (wchar_t *)1)
		GlobalFree((HGLOBAL)scan_extlist);
	scan_extlist=0;
	return 0;
}

static bool ScanCreateThread()
{
	if (!scan_thread)
	{
		/* create events */
		scan_killswitch = CreateEvent(NULL, TRUE, FALSE, NULL);
		scan_cancel = CreateEvent(NULL, TRUE, FALSE, NULL);
		scan_cancel_complete = CreateEvent(NULL, FALSE, FALSE, NULL); // auto-reset event

		/* start thread */
		scan_thread = CreateThread(NULL, 0, ScanThreadProc, 0, 0, 0);
	}

	return !!scan_thread;
}

void Scan_Cancel()
{
	HWND old = g_bgrescan_status_hwnd; // clear g_bgrescan_status_hwnd so that we don't deadlock when the BG thread calls SetWindowText
	g_bgrescan_status_hwnd = 0;
	if (scan_cancel)
		SignalObjectAndWait(scan_cancel, scan_cancel_complete, INFINITE, FALSE);
	g_bgrescan_status_hwnd = old;
}

void Scan_Kill()
{
	HWND old = g_bgrescan_status_hwnd; // clear g_bgrescan_status_hwnd so that we don't deadlock when the BG thread calls SetWindowText
	g_bgrescan_status_hwnd = 0;
	if (scan_thread)
		SignalObjectAndWait(scan_killswitch, scan_thread, INFINITE, FALSE);
	g_bgrescan_status_hwnd = old;
}

/*  ---------------
*  Scan_ScanFolder 
*  ---------------
*/

struct ScanFolderParams
{
	ScanFolderParams(const wchar_t *_path, int _guess, int _meta, int _recurse)
	{
		path = _wcsdup(_path);
		guess = _guess >= 0 ? _guess : g_config->ReadInt(L"guessmode",0);;
		meta = _meta >= 0 ? _meta : g_config->ReadInt(L"usemetadata",1);
		recurse = _recurse;
	}
	~ScanFolderParams()
	{
		free(path);
	}
	wchar_t *path;
	int guess;
	int meta;
	int recurse;
};

static VOID CALLBACK ScanFolderAPC(ULONG_PTR param)
{
	// clear extension list to get latest config
	if (scan_extlist && scan_extlist != (wchar_t *)1)
		GlobalFree((HGLOBAL)scan_extlist);
	scan_extlist = 0;

	ScanFolderParams *params = (ScanFolderParams *)param;
	ScanFolder(params->path, params->recurse, params->meta, params->guess, scan_cancel, 0);
	SyncTable();
	delete params;
}

void Scan_ScanFolderBackground(const wchar_t *path, int guess, int meta, int recurse)
{
	if (ScanCreateThread())
	{
		ScanFolderParams *params = new ScanFolderParams(path, guess, meta, recurse);
		if (QueueUserAPC(ScanFolderAPC, scan_thread, (ULONG_PTR)params) == 0)
			delete params;
	}
}

/*  ---------------
*  Scan_ScanFolders
*  ---------------
*/

struct ScanFoldersParams
{
	ScanFoldersParams(wchar_t **_path, size_t _count, int *_guess, int *_meta, int *_recurse)
	{
		path = _path;
		count = _count;
		guess = _guess;
		meta = _meta;
		recurse = _recurse;
		found = 0;
		scanned = 0;
		cancel_switch = CreateEvent(NULL, TRUE, FALSE, NULL);
		status = STATUS_SEARCHING;
		ui = 0;
		in_timer = 0;

	}
	~ScanFoldersParams()
	{
		for (size_t i=0;i!=count;i++)
			free(path[i]);
		free(path);
		free(guess);
		free(meta);
		free(recurse);
		CloseHandle(cancel_switch);
	}
	wchar_t **path;
	size_t count;
	int *guess;
	int *meta;
	int *recurse;
	volatile int found;
	volatile int scanned;
	volatile int status;
	int in_timer;
	HANDLE cancel_switch;
	HWND ui;
};

static VOID CALLBACK ScanFoldersAPC(ULONG_PTR param)
{
	// clear extension list to get latest config
	if (scan_extlist && scan_extlist != (wchar_t *)1)
		GlobalFree((HGLOBAL)scan_extlist);
	scan_extlist = 0;

	ScanFoldersParams *params = (ScanFoldersParams *)param;
	HANDLE events[2] = { scan_killswitch, params->cancel_switch};

	for (size_t i=0;i!=params->count;i++)
	{
		if (ScanCancelled(events, 2))
			break;

		CountFolder(params->path[i], params->recurse[i], params->cancel_switch, &params->found);
	}

	params->status = STATUS_GETINFO;

	for (size_t i=0;i!=params->count;i++)
	{
		if (ScanCancelled(events, 2))
			break;

		int guess = params->guess[i] >= 0 ? params->guess[i] : g_config->ReadInt(L"guessmode",0);;
		int meta = params->meta[i] >= 0 ? params->meta[i] : g_config->ReadInt(L"usemetadata",1);
		ScanFolder(params->path[i], params->recurse[i], meta, guess, params->cancel_switch, &params->scanned);
	}

	params->status = STATUS_DONE;
	PostMessage(params->ui, WM_APP, 0, 0);
}

static INT_PTR CALLBACK ScanFileUI(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	switch(uMsg) 
	{
		case WM_INITDIALOG:
		{
			SetDlgItemTextW(hwndDlg,IDC_STATUS,WASABI_API_LNGSTRINGW(IDS_INITIALIZING));

			ScanFoldersParams *params = (ScanFoldersParams *)lParam;
			params->ui = hwndDlg;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			if (QueueUserAPC(ScanFoldersAPC, scan_thread, (ULONG_PTR)lParam) == 0)
				EndDialog(hwndDlg, 0);
			else
			{
				SendDlgItemMessage(hwndDlg,IDC_PROGRESS1,PBM_SETRANGE,0,MAKELPARAM(0, 100));
				SetTimer(hwndDlg,0x123,300,NULL);
			}

			// show window and restore last position as applicable
			POINT pt = {g_config->ReadInt(L"scan_x", -1), g_config->ReadInt(L"scan_y", -1)};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
		}
		break;

		case WM_TIMER:
		{
			ScanFoldersParams *params = (ScanFoldersParams *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (params->in_timer) break;
			params->in_timer++;

			if(params->status==STATUS_SEARCHING)
			{
				wchar_t tmp[512] = {0};
				StringCchPrintfW(tmp, 512, WASABI_API_LNGSTRINGW(IDS_SEARCHING_X_FILES_FOUND), params->found);
				SetDlgItemTextW(hwndDlg,IDC_STATUS,tmp);
			}
			else if(params->status==STATUS_GETINFO) 
			{
				wchar_t tmp[512] = {0};
				int perc=params->found?(params->scanned*100/params->found):0;
				StringCchPrintfW(tmp, 512, WASABI_API_LNGSTRINGW(IDS_GETTING_INFO_FROM_FILES_PERCENT),perc);
				SetDlgItemTextW(hwndDlg,IDC_STATUS,tmp);
				SendDlgItemMessage(hwndDlg,IDC_PROGRESS1,PBM_SETPOS,perc,0);
			}
			params->in_timer--;
		}
		break;

		case WM_APP:
		{
			KillTimer(hwndDlg,0x123);
			SyncTable();
			EndDialog(hwndDlg,0);
		}
		break;

		case WM_COMMAND:
			if (LOWORD(wParam)==IDCANCEL) 
			{
				ScanFoldersParams *params = (ScanFoldersParams *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				SetEvent(params->cancel_switch);
			}
		break;

		case WM_DESTROY:
		{
			RECT scan_rect = {0};
			GetWindowRect(hwndDlg, &scan_rect);
			g_config->WriteInt(L"scan_x", scan_rect.left);
			g_config->WriteInt(L"scan_y", scan_rect.top);

			KillTimer(hwndDlg,0x123);
			ScanFoldersParams *params = (ScanFoldersParams *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
			delete params;
			return FALSE;
		}
	}
	return FALSE;
}

/* When you call this function, it will own the memory and release it with free() */
void Scan_ScanFolders(HWND parent, size_t count, wchar_t **paths, int *guess, int *meta, int *recurse)
{
	openDb();

	if (g_table && ScanCreateThread())
	{
		ScanFoldersParams *params = new ScanFoldersParams(paths, count, guess, meta, recurse);
		WASABI_API_LNG->LDialogBoxParamW(WASABI_API_LNG->FindDllHandleByGUID(WinampLangGUID),
										 GetModuleHandleW(L"winamp.exe"), IDD_ADDSTUFF,
										 parent, (DLGPROC)ScanFileUI, (LPARAM)params);
		PostMessage(plugin.hwndWinampParent,WM_WA_IPC,NDE_Table_GetRecordsCount(g_table),IPC_STATS_LIBRARY_ITEMCNT);
	}
	else
	{
		for (size_t i=0;i!=count;i++)
			free(paths[i]);
		free(paths);
	}
}

void Scan_ScanFolder(HWND parent, const wchar_t *path, int guess, int meta, int recurse)
{
	// kind of a hack ...
	if (ScanCreateThread())
	{
		wchar_t **paths = (wchar_t **)calloc(1, sizeof(wchar_t*));
		int *guesses = (int *)calloc(1, sizeof(int));
		int *metas = (int *)calloc(1, sizeof(int));
		int *recs = (int *)calloc(1, sizeof(int));
		*guesses = guess;
		*metas = meta;
		*recs = recurse;
		paths[0] = _wcsdup(path);
		Scan_ScanFolders(parent, 1, paths, guesses, metas, recs);
	}
}

static VOID CALLBACK BackgroundScanAPC(ULONG_PTR param)
{
	openDb();
	if (!g_table)
		return; 

	HANDLE events[2] = { scan_killswitch, scan_cancel};

	// clear extension list to get latest config
	if (scan_extlist && scan_extlist != (wchar_t *)1)
		GlobalFree((HGLOBAL)scan_extlist);
	scan_extlist = 0;

	// read list from config
	UINT codePage = CP_ACP;
	char scandirlist[65536] = {0};
	if (!g_config->ReadString("scandirlist", 0, scandirlist, 65536))
	{
		g_config->ReadString("scandirlist_utf8","", scandirlist, 65536);
		codePage = CP_UTF8;
	}

	AutoWide s1(scandirlist, codePage);
	size_t len = wcslen(s1)+2;
	wchar_t *s =(wchar_t*)calloc(len, sizeof(wchar_t));
	if (s)
	{
		lstrcpynW(s, s1, len);
		s[wcslen(s)+1]=0;

		wchar_t *p=s;
		while (p && *p == L'|') p++;

		while ((p=wcsstr(p,L"|")))
		{
			*p++=0;
			while (p && *p == L'|') p++;
		}
		p=s;

		// iterate through list
		while (p && *p && !ScanCancelled(events, 2))
		{
			while (p && *p == L'|') p++;

			int use_metadata=g_config->ReadInt(L"usemetadata",1);
			int guess_mode=g_config->ReadInt(L"guessmode",0);
			int recurse=1;
			if (*p == L'<' && wcsstr(p,L">"))
			{
				p++;
				while (p && *p != L'>') 
				{
					// <MmSs>can prefix directory
					// M=metadata use override
					// m=no metadata
					// S=smart guessing
					// s=stupid guessing
					if (*p == L'M') use_metadata=1;
					else if (*p == L'm') use_metadata=0;
					else if (*p == L'S') guess_mode=0;
					else if (*p == L's') guess_mode=1;
					else if (*p == L'r') recurse=0;
					else if (*p == L'g') guess_mode=2;
					p++;
				}
				p++;
			}
			ScanFolder(p, recurse, use_metadata, guess_mode, scan_cancel, 0);
			p+=wcslen(p)+1;
		}

		free(s);
	}

	/* Remove missing files */
	if (!ScanCancelled(events, 2) && g_config->ReadInt(L"bgrescan_compact",1))
	{
		EnterCriticalSection(&g_db_cs);
		nde_scanner_t scanner = NDE_Table_CreateScanner(g_table);
		NDE_Scanner_Query(scanner, L"");
		NDE_Scanner_First(scanner);
again:
		nde_field_t f=NDE_Scanner_GetFieldByID(scanner, MAINTABLE_ID_FILENAME);
		wchar_t *gs=0;
		if (f) 
		{
			gs = NDE_StringField_GetString(f);
			ndestring_retain(gs);
			if (GetFileAttributesW(gs) != INVALID_FILE_ATTRIBUTES)
			{
				NDE_Scanner_Next(scanner);
			}
			else
			{
				// Issue wasabi callback for pre removal
				WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_REMOVED_PRE, (size_t)gs, 0);
				
				NDE_Scanner_Delete(scanner);
				NDE_Scanner_Post(scanner);

				// Issue wasabi callback for pre removal
				WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_REMOVED_POST, (size_t)gs, 0);
			}
		}
		LeaveCriticalSection(&g_db_cs);

		if (f) // done checking for unused files
		{
			if (IsWindow(g_bgrescan_status_hwnd))
			{
				wchar_t b[150+MAX_PATH] = {0};
				WASABI_API_LNGSTRINGW_BUF(IDS_CHECKING_FOR_FILE, b, 150);
				StringCbCatW(b, sizeof(b), PathFindFileNameW(gs));
				SetWindowTextW(g_bgrescan_status_hwnd,b);
			}
			ndestring_release(gs);
			gs=0;
			if (!ScanCancelled(events, 2))
			{
				EnterCriticalSection(&g_db_cs);
				goto again;
			}
		}

		EnterCriticalSection(&g_db_cs);
		NDE_Table_DestroyScanner(g_table, scanner);
		LeaveCriticalSection(&g_db_cs);

		if (IsWindow(g_bgrescan_status_hwnd))
		{
			wchar_t b[150] = {0};
			WASABI_API_LNGSTRINGW_BUF(IDS_COMPACTING, b, 150);
			SetWindowTextW(g_bgrescan_status_hwnd,b);
		}
	}

	// TODO: hmmm, safe to do on separate thread?
	EnterCriticalSection(&g_db_cs);
	if (!ScanCancelled(events, 2))
	{
		wchar_t *last_query = NULL;
		if (m_media_scanner) 
		{
			const wchar_t *lq = NDE_Scanner_GetLastQuery(m_media_scanner);
			if (lq) last_query = _wcsdup(lq);
			NDE_Table_DestroyScanner(g_table, m_media_scanner);
		}
		NDE_Table_Sync(g_table); // this is currently b0rk3d -- fucko :)
		NDE_Table_Compact(g_table);
		g_table_dirty=0;
		if (m_media_scanner) 
		{
			m_media_scanner=NDE_Table_CreateScanner(g_table);
			if (last_query != NULL) 
			{
				NDE_Scanner_Query(m_media_scanner, last_query);
				free(last_query);
			}
		}

		PostMessage(plugin.hwndWinampParent,WM_WA_IPC,NDE_Table_GetRecordsCount(g_table),IPC_STATS_LIBRARY_ITEMCNT);
	}
	else
	{
		NDE_Table_Sync(g_table); // this is currently b0rk3d -- fucko :)		  
		g_table_dirty=0;
	}
	LeaveCriticalSection(&g_db_cs);
	if (IsWindow(g_bgrescan_status_hwnd))
		SetWindowTextW(g_bgrescan_status_hwnd,L"");
	g_bgscan_last_rescan = time(NULL);
	g_bgscan_scanning = 0;
}

void Scan_BackgroundScan()
{
	if (ScanCreateThread())
	{
		g_bgrescan_force = 0;
		g_bgscan_last_rescan = time(NULL);
		g_bgscan_scanning = 1;

		QueueUserAPC(BackgroundScanAPC, scan_thread, (ULONG_PTR)0);

	}
}

static void RemoveFiles(HANDLE cancelswitch, volatile int *found, volatile int *count, volatile int *scanned)
{
	// TODO: benski> we might need to keep the database lock the whole time. need to think it thru
	EnterCriticalSection(&g_db_cs);
	nde_scanner_t myscanner=NDE_Table_CreateScanner(g_table);
	NDE_Scanner_Query(myscanner, L"");
	NDE_Scanner_First(myscanner);
	*found=0;
	*scanned=0;
	*count=NDE_Table_GetRecordsCount(g_table);
	LeaveCriticalSection(&g_db_cs);

	HANDLE events[2] = { scan_killswitch, cancelswitch};

	bool fileRemoved = false;
	wchar_t *filename;
	while (!ScanCancelled(events, 2))
	{
		EnterCriticalSection(&g_db_cs);
		if (!NDE_Scanner_BOF(myscanner) && !NDE_Scanner_EOF(myscanner))
		{
			nde_field_t f= NDE_Scanner_GetFieldByID(myscanner, MAINTABLE_ID_FILENAME);
			if (f)
			{
				(*scanned)++;

				filename = (NDE_StringField_GetString(f));

				if (GetFileAttributesW(NDE_StringField_GetString(f)) != INVALID_FILE_ATTRIBUTES)
				{
					NDE_Scanner_Next(myscanner);
				}
				else
				{
					// Issue wasabi callback for pre removal
					WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_REMOVED_PRE, (size_t)filename, 0);

					//remove file
					NDE_Scanner_Delete(myscanner);
					NDE_Scanner_Post(myscanner);
					(*found)++;
					fileRemoved = true;
				}
			}
			else
			{
				//remove file
				NDE_Scanner_Delete(myscanner);
				NDE_Scanner_Post(myscanner);
				(*found)++;
				fileRemoved = false;
			}
		}
		else // last file
		{
			LeaveCriticalSection(&g_db_cs);
			break;
		}
		LeaveCriticalSection(&g_db_cs);

		if (fileRemoved)
		{
			// Issue wasabi callback for pre removal
			WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_REMOVED_POST, (size_t)filename, 0);
			fileRemoved = false;
		}
	}
	EnterCriticalSection(&g_db_cs);
	NDE_Table_DestroyScanner(g_table, myscanner); // important that we delete the scanner BEFORE 
	myscanner=0;

	wchar_t *last_query = NULL;
	if (m_media_scanner) 
	{
		const wchar_t *lq = NDE_Scanner_GetLastQuery(m_media_scanner);
		if (lq) last_query = _wcsdup(lq);
		NDE_Table_DestroyScanner(g_table, m_media_scanner);
	}
	NDE_Table_Sync(g_table); // this is currently b0rk3d -- fucko :)
	NDE_Table_Compact(g_table);
	g_table_dirty=0;
	if (m_media_scanner) 
	{
		m_media_scanner=NDE_Table_CreateScanner(g_table);
		if (last_query != NULL) 
		{
			NDE_Scanner_Query(m_media_scanner, last_query);
			free(last_query);
		}
	}
	LeaveCriticalSection(&g_db_cs);
}

struct RemoveFilesParams
{
	RemoveFilesParams()
	{
		found = 0;
		scanned = 0;
		total = 0;
		cancel_switch = CreateEvent(NULL, TRUE, FALSE, NULL);
		ui = 0;
		in_timer = 0;
	}
	~RemoveFilesParams()
	{

		CloseHandle(cancel_switch);
	}
	volatile int found;
	volatile int scanned;
	volatile int total;

	int in_timer;
	HANDLE cancel_switch;
	HWND ui;
};

static VOID CALLBACK RemoveFilesAPC(ULONG_PTR param)
{
	RemoveFilesParams *params = (RemoveFilesParams *)param;

	RemoveFiles(params->cancel_switch, &params->found, &params->total, &params->scanned);
	PostMessage(params->ui, WM_APP, 0, 0);
}

static INT_PTR CALLBACK RemoveFilesUI(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	switch(uMsg) 
	{
		case WM_INITDIALOG:
		{
			SetWindowTextW(hwndDlg,WASABI_API_LNGSTRINGW(IDS_REMOVING_FILES_NOT_EXISTING));
			SetDlgItemTextW(hwndDlg,IDC_STATUS,WASABI_API_LNGSTRINGW(IDS_INITIALIZING));

			RemoveFilesParams *params = (RemoveFilesParams *)lParam;
			params->ui = hwndDlg;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			if (QueueUserAPC(RemoveFilesAPC, scan_thread, (ULONG_PTR)lParam) == 0)
				EndDialog(hwndDlg, 0);
			else
			{
				SendDlgItemMessage(hwndDlg,IDC_PROGRESS1,PBM_SETRANGE,0,MAKELPARAM(0, 100));
				SetTimer(hwndDlg,0x123,300,NULL);
			}

			// show window and restore last position as applicable
			POINT pt = {g_config->ReadInt(L"scan_x", -1), g_config->ReadInt(L"scan_y", -1)};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
		}
		break;

		case WM_TIMER:
		{
			RemoveFilesParams *params = (RemoveFilesParams *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (params->in_timer) break;
			params->in_timer++;

			if(params->total) 
			{
				wchar_t tmp[512] = {0};
				int perc=(params->scanned*100/(params->total?params->total:1));
				StringCchPrintfW(tmp, 512, WASABI_API_LNGSTRINGW(IDS_SCANNING_X_OF_X_X_REMOVED),params->scanned,params->total,params->found);
				SetDlgItemTextW(hwndDlg,IDC_STATUS,tmp);
				SendDlgItemMessage(hwndDlg,IDC_PROGRESS1,PBM_SETPOS,perc,0);
			}

			params->in_timer--;
		}
		break;

		case WM_APP:
		{
			RemoveFilesParams *params = (RemoveFilesParams *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			KillTimer(hwndDlg,0x123);

			wchar_t tmp[512] = {0};
			int perc=(params->scanned*100/(params->total?params->total:1));
			StringCchPrintfW(tmp, 512, WASABI_API_LNGSTRINGW(IDS_SCANNED_X_FILES_X_REMOVED),params->total,params->found);
			SetDlgItemTextW(hwndDlg,IDC_STATUS,tmp);
			SendDlgItemMessage(hwndDlg,IDC_PROGRESS1,PBM_SETPOS,perc,0);

			SyncTable();
			EndDialog(hwndDlg,0);
		}
		break;

		case WM_COMMAND:
		if (LOWORD(wParam)==IDCANCEL) 
		{
			RemoveFilesParams *params = (RemoveFilesParams *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			SetEvent(params->cancel_switch);
		}
		break;
		
		case WM_DESTROY:
		{
			RECT scan_rect = {0};
			GetWindowRect(hwndDlg, &scan_rect);
			g_config->WriteInt(L"scan_x", scan_rect.left);
			g_config->WriteInt(L"scan_y", scan_rect.top);

			KillTimer(hwndDlg,0x123);
			RemoveFilesParams *params = (RemoveFilesParams *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
			delete params;
		}
		return FALSE;
	}
	return FALSE;
}

void Scan_RemoveFiles(HWND parent) 
{
	openDb();
	if (g_table && ScanCreateThread())
	{
		RemoveFilesParams *params = new RemoveFilesParams();
		WASABI_API_LNG->LDialogBoxParamW(WASABI_API_LNG->FindDllHandleByGUID(WinampLangGUID),
										 GetModuleHandleW(L"winamp.exe"), IDD_ADDSTUFF,
										 parent, (DLGPROC)RemoveFilesUI, (LPARAM)params);
		PostMessage(plugin.hwndWinampParent,WM_WA_IPC,NDE_Table_GetRecordsCount(g_table),IPC_STATS_LIBRARY_ITEMCNT);
	}
}