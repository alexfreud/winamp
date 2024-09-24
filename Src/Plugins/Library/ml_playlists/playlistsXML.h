#ifndef NULLSOFT_ML_PLAYLISTS_PLAYLISTSXML_H
#define NULLSOFT_ML_PLAYLISTS_PLAYLISTSXML_H

#include "../xml/api_xml.h"
#include "../xml/api_xmlreadercallback.h"
#include "api.h"
#include <api/service/waServiceFactory.h>

class PlaylistsXML : public api_xmlreadercallback
{
public:
	PlaylistsXML();
	~PlaylistsXML();
	void LoadFile(const wchar_t *filename);

private:
	RECVS_DISPATCH;
	/* XML callbacks */
	void StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, api_xmlreaderparams *params);


	api_xml *parser;
	waServiceFactory *parserFactory;
};


#endif