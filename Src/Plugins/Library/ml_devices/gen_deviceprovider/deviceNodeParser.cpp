#include "main.h"
#include "./DeviceNodeParser.h"
#include "../../xml/obj_xml.h"

DeviceNodeParser::DeviceNodeParser()
	: reader(NULL), test(NULL)
{
}

DeviceNodeParser::~DeviceNodeParser()
{
	End();
}


BOOL DeviceNodeParser::Begin(obj_xml *xmlReader, TestSuite *testSuite)
{
	if (NULL != reader || NULL != test)
		return FALSE;

	if (NULL == xmlReader || NULL == testSuite) 
		return FALSE;

	reader = xmlReader;
	reader->AddRef();
		
	test = testSuite;

	reader->xmlreader_registerCallback(L"testprovider\fdevices\fdevice", this);
	
	return TRUE;
}

void DeviceNodeParser::End()
{	
	if (NULL != reader)
	{
		reader->xmlreader_unregisterCallback(this);
		reader->Release();
		reader = NULL;
	}

	if (NULL != test)
		test = NULL;
}


void DeviceNodeParser::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	elementParser.Begin(reader, params);
}

void DeviceNodeParser::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	Device *result;
	if (FALSE != elementParser.End(reader, &result))
	{
		if (NULL != test)
			test->AddDevice(result);
		
		result->Release();
	}
}

void DeviceNodeParser::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
}

#define CBCLASS DeviceNodeParser
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS