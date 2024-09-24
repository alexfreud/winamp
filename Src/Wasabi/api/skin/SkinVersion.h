#ifndef NULLSOFT_WASABI_SKINVERSION_H
#define NULLSOFT_WASABI_SKINVERSION_H

#include "../xml/ifc_xmlreadercallbacki.h"
#include <bfc/string/StringW.h>

class ifc_xmlreaderparams;
class SkinVersionXmlReader : public ifc_xmlreadercallbackI
{
public:
  SkinVersionXmlReader(const wchar_t *skinname);
  void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
  const wchar_t *getWalVersion();
private:
  StringW walversion;
};

#endif