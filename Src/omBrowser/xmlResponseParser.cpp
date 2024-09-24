#include "main.h"
#include "./xmlResponseParser.h"
#include "./ifc_omservicehost.h"
#include "./ifc_omservice.h"
#include "./ifc_omstoragehandlerenum.h"
#include "./ifc_omstorageext.h"
#include "./ifc_omfilestorage.h"


#include "../xml/obj_xml.h"

#include <shlwapi.h>
#include <strsafe.h>


XmlResponseParser::XmlResponseParser()
	: reader(NULL), code((UINT)-1), text(NULL), host(NULL)
{
}

XmlResponseParser::~XmlResponseParser()
{
	if (NULL != reader)
	{
		reader->Release();
		reader = NULL;
	}

	if (NULL != host)
		host->Release();

	while(false == deque.empty())
	{
		deque.front()->Release();
		deque.pop_front();
	}
}

HRESULT XmlResponseParser::Initialize(obj_xml *xml, ifc_omservicehost *serviceHost)
{
	if (NULL == xml) return E_INVALIDARG;
	
	if (NULL != reader)
	{
		if (reader == xml)
			return S_FALSE;

		reader->Release();
	}
	
	reader = xml;
	reader->AddRef();
	
	reader->xmlreader_registerCallback(L"response\fstatusCode", this);
	reader->xmlreader_registerCallback(L"response\fstatusText", this);
	reader->xmlreader_registerCallback(L"response\fdata\fservices\fservice", this);

	if (NULL != host)
		host->Release();

	ifc_omstoragehandlerenum *handlerEnum(NULL);

	host = serviceHost;
	if (NULL != host)
	{
		host->AddRef();

		ifc_omstorageext *storageExt;
		if (SUCCEEDED(host->QueryInterface(IFC_OmStorageExt, (void**)&storageExt)))
		{
			if (FAILED(storageExt->Enumerate(&SUID_OmStorageXml, &handlerEnum)))
				handlerEnum = NULL;
			storageExt->Release();
		}
	}

	parser.RegisterHandlers(handlerEnum);
	if (NULL != handlerEnum)
	{
		handlerEnum->Release();
	}

	Reset();

	return S_OK;
}
HRESULT XmlResponseParser::Finish()
{
	if (NULL != reader)
	{
		reader->Release();
		reader = NULL;
	}
	return S_OK;
}
HRESULT XmlResponseParser::Reset()
{
	code = (UINT)-1;

	if (NULL != text)
	{
		Plugin_FreeString(text);
		text = NULL;
	}

	string.Clear();

	return S_OK;
}

HRESULT XmlResponseParser::GetCode(UINT *value)
{
	if (NULL ==  value) return E_POINTER;
	if ((UINT)-1 == code) 
	{
		*value = 0;
		return E_PENDING;
	}

	*value = code;
	return S_OK;
}

HRESULT XmlResponseParser::GetText(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;
	if (NULL == text) 
	{
		pszBuffer[0] = L'\0';
		return E_PENDING;
	}
	
	return StringCchCopy(pszBuffer, cchBufferMax, text);
}

HRESULT XmlResponseParser::PeekService(ifc_omservice **service)
{
	if (NULL == service) return E_POINTER;
	if (0 == deque.size()) return S_FALSE;

	*service = deque.back();
    deque.pop_back();
	return S_OK;
}

void XmlResponseParser::OnStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, xmltag, -1, L"service", -1))
	{
		parser.Initialize(reader, xmlpath, host);
	}
	else
	{
		string.Clear();
	}
}

void XmlResponseParser::OnEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	if (S_OK == parser.GetActive())
	{
		ifc_omservice *service;
		if (SUCCEEDED(parser.Finish(NULL, &service)) && NULL != service)
		{
			deque.push_front(service);
		}
	}
	else
	{
		if ((UINT)-1 == code && CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, xmltag, -1, L"statusCode", -1))
		{
			LPCWSTR sCode = string.Get();
			if (NULL == sCode || L'\0' == *sCode ||
				FALSE == StrToIntEx(sCode, STIF_SUPPORT_HEX, (INT*)&code))
			{
				code = (UINT)-1;
			}
		}
		else if (NULL == text && CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, xmltag, -1, L"statusText", -1))
		{
			text = Plugin_CopyString(string.Get());
		}


	}
}

void XmlResponseParser::OnCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value)
{
	if (S_OK != parser.GetActive())
	{
		string.Append(value);
	}
}

void XmlResponseParser::OnError(int linenum, int errcode, const wchar_t *errstr)
{
	string.Clear();
}

#define CBCLASS XmlResponseParser
START_DISPATCH;
VCB(ONSTARTELEMENT, OnStartElement)
VCB(ONENDELEMENT, OnEndElement)
VCB(ONCHARDATA, OnCharData)
END_DISPATCH;
#undef CBCLASS