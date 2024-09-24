#include "main.h"
#include "api__ml_local.h"
#include "..\..\General\gen_ml/config.h"
#include "resource.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"

static HRGN g_rgnUpdate = NULL;
static int offsetX = 0, offsetY = 0;

static void LayoutWindows(HWND hwnd, BOOL fRedraw)
{
	RECT rc, rg;
	HRGN rgn;

	GetClientRect(hwnd, &rc);
	SetRect(&rg, 0, 0, 0, 0);

	rc.top += 2;
	rc.right -=2;

	if (rc.bottom <= rc.top || rc.right <= rc.left) return;

	rgn = NULL;

	HWND temp = GetDlgItem(hwnd, IDC_DB_ERROR);
	GetWindowRect(temp, &rg);
	SetWindowPos(temp, NULL, WASABI_API_APP->getScaleX(20), WASABI_API_APP->getScaleY(20),
				 rc.right - rc.left - WASABI_API_APP->getScaleX(40),
				 rc.bottom - rc.top - WASABI_API_APP->getScaleY(45),
				 SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);

	temp = GetDlgItem(hwnd, IDC_RESET_DB_ON_ERROR);
	GetWindowRect(temp, &rg);
	SetWindowPos(temp, NULL, ((rc.right - rc.left) - (rg.right - rg.left)) / 2,
				 rc.bottom - (rg.bottom - rg.top),
				 rg.right - rg.left, rg.bottom - rg.top,
				 SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);

	InvalidateRect(hwnd, NULL, TRUE);

	if (fRedraw) 
	{
		UpdateWindow(hwnd);
	}
	if (g_rgnUpdate)
	{
		GetUpdateRgn(hwnd, g_rgnUpdate, FALSE);
		if (rgn)
		{
			OffsetRgn(rgn, rc.left, rc.top);
			CombineRgn(g_rgnUpdate, g_rgnUpdate, rgn, RGN_OR);
		}
	}
	ValidateRgn(hwnd, NULL);
	if (rgn) DeleteObject(rgn);	
}

INT_PTR CALLBACK view_errorinfoDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	BOOL a=dialogSkinner.Handle(hwndDlg,uMsg,wParam,lParam); if (a) return a;
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			SetWindowText(GetDlgItem(hwndDlg, IDC_DB_ERROR),(LPCWSTR)WASABI_API_LOADRESFROMFILEW(L"TEXT", MAKEINTRESOURCE((nde_error ? IDR_NDE_ERROR : IDR_DB_ERROR)), 0));
			if (nde_error)
				DestroyWindow(GetDlgItem(hwndDlg, IDC_RESET_DB_ON_ERROR));

			MLSKINWINDOW m = {0};
			m.skinType = SKINNEDWND_TYPE_DIALOG;
			m.hwndToSkin = hwndDlg;
			m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
			MLSkinWindow(plugin.hwndLibraryParent, &m);
		}
		return TRUE;

		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_RESET_DB_ON_ERROR)
			{
				nukeLibrary(hwndDlg);
			}
		break;

		case WM_WINDOWPOSCHANGED:
			if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & ((WINDOWPOS*)lParam)->flags) || 
				(SWP_FRAMECHANGED & ((WINDOWPOS*)lParam)->flags))
			{
				LayoutWindows(hwndDlg, !(SWP_NOREDRAW & ((WINDOWPOS*)lParam)->flags));
			}
		return 0;

		case WM_USER+66:
			if (wParam == -1) 
			{
				LayoutWindows(hwndDlg, TRUE);
			}
		return TRUE;

		case WM_USER + 0x200:
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 1); // yes, we support no - redraw resize
		return TRUE;

		case WM_USER + 0x201:
			offsetX = (short)LOWORD(wParam);
			offsetY = (short)HIWORD(wParam);
			g_rgnUpdate = (HRGN)lParam;
		return TRUE;

		case WM_PAINT:
		{
			dialogSkinner.Draw(hwndDlg, 0, 0);
		}
		return 0;

		case WM_ERASEBKGND:
		return 1; //handled by WADlg_DrawChildWindowBorders in WM_PAINT
	}
	return FALSE;
}
