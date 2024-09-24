#include "precomp.h"
#include "regioncache.h"


RegionServer *RegionCache::requestSkinRegion(const wchar_t *filename) 
{
  int n = -1;
  cache.findItem(filename, &n);
  if (n == -1) return NULL;

  RegionCacheItem *el = cache.enumItem(n);
//  if (el->region != NULL) el->region->getRegion()->debug();
  return el->region;
}

void RegionCache::cacheSkinRegion(const wchar_t *filename, api_region *r) 
{
  int n = -1;
  cache.findItem(filename, &n);
  if (n == -1) return;
  RegionCacheItem *el = cache.enumItem(n);
  ASSERT(el != NULL);
  if (el->region != NULL) {
    DebugString("Trying to cache a region but cache is already set!\n");
    return;
  }
  el->region = new CacheRegionServer(r);
  //el->region->getRegion()->debug();
}


PtrListQuickSorted<RegionCacheItem, SortRegionCacheItem> RegionCache::cache;
