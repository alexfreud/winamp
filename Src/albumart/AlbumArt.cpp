#include "AlbumArt.h"
#include "api__albumart.h"
#include "ParamList.h"
#include <api/service/svcs/svc_imgload.h>
#include <api/service/svcs/svc_imgwrite.h>
#include <shlwapi.h>
#include <api/service/waservicefactory.h>
#include "../Agave/AlbumArt/svc_albumArtProvider.h"
#include <api/syscb/callbacks/metacb.h>
#include <strsafe.h>

static svc_imageLoader *FindImageLoader(const wchar_t *filespec, waServiceFactory **factory)
{
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = (int)WASABI_API_SVC->service_getNumServices(imgload);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
		if (sf)
		{	
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if (l)
			{
				if (l->isMine(filespec))
				{
					*factory = sf;
					return l;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return NULL;
}

static svc_imageLoader *FindImageLoader(void *data, size_t datalen, waServiceFactory **factory)
{
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = (int)WASABI_API_SVC->service_getNumServices(imgload);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
		if (sf)
		{
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if (l)
			{
				if (l->testData(data, (int)datalen))
				{
					*factory = sf;
					return l;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return NULL;
}

static svc_albumArtProvider *FindProvider(const wchar_t *filename, int providerType, waServiceFactory **factory)
{
	FOURCC albumartprovider = svc_albumArtProvider::getServiceType();
	int n = (int)WASABI_API_SVC->service_getNumServices(albumartprovider);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(albumartprovider,i);
		if (sf)
		{
			svc_albumArtProvider * provider = (svc_albumArtProvider*)sf->getInterface();
			if (provider)
			{
				if (provider->ProviderType() == providerType && provider->IsMine(filename))
				{
					*factory = sf;
					return provider;
				}
				sf->releaseInterface(provider);
			}
		}
	}
	return NULL;
}

static ARGB32 *loadImgFromFile(const wchar_t *file, int *w, int *h)
{
	waServiceFactory *sf = 0;
	svc_imageLoader *loader = FindImageLoader(file, &sf);
	if (loader)
	{
		HANDLE hf = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (hf != INVALID_HANDLE_VALUE)
		{
			int len = GetFileSize(hf, 0);
			HANDLE hmap = CreateFileMapping(hf, 0, PAGE_READONLY, 0, 0, 0);
			if (hmap)
			{
				void *data = MapViewOfFile(hmap, FILE_MAP_READ, 0, 0, 0);
				if (data)
				{
					if (loader->testData(data,len))
					{
						ARGB32* im = loader->loadImage(data,len,w,h);
						UnmapViewOfFile(data);
						CloseHandle(hmap);
						CloseHandle(hf);
						sf->releaseInterface(loader);
						return im;
					}
					UnmapViewOfFile(data);
				}

				CloseHandle(hmap);
			}
			CloseHandle(hf);
		}
		sf->releaseInterface(loader);
	}
	return 0;
}

static ARGB32 *loadImgFromFile(const wchar_t *path, const wchar_t *filespec, int *w, int *h, bool test=false, ifc_xmlreaderparams *params = NULL)
{
	waServiceFactory *sf = 0;
	svc_imageLoader *loader = FindImageLoader(filespec, &sf);
	if (loader)
	{
		if (test)
		{
			sf->releaseInterface(loader);
			return (ARGB32*)1;
		}
		wchar_t file[MAX_PATH] = {0};
		PathCombineW(file, path, filespec);
		HANDLE hf = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (hf != INVALID_HANDLE_VALUE)
		{
			int len = GetFileSize(hf, 0);
			HANDLE hmap = CreateFileMapping(hf, 0, PAGE_READONLY, 0, 0, 0);
			if (hmap)
			{
				void *data = MapViewOfFile(hmap, FILE_MAP_READ, 0, 0, 0);
				if (data)
				{
					if (loader->testData(data,len))
					{
						ARGB32* im = loader->loadImage(data,len,w,h, params);
						UnmapViewOfFile(data);
						CloseHandle(hmap);
						CloseHandle(hf);
						sf->releaseInterface(loader);
						return im;
					}
					UnmapViewOfFile(data);
				}

				CloseHandle(hmap);
			}
			CloseHandle(hf);
		}

		sf->releaseInterface(loader);
	}
	return 0;
}

static bool loadImgDataFromFile(const wchar_t *path, const wchar_t *filespec, void **bits, size_t *len, wchar_t **mimeType, bool originTest = false)
{
	waServiceFactory *sf = 0;
	svc_imageLoader *loader = FindImageLoader(filespec, &sf);
	if (loader)
	{
		wchar_t file[MAX_PATH] = {0};
		PathCombineW(file, path, filespec);
		HANDLE hf = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (hf != INVALID_HANDLE_VALUE)
		{
			if(!originTest)
			{
				int flen = GetFileSize(hf, 0);
				*bits = WASABI_API_MEMMGR->sysMalloc(flen);
				DWORD bytes_read = 0;
				ReadFile(hf, *bits, flen, &bytes_read, 0);
				*len = bytes_read;
			}
			if (mimeType)
			{
				*mimeType = (wchar_t *)WASABI_API_MEMMGR->sysMalloc(12 * sizeof(wchar_t));
				wcsncpy(*mimeType, loader->mimeType(), 12);
			}
			CloseHandle(hf);
			sf->releaseInterface(loader);
			return true;
		}
		sf->releaseInterface(loader);
	}	
	return false;
}

static ARGB32 *FindImage(const wchar_t *path, const wchar_t *mask, int *w, int *h, bool test=false, ifc_xmlreaderparams *params = NULL)
{
	wchar_t dirmask[MAX_PATH] = {0};
	PathCombineW(dirmask, path, mask);
	WIN32_FIND_DATAW find = {0};
	HANDLE hFind = FindFirstFileW(dirmask, &find);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			ARGB32 *bits = loadImgFromFile(path, find.cFileName, w, h, test, params);
			if (bits)
			{
				FindClose(hFind);
				return bits;
			}
		}
		while (FindNextFileW(hFind, &find));
		FindClose(hFind);
	}
	return 0;
}

static bool FindImageData(const wchar_t *path, const wchar_t *mask, void **bits, size_t *len, wchar_t **mimeType)
{
	wchar_t dirmask[MAX_PATH] = {0};
	PathCombineW(dirmask, path, mask);
	WIN32_FIND_DATAW find = {0};
	HANDLE hFind = FindFirstFileW(dirmask, &find);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (loadImgDataFromFile(path, find.cFileName, bits, len, mimeType))
			{
				return true;
			}
		}
		while (FindNextFileW(hFind, &find));
		FindClose(hFind);
	}
	return false;
}

static bool FindImageOrigin(const wchar_t *path, const wchar_t *mask, wchar_t **mimeType)
{
	wchar_t dirmask[MAX_PATH] = {0};
	PathCombineW(dirmask, path, mask);
	WIN32_FIND_DATAW find = {0};
	HANDLE hFind = FindFirstFileW(dirmask, &find);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (loadImgDataFromFile(path, find.cFileName, NULL, NULL, mimeType, true))
			{
				FindClose(hFind);
				return true;
			}
		}
		while (FindNextFileW(hFind, &find));
		FindClose(hFind);
	}
	return false;
}

static bool DeleteImage(const wchar_t *path, const wchar_t *mask)
{
	wchar_t dirmask[MAX_PATH] = {0};
	PathCombineW(dirmask, path, mask);
	WIN32_FIND_DATAW find = {0};
	HANDLE hFind = FindFirstFileW(dirmask, &find);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// i know this seems stupid, but we need to load the image
			// since this is supposed to delete the image that would show up
			// from a GetAlbumArt call
			int w = 0, h = 0;
			ARGB32 *bits = loadImgFromFile(path, find.cFileName, &w, &h);
			if (bits)
			{
				FindClose(hFind);
				WASABI_API_MEMMGR->sysFree(bits);
				wchar_t fullpath[MAX_PATH] = {0};
				PathCombineW(fullpath, path, find.cFileName);
				DeleteFileW(fullpath);
				return true;
			}
		}
		while (FindNextFileW(hFind, &find));
		FindClose(hFind);
	}
	return false;
}

static ARGB32 *FindAlbumArtByProvider(int providertype, const wchar_t *filename, const wchar_t *type, int *w, int *h, ifc_xmlreaderparams *params = NULL)
{
	waServiceFactory *factory = 0;
	svc_albumArtProvider *provider = FindProvider(filename, providertype, &factory);
	if (provider)
	{
		void *data = 0;
		size_t datalen = 0;
		wchar_t *mimeType=0;
		if (provider->GetAlbumArtData(filename, type, &data, &datalen, &mimeType) == ALBUMARTPROVIDER_SUCCESS && data && datalen)
		{
			waServiceFactory *sf;
			svc_imageLoader *loader = 0;
			if (mimeType)
			{
				wchar_t mask[MAX_PATH] = {0};
				StringCchPrintfW(mask, MAX_PATH, L"hi.%s", mimeType);
				WASABI_API_MEMMGR->sysFree(mimeType);
				loader = FindImageLoader(mask, &sf);
			}
			else
			{
				loader = FindImageLoader(data, datalen, &sf);
			}

			if (loader)
			{
				if (loader->testData(data, (int)datalen))
				{
					ARGB32* im = loader->loadImage(data, (int)datalen,w,h,params);
					WASABI_API_MEMMGR->sysFree(data);
					sf->releaseInterface(loader);
					factory->releaseInterface(provider);
					return im;
				}
				sf->releaseInterface(loader);
			}
			WASABI_API_MEMMGR->sysFree(data);
		}
		factory->releaseInterface(provider);
	}
	return 0;
}

