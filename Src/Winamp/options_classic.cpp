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

static HWND subWnd;

static LRESULT CALLBACK ClassicUIProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	hi helpinfo[]={
		{IDC_SNAP,IDS_P_DISP_SNAP},
		{IDC_SNAPW,IDS_P_DISP_SNAPW},
		{IDC_HIGHLIGHT,IDS_P_DISP_HILITE},
		{IDC_CLUTTERBAR,IDS_P_DISP_CB},
		{IDC_TTIPS,IDS_P_DISP_TTIPS},
		{IDC_WA_CURSORS,IDS_P_DISP_CURSORS},
		{IDC_BIFONT,IDS_P_DISP_BIFONT},
		{IDC_BIFONT_ALT,IDS_P_DISP_BIFONT_ALT},
		{IDC_EQDSIZE,IDS_P_O_EQDS},
		{IDC_SPLB,IDS_P_O_SPLB},
		{IDC_CHECK2,IDS_P_DISP_FREESIZE},
		{IDC_KEEPONSCREEN,IDS_P_O_LITESTEP},
		{IDC_AOVD,IDS_P_O_AOVD},
		{IDC_POS_IN_SONGTICKER,IDS_P_DISP_PLPOS},
	};
	DO_HELP();
	switch (uMsg)
	{
		case WM_INITDIALOG:
			CheckDlgButton(hwndDlg,IDC_SNAP,config_snap?1:0);
			SetDlgItemInt(hwndDlg,IDC_SNAPW,config_snaplen,0);
			CheckDlgButton(hwndDlg,IDC_HIGHLIGHT,config_hilite?1:0);
			CheckDlgButton(hwndDlg,IDC_CLUTTERBAR,config_ascb_new?1:0);
			CheckDlgButton(hwndDlg,IDC_TTIPS,config_ttips?1:0);
			CheckDlgButton(hwndDlg,IDC_WA_CURSORS,config_usecursors?1:0);
			CheckDlgButton(hwndDlg,IDC_BIFONT,config_bifont?1:0);
			CheckDlgButton(hwndDlg,IDC_BIFONT_ALT,config_bifont_alt?1:0);
			CheckDlgButton(hwndDlg,IDC_SPLB,config_ospb?1:0);		
			CheckDlgButton(hwndDlg,IDC_CHECK2,config_embedwnd_freesize?1:0);      
			CheckDlgButton(hwndDlg,IDC_EQDSIZE,config_eqdsize?1:0);
			CheckDlgButton(hwndDlg,IDC_KEEPONSCREEN,(config_keeponscreen&1)?0:1);
			CheckDlgButton(hwndDlg,IDC_AOVD,(config_keeponscreen&2)?1:0);
			CheckDlgButton(hwndDlg,IDC_POS_IN_SONGTICKER,config_dotitlenum?1:0);

			EnableWindow(GetDlgItem(hwndDlg,IDC_BIFONT_ALT),!config_bifont);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_CHECK2:
					config_embedwnd_freesize=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK2);
					return 0;

				case IDC_EQDSIZE: 
					config_eqdsize = (BST_CHECKED == IsDlgButtonChecked(hwndDlg,IDC_EQDSIZE)); 
					set_aot(0); 
					break;

				case IDC_SPLB: 
					config_ospb = (BST_CHECKED == IsDlgButtonChecked(hwndDlg,IDC_SPLB)); 
					break;

				case IDC_BIFONT: 
					config_bifont = (BST_CHECKED == IsDlgButtonChecked(hwndDlg,IDC_BIFONT)) ? 1 : 0;
					EnableWindow(GetDlgItem(hwndDlg,IDC_BIFONT_ALT),!config_bifont);
					g_need_titleupd=1;
					InvalidateRect(hPLWindow, NULL, TRUE);
					break;

				case IDC_BIFONT_ALT: 
					config_bifont_alt = (BST_CHECKED == IsDlgButtonChecked(hwndDlg,IDC_BIFONT_ALT)) ? 1 : 0;
					if (config_dsize)
					{
						draw_reinit_plfont(1);
					}
					break;

				case IDC_POS_IN_SONGTICKER:
					config_dotitlenum = IsDlgButtonChecked(hwndDlg,IDC_POS_IN_SONGTICKER)?1:0;
					draw_songname(FileTitle, &ui_songposition, PlayList_getcurrentlength());
					break;

				case IDC_SNAP: config_snap = IsDlgButtonChecked(hwndDlg,IDC_SNAP)?1:0; break;

				case IDC_SNAPW:
					if (HIWORD(wParam) == EN_CHANGE) {
						int t,a;
						a=GetDlgItemInt(hwndDlg,IDC_SNAPW,&t,0);
						if (t) config_snaplen= (unsigned char)a;
					}
					break;

				case IDC_HIGHLIGHT: config_hilite = IsDlgButtonChecked(hwndDlg,IDC_HIGHLIGHT)?1:0; set_aot(0); break;

				case IDC_CLUTTERBAR:
					config_ascb_new = IsDlgButtonChecked(hwndDlg,IDC_CLUTTERBAR)?1:0;
					if (hMainWindow) draw_clutterbar(0);
					break;

				case IDC_TTIPS:
					config_ttips = IsDlgButtonChecked(hwndDlg,IDC_TTIPS)?1:0;
					set_aot(0);
					break;

				case IDC_WA_CURSORS:
					config_usecursors = IsDlgButtonChecked(hwndDlg,IDC_WA_CURSORS)?1:0;
					break;

				case IDC_KEEPONSCREEN:
				case IDC_AOVD:
					{
					    config_keeponscreen = IsDlgButtonChecked(hwndDlg,IDC_KEEPONSCREEN)?0:1;
					    config_keeponscreen |= IsDlgButtonChecked(hwndDlg,IDC_AOVD)?2:0;
					    SetWindowLongPtrW(hMainWindow,GWLP_USERDATA,(config_keeponscreen&2)?0x49474541:0);
					    SetWindowLongPtrW(hPLWindow,GWLP_USERDATA,(config_keeponscreen&2)?0x49474541:0);
					    SetWindowLongPtrW(hEQWindow,GWLP_USERDATA,(config_keeponscreen&2)?0x49474541:0);
//					    SetWindowLong(hMBWindow,GWL_USERDATA,(config_keeponscreen&2)?0x49474541:0);
					    SetWindowLongPtrW(hVideoWindow,GWLP_USERDATA,(config_keeponscreen&2)?0x49474541:0);
					}
					break;
			}
			return FALSE;
	}
	return FALSE;
} //display

static LRESULT CALLBACK ClassicVisProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
  static int text_ids[4]=
  {
	  IDS_P_CLASSIC_70FPS,
	  IDS_P_CLASSIC_35FPS,
	  IDS_P_CLASSIC_18FPS,
	  IDS_P_CLASSIC_9FPS
  };
#if 0
	hi helpinfo[]={
		{IDC_USEID4,IDS_P_O_ONLOAD},
	};
	DO_HELP();
#endif
	if(uMsg == WM_INITDIALOG)
	{
		SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER3),TBM_SETRANGEMAX,0,3);
		SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER3),TBM_SETRANGEMIN,0,0);
		SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER3),TBM_SETPOS,1,4-config_saref);

		SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER1),TBM_SETRANGEMAX,0,4);
		SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER1),TBM_SETRANGEMIN,0,0);
		SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER1),TBM_SETPOS,1,config_safalloff);

		SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER2),TBM_SETRANGEMAX,0,4);
		SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER2),TBM_SETRANGEMIN,0,0);
		SendMessageW(GetDlgItem(hwndDlg,IDC_SLIDER2),TBM_SETPOS,1,config_sa_peak_falloff);
      
		SetDlgItemTextW(hwndDlg,IDC_RRATE,getStringW(text_ids[config_saref-1],NULL,0));
		if (config_sa == 1) CheckDlgButton(hwndDlg,IDC_RADIO1,BST_CHECKED);
		else if (config_sa == 2) CheckDlgButton(hwndDlg,IDC_RADIO2,BST_CHECKED);
		else CheckDlgButton(hwndDlg,IDC_RADIO3,BST_CHECKED);

		if ((config_safire&3) == 1) CheckDlgButton(hwndDlg,IDC_RADIO5,BST_CHECKED);
		else if ((config_safire&3) == 2) CheckDlgButton(hwndDlg,IDC_RADIO6,BST_CHECKED);
		else CheckDlgButton(hwndDlg,IDC_RADIO4,BST_CHECKED);

		if (config_sa_peaks) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
		if ((config_safire&32)) CheckDlgButton(hwndDlg,IDC_RADIO7,BST_CHECKED);
		else CheckDlgButton(hwndDlg,IDC_RADIO8,BST_CHECKED);

		if (((config_safire>>2)&3)==0) CheckDlgButton(hwndDlg,IDC_RADIO9,BST_CHECKED);
		else if (((config_safire>>2)&3)==1) CheckDlgButton(hwndDlg,IDC_RADIO10,BST_CHECKED);
		else CheckDlgButton(hwndDlg,IDC_RADIO11,BST_CHECKED);
	}

	if (uMsg == WM_HSCROLL)
	{
		HWND swnd = (HWND) lParam;
		int t=(int)SendMessageW(swnd,TBM_GETPOS,0,0);
		if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER3))
		{
			config_saref=(unsigned char)(4-t);
			SetDlgItemTextW(hwndDlg,IDC_RRATE,getStringW(text_ids[config_saref-1],NULL,0));
		}

		if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER2))
		{
			config_sa_peak_falloff = (unsigned char)t;
		}

		if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER1))
		{
			config_safalloff= (unsigned char)t;
		}
	}

	if (uMsg == WM_COMMAND)
		switch (LOWORD(wParam))
		{
			case IDC_RADIO1:
			case IDC_RADIO2:
			case IDC_RADIO3:
				if (IsDlgButtonChecked(hwndDlg,IDC_RADIO1)) config_sa=1;
				else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO2)) config_sa=2;
				else config_sa=0;
				sa_setthread(config_sa);
				break;

			case IDC_RADIO4:
			case IDC_RADIO5:
			case IDC_RADIO6:
				config_safire &= ~3;
				if (IsDlgButtonChecked(hwndDlg,IDC_RADIO5)) config_safire |= 1; 
				else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO6)) config_safire |= 2; 
				break;

			case IDC_CHECK1:
				config_sa_peaks = IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
				break;

			case IDC_RADIO7:
			case IDC_RADIO8:
				if (IsDlgButtonChecked(hwndDlg,IDC_RADIO7)) config_safire|=32;
				else config_safire&=~32;
				break;

			case IDC_RADIO9:
			case IDC_RADIO10:
			case IDC_RADIO11:
				config_safire &= ~(3<<2);
				if (IsDlgButtonChecked(hwndDlg,IDC_RADIO10)) config_safire |= 1<<2; 
				else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO11)) config_safire |= 2<<2; 
				break;
	}

	const int controls[] = 
	{
		IDC_SLIDER1,
		IDC_SLIDER2,
		IDC_SLIDER3,
	};
	if (FALSE != DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	return FALSE;
} // vis options

