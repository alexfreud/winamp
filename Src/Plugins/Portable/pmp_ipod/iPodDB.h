/*
 *
 *
 * Copyright (c) 2004 Samuel Wood (sam.wood@gmail.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *
 */


// For more information on how all this stuff works, see:
// http://www.ipodlinux.org/ITunesDB


// iPodDB.h: interface for the iPod classes.
//
//////////////////////////////////////////////////////////////////////

#ifndef __IPODDB_H__
#define __IPODDB_H__

#pragma once

#pragma warning( disable : 4786)

#include <algorithm>
#include <windows.h>
#include <bfc/platform/types.h>
#include <map>
#include <vector>

#ifdef _DEBUG
#undef ASSERT
#define ASSERT(x) assert(x)
#else
#define ASSERT(x) {}
#endif



// mhod types
#define MHOD_TITLE			1
#define MHOD_LOCATION		2
#define MHOD_ALBUM			3
#define MHOD_ARTIST			4
#define MHOD_GENRE			5
#define MHOD_FILETYPE		6
#define MHOD_EQSETTING		7
#define MHOD_COMMENT		8
#define MHOD_CATEGORY		9		// iTunes Music Store Podcast category
#define MHOD_COMPOSER		12
#define MHOD_GROUPING		13
#define MHOD_DESCRIPTION	14		// Podcast show notes text - accessible via the center iPod button
#define MHOD_ENCLOSUREURL	15		// Used by iTunes 4.9 for a Podcast's original enclosure URL
#define MHOD_RSSFEEDURL		16		// Used by iTunes 4.9 for a Podcast's RSS 2.0 feed URL
#define MHOD_CHAPTER		17		// M4A-style tagged data that is used to support subsongs/chapters
#define MHOD_SUBTITLE		18	
#define MHOD_SHOW       19
#define MHOD_EPISODE    20
#define MHOD_TVNETWORK  21
#define MHOD_ALBUMARTIST 22
#define MHOD_ARTIST_SORT 23
#define MHOD_TITLE_SORT 27
#define MHOD_ALBUM_SORT 28
#define MHOD_ALBUMARTIST_SORT 29
#define MHOD_COMPOSER_SORT 30
#define MHOD_SHOW_SORT 31
#define MHOD_SPLPREF		50
#define MHOD_SPLDATA		51
#define MHOD_LIBRARY		52		// Found in the default hidden playlist
#define MHOD_LIBRARY_LETTER 53 // letter jump table
#define MHOD_PLAYLIST		100
#define MHOD_ALBUMLIST_ALBUM 200
#define MHOD_ALBUMLIST_ARTIST 201
#define MHOD_ALBUMLIST_ARTIST_SORT 202
#define MHOD_ALBUMLIST_PODCASTURL 203
#define MHOD_ALBUMLIST_SHOW 204


// Equalizer defines
#define EQ_NONE				-1
#define EQ_ACOUSTIC			100
#define EQ_BASSBOOSTER		101
#define EQ_BASSREDUCER		102
#define EQ_CLASSICAL		103
#define EQ_DANCE			104
#define EQ_DEEP				105
#define EQ_ELECTRONIC		106
#define EQ_FLAT				107
#define EQ_HIPHOP			108
#define EQ_JAZZ				109
#define EQ_LATIN			110
#define EQ_LOUDNESS			111
#define EQ_LOUNGE			112
#define EQ_PIANO			113
#define EQ_POP				114
#define EQ_RNB				115
#define EQ_ROCK				116
#define EQ_SMALLSPEAKERS	117
#define EQ_SPOKENWORD		118
#define EQ_TREBLEBOOSTER	119
#define EQ_TREBLEREDUCER	120
#define EQ_VOCALBOOSTER		121



// Smart Playlist stuff
#define SPLMATCH_AND	0		// AND rule - all of the rules must be true in order for the combined rule to be applied
#define SPLMATCH_OR		1		// OR rule

// Limit Types.. like limit playlist to 100 minutes or to 100 songs
#define LIMITTYPE_MINUTES	0x01
#define LIMITTYPE_MB		0x02
#define LIMITTYPE_SONGS		0x03
#define LIMITTYPE_HOURS		0x04
#define LIMITTYPE_GB		0x05

// Limit Sorts.. Like which songs to pick when using a limit type
// Special note: the values for LIMITSORT_LEAST_RECENTLY_ADDED, LIMITSORT_LEAST_OFTEN_PLAYED,
//		LIMITSORT_LEAST_RECENTLY_PLAYED, and LIMITSORT_LOWEST_RATING are really 0x10, 0x14,
//		0x15, 0x17, with the 'limitsort_opposite' flag set.  This is the same value as the
//		"positive" value (i.e. LIMITSORT_LEAST_RECENTLY_ADDED), and is really very terribly
//		awfully weird, so we map the values to iPodDB specific values with the high bit set.
//
//		On writing, we check the high bit and write the limitsort_opposite from that. That
//		way, we don't have to deal with programs using the class needing to set the wrong
//		limit and then make it into the "opposite", which would be frickin' annoying.
#define LIMITSORT_RANDOM					0x02
#define LIMITSORT_SONG_NAME					0x03
#define LIMITSORT_ALBUM						0x04
#define LIMITSORT_ARTIST					0x05
#define LIMITSORT_GENRE						0x07
#define LIMITSORT_MOST_RECENTLY_ADDED		0x10
#define LIMITSORT_COMPOSER					0x12		// Not used by iTunes, but inferred from the Type 52 MHOD's Composer type
#define LIMITSORT_LEAST_RECENTLY_ADDED		0x80000010  // See note above
#define LIMITSORT_MOST_OFTEN_PLAYED			0x14
#define LIMITSORT_LEAST_OFTEN_PLAYED		0x80000014  // See note above
#define LIMITSORT_MOST_RECENTLY_PLAYED		0x15
#define LIMITSORT_LEAST_RECENTLY_PLAYED		0x80000015  // See note above
#define LIMITSORT_HIGHEST_RATING			0x17
#define LIMITSORT_LOWEST_RATING				0x80000017  // See note above

// Smartlist Actions - Used in the rules.
/*
 really this is a bitmapped field...
 high byte
 bit 0 = "string" values if set, "int" values if not set
 bit 1 = "not", or to negate the check.
 lower 2 bytes
 bit 0 = simple "IS" query
 bit 1 = contains
 bit 2 = begins with
 bit 3 = ends with
 bit 4 = greater than
 bit 5 = unknown, but probably greater than or equal to
 bit 6 = less than
 bit 7 = unknown, but probably less than or equal to
 bit 8 = a range selection
 bit 9 = "in the last"
*/
#define SPLACTION_IS_INT				0x00000001		// Also called "Is Set" in iTunes
#define SPLACTION_IS_GREATER_THAN		0x00000010		// Also called "Is After" in iTunes
#define SPLACTION_IS_LESS_THAN			0x00000040		// Also called "Is Before" in iTunes
#define SPLACTION_IS_IN_THE_RANGE		0x00000100
#define SPLACTION_IS_IN_THE_LAST		0x00000200
#define SPLACTION_BINARY_AND			0x00000400

#define SPLACTION_IS_STRING				0x01000001
#define SPLACTION_CONTAINS				0x01000002
#define SPLACTION_STARTS_WITH			0x01000004
#define SPLACTION_ENDS_WITH				0x01000008

#define SPLACTION_IS_NOT_INT			0x02000001		// Also called "Is Not Set" in iTunes
#define SPLACTION_IS_NOT_GREATER_THAN	0x02000010		// Note: Not available in iTunes
#define SPLACTION_IS_NOT_LESS_THAN		0x02000040		// Note: Not available in iTunes
#define SPLACTION_IS_NOT_IN_THE_RANGE	0x02000100		// Note: Not available in iTunes
#define SPLACTION_IS_NOT_IN_THE_LAST	0x02000200
#define SPLACTION_UNKNOWN2			0x02000800

#define SPLACTION_IS_NOT				0x03000001
#define SPLACTION_DOES_NOT_CONTAIN		0x03000002
#define	SPLACTION_DOES_NOT_START_WITH	0x03000004		// Note: Not available in iTunes
#define	SPLACTION_DOES_NOT_END_WITH		0x03000008		// Note: Not available in iTunes


// these are to pass to AddRule() when you need a unit for the two "in the last" action types
// Or, in theory, you can use any time range... iTunes might not like it, but the iPod might.
#define SPLACTION_LAST_DAYS_VALUE		86400		// number of seconds in 24 hours
#define SPLACTION_LAST_WEEKS_VALUE		604800		// number of seconds in 7 days
#define SPLACTION_LAST_MONTHS_VALUE		2628000		// number of seconds in 30.4167 days ~= 1 month

// Hey, why limit ourselves to what iTunes can do? If the iPod can deal with it, excellent!
#define SPLACTION_LAST_SECONDS_RULE		1			// one second
#define SPLACTION_LAST_HOURS_VALUE		3600		// number of seconds in 1 hour
#define SPLACTION_LAST_MINUTES_VALUE	60			// number of seconds in 1 minute
#define SPLACTION_LAST_YEARS_VALUE		31536000 	// number of seconds in 365 days

// fun ones.. Near as I can tell, all of these work. It's open like that. :)
#define SPLACTION_LAST_LUNARCYCLE_VALUE	2551443		// a "lunar cycle" is the time it takes the moon to circle the earth
#define SPLACTION_LAST_SIDEREAL_DAY		86164		// a "sidereal day" is time in one revolution of earth on its axis
#define SPLACTION_LAST_SWATCH_BEAT		86			// a "swatch beat" is 1/1000th of a day.. search for "internet time" on google
#define SPLACTION_LAST_MOMENT			90			// a "moment" is 1/40th of an hour, or 1.5 minutes
#define SPLACTION_LAST_OSTENT			600			// an "ostent" is 1/10th of an hour, or 6 minutes
#define SPLACTION_LAST_FORTNIGHT		1209600 	// a "fortnight" is 14 days
#define SPLACTION_LAST_VINAL			1728000 	// a "vinal" is 20 days
#define SPLACTION_LAST_QUARTER			7889231		// a "quarter" is a quarter year
#define SPLACTION_LAST_SOLAR_YEAR       31556926 	// a "solar year" is the time it takes the earth to go around the sun
#define SPLACTION_LAST_SIDEREAL_YEAR 	31558150	// a "sidereal year" is the time it takes the earth to reach the same point in space again, compared to the stars


// Smartlist fields - Used for rules.
#define SPLFIELD_SONG_NAME		0x02	// String
#define SPLFIELD_ALBUM			0x03	// String
#define SPLFIELD_ARTIST			0x04	// String
#define SPLFIELD_BITRATE		0x05	// Int	(e.g. from/to = 128)
#define SPLFIELD_SAMPLE_RATE	0x06	// Int  (e.g. from/to = 44100)
#define SPLFIELD_YEAR			0x07	// Int  (e.g. from/to = 2004)
#define SPLFIELD_GENRE			0x08	// String
#define SPLFIELD_KIND			0x09	// String
#define SPLFIELD_DATE_MODIFIED	0x0a	// Int/Mac Timestamp (e.g. from/to = bcf93280 == is before 6/19/2004)
#define SPLFIELD_TRACKNUMBER	0x0b	// Int  (e.g. from = 1, to = 2)
#define SPLFIELD_SIZE			0x0c	// Int  (e.g. from/to = 0x00600000 for 6MB)
#define SPLFIELD_TIME			0x0d	// Int  (e.g. from/to = 83999 for 1:23/83 seconds)
#define SPLFIELD_COMMENT		0x0e	// String
#define SPLFIELD_DATE_ADDED		0x10	// Int/Mac Timestamp (e.g. from/to = bcfa83ff == is after 6/19/2004)
#define SPLFIELD_COMPOSER		0x12	// String
#define SPLFIELD_PLAYCOUNT		0x16	// Int  (e.g. from/to = 1)
#define SPLFIELD_LAST_PLAYED	0x17	// Int/Mac Timestamp (e.g. from = bcfa83ff (6/19/2004), to = 0xbcfbd57f (6/20/2004))
#define SPLFIELD_DISC_NUMBER	0x18	// Int  (e.g. from/to = 1)
#define SPLFIELD_RATING			0x19	// Int/Stars Rating  (e.g. from/to = 60 (3 stars))
#define SPLFIELD_COMPILATION	0x1f	// Int  (e.g. is set -> SPLACTION_IS_INT/from=1, is not set -> SPLACTION_IS_NOT_INT/from=1)
#define SPLFIELD_BPM			0x23	// Int  (e.g. from/to = 60)
#define SPLFIELD_GROUPING		0x27	// String
#define SPLFIELD_PLAYLIST		0x28	// XXX - Unknown...not parsed correctly...from/to = 0xb6fbad5f for "Purchased Music".  Extra data after "to"...
#define SPLFIELD_VIDEO_KIND 0x3C // Logic Int (???)
#define SPLFIELD_TVSHOW 0x3E // Int
#define SPLFIELD_SEASON_NR 0x3F // Int
#define SPLFIELD_SKIPCOUNT 0x44 // Int
#define SPLFIELD_ALBUMARTIST 0x47 // string

#define SPLDATE_IDENTIFIER		0x2dae2dae2dae2dae


// MHOD Type 52 types
#define TYPE52_SONG_NAME		0x03
#define TYPE52_ARTIST			0x05
#define TYPE52_ALBUM			0x04
#define TYPE52_GENRE			0x07
#define TYPE52_COMPOSER			0x12

static const uint32_t FILETYPE_M4A=0x4d344120;
static const uint32_t FILETYPE_MP3=0x4d503320;
static const uint32_t FILETYPE_WAV=0x57415620;

// useful functions
time_t mactime_to_wintime (const unsigned long mactime);
unsigned long wintime_to_mactime (const __time64_t time);
char * UTF16_to_char(wchar_t * str, int length);


// Pre-declare iPod_* classes
class iPod_mhbd;
class iPod_mhsd;
class iPod_mhlt;
class iPod_mhit;
class iPod_mhlp;
class iPod_mhyp;
class iPod_slst;
class iPod_mhip;
class iPod_mhod;
class iPod_mqed;
class iPod_mhpo;
class iPod_pqed;
class iPod_mhla;

// Maximum string length that iTunes writes to the database
#define SPL_MAXSTRINGLENGTH 255


// a struct to hold smart playlist rules in mhods
struct SPLRule
{
	SPLRule() :
		field(0),
		action(0),
		length(0),
		fromvalue(0),
		fromdate(0),
		fromunits(0),
		tovalue(0),
		todate(0),
		tounits(0),
		unk1(0),
		unk2(0),
		unk3(0),
		unk4(0),
		unk5(0)
	{
		memset(string, 0, sizeof(string));
	}

	void SetString(const wchar_t *value)
	{
		if(value)
		{
			lstrcpynW(string, value, SPL_MAXSTRINGLENGTH);
			length = lstrlenW(string) * 2;
		}
		else
		{
			memset(string, 0, sizeof(string));
			length = 0;
		}
	}

	unsigned long field;
	unsigned long action;
	unsigned long length;
	wchar_t string[SPL_MAXSTRINGLENGTH + 1];

	// from and to are pretty stupid.. if it's a date type of field, then
	// value = 0x2dae2dae2dae2dae,
	// date = some number, like 2 or -2
	// units = unit in seconds, like 604800 = a week
	// but if this is actually some kind of integer comparison, like rating = 60 (3 stars)
	// value = the value we care about
	// date = 0
	// units = 1
	// So we leave these as they are, and will just deal with it in the rules functions.
	uint64_t fromvalue;
	int64_t fromdate;
	uint64_t fromunits;
	uint64_t tovalue;
	int64_t todate;
	uint64_t tounits;
	unsigned long unk1;
	unsigned long unk2;
	unsigned long unk3;
	unsigned long unk4;
	unsigned long unk5;
};


// PCEntry: Play Count struct for the entries in iPod_mhdp
struct PCEntry
{
	unsigned long playcount;
	unsigned long lastplayedtime;
	unsigned long bookmarktime;
	unsigned long stars;
	uint32_t unk1;
	uint32_t skipcount;
	uint32_t skippedtime;
};


