#include "main.h"
#include "../nu/AutoCharFn.h"
#include "../nu/DialogSkinner.h"
#include "Bookmark.h"
#include "../../General/gen_ml/ml_ipc.h"
#include <string>
#include "../nu/ListView.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "../nu/menushortcuts.h"
#include <strsafe.h>
#include "../../General/gen_ml/menu.h"

INT_PTR CALLBACK view_bmDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static W_ListView m_bmlist;
static HWND m_headerhwnd, m_hwnd;
extern C_Config *g_config;
static int customAllowed, groupBtn = 1, enqueuedef;
static viewButtons view;

// used for the send-to menu bits
static INT_PTR IPC_LIBRARY_SENDTOMENU;
static librarySendToMenuStruct s;
BOOL myMenu = FALSE;

extern HMENU g_context_menus, g_context_menus2;
extern HCURSOR hDragNDropCursor;

static int  bookmark_contextMenu( INT_PTR param1, HWND parent, POINTS pts );
static void bookmarks_contextMenu( HWND hwndDlg, HWND from, int x, int y );
static void bookmark_onTreeEnterDblClk();

static int pluginHandleIpcMessage(int msg, int param) 
{
	return (int)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, param, msg);
}

void bookmark_notifyAdd(wchar_t *filenametitle)
{
	if (!m_hwnd || !filenametitle) return;
	int cnt=m_bmlist.GetCount();
	m_bmlist.InsertItem(cnt,filenametitle+lstrlenW(filenametitle)+1,0);
	m_bmlist.SetItemText(cnt,1,filenametitle);
}

INT_PTR bm_pluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	if (message_type == ML_MSG_NO_CONFIG)
	{
		return TRUE;
	}
	else if (message_type == ML_MSG_TREE_ONCLICK)
	{
		switch(param2)
		{
			case ML_ACTION_ENTER:
			case ML_ACTION_DBLCLICK:
				if (param1 == bookmark_treeItem)
					bookmark_onTreeEnterDblClk();
				break;
		}
	}
	else if (message_type == ML_MSG_TREE_ONCREATEVIEW && param1 == bookmark_treeItem)
	{
		return (INT_PTR)WASABI_API_CREATEDIALOGW(IDD_VIEW_BM, (HWND)param2, view_bmDialogProc);
	}
	else if (message_type == ML_MSG_NAVIGATION_CONTEXTMENU)
	{
		return bookmark_contextMenu(param1, (HWND)param2, MAKEPOINTS(param3));
	}
	else if (message_type == ML_MSG_ONSENDTOBUILD)
	{
		if (!myMenu &&
			(param1 == ML_TYPE_ITEMRECORDLISTW || param1 == ML_TYPE_ITEMRECORDLIST ||
			param1 == ML_TYPE_FILENAMES || param1 == ML_TYPE_STREAMNAMES ||
			param1 == ML_TYPE_FILENAMESW || param1 == ML_TYPE_STREAMNAMESW ||
			param1 == ML_TYPE_CDTRACKS))
			mediaLibrary.AddToSendTo(WASABI_API_LNGSTRINGW(IDS_ADD_TO_BOOKMARKS),
									 param2, (INT_PTR)bm_pluginMessageProc);
	}
	else if (message_type == ML_MSG_ONSENDTOSELECT || message_type == ML_MSG_TREE_ONDROPTARGET)
	{
		// set with droptarget defaults =)
		UINT_PTR type = 0,data = 0;

		if (message_type == ML_MSG_ONSENDTOSELECT)
		{
			if (param3 != (INT_PTR)bm_pluginMessageProc) return 0;

			type=(int)param1;
			data = (int)param2;
		}
		else
		{
			if (param1 != bookmark_treeItem) return 0;

			type=(int)param2;
			data=(int)param3;

			if (!data)
			{
				return (type == ML_TYPE_ITEMRECORDLISTW || type == ML_TYPE_ITEMRECORDLIST ||
						type == ML_TYPE_FILENAMES || type == ML_TYPE_STREAMNAMES ||
						type == ML_TYPE_FILENAMESW || type == ML_TYPE_STREAMNAMESW ||
						type == ML_TYPE_CDTRACKS ||
						type == ML_TYPE_PLAYLIST || type == ML_TYPE_PLAYLISTS) ? 1 : -1;
			}
		}

		if (data)
		{
			if (type == ML_TYPE_ITEMRECORDLIST || type == ML_TYPE_CDTRACKS)
			{
				itemRecordList *p=(itemRecordList*)data;
				for (int x = 0; x < p->Size; x ++)
					mediaLibrary.AddBookmarkW(AutoWide(p->Items[x].filename));

				return 1;
			}
			else if (type == ML_TYPE_ITEMRECORDLISTW)
			{
				itemRecordListW *p=(itemRecordListW *)data;
				for (int x = 0; x < p->Size; x ++)
					mediaLibrary.AddBookmarkW(p->Items[x].filename);

				return 1;
			}
			else if (type == ML_TYPE_FILENAMES || type == ML_TYPE_STREAMNAMES)
			{
				char *p=(char*)data;
				while (p && *p)
				{
					mediaLibrary.AddBookmark(p);
					p+=lstrlenA(p)+1;
				}
				return 1;
			}
			else if (type == ML_TYPE_FILENAMESW || type == ML_TYPE_STREAMNAMESW)
			{
				wchar_t *p=(wchar_t*)data;
				while (p && *p)
				{
					mediaLibrary.AddBookmarkW(p);
					p+=wcslen(p)+1;
				}
				return 1;
			}
			else if(type == ML_TYPE_PLAYLIST)
			{
				mediaLibrary.AddBookmarkW((wchar_t*)((mlPlaylist*)data)->filename);
				return 1;
			}
			else if(type == ML_TYPE_PLAYLISTS)
			{
				mlPlaylist **playlists = (mlPlaylist **)data;
				while (playlists && *playlists)
				{
					mlPlaylist *pl = *playlists;
					mediaLibrary.AddBookmarkW((wchar_t*)pl->filename);
					playlists++;
				}
				return 1;
			}
		}
	}
	else if (message_type == ML_MSG_VIEW_PLAY_ENQUEUE_CHANGE)
	{
		enqueuedef = (int)param1;
		groupBtn = (int)param2;
		PostMessage(m_hwnd, WM_APP + 104, param1, param2);
		return 0;
	}
	return 0;
}

static void playFiles(int enqueue, int all) 
{
	int cnt=0;
	int l=m_bmlist.GetCount();
	for(int i=0;i<l;i++) 
	{
		if(all || m_bmlist.GetSelected(i)) 
		{
			if (!cnt)
			{
				if(!enqueue) SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_DELETE);
				cnt++;
			}
			//send the file to winamp
			COPYDATASTRUCT cds = {0};
			cds.dwData = IPC_ENQUEUEFILEW;
			wchar_t buf[1024] = {0};
			m_bmlist.GetText(i,1,buf,sizeof(buf)-1);
			buf[1023]=0;
			cds.lpData = (void *) buf;
			cds.cbData = (lstrlenW((wchar_t*)cds.lpData)+1)*sizeof(wchar_t); // include space for null char
			SendMessage(plugin.hwndWinampParent,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
		}
	}
	if (cnt)
	{
		if(!enqueue) SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_STARTPLAY);
	}
}

static wchar_t *g_bmedit_fn, *g_bmedit_ft;

static BOOL CALLBACK BookMarkEditProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SetDlgItemTextW(hwndDlg,IDC_TITLE,g_bmedit_ft);
			SetDlgItemTextW(hwndDlg,IDC_FILE,g_bmedit_fn);
			return 0;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					GetDlgItemTextW(hwndDlg,IDC_TITLE,g_bmedit_ft,1024);
					GetDlgItemTextW(hwndDlg,IDC_FILE,g_bmedit_fn,1024);
				case IDCANCEL:
					EndDialog(hwndDlg,0);
					return 0;
				case IDC_EDIT_FN:
				{
					wchar_t fn[1024] = {0};
					GetDlgItemTextW(hwndDlg,IDC_FILE,fn,1024);
					OPENFILENAMEW of = {0};
					of.lStructSize = sizeof(OPENFILENAMEW);
					of.hwndOwner = hwndDlg;
					of.nMaxFileTitle = 32;
					of.lpstrFilter = (wchar_t*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,1,IPC_GET_EXTLISTW);
					of.nMaxCustFilter = 1024;
					of.lpstrFile = fn;
					of.nMaxFile = 1024;
					of.lpstrTitle = WASABI_API_LNGSTRINGW(IDS_BROWSE_FOR_BM_ENTRY);
					of.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_EXPLORER|
							   OFN_PATHMUSTEXIST|OFN_ENABLESIZING;

					if(GetOpenFileNameW(&of))
					{
						SetDlgItemTextW(hwndDlg,IDC_FILE,fn);
					}
					GlobalFree((void*)of.lpstrFilter);
				}
				break;
			}
			return 0;
	}
	return 0;
}

static void readbookmarks(int play1enqueue2=0)
{
	if (!play1enqueue2) m_bmlist.Clear();

	int x=0;
	FILE *fp=NULL;
	wchar_t *fnp=(wchar_t*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,666,IPC_ADDBOOKMARKW);
	if ((unsigned int)fnp < 65536) return;

	fp=_wfopen(fnp,L"rt");
	if (fp)
	{
		while (1)
		{
			char ft[4096] = {0}, fn[4096] = {0};
			fgets(fn,4096,fp);
			if (feof(fp)) break;
			fgets(ft,4096,fp);
			if (feof(fp)) break;
			if (ft[0] && fn[0])
			{
				if (fn[strlen(fn)-1]=='\n') fn[strlen(fn)-1]=0;
				if (ft[strlen(ft)-1]=='\n') ft[strlen(ft)-1]=0;
				if (ft[0] && fn[0])
				{
					if (!play1enqueue2)
					{
						m_bmlist.InsertItem(x,AutoWide(ft,CP_UTF8),0);
						m_bmlist.SetItemText(x,1,AutoWide(fn,CP_UTF8));
					}
					else
					{
						if (!x)
						{
							if(play1enqueue2==1) SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_DELETE);
						}
						//send the file to winamp
						COPYDATASTRUCT cds = {0};
						cds.dwData = IPC_ENQUEUEFILEW;
						cds.lpData = (void *)AutoWideDup(fn,CP_UTF8);
						cds.cbData = (lstrlenW((wchar_t *) cds.lpData)+1)*sizeof(wchar_t); // include space for null char
						SendMessage(plugin.hwndWinampParent,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
					}
					x++;
				}
			}
		}
		fclose(fp);
	}

	if (!play1enqueue2 && AGAVE_API_STATS)
	{
		AGAVE_API_STATS->SetStat(api_stats::BOOKMARK_COUNT, m_bmlist.GetCount());
	}

	if (x && play1enqueue2 == 1)
	{
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_STARTPLAY);
	}
}

static void writebookmarks()
{
	wchar_t *fnp = (wchar_t *)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 666, IPC_ADDBOOKMARKW );
	
	if ( (unsigned int)fnp < 65536 )
		return;
	
	BookmarkWriter bookmarks;
	bookmarks.New( fnp );
	
	int l = m_bmlist.GetCount();
	for ( int x = 0; x < l; x++ )
	{
		wchar_t ftW[ 4096 ] = { 0 }, fnW[ 4096 ] = { 0 };
		m_bmlist.GetText( x, 0, ftW, ARRAYSIZE( ftW ) );
		m_bmlist.GetText( x, 1, fnW, ARRAYSIZE( fnW ) );
		bookmarks.Write( AutoChar( fnW, CP_UTF8 ), AutoChar( ftW, CP_UTF8 ) );
	}
	
	bookmarks.Close();

	fnp = (wchar_t *)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_ADDBOOKMARKW );
	
	if ( (unsigned int)fnp < 65536 )
		return;
	
	bookmarks.New( fnp );
	
	for ( int x = 0; x < l; x++ )
	{
		char ft[ 4096 ] = { 0 }, fn[ 4096 ] = { 0 };
		
		m_bmlist.GetText( x, 0, ft, sizeof( ft ) );
		m_bmlist.GetText( x, 1, fn, sizeof( fn ) );
		
		bookmarks.Write( fn, ft );
	}
	
	bookmarks.Close();
}

