#include "main.h"
#include "MetaTag.h"
#include "FileTypes.h"
#include "TagAlias.h"

ASFMetaTag::~ASFMetaTag()
{
	delete info;
}

const wchar_t *ASFMetaTag::getName()
{
	return L"ASF Metadata";
}

GUID ASFMetaTag::getGUID()
{
	return getServiceGuid();
}

int ASFMetaTag::getFlags()
{
	return METATAG_FILE_INFO;
}

int ASFMetaTag::isOurFile(const wchar_t *filename)
{
	const wchar_t *ext = PathFindExtension(filename);
	return !lstrcmpiW(ext, L".WMA")
		|| !lstrcmpiW(ext, L".WMV")
		|| !lstrcmpiW(ext, L".ASF");
	
}

int ASFMetaTag::metaTag_open(const wchar_t *filename)
{
	info = new WMInformation(filename);
	return METATAG_SUCCESS; // TODO: can we verify this?
}

void ASFMetaTag::metaTag_close()
{
	delete this;
}

const wchar_t *ASFMetaTag::enumSupportedTag(int n, int *datatype)
{
	return 0;
}

int ASFMetaTag::getTagSize(const wchar_t *tag, size_t *sizeBytes)
{
	size_t size;
	const wchar_t *tagName = GetAlias(tag);
	if (info && info->GetAttributeSize(tagName, size))
	{
		*sizeBytes = size;
		return METATAG_SUCCESS;
	}
	else
	{
		return METATAG_UNKNOWN_TAG;
	}
}

int ASFMetaTag::getMetaData(const wchar_t *tag, __int8 *buf, int buflenBytes, int datatype)
{
	const wchar_t *tagName = GetAlias(tag);
	info->GetAttribute(tagName,  reinterpret_cast<wchar_t *>(buf), buflenBytes / sizeof(wchar_t));
	return METATAG_SUCCESS;
	//return METATAG_FAILED;
}

int ASFMetaTag::setMetaData(const wchar_t *tag, const __int8 *buf, int buflenBytes, int datatype )
{
	return METATAG_FAILED;
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS ASFMetaTag
START_DISPATCH;
CB(SVC_METATAG_GETNAME,getName)
	 CB(SVC_METATAG_GETGUID,getGUID)
	 CB(SVC_METATAG_GETFLAGS,getFlags)
	 CB(SVC_METATAG_ISOURFILE,isOurFile)
	 CB(SVC_METATAG_OPEN,metaTag_open)
	 VCB(SVC_METATAG_CLOSE,metaTag_close)
	 CB(SVC_METATAG_ENUMTAGS,enumSupportedTag)
	 CB(SVC_METATAG_GETTAGSIZE,getTagSize)
	 CB(SVC_METATAG_GETMETADATA,getMetaData)
	 CB(SVC_METATAG_SETMETADATA,setMetaData)
END_DISPATCH;
