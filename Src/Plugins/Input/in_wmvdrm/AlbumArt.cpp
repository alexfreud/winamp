#include "main.h"
#include "../nu/AutoWide.h"
#include "AlbumArt.h"
#include "util.h"
#include <shlwapi.h>
#include <strsafe.h>

bool ASF_AlbumArtProvider::IsMine(const wchar_t *filename)
{
	const wchar_t *ext = PathFindExtension(filename);
	if (ext && *ext)
	{
		ext++;
		return fileTypes.GetAVType(ext) != -1;
	}
	return false;
}

int ASF_AlbumArtProvider::ProviderType()
{
	return ALBUMARTPROVIDER_TYPE_EMBEDDED;
}

bool NameToAPICType(const wchar_t *name, int &num)
{
	if (!name || !*name) // default to cover
		num=0x3;
	else if (!_wcsicmp(name, L"fileicon")) // 	32x32 pixels 'file icon' (PNG only)
		num=0x1;
	else if (!_wcsicmp(name, L"icon")) // 	Other file icon
		num=0x2;
	else if (!_wcsicmp(name, L"cover")) // Cover (front)
		num=0x3;
	else if (!_wcsicmp(name, L"back")) // Cover (back)
		num=0x4;
	else if (!_wcsicmp(name, L"leaflet")) // Leaflet page
		num=0x5;
	else if (!_wcsicmp(name, L"media")) // Media (e.g. lable side of CD)
		num=0x6;
	else if (!_wcsicmp(name, L"leadartist")) //Lead artist/lead performer/soloist
		num=0x7;
	else if (!_wcsicmp(name, L"artist")) // Artist/performer
		num=0x8;
	else if (!_wcsicmp(name, L"conductor")) // Conductor
		num=0x9;
	else if (!_wcsicmp(name, L"band")) // Band/Orchestra
		num=0xA;
	else if (!_wcsicmp(name, L"composer"))  // Composer
		num=0xB;
	else if (!_wcsicmp(name, L"lyricist")) // Lyricist/text writer
		num=0xC;
	else if (!_wcsicmp(name, L"location")) // Recording Location
		num=0xD;
	else if (!_wcsicmp(name, L"recording")) // During recording
		num=0xE;
	else if (!_wcsicmp(name, L"performance")) // During performance
		num=0xF;
	else if (!_wcsicmp(name, L"preview")) // Movie/video screen capture
		num=0x10;
	else if (!_wcsicmp(name, L"fish")) // A bright coloured fish
		num=0x11;
	else if (!_wcsicmp(name, L"illustration")) // Illustration
		num=0x12;
	else if (!_wcsicmp(name, L"artistlogo")) // Band/artist logotype
		num=0x13;
	else if (!_wcsicmp(name, L"publisherlogo")) // Publisher/Studio logotype
		num=0x14;
	else
		return false;
	return true;
}


int ASF_AlbumArtProvider::GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType)
{
	int pictype;
	if (NameToAPICType(type, pictype))
	{
		WMInformation wm(filename);
		if (wm.GetPicture(bits, len, mimeType, pictype))
			return ALBUMARTPROVIDER_SUCCESS;
	}

	return ALBUMARTPROVIDER_FAILURE;
}

int ASF_AlbumArtProvider::SetAlbumArtData(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mimeType)
{
	int pictype;
	if (NameToAPICType(type, pictype))
	{
		WMInformation wm(filename);
		if (!wm.MakeWritable(filename))
			return ALBUMARTPROVIDER_READONLY; // can't write

		if (wm.SetPicture(bits, len, mimeType, pictype))
		{
			wm.Flush();
			return ALBUMARTPROVIDER_SUCCESS;
		}
	}

	return ALBUMARTPROVIDER_FAILURE;
}

int ASF_AlbumArtProvider::DeleteAlbumArt(const wchar_t *filename, const wchar_t *type)
{
	int pictype;
	if (NameToAPICType(type, pictype))
	{
		WMInformation wm(filename);
		if (!wm.MakeWritable(filename))
		{
			if (wm.HasPicture(pictype))
				return ALBUMARTPROVIDER_READONLY; // can't write
			else
				return ALBUMARTPROVIDER_FAILURE;
		}

		if (wm.DeletePicture(pictype))
		{
			wm.Flush();
			return ALBUMARTPROVIDER_SUCCESS;
		}
	}

	return ALBUMARTPROVIDER_FAILURE;
}

#define CBCLASS ASF_AlbumArtProvider
START_DISPATCH;
CB(SVC_ALBUMARTPROVIDER_PROVIDERTYPE, ProviderType);
CB(SVC_ALBUMARTPROVIDER_GETALBUMARTDATA, GetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_SETALBUMARTDATA, SetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_DELETEALBUMART, DeleteAlbumArt);
CB(SVC_ALBUMARTPROVIDER_ISMINE, IsMine);
END_DISPATCH;
#undef CBCLASS

static ASF_AlbumArtProvider albumArtProvider;

// {B4184902-EE79-4015-B9A4-76209C6153FA}
static const GUID asf_albumartproviderGUID = 
{ 0xb4184902, 0xee79, 0x4015, { 0xb9, 0xa4, 0x76, 0x20, 0x9c, 0x61, 0x53, 0xfa } };


FOURCC AlbumArtFactory::GetServiceType()
{
	return svc_albumArtProvider::SERVICETYPE;
}

const char *AlbumArtFactory::GetServiceName()
{
	return "ASF Album Art Provider";
}

GUID AlbumArtFactory::GetGUID()
{
	return asf_albumartproviderGUID;
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
