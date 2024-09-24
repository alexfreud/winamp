//PORTABLE
#ifndef _STACK_H
#define _STACK_H

#include <bfc/common.h>
#include <bfc/wasabi_std.h>

// a self-growing stack. note that it never shrinks (for now)

class StackBase {
protected:
  StackBase();
  ~StackBase();

  int push(void *item, int sizeofT, int increment);
  int peek();
  int peekAt(void *ptr, int n, int sizeofT);
  int getRef(void **ptr, int n, int sizeofT);
  void *top(int sizeofT);
  int pop(void *ptr, int sizeofT);
  int isempty();
  void purge();

private:
  int nslots, cur;
  char *stack;
};

template<class T>
class Stack : public StackBase {
public:
  int push(T item, int increment=-1) { return StackBase::push(&item, sizeof(T), increment); }
  using StackBase::peek;
  T top() { return *static_cast<T*>(StackBase::top(sizeof(T))); }
  int peekAt(T *ptr = NULL, int n = 0) { return StackBase::peekAt(ptr, n, sizeof(T)); }
  int getRef(T **ptr = NULL, int n = 0) { if (!ptr) return peek(); return StackBase::getRef((void **)&(*ptr), n, sizeof(T)); }
  int pop(T *ptr = NULL) { return StackBase::pop(ptr, sizeof(T)); }
  using StackBase::isempty;
  using StackBase::purge;
};

#endif
