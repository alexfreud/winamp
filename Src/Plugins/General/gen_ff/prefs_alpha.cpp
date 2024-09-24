#include "precomp__gen_ff.h"
#include "resource.h"
#include "prefs.h"
#include "wa2cfgitems.h"
#include <bfc/wasabi_std_wnd.h>
#include "../Agave/Language/api_language.h"
#include <api/skin/skinparse.h>
#include <commctrl.h>
void turnonoff(HWND wnd, int *t, int n, int v);
extern int toggle_from_wa2;
int opacity_all_on[] = {IDC_SLIDER_CUSTOMALPHA, IDC_STATIC_TRANSP, IDC_STATIC_ALPHA, IDC_STATIC_OPAQUE, IDC_COMBO_OPACITY};
int opacity_all_off[] = {IDC_CHECK_LINKALPHA, };
int opacity_unavail[] = {IDC_STATIC_OPACITY, IDC_CHECK_LINKALLALPHA, IDC_COMBO_OPACITY, IDC_SLIDER_CUSTOMALPHA,
						 IDC_STATIC_TRANSP,IDC_STATIC_ALPHA,IDC_STATIC_OPAQUE,IDC_CHECK_LINKALPHA,IDC_STATIC_AUTOON,
						 IDC_STATIC_AUTOONTXT,IDC_STATIC_FADEIN2,IDC_SLIDER_FADEIN,IDC_STATIC_FADEIN,IDC_STATIC_HOLD2,
						 IDC_SLIDER_HOLD,IDC_STATIC_HOLD,IDC_STATIC_FADEOUT2,IDC_SLIDER_FADEOUT,IDC_STATIC_FADEOUT,
						 IDC_STATIC_EXTENDBOX,IDC_EDIT_EXTEND,IDC_STATIC_EXTEND,IDC_SPIN_EXTEND};
static int ratio_all_on[] = {IDC_SLIDER_CUSTOMSCALE, IDC_COMBO_SCALE, IDC_STATIC_SCALE};
static int ratio_all_off[] = {IDC_CHECK_LINKRATIO, };
static int spin_extend = 0;

static UINT get_logslider(HWND wnd) {
	int z = SendMessageW(wnd, TBM_GETPOS, 0, 0);
	long a = (long)(0.5 + 303.03 * pow(100.0, (double)z/30303.0) - 303.03);
	return a;
}

void set_logslider(HWND wnd, int b) {
	long a = (long) (0.5 + 30303.0 * log((double)(b+303.0)/303.03)/log(100.0));
	SendMessageW(wnd, TBM_SETPOS, 1, a);
}

int ResizeComboBoxDropDown(HWND hwndDlg, UINT id, const wchar_t * str, int width) {
	SIZE size = {0};
	HWND control = (id ? GetDlgItem(hwndDlg, id) : hwndDlg);
	HDC hdc = GetDC(control);
	// get and select parent dialog's font so that it'll calculate things correctly
	HFONT font = (HFONT)SendMessageW((id ? hwndDlg : GetParent(hwndDlg)), WM_GETFONT, 0, 0),
		  oldfont = (HFONT)SelectObject(hdc, font);
	GetTextExtentPoint32W(hdc, str, lstrlenW(str)+1, &size);

	int ret = width;
	if(size.cx > width)
	{
		if (id)
			SendDlgItemMessageW(hwndDlg, id, CB_SETDROPPEDWIDTH, size.cx, 0);
		else
			SendMessageW(hwndDlg, CB_SETDROPPEDWIDTH, size.cx, 0);

		ret = size.cx;
	}

	SelectObject(hdc, oldfont);
	ReleaseDC(control, hdc);
	return ret;
}

INT_PTR CALLBACK ffPrefsProc4(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch (uMsg) {
		case WM_INITDIALOG:
		{
			spin_extend = 0;

			int transpavail = Wasabi::Std::Wnd::isTransparencyAvailable();
			if (!transpavail) turnonoff(hwndDlg, opacity_unavail, sizeof(opacity_unavail)/sizeof(int), 0);
			CheckDlgButton(hwndDlg, IDC_CHECK_LINKALPHA, cfg_uioptions_linkalpha.getValueAsInt());
			CheckDlgButton(hwndDlg, IDC_CHECK_LINKALLALPHA, cfg_uioptions_linkallalpha.getValueAsInt());
			Layout *main = SkinParser::getMainLayout();
			int curalpha = 255;
			if (main) {
				if (!WASABI_API_WND->rootwndIsValid(main)) { return 0; }
				curalpha = cfg_uioptions_linkedalpha.getValueAsInt();
			}
			int v = (int)((curalpha / 255.0f * 100.0f)+0.5f);
			wchar_t msStr[16] = {0};
			WASABI_API_LNGSTRINGW_BUF(IDS_MS,msStr,16);
			SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER_CUSTOMALPHA),TBM_SETRANGEMAX,0,100);
			SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER_CUSTOMALPHA),TBM_SETRANGEMIN,0,10);
			SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER_CUSTOMALPHA),TBM_SETPOS,1,v);

			wchar_t *name = 0;
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_OPACITY, CB_ADDSTRING, 0, (LPARAM)(name = WASABI_API_LNGSTRINGW(IDS_OPAQUE_FOCUS)));
			int width = ResizeComboBoxDropDown(hwndDlg, IDC_COMBO_OPACITY, name, 0);
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_OPACITY, CB_SETITEMDATA, 0, 2);
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_OPACITY, CB_ADDSTRING, 0, (LPARAM)(name = WASABI_API_LNGSTRINGW(IDS_OPAQUE_HOVER)));
			width = ResizeComboBoxDropDown(hwndDlg, IDC_COMBO_OPACITY, name, width);
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_OPACITY, CB_SETITEMDATA, 1, 1);
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_OPACITY, CB_ADDSTRING, 0, (LPARAM)(name = WASABI_API_LNGSTRINGW(IDS_NO_OPACITY)));
			ResizeComboBoxDropDown(hwndDlg, IDC_COMBO_OPACITY, name, width);
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_OPACITY, CB_SETITEMDATA, 2, 0);

			int item = 2;
			switch(cfg_uioptions_autoopacitylinked.getValueAsInt())
			{
				case 1:
					item = 1;
					break;
				case 2:
					item = 0;
					break;
				default:
					item = 2;
					break;
			}
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_OPACITY, CB_SETCURSEL, item, 0);

			SetDlgItemTextA(hwndDlg, IDC_STATIC_ALPHA, StringPrintf("%d%%", v));
			turnonoff(hwndDlg, opacity_all_on, sizeof(opacity_all_on)/sizeof(int), cfg_uioptions_linkallalpha.getValueAsInt() && transpavail);
			turnonoff(hwndDlg, opacity_all_off, sizeof(opacity_all_off)/sizeof(int), !cfg_uioptions_linkallalpha.getValueAsInt() && transpavail);
			SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER_HOLD),TBM_SETRANGEMAX,0,30303);
			SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER_HOLD),TBM_SETRANGEMIN,0,0);
			set_logslider(GetDlgItem(hwndDlg,IDC_SLIDER_HOLD),cfg_uioptions_autoopacitytime.getValueAsInt());
			SetDlgItemTextW(hwndDlg, IDC_STATIC_HOLD, StringPrintfW(L"%d%s", cfg_uioptions_autoopacitytime.getValueAsInt(),msStr));
			SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER_FADEIN),TBM_SETRANGEMAX,0,30303);
			SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER_FADEIN),TBM_SETRANGEMIN,0,0);
			set_logslider(GetDlgItem(hwndDlg,IDC_SLIDER_FADEIN),cfg_uioptions_autoopacityfadein.getValueAsInt());
			SetDlgItemTextW(hwndDlg, IDC_STATIC_FADEIN, StringPrintfW(L"%d%s", cfg_uioptions_autoopacityfadein.getValueAsInt(),msStr));
			SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER_FADEOUT),TBM_SETRANGEMAX,0,30303);
			SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER_FADEOUT),TBM_SETRANGEMIN,0,0);
			set_logslider(GetDlgItem(hwndDlg,IDC_SLIDER_FADEOUT),cfg_uioptions_autoopacityfadeout.getValueAsInt());
			SetDlgItemTextW(hwndDlg, IDC_STATIC_FADEOUT, StringPrintfW(L"%d%s", cfg_uioptions_autoopacityfadeout.getValueAsInt(),msStr));

			SendMessageW(GetDlgItem(hwndDlg,IDC_SPIN_EXTEND),UDM_SETRANGE,0,MAKELONG(100,0));
			SetDlgItemInt(hwndDlg, IDC_EDIT_EXTEND, cfg_uioptions_extendautoopacity.getValueAsInt(), FALSE);

			CheckDlgButton(hwndDlg, IDC_CHECK_LINKALLRATIO, cfg_uioptions_linkallratio.getValueAsInt());
			CheckDlgButton(hwndDlg, IDC_CHECK_LINKRATIO, cfg_uioptions_linkratio.getValueAsInt());

			SendDlgItemMessageW(hwndDlg, IDC_COMBO_SCALE, CB_ADDSTRING, 0, (LPARAM)(name = WASABI_API_LNGSTRINGW(IDS_USELOCK)));
			width = ResizeComboBoxDropDown(hwndDlg, IDC_COMBO_SCALE, name, 0);
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_SCALE, CB_ADDSTRING, 0, (LPARAM)(name = WASABI_API_LNGSTRINGW(IDS_ALLLOCKED)));
			ResizeComboBoxDropDown(hwndDlg, IDC_COMBO_SCALE, name, width);
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_SCALE, CB_SETCURSEL, !!cfg_uioptions_uselocks.getValueAsInt(), 0);

			int oldscale = 1;
			if (main)
			{
				if (!WASABI_API_WND->rootwndIsValid(main)) { return 0; }
				oldscale = (int)main->getRenderRatio();
			}
			int u = (int)((oldscale * 100.0f) + 0.5f);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMSCALE), TBM_SETRANGEMAX, 0, 300);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMSCALE), TBM_SETRANGEMIN, 0, 10);
			SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMSCALE), TBM_SETPOS, 1, u);
			SetDlgItemTextA(hwndDlg, IDC_STATIC_SCALE, StringPrintf("%d%%", u));
			turnonoff(hwndDlg, ratio_all_on, sizeof(ratio_all_on) / sizeof(int), cfg_uioptions_linkallratio.getValueAsInt());
			turnonoff(hwndDlg, ratio_all_off, sizeof(ratio_all_off) / sizeof(int), !cfg_uioptions_linkallratio.getValueAsInt());

			spin_extend = 1;
			return 1;
		}
		case WM_HSCROLL:
		{
			char msStr[16] = {0};
			WASABI_API_LNGSTRING_BUF(IDS_MS,msStr,16);
			HWND ctrl = (HWND)lParam;
			int t=(int)SendMessageW((HWND) lParam,TBM_GETPOS,0,0);
			if (ctrl == GetDlgItem(hwndDlg,IDC_SLIDER_CUSTOMALPHA)) {
				SetDlgItemTextA(hwndDlg, IDC_STATIC_ALPHA, StringPrintf("%d%%", t));
				int v = (int)(t / 100.0 * 255 + 0.5);
				if (v == 254) v = 255;
				cfg_uioptions_linkedalpha.setValueAsInt(v);
			}
			else if (ctrl == GetDlgItem(hwndDlg, IDC_SLIDER_HOLD)) {
				t = get_logslider(ctrl);
				SetDlgItemTextA(hwndDlg, IDC_STATIC_HOLD, StringPrintf("%d%s", t, msStr));
				cfg_uioptions_autoopacitytime.setValueAsInt(t);
			}
			else if (ctrl == GetDlgItem(hwndDlg, IDC_SLIDER_FADEIN)) {
				t = get_logslider(ctrl);
				SetDlgItemTextA(hwndDlg, IDC_STATIC_FADEIN, StringPrintf("%d%s", t, msStr));
				cfg_uioptions_autoopacityfadein.setValueAsInt(t);
			}
			else if (ctrl == GetDlgItem(hwndDlg, IDC_SLIDER_FADEOUT)) {
				int t = get_logslider(ctrl);
				SetDlgItemTextA(hwndDlg, IDC_STATIC_FADEOUT, StringPrintf("%d%s", t, msStr));
				cfg_uioptions_autoopacityfadeout.setValueAsInt(t);
			}
			else if (ctrl == GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMSCALE))
			{
				SetDlgItemTextA(hwndDlg, IDC_STATIC_SCALE, StringPrintf("%d%%", t));
				Layout *main = SkinParser::getMainLayout();
				if (main)
				{
					main->setRenderRatio((float)t / 100.0);
					if (cfg_uioptions_linkratio.getValueAsInt())
					{
						int nc = SkinParser::getNumContainers();
						for (int i = 0;i < nc;i++)
						{
							Container *c = SkinParser::enumContainer(i);
							if (c)
							{
								int nl = c->getNumLayouts();
								for (int j = 0;j < nl;j++)
								{
									Layout *l = c->enumLayout(j);
									if (l)
									{
										UpdateWindow(l->gethWnd());
									}
								}
							}
						}
					}
				}
				return 0;
			}
			break;
		}
		case WM_COMMAND:
			toggle_from_wa2 = 1;
			switch (LOWORD(wParam)) {
				case IDC_CHECK_LINKALPHA:
					cfg_uioptions_linkalpha.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_LINKALPHA));
					return 0;
				case IDC_CHECK_LINKALLALPHA: 
					cfg_uioptions_linkallalpha.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_LINKALLALPHA));
					turnonoff(hwndDlg, opacity_all_on, sizeof(opacity_all_on)/sizeof(int), cfg_uioptions_linkallalpha.getValueAsInt());
					turnonoff(hwndDlg, opacity_all_off, sizeof(opacity_all_off)/sizeof(int), !cfg_uioptions_linkallalpha.getValueAsInt());
					return 0;
				case IDC_EDIT_EXTEND:
					if (HIWORD(wParam) == EN_CHANGE && spin_extend) {
						int t, a = GetDlgItemInt(hwndDlg,IDC_EDIT_EXTEND,&t,0);
						if (t) cfg_uioptions_extendautoopacity.setValueAsInt(MAX(a,0));
						if (a < 0)
						{
							char msStr[16] = {0};
							SetDlgItemTextA(hwndDlg, IDC_STATIC_HOLD,
													StringPrintf("%d%s", cfg_uioptions_autoopacitytime.getValueAsInt(),WASABI_API_LNGSTRING_BUF(IDS_MS,msStr,16)));
						}
					}
					return 0;
				case IDC_CHECK_LINKALLRATIO:
					cfg_uioptions_linkallratio.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_LINKALLRATIO));
					turnonoff(hwndDlg, ratio_all_on, sizeof(ratio_all_on) / sizeof(int), cfg_uioptions_linkallratio.getValueAsInt());
					turnonoff(hwndDlg, ratio_all_off, sizeof(ratio_all_off) / sizeof(int), !cfg_uioptions_linkallratio.getValueAsInt());
					return 0;
				case IDC_COMBO_OPACITY:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						int sel = SendDlgItemMessageW(hwndDlg, IDC_COMBO_OPACITY, CB_GETCURSEL, 0, 0);
						if (sel != CB_ERR)
						{
							cfg_uioptions_autoopacitylinked.setValueAsInt(SendDlgItemMessageW(hwndDlg, IDC_COMBO_OPACITY, CB_GETITEMDATA, sel, 0));
						}
					}
					return 0;
				case IDC_COMBO_SCALE:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						int sel = SendDlgItemMessageW(hwndDlg, IDC_COMBO_SCALE, CB_GETCURSEL, 0, 0);
						if (sel != CB_ERR)
						{
							cfg_uioptions_uselocks.setValueAsInt(!!sel);
						}
					}
					return 0;
				case IDC_CHECK_LINKRATIO:
					cfg_uioptions_linkratio.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_LINKRATIO));
					return 0;
			}
		case WM_DESTROY:
			spin_extend = 0;
			return 0;
	}

	const int controls[] = 
	{
		IDC_SLIDER_CUSTOMALPHA,
		IDC_SLIDER_HOLD,
		IDC_SLIDER_FADEIN,
		IDC_SLIDER_FADEOUT,
		IDC_SLIDER_CUSTOMSCALE,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	return 0;
}