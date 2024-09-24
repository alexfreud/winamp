#ifndef _LOADLIB_H
#define _LOADLIB_H

#include <bfc/wasabi_std.h>
#include <bfc/named.h>

class Library : public NamedW
{
public:
  Library(const wchar_t *filename=NULL);
  Library(const Library &l) {
    lib = NULL; // FG> overrides default constructor, so we need to init this too...
    load(l.getName());
  }
  ~Library();

  Library& operator =(const Library &l) {
    if (this != &l) {
      unload();
      load(l.getName());
    }
    return *this;
  }

  int load(const wchar_t *filename=NULL);
  void unload();

  void *getProcAddress(const char *procname);
  OSMODULEHANDLE getHandle() const { return lib; }

private:
  OSMODULEHANDLE lib;
};

#endif
