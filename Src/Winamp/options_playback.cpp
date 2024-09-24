/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename:
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "Main.h"
#include "resource.h"
#include "options.h"
#include "WinampAttributes.h"
#include "../Plugins/General/gen_ml/ml.h"

static void EnableDisableReplayGain(HWND hwndDlg)
{
	BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_USE_REPLAY);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_SOURCE), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_MODE), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_REPLAY_SOURCE), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_REPLAY_MODE), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_REPLAY_PREFERRED), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_NON_REPLAYGAIN), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_NON_REPLAYGAIN_DB), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_ADJ), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_REPLAYGAIN_PREAMP), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_REPLAYGAIN_PREAMP_DB), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_REPLAY_ADJ), enabled);
}

static DWORD ml_rg_config_ipc;
static DLGPROC ml_rg_config_dlg;

static void EnableReplayGainAnalyzer(HWND hwndDlg)
{
	pluginMessage message_build = {(int)ml_rg_config_ipc,0,0};

	BOOL enabled = sendMlIpc(ML_IPC_SEND_PLUGIN_MESSAGE,(WPARAM)&message_build);
	ShowWindow(GetDlgItem(hwndDlg, IDC_RGAS), enabled);
	ShowWindow(GetDlgItem(hwndDlg, IDC_RG_ASK), enabled);
	ShowWindow(GetDlgItem(hwndDlg, IDC_RG_ALBUM), enabled);
	ShowWindow(GetDlgItem(hwndDlg, IDC_RG_ALL), enabled);
	ShowWindow(GetDlgItem(hwndDlg, IDC_RGC), enabled);

	message_build.param1 = 1;
	ml_rg_config_dlg = (DLGPROC)sendMlIpc(ML_IPC_SEND_PLUGIN_MESSAGE,(WPARAM)&message_build);
}

#define NON_RG_MIN_DB 14
#define NON_RG_MAX_DB 6
#define NON_RG_SCALE 10

#define HAS_RG_MIN_DB 14
#define HAS_RG_MAX_DB 6
#define HAS_RG_SCALE 10

static HWND subWnd;

static LRESULT CALLBACK PlaybackUIProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	if (uMsg == WM_INITDIALOG)
	{
		SendMessageW(GetDlgItem(hwndDlg, IDC_PRIORITY), TBM_SETRANGEMAX, 0, THREAD_PRIORITY_HIGHEST-THREAD_PRIORITY_LOWEST);
		SendMessageW(GetDlgItem(hwndDlg, IDC_PRIORITY), TBM_SETRANGEMIN, 0, 0);
		SendMessageW(GetDlgItem(hwndDlg, IDC_PRIORITY), TBM_SETPOS, 1, config_playback_thread_priority-THREAD_PRIORITY_LOWEST);

		CheckDlgButton(hwndDlg,IDC_MONO,config_audio_mono);
		CheckDlgButton(hwndDlg,IDC_SURROUND,config_audio_surround);
		CheckDlgButton(hwndDlg,IDC_DITHER,config_audio_dither);
		CheckDlgButton(hwndDlg,IDC_24BIT,config_audio_bits==24);
	}
	if (uMsg == WM_HSCROLL)
	{
		HWND swnd = (HWND) lParam;
		if (swnd == GetDlgItem(hwndDlg,IDC_PRIORITY))
		{
			config_playback_thread_priority = (INT_PTR)SendMessageW(GetDlgItem(hwndDlg, IDC_PRIORITY), TBM_GETPOS, 0, 0)+THREAD_PRIORITY_LOWEST;
		}
	}

	if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
			case IDC_MONO: config_audio_mono = IsDlgButtonChecked(hwndDlg,IDC_MONO)?true:false; break;
			case IDC_SURROUND: config_audio_surround = IsDlgButtonChecked(hwndDlg,IDC_SURROUND)?true:false; break;
			case IDC_DITHER: config_audio_dither = IsDlgButtonChecked(hwndDlg,IDC_DITHER)?true:false; break;
			case IDC_24BIT:
			{
				BOOL allow24 = IsDlgButtonChecked(hwndDlg,IDC_24BIT);
				wchar_t title[64] = {0};
				getStringW(IDS_P_24BIT_WARNING_TITLE,title,64);
				if (allow24 && MessageBoxW(hwndDlg, getStringW(IDS_P_24BIT_WARNING,NULL,0), title, MB_YESNO | MB_DEFBUTTON2) == IDNO)
				{
					allow24 = FALSE;
					CheckDlgButton(hwndDlg,IDC_24BIT, BST_UNCHECKED);
				}
				config_audio_bits = allow24?24:16;
			}
				break;
		}
	}

	const int controls[] = 
	{
		IDC_PRIORITY,
	};
	if (FALSE != DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	return FALSE;
}

