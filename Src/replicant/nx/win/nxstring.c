#include "nxstring.h"
#include "foundation/error.h"
#include <shlwapi.h>
#include "foundation/atomics.h"
#include <wchar.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#pragma comment(lib, "shlwapi.lib")

//#define NX_STRING_STRICT_HEAP

HANDLE string_heap = 0;

int NXStringSetHeap(HANDLE _string_heap)
{
	if (!string_heap)
	{
		string_heap = _string_heap;
		return NErr_Success;
	}
	else
	{
		return NErr_NoAction;
	}
}

// don't include null terminator here
static size_t NXStringMallocSize(size_t characters)
{
	/* TODO: overflow check? */
	const nx_string_t dummy=NULL;
	size_t header = (size_t)&dummy->string[0] - (size_t)dummy;
	return header + (characters+1) * sizeof(wchar_t);
}

// don't include null terminator here
nx_string_t NXStringMalloc(size_t characters)
{
	if (!string_heap)
	{
		string_heap = GetProcessHeap();
	}
	return NXStringMallocWithHeap(string_heap, characters);
}

nx_string_t NXStringRealloc(nx_string_t str, size_t characters)
{
	nx_string_t new_str = (nx_string_t)HeapReAlloc(string_heap, 0, str, NXStringMallocSize(characters));
	// on failure, kick back the original block (TODO need to review this)
	if (!new_str)
	{
		return str;
	}
	return new_str;
}

nx_string_t NXStringMallocWithHeap(HANDLE heap, size_t characters)
{
#ifdef NX_STRING_STRICT_HEAP
	nx_string_t str;
	size_t string_size = NXStringMallocSize(characters);

	size_t allocated_size = (string_size + 8191) & ~4095;
	size_t offset = 4096 - (string_size & 4095);
	size_t pages = allocated_size / 4096;
	uint8_t *protect_start;
	void *mem = VirtualAlloc(0, allocated_size, MEM_COMMIT, PAGE_READWRITE);

	if (!mem)
		return 0;

	protect_start = (uint8_t *)mem + (pages-1)*4096;
	VirtualProtect(protect_start, 4096, PAGE_NOACCESS, 0);

	str = (nx_string_t)((uint8_t *)mem + offset);
	str->ref_count = 1;
	str->len = characters;
	return str;
#else
	nx_string_t str = (nx_string_t)HeapAlloc(heap, 0, NXStringMallocSize(characters));
	if (str)
	{
		str->ref_count = 1;
		str->len = characters;
	}
	return str;
#endif
}

int NXStringFree(HANDLE heap, nx_string_t str)
{
#ifdef NX_STRING_STRICT_HEAP
	uint8_t *mem = (uint8_t *)((size_t)str & 4095);
	VirtualProtect(mem, 4096, PAGE_NOACCESS, 0);
	assert(_heapchk() == _HEAPOK);
	return NErr_Success;
#else
	if (HeapFree(heap, 0, str))
	{
		return NErr_Success;
	}
	else
	{
		return NErr_Error;
	}
#endif
}

nx_string_t NXStringCreate(const wchar_t *str)
{
	size_t size;
	nx_string_t nxstr;

	if (!str || (size_t)str <= 65536)
	{
		return 0;
	}

	size = wcslen(str);
	nxstr = NXStringMalloc(size);
	if (nxstr)
	{
		memcpy(nxstr->string, str, size*sizeof(wchar_t));
		nxstr->string[size]=0;
	}
	return nxstr;
}

int NXStringCreateEmpty(nx_string_t *new_string)
{
	nx_string_t nxstr = NXStringMalloc(0);
	if (nxstr)
	{
		nxstr->string[0]=0;
		*new_string = nxstr;
		return NErr_Success;
	}
	else
	{
		return NErr_OutOfMemory;
	}
}

