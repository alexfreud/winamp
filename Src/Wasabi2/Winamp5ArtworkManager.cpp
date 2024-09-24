#include "Winamp5ArtworkManager.h"
#include "api__wasabi2.h"
#include "metadata/MetadataKeys.h"
#include "nswasabi/ReferenceCounted.h"

int Winamp5ArtworkManager::Artwork_GetArtwork(nx_uri_t filename, unsigned int field, artwork_t *artwork, data_flags_t flags, nx_time_unix_64_t *filename_modified)
{
	if (!AGAVE_API_ALBUMART)
		return NErr_FailedCreate;

	if (field != MetadataKeys::ALBUM)
		return NErr_Unknown;


	if (filename_modified)
	{
		struct __stat64 buffer; 
		if (_wstat64(filename->string, &buffer) == 0)
		{
			*filename_modified = buffer.st_atime;
		}
		else
		{
			*filename_modified = 0;
		}
	}

	wchar_t *mime_type;
	void *bits;
	size_t len;
	int ret = AGAVE_API_ALBUMART->GetAlbumArtData(filename->string, L"cover", &bits, &len, &mime_type);
	if (ret != 0)
		return NErr_Empty;

	NXDataCreate(&artwork->data, bits, len);
	WASABI_API_MEMMGR->sysFree(bits);

	if (mime_type)
	{
		ReferenceCountedNXString mime_type_nx;
		NXStringCreateWithUTF16(&mime_type_nx, mime_type);
		NXDataSetMIME(artwork->data, mime_type_nx);
		WASABI_API_MEMMGR->sysFree(mime_type);
	}
	artwork->height = 0;
	artwork->width = 0;
	return NErr_Success;
}
