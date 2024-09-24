#ifndef __WASABI_TYPES_H
#define __WASABI_TYPES_H

// first, some standard int types
typedef unsigned int UINT;
typedef signed int SINT;

typedef unsigned char UCHAR;
typedef signed char SCHAR;

typedef unsigned long ARGB32;
typedef unsigned long RGB32;

typedef unsigned long ARGB24;
typedef unsigned long RGB24;

typedef unsigned short ARGB16;
typedef unsigned short RGB16;

typedef unsigned long FOURCC;

#ifndef GUID_DEFINED
  #define GUID_DEFINED

  typedef struct _GUID 
	{
		unsigned long Data1;
		unsigned short Data2;
		unsigned short Data3;
		unsigned char Data4[8];
	} GUID;
/*
#ifndef _REFCLSID_DEFINED
#define REFGUID const GUID &
#define _REFCLSID_DEFINED
#endif
*/
#endif

#if defined(_WIN32) && !defined(__GNUC__)
#include <stddef.h>
	// since windows doesn't have stdint.h
	typedef unsigned __int64 uint64_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int8 uint8_t;
	typedef __int64 int64_t;
	typedef __int32 int32_t;
	typedef __int16 int16_t;
	typedef __int8  int8_t;
	typedef size_t ssize_t;
#else
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#endif

#ifdef WIN32
#include <windows.h>
// this is for GUID == and !=
#include <objbase.h>
#ifndef GUID_EQUALS_DEFINED
  #define GUID_EQUALS_DEFINED
#endif


#ifdef NULL
  #undef NULL
#endif
#ifndef NULL
  #define NULL 0
#endif

#ifdef _WIN32_WCE
typedef int intptr_t;
#endif
#endif
#endif