static int DeleteAlbumArtByProvider(int providertype, const wchar_t *filename, const wchar_t *type)
{
	waServiceFactory *factory = 0;
	svc_albumArtProvider *provider = FindProvider(filename, providertype, &factory);
	if (provider)
	{
		int ret = provider->DeleteAlbumArt(filename, type);
		factory->releaseInterface(provider);
		return ret == ALBUMARTPROVIDER_SUCCESS;
	}
	return false;
}

static bool FindSceneNFO(const wchar_t *path, wchar_t *mask)
{
	wchar_t nfo_mask[MAX_PATH] = {0};
	PathCombineW(nfo_mask, path, L"*.nfo");

	WIN32_FIND_DATAW find = {0};
	HANDLE hFind = FindFirstFileW(nfo_mask, &find);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		StringCchCopyW(mask, MAX_PATH, find.cFileName);
		PathRemoveExtensionW(mask);
		StringCchCatW(mask, MAX_PATH, L".*");
		FindClose(hFind);
		return true;
	}
	return false;
}

static void CleanNameForPath(wchar_t *name)
{
	while (name && *name)
	{
		switch(*name)
		{
			case L'?':
			case L'*':
			case  L'|':
				*name = L'_';
				break;
			case '/':
			case L'\\':
			case L':':
				*name =  L'-';
				break;
			case L'\"': 
				*name  = L'\'';
				break;
			case L'<':
				*name  = L'(';
				break;
			case L'>': *name = L')';
				break;
		}
		name++;			
	}
}


int AlbumArt::GetAlbumArt(const wchar_t *filename, const wchar_t *type, int *w, int *h, ARGB32 **bits)
{
	if (!filename || !*filename)
		return ALBUMART_FAILURE;

	/* First, look for embedded album art */
	if (*bits = FindAlbumArtByProvider(ALBUMARTPROVIDER_TYPE_EMBEDDED, filename, type, w,h))
		return ALBUMART_SUCCESS;

	/* moved to allow for SHOUTcast 2 in-stream metadata which is best classed as embedded */
	if (PathIsURLW(filename))
		return ALBUMART_FAILURE;

	/* Next, Search the albumart in a cover library dir */
	if (*bits = FindAlbumArtByProvider(ALBUMARTPROVIDER_TYPE_DATABASE, filename, type, w,h))
		return ALBUMART_SUCCESS;

	bool isCover = !_wcsicmp(type,L"cover");
	/* Get the folder of the file */
	wchar_t path[MAX_PATH] = {0};
	wchar_t mask[MAX_PATH] = {0};
	StringCchCopyW(path, MAX_PATH, filename);
	PathRemoveFileSpecW(path);

	/* Next, look for a file with the same name as the album name */
	wchar_t albumname[MAX_PATH] = {0};
	if (isCover && AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"album", albumname, MAX_PATH) && albumname[0])
	{
		CleanNameForPath(albumname);
		StringCchPrintfW(mask, MAX_PATH, L"%s.*", albumname);
		if (*bits = FindImage(path, mask, w, h))
			return ALBUMART_SUCCESS;
	}

	// look for 'scene' artwork (*.jpg with the same filename as *.nfo)
	if (isCover && FindSceneNFO(path, mask))
	{
		if (*bits = FindImage(path, mask, w, h))
			return ALBUMART_SUCCESS;
	}

	/* Next, let's look in the folder for some art */
	StringCchPrintfW(mask, MAX_PATH, L"%s.*", type);
	if (*bits = FindImage(path, mask, w, h))
		return ALBUMART_SUCCESS;

	/* Look for folder.jpg if the type is "cover" */
	if (isCover && (*bits = FindImage(path, L"folder.*", w, h)))
		return ALBUMART_SUCCESS;

	/* Look for front.jpg if the type is "cover" */
	if (isCover && (*bits = FindImage(path, L"front.*", w, h)))
		return ALBUMART_SUCCESS;

	/* Look for albumart.jpg if the type is "cover" */
	if (isCover && (*bits = FindImage(path, L"albumart.*", w, h)))
		return ALBUMART_SUCCESS;

	return ALBUMART_FAILURE;
}

