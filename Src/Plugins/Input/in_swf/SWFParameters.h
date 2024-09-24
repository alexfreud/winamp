#pragma once
#include "../xml/ifc_xmlreadercallback.h"
#include "XMLString.h"
#include <vector>
#include <crtdefs.h>

class obj_xml;

struct SWFArgument
{
	wchar_t *type;
	wchar_t *value;
};

class SWFParameters : public ifc_xmlreadercallback
{
public:
	typedef std::vector<SWFArgument*> ArgumentList;

	SWFParameters(obj_xml *_parser);
	~SWFParameters();

	wchar_t *functionName;

	bool GetUnsigned(size_t index, unsigned int *value);
	bool GetDouble(size_t index, double *value);
private:
	void StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void EndTag(const wchar_t *xmlpath, const wchar_t *xmltag);
	XMLString currentParameter;
	ArgumentList arguments;
	
	_locale_t C_locale;
	obj_xml *parser;
protected:
	RECVS_DISPATCH;
};