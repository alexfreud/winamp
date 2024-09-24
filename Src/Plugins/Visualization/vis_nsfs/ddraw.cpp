#include <windows.h>
#include <ddraw.h>
#include "dd.h"
#include "../Agave/Language/api_language.h"
#include "resource.h"

extern HINSTANCE visDll;

#pragma intrinsic(memset)
C_DD::C_DD()
{
	g_lpDD=NULL;
	cursurface=0;
	g_ddp=NULL;
	g_lpDDSPrim=NULL;
	g_lpPrimSurfBack=NULL;
	g_RenderSurface[0]=g_RenderSurface[1]=NULL;
	g_RenderSurfaceLocked[0]=g_RenderSurfaceLocked[1]=NULL;
	memset(g_palette_seek,0,sizeof(g_palette_seek));
	memset(g_palette_orig,0,sizeof(g_palette_orig));
	memset(g_palette_last,0,sizeof(g_palette_last));
	g_pal_lastv=0;
	g_palette_starttime=g_palette_endtime=0;
}

int C_DD::lock(unsigned char **input, unsigned char **output)
{
	DDSURFACEDESC d={sizeof(d),};
	if (g_RenderSurface[cursurface]->Lock(NULL,&d,DDLOCK_WAIT,NULL) != DD_OK) 
	{
		g_RenderSurfaceLocked[cursurface]=0;
		return 0;
	}
	*input=(unsigned char*)(g_RenderSurfaceLocked[cursurface]=d.lpSurface);

	DDSURFACEDESC e={sizeof(e),};
	if (g_RenderSurface[cursurface^1]->Lock(NULL,&e,DDLOCK_WAIT,NULL) != DD_OK) 
	{
		g_RenderSurface[cursurface]->Unlock(g_RenderSurfaceLocked[cursurface]);
		g_RenderSurfaceLocked[cursurface^1]=0;
		g_RenderSurfaceLocked[cursurface]=0;
		return 0;
  	}
    *output=(unsigned char*)(g_RenderSurfaceLocked[cursurface^1]=e.lpSurface);
	return 1;
}

int C_DD::palette_fadeleft()
{
	return g_palette_endtime-GetTickCount();
}

void C_DD::unlock(void)
{
	if (g_RenderSurfaceLocked[0]) 
	{
		g_RenderSurface[0]->Unlock(g_RenderSurfaceLocked[0]);
		g_RenderSurface[1]->Unlock(g_RenderSurfaceLocked[1]);
		if (g_palette_starttime)
		{
			PALETTEENTRY palette[256] = {0};
			int x = 0;
			int len=g_palette_endtime-g_palette_starttime;
			int pos=GetTickCount()-g_palette_starttime;
			int do_it=1;

			if (pos >= len || !len)
			{
				g_palette_starttime=0;
				g_pal_lastv=-128;
				memcpy(g_palette_last,g_palette_seek,768);
			}
			else
			{
				int adj=(pos<<8)/len;
				int poop=(255*adj)>>8;
				if (poop != g_pal_lastv)
				{
					g_pal_lastv=poop;
					for (x = 0; x < 768; x ++)
					{
						int c=(((int)g_palette_seek[x])*adj + ((int)g_palette_orig[x])*(255-adj))>>8;
						if (c > 255) c = 255;
						g_palette_last[x]=c;
					}
				}
				else do_it=0;
			}

			if (do_it)
			{
				int y=0;
				for (x = 0; x < 256; x ++)
				{
					palette[x].peRed=g_palette_last[y];
					palette[x].peGreen=g_palette_last[y+1];
					palette[x].peBlue=g_palette_last[y+2];
					palette[x].peFlags = PC_NOCOLLAPSE;
					y+=3;
				}
				g_ddp->SetEntries(0,0,256,palette);
			}
		}
		if (g_lpPrimSurfBack)
		{
			if (g_lpPrimSurfBack->Blt(NULL,g_RenderSurface[cursurface^1],NULL,DDBLT_WAIT,NULL) == DDERR_SURFACELOST)
			{
				g_lpDDSPrim->Restore();
				g_lpPrimSurfBack->Restore();
			}
			else 
				g_lpDDSPrim->Flip(NULL,DDFLIP_WAIT);
		}
		else
		{
			if (g_lpDDSPrim->Blt(NULL,g_RenderSurface[cursurface^1],NULL,DDBLT_WAIT,NULL) == DDERR_SURFACELOST)
			{
				g_lpDDSPrim->Restore();
			}
			else g_lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0);
		}
		g_RenderSurfaceLocked[0]=0;
		g_RenderSurfaceLocked[1]=0;
		cursurface^=1;
	}
}

