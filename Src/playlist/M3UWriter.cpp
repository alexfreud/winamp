#include "M3UWriter.h"
#include "../nu/AutoChar.h"

M3UWriter::~M3UWriter()
{
	Close();
}

int M3UWriter::Open( const wchar_t *filename )
{
	if ( fp != NULL )
		return 0;

	fp = _wfopen( filename, L"wt" );
	if ( !fp )
		return 0;

	fprintf( fp, "#EXTM3U\n" );

	return 1;
}

void M3UWriter::Write( const wchar_t *filename )
{
	if ( fp != NULL )
		fprintf( fp, "%s\n", (char *)AutoChar( filename ) );
}

void M3UWriter::Write( const wchar_t *filename, const wchar_t *title, int length )
{
	if ( fp != NULL )
		fprintf( fp, "#EXTINF:%d,%s\n%s\n", length, (char *)AutoChar( title ), (char *)AutoChar( filename ) );
}

void M3UWriter::Close()
{
	if ( fp != NULL )
	{
		fclose( fp );
		fp = NULL;
	}
}