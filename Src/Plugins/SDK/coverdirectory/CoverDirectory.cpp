#include "api.h"
#include "CoverDirectory.h"
#include <api/service/svcs/svc_imgload.h>
#include <api/service/waservicefactory.h>
#include <shlwapi.h>
#include <strsafe.h>

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

/* This is just hardcoded for now - Search the albumart in a cover library dir */
static bool BuildCoverFilename(const wchar_t *filename, const wchar_t *type, wchar_t path[MAX_PATH], wchar_t mask[MAX_PATH])
{
	// TODO: use tagz for this.
	wchar_t artistname[MAX_PATH]=L"", albumname[MAX_PATH]=L"";
	// benski> we're abusing short-circuit evaluation in a big way here :)
  if (((AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"albumartist", artistname, MAX_PATH) && artistname[0]) 
		 || (AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"artist", artistname, MAX_PATH) && artistname[0]))
		 && AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"album", albumname, MAX_PATH) && albumname[0])
	{
		CleanNameForPath(artistname);
		CleanNameForPath(albumname);
		StringCchPrintf(mask, MAX_PATH, L"%s - %s.*",artistname, albumname);
		PathCombine(path, WASABI_API_APP->path_getUserSettingsPath(), L"Cover Library");
		return true;
	}
	return false;
}

static svc_imageLoader *FindImageLoader(const wchar_t *filespec, waServiceFactory **factory)
{
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = WASABI_API_SVC->service_getNumServices(imgload);
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

static bool ImageExists(const wchar_t *path, const wchar_t *mask)
{
	wchar_t dirmask[MAX_PATH];
	PathCombineW(dirmask, path, mask);
	WIN32_FIND_DATAW find;
	HANDLE hFind = FindFirstFileW(dirmask, &find);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t fn[MAX_PATH];
			PathCombine(fn, path, mask);
			waServiceFactory *factory=0;
			svc_imageLoader *loader = FindImageLoader(fn, &factory);
			if (loader)
			{
				factory->releaseInterface(loader);
				FindClose(hFind);
				return true;
			}
			
		}
		while (FindNextFileW(hFind, &find));
		FindClose(hFind);
	}
	return false;
}

static bool LoadFile(const wchar_t *filename, void **bits, size_t *len)
{
	*len=0;
	FILE * f = _wfopen(filename,L"rb");
	if (!f)
		return false;
	fseek(f,0,2);
	*len = ftell(f);
	if (!*len)
	{
		fclose(f);
		return false;
	}
	fseek(f,0,0);
	void * data = WASABI_API_MEMMGR->sysMalloc(*len);
	fread(data,*len,1,f);
	fclose(f);
	*bits = data;
	return true;
}

static bool LoadImageData(const wchar_t *path, const wchar_t *mask, void **bits, size_t *len, wchar_t **mimeType)
{
	wchar_t dirmask[MAX_PATH];
	PathCombineW(dirmask, path, mask);
	WIN32_FIND_DATAW find;
	HANDLE hFind = FindFirstFileW(dirmask, &find);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t fn[MAX_PATH];
			PathCombine(fn, path, find.cFileName);
			waServiceFactory *factory=0;
			svc_imageLoader *loader = FindImageLoader(fn, &factory);
			if (loader)
			{
				factory->releaseInterface(loader);
				if (LoadFile(fn, bits, len))
				{
					const wchar_t *ext = PathFindExtension(fn);
					if (*ext)
					{
						ext++;
						size_t len = wcslen(ext);
						*mimeType = (wchar_t *)WASABI_API_MEMMGR->sysMalloc((len + 1) * sizeof(wchar_t));
						StringCchCopy(*mimeType, len+1, ext);
						CharLower(*mimeType);
					}
					else
					{
						*mimeType=0;
					}

					FindClose(hFind);
					return true;
				}
			}
			
		}
		while (FindNextFileW(hFind, &find));
		FindClose(hFind);
	}
	return false;
}

static bool DeleteImage(const wchar_t *path, const wchar_t *mask)
{
	wchar_t dirmask[MAX_PATH];
	PathCombineW(dirmask, path, mask);
	WIN32_FIND_DATAW find;
	HANDLE hFind = FindFirstFileW(dirmask, &find);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t fn[MAX_PATH];
			PathCombine(fn, path, mask);
			waServiceFactory *factory=0;
			svc_imageLoader *loader = FindImageLoader(fn, &factory);
			if (loader)
			{
				DeleteFile(fn);
				factory->releaseInterface(loader);
			}			
		}
		while (FindNextFileW(hFind, &find));
		FindClose(hFind);
	}
	return false;
}

bool CoverDirectory::IsMine(const wchar_t *filename)
{
	//wchar_t path[MAX_PATH], mask[MAX_PATH];
	//if (BuildCoverFilename(filename, path, mask) && ImageExists(path, mask))
		return true;
	//else
//		return false;
}

int CoverDirectory::ProviderType()
{
	return ALBUMARTPROVIDER_TYPE_DATABASE;
}

// implementation note: use WASABI_API_MEMMGR to alloc bits and mimetype, so that the recipient can free through that
int CoverDirectory::GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType)
{
	wchar_t path[MAX_PATH], mask[MAX_PATH];
	if (BuildCoverFilename(filename, type, path, mask))
	{
		if (LoadImageData(path, mask, bits, len, mimeType))
		{
			return ALBUMARTPROVIDER_SUCCESS;
		}
	}
	return ALBUMARTPROVIDER_FAILURE;
}

int CoverDirectory::SetAlbumArtData(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mimeType)
{
  // TODO:
	return ALBUMARTPROVIDER_FAILURE;
}

int CoverDirectory::DeleteAlbumArt(const wchar_t *filename, const wchar_t *type)
{
	wchar_t path[MAX_PATH], mask[MAX_PATH];
	if (BuildCoverFilename(filename, type, path, mask))
	{
		DeleteImage(path, mask);
		return ALBUMARTPROVIDER_SUCCESS;	
	}
	return ALBUMARTPROVIDER_FAILURE;
}

#define CBCLASS CoverDirectory
START_DISPATCH;
CB(SVC_ALBUMARTPROVIDER_PROVIDERTYPE, ProviderType);
CB(SVC_ALBUMARTPROVIDER_GETALBUMARTDATA, GetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_ISMINE, IsMine);
CB(SVC_ALBUMARTPROVIDER_SETALBUMARTDATA, SetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_DELETEALBUMART, DeleteAlbumArt);
END_DISPATCH;
#undef CBCLASS

static CoverDirectory albumArtProvider;

// {725084AC-DBAD-4f7b-98FA-478AE20B9517}
static const GUID coverDirectoryGUID = 
{ 0x725084ac, 0xdbad, 0x4f7b, { 0x98, 0xfa, 0x47, 0x8a, 0xe2, 0xb, 0x95, 0x17 } };


FOURCC AlbumArtFactory::GetServiceType()
{
	return svc_albumArtProvider::SERVICETYPE;
}

const char *AlbumArtFactory::GetServiceName()
{
	return "Cover Directory";
}

GUID AlbumArtFactory::GetGUID()
{
	return coverDirectoryGUID;
}

void *AlbumArtFactory::GetInterface(int global_lock)
{
	return &albumArtProvider;
}

int AlbumArtFactory::SupportNonLockingInterface()
{
	return 1;
}

int AlbumArtFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	return 1;
}

const char *AlbumArtFactory::GetTestString()
{
	return 0;
}

int AlbumArtFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS AlbumArtFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName)
CB(WASERVICEFACTORY_GETGUID, GetGUID)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface)
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
CB(WASERVICEFACTORY_GETTESTSTRING, GetTestString)
CB(WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify)
END_DISPATCH;
#undef CBCLASS
