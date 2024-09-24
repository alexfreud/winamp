#include "./commandNodeParser.h"
#include "./loginCommand.h"
#include "./loginProvider.h"

#include "../../xml/obj_xml.h"

LoginCommandNodeParser::LoginCommandNodeParser()
	: reader(NULL), provider(NULL)
{
}

LoginCommandNodeParser::~LoginCommandNodeParser()
{
	End();
}


HRESULT LoginCommandNodeParser::Begin(obj_xml *pReader, LoginProvider *pProvider)
{
	if (NULL != reader || NULL != provider)
		return E_PENDING;

	if (NULL == pReader || NULL == pProvider) 
		return E_INVALIDARG;

	reader = pReader;
	reader->AddRef();
		
	provider = pProvider;
	provider->AddRef();

	reader->xmlreader_registerCallback(L"loginProviders\fprovider\fcommand", this);
	
	return S_OK;
}

HRESULT LoginCommandNodeParser::End()
{	
	if (NULL != reader)
	{
		reader->xmlreader_unregisterCallback(this);
		reader->Release();
		reader = NULL;
	}

	if (NULL != provider)
	{
		provider->Release();
		provider = NULL;
	}
	return S_OK;
}


void LoginCommandNodeParser::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	elementParser.Begin(reader, params);
}

void LoginCommandNodeParser::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	LoginCommand *result;
	if (SUCCEEDED(elementParser.End(reader, &result)))
	{
		if (NULL != provider)
			provider->SetCommand(result);
		
		result->Release();
	}
}

void LoginCommandNodeParser::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
}

#define CBCLASS LoginCommandNodeParser
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS