#ifndef NULLSOFT_IN_MP3_ALBUMART_H
#define NULLSOFT_IN_MP3_ALBUMART_H

#include "../Agave/AlbumArt/svc_albumArtProvider.h"

class ASF_AlbumArtProvider : public svc_albumArtProvider
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

#include <api/service/waservicefactory.h>
#include <api/service/services.h>

class AlbumArtFactory : public waServiceFactory
{
public:
	FOURCC GetServiceType();
	const char *GetServiceName();
	GUID GetGUID();
	void *GetInterface(int global_lock);
	int SupportNonLockingInterface();
	int ReleaseInterface(void *ifc);
	const char *GetTestString();
	int ServiceNotify(int msg, int param1, int param2);
	
protected:
	RECVS_DISPATCH;
};


#endif