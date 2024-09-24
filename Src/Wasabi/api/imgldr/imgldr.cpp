#include <precomp.h>
#include <setjmp.h>
#include <bfc/bfc_assert.h>
#include <api.h>

#include <bfc/wasabi_std.h>
#include <tataki/bitmap/bitmap.h>
#include <api/skin/skinfilter.h>	// ApplySkinFilters

#include "imgldr.h"
#ifdef _WIN32
#include <api/imgldr/winbmp.h>
#endif
#include <api/imgldr/skinbmps.h>

#include <api/skin/skinparse.h>
#include <api/skin/skinelem.h>
#include <api/skin/gammamgr.h>

#include <api/service/svcs/svc_skinfilter.h>
#include <api/service/svcs/svc_imgload.h>
#include <api/service/svcs/svc_imggen.h>
#include "ImgLoaderEnum.h"
#include <api/memmgr/api_memmgr.h>
#include <bfc/string/PathString.h>
#include <api/locales/localesmgr.h>

//#define DEBUG_OUTPUT

#define IMAGEHEADERLEN 256

#ifdef _WIN32
ARGB32 *imageLoader::makeBmp(OSMODULEHANDLE hInst, int id, int *has_alpha, int *w, int *h, const wchar_t *forcegroup)
{
	ARGB32 *bits = makeBmp(StringPrintfW(L"res://%u,%i", hInst, id), NULL, has_alpha, w, h, NULL, TRUE, NULL);
	if (bits && *w > 0 && *h > 0)
	{
		ApplySkinFilters::apply(StringPrintfW(L"resource:%x,%d", hInst, id), forcegroup, bits, *w, *h);
	}
	return bits;
}
#endif

StringW imageLoader::getWallpaper()
{
	StringW ret(L"");
#ifdef WIN32
	HKEY hkey;
	static wchar_t file[MAX_PATH];
	file[0] = 0;
	if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop"), &hkey) == ERROR_SUCCESS)
	{
		unsigned long len = MAX_PATH;
		RegQueryValueExW(hkey, L"Wallpaper", 0, NULL, (unsigned char *)&file, &len);
		RegCloseKey(hkey);
	}
	if (file[0] && GetFileAttributesW(file) != (DWORD) - 1) ret = file;
#endif
	return ret;
}
extern StringW g_resourcepath;

