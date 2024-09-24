#ifndef _SKINFONT_H
#define _SKINFONT_H

#include <api/skin/xmlobject.h>

class SkinFont : public XmlObjectI 
{
public:
  SkinFont();
  ~SkinFont();
  void installFont(const wchar_t *filename, const wchar_t *path);
  virtual int setXmlOption(const wchar_t *name, const wchar_t *val);
private:
  StringW tempFn;
};

#endif
