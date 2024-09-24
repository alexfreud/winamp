#ifndef NULLSOFT_PLAYLIST_B4SWRITER_H
#define NULLSOFT_PLAYLIST_B4SWRITER_H

#include <stdio.h>
#include "PlaylistWriter.h"

class B4SWriter : public PlaylistWriter
{
public:
	B4SWriter();

	int  Open( const wchar_t *filename ) override;
	void Write( const wchar_t *filename ) override;
	void Write( const wchar_t *filename, const wchar_t *title, int length ) override;
	void Write( const wchar_t *p_filename, const wchar_t *p_title, const wchar_t *p_extended_infos, int p_length ) override
	{};

	void Close() override;
	
private:
	FILE *fp;
};

#endif