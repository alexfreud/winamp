#include "Winamp/in2.h"
#include "Winamp/wa_ipc.h"
#include "Wasabi.h"

AlbumArtFactory albumArtFactory;

class WavPack_AlbumArtProvider : public svc_albumArtProvider
{
public:
	bool IsMine(const wchar_t *filename);
	int ProviderType();
	// implementation note: use WASABI_API_MEMMGR to alloc bits and mimetype, so that the recipient can free through that
	int GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType);
	int SetAlbumArtData(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mimeType);
	int DeleteAlbumArt(const wchar_t *filename, const wchar_t *type);
protected:
	RECVS_DISPATCH;
};

static const wchar_t *GetLastCharactercW(const wchar_t *string)
{
	if (!string || !*string)
		return string;

	return CharPrevW(string, string+lstrlenW(string));
}

static const wchar_t *scanstr_backW(const wchar_t *str, const wchar_t *toscan, const wchar_t *defval)
{
	const wchar_t *s = GetLastCharactercW(str);
	if (!str[0]) return defval;
	if (!toscan || !toscan[0]) return defval; 
	while (1)
	{
		const wchar_t *t = toscan;
		while (*t)
		{
			if (*t == *s) return s;
			t = CharNextW(t);
		}
		t = CharPrevW(str, s);
		if (t == s)
			return defval;
		s = t;
	}
}

static const wchar_t *extensionW(const wchar_t *fn)
{
	const wchar_t *end = scanstr_backW(fn, L"./\\", 0);
	if (!end)
		return (fn+lstrlenW(fn));

	if (*end == L'.')
		return end+1;

	return (fn+lstrlenW(fn));
}

int WavPack_HandlesExtension(const wchar_t *extension);
bool WavPack_AlbumArtProvider::IsMine(const wchar_t *filename)
{
	const wchar_t *extension = extensionW(filename);
	if (extension && *extension)
	{
		return WavPack_HandlesExtension(extension) != 0;
	}
	return false;
}

int WavPack_AlbumArtProvider::ProviderType()
{
	return ALBUMARTPROVIDER_TYPE_EMBEDDED;
}

int WavPack_GetAlbumArt(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mime_type);
int WavPack_AlbumArtProvider::GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mime_type)
{
	return WavPack_GetAlbumArt(filename, type, bits, len, mime_type);
}

int WavPack_SetAlbumArt(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mime_type);
int WavPack_AlbumArtProvider::SetAlbumArtData(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mime_type)
{
	return WavPack_SetAlbumArt(filename, type, bits, len, mime_type);
}

int WavPack_DeleteAlbumArt(const wchar_t *filename, const wchar_t *type);
int WavPack_AlbumArtProvider::DeleteAlbumArt(const wchar_t *filename, const wchar_t *type)
{
	return WavPack_DeleteAlbumArt(filename, type);
}

#define CBCLASS WavPack_AlbumArtProvider
START_DISPATCH;
CB(SVC_ALBUMARTPROVIDER_PROVIDERTYPE, ProviderType);
CB(SVC_ALBUMARTPROVIDER_GETALBUMARTDATA, GetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_ISMINE, IsMine);
CB(SVC_ALBUMARTPROVIDER_SETALBUMARTDATA, SetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_DELETEALBUMART, DeleteAlbumArt);
END_DISPATCH;
#undef CBCLASS

static WavPack_AlbumArtProvider albumArtProvider;

// {A558560E-4334-446a-9219-E1F34F518ADE}
static const GUID wavpack_albumartproviderGUID = 
{ 0xa558560e, 0x4334, 0x446a, { 0x92, 0x19, 0xe1, 0xf3, 0x4f, 0x51, 0x8a, 0xde } };

FOURCC AlbumArtFactory::GetServiceType()
{
	return svc_albumArtProvider::SERVICETYPE;
}

const char *AlbumArtFactory::GetServiceName()
{
	return "WavPack Album Art Provider";
}

GUID AlbumArtFactory::GetGUID()
{
	return wavpack_albumartproviderGUID;
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