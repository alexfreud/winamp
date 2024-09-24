#pragma once
/* important defines and enums that are shared between the C++ api and the C api */

#define NDE_CACHE TRUE
#define NDE_NOCACHE FALSE

#define NDE_OPEN_ALWAYS TRUE
#define NDE_OPEN_EXISTING FALSE

#if defined(WIN32_NOLIB) || !defined(WIN32) && !defined(WIN64) 
#define NDE_API
#else
 #ifdef NDE_EXPORTS
    #define NDE_API __declspec(dllexport)
 #else
    #define NDE_API __declspec(dllimport)
 #endif
#endif

// Field types
enum
{
	FIELD_COLUMN   =  0,
	FIELD_INDEX     =  1,
	FIELD_REDIRECTOR =2,
	FIELD_STRING     =3,
	FIELD_INTEGER    = 4,
	FIELD_BOOLEAN    = 5,
	FIELD_BINARY     =6, // max size 65536
	FIELD_GUID       =7,
	FIELD_PRIVATE    = 8,
	FIELD_BITMAP     =6,
	FIELD_FLOAT      = 9,
	FIELD_DATETIME   =10,
	FIELD_LENGTH     =11,
	FIELD_FILENAME	=12,
	FIELD_INT64    = 13,
	FIELD_BINARY32  =14, // binary field, but 32bit sizes instead of 16bit
	FIELD_INT128   = 15,  // mainly for storing MD5 hashes
	FIELD_UNKNOWN = 255, 
	FIELD_CLONE = 255,// internal use
};

// Filter types
enum {
  FILTER_NONE = 100,
  FILTER_EQUALS,
  FILTER_NOTEQUALS,
  FILTER_CONTAINS,
  FILTER_NOTCONTAINS,
  FILTER_ABOVE,
  FILTER_BELOW,
  FILTER_ABOVEOREQUAL,
  FILTER_BELOWOREQUAL,
  FILTER_BEGINS,
  FILTER_ENDS,
  FILTER_LIKE,
  FILTER_ISEMPTY,
  FILTER_ISNOTEMPTY,
	FILTER_BEGINSLIKE,
};

// compare modes 
#define COMPARE_MODE_CONTAINS		1
#define COMPARE_MODE_EXACT			2
#define COMPARE_MODE_STARTS			3

// scanner 'from' special constant
#define FIRST_RECORD    -1