/**************************************
       iTunesDB Database Layout

  MHBD (Database)
  |
  |-MHSD (Data Set)
  | |
  | |-MHLT (Track List)
  | | |
  | | |-MHIT (Track Item)
  | | | |
  | | | |-MHOD (Description Object)
  | | | |-MHOD
  | | | | ...
  | | |
  | | |-MHIT
  | | | |
  | | | |-MHOD
  | | | |-MHOD
  | | | | ...
  | | |
  | | |-...
  |
  |
  |-MHSD
  | |
  | |-MHLP (Playlists List)
  | | |
  | | |-MHYP (Playlist)
  | | | |
  | | | |-MHOD
  | | | |-MHIP (Playlist Item)
  | | | | ...
  | | |
  | | |-MHYP
  | | | |
  | | | |-MHOD
  | | | |-MHIP
  | | | | ...
  | | |
  | | |-...

**************************************/


// base class, not used directly
class iPodObj
{
public:
	iPodObj();
	virtual ~iPodObj();

	// parse function is required in all subclasses
	// feed it a iTunesDB, it creates an object hierarchy
	virtual long parse(const uint8_t *data) = 0;

	// write function is required too
	// feed it a buffer and the size of the buffer, it fills it with an iTunesDB
	// return value is size of the resulting iTunesDB
	// return of -1 means the buffer was too small
	virtual long write(uint8_t * data, const unsigned long datasize) = 0;

	unsigned long size_head;
	unsigned long size_total;
};


// MHBD: The database - parent of all items
class iPod_mhbd : public iPodObj
{
public:
	iPod_mhbd();
	virtual ~iPod_mhbd();

	virtual long parse(const uint8_t *data);
	virtual long write(uint8_t * data, const unsigned long datasize);
	virtual long write(uint8_t * data, const unsigned long datasize, uint8_t * fwid);

	uint32_t unk1;
	uint32_t dbversion;
	uint32_t children;
	uint64_t id;
	uint16_t platform;
	uint16_t language;
	uint64_t library_id;
	uint32_t unk80;
	uint32_t unk84;
	int32_t timezone; // in seconds
	uint16_t audio_language;
	uint16_t subtitle_language;
	uint16_t unk164;
	uint16_t unk166;
	uint16_t unk168;

	iPod_mhsd *mhsdsongs;
	iPod_mhsd *mhsdplaylists;
	iPod_mhsd *mhsdsmartplaylists;
};


// MHSD: List container - parent of MHLT or MHLP, child of MHBD
class iPod_mhsd : public iPodObj
{
public:
	iPod_mhsd();
	iPod_mhsd(int newindex);
	virtual ~iPod_mhsd();

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char * data, const unsigned long datasize) {return write(data,datasize,1);}
	virtual long write(unsigned char * data, const unsigned long datasize, int index);

	uint32_t index; // 1 = mhlt, 3 = mhlp, 2 = legacy mhlp, 4 = album list, 5 = mhlp_smart
	iPod_mhlt * mhlt;
	iPod_mhlp * mhlp;
	iPod_mhlp * mhlp_smart;
	iPod_mhla * mhla;
};

class iPod_mhia : public iPodObj
{
public:
	iPod_mhia();
	virtual ~iPod_mhia();

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char * data, const unsigned long datasize);

	uint16_t unk1;
	uint16_t albumid;
	uint64_t dbid;
	uint32_t type;

	std::vector<iPod_mhod*> mhod;
};

class ArtistAlbumPair
{
public:
	const wchar_t* artist;
	const wchar_t* album;
	ArtistAlbumPair() : artist(0), album(0) {}
	ArtistAlbumPair(const wchar_t* artist, const wchar_t* album) : artist(artist), album(album) {}
	/*bool operator < (const ArtistAlbumPair& that) const
	{
		int yy = _wcsicmp(artist, that.artist);
		if(yy) return yy < 0;
		return _wcsicmp(album, that.album) < 0;
	}*/
};

struct ArtistAlbumPairComparer
{
	int operator ()(const ArtistAlbumPair &a, const ArtistAlbumPair &b) const
	{
		int yy = _wcsicmp(a.artist, b.artist);
		if(yy) return yy;
		return _wcsicmp(a.album, b.album);
	}
};

class iPod_mhla : public iPodObj
{
public:
	iPod_mhla();
	virtual ~iPod_mhla();

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char * data, const unsigned long datasize);
	uint16_t GetAlbumId(const wchar_t* artist, const wchar_t* album);
	void ClearAlbumsList();

	//typedef std::map<ArtistAlbumPair, uint16_t> albums_map_t;
	typedef std::map<ArtistAlbumPair, uint16_t, ArtistAlbumPairComparer> albums_map_t;
	albums_map_t albums;
	uint16_t nextAlbumId;
};


// MHLT: song list container - parent of MHIT, child of MHSD
class iPod_mhlt : public iPodObj
{
public:
	typedef std::map<uint32_t, iPod_mhit*> mhit_map_t;
	//typedef std::map<unsigned long, iPod_mhit*> mhit_map_t;		// Map the unique mhit.id to a mhit object
	typedef mhit_map_t::value_type mhit_value_t;

	iPod_mhlt();
	virtual ~iPod_mhlt();

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char * data, const unsigned long datasize);

	const unsigned long GetChildrenCount() const	{ return mhit.size(); }

	// returns a pointer to the new iPod_mhit object in the track list, which you edit directly
	iPod_mhit *NewTrack();
	void AddTrack(iPod_mhit *new_track);

	// takes a position index, returns a pointer to the track itself, or NULL if the index isn't found.
	iPod_mhit *GetTrack(uint32_t index) const;

	// searches for a track based on the track's id number (mhit.id). returns mhit pointer, or NULL if the id isn't found.
	iPod_mhit * GetTrackByID(const unsigned long id);

	// couple of ways to delete a track
	bool DeleteTrack(const unsigned	long index);
	bool DeleteTrackByID(const unsigned long id);

	// clears out the tracklist
	bool ClearTracks(const bool clearMap = true);

	// the map of the tracks themselves
	mhit_map_t mhit;
	std::vector<uint32_t> mhit_indexer;

	uint32_t GetNextID();

