#include "main.h"
#include ".\cddbui.h"
#include ".\cddb.h"
#include "api__in_cdda.h"
#include ".\resource.h"
#include "..\winamp\wa_ipc.h"

#include <api/application/api_application.h>

#include <shobjidl.h>
#include <commctrl.h>
#include <strsafe.h>

#include "cddbcontrolwinamp.tlh"

#define PROP_PRGDLG	L"PRGDLG"


#define TIMER_PROGRESS_DESTROY_ID		1978
#define TIMER_PROGRESS_DESTROY_ELAPSE	250
#define TIMER_PROGRESS_ANIMATE_ID		1980
#define TIMER_PROGRESS_ANIMATE_ELAPSE	65

#define MODAL_EXIT		0
#define MODAL_ACTIVE		1
#define MODAL_DESTROY	2

#define ICON_OFFSET_X		12
#define ICON_OFFSET_Y		12

#define DIALOG_HEIGHT_NORMAL			66
#define DIALOG_HEIGHT_EXTENDED		160

#ifndef IDC_HAND
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif //IDC_HAND



typedef struct _PROGRESSICON
{
	HINSTANCE hInstance;
	LONG	resId;
	RECT	rc;
	LONG	frames;
	LONG	step;
	HBITMAP	hbmp;
} PROGRESSICON;

typedef  struct _PROGRESSDLG
{
	PROGRESSICON		icon;
	UINT				uState;
	DWORD				dwAutoClose;
	CDDBDLG_ONBTNCLICK	OnAbort;
	CDDBDLG_ONBTNCLICK	OnButton1;
	BSTR				Btn1Data;
	BSTR				AbortData;
	WORD					Modal;
	HRESULT				rCode;
	HANDLE				user;
} PROGRESSDLG;

static HFONT hFont	= NULL;
static LONG fontRef = 0;

static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void CALLBACK ProgressDlg_OnTimer(HWND hwnd, UINT uMsg, UINT_PTR evntId, DWORD dwTime);
static void InvalidateLogo(HWND hwnd, PROGRESSICON *pIcon);
static BOOL EndProgressDialog(HWND hwnd);

#define GET_DATA(__hwnd) ((PROGRESSDLG*)GetPropW((__hwnd), PROP_PRGDLG))

static wchar_t szText1[256];
static wchar_t szText2[256];

#define GET_SAFE_LANGSTRING(__str, __buff, __cch) ((__str) ? (IS_INTRESOURCE(__str) ? WASABI_API_LNGSTRINGW_BUF((UINT)(UINT_PTR)(__str), (__buff), (__cch)) : (__str)) : L"")
#define GET_SAFE_LANGSTRING1(__str) GET_SAFE_LANGSTRING((__str), (szText1), (sizeof(szText1)/sizeof(wchar_t)))
#define GET_SAFE_LANGSTRING2(__str) GET_SAFE_LANGSTRING((__str), (szText2), (sizeof(szText2)/sizeof(wchar_t)))


typedef struct _ENUMWND_DATAPACK
{
	HWND		host;
	HWND		*list;
	INT		index;
	INT		count;
	UINT	flags;
	BOOL	found;
} ENUMWND_DATAPACK;

HWND CddbProgressDlg_Create(HWND hwndParent, INT nCmdShow)
{
	HWND hdlg = WASABI_API_CREATEDIALOGPARAMW(IDD_CDDB_PROGRESS, NULL, DialogProc, 0L);
	if (hdlg && SW_HIDE != nCmdShow) ShowWindow(hdlg, SW_HIDE);
	return hdlg;
}

BOOL CddbProgressDlg_Initialize(HWND hwnd, LPCWSTR pszCaption, CDDBDLG_ONBTNCLICK fnOnAbort, BSTR bstrAbortUser)
{
	HWND hwndCtrl;
	PROGRESSDLG *pDlg;

	if(!hwnd || !IsWindow(hwnd)) return FALSE;
	
	pDlg = GET_DATA(hwnd);
	if (!pDlg) return FALSE;

	KillTimer(hwnd, TIMER_PROGRESS_ANIMATE_ID);
	pDlg->OnAbort = (fnOnAbort) ? fnOnAbort : NULL;
	if (pDlg->AbortData) SysFreeString(pDlg->AbortData);
	pDlg->AbortData = (bstrAbortUser) ? SysAllocString(bstrAbortUser) : NULL;
		
	SetDlgItemTextW(hwnd, IDC_LBL_CAPTION, GET_SAFE_LANGSTRING1(pszCaption));
	SetDlgItemTextW(hwnd, IDC_LBL_STATUS, L"");
	
	if (NULL != (hwndCtrl = GetDlgItem(hwnd, IDC_LBL_REASON)))
	{
		ShowWindow(hwndCtrl, SW_HIDE);
		SetWindowTextW(hwndCtrl, L"");
	}

	if (NULL != (hwndCtrl = GetDlgItem(hwnd, IDC_PRG_STATUS)))
	{
		ShowWindow(hwndCtrl, SW_HIDE);
		SendMessageW(hwndCtrl, PBM_SETPOS, (WPARAM)0, (LPARAM)0L);
		
	}

	if (NULL != (hwndCtrl = GetDlgItem(hwnd, IDCANCEL)))
	{		
		SetWindowTextW(hwndCtrl, GET_SAFE_LANGSTRING1(MAKEINTRESOURCEW((fnOnAbort) ? IDS_ABORT : IDS_CLOSE)));
		EnableWindow(hwndCtrl, (NULL != fnOnAbort));
	}
	if (NULL != (hwndCtrl = GetDlgItem(hwnd, IDC_LV_EXT)))
	{		
		SendMessageW(hwndCtrl, LVM_DELETEALLITEMS, 0, 0L);
	}
	
	
	pDlg->icon.step = 0;
	InvalidateLogo(hwnd, &pDlg->icon);
	SetTimer(hwnd, TIMER_PROGRESS_ANIMATE_ID, TIMER_PROGRESS_ANIMATE_ELAPSE, ProgressDlg_OnTimer);
	
	pDlg->uState = STATE_ACTIVE;
	return TRUE;
}

