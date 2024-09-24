/*
** Copyright (C) 2003-2014 Winamp SA
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
*/

#ifndef _ML_H_
#define _ML_H_

#define MLHDR_VER     0x17		// used from Winamp v5.66
#define MLHDR_VER_U   0x16		// used from Winamp v5.64 to v5.65 (is loaded by v5.66+)
#define MLHDR_VER_OLD 0x15	    // used up to Winamp v5.63 (is loaded by v5.64+)

#include <windows.h>
#include <commctrl.h>
#include <stddef.h>
#include <api/service/api_service.h>

typedef struct
{
	int version;	// changed 5.64+ - MLHDR_VER will now require a unicode (wchar_t*) description and only work correctly on 5.64+
					//				   MLHDR_VER_OLD will still be loaded but will use the original (char*) description as before
					//				   note: we are using the fact that sizeof(char*) == sizeof(wchar_t*) to be able to allow this
					//						 so when using MLHDR_VER = 0x16 or higher then you will need to cast description to be
					//						 a (char*) to be able to set it
	const char *description;
	int (__cdecl *init)(); // return 0 on success, non-zero for failure (quit WON'T be called) 
	void (__cdecl *quit)();

	// return NONZERO if you accept this message as yours, otherwise 0 to pass it to other plugins
	INT_PTR (__cdecl *MessageProc)(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3); 
	// Note: INT_PTR becomes 64bit on 64bit compiles...
	//       if you don't like windows types or caps, you can #include <stddef.h> and use intptr_t

	// all the following data is filled in by the library
	HWND hwndWinampParent;
	HWND hwndLibraryParent; // send this any of the WM_ML_IPC messages
	HINSTANCE hDllInstance;

	// filled in by Winamp (added 5.66+ to replace need to call IPC_GET_API_SERVICE on loading)
	#ifdef __cplusplus
	api_service *service;
	#else
	void * service;
	#endif
} winampMediaLibraryPlugin;

// return values from the init(..) which determines if Winamp will continue loading
// and handling the plugin or if it will disregard the load attempt. If ML_INIT_FAILURE
// is returned then the plugin will be listed as [NOT LOADED] on the plug-in prefs page.
#define ML_INIT_SUCCESS 0
#define ML_INIT_FAILURE 1

// These are the return values to be used with the uninstall plugin export function:
// __declspec(dllexport) int __cdecl winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param)
// which determines if Winamp can uninstall the plugin immediately or on winamp restart.
// If this is not exported then Winamp will assume an uninstall with reboot is the only way.

#define ML_PLUGIN_UNINSTALL_NOW    0x1
#define ML_PLUGIN_UNINSTALL_REBOOT 0x0

//
// Uninstall support was added from 5.0+ and uninstall now support from 5.5+ though note
// that it is down to you to ensure that if uninstall now is returned that it will not
// cause a crash i.e. don't use if you've been subclassing the main or library windows.
//
// The HWND passed in the calling of winampUninstallPlugin(..) is the preference page HWND.
//

// For a Media Library plugin to be correctly detected by Winamp you need to ensure that
// the exported winampMediaLibraryPlugin(..) is exported as an undecorated function
// e.g.
// #ifdef __cplusplus
//   extern "C" {
// #endif
// __declspec(dllexport) winampMediaLibraryPlugin * __cdecl winampGetMediaLibraryPlugin(){ return &plugin; }
// #ifdef __cplusplus
//   }
// #endif
//

// messages your plugin may receive on MessageProc()

#define ML_MSG_TREE_BEGIN        0x100
#define ML_MSG_TREE_ONCREATEVIEW 0x100  // param1 = param of tree item, param2 is HWND of parent. return HWND if it is us

#define ML_MSG_TREE_ONCLICK      0x101  // param1 = param of tree item, param2 = action type (below), param3 = HWND of main window
#define ML_ACTION_RCLICK         0      // return value should be nonzero if ours
#define ML_ACTION_DBLCLICK       1
#define ML_ACTION_ENTER          2
#define ML_ACTION_LCLICK         3

#define ML_MSG_TREE_ONDROPTARGET 0x102  // param1 = param of tree item, param2 = type of drop (ML_TYPE_*), param3 = pointer to data (or NULL if querying).
									    // return -1 if not allowed, 1 if allowed, or 0 if not our tree item

#define ML_MSG_TREE_ONDRAG       0x103  // param1 = param of tree item, param2 = POINT * to the mouse position, and param3 = (int *) to the type
								        // set *(int*)param3 to the ML_TYPE you want and return 1, if you support drag&drop, or -1 to prevent d&d.

#define ML_MSG_TREE_ONDROP       0x104  // param1 = param of tree item, param2 = POINT * to the mouse position
								        // if you support dropping, send the appropriate ML_IPC_HANDLEDROP using SendMessageW() and return 1, otherwise return -1.

#define  ML_MSG_TREE_ONKEYDOWN   0x105  // Send when key pressed 
									    // param1 = param of tree item;
									    // param2 = pointer to NMTVKEYDOWN
									    // param3 = tree hwnd
									    // return 0 if it's not yours, 1 if it is

#define ML_MSG_TREE_END          0x1FF  // end of tree specific messages

#define ML_MSG_ONSENDTOBUILD     0x300 // you get sent this when the sendto menu gets built
// param1 = type of source, param2 param to pass as context to ML_IPC_ADDTOSENDTO
// be sure to return 0 to allow other plugins to add their context menus

// if your sendto item is selected, you will get this with your param3 == your user32 (preferably some
// unique identifier (like your plugin message proc). See ML_IPC_ADDTOSENDTO
#define ML_MSG_ONSENDTOSELECT    0x301
// param1 = type of source, param2 = data, param3 = user32

// param1 = write type - added 5.7
// this relates to IPC_WRITECONFIG (see wa_ipc.h) for values sent via 'param1'
#define ML_MSG_WRITE_CONFIG      0x397

// param1 = wchar_t* of the file filepath of the file when playback begins - added 5.7
// this relates to IPC_PLAYING_FILEW (see wa_ipc.h) without the need to subclass Winamp
#define ML_MSG_PLAYING_FILE      0x398

// return TRUE to block the closing of Winamp (relates to IPC_HOOK_OKTOQUIT) - added 5.64+
#define ML_MSG_NOTOKTOQUIT       0x399

// return TRUE and do a config dialog using param1 as a HWND parent for this one
#define ML_MSG_CONFIG            0x400

// return TRUE to allow the plug-in config button to be disabled as applicable
#define ML_MSG_NO_CONFIG         0x401

// added from 5.64+ (internal usage, do not re-use this message range unless allowed)
#define ML_MSG_CLOUD_MIN         0x402
#define ML_MSG_CLOUD_MAX         0x410

// added from 5.66+ (used for some easier plug-in hooking of the views 
#define ML_MSG_VIEW_BUTTON_HOOK         0x420	// param1 = hwndDlg, param2 = MAKELONG(button_id_to_use, button_id_to_follow), param3 = plug-in id e.g. "ml_history". return text to show on the button
#define ML_MSG_VIEW_BUTTON_HOOK_IN_USE  0x421	// param1 = current 'play' / 'enqueue' mode
#define ML_MSG_VIEW_PLAY_ENQUEUE_CHANGE 0x422	// param1 = new state of 'play' / 'enqueue' mode, param2 = new state of 'button grouping mode'
												// these can be also read from gen_ml.ini and the [gen_ml_config] using 'enqueuedef' and 'groupbtn' respectively
												// note: you can use IPC_GETMLINIFILEW from wa_ipc.h to get the gen_ml.ini path
typedef struct
{
	wchar_t* play;
	wchar_t* enqueue;
} viewButtons;

#define ML_IPC_GET_VIEW_BUTTON_TEXT    0x423    // pass viewButtons as the param and gen_ml will fill in appropriate 'play' and 'enqueue' button text to use (saves duplicating translations)

#define ML_MSG_DOWNLOADS_VIEW_LOADED   0x424    // returns TRUE by ml_downloads.dll if loaded and it's view is enabled (used internally)
#define ML_MSG_DOWNLOADS_VIEW_POSITION 0x425	// param1 = parent param2 = 1 (always ensure param2 is set as 1)
// end of those added for 5.66

// types for drag and drop
#define ML_TYPE_UNKNOWN			-1
#define ML_TYPE_ITEMRECORDLIST   0 // if this, cast obj to itemRecordList
#define ML_TYPE_FILENAMES        1 // double NULL terminated char * to files, URLS, or playlists
#define ML_TYPE_STREAMNAMES      2 // double NULL terminated char * to URLS, or playlists ( effectively the same as ML_TYPE_FILENAMES, but not for files)
#define ML_TYPE_CDTRACKS         3 // if this, cast obj to itemRecordList (CD tracks) -- filenames should be cda://<drive letter>,<track index>. artist/album/title might be valid (CDDB)
#define ML_TYPE_QUERYSTRING      4 // char * to a query string
#define ML_TYPE_PLAYLIST         5 // mlPlaylist *
#define ML_TYPE_ITEMRECORDLISTW  6 // if this, cast obj to itemRecordListW
// added from 5.36+
#define ML_TYPE_FILENAMESW       7 // double NULL terminated wchar_t * to files, URLS, or playlists
#define ML_TYPE_STREAMNAMESW     8 // double NULL terminated wchar_t * to URLS, or playlists ( effectively the same as ML_TYPE_FILENAMESW, but not for files)
#define ML_TYPE_PLAYLISTS        9 // mlPlaylist **, null terminated
#define ML_TYPE_TREEITEM        69 // uhh?

typedef struct
{
	const wchar_t *filename;
	const wchar_t *title;
	// only fill in the following two if already know.  don't calculate it just for the struct. 
	// put -1 for "don't know"
	int numItems; 
	int length; // in seconds.  
} mlPlaylist;

// if you wish to put your tree items under "devices", use this constant for ML_IPC_ADDTREEITEM
#define ML_TREEVIEW_ID_DEVICES 10000

// children communicate back to the media library by SendMessageW(plugin.hwndLibraryParent,WM_ML_IPC,param,ML_IPC_X);
#define WM_ML_IPC WM_USER+0x1000

#define ML_IPC_GETCURRENTVIEW	0x090 // Returns HWND to the currently selected view or NULL if nothing selected or error. (WA 5.22 and higher)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Old Tree Item API  (deprecated)
//
#define ML_IPC_ADDTREEITEM		0x0101 // pass mlAddTreeItemStruct as the param
#define ML_IPC_SETTREEITEM		0x0102 // pass mlAddTreeItemStruct with id valid
#define ML_IPC_DELTREEITEM		0x0103 // pass param of tree item to remove
#define ML_IPC_GETCURTREEITEM	0x0104 // returns current tree item param or 0 if none
#define ML_IPC_SETCURTREEITEM	0x0105 // selects the tree item passed, returns 1 if found, 0 if not
#define ML_IPC_GETTREE			0x0106 // returns a HMENU with all the tree items. the caller needs to delete the returned handle! pass mlGetTreeStruct as the param
/*
** This will produce a menu of every item in the media library left pane.
** mlGetTreeStruct mgts = { 0, 45000, -1 };
** HMENU media_library_menu = (HMENU)SendMessageW(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&mgts,ML_IPC_GETTREE);
** if(media_library_menu){
**   // show or insert the menu as applicable
** }
**
** Whereas this will produce a menu just of the available media library playlists stored.
** mlGetTreeStruct mgts = { TREE_PLAYLISTS, 45000, -1 };
** HMENU playlists_menu = (HMENU)SendMessageW(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&mgts,ML_IPC_GETTREE);
** if(playlists_menu){
**   // show or insert the menu as applicable
** }
*/

/* deprecated. Use MLTREEITEM instead */
typedef struct
{
	INT_PTR  parent_id; //0=root, or ML_TREEVIEW_ID_*
	char    *title;
	int      has_children;
	INT_PTR  this_id;   //filled in by the library on ML_IPC_ADDTREEITEM
} mlAddTreeItemStruct;

#define TREE_LOCALMEDIA 1000
#define TREE_PLAYLISTS  3001
#define TREE_QUERIES    1000
typedef struct
{
	int item_start;   // TREE_PLAYLISTS,...
	int cmd_offset;   // menu command offset if you need to make a command proxy, 0 otherwise
	int max_numitems; // maximum number of items you wish to insert or -1 for no limit
} mlGetTreeStruct;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// For Predixis, with Love
/// deprecated (never use!!!)
///
#define ML_IPC_ADDTREEITEM_EX 0x0111 // pass mlAddTreeItemStructEx as the param
#define ML_IPC_SETTREEITEM_EX 0x0112 // pass mlAddTreeItemStructEx with this_id valid

typedef struct {
	INT_PTR parent_id; //0=root, or ML_TREEVIEW_ID_*
	char *title;
	int has_children;
	INT_PTR this_id; //filled in by the library on ML_IPC_ADDTREEITEM
	int imageIndex;  // index of the image you want to be associated with your item
} mlAddTreeItemStructEx;

/// deprecatded (never use!!!)
#define ML_IPC_ADDTREEIMAGE 0x110 // adds tree image to the ml. Use mlAddTreeImageStruct as the param.
/// end of predixis special

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tree Item API  (starting from 5.3)
//
#define ML_IPC_TREEITEM_GETHANDLE	0x120 // Gives you HANDLE to the item with specified ID in the param
#define ML_IPC_TREEITEM_GETCHILD	0x121 // Returns HANDLE to the child item for the item HANDLE specified as a param. 
#define ML_IPC_TREEITEM_GETNEXT		0x122 // Returns HANDLE to the next item for the item HANDLE specified as a param.
#define ML_IPC_TREEITEM_GETSELECTED	0x123 // Returns HANDLE to selected item.
#define ML_IPC_TREEITEM_GETINFO		0x124 // Pass MLTREEITEMINFO as a param. return TRUE - if ok
#define ML_IPC_TREEITEM_SETINFO		0x125 // Pass MLTREEITEMINFO as a param. return TRUE - if ok
#define ML_IPC_TREEITEM_ADD			0x126 // Adds new item using MLTREEITEM passed as a param
#define ML_IPC_TREEITEM_DELETE		0x127 // Deletes tree item. Pass HANDLE as a param.
#define ML_IPC_TREEITEM_SELECT		0x128 // Selects tree item. Pass HANDLE as a param. 
#define ML_IPC_TREEITEM_GETROOT		0x129 // Gets first item.
#define ML_IPC_TREEITEM_INSERT		0x130 // like ML_IPC_TREEITEM_ADD, but id becomes an "insert after" ID
#define ML_IPC_TREEITEM_GETCHILD_ID	0x131 // Returns ID to the child item for the item ID specified as a param. 
#define ML_IPC_TREEITEM_GETNEXT_ID	0x132 // Returns ID to the next item for the item ID specified as a param.
#define ML_IPC_TREEITEM_ADDW		0x133 // Adds new item using MLTREEITEMW passed as a param
#define ML_IPC_TREEITEM_INSERTW		0x134 // like ML_IPC_TREEITEM_ADDW, but id becomes an "insert after" ID
#define ML_IPC_TREEITEM_SETINFOW	0x135 // Pass MLTREEITEMINFOW as a param. return TRUE - if ok
#define ML_IPC_TREEITEM_GETINFOW	0x136 // Pass MLTREEITEMINFO as a param. return TRUE - if ok
#define MLTI_ROOT			(INT_PTR)TVI_ROOT // can be used in ML_IPC_TREEITEM_GETCHILD

typedef struct {
	size_t size;		// size of this struct
	UINT_PTR id;		// depends on contxext
	UINT_PTR parentId;	// 0 = root, or ML_TREEVIEW_ID_*
	char *title;		// pointer to the buffer contained item name. 
	size_t titleLen;	// used for GetInfo 
	BOOL hasChildren;	// TRUE - has children
	int imageIndex;		// index of the associated image
} MLTREEITEM;

typedef struct {
	MLTREEITEM item;	// item data
	UINT mask;			// one or more of MLTI_* flags
	UINT_PTR handle;	// Handle to the item. If handle is NULL item->id will be used
} MLTREEITEMINFO;

typedef struct {
	size_t size;		// size of this struct
	UINT_PTR id;		// depends on context
	UINT_PTR parentId;	// 0 = root, or ML_TREEVIEW_ID_*
	wchar_t *title;		// pointer to the buffer contained item name. 
	size_t titleLen;	// used for GetInfo 
	BOOL hasChildren;	// TRUE - has children
	int imageIndex;		// index of the associated image
} MLTREEITEMW;

typedef struct {
  MLTREEITEMW item;		// item data
  UINT mask;			// one or more of MLTI_* flags
  UINT_PTR handle;		// Handle to the item. If handle is NULL item->id will be used
} MLTREEITEMINFOW;

// Flags that used in the MLTREEITEMINFO struct
#define MLTI_CHILDREN	TVIF_CHILDREN
#define MLTI_IMAGE		TVIF_IMAGE
#define MLTI_TEXT		TVIF_TEXT
#define MLTI_ID			TVIF_PARAM

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tree image (starting from 5.3)
//
#define ML_IPC_TREEIMAGE_ADD 0x140 // adds tree image to the ml. Use MLTREEIMAGE as the param.

typedef struct _COLOR24
{
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
}COLOR24; // color struct 

typedef void (*BMPFILTERPROC)(const COLOR24*, const COLOR24*, COLOR24*); // color filter procedure 
// you got two colors: color1 and color2 (usualy BG and FG colors) also you as a third parameter
// you get a pixel color value that you need (can) modify  

#define FILTER_NO			((BMPFILTERPROC)NULL)
#define FILTER_DEFAULT1		((BMPFILTERPROC)1)
#define FILTER_DEFAULT2		((BMPFILTERPROC)2)

#define MLTREEIMAGE_NONE					0
#define MLTREEIMAGE_DEFAULT					1
#define MLTREEIMAGE_BRANCH					2   // calculates at the time 	
#define MLTREEIMAGE_BRANCH_EXPANDED			3
#define MLTREEIMAGE_BRANCH_COLLAPSED		4
#define MLTREEIMAGE_BRANCH_NOCHILD			5

typedef struct {
	HINSTANCE hinst;			// hInstance
	int resourceId;				// resource id 
	int imageIndex;				// set image to specified index (specify -1 to get a new index back)
	BMPFILTERPROC filterProc;	// pointer to the filter proc to use or one of the FILTER_*
	int width;					// reserved
	int height;					// reserved
} MLTREEIMAGE;	// basicly ml will read your reosurce when it will need to create your image

#define ML_IPC_NEWPLAYLIST           0x107 // pass hwnd for dialog parent as param
#define ML_IPC_IMPORTPLAYLIST        0x108 // pass hwnd for dialog parent as param
#define ML_IPC_IMPORTCURRENTPLAYLIST 0x109
#define ML_IPC_GETPLAYLISTWND        0x10A
#define ML_IPC_SAVEPLAYLIST          0x10B // pass hwnd for dialog parent as param
#define ML_IPC_OPENPREFS             0x10C

#define ML_IPC_PLAY_PLAYLIST 0x010D // plays the playlist pointed to by the tree item passed, returns 1 if found, 0 if not
#define ML_IPC_LOAD_PLAYLIST 0x010E // loads the playlist pointed to by the tree item passed into the playlist editor, returns 1 if found, 0 if not

#define ML_IPC_REFRESH_PREFS 0x10F // this doesn't belong in here 

/** ------------------ 
 ** ml_playlists 
 ** ------------------ */

#define PL_FLAG_SHOW          1
#define PL_FLAG_SWITCH        2
#define PL_FLAGS_IMPORT       4 // set to have ml_playlists make a copy (only valid for mlAddPlaylist)
#define PL_FLAG_FILL_FILENAME 8 // only valid for mlMakePlaylist

typedef struct 
{
	size_t size;  // size of this struct
	const wchar_t *playlistName; // set to NULL (or empty string) to prompt the user for a name
	const wchar_t *filename;
	int flags; // see PL_FLAG_* above
	// the following two items can be optionally filled in (set to -1 otherwise)
	// if they aren't set, the playlist file will have to be opened and parsed
	// so prepopulating is faster (assuming if you already know the data)
	int numItems; // set to -1 if you don't know. 
	int length; // in seconds, set to -1 if you don't know
} mlAddPlaylist;

#define ML_IPC_PLAYLIST_ADD 0x180 // call to add a new playlist file to the Playlists treeview.  pass an mlAddPlaylist *

typedef struct 
{
	size_t size;  // size of this struct
	const wchar_t *playlistName; // set to NULL (or empty string) to prompt the user for a name
	int type; //ML_TYPE_ITEMRECORDLIST, etc
	void *data; // object to load
	int flags; // see PL_FLAG_* above
	wchar_t filename[MAX_PATH]; // this will get populated if PL_FLAG_FILL_NAME is set
} mlMakePlaylistV2;

// old structure, here to make it easy to do a sizeof() check
typedef struct 
{
	size_t size;  // size of this struct
	const wchar_t *playlistName; // set to NULL (or empty string) to prompt the user for a name
	int type; //ML_TYPE_ITEMRECORDLIST, etc
	void *data; // object to load
	int flags; // see PL_FLAG_* above
} mlMakePlaylist;

/* Call to add a new playlist to the Playlists treeview.  
   It will be automatically created based on the data you pass
	 type & data follow the same specifications as send-to, drag-and-drop, etc.
*/
#define ML_IPC_PLAYLIST_MAKE  0x181  // call to create a new playlist in the treeview based on passed information.  pass an mlMakePlaylist *
#define ML_IPC_PLAYLIST_COUNT 0x182
#define ML_IPC_PLAYLIST_INFO  0x183  // pass in the struct below. returns "1" on success and "0" on failure

typedef struct
{
	// you fill this in
	size_t size; // size of this struct
	size_t playlistNum; // number of the playlist you want to retrieve (0 index)
	// ml_playlists fills these in
	wchar_t playlistName[128];
	wchar_t filename[MAX_PATH];
	int numItems;
	int length; // in seconds
} mlPlaylistInfo;

/** ------------------ 
 ** 
 ** ------------------ */

#define ML_IPC_GETFILEINFO   0x0200 // pass it a &itemRecord with a valid filename (and all other fields NULL), and it will try to fill in the rest
#define ML_IPC_FREEFILEINFO  0x0201 // pass it a &itemRecord that was filled by getfileinfo, it will free the strings it allocated
// added for 5.58+
#define ML_IPC_GETFILEINFOW  0x0202 // pass it a &itemRecordW with a valid filename (and all other fields NULL), and it will try to fill in the rest
#define ML_IPC_FREEFILEINFOW 0x0203 // pass it a &itemRecordW that was filled by getfileinfo, it will free the strings it allocated

