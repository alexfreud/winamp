#include "main.h"
#include "resource.h"
#include"api__ml_rg.h"
#include <strsafe.h>

struct Progress
{
	Progress()
	{
		processedFiles = 0;
		currentBytes = 0;
		totalBytes = 0;
		activeHWND = 0;
		threadHandle = 0;
		openDialogs = 0;
		done = false;
		killSwitch = 0;
	}

	size_t processedFiles;
	uint64_t currentBytes;
	uint32_t totalBytes;
	WorkQueue activeQueue;
	HWND activeHWND;
	HANDLE threadHandle;
	size_t openDialogs;
	bool done;
	int killSwitch;
};

DWORD WINAPI ThreadProc(void *param)
{
	Progress *progress = (Progress *)param;
	ProgressCallback callback(progress->activeHWND);
	progress->activeQueue.Calculate(&callback, &progress->killSwitch);
	PostMessage(progress->activeHWND, WM_USER + 2, 0, 0);

	return 0;
}

void getViewport(RECT *r, HWND wnd, int full, RECT *sr)
{
	POINT *p = NULL;

	if (p || sr || wnd)
	{
		HMONITOR hm = NULL;

		if (sr)
			hm = MonitorFromRect(sr, MONITOR_DEFAULTTONEAREST);
		else if (wnd)
			hm = MonitorFromWindow(wnd, MONITOR_DEFAULTTONEAREST);
		else if (p)
			hm = MonitorFromPoint(*p, MONITOR_DEFAULTTONEAREST);

		if (hm)
		{
			MONITORINFOEXW mi;
			memset(&mi, 0, sizeof(mi));
			mi.cbSize = sizeof(mi);

			if (GetMonitorInfoW(hm, &mi))
			{
				if (!full)
					*r = mi.rcWork;
				else
					*r = mi.rcMonitor;

				return ;
			}
		}
	}
	if (full)
	{ // this might be borked =)
		r->top = r->left = 0;
		r->right = GetSystemMetrics(SM_CXSCREEN);
		r->bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		SystemParametersInfoW(SPI_GETWORKAREA, 0, r, 0);
	}
}

BOOL windowOffScreen(HWND hwnd, POINT pt)
{
	RECT r = {0}, wnd = {0}, sr = {0};
	GetWindowRect(hwnd, &wnd);
	sr.left = pt.x;
	sr.top = pt.y;
	sr.right = sr.left + (wnd.right - wnd.left);
	sr.bottom = sr.top + (wnd.bottom - wnd.top);
	getViewport(&r, hwnd, 0, &sr);
	return !PtInRect(&r, pt);
}

void SaveWindowPos(HWND hwnd)
{
	RECT rect = {0};
	GetWindowRect(hwnd, &rect);
	char buf[16] = {0};
	StringCchPrintfA(buf, 16, "%d", rect.left);
	WritePrivateProfileStringA("ml_rg", "prog_x", buf, iniFile);
	StringCchPrintfA(buf, 16, "%d", rect.top);
	WritePrivateProfileStringA("ml_rg", "prog_y", buf, iniFile);
}

