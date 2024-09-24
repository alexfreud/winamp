#include "./creddlg.h"


#include <pshpack2.h>

typedef struct _DLGTEMPLATEEX
{
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
    WORD menu;
    WORD windowClass;
    WCHAR title;
    WORD pointsize;
    WORD weight;
    BYTE italic;
    BYTE charset;
    WCHAR typeface;
} DLGTEMPLATEEX;

#include <poppack.h>
 
static const DLGTEMPLATEEX dlg_template = {1, 0xFFFF, 0, 
												WS_EX_DLGMODALFRAME | WS_EX_CONTROLPARENT | WS_EX_NOPARENTNOTIFY, 
												WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU,
												0, 0, 0, 0, 0, 0, 0, 0, };
typedef struct _CREDDATA
{
		
	LPWSTR	szUser;
	INT		cchUser;
	LPWSTR	szPassword;
	INT		cchPassword;
	DWORD	flags;
	HBITMAP	hbmp;
	LPCWSTR greating;
	INT		retcode;

} CREDDATA, *PCREDDATA;


typedef struct _WNDLIST
{
	HWND		hwndModal;
	HWND		*list;
	INT		count;
	INT		allocated;
} WNDLIST;

#define CREDCTRL	L"CREDCTRL"

#define BITMAP_HEIGHT	64
#define DIALOG_HEIGHT	236
#define DIALOG_WIDTH		316


static LRESULT WINAPI WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static HWND CreateCredCtrl(const PCREDDATA pcd, INT x, INT y, INT cx, INT cy, HWND hwndParent, HINSTANCE hInstance, INT ctrlID);

static void BlokedWndLst_Initialize(WNDLIST *plist, HWND hwndModal);
static void BlokedWndLst_Add(WNDLIST *plist, HWND hwnd);
static void BlokedWndLst_RemoveAll(WNDLIST *plist);
static BOOL CALLBACK BlockedWndLst_EnumThread(HWND hwnd, LPARAM param);

static HWND GetRoot(HWND hwnd )
{
	HWND hwndParent;
	while ((WS_CHILD & GetWindowLongPtr(hwnd, GWL_STYLE)) && (hwndParent = GetParent(hwnd))) hwnd = hwndParent;
	return hwnd;
}

static BOOL CALLBACK BlockedWndLst_EnumThread(HWND hwnd, LPARAM param)
{
	BlokedWndLst_Add((WNDLIST*)param, hwnd);
	return TRUE;
}

static void BlokedWndLst_Initialize(WNDLIST *plist, HWND hwndModal)
{
	plist->hwndModal = GetRoot(hwndModal);
	plist->count = 0;
	plist->allocated = 0;
	plist->list = NULL;

	EnumThreadWindows(GetCurrentThreadId(), BlockedWndLst_EnumThread, (LPARAM)plist);

}


static void BlokedWndLst_Add(WNDLIST *plist, HWND hwnd)
{
	int i;
	if (!hwnd || hwnd == plist->hwndModal || !IsWindowVisible(hwnd) || !IsWindowEnabled(hwnd)) return;
	hwnd = GetRoot(hwnd);
	if (!hwnd || hwnd == plist->hwndModal || !IsWindowVisible(hwnd) || !IsWindowEnabled(hwnd)) return;

	for (i = 0; i < plist->count; i++)  if (hwnd == plist->list[i]) return;
	
	if (plist->count == plist->allocated)
	{
		plist->allocated += 8;
		plist->list = realloc(plist->list, plist->allocated*sizeof(HWND));
	}
	if (!plist->list) return;
	plist->list[plist->count] = hwnd;
	plist->count++;
	EnableWindow(hwnd, FALSE);
}

static void BlokedWndLst_RemoveAll(WNDLIST *plist)
{
	int i;
	for ( i = 0; i < plist->count; i++) EnableWindow(plist->list[i], TRUE);
	if (plist->list) free(plist->list);
}