int first_eq_type = -1, prev_eq_type = EQ_TYPE_4FRONT;
static LRESULT CALLBACK EqualizerUIProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	if (uMsg == WM_INITDIALOG)
	{
		if(first_eq_type != -1 && first_eq_type != config_eq_type)
		{
			ShowWindow(GetDlgItem(hwndDlg,IDC_EQ_RESTART_TEXT),SW_SHOW);
		}
		else if(first_eq_type == -1)
		{
			first_eq_type = config_eq_type;
		}

		prev_eq_type = config_eq_type;
		SendDlgItemMessageW(hwndDlg,IDC_EQ,CB_ADDSTRING,0,(LPARAM)getStringW(IDS_P_4FRONT_EQ,NULL,0));
		SendDlgItemMessageW(hwndDlg,IDC_EQ,CB_ADDSTRING,0,(LPARAM)getStringW(IDS_P_CONSTANT_Q,NULL,0));
		SendDlgItemMessage(hwndDlg,IDC_EQ,CB_SETCURSEL,(config_eq_type==EQ_TYPE_CONSTANT_Q),0);

		wchar_t *name = 0;
		int width = 0;
		SendDlgItemMessageW(hwndDlg,IDC_FREQ_BANDS,CB_ADDSTRING,0,(LPARAM)(name = getStringW(IDS_P_WA_FREQ_BANDS,NULL,0)));
		width = ResizeComboBoxDropDownW(hwndDlg, IDC_FREQ_BANDS, name, width);
		SendDlgItemMessageW(hwndDlg,IDC_FREQ_BANDS,CB_ADDSTRING,0,(LPARAM)(name = getStringW(IDS_P_ISO_FREQ_BANDS,NULL,0)));
		ResizeComboBoxDropDownW(hwndDlg, IDC_FREQ_BANDS, name, width);
		SendDlgItemMessage(hwndDlg,IDC_FREQ_BANDS,CB_SETCURSEL,(config_eq_frequencies==EQ_FREQUENCIES_ISO),0);

		CheckDlgButton(hwndDlg,IDC_LIMITER,config_eq_limiter);
	}

	else if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
			case IDC_EQ:
				config_eq_type=SendDlgItemMessage(hwndDlg,IDC_EQ,CB_GETCURSEL,0,0);
				break;

			case IDC_FREQ_BANDS:
				config_eq_frequencies=SendDlgItemMessage(hwndDlg,IDC_FREQ_BANDS,CB_GETCURSEL,0,0);
				set_aot(0);
				draw_eq_init();
				eq_set(config_use_eq, (char*)eq_tab, config_preamp);
				PostMessageW(hMainWindow,WM_WA_IPC,IPC_CB_MISC_EQ,IPC_CB_MISC);
				InvalidateRect(hEQWindow, NULL, TRUE);
				break;

			case IDC_LIMITER:
				config_eq_limiter = IsDlgButtonChecked(hwndDlg,IDC_LIMITER)?true:false;
				break;
		}
	}

	else if (uMsg == WM_DESTROY)
	{
		if(config_eq_type != prev_eq_type)
		{
			if((first_eq_type != config_eq_type) &&
				LPMessageBox(hwndDlg,IDS_P_EQ_TYPE_CHANGE_MSG,IDS_P_EQ_TYPE_CHANGE,MB_YESNO|MB_ICONQUESTION) == IDYES)
			{
				_w_i("show_prefs", 42);
				PostMessageW(hMainWindow,WM_USER,0,IPC_RESTARTWINAMP);
			}
		}
	}
	return FALSE;
}

