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
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include "vis.h"
#include "draw.h"
#include "wnd.h"
#include "cfgwnd.h"
#include "r_defs.h"
#include "resource.h"
#include "render.h"
#include "undo.h"
#include <multimon.h>

#include "avs_eelif.h"

#define WINAMP_NEXT_WINDOW 40063
#include "wa_ipc.h"
#include "ff_ipc.h"
#include "../Agave/Language/api_language.h"

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED	0x80000
#endif

#ifndef LWA_ALPHA
#define LWA_ALPHA		2
#endif

#define ID_VIS_NEXT                     40382
#define ID_VIS_PREV                     40383
#define ID_VIS_RANDOM                   40384

struct winampVisModule *g_mod;
extern volatile int g_ThreadQuit;
extern int /*g_preset_dirty,*/ config_prompt_save_preset, config_reuseonresize;
int g_saved_preset_dirty;
extern int cfg_cancelfs_on_deactivate;

extern char g_noeffectstr[];
#ifndef WA2_EMBED
static int cfg_x=100, cfg_y=100, cfg_w=400, cfg_h=300;
#endif

#ifndef WA2_EMBED
static HDC hFrameDC;
static HBITMAP hFrameBitmap, hFrameBitmap_old;
#else
embedWindowState myWindowState;
HWND g_hWA2ParentWindow;
#endif

int g_reset_vars_on_recompile=1; // fucko: add config for this


// Wharf integration
int inWharf=0;
int need_redock=0;

extern int cfg_fs_use_overlay;
extern int g_config_seh;


void toggleWharfAmpDock(HWND hwnd);

//-

int g_in_destroy=0,g_minimized=0,g_fakeinit=0;

int g_rnd_cnt;
char g_skin_name[MAX_PATH];

int debug_reg[8];

void GetClientRect_adj(HWND hwnd, RECT *r)
{
  GetClientRect(hwnd,r);
#ifndef WA2_EMBED
  if (!inWharf)
  {
    r->right-=7+6;
    r->bottom-=15+5;
  }
#endif
}

HWND g_hwnd;

HWND hwnd_WinampParent;
extern HWND g_hwndDlg;
extern char last_preset[2048];
char *scanstr_back(char *str, char *toscan, char *defval)
{
	char *s=str+strlen(str)-1;
	if (strlen(str) < 1) return defval;
	if (strlen(toscan) < 1) return defval;
	while (1)
	{
		char *t=toscan;
		while (*t)
			if (*t++ == *s) return s;
		t=CharPrev(str,s);
		if (t==s) return defval;
		s=t;
	}
}	

int LoadPreset(int preset)
{
  char temp[MAX_PATH];
  wsprintf(temp,"%s\\PRESET%02d.APH",g_path,preset);
  if (g_render_transition->LoadPreset(temp,1))
    return 0;
  if (preset < 12) wsprintf(last_preset,"\\F%d.aph",preset+1);
  else if (preset < 22) wsprintf(last_preset,"\\%d.aph",preset-12);
  else if (preset < 32) wsprintf(last_preset,"\\Shift-%d.aph",preset-22);
  return 1;
}

void WritePreset(int preset)
{
  char temp[MAX_PATH];
  wsprintf(temp,"%s\\PRESET%02d.APH",g_path,preset);
  g_render_effects->__SavePreset(temp);
}

void my_getViewport( RECT *r, RECT *sr )
{
    if ( sr )
    {
        HINSTANCE h = LoadLibraryW( L"user32.dll" );
        if ( h )
        {
            HMONITOR( WINAPI * Mfr )( LPCRECT lpcr, DWORD dwFlags ) = ( HMONITOR( WINAPI * )( LPCRECT, DWORD ) ) GetProcAddress( h, "MonitorFromRect" );
            BOOL( WINAPI * Gmi )( HMONITOR mon, LPMONITORINFO lpmi ) = ( BOOL( WINAPI * )( HMONITOR, LPMONITORINFO ) ) GetProcAddress( h, "GetMonitorInfoW" );
            if ( Mfr && Gmi )
            {
                HMONITOR hm;
                hm = Mfr( sr, MONITOR_DEFAULTTONULL );
                if ( hm )
                {
                    MONITORINFOEXW mi;
                    memset( &mi, 0, sizeof( mi ) );
                    mi.cbSize = sizeof( mi );

                    if ( Gmi( hm, &mi ) )
                    {
                        *r = mi.rcWork;
                        return;
                    }
                }
            }
            FreeLibrary( h );
        }
    }
    SystemParametersInfoW( SPI_GETWORKAREA, 0, r, 0 );
}


void SetTransparency(HWND hWnd, int enable, int amount)
{
#ifdef WA2_EMBED
  // disable transparency if hosted in gen_ff
  HWND w = myWindowState.me;
  while (GetWindowLong(w, GWL_STYLE) & WS_CHILD) w = GetParent(w);
  char classname[256];
  GetClassName(w, classname, 255); classname[255] = 0;
  if (!stricmp(classname, "BaseWindow_RootWnd")) return;
  // --
#endif
  
	DWORD dwLong;
	HINSTANCE h;
	void (__stdcall *a)(HWND h, int a, int b, int c);

  hWnd=GetParent(hWnd);

	dwLong = GetWindowLong(hWnd, GWL_EXSTYLE);
	if(amount==255||!enable) {
		if(dwLong&WS_EX_LAYERED)
			SetWindowLong(hWnd, GWL_EXSTYLE, dwLong & ~WS_EX_LAYERED);
	} else {
		if(!(dwLong&WS_EX_LAYERED))
			SetWindowLong(hWnd, GWL_EXSTYLE, dwLong | WS_EX_LAYERED);
		h=LoadLibrary("USER32.DLL");
		if(h!=NULL) {
			a=(void (__stdcall *)(HWND,int,int,int))GetProcAddress(h,"SetLayeredWindowAttributes");
			if(a!=NULL) 
				a(hWnd, RGB(0,0,0), amount, LWA_ALPHA);
			FreeLibrary(h);
		}
	}
}

int readyToLoadPreset(HWND parent, int isnew)
{
  if (config_prompt_save_preset && C_UndoStack::isdirty())
  {
    static int here;
    if (here) return 0;
    here=1;
    
    // strange bugfix, ick
    void Wnd_GoWindowed(HWND hwnd);
    if (DDraw_IsFullScreen()) Wnd_GoWindowed(g_hwnd);

    char title[48];
    int ret=MessageBox(parent,
		       WASABI_API_LNGSTRING(!isnew?IDS_CURRENT_PRESET_EDITED_SAVE_BEFORE_LOAD:
						   IDS_CURRENT_PRESET_EDITED_SAVE_BEFORE_NEW),
		       WASABI_API_LNGSTRING_BUF(IDS_AVS_PRESET_MODIFIED,title,48),MB_YESNOCANCEL);
    here=0;

    if (ret == IDCANCEL)
    {
      return 0;
    }
    if (ret == IDYES)
    {
      int dosavePreset(HWND hwndDlg);
      int r=1;
//      if (last_preset[0])
  //      r=g_render_effects->SavePreset(last_preset);

      if (r) 
      {
        if (dosavePreset(parent)) return 0;
      }
    }
  }
  //C_UndoStack::clear();
  // g_preset_dirty=0;
  return 1;
}

char *extension(char *fn) 
{
  char *s = fn + strlen(fn);
  while (s >= fn && *s != '.' && *s != '\\') s--;
  if (s < fn) return fn;
  if (*s == '\\') return fn;
  return (s+1);
}

static int last_windowed_w, last_windowed_h;
void Wnd_GoWindowed(HWND hwnd)
{
  if (DDraw_IsFullScreen())
  {
#ifdef WA2_EMBED
  SendMessage(g_mod->hwndParent,WM_WA_IPC,0,IPC_SET_VIS_FS_FLAG);
#endif
#if !defined(WA2_EMBED)
		DDraw_SetFullScreen(0,cfg_w-7-6,cfg_h-15-5,cfg_fs_d&2,0);
#else
    SetParent(hwnd,myWindowState.me);
    SetWindowLong(hwnd,GWL_STYLE,WS_VISIBLE|WS_CHILDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_OVERLAPPED);
		DDraw_SetFullScreen(0,last_windowed_w,last_windowed_h,cfg_fs_d&2,0);
    HWND w = myWindowState.me;
    while (GetWindowLong(w, GWL_STYLE) & WS_CHILD) w = GetParent(w);
    ShowWindow(w,SW_SHOWNA);
#endif

		if (cfg_cancelfs_on_deactivate) ShowCursor(TRUE);
    int tm=(GetWindowLong(g_mod->hwndParent,GWL_EXSTYLE)&WS_EX_TOPMOST)==WS_EX_TOPMOST;

#if !defined(WA2_EMBED)
    SetWindowPos(hwnd,tm?HWND_TOPMOST:HWND_NOTOPMOST,cfg_x,cfg_y,cfg_w,cfg_h,SWP_NOACTIVATE);
#else
    PostMessage(GetParent(hwnd),WM_SIZE,0,0);
#endif

		SetTransparency(hwnd,cfg_trans,cfg_trans_amount);

    SetTimer(hwnd,88,100,NULL);
  }
}

void Wnd_GoFullScreen(HWND hwnd)
{
  if (!DDraw_IsFullScreen())
  {

#if 1
#ifdef WA2_EMBED
    if (SendMessage(g_mod->hwndParent,WM_WA_IPC,0,IPC_IS_PLAYING_VIDEO)>1)
    {
      PostMessage(hwnd,WM_USER+1667,1,2);
      return;
    }
#endif
#endif

    extern int cfg_fs_use_overlay;
#ifdef WA2_EMBED
    RECT r;
    GetClientRect(hwnd,&r);
    last_windowed_w=r.right;
    last_windowed_h=r.bottom;
#endif

    if (!DDraw_IsMode(cfg_fs_w,cfg_fs_h,cfg_fs_bpp))
    {
      int DDraw_PickMode(int *w, int *h, int *bpp);
      if (!DDraw_PickMode(&cfg_fs_w,&cfg_fs_h,&cfg_fs_bpp))
        return;
    }

    {
#ifdef WA2_EMBED
      SendMessage(g_mod->hwndParent,WM_WA_IPC,1,IPC_SET_VIS_FS_FLAG);
#endif
      RECT tr;
      if (inWharf) 
      {
        need_redock=1;
        toggleWharfAmpDock(hwnd);
      }
	    SetTransparency(hwnd,0,0);
			if (cfg_cancelfs_on_deactivate) ShowCursor(FALSE);
      GetWindowRect(hwnd,&tr);
      if (cfg_cfgwnd_open) ShowWindow(g_hwndDlg,SW_HIDE);
			if (!cfg_fs_use_overlay) 
      {
#if defined(WA2_EMBED)
        SetWindowLong(hwnd,GWL_STYLE,WS_VISIBLE);
        SetParent(hwnd,NULL);
        HWND w = myWindowState.me;
        while (GetWindowLong(w, GWL_STYLE) & WS_CHILD) w = GetParent(w);
        ShowWindow(w,SW_HIDE);
#endif
        DDraw_SetFullScreen(1,cfg_fs_w,cfg_fs_h,cfg_fs_d&1,cfg_fs_bpp);
        RECT r;
        my_getViewport(&r,&tr);
        SetWindowPos(hwnd,HWND_TOPMOST,r.left,r.top,cfg_fs_w,cfg_fs_h,0);
        SetForegroundWindow(hwnd);
      }
			else 
      {
#if defined(WA2_EMBED)
        SetWindowLong(hwnd,GWL_STYLE,WS_VISIBLE);
        SetParent(hwnd,NULL);
        HWND w = myWindowState.me;
        while (GetWindowLong(w, GWL_STYLE) & WS_CHILD) w = GetParent(w);
        ShowWindow(w,SW_HIDE);
#endif
        DDraw_SetFullScreen(1,cfg_fs_w,cfg_fs_h,cfg_fs_d&1,0);
      }
		}
#if 0
    else 
    {
      if (!cfg_cfgwnd_open) 
      {
			  ShowWindow(g_hwndDlg,SW_SHOWNA);
        CfgWnd_RePopIfNeeded();
        cfg_cfgwnd_open=1;
			}
      SendMessage(g_hwndDlg,WM_COMMAND,IDM_FULLSCREEN,0);
    }
#endif
  }
}

int g_config_smp_mt=2,g_config_smp=0;
static char *INI_FILE;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int cfg_fs_dblclk=1;

int Wnd_Init(struct winampVisModule *this_mod)
{
	WNDCLASS wc={0,};
	g_mod=this_mod;
	wc.style = CS_DBLCLKS|CS_VREDRAW|CS_HREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = this_mod->hDllInstance;
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = "avswnd";
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);

  hwnd_WinampParent=this_mod->hwndParent;


	if (!RegisterClass(&wc)) 
	{
//		MessageBox(this_mod->hwndParent,"Error registering window class","Error",MB_OK);
	//	return 1;
	}
	{
#ifndef REAPLAY_PLUGIN
    INI_FILE = (char*)SendMessage(this_mod->hwndParent,WM_WA_IPC,0,IPC_GETINIFILE);
#else
    extern const char *(*get_ini_file)();
    INI_FILE = (char *)get_ini_file();
#endif
#define AVS_SECTION "AVS"
#ifdef LASER
#undef AVS_SECTION
#define AVS_SECTION "AVS_L"
    extern int g_laser_nomessage,g_laser_zones;
    g_laser_nomessage=GetPrivateProfileInt(AVS_SECTION,"laser_nomessage",0,INI_FILE);
    g_laser_zones=GetPrivateProfileInt(AVS_SECTION,"laser_zones",1,INI_FILE);
#else
    g_config_smp=GetPrivateProfileInt(AVS_SECTION,"smp",0,INI_FILE);
    g_config_smp_mt=GetPrivateProfileInt(AVS_SECTION,"smp_mt",2,INI_FILE);
#endif
    need_redock=GetPrivateProfileInt(AVS_SECTION,"cfg_docked",0,INI_FILE);
		cfg_cfgwnd_x=GetPrivateProfileInt(AVS_SECTION,"cfg_cfgwnd_x",cfg_cfgwnd_x,INI_FILE);
		cfg_cfgwnd_y=GetPrivateProfileInt(AVS_SECTION,"cfg_cfgwnd_y",cfg_cfgwnd_y,INI_FILE);
		cfg_cfgwnd_open=GetPrivateProfileInt(AVS_SECTION,"cfg_cfgwnd_open",cfg_cfgwnd_open,INI_FILE);
		cfg_fs_w=GetPrivateProfileInt(AVS_SECTION,"cfg_fs_w",cfg_fs_w,INI_FILE);
		cfg_fs_h=GetPrivateProfileInt(AVS_SECTION,"cfg_fs_h",cfg_fs_h,INI_FILE);
		cfg_fs_bpp=GetPrivateProfileInt(AVS_SECTION,"cfg_fs_bpp",cfg_fs_bpp,INI_FILE);
		cfg_fs_d=GetPrivateProfileInt(AVS_SECTION,"cfg_fs_d",cfg_fs_d,INI_FILE);
    cfg_fs_fps=GetPrivateProfileInt(AVS_SECTION,"cfg_fs_fps",6,INI_FILE);
    cfg_fs_rnd=GetPrivateProfileInt(AVS_SECTION,"cfg_fs_rnd",cfg_fs_rnd,INI_FILE);
    cfg_fs_rnd_time=GetPrivateProfileInt(AVS_SECTION,"cfg_fs_rnd_time",cfg_fs_rnd_time,INI_FILE);
    cfg_fs_dblclk=GetPrivateProfileInt(AVS_SECTION,"cfg_fs_dblclk",cfg_fs_dblclk,INI_FILE);
    cfg_fs_flip=GetPrivateProfileInt(AVS_SECTION,"cfg_fs_flip",cfg_fs_flip,INI_FILE);
    cfg_fs_height=GetPrivateProfileInt(AVS_SECTION,"cfg_fs_height",cfg_fs_height,INI_FILE);
    cfg_fs_use_overlay=GetPrivateProfileInt(AVS_SECTION,"cfg_fs_use_overlay",cfg_fs_use_overlay,INI_FILE);
    cfg_cancelfs_on_deactivate=GetPrivateProfileInt(AVS_SECTION,"cfg_fs_cancelondeactivate",cfg_cancelfs_on_deactivate,INI_FILE);
    cfg_speed=GetPrivateProfileInt(AVS_SECTION,"cfg_speed",cfg_speed,INI_FILE);
    cfg_trans=GetPrivateProfileInt(AVS_SECTION,"cfg_trans",cfg_trans,INI_FILE);
    cfg_dont_min_avs=GetPrivateProfileInt(AVS_SECTION,"cfg_dont_min_avs",cfg_dont_min_avs,INI_FILE);
    cfg_smartbeat=GetPrivateProfileInt(AVS_SECTION,"cfg_smartbeat",cfg_smartbeat,INI_FILE);
    cfg_smartbeatsticky=GetPrivateProfileInt(AVS_SECTION,"cfg_smartbeatsticky",cfg_smartbeatsticky,INI_FILE);
    cfg_smartbeatresetnewsong=GetPrivateProfileInt(AVS_SECTION,"cfg_smartbeatresetnewsong",cfg_smartbeatresetnewsong,INI_FILE);
    cfg_smartbeatonlysticky=GetPrivateProfileInt(AVS_SECTION,"cfg_smartbeatonlysticky",cfg_smartbeatonlysticky,INI_FILE);
    GetPrivateProfileString( AVS_SECTION,"config_pres_subdir","",config_pres_subdir,sizeof(config_pres_subdir),INI_FILE);
    GetPrivateProfileString( AVS_SECTION,"last_preset_name","",last_preset,sizeof(last_preset),INI_FILE);
    cfg_transitions=GetPrivateProfileInt(AVS_SECTION,"cfg_transitions_en",cfg_transitions,INI_FILE);
    cfg_transitions2=GetPrivateProfileInt(AVS_SECTION,"cfg_transitions_preinit",cfg_transitions2,INI_FILE);
    cfg_transitions_speed=GetPrivateProfileInt(AVS_SECTION,"cfg_transitions_speed",cfg_transitions_speed,INI_FILE);
    cfg_transition_mode=GetPrivateProfileInt(AVS_SECTION,"cfg_transitions_mode",cfg_transition_mode,INI_FILE);
    cfg_bkgnd_render=GetPrivateProfileInt(AVS_SECTION,"cfg_bkgnd_render",cfg_bkgnd_render,INI_FILE);
    cfg_bkgnd_render_color=GetPrivateProfileInt(AVS_SECTION,"cfg_bkgnd_render_color",cfg_bkgnd_render_color,INI_FILE);
    cfg_render_prio=GetPrivateProfileInt(AVS_SECTION,"cfg_render_prio",cfg_render_prio,INI_FILE);
    g_saved_preset_dirty=GetPrivateProfileInt(AVS_SECTION,"g_preset_dirty",C_UndoStack::isdirty(),INI_FILE);
    config_prompt_save_preset=GetPrivateProfileInt(AVS_SECTION,"cfg_prompt_save_preset",config_prompt_save_preset,INI_FILE);
    config_reuseonresize=GetPrivateProfileInt(AVS_SECTION,"cfg_reuseonresize",config_reuseonresize,INI_FILE);
    g_log_errors=GetPrivateProfileInt(AVS_SECTION,"cfg_log_errors",g_log_errors,INI_FILE);
    g_reset_vars_on_recompile=GetPrivateProfileInt(AVS_SECTION,"cfg_reset_vars",g_reset_vars_on_recompile,INI_FILE);
    g_config_seh=GetPrivateProfileInt(AVS_SECTION,"cfg_seh",g_config_seh,INI_FILE);

    
#ifdef WA2_EMBED
    memset(&myWindowState,0,sizeof(myWindowState));
    myWindowState.r.left=GetPrivateProfileInt(AVS_SECTION,"wx",32,INI_FILE);
    myWindowState.r.top=GetPrivateProfileInt(AVS_SECTION,"wy",32,INI_FILE);
    myWindowState.r.right = GetPrivateProfileInt(AVS_SECTION,"ww",320,INI_FILE)+myWindowState.r.left;
    myWindowState.r.bottom = GetPrivateProfileInt(AVS_SECTION,"wh",240,INI_FILE)+myWindowState.r.top;
#else
		cfg_x=GetPrivateProfileInt(AVS_SECTION,"cfg_x",cfg_x,INI_FILE);
		cfg_y=GetPrivateProfileInt(AVS_SECTION,"cfg_y",cfg_y,INI_FILE);
		cfg_w=GetPrivateProfileInt(AVS_SECTION,"cfg_w",cfg_w,INI_FILE);
		cfg_h=GetPrivateProfileInt(AVS_SECTION,"cfg_h",cfg_h,INI_FILE);
#endif

    int x;
    for (x = 0; x < 8; x ++)
    {
      char debugreg[32];
      wsprintf(debugreg,"debugreg_%d",x);
		  debug_reg[x]=GetPrivateProfileInt(AVS_SECTION,debugreg,x,INI_FILE);
    }

	}
#ifdef LASER
  cfg_transitions=0;
  cfg_transition_mode=0;
  cfg_transitions2=0;
#endif

	g_in_destroy=0;
#ifndef WA2_EMBED
  {
    RECT ir={cfg_x,cfg_y,cfg_w+cfg_x,cfg_y+cfg_h};
    RECT or;
    my_getViewport(&or,&ir);
    if (cfg_x < or.left) cfg_x=or.left;
    if (cfg_y < or.top) cfg_y=or.top;
    if (cfg_x > or.right-16) cfg_x=or.right-16;
    if (cfg_y > or.bottom-16) cfg_y=or.bottom-16;
    // determine bounding rectangle for window
  }
#endif
#ifndef WA2_EMBED
  int styles=WS_VISIBLE;
  HWND par = g_minimized?NULL:this_mod->hwndParent;
#else
  int styles=WS_CHILDWINDOW|WS_OVERLAPPED|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
  HWND (*e)(embedWindowState *v) = NULL;
#ifndef REAPLAY_PLUGIN
  *(void**)&e = (void *)SendMessage(this_mod->hwndParent,WM_WA_IPC,(LPARAM)0,IPC_GET_EMBEDIF);
  HWND par=0;
  if (e) par=e(&myWindowState);
  if (par) SetWindowText(par,"AVS");
#else
  HWND par = this_mod->hwndParent;
#endif
  g_hWA2ParentWindow=par;
#endif

#ifndef WA2_EMBED

  CreateWindowEx(WS_EX_ACCEPTFILES,"avswnd","Winamp AVS Display",
		styles,cfg_x,cfg_y,cfg_w,cfg_h,par,NULL,
				this_mod->hDllInstance,0);
#else
	CreateWindowEx(WS_EX_ACCEPTFILES,"avswnd","avs",
	          	styles,0,0,100,100,par,NULL, this_mod->hDllInstance,0);
  SendMessage(this_mod->hwndParent, WM_WA_IPC, (int)g_hwnd, IPC_SETVISWND);
#endif
  if (!g_hwnd)
  {
		char title[32];
		MessageBox(this_mod->hwndParent,WASABI_API_LNGSTRING(IDS_ERROR_CREATING_WINDOW),
			   WASABI_API_LNGSTRING_BUF(IDS_AVS_ERROR,title,32),MB_OK);
		return 1;
  }
#ifndef REAPLAY_PLUGIN
#ifdef WA2_EMBED
  ShowWindow(g_hwnd,SW_SHOWNA);
  ShowWindow(par,SW_SHOWNA);
#endif
#endif
  SetTransparency(g_hwnd,cfg_trans,cfg_trans_amount);
  return 0;
}

static void WriteInt(char *name, int value)
{
  char str[128];
  wsprintf(str,"%d",value);
	WritePrivateProfileString(AVS_SECTION,name,str,INI_FILE);
}

void Wnd_Quit(void)	
{
  extern HWND g_hwndDlg;
  g_in_destroy=1;
#ifdef WA2_EMBED
  SendMessage(g_mod->hwndParent, WM_WA_IPC, 0, IPC_SETVISWND);
  if (myWindowState.me) 
  {
    SetForegroundWindow(g_mod->hwndParent);
    DestroyWindow(myWindowState.me);
  }
	else 
#endif
  if (g_hwnd && IsWindow(g_hwnd)) DestroyWindow(g_hwnd);
	g_hwnd=NULL;
	UnregisterClass("avswnd",g_mod->hDllInstance);
	{
#ifdef LASER
    extern int g_laser_zones,g_laser_nomessage;
    wsprintf(str,"%d",g_laser_zones);
		WriteInt("laser_zones",g_laser_zones);
		WriteInt("laser_nomessage",g_laser_nomessage);
#else
    WriteInt("smp",g_config_smp);
    WriteInt("smp_mt",g_config_smp_mt);
#endif
#ifdef WA2_EMBED
		WriteInt("wx",myWindowState.r.left);
    WriteInt("wy",myWindowState.r.top);
		WriteInt("ww",myWindowState.r.right-myWindowState.r.left);
		WriteInt("wh",myWindowState.r.bottom-myWindowState.r.top);
#else
		WriteInt("cfg_x",cfg_x);
		WriteInt("cfg_y",cfg_y);
		WriteInt("cfg_w",cfg_w);
		WriteInt("cfg_h",cfg_h);
#endif
		WritePrivateProfileString(AVS_SECTION,"config_pres_subdir",config_pres_subdir,INI_FILE);

		WriteInt("cfg_docked",inWharf?1:0);
		WriteInt("cfg_cfgwnd_open",cfg_cfgwnd_open);
		WriteInt("cfg_cfgwnd_x",cfg_cfgwnd_x);
		WriteInt("cfg_cfgwnd_y",cfg_cfgwnd_y);
		WriteInt("cfg_fs_w",cfg_fs_w);
		WriteInt("cfg_fs_h",cfg_fs_h);
		WriteInt("cfg_fs_d",cfg_fs_d);
		WriteInt("cfg_fs_bpp",cfg_fs_bpp);
		WriteInt("cfg_fs_fps",cfg_fs_fps);
		WriteInt("cfg_fs_rnd",cfg_fs_rnd);
		WriteInt("cfg_fs_rnd_time",cfg_fs_rnd_time);    
    WriteInt("cfg_fs_dblclk",cfg_fs_dblclk);
		WriteInt("cfg_fs_flip",cfg_fs_flip);
		WriteInt("cfg_fs_height",cfg_fs_height);
		WriteInt("cfg_fs_use_overlay",cfg_fs_use_overlay);
		WriteInt("cfg_fs_cancelondeactivate",cfg_cancelfs_on_deactivate);
		WriteInt("cfg_speed",cfg_speed);
		WriteInt("cfg_trans",cfg_trans);
		WriteInt("cfg_dont_min_avs",cfg_dont_min_avs);
		WriteInt("cfg_smartbeat",cfg_smartbeat);
		WriteInt("cfg_smartbeatsticky",cfg_smartbeatsticky);
		WriteInt("cfg_smartbeatresetnewsong",cfg_smartbeatresetnewsong);
		WriteInt("cfg_smartbeatonlysticky",cfg_smartbeatonlysticky);
		WriteInt("cfg_transitions_en",cfg_transitions);
		WriteInt("cfg_transitions_preinit",cfg_transitions2);
		WriteInt("cfg_transitions_speed",cfg_transitions_speed);
		WriteInt("cfg_transitions_mode",cfg_transition_mode);
		WriteInt("cfg_bkgnd_render",cfg_bkgnd_render);
		WriteInt("cfg_bkgnd_render_color",cfg_bkgnd_render_color);
		WriteInt("cfg_render_prio",cfg_render_prio);
    WriteInt("g_preset_dirty",C_UndoStack::isdirty());
    WriteInt("cfg_prompt_save_preset",config_prompt_save_preset);
		WritePrivateProfileString(AVS_SECTION,"last_preset_name",last_preset,INI_FILE);
    WriteInt("cfg_reuseonresize",config_reuseonresize);
    WriteInt("cfg_log_errors",g_log_errors);
    WriteInt("cfg_reset_vars",g_reset_vars_on_recompile);
    WriteInt("cfg_seh",g_config_seh);

    int x;
    for (x = 0; x < 8; x ++)
    {
      char debugreg[32];
      wsprintf(debugreg,"debugreg_%d",x);
      WriteInt(debugreg,debug_reg[x]);
    }
	}
}