BOOL CddbProgressDlg_Completed(HWND hwnd, LPCWSTR pszResult, LPCWSTR pszReason, DWORD nAutoCloseDelay, HRESULT rCode)
{	
	PROGRESSDLG *pDlg;
	if(!hwnd || !IsWindow(hwnd)) return FALSE;

	pDlg = GET_DATA(hwnd);
	if (!pDlg) return FALSE;
	
	KillTimer(hwnd, TIMER_PROGRESS_ANIMATE_ID);

	pDlg->uState = STATE_COMPLETED;
	pDlg->rCode = rCode;

	if (AUTOCLOSE_NOW == nAutoCloseDelay) 
	{
		EndProgressDialog(hwnd);
	}
	else
	{
		
		HWND hwndCtrl;
		SetDlgItemTextW(hwnd, IDC_LBL_STATUS, GET_SAFE_LANGSTRING1(pszResult));
		if (NULL != (hwndCtrl = GetDlgItem(hwnd, IDC_PRG_STATUS)))
		{
			ShowWindow(hwndCtrl, SW_HIDE);
			SendMessageW(hwndCtrl, PBM_SETPOS, (WPARAM)0, (LPARAM)0L);
		}
		if (NULL != (hwndCtrl = GetDlgItem(hwnd, IDC_LBL_REASON)))
		{
			SetWindowTextW(hwndCtrl, GET_SAFE_LANGSTRING1(pszReason));
			ShowWindow(hwndCtrl, SW_SHOWNORMAL);
		}
		if (NULL != (hwndCtrl = GetDlgItem(hwnd, IDCANCEL)))
		{
			
			if (AUTOCLOSE_NEVER != nAutoCloseDelay)
			{				
				INT time = nAutoCloseDelay/1000 + (((nAutoCloseDelay%1000)>500) ? 1 : 0);
				if (time) 
				{
					wchar_t szText[128] = {0};
					StringCchPrintfW(szText, sizeof(szText)/sizeof(wchar_t), L"%s (%d)", GET_SAFE_LANGSTRING1(MAKEINTRESOURCEW(IDS_CLOSE)),time);
					SetDlgItemTextW(hwnd, IDCANCEL, szText);
				}
				else SetWindowText(hwndCtrl, GET_SAFE_LANGSTRING1(MAKEINTRESOURCEW(IDS_CLOSE)));
			}
			else SetWindowText(hwndCtrl, GET_SAFE_LANGSTRING1(MAKEINTRESOURCEW(IDS_CLOSE)));
			EnableWindow(hwndCtrl, TRUE);
		}

		
		pDlg->icon.step = pDlg->icon.frames - 1;
		InvalidateLogo(hwnd, &pDlg->icon);
		
		if (AUTOCLOSE_NEVER != nAutoCloseDelay) 
		{
			pDlg->dwAutoClose = GetTickCount() + nAutoCloseDelay;
			return (0 != SetTimer(hwnd, TIMER_PROGRESS_DESTROY_ID, TIMER_PROGRESS_DESTROY_ELAPSE, ProgressDlg_OnTimer));
		}
	}
	return TRUE;
}

BOOL CddbProgressDlg_SetStatus(HWND hwnd, LPCWSTR pszStatus, INT nPercentCompleted)
{
	BOOL br(TRUE);
	HWND hwndCtrl;
	if (!hwnd || !IsWindow(hwnd)) return FALSE;
	if (pszStatus && !SetDlgItemTextW(hwnd, IDC_LBL_STATUS, GET_SAFE_LANGSTRING1(pszStatus))) br = FALSE;
	hwndCtrl = GetDlgItem(hwnd, IDC_PRG_STATUS);
	if (hwndCtrl)
	{
		if (nPercentCompleted > 100)  nPercentCompleted = 100;
		if (nPercentCompleted < 0) ShowWindow(hwndCtrl, SW_HIDE);
		else 
		{
			SendMessageW(hwndCtrl, PBM_SETPOS, (WPARAM)nPercentCompleted, (LPARAM)0L);
			ShowWindow(hwndCtrl, SW_SHOWNA);
		}
	}
	return br;
}

UINT CddbProgressDlg_GetState(HWND hwnd)
{
	PROGRESSDLG *pDlg;
	if(!hwnd || !IsWindow(hwnd)) return STATE_INACTIVE;
	pDlg = GET_DATA(hwnd);
	return (pDlg) ? pDlg->uState : STATE_INACTIVE;
}

