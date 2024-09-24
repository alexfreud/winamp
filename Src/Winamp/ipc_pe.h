#ifndef __IPC_PE_H
#define __IPC_PE_H

/*
** To use these messages you will need a valid window handle for the playlist window
** and the format to use them is:
**
** SendMessageW(playlist_wnd,WM_WA_IPC,IPC_*,(parameter));
**
** Note:
**      This IS the OPPOSITE way to how the messages to the main winamp window are sent
**      SendMessageW(hwnd_winamp,WM_WA_IPC,(parameter),IPC_*);
*/

/*
** Playlist Window:
**
** To get the playlist window there are two ways depending on the version you're using
**
** HWND playlist_wnd = 0;
** int wa_version = SendMessageW(plugin.hwndParent,WM_WA_IPC,0,IPC_GETVERSION);
**
** if(wa_version >= 0x2900)
** {
**   // use the built in api to get the handle
**   playlist_wnd = (HWND)SendMessageW(plugin.hwndParent,WM_WA_IPC,IPC_GETWND_PE,IPC_GETWND);
** }
**
** // if it failed then use the old way :o)
** if(!IsWindow(playlist_wnd))
** {
**   playlist_wnd = FindWindow("Winamp PE",0);
** }
*/


/*
** Structures used by some of the apis referenced.
*/
typedef struct {
	char file[MAX_PATH];
	int index;
} fileinfo;

typedef struct {
	wchar_t file[MAX_PATH];
	int index;
} fileinfoW;

typedef struct {
	HWND callback;
	int index;
} callbackinfo;

typedef struct {
	int fileindex;
	char filetitle[256];
	char filelength[16];
} fileinfo2;

typedef struct {
	int fileindex;
	wchar_t filetitle[256];
	wchar_t filelength[16];
} fileinfo2W;


#define IPC_PE_GETCURINDEX           100 // returns current idx (typically the playing item)
#define IPC_PE_GETINDEXTOTAL         101 // returns the number of items in the playlist

#define IPC_PE_GETINDEXINFO          102 // (copydata) lpData is of type callbackinfo, callback is called with copydata/fileinfo structure and msg IPC_PE_GETINDEXINFORESULT
#define IPC_PE_GETINDEXINFORESULT    103 // callback message for IPC_PE_GETINDEXINFO

#define IPC_PE_DELETEINDEX           104 // lParam = index

#define IPC_PE_SWAPINDEX             105 // (lParam & 0xFFFF0000) >> 16 = from, (lParam & 0xFFFF) = to
/*
** SendMessageW(playlist_wnd,WM_WA_IPC,IPC_PE_SWAPINDEX,MAKELPARAM(from,to));
*/

#define IPC_PE_INSERTFILENAME        106 // (copydata) lpData is of type fileinfo
#define IPC_PE_INSERTFILENAMEW       114 // (copydata) lpData is of type fileinfoW (5.3+)
/* COPYDATASTRUCT cds = {0};
** fileinfo f = {0};
**
**   lstrcpyn(f.file, file,MAX_PATH);	// path to the file
**   f.index = position;			// insert file position
**
**   cds.dwData = IPC_PE_INSERTFILENAME;
**   cds.lpData = (void*)&f;
**   cds.cbData = sizeof(fileinfo);
**   SendMessageW(playlist_wnd,WM_COPYDATA,0,(LPARAM)&cds);
*/


#define IPC_PE_GETDIRTY              107 // returns 1 if the playlist changed since the last IPC_PE_SETCLEAN
#define IPC_PE_SETCLEAN	             108 // resets the dirty flag until next modification

#define IPC_PE_GETIDXFROMPOINT       109 // pass a point param and will return a playlist index (if in the area)
/*
** POINT pt;
** RECT rc;
**
**   // Get the current position of the mouse and the current client area of the playlist window
**   // and then mapping the mouse position to the client area
**   GetCursorPos(&pt);
**
**   // Get the client area of the playlist window and then map the mouse position to it
**   GetClientRect(playlist_wnd,&rc);
**   ScreenToClient(playlist_wnd,&pt);
**
**   // this corrects so the selection works correctly on the selection boundary
**   // appears to happen on the older 2.x series as well
**   pt.y -= 2;
**
**   // corrections for the playlist window area so that work is only done for valid positions
**   // and nicely enough it works for both classic and modern skin modes
**   rc.top += 18;
**   rc.left += 12;
**   rc.right -= 19;
**   rc.bottom -= 40;
**
**   // is the click in 
**   if(PtInRect(&rc,pt))
**   {
**     // get the item index at the given point
**     // if this is out of range then it will return 0 (not very helpful really)
**     int idx = SendMessageW(playlist_wnd,WM_WA_IPC,IPC_PE_GETIDXFROMPOINT,(LPARAM)&pt);
**
**     // makes sure that the item isn't past the last playlist item
**     if(idx < SendMessageW(playlist_wnd,WM_WA_IPC,IPC_PE_GETINDEXTOTAL,0))
**     {
**       // ... do stuff in here (this example will start playing the selected track)
**       SendMessageW(plugin.hwndParent,WM_WA_IPC,idx,IPC_SETPLAYLISTPOS);
**       SendMessageW(plugin.hwndParent,WM_COMMAND,MAKEWPARAM(WINAMP_BUTTON2,0),0);
**     }
**   }
*/

#define IPC_PE_SAVEEND               110 // pass index to save from
#define IPC_PE_RESTOREEND            111 // no parm

#define IPC_PE_GETNEXTSELECTED       112 // same as IPC_PLAYLIST_GET_NEXT_SELECTED for the main window
#define IPC_PE_GETSELECTEDCOUNT      113

#define IPC_PE_GETINDEXINFO_TITLE    115 // like IPC_PE_GETINDEXINFO, but writes the title to char file[MAX_PATH] instead of filename
#define IPC_PE_GETINDEXINFORESULT_TITLE 116 // callback message for IPC_PE_GETINDEXINFO


// the following messages are in_process ONLY

#define IPC_PE_GETINDEXTITLE         200 // lParam = pointer to fileinfo2 struct
#define IPC_PE_GETINDEXTITLEW        201 // lParam = pointer to fileinfo2W struct
/*
** fileinfo2 file;
** int ret = 0;
**
**   file.fileindex = position;	// this is zero based!
**   ret = SendMessageW(playlist_wnd,WM_WA_IPC,IPC_PE_GETINDEXTITLE,(LPARAM)&file);
**
**   // if it returns 0 then track information was received
**   if(!ret)
**   {
**     // ... do stuff
**   }
*/

#define IPC_PE_GETINDEXINFO_INPROC   202 // lParam = pointer to fileinfo struct
#define IPC_PE_GETINDEXINFOW_INPROC  203 // lParam = pointer to fileinfoW struct

#endif