#ifndef NULLSOFT_WINAMP_XML_SERVICE_PARSER_HEADER
#define NULLSOFT_WINAMP_XML_SERVICE_PARSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../xml/ifc_xmlreadercallback.h"
#include "./stringBuilder.h"
#include <vector>

class obj_xml;
class OmService;
class ifc_omservice;
class ifc_omservicehost;
class ifc_omstoragehandler;
class ifc_omstoragehandlerenum;

class XmlServiceParser : public ifc_xmlreadercallback
{
public:
	XmlServiceParser();
	~XmlServiceParser();

public:
	HRESULT Initialize(obj_xml *reader, LPCWSTR match, ifc_omservicehost *host);
	HRESULT Finish(HRESULT *parserResult, ifc_omservice **ppService);
	HRESULT GetActive();
	HRESULT RegisterHandlers(ifc_omstoragehandlerenum *handlerEnum);
	
protected:
	void OnStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void OnEndElement(const wchar_t *xmlpath, const wchar_t *xmltag);
	void OnCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str);
	void OnError(int linenum, int errcode, const wchar_t *errstr);

protected:
	typedef std::vector<ifc_omstoragehandler*> HandlerList;
protected:
	StringBuilder buffer;
	HandlerList handlerList;
	BYTE *checkList;
	size_t checkSize;

	obj_xml	*parser;
	OmService *service;
	HRESULT result;

protected:
	RECVS_DISPATCH;

};
#endif// NULLSOFT_WINAMP_XML_SERVICE_PARSER_HEADER