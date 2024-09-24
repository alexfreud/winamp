// ui.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include "resource.h"

HINSTANCE g_hInstance;
HWND m_curwnd;

LPTSTR windows[] = {
  MAKEINTRESOURCE(IDD_LICENSE),
  MAKEINTRESOURCE(IDD_SELCOM),
  MAKEINTRESOURCE(IDD_DIR),
  MAKEINTRESOURCE(IDD_INSTFILES),
  MAKEINTRESOURCE(IDD_UNINST)
};

BOOL CALLBACK GenericProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam) {
  static LOGBRUSH b = {BS_SOLID, RGB(255,0,0), 0};
  static HBRUSH red;

  if (!red)
    red = CreateBrushIndirect(&b);

  switch (uMsg) {
    case WM_CTLCOLORSTATIC:
      return (int)red;
  }
  return 0;
}

BOOL CALLBACK DialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam) {
  static int i = -1;
	switch (uMsg) {
	case WM_INITDIALOG:
		ShowWindow(GetDlgItem(hwndDlg, IDC_CHILDRECT), SW_SHOW);
		ShowWindow(hwndDlg, SW_SHOW);
		break;
	case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
    case IDC_BACK:
      i+=(LOWORD(wParam)==IDOK)?1:-1;
      if (i < 0) {
        i++;
        break;
      }
      if (i >= (int)sizeof(windows)/sizeof(char*)) {
        i--;
        break;
      }
      if (m_curwnd) DestroyWindow(m_curwnd);
      m_curwnd=CreateDialog(g_hInstance,windows[i],hwndDlg,GenericProc);
      if (m_curwnd)
      {
        RECT r;
        GetWindowRect(GetDlgItem(hwndDlg,IDC_CHILDRECT),&r);
        ScreenToClient(hwndDlg,(LPPOINT)&r);
        SetWindowPos(m_curwnd,0,r.left,r.top,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
        ShowWindow(m_curwnd,SW_SHOWNA);
      }
      break;
    default:
      EndDialog(hwndDlg, 0);
      PostQuitMessage(0);
      break;
    }
    break;
	}
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

  g_hInstance = GetModuleHandle(0);

	DialogBox(
		g_hInstance,
		MAKEINTRESOURCE(IDD_INST),
		0,
		DialogProc
	);

	ExitProcess(0);

	return 0;
}
