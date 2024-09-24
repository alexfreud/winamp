#include "main.h"
#include "./strings.h"
#include <strsafe.h>


wchar_t * 
String_Malloc(size_t size)
{
	return (wchar_t *)malloc(sizeof(wchar_t) * size);
}

wchar_t * 
String_ReAlloc(wchar_t *string, size_t size)
{
	return (wchar_t *)realloc(string, sizeof(wchar_t) * size);
}

void 
String_Free(wchar_t *string)
{
	if (NULL != string)
		free(string);
}

wchar_t * 
String_Duplicate(const wchar_t *string)
{
	int length;
	wchar_t *copy;

	if (NULL == string)
		return NULL;

	length = lstrlenW(string) + 1;
		
	copy = String_Malloc(length);
	if (NULL != copy)
		CopyMemory(copy, string, sizeof(wchar_t) * length);
	
	return copy;
}


char *
String_ToAnsi(unsigned int codePage,  unsigned long flags, const wchar_t *string, 
			  int stringLength, const char *defaultChar, BOOL *usedDefaultChar)
{
	char *buffer;
	int bufferSize;

	if (stringLength < 0)
		stringLength = lstrlen(string);
	
	bufferSize = WideCharToMultiByte(codePage, flags, string, stringLength, 
							NULL, 0, defaultChar, usedDefaultChar);
	if (0 == bufferSize) 
		return NULL;
	
	buffer = AnsiString_Malloc(bufferSize + 1);
	if (NULL == buffer) 
		return NULL; 
		
	bufferSize = WideCharToMultiByte(codePage, flags, string, stringLength, 
						buffer, bufferSize, defaultChar, usedDefaultChar);
	if (0 == bufferSize)
	{
		AnsiString_Free(buffer);
		return NULL;
	}
	buffer[bufferSize] = '\0';
	return buffer;
}

size_t 
String_CopyTo(wchar_t *destination, const wchar_t *source, size_t size)
{
	size_t remaining;
	if (FAILED(StringCchCopyExW(destination, size, source, NULL, &remaining, STRSAFE_IGNORE_NULLS)))
		return 0;
	
	return (size - remaining);
}


char * 
AnsiString_Malloc(size_t size)
{
	return (char*)malloc(sizeof(char) * size);
}

char * 
AnsiString_ReAlloc(char *string, size_t size)
{
	return (char*)realloc(string, sizeof(char) * size);
}

void 
AnsiString_Free(char *string)
{
	if (NULL != string)
		free(string);
}

char * 
AnsiString_Duplicate(const char *string)
{
	char *copy;
	INT length;

	if (NULL == string)
		return NULL;

	length = lstrlenA(string) + 1;
		
	copy = AnsiString_Malloc(length);
	if (NULL != copy)
		CopyMemory(copy, string, sizeof(char) * length);
	
	return copy;
}


wchar_t *
AnsiString_ToUnicode(unsigned int codePage, unsigned long flags, const char* string, INT stringLength)
{
	wchar_t *buffer;
	int buffferSize;
	
	if (NULL == string) 
		return NULL;

	buffferSize = MultiByteToWideChar(codePage, flags, string, stringLength, NULL, 0);
	if (0 == buffferSize) 
		return NULL;
	
	if (stringLength > 0) 
		buffferSize++;
	
	buffer = String_Malloc(buffferSize);
	if (NULL == buffer) 
		return NULL;

	if (0 == MultiByteToWideChar(codePage, flags, string, stringLength, buffer, buffferSize))
	{
		String_Free(buffer);
		return NULL;
	}

	if (stringLength > 0)
		buffer[buffferSize - 1] = L'\0';

	return buffer;
}

wchar_t* 
ResourceString_Duplicate(const wchar_t *source)
{
	return (FALSE != IS_INTRESOURCE(source)) ? 
			(LPWSTR)source : 
			String_Duplicate(source);
}

void 
ResourceString_Free(wchar_t *string)
{
	if (FALSE == IS_INTRESOURCE(string))
		String_Free(string);
}

size_t 
ResourceString_CopyTo(wchar_t *destination, const wchar_t *source, size_t size)
{
	if (NULL == destination)
		return 0;

	if (NULL == source)
	{
		destination[0] = L'\0';
		return 0;
	}

	if (FALSE != IS_INTRESOURCE(source))
	{
		if (NULL == WASABI_API_LNG)
		{
			destination[0] = L'\0';
			return 0;
		}
		
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)source, destination, size);
		return lstrlenW(destination);
	}

	return String_CopyTo(destination, source, size);
}
