#include "main.h"
#include <windowsx.h>
#include <time.h>
#include <rpc.h>
#include "../winamp/gen.h"
#include "resource.h"
#include "childwnd.h"
#include "config.h"
#include "../winamp/ipc_pe.h"
#include "../winamp/wa_dlg.h"
#include "../winamp/strutil.h"
#include "ml.h"
#include "ml_ipc.h"
#include "./folderbrowser.h"
#include "./mldwm.h"

#ifndef _ML_HEADER_IMPMLEMENT
#define _ML_HEADER_IMPMLEMENT
#endif // _ML_HEADER_IMPMLEMENT
#include "ml_ipc_0313.h"
#undef _ML_HEADER_IMPMLEMENT

#include "sendto.h"
#include "../gen_hotkeys/wa_hotkeys.h"
#include "MediaLibraryCOM.h"
#include "../nu/CCVersion.h"
#include "../nu/AutoWideFn.h"
#include "../nu/htmlcontainer2.h"
#include <shlwapi.h>

#include "api__gen_ml.h"
#include <api/service/waServiceFactory.h>
#include "./navigation.h"
//#include "./skinnedwnd.h"
#include "./skinning.h"
#include "../nu/ServiceWatcher.h"
#include "MusicID.h"
#include <tataki/export.h>
#include <strsafe.h>
#include "../Winamp/wasabicfg.h"

// {6B0EDF80-C9A5-11d3-9F26-00C04F39FFC6}
static const GUID library_guid = 
{ 0x6b0edf80, 0xc9a5, 0x11d3, { 0x9f, 0x26, 0x0, 0xc0, 0x4f, 0x39, 0xff, 0xc6 } };

int m_calling_getfileinfo;
int IPC_GETMLWINDOW, IPC_LIBRARY_SENDTOMENU, IPC_GET_ML_HMENU;
int config_use_ff_scrollbars=1, config_use_alternate_colors=0;
LARGE_INTEGER freq;
C_Config *g_config;

embedWindowState myWindowState;
prefsDlgRecW myPrefsItem, myPrefsItemPlug;

DEFINE_EXTERNAL_SERVICE(api_service,          WASABI_API_SVC);
DEFINE_EXTERNAL_SERVICE(api_application,      WASABI_API_APP);
DEFINE_EXTERNAL_SERVICE(api_language,         WASABI_API_LNG);
DEFINE_EXTERNAL_SERVICE(obj_ombrowser,        AGAVE_OBJ_BROWSER);
DEFINE_EXTERNAL_SERVICE(api_mldb,             AGAVE_API_MLDB);
DEFINE_EXTERNAL_SERVICE(api_syscb,            WASABI_API_SYSCB);
DEFINE_EXTERNAL_SERVICE(api_threadpool,       AGAVE_API_THREADPOOL);
DEFINE_EXTERNAL_SERVICE(api_decodefile,       AGAVE_API_DECODE);
DEFINE_EXTERNAL_SERVICE(wnd_api,              WASABI_API_WND);
DEFINE_EXTERNAL_SERVICE(api_skin,             WASABI_API_SKIN);
DEFINE_EXTERNAL_SERVICE(api_config,           AGAVE_API_CONFIG);
DEFINE_EXTERNAL_SERVICE(api_palette,          WASABI_API_PALETTE);
#ifndef IGNORE_API_GRACENOTE
DEFINE_EXTERNAL_SERVICE(api_gracenote,        AGAVE_API_GRACENOTE);
#endif
DEFINE_EXTERNAL_SERVICE(JSAPI2::api_security, AGAVE_API_JSAPI2_SECURITY);

ifc_configitem *ieDisableSEH = 0;

// wasabi based services for localisation support
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

static ServiceWatcher serviceWatcher;
#ifndef IGNORE_API_GRACENOTE
MusicIDCOM musicIDCOM;
#endif

void config();
void quit();
int init();

BOOL init2(void); 

extern "C"
{
	HWND g_hwnd, g_ownerwnd;

	extern winampGeneralPurposePlugin plugin =
	    {
			GPPHDR_VER_U,
	        "nullsoft(gen_ml.dll)",
	        init,
	        config,
	        quit,
	    };
};
HWND g_PEWindow;

HMENU wa_main_menu = NULL;
HMENU wa_windows_menu = NULL;
HMENU wa_playlists_cmdmenu = NULL;
HMENU last_playlistsmenu = NULL;
HMENU last_viewmenu = NULL;
int last_viewmenu_insert = 0;
int g_safeMode = 0, sneak = 0;

HCURSOR hDragNDropCursor;
int profile = 0;

wchar_t pluginPath[MAX_PATH] = {0};
static wchar_t preferencesName[128];


HMENU g_context_menus;

extern C_ItemList m_plugins;
extern HNAVCTRL hNavigation;

//xp theme disabling shit
static HMODULE m_uxdll;
HRESULT (__stdcall *SetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
BOOL (__stdcall *IsAppThemed)(void);

template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC && api_t)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

bool IsVisible()
{
	return g_hwnd && IsWindowVisible(g_ownerwnd);
}

void MLVisibleChanged(BOOL fVisible)
{
	static BOOL visible = FALSE;
	if (fVisible != visible)
	{
		visible = fVisible;
		plugin_SendMessage(ML_MSG_MLVISIBLE, visible, 0, 0);
	}
}

BOOL MlWindow_SetMinimizedMode(BOOL fMinimized)
{
	if (FALSE != fMinimized)
		return SetPropW(g_ownerwnd, L"MLWindow_MinimizedMode", (HANDLE)1);
	
	RemovePropW(g_ownerwnd, L"MLWindow_MinimizedMode");
	return TRUE;
}

BOOL MlWindow_IsMinimizedMode(void)
{
	return  (0 != GetPropW(g_ownerwnd, L"MLWindow_MinimizedMode"));
}

void toggleVisible(int closecb)
{
	BOOL fVisible, fMinimized;
	HWND rootWindow;
	fVisible = (0 != (WS_VISIBLE & GetWindowLongPtrW(g_ownerwnd, GWL_STYLE)));//IsWindowVisible(g_ownerwnd);
	
	rootWindow = GetAncestor(g_ownerwnd, GA_ROOT);
	if (NULL == rootWindow || rootWindow == g_ownerwnd)
	{
		rootWindow = (HWND)(HWND)SENDWAIPC(plugin.hwndParent, IPC_GETDIALOGBOXPARENT, 0);
		if (NULL == rootWindow)
			rootWindow = plugin.hwndParent;
	}

	fMinimized = IsIconic(rootWindow);

	if (FALSE != fVisible || 1 == closecb)
	{
		if (FALSE == fMinimized && FALSE != fVisible)
		{
			HWND hwndFocus = GetFocus();

			if (hwndFocus == g_ownerwnd || IsChild(g_ownerwnd, hwndFocus))
				SendMessageW(plugin.hwndParent, WM_COMMAND, WINAMP_NEXT_WINDOW, 0);

			ShowWindow(g_ownerwnd, SW_HIDE);
		}
	}
	else
	{
		if (init2() && FALSE == fMinimized && FALSE == fVisible)
		{	
			ShowWindow(g_ownerwnd, SW_SHOWNORMAL);
			// make sure that we focus the tree to work around some modern skin quirks
			if(closecb != 2)
			{
				SetFocus(NavCtrlI_GetHWND(hNavigation));
			}
			else
			{
				// delay the focusing on loading as some machines are too fast and 
				// may cause the wrong view to the selected (root instead of child)
				PostMessage(g_ownerwnd,WM_ML_IPC,0,ML_IPC_FOCUS_TREE);
			}
		}
	}

	if (FALSE != fMinimized && 1 != closecb)
	{
		MlWindow_SetMinimizedMode(TRUE);

		if (NULL != g_config)
			g_config->WriteInt(L"visible", (FALSE == fVisible));
		
		UINT menuFlags = (FALSE == fVisible) ? MF_CHECKED : MF_UNCHECKED;
		menuFlags |= MF_BYCOMMAND;

		INT szMenu[] = { 0, 4, };
		for (INT i = 0; i < ARRAYSIZE(szMenu); i++)
		{
			HMENU hMenu = (HMENU)SendMessage(plugin.hwndParent, WM_WA_IPC, szMenu[i], IPC_GET_HMENU);
			if (NULL != hMenu)
				CheckMenuItem(hMenu, WA_MENUITEM_ID, menuFlags);
		}
	}
}


static WNDPROC wa_oldWndProc;

