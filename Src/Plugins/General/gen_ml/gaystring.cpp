#include "gaystring.h"

GayString::GayString(const char *initial)
{
	len = 0;
	m_buf = NULL;
	m_alloc = 0;
	if (initial && *initial) Set(initial);
}

GayString::~GayString()
{
	if ( m_buf )
		free( m_buf );
}

void GayString::Set(const char *value)
{
	if (!value) value="";
	len = strlen(value);
	Grow(len + 1);
	strncpy(m_buf, value, len);
	m_buf[len] = 0;
}

char *GayString::Get() { return m_buf ? m_buf : (char*)""; }

void GayString::Append(const char *append)
{
	size_t oldsize = len;
	len += strlen(append);
	Grow(len + 1);
	strncpy(m_buf + oldsize, append, len - oldsize);
	m_buf[len] = 0;
}

void GayString::Grow(size_t newsize)
{
	if (m_alloc < newsize)
	{
		size_t old_m_alloc = m_alloc;
		m_alloc = newsize + 32;
		char *new_m_buf = (char*)realloc(m_buf, m_alloc*sizeof(char));
		if (!new_m_buf)
		{
			new_m_buf = (char*)malloc(m_alloc*sizeof(char));
			if (!new_m_buf)
			{
				m_alloc = old_m_alloc;
			}
			else
			{
				memcpy(new_m_buf, m_buf, old_m_alloc*sizeof(char));
				free(m_buf);
				m_buf = new_m_buf;
			}
		}
		else m_buf = new_m_buf;
	}
}

void GayString::Compact()
{
	if (m_buf)
	{
		size_t old_m_alloc = m_alloc;
		//m_alloc = strlen(m_buf) + 1;
		m_alloc = len + 1;
		char* new_m_buf = (char*)realloc(m_buf, m_alloc*sizeof(char));
		if (!new_m_buf)
		{
			new_m_buf = (char*)malloc(m_alloc*sizeof(char));
			if (!new_m_buf)
			{
				if (m_alloc > old_m_alloc)
					m_alloc = old_m_alloc;
			}
			else
			{
				if (m_alloc > old_m_alloc)
					m_alloc = old_m_alloc;

				memcpy(new_m_buf, m_buf, old_m_alloc*sizeof(char));
				free(m_buf);
				m_buf = new_m_buf;
			}
		}
		else m_buf = new_m_buf;
	}
}

size_t GayString::Length() { return len; }

/* */
GayStringW::GayStringW(const wchar_t *initial)
{
	len = 0;
	m_buf = NULL;
	m_alloc = 0;
	if (initial && *initial) Set(initial);
}

GayStringW::~GayStringW()
{
	free(m_buf);
}

void GayStringW::Set(const wchar_t *value)
{
	if (!value) value=L"";
	len = wcslen(value);
	Grow(len + 1);
	wcsncpy(m_buf, value, len);
	m_buf[len] = 0;
}

const wchar_t *GayStringW::Get() { return m_buf ? m_buf : L""; }

void GayStringW::Append(const wchar_t *append)
{
	size_t oldsize = len;
	len += wcslen(append);
	Grow(len + 1);
	wcsncpy(m_buf + oldsize, append, len - oldsize);
	m_buf[len] = 0;
}

void GayStringW::Grow(size_t newsize)
{
	if (m_alloc < newsize)
	{
		size_t old_m_alloc = m_alloc;
		m_alloc = newsize + 32;
		wchar_t *new_m_buf = (wchar_t*)realloc(m_buf, m_alloc*sizeof(wchar_t));
		if (!new_m_buf)
		{
			new_m_buf = (wchar_t*)malloc(m_alloc*sizeof(wchar_t));
			if (!new_m_buf)
			{
				m_alloc = old_m_alloc;
			}
			else
			{
				memcpy(new_m_buf, m_buf, old_m_alloc*sizeof(wchar_t));
				free(m_buf);
				m_buf = new_m_buf;
			}
		}
		else m_buf = new_m_buf;
	}
}

void GayStringW::Compact()
{
	if (m_buf)
	{
		size_t old_m_alloc = m_alloc;
		m_alloc = len + 1;
		wchar_t* new_m_buf = (wchar_t*)realloc(m_buf, m_alloc*sizeof(wchar_t));
		if (!new_m_buf)
		{
			new_m_buf = (wchar_t*)malloc(m_alloc*sizeof(wchar_t));
			if (!new_m_buf)
			{
				if (m_alloc > old_m_alloc)
					m_alloc = old_m_alloc;
			}
			else
			{
				if (m_alloc > old_m_alloc)
					m_alloc = old_m_alloc;

				memcpy(new_m_buf, m_buf, old_m_alloc*sizeof(wchar_t));
				free(m_buf);
				m_buf = new_m_buf;
			}
		}
		else m_buf = new_m_buf;
	}
}

size_t GayStringW::Length() { return len; }