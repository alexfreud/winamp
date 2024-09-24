#include "main.h"
#include "./DeviceConnectionNodeParser.h"
#include "../../xml/obj_xml.h"

DeviceConnectionNodeParser::DeviceConnectionNodeParser()
	: reader(NULL), test(NULL)
{
}

DeviceConnectionNodeParser::~DeviceConnectionNodeParser()
{
	End();
}


BOOL DeviceConnectionNodeParser::Begin(obj_xml *xmlReader, TestSuite *testSuite)
{
	if (NULL != reader || NULL != test)
		return FALSE;

	if (NULL == xmlReader || NULL == testSuite) 
		return FALSE;

	reader = xmlReader;
	reader->AddRef();
		
	test = testSuite;

	reader->xmlreader_registerCallback(L"testprovider\fconnections\fconnection", this);
	
	return TRUE;
}

void DeviceConnectionNodeParser::End()
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


void DeviceConnectionNodeParser::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	elementParser.Begin(reader, params);
}

void DeviceConnectionNodeParser::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	ifc_deviceconnection *result;
	if (FALSE != elementParser.End(reader, &result))
	{
		if (NULL != test)
			test->AddConnection(result);
		
		result->Release();
	}
}

void DeviceConnectionNodeParser::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
}

#define CBCLASS DeviceConnectionNodeParser
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS