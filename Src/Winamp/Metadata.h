#ifndef NULLSOFT_WINAMP_METADATA_H
#define NULLSOFT_WINAMP_METADATA_H
/**
 ** Author: Ben Allison
 ** original date: April 10, 2006
 */
#include "../Agave/Metadata/api_metadata.h"
#include <vector>
#include "../nu/AutoLock.h"
class Metadata : public api_metadata
{
public:
	static const char *getServiceName() { return "Metadata API"; }
	static const GUID getServiceGuid() { return api_metadataGUID; }
public:
	~Metadata();
	int GetExtendedFileInfo(const wchar_t *filename, const wchar_t *tag, wchar_t *data, size_t dataLength);
	int SetExtendedFileInfo(const wchar_t *filename, const wchar_t *tag, const wchar_t *data);
	int WriteExtendedFileInfo(const wchar_t *filename);
	svc_metaTag *GetMetaTagObject(const wchar_t *filename, int flags, GUID *exclude, int numExcludes);
	svc_metaTag *GetMetaTagObjectByGUID(const GUID metaTagGuid); 
	uint32_t GenerateKey(const wchar_t *field);
protected:
	RECVS_DISPATCH;

private:
	// this is hella slow but it'll get the ball rolling on implementing this
	std::vector<wchar_t*> keys;
	Nullsoft::Utility::LockGuard keyGuard;
};

extern Metadata *metadata;
#endif