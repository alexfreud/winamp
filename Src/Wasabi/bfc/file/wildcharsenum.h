#ifndef __WILDCHARSENUM_H
#define __WILDCHARSENUM_H

#include <bfc/ptrlist.h>
#include <bfc/string/StringW.h>

class find_entry {
public:
  find_entry(const wchar_t *_path, const wchar_t *_filename) : path(_path), filename(_filename) {}
  ~find_entry() {}
  StringW path;
  StringW filename;
};

class WildcharsEnumerator 
{
public:  
  WildcharsEnumerator(const wchar_t *_selection);
  virtual ~WildcharsEnumerator();

  int getNumFiles();
  const wchar_t *enumFile(int n);
  void rescan();

  static int isWildchars(const wchar_t *filename);

private:
  StringW selection;
  PtrList <find_entry> finddatalist;
  StringW singfiledup;
  StringW enumFileString;
};

#endif