static BOOL Winamp_OnIPC(HWND hwnd, UINT uMsg, INT_PTR param, LRESULT *pResult)
{
	if (IPC_GETMLWINDOW == uMsg && IPC_GETMLWINDOW > 65536)
	{	
		if (param == -1 && !g_hwnd) init2();
		*pResult = (LRESULT)g_hwnd;
		return TRUE;
	}
	else if (IPC_LIBRARY_SENDTOMENU == uMsg && IPC_LIBRARY_SENDTOMENU > 65536)
	{
		librarySendToMenuStruct *s = (librarySendToMenuStruct*)param;
		if (!s || s->mode == 0) 
		{
			*pResult = 0xFFFFFFFF;
			return TRUE;
		}
		if (s->mode == 1)
		{
			if (!s->ctx[0])
			{
				if (!g_hwnd) init2();
				SendToMenu *stm = new SendToMenu();
				if (s->build_start_id && s->build_end_id)
				{
					stm->buildmenu(s->build_hMenu, s->data_type, s->ctx[1], s->ctx[2], s->build_start_id, s->build_end_id);
				}
				else
				{
					stm->buildmenu(s->build_hMenu, s->data_type, s->ctx[1], s->ctx[2]);
				}
				s->ctx[0] = (intptr_t)stm;
				*pResult = 0xFFFFFFFF;
				return TRUE;
			}
		}
		else if (s->mode == 2)
		{
			SendToMenu *stm = (SendToMenu *)s->ctx[0];
			if (stm && stm->isourcmd(s->menu_id))
			{
				*pResult = 0xFFFFFFFF;
				return TRUE;
			}
		}
		else if (s->mode == 3)
		{
			SendToMenu *stm = (SendToMenu *)s->ctx[0];
			if (stm)
			{
				*pResult = stm->handlecmd(s->hwnd, s->menu_id, s->data_type, s->data);
				return TRUE;
			}
		}
		else if (s->mode == 4)
		{
			delete (SendToMenu *)s->ctx[0];
			s->ctx[0] = 0;
		}
		*pResult = TRUE;
		return TRUE;
	}
	else if (IPC_GET_ML_HMENU == uMsg && IPC_GET_ML_HMENU > 65536) 	
	{
		*pResult = (LRESULT)g_context_menus;
		return TRUE;
	}

	switch(uMsg)
	{
		case IPC_CB_RESETFONT:
			PostMessageW(g_hwnd, WM_DISPLAYCHANGE, 0, 0);
			break;

		case IPC_CB_GETTOOLTIPW:
			if (param == 16 && g_config->ReadInt(L"attachlbolt", 0))
			{
				static wchar_t tlStr[64];
				*pResult = (LRESULT)WASABI_API_LNGSTRINGW_BUF(IDS_TOGGLE_LIBRARY,tlStr,64);
				return TRUE;
			}
			break;

		case IPC_GET_EXTENDED_FILE_INFO_HOOKABLE:
			if (!m_calling_getfileinfo)
			{
				extendedFileInfoStruct *extendedInfo;
				extendedInfo = (extendedFileInfoStruct*)param;
				if (NULL != extendedInfo && 
					NULL != extendedInfo->filename &&
					NULL != extendedInfo->metadata)
				{
					if (plugin_SendMessage(ML_IPC_HOOKEXTINFO, param, 0, 0)) 
					{ 
						*pResult = 1; 
						return TRUE; 
					}
				}
			}
			break;

		case IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE:
			if (!m_calling_getfileinfo)
			{
				extendedFileInfoStructW *extendedInfo;
				extendedInfo = (extendedFileInfoStructW*)param;
				if (NULL != extendedInfo && 
					NULL != extendedInfo->filename &&
					NULL != extendedInfo->metadata)
				{
					if (plugin_SendMessage(ML_IPC_HOOKEXTINFOW, param, 0, 0)) 
					{ 
						*pResult = 1; 
						return TRUE; 
					}
				}
			}
			break;

		case IPC_HOOK_TITLES:
			if (NULL != param)
			{
				waHookTitleStruct *hookTitle;
				hookTitle = (waHookTitleStruct*)param;
				if (NULL != hookTitle->filename &&
					plugin_SendMessage(ML_IPC_HOOKTITLE, param, 0, 0)) 
				{ 
					*pResult = 1; 
					return TRUE; 
				}
			}
			break;

		case IPC_HOOK_TITLESW:
			if (NULL != param)
			{
				waHookTitleStructW *hookTitle;
				hookTitle = (waHookTitleStructW*)param;
				if (NULL != hookTitle->filename &&
					plugin_SendMessage(ML_IPC_HOOKTITLEW, param, 0, 0)) 
				{ 
					*pResult = 1; 
					return TRUE; 
				}
			}
			break;

		case IPC_ADD_PREFS_DLG:
		case IPC_ADD_PREFS_DLGW:
			if (param && !((prefsDlgRec*)param)->where)
			{
				prefsDlgRec *p = (prefsDlgRec *)param;
				// we use the dialog proc for the preferences to determine the hinstance of the module and
				// use that to then determine if we set it as a child of the media library preference node
				// it also handles localised versions of the preference pages as the dialog proceedure is
				// going to be in the true plug-in dll and so can be matched to the main ml plugins list!
				MEMORY_BASIC_INFORMATION mbi = {0};
				if(VirtualQuery(p->proc, &mbi, sizeof(mbi)))
				{
					int i = m_plugins.GetSize();
					while (i-- > 0)
					{
						winampMediaLibraryPlugin *mlplugin = (winampMediaLibraryPlugin *)m_plugins.Get(i);
						if (mlplugin->hDllInstance == (HINSTANCE)mbi.AllocationBase)
						{
							p->where = (intptr_t)(INT_PTR)&myPrefsItem;
							break;
						}
					}
				}
			}
			break;

		case IPC_CB_ONSHOWWND:
			if ((HWND)param == g_ownerwnd) MLVisibleChanged(TRUE);
			break;

		case IPC_HOOK_OKTOQUIT:
			{
				if (plugin_SendMessage(ML_MSG_NOTOKTOQUIT, 0, 0, 0))
				{
					*pResult = 0;
					return TRUE;
				}
			}
			break;

		case IPC_PLAYING_FILEW:
			plugin_SendMessage(ML_MSG_PLAYING_FILE, param, 0, 0);
			break;

		case IPC_WRITECONFIG:
			plugin_SendMessage(ML_MSG_WRITE_CONFIG, param, 0, 0);
			break;
	}
	return FALSE;
}

static LRESULT WINAPI wa_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		// far from ideal fix but deals with differing plugin load orders (mainly from FAT32 drives)
		// and not being able to unload/clean up properly the scrollbar bitmaps used - DRO 29/09/07
		case WM_CLOSE:
			SkinnedScrollWnd_Quit();
			break;
		case WM_WA_IPC:
		{
			LRESULT result = 0;
			if (Winamp_OnIPC(hwndDlg, (UINT)lParam, (INT_PTR)wParam, &result)) return result;
			break;
		}
		case WM_SIZE:
			if (wParam == SIZE_RESTORED)
			{
				if (FALSE != MlWindow_IsMinimizedMode())
				{
					MlWindow_SetMinimizedMode(FALSE);
					int showCommand = (0 != g_config->ReadInt(L"visible", 1)) ?  SW_SHOWNA : SW_HIDE;
					ShowWindow(g_ownerwnd, showCommand);
				}
			}
			break;
		case WM_COMMAND:
		case WM_SYSCOMMAND:
			{
				WORD lowP = LOWORD(wParam);
				if (lowP == WA_MENUITEM_ID || lowP == WINAMP_LIGHTNING_CLICK)
				{
					if (lowP != WINAMP_LIGHTNING_CLICK || g_config->ReadInt(L"attachlbolt", 0))
					{
						toggleVisible();
						return 0;
					}
				}
		#if 0 // no radio - don't delete yet - tag will need to do this in ml_online
				else if (lowP == WINAMP_VIDEO_TVBUTTON) // && g_config->ReadInt("attachtv",1))
				{
					if (!g_hwnd || !IsWindowVisible(g_ownerwnd)) toggleVisible();
					PostMessage(g_ownerwnd, WM_NEXTDLGCTL, (WPARAM)g_hwnd, TRUE);
					HWND hwndTree = GetTreeHWND(g_hwnd);
					HTREEITEM hti = findByParam(hwndTree, TREE_INTERNET_VIDEO, TVI_ROOT);
					if (hti)
					{
						TreeView_SelectItem(hwndTree, hti);
						return 0;
					}
				}
		#endif
				// done like this since ml_online can't really subclass winamp to get the notification
				else if (lowP == WINAMP_VIDEO_TVBUTTON)
				{
					if (!g_hwnd || !IsWindowVisible(g_ownerwnd)) toggleVisible();
					HNAVITEM hDefItem = NavCtrlI_FindItemByName(hNavigation, LOCALE_USER_DEFAULT, NICF_INVARIANT_I | NICF_DISPLAY_I | NICF_IGNORECASE_I, L"Shoutcast TV", -1);

					if(!hDefItem)
					{
						// work with the localised version of the Online Services root (if there...)
						wchar_t OSName[64] = {L"Online Services"};
						WASABI_API_LNG->GetStringFromGUIDW(MlOnlineLangGUID,WASABI_API_ORIG_HINST,1,OSName,64);

						// just incase the localised dll was there but the file was missing the translation
						if(!lstrcmpiW(OSName,L"Error loading string"))
							lstrcpynW(OSName,L"Online Services",64);

						hDefItem = NavCtrlI_FindItemByName(hNavigation, LOCALE_USER_DEFAULT, NICF_INVARIANT_I | NICF_DISPLAY_I | NICF_IGNORECASE_I, OSName, -1);
					}
					if (hDefItem)
					{
						NavItemI_Select(hDefItem);
						NavCtrlI_Show(hNavigation, SW_SHOWNA);
					}
					else
					{
						wchar_t titleStr[128] = {0};
						MessageBoxW(hwndDlg,WASABI_API_LNGSTRINGW(IDS_ONLINE_SERVICES_NOT_PRESENT),
								    WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_SWITCHING_TO_VIEW,titleStr,128),0);
					}
					return 0;
				}
				if (lowP == WINAMP_SHOWLIBRARY)
				{
					if (!g_hwnd || !IsWindowVisible(g_hwnd)) 
						toggleVisible((2 == HIWORD(wParam) ? 2 : 0));
				}
				else if (lowP == WINAMP_CLOSELIBRARY)
				{
					if (g_hwnd && IsWindowVisible(g_ownerwnd)) toggleVisible();
				}
			} 
			break;
		case WM_DWMCOMPOSITIONCHANGED:
			if (IsWindow(g_hwnd)) PostMessageW(g_hwnd, WM_DWMCOMPOSITIONCHANGED, 0, 0L);
			break;
	}

	return CallWindowProcW(wa_oldWndProc, hwndDlg, uMsg, wParam, lParam);
}

INT_PTR CALLBACK dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL init2(void)
{
	if (!g_hwnd)
	{
		WADlg_init(plugin.hwndParent);

		//xp theme disabling shit
		m_uxdll = LoadLibraryA("uxtheme.dll");
		if (m_uxdll)
		{
			IsAppThemed = (BOOL (__stdcall *)(void))GetProcAddress(m_uxdll, "IsAppThemed");
			SetWindowTheme = (HRESULT (__stdcall *)(struct HWND__ *, LPCWSTR , LPCWSTR  ))GetProcAddress(m_uxdll, "SetWindowTheme");
		}
		else
		{
			IsAppThemed = NULL;
			SetWindowTheme = NULL;
		}

		g_context_menus = WASABI_API_LOADMENU(IDR_CONTEXTMENUS);

		// 02/11/08 DrO
		// defaults were 100,100,500,400 and not visible but these now make it align when opened under
		// a clean install starting with a classic skin and is also visible on start now as with modern
		myWindowState.r.left = g_config->ReadInt(L"mw_xpos", 301);
		myWindowState.r.top = g_config->ReadInt(L"mw_ypos", 29);
		myWindowState.r.right = myWindowState.r.left + g_config->ReadInt(L"mw_width", 500);
		myWindowState.r.bottom = myWindowState.r.top + g_config->ReadInt(L"mw_height", 348);
		SET_EMBED_GUID((&myWindowState), library_guid);

		g_ownerwnd = (HWND)SendMessage(plugin.hwndParent, WM_WA_IPC, (LPARAM) & myWindowState, IPC_GET_EMBEDIF);
		if (!g_ownerwnd) return FALSE;

		if (NULL != WASABI_API_APP) WASABI_API_APP->app_registerGlobalWindow(g_ownerwnd);

		SetWindowTextW(g_ownerwnd, WASABI_API_LNGSTRINGW(IDS_WINAMP_LIBRARY));
		g_hwnd = WASABI_API_CREATEDIALOGW(IDD_MAIN, g_ownerwnd, dialogProc);
		if (!g_hwnd)
		{
			DestroyWindow(g_ownerwnd);
			g_ownerwnd = NULL;
			return FALSE;
		}		
	}
	return TRUE;
}

wchar_t WINAMP_INI[MAX_PATH] = {0}, WINAMP_INI_DIR[MAX_PATH] = {0};
MediaLibraryCOM mediaLibraryCOM;
IDispatch *winampExternal = 0;

void TAG_FMT_EXT(const wchar_t *filename, void *f, void *ff, void *p, wchar_t *out, int out_len, int extended)
{
	waFormatTitleExtended fmt; 
	fmt.filename=filename;
	fmt.useExtendedInfo=extended;
	fmt.out = out;
	fmt.out_len = out_len;
	fmt.p = p;
	fmt.spec = 0;
	*(void **)&fmt.TAGFUNC = f;
	*(void **)&fmt.TAGFREEFUNC = ff;
	*out = 0;

	int oldCallingGetFileInfo=m_calling_getfileinfo;
	m_calling_getfileinfo=1;
	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&fmt, IPC_FORMAT_TITLE_EXTENDED);
	m_calling_getfileinfo=oldCallingGetFileInfo;
}

wchar_t *itemrecordTagFunc(wchar_t *tag, void * p) //return 0 if not found
{
	itemRecord *t = (itemRecord *)p;
	char buf[128] = {0};
	char *value = NULL;

	if (!_wcsicmp(tag, L"artist"))	value = t->artist;
	else if (!_wcsicmp(tag, L"album"))	value = t->album;
	else if (!_wcsicmp(tag, L"filename")) value = t->filename;
	else if (!_wcsicmp(tag, L"title"))	value = t->title;
	else if ( !_wcsicmp( tag, L"ext" ) ) value = t->ext;
	else if (!_wcsicmp(tag, L"year"))
	{
		if (t->year > 0)
		{
			StringCchPrintfA(buf, 128, "%04d", t->year);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"genre"))	value = t->genre;
	else if (!_wcsicmp(tag, L"comment")) value = t->comment;
	else if (!_wcsicmp(tag, L"tracknumber") || !_wcsicmp(tag, L"track"))
	{
		if (t->track > 0)
		{
			StringCchPrintfA(buf, 128, "%02d", t->track);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"rating")) value = getRecordExtendedItem(t, "RATING");
	else if (!_wcsicmp(tag, L"playcount")) value = getRecordExtendedItem(t, "PLAYCOUNT");
	else if (!_wcsicmp(tag, L"bitrate")) value = getRecordExtendedItem(t, "BITRATE");
	else
		return 0;

	if (!value) return reinterpret_cast<wchar_t *>(-1);
	else return AutoWideDup(value);
}

wchar_t *itemrecordWTagFunc(wchar_t *tag, void * p) //return 0 if not found
{
	itemRecordW *t = (itemRecordW *)p;
	wchar_t buf[128] = {0};
	wchar_t *value = NULL;

	// TODO: more fields
	if (!_wcsicmp(tag, L"artist")) value = t->artist;
	else if (!_wcsicmp(tag, L"album")) value = t->album;
	else if (!_wcsicmp(tag, L"albumartist")) value = t->albumartist;
	else if (!_wcsicmp(tag, L"category")) value = t->category;
	else if (!_wcsicmp(tag, L"comment")) value = t->comment;
	else if (!_wcsicmp(tag, L"composer")) value = t->composer;
	else if (!_wcsicmp(tag, L"publisher")) value = t->publisher;
	else if (!_wcsicmp(tag, L"filename")) value = t->filename;
	else if (!_wcsicmp(tag, L"title")) value = t->title;
	else if (!_wcsicmp(tag, L"year"))
	{
		if (t->year > 0)
		{
			StringCchPrintfW(buf, 128, L"%04d", t->year);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"genre"))	value = t->genre;
	else if (!_wcsicmp(tag, L"comment")) value = t->comment;
	else if (!_wcsicmp(tag, L"tracknumber") || !_wcsicmp(tag, L"track"))
	{
		if (t->track > 0)
		{
			StringCchPrintfW(buf, 128, L"%02d", t->track);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"rating"))
	{
		if (t->rating > 0)
		{
			StringCchPrintfW(buf, 128, L"%d", t->rating);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"playcount"))
	{
		if (t->playcount > 0)
		{
			StringCchPrintfW(buf, 128, L"%d", t->playcount);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"bitrate"))
	{
		if (t->bitrate > 0)
		{
			StringCchPrintfW(buf, 128, L"%d", t->bitrate);
			value = buf;
		}
	}
	else
		return 0;

	if (!value) return reinterpret_cast<wchar_t *>(-1);
	else return _wcsdup(value); 
}


void fieldTagFuncFree(wchar_t * tag, void * p)
{
	free(tag);
}

void main_playItemRecordList(itemRecordList *obj, int enqueue, int startplaying)
{
	if (obj->Size && !enqueue) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_DELETE);

	int x;
	wchar_t title[2048]=L"";

	for (x = 0; x < obj->Size; x ++)
	{
		if (obj->Items[x].filename && *obj->Items[x].filename)
		{
			AutoWideFn wfn( obj->Items[ x ].filename );

			TAG_FMT_EXT(wfn, itemrecordTagFunc, fieldTagFuncFree, (void*)&obj->Items[x], title, 2048, 1);

			{
				enqueueFileWithMetaStructW s;
				s.filename = wfn;
				s.title    = title;
				s.ext      = NULL;
				s.length   = obj->Items[x].length;
				SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
			}
		}
	}

	if (obj->Size && !enqueue && startplaying) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_STARTPLAY);
}

void main_playItemRecordListW(itemRecordListW *obj, int enqueue, int startplaying)
{
	if (obj->Size && !enqueue) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_DELETE);

	int x;
	wchar_t title[2048]=L"";

	for (x = 0; x < obj->Size; x ++)
	{
		if (obj->Items[x].filename && *obj->Items[x].filename)
		{
			TAG_FMT_EXT(obj->Items[x].filename, itemrecordWTagFunc, fieldTagFuncFree, (void*)&obj->Items[x], title, 2048, 1);
			{
				enqueueFileWithMetaStructW s;
				s.filename = obj->Items[x].filename;
				s.title    = title;
				s.ext      = NULL;
				s.length   = obj->Items[x].length;
				SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
			}
		}
	}

	if (obj->Size && !enqueue && startplaying) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_STARTPLAY);
}

void OpenMediaLibraryPreferences()
{
	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&myPrefsItem, IPC_OPENPREFSTOPAGE);
}

