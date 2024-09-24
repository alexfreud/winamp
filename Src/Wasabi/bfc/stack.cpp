#include "precomp_wasabi_bfc.h"
#include "stack.h"

//#define STACK_SIZE_INCREMENT 250

// going from 250 to 32 decreases size of VM working set by several megabytes
#define STACK_SIZE_INCREMENT 32

StackBase::StackBase() 
{
  nslots = 0;
  cur = 0;
  stack = NULL;
}

StackBase::~StackBase()
{
  if (stack != NULL) FREE(stack);
}

int StackBase::push(void *item, int sizeofT, int increment)
{
  if (increment <= 0) increment = STACK_SIZE_INCREMENT;
  if (stack == NULL) {
    nslots = increment;
    stack = (char*)MALLOC(sizeofT * nslots);
  } else if (cur >= nslots) {
    int newnslots = nslots + increment;
    stack = (char*)REALLOC(stack, sizeofT*newnslots);
    nslots = newnslots;
  }
  MEMCPY(stack + cur*sizeofT, item, sizeofT);
  cur++;
  return cur;
}

int StackBase::peek() {
  return cur;
}

int StackBase::peekAt(void *ptr, int n, int sizeofT) {
  if (ptr != NULL) MEMCPY(ptr, stack + (cur-1-n)*sizeofT, sizeofT);
  return cur;
}

int StackBase::getRef(void **ptr, int n, int sizeofT) {
  if (ptr != NULL) *ptr = stack + (cur-1-n)*sizeofT;
  return cur;
}

void *StackBase::top(int sizeofT) {
  ASSERT(cur >= 0);
  if (cur == 0) return NULL;
  return stack + (cur-1)*sizeofT;
}

int StackBase::pop(void *ptr, int sizeofT) {
  ASSERT(cur >= 0);
  if (cur == 0) return 0;
  ASSERT(stack != NULL);
  --cur;
  if (ptr != NULL) MEMCPY(ptr, stack + cur*sizeofT, sizeofT);
  return 1;
}

int StackBase::isempty() {
  return cur == 0;
}

void StackBase::purge() {
  ASSERT(isempty());
  if (stack != NULL) {
    FREE(stack);
    stack = NULL;
  }
}

