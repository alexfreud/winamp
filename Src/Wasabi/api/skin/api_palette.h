#pragma once
#include <bfc/dispatch.h>
#include <api/skin/skinitem.h>
#include <api/skin/skin.h>
#include <tataki/region/region.h>

class api_palette : public Dispatchable
{
protected:
	api_palette() {}
	~api_palette() {}
public:
	void StartTransaction(); // if you add a bunch of stuff at once, it's faster to call this at the beginning
	void EndTransaction(); // and this at the end.

	void Reset(); // clear everything

	const int *getSkinPartIteratorPtr(); // call this to get a pointer that's faster to check than calling getSkinPartIterator via Dispatchable
	int newSkinPart(); // call this before adding your elements, let's you easily remove just your stuff via UnloadElements
	int getSkinPartIterator(); // if this value changes, then something in the skin palette has changed

	void UnloadElements(int skinpart); // unload just your skin elements (get a skinpart value via newSkinPart)

	/* Aliases */
	void AddAlias(const wchar_t *id, const wchar_t *target);
	const wchar_t *getElementAlias(const wchar_t *alias);
	SkinItem *getAliasAncestor(SkinItem *item);

	/* Colors */
	void AddColor(const wchar_t *id, ARGB32 value, const wchar_t *colorgrp = NULL, const wchar_t *path = NULL, ifc_xmlreaderparams *p = NULL);
	int getNumColorElements();
	const wchar_t *enumColorElement(int n);
	ARGB32 *getColorElementRef(const wchar_t *type, const wchar_t **grp = NULL);
	SkinItem *getColorAncestor(SkinItem *item);
	ARGB32 getColorElement(const wchar_t *type, const wchar_t **grp = NULL);

	/* Cursors */
	void AddCursor(const wchar_t *id, const wchar_t *bitmapid, int x, int y, const wchar_t *path = NULL, ifc_xmlreaderparams *params = NULL);
	int getCursorElement(const wchar_t *id);
	OSCURSOR getCursor(const wchar_t *id);
	SkinItem *getCursorAncestor(SkinItem *item);
	const wchar_t *getSkinCursorBitmapId(const wchar_t *cursor);

	/* Bitmaps */
	void AddBitmap(const wchar_t *id, const wchar_t *filename, const wchar_t *path, int x, int y, int w, int h, ifc_xmlreaderparams *params = NULL, const wchar_t *colorgroup = NULL);
	int getBitmapElement(const wchar_t *type);
	SkinItem *getBitmapAncestor(SkinItem *item);
	int getNumBitmapElement();
	const wchar_t *getSkinBitmapFilename(const wchar_t *id, int *x, int *y, int *w, int *h, const wchar_t **rootpath, ifc_xmlreaderparams **params);
	const wchar_t *getGammaGroupFromId(const wchar_t *id);
	int getLayerFromId(const wchar_t *id);

	/* Region Server (part of Bitmaps) */
	RegionServer *requestSkinRegion(const wchar_t *id);
	void cacheSkinRegion(const wchar_t *id, api_region *r);

	enum
	{
		API_PALETTE_STARTTRANSACTION=0,
		API_PALETTE_ENDTRANSACTION=1,
		API_PALETTE_RESET=2,
		API_PALETTE_GETSKINPARTITERATORPTR=3,
		API_PALETTE_NEWSKINPART=4,
		API_PALETTE_GETSKINPARTITERATOR=5,
		API_PALETTE_UNLOADELEMENTS=6,
		API_PALETTE_ADDALIAS=7,
		API_PALETTE_GETELEMENTALIAS=8,
		API_PALETTE_GETALIASANCESTOR=9,
		API_PALETTE_ADDCOLOR=10,
		API_PALETTE_GETNUMCOLORELEMENTS=11,
		API_PALETTE_ENUMCOLORELEMENT=12,
		API_PALETTE_GETCOLORELEMENTREF=13,
		API_PALETTE_GETCOLORANCESTOR=14,
		API_PALETTE_GETCOLORELEMENT=15,
		API_PALETTE_ADDCURSOR=16,
		API_PALETTE_GETCURSORELEMENT=17,
		API_PALETTE_GETCURSOR=18,
		API_PALETTE_GETCURSORANCESTOR=19,
		API_PALETTE_GETSKINCURSORBITMAPID=20,
		API_PALETTE_ADDBITMAP=21,
		API_PALETTE_GETBITMAPELEMENT=22,
		API_PALETTE_GETBITMAPANCESTOR=23,
		API_PALETTE_GETNUMBITMAPELEMENT=24,
		API_PALETTE_GETSKINBITMAPFILENAME=25,
		API_PALETTE_GETGAMMAGROUPFROMID=26,
		API_PALETTE_GETLAYERFROMID=27,
		API_PALETTE_REQUESTSKINREGION=28,
		API_PALETTE_CACHESKINREGION=29,
	};
};

inline void api_palette::StartTransaction()
{
	_voidcall(API_PALETTE_STARTTRANSACTION);
}


inline void api_palette::EndTransaction()
{
	_voidcall(API_PALETTE_ENDTRANSACTION);
}


inline void api_palette::Reset()
{
	_voidcall(API_PALETTE_RESET);
}


inline const int *api_palette::getSkinPartIteratorPtr()
{
	return _call(API_PALETTE_GETSKINPARTITERATORPTR, (const int *)0);
}


