#include "./fileview.h"
#include "./fileview_internal.h"
#include "./resource.h"
#include "../nu/menushortcuts.h"
#include <windowsx.h>
#include <strsafe.h>

#define FVTOOLBAR_DATAW		L"FVTOOLBAR"

typedef struct _FVTOOLBAR
{
	HMENU hMenu;

} FVTOOLBAR;

#define GetToolbar(__hwnd) ((FVTOOLBAR*)GetPropW((__hwnd), FVTOOLBAR_DATAW))

static INT_PTR CALLBACK FileViewToolbar_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static HMLIMGLST hmlilModes = NULL;

HWND FileViewToolbar_Create(HWND hwndParent)
{
	HWND hwnd = WASABI_API_CREATEDIALOGPARAMW(IDD_FILEVIEW_TOOLBAR, hwndParent, FileViewToolbar_DialogProc, 0L);
	return hwnd;
}

static LRESULT FileViewToolbar_NotifyParent(HWND hdlg, UINT uCode, NMHDR *phdr)
{
	HWND hParent = GetParent(hdlg);
	if (!phdr || !hParent) return 0L;
	phdr->code		= uCode;
	phdr->hwndFrom	= hdlg;
	phdr->idFrom		= GetDlgCtrlID(hdlg);
	return SendMessageW(hParent, WM_NOTIFY, (WPARAM)phdr->idFrom, (LPARAM)phdr);
}

static void FileViewToolbar_LoadImages()
{
	MLIMAGESOURCE_I mlis;

	if (NULL != hmlilModes) return;

	ZeroMemory(&mlis, sizeof(MLIMAGESOURCE_I));
	mlis.type		= SRC_TYPE_PNG;
	mlis.hInst		= plugin.hDllInstance;

	hmlilModes = MLImageListI_Create(18, 12, MLILC_COLOR32, 3, 2, 3, hmlifMngr);
	if (NULL != hmlilModes)
	{
		INT imageList[] = { IDB_FILEVIEW_ICON, IDB_FILEVIEW_LIST, IDB_FILEVIEW_DETAIL };
		for(int i = 0; i < sizeof(imageList)/sizeof(imageList[0]); i++) 
		{
			mlis.lpszName	= MAKEINTRESOURCEW(imageList[i]);
			MLImageListI_Add(hmlilModes, &mlis, MLIF_BUTTONBLENDPLUSCOLOR_UID, imageList[i]);
		}
	}
}

static void FileViewToolbar_LayoutWindows(HWND hdlg, BOOL bRedraw)
{
	INT buttonList[] = { IDC_BTN_VIEW_ICON, IDC_BTN_VIEW_LIST, IDC_BTN_VIEW_DETAIL, IDC_BTN_ARRANGEBY, IDC_BTN_OPTIONS, };
	HDWP hdwp;
	DWORD flags, size;
	RECT rc;

	if (!GetClientRect(hdlg, &rc)) return;

	flags = SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW;

	hdwp = BeginDeferWindowPos(sizeof(buttonList)/sizeof(buttonList[0]));
	if (!hdwp) return;

	for(int i =0; i < sizeof(buttonList)/sizeof(buttonList[0]); i++)
	{
		HWND hctrl = GetDlgItem(hdlg, buttonList[i]);
		if (NULL != hctrl)
		{
			size = MLSkinnedButton_GetIdealSize(hctrl, NULL);
			INT width = LOWORD(size);
			hdwp = DeferWindowPos(hdwp, hctrl, NULL, 0, 0, width, rc.bottom - rc.top, flags);
		}
	}

	EndDeferWindowPos(hdwp);

	SetWindowPos(hdlg, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE |
					SWP_NOZORDER | SWP_FRAMECHANGED | ((bRedraw) ? 0 : SWP_NOREDRAW));
}


static BOOL FileViewToolbar_OnInitDialog(HWND hdlg, HWND hwndFocus, LPARAM lParam)
{
	FVTOOLBAR *ptb;
	ptb = (FVTOOLBAR*)calloc(1, sizeof(FVTOOLBAR));
	if (ptb)
	{
		if (!SetPropW(hdlg, FVTOOLBAR_DATAW, ptb))
		{
			free(ptb);
			ptb = NULL;
		}
	}

	if (!ptb) 
	{
		DestroyWindow(hdlg);
		return 0;
	}

	FileViewToolbar_LoadImages();

	HWND hctrl;
	SkinWindowEx(hdlg, SKINNEDWND_TYPE_AUTO, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT);

	hctrl = GetDlgItem(hdlg, IDC_BTN_VIEW_ICON);
	SkinWindowEx(hctrl, SKINNEDWND_TYPE_BUTTON, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT | SWBS_TOOLBAR);
	MLSkinnedButton_SetImageList(hctrl, hmlilModes, 0, 0, 0, 0);

	hctrl = GetDlgItem(hdlg, IDC_BTN_VIEW_LIST);
	SkinWindowEx(hctrl, SKINNEDWND_TYPE_BUTTON, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT | SWBS_TOOLBAR);
	MLSkinnedButton_SetImageList(hctrl, hmlilModes, 1, 1, 1, 1);

	hctrl = GetDlgItem(hdlg, IDC_BTN_VIEW_DETAIL);
	SkinWindowEx(hctrl, SKINNEDWND_TYPE_BUTTON, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT | SWBS_TOOLBAR);
	MLSkinnedButton_SetImageList(hctrl, hmlilModes, 2, 2, 2, 2);

	hctrl = GetDlgItem(hdlg, IDC_BTN_ARRANGEBY);
	SkinWindowEx(hctrl, SKINNEDWND_TYPE_BUTTON, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT | SWBS_DROPDOWNBUTTON | SWBS_TOOLBAR);
	hctrl = GetDlgItem(hdlg, IDC_BTN_OPTIONS);
	SkinWindowEx(hctrl, SKINNEDWND_TYPE_BUTTON, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT | SWBS_DROPDOWNBUTTON | SWBS_TOOLBAR);

	FileViewToolbar_LayoutWindows(hdlg, FALSE);

	return FALSE;
}

