#define APSTUDIO_READONLY_SYMBOLS
#include "main.h"
#include "./setup.h"
#include "./setup_resource.h"
#include "./loadimage.h"
#include "./langutil.h"
#include "../nu/AutoWide.h"
#include "api.h"

#define HEADER_FONT_NAME			"Arial"//"Lucida Sans Unicode"//"Verdana"//"Trebuchet MS"//"Arial Unicode MS"
#define HEADER_FONT_SIZE		13 //11
#define HEADER_FONT_ITALIC		FALSE
#define HEADER_FONT_WEIGHT		FW_MEDIUM
#define HEADER_TEXT_COLOR		RGB(255,255,255)//RGB(16, 72, 148)//RGB(7, 30, 140)
#define HEADER_BORDER_COLOR		RGB(236, 233, 216)

#define HEADER_PAGENUM_FONT_NAME			"Lucida Sans Unicode"//"Lucida Sans Unicode"//"Verdana"//"Trebuchet MS"//"Arial Unicode MS"
#define HEADER_PAGENUM_FONT_SIZE			10
#define HEADER_PAGENUM_FONT_ITALIC		FALSE
#define HEADER_PAGENUM_FONT_WEIGHT		FW_SEMIBOLD//FW_MEDIUM
#define HEADER_PAGENUM_TEXT_COLOR		RGB(210,120,42)

#define NAVIGATION_FONT_NAME			"Arial"
#define NAVIGATION_FONT_SIZE		9
#define NAVIGATION_FONT_ITALIC		FALSE
#define NAVIGATION_FONT_WEIGHT		FW_MEDIUM	

#define NAVIGATION_SEL_FONT_NAME		"Arial"
#define NAVIGATION_SEL_FONT_SIZE	10
#define NAVIGATION_SEL_FONT_ITALIC	FALSE
#define NAVIGATION_SEL_FONT_WEIGHT	FW_MEDIUM//FW_SEMIBOLD

#define NAVIGATION_SEL_TEXT_COLOR	RGB(252, 252, 255)
#define NAVIGATION_TEXT_COLOR		RGB(222, 225, 234)
#define NAVIGATION_BACK_COLOR		RGB(137,145,156)//RGB(150,156,163)
#define NAVIGATION_PADDING_LEFT		0
#define NAVIGATION_PADDING_RIGHT	0

#define PAGE_BACK_COLOR				RGB(236, 234, 232)


typedef struct _UI
{
	ULONG		ref;
	HBRUSH		hbPage;
	HBRUSH		hbHeader;
	HBRUSH		hbNavItemSel;
	HBRUSH		hbNavBack;
	HFONT		hfHeader;
	HFONT		hfNavItem;
	HFONT		hfNavItemSel;
	HFONT		hfHeaderPageNum;
	INT			nHdrTxtHeight;
	INT			nHdrPageTxtHeight;
	INT			nNavTxtHeight;
	INT			nNavTxtSelHeight;
	INT			nHdrHeight;
	INT			nNavItemHeight;
	COLORREF	rgbPageBk;
} UI;

typedef struct _SPTHEME 
{
	WNDPROC fnOldProc;
	UI		*pui;
	BOOL	bUnicode;
} SPTHEME;

static WASetup *g_pAttachInstance = NULL;
static BOOL bUseMarquee = -1;

static INT_PTR WINAPI AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR WINAPI JobStatusDialog(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR WINAPI ErrorPageDialog(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT WINAPI PageWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT ConvertFontHeight(HWND hwnd, INT ptHeight);
static DWORD GetHighestFontQuality(void);
static BOOL InitializeUI(UI *pui, HWND hwndCtrl);
static BOOL ReleaseUI(UI *pui);
static const wchar_t *GetUnknownStr(void);
static LRESULT WINAPI FrameWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL JobStatus_Advance(HWND hwndStatus);

WASetup::WASetup(void) 
	: ref((size_t)1), hwnd(NULL), hwndActive(NULL), nPageActive((size_t)-1), pui(NULL), hWinamp(NULL)
{	
}

WASetup::~WASetup(void)
{
	if (hwnd && IsWindow(hwnd)) DestroyWindow(hwnd);
	size_t index;
	index = pageList.size();
	while(index--)
	{
		pageList[index]->Release();
	}

	index = jobList.size();
	while(index--)
	{
		jobList[index]->Release();
	}
}
svc_setup *WASetup::CreateInstance()
{
	WASetup *instance = new WASetup();
	return (svc_setup*)instance;	
}

int WASetup::AddRef(void)
{
	return ++ref;
}

int WASetup::Release(void)
{	
	if (1 == ref) 
	{
		delete(this);
		return 0;
	}
	return --ref;
}


HRESULT WASetup::InsertPage(ifc_setuppage *pPage, int* pIndex)
{
	size_t index;
	if (!pIndex || !pPage) return E_INVALIDARG;
	
	index = *pIndex;
	if (index >= pageList.size()) 
	{
		index = pageList.size();
		pageList.push_back(pPage);
	}
	else
	{
		//pageList.insertBefore(*pIndex, pPage);
		pageList.insert(pageList.begin() + index, pPage);
	}

	*pIndex = (int)index;
	pPage->AddRef();
	return S_OK;
}

HRESULT WASetup::RemovePage(size_t index)
{
	if (index >= pageList.size()) 
		return HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
	pageList[index]->Release();
	pageList.erase(pageList.begin() + index);
	return S_OK;
}

HRESULT WASetup::GetPageCount(int *pCount)
{
	if (!pCount) return E_INVALIDARG;
	*pCount = (int)pageList.size();
	return S_OK;
}

HRESULT WASetup::GetPage(size_t index, ifc_setuppage **pPage)
{
	if (index >= pageList.size()) return HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
	*pPage = pageList[index];
	return S_OK;
}

HRESULT WASetup::AddJob(ifc_setupjob *pJob)
{
	for (size_t i = 0; i < jobList.size(); i++)
	{
		if (jobList[i] == pJob) return S_OK;
	}
	jobList.push_back(pJob);
	pJob->AddRef();
	return S_OK;
}

HRESULT WASetup::RemoveJob(ifc_setupjob *pJob)
{
	for (size_t i = 0; i < jobList.size(); i++)
	{
		if (jobList[i] == pJob)
		{
			jobList[i]->Release();
			jobList.erase(jobList.begin() + i);
			return S_OK;
		}
	}
	return E_INVALIDARG;
}

HRESULT WASetup::GetActiveIndex(int* pIndex)
{
	if (!pIndex) return E_INVALIDARG;
	*pIndex = (int)nPageActive;
	return S_OK;
}

HRESULT WASetup::CreateStatusWnd(HWND *phwndStatus)
{
	HWND hwndStatus;
	
	if (!phwndStatus) return S_FALSE;
    *phwndStatus = NULL;

	if (-1 == bUseMarquee)
	{
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		bUseMarquee = (GetVersionEx(&vi) && (vi.dwMajorVersion > 5 || (vi.dwMajorVersion == 5 && vi.dwMinorVersion > 0)));
	}

	hwndStatus = WACreateDialog(MAKEINTRESOURCEW(IDD_SETUPSTATUS), NULL, JobStatusDialog);
	if (!hwndStatus) return S_FALSE;
		
	if (rcUI.right != rcUI.left)
	{
		RECT rw;
		GetWindowRect(hwndStatus, &rw);
		SetWindowPos(hwndStatus, NULL, rcUI.left + ((rcUI.right - rcUI.left) - (rw.right - rw.left))/2, 
										rcUI.top + ((rcUI.bottom - rcUI.top) - (rw.bottom - rw.top))/2, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	HWND hwndCtrl = GetDlgItem(hwndStatus, IDC_PROGRESS);
	if (hwndCtrl)
	{
		if (bUseMarquee)
		{
			SetWindowLongPtrW(hwndCtrl, GWL_STYLE, GetWindowLongPtrW(hwndCtrl, GWL_STYLE) | 0x08/*PBS_MARQUEE*/);
			SendMessageW(hwndCtrl, (WM_USER + 10)/*PBM_SETMARQUEE*/, TRUE, (LPARAM)200);
		}
		else
		{
			SendMessageW(hwndCtrl, PBM_SETRANGE, 0, MAKELPARAM(0, 1 +  (INT)(pageList.size() + jobList.size())));
			SendMessageW(hwndCtrl, PBM_SETPOS, 0, 0L);
			SendMessageW(hwndCtrl, PBM_SETSTEP, 1, 0L);
		}
	}

	*phwndStatus = hwndStatus;
	return S_OK;
}

static BOOL WaSetup_MessageLoop(HWND hMainWnd, HACCEL  hAccel)
{
	MSG msg;

	for (;;)
	{
		DWORD status = MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
		if (WAIT_OBJECT_0 == status)
		{
			while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				//if (!CallMsgFilter(&msg, MSGF_DIALOGBOX))
				{
					if (msg.message == WM_QUIT)
						return (BOOL)msg.wParam;

					if (!TranslateAcceleratorW(hMainWnd, hAccel, &msg) && 
						!IsDialogMessageW(hMainWnd, &msg))
					{
						TranslateMessage(&msg);
						DispatchMessageW(&msg);
					}
				}
			}
		}
	}
}

HRESULT WASetup::Start(HWND hwndWinamp)
{
	INT_PTR r(IDOK);

	SetRectEmpty(&rcUI);

	hWinamp = hwndWinamp;

	if (pageList.size())
	{
		HACCEL hAccel;
		static UI ui = {0, };

		for (size_t i = 0; i < pageList.size(); i++)  pageList[i]->Revert();
		g_pAttachInstance = this;

		InitializeUI(&ui, hwnd);
		pui = &ui;

		hwnd = WACreateDialog(MAKEINTRESOURCEW(IDD_SETUP), NULL, ::DialogProc);
		if (!hwnd) return  E_UNEXPECTED;
		HINSTANCE hInst = (language_pack_instance) ? language_pack_instance : hMainInstance;
		hAccel = LoadAcceleratorsW(hInst, MAKEINTRESOURCEW(IDR_ACCEL_SETUP));

		r = WaSetup_MessageLoop(hwnd, hAccel);

		g_pAttachInstance = NULL;
		ReleaseUI(&ui);
	}

	return (IDOK == r) ? S_OK : S_FALSE;
}

HRESULT WASetup::Save(HWND hwndStatus)
{
	HRESULT hr(S_OK);
	HWND hwndText = GetDlgItem(hwndStatus, IDC_LBL_STATUS);
	for (size_t i = 0; i < pageList.size(); i++) 
	{
		if (hwndText) SetWindowTextW(hwndText, getStringW(IDS_STATUS_SAVING, NULL, 0));
		if (S_OK != pageList[i]->Save(hwndText)) hr = S_FALSE;
		JobStatus_Advance(hwndStatus);
	}
	WritePrivateProfileStringW(L"WinampReg", L"WAVer", AutoWide(APP_VERSION), INI_FILE);
	return hr;
}
HRESULT WASetup::ExecJobs(HWND hwndStatus)
{
	HRESULT hr(S_OK);
	HWND hwndText = GetDlgItem(hwndStatus, IDC_LBL_STATUS);
	HWND hwndBtn = GetDlgItem(hwndStatus, IDC_BTN_SKIP);
	for (size_t i = 0; i < jobList.size(); i++)  
	{
		if (hwndText) SetWindowTextW(hwndText, getStringW(IDS_STATUS_JOBS, NULL, 0));
		if (hwndBtn && S_OK == jobList[i]->IsCancelSupported() && 
			SetPropW(hwndStatus, L"JOB", (HANDLE)jobList[i]))
		{
			EnableWindow(hwndBtn, TRUE);
		}
		
		if (S_OK != jobList[i]->Execute(hwndText)) hr = S_FALSE;
		if (hwndBtn) EnableWindow(hwndBtn, FALSE);
		JobStatus_Advance(hwndStatus);
	}

	return hr;
}

HRESULT WASetup::GetWinampWnd(HWND *phwndWinamp)
{
	if (NULL == phwndWinamp)
		return E_INVALIDARG;
	*phwndWinamp = hWinamp;
	return (NULL == hWinamp) ? E_UNEXPECTED : S_OK;
}

static INT_PTR CALLBACK tmpProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

INT_PTR WASetup::OnInitDialog(HWND hwndFocused, LPARAM lParam)
{
	HWND hwndLB, hwndFrame, hwndHeader;
	RECT rw, rc;
	HICON hIcon = LoadIconW(hMainInstance, MAKEINTRESOURCE(ICON_XP));

	// make other people's dialogs show the winamp icon
	HWND h = CreateDialogW(hMainInstance, MAKEINTRESOURCE(IDD_OPENLOC), NULL, tmpProc);
	SetClassLongPtrW(h, GCLP_HICON, (LONG_PTR)hIcon);
	DestroyWindow(h);

	SendMessageW(hwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwnd, IDC_BTN_NEXT), TRUE);

	wchar_t buf[256] = {0};
	StringCchPrintfW(buf,256, getStringW(IDS_SETUP_WND_TITLE,NULL,0), AutoWideDup(app_version_string));
	SetWindowTextW(hwnd, buf);

	// adjust dialog
	rw.left = GetPrivateProfileIntW(L"SETUP", L"left", 0, INI_FILE);
	rw.top = GetPrivateProfileIntW(L"SETUP", L"top", 0, INI_FILE);
	rw.right = GetPrivateProfileIntW(L"SETUP", L"right", 0, INI_FILE);
	rw.bottom = GetPrivateProfileIntW(L"SETUP", L"bottom", 0, INI_FILE);

	if (rw.left != rw.right && rw.top != rw.bottom)
	{
		INT x, y;
		GetWindowRect(hwnd, &rc);
		x = (rw.right) ? (rw.left + ((rw.right - rw.left) - (rc.right - rc.left))/2) : rw.left;
		y = (rw.bottom) ? (rw.top + ((rw.bottom - rw.top) - (rc.bottom - rc.top))/2) : rw.top;
		SetWindowPos(hwnd, NULL, x, y, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	hwndFrame = GetDlgItem(hwnd, IDC_FRAME);

	// TODO if this is needed again then remove
	// deletes the 'Tools' menu as it's not used
	DeleteMenu(GetMenu(hwnd), 2, MF_BYPOSITION);
	DrawMenuBar(hwnd);

	WNDPROC fnOldProc = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(hwndFrame, GWLP_WNDPROC, (LONG_PTR)FrameWndProc);
	if (fnOldProc) SetPropW(hwndFrame, L"SKINFRAME", fnOldProc);

	SetWindowLongPtrW(hwndFrame, GWL_STYLE, GetWindowLongPtrW(hwndFrame, GWL_STYLE) | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	if (!IsWinXPTheme()) SetWindowLongPtrW(hwndFrame, GWL_EXSTYLE, (GetWindowLongPtrW(hwndFrame, GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE) | WS_EX_STATICEDGE);
	SetWindowPos(hwndFrame, HWND_BOTTOM, 0,0,0,0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOOWNERZORDER);

	GetClientRect(hwndFrame, &rc);
	MapWindowPoints(hwndFrame, hwnd, (POINT*)&rc, 2);

	hwndLB = GetDlgItem(hwnd, IDC_LB_NAVIGATION);
	if (hwndLB)
	{
		GetWindowRect(hwndLB, &rw);
		SetWindowPos(hwndLB, GetDlgItem(hwnd, IDC_BTN_BACK), rc.left, rc.top, rw.right - rw.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOOWNERZORDER);
		SendMessageW(hwndLB, WM_SETFONT, (WPARAM)pui->hfNavItem, FALSE);
	}

	hwndHeader = GetDlgItem(hwnd, IDC_HEADER);
	if (hwndHeader)
	{
		SendMessageW(hwndHeader, WM_SETFONT, (WPARAM)pui->hfHeader, FALSE);
		GetWindowRect(hwndLB, &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
		SetWindowPos(hwndHeader, NULL, rw.right, rc.top, rc.right - rw.right, pui->nHdrHeight, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
	}

	if (hwndLB)
	{
		for (size_t i = 0; i < pageList.size(); i++) SendMessageW(hwndLB, LB_ADDSTRING, 0, (LPARAM)i);
		SendMessageW(hwndLB, LB_SETCURSEL, 0, 0L);
		PostMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_LB_NAVIGATION, LBN_SELCHANGE), (LPARAM)hwndLB);
	}

	ShowWindow(hwnd, SW_SHOWNORMAL);
	DWORD ourThreadID, foregroundThreadID;
	foregroundThreadID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
	ourThreadID = GetCurrentThreadId();

	if (foregroundThreadID != ourThreadID) AttachThreadInput(foregroundThreadID, ourThreadID, TRUE);	
	SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	if (foregroundThreadID != ourThreadID) AttachThreadInput(foregroundThreadID, ourThreadID, FALSE);

	return 0;
}

void WASetup::OnDestroy(void)
{
	GetWindowRect(hwnd, &rcUI);
}

void WASetup::OnCancel(void)
{
	BOOL bNeedSave = FALSE;
	WCHAR szTitle[128] = {0};

	for (size_t i = 0; i < pageList.size() && !bNeedSave; i++) 
	{
		bNeedSave = (S_FALSE != pageList[i]->IsDirty());
	}

	GetWindowTextW(hwnd, szTitle, sizeof(szTitle)/sizeof(WCHAR));
	if (bNeedSave) 
	{
		INT nr = MessageBoxW(hwnd, getStringW(IDS_SAVE_CHANGES_BEFORE_EXIT, NULL, 0), szTitle, MB_YESNOCANCEL | MB_ICONWARNING);
		switch(nr)
		{
			case IDYES:	SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDM_FILE_SAVECHANGES, 0), 0L); break;
			case IDCANCEL:	return;
		}
	}
	else if (IDNO == MessageBoxW(hwnd, getStringW(IDS_QUIT_OK, NULL, 0), szTitle, MB_YESNO | MB_ICONWARNING)) return;

	DestroyWindow(hwnd);
	PostQuitMessage(IDCANCEL);
}

void WASetup::OnNext_Clicked(HWND hwndCtrl)
{
	HWND hwndLB = GetDlgItem(hwnd, IDC_LB_NAVIGATION);
	INT index = (INT)SendMessageW(hwndLB, LB_GETCURSEL, 0, 0L) + 1;
	if (index > -1) SendMessageW(hwndLB, LB_SETCURSEL, (WPARAM)index, 0L);
	PostMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_LB_NAVIGATION, LBN_SELCHANGE), (LPARAM)hwndLB);
}

void WASetup::OnBack_Clicked(HWND hwndCtrl)
{
	HWND hwndLB = GetDlgItem(hwnd, IDC_LB_NAVIGATION);
	INT index = (INT)SendMessageW(hwndLB, LB_GETCURSEL, 0, 0L)  -1;
	if (index > -1) SendMessageW(hwndLB, LB_SETCURSEL, (WPARAM)index, 0L);
	PostMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_LB_NAVIGATION, LBN_SELCHANGE), (LPARAM)hwndLB);
}

void WASetup::OnNavigation_SelChange(HWND hwndCtrl)
{
	HWND hwndFrame;
	HMENU hMenu;
	MENUITEMINFO mii;

	INT idList[] = { IDC_BTN_BACK, IDC_BTN_NEXT};
	INT count = (INT)SendMessageW(hwndCtrl, LB_GETCOUNT, 0, 0L);
	INT index = (INT)SendMessageW(hwndCtrl, LB_GETCURSEL, 0, 0L);

	if (nPageActive == (size_t)index) return;

	if (-1 != nPageActive && S_FALSE == pageList[nPageActive]->Validate()) 
	{
		SendMessageW(hwndCtrl, LB_SETCURSEL, nPageActive, 0L);
		return;
	}

	hwndFrame = GetDlgItem(hwnd, IDC_FRAME);
	hMenu = GetMenu(hwnd);

	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;

	for(int i = sizeof(idList)/sizeof(int) - 1; i >= 0 ; i--)
	{
		HWND hwndBtn = GetDlgItem(hwnd, idList[i]);
		BOOL bEnable = (IDC_BTN_NEXT == idList[i]) ? ((count - index) > 1) : (0 != index);
		if (hwndBtn)
		{
			if (bEnable != IsWindowEnabled(hwndBtn))
			{
				if (IDC_BTN_NEXT == idList[i])  SendMessageW(hwnd, DM_SETDEFID, (WPARAM)((bEnable) ? IDC_BTN_NEXT : IDOK), 0L); 
				EnableWindow(hwndBtn, bEnable);
			}

			if (hMenu)
			{
				mii.fState = (bEnable) ? MFS_ENABLED : MFS_DISABLED;
				SetMenuItemInfoW(hMenu, (IDC_BTN_NEXT == idList[i]) ? IDM_NAVIGATE_NEXT : IDM_NAVIGATE_BACK, FALSE, &mii);
			}
		}
	}

	if (hwndActive)
	{
		DestroyWindow(hwndActive);
		hwndActive = NULL;
		nPageActive = (size_t)-1;
	}

	if (S_OK != pageList[index]->CreateView(hwnd, &hwndActive))
		hwndActive = WACreateDialog(MAKEINTRESOURCEW(IDD_SETUP_PAGE_ERROR), hwnd, ErrorPageDialog);

	HWND hwndHeader;
	RECT rc;
	GetClientRect(hwndFrame, &rc);
	MapWindowPoints(hwndFrame, hwnd, (POINT*)&rc, 2);

	hwndHeader = GetDlgItem(hwnd, IDC_HEADER);
	if (hwndHeader && (WS_VISIBLE & GetWindowLongPtrW(hwndHeader, GWL_STYLE)))
	{
		RECT rw;
		GetWindowRect(hwndHeader, &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
		rc.top = rw.bottom;
		rc.left = rw.left;
	}

	SetWindowPos(hwndActive,	GetDlgItem(hwnd, IDC_LB_NAVIGATION), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE);

	if(IsWinXPTheme()) WAEnableThemeDialogTexture(hwndActive, ETDT_ENABLETAB);
	else 
	{ 
		SPTHEME *pTheme = (SPTHEME*)malloc(sizeof(SPTHEME));
		pTheme->bUnicode = IsWindowUnicode(hwndActive);
		pTheme->pui = pui;
		pTheme->fnOldProc = (WNDPROC)SetWindowLongPtrW(hwndActive, GWLP_WNDPROC, (LONG_PTR)PageWndProc);
		if (!pTheme->fnOldProc || !SetPropW(hwndActive, L"SPTHEME", pTheme))
		{
			if (pTheme->fnOldProc) SetWindowLongPtrW(hwndActive, GWLP_WNDPROC, (LONG_PTR)pTheme->fnOldProc);
			free(pTheme);
		}
	}
	ShowWindow(hwndActive, SW_SHOW);
	nPageActive = index;

	if (hwndHeader) InvalidateRect(hwndHeader, NULL, FALSE);

	HWND hwndTest = GetNextDlgTabItem(hwnd, GetWindow(hwndActive, GW_HWNDPREV), FALSE);
	if (hwndTest) SendMessageW(hwnd, WM_NEXTDLGCTL, (WPARAM)hwndTest, TRUE);
}

void WASetup::OnCommand(INT nCtrlID, INT nEvntID, HWND hwndCtrl)
{
	switch(nCtrlID)
	{
		case IDOK:
			if (BN_CLICKED == nEvntID)
			{
				if (-1 != nPageActive && S_FALSE != pageList[nPageActive]->Validate()) 
				{
					DestroyWindow(hwnd);
					PostQuitMessage(IDOK);
				}
			}
			break;
		case IDCANCEL:		if (BN_CLICKED == nEvntID) OnCancel(); break;
		case IDC_BTN_NEXT:	if (BN_CLICKED == nEvntID) OnNext_Clicked(hwndCtrl); break;
		case IDC_BTN_BACK:	if (BN_CLICKED == nEvntID) OnBack_Clicked(hwndCtrl); break;

		case IDC_LB_NAVIGATION:
			switch(nEvntID)
			{
				case LBN_SELCHANGE: OnNavigation_SelChange(hwndCtrl); break;
			}
			break;

		case IDM_HELP_ABOUT:	WADialogBox(MAKEINTRESOURCEW(IDD_ABOUT), hwnd, AboutDialogProc); break;
		case IDM_NAVIGATE_BACK:	SendMessageW(GetDlgItem(hwnd, IDC_BTN_BACK), BM_CLICK, 0, 0L); break;
		case IDM_NAVIGATE_NEXT:	SendMessageW(GetDlgItem(hwnd, IDC_BTN_NEXT), BM_CLICK, 0, 0L); break;
		case IDM_FILE_EXIT:		OnCancel(); break;
		case IDM_FILE_SAVECHANGES:
			if (S_OK != Save(NULL))
			{
				WCHAR szTitle[128] = {0}, szText[256] = {0};
				GetWindowTextW(hwnd, szTitle, sizeof(szTitle)/sizeof(WCHAR));
				MessageBoxW(hwnd, getStringW(IDS_CHANGES_NOT_SAVED, szText, sizeof(szText)/sizeof(wchar_t)), szTitle, MB_OK | MB_ICONERROR);
			}
			break;
	}
}

void WASetup::OnDrawHeader(DRAWITEMSTRUCT *pdis)
{
	const wchar_t *pszName;
	RECT ri, re;
	INT top;

	CopyRect(&ri, &pdis->rcItem);
	CopyRect(&re, &ri);
	re.right = ri.left + 1;

	FillRect(pdis->hDC, &re, (HBRUSH)GetStockObject(WHITE_BRUSH));

	ri.left = re.right;
	SetBrushOrgEx(pdis->hDC, ri.left, ri.top, NULL);
	FillRect(pdis->hDC, &ri, pui->hbHeader);

	if (nPageActive >= pageList.size() || S_OK != pageList[nPageActive]->GetName(FALSE, &pszName) || !*pszName) pszName = GetUnknownStr();

	SetBkMode(pdis->hDC, TRANSPARENT);

	InflateRect(&ri, -4, -2);

	top = ri.top + (ri.bottom - ri.top - pui->nHdrTxtHeight)/2 - 1;
	if (top > ri.top) ri.top = top;

	if (ri.left < ri.right)
	{
		RECT rn;
		wchar_t szPageInfo[64] = {0};
		CopyRect(&rn, &ri);
		SetTextColor(pdis->hDC, HEADER_PAGENUM_TEXT_COLOR);
		UINT oldMode = SetTextAlign(pdis->hDC, TA_RIGHT);
		HFONT hfOld = (pui->hfHeaderPageNum) ? (HFONT)SelectObject(pdis->hDC, pui->hfHeaderPageNum) : NULL;
		top = ri.top + pui->nHdrTxtHeight - pui->nHdrPageTxtHeight;
		if (top > rn.top) rn.top = top;
		rn.right -= 8;
		rn.left = rn.right - 42;
		StringCchPrintfW(szPageInfo, sizeof(szPageInfo)/sizeof(wchar_t), L"%d/%d", nPageActive + 1, pageList.size());
		ExtTextOutW(pdis->hDC, rn.right, rn.top, ETO_CLIPPED, &rn, szPageInfo, lstrlenW(szPageInfo), NULL);

		SetTextAlign(pdis->hDC, oldMode);
		if (hfOld) SelectObject(pdis->hDC, hfOld);
		ri.right = rn.left -= 4;
	}

	if (ri.left < ri.right)
	{
		SetTextColor(pdis->hDC, HEADER_TEXT_COLOR);
		SetTextAlign(pdis->hDC, TA_LEFT);
		ExtTextOutW(pdis->hDC, ri.left, ri.top, ETO_CLIPPED, &ri, pszName, lstrlenW(pszName), NULL);
	}
}

void WASetup::OnDrawNavigationItem(DRAWITEMSTRUCT *pdis)
{
	const wchar_t *pszName;
	RECT ri;
	HFONT hfOld;
	wchar_t szTitle[128] = {0};	
	COLORREF rgbText;

	if (pdis->itemID == -1) return;

	CopyRect(&ri, &pdis->rcItem);
	ri.left += NAVIGATION_PADDING_LEFT;
	ri.right -= NAVIGATION_PADDING_RIGHT;

	if (ODA_FOCUS == pdis->itemAction)
	{
		if (0 == (0x0200/*ODS_NOFOCUSRECT*/ & pdis->itemState))
		{
			InflateRect(&ri, -1, -1);	
			DrawFocusRect(pdis->hDC, &ri);
		}
		return;
	}

	if (ODS_SELECTED & pdis->itemState)
	{
		HBRUSH hbActive;
		if (pui->hbNavItemSel)
		{
			SetBrushOrgEx(pdis->hDC, ri.left, ri.top, NULL);
			hbActive = pui->hbNavItemSel;
		}
		else
		{
			SetBrushOrgEx(pdis->hDC, 0, 0, NULL);
			hbActive = pui->hbNavBack;
		}
		FillRect(pdis->hDC, &ri, hbActive);
		rgbText = SetTextColor(pdis->hDC, NAVIGATION_SEL_TEXT_COLOR);
		hfOld = (HFONT)SelectObject(pdis->hDC, pui->hfNavItemSel);
	}
	else
	{
		if (ODA_SELECT == pdis->itemAction) 
		{
			SetBrushOrgEx(pdis->hDC, 0, 0, NULL);
			FillRect(pdis->hDC, &ri, pui->hbNavBack);
		}
		rgbText = 0;
		hfOld = NULL;
	}

	if (pdis->itemData >= pageList.size() || S_OK != pageList[pdis->itemData]->GetName(TRUE, &pszName) || !*pszName)
		pszName = GetUnknownStr();
	
	SetBkMode(pdis->hDC, TRANSPARENT);

	InflateRect(&ri, -4, -2);
	INT top = ri.top + (ri.bottom - ri.top - pui->nNavTxtHeight)/2 - 1;
	if (top > ri.top) ri.top = top;

	StringCchPrintfW(szTitle, sizeof(szTitle)/sizeof(wchar_t), L"%d. %s", pdis->itemData + 1, pszName);
	ExtTextOutW(pdis->hDC, ri.left, ri.top, ETO_CLIPPED, &ri, szTitle, lstrlenW(szTitle), NULL);

	if (ODS_SELECTED & pdis->itemState)
	{
		SetTextColor(pdis->hDC, rgbText);
		if (hfOld) SelectObject(pdis->hDC, hfOld);
	}
}

INT_PTR WASetup::OnDrawItem(INT nCtrlID, DRAWITEMSTRUCT *pdis)
{
	switch(nCtrlID)
	{
		case IDC_HEADER:			OnDrawHeader(pdis); return TRUE;
		case IDC_LB_NAVIGATION:		OnDrawNavigationItem(pdis); return TRUE;
	}
	return 0;
}

INT_PTR WASetup::OnMeasureItem(INT nCtrlID, MEASUREITEMSTRUCT *pmis)
{
	switch(nCtrlID)
	{
		case IDC_LB_NAVIGATION:
			pmis->itemHeight = (pui) ? pui->nNavItemHeight : 0;
			return TRUE;
	}
	return FALSE;
}

INT_PTR WASetup::OnColorListBox(HDC hdc, HWND hwndCtrl)
{
	if (hwndCtrl == GetDlgItem(hwnd, IDC_LB_NAVIGATION))
	{

		SetTextColor(hdc, NAVIGATION_TEXT_COLOR);
		SetBkColor(hdc, NAVIGATION_BACK_COLOR);
		return (INT_PTR)pui->hbNavBack;
	}
	return NULL;
}

INT_PTR WASetup::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR result = 0;
	switch(uMsg)
	{
		case WM_INITDIALOG:		return OnInitDialog((HWND)wParam, lParam);
		case WM_DESTROY:		OnDestroy(); break;
		case WM_COMMAND:		OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case WM_DRAWITEM:		result = OnDrawItem((INT)wParam, (DRAWITEMSTRUCT*)lParam); break;
		case WM_MEASUREITEM:	result = OnMeasureItem((INT)wParam, (MEASUREITEMSTRUCT*)lParam); break;
		case WM_CTLCOLORLISTBOX: return OnColorListBox((HDC)wParam, (HWND)lParam); 
		case WM_CHAR:	
			if (0x30 == wParam)
			{
				OutputDebugStringA("test\n");
				return 0;
			}
	}

	if (result)
	{
		SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, (LONG_PTR)result);
		return TRUE;
	}

	return 0;
}

static INT_PTR WINAPI DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WASetup *pInst = (WASetup*)GetPropW(hwndDlg, L"SETUPDLG");
	if (!pInst && g_pAttachInstance)
	{
		pInst = g_pAttachInstance;
		pInst->hwnd = hwndDlg;
		SetPropW(hwndDlg, L"SETUPDLG", pInst);
		g_pAttachInstance = NULL;
	}

	switch(uMsg)
	{
		case WM_DESTROY:
			if (pInst) 
			{	
				pInst->DialogProc(uMsg, wParam, lParam);
				RemovePropW(hwndDlg,  L"SETUPDLG");
				pInst = NULL;
			}
			break;
	}

	return (pInst) ? pInst->DialogProc(uMsg, wParam, lParam) : 0;
}

static INT_PTR WINAPI AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
			{
				SendDlgItemMessageW(hwndDlg, IDC_PIC_ABOUT, STM_SETIMAGE, IMAGE_BITMAP, 
									(LPARAM)WALoadImage2(L"PNG", MAKEINTRESOURCEW(IDB_ABOUT), FALSE));

				wchar_t buf[2048] = {0}, buf2[2048] = {0};
				GetWindowTextW(GetDlgItem(hwndDlg,IDC_VER_TEXT),buf,ARRAYSIZE(buf));
				StringCchPrintfW(buf2,2048,(buf[0] ? buf : L"v%s %s - %s"),AutoWideDup(app_version_string),AutoWideDup(APP_VERSION_PLATFORM),AutoWideDup(app_date));
				SetWindowTextW(GetDlgItem(hwndDlg,IDC_VER_TEXT),buf2);
			}
			break;
		case WM_DESTROY:
			{
				DeleteObject((HBITMAP)SendDlgItemMessageW(hwndDlg, IDC_PIC_ABOUT, STM_GETIMAGE, IMAGE_BITMAP, 0L));
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					break;
			}
	}

	return 0;
}

static INT_PTR WINAPI JobStatusDialog(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_BTN_SKIP:
					if (BN_CLICKED == HIWORD(wParam))
					{
						ifc_setupjob *pj = (ifc_setupjob*)GetPropW(hwndDlg, L"JOB");
						if (pj)
						{
							HWND hwndStatus = GetDlgItem(hwndDlg, IDC_LBL_STATUS);
							EnableWindow((HWND)lParam, FALSE);
							if (hwndStatus) SetWindowTextW(hwndStatus, getStringW(IDS_HTTP_ABORT, NULL, 0));
							pj->Cancel(hwndStatus);
						}
					}
					break;
			}
		case WM_DESTROY:
			RemovePropW(hwndDlg, L"JOB");
	}

	return 0;
}

static INT_PTR WINAPI ErrorPageDialog(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_WINDOWPOSCHANGED:
			if (SWP_NOSIZE != ((SWP_NOSIZE | SWP_FRAMECHANGED) & ((WINDOWPOS*)lParam)->flags))
			{
				HWND messageWindow;
				messageWindow = GetDlgItem(hwndDlg, IDC_LBL_MESSAGE);
				if (NULL != messageWindow)
				{
					RECT rect;
					long top;

					GetWindowRect(messageWindow, &rect);
					MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT*)&rect, 1);

					top = rect.top;

					GetClientRect(hwndDlg, &rect);
					rect.top = top;

					SetWindowPos(messageWindow, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 
								SWP_NOACTIVATE | SWP_NOZORDER);
				}
			}
			break;
	}
	return 0;
}

static LRESULT WINAPI PageWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SPTHEME *pTheme = (SPTHEME*)GetPropW(hwnd, L"SPTHEME");
	if (!pTheme || !pTheme->fnOldProc) return DefWindowProcW(hwnd, uMsg, wParam, lParam);

	switch(uMsg)
	{
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSTATIC:
			SetBkColor((HDC)wParam, PAGE_BACK_COLOR);
			SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
			return (LRESULT)pTheme->pui->hbPage;
		case WM_DESTROY:
			{
				WNDPROC fnOldProc = pTheme->fnOldProc;
				RemovePropA(hwnd, "SPTHEME");
				free(pTheme);
				SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)fnOldProc);
				return CallWindowProcW(fnOldProc, hwnd, uMsg, wParam, lParam);
			}
	}

	return (pTheme->bUnicode) ? CallWindowProcW(pTheme->fnOldProc, hwnd, uMsg, wParam, lParam) :
								CallWindowProcA(pTheme->fnOldProc, hwnd, uMsg, wParam, lParam);
}

static DWORD GetHighestFontQuality(void)
{
	DWORD fdwQuality;
	BOOL bSmoothing;

	if (SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &bSmoothing, 0) && bSmoothing)
	{
		OSVERSIONINFO vi = { sizeof(OSVERSIONINFO), };
		fdwQuality = (GetVersionEx(&vi) && (vi.dwMajorVersion > 5 || (vi.dwMajorVersion == 5 && vi.dwMinorVersion > 0))) ?
								5/*CLEARTYPE_QUALITY*/ : ANTIALIASED_QUALITY;
	}
	else fdwQuality = DEFAULT_QUALITY;

	return fdwQuality;
}

static BOOL InitializeUI(UI *pui, HWND hwndCtrl)
{
	if (!pui) return FALSE;
	if (!pui->ref) 
	{
		HBITMAP hbmp;
		BITMAP bi;
		HDC hdc;
		INT logPx;
		TEXTMETRIC tm;

		hdc = GetWindowDC(hwndCtrl);
		logPx = GetDeviceCaps(hdc, LOGPIXELSY);

		pui->hbPage = CreateSolidBrush(PAGE_BACK_COLOR);

		hbmp = (HBITMAP)LoadImageW(hMainInstance, MAKEINTRESOURCEW(IDB_NAVIGATION_STRIP), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR); 
		if (hbmp)
		{		
			pui->hbNavBack = CreatePatternBrush(hbmp);
			DeleteObject(hbmp);
		}
		else pui->hbNavBack = CreateSolidBrush(NAVIGATION_BACK_COLOR);

		pui->hfNavItem = CreateFontA(-MulDiv(NAVIGATION_FONT_SIZE, logPx, 72), 0, 0, 0, NAVIGATION_FONT_WEIGHT,
									NAVIGATION_FONT_ITALIC, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
									GetHighestFontQuality(), FF_DONTCARE, NAVIGATION_FONT_NAME);

		pui->hfNavItemSel = CreateFontA(-MulDiv(NAVIGATION_SEL_FONT_SIZE, logPx, 72), 0, 0, 0, NAVIGATION_SEL_FONT_WEIGHT,
									NAVIGATION_SEL_FONT_ITALIC, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
									GetHighestFontQuality(), FF_DONTCARE, NAVIGATION_SEL_FONT_NAME);

		pui->hfHeader = CreateFontA(-MulDiv(HEADER_FONT_SIZE, logPx, 72), 0, 0, 0, HEADER_FONT_WEIGHT,
									HEADER_FONT_ITALIC, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
									GetHighestFontQuality(), FF_DONTCARE, HEADER_FONT_NAME);

		pui->hfHeaderPageNum = CreateFontA(-MulDiv(HEADER_PAGENUM_FONT_SIZE, logPx, 72), 0, 0, 0, HEADER_PAGENUM_FONT_WEIGHT,
									HEADER_PAGENUM_FONT_ITALIC, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
									GetHighestFontQuality(), FF_DONTCARE, HEADER_PAGENUM_FONT_NAME);

		pui->nHdrHeight = 36;

		hbmp = (HBITMAP)LoadImageW(hMainInstance, MAKEINTRESOURCEW(IDB_HEADER_STRIP), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR); 
		if (hbmp)
		{			
			if (GetObject(hbmp, sizeof(BITMAP), &bi)) pui->nHdrHeight = bi.bmHeight;
			pui->hbHeader = CreatePatternBrush(hbmp);
			DeleteObject(hbmp);
		}

		pui->nNavItemHeight = 32;
//		hbmp = (HBITMAP)LoadImageW(hMainInstance, MAKEINTRESOURCEW(IDB_NAVITEM_STRIP), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR); 
//		if (hbmp)
//		{			
//			if (GetObject(hbmp, sizeof(BITMAP), &bi)) pui->nNavItemHeight = bi.bmHeight;
//			pui->hbNavItemSel = CreatePatternBrush(hbmp);
//			DeleteObject(hbmp);
//		}

		HFONT hfOld = (HFONT)SelectObject(hdc, pui->hfHeader);
		GetTextMetrics(hdc, &tm);
		pui->nHdrTxtHeight = tm.tmAscent;

		SelectObject(hdc, pui->hfHeaderPageNum);
		GetTextMetrics(hdc, &tm);
		pui->nHdrPageTxtHeight = tm.tmAscent;

		SelectObject(hdc, pui->hfNavItem);
		GetTextMetrics(hdc, &tm);
		pui->nNavTxtHeight = tm.tmAscent;

		SelectObject(hdc, pui->hfNavItemSel);
		GetTextMetrics(hdc, &tm);
		pui->nNavTxtSelHeight = tm.tmAscent;

		SelectObject(hdc, hfOld);
		ReleaseDC(hwndCtrl, hdc);
	}
	pui->ref++;
	return TRUE;
}

static BOOL ReleaseUI(UI *pui)
{
	if (!pui) return FALSE;
	if (0 == pui->ref) 
	{
		return TRUE;
	}
	if (1 == pui->ref)
	{
		if (pui->hbPage) DeleteObject(pui->hbPage);
		if (pui->hbNavBack) DeleteObject(pui->hbNavBack);
		if (pui->hbHeader) DeleteObject(pui->hbHeader);
		if (pui->hbNavItemSel) DeleteObject(pui->hbNavItemSel);
		if (pui->hfNavItem) DeleteObject(pui->hfNavItem);
		if (pui->hfNavItemSel) DeleteObject(pui->hfNavItemSel);
		if (pui->hfHeader) DeleteObject(pui->hfHeader);
		if (pui->hfHeaderPageNum) DeleteObject(pui->hfHeaderPageNum);
		ZeroMemory(pui, sizeof(UI));
		return TRUE; 
	}
	pui->ref--;
	return TRUE;
}

static const wchar_t *GetUnknownStr(void)
{
	static wchar_t unknown[64] = {0,};
	return (unknown) ? unknown : getStringW(IDS_UNKNOWN, unknown, sizeof(unknown)/sizeof(wchar_t));
}

static LRESULT WINAPI FrameWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC fnOldProc = (WNDPROC)GetPropW(hwnd, L"SKINFRAME");
	if (!fnOldProc) return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	switch(uMsg)
	{
		case WM_DESTROY:
			RemovePropW(hwnd, L"SKINFRAME");
			SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)fnOldProc);
			break;
		case WM_PAINT:
			ValidateRect(hwnd, NULL);
			return 0;
		case WM_ERASEBKGND: 	return 1;
	}
	return CallWindowProcW(fnOldProc, hwnd, uMsg, wParam, lParam);
}

static BOOL JobStatus_Advance(HWND hwndStatus)
{
	if (bUseMarquee > 0 ) return TRUE;
	if (!hwndStatus) return FALSE;
	HWND hwndCtrl = GetDlgItem(hwndStatus, IDC_PROGRESS);
	if (!hwndCtrl) return FALSE;
	SendMessageW(hwndCtrl, PBM_STEPIT, 0, 0L);
	return TRUE;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS WASetup
START_DISPATCH
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(API_SETUP_INSERT_PAGE, InsertPage)
CB(API_SETUP_REMOVE_PAGE, RemovePage)
CB(API_SETUP_GET_PAGE_COUNT, GetPageCount)
CB(API_SETUP_GET_PAGE, GetPage)
CB(API_SETUP_GET_ACTIVE_INDEX, GetActiveIndex)
CB(API_SETUP_START, Start)
CB(API_SETUP_ADD_JOB, AddJob)
CB(API_SETUP_REMOVE_JOB, RemoveJob)
CB(API_SETUP_CREATE_STATUSWND, CreateStatusWnd)
CB(API_SETUP_SAVE, Save)
CB(API_SETUP_EXECJOBS, ExecJobs)
CB(API_SETUP_GETWINAMPWND, GetWinampWnd)
END_DISPATCH
#undef CBCLASS