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
#include <ddraw.h>
#include "draw.h"
#include <stdio.h>
#include <process.h>
#include "r_defs.h"
#include "resource.h"
#include "vis.h"
#include "../Agave/Language/api_language.h"

extern HINSTANCE g_hInstance;

#define RESIZE_ONRESIZE
int draw_title_p=0;
int cfg_cancelfs_on_deactivate=1;
extern int g_dlg_fps,g_dlg_w,g_dlg_h;
extern int cfg_fs_fps,cfg_fs_flip,cfg_fs_height,cfg_fs_use_overlay;
extern int cfg_bkgnd_render, cfg_bkgnd_render_color;
static LPDIRECTDRAW		   g_lpDD;
static LPDIRECTDRAWSURFACE g_lpRenderSurf[2], g_lpPrimSurf, g_lpPrimSurfBack;
static int g_bpp, g_fs, g_noshoww;
int g_w, g_h, g_dsw, g_dsh;
extern HWND g_hwnd;
static CRITICAL_SECTION g_cs;
static int g_windowed_dsize;
static int g_initted, g_fs_flip, g_fs_height, g_fs_w, g_fs_h;
static int nodraw=0;
extern int inWharf;
#ifdef RESIZE_ONRESIZE
static int last_used;
#endif
static LPDIRECTDRAWSURFACE g_lpddsOverlay = NULL;
static LPDIRECTDRAWSURFACE g_lpddsPrimary = NULL;
static int g_overlay_init_ok=0;
typedef enum {
  UYVY,
  YUY2
} fourcc_enum;
static fourcc_enum g_overlay_fourcc=UYVY;
static int g_saved_desktop_values=0;

static char g_saved_desktop_wallpaper[256];
static char g_saved_reg_bkgnd_color[64];
static DWORD g_saved_bkgnd_color;

extern void Wnd_GoWindowed(HWND hwnd);

#define INIT_DIRECTDRAW_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))
DDPIXELFORMAT g_ddpfOverlayFormats[] = 
{   {sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('U','Y','V','Y'),0,0,0,0,0}, // UYVY
    {sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','U','Y','2'),0,0,0,0,0}};  // YUY2

#define NUM_OVERLAY_FORMATS (sizeof(g_ddpfOverlayFormats) / sizeof(g_ddpfOverlayFormats[0]))

static HWND hwndOverlayWnd;

static LRESULT CALLBACK FSOverlayWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static unsigned int start_t;

	if (
    
        (((message == WM_KEYDOWN && (wParam == VK_ESCAPE || wParam == VK_RETURN)) || 
		     message == WM_LBUTTONUP) && GetTickCount()-start_t > 1000)
         
         || 

         (cfg_cancelfs_on_deactivate && ((message == WM_NCACTIVATE && !wParam) ||
         message == WM_KILLFOCUS))
        )
	{
    DestroyWindow(hwnd);
    Wnd_GoWindowed(g_hwnd);
    return 0;
	}
  switch(message)
  {
  case WM_CREATE:
    start_t=GetTickCount();
  return 0;
  case WM_SETCURSOR:
    SetCursor(NULL);
  return TRUE;
  case WM_DESTROY:
    hwndOverlayWnd=0;
  return 0;
  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc=BeginPaint(hwnd,&ps);
      RECT r;
      GetClientRect(hwnd,&r);
      int rv=0xff&(cfg_bkgnd_render_color>>16), gv=0xff&(cfg_bkgnd_render_color>>8), bv=0xff&cfg_bkgnd_render_color;
      HBRUSH b=CreateSolidBrush(RGB(rv,gv,bv));
      SelectObject(ps.hdc, b); 
      Rectangle(ps.hdc, r.left, r.top, r.right, r.bottom); 
      DeleteObject(b);
      EndPaint(hwnd,&ps);
    }
    return 0;
  case WM_KEYDOWN:
    return SendMessage(g_hwnd, message, wParam, lParam);
  }
  return DefWindowProc(hwnd, message, wParam, lParam);
}

static void DD_CreateFullscreenOverlayWindow()
{
  static int inited=0;
  if(!inited)
  {
  	WNDCLASSW wc={0,};
  	wc.style = CS_DBLCLKS|CS_VREDRAW|CS_HREDRAW;
  	wc.lpfnWndProc = FSOverlayWndProc;
  	wc.hInstance = g_hInstance;
  	wc.hbrBackground = NULL;
  	wc.lpszClassName = L"avsfsoverlaywnd";
  	wc.hCursor=NULL;
  	if(!RegisterClassW(&wc)) return;
    inited=1;
  }
  hwndOverlayWnd=CreateWindowExW(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,L"avsfsoverlaywnd",L"",WS_VISIBLE|WS_POPUP,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),NULL,NULL,g_hInstance,0);
}

static void DD_RestoreBkgndSettings()
{
  if (g_saved_desktop_values)
  {
    int e=COLOR_DESKTOP;
    unsigned long c=g_saved_bkgnd_color;
    SetSysColors(1, &e, &c); // FUCKING MESSAGE PUMP AND SETSYSCOLORS
    SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, g_saved_desktop_wallpaper, NULL);
    g_saved_desktop_values=0;
  }
}

