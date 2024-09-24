#include "main.h"
#include "api__ml_local.h"
#include "resource.h"
#include "../nu/listview.h"

#define WINAMP_FILE_ADDTOLIBRARY        40344
#define WINAMP_FILE_ADDCURRENTPLEDIT	40466
#define WINAMP_SHOWLIBRARY              40379
wchar_t *recent_fn = 0;
static HMENU last_viewmenu = 0;
WORD waMenuID = 0;

extern W_ListView resultlist;
extern volatile int no_lv_update;
void UpdateLocalResultsCache(const wchar_t *filename);

static void onPlayFileTrack()
{
	int autoaddplays = g_config->ReadInt(L"autoaddplays", 0);
	int trackplays = g_config->ReadInt(L"trackplays", 1);
	if (!trackplays && !autoaddplays)
		return ;

	if (g_table && recent_fn)
	{
		wchar_t filename2[MAX_PATH] = {0}; // full lfn path if set
		EnterCriticalSection(&g_db_cs);

		nde_scanner_t s = NDE_Table_CreateScanner(g_table);
		int found=FindFileInDatabase(s, MAINTABLE_ID_FILENAME, recent_fn, filename2);

		if (found) // if it's in the table already
		{
			if (trackplays)	// if we're tracking plays
			{
				NDE_Scanner_Edit(s);
				nde_field_t f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_PLAYCOUNT);
				int cnt = f ? NDE_IntegerField_GetValue(f) : 0;
				time_t t = time(NULL);

				db_setFieldInt(s, MAINTABLE_ID_PLAYCOUNT, ++cnt);
				db_setFieldInt(s, MAINTABLE_ID_LASTPLAY, (int)t);
				if (asked_for_playcount)
					PostMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_UPDTITLE);

				// Issue a wasabi system callback after we have successfully added a file in the ml database
				api_mldb::played_info info = {t, cnt};
				WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_PLAYED, (size_t)recent_fn, (size_t)&info);

				NDE_Scanner_Post(s);
				// disabled in 5.65 (was added in 5.64?) as causing
				// noticeable ui lockups on sync with large library
				// so instead we'll risk it and flush on 50 instead
				/*g_table_dirty = 0;
				NDE_Table_Sync(g_table);*/
			}
			NDE_Table_DestroyScanner(g_table, s);
		}
		else  // not found in main table, we are in the main table, time to add if set
		{
			NDE_Table_DestroyScanner(g_table, s);

			if (autoaddplays)
			{
				int guess = -1, meta = -1, rec = 1;
				autoscan_add_directory(recent_fn, &guess, &meta, &rec, 1); // use this folder's guess/meta options
				if (guess == -1) guess = g_config->ReadInt(L"guessmode", 0);
				if (meta == -1)	meta = g_config->ReadInt(L"usemetadata", 1);
				addFileToDb(recent_fn, 0, meta, guess, 1, (int)time(NULL));
			}
		}

		if (g_table_dirty > 50)
		{
			g_table_dirty = 0;
			NDE_Table_Sync(g_table);
		}

		LeaveCriticalSection(&g_db_cs);
	}
}

void onStartPlayFileTrack(const wchar_t *filename, bool resume)
{
	if (!(wcsstr(filename, L"://") && _wcsnicmp(filename, L"cda://", 6) && _wcsnicmp(filename, L"file://", 7)))
	{
		int timer = -1, timer1 = -1, timer2 = -1;

		KillTimer(plugin.hwndWinampParent, 8081);
		if (!resume)
		{
			free(recent_fn);
			recent_fn = _wcsdup(filename);
		}

		// wait for x seconds
		if(g_config->ReadInt(L"trackplays_wait_secs",0))
		{
			timer1 = g_config->ReadInt(L"trackplays_wait_secs_lim",5)*1000;
		}

		// wait for x percent of the song (approx to a second)
		if(g_config->ReadInt(L"trackplays_wait_percent",0))
		{
			basicFileInfoStructW bfiW = {0};
			bfiW.filename = recent_fn;
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&bfiW, IPC_GET_BASIC_FILE_INFOW);
			if(bfiW.length > 0)
			{
				bfiW.length=bfiW.length*1000;
				timer2 = (bfiW.length*g_config->ReadInt(L"trackplays_wait_percent_lim",50))/100;
			}
		}
		
		// decide on which playback option will be the prefered duration (smallest wins)
		if(timer1 != -1 && timer2 != -1)
		{
			if(timer1 > timer2)
			{
				timer = timer2;
			}
			if(timer2 > timer1)
			{
				timer = timer1;
			}
		}
		else if(timer1 == -1 && timer2 != -1)
		{
			timer = timer2;
		}
		else if(timer2 == -1 && timer1 != -1)
		{
			timer = timer1;
		}

		// if no match or something went wrong then try to ensure the default timer value is used
		SetTimer(plugin.hwndWinampParent, 8081, ((timer > 0)? timer : 350), NULL);
	}
}

static void FileUpdated(const wchar_t *filename)
{
	if (g_table)
	{
		EnterCriticalSection(&g_db_cs);
		int guess = -1, meta = -1, rec = 1;
		autoscan_add_directory(filename, &guess, &meta, &rec, 1); // use this folder's guess/meta options
		if (guess == -1) guess = g_config->ReadInt(L"guessmode", 0);
		if (meta == -1)	meta = g_config->ReadInt(L"usemetadata", 1);
		addFileToDb(filename, TRUE, meta, guess, 0, 0, true);
		LeaveCriticalSection(&g_db_cs);

		if (g_table_dirty > 10)
		{
			g_table_dirty = 0;
			EnterCriticalSection(&g_db_cs);
			NDE_Table_Sync(g_table);
			LeaveCriticalSection(&g_db_cs);
		}

		if (!no_lv_update)
			UpdateLocalResultsCache(filename);
	}
	ClearCache(filename);
	ClearTitleHookCache();
}

LRESULT APIENTRY wa_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_TIMER:
			if (recent_fn && wParam == 8081)
			{
				if (SendMessage(hwndDlg, WM_WA_IPC, 0, IPC_GETOUTPUTTIME) > 350)
				{
					KillTimer(hwndDlg, 8081);
					if (SendMessage(hwndDlg, WM_WA_IPC, 0, IPC_ISPLAYING) == 1)
					{
						onPlayFileTrack();
					}
					free(recent_fn);
					recent_fn = 0;
				}
			}
			break;
		case WM_WA_IPC:
			switch (lParam)
			{
				case IPC_CB_MISC:
					if (wParam == IPC_CB_MISC_TITLE)
					{
						if(!SendMessage(hwndDlg, WM_WA_IPC, 0, IPC_ISPLAYING))
						{
							KillTimer(hwndDlg, 8081);
						}
					}
					else if (wParam == IPC_CB_MISC_PAUSE)
					{
						KillTimer(hwndDlg, 8081);
					}
					else if (wParam == IPC_CB_MISC_UNPAUSE)
					{
						if (recent_fn) onStartPlayFileTrack(recent_fn, true);
					}
					break;
				case IPC_FILE_TAG_MAY_HAVE_UPDATEDW:
					{
						wchar_t *filename = (wchar_t *)wParam;
						if (filename)
							FileUpdated(filename);
					}
					break;
				case IPC_FILE_TAG_MAY_HAVE_UPDATED:
					{
						char *filename = (char *)wParam;
						if (filename)
							FileUpdated(AutoWide(filename));
					}
					break;
				case IPC_STOPPLAYING:
					{
						KillTimer(hwndDlg, 8081);
						free(recent_fn);
						recent_fn = 0;
					}
					break;
				case IPC_GET_EXTENDED_FILE_INFO_HOOKABLE:
					// guessing for metadata for when the library isn't open yet.
					if (!g_table && wParam && !m_calling_getfileinfo) return doGuessProc(hwndDlg, uMsg, wParam, lParam);
					break;
				default:
					{
						if (lParam == IPC_CLOUD_ENABLED)
						{
							if (m_curview_hwnd) SendMessage(m_curview_hwnd, WM_APP + 6, 0, 0); //update current view
						}
					}
					break;
			}
			break;
		case WM_INITMENUPOPUP:
			{
				HMENU hmenuPopup = (HMENU) wParam;
				if (hmenuPopup == wa_play_menu)
				{
					if (last_viewmenu)
					{
						RemoveMenu(wa_play_menu, waMenuID, MF_BYCOMMAND);
						DestroyMenu(last_viewmenu);
						last_viewmenu = NULL;
					}

					mlGetTreeStruct mgts = { 1000, 55000, -1};
					last_viewmenu = (HMENU)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM) & mgts, ML_IPC_GETTREE);
					if (last_viewmenu)
					{
						if (GetMenuItemCount(last_viewmenu) > 0)
						{
							int count = GetMenuItemCount(wa_play_menu);
							MENUITEMINFOW menuItem = {sizeof(MENUITEMINFOW), MIIM_SUBMENU | MIIM_ID | MIIM_TYPE, MFT_STRING, MFS_ENABLED, waMenuID,
													 last_viewmenu, NULL, NULL, NULL, WASABI_API_LNGSTRINGW(IDS_MEDIA_LIBRARY_VIEW_RESULTS), 0};
							InsertMenuItemW(wa_play_menu, count, TRUE, &menuItem);
						}
						else
						{
							DestroyMenu(last_viewmenu);
							last_viewmenu = 0;
						}
					}
				}
			}
			break;
		case WM_COMMAND:
		case WM_SYSCOMMAND:
			WORD lowP = LOWORD(wParam);
			if (lowP == WINAMP_FILE_ADDTOLIBRARY)
			{
				if (!plugin.hwndLibraryParent || !IsWindowVisible(plugin.hwndLibraryParent))
				{
					SendMessage(plugin.hwndWinampParent, WM_COMMAND, MAKEWPARAM(WINAMP_SHOWLIBRARY, 0), 0L); 
				}
				add_to_library(plugin.hwndLibraryParent);
			}
			else if (lowP == WINAMP_FILE_ADDCURRENTPLEDIT)
			{
				add_pledit_to_library();
			}
			else if (uMsg == WM_COMMAND && wParam > 45000 && wParam < 55000)
			{
				int n = wParam - 45000;
				if (m_query_list[n])
				{
					mediaLibrary.SwitchToPluginView(n);
					return 0;
				}
			}
			else if (uMsg == WM_COMMAND && wParam > 55000 && wParam < 65000)
			{
				int n = wParam - 55000;
				if (m_query_list[n])
				{
					queryItem *item = m_query_list[n];
					wchar_t configDir[MAX_PATH] = {0};
					PathCombineW(configDir, g_viewsDir, item->metafn);
					C_Config viewconf(configDir);
					main_playQuery(&viewconf, item->query, 0, 1);
					return 0;
				}
			}
			break;
	}
	return CallWindowProcW(wa_oldWndProc, hwndDlg, uMsg, wParam, lParam);
}