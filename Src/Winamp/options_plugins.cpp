/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "main.h"
#include "Options.h"
#include "resource.h"

static int CALLBACK WINAPI BrowseCallbackProc_VIS( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)VISDIR);

		// this is not nice but it fixes the selection not working correctly on all OSes
		EnumChildWindows(hwnd, browseEnumProc, 0);
	}
	if (uMsg == WM_CREATE) SetWindowTextW(hwnd,getStringW(IDS_SELVISDIR,NULL,0));
	return 0;
}

static int CALLBACK WINAPI BrowseCallbackProc_DSP( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)DSPDIR);

		// this is not nice but it fixes the selection not working correctly on all OSes
		EnumChildWindows(hwnd, browseEnumProc, 0);
	}
	if (uMsg == WM_CREATE) SetWindowTextW(hwnd,getStringW(IDS_SELDSPDIR,NULL,0));
	return 0;
}

void SetButtonText(HWND hwndDlg, int id, wchar_t* path)
{
	HWND control = GetDlgItem(hwndDlg, id);
	HDC hdc = GetDC(control);
	RECT r = {0};
	wchar_t temp[MAX_PATH] = {0};

	lstrcpynW(temp, path, MAX_PATH);
	SelectObject(hdc, (HFONT)SendMessageW(control, WM_GETFONT, 0, 0));
	GetClientRect(control, &r);
	r.left += 5;
	r.right -= 5;
	DrawTextW(hdc, temp, -1, &r, DT_PATH_ELLIPSIS | DT_WORD_ELLIPSIS | DT_MODIFYSTRING);
	SetWindowTextW(control, temp);
	ReleaseDC(control, hdc);
}