private:
	volatile uint32_t next_mhit_id;
};


// MHIT: song item - parent of MHOD, child of MHLT
class iPod_mhit : public iPodObj
{
public:
	iPod_mhit();
	virtual ~iPod_mhit();

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char * data, const unsigned long datasize);

	const unsigned long GetChildrenCount() const	{ return(mhod.size()); }

	// will add a new mhod string to the mhit
	// optional: pass in a type to get an existing string, if there is one,
	//			 or a new one with the type filled in already, if there is not one
	iPod_mhod * AddString(const int type=0);

	// Find a string by type
	iPod_mhod * FindString(const unsigned long type) const;

	// deletes a string from the track
	// if more than one string of given type exists, all of that type will be deleted,
	//		to ensure consistency. Pointers to these strings will be invalid after this.
	// return val is how many strings were deleted
	unsigned long DeleteString(const unsigned long type);

	// Creates a copy of the mhit. The operator = is overloaded so you can
	// more easily copy mhit's.
	static void Duplicate(const iPod_mhit *src, iPod_mhit *dst);
	iPod_mhit& operator=(const iPod_mhit& src);

	int GetRating() { return stars/20; }
	void SetRating(int rating) { stars=rating*20; }

	int GetEQSetting();
	void SetEQSetting(int value);

	unsigned int GetFileTypeID(const wchar_t *filename);


	uint32_t id;
	uint32_t visible;				// 0x01 means the song shows up on the iPod, all other values means it is hidden
	uint32_t filetype;				// MP3 = 0x4d503320, M4A = 0x4d344120, M4B = 0x4d344220, M4P = 0x4d345020, WAV = 0x57415620, AA = ???
	uint8_t vbr;
	uint8_t type;
	uint8_t compilation;
	uint8_t stars;
	uint32_t lastmodifiedtime;		// iTunes sets this the UTC time value for the Windows Last Modified timestamp
	uint32_t size;
	uint32_t length;
	uint32_t tracknum;
	uint32_t totaltracks;
	uint32_t year;
	uint32_t bitrate;
	uint16_t samplerate;
	uint16_t samplerate_fixedpoint;
	uint32_t volume;
	uint32_t starttime;
	uint32_t stoptime;
	uint32_t soundcheck;
	uint32_t playcount;
	uint32_t playcount2;			// Seems to always be the same as playcount(?!?)
	uint32_t lastplayedtime;
	uint32_t cdnum;
	uint32_t totalcds;
	uint32_t userID;				// Apple Store User ID
	uint32_t addedtime;			// iTunes sets this to the UTC time value for when the file was added to the iTunes library
	uint32_t bookmarktime;
	uint64_t dbid;		// 64 bit value that identifies this mhit across iPod databases.  iTunes increments this by 1 for each additional song.  (previously unk7 and unk8)
	uint32_t BPM;
	uint32_t app_rating;			// The rating set by the application, as opposed to the rating set on the iPod itself
	uint8_t checked;				// a "checked" song has the value of 0, a non-checked song is 1
	uint16_t unk9;				// Seems to always be 0xffff...
	uint16_t artworkcount;		// Number of artwork files attached to this song
	uint32_t artworksize;			// Size of all artwork files attached to this song, in bytes. (was unk10);
	uint32_t unk11;
	float samplerate2;
	uint32_t releasedtime;
	uint32_t unk14;
	uint32_t unk15;
	uint32_t unk16;
	/* --- */
	uint32_t skipcount;
	uint32_t skippedtime;
	uint8_t hasArtwork;
	uint8_t skipShuffle;
	uint8_t rememberPosition;
	uint8_t unk19;
	uint64_t dbid2;	// same as dbid?
	uint8_t lyrics_flag;
	uint8_t movie_flag;
	uint8_t mark_unplayed;
	uint8_t unk20;
	uint32_t unk21;
	uint32_t pregap;
	uint64_t samplecount;
	uint32_t unk25;
	uint32_t postgap;
	uint32_t unk27;
	uint32_t mediatype;
	uint32_t seasonNumber;
	uint32_t episodeNumber;
	uint32_t unk31;
	uint32_t unk32;
	uint32_t unk33;
	uint32_t unk34;
	uint32_t unk35;
	uint32_t unk36;
	/* --- */
  uint32_t unk37;
	uint32_t gaplessData;
	uint32_t unk39;
  uint16_t albumgapless;
  uint16_t trackgapless;
	uint32_t unk40;
	uint32_t unk41;
	uint32_t unk42;
	uint32_t unk43;
	uint32_t unk44;
	uint32_t unk45;
	uint32_t unk46;
  uint32_t album_id;
	uint32_t unk48;
	uint32_t unk49;
	uint32_t unk50;
	uint32_t unk51;
	uint32_t unk52;
	uint32_t unk53;
	uint32_t unk54;
	uint32_t unk55;
	uint32_t unk56;

	/* --- */
	// 22 bytes of unknown (we'll just write back zeroes)
	
	uint32_t mhii_link; // TODO: benski> figure this thing out
	// 32 more bytes of unknown (we'll just write back zeroes)

/* benski> this is a hack. i'm putting this in here so we can retrieve album art from the transfer thread and add it in the main thread 
it doesn't really belong as part of this object, though! */

  // protect these members, so stuff doesn't fuck up my cache
protected:
	std::vector<iPod_mhod*> mhod;
	iPod_mhod * mhodcache[25];
};