static HWND CreateCredCtrl(const PCREDDATA pcd, INT x, INT y, INT cx, INT cy, HWND hwndParent, HINSTANCE hInstance, INT ctrlID)
{
	WNDCLASSW wndclass;

	if (!GetClassInfoW(hInstance, CREDCTRL, &wndclass))
	{	
		wndclass.lpszClassName = CREDCTRL;
		wndclass.lpfnWndProc = WndProc;
		wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wndclass.hCursor = NULL;
		wndclass.hIcon = NULL;
		wndclass.hInstance = hInstance;
		wndclass.lpszMenuName = NULL;
		wndclass.style = 0x0;
		wndclass.cbClsExtra = 0x0;
		wndclass.cbWndExtra = sizeof(CREDDATA*);

		if (!RegisterClassW(&wndclass)) return NULL;
	}
	
	return CreateWindowExW(WS_EX_CONTROLPARENT | WS_EX_NOPARENTNOTIFY, CREDCTRL, pcd->greating, 
								WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD,  
								x, y, cx, cy, hwndParent, (HMENU)(INT_PTR)ctrlID, hInstance, (LPVOID)pcd);
}
typedef struct _CONTROLS
{
	INT		id;
	LPCWSTR pClass;
	DWORD	style;
	DWORD	exStyle;
    LPCWSTR pTitle;
}CONTROLS;

#define IDC_CREDCTRL			1101
#define IDC_BMP_LOGO				1001
#define IDC_LBL_GREATING		1002
#define IDC_LBL_USERNAME			1003
#define IDC_LBL_PASSWORD			1004
#define IDC_EDT_USERNAME			1005
#define IDC_EDT_PASSWORD			1006

#define BASE_WND_STYLE		WS_CHILD | WS_VISIBLE

static CONTROLS controls[] = 
{ 
	{IDC_BMP_LOGO, L"STATIC", BASE_WND_STYLE | SS_BITMAP | SS_REALSIZEIMAGE, WS_EX_NOPARENTNOTIFY, NULL },
	{IDC_LBL_GREATING, L"STATIC", BASE_WND_STYLE, WS_EX_NOPARENTNOTIFY, NULL },
	{IDC_LBL_USERNAME, L"STATIC", BASE_WND_STYLE, WS_EX_NOPARENTNOTIFY, L"User name:" },
	{IDC_EDT_USERNAME, L"EDIT", BASE_WND_STYLE | WS_TABSTOP, WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, NULL },
	{IDC_LBL_PASSWORD, L"STATIC", BASE_WND_STYLE, WS_EX_NOPARENTNOTIFY, L"Password:" },
	{IDC_EDT_PASSWORD, L"EDIT", BASE_WND_STYLE | WS_TABSTOP |  ES_PASSWORD, WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, NULL },
	{IDOK, L"BUTTON", BASE_WND_STYLE | BS_DEFPUSHBUTTON | WS_TABSTOP, WS_EX_NOPARENTNOTIFY, L"Ok" },
	{IDCANCEL, L"BUTTON", BASE_WND_STYLE | WS_TABSTOP, WS_EX_NOPARENTNOTIFY, L"Cancel" },
};


//// skinning 
#include "../winamp/wa_ipc.h"
#include "../winamp/wa_dlg.h"


typedef HRESULT (__stdcall *UX_SETWINDOWTHEME)(HWND, LPCWSTR, LPCWSTR);


static LRESULT OnCreate(HWND hwnd, LPCREATESTRUCTW pcs)
{
	int i;
	HFONT hf;
	CREDDATA *pcd;
	HMODULE uxdll;
	UX_SETWINDOWTHEME uxSetWindowTheme = NULL;

	pcd = (CREDDATA*)pcs->lpCreateParams;
	
	SetLastError(0);
	if (!SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG)(LONG_PTR)pcd) && GetLastError()) return -1;
	

	hf = GetStockObject(DEFAULT_GUI_FONT);
	if (CDS_SKINDIALOG & pcd->flags)
	{
		uxdll = LoadLibrary("uxtheme.dll");
		uxSetWindowTheme = (uxdll) ? (UX_SETWINDOWTHEME)GetProcAddress(uxdll, "SetWindowTheme") : NULL;
		if (uxSetWindowTheme) uxSetWindowTheme(hwnd, L" ", L" ");
	}else uxdll = NULL;

	for (i = 0; i < sizeof(controls)/sizeof(controls[0]); i++)
	{
		switch(controls[i].id)
		{
			case IDC_BMP_LOGO:
				controls[i].style &= ~WS_VISIBLE;
				if (pcd->hbmp) controls[i].style |= WS_VISIBLE;
				break;
			case IDC_LBL_GREATING:
				controls[i].pTitle = pcs->lpszName;
				break;
			case IDC_EDT_USERNAME:
				controls[i].exStyle &= ~WS_EX_CLIENTEDGE;
				if (0 == (CDS_SKINDIALOG & pcd->flags)) controls[i].exStyle |= WS_EX_CLIENTEDGE;
				controls[i].pTitle = (CDS_USEUSERNAME & pcd->flags) ? pcd->szUser : NULL;
				break;
			case IDC_EDT_PASSWORD:
				controls[i].exStyle &= ~WS_EX_CLIENTEDGE;
				if (0 == (CDS_SKINDIALOG & pcd->flags)) controls[i].exStyle |= WS_EX_CLIENTEDGE;
				controls[i].pTitle = (CDS_USEPASSWORD & pcd->flags) ? pcd->szPassword : NULL;
				break;
			case IDOK:
			case IDCANCEL:
				controls[i].style &= ~BS_OWNERDRAW;
				if (CDS_SKINDIALOG & pcd->flags) controls[i].style |= BS_OWNERDRAW;
				break;
		}

		HWND hwndCtrl = CreateWindowExW(controls[i].exStyle, controls[i].pClass, controls[i].pTitle, controls[i].style,
							0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)controls[i].id, NULL, NULL);
		if (IsWindow(hwndCtrl))
		{
			SendMessageW(hwndCtrl, WM_SETFONT, (WPARAM)hf, FALSE); 
			if ((CDS_SKINDIALOG & pcd->flags) && uxSetWindowTheme) uxSetWindowTheme(hwndCtrl, L" ", L" ");
		}
	}

	if (pcd->hbmp) SendDlgItemMessage(hwnd, IDC_BMP_LOGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)pcd->hbmp);
	
	if (uxdll) FreeLibrary(uxdll);
	pcd->retcode = -4; //- we are running
	return 0;
}

static void LayoutWindows(HWND hwnd)
{
	int i;
	RECT rc, ri;

	GetClientRect(hwnd, &rc);

	for (i = 0; i < sizeof(controls)/sizeof(controls[0]); i++)
	{
		HWND hwndCtrl = GetDlgItem(hwnd, controls[i].id);
		switch(controls[i].id)
		{
			case IDC_BMP_LOGO:
				SetRect(&ri, rc.left, rc.top, rc.right, rc.top + BITMAP_HEIGHT);
				if (WS_VISIBLE & GetWindowLongPtr(hwndCtrl, GWL_STYLE)) rc.top = ri.bottom + 2;
				InflateRect(&rc, -8, -8);
				break;

			case IDC_LBL_GREATING:
				SetRect(&ri, rc.left, rc.top, rc.right, rc.top + 36);
				rc.top = ri.bottom + 12;
				break;
			case IDC_LBL_USERNAME:
			case IDC_LBL_PASSWORD:
				SetRect(&ri, rc.left, rc.top, rc.left + 84, rc.top + 20);
				break;
			case IDC_EDT_USERNAME:
			case IDC_EDT_PASSWORD:
				ri.left = rc.left + 92;
				SetRect(&ri, ri.left, rc.top, rc.right, rc.top + 20);
				rc.top = ri.bottom + 8;
				break;
			case IDOK:
				SetRect(&ri, rc.right - 150, rc.bottom - 22, rc.right - 78, rc.bottom);
				break;
			case IDCANCEL:
				SetRect(&ri, rc.right - 72, rc.bottom - 22, rc.right, rc.bottom);
				break;
		}
		SetWindowPos(hwndCtrl, NULL, ri.left, ri.top, ri.right - ri.left, ri.bottom - ri.top, SWP_NOACTIVATE | SWP_NOZORDER); 
	}
}
static LRESULT WINAPI WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PCREDDATA pcd;
	pcd = (PCREDDATA)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if(pcd && (CDS_SKINDIALOG & pcd->flags))
	{
		INT_PTR a;
		a = WADlg_handleDialogMsgs(hwnd, uMsg, wParam, lParam); 
		if (a) return a;
	}
	switch(uMsg)
	{
		case WM_CREATE: return OnCreate(hwnd, (LPCREATESTRUCTW) lParam);
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					if (pcd)
					{
						pcd->retcode = 1;
						SetLastError(0);
						GetWindowTextW(GetDlgItem(hwnd, IDC_EDT_USERNAME), pcd->szUser, pcd->cchUser);
						if (GetLastError())  pcd->retcode = -1;
						GetWindowTextW(GetDlgItem(hwnd, IDC_EDT_PASSWORD), pcd->szPassword, pcd->cchPassword);
						if (GetLastError())  pcd->retcode = -1;
					}
				case IDCANCEL:
					if (pcd && IDCANCEL == LOWORD(wParam)) pcd->retcode = 0;
					DestroyWindow(GetParent(hwnd));
					return 0;
			}
			break;
		case WM_CLOSE:
			if (pcd) pcd->retcode = 0;
			DestroyWindow(GetParent(hwnd));
			break;
		case WM_WINDOWPOSCHANGED:
			LayoutWindows(hwnd);
			return 0;
		case WM_PAINT:
			if(pcd && (CDS_SKINDIALOG & pcd->flags))
			{
				int tab[] = { IDC_EDT_USERNAME | DCW_SUNKENBORDER , IDC_EDT_PASSWORD | DCW_SUNKENBORDER};
				WADlg_DrawChildWindowBorders(hwnd, tab, 2); 
			}
			break;
		case WM_ERASEBKGND:
			if(pcd && (CDS_SKINDIALOG & pcd->flags))
			{
				RECT rc;
				GetClientRect(hwnd, &rc);
				FillRect((HDC)wParam, &rc, (HBRUSH) SendMessage(hwnd, WM_CTLCOLORDLG, wParam, (LPARAM)hwnd));
				return 1;
			}
			break;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static INT_PTR WINAPI DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_WINDOWPOSCHANGED:
			{
				RECT rc;
				HWND hwndChild;
				hwndChild = FindWindowExW(hwnd, NULL, CREDCTRL, NULL);
				if (hwndChild)
				{
					GetClientRect(hwnd, &rc);
					SetWindowPos(hwndChild, NULL, 0,0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
				}
			}
			break;
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDCANCEL:
				case IDOK:
					{
						HWND hwndChild;
						hwndChild = FindWindowExW(hwnd, NULL, CREDCTRL, NULL);
						if (hwndChild) SendMessage(hwndChild, WM_CLOSE, 0, 0L);
					}
					return TRUE;
			}
			break;
	}
	return 0;
}


INT ShowCredentialDialog(const WACREDDLG *pcd)
{
	HWND hwnd;
	MSG msg;
	WNDLIST list;
	CREDDATA cd;
	embedWindowState embedWnd = {0};

	if (!pcd || pcd->size != sizeof(WACREDDLG) || !pcd->szUser || !pcd->cchUser || !pcd->szPassword || !pcd->cchPassword) return -1;

	ZeroMemory(&cd, sizeof(CREDDATA));

	cd.szUser		= pcd->szUser;
	cd.szPassword	= pcd->szPassword;
	cd.cchUser		= pcd->cchUser;
	cd.cchPassword	= pcd->cchPassword;
	cd.flags		= pcd->flags;
	cd.greating		= pcd->greating;
	cd.hbmp			= pcd->hbmp;
	cd.retcode		= -1;

	if ((CDS_SKINDIALOG & pcd->flags) && pcd->hwndWA)
	{
		ZeroMemory(&embedWnd, sizeof(embedWindowState));
		SetRect(&embedWnd.r, 0, 0, 1, 1);
		embedWnd.flags = EMBED_FLAGS_NORESIZE | EMBED_FLAGS_NOTRANSPARENCY | EMBED_FLAGS_NOWINDOWMENU;
		hwnd = (HWND)SendMessage(pcd->hwndWA, WM_WA_IPC, (LPARAM)&embedWnd, IPC_GET_EMBEDIF);
	}
	else hwnd = NULL;

	if(!IsWindow(hwnd)) hwnd = CreateDialogIndirect(GetModuleHandleW(NULL), (LPCDLGTEMPLATEW)&dlg_template, pcd->hwndParent, DialogProc);

	if (IsWindow(hwnd))
	{
		RECT rw;
		INT fx, fy;

		HWND hwndActive = GetActiveWindow();

		SetWindowTextW(hwnd, pcd->title);

		GetClientRect(hwnd, &rw);

		HWND hwndCtrl = CreateCredCtrl(&cd, 0, 0, rw.right, rw.bottom, hwnd, GetModuleHandleW(NULL), IDC_CREDCTRL);		

		if ((CDS_SKINDIALOG & pcd->flags))
		{
			SetWindowPos(hwnd, NULL, 0, 0, 100, 100, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
			GetWindowRect(hwnd, &rw);
			fx = rw.right - rw.left;
			fy = rw.bottom - rw.top;
			GetWindowRect(hwndCtrl, &rw);
			fx -= (rw.right - rw.left);
			fy -= (rw.bottom - rw.top);
		}
		else
		{
			GetWindowRect(hwnd, &rw);
			fx = rw.right - rw.left;
			fy = rw.bottom - rw.top;
			GetClientRect(hwnd, &rw);
			fx -= (rw.right - rw.left);
			fy -= (rw.bottom - rw.top);
		}

		if (pcd->hwndParent) GetWindowRect(pcd->hwndParent, &rw);
		else GetWindowRect(GetDesktopWindow(), &rw);

		SetWindowPos(hwnd, HWND_TOP, 
							rw.left + (rw.right - rw.left - (DIALOG_WIDTH + fx)) / 2,
							rw.top + (rw.bottom - rw.top - (DIALOG_HEIGHT + fy - ((pcd->hbmp) ? 0 : BITMAP_HEIGHT))) / 2,
							DIALOG_WIDTH + fx, 
							(DIALOG_HEIGHT + fy  - ((pcd->hbmp) ? 0 : BITMAP_HEIGHT)), 
							0);

		ShowWindow(hwnd, SW_SHOW);

		if (CDS_APPMODAL & pcd->flags) BlokedWndLst_Initialize(&list, hwnd);
		else if (pcd->hwndParent) EnableWindow(GetRoot(pcd->hwndParent), FALSE);

		BOOL result;
		while(0 != (result = GetMessage(&msg, NULL, 0, 0)) && -1 != result)
		{
			if (!IsDialogMessage(hwnd, &msg))
			{
				if ((CDS_APPMODAL & pcd->flags) && WM_TIMER != msg.message) BlokedWndLst_Add(&list, msg.hwnd);
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if (CDS_APPMODAL & pcd->flags) BlokedWndLst_RemoveAll(&list);
		else if (pcd->hwndParent) EnableWindow(GetRoot(pcd->hwndParent), TRUE);

		if (hwndActive) SetActiveWindow(hwndActive);

		if (!result) 
		{
			if (-4 == cd.retcode)
			{
				PostQuitMessage((INT)msg.wParam);
			}
		}
	}
	if (-4 == cd.retcode) cd.retcode = 0;
	return cd.retcode;
}