ARGB32 *imageLoader::makeBmp(const wchar_t *_filename, const wchar_t *path, int *has_alpha, int *w, int *h, ifc_xmlreaderparams *params, bool addMem, int *force_nocache)
{
	if (!_filename || !*_filename)
		return 0;

	
	ARGB32 *ret = NULL;

	if (has_alpha != NULL) 
		*has_alpha = 0;	//BU

	// test image generator services FIRST
	ImgGeneratorEnum ige(_filename);
	svc_imageGenerator *gen;
	while ((gen = ige.getNext()) != NULL)
	{
		ret = gen->genImage(_filename, has_alpha, w, h, params);
		int cacheable = gen->outputCacheable();
		ige.release(gen);
		if (ret != NULL)
		{
			ApplySkinFilters::apply(params->getItemValue(L"id"), params->getItemValue(L"gammagroup"), ret, *w, *h);
			if (addMem) addMemUsage(_filename, (*w) * (*h) * sizeof(int));
			optimizeHasAlpha(ret, *w * *h, has_alpha);
			// don't try to cache generated images
			if (force_nocache) *force_nocache = !cacheable;
			return ret;
		}
	}

	StringW wallpaper;
	if (!WCSICMP(_filename, L"$wallpaper"))
	{
		wallpaper = getWallpaper();
		_filename = wallpaper.getValue();
	}

	MemBlock<uint8_t> mem;

	wchar_t olddir[PATH_MAX] = {0};
	Wasabi::Std::getCurDir(olddir, PATH_MAX);
	Wasabi::Std::setCurDir(WASABI_API_APP->path_getAppPath());

	StringW file;

	// benski> try language pack first
	StringPathCombine skinLocalePath(LocalesManager::getLocaleRoot(), WASABI_API_SKIN->getSkinName());
	file.swap(StringPathCombine(skinLocalePath, _filename));
	OSFILETYPE in = WFOPEN(file, WF_READONLY_BINARY);

	if (in == OPEN_FAILED)
	{
		// try the language pack's folder itself before falling back to the resource path
		file.swap(StringPathCombine(LocalesManager::getLocaleRoot(), _filename));
		in = WFOPEN(file, WF_READONLY_BINARY);
	}

	if (in == OPEN_FAILED)
	{
		if (path)
		{
			file.swap(StringPathCombine(path, _filename));
			in = WFOPEN(file, WF_READONLY_BINARY);
		}
		else
			in = WFOPEN(file=_filename, WF_READONLY_BINARY);
	}

#ifdef WASABI_COMPILE_SKIN
	if (in == OPEN_FAILED)
	{
		file.swap(StringPathCombine(WASABI_API_SKIN->getSkinPath(), _filename));
		in = WFOPEN(file, WF_READONLY_BINARY);
	}

#if 0 // this isn't used in gen_ff, basically makes it look in C:/Program Files/Winamp/Skins/Default/
	if (in == OPEN_FAILED)
	{
		file.swap(StringPathCombine(Skin::getDefaultSkinPath(), _filename));
		in = WFOPEN(file, WF_READONLY_BINARY);
	}
#endif

	// look in the fallback stuff (in Winamp5, this is c:/program files/winamp/plugins/freeform/xml)
	if (in == OPEN_FAILED)
	{
		file.swap(StringPathCombine(g_resourcepath, _filename));
		in = WFOPEN(file, WF_READONLY_BINARY);
	}
#endif

	if (in == OPEN_FAILED && path)
	{
		in = WFOPEN(file = _filename, WF_READONLY_BINARY);
	}

	Wasabi::Std::setCurDir(olddir);

	if (in != OPEN_FAILED)
	{
		int filelen = (int)FGETSIZE(in);
		if (filelen > 0)
		{
			mem.setSize(filelen);
			int len = FREAD(mem, 1, mem.getSizeInBytes(), in);
			if (len == filelen)
			{
				svc_imageLoader *loader = ImgLoaderEnum(mem, len).getNext();
				if (loader != NULL)
				{
					ret = loader->loadImage(mem, mem.getSizeInBytes(), w, h, params);
					if (ret != NULL)
					{
						if (addMem) 
							addMemUsage(file, (*w) * (*h) * sizeof(ARGB32));
						optimizeHasAlpha(ret, *w * *h, has_alpha);
					}
					SvcEnum::release(loader);
				}
			}
		} // filelen > 0
		FCLOSE(in);
	} // if file opened
	if (ret != NULL)
	{
		return ret;
	}
	else if (in != OPEN_FAILED && mem)
	{
		int m = getFileType(mem);
#ifdef WIN32
		switch (m)
		{
		case FT_BMP:
			{
				wchar_t tmpname[WA_MAX_PATH] = L"";

				// FG> if loading bmp from disk, no need to do the copy to disk
				HBITMAP hbmp = (HBITMAP)LoadImageW(0, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

				if (!hbmp)
				{
					// CT> extract/copy the file into temp directory (so we don't have any trouble if the file
					// is in a ZIP file). this whole copying thing will go away as soon as we'll get rid of
					// the LoadImage win32 function and use our own bmp loading functions

					GetTempPathW(WA_MAX_PATH, tmpname);
					wcscat(tmpname, L"wa3tmp");
					OSFILETYPE fs = WFOPEN(file, WF_READONLY_BINARY);
					if (fs != OPEN_FAILED)
					{
						OSFILETYPE fd = WFOPEN(tmpname, L"wb");
						int l;
						do
						{
							char buf[1024] = {0};
							l = FREAD(buf, 1, sizeof(buf), fs);
							if (l > 0) FWRITE(buf, 1, l, fd);
						}
						while (l > 0);
						FCLOSE(fs);
						FCLOSE(fd);
						hbmp = (HBITMAP)LoadImageW(0, tmpname, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
					}
					if (!hbmp)
					{
#ifdef WASABI_COMPILE_SKIN_WA2
						// bitmap not found or broken (like in the netscape skin)
						// try to find it in the Classic skin (wa2)
						StringW wa2skinFn = WASABI_API_APP->getSkinsPath();
						wa2skinFn.AppendPath("Classic");
						wa2skinFn.AppendPath(_filename);
						fs = WFOPEN(wa2skinFn), WF_READONLY_BINARY);
						if (fs != OPEN_FAILED)
						{
							OSFILETYPE fd = WFOPEN(tmpname, L"wb");
							int l;
							do
							{
								char buf[1024] = {0};
								l = FREAD(buf, 1, sizeof(buf), fs);
								if (l > 0) FWRITE(buf, 1, l, fd);
							}
							while (l > 0);
							FCLOSE(fs);
							FCLOSE(fd);
							hbmp = (HBITMAP)LoadImageW(0, tmpname, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
						}
#endif //WASABI_COMPILE_SKIN_WA2
					}
					if (!hbmp)
					{
						// no luck :(
						_wunlink(tmpname);
						return 0;
					}
				}

				BITMAP bm;
				HDC hMemDC, hMemDC2;
				HBITMAP hprev, hprev2;
				HBITMAP hsrcdib;
				void *srcdib;
				BITMAPINFO srcbmi = {0, };
				int r = GetObject(hbmp, sizeof(BITMAP), &bm);
				ASSERT(r != 0);
				*w = bm.bmWidth;
				*h = ABS(bm.bmHeight);

				ARGB32 *newbits;
				srcbmi.bmiHeader.biSize = sizeof(srcbmi.bmiHeader);
				srcbmi.bmiHeader.biWidth = *w;
				srcbmi.bmiHeader.biHeight = -*h;
				srcbmi.bmiHeader.biPlanes = 1;
				srcbmi.bmiHeader.biBitCount = 32;
				srcbmi.bmiHeader.biCompression = BI_RGB;

				hMemDC = CreateCompatibleDC(NULL);
				hMemDC2 = CreateCompatibleDC(NULL);
				hsrcdib = CreateDIBSection(hMemDC, &srcbmi, DIB_RGB_COLORS, &srcdib, NULL, 0);
				ASSERTPR(hsrcdib != 0, "CreateDIBSection() failed #69");
				hprev2 = (HBITMAP) SelectObject(hMemDC2, hbmp);
				hprev = (HBITMAP) SelectObject(hMemDC, hsrcdib);
				BitBlt(hMemDC, 0, 0, *w, *h, hMemDC2, 0, 0, SRCCOPY);
				newbits = (ARGB32*)MALLOC((*w) * (*h) * sizeof(ARGB32));
				MEMCPY32(newbits, srcdib, (*w)*(*h) /**sizeof(ARGB32)*/);
				{
					// put the alpha channel to 255
					unsigned char *b = (unsigned char *)newbits;
					int l = (*w) * (*h);
					for (int i = 0;i < l;i++)
						b[(i*4) + 3] = 0xff;
				}
				SelectObject(hMemDC, hprev);
				SelectObject(hMemDC2, hprev2);
				DeleteObject(hsrcdib);
				DeleteDC(hMemDC2);
				DeleteDC(hMemDC);

				DeleteObject(hbmp);

				if (tmpname[0]) 
					_wunlink(tmpname);	// destroy temp extraction

				if (addMem) 
					addMemUsage(file, (*w)*(*h)*4);
				return newbits;
			}
		}
#endif
	}
	return ret;
}

int imageLoader::getFileType(unsigned char *pData)
{
	// Bmp ?
#ifdef WIN32
	WINBITMAPFILEHEADER * pBFH;
	pBFH = (WINBITMAPFILEHEADER *) pData;
#ifdef _WINDOWS
	if (pBFH->bfType == 0x4d42)
#else
	if (pBFH->bfType == 0x424d)
#endif
		return FT_BMP;
#endif
	return FT_UNKNOWN;
}

#ifdef WASABI_COMPILE_SKIN

ARGB32 *imageLoader::requestSkinBitmap(const wchar_t *id, int *has_alpha, int *x, int *y, int *subw, int *subh, int *w, int *h, int cached)
{
	ifc_xmlreaderparams *params = NULL;
	const wchar_t *rootpath = NULL;

	const wchar_t *aliastarget = WASABI_API_PALETTE->getElementAlias(id);

	if (aliastarget)
		id = aliastarget;

	const wchar_t *efile = WASABI_API_PALETTE->getSkinBitmapFilename(id, x, y, subw, subh, &rootpath, &params);

	if (x && *x == -1) *x = 0;
	if (y && *y == -1) *y = 0;
	
	if (!efile) 
		efile = id;

	if (cached)
	{
		StringPathCombine f(rootpath, efile);
		f.toupper();
		f.FixSlashes();

		int pos = -1;
		/*skinCacheEntry *entry = */skinCacheList.findItem(f.getValue(), &pos);
		if (pos != -1)
		{
			//find first one
			while (pos > 0 && !wcscmp(skinCacheList[pos - 1]->fullpathfilename, f)) pos--;
			do
			{
				skinCacheEntry *entry = skinCacheList[pos];
				if (GammaMgr::gammaEqual(entry->original_element_id, id) && layerEqual(entry->original_element_id, id))
				{
					entry->usageCount++;
					if (w) *w = entry->width;
					if (h) *h = entry->height;
					if (has_alpha) *has_alpha = entry->has_alpha;
					return entry->bitmapbits;
				}
				pos++;
				if (pos >= skinCacheList.getNumItems()) break;
			}
			while (!wcscmp(skinCacheList[pos]->fullpathfilename, f));
		}
	}

	int force_nocache = 0;
	int t_has_alpha = 0;
	ARGB32 *bits = makeBmp(efile, rootpath, &t_has_alpha, w, h, params, TRUE, &force_nocache);
	if (has_alpha != NULL) *has_alpha = t_has_alpha;

	if (!bits)
		return NULL;

	if (force_nocache || !cached) return bits;

	skinCacheEntry *cachedbmp = new skinCacheEntry;

	if (params)
	{
		for (size_t i = 0;i != params->getNbItems();i++)
			cachedbmp->params.addItem(params->getItemName(i), params->getItemValue(i));
	}

	cachedbmp->usageCount = 1;
	cachedbmp->bitmapbits = bits;
	cachedbmp->filename = efile;
	cachedbmp->has_alpha = !!t_has_alpha;
	cachedbmp->width = *w;
	cachedbmp->height = *h;
	cachedbmp->original_element_id = id;

	//needed for findItem above
	StringPathCombine b(rootpath, efile);
	b.toupper();
	b.FixSlashes();
	cachedbmp->fullpathfilename.swap(b);

	applySkinFilters(cachedbmp);
	skinCacheList.addItem(cachedbmp);

	return cachedbmp->bitmapbits;
}
/*
int imageLoader::paramsMatch(ifc_xmlreaderparams *a, ifc_xmlreaderparams *b)
{
	if (!a && !b) return 1;
	if ((!a && b) || (!b && a)) return 0;
	for (int i = 0;i < a->getNbItems();i++)
	{
		const wchar_t *name = a->getItemName(i);
		if (!_wcsicmp(name, L"w") || !_wcsicmp(name, L"h") || !_wcsicmp(name, L"x") || !_wcsicmp(name, L"y")) continue;
		if (_wcsicmp(a->getItemValue(i), b->getItemValue(name)))
			return 0;
	}
	return 1;
}
*/
int imageLoader::layerEqual(const wchar_t *id1, const wchar_t *id2)
{
	int a = WASABI_API_PALETTE->getLayerFromId(id1);
	int b = WASABI_API_PALETTE->getLayerFromId(id2);
	return (a == b);
}

void imageLoader::releaseSkinBitmap(ARGB32 *bitmapbits)
{ //FG
	int i;
	// TODO: add critical sections

	int ni = skinCacheList.getNumItems();
	for (i = 0;i < ni;i++)
	{
		skinCacheEntry *entry = skinCacheList.enumItem(i);
		if (entry->bitmapbits == bitmapbits)
		{
			entry->usageCount--;
			if (entry->usageCount == 0)
			{
				subMemUsage(entry->width*entry->height*sizeof(int));
				WASABI_API_MEMMGR->sysFree(entry->bitmapbits);
				skinCacheList.removeByPos(i);
				delete entry;
				if (skinCacheList.getNumItems() == 0) skinCacheList.removeAll();
			}
			return ;
		}
	}
	// bitmap was not a cached skin bitmap, simply free it
	release(bitmapbits);
}

#endif //WASABI_COMPILE_SKIN

void imageLoader::release(ARGB32 *bitmapbits)
{
	WASABI_API_MEMMGR->sysFree(bitmapbits);
}

void imageLoader::optimizeHasAlpha(ARGB32 *bits, int len, int *has_alpha)
{
	if (len <= 0 || has_alpha == NULL) return ;
	for (*has_alpha = 0; len; len--, bits++)
	{
		ARGB32 v = *bits;
		unsigned int alpha = v >> 24;
		if (alpha != 255)
		{
			*has_alpha = 1;
			break;
		}
	}
}

#ifdef WASABI_COMPILE_SKIN

void imageLoader::applySkinFilters()
{ //FG
	int i;

	Skin::unloadResources();
	for (i = 0; i < skinCacheList.getNumItems(); i++)
	{
		skinCacheEntry *entry = skinCacheList.q(i);
		applySkinFilters(entry);
	}

	WASABI_API_PALETTE->newSkinPart();
	Skin::reloadResources();
}

void imageLoader::applySkinFilters(skinCacheEntry *entry)
{
	ASSERT(entry != NULL);

	int w = entry->width, h = entry->height;

	ApplySkinFilters::apply(entry->original_element_id, NULL, (ARGB32*)entry->bitmapbits, w, h);
}

ARGB32 imageLoader::filterSkinColor(ARGB32 color, const wchar_t *element_id, const wchar_t *groupname)
{
	SkinFilterEnum sfe;

	svc_skinFilter *obj;
	while (1)
	{
		obj = sfe.getNext(FALSE);
		if (!obj) break;
		color = obj->filterColor(color, element_id, groupname);
		sfe.getLastFactory()->releaseInterface(obj);
	}

	return color;
}

#endif //WASABI_COMPILE_SKIN

void imageLoader::addMemUsage(const wchar_t *filename, int size)
{
	totalMemUsage += size;
#ifdef DEBUG_OUTPUT
	DebugStringW("Bitmaps memory usage : %s - %d\n",  filename, totalMemUsage);
#endif
}

void imageLoader::subMemUsage(int size)
{
	totalMemUsage -= size;
}

#ifdef WASABI_COMPILE_SKIN

//PtrList<skinCacheEntry> imageLoader::skinCacheList;
PtrListInsertMultiSorted<skinCacheEntry, skinCacheComp> imageLoader::skinCacheList;
#endif //WASABI_COMPILE_SKIN

int imageLoader::totalMemUsage = 0;