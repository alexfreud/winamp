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
#include "vis.h"
#include "main.hpp"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"

static void VisUpdateSel(HWND hwndDlg, HWND listWindow, INT iItem)
{
	if (ListView_GetNextItem(listWindow, -1, LVIS_SELECTED) != -1)
	{
		wchar_t fn[FILENAME_SIZE] = {0}, b[1024] = {0}, namestr[MAX_PATH] = {0};
		ListView_GetItemTextW(listWindow, iItem, 1, fn, ARRAYSIZE(fn));

		if (wcscmp(config_visplugin_name, fn))
			config_visplugin_num = 0;

		StringCchCopyW(config_visplugin_name, MAX_PATH, fn);
		SendDlgItemMessageA(hwndDlg,IDC_VISMOD,CB_RESETCONTENT,0,0);
		PathCombineW(b, VISDIR, fn);

		HINSTANCE hLib = LoadLibraryW(b);
		if (hLib)
		{
			winampVisGetHeaderType pr = (winampVisGetHeaderType) GetProcAddress(hLib,"winampVisGetHeader");
			if (pr)
			{
				for(int i = 0;;)
				{
					winampVisModule *l_module = pr(hMainWindow)->getModule(i++);

					if (!l_module )
						break;

					SendDlgItemMessageA(hwndDlg, IDC_VISMOD, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)l_module->description);
				}

				ListView_DeleteItem(listWindow, iItem);

				StringCchCopyW(namestr, MAX_PATH, AutoWide(pr(hMainWindow)->description));
				LVITEMW lvi = {LVIF_TEXT, 0, 0};
				lvi.pszText = namestr;
				lvi.iItem = ListView_InsertItemW(listWindow, &lvi);

				lvi.mask = LVIF_TEXT;
				lvi.iSubItem = 1;
				lvi.pszText = fn;
				ListView_SetItemW(listWindow, &lvi);

				ListView_SetItemState(listWindow, lvi.iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			}
			FreeModule(hLib);
		}
	}

	int count = SendDlgItemMessageA(hwndDlg, IDC_VISMOD, CB_GETCOUNT, 0, 0);
	SendDlgItemMessageA(hwndDlg, IDC_VISMOD, CB_SETCURSEL, (config_visplugin_num < count ? config_visplugin_num : (config_visplugin_num = 0)), 0);
}