multiPage classicPages[] = {
	{IDD_CLASSIC_UI, ClassicUIProc},
	{IDD_CLASSIC_VIS, ClassicVisProc},
};

INT_PTR CALLBACK classicSkinProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			TCITEMW item = {0};
			HWND tabwnd=GetDlgItem(hwndDlg,IDC_TAB1);
			item.mask=TCIF_TEXT;
			item.pszText=getStringW(IDS_P_CLASSICUI,NULL,0);
			SendMessageW(tabwnd, TCM_INSERTITEMW, 0, (LPARAM)&item);
			item.pszText=getStringW(IDS_P_CLASSICVIS,NULL,0);
			SendMessageW(tabwnd, TCM_INSERTITEMW, 1, (LPARAM)&item);

			TabCtrl_SetCurSel(tabwnd,config_last_classic_skin_page);
			subWnd = _dosetsel(hwndDlg,subWnd,&config_last_classic_skin_page,classicPages,sizeof(classicPages)/sizeof(classicPages[0]));
		}
			return 0;

		case WM_NOTIFY:
		{
			LPNMHDR p=(LPNMHDR)lParam;
			if (p->idFrom == IDC_TAB1 && p->code == TCN_SELCHANGE)
				subWnd = _dosetsel(hwndDlg,subWnd,&config_last_classic_skin_page,classicPages,sizeof(classicPages)/sizeof(classicPages[0]));
		}
			return 0;

		case WM_DESTROY:
			subWnd=NULL;
			return 0;
	}
	return 0;
}