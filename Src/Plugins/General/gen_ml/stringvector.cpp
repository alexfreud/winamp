#include "./stringvector.h"
#include <strsafe.h>

StringVector::StringVector(size_t cchAllocate, int cchAllocateStep) : pointers(NULL), ptCount(0), ptAllocated(0)
{	
	if (cchAllocate)
	{
		cchBuffer = cchAllocate;
		buffer = (wchar_t*)HeapAlloc(GetProcessHeap(), NULL, cchBuffer * sizeof(wchar_t));	
	}
	else
	{
		cchBuffer = 0;
		buffer = NULL;
	}

	tail = buffer;
	allocateStep = cchAllocateStep;
}

StringVector::~StringVector(void) 
{
	if (buffer) 
	{
		HANDLE hHeap = GetProcessHeap();
		HeapFree(hHeap, NULL, buffer);
		cchBuffer = 0;
		tail = NULL;
		buffer = NULL;
		if (pointers)
		{
			HeapFree(hHeap, NULL, pointers);
			ptCount = 0;
			ptAllocated =0;
			pointers = NULL;
		}
	}
}
size_t StringVector::Count(void) { return ptCount; }
size_t StringVector::GetCbAllocated(void){ return cchBuffer * sizeof(wchar_t); }
size_t StringVector::GetCchAllocated(void) { return cchBuffer; }
LPCWSTR StringVector::GetString(size_t index) { return (index >= 0 && index < ptCount) ? buffer + pointers[index].offset : NULL; }
int StringVector::GetStringLength(size_t index) { return (index >= 0 && index < ptCount) ? pointers[index].length : -1; }

void StringVector::Clear(void)
{
	tail = buffer;
	ptCount = 0;
}

int StringVector::SetAllocateStep(int cchNewStep)
{
	if (cchNewStep < 1) return allocateStep;
	int tmp = allocateStep;
	allocateStep = cchNewStep;
	return tmp;
}

size_t StringVector::Add(const wchar_t *entry, int cchLen)
{
	if (!entry) return -1;
	if (cchLen < 0) cchLen = lstrlenW(entry);
	if (cchLen == 0) return -1;

	size_t cchNeed = (size_t)(tail - buffer) + cchLen + 2;
	
	if (cchBuffer < cchNeed)
	{
		while ( cchBuffer < cchNeed) cchBuffer += allocateStep;
		size_t offset_tail = (size_t)(tail - buffer);
		buffer = (wchar_t*)( (buffer) ? HeapReAlloc(GetProcessHeap(), NULL, buffer, cchBuffer*sizeof(wchar_t)) : HeapAlloc(GetProcessHeap(), NULL, cchBuffer*sizeof(wchar_t)));
		tail = buffer + offset_tail;
	}
	if (S_OK != StringCchCopyNW(tail, cchBuffer - (size_t)(tail - buffer), entry, cchLen)) return -1;
	
	if (ptCount == ptAllocated) 
	{
		ptAllocated += 24;
		pointers = (HRECORD)( (pointers) ? HeapReAlloc(GetProcessHeap(), NULL, pointers, ptAllocated*sizeof(RECORD)) : HeapAlloc(GetProcessHeap(), NULL, ptAllocated*sizeof(RECORD)));
		if (pointers == NULL)
		{
			DWORD err = GetLastError();
			err += 1;
		}
	}
	pointers[ptCount].offset = tail - buffer;
	pointers[ptCount].length = cchLen;

	tail += cchLen + 1;
	ptCount++;

	return ptCount -1;
}

BOOL StringVector::Remove(size_t index)
{
	if (index < 0 || index >= ptCount) return FALSE;

	ptCount--;
	for (size_t i = index; i < ptCount; i++) pointers[i] = pointers[i + 1];
	return TRUE;
}

void StringVector::TrimCount(size_t newCount)
{
	if (newCount >= ptCount) return;
	if (newCount <= 0) { Clear();  return; }
	tail = buffer + pointers[newCount].offset;
	ptCount = newCount;
}

size_t StringVector::FindString(LCID lcid, LPCWSTR string, int cchLen, BOOL igonreCase)
{
	if (!string) return -1;
	if (cchLen < 0) cchLen = lstrlenW(string);
	if (cchLen == 0) return -1;

	for (size_t i = 0; i < ptCount; i ++)
	{
		if ((cchLen == pointers[i].length) &&
			( string == (buffer + pointers[i].offset) || CSTR_EQUAL == CompareStringW(lcid, (igonreCase) ? NORM_IGNORECASE : NULL, 
																						buffer + pointers[i].offset,
																						cchLen,
																						string,
																						cchLen)))
			return i;

	}
	return -1;
}