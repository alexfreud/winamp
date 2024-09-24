#include "main.h"
#include <shlwapi.h>
#include "resource.h"

wchar_t *createPlayListDBFileName(wchar_t *filename) // filename is ignored but used for temp space, make sure it's 1024+256 chars =)
{
	wchar_t *filenameptr;
	int x = 32;
	for (;;)
	{
		GetTempFileNameW(g_path, L"plf", GetTickCount() + x*5000, filename);
		if ( wcslen(filename) > 4)
		{
			if (g_config->ReadInt(L"playlist_m3u8", 1))
				lstrcpyW(filename + wcslen(filename) - 4, L".m3u8");
			else
				lstrcpyW(filename + wcslen(filename) - 4, L".m3u");
		}
		HANDLE h = CreateFileW(filename, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_NEW, 0, 0);
		if (h != INVALID_HANDLE_VALUE)
		{
			filenameptr = filename + wcslen(g_path) + 1;
			CloseHandle(h);
			break;
		}
		if (++x > 4096)
		{
			filenameptr = L"error.m3u";
			break;
		}
	}
	return filenameptr;
}

void playlists_AddToCloudPrompt(HWND hwndDlg)
{
	if (g_config->ReadInt(L"cloud_always", 0) && !g_config->ReadInt(L"cloud_prompt", 0) && g_config->ReadInt(L"cloud", 1))
	{
		wchar_t titleStr[64] = {0};
		if (MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_CLOUD_UNCHECKED),
					   WASABI_API_LNGSTRINGW_BUF(IDS_SENDTO_NEW_CLOUD_PLAYLIST, titleStr, 64),
					   MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
		{
			g_config->WriteInt(L"cloud", (IsDlgButtonChecked(hwndDlg, IDC_CLOUD) == BST_CHECKED));
		}
		else
		{
			CheckDlgButton(hwndDlg, IDC_CLOUD, playlists_CloudAvailable() && g_config->ReadInt(L"cloud", 1));
		}
		g_config->WriteInt(L"cloud_prompt", 1);
	}
	else
	{
		g_config->WriteInt(L"cloud", (IsDlgButtonChecked(hwndDlg, IDC_CLOUD) == BST_CHECKED));
	}
}

int AddToCloud()
{
	if (playlists_CloudAvailable())
	{
		if (g_config->ReadInt(L"cloud_always", 1))
		{
			g_config->WriteInt(L"cloud", 1);
			return 1;
		}
		else
		{
			g_config->WriteInt(L"cloud", 0);
		}
	}
	return 0;
}

INT_PTR CALLBACK AddPlaylistDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SetProp(hwndDlg, L"pladdcb", (HANDLE)lParam);
			CheckDlgButton(hwndDlg, IDC_CLOUD, AddToCloud());
			PostMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_NAME), TRUE);
			break;
		}

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					wchar_t name[256] = {0};
					GetDlgItemText(hwndDlg, IDC_NAME, name, 255);
					if (!name[0])
					{
						wchar_t titleStr[32] = {0};
						MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_ENTER_A_NAME),
								   WASABI_API_LNGSTRINGW_BUF(IDS_ERROR, titleStr, 32), MB_OK);
						break;
					}

					wchar_t filename[1024 + 256] = {0};
					createPlayListDBFileName(filename);
					bool callback = !!GetProp(hwndDlg, L"pladdcb");
					AddPlaylist(callback, name, filename, true, (playlists_CloudAvailable() ? g_config->ReadInt(L"cloud", 1) : 0)); 
					if (callback) AGAVE_API_PLAYLISTS->Flush(); // REVIEW: save immediately? or only at the end?
					EndDialog(hwndDlg, 1);
				}
				break;

				case IDCANCEL:
					EndDialog(hwndDlg, 0);
				break;

				case IDC_CLOUD:
				{
					playlists_AddToCloudPrompt(hwndDlg);
				}
				break;
			}
			break;
	}
	return FALSE;
};