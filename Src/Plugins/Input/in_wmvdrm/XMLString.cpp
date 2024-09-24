#include "main.h"
#include "XMLString.h"
#include <strsafe.h>

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
	StringCchCatW(data, 256, str);
}


void XMLString::ManualSet(const wchar_t *string)
{
StringCchCatW(data, 256, string);
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS XMLString
START_DISPATCH;
VCB(ONSTARTELEMENT, StartTag)
VCB(ONCHARDATA, TextHandler)
END_DISPATCH;
