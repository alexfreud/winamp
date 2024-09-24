#define PLUGIN_NAME		L"Nullsoft Tray Control"
#define PLUGIN_VERSION	L"2.49"

// Winamp general purpose plug-in mini-SDK
// Copyright (C) 1997, Justin Frankel/Nullsoft
// Modifications and useability enhancements by DrO aka Darren Owen 2006-2014
#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include "../winamp/gen.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/ipc_pe.h"
#include "resource.h"
#include "winampcmd.h"
#include "api__gen_tray.h"
#include <strsafe.h>


#ifndef _DEBUG
BOOL WINAPI _DllMainCRTStartup(HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hInst);
	return TRUE;
}
#endif


#define NUM_ICONS 8
#define FOURWAY_NUM 7
#define SYSTRAY_ICON_BASE 1024

// used for Win7+ usage inorder to get the direct location of the icon instead of the prior hacks (thanks MS!)
typedef HRESULT (WINAPI *SHELL_NOTIFYICONGETRECT)(const NOTIFYICONIDENTIFIER* identifier, RECT* iconLocation);
SHELL_NOTIFYICONGETRECT g_Shell_NotifyIconGetRect = 0;

// Use a guid to uniquely identify our icon
class __declspec(uuid("B4E5FE9B-6A22-450e-9565-941EF50CFEEB")) CompactIcon;


// wasabi based services for localisation support
api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

int config_enabled = 0,
    xporhigher = 0,
    custom_enabled = 0,
    winver = 0,
	flip = 0,
    on = 1,
    update_file = 0,
	isX64 = 0,
    no_uninstall = 1,
    dlg_init = 0;

UINT s_uTaskbarRestart=0;
HWND configwnd = 0;
WNDPROC lpWndProcOld = 0;
HICON Icons[NUM_ICONS] = {0}, dummyIcon = 0;
HBITMAP compact = 0;
fileinfo2W file = {0};

wchar_t ico_pack[MAX_PATH] = {0},
		ico_pack_base[MAX_PATH] = {0},
		ico_pack_safe[MAX_PATH] = {0},
		wa_path[MAX_PATH] = {0},
		*ini_file = 0,
		szDescription[256] = {0};

int tips[NUM_ICONS] = {
	IDS_PREVIOUS_TRACK,
	IDS_PLAY_PAUSE,
	IDS_STOP,
	IDS_NEXT_TRACK,
	IDS_OPEN_FILE,
	IDS_COMPACT_MODE,
	IDS_DECREASE_VOLUME,
	IDS_INCREASE_VOLUME,
};

int tips_ex[NUM_ICONS] = {
	IDS_HOLD_CTRL,
	IDS_HOLD_CTRL,
	IDS_HOLD_SHIFT,
	-1,
	-1,
//	IDS_WIN2K_PLUS,
	-1,
	IDS_CTRL_TO_DECREASE,
	IDS_CTRL_TO_INCREASE,
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ConfigProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
HICON CreateInternalIcon(void);
int FileExists(char* filename);
void config(void);
void quit(void);
int init(void);
void config_write();
void config_read();

extern "C" winampGeneralPurposePlugin plugin =
{
	GPPHDR_VER_U,
	"nullsoft(gen_tray.dll)",
	init,
	config,
	quit,
};

extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() { return &plugin; }


HWND GetPlaylistWnd(HWND winamp){
HWND pl_wnd = 0;

	// get the playlist editor window (either v2.9x method or the older
	// for compatibility incase < 2.9x are used
	if(SendMessage(winamp,WM_WA_IPC,0,IPC_GETVERSION) >= 0x2900)
	{
		pl_wnd = (HWND)SendMessage(winamp,WM_WA_IPC,IPC_GETWND_PE,IPC_GETWND);
	}
	if(!pl_wnd)
	{
		pl_wnd = FindWindow(L"Winamp PE",0);
	}
	return pl_wnd;
}

void FormCompactText(wchar_t* szTip, int szTipLength){
int got = 0;

	// only update if we really have to (to better mimick Winamp's title behaviour, etc)
	// otherwise we query in all cases which can often reflect what appears to be the wrong information since
	// the current playlist entry is altered, etc on playlist modification hence an incorrect observation
	if(!update_file){
		update_file = 1;
		file.fileindex = (int)SendMessage(GetPlaylistWnd(plugin.hwndParent),WM_WA_IPC,IPC_PE_GETCURINDEX,0);
		got = (int)SendMessage(GetPlaylistWnd(plugin.hwndParent),WM_WA_IPC,IPC_PE_GETINDEXTITLEW,(LPARAM)&file);
	}

	// if it returns 0 then track information was received
	if(!got && file.filetitle[0]){
	int time = (int)SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_GETOUTPUTTIME);
	wchar_t buf[MAX_PATH*2] = {0}, temp[1024] = {0}, *t = temp, *p = 0;
	int over = 0, state = 0, blah = 0;
	wchar_t stateStr[32] = {0};

		switch(SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_ISPLAYING)){
			case 0:
				WASABI_API_LNGSTRINGW_BUF(IDS_STOPPED_STR,stateStr,32);
				state = lstrlen(stateStr);
			break;
			case 3:
				WASABI_API_LNGSTRINGW_BUF(IDS_PAUSED_STR,stateStr,32);
				state = lstrlen(stateStr);
			break;
		}

		p = file.filetitle;
		while(p && *p){
			*t++ = *p++;
			if(*(p-1) == '&'){
				*t++ = '&';
				*t++ = '&';
			}
		}
		*t = 0;

		StringCchPrintf(buf,MAX_PATH*2,L"%d. %s",(file.fileindex)+1,file.filetitle);
		over = lstrlen(buf);
		if(over > szTipLength - 1){over = szTipLength - 1;}
		lstrcpyn(szTip,buf,szTipLength);

		if(time != -1){
			time = time/1000;

			if(file.filelength[0]){
				StringCchPrintf(buf,MAX_PATH*2,L" [%02d:%02d/%s]",(time/60),time%60,file.filelength);
				blah = lstrlen(buf);
			}
			else{
				StringCchPrintf(buf,MAX_PATH*2,L" [%02d:%02d]",(time/60),time%60);
				blah = lstrlen(buf);
			}
		}

		if((over + blah + state) > szTipLength){
		int adj = szTipLength-blah-state-1;
			szTip[adj] = 0;
			szTip[adj-1] = L'.';
			szTip[adj-2] = L'.';
			szTip[adj-3] = L'.';
		}

		if(time != -1){
			StringCchCat(szTip,szTipLength,buf);
		}

		if(state){
			StringCchCat(szTip,szTipLength,stateStr);
		}
	}

	// fall back to the Winamp version just incase
	else{
		wchar_t temp[16] = {0};
		StringCchPrintf(temp,16,L"%X",SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_GETVERSION));
		StringCchPrintf(szTip,szTipLength,L"Winamp %c.%s",temp[0],&temp[2]);
	}
}

