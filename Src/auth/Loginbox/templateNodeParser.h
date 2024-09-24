#ifndef NULLSOFT_AUTH_LOGIN_TEMPLATE_NODE_PARSER_HEADER
#define NULLSOFT_AUTH_LOGIN_TEMPLATE_NODE_PARSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../xml/ifc_xmlreadercallback.h"
#include "./templateParser.h"

class obj_xml;
class LoginTemplate;
class LoginProvider;

class LoginTemplateNodeParser : public ifc_xmlreadercallback
{

public:
	LoginTemplateNodeParser();
	~LoginTemplateNodeParser();

public:
	HRESULT Begin(obj_xml *reader, LoginProvider *provider);
	HRESULT End();

protected:
	void Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag);
	void Event_XmlError(int linenum, int errcode, const wchar_t *errstr);

protected:
	obj_xml *reader;
	LoginTemplateParser elementParser;
	LoginProvider *provider;
		
protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_AUTH_LOGIN_TEMPLATE_NODE_PARSER_HEADER