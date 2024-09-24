#include <ddraw.h>
#include <multimon.h>
#include "main.h"
#include "vid_subs.h"
#include "vid_ddraw.h"
#include "directdraw.h"
#include "WinampAttributes.h"
#include "../nsutil/image.h"

DDrawVideoOutput ddrawVideo;
#define INIT_DIRECTDRAW_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))

DDrawVideoOutput::DDrawVideoOutput()
{
	lpDD = NULL;
	lpddsOverlay = NULL;
	lastresizerect.bottom = 0;
	lastresizerect.top = 0;
	lastresizerect.left = 0;
	lastresizerect.right = 0;

	lpddsPrimary = NULL;
	lpddsClipper = NULL;
	lpddsSTTemp = NULL;

	width = height = flip = 0;
	type = VIDEO_MAKETYPE('Y', 'V', '1', '2');
	uDestSizeAlign = uSrcSizeAlign = 0;
	dwUpdateFlags = DDOVER_SHOW | DDOVER_KEYDESTOVERRIDE;
	m_depth = 16;

	initing = false;
	needchange = 0;
	m_palette = NULL;
	m_lastsubtitle = NULL;
	sttmp_w = sttmp_h = 0;
	subFont = NULL;
	m_sub_needremeasure = 0;
	m_fontsize = 0;
	memset(&winRect, 0, sizeof(winRect));
}

void DDrawVideoOutput::close()
{
	if (lpddsSTTemp) lpddsSTTemp->Release(); lpddsSTTemp = 0;
	if (lpddsPrimary) lpddsPrimary->Release(); lpddsPrimary = 0;
	if (lpddsOverlay) lpddsOverlay->Release(); lpddsOverlay = 0;
	if (lpddsClipper) lpddsClipper->Release(); lpddsClipper = 0;
	if (lpDD) lpDD->Release();	lpDD = 0;// BU added NULL check in response to talkback
	if (subFont) DeleteObject(subFont); subFont = 0;
//	 removeFullScreen();
}
DDrawVideoOutput::~DDrawVideoOutput()
{
	DDrawVideoOutput::close();
}

void DDrawVideoOutput::drawSubtitle(SubsItem *item)
{
	m_lastsubtitle = item;
	m_sub_needremeasure = 1;
}

