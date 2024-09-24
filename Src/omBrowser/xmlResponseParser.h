#ifndef NULLSOFT_WINAMP_XML_RESPONSE_PARSER_HEADER
#define NULLSOFT_WINAMP_XML_RESPONSE_PARSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../xml/ifc_xmlreadercallback.h"
#include "./xmlServiceParser.h"
#include "./stringBuilder.h"
#include <deque>

class obj_xml;
class ifc_omservicehost;
class ifc_omstorage;

class XmlResponseParser : public ifc_xmlreadercallback
{
public:
	XmlResponseParser();
	~XmlResponseParser();

public:
	HRESULT Initialize(obj_xml *xml, ifc_omservicehost *serviceHost);
	HRESULT Finish();
	HRESULT Reset();

	HRESULT GetCode(UINT *value);
	HRESULT GetText(LPWSTR pszBuffer, UINT cchBufferMax);
	
	HRESULT PeekService(ifc_omservice **service);

protected:
	void OnStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void OnEndElement(const wchar_t *xmlpath, const wchar_t *xmltag);
	void OnCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str);
	void OnError(int linenum, int errcode, const wchar_t *errstr);

protected:
	typedef std::deque<ifc_omservice*> ServiceDeque;

protected:
	obj_xml	*reader;
	UINT code;
	LPWSTR text;

	XmlServiceParser parser;
	ifc_omservicehost *host;
	ServiceDeque deque;
	StringBuilder string;
protected:
	RECVS_DISPATCH;

};
#endif// NULLSOFT_WINAMP_XML_RESPONSE_PARSER_HEADER