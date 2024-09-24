#include <shlwapi.h>

#include "main.h"
#include "../winamp/wa_ipc.h"
#include "CurrentPlaylist.h"
#include "SendTo.h"
#include "Playlist.h"
#include "api__ml_playlists.h"

using namespace Nullsoft::Utility;
WNDPROC ml_wndProc = 0;

static INT_PTR CALLBACK AddPlaylistDialogProc_sc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static wchar_t *title;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		title = (wchar_t*)lParam;
		PostMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_NAME), TRUE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				wchar_t name[256] = {0};
				GetDlgItemText(hwndDlg, IDC_NAME, name, 255);
				name[255] = 0;
				if (!name[0])
				{
					wchar_t titleStr[32] = {0};
					MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_ENTER_A_NAME),
							   WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,titleStr,32), MB_OK);
					break;
				}
				lstrcpyn(title,name,256);
				EndDialog(hwndDlg,1);
			}
			break;
		case IDCANCEL:
			EndDialog(hwndDlg,0);
			break;
		}
		break;
	}
	return 0;
}

static void AddPlaylist(mlAddPlaylist *addPlaylist)
{
	PlaylistInfo pl;
	wchar_t filename[1024+256] = {0}; // use a longer buffer than MAX_PATH here because createPlayListDBFileName needs it
	if (addPlaylist->flags & PL_FLAGS_IMPORT)
	{
		createPlayListDBFileName(filename);
		CopyFileW(addPlaylist->filename, filename, FALSE);
	}
	else
		lstrcpynW(filename, addPlaylist->filename, MAX_PATH);

	int numItems = 0;
	int length = 0;

	wchar_t title[256] = {0};

	if(addPlaylist->playlistName)
		lstrcpynW(title, addPlaylist->playlistName, 256);
	else // prompt for name
	{
		if(WASABI_API_DIALOGBOXPARAMW((playlists_CloudAvailable() ? IDD_ADD_CLOUD_PLAYLIST : IDD_ADD_PLAYLIST),
									  plugin.hwndLibraryParent, AddPlaylistDialogProc_sc, (LPARAM)title) == 0)
		{ // the user hit cancel
			return;
		}
	}

	if (addPlaylist->numItems == -1 || addPlaylist->length == -1)
	{
		Playlist temp;
		AGAVE_API_PLAYLISTMANAGER->Load(filename, &temp);
		numItems = temp.GetNumItems();
		for (size_t i = 0;i != numItems;i++)
		{
			int len = temp.GetItemLengthMilliseconds(i) / 1000;
			if (len>=0)
				length += len;
		}
	}
	else
	{
		length   = addPlaylist->length;
		numItems = addPlaylist->numItems;
	}

	AddPlaylist(true, title, filename, !!(addPlaylist->flags & PL_FLAG_SHOW), g_config->ReadInt(L"cloud", 1), numItems, length);
}

static void MakePlaylist(mlMakePlaylistV2 *makePlaylist)
{
	AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
	switch (makePlaylist->type)
	{
		case ML_TYPE_FILENAMES:
		case ML_TYPE_STREAMNAMES:
			AddPlaylistFromFilenames((const char *)makePlaylist->data, makePlaylist->playlistName, makePlaylist->flags);
			if (makePlaylist->flags & PL_FLAG_FILL_FILENAME && makePlaylist->size == sizeof(mlMakePlaylistV2))
			{
				// TODO: not guaranteed to be at the end
				size_t last_index = AGAVE_API_PLAYLISTS->GetCount() - 1;
				lstrcpynW(makePlaylist->filename, AGAVE_API_PLAYLISTS->GetFilename(last_index), MAX_PATH);
			}
			return ;

		case ML_TYPE_FILENAMESW:
		case ML_TYPE_STREAMNAMESW:
			AddPlaylistFromFilenamesW((const wchar_t *)makePlaylist->data, makePlaylist->playlistName, makePlaylist->flags);
			if (makePlaylist->flags & PL_FLAG_FILL_FILENAME && makePlaylist->size == sizeof(mlMakePlaylistV2))
			{
				// TODO: not guaranteed to be at the end
				size_t last_index = AGAVE_API_PLAYLISTS->GetCount() - 1;
				lstrcpynW(makePlaylist->filename, AGAVE_API_PLAYLISTS->GetFilename(last_index), MAX_PATH);
			}
			return ;

		case ML_TYPE_ITEMRECORDLIST:
		case ML_TYPE_CDTRACKS:
			AddPlaylistFromItemRecordList((itemRecordList *)makePlaylist->data, makePlaylist->playlistName, makePlaylist->flags);
			if (makePlaylist->flags & PL_FLAG_FILL_FILENAME && makePlaylist->size == sizeof(mlMakePlaylistV2))
			{
				// TODO: not guaranteed to be at the end
				size_t last_index = AGAVE_API_PLAYLISTS->GetCount() - 1;
				lstrcpynW(makePlaylist->filename, AGAVE_API_PLAYLISTS->GetFilename(last_index), MAX_PATH);
			}
			return ;

		case ML_TYPE_ITEMRECORDLISTW:
			AddPlaylistFromItemRecordListW((itemRecordListW *)makePlaylist->data, makePlaylist->playlistName, makePlaylist->flags);
			if (makePlaylist->flags & PL_FLAG_FILL_FILENAME && makePlaylist->size == sizeof(mlMakePlaylistV2))
			{
				// TODO: not guaranteed to be at the end
				size_t last_index = AGAVE_API_PLAYLISTS->GetCount() - 1;
				lstrcpynW(makePlaylist->filename, AGAVE_API_PLAYLISTS->GetFilename(last_index), MAX_PATH);
			}
			return ;
	}
}

static int GetPlaylistInfo(mlPlaylistInfo *info)
{
	AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);

	size_t num = info->playlistNum;
	if (num >= AGAVE_API_PLAYLISTS->GetCount())
		return 0;

	PlaylistInfo playlist(num);
	lstrcpynW(info->filename, playlist.GetFilename(), MAX_PATH);
	lstrcpynW(info->playlistName, playlist.GetName(), 128);
	info->length = playlist.GetLength();
	info->numItems = playlist.GetSize();

	return 1;
}

static INT_PTR PlaylistIPC(int msg, INT_PTR param)
{
	switch (msg)
	{
		case ML_IPC_NEWPLAYLIST: playlists_Add((HWND)param);	return 1;
		case ML_IPC_IMPORTPLAYLIST: Playlist_importFromFile((HWND)param);	return 1;
		case ML_IPC_SAVEPLAYLIST: CurrentPlaylist_Export((HWND)param);	return 1; // TODO: can we guarantee a currently active playlist?
		case ML_IPC_IMPORTCURRENTPLAYLIST: Playlist_importFromWinamp();	return 1;
		// play/load the playlist passed as param
		case ML_IPC_PLAY_PLAYLIST: PlayPlaylist(param);	return 1;
		case ML_IPC_LOAD_PLAYLIST: LoadPlaylist(param);	return 1;
		case ML_IPC_GETPLAYLISTWND:		return(INT_PTR)activeHWND;
		case ML_IPC_PLAYLIST_ADD: AddPlaylist((mlAddPlaylist *)param); return 1;
		case ML_IPC_PLAYLIST_MAKE: MakePlaylist((mlMakePlaylistV2 *)param); return 1;
		case ML_IPC_PLAYLIST_COUNT: return AGAVE_API_PLAYLISTS->GetCount();
		case ML_IPC_PLAYLIST_INFO: return GetPlaylistInfo((mlPlaylistInfo *)param);
	}
	return 0;
}

extern SendToMenu treeViewSendTo;
INT_PTR CALLBACK MediaLibraryProcedure(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITMENUPOPUP:
			if (treeViewSendTo.InitPopupMenu(wParam))
				return 0;

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case WINAMP_MANAGEPLAYLISTS:
					mediaLibrary.SelectTreeItem(playlistsTreeId);
					return 1;
				case ID_DOSHITMENU_ADDNEWPLAYLIST:
					playlists_Add(hwndDlg);
					return 1;
			}
		}
			break;

		case WM_ML_IPC:
		{
			INT_PTR res = PlaylistIPC(lParam, wParam);
			if (res)
			{
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, res);
				return TRUE;
			}
		}
	}
	return CallWindowProc(ml_wndProc, hwndDlg, uMsg, wParam, lParam);
}

void HookMediaLibrary()
{
	ml_wndProc = (WNDPROC)SetWindowLongPtr(plugin.hwndLibraryParent, DWLP_DLGPROC, (LONG_PTR)MediaLibraryProcedure);
}

void UnhookMediaLibrary()
{
	SetWindowLongPtr(plugin.hwndLibraryParent, DWLP_DLGPROC, (LONG_PTR)ml_wndProc);
}

#define TREE_PLAYLIST_ID_START 3002

INT_PTR LoadPlaylist(INT_PTR treeId)
{
	if (!FindTreeItem(treeId))
		return 0;

	wchar_t wstr[MAX_PATH+1] = {0};
	{ // scope for lock
		AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);

		PlaylistInfo info;
		info.Associate(treeId);

		playlist_SaveGUID(info.playlist_guid);

		memset(wstr, 0, sizeof(wstr));  // easy (but slow) double null terminate
		PathCombineW(wstr, g_path, info.GetFilename());
	}

	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_DELETE);
	enqueueFileWithMetaStructW s;
	s.filename = wstr;
	s.title    = 0;
	s.ext      = NULL;
	s.length   = -1;
	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
	return 1;
}

INT_PTR PlayPlaylist(INT_PTR treeId)
{
	if (LoadPlaylist(treeId))
	{
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_STARTPLAY);
		return 1;
	}
	else
		return 0;
}