// MHLP: playlist container - parent of MHYP, child of MHSD

// Important note: Playlist zero must always be the default playlist, containing every
// track in the DB. To do this, always call "GetDefaultPlaylist()" before you create any
// other playlists, if you start from scratch.
// After you're done adding/deleting tracks in the database, and just before you call
// write(), do the following: GetDefaultPlaylist()->PopulatePlaylist(ptr_to_mhlt);

class iPod_mhlp : public iPodObj
{
public:
	iPod_mhlp();
	virtual ~iPod_mhlp();

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char * data, const unsigned long datasize) {return write(data,datasize,3);}
	virtual long write(unsigned char * data, const unsigned long datasize, int index);

	const unsigned long GetChildrenCount() const	{ return mhyp.size(); }

	// returns a new playlist for you
	iPod_mhyp * AddPlaylist();

	// gets a playlist
	iPod_mhyp * GetPlaylist(const unsigned long pos) const			{ return mhyp.at(pos); }

	// finds a playlist by its ID
	iPod_mhyp * FindPlaylist(const uint64_t playlistID);

	// deletes the playlist at a position
	bool DeletePlaylist(const unsigned long pos);

	// deletes the playlist matching the ID
	bool DeletePlaylistByID(const uint64_t playlistID);

	// gets the default playlist ( GetPlaylist(0); )
	// if there are no playlists yet (empty db), then it creates the default playlist
	//    and returns a pointer to it
	iPod_mhyp * GetDefaultPlaylist();

	// erases all playlists, including the default one, so be careful here.
	// Set createDefaultPlaylist to create a new, empty default playlist
	bool ClearPlaylists(const bool createDefaultPlaylist = false);

	// Goes through all playlists and removed any songs that are no longer in the MHLT
	void RemoveDeadPlaylistEntries(iPod_mhlt *mhlt);

	std::vector<iPod_mhyp*> mhyp;
  
  void SortPlaylists();

private:
	bool beingDeleted;
};

int STRCMP_NULLOK(const wchar_t *pa, const wchar_t *pb);

// MHYP: playlist - parent of MHOD or MHIP, child of MHLP
class iPod_mhyp : public iPodObj
{
public:
	iPod_mhyp();
	virtual ~iPod_mhyp();

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char * data, const unsigned long datasize) {return write(data,datasize,3);}
	virtual long write(unsigned char * data, const unsigned long datasize, int index);

	bool IsSmartPlaylist(void) const  { return(isSmartPlaylist); }

	// add an entry to the playlist. Creates a new entry, returns the position in the vector
	// optionally fills in the songindex for you, with the ID from a track you might have
	long AddPlaylistEntry(iPod_mhip * entry, const unsigned long id=0);

	// give it a song id, it'll return a position in the playlist
	// -1, as always, means not found
	// if the same entry is in the playlist multiple times, this only gives back the first one
	long FindPlaylistEntry(const unsigned long id) const;

	// get an mhip given its position
	iPod_mhip * GetPlaylistEntry(const unsigned long pos) const		{ return mhip.at(pos); }

	// deletes an entry from the playlist. Pointers to that entry become invalid
	bool DeletePlaylistEntry(const unsigned long pos);

	// Removes all playlist entries matching the songindex parameter
	bool DeletePlaylistEntryByID(unsigned long songindex);

	// clears a playlist of all mhip entries
	bool ClearPlaylist();

	// populates a playlist to be the same as a track list you pass into it.
	// Mainly only useful for building the default playlist after you add/delete tracks
	// GetDefaultPlaylist()->PopulatePlaylist(ptr_to_mhlt);
	// for example...
	long PopulatePlaylist(iPod_mhlt * tracks, int hidden_field=1);

	// will add a new string to the playlist
	// optional: pass in a type to get an existing string, if there is one,
	//			 or a new one with the type filled in already, if there is not one
	iPod_mhod * AddString(const int type=0);

	// get an mhod given it's type.. Only really useful with MHOD_TITLE here, until
	// smartlists get worked out better
	iPod_mhod * FindString(const unsigned long type);

	// deletes a string from the playlist
	// if more than one string of given type exists, all of that type will be deleted,
	//		to ensure consistency. Pointers to these strings will be invalid after this.
	// ret val is number of strings removed
	unsigned long DeleteString(const unsigned long type);

	void SetPlaylistTitle(const wchar_t *string);

	const unsigned long GetMhodChildrenCount() const	{ return mhod.size(); }
	const unsigned long GetMhipChildrenCount() const	{ return mhip.size(); }

	static void Duplicate(iPod_mhyp *src, iPod_mhyp *dst);

	unsigned long hidden;
	unsigned long timestamp;
	uint64_t playlistID;	// ID of the playlist, used in smart playlist rules
	unsigned long unk3;
	unsigned short numStringMHODs;
	unsigned short podcastflag;
	unsigned long numLibraryMHODs;

	std::vector<iPod_mhod*> mhod;
	std::vector<iPod_mhip*> mhip;

	struct indexMhit
	{
		__forceinline bool operator()(indexMhit*& one, indexMhit*& two)
		{
#define RETIFNZ(x) { int yy = x; if(yy != 0) return yy < 0; }
			//return(STRCMP_NULLOK(one->str.c_str(), two->str.c_str()) < 0 ? true : false);
      RETIFNZ(STRCMP_NULLOK(one->str[0],two->str[0]));
      RETIFNZ(STRCMP_NULLOK(one->str[1],two->str[1]));
      RETIFNZ(STRCMP_NULLOK(one->str[2],two->str[2]));
      RETIFNZ(one->track - two->track);
      RETIFNZ(STRCMP_NULLOK(one->str[3],two->str[3]));
      return true;
#undef RETIFNZ
		}

		unsigned int index;
		const wchar_t *str[4];
    int track;
	};

	iPod_mhlt::mhit_map_t *mhit;
	std::vector<uint32_t> mhit_indexer;
	bool writeLibraryMHODs;

  bool operator()(iPod_mhyp*& one, iPod_mhyp*& two);

