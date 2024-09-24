/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "WinampAttributes.h"
#include "Options.h"
#include "resource.h"
#include "video.h"

void hideShowVideoOptions(HWND hwndDlg)
{
	int ids[] = {IDC_VIDEO_GROUP1, IDC_PREFS_VIDEO_AUTOOPEN, IDC_PREFS_VIDEO_AUTOCLOSE, IDC_PREFS_VIDEO_STOPCLOSE,
				 IDC_PREFS_VIDEO_UPDSIZE, IDC_PREFS_VIDEO_NOSS, IDC_PREFS_VIDEO_LOGO, IDC_VIDEO_GROUP2, IDC_PREFS_VIDEO_AUTO_FS_ON_START,
				 IDC_PREFS_VIDEO_REMOVE_FS_ON_STOP, IDC_PREFS_VIDEO_OSD, IDC_VIDEO_GROUP3, IDC_PREFS_VIDEO_FLIPRGB,
				 IDC_PREFS_VIDEO_ADJASP, IDC_USE_SCREENSHAPE, IDC_SCREENSHAPE1, IDC_STATIC1, IDC_SCREENSHAPE2,
				 IDC_STATIC2, IDC_PREFS_VIDEO_VSYNC};
	for (int i = 0; i < ARRAYSIZE(ids); i++)
	{
		ShowWindow(GetDlgItem(hwndDlg, ids[i]), !g_no_video_loaded);
	}
	ShowWindow(GetDlgItem(hwndDlg, IDC_VIDEO_RESTART), !g_no_video_loaded != IsDlgButtonChecked(hwndDlg, IDC_PREFS_VIDEO_ENABLE));
	if (g_no_video_loaded) ShowWindow(GetDlgItem(hwndDlg, IDC_PREFS_VIDEO_HIDE_PREFS), SW_SHOW);
}

void hideShowOverlayItems(HWND hwndDlg)
{
	int enabled = IsDlgButtonChecked(hwndDlg, IDC_PREFS_VIDEO_OVERLAYS);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PREFS_VIDEO_YV12), enabled);
}

void hideShowAspectRatioItems(HWND hwndDlg)
{
	int enabled = IsDlgButtonChecked(hwndDlg, IDC_PREFS_VIDEO_ADJASP);
	int subenabled = IsDlgButtonChecked(hwndDlg, IDC_USE_SCREENSHAPE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_USE_SCREENSHAPE), enabled);
	int ids[] = {IDC_SCREENSHAPE1, IDC_SCREENSHAPE2, IDC_STATIC1, IDC_STATIC2};
	for (int i = 0; i < ARRAYSIZE(ids); i++)
	{
		EnableWindow(GetDlgItem(hwndDlg, ids[i]), enabled && subenabled);
	}
}

