/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "main.h"
#include "language.h"
#include "resource.h"
#include "Options.h"
#include "main.hpp"

static WNDPROC list_oldWndProc;
static wchar_t rename_lang[MAX_PATH];
static int cur_lang = LB_ERR;

static BOOL FillEnumRec(ENUMLANG *pel, LPCWSTR pszFileName, BOOL bDirectory, LPWSTR pszName, INT cchName, LPCWSTR pszActiveFile)
{
	//if (bDirectory) return FALSE; 

    LPCWSTR pExt = 0;
	if (!bDirectory)
	{
		pExt = PathFindExtensionW(pszFileName);
		if (L'.' != *pExt) return FALSE;
		pExt++;
	}

	DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pExt, -1, L"wlz", -1)) pel->nType = LANG_FILETYPE_WLZ;
	else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pExt, -1, L"zip", -1)) pel->nType = LANG_FILETYPE_ZIP;
	else
	{
		wchar_t check[MAX_PATH] = {0};
		PathCombineW(check, LANGDIR, pszFileName);
		PathCombineW(check, check, L"winamp.lng");
		if (!PathFileExistsW(check)) return FALSE;
		pel->nType = LANG_FILETYPE_DIR;
	}

	if (!bDirectory)
	{
		LPCWSTR pszFile = PathFindFileNameW(pszFileName);
		StringCchCopyNW(pszName, cchName, pszFile, (size_t)(pExt - pszFile - 1));
	}
	else
		StringCchCopyW(pszName, cchName, pszFileName);

	pel->pszFileName = pszFileName;
	pel->pszName = pszName;
	pel->bActive = (pszActiveFile && CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pszActiveFile, -1, pel->pszFileName, -1));

	return TRUE;
}

int EnumerateLanguages(ENUMLANGPROC fnEnumLang, void *user)
{
	if (!fnEnumLang) return FALSE;

	WIN32_FIND_DATAW d = {0};
	wchar_t dirmask[1024] = {0}, szName[MAX_PATH] = {0}, *pszActive = 0;
	ENUMLANG el = {0};
	BOOL bActiveFound = FALSE, bTerminated = FALSE;

	if (*config_langpack)
	{
		pszActive = PathFindFileNameW(config_langpack);
		if (pszActive != config_langpack && BuildFullPath(LANGDIR, config_langpack, szName, sizeof(szName)/sizeof(wchar_t))) 
		{
			INT cr = ComparePath(szName, pszActive, LANGDIR);
			if(cr && CSTR_EQUAL != cr && PathFileExistsW(szName))
			{
				if (FillEnumRec(&el, config_langpack, FALSE, szName, sizeof(szName)/sizeof(wchar_t), NULL))
				{
					el.bActive = TRUE;
					bActiveFound = TRUE;
					if (!fnEnumLang(&el, user)) return FALSE;
				}
			}
		}
	}
	else pszActive = NULL;

	PathCombineW(dirmask, LANGDIR, L"*");
	HANDLE h = FindFirstFileW(dirmask, &d);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!wcscmp(d.cFileName, L".") || !wcscmp(d.cFileName, L"..")) continue;

			if (FillEnumRec(&el, d.cFileName, (FILE_ATTRIBUTE_DIRECTORY & d.dwFileAttributes), szName, sizeof(szName)/sizeof(wchar_t), (bActiveFound) ? NULL : pszActive))
			{
				if (el.bActive) bActiveFound = TRUE;
				if (!fnEnumLang(&el, user)) 
				{
					bTerminated = TRUE;
					break;
				}
			}
		}
		while (FindNextFileW(h, &d));
		FindClose(h);
	}

	if (!bTerminated)
	{
		el.pszFileName = NULL;
		el.pszName = L"English (US)";
		el.nType = LANG_FILETYPE_EMBED;
		el.bActive = !bActiveFound;
		bTerminated = !fnEnumLang(&el, user);
	}
	return !bTerminated;
}