static void FileViewToolbar_OnDestroy(HWND hdlg)
{
	FVTOOLBAR *ptb = GetToolbar(hdlg);
	if (ptb)
	{
		RemovePropW(hdlg, FVTOOLBAR_DATAW);
		if (ptb->hMenu) DestroyMenu(ptb->hMenu);
		free(ptb);
	}
}

static void FileViewToolbar_OnWindowPosChanged(HWND hdlg, WINDOWPOS *pwp)
{
	HWND hctrl;
	RECT rc, rw;
	UINT flags;
	LONG right, left;
	DWORD  ws;
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;

	GetClientRect(hdlg, &rc);
	right = rc.right -2;
	left = rc.left + 2;
	rc.bottom -= 2;

	HDWP hdwp = BeginDeferWindowPos(4);
	if (NULL == hdwp) return;

	flags = SWP_NOACTIVATE | SWP_NOZORDER | ((SWP_NOREDRAW | SWP_NOCOPYBITS) & pwp->flags);

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_BTN_VIEW_ICON)) && GetWindowRect(hctrl, &rw))
	{
		hdwp = DeferWindowPos(hdwp, hctrl, NULL, left, rc.top, rw.right - rw.left, rc.bottom - rc.top, flags); 
		left += (rw.right - rw.left) + 2;
	}
	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_BTN_VIEW_LIST)) && GetWindowRect(hctrl, &rw))
	{
		hdwp = DeferWindowPos(hdwp, hctrl, NULL, left, rc.top, rw.right - rw.left, rc.bottom - rc.top, flags); 
		left += (rw.right - rw.left) + 2;
	}
	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_BTN_VIEW_DETAIL)) && GetWindowRect(hctrl, &rw))
	{
		hdwp = DeferWindowPos(hdwp, hctrl, NULL, left, rc.top, rw.right - rw.left, rc.bottom - rc.top, flags); 
		left += (rw.right - rw.left) + 2;
	}

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_BTN_OPTIONS)) && GetWindowRect(hctrl, &rw))
	{
		right -= (rw.right - rw.left);
		ws = GetWindowLongPtrW(hctrl, GWL_STYLE);
		if (right < (left + 8)) { if (WS_VISIBLE & ws) ShowWindow(hctrl, SW_HIDE);}
		else { if (0 == (WS_VISIBLE & ws)) ShowWindow(hctrl, SW_SHOWNA); }
		hdwp = DeferWindowPos(hdwp, hctrl, NULL, right, rc.top, rw.right - rw.left, rc.bottom - rc.top, flags);
		right -= 4;
	}
	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_BTN_ARRANGEBY)) && GetWindowRect(hctrl, &rw))
	{
		right -= (rw.right - rw.left);
		ws = GetWindowLongPtrW(hctrl, GWL_STYLE);
		if (right < (left + 8)) { if (WS_VISIBLE & ws) ShowWindow(hctrl, SW_HIDE);}
		else { if (0 == (WS_VISIBLE & ws)) ShowWindow(hctrl, SW_SHOWNA); }
		hdwp = DeferWindowPos(hdwp, hctrl, NULL, right, rc.top, rw.right - rw.left, rc.bottom - rc.top, flags);
	}

	EndDeferWindowPos(hdwp);
}

static INT FileViewToolbar_OnGetBestHeight(HWND hdlg)
{
	INT height = 0;
	HWND hctrl;

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_BTN_OPTIONS)))
	{
		DWORD sz = MLSkinnedButton_GetIdealSize(hctrl, NULL);
		height = HIWORD(sz);
	}

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_BTN_VIEW_ICON)))
	{
		DWORD sz = MLSkinnedButton_GetIdealSize(hctrl, NULL);
		if (height < HIWORD(sz)) height = HIWORD(sz);
	}

	height += 1;
	if (height < 12) height = 12;
	return height;
}

