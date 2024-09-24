/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "../Plugins/General/gen_crasher/GetWinVer.h"
#include "resource.h"
#include "../Plugins/General/gen_ml/ml.h"
#include "../nu/AutoWide.h"
#include "api.h"
#include "ExplorerFindFile.h"
#include <assert.h>
#include "vis.h"
LRESULT sendMlIpc(int msg, WPARAM param);
int g_open_ml_item_in_pe = 0;
EXTERN_C BOOL eggTyping;

void showLibrary()
{
	SendMessageW(hMainWindow, WM_COMMAND, ID_FILE_SHOWLIBRARY, 0);
}

static void ToggleWindowShade_PL()
{
	if (config_pe_height == 14)
	{
		if (config_pe_height_ws < 116) config_pe_height_ws = 116;
		config_pe_height = config_pe_height_ws;
		config_pe_height_ws = 0;
	}
	else
	{
		config_pe_height_ws = config_pe_height;
		config_pe_height = 14;
	}
	set_aot(1);
}

static void ToggleWindowShade_EQ()
{
	config_eq_ws = !config_eq_ws;
	set_aot(1);
	draw_eq_tbar(GetForegroundWindow() == hEQWindow ? 1 : (config_hilite ? 0 : 1));
}

static void ToggleWindowShade_Main()
{
	config_windowshade = !config_windowshade;
	set_aot(1);
	draw_tbar(1, config_windowshade, eggstat);
	SendMessageW(hMainWindow, WM_TIMER, UPDATE_DISPLAY_TIMER + 4, 0);
	sa_setthread(config_sa);
}

