#pragma once
#include "nx/nxstring.h"
#include "nx/nxuri.h"
#include "foundation/error.h"

template <nx_charset_t charset>
class AutoCharNX
{
public:
	AutoCharNX()
	{
		Init();
	}

	AutoCharNX(size_t bytes)
	{
		Init();
		ptr = (char *)malloc(bytes);
		malloc_size = bytes;
	}

	AutoCharNX(nx_string_t string)
	{
		Init();

		Set(string);
	}

	AutoCharNX(nx_uri_t filename)
	{
		Init();

		Set(filename);
	}

	~AutoCharNX()
	{
		if (owned)
			free(ptr);
		if (reference_string)
			NXStringRelease(reference_string);
	}

	int Set(nx_string_t string)
	{
		if (reference_string == string)
			return NErr_Success;

		if (reference_string)
			NXStringRelease(reference_string);
		reference_string=0;

		size_t byte_count=0;
		int ret = NXStringGetBytesSize(&byte_count, string, charset, nx_string_get_bytes_size_null_terminate);
		if(ret == NErr_DirectPointer)
		{
			if (owned)
			{
				free(ptr);
				ptr=0;
				length=0;
				malloc_size=0;
			}
			ret = NXStringGetBytesDirect((const void **)&ptr, &length, string, charset, nx_string_get_bytes_size_null_terminate);
			reference_string = NXStringRetain(string);
			owned=false;
		}
		else if (ret == NErr_Success)
		{
			if (owned)
			{
				if (byte_count > malloc_size)
				{
					ptr = (char *)realloc(ptr, byte_count);
					malloc_size = byte_count;
				}
			}
			else
			{
				/* not owned. need to allocate */
				ptr = (char *)malloc(byte_count);
				malloc_size = byte_count;
				owned=true;
			}

			if (ptr)
			{
				ret = NXStringGetBytes(&length, string, ptr, byte_count, charset, nx_string_get_bytes_size_null_terminate);
			}
			else
			{
				return NErr_OutOfMemory;
			}
		}
		else
		{
			Clear();
		}
		return ret;
	}

	int Set(nx_uri_t filename)
	{
		int ret;
		nx_string_t string;
		ret = NXURIGetNXString(&string, filename);
		if (ret == NErr_Success)
		{
			ret = Set(string);
			NXStringRelease(string);
		}
		else
		{
			Clear();
			// failed! we need to clean up
		}
		return ret;
	}

	operator const char *() const
	{
		if (length)
			return ptr;
		else
			return 0;
	}

	/* this one will never return a NULL, always a valid string */
	const char *GetValidString() const
	{
		if (length)
			return ptr;
		else
			return "";
	}

	/* the Clear function clears the string but doesn't deallocate memory */
	void Clear()
	{
		if (!owned)
			ptr=0;
		length=0;
	
		if (reference_string)
			NXStringRelease(reference_string);
		reference_string=0;
	}

private:
	void Init()
	{
		ptr=0;
		length=0;
		owned=false;
		reference_string=0;
		malloc_size=0;
	}
	char *ptr;
	size_t length;
	size_t malloc_size;
	bool owned;
	nx_string_t reference_string;
};

typedef AutoCharNX<nx_charset_utf8> AutoCharUTF8;
#define AutoCharPrintfUTF8(x) (AutoCharUTF8(x).GetValidString())
class AutoCharNative
{
public:
};

class AutoFilename
{
public:
};

