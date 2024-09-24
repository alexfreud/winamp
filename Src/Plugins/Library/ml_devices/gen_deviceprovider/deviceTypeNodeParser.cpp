#include "main.h"
#include "./DeviceTypeNodeParser.h"
#include "../../xml/obj_xml.h"

DeviceTypeNodeParser::DeviceTypeNodeParser()
	: reader(NULL), test(NULL)
{
}

DeviceTypeNodeParser::~DeviceTypeNodeParser()
{
	End();
}


BOOL DeviceTypeNodeParser::Begin(obj_xml *xmlReader, TestSuite *testSuite)
{
	if (NULL != reader || NULL != test)
		return FALSE;

	if (NULL == xmlReader || NULL == testSuite) 
		return FALSE;

	reader = xmlReader;
	reader->AddRef();
		
	test = testSuite;

	reader->xmlreader_registerCallback(L"testprovider\ftypes\ftype", this);
	
	return TRUE;
}

void DeviceTypeNodeParser::End()
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


void DeviceTypeNodeParser::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	elementParser.Begin(reader, params);
}

void DeviceTypeNodeParser::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	ifc_devicetype *result;
	if (FALSE != elementParser.End(reader, &result))
	{
		if (NULL != test)
			test->AddType(result);
		
		result->Release();
	}
}

void DeviceTypeNodeParser::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
}

#define CBCLASS DeviceTypeNodeParser
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS