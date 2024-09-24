#ifndef NULLSOFT_AGAVE_SVC_METATAG_H
#define NULLSOFT_AGAVE_SVC_METATAG_H

/**
 ** Author: Ben Allison
 ** original date: April 10, 2006
 */
#include <bfc/dispatch.h>
#include <api/service/services.h>
#include <bfc/platform/types.h>
#include <bfc/std_mkncc.h> // for MKnCC()

/* these two GUIDs are to allow you to QueryInterface between readers and writers */
// {9FD00FBE-B707-4743-B630-CC14071EA443}
static const GUID AgaveMetadataReaderIID = 
{ 0x9fd00fbe, 0xb707, 0x4743, { 0xb6, 0x30, 0xcc, 0x14, 0x7, 0x1e, 0xa4, 0x43 } };

// {3444E4AD-D6B3-4ac4-A08D-C3F935DC7B98}
static const GUID AgaveMetadataWriterIID = 
{ 0x3444e4ad, 0xd6b3, 0x4ac4, { 0xa0, 0x8d, 0xc3, 0xf9, 0x35, 0xdc, 0x7b, 0x98 } };

enum
{
    METATAG_SUCCESS = 0,
    METATAG_FAILED = 1,

    METATAG_UNKNOWN_TAG = 2,   // the tag name isn't understood
    METATAG_NO_METADATA = 3,   // returned if the file has no metadata at all
    METATAG_NOT_APPLICABLE = 4,

};

enum
{
    METATYPE_STRING = 0,   // always unicode! and always null terminated
    METATYPE_FILENAME = 0,
    METATYPE_INTEGER = 1,
    METATYPE_UNSIGNED = 2,
    METATYPE_SIZE = 2,
    METATYPE_GUID = 3,
    METATYPE_BINARY = 4,
};

// flags
enum
{
    METATAG_FILE_INFO = 0x1,
    METATAG_ONLINE_LOOKUP = 0x2,
    METATAG_CACHE_DB = 0x4,
    METATAG_GUESS = 0x8,
		METATAG_ALL = 0xFFFFFFFF
};

class svc_metaTag : public Dispatchable
{
protected:
	svc_metaTag() {}
	~svc_metaTag() {}

public:
	/* These methods are to be used by api_metadata */
	static FOURCC getServiceType() { return svc_metaTag::SERVICETYPE; }
	const wchar_t *getName();	// i.e. "ID3v2" or something
	GUID getGUID(); // this needs to be the same GUID that you use when registering your service factory
	int getFlags(); // how this service gets its info
	int isOurFile(const wchar_t *filename);	
	int metaTag_open(const wchar_t *filename);
	void metaTag_close(); // self-destructs when this is called (you don't need to call serviceFactory->releaseInterface)

	/* user API starts here */
	const wchar_t *enumSupportedTag(int n, int *datatype = NULL);	// returns a list of understood tags. might not be complete (see note [1])
	int getTagSize(const wchar_t *tag, size_t *sizeBytes); // always gives you BYTES, not characters (be careful with your strings)
	int getMetaData(const wchar_t *tag, uint8_t *buf, int buflenBytes, int datatype = METATYPE_STRING); // buflen is BYTES, not characters (be careful with your strings)
	int setMetaData(const wchar_t *tag, const uint8_t *buf, int buflenBytes, int datatype = METATYPE_STRING);
public:
	DISPATCH_CODES
	{
	    SVC_METATAG_GETNAME = 10,
	    SVC_METATAG_GETGUID = 20,
			SVC_METATAG_GETFLAGS = 30,
	    SVC_METATAG_ISOURFILE = 40,
	    SVC_METATAG_OPEN = 50,
	    SVC_METATAG_CLOSE = 60,
	    SVC_METATAG_ENUMTAGS = 100,
	    SVC_METATAG_GETTAGSIZE = 110,
	    SVC_METATAG_GETMETADATA = 120,
	    SVC_METATAG_SETMETADATA = 130,

	};
	enum
	{
		SERVICETYPE = MK4CC('m','t','t','g')
	};
};

/** Notes:
 ** [1] Many metadata getters rely on an underlying library, and some metadata systems (e.g. Vorbis) are open-ended.  As a result, there might be no way of 
 **     generating a complete list
*/

inline const wchar_t *svc_metaTag::getName()
{
	return _call(SVC_METATAG_GETNAME, (const wchar_t*)NULL);
}

inline GUID svc_metaTag::getGUID()
{
	return _call(SVC_METATAG_GETGUID, GUID_NULL);
}

inline int svc_metaTag::getFlags()
{
	return _call(SVC_METATAG_GETFLAGS, 0);
}

inline int svc_metaTag::isOurFile(const wchar_t *filename)
{
	return _call(SVC_METATAG_ISOURFILE, (int)0, filename);
}

inline int svc_metaTag::metaTag_open(const wchar_t *filename)
{
	return _call(SVC_METATAG_OPEN, (int)METATAG_FAILED, filename);
}

inline void svc_metaTag::metaTag_close()
{
	_voidcall(SVC_METATAG_CLOSE);
}

inline const wchar_t *svc_metaTag::enumSupportedTag(int n, int *datatype)
{
	return _call(SVC_METATAG_ENUMTAGS, (const wchar_t *)NULL, n, datatype);
}

inline int svc_metaTag::getTagSize(const wchar_t *tag, size_t *sizeBytes)
{
	return _call(SVC_METATAG_GETTAGSIZE, (int)0, tag, sizeBytes);
}

inline int svc_metaTag::getMetaData(const wchar_t *tag, uint8_t *buf, int buflenBytes, int datatype)
{
	return _call(SVC_METATAG_GETMETADATA, (int)METATAG_FAILED, tag, buf, buflenBytes, datatype);
}

inline int svc_metaTag::setMetaData(const wchar_t *tag, const uint8_t *buf, int buflenBytes, int datatype)
{
	return _call(SVC_METATAG_SETMETADATA, (int)METATAG_FAILED, tag, buf, buflenBytes, datatype);
}


#endif
