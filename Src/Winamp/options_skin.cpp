/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "main.h"
#include "language.h"
#include "../nu/AutoWide.h"
#include "resource.h"
#include "Options.h"
#include "main.hpp"
#include "language.h"

static WNDPROC list_oldWndProc;
static wchar_t rename_skin[MAX_PATH];
static wchar_t CLASSIC_NAME[64];


static BOOL FillEnumRec(ENUMSKIN *pes, LPCWSTR pszFileName, BOOL bDirectory, LPWSTR pszName, INT cchName, LPCWSTR pszActiveFile)
{
	LPCWSTR pExt, pszFile;
	DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

	if (bDirectory)
	{
		if (pszFileName[0] == L'.' && ((pszFileName[1] == L'.' && pszFileName[2] == 0x00) || pszFileName[1] == 0x00)) return FALSE;
		pes->nType = SKIN_FILETYPE_DIR;
		pExt = pszFileName + lstrlenW(pszFileName) + 1;
	}
	else 
	{
		pExt = PathFindExtensionW(pszFileName);
		if (L'.' != *pExt) return FALSE; 
		pExt++;
		if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pExt, -1, L"zip", -1)) pes->nType = SKIN_FILETYPE_ZIP;
		else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pExt, -1, L"wal", -1)) pes->nType = SKIN_FILETYPE_WAL;
		else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pExt, -1, L"wsz", -1)) pes->nType = SKIN_FILETYPE_WSZ;
		else return FALSE;
	}

	pszFile = PathFindFileNameW(pszFileName);
	StringCchCopyNW(pszName, cchName, pszFile, (size_t)(pExt - pszFile - 1));

	pes->pszFileName = pszFileName;
	pes->pszName = pszName;
	pes->bActive = (pszActiveFile && CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pszActiveFile, -1, pes->pszFileName, -1));
	return TRUE;
}

int EnumerateSkins(ENUMSKINPROC fnEnumSkin, void *user)
{
	if (!fnEnumSkin) return FALSE;

	HANDLE h;
	BOOL bActiveFound, bTerminated;
	WIN32_FIND_DATAW d;
	wchar_t dirmask[MAX_PATH], szName[MAX_PATH], *pszActive;
	ENUMSKIN es;

	bActiveFound = FALSE;
	bTerminated = FALSE;

	if (*config_skin)
	{
		pszActive = PathFindFileNameW(config_skin);
		if (pszActive != config_skin && BuildFullPath(SKINDIR, config_skin, szName, sizeof(szName)/sizeof(wchar_t))) 
		{
			INT cr = ComparePath(szName, pszActive, SKINDIR);
			if(cr && CSTR_EQUAL != cr && PathFileExistsW(szName))
			{
				if (FillEnumRec(&es, config_skin, FALSE, szName, sizeof(szName)/sizeof(wchar_t), NULL))
				{
					es.bActive = TRUE;
					bActiveFound = TRUE;
					if (!fnEnumSkin(&es, user)) return FALSE;
				}
			}
		}
	}
	else pszActive = NULL;

	PathCombineW(dirmask, SKINDIR, L"*");
	h = FindFirstFileW(dirmask,&d);
	if (h != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			if (FillEnumRec(&es, d.cFileName, (FILE_ATTRIBUTE_DIRECTORY & d.dwFileAttributes), szName, sizeof(szName)/sizeof(wchar_t), (bActiveFound) ? NULL : pszActive))
			{
				if (es.bActive) bActiveFound = TRUE;
				if (!fnEnumSkin(&es, user)) 
				{
					bTerminated = TRUE;
					break;
				}
			}	
			
		} while (FindNextFileW(h,&d));
		FindClose(h);
	}

	if(!CLASSIC_NAME[0])getStringW(IDS_CLASSIC_SKIN_NAME,CLASSIC_NAME,64);
	es.pszFileName = NULL;
	es.pszName = CLASSIC_NAME;
	es.nType = SKIN_FILETYPE_EMBED;
	es.bActive = !bActiveFound;
	bTerminated = !fnEnumSkin(&es, user);
	return !bTerminated;
}

