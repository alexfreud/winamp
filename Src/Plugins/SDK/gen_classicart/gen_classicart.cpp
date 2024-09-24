/* gen_classicart
  version 0.5, February 7th, 2010
  version 0.4, February 4th, 2010
  version 0.3, November 9th, 2009
  version 0.2, February 27th, 2008

  Copyright (C) 2008 Will Fisher

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Will Fisher will.fisher@gmail.com

  ---------------------------------------------------------------------------

  Changes from v0.2 by Darren Owen aka DrO

  * Added Alt+A global shortcut to toggle the album art window
  * Album art window is now included in Winamp's ctrl+tab feature (broken in some 5.5x builds)
  * Double-click will now open the folder (like the native album art window does)
  * Changing menu id to be dynamic rather than hard-coded to show use of IPC_REGISTER_LOWORD_COMMAND
  * Made background colour match the media library album art views (override with waBkClr=1 in the plugin's ini section)
  * Made compatible to deal with Winamp being minimised on startup so the window won't appear briefly
  * Implemented winampUninstallPlugin(..) support

  ---------------------------------------------------------------------------

  Changes from v0.3 by Darren Owen aka DrO

  * Added localisation support (with example en-us language file included in the installer)
  * Adding an option to show the album art when clicking on an item in the playlist editor without having to play it
  * Fixed the main menu item not working via the system menu
  * Plugin will now only work on 5.53+ clients to ensure the Alt+A global shortcut will work (bug-fix for v0.3)
  * Fixed issue with api.h not containing all of the required headers (only affects compiling source code)

  ---------------------------------------------------------------------------

  Changes from v0.4 by Darren Owen aka DrO

  * Added detection of being used under a modern skin so the album art window and menu item can be automatically hidden (default: on)
  * Added in ability for the tracking action to work via the keyboard actions along with button clicking
  * Tweaked the menu item text so the items (at least with en-us translations) is better consistant with the rest of Winamp (if that's possible, heh)
  * Added config menu into the Winamp preferences so it's possible to access the options when the album art window isn't shown

*/

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>

#include "../winamp/gen.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/ipc_pe.h"
#define WA_DLG_IMPLEMENT
#include "../winamp/wa_dlg.h"

#include "api.h"
#include "resource.h"

#define ID_PE_SCUP   40289
#define ID_PE_SCDOWN 40290

#define PLUGIN_NAME "Album Art Viewer"
#define PLUGIN_VERSION "0.6"

int init();
void quit();
void config();

winampGeneralPurposePlugin plugin =
{
	GPPHDR_VER,
	"nullsoft(gen_classicart.dll)",
	init,
	config,
	quit,
};

// winampGeneralPurposePlugin plugin = {GPPHDR_VER,PLUGIN_NAME" v"PLUGIN_VERSION,init,config,quit};

static api_memmgr* WASABI_API_MEMMGR;
static api_service* WASABI_API_SVC;
static api_albumart* AGAVE_API_ALBUMART;
static api_application* WASABI_API_APP;
static api_language* WASABI_API_LNG;
HINSTANCE WASABI_API_LNG_HINST = 0,
		  WASABI_API_ORIG_HINST = 0;

embedWindowState myWndState={0};
HWND myWnd=NULL,
     myWndChild=NULL;
HMENU menu=NULL,
      context_menu=NULL;
WNDPROC oldWndProc=NULL,
        oldPlaylistWndProc=NULL;
BOOL no_uninstall=TRUE,
	 modernloaded=FALSE,
	 last_artwindow_open=FALSE;
int artwindow_open=0,
    lockAspect=1,
    autoHide=0,
    waBkClr=0,
	on_click=0,
	clickTrack=0,
	hidemodern=1;
UINT WINAMP_ARTVIEW_MENUID=0xa1ba;
char* INI_FILE=NULL;
RECT lastWnd={0};
HDC cacheDC = NULL;
COLORREF bgcolour = RGB(0,0,0);

static INT_PTR CALLBACK art_dlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT WINAPI SubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT WINAPI SubclassPlaylistProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void viewArtWindow(BOOL show);
void addAccelerators(HWND hwnd, ACCEL* accel, int accel_size, int translate_mode);

// this is used to identify the skinned frame to allow for embedding/control by modern skins if needed
// {8B9052B2-2782-4ac8-BA8E-E3DEDBF0BDB5}
static const GUID ArtViewerGUID = 
{ 0x8b9052b2, 0x2782, 0x4ac8, { 0xba, 0x8e, 0xe3, 0xde, 0xdb, 0xf0, 0xbd, 0xb5 } };

// this is used to identify the language dll to be used when this is running under a language pack
// {EAD1E933-6D75-4c2c-B9C4-B4D7F06B7D8D}
static const GUID GenClassicArtGUID = 
{ 0xead1e933, 0x6d75, 0x4c2c, { 0xb9, 0xc4, 0xb4, 0xd7, 0xf0, 0x6b, 0x7d, 0x8d } };

BOOL ArtView_SetMinimised(BOOL fMinimized)
{
	if(fMinimized == TRUE)
	{
		return SetPropW(myWnd,L"AAMinMode",(HANDLE)TRUE);
	}
	RemovePropW(myWnd,L"AAMinMode");
	return TRUE;
}

UINT ver = -1;
UINT GetWinampVersion(HWND winamp)
{
	if(ver == -1)
	{
		return (ver = SendMessage(winamp,WM_WA_IPC,0,IPC_GETVERSION));
	}
	return ver;
}

template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t){
	if (WASABI_API_SVC){
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory){
			api_t = (api_T *)factory->getInterface();
		}
	}
}

BOOL DetectModernSkinLoaded(void)
{
	wchar_t skindir[MAX_PATH]={0};
	SendMessage(plugin.hwndParent,WM_WA_IPC,(WPARAM)skindir,IPC_GETSKINW);
	StringCchCat(skindir,MAX_PATH,L"\\skin.xml");
	return PathFileExists(skindir);
}

void InsertItemIntoMainMenu(void){
	MENUITEMINFO i = {sizeof(i), MIIM_ID | MIIM_STATE | MIIM_TYPE, MFT_STRING, MFS_UNCHECKED, WINAMP_ARTVIEW_MENUID};
	i.dwTypeData = WASABI_API_LNGSTRINGW(IDS_ALBUM_ART_MENU);
	InsertMenuItem(menu, 10 + (int)SendMessage(plugin.hwndParent,WM_WA_IPC,(WPARAM)0,IPC_ADJUST_OPTIONSMENUPOS), TRUE, &i);
	SendMessage(plugin.hwndParent,WM_WA_IPC,(WPARAM)1,IPC_ADJUST_OPTIONSMENUPOS);
}

int init()
{
	if(GetWinampVersion(plugin.hwndParent) < 0x5053)
	{
		// this is due to the api_application dependancy to allow for registering a hotkey correctly
		MessageBoxA(plugin.hwndParent,"This plug-in requires Winamp v5.9 and up for it to work.\t\n"
									  "Please upgrade your Winamp client to be able to use this.",
									  plugin.description,MB_OK|MB_ICONINFORMATION);
		return GEN_INIT_FAILURE;
	}
	else
	{
		WASABI_API_SVC = (api_service*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
		if(WASABI_API_SVC != NULL)
		{
			INI_FILE = (char*)SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_GETINIFILE);

			ServiceBuild(WASABI_API_MEMMGR,memMgrApiServiceGuid);
			if(WASABI_API_MEMMGR == NULL) return 1;

			ServiceBuild(AGAVE_API_ALBUMART,albumArtGUID);
			if(AGAVE_API_ALBUMART == NULL) return 1;

			ServiceBuild(WASABI_API_APP,applicationApiServiceGuid);
			if(WASABI_API_APP == NULL) return 1;

			ServiceBuild(WASABI_API_LNG,languageApiGUID);
			if(WASABI_API_LNG == NULL) return 1;

			WASABI_API_START_LANG(plugin.hDllInstance,GenClassicArtGUID);

			static char pluginTitle[MAX_PATH] = {0};
			StringCchPrintfA(pluginTitle, MAX_PATH, WASABI_API_LNGSTRING(IDS_PLUGIN_NAME), PLUGIN_VERSION);
			plugin.description = pluginTitle;

			WADlg_init(plugin.hwndParent);

			// subclass main window
			oldWndProc = (WNDPROC)SetWindowLongPtrW(plugin.hwndParent, GWLP_WNDPROC, (LONG_PTR)SubclassProc);
			oldPlaylistWndProc = (WNDPROC)SetWindowLongPtrW((HWND)SendMessage(plugin.hwndParent,WM_WA_IPC,IPC_GETWND_PE,IPC_GETWND),
															GWLP_WNDPROC, (LONG_PTR)SubclassPlaylistProc);

			// do this dynamically (if on an older client then we'd need to check for a return of 1 and set an arbitrary default)
			WINAMP_ARTVIEW_MENUID = SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_REGISTER_LOWORD_COMMAND);

			// add our menu option
			menu = (HMENU)SendMessage(plugin.hwndParent,WM_WA_IPC,(WPARAM)0,IPC_GET_HMENU);
			if(hidemodern){
				if(!DetectModernSkinLoaded())
				{
					InsertItemIntoMainMenu();
				}
				else
				{
					modernloaded=TRUE;
				}
			}
			else
			{
				InsertItemIntoMainMenu();
			}

			// load values from ini file
			lockAspect = GetPrivateProfileIntA("gen_classicart","wnd_lock_aspect",lockAspect,INI_FILE);
			myWndState.r.top = GetPrivateProfileIntA("gen_classicart","wnd_top",0,INI_FILE);
			myWndState.r.bottom = GetPrivateProfileIntA("gen_classicart","wnd_bottom",0,INI_FILE);
			myWndState.r.left = GetPrivateProfileIntA("gen_classicart","wnd_left",0,INI_FILE);
			myWndState.r.right = GetPrivateProfileIntA("gen_classicart","wnd_right",0,INI_FILE);
			artwindow_open = GetPrivateProfileIntA("gen_classicart","wnd_open",artwindow_open,INI_FILE);
			autoHide = GetPrivateProfileIntA("gen_classicart","wnd_auto_hide",autoHide,INI_FILE);
			waBkClr = GetPrivateProfileIntA("gen_classicart","waBkClr",waBkClr,INI_FILE);
			clickTrack = GetPrivateProfileIntA("gen_classicart","clickTrack",clickTrack,INI_FILE);
			hidemodern = GetPrivateProfileIntA("gen_classicart","hidemodern",hidemodern,INI_FILE);
			last_artwindow_open = GetPrivateProfileIntA("gen_classicart","wnd_open_last",last_artwindow_open,INI_FILE);

			// create window
			myWndState.flags = EMBED_FLAGS_NOWINDOWMENU;
			myWnd = (HWND)SendMessage(plugin.hwndParent,WM_WA_IPC,(WPARAM)&myWndState,IPC_GET_EMBEDIF);
			WASABI_API_APP->app_registerGlobalWindow(myWnd);
			myWndChild = WASABI_API_CREATEDIALOGW(IDD_DIALOG,myWnd,art_dlgproc);

			SET_EMBED_GUID((&myWndState),ArtViewerGUID);
			SetWindowText(myWnd,WASABI_API_LNGSTRINGW(IDS_ALBUM_ART));

			if(SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_INITIAL_SHOW_STATE) == SW_SHOWMINIMIZED && artwindow_open)
			{
				// we are starting minimised so process as needed (keep our window hidden)
				MENUITEMINFO i = {sizeof(i), MIIM_STATE , MFT_STRING, MFS_UNCHECKED, WINAMP_ARTVIEW_MENUID};
				SetMenuItemInfo(menu, WINAMP_ARTVIEW_MENUID, FALSE, &i);
				ArtView_SetMinimised(TRUE);
			}
			else
			{
				if(artwindow_open) viewArtWindow(TRUE);
			}

			// not working correctly at the moment
			ACCEL accel = {FVIRTKEY|FALT,'A',WINAMP_ARTVIEW_MENUID};
			addAccelerators(myWndChild,&accel,1,TRANSLATE_MODE_GLOBAL);

			context_menu = WASABI_API_LOADMENUW(IDR_MENU1);
			return GEN_INIT_SUCCESS;
		}
	}
	return GEN_INIT_FAILURE;
}

void quit()
{
	if(no_uninstall)
	{
		// save window state
		#define WritePrivateProfileInt(key, val) \
		{ \
		char zzval[10] = {0}; \
			StringCchPrintfA(zzval,10,"%d",val); \
			WritePrivateProfileStringA("gen_classicart",key,zzval,INI_FILE); \
		}

		GetWindowRect(myWnd,&myWndState.r);
		WritePrivateProfileInt("wnd_top",myWndState.r.top);
		WritePrivateProfileInt("wnd_bottom",myWndState.r.bottom);
		WritePrivateProfileInt("wnd_left",myWndState.r.left);
		WritePrivateProfileInt("wnd_right",myWndState.r.right);
		WritePrivateProfileInt("wnd_open",artwindow_open);
		WritePrivateProfileInt("wnd_lock_aspect",lockAspect);
		WritePrivateProfileInt("wnd_auto_hide",autoHide);
		WritePrivateProfileInt("clickTrack",clickTrack);
		WritePrivateProfileInt("hidemodern",hidemodern);
		// reset the state so if we're using a classic skin then it'll work if the window is disabled
		if(modernloaded==FALSE) last_artwindow_open = artwindow_open;
		WritePrivateProfileInt("wnd_open_last",last_artwindow_open);
	}

	WASABI_API_APP->app_unregisterGlobalWindow(myWnd);
	ArtView_SetMinimised(FALSE);
	DestroyWindow(myWnd);
	WADlg_close();

	// restores the original winamp window proc now that we are closing and if the window was subclassed
	if(GetWindowLongPtr(plugin.hwndParent,GWLP_WNDPROC) == (LONG_PTR)SubclassProc){
		SetWindowLongPtr(plugin.hwndParent,GWLP_WNDPROC,(LONG_PTR)oldWndProc);		
	}

	// restores the original playlist window proc now that we are closing and if the window was subclassed
	HWND pe_wnd = (HWND)SendMessage(plugin.hwndParent,WM_WA_IPC,IPC_GETWND_PE,IPC_GETWND);
	if(GetWindowLongPtr(pe_wnd,GWLP_WNDPROC) == (LONG_PTR)SubclassPlaylistProc){
		SetWindowLongPtr(pe_wnd,GWLP_WNDPROC,(LONG_PTR)oldPlaylistWndProc);		
	}
}

