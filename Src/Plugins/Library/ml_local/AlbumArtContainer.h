#ifndef NULLSOFT_ML_LOCAL_ALBUMARTCONTAINER_H
#define NULLSOFT_ML_LOCAL_ALBUMARTCONTAINER_H

#include <windows.h> // for HDC
#include <tataki/canvas/bltcanvas.h>

class AlbumArtContainer
{
public:
	enum CacheStatus
	{
		CACHE_UNKNOWN,
		CACHE_CACHED,
		CACHE_NOTFOUND,
		CACHE_LOADING,
	};

	AlbumArtContainer();
	enum
	{
		DRAW_SUCCESS,
		DRAW_NOART,
		DRAW_LOADING,
	};
	int drawArt(DCCanvas *pCanvas, RECT *prcDst);
	// benski> this definition is just temporary to get things going

	void AddRef();
	void Release();
	wchar_t *filename; // actually an NDE reference counted string
	MSG updateMsg;
	void SetCache(SkinBitmap *bitmap, CacheStatus status);
	void Reset();
private:
	~AlbumArtContainer();
	SkinBitmap * volatile  cache;

	volatile CacheStatus cached;
	size_t references;

};

#endif