char *C_DD::open(int w, int h, HWND hwnd)
{
	static char errbuf[128];
	if (DirectDrawCreate(NULL,&g_lpDD,NULL) != DD_OK) return WASABI_API_LNGSTRING_BUF(IDS_DIRECTDRAWCREATE_FAILED,errbuf,128);
	if (g_lpDD->SetCooperativeLevel(hwnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN) != DD_OK)
	{
		g_lpDD->Release();
		return WASABI_API_LNGSTRING_BUF(IDS_COULD_NOT_SET_EXCLUSIVE_FULLSCREEN,errbuf,128);
	}
	if (g_lpDD->SetDisplayMode(w,h,8) != DD_OK)
	{
	    g_lpDD->Release();
		return WASABI_API_LNGSTRING_BUF(IDS_COULD_NOT_SET_DISPLAY_MODE,errbuf,128);
	}
	DDSURFACEDESC DDsd = {0};
	DDsd.dwSize = sizeof(DDsd);
	DDsd.dwFlags = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
	DDsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|DDSCAPS_COMPLEX|DDSCAPS_FLIP;
	DDsd.dwBackBufferCount = 1;
	if (g_lpDD->CreateSurface(&DDsd, &g_lpDDSPrim, NULL) != DD_OK)
	{
		g_lpPrimSurfBack=NULL;
		DDsd.dwSize = sizeof(DDsd);
		DDsd.dwFlags = DDSD_CAPS;
		DDsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		if (g_lpDD->CreateSurface(&DDsd, &g_lpDDSPrim, NULL) != DD_OK)
		{
			g_lpDD->Release();
  			return WASABI_API_LNGSTRING_BUF(IDS_COULD_NOT_CREATE_PRIMARY_SURFACE,errbuf,128);
		}
	}
	else
	{
		DDSCAPS ddscaps;
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		if (g_lpDDSPrim->GetAttachedSurface(&ddscaps, &g_lpPrimSurfBack) != DD_OK)  // no page flipping
		g_lpPrimSurfBack=NULL;
	}

	DDsd.dwFlags = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PITCH|DDSD_PIXELFORMAT;
	DDsd.dwWidth=w;
	DDsd.dwHeight=h;
	DDsd.lPitch=w;
	DDsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
	DDsd.ddpfPixelFormat.dwSize = sizeof(DDsd.ddpfPixelFormat);
	DDsd.ddpfPixelFormat.dwFlags=DDPF_PALETTEINDEXED8|DDPF_RGB;
	DDsd.ddpfPixelFormat.dwRGBBitCount = 8;

	if (g_lpDD->CreateSurface(&DDsd, &g_RenderSurface[0], NULL) != DD_OK ||
		g_lpDD->CreateSurface(&DDsd, &g_RenderSurface[1], NULL) != DD_OK)
	{
		g_lpDD->Release();
		return WASABI_API_LNGSTRING_BUF(IDS_COULD_NOT_CREATE_RENDER_SURFACE,errbuf,128);
	}

	PALETTEENTRY palette[256] = {0};
	for (int x = 0; x < 256; x ++)
	{
		palette[x].peRed=palette[x].peGreen=palette[x].peBlue=0;
		palette[x].peFlags = PC_NOCOLLAPSE;
	}

	if (g_lpDD->CreatePalette(DDPCAPS_8BIT|DDPCAPS_ALLOW256,palette,&g_ddp,NULL) != DD_OK)
	{
		g_lpDD->Release();
		return WASABI_API_LNGSTRING_BUF(IDS_COULD_NOT_CREATE_PALETTE,errbuf,128);
	}
	if (g_lpDDSPrim->SetPalette(g_ddp) != DD_OK)
	{
		g_lpDD->Release();
		return WASABI_API_LNGSTRING_BUF(IDS_COULD_NOT_SET_PALETTE,errbuf,128);
	}

	g_pal_lastv=-128;
	g_palette_starttime=0;
	return 0;
}

void C_DD::setpalette(unsigned char palette[768], unsigned int time_ms)
{
	memcpy(g_palette_seek,palette,768);
	memcpy(g_palette_orig,g_palette_last,768);
	g_palette_starttime=GetTickCount();
	g_palette_endtime=g_palette_starttime+time_ms;
}

void C_DD::close(void)
{
	if (g_lpDD) g_lpDD->Release();
	g_lpDD=0;
	cursurface=0;
	g_ddp=NULL;
	g_lpDDSPrim=NULL;
	g_lpPrimSurfBack=NULL;
	g_RenderSurface[0]=g_RenderSurface[1]=NULL;
	g_RenderSurfaceLocked[0]=g_RenderSurfaceLocked[1]=NULL;
	memset(g_palette_seek,0,sizeof(g_palette_seek));
	memset(g_palette_orig,0,sizeof(g_palette_orig));
	memset(g_palette_last,0,sizeof(g_palette_last));
	g_pal_lastv=0;
	g_palette_starttime=g_palette_endtime=0;
}