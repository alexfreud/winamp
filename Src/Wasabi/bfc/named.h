#ifndef _NAMED_H
#define _NAMED_H

#include <bfc/common.h>
#include <bfc/string/StringW.h>

class NOVTABLE NamedW
{
public:
  NamedW(const wchar_t *initial_name=NULL) : name(initial_name) {}
  virtual ~NamedW() {}	// placeholder to ensure name is destructed properly
  const wchar_t *getName() const { return name; }
  const wchar_t *getNameSafe(const wchar_t *defval=NULL) const {
    const wchar_t *str = name;
    return str ? str : (defval ? defval : L"(null)");
  }
  void setName(const wchar_t *newname) { if (name.isequal(newname)) return; name = newname; onSetName(); }
  // override this to catch name settings
  virtual void onSetName() {}

private:
  StringW name;
};

#endif
