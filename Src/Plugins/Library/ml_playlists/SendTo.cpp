#include <strsafe.h>

#include "main.h"
#include "api__ml_playlists.h"
#include "Playlist.h"
#include "PlaylistView.h"
#include "resource.h"
#include "../nu/AutoWideFn.h"
#include <vector>

/* TODO:
  Somehow replace index values in send-to with GUID's.  Possibly by making a vector of GUIDs (like view_playlists).
	Since index values could (in theory) change in a background thread while send-to interaction is going on.
*/

extern Playlist          currentPlaylist;

using namespace Nullsoft::Utility;

static std::vector<GUID> sendto_playlistGUIDs;

int playlists_CloudAvailable()
{
	if (IPC_GET_CLOUD_HINST == -1) IPC_GET_CLOUD_HINST = (INT)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloud", IPC_REGISTER_WINAMP_IPCMESSAGE);
	if (IPC_GET_CLOUD_ACTIVE == -1) IPC_GET_CLOUD_ACTIVE = (INT)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloudActive", IPC_REGISTER_WINAMP_IPCMESSAGE);
	if (!cloud_hinst) cloud_hinst = (HINSTANCE)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_HINST);

	return (cloud_avail = /*0/*/!(!cloud_hinst || cloud_hinst == (HINSTANCE)1 || !SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_ACTIVE))/**/);
}

int playlists_CloudInstalled()
{
	if (IPC_GET_CLOUD_HINST == -1) IPC_GET_CLOUD_HINST = (INT)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloud", IPC_REGISTER_WINAMP_IPCMESSAGE);
	if (!cloud_hinst) cloud_hinst = (HINSTANCE)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_HINST);

	return (!(!cloud_hinst || cloud_hinst == (HINSTANCE)1));
}

int playlists_BuildSendTo(int sourceType, INT_PTR context)
{
	if (sourceType == ML_TYPE_ITEMRECORDLISTW || sourceType == ML_TYPE_ITEMRECORDLIST ||
		sourceType == ML_TYPE_FILENAMES || sourceType == ML_TYPE_STREAMNAMES ||
		sourceType == ML_TYPE_FILENAMESW || sourceType == ML_TYPE_STREAMNAMESW ||
		sourceType == ML_TYPE_CDTRACKS)
	{
		if (g_config->ReadInt(L"pl_send_to", DEFAULT_PL_SEND_TO))
		{
			mediaLibrary.BranchSendTo(context);
			mediaLibrary.AddToSendTo(WASABI_API_LNGSTRINGW(IDS_SENDTO_NEW_PLAYLIST), context, playlistsTreeId);

			AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
			sendto_playlistGUIDs.clear();
			size_t count = AGAVE_API_PLAYLISTS->GetCount();
			for (size_t i = 0;i != count;i++)
			{
				PlaylistInfo info(i);
				if (info.Valid())
				{
					sendto_playlistGUIDs.push_back(info.playlist_guid);
					if (sendToIgnoreID != info.treeId)
					{
						mediaLibrary.AddToBranchSendTo(info.GetName(), context, reinterpret_cast<INT_PTR>(pluginMessageProc) + i);
					}
				}
			}
			mediaLibrary.EndBranchSendTo(WASABI_API_LNGSTRINGW(IDS_SENDTO_ML_PLAYLISTS), context);
		}
		else
		{
			mediaLibrary.AddToSendTo(WASABI_API_LNGSTRINGW(IDS_SENDTO_PLAYLIST), context, playlistsTreeId);
		}
	}

	return 0;
}