static void bookmark_onTreeEnterDblClk()
{
	int enq = ( ( !!( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ) ^ ( !!g_config->ReadInt( L"enqueuedef", 0 ) ) );
	readbookmarks( enq ? 2 : 1 );
}

void SyncMenuWithAccelerators(HWND hwndDlg, HMENU menu)
{
	HACCEL szAccel[24] = {0};
	INT c = WASABI_API_APP->app_getAccelerators(hwndDlg, szAccel, sizeof(szAccel)/sizeof(szAccel[0]), FALSE);
	AppendMenuShortcuts(menu, szAccel, c, MSF_REPLACE);
}

void SwapPlayEnqueueInMenu(HMENU listMenu)
{
	int playPos=-1, enqueuePos=-1;
	MENUITEMINFOW playItem={sizeof(MENUITEMINFOW), 0,}, enqueueItem={sizeof(MENUITEMINFOW), 0,};

	int numItems = GetMenuItemCount(listMenu);

	for (int i=0;i<numItems;i++)
	{
		UINT id = GetMenuItemID(listMenu, i);
		if (id == ID_BMWND_PLAYSELECTEDFILES)
		{
			playItem.fMask = MIIM_ID;
			playPos = i;
			GetMenuItemInfoW(listMenu, i, TRUE, &playItem);
		}
		else if (id == ID_BMWND_ENQUEUESELECTEDFILES)
		{
			enqueueItem.fMask = MIIM_ID;
			enqueuePos= i;
			GetMenuItemInfoW(listMenu, i, TRUE, &enqueueItem);
		}
	}
	
	playItem.wID = ID_BMWND_ENQUEUESELECTEDFILES;
	enqueueItem.wID = ID_BMWND_PLAYSELECTEDFILES;		
	SetMenuItemInfoW(listMenu, playPos, TRUE, &playItem);
	SetMenuItemInfoW(listMenu, enqueuePos, TRUE, &enqueueItem);
}

void UpdateMenuItems(HWND hwndDlg, HMENU menu)
{
	bool swapPlayEnqueue=false;
	if (g_config->ReadInt(L"enqueuedef", 0) == 1)
	{
		SwapPlayEnqueueInMenu(menu);
		swapPlayEnqueue=true;
	}
	
	SyncMenuWithAccelerators(hwndDlg, menu);
	if (swapPlayEnqueue) SwapPlayEnqueueInMenu(menu);
}

static int bookmark_contextMenu(INT_PTR param1, HWND hHost, POINTS pts)
{
	HNAVITEM hItem = (HNAVITEM)param1;
	HNAVITEM myItem = MLNavCtrl_FindItemById(plugin.hwndLibraryParent, bookmark_treeItem);
	if (hItem != myItem) 
		return FALSE;

	POINT pt;
	POINTSTOPOINT(pt, pts);
	if (-1 == pt.x || -1 == pt.y)
	{
		NAVITEMGETRECT itemRect;
		itemRect.fItem = FALSE;
		itemRect.hItem = hItem;
		if (MLNavItem_GetRect(plugin.hwndLibraryParent, &itemRect))
		{
			MapWindowPoints(hHost, HWND_DESKTOP, (POINT*)&itemRect.rc, 2);
			pt.x = itemRect.rc.left + 2;
			pt.y = itemRect.rc.top + 2;
		}
	}

	HMENU menu=GetSubMenu(g_context_menus,1);

	bool swapPlayEnqueue=false;
	if (g_config->ReadInt(L"enqueuedef", 0) == 1)
	{
		SwapPlayEnqueueInMenu(menu);
		swapPlayEnqueue=true;
	}

	if(!IsWindow(m_hwnd))
	{
		HACCEL accel = WASABI_API_LOADACCELERATORSW(IDR_VIEW_BM_ACCELERATORS);
		int size = CopyAcceleratorTable(accel,0,0);
		AppendMenuShortcuts(menu, &accel, size, MSF_REPLACE);
	}
	else
	{
		SyncMenuWithAccelerators(m_hwnd, menu);
	}

	if (swapPlayEnqueue)
		SwapPlayEnqueueInMenu(menu);

	int r = Menu_TrackPopup(plugin.hwndLibraryParent, menu,
							TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_NONOTIFY,
							pt.x, pt.y, hHost, NULL);
	switch(r) 
	{
		case ID_BMWND_PLAYSELECTEDFILES: 
			readbookmarks(1);
			break;
		case ID_BMWND_ENQUEUESELECTEDFILES:  
			readbookmarks(2);
			break;
		case ID_BMWND_HELP:
			SENDWAIPC(plugin.hwndWinampParent, IPC_OPEN_URL, L"https://help.winamp.com/hc/articles/8105304048660-The-Winamp-Media-Library");
			break;
	}
	Sleep(100);
	MSG msg;
	while(PeekMessage(&msg,NULL,WM_KEYFIRST,WM_KEYLAST,PM_REMOVE)); //eat return
	return TRUE;
}

static void bookmarks_contextMenu(HWND hwndDlg, HWND from, int x, int y)
{
	if (from != m_bmlist.getwnd())
		return ;

	POINT pt = {x,y};

	if (x == -1 || y == -1) // x and y are -1 if the user invoked a shift-f10 popup menu
	{
		RECT itemRect = {0};
		int selected = m_bmlist.GetNextSelected();
		if (selected != -1) // if something is selected we'll drop the menu from there
		{
			m_bmlist.GetItemRect(selected, &itemRect);
			ClientToScreen(hwndDlg, (POINT *)&itemRect);
		}
		else // otherwise we'll drop it from the top-left corner of the listview, adjusting for the header location
		{
			GetWindowRect(hwndDlg, &itemRect);

			HWND hHeader = (HWND)SNDMSG(from, LVM_GETHEADER, 0, 0L);
			RECT headerRect;
			if ((WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) && GetWindowRect(hHeader, &headerRect))
			{
				itemRect.top += (headerRect.bottom - headerRect.top);
			}
		}
		x = itemRect.left;
		y = itemRect.top;
	}

	HWND hHeader = (HWND)SNDMSG(from, LVM_GETHEADER, 0, 0L);
	RECT headerRect;
	if (0 == (WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)) || FALSE == GetWindowRect(hHeader, &headerRect))
	{
		SetRectEmpty(&headerRect);
	}

	if (FALSE != PtInRect(&headerRect, pt))
	{
		return; 
	}

	HMENU menu=GetSubMenu(g_context_menus,0);
	UpdateMenuItems(hwndDlg, menu);
	ZeroMemory(&s, sizeof(librarySendToMenuStruct));
				
	IPC_LIBRARY_SENDTOMENU = (INT_PTR)SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)&"LibrarySendToMenu",IPC_REGISTER_WINAMP_IPCMESSAGE);
	if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)0,IPC_LIBRARY_SENDTOMENU)==0xffffffff)
	{
		s.mode = 1;
		s.hwnd = hwndDlg;
		s.data_type = ML_TYPE_FILENAMESW;
		s.ctx[1] = 1;
		s.build_hMenu = CreatePopupMenu();

		MENUITEMINFOW mii = {0};
		mii.cbSize = sizeof(MENUITEMINFOW);
		mii.fMask = MIIM_SUBMENU;
		mii.hSubMenu = s.build_hMenu;
		SetMenuItemInfoW(menu, 2, TRUE, &mii);
	}

	UINT menuStatus;
	if (m_bmlist.GetNextSelected(-1) == -1)
	{
		menuStatus = MF_BYCOMMAND | MF_GRAYED;
		EnableMenuItem(menu, 2, MF_BYPOSITION | MF_GRAYED);
	}
	else
	{
		menuStatus = MF_BYCOMMAND | MF_ENABLED;
		EnableMenuItem(menu, 2, MF_BYPOSITION | MF_ENABLED);
	}

	EnableMenuItem(menu, ID_BMWND_PLAYSELECTEDFILES, menuStatus);
	EnableMenuItem(menu, ID_BMWND_ENQUEUESELECTEDFILES, menuStatus);
	EnableMenuItem(menu, ID_BMWND_REMOVESELECTEDBOOKMARKS, menuStatus);
	EnableMenuItem(menu, ID_BMWND_EDITSELECTEDBOOKMARKS, menuStatus);

	int r = Menu_TrackPopup(plugin.hwndLibraryParent, menu,
							TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
							x, y, hwndDlg, NULL);
	switch(LOWORD(r))
	{
		case ID_BMWND_PLAYSELECTEDFILES: 
		case ID_BMWND_ENQUEUESELECTEDFILES:
		case ID_BMWND_REMOVESELECTEDBOOKMARKS:
		case ID_BMWND_EDITSELECTEDBOOKMARKS:
		case ID_BMWND_SELECTALL:
			SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(r,0),0);
			break;
		default:
			if (s.mode == 2)
			{
				s.menu_id = r;
				if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
				{
					s.mode=3;
					s.data_type=ML_TYPE_FILENAMESW;
			
					//std::vector<wchar_t> sendStr;
					std::wstring sendStr;

					int l=m_bmlist.GetCount();
					for(int i=0;i<l;i++) 
					{
						if (m_bmlist.GetSelected(i)) 
						{
							wchar_t buf[1023] = {0};
							m_bmlist.GetText(i,1,buf,ARRAYSIZE(buf)-1);
							// HAKAN: why (len + 1) ?
							//sendStr.append(buf, wcslen(buf)+1);
							sendStr.append(buf, wcslen(buf));
						}
					}
					// HAKAN: No need to add trailing zero
					//sendStr.push_back(0);
					
					s.data = (void*)sendStr.c_str();
			
					if(SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU)!=1)
					{
						s.mode=3;
						s.data_type=ML_TYPE_FILENAMES;
				
						//std::vector<char> sendStrA;
						std::string sendStrA;

						int l=m_bmlist.GetCount();
						for(int i=0;i<l;i++) 
						{
							if (m_bmlist.GetSelected(i)) 
							{
								wchar_t buf[1023] = {0};
								m_bmlist.GetText(i,1,buf,ARRAYSIZE(buf)-1);

								// HAKAN: why (len + 1) ?
								//sendStrA.append(AutoCharFn(buf), strlen(AutoCharFn(buf)) + 1);
								sendStrA.append(AutoCharFn(buf), strlen(AutoCharFn(buf)));
							}
						}
						// HAKAN: No need to add trailing zero
						//sendStrA.push_back(0);
						
						s.data = (void*)sendStrA.c_str();
				
						SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU);
					}
				}
			}
			break;
	}

	if (s.mode) 
	{
		s.mode=4;
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU); // cleanup
	}
	
	if (NULL != s.build_hMenu)
	{
		DestroyMenu(s.build_hMenu);
		s.build_hMenu = NULL;
	}

	Sleep(100);
	MSG msg;
	while(PeekMessage(&msg,NULL,WM_KEYFIRST,WM_KEYLAST,PM_REMOVE)); //eat return
}

