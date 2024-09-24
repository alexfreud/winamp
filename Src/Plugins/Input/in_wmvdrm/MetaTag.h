#ifndef NULLSOFT_IN_WMVDRM_METATAG_H
#define NULLSOFT_IN_WMVDRM_METATAG_H

#include "../Agave/Metadata/svc_metatag.h"
#include "WMInformation.h"

// {8FF721E1-2FF4-4721-A7D5-60FDB32FEB1F}
static const GUID asfMetaTagGUID = 
{ 0x8ff721e1, 0x2ff4, 0x4721, { 0xa7, 0xd5, 0x60, 0xfd, 0xb3, 0x2f, 0xeb, 0x1f } };

class ASFMetaTag : public svc_metaTag
{
public:
		static const GUID getServiceGuid() { return asfMetaTagGUID; }
		static const char *getServiceName() { return "ASF Metadata"; }
public:
	ASFMetaTag() : info(0)
	{
	}
	~ASFMetaTag();

		/* These methods are to be used by api_metadata */
	const wchar_t *getName();	// i.e. "ID3v2" or something
	GUID getGUID(); // this needs to be the same GUID that you use when registering your service factory
	int getFlags(); // how this service gets its info
	int isOurFile(const wchar_t *filename);	
	int metaTag_open(const wchar_t *filename);
	void metaTag_close(); // self-destructs when this is called (you don't need to call serviceFactory->releaseInterface)

	/* user API starts here */
	const wchar_t *enumSupportedTag(int n, int *datatype);	// returns a list of understood tags. might not be complete (see note [1])
	int getTagSize(const wchar_t *tag, size_t *sizeBytes); // always gives you BYTES, not characters (be careful with your strings)
	int getMetaData(const wchar_t *tag, __int8 *buf, int buflenBytes, int datatype); // buflen is BYTES, not characters (be careful with your strings)
	int setMetaData(const wchar_t *tag, const __int8 *buf, int buflenBytes, int datatype);

private:
	WMInformation *info;
	RECVS_DISPATCH;
};


#endif