#include "PLSWriter.h"
#include "../nu/AutoChar.h"

PLSWriter::PLSWriter() : fp(0), numEntries(0)
{
}

int PLSWriter::Open(const wchar_t *filename)
{
	fp = _wfopen(filename, L"wt");
	if (!fp) 
		return 0;
  
	fprintf(fp, "[playlist]\r\n");

	return 1;
}

void PLSWriter::Write(const wchar_t *filename)
{
	fwprintf(fp, L"File%d=%s\r\n", (int)++numEntries, filename);
}

void PLSWriter::Write(const wchar_t *filename, const wchar_t *title, int length)
{
	Write(filename);
		fwprintf(fp, L"Title%d=%s\r\n", (int)numEntries, title);
				fprintf(fp, "Length%d=%d\r\n", (int)numEntries, length);
}

void PLSWriter::Close()
{
	fprintf(fp, "NumberOfEntries=%d\r\n", (int)numEntries);
	fprintf(fp, "Version=2\r\n");
	fclose(fp);
}