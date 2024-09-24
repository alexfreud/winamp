/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "resource.h"
#include "../Plugins/General/gen_ml/ml.h"
extern "C" HMENU g_submenus_vis;
#include "../Plugins/General/gen_ml/ml_ipc.h"
extern librarySendToMenuStruct mainSendTo;
#include "../Plugins/General/gen_ml/menufucker.h"

void ScaleMouseButtons(int &x, int &y)
{
	if (config_dsize)
	{
		x /= 2;
		y /= 2;
	}
}

// Pops up a menu on right click
int Main_OnRButtonUp(HWND hwnd, int x, int y, UINT flags)
{
	HMENU hmenu=main_menu;
	POINT p;

	if ((flags & MK_LBUTTON)) return 1;

	GetCursorPos(&p);
	ScaleMouseButtons(x,y);

	if (x < 266 && x > 105 && y < 35 && y > 24)
	{
		int rating = 0;
		static HMENU ratingmenu = NULL;
		hmenu = GetSubMenu(GetSubMenu(top_menu,3),0);
		CheckMenuItem(hmenu,WINAMP_TOGGLE_AUTOSCROLL,(config_autoscrollname&1)?MF_CHECKED:MF_UNCHECKED);
		rating = sendMlIpc(ML_IPC_GETRATING, 0);
		// deal with no ml being present and querying the rating of a file from the tag (if possible)
		// as well as getting a zero rating which could mean that it's not present in the library
		if (!rating && !got_ml)
		{
			wchar_t fn[FILENAME_SIZE] = {0};
			if (!PlayList_getitem2W(PlayList_getPosition(), fn, NULL))
			{
				wchar_t buf[64] = {0};
				in_get_extended_fileinfoW(fn, L"rating", buf, 64);
				rating = _wtoi(buf);
			}
		}

		if(!IsMenu(ratingmenu))
		{
			ratingmenu = GetSubMenu(hmenu, 5);
		}

		CheckMenuItem(ratingmenu, ID_RATING5, (rating == 5) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(ratingmenu, ID_RATING4, (rating == 4) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(ratingmenu, ID_RATING3, (rating == 3) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(ratingmenu, ID_RATING2, (rating == 2) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(ratingmenu, ID_RATING1, (rating == 1) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(ratingmenu, ID_RATING0, (rating == 0) ? MF_CHECKED : MF_UNCHECKED);

		LRESULT IPC_LIBRARY_SENDTOMENU = wa_register_ipc((WPARAM)&"LibrarySendToMenu");
		HMENU menu = 0;
		memset(&mainSendTo, 0, sizeof(mainSendTo));
		if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)0, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
		{
			MENUITEMINFOW mii = {sizeof(mii), MIIM_SUBMENU | MIIM_TYPE, MFT_STRING, };
			mii.hSubMenu = menu = CreatePopupMenu();
			mii.dwTypeData = getStringW(IDS_SENDTO_STR,NULL,0);
			mii.cch = (UINT)wcslen(mii.dwTypeData);

			InsertMenuItemW(hmenu, 4, TRUE, &mii);

			mainSendTo.mode = 1;
			mainSendTo.hwnd = hwnd;
			mainSendTo.data_type = ML_TYPE_FILENAMESW;
			mainSendTo.build_hMenu = menu;
		}

		menufucker_t mf = {sizeof(mf),MENU_SONGTICKER,hmenu,0x3000,0x4000,0};
		pluginMessage message_build = {(int)wa_register_ipc((WPARAM)&"menufucker_build"),(intptr_t)&mf,0};
		sendMlIpc(ML_IPC_SEND_PLUGIN_MESSAGE,(WPARAM)&message_build);
		
		int ret = DoTrackPopup(hmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, hwnd);

		pluginMessage message_result = {(int)wa_register_ipc((WPARAM)&"menufucker_result"),(intptr_t)&mf,ret,0};
		sendMlIpc(ML_IPC_SEND_PLUGIN_MESSAGE,(WPARAM)&message_result);

		if (menu)
		{
			if (mainSendTo.mode == 2)
			{
				mainSendTo.menu_id = ret;
				if (SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&mainSendTo, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
				{
					wchar_t buf[1041] = {0};
					wchar_t *end=buf;
					size_t endSize;
					StringCchCopyExW(buf, 1040, FileName, &end, &endSize, 0);
					end[1]=0; // double null terminate (since we passed 1040 for a 1041 size buffer, this is always safe)

					mainSendTo.mode = 3;
					mainSendTo.data = buf;
					mainSendTo.data_type = ML_TYPE_FILENAMESW;
					SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&mainSendTo, IPC_LIBRARY_SENDTOMENU);
				}
			}
			// remove sendto
			DeleteMenu(hmenu, 4, MF_BYPOSITION);
		}
		if (mainSendTo.mode)
		{
			mainSendTo.mode = 4;
			SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&mainSendTo, IPC_LIBRARY_SENDTOMENU); // cleanup
			memset(&mainSendTo, 0, sizeof(mainSendTo));
		}
		if (ret) SendMessageW(hwnd, WM_COMMAND, ret, 0);
		return 1;
	}
	else if ((!config_windowshade && x >= 36 && y >= 26 && x < 96 && y < 39) || 
		(config_windowshade && x >= 129 && y >= 3 && x < 129+28 && y < 3+6))
	{
		hmenu = GetSubMenu(GetSubMenu(top_menu,3),1);
		CheckMenuItem(hmenu,WINAMP_OPTIONS_ELAPSED,config_timeleftmode?MF_UNCHECKED:MF_CHECKED);
		CheckMenuItem(hmenu,WINAMP_OPTIONS_REMAINING,config_timeleftmode?MF_CHECKED:MF_UNCHECKED);		
	}
	else if ((x >= 27 && y >= 40 && x < 99 && y < 61) ||
		(config_windowshade && x >= 78 && y >= 4 && x < 116 && y < 11))
	{
		hmenu=g_submenus_vis;
	}
	else if ((x >= 16 && y >= 88 && x < 37 && y < 106) ||				/// button 1
		(config_windowshade && x >= 167 && y >= 3 && x < 176 && y < 12))
	{
		hmenu = GetSubMenu(GetSubMenu(top_menu,3),2);
	}
	else if ((x >= 37 && y >= 88 && x < 62 && y < 106) ||				/// button 2
		(config_windowshade && x >= 176 && y >= 3 && x < 186 && y < 12))
	{
		hmenu = GetSubMenu(GetSubMenu(top_menu,3),3);
	}
	else if ((x >= 62 && y >= 88 && x < 89 && y < 106) ||				/// button 3
		(config_windowshade && x >= 186 && y >= 3 && x < 196 && y < 12))
	{
		hmenu = GetSubMenu(GetSubMenu(top_menu,3),4);
	}
	else if ((x >= 89 && y >= 88 && x < 107 && y < 106) ||				/// button 4
		(config_windowshade && x >= 196 && y >= 3 && x < 206 && y < 12))
	{
		hmenu = GetSubMenu(GetSubMenu(top_menu,3),5);
	}
	else if ((x >= 107 && y >= 88 && x < 130 && y < 106) ||				/// button 5
		(config_windowshade && x >= 206 && y >= 3 && x < 216 && y < 12))
	{
		hmenu = GetSubMenu(GetSubMenu(top_menu,3),6);
	}
	else if ((x >= 136 && y >= 89 && x < 158 && y < 105) ||				/// eject
		(config_windowshade && x >= 215 && y >= 3 && x < 225 && y < 12))
	{
		hmenu = GetSubMenu(GetSubMenu(top_menu,3),7);
		UpdateAudioCDMenus(hmenu);
	}
	else if (playing && (in_mod && in_mod->is_seekable) && x >= 18 && y >= 73 && x < 263 && y < 81)
	{
		hmenu = GetSubMenu(GetSubMenu(top_menu,3),8);
	}
	else if (x >= 164 && y >= 89 && x < 211 && y < 104)			/// shuffle
	{
		hmenu = GetSubMenu(GetSubMenu(top_menu,3),9);
		CheckMenuItem(hmenu,WINAMP_FILE_SHUFFLE,config_shuffle?MF_CHECKED:MF_UNCHECKED); 
	}	
	else if (x >= 211 && y >= 89 && x < 211+28 && y < 104)			/// repeat
	{
		hmenu = GetSubMenu(GetSubMenu(top_menu,3),10);
		CheckMenuItem(hmenu,WINAMP_FILE_REPEAT,config_repeat?MF_CHECKED:MF_UNCHECKED); 
		CheckMenuItem(hmenu,WINAMP_FILE_MANUALPLADVANCE,config_pladv?MF_UNCHECKED:MF_CHECKED); 
	}	
	else if (x >= 219 && y >= 58 && x < 219+(265-219)/2 && y < 70)			/// eq
	{
		hmenu = GetSubMenu(GetSubMenu(top_menu,3),11);
		CheckMenuItem(hmenu,WINAMP_OPTIONS_EQ,config_eq_open?MF_CHECKED:MF_UNCHECKED); 
	}	
	else if (x >= 219+(265-219)/2 && y >= 58 && x < 265 && y < 70)			/// pe
	{
		hmenu = GetSubMenu(GetSubMenu(top_menu,3),12);
		CheckMenuItem(hmenu,WINAMP_OPTIONS_PLEDIT,config_pe_open?MF_CHECKED:MF_UNCHECKED); 
	}	
	DoTrackPopup(hmenu, TPM_LEFTALIGN|TPM_RIGHTBUTTON, p.x, p.y, hwnd);
	return 1;
}

// Doubleclick handler. Checks for a few regions, and usually does nothing,
// except pass on a WM_LBUTTONDOWN.
int Main_OnLButtonDblClk(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	int nx=x, ny=y;

	ScaleMouseButtons(nx,ny);

	if (nx <= 16 && nx >= 6 && ny <= 12 && ny >= 4)
	{
		KillTimer(hwnd,666);
		SendMessageW(hwnd,WM_CLOSE,0xDEADBEEF,0xDEADF00D);
	}
	else if (config_windowshade && nx <= 158 && nx >= 129 &&
		ny <= 10 && ny >= 5)
		Main_OnLButtonDown(hwnd,0,x,y,keyFlags);
	else if (config_windowshade && nx <= 213 && nx >= 168 &&
		ny <= 11 && ny >= 2)
		Main_OnLButtonDown(hwnd,0,x,y,keyFlags);
	else if (nx < 266 && nx > 117 && ny < 35 && ny > 24)
	{
		SendMessageW(hwnd,WM_COMMAND,WINAMP_EDIT_ID3,0);
	}
	else if ((nx >= 24 && ny >= 43 && nx < 24+76 && ny < 43+16) || 
		(config_windowshade && nx >= 79 && ny >= 5 && nx < 79+38 && ny < 5+5))
	{
		if (config_windowshade) config_sa = !config_sa;
		else
		{
			config_sa+=2;
			config_sa %= 3;
		}
		sa_setthread(config_sa);
		if (vis_running()) vis_stop();
		else vis_start(hMainWindow,NULL);
	}
	else if (ny < 15 && nx < 244) 
	{
		SendMessageW(hwnd,WM_COMMAND,WINAMP_OPTIONS_WINDOWSHADE,0);
		UpdateWindow(hwnd);
		Sleep(200);
		for (;;)
		{
			MSG msg;
			if (!PeekMessage(&msg,hMainWindow,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE)) 
				break;
		}
	}
	else Main_OnLButtonDown(hwnd,0,x,y,keyFlags);
	return 1;
}

int Main_OnCaptureChanged(HWND hwnd)
{
	if (hwnd != hMainWindow)
		ui_handlemouseevent(0,0,-2,0);
	return 0;
}
// Mouseup handler. Just passes to routines in ui.c, scaling if in doublesize mode
int Main_OnLButtonUp(HWND hwnd, int x, int y, UINT flags)
{
	ScaleMouseButtons(x,y);
	ui_handlemouseevent(x,y,-1,flags);
	ReleaseCapture();
	return 1;
}

// Mousedown handler. Just passes to routines in ui.c, scaling if in doublesize mode
int Main_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	ScaleMouseButtons(x,y);
	SetCapture(hwnd);
	ui_handlemouseevent(x,y,1,keyFlags);
	return 1;
}

// Mousemove handler. Just passes to routines in ui.c, scaling if in doublesize mode
int Main_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	ScaleMouseButtons(x,y);
	ui_handlemouseevent(x,y,0,keyFlags);
	return 1;
}