static HRGN g_rgnUpdate = NULL;
static int offsetX = 0, offsetY = 0;

typedef struct _LAYOUT
{
	INT		id;
	HWND		hwnd;
	INT		x;
	INT		y;
	INT		cx;
	INT		cy;
	DWORD	flags;
	HRGN	rgn;
}
LAYOUT, PLAYOUT;

#define SETLAYOUTPOS(_layout, _x, _y, _cx, _cy) { _layout->x=_x; _layout->y=_y;_layout->cx=_cx;_layout->cy=_cy;_layout->rgn=NULL; }
#define SETLAYOUTFLAGS(_layout, _r)																						\
	{																													\
		BOOL fVis;																										\
		fVis = (WS_VISIBLE & (LONG)GetWindowLongPtr(_layout->hwnd, GWL_STYLE));											\
		if (_layout->x == _r.left && _layout->y == _r.top) _layout->flags |= SWP_NOMOVE;									\
		if (_layout->cx == (_r.right - _r.left) && _layout->cy == (_r.bottom - _r.top)) _layout->flags |= SWP_NOSIZE;	\
		if ((SWP_HIDEWINDOW & _layout->flags) && !fVis) _layout->flags &= ~SWP_HIDEWINDOW;								\
		if ((SWP_SHOWWINDOW & _layout->flags) && fVis) _layout->flags &= ~SWP_SHOWWINDOW;									\
	}

#define LAYOUTNEEEDUPDATE(_layout) ((SWP_NOMOVE | SWP_NOSIZE) != ((SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW | SWP_SHOWWINDOW) & _layout->flags))

#define GROUP_MIN			0x1
#define GROUP_MAX			0x2
#define GROUP_STATUSBAR		0x1
#define GROUP_MAIN			0x2