int AlbumArt::GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType)
{
	if (!filename || !*filename)
		return ALBUMART_FAILURE;
	if (PathIsURLW(filename))
		return ALBUMART_FAILURE;

	/* First, look for embedded album art */
	waServiceFactory *sourceFactory = 0;
	svc_albumArtProvider *sourceProvider = FindProvider(filename, ALBUMARTPROVIDER_TYPE_EMBEDDED, &sourceFactory);
	if (sourceProvider)
	{
		void *data = 0;
		size_t datalen = 0;
		wchar_t *mime_type = 0;
		if (sourceProvider->GetAlbumArtData(filename, type, &data, &datalen, &mime_type) == ALBUMARTPROVIDER_SUCCESS && data && datalen)
		{
			if (bits)
				*bits = data;
			if (len)
				*len = datalen;
			if (mimeType)
				*mimeType = mime_type;
			sourceFactory->releaseInterface(sourceProvider);
			return ALBUMART_SUCCESS;
		}
		sourceFactory->releaseInterface(sourceProvider);
	}

#if 0 // TODO
	/* Next, Search the albumart in a cover library dir */
	if (*bits = FindAlbumArtByProvider(ALBUMARTPROVIDER_TYPE_DATABASE, filename, type, w,h))
		return ALBUMART_SUCCESS;
#endif

	bool isCover = !_wcsicmp(type,L"cover");
	/* Get the folder of the file */
	wchar_t path[MAX_PATH] = {0};
	wchar_t mask[MAX_PATH] = {0};
	StringCchCopyW(path, MAX_PATH, filename);
	PathRemoveFileSpecW(path);

	/* Next, look for a file with the same name as the album name */
	wchar_t albumname[MAX_PATH] = {0};
	if (isCover && AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"album", albumname, MAX_PATH) && albumname[0])
	{
		CleanNameForPath(albumname);
		StringCchPrintfW(mask, MAX_PATH, L"%s.*", albumname);
		if (FindImageData(path, mask, bits, len, mimeType))
			return ALBUMART_SUCCESS;
	}

	// look for 'scene' artwork (*.jpg with the same filename as *.nfo)
	if (isCover && FindSceneNFO(path, mask))
	{
		if (FindImageData(path, mask, bits, len, mimeType))
			return ALBUMART_SUCCESS;
	}

	/* Next, let's look in the folder for some art */
	StringCchPrintfW(mask, MAX_PATH, L"%s.*", type);
	if (FindImageData(path, mask, bits, len, mimeType))
		return ALBUMART_SUCCESS;

	/* Look for folder.jpg if the type is "cover" */
	if (isCover && FindImageData(path, L"folder.*", bits, len, mimeType))
		return ALBUMART_SUCCESS;

	/* Look for front.jpg if the type is "cover" */
	if (isCover && FindImageData(path, L"front.*", bits, len, mimeType))
		return ALBUMART_SUCCESS;

	/* Look for albumart.jpg if the type is "cover" */
	if (isCover && FindImageData(path, L"albumart.*", bits, len, mimeType))
		return ALBUMART_SUCCESS;

	return ALBUMART_FAILURE;
}

int AlbumArt::GetAlbumArt_NoAMG(const wchar_t *filename, const wchar_t *type, int *w, int *h, ARGB32 **bits)
{
	ParamList params;
	params.addItem(L"AMG", L"1");

	if (!filename || !*filename)
		return ALBUMART_FAILURE;
	if (PathIsURLW(filename))
		return ALBUMART_FAILURE;

	/* First, look for embedded album art */
	if (*bits = FindAlbumArtByProvider(ALBUMARTPROVIDER_TYPE_EMBEDDED, filename, type, w,h, &params))
		return ALBUMART_SUCCESS;

	/* Next, Search the albumart in a cover library dir */
	if (*bits = FindAlbumArtByProvider(ALBUMARTPROVIDER_TYPE_DATABASE, filename, type, w,h, &params))
		return ALBUMART_SUCCESS;

	bool isCover = !_wcsicmp(type,L"cover");
	/* Get the folder of the file */
	wchar_t path[MAX_PATH] = {0};
	wchar_t mask[MAX_PATH] = {0};
	StringCchCopyW(path, MAX_PATH, filename);
	PathRemoveFileSpecW(path);

	/* Next, look for a file with the same name as the album name */
	wchar_t albumname[MAX_PATH] = {0};
	if (isCover && AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"album", albumname, MAX_PATH) && albumname[0])
	{
		CleanNameForPath(albumname);
		StringCchPrintfW(mask, MAX_PATH, L"%s.*", albumname);
		if (*bits = FindImage(path, mask, w, h, false, &params))
			return ALBUMART_SUCCESS;
	}

	// look for 'scene' artwork (*.jpg with the same filename as *.nfo)
	if (isCover && FindSceneNFO(path, mask))
	{
		if (*bits = FindImage(path, mask, w, h, false, &params))
			return ALBUMART_SUCCESS;
	}

	/* Next, let's look in the folder for some art */
	StringCchPrintfW(mask, MAX_PATH, L"%s.*", type);
	if (*bits = FindImage(path, mask, w, h, false, &params))
		return ALBUMART_SUCCESS;

	/* Look for folder.jpg if the type is "cover" */
	if (isCover && (*bits = FindImage(path, L"folder.*", w, h, false, &params)))
		return ALBUMART_SUCCESS;

	/* Look for front.jpg if the type is "cover" */
	if (isCover && (*bits = FindImage(path, L"front.*", w, h, false, &params)))
		return ALBUMART_SUCCESS;

	/* Look for albumart.jpg if the type is "cover" */
	if (isCover && (*bits = FindImage(path, L"albumart.*", w, h, false, &params)))
		return ALBUMART_SUCCESS;

	return ALBUMART_FAILURE;
}

