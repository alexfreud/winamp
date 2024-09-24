#include "AlbumArtContainer.h"
#include "api__ml_local.h"
#include <tataki/bitmap/bitmap.h>
#include <tataki/canvas/canvas.h>
#include <assert.h>
#include "AlbumArtCache.h"
#include "../nde/nde.h"
#include <commctrl.h>

AlbumArtContainer::AlbumArtContainer() :filename(0)
{
	references = 1;
	cached = CACHE_UNKNOWN;
	cache = 0;
	updateMsg.hwnd = NULL;
}

void AlbumArtContainer::AddRef()
{
	references++;
}

void AlbumArtContainer::Release()
{
	if (--references == 0)
		delete this;
}

AlbumArtContainer::~AlbumArtContainer()
{
	ndestring_release(filename);
	if (cache) cache->Release();
}

void AlbumArtContainer::Reset()
{
	if (cache)
		cache->Release();
	cache = 0;
	cached = AlbumArtContainer::CACHE_UNKNOWN;
}

void AlbumArtContainer::SetCache(SkinBitmap *bitmap, CacheStatus status)
{
	if (cache)
		cache->Release();
	cache = bitmap;
	cached = status;

	// lets post the update message
	if(updateMsg.hwnd && (CACHE_CACHED == cached || CACHE_UNKNOWN == cached)) PostMessageW(updateMsg.hwnd,updateMsg.message,updateMsg.wParam,updateMsg.lParam);
}

int AlbumArtContainer::drawArt(DCCanvas *pCanvas, RECT *prcDst)
{
	switch (cached)
	{
		case CACHE_NOTFOUND:
			return DRAW_NOART;
		case CACHE_LOADING:
			return DRAW_LOADING;
		case CACHE_UNKNOWN:
			cached = CACHE_LOADING;
			CreateCache(this, prcDst->right - prcDst->left, prcDst->bottom - prcDst->top);
			return DRAW_LOADING;
		case CACHE_CACHED:
		{
			if (cache->getWidth() != (prcDst->right - prcDst->left) || cache->getHeight() != (prcDst->bottom - prcDst->top))
			{
				cached = CACHE_LOADING;
				CreateCache(this, prcDst->right - prcDst->left, prcDst->bottom - prcDst->top);
				return DRAW_LOADING;
			}
			if (pCanvas) cache->blitAlpha(pCanvas, prcDst->left, prcDst->top);
			return DRAW_SUCCESS;
		}
		default:
			return DRAW_NOART; // shouldn't reach this;
	}
}