#include "WasabiMetadata.h"
#include <shlwapi.h>
#include "../nu/AutoChar.h"

const wchar_t *MP3StreamMetadata::GetName()
{
	return L"MP3 Stream Metadata";
}

GUID MP3StreamMetadata::getGUID()
{
	return MP3StreamMetadataGUID;
}

int MP3StreamMetadata::getFlags()
{
	return METATAG_FILE_INFO;
}

int MP3StreamMetadata::isOurFile(const wchar_t *filename)
{
	if (PathIsURL(filename) && !_wcsicmp(PathFindExtension(filename), L".mp3"))
		return 1;
	else
		return 0;
}

int MP3StreamMetadata::metaTag_open(const wchar_t *filename)
{
	if (metadata.Open(filename) == METADATA_SUCCESS)
		return METATAG_SUCCESS;
	else
		return METATAG_FAILED;
}

void MP3StreamMetadata::metaTag_close()
{
	delete this;
}

int MP3StreamMetadata::getMetaData(const wchar_t *tag, __int8 *buf, int buflenBytes, int datatype)
{
	if (datatype == METATYPE_STRING)
	{
		if (metadata.GetExtendedData(AutoChar(tag), (wchar_t *)buf, buflenBytes/sizeof(wchar_t)))
			return METATAG_SUCCESS;
		else
			return METATAG_UNKNOWN_TAG;
	}
	else
		return METATAG_FAILED;
}

#define CBCLASS MP3StreamMetadata
START_DISPATCH;
CB(SVC_METATAG_GETNAME,getName)
CB(SVC_METATAG_GETGUID,getGUID)
CB(SVC_METATAG_GETFLAGS,getFlags)
CB(SVC_METATAG_ISOURFILE,isOurFile)
CB(SVC_METATAG_OPEN,metaTag_open)
VCB(SVC_METATAG_CLOSE,metaTag_close)
//CB(SVC_METATAG_ENUMTAGS,enumSupportedTag)
//CB(SVC_METATAG_GETTAGSIZE,getTagSize)
CB(SVC_METATAG_GETMETADATA,getMetaData)
//CB(SVC_METATAG_SETMETADATA,setMetaData)
END_DISPATCH;
#undef CBCLASS