protected:
	bool isSmartPlaylist;
	bool isPopulated;
};


// MHIP: playlist item - child of MHYP
class iPod_mhip : public iPodObj
{
public:
	iPod_mhip();
	virtual ~iPod_mhip();

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char * data, const unsigned long datasize) { return write(data,datasize,0); }
	virtual long write(unsigned char * data, const unsigned long datasize, int entrynum);

	static void Duplicate(iPod_mhip *src, iPod_mhip *dst);

	unsigned long dataobjectcount; // was unk1
	unsigned long podcastgroupflag;	// was corrid
	unsigned long groupid;	// was unk2
	unsigned long songindex;
	unsigned long timestamp;
	unsigned long podcastgroupref;
	std::vector<iPod_mhod*> mhod;
};

// MHOD: string container item, child of MHIT or MHYP
// MHOD: string container item, child of MHIT or MHYP
class iPod_mhod : public iPodObj
{
public:
	iPod_mhod();
	virtual ~iPod_mhod();

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char * data, const unsigned long datasize);

	void SetString(const wchar_t *string);

	static void Duplicate(iPod_mhod *src, iPod_mhod *dst);
	
	static bool IsSimpleStringType(const unsigned int type);

	uint32_t type;
	uint32_t unk1;
	uint32_t unk2;

	// renamed this from corrid.. all it is is a position in the playlist
	// for type 100 mhods that come immediately after mhips.
	// for strings, this is the encoded type.  1 == UTF-16, 2 == UTF-8
	union
	{
		uint32_t position;
		uint32_t encoding_type;
	};

	uint32_t length;
	uint32_t unk3;
	uint32_t unk4;

	// string mhods get the string put here, unaltered, still byte reversed
	// Use unicode functions to work with this string.
	wchar_t *str;

	// mhod types 50 and up get the whole thing put here.
	// until I can figure out all of these, I won't bother to try to recreate them
	// and i'll just copy them back as needed when rewriting the iTunesDB file.
	uint8_t * binary;

	// stuff for type 50 mhod
	uint8_t liveupdate;			// "Live Updating" check box
	uint8_t checkrules;			// "Match X of the following conditions" check box
	uint8_t checklimits;			// "Limit To..." check box.		1 = checked, 0 = not checked
	uint8_t matchcheckedonly;		// "Match only checked songs" check box.
	uint8_t limitsort_opposite;	// Limit Sort rule is reversed (e.g. limitsort == LIMIT_HIGHEST_RATING really means LIMIT_LOWEST_RATING...quite weird...)
	uint32_t limittype;			// See Limit Types defines above
	uint32_t limitsort;			// See Limit Sort defines above
	uint32_t limitvalue;			// Whatever value you type next to "limit type".

	// stuff for type 51 mhod
	uint32_t unk5;					// not sure, probably junk data
	uint32_t rules_operator;		// "All" (logical AND / value = 0) or "Any" (logical OR / value = 1).
	std::vector<SPLRule*> rule;

	bool parseSmartPlaylists;
};


// Smart Playlist.  A smart playlist doesn't act different from a regular playlist,
// except that it contains a type 50 and type 51 MHOD.  But deriving the iPod_slst
// class makes sense, since there are a lot of functions that are only appropriate
// for smart playlists, and it can guarantee that a type 50 and 51 MHOD will always
// be available.
class iPod_slst  : public iPod_mhyp
{
public:
	enum FieldType
	{
		ftString,
		ftInt,
		ftBoolean,
		ftDate,
		ftPlaylist,
		ftUnknown,
		ftBinaryAnd,
	};

	enum ActionType
	{
		atString,
		atInt,
		atBoolean,
		atDate,
		atRange,
		atInTheLast,
		atPlaylist,
		atNone,
		atInvalid,
		atUnknown,
		atBinaryAnd,
	};


	iPod_slst();
	virtual ~iPod_slst();

	iPod_mhod* GetPrefs(void) { UpdateMHODPointers(); return(splPref); }
	void SetPrefs(const bool liveupdate = true, const bool rules_enabled = true, const bool limits_enabled = false,
				  const unsigned long limitvalue = 0, const unsigned long limittype = 0, const unsigned long limitsort = 0);

	static FieldType  GetFieldType(const unsigned long field);
	static ActionType GetActionType(const unsigned long field, const unsigned long action);

	static uint64_t ConvertDateValueToNum(const uint64_t val)	{ return(-(int64_t)val); }
	static uint64_t ConvertNumToDateValue(const uint64_t val)	{ return(-(int64_t)val); }

	// returns a pointer to the SPLDATA mhod
	iPod_mhod* GetRules() { UpdateMHODPointers(); return(splData); }

	// get the number of rules in the smart playlist
	unsigned long GetRuleCount();

	// Returns rule number (0 == first rule, -1 == error)
	int AddRule(const unsigned long field,
		        const unsigned long action,
				const wchar_t * string = NULL,	// use string for string based rules
				const uint64_t value  = 0,		// use value for single variable rules
				const uint64_t from   = 0,		// use from and to for range based rules
				const uint64_t to     = 0,
				const uint64_t units  = 0);		// use units for "in the last" based rules

	int AddRule(const SPLRule& rule);
	
