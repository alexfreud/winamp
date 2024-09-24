#include "main.h"
#include "./scanfolderbrowser.h"
#include "resource.h"

/// New controls
#define IDC_CHK_BGSCAN		0x1980
#define IDC_TLB_BOOKMARKS	0x1978
#define TOOLBAR_BTN_STARTID	0x1000

#define PATHTYPE_CSIDL		0
#define PATHTYPE_STRING		1
#define PATHTYPE_PIDL		2

typedef struct __FOLDER
{
	int pathtype;
	INT_PTR	path;
	LPCWSTR	caption;	// if NULL display name will be used
	LPCWSTR	tooltip;	// if NULL display name will be used
} FOLDER;

typedef struct _FBUTTON
{
	LPITEMIDLIST pidl;
	LPWSTR caption;	
	LPWSTR tooltip;	
	INT iImage;
} FBUTTON;

static const FOLDER BOOKMARKS[] = 
{
	{PATHTYPE_STRING, (INT_PTR)L"C:\\Program Files\\Winamp", NULL, NULL },
	{PATHTYPE_CSIDL, CSIDL_MYMUSIC, NULL, NULL },
	{PATHTYPE_CSIDL, CSIDL_MYVIDEO, NULL, NULL },
	{PATHTYPE_CSIDL, CSIDL_PERSONAL, NULL, NULL },
	{PATHTYPE_CSIDL, CSIDL_DESKTOP, NULL, NULL },
	{PATHTYPE_CSIDL, CSIDL_DRIVES, NULL, NULL},
	{PATHTYPE_CSIDL, CSIDL_NETWORK, NULL, NULL },
};

static HIMAGELIST hSysImageList;

//// XP THEME - BEGIN 
typedef HANDLE HTHEME;

typedef HRESULT (WINAPI *XPT_CLOSETHEMEDATA)(HTHEME);
typedef BOOL (WINAPI *XPT_ISAPPTHEMED)(void);
typedef HTHEME (WINAPI *XPT_OPENTHEMEDATA)(HWND, LPCWSTR);
typedef HRESULT (WINAPI *XPT_GETTHEMECOLOR)(HTHEME, INT, INT, INT, COLORREF*);

#define GP_BORDER 1
#define BSS_FLAT 1
#define TMT_BORDERCOLOR 3801

static XPT_CLOSETHEMEDATA xpCloseThemeData;
static XPT_ISAPPTHEMED xpIsAppThemed;
static XPT_OPENTHEMEDATA xpOpenThemeData;
static XPT_GETTHEMECOLOR xpGetThemeColor;

static HINSTANCE xpThemeDll = NULL;

static BOOL LoadXPTheme(void);
static void UnloadXPTheme(void);
//// XP THEME - END

static HBRUSH GetBorderBrush(HWND hwnd);

static void Initialize(ScanFolderBrowser *browser, BOOL showBckScan, BOOL checkBckScan)
{
	browser->buttonsCount = 0;
	browser->buttons = NULL;
	browser->bkScanChecked = checkBckScan;
	browser->bkScanShow = showBckScan;
	browser->pac = NULL;
	browser->pacl2 = NULL;

	HRESULT result = CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER, IID_IAutoComplete, (LPVOID*)&browser->pac);
	if (S_OK == result)
	{
		IAutoComplete2 *pac2;
		if (SUCCEEDED(browser->pac->QueryInterface(IID_IAutoComplete2, (LPVOID*)&pac2)))
		{
			pac2->SetOptions(ACO_AUTOSUGGEST | ACO_AUTOAPPEND | ACO_USETAB | 0x00000020/*ACF_UPDOWNKEYDROPSLIST*/);
		}
		result = CoCreateInstance(CLSID_ACListISF, NULL, CLSCTX_INPROC_SERVER, IID_IACList2, (LPVOID*)&browser->pacl2);
		if (S_OK == result) browser->pacl2->SetOptions(ACLO_FILESYSDIRS);
		else  { browser->pac->Release(); browser->pac = NULL; }
	}

	lstrcpynW(browser->selectionPath, WASABI_API_APP->path_getWorkingPath(), MAX_PATH);

	browser->SetCaption(WASABI_API_LNGSTRINGW(IDS_ADD_MEDIA_TO_LIBRARY_));
	browser->SetTitle(WASABI_API_LNGSTRINGW(IDS_SELECT_FOLDER_TO_ADD_TO_WINAMP_MEDIA_LIBRARY));
	browser->SetSelection(browser->selectionPath);
	browser->SetFlags(BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_VALIDATE | BIF_USENEWUI | BIF_NONEWFOLDERBUTTON | BIF_NEWDIALOGSTYLE);
	browser->LoadBookmarks();
}

ScanFolderBrowser::ScanFolderBrowser(BOOL showBckScanOption)
{
	Initialize(this, showBckScanOption, FALSE);
}

ScanFolderBrowser::ScanFolderBrowser(void)
{
	Initialize(this, TRUE, FALSE);
}

ScanFolderBrowser::~ScanFolderBrowser(void)
{
	if (pacl2) pacl2->Release();
	if (pac) pac->Release();
	FreeBookmarks();
}

void ScanFolderBrowser::LoadBookmarks(void)
{
	FreeBookmarks();
	INT count = sizeof(BOOKMARKS)/sizeof(FOLDER);
	if (!count) return;

	buttons = (FBUTTON*)calloc(count, sizeof(FBUTTON));
	if (!buttons) return;
	buttonsCount = 0;

	for (int i = 0; i < count; i++)
	{
		const FOLDER *pfolder = &BOOKMARKS[i];
		FBUTTON *pfb = &buttons[buttonsCount];
		switch(BOOKMARKS[i].pathtype)
		{
			case PATHTYPE_PIDL:  
				pfb->pidl = ILClone((LPITEMIDLIST)pfolder->path);  // need to copy it
				break;
			case PATHTYPE_CSIDL:  
				if (S_OK != SHGetSpecialFolderLocation(NULL, (INT)pfolder->path, &pfb->pidl)) pfb->pidl = NULL;
				break;
			case PATHTYPE_STRING:
				if (S_OK != ParseDisplayName((LPCWSTR)pfolder->path, NULL, &pfb->pidl, 0, NULL))pfb->pidl = NULL;
				break;
		}
		if (!buttons[buttonsCount].pidl)
		{
			continue;
		}

		SHFILEINFOW shfi = {0};
		hSysImageList = (HIMAGELIST)SHGetFileInfoW((LPCWSTR)pfb->pidl, 0, &shfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_LARGEICON | SHGFI_SYSICONINDEX | SHGFI_PIDL);
		pfb->caption = _wcsdup((pfolder->caption) ? pfolder->caption : shfi.szDisplayName);
		pfb->tooltip = _wcsdup((pfolder->tooltip) ? pfolder->tooltip : shfi.szDisplayName);
		pfb->iImage = shfi.iIcon;
		buttonsCount++;
	}
}

void ScanFolderBrowser::FreeBookmarks(void)
{
	if (buttons)
	{
		FBUTTON *pbtn = buttons;
		while(buttonsCount--)
		{
			if (pbtn->caption) { free(pbtn->caption); pbtn->caption = NULL; }
			if (pbtn->tooltip) { free(pbtn->tooltip); pbtn->tooltip = NULL; }
			if (pbtn->pidl) { ILFree(pbtn->pidl); pbtn->pidl = NULL; }
			pbtn++;
		}
		free(buttons);
	}
	buttonsCount = 0;
}

void ScanFolderBrowser::OnInitialized(void)
{
	HWND ctrlWnd;

	if (!FindWindowExW(GetHandle(), NULL,L"SHBrowseForFolder ShellNameSpace Control",NULL))
	{
		RECT rw;
		int cx;
		GetWindowRect(GetHandle(), &rw);
		cx = rw.right - rw.left;
		SetWindowPos(GetHandle(), NULL, 0, 0, cx, rw.bottom - rw.top, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
		GetWindowRect(GetHandle(), &rw);
		ShrinkWindows((rw.right - rw.left) - cx);
	}
	SetOKText(WASABI_API_LNGSTRINGW(IDS_ADD));

	ShiftWindows(88);

	ctrlWnd = CreateWindowExW(WS_EX_NOPARENTNOTIFY , TOOLBARCLASSNAMEW, NULL, 
								WS_CHILD | WS_TABSTOP | 
								CCS_TOP | CCS_NODIVIDER | CCS_NOPARENTALIGN  | CCS_NORESIZE |  
								TBSTYLE_FLAT | TBSTYLE_TRANSPARENT | TBSTYLE_WRAPABLE | TBSTYLE_CUSTOMERASE | TBSTYLE_TOOLTIPS,
								12, 10, 84, 272, GetHandle(), (HMENU)IDC_TLB_BOOKMARKS, NULL, NULL);
	if (ctrlWnd)
	{
		::SendMessage(ctrlWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0L); 
		::SendMessage(ctrlWnd, TB_SETBUTTONSIZE, (WPARAM)0, MAKELPARAM(84, 50)); 
		::SendMessage(ctrlWnd, TB_SETBITMAPSIZE, (WPARAM)0, MAKELPARAM(32, 32)); 
		::SendMessage(ctrlWnd, TB_SETPADDING, (WPARAM)0, MAKELPARAM(8, 8)); 
		::SendMessage(ctrlWnd, TB_SETBUTTONWIDTH, (WPARAM)0, MAKELPARAM(84, 84)); 
		::SendMessage(ctrlWnd, TB_SETEXTENDEDSTYLE, (WPARAM)0, 0x0000080 /*TBSTYLE_EX_DOUBLEBUFFER*/); 
		::SendMessage(ctrlWnd, TB_SETMAXTEXTROWS, (WPARAM)2, 0L); 
		::SendMessage(ctrlWnd, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)hSysImageList);  

		LPTBBUTTON ptbb = (LPTBBUTTON)calloc(buttonsCount, sizeof(TBBUTTON));
		for (int i = 0; i < buttonsCount; i++)
		{
			ptbb[i].fsState = TBSTATE_ENABLED | TBSTATE_WRAP;
			ptbb[i].idCommand = TOOLBAR_BTN_STARTID + i;
			ptbb[i].fsStyle = TBSTYLE_CHECKGROUP; 
			ptbb[i].iString = (INT_PTR)buttons[i].caption;
			ptbb[i].iBitmap = (INT_PTR) buttons[i].iImage; 
		}
		::SendMessage(ctrlWnd, TB_ADDBUTTONSW, (WPARAM)buttonsCount,(LPARAM)ptbb); 
	}

	ctrlWnd = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"BUTTON",
							  WASABI_API_LNGSTRINGW(IDS_SCAN_FOLDER_IN_BACKGROUND), 
							  BS_AUTOCHECKBOX | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP,
							  0, 0, 100, 16, GetHandle(), (HMENU)IDC_CHK_BGSCAN, NULL, NULL);
	if (ctrlWnd)
	{
		::SendMessage(ctrlWnd, WM_SETFONT, (WPARAM)SendMessage(WM_GETFONT, 0, 0), FALSE);
		::SendMessage(ctrlWnd, BM_SETCHECK, (WPARAM) (bkScanChecked) ? BST_CHECKED : BST_UNCHECKED, 0L);
	}

	hbBorder = GetBorderBrush(GetHandle());
	RepositionWindows();

	ShowWindow(GetDlgItem(IDC_TLB_BOOKMARKS), SW_SHOWNORMAL);
	if (bkScanShow) ShowWindow(GetDlgItem(IDC_CHK_BGSCAN), SW_SHOWNORMAL);

	// edit box autocomplete chnages
	if(pac)	pac->Init(GetDlgItem(IDC_EDT_PATH), pacl2, NULL, NULL);

	FolderBrowseEx::OnInitialized();
}

void ScanFolderBrowser::OnSelectionChanged(LPCITEMIDLIST pidl)
{
	for(int i =0; i < buttonsCount; i++)
	{
		::SendMessage(GetDlgItem(IDC_TLB_BOOKMARKS), TB_CHECKBUTTON, (WPARAM)TOOLBAR_BTN_STARTID + i, ILIsEqual(pidl, buttons[i].pidl));
	}

	FolderBrowseEx::OnSelectionChanged(pidl);
}

