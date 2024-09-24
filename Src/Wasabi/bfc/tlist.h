//PORTABLE
#ifndef _TLIST_H
#define _TLIST_H

#include <bfc/common.h>
#include <bfc/wasabi_std.h>
#include <bfc/named.h>

/*

A generic std::vector wannabe...

NOTE: you must only use this with basic types! constructors and destructors
will not be called.

*/

#include <bfc/bfc_assert.h>
#include <bfc/std_mem.h>

#define TLIST_INCREMENT 32

template<class T>
class TList {
public:
  TList() {
    nitems = 0;
    nslots = 0;
    items = NULL;
  }
  virtual ~TList() {
    FREE(items);
  }

  int getNumItems() const { return nitems; };

  // CAN'T PROVIDE BOUNDS CHECKING!!! (except by ASSERT() :( )
  T enumItem(int n) const {
    ASSERT(items != NULL);
    ASSERT(n >= 0 && n < nitems);
    return items[n];
  }
  T operator[](int n) const { return enumItem(n); }

  T getFirst() const { return enumItem(0); }
  T getLast() const { return enumItem(getNumItems()-1); }

  T *enumRef(int n) const {
    ASSERT(items != NULL);
    ASSERT(n >= 0 && n < nitems);
    return items+n;
  }

  T *addItem(const T &item) {
    ASSERT(nitems <= nslots);
    if (items == NULL) {
      nslots = TLIST_INCREMENT;
      items = (T *)MALLOC(sizeof(T) * nslots);
      nitems = 0;
    } else if (nslots == nitems) {	// time to expand
      T *newitems;
      nslots += TLIST_INCREMENT;
      newitems = (T *)MALLOC(sizeof(T) * nslots);
      MEMCPY(newitems, items, nitems * sizeof(T));
      FREE(items);
      items = newitems;
    }
    items[nitems++] = item;

    return &items[nitems-1];
  }

  T *insertItem(const T &item, int pos) { // position to insert
    ASSERT(pos >= 0 && pos <= nitems);
    T *tmp=addItem(item);
    int n=nitems-1;
    while (n > pos) {
      items[n]=items[n-1];
      n--;
    }
    tmp=items+pos;
    *tmp=item;
    return tmp;
  }

  T *replaceItemByPos(const T &item, int pos) {
    ASSERT(items != NULL);
    ASSERT(nitems != 0);
    ASSERT(pos >= 0 && pos < nitems);
    items[pos]=item;
    return &items[pos];
  }

  void setItem(int pos, const T &newval) {
    if (pos < 0) return;
    if (pos >= nitems) return;
    items[pos] = newval;
  }

  int delItem(const T &item) {
    int c = 0;

    ASSERT(items != NULL);
    ASSERT(nitems != 0);

    T *src = items;
    T *dst = items;
    for (int i = 0; i < nitems; i++) {
      if (*src != item) {
        *dst++ = *src;
        c++;
      }
      src++;
    }
    nitems -= c;

    return c;
  }

  int delByPos(int pos) {
    if (pos < 0 || pos >= nitems) return 0;
    --nitems;
    if (pos == nitems) return 1;	// last one? nothing to copy over
    MEMCPY(items+pos, items+(pos+1), sizeof(T)*(nitems-pos));  // CT:not (nitems-(pos+1)) as nitems has been decremented earlier
    return 1;
  }

  void removeAll() {
    FREE(items); items = NULL;
    nitems = 0;
    nslots = 0;
  }

  void freeAll() {
    for (int i = 0; i < nitems; i++)
      FREE(items[i]);
    removeAll();
  }

  int getItemPos(const T &it) const {
    int n = getNumItems();
    for (int i=0;i<n;i++)
      if(!MEMCMP(&items[i],&it,sizeof(T))) return i; // CT     if (items[i] == it) return i;
    return -1;
  }
  int haveItem(const T &it) const {
    return (getItemPos(it) != -1);
  }

private:
  int nitems, nslots;
  T *items;
};


#endif
