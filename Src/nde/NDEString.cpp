#include "NDEString.h"
#include "foundation/error.h"

#include <windows.h>

typedef BOOL (WINAPI *HEAPSETINFORMATION)(HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass,  PVOID HeapInformation, SIZE_T HeapInformationLength);

static HANDLE string_heap=0;
static HMODULE kernel=0;
extern "C" void NDE_HeapInit()
{
	if (!string_heap)
	{
		string_heap=HeapCreate(0, 0, 0);
		kernel = LoadLibraryW(L"kernel32.dll");
		if (kernel)
		{
			HEAPSETINFORMATION hsi = (HEAPSETINFORMATION)GetProcAddress(kernel, "HeapSetInformation");
			if (hsi)
			{
				ULONG argh = 2;
				hsi(string_heap, HeapCompatibilityInformation, &argh, sizeof(ULONG));
			}
		}
		NXStringSetHeap(string_heap);
	}
}

extern "C" void NDE_HeapQuit()
{
	if (kernel)
		FreeLibrary(kernel);	
}

nx_string_t ndestring_get_string(wchar_t *str)
{
	if (!str)
		return 0;
	nx_string_t self = (nx_string_t)((uint8_t *)str - sizeof(size_t) - sizeof(size_t));
	return self;
}

wchar_t *ndestring_wcsdup(const wchar_t *str)
{
	NDE_HeapInit();
	if (!str)
		return 0;
	nx_string_t value;
	if (NXStringCreateWithUTF16(&value, str) != NErr_Success)
		return 0;
	return value->string;
}

wchar_t *ndestring_wcsndup(const wchar_t *str, size_t len)
{
	NDE_HeapInit();
	if (!str)
		return 0;
	nx_string_t value;
	if (NXStringCreateWithBytes(&value, str, len*2, nx_charset_utf16le) != NErr_Success)
		return 0;
	return value->string;	
}

wchar_t *ndestring_malloc(size_t str_size)
{
	NDE_HeapInit();
	nx_string_t value=NXStringMalloc((str_size+1)/2 - 1);
	if (!value)
		return 0;
	return value->string;
}

void ndestring_release(wchar_t *str)
{
	nx_string_t value = ndestring_get_string(str);
	NXStringRelease(value);
}

void ndestring_retain(wchar_t *str)
{
	nx_string_t value = ndestring_get_string(str);
	NXStringRetain(value);	
}