static int CALLBACK BrowseSkinCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
		{
			SetWindowTextW(hwnd, getStringW(IDS_P_SELECT_SKINDIR,NULL,0));
			SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)SKINDIR);
		}
		return 0;
	}
	return 0;
}

void SetDialogBoxFromFile(FILE *fp, HWND hwndDlg, int id)
{
	bool utf8=false, utf16=false;
	unsigned char BOM[3] = {0, 0, 0};
	if (fread(BOM, 3, 1, fp) == 1 && BOM[0] == 0xEF && BOM[1] == 0xBB && BOM[2] == 0xBF)
		utf8 = true;
	else
	{
		fseek(fp, 0, SEEK_SET);
		if (fread(BOM, 2, 1, fp) == 1 && BOM[0] == 0xFF && BOM[1] == 0xFE)
			utf16=true;
		else
			fseek(fp, 0, SEEK_SET);					
	}

	if (utf16)
	{
		wchar_t buffer[32768+1024] = {0},*p = buffer;
		for (;;)
		{
			fgetws(p,1024,fp);
			if (feof(fp)) break;
			if (p[wcslen(p)-1]==L'\n')
				p[wcslen(p)-1]=0;
			StringCchCatW(p,32768+1024,L"\r\n");
			p=p+wcslen(p);
			if (p-buffer > 32768) break;
		}

		buffer[32767]=0;
		SetDlgItemTextW(hwndDlg,id,buffer);
	}
	else
	{
		char buffer[32768+1024] = {0}, *p = buffer;
		for (;;)
		{
			fgets(p,1024,fp);
			if (feof(fp)) break;
			if (p[lstrlenA(p)-1]=='\n')
				p[lstrlenA(p)-1]=0;
			StringCchCatA(p,32768+1024,"\r\n");
			p=p+lstrlenA(p);
			if (p-buffer > 32768) break;
		}

		buffer[32767]=0;
		if (utf8)
			SetDlgItemTextW(hwndDlg,id,AutoWide(buffer, CP_UTF8));
		else
			SetDlgItemTextA(hwndDlg,id,buffer);
	}
}


static void _setreadme(HWND hwndDlg)
{
	if (config_skin[0])
	{
		LRESULT ipcRet;    

		if ((ipcRet=SendMessageW(hMainWindow,WM_WA_IPC,0,IPC_GETSKININFOW)) > 65536)
		{
			SetDlgItemTextW(hwndDlg,IDC_EDIT1,(const wchar_t *)ipcRet);
		}
		else if ((ipcRet=SendMessageW(hMainWindow,WM_WA_IPC,0,IPC_GETSKININFO)) > 65536)
		{
			SetDlgItemTextA(hwndDlg,IDC_EDIT1,(const char*)ipcRet);
		}
		else
		{
			wchar_t s[MAX_PATH]={0};
			PathCombineW(s, skin_directory, L"readme.txt");      
			FILE *fp=_wfopen(s,L"rt");
			if (!fp)
			{
				PathCombineW(s, skin_directory, L"read me.txt");
				fp=_wfopen(s,L"rt");
			}
			if (!fp)
			{
				PathCombineW(s, skin_directory, L"file_id.diz");
				fp=_wfopen(s,L"rt");
			}
			if (!fp)
			{
				WIN32_FIND_DATAW d;
				PathCombineW(s, skin_directory, L"*.txt");

				HANDLE h = FindFirstFileW(s,&d);
				s[0]=0;
				if (h != INVALID_HANDLE_VALUE) 
				{
					do 
					{
						if (_wcsicmp(d.cFileName,L"pledit.txt") &&
							_wcsicmp(d.cFileName,L"viscolor.txt") &&
							_wcsicmp(d.cFileName,L"region.txt")) 
						{
							PathCombineW(s, skin_directory, d.cFileName);
							break;
						}
      				} while (FindNextFileW(h,&d));
					FindClose(h);
					if (s && L'\0' == *s) fp=_wfopen(s,L"rb");
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
	}
	else
	{
		char buf[256]={0}, form[64]={0};
		StringCchPrintfA(buf,256, getString(IDS_CLASSIC_BASE_SKIN_VERSION,form,64),app_version);
		SetDlgItemTextA(hwndDlg,IDC_EDIT1,buf);
	}
}

static BOOL CALLBACK renameSkinProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SetDlgItemTextW(hwndDlg,IDC_OLD,rename_skin);
			SetDlgItemTextW(hwndDlg,IDC_NEW,rename_skin);
		return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					GetDlgItemTextW(hwndDlg,IDC_NEW,rename_skin,MAX_PATH);
					EndDialog(hwndDlg,!!rename_skin[0]);
				return 0;
				case IDCANCEL:
					EndDialog(hwndDlg,0);
				return 0;
			}
		return 0;
	}
	return 0;
}

