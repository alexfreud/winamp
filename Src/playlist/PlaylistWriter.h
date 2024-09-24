#ifndef NULLSOFT_PLAYLIST_PLAYLISTWRITER_H
#define NULLSOFT_PLAYLIST_PLAYLISTWRITER_H

// probably not the final interface, so we won't dispatch it yet

class PlaylistWriter
{
public:
	virtual ~PlaylistWriter()                                         {}
	virtual int Open( const wchar_t *filename ) = 0;
	virtual void Write( const wchar_t *filename ) = 0;
	virtual void Write( const wchar_t *filename, const wchar_t *title, int length ) = 0;
	virtual void Write( const wchar_t *filename, const wchar_t *title, const wchar_t *p_extended_infos, int length ) = 0;
	virtual void Close() = 0;
};

#endif