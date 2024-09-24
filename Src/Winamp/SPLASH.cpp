/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "Main.h"

#define SPLASH_TIMER 34
//#define MODAL_SPLASHSCREEN

static INT_PTR CALLBACK splashFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static int wait;
void splashDlg(int wait_in_ms)
{
	wait = wait_in_ms;
#ifdef MODAL_SPLASHSCREEN
	DialogBox(hMainInstance, MAKEINTRESOURCE(IDD_SPLASH), NULL, splashFunc);
#else
	CreateDialogW(hMainInstance, MAKEINTRESOURCE(IDD_SPLASH), NULL, splashFunc);
#endif
}

static INT_PTR CALLBACK splashFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		GetWindowRect(GetDlgItem(hwndDlg, IDC_SPLASHIMG), &r);
		SetWindowPos(hwndDlg, 0, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER);
		SetTimer(hwndDlg, SPLASH_TIMER, wait, NULL);
		return 0;
	case WM_KEYDOWN:
case WM_LBUTTONDOWN: case WM_RBUTTONDOWN:
#ifdef MODAL_SPLASHSCREEN
		EndDialog(hwndDlg, 0);
#else
		DestroyWindow(hwndDlg);
#endif
		return 0;
	case WM_TIMER:
		if (wParam == SPLASH_TIMER)
		{
#ifdef MODAL_SPLASHSCREEN
			EndDialog(hwndDlg, 1);
#else
			DestroyWindow(hwndDlg);
#endif
		}
		return 0;;
	}
	return 0;
}
