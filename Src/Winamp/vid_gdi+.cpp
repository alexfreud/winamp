#include "main.h"
#include "vid_subs.h"
#include "vid_gdi+.h"
#include "WinampAttributes.h"
#include "../nu/AutoWide.h"
#include "../nsutil/image.h"

GDIPVideoOutput gdiplusVideo;

static void colorspace_convert(UINT inputtype, const char * inputbuf, char * output, int flip, int width, int height);

void GDIPVideoOutput::SetupGraphics()
{
	// create new canvas
	if (graphics) delete graphics;
	graphics = new Graphics(parent, FALSE);
	graphics->Clear(Color(0));

	HDC h = graphics->GetHDC();

	// recreate back device context
	if (graphicsback) delete graphicsback; // we must delete this before deleting backdc
	if (backdc) DeleteDC(backdc);
	backdc = CreateCompatibleDC(h);

	// make sure back device context has right size and color depth
	HBITMAP memBM = CreateCompatibleBitmap(h, winw, winh);
	SelectObject(backdc, memBM);
	DeleteObject(memBM);

	// create back graphics canvas
	graphicsback = new Graphics(backdc);
	graphicsback->Clear(Color(0));

	graphics->ReleaseHDC(h);

	// set parameters
	/* fuck it, all default for now.
	graphicsback->SetInterpolationMode(InterpolationModeBilinear);
	graphicsback->SetCompositingQuality(CompositingQualityHighSpeed);
	graphicsback->SetCompositingMode(CompositingModeSourceCopy);
	graphicsback->SetSmoothingMode(SmoothingModeNone);
	*/
}

GDIPVideoOutput::GDIPVideoOutput() : graphics(0), frame(0), type(0), graphicsback(0), backdc(0), w(0), h(0), flip(0), winw(0), winh(0), gdiplusToken(0), subs(0), needschange(0), parent(0), adjuster(0)
{
}

GDIPVideoOutput::~GDIPVideoOutput()
{
}

int GDIPVideoOutput::create(HWND parent, VideoAspectAdjuster *_adjuster, int w, int h, unsigned int type, int flipit, double aspectratio) //return 1 if ok
{
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	needschange = 1;
	adjuster = _adjuster;
	needschange = 0;
	RECT r;
	GetWindowRect(parent, &r);
	winw = r.right - r.left;
	winh = r.bottom = r.top;
	this->parent = parent;
	this->flip = flipit;
	this->w = w;
	this->h = h;
	this->type = type;
	SetupGraphics();
	frame = new Bitmap(w, h, graphicsback);
	ZeroMemory(&lastrect, sizeof(RECT));
	return 1;
}

// TODO: verify that this works
bool GDIPVideoOutput::FillFrame(Bitmap * frame, void *buf)
{
	switch (type)
	{
		case VIDEO_MAKETYPE('R', 'G', '3', '2'):
		{
			BITMAPINFO info;
			info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			info.bmiHeader.biWidth = w;
			info.bmiHeader.biHeight  = h;
			info.bmiHeader.biPlanes = 1;
			info.bmiHeader.biBitCount = 32;
			info.bmiHeader.biCompression = BI_RGB;
			info.bmiHeader.biSizeImage = 0;
			info.bmiHeader.biXPelsPerMeter  = 0;
			info.bmiHeader.biYPelsPerMeter  = 0;
			info.bmiHeader.biXPelsPerMeter  = 0;
			info.bmiHeader.biYPelsPerMeter  = 0;
			info.bmiHeader.biClrUsed = 0;
			info.bmiHeader.biClrImportant = 0;

			frame->FromBITMAPINFO(&info, buf);

		}
		return true;
		case VIDEO_MAKETYPE('R', 'G', '2', '4'):
		{
			BITMAPINFO info;
			info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			info.bmiHeader.biWidth = w;
			info.bmiHeader.biHeight  = h;
			info.bmiHeader.biPlanes = 1;
			info.bmiHeader.biBitCount = 24;
			info.bmiHeader.biCompression = BI_RGB;
			info.bmiHeader.biSizeImage = 0;
			info.bmiHeader.biXPelsPerMeter  = 0;
			info.bmiHeader.biYPelsPerMeter  = 0;
			info.bmiHeader.biXPelsPerMeter  = 0;
			info.bmiHeader.biYPelsPerMeter  = 0;
			info.bmiHeader.biClrUsed = 0;
			info.bmiHeader.biClrImportant = 0;

			frame->FromBITMAPINFO(&info, buf);

		}
		return true;
	}
	return false;
}