int AddTreeImageBmp(int resourceId)
{
	HMLIMGLST hmlilNavigation = MLNavCtrl_GetImageList(g_hwnd);
	MLIMAGESOURCE mlis = {sizeof(MLIMAGESOURCE),0};
	MLIMAGELISTITEM item = {0};
	item.cbSize = sizeof(MLIMAGELISTITEM);
	item.hmlil = hmlilNavigation;
	item.filterUID = MLIF_FILTER1_UID;
	item.pmlImgSource = &mlis;

	mlis.hInst = WASABI_API_ORIG_HINST;
	mlis.bpp = 24;
	mlis.lpszName = MAKEINTRESOURCEW(resourceId);
	mlis.type = SRC_TYPE_BMP;
	mlis.flags = ISF_FORCE_BPP;
	return MLImageList_Add(g_hwnd, &item);
}

void SkinnedScrollWnd_Init();
void SkinnedScrollWnd_Quit();
int init()
{
	wchar_t g_path[MAX_PATH] = {0};
	QueryPerformanceFrequency(&freq);

	WASABI_API_SVC = (api_service*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (!WASABI_API_SVC || WASABI_API_SVC == (api_service *)1)
		return GEN_INIT_FAILURE;

	HTMLContainer2_Initialize();
	Tataki::Init(WASABI_API_SVC);

	// loader so that we can get the localisation service api for use
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(WASABI_API_SYSCB, syscbApiServiceGuid);
	ServiceBuild(AGAVE_API_DECODE, decodeFileGUID);
	ServiceBuild(AGAVE_API_JSAPI2_SECURITY, JSAPI2::api_securityGUID);
	ServiceBuild(AGAVE_OBJ_BROWSER, OBJ_OmBrowser);
	#ifndef IGNORE_API_GRACENOTE
	ServiceBuild(AGAVE_API_GRACENOTE, gracenoteApiGUID);
	#endif
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_PALETTE, PaletteManagerGUID);	
	ServiceBuild(AGAVE_API_THREADPOOL, ThreadPoolGUID);
	// no guarantee that AGAVE_API_MLDB will be available yet, so we'll start a watcher for it
	serviceWatcher.WatchWith(WASABI_API_SVC);
	serviceWatcher.WatchFor(&AGAVE_API_MLDB, mldbApiGuid);
	serviceWatcher.WatchFor(&WASABI_API_SKIN, skinApiServiceGuid);
	serviceWatcher.WatchFor(&WASABI_API_WND,wndApiServiceGuid);
	WASABI_API_SYSCB->syscb_registerCallback(&serviceWatcher);
	SkinnedScrollWnd_Init();

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,GenMlLangGUID);

	// Build plugin description string...
	static wchar_t szDescription[256];
	StringCchPrintfW(szDescription, ARRAYSIZE(szDescription),
					 WASABI_API_LNGSTRINGW(IDS_NULLSOFT_ML_STR),
					 LOWORD(PLUGIN_VERSION) >> 8,
					 PLUGIN_VERSION & 0xFF);
	plugin.description = (char*)szDescription;

	DispatchInfo dispatchInfo;
	dispatchInfo.name = L"MediaLibrary";
	dispatchInfo.dispatch = &mediaLibraryCOM;
	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&dispatchInfo, IPC_ADD_DISPATCH_OBJECT);

	#ifndef IGNORE_API_GRACENOTE
	dispatchInfo.name = L"MusicID";
	dispatchInfo.dispatch = &musicIDCOM;
	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&dispatchInfo, IPC_ADD_DISPATCH_OBJECT);
	#endif

	IPC_LIBRARY_SENDTOMENU = (INT)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&"LibrarySendToMenu", IPC_REGISTER_WINAMP_IPCMESSAGE);
	IPC_GETMLWINDOW = (INT)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&"LibraryGetWnd", IPC_REGISTER_WINAMP_IPCMESSAGE);
	IPC_GET_ML_HMENU = (INT)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&"LibraryGetHmenu", IPC_REGISTER_WINAMP_IPCMESSAGE);

	lstrcpynW(WINAMP_INI, (wchar_t*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETINIFILEW), MAX_PATH);
	lstrcpynW(WINAMP_INI_DIR, (wchar_t*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETINIDIRECTORYW), MAX_PATH);

	PathCombineW(g_path, WINAMP_INI_DIR, L"Plugins");
	CreateDirectoryW(g_path, NULL);

	wchar_t *dir = (wchar_t*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETPLUGINDIRECTORYW);
	if (dir == (wchar_t *)1 || dir == 0)
		lstrcpynW(pluginPath, g_path, MAX_PATH);
	else
		lstrcpynW(pluginPath, dir, MAX_PATH);

	hDragNDropCursor = LoadCursor(plugin.hDllInstance, MAKEINTRESOURCE(ML_IDC_DRAGDROP));
	profile = GetPrivateProfileIntW(L"winamp", L"profile", 0, WINAMP_INI);

	wchar_t configName[1024 + 32] = {0};
	StringCchPrintfW(configName, 1024 + 32, L"%s\\gen_ml.ini", g_path);
	g_config = new C_Config(configName);
	config_use_ff_scrollbars = g_config->ReadInt(L"ffsb", 1);
	config_use_alternate_colors = g_config->ReadInt(L"alternate_items", 1);
	
	int vis = g_config->ReadInt(L"visible", 1);
	wa_main_menu = (HMENU)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_HMENU);
	wa_windows_menu = (HMENU)SendMessage(plugin.hwndParent, WM_WA_IPC, 4, IPC_GET_HMENU);
	wa_playlists_cmdmenu = NULL;
	if (wa_main_menu || wa_windows_menu)
	{
		if (wa_main_menu)
		{
			MENUITEMINFOW i = {sizeof(i), MIIM_ID | MIIM_STATE | MIIM_TYPE, MFT_STRING, vis ? (UINT)MFS_CHECKED : 0, WA_MENUITEM_ID};
			int prior_item = GetMenuItemID(wa_main_menu,9);
			if(prior_item <= 0) prior_item = GetMenuItemID(wa_main_menu,8);
			i.dwTypeData = WASABI_API_LNGSTRINGW(IDS_ML_ALT_L_SHORTCUT);

			// append before the video menu entry (more reliable than inserting into position '9' in the menu
			InsertMenuItemW(wa_main_menu, prior_item, FALSE, &i);
			SendMessage(plugin.hwndParent, WM_WA_IPC, 1, IPC_ADJUST_OPTIONSMENUPOS);
		}

		if (wa_windows_menu)
		{
			MENUITEMINFOW i = {sizeof(i), MIIM_ID | MIIM_STATE | MIIM_TYPE, MFT_STRING, vis ? (UINT)MFS_CHECKED : 0, WA_MENUITEM_ID};
			int prior_item = GetMenuItemID(wa_windows_menu,3);
			if(prior_item <= 0) prior_item = GetMenuItemID(wa_windows_menu,2);
			i.dwTypeData = WASABI_API_LNGSTRINGW(IDS_ML_ALT_L_SHORTCUT);
			InsertMenuItemW(wa_windows_menu, prior_item, FALSE, &i);
			SendMessage(plugin.hwndParent, WM_WA_IPC, 1, IPC_ADJUST_FFWINDOWSMENUPOS);
		}
	}

	// subclass the winamp window to get our leet menu item to work
	wa_oldWndProc = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(plugin.hwndParent, GWLP_WNDPROC, (LONGX86)(LONG_PTR)wa_newWndProc);

	myPrefsItem.dlgID = IDD_PREFSFR;
	myPrefsItem.name = WASABI_API_LNGSTRINGW_BUF(IDS_MEDIA_LIBRARY,preferencesName,128);
	myPrefsItem.proc = (void*)PrefsProc;
	myPrefsItem.hInst = WASABI_API_LNG_HINST;
	myPrefsItem.where = -6; // to become root based item
	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&myPrefsItem, IPC_ADD_PREFS_DLGW);

	myPrefsItemPlug.dlgID = IDD_MLPLUGINS;
	myPrefsItemPlug.name = preferencesName;
	myPrefsItemPlug.proc = (void*)PluginsProc;
	myPrefsItemPlug.hInst = WASABI_API_LNG_HINST;
	myPrefsItemPlug.where = 1;
	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&myPrefsItemPlug, IPC_ADD_PREFS_DLGW);

	g_PEWindow = (HWND)SendMessage(plugin.hwndParent, WM_WA_IPC, IPC_GETWND_PE, IPC_GETWND);
	g_safeMode = SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_IS_SAFEMODE);

	// we're gonna go ahead and make this directory just to be safe.
	// if a plugin tries to use it as an INI directory but it doesn't exist, things go wrong
	wchar_t mldir[MAX_PATH] = {0};
	PathCombineW(mldir, g_path, L"ml");
	CreateDirectoryW(mldir, NULL);
	PathCombineW(mldir, mldir, L"views");
	CreateDirectoryW(mldir, NULL);

	//add general hotkey
	int m_genhotkeys_add_ipc = (INT)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&"GenHotkeysAdd", IPC_REGISTER_WINAMP_IPCMESSAGE);

	static genHotkeysAddStruct ghas = {
		(char*)_wcsdup(WASABI_API_LNGSTRINGW(IDS_ML_GHK_STR)),
	    HKF_BRING_TO_FRONT|HKF_UNICODE_NAME,
	    WM_COMMAND,
	    WA_MENUITEM_ID,
	    0,
		// specifically set the id str now so that it'll work correctly with whatever lngpack is in use
		"ML: Show/Hide Media Library"
	};
	if (m_genhotkeys_add_ipc > 65536) PostMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&ghas, m_genhotkeys_add_ipc); //post so gen_hotkeys will catch it if not inited yet

	init2();

	// register the art view window classes
	{
		extern void InitSmoothScrollList();
		extern void InitHeaderIconList();
		InitSmoothScrollList();
		InitHeaderIconList();
		RegisterFolderBrowserControl(plugin.hDllInstance);
	}

	NavCtrlI_BeginUpdate(hNavigation, NUF_LOCK_NONE_I);
	loadMlPlugins();

