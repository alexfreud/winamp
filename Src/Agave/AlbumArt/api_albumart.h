#ifndef NULLSOFT_WINAMP_API_ALBUMART_H
#define NULLSOFT_WINAMP_API_ALBUMART_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
#include <api/service/services.h>

enum
{
	ALBUMART_SUCCESS = 0,
	ALBUMART_FAILURE = 1,
};

enum
{
	ALBUMART_NONE = 0,
	ALBUMART_EMBEDDED = 1,
	ALBUMART_ALBUM = 2,
	ALBUMART_NFO = 3,
	ALBUMART_FILENAME = 4,
	ALBUMART_FOLDER = 5,
	ALBUMART_FRONT = 6,
	ALBUMART_ARTWORK = 7,
};

class api_albumart : public Dispatchable
{
protected:
	api_albumart(){}
	~api_albumart(){}
public:
	static FOURCC getServiceType() { return WaSvc::UNIQUE; } 
	// use WASABI_API_MEMMGR->sysFree on the bits you get back from here.
	// if this function fails (return value != ALBUMART_SUCCESS), there is no guarantee about the values
	// in w, h or bits.  please, please, please don't check bits == 0 for success/failure
	int GetAlbumArt(const wchar_t *filename, const wchar_t *type, int *w, int *h, ARGB32 **bits);
	// hack alert
	int GetAlbumArt_NoAMG(const wchar_t *filename, const wchar_t *type, int *w, int *h, ARGB32 **bits);
	// use to get still-compressed data
	int GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType);
	// use to get the origin of the artwork for the file, e.g. folder, embedded
	int GetAlbumArtOrigin(const wchar_t *filename, const wchar_t *type, wchar_t **mimeType);

	// use WASABI_API_MEMMGR->sysFree to free types
	int GetAlbumArtTypes(const wchar_t *filename, wchar_t **types);

	int GetValidAlbumArtTypes(const wchar_t *filename, wchar_t **types);

	// returns ALBUMART_SUCCESS or ALBUMART_FAILURE
	// if mimeType is NULL, bits is ARGB32. w and h are not used for mimeTypes where the dimentions are in the data
	// if bits is NULL, this removes albumart.
	int SetAlbumArt(const wchar_t *filename, const wchar_t *type, int w, int h, const void *bits, size_t len, const wchar_t *mimeType);

	int DeleteAlbumArt(const wchar_t *filename, const wchar_t *type);

	// copies all album art from one file to another
	// also copies bits like folder.jpg if the two files live in different places
	// if you don't like the logic of this function, implement your own using svc_albumArtProvider directly
	int CopyAlbumArt(const wchar_t *sourceFilename, const wchar_t *destinationFilename);
	DISPATCH_CODES
	{
		API_ALBUMART_GETALBUMART = 10,
		API_ALBUMART_GETALBUMART_NOAMG = 11,
		API_ALBUMART_GETALBUMARTDATA = 12,
		API_ALBUMART_GETALBUMARTORIGIN = 13,
		API_ALBUMART_GETALBUMARTTYPES = 20,
		API_ALBUMART_GETVALIDALBUMARTTYPES = 30,
		API_ALBUMART_SETALBUMART = 40,
		API_ALBUMART_DELETEALBUMART = 50,
		API_ALBUMART_COPYALBUMART = 60,
	};
};

inline int api_albumart::GetAlbumArt(const wchar_t *filename, const wchar_t *type, int *w, int *h, ARGB32 **bits)
{
	return _call(API_ALBUMART_GETALBUMART, (int)ALBUMART_FAILURE, filename, type, w, h, bits);
}

inline int api_albumart::GetAlbumArt_NoAMG(const wchar_t *filename, const wchar_t *type, int *w, int *h, ARGB32 **bits)
{
	return _call(API_ALBUMART_GETALBUMART_NOAMG, (int)ALBUMART_FAILURE, filename, type, w, h, bits);
}

inline int api_albumart::GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType)
{
		return _call(API_ALBUMART_GETALBUMARTDATA, (int)ALBUMART_FAILURE, filename, type, bits, len, mimeType);
}

inline int api_albumart::GetAlbumArtOrigin(const wchar_t *filename, const wchar_t *type, wchar_t **mimeType)
{
	return _call(API_ALBUMART_GETALBUMARTORIGIN, (int)ALBUMART_FAILURE, filename, type, mimeType);
}

inline int api_albumart::GetAlbumArtTypes(const wchar_t *filename, wchar_t **types)
{
	return _call(API_ALBUMART_GETALBUMARTTYPES, (int)ALBUMART_FAILURE, filename, types);
}

inline int api_albumart::GetValidAlbumArtTypes(const wchar_t *filename, wchar_t **types)
{
	return _call(API_ALBUMART_GETVALIDALBUMARTTYPES, (int)ALBUMART_FAILURE, filename, types);
}

inline int api_albumart::SetAlbumArt(const wchar_t *filename, const wchar_t *type, int w, int h, const void *bits, size_t len, const wchar_t *mimeType)
{
	return _call(API_ALBUMART_SETALBUMART, (int)ALBUMART_FAILURE, filename, type, w, h, bits, len, mimeType);
}

inline int api_albumart::DeleteAlbumArt(const wchar_t *filename, const wchar_t *type)
{
	return _call(API_ALBUMART_DELETEALBUMART, (int)ALBUMART_FAILURE, filename, type);
}

inline int api_albumart::CopyAlbumArt(const wchar_t *sourceFilename, const wchar_t *destinationFilename)
{
		return _call(API_ALBUMART_COPYALBUMART, (int)ALBUMART_FAILURE, sourceFilename, destinationFilename);
}

// {AC4C4468-F91F-41f3-A5FA-E2B81DC6EB3A}
static const GUID albumArtGUID = 
{ 0xac4c4468, 0xf91f, 0x41f3, { 0xa5, 0xfa, 0xe2, 0xb8, 0x1d, 0xc6, 0xeb, 0x3a } };


#endif