inline int api_palette::newSkinPart()
{
	return _call(API_PALETTE_NEWSKINPART, (int)0);
}


inline int api_palette::getSkinPartIterator()
{
	return _call(API_PALETTE_GETSKINPARTITERATOR, (int)0);
}


inline void api_palette::UnloadElements(int skinpart)
{
	_voidcall(API_PALETTE_UNLOADELEMENTS, skinpart);
}


inline void api_palette::AddAlias(const wchar_t *id, const wchar_t *target)
{
	_voidcall(API_PALETTE_ADDALIAS, id, target);
}


inline const wchar_t *api_palette::getElementAlias(const wchar_t *alias)
{
	return _call(API_PALETTE_GETELEMENTALIAS, (const wchar_t *)0, alias);
}


inline SkinItem *api_palette::getAliasAncestor(SkinItem *item)
{
	return _call(API_PALETTE_GETALIASANCESTOR, (SkinItem *)0, item);
}


inline void api_palette::AddColor(const wchar_t *id, ARGB32 value, const wchar_t *colorgrp, const wchar_t *path, ifc_xmlreaderparams *p)
{
	_voidcall(API_PALETTE_ADDCOLOR, id, value, colorgrp, path, p);
}


inline int api_palette::getNumColorElements()
{
	return _call(API_PALETTE_GETNUMCOLORELEMENTS, (int)0);
}


inline const wchar_t *api_palette::enumColorElement(int n)
{
	return _call(API_PALETTE_ENUMCOLORELEMENT, (const wchar_t *)0, n);
}


inline ARGB32 *api_palette::getColorElementRef(const wchar_t *type, const wchar_t **grp)
{
	return _call(API_PALETTE_GETCOLORELEMENTREF, (ARGB32 *)0, type, grp);
}


inline SkinItem *api_palette::getColorAncestor(SkinItem *item)
{
	return _call(API_PALETTE_GETCOLORANCESTOR, (SkinItem *)0, item);
}


inline ARGB32 api_palette::getColorElement(const wchar_t *type, const wchar_t **grp)
{
	return _call(API_PALETTE_GETCOLORELEMENT, (ARGB32)RGB(255, 0, 255), type, grp);
}


inline void api_palette::AddCursor(const wchar_t *id, const wchar_t *bitmapid, int x, int y, const wchar_t *path, ifc_xmlreaderparams *params)
{
	_voidcall(API_PALETTE_ADDCURSOR, id, bitmapid, x, y, path, params);
}


inline int api_palette::getCursorElement(const wchar_t *id)
{
	return _call(API_PALETTE_GETCURSORELEMENT, (int)-1, id);
}


inline OSCURSOR api_palette::getCursor(const wchar_t *id)
{
	return _call(API_PALETTE_GETCURSOR, INVALIDOSCURSORHANDLE, id);
}


inline SkinItem *api_palette::getCursorAncestor(SkinItem *item)
{
	return _call(API_PALETTE_GETCURSORANCESTOR, (SkinItem *)0, item);
}


inline const wchar_t *api_palette::getSkinCursorBitmapId(const wchar_t *cursor)
{
	return _call(API_PALETTE_GETSKINCURSORBITMAPID, (const wchar_t *)0, cursor);
}


inline void api_palette::AddBitmap(const wchar_t *id, const wchar_t *filename, const wchar_t *path, int x, int y, int w, int h, ifc_xmlreaderparams *params, const wchar_t *colorgroup)
{
	_voidcall(API_PALETTE_ADDBITMAP, id, filename, path, x, y, w, h, params, colorgroup);
}


inline int api_palette::getBitmapElement(const wchar_t *type)
{
	return _call(API_PALETTE_GETBITMAPELEMENT, (int)-1, type);
}


inline SkinItem *api_palette::getBitmapAncestor(SkinItem *item)
{
	return _call(API_PALETTE_GETBITMAPANCESTOR, (SkinItem *)0, item);
}


inline int api_palette::getNumBitmapElement()
{
	return _call(API_PALETTE_GETNUMBITMAPELEMENT, (int)0);
}


inline const wchar_t *api_palette::getSkinBitmapFilename(const wchar_t *id, int *x, int *y, int *w, int *h, const wchar_t **rootpath, ifc_xmlreaderparams **params)
{
	return _call(API_PALETTE_GETSKINBITMAPFILENAME, (const wchar_t *)0,id,x,y,w,h,rootpath, params);
}


inline const wchar_t *api_palette::getGammaGroupFromId(const wchar_t *id)
{
	return _call(API_PALETTE_GETGAMMAGROUPFROMID, (const wchar_t *)0, id);
}


inline int api_palette::getLayerFromId(const wchar_t *id)
{
	return _call(API_PALETTE_GETLAYERFROMID, (int)-1, id);
}


inline RegionServer *api_palette::requestSkinRegion(const wchar_t *id)
{
	return _call(API_PALETTE_REQUESTSKINREGION, (RegionServer *)0, id);
}


inline void api_palette::cacheSkinRegion(const wchar_t *id, api_region *r)
{
	_voidcall(API_PALETTE_CACHESKINREGION, id, r);
}

// {DCA2E3C2-4C3E-4dd9-AB1A-1940F2CA31F7}
static const GUID PaletteManagerGUID = 
{ 0xdca2e3c2, 0x4c3e, 0x4dd9, { 0xab, 0x1a, 0x19, 0x40, 0xf2, 0xca, 0x31, 0xf7 } };
