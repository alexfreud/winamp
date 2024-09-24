#ifndef NULLSOFT_AUTH_LOGIN_COMMAND_NODE_PARSER_HEADER
#define NULLSOFT_AUTH_LOGIN_COMMAND_NODE_PARSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../xml/ifc_xmlreadercallback.h"
#include "./commandParser.h"

class obj_xml;
class LoginCommand;
class LoginProvider;

class LoginCommandNodeParser : public ifc_xmlreadercallback
{

public:
	LoginCommandNodeParser();
	~LoginCommandNodeParser();

public:
	HRESULT Begin(obj_xml *reader, LoginProvider *provider);
	HRESULT End();

protected:
	void Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag);
	void Event_XmlError(int linenum, int errcode, const wchar_t *errstr);

protected:
	obj_xml *reader;
	LoginCommandParser elementParser;
	LoginProvider *provider;
		
protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_AUTH_LOGIN_COMMAND_NODE_PARSER_HEADER