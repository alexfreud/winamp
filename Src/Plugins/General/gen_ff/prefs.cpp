#include "precomp__gen_ff.h"
#include "main.h"
#include "resource.h"
#include "prefs.h"
#include "wa2cfgitems.h"
#include "wa2frontend.h"
#include "../Agave/Language/api_language.h"
#include "gen.h"
#include <commctrl.h>
#include <windowsx.h>

void turnonoff(HWND wnd, int *t, int n, int v) {
  for (int i=0;i<n;i++) {
    EnableWindow(GetDlgItem(wnd, t[i]), v);
  }
}

extern void initFFApi();
extern Wa2CfgItems *cfgitems;
extern HINSTANCE hInstance;

_int last_page(L"Last Page", 0);
Wa2FFOptions *ffoptions = NULL;
HWND subWnd = NULL, tabwnd = NULL;
int subWndId = -1;
extern int m_are_we_loaded;
int toggle_from_wa2 = 0;

void _dosetsel(HWND hwndDlg) 
{
	tabwnd = GetDlgItem(hwndDlg,IDC_TAB1);
	int sel=TabCtrl_GetCurSel(tabwnd);

	if (sel >= 0 && (sel != last_page.getValueAsInt() || !subWnd))
	{
	    last_page.setValueAsInt(sel);
		if (subWnd) DestroyWindow(subWnd);
		subWnd = NULL;
		subWndId = -1;

		UINT t=0;
		DLGPROC p=0;
		switch (sel) 
		{
			case 0: t=IDD_PREFS_GENERAL;	p=ffPrefsProc1; subWndId = 0;	break;
			case 1: t=IDD_PREFS_WINDOWS;	p=ffPrefsProc4; subWndId = 1;	break;
			case 2: t=IDD_PREFS_FONTS;		p=ffPrefsProc2; subWndId = 2;	break;
			case 3: t=IDD_PREFS_THEMES;		p=ffPrefsProc3; subWndId = 3;	break;
			case 4: t=IDD_PREFS_SKIN;		p=ffPrefsProc5; subWndId = 5;	break;
		}
		if (t) subWnd=WASABI_API_CREATEDIALOGW(t,hwndDlg,p);

	    if (IsWindow(subWnd))
		{
			RECT r = {0};
			GetClientRect(tabwnd,&r);
			TabCtrl_AdjustRect(tabwnd,FALSE,&r);
			SetWindowPos(subWnd,HWND_TOP,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOACTIVATE);
			ShowWindow(subWnd,SW_SHOWNA);
		}

		if(!SendMessageW(plugin.hwndParent,WM_WA_IPC,IPC_ISWINTHEMEPRESENT,IPC_USE_UXTHEME_FUNC))
		{
			SendMessageW(plugin.hwndParent,WM_WA_IPC,(WPARAM)tabwnd,IPC_USE_UXTHEME_FUNC);
			SendMessageW(plugin.hwndParent,WM_WA_IPC,(WPARAM)subWnd,IPC_USE_UXTHEME_FUNC);
		}
	}
}

#define TabCtrl_InsertItemW(hwnd, iItem, pitem)   \
    (int)SNDMSG((hwnd), TCM_INSERTITEMW, (WPARAM)(int)(iItem), (LPARAM)(const TC_ITEMW *)(pitem))

// frame proc
INT_PTR CALLBACK ffPrefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			if (WASABI_API_APP == NULL)
			{
				// wasabi is not initialized ! we need to init before we can access cfgitems otherwise we'd have
				// to mirror their values with winamp.ini and that'd be seriously crappy
				initFFApi();
			}

			if (!ffoptions)
				ffoptions = new Wa2FFOptions();

			TCITEMW item = {0};
			HWND tabwnd=GetDlgItem(hwndDlg,IDC_TAB1);
			item.mask=TCIF_TEXT;
			item.pszText=WASABI_API_LNGSTRINGW(IDS_GENERAL);
			TabCtrl_InsertItemW(tabwnd,0,&item);
			item.pszText=WASABI_API_LNGSTRINGW(IDS_WINDOW_SETTINGS);
			TabCtrl_InsertItemW(tabwnd,1,&item);
			item.pszText=WASABI_API_LNGSTRINGW(IDS_FONT_RENDERING);
			TabCtrl_InsertItemW(tabwnd,2,&item);
			if (m_are_we_loaded)
			{
				item.pszText=WASABI_API_LNGSTRINGW(IDS_COLOR_THEMES);
				TabCtrl_InsertItemW(tabwnd,3,&item);
				item.pszText=WASABI_API_LNGSTRINGW(IDS_CURRENT_SKIN);
				TabCtrl_InsertItemW(tabwnd,4,&item);
			}

			TabCtrl_SetCurSel(tabwnd,last_page.getValueAsInt());
			_dosetsel(hwndDlg);
		}
		return 0;

		case WM_NOTIFY:
		{
			LPNMHDR p=(LPNMHDR) lParam;
			if (p->idFrom == IDC_TAB1 && p->code == TCN_SELCHANGE)
			{
				_dosetsel(hwndDlg);
				return 0;
			}
		}
			break;

		case WM_DESTROY:
			subWnd=NULL;
			return 0;
	}
	return 0;
}

Wa2FFOptions::Wa2FFOptions() : CfgItemI(L"Winamp5", Wa2FFOptionsGuid) {
  registerAttribute(&last_page);
}

int ComboBox_AddStringW(HWND list, const wchar_t *string)
{
	return SendMessageW(list, CB_ADDSTRING, 0, (LPARAM)string);
}