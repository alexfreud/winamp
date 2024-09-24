#ifndef NULLSOFT_AGAVE_SVC_ALBUMARTPROVIDER_H
#define NULLSOFT_AGAVE_SVC_ALBUMARTPROVIDER_H

#include "../../bfc/dispatch.h"
#include "../../bfc/std_mkncc.h" // for MKnCC()

enum
{
	ALBUMARTPROVIDER_SUCCESS = 0,
	ALBUMARTPROVIDER_FAILURE = 1,
	ALBUMARTPROVIDER_READONLY = 2,

	ALBUMARTPROVIDER_TYPE_EMBEDDED = 0, // contained within another file (e.g. inside id3v2 tag)
	ALBUMARTPROVIDER_TYPE_DATABASE = 1, // cached in a database somewhere (e.g. ipod artwork DB)
	ALBUMARTPROVIDER_TYPE_FOLDER = 2, // sitting on a folder somewhere (e.g. folder.jpg)
};
class svc_albumArtProvider : public Dispatchable
{
protected:
	svc_albumArtProvider() {}
	~svc_albumArtProvider() {}
public:
	
  static FOURCC getServiceType() { return svc_albumArtProvider::SERVICETYPE; }
	bool IsMine(const wchar_t *filename);
	int ProviderType();
	// implementation note: use WASABI_API_MEMMGR to alloc bits and mimetype, so that the recipient can free through that
	int GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType);
	int SetAlbumArtData(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mimeType);
	int DeleteAlbumArt(const wchar_t *filename, const wchar_t *type);

	DISPATCH_CODES
	{
		SVC_ALBUMARTPROVIDER_PROVIDERTYPE = 0,
		SVC_ALBUMARTPROVIDER_GETALBUMARTDATA = 10,
		SVC_ALBUMARTPROVIDER_ISMINE = 20,
		SVC_ALBUMARTPROVIDER_SETALBUMARTDATA = 30,
		SVC_ALBUMARTPROVIDER_DELETEALBUMART = 40,
	};

	enum
	{
		SERVICETYPE = MK3CC('a','a','p')
	};

};

inline bool svc_albumArtProvider::IsMine(const wchar_t *filename)
{
	return _call(SVC_ALBUMARTPROVIDER_ISMINE, false, filename);
}

inline int svc_albumArtProvider::ProviderType()
{
	return _call(SVC_ALBUMARTPROVIDER_PROVIDERTYPE, (int)ALBUMARTPROVIDER_TYPE_EMBEDDED);
}

inline int svc_albumArtProvider::GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType)
{
	return _call(SVC_ALBUMARTPROVIDER_GETALBUMARTDATA, (int)ALBUMARTPROVIDER_FAILURE, filename, type, bits, len, mimeType);
}

inline int svc_albumArtProvider::SetAlbumArtData(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mimeType)
{
	return _call(SVC_ALBUMARTPROVIDER_SETALBUMARTDATA, (int)ALBUMARTPROVIDER_FAILURE, filename, type, bits, len, mimeType);
}

inline int svc_albumArtProvider::DeleteAlbumArt(const wchar_t *filename, const wchar_t *type)
{
	return _call(SVC_ALBUMARTPROVIDER_DELETEALBUMART, (int)ALBUMARTPROVIDER_FAILURE, filename, type);
}

#endif