#if 0
	#ifdef _DEBUG
	#define BETA
	#endif
	#ifdef BETA
	sneak = GetPrivateProfileIntW(L"winamp", L"sneak", 0, WINAMP_INI);
	if (!(sneak & 1))
	{
		NAVINSERTSTRUCT nis = {0};
		nis.item.cbSize = sizeof(NAVITEM);
		nis.item.pszText = L"Winamp Labs";
		nis.item.pszInvariant = L"winamp_labs";
		nis.item.mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL | NIMF_STYLE;
		nis.item.iImage = nis.item.iSelectedImage = AddTreeImageBmp(IDB_TREEITEM_LABS);
		nis.item.style = NIS_BOLD;
		nis.hInsertAfter = NCI_FIRST;
		NAVITEM nvItem = {sizeof(NAVITEM),0,NIMF_ITEMID,};
		nvItem.hItem = MLNavCtrl_InsertItem(g_hwnd, &nis);
	}
	#endif
#endif

	NavCtrlI_EndUpdate(hNavigation);
	
	if (SW_SHOWMINIMIZED == SENDWAIPC(plugin.hwndParent, IPC_INITIAL_SHOW_STATE, 0))
	{
		MlWindow_SetMinimizedMode(TRUE);
	}
	else if (0 != vis)
	{
		PostMessageW(plugin.hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_SHOWLIBRARY, 2), 0L); 
	}

	return GEN_INIT_SUCCESS;
}