void DD_CreateSurfaces(int w, int h, int fsh, int fs, int fsbpp, int flip, int dbl, int fsovl) // fsh is the height to use (not screen res)
{
  int resize_h=8, resize_w=8;
#ifdef RESIZE_ONRESIZE
  int fb_save[64*65+1];
  int fb_save_use=0;
#endif
  EnterCriticalSection(&g_cs);
  nodraw=0;
  if (g_lpDD)
  {
    extern int config_reuseonresize;
#ifdef RESIZE_ONRESIZE
    HRESULT han;
    int ll=!!last_used;
    DDSURFACEDESC d={sizeof(d),};
  	if (config_reuseonresize && g_w && g_h && g_lpRenderSurf[ll] &&
        (han = g_lpRenderSurf[ll]->Lock(NULL,&d,DDLOCK_WAIT,NULL)) == DD_OK) 
    {
      if (d.lpSurface)
      {
        int x,y;
        int dxpos=(g_w<<16)/64;
        int ypos=0;
        int dypos=(g_h<<16)/64;
        int *pp=((int *)d.lpSurface);

        if (g_fs && g_fs_height < g_h) // adjust to use partial screen when 
        {
          int fsy=g_h/2-g_fs_height/2;
          dypos=(g_fs_height<<16)/64;
          pp+=fsy * g_w;
        }

        for (y = 0; y < 64; y++)
        {
          int *p=pp + g_w * (ypos>>16);
          int xpos=0;
          for (x = 0; x < 64; x ++)
          {
            fb_save[(y<<6)+x]=p[xpos>>16];
            xpos+=dxpos;
          }
          ypos+=dypos;
        }
        memset(fb_save+64*64,0,sizeof(int)*65);
        fb_save_use=1;
      }
      g_lpRenderSurf[ll]->Unlock(d.lpSurface);
    }
#endif

		if (g_lpPrimSurf)
		{
			g_lpPrimSurf->Release();
			g_lpPrimSurfBack=g_lpPrimSurf=NULL;
		}
    if (g_lpRenderSurf[0]) g_lpRenderSurf[0]->Release();
    if (g_lpRenderSurf[1]) g_lpRenderSurf[1]->Release();
    g_lpRenderSurf[0]=0;
    g_lpRenderSurf[1]=0;
    if (g_lpddsOverlay) g_lpddsOverlay->Release();
    if (g_lpddsPrimary) g_lpddsPrimary->Release();
    g_lpddsOverlay=g_lpddsPrimary=NULL;
    g_lpDD->Release();
    g_lpDD=NULL;
  }
  
  if (DirectDrawCreate(NULL,&g_lpDD,NULL) != DD_OK) 
	{
    g_lpDD=NULL;
		MessageBox(g_hwnd,WASABI_API_LNGSTRING(IDS_ERROR_CREATING_DDRAW_OBJECT),"DDraw",0);
    LeaveCriticalSection(&g_cs);
		return;
	}
	if (fs && !fsovl)
	{
    g_fs_w=w;
    g_fs_h=h;
    g_fs_height=fsh>>dbl;
    g_fs_flip=flip;
    resize_w=w>>dbl;
		resize_h=h>>dbl;
		if (g_lpDD->SetCooperativeLevel(g_hwnd,DDSCL_NOWINDOWCHANGES|DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN) != DD_OK)
		{
			fs=0;
		}
		else
		{
			if (g_lpDD->SetDisplayMode(w,h,fsbpp) != DD_OK)
			{
				fs=0;
			}
			else
			{
				DDSURFACEDESC DDsd={sizeof(DDsd),};
				DDsd.dwFlags = DDSD_CAPS;
				DDsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
        if (g_fs_flip&1) 
        {
          DDsd.ddsCaps.dwCaps |= DDSCAPS_COMPLEX|DDSCAPS_FLIP;
          DDsd.dwFlags|=DDSD_BACKBUFFERCOUNT;
          DDsd.dwBackBufferCount = 1;
        }
				if (g_lpDD->CreateSurface(&DDsd, &g_lpPrimSurf, NULL) != DD_OK)
				{
					fs=0;
				}
				else
				{
          DDBLTFX ddbfx={sizeof(ddbfx),};
          g_lpPrimSurf->Blt(NULL,NULL,NULL,DDBLT_WAIT|DDBLT_COLORFILL,&ddbfx);
          if (g_fs_flip&1)
          {
						DDSCAPS ddscaps;
						ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
						if (g_lpPrimSurf->GetAttachedSurface(&ddscaps, &g_lpPrimSurfBack) != DD_OK) 
						{
							g_lpPrimSurf->Release();
							fs=0;
						}
            else g_lpPrimSurfBack->Blt(NULL,NULL,NULL,DDBLT_WAIT|DDBLT_COLORFILL,&ddbfx);
          }
          else
            g_lpPrimSurfBack=g_lpPrimSurf;
				}
			}
		}
    SetForegroundWindow(g_hwnd);
  } else {
    g_lpDD->SetCooperativeLevel(g_hwnd,DDSCL_NOWINDOWCHANGES|DDSCL_NORMAL);
    resize_w=(((w>>dbl)+3)&~3);
    g_noshoww=resize_w-(w>>dbl);
    resize_h=h>>dbl;
	}
	g_fs=fs;

  int wh;
  for (wh = 0; wh < 2; wh ++)
	{
		DDSURFACEDESC DDsd={sizeof(DDsd),};
		DDsd.dwFlags = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PITCH|DDSD_PIXELFORMAT;
		DDsd.dwWidth=resize_w;
		DDsd.dwHeight=resize_h;
		DDsd.lPitch=resize_w*sizeof(int);
		DDsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
		DDsd.ddpfPixelFormat.dwSize = sizeof(DDsd.ddpfPixelFormat);
		DDsd.ddpfPixelFormat.dwFlags=DDPF_RGB;
		DDsd.ddpfPixelFormat.dwRGBBitCount = 32;
		DDsd.ddpfPixelFormat.dwRBitMask=0xff0000;
		DDsd.ddpfPixelFormat.dwGBitMask=0x00ff00;
		DDsd.ddpfPixelFormat.dwBBitMask=0x0000ff;
		if (g_lpDD->CreateSurface(&DDsd, &g_lpRenderSurf[wh], NULL) != DD_OK)
		{
      if (wh)
      {
        g_lpRenderSurf[0]->Release();
        g_lpRenderSurf[0]=0;
      }
			g_lpRenderSurf[wh]=0;
			LeaveCriticalSection(&g_cs);
			return;
		}

#ifdef RESIZE_ONRESIZE
    DDSURFACEDESC d={sizeof(d),};
    HRESULT han;
  	if (fb_save_use && g_lpRenderSurf[wh] &&
        (han = g_lpRenderSurf[wh]->Lock(NULL,&d,DDLOCK_WAIT,NULL)) == DD_OK) 
    {
      if (d.lpSurface)
      {
        int x,y;
        int dxpos=(64<<16)/resize_w;
        int ypos=0;
        int *p=(int *)d.lpSurface;
        int h=resize_h;

        if (fs && (fsh>>dbl) < resize_h)
        {
          int fsy=resize_h/2-(((fsh>>dbl)/2));
          p+=fsy * resize_w;
          h -= fsy*2;
        }
        int dypos=(64<<16)/h;

        for (y = 0; y < h; y ++)
        {
          int xpos=0;
          for (x = 0; x < resize_w; x ++)
          {
            *p++ = BLEND4_16((unsigned int *)fb_save + ((ypos>>10)&~63) + (xpos>>16),64,xpos,ypos);
            xpos+=dxpos;
          }
          ypos+=dypos;
        }
#ifndef NO_MMX
        __asm emms;
#endif
      }
      g_lpRenderSurf[wh]->Unlock(d.lpSurface);
    }
#endif
	}
	g_w=resize_w;
	g_h=resize_h;
  g_dsh=g_h<<dbl;
  g_dsw=g_w<<dbl;
	g_bpp=fs?fsbpp:32;
  g_windowed_dsize=dbl;

  g_overlay_init_ok=0;
  if(!(cfg_bkgnd_render&1)) DD_RestoreBkgndSettings();
  if((cfg_bkgnd_render&1) || (g_fs && fsovl)) {
    // init overlay stuff
    DDSURFACEDESC   ddsdOverlay;
    HRESULT         ddrval;
    int             i;

    // It's currently not possible to query for pixel formats supported by the
    // overlay hardware (though GetFourCCCodes() usually provides a partial 
    // list).  Instead you need to call CreateSurface() to try a variety of  
    // formats till one works.  
    INIT_DIRECTDRAW_STRUCT(ddsdOverlay);
    ddsdOverlay.ddsCaps.dwCaps=DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
    ddsdOverlay.dwFlags= DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_PITCH;
    ddsdOverlay.dwWidth=resize_w;
    ddsdOverlay.dwHeight=resize_h;
    ddsdOverlay.lPitch=resize_w*sizeof(int);
    ddsdOverlay.dwBackBufferCount=0;

    // Try to create an overlay surface using one of the pixel formats in our
    // global list.
    i=0;
    do 
    {
      ddsdOverlay.ddpfPixelFormat=g_ddpfOverlayFormats[i];
      // Try to create the overlay surface
      ddrval = g_lpDD->CreateSurface(&ddsdOverlay, &g_lpddsOverlay, NULL);
    } while( FAILED(ddrval) && (++i < NUM_OVERLAY_FORMATS) );

    if(!FAILED(ddrval))
    {
      g_overlay_fourcc=(fourcc_enum)i;

      HRESULT ddrval;
	    DDSURFACEDESC   ddsd;
      INIT_DIRECTDRAW_STRUCT(ddsd);
      ddsd.dwFlags = DDSD_CAPS;
      ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
      ddrval = g_lpDD->CreateSurface(&ddsd, &g_lpddsPrimary, NULL );

      RECT            rs, rd;
      DDOVERLAYFX     ovfx;
      DDCAPS          capsDrv;
      unsigned int    uDestSizeAlign, uSrcSizeAlign;
      DWORD           dwUpdateFlags;

      INIT_DIRECTDRAW_STRUCT(capsDrv);
      ddrval = g_lpDD->GetCaps(&capsDrv, NULL);

      uDestSizeAlign = capsDrv.dwAlignSizeDest;
      uSrcSizeAlign =  capsDrv.dwAlignSizeSrc;

      dwUpdateFlags = DDOVER_SHOW | DDOVER_DDFX | DDOVER_KEYDESTOVERRIDE;

      DEVMODE d;
      d.dmSize=sizeof(d);
      d.dmDriverExtra=0;
      EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &d);

      int rv=0xff&(cfg_bkgnd_render_color>>16), gv=0xff&(cfg_bkgnd_render_color>>8), bv=0xff&cfg_bkgnd_render_color;

      if(!fsovl)
      {
        if (cfg_bkgnd_render&2)
        {
          if (!g_saved_desktop_values)
          {
            HKEY key;
            g_saved_desktop_wallpaper[0]=0;
            g_saved_reg_bkgnd_color[0]=0;
  
            // get wallpaper
            if(RegOpenKey(HKEY_CURRENT_USER, "Control Panel\\Desktop",&key)==ERROR_SUCCESS)
            {
              unsigned long s=sizeof(g_saved_desktop_wallpaper),vt;
              RegQueryValueEx(key,"Wallpaper", 0, &vt, (unsigned char *)g_saved_desktop_wallpaper, &s);
            }
  
            // get registry bkgnd color
            if(RegOpenKey(HKEY_CURRENT_USER, "Control Panel\\Colors",&key)==ERROR_SUCCESS)
            {
              unsigned long s=sizeof(g_saved_reg_bkgnd_color),vt;
              RegQueryValueEx(key,"Background", 0, &vt, (unsigned char *)g_saved_reg_bkgnd_color, &s);
            }
          
            g_saved_bkgnd_color=GetSysColor(COLOR_DESKTOP);
            g_saved_desktop_values=1;
          }
          int e=COLOR_DESKTOP;
          unsigned long c=RGB(rv,gv,bv);
          SetSysColors(1, &e, &c);
          SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, "", NULL);
  
          // rewrite registry settings right now so we don't fuck the user desktop if avs crashes
          {
            HKEY key;
            if(RegOpenKey(HKEY_CURRENT_USER, "Control Panel\\Desktop",&key)==ERROR_SUCCESS)
              RegSetValueEx(key,"Wallpaper", 0, REG_SZ, (unsigned char *)g_saved_desktop_wallpaper, strlen(g_saved_desktop_wallpaper)+1);
            if(RegOpenKey(HKEY_CURRENT_USER, "Control Panel\\Colors",&key)==ERROR_SUCCESS)
              RegSetValueEx(key,"Background", 0, REG_SZ, (unsigned char *)g_saved_reg_bkgnd_color, strlen(g_saved_reg_bkgnd_color)+1);
          }
        }
        else DD_RestoreBkgndSettings();
      }

      INIT_DIRECTDRAW_STRUCT(ovfx);
      switch(d.dmBitsPerPel)
      {
      case 16:
        ovfx.dckDestColorkey.dwColorSpaceLowValue=((rv>>3) << 11) | ((gv>>2) << 5) | (bv>>3);
        break;
      case 15:
        ovfx.dckDestColorkey.dwColorSpaceLowValue=((rv>>3) << 10) | ((gv>>3) << 5) | (bv>>3);
        break;
      case 24: case 32:
        ovfx.dckDestColorkey.dwColorSpaceLowValue=(rv << 16) | (gv << 8) | bv;
        break;
      }
      ovfx.dckDestColorkey.dwColorSpaceHighValue=ovfx.dckDestColorkey.dwColorSpaceLowValue;

      rs.left=0; rs.top=0;
      rs.right = resize_w;
      rs.bottom = resize_h;
      if (capsDrv.dwCaps & DDCAPS_ALIGNSIZESRC && uSrcSizeAlign)
          rs.right -= rs.right % uSrcSizeAlign;
      rd.left=0; rd.top=0; 
      rd.right=GetSystemMetrics(SM_CXSCREEN); rd.bottom=GetSystemMetrics(SM_CYSCREEN);
      if (capsDrv.dwCaps & DDCAPS_ALIGNSIZEDEST && uDestSizeAlign)
          rd.right = (int)((rd.right+uDestSizeAlign-1)/uDestSizeAlign)*uDestSizeAlign;
      // Make the call to UpdateOverlay() which actually displays the overlay on
      // the screen.
      ddrval = g_lpddsOverlay->UpdateOverlay(&rs, g_lpddsPrimary, &rd, dwUpdateFlags, &ovfx);
      if(!FAILED(ddrval)) g_overlay_init_ok=1;
    }
  }

  if(g_fs && fsovl) 
  {
    g_fs_height=fsh>>dbl;
    DD_CreateFullscreenOverlayWindow();
  }

	LeaveCriticalSection(&g_cs);
}

