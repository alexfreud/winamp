#pragma once
#include "../Agave/Metadata/svc_metatag.h"
#include "Metadata.h"

// {9937E02D-205B-4964-86A9-F784D9C05F5D}
static const GUID MP3StreamMetadataGUID =
  { 0x9937e02d, 0x205b, 0x4964, { 0x86, 0xa9, 0xf7, 0x84, 0xd9, 0xc0, 0x5f, 0x5d } };

class MP3StreamMetadata : public svc_metaTag
{
private:
	/* These methods are to be used by api_metadata */
	const wchar_t *GetName();
	GUID getGUID(); // this needs to be the same GUID that you use when registering your service factory
	int getFlags(); // how this service gets its info
	int isOurFile(const wchar_t *filename);
	int metaTag_open(const wchar_t *filename);
	void metaTag_close(); // self-destructs when this is called (you don't need to call serviceFactory->releaseInterface)

	/* user API starts here */
	const wchar_t *enumSupportedTag(int n, int *datatype = NULL);	// returns a list of understood tags. might not be complete (see note [1])
	int getTagSize(const wchar_t *tag, size_t *sizeBytes); // always gives you BYTES, not characters (be careful with your strings)
	int getMetaData(const wchar_t *tag, __int8 *buf, int buflenBytes, int datatype = METATYPE_STRING); // buflen is BYTES, not characters (be careful with your strings)
	int setMetaData(const wchar_t *tag, const __int8 *buf, int buflenBytes, int datatype = METATYPE_STRING);
private:
	Metadata metadata;

	RECVS_DISPATCH;
};

