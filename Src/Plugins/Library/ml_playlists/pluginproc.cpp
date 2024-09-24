#include "main.h"
#include "resource.h"
#include <windows.h>
#include <commctrl.h>
#include "SendTo.h"
#include "ml_local/api_mldb.h"
#include "ml_pmp/pmp.h"
#include "replicant/nswasabi/ReferenceCounted.h"
#include "replicant/nx/win/nxstring.h"
#include "replicant/nu/AutoChar.h"
#include "nu/AutoCharFn.h"
#include "nu/menushortcuts.h"

#include <api/syscb/callbacks/syscb.h>
#include <api/syscb/callbacks/browsercb.h>

using namespace Nullsoft::Utility;
INT_PTR lastActiveID = 0;
SendToMenu treeViewSendTo;
HWND currentView = 0;
int playlists_ContextMenu(INT_PTR param1, HWND hHost, POINTS pts);

LRESULT pluginHandleIpcMessage(int msg, WPARAM param)
{
	return SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, param, msg);
}

INT_PTR pluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	switch (message_type)
	{
		case ML_MSG_NO_CONFIG:
			return TRUE;

		case ML_MSG_TREE_ONCREATEVIEW:
			if (param1 == playlistsTreeId)
			{
				return (INT_PTR)(currentView = WASABI_API_CREATEDIALOGW(IDD_VIEW_PLAYLISTS, (HWND)param2, view_playlistsDialogProc));
			}
			else // if param1 is a valid playlist
			{
				if (FindTreeItem(param1))
				{
					lastActiveID = param1;
					// TODO: what if it's an ifc_playlist provided from elsewhere instead of just a filename?
					return (INT_PTR)(currentView = WASABI_API_CREATEDIALOGPARAMW(IDD_VIEW_PLAYLIST, (HWND)param2, view_playlistDialogProc, lastActiveID));
				}
			}
			break;

		case ML_MSG_ONSENDTOBUILD:
			return playlists_BuildSendTo(param1, param2);

		case ML_MSG_ONSENDTOSELECT:
			return playlists_OnSendTo(param1, param2, param3);

		case ML_MSG_TREE_ONCLICK:
			return playlists_OnClick(param1, param2, (HWND)param3);

		case ML_MSG_NAVIGATION_CONTEXTMENU:
		{
			HNAVITEM hItem = (HNAVITEM)param1;
			HNAVITEM myItem = MLNavCtrl_FindItemById(plugin.hwndLibraryParent, playlistsTreeId);
			if (hItem == myItem)
			{
				return playlists_ContextMenu(param1, (HWND)param2, MAKEPOINTS(param3));
			}
			else
			{
				NAVITEM nvItem = {sizeof(NAVITEM),hItem,NIMF_ITEMID,};
				MLNavItem_GetInfo(plugin.hwndLibraryParent, &nvItem);
				if (FindTreeItem(nvItem.id))
				{
					HWND wnd = (HWND)param2;
					sendToIgnoreID = nvItem.id;
					HMENU menu = GetSubMenu(g_context_menus, 0),
						  sendToMenu = GetSubMenu(menu, 2);
					treeViewSendTo.AddHere(wnd, sendToMenu, ML_TYPE_FILENAMES, 1, (ML_TYPE_PLAYLIST+1)); // we're going to lie about the type for now
					// make sure that we call init on this otherwise the sendto menu will most likely fail
					treeViewSendTo.InitPopupMenu((WPARAM)sendToMenu);

					// tweaked to not fudge on the ml tree
					EnableMenuItem(menu, IDC_PLAY, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(menu, IDC_ENQUEUE, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(menu, IDC_DELETE, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(menu, ID_QUERYMENU_ADDNEWQUERY, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(menu, IDC_RENAME, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(menu, IDC_ENQUEUE, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(menu, 2, MF_BYPOSITION | MF_ENABLED);
					EnableMenuItem(menu, IDC_VIEWLIST, MF_BYCOMMAND | (nvItem.id != lastActiveID ? MF_ENABLED : MF_DISABLED));

					HMENU cloud_hmenu = (HMENU)0x666;
					size_t index = 0;
					if (playlists_CloudAvailable())
					{
						AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
						PlaylistInfo info;
						if (info.Associate(nvItem.id))
						{
							ReferenceCountedNXString uid;
							NXStringCreateWithFormatting(&uid, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
														 (int)info.playlist_guid.Data1, (int)info.playlist_guid.Data2,
														 (int)info.playlist_guid.Data3, (int)info.playlist_guid.Data4[0],
														 (int)info.playlist_guid.Data4[1], (int)info.playlist_guid.Data4[2],
														 (int)info.playlist_guid.Data4[3], (int)info.playlist_guid.Data4[4],
														 (int)info.playlist_guid.Data4[5], (int)info.playlist_guid.Data4[6],
														 (int)info.playlist_guid.Data4[7]);

							index = info.GetIndex();

							WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_GET_CLOUD_STATUS, (intptr_t)uid->string, (intptr_t)&cloud_hmenu);

							if (cloud_hmenu && cloud_hmenu != (HMENU)0x666)
							{
								MENUITEMINFOW m = {sizeof(m), MIIM_TYPE | MIIM_ID | MIIM_SUBMENU, MFT_SEPARATOR, 0};
								m.wID = CLOUD_SOURCE_MENUS - 1;
								InsertMenuItemW(menu, 3, TRUE, &m);

								wchar_t a[100] = {0};
								m.fType = MFT_STRING;
								m.dwTypeData = WASABI_API_LNGSTRINGW_BUF(IDS_CLOUD_SOURCES, a, 100);
								m.wID = CLOUD_SOURCE_MENUS;
								m.hSubMenu = cloud_hmenu;
								InsertMenuItemW(menu, 4, TRUE, &m);
							}
						}
					}

					bool swapPlayEnqueue=false;
					if (g_config->ReadInt(L"enqueuedef", 0) == 1)
					{
						SwapPlayEnqueueInMenu(menu);
						swapPlayEnqueue=true;
					}

					{
					HACCEL accel = WASABI_API_LOADACCELERATORSW(IDR_VIEW_PL_ACCELERATORS);
					int size = CopyAcceleratorTable(accel,0,0);
						AppendMenuShortcuts(menu, &accel, size, MSF_REPLACE);
					}

					if (swapPlayEnqueue)
						SwapPlayEnqueueInMenu(menu);

					POINT pt;
					POINTSTOPOINT(pt, MAKEPOINTS(param3));
					if (-1 == pt.x || -1 == pt.y)
					{
						NAVITEMGETRECT itemRect;
						itemRect.fItem = FALSE;
						itemRect.hItem = hItem;
						if (MLNavItem_GetRect(plugin.hwndLibraryParent, &itemRect))
						{
							MapWindowPoints(wnd, HWND_DESKTOP, (POINT*)&itemRect.rc, 2);
							pt.x = itemRect.rc.left + 2;
							pt.y = itemRect.rc.top + 2;
						}
					}

					int r = Menu_TrackPopup(plugin.hwndLibraryParent, menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, pt.x, pt.y, wnd, NULL);
					switch (r)
					{
						case IDC_PLAY:
						{
							AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
							PlaylistInfo info;
							if (info.Associate(nvItem.id))
							{
								playlist_SaveGUID(info.playlist_guid);
								mediaLibrary.PlayFile(info.GetFilename());
							}
						}
						break;
						case IDC_ENQUEUE:
						{
							AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
							PlaylistInfo info;
							if (info.Associate(nvItem.id))
							{
								playlist_SaveGUID(info.playlist_guid);
								mediaLibrary.EnqueueFile(info.GetFilename());
							}
						}
						break;
						case IDC_NEWPLAYLIST:
							playlists_Add(wnd);
							break;
						case IDC_DELETE:
							DeletePlaylist(tree_to_guid_map[nvItem.id], wnd, true);
							break;
						case IDC_RENAME:
							RenamePlaylist(tree_to_guid_map[nvItem.id], wnd);
							break;
						case IDC_VIEWLIST:
							mediaLibrary.SelectTreeItem(nvItem.id);
							break;
						default:
						{
							if (treeViewSendTo.WasClicked(r))
							{
								AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
								PlaylistInfo info;
								if (info.Associate(nvItem.id))
								{
									playlist_SaveGUID(info.playlist_guid);

									info.Refresh();

									mlPlaylist sendToPlaylist;
									sendToPlaylist.filename = info.GetFilename();
									sendToPlaylist.title = info.GetName();
									sendToPlaylist.numItems = info.GetSize();
									sendToPlaylist.length = info.GetLength();

									if (treeViewSendTo.SendPlaylist(&sendToPlaylist) != 1)
									{
										// didn't like that
										// let's try this way
										wchar_t filenames[MAX_PATH + 1] = {0};
										lstrcpyn(filenames, info.GetFilename(), MAX_PATH);
										filenames[lstrlen(filenames) + 1] = 0;
										treeViewSendTo.SendFilenames(filenames);
									}
								}
							}
							else
							{
								if (r >= CLOUD_SOURCE_MENUS && r < CLOUD_SOURCE_MENUS_PL_UPPER)	// deals with cloud specific menus
								{
									// 0 = no change
									// 1 = adding to cloud
									// 2 = added locally
									// 4 = removed
									int mode = -(int)index;
									WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_PROCESS_CLOUD_STATUS, (intptr_t)r, (intptr_t)&mode);
									if (mode > 0)
									{
										AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
										PlaylistInfo info;
										if (info.Associate(nvItem.id))
										{
											info.SetCloud((mode == 1 ? 1 : 0));
											AGAVE_API_PLAYLISTS->Flush();
											UpdatePlaylists();
										}
									}
								}
							}
						}
					}
					treeViewSendTo.Cleanup();
					sendToIgnoreID = 0;
					if (cloud_hmenu && cloud_hmenu != (HMENU)0x666)
					{
						DeleteMenu(menu, CLOUD_SOURCE_MENUS - 1, MF_BYCOMMAND);
						DeleteMenu(menu, CLOUD_SOURCE_MENUS, MF_BYCOMMAND);
						DestroyMenu(cloud_hmenu);
					}
					return TRUE;
				}
			}
		}
			return FALSE;

		case ML_MSG_TREE_ONKEYDOWN:
			return playlists_OnKeyDown(param1, (NMTVKEYDOWN *)param2, (HWND)param3);

		case ML_MSG_TREE_ONDRAG:
			return playlists_OnDrag(param1, (POINT *)param2, (int *)param3);

		case ML_MSG_TREE_ONDROP:
			return playlists_OnDrop(param1, (POINT *)param2, param3);

		case ML_MSG_TREE_ONDROPTARGET:
			return playlists_OnDropTarget(param1, param2, param3);

		case ML_MSG_PLAYING_FILE:
			lstrcpynW(current_playing, (wchar_t*)param1, FILENAME_SIZE);
			if (IsWindow(currentView)) PostMessage(currentView, WM_APP + 103, (WPARAM)current_playing, 0);
			return FALSE;

		case ML_MSG_WRITE_CONFIG:
			if (param1)
			{
				// only save the ml playlists if saving winamp.m3u/m3u8
				// else this will happen everytime the prefs are closed
				AGAVE_API_PLAYLISTS->Flush();
			}
			return FALSE;

		case ML_MSG_VIEW_PLAY_ENQUEUE_CHANGE:
		{
			enqueuedef = param1;
			groupBtn = param2;
			PostMessage(currentView, WM_APP + 104, param1, param2);
			return 0;
		}
	}
	return 0;
}

void myOpenURL(HWND hwnd, wchar_t *loc)
{
	if (loc)
	{
		bool override=false;
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::BROWSER, BrowserCallback::ONOPENURL, reinterpret_cast<intptr_t>(loc), reinterpret_cast<intptr_t>(&override));
		if (!override)
			ShellExecuteW(hwnd, L"open", loc, NULL, NULL, SW_SHOWNORMAL);
	}
}

int playlists_ContextMenu( INT_PTR param1, HWND hHost, POINTS pts )
{
	POINT pt;
	POINTSTOPOINT( pt, pts );
	if ( -1 == pt.x || -1 == pt.y )
	{
		HNAVITEM hItem = (HNAVITEM)param1;
		NAVITEMGETRECT itemRect;
		itemRect.fItem = FALSE;
		itemRect.hItem = hItem;
		if ( MLNavItem_GetRect( plugin.hwndLibraryParent, &itemRect ) )
		{
			MapWindowPoints( hHost, HWND_DESKTOP, (POINT *)&itemRect.rc, 2 );
			pt.x = itemRect.rc.left + 2;
			pt.y = itemRect.rc.top + 2;
		}
	}

	HMENU menu = GetSubMenu( g_context_menus, 1 );

	int r = Menu_TrackPopup( plugin.hwndLibraryParent, menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_NONOTIFY, pt.x, pt.y, hHost, NULL );
	switch ( r )
	{
		case IDC_NEWPLAYLIST:
			playlists_Add( hHost );
			break;
		case IDC_IMPORT_PLAYLIST_FROM_FILE:
			Playlist_importFromFile( hHost );
			break;
		case IDC_IMPORT_WINAMP_PLAYLIST:
			Playlist_importFromWinamp();
			break;
		case ID_PLAYLISTSMENU_IMPORTPLAYLISTFROMFOLDERS:
			Playlist_importFromFolders( hHost );
			break;
		case ID_SORTPLAYLIST_TITLE_A_Z:
			playlists_Sort( SORT_TITLE_ASCENDING );
			break;
		case ID_SORTPLAYLIST_TITLE_Z_A:
			playlists_Sort( SORT_TITLE_DESCENDING );
			break;
		case ID_SORTPLAYLIST_NUMBEROFITEMSASCENDING:
			playlists_Sort( SORT_NUMBER_ASCENDING );
			break;
		case ID_SORTPLAYLIST_NUMBEROFITEMSDESCENDING:
			playlists_Sort( SORT_NUMBER_DESCENDING );
			break;
		case ID_PLAYLISTS_HELP:
			myOpenURL( hHost, L"https://help.winamp.com/hc/articles/8109547717268-Winamp-Playlists" );
			break;
	}

	Sleep( 100 );
	MSG msg;
	while ( PeekMessage( &msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE ) ); //eat return

	return TRUE;
}

// param1 = param of tree item, param2 = action type (below), param3 = HWND of main window
INT_PTR playlists_OnClick(INT_PTR treeId, int clickType, HWND wnd)
{
	switch (clickType)
	{
		case ML_ACTION_DBLCLICK:
		case ML_ACTION_ENTER:
		{
			if (FindTreeItem(treeId))
			{
				AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
				PlaylistInfo info;
				if (info.Associate(treeId))
				{
					playlist_SaveGUID(info.playlist_guid);
					bool flip = (clickType==ML_ACTION_ENTER) && (GetAsyncKeyState(VK_SHIFT)&0x8000);
			
					if ((!!(g_config->ReadInt(L"enqueuedef", 0) == 1)) ^ flip)
						mediaLibrary.EnqueueFile(info.GetFilename());
					else
						mediaLibrary.PlayFile(info.GetFilename());
				}
				return 1;
			}
		}
		break;
	}
	return 0;
}

int playlists_OnKeyDown(int treeId, NMTVKEYDOWN *p, HWND hwndDlg)
{
	int ctrl = (GetAsyncKeyState(VK_CONTROL)&0x8000);
	int shift = (GetAsyncKeyState(VK_SHIFT)&0x8000);

	if (treeId == playlistsTreeId)
	{
		switch (p->wVKey)
		{
			case VK_INSERT:
			{
				if (shift && !ctrl) playlists_Add(plugin.hwndLibraryParent); return 1;
			}
		}
	}
	else if (FindTreeItem(treeId))
	{
		switch (p->wVKey)
		{
			case VK_F2: if (!shift && !ctrl) RenamePlaylist(tree_to_guid_map[treeId], plugin.hwndLibraryParent);	return 1;
			case VK_INSERT: if (shift && !ctrl) playlists_Add(plugin.hwndLibraryParent); return 1;
			case VK_DELETE: if (!shift && !ctrl) DeletePlaylist(tree_to_guid_map[treeId], hwndDlg, true);  return 1;
		}
	}
	return 0;
}

int playlists_OnDrag(int treeId, POINT *pt, int *type)
{
	if (FindTreeItem(treeId))
	{
		*type = ML_TYPE_FILENAMES;
		return 1;
	}
	return 0;
}

int playlists_OnDrop(int treeId, POINT *pt, int destTreeId)
{
	if (FindTreeItem(treeId))
	{
		AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
		PlaylistInfo info;
		if (info.Associate(treeId))
		{
			if (destTreeId == playlistsTreeId)
			{
				mediaLibrary.RemoveTreeItem(info.treeId);
				tree_to_guid_map.erase(info.treeId);
				AGAVE_API_PLAYLISTS->MoveBefore(info.GetIndex(), 0);

				// TODO: move most of this to PlaylistsCB
				// TODO use the more native code like in MakeTree(..)
				MLTREEITEMW src = {sizeof(MLTREEITEMW), };
				src.title = const_cast<wchar_t *>(info.GetName());
				src.hasChildren = 0;
				src.parentId = playlistsTreeId;
				src.id = destTreeId;
				src.imageIndex = (!info.GetCloud() ? imgPL : imgCloudPL);
				mediaLibrary.InsertTreeItem(src);
				info.treeId = src.id;
				tree_to_guid_map[info.treeId] = info.playlist_guid;
				mediaLibrary.SelectTreeItem(info.treeId);
			}
			else if (FindTreeItem(destTreeId))
			{
				PlaylistInfo dest;
				if (dest.Associate(destTreeId))
				{
					mediaLibrary.RemoveTreeItem(info.treeId);
					tree_to_guid_map.erase(info.treeId);
					AGAVE_API_PLAYLISTS->MoveBefore(info.GetIndex(), dest.GetIndex()+1);

					// TODO: move most of this to PlaylistsCB
					// TODO use the more native code like in MakeTree(..)
					MLTREEITEMW src = {sizeof(MLTREEITEMW), };
					src.title = const_cast<wchar_t *>(info.GetName());
					src.hasChildren = 0;
					src.parentId = playlistsTreeId;
					src.id = destTreeId;
					src.imageIndex = (!info.GetCloud() ? imgPL : imgCloudPL);
					mediaLibrary.InsertTreeItem(src);
					info.treeId = src.id;
					tree_to_guid_map[info.treeId] = info.playlist_guid;
					mediaLibrary.SelectTreeItem(info.treeId);
				}
				return 1;
			}
			else
			{
				playlist_SaveGUID(info.playlist_guid);

				info.Refresh();
				mlPlaylist pl;
				pl.filename = info.GetFilename();
				pl.length = info.GetLength();
				pl.numItems = info.GetSize();
				pl.title = info.GetName();

				mlDropItemStruct m = {0};
				m.p = *pt;
				m.flags = 0;
				m.type = ML_TYPE_PLAYLIST;
				m.result = 0;
				m.data = (void *) & pl;
				m.name = 0;
				pluginHandleIpcMessage(ML_IPC_HANDLEDROP, (WPARAM)&m);
				/* TODO: fall back to this is result fails?
				mlDropItemStruct m = {0};
				m.p = *pt;
				m.flags = ML_HANDLEDRAG_FLAG_NAME;
				m.type=ML_TYPE_FILENAMES;
				m.result = 0;
				char filename[1025] = {0};
				lstrcpynA(filename, AutoCharFn(playlists[index].filename), 1024);
				filename[lstrlenA(filename)+1]=0;
				m.data=(void *)filename;
				AutoChar charTitle(playlists[index].title);
				m.name = charTitle;
				pluginHandleIpcMessage(ML_IPC_HANDLEDROP, (WPARAM)&m);
				*/
			}
		}
	}
	return 0;
}