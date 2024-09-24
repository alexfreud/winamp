/*
** Copyright © 2007-2014 Winamp SA
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
** Author: Ben Allison benski@winamp.com
** Created: July 25, 2007
**
*/

#include "main.h"
#include "api__in_flv.h"
#include "Metadata.h"
#include "../nu/AutoWide.h"
#include "AlbumArt.h"
#include <shlwapi.h>
#include <strsafe.h>

bool FLAC_AlbumArtProvider::IsMine(const wchar_t *filename)
{
	const wchar_t *extension = PathFindExtensionW(filename);
	if (extension && *extension)
	{
		wchar_t exts[128] = {0};
		GetPrivateProfileStringW(L"in_flac", L"extensions", DEFAULT_EXTENSIONSW, exts, 128, winampINI);

		extension++;
		wchar_t *b = exts;
		wchar_t *c;
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
	return false;
}

int FLAC_AlbumArtProvider::ProviderType()
{
	return ALBUMARTPROVIDER_TYPE_EMBEDDED;
}

bool NameToAPICType(const wchar_t *name, FLAC__StreamMetadata_Picture_Type &num)
{
	if (!name || !*name) // default to cover
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER;
	else if (!_wcsicmp(name, L"fileicon")) // 	32x32 pixels 'file icon' (PNG only)
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_FILE_ICON_STANDARD;
	else if (!_wcsicmp(name, L"icon")) // 	Other file icon
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_FILE_ICON;
	else if (!_wcsicmp(name, L"cover")) // Cover (front)
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER;
	else if (!_wcsicmp(name, L"back")) // Cover (back)
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_BACK_COVER;
	else if (!_wcsicmp(name, L"leaflet")) // Leaflet page
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_LEAFLET_PAGE;
	else if (!_wcsicmp(name, L"media")) // Media (e.g. lable side of CD)
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_MEDIA;
	else if (!_wcsicmp(name, L"leadartist")) //Lead artist/lead performer/soloist
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_LEAD_ARTIST;
	else if (!_wcsicmp(name, L"artist")) // Artist/performer
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_ARTIST;
	else if (!_wcsicmp(name, L"conductor")) // Conductor
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_CONDUCTOR;
	else if (!_wcsicmp(name, L"band")) // Band/Orchestra
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_BAND;
	else if (!_wcsicmp(name, L"composer"))  // Composer
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_COMPOSER;
	else if (!_wcsicmp(name, L"lyricist")) // Lyricist/text writer
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_LYRICIST;
	else if (!_wcsicmp(name, L"location")) // Recording Location
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_RECORDING_LOCATION;
	else if (!_wcsicmp(name, L"recording")) // During recording
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_DURING_RECORDING;
	else if (!_wcsicmp(name, L"performance")) // During performance
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_DURING_PERFORMANCE;
	else if (!_wcsicmp(name, L"preview")) // Movie/video screen capture
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_VIDEO_SCREEN_CAPTURE;
	else if (!_wcsicmp(name, L"fish")) // A bright coloured fish
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_FISH;
	else if (!_wcsicmp(name, L"illustration")) // Illustration
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_ILLUSTRATION;
	else if (!_wcsicmp(name, L"artistlogo")) // Band/artist logotype
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_BAND_LOGOTYPE;
	else if (!_wcsicmp(name, L"publisherlogo")) // Publisher/Studio logotype
		num=FLAC__STREAM_METADATA_PICTURE_TYPE_PUBLISHER_LOGOTYPE;
	else
		return false;
	return true;
}

int FLAC_AlbumArtProvider::GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType)
{
	FLACMetadata metadata;

	if (metadata.Open(filename))
	{
		FLAC__StreamMetadata_Picture_Type pictype;
		if (NameToAPICType(type, pictype))
		{
			if (metadata.GetPicture(pictype, bits, len, mimeType))
				return ALBUMARTPROVIDER_SUCCESS;
		}
	}

	return ALBUMARTPROVIDER_FAILURE;
}

int FLAC_AlbumArtProvider::SetAlbumArtData(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mimeType)
{
	if (!info || info && _wcsicmp(filename, info->filename))
	{
		FLACMetadata metadata;
		if (metadata.Open(filename))
		{
			FLAC__StreamMetadata_Picture_Type pictype;
			if (NameToAPICType(type, pictype))
			{
				if (metadata.SetPicture(pictype, bits, len, mimeType, 0/*TODO*/, 0/*TODO*/))
				{
					if (metadata.Save(filename))
						return ALBUMARTPROVIDER_SUCCESS;
					else 
						return ALBUMARTPROVIDER_FAILURE;
				}
			}
		}
	}
	else
	{
		FLAC__StreamMetadata_Picture_Type pictype;
		if (NameToAPICType(type, pictype))
		{
			if (info->metadata.SetPicture(pictype, bits, len, mimeType, 0/*TODO*/, 0/*TODO*/))
			{
				return ALBUMARTPROVIDER_SUCCESS;
			}
		}
	}

	return ALBUMARTPROVIDER_FAILURE;
}

int FLAC_AlbumArtProvider::DeleteAlbumArt(const wchar_t *filename, const wchar_t *type)
{
	if (!info || info && _wcsicmp(filename, info->filename))
	{
		FLACMetadata metadata;
		if (metadata.Open(filename))
		{
			FLAC__StreamMetadata_Picture_Type pictype;
			if (NameToAPICType(type, pictype))
			{
				if (info->metadata.RemovePicture(pictype))
				{
					if (info->metadata.Save(filename))
						return ALBUMARTPROVIDER_SUCCESS;
					else
						return ALBUMARTPROVIDER_FAILURE;
				}
			}
		}
	}
	else
	{
		FLAC__StreamMetadata_Picture_Type pictype;
		if (NameToAPICType(type, pictype))
		{
			if (info->metadata.RemovePicture(pictype))
			{
				return ALBUMARTPROVIDER_SUCCESS;
			}
		}
	}

	return ALBUMARTPROVIDER_FAILURE;
}

#define CBCLASS FLAC_AlbumArtProvider
START_DISPATCH;
CB(SVC_ALBUMARTPROVIDER_PROVIDERTYPE, ProviderType);
CB(SVC_ALBUMARTPROVIDER_GETALBUMARTDATA, GetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_ISMINE, IsMine);
CB(SVC_ALBUMARTPROVIDER_SETALBUMARTDATA, SetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_DELETEALBUMART, DeleteAlbumArt);
END_DISPATCH;
#undef CBCLASS

static FLAC_AlbumArtProvider albumArtProvider;

// {622C3B42-866E-4935-AA52-3B456AE8B036}
static const GUID flac_albumartproviderGUID =
  { 0x622c3b42, 0x866e, 0x4935, { 0xaa, 0x52, 0x3b, 0x45, 0x6a, 0xe8, 0xb0, 0x36 } };

FOURCC AlbumArtFactory::GetServiceType()
{
	return svc_albumArtProvider::SERVICETYPE;
}

const char *AlbumArtFactory::GetServiceName()
{
	return "FLAC Album Art Provider";
}

GUID AlbumArtFactory::GetGUID()
{
	return flac_albumartproviderGUID;
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