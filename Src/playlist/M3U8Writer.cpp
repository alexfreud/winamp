#include "M3U8Writer.h"
#include "../nu/AutoChar.h"
#include <bfc/platform/types.h>

M3U8Writer::M3U8Writer() : fp(0)
{
}

int M3U8Writer::Open(const wchar_t *filename)
{
	fp = _wfopen(filename, L"wt");
	if (!fp)
		return 0;
  
	fputs( "\xEF\xBB\xBF", fp );
	fprintf(fp,"#EXTM3U\n");

	return 1;
}

void M3U8Writer::Write(const wchar_t *filename)
{
	fwprintf(fp,L"%s\n", filename);
}

void M3U8Writer::Write(const wchar_t *filename, const wchar_t *title, int length)
{
	fprintf( fp, "#EXTINF:%d,%s\n%s\n", length, (char *) AutoChar( title, CP_UTF8 ), (char *) AutoChar( filename, CP_UTF8 ) );
}

void M3U8Writer::Write( const wchar_t *p_filename, const wchar_t *p_title, const wchar_t *p_extended_infos, int p_length )
{
	fprintf( fp, "#EXTINF:%d %s,%s\n%s\n", p_length, (char *)AutoChar( p_extended_infos, CP_UTF8 ), (char *)AutoChar( p_title, CP_UTF8 ), (char *)AutoChar( p_filename, CP_UTF8 ) );
}

void M3U8Writer::Close()
{
	fclose(fp);
}