INT_PTR WINAPI ReplayGainProgressProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			Progress *progress = (Progress *)lParam;
			progress->killSwitch = 0;
			progress->done = false;
			progress->openDialogs = 0;
			progress->processedFiles = 0;
			progress->activeHWND = hwndDlg;

			wchar_t dummy[64] = {0};
			StringCchPrintfW(dummy, 64, WASABI_API_LNGSTRINGW(IDS_1_OF_X_FILES), progress->activeQueue.totalFiles);
			SetDlgItemTextW(hwndDlg, IDC_PROGRESS_FILES, dummy);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)progress);
			DWORD threadId;
			progress->threadHandle = CreateThread(NULL, 0, ThreadProc, (void *)progress, CREATE_SUSPENDED, &threadId);
			SetThreadPriority(progress->threadHandle, THREAD_PRIORITY_IDLE);
			ResumeThread(progress->threadHandle);

			POINT pt = {(LONG)GetPrivateProfileIntA("ml_rg", "prog_x", -1, iniFile),
						(LONG)GetPrivateProfileIntA("ml_rg", "prog_y", -1, iniFile)};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
			else
				ShowWindow(hwndDlg, SW_SHOW);
		}
		break;
	case WM_DESTROY:
		{
			SaveWindowPos(hwndDlg);
			Progress *progress = (Progress *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			CloseHandle(progress->threadHandle);
			progress->activeHWND = 0;
			delete progress;
		}
		break;
	case WM_USER:  // file finished
		{
			Progress *progress = (Progress *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			progress->processedFiles++;

			if (progress->processedFiles + 1 > progress->activeQueue.totalFiles)
				SetDlgItemTextW(hwndDlg, IDC_PROGRESS_FILES, WASABI_API_LNGSTRINGW(IDS_FINISHED));
			else
			{
				wchar_t dummy[64] = {0};
				StringCchPrintfW(dummy, 64,
								 WASABI_API_LNGSTRINGW(IDS_X_OF_X_FILES),
								 progress->processedFiles + 1, progress->activeQueue.totalFiles);
				SetDlgItemTextW(hwndDlg, IDC_PROGRESS_FILES, dummy);
			}
		}
		break;
	case WM_USER + 1:  // album done
		{
			Progress *progress = (Progress *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			WorkQueue::RGWorkQueue *queue = (WorkQueue::RGWorkQueue *)lParam;
			SaveWindowPos(hwndDlg);
			if (config_ask && config_ask_each_album)
			{
				progress->openDialogs++;
				DoResults(*queue);
				progress->openDialogs--;
				if (!progress->openDialogs && progress->done)
					DestroyWindow(hwndDlg);
			}
			else if (config_ask == 0)
			{
				WriteAlbum(*queue);
			}
		}
		break;
	case WM_USER + 2:  // all tracks done
		{
			Progress *progress = (Progress *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			ShowWindow(hwndDlg, SW_HIDE);
			SaveWindowPos(hwndDlg);
			if (config_ask && config_ask_each_album == 0)
			{
				DoResults(progress->activeQueue);
			}
			progress->killSwitch = 1;
			WaitForSingleObject(progress->threadHandle, INFINITE);
			progress->done = true;
			if (!progress->openDialogs)
				DestroyWindow(hwndDlg);
		}
		break;
	case WM_USER + 3:  // total bytes of current file
		{
			Progress *progress = (Progress *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			progress->currentBytes = 0;
			progress->totalBytes = (uint32_t)lParam;
			if (progress->totalBytes == 0)
			{
				SetDlgItemTextW(hwndDlg, IDC_FILE_PROGRESS, WASABI_API_LNGSTRINGW(IDS_PROCESSING));
			}
			else
			{
				wchar_t dummy[64] = {0};
				StringCchPrintfW(dummy, 64, L"%u%%", (progress->currentBytes * 100) / progress->totalBytes);
				SetDlgItemTextW(hwndDlg, IDC_FILE_PROGRESS, dummy);
			}
		}
		break;
	case WM_USER + 4:  // more bytes read
		{
			Progress *progress = (Progress *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			progress->currentBytes += lParam;
			if (progress->totalBytes == 0)
			{
				SetDlgItemTextW(hwndDlg, IDC_FILE_PROGRESS, WASABI_API_LNGSTRINGW(IDS_PROCESSING));
			}
			else 
			{
				wchar_t dummy[64] = {0};
				StringCchPrintfW(dummy, 64, L"%u%%", (progress->currentBytes * 100) / progress->totalBytes);
				SetDlgItemTextW(hwndDlg, IDC_FILE_PROGRESS, dummy);
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDCANCEL:
				{
					Progress *progress = (Progress *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					progress->killSwitch = 1;
					break;
				}
		}
		break;
	}
	return 0;
}

void DoProgress(WorkQueue &workQueue)
{
	Progress *progress = new Progress;
	progress->activeQueue = workQueue; // this is a huge slow copy, but I don't care at the moment
	WASABI_API_CREATEDIALOGPARAMW(IDD_PROGRESS, GetDialogBoxParent(), ReplayGainProgressProc, (LPARAM)progress);
}

HWND GetDialogBoxParent()
{
	HWND parent = (HWND)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETDIALOGBOXPARENT);
	if (!parent || parent == (HWND)1)
		return plugin.hwndWinampParent;
	return parent;
}