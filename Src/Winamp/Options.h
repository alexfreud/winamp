#ifndef NULLSOFT_OPTIONSH
#define NULLSOFT_OPTIONSH
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

#include <shlobj.h>

typedef struct { int id, id2; } hi;

typedef struct { UINT id; WNDPROC proc; } multiPage;
HWND _dosetsel(HWND hwndDlg, HWND subwnd, int* last_page, multiPage* pages, int numpages);

INT_PTR CALLBACK PlaybackProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
//INT_PTR CALLBACK StationInfoProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DspProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GenProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK VisProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK VideoProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK PlaybackOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK TitleProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK SetupProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK FtypeProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK SkinProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK classicSkinProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK OutputProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK InputProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK PlugProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK LangProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

void do_help(HWND hwnd, UINT id, HWND hTooltipWnd);
#ifdef DO_COLORS
#define C_BLAH \
	if (uMsg == WM_CTLCOLORDLG) return GetStockObject(DKGRAY_BRUSH);	\
	if (uMsg == WM_CTLCOLORSTATIC) { \
		SetTextColor(wParam,RGB(220,220,220)); SetBkColor(wParam,RGB(64,64,64)); \
		return GetStockObject(DKGRAY_BRUSH);} \
	if (uMsg == WM_CTLCOLORLISTBOX) { \
		SetTextColor(wParam,RGB(0,220,0)); SetBkColor(wParam,RGB(0,0,0)); \
		return GetStockObject(BLACK_BRUSH);} \
	if (uMsg == WM_CTLCOLOREDIT) { \
		SetTextColor(wParam,RGB(0,220,0)); SetBkColor(wParam,RGB(0,0,0)); \
		return GetStockObject(BLACK_BRUSH);}
#else
#define C_BLAH
#endif

#define DO_HELP()	\
	static HWND hTooltipWnd;	\
	C_BLAH	\
	if (uMsg == WM_HELP) {		\
		HELPINFO *hi=(HELPINFO *)(lParam); \
		if (hi->iContextType == HELPINFO_WINDOW) { do_help(hwndDlg,hi->iCtrlId,hTooltipWnd);}	\
		return TRUE;	\
	} \
	if (uMsg == WM_NOTIFY) { LPNMHDR t=(LPNMHDR)lParam; if (t->code == TTN_POP) { SendMessageW(hTooltipWnd,TTM_SETDELAYTIME,TTDT_INITIAL,1000); SendMessageW(hTooltipWnd,TTM_SETDELAYTIME,TTDT_RESHOW,1000);  } }	\
	if (uMsg == WM_DESTROY && IsWindow(hTooltipWnd)) { DestroyWindow(hTooltipWnd); hTooltipWnd=NULL; }	\
	if (uMsg == WM_INITDIALOG) {	\
		int x; \
		hTooltipWnd = CreateWindowW(TOOLTIPS_CLASSW,(LPCWSTR)NULL,TTS_ALWAYSTIP, \
								   CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT, hwndDlg,NULL,GetModuleHandle(NULL),NULL);	\
	    SendMessageW(hTooltipWnd,TTM_SETMAXTIPWIDTH,0,587); \
		SendMessageW(hTooltipWnd,TTM_SETDELAYTIME,TTDT_INITIAL,1000);	\
		SendMessageW(hTooltipWnd,TTM_SETDELAYTIME,TTDT_RESHOW,1000);	\
		for (x = 0; x < sizeof(helpinfo)/sizeof(helpinfo[0]); x ++) { \
			TOOLINFOW ti; ti.cbSize = sizeof(ti); ti.uFlags = TTF_SUBCLASS|TTF_IDISHWND;	\
			ti.uId=(UINT_PTR)GetDlgItem(hwndDlg,helpinfo[x].id); ti.hwnd=hwndDlg; ti.lpszText=getStringW(helpinfo[x].id2,NULL,0);	\
			SendMessageW(hTooltipWnd,TTM_ADDTOOLW,0,(LPARAM) &ti);	\
		}	\
	}

#define ListView_InsertColumnW(hwnd, iCol, pcol) \
    (int)SNDMSG((hwnd), LVM_INSERTCOLUMNW, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMNW *)(pcol))

#define ListView_InsertItemW(hwnd, pitem)   \
    (int)SNDMSG((hwnd), LVM_INSERTITEMW, 0, (LPARAM)(const LV_ITEMW *)(pitem))

#define ListView_SetItemTextW(hwndLV, i, iSubItem_, pszText_) \
{ LV_ITEMW _macro_lvi;\
  _macro_lvi.iSubItem = (iSubItem_);\
  _macro_lvi.pszText = (pszText_);\
  SNDMSG((hwndLV), LVM_SETITEMTEXTW, (WPARAM)(i), (LPARAM)(LV_ITEMW *)&_macro_lvi);\
}

#define ListView_SetItemW(hwnd, pitem) \
    (BOOL)SNDMSG((hwnd), LVM_SETITEMW, 0, (LPARAM)(LV_ITEMW *)(pitem))

#define ListView_GetItemW(hwnd, pitem) \
    (BOOL)SNDMSG((hwnd), LVM_GETITEMW, 0, (LPARAM)(LV_ITEMW *)(pitem))

#define ListView_GetItemTextW(hwndLV, i, iSubItem_, pszText_, cchTextMax_) \
{ LV_ITEMW _macro_lvi;\
  _macro_lvi.iSubItem = (iSubItem_);\
  _macro_lvi.cchTextMax = (cchTextMax_);\
  _macro_lvi.pszText = (pszText_);\
  SNDMSG((hwndLV), LVM_GETITEMTEXTW, (WPARAM)(i), (LPARAM)(LV_ITEMW *)&_macro_lvi);\
}


#endif
#ifdef __cplusplus
}
#endif