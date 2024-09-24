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
#include "winampattributes.h"
#include "../nu/ns_wc.h"

static wchar_t icon_tmp[MAX_PATH] = {0};

// setup tab procedure
INT_PTR CALLBACK SetupProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	hi helpinfo[]={
		{IDC_PROXYSTR,IDS_P_SETUP_PROXY},
		{IDC_NEWVERCHECK,IDS_P_SETUP_VER},
		{IDC_NEWVERCHECK2,IDS_P_SETUP_VER2},
		{IDC_COMBO2,IDS_P_SETUP_INTERNET},
		{IDC_MINST,IDS_P_SETUP_MINST},
		{IDC_PREFS_PRIORITY_CLASS,IDS_P_SETUP_PRIO},
		{IDC_SCROLLTITLE,IDS_P_DISP_STITLE},
		{IDC_SHOWPLEDITPOS,IDS_P_DISP_SPLEDPOS},
		{IDC_SYSTRAY_SCROLLER,IDS_P_DISP_SYSTRAY},
		{IDC_CHECK5,IDS_P_DISP_SYSTRAYICON},
		{IDC_CHECK1,IDS_P_DISP_TASKBAR},
		{IDC_PREFS_SPLASH,IDS_P_O_SPLASH},
		{IDC_RECYCLE, IDS_P_O_RECYCLE},
		{IDC_NO_MOUSEWHEEL, IDS_P_NO_MOUSEWHEEL},
	};
	DO_HELP();
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SendMessageW(GetDlgItem(hwndDlg,IDC_PROXYSTR),EM_LIMITTEXT,sizeof(config_proxy),0);
			/// SetWindowTextA(GetDlgItem(hwndDlg,IDC_PROXYSTR),config_proxy);   // PROXY disabled
			CheckDlgButton(hwndDlg,IDC_MINST,config_minst?1:0);

			SendDlgItemMessageW(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)getStringW(IDS_INST_INET1,NULL,0));
			SendDlgItemMessageW(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)getStringW(IDS_INST_INET2,NULL,0));
			SendDlgItemMessageW(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)getStringW(IDS_INST_INET3,NULL,0));

			if (config_inet_mode==3) isInetAvailable(); // autodetect

			if (config_inet_mode==2) SendDlgItemMessage(hwndDlg,IDC_COMBO2, CB_SETCURSEL,2,0);
			else if (config_inet_mode==1) SendDlgItemMessage(hwndDlg,IDC_COMBO2, CB_SETCURSEL,1,0);
			else SendDlgItemMessage(hwndDlg,IDC_COMBO2, CB_SETCURSEL,0,0);

			// disable the version and stats aspects as
			// they are when internet access is disabled
			EnableWindow(GetDlgItem(hwndDlg, IDC_NEWVERCHECK), (config_inet_mode != 2));
			EnableWindow(GetDlgItem(hwndDlg, IDC_NEWVERCHECK2), (config_inet_mode != 2));
			EnableWindow(GetDlgItem(hwndDlg, IDC_NEWVERCHECK_RC), (config_inet_mode != 2));

			if (config_newverchk) 
				CheckDlgButton(hwndDlg,IDC_NEWVERCHECK,BST_CHECKED);
			if (config_newverchk2) 
				CheckDlgButton(hwndDlg,IDC_NEWVERCHECK2,BST_CHECKED);
			if (config_newverchk_rc)
				CheckDlgButton(hwndDlg, IDC_NEWVERCHECK_RC, BST_CHECKED);
			
			EnableWindow(GetDlgItem(hwndDlg, IDC_NEWVERCHECK_RC), config_newverchk > 0);

			if (GetPrivateProfileIntW(L"Winamp",L"proxyonly80",0,INI_FILE)) CheckDlgButton(hwndDlg,IDC_PROXY_ONLY_PORT_80,BST_CHECKED);

			CheckDlgButton(hwndDlg,IDC_DROPAOTFS,config_dropaotfs?1:0);
			CheckDlgButton(hwndDlg,IDC_NO_MOUSEWHEEL,config_nomwheel?1:0);

			SendDlgItemMessage(hwndDlg,IDC_PREFS_PRIORITY_CLASS,TBM_SETRANGEMAX,0,4);
			SendDlgItemMessage(hwndDlg,IDC_PREFS_PRIORITY_CLASS,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_PREFS_PRIORITY_CLASS,TBM_SETPOS,1,4-config_priority);

			CheckDlgButton(hwndDlg,IDC_SCROLLTITLE,(config_autoscrollname&2)?1:0);
			CheckDlgButton(hwndDlg,IDC_SHOWPLEDITPOS,config_dotasknum);
			CheckDlgButton(hwndDlg,IDC_PREFS_SPLASH,config_splash?1:0);

			if(!icon_tmp[0]) StringCchPrintfW(icon_tmp,MAX_PATH,L"%s\\winamp.ico",CONFIGDIR);
			int custom_icon = PathFileExistsW(icon_tmp);
			if(!custom_icon && geticonid(config_sticon) == -666) {
				config_sticon = 0;
			}
			SendDlgItemMessage(hwndDlg,IDC_SYSTRAY_SCROLLER,TBM_SETRANGEMAX,0,12+custom_icon);
			SendDlgItemMessage(hwndDlg,IDC_SYSTRAY_SCROLLER,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SYSTRAY_SCROLLER,TBM_SETPOS,1,config_sticon);

			CheckDlgButton(hwndDlg,IDC_RECYCLE,config_playlist_recyclebin?1:0);

			CheckDlgButton(hwndDlg, IDC_CHECK1, !(config_taskbar & 1));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLLTITLE), !(config_taskbar & 1));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWPLEDITPOS), !(config_taskbar & 1));
			CheckDlgButton(hwndDlg, IDC_CHECK5, config_taskbar == 1 || config_taskbar == 2);
		}
			return 0;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_NEWVERCHECK:
				case IDC_NEWVERCHECK2:
					config_newverchk=IsDlgButtonChecked(hwndDlg,IDC_NEWVERCHECK)?(config_newverchk?config_newverchk:1):0;
					EnableWindow(GetDlgItem(hwndDlg, IDC_NEWVERCHECK_RC), config_newverchk > 0);
					if (config_newverchk)
					{
						config_newverchk_rc = IsDlgButtonChecked(hwndDlg, IDC_NEWVERCHECK_RC) ? (config_newverchk_rc ? config_newverchk_rc : 1) : 0;
					}

					config_newverchk2=!!IsDlgButtonChecked(hwndDlg,IDC_NEWVERCHECK2);
					return 0;

				case IDC_COMBO2:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						int l=SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCURSEL,0,0);
						if (l == 2) config_inet_mode=2;
						else if (l == 1) config_inet_mode=1;
						else config_inet_mode=0;

						// disable the version and stats aspects as
						// they are when internet access is disabled
						EnableWindow(GetDlgItem(hwndDlg, IDC_NEWVERCHECK), (l != 2));
						EnableWindow(GetDlgItem(hwndDlg, IDC_NEWVERCHECK2), (l != 2));
					}
					return 0;

				case IDC_MINST:
					config_minst = !!IsDlgButtonChecked(hwndDlg,IDC_MINST);
					return 0;

				case IDC_PROXY_ONLY_PORT_80:
					config_proxy80 = (IsDlgButtonChecked(hwndDlg,IDC_PROXY_ONLY_PORT_80)==BST_CHECKED);
					WritePrivateProfileStringW(L"Winamp",L"proxyonly80",config_proxy80?L"1":L"0",INI_FILE);
					return 0;

				case IDC_RECYCLE:
					config_playlist_recyclebin = IsDlgButtonChecked(hwndDlg, IDC_RECYCLE)?1:0;
					break;

    			case IDC_PREFS_SPLASH:
					config_splash = IsDlgButtonChecked(hwndDlg,IDC_PREFS_SPLASH)?1:0;
					break;

				case IDC_CHECK1:
				case IDC_CHECK5:
					if (IsDlgButtonChecked(hwndDlg,IDC_CHECK1)) // taskbar
					{
						if (IsDlgButtonChecked(hwndDlg,IDC_CHECK5)) config_taskbar=2;
						else config_taskbar=0;
					}
					else // no taskbar
					{
						if (IsDlgButtonChecked(hwndDlg,IDC_CHECK5)) config_taskbar=1;
						else config_taskbar=3;
					}

					EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLLTITLE), !(config_taskbar & 1));
					EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWPLEDITPOS), !(config_taskbar & 1));
	        
					if (IsWindow(hMainWindow) && IsWindowVisible(hMainWindow)) 
					{
						set_taskbar();
					}
					else g_taskbar_dirty=1;
					break;
				case IDC_SCROLLTITLE:
				{
					int t=config_autoscrollname;
					config_autoscrollname &= ~2;
					if (IsDlgButtonChecked(hwndDlg,IDC_SCROLLTITLE)) config_autoscrollname |= 2;
					{
						if (config_autoscrollname && !t) SetTimer(hMainWindow,UPDATE_DISPLAY_TIMER+1,200,NULL);
						else if (!config_autoscrollname && t)
						{
							KillTimer(hMainWindow,UPDATE_DISPLAY_TIMER+1);
						}
						if (!(config_autoscrollname&2))
							set_caption(0, NULL);
					}
				}
					break;

				case IDC_SHOWPLEDITPOS:
					config_dotasknum = IsDlgButtonChecked(hwndDlg, IDC_SHOWPLEDITPOS)?1:0;
					// has cropped up in a few crash reports where app_name is null which causes the update to bork
					// when receiving a 'stop' command and we try to send an update via JSAPI1_CurrentTitleChanged()
					if (!app_name || app_name && !*app_name || (unsigned int)(ULONG_PTR)app_name < 65536) BuildAppName();
					set_caption(0, L"%s - %S", (config_dotasknum?FileTitleNum:FileTitle), app_name);
					break;

				case IDC_DROPAOTFS:
					config_dropaotfs = IsDlgButtonChecked(hwndDlg, IDC_DROPAOTFS)?1:0;
					break;

				case IDC_NO_MOUSEWHEEL:
					config_nomwheel = IsDlgButtonChecked(hwndDlg, IDC_NO_MOUSEWHEEL)?1:0;
					break;
			} // end of WM_COMMAND
			return 0;

		case WM_VSCROLL:
	    {
		    HWND swnd = (HWND) lParam;
		    if (swnd == GetDlgItem(hwndDlg,IDC_PREFS_PRIORITY_CLASS))
		    {
			    config_priority = 4-(unsigned char)SendMessageW(GetDlgItem(hwndDlg,IDC_PREFS_PRIORITY_CLASS),TBM_GETPOS,0,0);
			    set_priority();
		    }
	    }	
			return 0;

		case WM_DESTROY:
			GetWindowTextA(GetDlgItem(hwndDlg,IDC_PROXYSTR),config_proxy,sizeof(config_proxy));
			return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HICON hIcon;
			RECT r;
			GetWindowRect(GetDlgItem(hwndDlg,IDC_SYSTRAY_ICON),&r);
			ScreenToClient(hwndDlg,(LPPOINT) &r);
			ScreenToClient(hwndDlg,(LPPOINT) &r + 1);
			BeginPaint(hwndDlg,&ps);
			int icon_idx = geticonid(config_sticon);
			if(icon_idx != -666) {
				hIcon = (HICON)LoadImageW(hMainInstance,MAKEINTRESOURCEW(icon_idx),IMAGE_ICON,16,16,LR_SHARED);
			}
			else {
				if(!PathFileExistsW(icon_tmp)) {
					hIcon = (HICON)LoadImageW(hMainInstance,MAKEINTRESOURCEW(ICON_XP),IMAGE_ICON,16,16,LR_SHARED);
				}
				else {
					hIcon = (HICON)LoadImageW(0,icon_tmp,IMAGE_ICON,16,16,LR_LOADFROMFILE);
				}
			}

			if (hIcon)
			{
				DrawIconEx(ps.hdc,r.left+8,r.top+8,hIcon,16,16,0,NULL,DI_NORMAL);
			}
			EndPaint(hwndDlg,&ps);
		}
			break;

		case WM_HSCROLL:
		{
			HWND swnd = (HWND) lParam;
			if (swnd == GetDlgItem(hwndDlg,IDC_SYSTRAY_SCROLLER))
			{
				RECT r;
				HWND hwnd;
				config_sticon = (unsigned char) SendMessageW(swnd,TBM_GETPOS,0,0);
				_w_i("sticon",config_sticon);
				hwnd=FindWindowW(L"WinampAgentMain",NULL);
				if (hwnd) SendMessageW(hwnd,WM_USER+1,0,0);

				GetWindowRect(GetDlgItem(hwndDlg,IDC_SYSTRAY_ICON),&r);
				ScreenToClient(hwndDlg,(LPPOINT) &r);
				ScreenToClient(hwndDlg,(LPPOINT) &r + 1);
				InvalidateRect(hwndDlg,&r,TRUE);
				if (hMainWindow && (config_taskbar == 1 || config_taskbar == 2)) 
				{
					set_taskbar();
				}
			}
		}
			return FALSE;
	}

	const int controls[] = 
	{
		IDC_PREFS_PRIORITY_CLASS,
		IDC_SYSTRAY_SCROLLER,
	};
	if (FALSE != DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	return 0;
} //setup