void quit()
{
	serviceWatcher.StopWatching();
	serviceWatcher.Clear();

	MlWindow_SetMinimizedMode(FALSE);

	if (g_ownerwnd)
	{
		g_config->WriteInt(L"mw_xpos", myWindowState.r.left);
		g_config->WriteInt(L"mw_ypos", myWindowState.r.top);
		g_config->WriteInt(L"mw_width", myWindowState.r.right - myWindowState.r.left);
		g_config->WriteInt(L"mw_height", myWindowState.r.bottom - myWindowState.r.top);

		if (NULL != WASABI_API_APP) WASABI_API_APP->app_unregisterGlobalWindow(g_ownerwnd);
		DestroyWindow(g_ownerwnd);
		g_ownerwnd = NULL;
	}

	// unload any services from ml_ plugins before unloading the plugins
	ServiceRelease(AGAVE_API_MLDB, mldbApiGuid);	

	#ifndef IGNORE_API_GRACENOTE
	musicIDCOM.Quit();
	#endif

	unloadMlPlugins();	

	WADlg_close();

	if (g_config)
	{
		delete g_config;
		g_config = NULL;
	}

	if (m_uxdll)
	{
		FreeLibrary(m_uxdll);
		m_uxdll = NULL;
	}
	SkinnedScrollWnd_Quit();
	#ifndef IGNORE_API_GRACENOTE
	ServiceRelease(AGAVE_API_GRACENOTE, gracenoteApiGUID);
	#endif
	ServiceRelease(WASABI_API_SYSCB, syscbApiServiceGuid);
	ServiceRelease(AGAVE_API_DECODE, decodeFileGUID);
	ServiceRelease(AGAVE_API_JSAPI2_SECURITY, JSAPI2::api_securityGUID);
	ServiceRelease(AGAVE_OBJ_BROWSER, OBJ_OmBrowser);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(WASABI_API_WND, wndApiServiceGuid);
	ServiceRelease(WASABI_API_SKIN, skinApiServiceGuid);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_PALETTE, PaletteManagerGUID);
	
	Tataki::Quit();

	HTMLContainer2_Uninitialize();

	ServiceRelease(AGAVE_API_THREADPOOL, ThreadPoolGUID);
}

void config()
{
	OpenMediaLibraryPreferences();
}

INT MediaLibrary_TrackPopupEx(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, HMLIMGLST hmlil, 
							INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam)
{
	if (NULL == hMenu)
		return NULL;

	return IsSkinnedPopupEnabled(FALSE) ?
		TrackSkinnedPopupMenuEx(hMenu, fuFlags, x, y, hwnd, lptpm, hmlil, width, skinStyle, customProc, customParam) :
		TrackPopupMenuEx(hMenu, fuFlags, x, y, hwnd, lptpm);
}


INT MediaLibrary_TrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd)
{	
	return MediaLibrary_TrackPopupEx(hMenu, fuFlags, x, y, hwnd, NULL, NULL, 0, SMS_USESKINFONT, NULL, NULL);
}

 HANDLE MediaLibrary_InitSkinnedPopupHook(HWND hwnd, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam)
 {
	 if (FALSE == IsSkinnedPopupEnabled(FALSE))
		 return FALSE;

	 return InitSkinnedPopupHook(hwnd, hmlil, width, skinStyle, customProc, customParam);
 }



BOOL 
MediaLibrary_OpenUrl(HWND ownerWindow, const wchar_t *url, BOOL forceExternal)
{	
	BOOL result;
	HCURSOR cursor;

	cursor = LoadCursor(NULL, IDC_APPSTARTING);
	if (NULL != cursor) 
		cursor = SetCursor(cursor);

	if (FALSE != forceExternal)
	{		
		HINSTANCE instance;

		if (NULL == ownerWindow)
			ownerWindow = (HWND)SENDWAIPC(plugin.hwndParent, IPC_GETDIALOGBOXPARENT, 0);

		instance = ShellExecuteW(ownerWindow, L"open", url, NULL, NULL, SW_SHOWNORMAL);
		result = ((INT_PTR)instance > 32) ? TRUE: FALSE;
	}
	else
	{
		SENDWAIPC(plugin.hwndParent, IPC_OPEN_URL, url);
		result = TRUE;
	}
		
	if (NULL != cursor) 
		SetCursor(cursor);

	return result;
}

BOOL
MediaLibrary_OpenHelpUrl(const wchar_t *helpUrl)
{
	HWND ownerWindow;
	
	ownerWindow = (HWND)SENDWAIPC(plugin.hwndParent, IPC_GETDIALOGBOXPARENT, 0);

	return MediaLibrary_OpenUrl(ownerWindow, helpUrl, FALSE);
}

extern "C"
{
	int getFileInfo(const char *filename, const char *metadata, char *dest, int len)
	{
		m_calling_getfileinfo = 1;
		dest[0] = 0;
		extendedFileInfoStruct efis = {
		                                  filename,
		                                  metadata,
		                                  dest,
		                                  (size_t)len,
		                              };
		int r = (INT)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM) & efis, IPC_GET_EXTENDED_FILE_INFO); //will return 1 if wa2 supports this IPC call
		m_calling_getfileinfo = 0;
		return r;
	}

	__declspec(dllexport) winampGeneralPurposePlugin *winampGetGeneralPurposePlugin()
	{
		return &plugin;
	}
};

