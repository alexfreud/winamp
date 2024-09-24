#ifndef NULLSOFT_AGAVE_METADATA_API_METADATA_H
#define NULLSOFT_AGAVE_METADATA_API_METADATA_H

/**
 ** Author: Ben Allison
 ** original date: April 10, 2006
 */
#include <bfc/dispatch.h>
#include "svc_metatag.h"


class api_metadata : public Dispatchable
{
protected:
	api_metadata() {}
	~api_metadata() {}

public:
	// replacement for IPC_GET_EXTENDED_FILE_INFO, it's slow, so only use for converting old code, or one-off metadata grabbing
	int GetExtendedFileInfo(const wchar_t *filename, const wchar_t *tag, wchar_t *data, size_t dataLength);

	// replacement for IPC_SET_EXTENDED_FILE_INFO
	int SetExtendedFileInfo(const wchar_t *filename, const wchar_t *tag, const wchar_t *data);
	int WriteExtendedFileInfo(const wchar_t *filename);

	/** faster methods below.  these return you an object that you can keep re-using (for a single file) 
	 ** it's still your job to call svc_metaTag::metaTag_open() - see note [2]
	 ** call svc_metaTag::close() when you're done  - see note [3]
	 */
	svc_metaTag *GetMetaTagObject(const wchar_t *filename, int flags=METATAG_ALL, GUID *exclude=0, int numExcludes=0); // see note [1]
	svc_metaTag *GetMetaTagObject(const GUID metaTagGuid); // gets a specific svc_metaTag object by GUID

	/** 
	 ** Retrieves a unique key for a given field name
	 ** if one already exists, that index is returned
	 ** returns -1 on failure/not implemented
	 **/
	uint32_t GenerateKey(const wchar_t *field); 


	DISPATCH_CODES
	{
		API_METADATA_GETEXTENDEDFILEINFO    = 10,
		API_METADATA_SETEXTENDEDFILEINFO    = 11,
		API_METADATA_WRITEEXTENDEDFILEINFO  = 12,
		API_METADATA_GETMETATAGOBJECT       = 20,
		API_METADATA_GETMETATAGOBJECTBYGUID = 30,
		API_METADATA_GENERATEKEY            = 40,
	};
};

/**
 ** [1] flags can be set to only use certain metadata providers, file info, database, online lookup (CDDB, etc), guessing
 **     exclude list can be use to exclude certain metatag GUIDs.  This is useful for metadata services that need to look up metadata themselves
 **     e.g. CDDB might need to get a Disc ID, media library wants to ask for info to fill itself in.  Neither services wants to have themselves
 **     be asked, and might not want a "guessing" metatag provider to be used, either.
 ** [2] these methods could technically open the file also, but we've left that out to allow for some flexibility
 **     e.g. someone might just be looking for the metaTag service name or GUID.
 ** [3] you need to close it even if you never opened it.  This allows the object to "self-destruct".  
 **     If we didn't do this, we would also have to pass back the service factory
 **
 */

inline int api_metadata::GetExtendedFileInfo(const wchar_t *filename, const wchar_t *tag, wchar_t *data, size_t dataLength)
{
	return _call(API_METADATA_GETEXTENDEDFILEINFO, (int)0, filename, tag, data, dataLength);
}

inline int api_metadata::SetExtendedFileInfo(const wchar_t *filename, const wchar_t *tag, const wchar_t *data)
{
	return _call(API_METADATA_SETEXTENDEDFILEINFO, (int)0, filename, tag, data);
}

inline int api_metadata::WriteExtendedFileInfo(const wchar_t *filename)
{
	return _call(API_METADATA_WRITEEXTENDEDFILEINFO, (int)0, filename);
}


inline svc_metaTag *api_metadata::GetMetaTagObject(const wchar_t *filename, int flags, GUID *exclude, int numExcludes)
{
	return _call(API_METADATA_GETMETATAGOBJECT, (svc_metaTag *)NULL, filename, flags, exclude, numExcludes);
}

inline svc_metaTag *api_metadata::GetMetaTagObject(const GUID metaTagGuid)
{
	return _call(API_METADATA_GETMETATAGOBJECTBYGUID, (svc_metaTag *)NULL, metaTagGuid);
}

#pragma warning(push)
#pragma warning(disable : 4267)
inline uint32_t api_metadata::GenerateKey(const wchar_t *field)
{
	return _call(API_METADATA_GENERATEKEY, (uint32_t)-1);
}
#pragma warning(pop)

// {DFA89F63-995A-407b-8BC8-827900440727}
static const GUID api_metadataGUID =
{ 0xdfa89f63, 0x995a, 0x407b, { 0x8b, 0xc8, 0x82, 0x79, 0x0, 0x44, 0x7, 0x27 } };

#endif
