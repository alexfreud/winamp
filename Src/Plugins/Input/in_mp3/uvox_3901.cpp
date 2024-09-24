#include "uvox_3901.h"
#include "api__in_mp3.h"
#include "api/service/waservicefactory.h"
#include <strsafe.h>
#include "in2.h"
extern In_Module mod;

#include "FactoryHelper.h"


Ultravox3901::Ultravox3901() : parser(0)
{
	title[0]=album[0]=artist[0]=album_art_url[0]=0;
	ServiceBuild(parser, obj_xmlGUID);
	if (parser)
	{
		parser->xmlreader_registerCallback(L"metadata\fsong\f*", this);
		parser->xmlreader_open();
	}
}

Ultravox3901::~Ultravox3901()
{
	ServiceRelease(parser, obj_xmlGUID);
}

int Ultravox3901::Parse(const char *xml_data)
{
	if (parser)
	{
		int ret = parser->xmlreader_feed((void *)xml_data, strlen(xml_data));
		if (ret != API_XML_SUCCESS)
			return ret;
		return parser->xmlreader_feed(0, 0);
	}
	return API_XML_FAILURE;
}

void Ultravox3901::TextHandler(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str)
{
	if (!_wcsicmp(xmltag, L"name"))
	{
		StringCbCatW(title, sizeof(title), str);
	}
	else if (!_wcsicmp(xmltag, L"album"))
	{
		StringCbCatW(album, sizeof(album), str);
	}
	else if (!_wcsicmp(xmltag, L"artist"))
	{
		StringCbCatW(artist, sizeof(artist), str);
	}
	else if (!_wcsicmp(xmltag, L"album_art"))
	{
		StringCbCatW(album_art_url, sizeof(album_art_url), str);
	}
}

int Ultravox3901::GetExtendedData(const char *tag, wchar_t *data, int dataLen)
{
	if (!_stricmp(tag, "uvox/title"))
		StringCchCopy(data, dataLen, title);
	else if (!_stricmp(tag, "uvox/album"))
		StringCchCopy(data, dataLen, album);
	else if (!_stricmp(tag, "uvox/artist"))
		StringCchCopy(data, dataLen, artist);
	else if (!_stricmp(tag, "uvox/albumart"))
		StringCchCopy(data, dataLen, album_art_url);
	else
		return 0;

	return 1;
}

#define CBCLASS Ultravox3901
START_DISPATCH;
VCB(ONCHARDATA, TextHandler)
END_DISPATCH;
#undef CBCLASS