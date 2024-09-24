#include "precomp__gen_ff.h"
#include "resource.h"
#include <commctrl.h>
#include "wa2cfgitems.h"
#include "gen.h"
#include "prefs.h"
#include "../Agave/Language/api_language.h"
#include <api/skin/skinparse.h>

void turnonoff(HWND wnd, int *t, int n, int v);
extern int toggle_from_wa2;
extern HWND subWnd;

static int auto_res = -1;
static int cur_res = 10;
static DWORD cur_res_last = 0;
static int cur_res_total = 0;
static int cur_res_num = 0;
static int old_res = 0;
static int dock_dist = 0;
static int dock_dist2 = 0;
static int spin_inc = 0;
static int spin_show = 0;
static int spin_hide = 0;
int desktopalpha_unavail[] = {IDC_STATIC_DA1, IDC_CHECK_DESKTOPALPHA};

static const wchar_t *getScrollTextSpeedW(float v)
{
	int skipn = (int)((1.0f / v) - 1 + 0.5f);
	static wchar_t buf[64];
	ZERO(buf);
	switch (skipn)
	{
		case 0: return WASABI_API_LNGSTRINGW_BUF(IDS_FASTER, buf, 64);
		case 1: return WASABI_API_LNGSTRINGW_BUF(IDS_FAST, buf, 64);
		case 2: return WASABI_API_LNGSTRINGW_BUF(IDS_AVERAGE, buf, 64);
		case 3: return WASABI_API_LNGSTRINGW_BUF(IDS_SLOW, buf, 64);
		case 4: return WASABI_API_LNGSTRINGW_BUF(IDS_SLOWER, buf, 64);
	}
	return WASABI_API_LNGSTRINGW_BUF(IDS_N_A, buf, 64);
}

static void nextRes(HWND dlg)
{
	if (cur_res == 250)
	{
		cfg_uioptions_timerresolution.setValueAsInt(old_res);
		SetDlgItemTextW(dlg, IDC_TXT, WASABI_API_LNGSTRINGW(IDS_FAILED_TO_DETECT_OPTIMAL_RESOLUTION));
	}
	else
	{
		if (cur_res >= 100)
			cur_res += 5;
		else if (cur_res >= 50)
			cur_res += 2;
		else
			cur_res++;
		SetDlgItemTextW(dlg, IDC_TXT, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_AUTO_DETECTING), cur_res));
		cur_res_last = Wasabi::Std::getTickCount();
		cur_res_total = 0;
		cur_res_num = 0;
		SetTimer(dlg, 1, cur_res, NULL);
	}
}

static INT_PTR CALLBACK autoTimerResProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			old_res = cfg_uioptions_timerresolution.getValueAsInt();
			cur_res = -1;
			return 1;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			cfg_uioptions_timerresolution.setValueAsInt(old_res);
			EndDialog(hwndDlg, 0);
			return 0;
		case IDOK:
			if (cur_res == -1)
			{
				cfg_uioptions_timerresolution.setValueAsInt(250);
				EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
				SetDlgItemTextW(hwndDlg, IDC_TXT, WASABI_API_LNGSTRINGW(IDS_PREPARING_AUTO_DETECTION));
				SetTimer(hwndDlg, 2, 1000, NULL);
			}
			else EndDialog(hwndDlg, IDOK);
			return 0;
		}
		break;
	case WM_TIMER:
		{
			if (wParam == 1)
			{
				DWORD now = Wasabi::Std::getTickCount();
				cur_res_total += now - cur_res_last;
				cur_res_num++;
				cur_res_last = now;
				int m = 5;
				if (cur_res >= 100) m = 2;
				else if (cur_res >= 50) m = 3;
				if (cur_res_num == m)
				{
					float average = (float)cur_res_total / (float)m;
					if (average <= (float)cur_res*1.1)
					{
						auto_res = cur_res;
						cfg_uioptions_timerresolution.setValueAsInt(old_res);
						SetDlgItemTextW(hwndDlg, IDC_TXT, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_AUTO_DETECTION_SUCCESSFUL), cur_res));
						SetDlgItemTextW(hwndDlg, IDOK, WASABI_API_LNGSTRINGW(IDS_ACCEPT));
						EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
					}
					else
					{
						nextRes(hwndDlg);
					}
				}
				return 0;
			}
			else if (wParam == 2)
			{
				KillTimer(hwndDlg, 2);
				cur_res = 9;
				nextRes(hwndDlg);
				return 0;
			}
			break;
		}
	}
	return FALSE;
}

