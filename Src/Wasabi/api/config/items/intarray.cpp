#include <precomp.h>

#include "intarray.h"


enum { MAX_ARRAY=8 };

int IntArray::read(const wchar_t *name, int *x1, int *x2, int *x3, int *x4, int *x5, int *x6, int *x7, int *x8) {
  PtrList<int> list;
  if (x1) { list.addItem(x1); }
  if (x2) { list.addItem(x2); }
  if (x3) { list.addItem(x3); }
  if (x4) { list.addItem(x4); }
  if (x5) { list.addItem(x5); }
  if (x6) { list.addItem(x6); }
  if (x7) { list.addItem(x7); }
  if (x8) { list.addItem(x8); }
  ASSERT(list.getNumItems() >= 1);

  int array[MAX_ARRAY];	// gcc rules, msvc drools
  for (int i = 0; i < list.getNumItems(); i++) {
    if (list[i]) array[i] = *list[i];
  }
  if (!WASABI_API_CONFIG->getIntArrayPrivate(name, array, list.getNumItems())) return 0;
  for (int j = 0; j < list.getNumItems(); j++) {
    if (list[j]) *list[j] = array[j];
  }
  return 1;
}

void IntArray::write(const wchar_t *name, int x1) {
  int array[] = { x1 };
  WASABI_API_CONFIG->setIntArrayPrivate(name, array, sizeof(array)/sizeof(int));
}

void IntArray::write(const wchar_t *name, int x1, int x2) {
  int array[] = { x1, x2 };
  WASABI_API_CONFIG->setIntArrayPrivate(name, array, sizeof(array)/sizeof(int));
}

void IntArray::write(const wchar_t *name, int x1, int x2, int x3) {
  int array[] = { x1, x2, x3 };
  WASABI_API_CONFIG->setIntArrayPrivate(name, array, sizeof(array)/sizeof(int));
}

void IntArray::write(const wchar_t *name, int x1, int x2, int x3, int x4) {
  int array[] = { x1, x2, x3, x4 };
  WASABI_API_CONFIG->setIntArrayPrivate(name, array, sizeof(array)/sizeof(int));
}
