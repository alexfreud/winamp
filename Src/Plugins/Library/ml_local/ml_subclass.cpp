#include "main.h"
extern void AccessingGracenoteHack(int);
extern HWND subWnd;

// TODO: benski> a lot of things don't need to be part of gen_ml window - they could easily be done with a hidden window
LRESULT APIENTRY ml_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_USER+641:
			{
				AccessingGracenoteHack(wParam);
				break;
			}
		case WM_ML_IPC:
			{
				INT_PTR ret = HandleIpcMessage((INT_PTR)lParam, (INT_PTR)wParam);
				if (ret != 0)
				{
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, ret);
					return ret; // supposed to return TRUE but thus is not working for me :(
				}
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDM_DOSHITMENU_ADDNEWVIEW:
				addNewQuery(hwndDlg);
				return 0;
			case IDM_ADD_PLEDIT:
				add_pledit_to_library();
				return 0;
			case IDM_ADD_DIRS:
				add_to_library(hwndDlg);
				return 0;
			case IDM_REMOVE_UNUSED_FILES:
				Scan_RemoveFiles(hwndDlg);
				if (m_curview_hwnd) SendMessage(m_curview_hwnd, WM_APP + 1, 0, 0); //update current view
				return 0;
			case IDM_RESCANFOLDERSNOW:
				if (!g_bgscan_scanning) SendMessage(hwndDlg, WM_USER + 575, 0xffff00dd, 0);
				return 0;
			}
			break;
		case WM_USER + 575:        //sent by prefs to start scanning
			if (wParam == 0xffff00dd && !lParam)
			{
				if (!g_bgscan_scanning)
				{
					Scan_BackgroundScan();
				}
			}
			break;
		case WM_TIMER:
			{
				static int in_timer;
				if (in_timer) return 0;
				in_timer = 1;
				if (wParam == 200) // decide if it is time to scan yet
				{
					if (!g_bgscan_scanning)
					{
						if (g_bgrescan_force || (g_bgrescan_do && (time(NULL) - g_bgscan_last_rescan) > g_bgrescan_int*60))
						{
							// send to the prefs page so it'll show the status if it's open
							// (makes it easier to see if things are working with the rescan every x option)
							if (IsWindow(subWnd)) SendMessage(subWnd, WM_USER+101, 0, 0);
							Scan_BackgroundScan();
						}
					}
					in_timer = 0;
					return 0;
				}
				in_timer = 0;
			}
			break;
	}
	return CallWindowProc(ml_oldWndProc, hwndDlg, uMsg, wParam, lParam);
}