#include "main.h"
#include "PlaylistView.h"
#include "Playlist.h"
#include "CurrentPlaylist.h"
#include "api__ml_playlists.h"
#include "../ml_local/api_mldb.h"
#include "../ml_pmp/pmp.h"
#include <strsafe.h>

extern Playlist currentPlaylist;

static BOOL playlist_GetDisplayInfo( NMLVDISPINFO *lpdi )
{
	size_t item = lpdi->item.iItem;
	if ( item < 0 || item >= currentPlaylist.GetNumItems() )
		return 0;

	if ( lpdi->item.mask & LVIF_TEXT )
	{
		switch ( lpdi->item.iSubItem )
		{
			case 0:
			{
				if ( !currentPlaylist.IsCached( item ) )
				{
					wchar_t title[ FILETITLE_SIZE ] = { 0 };
					int length = -1;
					mediaLibrary.GetFileInfo( currentPlaylist.ItemName( item ), title, FILETITLE_SIZE, &length );
					currentPlaylist.SetItemLengthMilliseconds( item, length * 1000 );
					currentPlaylist.SetItemTitle( item, title );
				}

				// CUT: currentPlaylist.GetItemTitle(item, lpdi->item.pszText, lpdi->item.cchTextMax);
				const wchar_t *title = currentPlaylist.ItemTitle( item );
				if ( !title )
					title = currentPlaylist.ItemName( item );

				// TODO - just using for debugging to check values
#ifdef DEBUG
				wchar_t info[ 128 ] = { 0 };
				if ( currentPlaylist.GetItemExtendedInfo( item, L"cloud", info, 128 ) )
				{
					StringCchPrintf( lpdi->item.pszText, lpdi->item.cchTextMax, L"[%s] %d. %s", info, item + 1, title );
				}
				else
				{
#endif
					StringCchPrintf( lpdi->item.pszText, lpdi->item.cchTextMax, L"%d. %s", item + 1, title );
#ifdef DEBUG
				}
#endif
			}
			break;

			case 1:
			{
				wchar_t info[ 16 ] = { 0 };
				if ( currentPlaylist.GetItemExtendedInfo( item, L"cloud_status", info, 16 ) )
				{
					StringCchPrintf( lpdi->item.pszText, lpdi->item.cchTextMax, L"%s", info );
				}
				else
					StringCchPrintf( lpdi->item.pszText, lpdi->item.cchTextMax, L"%d", 4 );
			}
			break;

			case 2:
			{
				if ( currentPlaylist.GetItemLengthMilliseconds( item ) == 0 ) // if the length is 0, then we'll re-read it
				{
					wchar_t title[ FILETITLE_SIZE ] = { 0 };
					int length = 0;
					mediaLibrary.GetFileInfo( currentPlaylist.ItemName( item ), title, FILETITLE_SIZE, &length );
					if ( length == 0 )
						currentPlaylist.SetItemLengthMilliseconds( item, -1000 );
					else
					{
						currentPlaylist.SetItemLengthMilliseconds( item, length * 1000 );
					}
				}

				int length = currentPlaylist.GetItemLengthMilliseconds( item ) / 1000;
				if ( length <= 0 )
					lpdi->item.pszText[ 0 ] = 0;
				else
					StringCchPrintf( lpdi->item.pszText, lpdi->item.cchTextMax, L"%d:%02d", length / 60, length % 60 );
			}
			break;
		}
	}

	return 0;
}

BOOL playlist_OnCustomDraw( HWND hwndDlg, NMLVCUSTOMDRAW *plvcd, LRESULT *pResult )
{
	static BOOL bDrawFocus;
	static RECT rcView;
	static CLOUDCOLUMNPAINT cloudColumnPaint;

	*pResult = CDRF_DODEFAULT;

	switch ( plvcd->nmcd.dwDrawStage )
	{
		case CDDS_PREPAINT:
			*pResult |= CDRF_NOTIFYITEMDRAW;
			CopyRect( &rcView, &plvcd->nmcd.rc );

			cloudColumnPaint.hwndList = plvcd->nmcd.hdr.hwndFrom;
			cloudColumnPaint.hdc      = plvcd->nmcd.hdc;
			cloudColumnPaint.prcView  = &rcView;
			return TRUE;

		case CDDS_ITEMPREPAINT:
			*pResult |= CDRF_NOTIFYSUBITEMDRAW;
			bDrawFocus = ( CDIS_FOCUS & plvcd->nmcd.uItemState );
			if ( bDrawFocus )
			{
				plvcd->nmcd.uItemState &= ~CDIS_FOCUS;
				*pResult |= CDRF_NOTIFYPOSTPAINT;
			}
			return TRUE;

		case CDDS_ITEMPOSTPAINT:
			if ( bDrawFocus )
			{
				RECT rc;
				rc.left = LVIR_BOUNDS;
				SendMessageW( plvcd->nmcd.hdr.hwndFrom, LVM_GETITEMRECT, plvcd->nmcd.dwItemSpec, (LPARAM)&rc );

				rc.left += 3;
				DrawFocusRect( plvcd->nmcd.hdc, &rc );

				plvcd->nmcd.uItemState |= CDIS_FOCUS;
				bDrawFocus = FALSE;
			}
			*pResult = CDRF_SKIPDEFAULT;
			return TRUE;

		case( CDDS_SUBITEM | CDDS_ITEMPREPAINT ):
			// TODO need to have a map between column ids so we do this correctly
			if ( plvcd->iSubItem == 1 )
			{
				if ( 0 == plvcd->iSubItem && 0 == plvcd->nmcd.rc.right )
					break;

				cloudColumnPaint.iItem    = plvcd->nmcd.dwItemSpec;
				cloudColumnPaint.iSubItem = plvcd->iSubItem;

				int cloud_icon = 4;
				size_t item = plvcd->nmcd.dwItemSpec;

				wchar_t info[ 16 ] = { 0 };
				if ( currentPlaylist.GetItemExtendedInfo( item, L"cloud_status", info, 16 ) )
					cloud_icon = _wtoi( info );

				// TODO have this show an appropriate cloud icon for the playlist
				//		currently all we have is cloud or nothing as we'll only
				//		have files locally for this for the moment (need todo!!!)
				cloudColumnPaint.value   = cloud_icon;
				cloudColumnPaint.prcItem = &plvcd->nmcd.rc;
				cloudColumnPaint.rgbBk   = plvcd->clrTextBk;
				cloudColumnPaint.rgbFg   = plvcd->clrText;

				if ( MLCloudColumn_Paint( plugin.hwndLibraryParent, &cloudColumnPaint ) )
				{
					*pResult = CDRF_SKIPDEFAULT;

					return TRUE;
				}
			}
			break;
	}

	return FALSE;
}

BOOL playlist_Notify( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	LPNMHDR l = (LPNMHDR)lParam;
	if ( l->idFrom == IDC_PLAYLIST_EDITOR )
	{
		switch ( l->code )
		{
			case NM_DBLCLK:
				PlaySelection( g_config->ReadInt( L"enqueuedef", 0 ) == 1, g_config->ReadInt( L"plplaymode", 1 ) );
				break;

			case LVN_GETDISPINFO:
				return playlist_GetDisplayInfo( (NMLVDISPINFO *)lParam );

			case LVN_BEGINDRAG:
				we_are_drag_and_dropping = 1;
				SetCapture( hwndDlg );
				break;

			case LVN_ITEMCHANGED:
			case LVN_ODSTATECHANGED:
				UpdatePlaylistTime( hwndDlg );
				break;

			case NM_CUSTOMDRAW:
			{
				LRESULT result = 0;
				if ( cloud_avail && playlist_OnCustomDraw( hwndDlg, (NMLVCUSTOMDRAW *)lParam, &result ) )
				{
					SetWindowLongPtrW( hwndDlg, DWLP_MSGRESULT, (LONG_PTR)result );

					return 1;
				}
				break;
			}

			case NM_CLICK:
			{
				LPNMITEMACTIVATE pnmitem = (LPNMITEMACTIVATE)lParam;
				if ( cloud_avail && pnmitem->iItem != -1 && pnmitem->iSubItem == 1 )
				{
					RECT itemRect = { 0 };
					if ( pnmitem->iSubItem )
						ListView_GetSubItemRect( pnmitem->hdr.hwndFrom, pnmitem->iItem, pnmitem->iSubItem, LVIR_BOUNDS, &itemRect );
					else
					{
						ListView_GetItemRect( pnmitem->hdr.hwndFrom, pnmitem->iItem, &itemRect, LVIR_BOUNDS );
						itemRect.right = itemRect.left + ListView_GetColumnWidth( pnmitem->hdr.hwndFrom, pnmitem->iSubItem );
					}

					MapWindowPoints( l->hwndFrom, HWND_DESKTOP, (POINT *)&itemRect, 2 );

					//int cloud_devices = 0;
					HMENU cloud_hmenu = 0;
					int mark = playlist_list.GetSelectionMark();
					if ( mark != -1 )
					{
						wchar_t filename[ MAX_PATH ] = { 0 };
						currentPlaylist.entries[ mark ]->GetFilename( filename, MAX_PATH );

						cloud_hmenu = CreatePopupMenu();
						WASABI_API_SYSCB->syscb_issueCallback( api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_GET_CLOUD_STATUS, (intptr_t)&filename, (intptr_t)&cloud_hmenu );
						if ( cloud_hmenu )
						{
							int r = Menu_TrackPopup( plugin.hwndLibraryParent, cloud_hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_NONOTIFY, itemRect.right, itemRect.top, hwndDlg, NULL );
							if ( r >= CLOUD_SOURCE_MENUS && r < CLOUD_SOURCE_MENUS_UPPER )
							{   // deals with cloud specific menus
								// 0 = no change
								// 1 = adding to cloud
								// 2 = added locally
								// 4 = removed

								int mode = 0;	// deals with cloud specific menus
								WASABI_API_SYSCB->syscb_issueCallback( api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_PROCESS_CLOUD_STATUS, (intptr_t)r, (intptr_t)&mode );
								// TODO
								/*switch (mode)
								{
									case 1:
										setCloudValue(&itemCache.Items[pnmitem->iItem], L"5");
									break;

									case 2:
										setCloudValue(&itemCache.Items[pnmitem->iItem], L"4");
									break;

									case 4:
										setCloudValue(&itemCache.Items[pnmitem->iItem], L"4");
									break;
								}
								InvalidateRect(resultlist.getwnd(), NULL, TRUE);*/
							}

							DestroyMenu( cloud_hmenu );
						}
					}
				}
			}
			break;
		}
	}

	switch ( l->code )
	{
		case HDN_ITEMCHANGING:
		{
			LPNMHEADERW phdr = (LPNMHEADERW)lParam;
			if ( phdr->pitem && ( HDI_WIDTH & phdr->pitem->mask ) && phdr->iItem == 1 )
			{
				if ( !cloud_avail )
					phdr->pitem->cxy = 0;
				else
				{
					INT width = phdr->pitem->cxy;
					if ( MLCloudColumn_GetWidth( plugin.hwndLibraryParent, &width ) )
						phdr->pitem->cxy = width;
				}
			}
			break;
		}
	}

	return 0;
}