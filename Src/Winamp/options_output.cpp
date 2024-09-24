/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename:
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "main.h"
#include "resource.h"
#include "Options.h"
#include "../nu/AutoWide.h"

// output tab procedure
static int last_sel = -1;
static bool pluginsLoaded;
INT_PTR CALLBACK OutputProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	hi helpinfo[]=
	{
		{IDC_OUTPUTS,IDS_P_OUT_OUTPUTS},
		{IDC_CONF2,IDS_P_OUT_CONF},
		{IDC_ABOUT2,IDS_P_OUT_ABOUT},
		{IDC_UNINSTOUT,IDS_P_OUT_UNINST},
	};

	DO_HELP();

	if (uMsg == WM_INITDIALOG)
	{
		pluginsLoaded = false;

		WIN32_FIND_DATAW d = {0};
		link_startsubclass(hwndDlg, IDC_PLUGINVERS);

		HWND listWindow = GetDlgItem(hwndDlg, IDC_OUTPUTS);
		if (IsWindow(listWindow))
		{
			size_t x;
			RECT r = {0};
			GetWindowRect(listWindow, &r);
			GetClientRect(listWindow, &r);
			MapWindowPoints(listWindow, hwndDlg, (LPPOINT)&r, 2);
			InflateRect(&r, 2, 2);
			DestroyWindow(listWindow);
			listWindow = CreateWindowExW(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
										 WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL |
										 LVS_SHOWSELALWAYS | LVS_SORTASCENDING | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER,
										 r.left, r.top, r.right - r.left, r.bottom - r.top,
										 hwndDlg, (HMENU)IDC_OUTPUTS, NULL, NULL);
			SetWindowPos(listWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
			ListView_SetExtendedListViewStyleEx(listWindow, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
			SendMessageW(listWindow, WM_SETFONT, SendMessageW(hwndDlg, WM_GETFONT, 0, 0), FALSE);

			LVCOLUMNW lvc = {0};
			ListView_InsertColumnW(listWindow, 0, &lvc);
			ListView_InsertColumnW(listWindow, 1, &lvc);

			for (x = 0; out_modules[x]; x ++)
			{
				wchar_t fn[MAX_PATH] = {0}, buf[512] = {0};
				GetModuleFileNameW(out_modules[x]->hDllInstance, fn, MAX_PATH);
				PathStripPathW(fn);

				LVITEMW lvi = {LVIF_TEXT | LVIF_PARAM, (int)x, 0};
				lstrcpynW(buf, ((out_modules[x]->version == OUT_VER_U) ? (wchar_t*)out_modules[x]->description : AutoWide(out_modules[x]->description)), 512);
				lvi.pszText = buf;
				lvi.lParam = x;
				lvi.iItem = ListView_InsertItemW(listWindow, &lvi);

				lvi.mask = LVIF_TEXT;
				lvi.iSubItem = 1;
				lvi.pszText = fn;
				ListView_SetItemW(listWindow, &lvi);

				if (!_stricmp(config_outname, (char*)out_modules[x]->id))
				{
					last_sel = lvi.iItem;
					ListView_SetItemState(listWindow, lvi.iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
			}

			wchar_t dirstr[MAX_PATH] = {0};
			PathCombineW(dirstr, PLUGINDIR, L"OUT*.DLL");
			HANDLE h = FindFirstFileW(dirstr, &d);
			if (h != INVALID_HANDLE_VALUE)
			{
				do
				{
					PathCombineW(dirstr, PLUGINDIR, d.cFileName);
					HMODULE b = LoadLibraryExW(dirstr, NULL, LOAD_LIBRARY_AS_DATAFILE);
					bool found = false;
					for (x = 0; b && out_modules[x]; x ++)
					{
						if (out_modules[x]->hDllInstance == b)
						{
							found = true;
							break;
						}
					}

					if (!found)
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
	else if (uMsg == WM_DESTROY)
	{
		HWND listWindow = GetDlgItem(hwndDlg, IDC_OUTPUTS);
		if (IsWindow(listWindow))
			DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
	}
	else if (uMsg == WM_NOTIFY)
	{
		LPNMHDR p = (LPNMHDR)lParam;
		if (p->idFrom == IDC_OUTPUTS)
		{
			if (p->code == LVN_ITEMCHANGED)
			{
				LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
				LVITEM lvi = {LVIF_PARAM, pnmv->iItem};
				if (ListView_GetItem(p->hwndFrom, &lvi) && (pnmv->uNewState & LVIS_SELECTED))
				{
					int loaded = (lvi.lParam != -2);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CONF2), loaded);
					EnableWindow(GetDlgItem(hwndDlg, IDC_ABOUT2), loaded);
					EnableWindow(GetDlgItem(hwndDlg, IDC_UNINSTOUT), 1);

					if (loaded && out_modules[lvi.lParam])
					{
						StringCbCopyA(config_outname, sizeof(config_outname), (char *)out_modules[lvi.lParam]->id);
						if (last_sel != -1 && out_modules[last_sel]) out_changed(out_modules[last_sel]->hDllInstance, OUT_UNSET);
						out_changed(out_modules[lvi.lParam]->hDllInstance, OUT_SET);
						PostMessageW(hMainWindow, WM_WA_IPC, (WPARAM)config_outname, IPC_CB_OUTPUTCHANGED);
					}
				}
				else
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_CONF2), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_ABOUT2), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_UNINSTOUT), 0);
				}
			}
			else if (p->code == NM_DBLCLK || p->code == NM_CLICK)
			{
				if (p->code == NM_DBLCLK)
				{
					PostMessageW(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_CONF2, 0), (LPARAM)GetDlgItem(hwndDlg, IDC_CONF2));
				}

				// helps to keep the selection on things...
				LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
				if (lpnmitem->iItem == -1)
				{
					ListView_SetItemState(p->hwndFrom, ListView_GetSelectionMark(p->hwndFrom), LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
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
	else if (uMsg == WM_COMMAND) 
	{
		switch (LOWORD(wParam))
		{
			case IDC_ABOUT2:
			{
				if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ABOUT2)))
				{
					HWND listWindow = GetDlgItem(hwndDlg, IDC_OUTPUTS);
					LVITEM lvi = {LVIF_PARAM, ListView_GetSelectionMark(listWindow)};
					if (ListView_GetItem(listWindow, &lvi))
					{
						if (lvi.lParam >= 0 && lvi.lParam < 32 && out_modules[lvi.lParam]) out_modules[lvi.lParam]->About(hwndDlg);
					}
				}
			}
			return FALSE;

			case IDC_CONF2:
			{
				if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CONF2)))
				{
					HWND listWindow = GetDlgItem(hwndDlg, IDC_OUTPUTS);
					LVITEM lvi = {LVIF_PARAM, ListView_GetSelectionMark(listWindow)};
					if (ListView_GetItem(listWindow, &lvi))
					{
						if (lvi.lParam >= 0 && lvi.lParam < 32 && out_modules[lvi.lParam]) out_modules[lvi.lParam]->Config(hwndDlg);
					}
				}
			}
			return FALSE;

			case IDC_UNINSTOUT:
			{
				HWND listWindow = GetDlgItem(hwndDlg, IDC_OUTPUTS);
				int which_sel = ListView_GetSelectionMark(listWindow);
				LVITEM lvi = {LVIF_PARAM, which_sel};
				if (ListView_GetItem(listWindow, &lvi))
				{
					if (LPMessageBox(hwndDlg, IDS_P_PLUGIN_UNINSTALL,IDS_P_PLUGIN_UNINSTALL_CONFIRM,MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
					{
						if (lvi.lParam >= 0 && lvi.lParam < 32 && out_modules[lvi.lParam])
						{
							int ret=0;
							int (*pr)(HINSTANCE hDllInst, HWND hwndDlg, int param);

							*(void**)&pr = (void*)GetProcAddress(out_modules[lvi.lParam]->hDllInstance,"winampUninstallPlugin");
							// changed 28/11/2010 so that even if non-zero is returned then the plug-in will uninstall
							if (pr)/*ret=*/pr(out_modules[lvi.lParam]->hDllInstance,hwndDlg,0);
							if (!ret)
							{
								char buf[1024] = {0};
								int w=-1;
								char *p=buf;
								int y;
								GetModuleFileNameA(out_modules[lvi.lParam]->hDllInstance,buf,sizeof(buf));
								_w_s("remove_genplug",buf);
								_w_i("show_prefs",32);

								buf[0]=0;

								for (y = 0; out_modules[y]; y ++)
								{
									if (lvi.lParam != y)
									{
										GetModuleFileNameA(out_modules[y]->hDllInstance,buf,sizeof(buf));
										p = PathFindFileNameA(buf);

										if (!_stricmp(p,"out_wave.dll") && w < 1) w=1;
										if (!_stricmp(p,"out_ds.dll") && w < 0) w=0;

										if (w>0) break;
									}
								}

								if (w==1) StringCbCopyA(config_outname, sizeof(config_outname), "out_wave.dll");
								else if (w==0) StringCbCopyA(config_outname,sizeof(config_outname), "out_ds.dll");
								else StringCbCopyA(config_outname,sizeof(config_outname),p);
								PostMessageW(hMainWindow,WM_USER,0,IPC_RESTARTWINAMP);
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
									_w_i("show_prefs", 32);
									PostMessageW(hMainWindow, WM_USER, 0, IPC_RESTARTWINAMP);
								}
								else
									ListView_DeleteItem(listWindow, which_sel);
								registrar->Release();
							}
						}

						// resets the focus to the listbox so it'll keep ui response working
						SetFocus(GetDlgItem(hwndDlg,IDC_INPUTS));
					}
				}
			}
			break;

			case IDC_PLUGINVERS:
				myOpenURLWithFallback(hwndDlg,L"http://www.google.com/search?q=Winamp+Output+Plugins", L"http://www.google.com/search?q=Winamp+Output+Plugins");
				break;
		}
	}
	else
	{
		link_handledraw(hwndDlg,uMsg,wParam,lParam);
	}

	return FALSE;
} //output