void toggleWharfAmpDock(HWND hwnd)
{
  if (DDraw_IsFullScreen()) return;
  HWND Wharf=g_hwndDlg?GetDlgItem(g_hwndDlg,IDC_RRECT):NULL;
  if (!Wharf) return;

  if (!inWharf)
	{
	  RECT r,r2;

    // show IDC_RRECT, resize IDC_TREE1 down
    GetWindowRect(GetDlgItem(g_hwndDlg,IDC_RRECT),&r);
    GetWindowRect(GetDlgItem(g_hwndDlg,IDC_TREE1),&r2);
    SetWindowPos(GetDlgItem(g_hwndDlg,IDC_TREE1),NULL,0,0,r2.right-r2.left,r.top - 4 - r2.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
    ShowWindow(GetDlgItem(g_hwndDlg,IDC_RRECT),SW_SHOWNA);

#ifdef WA2_EMBED
    GetClientRect(hwnd,&r);
    last_windowed_w=r.right;
    last_windowed_h=r.bottom;
#endif
    inWharf=1;
	  GetWindowRect(Wharf, &r);
    SetParent(hwnd, Wharf);
#ifndef WA2_EMBED
	  SetWindowLong(hwnd, GWL_STYLE, (GetWindowLong(hwnd,GWL_STYLE)&(~WS_POPUP))|WS_CHILD);
#else
    HWND w = g_hWA2ParentWindow;
    while (GetWindowLong(w, GWL_STYLE) & WS_CHILD) 
    {
      w = GetParent(w);
    }
    if (!SendMessage(g_mod->hwndParent, WM_WA_IPC, (int)w, IPC_FF_ISMAINWND)) ShowWindow(w,SW_HIDE);
    else ShowWindow(g_hWA2ParentWindow,SW_HIDE);
#endif
	  SetWindowPos(hwnd, NULL, 0, 0, r.right-r.left, r.bottom-r.top, SWP_NOZORDER|SWP_NOACTIVATE);
	  DDraw_Resize(r.right-r.left, r.bottom-r.top,cfg_fs_d&2);
	}
  else
	{

	  RECT r,r2;
    // hide IDC_RRECT, resize IDC_TREE1 up
    GetWindowRect(GetDlgItem(g_hwndDlg,IDC_RRECT),&r);
    GetWindowRect(GetDlgItem(g_hwndDlg,IDC_TREE1),&r2);
    ShowWindow(GetDlgItem(g_hwndDlg,IDC_RRECT),SW_HIDE);
    SetWindowPos(GetDlgItem(g_hwndDlg,IDC_TREE1),NULL,0,0,r2.right-r2.left,r.bottom - r2.top - 2,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

#ifndef WA2_EMBED
	  SetWindowLong(hwnd, GWL_STYLE, (GetWindowLong(hwnd,GWL_STYLE)&(~(WS_CHILD|WS_VISIBLE)))|WS_POPUP);
	  SetParent(hwnd, NULL);
	  SetWindowPos(hwnd, NULL, cfg_x, cfg_y, cfg_w,cfg_h, SWP_NOZORDER|SWP_NOACTIVATE);
    DDraw_Resize(cfg_w-7-6,cfg_h-15-5,cfg_fs_d&2);
    ShowWindow(hwnd,SW_SHOWNA);
#else
	  SetParent(hwnd, g_hWA2ParentWindow);
    DDraw_Resize(last_windowed_w,last_windowed_h,cfg_fs_d&2);

    HWND w = g_hWA2ParentWindow;
    while (GetWindowLong(w, GWL_STYLE) & WS_CHILD) 
    {
      w = GetParent(w);
    }
    if (SendMessage(g_mod->hwndParent, WM_WA_IPC, (int)w, IPC_FF_ISMAINWND)) 
      w=g_hWA2ParentWindow;

    PostMessage(GetParent(hwnd),WM_SIZE,0,0);

    ShowWindow(w,SW_SHOWNA);

    //SetWindowPos(hwnd,0,0,0,cfg_w,cfg_h,SWP_NOACTIVATE|SWP_NOZORDER);
    //SetTimer(hwnd,66,500,NULL);
#endif
    InvalidateRect(Wharf,NULL,TRUE);
	  inWharf=0;
	}	
}

int findInMenu(HMENU parent, HMENU sub, UINT id, char *buf, int buf_len)
{
  int x,l=GetMenuItemCount(parent);
  char *bufadd=buf+strlen(buf);
  bufadd[0]='\\';
  for (x = 0; x < l; x ++)
  {
    MENUITEMINFO mi={sizeof(mi),MIIM_SUBMENU|MIIM_TYPE|MIIM_ID,};
    mi.dwTypeData=bufadd+1;
    mi.cch=buf_len - (bufadd-buf+2);
    GetMenuItemInfo(parent,x,TRUE,&mi);
    if (mi.hSubMenu)
    {
      if (sub && mi.hSubMenu == sub)
        return 1;
      if (findInMenu(mi.hSubMenu,sub,id,buf,buf_len)) 
        return 1;
    }
    else
    {
      if (!sub && id && mi.wID == id)
        return 1;
    }
  }
  bufadd[0]=0;
  return 0;
}

static int find_preset(char *parent_path, int dir, char *lastpreset, char *newpreset, int *state)
{
	HANDLE h;
	WIN32_FIND_DATA d;
  char dirmask[4096];
  wsprintf(dirmask,"%s\\*.avs",parent_path);
	h = FindFirstFile(dirmask,&d);
	if (h != INVALID_HANDLE_VALUE)
	{
		do 
    {
      if (!(d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
        wsprintf(dirmask,"%s\\%s",parent_path,d.cFileName);
        
        if (lastpreset)
        {
          if (*state)
          {
            strcpy(newpreset,dirmask);
            FindClose(h);
            return 1;
          }
          if (dir > 0)
          {
            if (!newpreset[0]) // save the first one we find, in case we fail (wrap)
              strcpy(newpreset,dirmask);

            if (!stricmp(dirmask,lastpreset)) *state=1;
          }
          if (dir < 0)
          {
            if (!stricmp(dirmask,lastpreset))
            {
              if (newpreset[0]) { // if we find it first, skip it so we can go to the end :)
                FindClose(h);
                return 1;
              }
            }
            strcpy(newpreset,dirmask);
          }
          
        }
        else
        {
          int cnt=++(*state);
          if (cnt < 1) cnt=1;
          int r=((rand()&1023)<<10)|(rand()&1023);
          if (r < (1024*1024)/cnt)
          {
            strcpy(newpreset,dirmask);
          }
        }
      }
		} while (FindNextFile(h,&d));
		FindClose(h);
	}
  wsprintf(dirmask,"%s\\*.*",parent_path);
	h = FindFirstFile(dirmask,&d);
	if (h != INVALID_HANDLE_VALUE)
	{
		do 
    {
      if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && d.cFileName[0] != '.')
      {
        wsprintf(dirmask,"%s\\%s",parent_path,d.cFileName);
        if (find_preset(dirmask,dir,lastpreset,newpreset,state)) {
          FindClose(h);
          return 1;
        }
      }
		} while (FindNextFile(h,&d));
		FindClose(h);
  }
  return 0;
}

void next_preset(HWND hwnd) {
  g_rnd_cnt=0;

  if (readyToLoadPreset(hwnd,0))
  {
    char dirmask[2048];
    char i_path[1024];
    if (config_pres_subdir[0]) wsprintf(i_path,"%s\\%s",g_path,config_pres_subdir);
    else strcpy(i_path,g_path);

    dirmask[0]=0;

    int state=0;
    find_preset(i_path,1,last_preset,dirmask,&state);

    if (dirmask[0] && stricmp(last_preset,dirmask))
    {
      if (g_render_transition->LoadPreset(dirmask,2) != 2)
  	    lstrcpyn(last_preset,dirmask,sizeof(last_preset));
    }
  }
}

void random_preset(HWND hwnd) {
  g_rnd_cnt=0;
  if (readyToLoadPreset(hwnd,0))
  {
    char dirmask[2048];
    char i_path[1024];
    if (config_pres_subdir[0]) wsprintf(i_path,"%s\\%s",g_path,config_pres_subdir);
    else strcpy(i_path,g_path);

    dirmask[0]=0;

    int state=0;
    find_preset(i_path,0,NULL,dirmask,&state);

    if (dirmask[0])
    {  			
      if (g_render_transition->LoadPreset(dirmask,4) != 2)
        lstrcpyn(last_preset,dirmask,sizeof(last_preset));
    }
  }
}

void previous_preset(HWND hwnd) {
  g_rnd_cnt=0;
  if (readyToLoadPreset(hwnd,0))
  {
    char dirmask[2048];
    char i_path[1024];
    if (config_pres_subdir[0]) wsprintf(i_path,"%s\\%s",g_path,config_pres_subdir);
    else strcpy(i_path,g_path);

    dirmask[0]=0;

    int state=0;
    find_preset(i_path,-1,last_preset,dirmask,&state);

    if (dirmask[0] && stricmp(last_preset,dirmask))
    {
      if (g_render_transition->LoadPreset(dirmask,2) != 2)
        lstrcpyn(last_preset,dirmask,sizeof(last_preset));
    }
  }
}

static HMENU presetTreeMenu;
static int presetTreeCount;

void DoPopupMenu() {
  // Winamp3 Bug#331: Don't let the popupmenu pop when in fullscreen.
  if (!DDraw_IsFullScreen())
  {
    void DDraw_NoUpdateScreen(int r);
		HANDLE h;
		WIN32_FIND_DATA d;
		char dirmask[1024];

    if (presetTreeMenu) DestroyMenu(presetTreeMenu);

    POINT p;
    int x;
    int insert_pos=0, directory_pos=0;
    presetTreeMenu=CreatePopupMenu();

    {
      MENUITEMINFO i={sizeof(i),};
      i.fMask=MIIM_TYPE|MIIM_DATA|MIIM_ID;
      i.fType=MFT_STRING;
      i.dwItemData=0;

		  i.wID = 1024;
		  i.dwTypeData=WASABI_API_LNGSTRING(IDS_FULLSCREEN);
		  i.cch=strlen(i.dwTypeData);
		  InsertMenuItem(presetTreeMenu,insert_pos++,TRUE,&i);

      if (!inWharf)
      {
		    i.wID = 256;
		    i.dwTypeData=WASABI_API_LNGSTRING(IDS_AVS_EDITOR);
		    i.cch=strlen(i.dwTypeData);
		    InsertMenuItem(presetTreeMenu,insert_pos++,TRUE,&i);
      }

  	  if (!DDraw_IsFullScreen())
		  {
		    i.wID = 512;
	  		i.dwTypeData=WASABI_API_LNGSTRING(IDS_DOCK_IN_AVS_EDITOR);
		  	i.cch=strlen(i.dwTypeData);
		    InsertMenuItem(presetTreeMenu,insert_pos++,TRUE,&i);
		  }

      i.wID=0;
      i.fType=MFT_SEPARATOR;
      InsertMenuItem(presetTreeMenu,insert_pos++,TRUE,&i);
    }

    GetCursorPos(&p);
    if (DDraw_IsFullScreen())
    {
      CheckMenuItem(presetTreeMenu,1024,MF_CHECKED);
    }
    if (IsWindowVisible(g_hwndDlg)) CheckMenuItem(presetTreeMenu,256,MF_CHECKED);
    if (inWharf) CheckMenuItem(presetTreeMenu,512,MF_CHECKED);
      
    wsprintf(dirmask,"%s\\*.*",g_path);

    directory_pos=insert_pos;

    presetTreeCount=1025;
		h = FindFirstFile(dirmask,&d);
		if (h != INVALID_HANDLE_VALUE)
		{
			do 
      {
        if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && d.cFileName[0] != '.')
        {
          MENUITEMINFO mi={sizeof(mi),MIIM_SUBMENU|MIIM_TYPE,MFT_STRING,MFS_DEFAULT
          };
          mi.hSubMenu=CreatePopupMenu();
          mi.dwTypeData=d.cFileName;
          mi.cch = strlen(d.cFileName);
          InsertMenuItem(presetTreeMenu,directory_pos++,TRUE,&mi);
          insert_pos++;
        }
        else if (!stricmp(extension(d.cFileName),"avs"))
        {
				  extension(d.cFileName)[-1]=0;
          MENUITEMINFO i={sizeof(i),MIIM_TYPE|MIIM_ID,MFT_STRING,MFS_DEFAULT };
				  i.dwTypeData = d.cFileName;
				  i.cch = strlen(d.cFileName);
				  i.wID=presetTreeCount++;
				  InsertMenuItem(presetTreeMenu,insert_pos++,TRUE,&i);
        }
			} while (FindNextFile(h,&d));
			FindClose(h);
		}

    x=TrackPopupMenu(presetTreeMenu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON,p.x,p.y,0,g_hwnd,NULL);
    if (x == 1024)
    {
      if (DDraw_IsFullScreen())
      {
        if (!cfg_fs_use_overlay) Wnd_GoWindowed(g_hwnd);
      }
  	  else
        Wnd_GoFullScreen(g_hwnd);
    }
    else if (x == 512)
	  {
      if (!inWharf && !cfg_cfgwnd_open)
      {
        cfg_cfgwnd_open=1;
			  ShowWindow(g_hwndDlg,SW_SHOWNA);
        CfgWnd_RePopIfNeeded();
      }
	    toggleWharfAmpDock(g_hwnd);
	  }
    else if (x == 256)
	  {
      SendMessage(g_hwnd,WM_USER+33,0,0);
	  }
    else if (x >= 1025)
    {
      char buf[2048];
      buf[0]=0;
      if (readyToLoadPreset(g_hwnd,0))
      {
        if (findInMenu(presetTreeMenu,0,x,buf,2048))
        {
          char temp[4096];
          wsprintf(temp,"%s%s.avs",g_path,buf);
          if (g_render_transition->LoadPreset(temp,1) != 2)
					  lstrcpyn(last_preset,temp,sizeof(last_preset));
        }
        else
        {
//            g_render_transition->LoadPreset
//          wsprintf(temp,"%s\\%s",g_path,curfilename);
        }
      }
    }

    DestroyMenu(presetTreeMenu);
    presetTreeMenu=0;

  }
}
 
static LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if ( DDraw_IsFullScreen() &&
         !cfg_fs_use_overlay &&
         ( ( message == WM_KEYDOWN && wParam == VK_ESCAPE ) ||
           message == WM_LBUTTONUP ||
           ( message == WM_NCACTIVATE && !wParam ) ||
           message == WM_KILLFOCUS
           )
         )
    {
        Wnd_GoWindowed( hwnd );
        return 0;
    }
    else if ( message == WM_LBUTTONUP )
    {
        if ( !DDraw_IsFullScreen() )
        {
#ifndef WA2_EMBED
            int x = GET_X_LPARAM( lParam );
            int y = GET_Y_LPARAM( lParam );
            RECT r;
            GetClientRect( hwnd, &r );
            if ( x > r.right - 12 && x < r.right - 3 && y > r.top + 3 && y < r.top + 13 )
            {
                g_in_destroy = 1;
                DestroyWindow( hwnd );
            }
#endif
            /*
          else
          {
            static int lastdblclk;
            if (lastdblclk && GetTickCount()-lastdblclk > 1000 && !inWharf)
              DDraw_SetStatusText("double-click for configuration",600);
            lastdblclk=GetTickCount();
          }
          */
        }
    }
    if ( message == WM_LBUTTONDOWN )
    {
        SetFocus( g_hwnd );
        if ( inWharf ) SetFocus( hwnd );
        SetCapture( hwnd );
        return 0;
    }
    if ( message == WM_LBUTTONUP )
    {
        POINT p;
        RECT r;
        p.x = GET_X_LPARAM( lParam );
        p.y = GET_Y_LPARAM( lParam );
        ClientToScreen( hwnd, &p );
        if ( inWharf )
        {
            GetWindowRect( g_hwndDlg, &r );
            if ( !PtInRect( &r, p ) )
            {
                toggleWharfAmpDock( hwnd );
            }
        }
        else
        {
            RECT r2;
            GetWindowRect( GetDlgItem( g_hwndDlg, IDC_TREE1 ), &r );
            GetWindowRect( hwnd, &r2 );
            if ( PtInRect( &r, p ) && cfg_cfgwnd_open && !PtInRect( &r2, p ) )
            {
                toggleWharfAmpDock( hwnd );
            }
        }
        ReleaseCapture();
        return 0;
    }
    if ( message == WM_LBUTTONDBLCLK && !DDraw_IsFullScreen() )
    {
        if ( cfg_fs_dblclk )
        {
            if ( DDraw_IsFullScreen() )
            {
                if ( !cfg_fs_use_overlay ) Wnd_GoWindowed( hwnd );
            }
            else
                Wnd_GoFullScreen( hwnd );
        }
        else
        {
            if ( inWharf )
            {
                toggleWharfAmpDock( hwnd );
            }
            else if ( IsWindowVisible( g_hwndDlg ) )
            {
                cfg_cfgwnd_open = 0;
                ShowWindow( g_hwndDlg, SW_HIDE );
            }
            else
            {
                cfg_cfgwnd_open = 1;
                ShowWindow( g_hwndDlg, SW_SHOWNA );
                CfgWnd_RePopIfNeeded();
            }
        }
        return 0;
    }

    switch ( message )
    {
        case WM_USER + 1667:
            if ( wParam == 1 && lParam == 2 )
            {
                char title[ 32 ];
                MessageBox( hwnd, WASABI_API_LNGSTRING( IDS_CANNOT_GO_FULLSCREEN_WHEN_VIDEO_PLAYING ),
                            WASABI_API_LNGSTRING_BUF( IDS_AVS_FULLSCREEN, title, 32 ),
                            MB_OK | MB_ICONINFORMATION );
            }
            return 0;
        case WM_USER + 1666:
            if ( wParam == 1 && lParam == 15 )
            {
                if ( DDraw_IsFullScreen() )
                {
                    if ( cfg_fs_use_overlay ) SetFocus( hwnd ); // kill overlay window
                    else Wnd_GoWindowed( hwnd );
                }
            }
            return 0;
        case WM_INITMENUPOPUP:
            if ( HIWORD( lParam ) == 0 && presetTreeMenu && !GetMenuItemCount( (HMENU) wParam ) )
            {
                char buf[ 2048 ];
                buf[ 0 ] = 0;
                if ( findInMenu( presetTreeMenu, (HMENU) wParam, 0, buf, 2048 ) )
                {
                    HANDLE h;
                    WIN32_FIND_DATA d;
                    char dirmask[ 4096 ];
                    wsprintf( dirmask, "%s%s\\*.*", g_path, buf );
                    int directory_pos = 0, insert_pos = 0;
                    // build menu
                    h = FindFirstFile( dirmask, &d );
                    if ( h != INVALID_HANDLE_VALUE )
                    {
                        do
                        {
                            if ( d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && d.cFileName[ 0 ] != '.' )
                            {
                                MENUITEMINFO mi = { sizeof( mi ),MIIM_SUBMENU | MIIM_TYPE,MFT_STRING,MFS_DEFAULT };
                                mi.hSubMenu = CreatePopupMenu();
                                mi.dwTypeData = d.cFileName;
                                mi.cch = strlen( d.cFileName );
                                InsertMenuItem( (HMENU) wParam, directory_pos++, TRUE, &mi );
                                insert_pos++;
                            }
                            else if ( !stricmp( extension( d.cFileName ), "avs" ) )
                            {
                                extension( d.cFileName )[ -1 ] = 0;
                                MENUITEMINFO i = { sizeof( i ),MIIM_TYPE | MIIM_ID,MFT_STRING,MFS_DEFAULT };
                                i.dwTypeData = d.cFileName;
                                i.cch = strlen( d.cFileName );
                                i.wID = presetTreeCount++;
                                InsertMenuItem( (HMENU) wParam, insert_pos++, TRUE, &i );
                            }
                        } while ( FindNextFile( h, &d ) );
                        FindClose( h );
                    }
                }
            }
            return 0;
        case WM_RBUTTONUP:
            DoPopupMenu();
            return 0;
        case WM_USER + 33:
            DDraw_SetStatusText( "", 100 );
            if ( inWharf )
            {
                toggleWharfAmpDock( hwnd );
            }
            else if ( IsWindowVisible( g_hwndDlg ) )
            {
                cfg_cfgwnd_open = 0;
                ShowWindow( g_hwndDlg, SW_HIDE );
            }
            else
            {
                cfg_cfgwnd_open = 1;
                ShowWindow( g_hwndDlg, SW_SHOWNA );
                CfgWnd_RePopIfNeeded();
            }
            return 0;
        case WM_USER + 32:
            Wnd_GoFullScreen( hwnd );
            return 0;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            if ( ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) && wParam == VK_F4 )
            {
                SendMessage( hwnd, WM_CLOSE, 0, 0 );
                return 0;
            }
            if ( ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) && wParam == VK_F4 )
            {
                PostMessage( hwnd_WinampParent, message, wParam, lParam );
                break;
            }

            if ( wParam == VK_SPACE )
            {
            do_random:
                random_preset( hwnd );
            }
            else if ( wParam == 0x55 )
            {
                next_preset( hwnd );
            }
            else if ( wParam == 0x59 )
            {
                previous_preset( hwnd );
            }
            else if ( wParam == VK_RETURN )
            {
                if ( GetAsyncKeyState( VK_MENU ) & 0x8000 )
                {
                    if ( DDraw_IsFullScreen() )
                    {
                        if ( !cfg_fs_use_overlay ) Wnd_GoWindowed( hwnd );
                    }
                    else
                        Wnd_GoFullScreen( hwnd );
                }
            }
            else if ( wParam == /* VK_T */ 0x54 )
            {
                extern int draw_title_p;
                draw_title_p = 2;
            }
            else if ( wParam == /* VK_F */ 0x52 + 'F' - 'R' )
            {
                cfg_fs_fps ^= 1;
                DDraw_SetStatusText( WASABI_API_LNGSTRING( cfg_fs_fps & 1 ? IDS_FULLSCREEN_FPS_ON : IDS_FULLSCREEN_FPS_OFF ) );
            }
            else if ( wParam == /* VK_R */ 0x52 )
            {
                cfg_fs_rnd = !cfg_fs_rnd;
                g_rnd_cnt = 0;
                DDraw_SetStatusText( WASABI_API_LNGSTRING( cfg_fs_rnd ? IDS_RANDOM_PRESETS_ON : IDS_RANDOM_PRESETS_OFF ) );
            }
            else if ( wParam >= VK_F1 && wParam <= VK_F12 )
            {
                char s[ 128 ], *st, stBuf[ 48 ];
                if ( GetAsyncKeyState( VK_CONTROL ) & ( 1 << 15 ) )
                {
                    st = WASABI_API_LNGSTRING_BUF( IDS_SAVED_TO, stBuf, 48 );
                    WritePreset( wParam - VK_F1 );
                }
                else
                {
                    if ( !readyToLoadPreset( hwnd, 0 ) ) return 0;

                    if ( LoadPreset( wParam - VK_F1 ) ) st = WASABI_API_LNGSTRING_BUF( IDS_LOADED_FROM, stBuf, 48 );
                    else st = WASABI_API_LNGSTRING_BUF( IDS_ERROR_LOADING_FROM, stBuf, 48 );
                }
                wsprintf( s, "%s F%d", st, wParam - VK_F1 + 1 );
                DDraw_SetStatusText( s );
            }
            else if ( wParam >= '0' && wParam <= '9' )
            {
                int n = 0;
                char s[ 128 ], *st, stBuf[ 48 ];
                if ( GetAsyncKeyState( VK_SHIFT ) & ( 1 << 15 ) )
                    n = 10;
                if ( GetAsyncKeyState( VK_CONTROL ) & ( 1 << 15 ) )
                {
                    st = WASABI_API_LNGSTRING_BUF( IDS_SAVED_TO, stBuf, 48 );
                    WritePreset( wParam - '0' + 12 + n );
                }
                else
                {
                    if ( !readyToLoadPreset( hwnd, 0 ) ) return 0;
                    if ( LoadPreset( wParam - '0' + 12 + n ) ) st = WASABI_API_LNGSTRING_BUF( IDS_LOADED_FROM, stBuf, 48 );
                    else st = WASABI_API_LNGSTRING_BUF( IDS_ERROR_LOADING_FROM, stBuf, 48 );
                }
                wsprintf( s, "%s %s%d", st, n ? WASABI_API_LNGSTRING( IDS_SHIFT_ ) : "", wParam - '0' );
                DDraw_SetStatusText( s );
            }
            else
            {
                if ( wParam == 0x4B && DDraw_IsFullScreen() )
                {
                    if ( !cfg_fs_use_overlay ) Wnd_GoWindowed( hwnd );
                }
                else PostMessage( hwnd_WinampParent, message, wParam, lParam );
            }
            return 0;
        case WM_TIMER:
