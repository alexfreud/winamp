#include "main.h"
#include "./preferences.h"

#include "../winamp/wa_ipc.h"
#include "./resource.h"
#include "./api__ml_online.h"
#include "./config.h"

#include <windows.h>
#include <shlobj.h>

static prefsDlgRecW preferences;
extern C_Config *g_config;

static INT_PTR CALLBACK Preferences_DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL Preferences_Register()
{
	WCHAR szBuffer[256] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_ONLINE_SERVICES, szBuffer, ARRAYSIZE(szBuffer));

	preferences.hInst = WASABI_API_LNG_HINST;
	preferences.dlgID = IDD_OMPREF;
	preferences.proc = (void *)Preferences_DialogProc;
	preferences.name = Plugin_CopyString(szBuffer);
	preferences.where = 6; // Media Library

	return (BOOL)SENDWAIPC(Plugin_GetWinamp(), IPC_ADD_PREFS_DLGW, &preferences);

}

void Preferences_Unregister()
{
	SENDWAIPC(Plugin_GetWinamp(), IPC_REMOVE_PREFS_DLG, &preferences);

}

BOOL Preferences_Show()
{
	return (BOOL)SENDWAIPC(Plugin_GetWinamp(), IPC_OPENPREFSTOPAGE, &preferences);
}

static INT_PTR CALLBACK Preferences_DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			CheckDlgButton(hwndDlg,IDC_AUTOSIZE,g_config->ReadInt("AutoSize",1));

			char tmp[64] = {0};
			wsprintfA(tmp,"%i",g_config->ReadInt("maxbandwidth", MAXBANDWIDTH ));
			SetDlgItemTextA(hwndDlg,IDC_RADIO_MAXBW,tmp);
			wsprintfA(tmp,"%i",g_config->ReadInt("minbandwidth",1));
			SetDlgItemTextA(hwndDlg,IDC_RADIO_MINBW,tmp);
			int radiofreq=g_config->ReadInt("radio_upd_freq",0);
			CheckDlgButton(hwndDlg,radiofreq==0?IDC_RADIO_HOURLY:radiofreq==1?IDC_RADIO_DAILY:radiofreq==2?IDC_RADIO_WEEKLY:IDC_RADIO_NEVER,BST_CHECKED);
			SetDlgItemTextA(hwndDlg, IDC_NOWPLAYINGURL, g_config->ReadString("nowplayingurl",""));
		}
		break;

        case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_AUTOSIZE:
					g_config->WriteInt("AutoSize",IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
                    break;

				case IDC_RADIO_NEVER:
				case IDC_RADIO_DAILY:
				case IDC_RADIO_WEEKLY:
				case IDC_RADIO_HOURLY:
				{
					int radiofreq=0;
					if(IsDlgButtonChecked(hwndDlg,IDC_RADIO_NEVER)) radiofreq=3;
					if(IsDlgButtonChecked(hwndDlg,IDC_RADIO_DAILY)) radiofreq=1;
					if(IsDlgButtonChecked(hwndDlg,IDC_RADIO_WEEKLY)) radiofreq=2;
					if(IsDlgButtonChecked(hwndDlg,IDC_RADIO_HOURLY)) radiofreq=0;
					g_config->WriteInt("radio_upd_freq",radiofreq);
				}
				break;

				case IDC_NOWPLAYINGURL:
				if (HIWORD(wParam) == EN_CHANGE)
				{
					char nowplayingurl[1024] = {0};
					GetDlgItemTextA(hwndDlg, IDC_NOWPLAYINGURL, nowplayingurl, ARRAYSIZE(nowplayingurl));
					g_config->WriteString("nowplayingurl",nowplayingurl);
				}
				break;
			}
		break;

		case WM_DESTROY:
        {
          char tmp[64]={0,};
          GetDlgItemTextA(hwndDlg,IDC_RADIO_MAXBW,tmp,sizeof(tmp)-1);
          int x = atoi(tmp);
          if ( x < 2 ) x = 2;
          g_config->WriteInt("maxbandwidth",x);
          GetDlgItemTextA(hwndDlg,IDC_RADIO_MINBW,tmp,sizeof(tmp)-1);
          int y = atoi(tmp);
          if ( y < 1 ) y = 1;
          if ( y > x ) y = x-1;
          g_config->WriteInt("minbandwidth",y);
        }
		break;
	}
	return 0;
}