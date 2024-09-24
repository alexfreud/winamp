#ifndef _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_NODE_PARSER_HEADER
#define _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_NODE_PARSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../xml/ifc_xmlreadercallback.h"
#include "./deviceParser.h"

class obj_xml;

class DeviceNodeParser : public ifc_xmlreadercallback
{

public:
	DeviceNodeParser();
	~DeviceNodeParser();

public:
	BOOL Begin(obj_xml *xmlReader, TestSuite *testSuite);
	void End();

protected:
	void Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag);
	void Event_XmlError(int linenum, int errcode, const wchar_t *errstr);

protected:
	obj_xml *reader;
	DeviceParser elementParser;
	TestSuite *test;
		
protected:
	RECVS_DISPATCH;
};

#endif //_NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_NODE_PARSER_HEADER