// vis tab procedure
static bool pluginsLoaded;
INT_PTR CALLBACK VisProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	hi helpinfo[]={
		{IDC_VISLIB,IDS_P_VIS_LIB},
		{IDC_VISMOD,IDS_P_VIS_MOD},
		{IDC_VISCONF,IDS_P_VIS_CONF},
		{IDC_VISSTOP,IDS_P_VIS_STOP},
		{IDC_VISTEST,IDS_P_VIS_START},
		{IDC_UNINSTVIS,IDS_P_VIS_UNINST},
	};

	DO_HELP();

	if (uMsg == WM_INITDIALOG)
	{
		pluginsLoaded = false;

		link_startsubclass(hwndDlg, IDC_PLUGINVERS);

		HWND listWindow = GetDlgItem(hwndDlg, IDC_VISLIB);
		if (IsWindow(listWindow))
		{
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
										 hwndDlg, (HMENU)IDC_VISLIB, NULL, NULL);
			SetWindowPos(listWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
			ListView_SetExtendedListViewStyleEx(listWindow, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
			SendMessageW(listWindow, WM_SETFONT, SendMessageW(hwndDlg, WM_GETFONT, 0, 0), FALSE);

			LVCOLUMNW lvc = {0};
			ListView_InsertColumnW(listWindow, 0, &lvc);
			ListView_InsertColumnW(listWindow, 1, &lvc);


			WIN32_FIND_DATAW d = {0};
			wchar_t dirstr[MAX_PATH+128] = {0};
			PathCombineW(dirstr,VISDIR,L"*.DLL");
			HANDLE h = FindFirstFileW(dirstr,&d);
			if (h != INVALID_HANDLE_VALUE) 
			{
				do
				{
					if (_wcsnicmp(d.cFileName,L"dsp_",4)
						&& _wcsnicmp(d.cFileName,L"in_",3) 
						&& _wcsnicmp(d.cFileName,L"gen_",4) 
						&& _wcsnicmp(d.cFileName,L"ml_",3) 
						&& _wcsnicmp(d.cFileName,L"enc_",4) 
						&& _wcsnicmp(d.cFileName,L"CDDB",4)
						&& _wcsnicmp(d.cFileName,L"out_",4)
						&& _wcsnicmp(d.cFileName, L"pmp_", 4))
					{
						wchar_t b[1024] = {0}, namestr[MAX_PATH] = {0};
						PathCombineW(b, VISDIR, d.cFileName);
						HINSTANCE hLib = LoadLibrary(b);
						if (hLib)
						{
							winampVisGetHeaderType pr = (winampVisGetHeaderType) GetProcAddress(hLib,"winampVisGetHeader");
							if (pr)
							{
								if (!g_safeMode)
								{
									winampVisHeader* pv = pr(hMainWindow);
									if (pv)
									{
										StringCchCopyW(namestr,MAX_PATH+256,AutoWide(pv->description));
									}
									else
									{
										StringCchCopyW(namestr,MAX_PATH+256,L"!");
									}
								}
							} 
							else
							{
								StringCchCopyW(namestr,MAX_PATH+256,L"!");
							}
							FreeModule(hLib);
						}
						else
						{
							StringCchCopyW(namestr,MAX_PATH+256,L"!");
						}

						if (wcscmp(namestr,L"!"))
						{
							LVITEMW lvi = {LVIF_TEXT, 0, 0};
							lvi.pszText = namestr;
							lvi.iItem = ListView_InsertItemW(listWindow, &lvi);

							lvi.mask = LVIF_TEXT;
							lvi.iSubItem = 1;
							lvi.pszText = d.cFileName;
							ListView_SetItemW(listWindow, &lvi);

							if (!_wcsicmp(d.cFileName, config_visplugin_name))
							{
								ListView_SetItemState(listWindow, lvi.iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
								VisUpdateSel(hwndDlg, listWindow, lvi.iItem);
							}
						}
					}								
				} while (FindNextFileW(h,&d));

				FindClose(h);
			}

			// [5.55+] only send this to correct the page state if we have no valid vis plugins available and loaded
			if(!ListView_GetItemCount(listWindow) ||
			   (ListView_GetNextItem(listWindow, -1, LVIS_SELECTED) == -1) ||
			   g_safeMode || !config_visplugin_name[0])
			{
				VisUpdateSel(hwndDlg, listWindow, -1);

				int enable = (ListView_GetSelectionMark(listWindow) != -1);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VISTEST),enable);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VISSTOP),enable);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VISCONF),enable);
				EnableWindow(GetDlgItem(hwndDlg,IDC_UNINSTVIS),enable);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VISMOD),enable);
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
		HWND listWindow = GetDlgItem(hwndDlg, IDC_VISLIB);
		if (IsWindow(listWindow))
			DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
	}
	else if (uMsg == WM_NOTIFY)
	{
		static int own_update;
		LPNMHDR p = (LPNMHDR)lParam;
		if (p->idFrom == IDC_VISLIB)
		{
			if (p->code == LVN_ITEMCHANGED && pluginsLoaded && !own_update)
			{
				if (!g_safeMode)
				{
					LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
					LVITEM lvi = {LVIF_PARAM, pnmv->iItem};
					if (ListView_GetItem(p->hwndFrom, &lvi) && (pnmv->uNewState & LVIS_SELECTED))
					{
						own_update = 1;
						VisUpdateSel(hwndDlg, p->hwndFrom, pnmv->iItem);
						own_update = 0;
					}
				}

				int enable = (ListView_GetSelectionMark(p->hwndFrom) != -1);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VISTEST),enable);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VISSTOP),enable);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VISCONF),enable);
				EnableWindow(GetDlgItem(hwndDlg,IDC_UNINSTVIS),enable);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VISMOD),enable);
			}
			else if (p->code == NM_DBLCLK || p->code == NM_CLICK)
			{
				// helps to keep the selection on things...
				LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
				if (lpnmitem->iItem == -1)
				{
					int which = ListView_GetNextItem(p->hwndFrom, -1, LVIS_SELECTED);
					if (which == -1)
					{
						for (int i = 0; i < ListView_GetItemCount(p->hwndFrom); i++)
						{
							wchar_t fn[MAX_PATH*2] = {0};
							ListView_GetItemTextW(p->hwndFrom, i, 1, fn, ARRAYSIZE(fn));

							if (!_wcsicmp(fn, config_visplugin_name))
							{
								ListView_SetItemState(p->hwndFrom, i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
								break;
							}
						}
					}
					else ListView_SetItemState(p->hwndFrom, which, LVIS_SELECTED, LVIS_SELECTED);
				}

				if (p->code == NM_DBLCLK)
				{
					PostMessageW(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_VISTEST, 0), (LPARAM)GetDlgItem(hwndDlg, IDC_VISTEST));
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
			case IDC_VISMOD:
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					config_visplugin_num = (unsigned char) SendDlgItemMessageA(hwndDlg,IDC_VISMOD,CB_GETCURSEL,0,0);

					if (config_visplugin_num == CB_ERR)
						config_visplugin_num = 0;
				}
				return FALSE;

			case IDC_VISCONF:
				{
					if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_VISCONF)))
					{
						HWND listWindow = GetDlgItem(hwndDlg, IDC_VISLIB);
						int which = ListView_GetNextItem(listWindow, -1, LVIS_SELECTED);
						if (which >= 0 && which < ListView_GetItemCount(listWindow))
						{
							wchar_t b[MAX_PATH] = {0}, fn[MAX_PATH*2] = {0};
							ListView_GetItemTextW(listWindow, which, 1, fn, ARRAYSIZE(fn));

							PathCombineW(b, VISDIR, fn);
							HINSTANCE hLib = LoadLibraryW(b);
							if (hLib)
							{
								winampVisGetHeaderType pr = (winampVisGetHeaderType) GetProcAddress(hLib,"winampVisGetHeader");
								winampVisModule *module = pr(hMainWindow)->getModule(SendDlgItemMessageA(hwndDlg,IDC_VISMOD,CB_GETCURSEL,0,0));
								if (module)
								{
									module->hDllInstance = hLib;
									module->hwndParent = hMainWindow;
									if (!(config_no_visseh&1))
									{
										try {
											module->Config(module);
										}
										catch(...)
										{
											LPMessageBox(hwndDlg, IDS_PLUGINERROR, IDS_ERROR, MB_OK|MB_ICONEXCLAMATION);
										}	
									}
									else
									{
										module->Config(module);
									}
								}
								else
								{
									LPMessageBox(hwndDlg, IDS_ERRORLOADINGPLUGIN, IDS_ERROR, MB_OK);
								}
								FreeLibrary(hLib);
							}
							else
							{
								LPMessageBox(hwndDlg, IDS_ERRORLOADINGPLUGIN, IDS_ERROR, MB_OK);
							}
						}
					}
				}
				return FALSE;

			case IDC_VISSTOP:
				{
					vis_stop();
				}
				return FALSE;

			case IDC_VISTEST:
				{
					if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_VISTEST)))
					{
						HWND listWindow = GetDlgItem(hwndDlg, IDC_VISLIB);
						int which = ListView_GetNextItem(listWindow, -1, LVIS_SELECTED);
						if (which >= 0 && which < ListView_GetItemCount(listWindow))
						{
							wchar_t fn[MAX_PATH] = {0};
							ListView_GetItemTextW(listWindow, which, 1, fn, ARRAYSIZE(fn));
							StringCchCopyW(config_visplugin_name, MAX_PATH, fn);
							config_visplugin_num = (unsigned char)SendDlgItemMessageA(hwndDlg, IDC_VISMOD, CB_GETCURSEL, 0, 0);
						}
					}

					vis_start(hMainWindow,NULL);
				}
				return FALSE;

			case IDC_UNINSTVIS:
				vis_stop();
				{
					if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_UNINSTVIS)))
					{
						HWND listWindow = GetDlgItem(hwndDlg, IDC_VISLIB);
						int which = ListView_GetNextItem(listWindow, -1, LVIS_SELECTED);
						wchar_t b[MAX_PATH] = {0}, fn[FILENAME_SIZE] = {0};
						ListView_GetItemTextW(listWindow, which, 1, fn, ARRAYSIZE(fn));
						PathCombineW(b, VISDIR, fn);

						HINSTANCE hLib = LoadLibraryW(b);
						if (hLib)
						{
							wchar_t buf[MAX_PATH] = {0};
							GetModuleFileNameW(hLib, buf, MAX_PATH);
							int ret=0;

							int (*pr)
								(HINSTANCE hDllInst, HWND hwndDlg, int param);

							*(void**)&pr = (void*)GetProcAddress(hLib,"winampUninstallPlugin");

							if (pr)
								ret=pr(hLib,hwndDlg,0);

							FreeLibrary(hLib);

							if (!ret)
							{
								IFileTypeRegistrar *registrar=0;
								if (GetRegistrar(&registrar, true) == 0 && registrar)
								{
									if (registrar->DeleteItem(buf) != S_OK)
									{
										_w_sW("remove_genplug", buf);
										_w_i("show_prefs", 33);
										PostMessageW(hMainWindow, WM_USER, 0, IPC_RESTARTWINAMP);
									}
									else
										ListView_DeleteItem(listWindow, which);

									registrar->Release();
								}

								SendDlgItemMessageA(hwndDlg,IDC_VISMOD,CB_RESETCONTENT,0,0);
							}
						}
					}
				}
				return FALSE;

			case IDC_PLUGINVERS:
				myOpenURLWithFallback(hwndDlg, L"http://www.google.com/search?q=Winamp+Visualization+Plugins", L"http://www.google.com/search?q=Winamp+Visualization+Plugins");
				return TRUE;
		}
	}

	link_handledraw(hwndDlg,uMsg,wParam,lParam);
	return FALSE;
} //vis