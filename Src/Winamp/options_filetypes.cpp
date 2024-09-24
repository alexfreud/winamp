/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "main.h"
#include "Options.h"
#include "api.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "resource.h"

#define OPT_CD  0x1
#define OPT_ENQ 0x2
#define OPT_DIR 0x4
#define OPT_EXT 0x8
#define OPT_ICON 0x10

static int optchanged,
		   old_whichicon,
		   old_whichicon2;

static void hideShowAgentItems(HWND hwndDlg) 
{
	int enabled = IsDlgButtonChecked(hwndDlg, IDC_CHECK1);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK2), enabled);
}

static BOOL iconsChanged(void)
{
	if(optchanged & OPT_ICON)
	{
		if(old_whichicon != config_whichicon || old_whichicon2 != config_whichicon2)
		{
			return TRUE;
		}
	}
	return FALSE;
}

// this is used to block selections being shown in the file types listbox
// when running on Windows 8 as selecting doesn't work due to OS changes,
// so we block mouse clicks as well as the space key for toggling things.
LRESULT win8_handleclick(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_KEYDOWN && wParam == VK_SPACE)
	{
		return 0;
	}
	LRESULT ret = CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"win8_proc"), hwndDlg, uMsg, wParam, lParam);
	if(uMsg == WM_LBUTTONDOWN)
	{
		SendMessageW(hwndDlg,LB_SETSEL,0,-1);
	}
	return ret;
}

// file type tab procedure
INT_PTR CALLBACK FtypeProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	hi helpinfo[]={
		{IDC_FILETYPES_ICONSCROLL,IDS_P_FT_ICON},
		{IDC_FILETYPES_ICONSCROLL2,IDS_P_FT_ICON2},
		{IDC_ADDFILES,IDS_P_FT_ENQUEUE},
		{IDC_DIRCONTEXT,IDS_P_FT_DIRCONTEXT},
		{IDC_FTYPE_LIST,(!IsWin8()?IDS_P_FT_EXTENSIONS:IDS_P_FT_EXTENSIONS_WIN8)},
		{IDC_SELALL,IDS_P_FT_ALL},
		{IDC_SELNONE,IDS_P_FT_NO},
		{IDC_CD,IDS_P_FT_CD},
		{IDC_RSTART,IDS_P_FT_RSTART},
		{IDC_CHECK1,IDS_P_A_ENABLE},
		{IDC_CHECK2,IDS_P_A_TRAY},
	};
	DO_HELP();

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			optchanged=0;
			old_whichicon=config_whichicon;
			old_whichicon2=config_whichicon2;

			link_startsubclass(hwndDlg, IDC_SET_DEF_PROGRAM);
			SetPropW(GetDlgItem(hwndDlg, IDC_SET_DEF_PROGRAM), L"slim", (HANDLE)1);

			if (FindWindowW(L"WinampAgentMain",NULL))
				CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
			if (GetPrivateProfileIntW(L"WinampAgent",L"is_intray",1,INI_FILE))
				CheckDlgButton(hwndDlg,IDC_CHECK2,BST_CHECKED);

			SendMessageW(GetDlgItem(hwndDlg,IDC_FILETYPES_ICONSCROLL),TBM_SETRANGEMAX,0,12);
			SendMessageW(GetDlgItem(hwndDlg,IDC_FILETYPES_ICONSCROLL),TBM_SETRANGEMIN,0,0);
			SendMessageW(GetDlgItem(hwndDlg,IDC_FILETYPES_ICONSCROLL),TBM_SETPOS,1,config_whichicon);
			SendMessageW(GetDlgItem(hwndDlg,IDC_FILETYPES_ICONSCROLL2),TBM_SETRANGEMAX,0,12);
			SendMessageW(GetDlgItem(hwndDlg,IDC_FILETYPES_ICONSCROLL2),TBM_SETRANGEMIN,0,0);
			SendMessageW(GetDlgItem(hwndDlg,IDC_FILETYPES_ICONSCROLL2),TBM_SETPOS,1,config_whichicon2);
			CheckDlgButton(hwndDlg,IDC_ADDFILES,config_addtolist?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_DIRCONTEXT,config_isdircontext()?BST_CHECKED:BST_UNCHECKED);		
			CheckDlgButton(hwndDlg,IDC_CD,config_iscdplayer()?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_RSTART,config_check_ft_startup?BST_CHECKED:BST_UNCHECKED);

			wchar_t *exl = in_getextlistW();
			wchar_t *a = exl;
			wchar_t buf[MAX_PATH] = {0};
			HWND hwnd = GetDlgItem(hwndDlg,IDC_FTYPE_LIST);
			if (IsWindow(hwnd))
			{
				while (a && *a)
				{
					int len = min(lstrlenW(a) + 1, MAX_PATH);
					lstrcpynW(buf, a, len);

					int i = SendMessageW(hwnd, LB_ADDSTRING, 0, (LPARAM)buf);
					if (!IsWin8() && config_isregistered(a))
					{
						SendMessageW(hwnd,LB_SETSEL,(WPARAM)TRUE,(LPARAM)i);
					}
					a+=len;
				}
				DirectMouseWheel_EnableConvertToMouseWheel(hwnd, TRUE);
				if (IsWin8())
				{
					SetPropW(hwnd, L"win8_proc",
							(HANDLE)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)win8_handleclick));
				}
			}

			if (playlistManager && playlistManager != (api_playlistmanager *)1)
			{
				size_t playlistEnum = 0;
				int lastFind = -1;
				const wchar_t *playlistExt=0;
				while (NULL != (playlistExt=playlistManager->EnumExtension(playlistEnum++)))
				{
					int len = min(lstrlenW(playlistExt) + 1, MAX_PATH);
					lstrcpynW(buf, playlistExt, len);
					CharUpperBuffW(buf, len);

					if ((lastFind = SendMessageW(hwnd, LB_FINDSTRINGEXACT, lastFind, (LPARAM)buf)) == LB_ERR)
					{
						int i = SendMessageW(hwnd, LB_ADDSTRING, 0, (LPARAM)buf);
						if (!IsWin8() && config_isregistered((wchar_t*)playlistExt))
						{
							SendMessageW(hwnd, LB_SETSEL, (WPARAM)TRUE, (LPARAM)i);
						}
					}
				}
			}
			GlobalFree((HGLOBAL)exl);
			SendMessageW(hwnd,LB_SETTOPINDEX,0,0);

			wchar_t s[MAX_PATH] = {0};
			GetModuleFileNameW(hMainInstance,s,MAX_PATH);
			PathRemoveFileSpecW(s);
			PathAppendW(s, L"winampa.exe");
			HANDLE hf = CreateFileW(s,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
			if (hf != INVALID_HANDLE_VALUE)
				CloseHandle(hf);
			else
			{
				int ids[]={IDC_WATEXT2,IDC_WATEXT1,IDC_CHECK1,IDC_CHECK2};
				for (int x=0; x<sizeof(ids)/sizeof(ids[0]); x++)
					ShowWindow(GetDlgItem(hwndDlg,ids[x]),SW_HIDE);
			}

			hideShowAgentItems(hwndDlg);
		}
		return 0;
		case WM_DESTROY:
		{
			extern void _w_s(char *name, char *data);
			HWND hwnd=GetDlgItem(hwndDlg,IDC_FTYPE_LIST);
			int top = SendMessageW(hwnd,LB_GETCOUNT,0,0);

			DirectMouseWheel_EnableConvertToMouseWheel(hwnd, FALSE);

			// if control is held when closing the page then force a saving irrespective
			if ((GetAsyncKeyState(VK_CONTROL)&0x8000)) optchanged = OPT_CD|OPT_ENQ|OPT_DIR|OPT_EXT|OPT_ICON;

			// TODO: clear all capabilities/FileAssociations in registry so we can have a fresh list
			if (optchanged & OPT_EXT || iconsChanged())
			{
				if (!config_registermediaplayer(2))
				{
					LPMessageBox(hwndDlg, IDS_P_FILE_ASSOC_FAILURE, IDS_P_FILE_ASSOC, MB_OK | MB_ICONEXCLAMATION);
					return 0;
				}
			}

		    config_addtolist = IsDlgButtonChecked(hwndDlg,IDC_ADDFILES)?1:0;
			if (optchanged & OPT_EXT || optchanged & OPT_ENQ || iconsChanged())
			{
				if (!config_setup_filetypes(1))
				{
					LPMessageBox(hwndDlg, IDS_P_FILE_ASSOC_FAILURE, IDS_P_FILE_ASSOC, MB_OK | MB_ICONEXCLAMATION);
					return 0;
				}
			}

			if (optchanged & OPT_DIR)
			{
				if (IsDlgButtonChecked(hwndDlg,IDC_DIRCONTEXT))
				{
					if (!config_isdircontext())
					{
						if (!config_adddircontext(1))
						{
							LPMessageBox(hwndDlg, IDS_P_FILE_ASSOC_FAILURE, IDS_P_FILE_ASSOC, MB_OK | MB_ICONEXCLAMATION);
							return 0;
						}
					}
				}
				else
				{
					if (config_isdircontext())
					{
						if (!config_removedircontext(1))
						{
							LPMessageBox(hwndDlg, IDS_P_FILE_ASSOC_FAILURE, IDS_P_FILE_ASSOC, MB_OK | MB_ICONEXCLAMATION);
							return 0;
						}
					}
				}
			}

			if (optchanged & OPT_CD)
			{
				if (!config_regcdplayer(IsDlgButtonChecked(hwndDlg,IDC_CD), 1))
				{
					LPMessageBox(hwndDlg, IDS_P_FILE_ASSOC_FAILURE, IDS_P_FILE_ASSOC, MB_OK | MB_ICONEXCLAMATION);
					return 0;
				}
			}

			if (optchanged & OPT_EXT || iconsChanged())
			{
				// on Windows 8 we specify all of the extensions which are supported
				// so they can be selected by the 'set default programs' interface as
				// the preferences dialog is not able to do it due to OS changes made
				wchar_t ext_list[16384] = {0};
				if (IsWin8())
				{
					for (int x = 0; x < top; x ++) 
					{
						wchar_t buf[256] = {0};
						SendMessageW(hwnd, LB_GETTEXT, x, (LPARAM)buf);
						if (x) StringCchCatW(ext_list, ARRAYSIZE(ext_list), L":");
						StringCchCatW(ext_list, ARRAYSIZE(ext_list), buf);
						if (!config_register_capability(buf, 1))
						{
							LPMessageBox(hwndDlg, IDS_P_FILE_ASSOC_FAILURE, IDS_P_FILE_ASSOC, MB_OK | MB_ICONEXCLAMATION);
							break;
						}
						config_register(buf,1);
					}
				}
				else
				{
					for (int x = 0; x < top; x ++) 
					{
						wchar_t buf[256] = {0};
						SendMessageW(hwnd, LB_GETTEXT, x, (LPARAM)buf);
						if (SendMessageW(hwnd,LB_GETSEL,x,0)) 
						{
							if (x) StringCchCatW(ext_list, ARRAYSIZE(ext_list), L":");
							StringCchCatW(ext_list, ARRAYSIZE(ext_list), buf);
						}
						if (!config_register_capability(buf, 1))
						{
							LPMessageBox(hwndDlg, IDS_P_FILE_ASSOC_FAILURE, IDS_P_FILE_ASSOC, MB_OK | MB_ICONEXCLAMATION);
							break;
						}
						config_register(buf, SendMessageW(hwnd,LB_GETSEL,x,0));
					}
				}
				_w_sW("config_extlist", ext_list);
			}

			hwnd = FindWindowW(L"WinampAgentMain",NULL);
			if (IsWindow(hwnd)) SendMessageW(hwnd,WM_USER+1,0,0);

			if (iconsChanged()) SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_IDLIST|SHCNF_FLUSHNOWAIT,NULL,NULL);
	    }
		return 0;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_SET_DEF_PROGRAM:
				{
					if(IsWin8())
					{
						IApplicationAssociationRegistrationUI* pAARUI;
						HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistrationUI,
													  NULL,
													  CLSCTX_INPROC,
													  __uuidof(IApplicationAssociationRegistrationUI),
													  (void**)&pAARUI);
						BOOL opened = FALSE;
						if (SUCCEEDED(hr))
						{
							hr = pAARUI->LaunchAdvancedAssociationUI(AutoWide(app_name));
							pAARUI->Release();
							if (SUCCEEDED(hr)){
								opened = TRUE;
							}
						}
						if(opened == FALSE)
						{
							LPMessageBox(hwndDlg, IDS_DEF_PROG_LOAD_ERROR, IDS_ERROR, MB_OK | MB_ICONWARNING);
						}
					}
				}
				break;
				case IDC_CHECK2:
				{
					HWND hwnd=FindWindowW(L"WinampAgentMain",NULL);
					if (IsDlgButtonChecked(hwndDlg,IDC_CHECK2))
						WritePrivateProfileStringW(L"WinampAgent",L"is_intray",L"1",INI_FILE);
					else
						WritePrivateProfileStringW(L"WinampAgent",L"is_intray",L"0",INI_FILE);
					if (hwnd) SendMessageW(hwnd,WM_USER+1,0,0);
				}
				break;
				case IDC_CHECK1:
					if (IsDlgButtonChecked(hwndDlg,IDC_CHECK1))
					{
						config_agent_add();
					}
					else
					{
						config_agent_remove();
					}
					hideShowAgentItems(hwndDlg);
				break;
				case IDC_SELALL:
				{
					int top = SendDlgItemMessage(hwndDlg,IDC_FTYPE_LIST,LB_GETCOUNT,0,0),x;
					for (x = 0; x < top; x ++)
					{
						SendDlgItemMessage(hwndDlg,IDC_FTYPE_LIST,LB_SETSEL,1,x);
					}
					optchanged |= OPT_EXT;
				}
				break;
				case IDC_SELALL2:
				case IDC_SELALL3:
				{
					HWND h=GetDlgItem(hwndDlg,IDC_FTYPE_LIST);
					int top = SendMessageW(h,LB_GETCOUNT,0,0), x;
					for (x = 0; x < top; x ++)
					{
						char buf[1024] = {0}, buf2[64] = {0};
						lstrcpynA(buf,"test.", sizeof(buf)/sizeof(*buf));
						SendMessageW(h,LB_GETTEXT,x,(LPARAM)(buf+5));

						in_get_extended_fileinfo(buf,"type",buf2,32); // I FUCKING LOVE YOU rOn
						SendMessageW(h,LB_SETSEL,(!!atoi(buf2)) ^ (!!(LOWORD(wParam) == IDC_SELALL2)),x);
					}
					optchanged |= OPT_EXT;
				}
				break;
				case IDC_SELNONE:
				{
					int top = SendDlgItemMessage(hwndDlg,IDC_FTYPE_LIST,LB_GETCOUNT,0,0),x;
					for (x = 0; x < top; x ++)
					{
						SendDlgItemMessage(hwndDlg,IDC_FTYPE_LIST,LB_SETSEL,0,x);
					}
					optchanged |= OPT_EXT;
				}  
				break;
				case IDC_RSTART:
					config_check_ft_startup=(IsDlgButtonChecked(hwndDlg,IDC_RSTART)?1:0);
				break;

				// 5.58+ (22/01/2010 - dro)
				// do this to track if an option has been changed so that elevation, etc
				// will only be done when it is needed as prior behaviour is to just set
				// the options again when things haven't been altered which is annoying
				// when you have a UAC prompt appearing when it really isn't needed
				case IDC_CD:
					optchanged ^= OPT_CD;
				break;
				case IDC_ADDFILES:
					optchanged ^= OPT_ENQ;
				break;
				case IDC_DIRCONTEXT:
					optchanged ^= OPT_DIR;
				break;
				case IDC_FTYPE_LIST:
					if(HIWORD(wParam) == LBN_SELCHANGE) optchanged |= OPT_EXT;
				break;
			}
		return 0;
		case WM_PAINT:
	    {
			PAINTSTRUCT ps;
			HICON hIcon;
			RECT r;
		    BeginPaint(hwndDlg,&ps);
		    GetWindowRect(GetDlgItem(hwndDlg,IDC_FILETYPES_ICON),&r);
		    ScreenToClient(hwndDlg,(LPPOINT) &r);
		    ScreenToClient(hwndDlg,(LPPOINT) &r + 1);
		    hIcon = (HICON)LoadImage(hMainInstance,MAKEINTRESOURCE(geticonid(config_whichicon)),IMAGE_ICON,32,32,LR_SHARED);
		    if (hIcon)
		    {
			    DrawIconEx(ps.hdc,r.left,r.top,hIcon,32,32,0,NULL,DI_NORMAL);
		    }
		    GetWindowRect(GetDlgItem(hwndDlg,IDC_FILETYPES_ICON2),&r);
		    ScreenToClient(hwndDlg,(LPPOINT) &r);
		    ScreenToClient(hwndDlg,(LPPOINT) &r + 1);
		    hIcon = (HICON)LoadImage(hMainInstance,MAKEINTRESOURCE(geticonid(config_whichicon2)),IMAGE_ICON,32,32,LR_SHARED);
		    if (hIcon)
		    {
			    DrawIconEx(ps.hdc,r.left,r.top,hIcon,32,32,0,NULL,DI_NORMAL);
		    }
		    EndPaint(hwndDlg,&ps);
	    }
		return 0;
		case WM_VSCROLL:
	    {
		    HWND swnd = (HWND) lParam;
		    if (swnd == GetDlgItem(hwndDlg,IDC_FILETYPES_ICONSCROLL))
		    {
			RECT r;
			    config_whichicon = (unsigned char) SendMessageW(swnd,TBM_GETPOS,0,0);
			    GetWindowRect(GetDlgItem(hwndDlg,IDC_FILETYPES_ICON),&r);
			    ScreenToClient(hwndDlg,(LPPOINT) &r);
			    ScreenToClient(hwndDlg,(LPPOINT) &r + 1);
			    InvalidateRect(hwndDlg,&r,TRUE);
				optchanged |= OPT_ICON;
		    }
		    if (swnd == GetDlgItem(hwndDlg,IDC_FILETYPES_ICONSCROLL2))
		    {
			RECT r;
			    config_whichicon2 = (unsigned char) SendMessageW(swnd,TBM_GETPOS,0,0);
			    GetWindowRect(GetDlgItem(hwndDlg,IDC_FILETYPES_ICON2),&r);
			    ScreenToClient(hwndDlg,(LPPOINT) &r);
			    ScreenToClient(hwndDlg,(LPPOINT) &r + 1);
			    InvalidateRect(hwndDlg,&r,TRUE);
				optchanged |= OPT_ICON;
		    }
	    }
		return 0;
	}

	const int controls[] = 
	{
		IDC_FILETYPES_ICONSCROLL,
		IDC_FILETYPES_ICONSCROLL2,
	};
	if (FALSE != DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	link_handledraw(hwndDlg,uMsg,wParam,lParam);
	return FALSE;
} // filetypes