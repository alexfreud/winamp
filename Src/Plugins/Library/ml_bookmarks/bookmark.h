#ifndef NULLSOFT_BOOKMARKH
#define NULLSOFT_BOOKMARKH

#include <stdio.h>

class BookmarkWriter
{
public:
	BookmarkWriter();
	void New(const wchar_t *filename);
	void Open(const wchar_t *filename);
	void Write(const char *filename, const char *title);
	void Close();
private:
	FILE *fp;
};

#endif