#ifndef WA2_EMBED
            if ( wParam == 66 )
            {
                KillTimer( hwnd, 66 );
                SetWindowPos( hwnd, 0, 0, 0, cfg_w, cfg_h, SWP_NOACTIVATE | SWP_NOZORDER );
            }
#endif
            if ( wParam == 88 )
            {
                KillTimer( hwnd, 88 );
                if ( cfg_cfgwnd_open )
                {
                    ShowWindow( g_hwndDlg, SW_SHOWNA );
                    UpdateWindow( g_hwndDlg );
                    CfgWnd_RePopIfNeeded();
                }

                if ( need_redock && g_hwndDlg )
                {
                    need_redock = 0;
                    toggleWharfAmpDock( hwnd );
                }
            }
            if ( wParam == 32 )
            {
                DWORD a;

#ifndef REAPLAY_PLUGIN
                if ( SendMessageTimeout( hwnd_WinampParent, WM_USER, (WPARAM) 0, 201, SMTO_BLOCK, 1000, &a ) && a )
                {
                    if ( strcmp( g_skin_name, (char *) a ) )
                    {
                        lstrcpyn( g_skin_name, (char *) a, sizeof( g_skin_name ) );
                        PostMessage( hwnd, WM_DISPLAYCHANGE, 0, 0 );
                    }
                }
#endif

                if ( g_rnd_cnt >= 0 && g_rnd_cnt++ >= max( cfg_fs_rnd_time, 1 ) )
                {
                    g_rnd_cnt = 0;
                    if ( ( !IsWindowVisible( g_hwndDlg ) || DDraw_IsFullScreen() ) && cfg_fs_rnd )
                        goto do_random;
                }
            }
            if ( wParam == 30 )
            {
                KillTimer( hwnd, 30 );
                if ( !DDraw_IsFullScreen() && !inWharf )
                {
                    InvalidateRect( hwnd, NULL, FALSE );
                    int tm = ( GetWindowLong( g_mod->hwndParent, GWL_EXSTYLE ) & WS_EX_TOPMOST ) == WS_EX_TOPMOST;
                    SetWindowPos( hwnd, tm ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
                    RECT r;
                    GetClientRect_adj( hwnd, &r );
                    DDraw_Resize( r.right - r.left, r.bottom - r.top, cfg_fs_d & 2 );
                }
            }
#ifndef WA2_EMBED
            if ( wParam == 29 )
            {
                if ( !IsIconic( hwnd_WinampParent ) )
                {
                    KillTimer( hwnd, 29 );
                    g_mod->Quit( g_mod );
                    g_minimized = 0;
                    g_fakeinit = 1;
                    g_mod->Init( g_mod );
                    g_fakeinit = 0;
                    SetActiveWindow( g_mod->hwndParent );
                }
            }
#endif
            return 0;
        case WM_DROPFILES:
        {
            char temp[ MAX_PATH ];
            HDROP hdrop = (HDROP) wParam;
            DragQueryFile( hdrop, 0, temp, sizeof( temp ) );
            if ( readyToLoadPreset( hwnd, 0 ) )
            {
                if ( !stricmp( extension( temp ), "avs" ) )
                {
                    if ( g_render_transition->LoadPreset( temp, 1 ) != 2 )
                        lstrcpyn( last_preset, temp, sizeof( last_preset ) );
                }
            }
            DragFinish( hdrop );
        }
        return 0;
        case WM_DISPLAYCHANGE:
#ifndef WA2_EMBED
            SelectObject( hFrameDC, hFrameBitmap_old );
            DeleteObject( hFrameBitmap );
            hFrameBitmap = NULL;
            {
                char buf[ MAX_PATH ];
                if ( SendMessage( hwnd_WinampParent, WM_USER, (WPARAM) buf, 201 ) )
                {
                    strcat( buf, "\\avs.bmp" );
                    hFrameBitmap = (HBITMAP) LoadImage( g_hInstance, buf, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE );
                }
                if ( !hFrameBitmap )
                    hFrameBitmap = (HBITMAP) LoadImage( g_hInstance, MAKEINTRESOURCE( IDB_BITMAP1 ), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );
            }
            hFrameBitmap_old = (HBITMAP) SelectObject( hFrameDC, hFrameBitmap );
#endif
            InvalidateRect( hwnd, NULL, FALSE );
            return 0;
        case WM_CREATE:
            g_hwnd = hwnd;

            if ( DDraw_Init() )
            {
                return 1;
            }

#ifndef WA2_EMBED

            {
                char buf[ MAX_PATH ];
                int a;
                if ( ( a = SendMessage( hwnd_WinampParent, WM_USER, (WPARAM) buf, 201 ) ) )
                {
                    lstrcpyn( g_skin_name, (char *) a, sizeof( g_skin_name ) );
                    strcat( buf, "\\avs.bmp" );
                    hFrameBitmap = (HBITMAP) LoadImage( g_hInstance, buf, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE );
                }
                if ( !hFrameBitmap )
                    hFrameBitmap = (HBITMAP) LoadImage( g_hInstance, MAKEINTRESOURCE( IDB_BITMAP1 ), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );
            }
            hFrameDC = (HDC) CreateCompatibleDC( NULL );
            hFrameBitmap_old = (HBITMAP) SelectObject( hFrameDC, hFrameBitmap );
#endif
            //FG> This totally fucks up a child layered window painting in wa3, i'm not even sure that's a good thing for wa2... basically the child window gets excluded from the layered update and ends up updating behind the layer, on top of the desktop
#ifndef WA2_EMBED
            SetWindowLong( hwnd, GWL_STYLE, 0 );
            SetWindowPos( hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME | SWP_NOACTIVATE );
#endif
            SetTimer( hwnd, 32, 1000, NULL );
            return 0;
#ifndef WA2_EMBED
        case WM_NCHITTEST:
        {
            int x = GET_X_LPARAM( lParam );
            int y = GET_Y_LPARAM( lParam );
            RECT r;
            GetWindowRect( hwnd, &r );
            if ( inWharf )
                return HTCLIENT;
            if ( DDraw_IsFullScreen() || ( x > r.right - 12 && x < r.right - 3 && y > r.top + 3 && y < r.top + 13 ) )
                return HTCLIENT;
            if ( x < r.left + 6 && y > r.bottom - 6 ) return HTBOTTOMLEFT;
            if ( x > r.right - 6 && y > r.bottom - 6 ) return HTBOTTOMRIGHT;
            if ( x < r.left + 6 && y < r.top + 6 ) return HTTOPLEFT;
            if ( x > r.right - 6 && y < r.top + 6 ) return HTTOPRIGHT;
            if ( y < r.top + 6 ) return HTTOP;
            if ( y > r.bottom - 6 ) return HTBOTTOM;
            if ( x < r.left + 6 ) return HTLEFT;
            if ( x > r.right - 6 ) return HTRIGHT;
            if ( y < r.top + 15 ) return HTCAPTION;
        }
        return HTCLIENT;
#endif
        case WM_CLOSE:
#ifndef WA2_EMBED
            g_in_destroy = 1;
            DestroyWindow( hwnd );
#else
            g_in_destroy = 1;
            SetForegroundWindow( g_mod->hwndParent );
            DestroyWindow( myWindowState.me );
#endif
            return 0;
        case WM_DESTROY:
#ifndef WA2_EMBED
            SelectObject( hFrameDC, hFrameBitmap_old );
            DeleteObject( hFrameDC );
            DeleteObject( hFrameBitmap );
#endif
            g_ThreadQuit = 1;
            if ( !g_minimized )
            {
                PostQuitMessage( 0 );
            }
            return 0;
        case WM_MOVE:
            if ( !DDraw_IsFullScreen() && !inWharf )
            {
#ifndef WA2_EMBED
                int w;
                RECT r;
                RECT r2;
                int xPos, yPos, f = 0;
                xPos = (int) (short) LOWORD( lParam );    // horizontal position 
                yPos = (int) (short) HIWORD( lParam );    // vertical position 
                GetClientRect( hwnd, &r2 );

                for ( w = 0; w < 5; w++ )
                {
                    HWND hw;
                    if ( w == 0 )
                    {
                        RECT r2;
                        GetWindowRect( hwnd, &r2 );
                        my_getViewport( &r, &r2 );
                    }
                    else if ( w == 1 && IsWindowVisible( hwnd_WinampParent ) )
                        GetWindowRect( hwnd_WinampParent, &r );
                    else if ( w == 2 && ( hw = FindWindowEx( NULL, NULL, "Winamp EQ", NULL ) )
                              && IsWindowVisible( hw ) )
                        GetWindowRect( hw, &r );
                    else if ( w == 3 && ( hw = FindWindowEx( NULL, NULL, "Winamp PE", NULL ) )
                              && IsWindowVisible( hw ) )
                        GetWindowRect( hw, &r );
                    else if ( w == 4 && ( hw = FindWindowEx( NULL, NULL, "Winamp MB", NULL ) )
                              && IsWindowVisible( hw ) )
                        GetWindowRect( hw, &r );
                    else continue;

#define intersect(x1,x2,y1,y2)  \
					(((x1)>(y1)&&(x1)<(y2))||((x2)>(y1)&&(x2)<(y2))||((y1)>(x1)&&(y1)<(x2))||((y2)>(x1)&&(y2)<(x2)))

                    if ( xPos > r.left - 10 && xPos < r.left + 10 && intersect( yPos, yPos + r2.bottom, r.top, r.bottom ) )
                    {
                        xPos = r.left;
                        f++;
                    }
                    if ( yPos > r.top - 10 && yPos < r.top + 10 && intersect( xPos, xPos + r2.right, r.left, r.right ) )
                    {
                        yPos = r.top;
                        f++;
                    }
                    if ( xPos + r2.right > r.right - 10 && xPos + r2.right < r.right + 10 && intersect( yPos, yPos + r2.bottom, r.top, r.bottom ) )
                    {
                        xPos = r.right - r2.right;
                        f++;
                    }
                    if ( yPos + r2.bottom > r.bottom - 10 && yPos + r2.bottom < r.bottom + 10 && intersect( xPos, xPos + r2.right, r.left, r.right ) )
                    {
                        yPos = r.bottom - r2.bottom;
                        f++;
                    }



                    if ( xPos + r2.right > r.left - 10 && xPos + r2.right < r.left + 10 && intersect( yPos, yPos + r2.bottom, r.top, r.bottom ) )
                    {
                        xPos = r.left - r2.right;
                        f++;
                    }
                    if ( yPos + r2.bottom > r.top - 10 && yPos + r2.bottom < r.top + 10 && intersect( xPos, xPos + r2.right, r.left, r.right ) )
                    {
                        yPos = r.top - r2.bottom;
                        f++;
                    }
                    if ( xPos > r.right - 10 && xPos < r.right + 10 && intersect( yPos, yPos + r2.bottom, r.top, r.bottom ) )
                    {
                        xPos = r.right;
                        f++;
                    }
                    if ( yPos > r.bottom - 10 && yPos < r.bottom + 10 && intersect( xPos, xPos + r2.right, r.left, r.right ) )
                    {
                        yPos = r.bottom;
                        f++;
                    }
                }

                if ( f )
                {
                    SetWindowPos( hwnd, NULL, xPos, yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
                }
                {
                    RECT r;
                    GetWindowRect( hwnd, &r );
                    cfg_x = r.left;
                    cfg_y = r.top;
                    cfg_w = r.right - r.left;
                    cfg_h = r.bottom - r.top;
                }
#endif
            }
            return 0;
        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO mmi = (LPMINMAXINFO) lParam;
            mmi->ptMinTrackSize.x = 80;
            mmi->ptMinTrackSize.y = 40;
        }
        return 0;
        case WM_SETCURSOR:
            if ( DDraw_IsFullScreen() )
            {
                SetCursor( NULL );
                return TRUE;
            }
            break;
        case WM_SIZE:
            if ( wParam != SIZE_MINIMIZED )
            {
                if ( !DDraw_IsFullScreen() )
                {
                    if ( !inWharf )
                    {
#ifndef WA2_EMBED
                        RECT r;
                        GetWindowRect( hwnd, &r );
                        cfg_x = r.left;
                        cfg_y = r.top;
                        cfg_w = r.right - r.left;
                        cfg_h = r.bottom - r.top;
#endif
                        DDraw_BeginResize();
                        KillTimer( hwnd, 30 );
                        SetTimer( hwnd, 30, 33, NULL );
                    }
                }
            }
            break;
        case WM_WINDOWPOSCHANGING:
        {
            LPWINDOWPOS lpwp = (LPWINDOWPOS) lParam;
            if ( cfg_dont_min_avs && !g_in_destroy && lpwp->flags & SWP_HIDEWINDOW && !g_minimized && !inWharf )
            {
#ifndef WA2_EMBED
                g_minimized = 1;
                g_mod->Quit( g_mod );
                g_fakeinit = 1;
                g_mod->Init( g_mod );
                g_fakeinit = 0;
                if ( ( GetWindowLong( g_mod->hwndParent, GWL_EXSTYLE ) & WS_EX_TOPMOST ) == WS_EX_TOPMOST )
                    SetWindowPos( g_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
                SetTimer( g_hwnd, 29, 500, NULL );
#endif
            }
        }
        return 0;
        case WM_PAINT:
            if ( !DDraw_IsFullScreen() )
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint( hwnd, &ps );
#ifndef WA2_EMBED
                RECT r;
                HDC tempdc = CreateCompatibleDC( hdc );
                GetClientRect( hwnd, &r );
                HBITMAP oldbm, tempbm = CreateCompatibleBitmap( hdc, 7, r.bottom - 20 );
                oldbm = (HBITMAP) SelectObject( tempdc, tempbm );
                if ( !inWharf )
                { // draw window frame 
                    SetStretchBltMode( hdc, COLORONCOLOR );
                    SetStretchBltMode( tempdc, COLORONCOLOR );
                    BitBlt( hdc, 0, 0, 50, 15, hFrameDC, 15, 0, SRCCOPY ); // top left
                    StretchBlt( hdc, 50, 0, r.right - 50 - 16, 15, hFrameDC, 66, 0, 14, 15, SRCCOPY ); // top middle
                    BitBlt( hdc, r.right - 16, 0, 16, 15, hFrameDC, 81, 0, SRCCOPY ); // top right

                    StretchBlt( tempdc, 0, 0, 7, r.bottom - 15 - 5, hFrameDC, 0, 16, 7, 172, SRCCOPY ); // left middle
                    BitBlt( hdc, 0, 15, 7, r.bottom - 15 - 5, tempdc, 0, 0, SRCCOPY );
                    StretchBlt( tempdc, 0, 0, 6, r.bottom - 15 - 5, hFrameDC, 8, 16, 6, 172, SRCCOPY ); // right middle
                    BitBlt( hdc, r.right - 6, 15, 6, r.bottom - 15 - 5, tempdc, 0, 0, SRCCOPY );

                    BitBlt( hdc, 0, r.bottom - 5, 50, 15, hFrameDC, 15, 16, SRCCOPY ); // bottom left
                    StretchBlt( hdc, 50, r.bottom - 5, r.right - 50 - 16, 5, hFrameDC, 66, 16, 14, 5, SRCCOPY ); // bottom middle
                    BitBlt( hdc, r.right - 16, r.bottom - 5, 16, 5, hFrameDC, 81, 16, SRCCOPY ); // bottom right
                }
                SelectObject( tempdc, oldbm );
                DeleteObject( tempbm );
                DeleteObject( tempdc );
#endif
                EndPaint( hwnd, &ps );
                return 0;
            }
            break;
#ifdef WA2_EMBED
        case WM_COMMAND:
        {
            int id = LOWORD( wParam );
            switch ( id )
            {
                case ID_VIS_NEXT: next_preset( hwnd ); break;
                case ID_VIS_PREV: previous_preset( hwnd ); break;
                case ID_VIS_RANDOM:
                {
                    int v = HIWORD( wParam ) ? 1 : 0;
                    if ( wParam >> 16 == 0xFFFF )
                    {
                        SendMessage( g_mod->hwndParent, WM_WA_IPC, cfg_fs_rnd, IPC_CB_VISRANDOM );
                        break;
                    }
                    cfg_fs_rnd = v;
                    if ( cfg_fs_rnd ) random_preset( hwnd );
                    DDraw_SetStatusText( WASABI_API_LNGSTRING( cfg_fs_rnd ? IDS_RANDOM_PRESETS_ON : IDS_RANDOM_PRESETS_OFF ) );
                    break;
                }
                case ID_VIS_FS: Wnd_GoFullScreen( hwnd ); break;
                case ID_VIS_CFG: SendMessage( hwnd, WM_USER + 33, 0, 0 ); break;
                case ID_VIS_MENU: DoPopupMenu(); break;
            }
            break;
        }
#endif
    }
    return DefWindowProc( hwnd, message, wParam, lParam );
}