static void SkinDeleteSkin(HWND hwndDlg, LRESULT x)
{
	wchar_t skin[MAX_PATH] = {0};
	int x2 = SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETCURSEL,0,0);
	SendDlgItemMessageW(hwndDlg,IDC_SELBOX,LB_GETTEXT,(x!=-1?x:x2),(LPARAM)skin);

	switch (SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETITEMDATA,(x!=-1?x:x2),0))
	{
		case SKIN_FILETYPE_ZIP: StringCchCatW(skin,MAX_PATH, L".zip"); break;
		case SKIN_FILETYPE_WSZ: StringCchCatW(skin,MAX_PATH, L".wsz"); break;
		case SKIN_FILETYPE_WAL: StringCchCatW(skin,MAX_PATH, L".wal"); break;
		default: break;
	}

	if (skin[0] && _wcsicmp(skin,CLASSIC_NAME) && _wcsicmp(skin,MODERN_SKIN_NAME) &&
				   _wcsicmp(skin,BENTO_SKIN_NAME) && _wcsicmp(skin,BIG_BENTO_SKIN_NAME))
	{
		wchar_t buf[2048] = {0};
		StringCchPrintfW(buf, 2048, getStringW(IDS_P_SKINS_DELETESKIN_PROMPT,NULL,0),skin);

		if (MessageBoxW(hwndDlg,buf,getStringW(IDS_P_SKINS_DELETESKIN,NULL,0),MB_YESNO|MB_ICONQUESTION) == IDYES)
		{
			PathCombineW(buf, SKINDIR, skin);
			IFileTypeRegistrar *registrar=0;
			if (GetRegistrar(&registrar, true) == 0 && registrar)
			{
				registrar->DeleteItem(buf);
				registrar->CleanupDirectory(buf);
				registrar->Release();
			}

			SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_DELETESTRING,(x!=-1?x:x2),0);

			// check if it's the current skin and if so reset to 'classic' and refresh
			if (((x == x2) && !lstrcmpiW(config_skin, skin)) || x == -1)
			{
				SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_SETCURSEL,0,0);
				config_skin[0]=0;
				SendMessageW(hMainWindow,WM_COMMAND,WINAMP_REFRESHSKIN,0);
				_setreadme(hwndDlg);
			}
		}
	}
}

static BOOL CALLBACK AddSkinToListBox(ENUMSKIN *pes, void *user)
{
	int index;
	index = SendMessageW((HWND)user, LB_ADDSTRING, 0, (LPARAM)pes->pszName);
	if (LB_ERR != index)
	{
		SendMessageW((HWND)user, LB_SETITEMDATA, index, (LPARAM)pes->nType);
		if (pes->bActive) SendMessageW((HWND)user, LB_SETCURSEL, index, 0); 
	}
	return TRUE;
}

