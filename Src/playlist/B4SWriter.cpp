#include "B4SWriter.h"

/*
TODO: escape XML shit
*/

B4SWriter::B4SWriter() : fp(0)
{
}

int B4SWriter::Open(const wchar_t *filename)
{
	fp = _wfopen(filename, L"wt");
	if (!fp) 
		return 0;

	fwprintf(fp, L"<playlist>\n");

	return 1;
}

void B4SWriter::Write(const wchar_t *filename)
{
	fwprintf(fp, L"<entry playstring=\"%s\"/>\n", filename);
}

void B4SWriter::Write(const wchar_t *filename, const wchar_t *title, int length)
{
	fwprintf(fp, L"<entry playstring=\"%s\">\n", filename);
	fwprintf(fp, L"<name>%s</name>\n", title);
	fwprintf(fp, L"<length>%d</length>\n", length);
	fwprintf(fp, L"</entry>\n");
}

void B4SWriter::Close()
{
	fputs("</playlist>", fp);
	fclose(fp);

}