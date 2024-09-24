#include "api.h"
#include "SkinCursorElement.h"
#include "PaletteManager.h"
#include <tataki/canvas/bltcanvas.h>

SkinCursorElement::SkinCursorElement(const wchar_t *_id, const wchar_t *_bitmapid, int _x, int _y, int script_id, int secondarycounter, const wchar_t *path, ifc_xmlreaderparams *p)
{
	id = _id;
	bitmap = _bitmapid;
	x = _x;
	y = _y;
	icon = NULL;
	scriptid = script_id;
	seccount = secondarycounter;

	if (p != NULL)
	{
		for (size_t i = 0;i != p->getNbItems();i++)
			params.addItem(p->getItemName(i), p->getItemValue(i));
	}
	rootpath = path;
}

SkinCursorElement::~SkinCursorElement()
{
	if (icon != 0)
	{
#ifdef WIN32
		DestroyIcon(icon);
#else
		DebugString("portme: ~skincursorelement\n");
#endif

	}

}

void SkinCursorElement::makeCursor()
{
	if (icon)
	{
#ifdef WIN32
		DestroyIcon(icon);
#else
		DebugString("portme: skincursorelement::makeCursor\n");
#endif
		icon = NULL;
	}

#ifdef WIN32
	ICONINFO info;
	info.fIcon = FALSE;
	info.xHotspot = x;
	info.yHotspot = y;

	// make AND bitmask from alpha channel

	SkinBitmap bm(bitmap);
	int *bits = (int *)bm.getBits();
	int _x = bm.getX();
	int _y = bm.getY();
	int w = bm.getWidth();
	int h = bm.getHeight();
	int fw = bm.getFullWidth();
//CUT:	int fh = bm.getFullHeight();

	BltCanvas c(w, h, NULL, 8);
	unsigned __int8 *d = (unsigned __int8 *)c.getBits();
	for (int i = 0;i < h;i++)
	{
		int *p = bits + _x + x + (_y + y) * fw;
		int j = w;
		while (j--)
		{
			int a = (*p++ & 0xFF000000) >> 24;
			*d++ = (a > 0x7F) ? 0xFF : 0;
		}
	}

	// copy bits onto a canvas to get hbitmap
	BltCanvas cc(w, h);
	bm.blit(&cc, 0, 0);

	info.hbmMask = c.getBitmap();
	info.hbmColor = cc.getBitmap();

	// create cursor
	icon = CreateIconIndirect(&info);
#else
	DebugString("portme: skincursorelement::makeCursor\n");
#endif
}

OSCURSOR SkinCursorElement::getCursor()
{
	if (icon == INVALIDOSCURSORHANDLE)
		makeCursor();
	return icon;
}

SkinItem *SkinCursorElement::getAncestor()
{
	return WASABI_API_PALETTE->getCursorAncestor(this);
}