static void NewPlaylist( Playlist &newPlaylist, const wchar_t *playlistTitle, int makeTree, const wchar_t *newFilename = 0 )
{
	if ( playlistTitle && *playlistTitle )
	{
		size_t numItems = newPlaylist.GetNumItems();
		wchar_t filename[ 1024 + 256 ] = { 0 };

		if ( newFilename )
		{
			if ( PathIsFileSpecW( newFilename ) )
				PathCombineW( filename, g_path, newFilename );
			else
				lstrcpynW( filename, newFilename, MAX_PATH );
		}
		else
			PathCombineW( filename, g_path, createPlayListDBFileName( filename ) );

		AGAVE_API_PLAYLISTMANAGER->Save( filename, &newPlaylist );
		AddPlaylist( true, playlistTitle, filename, ( LOWORD( makeTree ) != 0 ), HIWORD( makeTree ), numItems, newPlaylist.lengthInMS );
	}
	else
	{
		AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );
		size_t a = AGAVE_API_PLAYLISTS->GetCount();
		playlists_Add( plugin.hwndLibraryParent, false );

		if ( AGAVE_API_PLAYLISTS->GetCount() == a + 1 )
		{
			PlaylistInfo info( a );
			if ( info.Valid() )
			{
				info.SetLength( (int)( newPlaylist.lengthInMS > 0 ? newPlaylist.lengthInMS / 1000 : 0 ) );
				info.SetSize( newPlaylist.GetNumItems() );
				info.SetCloud( HIWORD( makeTree ) );
				wchar_t fn[ MAX_PATH ] = { 0 };
				if ( PathIsFileSpecW( info.GetFilename() ) )
					PathCombineW( fn, g_path, info.GetFilename() );
				else
					lstrcpynW( fn, info.GetFilename(), MAX_PATH );


				for ( pl_entry *l_entry : newPlaylist.entries )
				{
					for ( pl_entry *l_current_entry : currentPlaylist.entries )
					{
						if ( wcscmp( l_entry->filename, l_current_entry->filename ) == 0 )
						{
							//SetTitle releases previously allocated string, no need to free previous one
							l_entry->SetTitle(l_current_entry->filetitle);


							if ( !l_current_entry->_extended_infos.empty() )
							{
								for ( auto l_current_extended_info : l_current_entry->_extended_infos )
									l_entry->_extended_infos.emplace( _wcsdup( l_current_extended_info.first.c_str() ), _wcsdup( l_current_extended_info.second.c_str() ) );
							}

							break;
						}
					}
				}				

				AGAVE_API_PLAYLISTMANAGER->Save( fn, &newPlaylist );
				// delay the added notification being sent so for the cloud
				// we can have the complete playlist available to work with
				AGAVE_API_PLAYLISTS->Flush();
				WASABI_API_SYSCB->syscb_issueCallback( api_playlists::SYSCALLBACK, api_playlists::PLAYLIST_ADDED, a/* + 1*/, 0 );
				UpdateTree( info, info.treeId );
			}
		}
	}
	RefreshPlaylistsList();
}

void AddPlaylistFromFilenames(const char *filenames, const wchar_t *playlistTitle, int makeTree, const wchar_t *filename)
{
	Playlist newPlaylist;
	while (filenames && *filenames)
	{
		AutoWide wideFn(filenames);
		if (AGAVE_API_PLAYLISTMANAGER->Load(wideFn, &newPlaylist) != PLAYLISTMANAGER_SUCCESS) // try to load it as a playlist first
		{
			wchar_t title[FILETITLE_SIZE] = {0};
			int length = 0;
			mediaLibrary.GetFileInfo(wideFn, title, FILETITLE_SIZE, &length);
			newPlaylist.AppendWithInfo(wideFn, title, length*1000); // otherwise just add it to the playlist directly
		}

		filenames += strlen(filenames) + 1;
	}

	NewPlaylist(newPlaylist, playlistTitle, makeTree, filename);
}

