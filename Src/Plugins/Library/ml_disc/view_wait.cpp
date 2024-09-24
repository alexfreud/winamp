#include "main.h"
#include "./resource.h"
#include "../nu/DialogSkinner.h"

#include <windowsx.h>

static HBRUSH hbBack = NULL;
#define TIMER_SHOWTEXT_ID		1985
#define TIMER_SHOWTEXT_DELAY	1000

static INT_PTR WINAPI DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// public function
HWND CreateWaitWindow(HWND hwndParent, CHAR cLetter)
{
	return WASABI_API_CREATEDIALOGPARAMW(IDD_VIEW_WAIT, hwndParent, DlgProc, (LPARAM)cLetter);
}

static void CALLBACK Window_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	switch(idEvent)
	{
		case TIMER_SHOWTEXT_ID:
			KillTimer(hwnd, TIMER_SHOWTEXT_ID);
			SetDlgItemTextW(hwnd, IDC_LBL_TEXT, WASABI_API_LNGSTRINGW(IDS_READINGDISC));
			break;
	}
}

static INT_PTR Window_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SendMessageW(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_BTN_SHOWINFO, BN_EX_GETTEXT), (LPARAM)GetDlgItem(hwnd, IDC_BTN_SHOWINFO));
	SetTimer(hwnd, TIMER_SHOWTEXT_ID, TIMER_SHOWTEXT_DELAY, Window_TimerProc);
	return 0;
}

static void Window_OnDestroy(HWND hwnd)
{
	if (hbBack) 
	{
		DeleteObject(hbBack);
		hbBack = NULL;
	}
}

static void Window_OnDisplayChange(HWND hwnd, INT dpi, INT resX, INT resY)
{
	if (hbBack) 
	{
		DeleteObject(hbBack);
		hbBack = NULL;
	}
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

static void Window_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	HWND hwndCtrl;
	RECT rw;

	hwndCtrl = GetDlgItem(hwnd, IDC_BTN_SHOWINFO);
	if(hwndCtrl)
	{
		GetWindowRect(hwndCtrl, &rw);
		OffsetRect(&rw, -rw.left, -rw.top);
		SetWindowPos(hwndCtrl, NULL, pwp->cx - rw.right, pwp->cy - rw.bottom, 0, 0, 
							SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
	}

	hwndCtrl = GetDlgItem(hwnd, IDC_LBL_TEXT);
	if(hwndCtrl)
	{
		GetWindowRect(hwndCtrl, &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
		SetWindowPos(hwndCtrl, NULL, 0, 0, pwp->cx - rw.left - 2, pwp->cy - 22 - rw.top, 
							SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW  | SWP_NOCOPYBITS);
	}
	if (0 == (SWP_NOREDRAW & pwp->flags)) 
	{
		InvalidateRect(hwnd, NULL, TRUE);
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}

static void Window_OnCommand(HWND hwnd, INT eventId, INT ctrlId, HWND hwndCtrl)
{
	switch(ctrlId)
	{
		case IDC_BTN_SHOWINFO:
			switch(eventId)
			{
				case BN_CLICKED: 
					SendMessageW(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(ctrlId, eventId),(LPARAM)hwndCtrl);
					break;
			}
			break;
	}
}

static HBRUSH Window_OnStaticColor(HWND hwnd, HDC hdc)
{
	if (!hbBack) hbBack = CreateSolidBrush(dialogSkinner.Color(WADLG_ITEMBG));
	SetTextColor(hdc, dialogSkinner.Color(WADLG_ITEMFG));
	SetBkColor(hdc, dialogSkinner.Color(WADLG_ITEMBG));
	return hbBack;
}

static void Window_OnPaint(HWND hwnd)
{
	int tab[] = { IDC_LBL_TEXT | DCW_SUNKENBORDER};
	dialogSkinner.Draw(hwnd, tab, 1);
}

static void Window_OnQueryInfo(HWND hwnd)
{
	HWND hwndParent;
	hwndParent = GetParent(hwnd);
	if (hwndParent) 	SendMessageW(hwndParent, WM_SHOWFILEINFO, (WPARAM)WISF_NORMAL, (LPARAM)L"");
}

static INT_PTR WINAPI DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR result;
	
	if (WM_CTLCOLORSTATIC == uMsg) return (INT_PTR)Window_OnStaticColor(hwnd, (HDC)wParam);
	result = dialogSkinner.Handle(hwnd, uMsg, wParam, lParam); 
	if (result) return result;

	switch(uMsg)
	{
		case WM_INITDIALOG:			return (INT_PTR)Window_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:			Window_OnDestroy(hwnd); break;
		case WM_DISPLAYCHANGE:		Window_OnDisplayChange(hwnd, (INT)wParam, LOWORD(lParam), HIWORD(lParam)); break;
		case WM_WINDOWPOSCHANGED:	Window_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); break;
		case WM_COMMAND:			Window_OnCommand(hwnd, HIWORD(wParam), LOWORD(wParam), (HWND)lParam); break;
		case WM_PAINT:				Window_OnPaint(hwnd); break;
		case WM_QUERYFILEINFO:		Window_OnQueryInfo(hwnd); break;
	}
	return 0;
}