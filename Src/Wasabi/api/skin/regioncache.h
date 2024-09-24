#ifndef __REGIONCACHE_H
#define __REGIONCACHE_H

#include <bfc/string/StringW.h>
#include <tataki/region/region.h>
#include <bfc/ptrlist.h>

class CacheRegionServer : public RegionServerI {
  public:
    CacheRegionServer(api_region *r) {
      reg = new RegionI(r->getOSHandle());
    }
    ~CacheRegionServer() {
      delete reg;
    }

  virtual api_region *getRegion() {
    return reg;
  }
  
  private:
    RegionI *reg;
};

struct RegionCacheItem {
  RegionCacheItem(const wchar_t *filename) : region(NULL) { }
  virtual ~RegionCacheItem() { }
	StringW filename;
  CacheRegionServer *region;
};

class SortRegionCacheItem {
public:
  static int compareItem(RegionCacheItem *p1, RegionCacheItem *p2) {
    return WCSICMP(p1->filename, p2->filename);
  }
  static int compareAttrib(const wchar_t *attrib, RegionCacheItem *item) {
    return WCSICMP(attrib, item->filename);
  }
};


class RegionCache {
  public:
  
  static RegionServer *requestSkinRegion(const wchar_t *id);
  static void cacheSkinRegion(const wchar_t *id, api_region *r);

  static PtrListQuickSorted<RegionCacheItem, SortRegionCacheItem> cache;
  static int getNumCaches() { return cache.getNumItems(); }
};

#endif