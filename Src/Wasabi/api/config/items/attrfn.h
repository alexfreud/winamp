#ifndef _ATTRFN_H
#define _ATTRFN_H

#include "attrstr.h"

class _filename : public _string {
public:
	_filename(const wchar_t *name, const wchar_t *default_val=L"")
    : _string(name, default_val) { }

  virtual int getAttributeType() { return AttributeType::FILENAME; }
  virtual const wchar_t *getConfigGroup() { return L"studio.configgroup.filename"; }
};

#endif