static void SkinRenameSkin(HWND hwndDlg, LRESULT x, int* timer_active)
{
	int x2 = SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETCURSEL,0,0);
	int nType = SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETITEMDATA,(x!=-1?x:x2),0);

	SendDlgItemMessageW(hwndDlg,IDC_SELBOX,LB_GETTEXT,(x!=-1?x:x2),(LPARAM)rename_skin);

	if (rename_skin[0] && _wcsicmp(rename_skin,CLASSIC_NAME) &&
		(nType || (_wcsicmp(rename_skin,MODERN_SKIN_NAME) &&
				   _wcsicmp(rename_skin,BENTO_SKIN_NAME) &&
				   _wcsicmp(rename_skin,BIG_BENTO_SKIN_NAME))))
	{
		wchar_t oldskin[MAX_PATH] = {0};
		StringCchCopyW(oldskin, MAX_PATH,rename_skin);

		if (LPDialogBoxW(IDD_RENAMESKIN,hwndDlg,renameSkinProc) && wcscmp(oldskin,rename_skin))
		{
			wchar_t oldname[MAX_PATH] = {0}, newname[MAX_PATH] = {0};
						
			PathCombineW(oldname, SKINDIR, oldskin);
			PathCombineW(newname, SKINDIR, rename_skin);

			switch (nType)
			{
				case SKIN_FILETYPE_ZIP: StringCchCatW(oldname,MAX_PATH, L".zip"); StringCchCatW(newname,MAX_PATH, L".zip"); break;
				case SKIN_FILETYPE_WSZ: StringCchCatW(oldname,MAX_PATH, L".wsz"); StringCchCatW(newname,MAX_PATH, L".wsz"); break;
				case SKIN_FILETYPE_WAL: StringCchCatW(oldname,MAX_PATH, L".wal"); StringCchCatW(newname,MAX_PATH, L".wal"); break;
				default: break;
			}

			IFileTypeRegistrar *registrar=0;
			if (GetRegistrar(&registrar, true) == 0 && registrar)
			{
				if (SUCCEEDED(registrar->RenameItem(oldname,newname, FALSE)))
				{
					SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_DELETESTRING,(x!=-1?x:x2),0);
					SendDlgItemMessageW(hwndDlg,IDC_SELBOX,LB_INSERTSTRING,(x!=-1?x:x2),(LPARAM)rename_skin);
					SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_SETITEMDATA,(x!=-1?x:x2), nType);

					// check if it's the current skin and if so reset to 'classic' and refresh
					if (((x == x2) && !lstrcmpiW(config_skin, newname)) || x == -1)
					{
						SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_SETCURSEL,(x!=-1?x:x2),0);
						KillTimer(hwndDlg,1);					
						SetTimer(hwndDlg,1,250,NULL);
						*timer_active=1;
					}
				}
				else
				{
					LPMessageBox(hwndDlg, IDS_P_SKINS_RN_ERR, IDS_P_SKINS_RN_ERR_CAP, MB_OK);
				}

				registrar->Release();
			}
		}
	}
}

static LRESULT WINAPI list_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if(uMsg == WM_RBUTTONUP)
	{
		PostMessageW(GetParent(hwndDlg),WM_USER+0x123,0,0);
	}
	else if(uMsg == WM_KEYDOWN && wParam == VK_DELETE)
	{
		SkinDeleteSkin(GetParent(hwndDlg),-1);
	}
	return CallWindowProcW(list_oldWndProc,hwndDlg,uMsg,wParam,lParam);
}