void free_icons(void){
int i = 0;
	for (i = 0; i < NUM_ICONS; i++)
	{
		if( Icons[i] ) {
			DestroyIcon(Icons[i]);
			Icons[i] = 0;
		}
	}

	if(dummyIcon) {
		DestroyIcon(dummyIcon);
		dummyIcon = 0;
	}

	if(compact) {
		DeleteObject(compact);
	}
}

void do_icons(int force)
{
	static int l=0;
	int i=NUM_ICONS;

	if (l == config_enabled && !force) return;

	if( force ) free_icons();

	while (i--)
	{
		if (l & (1<<i))
		{
			NOTIFYICONDATAW tnid={0};
			tnid.cbSize=sizeof(NOTIFYICONDATAW);
			tnid.hWnd=plugin.hwndParent;
			tnid.uID=i+SYSTRAY_ICON_BASE;
			Shell_NotifyIcon(NIM_DELETE, &tnid);
		}
	}

	l=config_enabled;

	if(!on) {return;}

	// have to do XP+ specific changes here in order for the icon addition order to appear as expected and not back to front!
	for (i = (xporhigher?(NUM_ICONS-1):0); (xporhigher?i>-1:i < NUM_ICONS); (xporhigher?i --:i ++))
	{
		if (config_enabled & (1<<i))
		{
			// check if an icon pack has not been set or it's not a valid file that's being passed
			// if so then need to use the default icons
			if (!ico_pack[0] || !PathFileExists(ico_pack) )
			{
				if (!Icons[i]) Icons[i] = LoadIcon(plugin.hDllInstance,MAKEINTRESOURCE(IDI_ICON1+i));
				if (i == 5) {
					compact = (HBITMAP)LoadImage(plugin.hDllInstance,MAKEINTRESOURCE(IDB_BITMAP1),IMAGE_BITMAP,0,0,LR_SHARED);
					Icons[i] = CreateInternalIcon();
				}
			}
			else
			{
				wchar_t* icpb = 0;
				lstrcpyn(ico_pack_base,ico_pack,ARRAYSIZE(ico_pack_base));
				icpb = ico_pack_base + lstrlen(ico_pack_base) - 1;
				while(icpb && *icpb && *icpb != L'\\'){icpb = CharPrev(ico_pack,icpb);}
				if (icpb) *icpb = 0;

				if (!Icons[i]){
					wchar_t entry[MAX_PATH] = {0}, buf[MAX_PATH] = {0};
					int compact_loaded = 0;

					StringCchPrintf(entry,MAX_PATH,L"ico%d",i+1);
					GetPrivateProfileString(L"tray icon pack",entry,buf,buf,ARRAYSIZE(buf),ico_pack);
					StringCchPrintf(entry,MAX_PATH,L"%s\\%s",ico_pack_base,buf);

					Icons[i] = (HICON)LoadImage(0,entry,(i != 5?IMAGE_ICON:IMAGE_BITMAP),0,0,LR_LOADFROMFILE|LR_SHARED);
					if (i == 5) {
						compact = (HBITMAP)Icons[i];
						if(compact) {
							compact_loaded = 1;
						}
					}
					// if this fails then we use the built-in versions
					if (!Icons[i]) Icons[i] = LoadIcon(plugin.hDllInstance,MAKEINTRESOURCE(IDI_ICON1+i));
					if (i == 5) {
						if (!compact_loaded) {
							compact = (HBITMAP)LoadImage(plugin.hDllInstance,MAKEINTRESOURCE(IDB_BITMAP1),IMAGE_BITMAP,0,0,LR_SHARED);
						}
						Icons[i] = CreateInternalIcon();
					}
				}
			}

			{
				NOTIFYICONDATAW tnid={0};
				tnid.cbSize=sizeof(NOTIFYICONDATAW);
				tnid.hWnd=plugin.hwndParent;
				tnid.uID=i+SYSTRAY_ICON_BASE;
				tnid.uFlags=NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_GUID;
				tnid.uCallbackMessage=WM_USER + 2707;
				tnid.hIcon=Icons[i];
				if(i != 5)
					StringCchPrintf(tnid.szTip,sizeof(tnid.szTip)/sizeof(wchar_t),L"%s - Winamp",WASABI_API_LNGSTRINGW(tips[i]));
				else
					FormCompactText(tnid.szTip,(sizeof(tnid.szTip)/sizeof(wchar_t)));

				Shell_NotifyIcon(NIM_ADD, &tnid);
			}
		}
	}
}

void config(void)
{
	if(!IsWindow(configwnd))
		WASABI_API_DIALOGBOXW(IDD_DIALOG1,0,ConfigProc);
	else
		SetActiveWindow(configwnd);
}

void quit(void)
{
	config_enabled=0;
	do_icons(0);
	free_icons();
}

BOOL CALLBACK FindTrayWnd(HWND hwnd, LPARAM lParam)
{    
	wchar_t szClassName[256] = {0};
    GetClassName(hwnd, szClassName, 255);    // Did we find the Main System Tray? If so, then get its size and quit
	if (!lstrcmpi(szClassName, L"TrayNotifyWnd"))
	{        
		HWND* pWnd = (HWND*)lParam;
		*pWnd = hwnd;
        return FALSE;    
	}    
	
	//Original code I found on Internet were seeking here for system clock and it was assumming that clock is on the right side of tray.
	//After that calculated size of tray was adjusted by removing space occupied by clock.
	//This is not a good idea - some clocks are ABOVE or somewhere else on the screen. I found that is far safer to just ignore clock space.
	return TRUE;
}

BOOL CALLBACK FindToolBarInTrayWnd(HWND hwnd, LPARAM lParam)
{    
	wchar_t szClassName[256] = {0};
    GetClassName(hwnd, szClassName, 255);    // Did we find the Main System Tray? If so, then get its size and quit
	if (!lstrcmpi(szClassName, L"ToolbarWindow32"))
	{
	HWND* pWnd = (HWND*)lParam;
		*pWnd = hwnd;
        return FALSE;
	}
	return TRUE;
}

HWND GetTrayNotifyWnd(BOOL a_bSeekForEmbedToolbar)
{
	HWND hWndTrayNotifyWnd = 0, hWndShellTrayWnd = FindWindow(L"Shell_TrayWnd", 0);

    if (hWndShellTrayWnd)    
	{        
		EnumChildWindows(hWndShellTrayWnd, FindTrayWnd, (LPARAM)&hWndTrayNotifyWnd);   
		
		if(hWndTrayNotifyWnd && IsWindow(hWndTrayNotifyWnd))
		{
		HWND hWndToolBarWnd = 0;
			EnumChildWindows(hWndTrayNotifyWnd, FindToolBarInTrayWnd, (LPARAM)&hWndToolBarWnd);   
			if(hWndToolBarWnd)
			{
				return hWndToolBarWnd;
			}
		}

		return hWndTrayNotifyWnd;
	}  

	return hWndShellTrayWnd;
}

