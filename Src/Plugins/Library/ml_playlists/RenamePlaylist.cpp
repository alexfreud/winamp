#include "main.h"
#include "resource.h"
using namespace Nullsoft::Utility;

static INT_PTR CALLBACK RenamePlaylistDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static GUID playlist_guid = INVALID_GUID;
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			playlist_guid = *(GUID *)lParam;

			AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
			PlaylistInfo playlist(playlist_guid);
			if (playlist.Valid())
			{
				size_t index = playlist.GetIndex();
				const wchar_t *title = AGAVE_API_PLAYLISTS->GetName(index);
				SetDlgItemText(hwndDlg, IDC_NAME, title);

				SendMessage(GetDlgItem(hwndDlg, IDC_NAME), EM_SETSEL, 0, -1);
				SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_NAME), TRUE);
			
			}
			else
				EndDialog(hwndDlg, 1);
		}
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					wchar_t name[1024] = {0};
					GetDlgItemText(hwndDlg, IDC_NAME, name, 1023);
					name[1023] = 0;
					if (!name[0])
					{
						wchar_t titleStr[32] = {0};
						MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_ENTER_A_NAME),
								   WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,titleStr,32), MB_OK);
						break;
					}

					AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);

					PlaylistInfo playlist(playlist_guid);

					if (playlist.Valid())
					{
						size_t index = playlist.GetIndex();
						AGAVE_API_PLAYLISTS->RenamePlaylist(index, name);
						AGAVE_API_PLAYLISTS->Flush(); // TODO: save immediately? or only at the end?
					}

					EndDialog(hwndDlg, 1);
				}
				break;
				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					break;
			}
			break;
	}
	return FALSE;
};

void RenamePlaylist(GUID _guid, HWND parent)
{
	WASABI_API_DIALOGBOXPARAMW(IDD_RENAME_PLAYLIST, parent, RenamePlaylistDialogProc, (LPARAM)&_guid);
}