#ifndef NO_MMX
bool CopyRGBSurfaceToYUVSurfaceMMX(
        LPDDSURFACEDESC pddsd1,
        LPDDSURFACEDESC pddsd2,
		fourcc_enum eOverlayFormat)
{
    if (pddsd1->dwWidth != pddsd2->dwWidth) 
        return false;
    if (pddsd1->dwHeight != pddsd2->dwHeight) 
        return false;
    
    DWORD  w = pddsd1->dwWidth;
    DWORD  h = pddsd1->dwHeight;
    LONG   pitch1 = pddsd1->lPitch;
    LONG   pitch2 = pddsd2->lPitch;
    unsigned __int32 *pPixels1 = (unsigned __int32 *)pddsd1->lpSurface;
    unsigned __int32 *pPixels2 = (unsigned __int32 *)pddsd2->lpSurface;
	signed __int16 cm1[4];
	signed __int16 cm2[4];
	signed __int16 cm3[4];
	signed __int16 cm4[4];
	int loops_per_scanline = w/2;
	int extra_bytes_per_scanline_src  = pitch1 - w*4;
	int extra_bytes_per_scanline_dest = pitch2 - w*2;

    if (eOverlayFormat == UYVY) // U Y V Y
    {
		// swap 0<->1, and 2<->3
		cm1[1] = 77/2;  cm1[0] = -38/2;   cm1[3] = 77/2;  cm1[2] = 110/2;
		cm2[1] = 150/2; cm2[0] = -74/2;   cm2[3] = 150/2; cm2[2] = -92/2;
		cm3[1] = 29/2;  cm3[0] = 112/2;   cm3[3] = 29/2;  cm3[2] = -18/2;
		cm4[1] = 0;     cm4[0] = 32768/2; cm4[3] = 0;     cm4[2] = 32768/2;
    }
    else    // Y U Y 2
    {
		// (laptop)
		cm1[0] = 77/2;  cm1[1] = -38/2;   cm1[2] = 77/2;  cm1[3] = 110/2;
		cm2[0] = 150/2; cm2[1] = -74/2;   cm2[2] = 150/2; cm2[3] = -92/2;
		cm3[0] = 29/2;  cm3[1] = 112/2;   cm3[2] = 29/2;  cm3[3] = -18/2;
		cm4[0] = 0;     cm4[1] = 32768/2; cm4[2] = 0;     cm4[3] = 32768/2;
    }
    
	__asm
	{
		mov   edx, h

		mov   esi, pPixels1
		mov   edi, pPixels2
		sub   edi, 4			// pre-subtract
		movq  mm4, cm4
		movq  mm5, cm1
		movq  mm6, cm2
		movq  mm7, cm3

		ALIGN 8
		yuvscanlineloop:

			mov   ecx, loops_per_scanline

			ALIGN 8
			yuvloop:

                /*
                // prefetch
			    test     ecx, 0x000001ff
			    jnz      PROCESS_PIXEL_MMX32           // every 256th pixel do some prefetches

						 mov      ebx, 2*256                 // need to prefetch 256*6 bytes
                        ALIGN 8
					    LOAD_ESI_ARRAY_MMX32:
						 mov      eax, [ebx+esi]
						 mov      eax, [ebx+esi+32]
						 sub      ebx, 64
						 jnz      LOAD_ESI_ARRAY_MMX32

                ALIGN 8
				PROCESS_PIXEL_MMX32:
                */

				// read in 2 pixels
				movq mm0, qword ptr [esi]	// -- b1 g1 r1 -- b2 g2 r2
				movq mm1, qword ptr [esi]	// -- b1 g1 r1 -- b2 g2 r2
				movq mm2, qword ptr [esi]	// -- b1 g1 r1 -- b2 g2 r2

                // quick reference:
                // punpcklbw mm7, mm7     // abcdefgh -> eeffgghh
                // punpcklbw mm7, mm0     // abcdefgh -> 0e0f0g0h (if mm0 is zero)
				// packuswb mm7, mm7      // 0a0b0g0r -> abgrabgr ?

				// step 1: get to this state:
				//	mm0: r1   r1   r2   r2
				//	mm1: g1   g1   g2   g2
				//	mm2: b1   b1   b2   b2
				//  mm3: junk
				//  mm4: 0    32k  0    32k
				//  mm5: c1r1 c1r2 c2r1 c2r2
				//  mm6: c1g1 c1g2 c2g1 c2g2
				//  mm7: c1b1 c1b2 c2b1 c2b2

				// NOTE: the shifts of 8, 16, and 24 below are
				//       correct (vs. 0-8-16) but might be in
				//       backwards order!

				pslld mm0, 8
				pslld mm1, 16
				pslld mm2, 24
				psrld mm0, 24    // 00 00 00 r1 00 00 00 r2
				psrld mm1, 24    // 00 00 00 g1 00 00 00 g2
				psrld mm2, 24    // 00 00 00 b1 00 00 00 b2
				
                movq  mm3, mm0
				pslld mm0, 16
				por   mm0, mm3   // 00 r1 00 r1 00 r2 00 r2

				movq  mm3, mm1
				pslld mm1, 16
				por   mm1, mm3   // 00 g1 00 g1 00 g2 00 g2

				movq  mm3, mm2
				pslld mm2, 16
				por   mm2, mm3   // 00 b1 00 b1 00 b2 00 b2

				// step 2: multiply to get to this state:
				//	mm0: r1*c1r1  r1*c1r2  r2*c2r1  r2*c2r2
				//	mm1: g1*c1g1  g1*c1g2  g2*c2g1  g2*c2g2
				//	mm2: b1*c1b1  b1*c1b2  b2*c2b1  b2*c2b2
				//  mm4: 0    32k  0    32k
				pmullw mm0, mm5
				 add  edi, 4
				pmullw mm1, mm6
				 add  esi, 8
					pmullw mm2, mm7
				
				// step 3: add to get to this state:
				//	mm0: d1*256   d2*256   d3*256   d4*256
				paddsw mm0, mm4
				paddsw mm0, mm1
				paddsw mm0, mm2

				psrlw    mm0, 7
				packuswb mm0, mm0    // bytes: abgrabgr
				movd     dword ptr [edi], mm0  // store 
				
				loop yuvloop

			// scanline complete
			add esi, extra_bytes_per_scanline_src
			add edi, extra_bytes_per_scanline_dest

			dec edx
			jnz yuvscanlineloop

		emms
	}
   
    return true;
}
#else
bool CopyRGBSurfaceToYUVSurface(
        LPDDSURFACEDESC pddsd1,
        LPDDSURFACEDESC pddsd2,
		fourcc_enum eOverlayFormat)
{
  if (pddsd1->dwWidth != pddsd2->dwWidth) 
    return false;
  if (pddsd1->dwHeight != pddsd2->dwHeight) 
    return false;
    
  DWORD  w = pddsd1->dwWidth;
  DWORD  h = pddsd1->dwHeight;
  LONG   pitch1 = pddsd1->lPitch;
  LONG   pitch2 = pddsd2->lPitch;
  unsigned __int32 *pPixels1 = (unsigned __int32 *)pddsd1->lpSurface;
  unsigned __int32 *pPixels2 = (unsigned __int32 *)pddsd2->lpSurface;
  unsigned __int32 color1;
  LONG   offset1 = 0;
  LONG   offset2 = 0;
  unsigned int    R, G, B, i1, i2, i3, i4;
  BYTE            yuv[4];

  if (eOverlayFormat == UYVY) // U Y V Y
  {
    i1 = 1;
    i2 = 0;
    i3 = 3;
    i4 = 2;
  }
  else    // Y U Y 2
  {
    i1 = 0;
    i2 = 1;
    i3 = 2;
    i4 = 3;
  }
    
  // Go through the image 2 pixels at a time and convert to YUV
  for (unsigned int y=0; y<h; y++)
  {
    offset1 = y*pitch1/4;
    offset2 = y*pitch2/4;

	  for (unsigned int x=0; x<w; x+=2)
    {
      color1 = pPixels1[offset1++];
      B = (color1) & 0xFF;
      G = (color1 >> 8) & 0xFF;
      R = (color1 >> 16) & 0xFF;
      yuv[i1] = (77*R + 150*G + 29*B) >> 8;
      yuv[i2] = (32768 - 38*R - 74*G + 112*B) >> 8;
            
      color1 = pPixels1[offset1++];
      B = (color1) & 0xFF;
      G = (color1 >> 8) & 0xFF;
      R = (color1 >> 16) & 0xFF;
      yuv[i3] = (77*R + 150*G + 29*B) >> 8;
      yuv[i4] = (32768 + 110*R - 92*G - 18*B) >> 8;

      pPixels2[offset2++] = *((unsigned __int32 *)yuv);
    }
  }    
  return true;
}
#endif

int DDraw_Init()
{
	InitializeCriticalSection(&g_cs);
	return 0;

}
void DDraw_Quit(void)
{
  if (g_lpDD)
  {
		if (g_lpPrimSurf)
		{
			g_lpPrimSurf->Release();
			g_lpPrimSurfBack=g_lpPrimSurf=NULL;
		}
    if (g_lpRenderSurf[0]) g_lpRenderSurf[0]->Release();
    if (g_lpRenderSurf[1]) g_lpRenderSurf[1]->Release();
    g_lpRenderSurf[0]=0;
    g_lpRenderSurf[1]=0;
    if (g_lpddsOverlay) g_lpddsOverlay->Release();
    if (g_lpddsPrimary) g_lpddsPrimary->Release();
    g_lpddsOverlay=g_lpddsPrimary=NULL;
    g_lpDD->Release();
    g_lpDD=NULL;
    DD_RestoreBkgndSettings();
  }
	DeleteCriticalSection(&g_cs);
}


void DDraw_BeginResize(void)
{
  if (!g_fs) nodraw=1;
}
void DDraw_Resize(int w, int h, int dsize)
{
  if (!g_fs) DD_CreateSurfaces(w, h, 0, 0, 0, 0, !!dsize, 0);
}

static void *g_lpsurf[2];