INT_PTR CALLBACK SkinProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{	
	static int timer_active;
	hi helpinfo[]={{IDC_SELBOX,IDS_P_SK_SEL},
			{IDC_RANDOM,IDS_P_SK_RND},
			{IDC_CHDIR,IDS_P_SK_CHDR},
			{IDC_SKIN_INSTALL_PROMPT,IDS_P_SK_PROMPT}};
	DO_HELP();

	switch (uMsg)
	{
		case WM_NOTIFYFORMAT:
		{
			return NFR_UNICODE;
		}
		case WM_INITDIALOG:
		{
			HWND listWindow = GetDlgItem(hwndDlg,IDC_SELBOX);
			if (NULL != listWindow)
			{
				SendMessageW(listWindow, CCM_SETUNICODEFORMAT, TRUE, 0);
				list_oldWndProc=(WNDPROC)SetWindowLongPtrW(listWindow,GWLP_WNDPROC,(LONG_PTR)list_newWndProc);
				DirectMouseWheel_EnableConvertToMouseWheel(listWindow, TRUE);
			}
			CheckDlgButton(hwndDlg, IDC_SKIN_INSTALL_PROMPT, config_skin_prompt);
			link_startsubclass(hwndDlg, IDC_WINAMPLINK);
		}
		case WM_USER+50:
		{
			int index, en, modern = 0, bento = 0;
			wchar_t selected[MAX_PATH] = {0};

			HWND hw = GetDlgItem(hwndDlg,IDC_SELBOX);

			SendMessageW(hw,WM_SETREDRAW,FALSE,0);

			EnumerateSkins(AddSkinToListBox, hw);
			index = (INT)SendMessageW(hw,LB_GETCURSEL,0,0);
			if (LB_ERR == index || LB_ERR == SendMessageW(hw, LB_GETTEXT, index, (LPARAM)selected))
				selected[0] = 0x00;

			index = (INT)SendMessageW(hw, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)CLASSIC_NAME);
			if (LB_ERR != index) 
			{
				SendMessageW(hw, LB_DELETESTRING, index, 0);
				SendMessageW(hw, LB_INSERTSTRING, 0, (LPARAM)CLASSIC_NAME);
			}
				
			index = (INT)SendMessageW(hw, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)MODERN_SKIN_NAME);
			if (LB_ERR != index) 
			{
				SendMessageW(hw, LB_DELETESTRING, index, 0);
				SendMessageW(hw, LB_INSERTSTRING, 1, (LPARAM)MODERN_SKIN_NAME);
				modern = 1;
			}

			index = (INT)SendMessageW(hw, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)BENTO_SKIN_NAME);
			if (LB_ERR != index) 
			{
				SendMessageW(hw, LB_DELETESTRING, index, 0);
				SendMessageW(hw, LB_INSERTSTRING, 1 + modern, (LPARAM)BENTO_SKIN_NAME);
				bento = 1;
			}

			index = (INT)SendMessageW(hw, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)BIG_BENTO_SKIN_NAME);
			if (LB_ERR != index) 
			{
				SendMessageW(hw, LB_DELETESTRING, index, 0);
				SendMessageW(hw, LB_INSERTSTRING, 1 + modern + bento, (LPARAM)BIG_BENTO_SKIN_NAME);
			}

			index =  (*selected) ? (INT)SendMessageW(hw, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)selected) : LB_ERR;
			if (LB_ERR == index) index = 0;
			SendMessageW(hw,LB_SETCURSEL, index, 0);

			en = config_skin[0] && _wcsicmp(config_skin,MODERN_SKIN_NAME) &&
								   _wcsicmp(config_skin,BENTO_SKIN_NAME) &&
								   _wcsicmp(config_skin,BIG_BENTO_SKIN_NAME);
			EnableWindow(GetDlgItem(hwndDlg,IDC_RENAME_SKIN),en);
			EnableWindow(GetDlgItem(hwndDlg,IDC_DELETE_SKIN),en);

			SendMessageW(hw,WM_SETREDRAW,TRUE,0);
			CheckDlgButton(hwndDlg,IDC_RANDOM,config_randskin);
			_setreadme(hwndDlg);
			return FALSE;
		}
		case WM_USER+0x123:
		{
			HMENU h=GetSubMenu(GetSubMenu(top_menu,5),0);
			if (h)
			{
				POINT p,ps;
				GetCursorPos(&p);
				ps=p;
				ScreenToClient(GetDlgItem(hwndDlg,IDC_SELBOX),&ps);
				LRESULT x=SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_ITEMFROMPOINT,0,MAKELPARAM(ps.x,ps.y));
	          
				if (HIWORD(x)==0 && (x=LOWORD(x)) >= 0)
				{         
					bool allowo=x > 0;
					wchar_t skin[MAX_PATH] = {0};
					int nType = SendDlgItemMessageW(hwndDlg,IDC_SELBOX,LB_GETITEMDATA,x,0);
					SendDlgItemMessageW(hwndDlg,IDC_SELBOX,LB_GETTEXT,x,(LPARAM)skin);
					if (allowo &&
						(!_wcsicmp(skin,MODERN_SKIN_NAME) || !_wcsicmp(skin,BENTO_SKIN_NAME) ||
						 !_wcsicmp(skin,BIG_BENTO_SKIN_NAME)) && SKIN_FILETYPE_DIR == nType)
						allowo=0;

					EnableMenuItem(h,2,MF_BYPOSITION|(allowo?MF_ENABLED:MF_GRAYED));
					EnableMenuItem(h,3,MF_BYPOSITION|(allowo?MF_ENABLED:MF_GRAYED));

					//int sel=DoTrackPopup(h,TPM_RETURNCMD|TPM_NONOTIFY|TPM_RIGHTBUTTON,p.x,p.y,hwndDlg);
					int sel=TrackPopupMenu(h,TPM_RETURNCMD|TPM_NONOTIFY|TPM_RIGHTBUTTON,p.x,p.y,0,hwndDlg,NULL);
					if (sel)
					{
						if (sel == ID_PREFS_SKIN_SWITCHTOSKIN)
						{
							SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_SETCURSEL,LOWORD(x),0);
							KillTimer(hwndDlg,1);					
							SetTimer(hwndDlg,1,250,NULL);
							timer_active=1;
						}
						else if (sel == ID_PREFS_SKIN_RENAMESKIN)
						{
							SkinRenameSkin(hwndDlg, x, &timer_active);
						}
						else if (sel == ID_PREFS_SKIN_DELETESKIN)
						{
							SkinDeleteSkin(hwndDlg,x);
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
				case IDC_SKIN_INSTALL_PROMPT:
				{
					config_skin_prompt = !config_skin_prompt;
					return 0;
				}
				case IDC_RENAME_SKIN:
				{
					SkinRenameSkin(hwndDlg, -1, &timer_active);
					return 0;
				}
				case IDC_DELETE_SKIN:
				{
					SkinDeleteSkin(hwndDlg,-1);
					return 0;
				}
		        	case IDC_WINAMPLINK:
				{
					myOpenURLWithFallback(hwndDlg,L"http://www.google.com/search?q=%22winamp+skins%22", L"http://www.google.com/search?q=%22winamp+skins%22");
					return 0;
				}
				case IDC_RANDOM:
				{
					config_randskin = IsDlgButtonChecked(hwndDlg,IDC_RANDOM)?1:0;
					return 0;
				}
				case IDC_CHDIR:
				{
					BROWSEINFOW bi = {0};
					bi.hwndOwner = hwndDlg;
					bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
					bi.lpfn = BrowseSkinCallbackProc;
					ITEMIDLIST *idlist = SHBrowseForFolderW(&bi);
					if (idlist) {
						wchar_t path[MAX_PATH] = {0};
						SHGetPathFromIDListW(idlist, path);
						Shell_Free(idlist);

						if(!PathIsRootW(path)) {
							wchar_t orig[MAX_PATH] = {0};
							StringCchCopyW(orig, MAX_PATH, SKINDIR);
							StringCchCopyW(SKINDIR, MAX_PATH, path);
							_w_sW("SkinDir", SKINDIR);

							wchar_t message[2048] = {0};
							StringCchPrintfW(message,2048,getStringW(IDS_SKIN_DIR_MOVE_MESSAGE,NULL,0),orig,SKINDIR);
							if(MessageBoxW(hwndDlg, message, getStringW(IDS_SKIN_DIR_MOVE,NULL,0),MB_YESNO|MB_ICONQUESTION)==IDYES)
							{
							IFileTypeRegistrar *registrar=0;
								if (GetRegistrar(&registrar, true) == 0 && registrar)
								{
									registrar->MoveDirectoryContents(orig,SKINDIR);
									registrar->Release();
								}
							}

							SendMessageW(GetDlgItem(hwndDlg,IDC_SELBOX),LB_RESETCONTENT,0,0);
							SendMessageW(hwndDlg,WM_USER+50,0,0);
						}
						else {
						wchar_t message[512] = {0};
						MessageBoxW(hwndDlg, getStringW(IDS_DIR_MOVE_ERROR, message, 512),
									getStringW(IDS_SKIN_DIR_MOVE,NULL,0), MB_OK|MB_ICONEXCLAMATION);
						}
					}
					return FALSE;
				}
				case IDC_SELBOX:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						SendMessageW(hwndDlg,WM_COMMAND,IDOK,0);
					}
					if (HIWORD(wParam) == LBN_SELCHANGE)
					{
		            	KillTimer(hwndDlg,1);
						SetTimer(hwndDlg,1,250,NULL);
						timer_active=1;
					}
					return FALSE;
				}
			return FALSE;
		}
		case WM_DESTROY:
		{
			HWND listWindow = GetDlgItem(hwndDlg,IDC_SELBOX);
			if (NULL != listWindow)
			{
				DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
			}
			if (!timer_active) return 0;
		}
		case WM_TIMER:
		{
			wchar_t oldbuf[MAX_PATH] = {0};
			int x = SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETCURSEL,0,0);
			timer_active=0;
		        KillTimer(hwndDlg,1);
		        StringCchCopyW(oldbuf,MAX_PATH,config_skin);
			SendDlgItemMessageW(hwndDlg,IDC_SELBOX,LB_GETTEXT,x,(LPARAM)config_skin);

			switch (SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETITEMDATA,x,0))
        		{
				case SKIN_FILETYPE_ZIP: StringCchCatW(config_skin,MAX_PATH, L".zip"); break;
				case SKIN_FILETYPE_WSZ: StringCchCatW(config_skin,MAX_PATH, L".wsz"); break;
				case SKIN_FILETYPE_WAL: StringCchCatW(config_skin,MAX_PATH, L".wal"); break;
				default: break;
			}

			if (!_wcsicmp(config_skin,CLASSIC_NAME)) config_skin[0]=0;
        		{
				int en = config_skin[0] && _wcsicmp(config_skin,MODERN_SKIN_NAME) &&
									   _wcsicmp(config_skin,BENTO_SKIN_NAME) &&
									   _wcsicmp(config_skin,BIG_BENTO_SKIN_NAME);
				EnableWindow(GetDlgItem(hwndDlg,IDC_RENAME_SKIN),en);
				EnableWindow(GetDlgItem(hwndDlg,IDC_DELETE_SKIN),en);
		        }
		        if (_wcsicmp(oldbuf,config_skin)) 
		        {
				SendMessageW(hMainWindow,WM_COMMAND,WINAMP_REFRESHSKIN,0);
				_setreadme(hwndDlg);
        		}
		}
		return FALSE;
	}

	if (uMsg == WM_COMMAND && LOWORD(wParam)==IDC_PLUGINVERS)
	{
		myOpenURLWithFallback(hwndDlg, L"http://www.google.com/search?q=%22winamp+plugins%22", L"http://www.winamp.com/plugins");
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
} // skins