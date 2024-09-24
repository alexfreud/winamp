#include "resource.h"
#include "main.h"
#include "api.h"
#include "../nu/listview.h"
#include "../nu/ChildSizer.h"
#include "../nu/DialogSkinner.h"
#include "impl_playlist.h"
#include "../nu/SendTo.h"
#include <strsafe.h>

static ChildWndResizeItem view_rlist[] =
{
	{ IDC_PLAYLIST, ResizeBottom | ResizeRight},
	{ IDC_PLAY, DockToBottom},
	{ IDC_ENQUEUE, DockToBottom},
	{ IDC_SAVE, DockToBottom},
	{ IDC_SENDTO, DockToBottom},
};

W_ListView playlist_list;
Playlist currentPlaylist;
int playlist_skin;
static SendToMenu sendTo(plugin);


static void PlaySelection(int enqueue, int is_all)
{
	if (!enqueue)
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_DELETE);

	int numTracks = playlist_list.GetCount();
	for (int i = 0;i < numTracks;i++)
	{
		if (is_all || playlist_list.GetSelected(i))
		{
			if (currentPlaylist.IsCached(i))
			{
				enqueueFileWithMetaStructW s;
				s.filename = currentPlaylist.ItemName(i);
				s.title = currentPlaylist.ItemTitle(i);
				s.length = currentPlaylist.GetItemLengthMilliseconds(i) / 1000;
				SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
			}
			else
			{
				enqueueFileWithMetaStructW s;
				s.filename = currentPlaylist.ItemName(i);
				s.title = 0;
				s.length = 0;
				SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
			}

		}
	}

	if (!enqueue)
	{
		if (is_all)
		{
			int pos = playlist_list.GetNextSelected(-1);
			if (pos != -1)
			{
				SendMessage(plugin.hwndWinampParent, WM_WA_IPC, pos, IPC_SETPLAYLISTPOS);
				SendMessage(plugin.hwndWinampParent, WM_COMMAND, 40047, 0); // stop button, literally
				SendMessage(plugin.hwndWinampParent, WM_COMMAND, 40045, 0); // play button, literally
				return ;
			}
		}
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_STARTPLAY);
	}
	/*
		int cnt=0;
		int l=PlayList_getlength();
	 
		int foo_all=0; // all but play the only selected
		int foo_selected=-1;
	 
		if (g_config->ReadInt("plplaymode",1)||is_all)
		{
		int selcnt=0;
		for(int i=0;i<l;i++)
		{
		if(PlayList_getselect(i)) selcnt++;
		}
		if ((selcnt == 1 && !enqueue) || selcnt == 0)
		{
		foo_all=-1;
		}
		}
	 
	*/
}


static void playlist_DisplayChange()
{
	playlist_list.SetTextColors(dialogSkinner.Color(WADLG_ITEMFG), dialogSkinner.Color(WADLG_ITEMBG));
	ListView_SetBkColor(playlist_list.getwnd(), dialogSkinner.Color(WADLG_ITEMBG));
	playlist_list.SetFont(dialogSkinner.GetFont());
}

static BOOL playlist_GetDisplayInfo(NMLVDISPINFO *lpdi)
{
	size_t item = lpdi->item.iItem;
	if (item < 0 || item >= currentPlaylist.GetNumItems()) return 0;

	if (lpdi->item.mask & LVIF_TEXT)
	{
		switch (lpdi->item.iSubItem)
		{
		case 0:
		{
			bool cached = currentPlaylist.IsCached(item);

			if (!cached)
			{
				wchar_t title[400];
				int length;
				mediaLibrary.GetFileInfo(currentPlaylist.ItemName(item), title, 400, &length);
				if (length == 0)
					currentPlaylist.SetItemLengthMilliseconds(item, -1000);
				else
					currentPlaylist.SetItemLengthMilliseconds(item, length*1000);
				currentPlaylist.SetItemTitle(item, title);
			}
			// CUT: currentPlaylist.GetItemTitle(item, lpdi->item.pszText, lpdi->item.cchTextMax);
			StringCchPrintf(lpdi->item.pszText, lpdi->item.cchTextMax, L"%d. %s", item + 1, currentPlaylist.ItemTitle(item));
		}
		break;
		case 1:
		{

			if (currentPlaylist.GetItemLengthMilliseconds(item) == 0) // if the length is 0, then we'll re-read it
			{
				wchar_t title[400];
				int length;
				mediaLibrary.GetFileInfo(currentPlaylist.ItemName(item), title, 400, &length);
				if (length == 0)
					currentPlaylist.SetItemLengthMilliseconds(item, -1000);
				else
				{
					currentPlaylist.SetItemLengthMilliseconds(item, length*1000);
//						currentPlaylist.SetItemTitle(item, AutoWide(title));
				}
			}

			int length = currentPlaylist.GetItemLengthMilliseconds(item) / 1000;
			if (length <= 0)
				lpdi->item.pszText[0] = 0;
			else
				StringCchPrintf(lpdi->item.pszText, lpdi->item.cchTextMax, L"%d:%02d", length / 60, length % 60);
		}
		break;
		}
	}
	return 0;
}


static INT_PTR playlist_Notify(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR l = (LPNMHDR)lParam;
	if (l->idFrom == IDC_PLAYLIST)
	{
		switch (l->code)
		{
			/*
			case NM_RETURN:
			if (!(GetAsyncKeyState(VK_SHIFT)&0x8000))
				PlaySelection(g_config->ReadInt("enqueuedef", 0), g_config->ReadInt("plplaymode", 1));
			else
				PlaySelection(!g_config->ReadInt("enqueuedef", 0), 0);
			break;

			case NM_DBLCLK:
			PlaySelection(g_config->ReadInt("enqueuedef", 0), g_config->ReadInt("plplaymode", 1));
			break;*/
		case LVN_GETDISPINFO:
			return playlist_GetDisplayInfo((NMLVDISPINFO*) lParam);
			/*case LVN_BEGINDRAG:
				we_are_drag_and_dropping = 1;
				SetCapture(hwndDlg);
				break;
				*/
			/*
			case LVN_ITEMCHANGED:
			case LVN_ODSTATECHANGED:
			UpdatePlaylistTime(hwndDlg);
			break;
			*/
			/*
			case LVN_KEYDOWN:
			{
				LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
				switch (pnkd->wVKey)
				{
				case VK_DELETE:
			    if (GetAsyncKeyState(VK_CONTROL)&0x8000)
			      Playlist_DeleteSelected(0);
					else
			    Playlist_DeleteSelected(1);

					break;

				case '3':
					if (GetAsyncKeyState(VK_MENU)&0x8000)
						TagEditor(hwndDlg);
					break;
				case 'A':
					if (GetAsyncKeyState(VK_CONTROL)&0x8000)
						playlist_list.SelectAll();
					break;
				case 'I':
					if (GetAsyncKeyState(VK_CONTROL)&0x8000)
						playlist_list.InvertSelection();
					break;
				case 'L':
					if (GetAsyncKeyState(VK_CONTROL)&0x8000)
						CurrentPlaylist_AddLocation(hwndDlg);
					else if (GetAsyncKeyState(VK_SHIFT)&0x8000)
						CurrentPlaylist_AddDirectory(hwndDlg);
					else
						CurrentPlaylist_AddFiles(hwndDlg);
					SyncPlaylist();
					break;
				case 'R':
					if (GetAsyncKeyState(VK_CONTROL)&0x8000)
					{
						if (GetAsyncKeyState(VK_SHIFT)&0x8000)
							AGAVE_API_PLAYLISTMANAGER->Randomize(&currentPlaylist);
						else
							AGAVE_API_PLAYLISTMANAGER->Reverse(&currentPlaylist);
						playlist_list.RefreshAll();
					}
					break;
				case 'E':
					if (GetAsyncKeyState(VK_CONTROL)&0x8000)
					{
					if (GetAsyncKeyState(VK_MENU)&0x8000)
						Playlist_ResetSelected();
					else
						EditEntry(l->hwndFrom);
					}
						break;
					
				}
			}
			break;
			*/
		}
	}
	return 0;
}

static wchar_t *BuildFilenameList(int is_all)
{
	wchar_t filename[MAX_PATH] = {0};
	size_t len = 200;
	wchar_t *str = (wchar_t *)malloc(len*sizeof(wchar_t));
	size_t sofar = 0;

	int numTracks = playlist_list.GetCount();
	for (int i = 0;i < numTracks;i++)
	{
		if (is_all || playlist_list.GetSelected(i))
		{
			if (currentPlaylist.GetItem(i, filename, MAX_PATH))
			{
				size_t filenameLen = wcslen(filename);
				if ((filenameLen + sofar) > len)
				{
					size_t newLen = filenameLen + sofar + 32; // add some cushion
					wchar_t *newStr = (wchar_t *)malloc(newLen*sizeof(wchar_t));
					memcpy(newStr, str, sofar*sizeof(wchar_t));
					len = newLen;
					free(str);
					str = newStr;
				}

				StringCchCopyW(str + sofar, filenameLen, filename);
				sofar += filenameLen + 1;
			}
		}
	}
	*(str + sofar) = 0;

	return str;
}

INT_PTR CALLBACK ViewProcedure(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR a = dialogSkinner.Handle(hwndDlg, msg, wParam, lParam);
	if (a)
		return a;

	switch (msg)
	{
	case WM_INITMENUPOPUP:
		sendTo.InitPopupMenu(wParam);
		return 0;
	case WM_INITDIALOG:
		childSizer.Init(hwndDlg, view_rlist, sizeof(view_rlist) / sizeof(view_rlist[0]));

		playlist_list.setwnd(GetDlgItem(hwndDlg, IDC_PLAYLIST));
		playlist_skin = mediaLibrary.SkinList(GetDlgItem(hwndDlg, IDC_PLAYLIST));
		playlist_list.AddCol(L"Song",200);
		playlist_list.AddCol(L"Length",100);
		playlist_list.JustifyColumn(1, LVCFMT_RIGHT);
		playlist_DisplayChange();
		playlist_list.SetVirtualCount(currentPlaylist.GetNumItems());
		break;
	case WM_PAINT:
	{
		int tab[] = {IDC_PLAYLIST|DCW_SUNKENBORDER};
		dialogSkinner.Draw(hwndDlg,tab,1);
	}
	return 0;
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
			childSizer.Resize(hwndDlg, view_rlist, sizeof(view_rlist) / sizeof(view_rlist[0]));

		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PLAY:
			if (playlist_list.GetSelectedCount() > 0)
				PlaySelection(0, 0/*g_config->ReadInt("plplaymode", 1)*/);
			else
				PlaySelection(0, 1);
			break;
		case IDC_ENQUEUE:
			if (playlist_list.GetSelectedCount() > 0)
				PlaySelection(0, 0);
			else
				PlaySelection(1, 0);
			break;
		case IDC_SAVE:
			break;
		case IDC_SENDTO:
		{
			HMENU blah = CreatePopupMenu();
			RECT r;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_SENDTO), &r);
			sendTo.AddHere(hwndDlg, blah, ML_TYPE_FILENAMESW);
			int x = TrackPopupMenu(blah, TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, r.left, r.top, 0, hwndDlg, NULL);
			if (sendTo.WasClicked(x))
			{
				int is_all = playlist_list.GetSelectedCount() == 0;
				wchar_t *names = BuildFilenameList(is_all);
				sendTo.SendFilenames(names);
				free(names);
			}

			sendTo.Cleanup();

		}
		break;
		}
		break;
	case WM_DISPLAYCHANGE: playlist_DisplayChange(); return 0;
	case WM_NOTIFY: return playlist_Notify(hwndDlg, wParam, lParam);
	case WM_DESTROY:
		mediaLibrary.UnskinList(playlist_skin);
		break;
	}
	return 0;
}