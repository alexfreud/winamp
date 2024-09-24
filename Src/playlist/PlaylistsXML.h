#ifndef NULLSOFT_AGAVE_PLAYLISTSXML_H
#define NULLSOFT_AGAVE_PLAYLISTSXML_H

#include "../xml/obj_xml.h"
#include "../xml/ifc_xmlreadercallback.h"
#include <api/service/waServiceFactory.h>
class Playlists;

enum
{
	PLAYLISTSXML_SUCCESS         = 0,
	PLAYLISTSXML_FAILURE         = 1,
	PLAYLISTSXML_NO_PARSER       = 2,
	PLAYLISTSXML_NO_FILE         = 3,
	PLAYLISTSXML_XML_PARSE_ERROR = 4,
};

class PlaylistsXML : public ifc_xmlreadercallback
{
public:
	PlaylistsXML( Playlists *_playlists );
	~PlaylistsXML();

	int LoadFile( const wchar_t *filename );

private:
	RECVS_DISPATCH;
	/* XML callbacks */
	void StartTag( const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params );

	obj_xml          *parser        = 0;
	waServiceFactory *parserFactory = 0;
	Playlists        *playlists     = 0;

	wchar_t           rootPath[ MAX_PATH ];
};

#endif