// Big nasty command handler.
// it calls Main_OnButtonX() for the button controls
LRESULT Main_OnCommand(HWND hwnd, int wID, HWND hwndCtl, UINT codeNotify)
{
	if (codeNotify == THBN_CLICKED && !atti_present)
	{
		switch (wID)
		{
		case 0: //previous
			{
				wID = WINAMP_BUTTON1;
			}
			break;
		case 1: //play
			{
				wID = WINAMP_BUTTON2;
			}
			break;
		case 2: //pause
			{
				wID = WINAMP_BUTTON3;
			}
			break;
		case 3: //stop
			{
				wID = WINAMP_BUTTON4;
			}
			break;
		case 4: //next
			{
				wID = WINAMP_BUTTON5;
			}
			break;
		default:
			return 1;
		}
	}

	if (1 == codeNotify && eggTyping)
	{
		BYTE kb[256] = {0};
		if(GetKeyboardState(kb))
		{
			for (INT i = 0x30; i < 0x87; i++)
			{
				if (0 != (0x80 & kb[i]))
					SendMessageW(hwnd, WM_KEYDOWN, i, 0L);
				
			}
			if (0 != (0x80 & kb[0x20]))
				SendMessageW(hwnd, WM_KEYDOWN, 0x20, 0L);
		}
	}

	if (LOWORD(wID) >= 34768 && LOWORD(wID) < g_BookmarkTop)
	{
		FILE *fp = _wfopen(BOOKMARKFILE8, L"rt");
		if (fp)
		{
			int x = 0, id = LOWORD(wID)-34768;
			for (;;)
			{
				char ft[4096] = {0}, fn[MAX_PATH] = {0};
				fgets(fn, MAX_PATH, fp);
				if (feof(fp)) break;
				fgets(ft, 4096, fp);
				if (feof(fp)) break;
				if (ft[0] && fn[0])
				{
					if (fn[lstrlenA(fn) - 1] == '\n') fn[lstrlenA(fn) - 1] = 0;
					if (ft[lstrlenA(ft) - 1] == '\n') ft[lstrlenA(ft) - 1] = 0;
					if (ft[0] && fn[0])
					{
						if (x++ == id)
						{
							// this will track the gen_ml or main play/enqueue setting
							// done as something which koopa requested back in 09/2011
							// including following 'shift' to invert the default mode.
							int addtolist, enqplay = 0;
							if (got_ml)
							{
								enqplay = GetPrivateProfileIntW(L"MLEnqPlay", L"enqueuedef", -1, INI_FILE);
								if (enqplay != -1)
								{
									addtolist = (!!enqplay) ^ (!!(GetAsyncKeyState(VK_SHIFT)&0x8000));
								}
								else
								{
									addtolist = (!!GetPrivateProfileIntW(L"gen_ml_config", L"enqueuedef", 0, ML_INI_FILE)) ^ (!!(GetAsyncKeyState(VK_SHIFT)&0x8000));
								}
							}
							else
								addtolist = (!!config_addtolist) ^ (!!(GetAsyncKeyState(VK_SHIFT)&0x8000));

							if(!addtolist) PlayList_delete();
							PlayList_appendthing(AutoWideDup(fn, CP_UTF8), 0, 0);
							if(!addtolist)
							{
								if (config_shuffle) PlayList_setposition(PlayList_getNextPosition());
								else PlayList_setposition(0);
								PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
							}
							else
							{
								if (enqplay == 2) PlayList_setposition(PlayList_getlength());
							}
							plEditRefresh();
							if(!addtolist || enqplay == 2)
								StartPlaying();
							break;
						}
					}
				}
			}
			fclose(fp);
		}
		return 1;
	}

	if (LOWORD(wID) >= 32767 && LOWORD(wID) < g_SkinTop)
	{
		wchar_t curfilename[MAX_PATH] = {0};
		int id = LOWORD(wID);
		if (id >= 32768)
		{
			MENUITEMINFOW mi = {sizeof(mi), MIIM_DATA | MIIM_TYPE, MFT_STRING};
			mi.dwTypeData = curfilename;
			mi.cch = MAX_PATH;
			GetMenuItemInfoW(main_menu, id, FALSE, &mi);
			if (mi.dwItemData == 1)
				StringCchCatW(curfilename, MAX_PATH, L".zip");
			else if (mi.dwItemData == 2)
				StringCchCatW(curfilename, MAX_PATH, L".wsz");
			else if (mi.dwItemData == 4)
				StringCchCatW(curfilename, MAX_PATH, L".wal");
		}

		if (_wcsicmp(config_skin, curfilename))
		{
			StringCchCopyW(config_skin, MAX_PATH, curfilename);
			SendMessageW(hMainWindow, WM_COMMAND, WINAMP_REFRESHSKIN, 0);
			if (prefs_last_page == 40 && IsWindow(prefs_hwnd))
			{
				prefs_last_page = 0;
				prefs_dialog(1);
				prefs_last_page = 40;
				prefs_dialog(1);
			}
		}
		return 1;
	}

	if (LOWORD(wID) >= 34700 && LOWORD(wID) < g_LangTop && config_wlz_menu)
	{
		wchar_t curfilename[MAX_PATH]={0};
		int id = LOWORD(wID);
		if (id >= 34701)
		{
			MENUITEMINFOW mi = {sizeof(mi), MIIM_DATA | MIIM_TYPE, MFT_STRING};
			mi.dwTypeData = curfilename;
			mi.cch = MAX_PATH;
			GetMenuItemInfoW(main_menu, id, FALSE, &mi);

			if (mi.dwItemData == 1)
				StringCchCatW(curfilename, MAX_PATH, L".zip");
			else if (mi.dwItemData == 2)
				StringCchCatW(curfilename, MAX_PATH, L".wlz");
		}

		if (_wcsicmp(config_langpack, curfilename))
		{
			LangSwitchToLangPrompt(hMainWindow, curfilename);
		}
		return 1;
	}

	if (LOWORD(wID) >= 45000 && LOWORD(wID) < 55000)
	{
		int id = LOWORD(wID);
		HWND mlhwnd = (HWND)sendMlIpc(0, 0);
		if (mlhwnd != NULL)
		{
			if (!g_open_ml_item_in_pe && !IsWindowVisible(mlhwnd))
				SendMessageW(hMainWindow, WM_COMMAND, ID_FILE_SHOWLIBRARY, 0);

			// should switching the ml view to the target be disabled if we only want to play the playlist entry in the pe ?
			if (!g_open_ml_item_in_pe)
				SendMessageW(mlhwnd, WM_ML_IPC, id - 45000, ML_IPC_SETCURTREEITEM);
		}
		return 1;
	}

	switch (LOWORD(wID))
	{
	case ID_HELP_HELPTOPICS:
		myOpenURL(hwnd, L"https://help.winamp.com/hc/sections/8103009633300-Winamp-Desktop-Player-");
		break;
	case ID_HELP_FEEDBACK:
	{
		wchar_t url[512] = {0}, winVer[32] = {0}, build[32] = {0};
		int nWinVer = 0;
		GetWinVer(winVer, &nWinVer, build);
		// May 2022 - this services url is dead and we currently don't have a replacement
		StringCchPrintfW(url, 512, L"http://services.winamp.com/redirect/support?reason=Feedback&subject=Winamp Feedback&product=Winamp Desktop&v=%s Build %d&platform=%s (%s)",
						 WIDEN(APP_VERSION), BUILD_NUMBER, winVer, build);
		myOpenURL(hwnd, url);
		break;
	}
	case ID_VIS_NEXT:
	case ID_VIS_PREV:
	case ID_VIS_RANDOM:
	case ID_VIS_FS:
	case ID_VIS_CFG:
	case ID_VIS_MENU:
		if (hExternalVisWindow != NULL)
			SendMessageW(hExternalVisWindow, WM_COMMAND, LOWORD(wID) | (codeNotify << 16), 0);
		break;
	case EQ_ENABLE:
	case IDM_EQ_LOADPRE:
	case IDM_EQ_LOADMP3:
	case IDM_EQ_LOADDEFAULT:
	case ID_LOAD_EQF:
	case IDM_EQ_SAVEPRE:
	case IDM_EQ_SAVEMP3:
	case IDM_EQ_SAVEDEFAULT:
	case ID_SAVE_EQF:
	case IDM_EQ_DELPRE:
	case IDM_EQ_DELMP3:
		return SendMessageW(hEQWindow, WM_COMMAND, LOWORD(wID), 0);
	case ID_POST_PLAY_PLAYLIST: g_open_ml_item_in_pe = 0; return 0;
	case ID_MAIN_PLAY_AUDIOCD:
	case ID_MAIN_PLAY_AUDIOCD2:
	case ID_MAIN_PLAY_AUDIOCD3:
	case ID_MAIN_PLAY_AUDIOCD4:
		if (g_audiocdletter[LOWORD(wID) - ID_MAIN_PLAY_AUDIOCD])
		{
			wchar_t s[32] = {0};
			StopPlaying(0);
			StringCchPrintfW(s, 32, L"cda://%c", g_audiocdletter[LOWORD(wID) - ID_MAIN_PLAY_AUDIOCD]);
			PlayList_delete();
			PlayList_appendthing(s, 0, 0);
			BeginPlayback();
			plEditRefresh();
		}
		return 1;
	case ID_PE_OPEN:
		{
			if (!playlist_open(g_dialog_box_parent ? g_dialog_box_parent : hMainWindow)) break;
		}
		return 1;
	case ID_PE_SAVEAS:
		savepls(DIALOG_PARENT(hMainWindow));
		return 1;
	case WINAMP_EDIT_BOOKMARKS:
		MessageBoxW(hwnd, getStringW(IDS_ML_MISSING_FOR_BOOKMARKS,NULL,0), L"Winamp", MB_OK);
		return 1;
	case WINAMP_MAKECURBOOKMARK:
		{
			wchar_t fn[FILENAME_SIZE] = {0}, ft[FILETITLE_SIZE] = {0};
			PlayList_getitem2W(PlayList_getPosition(), fn, ft);
			Bookmark_additem(fn, ft);
		}
		return 1;
	case WINAMP_LIGHTNING_CLICK: about_dialog(); return 1;
	case WINAMP_NEW_INSTANCE:
		{
			char buf[MAX_PATH] = "\"";
			STARTUPINFO si = {sizeof(si), };
			PROCESS_INFORMATION pi;
			GetModuleFileNameA(NULL, buf + 1, sizeof(buf) - 1);
			StringCchCatA(buf, MAX_PATH, "\" /NEW");
			config_write(1);
			CreateProcessA(NULL, buf, NULL, NULL, FALSE, 0, NULL, NULL,(LPSTARTUPINFOA)&si, &pi);
		}
		return 1;
#if 0
	case WINAMP_VIS_OPTIONS:
		{
			extern int last_classic_skin_page;
			last_classic_skin_page = 1;
			prefs_last_page = 22;
			prefs_dialog(1);
		}
		return 1;
#endif
	case WINAMP_REFRESHSKIN:
		{
			g_skinloadedmanually = 1;
			SendMessageW(hwnd, WM_DISPLAYCHANGE, 0, 0);
			g_skinloadedmanually = 0;
		}
		if (config_pe_open) InvalidateRect(hPLWindow, NULL, FALSE);
#if 0
		if (config_mb_open)
		{
			RECT r;
			r.left = 0;r.top = 0;r.right = 11;r.bottom = config_mb_height;
			InvalidateRect(hMBWindow, &r, FALSE);
			r.left = 11;r.top = 0;r.right = config_mb_width;r.bottom = 20;
			InvalidateRect(hMBWindow, &r, FALSE);
			r.left = config_mb_width - 8;r.top = 20;r.right = config_mb_width;r.bottom = config_mb_height;
			InvalidateRect(hMBWindow, &r, FALSE);
			r.left = 11;r.top = config_mb_height - 38;r.right = config_mb_width - 8;r.bottom = config_mb_height;
			InvalidateRect(hMBWindow, &r, FALSE);
		}
#endif
		if (config_eq_open) InvalidateRect(hEQWindow, NULL, FALSE);
		if (config_video_open) InvalidateRect(hVideoWindow, NULL, FALSE);
		// plugin wnds get the WM_DISPLAYCHANGE which is all they need
		return 1;
	case WINAMP_SELSKIN:
		prefs_last_page = 40;
		prefs_dialog(1);
		return 1;
	case WINAMP_VISPLUGIN:
		if (vis_running()) vis_stop();
		else vis_start(hMainWindow, NULL);
		return 1;
	case WINAMP_PLGSETUP:
		prefs_last_page = 33;
		prefs_dialog(1);
		return 1;
	case WINAMP_TOGGLE_AUTOSCROLL:
		config_autoscrollname ^= 1;
		if (config_autoscrollname == 1) SetTimer(hwnd, UPDATE_DISPLAY_TIMER + 1, 200, NULL);
		else if (config_autoscrollname == 0)
		{
			KillTimer(hwnd, UPDATE_DISPLAY_TIMER + 1);
			ui_songposition = 0;
			draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
		}
		return 1;
	case WINAMP_EDIT_ID3:
		if (FileName[0]) in_infobox(DIALOG_PARENT(hMainWindow), FileName);
		return 1;
	case WINAMP_OPTIONS_WINDOWSHADE_PL:         // toggle windowshade
		ToggleWindowShade_PL();
		return 1;
	case WINAMP_OPTIONS_WINDOWSHADE_EQ:
		ToggleWindowShade_EQ();
		return 1;
	case WINAMP_OPTIONS_WINDOWSHADE:         // toggle windowshade
		ToggleWindowShade_Main();
		return 1;
	case WINAMP_OPTIONS_WINDOWSHADE_GLOBAL:         // toggle windowshade
		{
			HWND hFocus = GetForegroundWindow();
			if (hFocus == hPLWindow || IsChild(hFocus, hPLWindow))
				ToggleWindowShade_PL();
			else if (hFocus == hEQWindow || IsChild(hFocus, hEQWindow))
				ToggleWindowShade_EQ();
			else 
				ToggleWindowShade_Main();
		}
		return 1;
	case ID_PE_CLOSE:
	case WINAMP_MAIN_WINDOW:
		config_mw_open = !config_mw_open;
		if (config_mw_open)
		{
			if (config_pe_open && config_pe_width >= 350 && config_pe_height != 14)
			{
				RECT r = {config_pe_width - 150 - 75, config_pe_height - 26, r.left + 77, r.top + 16};
				InvalidateRect(hPLWindow, &r, FALSE);
			}
		}
		else
		{
			MoveOffscreen(hwnd);
		}
		//sa_setthread(-1);
		CheckMenuItem(main_menu, WINAMP_MAIN_WINDOW, config_mw_open ? MF_CHECKED : MF_UNCHECKED);
		set_aot(0);

		Ipc_WindowToggle(IPC_CB_WND_MAIN, config_mw_open);

	case WINAMP_NEXT_WINDOW:
		EnterCriticalSection(&embedcs);
		{
			embedWindowState *p = embedwndlist;
			int x;
			int state = 0;
			for (x = 0; ; x ++)
			{
				HWND dockwnd = NULL;
				int vis = 0;
				if ( x == 0 )
				{
					dockwnd = hMainWindow;
					vis = config_mw_open;
				}
				else if ( x == 1 )
				{
					dockwnd = hEQWindow;
					vis = config_eq_open;
				}
				else if ( x == 2 )
				{
					dockwnd = hPLWindow;
					vis = config_pe_open;
				}
//else if (x == 3) { /*dockwnd = hMBWindow; vis = 0;*/ } // config_mb_open; }
				else if ( x == 4 )
				{
					dockwnd = hVideoWindow;
					vis = config_video_open;
				}
				else
				{
					if ( !p )
					{
						if ( state == 1 )
						{
							p = embedwndlist;
							x = -1;
							state = 2;
							continue;
						}
						else
							break;
					}
					dockwnd = p->me;
					vis = IsWindowVisible( p->me );
				}

				if (!state)
				{
					HWND fg = GetForegroundWindow();
					if (fg == dockwnd || IsChild(dockwnd, fg))
					{
						state = 1;
					}
				}
				else if (vis)
				{
					SetForegroundWindow(dockwnd);
					break;
				}

				// changed for 5.58 to cope with x == 3 as this can otherwise cause
				// one embedwnd to stay as the active window irrespective of doing
				// additional ctrl+tab actions as it was not wanting to handle x==3
				if (x == 3 || x > 4)
					p = p->link;
			}
		}
		LeaveCriticalSection(&embedcs);
		return 1;
	case WINAMP_HELP_ABOUT:         // about box
		about_dialog();
		return 1;
	case WINAMP_PE_SEARCH:
	case WINAMP_JUMPFILE:
		jump_file_dialog(hMainWindow);
		return 1;
	case WINAMP_TOGGLE_LIBRARY:
		sendMlIpc(ML_IPC_TOGGLE_VISIBLE, 0);
		return 1;
	case WINAMP_JUMP:
		if (playing && in_mod && in_mod->is_seekable)
		{
			jump_dialog(hMainWindow);
		}
		return 1;
	case WINAMP_FFWD5S:         // left and right arrows, fastforward and rewind by 5 seconds
	case WINAMP_REW5S:
		{
			int i;
			if (!in_mod || !in_mod->is_seekable || PlayList_ishidden(PlayList_getPosition())) return 1;
			if (LOWORD(wID) == WINAMP_FFWD5S) i = 5000;
			else i = -5000;
			if (playing)
			{
				int t;
				t = in_getouttime() + i;
				if (t < 0) t = 0;
				if (in_seek(t) < 0)
					SendMessageW(hwnd, WM_WA_MPEG_EOF, 0, 0);
				else
				{
					ui_drawtime(in_getouttime() / 1000, 0);
				}
			}
		}
		return 1;
	case WINAMP_MAINMENU:         // alt-f for mainmenu
		{
			POINT p = { 6, 13 };
			if (config_dsize)
			{
				p.x *= 2;
				p.y *= 2;
			}
			ClientToScreen(hwnd, &p);
			if ( /*p.x > 3000  || */p.y > OFFSCREEN_Y_POS && p.y < (OFFSCREEN_Y_POS + 2000) )
				GetCursorPos(&p);
			DoTrackPopup(main_menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, hwnd);
		}
		return 1;
	case WINAMP_PREVSONG:
		{
			int s = 1;
			if (!config_shuffle && PlayList_advance( -1) < 0)
			{
				s = 0;
				if (config_repeat)
				{
					s = 1;
					PlayList_advance(BIGINT);
				}
			}
			if (s)
			{
				if (PlayList_getlength())
				{
					if (config_shuffle)
					{
						if (PlayList_randpos( -1)) return 1;
					}
					PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
					if (playing)
					{
						StopPlaying(0);
						StartPlaying();
					}
					else StopPlaying(0);
				}
			}
		}
		return 0;
	case WINAMP_BUTTON1_SHIFT:         // button 1 (previous), sent from either windowshade,
	case WINAMP_BUTTON1_CTRL:          // keyboard shortcut, or normal buttons (ui.c)
	case WINAMP_BUTTON1:
		return (Main_OnButton1(hwnd, LOWORD(wID), hwndCtl, codeNotify));
	case WINAMP_BUTTON2_SHIFT:         // button 2 (play)
	case WINAMP_BUTTON2_CTRL:
	case WINAMP_BUTTON2:
		return (Main_OnButton2(hwnd, LOWORD(wID), hwndCtl, codeNotify));
	case WINAMP_BUTTON3:               // button 3 (pause)
		return (Main_OnButton3(hwnd, LOWORD(wID), hwndCtl, codeNotify));
	case WINAMP_BUTTON4_CTRL:         // button 4 (stop)
	case WINAMP_BUTTON4_SHIFT:         // button 4 (stop)
	case WINAMP_BUTTON4:
		return (Main_OnButton4(hwnd, LOWORD(wID), hwndCtl, codeNotify));
	case WINAMP_BUTTON5_CTRL:         // button 5 (next)
	case WINAMP_BUTTON5_SHIFT:         // button 5 (next)
	case WINAMP_BUTTON5:
		return (Main_OnButton5(hwnd, LOWORD(wID), hwndCtl, codeNotify));
	case WINAMP_FILE_QUIT:          // keyboard Alt-X or menu option
		if (g_exit_disabled) return 0;
		SendMessageW(hwnd, WM_CLOSE, 0, 0);
		return 1;
	case WINAMP_FILE_LOC:          // open location
		getNewLocation(1, DIALOG_PARENT(hwnd));
		plEditRefresh();
		return 1;
	case WINAMP_FILE_PLAY:         // open file(s)
		getNewFile(1,        /*NULL*/DIALOG_PARENT(hwnd), 0);
		plEditRefresh();
		return 1;
	case WINAMP_FILE_DIR:         // open directory
		{
			BROWSEINFOW bi = {0};
			wchar_t name[MAX_PATH] = {0};
			bi.hwndOwner = DIALOG_PARENT(hwnd);
			bi.pszDisplayName = name;
			bi.lpszTitle = L"__foo";
			bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
			bi.lpfn = BrowseCallbackProc;
			ITEMIDLIST *idlist = SHBrowseForFolderW(&bi);
			if (idlist)
			{
				wchar_t path[MAX_PATH] = {0};
				SHGetPathFromIDListW( idlist, path );
				Shell_Free(idlist);
				WASABI_API_APP->path_setWorkingPath(path);
				PlayList_delete();
				PlayList_adddir(path, (config_rofiob&2) ? 0 : 1);
				if (config_rofiob&1) PlayList_sort(2, 0);
				BeginPlayback();
				plEditRefresh();
			}
		}
		return 1;
	case WINAMP_OPTIONS_PREFS:         // open preferences
		prefs_dialog(0);
		return 1;
	case WINAMP_OPTIONS_AOT:         // toggle always on top
		config_aot = !config_aot;
		set_aot(0);
		return 1;
	case WINAMP_OPTIONS_DSIZE:         // toggle doublesize mode
		config_dsize = !config_dsize;
		if (!config_bifont && config_bifont_alt) draw_reinit_plfont(1);
		set_aot(1);
		return 1;
	case WINAMP_OPTIONS_EASYMOVE:         // toggle easymove (easy window moving)
		config_easymove = !config_easymove;
		CheckMenuItem(main_menu, WINAMP_OPTIONS_EASYMOVE, config_easymove ? MF_CHECKED : MF_UNCHECKED);
		return 1;
	case WINAMP_FILE_SHUFFLE:         // toggle shuffle (and draw button)
		config_shuffle = !config_shuffle;
		CheckMenuItem(main_menu, WINAMP_FILE_SHUFFLE, config_shuffle ? MF_CHECKED : MF_UNCHECKED);
		draw_shuffle(config_shuffle, 0);
		if (config_shuffle) PlayList_updaterandpos();
		if (config_shuffle && !playing)
		{
			PlayList_randpos( -BIGINT);
			PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
			//draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
		}
		draw_songname(getStringW((config_shuffle ? IDS_SHUFFLE_ON : IDS_SHUFFLE_OFF), NULL, 0), &ui_songposition, -2);
		return 1;
	case WINAMP_FILE_REPEAT:         // toggle repeat
	{
		config_repeat = !config_repeat;
		CheckMenuItem(main_menu, WINAMP_FILE_REPEAT, config_repeat ? MF_CHECKED : MF_UNCHECKED);
		draw_repeat(config_repeat, 0);

		UINT id = 0;
		if (!config_repeat) id = IDS_REPEAT_OFF;
		else if (config_repeat && config_pladv) id = IDS_REPEAT_PL;
		else if (config_repeat && !config_pladv) id = IDS_REPEAT_TRACK;
		if (id) draw_songname(getStringW(id, NULL, 0), &ui_songposition, -2);
		return 1;
	}
	case WINAMP_FILE_MANUALPLADVANCE:         // toggle manual playlist advance
	{
		config_pladv = !config_pladv;
		UpdateManualAdvanceState();

		UINT id = 0;
		if (!config_repeat) id = (config_pladv ? IDS_MAN_ADV_OFF : IDS_MAN_ADV_ON);
		else if (config_repeat && config_pladv) id = IDS_REPEAT_PL;
		else if (config_repeat && !config_pladv) id = IDS_REPEAT_TRACK;
		if (id) draw_songname(getStringW(id, NULL, 0), &ui_songposition, -2);
		return 1;
	}
	case WINAMP_OPTIONS_EQ:         // toggle EQ window
		eq_dialog(hwnd,0);
		return 1;
	case WINAMP_OPTIONS_PLEDIT:         // toggle playlist editor
		// if we get a 2 then it came from gen_ff so that we can work around a docking/position issue
		pleditDlg(hwnd,codeNotify==2);
		return 1;
	case WINAMP_OPTIONS_VIDEO:
		config_video_open ? HideVideoWindow(1) : ShowVideoWindow(0);
		return 1;
	case WINAMP_OPTIONS_TOGTIME:
		config_timeleftmode = !config_timeleftmode;
		CheckMenuItem(main_menu, WINAMP_OPTIONS_ELAPSED, config_timeleftmode ? MF_UNCHECKED : MF_CHECKED);
		CheckMenuItem(main_menu, WINAMP_OPTIONS_REMAINING, config_timeleftmode ? MF_CHECKED : MF_UNCHECKED);
		SendMessageW(hwnd, WM_TIMER, UPDATE_DISPLAY_TIMER + 4, 0);
		return 1;
	case WINAMP_OPTIONS_ELAPSED:         // set time mode to elapsed
		config_timeleftmode = 0;
		CheckMenuItem(main_menu, WINAMP_OPTIONS_ELAPSED, MF_CHECKED);
		CheckMenuItem(main_menu, WINAMP_OPTIONS_REMAINING, MF_UNCHECKED);
		SendMessageW(hwnd, WM_TIMER, UPDATE_DISPLAY_TIMER + 4, 0);
		return 1;
	case WINAMP_OPTIONS_REMAINING:         // set time mode to remaining
		config_timeleftmode = 1;
		CheckMenuItem(main_menu, WINAMP_OPTIONS_ELAPSED, MF_UNCHECKED);
		CheckMenuItem(main_menu, WINAMP_OPTIONS_REMAINING, MF_CHECKED);
		SendMessageW(hwnd, WM_TIMER, UPDATE_DISPLAY_TIMER + 4, 0);
		return 1;
	case WINAMP_VOLUMEUP:		// increase volume by ~2%
	case WINAMP_VOLUMEDOWN:		// decrease volume by ~2%
		if (LOWORD(wID) == WINAMP_VOLUMEUP)
		{
			if (config_volume <= 251) config_volume += 4;
			else config_volume = 255;
		}
		else
		{
			if (config_volume > 3) config_volume -= 4;
			else config_volume = 0;
		}

		in_setvol(config_volume);
		draw_volumebar(config_volume, 0);
		update_volume_text(-2);
		return 1;
	case WINAMP_JUMP10BACK:
		{
			int x;
			int i = playing;
			if (i) StopPlaying(0);
			for (x = 0; x < 10; x ++)
				SendMessageW(hwnd, WM_COMMAND, WINAMP_BUTTON1, 0);
			if (i) StartPlaying();
		}
		return 1;
	case WINAMP_JUMP10FWD:
		{
			int x;
			int i = playing;
			if (i) StopPlaying(0);
			for (x = 0; x < 10; x ++)
				SendMessageW(hwnd, WM_COMMAND, WINAMP_BUTTON5, 0);
			if (i) StartPlaying();
		}
		return 1;
	case WINAMP_GETMORESKINS:
		myOpenURLWithFallback(hwnd, L"http://www.google.com/search?q=Winamp+Skins", L"http://www.google.com/search?q=Winamp+Skins");
		return 0;
	case WINAMP_VISCONF:
		{
			wchar_t b[MAX_PATH] = {0};
			HINSTANCE hLib;
			PathCombineW(b, VISDIR, config_visplugin_name);
			hLib = LoadLibraryW(b);
			if (hLib)
			{
				winampVisModule *module = NULL;
				winampVisGetHeaderType pr = (winampVisGetHeaderType) GetProcAddress(hLib, "winampVisGetHeader");
				winampVisHeader* pv = pr(hMainWindow);
				if (pv) module = pv->getModule(config_visplugin_num);
				if (module)
				{
					module->hDllInstance = hLib;
					module->hwndParent = hMainWindow;
					if (!(config_no_visseh&1))
					{
						__try {
						    module->Config(module);
						}
						__except(EXCEPTION_EXECUTE_HANDLER)
						{
							LPMessageBox(hwnd, IDS_PLUGINERROR, IDS_ERROR, MB_OK | MB_ICONEXCLAMATION);
						}
					}
					else
					{
						module->Config(module);
					}
				}
				else
				{
					LPMessageBox(hwnd, IDS_ERRORLOADINGPLUGIN, IDS_ERROR, MB_OK);
				}
				FreeLibrary(hLib);
			}
		}
		return 1;
	case WINAMP_MINIMIZE:
		if (GetAsyncKeyState(VK_SHIFT) >> 15)
			minimize_hack_winamp = 1;
		else
			minimize_hack_winamp = 0;
		ShowWindow(hMainWindow, SW_MINIMIZE);
		return 1;
	case WINAMP_VIDEO_TOGGLE_FS:
		videoToggleFullscreen();
		return 1;
	case ID_MLFILE_NEWPLAYLIST:
		{
			HWND h = (HWND)sendMlIpc(0, 0);
			HWND fgw = GetForegroundWindow();
			if (h && IsWindow(h) && IsWindowVisible(h) && IsChild(fgw, h))
				sendMlIpc(ML_IPC_NEWPLAYLIST, (WPARAM)h);
			else sendMlIpc(ML_IPC_NEWPLAYLIST, (WPARAM)(g_dialog_box_parent ? g_dialog_box_parent : hMainWindow));
			showLibrary();
		}
		return 1;
	case ID_MLFILE_SAVEPLAYLIST:
		showLibrary();
		{
			HWND h = (HWND)sendMlIpc(0, 0);
			HWND fgw = GetForegroundWindow();
			if (h && IsWindow(h) && IsWindowVisible(h) && IsChild(fgw, h))
				sendMlIpc(ML_IPC_SAVEPLAYLIST, (WPARAM)h);
			else sendMlIpc(ML_IPC_SAVEPLAYLIST, (WPARAM)(g_dialog_box_parent ? g_dialog_box_parent : hMainWindow));
		}
		return 1;
	case ID_MLFILE_LOADPLAYLIST:
		{
			HWND h = (HWND)sendMlIpc(0, 0);
			HWND fgw = GetForegroundWindow();
			if (h && IsWindow(h) && IsWindowVisible(h) && IsChild(fgw, h))
				sendMlIpc(ML_IPC_IMPORTPLAYLIST, (WPARAM)h);
			else
				sendMlIpc(ML_IPC_IMPORTPLAYLIST, (WPARAM)(g_dialog_box_parent ? g_dialog_box_parent : hMainWindow));
			showLibrary();
		}
		return 1;
	case ID_MLFILE_IMPORTCURRENTPLAYLIST:
		sendMlIpc(ML_IPC_IMPORTCURRENTPLAYLIST, 0);
		showLibrary();
		return 1;
	case ID_MLVIEW_MEDIA:
#define TREE_LOCALMEDIA 1000
		showLibrary();
		sendMlIpc(ML_IPC_SETCURTREEITEM, TREE_LOCALMEDIA);
		return 1;
	case ID_MLVIEW_PLAYLISTS:
#define TREE_PLAYLISTS 3001
		showLibrary();
		sendMlIpc(ML_IPC_SETCURTREEITEM, TREE_PLAYLISTS);
		return 1;
	case ID_MLVIEW_DEVICES:
#define TREE_DEVICES 10000
		showLibrary();
		sendMlIpc(ML_IPC_SETCURTREEITEM, TREE_DEVICES);
		return 1;
	case ID_MLVIEW_INTERNETRADIO:
#define TREE_INTERNET_RADIO 11000
		showLibrary();
		sendMlIpc(ML_IPC_SETCURTREEITEM, TREE_INTERNET_RADIO);
		return 1;
	case ID_MLVIEW_INTERNETTV:
#define TREE_INTERNET_VIDEO 11001
		showLibrary();
		sendMlIpc(ML_IPC_SETCURTREEITEM, TREE_INTERNET_VIDEO);
		return 1;
	case ID_MLVIEW_LIBRARYPREFERENCES:
		showLibrary();
		sendMlIpc(ML_IPC_OPENPREFS, 0);
		return 1;
	case ID_PLAY_VOLUMEUP:          // increase by ~ 10%
		{
			int a = 5;
			while (a--) SendMessageW(hwnd, WM_COMMAND, WINAMP_VOLUMEUP, 0);
		}
		return 1;
	case ID_PLAY_VOLUMEDOWN:         // decrease by ~10%
		{
			int a = 5;
			while (a--)
				SendMessageW(hwnd, WM_COMMAND, WINAMP_VOLUMEDOWN, 0);
		}
		return 1;

	case ID_RATING5: return setCurrentRating(5);
	case ID_RATING4: return setCurrentRating(4);
	case ID_RATING3: return setCurrentRating(3);
	case ID_RATING2: return setCurrentRating(2);
	case ID_RATING1: return setCurrentRating(1);
	case ID_RATING0: return setCurrentRating(0);
	case ID_PL_RATING5: return setPlRating(5);
	case ID_PL_RATING4: return setPlRating(4);
	case ID_PL_RATING3: return setPlRating(3);
	case ID_PL_RATING2: return setPlRating(2);
	case ID_PL_RATING1: return setPlRating(1);
	case ID_PL_RATING0: return setPlRating(0);

	case ID_PE_FFOD:
		{
			wchar_t fn[FILENAME_SIZE] = {0};
			if (!PlayList_getitem2W(PlayList_getPosition(), fn, NULL))
			{
				explorerFindFileManager->AddFile(fn);
				explorerFindFileManager->ShowFiles();
			}
		}
		return 1;
	} // switch()
	return 0;
} // Main_OnCommand()