static int CALLBACK BrowseLangCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		{
			SetWindowTextW(hwnd, getStringW(IDS_P_SELECT_LANGDIR,NULL,0));
			SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)LANGDIR);
		}
		return 0;
	}
	return 0;
}

static void _setreadme(HWND hwndDlg, wchar_t* readme_only_wlz_extraction)
{
	if (config_langpack[0] || readme_only_wlz_extraction && readme_only_wlz_extraction[0])
	{
		wchar_t s[MAX_PATH] = {0}, *dirpath = (readme_only_wlz_extraction?readme_only_wlz_extraction:lang_directory);
		FILE *fp = NULL;
		PathCombineW(s, dirpath, L"readme.txt");
		fp=_wfopen(s,L"rt");
		if (!fp)
		{
			PathCombineW(s, dirpath, L"read me.txt");
			fp=_wfopen(s,L"rt");
		}
		if (!fp)
		{
			WIN32_FIND_DATAW d = {0};
			PathCombineW(s, dirpath, L"*.txt");

			HANDLE h = FindFirstFileW(s,&d);
			s[0]=0;
			if (h != INVALID_HANDLE_VALUE) 
			{
				do 
				{
					if (NULL != PathCombineW(s, dirpath, d.cFileName))
						break;
      			} while (FindNextFileW(h,&d));

				FindClose(h);
				fp=_wfopen(s,L"rt");
			}
		}

		if (fp)
		{
			SetDialogBoxFromFile(fp, hwndDlg, IDC_EDIT1);
			fclose(fp);
		}
		else
			SetDlgItemTextW(hwndDlg,IDC_EDIT1,getStringW(IDS_P_SKIN_NO_INFO_FOUND,NULL,0));
	}
	else SetDlgItemTextA(hwndDlg,IDC_EDIT1,"\tWinamp Default Language");
}

static BOOL CALLBACK renameSkinProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SetDlgItemTextW(hwndDlg,IDC_OLD,rename_lang);
			SetDlgItemTextW(hwndDlg,IDC_NEW,rename_lang);
			SetWindowTextW(hwndDlg,getStringW(IDS_RENAME_WLZ, NULL, 0));
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					GetDlgItemTextW(hwndDlg,IDC_NEW,rename_lang,sizeof(rename_lang)/sizeof(*rename_lang));
					EndDialog(hwndDlg,!!rename_lang[0]);
					return 0;

				case IDCANCEL:
					EndDialog(hwndDlg,0);
					return 0;
			}
			return 0;
	}
	return 0;
}

static DWORD WINAPI list_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if(uMsg == WM_RBUTTONUP)
	{
		PostMessageW(GetParent(hwndDlg),WM_USER+0x123,0,0);
	}
	return CallWindowProcW(list_oldWndProc,hwndDlg,uMsg,wParam,lParam);
}

static void LangSwitchDelayText(HWND hwndDlg){
	if(config_langpack2[0]){
		wchar_t tmp[MAX_PATH] = {0};
		StringCchPrintfW(tmp,MAX_PATH,getStringW(IDS_WA_RESTART_FOR_WLZ_NEEDED,NULL,0),
						 (config_langpack2[0]=='<')?L"Winamp Default Language":config_langpack2);
		SetDlgItemTextW(hwndDlg,IDC_LANG_RESTART_TEXT,tmp);
	}
	ShowWindow(GetDlgItem(hwndDlg,IDC_LANG_RESTART_TEXT),config_langpack2[0]?SW_SHOW:SW_HIDE);
}

