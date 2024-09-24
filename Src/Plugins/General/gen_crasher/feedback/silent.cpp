#include ".\main.h"
#include <commctrl.h>
#include <shlobj.h>
#include "resource.h"

BOOL CALLBACK SilentDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ICON_XP));
			SetClassLongPtr(hwndDlg, GCLP_HICON, (LONG_PTR)hIcon);

			HWND hwndPrg = GetDlgItem(hwndDlg, IDC_PRG_COLLECT);
			SendMessage(hwndPrg, PBM_SETRANGE, 0, MAKELPARAM(0,100));
			SendMessage(hwndPrg, PBM_SETPOS, 0, 0);
			SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Starting reporter...");
			ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON1), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON2), SW_HIDE);
			UpdateWindow(hwndDlg);

			if ((settings.createLOG && !settings.ReadLogCollectResult())  &&
				(settings.createDMP && !settings.ReadDmpCollectResult()) ) 
			{
				SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Error. Data was not generated.");
				SendMessage(hwndPrg, PBM_SETPOS, 100, 0);
				UpdateWindow(hwndDlg);
				SetTimer(hwndDlg, 126, 2000, NULL);
				break;
			}
			SetTimer(hwndDlg, 123, 500, NULL);
			break;
		}
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_BUTTON1:
				{
					BOOL ret = FALSE;
					wchar_t file[MAX_PATH] = {0};
					lstrcpyn(file, settings.zipPath, MAX_PATH);

					LPSHELLFOLDER pDesktopFolder = 0;
					if(SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
					{
						LPITEMIDLIST filepidl = 0;
						HRESULT hr = pDesktopFolder->ParseDisplayName(NULL,0,file,0,&filepidl,0);
						if(FAILED(hr)){ pDesktopFolder->Release(); ret = FALSE; }
						else
						{
							if(SUCCEEDED(SHOpenFolderAndSelectItems(filepidl,0,NULL,NULL))){
								ret = TRUE;
							}
						}
					}

					if (ret == FALSE)
					{
						SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Error. Unable to locate crash report.");
						UpdateWindow(hwndDlg);
					}
				}
				break;
				case IDCANCEL:
				case IDC_BUTTON2:
					SetTimer(hwndDlg, 126, 1, NULL);
				break;
			}
			break;
		case WM_TIMER:
			if (wParam == 123)
			{
				KillTimer(hwndDlg, wParam);
				HWND hwndPrg;
				hwndPrg = GetDlgItem(hwndDlg, IDC_PRG_COLLECT);
				SendMessage(hwndPrg, PBM_SETPOS, 20, 0);
				if (settings.zipData)
				{
					SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Packing results...");		
					if(!ZipData())
					{
						SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Error. Unable to pack results.");
						SendMessage(hwndPrg, PBM_SETPOS, 100, 0);
						UpdateWindow(hwndDlg);
						SetTimer(hwndDlg, 126, 2000, NULL);
						break;
					}
				}
				SendMessage(hwndPrg, PBM_SETPOS, 40, 0);
				UpdateWindow(hwndDlg);
				if (settings.sendData)
				{
					SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Sending results...");
					UpdateWindow(hwndDlg);
					if(!SendData(hwndDlg))
					{
						SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Error. Unable to send crash report.");
						SendMessage(hwndPrg, PBM_SETPOS, 100, 0);
						ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON1), SW_SHOW);
						ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON2), SW_SHOW);
						ShowWindow(GetDlgItem(hwndDlg, IDC_PRG_COLLECT), SW_HIDE);
						UpdateWindow(hwndDlg);
						break;
					}
				}
				if (settings.autoRestart)
				{
					SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Restarting Winamp...");		
					SendMessage(hwndPrg, PBM_SETPOS, 80, 0);
					UpdateWindow(hwndDlg);
					if(!Restart())
					{
						SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Error. Unable to restart Winamp.");
						SendMessage(hwndPrg, PBM_SETPOS, 100, 0);
						UpdateWindow(hwndDlg);
						SetTimer(hwndDlg, 126, 2000, NULL);
						break;
					}
				}
				SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Done.");		
				SendMessage(hwndPrg, PBM_SETPOS, 100, 0);
				UpdateWindow(hwndDlg);
				SetTimer(hwndDlg, 126, 1000, NULL);
			}
			else if (wParam == 126)
			{
				KillTimer(hwndDlg, wParam);
				EndDialog(hwndDlg, TRUE);
			}
			break;
	}
	return FALSE;
}