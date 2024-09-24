#include <precomp.h>

#include "skinfont.h"

#include <bfc/wasabi_std.h>

SkinFont::SkinFont()
{}

SkinFont::~SkinFont()
{
	if (!tempFn.isempty())
	{
#ifdef WIN32
		RemoveFontResourceW(tempFn);
#else
		DebugString( "portme -- SkinFont::~SkinFont\n" );
#endif
		UNLINK(tempFn);
	}
}

int SkinFont::setXmlOption(const wchar_t *paramname, const wchar_t *strvalue)
{
	return 0;
}

void SkinFont::installFont(const wchar_t *filename, const wchar_t *path)
{
	OSFILETYPE in, out;
	StringPathCombine temp(path, filename);
	in = WFOPEN(temp, WF_READONLY_BINARY);
	if (in == OPEN_FAILED) return ;
	int len = (int)FGETSIZE(in);
	MemBlock<char> m(len);
	FREAD(m.getMemory(), len, 1, in);
	tempFn = TMPNAM(NULL);
	out = WFOPEN(tempFn, WF_WRITE_BINARY);
	ASSERT(out != OPEN_FAILED);
	FWRITE(m.getMemory(), len, 1, out);
	FCLOSE(out);
	FCLOSE(in);
#ifdef WIN32
	AddFontResourceW(tempFn);
#else
	DebugString( "portme -- SkinFont::installFont\n" );
#endif
}