BOOL CddbProgressDlg_EnableAbortButton(HWND hwnd, BOOL bEnable)
{
	HWND hwndBtn;
	if(!hwnd || !IsWindow(hwnd)) return FALSE;
	if (NULL == (hwndBtn = GetDlgItem(hwnd, IDCANCEL))) return FALSE;
	return EnableWindow(hwndBtn, bEnable);
}

BOOL CddbProgressDlg_ShowButton1(HWND hwnd, LPCWSTR pszCaption, CDDBDLG_ONBTNCLICK fnOnButton1, BSTR bstrUser)
{
	PROGRESSDLG *pDlg;
	HWND hwndBtn;

	if(!hwnd || !IsWindow(hwnd)) return FALSE;
	pDlg = GET_DATA(hwnd);
	if (!pDlg) return FALSE;

	if (NULL == (hwndBtn = GetDlgItem(hwnd, IDC_BUTTON1))) return FALSE;
	
	if (pDlg->Btn1Data) SysFreeString(pDlg->Btn1Data);

	if(pszCaption && fnOnButton1) 
	{
		SetWindowTextW(hwndBtn, GET_SAFE_LANGSTRING1(pszCaption));
		ShowWindow(hwndBtn, SW_SHOWNORMAL);
		pDlg->OnButton1 = fnOnButton1;
		pDlg->Btn1Data = (bstrUser) ? SysAllocString(bstrUser) : NULL;
		SendMessageW(hwnd, WM_NEXTDLGCTL, (WPARAM)TRUE, (LPARAM)hwndBtn);
	}
	else 
	{
		ShowWindow(hwndBtn, SW_HIDE);
		pDlg->OnButton1 = NULL;
		pDlg->Btn1Data = NULL;

	}
	return TRUE;
}