typedef BOOL (WINAPI *ISWOW64PROCESS)(HANDLE hProcess,PBOOL Wow64Process);
BOOL IsRunningX64(void){
	ISWOW64PROCESS iswow64process = (ISWOW64PROCESS)GetProcAddress(GetModuleHandle(L"kernel32"),"IsWow64Process");
	if (iswow64process) {
	BOOL Wow64Process = 0;
		if(iswow64process(GetCurrentProcess(),&Wow64Process)){
			return Wow64Process;
		}
	}
	return FALSE;
}

struct TRAYDATA
{
    HWND hwnd;                
    UINT uID;                
    UINT uCallbackMessage;    
    DWORD Reserved[2];        
    HICON hIcon;                
};

typedef struct _TBBUTTON64 {
      int iBitmap;
      int idCommand;
      BYTE fsState;
      BYTE fsStyle;
      BYTE bReserved[6];               // padding for alignment
      DWORD_PTR dwData;
      INT_PTR iString;
} TBBUTTON64, NEAR* PTBBUTTON64, *LPTBBUTTON64;
typedef const TBBUTTON64 *LPCTBBUTTON64;

// this is used on Win7+ installs where the OS has a direct api to allow for the querying of the icon position
// most likely added natively due to the notification area fly out
BOOL NotifyIconGetRect(LPRECT a_rcIcon){
	if(g_Shell_NotifyIconGetRect){
	NOTIFYICONIDENTIFIER niid = {sizeof(NOTIFYICONIDENTIFIER),plugin.hwndParent,SYSTRAY_ICON_BASE+5,0};
		return SUCCEEDED(g_Shell_NotifyIconGetRect(&niid,a_rcIcon));
	}
	return FALSE;
}

//First tracking method: attaches to Tray process and reads data directly, is fast and reliable but will fail if user uses non standard tray software
//It was suggested by Neal Andrews with VB example: http://www.codeproject.com/shell/ctrayiconposition.asp?select=999036&forumid=14631&df=100#xx999036xx
//Ported to C++ by Ireneusz Zielinski
//Made vaguely 64-bit compatible in v2.2 of this plugin
BOOL FindOutPositionOfIconDirectly(HWND a_hWndOwner, const int a_iButtonID, LPRECT a_rcIcon)
{
	if(!NotifyIconGetRect(a_rcIcon))
	{
		DWORD dwTrayProcessID = -1, tbSize = (isX64?sizeof(TBBUTTON64):sizeof(TBBUTTON));
		HANDLE hTrayProc = NULL;
		int iButtonsCount = 0, iButton = 0;
		LPVOID lpData = 0, lpData2 = 0;
		BOOL bIconFound = FALSE;

		//first of all let's find a Tool bar control embed in Tray window
		HWND hWndTray = GetTrayNotifyWnd(TRUE);

		if (hWndTray == NULL)    
		{
			return FALSE;
		}

		//now we have to get an ID of the parent process for system tray
		GetWindowThreadProcessId(hWndTray, &dwTrayProcessID);
		if(!dwTrayProcessID)
		{
			return FALSE;
		}

		// need to use the older PROCESS_ALL_ACCESS define as it otherwise causes
		// this to fail on all XP machines but will still work ok on Vista / Win7
		#define PROCESS_ALL_ACCESS_XP (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xFFF)
		hTrayProc = OpenProcess(PROCESS_ALL_ACCESS_XP/*PROCESS_ALL_ACCESS*/, 0, dwTrayProcessID);
		if(hTrayProc == NULL)
		{
			return FALSE;
		}

		//now we check how many buttons is there - should be more than 0
		iButtonsCount = (int)SendMessage(hWndTray, TB_BUTTONCOUNT, 0, 0);

		//We want to get data from another process - it's not possible to just send messages like TB_GETBUTTON with a localy
		//allocated buffer for return data. Pointer to localy allocated data has no usefull meaning in a context of another
		//process (since Win95) - so we need to allocate some memory inside Tray process.
		//We allocate sizeof(TBBUTTON) bytes of memory - because TBBUTTON is the biggest structure we will fetch. But this buffer
		//will be also used to get smaller pieces of data like RECT structures.
		lpData = VirtualAllocEx(hTrayProc, NULL, tbSize, MEM_COMMIT, PAGE_READWRITE);
		if(lpData == NULL || iButtonsCount < 1)
		{
			CloseHandle(hTrayProc);
			return FALSE;
		}

		for(iButton = 0; iButton < iButtonsCount; iButton++)
		{
			HWND hWndOfIconOwner = 0;
			int iIconId = 0;

			//first let's read TBUTTON information about each button in a task bar of tray
			SIZE_T dwBytesRead = -1;
			TBBUTTON64 buttonData64 = {0};
			TBBUTTON buttonData = {0};
			TRAYDATA traydata = {0};

			SendMessage(hWndTray, TB_GETBUTTON, iButton, (LPARAM)lpData);
			ReadProcessMemory(hTrayProc, lpData, (isX64?(LPVOID)&buttonData64:&buttonData), tbSize, &dwBytesRead);

			if(dwBytesRead < tbSize)
			{
				continue;
			}

			// now let's read extra data associated with each button: there will be a HWND of the window that created an icon and icon ID
			ReadProcessMemory(hTrayProc, (LPCVOID)(isX64?buttonData64.dwData:buttonData.dwData), (LPVOID)&traydata, sizeof(TRAYDATA), &dwBytesRead);
			if(dwBytesRead < sizeof(TRAYDATA))
			{
				continue;
			}

			// will get the hwnd and icon id of the 'button' being checked factoring for x86 and x64 structures
			if(!isX64)
			{
				hWndOfIconOwner = traydata.hwnd;
				iIconId = traydata.uID;
			}
			else
			{
				LPARAM *tb = (LPARAM*)&traydata;
				hWndOfIconOwner = (HWND)tb[0];
				iIconId = (int)tb[2];
			}

			if(hWndOfIconOwner != a_hWndOwner || iIconId != a_iButtonID)
			{
				continue;
			}

			//we found our icon - in WinXP/Vista+ it could be hidden - let's check it:
			if(buttonData.fsState & TBSTATE_HIDDEN)
			{
				break;
			}

			//now just ask a tool bar of rectangle of our icon
			SendMessage(hWndTray, TB_GETITEMRECT, iButton, (LPARAM)lpData);
			ReadProcessMemory(hTrayProc, lpData, a_rcIcon, sizeof(RECT), &dwBytesRead);

			if(dwBytesRead < sizeof(RECT))
			{
				continue;
			}

			MapWindowPoints(hWndTray, NULL, (LPPOINT)a_rcIcon, 2);
			bIconFound = TRUE;
			break;
		}

		VirtualFreeEx(hTrayProc, lpData, 0, MEM_RELEASE);
		VirtualFreeEx(hTrayProc, lpData2, 0, MEM_RELEASE);
		CloseHandle(hTrayProc);

		return bIconFound;
	}
	return TRUE;
}

HICON CreateInternalIcon(void)
{
	HICON hGrayIcon = 0;
	HDC hMainDC = 0, hMemDC1 = 0, hMemDC2 = 0;
	BITMAP bmp = {0};
	ICONINFO csII = {0}, csGrayII = {0};

	// destroy the old version of the icon where possible otherwise we'll get a resource leak
	// which can have a nasty effect if allowed to grow too large
	if(Icons[5]){
		DestroyIcon(Icons[5]);
	}

	// create a dummy base icon with which to work on (saves having to bundle a blank on in the dll)
	if(!dummyIcon){dummyIcon = CreateIcon(plugin.hDllInstance,32,32,1,32,0,0);}

	if(!GetIconInfo(dummyIcon,&csII)){return 0;}

	if(!(hMainDC = GetDC(plugin.hwndParent)) || !(hMemDC1 = CreateCompatibleDC(hMainDC)) || !(hMemDC2 = CreateCompatibleDC(hMainDC))){
		return 0;
	}

	if(GetObject(csII.hbmColor,sizeof(BITMAP),&bmp))
	{
		int width = 0, height = 0;
		csGrayII.hbmColor = CreateBitmap((width = csII.xHotspot*2),(height = csII.yHotspot*2),bmp.bmPlanes,bmp.bmBitsPixel,0);
		if(csGrayII.hbmColor){
			int is_playing = (int)SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_ISPLAYING), dwLoopY = 0, dwLoopX = 0;
			// this is used for the temporary bitmap where the mask is created for the transparency (magic pink fun)
			HBITMAP hAndMask = CreateCompatibleBitmap(hMemDC2,width,height);
			HDC hAndMaskDC = CreateCompatibleDC(hMemDC2);
			SelectObject(hAndMaskDC,hAndMask);

			HBITMAP hOldBmp1 = (HBITMAP)SelectObject(hMemDC1,csII.hbmColor);
			HBITMAP hOldBmp2 = (HBITMAP)SelectObject(hMemDC2,csGrayII.hbmColor);

			SetStretchBltMode(hMemDC2,COLORONCOLOR);

			// play or pause or 'blank' image would go here
			SelectObject(hMemDC1,compact);
			// not the most elegant code but it'll correctly select the play/pause icon as needed based on
			// the current playback state and it's flip play/pause state as appropriately
			StretchBlt(hMemDC2,0,0,16,16,hMemDC1,(!is_playing?0:(is_playing!=1?(flip?32:0):32)),0,8,8,SRCCOPY);

			// open or stop image would go here
			StretchBlt(hMemDC2,16,0,16,16,hMemDC1,(!is_playing?8:40),0,8,8,SRCCOPY);

			// previous track image
			StretchBlt(hMemDC2,0,16,16,16,hMemDC1,16,0,8,8,SRCCOPY);

			// next track image
			StretchBlt(hMemDC2,16,16,16,16,hMemDC1,24,0,8,8,SRCCOPY);

			// process the image now that we've generated it
			for(dwLoopX=0;dwLoopX<width;++dwLoopX)
			{
				for(dwLoopY=0;dwLoopY<height;++dwLoopY)
				{
				COLORREF MainBitPixel = GetPixel(hMemDC2,dwLoopX,dwLoopY);
					// checks for magic pink and then will remove it and clear/set the relevant areas in the image mask
					if(MainBitPixel == 0xff00ff)
					{
						SetPixel(hAndMaskDC,dwLoopX,dwLoopY,RGB(255,255,255));
						SetPixel(hMemDC2,dwLoopX,dwLoopY,RGB(0,0,0));
					}
					else
					{
						SetPixel(hAndMaskDC,dwLoopX,dwLoopY,RGB(0,0,0));
						SetPixel(hMemDC2,dwLoopX,dwLoopY,MainBitPixel);
					}
				}
			}

			// set the mask for the transparent areas, etc
			csGrayII.hbmMask = hAndMask;

			DeleteDC(hAndMaskDC);

			SelectObject(hMemDC1,hOldBmp1);
			SelectObject(hMemDC2,hOldBmp2);

			csGrayII.fIcon = 1;
			hGrayIcon = CreateIconIndirect(&csGrayII);
			DeleteObject(hAndMask);
		}

		DeleteObject(csGrayII.hbmColor);
		DeleteObject(csGrayII.hbmMask);
	}

	DeleteObject(csII.hbmColor);
	DeleteObject(csII.hbmMask);
	DeleteDC(hMemDC1);
	DeleteDC(hMemDC2);
	ReleaseDC(plugin.hwndParent,hMainDC);
	return hGrayIcon;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// this will detect the start of playback and allow us to query for the new compact mode text
	// done in 2.1+ to resolve issues with the compact mode text not updating correctly in all cases
	if(lParam == IPC_PLAYING_FILE){
		update_file=0;
	}

	if((lParam == IPC_CB_MISC && wParam == IPC_CB_MISC_STATUS)){
		if (config_enabled & (1<<5)) {
			NOTIFYICONDATAW tnid={0};
			tnid.cbSize=sizeof(NOTIFYICONDATAW);
			tnid.hWnd=plugin.hwndParent;
			tnid.uID=1029;
			tnid.uFlags=NIF_ICON|NIF_TIP;
			tnid.hIcon=(Icons[5] = CreateInternalIcon());

			// force an update if stopping and the playlist is clear
			if(!SendMessage(hwnd,WM_WA_IPC,0,IPC_ISPLAYING)){
				if(!SendMessage(hwnd,WM_WA_IPC,0,IPC_GETLISTLENGTH)){
					file.filetitle[0] = 0;
					update_file=0;
				}
			}

			FormCompactText(tnid.szTip,(sizeof(tnid.szTip)/sizeof(wchar_t)));
			Shell_NotifyIcon(NIM_MODIFY,&tnid);
		}
	}

	if(message == WM_TIMER && wParam == 64){
		if (config_enabled & (1<<5)) {
			if(SendMessage(hwnd,WM_WA_IPC,0,IPC_ISPLAYING) == 3){
				NOTIFYICONDATAW tnid={0};
				tnid.cbSize=sizeof(NOTIFYICONDATAW);
				flip = !flip;
				tnid.hWnd=plugin.hwndParent;
				tnid.uID=1029;
				tnid.uFlags=NIF_ICON|NIF_TIP;
				tnid.hIcon=(Icons[5] = CreateInternalIcon());
				FormCompactText(tnid.szTip,(sizeof(tnid.szTip)/sizeof(wchar_t)));
				Shell_NotifyIcon(NIM_MODIFY,&tnid);
			}
			else
			{
				NOTIFYICONDATAW tnid={0};
				// this resets the play/pause flashing so it's in a known state when not paused
				tnid.cbSize=sizeof(NOTIFYICONDATAW);
				tnid.hWnd=plugin.hwndParent;
				tnid.uID=1029;
				tnid.uFlags=(flip?NIF_ICON:0)|NIF_TIP;
				// only re-create the icon when it's needed to be done otherwise, just update the tooltip
				if(flip)
				{
					tnid.hIcon=(Icons[5] = CreateInternalIcon());
				}
				flip = 0;
				FormCompactText(tnid.szTip,sizeof(tnid.szTip)/sizeof(wchar_t));
				Shell_NotifyIcon(NIM_MODIFY,&tnid);
			}
		}
	}

	if (message == WM_USER+2707)
	{
		switch (LOWORD(lParam))
		{
			case WM_LBUTTONDOWN:
				if (config_enabled) switch (LOWORD(wParam))
				{
					// previous icon
					case 1024:
					{
						int a;
						if ((a= (int)SendMessage(hwnd,WM_USER,0,IPC_ISPLAYING)) == 0) // not playing, let's hit prev
						{
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);
						}
						else if (a != 3) // restart or full previous action
						{
							if ((GetKeyState(VK_CONTROL)&0x1000) && SendMessage(hwnd,WM_USER,0,IPC_GETOUTPUTTIME) > 2000 )
							{
								SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);	// restart (only on a ctrl+click)
							}
							else
							{
								SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);	// move to the previous track and then start
							}
						}
						else 
						{ // prev
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);
						}
					}
					return 0;

					// play/pause icon
					case 1025:
						if ((GetKeyState(VK_CONTROL)&0x1000) )	// restart the current track
						{
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON4,0);
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);
						}
						else
						{
							// do play/pause switching to maintain current usability of the plugin
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2+(SendMessage(hwnd,WM_USER,0,IPC_ISPLAYING) == 1),0);
						}
					return 0;

					// stop icon
					case 1026:
						SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON4 + ((GetKeyState(VK_SHIFT) & 0x1000)?100:0) ,0);
					return 0;

					// next icon
					case 1027:
						SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON5,0);
					return 0;

					// open file(s) icon
					case 1028:
						SetForegroundWindow(hwnd);
						if (GetKeyState(VK_CONTROL) & (1<<15))
							SendMessage(hwnd,WM_COMMAND,WINAMP_FILE_LOC,0);
						else if (GetKeyState(VK_SHIFT) & (1<<15))
							SendMessage(hwnd,WM_COMMAND,WINAMP_FILE_DIR,0);
						else
							SendMessage(hwnd,WM_COMMAND,WINAMP_FILE_PLAY,0);
					return 0;

					// 4way mode handling, etc
					case 1029:
					{
						RECT rc = {0}, r = {0};

						// Note: this isn't compatible with hidden icons on Win7 (still to be fixed for v2.4)
						if(FindOutPositionOfIconDirectly(hwnd,1029,&rc))
						{
							int i = 0,
								// on at least Win7 (possibly earlier) the icon size is different than a fixed size
								// so for v2.3 we're just going to split the icon based on the reported size rather
								// than assuming it is a 16x16 (really 18x18 icon) as Win7's taskbar is different!
								height = (rc.bottom-rc.top)/2,
								width = (rc.right-rc.left)/2,
								x[4] = {0,width,0,width},
								y[4] = {0,0,height,height};
							POINT pt = {0};
							GetCursorPos(&pt);
							for(i = 0; i < 4; i++)
							{
								CopyRect(&r,&rc);
								r.right = r.left+x[i]+width;
								r.bottom = r.top+y[i]+height;

								if(PtInRect(&r,pt))
								{
									switch(i)
									{
										// play/pause icon
										case 0:
											if ((GetKeyState(VK_CONTROL)&0x1000) )	// restart the current track
											{
												SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON4,0);
												SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);
											}
											else
											{
												// do play/pause switching to maintain current usability of the plugin
												SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2+(SendMessage(hwnd,WM_USER,0,IPC_ISPLAYING) == 1),0);
											}
										break;

										// open file(s) / stop icon
										case 1:
											if(!SendMessage(hwnd,WM_USER,0,IPC_ISPLAYING)){
												SetForegroundWindow(hwnd);
												if (GetKeyState(VK_CONTROL) & (1<<15))
													SendMessage(hwnd,WM_COMMAND,WINAMP_FILE_LOC,0);
												else if (GetKeyState(VK_SHIFT) & (1<<15))
													SendMessage(hwnd,WM_COMMAND,WINAMP_FILE_DIR,0);
												else
													SendMessage(hwnd,WM_COMMAND,WINAMP_FILE_PLAY,0);
											}
											else{
												SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON4 + ((GetKeyState(VK_SHIFT) & 0x1000)?100:0) ,0);
											}
										break;

										// previous icon
										case 2:
										{
										int a;
											if ((a= (int)SendMessage(hwnd,WM_USER,0,IPC_ISPLAYING)) == 0) // not playing, let's hit prev
											{
												SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);
											}
											else if (a != 3) // restart or full previous action
											{
												if ((GetKeyState(VK_CONTROL)&0x1000) && SendMessage(hwnd,WM_USER,0,IPC_GETOUTPUTTIME) > 2000 )
												{
													SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);	// restart (only on a ctrl+click)
												}
												else
												{
													SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);	// move to the previous track and then start
												}
											}
											else 
											{ // prev
												SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);
											}
										}
										break;

										// next icon
										case 3:
											SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON5,0);
										break;
									}
									break;
								}
							}
						}
					}
					return 0;

					// vol down
					case 1030:
					{
						int curvol = (int)SendMessage(hwnd,WM_WA_IPC,-666,IPC_SETVOLUME)-((GetKeyState(VK_CONTROL)&0x1000)?30:10);
						if(curvol<0){curvol = 0;}
						SendMessage(hwnd,WM_WA_IPC,curvol,IPC_SETVOLUME);
					}
					return 0;

					// vol up
					case 1031:
					{
						int curvol = (int)SendMessage(hwnd,WM_WA_IPC,-666,IPC_SETVOLUME)+((GetKeyState(VK_CONTROL)&0x1000)?30:10);
						if(curvol>255){curvol = 255;}
						SendMessage(hwnd,WM_WA_IPC,curvol,IPC_SETVOLUME);
					}
					return 0;
				}
			break;

			case WM_RBUTTONDOWN:
				if (config_enabled) switch (LOWORD(wParam))
				{
					// previousicon
					case 1024:
					{
						SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON5,0);
					}
					break;

					// next icon
					case 1027:
					{
						int a;
						if ((a= (int)SendMessage(hwnd,WM_USER,0,IPC_ISPLAYING)) == 0) // not playing, let's hit prev
						{
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);
						}
						else if (a != 3) // restart or full previous action
						{
							if ((GetKeyState(VK_CONTROL)&0x1000) && SendMessage(hwnd,WM_USER,0,IPC_GETOUTPUTTIME) > 2000 )
							{
								SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);	// restart (only on a ctrl+click)
							}
							else
							{
								SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);	// move to the previous track and then start
								SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);
							}
						}
						else 
						{ // prev
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);
						}
					}
					break;

					// vol down
					case 1031:
					{
						int curvol = (int)SendMessage(hwnd,WM_WA_IPC,-666,IPC_SETVOLUME)-((GetKeyState(VK_CONTROL)&0x1000)?30:10);
						if(curvol<0){curvol = 0;}
						SendMessage(hwnd,WM_WA_IPC,curvol,IPC_SETVOLUME);
					}
					return 0;

					// vol up
					case 1030:
					{
						int curvol = (int)SendMessage(hwnd,WM_WA_IPC,-666,IPC_SETVOLUME)+((GetKeyState(VK_CONTROL)&0x1000)?30:10);
						if(curvol>255){curvol = 255;}
						SendMessage(hwnd,WM_WA_IPC,curvol,IPC_SETVOLUME);
					}
					return 0;
				}
			break;
		}
	}

	{
	int ret = (int)CallWindowProc(lpWndProcOld,hwnd,message,wParam,lParam);

		// do this after passing the main batch of messages onto Winamp/rest of the subclass chain so
		// that Winamp will restore its tray icon first and then we do ours (otherwise it looks silly)
		if(message == s_uTaskbarRestart)
		{
			// have to force the icons to be displayed since there are none in the tray at this point
			do_icons(1);
		}

		return ret;
	}
}

// GetWindowsVersionRunningOnCompact(...)
//
// Function to get the version of windows being run on
//
// Optionally a 'short version[2]' can be passed into the
// function as 'GetWindowsVersionRunningOnCompact(version)'
// which allows the OS version to be returned for the user to
// be able to make use of
//
int GetWindowsVersionRunningOnCompact(DWORD* version)
{
	OSVERSIONINFO osvi = {sizeof(OSVERSIONINFO),0};
	int ver_detect = -1;

	// Win9x detection
	//
	// Windows 95 - Major 4 & Minor 0     		ver_detect = 1
	// Windows 98 - Major 4 & Minor 10     		ver_detect = 2
	// Windows ME - Major 4 & Minor 90      	ver_detect = 3
	//
	// Win NT detection
	//
	// Windows NT 3.51 - Major 3 & Minor 51		ver_detect = 4
	// Windows NT 4 - Major 4 & Minor 0     	ver_detect = 5
	// Windows 2000 - Major 5 & Minor 0     	ver_detect = 6
	// Windows XP - Major 5 & Minor 1   		ver_detect = 7
	// Windows Server 2003 - Major 5 & Minor 2 	ver_detect = 8
	// Windows Vista - Major 6 & Minor 0		ver_detect = 9
	// Windows 7 - Major 6 & Minor 1		ver_detect = 9
	// Windows 8 - Major 6 & Minor 2		ver_detect = 9
	// Windows 8.1 - Major 6 & Minor 3		ver_detect = 9
	// Windows 10 - Major 10 & Minor 0		ver_detect = 10
	// Windows 11 - Major 11 & Minor 0		ver_detect = 11
	//
	// Unknown OS version						ver_detect = -1

	GetVersionEx(&osvi);

	// is it a Win9x platform that we are running on?
	if(osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		// Windows 98 (4.10)
		if(osvi.dwMinorVersion == 10){
			ver_detect = 2;
		}

		// Windows ME (4.90)
		else if(osvi.dwMinorVersion == 90){
			ver_detect = 3;
		}

		// Windows 95 (4.0)
		else {
			ver_detect = 1;
		}
	}

	// is it a WinNT platform that we are running on?
	else if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		// Windows NT 4 (4.0)
		if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			ver_detect = 5;
		}

		else if(osvi.dwMajorVersion == 5)
		{
			// Windows XP (5.1)
			if(osvi.dwMinorVersion == 1)
			{
				ver_detect = 7;
			}

			// Windows Server 2003 (5.2)
			else if(osvi.dwMinorVersion == 2)
			{
				ver_detect = 8;
			}

			// Windows 2000 (5.0)
			else
			{
				ver_detect = 6;
			}
		}

		// Windows Vista/7/8/8.1 (6.0)
		else if(osvi.dwMajorVersion == 6)
		{
			ver_detect = 9;
		}

		// Windows 10 (10.0)
		else if(osvi.dwMajorVersion == 10)
		{
			ver_detect = 10;
		}

		// Windows 11 (11.0)
		else if(osvi.dwMajorVersion == 11)
		{
			ver_detect = 11;
		}

		// Windows NT 3.51 (3.51)
		else
		{
			ver_detect = 4;
		}
	}
	else
	{
		ver_detect = -1;
	}

	// copies the value into the structure
	if(version)
	{
		*version = MAKELONG(osvi.dwMinorVersion,osvi.dwMajorVersion);
	}

	return ver_detect;
}

