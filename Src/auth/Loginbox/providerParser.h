#ifndef NULLSOFT_AUTH_LOGIN_PROVIDER_PARSER_HEADER
#define NULLSOFT_AUTH_LOGIN_PROVIDER_PARSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../xml/ifc_xmlreadercallback.h"
#include "./stringBuilder.h"
#include "./templateNodeParser.h"
#include "./commandNodeParser.h"

class obj_xml;
class LoginProvider;

#define PROVIDER_TAG_MAX	6

class LoginProviderParser : public ifc_xmlreadercallback
{

public:
	LoginProviderParser();
	~LoginProviderParser();
public:
	HRESULT SetReader(obj_xml *pReader);
	HRESULT Begin(ifc_xmlreaderparams *params);
	HRESULT End(LoginProvider **ppProvider);

protected:
	void Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag);
	void Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value);
	void Event_XmlError(int linenum, int errcode, const wchar_t *errstr);

protected:
	obj_xml *reader;
	StringBuilder elementString;
	LoginProvider *provider;
	LoginTemplateNodeParser templateNodeParser;
	LoginCommandNodeParser	commandNodeParser;
	BOOL hitList[PROVIDER_TAG_MAX];

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_AUTH_LOGIN_PROVIDER_PARSER_HEADER