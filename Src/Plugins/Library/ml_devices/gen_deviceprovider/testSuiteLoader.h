#ifndef _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_TEST_SUITE_LOADER_HEADER
#define _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_TEST_SUITE_LOADER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../xml/ifc_xmlreadercallback.h"
#include <vector>
#include "./DeviceNodeParser.h"
#include "./DeviceTypeNodeParser.h"
#include "./DeviceConnectionNodeParser.h"
#include "./DeviceCommandNodeParser.h"

class obj_xml;

#define LOADER_TAG_MAX	2

class TestSuiteLoader : public ifc_xmlreadercallback
{

public:
	TestSuiteLoader();
	~TestSuiteLoader();

public:
	BOOL Load(const wchar_t *path, TestSuite *testSuite);

private:
	BOOL FeedFile(obj_xml *reader, HANDLE hFile, DWORD bufferSize);

	void Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag);
	void Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value);
	void Event_XmlError(int linenum, int errcode, const wchar_t *errstr);

protected:
	friend static void CALLBACK LoaderTag_ImageBase(TestSuiteLoader *loader, const wchar_t *value);
	friend static void CALLBACK LoaderTag_Connect(TestSuiteLoader *loader, const wchar_t *value);
protected:
	typedef std::vector<char*> NameList;

protected:
	StringBuilder elementString;
	DeviceNodeParser deviceParser;
	DeviceTypeNodeParser typeParser;
	DeviceConnectionNodeParser connectionParser;
	DeviceCommandNodeParser commandParser;
	BOOL hitList[LOADER_TAG_MAX];

	wchar_t *imageBase;
	
	NameList connectList;

protected:
	RECVS_DISPATCH;
};

#endif // _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_TEST_SUITE_LOADER_HEADER