// video tab procedure
INT_PTR CALLBACK VideoProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	hi helpinfo[]={
		{IDC_PREFS_VIDEO_ENABLE,IDS_P_O_VIDEO_ENABLE},
		{IDC_PREFS_VIDEO_HIDE_PREFS,IDS_P_O_VIDEO_HIDE_PREFS},
		{IDC_PREFS_VIDEO_VSYNC,IDS_P_O_VIDEO_VSYNC},
		{IDC_PREFS_VIDEO_ADJASP,IDS_P_O_VIDEO_ADJASP},
		{IDC_PREFS_VIDEO_OVERLAYS,IDS_P_O_VIDEO_OVERLAYS},
		{IDC_PREFS_VIDEO_UPDSIZE,IDS_P_O_VIDEO_UPDSIZE},
		{IDC_PREFS_VIDEO_AUTOCLOSE,IDS_P_O_VIDEO_AUTOCLOSE},
		{IDC_PREFS_VIDEO_NOSS,IDS_P_O_VIDEO_NOSS},
		{IDC_PREFS_VIDEO_OSD,IDS_P_O_VIDEO_OSD},
		{IDC_PREFS_VIDEO_YV12,IDS_P_O_VIDEO_YV12},
		{IDC_PREFS_VIDEO_STOPCLOSE,IDS_P_O_VIDEO_STOPCLOSE},
		{IDC_PREFS_VIDEO_AUTO_FS_ON_START,IDS_P_O_VIDEO_AUTO_FS_ON_START},
		{IDC_PREFS_VIDEO_REMOVE_FS_ON_STOP,IDS_P_O_VIDEO_REMOVE_FS_ON_STOP},
		{IDC_PREFS_VIDEO_AUTOOPEN,IDS_P_O_VIDEO_AUTOOPEN},
		{IDC_PREFS_VIDEO_FLIPRGB,IDS_P_O_VIDEO_FLIPRGB},
	};
	DO_HELP();
	if (uMsg == WM_INITDIALOG)
	{
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_ENABLE,!_r_i("no_video", 0));
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_HIDE_PREFS,_r_i("no_video", 0) == 2);

		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_VSYNC,config_video_vsync2);
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_ADJASP,config_video_aspectadj);
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_OVERLAYS,config_video_overlays);
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_UPDSIZE,config_video_updsize);
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_AUTOOPEN,config_video_autoopen);
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_AUTOCLOSE,config_video_autoclose);
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_NOSS,!!config_video_noss);
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_LOGO,!!config_video_logo);
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_OSD,config_video_osd);
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_YV12,config_video_yv12);
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_STOPCLOSE,config_video_stopclose);
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_AUTO_FS_ON_START,config_video_auto_fs);
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_REMOVE_FS_ON_STOP,config_video_remove_fs_on_stop);
		CheckDlgButton(hwndDlg,IDC_PREFS_VIDEO_FLIPRGB,config_video_fliprgb);
		//EnableWindow(GetDlgItem(hwndDlg,IDC_PREFS_VIDEO_YV12),config_video_overlays);
		if (config_video_useratio) CheckDlgButton(hwndDlg,IDC_USE_SCREENSHAPE,BST_CHECKED);
		SetDlgItemInt(hwndDlg,IDC_SCREENSHAPE1,config_video_ratio1,FALSE);
		SetDlgItemInt(hwndDlg,IDC_SCREENSHAPE2,config_video_ratio2,FALSE);

#ifdef EXPERIMENTAL_CONTRAST
		SendDlgItemMessage(hwndDlg,IDC_PREFS_VIDEO_CONTRAST,TBM_SETRANGE,TRUE,MAKELONG(0,255));
		SendDlgItemMessage(hwndDlg,IDC_PREFS_VIDEO_CONTRAST,TBM_SETPOS,TRUE,config_video_contrast);
		SendDlgItemMessage(hwndDlg,IDC_PREFS_VIDEO_BRIGHTNESS,TBM_SETRANGE,TRUE,MAKELONG(0,255));
		SendDlgItemMessage(hwndDlg,IDC_PREFS_VIDEO_BRIGHTNESS,TBM_SETPOS,TRUE,config_video_brightness);
#endif
		hideShowAspectRatioItems(hwndDlg);
		hideShowOverlayItems(hwndDlg);
		hideShowVideoOptions(hwndDlg);
	}
#ifdef EXPERIMENTAL_CONTRAST
	if (uMsg == WM_HSCROLL) 
	{
		HWND swnd = (HWND) lParam;
		if (swnd == GetDlgItem(hwndDlg, IDC_PREFS_VIDEO_CONTRAST))
		{
			config_video_contrast = (unsigned char) SendMessageW(GetDlgItem(hwndDlg,IDC_PREFS_VIDEO_CONTRAST),TBM_GETPOS,0,0);
		}
		if (swnd == GetDlgItem(hwndDlg, IDC_PREFS_VIDEO_BRIGHTNESS))
		{
			config_video_brightness = (unsigned char) SendMessageW(GetDlgItem(hwndDlg,IDC_PREFS_VIDEO_BRIGHTNESS),TBM_GETPOS,0,0);
		}
	}
#endif
	if (uMsg == WM_COMMAND) switch (LOWORD(wParam))
	{
		case IDC_PREFS_VIDEO_ENABLE:
			{
				int val = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_ENABLE)?1:0;
				if (val == _r_i("no_video", 0))
				{
					_w_i("no_video", !val);
					if (val == g_no_video_loaded &&
						LPMessageBox(hwndDlg, IDS_P_VIDEO_CHANGE_MSG, IDS_P_VIDEO_CHANGE, MB_YESNO | MB_ICONQUESTION) == IDYES)
					{
						PostMessageW(hMainWindow,WM_USER,0,IPC_RESTARTWINAMP);
					}
					ShowWindow(GetDlgItem(hwndDlg, IDC_VIDEO_RESTART), !g_no_video_loaded != IsDlgButtonChecked(hwndDlg, IDC_PREFS_VIDEO_ENABLE));
				}
			}
			break;
		case IDC_PREFS_VIDEO_HIDE_PREFS:
			{
				_w_i("no_video", IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_HIDE_PREFS)?2:1);
				prefsDlgRec t = {0};
				t._id = 24;
				prefs_liveDlgRemove(&t);
				prefs_last_page = 0;
				prefs_dialog(1);
			}
			break;
		case IDC_PREFS_VIDEO_VSYNC: config_video_vsync2 = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_VSYNC)?1:0; break;
		case IDC_PREFS_VIDEO_ADJASP: config_video_aspectadj = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_ADJASP)?1:0; hideShowAspectRatioItems(hwndDlg); videoReinit(); break;
		case IDC_PREFS_VIDEO_OVERLAYS: config_video_overlays = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_OVERLAYS)?1:0; hideShowOverlayItems(hwndDlg); videoReinit(); break;
		case IDC_PREFS_VIDEO_UPDSIZE: config_video_updsize = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_UPDSIZE)?1:0; break;
		case IDC_PREFS_VIDEO_AUTOOPEN: config_video_autoopen = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_AUTOOPEN)?1:0; break;
		case IDC_PREFS_VIDEO_AUTOCLOSE: config_video_autoclose = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_AUTOCLOSE)?1:0; break;
		case IDC_PREFS_VIDEO_NOSS: config_video_noss = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_NOSS)?1:0; break;
		case IDC_PREFS_VIDEO_LOGO:
		{
			config_video_logo = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_LOGO)?1:0;
			if (IsWindow(hVideoWindow))
			{
				RedrawWindow(hVideoWindow, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);
			}
			break;
		}
		case IDC_PREFS_VIDEO_OSD: config_video_osd = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_OSD)?1:0; break;
		case IDC_PREFS_VIDEO_YV12: config_video_yv12 = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_YV12)?1:0; break;
		case IDC_PREFS_VIDEO_STOPCLOSE: config_video_stopclose = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_STOPCLOSE)?1:0; break;
		case IDC_PREFS_VIDEO_AUTO_FS_ON_START: config_video_auto_fs = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_AUTO_FS_ON_START)?1:0; break;
		case IDC_PREFS_VIDEO_REMOVE_FS_ON_STOP: config_video_remove_fs_on_stop = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_REMOVE_FS_ON_STOP)?1:0; break;
		case IDC_PREFS_VIDEO_FLIPRGB:
		{
			int new_fliprgb = IsDlgButtonChecked(hwndDlg,IDC_PREFS_VIDEO_FLIPRGB)?1:0;
			config_video_fliprgb = 0;
			videoSetFlip(new_fliprgb);
			config_video_fliprgb = new_fliprgb;
		}
		break;
		case IDC_USE_SCREENSHAPE:
			config_video_useratio=!!IsDlgButtonChecked(hwndDlg,IDC_USE_SCREENSHAPE);
			hideShowAspectRatioItems(hwndDlg);
			videoReinit(); 
		break;
		case IDC_SCREENSHAPE1:
		{
	        BOOL t;
		    config_video_ratio1 = GetDlgItemInt(hwndDlg,IDC_SCREENSHAPE1,&t,FALSE);
			if (!t || !config_video_ratio1) config_video_ratio1=4;
	        if (config_video_useratio) videoReinit(); 
		}
		break;
		case IDC_SCREENSHAPE2:
		{
	        BOOL t;
		    config_video_ratio2 = GetDlgItemInt(hwndDlg,IDC_SCREENSHAPE2,&t,FALSE);
			if (!t || !config_video_ratio2) config_video_ratio2=3;
	        if (config_video_useratio) videoReinit(); 
		}
		break;
	}
	return FALSE;
} //video