BOOL ScanFolderBrowser::OnValidateFailed(LPCWSTR lpName)
{
	wchar_t buffer[2048] = {0}, titleStr[64] = {0};
    FolderBrowseEx::OnValidateFailed(lpName);
	wsprintfW(buffer, WASABI_API_LNGSTRINGW(IDS_FOLDER_NAME_IS_INCORRECT), lpName);
	MessageBoxW(GetHandle(), buffer, WASABI_API_LNGSTRINGW_BUF(IDS_INCORRECT_FOLDER_NAME,titleStr,64), MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
	return TRUE;
}

void ScanFolderBrowser::OnSelectionDone(LPCITEMIDLIST pidl)
{
	WCHAR pszPath[MAX_PATH] = {0};
	SHGetPathFromIDListW(pidl, pszPath);
	WASABI_API_APP->path_setWorkingPath(pszPath);
}

void ScanFolderBrowser::ShiftWindows(int cx)
{
	HWND hwnd = GetWindow(GetHandle(), GW_CHILD);
	while(hwnd)
	{
		RECT rw;
		UINT ctrlID = GetDlgCtrlID(hwnd);
		if (ctrlID != IDOK && ctrlID != IDCANCEL && ctrlID != IDC_SB_GRIPPER)
		{
			GetWindowRect(hwnd, &rw);
			MapWindowPoints(HWND_DESKTOP, GetHandle(), (LPPOINT)&rw, 2);
			SetWindowPos(hwnd, NULL, rw.left + cx, rw.top, (rw.right - (rw.left + cx)), rw.bottom - rw.top, SWP_NOACTIVATE | SWP_NOZORDER | ((ctrlID == IDC_LBL_FOLDER) ? SWP_NOSIZE : 0));
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}

void ScanFolderBrowser::ShrinkWindows(int cx)
{
	HWND hwnd = GetWindow(GetHandle(), GW_CHILD);
	while(hwnd)
	{
		UINT ctrlID = GetDlgCtrlID(hwnd);
		if (ctrlID != IDC_LBL_FOLDER) 
		{
			RECT rw;
			GetWindowRect(hwnd, &rw);
			MapWindowPoints(HWND_DESKTOP, GetHandle(), (LPPOINT)&rw, 2);
			SetWindowPos(hwnd, NULL, 
						rw.left + cx, rw.top, (rw.right - rw.left) + cx, rw.bottom - rw.top, 
						SWP_NOACTIVATE | SWP_NOZORDER | ((ctrlID == IDOK || ctrlID == IDCANCEL) ? SWP_NOSIZE : SWP_NOMOVE));
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}

void ScanFolderBrowser::RepositionWindows(void)
{
	RECT rwBtn, rwLV, rwCtrl;
	HWND pwnd, ctrlWnd;

	GetWindowRect(GetDlgItem(IDOK), &rwBtn);
	pwnd = FindWindowExW(GetHandle(), NULL,L"SHBrowseForFolder ShellNameSpace Control",NULL);

	GetWindowRect((pwnd) ? pwnd : GetDlgItem(IDC_TV_FOLDERS), &rwLV);
	ctrlWnd = GetDlgItem(IDC_CHK_BGSCAN);
    GetWindowRect(ctrlWnd, &rwCtrl);
	SetRect(&rwCtrl, rwLV.left, rwBtn.bottom - (rwCtrl.bottom - rwCtrl.top), rwBtn.left - 4 - rwLV.left, rwCtrl.bottom - rwCtrl.top);
	MapWindowPoints(HWND_DESKTOP, GetHandle(), (LPPOINT)&rwCtrl, 1);
	SetWindowPos(ctrlWnd, NULL, rwCtrl.left, rwCtrl.top, rwCtrl.right, rwCtrl.bottom, SWP_NOACTIVATE | SWP_NOZORDER);

	ctrlWnd = GetDlgItem(IDC_TLB_BOOKMARKS);
    GetWindowRect(ctrlWnd, &rwCtrl);
	GetWindowRect(GetDlgItem(IDC_LBL_CAPTION), &rwLV);
	SetWindowPos(ctrlWnd, NULL, 0, 0, rwCtrl.right - rwCtrl.left, rwBtn.bottom - rwLV.top, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
}

void ScanFolderBrowser::OnSize(UINT nType, int cx, int cy)
{
	FolderBrowseEx::DialogProc(WM_SIZE, nType, MAKELPARAM(cx, cy));
	
	RepositionWindows();
}

void ScanFolderBrowser::OnWindowPosChanging(WINDOWPOS *lpwp)
{
	if (lpwp->cx < 346 + 88) lpwp->cx = 346 + 88;
}

BOOL ScanFolderBrowser::OnNotify(UINT idCtrl, LPNMHDR pnmh, LRESULT *result)
{
	switch(pnmh->code)
	{
		case NM_CUSTOMDRAW:
			switch(pnmh->idFrom)
			{
				case IDC_TLB_BOOKMARKS:  
					*result =  OnToolBarCustomDraw((LPNMTBCUSTOMDRAW)pnmh);
					return TRUE;
			}
			break;
		case TTN_GETDISPINFOW: 
			OnToolTipGetDispInfo((LPNMTTDISPINFOW)pnmh);
			break;
	}
	return FALSE;
}

BOOL ScanFolderBrowser::OnCommand(UINT idCtrl, UINT idEvnt, HWND hwndCtrl)
{
	int tbid;
	tbid = idCtrl - TOOLBAR_BTN_STARTID;
	if (tbid >= 0 && tbid < buttonsCount)
	{
		LPITEMIDLIST pidl1 = buttons[tbid].pidl;
		SetExpanded(pidl1);
		SetSelection(pidl1);

		wchar_t test_path[MAX_PATH] = {0};
		EnableOK(SHGetPathFromIDListW(pidl1, test_path));
		return TRUE;
	}
	return FALSE;
}

LRESULT ScanFolderBrowser::OnToolBarCustomDraw(LPNMTBCUSTOMDRAW pnmcd)
{
	switch(pnmcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT: 
			FrameRect(pnmcd->nmcd.hdc, &pnmcd->nmcd.rc, hbBorder);
			InflateRect(&pnmcd->nmcd.rc, -1, -1);
			IntersectClipRect(pnmcd->nmcd.hdc, pnmcd->nmcd.rc.left, pnmcd->nmcd.rc.top, pnmcd->nmcd.rc.right, pnmcd->nmcd.rc.bottom);
			return CDRF_NOTIFYITEMDRAW;
		case CDDS_ITEMPREPAINT:
			InflateRect(&pnmcd->nmcd.rc, -1, 0);
			if (0 == pnmcd->nmcd.rc.top) pnmcd->nmcd.rc.top++;
			return CDRF_DODEFAULT;
	}
	return CDRF_DODEFAULT;
}

void ScanFolderBrowser::OnToolTipGetDispInfo(LPNMTTDISPINFOW lpnmtdi)
{
	int tbid;
	tbid = lpnmtdi->hdr.idFrom - TOOLBAR_BTN_STARTID;
	lpnmtdi->lpszText = (tbid >= 0 && tbid < buttonsCount) ? buttons[tbid].tooltip : L"";
}

INT_PTR ScanFolderBrowser::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;
	switch(uMsg)
	{
		case WM_WINDOWPOSCHANGING:	OnWindowPosChanging((WINDOWPOS*)lParam);
		case WM_SIZE:				OnSize(wParam, LOWORD(lParam), HIWORD(lParam)); return 0;
		case WM_NOTIFY:				
			if (OnNotify(wParam, (LPNMHDR)lParam, &result))
			{
				SetDialogResult(result);
				return TRUE;
			}
			break;
		case WM_COMMAND: 
			if (OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam)) return 0;
			break;
		case WM_DESTROY:
			if (hbBorder) DeleteObject(hbBorder);
			hbBorder = NULL;	
			bkScanChecked = (BST_CHECKED == IsDlgButtonChecked(GetHandle(), IDC_CHK_BGSCAN));
			break;
	}
	return FolderBrowseEx::DialogProc(uMsg, wParam, lParam);
}

static BOOL LoadXPTheme(void)
{
	xpThemeDll = LoadLibraryW(L"UxTheme.dll");
	if (!xpThemeDll) return FALSE;

	xpCloseThemeData = (XPT_CLOSETHEMEDATA) GetProcAddress(xpThemeDll, "CloseThemeData"); 
	xpIsAppThemed = (XPT_ISAPPTHEMED) GetProcAddress(xpThemeDll, "IsAppThemed");
	xpOpenThemeData = (XPT_OPENTHEMEDATA) GetProcAddress(xpThemeDll, "OpenThemeData");
	xpGetThemeColor = (XPT_GETTHEMECOLOR) GetProcAddress(xpThemeDll, "GetThemeColor");

	if (!xpCloseThemeData || !xpIsAppThemed || !xpOpenThemeData || !xpGetThemeColor) UnloadXPTheme();
	return (NULL != xpThemeDll);
}

static void UnloadXPTheme(void)
{
	xpCloseThemeData	= NULL; 
	xpIsAppThemed	= NULL;
	xpOpenThemeData	= NULL;
	xpGetThemeColor	= NULL;
	if (xpThemeDll)
	{
		FreeLibrary(xpThemeDll);
		xpThemeDll = NULL;
	}
}

static HBRUSH GetBorderBrush(HWND hwnd)
{
	HBRUSH hb;
	COLORREF clr;
	HRESULT result;

	clr = 0x00000;
	result = S_FALSE;
	if(LoadXPTheme())
	{
		if(xpIsAppThemed())
		{
			HTHEME ht;			
			ht = xpOpenThemeData(GetDlgItem(hwnd, IDC_EDT_PATH), L"Edit");
			if (ht)
			{
				result = xpGetThemeColor(ht, GP_BORDER, BSS_FLAT, TMT_BORDERCOLOR, &clr); 
				xpCloseThemeData(ht);
			}
		}
		UnloadXPTheme();
	}

	hb = CreateSolidBrush((S_OK == result) ? clr : GetSysColor(COLOR_WINDOWFRAME));
	
	return hb;
}