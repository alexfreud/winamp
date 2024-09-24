/*
  LICENSE
  -------
Copyright 2005 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 

  * Neither the name of Nullsoft nor the names of its contributors may be used to 
    endorse or promote products derived from this software without specific prior written permission. 
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include "r_defs.h"
#include "vis.h"
#include "cfgwnd.h"
#include "resource.h"
#include "render.h"
#include "rlib.h"
#include "draw.h"
#include "wnd.h"
#include "bpm.h"
#include "avs_eelif.h"
#include "undo.h"
#include "../Agave/Language/api_language.h"
#include "../WAT/WAT.h"

#ifdef LASER
extern "C" {
#include "laser/ld32.h"
}
#endif
static void _do_add(HWND hwnd, HTREEITEM h, C_RenderListClass *list);
static int treeview_hack;
static HTREEITEM g_hroot;

extern int g_config_smp_mt,g_config_smp;
extern struct winampVisModule *g_mod;
extern int cfg_cancelfs_on_deactivate;

HWND g_debugwnd;

char g_noeffectstr[48];
//extern char *verstr;
static HWND cur_hwnd;
int is_aux_wnd=0;
int config_prompt_save_preset=1,config_reuseonresize=1;
//int g_preset_dirty;

// extern BOOL CALLBACK aboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
extern BOOL CALLBACK DlgProc_Bpm(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
extern int readyToLoadPreset(HWND parent, int isnew);

extern char *extension(char *fn) ;

int g_dlg_fps,g_dlg_w,g_dlg_h;

int cfg_cfgwnd_x=50,cfg_cfgwnd_y=50,cfg_cfgwnd_open=0;

int cfg_fs_w=0,cfg_fs_h=0,cfg_fs_d=2,cfg_fs_bpp=0,cfg_fs_fps=0,cfg_fs_rnd=1,
#ifdef LASER
cfg_fs_flip=6,
#else
cfg_fs_flip=0,
#endif
cfg_fs_height=80,cfg_speed=5,cfg_fs_rnd_time=10,cfg_fs_use_overlay=0;
int cfg_trans=0,cfg_trans_amount=128;
int cfg_dont_min_avs=0;
int cfg_transitions=4;
int cfg_transitions2=4|32;
int cfg_transitions_speed=8;
int cfg_transition_mode=0x8001;
int cfg_bkgnd_render=0,cfg_bkgnd_render_color=0x1F000F;
int cfg_render_prio=0;

char config_pres_subdir[MAX_PATH];
char last_preset[2048];

static BOOL CALLBACK dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
HWND g_hwndDlg;

extern HWND g_hwnd;

#ifdef WA2_EMBED
#include "wa_ipc.h"
extern embedWindowState myWindowState;
#endif

/*
HINSTANCE g_hDllInstance;
HWND g_hwndParent;
HANDLE hcfgThread;
DWORD WINAPI cfgwnd_thread(LPVOID p)
{
	g_hwndDlg=CreateDialog(g_hDllInstance,MAKEINTRESOURCE(IDD_DIALOG1),NULL,dlgProc);
  while (1)
  {
    MSG msg;
    if (!GetMessage(&msg,NULL,0,0)) break;
    if (!IsDialogMessage(g_hwndDlg,&msg)) DispatchMessage(&msg);
  }
  return 0;
}
*/
static int ExtractWindowsVersion(void)
{
	int dwVersion,dwWindowsMajorVersion,dwWindowsMinorVersion,WindowsType,dwBuild;

	dwVersion = GetVersion();

	// Get major and minor version numbers of Windows

	dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));

	// Get build numbers for Windows NT or Win32s
	if (dwVersion < 0x80000000)                // Windows NT
		{
		dwBuild = (DWORD)(HIWORD(dwVersion));
		WindowsType = 0x4; // VER_WINNT
		}
	else if (dwWindowsMajorVersion < 4)        // Win32s
	    {
	    dwBuild = (DWORD)(HIWORD(dwVersion) & ~0x8000);
	    WindowsType = 0x2; // VER_WIN32S
	    }
	else         // Windows 95 -- No build numbers provided
	    {
	    dwBuild =  0;
	    WindowsType = 0x1; // VER_WIN95
	    }

	return dwWindowsMajorVersion;
}

void CfgWnd_Create(struct winampVisModule *this_mod)
{
	WASABI_API_CREATEDIALOG(IDD_DIALOG1,this_mod->hwndParent,dlgProc);
	//CreateDialogA(this_mod->hDllInstance,MAKEINTRESOURCE(IDD_DIALOG1),this_mod->hwndParent,dlgProc);
}

void CfgWnd_Destroy(void)
{
  if (g_hwndDlg && IsWindow(g_hwndDlg))
  {
    RECT r;
    GetWindowRect(g_hwndDlg,&r);
    cfg_cfgwnd_x=r.left;
    cfg_cfgwnd_y=r.top;
	  DestroyWindow(g_hwndDlg);
  }
  g_hwndDlg=0;
  if (g_debugwnd) DestroyWindow(g_debugwnd);
  /*
  if (hcfgThread)
  {
	  SendMessage(g_hwndDlg,WM_USER+6,0,0);
	  g_hwndDlg=0;
    WaitForSingleObject(hcfgThread,INFINITE);
    CloseHandle(hcfgThread);
    hcfgThread=0;
  }
  */
}

static void recursiveAddDirList(HMENU menu, UINT *id, char *path, int pathlen)
{
  HANDLE h;
  WIN32_FIND_DATA d;
  char dirmask[4096];
  wsprintf(dirmask,"%s\\*.*",path);

  h = FindFirstFile(dirmask,&d);
  if (h != INVALID_HANDLE_VALUE)
  {
	  do {
      if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && d.cFileName[0] != '.')
      {
        wsprintf(dirmask,"%s\\%s",path,d.cFileName);

        MENUITEMINFO i={sizeof(i),};
        i.fType=MFT_STRING;
        i.fMask=MIIM_TYPE|MIIM_ID;
			  i.dwTypeData = dirmask+pathlen+1;
			  i.cch = strlen(i.dwTypeData);
        i.wID=*id;
        InsertMenuItem(menu,*id+2-1025,TRUE,&i);
			  (*id)++;

        recursiveAddDirList(menu,id,dirmask,pathlen);
      }
    } while (FindNextFile(h,&d));
	  FindClose(h);
  }
}

static BOOL CALLBACK DlgProc_Preset(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
	    case WM_INITDIALOG:
        {
            int x;
            for (x = 0; x < 12; x ++)
            {
                char s[123];
                wsprintf(s,"F%d",x+1);
                SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)s);
            }
            for (x = 0; x < 10; x ++)
            {
                char s[123];
                wsprintf(s,"%d",x);
                SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)s);
            }
            for (x = 0; x < 10; x ++)
            {
                char s[123];
                wsprintf(s,WASABI_API_LNGSTRING(IDS_SHIFT_X),x);
                SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)s);
            }
    
  			CheckDlgButton(hwndDlg,IDC_CHECK3,cfg_fs_rnd?BST_CHECKED:BST_UNCHECKED);
            SetDlgItemInt(hwndDlg,IDC_EDIT1,cfg_fs_rnd_time,FALSE);
            if (config_prompt_save_preset)
              CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);

            if (config_pres_subdir[0]) 
              SetDlgItemText(hwndDlg,IDC_BUTTON3,config_pres_subdir);
            else
              SetDlgItemText(hwndDlg,IDC_BUTTON3,WASABI_API_LNGSTRING(IDS_ALL));
        }
		return 1;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_BUTTON3:
                {
                    MENUITEMINFO i={sizeof(i),};
    
                    HMENU hMenu;
                    hMenu=CreatePopupMenu();
                    i.fMask=MIIM_TYPE|MIIM_ID;
                    i.fType=MFT_STRING;
                    i.wID = 1024;
                    i.dwTypeData=WASABI_API_LNGSTRING(IDS_ALL);
                    i.cch=strlen(i.dwTypeData);
                    InsertMenuItem(hMenu,0,TRUE,&i);
                    i.wID=0;
                    i.fType=MFT_SEPARATOR;
                    InsertMenuItem(hMenu,1,TRUE,&i);

                    UINT id=1025;
                    recursiveAddDirList(hMenu,&id,g_path,strlen(g_path));

                    RECT r;
                    GetWindowRect(GetDlgItem(hwndDlg,IDC_BUTTON3),&r);

                    int x=TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY,r.left,r.bottom,0,hwndDlg,NULL);
                    if (x == 1024)
                    {
                        config_pres_subdir[0]=0;
                        SetDlgItemText(hwndDlg,IDC_BUTTON3,WASABI_API_LNGSTRING(IDS_ALL));
                    }
                    else if (x >= 1025)
                    {
			                MENUITEMINFO mi={sizeof(mi),MIIM_TYPE,};
			                mi.dwTypeData=config_pres_subdir;
			                mi.cch = sizeof(config_pres_subdir);
			                GetMenuItemInfo(hMenu,x,FALSE,&mi);     
                        SetDlgItemText(hwndDlg,IDC_BUTTON3,config_pres_subdir);
                    }
                    DestroyMenu(hMenu);
                }
                return 0;
                case IDC_CHECK1:
                    config_prompt_save_preset=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
                    return 0;
			            case IDC_CHECK3:
				            cfg_fs_rnd=IsDlgButtonChecked(hwndDlg,IDC_CHECK3)?1:0;
                #ifdef WA2_EMBED
       	            SendMessage(g_mod->hwndParent,WM_WA_IPC,cfg_fs_rnd,IPC_CB_VISRANDOM);
                #endif
			            return 0;
                case IDC_EDIT1:
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        BOOL f;
                        int r=GetDlgItemInt(hwndDlg,IDC_EDIT1,&f,0);
                        if (f) 
                        {
                            cfg_fs_rnd_time=r;
                        }
                    }
                return 0;
                case IDC_BUTTON1:
                    {
                        int w=SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
                        if (w != CB_ERR)
                        {
                            extern void WritePreset(int preset);
                            WritePreset(w);
                        }
                    }
                return 0;
                case IDC_BUTTON2:
                    if (readyToLoadPreset(hwndDlg,0)) 
                    {
                        int w=SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
                        if (w != CB_ERR)
                        {
                            extern int LoadPreset(int preset);
                            LoadPreset(w);
                        }
                    }
                return 0;
            }
	}
	return 0;
}