int config_open = 0;
void config()
{
	/*if(!config_open){
	char message[256]={0}, tmp[128]={0};
		config_open = 1;
		StringCchPrintfA(message,256,WASABI_API_LNGSTRING(IDS_ABOUT_STRING),WASABI_API_LNGSTRING_BUF(IDS_ALBUM_ART,tmp,128));
		MessageBoxA(0,message,plugin.description,0);
		config_open = 0;
	}
	else{
		SetActiveWindow(FindWindowA("#32770",plugin.description));
	}*/

HWND list =	FindWindowEx(GetParent(GetFocus()),0,L"ListBox",0);
HMENU popup = GetSubMenu(WASABI_API_LOADMENUW(IDR_MENU1),0);
RECT r = {0};
wchar_t temp[MAX_PATH] = {0};

	DeleteMenu(popup,ID_CONTEXTMENU_GETALBUMART,MF_BYCOMMAND);
	DeleteMenu(popup,ID_CONTEXTMENU_REFRESH,MF_BYCOMMAND);
	DeleteMenu(popup,ID_CONTEXTMENU_OPENFOLDER,MF_BYCOMMAND);

	MENUITEMINFO i = {sizeof(i),MIIM_ID|MIIM_STATE|MIIM_TYPE,MFT_STRING,MFS_UNCHECKED|MFS_DISABLED,1};
	StringCchPrintf(temp,MAX_PATH,WASABI_API_LNGSTRINGW(IDS_PLUGIN_NAME),TEXT(PLUGIN_VERSION));
	i.dwTypeData = temp;
	InsertMenuItem(popup, 0, TRUE, &i);

	i.wID=2;
	i.fState=0;
	i.fType=MFT_SEPARATOR;
	InsertMenuItem(popup, -1, TRUE, &i);

	i.dwTypeData = WASABI_API_LNGSTRINGW(IDS_ABOUT);
	i.wID=4;
	i.fType=MFT_STRING;
	InsertMenuItem(popup, -1, TRUE, &i);

	SendMessage(list,LB_GETITEMRECT,SendMessage(list,LB_GETCURSEL,1,0),(LPARAM)&r);
	ClientToScreen(list,(LPPOINT)&r);

	CheckMenuItem(popup,ID_CONTEXTMENU_LOCKASPECTRATIO,(lockAspect?MF_CHECKED:MF_UNCHECKED)|MF_BYCOMMAND);
	CheckMenuItem(popup,ID_CONTEXTMENU_AUTOHIDE,(autoHide?MF_CHECKED:MF_UNCHECKED)|MF_BYCOMMAND);
	CheckMenuItem(popup,ID_CONTEXTMENU_CLICKTRACK,(clickTrack?MF_CHECKED:MF_UNCHECKED)|MF_BYCOMMAND);
	CheckMenuItem(popup,ID_CONTEXTMENU_AUTO_HIDE_MODERN,(hidemodern?MF_CHECKED:MF_UNCHECKED)|MF_BYCOMMAND);

	switch(TrackPopupMenu(popup,TPM_RETURNCMD|TPM_LEFTBUTTON,r.left,r.top,0,list,NULL)){
		case 4:
			{
			char message[256]={0}, tmp[128]={0};
				StringCchPrintfA(message,256,WASABI_API_LNGSTRING(IDS_ABOUT_STRING),WASABI_API_LNGSTRING_BUF(IDS_ALBUM_ART,tmp,128));
				MessageBoxA(list,message,plugin.description,0);
			}
			break;
		case ID_CONTEXTMENU_LOCKASPECTRATIO:
			lockAspect = (!lockAspect);
			if(cacheDC) DeleteDC(cacheDC); cacheDC = NULL;
			InvalidateRect(myWndChild,NULL,TRUE);
			break;
		case ID_CONTEXTMENU_AUTOHIDE:
			autoHide = (!autoHide);
			break;
		case ID_CONTEXTMENU_CLICKTRACK:
			clickTrack = (!clickTrack);
			break;
		case ID_CONTEXTMENU_AUTO_HIDE_MODERN:
			hidemodern = (!hidemodern);
			break;
	}

	// clean up as we're sharing this menu
	DestroyMenu(popup);
}

static LRESULT WINAPI SubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// handles the item being selected through the main window menu
	// including via the windows taskbar menu as it'll fail otherwise
	if((msg == WM_COMMAND || msg == WM_SYSCOMMAND) && LOWORD(wParam) == WINAMP_ARTVIEW_MENUID)
	{
		if(artwindow_open) PostMessage(myWndChild,WM_CLOSE,0,0);
		else PostMessage(myWndChild,WM_USER+1,0,0);
	}

	else if(msg == WM_WA_IPC)
	{
		if(lParam == IPC_CB_MISC && (wParam == IPC_CB_MISC_TITLE || wParam == IPC_CB_MISC_TITLE_RATING))
		{
			// art change
			PostMessage(myWndChild,WM_USER,0,0);
		}
		else if(lParam == IPC_SKIN_CHANGED && hidemodern)
		{
			// need to check this when doing classic->modern as it causes the pledit to be hidden
			if(DetectModernSkinLoaded()){
				if(modernloaded==FALSE){
					last_artwindow_open = artwindow_open;
					if(last_artwindow_open==TRUE){
						PostMessage(myWndChild,WM_CLOSE,0,0);
					}
					DeleteMenu(menu,WINAMP_ARTVIEW_MENUID,MF_BYCOMMAND);
					SendMessage(plugin.hwndParent,WM_WA_IPC,(WPARAM)-1,IPC_ADJUST_OPTIONSMENUPOS);
				}
				modernloaded=TRUE;
			}
			else{
				if(modernloaded==TRUE)
				{
					InsertItemIntoMainMenu();
					if(last_artwindow_open==TRUE)
					{
						PostMessage(myWndChild,WM_USER+1,0,0);
					}
				}
				modernloaded=FALSE;
			}
		}
	}

	// the purpose of this is show our gen frame window if we started in a minimised state
	// as showing then hiding will otherwise cause a window to flash briefly on screen
	else if(msg == WM_SIZE)
	{
		if (wParam == SIZE_RESTORED)
		{
			if(GetPropW(myWnd,L"AAMinMode"))
			{
				ShowWindow(myWnd,SW_SHOWNA);
				ArtView_SetMinimised(FALSE);
			}
		}
	}

	return CallWindowProc(oldWndProc, hwnd, msg, wParam, lParam);
}

int IsInPlaylistArea(HWND playlist_wnd, int mode)
{
POINT pt = {0};
RECT rc = {0};

	GetCursorPos(&pt);
	GetClientRect(playlist_wnd,&rc);

	ScreenToClient(playlist_wnd,&pt);
	// this corrects so the selection works correctly on the selection boundary
	pt.y -= 2;

	if(!mode)
	{
		// corrects for the window area so it only happens if a selection happens
		rc.top += 18;
		rc.left += 12;
		rc.right -= 19;
		rc.bottom -= 40;
		return PtInRect(&rc,pt);
	}
	else
	{
		rc.bottom -= 13;
		rc.top = rc.bottom - 19;
		rc.left += 14;

		for(int i = 0; i < 4; i++)
		{
			rc.right = rc.left + 22;
			if(PtInRect(&rc,pt))
			{
				return 1;	
			}
			else{
				rc.left = rc.right + 7;
			}
		}
		return 0;
	}
}

static LRESULT WINAPI SubclassPlaylistProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret=0;

	// this will detect false clicks such as when the menus are shown in the classic playlist editor
	if(msg == WM_LBUTTONDOWN)
	{
		if(IsInPlaylistArea(hwnd,1))
		{
			on_click = 1;
		}
	}

	ret = CallWindowProc(oldPlaylistWndProc, hwnd, msg, wParam, lParam);

	// this will then handle the detection of a proper click in the playlist editor so
	// if enabled then we can get and show the album art for the selected playlist item
	if(msg == WM_LBUTTONDOWN && !(GetKeyState(VK_MENU)&0x1000) &&
								!(GetKeyState(VK_SHIFT)&0x1000) &&
								!(GetKeyState(VK_CONTROL)&0x1000))
	{
		if(!on_click && clickTrack)
		{
			POINT pt = {0};
			INT index = 0;
			GetCursorPos(&pt);
			ScreenToClient(hwnd,&pt);
			// this corrects so the selection works correctly on the selection boundary
			pt.y -= 2;
			index = SendMessage(hwnd,WM_WA_IPC,IPC_PE_GETIDXFROMPOINT,(LPARAM)&pt);
			if(IsInPlaylistArea(hwnd,0))
			{
				// bounds check things
				if(index < SendMessage(hwnd,WM_WA_IPC,IPC_PE_GETINDEXTOTAL,0))
				{
					// art change to show the selected item in the playlist editor
					PostMessage(myWndChild,WM_USER,0,(LPARAM)SendMessage(plugin.hwndParent,WM_WA_IPC,index,IPC_GETPLAYLISTFILEW));
				}
			}
		}
		else
		{
			// needs to do an increment for the next click will be a false
			if(on_click == 1)
			{
				on_click = 2;
			}
			else{
				on_click = 0;
			}
		}
	}

	else if(msg == WM_COMMAND)
	{
		// this is used for tracking the selection of items in the playlist editor
		// so we can update the album art when doing a single up/down selection
		if((LOWORD(wParam) == ID_PE_SCUP || LOWORD(wParam) == ID_PE_SCDOWN) && clickTrack)
		{
			// only do on up/down without any other keyboard accelerators pressed
			if(!(GetKeyState(VK_MENU)&0x1000) &&
			   !(GetKeyState(VK_SHIFT)&0x1000) &&
			   !(GetKeyState(VK_CONTROL)&0x1000))
			{
				if(SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_PLAYLIST_GET_SELECTED_COUNT))
				{
					int sel = SendMessage(plugin.hwndParent,WM_WA_IPC,-1,IPC_PLAYLIST_GET_NEXT_SELECTED);
					if(sel != -1)
					{
						// art change to show the selected item in the playlist editor
						PostMessage(myWndChild,WM_USER,0,(LPARAM)SendMessage(plugin.hwndParent,WM_WA_IPC,sel,IPC_GETPLAYLISTFILEW));
					}
				}
			}
		}
	}

	return ret;
}

