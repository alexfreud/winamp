#define APSTUDIO_READONLY_SYMBOLS
#include "main.h"
#include ".\postsetup.h"
#include "./setup_resource.h"
#include "./langutil.h"
#include "api.h"


static BOOL SleepMsg(DWORD dwTimeout)
{
	DWORD dwStart = GetTickCount();
	DWORD dwElapsed;
	while ((dwElapsed = GetTickCount() - dwStart) < dwTimeout) 
	{
		DWORD dwStatus = MsgWaitForMultipleObjectsEx(0, NULL, dwTimeout - dwElapsed, QS_ALLINPUT, MWMO_WAITALL | MWMO_INPUTAVAILABLE);
		if (dwStatus == WAIT_OBJECT_0)	while (application->app_messageLoopStep());
	}
	return TRUE; // timed out
}

BOOL StartWinamp(BOOL bWaitShow, HWND *phwndWA, LPCSTR pszParam)
{
	HWND hwndWA;
	DWORD pid;
	wchar_t buf[MAX_PATH] = L"\"";
	STARTUPINFOW si = {sizeof(si), };
	PROCESS_INFORMATION pi;
	
	if (phwndWA) *phwndWA = NULL;

	GetModuleFileNameW(NULL, buf + 1, sizeof(buf)/sizeof(wchar_t) - 1);
	StringCchCatW(buf, MAX_PATH, L"\" /NEW ");
	if (*pszParam && lstrlenA(pszParam))
	{
		int count, len;
		len = sizeof(buf)/sizeof(wchar_t) - lstrlenW(buf)- 1;
		count = MultiByteToWideChar(CP_ACP, 0, pszParam, -1, NULL, 0);
		if (count < len) MultiByteToWideChar(CP_ACP, 0, pszParam, -1, buf + lstrlenW(buf), len);
	}
	si.dwFlags = STARTF_FORCEONFEEDBACK | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNOACTIVATE;
	if ( 0 ==CreateProcessW(NULL, buf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		return FALSE;
	}

	hwndWA = NULL;
	if (bWaitShow)
	{
		for (int a = 0; a < 20; a++)
		{
			if (!hwndWA)
			{
				while (NULL != (hwndWA = FindWindowExW(NULL, hwndWA, szAppName, NULL)))
				{
					GetWindowThreadProcessId(hwndWA, &pid);
					if (pid == pi.dwProcessId) break;
				}
			}
			SleepMsg(250);
			if (hwndWA && IsWindowVisible(hwndWA)) 
				break;
		}
	}

	if (phwndWA) *phwndWA = hwndWA;
	return TRUE;
}