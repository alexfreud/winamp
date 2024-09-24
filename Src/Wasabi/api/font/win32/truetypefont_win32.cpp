#include "precomp.h"
// ============================================================================================================================================================
// Font abstract class + statics to install TT fonts and Bitmap fonts
// ============================================================================================================================================================
#define WIN32_LEAN_AND_MEAN
#include <fcntl.h>
#include "../nu/ns_wc.h"
#include "truetypefont_win32.h"

#include <tataki/canvas/bltcanvas.h>
#include <tataki/bitmap/bitmap.h>
#include <bfc/file/tmpnamestr.h>
#include <api/memmgr/api_memmgr.h>
#if UTF8
#ifdef WANT_UTF8_WARNINGS
#pragma CHAT("mig", "all", "UTF8 is enabled in std.cpp -- Things might be screwy till it's all debugged?")
#endif
# include <bfc/string/encodedstr.h>
#endif
#define VERTICAL_LPADDING 0
#define VERTICAL_RPADDING 2
#define VERTICAL_TPADDING 1
#define VERTICAL_BPADDING 1

/** ============================================================================================================================================================
 ** TrueTypeFont_Win32 implementation.
 **
 ** TODO:
 ** use GDI transformation to draw in 72 DPI
 ** fractional point sizes?
 ** ===========================================================================================================================================================
 */

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
TrueTypeFont_Win32::TrueTypeFont_Win32()
{
	scriptid = 0;
	font = oldFont = NULL;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
TrueTypeFont_Win32::~TrueTypeFont_Win32()
{
	if (!tmpfilename.isempty())
	{
		RemoveFontResourceW(tmpfilename);
		// explict call to this instead of UNLINK allows correct removal of the temp files
		_wunlink(tmpfilename);
	}
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int TrueTypeFont_Win32::isBitmap()
{
	return 0;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int TrueTypeFont_Win32::addFontResource(OSFILETYPE in, const wchar_t *name)
{
	ASSERT(in != NULL);

	int len = (int)FGETSIZE(in);
	OSFILETYPE out;
	char *m = (char *)MALLOC(len);
	ASSERT(m != NULL);
	FREAD(m, len, 1, in);
	TmpNameStrW tempfn;
	out = WFOPEN(tempfn, L"wb");
	ASSERT(out != OPEN_FAILED);
	FWRITE(m, len, 1, out);
	FCLOSE(out);
	AddFontResourceW(tempfn);
	FREE(m);
	tmpfilename = tempfn;
	setFontFace(filenameToFontFace(tempfn));
	return 1;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int TrueTypeFont_Win32::addFontResource2(void *data, int datalen, const wchar_t *name)
{
	TmpNameStrW tempfn;
	OSFILETYPE out = WFOPEN(tempfn, L"wb");
	ASSERT(out != OPEN_FAILED);
	FWRITE(data, datalen, 1, out);
	FCLOSE(out);
	AddFontResourceW(tempfn);
	tmpfilename = tempfn;
	setFontFace(filenameToFontFace(tempfn));

	WASABI_API_MEMMGR->sysFree(data);

	return 1;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Win32::setFontFace(const wchar_t *face)
{
	face_name = face;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
const wchar_t *TrueTypeFont_Win32::getFaceName()
{
	return face_name;
}

static int IsXp_or_higher()
{
	static int checked=0;
	static int isXp=0;
	if (!checked)
	{
		OSVERSIONINFO osver;
		osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
		isXp = ( ::GetVersionEx(&osver) && osver.dwPlatformId == VER_PLATFORM_WIN32_NT && 	(osver.dwMajorVersion >= 5 )) ? 1 : 0;
		checked=1;
	}
	return isXp;
}

#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY 5
#endif
HFONT TrueTypeFont_Win32::MakeFont(int size, int bold, int underline, int italic, int antialiased)
{
	// TODO: we got the height to be in 72 DPI, but how can we get the width???
	int nHeight = MulDiv(size, 72, 96); // this is lame, but we have to do it to match freetype

	int quality;
	if (antialiased)
	{
		if (IsXp_or_higher())
			quality=CLEARTYPE_QUALITY;
		else
			quality=ANTIALIASED_QUALITY;
	}
	else
		quality=NONANTIALIASED_QUALITY;

	return CreateFontW(-nHeight, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL,
	                   italic, underline, FALSE, DEFAULT_CHARSET,
	                   OUT_TT_ONLY_PRECIS, 
										 CLIP_DEFAULT_PRECIS,
										 quality,
	                   DEFAULT_PITCH | FF_DONTCARE, face_name);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Win32::prepareCanvas(BltCanvas *canvas, int size, int bold, int opaque, int underline, int italic, int antialiased)
{
	HDC canvasHDC = canvas->getHDC();

	font = MakeFont(size, bold, underline, italic, antialiased);
	oldFont = (HFONT)SelectObject(canvasHDC, font);

	SetBkColor(canvasHDC, RGB(0, 0, 0));
	//SetBkMode(canvasHDC, TRANSPARENT);
	SetTextColor(canvasHDC, RGB(255, 255, 255));
}

typedef DWORD ARGB;

#define MIN_TONE 100.0
//BYTE min = 255, max=0;
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Win32::restoreCanvas(BltCanvas *canvas, ifc_canvas *dest, int w, int h, COLORREF color, COLORREF bkcolor, int antialiased, int _x, int _y)
{
#ifndef FE_FONTSMOOTHINGCLEARTYPE
#define FE_FONTSMOOTHINGCLEARTYPE 0x0002
#endif

#ifndef SPI_GETFONTSMOOTHINGTYPE
#define SPI_GETFONTSMOOTHINGTYPE 0x200A
#endif

	bool clearType = false;
	UINT fontSmoothing;
	if (antialiased && SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &fontSmoothing, 0))
		clearType = fontSmoothing == FE_FONTSMOOTHINGCLEARTYPE;

	ARGB32 *buf = static_cast<ARGB32 *>(canvas->getBits());

	for (int y = 0; y < h; y++)
	{
		int linewidth = y * w;

		for (int x = 0; x < w; x++)
		{

			ARGB32* prgb = &buf[linewidth + x];
			unsigned char *pixel = (unsigned char *)prgb;
			if (*prgb == 0)
			{
				// Do nothing
			}
			else
			{
				BYTE alpha = 255;
				BYTE rAlpha = 255;
				BYTE gAlpha = 255;
				BYTE bAlpha = 255;

				if (*prgb != 0xFFFFFF)
				{
					if (clearType)
					{
						rAlpha = pixel[2];
						gAlpha = pixel[1];
						bAlpha = pixel[0];
					}

					UINT value = pixel[0] + pixel[1] + pixel[2];
					value = value / 3;
					alpha = (BYTE)value;

					if (!clearType)
					{
						rAlpha = alpha;
						gAlpha = alpha;
						bAlpha = alpha;
					}

					//alpha=pixel[0];
					//alpha = (((float)pixel[0] - MIN_TONE) / (255.0 - MIN_TONE)) * 255 ;

					//min = (min > value) ? value : min;
					//max = (max < value) ? value : max;
				}

				pixel[3] = (BYTE)alpha;
				pixel[2] = ((GetRValue(color) * rAlpha) + 128) / 255;
				pixel[1] = ((GetGValue(color) * gAlpha) + 128) / 255;
				pixel[0] = ((GetBValue(color) * bAlpha) + 128) / 255;
			}
		}
	}

	canvas->blitAlpha(dest, _x + VERTICAL_LPADDING, _y + VERTICAL_TPADDING);
	//SkinBitmap *bitmap = canvas->getSkinBitmap();
	//bitmap->blitAlpha(dest, _x + VERTICAL_LPADDING, _y + VERTICAL_TPADDING);

	SelectObject(canvas->getHDC(), oldFont);
	oldFont = 0;
	DeleteObject(font);
	font = 0;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Win32::textOut(ifc_canvas *c, int x, int y, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialiased)
{
	int w, h;
	c->getDim(&w, &h);

	if (!h || !w) return;

	BltCanvas canvas;
	prepareCanvas(&canvas, size, bold, opaque, underline, italic, antialiased);
	canvas.DestructiveResize(w, h);

	TextOutW(canvas.getHDC(), x + xoffset, y + yoffset, txt, wcslen(txt));

	restoreCanvas(&canvas, c, w, h, color, bkcolor, antialiased);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Win32::textOut2(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialiased)
{
	if (!h || !w) return;

	BltCanvas canvas;
	prepareCanvas(&canvas,  size, bold, opaque, underline, italic, antialiased);
	canvas.DestructiveResize(/*x +*/ w, /*y +*/ h);

	RECT r = {
	           /*x +*/ xoffset,
	           /*y + */yoffset,
	           /*x +  */w,
	           /*y +  */h
	         };

	DrawTextW(canvas.getHDC(), txt, -1, &r, align |  DT_NOPREFIX | DT_NOCLIP);

	restoreCanvas(&canvas, c, /*x + */w, /*y + */h, color, bkcolor, antialiased, x, y);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Win32::textOutEllipsed(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialiased)
{
	if (!h || !w) return;

	BltCanvas canvas;
	prepareCanvas(&canvas, size, bold, opaque, underline, italic, antialiased);
	canvas.DestructiveResize(/*x + */w, /*y + */h);

	RECT r = {
	           /*x + */xoffset,
	           /*y + */yoffset,
	           /*x +  */w,
	           /*y + */ h
	         };

	DrawTextW(canvas.getHDC(), txt, -1, &r, align | DT_NOPREFIX | DT_END_ELLIPSIS | DT_NOCLIP);
	restoreCanvas(&canvas, c,/* x + */w, /*y + */h, color, bkcolor, antialiased, x, y);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Win32::textOutWrapped(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialiased)
{
	if (!h || !w) return;

	BltCanvas canvas;
	prepareCanvas(&canvas, size, bold, opaque, underline, italic, antialiased);
	canvas.DestructiveResize(w, h);

	RECT r;
	r.left = x + xoffset;
	r.top = y + yoffset;
	r.right = r.left + w;
	r.bottom = r.top + h;

	DrawTextW(canvas.getHDC(), txt, -1, &r, align | DT_NOPREFIX | DT_WORDBREAK);

	restoreCanvas(&canvas, c, w, h, color, bkcolor, antialiased);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Win32::textOutWrappedPathed(ifc_canvas *c, int x, int y, int w, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialiased)
{
	int w_dummy, h;
	c->getDim(&w_dummy, &h);

	if (!h || !w) return;

	BltCanvas canvas;
	prepareCanvas(&canvas, size, bold, opaque, underline, italic, antialiased);
	canvas.DestructiveResize(w, h);

	RECT r;
	wchar_t *ptr, *d;
	const wchar_t *s;
	ptr = (wchar_t *)MALLOC(sizeof(wchar_t) * (wcslen(txt) + 1 + 4));
	for (s = txt, d = ptr; *s; s++, d++)
	{
		if (*s == '/') *d = '\\';
		else *d = *s;
	}
	r.left = x + xoffset;
	r.top = y + yoffset;
	r.right = r.left + w;
	r.bottom = r.top + getTextHeight2(c, size, bold, underline, italic, antialiased);

	DrawTextW(canvas.getHDC(), ptr, -1, &r, align | DT_NOPREFIX | DT_PATH_ELLIPSIS | DT_SINGLELINE | DT_MODIFYSTRING | DT_CALCRECT);

	for (d = ptr; *d; d++)
	{
		if (*d == '\\') *d = '/';
	}

	DrawTextW(canvas.getHDC(), ptr, -1, &r, align | DT_NOPREFIX | DT_PATH_ELLIPSIS | DT_SINGLELINE);
	restoreCanvas(&canvas, c, w, h, color, bkcolor, antialiased);

	FREE(ptr);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Win32::textOutCentered(ifc_canvas *c, RECT *r, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialiased)
{
	yoffset += 1;
	ASSERT(r != NULL);
	ASSERT(txt != NULL);
	RECT rr = *r;
	rr.left += xoffset;
	rr.right += xoffset;
	rr.top += yoffset;
	rr.bottom += yoffset;

	int w = rr.right - rr.left;
	int h = rr.bottom - rr.top;

	RECT r2 = {0, 0, w, h};

	BltCanvas canvas;
	prepareCanvas(&canvas, size, bold, opaque, underline, italic, antialiased);
	canvas.DestructiveResize(w, h);

	DrawTextW(canvas.getHDC(), txt, -1, &r2, align | DT_CENTER |  /*DT_VCENTER | */DT_NOPREFIX | DT_WORDBREAK | DT_SINGLELINE);

	restoreCanvas(&canvas, c, w, h, color, bkcolor, antialiased, rr.left, rr.top);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int TrueTypeFont_Win32::getTextWidth(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialiased)
{
	int w = 0;
	getTextExtent(c, text, &w, NULL, size, bold, underline, italic, antialiased);
	return w;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int TrueTypeFont_Win32::getTextHeight(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialiased)
{
	int h = 0;
	getTextExtent(c, text, NULL, &h, size, bold, underline, italic, antialiased);
	{
		// calcul for multiline text
		const wchar_t *p = text;
		int n = 0;
		while (p && *p != 0) if (*p++ == '\n') n++;
		if (n) h *= (n + 1);
	}
	return h ;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Win32::getTextExtent(ifc_canvas *c, const wchar_t *txt, int *w, int *h, int size, int bold, int underline, int italic, int antialiased)
{
	SIZE rsize = {0, 0};
	ASSERT(txt != NULL);
	if (*txt == 0)
	{
		if (w != NULL) *w = 0;
		if (h != NULL) *h = 0;
		return ;
	}

	HFONT newFont = MakeFont(size, bold, underline, italic, antialiased);
	HFONT theOldFont = (HFONT)SelectObject(c->getHDC(), newFont);
	GetTextExtentPoint32W(c->getHDC(), txt, wcslen(txt), &rsize);

	SelectObject(c->getHDC(), theOldFont);
	DeleteObject(newFont);

	if (w != NULL) *w = rsize.cx + VERTICAL_LPADDING + VERTICAL_RPADDING;
	if (h != NULL) *h = rsize.cy + VERTICAL_TPADDING + VERTICAL_BPADDING;

}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int TrueTypeFont_Win32::getTextHeight2(ifc_canvas *c, int size, int bold, int underline, int italic, int antialiased)
{
	return getTextHeight(c, L"Mg", size, bold, underline, italic, antialiased);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// code from ftp.microsoft.com/Softlib/MSLFILES/FONTINST.EXE
// retrieves the friendly font name from its filename
wchar_t *TrueTypeFont_Win32::filenameToFontFace(const wchar_t *pszFile)
{
	static wchar_t lpszLongName[256];
	unsigned i;
	char namebuf[255] = {0};
	int	fp;
	unsigned short numNames;
	long	curseek;
	unsigned cTables;
	sfnt_OffsetTable OffsetTable;
	sfnt_DirectoryEntry Table;
	sfnt_NamingTable NamingTable;
	sfnt_NameRecord	NameRecord;


	lpszLongName[0] = '\0';
	if ((fp = _wopen(pszFile, O_RDONLY | O_BINARY)) == -1)
		return NULL;

	/* First off, read the initial directory header on the TTF.  We're only
	* interested in the "numOffsets" variable to tell us how many tables 
	* are present in this file.
	*
	* Remember to always convert from Motorola format (Big Endian to 
	* Little Endian).
	*/
	_read(fp, &OffsetTable, sizeof(OffsetTable) - sizeof
	      (sfnt_DirectoryEntry));
	cTables = (int) SWAPW(OffsetTable.numOffsets);

	for (i = 0; i < cTables && i < 40; i++)
	{
		if ((read(fp, &Table, sizeof(Table))) != sizeof(Table)) return NULL;
		if (Table.tag == tag_NamingTable) /* defined in sfnt_en.h */ {
			/* Now that we've found the entry for the name table, seek to that   * position in the file and read in the initial header for this   * particular table.  See "True Type Font Files" for information   * on this record layout.
			  */
			lseek(fp, SWAPL(Table.offset), SEEK_SET);
			read(fp, &NamingTable, sizeof(NamingTable));
			numNames = SWAPW(NamingTable.count);
			while (numNames--)
			{
				read(fp, &NameRecord, sizeof(NameRecord));
				curseek = tell(fp);
				if (SWAPW(NameRecord.platformID) == 1 &&
				    SWAPW(NameRecord.nameID) == 4)
				{
					lseek(fp, SWAPW(NameRecord.offset) +
					      SWAPW(NamingTable.stringOffset) +
					      SWAPL(Table.offset), SEEK_SET);
					read(fp, &namebuf, MIN(255, (int)SWAPW(NameRecord.length)));
					namebuf[MIN(255, (int)SWAPW(NameRecord.length))] = '\0';
					MultiByteToWideCharSZ(1252, 0, namebuf, -1, lpszLongName, 256); // TODO: benski> what codepage is TTF using internally?
					//CUT: lstrcpy(lpszLongName, namebuf);
					lseek(fp, curseek, SEEK_SET);
				}
			}
			close(fp);
			return lpszLongName;
		}
	}
	close(fp);

	return NULL;
}


/*
TODO:
in order to do anti-aliased stuff, we need to draw onto a Compatible Bitmap, and then get the DIB bits out
we might also be able to create a Compatible HBITMAP and then pass it as the constructor parameter to SkinBitmap

sample code:
void RenderFont(int x, int y, int size, char *str, char *fontname,
                        void *buf, int w, int h)
   {

        HDC hdc=GetDC(NULL);
        HDC mdc = CreateCompatibleDC(hdc);
        HBITMAP bm = CreateCompatibleBitmap(hdc,w,h);

        HFONT hf=CreateFont(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, DEFAULT_PITCH, fontname);

        RECT r;
        r.top=0;
        r.left=0;
        r.right=w;
        r.bottom=h;

        SelectObject(mdc, bm);
        FillRect(mdc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));

        SelectObject(mdc, hf);
        SetBkMode(mdc, TRANSPARENT);
        SetTextColor(mdc, 0xFFFFFF);
        TextOut(mdc, 0, 0, str, strlen(str));

        BITMAPINFO bmi;

        bmi.bmiHeader.biSize=sizeof(bmi.bmiHeader);
        bmi.bmiHeader.biWidth=256;
        bmi.bmiHeader.biHeight=256;
        bmi.bmiHeader.biPlanes=1;
        bmi.bmiHeader.biBitCount=32;
        bmi.bmiHeader.biCompression=BI_RGB;

        GetDIBits(mdc,bm,0,256,buf,&bmi,DIB_RGB_COLORS);

        DeleteObject(hf);
        DeleteObject(bm);
}
*/