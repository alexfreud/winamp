/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "resource.h"
#include "Options.h"
#include "gen.h"
#include <vector>
#include "main.hpp"
#include "../nu/AutoWide.h"

extern std::vector<winampGeneralPurposePlugin*> gen_plugins;
// gen tab procedure
static bool pluginsLoaded;
INT_PTR CALLBACK GenProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	hi helpinfo[] = {
		{IDC_GENLIB, IDS_P_GEN_LIB},
	    {IDC_GENCONF, IDS_P_GEN_CONF},
	};

	DO_HELP();

	if (uMsg == WM_INITDIALOG)
	{
		pluginsLoaded = false;

		WIN32_FIND_DATAW d = {0};
		link_startsubclass(hwndDlg, IDC_PLUGINVERS);
		HWND listWindow = GetDlgItem(hwndDlg, IDC_GENLIB);
		if (IsWindow(listWindow))
		{
			size_t x;
			RECT r = {0}, rc = {0};
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
			SendMessageW(listWindow, WM_SETFONT, SendMessageW(hwndDlg, WM_GETFONT, 0, 0), FALSE);

			LVCOLUMNW lvc = {0};
			ListView_InsertColumnW(listWindow, 0, &lvc);
			ListView_InsertColumnW(listWindow, 1, &lvc);

			for (x = 0; x != gen_plugins.size(); x ++)
			{
				// only try to add if the plugin is still there
				// (as 5.5+ allows for dynamic unloads if supported)
				if(gen_plugins[x])
				{
					wchar_t fn[MAX_PATH] = {0}, buf[512] = {0};
					GetModuleFileNameW(gen_plugins[x]->hDllInstance, fn, MAX_PATH);
					PathStripPathW(fn);

					LVITEMW lvi = {LVIF_TEXT | LVIF_PARAM, (int)x, 0};
					lstrcpyn(buf, (gen_plugins[x]->version == GPPHDR_VER_U ? (wchar_t*)gen_plugins[x]->description : AutoWide(gen_plugins[x]->description)), 512);
					lvi.pszText = buf;
					lvi.lParam = x;
					lvi.iItem = ListView_InsertItemW(listWindow, &lvi);

					lvi.mask = LVIF_TEXT;
					lvi.iSubItem = 1;
					lvi.pszText = fn;
					ListView_SetItemW(listWindow, &lvi);
				}
			}

			wchar_t dirstr[MAX_PATH] = {0};
			PathCombineW(dirstr, PLUGINDIR, L"GEN_*.DLL");
			HANDLE h = FindFirstFileW(dirstr, &d);
			if (h != INVALID_HANDLE_VALUE)
			{
				do
				{
					PathCombineW(dirstr, PLUGINDIR, d.cFileName);
					HMODULE b = LoadLibraryExW(dirstr, NULL, LOAD_LIBRARY_AS_DATAFILE);
					for (x = 0; b && (x != gen_plugins.size()); x ++)
					{
						if (gen_plugins[x]->hDllInstance == b)
						{
							break;
						}
					}

					if (x == gen_plugins.size() || !b)
					{
						LVITEMW lvi = {LVIF_TEXT | LVIF_PARAM, (int)x, 0};
						lvi.pszText = d.cFileName;
						lvi.lParam = -2;
						lvi.iItem = ListView_InsertItemW(listWindow, &lvi);

						lvi.mask = LVIF_TEXT;
						lvi.iSubItem = 1;
						lvi.pszText = getStringW(IDS_NOT_LOADED, NULL, 0);
						ListView_SetItemW(listWindow, &lvi);
					}
					FreeLibrary(b);
				}
				while (FindNextFileW(h, &d));
				FindClose(h);
			}

			GetClientRect(listWindow, &r);
			ListView_SetColumnWidth(listWindow, 1, LVSCW_AUTOSIZE);
			ListView_SetColumnWidth(listWindow, 0, (r.right - r.left) - ListView_GetColumnWidth(listWindow, 1));

			DirectMouseWheel_EnableConvertToMouseWheel(listWindow, TRUE);

			pluginsLoaded = true;
		}
	}
	else if (uMsg == WM_NOTIFY)
	{
		LPNMHDR p = (LPNMHDR)lParam;
		if (p->idFrom == IDC_GENLIB)
		{
			if (p->code == LVN_ITEMCHANGED)
			{
				LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
				LVITEM lvi = {LVIF_PARAM, pnmv->iItem};
				if (ListView_GetItem(p->hwndFrom, &lvi) && (pnmv->uNewState & LVIS_SELECTED))
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_GENCONF), (lvi.lParam != -2));
					EnableWindow(GetDlgItem(hwndDlg, IDC_UNINST), 1);
				}
				else
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_GENCONF), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_UNINST), 0);
				}
			}
			else if (p->code == NM_DBLCLK)
			{
				PostMessageW(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_GENCONF, 0), (LPARAM)GetDlgItem(hwndDlg, IDC_GENCONF));
			}
		}
		else if (p->code == HDN_ITEMCHANGINGW)
		{
			if (pluginsLoaded)
			{
#ifdef WIN64
				SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, TRUE);
#else
				SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
#endif
				return TRUE;
			}
		}
	}
	else if (uMsg == WM_DESTROY)
	{
		HWND listWindow = GetDlgItem(hwndDlg, IDC_GENLIB);
		if (IsWindow(listWindow))
			DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
	}
	else if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
			case IDC_GENCONF:
				{
					if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_GENCONF)))
					{
						HWND listWindow = GetDlgItem(hwndDlg, IDC_GENLIB);
						LVITEM lvi = {LVIF_PARAM, ListView_GetSelectionMark(listWindow)};
						if (ListView_GetItem(listWindow, &lvi))
						{
							if (lvi.lParam >= 0 && lvi.lParam < (int)gen_plugins.size())
							{
								if (!(config_no_visseh&4))
								{
									try {
										gen_plugins[lvi.lParam]->config();
									}
									catch(...)
									{
										LPMessageBox(hwndDlg, IDS_PLUGINERROR, IDS_ERROR, MB_OK | MB_ICONEXCLAMATION);
									}
								}
								else
								{
									gen_plugins[lvi.lParam]->config();
								}
							}
						}
					}
				}
				return FALSE;

			case IDC_UNINST:
				{
					if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_UNINST)))
					{
						HWND listWindow = GetDlgItem(hwndDlg, IDC_GENLIB);
						int which_sel = ListView_GetSelectionMark(listWindow);
						LVITEM lvi = {LVIF_PARAM, which_sel};
						if (ListView_GetItem(listWindow, &lvi) &&
							LPMessageBox(hwndDlg, IDS_P_PLUGIN_UNINSTALL,IDS_P_PLUGIN_UNINSTALL_CONFIRM, MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
						{
							if (lvi.lParam >= 0 && lvi.lParam < (int)gen_plugins.size())
							{
								int ret = GEN_PLUGIN_UNINSTALL_REBOOT;
								int (*pr)(HINSTANCE hDllInst, HWND hwndDlg, int param);

								if (lvi.lParam != -2)
								{
									*(void**)&pr = (void*)GetProcAddress(gen_plugins[lvi.lParam]->hDllInstance, "winampUninstallPlugin");
									if (pr) ret = pr(gen_plugins[lvi.lParam]->hDllInstance, hwndDlg, 0);
								}

								if (ret == GEN_PLUGIN_UNINSTALL_REBOOT)
								{
									extern void _w_s(char *name, char *data);
									wchar_t buf[MAX_PATH] = {0};
									GetModuleFileNameW(gen_plugins[lvi.lParam]->hDllInstance, buf, MAX_PATH);
									_w_sW("remove_genplug", buf);
									_w_i("show_prefs", 35);
									PostMessageW(hMainWindow, WM_USER, 0, IPC_RESTARTWINAMP);
								}
								// do dynamic unload if the plugin is able to support it (5.5+)
								else if (ret == GEN_PLUGIN_UNINSTALL_NOW)
								{
									wchar_t buf[MAX_PATH] = {0};
									GetModuleFileNameW(gen_plugins[lvi.lParam]->hDllInstance, buf, MAX_PATH);
									gen_plugins[lvi.lParam]->quit();
									FreeModule(gen_plugins[lvi.lParam]->hDllInstance);
									gen_plugins[lvi.lParam]=0;

									IFileTypeRegistrar *registrar=0;
									if (GetRegistrar(&registrar, true) == 0 && registrar)
									{
										if (registrar->DeleteItem(buf) != S_OK)
										{
											_w_sW("remove_genplug", buf);
											_w_i("show_prefs", 35);
											PostMessageW(hMainWindow, WM_USER, 0, IPC_RESTARTWINAMP);
										}
										registrar->Release();
									}

									ListView_DeleteItem(listWindow, which_sel);
								}
							}
							// will cope with not loaded plug-ins so we can still remove them, etc
							else if (lvi.lParam == -2)
							{
								wchar_t buf[1024] = {0}, base[1024] = {0};
								StringCchCopyW(base, 1024, PLUGINDIR);
								PathAddBackslashW(base);

								LVITEMW lvi = {LVIF_TEXT, which_sel};
								lvi.pszText = buf;
								lvi.cchTextMax = ARRAYSIZE(buf);
								ListView_GetItemW(listWindow, &lvi);

								wchar_t *p = wcschr(buf, L'.');
								if (p && *p == L'.')
								{
									p += 4;
									*p = 0;
									PathRemoveFileSpecW(base);
									PathAppendW(base, buf);
								}

								IFileTypeRegistrar *registrar=0;
								if (GetRegistrar(&registrar, true) == 0 && registrar)
								{
									if (registrar->DeleteItem(base) != S_OK)
									{
										_w_sW("remove_genplug", base);
										_w_i("show_prefs", 35);
										PostMessageW(hMainWindow, WM_USER, 0, IPC_RESTARTWINAMP);
									}
									else
										ListView_DeleteItem(listWindow, which_sel);
									registrar->Release();
								}
							}

							// resets the focus to the listbox so it'll keep ui response working
							SetFocus(GetDlgItem(hwndDlg, IDC_GENLIB));
						}
					}
				}
				return FALSE;

			case IDC_PLUGINVERS:
				myOpenURLWithFallback(hwndDlg, L"http://www.google.com/search?q=Winamp+General+Plugins", L"http://www.google.com/search?q=Winamp+General+Plugins");
				break;
		}
	}
	else
		link_handledraw(hwndDlg, uMsg, wParam, lParam);

	return FALSE;
} //gen