void GetWinampPath(void)
{
	wchar_t* p = wa_path;
	p += GetModuleFileName(0,wa_path,ARRAYSIZE(wa_path)) - 1;
	while(p && *p && *p != L'\\'){p = CharPrev(wa_path,p);}
	if (p) *p = 0;
}

int init(void)
{
	// loader so that we can get the localisation service api for use
	WASABI_API_SVC = (api_service*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (WASABI_API_SVC == (api_service*)1) WASABI_API_SVC = NULL;
	if (!WASABI_API_SVC || WASABI_API_SVC == (api_service *)1)
		return GEN_INIT_FAILURE;

	xporhigher = ((winver=GetWindowsVersionRunningOnCompact(0))>6);
	isX64 = IsRunningX64();
	s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
	g_Shell_NotifyIconGetRect = (SHELL_NOTIFYICONGETRECT)GetProcAddress(GetModuleHandle(L"SHELL32"),"Shell_NotifyIconGetRect");

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,GenTrayLangGUID);

	StringCchPrintf(szDescription, ARRAYSIZE(szDescription),
					WASABI_API_LNGSTRINGW(IDS_NULLSOFT_TRAY_CONTROL), PLUGIN_VERSION);
	plugin.description = (char*)szDescription;

	GetWinampPath();
	config_read();

	if (IsWindowUnicode(plugin.hwndParent))
		lpWndProcOld = (WNDPROC)SetWindowLongPtrW(plugin.hwndParent,GWLP_WNDPROC,(LPARAM)WndProc);
	else
		lpWndProcOld = (WNDPROC)SetWindowLongPtrA(plugin.hwndParent,GWLP_WNDPROC,(LPARAM)WndProc);

	do_icons(0);

	return 0;
}

static HCURSOR link_hand_cursor;
LRESULT link_handlecursor(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"link_proc"), hwndDlg, uMsg, wParam, lParam);
	// override the normal cursor behaviour so we have a hand to show it is a link
	if(uMsg == WM_SETCURSOR)
	{
		if((HWND)wParam == hwndDlg)
		{
			if(!link_hand_cursor)
			{
				link_hand_cursor = LoadCursor(NULL, IDC_HAND);
			}
			SetCursor(link_hand_cursor);
			return TRUE;
		}
	}
	return ret;
}

void link_startsubclass(HWND hwndDlg, UINT id){
HWND ctrl = GetDlgItem(hwndDlg, id);
	if(!GetPropW(ctrl, L"link_proc"))
	{
		SetPropW(ctrl, L"link_proc",
				(HANDLE)SetWindowLongPtrW(ctrl, GWLP_WNDPROC, (LONG_PTR)link_handlecursor));
	}
}

