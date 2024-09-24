#ifndef __IMGLDR_H
#define __IMGLDR_H

//CUT
#define FT_UNKNOWN 0
#define FT_BMP		 1
#define FT_JPEG		 5
#define FT_PNG		 6

#include <bfc/ptrlist.h>
#include <tataki/bitmap/bitmap.h>
#ifdef WASABI_COMPILE_XMLPARSER
#include <api/xml/xmlreader.h>
#include <api/xml/xmlparamsi.h>
#else
class XmlReaderParamsI;
#endif
#ifdef WASABI_COMPILE_SKIN
#include <api/skin/skin.h>
#include <api/skin/skinfilter.h>
#endif

#include <bfc/string/StringW.h>

typedef struct {
  ARGB32 *bitmapbits;
  StringW filename;
  int usageCount;
  bool has_alpha;
  int width;
  int height;
  StringW includepath;
//  String rootpath;
  XmlReaderParamsI params;
  StringW original_element_id;
  StringW fullpathfilename;
} skinCacheEntry;

typedef unsigned long ARGB32;

#ifdef WASABI_COMPILE_SKIN

class skinCacheComp 
{
public:
  static int compareItem(void *p1, void *p2) {
    return wcscmp(((skinCacheEntry *)p1)->fullpathfilename, ((skinCacheEntry *)p2)->fullpathfilename);
  }
  static int compareAttrib(const wchar_t *attrib, void *item) {
    return wcscmp(attrib, ((skinCacheEntry *)item)->fullpathfilename);
  }
};

#endif //WASABI_COMPILE_SKIN

class imageLoader 
{
public:
	static ARGB32 *makeBmp(const wchar_t *filename, const wchar_t *path, int *has_alpha, int *w, int *h, ifc_xmlreaderparams *params, bool addmem, int *force_nocache);
#ifdef _WIN32
  static ARGB32 *makeBmp(OSMODULEHANDLE hInst, int id, int *has_alpha, int *w, int *h, const wchar_t *forcegroup=NULL);
#endif
	static int getFileType(uint8_t *pData);
  static StringW getWallpaper();
  static void release(ARGB32 *bitmapbits);
#ifdef WASABI_COMPILE_SKIN
	static ARGB32 *requestSkinBitmap(const wchar_t *id, int *has_alpha, int *x, int *y, int *subw, int *subh, int *w, int *h, int cached=1);
	static void releaseSkinBitmap(ARGB32 *bmpbits);
	static void applySkinFilters();
	static void applySkinFilters(skinCacheEntry *entry);
  static ARGB32 filterSkinColor(ARGB32 color, const wchar_t *elementid, const wchar_t *groupname);
  static int layerEqual(const wchar_t *id1, const wchar_t *id2);
#endif //WASABI_COMPILE_SKIN
  static int getMemUsage() { return totalMemUsage; }
  static int getNumCached() { return skinCacheList.getNumItems(); }
private:
#ifdef WASABI_COMPILE_SKIN
  //static int paramsMatch(ifc_xmlreaderparams *a, ifc_xmlreaderparams *b);
	static PtrListInsertMultiSorted<skinCacheEntry,skinCacheComp> skinCacheList;
#endif //WASABI_COMPILE_SKIN
	static void optimizeHasAlpha(ARGB32 *bits, int len, int *has_alpha);
	static void addMemUsage(const wchar_t *filename, int size);
	static void subMemUsage(int size);
	static int totalMemUsage;
};
#endif