static void FolderBrowserToolbar_NotifyViewSwitch(HWND hdlg)
{
	INT nCmd = 0;
	if (BST_CHECKED == IsDlgButtonChecked(hdlg, IDC_BTN_VIEW_LIST)) nCmd = ID_FILEVIEW_SETMODE_LIST;
	else if (BST_CHECKED == IsDlgButtonChecked(hdlg, IDC_BTN_VIEW_ICON)) nCmd = ID_FILEVIEW_SETMODE_ICON;
	else if (BST_CHECKED == IsDlgButtonChecked(hdlg, IDC_BTN_VIEW_DETAIL)) nCmd = ID_FILEVIEW_SETMODE_DETAIL;
	SendMessageW(GetParent(hdlg), WM_COMMAND, MAKEWPARAM(nCmd, 0), 0L);
}

static void FileViewToolbar_InvertParentStyle(HWND hdlg, UINT style)
{
	HWND hParent = GetParent(hdlg);
	if (hParent) 
	{
		FileView_SetStyle(hParent, FileView_GetStyle(hParent) ^ style, style);
		FileView_Refresh(hParent, FALSE);
	}
}

static void FileViewToolbar_DisplayOptionsMenu(HWND hdlg, HWND hButton)
{
	RECT r;
	if (!hButton  || !GetWindowRect(hButton, &r))
	{
		GetCursorPos((POINT*)&r);
		SetRect(&r, r.left, r.top, r.left, r.top);
	}

	if (hButton) MLSkinnedButton_SetDropDownState(hButton, TRUE);

	FileView_DisplayPopupMenu(GetParent(hdlg), FVMENU_OPTIONS,
		TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_TOPALIGN | TPM_RIGHTALIGN | TPM_VERPOSANIMATION,
		*(((POINT*)&r) + 1));

	if (hButton) MLSkinnedButton_SetDropDownState(hButton, FALSE);

}

static void FileViewToolbar_DisplayArrangeByMenu(HWND hdlg, HWND hButton)
{	
	RECT r;
	if (!hButton  || !GetWindowRect(hButton, &r))
	{
		GetCursorPos((POINT*)&r);
		SetRect(&r, r.left, r.top, r.left, r.top);
	}
	
	if (hButton) MLSkinnedButton_SetDropDownState(hButton, TRUE);
	
	FileView_DisplayPopupMenu(GetParent(hdlg),
			FVMENU_ARRANGEBY, TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_TOPALIGN | TPM_RIGHTALIGN | TPM_VERPOSANIMATION,
			*(((POINT*)&r) + 1));

	if (hButton) MLSkinnedButton_SetDropDownState(hButton, FALSE);
}

static void FileViewToolbar_OnCommand(HWND hdlg, INT eventId, INT ctrlId, HWND hwndCtrl)
{
	switch (ctrlId)
	{
		case IDC_BTN_VIEW_ICON:
		case IDC_BTN_VIEW_LIST:
		case IDC_BTN_VIEW_DETAIL:
			switch(eventId)
			{
				case BN_CLICKED: FolderBrowserToolbar_NotifyViewSwitch(hdlg); break;
			}
			break;
		case IDC_BTN_OPTIONS:
			switch(eventId)
			{
				case MLBN_DROPDOWN: FileViewToolbar_DisplayOptionsMenu(hdlg, hwndCtrl); break;
			}
			break;
		case IDC_BTN_ARRANGEBY:
			switch(eventId)
			{
				case MLBN_DROPDOWN: FileViewToolbar_DisplayArrangeByMenu(hdlg, hwndCtrl); break;
			}
			break;
	}
}

static void FileViewToolbar_OnSetStyle(HWND hdlg, UINT uStyle, UINT uStyleMask)
{
	if (FVS_VIEWMASK & uStyleMask)
	{
		UINT idc = ((UINT)-1);
		switch(FVS_VIEWMASK & uStyle)
		{
			case FVS_LISTVIEW:	idc = IDC_BTN_VIEW_LIST; break;
			case FVS_ICONVIEW:	idc = IDC_BTN_VIEW_ICON; break;
			case FVS_DETAILVIEW:	idc = IDC_BTN_VIEW_DETAIL; break;
		}
		CheckRadioButton(hdlg, IDC_BTN_VIEW_ICON, IDC_BTN_VIEW_DETAIL, idc);
	}
}

static INT_PTR CALLBACK FileViewToolbar_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return FileViewToolbar_OnInitDialog(hdlg, (HWND)wParam, lParam);
		case WM_DESTROY:				FileViewToolbar_OnDestroy(hdlg); return TRUE;
		case WM_WINDOWPOSCHANGED:	FileViewToolbar_OnWindowPosChanged(hdlg, (WINDOWPOS*)lParam); return TRUE;
		case WM_COMMAND:				FileViewToolbar_OnCommand(hdlg, HIWORD(wParam), LOWORD(wParam), (HWND)lParam); return TRUE;
		case FVM_GETIDEALHEIGHT:		SetWindowLongPtrW(hdlg, DWLP_MSGRESULT, FileViewToolbar_OnGetBestHeight(hdlg)); return TRUE;
		case WM_SETFONT:				FileViewToolbar_LayoutWindows(hdlg, LOWORD(lParam)); return TRUE;
		case FVM_SETSTYLE:			FileViewToolbar_OnSetStyle(hdlg, (UINT)lParam, (UINT)wParam); return TRUE;
	}
	return 0;
}