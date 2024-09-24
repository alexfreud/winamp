#ifndef _NULLSOFT_WINAMP_ML_DEVICES_STRINGS_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_STRINGS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#define IS_STRING_EMPTY(_string) (NULL == (_string) || L'\0' == *(_string))

wchar_t *
String_Malloc(size_t size);

wchar_t * 
String_ReAlloc(wchar_t *string, 
			   size_t size);

void 
String_Free(wchar_t *string);

wchar_t * 
String_Duplicate(const wchar_t *string);

char *
String_ToAnsi(unsigned int codePage, 
			  unsigned long flags, 
			  const wchar_t *string, 
			  int stringLength, 
			  const char *defaultChar, 
			  BOOL *usedDefaultChar);

size_t 
String_CopyTo(wchar_t *destination,
			  const wchar_t *source,
			  size_t size);
/*
	Ansi String
*/

char * 
AnsiString_Malloc(size_t size);

char *
AnsiString_ReAlloc(char *string, 
				   size_t size);

void 
AnsiString_Free(char *string);

char * 
AnsiString_Duplicate(const char *string);

wchar_t *
AnsiString_ToUnicode(unsigned int codePage, 
					 unsigned long flags, 
					 const char *string, 
					 int stringLength);

/*
	Resource String
*/

wchar_t* 
ResourceString_Duplicate(const wchar_t *source);

void 
ResourceString_Free(wchar_t *string);

size_t 
ResourceString_CopyTo(wchar_t *destination,
					  const wchar_t *source,
					  size_t size);


#endif //_NULLSOFT_WINAMP_ML_DEVICES_PLUGIN_STRINGS_HEADER