static BOOL CALLBACK DlgProc_Disp(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
    		if (ExtractWindowsVersion()<5)
            {
                EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_CHECK),0);
                EnableWindow(GetDlgItem(hwndDlg,IDC_TRANS_SLIDER),0);
            }  
            else
            {
  			    CheckDlgButton(hwndDlg,IDC_TRANS_CHECK,cfg_trans?BST_CHECKED:BST_UNCHECKED);
			    SendDlgItemMessage(hwndDlg,IDC_TRANS_SLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(16,255));
			    SendDlgItemMessage(hwndDlg,IDC_TRANS_SLIDER, TBM_SETTICFREQ, 10, (LPARAM)0);
			    SendDlgItemMessage(hwndDlg,IDC_TRANS_SLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)cfg_trans_amount);
            }
#ifdef LASER
      extern int g_laser_nomessage,g_laser_zones;              
      ShowWindow(GetDlgItem(hwndDlg,IDC_L_SUPPRESS_OUTPUT),SW_SHOWNA);
      ShowWindow(GetDlgItem(hwndDlg,IDC_L_SYNC),SW_SHOWNA);      
      ShowWindow(GetDlgItem(hwndDlg,IDC_L_SUPPRESS_DIALOGS),SW_SHOWNA);
      ShowWindow(GetDlgItem(hwndDlg,IDC_L_PROJZONES),SW_SHOWNA);
      ShowWindow(GetDlgItem(hwndDlg,IDC_L_FRAME),SW_SHOWNA);
      ShowWindow(GetDlgItem(hwndDlg,IDC_L_ACTIVEOUTPUT),SW_SHOWNA);     
      ShowWindow(GetDlgItem(hwndDlg,IDC_THREADSBORDER),SW_HIDE);
      ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK4),SW_HIDE);
      ShowWindow(GetDlgItem(hwndDlg,IDC_EDIT1),SW_HIDE);
      ShowWindow(GetDlgItem(hwndDlg,IDC_THREADS),SW_HIDE);
      CheckDlgButton(hwndDlg,IDC_L_SUPPRESS_DIALOGS,(g_laser_nomessage&1)?BST_CHECKED:BST_UNCHECKED);
      CheckDlgButton(hwndDlg,IDC_L_SUPPRESS_OUTPUT,(g_laser_nomessage&4)?BST_CHECKED:BST_UNCHECKED);
      CheckDlgButton(hwndDlg,IDC_L_SYNC,(g_laser_nomessage&8)?BST_CHECKED:BST_UNCHECKED);
      CheckDlgButton(hwndDlg,IDC_L_ACTIVEOUTPUT,(g_laser_nomessage&2)?BST_CHECKED:BST_UNCHECKED);
#else
            CheckDlgButton(hwndDlg,IDC_CHECK4,g_config_smp?BST_CHECKED:0);
            SetDlgItemInt(hwndDlg,IDC_EDIT1,g_config_smp_mt,FALSE);
#endif
		}
#ifdef WA2_EMBED
        {
            HWND w = myWindowState.me;
            while (GetWindowLong(w, GWL_STYLE) & WS_CHILD) w = GetParent(w);
            char classname[256];
            GetClassName(w, classname, 255); classname[255] = 0;
            if (!stricmp(classname, "BaseWindow_RootWnd")) 
            {
                EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_ALPHA), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_TRANS_CHECK), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_TRANS_SLIDER), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_TRANS_TOTAL), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_TRANS_NONE), FALSE);
            }
        }
#endif
        CheckDlgButton(hwndDlg,IDC_CHECK1,(cfg_fs_d&2)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hwndDlg,IDC_CHECK6,(cfg_fs_flip&4)?BST_UNCHECKED:BST_CHECKED);
        CheckDlgButton(hwndDlg,IDC_CHECK3,(cfg_fs_fps&2)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hwndDlg,IDC_CHECK5,(cfg_fs_fps&4)?BST_CHECKED:BST_UNCHECKED);
        //CheckDlgButton(hwndDlg,IDC_DONT_MIN_AVS,cfg_dont_min_avs?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hwndDlg,IDC_CHECK2,config_reuseonresize?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hwndDlg,IDC_BKGND_RENDER,(cfg_bkgnd_render&1)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hwndDlg,IDC_SETDESKTOPCOLOR,(cfg_bkgnd_render&2)?BST_CHECKED:BST_UNCHECKED);     
        SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMIN,0,0);
        SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMAX,0,80);
        SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,cfg_speed&0xff);
        SendDlgItemMessage(hwndDlg, IDC_THREAD_PRIORITY, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRING(IDS_SAME_AS_WINAMP));
        SendDlgItemMessage(hwndDlg, IDC_THREAD_PRIORITY, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRING(IDS_IDLE));
        SendDlgItemMessage(hwndDlg, IDC_THREAD_PRIORITY, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRING(IDS_LOWEST));
        SendDlgItemMessage(hwndDlg, IDC_THREAD_PRIORITY, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRING(IDS_NORMAL));
        SendDlgItemMessage(hwndDlg, IDC_THREAD_PRIORITY, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRING(IDS_HIGHEST));
        SendDlgItemMessage(hwndDlg, IDC_THREAD_PRIORITY, CB_SETCURSEL, cfg_render_prio, 0);
		return 1;

		case WM_DRAWITEM:
			{
				DRAWITEMSTRUCT *di=(DRAWITEMSTRUCT *)lParam;
				switch (di->CtlID)
				{
					case IDC_OVERLAYCOLOR:
						GR_DrawColoredButton(di,cfg_bkgnd_render_color);
					break;
				}
			}
		return 0;
		case WM_HSCROLL:
			{
				HWND swnd = (HWND) lParam;
				int t = (int) SendMessage(swnd,TBM_GETPOS,0,0);
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER1))
				{
                    cfg_speed&=~0xff;
					cfg_speed|=t;
				}
				if (swnd == GetDlgItem(hwndDlg,IDC_TRANS_SLIDER))
				{
					cfg_trans_amount=t;
					SetTransparency(g_hwnd,cfg_trans,cfg_trans_amount);
				}
			}
		return 0;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
#ifdef LASER
        case IDC_L_PROJZONES:
          {
            extern int g_laser_zones;
            HMENU hMenu=CreatePopupMenu();
            int x;
            for (x=0;x <20; x++)
            {
              PROJECTIONZONE pz;
              if (ReadProjectionZone(x+1,&pz))
                wsprintf(pz.Name,"Zone %d",x);
              if (!pz.Name[0]) break;

              MENUITEMINFO i={sizeof(i),};
              i.wID = x+1;
              i.fMask=MIIM_TYPE|MIIM_ID|MIIM_STATE;
              i.fState=(g_laser_zones&(1<<x))?MFS_CHECKED:0;
              i.fType=MFT_STRING;
              i.dwTypeData=pz.Name;
              i.cch=strlen(pz.Name);
              InsertMenuItem(hMenu,x,TRUE,&i);
            }

            RECT r;
            GetWindowRect(GetDlgItem(hwndDlg,IDC_L_PROJZONES),&r);

            int v=TrackPopupMenu(hMenu,TPM_NONOTIFY|TPM_RETURNCMD,r.right,r.top,0,GetDlgItem(hwndDlg,IDC_L_PROJZONES),NULL);
            DestroyMenu(hMenu);
            if (v > 0)
            {
              g_laser_zones^=1<<(v-1);
              if (!g_laser_zones) g_laser_zones=1;
            }
          }

        return 0;
        case IDC_L_SYNC:
          {
            extern int g_laser_nomessage;
            g_laser_nomessage&=~8;
            g_laser_nomessage|=IsDlgButtonChecked(hwndDlg,IDC_L_SYNC)?8:0;
          }
        return 0;
        case IDC_L_SUPPRESS_OUTPUT:
          {
            extern int g_laser_nomessage;
            g_laser_nomessage&=~4;
            g_laser_nomessage|=IsDlgButtonChecked(hwndDlg,IDC_L_SUPPRESS_OUTPUT)?4:0;
          }
        return 0;
        case IDC_L_SUPPRESS_DIALOGS:
          {
            extern int g_laser_nomessage;
            g_laser_nomessage&=~1;
            g_laser_nomessage|=IsDlgButtonChecked(hwndDlg,IDC_L_SUPPRESS_DIALOGS)?1:0;
          }
        return 0;
        case IDC_L_ACTIVEOUTPUT:
          {
            extern int g_laser_nomessage;
            g_laser_nomessage&=~2;
            g_laser_nomessage|=IsDlgButtonChecked(hwndDlg,IDC_L_ACTIVEOUTPUT)?2:0;
          }
        return 0;
#else
        case IDC_CHECK4:
            g_config_smp=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK4);
        return 0;
        case IDC_EDIT1:
          {
                BOOL t;
                g_config_smp_mt=GetDlgItemInt(hwndDlg,IDC_EDIT1,&t,FALSE);
          }
        return 0;
#endif
			    case IDC_TRANS_CHECK:
					cfg_trans=IsDlgButtonChecked(hwndDlg,IDC_TRANS_CHECK)?1:0;
					SetTransparency(g_hwnd,cfg_trans,cfg_trans_amount);
					return 0;
				case IDC_CHECK1:
                    cfg_fs_d&=~2;
					cfg_fs_d|=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?2:0;
                    {
				        RECT r;
                        extern void GetClientRect_adj(HWND hwnd, RECT *r);
				        GetClientRect_adj(g_hwnd,&r);
				        DDraw_Resize(r.right,r.bottom,cfg_fs_d&2);
                    }

				return 0;
				case IDC_CHECK6:
                    cfg_fs_flip&=~4;
					cfg_fs_flip|=IsDlgButtonChecked(hwndDlg,IDC_CHECK6)?0:4;
				return 0;
				case IDC_CHECK3:
                    cfg_fs_fps&=~2;
					cfg_fs_fps|=IsDlgButtonChecked(hwndDlg,IDC_CHECK3)?2:0;
				return 0;
				case IDC_CHECK5:
                    cfg_fs_fps&=~4;
					cfg_fs_fps|=IsDlgButtonChecked(hwndDlg,IDC_CHECK5)?4:0;
				return 0;
                case IDC_CHECK2:
                    config_reuseonresize = !!IsDlgButtonChecked(hwndDlg,IDC_CHECK2);
				    return 0;
//				case IDC_DONT_MIN_AVS:
	//				cfg_dont_min_avs=IsDlgButtonChecked(hwndDlg,IDC_DONT_MIN_AVS)?1:0;
                case IDC_DEFOVERLAYCOLOR:
                    cfg_bkgnd_render_color=0x1F000F;
					InvalidateRect(GetDlgItem(hwndDlg,IDC_OVERLAYCOLOR),NULL,FALSE);
                goto update_overlayshit;
				case IDC_OVERLAYCOLOR:
                    GR_SelectColor(hwndDlg,&cfg_bkgnd_render_color);
				    InvalidateRect(GetDlgItem(hwndDlg,IDC_OVERLAYCOLOR),NULL,FALSE);
                    goto update_overlayshit;
                case IDC_SETDESKTOPCOLOR:
                case IDC_BKGND_RENDER:
                    cfg_bkgnd_render=(IsDlgButtonChecked(hwndDlg,IDC_BKGND_RENDER)?1:0) |
                        (IsDlgButtonChecked(hwndDlg,IDC_SETDESKTOPCOLOR)?2:0);
        update_overlayshit:
        {
				  RECT r;
          extern void GetClientRect_adj(HWND hwnd, RECT *r);
				  GetClientRect_adj(g_hwnd,&r);
				  DDraw_Resize(r.right,r.bottom,cfg_fs_d&2);
        }
        return 0;
        case IDC_THREAD_PRIORITY:
            extern void main_setRenderThreadPriority();
            cfg_render_prio=SendDlgItemMessage(hwndDlg,IDC_THREAD_PRIORITY,CB_GETCURSEL,0,0);
            main_setRenderThreadPriority();
        return 0;
      }
		return 0;
	}
	return 0;
}

static void enableFSWindows(HWND hwndDlg, int v)
{
  EnableWindow(GetDlgItem(hwndDlg,IDC_BPP_CONV),v);
  //EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),v);
  EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK4),v);
  EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK2),v);
  EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK3),v);
  EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK5),v);
}

extern int cfg_fs_dblclk;

static BOOL CALLBACK DlgProc_FS(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
			int l;
			DDraw_EnumDispModes(GetDlgItem(hwndDlg,IDC_COMBO1));
			l=SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCOUNT,0,0);
			if (l < 1 || l == CB_ERR)
			{
				SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRING(IDS_NO_SUITABLE_MODES_FOUND));
				SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_SETCURSEL,0,0);
			}
			else
			{
				int x;
				for (x = 0; x < l; x ++)
				{
					char b[256],*p=b;
					SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETLBTEXT,x,(long)b);
					int w,h,bpp;
					w=atoi(p);
					while (*p >= '0' && *p <= '9') p++;
					if (!*p) continue;
          h=atoi(++p);
					while (*p >= '0' && *p <= '9') p++;
					if (!*p) continue;
					bpp=atoi(++p);
					if (w == cfg_fs_w && h == cfg_fs_h && bpp == cfg_fs_bpp)
						break;
				}
				if (x != l) 
					SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_SETCURSEL,x,0);
			}
			CheckDlgButton(hwndDlg,IDC_USE_OVERLAY,cfg_fs_use_overlay?BST_CHECKED:BST_UNCHECKED);
      enableFSWindows(hwndDlg,!cfg_fs_use_overlay);
      EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK8), cfg_fs_use_overlay);
			CheckDlgButton(hwndDlg,IDC_CHECK1,(cfg_fs_d&1)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_CHECK2,(cfg_fs_fps&1)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_CHECK3,(cfg_fs_fps&8)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_CHECK5,(cfg_fs_fps&16)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_CHECK4,(cfg_fs_flip&1)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_CHECK7,(cfg_fs_flip&2)?BST_UNCHECKED:BST_CHECKED);
			CheckDlgButton(hwndDlg,IDC_CHECK8,(cfg_cancelfs_on_deactivate)?BST_UNCHECKED:BST_CHECKED);
			CheckDlgButton(hwndDlg,IDC_BPP_CONV,(cfg_fs_flip&8)?BST_UNCHECKED:BST_CHECKED);
      SetDlgItemInt(hwndDlg,IDC_EDIT1,cfg_fs_height,FALSE);
      SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMAX,0,80);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,(cfg_speed>>8)&0xff);      
      CheckDlgButton(hwndDlg,IDC_CHECK6,cfg_fs_dblclk?BST_CHECKED:BST_UNCHECKED);
			}
			return 1;
		case WM_HSCROLL:
			{
				HWND swnd = (HWND) lParam;
				int t = (int) SendMessage(swnd,TBM_GETPOS,0,0);
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER1))
				{
          cfg_speed&=~0xff00;
					cfg_speed|=(t<<8);
				}
			}
		return 0;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
        case IDC_BUTTON1:
          if (IsDlgButtonChecked(hwndDlg,IDC_USE_OVERLAY) || DDraw_IsMode(cfg_fs_w,cfg_fs_h,cfg_fs_bpp)) 
          {
            SetForegroundWindow(g_hwnd);
            PostMessage(g_hwnd,WM_USER+32,0,0);
          } else
          {
            char title[32];
            MessageBox(hwndDlg,WASABI_API_LNGSTRING(IDS_CHOOSE_A_VIDEO_MODE),
            WASABI_API_LNGSTRING_BUF(IDS_AVS_FULLSCREEN,title,32),MB_OK);
          }
        return 0;
        case IDC_BPP_CONV:
          cfg_fs_flip&=~8;
					cfg_fs_flip|=IsDlgButtonChecked(hwndDlg,IDC_BPP_CONV)?0:8;
        return 0;
        case IDC_CHECK6:
          cfg_fs_dblclk=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK6);
        return 0;
				case IDC_CHECK1:
          cfg_fs_d&=~1;
					cfg_fs_d|=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
				return 0;
				case IDC_CHECK2:
          cfg_fs_fps&=~1;
					cfg_fs_fps|=IsDlgButtonChecked(hwndDlg,IDC_CHECK2)?1:0;
				return 0;
				case IDC_CHECK3:
          cfg_fs_fps&=~8;
					cfg_fs_fps|=IsDlgButtonChecked(hwndDlg,IDC_CHECK3)?8:0;
				return 0;
				case IDC_CHECK5:
          cfg_fs_fps&=~16;
					cfg_fs_fps|=IsDlgButtonChecked(hwndDlg,IDC_CHECK5)?16:0;
				return 0;
				case IDC_CHECK4:
          cfg_fs_flip&=~1;
					cfg_fs_flip|=IsDlgButtonChecked(hwndDlg,IDC_CHECK4)?1:0;
				return 0;
				case IDC_CHECK7:
          cfg_fs_flip&=~2;
					cfg_fs_flip|=IsDlgButtonChecked(hwndDlg,IDC_CHECK7)?0:2;
				return 0;
        case IDC_CHECK8:
          cfg_cancelfs_on_deactivate = IsDlgButtonChecked(hwndDlg,IDC_CHECK8)?0:1;
          return 0;
        case IDC_EDIT1:
          if (HIWORD(wParam) == EN_CHANGE) {
            BOOL t;
            int r=GetDlgItemInt(hwndDlg,IDC_EDIT1,&t,FALSE);
            if (r > 0 && r <= 100 && t)
              cfg_fs_height=r;
          }
        return 0;
				case IDC_COMBO1:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						int bps=-1;
						char b[256],*p=b;
						int l=SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
						if (l == CB_ERR) return 0;
						SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETLBTEXT,l,(long)b);
						int w,h;
						while (*p >= '0' && *p <= '9') p++;
						if (!*p) return 0;
						*p++=0;
						w=atoi(b);
						while (*p < '0' && *p > '9' && *p) p ++;
						h=atoi(p);
						while (*p >= '0' && *p <= '9') p++;
						if (!*p) return 0;
						p++;
						bps=atoi(p);
						if (w < 1|| h < 1 || bps < 1) return 0;
						cfg_fs_h=h;
						cfg_fs_w=w;
						cfg_fs_bpp=bps;
					}
				return 0;
        case IDC_USE_OVERLAY:
					cfg_fs_use_overlay=IsDlgButtonChecked(hwndDlg,IDC_USE_OVERLAY)?1:0;
          enableFSWindows(hwndDlg,!cfg_fs_use_overlay);
          EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK8), cfg_fs_use_overlay);
        return 0;
			}
		return 0;
	}
	return 0;
}


static void _insertintomenu2(HMENU hMenu, int wid, int id, char *str)
{
  int x;
  for (x=0; x < 4096; x ++)
  {
	  MENUITEMINFO mi={sizeof(mi),MIIM_DATA|MIIM_TYPE|MIIM_SUBMENU,MFT_STRING};
    char c[512];
	  mi.dwTypeData=c;
	  mi.cch = 512;
	  if (!GetMenuItemInfo(hMenu,x,TRUE,&mi)) break;
    if (strcmp(str,c)<0 && !mi.hSubMenu)
      break;
  }

  MENUITEMINFO i={sizeof(i),};
  i.wID = wid;
  i.fMask=MIIM_TYPE|MIIM_DATA|MIIM_ID;
  i.fType=MFT_STRING;
  i.dwItemData=id;
  i.dwTypeData=str;
  i.cch=strlen(str);
  InsertMenuItem(hMenu,x,TRUE,&i);
}

static HMENU _findsubmenu(HMENU hmenu, char *str)
{
  int x;
  for (x=0; x < 4096; x ++)
  {
	  MENUITEMINFO mi={sizeof(mi),MIIM_DATA|MIIM_TYPE|MIIM_SUBMENU,MFT_STRING};
    char c[512];
	  mi.dwTypeData=c;
	  mi.cch = 512;
	  if (!GetMenuItemInfo(hmenu,x,TRUE,&mi)) break;
    if (!strcmp(str,c) && mi.hSubMenu) return mi.hSubMenu;
  }
  return 0;
}

static void _insertintomenu(HMENU hMenu, int wid, int id, char *str)
{
  char ostr[1024];
  strncpy(ostr,str,1023);
  char *first=str;
  char *second=str;
  while (*second && *second != '/') second++;
  if (*second) *second++=0;
  if (*second)
  {
    while (*second == ' ' || *second == '/') second++;
    if (*second)
    {
      HMENU hs;

      if (!(hs=_findsubmenu(hMenu,first)))
      {
        MENUITEMINFO i={sizeof(i),};
        i.fMask=MIIM_TYPE|MIIM_SUBMENU|MIIM_DATA|MIIM_ID;
        i.fType=MFT_STRING;
				i.dwTypeData = first;
				i.cch = strlen(first);
        i.hSubMenu=hs=CreatePopupMenu();
        i.wID=0;
        InsertMenuItem(hMenu,0,TRUE,&i);
      }
      _insertintomenu2(hs,wid,id,second);
      return;
    }
  }
  _insertintomenu2(hMenu,wid,id,ostr);

}
static HTREEITEM g_dragsource_item,g_dragsource_parent, g_draglastdest, g_dragplace;
static int g_dragplaceisbelow;
extern int findInMenu(HMENU parent, HMENU sub, UINT id, char *buf, int buf_len);

#define UNDO_TIMER_INTERVAL 333

static WNDPROC sniffConfigWindow_oldProc;
static BOOL CALLBACK sniffConfigWindow_newProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  bool dirty = false;
  if (uMsg == WM_COMMAND || uMsg == WM_HSCROLL || uMsg == WM_VSCROLL) 
  {
    if (uMsg != WM_COMMAND || HIWORD(wParam) == EN_CHANGE || 
                              HIWORD(wParam) == BN_CLICKED ||
                              HIWORD(wParam) == LBN_SELCHANGE ||
                              HIWORD(wParam) == CBN_SELCHANGE
                              
      )
      dirty = true;
  }

  BOOL retval = CallWindowProc(sniffConfigWindow_oldProc,hwndDlg,uMsg,wParam,lParam);

  // Don't save the new state until the window proc handles it.  :)
  if (dirty)
  {
    KillTimer(GetParent(hwndDlg),69);
    SetTimer(GetParent(hwndDlg),69,UNDO_TIMER_INTERVAL,NULL);


//      g_preset_dirty=1;
  }

  return retval;
}


int dosavePreset(HWND hwndDlg)
{
  int r=1;
	char temp[2048];
	OPENFILENAME l={sizeof(l),0};
	char buf1[2048],buf2[2048];
	char filter[128], *ptr = filter;
	char savePreset[64];

	WASABI_API_LNGSTRING_BUF(IDS_AVS_FILTER_STRING,filter,128);
	while(*ptr)
	{
		if (*ptr=='|') *ptr=0;
		ptr++;
	}
  temp[0]=0;
	GetCurrentDirectory(sizeof(buf2),buf2);
	strcpy(buf1,g_path);
	l.hwndOwner = hwndDlg;
	l.lpstrFilter = filter;
	l.lpstrFile = temp;
	strcpy(temp,last_preset);
	l.nMaxFile = 2048-1;
	l.lpstrTitle = WASABI_API_LNGSTRING_BUF(IDS_SAVE_PRESET,savePreset,64);
	l.lpstrDefExt = "AVS";
	l.lpstrInitialDir = buf1;
	l.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_OVERWRITEPROMPT; 	        
	if (GetSaveFileName(&l)) 
	{
		strcpy(last_preset,temp); // this was changed from: lstrcpyn(last_preset,temp, 2048);
    r=g_render_effects->__SavePreset(temp);
		if (r==1) MessageBox(hwndDlg,WASABI_API_LNGSTRING(IDS_ERROR_SAVING_PRESET),savePreset,MB_OK);
		else if (r==2) MessageBox(hwndDlg,WASABI_API_LNGSTRING(IDS_PRESET_TOO_LARGE),savePreset,MB_OK);
		else if (r==-1) MessageBox(hwndDlg,WASABI_API_LNGSTRING(IDS_OUT_OF_MEMORY),savePreset,MB_OK);
    else 
    {
      C_UndoStack::cleardirty();
      // g_preset_dirty=0;
    }
	} 
	SetCurrentDirectory(buf2);
  return r;
}

extern int g_config_seh;

static BOOL CALLBACK debugProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  extern int debug_reg[8];
  switch (uMsg)
  {
    case WM_INITDIALOG:
      {
        int x;
        for (x = 0; x < 8; x ++)
        {
          SetDlgItemInt(hwndDlg,IDC_DEBUGREG_1+x*2,debug_reg[x],FALSE);
        }
        SetTimer(hwndDlg,1,250,NULL);
      }
      if (g_log_errors) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
      if (g_reset_vars_on_recompile) CheckDlgButton(hwndDlg,IDC_CHECK2,BST_CHECKED);
      if (!g_config_seh) CheckDlgButton(hwndDlg,IDC_CHECK3,BST_CHECKED);     
     
    return 0;
    case WM_TIMER:
      if (wParam == 1)
      {
        int x;
        for (x = 0; x < 8; x ++)
        {
          char buf[128];
          int v=debug_reg[x];
          if (v >=0 && v < 100)
            sprintf(buf,"%.14f",NSEEL_getglobalregs()[v]);
          else strcpy(buf,"?");
          SetDlgItemText(hwndDlg,IDC_DEBUGREG_1+x*2+1,buf);
        }

        if (g_log_errors)
        {
          //IDC_EDIT1
          EnterCriticalSection(&g_eval_cs);
          char buf[1025];
          GetDlgItemText(hwndDlg,IDC_EDIT1,buf,sizeof(buf)-1);
          buf[sizeof(buf)-1]=0;
          if (strcmp(buf,last_error_string))
            SetDlgItemText(hwndDlg,IDC_EDIT1,last_error_string);
          LeaveCriticalSection(&g_eval_cs);
        }

        {
          char buf[512];
          int *g_evallib_stats=NSEEL_getstats();
          wsprintf(buf,WASABI_API_LNGSTRING(IDS_EVAL_CODE_STATS_ETC),
            g_evallib_stats[4],g_evallib_stats[0],g_evallib_stats[1],g_evallib_stats[2],g_evallib_stats[3]);
          SetDlgItemText(hwndDlg,IDC_EDIT2,buf);
        }

      }
    return 0;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_CHECK1:
          g_log_errors = !!IsDlgButtonChecked(hwndDlg,IDC_CHECK1);
        return 0;
        case IDC_CHECK2:
          g_reset_vars_on_recompile = !!IsDlgButtonChecked(hwndDlg,IDC_CHECK2);
        return 0;     
        case IDC_CHECK3:
          g_config_seh = !IsDlgButtonChecked(hwndDlg,IDC_CHECK3);
        return 0;             
        case IDC_BUTTON1:
          EnterCriticalSection(&g_eval_cs);
          last_error_string[0]=0;
          SetDlgItemText(hwndDlg,IDC_EDIT1,"");
          LeaveCriticalSection(&g_eval_cs);
        return 0;
        case IDOK:
        case IDCANCEL:
          DestroyWindow(hwndDlg);
        return 0;
        default:
          if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) >= IDC_DEBUGREG_1 && LOWORD(wParam) <= IDC_DEBUGREG_16)
          {
            int x=LOWORD(wParam)-IDC_DEBUGREG_1;
            if (!(x&1))
            {
              x/=2;
              if (x > 7) x= 7;
              BOOL t;
              int v=GetDlgItemInt(hwndDlg,IDC_DEBUGREG_1+x*2,&t,FALSE);
              if (t) debug_reg[x]=v;
            }
          }
        break;
      }
    return 0;
    case WM_DESTROY:
      g_debugwnd=0;
    return 0;
  }
  return 0;
}

static BOOL CALLBACK dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    static HMENU presetTreeMenu;
    static int presetTreeCount;
    extern int need_redock;
    extern int inWharf;
    extern void toggleWharfAmpDock(HWND hwnd);

	switch (uMsg)
	{
        case WM_INITMENU:
          EnableMenuItem((HMENU)wParam,IDM_UNDO,MF_BYCOMMAND|(C_UndoStack::can_undo()?MF_ENABLED:MF_GRAYED));
          EnableMenuItem((HMENU)wParam,IDM_REDO,MF_BYCOMMAND|(C_UndoStack::can_redo()?MF_ENABLED:MF_GRAYED));
        return 0;
        case WM_INITMENUPOPUP:
            if (!HIWORD(lParam) && presetTreeMenu && !GetMenuItemCount((HMENU)wParam))
            {
                char buf[2048];
                buf[0]=0;
                if (findInMenu(presetTreeMenu,(HMENU)wParam,0,buf,2048))
                {
		            HANDLE h;
		            WIN32_FIND_DATA d;
  		            char dirmask[4096];
                    wsprintf(dirmask,"%s%s\\*.*",g_path,buf);
                    int directory_pos=0, insert_pos=0;
                    // build menu
		            h = FindFirstFile(dirmask,&d);
		            if (h != INVALID_HANDLE_VALUE)
		            {
			            do 
                        {
                            if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && d.cFileName[0] != '.')
                            {
                                MENUITEMINFO mi={sizeof(mi),MIIM_SUBMENU|MIIM_TYPE,MFT_STRING,MFS_DEFAULT };
                                mi.hSubMenu=CreatePopupMenu();
                                mi.dwTypeData=d.cFileName;
                                mi.cch = strlen(d.cFileName);
                                InsertMenuItem((HMENU)wParam,directory_pos++,TRUE,&mi);
                                insert_pos++;
                            }
                            else if (!stricmp(extension(d.cFileName),"avs"))
                            {
	                            extension(d.cFileName)[-1]=0;
                                MENUITEMINFO i={sizeof(i),MIIM_TYPE|MIIM_DATA|MIIM_ID,MFT_STRING,MFS_DEFAULT };
	                            i.dwTypeData = d.cFileName;
	                            i.cch = strlen(d.cFileName);
                                i.dwItemData=0xFFFFFFFF;//preset
	                            i.wID=presetTreeCount++;
	                            InsertMenuItem((HMENU)wParam,insert_pos++,TRUE,&i);
                            }
			            } while (FindNextFile(h,&d));
			            FindClose(h);
		            }
                }
            }
            return 0;
        case WM_USER+20:
            CfgWnd_Unpopulate();
            CfgWnd_Populate();
        return 0;
		case WM_CLOSE:
        if (inWharf)
        {
#ifdef WA2_EMBED
            toggleWharfAmpDock(g_hwnd);
        }
#else
            PostMessage(g_hwnd,WM_CLOSE,0,0);
        }
        else
#endif
        {
            cfg_cfgwnd_open=0;
				    ShowWindow(hwndDlg,SW_HIDE);
        }
		return 0;
        case WM_DESTROY:
        return 0;
	    case WM_INITDIALOG:
        {
        #ifdef LASER
            HMENU m=GetMenu(hwndDlg);
            m=GetSubMenu(m,1);
            DeleteMenu(m,IDM_TRANSITIONS,MF_BYCOMMAND);
        #endif
            g_hwndDlg=hwndDlg;
            //SetDlgItemText(hwndDlg,IDC_AVS_VER,verstr);
	        WASABI_API_LNGSTRING_BUF(IDS_NO_EFFECT_SETTING_SELECTED,g_noeffectstr,48);
        
            TreeView_SetIndent(GetDlgItem(hwndDlg,IDC_TREE1),8);
            SetTimer(hwndDlg,1,250,NULL);
            if (cfg_cfgwnd_open) 
                ShowWindow(hwndDlg,SW_SHOWNA);
            CfgWnd_Populate();
            SetWindowPos(hwndDlg,NULL,cfg_cfgwnd_x,cfg_cfgwnd_y,0,0,SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
		    if (need_redock)
            {
                need_redock=0;
                toggleWharfAmpDock(g_hwnd);
            }
            else
            {
	            RECT r,r2;
                // hide IDC_RRECT, resize IDC_TREE1 up
                GetWindowRect(GetDlgItem(g_hwndDlg,IDC_RRECT),&r);
                GetWindowRect(GetDlgItem(g_hwndDlg,IDC_TREE1),&r2);
                ShowWindow(GetDlgItem(g_hwndDlg,IDC_RRECT),SW_HIDE);
                SetWindowPos(GetDlgItem(g_hwndDlg,IDC_TREE1),NULL,0,0,r2.right-r2.left,r.bottom - r2.top - 2,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
            }
        }
            return TRUE;
        case WM_TIMER:
            if (wParam == 1)
            {
			    char s[1024];
                char *tp=last_preset;
                while (*tp) tp++;
                while (tp >= last_preset && *tp != '\\') 
                    tp--;
                tp++;
                wsprintf(s,"%d.%d FPS @ %dx%d%s%s",g_dlg_fps/10,g_dlg_fps%10,g_dlg_w,g_dlg_h,*tp?" - ":".",tp);
                tp=s;
                while (*tp) tp++;
                while (tp > s && *tp != '.' && *tp != '-') tp--;
                if (*tp == '.') *tp=0;
                SetDlgItemText(hwndDlg,IDC_FPS,s);
            }
            if (wParam == 69)
            {
                KillTimer(hwndDlg,69);
                C_UndoStack::saveundo();
            }
        return FALSE;
        case WM_MOUSEMOVE:
            if (g_dragsource_item) 
            {
                TVHITTESTINFO hti={0,};
                HWND hwnd=GetDlgItem(hwndDlg,IDC_TREE1);
                hti.pt.x=(int)LOWORD(lParam);
                hti.pt.y=(int)HIWORD(lParam);
                ClientToScreen(hwndDlg,&hti.pt);
                ScreenToClient(hwnd,&hti.pt);
                HTREEITEM h=TreeView_HitTest(hwnd,&hti);
                if (hti.flags&TVHT_ABOVE)
                {
                    SendMessage(hwnd,WM_VSCROLL,SB_LINEUP,NULL);
                }
                if (hti.flags&TVHT_BELOW)
                {
                    SendMessage(hwnd,WM_VSCROLL,SB_LINEDOWN,NULL);
                }
                if (hti.flags&TVHT_NOWHERE)
                {
                    h=g_hroot;
                }
                if ((hti.flags&(TVHT_NOWHERE|TVHT_ONITEMINDENT|TVHT_ONITEMRIGHT|TVHT_ONITEM|TVHT_ONITEMBUTTON))&&h)
                {
                    HTREEITEM temp=h;
                    while (temp && temp != TVI_ROOT)
                    {
                        if (temp==g_dragsource_item) 
                        {
                            h=g_dragsource_item;
                            break;
                        }
                        temp=TreeView_GetParent(hwnd,temp);
                    }
                    if (h == g_dragsource_item)
                    {
                        SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_NO)));
                        if (g_dragplace) TreeView_DeleteItem(hwnd,g_dragplace);
                        g_dragplace=0;
                    }
                    else
                    {
                        SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
  	                        TV_ITEM i={TVIF_HANDLE|TVIF_PARAM,h,0,0,0,0,0};
  		                        TreeView_GetItem(hwnd,&i);
                        if(i.lParam)
                        {
                            RECT r;
                            TreeView_GetItemRect(hwnd,h,&r,FALSE);
                            if (hti.pt.y > r.bottom-(r.bottom-r.top)/2) g_dragplaceisbelow=1;
                            else g_dragplaceisbelow=0;
                            HTREEITEM parenth;
	                        g_draglastdest=h;
                            C_RenderListClass::T_RenderListType *it=(C_RenderListClass::T_RenderListType *)i.lParam;

                            if (it->effect_index==LIST_ID && (hti.flags&(TVHT_ONITEMINDENT|TVHT_ONITEMBUTTON)||h==g_hroot))
                            {
                                if (g_dragplace && (TreeView_GetParent(hwnd,g_dragplace)!=h || 
                                    TreeView_GetNextSibling(hwnd,g_dragplace)))
                                {
                                    TreeView_DeleteItem(hwnd,g_dragplace);
                                    g_dragplace=0;
                                }

                                g_dragplaceisbelow=2;
                                parenth=h;
                                h=TVI_LAST;
                            }
                            else
                            {            
                                parenth=TreeView_GetParent(hwnd,h);
                                if (g_dragplace && ((g_dragplaceisbelow&1)?
                                    TreeView_GetNextSibling(hwnd,h)!=g_dragplace:
                                    TreeView_GetPrevSibling(hwnd,h)!=g_dragplace))
                                {
                                    TreeView_DeleteItem(hwnd,g_dragplace);
                                    g_dragplace=0;
                                }
                                if (!g_dragplaceisbelow)
                                {
                                    h=TreeView_GetPrevSibling(hwnd,h);
                                    if (!h) h=TVI_FIRST;
                                }
                            }
                            if (!g_dragplace)
                            {
                                TV_INSERTSTRUCT is={parenth,h,{TVIF_PARAM|TVIF_TEXT|TVIF_CHILDREN,0,0,0,
                                WASABI_API_LNGSTRING(IDS_MOVE_HERE),0,0,0,0,(int)0}};
                                g_dragplace=TreeView_InsertItem(hwnd,&is);
                                if (g_dragplaceisbelow==2)
                                    SendMessage(hwnd,TVM_EXPAND,TVE_EXPAND,(long)parenth);
                            }
                        }
                    }
                }
            return 0;
            }
        break;
        case WM_LBUTTONUP:
            if (g_dragsource_item) 
            {
                HWND hwnd=GetDlgItem(hwndDlg,IDC_TREE1);
                SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
                if (g_dragplace) 
                {
                    TreeView_DeleteItem(hwnd,g_dragplace);
                    g_dragplace=0;
                }
                HTREEITEM h=g_draglastdest;
            if (h)
            {
                C_RenderListClass::T_RenderListType *source, *source_parent;
                C_RenderListClass::T_RenderListType *dest,  // handle of item to insert above. NULL if end.
                        *dest_parent=NULL;  // handle of parent to insert into
                HTREEITEM dest_handle=h,  // handle of item to insert above. NULL if folder.
                dest_parent_handle=TreeView_GetParent(hwnd,h);  // handle of parent
                {
				        TV_ITEM i={TVIF_HANDLE|TVIF_PARAM,dest_handle,0,0,0,0,0};
				        TreeView_GetItem(hwnd,&i);
                dest=(C_RenderListClass::T_RenderListType *)i.lParam;
                }
                if (dest_parent_handle)
                {
				        TV_ITEM i={TVIF_HANDLE|TVIF_PARAM,dest_parent_handle,0,0,0,0,0};
				        TreeView_GetItem(hwnd,&i);
                dest_parent=(C_RenderListClass::T_RenderListType *)i.lParam;
                }
                if (dest->effect_index==LIST_ID && (!dest_parent_handle||g_dragplaceisbelow==2))
                {
                dest_parent_handle=dest_handle;
                dest_handle=NULL;
                dest_parent=dest;
                dest=NULL;
                }

				TV_ITEM i={TVIF_HANDLE|TVIF_PARAM|TVIF_STATE,g_dragsource_item,0,TVIS_EXPANDED,0,0,0};
				TreeView_GetItem(hwnd,&i);
                int expand=i.state&TVIS_EXPANDED;
                source=(C_RenderListClass::T_RenderListType *)i.lParam;

				TV_ITEM i2={TVIF_HANDLE|TVIF_PARAM,g_dragsource_parent,0,0,0,0,0};
				TreeView_GetItem(hwnd,&i2);
                source_parent=(C_RenderListClass::T_RenderListType *)i2.lParam;

                int recurse_okay=1;
                {
                    HTREEITEM temp=dest_parent_handle;
                    while (temp && temp != TVI_ROOT)
                    {
                        if (temp==g_dragsource_item) recurse_okay=0;
                        temp=TreeView_GetParent(hwnd,temp);
                    }
                }
                if (dest_handle != g_dragsource_item && recurse_okay)
                {
                    C_RenderListClass *s=(C_RenderListClass *)source_parent->render;
                    C_RenderListClass *d=(C_RenderListClass *)dest_parent->render;
                    int os=0;
                    int a=s->findRender(source);
                    int b=d->findRender(dest);
                    int err=1;
                    if (a >= 0)
                    {
                        EnterCriticalSection(&g_render_cs);
                        err=s->removeRender(a,0);
                        if (!err)
                        {
                            d->insertRender(source,b+(g_dragplaceisbelow&1));
                        }
                        LeaveCriticalSection(&g_render_cs);       
                    }
                    if (err) 
                    {
		                char title[48];
		                MessageBox(NULL,WASABI_API_LNGSTRING(IDS_ERROR_TREE_INCONSISTANCY),
		                WASABI_API_LNGSTRING_BUF(IDS_CRITICAL_ERROR_OCCURRED,title,48),MB_OK);
                    }

                    treeview_hack=1;
                    TreeView_DeleteItem(hwnd,g_dragsource_item);

                    TV_INSERTSTRUCT is={dest_parent_handle,0,{TVIF_PARAM|TVIF_TEXT|TVIF_CHILDREN,0,0,0,source->render->get_desc(),0,0,0,source->effect_index==LIST_ID?1:0,(int)source}};
            
                    if (dest_handle)
                    {
                        if (g_dragplaceisbelow&1) is.hInsertAfter=dest_handle;
                        else 
                        {
                        is.hInsertAfter=TreeView_GetPrevSibling(hwnd,dest_handle);
                        if (!is.hInsertAfter) is.hInsertAfter=TVI_FIRST;
                        }
                    }
                    else is.hInsertAfter=TVI_LAST;

                    HTREEITEM newi=TreeView_InsertItem(hwnd,&is);
                    if (source->effect_index == LIST_ID) 
                    {
                      _do_add(hwnd,newi,(C_RenderListClass *)source->render);
                      if (expand) SendMessage(hwnd,TVM_EXPAND,TVE_EXPAND,(long)newi);
                    }
                    TreeView_Select(hwnd,newi,TVGN_CARET);
                    treeview_hack=0;

                    // After everything is changed, then save the undo and set the dirty bit.
                    KillTimer(hwndDlg,69);
                    SetTimer(hwndDlg,69,UNDO_TIMER_INTERVAL,NULL);
                    // g_preset_dirty=1;
                }
            }
            TreeView_Select(hwnd,NULL,TVGN_DROPHILITE);
            ReleaseCapture();
            g_dragsource_item=0;
            return 0;
          }
        break;
       
	    case WM_NOTIFY:
		{
            NM_TREEVIEW* p = (NM_TREEVIEW*)lParam;
            if (p->hdr.hwndFrom == GetDlgItem(hwndDlg, IDC_TREE1))
            {
                // Element is being dragged
                if (p->hdr.code==TVN_BEGINDRAG)
                {
                    if (p->itemNew.hItem != g_hroot)
                    {
                        g_draglastdest=0;
                        g_dragsource_parent=TreeView_GetParent(p->hdr.hwndFrom,p->itemNew.hItem);
                        if (g_dragsource_parent) 
                        {
                            SetCapture(hwndDlg);
                            g_dragsource_item=p->itemNew.hItem;
                            g_dragplace=0;

                            TreeView_Select(p->hdr.hwndFrom,g_dragsource_item,TVGN_CARET);
                            SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_APPSTARTING)));
                        }
                        else 
                            g_dragsource_item=NULL;
                    }
                    else
                    g_dragsource_item=NULL;
                }
                // new element is selected selectis
	            if ((p->hdr.code== TVN_SELCHANGEDW || p->hdr.code == TVN_SELCHANGEDA) && !treeview_hack)
	            {
		            HTREEITEM hTreeItem = TreeView_GetSelection(p->hdr.hwndFrom);
                    if (hTreeItem)
                    {
		                TV_ITEM i={TVIF_HANDLE|TVIF_PARAM,hTreeItem,0,0,0,0,0};
		                TreeView_GetItem(p->hdr.hwndFrom,&i);
                        C_RenderListClass::T_RenderListType *tp=(C_RenderListClass::T_RenderListType *)i.lParam;
                        is_aux_wnd=0;
                        if (tp&&tp->render)
                        {
			                SetDlgItemText(hwndDlg,IDC_EFNAME,tp->render->get_desc());
			                if (cur_hwnd) 
                                DestroyWindow(cur_hwnd);
                            cur_hwnd=tp->render->conf(g_render_library->GetRendererInstance(tp->effect_index,g_hInstance),hwndDlg);
                            if (cur_hwnd) 
                                sniffConfigWindow_oldProc=(WNDPROC)SetWindowLong(cur_hwnd, GWLP_WNDPROC,(LONG)sniffConfigWindow_newProc);
                        }
		            if (cur_hwnd)
                    {
			            RECT r;
			            GetWindowRect(GetDlgItem(hwndDlg,IDC_EFFECTRECT),&r);
			            ScreenToClient(hwndDlg,(LPPOINT)&r);
			            SetWindowPos(cur_hwnd,0,r.left,r.top,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
			            ShowWindow(cur_hwnd,SW_SHOWNA);
		            } else 
                        SetDlgItemText(hwndDlg,IDC_EFNAME,g_noeffectstr);
                    }
                }
                // a tree view item has been clicked

            }
        }
        break;
	    case WM_COMMAND:
		    switch (LOWORD(wParam))
		    {
                case IDC_RRECT:
                  if (HIWORD(wParam)==1) 
                  {
                    toggleWharfAmpDock(g_hwnd);
                  }
                return 0;
                case IDM_HELP_DEBUGWND:
                  if (!g_debugwnd) g_debugwnd=WASABI_API_CREATEDIALOG(IDD_DEBUG,g_hwnd,debugProc);
                  ShowWindow(g_debugwnd,SW_SHOW);
                return 0;
                case IDM_ABOUT:
                  // DialogBox(g_hInstance,MAKEINTRESOURCE(IDD_DIALOG2),hwndDlg,aboutProc);
                  about(hwndDlg);
                  return 0;
                case IDM_DISPLAY:
                case IDM_FULLSCREEN:
                case IDM_PRESETS:
                case IDM_BPM:
                case IDM_TRANSITIONS:
                {
                    int name_ids[5]={IDS_DISPLAY,IDS_FULLSCREEN_SETTINGS,IDS_PRESET_HOTKEYS,IDS_BEAT_DETECTION,IDS_TRANSITIONS};
                    int x=0;
                    if (LOWORD(wParam) == IDM_DISPLAY) x=1;
                    if (LOWORD(wParam) == IDM_FULLSCREEN) x=2;
                    if (LOWORD(wParam) == IDM_PRESETS) x=3;
                    if (LOWORD(wParam) == IDM_BPM) x=4;
                    if (LOWORD(wParam) == IDM_TRANSITIONS) x=5;

                    if (x >= 1 && x <= 5)
                    {
                        SetDlgItemText(hwndDlg,IDC_EFNAME,WASABI_API_LNGSTRING(name_ids[x-1]));
                        TreeView_Select(GetDlgItem(hwndDlg,IDC_TREE1),NULL,TVGN_CARET);
                        if (cur_hwnd) DestroyWindow(cur_hwnd);
			            if (x==1)
				            cur_hwnd=WASABI_API_CREATEDIALOG(IDD_GCFG_DISP,hwndDlg,DlgProc_Disp);
			            if (x==2)
				            cur_hwnd=WASABI_API_CREATEDIALOG(IDD_GCFG_FS,hwndDlg,DlgProc_FS);
			            if (x==3)
				            cur_hwnd=WASABI_API_CREATEDIALOG(IDD_GCFG_PRESET,hwndDlg,DlgProc_Preset);
			            if (x==4)
				            cur_hwnd=WASABI_API_CREATEDIALOG(IDD_GCFG_BPM,hwndDlg,DlgProc_Bpm);
			            if (x==5)
				            cur_hwnd=g_render_transition->conf(g_hInstance,hwndDlg);
			            if (cur_hwnd) 
                        {
					        RECT r;
                            is_aux_wnd=1;
					        GetWindowRect(GetDlgItem(hwndDlg,IDC_EFFECTRECT),&r);
					        ScreenToClient(hwndDlg,(LPPOINT)&r);
					        SetWindowPos(cur_hwnd,0,r.left,r.top,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
					        ShowWindow(cur_hwnd,SW_SHOWNA);
				        } 
                        else 
                            SetDlgItemText(hwndDlg,IDC_EFNAME,g_noeffectstr);
                    }
                }
                return 0;
		        case IDC_ADD:
		        {
                    C_RenderListClass::T_RenderListType ren={0};
                    RECT r;
                    presetTreeMenu=CreatePopupMenu();

                    HMENU hAddMenu=CreatePopupMenu();

        //          int insert_pos=0;
                    int p=64;
			        int x=0;
			        while (1)
			        {
				        char str[1024];
                        if (!g_render_library->GetRendererDesc(x,str))
                            break;

                        if (str[0]) 
                            _insertintomenu(hAddMenu,p++,x,str);

                        x++;
			        }
                    x=DLLRENDERBASE;
			        while (1)
			        {
			            char str[1024];
                        int id=g_render_library->GetRendererDesc(x++, str);
                        if (!id) break;
                        if (str[0]) _insertintomenu(hAddMenu,p++,id,str);
			        }
                    _insertintomenu(hAddMenu,p++,LIST_ID,WASABI_API_LNGSTRING(IDS_EFFECT_LIST));

                    int preset_base=presetTreeCount=p;
                    // add presets
                    {
                        MENUITEMINFO i={sizeof(i),};
                        i.hSubMenu=presetTreeMenu;
                        i.fMask=MIIM_SUBMENU|MIIM_TYPE|MIIM_ID;
                        i.fType=MFT_STRING;
                        i.dwTypeData = WASABI_API_LNGSTRING(IDS_PRESETS);
				        i.cch = strlen((char*)i.dwTypeData);
				        InsertMenuItem(hAddMenu,0,TRUE,&i);
                        i.hSubMenu=0;
                        i.fMask=MIIM_TYPE|MIIM_ID;
                        i.fType=MFT_SEPARATOR;
                        InsertMenuItem(hAddMenu,1,TRUE,&i);

		                HANDLE h;
		                WIN32_FIND_DATA d;
		                char dirmask[1024];
                        wsprintf(dirmask,"%s\\*.*",g_path);

                        int directory_pos=0,insert_pos=0;

		                 h = FindFirstFile(dirmask,&d);
		                if (h != INVALID_HANDLE_VALUE)
		                {
			                do 
                            {
                                if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && d.cFileName[0] != '.')
                                {
                                    MENUITEMINFO mi={sizeof(mi),MIIM_SUBMENU|MIIM_TYPE,MFT_STRING,MFS_DEFAULT};
                                    mi.hSubMenu=CreatePopupMenu();
                                    mi.dwTypeData=d.cFileName;
                                    mi.cch = strlen(d.cFileName);
                                    InsertMenuItem(presetTreeMenu,directory_pos++,TRUE,&mi);
                                    insert_pos++;
                                }
                                else if (!stricmp(extension(d.cFileName),"avs"))
                                {
				                    extension(d.cFileName)[-1]=0;
                                    MENUITEMINFO i={sizeof(i),MIIM_DATA|MIIM_TYPE|MIIM_ID,MFT_STRING,MFS_DEFAULT };
				                    i.dwTypeData = d.cFileName;
				                    i.cch = strlen(d.cFileName);
                                    i.dwItemData=0xffffffff;
				                     i.wID=presetTreeCount++;
				                    InsertMenuItem(presetTreeMenu,insert_pos++,TRUE,&i);
                                }
			                } while (FindNextFile(h,&d));
			                FindClose(h);
		                }
                    }

                    GetWindowRect(GetDlgItem(hwndDlg,IDC_ADD),&r);
                    int t=TrackPopupMenu(hAddMenu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON,r.right,r.top,0,hwndDlg,NULL);
                    if (t)
                    {
                        char buf[2048];
                        buf[0]=0;
                        if (t >= preset_base)                
                        {
                            if (findInMenu(presetTreeMenu,0,t,buf,2048))
                            {
                                //preset
                                C_RenderListClass *r;
                                char temp[4096];
                                ren.effect_index=LIST_ID;
                                wsprintf(temp,"%s%s.avs",g_path,buf);
                                r=new C_RenderListClass();
                                if (!r->__LoadPreset(temp,1))
                                {
                                ren.render=(C_RBASE*)r;
                                }
                                else 
                                {
                                delete r;
                                ren.render=NULL;
                                }
                            }
                        }
                        else 
                        {
			                MENUITEMINFO mi={sizeof(mi),MIIM_DATA,};
                            GetMenuItemInfo(hAddMenu,t,FALSE,&mi);
                            if (mi.dwItemData != 0xffffffff) // effect
                            {
                              ren.effect_index=mi.dwItemData;
                              ren.render=g_render_library->CreateRenderer(&ren.effect_index,&ren.has_rbase2);
                            }
                        }

                        if (ren.render)
                        {
                            int insert_pos=0;
				            HTREEITEM hTreeItem = TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_TREE1));
                            C_RenderListClass *parentrender=g_render_effects;
                            HTREEITEM parenthandle=g_hroot;
                            if (hTreeItem)
                            {
				                TV_ITEM i={TVIF_HANDLE|TVIF_PARAM,hTreeItem,0,0,0,0,0};
				                TreeView_GetItem(GetDlgItem(hwndDlg,IDC_TREE1),&i);
                                C_RenderListClass::T_RenderListType *tp=(C_RenderListClass::T_RenderListType *)i.lParam;
                                if (tp->effect_index == LIST_ID)
                                {
                                    parentrender=(C_RenderListClass *)tp->render;
                                    parenthandle=hTreeItem;
                                }
                                else
                                {
                                    HTREEITEM hParent=TreeView_GetParent(GetDlgItem(hwndDlg,IDC_TREE1),hTreeItem);
                                    if (hParent && hParent != TVI_ROOT)
                                    {
                                        TV_ITEM i2={TVIF_HANDLE|TVIF_PARAM,hParent,0,0,0,0,0};
				                                TreeView_GetItem(GetDlgItem(hwndDlg,IDC_TREE1),&i2);
                                        C_RenderListClass::T_RenderListType *tparent=(C_RenderListClass::T_RenderListType *)i2.lParam;
                                        parentrender=(C_RenderListClass *)tparent->render;
                                        parenthandle=hParent;
                                    }
                                    for (insert_pos=0; insert_pos<parentrender->getNumRenders() 
                                            && parentrender->getRender(insert_pos)->render != tp->render; insert_pos++);
                                }                  
                            }
                
  					        EnterCriticalSection(&g_render_cs);
                            parentrender->insertRender(&ren,insert_pos);
					        LeaveCriticalSection(&g_render_cs);
                            C_RenderListClass::T_RenderListType *newt=(C_RenderListClass::T_RenderListType *)GlobalAlloc(GMEM_FIXED,sizeof(C_RenderListClass::T_RenderListType));
                            *newt=ren;
                            TV_INSERTSTRUCT is={parenthandle,0,{TVIF_PARAM|TVIF_TEXT|TVIF_CHILDREN,0,0,0,ren.render->get_desc(),0,0,0,newt->effect_index==LIST_ID?1:0,(int)newt}};
                            if (!hTreeItem || parenthandle==hTreeItem) 
                            { 
                                is.hInsertAfter=TVI_FIRST;
                            }
                            else 
                            {
                                is.hInsertAfter=TreeView_GetPrevSibling(GetDlgItem(hwndDlg,IDC_TREE1),hTreeItem);
                                if (!is.hInsertAfter) is.hInsertAfter=TVI_FIRST;
                            }
                            HTREEITEM newh=TreeView_InsertItem(GetDlgItem(hwndDlg,IDC_TREE1),&is);
                            TreeView_Select(GetDlgItem(hwndDlg,IDC_TREE1),newh,TVGN_CARET);
                            if (ren.effect_index == LIST_ID) 
                                _do_add(GetDlgItem(hwndDlg,IDC_TREE1),newh,(C_RenderListClass *)ren.render);

                            // Always do undo last.
                            KillTimer(hwndDlg,69);
                            SetTimer(hwndDlg,69,UNDO_TIMER_INTERVAL,NULL);
                            // g_preset_dirty=1;
                        }
                    }
                    DestroyMenu(hAddMenu);
                    presetTreeMenu=0;
		        } // end of IDC_ADD
		        return 0;
                case IDC_CLEAR:
                    if (readyToLoadPreset(hwndDlg,1))
                    {
                    if (g_render_transition->LoadPreset("",0) != 2)
                        last_preset[0]=0;
                    }
                    return 0;
		        case IDC_REMSEL:
                {
			        HTREEITEM hTreeItem = TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_TREE1));
                    if (hTreeItem == g_hroot)
                    {
                        CfgWnd_Unpopulate();
			            EnterCriticalSection(&g_render_cs);
                        g_render_effects->clearRenders();
			            LeaveCriticalSection(&g_render_cs);
                        CfgWnd_Populate();
                    }
                    else if (hTreeItem)
                    {
                        C_RenderListClass *parentrender;
			            TV_ITEM i={TVIF_HANDLE|TVIF_PARAM,hTreeItem,0,0,0,0,0};
			            if (TreeView_GetItem(GetDlgItem(hwndDlg,IDC_TREE1),&i))
                        {
                            C_RenderListClass::T_RenderListType *tp=(C_RenderListClass::T_RenderListType *)i.lParam;
                            HTREEITEM hParent=TreeView_GetParent(GetDlgItem(hwndDlg,IDC_TREE1),hTreeItem);
                            if (hParent != NULL)
                            {
                                TV_ITEM i2={TVIF_HANDLE|TVIF_PARAM,hParent,0,0,0,0,0};
				                TreeView_GetItem(GetDlgItem(hwndDlg,IDC_TREE1),&i2);
                                C_RenderListClass::T_RenderListType *tparent=(C_RenderListClass::T_RenderListType *)i2.lParam;
                                parentrender=(C_RenderListClass*)tparent->render;
					            EnterCriticalSection(&g_render_cs);
                                if (!parentrender->removeRenderFrom(tp,1))
                                {
                                    TreeView_DeleteItem(GetDlgItem(hwndDlg,IDC_TREE1),hTreeItem);
	                                if (tp) 
                                        GlobalFree((HGLOBAL)tp);
                                }
  				                LeaveCriticalSection(&g_render_cs);
                            }
                        }
                    }
                    // Always save undo last.
                    KillTimer(hwndDlg,69);
                    SetTimer(hwndDlg,69,UNDO_TIMER_INTERVAL,NULL);
                    // g_preset_dirty=1;
                }
		        return 0;
		        case IDC_CLONESEL:
		        {
                    C_RenderListClass::T_RenderListType ren={0,};
                    int insert_pos=-1;

	                HTREEITEM hTreeItem = TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_TREE1));
                    if (hTreeItem && hTreeItem != g_hroot)
                    {
			            TV_ITEM i={TVIF_HANDLE|TVIF_PARAM,hTreeItem,0,0,0,0,0};
			            TreeView_GetItem(GetDlgItem(hwndDlg,IDC_TREE1),&i);
                        C_RenderListClass::T_RenderListType *tp=(C_RenderListClass::T_RenderListType *)i.lParam;
                        ren.effect_index=tp->effect_index;
                        ren.render=g_render_library->CreateRenderer(&ren.effect_index,&ren.has_rbase2);
                        if (ren.render)
                        {
                            HTREEITEM hParent=TreeView_GetParent(GetDlgItem(hwndDlg,IDC_TREE1),hTreeItem);
                            if (hParent && hParent != TVI_ROOT)
                            {
                                TV_ITEM i2={TVIF_HANDLE|TVIF_PARAM,hParent,0,0,0,0,0};
				                        TreeView_GetItem(GetDlgItem(hwndDlg,IDC_TREE1),&i2);
                                C_RenderListClass::T_RenderListType *tparent=(C_RenderListClass::T_RenderListType *)i2.lParam;
                                C_RenderListClass *parentrender=(C_RenderListClass *)tparent->render;
                                for (insert_pos=0; insert_pos<parentrender->getNumRenders() 
                                    && parentrender->getRender(insert_pos)->render != tp->render; insert_pos++);
                                insert_pos++;                
                                unsigned char *buf = (unsigned char *) GlobalAlloc(GPTR,1024*1024);
                                if (buf)
                                {
                                int len=tp->render->save_config(buf);
                                ren.render->load_config(buf,len);
                                GlobalFree((HGLOBAL)buf);
                                }
						                    EnterCriticalSection(&g_render_cs);
                                parentrender->insertRender(&ren,insert_pos);
						                    LeaveCriticalSection(&g_render_cs);
                  
                                C_RenderListClass::T_RenderListType *newt=(C_RenderListClass::T_RenderListType *)GlobalAlloc(GMEM_FIXED,sizeof(C_RenderListClass::T_RenderListType));
                                *newt=ren;
                                TV_INSERTSTRUCT is={hParent,0,{TVIF_PARAM|TVIF_TEXT|TVIF_CHILDREN,0,0,0,ren.render->get_desc(),0,0,0,newt->effect_index==LIST_ID?1:0,(int)newt}};
                                is.hInsertAfter=hTreeItem;
                                HTREEITEM newh=TreeView_InsertItem(GetDlgItem(hwndDlg,IDC_TREE1),&is);
                                TreeView_Select(GetDlgItem(hwndDlg,IDC_TREE1),newh,TVGN_CARET);
                                if (ren.effect_index == LIST_ID) 
                                _do_add(GetDlgItem(hwndDlg,IDC_TREE1),newh,(C_RenderListClass *)ren.render);
                            }
                        }
                        // Always save undo last.
                        KillTimer(hwndDlg,69);
                        SetTimer(hwndDlg,69,UNDO_TIMER_INTERVAL,NULL);
                        // g_preset_dirty=1;
                    }
		        }
		        return 0;
		        case IDC_LOAD:
                {
		            char temp[2048];
		            OPENFILENAME l={sizeof(l),0};
		            char buf1[2048],buf2[2048];
		            char filter[128], *ptr = filter;
		            char loadPreset[64];

		            WASABI_API_LNGSTRING_BUF(IDS_AVS_FILTER_STRING,filter,128);
		            while(*ptr)
		            {
			            if (*ptr=='|') *ptr=0;
			            ptr++;
		            }

		            GetCurrentDirectory(sizeof(buf2),buf2);
		            strcpy(buf1,g_path); // this was changed from: lstrcpyn(buf1,g_path,sizeof(buf1));
                    temp[0]=0;
			        l.lpstrInitialDir = buf1;
			        l.hwndOwner = hwndDlg;
			        l.lpstrFilter = filter;
			        l.lpstrFile = temp;
			        l.nMaxFile = 2048-1;
			        l.lpstrTitle = WASABI_API_LNGSTRING_BUF(IDS_LOAD_PRESET,loadPreset,48);
			        l.lpstrDefExt = "AVS";
			        l.Flags = OFN_HIDEREADONLY|OFN_EXPLORER; 	        
			        if (readyToLoadPreset(hwndDlg,0) && GetOpenFileName(&l)) 
			        {
                        int x=g_render_transition->LoadPreset(temp,0);
                        if (x == 2) 
                            MessageBox(hwndDlg,WASABI_API_LNGSTRING(IDS_STILL_INITIALIZING_PREVIOUS_PRESET),loadPreset,MB_OK);
                        else 
                            lstrcpyn(last_preset,temp,sizeof(last_preset)); 
			        } 
			        SetCurrentDirectory(buf2);
		        }
		        return 0;
		        case IDC_SAVE:
                    dosavePreset(hwndDlg);
		            return 0;
		        case IDM_UNDO:
                    C_UndoStack::undo();
			        return 0;
		        case IDM_REDO:
                    C_UndoStack::redo();
			        return 0;
	        } // end of  switch(LOWORD(Param))
		    return 0;
	    }// end of switch(uMsg)
	return 0;
}

static void _do_add(HWND hwnd, HTREEITEM h, C_RenderListClass *list)
{
  int x,l;
  l=list->getNumRenders();
	for (x = 0; x < l; x ++)
	{
    C_RenderListClass::T_RenderListType *t=list->getRender(x);
    if (t)
    {
      C_RenderListClass::T_RenderListType *newt=(C_RenderListClass::T_RenderListType *)GlobalAlloc(GMEM_FIXED,sizeof(C_RenderListClass::T_RenderListType));
      memcpy(newt,t,sizeof(C_RenderListClass::T_RenderListType));
      TV_INSERTSTRUCT is;
      memset(&is,0,sizeof(is));

      is.hParent=h;
      is.hInsertAfter=TVI_LAST;
      is.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_CHILDREN;
      is.item.pszText=t->render->get_desc();
      is.item.cChildren=t->effect_index==LIST_ID?1:0;
      is.item.lParam=(int)newt;

	    HTREEITEM h2=TreeView_InsertItem(hwnd,&is);
      if (t->effect_index==LIST_ID)
      {
        _do_add(hwnd,h2,(C_RenderListClass *)t->render);
      }
    }
	}
  SendMessage(hwnd,TVM_EXPAND,TVE_EXPAND,(long)h);
}

static void _do_free(HWND hwnd, HTREEITEM h)
{
  while (h)
  {
	  TV_ITEM i={TVIF_HANDLE|TVIF_PARAM,h,0,0,0,0,0};
  	TreeView_GetItem(hwnd,&i);
    if (i.lParam) GlobalFree((HGLOBAL)(void*)i.lParam);
    HTREEITEM h2=TreeView_GetChild(hwnd,h);
    if (h2) _do_free(hwnd,h2);
    h=TreeView_GetNextSibling(hwnd,h);
  }
}

int need_repop;

void CfgWnd_RePopIfNeeded(void)
{
  if (need_repop)
  {
    CfgWnd_Unpopulate(1);
    CfgWnd_Populate(1);
    need_repop=0;
  }
}

void CfgWnd_Unpopulate(int force)
{
  if (force || (IsWindowVisible(g_hwndDlg)&&!DDraw_IsFullScreen()))
  {
    HWND hwnd=GetDlgItem(g_hwndDlg,IDC_TREE1);
	  if (!is_aux_wnd) 
    {
      if (cur_hwnd) DestroyWindow(cur_hwnd); 
      cur_hwnd=0;
      SetDlgItemText(g_hwndDlg,IDC_EFNAME,g_noeffectstr);
    }
    treeview_hack=1;
    _do_free(hwnd,TreeView_GetChild(hwnd,TVI_ROOT));
    TreeView_DeleteAllItems(hwnd);
    treeview_hack=0;
  }
  else need_repop=1;
}

void CfgWnd_Populate(int force)
{
  if (force || (IsWindowVisible(g_hwndDlg)&&!DDraw_IsFullScreen()))
  {
    treeview_hack=1;
    HWND hwnd=GetDlgItem(g_hwndDlg,IDC_TREE1);
    C_RenderListClass::T_RenderListType *newt=(C_RenderListClass::T_RenderListType *)GlobalAlloc(GMEM_FIXED,sizeof(C_RenderListClass::T_RenderListType));
    newt->render=g_render_effects;
    newt->effect_index=LIST_ID;
    TV_INSERTSTRUCT is;
    memset(&is,0,sizeof(is));
    is.hParent=TVI_ROOT;
    is.hInsertAfter=TVI_LAST;
    is.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_CHILDREN;
    is.item.pszText=WASABI_API_LNGSTRING(IDS_MAIN);
    is.item.cChildren=1;
    is.item.lParam=(int)newt;
    g_hroot=TreeView_InsertItem(hwnd,&is);
    if (g_hroot)
    {
      _do_add(hwnd,g_hroot,g_render_effects);
      SendMessage(hwnd,TVM_EXPAND,TVE_EXPAND,(long)g_hroot);
    }
    treeview_hack=0;
  }
  else need_repop=1;
}