BOOL CALLBACK ConfigProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			int count = CB_ERR;
			configwnd = hwndDlg;
			dlg_init = 1;
			link_startsubclass(hwndDlg,IDC_LINK);

			CheckDlgButton(hwndDlg,IDC_ONOFF,on?BST_CHECKED:BST_UNCHECKED);

			for (int i = 0; i < NUM_ICONS; i++)
			{
				wchar_t str[512] = {0}, tmp[256] = {0}, tmp2[256] = {0};
				CheckDlgButton(hwndDlg,IDC_PREV+i,(config_enabled&(1<<i))?BST_CHECKED:BST_UNCHECKED);
				StringCchPrintf(str,512,L"%s  %s",
								WASABI_API_LNGSTRINGW_BUF(tips[i],tmp2,256),
								(tips_ex[i]!=-1?WASABI_API_LNGSTRINGW_BUF(tips_ex[i],tmp,256):L""));
				SetDlgItemText(hwndDlg,IDC_PREV+i,str);
				EnableWindow(GetDlgItem(hwndDlg,IDC_PREV+i),on);
			}

			EnableWindow(GetDlgItem(hwndDlg,IDC_COMBO2),on);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON1),custom_enabled && on);
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),custom_enabled && on);

			SetDlgItemText(hwndDlg,IDC_EDIT1,ico_pack_safe);
			
			dlg_init = 0;

			WIN32_FIND_DATA findfile = {0};
			wchar_t use[MAX_PATH] = {0};

			StringCchPrintf(use,MAX_PATH,L"%s\\Plugins\\Tray_Control\\*.*",wa_path);
			HANDLE hFind = FindFirstFile(use, &findfile);

			while(hFind && hFind != INVALID_HANDLE_VALUE)
			{
				if(findfile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				WIN32_FIND_DATA subfile = {0};
					StringCchPrintf(use,MAX_PATH,L"%s\\Plugins\\Tray_Control\\%s\\*.icp",
									wa_path,findfile.cFileName,findfile.cFileName);

					HANDLE hsubFind = FindFirstFile(use,&subfile);

					while(hsubFind && hsubFind != INVALID_HANDLE_VALUE){
					wchar_t icpname[MAX_PATH] = {0};
						StringCchPrintf(icpname,MAX_PATH,
										L"%s\\Plugins\\Tray_Control\\%s\\%s",
										wa_path,findfile.cFileName,subfile.cFileName);

						if(PathFileExists(icpname))
						{
							// need to ideally make this one work with CharPrev(..)
							wchar_t* p = subfile.cFileName + lstrlen(subfile.cFileName) - 1;
							while(p && *p && *p != L'.'){p = CharPrevW(subfile.cFileName, p);}
							if (p) *p = 0;
							if(!lstrcmpiW(findfile.cFileName,subfile.cFileName))
							{
								SendDlgItemMessageW(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)subfile.cFileName);
							}
							else
							{
								wchar_t str[MAX_PATH] = {0};
								int insertpos = 0;
								StringCchPrintf(str,MAX_PATH,L"%s\\%s",findfile.cFileName,subfile.cFileName);
								insertpos = (int)SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)str);
								if(insertpos != CB_ERR)
								{
									SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_SETITEMDATA,insertpos,1);
								}
							}
						}

						// if there are no more files then stop the search
						if(hsubFind && !FindNextFile(hsubFind, &subfile))
						{
							FindClose(hsubFind);
							hsubFind = 0;
						}
					}
				}

				// if there are no more files then stop the search
				if(hFind && !FindNextFileW(hFind, &findfile))
				{
					FindClose(hFind);
					hFind = 0;
				}
			}

			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),custom_enabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON1),custom_enabled);

			EnableWindow(GetDlgItem(hwndDlg,IDC_PREV6),winver>=6 && on);

			SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_INSERTSTRING,0,
							   (LPARAM)WASABI_API_LNGSTRINGW(IDS_DEFAULT_ICONS));
			SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_INSERTSTRING,
							   (count = (int)SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCOUNT,0,0)),
							   (LPARAM)WASABI_API_LNGSTRINGW(IDS_CUSTOM_ICON_PACK));

			if (!custom_enabled)
			{
				if(ico_pack[0])
				{
					wchar_t base[MAX_PATH] = {0}, *pack = 0, use[MAX_PATH] = {0};
					StringCchPrintf(use,MAX_PATH,L"%s\\Plugins\\Tray_Control\\",wa_path);
					lstrcpyn(base,ico_pack,ARRAYSIZE(base));

					if(StrStrI(base,use))
					{
						pack = PathFindFileName(base);
						if (pack && *pack)
						{
							PathRemoveExtension(pack);
						}
					}

					count = (int)SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_FINDSTRINGEXACT,0,(LPARAM)pack);
					if(count == CB_ERR)
					{
						count = 0;
					}
				}
				else
				{
					count = 0;
				}
			}

			SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_SETCURSEL,count,0);
			SetFocus(GetDlgItem(hwndDlg,IDC_COMBO2));
		}
		break;

		// mimicks the get... links in the winamp preferences
		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT* lpdis = (DRAWITEMSTRUCT*)lParam;
			if(lpdis->CtlID == IDC_LINK)
			{
				HFONT OldFont = 0;
				LOGFONT lf = {0};
				RECT rc = lpdis->rcItem, r = {0};
				POINT pt = {0};
				int in = 0;
				wchar_t gicpStr[128] = {0};
				
				WASABI_API_LNGSTRINGW_BUF(IDS_GET_ICON_PACKS,gicpStr,128);
				GetObject(GetCurrentObject(lpdis->hDC,OBJ_FONT),sizeof(lf),&lf);
				lf.lfUnderline = TRUE;
				OldFont = (HFONT)SelectObject(lpdis->hDC,CreateFontIndirect(&lf));

				// Calculate needed size of the control
				DrawText(lpdis->hDC,gicpStr,-1,&rc,DT_VCENTER|DT_SINGLELINE|DT_CALCRECT);

				// Make some more room so the focus rect won't cut letters off
				rc.right = min(rc.right + 2, lpdis->rcItem.right);

				GetWindowRect(lpdis->hwndItem,&r);
				GetCursorPos(&pt);
				in = PtInRect(&r,pt);
				SetTextColor(lpdis->hDC,(COLORREF)RGB((in?255:0),0,(!in?255:0)));

				// Draw the text
				DrawText(lpdis->hDC,gicpStr,-1,&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);

				DeleteObject(SelectObject(lpdis->hDC, OldFont));
			}
		}
		break;

		case WM_COMMAND:
			if (LOWORD(wParam) >= IDC_PREV && LOWORD(wParam) <= IDC_PREV+NUM_ICONS)
			{
  				config_enabled=0;
				for (int i = 0; i < NUM_ICONS; i++)
					if (IsDlgButtonChecked(hwndDlg,IDC_PREV+i))
						config_enabled |= 1<<i;
				do_icons(0);
			}
			
			else if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				GetDlgItemText(hwndDlg,IDC_EDIT1,ico_pack_safe,ARRAYSIZE(ico_pack_safe));
				config_write();
				EndDialog(hwndDlg,0);
				configwnd = 0;
			}

			else if (LOWORD(wParam) == IDC_ONOFF)
			{
				on = (IsDlgButtonChecked(hwndDlg,LOWORD(wParam))==BST_CHECKED);
				for (int i = 0; i < NUM_ICONS; i++)
				{
					EnableWindow(GetDlgItem(hwndDlg,IDC_PREV+i),on);
				}
				EnableWindow(GetDlgItem(hwndDlg,IDC_PREV6),winver>=6 && on);
				EnableWindow(GetDlgItem(hwndDlg,IDC_COMBO2),on);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON1),custom_enabled && on);
				EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),custom_enabled && on);
				
				do_icons(1);
			}

			else if (LOWORD(wParam) == ID_INFO)
			{
				wchar_t str[2560] = {0}, str1[2048] = {0};
				StringCchPrintf(str, 2560, WASABI_API_LNGSTRINGW_BUF(IDS_CONFIG_INFO, str1, 2048), wa_path);
				MessageBoxW(hwndDlg,str,szDescription,0);
			}

			else if (LOWORD(wParam) == IDC_LINK && HIWORD(wParam) == BN_CLICKED)
			{
				SendMessage(plugin.hwndParent,WM_WA_IPC,(WPARAM)"https://winampheritage.com/plugin/nullsoft-tray-control-plug-in-icon-pack/222396",IPC_OPEN_URL);
			}

			else if (LOWORD(wParam) == IDC_BUTTON1)
			{
				OPENFILENAME of = {sizeof(OPENFILENAME),0};
				wchar_t titleStr[128], filterStr[128] = {0};

				of.hwndOwner = hwndDlg;
				of.hInstance = plugin.hDllInstance;
				of.lpstrFilter = WASABI_API_LNGSTRINGW_BUF(IDS_OFD_FILTER_STR,filterStr,128);
				StringCchCat(filterStr+lstrlen(filterStr)+1,128,L"*.icp");
				of.nMaxCustFilter = 64;
				of.lpstrFile = ico_pack;
				of.nMaxFile = ARRAYSIZE(ico_pack);
				of.lpstrTitle = WASABI_API_LNGSTRINGW_BUF(IDS_OFD_TITLE_STR,titleStr,128);
				of.nMaxFileTitle = lstrlen(of.lpstrTitle);
				of.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_CREATEPROMPT|OFN_ENABLESIZING|OFN_ALLOWMULTISELECT;
				of.lpstrDefExt = L"icp";

				if(GetOpenFileName(&of))
				{
					SetDlgItemText(hwndDlg,IDC_EDIT1,ico_pack);
					do_icons(1);
				}
			}

			else if (LOWORD(wParam) == IDC_EDIT1 && HIWORD(wParam)==EN_CHANGE)
			{
				if(!dlg_init)
				{
					GetDlgItemText(hwndDlg,IDC_EDIT1,ico_pack,ARRAYSIZE(ico_pack));
					do_icons(1);
				}
			}

			else if (LOWORD(wParam) == IDC_COMBO2 && HIWORD(wParam)== CBN_SELCHANGE)
			{
				wchar_t buf[MAX_PATH] = {0};
				int cursel = (int)SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCURSEL,0,0);
				SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETLBTEXT,cursel,(LPARAM)buf);

				custom_enabled=0;

				if(cursel && cursel != CB_ERR)
				{
					if(cursel != (SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCOUNT,0,0)-1))
					{
						// this will detect if it was a multi-entry style icon pack and form as required
						if(!SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETITEMDATA,cursel,0))
						{
							StringCchPrintf(ico_pack,MAX_PATH,L"%s\\Plugins\\Tray_Control\\%s\\%s.icp",wa_path,buf,buf);
						}
						else
						{
							StringCchPrintf(ico_pack,MAX_PATH,L"%s\\Plugins\\Tray_Control\\%s.icp",wa_path,buf);
						}
					}
					else
					{
						custom_enabled=1;
						GetDlgItemText(hwndDlg,IDC_EDIT1,ico_pack,ARRAYSIZE(ico_pack));
					}
				}
				else
				{
					ico_pack[0] = 0;
				}

				EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),custom_enabled);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON1),custom_enabled);

				do_icons(1);
			}
		break;
	}
	return FALSE;
}

void config_read(void)
{
	// this will only correctly work with Winamp 2.9+/5.x+
	// see IPC_GETINIFILE for a way to query the location of Winamp.ini correctly
	// whatever version of Winamp is being run on
	// as of v2.41 this now uses IPC_GETINIFILEW though it's trivial to use
	// IPC_GETINIFILE as a fallback on older clients (built for 5.58+)
	ini_file=(wchar_t*)SendMessage(plugin.hwndParent,WM_USER,0,IPC_GETINIFILEW);
	config_enabled = GetPrivateProfileInt(PLUGIN_NAME,L"BEN",config_enabled,ini_file);
	custom_enabled = GetPrivateProfileInt(PLUGIN_NAME,L"custom",custom_enabled,ini_file);
	on = GetPrivateProfileInt(PLUGIN_NAME,L"on",on,ini_file);
	GetPrivateProfileString(PLUGIN_NAME,L"ico_pack",ico_pack,ico_pack,ARRAYSIZE(ico_pack),ini_file);
	GetPrivateProfileString(PLUGIN_NAME,L"ico_pack_safe",ico_pack_safe,ico_pack_safe,ARRAYSIZE(ico_pack_safe),ini_file);
}

void config_write(void)
{
	if(!no_uninstall) return;

	wchar_t string[32] = {0};
	StringCchPrintf(string,32,L"%d",config_enabled);
	WritePrivateProfileString(PLUGIN_NAME,L"BEN",string,ini_file);
	WritePrivateProfileString(PLUGIN_NAME,L"ico_pack",ico_pack,ini_file);
	WritePrivateProfileString(PLUGIN_NAME,L"ico_pack_safe",ico_pack_safe,ini_file);
	StringCchPrintf(string,32,L"%d",custom_enabled);
	WritePrivateProfileString(PLUGIN_NAME,L"custom",string,ini_file);
	StringCchPrintf(string,32,L"%d",on);
	WritePrivateProfileString(PLUGIN_NAME,L"on",string,ini_file);
}

#ifdef __cplusplus
extern "C" {
#endif

	__declspec(dllexport) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param){
		// prompt to remove our settings with default as no (just incase)
		if(MessageBoxW(hwndDlg,WASABI_API_LNGSTRINGW(IDS_DO_YOU_ALSO_WANT_TO_REMOVE_SETTINGS),
					   szDescription,MB_YESNO|MB_DEFBUTTON2) == IDYES)
		{
			WritePrivateProfileString(PLUGIN_NAME,0,0,ini_file);
			no_uninstall = 0;
		}

		// as we're doing too much in subclasses, etc we cannot allow for on-the-fly removal so need to do a normal reboot
		return GEN_PLUGIN_UNINSTALL_REBOOT;
	}

#ifdef __cplusplus
}
#endif