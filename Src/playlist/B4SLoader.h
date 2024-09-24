#ifndef NULLSOFT_WINAMP_B4S_H
#define NULLSOFT_WINAMP_B4S_H

#include "../xml/obj_xml.h"
#include "../xml/ifc_xmlreadercallback.h"
#include "api__playlist.h"
#include <api/service/waServiceFactory.h>
#include "XMLString.h"
#include "main.h"
#include "ifc_playlistloader.h"

class B4SXML : public ifc_xmlreadercallback
{
public:
	B4SXML();
	ifc_playlistloadercallback *playlist;
	//private:
	
	/* XML callbacks */
	void StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void EndTag(const wchar_t *xmlpath, const wchar_t *xmltag);
	
	XMLString /*name, artist, */title/*, album*/, length; // wa2 only supports title and length in the playlist anyway
	RECVS_DISPATCH;
	wchar_t filename[FILENAME_SIZE];
};

class B4SLoader :  public ifc_playlistloader
{
public:
	B4SLoader();
	~B4SLoader();

	void LoadFile(const char *filename);
	void LoadURL(const char *url);

	int Load(const wchar_t *filename, ifc_playlistloadercallback *playlist);

	RECVS_DISPATCH;

	obj_xml *parser;
	waServiceFactory *parserFactory;
	B4SXML b4sXml;
};

#endif