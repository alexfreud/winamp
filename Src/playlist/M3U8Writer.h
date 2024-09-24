#ifndef NULLSOFT_M3U8WRITERH
#define NULLSOFT_M3U8WRITERH

#include <stdio.h>
#include "PlaylistWriter.h"
class M3U8Writer : public PlaylistWriter
{
public:
	M3U8Writer();

	int  Open( const wchar_t *filename ) override;
	void Write( const wchar_t *filename ) override;
	void Write( const wchar_t *filename, const wchar_t *title, int length ) override;
	void Write( const wchar_t *p_filename, const wchar_t *p_title, const wchar_t* p_extended_infos, int p_length ) override;
	void Close() override;

private:
	FILE *fp;
};

#endif