INT_PTR CALLBACK PlugProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	hi helpinfo[]={
		{IDC_VISDIR, IDS_P_PLUG_VISDIR},
		{IDC_DSPDIR, IDS_P_PLUG_DSPDIR},
		{IDC_VISPRIO, IDS_P_PLUG_PRIO},
		{IDC_AUTOEXEC, IDS_P_PLUG_AUTO},
		{IDC_DISVIS, IDS_P_PLUG_DIS},
		{IDC_SAFEMODE, IDS_P_PLUG_SAFEMODE},
		{IDC_SAFEMODEALWAYS, IDS_P_PLUG_SAFEMODEALWAYS},
		{IDC_CHECK1, IDS_P_PLUG_DISSEHVIS},
		{IDC_CHECK5, IDS_P_PLUG_DISSEHGEN},
		{IDC_CHECK6, IDS_P_PLUG_DISSEHDSP},
	};

	DO_HELP();

	if (uMsg == WM_INITDIALOG)
	{
		CheckDlgButton(hwndDlg, IDC_AUTOEXEC, config_visplugin_autoexec ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DISVIS, config_disvis ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK1, (config_no_visseh & 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK6, (config_no_visseh & 2) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK5, (config_no_visseh & 4) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SAFEMODEALWAYS, _r_i("safemode", 0) ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_VISPRIO, TBM_SETRANGE, TRUE, MAKELONG(0, 4));
		SendDlgItemMessage(hwndDlg, IDC_VISPRIO, TBM_SETPOS, TRUE, config_visplugin_priority);
		SetDlgItemTextW(hwndDlg, IDC_VISDIR, VISDIR);
		SetButtonText(hwndDlg, IDC_VISDIR, VISDIR);
		SetDlgItemTextW(hwndDlg, IDC_DSPDIR, DSPDIR);
		SetButtonText(hwndDlg, IDC_DSPDIR, DSPDIR);

		if (g_safeMode)
		{
			SetDlgItemTextW(hwndDlg, IDC_SAFEMODE, getStringW(IDS_RESTART_NORMAL, NULL, 0));
		}
	}

	if (uMsg == WM_HSCROLL)
	{
		HWND swnd = (HWND)lParam;
		if (swnd == GetDlgItem(hwndDlg, IDC_VISPRIO))
		{
			config_visplugin_priority = (unsigned char) SendDlgItemMessage(hwndDlg, IDC_VISPRIO, TBM_GETPOS, 0, 0);
			vis_setprio();
		}
	}

	if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
			case IDC_CHECK1:
			case IDC_CHECK5:
			case IDC_CHECK6:
				config_no_visseh = 
					(IsDlgButtonChecked(hwndDlg, IDC_CHECK1) ? 1 : 0) |
					(IsDlgButtonChecked(hwndDlg, IDC_CHECK6) ? 2 : 0) |
					(IsDlgButtonChecked(hwndDlg, IDC_CHECK5) ? 4 : 0);
				break;

			case IDC_AUTOEXEC:
				config_visplugin_autoexec = IsDlgButtonChecked(hwndDlg, IDC_AUTOEXEC) ? 1 : 0;
				break;

			case IDC_DISVIS:
				config_disvis = IsDlgButtonChecked(hwndDlg, IDC_DISVIS) ? 1 : 0;
				break;

			case IDC_VISDIR:
				{
					BROWSEINFOW bi = {0};
					wchar_t name[MAX_PATH] = {0};
					bi.hwndOwner = hMainWindow;
					bi.pszDisplayName = name;
					bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
					bi.lpfn = BrowseCallbackProc_VIS;
					ITEMIDLIST *idlist = SHBrowseForFolderW(&bi);
					if (idlist)
					{
						wchar_t path[MAX_PATH] = {0};
						SHGetPathFromIDListW(idlist, path);
						Shell_Free(idlist);
						lstrcpynW(VISDIR, path, MAX_PATH);
						SetDlgItemTextW(hwndDlg, IDC_VISDIR,VISDIR);
						_w_sW("VISDir", VISDIR);
						SetButtonText(hwndDlg, IDC_VISDIR, VISDIR);
					}
				}
				return FALSE;

			case IDC_DSPDIR:
				{
					BROWSEINFOW bi = {0};
					wchar_t name[MAX_PATH] = {0};
					bi.hwndOwner = hMainWindow;
					bi.pszDisplayName = name;
					bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
					bi.lpfn = BrowseCallbackProc_DSP;
					ITEMIDLIST *idlist = SHBrowseForFolderW(&bi);
					if (idlist)
					{
						wchar_t path[MAX_PATH] = {0};
						SHGetPathFromIDListW(idlist, path);
						Shell_Free(idlist);
						lstrcpynW(DSPDIR, path, MAX_PATH);
						SetDlgItemTextW(hwndDlg, IDC_DSPDIR, DSPDIR);
						_w_sW("DSPDir", DSPDIR);
						SetButtonText(hwndDlg, IDC_DSPDIR, DSPDIR);
					}
				}
				return FALSE;

			case IDC_SAFEMODE:
				if (LPMessageBox(hwndDlg, IDS_RESTART_SAFE, IDS_RESTART, MB_YESNO | MB_ICONQUESTION | MB_TOPMOST) == IDYES)
				{
					_w_i("show_prefs", 30);
					PostMessageW(hMainWindow, WM_USER, 0, (!g_safeMode ? IPC_RESTARTSAFEWINAMP : IPC_RESTARTWINAMP));
				}
				return FALSE;

			case IDC_SAFEMODEALWAYS:
			{
				int mode = (IsDlgButtonChecked(hwndDlg, IDC_SAFEMODEALWAYS) ? 1 : 0);
				_w_i("safemode", mode);
				if (mode != !!g_safeMode)
				{
					if (LPMessageBox(hwndDlg, IDS_RESTART_SAFE, IDS_RESTART, MB_YESNO | MB_ICONQUESTION | MB_TOPMOST) == IDYES)
					{
						_w_i("show_prefs", 30);
						PostMessageW(hMainWindow, WM_USER, 0, (!g_safeMode ? IPC_RESTARTSAFEWINAMP : IPC_RESTARTWINAMP));
					}
				}
				return FALSE;
			}
		}
	}

	const int controls[] = 
	{
		IDC_VISPRIO,
	};
	if (FALSE != DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	return FALSE;
} //audio