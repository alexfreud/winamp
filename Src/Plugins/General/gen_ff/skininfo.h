#ifndef _SKININFO_H
#define _SKININFO_H

#include "../xml/obj_xml.h"
#include "../xml/ifc_xmlreadercallbacki.h"
#include <bfc/string/StringW.h>

class SkinInfoBlock {
public:
  SkinInfoBlock(const wchar_t *name) : name(name),fullname(name) { }
  const wchar_t *getName() { return name; }
  const wchar_t *getParentSkin() { return parentskin; }
  const wchar_t *getFullName() { return fullname; }
  const wchar_t *getVersion() { return version; }
  const wchar_t *getComment() { return comment; }
  const wchar_t *getAuthor() { return author; }
  const wchar_t *getEmail() { return email; }
  const wchar_t *getHomepage() { return homepage; }
  const wchar_t *getScreenshot() { return screenshot; }
protected:
  StringW name;
  StringW walversion;
  StringW parentskin;
  
  StringW fullname;
  StringW version;
  StringW comment;
  StringW author;
  StringW email;
  StringW homepage;
  StringW screenshot;
};

class SkinInfosXmlReader : public SkinInfoBlock, public ifc_xmlreadercallbackI
{
public:
  SkinInfosXmlReader(const wchar_t *skinname);
  void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
  void xmlReaderOnCharacterDataCallback(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *s);
};

#endif
