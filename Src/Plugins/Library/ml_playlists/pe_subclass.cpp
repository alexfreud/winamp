#include "main.h"

static HMENU last_playlistscmdmenu = NULL;
static WNDPROC PE_oldWndProc;
static WORD waCmdMenuID;

static BOOL CALLBACK PE_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_COMMAND && wParam > 45000 && wParam < 55000)
	{
		if (LoadPlaylist(wParam - 45000))
			return 0;
	}
	else if (uMsg == WM_INITMENUPOPUP)
	{
		HMENU hmenuPopup = (HMENU) wParam;
		if (hmenuPopup == wa_playlists_cmdmenu)
		{
			if (!waCmdMenuID)
			{
				waCmdMenuID = (WORD)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_REGISTER_LOWORD_COMMAND);
			}
			if (last_playlistscmdmenu)
			{
				RemoveMenu(wa_playlists_cmdmenu, waCmdMenuID, MF_BYCOMMAND);
				DestroyMenu(last_playlistscmdmenu);
				last_playlistscmdmenu = NULL;
			}
			mlGetTreeStruct mgts = { 3001, 45000, -1};
			last_playlistscmdmenu = (HMENU)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM) &mgts, ML_IPC_GETTREE);
			if (last_playlistscmdmenu)
			{
				MENUITEMINFOW menuItem = {sizeof(MENUITEMINFOW), MIIM_SUBMENU | MIIM_ID | MIIM_TYPE, MFT_STRING,
										  MFS_ENABLED, waCmdMenuID, last_playlistscmdmenu, NULL, NULL, NULL,
										  WASABI_API_LNGSTRINGW(IDS_OPEN_PLAYLIST_FROM_ML), 0};
				// if there's no playlists then let the user know this
				if(!AGAVE_API_PLAYLISTS->GetCount())
				{
					wchar_t buf[64] = {0};
					DestroyMenu(last_playlistscmdmenu);
					menuItem.hSubMenu = last_playlistscmdmenu = CreateMenu();
					InsertMenuW(menuItem.hSubMenu, 0, MF_BYPOSITION | MF_STRING | MF_GRAYED, 0, WASABI_API_LNGSTRINGW_BUF(IDS_NO_PLAYLIST_IN_LIBRARY,buf,64));
				}
				InsertMenuItemW(wa_playlists_cmdmenu, 1, TRUE, &menuItem);
			}
		}
	}
	return CallWindowProc(PE_oldWndProc, hwndDlg, uMsg, wParam, lParam);
}

static HWND hwnd_pe = NULL;
void HookPlaylistEditor()
{
	hwnd_pe =(HWND)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,IPC_GETWND_PE,IPC_GETWND); 

	if (hwnd_pe)
		PE_oldWndProc=(WNDPROC) SetWindowLongPtr(hwnd_pe,GWLP_WNDPROC,(LONG_PTR)PE_newWndProc); 
}

void UnhookPlaylistEditor()
{
	SetWindowLongPtr(hwnd_pe,GWLP_WNDPROC,(LONG_PTR)PE_oldWndProc);
}