static LRESULT CALLBACK ReplayGainUIProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	if (uMsg == WM_INITDIALOG)
	{
		SendDlgItemMessageW(hwndDlg,IDC_REPLAY_SOURCE,CB_ADDSTRING,0,(LPARAM)getStringW(IDS_P_PLAYBACK_RG_SOURCE_TRACK,NULL,0));
		SendDlgItemMessageW(hwndDlg,IDC_REPLAY_SOURCE,CB_ADDSTRING,0,(LPARAM)getStringW(IDS_P_PLAYBACK_RG_SOURCE_ALBUM,NULL,0));
		SendDlgItemMessage(hwndDlg,IDC_REPLAY_SOURCE, CB_SETCURSEL, config_replaygain_source,0);

		SendDlgItemMessageW(hwndDlg,IDC_REPLAY_MODE,CB_ADDSTRING,0,(LPARAM)getStringW(IDS_P_PLAYBACK_RG_MODE_AG,NULL,0));
		SendDlgItemMessageW(hwndDlg,IDC_REPLAY_MODE,CB_ADDSTRING,0,(LPARAM)getStringW(IDS_P_PLAYBACK_RG_MODE_AG_PC,NULL,0));
		SendDlgItemMessageW(hwndDlg,IDC_REPLAY_MODE,CB_ADDSTRING,0,(LPARAM)getStringW(IDS_P_PLAYBACK_RG_MODE_N,NULL,0));
		SendDlgItemMessageW(hwndDlg,IDC_REPLAY_MODE,CB_ADDSTRING,0,(LPARAM)getStringW(IDS_P_PLAYBACK_RG_MODE_PC,NULL,0));

		SendDlgItemMessage(hwndDlg,IDC_REPLAY_MODE, CB_SETCURSEL, config_replaygain_mode,0);

		CheckDlgButton(hwndDlg,IDC_REPLAY_PREFERRED,config_replaygain_preferred_only);

		CheckDlgButton(hwndDlg,IDC_USE_REPLAY, config_replaygain);

		SendMessageW(GetDlgItem(hwndDlg,IDC_NON_REPLAYGAIN),TBM_SETRANGEMAX,0,(NON_RG_MIN_DB+NON_RG_MAX_DB)*NON_RG_SCALE);
		SendMessageW(GetDlgItem(hwndDlg,IDC_NON_REPLAYGAIN),TBM_SETRANGEMIN,0,0);
		SendMessageW(GetDlgItem(hwndDlg,IDC_NON_REPLAYGAIN),TBM_SETPOS,1,(LPARAM)((NON_RG_MIN_DB*NON_RG_SCALE)+config_replaygain_non_rg_gain.GetFloat()*(float)NON_RG_SCALE));

		SendMessageW(GetDlgItem(hwndDlg,IDC_REPLAYGAIN_PREAMP),TBM_SETRANGEMAX,0,(HAS_RG_MIN_DB+HAS_RG_MAX_DB)*HAS_RG_SCALE);
		SendMessageW(GetDlgItem(hwndDlg,IDC_REPLAYGAIN_PREAMP),TBM_SETRANGEMIN,0,0);
		SendMessageW(GetDlgItem(hwndDlg,IDC_REPLAYGAIN_PREAMP),TBM_SETPOS,1,(LPARAM)((HAS_RG_MIN_DB*HAS_RG_SCALE)+config_replaygain_preamp.GetFloat()*(float)HAS_RG_SCALE));

		wchar_t dummy[64] = {0};
		StringCchPrintfW(dummy, 64, L"%+6.1f %s", config_replaygain_non_rg_gain.GetFloat(), getStringW(IDS_EQ_DB,NULL,0));
		SetDlgItemTextW(hwndDlg, IDC_NON_REPLAYGAIN_DB, dummy);

		StringCchPrintfW(dummy, 64, L"%+6.1f %s", config_replaygain_preamp.GetFloat(), getStringW(IDS_EQ_DB,NULL,0));
		SetDlgItemTextW(hwndDlg, IDC_REPLAYGAIN_PREAMP_DB, dummy);

		EnableDisableReplayGain(hwndDlg);

		EnableReplayGainAnalyzer(hwndDlg);
	}

	if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
			case IDC_USE_REPLAY:
				config_replaygain = IsDlgButtonChecked(hwndDlg,IDC_USE_REPLAY)?true:false;
				EnableDisableReplayGain(hwndDlg); break;
			case IDC_REPLAY_SOURCE:
				config_replaygain_source=SendDlgItemMessage(hwndDlg,IDC_REPLAY_SOURCE,CB_GETCURSEL,0,0);
				break;
			case IDC_REPLAY_MODE:
				config_replaygain_mode=SendDlgItemMessage(hwndDlg,IDC_REPLAY_MODE,CB_GETCURSEL,0,0);
				break;
			case IDC_REPLAY_PREFERRED:
				config_replaygain_preferred_only = IsDlgButtonChecked(hwndDlg,IDC_REPLAY_PREFERRED)?true:false;
				break;
		}
	}

	if (uMsg == WM_HSCROLL)
	{
		HWND swnd = (HWND) lParam;
		if (swnd == GetDlgItem(hwndDlg,IDC_NON_REPLAYGAIN))
		{
			config_replaygain_non_rg_gain = (float)((int)SendMessageW(GetDlgItem(hwndDlg,IDC_NON_REPLAYGAIN),TBM_GETPOS,0,0)-NON_RG_MIN_DB*NON_RG_SCALE) / (float)NON_RG_SCALE;
			wchar_t dummy[64] = {0};
			StringCchPrintfW(dummy, 64, L"%+6.1f %s", config_replaygain_non_rg_gain.GetFloat(), getStringW(IDS_EQ_DB,NULL,0));
			SetDlgItemTextW(hwndDlg, IDC_NON_REPLAYGAIN_DB, dummy);
		}
		else if (swnd == GetDlgItem(hwndDlg,IDC_REPLAYGAIN_PREAMP))
		{
			config_replaygain_preamp = (float)((int)SendMessageW(GetDlgItem(hwndDlg,IDC_REPLAYGAIN_PREAMP),TBM_GETPOS,0,0)-HAS_RG_MIN_DB*HAS_RG_SCALE) / (float)HAS_RG_SCALE;
			wchar_t dummy[64] = {0};
			StringCchPrintfW(dummy, 64, L"%+6.1f %s", config_replaygain_preamp.GetFloat(), getStringW(IDS_EQ_DB,NULL,0));
			SetDlgItemTextW(hwndDlg, IDC_REPLAYGAIN_PREAMP_DB, dummy);
		}
	}

	// call the ml_rg config dialog (if we've got a dlgproc for the embedded options)
	if(ml_rg_config_dlg) ml_rg_config_dlg(hwndDlg,uMsg,wParam,lParam);

	const int controls[] = 
	{
		IDC_NON_REPLAYGAIN,
		IDC_REPLAYGAIN_PREAMP,
	};
	if (FALSE != DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	return FALSE;
}

multiPage playbackPages[] = {
	{IDD_PLAYBACK, PlaybackUIProc},
	{IDD_EQUALIZER_UI, EqualizerUIProc},
	{IDD_REPLAY_GAIN_UI, ReplayGainUIProc},
	//{IDD_SILENCE_DETECT, 0},
};

#define TabCtrl_InsertItemW(hwnd, iItem, pitem)   \
    (int)SNDMSG((hwnd), TCM_INSERTITEMW, (WPARAM)(int)(iItem), (LPARAM)(const TC_ITEMW *)(pitem))

INT_PTR CALLBACK PlaybackProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			TCITEMW item = {0};
			HWND tabwnd=GetDlgItem(hwndDlg,IDC_TAB1);
			item.mask=TCIF_TEXT;
			item.pszText=getStringW(IDS_PREFS_PLAYBACK,NULL,0);
			TabCtrl_InsertItemW(tabwnd,0,&item);
			item.pszText=getStringW(IDS_P_EQUALIZER,NULL,0);
			TabCtrl_InsertItemW(tabwnd,1,&item);
			item.pszText=getStringW(IDS_P_REPLAY_GAIN,NULL,0);
			TabCtrl_InsertItemW(tabwnd,2,&item);
			#if 0
			item.pszText=getStringW(IDS_P_SILENCE_DETECT,NULL,0);
			TabCtrl_InsertItemW(tabwnd,3,&item);
			#endif

			ml_rg_config_ipc = wa_register_ipc((WPARAM)&"ml_rg_config");
			pluginMessage message_build = {(int)ml_rg_config_ipc,2,0};
			BOOL select_rg_page = sendMlIpc(ML_IPC_SEND_PLUGIN_MESSAGE,(WPARAM)&message_build);
			if(select_rg_page) config_last_playback_page = 2;

			TabCtrl_SetCurSel(tabwnd,config_last_playback_page);
			subWnd = _dosetsel(hwndDlg,subWnd,&config_last_playback_page,playbackPages,sizeof(playbackPages)/sizeof(playbackPages[0]));
		}
			break;

		case WM_NOTIFY:
		{
			LPNMHDR p=(LPNMHDR) lParam;
			if (p->idFrom == IDC_TAB1 && p->code == TCN_SELCHANGE)
				subWnd = _dosetsel(hwndDlg,subWnd,&config_last_playback_page,playbackPages,sizeof(playbackPages)/sizeof(playbackPages[0]));
		}
			break;

		case WM_DESTROY:
			subWnd=NULL;
			break;
	}
	return 0;
} // playback