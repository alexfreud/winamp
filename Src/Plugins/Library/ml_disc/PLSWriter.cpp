#include "PLSWriter.h"
#include <windows.h>

PLSWriter::PLSWriter() : numEntries(0), entryUsed(0)
{
	memset(plsFile, 0, sizeof(plsFile));
}

void PLSWriter::Open(char *filename)
{
	lstrcpynA(plsFile, filename, MAX_PATH);
}

void PLSWriter::SetFilename(char *filename)
{
	char fieldbuf[32] = {0};
	BeforeSet();
	wsprintfA(fieldbuf,"File%u",numEntries);
	WritePrivateProfileStringA("playlist",fieldbuf,filename,plsFile);
}

void PLSWriter::SetTitle(char *title)
{
	char fieldbuf[32] = {0};
	BeforeSet();
	wsprintfA(fieldbuf,"Title%u",numEntries);
	WritePrivateProfileStringA("playlist",fieldbuf,title,plsFile);
}

void PLSWriter::SetLength(int length)
{
	char fieldbuf[32] = {0};
	char lenStr[32] = {0};
	BeforeSet();
	wsprintfA(fieldbuf,"Length%u",numEntries);
	wsprintfA(lenStr,"%d", length);
	WritePrivateProfileStringA("playlist",fieldbuf,lenStr,plsFile);
}

void PLSWriter::BeforeSet()
{
	if (!entryUsed)
	{
		entryUsed=1;
		numEntries++;
	}
}

void PLSWriter::Next()
{
	entryUsed=0;
}

void PLSWriter::Close()
{
	if (numEntries)
	{
		char temp[32] = {0};
		wsprintfA(temp,"%u",numEntries);
		WritePrivateProfileStringA("playlist","NumberOfEntries",temp,plsFile);
		WritePrivateProfileStringA("playlist","Version","2",plsFile);
	}
	numEntries=0;
	entryUsed=0;
}