ARGB32 * loadImg(const void * data, int len, int *w, int *h, bool ldata=false)
{
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = WASABI_API_SVC->service_getNumServices(imgload);
	for(int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
		if(sf)
		{
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if(l)
			{
				if(l->testData(data,len))
				{
					ARGB32* ret;
					if(ldata) ret = l->loadImageData(data,len,w,h);
					else ret = l->loadImage(data,len,w,h);
					sf->releaseInterface(l);
					return ret;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return NULL;
}

ARGB32 * loadRrc(int id, wchar_t * sec, int *w, int *h, bool data=false)
{
	DWORD size=0;
	HGLOBAL resourceHandle = WASABI_API_LOADRESFROMFILEW(sec,MAKEINTRESOURCE(id),&size);
	if(resourceHandle)
	{
		ARGB32* ret = loadImg(resourceHandle,size,w,h,data);
		UnlockResource(resourceHandle);
		return ret;
	}
	return NULL;
}

void adjustbmp(ARGB32 * p, int len, COLORREF fg)
{
	ARGB32 * end = p+len;
	while (p < end)
	{
		int a = (*p>>24)&0xff ;
		int b = a*((*p&0xff) * (fg&0xff)) / (0xff*0xff);
		int g = a*(((*p>>8)&0xff) * ((fg>>8)&0xff)) / (0xff*0xff);
		int r = a*(((*p>>16)&0xff) * ((fg>>16)&0xff)) / (0xff*0xff);
		*p = (a<<24) | (r&0xff) | ((g&0xff)<<8) | ((b&0xff)<<16);
		p++;
	}
}

void DrawArt(HDC dc, HWND hwndDlg, ARGB32 * cur_image, int cur_w, int cur_h)
{
	RECT dst, wnd;
	GetWindowRect(hwndDlg,&wnd);
	wnd.right = wnd.right - wnd.left;
	wnd.left = 0;
	wnd.bottom = wnd.bottom - wnd.top;
	wnd.top = 0;

	if(!memcmp(&lastWnd,&wnd,sizeof(RECT)) && cacheDC)
	{
		BitBlt(dc,0,0,wnd.right,wnd.bottom,cacheDC,0,0,SRCCOPY);
		return;
	}

	// create cacheDC
	if(cacheDC) DeleteDC(cacheDC);
	cacheDC = CreateCompatibleDC(dc);
	HBITMAP hbm = CreateCompatibleBitmap(dc,wnd.right,wnd.bottom);
	SelectObject(cacheDC,hbm);
	DeleteObject(hbm);
	lastWnd = wnd;

	if(!lockAspect) dst = wnd;
	else 
	{
		// maintain 'square' stretching, fill in dst
		double aspX = (double)(wnd.right)/(double)cur_w;
		double aspY = (double)(wnd.bottom)/(double)cur_h;
		double asp = min(aspX, aspY);
		int newW = (int)(cur_w*asp);
		int newH = (int)(cur_h*asp);
		dst.left = (wnd.right - newW)/2;
		dst.top = (wnd.bottom - newH)/2;
		dst.right = dst.left + newW;
		dst.bottom = dst.top + newH;
	}

	// fill the background to black
	HBRUSH brush = CreateSolidBrush(bgcolour);
	FillRect(cacheDC,&wnd,brush);
	DeleteObject(brush);

	//SkinBitmap(cur_image, cur_w, cur_h).stretchToRect(&DCCanvas(cacheDC), &dst);
	HDC srcDC = CreateCompatibleDC(dc);
	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = cur_w;
	bmi.bmiHeader.biHeight = -cur_h;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	void *bits = 0;
	HBITMAP srcBMP = CreateDIBSection(srcDC, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
	memcpy(bits, cur_image, cur_w*cur_h*4);

	HBITMAP oldSrcBM = (HBITMAP)SelectObject(srcDC,srcBMP);
	BLENDFUNCTION blendFn;
	blendFn.BlendOp = AC_SRC_OVER;
	blendFn.BlendFlags  = 0;
	blendFn.SourceConstantAlpha  = 255;
	blendFn.AlphaFormat = AC_SRC_ALPHA;

	AlphaBlend(cacheDC,
	           dst.left, dst.top,
	           dst.right-dst.left, dst.bottom-dst.top,
	           srcDC,
	           0, 0,
	           cur_w, cur_h,
	           blendFn);

	BitBlt(dc,0,0,wnd.right,wnd.bottom,cacheDC,0,0,SRCCOPY);
	SelectObject(srcDC,oldSrcBM);
	DeleteObject(srcBMP);
	DeleteDC(srcDC);
}

static INT_PTR CALLBACK art_dlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static ARGB32 * cur_image;
	static int cur_w, cur_h;
	static bool closed;
	switch(msg)
	{
	case WM_INITDIALOG:
		closed = 0;
		cur_image = 0;
		bgcolour = WADlg_getColor(WADLG_ITEMBG);
		PostMessage(hwndDlg,WM_USER,0,0);
		break;
	case WM_USER+1:
		viewArtWindow(TRUE);
		closed=0;
		break;
	case WM_DISPLAYCHANGE:
		WADlg_init(plugin.hwndParent);
		bgcolour = WADlg_getColor(WADLG_ITEMBG);
	case WM_USER:
		{
			wchar_t *filename = (!lParam?(wchar_t *)SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_GET_PLAYING_FILENAME):(wchar_t*)lParam);
			if(cur_image) WASABI_API_MEMMGR->sysFree(cur_image); cur_image = 0;
			if (AGAVE_API_ALBUMART->GetAlbumArt(filename, L"cover", &cur_w, &cur_h, &cur_image) != ALBUMART_SUCCESS)
			{/*
				SkinBitmap b(L"winamp.cover.notfound");
				if(!b.isInvalid())
				{
					cur_w = b.getWidth();
					cur_h = b.getHeight();
					cur_image = (ARGB32*)WASABI_API_MEMMGR->sysMalloc(cur_w * cur_h * sizeof(ARGB32));
					memcpy(cur_image,b.getBits(),cur_w * cur_h * sizeof(ARGB32));
				}
				else*/
				{
					cur_image = loadRrc(IDR_IMAGE_NOTFOUND,L"PNG",&cur_w, &cur_h,true);
					if(cur_image) adjustbmp(cur_image, cur_w*cur_h, WADlg_getColor(WADLG_ITEMFG));
				}

				if(!waBkClr) bgcolour = WADlg_getColor(WADLG_ITEMBG);
				if(autoHide && !closed && msg != WM_DISPLAYCHANGE)
					viewArtWindow(FALSE);
			}
			else
			{
				if(waBkClr) bgcolour = RGB(0,0,0);
				if(autoHide && !closed && msg != WM_DISPLAYCHANGE)
					viewArtWindow(TRUE);
			}
			if(cacheDC) DeleteDC(cacheDC); cacheDC = NULL;
			InvalidateRect(hwndDlg,NULL,TRUE);
		}
		break;
	case WM_DESTROY:
		if(cur_image) WASABI_API_MEMMGR->sysFree(cur_image); cur_image = 0;
		if(cacheDC) DeleteDC(cacheDC); cacheDC = NULL;
		break;
	case WM_PAINT:
		{
			if (cur_image)
			{
				PAINTSTRUCT psPaint={0};
				HDC dc = BeginPaint(hwndDlg, &psPaint);
				DrawArt(dc,hwndDlg,cur_image,cur_w,cur_h);
				EndPaint(hwndDlg, &psPaint);
			}
		}
		break;
	case WM_ERASEBKGND:
		{
			if (cur_image)
			{
				HDC dc = (HDC)wParam;
				DrawArt(dc,hwndDlg,cur_image,cur_w,cur_h);
				return 1;
			}
		}
		break;
	case WM_SIZE:
		if(cacheDC) DeleteDC(cacheDC); cacheDC = NULL;
		InvalidateRect(hwndDlg,NULL,TRUE);
		break;
	case WM_CLOSE:
		closed=1;
		viewArtWindow(FALSE);
		break;
	case WM_LBUTTONDBLCLK:
		PostMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(ID_CONTEXTMENU_OPENFOLDER,0),0);
		break;
	case WM_RBUTTONDOWN:
		{
			HMENU menu = GetSubMenu(context_menu,0);
			POINT p;
			GetCursorPos(&p);
			CheckMenuItem(menu,ID_CONTEXTMENU_LOCKASPECTRATIO,(lockAspect?MF_CHECKED:MF_UNCHECKED)|MF_BYCOMMAND);
			CheckMenuItem(menu,ID_CONTEXTMENU_AUTOHIDE,(autoHide?MF_CHECKED:MF_UNCHECKED)|MF_BYCOMMAND);
			CheckMenuItem(menu,ID_CONTEXTMENU_CLICKTRACK,(clickTrack?MF_CHECKED:MF_UNCHECKED)|MF_BYCOMMAND);
			CheckMenuItem(menu,ID_CONTEXTMENU_AUTO_HIDE_MODERN,(hidemodern?MF_CHECKED:MF_UNCHECKED)|MF_BYCOMMAND);
			TrackPopupMenu(menu,TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY,p.x,p.y,0,hwndDlg,NULL);
		}
		break;
	case WM_COMMAND:
		// this is used when 'Alt+A' is pressed by the user as part of the registered global shortcut
		if(LOWORD(wParam) == WINAMP_ARTVIEW_MENUID)
		{
			if(artwindow_open) PostMessage(myWndChild,WM_CLOSE,0,0);
			else PostMessage(myWndChild,WM_USER+1,0,0);
		}

		switch(LOWORD(wParam))
		{
		case ID_CONTEXTMENU_GETALBUMART:
			{
				wchar_t *filename = (wchar_t *)SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_GET_PLAYING_FILENAME);
				if(filename && *filename)
				{
					wchar_t artist[1024],album[1024];
					extendedFileInfoStructW a = {filename,L"artist",artist,1024};
					SendMessage(plugin.hwndParent,WM_WA_IPC,(LPARAM)&a,IPC_GET_EXTENDED_FILE_INFOW);
					a.metadata = L"album";
					a.ret = album;
					SendMessage(plugin.hwndParent,WM_WA_IPC,(LPARAM)&a,IPC_GET_EXTENDED_FILE_INFOW);
					artFetchData d = {sizeof(d),hwndDlg,artist,album,0};
					int r = (int)SendMessage(plugin.hwndParent,WM_WA_IPC,(LPARAM)&d,IPC_FETCH_ALBUMART);
					if(r == 0 && d.imgData && d.imgDataLen) // success, save art in correct location
					{
						AGAVE_API_ALBUMART->SetAlbumArt(filename,L"cover",0,0,d.imgData,d.imgDataLen,d.type);
						WASABI_API_MEMMGR->sysFree(d.imgData);
						SendMessage(hwndDlg,WM_USER,0,0);	
					}
				}
			}
			break;
		case ID_CONTEXTMENU_LOCKASPECTRATIO:
			lockAspect = (!lockAspect);
			if(cacheDC) DeleteDC(cacheDC); cacheDC = NULL;
			InvalidateRect(hwndDlg,NULL,TRUE);
			break;
		case ID_CONTEXTMENU_REFRESH:
			SendMessage(hwndDlg,WM_USER,0,0);
			break;
		case ID_CONTEXTMENU_OPENFOLDER:
			{
				wchar_t *filename = (wchar_t *)SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_GET_PLAYING_FILENAME);
				if(filename && *filename)
				{
					wchar_t fn[MAX_PATH];
					lstrcpynW(fn,filename,MAX_PATH);
					PathRemoveFileSpecW(fn);
					ShellExecuteW(NULL,L"open",fn,NULL,NULL,SW_SHOW);
				}
			}
			break;
		case ID_CONTEXTMENU_AUTOHIDE:
			autoHide = (!autoHide);
			break;
		case ID_CONTEXTMENU_CLICKTRACK:
			clickTrack = (!clickTrack);
			break;
		case ID_CONTEXTMENU_AUTO_HIDE_MODERN:
			hidemodern = (!hidemodern);
			break;
		}
		break;
	}
	return 0;
}

void viewArtWindow(BOOL show)
{
	artwindow_open=show;
	MENUITEMINFO i = {sizeof(i), MIIM_STATE , MFT_STRING, (artwindow_open?MFS_CHECKED:MFS_UNCHECKED), WINAMP_ARTVIEW_MENUID};
	SetMenuItemInfo(menu, WINAMP_ARTVIEW_MENUID, FALSE, &i);
	ShowWindow(myWnd,(artwindow_open?SW_SHOW:SW_HIDE));
}

void addAccelerators(HWND hwnd, ACCEL* accel, int accel_size, int translate_mode)
{
	HACCEL hAccel = CreateAcceleratorTable(accel,accel_size);
	if (hAccel) WASABI_API_APP->app_addAccelerators(hwnd, &hAccel, accel_size, translate_mode);
}

#ifdef __cplusplus
extern "C" {
#endif

	__declspec( dllexport ) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin()
	{
		return &plugin;
	}

	__declspec(dllexport) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param)
	{
		// prompt to remove our settings with default as no (just incase)
		if(MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_DO_YOU_ALSO_WANT_TO_REMOVE_SETTINGS),
					   plugin.description,MB_YESNO|MB_DEFBUTTON2) == IDYES)
		{
			WritePrivateProfileStringA("gen_classicart",0,0,INI_FILE);
			no_uninstall = FALSE;
		}

		// as we're doing too much in subclasses, etc we cannot allow for on-the-fly removal so need to do a normal reboot
		return GEN_PLUGIN_UNINSTALL_REBOOT;
	}

#ifdef __cplusplus
}
#endif