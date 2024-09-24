#ifndef _GAYSTRING_H_
#define _GAYSTRING_H_

#include <windows.h>

class GayString
{
public:
	GayString(const char *initial=NULL);
	~GayString();
	void Set(const char *value);
	char *Get();

	void Append(const char *append);
	void Grow(size_t newsize);
	void Compact();
	size_t Length();

private:
	char *m_buf;
	size_t m_alloc;
	size_t len;
};

class GayStringW
{
public:
	GayStringW(const wchar_t *initial=NULL);
	~GayStringW();
	void Set(const wchar_t *value);
	const wchar_t *Get();

	void Append(const wchar_t *append);
	void Grow(size_t newsize);
	void Compact();
	size_t Length();

private:
	wchar_t *m_buf;
	size_t m_alloc;
	size_t len;
};

#endif//_GAYSTRING_H_