nx_string_t NXStringCreateWithHeap(HANDLE heap, const wchar_t *str)
{
	size_t size = wcslen(str);
	nx_string_t nxstr = NXStringMallocWithHeap(heap, size);
	if (nxstr)
	{
		memcpy(nxstr->string, str, size*sizeof(wchar_t));
		nxstr->string[size]=0;
	}
	return nxstr;
}

nx_string_t NXStringCreateFromUTF8(const char *str)
{
	nx_string_t nxstr;
	size_t size = MultiByteToWideChar(CP_UTF8, 0, str, -1, 0,0);
	if (!size)
	{
		return 0;
	}

	nxstr = NXStringMalloc(size-1);
	if (nxstr)
	{
		if (!MultiByteToWideChar(CP_UTF8, 0, str, -1, nxstr->string, (int)size))
		{
			NXStringFree(string_heap, nxstr);
			return 0;
		}
	}

	return nxstr;
}

int NXStringCreateWithUTF8(nx_string_t *new_value, const char *str)
{
	size_t size;
	nx_string_t nxstr;

	if (!str)
	{
		return NErr_Empty;
	}

	size = MultiByteToWideChar(CP_UTF8, 0, str, -1, 0,0);
	if (!size)
	{
		return NErr_Error;
	}

	nxstr = NXStringMalloc(size-1);
	if (!nxstr)
	{
		return NErr_OutOfMemory;
	}

	if (!MultiByteToWideChar(CP_UTF8, 0, str, -1, nxstr->string, (int)size))
	{
		NXStringFree(string_heap, nxstr);
		return NErr_Error;
	}

	*new_value = nxstr;
	return NErr_Success;
}

int NXStringCreateWithUTF16(nx_string_t *new_value, const wchar_t *str)
{
	size_t size;
	nx_string_t nxstr;

	if (!str)
	{
		return NErr_Empty;
	}

	size = wcslen(str);
	nxstr = NXStringMalloc(size);
	if (!nxstr)
	{
		return NErr_OutOfMemory;
	}

	memcpy(nxstr->string, str, size*sizeof(wchar_t));
	nxstr->string[size]=0;
	*new_value = nxstr;
	return NErr_Success;
}

int NXStringCreateWithCString(nx_string_t *new_value, const char *str, nx_charset_t charset)
{
	nx_string_t nxstr;
	size_t size = MultiByteToWideChar(charset, 0, str, -1, 0,0);
	if (!size)
	{
		return NErr_Error;
	}

	nxstr = NXStringMalloc(size-1);
	if (!nxstr)
	{
		return NErr_OutOfMemory;
	}

	if (!MultiByteToWideChar(charset, 0, str, -1, nxstr->string, (int)size))
	{
		NXStringFree(string_heap, nxstr);
		return NErr_Error;
	}

	*new_value = nxstr;
	return NErr_Success;	
}

nx_string_t NXStringRetain(nx_string_t string)
{
	if (!string)
	{
		return 0;
	}

	nx_atomic_inc(&string->ref_count);
	return string;
}

void NXStringRelease(nx_string_t string)
{
	if (string)
	{
		if (nx_atomic_dec(&string->ref_count) == 0)
		{
			NXStringFree(string_heap, string);
		}
	}
}

nx_string_t NXStringCreateFromPath(const wchar_t *folder, const wchar_t *filename)
{
	nx_string_t pathstr = NXStringMalloc(MAX_PATH);
	if (pathstr)
	{
		PathCombineW(pathstr->string, folder, filename);
		pathstr->len = wcslen(pathstr->string);
	}
	return pathstr;
}

nx_string_t NXStringCreateFromUInt64(uint64_t value)
{
	nx_string_t intstr = NXStringMalloc(21); 
	if (intstr)
	{
		_ui64tow(value, intstr->string, 10);
		intstr->len = wcslen(intstr->string);
	}
	return intstr;
}

int NXStringCreateWithUInt64(nx_string_t *new_value, uint64_t value)
{
	nx_string_t intstr = NXStringMalloc(21); 
	if (!intstr)
	{
		return NErr_OutOfMemory;
	}

	_ui64tow(value, intstr->string, 10);
	intstr->len = wcslen(intstr->string);
	*new_value = intstr;
	return NErr_Success;
}

int NXStringCreateWithInt64(nx_string_t *new_value, int64_t value)
{
	nx_string_t intstr = NXStringMalloc(21); 
	if (!intstr)
	{
		return NErr_OutOfMemory;
	}

	_i64tow(value, intstr->string, 10);
	intstr->len = wcslen(intstr->string);
	*new_value = intstr;
	return NErr_Success;
}

int NXStringCreateWithBytes(nx_string_t *new_string, const void *data, size_t len, nx_charset_t charset)
{
	nx_string_t nxstr;
	if (!len)
	{
		return NXStringCreateEmpty(new_string);
	}
	if (charset == nx_charset_utf16le)
	{
		nxstr = NXStringMalloc(len/2);
		if (nxstr)
		{
			memcpy(nxstr->string, data, len);
			nxstr->string[len/2]=0;
			nxstr->len = len/2;
			*new_string = nxstr;
			return NErr_Success;
		}
		else
		{
			return NErr_OutOfMemory;
		}
	}
	else if (charset == nx_charset_utf16be)
	{
		nxstr = NXStringMalloc(len/2);
		if (nxstr)
		{
			LCMapString(LOCALE_INVARIANT, LCMAP_BYTEREV, (LPCWSTR)data, (int)len/2, nxstr->string, (int)len/2);
			nxstr->string[len/2]=0;
			nxstr->len = len/2;
			*new_string = nxstr;
			return NErr_Success;
		}
		else
		{
			return NErr_OutOfMemory;
		}
	}
	else
	{
		int size = MultiByteToWideChar((UINT)charset, 0, (const char *)data, (int)len, 0, 0);
		if (!size)
		{
			return NErr_Error;
		}

		nxstr = NXStringMalloc(size);
		if (nxstr)
		{
			if (!MultiByteToWideChar((UINT)charset, 0, (const char *)data, (int)len, nxstr->string, size))
			{
				NXStringFree(string_heap, nxstr);
				return NErr_Error;
			}
			nxstr->string[size]=0;
			nxstr->len = size;
			*new_string = nxstr;
			return NErr_Success;
		}
		else
		{
			return NErr_OutOfMemory;
		}
	}
}

size_t NXStringGetLength(nx_string_t string)
{
	return (string ? string->len : 0);
}

/* --- Keyword (ASCII) comparison --- */
int NXStringKeywordCompareWithCString(nx_string_t string, const char *compare_to)
{
	const wchar_t *src = string->string;
	const char *dst = compare_to;

	int ret = 0 ;

	while( ! (ret = (int)((*src & ~0x20) - (*dst & ~0x20))) && *dst)
	{
		++src, ++dst;
	}

	if ( ret < 0 )
	{
		ret = -1 ;
	}
	else if ( ret > 0 )
	{
		ret = 1 ;
	}
	return( ret );
}

int NXStringKeywordCompare(nx_string_t string, nx_string_t compare_to)
{
	const wchar_t *src = string->string;
	const wchar_t *dst = compare_to->string;

	int ret = 0 ;

	while( ! (ret = (int)((*src & ~0x20) - (*dst & ~0x20))) && *dst)
	{
		++src, ++dst;
	}

	if ( ret < 0 )
	{
		ret = -1 ;
	}
	else if ( ret > 0 )
	{
		ret = 1 ;
	}
	return( ret );
}

int NXStringKeywordCaseCompare(nx_string_t string, nx_string_t compare_to)
{
	const wchar_t *src = string->string;
	const wchar_t *dst = compare_to->string;

	int ret = 0 ;

	while( ! (ret = (int)(*src - (wchar_t)*dst)) && *dst)
	{
		++src, ++dst;
	}

	if ( ret < 0 )
	{
		ret = -1 ;
	}
	else if ( ret > 0 )
	{
		ret = 1 ;
	}

	return( ret );
}

int NXStringCreateBasePathFromFilename(nx_string_t filename, nx_string_t *basepath)
{
	nx_string_t nxstr;
	size_t len = filename->len;
	while (len && filename->string[len-1] != '\\' && filename->string[len-1] != '/')
	{
		len--;
	}

	if (!len)
	{
		return NErr_Empty;
	}

	nxstr = NXStringMalloc(len);
	if (!nxstr)
	{
		return NErr_OutOfMemory;
	}

	memcpy(nxstr->string, filename->string, sizeof(wchar_t)*len);
	nxstr->string[len]=0;
	*basepath = nxstr;
	return NErr_Success;
}

int NXStringGetCString(nx_string_t string, char *user_buffer, size_t user_buffer_length, const char **out_cstring, size_t *out_cstring_length)
{
	size_t size;

	/* TODO: error check this with large strings and small user_buffer_length sizes */
	if (!string)
	{
		return NErr_NullPointer;
	}

	if (user_buffer_length == 0)
		return NErr_Insufficient;

	size = WideCharToMultiByte(CP_ACP, 0, string->string, (int)string->len, user_buffer, (int)user_buffer_length-1, NULL, NULL);
	if (size == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		size = user_buffer_length-1;
	user_buffer[size]=0;
	*out_cstring = user_buffer;
	*out_cstring_length = (size_t)size;
	return NErr_Success;
}

int NXStringGetDoubleValue(nx_string_t string, double *value)
{
	if (!string)
		return NErr_NullPointer;

	*value = wcstod(string->string, 0);
	return NErr_Success;
}

int NXStringGetBytesSize(size_t *byte_count, nx_string_t string, nx_charset_t charset, int flags)
{
	if (charset == nx_charset_utf16le)
	{
		if (flags & nx_string_get_bytes_size_null_terminate)
			*byte_count = (string->len + 1)* sizeof(wchar_t);
		else
			*byte_count = string->len * sizeof(wchar_t);
		return NErr_DirectPointer;
	}
	else
	{
		size_t size=0;
		/*if (flags & nx_string_get_bytes_size_null_terminate)
			size = WideCharToMultiByte(charset, 0, string->string, string->len, 0, 0, NULL, NULL);
		else*/
			size = WideCharToMultiByte(charset, 0, string->string, (int)string->len, 0, 0, NULL, NULL);

		if (!size)
			return NErr_Error;

		if (flags & nx_string_get_bytes_size_null_terminate)
			*byte_count = size+1;
		else
			*byte_count = size;

		return NErr_Success;
	}
}

int NXStringGetBytesDirect(const void **bytes, size_t *length, nx_string_t string, nx_charset_t charset, int flags)
{
	if (charset == nx_charset_utf16le)
	{
		*bytes = string->string;
		if (length)
		{
			if (flags & nx_string_get_bytes_size_null_terminate)
				*length = (string->len+1) * sizeof(wchar_t); // TODO: overflow check
			else
				*length = string->len * sizeof(wchar_t); // TODO: overflow check
		}
		return NErr_Success;
	}
	else
	{
		return NErr_Error;
	}

}

int NXStringGetBytes(size_t *bytes_copied, nx_string_t string, void *bytes, size_t length, nx_charset_t charset, int flags)
{
	if (charset == nx_charset_utf16le)
	{
		length/=2;

		if (flags & nx_string_get_bytes_size_null_terminate)
		{
			if (length == 0)
				return NErr_Insufficient;

			length--;
		}

		if (length > string->len)
			length = string->len;
		wmemcpy((wchar_t *)bytes, string->string, length);

		if (flags & nx_string_get_bytes_size_null_terminate)
			((wchar_t *)bytes)[length++]=0;

		if (bytes_copied)
			*bytes_copied = length * 2;
		return NErr_Success;
	}
	else
	{
		size_t size=0;
		if (flags & nx_string_get_bytes_size_null_terminate)
		{
			size = WideCharToMultiByte(charset, 0, string->string, (int)string->len, (LPSTR)bytes, (int)length-1, NULL, NULL);
			((char *)bytes)[size]=0;
		}
		else
		{
			size = WideCharToMultiByte(charset, 0, string->string, (int)string->len, (LPSTR)bytes, (int)length, NULL, NULL);
		}

		if (!size)
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				if (flags & nx_string_get_bytes_size_null_terminate)
					size = length-1;
				else
					size=length;
			}
			else
			{
				return NErr_Error;
			}
		}

		if (bytes_copied)
		{
			if (flags & nx_string_get_bytes_size_null_terminate)
			{
				if (size)
					*bytes_copied = size+1;
				else
					*bytes_copied = length+1;
			}
			else
			{
				if (size)
					*bytes_copied = size;
				else
					*bytes_copied = length;
			}
		}
		return NErr_Success;
	}
}