void LangSwitchToLangPrompt(HWND hwndDlg, wchar_t* newLang)
{
	wchar_t title[64] = {0};
	// will do an instant switch to the selected language pack
	if (!(GetAsyncKeyState(VK_SHIFT)&0x8000) && MessageBoxW(hwndDlg,getStringW(IDS_LANGCHANGE,NULL,0),
				   getStringW(IDS_LANGCHANGE_TITLE,title,64),MB_ICONEXCLAMATION|MB_OKCANCEL) == IDOK ||
				(GetAsyncKeyState(VK_SHIFT)&0x8000))
	{
		config_langpack2[0] = 0;
		lstrcpynW(config_langpack,newLang,MAX_PATH);
		config_save_langpack_var();
		_w_i("show_prefs", 25);
		PostMessageW(hMainWindow,WM_USER,0,IPC_RESTARTWINAMP);
	}
	// will switch to the selected language pack on the next restart (as per the options description)
	// need to indicate which one is pending to be chosen (if there is one) on the dialog
	// otherwise it might cause confusion with showing one selected when it otherwise shouldn't be
	else
	{
		// fool the exit code to think that we've got a change to save out
		if(!newLang[0]) newLang[0] = '<';
		lstrcpynW(config_langpack2,newLang,MAX_PATH);
	}
}

static void LangSwitchToLang(HWND hwndDlg, int id)
{
	wchar_t buf2[1024] = {0}, buf[1024] = {0}, *o = buf, *p = config_langpack;
	int ld = (id!=-1?id:SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETCURSEL,0,0));
	if (ld == LB_ERR) return;
	int itemdata = SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETITEMDATA,ld,0);
	SendDlgItemMessageW(hwndDlg,IDC_SELBOX,LB_GETTEXT,ld,(LPARAM)buf2);

	if (config_langpack[0])
	{
		if (_wcsicmp(p, L"winamp.exe"))
		{
			while (p && *p != L'.' && *p && o-buf < 512)
			{
				wchar_t *t=CharNextW(p);
				memcpy(o,p,t-p);
				o+=t-p;
				p=t;
			}
			*o=0;
		}
	}

	if (!_wcsicmp(buf,buf2)) return; // no change

	if (buf2[0])
	{
		if (itemdata == 2)
		{
			StringCbCatW(buf2, sizeof(buf2), L".wlz");
		}
		else if (itemdata == 4)
		{
			StringCbCatW(buf2, sizeof(buf2), L".zip");
		}
	}

	LangSwitchToLangPrompt(hwndDlg, buf2);
	LangSwitchDelayText(hwndDlg);
}

static void LangRenameLang(HWND hwndDlg, int id)
{
	int x = (id!=-1?id:SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETCURSEL,0,0));
	int extidx = SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETITEMDATA,x,0);
	int is_cur_lp = 0;
	wchar_t test_lng[MAX_PATH] = {0};

	SendDlgItemMessageW(hwndDlg,IDC_SELBOX,LB_GETTEXT,x,(LPARAM)rename_lang);
	SendDlgItemMessageW(hwndDlg,IDC_SELBOX,LB_GETTEXT,x,(LPARAM)test_lng);

	switch (extidx)
	{
		case 2:
		{
			StringCchCatW(test_lng, MAX_PATH, L".wlz");
			break;
		}
		case 4:
		{
			StringCchCatW(test_lng, MAX_PATH, L".zip");
			break;
		}
		default: break;
	}

	// make sure if renaming the current lang pack that it's followed back through
	if (!_wcsicmp(test_lng,config_langpack))
	{
		is_cur_lp = 1;
	}

	if (rename_lang[0] && _wcsicmp(rename_lang, L"English (US)"))
	{
		wchar_t oldlang[MAX_PATH] = {0};
		lstrcpynW(oldlang, rename_lang, MAX_PATH);
    
		if (LPDialogBoxW(IDD_RENAMESKIN,hwndDlg,renameSkinProc) && wcscmp(oldlang,rename_lang))
		{
			wchar_t oldname[MAX_PATH] = {0}, newname[MAX_PATH] = {0};
			PathCombineW(oldname, LANGDIR, oldlang);
			PathCombineW(newname, LANGDIR, rename_lang);
			
			switch (extidx)
			{
				case 2: StringCchCatW(oldname,MAX_PATH, L".wlz"); StringCchCatW(newname,MAX_PATH,L".wlz"); break;
				case 4: StringCchCatW(oldname,MAX_PATH, L".zip"); StringCchCatW(newname,MAX_PATH,L".zip"); break;
				default: break;
			}

			if (MoveFileW(oldname,newname))
			{
				SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_DELETESTRING,x,0);
				SendDlgItemMessageW(hwndDlg,IDC_SELBOX,LB_INSERTSTRING,x,(LPARAM)rename_lang);

				int par=0;
				SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_SETITEMDATA,x,par);
				SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_SETCURSEL,x,0);

				// only resave if the rename worked
				if(is_cur_lp)
				{
					wchar_t *p = scanstr_backW(newname,L"\\",0)+1;
					lstrcpynW(config_langpack, p, MAX_PATH);
					config_save_langpack_var();
				}
			}
			else
			{
				wchar_t title[64] = {0};
				MessageBoxW(hwndDlg,getStringW(IDS_P_LANG_ERR_RENAME,NULL,0),
							getStringW(IDS_P_LANG_ERR_RENAME_TITLE,title,64),MB_OK);
			}
		}
	}
}

