/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/

#include "XMLString.h"
#include "../nu/strsafe.h"

XMLString::XMLString() 
{
	data[0]=0;
}

void XMLString::Reset()
{
	data[0]=0;
}

const wchar_t *XMLString::GetString()
{
	return data;
}

void XMLString::StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	data[0]=0;
}


void XMLString::TextHandler(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str)
{
	StringCchCatW(data, XMLSTRING_SIZE, str);
}


void XMLString::ManualSet(const wchar_t *string)
{
StringCchCatW(data, XMLSTRING_SIZE, string);
}

uint32_t XMLString::GetUInt32()
{
	return wcstoul(data, 0, 10);
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS XMLString
START_DISPATCH;
VCB(ONSTARTELEMENT, StartTag)
VCB(ONCHARDATA, TextHandler)
END_DISPATCH;
