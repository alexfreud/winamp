/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "Main.h"
#include "language.h"
#define APSTUDIO_INVOKED
#include "resource.h"

#include "../Plugins/General/gen_ml/ml.h"
#include "../Plugins/General/gen_ff/ff_ipc.h"
#include "../nu/ns_wc.h"
#include "menuv5.h"
#include "vis.h"
#include "minizip/unzip.h"
#include "wa_dlg.h"
#include "CurrentSongCOM.h"
#include "SkinCOM.h"
#include "ExternalCOM.h"
#include "api.h"

#include "tagz.h"
#include "Browser.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoWideFn.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoCharFn.h"
#include <api/syscb/callbacks/metacb.h>

#include "./skinwindow.h"
#include "JSAPI2_ExternalObject.h"
#include "JSAPI2_CallbackManager.h"

#include "stats.h"

#include "..\WAT\WAT.h"

extern std::vector<prefsDlgRec*> g_piprefsdlgs;
int unique_loword_command = _APS_NEXT_COMMAND_VALUE;

//extern "C" wa_inflate_struct inflate_struct;
wa_inflate_struct inflate_struct =
{
	(int (*)(void *))inflateReset,
		(int (*)(void *, const char *, int))inflateInit_,
		(int (*)(void *, int))inflate,
		(int (*)(void *))inflateEnd,
		crc32,
};

LRESULT wa_register_ipc(WPARAM data)
{
	if (data)
	{
		typedef struct _regipcrec
		{
			char *name;
			struct _regipcrec *next;
		}
		regipcrec;
		int cnt = 65537;
		static regipcrec *root = NULL;
		regipcrec *p = root, *lp = root;
		while (p)
		{
			if (!lstrcmpiA(p->name, (char*)data)) break;
			lp = p;
			p = p->next;
			cnt++;
		}
		if (!p)
		{
			if (!lp) // first item
			{
				root = (regipcrec *)calloc(1, sizeof(regipcrec));
				if (!root) return 0;
				root->next = 0;
				root->name = _strdup((char*)data);
			}
			else
			{
				lp->next = (regipcrec *)calloc(1, sizeof(regipcrec));
				if (!lp->next) return 0;

				lp->next->next = 0;
				lp->next->name = _strdup((char*)data);
			}
		}
		return cnt;
	}
	return 0;
}

// used to delay / filter out quick IPC_SETDIALOGBOXPARENT messages
// to try to prevent the aero-peek buttons failing / part loading
#define AEROPEEK 0xC0DE+4
VOID CALLBACK DialogBoxParentChanged(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	OnTaskbarButtonCreated(FALSE);
}

// IPC handler (for frontends and ourself)
LRESULT Main_OnIPC(HWND hwnd, int which, WPARAM data)
{
	LRESULT returnValue = 0;
	if (VideoIPCProcedure(which, data, &returnValue))
		return returnValue;

	static int ldata;
	static char fn[FILENAME_SIZE];
	static wchar_t fnW[FILENAME_SIZE];
	switch (which)
	{
		case IPC_GET_API_SERVICE:
			return reinterpret_cast<LRESULT>((api_service *)WASABI_API_SVC);
		case IPC_USE_REGISTRY:
			return !config_no_registry;
		case IPC_GET_DISPATCH_OBJECT:
			{
				ExternalCOM *external;
				return (SUCCEEDED(JSAPI1_GetExternal(&external))) ? reinterpret_cast<LRESULT>((IDispatch*)external) : NULL;
			}
		case IPC_REGISTER_LOWORD_COMMAND:
			if (data == 0)
				return unique_loword_command++;
			else
			{
				LRESULT temp = unique_loword_command;
				unique_loword_command+=data;
				return temp;
			}
		case IPC_METADATA_CHANGED:
			{
				CurrentSongCOM *currentSongCOM;
				if(SUCCEEDED(JSAPI1_GetCurrentSongCOM(&currentSongCOM)))
				{
					currentSongCOM->MetadataChanged((char *)data);
					currentSongCOM->Release();
				}
				break;
			}
		case IPC_FF_ONCOLORTHEMECHANGED:
		case IPC_SKIN_CHANGED:
			{
				WADlg_init(hMainWindow);
				JSAPI1_SkinChanged();
				break;
			}
		case IPC_GET_RANDFUNC:
			switch(data)
			{
				case 0:
					return reinterpret_cast<LRESULT>(warand);
				case 1 :
					return reinterpret_cast<LRESULT>(warandf);
			}
		break;
		case IPC_USE_UXTHEME_FUNC:
			switch(data)
			{
				case IPC_ISWINTHEMEPRESENT:
					return !IsWinXPTheme();
				case IPC_ISAEROCOMPOSITIONACTIVE:
					return !IsAero();
				default:
					if(IsWindow((HWND)data))
						DoWinXPStyle((HWND)data);
				return 0;
			}
		break;
		case IPC_USES_RECYCLEBIN:
			return config_playlist_recyclebin;
		case IPC_IS_AOT:
			return config_aot;
		case IPC_IS_EXIT_ENABLED:
			return g_exit_disabled == 0;
		case IPC_PUSH_DISABLE_EXIT:
			g_exit_disabled++;
			return g_exit_disabled;
		case IPC_POP_DISABLE_EXIT:
			g_exit_disabled--;
			return g_exit_disabled;
		case IPC_REGISTER_WINAMP_IPCMESSAGE:
			return wa_register_ipc(data);
		case IPC_WRITECONFIG:
			if (data >= 0 && data <= 2) config_write(data);
			return 0;
		case IPC_HOOK_OKTOQUIT:
			// do nothing, but let people hook it
			return 1;
		case IPC_ALLOW_PLAYTRACKING:
			// keep a track of the old state since we can use it to see
			// if we need to ignore showing the video window on tag edit
			last_no_notify_play = no_notify_play;
			no_notify_play = !data;
			break;
		case IPC_SETDRAWBORDERS:
			disable_skin_borders = !data;
			break;
		case IPC_DISABLESKINCURSORS:
			disable_skin_cursors = data;
			break;
		case IPC_GETSKINCURSORS:
			return ((INT)data >= 0 && (INT)data < N_CURSORS && config_usecursors && !disable_skin_cursors) ? (LRESULT)Skin_Cursors[(INT)data] : NULL;
		case IPC_STATS_LIBRARY_ITEMCNT:
			stats.SetStat(Stats::LIBRARY_SIZE, data);
			return 0;
		case IPC_GETOUTPUTPLUGIN:
			return reinterpret_cast<LRESULT>(config_outname);
		case IPC_GETLANGUAGEPACKINSTANCE:
			// changed to check on LOWORD(data) in 5.51+ since we can take extra params in hiword for data==5
			if (LOWORD(data) == 1) return (language_pack_instance == hMainInstance);
			else if (LOWORD(data) == 2) return reinterpret_cast<LRESULT>(LANGDIR);
			else if (LOWORD(data) == 3) return reinterpret_cast<LRESULT>(lang_directory);
			else if (LOWORD(data) == 4)
			{
				static wchar_t lngfilename[MAX_PATH] = {0};
				// cache the result once done - no point in re-generating it when not needed
				if(!lngfilename[0])
				{
					if(config_langpack[0])
					{
						// strip off the lng/wlz from the file so it matches with the config display
						lstrcpynW(lngfilename, config_langpack, MAX_PATH);
						wchar_t* p = extensionW(lngfilename);
						if(p) *CharPrevW(lngfilename, p) = 0;
					}
					else
					{
						// otherwise set it to the default
						lstrcpynW(lngfilename, L"English (US)", MAX_PATH);
					}
				}
				return reinterpret_cast<LRESULT>(lngfilename);
			}
			else if (LOWORD(data) == 5)
			{
				return reinterpret_cast<LRESULT>(langManager->GetLanguageIdentifier(HIWORD(data)));
			}
			return reinterpret_cast<LRESULT>(language_pack_instance);
		case IPC_SET_PE_WIDTHHEIGHT:
			{
				POINT *pt = (POINT *)data;
				config_pe_width = pt->x;
				if (config_pe_height != 14)
				{
					config_pe_height = pt->y;
					if (hPLVisWindow)
					{
						int x, y, w, h;
						x = config_pe_width - 150 - 75 + 2;
						y = config_pe_height - 26;
						w = (config_pe_width >= 350 && config_pe_height != 14 ? 72 : 0);
						h = 16;
						SetWindowPos(hPLVisWindow, 0, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
					}
				}
				else
					config_pe_height_ws = pt->y;

				// update the position of the tooltip on window resize
				set_pl_wnd_tooltip();
				return 0;
			}
		case IPC_TRANSLATEACCELERATOR:
			return 0;
		case IPC_FORMAT_TITLE:
			if (data)
			{
				waFormatTitle *p = (waFormatTitle*)data;
				FormatTitle(p);
			}
			return 0;
		case IPC_FORMAT_TITLE_EXTENDED:
			if (data)
			{
				waFormatTitleExtended *p = (waFormatTitleExtended *)data;
				FormatTitleExtended(p);
			}
			return 0;
		case IPC_HOOK_TITLES:
			return 0;
		case IPC_HOOK_TITLESW:
			if (data)
			{
				waHookTitleStructW *hts = (waHookTitleStructW *)data;
				return FormatTitle(hts);
			}
			// hookable
			return 0;
		case IPC_FF_ISMAINWND:
			return 0;
		case IPC_ISVISRUNNING:
			return vis_running();
		case IPC_SETRATING:
		{
			LRESULT ret = sendMlIpc(ML_IPC_SETRATING, data);
			if (!ret)
			{
				wchar_t fn[FILENAME_SIZE] = {0};
				if (!PlayList_getitem2W(PlayList_getPosition(), fn, NULL))
				{
					wchar_t buf[64] = {0};
					if (data > 0)
						StringCchPrintfW(buf, 64, L"%d", data);
					else
						buf[0] = 0;

					in_set_extended_fileinfoW(fn, L"rating", buf);
					in_write_extended_fileinfo();
				}
			}
			return ret;
		}
		case IPC_GETRATING:
		{
			LRESULT ret = sendMlIpc(ML_IPC_GETRATING, 0);

			// deal with no ml being present and querying the rating of a file from the tag (if possible)
			// as well as getting a zero rating which could mean that it's not present in the library
			if (!ret && !got_ml)
			{
				wchar_t fn[FILENAME_SIZE] = {0};
				if (!PlayList_getitem2W(PlayList_getPosition(), fn, NULL))
				{
					wchar_t buf[64] = {0};
					in_get_extended_fileinfoW(fn, L"rating", buf, 64);
					ret = _wtoi(buf);
				}
			}
			return ret;
		}
		case IPC_GETVISWND:
			return reinterpret_cast<LRESULT>(hExternalVisWindow);
		case IPC_SETVISWND:
			if (hExternalVisWindow != (HWND)data)
			{
				hExternalVisWindow = (HWND)data;
				vis_setextwindow(hExternalVisWindow);
			}
			break;
		case IPC_GETTIMEDISPLAYMODE:
			return config_timeleftmode;
		case IPC_ISDOUBLESIZE:
			return config_dsize;
		case IPC_SPAWNEQPRESETMENU:
			if (data)
			{
				waSpawnMenuParms *p = (waSpawnMenuParms*)data;
				int x = DoTrackPopup(GetSubMenu(top_menu, 1), TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD, p->xpos, p->ypos, p->wnd);
				if (x) SendMessageW(hEQWindow, WM_COMMAND, x, 0);
			}
			return 0;
		case IPC_SETPLEDITCOLORS:
			if (data)
			{
				waSetPlColorsStruct *p = (waSetPlColorsStruct *)data;
				if (p->numElems && p->elems) memcpy(Skin_PLColors, p->elems, 4*min(p->numElems, sizeof(Skin_PLColors) / sizeof(Skin_PLColors[0])));
				if (p->bm) draw_set_plbm(p->bm);
				draw_reinit_plfont(0);
				if (config_pe_open) InvalidateRect(hPLWindow, NULL, FALSE);
			}
			return 0;
		case IPC_ADDBOOKMARK:
			{
				static char *narrowBMfile=0;
				static char *narrowBMfile8=0;
				if (!data && !narrowBMfile)
					narrowBMfile = _strdup(AutoCharFn(BOOKMARKFILE));
				if(data == 666 && !narrowBMfile8)
					narrowBMfile8 = _strdup(AutoCharFn(BOOKMARKFILE8));
				return (data == 666?reinterpret_cast<LRESULT>(narrowBMfile8):reinterpret_cast<LRESULT>(narrowBMfile));
			}
		case IPC_ADDBOOKMARKW:
			return (data == 666?reinterpret_cast<LRESULT>(BOOKMARKFILE8):reinterpret_cast<LRESULT>(BOOKMARKFILE));
		case IPC_OPENPREFSTOPAGE:
			if (data != -1) prefs_last_page = data;
			prefs_dialog(1);
			return 0;
		case IPC_ISMAINWNDVISIBLE: return config_mw_open;
		case IPC_GETSADATAFUNC:
			if (data == 0) return reinterpret_cast<LRESULT>(export_sa_get_deprecated);
			else if (data == 1) return reinterpret_cast<LRESULT>(export_sa_setreq);
			else if (data == 2) return reinterpret_cast<LRESULT>(export_sa_get);
			return 0;
		case IPC_GETVUDATAFUNC:
			if (data == 0) return reinterpret_cast<LRESULT>(export_vu_get);
		return 0;
		case IPC_CB_MISC:
			switch (data)
			{
				case IPC_CB_MISC_STATUS:
					if (NULL != g_dialog_box_parent)
					{
						wchar_t title[512] = {0};
						GetWindowTextW(hMainWindow, title, sizeof(title));
						SetWindowTextW(g_dialog_box_parent, title);
					}
					break;
			}
			return 0;
		case IPC_CB_GETTOOLTIP:
			return 0;
		case IPC_CB_GETTOOLTIPW:
			return 0;
		case IPC_GETWND:
			if (data == IPC_GETWND_EQ) return reinterpret_cast<LRESULT>(hEQWindow);
			if (data == IPC_GETWND_PE) return reinterpret_cast<LRESULT>(hPLWindow);
			//if (data == IPC_GETWND_MB) return reinterpret_cast<LRESULT>(hMBWindow);
			//if (data == IPC_GETWND_VIDEO) return reinterpret_cast<LRESULT>(hVideoWindow);
			return 0;
		case IPC_ISWNDVISIBLE:
			if (data == IPC_GETWND_EQ) return config_eq_open;
			if (data == IPC_GETWND_PE) return config_pe_open;
			//if (data == IPC_GETWND_MB) return config_mb_open;
			//if (data == IPC_GETWND_VIDEO) return config_video_open;
			return 0;
		case IPC_GET_EXTLIST:
			if (data == 1) return reinterpret_cast<LRESULT>(in_getfltstr());
			return reinterpret_cast<LRESULT>(in_getextlist());
		case IPC_GET_EXTLISTW:
			if (data == 1) return reinterpret_cast<LRESULT>(in_getfltstrW(FALSE));
			return reinterpret_cast<LRESULT>(in_getextlistW());
		case IPC_GET_PLAYLIST_EXTLISTW:
		{
			if (!data)
			{
				static wchar_t extStr[1024] = {0};
				if (!extStr[0])
				{
					playlistManager->GetExtensionList(extStr, ARRAYSIZE(extStr));
				}
				return reinterpret_cast<LRESULT>(extStr);
			}
			else if (data == 1)
			{
				static wchar_t fltStr[1024] = {0};
				if (!fltStr[0])
				{
					playlistManager->GetFilterList(fltStr, ARRAYSIZE(fltStr));
				}
				return reinterpret_cast<LRESULT>(fltStr);
			}
			else if (data == 2)
			{
				// TODO if a playlist writer service is implemented then update
				//		this so we're not using a hard-coded implementation
				static wchar_t extSaveStr[1024] = {L"*.M3U;*.PLS;*.M3U8\0\0"};
				return reinterpret_cast<LRESULT>(extSaveStr);
			}
			else if (data == 3)
			{
				// TODO if a playlist writer service is implemented then update
				//		this so we're not using a hard-coded implementation
				static wchar_t fltSaveStr[1024] = {0};
				if (!fltSaveStr[0])
				{
					wchar_t *fStr = fltSaveStr;
					ZeroMemory(fltSaveStr, sizeof(fltSaveStr));
					getStringW(IDS_PLAYLIST_FILTER_STRING, fltSaveStr, MAX_PATH);
					// store the filter string with | separators so that parts can be translated as needed
					// (\0 or more \0000 will work but it's not reliable to work with that - the pipe is a
					// a work around from mfc apps wanting to do what we want for language support)
					while(fStr && *fStr)
					{
						wchar_t* rfsStr = 0;
						if(*fStr == L'|')
						{
							rfsStr = fStr;
						}
						fStr = CharNextW(fStr);
						if(rfsStr)
						{
							*rfsStr = 0;
						}
					}
				}
				return reinterpret_cast<LRESULT>(fltSaveStr);
			}
			return 0;
		}
		case IPC_GET_GENSKINBITMAP:
			{
				if (!data) return reinterpret_cast<LRESULT>(draw_LBitmap(MAKEINTRESOURCE(IDB_GENEX), L"genex.bmp"));

				if (data == 1)
				{
					return reinterpret_cast<LRESULT>(GetFontName());
				}
				else if (data == 2)
				{
					return atoi(getString(IDS_PLFONT_CHARSET, NULL, 0));
				}
				else if (data == 3)
				{
					return ScaleY(config_pe_fontsize);
				}
				else if (data == 4)
				{
					return Skin_PLColors[1];
				}
				return 0;
			}
		case IPC_REMOVE_PREFS_DLG:
			{
				prefsDlgRec *t = (prefsDlgRec *)data;

				//g_piprefsdlgs.eraseObject(t);
				auto it = std::find(g_piprefsdlgs.begin(), g_piprefsdlgs.end(), t);
				if (it != g_piprefsdlgs.end())
				{
					g_piprefsdlgs.erase(it);
				}

				prefs_liveDlgRemove(t);
			}
			return 0;
		case IPC_ADD_PREFS_DLG:
		case IPC_ADD_PREFS_DLGW:
			{
				prefsDlgRec *p = (prefsDlgRec *)data;
				p->next = (which == IPC_ADD_PREFS_DLG ? 0 : PREFS_UNICODE);

				// we use the dialog proc for the preferences to determine the hinstance of the module and
				// use that to then determine if we have gen_crasher.dll or not to move to the very bottom
				MEMORY_BASIC_INFORMATION mbi = {0};
				if(VirtualQuery(p->proc, &mbi, sizeof(mbi)))
				{
					if (GetModuleHandleW(L"gen_crasher.dll") == (HINSTANCE)mbi.AllocationBase)
					{
						p->where = -2;
					}
				}

				g_piprefsdlgs.push_back(p);
				prefs_liveDlgAdd(p);
			}
			return 0;
		case IPC_UPDATE_PREFS_DLG:
		case IPC_UPDATE_PREFS_DLGW:
			{
				prefsDlgRec *p = (prefsDlgRec *)data;
				p->next = (which == IPC_UPDATE_PREFS_DLG ? 0 : PREFS_UNICODE);
				prefs_liveDlgUpdate(p);
			}
			return 0;
		case IPC_ADJUST_OPTIONSMENUPOS:
			if (data == -1) g_mm_optionsbase_adj--;
			if (data == 1) g_mm_optionsbase_adj++;
			return g_mm_optionsbase_adj;
		case IPC_ADJUST_FFWINDOWSMENUPOS:
			if (data == -1) g_mm_ffwindowsbase_adj--;
			if (data == 1) g_mm_ffwindowsbase_adj++;
			return g_mm_ffwindowsbase_adj;
		case IPC_ADJUST_FFOPTIONSMENUPOS:
			if (data == -1) g_mm_ffoptionsbase_adj--;
			if (data == 1) g_mm_ffoptionsbase_adj++;
			return g_mm_ffoptionsbase_adj;
		case IPC_GET_HMENU:
			if (data == -1) return reinterpret_cast<LRESULT>(top_menu);
			if (data == 0) return reinterpret_cast<LRESULT>(main_menu);
			if (data > 0 && data < 11) return reinterpret_cast<LRESULT>(GetSubMenu(v5_top_menu, data - 1));
			return 0;
		case IPC_GETUNCOMPRESSINTERFACE:
			if (data == 0x10100000)
				return reinterpret_cast<LRESULT>(&inflate_struct);
			return reinterpret_cast<LRESULT>(uncompress);
		case IPC_ENABLEDISABLE_ALL_WINDOWS:
			EnableWindow(hwnd, data != 0xdeadbeef);
			EnableWindow(hPLWindow, data != 0xdeadbeef);
			EnableWindow(hEQWindow, data != 0xdeadbeef);
			//EnableWindow(hMBWindow,data!=0xdeadbeef);
			//EnableWindow(hVideoWindow,data!=0xdeadbeef);
			return 0;
		case IPC_RESTARTWINAMP:
		case IPC_RESTARTSAFEWINAMP:
			{
				g_restartonquit = (1 + (which == IPC_RESTARTSAFEWINAMP));
				Main_OnClose(hwnd);
			}
			return 0;
		case IPC_UPDTITLE:
			g_need_titleupd = 1;
			return 0;
		case IPC_REFRESHPLCACHE:
			{
				const wchar_t *prefix = (const wchar_t *)data;
				int t = PlayList_getlength();
				int x;

				if (prefix)
				{
					wchar_t filename[FILENAME_SIZE] = {0};
					int prefixLen = lstrlenW(prefix);
					for (x = 0; x < t; x ++)
					{
						PlayList_getitem(x, filename, 0);
						if (!_wcsnicmp(prefix, filename, prefixLen))
							PlayList_setcached(x, 0);
					}
				}
				else
				{
					for (x = 0; x < t; x ++)
						PlayList_setcached(x, 0);
				}
				return 0;
			}
		case IPC_CHANGECURRENTFILE:
			PlayList_setcurrent(AutoWideFn((const char *)data), PlayList_gettitle(AutoWide((const char*)data), 1));
			SendMessageW(hPLWindow, WM_USER + 1, 0, 0);
			return 0;
		case IPC_CHANGECURRENTFILEW:
			PlayList_setcurrent((const wchar_t *)data, PlayList_gettitle((const wchar_t *)data, 1));
			SendMessageW(hPLWindow, WM_USER + 1, 0, 0);
			return 0;
		case IPC_ISFULLSTOP:
			return g_fullstop;
		case IPC_GETEQDATA:
			ldata = data;
			if (data >= 0 && data <= 9) return eq_tab[data];
			if (data == 10) return config_preamp;
			if (data == 11) return config_use_eq;
			if (data == 12) return config_autoload_eq;
			return 0;
		case IPC_SETEQDATA:
			{
				int thisone = ldata; // compatability mode, shouldnt use anymore
				if ((data & 0xFF000000) == 0xDB000000) thisone = (data >> 16) & 0xFF;
				data &= 0xFFFF;

				if (thisone >= 0 && thisone <= 9) eq_tab[thisone] = (unsigned char)data;
				else if (thisone == 10) config_preamp = (unsigned char)data;
				else if (thisone == 11) config_use_eq = (unsigned char)data;
				else if (thisone == 12) config_autoload_eq = (unsigned char)data;
				if (thisone == 11 || thisone == 12) draw_eq_onauto(config_use_eq, config_autoload_eq, 0, 0);
				if (thisone != 12) eq_set(config_use_eq, (char *)eq_tab, config_preamp);

				if (config_eq_open)
				{
					if (thisone >= 0 && thisone <= 10)
					{
						draw_eq_slid(thisone == 10 ? 0 : thisone + 1, data, 0);
						draw_eq_graphthingy();
					}
				}
				PostMessageW(hMainWindow, WM_WA_IPC, IPC_CB_MISC_EQ, IPC_CB_MISC);
			}
			return 0;
		case IPC_GETLISTPOS:
			return PlayList_getPosition();
		case IPC_GETNEXTLISTPOS:
			return PlayList_getNextPosition();
		case IPC_GETINFO:
			if (data == 0) return g_srate;
			if (data == 1) return g_brate;
			if (data == 2) return g_nch;
			if (data == 5) return g_srate_exact;

			return 0;
		case IPC_SETSKIN:
			if (data && lstrlenA((char *)data))
			{
				AutoWide dataW((char *)data);
				if (_wcsicmp(config_skin, dataW))
				{
					StringCchCopyW(config_skin, MAX_PATH, dataW);
				}
				SendMessageW(hMainWindow, WM_COMMAND, WINAMP_REFRESHSKIN, 0);
			}
			return 0;
		case IPC_SETSKINW:
			if (data && lstrlenW((wchar_t *)data))
			{
				if (_wcsicmp(config_skin, (wchar_t *)data))
				{
					StringCchCopyW(config_skin, MAX_PATH, (wchar_t *)data);
				}
				SendMessageW(hMainWindow, WM_COMMAND, WINAMP_REFRESHSKIN, 0);
			}
			return 0;
		case IPC_GETSKINW:
		{
			if (data) 
				StringCchCopyW((wchar_t *)data, MAX_PATH, skin_directory);
			return (LRESULT)config_skin;
		}
		case IPC_GETSKIN:
		{
			if (data) 
				WideCharToMultiByteSZ(CP_ACP, 0, skin_directory, -1, (char *)data, MAX_PATH, 0, 0);
			static char config_skinA[MAX_PATH];
			WideCharToMultiByteSZ(CP_ACP, 0, config_skin, -1, config_skinA, MAX_PATH,0 ,0);
			return (LRESULT)config_skinA;
		}
		case IPC_EXECPLUG:
			if (data) vis_start(hwnd, AutoWide((char *)data));
			return 0;
		case IPC_GETPLAYLISTFILE:
			{
				if (!PlayList_getitem2(data, fn, NULL)) return (LRESULT)fn;
				return 0;
			}
		case IPC_GETPLAYLISTFILEW:
			{
				if (!PlayList_getitem2W(data, fnW, NULL)) return (LRESULT)fnW;
				return 0;
			}
		case IPC_GETPLAYLISTTITLE:
			{
				if (!PlayList_getitem2(data, NULL, fn)) return (LRESULT)fn;
				return 0;
			}
		case IPC_GETPLAYLISTTITLEW:
			{
				if (!PlayList_getitem2W(data, NULL, fnW)) return (LRESULT)fnW;
				return 0;
			}
		case IPC_GETVERSION:
			return APP_VERSION_NUM;
		case IPC_GETVERSIONSTRING:
			return (LRESULT)app_version_string;
		case IPC_OPEN_URL:
			{
				char *url = (char *)data;
				if ((url[0] == 'h' || url[0] == 'H') && url[1] == 0) // unicode!
				{
					myOpenURL(NULL, (wchar_t *)url);
				}
				else // ANSI
				{
					// copes with trying to open local files e.g. local help
					// as above checking would otherwise force it always to
					// be treated as ansi even if sent as a unicode file:///
					if(PathFileExistsW((wchar_t *)url))
					{
						myOpenURL(NULL, (wchar_t *)url);
					}
					else
					{
						myOpenURL(NULL, AutoWide(url));
					}
				}

			return 0;
			}
		case IPC_GETREGISTEREDVERSION:
			if (((unsigned int) data) > 0xffff)
			{
				unsigned long *p = (unsigned long *)data;
				p[2] = 0;
				static unsigned long key[4] = { 31337, 0xf00d, 0xdead, 0xbeef}; //fucko: hide this a bit
				tealike_crappy_code(p, key);
				p[2] = 2;
			}
			else if (!data)
			{
				prefs_last_page = 20;
				prefs_dialog(1);
			}
			return 0;
		case IPC_IS_SAFEMODE:
			if (g_safeMode == 2)
				return 2;
			return (!g_safeMode);
		case IPC_SETPLAYLISTPOS:
			PlayList_setposition(data);
			InvalidateRect(hPLWindow, NULL, FALSE);
			return 0;
		case IPC_GETLISTLENGTH:
			return PlayList_getlength();
		case IPC_SETVOLUME:
			if (data == -666) return config_volume;
			config_volume = (unsigned char)data;
			if (config_volume < 0) config_volume = 0;
			if (config_volume > 255) config_volume = 255;
			in_setvol(config_volume);
			draw_volumebar(config_volume, 0);
			update_volume_text(-2);
			return 0;
		case IPC_SETPANNING:
			if (data == -666) return config_pan;
			config_pan = (char)data;
			if (config_pan < -127) config_pan = -127;
			if (config_pan > 127) config_pan = 127;
			draw_panbar(config_pan, 0);
			in_setpan(config_pan);
			update_panning_text(-2);
			return 0;
		case IPC_WRITEPLAYLIST:
			if (plneedsave)
			{
				plneedsave = 0;
				savem3ufn(OLD_M3U_FILE, 0, 1);
				savem3ufn(M3U_FILE, 0, 1);
			}
			return PlayList_getlength() ? PlayList_getPosition() : -1;
		case IPC_INFOBOX:
			{
				infoBoxParam *p = (infoBoxParam*)data;
				int a = in_infobox(p->parent, AutoWideFn(p->filename));
				MSG msg;
				Sleep(100);
				while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)); //eat return
				return a;
			}
		case IPC_INFOBOXW:
			{
				infoBoxParamW *p = (infoBoxParamW*)data;
				int a = in_infobox(p->parent, p->filename);
				MSG msg;
				Sleep(100);
				while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)); //eat return
				return a;
			}
		case IPC_GETMLINIFILE:
			{
				static char *narrowMLINIfile=0;
				if (!narrowMLINIfile)
					narrowMLINIfile = _strdup(AutoCharFn(ML_INI_FILE));
				return reinterpret_cast<LRESULT>(narrowMLINIfile);
			}
		case IPC_GETMLINIFILEW:
			return reinterpret_cast<LRESULT>(ML_INI_FILE);
		case IPC_GETSHAREDDLLDIRECTORYW:
			return reinterpret_cast<LRESULT>(SHAREDDIR);
		case IPC_GETINIFILE:
			{
				static char *narrowINIfile=0;
				if (!narrowINIfile)
					narrowINIfile = _strdup(AutoCharFn(INI_FILE));
				return reinterpret_cast<LRESULT>(narrowINIfile);
			}
		case IPC_GETINIFILEW:
			return reinterpret_cast<LRESULT>(INI_FILE);
		case IPC_GETINIDIRECTORY:
			{
				static char *narrowINIdir=0;
				if (!narrowINIdir)
					narrowINIdir = _strdup(AutoCharFn(CONFIGDIR));
				return reinterpret_cast<LRESULT>(narrowINIdir);
			}
		case IPC_GETINIDIRECTORYW:
			return reinterpret_cast<LRESULT>(CONFIGDIR);
		case IPC_GETPLUGINDIRECTORY:
			{
				static char *narrowPlugindir=0;
				if (!narrowPlugindir)
					narrowPlugindir = _strdup(AutoCharFn(PLUGINDIR));
				return reinterpret_cast<LRESULT>(narrowPlugindir);
			}
		case IPC_GETPLUGINDIRECTORYW:
			return reinterpret_cast<LRESULT>(PLUGINDIR);
		case IPC_GETM3UDIRECTORY:
			{
				static char *narrowM3Udir=0;
				if (!narrowM3Udir)
					narrowM3Udir = _strdup(AutoCharFn(M3UDIR));
				return reinterpret_cast<LRESULT>(narrowM3Udir);
			}
		case IPC_GETM3UDIRECTORYW:
			return reinterpret_cast<LRESULT>(M3UDIR);
		case IPC_GETVISDIRECTORYW:
			return reinterpret_cast<LRESULT>(VISDIR);
		case IPC_GETSKINDIRECTORYW:
			return reinterpret_cast<LRESULT>(SKINDIR);
		case IPC_GETDSPDIRECTORYW:
			return reinterpret_cast<LRESULT>(DSPDIR);
		case IPC_OPENURLBOX:
			return getNewLocation( -1, (HWND)data);
		//FG> 5/19/03 -- added an ipc call similar to WINAMP_FILE_PLAY but with a parent HWND param, like OPENURLBOX
		case IPC_OPENFILEBOX:
			getNewFile(1, (HWND)data, 0);
			plEditRefresh();
			return 1;
		//FG> 5/19/03 -- added an ipc call similar to WINAMP_FILE_DIR but with a parent HWND param, like OPENURLBOX
		case IPC_OPENDIRBOX:
			{
				BROWSEINFOW bi = {0};
				wchar_t name[MAX_PATH] = {0};
				bi.hwndOwner = (HWND)data;
				bi.pszDisplayName = name;
				bi.lpszTitle = L"__foo";
				bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
				bi.lpfn = BrowseCallbackProc;
				ITEMIDLIST *idlist = SHBrowseForFolderW(&bi);
				if (idlist)
				{
					wchar_t path[MAX_PATH] = {0};
					SHGetPathFromIDListW(idlist, path);
					Shell_Free(idlist);
					WASABI_API_APP->path_setWorkingPath(path);
					PlayList_delete();
					PlayList_adddir(path, (config_rofiob&2) ? 0 : 1);
					if (config_rofiob&1) PlayList_sort(2, 0);
					BeginPlayback();
					plEditRefresh();
				}
				return 1;
			}
		case IPC_SPAWNBUTTONPOPUP:
			{
				HMENU hmenu = NULL;
				switch (data)
				{
					case 0:
					{
						hmenu = GetSubMenu(GetSubMenu(top_menu, 3), 7);
						UpdateAudioCDMenus(hmenu);
					}
					break;
					case 1: hmenu = GetSubMenu(GetSubMenu(top_menu, 3), 2); break;
					case 2: hmenu = GetSubMenu(GetSubMenu(top_menu, 3), 6); break;
					case 3: hmenu = GetSubMenu(GetSubMenu(top_menu, 3), 4); break;
					case 4: hmenu = GetSubMenu(GetSubMenu(top_menu, 3), 3); break;
					case 5: hmenu = GetSubMenu(GetSubMenu(top_menu, 3), 5); break;
				}
				if (hmenu != NULL)
				{
					DWORD msgpos = GetMessagePos();
					POINTS pt = MAKEPOINTS(msgpos);
					DoTrackPopup(hmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, hwnd);
				}
				return 1;
			}
		//FG> 5/19/03 -- added ipc call to set the parent HWND for open operations (file, dir, url) call with NULL to reset
		case IPC_UPDATEDIALOGBOXPARENT:
			{
				if (g_dialog_box_parent == (HWND)data && IsWindow((HWND)data))
				{
					RefreshIconicThumbnail();
					return 0;
				}
				// run through otherwise to force an update of things
			}
		case IPC_SETDIALOGBOXPARENT:
			{
				UnregisterThumbnailTab(g_dialog_box_parent);

				g_dialog_box_parent = (HWND)data;

				HWND new_parent = (IsWindow(g_dialog_box_parent) ? g_dialog_box_parent : hMainWindow);
				if (IsWindow(prefs_hwnd))
					SetWindowLongPtrW(prefs_hwnd, 0xFFFFFFF8, (LONG_PTR)new_parent);
				if (IsWindow(about_hwnd))
					SetWindowLongPtrW(about_hwnd, 0xFFFFFFF8, (LONG_PTR)new_parent);
				if (IsWindow(jump_hwnd))
					SetWindowLongPtrW(jump_hwnd, 0xFFFFFFF8, (LONG_PTR)new_parent);
				if (IsWindow(jump_hwnd2))
					SetWindowLongPtrW(jump_hwnd2, 0xFFFFFFF8, (LONG_PTR)new_parent);

				KillTimer(hMainWindow, AEROPEEK);
				SetTimer(hMainWindow, AEROPEEK, 250, DialogBoxParentChanged);
				return 1;
			}
		case IPC_GETDIALOGBOXPARENT:
			return reinterpret_cast<LRESULT>(IsWindow(g_dialog_box_parent) ? g_dialog_box_parent : hMainWindow);
		case IPC_GETPREFSWND:
			return reinterpret_cast<LRESULT>((IsWindow(prefs_hwnd) ? prefs_hwnd : NULL));
		case IPC_INITIAL_SHOW_STATE:
			return g_showcode;
		case IPC_PLAYFILE:
			{
				static char *filename;
				if (data > 0xffff)
				{
					enqueueFileWithMetaStruct *p = (enqueueFileWithMetaStruct*)data;
					if ( p->title )
					{
						wa::strings::wa_string l_ext( p->filename );
						PlayList_append_withinfo( AutoWideFn( p->filename ), AutoWide( p->title ), PathFindExtensionW( l_ext.GetW().c_str() ), p->length, 0 );
					}
					else
						PlayList_appendthing(AutoWideFn(p->filename), 0, 0);

					plEditRefresh();
				}
				else if (!data)
				{
					if (filename)
					{
						PlayList_appendthing(AutoWideFn(filename), 0, 0);
						GlobalFree(filename);
						filename = 0;
						plEditRefresh();
					}
				}
				else
				{
					static int i;
					if (!i) filename = (char *) GlobalAlloc(GPTR, MAX_PATH);
					filename[i++] = (char)data;
				}
				return 1;
			}
		case IPC_PLAYFILEW:
			{
				if ( data > 0xffff )
				{
					enqueueFileWithMetaStructW *p = (enqueueFileWithMetaStructW *)data;
					if ( p->title )
						PlayList_append_withinfo( p->filename, p->title, p->ext, p->length, 0 );
					else
						PlayList_appendthing( p->filename, 0, 0 );

					plEditRefresh();
				}
				return 1;
			}
		case IPC_PLAYFILEW_NDE:
			{
				if (data > 0xffff)
				{
					enqueueFileWithMetaStructW *p = (enqueueFileWithMetaStructW *)data;
					if (p->title)
						PlayList_append_withinfo(p->filename, p->title, p->ext, p->length, 1);
					else
						PlayList_appendthing(p->filename, 0, 1);
					plEditRefresh();
				}
				return 1;
			}
		case IPC_PLAYFILEW_NDE_TITLE:
			{
				if (data > 0xffff)
				{
					enqueueFileWithMetaStructW *p = (enqueueFileWithMetaStructW *)data;
					if (p->title)
						PlayList_append_withinfo(p->filename, p->title, p->ext, p->length, 3);
					else
						PlayList_appendthing(p->filename, 0, 3);

					plEditRefresh();
				}
				return 1;
			}
		case IPC_CHDIR:
			{
				static char *filename;
				static int i;
				if (!data)
				{
					if (filename)
					{
						filename[i] = 0;
						i = 0;
						SetCurrentDirectoryA(filename);
						GlobalFree(filename);
						filename = 0;
					}
				}
				else
				{
					if (!i) filename = (char *) GlobalAlloc(GPTR, MAX_PATH);
					filename[i++] = (char)data;
				}
				return 1;
			}
		case IPC_DELETE_INT:
		case IPC_DELETE:
			{
				int x = PlayList_getlength() > 0 ? 1 : 0;
				PlayList_delete();
				PlayList_randpos(0);
				plEditRefresh();
				return x;
			}
		case IPC_STARTPLAY_INT:
		case IPC_STARTPLAY:
			BeginPlayback();
			plEditRefresh();
			return 1;
		case IPC_ISPLAYING:
			return (playing ? 1 : 0) | (paused ? 2 : 0);
		case IPC_GETOUTPUTTIME:
			if (!data)
			{
				if (playing) return in_getouttime();
				else return -1;
			}
			else if (data == 1)
			{
				if (!playing || !in_mod) return PlayList_getcurrentlength();
				else return in_mod->GetLength() / 1000;
			}
			else if (data == 2)
			{
				if (!playing || !in_mod) return PlayList_getcurrentlength() * 1000;
				else return in_mod->GetLength();
			}
			return 0;
		case IPC_GETMODULENAME:
			return 0;
		case IPC_JUMPTOTIME:
			if (playing && in_mod && in_mod->is_seekable && !PlayList_ishidden(PlayList_getPosition()))
			{

				int t = data;
				if (t < 0) t = 0;
				if (in_seek(t) < 0)
				{
					SendMessageW(hwnd, WM_WA_MPEG_EOF, 0, 0);
					return 1;
				}
				else
				{
					ui_drawtime(in_getouttime() / 1000, 0);
				}
				return 0;
			}
			return -1;
		case IPC_EX_ISRIGHTEXE:
			return stat_isit;
		case IPC_GETHTTPGETTER:
			return (LRESULT)httpRetrieveFile;
		case IPC_GETHTTPGETTERW:
			return (LRESULT)httpRetrieveFileW;
		case IPC_INETAVAILABLE:
			return isInetAvailable();
		case IPC_GET_SHUFFLE:
			return !!config_shuffle;
		case IPC_GET_MANUALPLADVANCE:
			return !config_pladv;
		case IPC_GET_STOP_AFTER_CURRENT:
			return !g_stopaftercur; 
		case IPC_GET_REPEAT:
			return !!config_repeat;
		case IPC_SET_SHUFFLE:
			if (!!config_shuffle != !!data)
			{
				SendMessageW(hwnd, WM_COMMAND, WINAMP_FILE_SHUFFLE, 0);
			}
			break;
		case IPC_SET_REPEAT:
			if (!!config_repeat != !!data)
			{
				SendMessageW(hwnd, WM_COMMAND, WINAMP_FILE_REPEAT, 0);
			}
			break;
		case IPC_SET_MANUALPLADVANCE:
			if ((!!config_pladv) != (!data))
			{
				SendMessageW(hwnd, WM_COMMAND, WINAMP_FILE_MANUALPLADVANCE, 0);
			}
			break;
		case IPC_IS_FULLSCREEN:
			return vis_fullscreen;
		case IPC_SET_VIS_FS_FLAG:
			vis_fullscreen = !!data;
			break;
		case IPC_GET_EMBEDIF:
			if (data) return reinterpret_cast<LRESULT>(embedWindow((embedWindowState*)data));
			else return reinterpret_cast<LRESULT>(embedWindow);
		case IPC_SKINWINDOW:
			{
				SKINWINDOWPARAM *swp = (SKINWINDOWPARAM*)data;
				if (NULL == swp || swp->cbSize != sizeof(SKINWINDOWPARAM))
					return 0;
				return SkinWindow(swp->hwndToSkin, swp->windowGuid, swp->flagsEx, swp->callbackFF);
			}
			break;
		case IPC_EMBED_ENUM:
			EnterCriticalSection(&embedcs);
			{
				embedEnumStruct *parms = (embedEnumStruct*)data;
				embedWindowState *p = embedwndlist;
				while (p)
				{
					int x = parms->enumProc(p, parms);
					if (x)
					{
						LeaveCriticalSection(&embedcs);
						return x;
					}
					p = p->link;
				}
			}
			LeaveCriticalSection(&embedcs);
			return 0;
		case IPC_EMBED_ISVALID:
			EnterCriticalSection(&embedcs);
			{
				embedWindowState *p = embedwndlist;
				embedWindowState *parm = (embedWindowState*)data;
				while (p)
				{
					if (p == parm)
					{
						LeaveCriticalSection(&embedcs);
						return 1;
					}
					p = p->link;
				}
			}
			LeaveCriticalSection(&embedcs);
			return 0;
		case IPC_EMBED_ADD_LEGACY:
			EnterCriticalSection(&embedcs);
			{
				embedWindowState *parms = (embedWindowState*)data;

				if (!parms || (((unsigned int)parms) < 65536) || !IsWindow(parms->me)) return 1;

				EMBEDWND *pew = (EMBEDWND*)calloc(1, sizeof(EMBEDWND));
				SetPropW(parms->me, EMBEDWND_PROPW, pew);
				// this makes sure that JTFE won't mess with the windows
				SetPropA(parms->me, "WnShdProcIgnore", (HANDLE)1);

				// do this just incase it's missed off by the plug-in
				parms->flags |= EMBED_FLAGS_LEGACY_WND;

				SetWindowLongPtrW(parms->me, GWLP_USERDATA, (LONG_PTR)parms);

				EnterCriticalSection(&embedcs);
				GUID temp = GUID_NULL;
				if (parms->flags & EMBED_FLAGS_GUID)
					temp = parms->guid;

				memset(parms->extra_data, 0, sizeof(parms->extra_data));
				if (parms->flags & EMBED_FLAGS_GUID)
					parms->guid = temp;

				parms->link = embedwndlist;
				embedwndlist = parms;
				embedwndlist_cnt++;
				LeaveCriticalSection(&embedcs);
			}
			LeaveCriticalSection(&embedcs);
			return 0;
		case IPC_EMBED_REMOVE_LEGACY:
			EnterCriticalSection(&embedcs);
			{
				embedWindowState *parm = (embedWindowState*)data;
				if (parm)
				{
					EnterCriticalSection(&embedcs);
					embedWindowState *p=embedwndlist;
					if (p == parm)
					{
						embedwndlist = parm->link;// remove ourselves
						embedwndlist_cnt--;
					}
					else
					{
						while (p)
						{
							if (p->link == parm)
							{
								p->link = parm->link;
								embedwndlist_cnt--;
								break;
							}
							p=p->link;
						}
					}
					LeaveCriticalSection(&embedcs);

					EMBEDWND *pew = GetEmbedWnd(parm->me);
					if (pew)
					{
						RemovePropW(parm->me, EMBEDWND_PROPW);
						free(pew);
					}
				}
			}
			LeaveCriticalSection(&embedcs);
			return 0;
		case IPC_EMBED_UPDATE_LEGACY_POS:
			EnterCriticalSection(&embedcs);
			{
				embedWindowState *p = embedwndlist;
				embedWindowState *parm = (embedWindowState*)data;
				while (p)
				{
					if (p == parm)
					{
						CopyRect(&p->r, &parm->r);
						return 1;
					}
					p = p->link;
				}
			}
			LeaveCriticalSection(&embedcs);
			return 0;
		case IPC_GET_EMBED_SNAPFUNC:
			if (data) return reinterpret_cast<LRESULT>(SnapWindowToAllWindows);
			else return reinterpret_cast<LRESULT>(SnapToScreen);
		case IPC_GET_EXTENDED_FILE_INFO_HOOKABLE:
			{
			}
			// fall through
		case IPC_GET_EXTENDED_FILE_INFO:
			{
				extendedFileInfoStruct *efis = (extendedFileInfoStruct *)data;
				if (efis && efis->filename)
					return in_get_extended_fileinfo(efis->filename, efis->metadata, efis->ret, efis->retlen);
				else
					return 0;
			}
		case IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE:
			{
				extendedFileInfoStructW *efis = (extendedFileInfoStructW *)data;
				if (JSAPI2::callbackManager.OverrideMetadata(efis->filename, efis->metadata, efis->ret, efis->retlen))
					return 1;
			}
			// fall through
		case IPC_GET_EXTENDED_FILE_INFOW:
			{
				extendedFileInfoStructW *efis = (extendedFileInfoStructW *)data;
				if (efis && efis->filename)
					return in_get_extended_fileinfoW(efis->filename, efis->metadata, efis->ret, efis->retlen);
				else
					return 0;
			}
		case IPC_GET_BASIC_FILE_INFO:
			{
				basicFileInfoStruct *bfis = (basicFileInfoStruct*)data;
				if (bfis->quickCheck == 2 && config_riol == 1) bfis->quickCheck = 0;
				AutoWideFn wfn(bfis->filename);
				if (PathIsURLW(wfn))
				{
					const wchar_t *cachedTitle = PlayList_GetCachedTitle(wfn);
					if (cachedTitle && cachedTitle[0])
					{
						lstrcpynA(bfis->title, AutoChar(cachedTitle), bfis->titlelen);
						bfis->length = PlayList_get_lastlen();
						return 0;
					}
				}
				lstrcpynA(bfis->title, AutoChar(remove_urlcodesW(PlayList_gettitle(wfn, !bfis->quickCheck))), bfis->titlelen);
				bfis->length = PlayList_get_lastlen();
				return 0;
			}
		case IPC_GET_BASIC_FILE_INFOW:
		{
			basicFileInfoStructW *bfis = (basicFileInfoStructW *)data;
			if ( bfis->quickCheck == 2 && config_riol == 1 ) bfis->quickCheck = 0;
			if ( bfis->filename && *bfis->filename && PathIsURLW( bfis->filename ) )
			{
				const wchar_t *cachedTitle = PlayList_GetCachedTitle( bfis->filename );
				if ( cachedTitle && cachedTitle[ 0 ] )
				{
					lstrcpynW( bfis->title, cachedTitle, bfis->titlelen );
					bfis->length = PlayList_get_lastlen();
					return 0;
				}
			}

			lstrcpynW( bfis->title, remove_urlcodesW( PlayList_gettitle( bfis->filename, !bfis->quickCheck ) ), bfis->titlelen );
			bfis->length = PlayList_get_lastlen();

			return 0;
		}
		case IPC_SET_EXTENDED_FILE_INFO:
			{
				extendedFileInfoStruct *efis = (extendedFileInfoStruct *)data;
				return in_set_extended_fileinfo(efis->filename, efis->metadata, efis->ret);
			}
		case IPC_SET_EXTENDED_FILE_INFOW:
			{
				extendedFileInfoStructW *efis = (extendedFileInfoStructW *)data;
				return in_set_extended_fileinfoW(efis->filename, efis->metadata, efis->ret);
			}
		case IPC_WRITE_EXTENDED_FILE_INFO:
			return in_write_extended_fileinfo();
		case IPC_CONVERTFILE:
			return convert_file((convertFileStruct *)data);
		case IPC_CONVERTFILEW:
			return convert_fileW((convertFileStructW *)data);
		case IPC_CONVERT_TEST:
			return convert_file_test((convertFileStructW *)data);
		case IPC_CONVERTFILE_END:
			convert_end((convertFileStruct *)data);
			break;
		case IPC_CONVERTFILEW_END:
			convert_endW((convertFileStructW *)data);
			break;
		case IPC_CONVERT_CONFIG:
			return reinterpret_cast<LRESULT>(convert_config((convertConfigStruct *)data));
		case IPC_CONVERT_CONFIG_END:
			convert_config_end((convertConfigStruct *)data);
			break;
		case IPC_CONVERT_CONFIG_ENUMFMTS:
			convert_enumfmts((converterEnumFmtStruct *)data);
			break;
		case IPC_CONVERT_SET_PRIORITY:
			convert_setPriority((convertSetPriority *)data);
			break;
		case IPC_CONVERT_SET_PRIORITYW:
			convert_setPriorityW((convertSetPriorityW *)data);
			break;
		case IPC_CONVERT_CONFIG_SET_ITEM:
			return convert_setConfigItem((convertConfigItem*)data);
		case IPC_CONVERT_CONFIG_GET_ITEM:
			return convert_getConfigItem((convertConfigItem*)data);
		case IPC_PLCMD:
			PE_Cmd((windowCommand*)data);
			break;
		case IPC_GETDROPTARGET:
			return reinterpret_cast<LRESULT>(Ole_getDropTarget());
	#ifdef BURN_SUPPORT
	#if !defined(_WIN64)
		case IPC_BURN_CD:
			return burn_start((burnCDStruct *)data);
	#endif
	#endif
		case IPC_GET_NEXT_PLITEM:
			return -1;
		case IPC_GET_PREVIOUS_PLITEM:
			return -1;
		case IPC_IS_WNDSHADE:
			if (data == -1) return config_windowshade;
			if (data == IPC_GETWND_PE) return config_pe_height == 14;
			if (data == IPC_GETWND_EQ) return config_eq_ws;
			return 0;
		case IPC_SPAWNFILEMENU:
			{
				waSpawnMenuParms2 *p = (waSpawnMenuParms2*)data;
				return V5_File_Menu(p->wnd, p->xpos, p->ypos, p->width, p->height);
			}
		case IPC_SPAWNPLAYMENU:
			{
				waSpawnMenuParms2 *p = (waSpawnMenuParms2*)data;
				return V5_Play_Menu(p->wnd, p->xpos, p->ypos, p->width, p->height);
			}
		case IPC_SPAWNOPTIONSMENU:
			{
				waSpawnMenuParms2 *p = (waSpawnMenuParms2*)data;
				return V5_Options_Menu(p->wnd, p->xpos, p->ypos, p->width, p->height);
			}
		case IPC_SPAWNWINDOWSMENU:
			{
				waSpawnMenuParms2 *p = (waSpawnMenuParms2*)data;
				return V5_Windows_Menu(p->wnd, p->xpos, p->ypos, p->width, p->height);
			}
		case IPC_SPAWNHELPMENU:
			{
				waSpawnMenuParms2 *p = (waSpawnMenuParms2*)data;
				return V5_Help_Menu(p->wnd, p->xpos, p->ypos, p->width, p->height);
			}
		case IPC_SPAWNPEFILEMENU:
			{
				waSpawnMenuParms2 *p = (waSpawnMenuParms2*)data;
				return V5_PE_File_Menu(p->wnd, p->xpos, p->ypos, p->width, p->height);
			}
		case IPC_SPAWNPEPLAYLISTMENU:
			{
				waSpawnMenuParms2 *p = (waSpawnMenuParms2*)data;
				return V5_PE_Playlist_Menu(p->wnd, p->xpos, p->ypos, p->width, p->height);
			}
		case IPC_SPAWNPESORTMENU:
			{
				waSpawnMenuParms2 *p = (waSpawnMenuParms2*)data;
				return V5_PE_Sort_Menu(p->wnd, p->xpos, p->ypos, p->width, p->height);
			}
		case IPC_SPAWNPEHELPMENU:
			{
				waSpawnMenuParms2 *p = (waSpawnMenuParms2*)data;
				return V5_PE_Help_Menu(p->wnd, p->xpos, p->ypos, p->width, p->height);
			}
		case IPC_SPAWNMLFILEMENU:
			{
				waSpawnMenuParms2 *p = (waSpawnMenuParms2*)data;
				return V5_ML_File_Menu(p->wnd, p->xpos, p->ypos, p->width, p->height);
			}
		case IPC_SPAWNMLVIEWMENU:
			{
				waSpawnMenuParms2 *p = (waSpawnMenuParms2*)data;
				return V5_ML_View_Menu(p->wnd, p->xpos, p->ypos, p->width, p->height);
			}
		case IPC_SPAWNMLHELPMENU:
			{
				waSpawnMenuParms2 *p = (waSpawnMenuParms2*)data;
				return V5_ML_Help_Menu(p->wnd, p->xpos, p->ypos, p->width, p->height);
			}
		case IPC_SPAWNPELISTOFPLAYLISTS:
			{
				waSpawnMenuParms *p = (waSpawnMenuParms*)data;
				return V5_PE_ListOfPlaylists_Menu(p->xpos, p->ypos);
			}
		case IPC_GET_UNIQUE_DISPATCH_ID:
			return JSAPI1_GenerateUniqueDispatchId();
		case IPC_ADD_DISPATCH_OBJECT:
			{
				DispatchInfo *info = (DispatchInfo*)data;
				if (NULL != info)
				{				
					ExternalCOM *external;
					if (SUCCEEDED(JSAPI1_GetExternal(&external)))
					{
						info->id = external->AddDispatch(info->name, info->dispatch);
						external->Release();
					}
					else
					{
						info->id = 0;
					}
				}
				return 0;
			}
		case IPC_REMOVE_DISPATCH_OBJECT:
			{
				ExternalCOM *external;
				if (SUCCEEDED(JSAPI1_GetExternal(&external)))
				{
					external->RemoveDispatch((DWORD)data);
					external->Release();
				}
			}
			return 0;
		case IPC_GET_PROXY_STRING:
			return reinterpret_cast<LRESULT>(config_proxy);
		case IPC_PLAYLIST_GET_NEXT_SELECTED:
			return PlayList_GetNextSelected(data);
		case IPC_PLAYLIST_GET_SELECTED_COUNT:
			return PlayList_GetSelectedCount();
		case IPC_FILE_TAG_MAY_HAVE_UPDATED:
			{
				extern In_Module *g_in_infobox;
				if (!g_in_infobox)
					PlayList_UpdateTitle(AutoWideFn((const char *)data));
				return 0;
			}
		case IPC_FILE_TAG_MAY_UPDATEW:
			{
				// issue sys callback
				WASABI_API_SYSCB->syscb_issueCallback(SysCallback::META, MetadataCallback::FILE_MAY_UPDATE, (intptr_t)data);
				return 0;
			}
		case IPC_FILE_TAG_MAY_HAVE_UPDATEDW:
			{
				extern In_Module *g_in_infobox;
				if (!g_in_infobox)
					PlayList_UpdateTitle((const wchar_t *)data);

				// issue sys callback
				WASABI_API_SYSCB->syscb_issueCallback(SysCallback::META, MetadataCallback::FILE_UPDATED, (intptr_t)data);
				return 0;
			}
		case IPC_SET_JTF_COMPARATOR:
			SetJumpComparator((void *)data);
			return 0;
		case IPC_SET_JTF_COMPARATOR_W:
			SetJumpComparatorW((void *)data);
			return 0;
		case IPC_SET_JTF_DRAWTEXT:
			jtf_drawtext=reinterpret_cast<int (WINAPI *)(HDC, LPCWSTR, int, LPRECT, UINT)>(data);
			return 0;
		case IPC_SET_JTF_LOAD_MODE:
			if (data == -666) return config_jtf_check;
			config_jtf_check = data;
			_w_i("jtf_check", config_jtf_check);
			return 0;
		case IPC_UPDATE_URL:
			{
				ReplyMessage(0);
				char *url=(char *)data;
				UpdateWindow_Show(url);
				free(url);
				return 0;
			}
		case IPC_GETPLAYITEM_START:
			return PlayList_GetItem_Start(PlayList_getPosition());
		case IPC_GETPLAYITEM_END:
			return PlayList_GetItem_End(PlayList_getPosition());
		case IPC_GET_PLAYING_FILENAME:
			return reinterpret_cast<LRESULT>(FileName);
		case IPC_GET_PLAYING_TITLE:
			return reinterpret_cast<LRESULT>(FileTitle);
		case IPC_COPY_EXTENDED_FILE_INFO:
			{
				copyFileInfoStruct *copy = (copyFileInfoStruct *)data;
				CopyExtendedFileInfo(AutoWideFn(copy->source), AutoWideFn(copy->dest));
				return 0;
			}
		case IPC_COPY_EXTENDED_FILE_INFOW:
			{
				copyFileInfoStructW *copy = (copyFileInfoStructW *)data;
				CopyExtendedFileInfo(copy->source, copy->dest);
				return 0;
			}
		case IPC_CANPLAY:
			{
				int a=0;
				return (LRESULT)in_setmod_noplay((const wchar_t *)data, &a);
			}
		// disabled 30 May 2012 as per email from Tejas w.r.t. to Rovi deal ending
		// if called it will now act like a failure and return 1 by default handling
		/*case IPC_FETCH_ALBUMART:
			{
				extern int RetrieveAlbumArt(artFetchData * data);
				return RetrieveAlbumArt((artFetchData *)data);
			}*/
		case IPC_JSAPI2_GET_DISPATCH_OBJECT:
			return (LRESULT)new JSAPI2::ExternalObject((const wchar_t *)data);
		case IPC_HANDLE_URI:
			{
				const wchar_t *cmd  = (const wchar_t *)data;
				if (cmd)
				{
					if (cmd[0] == '\"')
					{
						wchar_t temp[1024] = {0};
						GetParameter(cmd, temp, 1024);
						HandleFilename(temp);
					}
					else
						HandleFilename(cmd);
				return 0;
				}
			}
		case IPC_GET_D3DX9:
			{
				HMODULE d3dx_lib = FindD3DX9();
				return (LRESULT)d3dx_lib;
			}
		case IPC_GET_FILEREGISTRAR_OBJECT:
			{
				IFileTypeRegistrar *registrar = 0;
				if (GetRegistrar(&registrar, !data) == 0 && registrar)
					return (LRESULT)registrar;
				return 0;
			}
	} // switch(which)
	return 1;
}

// Main_OnIPC()