static DDSURFACEDESC d={sizeof(d),};
static DDSURFACEDESC d2={sizeof(d),};

void DDraw_Enter(int *w, int *h, int **fb1, int **fb2)
{
	HRESULT han;
  if (nodraw) 
  {
    *fb1=*fb2=0;
    return;
  }
  EnterCriticalSection(&g_cs);
	*w=g_w;
	*h=g_h;
  if (!g_lpDD || !g_lpRenderSurf[0] || !g_lpRenderSurf[1]) 
  {
    *fb1=*fb2=0;
    LeaveCriticalSection(&g_cs);
    return;
  }
	if ((han = g_lpRenderSurf[0]->Lock(NULL,&d,DDLOCK_WAIT,NULL)) != DD_OK) 
	{
    *fb1=*fb2=0;
    LeaveCriticalSection(&g_cs);
		return;
	}
	if ((han = g_lpRenderSurf[1]->Lock(NULL,&d2,DDLOCK_WAIT,NULL)) != DD_OK) 
	{
    g_lpRenderSurf[0]->Unlock(d.lpSurface);
    *fb1=*fb2=0;
    LeaveCriticalSection(&g_cs);
		return;
	}
	*fb1=(int*)(g_lpsurf[0]=d.lpSurface);
	*fb2=(int*)(g_lpsurf[1]=d2.lpSurface);

  if (g_fs && g_fs_height!=g_h)// && !cfg_fs_use_overlay)
  {
    int y=g_h/2-((g_fs_height/2));
    *h-=y*2;
    *fb1 += y*g_w;
    *fb2 += y*g_w;
  }
}
      
static unsigned int draw_title_time;
static char last_title[1024];

extern HWND hwnd_WinampParent;

static void do_gettitle()
{
  if (draw_title_p < 1 && --draw_title_p < -7)
  {
    char this_title[2048]={0,};
    char *p;
	  if (IsWindow(hwnd_WinampParent)) 
    {
      DWORD id;
      if (!SendMessageTimeout( hwnd_WinampParent,WM_GETTEXT,(WPARAM)sizeof(this_title),(LPARAM)this_title,SMTO_BLOCK,50,&id) || !id) return;
    }
	  p = this_title+strlen(this_title);
	  while (p >= this_title)
	  {
		  char buf[9];
		  memcpy(buf,p,8);
		  buf[8]=0;
		  if (!lstrcmpi(buf,"- Winamp")) break;
		  p--;
	  }
	  if (p >= this_title) p--;
	  while (p >= this_title && *p == ' ') p--;
	  *++p=0;
	  if (lstrcmpi(this_title,last_title))
	  {
		  strcpy(last_title,this_title);
      draw_title_p=1;
      draw_title_time=GetTickCount()+1000;
	  }
    else draw_title_p=0;
  }
  if (draw_title_p == 2)
  {
    draw_title_p=1;
    draw_title_time=GetTickCount()+1000;
  }
}


char statustext[256];
DWORD statustext_life;
int statustext_len;

void DDraw_SetStatusText(char *text, int life)
{
  strcpy(statustext,text);
  statustext_len=life?life:2000;
  statustext_life=1;

}



#ifndef NO_X86ASM
void homemadeBlitFrom32bpp(DDSURFACEDESC *out, void *in, int w, int h, int sy, int ey)
{
  int mh=min(h,(int)out->dwHeight);
  if (sy < 0) sy=0;
  if (ey > mh) ey=mh;
    if (out->ddpfPixelFormat.dwRGBBitCount == 15)
  {
    int y;
    int mw;
    mw=min(w,out->lPitch/2);
    unsigned int *inptr=(unsigned int *)in + w*sy;
    for (y = sy; y < ey; y ++)
    {
      unsigned char *optr = (unsigned char *)out->lpSurface + (y * out->lPitch);
      __asm
      {
        mov edi, optr
        mov esi, inptr
        mov ecx, mw
        mov ebx, 00000000111110001111100011111000b
        shr ecx, 1
      conv15_loop:
        mov eax,[esi]

        mov edx,[esi+4]

        and eax, ebx
        and edx, ebx

                  // 00000000RRRRR000GGGGG000BBBBB000
        shr ah,3  // 00000000RRRRR000000GGGGGBBBBB000
        shr dh,3

        shr ax,3  // 00000000RRRRR000000000GGGGGBBBBB
        shr dx,3

                  // ----------------HHHHHHHHLLLLLLLL
        ror eax,10// GGGGGBBBBB00000000RRRRR000000000
        ror edx,10

        shr ah,1
        shr dh,1

        mov al,ah
        mov dl,dh

        rol eax,10

        rol edx,10

        shl edx, 16

        and eax, 0x0000ffff
        add esi,8

        or eax, edx

        mov [edi], eax

        add edi, 4
        dec ecx
        jnz conv15_loop  
      }
      inptr += w;
    }

  }
  else if (out->ddpfPixelFormat.dwRGBBitCount == 16)
  {
    int y;
    int mw;
    mw=min(w,out->lPitch/2);
    unsigned int *inptr=(unsigned int *)in + w*sy;
    for (y = sy; y < ey; y ++)
    {
      unsigned char *optr = (unsigned char *)out->lpSurface + (y * out->lPitch);
      __asm
      {
        mov edi, optr
        mov esi, inptr
        mov ecx, mw
        mov ebx, 00000000111110001111110011111000b
        shr ecx, 1
      conv16_loop:
        mov eax,[esi]

        mov edx,[esi+4]

        and eax, ebx
        and edx, ebx

        // RRRRR000GGGGGG00BBBBB000

        shr ah,2        // RRRRR00000GGGGGGBBBBB000
        shr dh,2

        shr ax,3        // RRRRR00000000GGGGGGBBBBB
        shr dx,3

        ror eax,8       // GGGBBBBBRRRRR00000000GGG
        ror edx,8

        add al,ah       // GGGBBBBBRRRRR000RRRRRGGG
        add dl,dh

        rol eax,8       // RRRRR000RRRRRGGGGGGBBBBB

        rol edx,8

        shl edx, 16

        and eax, 0x0000ffff
        add esi,8

        or eax, edx

        mov [edi], eax

        add edi, 4
        dec ecx
        jnz conv16_loop  
      }
      inptr += w;
    }

  }
  else if (out->ddpfPixelFormat.dwRGBBitCount == 24)
  { // very unoptimized
    int y,x;
    int mw;
    unsigned int *inptr=(unsigned int *)in+sy;
    mw=min(w,out->lPitch/3);
    for (y = sy; y < ey; y ++)
    {
      unsigned char *optr = (unsigned char *)out->lpSurface + (y * out->lPitch);
      x=mw/2;
      while (x--)
      {
        int a=inptr[0];
        int b=inptr[1];
        optr[0]=(unsigned char) a;
        optr[3]=(unsigned char) b;
        optr[1]=(unsigned char) (a>>8);
        optr[4]=(unsigned char) (b>>8);
        optr[2]=(unsigned char) (a>>16);
        optr[5]=(unsigned char) (b>>16);

        optr+=6;
        inptr+=2;
      }
      inptr += (w-mw);
    }
  }
  else
  {
    unsigned char *outptr;
    unsigned int *inptr;
    int mw=min(w*4,out->lPitch);
    int y;
    inptr=(unsigned int*)in + (sy*w);
    outptr=(unsigned char *)out->lpSurface + (sy*out->lPitch);
    for (y = sy; y < ey; y ++)
    {
      memcpy(outptr,inptr,mw);
      inptr+=w;
      outptr+=out->lPitch;
    }
  }
}
#endif

static int unlocksurfaces()
{
  int a=0;
  if (g_lpRenderSurf[0] && g_lpRenderSurf[0]->Unlock(g_lpsurf[0]) != DD_OK) a=1;
  if (g_lpRenderSurf[1] && g_lpRenderSurf[1]->Unlock(g_lpsurf[1]) != DD_OK) a=1;
  return a;
}