int DDrawVideoOutput::create(HWND parent, VideoAspectAdjuster *_adjuster, int w, int h, unsigned int ptype, int flipit, double aspectratio)
{
	needchange = 0;
	this->parent = parent;
	m_lastsubtitle = NULL;
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

	if (_DirectDrawCreate)
	{
		if (!foundGUID) _DirectDrawCreate(NULL, &lpDD, NULL);
		else _DirectDrawCreate(&m_devguid, &lpDD, NULL);
	}

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
	HRESULT ddrval = lpDD->CreateSurface(&ddsd, &lpddsPrimary, NULL);

	if (FAILED(ddrval) || !lpddsPrimary)
	{
		initing = false;
		return 0;
	}

	HRESULT v = -1;
	DDSURFACEDESC DDsd = {sizeof(DDsd), };
	lpddsPrimary->GetSurfaceDesc(&ddsd);
	DDsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT; //create the surface at screen depth
	DDsd.dwWidth = w;
	DDsd.dwHeight = h;
	DDsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_OFFSCREENPLAIN;
	if (config_video_ddraw) v = lpDD->CreateSurface(&DDsd, &lpddsOverlay, NULL);
	if (!config_video_ddraw || FAILED(v))
	{
		// fall back to system memory if video mem doesn't work
		DDsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN;
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
		lpddsClipper = NULL;
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

bool DDrawVideoOutput::Paint(HWND hwnd)
{
	RECT r;
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


	int needst = 0;


	SubsItem *mlst = m_lastsubtitle;
	if (mlst)
	{
		int curw = r.right - r.left, curh = r.bottom - r.top;
		if (!lpddsSTTemp || sttmp_w != curw || sttmp_h != curh)
		{
			if (lpddsSTTemp) lpddsSTTemp->Release();
			lpddsSTTemp = 0;

			HRESULT v = -1;
			DDSURFACEDESC DDsd = {sizeof(DDsd), };
			DDSURFACEDESC ddsd;
			INIT_DIRECTDRAW_STRUCT(ddsd);
			ddsd.dwFlags = DDSD_CAPS;
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
			lpddsPrimary->GetSurfaceDesc(&ddsd);
			DDsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT; //create the surface at screen depth
			DDsd.dwWidth = sttmp_w = curw;
			DDsd.dwHeight = sttmp_h = curh;
			DDsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
			if (config_video_ddraw) v = lpDD->CreateSurface(&DDsd, &lpddsSTTemp, NULL);
			if (!config_video_ddraw || FAILED(v))
			{
				// fall back to system memory if video mem doesn't work
				DDsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
				lpDD->CreateSurface(&DDsd, &lpddsSTTemp, NULL);
			}
			m_sub_needremeasure = 1;
		}
		if (lpddsSTTemp) needst = 1;
	}

	if (needst)
	{
		HDC tmpdc = NULL;
		if (!config_video_ddraw || lpddsSTTemp->Blt(NULL, lpddsOverlay, NULL, DDBLT_WAIT, 0) != DD_OK)
		{
			// as a last resort, BitBlt().
			HDC tmpdc2;
			if (lpddsOverlay->GetDC(&tmpdc2) == DD_OK)
			{
				if (lpddsSTTemp->GetDC(&tmpdc) == DD_OK)
				{
					BitBlt(tmpdc, 0, 0, sttmp_w, sttmp_h, tmpdc2, 0, 0, SRCCOPY);
				}
			}
		}

		if (mlst && (tmpdc || lpddsSTTemp->GetDC(&tmpdc) == DD_OK))
		{
			int m_lastsubxp = mlst->xPos;
			int m_lastsubyp = mlst->yPos;

			RECT oldwinRect = winRect;
			GetClientRect(hwnd, &winRect);
			if (!subFont || ((winRect.bottom - winRect.top) != (oldwinRect.bottom - oldwinRect.top)) || m_fontsize != mlst->fontSize)
			{
				if (subFont) DeleteObject(subFont);
				m_fontsize = mlst->fontSize;
				subFont = CreateFontA(14 + m_fontsize + 18 * (winRect.bottom - winRect.top) / 768, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
			}

			HGDIOBJ oldobj = SelectObject(tmpdc, subFont);

			int centerflags = 0;
			if (m_lastsubxp < 127) centerflags |= DT_LEFT;
			else if (m_lastsubxp > 127) centerflags |= DT_RIGHT;
			else centerflags |= DT_CENTER;

			if (m_lastsubyp < 127) centerflags |= DT_TOP;
			else if (m_lastsubyp > 127) centerflags |= DT_BOTTOM;

			if (m_sub_needremeasure && mlst)
			{
				subRect = r;
				subRect.bottom -= subRect.top;
				subRect.right -= subRect.left;
				subRect.top = subRect.left = 0;

				SIZE s;
				GetTextExtentPoint32A(tmpdc, mlst->text, lstrlenA(mlst->text), &s);

				// calcul for multiline text
				const char *p = mlst->text;
				int n = 0;
				while (*p != 0) if (*p++ == '\n') n++;
				if (n) s.cy *= (n + 1);

				if (m_lastsubxp > 127) // towards the right
				{
					subRect.right -= ((subRect.right - subRect.left) * (255 - m_lastsubxp)) / 256;
				}
				else if (m_lastsubxp < 127)
				{
					subRect.left += ((subRect.right - subRect.left) * m_lastsubxp) / 256;
				}

				subRect.top += ((subRect.bottom - s.cy - subRect.top) * m_lastsubyp) / 255;

				subRect.bottom = subRect.top + s.cy;
			}

			SetBkMode(tmpdc, TRANSPARENT);

			// draw outline
			SetTextColor(tmpdc, RGB(0, 0, 0));
			int y = 1;
			int x = 1;
			RECT r2 = {subRect.left + x, subRect.top + y, subRect.right + x, subRect.bottom + y};
			if (mlst)
			{
				DrawTextA(tmpdc, mlst->text, -1, &r2, centerflags | DT_NOCLIP | DT_NOPREFIX);
				// draw text
				SetTextColor(tmpdc, RGB(mlst->colorRed, mlst->colorGreen, mlst->colorBlue));
				DrawTextA(tmpdc, mlst->text, -1, &subRect, centerflags | DT_NOCLIP | DT_NOPREFIX);
			}
			SelectObject(tmpdc, oldobj);
			lpddsSTTemp->ReleaseDC(tmpdc);
		}
		if (!config_video_ddraw || lpddsPrimary->Blt(&r, lpddsSTTemp, pSrcRect, DDBLT_WAIT, 0) != DD_OK)
		{
			// as a last resort, BitBlt().
			if (lpddsOverlay->GetDC(&inhdc) != DD_OK)
			{
				needchange = 1;
				return false;
			}
			if (lpddsPrimary->GetDC(&hdc) != DD_OK)
			{
				lpddsOverlay->ReleaseDC(inhdc); inhdc = NULL;
				needchange = 1;
				return false;
			}

			int src_w = width;
			int src_h = pSrcRect ? (pSrcRect->bottom - pSrcRect->top) : height;
			if (r.right - r.left == src_w && r.bottom - r.top == src_h)
				BitBlt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, inhdc, 0, 0, SRCCOPY);
			else
				StretchBlt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, inhdc, 0, 0, src_w, src_h, SRCCOPY);
		}
	}
	else
	{
		if (!config_video_ddraw ||  lpddsPrimary->Blt(&r, lpddsOverlay, pSrcRect, DDBLT_WAIT, 0) != DD_OK)
		{
			// as a last resort, BitBlt().
			if (lpddsOverlay->GetDC(&inhdc) != DD_OK)
			{
				needchange = 1;
				return false;
			}
			if (lpddsPrimary->GetDC(&hdc) != DD_OK)
			{
				lpddsOverlay->ReleaseDC(inhdc); inhdc = NULL;
				needchange = 1;
				return false;
			}

			int src_w = width;
			int src_h = pSrcRect ? (pSrcRect->bottom - pSrcRect->top) : height;
			if (r.right - r.left == src_w && r.bottom - r.top == src_h)
				BitBlt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, inhdc, 0, 0, SRCCOPY);
			else
				StretchBlt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, inhdc, 0, 0, src_w, src_h, SRCCOPY);
		}
	}

	if (hdc)
	{
		lpddsPrimary->ReleaseDC(hdc); hdc = NULL;
	}
	if (inhdc)
	{
		lpddsOverlay->ReleaseDC(inhdc); inhdc = NULL;
	}

	return true;
}