void AddPlaylistFromFilenamesW(const wchar_t *filenames, const wchar_t *playlistTitle, int makeTree, const wchar_t *filename)
{
	Playlist newPlaylist;
	while (filenames && *filenames)
	{
		if (AGAVE_API_PLAYLISTMANAGER->Load(filenames, &newPlaylist) != PLAYLISTMANAGER_SUCCESS) // try to load it as a playlist first
		{
			wchar_t title[FILETITLE_SIZE] = {0};
			int length = 0;
			mediaLibrary.GetFileInfo(filenames, title, FILETITLE_SIZE, &length);
			newPlaylist.AppendWithInfo(filenames, title, (length > 0 ? length*1000 : 0)); // otherwise just add it to the playlist directly
		}

		filenames += wcslen(filenames) + 1;
	}

	NewPlaylist(newPlaylist, playlistTitle, makeTree, filename);
}

static wchar_t *itemrecordTagFunc(wchar_t *tag, void * p) //return 0 if not found
{
	itemRecord *t = (itemRecord *)p;
	char buf[128] = {0};
	char *value = NULL;

	if (!_wcsicmp(tag, L"artist"))	value = t->artist;
	else if (!_wcsicmp(tag, L"album"))	value = t->album;
	else if (!_wcsicmp(tag, L"filename")) value = t->filename;
	else if (!_wcsicmp(tag, L"title"))	value = t->title;
	else if (!_wcsicmp(tag, L"ext"))	value = t->ext;
	else if (!_wcsicmp(tag, L"year"))
	{
		if (t->year > 0)
		{
			StringCchPrintfA(buf, 128, "%04d", t->year);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"genre"))	value = t->genre;
	else if (!_wcsicmp(tag, L"comment")) value = t->comment;
	else if (!_wcsicmp(tag, L"tracknumber") || !_wcsicmp(tag, L"track"))
	{
		if (t->track > 0)
		{
			StringCchPrintfA(buf, 128, "%02d", t->track);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"rating"))		value = getRecordExtendedItem(t, "RATING");
	else if (!_wcsicmp(tag, L"playcount")) value = getRecordExtendedItem(t, "PLAYCOUNT");
	else if (!_wcsicmp(tag, L"bitrate")) value = getRecordExtendedItem(t, "BITRATE");
	else
		return 0;

	if (!value) return reinterpret_cast<wchar_t *>(-1);
	else return AutoWideDup(value);
}

static wchar_t *itemrecordWTagFunc(wchar_t *tag, void * p) //return 0 if not found
{
	itemRecordW *t = (itemRecordW *)p;
	wchar_t buf[128] = {0};
	wchar_t *value = NULL;

	// TODO: more fields
	if (!_wcsicmp(tag, L"artist"))	value = t->artist;
	else if (!_wcsicmp(tag, L"album"))	value = t->album;
	else if (!_wcsicmp(tag, L"filename")) value = t->filename;
	else if (!_wcsicmp(tag, L"title"))	value = t->title;
	else if (!_wcsicmp(tag, L"year"))
	{
		if (t->year > 0)
		{
			StringCchPrintfW(buf, 128, L"%04d", t->year);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"genre"))	value = t->genre;
	else if (!_wcsicmp(tag, L"comment")) value = t->comment;
	else if (!_wcsicmp(tag, L"tracknumber") || !_wcsicmp(tag, L"track"))
	{
		if (t->track > 0)
		{
			StringCchPrintfW(buf, 128, L"%02d", t->track);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"rating"))
	{
		if (t->rating > 0)
		{
			StringCchPrintfW(buf, 128, L"%d", t->rating);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"playcount")) value = t->comment;
	else if (!_wcsicmp(tag, L"bitrate"))
	{
		if (t->bitrate > 0)
		{
			StringCchPrintfW(buf, 128, L"%d", t->bitrate);
			value = buf;
		}
	}
	else
		return 0;

	if (!value) return reinterpret_cast<wchar_t *>(-1);
	else return _wcsdup(value);
}

static void fieldTagFuncFree(char * tag, void * p)
{
	free(tag);
}

static void BuildTitle(itemRecord *record, wchar_t *title, int lenCch)
{
	AutoWideFn wfn(record->filename);
	waFormatTitleExtended fmt;
	fmt.filename = wfn;
	fmt.useExtendedInfo = 1;
	fmt.out = title;
	fmt.out_len = lenCch;
	fmt.p = record;
	fmt.spec = 0;
	*(void **)&fmt.TAGFUNC = itemrecordTagFunc;
	*(void **)&fmt.TAGFREEFUNC = fieldTagFuncFree;
	*title = 0;

	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&fmt, IPC_FORMAT_TITLE_EXTENDED);
}

static void BuildTitleW(itemRecordW *record, wchar_t *title, int lenCch)
{
	waFormatTitleExtended fmt;
	fmt.filename = record->filename;
	fmt.useExtendedInfo = 1;
	fmt.out = title;
	fmt.out_len = lenCch;
	fmt.p = record;
	fmt.spec = 0;
	*(void **)&fmt.TAGFUNC = itemrecordWTagFunc;
	*(void **)&fmt.TAGFREEFUNC = fieldTagFuncFree;
	*title = 0;

	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&fmt, IPC_FORMAT_TITLE_EXTENDED);
}

void AddPlaylistFromItemRecordList(itemRecordList *obj, const wchar_t *playlistTitle, int makeTree, const wchar_t *filename)
{
	Playlist newPlaylist;
	wchar_t title[FILETITLE_SIZE] = {0};
	for (int x = 0; x < obj->Size; x ++)
	{
		BuildTitle(&obj->Items[x], title, FILETITLE_SIZE);
		newPlaylist.AppendWithInfo(AutoWide(obj->Items[x].filename), title, obj->Items[x].length*1000);
	}

	NewPlaylist(newPlaylist, playlistTitle, makeTree, filename);
}

void AddPlaylistFromItemRecordListW(itemRecordListW *obj, const wchar_t *playlistTitle, int makeTree, const wchar_t *filename)
{
	Playlist newPlaylist;
	wchar_t title[FILETITLE_SIZE] = {0};
	for (int x = 0; x < obj->Size; x ++)
	{
		BuildTitleW(&obj->Items[x], title, FILETITLE_SIZE);
		newPlaylist.AppendWithInfo(obj->Items[x].filename, title, obj->Items[x].length*1000);
	}

	NewPlaylist(newPlaylist, playlistTitle, makeTree, filename);
}

static void AddToPlaylist(GUID playlist_guid, int sourceType, INT_PTR data)
{
	AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);

	playlist_Save(plugin.hwndWinampParent);
	PlaylistInfo p(playlist_guid);
	if (p.Valid())
	{
		Playlist playlist;
		AGAVE_API_PLAYLISTMANAGER->Load(p.GetFilename(), &playlist);

		if (sourceType == ML_TYPE_FILENAMES || sourceType == ML_TYPE_STREAMNAMES)
		{
			const char *ptr = (const char*)data;

			while (ptr && *ptr)
			{
				AutoWide wideFn(ptr);
				// try to load in playlist manager first
				Playlist sentPlaylist;
				if (sourceType == ML_TYPE_FILENAMES
				    && AGAVE_API_PLAYLISTMANAGER->Load(wideFn, &sentPlaylist) == PLAYLISTMANAGER_SUCCESS)
				{
					playlist.AppendPlaylist(sentPlaylist);
				}
				else
				{
					wchar_t title[FILETITLE_SIZE] = {0};
					int length = -1;
					mediaLibrary.GetFileInfo(wideFn, title, FILETITLE_SIZE, &length);
					playlist.AppendWithInfo(wideFn, title, length*1000);
				}
				ptr += strlen(ptr) + 1;
			}
		}
		else if (sourceType == ML_TYPE_FILENAMESW || sourceType == ML_TYPE_STREAMNAMESW)
		{
			const wchar_t *ptr = (const wchar_t*)data;

			while (ptr && *ptr)
			{
				// try to load in playlist manager first
				Playlist sentPlaylist;
				if (sourceType == ML_TYPE_FILENAMES
				    && AGAVE_API_PLAYLISTMANAGER->Load(ptr, &sentPlaylist) == PLAYLISTMANAGER_SUCCESS)
				{
					playlist.AppendPlaylist(sentPlaylist);
				}
				else
				{
					wchar_t title[FILETITLE_SIZE] = {0};
					int length = -1;
					mediaLibrary.GetFileInfo(ptr, title, FILETITLE_SIZE, &length);

					std::map<std::wstring, std::wstring> l_extended_infos;

					if ( PathIsURLW( title ) )
					{
						wchar_t *end = 0;
						for ( pl_entry *l_current_entry : currentPlaylist.entries )
						{
							if ( wcscmp( title, l_current_entry->filename ) == 0 )
							{
								StringCchCopyExW( title, FILETITLE_SIZE, l_current_entry->filetitle, &end, 0, 0 );

								if ( !l_current_entry->_extended_infos.empty() )
								{
									for ( auto l_current_extended_info : l_current_entry->_extended_infos )
										l_extended_infos.emplace( _wcsdup( l_current_extended_info.first.c_str() ), _wcsdup( l_current_extended_info.second.c_str() ) );
								}

								break;
							}
						}
					}

					if ( l_extended_infos.empty() )
						playlist.AppendWithInfo( ptr, title, length * 1000 );
					else
						playlist.AppendWithInfo( ptr, title, length * 1000, l_extended_infos );
				}
				ptr += wcslen(ptr) + 1;
			}
		}
		else if (sourceType == ML_TYPE_ITEMRECORDLIST || sourceType == ML_TYPE_CDTRACKS)
		{
			itemRecordList *obj = (itemRecordList *)data;

			wchar_t title[FILETITLE_SIZE] = {0};
			for (int x = 0; x < obj->Size; x ++)
			{
				BuildTitle(&obj->Items[x], title, FILETITLE_SIZE);
				playlist.AppendWithInfo(AutoWide(obj->Items[x].filename), title, obj->Items[x].length*1000);
			}
		}
		else if (sourceType == ML_TYPE_ITEMRECORDLISTW)
		{
			itemRecordListW *obj = (itemRecordListW *)data;

			wchar_t title[FILETITLE_SIZE] = {0};
			for (int x = 0; x < obj->Size; x ++)
			{
				BuildTitleW(&obj->Items[x], title, FILETITLE_SIZE);
				playlist.AppendWithInfo(obj->Items[x].filename, title, obj->Items[x].length*1000);
			}
		}
		else if (sourceType == ML_TYPE_PLAYLIST)
		{
			mlPlaylist *ptr = (mlPlaylist *)data;

			Playlist sentPlaylist;
			if (AGAVE_API_PLAYLISTMANAGER->Load(ptr->filename, &sentPlaylist) == PLAYLISTMANAGER_SUCCESS)
			{
				playlist.AppendPlaylist(sentPlaylist);
			}
		}
		else if (sourceType == ML_TYPE_PLAYLISTS)
		{
			mlPlaylist **playlists= (mlPlaylist **)data;

			while(playlists && *playlists)
			{
				mlPlaylist *ptr = *playlists;
				Playlist sentPlaylist;
				if (AGAVE_API_PLAYLISTMANAGER->Load(ptr->filename, &sentPlaylist) == PLAYLISTMANAGER_SUCCESS)
				{
					playlist.AppendPlaylist(sentPlaylist);
				}
				playlists++;
			}
		}

		AGAVE_API_PLAYLISTMANAGER->Save(p.GetFilename(), &playlist);
		p.SetSize(playlist.GetNumItems());
		p.IssueSaveCallback();

		playlist_Reload();
	}
}

