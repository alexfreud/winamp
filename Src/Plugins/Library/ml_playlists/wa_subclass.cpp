#include "main.h"
#include "../Winamp/wa_ipc.h"
#include "replicant/nu/AutoLock.h"
#include <algorithm>

using namespace Nullsoft::Utility;

static WNDPROC waProc = 0;
extern HMENU wa_play_menu;
static HMENU last_playlistsmenu = NULL;
WORD waMenuID = 0;
extern int IPC_LIBRARY_PLAYLISTS_REFRESH, IPC_CLOUD_ENABLED;

LRESULT WINAPI WinampProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_COMMAND || msg == WM_SYSCOMMAND)
	{
		if (LOWORD(wParam) == WINAMP_MANAGEPLAYLISTS)
		{
			mediaLibrary.ShowMediaLibrary();
			mediaLibrary.SelectTreeItem(playlistsTreeId);
			return 1;
		}
		else if (msg == WM_COMMAND && wParam > 45000 && wParam < 55000)
		{
			INT_PTR treeId = wParam - 45000;
			if (FindTreeItem(treeId))
			{
				mediaLibrary.SwitchToPluginView(treeId);
			}
		}
		else if (msg == WM_COMMAND && wParam > 55000 && wParam < 65000)
		{
			if (PlayPlaylist(wParam - 55000))
				return 0;
		}
	}
	else if (msg == WM_INITMENUPOPUP)
	{
		HMENU hmenuPopup = (HMENU) wParam;
		if (hmenuPopup == wa_play_menu)
		{
			if (last_playlistsmenu)
			{
				RemoveMenu(wa_play_menu, waMenuID, MF_BYCOMMAND);
				DestroyMenu(last_playlistsmenu);
				last_playlistsmenu = NULL;
			}
			mlGetTreeStruct mgts = { 3001, 55000, -1};
			last_playlistsmenu = (HMENU)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM) &mgts, ML_IPC_GETTREE);
			if (last_playlistsmenu)
			{
				MENUITEMINFOW menuItem = {sizeof(MENUITEMINFOW), MIIM_SUBMENU | MIIM_ID | MIIM_TYPE, MFT_STRING,
										  MFS_ENABLED, waMenuID, last_playlistsmenu, NULL, NULL, NULL,
										  WASABI_API_LNGSTRINGW(IDS_PLAYLIST_FROM_ML), 0};
				// if there's no playlists then let the user know this
				if(!AGAVE_API_PLAYLISTS->GetCount())
				{
					wchar_t buf[64] = {0};
					DestroyMenu(last_playlistsmenu);
					menuItem.hSubMenu = last_playlistsmenu = CreateMenu();
					InsertMenuW(menuItem.hSubMenu, 0, MF_BYPOSITION | MF_STRING | MF_GRAYED, 0, WASABI_API_LNGSTRINGW_BUF(IDS_NO_PLAYLIST_IN_LIBRARY,buf,64));
				}
				InsertMenuItemW(wa_play_menu, GetMenuItemCount(wa_play_menu), TRUE, &menuItem);
			}
		}
	}
	else if (msg == WM_WA_IPC && lParam == IPC_LIBRARY_PLAYLISTS_REFRESH)
	{
		// refresh the status of the tree items e.g. when made
		// being made into a cloud playlist or remove from it
		UpdatePlaylists();
	}
	else if (msg == WM_WA_IPC && lParam == IPC_CLOUD_ENABLED)
	{
		cloud_avail = 1;
		if (IsWindow(currentView)) PostMessage(currentView, WM_APP + 102, 0, 0);
	}

	if (waProc)
		return CallWindowProcW(waProc, hwnd, msg, wParam, lParam);
	else
		return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Hook(HWND winamp)
{
	if (IsWindow(winamp))
		waProc = (WNDPROC)SetWindowLongPtrW(winamp, GWLP_WNDPROC, (LONG_PTR)WinampProcedure);
}