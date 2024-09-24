#include "main.h"
#include "./xmlServiceParser.h"

#include "../xml/obj_xml.h"

#include "./service.h"
#include "./ifc_omservicehost.h"
#include "./ifc_omstoragehandler.h"
#include "./ifc_omstoragehandlerenum.h"
#include "./ifc_omstorageext.h"

#include <shlwapi.h>
#include <strsafe.h>

typedef void (CALLBACK *TAGCALLBACK)(XmlServiceParser* /*reader*/, OmService* /*editor*/, LPCWSTR /*value*/);

typedef struct __TAGRECORD
{
	LPCWSTR		name;
	TAGCALLBACK callback;
} TAGRECORD;

static void CALLBACK ParserXml_ReadId(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	INT iVal;
	if (StrToIntEx(value, STIF_SUPPORT_HEX, &iVal))
	{
		service->SetId(iVal);
	}
}

static void CALLBACK ParserXml_ReadName(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	service->SetName(value, FALSE);
}

static void CALLBACK ParserXml_ReadUrl(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	service->SetUrl(value, FALSE);
}

static void CALLBACK ParserXml_ReadAuth(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	INT iVal;
	if (StrToIntEx(value, STIF_SUPPORT_HEX, &iVal))
	{
	//	service->SetAuth(iVal);
	}
}

static void CALLBACK ParserXml_ReadIcon(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	service->SetIcon(value, FALSE);
}

static void CALLBACK ParserXml_ReadThumbnail(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	service->SetThumbnail(value, FALSE);
}

static void CALLBACK ParserXml_ReadScreenshot(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	service->SetScreenshot(value, FALSE);
}

static void CALLBACK ParserXml_ReadVersion(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	INT iVal;
	if (StrToIntEx(value, STIF_SUPPORT_HEX, &iVal))
		service->SetVersion(iVal);
}

static void CALLBACK ParserXml_ReadGeneration(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	INT iVal;
	if (StrToIntEx(value, STIF_SUPPORT_HEX, &iVal))
	{
		service->SetGeneration(iVal);
	}
}

static void CALLBACK ParserXml_ReadDescription(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	service->SetDescription(value, FALSE);
}

static void CALLBACK ParserXml_ReadAuthorFirst(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	service->SetAuthorFirst(value, FALSE);
}

static void CALLBACK ParserXml_ReadAuthorLast(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	service->SetAuthorLast(value, FALSE);
}

static void CALLBACK ParserXml_ReadPublished(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	service->SetPublished(value, FALSE);
}

static void CALLBACK ParserXml_ReadUpdated(XmlServiceParser *reader, OmService *service, LPCWSTR value)
{
	service->SetUpdated(value, FALSE);
}

static const TAGRECORD szTagTable[] = 
{
	{ L"id", ParserXml_ReadId },
	{ L"name", ParserXml_ReadName },
	{ L"version", ParserXml_ReadVersion },
	{ L"icon_url", ParserXml_ReadIcon },
	{ L"entry_url", ParserXml_ReadUrl },
	{ L"generation", ParserXml_ReadGeneration },
	{ L"author_comments_long", ParserXml_ReadDescription },
	{ L"author_firstname", ParserXml_ReadAuthorFirst},
	{ L"author_lastname", ParserXml_ReadAuthorLast },
	{ L"date_published", ParserXml_ReadPublished },
	{ L"date_updated", ParserXml_ReadUpdated },
	{ L"thumbnail", ParserXml_ReadThumbnail },
	{ L"screenshot", ParserXml_ReadScreenshot },
};

XmlServiceParser::XmlServiceParser() 
	:  checkList(NULL), checkSize(0), parser(NULL), service(NULL), result(E_UNEXPECTED)
{
}

XmlServiceParser::~XmlServiceParser()
{
	Finish(NULL, NULL);

	if (NULL != checkList)
		free(checkList);
	size_t index = handlerList.size();
	while (index--)
		handlerList[index]->Release();
}

HRESULT XmlServiceParser::RegisterHandlers(ifc_omstoragehandlerenum *handlerEnum)
{
	if (E_PENDING == result)
		return E_PENDING;

	if (NULL != checkList)
	{
		free(checkList);
		checkList = NULL;
		checkSize = 0;
	}

	size_t index = handlerList.size();
	while(index--) handlerList[index]->Release();
	handlerList.clear();

	if (NULL != handlerEnum)
	{
		ifc_omstoragehandler *handler;
		handlerEnum->Reset();
		while(S_OK == handlerEnum->Next(1, &handler, NULL))
		{			
			handlerList.push_back(handler);
		}
	}

	return S_OK;
}

HRESULT XmlServiceParser::Initialize(obj_xml *reader, LPCWSTR match, ifc_omservicehost *host)
{
	result = E_UNEXPECTED;

	if (NULL != parser) return E_UNEXPECTED;
	if (NULL == reader) return E_INVALIDARG;

	HRESULT hr;
	WCHAR szBuffer[1024] = {0};
	LPWSTR cursor = szBuffer;
	size_t remaining = ARRAYSIZE(szBuffer);

	if (NULL == checkList)
	{
		size_t checkCount = ARRAYSIZE(szTagTable) + handlerList.size();
		checkSize = checkCount/8 + ((checkCount%8) ? 1 : 0);

		checkList = (BYTE*)calloc(checkSize, sizeof(BYTE));
		if (NULL != checkList)
			ZeroMemory(checkList, checkSize);
	}

	hr = StringCchCopyEx(cursor, remaining, match, &cursor, &remaining, STRSAFE_IGNORE_NULLS);
	if (FAILED(hr)) return E_UNEXPECTED;

	hr = StringCchCopyEx(cursor, remaining, ((cursor != szBuffer) ? L"\f*" : L"*"), &cursor, &remaining, STRSAFE_IGNORE_NULLS);
	if (FAILED(hr)) return E_UNEXPECTED;

	parser = reader;
	parser->AddRef();
	parser->xmlreader_registerCallback(szBuffer, this);

	hr = OmService::CreateInstance(0, host, &service);
	if (FAILED(hr)) return hr;

	service->BeginUpdate();

	result = E_PENDING;
	return S_OK;
}

HRESULT XmlServiceParser::Finish(HRESULT *parserResult, ifc_omservice **ppService)
{
	if (NULL == parser)
		return E_UNEXPECTED;

	if (E_PENDING == result)
		result= S_OK;

	buffer.Clear();

	if (NULL != checkList)
		ZeroMemory(checkList, checkSize);

	parser->xmlreader_unregisterCallback(this);
	parser->Release();
	parser = NULL;

	if (NULL != service && 0 == service->GetId())
	{
		if (SUCCEEDED(result))
			result = E_UNEXPECTED;
		
		service->Release();
		service = NULL;
	}

	if (NULL != parserResult)
		*parserResult = result;

	if (NULL != ppService)
	{
		if (SUCCEEDED(result))
		{
			*ppService = service;
			service->AddRef();
		}
		else
		{
			*ppService = NULL;
		}
	}

	if (NULL != service)
	{
		service->SetModified(0, (UINT)-1);
		service->EndUpdate();

		service->Release();
		service = NULL;
	}

	result = E_UNEXPECTED;
	return S_OK;
}

HRESULT XmlServiceParser::GetActive()
{
	return (E_PENDING == result) ? S_OK : S_FALSE;
}

void XmlServiceParser::OnStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	buffer.Clear();
}

void XmlServiceParser::OnEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{	
	size_t i;
	for (i = 0; i < ARRAYSIZE(szTagTable); i++)
	{
		if (NULL == checkList || 0 == (checkList[i/8] & (0x01 << (i%8))) && E_PENDING == result)
		{
			if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, szTagTable[i].name, -1, xmltag, -1))
			{
				checkList[i/8] |= (0x01 << (i%8));
				szTagTable[i].callback(this, service, buffer.Get());
				return;
			}
		}
	}

	size_t handlerSize = handlerList.size();
	size_t iCheck = ARRAYSIZE(szTagTable);
	for (i = 0; i < handlerSize; i++, iCheck++)
	{
		if (NULL == checkList || 0 == (checkList[iCheck/8] & (0x01 << (iCheck%8))) && E_PENDING == result)
		{
			LPCWSTR pszKey;
			if (SUCCEEDED(handlerList[i]->GetKey(&pszKey)) &&
				CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, pszKey, -1, xmltag, -1))
			{
				checkList[iCheck/8] |= (0x01 << (iCheck%8));
				handlerList[i]->Invoke(service, xmltag, buffer.Get());
				return;
			}
		}
	}
}

void XmlServiceParser::OnCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value)
{
	buffer.Append(value);
}

void XmlServiceParser::OnError(int linenum, int errcode, const wchar_t *errstr)
{
	result = E_FAIL;
	if (NULL != service)
	{
		service->Release();
		service = NULL;
	}
}

#define CBCLASS XmlServiceParser
START_DISPATCH;
VCB(ONSTARTELEMENT, OnStartElement)
VCB(ONENDELEMENT, OnEndElement)
VCB(ONCHARDATA, OnCharData)
VCB(ONERROR, OnError)
END_DISPATCH;
#undef CBCLASS