#define ML_IPC_HANDLEDRAG    0x0300 // pass it a &mlDropItemStruct it will handle cursors etc (unless flags has the lowest bit set), and it will set result appropriately:
#define ML_IPC_HANDLEDROP    0x0301 // pass it a &mlDropItemStruct with data on drop:

#define ML_IPC_SENDTOWINAMP  0x0302 // send with a mlSendToWinampStruct:
typedef struct {
  int type; //ML_TYPE_ITEMRECORDLIST, etc
  void *data; // object to play

  int enqueue; // low bit set specifies enqueuing, and second bit NOT set specifies that 
               // the media library should use its default behavior as the user configured it (if 
               // enqueue is the default, the low bit will be flipped by the library)
} mlSendToWinampStruct;

typedef struct {
	char *desc;			// str (addition 5.64+ - set as L"-" to create a separator and begin with a # to show as grayed)
	intptr_t context;	// context passed by ML_MSG_ONSENDTOBUILD
	intptr_t user32;	// use some unique ptr in memory, you will get it back in ML_MSG_ONSENDTOSELECT...
} mlAddToSendToStruct;
#define ML_IPC_ADDTOSENDTO   0x0400

typedef struct {
	wchar_t *desc;		// str (addition 5.64+ - set as L"-" to create a separator and begin with a # to show as grayed)
	intptr_t context;	// context passed by ML_MSG_ONSENDTOBUILD
	intptr_t user32;	// use some unique ptr in memory, you will get it back in ML_MSG_ONSENDTOSELECT...
} mlAddToSendToStructW;
#define ML_IPC_ADDTOSENDTOW  0x0401 // pass mlAddToSendToStructW

// used to make a submenu in sendto
// pass mlAddToSendToStructW, set desc to 0 to start, set valid desc to end
// user32 is unused
// note: the 5.64+ additions for separators and greyed items do not apply to the branch node
#define ML_IPC_BRANCHSENDTO  0x0402 

// follow same rules as ML_IPC_ADDTOSENDTOW, but adds to branch instead of main send-to menu
#define ML_IPC_ADDTOBRANCH   0x0403  // pass mlAddToSendToStructW

#define ML_IPC_HOOKTITLE     0x0440 // this is like winamp's IPC_HOOK_TITLES... :) param1 is waHookTitleStruct
#define ML_IPC_HOOKEXTINFO   0x0441 // called on IPC_GET_EXTENDED_FILE_INFO_HOOKABLE, param1 is extendedFileInfoStruct
#define ML_IPC_HOOKEXTINFOW  0x0442 // called on IPC_GET_EXTENDED_FILE_INFO_HOOKABLEW, param1 is extendedFileInfoStructW
#define ML_IPC_HOOKTITLEW    0x0443 // this is like winamp's IPC_HOOK_TITLESW... :) param1 is waHookTitleStructW

#define ML_HANDLEDRAG_FLAG_NOCURSOR 1
#define ML_HANDLEDRAG_FLAG_NAME     2

typedef struct {
	int type; //ML_TYPE_ITEMRECORDLIST, etc
	void *data; // NULL if just querying

	int result; // filled in by client: -1 if dont allow, 0 if dont know, 1 if allow.

	POINT p; // cursor pos in screen coordinates
	int flags; 

	char *name; // only valid if ML_HANDLEDRAG_FLAG_NAME
} mlDropItemStruct;

#define ML_IPC_SKIN_LISTVIEW            0x0500 // pass the hwnd of your listview. returns a handle to use with ML_IPC_UNSKIN_LISTVIEW
#define ML_IPC_UNSKIN_LISTVIEW          0x0501 // pass the handle you got from ML_IPC_SKIN_LISTVIEW
#define ML_IPC_LISTVIEW_UPDATE          0x0502 // pass the handle you got from ML_IPC_SKIN_LISTVIEW
#define ML_IPC_LISTVIEW_DISABLEHSCROLL  0x0503 // pass the handle you got from ML_IPC_SKIN_LISTVIEW
#define ML_IPC_LISTVIEW_DISABLEVSCROLL  0x050A // pass the handle you got from ML_IPC_SKIN_LISTVIEW
#define ML_IPC_LISTVIEW_SHOWSORT		0x0504 // use LV_SKIN_SHOWSORT
#define ML_IPC_LISTVIEW_SORT			0x0505 // use LV_SKIN_SORT

typedef struct 
{
	INT_PTR listView;
	BOOL showSort;
} LV_SKIN_SHOWSORT;

typedef struct
{
	INT_PTR listView;
	int columnIndex;
	BOOL ascending;
} LV_SKIN_SORT;


#define ML_IPC_SKIN_COMBOBOX      0x0508 // pass the hwnd of your combobox to skin, returns a ahndle to use with ML_IPC_UNSKIN_COMBOBOX
#define ML_IPC_UNSKIN_COMBOBOX    0x0509 // pass the handle from ML_IPC_SKIN_COMBOBOX

#define ML_IPC_SKIN_WADLG_GETFUNC 0x0600 
    // 1: return int (*WADlg_getColor)(int idx); // pass this an index, returns a RGB value (passing 0 or > 3 returns NULL)
    // 2: return int (*WADlg_handleDialogMsgs)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 
    // 3: return void (*WADlg_DrawChildWindowBorders)(HWND hwndDlg, int *tab, int tabsize); // each entry in tab would be the id | DCW_*
	// 4: return HBITMAP (*WADlg_getBitmap)(); // a copy of the current skin's genex.bmp
    // 32: return void (*childresize_init)(HWND hwndDlg, ChildWndResizeItem *list, int num);
    // 33: return void (*childresize_resize)(HWND hwndDlg, ChildWndResizeItem *list, int num);
    // 66: return (HFONT) font to use for dialog elements, if desired (0 otherwise)
   
// itemRecord type for use with ML_TYPE_ITEMRECORDLIST, as well as many other functions
typedef struct
{
	char *filename;
	char *title;
	char *ext;
	char *album;
	char *artist;
	char *comment;
	char *genre;
	int   year;
	int   track;
	int   length;
	char **extended_info;
	// currently defined extended columns (while they are stored internally as integers
	// they are passed using extended_info as strings):
	// use getRecordExtendedItem and setRecordExtendedItem to get/set.
	// for your own internal use, you can set other things, but the following values
	// are what we use at the moment. Note that setting other things will be ignored
	// by ML_IPC_DB*.
	// 
	//"RATING" file rating. can be 1-5, or 0 or empty for undefined
	//"PLAYCOUNT" number of file plays.
	//"LASTPLAY" last time played, in standard time_t format
	//"LASTUPD" last time updated in library, in standard time_t format
	//"FILETIME" last known file time of file, in standard time_t format
	//"FILESIZE" last known file size, in kilobytes.
	//"BITRATE" file bitrate, in kbps
	//"TYPE" - "0" for audio, "1" for video
	// TODO list all of the other values available for completeness between this and itemRecordW
	//"ISPODCAST",
	//"PODCASTCHANNEL",
	//"PODCASTPUBDATE",
	//"LOSSLESS",
	//"CODEC",
	//"DIRECTOR",
	//"PRODUCER",
	//"WIDTH",
	//"HEIGHT",
	//"MIME",
	// TODO END
	//"REALSIZE" last known file size, in bytes (provides a 64-bit integer as a string due to 'filesize being limited to 32-bit).
	//"DATEADDED" time the file was added to the library, in standard time_t format
} itemRecord;

typedef struct 
{
	itemRecord *Items;
	int Size;
	int Alloc;
} itemRecordList;

#include <time.h>

typedef struct
{
	wchar_t *key;
	wchar_t *value;
} extendedInfoW;

/* sizeof(itemRecordW) should be 128 (on x86).  If you use a compiler other than 
Microsoft Visual Studio, it's worth checking to make sure it's the same */
typedef struct
{
	wchar_t   *filename;
	wchar_t   *title;
	wchar_t   *ext;
	wchar_t   *album;
	wchar_t   *artist;
	wchar_t   *comment;
	wchar_t   *genre;
	wchar_t   *albumartist; 
	wchar_t   *replaygain_album_gain; // these are strings rather than float's to differentiate between '0 gain' and 'not defined'
	wchar_t   *replaygain_track_gain; // these are strings rather than float's to differentiate between '0 gain' and 'not defined'
	wchar_t   *publisher;
	wchar_t   *composer;
	int        year;
	int        track;
	int        tracks;
	int        length;
	int        rating;     // file rating. can be 1-5, or 0 for undefined
	int        playcount;  // number of file plays.
	__time64_t lastplay;   // last time played, in standard time_t format
	__time64_t lastupd;    // last time updated in library, in standard time_t format
	__time64_t filetime;   // last known file time of file, in standard time_t format
	int        filesize;   // last known file size, in kilobytes.
	int        bitrate;    // file bitrate, in kbps
	int        type;       // 0 for audio, 1 for video
	int        disc;       // disc number
	int        discs;      // number of discs
	int        bpm;
	extendedInfoW *extended_info;
	// currently defined extended columns (while they are stored internally as integers
	// they are passed using extended_info as strings):
	// use getRecordExtendedItem and setRecordExtendedItem to get/set.
	// for your own internal use, you can set other things, but the following values
	// are what we use at the moment. Note that setting other things will be ignored
	// by ML_IPC_DB*.
	// 
	wchar_t *category; // 5.61+: we were able to sneak this in because of padding.  it'll be zero on older versions because the structs get memset
} itemRecordW;

typedef struct
{
	itemRecordW *Items;
	int Size;
	int Alloc;
} itemRecordListW;

//
// all return 1 on success, -1 on error. or 0 if not supported, maybe?

// pass these a mlQueryStruct
// results should be zeroed out before running a query, but if you wish you can run multiple queries and 
// have it concatenate the results. tho it would be more efficient to just make one query that contains both,
// as running multiple queries might have duplicates etc.
// in general, though, if you need to treat "results" as if they are native, you should use
// copyRecordList to save a copy, then free the results using ML_IPC_DB_FREEQUERYRESULTS.
// if you need to keep an exact copy that you will only read (and will not modify), then you can
// use the one in the mlQueryStruct.

#define ML_IPC_DB_RUNQUERY          0x0700 
#define ML_IPC_DB_RUNQUERY_SEARCH   0x0701 // "query" should be interpreted as keyword search instead of query string
#define ML_IPC_DB_RUNQUERY_FILENAME 0x0702 // searches for one exact filename match of "query"
#define ML_IPC_DB_RUNQUERY_INDEX    0x0703 // retrieves item #(int)query [deprecated and doesn't work]

#define ML_IPC_DB_FREEQUERYRESULTS  0x0705 // frees memory allocated by ML_IPC_RUNQUERY (empties results)
typedef struct 
{
	char *query;
	int max_results;      // can be 0 for unlimited
	itemRecordList results;
} mlQueryStruct;

/* Unicode versions of the above */

#define ML_IPC_DB_RUNQUERYW          0x1700
#define ML_IPC_DB_RUNQUERY_SEARCHW   0x1701 // "query" should be interpreted as keyword search instead of query string
#define ML_IPC_DB_RUNQUERY_FILENAMEW 0x1702 // searches for one exact filename match of "query"
#define ML_IPC_DB_RUNQUERY_INDEXW    0x1703 // retrieves item #(int)query [deprecated and doesn't work]
#define ML_IPC_DB_FREEQUERYRESULTSW  0x1705 // frees memory allocated by ML_IPC_RUNQUERYW (empties results)
typedef struct 
{
	wchar_t *query;
	int max_results;      // can be 0 for unlimited
	itemRecordListW results;
} mlQueryStructW;

/* ----------------------------- */

// pass these an (itemRecord *) to add/update.
// note that any NULL fields in the itemRecord won't be updated, 
// and year, track, or length of -1 prevents updating as well.
#define ML_IPC_DB_UPDATEITEM      0x0706    // returns -2 if item not found in db
#define ML_IPC_DB_ADDORUPDATEITEM 0x0707

#define ML_IPC_DB_REMOVEITEM      0x0708 // pass a char * to the filename to remove. returns -2 if file not found in db.
// added in v5.58
#define ML_IPC_DB_REMOVEITEMW     0x1708 // pass a wchar_t * to the filename to remove. returns -2 if file not found in db.

typedef struct 
{
	char*  fileName;   // file name to add
	int    meta_mode;  // metadata get mode (0 - don't use metadata, 1 - use metadata; -1 - read from user settings (ini file)
	int    gues_mode;  // metadata guessing mode (0 - smart, 1 - simple; 2 - no, -1 - read from user settings (ini file)
} LMDB_FILE_ADD_INFO;

#define ML_IPC_DB_UPDATEFILE      0x0710      // Update File in the Local Media Data Base (return -2 if file record not found)
#define ML_IPC_DB_ADDORUPDATEFILE 0x0711      // Adds or Updates File in the Local Media Data Base.

/* Unicode versions of the above */

// pass these an (itemRecordW *) to add/update.
// note that any NULL fields in the itemRecordW won't be updated, 
// and year, track, or length of -1 prevents updating as well.
#define ML_IPC_DB_UPDATEITEMW      0x1706    // returns -2 if item not found in db
#define ML_IPC_DB_ADDORUPDATEITEMW 0x1707

typedef struct 
{
	wchar_t*  fileName;   // file name to add
	int    meta_mode;  // metadata get mode (0 - don't use metadata, 1 - use metadata; -1 - read from user settings (ini file)
	int    gues_mode;  // metadata guessing mode (0 - smart, 1 - simple; 2 - no, -1 - read from user settings (ini file)
} LMDB_FILE_ADD_INFOW;

#define ML_IPC_DB_UPDATEFILEW      0x1710      // Update File in the Local Media Data Base (return -2 if file record not found) NOTE that this call is broken on 5.33.  Only use on 5.34+
#define ML_IPC_DB_ADDORUPDATEFILEW 0x1711      // Adds or Updates File in the Local Media Data Base.

/* ----------------------------- */

#define ML_IPC_DB_SYNCDB           0x0709      // sync db if dirty flags are set. good to do after a batch of updates.

// these return 0 if unsupported, -1 if failed, 1 if succeeded

// pass a winampMediaLibraryPlugin *. Will not call plugin's init() func. 
// YOU MUST set winampMediaLibraryPlugin->hDllInstance to NULL, and version to MLHDR_VER
// 5.25+:  You can set hDllInstance to valid value.  
//         This IPC will return -1 on failure, so a good check against old verions
//         is to try with hDllInstance set, if it returns -1, try again with hDllInstance=0
#define ML_IPC_ADD_PLUGIN          0x0750 
#define ML_IPC_REMOVE_PLUGIN       0x0751 // winampMediaLibraryPlugin * of plugin to remove. Will not call plugin's quit() func

#define ML_IPC_SEND_PLUGIN_MESSAGE 0x0752 // sends message to plugins (wParam = 0, lParam = pointer to the pluginMessage struct)
// pluginMessage struct
typedef struct {
	int     messageType;
	INT_PTR param1;
	INT_PTR param2;
	INT_PTR param3;
} pluginMessage;

#define ML_IPC_ENSURE_VISIBLE      0x0753 // ensures that the media library is visible
#define ML_IPC_IS_VISIBLE          0x0754 // queries the current visibility status

#define ML_IPC_GET_PARENTAL_RATING 0x0755 

#define ML_IPC_TOGGLE_VISIBLE      0x0756

// 5.58+ to fix modern skin focusing issue
#define ML_IPC_FOCUS_TREE          0x0757

/* start of ML_CHILDIPC_* section */

// this gets sent to any child windows of the library windows, and then (if not handled) the library window itself
#define WM_ML_CHILDIPC WM_APP+       0x800 // avoids conflicts with any windows controls

// lParam = 0x100, wParam = &mlDropItemStruct
#define ML_CHILDIPC_DROPITEM         0x100

// notification only message sent via WM_ML_CHILDIPC when F6 (or the accelerator defined against id 40157)
// is pressed - use this to focus to the searchbar in the view if you have one (usability change for 5.55+)
// lParam == 0x200, wParam = 0
#define ML_CHILDIPC_GO_TO_SEARCHBAR 0x200

// added 5.666+
// notification only message sent via WM_ML_CHILDIPC when F8 (or the accelerator defined against id 40165)
// is pressed - use this to re-run the current search or to act as a way to refresh the current view results
// lParam == 0x300, wParam = 0
#define ML_CHILDIPC_REFRESH_SEARCH  0x300

/* end of ML_CHILDIPC_* section */


// current item ratings
#define ML_IPC_SETRATING 0x0900 // lParam = 0 to 5, rates current track -- inserts it in the db if it's not in it yet 
#define ML_IPC_GETRATING 0x0901 // return the current track's rating or 0 if not in db/no rating

// playlist entry rating
typedef struct {
	int plentry;
	int rating;
} pl_set_rating;

#define ML_IPC_PL_SETRATING 0x0902   // lParam = pointer to pl_set_rating struct
#define ML_IPC_PL_GETRATING 0x0903   // lParam = playlist entry, returns the rating or 0 if not in db/no rating

typedef struct {
	HWND dialog_parent;              // Use this window as a parent for the query editor dialog
	const char *query;               // The query to edit, or "" / null for new query
} ml_editquery;                 

#define ML_IPC_EDITQUERY    0x0904   // lParam = pointer to ml_editquery struct, returns 0 if edition was canceled and 1 on success
                                     // After returning, and if ok was clicked, the struct contains a pointer to the edited query. this pointer is static : 
                                     // - do *not* free it
                                     // - if you need to keep it around, _strdup it, as it may be changed later by other plugins calling ML_IPC_EDITQUERY.

typedef struct {                
	HWND dialog_parent;              // Use this window as a parent for the view editor dialog
	const char *query;               // The query to edit, or "" / null for new views
	const char *name;                // Name of the view (ignored for new views)
	int mode;                        // View mode (0=simple view, 1=artist/album view, -1=hide mode radio boxes)
} ml_editview;                 

#define ML_IPC_EDITVIEW     0x0905  // lParam = pointer to ml_editview struct, returns 0 if edition was canceled and 1 on success
                                    // After returning, and if ok was clicked, the struct contains the edited values. String pointers are static: 
                                    // - do *not* free them 
                                    // - if you need to keep them around, _strdup them, as they may be changed later by other plugins calling ML_IPC_EDITQUERY.

#define ML_IPC_SET_FILE_RATING 0x0906 // lParam = 0 to 5, rates current track -- inserts it in the db if it's not in it yet 
#define ML_IPC_GET_FILE_RATING 0x0907 // return the current track's rating or 0 if not in db/no rating

// playlist entry rating
typedef struct {
	const char* fileName;
	int newRating;
} file_set_rating;

#define ML_IPC_SMARTVIEW_COUNT 0x0908   // returns the number of smartviews. no parameter required
#define ML_IPC_SMARTVIEW_INFO  0x0909   // pass a mlSmartViewInfo*. returns 1 on success and 0 on failure
#define ML_IPC_SMARTVIEW_ADD   0x0910   // pass a mlSmartViewInfo* with filled in size, name, query, mode, iconImgIndex. treeitemid gets filled in. returns 1 on success and 0 on failure

typedef struct
{
	// you fill these in
	size_t  size;  // set to sizeof(mlSmartViewInfo)
	size_t  smartViewNum;
	// ml_local fills these in
	wchar_t smartViewName[128];
	wchar_t smartViewQuery[512];
	int     mode;
	int     iconImgIndex;
	int     treeItemId;
} mlSmartViewInfo;


#define ML_IPC_SET_FILE_RATINGW 0x0911 // lParam = 0 to 5, rates current track -- inserts it in the db if it's not in it yet 
#define ML_IPC_GET_FILE_RATINGW 0x0912 // return the current track's rating or 0 if not in db/no rating

// playlist entry rating
typedef struct {
	const wchar_t *fileName;
	int newRating;
} file_set_ratingW;


// utility functions in ml_lib.cpp
void freeRecordList(itemRecordList *obj);
void emptyRecordList(itemRecordList *obj); // does not free Items
void freeRecord(itemRecord *p);

// if out has items in it copyRecordList will append "in" to "out".
void copyRecordList(itemRecordList *out, const itemRecordList *in);
//copies a record
void copyRecord(itemRecord *out, const itemRecord *in);

void allocRecordList(itemRecordList *obj, int newsize, int granularity
#ifdef __cplusplus
=1024
#endif
);

char *getRecordExtendedItem(const itemRecord *item, const char *name);
void setRecordExtendedItem(itemRecord *item, const char *name, char *value);

#ifdef __cplusplus
// utility functions for itemRecordListW
void freeRecordList(itemRecordListW *obj);
void emptyRecordList(itemRecordListW *obj); // does not free Items
void freeRecord(itemRecordW *p);

wchar_t *getRecordExtendedItem(const itemRecordW *item, const wchar_t *name);
void setRecordExtendedItem(itemRecordW *item, const wchar_t *name, const wchar_t *value);
void copyRecordList(itemRecordListW *out, const itemRecordListW *in);
void copyRecord(itemRecordW *out, const itemRecordW *in);
void allocRecordList(itemRecordListW *obj, int newsize, int granularity=1024);

void convertRecord(itemRecord *output, const itemRecordW *input);
void convertRecord(itemRecordW *output, const itemRecord *input);
void convertRecordList(itemRecordList *output, const itemRecordListW *input);
void convertRecordList(itemRecordListW *output, const itemRecordList *input);
#endif

#define ML_IPC_GRACENOTE         0x1000
#define GRACENOTE_TUID           1
#define GRACENOTE_IS_WORKING     2
#define GRACENOTE_DO_TIMER_STUFF 3
#define GRACENOTE_CANCEL_REQUEST 4

#define ML_IPC_FLICKERFIX	0x1002		/// param = (FLICKERFIX*)ffix; Returns 1 if succesfull. if window already treated will update flag data

#define FFM_ERASE			0x00000000	// flicker treatment will be removed
#define FFM_FORCEERASE		0x01000000	// erase backgrund with speicfied color (WM_ERASEBKGND will fill hdc with color and return 1). Use mode = FFM_MODE | RGB(0,0,0)
#define FFM_NOERASE			0x02000000	// block erase operation ( WM_ERASEBKGND will return 1);
#define FFM_ERASEINPAINT	0x04000000	// forward erase operation to WM_PAINT ( WM_ERASEBKGND will return 0);

typedef struct _FLICKERFIX
{
	HWND		hwnd;		// target hwnd 
	DWORD	mode;		// FFM_XXX; 
}FLICKERFIX, *PFLICKERFIX;

// resource id of the drag & drop cursor used by the ml plugins
// you can use the following to get the cursor rather than rebundling the same icon
// hDragNDropCursor = LoadCursor(GetModuleHandle("gen_ml.dll"), MAKEINTRESOURCE(ML_IDC_DRAGDROP));
#define ML_IDC_DRAGDROP 107


#endif//_ML_H_