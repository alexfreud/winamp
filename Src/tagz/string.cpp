#include "string.h"
#include <windows.h>
#include <strsafe.h>
using namespace tagz_;

string::string()
{
	data = 0;
	size = 0;
	used = 0;
}

void string::AddDBChar(LPTSTR c)
{
	LPTSTR end = CharNext(c);
	while (c != end)
		AddChar(*c++);
}

void string::AddChar(TCHAR c)
{
	if (!data)
	{
		size = 512;
		data = (LPTSTR)calloc(size, sizeof(TCHAR));
		if (!data) size = 0;
		used = 0;
	}
	else if (size == used)
	{
		size <<= 1;
		LPTSTR newData = (LPTSTR)realloc((LPTSTR)data, size * sizeof(TCHAR));
		if (!newData)
		{
			free(data);
			data = (LPTSTR)calloc(size, sizeof(TCHAR));
			if (!data) size = 0;
			used = 0;
		}
		else
		{
			data = newData;
			memset(data+used, 0, (size - used) * sizeof(TCHAR));
		}
	}
	if (data) 
		data[used++] = c;
}

void string::AddInt(int i)
{
	TCHAR simpleInt[16] = {0};
	StringCchPrintf(simpleInt, 16, TEXT("%i"), i);
	AddString(simpleInt);
}

void string::AddString(LPCTSTR z)
{
	while (z && *z)
	{
		AddChar(*z);
		z++;
	}
}

void string::AddString(string & s)
{
	AddString(s.Peek());
}

string::~string()
{
	if (data) free(data);
}

LPTSTR string::GetBuf()
{
	if (!data)
		return NULL;

	LPTSTR r = (LPTSTR)realloc(data, (used + 1) * sizeof(TCHAR));
	if (!r)
	{
		free(data);
		data = 0;
		size = used + 1;
		r = (LPTSTR)calloc((used + 1), sizeof(TCHAR));
		if (!r) size = 0;
	}
	if (r) r[used] = 0;
	data = 0;
	return r;
}

TCHAR string::operator[](size_t i)
{
	if (!data || i >= used) 
		return 0;
	else 
		return data[i];
}

size_t string::Len()
{
	return data ? used : 0;
}

void string::Reset()
{
	if (data)
	{
		free(data);
		data = 0;
	}
	size = 0;
	used = 0;
}

LPCTSTR string::Peek()
{
	AddChar(0);
	used--;
	return data;
}