BOOL CddbProgressDlg_ShowInTaskbar(HWND hwnd, BOOL bShow)
{
	HRESULT hr;
	ITaskbarList *pTaskbar;
	if(!hwnd || !IsWindow(hwnd)) return FALSE;
	hr = CoCreateInstance(CLSID_TaskbarList,0, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (void**)&pTaskbar);
	if(SUCCEEDED(hr))
	{
		hr = pTaskbar->HrInit();
		if (SUCCEEDED(hr))
		{
			if (bShow)
			{
				hr = pTaskbar->AddTab(hwnd);
				pTaskbar->ActivateTab(hwnd);

			}
			else pTaskbar->DeleteTab(hwnd);
		}
		pTaskbar->Release();
	}
	return (S_OK == hr);
}
BOOL CddbProgressDlg_SetExtendedMode(HWND hwnd, BOOL bEnable)
{
	RECT rc;
	HWND hwndCtrl;
	INT height;
	if(!hwnd || !IsWindow(hwnd)) return FALSE;
	GetWindowRect(hwnd, &rc);

	RECT rw;
	GetClientRect(hwnd, &rw);
	height = (rc.bottom - rc.top)  - (rw.bottom - rw.top);
	SetRect(&rw, 0, 0, 1, (bEnable) ? DIALOG_HEIGHT_EXTENDED : DIALOG_HEIGHT_NORMAL);
	MapDialogRect(hwnd, &rw);
	height += rw.bottom;

	if (height == rc.bottom - rc.top) return TRUE;

	if (NULL != (hwndCtrl = GetDlgItem(hwnd, IDC_BUTTON1)))
	{
		GetWindowRect(hwndCtrl, &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
		SetWindowPos(hwndCtrl, NULL, rw.left, rw.top + (height - (rc.bottom - rc.top)), 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);	
	}

	if (NULL != (hwndCtrl = GetDlgItem(hwnd, IDCANCEL)))
	{
		GetWindowRect(hwndCtrl, &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
		SetWindowPos(hwndCtrl, NULL, rw.left, rw.top + (height - (rc.bottom - rc.top)), 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);	
	}
	
	if (NULL != (hwndCtrl = GetDlgItem(hwnd, IDC_LV_EXT)))
	{
		PROGRESSDLG *pDlg;
		INT listBottom;
		GetWindowRect(GetDlgItem(hwnd, IDCANCEL), &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 1);
		listBottom = rw.top - MulDiv(4, HIWORD(GetDialogBaseUnits()), 8);
		
		pDlg = GET_DATA(hwnd);
		if (pDlg)
		{
			GetWindowRect(hwndCtrl, &rw);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
			INT listTop = ICON_OFFSET_Y * 2 + (pDlg->icon.rc.bottom - pDlg->icon.rc.top);
			SetWindowPos(hwndCtrl, NULL, ICON_OFFSET_X, listTop, rc.right - rc.left - ICON_OFFSET_X*2, listBottom - listTop, SWP_NOACTIVATE | SWP_NOZORDER);
			ShowWindow(hwndCtrl, (bEnable) ? SW_SHOW : SW_HIDE);
		}
	}
	
	SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, height, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	

	return TRUE;
}
BOOL CddbProgressDlg_AddRecord(HWND hwnd, LPCWSTR pszArtist, LPCWSTR pszTitle, LPCWSTR pszLanguage)
{
	HWND hwndList;
	LVITEMW item;
	INT index;
	if(!hwnd || !IsWindow(hwnd)) return FALSE;
	
	hwndList  = GetDlgItem(hwnd, IDC_LV_EXT);
	if (!hwndList) return FALSE;
	
	item.mask		= LVIF_TEXT;
	item.iItem		= 0xFFFF;
	item.iSubItem	= 0;
	item.pszText		= GET_SAFE_LANGSTRING1(pszArtist);
	index = (INT)SendMessageW(hwndList, LVM_INSERTITEMW, 0, (LPARAM)&item);
	if (-1 == index) return FALSE;

	if (0 == index)
	{
		item.state = LVIS_FOCUSED | LVIS_SELECTED;
		item.stateMask = item.state;
		SendMessageW(hwndList, LVM_SETITEMSTATE, (WPARAM)index, (LPARAM)&item);
	}
	item.iItem = index;
	item.mask = LVIF_TEXT;
	item.iSubItem = 1;
	item.pszText = GET_SAFE_LANGSTRING1(pszTitle);;
	SendMessageW(hwndList, LVM_SETITEMW, 0, (LPARAM)&item);

	item.iItem = index;
	item.mask = LVIF_TEXT;
	item.iSubItem = 2;
	item.pszText = GET_SAFE_LANGSTRING1(pszLanguage);;
	SendMessageW(hwndList, LVM_SETITEMW, 0, (LPARAM)&item);
	
	return TRUE;
}


#define HOOK_MAX_DATA	12
typedef struct _HOOKDATA
{
	HHOOK	handle;
	int		ref;
	HWND		modalList[HOOK_MAX_DATA];
} HOOKDATA;

static HOOKDATA g_hook = { NULL, 0};

static LRESULT CALLBACK HookProc(int code, WPARAM wParam, LPARAM lParam) 
{
	MOUSEHOOKSTRUCT *pMouse = (MOUSEHOOKSTRUCT*)lParam;
	switch(wParam)
	{
		case WM_NCLBUTTONDOWN:
		case WM_NCLBUTTONDBLCLK:
		case WM_NCRBUTTONDOWN:
		case WM_NCRBUTTONDBLCLK:
		case WM_NCMBUTTONDOWN:
		case WM_NCMBUTTONDBLCLK:
			if (FALSE == IsWindowEnabled(pMouse->hwnd))
			{
				
				for(int i = g_hook.ref - 1; i > -1 ; i--)
				{
					HWND hwndOwner = GetWindow(g_hook.modalList[i], GW_OWNER);
					if (hwndOwner == pMouse->hwnd || hwndOwner == GetWindow(pMouse->hwnd, GW_OWNER))
					{
						DWORD style = GetWindowLongPtrW(g_hook.modalList[i], GWL_STYLE);
						if (0 != (WS_VISIBLE & style) && 0 == (WS_DISABLED & style)) 
						{
							HWND hwndTest = GetForegroundWindow();
							if (hwndTest == g_hook.modalList[i])
							{
								FLASHWINFO flash;
								flash.hwnd = g_hook.modalList[i];
								flash.cbSize = sizeof(FLASHWINFO);
								flash.dwFlags = FLASHW_CAPTION;
								flash.uCount = 2;
								flash.dwTimeout = 100;
								FlashWindowEx(&flash);
								MessageBeep(MB_OK);
							}
							else SetForegroundWindow(g_hook.modalList[i]); 
							return CallNextHookEx(g_hook.handle, code, wParam, lParam); 
						}
					}
				}
			}
			break;
	}
	return CallNextHookEx(g_hook.handle, code, wParam, lParam); 
}
static void AddModalHook(HWND hdlg)
{
	if (!hdlg) return;
	if (!g_hook.handle)
	{
		ZeroMemory(&g_hook, sizeof(HOOKDATA));
		g_hook.handle = SetWindowsHookEx(WH_MOUSE, HookProc, line.hDllInstance, NULL);
		if (!g_hook.handle) return;
	}
	if (HOOK_MAX_DATA == (g_hook.ref - 1)) return;
	g_hook.modalList[g_hook.ref] = hdlg;
	g_hook.ref++;
	return;
}

static void ReleaseModalHook(HWND hdlg)
{
	if (!hdlg) return;
	for (int i = 0; i < g_hook.ref; i++)
	{
		if (g_hook.modalList[i] == hdlg) 
		{
			if (i != g_hook.ref -1) MoveMemory(&g_hook.modalList[i], &g_hook.modalList[i + 1], (g_hook.ref - i -1)*sizeof(HWND));

			g_hook.ref--;
			if (!g_hook.ref)
			{
				UnhookWindowsHookEx(g_hook.handle);
				ZeroMemory(&g_hook, sizeof(HOOKDATA));
			}
			return;
		}
	}
	return ;
}

HRESULT CddbProgressDlg_DoModal(HWND hwnd, RECT *prc)
{
	MSG msg;
	HWND hwndOwner;

	PROGRESSDLG *pDlg;
	HRESULT rCode;
	HWND disabledList[32] = {0};

	if(!hwnd || !IsWindow(hwnd)) return E_INVALIDARG;
	pDlg = GET_DATA(hwnd);
	if (!pDlg || MODAL_ACTIVE == pDlg->Modal) return E_POINTER;
		
	pDlg->Modal = MODAL_ACTIVE;
	hwndOwner = GetParent(hwnd);
	if (hwndOwner == GetDesktopWindow()) hwndOwner = NULL;
	if (hwndOwner != line.hMainWindow && 
		hwndOwner == (HWND)SendMessageW(line.hMainWindow, WM_WA_IPC, 0, IPC_GETDIALOGBOXPARENT)) 
	{
		hwndOwner = line.hMainWindow;
	}

	DWORD mineTID, parentTID;
	mineTID = GetWindowThreadProcessId(hwnd, NULL);
	parentTID = (hwndOwner) ? GetWindowThreadProcessId(hwndOwner, NULL) : mineTID;
	if (hwndOwner) 
	{
		HWND *p;
		if (mineTID != parentTID) AttachThreadInput(parentTID, mineTID, TRUE);
		p = disabledList;
		if (IsWindowEnabled(hwndOwner)) 
		{
			*p = hwndOwner;
			p++;
		}
		FindAllOwnedWindows(hwndOwner, p, sizeof(disabledList)/sizeof(HWND) - (INT)(p - disabledList), FINDWND_ONLY_ENABLED);
		for (p = disabledList; *p != NULL; p++) { if (hwnd != *p) EnableWindow(*p, FALSE); }
	}

	AddModalHook(hwnd);

	msg.message = WM_NULL;
	while(MODAL_ACTIVE == pDlg->Modal)
	{
		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			else if (!CallMsgFilter(&msg, MSGF_DIALOGBOX) &&
					!IsDialogMessage(hwnd, &msg)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else if (MODAL_ACTIVE == pDlg->Modal)  WaitMessage();
	}

	ReleaseModalHook(hwnd);

	rCode = pDlg->rCode;

	if (msg.message == WM_QUIT) PostQuitMessage((int)msg.wParam);
	if (hwndOwner)
	{
		if (mineTID != parentTID) AttachThreadInput(parentTID, mineTID, FALSE);
		for (HWND *p = disabledList; *p != NULL; p++) { if (hwnd != *p) EnableWindow(*p, TRUE); }
		SetActiveWindow(hwndOwner);
	}

	if (prc && !GetWindowRect(hwnd, prc)) SetRect(prc, 0, 0, 0,0);

	if(MODAL_EXIT != pDlg->Modal) DestroyWindow(hwnd);
	return rCode;
}

BOOL CddbProgressDlg_ExitModal(HWND hwnd, HRESULT rCode, BOOL bDesroy)
{
	PROGRESSDLG *pDlg;
	if(!hwnd || !IsWindow(hwnd)) return FALSE;
	pDlg = GET_DATA(hwnd);
	if (!pDlg) return FALSE;
	if (MODAL_ACTIVE == pDlg->Modal)
	{
		pDlg->Modal = (bDesroy) ? MODAL_DESTROY : MODAL_EXIT;
		pDlg->rCode = rCode;
		PostMessageW(hwnd, WM_NULL, 0, 0);
	}
	return TRUE;
}

BOOL CddbProgressDlg_IsModal(HWND hwnd)
{
	PROGRESSDLG *pDlg;
	if(!hwnd || !IsWindow(hwnd)) return FALSE;
	pDlg = GET_DATA(hwnd);
	return (pDlg && (MODAL_ACTIVE == pDlg->Modal));
}

INT CddbProgressDlg_GetSelRecordIndex(HWND hwnd)
{
	HWND hwndList;
	if(!hwnd || !IsWindow(hwnd)) return -1;
	hwndList = GetDlgItem(hwnd, IDC_LV_EXT);
	if (!hwndList) return -1;
	return (INT)(INT_PTR)SendMessageW(hwndList, LVM_GETNEXTITEM, -1, (LPARAM)(LVNI_SELECTED | LVNI_FOCUSED));
}

BOOL CddbProgressDlg_SetUserData(HWND hwnd, HANDLE user)
{
	PROGRESSDLG *pDlg;
	if(!hwnd || !IsWindow(hwnd)) return FALSE;
	pDlg = GET_DATA(hwnd);
	if (!pDlg) return FALSE;
	pDlg->user = user;
	return TRUE;
}

HANDLE CddbProgressDlg_GetUserData(HWND hwnd)
{
	PROGRESSDLG *pDlg;
	if(!hwnd || !IsWindow(hwnd)) return NULL;
	pDlg = GET_DATA(hwnd);
	return (pDlg) ? pDlg->user : NULL;
}

static void InvalidateLogo(HWND hwnd, PROGRESSICON *pIcon)
{
	RECT rc;
	SetRect(&rc, ICON_OFFSET_X, ICON_OFFSET_Y, 
						ICON_OFFSET_X + (pIcon->rc.right - pIcon->rc.left), 
						ICON_OFFSET_Y + (pIcon->rc.bottom - pIcon->rc.top));
	InvalidateRect(hwnd, &rc, TRUE);
}

static BOOL EnableWindowTheme(HWND hwnd, BOOL bEnable)
{
	static HMODULE hModule = NULL;
	static BOOL firstTime = TRUE;
	static HRESULT (WINAPI *__setwintheme)(HWND, LPCWSTR, LPCWSTR) = NULL;

	if (!hModule)
	{
		if (!firstTime) return FALSE;
		firstTime = FALSE;
		hModule = LoadLibraryW(L"UxTheme.dll");
		if (!hModule) return FALSE;
		__setwintheme = (HRESULT (WINAPI *)(HWND, LPCWSTR, LPCWSTR))GetProcAddress(hModule, "SetWindowTheme");
		if (!__setwintheme) 
		{
			FreeLibrary(hModule);
			hModule = NULL;
			return FALSE;
		}
	}
	return (S_OK == __setwintheme(hwnd, NULL, ((bEnable) ? NULL : L"")));
}

static HRESULT InitializeProgressIcon(PROGRESSICON *pIcon)
{
	HRESULT hr;
	LONG/*_PTR*/ lVal; // benski> windows 64 isn't supported by gracenote

	ICddbUIOptions *pUIOptions;

	if (!pIcon) return E_INVALIDARG;
	ZeroMemory(pIcon, sizeof(PROGRESSICON));

	hr = Cddb_GetIUIOptions((void**)&pUIOptions);
	if (FAILED(hr)) return hr;

	hr = pUIOptions->GetCurrent(UI_DISP_PROGRESS);
	if (SUCCEEDED(hr)) 
	{
		if (SUCCEEDED(pUIOptions->get_ResourceHINSTANCE(&lVal))) pIcon->hInstance = (HINSTANCE)lVal;
		if (SUCCEEDED(pUIOptions->get_Left(&lVal))) pIcon->rc.left = (LONG)lVal;
		if (SUCCEEDED(pUIOptions->get_Top(&lVal))) pIcon->rc.top = (LONG)lVal;
		if (SUCCEEDED(pUIOptions->get_Right(&lVal))) pIcon->rc.right = (LONG)lVal;
		if (SUCCEEDED(pUIOptions->get_Bottom(&lVal))) pIcon->rc.bottom = (LONG)lVal;
		pUIOptions->get_ProgressResourceID(&pIcon->resId);
		pUIOptions->get_Frames(&pIcon->frames);
	}
	pUIOptions->Release();
	return hr;
}

static BOOL AnimateProgressIcon(HDC hdc, INT x, INT y, PROGRESSICON *pIcon)
{
	INT w, h;
	HDC hdcDst;
	HBITMAP bmpOld;

	if (!hdc || !pIcon) return FALSE;

	if (!pIcon->hbmp)
	{
		if (pIcon->hInstance && pIcon->resId)
		{
			pIcon->hbmp = (HBITMAP)LoadImageW(pIcon->hInstance, 	MAKEINTRESOURCEW(pIcon->resId),
								IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		}
		if (!pIcon->hbmp) return FALSE;
	}

	hdcDst = CreateCompatibleDC(hdc);
	bmpOld = (HBITMAP)SelectObject(hdcDst, pIcon->hbmp);

	w = pIcon->rc.right - pIcon->rc.left;
	h = pIcon->rc.bottom - pIcon->rc.top;

	BitBlt(hdc, x, y, w, h, hdcDst, pIcon->rc.left + (pIcon->step * w), pIcon->rc.top, SRCCOPY);
	SelectObject(hdcDst, bmpOld);
	DeleteDC(hdcDst);
	return TRUE;
}

static void CALLBACK ProgressDlg_OnTimer(HWND hwnd, UINT uMsg, UINT_PTR evntId, DWORD dwTime)
{
	PROGRESSDLG *pDlg;
	pDlg = GET_DATA(hwnd);
	if (!pDlg) return;

	DWORD tclose;
	switch(evntId)
	{
		case TIMER_PROGRESS_ANIMATE_ID: 
			InvalidateLogo(hwnd, &pDlg->icon);
			if (++pDlg->icon.step >= pDlg->icon.frames) 
			{
				KillTimer(hwnd, evntId);
				pDlg->icon.step = pDlg->icon.frames - 1;
			}
			break;
		case TIMER_PROGRESS_DESTROY_ID:
			tclose = pDlg->dwAutoClose;
			if (dwTime >= tclose)
			{
				KillTimer(hwnd, evntId);
				EndProgressDialog(hwnd);
			}
			else
			{
				wchar_t szText[128] = {0};
				tclose = (tclose - dwTime)/1000 + ((((tclose - dwTime)%1000) > 500) ? 1 : 0);
				StringCchPrintfW(szText, sizeof(szText)/sizeof(wchar_t), L"%s (%d)", GET_SAFE_LANGSTRING1(MAKEINTRESOURCEW(IDS_CLOSE)), tclose);
				SetDlgItemTextW(hwnd, IDCANCEL, szText);
			}
			break;
	}
}

static BOOL EndProgressDialog(HWND hwnd)
{
	PROGRESSDLG *pDlg;
	pDlg = GET_DATA(hwnd);
	if (!pDlg) return FALSE;

	if (MODAL_ACTIVE == pDlg->Modal)
	{
		pDlg->Modal = MODAL_DESTROY;
		PostMessageW(hwnd, WM_NULL, 0, 0);
	}
	else
	{
		DestroyWindow(hwnd);
	}
	return TRUE;
}

static INT_PTR ProgressDlg_OnDialogInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	HWND hwndCtrl;
	PROGRESSDLG *pDlg(NULL);

	pDlg = (PROGRESSDLG*)calloc(1, sizeof(PROGRESSDLG));	
	if (pDlg)
	{
		pDlg->uState = STATE_INACTIVE;
		if ((FAILED(InitializeProgressIcon(&pDlg->icon)) || !SetPropW(hwnd, PROP_PRGDLG, pDlg)))
		{
			free(pDlg);
			pDlg = NULL;
			DestroyWindow(hwnd);
			return TRUE;
		}
	}
	hwndCtrl = GetDlgItem(hwnd, IDC_PRG_STATUS);
	if (hwndCtrl)
	{
		RECT rc;
		EnableWindowTheme(hwndCtrl, FALSE);
		SetWindowLongPtrW(hwndCtrl, GWL_EXSTYLE, GetWindowLongPtrW(hwndCtrl, GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
		GetWindowRect(hwndCtrl, &rc);
		SetWindowPos(hwndCtrl, NULL, 0, 0, rc.right - rc.left, 3, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
		SendMessageW(hwndCtrl, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		SendMessageW(hwndCtrl, PBM_SETPOS, 0, 0L);
		SendMessageW(hwndCtrl, PBM_SETSTEP, 1, 0L);
		SendMessageW(hwndCtrl, PBM_SETBARCOLOR, 0, (LPARAM)GetSysColor(COLOR_WINDOW));
		SendMessageW(hwndCtrl, PBM_SETBKCOLOR, 0, (LPARAM)GetSysColor(COLOR_WINDOWTEXT));
	}
	hwndCtrl = GetDlgItem(hwnd, IDC_LBL_STATUS);
	if(hwndCtrl)
	{
		if (!hFont)
		{
			HFONT hf;
			LOGFONT lf;

			hf = (HFONT)SendMessageW(hwndCtrl, WM_GETFONT, 0, 0L);
			if (hf) hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			if (hf && GetObject(hf, sizeof(LOGFONT), &lf))
			{
				HDC hdc = GetDC(hwndCtrl);
				lf.lfHeight = (hdc) ? -MulDiv(7, GetDeviceCaps(hdc, LOGPIXELSY), 72) : -9;
				lf.lfWeight = FW_THIN;
				lf.lfQuality = PROOF_QUALITY;
				StringCchCopy(lf.lfFaceName, sizeof(lf.lfFaceName)/sizeof(*lf.lfFaceName), L"Arial");
				hFont = CreateFontIndirect(&lf);
				if (hdc) ReleaseDC(hwnd, hdc);
			}
		}
		if (hFont)
		{
			SendMessageW(hwndCtrl, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
			SendDlgItemMessageW(hwnd, IDC_LBL_REASON, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
			fontRef++;
		}
	}

	if (NULL != (hwndCtrl = GetDlgItem(hwnd, IDC_LV_EXT)))
	{
		DWORD exstyle = LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP;
		SendMessageW(hwndCtrl, LVM_SETEXTENDEDLISTVIEWSTYLE, exstyle, exstyle);
		LVCOLUMNW column;
		column.mask = LVCF_WIDTH | LVCF_TEXT;
		column.cx = 120;
		column.pszText = L"Artist"; 
		SendMessageW(hwndCtrl, LVM_INSERTCOLUMNW, (WPARAM)0xEFFF, (LPARAM)&column);
		column.cx = 160;
		column.pszText = L"Album"; 
		SendMessageW(hwndCtrl, LVM_INSERTCOLUMNW, (WPARAM)0xEFFF, (LPARAM)&column);
		column.cx = 40;
		column.pszText = L"Language"; 
		SendMessageW(hwndCtrl, LVM_INSERTCOLUMNW, (WPARAM)0xEFFF, (LPARAM)&column);
	}

	return TRUE;
}

static void ProgressDlg_OnDestroy(HWND hwnd)
{
	PROGRESSDLG *pDlg;
	pDlg = GET_DATA(hwnd);
	if (pDlg) 
	{
		RemovePropW(hwnd, PROP_PRGDLG);
		if (pDlg->icon.hbmp) DeleteObject(pDlg->icon.hbmp);
		if (pDlg->Btn1Data) SysFreeString(pDlg->Btn1Data);
		if (pDlg->AbortData) SysFreeString(pDlg->AbortData);
		
		free(pDlg);
		pDlg = NULL;
	}

	if (fontRef && 0 == --fontRef) 
	{
		DeleteObject(hFont);
		hFont = NULL;
	}
}

static void ProgressDlg_OnCommand(HWND hwnd, WORD ctrlId, WORD evntId, HWND hwndCtrl)
{
	PROGRESSDLG *pDlg;

	pDlg = GET_DATA(hwnd);
    if (!pDlg) return;

	switch(ctrlId)
	{
		case IDCANCEL:
			pDlg->rCode = S_FALSE;
			switch(pDlg->uState)
			{
				case STATE_ACTIVE:
					if (!pDlg->OnAbort) return; 
					SetWindowTextW(hwndCtrl, WASABI_API_LNGSTRINGW(IDS_ABORTING));
					EnableWindow(hwndCtrl, FALSE);
					pDlg->uState = STATE_ABORTING;
					pDlg->OnAbort(hwnd, pDlg->AbortData);
					return;
				case STATE_ABORTING: return; // don't do anything
			}
			;
			EndProgressDialog(hwnd);
			break;
		case IDC_BUTTON1:
			if (BN_CLICKED == evntId)
			{
				if (pDlg->OnButton1) pDlg->OnButton1(hwnd, pDlg->Btn1Data);
			}
			break;
	}
}

static void ProgressDlg_OnErase(HWND hwnd, HDC hdc)
{
	PROGRESSDLG *pDlg;

	if (NULL != (pDlg = GET_DATA(hwnd)))
	{
		RECT rc;
		SetRect(&rc, ICON_OFFSET_X, ICON_OFFSET_Y, 
						ICON_OFFSET_X + (pDlg->icon.rc.right - pDlg->icon.rc.left),
						ICON_OFFSET_Y + (pDlg->icon.rc.bottom - pDlg->icon.rc.top));
		if (RectVisible(hdc, &rc))
		{
			if (AnimateProgressIcon(hdc, ICON_OFFSET_X, ICON_OFFSET_Y, &pDlg->icon)) 
					ExcludeClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
		}
	}
}

static void ProgressDlg_OnLButtonDown(HWND hwnd, DWORD wKey, POINTS pts)
{
	PROGRESSDLG *pDlg = GET_DATA(hwnd);
	if (pDlg)
	{
		POINT pt;
		RECT rc;
		POINTSTOPOINT(pt, pts);
		SetRect(&rc, ICON_OFFSET_X, ICON_OFFSET_Y, 
					ICON_OFFSET_X + (pDlg->icon.rc.right - pDlg->icon.rc.left), 
					ICON_OFFSET_Y + (pDlg->icon.rc.bottom - pDlg->icon.rc.top));

		if (PtInRect(&rc, pt)) SendMessageW(line.hMainWindow, WM_WA_IPC, (WPARAM)L"http://www.cddb.com/", IPC_OPEN_URL);
	}
}

static INT_PTR ProgressDlg_OnSetCursor(HWND hwnd, HWND hwndCursor, WORD htCode, WORD msgId)
{
	PROGRESSDLG *pDlg = GET_DATA(hwnd);
	if (pDlg)
	{
		RECT rc;
		POINT pt;
		GetCursorPos(&pt);
		MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
		SetRect(&rc, ICON_OFFSET_X, ICON_OFFSET_Y, 
					ICON_OFFSET_X + (pDlg->icon.rc.right - pDlg->icon.rc.left), 
					ICON_OFFSET_Y + (pDlg->icon.rc.bottom - pDlg->icon.rc.top));
		if (PtInRect(&rc, pt)) return (NULL != SetCursor(LoadCursor(NULL, IDC_HAND)));
	}
	return FALSE;
}

static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG: return ProgressDlg_OnDialogInit(hwndDlg, (HWND)wParam, lParam);
		case WM_DESTROY:		ProgressDlg_OnDestroy(hwndDlg); break;
		case WM_COMMAND:		ProgressDlg_OnCommand(hwndDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case WM_ERASEBKGND:	ProgressDlg_OnErase(hwndDlg, (HDC)wParam); break;
		case WM_LBUTTONDOWN:	ProgressDlg_OnLButtonDown(hwndDlg, (DWORD)wParam, MAKEPOINTS(lParam)); break;
		case WM_SETCURSOR:	return ProgressDlg_OnSetCursor(hwndDlg, (HWND)wParam, LOWORD(lParam), HIWORD(lParam)); 
	}
	return 0;
}

static BOOL CALLBACK EnumWnd_OnNextWindow(HWND hwnd, LPARAM lParam)
{
	ENUMWND_DATAPACK *pData = (ENUMWND_DATAPACK*)lParam;
	if (!pData) return FALSE;

	if (!pData->found)
	{
		if (pData->host == hwnd) pData->found = TRUE;
	//	return TRUE; 
	}
	ULONG_PTR style  = GetWindowLongPtrW(hwnd, GWL_STYLE);
	if (0 == (WS_CHILD  & style) && 
		(0 == (FINDWND_ONLY_VISIBLE & pData->flags) || (WS_VISIBLE & style)) &&
		(0 == (FINDWND_ONLY_ENABLED & pData->flags) || 0 == (WS_DISABLED & style)))
	{
		HWND hwndOwner = GetWindow(hwnd, GW_OWNER);
		if (pData->host == hwndOwner)
		{
			if (pData->index == pData->count) return FALSE; ///
			pData->list[pData->index] = hwnd;
			pData->index++;
		}
	}
	return TRUE;
}

BOOL FindAllOwnedWindows(HWND hwndHost, HWND *hwndList, INT cList, UINT flags)
{
	BOOL br;
	ENUMWND_DATAPACK data;

	ZeroMemory(&data, sizeof(ENUMWND_DATAPACK));
	if (!hwndHost || !hwndList) return FALSE;

	data.host = hwndHost;
	data.list = hwndList;
	data.count = cList;
	data.flags = flags;
	data.list[0] = NULL;
	br = EnumWindows(EnumWnd_OnNextWindow, (LPARAM)&data);
	data.list[data.index] = NULL;
	return br;
}