int NXStringGetIntegerValue(nx_string_t string, int *value)
{
	*value = wcstol(string->string, 0, 10);
	return NErr_Success;
}

int NXStringGetGUIDValue(nx_string_t string, GUID *out_guid)
{
	/* TODO: it'd be nice if this was a bit more flexible on input, e.g. no dashes vs dashes */
	GUID guid = GUID_NULL;
	size_t offset = 0;
	int Data1, Data2, Data3;
	int Data4[8] = {0};

	for (;;)
	{
		if (string->string[offset] == '{')
		{
			offset++;
		}
		else if (string->string[offset] == ' ')
		{
			offset++;
		}
		else
		{
			break;
		}
	}

	//{ 0x1b3ca60c, 0xda98, 0x4826, { 0xb4, 0xa9, 0xd7, 0x97, 0x48, 0xa5, 0xfd, 0x73 } };
	swscanf( string->string, L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
    &Data1, &Data2, &Data3, Data4 + 0, Data4 + 1,
    Data4 + 2, Data4 + 3, Data4 + 4, Data4 + 5, Data4 + 6, Data4 + 7 );

	// Cross assign all the values
	guid.Data1 = Data1;
	guid.Data2 = Data2;
	guid.Data3 = Data3;
	guid.Data4[0] = Data4[0];
	guid.Data4[1] = Data4[1];
	guid.Data4[2] = Data4[2];
	guid.Data4[3] = Data4[3];
	guid.Data4[4] = Data4[4];
	guid.Data4[5] = Data4[5];
	guid.Data4[6] = Data4[6];
	guid.Data4[7] = Data4[7];

	*out_guid = guid;
	return NErr_Success;
}

nx_compare_result NXStringCompare(nx_string_t string1, nx_string_t string2, nx_compare_options options)
{
	int compareFlags = 0;
	
	if (0 != (nx_compare_case_insensitive & options))
	{
		compareFlags |= NORM_IGNORECASE;
	}
	return CompareString(LOCALE_USER_DEFAULT, compareFlags, string1->string, -1, string2->string, -1) - 2;
}

int NXStringCreateWithFormatting(nx_string_t *new_string, const char *format, ...)
{
	size_t cch, ret;
	char *temp = 0;
	va_list v;
	va_start(v, format);
	
	cch = _vscprintf(format, v);
	if (cch == -1)
	{
		return NErr_Error;
	}

	if (cch > 256)
	{
		temp = (char *)malloc(cch+1);
		if (!temp)
		{
			return NErr_OutOfMemory;
		}	
		vsprintf(temp, format, v);
	
		ret = NXStringCreateWithUTF8(new_string, temp);
		free(temp);
	}
	else
	{
		temp = (char *)_malloca(cch+1);
		if (!temp)
		{
			return NErr_OutOfMemory;
		}
		vsprintf(temp, format, v);
		ret = NXStringCreateWithUTF8(new_string, temp);
	}
	va_end(v);
	return (int)ret;
}