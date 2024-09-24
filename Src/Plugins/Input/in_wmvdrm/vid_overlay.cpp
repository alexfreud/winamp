#include "main.h"
#include <windows.h>
#include <multimon.h>
#include "vid_overlay.h"
#include "directdraw.h"
#include <api.h>
#include "../Winamp/wa_ipc.h"
#include <assert.h>
void getViewport(RECT *r, HWND wnd, int full, RECT *sr);


void YV12_to_YUY2(unsigned char *output, const YV12_PLANES *planes, int pitch, int width, int height, int flip)
{
	const unsigned char *yi = planes->y.baseAddr;
	const unsigned char *ui = planes->u.baseAddr;
	const unsigned char *vi = planes->v.baseAddr;
	if (flip)
		output += pitch * (height - 1);
	while (height > 0)
	{
		int x = width;
		unsigned char *oo = output;

		while (x > 0)
		{
			output[0] = *yi++; output[1] = *ui++; output[2] = *yi++; output[3] = *vi++;
			output += 4; x -= 2;
		}
		ui -= width / 2;
		vi -= width / 2;
		yi += planes->y.rowBytes - width;
		x = width;
		if (flip) output = oo - pitch;
		else output += pitch - width * 2;
		oo = output;
		while (x > 0)
		{
			output[0] = *yi++; output[1] = *ui++; output[2] = *yi++; output[3] = *vi++;
			output += 4; x -= 2;
		}
		if (flip) output = oo - pitch;
		else output += pitch - width * 2;
		ui += planes->u.rowBytes - (width / 2);
		vi += planes->v.rowBytes - (width / 2);
		yi += planes->y.rowBytes - width;
		height -= 2;
	}
}

void YV12_to_UYVY(unsigned char *output, const YV12_PLANES *planes, int pitch, int width, int height, int flip)
{
	const unsigned char *yi = planes->y.baseAddr;
	const unsigned char *ui = planes->u.baseAddr;
	const unsigned char *vi = planes->v.baseAddr;

	if (flip) output += pitch * (height - 1);
	while (height > 0)
	{
		int x = width;
		unsigned char *oo = output;

		while (x > 0)
		{
			output[0] = *ui++; output[1] = *yi++; output[2] = *vi++; output[3] = *yi++;
			output += 4; x -= 2;
		}

		ui -= width / 2;
		vi -= width / 2;
		yi += planes->y.rowBytes - width;
		x = width;
		if (flip) output = oo - pitch;
		else output += pitch - width * 2;
		oo = output;
		while (x > 0)
		{
			output[0] = *ui++; output[1] = *yi++; output[2] = *vi++; output[3] = *yi++;
			output += 4; x -= 2;
		}

		if (flip) output = oo - pitch;
		else output += pitch - width * 2;
		ui += planes->u.rowBytes - (width / 2);
		vi += planes->v.rowBytes - (width / 2);
		yi += planes->y.rowBytes - width;
		height -= 2;
	}
}

void YV12_to_YV12(unsigned char *output, const YV12_PLANES *planes, int pitch, int width, int height, int flip)
{ // woo native YV12 copy
	int f = !!flip;
	unsigned char *o = output + (f * height * pitch);
	const char *i = (const char*)planes->y.baseAddr;


	int d_o = pitch;
	if (f) d_o = -d_o;
	else o -= d_o;

	int h2 = height;
	while (h2--)
	{
		o += d_o; memcpy(o, i, width); i += planes->y.rowBytes;
	}

	d_o /= 2;

	int w2 = width / 2;
	h2 = height / 2;
	i = (const char*)planes->v.baseAddr;
	o = output + (height * pitch * (f + 4)) / 4;

	if (!f) o -= d_o;
	while (h2--)
	{
		o += d_o; memcpy(o, i, w2); i += planes->v.rowBytes;
	}
	o = output + (height * pitch * (f + 5)) / 4;
	i = (const char*)planes->u.baseAddr;
	h2 = height / 2;

	if (!f) o -= d_o;
	while (h2--)
	{
		o += d_o; memcpy(o, i, w2);i += planes->u.rowBytes;
	}
}


void YUY2_to_YUY2(unsigned char *output, const char *buf, int pitch, int width, int height, int flip)
{
	const char *a = buf;
	unsigned char *b = output;
	int l = width * 2, l2 = pitch;
	if (flip)
	{
		b += (height - 1) * l2;
		l2 = -l2;
	}


	//wee straight YUY2 copy
	for (int i = 0;i < height;i++)
	{
		memcpy(b, a, l);
		b += l2;
		a += l;
	}

}

void YUY2_to_UYVY(unsigned char *output, const char *buf, int pitch, int width, int height, int flip)
{
	const char *a = buf;
	unsigned char *b = output;
	int l = width * 2, l2 = pitch;
	if (flip)
	{
		b += (height - 1) * l2;
		l2 = -l2;
	}

	for (int i = 0;i < height;i++)
	{
		int x = width / 2;
		while (x-- > 0)
		{
			b[0] = a[1];
			b[1] = a[0];
			b[2] = a[3];
			b[3] = a[2];
			a += 4;
			b += 4;
		}
		memcpy(b, a, l);
		b += l2;
		a += l;
	}
}

#define INIT_DIRECTDRAW_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x)) 
// I like to set these to 255,0,255 to test that we arent drawing this color too many playces
#define OV_COL_R 16
#define OV_COL_G 0
#define OV_COL_B 16

OverlayVideoOutput::OverlayVideoOutput() : frame(NULL), width(0), height(0),
		flip(0), type(0), uDestSizeAlign(0), uSrcSizeAlign(0), dwUpdateFlags(0)
{
	lpDD = NULL;
	m_closed = 0;
	overlay_color = RGB(OV_COL_R, OV_COL_G, OV_COL_B);
	lpddsOverlay = NULL;
	lpddsPrimary = NULL;
	lpBackBuffer = NULL;

	yuy2_output = uyvy_output = 0;
	initing = false;
	needchange = 0;
	memset(&m_oldrd, 0, sizeof(m_oldrd));
	memset(&winRect, 0, sizeof(winRect));
	m_fontsize = 0;
}

OverlayVideoOutput::~OverlayVideoOutput()
{
	OverlayVideoOutput::close();
}


static DWORD DD_ColorMatch(LPDIRECTDRAWSURFACE pdds, COLORREF rgb)
{
	COLORREF rgbT;
	HDC hdc;
	DWORD dw = CLR_INVALID;
	DDSURFACEDESC ddsd;
	HRESULT hres;

	//
	//  use GDI SetPixel to color match for us
	//
	if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
	{
		rgbT = GetPixel(hdc, 0, 0);     // save current pixel value
		SetPixel(hdc, 0, 0, rgb);       // set our value
		pdds->ReleaseDC(hdc);
	}

	// now lock the surface so we can read back the converted color
	ddsd.dwSize = sizeof(ddsd);
	while ((hres = pdds->Lock(NULL, &ddsd, 0, NULL)) ==
	       DDERR_WASSTILLDRAWING)
		;

	if (hres == DD_OK)
	{
		dw = *(DWORD *)ddsd.lpSurface;    // get DWORD
		if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
			dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;                        // mask it to bpp
		pdds->Unlock(NULL);
	}


	//  now put the color that was there back.
	if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
	{
		SetPixel(hdc, 0, 0, rgbT);
		pdds->ReleaseDC(hdc);
	}

	return dw;
}

int OverlayVideoOutput::create(HWND parent, VideoAspectAdjuster *_adjuster, int w, int h, unsigned int ptype, int flipit, double aspectratio)
{
  OverlayVideoOutput::close();
	this->parent = parent;
	type = ptype;
	width = w;
	height = h;
	flip = flipit;

	adjuster = _adjuster;

	initing = true;
	HWND hwnd = this->parent;

	if (lpDD) lpDD->Release();
	lpDD = NULL;

	update_monitor_coords();

	if (!foundGUID) DDrawCreate(NULL, &lpDD, NULL);
	else DDrawCreate(&m_devguid, &lpDD, NULL);


	if (!lpDD)
	{
		initing = false;
		return 0;
	}

	lpDD->SetCooperativeLevel(hwnd, DDSCL_NOWINDOWCHANGES | DDSCL_NORMAL);

	DDSURFACEDESC ddsd;
	INIT_DIRECTDRAW_STRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	lpDD->CreateSurface(&ddsd, &lpddsPrimary, NULL );

	if (!lpddsPrimary)
	{
		if (lpDD) lpDD->Release();
		lpDD = NULL;
		initing=false;
		return 0;
	}

	// init overlay
	DDSURFACEDESC ddsdOverlay;
	INIT_DIRECTDRAW_STRUCT(ddsdOverlay);
	//ddsdOverlay.ddsCaps.dwCaps=DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
	//ddsdOverlay.dwFlags= DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_PITCH;
	//ddsdOverlay.dwBackBufferCount=0;
	ddsdOverlay.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsdOverlay.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_PITCH | DDSD_BACKBUFFERCOUNT;
	ddsdOverlay.dwBackBufferCount = 1;
	ddsdOverlay.dwWidth = w;
	ddsdOverlay.dwHeight = h;
	ddsdOverlay.lPitch = w * 4;
	DDPIXELFORMAT pf[] =
	    {
	        {sizeof(DDPIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('Y', 'U', 'Y', '2'), 0, 0, 0, 0, 0},
	        {sizeof(DDPIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('U', 'Y', 'V', 'Y'), 0, 0, 0, 0, 0},         // UYVY
	        {sizeof(DDPIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('Y', 'V', '1', '2'), 0, 0, 0, 0, 0},
	    };
	int tab[5] = {0};
	if (type == VIDEO_MAKETYPE('Y', 'U', 'Y', '2'))
	{
		tab[0] = 0; // default is YUY2
		tab[1] = 1;
		tab[2] = -1;
	}
	else if (type == VIDEO_MAKETYPE('U', 'Y', 'V', 'Y'))
	{
		tab[0] = 1; // make UYVY default
		tab[1] = 0;
		tab[2] = -1;
	}
	else if (type == VIDEO_MAKETYPE('Y', 'V', '1', '2'))
	{
		if (config_video.yv12())
		{
			tab[0] = 2;
			tab[1] = 0;
			tab[2] = 1;
			tab[3] = -1;
		}
		else
		{
			//use YUY2
			tab[0] = 0; // default is YUY2
			tab[1] = 1;
			tab[2] = -1;
		}
	}
	else
	{
		tab[0] = -1; // default is RGB
	}

	int x = 4096;
	HRESULT v = -1;
	for (x = 0; x < sizeof(tab) / sizeof(tab[0]) && tab[x] >= 0; x ++)
	{
		ddsdOverlay.ddpfPixelFormat = pf[tab[x]];
		v = lpDD->CreateSurface(&ddsdOverlay, &lpddsOverlay, NULL);
		if (!FAILED(v)) break;
	}
	if (FAILED(v) || x >= sizeof(tab) / sizeof(tab[0]) || tab[x] < 0)
	{
		initing = false;
		return 0;
	}

	yuy2_output = (tab[x] == 0);
	uyvy_output = (tab[x] == 1);

	//get the backbuffer surface
	DDSCAPS ddscaps;
	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	v = lpddsOverlay->GetAttachedSurface(&ddscaps, &lpBackBuffer);
	if (v != DD_OK || lpBackBuffer == 0)
	{
		// make it use normal vsync
		lpBackBuffer=0;
		initing = FALSE;
		return 0;
	}

	INIT_DIRECTDRAW_STRUCT(capsDrv);
	lpDD->GetCaps(&capsDrv, NULL);

	uDestSizeAlign = capsDrv.dwAlignSizeDest;
	uSrcSizeAlign = capsDrv.dwAlignSizeSrc;

	dwUpdateFlags = DDOVER_SHOW | DDOVER_KEYDESTOVERRIDE;

	DEVMODE d;
	d.dmSize = sizeof(d);
	d.dmDriverExtra = 0;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &d);

	int rv = OV_COL_R, gv = OV_COL_G, bv = OV_COL_B;
	overlay_color = RGB(rv, gv, bv);

	if (d.dmBitsPerPel == 8)
	{
		overlay_color = RGB(255, 0, 255);
	}

	INIT_DIRECTDRAW_STRUCT(ovfx);
	ovfx.dwDDFX = 0;
	switch (d.dmBitsPerPel)
	{
		case 8:
			ovfx.dckDestColorkey.dwColorSpaceLowValue = 253;
			break;
		case 16:
			ovfx.dckDestColorkey.dwColorSpaceLowValue = ((rv >> 3) << 11) | ((gv >> 2) << 5) | (bv >> 3);
			break;
		case 15:
			ovfx.dckDestColorkey.dwColorSpaceLowValue = ((rv >> 3) << 10) | ((gv >> 3) << 5) | (bv >> 3);
			break;
		case 24:
		case 32:
			ovfx.dckDestColorkey.dwColorSpaceLowValue = (rv << 16) | (gv << 8) | bv;
			break;
	}

	//try to get the correct bit depth thru directdraw (for fucked up 16 bits displays for ie.)
	{
		DDSURFACEDESC DDsd = {sizeof(DDsd), };
		lpddsPrimary->GetSurfaceDesc(&ddsd);
		DDsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT; //create the surface at screen depth
		DDsd.dwWidth = 8;
		DDsd.dwHeight = 8;
		DDsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
		LPDIRECTDRAWSURFACE tempsurf;
		if (lpDD->CreateSurface(&DDsd, &tempsurf, NULL) == DD_OK)
		{
			int res = DD_ColorMatch(tempsurf, overlay_color);
			if (res != CLR_INVALID) ovfx.dckDestColorkey.dwColorSpaceLowValue = res;
			tempsurf->Release();
		}
	}

	ovfx.dckDestColorkey.dwColorSpaceHighValue = ovfx.dckDestColorkey.dwColorSpaceLowValue;

	getRects(&rs, &rd);
	if (FAILED(lpddsOverlay->UpdateOverlay(&rs, lpddsPrimary, &rd, dwUpdateFlags, &ovfx)))
	{
		initing = false;
		return 0;
	}
	initing = false;

	DDSURFACEDESC dd = {sizeof(dd), };
	if (lpddsOverlay->Lock(NULL, &dd, DDLOCK_WAIT, NULL) != DD_OK) return 0;
	unsigned char *o = (unsigned char*)dd.lpSurface;
	if (uyvy_output || yuy2_output)
	{
		int x = dd.lPitch * height / 2;
		while (x--)
		{
			if (uyvy_output)
			{
				*o++ = 128;
				*o++ = 0;
			}
			else
			{
				*o++ = 0;
				*o++ = -128;
			}
		}
	}
	else
	{
		memset(o, 0, dd.lPitch*height); o += dd.lPitch * height;
		memset(o, 128, dd.lPitch*height / 2);
	}
	lpddsOverlay->Unlock(&dd);

	m_closed=0;
	needchange = 0;
	InvalidateRect(hwnd, NULL, TRUE);
	
	return 1;
}

void OverlayVideoOutput::close()
{
	m_closed = 1;
	if (lpddsOverlay) lpddsOverlay->UpdateOverlay(NULL, lpddsPrimary, NULL, DDOVER_HIDE , NULL); 
	if (lpBackBuffer) lpBackBuffer->Release(); lpBackBuffer=0;
	if (lpddsOverlay) lpddsOverlay->Release(); lpddsOverlay=0;
	if (lpddsPrimary) lpddsPrimary->Release(); lpddsPrimary=0;
	if (lpDD) lpDD->Release();	lpDD=0; // BU added NULL check in response to talkback
}

void OverlayVideoOutput::getRects(RECT *drs, RECT *drd, int fixmultimon) const
{
	//if(GetParent(hwnd)) hwnd=GetParent(hwnd);

	RECT rd, rs;
	GetClientRect(parent, &rd);
	ClientToScreen(parent, (LPPOINT)&rd);
	ClientToScreen(parent, ((LPPOINT)&rd) + 1);

	adjuster->adjustAspect(rd);

	rd.left -= m_mon_x;
	rd.right -= m_mon_x;
	rd.top -= m_mon_y;
	rd.bottom -= m_mon_y;

	memset(&rs, 0, sizeof(rs));
	rs.right = width;
	rs.bottom = height;

	if (fixmultimon)
	{
		//resize overlay for off-screen
		RECT rfull;
		getViewport(&rfull, parent, 1, NULL);

		rfull.left -= m_mon_x;
		rfull.right -= m_mon_x;
		rfull.top -= m_mon_y;
		rfull.bottom -= m_mon_y;

		if (rd.right > rfull.right)
		{
			int diff = rd.right - rfull.right;
			float sc = (float)(width) / (float)(rd.right - rd.left);
			rd.right = rfull.right;
			rs.right = width - (int)(diff * sc);
		}
		if (rd.left < rfull.left)
		{
			int diff = rfull.left - rd.left;
			float sc = (float)(width) / (float)(rd.right - rd.left);
			rd.left = rfull.left;
			rs.left = (int)(diff * sc);
		}
		if (rd.bottom > rfull.bottom)
		{
			int diff = rd.bottom - rfull.bottom;
			float sc = (float)(height) / (float)(rd.bottom - rd.top);
			rd.bottom = rfull.bottom;
			rs.bottom = height - (int)(diff * sc);
		}
		if (rd.top < rfull.top)
		{
			int diff = rfull.top - rd.top;
			float sc = (float)(height) / (float)(rd.bottom - rd.top);
			rd.top = rfull.top;
			rs.top = (int)(diff * sc);
		}
	}

	if (capsDrv.dwCaps & DDCAPS_ALIGNSIZESRC && uDestSizeAlign)
	{
		rs.left = (int)((rs.left + uDestSizeAlign - 1) / uDestSizeAlign) * uDestSizeAlign;
		rs.right = (int)((rs.right + uDestSizeAlign - 1) / uDestSizeAlign) * uDestSizeAlign;
	}
	if (capsDrv.dwCaps & DDCAPS_ALIGNSIZEDEST && uDestSizeAlign)
	{
		rd.left = (int)((rd.left + uDestSizeAlign - 1) / uDestSizeAlign) * uDestSizeAlign;
		rd.right = (int)((rd.right + uDestSizeAlign - 1) / uDestSizeAlign) * uDestSizeAlign;
	}

	*drd = rd;
	*drs = rs;
}

void OverlayVideoOutput::timerCallback()
{
	if (!adjuster)
		return;
	RECT rd, rs;
	getRects(&rs, &rd);

	if (memcmp(&m_oldrd, &rd, sizeof(RECT)))
	{
		m_oldrd = rd;
		if (!initing && lpddsOverlay)
			if (FAILED(lpddsOverlay->UpdateOverlay(&rs, lpddsPrimary, &rd, dwUpdateFlags, &ovfx)))
			{
				needchange = 1;
			}
		InvalidateRect(parent, NULL, FALSE);
	}
}

int OverlayVideoOutput::onPaint(HWND hwnd)
{
	if (!adjuster)
		return 0;
	PAINTSTRUCT p;
	BeginPaint(hwnd, &p);

	if (!m_closed)
	{
		RECT r, rs, rfull, clientRect;
		RECT drawRect;
		getRects(&rs, &r, 0); // we don't just fill the entire client rect, cause that looks gross

		getViewport(&rfull, hwnd, 1, NULL);

		// go from this screen coords to global coords
		r.left += rfull.left;
		r.top += rfull.top;
		r.right += rfull.left;
		r.bottom += rfull.top;

		// go from global back to client
		ScreenToClient(hwnd, (LPPOINT)&r);
		ScreenToClient(hwnd, ((LPPOINT)&r) + 1);


		HBRUSH br = (HBRUSH) GetStockObject(BLACK_BRUSH);
		GetClientRect(hwnd, &clientRect);

		// left black box
		drawRect.left = clientRect.left;
		drawRect.right = r.left;
		drawRect.top = clientRect.top;
		drawRect.bottom = clientRect.bottom;
		FillRect(p.hdc, &drawRect, br);

		// right black box
		drawRect.left = r.right;
		drawRect.right = clientRect.right;
		drawRect.top = clientRect.top;
		drawRect.bottom = clientRect.bottom;
		FillRect(p.hdc, &drawRect, br);

		// top black box
		drawRect.left = clientRect.left;
		drawRect.right = clientRect.right;
		drawRect.top = clientRect.top;
		drawRect.bottom = r.top;
		FillRect(p.hdc, &drawRect, br);

		// bottom black box
		drawRect.left = clientRect.left;
		drawRect.right = clientRect.right;
		drawRect.top = r.bottom;
		drawRect.bottom = clientRect.bottom;
		FillRect(p.hdc, &drawRect, br);

		LOGBRUSH lb = {BS_SOLID, (COLORREF)overlay_color, };
		br = CreateBrushIndirect(&lb);

		FillRect(p.hdc, &r, br);
		DeleteObject(br);
	}

	EndPaint(hwnd, &p);

	return 1;
}

void OverlayVideoOutput::displayFrame(const char * /*buf*/, int size, int time)
{
	if (m_closed)
		return;
	DDSURFACEDESC dd = {sizeof(dd), };
	//CT> vsync wait not used anymore
	//if (config_video.vsync()) lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0);
	HRESULT result;
	//if ((result=lpddsOverlay->Lock(NULL,&dd,DDLOCK_WAIT,NULL)) != DD_OK) {
	if ((result = lpBackBuffer->Lock(NULL, &dd, DDLOCK_WAIT, NULL)) != DD_OK)
	{
		if (result == DDERR_SURFACELOST) needchange = 1;
		return ;
	}
	if (type == VIDEO_MAKETYPE('Y', 'V', '1', '2'))
	{
		const YV12_PLANES *planes = (YV12_PLANES *)frame;
		if (uyvy_output)
			YV12_to_UYVY((unsigned char*)dd.lpSurface, planes, dd.lPitch, width, height, flip);
		else if (yuy2_output)
			YV12_to_YUY2((unsigned char*)dd.lpSurface, planes, dd.lPitch, width, height, flip);
		else
			YV12_to_YV12((unsigned char*)dd.lpSurface, planes, dd.lPitch, width, height, flip);
	}
	else if (type == VIDEO_MAKETYPE('Y', 'U', 'Y', '2'))
	{
		if (yuy2_output)
			YUY2_to_YUY2((unsigned char*)dd.lpSurface, (const char *)frame, dd.lPitch, width, height, flip);
		else if (uyvy_output)
			YUY2_to_UYVY((unsigned char*)dd.lpSurface, (const char *)frame, dd.lPitch, width, height, flip);
		else
			YUY2_to_YUY2((unsigned char*)dd.lpSurface, (const char *)frame, dd.lPitch, width, height, flip); // is this right?
	}
	else if (type == VIDEO_MAKETYPE('U', 'Y', 'V', 'Y'))
	{
		if (yuy2_output)
			YUY2_to_UYVY((unsigned char*)dd.lpSurface, (const char *)frame, dd.lPitch, width, height, flip);
		else
			YUY2_to_YUY2((unsigned char*)dd.lpSurface, (const char *)frame, dd.lPitch, width, height, flip);
	}

	lpBackBuffer->Unlock(&dd);
	lpddsOverlay->Flip(lpBackBuffer, DDFLIP_WAIT);
}

void OverlayVideoOutput::drawSubtitle(SubsItem *item) 
{
}

void OverlayVideoOutput::resetSubtitle()
{
}
