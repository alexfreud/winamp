#include "foreach.h"

__foreach::__foreach(const PtrListRoot &ptrlist, int reverse) :
  list(ptrlist.getNumItems(), ptrlist.getItemList()), pos(0)
{
  if (reverse) list.reverse();
}

__foreach::__foreach(const PtrListRoot *ptrlist, int reverse) :
  list(ptrlist->getNumItems(), ptrlist->getItemList()), pos(0) {
  if (reverse) list.reverse();
}

int __foreach::done() const { return (pos >= list.getSize()); }

void *__foreach::next(int advance) { if (advance) pos++; return getPtr(); }

void *__foreach::getPtr() const { return (pos < list.getSize()) ? list[pos] : NULL; }

int __foreach::getPos() const { return pos; }


