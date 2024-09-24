#include "../Agave/Language/api_language.h"
#include "config.h"
#include "preferences.h"
#include "../Winamp/wa_ipc.h"
#include <windows.h>
#include <assert.h>
#include "resource.h"
#include "link_control.h"
#include "../nu/ComboBox.h"
#include "../nu/Slider.h"
#include <strsafe.h>

extern HWND winampwnd;

INT_PTR CALLBACK Preferences_ADTS_DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			AACConfigurationFile *cfg = (AACConfigurationFile *)lParam;
			cfg->changing = false;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cfg);


			if (cfg->shoutcast == 1)
			{
				ShowWindow(GetDlgItem(hwnd, IDC_WARNING), SW_HIDE);
			}

			ComboBox profile_list(hwnd, IDC_PROFILE);
			profile_list.AddString(WASABI_API_LNGSTRINGW(IDS_AUTOMATIC), AAC_PROFILE_AUTOMATIC); assert(AAC_PROFILE_AUTOMATIC == 0);
			profile_list.AddString(L"AAC LC", AAC_PROFILE_LC); assert(AAC_PROFILE_LC == 1);
			profile_list.AddString(L"HE-AAC", AAC_PROFILE_HE); assert(AAC_PROFILE_HE == 2);
			profile_list.AddString(L"HE-AAC v2", AAC_PROFILE_HE_V2); assert(AAC_PROFILE_HE_V2 == 3);

			UpdateUI(hwnd, cfg);
			UpdateInfo(hwnd, &cfg->config);
			link_startsubclass(hwnd, IDC_URL);
			link_startsubclass(hwnd, IDC_LOGO);
			link_startsubclass(hwnd, IDC_HELPLINK);

			char temp[128] = {0};
			if (Preferences_GetEncoderVersion(temp, sizeof(temp)/sizeof(*temp)) == 0)
			{
				SetDlgItemTextA(hwnd, IDC_VERSION, temp);
			}

			// this will make sure that we've got the aacplus logo shown even when using a localised version
			SendDlgItemMessage(hwnd,IDC_LOGO,STM_SETIMAGE,IMAGE_BITMAP, (LPARAM)LoadImage(WASABI_API_ORIG_HINST?WASABI_API_ORIG_HINST:enc_fhg_HINST,MAKEINTRESOURCE(IDB_FHGLOGO),IMAGE_BITMAP,0,0,LR_SHARED));
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_HELPLINK:
			SendMessage(winampwnd, WM_WA_IPC, (WPARAM)"http://help.winamp.com/", IPC_OPEN_URL);
			break;
		case IDC_LOGO:
		case IDC_URL:
			SendMessage(winampwnd, WM_WA_IPC, (WPARAM)"http://www.iis.fraunhofer.de/en/bf/amm", IPC_OPEN_URL);
			break;

		case IDC_EDIT_PRESET:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				AACConfigurationFile *cfg = (AACConfigurationFile *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				if (!cfg->changing)
				{
					BOOL s=0;
					unsigned int t=GetDlgItemInt(hwnd,IDC_EDIT_PRESET,&s,0);
					if (s)
					{
							cfg->config.bitrate = t;
						UpdateUI(hwnd, cfg);
						UpdateInfo(hwnd, &cfg->config);
						AACConfig_Save(cfg);
					}
				}
			}
			break;

		case IDC_PROFILE:
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				AACConfigurationFile *cfg = (AACConfigurationFile *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				cfg->config.profile = (unsigned int)ComboBox(hwnd, IDC_PROFILE).GetSelection();
				UpdateUI(hwnd, cfg);
				UpdateInfo(hwnd, &cfg->config);
				AACConfig_Save(cfg);
			}
			break;
		}
		break;

	case WM_HSCROLL:
		if (GetDlgItem(hwnd, IDC_SLIDER_PRESET) == (HWND)lParam)
		{
			wchar_t temp[128] = {0};
			AACConfigurationFile *cfg = (AACConfigurationFile *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

				cfg->config.bitrate = GetSliderBitrate(hwnd);
				StringCbPrintf(temp, sizeof(temp), L"%u", cfg->config.bitrate);
				SetDlgItemText(hwnd, IDC_EDIT_PRESET, temp);
				UpdateInfo(hwnd, &cfg->config);
				AACConfig_Save(cfg);
		}
		break;
	}

	link_handledraw(hwnd,msg,wParam,lParam);

	const int controls[] = 
	{
		IDC_SLIDER_PRESET,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwnd, msg, wParam, lParam, controls, ARRAYSIZE(controls)))
	{
		return 1;
	}

	return 0;
}