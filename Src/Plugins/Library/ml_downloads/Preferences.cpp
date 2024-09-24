#include "main.h"
#include "api__ml_downloads.h"
#include "../winamp/wa_ipc.h"
#include "Defaults.h"
#include <shlobj.h>

void Preferences_Init(HWND hwndDlg)
{
	SetDlgItemText(hwndDlg, IDC_DOWNLOADLOCATION, defaultDownloadPath);
}

BOOL CALLBACK browseEnumProc(HWND hwnd, LPARAM lParam)
{
	wchar_t cl[32] = {0};
	GetClassNameW(hwnd, cl, ARRAYSIZE(cl));
	if (!lstrcmpiW(cl, WC_TREEVIEW))
	{
		PostMessage(hwnd, TVM_ENSUREVISIBLE, 0, (LPARAM)TreeView_GetSelection(hwnd));
		return FALSE;
	}

	return TRUE;
}

int CALLBACK WINAPI BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED)
	{
		SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)defaultDownloadPath);

		// this is not nice but it fixes the selection not working correctly on all OSes
		EnumChildWindows(hwnd, browseEnumProc, 0);
	}
	return 0;
}

void Preferences_Browse(HWND hwndDlg)
{
	wchar_t folder[MAX_PATH] = {0};
	BROWSEINFO browse = {0};
	lstrcpyn(folder, defaultDownloadPath, MAX_PATH);
	browse.hwndOwner = hwndDlg;
	browse.lpszTitle = WASABI_API_LNGSTRINGW(IDS_CHOOSE_FOLDER);
	browse.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	browse.lpfn = BrowseCallbackProc;
	LPITEMIDLIST itemList = SHBrowseForFolder(&browse);
	if (itemList)
	{
		SHGetPathFromIDList(itemList, folder);
		lstrcpyn(defaultDownloadPath, folder, MAX_PATH);
		SetWindowText(GetDlgItem(hwndDlg, IDC_DOWNLOADLOCATION), folder);
		LPMALLOC malloc;
		SHGetMalloc(&malloc);
		malloc->Free(itemList);
	}
}


BOOL CALLBACK PreferencesDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		Preferences_Init(hwndDlg);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BROWSE:
			Preferences_Browse(hwndDlg);
			break;
		}
		break;
	}

	return 0;
}
