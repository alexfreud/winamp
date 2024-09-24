//#define PLUGIN_NAME "Nullsoft Tiny Fullscreen"
#define PLUGIN_VERSION "v2.16"

#include <windows.h>
#include <commctrl.h>
#include <ddraw.h>
#include "resource.h"
#include "../winamp/vis.h"
#include "../winamp/wa_ipc.h"
#include "../Agave/Language/api_language.h"
#include <api/service/waServiceFactory.h>

/* global variables */
wchar_t g_title[1024]={0};
wchar_t *ini_file = 0;
HWND hwndParent = 0;
HFONT hFont = 0;
unsigned char *g_scrollbuf;

extern void do_min(HWND hwnd);
extern void do_unmin(HWND hwnd);
extern BOOL DirectMouseWheel_ProcessDialogMessage(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);

/* wasabi based services for localisation support */
api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

/* window procedures */
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK dlgProc1(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

/* winamp vis module functions */
static winampVisModule *getModule(int which);
static void config_write(struct winampVisModule *this_mod);
static void config_read(struct winampVisModule *this_mod);
static void config(struct winampVisModule *this_mod);
static int init(struct winampVisModule *this_mod);
static int render_sa_vp_mono(struct winampVisModule *this_mod);
static int render_sa(struct winampVisModule *this_mod);
static int render_osc(struct winampVisModule *this_mod);
static int render_osc_sa_mono(struct winampVisModule *this_mod);
static int render_super_vp(struct winampVisModule *this_mod);
static void quit(struct winampVisModule *this_mod);

/* uninitialized data (zeroed) */
DDBLTFX ddbfx;
HWND g_hwnd;
LPDIRECTDRAW lpDD;
LPDIRECTDRAWPALETTE ddp;
LPDIRECTDRAWSURFACE lpDDSPrim;
LPDIRECTDRAWSURFACE lpDDSBack;
unsigned char *fb_locked;
int scrpitch;
unsigned char last[4][4096];
int lpos;
int rpos;
unsigned char colpoints[2][6][3];
winampVisModule mod[4];
int hadjusted;
COLORREF custcolors[16];

/* initialized data */
wchar_t szAppName[] = L"NSFSVis";
winampVisHeader hdr = { VIS_HDRVER, 0, getModule };
unsigned char orig_colpoints[2][6][3] =
{
	{
		{ 200, 0, 0 },
		{ 250, 0, 25 },
		{ 243, 65, 5 },
		{ 237, 163, 7 },
		{ 250, 250, 0 },
		{ 255, 255, 255 }
	},{
		{ 34, 29, 54 },
		{ 251, 125, 0 },
		{ 255, 160, 66 },
		{ 242, 237, 21 },
		{ 255, 255, 0 },
		{ 255, 255, 255 },
	}
};

struct
{
  int w,h,nbands, hpercent,scope,scopesc;
  int falloff;
  int peak_falloff, peak_hold;
} configst={640,480,188,80,1,33,8,8,1};


/* function implimentations */
void sd_config(struct winampVisModule *this_mod);
int sd_init(struct winampVisModule *this_mod);
int sd_render(struct winampVisModule *this_mod);
void sd_quit(struct winampVisModule *this_mod);

extern "C" {

// do this so we can have the original hinstance of our vis for localisation
/*BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		visDll = hinstDLL;
		DisableThreadLibraryCalls(hinstDLL);
	}
	return TRUE;
}*/
	static HINSTANCE GetMyInstance()
	{
		MEMORY_BASIC_INFORMATION mbi = {0};
		if(VirtualQuery(GetMyInstance, &mbi, sizeof(mbi)))
			return (HINSTANCE)mbi.AllocationBase;
		return NULL;
	}

	/* this is the only exported symbol. returns our main header. */
	__declspec( dllexport ) winampVisHeader* winampVisGetHeader(HWND hwndParent)
	{
		static char module1[96], module2[96], module3[96], module4[96];

		OSVERSIONINFO version = {0};
		version.dwOSVersionInfoSize = sizeof(version);
		if (!GetVersionEx(&version)) ZeroMemory(&version, sizeof(OSVERSIONINFO));
		if (version.dwMajorVersion < 6 ||
			(version.dwMajorVersion <= 6 && version.dwMinorVersion < 2)) // Windows 8.x+
		{
			if(!WASABI_API_LNG_HINST)
			{
				// loader so that we can get the localisation service api for use
				WASABI_API_SVC = (api_service*)SendMessage(hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
				if (WASABI_API_SVC == (api_service*)1) WASABI_API_SVC = NULL;

				waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
				if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

				// need to have this initialised before we try to do anything with localisation features
				WASABI_API_START_LANG(GetMyInstance(),VisNFSFLangGUID);
			}

			static char szDescription[256];
			char temp[256];
			wsprintfA(szDescription,"%s %s",WASABI_API_LNGSTRING_BUF(IDS_NULLSOFT_TINY_FULLSCREEN, temp, 256), PLUGIN_VERSION);
			hdr.description = szDescription;

			mod[1].latencyMs=1000/60;
			mod[1].delayMs=5;
			mod[1].spectrumNch=
			mod[1].waveformNch=1;
			mod[1].Init=init;
			mod[1].Config=config;
			mod[1].Quit=quit;
			mod[1].description=WASABI_API_LNGSTRING_BUF(IDS_SEXY_SCROLLING_VOICEPRINT,module1,96);
			mod[1].Render=render_super_vp;

			mod[2]=mod[1];
			mod[2].description=WASABI_API_LNGSTRING_BUF(IDS_SPECTRUM_ANALYZER_VOICEPRINT,module2,96);
			mod[2].waveformNch--;
			mod[2].Render=render_sa_vp_mono;

			mod[3]=mod[1];
			mod[3].description=WASABI_API_LNGSTRING_BUF(IDS_SPECTRUM_ANALYZER_OSCILLOSCOPE,module3,96);
			mod[3].Render=render_osc_sa_mono;

			mod[0]=mod[1];
			mod[0].description=WASABI_API_LNGSTRING_BUF(IDS_RANDOM_INTELLIGENT_VISUALIZATION,module4,96);
			mod[0].spectrumNch--;
			mod[0].delayMs=1;
			mod[0].Init=sd_init;
			mod[0].Config=sd_config;
			mod[0].Render=sd_render;
			mod[0].Quit=sd_quit;

			return &hdr;
		}
		return NULL;
	}
};

winampVisModule *getModule(int which)
{
	OSVERSIONINFO version = {0};
	version.dwOSVersionInfoSize = sizeof(version);
	if (!GetVersionEx(&version)) ZeroMemory(&version, sizeof(OSVERSIONINFO));
	if (version.dwMajorVersion < 6 ||
		(version.dwMajorVersion <= 6 && version.dwMinorVersion < 2)) // Windows 8.x+
	{
		if (which < 4 && which >= 0) return mod+which;
	}
	return 0;
}

/* configuration. Passed this_mod, as a "this" parameter. Allows you to make one configuration
function that shares code for all your modules (you don't HAVE to use it though, you can make
config1(), config2(), etc...)
*/
void config(struct winampVisModule *this_mod)
{
	config_read(this_mod);
	if (WASABI_API_DIALOGBOXW(IDD_DIALOG1,this_mod->hwndParent,dlgProc1) == IDOK)
		config_write(this_mod);
}

int DD_Init(int this_w, int this_h, PALETTEENTRY *palette)
{
	DDSURFACEDESC DDsd;
	DDSCAPS ddscaps;
	DDsd.dwSize = sizeof(DDsd);
	DDsd.dwFlags = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
	DDsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|DDSCAPS_COMPLEX|DDSCAPS_FLIP;
	DDsd.dwBackBufferCount = 1;
	ddbfx.dwSize=sizeof(ddbfx);

	if (DirectDrawCreate(NULL,&lpDD,NULL) != DD_OK)
	{
		lpDD=0;
		return 1;
	}
	if (IDirectDraw_SetCooperativeLevel(lpDD,g_hwnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN) != DD_OK ||
		IDirectDraw_SetDisplayMode(lpDD,this_w,this_h,8) != DD_OK ||
		IDirectDraw_CreateSurface(lpDD,&DDsd, &lpDDSPrim, NULL) != DD_OK)
	{
		IDirectDraw_Release(lpDD);
		lpDD=0;
		return 1;
	}
	IDirectDrawSurface_GetSurfaceDesc(lpDDSPrim,&DDsd);
	scrpitch = DDsd.lPitch;
	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	if (IDirectDrawSurface_GetAttachedSurface(lpDDSPrim,&ddscaps, &lpDDSBack) != DD_OK)
	{
		IDirectDraw_Release(lpDD);
		lpDD=0;
		return 1;
	}

	if (IDirectDraw_CreatePalette(lpDD,DDPCAPS_8BIT|DDPCAPS_ALLOW256,palette,&ddp,NULL) == DD_OK)
	  IDirectDrawSurface_SetPalette(lpDDSPrim,ddp);

	{
		RECT r1={0,0,this_w,this_h};
		RECT r2=r1;
		IDirectDrawSurface_Blt(lpDDSBack,&r1,NULL,NULL,DDBLT_WAIT|DDBLT_COLORFILL,&ddbfx);
		IDirectDrawSurface_Blt(lpDDSPrim,&r2,NULL,NULL,DDBLT_WAIT|DDBLT_COLORFILL,&ddbfx);
	}
	fb_locked=0;
	return 0;
}

void initwindow(struct winampVisModule *this_mod, int w, int h)
{
	if (SendMessage(this_mod->hwndParent,WM_WA_IPC,0,IPC_IS_PLAYING_VIDEO)>1)
	{
		g_hwnd=0;
		MessageBox(this_mod->hwndParent,
				   WASABI_API_LNGSTRING(IDS_CANNOT_GO_FULLSCREEN_WHEN_VIDEO_PLAYING),
				   hdr.description,MB_OK|MB_ICONINFORMATION);
		return;
	}

	WNDCLASSW wc={0,WndProc,0,0,this_mod->hDllInstance,0,0,(HBRUSH)GetStockObject(BLACK_BRUSH),0,szAppName};
	UnregisterClassW(szAppName,this_mod->hDllInstance);
	RegisterClassW(&wc);
	hwndParent=this_mod->hwndParent;
	g_hwnd = CreateWindowEx(WS_EX_APPWINDOW|WS_EX_TOPMOST,"NSFSVis",this_mod->description,
							WS_VISIBLE|WS_POPUP|WS_SYSMENU,0,0,w,h,hwndParent,NULL,
							this_mod->hDllInstance,0);
	SendMessage(this_mod->hwndParent, WM_WA_IPC, (WPARAM)g_hwnd, IPC_SETVISWND);
}

int init(struct winampVisModule *this_mod)
{
	PALETTEENTRY palette[256];
	int x,e,a;

	config_read(this_mod);

	initwindow(this_mod,configst.w,configst.h);
	if (!g_hwnd)
	{
		return 1;
	}

	SetTimer(g_hwnd,0,1000,NULL);

	{
		char *t=(char*)palette;
		int x=sizeof(palette);
		while (x--) *t++=0;
	}

	palette[255].peRed=palette[255].peGreen=palette[255].peBlue=255;
	palette[254].peRed=colpoints[0][5][0];
	palette[254].peGreen=colpoints[0][5][1];
	palette[254].peBlue=colpoints[0][5][2];
	palette[253].peRed=colpoints[1][5][0];
	palette[253].peGreen=colpoints[1][5][1];
	palette[253].peBlue=colpoints[1][5][2];

	if (this_mod == mod+1)
	{
		PALETTEENTRY *p=palette+1;
		unsigned char *t=colpoints[1][0];
		for (a = 0; a < 4; a ++)
		{
			int dr, dg, db;
			int r=*t++;
			int g=*t++;
			int b=*t++;
			dr=((t[0]-r)<<16)/60;
			dg=((t[1]-g)<<16)/60;
			db=((t[2]-b)<<16)/60;
			r<<=16;
			g<<=16;
			b<<=16;
			for (x = 0; x < 60; x ++)
			{
				p->peRed = r>>16;
				p->peGreen = g>>16;
				p->peBlue = b>>16;
				r+=dr;g+=dg;b+=db;
				p++;
			}
		}
	}
	else
	{
		PALETTEENTRY *p=palette+1;
		unsigned char *t=colpoints[0][0];
		for (e = 0; e < 3; e ++)
		{
			for (a = 0; a < 4; a ++)
			{
				int dr, dg, db;
				int r=*t++;
				int g=*t++;
				int b=*t++;
				dr=(t[0]-r);
				dg=(t[1]-g);
				db=(t[2]-b);
				r<<=4;
				g<<=4;
				b<<=4;
				for (x = 0; x < 16; x ++)
				{
					p->peRed = r>>4;
					p->peGreen = g>>4;
					p->peBlue = b>>4;
					r+=dr;g+=dg;b+=db;
					p++;
				}
			}
			t+=3*2;
		}
	}

	do_min(this_mod->hwndParent);

	if (DD_Init(configst.w,configst.h,palette))
	{
		DestroyWindow(g_hwnd);
		do_unmin(this_mod->hwndParent);
		return 1;
	}

	char *t=(char*)last;
	int y=sizeof(last);
	while (y--) *t++=0;

	hadjusted=((configst.h)*configst.hpercent)/100;
	if (hadjusted>configst.h) hadjusted=configst.h;
	if (hadjusted < 1) hadjusted=1;
	if (this_mod == mod+1) g_scrollbuf = (unsigned char *)GlobalAlloc(GPTR,configst.w*hadjusted);
	else g_scrollbuf = (unsigned char *)GlobalAlloc(GPTR,hadjusted);

	if (!g_scrollbuf)
	{
		IDirectDraw_Release(lpDD);
		lpDD=0;
		DestroyWindow(g_hwnd);
		do_unmin(this_mod->hwndParent);
		return 1;
	}

	return 0;
}

unsigned char *DD_Enter(int l, int t, int r, int b)
{
	int h;
	DDSURFACEDESC d={sizeof(d),};
	RECT r2={l,t,r,b};

	if (fb_locked) return fb_locked;

	if (l<r && t<b) IDirectDrawSurface_Blt(lpDDSBack,&r2,NULL,NULL,DDBLT_WAIT|DDBLT_COLORFILL,&ddbfx);

	if ((h = IDirectDrawSurface_Lock(lpDDSBack,NULL,&d,DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY,NULL)) != DD_OK)
	{
		if (h == DDERR_SURFACELOST)
		{
			IDirectDrawSurface_Restore(lpDDSPrim);
		}
		return 0;
	}

	return (fb_locked = (unsigned char *) d.lpSurface);
}

void DD_Unlock(int w, int transparent)
{
	if (fb_locked)
	{
  		HDC hdc;
		RECT r1={0,0,w,32};
		RECT r3=r1;
		IDirectDrawSurface_Unlock(lpDDSBack,fb_locked);
		fb_locked=0;
		if (!transparent) IDirectDrawSurface_Blt(lpDDSBack,&r1,NULL,NULL,DDBLT_WAIT|DDBLT_COLORFILL,&ddbfx);
		if (IDirectDrawSurface_GetDC(lpDDSBack,&hdc) == DD_OK)
		{
			if (!transparent) SetBkColor(hdc,0);
			else SetBkMode(hdc,TRANSPARENT);
			SetTextColor(hdc,RGB(255,255,255));
			HFONT oldfont = (HFONT)SelectObject(hdc,hFont);
  			DrawTextW(hdc,g_title,-1,&r3,DT_CENTER|DT_TOP|DT_SINGLELINE);
			SelectObject(hdc,oldfont);
			IDirectDrawSurface_ReleaseDC(lpDDSBack,hdc);
		}
		IDirectDrawSurface_Flip(lpDDSPrim,NULL, DDFLIP_WAIT);
	}
}

void _render_analyzer(struct winampVisModule *this_mod, int hdiv)
{
	int x;
	int pos;
	int dpos;
	unsigned char *sd=this_mod->spectrumData[0];
	unsigned char *sb;
	int len=min(configst.w,4096);

	sb=fb_locked+scrpitch*(configst.h-1);

	pos=0;
	dpos=(400<<16)/configst.w;
	for (x = 0; x < len; x ++)
	{
		unsigned char *f = sb++;
		int c = 2;
		int lo=0;
		int lv=last[0][x]-configst.falloff*2;

		int ipos=(pos>>16);
		int l = (((int)sd[ipos+1])*((pos&0xffff)) + ((int)sd[ipos  ])*(0x10000-(pos&0xffff)))>>16;

		l = l*(ipos+40)/80;
		l>>=hdiv;
		if (l > (128>>hdiv)-1) l = (128>>hdiv)-1;
		pos+=dpos;
		if (configst.peak_hold)
		{
			if ((int)last[2][x]+configst.peak_falloff < 255) last[2][x]+=configst.peak_falloff;
			else last[2][x]=255;
			if (last[3][x] < last[2][x]/32) last[3][x]=0;
			else last[3][x]-=last[2][x]/32;
			if (l*2 > last[3][x])
			{
				last[2][x]=0;
				last[3][x]=l*2;
			}
			if (last[3][x]/2 >= l) lo=(-(int)last[3][x]/2)*scrpitch;
		}

		if (l < lv) l = lv;
		last[0][x]= l;

		while (l--)
		{
			f[0] = c/2;
			c+=1+hdiv;
			f -= scrpitch;
		}
		if (lo) sb[lo-1]=254;
	}
}

void _render_scope(struct winampVisModule *this_mod, int hr)
{
	int x;
	unsigned char *wd=this_mod->waveformData[0];
	int scsc;
	int pos,dpos,lastv;
	unsigned char *sb;
	scsc=(configst.scopesc*hr)/100;
	if (scsc < 1) scsc=1;
	if (scsc > hr) scsc=hr;
	sb=fb_locked+scrpitch*(configst.h/2-scsc/2);
	lastv=-1;
	pos=0;
	dpos=(575<<16)/configst.w;
	for (x = 0; x < configst.w; x ++)
	{
		int tv = (((int)wd[(pos>>16)+1]^128)*((pos&0xffff)) + ((int)wd[(pos>>16)]  ^128)*(0x10000-(pos&0xffff)))>>16;
		tv=(tv*scsc)>>8;
		pos+=dpos;
		if (lastv<0) lastv=tv;
		if (tv==lastv) sb[scrpitch*lastv]=253;
		else
		{
			while (tv < lastv) sb[scrpitch*lastv--]=253;
			while (tv > lastv) sb[scrpitch*lastv++]=253;
		}
		sb++;
	}
}

int render_super_vp(struct winampVisModule *this_mod)
{
	int tb=min(configst.nbands,511);
	int sy=((configst.h-hadjusted)/2);
	unsigned char *src=&this_mod->spectrumData[0][tb];
	unsigned char *op;
	unsigned char *p1=g_scrollbuf;
	unsigned char *p2=g_scrollbuf+configst.w;
	unsigned int pos=0;
	unsigned int dpos=(tb<<16)/hadjusted;

	if (!g_scrollbuf || !DD_Enter(0,0,configst.w,min(sy,32))) return 0;

	op=fb_locked+sy*scrpitch;

	int y=hadjusted/2;
	while (y--)
	{
		int x=configst.w-1;
		while (x--)
		{
			*p1++=p1[1];
			*p2++=p2[1];
		}
		x = (((int)src[0 - (pos>>16)])*((pos&0xffff)) + ((int)src[1 - (pos>>16)])*(0x10000-(pos&0xffff)))>>16;
		pos+=dpos;
		if (++x > 240) x = 240;
		*p1++ = x;
		x = (((int)src[0 - (pos>>16)])*((pos&0xffff)) + ((int)src[1 - (pos>>16)])*(0x10000-(pos&0xffff)))>>16;
		pos+=dpos;
		if (++x > 240) x = 240;
		*p2++ = x;

		memcpy(op,p1-configst.w,configst.w);
		op+=scrpitch;
		memcpy(op,p2-configst.w,configst.w);
		op+=scrpitch;
		p1+=configst.w;
		p2+=configst.w;
	}
	if (configst.scope) _render_scope(this_mod,hadjusted);

	DD_Unlock(configst.w,1);
	return 0;
}

int render_sa_vp_mono(struct winampVisModule *this_mod)
{
	int x;
	unsigned char *sd=this_mod->spectrumData[0];
	unsigned char *b3,*b1,*b2;
	int pos=0;
	int vsize=hadjusted;
	int dpos=(min(configst.nbands,511)<<16)/vsize;

	if (!DD_Enter(0,configst.h-64,configst.w,configst.h)) return 0;

	if (vsize > configst.h-32-64) vsize=configst.h-32-64;
	b1 = fb_locked + scrpitch*((configst.h-32-64)/2+32+vsize/2) + lpos;
	b2 = b1+rpos-lpos;

	_render_analyzer(this_mod,1);

	for (x = 0; x < vsize; x ++)
	{
		b1[0] = g_scrollbuf[x];
		b1-=scrpitch;
	}

	if (rpos < configst.w-2) b3 = b2+1;
	else b3 = b2 - (configst.w-2);

  	for (x = 0; x < vsize; x++)
	{
    int ipos=(pos>>16);
		int l = (((int)sd[ipos+1])*((pos&0xffff)) + ((int)sd[ipos])  *(0x10000-(pos&0xffff)))>>16;
		l=(l*240)/1024;
		pos+=dpos;
		if (l > 63) l = 63;
		l+=65;
		b2[0]=l;
		g_scrollbuf[x]=l;
		b2-=scrpitch;
		b3[0] = 255;
		b3-=scrpitch;
	}

	lpos=rpos;
	if (++rpos >= configst.w) rpos -= configst.w;

	DD_Unlock(configst.w,0);
	return 0;
}

int render_osc_sa_mono(struct winampVisModule *this_mod)
{
	if (!DD_Enter(0,0,configst.w,configst.h)) return 0;

	_render_analyzer(this_mod,0);
	_render_scope(this_mod,configst.h);

	DD_Unlock(configst.w,1);
	return 0;
}

/*
cleanup (opposite of init()). Destroys the window, unregisters the window class
*/
void quit(struct winampVisModule *this_mod)
{
	IDirectDrawSurface_Release(lpDDSPrim);
	IDirectDraw_Release(lpDD);
	DestroyWindow(g_hwnd);
	if (g_scrollbuf) GlobalFree(g_scrollbuf);
	g_scrollbuf=0;
	UnregisterClassW(szAppName,this_mod->hDllInstance);
	do_unmin(this_mod->hwndParent);
}

/*
window procedure for our window
*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CREATE){
		if(hFont) DeleteObject(hFont);
		HDC hdc = GetDC(hwnd);
		hFont = CreateFontW(-MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72),
							0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, L"Arial");
		ReleaseDC(hwnd,hdc);
	}
	else if (message == WM_PAINT)
	{
		PAINTSTRUCT ps = {0};
		HDC hdc = BeginPaint(hwnd,&ps);
		EndPaint(hwnd,&ps);
	}
	else if (message == WM_KEYUP || message == WM_KEYDOWN)
	{
		if (wParam == VK_ESCAPE) DestroyWindow(hwnd);
		else
		{
		PostMessage(hwndParent,message,wParam,lParam);
		}
	}
	else if (message ==WM_USER+1666)
	{
		if (wParam == 1 && lParam == 15)
		{
			DestroyWindow(hwnd);
		}
		return 0;
	}
	else if (message == WM_DESTROY)
	{
		SendMessage(hwndParent, WM_WA_IPC, 0, IPC_SETVISWND);
		if(hFont) DeleteObject(hFont);
		PostQuitMessage(0);
	}
	else if (message == WM_TIMER)
	{
		g_title[0] = 0;
		GetWindowTextW(hwndParent,g_title,sizeof(g_title)-1-32);
		wchar_t *p = g_title;
		while (p && *p) p = CharNextW(p); p = CharPrevW(g_title,p);
		while (p >= g_title)
		{
			if (p[0] == L'-' && p[1] == L' ' && p[2] == L'W' && p[3] == L'i') break;
			p = CharPrevW(g_title,p);
		}
		if (p >= g_title)
		{
			wchar_t *e=p;
			while (e && *e && *e != L'[') e = CharNextW(e);
			while (p >= g_title && *p == L' ') p = CharPrevW(g_title,p);
			if (p < g_title) p = CharNextW(p);
			*(p = CharNextW(p)) = L' ';
			*p = 0;
			if (e && *e)
			{
				int n=29;
				while (e && *e && n--)
				{
					*p = *e;
					p = CharNextW(p);
					e = CharNextW(e);
				}
				*p = 0;
			}
		}
		else g_title[0]=0;

		LRESULT t = (SendMessage(hwndParent,WM_USER,0,105) + 500) / 1000;
		if (t)
		{
			p = g_title;
			while (p && *p) p = CharNextW(p);
			LRESULT tt = (SendMessage(hwndParent,WM_USER,1,105)*1000);
			if(tt > 0){
			int length = tt/=1000,
				minutes = length/60,
				seconds = length - (minutes*60);
				wsprintfW(p,L" [%d:%02d / %d:%02d]",t/60,t%60,minutes,seconds);
			}
			else{
				wsprintfW(p,L" [%d:%02d / %d:%02d]",t/60,t%60);
			}
		}
		// if we did not get a time then remove the '-' on the end
		else{
			*(p = CharPrevW(g_title,p)) = 0;
		}
	}
	else if (message == WM_SETCURSOR) SetCursor(NULL);
	else return DefWindowProc(hwnd,message,wParam,lParam);
	return 0;
}

void init_inifile(struct winampVisModule *this_mod)
{
	ini_file = (wchar_t*)SendMessage(this_mod->hwndParent,WM_WA_IPC,0,IPC_GETINIFILEW);
}

static void config_read(struct winampVisModule *this_mod)
{
	init_inifile(this_mod);
	if (!GetPrivateProfileStructW(szAppName,L"colors",colpoints,sizeof(colpoints),ini_file))
	{
		int x=sizeof(colpoints);
		char *i=(char*)orig_colpoints, *o=(char*)colpoints;
		while (x--) *o++=*i++;
	}

	GetPrivateProfileStructW(szAppName,L"config",&configst,sizeof(configst),ini_file);
}

void config_write(struct winampVisModule *this_mod)
{
	init_inifile(this_mod);
	WritePrivateProfileStructW(szAppName,L"colors",colpoints,sizeof(colpoints),ini_file);
	WritePrivateProfileStructW(szAppName,L"config",&configst,sizeof(configst),ini_file);
}

void do_vprintcol(HWND hwndDlg, unsigned char *t)
{
	CHOOSECOLORW cs={sizeof(cs),hwndDlg,};
	cs.hInstance = 0;
	cs.rgbResult=t[0] | (t[1]<<8) | (t[2]<<16);
	cs.lpCustColors = custcolors;
	cs.Flags = CC_RGBINIT|CC_FULLOPEN;
	if (ChooseColorW(&cs))
	{
		t[0] = (unsigned char) cs.rgbResult;
		t[1] = (unsigned char) (cs.rgbResult>>8);
		t[2] = (unsigned char) (cs.rgbResult>>16);
  		InvalidateRect(hwndDlg,NULL,0);
	}
}

static HRESULT WINAPI _cb(LPDDSURFACEDESC lpDDSurfaceDesc,  LPVOID lpContext)
{
	if (lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount==8)
	{
		wchar_t s[32] = {0};
		wsprintfW(s, L"%dx%d", lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight);
		LRESULT idx = SendMessageW((HWND)lpContext,CB_ADDSTRING,0,(LPARAM)s);
		SendMessage((HWND)lpContext,CB_SETITEMDATA,idx,MAKELONG(lpDDSurfaceDesc->dwWidth,lpDDSurfaceDesc->dwHeight));

		if ((int)lpDDSurfaceDesc->dwWidth == configst.w && (int)lpDDSurfaceDesc->dwHeight == configst.h)
			SendMessage((HWND)lpContext,CB_SETCURSEL,idx,0);
	}
	return DDENUMRET_OK;
}

static INT_PTR CALLBACK dlgProc1(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_DRAWITEM)
	{
  		DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
		unsigned char *thiscol=NULL;
		if (di->CtlID >= IDC_AP1 && di->CtlID <= IDC_VP6) thiscol=colpoints[0][di->CtlID-IDC_AP1];

		if (thiscol)
		{
  			HBRUSH b, ob;
	  		RECT r;
			GetClientRect(di->hwndItem,&r);
			b = CreateSolidBrush(thiscol[0] | (thiscol[1]<<8) | (thiscol[2]<<16));
			ob = (HBRUSH) SelectObject(di->hDC,b);
			Rectangle(di->hDC,r.left,r.top,r.right,r.bottom);
			SelectObject(di->hDC,ob);
			DeleteObject(b);
		}
	}

	if (uMsg == WM_INITDIALOG)
	{
		SetWindowText(hwndDlg,hdr.description);
		if (configst.peak_hold) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
		SendDlgItemMessage(hwndDlg,IDC_FALLOFF,TBM_SETRANGE,0,MAKELONG(1,40));
		SendDlgItemMessage(hwndDlg,IDC_FALLOFF,TBM_SETPOS,1,configst.falloff);
		SendDlgItemMessage(hwndDlg,IDC_FALLOFF2,TBM_SETRANGE,0,MAKELONG(1,40));
		SendDlgItemMessage(hwndDlg,IDC_FALLOFF2,TBM_SETPOS,1,configst.peak_falloff);
		{
		LPDIRECTDRAW dd;
			if (DirectDrawCreate(NULL,&dd,NULL) == DD_OK)
		{
			IDirectDraw_EnumDisplayModes(dd,0,NULL,GetDlgItem(hwndDlg,IDC_MODELIST),_cb);
			IDirectDraw_Release(dd);
		}
	  		SendDlgItemMessage(hwndDlg,IDC_BANDS,TBM_SETRANGE,0,MAKELONG(30,511));
  			SendDlgItemMessage(hwndDlg,IDC_BANDS,TBM_SETPOS,1,configst.nbands);
	  		SendDlgItemMessage(hwndDlg,IDC_SCSCALE,TBM_SETRANGE,0,MAKELONG(1,100));
  			SendDlgItemMessage(hwndDlg,IDC_SCSCALE,TBM_SETPOS,1,configst.scopesc);
	  		SendDlgItemMessage(hwndDlg,IDC_VUSE,TBM_SETRANGE,0,MAKELONG(1,100));
  			SendDlgItemMessage(hwndDlg,IDC_VUSE,TBM_SETPOS,1,configst.hpercent);
			if (configst.scope) CheckDlgButton(hwndDlg,IDC_SCOPE,BST_CHECKED);
		}
	}

	if (uMsg == WM_CLOSE)
	{
		uMsg=WM_COMMAND;
		wParam=IDCANCEL;
	}

	if (uMsg == WM_COMMAND)
	{
	    int w=LOWORD(wParam);
		if (w == IDOK || w == IDCANCEL)
		{
			configst.falloff = SendDlgItemMessage(hwndDlg,IDC_FALLOFF,TBM_GETPOS,0,0);
			configst.peak_falloff = SendDlgItemMessage(hwndDlg,IDC_FALLOFF2,TBM_GETPOS,0,0);
			configst.peak_hold=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK1);
			configst.scope=IsDlgButtonChecked(hwndDlg,IDC_SCOPE)?1:0;
			configst.scopesc = SendDlgItemMessage(hwndDlg,IDC_SCSCALE,TBM_GETPOS,0,0);
			configst.nbands = SendDlgItemMessage(hwndDlg,IDC_BANDS,TBM_GETPOS,0,0);
			configst.hpercent = SendDlgItemMessage(hwndDlg,IDC_VUSE,TBM_GETPOS,0,0);
			EndDialog(hwndDlg,w);
		}

		if (w == IDC_MODELIST && HIWORD(wParam) == CBN_SELCHANGE)
		{
			DWORD n=SendDlgItemMessage(hwndDlg,IDC_MODELIST,CB_GETCURSEL,0,0);
			if (n != CB_ERR)
			{
				n=SendDlgItemMessage(hwndDlg,IDC_MODELIST,CB_GETITEMDATA,n,0);
				if (n != CB_ERR)
				{
					configst.w=LOWORD(n);
					configst.h=HIWORD(n);
				}
			}
		}
		if (w == IDC_DEFVP || w == IDC_DEFAP)
		{
			int p=(w != IDC_DEFAP);
			int x=sizeof(colpoints)/2;
			char *i=(char*)orig_colpoints[p], *o=(char*)colpoints[p];
			if (p) x-=3;
			while (x--) *o++=*i++;
			InvalidateRect(hwndDlg,NULL,FALSE);
		}
		if (w >= IDC_AP1 && w <= IDC_VP6) do_vprintcol(hwndDlg,colpoints[0][w-IDC_AP1]);
	}

	if (FALSE != DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam))
	{
		return TRUE;
	}

	return 0;
}