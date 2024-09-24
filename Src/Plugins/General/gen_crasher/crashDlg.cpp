#include ".\crashdlg.h"
#include ".\configdlg.h"
#include ".\resource.h"
#include ".\settings.h"
#include "exceptionhandler.h"
#include <strsafe.h>

extern Settings settings;
extern PEXCEPTION_POINTERS gExceptionInfo;

BOOL CALLBACK CrashDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (uMsg) 
	{
		case WM_INITDIALOG: 
		{
			// as we're loading things, make sure we've got a decent icon size to use in the second usage
			HICON hIcon = (HICON)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(102),IMAGE_ICON,48,48,LR_SHARED);
			SetClassLongPtr(hwndDlg, GCLP_HICON, (LONG_PTR)hIcon);

			HWND hwndPrg = GetDlgItem(hwndDlg, IDC_PRG_COLLECT);
			SendMessage(hwndPrg, PBM_SETRANGE, 0, MAKELPARAM(0,100));
			SendMessage(hwndPrg, PBM_SETPOS, 0, 0);

			// this will make sure that we've got the logo shown even when using a localised version
			SendDlgItemMessage(hwndDlg,IDC_BMP_LOGO,STM_SETIMAGE,IMAGE_ICON,(LPARAM)hIcon);

			SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Analyzing settings...");
			settings.ClearTempData();

			wchar_t waPath[2*_MAX_PATH] = {0};
			if (GetModuleFileName( NULL, waPath, 2*_MAX_PATH ))
			{
				settings.WriteWinamp(waPath);
			}

			SetTimer(hwndDlg, 123, 1000, NULL);
			break;
		}
		case WM_TIMER:
			if (wParam == 123)
			{
				KillTimer(hwndDlg,wParam);
				HWND hwndPrg = GetDlgItem(hwndDlg, IDC_PRG_COLLECT);
				SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Generating log file...");
				SendMessage(hwndPrg, PBM_SETPOS, 30, 0);
				UpdateWindow(hwndDlg);
				if (settings.createLOG) settings.WriteLogCollectResult(CreateLog(gExceptionInfo, L"Winamp"));
				SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Generating dump file...");
				SendMessage(hwndPrg, PBM_SETPOS, 50, 0);
				UpdateWindow(hwndDlg);
				if (settings.createDMP) settings.WriteDmpCollectResult(CreateDump(gExceptionInfo));
				SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Starting error reporter...");
				SendMessage(hwndPrg, PBM_SETPOS, 90, 0);
				UpdateWindow(hwndDlg);
				STARTUPINFO si = {0};
				si.cb = sizeof(si);
				si.dwFlags = STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_SHOW;

				PROCESS_INFORMATION pi = {0};
				wchar_t reporter[512] = {0}, waPlugPath[MAX_PATH] = {0}, cmd[512] = {0}, *waPath = 0;
				GetModuleFileName( NULL, waPlugPath, MAX_PATH);
				CreatePathFromFullName(&waPath, waPlugPath);
				StringCchPrintf(reporter, 512, L"%s\\reporter.exe", waPath);
				StringCchPrintf(cmd, 512, L"  \"%s\"", settings.GetPath());

				if (CreateProcess(
									reporter,		// name of executable module
									cmd,			// command line string
									NULL,			// process attributes
									NULL,			// thread attributes
									FALSE,			// handle inheritance option
									0,				// creation flags
									NULL,			// new environment block
									NULL,			// current directory name
									&si,			// startup information
									&pi))			// process information
				{
					SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Done.");
					SetTimer(hwndDlg, 126, 200, NULL);
				}
				else
				{
					SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Error. Unable to run reporter.");
					SetTimer(hwndDlg, 126, 3000, NULL);
				}

				SendMessage(hwndPrg, PBM_SETPOS, 100, 0);
				UpdateWindow(hwndDlg);
			}
			else if (wParam == 126)
			{
				KillTimer(hwndDlg,wParam);
				DestroyWindow(hwndDlg);	
			}
		break;
	} 
	return FALSE;
}