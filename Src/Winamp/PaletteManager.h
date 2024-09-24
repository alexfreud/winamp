#pragma once
#include "SkinElementAlias.h"
#include "SkinColorElement.h"
#include "SkinCursorElement.h"
#include "SkinBitmapElement.h"
#include <api/syscb/callbacks/gccb.h>
#include <api/skin/api_palette.h>
#include <bfc/ptrlist.h>
#include "GammaFilter.h"

class PaletteManager;
class PaletteGC : public GarbageCollectCallbackI
{
public:
	PaletteGC(PaletteManager *_parent)
	{
		parent = _parent;
	}
private:
	int gccb_onGarbageCollect();
	PaletteManager *parent;
};
class PaletteManager : public api_palette
{
public:
	static const char *getServiceName() { return "Skin Palette API"; }
	static const GUID getServiceGuid() { return PaletteManagerGUID; }	
	PaletteManager();
	~PaletteManager();

	void StartTransaction();
	void EndTransaction();

	void Reset();

	const int *getSkinPartIteratorPtr();
	int newSkinPart();
	int getSkinPartIterator();

	void UnloadElements(int skinpart);

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
	SkinBitmapElement *enumBitmapElement(int n);
	int getNumBitmapElement();
	const wchar_t *getSkinBitmapFilename(const wchar_t *id, int *x, int *y, int *w, int *h, const wchar_t **rootpath, ifc_xmlreaderparams **params);
	const wchar_t *getGammaGroupFromId(const wchar_t *id);
	int getLayerFromId(const wchar_t *id);

	/* Region Server (part of Bitmaps) */
	RegionServer *requestSkinRegion(const wchar_t *id);
	void cacheSkinRegion(const wchar_t *id, api_region *r);
	void onGarbageCollect();
	void garbageCollectRegionServer(ElementRegionServer *rs);

protected:
	RECVS_DISPATCH;
private:
	SkinCursorElement *enumCursorElement(int n);
	int getNumSkinCursorElements();

	int skinpart_iterator;
	int genericcounter;

	typedef PtrListQuickMultiSorted<SkinElementAlias, SortSkinElementAlias> SkinAliasList;
	SkinAliasList skinAliasList;
	typedef PtrListQuickMultiSorted<SkinColorElement, SortSkinColorElement> SkinColorList; 
	SkinColorList skinColorList;
	typedef PtrListQuickMultiSorted<SkinCursorElement, SortSkinCursorElement> SkinCursorList;
	SkinCursorList skinCursorList;
	typedef PtrListQuickMultiSorted<SkinBitmapElement, SortSkinBitmapElement> SkinBitmapList;
	SkinBitmapList skinBitmapList;

	PtrList<ElementRegionServer> regsrvGC;
	PaletteGC paletteGC;
	GammaFilterFactory gammaFilterFactory;
};

