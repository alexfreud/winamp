#include "main.h"
#include "Metadata.h"
#include "api__in_mp3.h"
#include "../nu/AutoWide.h"
#include "AlbumArt.h"
#include "Stopper.h"
#include <shlwapi.h>
#include <strsafe.h>

bool IsMyExtension(const wchar_t *filename)
{
	// check if it's the current stream and is playing and is SHOUTcast2
	if (PathIsURLW(filename))
	{
		if (g_playing_file)
		{
			EnterCriticalSection(&streamInfoLock);
			if (g_playing_file &&
				(g_playing_file->uvox_artwork.uvox_stream_artwork || g_playing_file->uvox_artwork.uvox_playing_artwork) &&
				!lstrcmpW(lastfn, filename)) // check again now that we've acquired the lock
			{
				LeaveCriticalSection(&streamInfoLock);
				return true;
			}
			LeaveCriticalSection(&streamInfoLock);
		}
	}
	// otherwise handle as normal embedded 
	else
	{
		const wchar_t *extension = PathFindExtension(filename);
		if (extension && *extension)
		{
			AutoWide wideList(config_extlist); // TODO: build a copy of this at config load time so we don't have to run this every time
			extension++;
			wchar_t *b = wideList;

			wchar_t *c = 0;
			do
			{
				wchar_t d[20] = {0};
				StringCchCopyW(d, 15, b);
				if ((c = wcschr(b, L';')))
				{
					if ((c-b)<15)
						d[c - b] = 0;
				}

				if (!lstrcmpiW(extension, d))
					return true;

				b = c + 1;
			}
			while (c);
		}
	}
	return false;
}

bool ID3v2_AlbumArtProvider::IsMine(const wchar_t *filename)
{
	return IsMyExtension(filename);
}

int ID3v2_AlbumArtProvider::ProviderType()
{
	return ALBUMARTPROVIDER_TYPE_EMBEDDED;
}

int ID3v2_AlbumArtProvider::GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType)
{
	if (g_playing_file)
	{
		EnterCriticalSection(&streamInfoLock);
		if (g_playing_file && !lstrcmpW(lastfn, filename)) // check again now that we've acquired the lock
		{
			wchar_t* mimeType[] = {
					L"image/jpeg",
					L"image/png",
					L"image/bmp",
					L"image/gif"
			};
			if (!g_playing_file->uvox_artwork.uvox_stream_artwork)
			{
				int ret = g_playing_file->info.GetAlbumArt(type, bits, len, mimeType);
				LeaveCriticalSection(&streamInfoLock);
				return ret;
			}
			else
			{
				// will handle "playing" and "cover" - cover is the stream branding
				// with "playing" used to provide song specific stream artwork
				if (!_wcsicmp(type, L"playing"))
				{
					if (g_playing_file->uvox_artwork.uvox_playing_artwork_len > 0)
					{
						*len = g_playing_file->uvox_artwork.uvox_playing_artwork_len;
						int type = g_playing_file->uvox_artwork.uvox_playing_artwork_type;
						if(type >= 0 && type <= 3)
						{
							*mimeType = (wchar_t *)WASABI_API_MEMMGR->sysMalloc(12 * sizeof(wchar_t));
							wcsncpy(*mimeType, mimeType[type], 12);
						}
						else
						{
							*mimeType = 0;
						}

						*bits = WASABI_API_MEMMGR->sysMalloc(*len);
						memcpy(*bits, g_playing_file->uvox_artwork.uvox_playing_artwork, *len);

						LeaveCriticalSection(&streamInfoLock);
						return ALBUMARTPROVIDER_SUCCESS;
					}
					else
					{
						LeaveCriticalSection(&streamInfoLock);
						return ALBUMARTPROVIDER_FAILURE;
					}
				}
				else
				{
					if (g_playing_file->uvox_artwork.uvox_stream_artwork_len > 0)
					{
						*len = g_playing_file->uvox_artwork.uvox_stream_artwork_len;
						
						int type = g_playing_file->uvox_artwork.uvox_stream_artwork_type;
						if(type >= 0 && type <= 3)
						{
							*mimeType = (wchar_t *)WASABI_API_MEMMGR->sysMalloc(12 * sizeof(wchar_t));
							wcsncpy(*mimeType, mimeType[type], 12);
						}
						else
						{
							*mimeType = 0;
						}

						*bits = WASABI_API_MEMMGR->sysMalloc(*len);
						memcpy(*bits, g_playing_file->uvox_artwork.uvox_stream_artwork, *len);

						LeaveCriticalSection(&streamInfoLock);
						return ALBUMARTPROVIDER_SUCCESS;
					}
					else
					{
						LeaveCriticalSection(&streamInfoLock);
						return ALBUMARTPROVIDER_FAILURE;
					}
				}
			}
		}
		LeaveCriticalSection(&streamInfoLock);
	}

	Metadata metadata;
	if (metadata.Open(filename) == METADATA_SUCCESS)
	{
		return metadata.id3v2.GetAlbumArt(type, bits, len, mimeType);
	}

	return ALBUMARTPROVIDER_FAILURE;
}

extern Metadata *m_ext_get_mp3info;

int ID3v2_AlbumArtProvider::SetAlbumArtData(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mimeType)
{
	Metadata metadata;
	if (metadata.Open(filename) == METADATA_SUCCESS)
	{
		int ret = metadata.id3v2.SetAlbumArt(type, bits, len, mimeType);
		if (ret == METADATA_SUCCESS)
		{
			// flush our read cache too :)
			if (m_ext_get_mp3info) m_ext_get_mp3info->Release();
			m_ext_get_mp3info = NULL;

			Stopper stopper;
			if (metadata.IsMe(lastfn))
				stopper.Stop();
			metadata.Save();
			stopper.Play();
		}
		return ret;
	}
	return ALBUMARTPROVIDER_FAILURE;
}

int ID3v2_AlbumArtProvider::DeleteAlbumArt(const wchar_t *filename, const wchar_t *type)
{
	Metadata metadata;
	if (metadata.Open(filename) == METADATA_SUCCESS)
	{
		int ret = metadata.id3v2.DeleteAlbumArt(type);
		if (ret == METADATA_SUCCESS)
		{
			// flush our read cache too :)
			if (m_ext_get_mp3info) m_ext_get_mp3info->Release();
			m_ext_get_mp3info = NULL;

			Stopper stopper;
			if (metadata.IsMe(lastfn))
				stopper.Stop();
			metadata.Save();
			stopper.Play();
		}
		return ret;
	}
	return ALBUMARTPROVIDER_FAILURE;
}

#define CBCLASS ID3v2_AlbumArtProvider
START_DISPATCH;
CB(SVC_ALBUMARTPROVIDER_PROVIDERTYPE, ProviderType);
CB(SVC_ALBUMARTPROVIDER_GETALBUMARTDATA, GetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_ISMINE, IsMine);
CB(SVC_ALBUMARTPROVIDER_SETALBUMARTDATA, SetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_DELETEALBUMART, DeleteAlbumArt);
END_DISPATCH;
#undef CBCLASS

static ID3v2_AlbumArtProvider albumArtProvider;

// {C8222317-8F0D-4e79-9222-447381C46E07}
static const GUID id3v2_albumartproviderGUID =
  { 0xc8222317, 0x8f0d, 0x4e79, { 0x92, 0x22, 0x44, 0x73, 0x81, 0xc4, 0x6e, 0x7 } };

FOURCC AlbumArtFactory::GetServiceType()
{
	return svc_albumArtProvider::SERVICETYPE;
}

const char *AlbumArtFactory::GetServiceName()
{
	return "ID3v2 Album Art Provider";
}

GUID AlbumArtFactory::GetGUID()
{
	return id3v2_albumartproviderGUID;
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