	void RemoveAllRules(void);

	// populates a smart playlist
	// Pass in the mhlt with all the songs on the iPod, and it populates the playlist
	//	given those songs and the current rules
	// Return value is number of songs in the resulting playlist.
	long PopulateSmartPlaylist(iPod_mhlt * tracks, iPod_mhlp * playlists);

	// used in PopulateSmartPlaylist
	static bool EvalRule(
		SPLRule * r,
		iPod_mhit * track,
		iPod_mhlt * tracks = NULL,		// if you're going to allow playlist type rules
		iPod_mhlp * playlists = NULL	// these are required to be passed in
		);

	// Restore default prefs and remove all rules
	void Reset(void);

protected:
	void UpdateMHODPointers(void);

	iPod_mhod *splPref;
	iPod_mhod *splData;
};




// MHDP: Play Count class
class iPod_mhdp
{
public:
	iPod_mhdp();
	~iPod_mhdp();
	unsigned long size_head;
	unsigned long entrysize;

	const unsigned long GetChildrenCount() const	{ return children; }

	// return value is number of songs or -1 if error.
	// you should probably check to make sure the number of songs is the same
	//     as the number of songs you read in from parsing the iTunesDB
	virtual long parse(const uint8_t *data);

	// there is no write() function because there is no conceivable need to ever write a
	// play counts file.

	const PCEntry &GetPlayCount(const unsigned int pos) const		{ return entry[pos]; }

	// playcounts are stored in the Play Counts file, in the same order as the mhits are
	//  stored in the iTunesDB. So you should apply the changes from these entries to the
	//  mhits in order and then probably delete the Play Counts file entirely to prevent
	//  doing it more than once.
	PCEntry *entry;
	uint32_t children;
};




// MHPO: On-The-Go Playlist class
class iPod_mhpo
{
public:
	iPod_mhpo();
	virtual ~iPod_mhpo();

	unsigned long size_head;
	unsigned long unk1;
	unsigned long unk2;	// this looks like a timestamp, sorta

	const unsigned long GetChildrenCount() const	{ return children; }

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char * data, const unsigned long datasize);

	// This will create a new playlist from the OTGPlaylist..
	// Give it the DB to create the playlist in and from, and a name for the playlist.
	// Return value is a pointer to the playlist itself, which will be inside the DB you
	// give to it as well.
	// Returns NULL on error (can't create the playlist)
	iPod_mhyp * CreatePlaylistFromOTG(iPod_mhbd * iPodDB, wchar_t * name);

	// OTGPlaylists are stored in the OTGPlaylist file. When iTunes copies them into a
	// new playlist, it deletes the file afterwards. I do not know if creating this file
	// will make the iPod have an OTGPlaylist after you undock it. I added the write function
	// anyway, in case somebody wants to try it. Not much use for it though, IMO.
	uint32_t *idList;
	uint32_t children;
};


// MQED: EQ Presets holder
class iPod_mqed
{
public:
	iPod_mqed();
	virtual ~iPod_mqed();

	unsigned long size_head;
	unsigned long unk1;
	unsigned long unk2;	// this looks like a timestamp, sorta

	const unsigned long GetChildrenCount() const	{ return eqList.size(); }

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char * data, const unsigned long datasize);

	std::vector<iPod_pqed*> eqList;
};

// PQED: A single EQ Preset
class iPod_pqed
{
public:
	iPod_pqed();
	virtual ~iPod_pqed();

	unsigned long length;		// length of name string
	wchar_t * name;		// name string

/*
  10 band eq is not exactly what iTunes shows it to be.. It really is these:
  32Hz, 64Hz, 128Hz, 256Hz, 512Hz, 1024Hz, 2048Hz, 4096Hz, 8192Hz, 16384Hz

  Also note that although these are longs, The range is only -1200 to +1200. That's dB * 100.
*/

	signed long preamp;			// preamp setting

	signed long eq[10];			// iTunes shows 10 bands for EQ presets
	signed long short_eq[5];	// This is a 5 band version of the same thing (possibly what the iPod actually uses?)

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char * data, const unsigned long datasize);
};




struct iTunesStatsEntry
{
	unsigned int GetBookmarkTimeInMilliseconds()		{ if(bookmarktime == 0xffffff) return(0); return(bookmarktime * 256); }

	// These are 3 byte values
	unsigned int entry_size;
	unsigned int bookmarktime;		// In 0.256 seconds units
	unsigned int unk1;				// Somehow associated with bookmark time
	unsigned int unk2;
	unsigned int playcount;
	unsigned int skippedcount;
};

class iTunesStats
{
public:
	iTunesStats();
	~iTunesStats();

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char * data, const unsigned long datasize);

	const unsigned long GetChildrenCount() const				{ return children; }
	const iTunesStatsEntry &GetEntry(const unsigned int pos) const		{ return entry[pos];}

	// This is a 3 byte value
	unsigned int unk1;

	iTunesStatsEntry *entry;
	uint32_t children;
	iPod_mhlt *mhlt;
};


class iTunesShuffle
{
public:
	iTunesShuffle();
	~iTunesShuffle();

	virtual long parse(const uint8_t *data);
	virtual long write(unsigned char *data, const unsigned long datasize);

	unsigned int GetChildrenCount() const					{ return numentries; }
	unsigned int GetEntry(const unsigned int pos) const		{ return entry[pos]; }
	//void AddEntry(const unsigned int index)					{ entry.push_back(index); }
	void Randomize();
	void Randomize(const unsigned int numsongs);

	uint32_t *entry;
	uint32_t numentries;
	unsigned int datasize;
};
#endif