int playlists_OnDropTarget(int id, int sourceType, INT_PTR data)
{
	if (id == playlistsTreeId)
	{
		switch (sourceType)
		{
			case ML_TYPE_FILENAMES:
			case ML_TYPE_STREAMNAMES:
				if (data)
					AddPlaylistFromFilenames((const char *)data, 0, 1);
				return 1;
			case ML_TYPE_FILENAMESW:
			case ML_TYPE_STREAMNAMESW:
				if (data)
					AddPlaylistFromFilenamesW((const wchar_t *)data, 0, 1);
				return 1;
			case ML_TYPE_ITEMRECORDLIST:
			case ML_TYPE_CDTRACKS:
				if (data)
					AddPlaylistFromItemRecordList((itemRecordList *)data, 0, 1);
				return 1;
			case ML_TYPE_ITEMRECORDLISTW:
				if (data)
					AddPlaylistFromItemRecordListW((itemRecordListW *)data, 0, 1);
				return 1;
			default:
				return -1;
		}
	}
	else if (FindTreeItem(id))
	{
		switch (sourceType)
		{
			case ML_TYPE_FILENAMES:
			case ML_TYPE_STREAMNAMES:
			case ML_TYPE_FILENAMESW:
			case ML_TYPE_STREAMNAMESW:
			case ML_TYPE_ITEMRECORDLIST:
			case ML_TYPE_ITEMRECORDLISTW:
			case ML_TYPE_CDTRACKS:
				if (data && !we_are_drag_and_dropping)
				{
					AddToPlaylist(tree_to_guid_map[id], sourceType, data);
				}
				return 1;
			default:
				return -1;
		}
	}
	return 0;
}

static INT_PTR CALLBACK SelectPlaylistProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			LRESULT index = SendDlgItemMessage(hwndDlg, IDC_PLAYLISTS, CB_ADDSTRING, 0, (LPARAM) WASABI_API_LNGSTRINGW(IDS_SENDTO_NEW_PLAYLIST));
			SendDlgItemMessage(hwndDlg, IDC_PLAYLISTS, CB_SETITEMDATA , index, -1);
			/*if (playlists_CloudAvailable())
			{
				index = SendDlgItemMessage(hwndDlg, IDC_PLAYLISTS, CB_ADDSTRING, 0, (LPARAM) WASABI_API_LNGSTRINGW(IDS_SENDTO_NEW_CLOUD_PLAYLIST));
				SendDlgItemMessage(hwndDlg, IDC_PLAYLISTS, CB_SETITEMDATA , index, -1);
			}*/
			AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
			sendto_playlistGUIDs.clear();
			size_t count = AGAVE_API_PLAYLISTS->GetCount();
			for (size_t i = 0;i != count; i++)
			{
				PlaylistInfo info(i);
				if (info.Valid())
				{
					sendto_playlistGUIDs.push_back(info.playlist_guid);
					if (sendToIgnoreID != info.treeId)
					{
						index = SendDlgItemMessage(hwndDlg, IDC_PLAYLISTS, CB_ADDSTRING, 0, (LPARAM)info.GetName());
						SendDlgItemMessage(hwndDlg, IDC_PLAYLISTS, CB_SETITEMDATA , index, i);
					}
				}
			}
			SendDlgItemMessage(hwndDlg, IDC_PLAYLISTS, CB_SETCURSEL, 0, 0);
		}
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					int selection = SendDlgItemMessage(hwndDlg, IDC_PLAYLISTS, CB_GETCURSEL, 0, 0);
					if (selection != CB_ERR)
						EndDialog(hwndDlg, SendDlgItemMessage(hwndDlg, IDC_PLAYLISTS, CB_GETITEMDATA , selection, 0));
					else
						EndDialog(hwndDlg, -2);
				}
				break;
				case IDCANCEL:
				{
					EndDialog(hwndDlg, -2);
				}
				break;
			}
			break;

	}
	return 0;
}

int playlists_OnSendTo(int sourceType, INT_PTR data, int id)
{
	if (id == playlistsTreeId || id == playlistsCloudTreeId)
	{
		if (g_config->ReadInt(L"pl_send_to", DEFAULT_PL_SEND_TO))
		{
			int is_cloud = (id == playlistsCloudTreeId) || (id == playlistsTreeId && g_config->ReadInt(L"cloud", 1));
			switch (sourceType)
			{
				case ML_TYPE_FILENAMES:
				case ML_TYPE_STREAMNAMES:
					AddPlaylistFromFilenames((const char *)data, 0, MAKELONG(1, is_cloud));
					return 1;
				case ML_TYPE_FILENAMESW:
				case ML_TYPE_STREAMNAMESW:
					AddPlaylistFromFilenamesW((const wchar_t *)data, 0, MAKELONG(1, is_cloud));
					return 1;
				case ML_TYPE_ITEMRECORDLIST:
				case ML_TYPE_CDTRACKS:
					AddPlaylistFromItemRecordList((itemRecordList *)data, 0, MAKELONG(1, is_cloud));
					return 1;
				case ML_TYPE_ITEMRECORDLISTW:
					AddPlaylistFromItemRecordListW((itemRecordListW *)data, 0, MAKELONG(1, is_cloud));
					return 1;
				case ML_TYPE_PLAYLIST:
				{
					mlPlaylist *ptr = (mlPlaylist *)data;
					AddPlaylistFromFilenamesW(ptr->filename, 0, MAKELONG(1, is_cloud));
					return 1;
				}
				case ML_TYPE_PLAYLISTS:
				{
					mlPlaylist **playlists= (mlPlaylist **)data;
					while(playlists && *playlists)
					{
						mlPlaylist *ptr = *playlists;
						AddPlaylistFromFilenamesW(ptr->filename, 0, MAKELONG(1, is_cloud));
						playlists++;
					}
					return 1;
				}
			}
		}
		else
		{
			size_t selection = WASABI_API_DIALOGBOXW(IDD_SELECT_PLAYLIST, NULL, SelectPlaylistProc);
			if (selection == -2)
				return -1;
			else if (selection == -1)
			{
				switch (sourceType)
				{
					case ML_TYPE_FILENAMES:
					case ML_TYPE_STREAMNAMES:
						AddPlaylistFromFilenames((const char *)data, 0, true);
						return 1;
					case ML_TYPE_FILENAMESW:
					case ML_TYPE_STREAMNAMESW:
						AddPlaylistFromFilenamesW((const wchar_t *)data, 0, true);
						return 1;
					case ML_TYPE_ITEMRECORDLIST:
					case ML_TYPE_CDTRACKS:
						AddPlaylistFromItemRecordList((itemRecordList *)data, 0, 1);
						return 1;
					case ML_TYPE_ITEMRECORDLISTW:
						AddPlaylistFromItemRecordListW((itemRecordListW *)data, 0, 1);
						return 1;
					case ML_TYPE_PLAYLIST:
					{
						mlPlaylist *ptr = (mlPlaylist *)data;
						AddPlaylistFromFilenamesW(ptr->filename, 0, true);
						return 1;
					}
					case ML_TYPE_PLAYLISTS:
					{
						mlPlaylist **playlists= (mlPlaylist **)data;
						while(playlists && *playlists)
						{
							mlPlaylist *ptr = *playlists;
							AddPlaylistFromFilenamesW(ptr->filename, 0, true);
							playlists++;
						}
						return 1;
					}
				}
			}
			else if (selection >= 0 && selection < sendto_playlistGUIDs.size())
			{
				AddToPlaylist(sendto_playlistGUIDs[selection], sourceType, data);
				return 1;
			}
		}
	}
	else
	{
		size_t x = id - reinterpret_cast<size_t>(pluginMessageProc);
		if (x < sendto_playlistGUIDs.size())
		{
			AddToPlaylist(sendto_playlistGUIDs[x], sourceType, data);
			return 1;
		}
	}
	return 0;
}