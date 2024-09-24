#ifndef _duktypes_h
#define _duktypes_h

#if defined(__cplusplus)
extern "C" {
#endif
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;
typedef signed char I8;
typedef short I16;
typedef long I32;
/*
//typedef int Boolean;
*/
#if 1 /* def _MSC_VER */
typedef unsigned char uchar;
/*typedef unsigned short ushort;*/
typedef unsigned long ulong;
#endif
#if defined(__cplusplus)
}
#endif

#endif
