#ifndef __API_IMGLOADER_H
#define __API_IMGLOADER_H

#include <wasabicfg.h>
#include <bfc/dispatch.h>

class RegionServer;
class api_region;

class imgldr_api : public Dispatchable
{
public:
	ARGB32 *imgldr_makeBmp(const wchar_t *filename, int *has_alpha, int *w, int *h);
	ARGB32 *imgldr_makeBmp(OSMODULEHANDLE hInst, int id, int *has_alpha, int *w, int *h, const wchar_t *colorgroup = NULL);
	void imgldr_releaseBmp(ARGB32 *bmpbits);
	ARGB32 *imgldr_requestSkinBitmap(const wchar_t *file, int *has_alpha, int *x, int *y, int *subw, int *subh, int *w, int *h, int cached);
	RegionServer *imgldr_requestSkinRegion(const wchar_t *id);
	void imgldr_cacheSkinRegion(const wchar_t *id, api_region *r);
	void imgldr_releaseSkinBitmap(ARGB32 *bmpbits);

	DISPATCH_CODES
	{
	    IMGLDR_API_MAKEBMP = 0,
	    IMGLDR_API_MAKEBMP2 = 10,
	    IMGLDR_API_RELEASEBMP = 20,
	    IMGLDR_API_REQUESTSKINBITMAP = 30,
	    IMGLDR_API_REQUESTSKINREGION = 40,
	    IMGLDR_API_CACHESKINREGION = 50,
	    IMGLDR_API_RELEASESKINBITMAP = 60,
	};
};

inline ARGB32 *imgldr_api::imgldr_makeBmp(const wchar_t *filename, int *has_alpha, int *w, int *h)
{
	return _call(IMGLDR_API_MAKEBMP, (ARGB32 *)NULL, filename, has_alpha, w, h);
}

inline ARGB32 *imgldr_api::imgldr_makeBmp(OSMODULEHANDLE hInst, int id, int *has_alpha, int *w, int *h, const wchar_t *colorgroup)
{
	return _call(IMGLDR_API_MAKEBMP2, (ARGB32 *)NULL, hInst, id, has_alpha, w, h, colorgroup);
}

inline void imgldr_api::imgldr_releaseBmp(ARGB32 *bmpbits)
{
	_voidcall(IMGLDR_API_RELEASEBMP, bmpbits);
}

#ifdef WASABI_COMPILE_SKIN

inline ARGB32 *imgldr_api::imgldr_requestSkinBitmap(const wchar_t *file, int *has_alpha, int *x, int *y, int *subw, int *subh, int *w, int *h, int cached)
{
	return _call(IMGLDR_API_REQUESTSKINBITMAP, (ARGB32 *)NULL, file, has_alpha, x, y, subw, subh, w, h, cached);
}

inline RegionServer *imgldr_api::imgldr_requestSkinRegion(const wchar_t *id)
{
	return _call(IMGLDR_API_REQUESTSKINREGION, (RegionServer *)NULL, id);
}

inline void imgldr_api::imgldr_cacheSkinRegion(const wchar_t *id, api_region *r)
{
	_voidcall(IMGLDR_API_CACHESKINREGION, id, r);
}

inline void imgldr_api::imgldr_releaseSkinBitmap(ARGB32 *bmpbits)
{
	_voidcall(IMGLDR_API_RELEASESKINBITMAP, bmpbits);
}

#endif //WASABI_COMPILE_SKIN

class imgldr_apiI : public imgldr_api
{
public:
	virtual ARGB32 *imgldr_makeBmp(const wchar_t *filename, int *has_alpha, int *w, int *h) = 0;
#ifdef _WIN32
	virtual ARGB32 *imgldr_makeBmp2(OSMODULEHANDLE hInst, int id, int *has_alpha, int *w, int *h, const wchar_t *colorgroup = NULL) = 0;
#endif
	virtual void imgldr_releaseBmp(ARGB32 *bmpbits) = 0;
#ifdef WASABI_COMPILE_SKIN
	virtual ARGB32 *imgldr_requestSkinBitmap(const wchar_t *file, int *has_alpha, int *x, int *y, int *subw, int *subh, int *w, int *h, int cached) = 0;
	virtual RegionServer *imgldr_requestSkinRegion(const wchar_t *id) = 0;
	virtual void imgldr_cacheSkinRegion(const wchar_t *id, api_region *r) = 0;
	virtual void imgldr_releaseSkinBitmap(ARGB32 *bmpbits) = 0;
#endif //WASABI_COMPILE_SKIN

protected:
	RECVS_DISPATCH;
};

// {703ECC7C-B3D8-4e1e-B8B5-A7563D9D6F30}
static const GUID imgLdrApiServiceGuid =
    { 0x703ecc7c, 0xb3d8, 0x4e1e, { 0xb8, 0xb5, 0xa7, 0x56, 0x3d, 0x9d, 0x6f, 0x30 } };

extern imgldr_api *imgLoaderApi;


#endif
