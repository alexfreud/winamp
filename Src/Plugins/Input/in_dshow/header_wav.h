#ifndef NULLSOFT_IN_DSHOW_HEADER_WAV_H
#define NULLSOFT_IN_DSHOW_HEADER_WAV_H

#include <windows.h>

#include "Header.h"

class HeaderWav : public Header
{
public:
	HeaderWav(bool bAllowHttpConnection = false); // bAllowHttp will allow an http connection to read header info
	int getInfos(const wchar_t *filename, bool checkMetadata=false);
	unsigned __int32 read_dword() { unsigned __int32 v=0; myfread(&v,sizeof(v),1); return v; }

private:
	HANDLE fh;

	bool bAllowHttp;
	size_t myfread( void *buffer, size_t size, size_t count);
	int myfclose();
	int myfseek(long offset, DWORD origin);
};

#endif