#ifndef _PLAYSTRING_H
#define _PLAYSTRING_H

#include <bfc/common.h>
#include <bfc/string/StringW.h>

class Playstring 
{
public:
  Playstring(const wchar_t *val=NULL);
  Playstring(const Playstring &ps);
  ~Playstring();

  const wchar_t *getValue() const { return val; }
  operator const wchar_t *() const { return getValue(); }

  void setValue(const wchar_t *newval);

  // copy
  Playstring& operator =(const Playstring &ps);

protected:
  void _setValue(const wchar_t *newval, int tablenum);

private:
  const wchar_t *val;
};

class PlaystringComparator {
public:
  // comparator for sorting
  static int compareItem(Playstring *p1, Playstring* p2) {
    return wcscmp(p1->getValue(), p2->getValue());
  }
};

template <int tablenum=0>
class PlaystringT : public Playstring {
public:
  PlaystringT(const wchar_t *newval) {
    val = NULL; setValue(newval);
  }
  PlaystringT(const Playstring &ps) {
    val = NULL; setValue(ps.getValue());
  }
  void setValue(const wchar_t *newval) { _setValue(newval, tablenum); }
};

#endif
