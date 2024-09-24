#ifndef NULLSOFT_PLAYLIST_ASX_LOADER_H
#define NULLSOFT_PLAYLIST_ASX_LOADER_H

#include "../playlist/ifc_playlistloader.h"
#include "../playlist/ifc_playlistloadercallback.h"
#include <stdio.h>

class obj_xml;
class api_httpreceiver;
class ASXLoader : public ifc_playlistloader
{
public:
	ASXLoader() : inTag(false), inQuotes(false) {}
	int Load(const wchar_t *filename, ifc_playlistloadercallback *playlist);

private:
	int LoadURL(obj_xml *parser, const wchar_t *url);
	int LoadFile(obj_xml *parser, const wchar_t *filename);
	void RunXMLDownload(api_httpreceiver *http, obj_xml *parser);
	int FeedXMLHTTP(api_httpreceiver *http, obj_xml *parser, bool *noData);
	int GayASX_to_XML_converter(obj_xml *parser, char *buffer, int len);
	

protected:
	bool inTag;
	bool inQuotes;
	RECVS_DISPATCH;
};
#endif