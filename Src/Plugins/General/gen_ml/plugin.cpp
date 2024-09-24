#include "main.h"
#include "ml.h"
#include "itemlist.h"
#include "../winamp/gen.h"
#include "config.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/ipc_pe.h"
#include "resource.h"
#include "comboskin.h"
#include "../winamp/wa_dlg.h"
#include "childwnd.h"
#include "sendto.h"
#include "api__gen_ml.h"
#include "../nu/autowide.h"
#include "../nu/autochar.h"
#if defined(_WIN64)
#include "../Elevator/IFileTypeRegistrar_64.h"
#else
#include "../Elevator/IFileTypeRegistrar_32.h"
#endif
#include <time.h>
#include <shlwapi.h>
#include <strsafe.h>

#include "..\WAT\wa_logger.h"

#define ListView_InsertColumnW(hwnd, iCol, pcol) \
    (int)SNDMSG((hwnd), LVM_INSERTCOLUMNW, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMNW *)(pcol))

#define ListView_InsertItemW(hwnd, pitem)   \
    (int)SNDMSG((hwnd), LVM_INSERTITEMW, 0, (LPARAM)(const LV_ITEMW *)(pitem))

#define ListView_SetItemTextW(hwndLV, i, iSubItem_, pszText_) \
{ LV_ITEMW _macro_lvi;\
  _macro_lvi.iSubItem = (iSubItem_);\
  _macro_lvi.pszText = (pszText_);\
  SNDMSG((hwndLV), LVM_SETITEMTEXTW, (WPARAM)(i), (LPARAM)(LV_ITEMW *)&_macro_lvi);\
}

#define ListView_SetItemW(hwnd, pitem) \
    (BOOL)SNDMSG((hwnd), LVM_SETITEMW, 0, (LPARAM)(LV_ITEMW *)(pitem))

#define ListView_GetItemW(hwnd, pitem) \
    (BOOL)SNDMSG((hwnd), LVM_GETITEMW, 0, (LPARAM)(LV_ITEMW *)(pitem))

extern "C" winampGeneralPurposePlugin plugin;
C_ItemList m_plugins;

static HCURSOR link_hand_cursor;
LRESULT link_handlecursor(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"link_proc"), hwndDlg, uMsg, wParam, lParam);
	// override the normal cursor behaviour so we have a hand to show it is a link
	if ( uMsg == WM_SETCURSOR )
	{
		if ( (HWND)wParam == hwndDlg )
		{
			if ( !link_hand_cursor )
			{
				link_hand_cursor = LoadCursor(NULL, IDC_HAND);
			}
			SetCursor(link_hand_cursor);
			return TRUE;
		}
	}
	return ret;
}

void link_startsubclass(HWND hwndDlg, UINT id)
{
	HWND ctrl = GetDlgItem(hwndDlg, id);
	SetPropW(ctrl, L"link_proc",
		(HANDLE)(LONG_PTR)SetWindowLongPtrW(ctrl, GWLP_WNDPROC, (LONGX86)(LONG_PTR)link_handlecursor));
}

void link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( uMsg == WM_DRAWITEM )
	{
		DRAWITEMSTRUCT* di = (DRAWITEMSTRUCT*)lParam;
		if ( di->CtlType == ODT_BUTTON )
		{
			wchar_t wt[123] = { 0 };
			int y;
			RECT r;
			HPEN hPen, hOldPen;
			GetDlgItemTextW(hwndDlg, (INT)wParam, wt, 123);

			// draw text
			SetTextColor(di->hDC, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
			r = di->rcItem;
			r.left += 2;
			DrawTextW(di->hDC, wt, -1, &r, DT_VCENTER | DT_SINGLELINE);

			memset(&r, 0, sizeof(r));
			DrawTextW(di->hDC, wt, -1, &r, DT_SINGLELINE | DT_CALCRECT);

			// draw underline
			y = di->rcItem.bottom - ((di->rcItem.bottom - di->rcItem.top) - (r.bottom - r.top)) / 2 - 1;
			hPen = CreatePen(PS_SOLID, 0, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
			hOldPen = (HPEN)SelectObject(di->hDC, hPen);
			MoveToEx(di->hDC, di->rcItem.left + 2, y, NULL);
			LineTo(di->hDC, di->rcItem.right + 2 - ((di->rcItem.right - di->rcItem.left) - (r.right - r.left)), y);
			SelectObject(di->hDC, hOldPen);
			DeleteObject(hPen);

		}
	}
}

/* In Winamp's preferences, Plugins->Media Library  */
static bool pluginsLoaded;
INT_PTR CALLBACK PluginsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( uMsg == WM_INITDIALOG )
	{
		pluginsLoaded = false;

		extern BOOL init2();
		if ( !g_hwnd ) init2();
		WIN32_FIND_DATAW d = { 0 };

		link_startsubclass(hwndDlg, IDC_PLUGINVERS);

		HWND listWindow = GetDlgItem(hwndDlg, IDC_GENLIB);
		if ( NULL != listWindow )
		{
			RECT r = { 0 }, rc = { 0 };
			GetWindowRect(listWindow, &r);
			GetClientRect(listWindow, &r);
			MapWindowPoints(listWindow, hwndDlg, (LPPOINT)&r, 2);
			InflateRect(&r, 2, 2);
			DestroyWindow(listWindow);
			listWindow = CreateWindowExW(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL |
				LVS_SHOWSELALWAYS | LVS_SORTASCENDING | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER,
				r.left, r.top, r.right - r.left, r.bottom - r.top,
				hwndDlg, (HMENU)IDC_GENLIB, NULL, NULL);
			SetWindowPos(listWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
			ListView_SetExtendedListViewStyleEx(listWindow, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
			SendMessage(listWindow, WM_SETFONT, SendMessage(hwndDlg, WM_GETFONT, 0, 0), FALSE);

			LVCOLUMNW lvc = { 0 };
			ListView_InsertColumnW(listWindow, 0, &lvc);
			ListView_InsertColumnW(listWindow, 1, &lvc);

			wchar_t filename[MAX_PATH] = { 0 }, description[512] = { 0 };
			for ( int x = 0; x < m_plugins.GetSize(); x++ )
			{
				winampMediaLibraryPlugin* mlplugin = (winampMediaLibraryPlugin*)m_plugins.Get(x);

				if ( mlplugin && mlplugin->hDllInstance )
				{
					wchar_t buf[512] = { 0 };
					LVITEMW lvi = { LVIF_TEXT | LVIF_PARAM, x, 0 };
					lstrcpynW(buf, (mlplugin->version >= MLHDR_VER_U ? (wchar_t*)mlplugin->description : AutoWide(mlplugin->description)), 512);
					lvi.pszText = buf;
					lvi.lParam = x;
					lvi.iItem = ListView_InsertItemW(listWindow, &lvi);

					GetModuleFileNameW(mlplugin->hDllInstance, filename, ARRAYSIZE(filename));
					PathStripPathW(filename);

					lvi.mask = LVIF_TEXT;
					lvi.iSubItem = 1;
					lvi.pszText = filename;
					ListView_SetItemW(listWindow, &lvi);
				}
			}

			wchar_t dirstr[MAX_PATH] = { 0 };
			PathCombineW(dirstr, pluginPath, L"ML_*.DLL");
			HANDLE h = FindFirstFileW(dirstr, &d);
			if ( h != INVALID_HANDLE_VALUE )
			{
				do
				{
					PathCombineW(dirstr, pluginPath, d.cFileName);

					HMODULE b = LoadLibraryExW(dirstr, NULL, LOAD_LIBRARY_AS_DATAFILE);

					int x = 0;
					for ( x = 0; b && (x != m_plugins.GetSize()); x++ )
					{
						winampMediaLibraryPlugin* mlplugin = (winampMediaLibraryPlugin*)m_plugins.Get(x);
						if ( mlplugin->hDllInstance == b )
						{
							break;
						}
					}

					if ( x == m_plugins.GetSize() || !b )
					{
						LVITEMW lvi = { LVIF_TEXT | LVIF_PARAM, x, 0 };
						lvi.pszText  = d.cFileName;
						lvi.lParam   = -2;
						lvi.iItem    = ListView_InsertItemW(listWindow, &lvi);

						lvi.mask     = LVIF_TEXT;
						lvi.iSubItem = 1;
						lvi.pszText  = WASABI_API_LNGSTRINGW(IDS_NOT_LOADED);
						ListView_SetItemW(listWindow, &lvi);
					}

					FreeLibrary(b);
				} while ( FindNextFileW(h, &d) );

				FindClose(h);
			}

			GetClientRect(listWindow, &r);
			ListView_SetColumnWidth(listWindow, 1, LVSCW_AUTOSIZE);
			ListView_SetColumnWidth(listWindow, 0, (r.right - r.left) - ListView_GetColumnWidth(listWindow, 1));

			if ( NULL != WASABI_API_APP )
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(listWindow, TRUE);

			pluginsLoaded = true;
		}
	}
	else if ( uMsg == WM_NOTIFY )
	{
		LPNMHDR p = (LPNMHDR)lParam;
		if ( p->idFrom == IDC_GENLIB )
		{
			if ( p->code == LVN_ITEMCHANGED )
			{
				LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
				LVITEM lvi = { LVIF_PARAM, pnmv->iItem };
				if ( ListView_GetItem(p->hwndFrom, &lvi) && (pnmv->uNewState & LVIS_SELECTED) )
				{
					int loaded = (lvi.lParam != -2);
					if ( loaded )
					{
						winampMediaLibraryPlugin* mlplugin;
						if ( lvi.lParam >= 0 && lvi.lParam < m_plugins.GetSize() && (mlplugin = (winampMediaLibraryPlugin*)m_plugins.Get(lvi.lParam)) )
						{
							EnableWindow(GetDlgItem(hwndDlg, IDC_UNINST), !!mlplugin->hDllInstance);

							// enables / disables the config button as applicable instead of the
							// "This plug-in has no configuration implemented" message (opt-in)
							EnableWindow(GetDlgItem(hwndDlg, IDC_GENCONF), (!mlplugin->MessageProc(ML_MSG_NO_CONFIG, 0, 0, 0)));
						}
					}
					else
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_GENCONF), 0);
					}
					EnableWindow(GetDlgItem(hwndDlg, IDC_UNINST), 1);
				}
				else
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_GENCONF), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_UNINST), 0);
				}
			}
			else if ( p->code == NM_DBLCLK )
			{
				PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_GENCONF, 0), (LPARAM)GetDlgItem(hwndDlg, IDC_GENCONF));
			}
		}
		else if ( p->code == HDN_ITEMCHANGINGW )
		{
			if ( pluginsLoaded )
			{
#if defined(_WIN64)
				SetWindowLong(hwndDlg, DWLP_MSGRESULT, TRUE);
#else
				SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
#endif
				return TRUE;
			}
		}
	}
	else if ( uMsg == WM_DESTROY )
	{
		HWND listWindow = GetDlgItem(hwndDlg, IDC_GENLIB);
		if ( IsWindow(listWindow) && (NULL != WASABI_API_APP) )
			WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
	}
	else if ( uMsg == WM_COMMAND )
	{
		switch ( LOWORD(wParam) )
		{
		case IDC_GENCONF:
		{
			if ( IsWindowEnabled(GetDlgItem(hwndDlg, IDC_GENCONF)) )
			{
				HWND listWindow = GetDlgItem(hwndDlg, IDC_GENLIB);
				LVITEM lvi = { LVIF_PARAM, ListView_GetSelectionMark(listWindow) };
				if ( ListView_GetItem(listWindow, &lvi) )
				{
					winampMediaLibraryPlugin* mlplugin;
					if ( lvi.lParam >= 0 && lvi.lParam < m_plugins.GetSize() && (mlplugin = (winampMediaLibraryPlugin*)m_plugins.Get(lvi.lParam)) )
					{
						if ( mlplugin->MessageProc && mlplugin->MessageProc(ML_MSG_CONFIG, (INT_PTR)hwndDlg, 0, 0) )
						{
						}
						else
						{
							wchar_t title[128] = { 0 };
							MessageBoxW(hwndDlg, WASABI_API_LNGSTRINGW(IDS_NO_CONFIG_PRESENT),
								WASABI_API_LNGSTRINGW_BUF(IDS_ML_PLUGIN_INFO, title, 128), MB_OK);
						}
					}
				}
			}
		}
		return FALSE;

		case IDC_UNINST:
		{
			if ( IsWindowEnabled(GetDlgItem(hwndDlg, IDC_UNINST)) )
			{
				HWND listWindow = GetDlgItem(hwndDlg, IDC_GENLIB);
				int which_sel = ListView_GetSelectionMark(listWindow);
				LVITEM lvi = { LVIF_PARAM, which_sel };
				if ( ListView_GetItem(listWindow, &lvi) )
				{
					winampMediaLibraryPlugin* mlplugin;
					wchar_t title[32] = { 0 };
					int msgBox = MessageBoxW(hwndDlg, WASABI_API_LNGSTRINGW(IDS_UNINSTALL_PROMPT),
						WASABI_API_LNGSTRINGW_BUF(IDS_UINSTALL_CONFIRMATION, title, 32),
						MB_YESNO | MB_ICONEXCLAMATION);

					if ( lvi.lParam >= 0 && lvi.lParam < m_plugins.GetSize() && (mlplugin = (winampMediaLibraryPlugin*)m_plugins.Get(lvi.lParam)) && msgBox == IDYES )
					{
						int ret = 0;
						int (*pr)(HINSTANCE hDllInst, HWND hwndDlg, int param);

						*(void**)&pr = (void*)GetProcAddress(mlplugin->hDllInstance, "winampUninstallPlugin");
						if ( pr )ret = pr(mlplugin->hDllInstance, hwndDlg, 0);
						// ok to uninstall but do with a full restart (default/needed in subclassing cases)
						if ( ret == ML_PLUGIN_UNINSTALL_REBOOT )
						{
							wchar_t buf[MAX_PATH] = { 0 };
							GetModuleFileNameW(mlplugin->hDllInstance, buf, MAX_PATH);
							WritePrivateProfileStringW(L"winamp", L"remove_genplug", buf, WINAMP_INI);
							WritePrivateProfileStringW(L"winamp", L"show_prefs", L"-1", WINAMP_INI);
							PostMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_RESTARTWINAMP);
						}
						// added from 5.37+ so we can do true on-the-fly removals (will fall back to default if fails)
						else if ( ret == ML_PLUGIN_UNINSTALL_NOW )
						{
							// get the filename before we free the dll otherwise things may go boom
							wchar_t buf[MAX_PATH] = { 0 };
							GetModuleFileNameW(mlplugin->hDllInstance, buf, MAX_PATH);

							mlplugin->quit();
							//if (mlplugin->hDllInstance) FreeLibrary(mlplugin->hDllInstance);
							m_plugins.Del(lvi.lParam);

							// try to use the elevator to do this
							IFileTypeRegistrar* registrar = (IFileTypeRegistrar*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_FILEREGISTRAR_OBJECT);
							if ( registrar && (registrar != (IFileTypeRegistrar*)1) )
							{
								if ( registrar->DeleteItem(buf) != S_OK )
								{
									WritePrivateProfileStringW(L"winamp", L"remove_genplug", buf, WINAMP_INI);
									WritePrivateProfileStringW(L"winamp", L"show_prefs", L"-1", WINAMP_INI);
									PostMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_RESTARTWINAMP);
								}
								else
									ListView_DeleteItem(listWindow, which_sel);
								registrar->Release();
							}
						}
					}
					// will cope with not loaded plug-ins so we can still remove them, etc
					else if ( lvi.lParam == -2 && msgBox == IDYES )
					{
						wchar_t buf[1024] = { 0 }, base[1024] = { 0 };
						GetModuleFileNameW(plugin.hDllInstance, base, sizeof(base) / sizeof(wchar_t));

						LVITEMW lvi = { LVIF_TEXT, which_sel };
						lvi.pszText = buf;
						lvi.cchTextMax = ARRAYSIZE(buf);
						ListView_GetItemW(listWindow, &lvi);

						wchar_t* p = wcschr(buf, L'.');
						if ( p && *p == L'.' )
						{
							p += 4;
							*p = 0;
							PathRemoveFileSpecW(base);
							PathAppendW(base, buf);
						}

						// try to use the elevator to do this
						IFileTypeRegistrar* registrar = (IFileTypeRegistrar*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_FILEREGISTRAR_OBJECT);
						if ( registrar && (registrar != (IFileTypeRegistrar*)1) )
						{
							if ( registrar->DeleteItem(base) != S_OK )
							{
								WritePrivateProfileStringW(L"winamp", L"remove_genplug", base, WINAMP_INI);
								WritePrivateProfileStringW(L"winamp", L"show_prefs", L"-1", WINAMP_INI);
								PostMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_RESTARTWINAMP);
							}
							else
								ListView_DeleteItem(listWindow, which_sel);
							registrar->Release();
						}
						// otherwise revert to a standard method
						else
						{
							WritePrivateProfileStringW(L"winamp", L"remove_genplug", base, WINAMP_INI);
							WritePrivateProfileStringW(L"winamp", L"show_prefs", L"-1", WINAMP_INI);
							PostMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_RESTARTWINAMP);
						}
					}

					// resets the focus to the listbox so it'll keep ui response working
					SetFocus(GetDlgItem(hwndDlg, IDC_GENLIB));
				}
			}
		}
		return FALSE;

		case IDC_PLUGINVERS:
			myOpenURLWithFallback(hwndDlg, L"http://www.google.com/search?q=Winamp+Library+Plugins", L"http://www.google.com/search?q=Winamp+Library+Plugins");
			return TRUE;
		}
	}

	link_handledraw(hwndDlg, uMsg, wParam, lParam);
	return 0;
}

