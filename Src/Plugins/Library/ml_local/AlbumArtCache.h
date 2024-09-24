#ifndef NULLSOFT_ML_LOCAL_ALBUMARTCACHE_H
#define NULLSOFT_ML_LOCAL_ALBUMARTCACHE_H

#include "AlbumArtContainer.h"
void KillArtThread();
void CreateCache(AlbumArtContainer *container, int w, int h);
void FlushCache();
void ResumeCache();

void HintCacheSize(int _cachesize);

void ClearCache(const wchar_t *filename);
void DumpArtCache();
#endif