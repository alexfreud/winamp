#include "./commandParser.h"
#include "./loginCommand.h"
#include "./commandWinampAuth.h"
#include "./commandWebAuth.h"

#include "./common.h"

#include "../../xml/obj_xml.h"


LoginCommandParser::LoginCommandParser()
	: object(NULL)
{
}

LoginCommandParser::~LoginCommandParser()
{
	if (NULL != object)
		object->Release();
	
}


HRESULT LoginCommandParser::Begin(obj_xml *reader, ifc_xmlreaderparams *params)
{
	if (NULL != object)
		return E_PENDING;

	if (NULL == reader || NULL == params) 
		return E_INVALIDARG;
		
	GUID commandId;
	LPCWSTR pszId = params->getItemValue(L"id");
	if (NULL == pszId || RPC_S_OK != UuidFromString((RPC_WSTR)pszId, &commandId))
		return E_INVALIDARG;

	HRESULT hr;
	if (IsEqualGUID(LCUID_WINAMPAUTH, commandId))
		hr = LoginCommandWinampAuth::CreateInstance((LoginCommandWinampAuth**)&object);
	else if (IsEqualGUID(LCUID_WEBAUTH, commandId))
		hr = LoginCommandWebAuth::CreateInstance((LoginCommandWebAuth**)&object);
	else
		hr = E_INVALIDARG;
	
	if (SUCCEEDED(hr)) 
		reader->xmlreader_registerCallback(L"loginProviders\fprovider\fcommand\f*", this);
	
	return hr;
}

HRESULT LoginCommandParser::End(obj_xml *reader, LoginCommand **instance)
{
	if (NULL == object)
		return E_UNEXPECTED;

	HRESULT hr;
	
	if (SUCCEEDED(object->IsValid()))
	{
		if (NULL != instance)
		{
			*instance = object;
			object->AddRef();
		}
		hr = S_OK;
	}
	else
		hr = E_FAIL;

	object->Release();
	object = NULL;

	if (NULL != reader)
		reader->xmlreader_unregisterCallback(this);

	return hr;
}


void LoginCommandParser::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	elementString.Clear();
}

void LoginCommandParser::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	if (NULL != object)
		object->SetParameter(xmltag, elementString.Get());
}

void LoginCommandParser::Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value)
{
	elementString.Append(value);
}

void LoginCommandParser::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
	elementString.Clear();
}

#define CBCLASS LoginCommandParser
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONCHARDATA, Event_XmlCharData)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS