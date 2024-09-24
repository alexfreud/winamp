#include "main.h"
#include "api__ml_wire.h"
#include "../winamp/wa_ipc.h"
#include "Defaults.h"
#include "UpdateTime.h"
#include "UpdateAutoDownload.h"
#include "./cloud.h"
#include <shlobj.h>

extern Cloud cloud;

void Preferences_Init(HWND hwndDlg)
{
	WCHAR szBuffer[256] = {0};
	for (int i = 0;i < Update::TIME_NUMENTRIES;i++)
	{
		const wchar_t *str = Update::GetTitle(i, szBuffer, ARRAYSIZE(szBuffer));
		SendMessage(GetDlgItem(hwndDlg, IDC_UPDATELIST), CB_ADDSTRING, 0, (LPARAM) str);
	}
	int selection = Update::GetSelection(updateTime, autoUpdate);
	SendMessage(GetDlgItem(hwndDlg, IDC_UPDATELIST), CB_SETCURSEL, selection, 0);

	WCHAR adBuffer[256] = {0};
	for (int i = 0;i < UpdateAutoDownload::AUTODOWNLOAD_NUMENTRIES;i++)
	{
		const wchar_t *str = UpdateAutoDownload::GetTitle(i, adBuffer, ARRAYSIZE(adBuffer));
		SendMessage(GetDlgItem(hwndDlg, IDC_AUTODOWNLOADLIST), CB_ADDSTRING, 0, (LPARAM) str);
	}
	selection = UpdateAutoDownload::GetSelection(autoDownloadEpisodes, autoDownload);
	SendMessage(GetDlgItem(hwndDlg, IDC_AUTODOWNLOADLIST), CB_SETCURSEL, selection, 0);

	SetDlgItemText(hwndDlg, IDC_DOWNLOADLOCATION, defaultDownloadPath);
	SetDlgItemText(hwndDlg, IDC_DIRECTORYURL, serviceUrl);
	CheckDlgButton(hwndDlg, IDC_UPDATEONLAUNCH, updateOnLaunch?BST_CHECKED:BST_UNCHECKED);
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

void Preferences_UpdateOnLaunch( HWND hwndDlg )
{
	updateOnLaunch = ( IsDlgButtonChecked( hwndDlg, IDC_UPDATEONLAUNCH ) == BST_CHECKED );
}

void Preferences_UpdateList(HWND hwndDlg)
{
	LRESULT timeSelection;
	timeSelection = SendMessage(GetDlgItem(hwndDlg, IDC_UPDATELIST), CB_GETCURSEL, 0, 0);
	if (timeSelection != CB_ERR)
	{
		autoUpdate = Update::GetAutoUpdate(timeSelection);
		updateTime = Update::GetTime(timeSelection);
		if ( autoUpdate )
			cloud.Pulse();	// update the waitable timer
	}
}

void Preferences_AutoDownloadList(HWND hwndDlg)
{
	LRESULT episodeSelection;
	episodeSelection = SendMessage(GetDlgItem(hwndDlg, IDC_AUTODOWNLOADLIST), CB_GETCURSEL, 0, 0);
	if (episodeSelection != CB_ERR)
	{
		autoDownload         = UpdateAutoDownload::GetAutoDownload(episodeSelection);
		autoDownloadEpisodes = UpdateAutoDownload::GetAutoDownloadEpisodes(episodeSelection);
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
		case IDC_UPDATELIST:
			Preferences_UpdateList(hwndDlg);
			break;
		case IDC_AUTODOWNLOADLIST:
			Preferences_AutoDownloadList(hwndDlg);
			break;
		case IDC_UPDATEONLAUNCH:
			Preferences_UpdateOnLaunch(hwndDlg);
			break;
		case IDC_DOWNLOADLOCATION:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				GetDlgItemText(hwndDlg, IDC_DOWNLOADLOCATION, defaultDownloadPath, MAX_PATH);
				if (!PathFileExists(defaultDownloadPath))
				{
					BuildDefaultDownloadPath(plugin.hwndWinampParent);
					SetDlgItemText(hwndDlg, IDC_DOWNLOADLOCATION, defaultDownloadPath);
				}
			}
			break;
		case IDC_DIRECTORYURL:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				GetDlgItemText(hwndDlg, IDC_DIRECTORYURL, serviceUrl, ARRAYSIZE(serviceUrl));
			}
			break;
		}
		break;
	case WM_DESTROY:
		// ensure we save the changed settings
		SaveAll(true);
		break;
	}

	return 0;
}