static void LangDeleteLang(HWND hwndDlg, int id)
{
	wchar_t lang[MAX_PATH] = {0};
	int cur = SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETCURSEL,0,0);
	int x = (id != -1 ? id : cur);
	if (x == LB_ERR) return;
	SendDlgItemMessageW(hwndDlg,IDC_SELBOX,LB_GETTEXT,x,(LPARAM)lang);

	switch (SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETITEMDATA,x,0))
	{
		case 2:
		{
			StringCchCatW(lang, MAX_PATH, L".wlz");
			break;
		}
		case 4:
		{
			StringCchCatW(lang, MAX_PATH, L".zip");
			break;
		}
		default: break;
	}

	if (lang[0] && _wcsicmp(lang, L"English (US)"))
	{
		wchar_t buf[2048] = {0};
		StringCchPrintfW(buf, 2048, getStringW(IDS_P_LANG_PACK_DELETEWLZ_PROMPT,NULL,0),lang);          

		if (MessageBoxW(hwndDlg,buf,getStringW(IDS_P_LANG_PACK_DELETEWLZ,NULL,0),MB_YESNO|MB_ICONQUESTION) == IDYES)
		{
			PathCombineW(buf, LANGDIR, lang);
			IFileTypeRegistrar *registrar=0;
			if (GetRegistrar(&registrar, true) == 0 && registrar)
			{
				registrar->DeleteItem(buf);
				registrar->Release();
			}

			if(id == cur_lang)
				SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_SETCURSEL,0,0);

			SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_DELETESTRING,x,0);

			if(id == cur_lang)
			{
				cur_lang = 0;
				config_langpack[0]=0;
				config_save_langpack_var();
				wchar_t title[64] = {0};
				if (MessageBoxW(hwndDlg,getStringW(IDS_LANGCHANGE,NULL,0),
							getStringW(IDS_LANGCHANGE_TITLE,title,64),MB_ICONEXCLAMATION|MB_OKCANCEL) == IDOK)
					_w_i("show_prefs", 25);
					PostMessageW(hMainWindow,WM_USER,0,IPC_RESTARTWINAMP);
			}
		}
	}
}

static BOOL CALLBACK AddLangToListBox(ENUMLANG *pel, void *user)
{
	int index = SendMessageW((HWND)user, LB_ADDSTRING, 0, (LPARAM)pel->pszName);
	if (LB_ERR != index)
	{
		SendMessageW((HWND)user, LB_SETITEMDATA, index, (LPARAM)pel->nType);
		if (pel->bActive) SendMessageW((HWND)user, LB_SETCURSEL, index, 0); 
	}
	return TRUE;
}

