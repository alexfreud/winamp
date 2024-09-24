#ifndef NULLSOFT_AUTH_LOGIN_XML_INT32_PARSER_HEADER
#define NULLSOFT_AUTH_LOGIN_XML_INT32_PARSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../xml/ifc_xmlreadercallback.h"

class obj_xml;

class XmlInt32Parser: public ifc_xmlreadercallback
{

public:
	XmlInt32Parser();
	~XmlInt32Parser();

public:
	HRESULT GetValue(INT *pValue);

protected:
	void Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag);
	void Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value);
	void Event_XmlError(int linenum, int errcode, const wchar_t *errstr);

protected:
	INT value;
	HRESULT result;
	WCHAR szBuffer[33];

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_AUTH_LOGIN_XML_INT32_PARSER_HEADER