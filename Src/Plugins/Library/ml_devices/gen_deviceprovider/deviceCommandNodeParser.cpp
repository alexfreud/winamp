#include "main.h"
#include "./DeviceCommandNodeParser.h"
#include "../../xml/obj_xml.h"

DeviceCommandNodeParser::DeviceCommandNodeParser()
	: reader(NULL), test(NULL)
{
}

DeviceCommandNodeParser::~DeviceCommandNodeParser()
{
	End();
}


BOOL DeviceCommandNodeParser::Begin(obj_xml *xmlReader, TestSuite *testSuite)
{
	if (NULL != reader || NULL != test)
		return FALSE;

	if (NULL == xmlReader || NULL == testSuite) 
		return FALSE;

	reader = xmlReader;
	reader->AddRef();
		
	test = testSuite;

	reader->xmlreader_registerCallback(L"testprovider\fcommands\fcommand", this);
	
	return TRUE;
}

void DeviceCommandNodeParser::End()
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


void DeviceCommandNodeParser::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	elementParser.Begin(reader, params);
}

void DeviceCommandNodeParser::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	ifc_devicecommand *result;
	if (FALSE != elementParser.End(reader, &result))
	{
		if (NULL != test)
			test->AddCommand(result);
		
		result->Release();
	}
}

void DeviceCommandNodeParser::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
}

#define CBCLASS DeviceCommandNodeParser
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS