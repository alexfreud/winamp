#include "main.h"
#include ".\MLString.h"
#include <strsafe.h>

#define ALLOCATION_STEP		16

void* MLString::heap = GetProcessHeap();

MLString::MLString(void) : buffer(NULL), allocated(0), cchLen(0)
{
}

MLString::MLString(const wchar_t* string)  : buffer(NULL), allocated(0), cchLen(0)
{
	if (string) Append(string, lstrlenW(string));
}
MLString::MLString(unsigned int cchBuffer)  : buffer(NULL), allocated(0), cchLen(0)
{
	Allocate(cchBuffer);
}

MLString::MLString(const MLString &copy)  : buffer(NULL), allocated(0)
{
	cchLen = copy.cchLen;
	Allocate(cchLen +  1);
	CopyMemory(buffer, copy.buffer, cchLen*sizeof(wchar_t));
}

MLString::~MLString(void)
{
	if (buffer)
	{
		HeapFree(heap, NULL, buffer);
		allocated = 0;
		cchLen = 0;
		buffer = NULL;
	}
}

HRESULT MLString::Append(const wchar_t* string, unsigned int cchLength)
{
	if (allocated <= cchLen + cchLength)
	{
		do { allocated += ALLOCATION_STEP; } while(allocated < cchLen + cchLength + 1);
		buffer = (wchar_t*) ((!buffer) ? HeapAlloc(heap, NULL, allocated*sizeof(wchar_t)) : 
								HeapReAlloc(heap, NULL, buffer, allocated*sizeof(wchar_t)));
	}
	if (!buffer) return ERROR_OUTOFMEMORY;
	CopyMemory(buffer + cchLen, string, cchLength*sizeof(wchar_t));
	cchLen += cchLength;
	buffer[cchLen] = 0x0000;
	return S_OK;
}

HRESULT MLString::Set(const wchar_t* string, unsigned int cchLength)
{
	cchLen = 0;
	return Append(string, cchLength);
}

HRESULT MLString::Allocate(unsigned int cchNewSize)
{
	if (cchNewSize <= cchLen) return ERROR_BAD_LENGTH;
	if (allocated >= cchNewSize) return S_OK;
	allocated = cchNewSize;
	buffer = (wchar_t*) ((!buffer) ? HeapAlloc(heap, NULL, allocated*sizeof(wchar_t)) : 
								HeapReAlloc(heap, NULL, buffer, allocated*sizeof(wchar_t)));
	return (buffer) ? S_OK : ERROR_OUTOFMEMORY;
}

void MLString::Compact(void)
{
	if (!buffer) return;
	if (0 == cchLen)
	{
		HeapFree(heap, NULL, buffer);
		allocated = 0;
		cchLen = 0;
		buffer = NULL;
	}
	else
	{
		allocated = cchLen + 1;
		buffer = (wchar_t*)HeapReAlloc(heap, NULL, buffer, allocated*sizeof(wchar_t));
	}
}

HRESULT MLString::Format(const wchar_t *format, ...)
{
	va_list argList;
	va_start(argList, format);
	HRESULT retCode;
	size_t remaining = 0;
	retCode = (allocated) ? StringCchVPrintfExW(buffer, allocated, NULL, &remaining, STRSAFE_NULL_ON_FAILURE, format, argList) : STRSAFE_E_INSUFFICIENT_BUFFER; 
	while (STRSAFE_E_INSUFFICIENT_BUFFER == retCode)
	{
		int attempt = 1;
		allocated += ALLOCATION_STEP*attempt;
		attempt++;
		buffer = (wchar_t*) ((!buffer) ? HeapAlloc(heap, NULL, allocated*sizeof(wchar_t)) : 
								HeapReAlloc(heap, NULL, buffer, allocated*sizeof(wchar_t)));
		retCode = StringCchVPrintfExW(buffer, allocated, NULL, &remaining, STRSAFE_NULL_ON_FAILURE, format, argList); 
	}
	va_end(argList);
	cchLen = (S_OK == retCode) ? allocated - (unsigned int)remaining : 0;
	return retCode;
}

HRESULT MLString::CopyTo(MLString *destination)
{
	HRESULT retCode = destination->Allocate(allocated);
	if (S_OK != retCode) return retCode;
	destination->cchLen = cchLen;
	CopyMemory(destination->buffer, buffer, cchLen*sizeof(wchar_t));
	return S_OK;
}