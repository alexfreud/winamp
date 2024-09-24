#ifndef _HEADER_AVI_H
#define _HEADER_AVI_H

#include <windows.h>

#include "Header.h"

class HeaderAvi : public Header
{
public:
	HeaderAvi(bool bAllowHttpConnection = false); // bAllowHttp will allow an http connection to read header info
~HeaderAvi();
	int getInfos(const wchar_t *filename, bool checkMetadata=false);
	int read_dword() { int v=0; myfread(&v,sizeof(v),1); return v; }

private:
	HANDLE fh;

	bool bAllowHttp;
	size_t myfread( void *buffer, size_t size, size_t count);
	int myfclose();
	int myfseek(long offset, DWORD origin);
};

#endif
