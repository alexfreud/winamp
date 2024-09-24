#ifndef NULLSOFT_WINAMP_ALBUMART_H
#define NULLSOFT_WINAMP_ALBUMART_H

#include "../Agave/AlbumArt/api_albumart.h"

class AlbumArt : public api_albumart
{
public:
	static const char *getServiceName() { return "Album Art API"; }
	static const GUID getServiceGuid() { return albumArtGUID; }
		
public:
	int GetAlbumArt(const wchar_t *filename, const wchar_t *type, int *w, int *h, ARGB32 **bits);
	int GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType);
	int GetAlbumArt_NoAMG(const wchar_t *filename, const wchar_t *type, int *w, int *h, ARGB32 **bits);
	int GetAlbumArtOrigin(const wchar_t *filename, const wchar_t *type, wchar_t **mimeType);

	int GetAlbumArtTypes(const wchar_t *filename, wchar_t **types);
	int GetValidAlbumArtTypes(const wchar_t *filename, wchar_t **type);
	int SetAlbumArt(const wchar_t *filename, const wchar_t *type, int w, int h, const void *bits, size_t len, const wchar_t *mimeType);
	int DeleteAlbumArt(const wchar_t *filename, const wchar_t *type);
	int CopyAlbumArt(const wchar_t *sourceFilename, const wchar_t *destinationFilename);
protected:
	RECVS_DISPATCH;
};
#endif