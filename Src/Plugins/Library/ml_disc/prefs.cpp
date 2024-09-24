#include "main.h"
#include "../nu/AutoWide.h"
#include "./resource.h"
#include "./settings.h"
#include "../Winamp/wa_ipc.h"
#include <strsafe.h>

static convertConfigStruct m_ccs;
static int m_has_seled;

static void myEnumProc(intptr_t user_data, const char *desc, int fourcc)
{
	HWND hwndDlg = (HWND) user_data;
	if (fourcc == OLD_AAC_CODEC)
		return ;

	int a = (INT)SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_ADDSTRING, 0, (LPARAM)(const wchar_t *)AutoWide(desc));

	SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_SETITEMDATA, (WPARAM)a, fourcc);

	if ( m_ccs.format == fourcc )
	{
		m_has_seled = 1;
		SendDlgItemMessage( hwndDlg, IDC_ENCFORMAT, CB_SETCURSEL, (WPARAM)a, 0 );
	}
}

static void doConfigResizeChild(HWND parent, HWND child)
{
	if (child)
	{
		RECT r;
		GetWindowRect(GetDlgItem(parent, IDC_ENC_CONFIG), &r);
		ScreenToClient(parent, (LPPOINT)&r);
		SetWindowPos(child, 0, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		ShowWindow(child, SW_SHOWNA);
	}
}

static HWND subWnd;

static void DisplayFormatExample(HWND hdlg, INT nItemId, BOOL bFile)
{
	BOOL bUpper;
	TCHAR szBuffer[MAX_PATH*2] = {0};
	TCHAR szFormat[MAX_PATH] = {0};

	Settings_ReadString(C_EXTRACT, (bFile) ? EF_TITLEFMT : EF_PLAYLISTFMT, szFormat, ARRAYSIZE(szFormat));
	
	WASABI_API_LNGSTRINGW_BUF(((bFile) ? IDS_EXAMPLE_RIPPED_FILE_FILENAME : IDS_EXAMPLE_PLAYLIST_FILENAME),
				szBuffer, ARRAYSIZE(szBuffer));

	FormatFileName(szBuffer, ARRAYSIZE(szBuffer), szFormat,
				  (bFile) ? 10 : 0xdeadbeef,
				  TEXT("U2"), TEXT("The Joshua Tree"),
				  (bFile) ? TEXT("Exit") : NULL,
				  TEXT("Rock"),
				  TEXT("1987"),
				  TEXT("U2"),
				  NULL,
				  TEXT(""));
	
	wchar_t szExtension[32] = {0};
	if (bFile)
	{		
		int c;
		Settings_GetInt(C_EXTRACT, EF_FOURCC, &c);
		if (c == OLD_AAC_CODEC) Settings_GetDefault(C_EXTRACT, EF_FOURCC, &c);
		GetExtensionString(szExtension, ARRAYSIZE(szExtension), c);
				
		Settings_GetBool(C_EXTRACT, EF_UPPEREXTENSION, &bUpper);
		if (bUpper) CharUpper(szExtension);
		else CharLower(szExtension);
	}
	else StringCchCopy(szExtension, ARRAYSIZE(szExtension), TEXT("m3u"));
	
	StringCchCat(szBuffer, ARRAYSIZE(szBuffer), TEXT("."));
	StringCchCat(szBuffer, ARRAYSIZE(szBuffer), szExtension);
	SetDlgItemText(hdlg, nItemId, szBuffer);
}

static INT_PTR CALLBACK CDPrefs1Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hActiveHelp = NULL;

	switch (uMsg)
	{
		case WM_INITDIALOG:
				SendDlgItemMessage(hwndDlg, IDC_DESTPATH, EM_SETLIMITTEXT, MAX_PATH, 0);
				Settings_SetCheckBox(C_EXTRACT, EF_UPPEREXTENSION, hwndDlg, IDC_UPPERCASEEXT);
				Settings_SetDirectoryCtrl(C_EXTRACT, EF_PATH, hwndDlg, IDC_DESTPATH);
				Settings_SetDlgItemText(C_EXTRACT, EF_TITLEFMT, hwndDlg, IDC_FILENAMEFMT);
				Settings_SetCheckBox(C_EXTRACT, EF_ADDMETADATA, hwndDlg, IDC_TAGFILES);
				Settings_SetCheckBox(C_EXTRACT, EF_CALCULATERG, hwndDlg, IDC_AUTO_RG);
				Settings_SetCheckBox(C_EXTRACT, EF_USETOTALTRACKS, hwndDlg, IDC_TOTAL_TRACKS);
				Settings_SetCheckBox(C_EXTRACT, EF_ADDTOMLDB, hwndDlg, IDC_CHECK_ML);
				Settings_SetDlgItemInt(C_EXTRACT, EF_TRACKOFFSET, hwndDlg, IDC_EDIT2);
				Settings_SetDlgItemText(C_EXTRACT, EF_COMMENTTEXT, hwndDlg, IDC_EDIT1);

			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_DESTPATH:
					if (HIWORD(wParam) == EN_CHANGE) Settings_FromDirectoryCtrl(C_EXTRACT, EF_PATH, hwndDlg,IDC_DESTPATH);  
					break;
				case IDC_UPPERCASEEXT:
					Settings_FromCheckBox(C_EXTRACT, EF_UPPEREXTENSION, hwndDlg, IDC_UPPERCASEEXT);
					DisplayFormatExample(hwndDlg, IDC_FMTOUT, TRUE);
					break;
				case IDC_FILENAMEFMT:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						Settings_FromDlgItemText(C_EXTRACT, EF_TITLEFMT, hwndDlg, IDC_FILENAMEFMT);
						DisplayFormatExample(hwndDlg, IDC_FMTOUT, TRUE);
					}
					break;
				case IDC_BUTTON1:
					Settings_BrowseForFolder(C_EXTRACT, EF_PATH, hwndDlg, IDC_DESTPATH);
					break;
				case IDC_BUTTON2:
					if (hActiveHelp) SetWindowPos(hActiveHelp, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
					else hActiveHelp = MLDisc_ShowHelp(hwndDlg, MAKEINTRESOURCE(IDS_RIPPED_FILENAME_FORMAT_HELP), 
								MAKEINTRESOURCE(IDS_RIPPED_FILENAME_FORMAT_CAPTION), MAKEINTRESOURCE(IDS_RIPPED_FILENAME_FORMAT), HF_ALLOWRESIZE);
					break;
				case IDC_EDIT2:			if (EN_CHANGE == HIWORD(wParam)) Settings_FromDlgItemText(C_EXTRACT, EF_TRACKOFFSET, hwndDlg, IDC_EDIT2);break;
				case IDC_EDIT1:			if (EN_CHANGE == HIWORD(wParam)) Settings_FromDlgItemText(C_EXTRACT, EF_COMMENTTEXT, hwndDlg, IDC_EDIT1); break;
				case IDC_AUTO_RG:		if (BN_CLICKED == HIWORD(wParam)) Settings_FromCheckBox(C_EXTRACT, EF_CALCULATERG, hwndDlg, IDC_AUTO_RG); break;
				case IDC_TOTAL_TRACKS:	if (BN_CLICKED == HIWORD(wParam)) Settings_FromCheckBox(C_EXTRACT, EF_USETOTALTRACKS, hwndDlg, IDC_TOTAL_TRACKS); break;
				case IDC_TAGFILES:		if (BN_CLICKED == HIWORD(wParam)) Settings_FromCheckBox(C_EXTRACT, EF_ADDMETADATA, hwndDlg, IDC_TAGFILES); break;
				case IDC_CHECK_ML:		if (BN_CLICKED == HIWORD(wParam)) Settings_FromCheckBox(C_EXTRACT, EF_ADDTOMLDB, hwndDlg, IDC_CHECK_ML); break;
				
			}
			break;
		case WM_DESTROY:
			if (hActiveHelp) DestroyWindow(hActiveHelp);
			break;
		case WM_PARENTNOTIFY:
			if (hActiveHelp && LOWORD(wParam) == WM_DESTROY && hActiveHelp  == (HWND)lParam) 
			hActiveHelp = NULL;
			break;
	}
	return 0;
}

int getRegVer();

static INT_PTR CALLBACK CDPrefs2Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			m_ccs.hwndParent = hwndDlg;
			Settings_GetInt(C_EXTRACT, EF_FOURCC, &m_ccs.format);
			if (m_ccs.format == OLD_AAC_CODEC) Settings_GetDefault(C_EXTRACT, EF_FOURCC, &m_ccs.format);

			converterEnumFmtStruct enumf = { myEnumProc, (INT)(INT_PTR)hwndDlg };
			m_has_seled = 0;
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&enumf, IPC_CONVERT_CONFIG_ENUMFMTS);
			if (!m_has_seled)
			{
				SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_SETCURSEL, 0, 0);
				m_ccs.format = mmioFOURCC('W', 'A', 'V', ' ');
			}

			HWND h = (HWND)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM) & m_ccs, IPC_CONVERT_CONFIG);
			doConfigResizeChild(hwndDlg, h);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ENCFORMAT:
			if (HIWORD(wParam) != CBN_SELCHANGE) return 0;
			{
				int sel = (INT)(INT_PTR)SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_GETCURSEL, 0, 0);
				if (sel != CB_ERR)
				{
					SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&m_ccs, IPC_CONVERT_CONFIG_END);
					int last = m_ccs.format;
					if (RegisteredEncoder(last) || last == OLD_AAC_CODEC) Settings_GetDefault(C_EXTRACT, EF_FOURCC, &last);

					m_ccs.format = (int)SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_GETITEMDATA, sel, 0);
					Settings_SetInt(C_EXTRACT, EF_FOURCC, m_ccs.format);

					HWND h = (HWND)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM) & m_ccs, IPC_CONVERT_CONFIG);
					doConfigResizeChild(hwndDlg, h);
				}
			}
			break;
		}
		break;
	case WM_DESTROY:
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&m_ccs, IPC_CONVERT_CONFIG_END);
		break;
	}
	return 0;
}