void GDIPVideoOutput::displayFrame(const char *buf, int size, int time)
{
	// TODO: verify that this works before uncommenting  if (!FillFrame(frame, const_cast<char *>(buf)))
	{
		BitmapData d;
		d.Width = w;
		d.Height = h;
		d.PixelFormat = PixelFormat32bppRGB;
		d.Stride = 4 * w;
		d.Scan0 = 0;

		// write the frame to our bitmap object
		if (frame->LockBits(&Rect(0, 0, w, h), ImageLockModeWrite, PixelFormat32bppRGB, &d) != Ok)
		{
			needschange = 1; return;
		}
		colorspace_convert(type, buf, (char*)d.Scan0, flip, w, h);
		frame->UnlockBits(&d);
	}

	// fix aspect ratio
	RECT r = {0, 0, winw, winh};
	adjuster->adjustAspect(r);
	if (memcmp(&r, &lastrect, sizeof(RECT))) graphicsback->Clear(Color(0));
	lastrect = r;

	// draw the image
	graphicsback->DrawImage(frame, r.left, r.top, r.right - r.left, r.bottom - r.top);
	if (subs)
	{ // draw subtitles
		//graphicsback->DrawString(AutoWide(subs->text),-1,&Font(L"Arial.ttf",36),PointF(subs->xPos,subs->yPos),&SolidBrush(Color(subs->colorRed,subs->colorGreen,subs->colorBlue)));
	}

	// flip graphics and graphicsback
	HDC h = graphics->GetHDC();
	HDC b = graphicsback->GetHDC();
	BitBlt(h, r.left, r.top, r.right - r.left, r.bottom - r.top, b, r.left, r.top, SRCCOPY);
	graphicsback->ReleaseHDC(b);
	graphics->ReleaseHDC(h);
}

int GDIPVideoOutput::needChange()
{
	return needschange;
}

void GDIPVideoOutput::close()
{
	if (graphics) delete graphics; graphics = 0;
	if (frame) delete frame; frame = 0;
	if (graphicsback) delete graphicsback; graphicsback = 0;
	if (backdc) DeleteDC(backdc); backdc = 0;
	subs = 0;
	type = 0;

	GdiplusShutdown(gdiplusToken);
}

void GDIPVideoOutput::Refresh()
{}

void GDIPVideoOutput::timerCallback()
{
	RECT r;
	GetWindowRect(parent, &r);
	UINT w, h;
	w = r.right - r.left;
	h = r.bottom - r.top;
	bool change = (w != winw || h != winh);
	if (change)
	{
		winw = w;
		winh = h;
		// sizes have changed, we must reset the graphics
		SetupGraphics();
	}
}

//mmm. ctrl+c ctrl+v.
static void colorspace_convert(UINT type, const char * buf, char * lpSurface, int flip, int width, int height)
{
	const int lPitch = width * 4;
	if (type == VIDEO_MAKETYPE('Y', 'V', '1', '2'))
	{
		const YV12_PLANES *planes = (YV12_PLANES *)buf;
		// convert yv12 to rgb
		const int bytes = 4;
		int i, j, y00, y01, y10, y11, u, v;
		unsigned char *pY = (unsigned char *)planes->y.baseAddr;
		unsigned char *pU = (unsigned char *)planes->u.baseAddr;
		unsigned char *pV = (unsigned char *)planes->v.baseAddr;
		unsigned char *pOut = (unsigned char*)lpSurface;
		const int rvScale = (int)(2.017 * 65536.0);  //91881;
		const int gvScale = - (int)(0.392 * 65536.0);  // -22553;
		const int guScale = - (int)(0.813 * 65536.0);  // -46801;
		const int buScale = (int)(1.596 * 65536.0);  //116129;
		const int yScale = (int)(1.164 * 65536.0);   //(1.164*65536.0);
		int addOut = lPitch * 2 - width * bytes;
		int yrb = planes->y.rowBytes;
		int addL = lPitch;

		/* LIMIT: convert a 16.16 fixed-point value to a byte, with clipping. */
#define LIMIT(x) ((x)>0xffffff?0xff: ((x)<=0xffff?0:((x)>>16)))

		if (flip)
		{
			pOut += (lPitch) * (height - 1);
			addOut = -lPitch * 2 - width * bytes;
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

					{
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
		if (flip)
			nsutil_image_CopyFlipped_U8((uint8_t *)lpSurface, lPitch, (const uint8_t *)buf, width*4, width, height);
		else
			nsutil_image_Copy_U8((uint8_t *)lpSurface, lPitch, (const uint8_t *)buf, width*4, width, height);
	}
	else if (type == VIDEO_MAKETYPE('Y', 'U', 'Y', '2') || type == VIDEO_MAKETYPE('U', 'Y', 'V', 'Y'))
	{
		char *b = (char *)lpSurface;
		int l2 = lPitch;
		if (flip)
		{
			b += (height - 1) * l2;
			l2 = -l2;
		}
		{
			{
				// yuy2->rgb32 conversion
				unsigned char *src = (unsigned char *)buf;
				unsigned char *dst = (unsigned char *)lpSurface;
				int line, col; //, linewidth;
				int y, yy;
				int u, v;
				int vr, ug, vg, ub;
				unsigned char *py, *pu, *pv;

				//linewidth = width - (width >> 1);
				py = src;
				pu = src + 1;
				pv = src + 3;

				int pitchadd = lPitch - (width * 4);

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
		}
	}
	else if (type == VIDEO_MAKETYPE('R', 'G', '2', '4'))
	{
		if (flip)
				nsutil_image_ConvertFlipped_RGB24_RGB32((RGB32 *)lpSurface, lPitch, (const uint8_t *)buf, width*3, width, height);
			else
				nsutil_image_Convert_RGB24_RGB32((RGB32 *)lpSurface, lPitch, (const uint8_t *)buf, width*3, width, height);
		
	}
}