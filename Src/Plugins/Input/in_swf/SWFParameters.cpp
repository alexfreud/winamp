#include "SWFParameters.h"
#include "../xml/obj_xml.h"
#include <locale.h>

/*
example:
<invoke name="Benski" returntype="xml">
<arguments>
</arguments>
</invoke>
*/

SWFParameters::SWFParameters(obj_xml *_parser)
{
	parser = _parser;
	parser->xmlreader_setCaseSensitive();
	parser->xmlreader_registerCallback(L"invoke", this);
	parser->xmlreader_registerCallback(L"invoke\farguments\f*", this);
	parser->xmlreader_registerCallback(L"invoke\farguments\f*", &currentParameter);
	functionName=0;
	C_locale = _create_locale(LC_NUMERIC, "C");
}

SWFParameters::~SWFParameters()
{
	for (ArgumentList::iterator itr=arguments.begin();itr!=arguments.end();itr++)
	{
		SWFArgument *argument = *itr;
		free(argument->type);
		free(argument->value);
		free(argument);
	}
	arguments.clear();
	parser->xmlreader_unregisterCallback(this);
	parser->xmlreader_unregisterCallback(&currentParameter);
}

void SWFParameters::StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	if (!wcscmp(xmlpath, L"invoke"))
	{
		const wchar_t *name = params->getItemValue(L"name");
		if (name)
			functionName = _wcsdup(name);
	}
}

void SWFParameters::EndTag(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	if (!wcsncmp(xmlpath, L"invoke\farguments\f",  6 /*invoke*/+ 1/*\f*/ + 9/*arguments*/ + 1/*\f*/))
	{
		SWFArgument *argument = new SWFArgument;
		argument->type = _wcsdup(xmltag);
		const wchar_t *value = currentParameter.GetString();
		if (value)
			argument->value = _wcsdup(value);
		else
			argument->value = 0;
		arguments.push_back(argument);
	}
}

bool SWFParameters::GetUnsigned(size_t index, unsigned int *value)
{
	if (index < arguments.size())
	{
		SWFArgument *argument = arguments[index];
		if (argument && argument->type && !wcscmp(argument->type, L"number"))
		{
			const wchar_t *val = argument->value;
			if (val)
			{
				*value = wcstoul(val, 0, 10);
				return true;
			}
		}
	}
	return false;
}

bool SWFParameters::GetDouble(size_t index, double *value)
{
		if (index < arguments.size())
	{
		SWFArgument *argument = arguments[index];
		if (argument && argument->type && !wcscmp(argument->type, L"number"))
		{
			const wchar_t *val = argument->value;
			if (val)
			{
				*value = _wtof_l(val, C_locale);
				return true;
			}
		}
	}
	return false;
}

#define CBCLASS SWFParameters
START_DISPATCH;
VCB(ONSTARTELEMENT, StartTag)
VCB(ONENDELEMENT, EndTag)
END_DISPATCH;
#undef CBCLASS