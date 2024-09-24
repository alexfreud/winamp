/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "Main.h"
#include "Options.h"
#include "resource.h"


// video tab procedure
INT_PTR CALLBACK StationInfoProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	/*
	hi helpinfo[]={
		{IDC_PREFS_VIDEO_VSYNC,IDS_P_O_VIDEO_VSYNC},
		{IDC_PREFS_VIDEO_ADJASP,IDS_P_O_VIDEO_ADJASP},
		{IDC_PREFS_VIDEO_OVERLAYS,IDS_P_O_VIDEO_OVERLAYS},
		{IDC_PREFS_VIDEO_UPDSIZE,IDS_P_O_VIDEO_UPDSIZE},
		{IDC_PREFS_VIDEO_AUTOCLOSE,IDS_P_O_VIDEO_AUTOCLOSE},
		{IDC_PREFS_VIDEO_NOSS,IDS_P_O_VIDEO_NOSS},
		{IDC_PREFS_VIDEO_OSD,IDS_P_O_VIDEO_OSD},
		{IDC_PREFS_VIDEO_YV12,IDS_P_O_VIDEO_YV12},
		{IDC_PREFS_VIDEO_STOPCLOSE,IDS_P_O_VIDEO_STOPCLOSE},
    {IDC_PREFS_VIDEO_REMOVE_FS_ON_STOP,IDS_P_O_VIDEO_REMOVE_FS_ON_STOP},
    {IDC_PREFS_VIDEO_AUTOOPEN,IDS_P_O_VIDEO_AUTOOPEN},
	};
	DO_HELP();*/
	if (uMsg == WM_INITDIALOG)
	{
    CheckDlgButton(hwndDlg,IDC_PREFS_STATIONINFO_AUTOSHOW,config_si_autoshow);
		CheckDlgButton(hwndDlg,IDC_PREFS_STATIONINFO_AUTOHIDE,config_si_autohide);
    CheckDlgButton(hwndDlg,IDC_PREFS_STATIONINFO_AUTOSIZE,config_si_autosize);

	}

	if (uMsg == WM_COMMAND) switch (LOWORD(wParam))
	{
		case IDC_PREFS_STATIONINFO_AUTOSHOW: config_si_autoshow = IsDlgButtonChecked(hwndDlg,IDC_PREFS_STATIONINFO_AUTOSHOW)?1:0; break;
		case IDC_PREFS_STATIONINFO_AUTOHIDE: config_si_autohide = IsDlgButtonChecked(hwndDlg,IDC_PREFS_STATIONINFO_AUTOHIDE)?1:0; break;
		case IDC_PREFS_STATIONINFO_AUTOSIZE: config_si_autosize = IsDlgButtonChecked(hwndDlg,IDC_PREFS_STATIONINFO_AUTOSIZE)?1:0; break;
	}
	return FALSE;
} //video
