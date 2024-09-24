#include "mp4.h"
#include "AlbumArt.h"
#include "api__in_mp4.h"
#include "main.h"
#include "../nu/AutoWide.h"
#include "VirtualIO.h"
#include "Stopper.h"
#include <shlwapi.h>
#include <strsafe.h>

bool IsMyExtension(const wchar_t *filename)
{
	const wchar_t *extension = PathFindExtension(filename);
	if (extension && *extension)
	{
		wchar_t exts[1024] = L"";
		// TODO: build a copy of this at config load time so we don't have to run this every time
		GetPrivateProfileStringW(L"in_mp4", L"extensionlist", defaultExtensions, exts, 1024, m_ini);

		extension++;
		wchar_t *b = exts;
		wchar_t *c = 0;
		do
		{
			wchar_t d[20] = {0};
			StringCchCopyW(d, 15, b);
			c = wcschr(b, L';');
			if (c)
			{
				if ((c-b)<15)
					d[c - b] = 0;
			}

			if (!_wcsicmp(extension, d))
				return true;

			b = c + 1;
		}
		while (c);
	}
	return false;
}

bool MP4_AlbumArtProvider::IsMine(const wchar_t *filename)
{
	return IsMyExtension(filename);
}

int MP4_AlbumArtProvider::ProviderType()
{
	return ALBUMARTPROVIDER_TYPE_EMBEDDED;
}

static int MimeTypeToFlags(const wchar_t *mime_type)
{
	if (!mime_type)
		return 0;

	if (!_wcsicmp(mime_type, L"jpeg")
		|| !_wcsicmp(mime_type, L"jpg")
		|| !_wcsicmp(mime_type, L"image/jpeg")
		|| !_wcsicmp(mime_type, L"image/jpg"))
		return 13; /* JPEG */

	if (!_wcsicmp(mime_type, L"png")
		|| !_wcsicmp(mime_type, L"image/png"))
		return 14; /* PNG */

	if (!_wcsicmp(mime_type, L"gif")
		|| !_wcsicmp(mime_type, L"image/gif"))
		return 12; /* GIF */

	if (!_wcsicmp(mime_type, L"bmp")
		|| !_wcsicmp(mime_type, L"image/bmp"))
		return 27; /* BMP */

	return 0; /* default to binary, I guess */
}

int MP4_AlbumArtProvider::GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType)
{
	void *reader = CreateUnicodeReader(filename);
	if (!reader)
		return ALBUMARTPROVIDER_FAILURE;

	MP4FileHandle mp4 = MP4ReadEx(filename, reader, &UnicodeIO);
	if (!mp4)
	{
		DestroyUnicodeReader(reader);
		return ALBUMARTPROVIDER_FAILURE;
	}
	else
	{
		UnicodeClose(reader); // go ahead and close the file so we don't lock it
	}

	u_int8_t *art = 0;
	u_int32_t artSize = 0;
	int flags = 0;
	if (MP4GetMetadataCoverArt(mp4, &art, &artSize, &flags))
	{
		*bits = WASABI_API_MEMMGR->sysMalloc(artSize);
		memcpy(*bits, art, artSize);
		*len=artSize;
		/* TODO: use flags */
		*mimeType = 0; // no idea what the mime type is :(
		MP4Free(art);
		MP4Close(mp4);
		DestroyUnicodeReader(reader);
		return ALBUMARTPROVIDER_SUCCESS;
	}

	MP4Close(mp4);
	DestroyUnicodeReader(reader);
	return ALBUMARTPROVIDER_FAILURE;
}

int MP4_AlbumArtProvider::SetAlbumArtData(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mimeType)
{
	MP4FileHandle mp4 = MP4Modify(filename, 0, 0); 
	if (!mp4)
	{
		return ALBUMARTPROVIDER_FAILURE;
	}

	int flags = MimeTypeToFlags(mimeType);
	if (MP4SetMetadataCoverArt(mp4, (u_int8_t *)bits, len, flags))
	{
		MP4Close(mp4);
		return ALBUMARTPROVIDER_SUCCESS;
	}

	MP4Close(mp4);
	return ALBUMARTPROVIDER_FAILURE;
}

int MP4_AlbumArtProvider::DeleteAlbumArt(const wchar_t *filename, const wchar_t *type)
{
	MP4FileHandle mp4 = MP4Modify(filename, 0, 0);

	if (!mp4)
	{
		return ALBUMARTPROVIDER_FAILURE;
	}

	if (MP4DeleteMetadataCoverArt(mp4))
	{
		Stopper stopper;
		if (!_wcsicmp(filename, lastfn))
			stopper.Stop();
		MP4Close(mp4);
		stopper.Play();
		return ALBUMARTPROVIDER_SUCCESS;
	}

	MP4Close(mp4);
	return ALBUMARTPROVIDER_FAILURE;
}


#define CBCLASS MP4_AlbumArtProvider
START_DISPATCH;
CB(SVC_ALBUMARTPROVIDER_PROVIDERTYPE, ProviderType);
CB(SVC_ALBUMARTPROVIDER_GETALBUMARTDATA, GetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_SETALBUMARTDATA, SetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_DELETEALBUMART, DeleteAlbumArt);
CB(SVC_ALBUMARTPROVIDER_ISMINE, IsMine);
END_DISPATCH;
#undef CBCLASS

static MP4_AlbumArtProvider albumArtProvider;

// {315CA473-4A7B-43a9-BB1B-7E1C24B3BFE2}
static const GUID mp4_albumartproviderGUID =
  { 0x315ca473, 0x4a7b, 0x43a9, { 0xbb, 0x1b, 0x7e, 0x1c, 0x24, 0xb3, 0xbf, 0xe2 } };

FOURCC AlbumArtFactory::GetServiceType()
{
	return svc_albumArtProvider::SERVICETYPE;
}

const char *AlbumArtFactory::GetServiceName()
{
	return "MP4 Album Art Provider";
}

GUID AlbumArtFactory::GetGUID()
{
	return mp4_albumartproviderGUID;
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
	//plugin.service->service_unlock(ifc);
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

#ifdef CBCLASS
#undef CBCLASS
#endif

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