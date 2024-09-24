#include "api.h"
#include "PaletteManager.h"
#include <api/skin/skinparse.h>

#define COLOR_WHITE (0xffffff)
#define COLOR_BLACK (0x000000)
#define COLOR_ERROR (0xff00ff)


static ARGB32 parseColor(const wchar_t *color, int *error)
{
	if (color == NULL || *color == '\0') { if (error) *error = 1; return COLOR_ERROR; }
	if (!WCSICMP(color, L"white")) return COLOR_WHITE;
	if (!WCSICMP(color, L"black")) return COLOR_BLACK;
	if (wcschr(color, ','))
	{
		int r = 0, g = 0, b = 0;
		if (swscanf(color, L"%d,%d,%d", &r, &g, &b) != 3) return COLOR_ERROR;
		return RGB(r, g, b); // our colors are reversed internally
	}
	if (*color == '#')
	{
		int r = 0, g = 0, b = 0;
		if (swscanf(color, L"#%02x%02x%02x", &r, &g, &b) != 3) return COLOR_ERROR;
		return RGB(r, g, b);
	}
	if (error) *error = 1;
	return COLOR_ERROR;
}
#pragma warning(push)
#pragma warning(disable : 4355)
PaletteManager::PaletteManager() : paletteGC(this)
{
	skinpart_iterator = 1;
	genericcounter = 0;
	WASABI_API_SYSCB->syscb_registerCallback(&paletteGC);
	WASABI_API_SVC->service_register(&gammaFilterFactory);
}
#pragma warning(pop)

PaletteManager::~PaletteManager()
{
	WASABI_API_SVC->service_deregister(&gammaFilterFactory);
	WASABI_API_SYSCB->syscb_deregisterCallback(&paletteGC);
}

void PaletteManager::StartTransaction()
{
	skinColorList.setAutoSort(FALSE);
	skinAliasList.setAutoSort(FALSE);
	skinCursorList.setAutoSort(FALSE);
	skinBitmapList.setAutoSort(FALSE);
}

void PaletteManager::EndTransaction()
{
	skinAliasList.sort();
	skinColorList.sort();
	skinCursorList.sort();
	skinBitmapList.sort();
}

const int *PaletteManager::getSkinPartIteratorPtr()
{
	return &skinpart_iterator;
}

int PaletteManager::newSkinPart()
{
	return ++skinpart_iterator;
}

int PaletteManager::getSkinPartIterator()
{
	return skinpart_iterator;
}

void PaletteManager::AddAlias(const wchar_t *id, const wchar_t *target)
{
	// TODO: benski, in transaction, set some transaction_iterator = skinpart_iterator?
	skinAliasList.addItem(new SkinElementAlias(id, target, skinpart_iterator, genericcounter++));
}

void PaletteManager::Reset()
{
	skinAliasList.deleteAll();
	skinColorList.deleteAll();
	skinCursorList.deleteAll();
	skinBitmapList.deleteAll();
	skinAliasList.setAutoSort(FALSE);
	skinColorList.setAutoSort(FALSE);
	skinCursorList.setAutoSort(FALSE);
	skinBitmapList.setAutoSort(FALSE);
}

const wchar_t *PaletteManager::getElementAlias(const wchar_t *alias)
{
	if (alias == NULL) return NULL;
	int pos;
	SkinElementAlias *sea = skinAliasList.findLastItem(alias, &pos);
	if (sea == NULL) return NULL;
	return skinAliasList.enumItem(pos)->getTargetId();
}

void PaletteManager::UnloadElements(int skinpart)
{
	for (int i = 0;i < skinAliasList.getNumItems();i++)
	{
		if (skinAliasList.enumItem(i)->getSkinPartId() == skinpart)
		{
			delete skinAliasList.enumItem(i);
			skinAliasList.removeByPos(i);
			i--;
		}
	}
	for (int i = 0;i < skinColorList.getNumItems();i++)
	{
		if (skinColorList.enumItem(i)->getSkinPartId() == skinpart)
		{
			delete skinColorList.enumItem(i);
			skinColorList.removeByPos(i);
			i--;
		}
	}
	for (int i = 0;i < skinCursorList.getNumItems();i++)
	{
		if (skinCursorList.enumItem(i)->getScriptId() == skinpart)
		{
			delete skinCursorList.enumItem(i);
			skinCursorList.removeByPos(i);
			i--;
		}
	}
	for (int i = 0;i < skinBitmapList.getNumItems();i++)
	{
		if (skinBitmapList.enumItem(i)->getSkinPartId() == skinpart)
		{
			delete skinBitmapList.enumItem(i);
			skinBitmapList.removeByPos(i);
			i--;
		}
	}

}

SkinItem *PaletteManager::getAliasAncestor(SkinItem *item)
{
	SkinElementAlias *elem = static_cast<SkinElementAlias *>(item);
	int pos = skinAliasList.searchItem(elem);
	if (pos <= 0) return NULL;
	const wchar_t *it = elem->getAliasName();
	pos--;
	SkinElementAlias *aelem = skinAliasList.enumItem(pos);
	if (aelem == NULL) return NULL;
	const wchar_t *ait = aelem->getAliasName();
	if (!WCSICMP(it, ait)) return aelem;
	return NULL;
}

void PaletteManager::AddColor(const wchar_t *id, ARGB32 value, const wchar_t *colorgroup, const wchar_t *path, ifc_xmlreaderparams *params)
{
	skinColorList.addItem(new SkinColorElement(id, value, skinpart_iterator, genericcounter++, colorgroup, path, params));
}

int PaletteManager::getNumColorElements() 
{
	return skinColorList.getNumItems(); 
}

const wchar_t *PaletteManager::enumColorElement(int n)
{
	return skinColorList.enumItem(n)->getId(); 
}

ARGB32 *PaletteManager::getColorElementRef(const wchar_t *type, const wchar_t **grp)
{
	const wchar_t *alias = getElementAlias(type);
	if (alias != NULL)
		type = alias;

	if (grp != NULL) *grp = NULL;
	SkinColorElement *sce = skinColorList.findLastItem(type);
	if (sce == NULL) return NULL;
	if (grp != NULL) *grp = sce->getColorGroup();
	return (unsigned long *)(sce->getColorRef());
}

SkinItem *PaletteManager::getColorAncestor(SkinItem *item)
{
	SkinColorElement *elem = static_cast<SkinColorElement *>(item);
	int pos = skinColorList.searchItem(elem);
	if (pos <= 0) return NULL;
	const wchar_t *it = elem->getId();
	pos--;
	SkinColorElement *aelem = skinColorList.enumItem(pos);
	if (aelem == NULL) return NULL;
	const wchar_t *ait = aelem->getId();
	if (!WCSICMP(it, ait)) return aelem;
	return NULL;
}

ARGB32 PaletteManager::getColorElement(const wchar_t *type, const wchar_t **grp)
{
	const wchar_t *alias = getElementAlias(type);
	if (alias != NULL)
		type = alias;
	ARGB32 *v = getColorElementRef(type, grp);
	if (!v)
	{
		int err = 0;
		ARGB32 r = parseColor(type, &err);
		if (!err) return r;
	}
	return v ? *v : RGB(255, 0, 255);
}

void PaletteManager::AddCursor(const wchar_t *id, const wchar_t *bitmapid, int x, int y, const wchar_t *path, ifc_xmlreaderparams *params)
{
	skinCursorList.addItem(new SkinCursorElement(id, bitmapid, x, y, skinpart_iterator, genericcounter++, path, params));
}

OSCURSOR PaletteManager::getCursor(const wchar_t *id)
{
	int pos = getCursorElement(id);
	if (pos >= 0) 
	{
		SkinCursorElement *sce = enumCursorElement(pos);
		return sce->getCursor();
	}
	return INVALIDOSCURSORHANDLE;
}

int PaletteManager::getCursorElement(const wchar_t *id)
{
	const wchar_t *alias = getElementAlias(id);
	if (alias != NULL)
		id = alias;

	int pos;
	SkinCursorElement *sce = skinCursorList.findLastItem(id, &pos);
	if (sce == NULL) return 0;
	return pos;
}

SkinCursorElement *PaletteManager::enumCursorElement(int n)
{
	return skinCursorList.enumItem(n);
}

int PaletteManager::getNumSkinCursorElements()
{
	return skinCursorList.getNumItems();
}

SkinItem *PaletteManager::getCursorAncestor(SkinItem *item)
{
	SkinCursorElement *elem = static_cast<SkinCursorElement *>(item);
	int pos = skinCursorList.searchItem(elem);
	if (pos <= 0) return NULL;
	const wchar_t *it = elem->getId();
	pos--;
	SkinCursorElement *aelem = skinCursorList.enumItem(pos);
	if (aelem == NULL) return NULL;
	const wchar_t *ait = aelem->getId();
	if (!WCSICMP(it, ait)) return aelem;
	return NULL;
}

const wchar_t *PaletteManager::getSkinCursorBitmapId(const wchar_t *cursor)
{
	int pos = getCursorElement(cursor);
	if (pos < 0) return NULL;
	SkinCursorElement *sce = enumCursorElement(pos);
	return sce->getBitmapId();
}

void PaletteManager::AddBitmap(const wchar_t *id, const wchar_t *filename, const wchar_t *path, int x, int y, int w, int h,  ifc_xmlreaderparams *params, const wchar_t *colorgroup)
{
	skinBitmapList.addItem(new SkinBitmapElement(id, filename, path, x, y, w, h, params, skinpart_iterator, genericcounter++, colorgroup));
}


int PaletteManager::getBitmapElement(const wchar_t *type)
{
	if (type == NULL)
		return -1;
	const wchar_t *alias = getElementAlias(type);
	if (alias != NULL)
	{
		int pos;
		SkinBitmapElement *sbe = skinBitmapList.findLastItem(alias, &pos);
		if (sbe == NULL) return -1;
		return pos;
	}
	else
	{
		int pos;
		SkinBitmapElement *sbe = skinBitmapList.findLastItem(type, &pos);
		if (sbe == NULL) return -1;
		return pos;
	}
}

RegionServer *PaletteManager::requestSkinRegion(const wchar_t *id)
{
	int n = getBitmapElement(id);
	if (n == -1) return NULL;

	SkinBitmapElement *el = skinBitmapList.enumItem(n);
	//  if (el->region != NULL) el->region->getRegion()->debug();
	return el->getRegionServer();
}

void PaletteManager::cacheSkinRegion(const wchar_t *id, api_region *r)
{
	int n = getBitmapElement(id);
	if (n == -1) return ;
	SkinBitmapElement *el = skinBitmapList.enumItem(n);
	ASSERT(el != NULL);
	if (el->getRegionServer() != NULL)
	{
		DebugString("Trying to cache a region but cache is already set!\n");
		return ;
	}
	el->setRegionServer(new ElementRegionServer(r));
	//el->region->getRegion()->debug();
}

SkinItem *PaletteManager::getBitmapAncestor(SkinItem *item)
{
	SkinBitmapElement *elem = static_cast<SkinBitmapElement *>(item);
	int pos = skinBitmapList.searchItem(elem);
	if (pos <= 0) return NULL;
	const wchar_t *it = elem->getId();
	pos--;
	SkinBitmapElement *aelem = skinBitmapList.enumItem(pos);
	if (aelem == NULL) return NULL;
	const wchar_t *ait = aelem->getId();
	if (!WCSICMP(it, ait)) return aelem;
	return NULL;
}

SkinBitmapElement *PaletteManager::enumBitmapElement(int n)
{
	return skinBitmapList.enumItem(n);
}

int PaletteManager::getNumBitmapElement()
{
	return skinBitmapList.getNumItems();
}

const wchar_t *PaletteManager::getSkinBitmapFilename(const wchar_t *id, int *x, int *y, int *w, int *h, const wchar_t **root_path, ifc_xmlreaderparams **params)
{
	int i = getBitmapElement(id); // can return skinBitmapList.getNumItems(), check for that.
	if (i < 0) return id;

	SkinBitmapElement *sbe = skinBitmapList.enumItem(i);

	if (i < skinBitmapList.getNumItems() && !WCSICMP(id, sbe->getId()))
	{
		if (x) *x = sbe->getX();
		if (y) *y = sbe->getY();
		if (w) *w = sbe->getW();
		if (h) *h = sbe->getH();
		if (params) *params = sbe->getParams();
		if (root_path) *root_path = sbe->getXmlRootPath();
		return sbe->getFilename();
	}
	return id;	//FUCKO: const
}





const wchar_t *PaletteManager::getGammaGroupFromId(const wchar_t *id)
{
	int i = getBitmapElement(id);
	if (i < 0)
		return NULL;
	return skinBitmapList[i]->getParams()->getItemValue(L"gammagroup");
}

int PaletteManager::getLayerFromId(const wchar_t *id)
{
	int i = getBitmapElement(id);
	if (i < 0) return 0;
	const wchar_t *a = skinBitmapList[i]->getParams()->getItemValue(L"layer");
	if (a == NULL) return 0;
	return WTOI(a);
}

void PaletteManager::onGarbageCollect()
{
	for (int i = 0;i < regsrvGC.getNumItems();i++)
	{
		ElementRegionServer *srv = regsrvGC.enumItem(i);
		if (srv->getNumRefs() == 0)
		{
			delete srv;
			regsrvGC.removeByPos(i);
			i--;
		}
	}
}

void PaletteManager::garbageCollectRegionServer(ElementRegionServer *rs)
{
	if (rs->getNumRefs() == 0)
	{
		delete rs;
		return ;
	}
	regsrvGC.addItem(rs);
}


int PaletteGC::gccb_onGarbageCollect()
{
	parent->onGarbageCollect();
	return 1;
}

#define CBCLASS PaletteManager
START_DISPATCH;
VCB(API_PALETTE_STARTTRANSACTION, StartTransaction)
VCB(API_PALETTE_ENDTRANSACTION, EndTransaction)
VCB(API_PALETTE_RESET, Reset)
CB(API_PALETTE_GETSKINPARTITERATORPTR,getSkinPartIteratorPtr)
CB(API_PALETTE_NEWSKINPART,newSkinPart)
CB(API_PALETTE_GETSKINPARTITERATOR,getSkinPartIterator)
VCB(API_PALETTE_UNLOADELEMENTS,UnloadElements)
VCB(API_PALETTE_ADDALIAS,AddAlias)
CB(API_PALETTE_GETELEMENTALIAS,getElementAlias)
CB(API_PALETTE_GETALIASANCESTOR,getAliasAncestor)
VCB(API_PALETTE_ADDCOLOR,AddColor)
CB(API_PALETTE_GETNUMCOLORELEMENTS,getNumColorElements)
CB(API_PALETTE_ENUMCOLORELEMENT,enumColorElement)
CB(API_PALETTE_GETCOLORELEMENTREF,getColorElementRef)
CB(API_PALETTE_GETCOLORANCESTOR,getColorAncestor)
CB(API_PALETTE_GETCOLORELEMENT,getColorElement)
VCB(API_PALETTE_ADDCURSOR,AddCursor)
CB(API_PALETTE_GETCURSORELEMENT,getCursorElement)
CB(API_PALETTE_GETCURSOR,getCursor)
CB(API_PALETTE_GETCURSORANCESTOR,getCursorAncestor)
CB(API_PALETTE_GETSKINCURSORBITMAPID,getSkinCursorBitmapId)
VCB(API_PALETTE_ADDBITMAP,AddBitmap)
CB(API_PALETTE_GETBITMAPELEMENT,getBitmapElement)
CB(API_PALETTE_GETBITMAPANCESTOR,getBitmapAncestor)
CB(API_PALETTE_GETNUMBITMAPELEMENT,getNumBitmapElement)
CB(API_PALETTE_GETSKINBITMAPFILENAME,getSkinBitmapFilename)
CB(API_PALETTE_GETGAMMAGROUPFROMID,getGammaGroupFromId)
CB(API_PALETTE_GETLAYERFROMID,getLayerFromId)
CB(API_PALETTE_REQUESTSKINREGION,requestSkinRegion)
VCB(API_PALETTE_CACHESKINREGION,cacheSkinRegion)
END_DISPATCH;
#undef CBCLASS