#include "./xmlInt32Parser.h"

#include <shlwapi.h>
#include <strsafe.h>


XmlInt32Parser::XmlInt32Parser()
	: value(0), result(E_PENDING)
{
	memset(szBuffer, 0, sizeof(szBuffer));
}

XmlInt32Parser::~XmlInt32Parser()
{

}

HRESULT XmlInt32Parser::GetValue(INT *pValue)
{
	if (NULL == pValue) return E_POINTER;
	*pValue = value;
	return result;
}

void XmlInt32Parser::Event_XmlStartElement(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	szBuffer[0] = L'\0';
	result = S_FALSE;
}

void XmlInt32Parser::Event_XmlEndElement(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	if (SUCCEEDED(result))
	{
		if (FALSE == StrToIntEx(szBuffer, STIF_SUPPORT_HEX, &value))
			result = E_FAIL;
		else
			result = S_OK;
	}
	szBuffer[0] = L'\0';
}

void XmlInt32Parser::Event_XmlCharData(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *value)
{
	if (SUCCEEDED(result))
	{
		if (FAILED(StringCchCat(szBuffer, ARRAYSIZE(szBuffer), value)))
			result = E_FAIL;
	}
}

void XmlInt32Parser::Event_XmlError(int linenum, int errcode, const wchar_t *errstr)
{	
	szBuffer[0] = L'\0';
	result = E_FAIL;
}

#define CBCLASS XmlInt32Parser
START_DISPATCH;
VCB(ONSTARTELEMENT, Event_XmlStartElement)
VCB(ONENDELEMENT, Event_XmlEndElement)
VCB(ONCHARDATA, Event_XmlCharData)
VCB(ONERROR, Event_XmlError)
END_DISPATCH;
#undef CBCLASS