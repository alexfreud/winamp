/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "main.h"
#include "menuv5.h"
#include "../nu/AutoWide.h"
#include "../Plugins/General/gen_ml/ml.h"

#define TREE_LOCALMEDIA 1000
#define TREE_PLAYLISTS 3001
#define TREE_DEVICES 10000
#define TREE_QUERIES 1000

extern HINSTANCE language_pack_instance;


void ensureInScreen(HMENU menu, int *x, int *y, int *flag, int width, int height);

HMENU v5_top_menu = NULL;


int MergeMenu(HMENU pMenuDestination, const HMENU pMenuAdd, int bTopLevel /*=false*/)
{
    // Abstract:
    //      Merges two menus.
    //
    // Parameters:
    //      pMenuDestination    - [in, retval] destination menu handle
    //      pMenuAdd            - [in] menu to merge
    //      bTopLevel           - [in] indicator for special top level behavior
    //
    // Return value:
    //      <false> in case of error.
    //
    // Comments:
    //      This function calles itself recursivley. If bTopLevel is set to true,
    //      we append popups at top level or we insert before <Window> or <Help>.

    // get the number menu items in the menus
    int iMenuAddItemCount = GetMenuItemCount(pMenuAdd);
    int iMenuDestItemCount = GetMenuItemCount(pMenuDestination);
		int iLoop;
    
    // if there are no items return
    if( iMenuAddItemCount == 0 )
        return 1;
    
    // if we are not at top level and the destination menu is not empty
    // -> we append a seperator
    if( !bTopLevel && iMenuDestItemCount > 0 )
			AppendMenu(pMenuDestination, MF_SEPARATOR, 0, 0);

    // iterate through the top level of 
    for(  iLoop = 0; iLoop < iMenuAddItemCount; iLoop++ )
    {
		HMENU pSubMenu = 0;
        // get the menu string from the add menu
        wchar_t sMenuAddString[1024] = {0}; // hope it's enough

		GetMenuStringW(pMenuAdd, iLoop, sMenuAddString, 1024, MF_BYPOSITION);

        // try to get the submenu of the current menu item
         pSubMenu =GetSubMenu(pMenuAdd, iLoop);

        // check if we have a sub menu
        if (!pSubMenu)
        {
            // normal menu item
            // read the source and append at the destination
            UINT nState = GetMenuState(pMenuAdd, iLoop, MF_BYPOSITION);
            UINT nItemID = GetMenuItemID(pMenuAdd, iLoop);
            
            if(AppendMenuW(pMenuDestination, nState, nItemID, sMenuAddString))
            {
                // menu item added, don't forget to correct the item count
                iMenuDestItemCount++;
            }
            else
            {
                // MergeMenu: AppendMenu failed!
                return 0;
            }
        }
        else
        {
					HMENU NewPopupMenu=NULL;
            // create or insert a new popup menu item
            
            // default insert pos is like ap
            int iInsertPosDefault = -1;
            
            // if we are at top level merge into existing popups rather than
            // creating new ones
            if( bTopLevel )
            {
                //ASSERT( sMenuAddString != "&?" && sMenuAddString != "?" );
                //CString sAdd( sMenuAddString );
                //sAdd.Remove('&');  // for comparison of menu items supress '&'
                int bAdded = 0;
								int iLoop1=0;

                // try to find existing popup
                for(  iLoop1 = 0; iLoop1 < iMenuDestItemCount; iLoop1++ )
                {
                    // get the menu string from the destination menu
                    wchar_t sDest[1024] = {0}; // hope it's enough
                    GetMenuStringW(pMenuDestination, iLoop1, sDest, 1024, MF_BYPOSITION );
                    //sDest.Remove( '&' ); // for a better compare (s.a.)

                    //if( !lstrcmp(sAdd,sDest))
                    {
                        // we got a hit -> merge the two popups
                        // try to get the submenu of the desired destination menu item
                        HMENU pSubMenuDest = GetSubMenu(pMenuDestination, iLoop1 );
                        
                        if( pSubMenuDest )
                        {
                            // merge the popup recursivly and continue with outer for loop
                            if( !MergeMenu( pSubMenuDest, pSubMenu, 0 ))
                                return 0;
                            
                            bAdded = 1;
                            break;
                        }
                    }

                    // alternativ insert before <Window> or <Help>
                    //if( iInsertPosDefault == -1 && ( sDest == "Window" || sDest == "?" || sDest == "Help" ))
                    //    iInsertPosDefault = iLoop1;

                }
                
                if( bAdded )
                {
                    // menu added, so go on with loop over pMenuAdd's top level
                    continue;
                }
            }

            // if the top level search did not find a position append the menu
            if( iInsertPosDefault == -1 )
                iInsertPosDefault = GetMenuItemCount(pMenuDestination);
            
            // create a new popup and insert before <Window> or <Help>
            NewPopupMenu = CreatePopupMenu();
            if( !NewPopupMenu)
            {
                // MergeMenu: CreatePopupMenu failed!
                return 0;
            }
            
            // merge the new popup recursivly
            if( !MergeMenu( NewPopupMenu, pSubMenu, 0 ))
                return 0;
            
            // insert the new popup menu into the destination menu
             HMENU hNewMenu = NewPopupMenu;
						{
						MENUITEMINFOW menuItem={sizeof(MENUITEMINFO),
							MIIM_TYPE|MIIM_SUBMENU, 
							MFT_STRING,
							MFS_ENABLED, 
							0,  //wID
							hNewMenu, // hSubMenu
							NULL,  // hbmpChecked
							NULL, // hbmpUnchecked
							0, // dwItemData
							sMenuAddString,  // dwTypeData
							0,  // cch
						};
						
						if (InsertMenuItemW(pMenuDestination, iInsertPosDefault, TRUE, &menuItem))
            {
                // don't forget to correct the item count
                iMenuDestItemCount++;
            }
            else
            {
                // MergeMenu: InsertMenu failed!
                return 0;
            }
						}

        } 
    } 
    
    return 1;
}

int getMenuItemPos(HMENU menu, UINT command)
{
	int i;
	for (i = 0;i < 256;i++)
	{
		MENUITEMINFO mii = {sizeof(mii), MIIM_ID, };
		if (!GetMenuItemInfo(menu, i, TRUE, &mii)) break;
		if (mii.wID == command) return i;
	}
	return -1;
}

extern int g_SkinTop, g_BookmarkTop;

int V5_File_Menu(HWND hwnd, int x, int y, int width, int height)
{
	int flag = TPM_LEFTALIGN;
	HMENU file_menu = GetSubMenu(v5_top_menu, 0);
	HMENU hMenu = GetSubMenu(file_menu, 3);
	MENUITEMINFOW i = {sizeof(i), };
	FILE *fp;
	int a = 34768;
	int offs = 3;
	int count = GetMenuItemCount(hMenu) + 1;

	i.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID;
	i.fType = MFT_STRING;
	i.wID = 34768;

	// remove all of the items we might have added - do by command for certainty
	while (count){
		if(!RemoveMenu(hMenu, a++, MF_BYCOMMAND)) break;
		count--;
	}

	fp = _wfopen(BOOKMARKFILE8, L"rt");
	if (fp)
	{
		while (1)
		{
			char ft[FILETITLE_SIZE] = {0}, fn[FILENAME_SIZE] = {0};
			fgets(fn, FILENAME_SIZE, fp);
			if (feof(fp)) break;
			fgets(ft, FILETITLE_SIZE, fp);
			if (feof(fp)) break;
			if (ft[0] && fn[0])
			{
				if (fn[lstrlenA(fn) - 1] == '\n') fn[lstrlenA(fn) - 1] = 0;
				if (ft[lstrlenA(ft) - 1] == '\n') ft[lstrlenA(ft) - 1] = 0;
				if (ft[0] && fn[0])
				{
					i.dwTypeData = AutoWideDup(ft, CP_UTF8);
					i.cch = lstrlenW(i.dwTypeData);
					RemoveMenu(hMenu, i.wID, MF_BYCOMMAND);
					InsertMenuItemW(hMenu, i.wID + offs - 34768, TRUE, &i);
					i.wID++;
				}
			}
		}
		fclose(fp);
	}
	g_BookmarkTop = i.wID;

	// put in a place holder item if there were no read bookmarks
	if (g_BookmarkTop == 34768)
	{
		i.dwTypeData = getStringW(IDS_NO_BOOKMARKS,NULL,0);
		i.cch = lstrlenW(i.dwTypeData);
		InsertMenuItemW(hMenu, i.wID + offs - 34768, TRUE, &i);
		EnableMenuItem(hMenu, i.wID, MF_BYCOMMAND | MF_GRAYED);
	}

	ensureInScreen(file_menu, &x, &y, &flag, width, height);

	DoTrackPopup(file_menu, flag, x, y, hwnd);
	return 1;
}

int V5_Play_Menu(HWND hwnd, int x, int y, int width, int height)
{
	HMENU play_menu = GetSubMenu(v5_top_menu, 1);
	int flag = TPM_LEFTALIGN;
	ensureInScreen(play_menu, &x, &y, &flag, width, height);
	DoTrackPopup(play_menu, flag, x, y, hwnd);
	return 1;
}


int V5_Options_Menu(HWND hwnd, int x, int y, int width, int height)
{
	int flag = TPM_LEFTALIGN;
	HMENU options_menu = GetSubMenu(v5_top_menu, 2);
	HMENU eqMenu = NULL;

	{ // set options skin menu to the skin menu
		extern HMENU g_submenus_skins1;
		MENUITEMINFO mi = {sizeof(mi), MIIM_SUBMENU};
		mi.hSubMenu = g_submenus_skins1;
		SetMenuItemInfoW(options_menu, 0, TRUE, &mi);
	}

	eqMenu = GetSubMenu(options_menu, 2);
	CheckMenuItem(eqMenu, EQ_ENABLE, config_use_eq ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(options_menu, WINAMP_OPTIONS_DSIZE, config_dsize ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(options_menu, WINAMP_OPTIONS_AOT, config_aot ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(options_menu, WINAMP_OPTIONS_ELAPSED, config_timeleftmode ? MF_UNCHECKED : MF_CHECKED);
	CheckMenuItem(options_menu, WINAMP_OPTIONS_REMAINING, config_timeleftmode ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(options_menu, WINAMP_OPTIONS_PREFS, IsWindow(prefs_hwnd) ? MF_CHECKED : MF_UNCHECKED);
	ensureInScreen(options_menu, &x, &y, &flag, width, height);
	DoTrackPopup(options_menu, flag, x, y, hwnd);
	return 1;
}

int V5_Windows_Menu(HWND hwnd, int x, int y, int width, int height)
{
	HMENU windows_menu = GetSubMenu(v5_top_menu, 3);
	int flag = TPM_LEFTALIGN;
	CheckMenuItem(windows_menu, WINAMP_OPTIONS_PLEDIT, config_pe_open ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(windows_menu, WINAMP_VISPLUGIN, vis_running() ? MF_CHECKED : MF_UNCHECKED);
	if (g_has_video_plugin) CheckMenuItem(windows_menu, WINAMP_OPTIONS_VIDEO, config_video_open ? MF_CHECKED : MF_UNCHECKED);
	ensureInScreen(windows_menu, &x, &y, &flag, width, height);
	DoTrackPopup(windows_menu, flag, x, y, hwnd);
	return 1;
}

int Help_Menu(HWND hwnd, int x, int y, int width, int height)
{
	HMENU help_menu = GetSubMenu(v5_top_menu, 4);
	int flag = TPM_LEFTALIGN;

	MENUITEMINFOW i = {sizeof(i), };
	i.fMask = MIIM_TYPE;
	i.fType = MFT_STRING;
	i.dwTypeData = getStringW(IDS_WINAMP_MENUITEM, NULL, 0);
	i.cch = (UINT)wcslen(i.dwTypeData);
	SetMenuItemInfoW(help_menu, WINAMP_HELP_ABOUT, FALSE, &i);

	ensureInScreen(help_menu, &x, &y, &flag, width, height);
	DoTrackPopup(help_menu, flag, x, y, hwnd);
	return 1;
}

int V5_Help_Menu(HWND hwnd, int x, int y, int width, int height)
{
	return Help_Menu(hwnd, x, y, width, height);
}

LRESULT sendMlIpc(int msg, WPARAM param);

// TODO:: need to make this only show what's needed at the time ie it'll still show even if there's no ml_playlists
//        and properly show if there's no playlists
int V5_PE_File_Menu(HWND hwnd, int x, int y, int width, int height)
{
	int flag = TPM_LEFTALIGN;
	HMENU pefile_menu = GetSubMenu(v5_top_menu, 5);
	HMENU playlistsmenu = NULL;
	HMENU viewmenu = NULL;
	HWND mlwnd = (HWND)sendMlIpc(0, 0);
	int viewmenu_added = 0;
	g_open_ml_item_in_pe = 1;
	if (mlwnd)
	{
		mlGetTreeStruct mgts = { TREE_PLAYLISTS, 45000, -1 };
		playlistsmenu = (HMENU)SendMessageW(mlwnd, WM_ML_IPC, (WPARAM) & mgts, ML_IPC_GETTREE);
		if (playlistsmenu)
		{
			mlGetTreeStruct mgts = { TREE_QUERIES, 45000, -1 };
			viewmenu = (HMENU)SendMessageW(mlwnd, WM_ML_IPC, (WPARAM) & mgts, ML_IPC_GETTREE);
			if (GetMenuItemCount(playlistsmenu) == 0) InsertMenuW(playlistsmenu, 0, MF_BYPOSITION | MF_STRING | MF_GRAYED, 0, getStringW(IDS_ML_NO_PLAYLISTS,NULL,0));
			InsertMenuW(pefile_menu, 2, MF_BYPOSITION | MF_ENABLED | MF_POPUP, (UINT_PTR)playlistsmenu, getStringW(IDS_ML_OPEN_PLAYLIST,NULL,0));
			if (viewmenu && GetMenuItemCount(viewmenu) > 0)
			{
				viewmenu_added = 1;
				InsertMenuW(pefile_menu, 3, MF_BYPOSITION | MF_ENABLED | MF_POPUP, (UINT_PTR)viewmenu, getStringW(IDS_ML_OPEN_VIEW_RESULTS,NULL,0));
			}
		}
	}

	ModifyMenuW(pefile_menu, ID_PE_CLOSE, MF_BYCOMMAND | MF_STRING, ID_PE_CLOSE, config_pe_open ? getStringW(IDS_PE_CLOSE,NULL,0) : getStringW(IDS_PE_OPEN,NULL,0));
	ensureInScreen(pefile_menu, &x, &y, &flag, width, height);
	DoTrackPopup(pefile_menu, flag, x, y, hwnd);

	if (playlistsmenu)
	{
		if (viewmenu_added) RemoveMenu(pefile_menu, 3, MF_BYPOSITION);
		if (viewmenu) DestroyMenu(viewmenu);
		RemoveMenu(pefile_menu, 2, MF_BYPOSITION);
		DestroyMenu(playlistsmenu);
	}
	return 1;
}

int V5_PE_Playlist_Menu(HWND hwnd, int x, int y, int width, int height)
{
	HMENU peplaylist_menu = GetSubMenu(v5_top_menu, 6);
	int flag = TPM_LEFTALIGN;
	ensureInScreen(peplaylist_menu, &x, &y, &flag, width, height);
	DoTrackPopup(peplaylist_menu, flag, x, y, hwnd);
	return 1;
}

int V5_PE_Sort_Menu(HWND hwnd, int x, int y, int width, int height)
{
	HMENU pesort_menu = GetSubMenu(v5_top_menu, 7);
	int flag = TPM_LEFTALIGN;
	ensureInScreen(pesort_menu, &x, &y, &flag, width, height);
	DoTrackPopup(pesort_menu, flag, x, y, hwnd);
	return 1;
}

int V5_PE_Help_Menu(HWND hwnd, int x, int y, int width, int height)
{
	return Help_Menu(hwnd, x, y, width, height);
}

int V5_ML_File_Menu(HWND hwnd, int x, int y, int width, int height)
{
	int flag = TPM_LEFTALIGN;
	HMENU mlfile_menu = GetSubMenu(v5_top_menu, 8);
	HWND mlwnd = (HWND)sendMlIpc(0, 0);
	HWND mlplwnd = (HWND)SendMessageW(mlwnd, WM_ML_IPC, 0, ML_IPC_GETPLAYLISTWND);
	ModifyMenuW(mlfile_menu, 3, MF_BYPOSITION | (mlplwnd == NULL ? MF_GRAYED : 0), ID_MLFILE_SAVEPLAYLIST, getStringW(IDS_ML_EXPORT_PLAYLIST,NULL,0));
	{
		int visible = IsWindowVisible(mlwnd);
		int p = getMenuItemPos(mlfile_menu, ID_FILE_CLOSELIBRARY);
		if (p == -1) p = getMenuItemPos(mlfile_menu, ID_FILE_SHOWLIBRARY);
		ModifyMenuW(mlfile_menu, p, MF_BYPOSITION | MF_STRING, visible ? ID_FILE_CLOSELIBRARY : ID_FILE_SHOWLIBRARY, visible ? getStringW(IDS_ML_CLOSE_ML,NULL,0) : getStringW(IDS_ML_OPEN_ML,NULL,0));
	}
	ensureInScreen(mlfile_menu, &x, &y, &flag, width, height);
	DoTrackPopup(mlfile_menu, flag, x, y, hwnd);
	return 1;
}

int V5_ML_View_Menu(HWND hwnd, int x, int y, int width, int height)
{
	int flag = TPM_LEFTALIGN;
	HMENU mlview_menu = GetSubMenu(v5_top_menu, 9), mediamenu = NULL;
	HWND mlwnd = (HWND)sendMlIpc(0, 0);
	if (mlwnd)
	{
		mlGetTreeStruct mgts = { 0, 45000, -1 };
		mediamenu = (HMENU)SendMessageW(mlwnd, WM_ML_IPC, (WPARAM) & mgts, ML_IPC_GETTREE);
		if (mediamenu)
		{
			MergeMenu(mediamenu, mlview_menu, 0);
			//InsertMenu(mediamenu, 0, MF_BYPOSITION | MF_STRING, ID_MLVIEW_MEDIA, "All &Media");
			//InsertMenu(mlview_menu, 1, MF_BYPOSITION | MF_ENABLED | MF_POPUP, (UINT)mediamenu, "&Local Media");
		}/*
		mgts.item_start = TREE_PLAYLISTS;
		playlistsmenu = (HMENU)SendMessageW(mlwnd, WM_ML_IPC, (int) & mgts, ML_IPC_GETTREE);
		if (playlistsmenu)
		{
			if (GetMenuItemCount(playlistsmenu) == 0) InsertMenu(playlistsmenu, 0, MF_BYPOSITION | MF_STRING | MF_GRAYED, 0, "No playlists");
			InsertMenu(mlview_menu, 2, MF_BYPOSITION | MF_ENABLED | MF_POPUP, (UINT)playlistsmenu, "&Playlists");
		}
		mgts.item_start = TREE_DEVICES;
		devicesmenu = (HMENU)SendMessageW(mlwnd, WM_ML_IPC, (int) & mgts, ML_IPC_GETTREE);
		if (devicesmenu)
		{
			if (GetMenuItemCount(devicesmenu) == 0) InsertMenu(devicesmenu, 0, MF_BYPOSITION | MF_STRING | MF_GRAYED, 0, "No devices");
			InsertMenu(mlview_menu, 3, MF_BYPOSITION | MF_ENABLED | MF_POPUP, (UINT)devicesmenu, "&Devices");
		}*/
	}

	g_open_ml_item_in_pe = 0;

	//ID_MLVIEW_PLAYLISTS
	//ID_MLVIEW_DEVICES

	ensureInScreen(mediamenu, &x, &y, &flag, width, height);
	DoTrackPopup(mediamenu, flag, x, y, hwnd);
/*
	if (devicesmenu)
	{
		RemoveMenu(mlview_menu, 3, MF_BYPOSITION);
		DestroyMenu(devicesmenu);
	}
	if (playlistsmenu)
	{
		RemoveMenu(mlview_menu, 2, MF_BYPOSITION);
		DestroyMenu(playlistsmenu);
	}*/
	if (mediamenu)
	{
		//RemoveMenu(mlview_menu, 1, MF_BYPOSITION);
		DestroyMenu(mediamenu);
	}
	return 1;
}

int V5_ML_Help_Menu(HWND hwnd, int x, int y, int width, int height)
{
	return Help_Menu(hwnd, x, y, width, height);
}

int V5_PE_ListOfPlaylists_Menu(int x, int y)
{
	HMENU viewmenu = NULL;
	mlGetTreeStruct mgts = { TREE_PLAYLISTS, 55000, -1 };
	HWND mlwnd = (HWND)sendMlIpc(0, 0);
	HMENU playlistsmenu = (HMENU)SendMessageW(mlwnd, WM_ML_IPC, (WPARAM) & mgts, ML_IPC_GETTREE);
	if (playlistsmenu)
	{
		InsertMenuW(playlistsmenu, 0, MF_BYPOSITION | MF_STRING, ID_MANAGEPLAYLISTS, getStringW(IDS_ML_MANAGE_PLAYLISTS,NULL,0));
		{
			mlGetTreeStruct mgts = { TREE_QUERIES, 55000, -1 };
			viewmenu = (HMENU)SendMessageW(mlwnd, WM_ML_IPC, (WPARAM) &mgts, ML_IPC_GETTREE);
		}
		if (viewmenu && GetMenuItemCount(viewmenu) > 0)
			InsertMenuW(playlistsmenu, 1, MF_BYPOSITION | MF_ENABLED | MF_POPUP, (UINT_PTR)viewmenu, getStringW(IDS_ML_SMART_VIEW_RESULTS,NULL,0));
		InsertMenu(playlistsmenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		g_open_ml_item_in_pe = 1;
		DoTrackPopup(playlistsmenu, TPM_LEFTALIGN, x, y, hMainWindow);
	}
	DestroyMenu(playlistsmenu);
	DestroyMenu(viewmenu);
	return 1;
}

void getViewportFromPoint(POINT *pt, RECT *r)
{
	if (!r || !pt) return ;
	{

		HMONITOR hm;
		hm = MonitorFromPoint(*pt, MONITOR_DEFAULTTONULL);
		if (hm)
		{
			MONITORINFOEXW mi;
			memset(&mi, 0, sizeof(mi));
			mi.cbSize = sizeof(mi);

			if (GetMonitorInfo(hm, &mi))
			{
				*r = mi.rcMonitor;
				return ;
			}
		}
	}
}

#undef GetSystemMetrics

void ensureInScreen(HMENU menu, int *x, int *y, int *flag, int width, int height)
{
	POINT pt = {*x, *y};
	int nitems = GetMenuItemCount(menu);
	int i;
	RECT mwr;
	RECT monitor;
	int rightdone = 0;
	int bottomdone = 0;
	int xedge = GetSystemMetrics(SM_CXEDGE);
	int yedge = GetSystemMetrics(SM_CYEDGE);
	int itemheight = GetSystemMetrics(SM_CYMENU);
	int checkmarkwidth = GetSystemMetrics(SM_CXMENUCHECK);
	int cury = *y + yedge + 1;
	GetWindowRect(hMainWindow, &mwr);
	getViewportFromPoint(&pt, &monitor);
	/*if (nitems*GetSystemMetrics(SM_CYMENU)+yedge*2+*y > monitor.bottom) {
	  bottomdone = 1;
	  *y -= height;
	  *flag &= ~TPM_TOPALIGN;
	  *flag |= TPM_BOTTOMALIGN;
	}*/
	for (i = 0;i < nitems;i++)
	{
		SIZE s={0};
		RECT item;
		MENUITEMINFOW info = {sizeof(info), MIIM_DATA | MIIM_TYPE | MIIM_STATE | MIIM_ID, MFT_STRING, };
		GetMenuItemRect(hMainWindow, menu, i, &item);
		item.left -= mwr.left;
		item.top -= mwr.top;
		item.right -= mwr.left;
		item.bottom -= mwr.top;
		if (item.top == 0 && item.left == 0)
		{
			// item has never been shown so MS wont give us the rect, I HATE THEM ! I HATE THEM SO MUCH ARRRG !

			// y
			item.top = cury;
			{
				GetMenuItemInfoW(menu, i, TRUE, &info);
				if (info.fType & MFT_SEPARATOR)
					cury += (itemheight - 1) >> 1;
				else
				{
					cury += itemheight - 2;
					//info.dwTypeData = (LPTSTR) MALLOC(++info.cch + 1);
					GetMenuItemInfoW(menu, i, TRUE, &info);
					//info.dwTypeData[info.cch] = 0;
				}
			}
			item.bottom = cury;

			// x
			if (info.dwTypeData)
			{
				LOGFONT m_lf;
				HFONT font = nullptr;
				HDC dc = nullptr;
				NONCLIENTMETRICS nm = {sizeof (NONCLIENTMETRICS), };
				memset((PVOID) &m_lf, 0, sizeof (LOGFONT));
				SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, nm.cbSize, &nm, 0);
				m_lf = nm.lfMenuFont;
				font = CreateFontIndirectW(&m_lf);
				dc = GetDC(hMainWindow);
				GetTextExtentPoint32(dc, info.dwTypeData, lstrlen(info.dwTypeData), &s);
				ReleaseDC(hMainWindow, dc);
				DeleteObject(font);
				//free(info.dwTypeData);
			}
			if (!(info.fType & MFT_SEPARATOR))
			{
				item.left = *x + xedge + 1;
				item.right = item.left + s.cx + checkmarkwidth * 2 + xedge + 1;
			}
			else
			{
				item.left = *x + xedge + 1;
				item.right = item.left + xedge + 1;
			}
		}
		else
		{
			item.left += *x;
			item.top += *y;
			item.right += *x;
			item.bottom += *y;
			item.right += xedge * 2 + 2; // to avoid last test
			cury = item.bottom;
		}
		if (!bottomdone && cury > monitor.bottom)
		{
			bottomdone = 1;
			*y -= height;
			*flag &= ~TPM_TOPALIGN;
			*flag |= TPM_BOTTOMALIGN;
		}
		if (!rightdone && item.right > monitor.right)
		{
			rightdone = 1;
			*x += width;
			*flag &= ~TPM_LEFTALIGN;
			*flag |= TPM_RIGHTALIGN;
		}
		if (rightdone && bottomdone) return ;
	}
	cury += yedge + 1;
	if (!bottomdone && cury > monitor.bottom)
	{
		//bottomdone = 1;
		*y -= height;
		*flag &= ~TPM_TOPALIGN;
		*flag |= TPM_BOTTOMALIGN;
	}
}