static INT_PTR CALLBACK CDPrefs4Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hActiveHelp = NULL;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			Settings_SetDlgItemText(C_EXTRACT, EF_PLAYLISTFMT, hwndDlg, IDC_FILENAMEFMT);
			Settings_SetCheckBox(C_EXTRACT, EF_CREATEM3U, hwndDlg, IDC_CHECK1);
			Settings_SetCheckBox(C_EXTRACT, EF_USEM3UEXT, hwndDlg, IDC_CHECK3);
			Settings_SetCheckBox(C_EXTRACT, EF_CREATEPLS, hwndDlg, IDC_CHECK2);
			Settings_SetCheckBox(C_EXTRACT, EF_CREATEMLPL, hwndDlg, IDC_CHECK4);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK3), IsDlgButtonChecked(hwndDlg, IDC_CHECK1));
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_FILENAMEFMT:
					if (LOWORD(wParam) != IDC_FILENAMEFMT || HIWORD(wParam) == EN_CHANGE)
					{
						Settings_FromDlgItemText(C_EXTRACT, EF_PLAYLISTFMT, hwndDlg, IDC_FILENAMEFMT);
						DisplayFormatExample(hwndDlg, IDC_FMTOUT, FALSE);
					}
					return 0;
				case IDC_BUTTON2:
					if (hActiveHelp) SetWindowPos(hActiveHelp, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
					else hActiveHelp = MLDisc_ShowHelp(hwndDlg, MAKEINTRESOURCE(IDS_RIPPPED_PLAYLIST_FORMAT_HELP), 
									MAKEINTRESOURCE(IDS_RIPPED_PLAYLIST_FORMAT_CAPTION), MAKEINTRESOURCE(IDS_RIPPED_PLAYLIST_FORMAT), HF_ALLOWRESIZE);
					break;
				case IDC_CHECK1:	
					if (BN_CLICKED == HIWORD(wParam)) Settings_FromCheckBox(C_EXTRACT, EF_CREATEM3U, hwndDlg, IDC_CHECK1);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK3), IsDlgButtonChecked(hwndDlg, IDC_CHECK1));
					break;
				
				case IDC_CHECK3:	if (BN_CLICKED == HIWORD(wParam)) Settings_FromCheckBox(C_EXTRACT, EF_USEM3UEXT, hwndDlg, IDC_CHECK3); break;
				case IDC_CHECK2:		if (BN_CLICKED == HIWORD(wParam)) Settings_FromCheckBox(C_EXTRACT, EF_CREATEPLS, hwndDlg, IDC_CHECK2); break;
				case IDC_CHECK4:	if (BN_CLICKED == HIWORD(wParam)) Settings_FromCheckBox(C_EXTRACT, EF_CREATEMLPL, hwndDlg, IDC_CHECK4); break;
			}
			break;

		case WM_DESTROY:
			if (hActiveHelp) DestroyWindow(hActiveHelp);
			break;
		case WM_PARENTNOTIFY:
			if (hActiveHelp && LOWORD(wParam) == WM_DESTROY && hActiveHelp  == (HWND)lParam) 
			hActiveHelp = NULL;
			break;
	}
	return 0;
}