INT_PTR CALLBACK LangProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{	
	hi helpinfo[]={{IDC_SELBOX,IDS_P_SETUP_LANG},
				   {IDC_CHDIR,IDS_P_LNG_CHDR}};
	DO_HELP();

	switch (uMsg)
	{
		case WM_INITDIALOG:
			link_startsubclass(hwndDlg, IDC_WINAMPLINK);
		case WM_USER+50:
		{
			HWND hw = GetDlgItem(hwndDlg,IDC_SELBOX);
			SetDlgItemTextW(hwndDlg,IDC_LANG_ID_STR,langManager->GetLanguageIdentifier(LANG_IDENT_STR));

			if(uMsg == WM_INITDIALOG)
			{
				int tabs[] = {150};
				SendMessageW(hw,LB_SETTABSTOPS,1,(LPARAM)tabs);
				SetDlgItemTextW(hwndDlg, IDC_CHDIR, LANGDIR);
				list_oldWndProc=(WNDPROC)SetWindowLongPtrW(hw,GWLP_WNDPROC,(LONG_PTR)list_newWndProc);
				DirectMouseWheel_EnableConvertToMouseWheel(hw, TRUE);
				CheckDlgButton(hwndDlg, IDC_SHOW_LNG_PACK, config_wlz_menu);
				CheckDlgButton(hwndDlg, IDC_LANG_INSTALL_PROMPT, config_wlz_prompt);
			}

			wchar_t selected[MAX_PATH] = {0};

			SendMessageW(hw,WM_SETREDRAW,FALSE,0);

			EnumerateLanguages(AddLangToListBox, hw);
			int index = (INT)SendMessageW(hw,LB_GETCURSEL,0,0);
			if (LB_ERR == index || LB_ERR == SendMessageW(hw, LB_GETTEXT, index, (LPARAM)selected))
				selected[0] = 0x00;
							
			index = (INT)SendMessageW(hw, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)L"English (US)");
			if (LB_ERR != index) 
			{
				SendMessageW(hw, LB_DELETESTRING, index, 0);
				SendMessageW(hw, LB_INSERTSTRING, 0, (LPARAM)L"English (US)");
			}
						
			index =  (*selected) ? (INT)SendMessageW(hw, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)selected) : LB_ERR;
			if (LB_ERR == index) index = 0;
			SendMessageW(hw,LB_SETCURSEL, index, 0);
			cur_lang = index;
			
			// set buttons to the correct state on init
			SendMessageW(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_SELBOX,LBN_SELCHANGE),
						(LPARAM)GetDlgItem(hwndDlg,IDC_SELBOX));

			SendMessageW(hw,WM_SETREDRAW,TRUE,0);
			_setreadme(hwndDlg,0);

			LangSwitchDelayText(hwndDlg);
		}
		return FALSE;
		case WM_DESTROY:
			{
				HWND listWindow;
				listWindow = GetDlgItem(hwndDlg, IDC_SELBOX);
				if (NULL != listWindow)
					DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
			}
			return FALSE;

		case WM_USER+0x123:
		{
			HMENU h = GetSubMenu(GetSubMenu(top_menu,5), 1);
			if (h)
			{
				POINT p,ps;
				GetCursorPos(&p);
				ps=p;
				ScreenToClient(GetDlgItem(hwndDlg,IDC_SELBOX),&ps);
				LRESULT x = SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_ITEMFROMPOINT,0,MAKELPARAM(ps.x,ps.y));

				if (HIWORD(x)==0 && (x=LOWORD(x)) >= 0)
				{         
					wchar_t lang[MAX_PATH] = {0};
					SendDlgItemMessageW(hwndDlg,IDC_SELBOX,LB_GETTEXT,x,(LPARAM)lang);

					EnableMenuItem(h,0,MF_BYPOSITION|((LOWORD(x)!=cur_lang)?MF_ENABLED:MF_GRAYED));
					EnableMenuItem(h,2,MF_BYPOSITION|(x ? MF_ENABLED : MF_GRAYED));
					EnableMenuItem(h,3,MF_BYPOSITION|(x ? MF_ENABLED : MF_GRAYED));

					//int sel=DoTrackPopup(h,TPM_RETURNCMD|TPM_NONOTIFY|TPM_RIGHTBUTTON,p.x,p.y,hwndDlg);
					int sel=TrackPopupMenu(h,TPM_RETURNCMD|TPM_NONOTIFY|TPM_RIGHTBUTTON,p.x,p.y,0,hwndDlg,NULL);
					if (sel)
					{
						if (sel == ID_LANG_SWITCHTOLANGUAGEPACK)
						{
							LangSwitchToLang(hwndDlg,LOWORD(x));
						}
						else if (sel == ID_LANG_RENAMELANGUAGEPACK)
						{
							LangRenameLang(hwndDlg,LOWORD(x));
						}
						else if (sel == ID_LANG_DELETELANGUAGEPACK)
						{
							LangDeleteLang(hwndDlg,LOWORD(x));
						}
					}
				}
			}
			return 0;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_SELECT_LNG_PACK:
				{
					SendMessageW(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_SELBOX,LBN_DBLCLK),
								(LPARAM)GetDlgItem(hwndDlg,IDC_SELBOX));
					break;
				}
				case IDC_RENAME_LNG_PACK:
				{
					LangRenameLang(hwndDlg,-1);
					return 0;
				}
				case IDC_DELETE_LNG_PACK:
				{
					LangDeleteLang(hwndDlg,-1);
					return 0;
				}
				case IDC_LANG_INSTALL_PROMPT:
				{
					config_wlz_prompt = !config_wlz_prompt;
					return 0;
				}
				case IDC_SHOW_LNG_PACK:
				{
					config_wlz_menu = !config_wlz_menu;

					if (config_wlz_menu)
					{
						MENUITEMINFOW mii = {sizeof(mii), MIIM_SUBMENU | MIIM_TYPE | MIIM_ID, MFT_STRING, };
						mii.hSubMenu = g_submenus_lang = CreatePopupMenu();
						mii.dwTypeData = getStringW(IDS_LANGUAGEPACKS_MENU, NULL, 0);
						mii.cch = (UINT)wcslen(mii.dwTypeData);
						g_submenus_lang_id = mii.wID = unique_loword_command++;
						InsertMenuItemW(main_menu, MAINMENU_OPTIONS_BASE+4, TRUE, &mii);

						mii.hSubMenu = g_submenus_lang;
						mii.dwTypeData = getStringW(IDS_LANGUAGEPACKS_MENU, NULL, 0);
						mii.cch = (UINT)wcslen(mii.dwTypeData);
						mii.wID = g_submenus_lang_id;
						InsertMenuItemW(GetSubMenu(v5_top_menu, 2), 1, TRUE, &mii);
						g_mm_ffoptionsbase_adj++;
					}
					else
					{
						DeleteMenu(main_menu, g_submenus_lang_id, MF_BYCOMMAND);
						DeleteMenu(GetSubMenu(v5_top_menu, 2), g_submenus_lang_id, MF_BYCOMMAND);

						g_mm_ffoptionsbase_adj--;
						g_submenus_lang_id = 0;
						g_submenus_lang = 0;
					}
					break;
				}
				case IDC_WINAMPLINK:
				{
					myOpenURLWithFallback(hwndDlg, L"http://www.google.com/search?q=%22winamp+language+packs%22+5.9", L"http://forums.winamp.com/showthread.php?t=458120#lang");
					return 0;
				}
				case IDC_CHDIR:
				{
					BROWSEINFOW bi = {0};
					bi.hwndOwner = hwndDlg;
					bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
					bi.lpfn = BrowseLangCallbackProc;
					ITEMIDLIST *idlist = SHBrowseForFolderW(&bi);
					if (idlist)
					{
						wchar_t path[MAX_PATH] = {0};
						SHGetPathFromIDListW(idlist, path);
						Shell_Free(idlist);

						if(!PathIsRootW(path))
						{
							wchar_t orig[MAX_PATH] = {0};
							StringCchCopyW(orig, MAX_PATH, LANGDIR);
							StringCchCopyW(LANGDIR, MAX_PATH, path);
							_w_sW("LangDir", LANGDIR);

							wchar_t message[2048] = {0};
							StringCchPrintfW(message,2048,getStringW(IDS_LANG_DIR_MOVE_MESSAGE,NULL,0),orig,LANGDIR);
							if(MessageBoxW(hwndDlg, message, getStringW(IDS_LANG_DIR_MOVE,NULL,0),MB_YESNO|MB_ICONQUESTION)==IDYES)
							{
								IFileTypeRegistrar *registrar=0;
								if (GetRegistrar(&registrar, true) == 0 && registrar)
								{
									registrar->MoveDirectoryContents(orig,LANGDIR);
									registrar->Release();
								}
							}

							SendMessageW(GetDlgItem(hwndDlg,IDC_SELBOX),LB_RESETCONTENT,0,0);
							SetDlgItemTextW(hwndDlg, IDC_CHDIR, LANGDIR);
							SendMessageW(hwndDlg,WM_USER+50,0,0);
						}
						else
						{
							wchar_t message[512] = {0};
							MessageBoxW(hwndDlg, getStringW(IDS_DIR_MOVE_ERROR, message, 512),
									getStringW(IDS_LANG_DIR_MOVE,NULL,0), MB_OK|MB_ICONEXCLAMATION);
						}
					}
					return FALSE;
				}
				case IDC_SELBOX:
				{
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						LangSwitchToLang(hwndDlg,-1);
					}

					if (HIWORD(wParam) == LBN_SELCHANGE)
					{
						int cur = SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETCURSEL,0,0);
						int en = SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETITEMDATA,cur,0);
						EnableWindow(GetDlgItem(hwndDlg,IDC_RENAME_LNG_PACK),cur);
						EnableWindow(GetDlgItem(hwndDlg,IDC_DELETE_LNG_PACK),cur);
						EnableWindow(GetDlgItem(hwndDlg,IDC_SELECT_LNG_PACK),cur!=cur_lang);
					
						wchar_t test_lng[MAX_PATH] = {0};
						SendDlgItemMessageW(hwndDlg,IDC_SELBOX,LB_GETTEXT,cur,(LPARAM)test_lng);

						if (en == 2) StringCchCatW(test_lng, MAX_PATH, L".wlz");
						else if (en == 4) StringCchCatW(test_lng, MAX_PATH, L".zip");
						PathCombineW(test_lng, LANGDIR, test_lng);

						SetDlgItemTextW(hwndDlg,IDC_LANG_INFO_STR,
										getStringW((cur!=cur_lang)?IDS_SEL_WLZ_INFO:IDS_CUR_WLZ_INFO,NULL,0));
						SetDlgItemTextW(hwndDlg,IDC_LANG_ID_STR,(cur!=cur_lang)?L"":langManager->GetLanguageIdentifier(LANG_IDENT_STR));

						// try to pull information about the selected wlz from the current selection
						if(cur){
							if(en && cur!=cur_lang){
								extract_wlz_to_dir(test_lng, 0);
								_setreadme(hwndDlg,(cur!=cur_lang)?test_lng:0);
								_cleanupDirW(test_lng);
							}
							else
								_setreadme(hwndDlg,(!en ? test_lng : 0));
						}
						else{
							SetDlgItemTextA(hwndDlg,IDC_EDIT1,"\tWinamp Default Language");
						}
					}
					return FALSE;
				}
			}
			return FALSE;
		}
	}

	link_handledraw(hwndDlg,uMsg,wParam,lParam);
	const int controls[] = 
	{
		IDC_EDIT1,
	};
	if (FALSE != DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
	{
		return TRUE;
	}
	return 0;
}