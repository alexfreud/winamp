#pragma once
#include <bfc/platform/types.h>
#include "iPodDB.h"
/* iPod shuffle Shadow Database code */

// iTunesSD (iPod Shuffle) Database Classes
class iTunesSD_Song;
class iTunesSD2_Song;


class iTunesSD1 
{
public:
	iTunesSD1();
	~iTunesSD1();

	long write(const iPod_mhlt::mhit_map_t *mhit, unsigned char * data, const unsigned long datasize);
};

class iTunesSD2
{
public:
	long write(const iPod_mhlt *mhit, const iPod_mhlp *playlists, unsigned char * data, const unsigned long datasize);
};

#define SDSONG_FILENAME_LEN		260


class iTunesSD_Song
{
public:
	iTunesSD_Song(const iPod_mhit *mhit);
	enum FileType
	{
		MP3		= 0x01,
		AAC		= 0x02,
		WAV		= 0x04
	};

	enum PlayFlags
	{
		UNKNOWN			= 0x000001,		// Might do something special, but nothing has been observed so far
		BOOKMARKABLE	= 0x000100,		// Any song that has flag is bookmarked
		SHUFFLE			= 0x010000		// Only songs that have this flag are available in shuffle playback mode
	};


	

	long write(unsigned char * data, const unsigned long datasize);

	void SetFilename(const wchar_t *filename);
	void SetStartTime(const double milliseconds)		{ starttime = (unsigned int)(milliseconds / 256.0); }
	void SetStopTime(const double milliseconds)			{ stoptime  = (unsigned int)(milliseconds / 256.0); }
	void SetVolume(const int percent);

	// These are also only 3 byte values
	uint32_t size_total;
	uint32_t starttime;
	uint32_t stoptime;
	uint32_t volume;		// -100% = 0x0, 0% = 0x64 (100), 100% = 0xc8 (200)
	uint32_t filetype;		// 0x01 = MP3, 0x02 = AAC, 0x04 = WAV
	wchar_t filename[SDSONG_FILENAME_LEN + 1];		// Equal to Windows' MAX_PATH, plus the trailing NULL (261 wide chars = 522 bytes)
	unsigned int playflags;
};

class iTunesSD2_Song
{
public:
	static long write(const iPod_mhit *mhit, unsigned char * data, const unsigned long datasize);
	static uint32_t header_size;
};

class iTunesSD2_Playlist
{
public:
	static long write(const iPod_mhyp *master_playlist, const iPod_mhyp *playlist, unsigned char * data, const unsigned long datasize);
};