int DDrawVideoOutput::onPaint(HWND hwnd)
{
	// reblit the last frame
	if (lpddsPrimary)
	{
		PAINTSTRUCT p;
		BeginPaint(hwnd, &p);

		RECT r;

		GetClientRect(hwnd, &r);

		HRGN hrgn = CreateRectRgnIndirect(&r);
		HBRUSH b = (HBRUSH)GetStockObject(BLACK_BRUSH);
		FillRgn(p.hdc, hrgn, b);
		DeleteObject(b);
		DeleteObject(hrgn);

		if (Paint(hwnd))
		{
			EndPaint(hwnd, &p);

			return 1;
		}
		EndPaint(hwnd, &p);
	}
	return 0;

}

bool DDrawVideoOutput::LockSurface(DDSURFACEDESC *dd)
{
	for (;;Sleep(0))
	{
		HRESULT hr = lpddsOverlay->Lock(0, dd, DDLOCK_WAIT | DDLOCK_WRITEONLY, 0);

		if (dd->lpSurface)
			break;

		if (hr == DDERR_SURFACELOST)
		{
			lpddsPrimary->Restore();
			lpddsOverlay->Restore();
			hr = lpddsOverlay->Lock(0, dd, DDLOCK_WAIT | DDLOCK_WRITEONLY, 0);
			if (hr == DDERR_SURFACELOST)
				return false;
		}
		else if (hr != DDERR_WASSTILLDRAWING)
			return false;
	}

	return true;
}

void DDrawVideoOutput::displayFrame(const char *buf, int size, int time)
{
	DDSURFACEDESC dd = {sizeof(dd), };
	if (config_video_vsync2) lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);

	if (!LockSurface(&dd))
	{
		needchange = 1;
		return ;
	}
	if (type == VIDEO_MAKETYPE('Y', 'V', '1', '2'))
	{
		const YV12_PLANES *planes = (YV12_PLANES *)buf;
		// convert yv12 to rgb
		int bytes = m_depth >> 3;
		if (m_depth == 15) bytes = 2;
		int i, j, y00, y01, y10, y11, u, v;
		unsigned char *pY = (unsigned char *)planes->y.baseAddr;
		unsigned char *pU = (unsigned char *)planes->u.baseAddr;
		unsigned char *pV = (unsigned char *)planes->v.baseAddr;
		unsigned char *pOut = (unsigned char*)dd.lpSurface;
		const int rvScale = (int)(2.017 * 65536.0);  //91881;
		const int gvScale = - (int)(0.392 * 65536.0);  // -22553;
		const int guScale = - (int)(0.813 * 65536.0);  // -46801;
		const int buScale = (int)(1.596 * 65536.0);  //116129;
		const int yScale = (int)(1.164 * 65536.0);   //(1.164*65536.0);
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
				const char *a = buf;
				char *b = (char *)dd.lpSurface;
				int l = width * 4, l2 = dd.lPitch;
				int ladj = l;
				if (flip)
				{
					a += l * (height - 1); ladj = -ladj;
				}
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
				const char *a = buf;
				char *b = (char *)dd.lpSurface;
				int l = width * 4, l2 = dd.lPitch;
				int ladj = l;
				if (flip)
				{
					a += l * (height - 1); ladj = -ladj;
				}
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
				const char *a = buf;
				char *b = (char *)dd.lpSurface;
				int l = width * 4, l2 = dd.lPitch;
				int ladj = l;
				if (flip)
				{
					a += l * (height - 1); ladj = -ladj;
				}
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
				if (flip)
				nsutil_image_CopyFlipped_U8((uint8_t *)dd.lpSurface, dd.lPitch, (const uint8_t *)buf, width*4, width, height);
			else
				nsutil_image_Copy_U8((uint8_t *)dd.lpSurface, dd.lPitch, (const uint8_t *)buf, width*4, width, height);
			}
			break;
		}
	}
	else if (type == VIDEO_MAKETYPE('Y', 'U', 'Y', '2') || type == VIDEO_MAKETYPE('U', 'Y', 'V', 'Y'))
	{
		//const char *a = buf;
		char *b = (char *)dd.lpSurface;
		int /*l = width * 2, */l2 = dd.lPitch;
		if (flip)
		{
			b += (height - 1) * l2;
			l2 = -l2;
		}
		switch (m_depth)
		{
			case 15:
			{
				// yuy2->rgb16 (555) conversion
				unsigned char *src = (unsigned char *)buf;
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

						unsigned char b = LIMIT(yy + ub);
						unsigned char g = LIMIT(yy - ug - vg);
						unsigned char r = LIMIT(yy + vr);
						*(dst++) = ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);

						py += 2;
						if ((col & 1) == 1)
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
				unsigned char *src = (unsigned char *)buf;
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

						unsigned char b = LIMIT(yy + ub);
						unsigned char g = LIMIT(yy - ug - vg);
						unsigned char r = LIMIT(yy + vr);
						*(dst++) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

						py += 2;
						if ((col & 1))
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
				unsigned char *src = (unsigned char *)buf;
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

						*(dst++) = LIMIT(yy + ub);
						*(dst++) = LIMIT(yy - ug - vg);
						*(dst++) = LIMIT(yy + vr);

						py += 2;
						if ((col & 1) == 1)
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
				unsigned char *src = (unsigned char *)buf;
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

						*dst++ = LIMIT(yy + ub);  // b
						*dst++ = LIMIT(yy - ug - vg); // g
						*dst++ = LIMIT(yy + vr); // r
						dst++;

						py += 2;
						if ((col & 1) == 1)
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
							if (flip)
				nsutil_image_ConvertFlipped_RGB24_RGB32((RGB32 *)dd.lpSurface, dd.lPitch, (const uint8_t *)buf, width*3, width, height);
			else
				nsutil_image_Convert_RGB24_RGB32((RGB32 *)dd.lpSurface, dd.lPitch, (const uint8_t *)buf, width*3, width, height);

			}
			break;
		}
	}
	else if (type == VIDEO_MAKETYPE('R', 'G', 'B', '8') && m_palette)
	{
		unsigned char *d = (unsigned char *)dd.lpSurface;
		int pitch = dd.lPitch;
		unsigned char *src = (unsigned char *)buf;
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
				case 32:
								if (flip)
				nsutil_image_PaletteFlipped_RGB32((RGB32 *)dd.lpSurface, dd.lPitch, (const uint8_t *)buf, width, width, height, (RGB32 *)m_palette);
			else
				nsutil_image_Palette_RGB32((RGB32 *)dd.lpSurface, dd.lPitch, (const uint8_t *)buf, width, width, height, (RGB32 *)m_palette);
				break;
			}
			d += pitch;
			src -= newwidth;
		}
	}

	lpddsOverlay->Unlock(&dd);


	HWND hwnd = this->parent;
	if (!IsWindow(hwnd)) return ;

	// if(GetParent(hwnd)) hwnd=GetParent(hwnd);

	Paint(hwnd);
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
	m_lastsubtitle = 0;
}



void DDrawVideoOutput::Refresh()
{
	// do nothing, we'll refresh on the next frame
}