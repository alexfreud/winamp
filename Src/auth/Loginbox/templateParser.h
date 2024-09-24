#ifndef NULLSOFT_AUTH_LOGIN_TEMPLATE_PARSER_HEADER
#define NULLSOFT_AUTH_LOGIN_TEMPLATE_PARSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../xml/ifc_xmlreadercallback.h"
#include "./stringBuilder.h"

class obj_xml;
class LoginTemplate;

class LoginTemplateParser : public ifc_xmlreadercallback
{

public:
	LoginTemplateParser();
	~LoginTemplateParser();

public:
	HRESULT Begin(obj_xml *reader, ifc_xmlreaderparams *params);
	HRESULT End(obj_xml *reader, LoginTemplate **instance);

protected:
	void Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag);
	void Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value);
	void Event_XmlError(int linenum, int errcode, const wchar_t *errstr);

protected:
	LoginTemplate *object;
	StringBuilder elementString;
	
protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_AUTH_LOGIN_TEMPLATE_PARSER_HEADER