static void LayoutWindows(HWND hwnd, BOOL fRedraw, BOOL fUpdateAll = FALSE)
{
	static INT controls[] =
	{
		GROUP_STATUSBAR, IDC_BUTTON_PLAY, IDC_BUTTON_ENQUEUE, IDC_BUTTON_CUSTOM, IDC_EDITBOOK, IDC_REMOVEBOOK,
		GROUP_MAIN, IDC_LIST
	};

	INT    index;
	RECT   rc;
	RECT   rg;
	RECT   ri;
	LAYOUT layout[sizeof(controls)/sizeof(controls[0])], *pl;
	BOOL   skipgroup;
	HRGN   rgn = NULL;

	GetClientRect(hwnd, &rc);
	if ( rc.right == rc.left || rc.bottom == rc.top )
		return;
	
	if ( rc.right > WASABI_API_APP->getScaleX( 4 ) )
		rc.right -= WASABI_API_APP->getScaleX( 4 );
	
	SetRect(&rg, rc.left, rc.top, rc.right, rc.top);

	pl        = layout;
	skipgroup = FALSE;

	InvalidateRect( hwnd, NULL, TRUE );

	for (index = 0; index < sizeof(controls) / sizeof(*controls); index++)
	{
		if ( controls[ index ] >= GROUP_MIN && controls[ index ] <= GROUP_MAX ) // group id
		{
			skipgroup = FALSE;
			switch ( controls[ index ] )
			{
				case GROUP_STATUSBAR:
				{
					wchar_t buffer[ 128 ] = { 0 };
					WASABI_API_LNGSTRINGW_BUF( IDC_BUTTON_PLAY, buffer, ARRAYSIZE( buffer ) );
					LRESULT idealSize = MLSkinnedButton_GetIdealSize( GetDlgItem( hwnd, IDC_BUTTON_PLAY ), buffer );

					SetRect( &rg, rc.left + WASABI_API_APP->getScaleX( 1 ),
							 rc.bottom - WASABI_API_APP->getScaleY( HIWORD( idealSize ) ), rc.right, rc.bottom );
					rc.bottom = rg.top - WASABI_API_APP->getScaleY( 3 );
					break;
				}
				case GROUP_MAIN:
					SetRect( &rg, rc.left + WASABI_API_APP->getScaleX( 1 ), rc.top, rc.right, rc.bottom );
					break;
			}
			continue;
		}
		
		if (skipgroup)
			continue;

		pl->id   = controls[ index ];
		pl->hwnd = GetDlgItem( hwnd, pl->id );
		if (!pl->hwnd)
			continue;

		GetWindowRect(pl->hwnd, &ri);
		MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&ri, 2);
		pl->flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS;

		switch ( pl->id )
		{
			case IDC_BUTTON_PLAY:
			case IDC_BUTTON_ENQUEUE:
			case IDC_BUTTON_CUSTOM:
			case IDC_EDITBOOK:
			case IDC_REMOVEBOOK:
				if ( IDC_BUTTON_CUSTOM != pl->id || customAllowed )
				{
					if ( groupBtn && pl->id == IDC_BUTTON_PLAY && enqueuedef == 1 )
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}

					if ( groupBtn && pl->id == IDC_BUTTON_ENQUEUE && enqueuedef != 1 )
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}

					if ( groupBtn && ( pl->id == IDC_BUTTON_PLAY || pl->id == IDC_BUTTON_ENQUEUE ) && customAllowed )
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}

					wchar_t buffer[ 128 ] = { 0 };
					GetWindowTextW( pl->hwnd, buffer, ARRAYSIZE( buffer ) );
					LRESULT idealSize = MLSkinnedButton_GetIdealSize( pl->hwnd, buffer );
					LONG width = LOWORD( idealSize ) + WASABI_API_APP->getScaleX( 6 );
					SETLAYOUTPOS( pl, rg.left, rg.bottom - WASABI_API_APP->getScaleY( HIWORD( idealSize ) ), width, WASABI_API_APP->getScaleY( HIWORD( idealSize ) ) );
					pl->flags |= ( ( rg.right - rg.left ) > width ) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
					if ( SWP_SHOWWINDOW & pl->flags ) rg.left += ( pl->cx + WASABI_API_APP->getScaleX( 4 ) );
				}
				else
					pl->flags |= SWP_HIDEWINDOW;
				break;
			case IDC_LIST:
				pl->flags |= ( rg.top < rg.bottom ) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				SETLAYOUTPOS( pl, rg.left, rg.top + WASABI_API_APP->getScaleY( 1 ),
							  rg.right - rg.left + WASABI_API_APP->getScaleY( 1 ),
							  ( rg.bottom - rg.top ) - WASABI_API_APP->getScaleY( 2 ) );
				break;
		}

		SETLAYOUTFLAGS(pl, ri);
		if ( LAYOUTNEEEDUPDATE( pl ) )
		{
			if ( SWP_NOSIZE == ( ( SWP_HIDEWINDOW | SWP_SHOWWINDOW | SWP_NOSIZE ) & pl->flags ) &&
				 ri.left == ( pl->x + offsetX ) && ri.top == ( pl->y + offsetY ) && IsWindowVisible( pl->hwnd ) )
			{
				SetRect( &ri, pl->x, pl->y, pl->cx + pl->x, pl->y + pl->cy );
				ValidateRect( hwnd, &ri );
			}
			pl++;
		}
		else if ( ( fRedraw || ( !offsetX && !offsetY ) ) && IsWindowVisible( pl->hwnd ) )
		{
			ValidateRect( hwnd, &ri );
			if ( GetUpdateRect( pl->hwnd, NULL, FALSE ) )
			{
				if ( !rgn ) rgn = CreateRectRgn( 0, 0, 0, 0 );
				GetUpdateRgn( pl->hwnd, rgn, FALSE );
				OffsetRgn( rgn, pl->x, pl->y );
				InvalidateRgn( hwnd, rgn, FALSE );
			}
		}
	}

	if ( pl != layout )
	{
		LAYOUT *pc;
		HDWP hdwp = BeginDeferWindowPos( (INT)( pl - layout ) );
		for ( pc = layout; pc < pl && hdwp; pc++ )
		{
			hdwp = DeferWindowPos( hdwp, pc->hwnd, NULL, pc->x, pc->y, pc->cx, pc->cy, pc->flags );
		}

		if ( hdwp )
			EndDeferWindowPos( hdwp );

		if ( !rgn )
			rgn = CreateRectRgn( 0, 0, 0, 0 );

		if ( fRedraw )
		{
			GetUpdateRgn( hwnd, rgn, FALSE );
			for ( pc = layout; pc < pl && hdwp; pc++ )
			{
				if ( pc->rgn )
				{
					OffsetRgn( pc->rgn, pc->x, pc->y );
					CombineRgn( rgn, rgn, pc->rgn, RGN_OR );
				}
			}

			RedrawWindow( hwnd, NULL, rgn, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN );
		}

		if ( g_rgnUpdate )
		{
			GetUpdateRgn( hwnd, g_rgnUpdate, FALSE );
			for ( pc = layout; pc < pl && hdwp; pc++ )
			{
				if ( pc->rgn )
				{
					OffsetRgn( pc->rgn, pc->x, pc->y );
					CombineRgn( g_rgnUpdate, g_rgnUpdate, pc->rgn, RGN_OR );
				}
			}
		}

		for ( pc = layout; pc < pl && hdwp; pc++ )
			if ( pc->rgn )
				DeleteObject( pc->rgn );
	}

	if ( rgn )
		DeleteObject( rgn );
	
	ValidateRgn( hwnd, NULL );
}

static BOOL Bookmark_OnDisplayChange()
{
	ListView_SetTextColor(m_bmlist.getwnd(),dialogSkinner.Color(WADLG_ITEMFG));
	ListView_SetBkColor(m_bmlist.getwnd(),dialogSkinner.Color(WADLG_ITEMBG));
	ListView_SetTextBkColor(m_bmlist.getwnd(),dialogSkinner.Color(WADLG_ITEMBG));
	m_bmlist.SetFont(dialogSkinner.GetFont());
	LayoutWindows(m_hwnd, TRUE);
	return 0;
}

static BOOL Bookmark_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	if (GetCapture()==hwnd)
	{
		POINT p={x,y};
		ClientToScreen(hwnd,&p);

		if (WindowFromPoint(p) == m_bmlist.getwnd())
		{
			SetCursor(hDragNDropCursor);
		}
		else
		{
			mlDropItemStruct m={0};
			m.type=ML_TYPE_FILENAMES;
			m.p=p;
			pluginHandleIpcMessage(ML_IPC_HANDLEDRAG,(int)&m);
		}
	}
	return FALSE;
}

static BOOL Bookmark_OnLButtonUp(HWND hwnd, int x, int y, UINT flags)
{
	if (GetCapture()==hwnd)
	{
		ReleaseCapture();
		POINT p={x,y};
		ClientToScreen(hwnd,&p);

		if (WindowFromPoint(p) == m_bmlist.getwnd())
		{
			LVHITTESTINFO lvi;
			lvi.pt=p;
			ScreenToClient(m_bmlist.getwnd(),&lvi.pt);
			ListView_HitTest(m_bmlist.getwnd(),&lvi);

			int num_items = m_bmlist.GetCount();

			int destination_position=lvi.iItem;
			if ((lvi.flags & (LVHT_ONITEM)));
			else if (lvi.flags & LVHT_ABOVE) destination_position=0;
			else if (lvi.flags & (LVHT_BELOW|LVHT_NOWHERE )) destination_position=num_items;
			else return 0;

			int x = 0, dirty=0;

			for (; x < num_items; x ++)
			{
				m_bmlist.SetItemParam(x,0);
			}

			for (x = 0; x < num_items; x ++)
			{
				int sel=m_bmlist.GetSelected(x);
				if (sel && x != destination_position)
				{
					wchar_t ft[1024] = {0}, fn[1024] = {0};
					m_bmlist.GetText(x,0,ft,ARRAYSIZE(ft));
					m_bmlist.GetText(x,1,fn,ARRAYSIZE(fn));
					m_bmlist.DeleteItem(x);
					if (x < destination_position) 
					{
						x--;
					}

					if (destination_position >= num_items) destination_position--;

					m_bmlist.InsertItem(destination_position,ft,1);
					m_bmlist.SetItemText(destination_position,1,fn);
					// have to do this otherwise first item isn't correctly flagged & reselected
					m_bmlist.SetItemParam(destination_position,1);

					destination_position++;

					dirty=1;
				}
				else if (sel) destination_position++;
			}

			int w=0;
			for (x = 0; x < num_items; x ++)
			{
				if (m_bmlist.GetParam(x))
				{
					if (!w) 
					{
						w=1;
						ListView_SetItemState(m_bmlist.getwnd(),x,LVIS_FOCUSED,LVIS_FOCUSED);
					}
					m_bmlist.SetSelected(x);
				}
			}

			if (dirty) writebookmarks();
		}
		else
		{
			mlDropItemStruct m={0};
			m.type=ML_TYPE_FILENAMESW;
			m.p=p;
			m.flags=ML_HANDLEDRAG_FLAG_NOCURSOR;

			pluginHandleIpcMessage(ML_IPC_HANDLEDRAG,(int)&m);

			if (m.result>0)
			{
				wchar_t *buf=(wchar_t*)calloc(4096, sizeof(wchar_t));
				size_t buf_size=4096;
				size_t buf_pos=0;

				int l=m_bmlist.GetCount();
				for(int i=0;i<l;i++) 
				{
					if (m_bmlist.GetSelected(i)) 
					{
						wchar_t tbuf[1024]={0};
						m_bmlist.GetText(i,1,tbuf,ARRAYSIZE(tbuf)-1);
						tbuf[1023]=0;
						size_t newsize=buf_pos + wcslen(tbuf) + 1;
						if (newsize < buf_size)
						{
							size_t old_buf_size=buf_size;
							buf_size=newsize+4096;

							wchar_t *data = (wchar_t*)realloc(buf, (buf_size + 1) * sizeof(wchar_t));
							if (data)
							{
								buf=data;
							}
							else
							{
								data=(wchar_t*)calloc((buf_size + 1), sizeof(wchar_t));
								if (data)
								{
									memcpy(data, buf, sizeof(wchar_t)*old_buf_size);
									free(buf);
									buf=data;
								}
								else buf_size = old_buf_size;
							}
						}
						lstrcpynW(buf+buf_pos,tbuf,(int)buf_size);
						buf_pos=newsize;
					}
				}
				if (buf_pos)
				{
					buf[buf_pos]=0;
					m.flags=0;
					m.result=0;
					m.data=(void*)buf;
					pluginHandleIpcMessage(ML_IPC_HANDLEDROP,(int)&m);
				}
				free(buf);
			}      
		}
	}
	return FALSE;
}

void Bookmark_SelectAll(void)
{
	LVITEM item;
	item.state = LVIS_SELECTED;
	item.stateMask = LVIS_SELECTED;
	SendMessageW(m_bmlist.getwnd(), LVM_SETITEMSTATE, -1, (LPARAM)&item);
}

enum
{
	BPM_ECHO_WM_COMMAND = 0x1, // send WM_COMMAND and return value
	BPM_WM_COMMAND      = 0x2, // just send WM_COMMAND
};

BOOL Bookmark_ButtonPopupMenu( HWND hwndDlg, int buttonId, HMENU menu, int flags = 0 )
{
	RECT r;
	HWND buttonHWND = GetDlgItem( hwndDlg, buttonId );
	GetWindowRect( buttonHWND, &r );
	UpdateMenuItems( hwndDlg, menu );
	MLSkinnedButton_SetDropDownState( buttonHWND, TRUE );
	
	UINT tpmFlags = TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN;
	
	if ( !( flags & BPM_WM_COMMAND ) )
		tpmFlags |= TPM_RETURNCMD;
	
	int x = Menu_TrackPopup( plugin.hwndLibraryParent, menu, tpmFlags, r.left, r.top, hwndDlg, NULL );
	
	if ( ( flags & BPM_ECHO_WM_COMMAND ) && x )
		SendMessage( hwndDlg, WM_COMMAND, MAKEWPARAM( x, 0 ), 0 );
	
	MLSkinnedButton_SetDropDownState( buttonHWND, FALSE );
	
	return x;
}

static void Bookmark_Play(HWND hwndDlg, HWND from, UINT idFrom)
{
	HMENU listMenu = GetSubMenu(g_context_menus2, 0);
	int count = GetMenuItemCount(listMenu);
	if (count > 2)
	{
		for (int i = 2; i < count; i++)
		{
			DeleteMenu(listMenu, 2, MF_BYPOSITION);
		}
	}

	Bookmark_ButtonPopupMenu(hwndDlg, idFrom, listMenu, BPM_WM_COMMAND);
}

static BOOL Bookmark_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(LOWORD(id)) 
	{
		case IDC_BUTTON_PLAY:
		case ID_BMWND_PLAYSELECTEDFILES:
		case IDC_BUTTON_ENQUEUE:
		case ID_BMWND_ENQUEUESELECTEDFILES:
		case IDC_BUTTON_CUSTOM:
		{
			if (codeNotify == MLBN_DROPDOWN)
			{
				Bookmark_Play(hwnd, hwndCtl, id);
			}
			else
			{
				int action;
				if (LOWORD(id) == IDC_BUTTON_PLAY || LOWORD(id) == ID_BMWND_PLAYSELECTEDFILES)
				{
					action = (codeNotify == 1) ? g_config->ReadInt(L"enqueuedef", 0) == 1 : 0;
				}
				else if (LOWORD(id) == IDC_BUTTON_ENQUEUE || LOWORD(id) == ID_BMWND_ENQUEUESELECTEDFILES)
				{
					action = (codeNotify == 1) ? g_config->ReadInt(L"enqueuedef", 0) != 1 : 1;
				}
				else
					break;

				playFiles(action, 0);
			}
			break;
		}
		case ID_BMWND_EDITSELECTEDBOOKMARKS:
		case IDC_EDITBOOK:
			{
				int dirty=0;
				int x=0,l=m_bmlist.GetCount();
				while (x < l)
				{
					if (m_bmlist.GetSelected(x))
					{
						dirty=1;
						wchar_t fn[1024] = {0}, ft[1024] = {0};
						m_bmlist.GetText(x,0,ft,ARRAYSIZE(ft));
						m_bmlist.GetText(x,1,fn,ARRAYSIZE(fn));
						g_bmedit_fn=fn;
						g_bmedit_ft=ft;
						WASABI_API_DIALOGBOXW(IDD_EDITBOOKMARK,hwnd,BookMarkEditProc);
						m_bmlist.SetItemText(x,0,ft);
						m_bmlist.SetItemText(x,1,fn);
					}
					x++;
				}
				if (dirty) writebookmarks();
			}
			break;
		case ID_BMWND_REMOVESELECTEDBOOKMARKS:
		case IDC_REMOVEBOOK: // remove
			{
				int dirty=0;
				int x=0,l=m_bmlist.GetCount();
				while (x < l)
				{
					if (m_bmlist.GetSelected(x))
					{
						dirty=1;
						m_bmlist.DeleteItem(x);
						l--;
					}
					else x++;
				}
				if (dirty) writebookmarks();
			}
			break;
		case ID_BMWND_SELECTALL:
			Bookmark_SelectAll();
			break;
	}
	return FALSE;
}

static BOOL Bookmark_OnDestroy(HWND hwnd)
{
	if (m_bmlist.getwnd())
	{
		g_config->WriteInt(L"bm_col_title",m_bmlist.GetColumnWidth(0));
		g_config->WriteInt(L"bm_col_filename",m_bmlist.GetColumnWidth(1));
	}

	m_hwnd=0;
	return FALSE;
}

static void Bookmark_ManageButtons(HWND hwndDlg)
{
	int has_selection = m_bmlist.GetSelectedCount();

	const int buttonids[] = { IDC_BUTTON_PLAY, IDC_BUTTON_ENQUEUE, IDC_BUTTON_CUSTOM, IDC_EDITBOOK, IDC_REMOVEBOOK};
	for (size_t i = 0; i != sizeof(buttonids)/sizeof(buttonids[0]); i++)
	{
		HWND controlHWND = GetDlgItem(hwndDlg, buttonids[i]);
		EnableWindow(controlHWND, has_selection);
	}
}

static BOOL Bookmark_OnNotify( HWND hwnd, NMHDR *notification )
{
	if ( notification->idFrom == IDC_LIST )
	{
		if ( notification->code == NM_DBLCLK )
		{
			playFiles( ( !!( g_config->ReadInt( L"enqueuedef", 0 ) == 1 ) ) ^ ( !!( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ), 0 );
		}
		else if ( notification->code == LVN_BEGINDRAG )
		{
			SetCapture( hwnd );
		}
		else if ( notification->code == LVN_ITEMCHANGED )
		{
			Bookmark_ManageButtons( hwnd );
		}
	}
	return FALSE;
}

void Bookmark_UpdateButtonText(HWND hwndDlg, int _enqueuedef)
{
	if (groupBtn)
	{
		switch(_enqueuedef)
		{
			case 1:
				SetDlgItemTextW(hwndDlg, IDC_BUTTON_PLAY, view.enqueue);
				customAllowed = FALSE;
			break;

			default:
				// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
				//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
				pluginMessage p = {ML_MSG_VIEW_BUTTON_HOOK_IN_USE, (INT_PTR)_enqueuedef, 0, 0};

				wchar_t *pszTextW = (wchar_t *)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p);
				if (pszTextW && pszTextW[0] != 0)
				{
					// set this to be a bit different so we can just use one button and not the
					// mixable one as well (leaving that to prevent messing with the resources)
					SetDlgItemTextW(hwndDlg, IDC_BUTTON_PLAY, pszTextW);
					customAllowed = TRUE;
				}
				else
				{
					SetDlgItemTextW(hwndDlg, IDC_BUTTON_PLAY, view.play);
					customAllowed = FALSE;
				}
			break;
		}
	}
}

static BOOL Bookmark_OnInitDialog( HWND hwndDlg, HWND hwndFocus, LPARAM lParam )
{
	m_hwnd = hwndDlg;
	m_bmlist.setwnd( GetDlgItem( hwndDlg, IDC_LIST ) );

	HACCEL accel = WASABI_API_LOADACCELERATORSW( IDR_VIEW_BM_ACCELERATORS );
	if ( accel )
		WASABI_API_APP->app_addAccelerators( hwndDlg, &accel, 1, TRANSLATE_MODE_CHILD );

	if ( !view.play )
	{
		SENDMLIPC( plugin.hwndLibraryParent, ML_IPC_GET_VIEW_BUTTON_TEXT, (WPARAM)&view );
	}

	// check the column widths and ensure that if <=0 it'll re-show
	// (based on some forum reports where the width is set to zero somehow)
	// -> shouldn't annoy anyone but you never know with the users, heh
	int col_width1 = g_config->ReadInt( L"bm_col_title", 400 );
	if ( col_width1 <= 0 )
		col_width1 = 400;

	int col_width2 = g_config->ReadInt( L"bm_col_filename", 240 );
	if ( col_width2 <= 0 )
		col_width2 = 240;

	m_bmlist.AddCol( WASABI_API_LNGSTRINGW( IDS_BOOKMARK_TITLE ), col_width1 );
	m_bmlist.AddCol( WASABI_API_LNGSTRINGW( IDS_BOOKMARK_FN_URL ), col_width2 );

	groupBtn   = g_config->ReadInt( L"groupbtn", 1 );
	enqueuedef = ( g_config->ReadInt( L"enqueuedef", 0 ) == 1 );

	Bookmark_OnDisplayChange();

	m_headerhwnd = ListView_GetHeader( m_bmlist.getwnd() );

	// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
	//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
	pluginMessage p = { ML_MSG_VIEW_BUTTON_HOOK, (INT_PTR)hwndDlg, (INT_PTR)MAKELONG( IDC_BUTTON_CUSTOM, IDC_BUTTON_ENQUEUE ), (INT_PTR)L"ml_bookmark" };
	wchar_t *pszTextW = (wchar_t *)SENDMLIPC( plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p );
	if ( pszTextW && pszTextW[ 0 ] != 0 )
	{
		// set this to be a bit different so we can just use one button and not the
		// mixable one as well (leaving that to prevent messing with the resources)
		customAllowed = TRUE;
		SetDlgItemTextW( hwndDlg, IDC_BUTTON_CUSTOM, pszTextW );
	}
	else
		customAllowed = FALSE;

	MLSKINWINDOW m = { 0 };
	m.skinType   = SKINNEDWND_TYPE_DIALOG;
	m.style      = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	m.hwndToSkin = hwndDlg;
	MLSkinWindow( plugin.hwndLibraryParent, &m );

	m.skinType   = SKINNEDWND_TYPE_LISTVIEW;
	m.style      = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS;
	m.hwndToSkin = m_bmlist.getwnd();
	MLSkinWindow( mediaLibrary.library, &m );

	m.skinType = SKINNEDWND_TYPE_BUTTON;
	m.style    = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | ( groupBtn ? SWBS_SPLITBUTTON : 0 );
	FLICKERFIX ff = { 0, FFM_ERASEINPAINT };
	const int buttonids[] = { IDC_BUTTON_PLAY, IDC_BUTTON_ENQUEUE, IDC_BUTTON_CUSTOM };
	for ( size_t i = 0; i != sizeof( buttonids ) / sizeof( buttonids[ 0 ] ); i++ )
	{
		ff.hwnd = m.hwndToSkin = GetDlgItem( hwndDlg, buttonids[ i ] );
		if ( IsWindow( m.hwndToSkin ) )
		{
			MLSkinWindow( plugin.hwndLibraryParent, &m );
			SENDMLIPC( plugin.hwndLibraryParent, ML_IPC_FLICKERFIX, (WPARAM)&ff );
		}
	}

	m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
	const int buttonidz[] = { IDC_EDITBOOK, IDC_REMOVEBOOK };
	for ( size_t i = 0; i != sizeof( buttonidz ) / sizeof( buttonidz[ 0 ] ); i++ )
	{
		ff.hwnd = m.hwndToSkin = GetDlgItem( hwndDlg, buttonidz[ i ] );
		if ( IsWindow( m.hwndToSkin ) )
		{
			MLSkinWindow( plugin.hwndLibraryParent, &m );
			SENDMLIPC( plugin.hwndLibraryParent, ML_IPC_FLICKERFIX, (WPARAM)&ff );
		}
	}

	Bookmark_ManageButtons( hwndDlg );
	Bookmark_UpdateButtonText( hwndDlg, enqueuedef == 1 );

	SetTimer( hwndDlg, 100, 15, NULL );

	return TRUE;
}

static void Bookmark_OnTimer(HWND hwnd, UINT id)
{
	if (id == 100)
	{
		KillTimer(hwnd,100);
		// populate list
		readbookmarks();
	}
}

void Bookmark_DropFiles(HWND hwnd, HDROP hDrop)
{
	wchar_t temp[2048] = {0};
	int y = DragQueryFileW(hDrop, 0xffffffff, temp, 2048);

	for (int x = 0; x < y; x ++)
	{
		DragQueryFileW(hDrop, x, temp, 2048);
		mediaLibrary.AddBookmarkW(temp);
	}
}

static BOOL Bookmark_OnMLDropItem(mlDropItemStruct *dis)
{
	if (dis)
	{
		if (dis->type != ML_TYPE_ITEMRECORDLISTW && dis->type != ML_TYPE_ITEMRECORDLIST &&
			dis->type != ML_TYPE_FILENAMES && dis->type != ML_TYPE_FILENAMESW &&
			dis->type != ML_TYPE_STREAMNAMES && dis->type != ML_TYPE_STREAMNAMESW &&
			dis->type != ML_TYPE_CDTRACKS &&
			dis->type != ML_TYPE_PLAYLIST && dis->type != ML_TYPE_PLAYLISTS)
		{ 
			dis->result=-1;
		}
		else
		{
			dis->result=1;
			if (dis->data)
			{
				if (dis->type == ML_TYPE_ITEMRECORDLIST || dis->type == ML_TYPE_CDTRACKS) 
				{
					itemRecordList *p=(itemRecordList *)dis->data;
					for (int x = 0; x < p->Size; x ++)
						mediaLibrary.AddBookmark(p->Items[x].filename);
				}
				else if (dis->type == ML_TYPE_ITEMRECORDLISTW) 
				{
					itemRecordListW *p=(itemRecordListW *)dis->data;
					for (int x = 0; x < p->Size; x ++)
						mediaLibrary.AddBookmark(AutoChar(p->Items[x].filename));
				}
				else if (dis->type == ML_TYPE_FILENAMES || dis->type == ML_TYPE_STREAMNAMES) // playlist
				{
					char *p=(char*)dis->data;
					while (p && *p)
					{
						mediaLibrary.AddBookmark(p);
						p+=strlen(p)+1;
					}
				}
				else if (dis->type == ML_TYPE_FILENAMESW || dis->type == ML_TYPE_STREAMNAMESW) // playlist
				{
					wchar_t *p=(wchar_t*)dis->data;
					while (p && *p)
					{
						mediaLibrary.AddBookmarkW(p);
						p+=wcslen(p)+1;
					}
				}
				else if(dis->type == ML_TYPE_PLAYLIST)
				{
					mediaLibrary.AddBookmarkW((wchar_t*)((mlPlaylist*)dis->data)->filename);
				}
				else if(dis->type == ML_TYPE_PLAYLISTS)
				{
					mlPlaylist **playlists = (mlPlaylist **)dis->data;
					while (playlists && *playlists)
					{
						mlPlaylist *pl = *playlists;
						mediaLibrary.AddBookmarkW((wchar_t*)pl->filename);
						playlists++;
					}
				}
			}
		}
	}
	return FALSE;
}

#define HANDLE_ML_DROPITEM(func) case ML_CHILDIPC_DROPITEM: return Bookmark_OnMLDropItem((mlDropItemStruct *)wParam);
INT_PTR CALLBACK view_bmDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	INT_PTR a = dialogSkinner.Handle(hwndDlg, uMsg, wParam, lParam);
	if (a) return a;

	switch(uMsg) 
	{
		HANDLE_MSG(hwndDlg, WM_INITDIALOG, Bookmark_OnInitDialog);
		HANDLE_MSG(hwndDlg, WM_TIMER, Bookmark_OnTimer);
		HANDLE_MSG(hwndDlg, WM_COMMAND, Bookmark_OnCommand);
		HANDLE_MSG(hwndDlg, WM_DESTROY, Bookmark_OnDestroy);
		HANDLE_MSG(hwndDlg, WM_MOUSEMOVE, Bookmark_OnMouseMove);
		HANDLE_MSG(hwndDlg, WM_LBUTTONUP, Bookmark_OnLButtonUp);
		HANDLE_MSG(hwndDlg, WM_DROPFILES, Bookmark_DropFiles);

		case WM_WINDOWPOSCHANGED:
			if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & ((WINDOWPOS*)lParam)->flags) || 
				(SWP_FRAMECHANGED & ((WINDOWPOS*)lParam)->flags))
			{
				LayoutWindows(hwndDlg, !(SWP_NOREDRAW & ((WINDOWPOS*)lParam)->flags));
			}
			return 0;

		case WM_USER + 0x200:
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 1); // yes, we support no - redraw resize
			return TRUE;

		case WM_USER + 0x201:
			offsetX = (short)LOWORD(wParam);
			offsetY = (short)HIWORD(wParam);
			g_rgnUpdate = (HRGN)lParam;
			return TRUE;

		case WM_PAINT:
			{
				int tab[] = { IDC_LIST|DCW_SUNKENBORDER};
				dialogSkinner.Draw(hwndDlg, tab, sizeof(tab) / sizeof(tab[0]));
			}
			return 0;

		case WM_INITMENUPOPUP:
			if (wParam && (HMENU)wParam == s.build_hMenu && s.mode==1)
			{
				myMenu = TRUE;
				if(SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU)==0xffffffff)
					s.mode=2;
				myMenu = FALSE;
			}
			return 0;

		case WM_DISPLAYCHANGE:
			return Bookmark_OnDisplayChange();

		case WM_NOTIFY:
			return Bookmark_OnNotify(hwndDlg, (LPNMHDR)lParam);

		case WM_CONTEXTMENU:
			bookmarks_contextMenu(hwndDlg, (HWND)wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		case WM_APP + 104:
		{
			Bookmark_UpdateButtonText(hwndDlg, (int)wParam);
			LayoutWindows(hwndDlg, TRUE);
			return 0;
		}

		case WM_ML_CHILDIPC:
			if (lParam == ML_CHILDIPC_DROPITEM && wParam)
				HANDLE_ML_DROPITEM(Bookmark_OnMLDropItem);
	}
	return FALSE;
}