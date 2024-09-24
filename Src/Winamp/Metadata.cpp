#include "main.h"
#include "Metadata.h"
#include "api.h"
#include <api/service/waservicefactory.h>

/**
 ** Author: Ben Allison
 ** original date: April 10, 2006
 */
Metadata::~Metadata()
{
	for ( wchar_t *l_key : keys )
		free( l_key );

	keys.clear();
}

int Metadata::GetExtendedFileInfo(const wchar_t *filename, const wchar_t *tag, wchar_t *data, size_t dataLength)
{
	// benski> old way CUT: return in_get_extended_fileinfoW(filename, tag, data, dataLength);
/*
	int result = 0; // failure
	svc_metaTag *metaTag = GetMetaTagObject(filename, METATAG_ALL, 0, 0);
	if (metaTag)
	{
		if (metaTag->metaTag_open(filename) == METATAG_SUCCESS)
			metaTag->getMetaData(tag, reinterpret_cast<__int8 *>(data), dataLength*sizeof(wchar_t), METATYPE_STRING);

		metaTag->metaTag_close();
	}
	else // TODO: we could get rid of this as soon as we make extendedMetaTag (see TODO at end of file)
		*/
	return in_get_extended_fileinfoW(filename, tag, data, dataLength);

	//return !!(result == METATAG_SUCCESS);
}

int Metadata::SetExtendedFileInfo(const wchar_t *filename, const wchar_t *tag, const wchar_t *data)
{
	return in_set_extended_fileinfoW(filename, tag, const_cast<wchar_t *>(data));
}

int Metadata::WriteExtendedFileInfo(const wchar_t *filename)
{
	return in_write_extended_fileinfo();
}

inline static bool InList(const GUID guid, GUID *exclude, int numExcludes)
{
	if (!exclude)
		return false;
	while (numExcludes--)
	{
		if (guid == exclude[numExcludes])
			return true;
	}
	return false;
}

svc_metaTag *Metadata::GetMetaTagObject(const wchar_t *filename, int flags, GUID *exclude, int numExcludes)
{
	int i = 0;
	waServiceFactory *sf = 0;
	do
	{
		sf = WASABI_API_SVC->service_enumService(WaSvc::METATAG, i++);
		if (sf)
		{
			svc_metaTag *metaTag = static_cast<svc_metaTag *>(sf->getInterface());
			if (metaTag
			    && !InList(metaTag->getGUID(), exclude, numExcludes)
			    && flags & metaTag->getFlags()
			    && metaTag->isOurFile(filename))
				return metaTag;

			sf->releaseInterface(metaTag);
		}

	}
	while (sf);
	return 0;
}

svc_metaTag *Metadata::GetMetaTagObjectByGUID(const GUID metaTagGuid)
{
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(metaTagGuid);
	if (sf && sf->getServiceType() == WaSvc::METATAG) // just to be safe
		return static_cast<svc_metaTag *>(sf->getInterface());

	return NULL;
}

uint32_t Metadata::GenerateKey(const wchar_t *field)
{
	Nullsoft::Utility::AutoLock keyLock(keyGuard);
	// TODO this is hella slow but it'll get the ball rolling on implementing this
	for (size_t i=0;i!=keys.size();i++)
	{
		if (!_wcsicmp(field, keys[i]))
			return (uint32_t)i;
	}

	keys.push_back(_wcsdup(field));
	return (uint32_t)keys.size()-1;
}
// TODO: extendedMetaTag, derived from svc_metaTag, that uses getExtendedFileInfo

#define CBCLASS Metadata
START_DISPATCH;
CB(API_METADATA_GETEXTENDEDFILEINFO, GetExtendedFileInfo)
CB(API_METADATA_SETEXTENDEDFILEINFO, SetExtendedFileInfo)
CB(API_METADATA_WRITEEXTENDEDFILEINFO, WriteExtendedFileInfo)
CB(API_METADATA_GETMETATAGOBJECT, GetMetaTagObject)
CB(API_METADATA_GETMETATAGOBJECTBYGUID, GetMetaTagObjectByGUID)
CB(API_METADATA_GENERATEKEY, GenerateKey)
END_DISPATCH;
#undef CBCLASS