typedef struct _PLUGINORDER
{
	LPCWSTR	name;
	bool	found;
} PLUGINORDER;

static PLUGINORDER preload[] =
{
 	{ L"ml_local.dll",       false },
	{ L"ml_downloads.dll",   false },
	{ L"ml_playlists.dll",   false },
	{ L"ml_wire.dll",        false },
	{ L"ml_online.dll",      false },
	{ L"ml_history.dll",     false },
	{ L"ml_rg.dll",          false },
	{ L"ml_bookmarks.dll",   false },
	{ L"ml_disc.dll",        false },
	{ L"ml_nowplaying2.dll", false },
	{ L"ml_devices.dll",     false },
	{ L"ml_pmp.dll",         false },
};

static HANDLE hProfile = INVALID_HANDLE_VALUE;
HANDLE GetProfileFileHandle(int mode)
{
	if ( profile & mode )
	{
		if ( hProfile == INVALID_HANDLE_VALUE )
		{
			wchar_t profileFile[MAX_PATH] = { 0 };
			PathCombineW(profileFile, WINAMP_INI_DIR, ((mode == 2) ? L"profile_load.txt" : L"profile.txt"));
			hProfile = CreateFileW(profileFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if ( hProfile != INVALID_HANDLE_VALUE )
			{
				// just to make sure we don't over-write things
				SetFilePointer(hProfile, NULL, NULL, FILE_END);
			}
		}
		return hProfile;
	}
	return INVALID_HANDLE_VALUE;
}

void LoadPlugin(const wchar_t* filename)
{
	wsprintfW( _log_message_w, L"filename( %s )", filename );

	LOG_DEBUG( _log_message_w );

	wchar_t profile[MAX_PATH * 2] = { 0 };
	LARGE_INTEGER starttime, endtime;
	if ( hProfile != INVALID_HANDLE_VALUE )
	{
		QueryPerformanceCounter(&starttime);
	}

	wchar_t file[MAX_PATH] = { 0 };
	PathCombineW(file, pluginPath, filename);

	if ( !wa::files::file_exists( file ) )
	{
		wsprintfW( _log_message_w, L"The plugin '%s' is not found in the \"Plugins\" folder!", filename );

		LOG_ERROR( _log_message_w );


		return;
	}


	HMODULE hLib = LoadLibraryW(file);
	if (hLib == NULL)
	{
		DWORD l_error_code = ::GetLastError();
		
		wsprintfW( _log_message_w, L"Error when loading the plugin '%s'! Error code : %d!", filename, l_error_code );		

		LOG_ERROR( _log_message_w );


		return;
	}

	winampMediaLibraryPlugin* (*pr)();
	pr = (winampMediaLibraryPlugin * (__cdecl*)(void)) GetProcAddress(hLib, "winampGetMediaLibraryPlugin");
	if (pr == NULL)
	{
		wsprintfW( _log_message_w, L"No entry point found for the plugin '%s'!", filename );

		LOG_ERROR( _log_message_w );


		FreeLibrary(hLib);
		return;
	}

	winampMediaLibraryPlugin* mlplugin = pr();
	if ( !mlplugin || (mlplugin->version > MLHDR_VER && mlplugin->version < MLHDR_VER_OLD) )
	{
		wsprintfW( _log_message_w, L"Either the plugin '%s' can't be loaded, or its version is incorrect!", filename );

		LOG_ERROR( _log_message_w );


		FreeLibrary(hLib);
		return;
	}

	wsprintfW( _log_message_w, L"The plugin '%s' is correctly loaded!", filename );

	LOG_DEBUG( _log_message_w );


	if ( g_safeMode != 1 )
	{
		if ( g_safeMode == 2 )
		{
			FreeModule(hLib);
			return;
		}

		char desc[128] = { 0 };
		lstrcpynA(desc, mlplugin->description, sizeof(desc));
		if ( desc[0] && !memcmp(desc, "nullsoft(", 9) )
		{
			char* p = strrchr(desc, ')');
			if ( p )
			{
				*p = 0;
				if ( stricmp(AutoChar(filename), (desc + 9)) )
				{
					FreeModule(hLib);
					return;
				}
			}
		}
		else
		{
			FreeModule(hLib);
			return;
		}
	}

	mlplugin->hwndLibraryParent = g_hwnd;
	mlplugin->hwndWinampParent  = plugin.hwndParent;
	mlplugin->hDllInstance      = hLib;

	int index = m_plugins.GetSize();


	if ( mlplugin->version >= MLHDR_VER )
	{
		mlplugin->service = WASABI_API_SVC;
	}

	if ( mlplugin->init() == ML_INIT_SUCCESS )
	{
		wsprintfW( _log_message_w, L"The plugin '%s' is initialized!", filename );

		LOG_DEBUG( _log_message_w );


		m_plugins.Add( (void *)mlplugin );
	}
	else
	{
		wsprintfW( _log_message_w, L"An error occurs when initializing the plugin '%s'!", filename );

		LOG_ERROR( _log_message_w );


		FreeLibrary( hLib );

		return;
	}

	if ( hProfile != INVALID_HANDLE_VALUE && mlplugin->hDllInstance )
	{
		QueryPerformanceCounter(&endtime);

		DWORD written = 0;
		unsigned int ms = (UINT)((endtime.QuadPart - starttime.QuadPart) * 1000 / freq.QuadPart);
		int len = swprintf(profile, L"Library\t%s\t[%s]\t%dms\r\n", file,
			(mlplugin->version >= MLHDR_VER_U ? (wchar_t*)mlplugin->description : AutoWide(mlplugin->description)), ms);
		SetFilePointer(hProfile, NULL, NULL, FILE_END);
		WriteFile(hProfile, profile, len * sizeof(wchar_t), &written, NULL);
	}
}

void loadMlPlugins()
{
	HANDLE hProfile = GetProfileFileHandle(2);
	WIN32_FIND_DATAW d = { 0 };
	wchar_t tofind[MAX_PATH] = { 0 };

	int i, count = ARRAYSIZE(preload);
	for ( i = 0; i < count; i++ )
		LoadPlugin(preload[i].name);

	PathCombineW(tofind, pluginPath, L"ML_*.DLL");

	HANDLE h = FindFirstFileW(tofind, &d);
	if ( h != INVALID_HANDLE_VALUE )
	{
		do
		{
			for ( i = 0; i < count && (preload[i].found || lstrcmpiW(preload[i].name, d.cFileName)); i++ );

			if ( i == count )
				LoadPlugin(d.cFileName);
			else
				preload[i].found = true;
		} while ( FindNextFileW(h, &d) );

		FindClose(h);
	}

	if ( hProfile != INVALID_HANDLE_VALUE )
	{
		DWORD written = 0;
		WriteFile(hProfile, L"\r\n", 2, &written, NULL);
		CloseHandle(hProfile);
		hProfile = INVALID_HANDLE_VALUE;
	}

	if ( !m_plugins.GetSize() )
		ShowWindow(GetDlgItem(g_hwnd, IDC_NO_VIEW), SW_SHOW);
}

void unloadMlPlugins()
{
	HANDLE hProfile = GetProfileFileHandle(1);
	if ( hProfile != INVALID_HANDLE_VALUE )
	{
		DWORD written = 0;
		WriteFile(hProfile, L"\r\n", 2, &written, NULL);
	}

	int i = m_plugins.GetSize();
	while ( i-- > 0 )  // reverse order to aid in not fucking up subclassing shit
	{
		winampMediaLibraryPlugin* mlplugin = (winampMediaLibraryPlugin*)m_plugins.Get(i);
		wchar_t profile[MAX_PATH * 2] = { 0 }, filename[MAX_PATH] = { 0 };
		LARGE_INTEGER starttime, endtime;
		if ( hProfile != INVALID_HANDLE_VALUE )
		{
			GetModuleFileNameW(mlplugin->hDllInstance, filename, MAX_PATH);
			QueryPerformanceCounter(&starttime);
		}

		if ( mlplugin->quit )
			mlplugin->quit();	// deals with 'virtual' ml items on restart

		if ( hProfile != INVALID_HANDLE_VALUE && mlplugin->hDllInstance )
		{
			QueryPerformanceCounter(&endtime);

			DWORD written = 0;
			unsigned int ms = (UINT)((endtime.QuadPart - starttime.QuadPart) * 1000 / freq.QuadPart);
			int len = swprintf(profile, L"Library\t%s\t[%s]\t%dms\r\n", filename,
				(mlplugin->version >= MLHDR_VER_U ? (wchar_t*)mlplugin->description : AutoWide(mlplugin->description)), ms);
			SetFilePointer(hProfile, NULL, NULL, FILE_END);
			WriteFile(hProfile, profile, len * sizeof(wchar_t), &written, NULL);
		}

		m_plugins.Del(i);
	}

	if ( hProfile != INVALID_HANDLE_VALUE )
	{
		DWORD written = 0;
		WriteFile(hProfile, L"\r\n", 2, &written, NULL);
		CloseHandle(hProfile);
		hProfile = INVALID_HANDLE_VALUE;
	}
}

INT_PTR plugin_SendMessage(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	for ( int i = 0; i < m_plugins.GetSize(); i++ )
	{
		winampMediaLibraryPlugin* mlplugin = (winampMediaLibraryPlugin*)m_plugins.Get(i);
		if ( mlplugin && mlplugin->MessageProc )
		{
			INT_PTR h = mlplugin->MessageProc(message_type, param1, param2, param3);
			if ( h )
				return h;
		}
	}
	return 0;
}
