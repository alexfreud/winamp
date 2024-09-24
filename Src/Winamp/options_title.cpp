/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "main.h"
#include "resource.h"
#include "options.h"
#include "api.h"
#include <Wininet.h>
#include "language.h"

int atf_changed = 0, old_config_useexttitles = 0;
wchar_t old_config_titlefmt[1024] = {0};
INT_PTR CALLBACK TitleProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	extern char *g_titlefmt_default;
	hi helpinfo[]={
 		{IDC_USEID5,IDS_P_O_ONPLAY},
		{IDC_USEID4,IDS_P_O_ONLOAD},
		{IDC_USEID3,IDS_P_O_ONDEM},
		{IDC_CONVERT1,IDS_P_O_CONVERT1},
		{IDC_CONVERT2,IDS_P_O_CONVERT2},
	};
	DO_HELP();

	switch (uMsg)
	{
		case WM_INITDIALOG:
			atf_changed = 0;
			link_startsubclass(hwndDlg, IDC_TAGZ_HELP);
			lstrcpynW(old_config_titlefmt,config_titlefmt,1024);
			old_config_useexttitles = !!config_useexttitles;
			SetDlgItemTextW(hwndDlg,IDC_FORMAT,config_titlefmt);
			CheckDlgButton(hwndDlg,IDC_CHECK1,!!config_useexttitles);
			CheckDlgButton(hwndDlg,IDC_USEID3,config_riol == 0?1:0);
			CheckDlgButton(hwndDlg,IDC_USEID4,config_riol == 1?1:0);
			CheckDlgButton(hwndDlg,IDC_USEID6,(config_riol == 4)?1:0);
			CheckDlgButton(hwndDlg,IDC_USEID5,(config_riol != 0 && config_riol != 1 && config_riol != 4)?1:0);
			CheckDlgButton(hwndDlg,IDC_CONVERT1,(config_fixtitles&1)?1:0);
			CheckDlgButton(hwndDlg,IDC_CONVERT2,(config_fixtitles&2)?1:0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_TAGZ_DEFAULT),config_useexttitles);
			EnableWindow(GetDlgItem(hwndDlg,IDC_FORMAT),config_useexttitles);
			EnableWindow(GetDlgItem(hwndDlg,IDC_ATF_INFO),config_useexttitles);
		return 0;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_CHECK1:
					config_useexttitles=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK1);
					EnableWindow(GetDlgItem(hwndDlg,IDC_TAGZ_DEFAULT),config_useexttitles);
					EnableWindow(GetDlgItem(hwndDlg,IDC_FORMAT),config_useexttitles);
					EnableWindow(GetDlgItem(hwndDlg,IDC_ATF_INFO),config_useexttitles);
				break;

				case IDC_TAGZ_HELP:
				{
					wchar_t dirmask[MAX_PATH] = {0}, lang_code[4] = {0};
					lstrcpynW(lang_code, langManager->GetLanguageIdentifier(LANG_LANG_CODE), 3);

					// if we're not using a language pack or it reports en-US then use the default
					if(!config_langpack[0] || !lang_code[0] || !_wcsicmp(lang_code, L"en"))
					{
						GetTempPathW(MAX_PATH, dirmask);
						PathCombineW(dirmask, dirmask, L"atf.htm");
						if (!PathFileExistsW(dirmask))
						{
						save_local:
							DWORD size = 0, written = 0;
							HGLOBAL hResource = langManager->LoadResourceFromFile(hMainInstance, hMainInstance, RT_HTML,MAKEINTRESOURCE(IDR_ATF_HTML), &size);

							HANDLE hFile = CreateFileW(dirmask, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
							if (INVALID_HANDLE_VALUE != hFile)
							{
								WriteFile(hFile, hResource, size, &written, NULL);
								CloseHandle(hFile);
							}
						}
					}
					else
					{
						PathCombineW(dirmask, lang_directory, L"atf.htm");
						if (!PathFileExistsW(dirmask))
						{
							goto save_local;
						}
					}
					myOpenURL(hwndDlg, dirmask);
				}
				break;

				case IDC_TAGZ_DEFAULT:
					StringCchCopyW(config_titlefmt, sizeof(config_titlefmt)/sizeof(*config_titlefmt), L"[%artist% - ]$if2(%title%,$filepart(%filename%))");
					SetDlgItemTextW(hwndDlg,IDC_FORMAT,config_titlefmt);
				break;

				case IDC_FORMAT:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						GetDlgItemTextW(hwndDlg,IDC_FORMAT,config_titlefmt, sizeof(config_titlefmt)/sizeof(*config_titlefmt) - 1);
						config_titlefmt[sizeof(config_titlefmt)/sizeof(*config_titlefmt) - 1]=0;
						atf_changed = !!lstrcmpW(old_config_titlefmt,config_titlefmt);
					}
				break;

				case IDC_CONVERT1:
				case IDC_CONVERT2:
					config_fixtitles = (IsDlgButtonChecked(hwndDlg,IDC_CONVERT1)?1:0)|(IsDlgButtonChecked(hwndDlg,IDC_CONVERT2)?2:0);
				break;

				case IDC_USEID3:
				case IDC_USEID4:
				case IDC_USEID5:
				case IDC_USEID6:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						config_riol = IsDlgButtonChecked(hwndDlg,IDC_USEID4)?1:2;
						if (config_riol==2) {
							config_riol = IsDlgButtonChecked(hwndDlg,IDC_USEID3)?0:2;
						}
						if (config_riol==2) {
							config_riol = IsDlgButtonChecked(hwndDlg,IDC_USEID6)?4:2;
						}
					}
				break;
			}
		return 0;

		case WM_DESTROY:
			char titleStr[64] = {0};
			if(((atf_changed && !!config_useexttitles) || (old_config_useexttitles != config_useexttitles )) &&
				MessageBoxA(hwndDlg,getString(IDS_ATF_HAS_CHANGED,NULL,0),
						   getString(IDS_REFRESH_PLAYLIST_TITLES,titleStr,64),
						   MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				wchar_t ft[FILETITLE_SIZE] = {0};
				for (int x = 0; x < PlayList_getlength(); x ++)
				{
					if (!PlayList_gethidden(x) && !PlayList_hasanycurtain(x))
					{
						wchar_t fn[FILENAME_SIZE] = {0};
						PlayList_getitem(x, fn, ft);
						PlayList_setitem(x, fn, PlayList_gettitle(fn, 1));
						PlayList_setlastlen(x);
					}
				}
				InvalidateRect(hPLWindow, NULL, FALSE);
			}
		break;
	}

	link_handledraw(hwndDlg,uMsg,wParam,lParam);
	return 0;
}