void DDraw_Exit(int which)
{
  if (!g_lpRenderSurf[0] || !g_lpRenderSurf[1]) 
  {
    unlocksurfaces();
    goto endfunc;
  }
#ifdef RESIZE_ONRESIZE
  last_used=which;
#endif

	if (g_fs && !cfg_fs_use_overlay)
	{
    int fsy=g_h/2-((g_fs_height/2));
		RECT r={0,0,g_w,g_h};
    if (g_bpp != 32) // non-32bpp mode
    {      
      if (fsy>0 && ((cfg_fs_fps&1) || (statustext_life&&!(cfg_fs_fps&8)))) // clear portions of backbuffer if necessary
      {
        DDBLTFX ddbfx={sizeof(ddbfx),};
        int ty=min(fsy,16);
        RECT r2={0,0,g_w,ty};
        g_lpPrimSurfBack->Blt(&r2,NULL,NULL,DDBLT_WAIT|DDBLT_COLORFILL,&ddbfx);
        r2.top=g_h-ty;
        r2.bottom=g_h-1;
        g_lpPrimSurfBack->Blt(&r2,NULL,NULL,DDBLT_WAIT|DDBLT_COLORFILL,&ddbfx);
      }
#ifndef NO_X86ASM
		  if (!(g_fs_flip&8) && !g_windowed_dsize) // homemade bltshit
      {
	      DDSURFACEDESC d={sizeof(d),};
        HRESULT han;
	      han = g_lpPrimSurfBack->Lock(NULL,&d,DDLOCK_WAIT,NULL);
        if (han == DDERR_SURFACELOST)
        {
          DDBLTFX ddbfx={sizeof(ddbfx),};
          g_lpPrimSurfBack->Restore();
          g_lpPrimSurfBack->Blt(NULL,NULL,NULL,DDBLT_WAIT|DDBLT_COLORFILL,&ddbfx);
          han = g_lpPrimSurfBack->Lock(NULL,&d,DDLOCK_WAIT,NULL);
        }
        if (han != DD_OK) goto slow_fs_non32bpp;

        homemadeBlitFrom32bpp(&d,g_lpsurf[which],g_w,g_h,fsy,g_h-fsy);

        int a=0;
        if (g_lpPrimSurfBack->Unlock(d.lpSurface) != DD_OK ) a=1;
	      if (unlocksurfaces()||a)
	      {
          goto endfunc;
	      }
      }
      else // slow (stretchblt)
#endif
      {
		    HDC in, out;
#ifndef NO_X86ASM
slow_fs_non32bpp:
#endif
        if (unlocksurfaces() || g_lpPrimSurfBack->GetDC(&out) != DD_OK)
        {
          goto endfunc;
        }
		    if (g_lpRenderSurf[which]->GetDC(&in) != DD_OK)
		    {
          g_lpPrimSurfBack->ReleaseDC(out);
          goto endfunc;
		    }
        if (g_windowed_dsize) StretchBlt(out,0,fsy*2,g_dsw,g_dsh-fsy*4,in,0,fsy,g_w,g_h-fsy*2,SRCCOPY);
        else BitBlt(out,0,fsy,g_w,g_h-fsy*2,in,0,fsy,SRCCOPY);
		    g_lpRenderSurf[which]->ReleaseDC(in);
        g_lpPrimSurfBack->ReleaseDC(out);        
      }
    }
    else // 32bpp - always just use Blt() - will scale if necessary (yay Blt!)
    {
      //RECT or={0,g_go_fs_h/2-g_go_fs_height/2,g_go_fs_w,g_go_fs_h/2+g_go_fs_height/2};
      //RECT ir={0,g_h/2-g_go_fs_height/2,g_w,g_h/2+g_go_fs_height/2};
      if (unlocksurfaces())
      {
        goto endfunc;
	    }
		  if (g_lpPrimSurfBack->Blt(NULL,g_lpRenderSurf[which],&r,DDBLT_WAIT,NULL) == DDERR_SURFACELOST)
		  {
        DDBLTFX ddbfx={sizeof(ddbfx),};
			  if (g_fs_flip&1)
        {
          g_lpPrimSurfBack->Restore();
          g_lpPrimSurfBack->Blt(NULL,NULL,NULL,DDBLT_WAIT|DDBLT_COLORFILL,&ddbfx);
        }
			  g_lpPrimSurf->Restore();
        g_lpPrimSurf->Blt(NULL,NULL,NULL,DDBLT_WAIT|DDBLT_COLORFILL,&ddbfx);
        goto endfunc;
		  }
    }
    do_gettitle();
    if ((cfg_fs_fps&1) || (draw_title_p>0&&!(cfg_fs_fps&16)) || (statustext_life&&!(cfg_fs_fps&8)))
    {
		  HDC out;
      if (g_lpPrimSurfBack->GetDC(&out) == DD_OK)
      {
        char str[2048];
        SetBkMode(out,TRANSPARENT);
        if (cfg_fs_fps&1) 
        {
          RECT r={2,2,g_fs_w,g_fs_h};
          wsprintf(str,"%d.%d",g_dlg_fps/10,g_dlg_fps%10);
			    SetTextColor(out,RGB(0,0,0));
          DrawText(out,str,-1,&r,DT_TOP|DT_LEFT|DT_SINGLELINE|DT_NOCLIP|DT_NOPREFIX);
			    r.left=r.top=0;
			    SetTextColor(out,RGB(255,255,255));
          DrawText(out,str,-1,&r,DT_TOP|DT_LEFT|DT_SINGLELINE|DT_NOCLIP|DT_NOPREFIX);
        }
        if (statustext_life&&!(cfg_fs_fps&8))
        {
          if (statustext_life==1) statustext_life=GetTickCount()+statustext_len;
          RECT r={0,0,g_fs_w,g_fs_h};
          if (GetTickCount() > statustext_life) 
          {
            if (statustext_life==2) statustext_life=0;
            else statustext_life=2;
          }
          else
          {
            SetTextColor(out,RGB(0,0,0));
            DrawText(out,statustext,-1,&r,DT_BOTTOM|DT_RIGHT|DT_SINGLELINE|DT_NOPREFIX);
            SetTextColor(out,RGB(255,255,255));
            r.right-=2;
            r.bottom-=2;
            DrawText(out,statustext,-1,&r,DT_BOTTOM|DT_RIGHT|DT_SINGLELINE|DT_NOPREFIX);
          }
        }
        if (draw_title_p>0&&!(cfg_fs_fps&16)) 
        { 
          RECT r={4,4,g_fs_w,g_fs_h};
			    SetTextColor(out,RGB(0,0,0));
          DrawText(out,last_title,-1,&r,DT_VCENTER|DT_CENTER|DT_SINGLELINE|DT_NOPREFIX);
			    r.left=r.top=0;
			    SetTextColor(out,RGB(255,255,255));
          DrawText(out,last_title,-1,&r,DT_VCENTER|DT_CENTER|DT_SINGLELINE|DT_NOPREFIX);
        }
        g_lpPrimSurfBack->ReleaseDC(out);
      }
      if (draw_title_p>0 && draw_title_time < GetTickCount() && !(cfg_fs_fps&16))
      {
        if (g_lpRenderSurf[which]->GetDC(&out) == DD_OK)
        {
          RECT r={4,4,g_w,g_h};
			    SetBkMode(out,TRANSPARENT);
			    SetTextColor(out,RGB(0,0,0));
          DrawText(out,last_title,-1,&r,DT_VCENTER|DT_CENTER|DT_SINGLELINE|DT_NOPREFIX);
			    r.left=r.top=0;
			    SetTextColor(out,RGB(255,255,255));
          DrawText(out,last_title,-1,&r,DT_VCENTER|DT_CENTER|DT_SINGLELINE|DT_NOPREFIX);
          g_lpRenderSurf[which]->ReleaseDC(out);
        }
        draw_title_p=0;
      }    
    }
		if (g_fs_flip&1) g_lpPrimSurf->Flip(NULL, DDFLIP_WAIT);
    else if (!(g_fs_flip&2)) g_lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0);
	}
	else
	{
    if(g_overlay_init_ok)
    {
      LPDDSURFACEDESC pd=(which==0?&d:&d2);
	    DDSURFACEDESC dd={sizeof(dd),};
      if (g_fs) if (!(cfg_fs_flip&2)) g_lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0);

	    if (g_lpddsOverlay->Lock(NULL,&dd,DDLOCK_WAIT,NULL) != DD_OK) 
      {
        g_overlay_init_ok=0;
        goto endfunc;
      }
#ifndef NO_MMX
        CopyRGBSurfaceToYUVSurfaceMMX(pd,&dd,g_overlay_fourcc);
#else
        CopyRGBSurfaceToYUVSurface(pd,&dd,g_overlay_fourcc);
#endif
      g_lpddsOverlay->Unlock(&dd);
      if (g_fs) 
      {
        unlocksurfaces();
        goto endfunc;
      }
    }

		HDC in1, out, in2=NULL;
	  if (unlocksurfaces() || g_lpRenderSurf[which]->GetDC(&in1) != DD_OK)
	  {
      goto endfunc;
	  }
    do_gettitle();
    if (draw_title_p>0&&!(cfg_fs_fps&4) && draw_title_time < GetTickCount())
    {
      RECT r={4,4,g_w,g_h};
			SetBkMode(in1,TRANSPARENT);
			SetTextColor(in1,RGB(0,0,0));
      DrawText(in1,last_title,-1,&r,DT_VCENTER|DT_CENTER|DT_SINGLELINE|DT_NOPREFIX);
			r.left=r.top=0;
			SetTextColor(in1,RGB(255,255,255));
      DrawText(in1,last_title,-1,&r,DT_VCENTER|DT_CENTER|DT_SINGLELINE|DT_NOPREFIX);
      draw_title_p=0;
    }
    if ((draw_title_p > 0&&!(cfg_fs_fps&4)) || (statustext_life&&!(cfg_fs_fps&2)))
    {
      // draw perframe
      RECT r={4,4,g_w,g_h};
		  if (g_lpRenderSurf[which^1]->GetDC(&in2) != DD_OK)
		  {
        in2=NULL; 
        goto abort_thingy;
		  }
      BitBlt(in2,0,0,g_w,g_h,in1,0,0,SRCCOPY);
      SetBkMode(in2,TRANSPARENT);
      if (draw_title_p > 0&&!(cfg_fs_fps&4))
      {
			  SetTextColor(in2,RGB(0,0,0));
        DrawText(in2,last_title,-1,&r,DT_VCENTER|DT_CENTER|DT_SINGLELINE|DT_NOPREFIX);
			  r.left=r.top=0;
			  SetTextColor(in2,RGB(255,255,255));
        DrawText(in2,last_title,-1,&r,DT_VCENTER|DT_CENTER|DT_SINGLELINE|DT_NOPREFIX);
      }
      if (statustext_life&&!(cfg_fs_fps&2))
      {
        if (statustext_life==1) statustext_life=GetTickCount()+statustext_len;
        if (GetTickCount() > statustext_life) statustext_life=0;
			  SetTextColor(in2,RGB(0,0,0));
        DrawText(in2,statustext,-1,&r,DT_BOTTOM|DT_RIGHT|DT_SINGLELINE|DT_NOPREFIX);
        r.right-=2;
        r.bottom-=2;
			  SetTextColor(in2,RGB(255,255,255));
        DrawText(in2,statustext,-1,&r,DT_BOTTOM|DT_RIGHT|DT_SINGLELINE|DT_NOPREFIX);
      }
    }
abort_thingy:
    if (!(cfg_fs_flip&4)) g_lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0);
		out=GetDC(g_hwnd);
    if (out)
    {
      if (!g_windowed_dsize)
      {
#ifndef WA2_EMBED
        BitBlt(out,inWharf?0:7,inWharf?0:15,g_w-g_noshoww,g_h,in2?in2:in1,0,0,SRCCOPY);
#else
        BitBlt(out,0,0,g_w-g_noshoww,g_h,in2?in2:in1,0,0,SRCCOPY);
#endif
      }
      else
      {
#ifndef WA2_EMBED
        StretchBlt(out,inWharf?0:7,inWharf?0:15,g_dsw,g_dsh,in2?in2:in1,0,0,g_w,g_h,SRCCOPY);
#else
        StretchBlt(out,0,0,g_dsw,g_dsh,in2?in2:in1,0,0,g_w,g_h,SRCCOPY);
#endif
      }
  		ReleaseDC(g_hwnd,out);
    }
		g_lpRenderSurf[which]->ReleaseDC(in1);
    if (in2)
      g_lpRenderSurf[which^1]->ReleaseDC(in2);
	}
endfunc:
  LeaveCriticalSection(&g_cs);
}


void DDraw_SetFullScreen(int fs, int w, int h, int dbl, int bpp)
{
  if (!fs) DD_CreateSurfaces(w, h, 0, 0, 0, 0, !!dbl, 0);
  else
  {
    DD_CreateSurfaces(w, h, (min(max(1,cfg_fs_height),100)*h)/100, 1, bpp, cfg_fs_flip, !!dbl, cfg_fs_use_overlay);
  }
}

int DDraw_IsFullScreen(void)
{
	return g_fs;
}

HRESULT WINAPI _cb(  
				   LPDDSURFACEDESC lpDDSurfaceDesc,  LPVOID lpContext)
{
	HWND h=(HWND)lpContext;
	if ((lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_RGB) &&
		(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount==32 ||
		lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount==24 ||
    lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount==16 ||
    lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount==15))
	{
		char s[256];
    wsprintf(s,"%dx%d@%dBPP",lpDDSurfaceDesc->dwWidth,lpDDSurfaceDesc->dwHeight,lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);
		SendMessage(h,CB_ADDSTRING,0,(long)s);
	}

	return DDENUMRET_OK;
}


void DDraw_EnumDispModes(HWND hwnd)
{
	if (g_lpDD)
  	g_lpDD->EnumDisplayModes(0,NULL,hwnd,_cb);
}

static int g_st;

HRESULT WINAPI _cb2(  
				   LPDDSURFACEDESC lpDDSurfaceDesc,  LPVOID lpContext)
{
	DWORD *x=(DWORD *)lpContext;
  if (x[0]==lpDDSurfaceDesc->dwWidth &&
      x[1]==lpDDSurfaceDesc->dwHeight &&
      x[2]==lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount) 
  {
    g_st=1;
    return DDENUMRET_CANCEL;
  }

	return DDENUMRET_OK;
}

int DDraw_IsMode(int w, int h, int bpp)
{
  int x[3]={w,h,bpp};
  if (!x[0] || !x[1] || !x[2]) return 0;
	if (!g_lpDD) return 0;
  g_st=0;
  g_lpDD->EnumDisplayModes(0,NULL,(LPVOID)x,_cb2);
  return g_st;
}

HRESULT WINAPI _cb3(  
				   LPDDSURFACEDESC lpDDSurfaceDesc,  LPVOID lpContext)
{
	if ((lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_RGB) &&
		(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount==32 ||
		lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount==24 ||
    lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount==16 ||
    lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount==15))
  {  
    int *(*x)=(int **)lpContext;
    if (lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount > (DWORD)x[2][0] || 
        lpDDSurfaceDesc->dwWidth < (DWORD)x[0][0] ||
        lpDDSurfaceDesc->dwHeight < (DWORD)x[1][0])
    {
      x[0][0]=lpDDSurfaceDesc->dwWidth;
      x[1][0]=lpDDSurfaceDesc->dwHeight;
      x[2][0]=lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
    }
  }

	return DDENUMRET_OK;
}


int DDraw_PickMode(int *w, int *h, int *bpp)
{
  int *(x[3])={w,h,bpp};
  *w=0;
  *h=0;
  *bpp=0;
  // return if mode found that is suitable
  g_lpDD->EnumDisplayModes(0,NULL,(LPVOID)x,_cb3);
  return *w && *h && *bpp;
}

double DDraw_translatePoint(POINT p, int isY)
{
  double v=0.0;
  if(g_fs && g_overlay_init_ok && hwndOverlayWnd && IsWindow(hwndOverlayWnd)) 
  {
    RECT r;
    ScreenToClient(hwndOverlayWnd,&p);
    GetClientRect(hwndOverlayWnd,&r);
    if (isY) 
    {
      if (r.bottom>0)
        v=p.y/(double)(r.bottom*0.5) - 1.0;
    }
    else
    {
      if (r.right>0)
        v=p.x/(double)(r.right*0.5) - 1.0;
    }
  }
  else
  {
    ScreenToClient(g_hwnd,&p);
 
    if (isY) 
    {
      if (g_dsh>0)
        v=p.y/(double)(g_dsh*0.5) - 1.0;
    }
    else
    {
      if (g_dsw>0)
        v=p.x/(double)(g_dsw*0.5) - 1.0;
    }
  }
  //if (v > 1.0) v=1.0;
  //if (v < -1.0) v=-1.0;
  return v;
}