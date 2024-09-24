#ifndef _INTARRAY_H
#define _INTARRAY_H

#include <bfc/common.h>
#include <bfc/named.h>
#include <bfc/ptrlist.h>

class IntArray 
{
public:
  static int read(const wchar_t *name, int *x1, int *x2=NULL, int *x3=NULL, int *x4=NULL, int *x5=NULL, int *x6=NULL, int *x7=NULL, int *x8=NULL);
  static void write(const wchar_t *name, int x1);
  static void write(const wchar_t *name, int x1, int x2);
  static void write(const wchar_t *name, int x1, int x2, int x3);
  static void write(const wchar_t *name, int x1, int x2, int x3, int x4);
};

#endif
