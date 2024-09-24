#include "./providerParser.h"
#include "./loginProvider.h"
#include "./loginTemplate.h"
#include "./loginCommand.h"
#include "./common.h"

//#include "../api.h"
#include "../../xml/obj_xml.h"


typedef void (CALLBACK *PROVIDERTAGCALLBACK)(LoginProvider* /*provider*/, LPCWSTR /*value*/);

typedef struct __PROVIDERTAG
{
	LPCWSTR name;
	PROVIDERTAGCALLBACK callback;
} PROVIDERTAG;

static void CALLBACK ProviderTag_Name(LoginProvider *provider, LPCWSTR value)
{
	provider->SetName(value);
}

static void CALLBACK ProviderTag_ImagePath(LoginProvider *provider, LPCWSTR value)
{
	provider->SetImagePath(value);
}

static void CALLBACK ProviderTag_Description(LoginProvider *provider, LPCWSTR value)
{
	provider->SetDescription(value);
}

static void CALLBACK ProviderTag_TosLink(LoginProvider *provider, LPCWSTR value)
{
	provider->SetTosLink(value);
}

static void CALLBACK ProviderTag_PrivacyLink(LoginProvider *provider, LPCWSTR value)
{
	provider->SetPrivacyLink(value);
}

static void CALLBACK ProviderTag_HelpLink(LoginProvider *provider, LPCWSTR value)
{
	provider->SetHelpLink(value);
}

static const PROVIDERTAG szProviderTags[PROVIDER_TAG_MAX] = 
{
	{L"name", ProviderTag_Name},
	{L"icon", ProviderTag_ImagePath},
	{L"description", ProviderTag_Description},
	{L"tos", ProviderTag_TosLink},
	{L"privacy", ProviderTag_PrivacyLink},
	{L"help", ProviderTag_HelpLink},
};

LoginProviderParser::LoginProviderParser()
	: reader(NULL), provider(NULL)
{
	ZeroMemory(hitList, sizeof(hitList));
}

LoginProviderParser::~LoginProviderParser()
{
	if (NULL != reader) 
		reader->Release();
	
	if (NULL != provider)
		provider->Release();
}


HRESULT LoginProviderParser::SetReader(obj_xml *pReader)
{
	if (NULL != reader)
	{
		reader->Release();
	}

	reader = pReader;
	if (NULL != reader)
		reader->AddRef();

	return S_OK;
}

HRESULT LoginProviderParser::Begin(ifc_xmlreaderparams *params)
{
	if (NULL != provider)
		return E_PENDING;

	if (NULL == reader) return E_UNEXPECTED;
	if (NULL == params) return E_INVALIDARG;
		
	GUID providerId;
	LPCWSTR pszId = params->getItemValue(L"id");
	if (NULL != pszId && RPC_S_OK == UuidFromString((RPC_WSTR)pszId, &providerId))
	{
		if (FAILED(LoginProvider::CreateInstance(&providerId, &provider)))
			provider = NULL;
	}

	if (NULL == provider) 
		return E_FAIL;

	reader->xmlreader_registerCallback(L"loginProviders\fprovider\fname", this);
	reader->xmlreader_registerCallback(L"loginProviders\fprovider\ficon", this);
	reader->xmlreader_registerCallback(L"loginProviders\fprovider\fdescription", this);
	reader->xmlreader_registerCallback(L"loginProviders\fprovider\ftos", this);
	reader->xmlreader_registerCallback(L"loginProviders\fprovider\fprivacy", this);
	reader->xmlreader_registerCallback(L"loginProviders\fprovider\fhelp", this);
	ZeroMemory(hitList, sizeof(hitList));

	templateNodeParser.Begin(reader, provider);
	commandNodeParser.Begin(reader, provider);

	return S_OK;
}

HRESULT LoginProviderParser::End(LoginProvider **ppProvider)
{	
	templateNodeParser.End();
	commandNodeParser.End();
	reader->xmlreader_unregisterCallback(this);

	
	
	if (NULL == provider || S_OK != provider->IsValid())
	{
		if (NULL != provider) 
		{
			provider->Release();
			provider = NULL;
		}

		if (NULL != ppProvider)
			*ppProvider = NULL;
		
		return E_FAIL;
	}
	
	if (NULL != ppProvider)
	{		
		*ppProvider = provider;
		if (NULL != provider)
			provider->AddRef();
	}

	provider->Release();
	provider = NULL;
	
	return S_OK;
}


void LoginProviderParser::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	elementString.Clear();
}

void LoginProviderParser::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	for (INT i = 0; i < PROVIDER_TAG_MAX; i++)
	{
		if (FALSE == hitList[i] && 
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, szProviderTags[i].name, -1, xmltag, -1))
		{
			szProviderTags[i].callback(provider, elementString.Get());
			hitList[i] = TRUE;
			break;
		}
	}
}

void LoginProviderParser::Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value)
{
	elementString.Append(value);
}

void LoginProviderParser::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
	elementString.Clear();
}

#define CBCLASS LoginProviderParser
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONCHARDATA, Event_XmlCharData)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS