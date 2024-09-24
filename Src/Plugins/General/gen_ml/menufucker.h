#ifndef _NULLSOFT_GEN_ML_MENUFUCKER_H_
#define _NULLSOFT_GEN_ML_MENUFUCKER_H_

#include "ml.h"
#include "../playlist/ifc_playlist.h"

/*
there are two IPC messages, both sent to your ml plugins messageproc. Get the message IDs by doing:
ML_IPC_MENUFUCKER_BUILD = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"menufucker_build", IPC_REGISTER_WINAMP_IPCMESSAGE);
ML_IPC_MENUFUCKER_RESULT = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"menufucker_result", IPC_REGISTER_WINAMP_IPCMESSAGE);

ML_IPC_MENUFUCKER_BUILD:
This is sent just before the menu is shown. param1 is a pointer to a menufucker_t struct.
Do what you like to the menu, if you add anything, give it the id nextidx, and increment nextidx

ML_IPC_MENUFUCKER_RESULT:
param1 is a pointer to a menufucker_t struct, param2 is the id of the menu item selected
*/

#define MENU_MEDIAVIEW 0
#define MENU_MLPLAYLIST 1
#define MENU_PLAYLIST 2
#define MENU_SONGTICKER 3

typedef struct {
	size_t size;
	int type;
	HMENU menu;
	int nextidx;
	int maxidx;
	union {
		struct {
			HWND list;
			itemRecordListW *items;
		} mediaview; // valid if type==MENU_MEDIAVIEW
		struct {
			HWND list;
			ifc_playlist * pl;
		} mlplaylist; // valid if type==MENU_MLPLAYLIST
	} extinf;
} menufucker_t;

#endif // _NULLSOFT_GEN_ML_MENUFUCKER_H_