static int has_extract;

static void _dosetsel(HWND hwndDlg)
{
	HWND tabwnd = GetDlgItem(hwndDlg, IDC_TAB1);
	int sel = TabCtrl_GetCurSel(tabwnd);

	if (sel >= 0 && (sel != g_config->ReadInt(L"lastcdprefp", 0) || !subWnd))
	{
		g_config->WriteInt(L"lastcdprefp", sel);
		if (subWnd) DestroyWindow(subWnd);
		subWnd = 0;

		UINT t = 0;
		DLGPROC p = NULL;
		if (!has_extract && sel) sel++;
		switch (sel)
		{
			case 2: t = IDD_PREFS_CDRIP1; p = CDPrefs1Proc; break;
			case 0: t = IDD_PREFS_CDRIP2; p = CDPrefs2Proc; break;
			case 3: t = IDD_PREFS_CDRIP4; p = CDPrefs4Proc; break;
			case 1:
			{
				t = 0;
				char buf2[512] = {0};
				char buf3[512] = {0};
				StringCchPrintfA(buf3, 512, "cdda_cf_%d", (INT)(INT_PTR)hwndDlg);
				getFileInfo("cda://", buf3, buf2, sizeof(buf2));
				subWnd = (HWND)(INT_PTR)atoi(buf2);
			}
			break;
			default: subWnd = 0; t = 0; break;
		}
		if (t) subWnd = WASABI_API_CREATEDIALOGW(t, hwndDlg, p);

		if (subWnd)
		{
			RECT r;
			GetClientRect(tabwnd, &r);
			TabCtrl_AdjustRect(tabwnd, FALSE, &r);
			SetWindowPos(subWnd, HWND_TOP, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOACTIVATE);
			ShowWindow(subWnd, SW_SHOWNA);
		}

		if(!SendMessage(plugin.hwndWinampParent,WM_WA_IPC,IPC_ISWINTHEMEPRESENT,IPC_USE_UXTHEME_FUNC))
		{
			SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)tabwnd,IPC_USE_UXTHEME_FUNC);
			SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)subWnd,IPC_USE_UXTHEME_FUNC);
		}
	}
}


BOOL CALLBACK CDRipPrefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			TCITEM item;
			HWND tabwnd = GetDlgItem(hwndDlg, IDC_TAB1);
			item.mask = TCIF_TEXT;
			item.pszText = WASABI_API_LNGSTRINGW(IDS_ENCODER);
			TabCtrl_InsertItem(tabwnd, 0, &item);

			wchar_t buf2[512] = {0};
			getFileInfoW(L"cda://", L"cdda_config_text", buf2, 512);

			if (buf2[0])
			{
				item.pszText = buf2;
				TabCtrl_InsertItem(tabwnd, 3, &item);
				has_extract = 1;
			}
			else has_extract = 0;

			item.pszText = WASABI_API_LNGSTRINGW(IDS_OUTPUT_FILE_SETTINGS);
			TabCtrl_InsertItem(tabwnd, 1 + has_extract, &item);
			item.pszText = WASABI_API_LNGSTRINGW(IDS_PLAYLIST_GENERATION);
			TabCtrl_InsertItem(tabwnd, 2 + has_extract, &item);

			TabCtrl_SetCurSel(tabwnd, g_config->ReadInt(L"lastcdprefp", 0));
			_dosetsel(hwndDlg);
		}
		return 0;
	case WM_NOTIFY:
		{
			LPNMHDR p = (LPNMHDR) lParam;
			if (p->idFrom == IDC_TAB1 && p->code == TCN_SELCHANGE) _dosetsel(hwndDlg);
		}
		return 0;
	case WM_DESTROY:
		subWnd = NULL;
		return 0;
	}
	return 0;
}