int AlbumArt::GetAlbumArtOrigin(const wchar_t *filename, const wchar_t *type, wchar_t **mimeType)
{
	if (!filename || !*filename)
		return ALBUMART_NONE;
	if (PathIsURLW(filename))
		return ALBUMART_NONE;

	/* First, look for embedded album art */
	waServiceFactory *sourceFactory = 0;
	svc_albumArtProvider *sourceProvider = FindProvider(filename, ALBUMARTPROVIDER_TYPE_EMBEDDED, &sourceFactory);
	if (sourceProvider)
	{
		void *data = 0;
		size_t datalen = 0;
		wchar_t *mime_type = 0;
		if (sourceProvider->GetAlbumArtData(filename, type, &data, &datalen, &mime_type) == ALBUMARTPROVIDER_SUCCESS && data && datalen)
		{
			if (mimeType)
			{
				*mimeType = mime_type;
				if(!mime_type)
				{
					waServiceFactory *sf = 0;
					svc_imageLoader *loader = 0;
					loader = FindImageLoader(data, datalen, &sf);
					if (loader)
					{
						*mimeType = (wchar_t *)WASABI_API_MEMMGR->sysMalloc(16 * sizeof(wchar_t));
						wcsncpy(*mimeType, loader->mimeType(), 11);
						sf->releaseInterface(loader);
					}
					else
					{
						*mimeType = 0;
					}
				}
			}

			sourceFactory->releaseInterface(sourceProvider);
			WASABI_API_MEMMGR->sysFree(data);
			return ALBUMART_EMBEDDED;
		}
		sourceFactory->releaseInterface(sourceProvider);
	}

#if 0 // TODO
	/* Next, Search the albumart in a cover library dir */
	if (*bits = FindAlbumArtByProvider(ALBUMARTPROVIDER_TYPE_DATABASE, filename, type, w,h))
		return ALBUMART_SUCCESS;
#endif

	bool isCover = !_wcsicmp(type,L"cover");
	/* Get the folder of the file */
	wchar_t path[MAX_PATH] = {0};
	wchar_t mask[MAX_PATH] = {0};
	StringCchCopyW(path, MAX_PATH, filename);
	PathRemoveFileSpecW(path);

	/* Next, look for a file with the same name as the album name */
	wchar_t albumname[MAX_PATH] = {0};
	if (isCover && AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"album", albumname, MAX_PATH) && albumname[0])
	{
		CleanNameForPath(albumname);
		StringCchPrintfW(mask, MAX_PATH, L"%s.*", albumname);
		if (FindImageOrigin(path, mask, mimeType))
			return ALBUMART_ALBUM;
	}

	// look for 'scene' artwork (*.jpg with the same filename as *.nfo)
	if (isCover && FindSceneNFO(path, mask))
	{
		if (FindImageOrigin(path, mask, mimeType))
			return ALBUMART_NFO;
	}

	/* Next, let's look in the folder for some art */
	StringCchPrintfW(mask, MAX_PATH, L"%s.*", type);
	if (FindImageOrigin(path, mask, mimeType))
		return ALBUMART_FILENAME;

	/* Look for folder.jpg if the type is "cover" */
	if (isCover && FindImageOrigin(path, L"folder.*", mimeType))
		return ALBUMART_FOLDER;

	/* Look for front.jpg if the type is "cover" */
	if (isCover && FindImageOrigin(path, L"front.*", mimeType))
		return ALBUMART_FRONT;

	/* Look for albumart.jpg if the type is "cover" */
	if (isCover && FindImageOrigin(path, L"albumart.*", mimeType))
		return ALBUMART_ARTWORK;

	return ALBUMART_NONE;
}

// benski> TODO, i really don't like duplicating this logic from GetAlbumArt, maybe we can find a way
int AlbumArt::DeleteAlbumArt(const wchar_t *filename, const wchar_t *type)
{
	if (!filename || !*filename)
		return ALBUMART_FAILURE;
	if (PathIsURLW(filename))
		return ALBUMART_FAILURE;

	/* First, look for embedded album art */
	if (DeleteAlbumArtByProvider(ALBUMARTPROVIDER_TYPE_EMBEDDED, filename, type))
		return ALBUMART_SUCCESS;

	/* Next, Search the albumart in a cover library dir */
	if (DeleteAlbumArtByProvider(ALBUMARTPROVIDER_TYPE_DATABASE, filename, type))
		return ALBUMART_SUCCESS;

	bool isCover = !_wcsicmp(type,L"cover");
	/* Get the folder of the file */
	wchar_t path[MAX_PATH] = {0};
	wchar_t mask[MAX_PATH] = {0};
	StringCchCopyW(path, MAX_PATH, filename);
	PathRemoveFileSpecW(path);

	/* Next, look for a file with the same name as the album name */
	wchar_t albumname[MAX_PATH] = {0};
	if (AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"album", albumname, MAX_PATH) && albumname[0])
	{
		CleanNameForPath(albumname);
		StringCchPrintfW(mask, MAX_PATH, L"%s.*", albumname);
		if (DeleteImage(path, mask))
			return ALBUMART_SUCCESS;
	}

	// look for 'scene' artwork (*.jpg with the same filename as *.nfo)
	if (isCover && FindSceneNFO(path, mask))
	{
		if (DeleteImage(path, mask))
			return ALBUMART_SUCCESS;
	}

	/* Next, let's look in the folder for some art */
	StringCchPrintfW(mask, MAX_PATH, L"%s.*", type);
	if (DeleteImage(path, mask))
		return ALBUMART_SUCCESS;

	/* Look for folder.jpg if the type is "cover" */
	if (isCover && DeleteImage(path, L"folder.*"))
		return ALBUMART_SUCCESS;

	/* Look for folder.jpg if the type is "cover" */
	if (isCover && DeleteImage(path, L"front.*"))
		return ALBUMART_SUCCESS;

	/* Look for albumart.jpg if the type is "cover" */
	if (isCover && DeleteImage(path, L"albumart.*"))
		return ALBUMART_SUCCESS;

	return ALBUMART_FAILURE;
}

class strbuilder
{
public:
	wchar_t *str;
	wchar_t *end;
	size_t alloc;
	strbuilder()
	{
		alloc = 512;
		end = str = (wchar_t*)WASABI_API_MEMMGR->sysMalloc(alloc);
		str[0]=str[1]=0;
	}
	void append(const wchar_t *s)
	{
		size_t oldlen = end - str;
		size_t l = wcslen(s) + 1;
		while (alloc < l + oldlen + 1)
		{
			alloc += 512;
			str = (wchar_t*)WASABI_API_MEMMGR->sysRealloc(str,alloc);
			end = str+oldlen;
		}
		lstrcpynW(end,s, (int)l);
		end += l;
		*end=0;
	}
	wchar_t *get()
	{
		return str;
	}
};

int AlbumArt::GetAlbumArtTypes(const wchar_t *filename, wchar_t **types)
{
	if (!filename || !*filename)
		return ALBUMART_FAILURE;
	if (PathIsURLW(filename))
		return ALBUMART_FAILURE;

	wchar_t path[MAX_PATH] = {0};
	wchar_t mask[MAX_PATH] = {0};
	strbuilder str;

	/* First, let's look in the folder for some art */
	StringCchCopyW(path, MAX_PATH, filename);
	PathRemoveFileSpecW(path);

	// TODO (mpdeimos) Make the stuff here configurable, add wildcards *front* / *cover* ...
	StringCchPrintfW(mask, MAX_PATH, L"cover.*");
	if (FindImage(path, mask, 0, 0,true))
		str.append(L"cover");
	else // mpdeimos> front.jpg is much more common than cover.jpg
	{
		StringCchPrintfW(mask, MAX_PATH, L"front.*");
		if (FindImage(path, mask, 0, 0,true))
			str.append(L"cover");
		else
		{
			StringCchPrintfW(mask, MAX_PATH, L"folder.*");
			if (FindImage(path, mask, 0, 0,true))
				str.append(L"cover");
			else
			{
				StringCchPrintfW(mask, MAX_PATH, L"albumart.*");
				if (FindImage(path, mask, 0, 0,true))
					str.append(L"cover");
				else
				{
					wchar_t albumname[MAX_PATH]=L"";
					if (AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"album", albumname, MAX_PATH) && albumname[0])
					{
						CleanNameForPath(albumname);
						StringCchPrintfW(mask, MAX_PATH, L"%s.*", albumname);
						if (FindImage(path, mask, 0, 0))
							str.append(L"cover");
					}
				}
			}
		}
	}
	// add other shit to str

	//str.append(L"foo");
	//str.append(L"bar");

	*types = str.get();
	return ALBUMART_SUCCESS;
}

int AlbumArt::GetValidAlbumArtTypes(const wchar_t *filename, wchar_t **type)
{
	if (!filename || !*filename)
		return ALBUMART_FAILURE;
	if (PathIsURLW(filename))
		return ALBUMART_FAILURE;

	strbuilder str;
	str.append(L"cover");

	// add other shit to str
	//str.append(L"foo");
	//str.append(L"bar");

	*type = str.get();
	return ALBUMART_SUCCESS;
}

static void * writeImg(const ARGB32 *data, int w, int h, int *length, const wchar_t *ext)
{
	if (!ext || (ext && !*ext)) return NULL;
	if (*ext == L'.') ext++;
	FOURCC imgwrite = svc_imageWriter::getServiceType();
	int n = (int)WASABI_API_SVC->service_getNumServices(imgwrite);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgwrite,i);
		if (sf)
		{
			svc_imageWriter * l = (svc_imageWriter*)sf->getInterface();
			if (l)
			{
				if (wcsstr(l->getExtensions(),ext))
				{
					void* ret = l->convert(data,32,w,h,length);
					sf->releaseInterface(l);
					return ret;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return NULL;
}

static int writeFile(const wchar_t *file, const void * data, int length)
{
	FILE *f=_wfopen(file,L"wb");
	if (!f) return ALBUMART_FAILURE;
	if (fwrite(data,length,1,f) != 1)
	{
		fclose(f);
		return ALBUMART_FAILURE;
	}
	fclose(f);
	return ALBUMART_SUCCESS;
}

static void writeImageToFile(ARGB32 * img, int w, int h, const wchar_t *file)
{
	int length = 0;
	void * data = writeImg(img,w,h,&length,wcsrchr(file,L'.'));
	if (data)
	{
		writeFile(file,data,length);
		WASABI_API_MEMMGR->sysFree(data);
	}
}

int AlbumArt::SetAlbumArt(const wchar_t *filename, const wchar_t *type, int w, int h, const void *bits, size_t len, const wchar_t *mimeType)
{
	if (!bits) 
		return ALBUMART_FAILURE;

	if (!filename || !*filename)
		return ALBUMART_FAILURE;
	if (PathIsURLW(filename))
		return ALBUMART_FAILURE;

	if (!type) type = L"cover";

	bool freebits = false;
	if (!mimeType)
	{
		mimeType = L"jpg"; // TODO: read from ini?
		int l = 0;
		bits = writeImg((ARGB32*)bits,w,h,&l,mimeType);
		if (!bits) return ALBUMART_FAILURE;
		freebits = true;
		len = l;
	}

	wchar_t path[MAX_PATH] = {0};
	wchar_t fn[MAX_PATH] = {0};

	StringCchCopyW(path, MAX_PATH, filename);
	PathRemoveFileSpecW(path);

	wchar_t albumname[MAX_PATH] = {0};
	if (!_wcsicmp(type,L"cover") && AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"album", albumname, MAX_PATH) && albumname[0])
	{
		CleanNameForPath(albumname);
		type = albumname;
	}

	StringCchPrintfW(fn, MAX_PATH, L"%s\\%s.%s", path, type, mimeType);

	int ret = ALBUMART_SUCCESS;
	if (bits)
		ret = writeFile(fn,bits, (int)len);

	if (ret == ALBUMART_SUCCESS)
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::META, MetadataCallback::ART_UPDATED, (intptr_t)fn);

	/*
	else //bits == NULL, so delete!
	_wunlink(fn);
	*/

	if (freebits) WASABI_API_MEMMGR->sysFree((void*)bits);
	return ret;
}

static bool CopySceneNFO(const wchar_t *sourcePath, const wchar_t *destinationPath)
{
	wchar_t nfo_mask[MAX_PATH] = {0};
	PathCombineW(nfo_mask, sourcePath, L"*.nfo");

	WIN32_FIND_DATAW find = {0};
	HANDLE hFind = FindFirstFileW(nfo_mask, &find);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		wchar_t sourceFile[MAX_PATH] = {0};
		wchar_t destinationFile[MAX_PATH] = {0};
		PathCombineW(sourceFile, sourcePath, find.cFileName);
		PathCombineW(destinationFile, destinationPath, find.cFileName);
		CopyFileW(sourceFile, destinationFile, TRUE);
		FindClose(hFind);
		return true;
	}
	return false;
}

static void CopyMask(const wchar_t *sourcePath, const wchar_t *destinationPath, const wchar_t *mask)
{
	wchar_t findMask[MAX_PATH] = {0};
	PathCombineW(findMask, sourcePath, mask);

	WIN32_FIND_DATAW find = {0};
	HANDLE hFind = FindFirstFileW(findMask, &find);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// make sure it's an actual loadable image
			waServiceFactory *factory = 0;
			svc_imageLoader *loader = FindImageLoader(find.cFileName, &factory);

			if (loader)
			{
				wchar_t sourceFile[MAX_PATH] = {0};
				wchar_t destinationFile[MAX_PATH] = {0};
				PathCombineW(sourceFile, sourcePath, find.cFileName);
				PathCombineW(destinationFile, destinationPath, find.cFileName);
				CopyFileW(sourceFile, destinationFile, TRUE);
				factory->releaseInterface(loader);
			}
		}
		while (FindNextFileW(hFind, &find));
		FindClose(hFind);
	}
}

int AlbumArt::CopyAlbumArt(const wchar_t *sourceFilename, const wchar_t *destinationFilename)
{
	if (!sourceFilename || !*sourceFilename || !destinationFilename || !*destinationFilename)
		return ALBUMART_FAILURE;
	if (PathIsURLW(sourceFilename) || PathIsURLW(destinationFilename))
		return ALBUMART_FAILURE;

	// first, copy embedded album art
	waServiceFactory *sourceFactory = 0;
	svc_albumArtProvider *sourceProvider = FindProvider(sourceFilename, ALBUMARTPROVIDER_TYPE_EMBEDDED, &sourceFactory);
	if (sourceProvider)
	{
		waServiceFactory *destinationFactory = 0;
		svc_albumArtProvider *destinationProvider = FindProvider(destinationFilename, ALBUMARTPROVIDER_TYPE_EMBEDDED, &destinationFactory);

		if (destinationProvider)
		{
			// TODO: iterate through all the different types
			void *data = 0;
			size_t datalen = 0;
			wchar_t *mimeType = 0;
			if (sourceProvider->GetAlbumArtData(sourceFilename, L"cover", &data, &datalen, &mimeType) == ALBUMARTPROVIDER_SUCCESS && data && datalen)
			{
				destinationProvider->SetAlbumArtData(destinationFilename, L"cover", data, datalen, mimeType);
				WASABI_API_MEMMGR->sysFree(data);
				WASABI_API_MEMMGR->sysFree(mimeType);
			}
			destinationFactory->releaseInterface(destinationProvider);
		}
		sourceFactory->releaseInterface(sourceProvider);
	}

	// now, if they're in different directories, copy folder.jpg, cover.jpg, front.jpg and %album%.jpg
	wchar_t sourcePath[MAX_PATH] = {0}, destinationPath[MAX_PATH] = {0};
	StringCchCopyW(sourcePath, MAX_PATH, sourceFilename);
	PathRemoveFileSpecW(sourcePath);
	StringCchCopyW(destinationPath, MAX_PATH, destinationFilename);
	PathRemoveFileSpecW(destinationPath);

	if (_wcsicmp(sourcePath, destinationPath) != 0) // if they're different 
	{
		CopyMask(sourcePath, destinationPath, L"cover.*");
		CopyMask(sourcePath, destinationPath, L"folder.*");
		CopyMask(sourcePath, destinationPath, L"front.*");
		CopyMask(sourcePath, destinationPath, L"albumart.*");
		wchar_t mask[MAX_PATH] = {0};
		if (FindSceneNFO(sourcePath, mask))
		{
			CopyMask(sourcePath, destinationPath, mask);
			CopySceneNFO(sourcePath, destinationPath);
		}

		wchar_t albumname[MAX_PATH] = {0};
		if (AGAVE_API_METADATA->GetExtendedFileInfo(sourceFilename, L"album", albumname, MAX_PATH) && albumname[0])
		{
			CleanNameForPath(albumname);
			StringCchPrintfW(mask, MAX_PATH, L"%s.*", albumname);
			CopyMask(sourcePath, destinationPath, mask);
		}
	}
	return 0;
}

#define CBCLASS AlbumArt
START_DISPATCH;
CB(API_ALBUMART_GETALBUMART, GetAlbumArt);
CB(API_ALBUMART_GETALBUMART_NOAMG, GetAlbumArt_NoAMG);
CB(API_ALBUMART_GETALBUMARTDATA, GetAlbumArtData);
CB(API_ALBUMART_GETALBUMARTORIGIN, GetAlbumArtOrigin);
CB(API_ALBUMART_GETALBUMARTTYPES, GetAlbumArtTypes);
CB(API_ALBUMART_GETVALIDALBUMARTTYPES, GetValidAlbumArtTypes);
CB(API_ALBUMART_SETALBUMART, SetAlbumArt);
CB(API_ALBUMART_DELETEALBUMART, DeleteAlbumArt);
CB(API_ALBUMART_COPYALBUMART, CopyAlbumArt);
END_DISPATCH;
#undef CBCLASS