static void autoTimerRes(HWND dlg)
{
	INT_PTR r = WASABI_API_DIALOGBOXW(IDD_AUTOTIMERRES, dlg, autoTimerResProc);
	if (r == IDOK)
	{
		cfg_uioptions_timerresolution.setValueAsInt(auto_res);
		SendMessageW(GetDlgItem(dlg, IDC_SLIDER_TIMERRESOLUTION), TBM_SETPOS, 1, auto_res);
		SetDlgItemTextW(dlg, IDC_STATIC_TIMERRES, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_TIMERS_RESOLUTION), auto_res));
	}
}

INT_PTR CALLBACK ffPrefsProc1(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			dock_dist = dock_dist2 = spin_inc = spin_show = spin_hide = 0;

			CheckDlgButton(hwndDlg, IDC_CHECK_DESKTOPALPHA, cfg_uioptions_desktopalpha.getValueAsInt());
			if (!Wasabi::Std::Wnd::isDesktopAlphaAvailable()) turnonoff(hwndDlg, desktopalpha_unavail, sizeof(desktopalpha_unavail)/sizeof(int), 0);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_TIMERRESOLUTION), TBM_SETRANGEMAX, 0, 250);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_TIMERRESOLUTION), TBM_SETRANGEMIN, 0, 10);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_TIMERRESOLUTION), TBM_SETPOS, 1, cfg_uioptions_timerresolution.getValueAsInt());
			SetDlgItemTextW(hwndDlg, IDC_STATIC_TIMERRES, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_TIMERS_RESOLUTION), cfg_uioptions_timerresolution.getValueAsInt()));
			CheckDlgButton(hwndDlg, IDC_CHECK_TOOLTIPS, cfg_uioptions_tooltips.getValueAsInt());
			CheckDlgButton(hwndDlg, IDC_CHECK_DOCKING, cfg_options_docking.getValueAsInt());
			CheckDlgButton(hwndDlg, IDC_CHECK_DOCKING2, cfg_options_appbarondrag.getValueAsInt());

			SendMessageW(GetDlgItem(hwndDlg,IDC_SPIN_DOCKDISTANCE),UDM_SETRANGE,0,MAKELONG(1000,0));
			SetDlgItemInt(hwndDlg, IDC_EDIT_DOCKDISTANCE, cfg_options_dockingdistance.getValueAsInt(), 0);
			SendMessageW(GetDlgItem(hwndDlg,IDC_SPIN_DOCKDISTANCE2),UDM_SETRANGE,0,MAKELONG(1000,0));
			SetDlgItemInt(hwndDlg, IDC_EDIT_DOCKDISTANCE2, cfg_options_appbardockingdistance.getValueAsInt(), 0);

			SetDlgItemTextA(hwndDlg, IDC_EDIT_INCREMENT, StringPrintf("%d", cfg_uioptions_textincrement.getValueAsInt()));
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_TICKERSPEED), TBM_SETRANGEMAX, 0, 4);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_TICKERSPEED), TBM_SETRANGEMIN, 0, 0);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_TICKERSPEED), TBM_SETPOS, 1, (int)(1.0f / cfg_uioptions_textspeed.getValueAsDouble() - 1.0f + 0.5f));
			SetDlgItemTextW(hwndDlg, IDC_STATIC_TICKER, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_TEXT_SCROLL_SPEED), getScrollTextSpeedW((float)cfg_uioptions_textspeed.getValueAsDouble())));

			SendMessageW(GetDlgItem(hwndDlg,IDC_SPIN_SHOWTIME),UDM_SETRANGE,0,MAKELONG(9999,0));
			SetDlgItemInt(hwndDlg, IDC_EDIT_SHOWTIME, cfg_uioptions_appbarsshowtime.getValueAsInt(), 0);
			SendMessageW(GetDlgItem(hwndDlg,IDC_SPIN_HIDETIME),UDM_SETRANGE,0,MAKELONG(9999,0));
			SetDlgItemInt(hwndDlg, IDC_EDIT_HIDETIME, cfg_uioptions_appbarshidetime.getValueAsInt(), 0);

			dock_dist = dock_dist2 = spin_inc = spin_show = spin_hide = 1;
			return 1;
		}
	case WM_COMMAND:
		toggle_from_wa2 = 1;
		switch (LOWORD(wParam))
		{
			case IDC_CHECK_DESKTOPALPHA:
				cfg_uioptions_desktopalpha.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_DESKTOPALPHA));
			return 0;
			case IDC_BUTTON_AUTOTIMERRES:
				autoTimerRes(hwndDlg);
				return 0;
			case IDC_CHECK_TOOLTIPS:
				cfg_uioptions_tooltips.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_TOOLTIPS));
				return 0;
			case IDC_CHECK_DOCKING:
				cfg_options_docking.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_DOCKING));
				return 0;
			case IDC_CHECK_DOCKING2:
				cfg_options_appbarondrag.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_DOCKING2));
				return 0;
			case IDC_EDIT_DOCKDISTANCE:
				if (HIWORD(wParam) == EN_CHANGE && dock_dist)
				{
					int t, a = GetDlgItemInt(hwndDlg, IDC_EDIT_DOCKDISTANCE, &t, 0);
					if (t) cfg_options_dockingdistance.setValueAsInt(a);
				}
				return 0;
			case IDC_EDIT_DOCKDISTANCE2:
				if (HIWORD(wParam) == EN_CHANGE && dock_dist2)
				{
					int t, a = GetDlgItemInt(hwndDlg, IDC_EDIT_DOCKDISTANCE2, &t, 0);
					if (t) cfg_options_appbardockingdistance.setValueAsInt(a);
				}
				return 0;
			case IDC_EDIT_INCREMENT:
				if (HIWORD(wParam) == EN_CHANGE && spin_inc)
				{
					int t, a = GetDlgItemInt(hwndDlg, IDC_EDIT_INCREMENT, &t, 0);
					if (t) cfg_uioptions_textincrement.setValueAsInt(a);
				}
				return 0;
			case IDC_EDIT_SHOWTIME:
				if (HIWORD(wParam) == EN_CHANGE && spin_show)
				{
					int t, a = GetDlgItemInt(hwndDlg, IDC_EDIT_SHOWTIME, &t, 0);
					if (t) cfg_uioptions_appbarsshowtime.setValueAsInt(a);
				}
				return 0;
			case IDC_EDIT_HIDETIME:
				if (HIWORD(wParam) == EN_CHANGE && spin_hide)
				{
					int t, a = GetDlgItemInt(hwndDlg, IDC_EDIT_HIDETIME, &t, 0);
					if (t) cfg_uioptions_appbarshidetime.setValueAsInt(a);
				}
				return 0;
		}
		toggle_from_wa2 = 0;
		break;
	case WM_HSCROLL:
		{
			int t = (int)SendMessageW((HWND) lParam, TBM_GETPOS, 0, 0);
			HWND ctrl = (HWND) lParam;
			if (ctrl == GetDlgItem(hwndDlg, IDC_SLIDER_TIMERRESOLUTION))
			{
				cfg_uioptions_timerresolution.setValueAsInt(t);
				SetDlgItemTextW(hwndDlg, IDC_STATIC_TIMERRES, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_TIMERS_RESOLUTION), cfg_uioptions_timerresolution.getValueAsInt()));
			}
			else if (ctrl == GetDlgItem(hwndDlg, IDC_SLIDER_TICKERSPEED))
			{
				cfg_uioptions_textspeed.setValueAsDouble(1.0 / (float)(t + 1));
				SetDlgItemTextW(hwndDlg, IDC_STATIC_TICKER, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_TEXT_SCROLL_SPEED), getScrollTextSpeedW((float)cfg_uioptions_textspeed.getValueAsDouble())));
			}
			break;
		}
	case WM_DESTROY:
		subWnd = NULL;
		dock_dist = dock_dist2 = spin_inc = spin_show = spin_hide = 0;
		return 0;
	}

	const int controls[] = 
	{
		IDC_SLIDER_TIMERRESOLUTION,
		IDC_SLIDER_TICKERSPEED,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	return 0;
}
