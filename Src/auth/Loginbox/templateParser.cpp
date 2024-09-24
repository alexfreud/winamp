#include "./templateParser.h"
#include "./loginTemplate.h"
#include "./templateCredentials.h"
#include "./templateInfo.h"
#include "./templateAddress.h"

#include "./common.h"

#include "../../xml/obj_xml.h"


LoginTemplateParser::LoginTemplateParser()
	: object(NULL)
{
}

LoginTemplateParser::~LoginTemplateParser()
{
	if (NULL != object)
		object->Release();
	
}


HRESULT LoginTemplateParser::Begin(obj_xml *reader, ifc_xmlreaderparams *params)
{
	if (NULL != object)
		return E_PENDING;

	if (NULL == reader || NULL == params) 
		return E_INVALIDARG;
		
	GUID templateId;
	LPCWSTR pszId = params->getItemValue(L"id");
	if (NULL == pszId || RPC_S_OK != UuidFromString((RPC_WSTR)pszId, &templateId))
		return E_INVALIDARG;

	HRESULT hr;
	if (IsEqualGUID(LTUID_CREDENTIALS, templateId))
		hr = LoginTemplateCredentials::CreateInstance((LoginTemplateCredentials**)&object);
	else if (IsEqualGUID(LTUID_INFO, templateId))
		hr = LoginTemplateInfo::CreateInstance((LoginTemplateInfo**)&object);
	else if (IsEqualGUID(LTUID_ADDRESS, templateId))
		hr = LoginTemplateAddress::CreateInstance((LoginTemplateAddress**)&object);
	else
		hr = E_INVALIDARG;
	
	if (SUCCEEDED(hr)) 
		reader->xmlreader_registerCallback(L"loginProviders\fprovider\ftemplate\f*", this);
	
	return hr;
}

HRESULT LoginTemplateParser::End(obj_xml *reader, LoginTemplate **instance)
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


void LoginTemplateParser::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	elementString.Clear();
}

void LoginTemplateParser::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	if (NULL != object)
		object->SetParameter(xmltag, elementString.Get());
}

void LoginTemplateParser::Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value)
{
	elementString.Append(value);
}

void LoginTemplateParser::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
	elementString.Clear();
}

#define CBCLASS LoginTemplateParser
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONCHARDATA, Event_XmlCharData)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS