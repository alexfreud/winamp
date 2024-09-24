#include "main.h"
#include "vid_ddraw.h"
#include "directdraw.h"

	void *frame;
	int width, height, flip;
	int needchange;
	unsigned int type;
	LPDIRECTDRAW	lpDD;
	LPDIRECTDRAWSURFACE lpddsOverlay, lpddsPrimary;
	DDCAPS capsDrv;
	unsigned int uDestSizeAlign, uSrcSizeAlign;
	DWORD dwUpdateFlags;
	RECT rs, rd;
	RECT lastresizerect;
	bool initing;

	LPDIRECTDRAWCLIPPER lpddsClipper;
	DDPIXELFORMAT m_ddpf;
	int m_depth;
	RGBQUAD *m_palette;

	RECT winRect;
	RECT m_monRect;

void getViewport(RECT *r, HWND wnd, int full, RECT *sr);
#define INIT_DIRECTDRAW_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x)) 
DDrawVideoOutput::DDrawVideoOutput() : frame(0), width(0), height(0),
									   flip(0), type(0), uDestSizeAlign(0),
									   uSrcSizeAlign(0), dwUpdateFlags(0),
									   m_depth(0)
{
	lpDD = NULL;
	lpddsOverlay = NULL;
	lastresizerect.bottom = 0;
	lastresizerect.top = 0;
	lastresizerect.left = 0;
	lastresizerect.right = 0;

	lpddsPrimary = NULL;
	lpddsClipper = NULL;

	initing = false;
	needchange = 0;
	m_palette = NULL;
	memset(&winRect, 0, sizeof(winRect));
}

void DDrawVideoOutput::close()
{
	//  LPDIRECTDRAWSURFACE o=lpddsOverlay;
	lpddsOverlay = NULL;
	//  if(o) o->Release();
	//if(lpddsPrimary) lpddsPrimary->Release();
	//  if(lpddsClipper) lpddsClipper->Release();
	if (lpDD) lpDD->Release();	lpDD = 0;// BU added NULL check in response to talkback
//	 removeFullScreen();
}

DDrawVideoOutput::~DDrawVideoOutput()
{
	DDrawVideoOutput::close();
}

void DDrawVideoOutput::drawSubtitle(SubsItem *item)
{
}

int DDrawVideoOutput::create(HWND parent, VideoAspectAdjuster *_adjuster, int w, int h, unsigned int ptype, int flipit, double aspectratio)
{
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
	HRESULT ddrval = lpDD->CreateSurface(&ddsd, &lpddsPrimary, NULL );

	if (FAILED(ddrval) || !lpddsPrimary)
	{
		initing=false;
		return 0;
	}

	HRESULT v = -1;
	DDSURFACEDESC DDsd = {sizeof(DDsd), };
	lpddsPrimary->GetSurfaceDesc(&ddsd);
	DDsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT; //create the surface at screen depth
	DDsd.dwWidth = w;
	DDsd.dwHeight = h;
	DDsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
	if (config_video.ddraw()) v = lpDD->CreateSurface(&DDsd, &lpddsOverlay, NULL);
	if (!config_video.ddraw() || FAILED(v))
	{
		// fall back to system memory if video mem doesn't work
		DDsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
		v = lpDD->CreateSurface(&DDsd, &lpddsOverlay, NULL);
	}
	if (FAILED(v))
	{
		// this video card sucks then :)
		lpddsOverlay = NULL;
		initing = false;
		return 0;
	}

	// get the depth
	m_depth = 8;
	INIT_DIRECTDRAW_STRUCT(m_ddpf);
	if (lpddsOverlay->GetPixelFormat(&m_ddpf) >= 0)
	{
		m_depth = m_ddpf.dwRGBBitCount;
		if (m_depth == 16 && m_ddpf.dwGBitMask == 0x03e0) m_depth = 15;
	}

	if (lpDD->CreateClipper(0, &lpddsClipper, NULL) != DD_OK)
	{
		lpddsOverlay = NULL;
		initing = false;
		return 0;
	}

	lpddsClipper->SetHWnd(0, hwnd);
	lpddsPrimary->SetClipper(lpddsClipper);
	initing = false;

	//get current monitor
	getViewport(&m_monRect, hwnd, 1, NULL);

	return 1;
}

int DDrawVideoOutput::onPaint(HWND hwnd) 
{
	return 0;
}

void DDrawVideoOutput::displayFrame(const char * /*buf*/, int size, int time)
{
	DDSURFACEDESC dd = {sizeof(dd), };
	if (config_video.vsync()) lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
	if (lpddsOverlay->Lock(NULL, &dd, DDLOCK_WAIT, NULL) != DD_OK)
	{
		needchange = 1;
		return ;
	}
	if (type == VIDEO_MAKETYPE('Y', 'V', '1', '2'))
	{
		const YV12_PLANES *planes = (YV12_PLANES *)frame;
		// convert yv12 to rgb
		int bytes = m_depth >> 3;
		if (m_depth == 15) bytes = 2;
		int i, j, y00, y01, y10, y11, u, v;
		unsigned char *pY = (unsigned char *)planes->y.baseAddr;
		unsigned char *pU = (unsigned char *)planes->u.baseAddr;
		unsigned char *pV = (unsigned char *)planes->v.baseAddr;
		unsigned char *pOut = (unsigned char*)dd.lpSurface;
		const int rvScale = (int) (2.017 * 65536.0); //91881;
		const int gvScale = - (int) (0.392 * 65536.0); // -22553;
		const int guScale = - (int) (0.813 * 65536.0); // -46801;
		const int buScale = (int) (1.596 * 65536.0); //116129;
		const int yScale = (int)( 1.164 * 65536.0 ); //(1.164*65536.0);
		int addOut = dd.lPitch * 2 - width * bytes;
		int yrb = planes->y.rowBytes;
		int addL = dd.lPitch;

		/* LIMIT: convert a 16.16 fixed-point value to a byte, with clipping. */
#define LIMIT(x) ((x)>0xffffff?0xff: ((x)<=0xffff?0:((x)>>16)))

		if (flip)
		{
			pOut += (dd.lPitch) * (height - 1);
			addOut = -dd.lPitch * 2 - width * bytes;
			addL = -addL;
		}

		for (j = 0; j <= height - 2; j += 2)
		{
			for (i = 0; i <= width - 2; i += 2)
			{
				y00 = *pY - 16;
				y01 = *(pY + 1) - 16;
				y10 = *(pY + yrb) - 16;
				y11 = *(pY + yrb + 1) - 16;
				u = (*pU++) - 128;
				v = (*pV++) - 128;

				{
					int r, g, b;

					g = guScale * v + gvScale * u;
					r = buScale * v;
					b = rvScale * u;

					y00 *= yScale; y01 *= yScale;
					y10 *= yScale; y11 *= yScale;

					switch (m_depth)
					{
					case 15:
						{
							unsigned short *rgb = (unsigned short *)pOut;
							rgb[0] = ((LIMIT(r + y00) >> 3) << 10) | ((LIMIT(g + y00) >> 3) << 5) | (LIMIT(b + y00) >> 3);
							rgb[1] = ((LIMIT(r + y01) >> 3) << 10) | ((LIMIT(g + y01) >> 3) << 5) | (LIMIT(b + y01) >> 3);
							rgb += addL / 2;
							rgb[0] = ((LIMIT(r + y10) >> 3) << 10) | ((LIMIT(g + y10) >> 3) << 5) | (LIMIT(b + y10) >> 3);
							rgb[1] = ((LIMIT(r + y11) >> 3) << 10) | ((LIMIT(g + y11) >> 3) << 5) | (LIMIT(b + y11) >> 3);
						}
						break;
					case 16:
						{
							unsigned short *rgb = (unsigned short *)pOut;
							rgb[0] = ((LIMIT(r + y00) >> 3) << 11) | ((LIMIT(g + y00) >> 2) << 5) | (LIMIT(b + y00) >> 3);
							rgb[1] = ((LIMIT(r + y01) >> 3) << 11) | ((LIMIT(g + y01) >> 2) << 5) | (LIMIT(b + y01) >> 3);
							rgb += addL / 2;
							rgb[0] = ((LIMIT(r + y10) >> 3) << 11) | ((LIMIT(g + y10) >> 2) << 5) | (LIMIT(b + y10) >> 3);
							rgb[1] = ((LIMIT(r + y11) >> 3) << 11) | ((LIMIT(g + y11) >> 2) << 5) | (LIMIT(b + y11) >> 3);
						}
						break;
					case 24:
						{
							unsigned char *rgb = pOut;
							/* Write out top two pixels */
							rgb[0] = LIMIT(b + y00); rgb[1] = LIMIT(g + y00); rgb[2] = LIMIT(r + y00);
							rgb[3] = LIMIT(b + y01); rgb[4] = LIMIT(g + y01); rgb[5] = LIMIT(r + y01);

							/* Skip down to next line to write out bottom two pixels */
							rgb += addL;
							rgb[0] = LIMIT(b + y10); rgb[1] = LIMIT(g + y10); rgb[2] = LIMIT(r + y10);
							rgb[3] = LIMIT(b + y11); rgb[4] = LIMIT(g + y11); rgb[5] = LIMIT(r + y11);
						}
						break;
					case 32:
						{
							unsigned char *rgb = pOut;
							/* Write out top two pixels */
							rgb[0] = LIMIT(b + y00); rgb[1] = LIMIT(g + y00); rgb[2] = LIMIT(r + y00);
							rgb[4] = LIMIT(b + y01); rgb[5] = LIMIT(g + y01); rgb[6] = LIMIT(r + y01);

							/* Skip down to next line to write out bottom two pixels */
							rgb += addL;
							rgb[0] = LIMIT(b + y10); rgb[1] = LIMIT(g + y10); rgb[2] = LIMIT(r + y10);
							rgb[4] = LIMIT(b + y11); rgb[5] = LIMIT(g + y11); rgb[6] = LIMIT(r + y11);
						}
						break;
					}
				}

				pY += 2;
				pOut += 2 * bytes;
			}
			pY += yrb + yrb - width;
			pU += planes->u.rowBytes - width / 2;
			pV += planes->v.rowBytes - width / 2;
			pOut += addOut;
		}
	}
	else if (type == VIDEO_MAKETYPE('R', 'G', '3', '2'))
	{
		//FUCKO: do we need to support 8bits depth?
		switch (m_depth)
		{
		case 15:
			{ // convert RGB32 -> RGB16 (555)
				const char *a = (const char *)frame;
				char *b = (char *)dd.lpSurface;
				int l = width * 4, l2 = dd.lPitch;
				int ladj = l;
				if (flip) { a += l * (height - 1); ladj = -ladj; }
				for (int i = 0;i < height;i++)
				{
					short *dest = (short *)b;
					int *src = (int *)a;
					for (int j = 0;j < width;j++)
					{
						int c = *(src++);
						int r = c >> 16;
						int g = (c >> 8) & 0xff;
						int b = (c) & 0xff;
						*(dest++) = ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
					}
					a += ladj; b += l2;
				}
			}
			break;
		case 16:
			{ // convert RGB32 -> RGB16
				//FUCKO: this assumes 565
				const char *a = (const char *)frame;
				char *b = (char *)dd.lpSurface;
				int l = width * 4, l2 = dd.lPitch;
				int ladj = l;
				if (flip) { a += l * (height - 1); ladj = -ladj; }
				for (int i = 0;i < height;i++)
				{
					short *dest = (short *)b;
					int *src = (int *)a;
					for (int j = 0;j < width;j++)
					{
						//FUCKO: optimize here
						int c = *(src++);
						int r = c >> 16;
						int g = (c >> 8) & 0xff;
						int b = (c) & 0xff;
						*(dest++) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
					}
					a += ladj; b += l2;
				}
			}
			break;
		case 24:
			{ // convert RGB32 -> RGB24
				const char *a = (const char *)frame;
				char *b = (char *)dd.lpSurface;
				int l = width * 4, l2 = dd.lPitch;
				int ladj = l;
				if (flip) { a += l * (height - 1); ladj = -ladj; }
				for (int i = 0;i < height;i++)
				{
					char *dest = (char *)b;
					int *src = (int *)a;
					for (int j = 0;j < width;j++)
					{
						//FUCKO: optimize here
						int c = *(src++);
						int r = c >> 16;
						int g = (c >> 8) & 0xff;
						int b = (c) & 0xff;
						*dest++ = b;
						*dest++ = g;
						*dest++ = r;
					}
					a += ladj; b += l2;
				}
			}
			break;
		case 32:
			{ // straight RGB32 copy
				const char *a = (const char *)frame;
				char *b = (char *)dd.lpSurface;
				int l = width * 4, l2 = dd.lPitch;
				int ladj = l;
				if (flip) { a += l * (height - 1); ladj = -ladj; }
				for (int i = 0;i < height;i++)
				{
					memcpy(b, a, l);
					a += ladj; b += l2;
				}
			}
			break;
		}
	}
	else if (type == VIDEO_MAKETYPE('Y', 'U', 'Y', '2') || type == VIDEO_MAKETYPE('U', 'Y', 'V', 'Y'))
	{
		//const char *a = (const char *)frame;
		//char *b = (char *)dd.lpSurface;
		int /*l = width * 2, */l2 = dd.lPitch;
		if (flip)
		{
			//b += (height - 1) * l2;
			l2 = -l2;
		}
		switch (m_depth)
		{
		case 15:
			{
				// yuy2->rgb16 (555) conversion
				unsigned char *src = (unsigned char *)frame;
				unsigned short *dst = (unsigned short *)dd.lpSurface;
				int line, col; //, linewidth;
				int y, yy;
				int u, v;
				int vr, ug, vg, ub;
				unsigned char *py, *pu, *pv;

				//linewidth = width - (width >> 1);
				py = src;
				pu = src + 1;
				pv = src + 3;

				int pitchadd = dd.lPitch / 2 - width;

				for (line = 0; line < height; line++)
				{
					for (col = 0; col < width; col++)
					{
#undef LIMIT
 #define LIMIT(x)  ( (x) > 0xffff ? 0xff : ( (x) <= 0xff ? 0 : ( (x) >> 8 ) ) )

						y = *py;
						yy = y << 8;
						u = *pu - 128;
						ug = 88 * u;
						ub = 454 * u;
						v = *pv - 128;
						vg = 183 * v;
						vr = 359 * v;

						unsigned char b = LIMIT(yy + ub );
						unsigned char g = LIMIT(yy - ug - vg);
						unsigned char r = LIMIT(yy + vr);
						*(dst++) = ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);

						py += 2;
						if ( (col & 1) == 1)
						{
							pu += 4; // skip yvy every second y
							pv += 4; // skip yuy every second y
						}
					} // ..for col
					dst += pitchadd;
				} /* ..for line */
			}
			break;
		case 16:
			{
				// yuy2->rgb16 conversion
				//FUCKO: only supports 565
				unsigned char *src = (unsigned char *)frame;
				unsigned short *dst = (unsigned short *)dd.lpSurface;
				int line, col; //, linewidth;
				int y, yy;
				int u, v;
				int vr, ug, vg, ub;
				unsigned char *py, *pu, *pv;

				//linewidth = width - (width >> 1);
				py = src;
				pu = src + 1;
				pv = src + 3;

				int pitchadd = dd.lPitch / 2 - width;

				for (line = 0; line < height; line++)
				{
					for (col = 0; col < width; col++)
					{
#undef LIMIT
 #define LIMIT(x)  ( (x) > 0xffff ? 0xff : ( (x) <= 0xff ? 0 : ( (x) >> 8 ) ) )

						y = *py;
						yy = y << 8;
						u = *pu - 128;
						ug = 88 * u;
						ub = 454 * u;
						v = *pv - 128;
						vg = 183 * v;
						vr = 359 * v;

						unsigned char b = LIMIT(yy + ub );
						unsigned char g = LIMIT(yy - ug - vg);
						unsigned char r = LIMIT(yy + vr);
						*(dst++) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

						py += 2;
						if ( (col & 1) )
						{
							pu += 4; // skip yvy every second y
							pv += 4; // skip yuy every second y
						}
					} // ..for col
					dst += pitchadd;
				} /* ..for line */
			}
			break;
		case 24:
			{
				// yuy2->rgb24 conversion
				unsigned char *src = (unsigned char *)frame;
				unsigned char *dst = (unsigned char *)dd.lpSurface;
				int line, col; //, linewidth;
				int y, yy;
				int u, v;
				int vr, ug, vg, ub;
				unsigned char *py, *pu, *pv;

				//linewidth = width - (width >> 1);
				py = src;
				pu = src + 1;
				pv = src + 3;

				int pitchadd = dd.lPitch - (width * 3);

				for (line = 0; line < height; line++)
				{
					for (col = 0; col < width; col++)
					{
#undef LIMIT
 #define LIMIT(x)  ( (x) > 0xffff ? 0xff : ( (x) <= 0xff ? 0 : ( (x) >> 8 ) ) )

						y = *py;
						yy = y << 8;
						u = *pu - 128;
						ug = 88 * u;
						ub = 454 * u;
						v = *pv - 128;
						vg = 183 * v;
						vr = 359 * v;

						*(dst++) = LIMIT(yy + ub );
						*(dst++) = LIMIT(yy - ug - vg);
						*(dst++) = LIMIT(yy + vr);

						py += 2;
						if ( (col & 1) == 1)
						{
							pu += 4; // skip yvy every second y
							pv += 4; // skip yuy every second y
						}
					} // ..for col
					dst += pitchadd;
				} /* ..for line */
			}
			break;
		case 32:
			{
				// yuy2->rgb32 conversion
				unsigned char *src = (unsigned char *)frame;
				unsigned char *dst = (unsigned char *)dd.lpSurface;
				int line, col; //, linewidth;
				int y, yy;
				int u, v;
				int vr, ug, vg, ub;
				unsigned char *py, *pu, *pv;

				//linewidth = width - (width >> 1);
				py = src;
				pu = src + 1;
				pv = src + 3;

				int pitchadd = dd.lPitch - (width * 4);

				for (line = 0; line < height; line++)
				{
					for (col = 0; col < width; col++)
					{
#undef LIMIT
 #define LIMIT(x)  ( (x) > 0xffff ? 0xff : ( (x) <= 0xff ? 0 : ( (x) >> 8 ) ) )

						y = *py;
						yy = y << 8;
						u = *pu - 128;
						ug = 88 * u;
						ub = 454 * u;
						v = *pv - 128;
						vg = 183 * v;
						vr = 359 * v;

						*dst++ = LIMIT(yy + ub ); // b
						*dst++ = LIMIT(yy - ug - vg); // g
						*dst++ = LIMIT(yy + vr); // r
						dst++;

						py += 2;
						if ( (col & 1) == 1)
						{
							pu += 4; // skip yvy every second y
							pv += 4; // skip yuy every second y
						}
					} // ..for col
					dst += pitchadd;
				} /* ..for line */
			}
			break;
		}
	}
	else if (type == VIDEO_MAKETYPE('R', 'G', '2', '4'))
	{
		//FUCKO: only ->RGB32 conversion supported
		switch (m_depth)
		{
		case 32:
			{
				const char *a = (const char *)frame;
				char *b = (char *)dd.lpSurface;
				int l = width, l2 = dd.lPitch;
				int ladj = l * 3;
				if (!flip) ladj = 0;
			if (flip) { a += (l * 3) * (height - 1); ladj = -(ladj + l * 3); }
				l2 -= l * 4;
				for (int i = 0;i < height;i++)
				{
					//memcpy(b,a,l);
					for (int j = 0;j < l;j++)
					{
						b[0] = a[0];
						b[1] = a[1];
						b[2] = a[2];
						b += 4; a += 3;
					}
					a += ladj; b += l2;
				}
			}
			break;
		}
	}
	else if (type == VIDEO_MAKETYPE('R', 'G', 'B', '8') && m_palette)
	{
		unsigned char *d = (unsigned char *)dd.lpSurface;
		int pitch = dd.lPitch;
		unsigned char *src = (unsigned char *)frame;
		int newwidth = (width + 3) & 0xfffc;
		src += newwidth * height - 1;
		for (int j = 0;j < height;j++)
		{
			switch (m_depth)
			{
			case 15:
			case 16:
				{
					unsigned short *dest = (unsigned short *)d;
					for (int i = 0;i < newwidth;i++)
					{
						unsigned char c = src[ -newwidth + 1 + i];
						RGBQUAD *rgb = &m_palette[c];
						switch (m_depth)
						{
						case 15: *(dest++) = ((rgb->rgbRed >> 3) << 10) | ((rgb->rgbGreen >> 3) << 5) | (rgb->rgbBlue >> 3); break;
						case 16: *(dest++) = ((rgb->rgbRed >> 3) << 11) | ((rgb->rgbGreen >> 2) << 5) | (rgb->rgbBlue >> 3); break;
						}
					}
				}
				break;
			case 24:
			case 32:
				{
					unsigned char *dest = d;
					for (int i = 0;i < newwidth;i++)
					{
						unsigned char c = src[ -newwidth + 1 + i];
						RGBQUAD *rgb = &m_palette[c];
						*dest++ = rgb->rgbBlue;
						*dest++ = rgb->rgbGreen;
						*dest++ = rgb->rgbRed;
						if (m_depth == 32) dest++;
					}
				}
				break;
			}
			d += pitch;
			src -= newwidth;
		}
	}

	lpddsOverlay->Unlock(&dd);


	RECT r;
	HWND hwnd = this->parent;
	if (!IsWindow(hwnd)) return ;

	// if(GetParent(hwnd)) hwnd=GetParent(hwnd);

	GetClientRect(hwnd, &r);
	RECT fullr = r;
	adjuster->adjustAspect(r);
	if (r.left != lastresizerect.left || r.right != lastresizerect.right || r.top != lastresizerect.top ||
	    r.bottom != lastresizerect.bottom)
	{
		if (r.left != 0)
		{
			RECT tmp = {0, 0, r.left, fullr.bottom};
			InvalidateRect(hwnd, &tmp, TRUE);
		}

		if (r.right != fullr.right)
		{
			RECT tmp = {r.right, 0, fullr.right, fullr.bottom};
			InvalidateRect(hwnd, &tmp, TRUE);
		}
		if (r.top != 0)
		{
			RECT tmp = {r.left, 0, r.right, r.top};
			InvalidateRect(hwnd, &tmp, TRUE);
		}
		if (r.bottom != fullr.bottom)
		{
			RECT tmp = {r.left, r.bottom, r.right, fullr.bottom};
			InvalidateRect(hwnd, &tmp, TRUE);
		}

		lastresizerect = r;
	}

	ClientToScreen(hwnd, (LPPOINT)&r);
	ClientToScreen(hwnd, ((LPPOINT)&r) + 1);

	// transform coords from windows desktop coords (where 0,0==upper-left corner of box encompassing all monitors)
	// to the coords for the monitor we're displaying on:
	r.left -= m_mon_x;
	r.right -= m_mon_x;
	r.top -= m_mon_y;
	r.bottom -= m_mon_y;

	HDC hdc = NULL;
	HDC inhdc = NULL;

	RECT *pSrcRect = NULL;

	{
		if (!config_video.ddraw() || lpddsPrimary->Blt(&r, lpddsOverlay, pSrcRect, DDBLT_WAIT, 0) != DD_OK)
		{
			// as a last resort, BitBlt().
			if (lpddsOverlay->GetDC(&inhdc) != DD_OK)
			{
				needchange = 1;
				return ;
			}
			if (lpddsPrimary->GetDC(&hdc) != DD_OK)
			{
				lpddsOverlay->ReleaseDC(inhdc); inhdc = NULL;
				needchange = 1;
				return ;
			}

			int src_w = width;
			int src_h = pSrcRect ? (pSrcRect->bottom - pSrcRect->top) : height;
			if (r.right - r.left == src_w && r.bottom - r.top == src_h)
				BitBlt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, inhdc, 0, 0, SRCCOPY);
			else
				StretchBlt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, inhdc, 0, 0, src_w, src_h, SRCCOPY);
		}
	}

	if (hdc) { lpddsPrimary->ReleaseDC(hdc); hdc = NULL; }
	if (inhdc) { lpddsOverlay->ReleaseDC(inhdc); inhdc = NULL; }
}

void DDrawVideoOutput::timerCallback()
{
	//check if the video has been dragged to another monitor
	RECT curRect;

	getViewport(&curRect, parent, 1, NULL);
	if (memcmp(&curRect, &m_monRect, sizeof(RECT)))
		needchange = 1;
}

void DDrawVideoOutput::resetSubtitle()
{
}

void DDrawVideoOutput::Refresh()
{
	// do nothing, we'll refresh on the next frame
}