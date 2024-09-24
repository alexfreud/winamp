#ifndef _FOREACH_H
#define _FOREACH_H

#include "memblock.h"
#include "ptrlist.h"


// foreach stuff
/* use like this:
  PtrList<blah> list;
  foreach(list)
    list.getfor()->booga();
    something(list.getfor());
  endfor
*/

// foreach stuff
class __foreach 
{
public:
  __foreach(const PtrListRoot &ptrlist, int reverse=FALSE);
  __foreach(const PtrListRoot *ptrlist, int reverse=FALSE);

  int done() const;
  void *next(int advance = TRUE);
  void *getPtr() const;

  int getPos() const;

private:
  MemBlock<void *> list;
  int pos;
};

#define foreach(x) \
{ \
  void *__fe_void; \
  __foreach ___f(x); \
  for (__fe_void = ___f.getPtr(); !___f.done(); __fe_void = ___f.next()) {
#define getfor() castFor(__fe_void)
#define endfor \
  } \
}
#define foreach_reverse(x) \
{ \
  void *__fe_void; \
  __foreach ___f(x, TRUE); \
  for (__fe_void = ___f.getPtr(); !___f.done(); __fe_void = ___f.next()) {
#define getfor() castFor(__fe_void)
#define endfor \
  } \
}
#define foreach_index (___f.getPos())

#endif
