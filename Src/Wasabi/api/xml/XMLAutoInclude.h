#ifndef NULLSOFT_WASABI_XMLAUTOINCLUDE_H
#define NULLSOFT_WASABI_XMLAUTOINCLUDE_H

#include "../xml/obj_xml.h"
#include "../xml/ifc_xmlreadercallbackI.h"
#include <bfc/string/StringW.h>

class XMLAutoInclude : public ifc_xmlreadercallbackI
{
public:
  XMLAutoInclude(obj_xml *_parser, const wchar_t *_path);
  ~XMLAutoInclude();
  void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
  void xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag);
//private:
  obj_xml *parser;
  StringW path, includeFn;
private:
	void Include(const wchar_t *filename);
};

#endif