#include "ID3v1.h"
#include "../nu/ns_wc.h"
#include <windows.h>
#include "config.h"
#include <strsafe.h>

const wchar_t *id3v1_genres[] =
{
	L"Blues", L"Classic Rock", L"Country", L"Dance", L"Disco", L"Funk", L"Grunge",
	L"Hip-Hop", L"Jazz", L"Metal", L"New Age", L"Oldies", L"Other", L"Pop", L"R&B",
	L"Rap", L"Reggae", L"Rock", L"Techno", L"Industrial", L"Alternative", L"Ska",
	L"Death Metal", L"Pranks", L"Soundtrack", L"Euro-Techno", L"Ambient", L"Trip-Hop",
	L"Vocal", L"Jazz+Funk", L"Fusion", L"Trance", L"Classical", L"Instrumental",
	L"Acid", L"House", L"Game", L"Sound Clip", L"Gospel", L"Noise", L"Alt Rock",
	L"Bass", L"Soul", L"Punk", L"Space", L"Meditative", L"Instrumental Pop",
	L"Instrumental Rock", L"Ethnic", L"Gothic", L"Darkwave", L"Techno-Industrial",
	L"Electronic", L"Pop-Folk", L"Eurodance", L"Dream", L"Southern Rock", L"Comedy",
	L"Cult", L"Gangsta Rap", L"Top 40", L"Christian Rap", L"Pop/Funk", L"Jungle",
	L"Native American", L"Cabaret", L"New Wave", L"Psychedelic", L"Rave", L"Showtunes",
	L"Trailer", L"Lo-Fi", L"Tribal", L"Acid Punk", L"Acid Jazz", L"Polka", L"Retro",
	L"Musical", L"Rock & Roll", L"Hard Rock", L"Folk", L"Folk-Rock", L"National Folk",
	L"Swing", L"Fast-Fusion", L"Bebop", L"Latin", L"Revival", L"Celtic", L"Bluegrass",
	L"Avantgarde", L"Gothic Rock", L"Progressive Rock", L"Psychedelic Rock",
	L"Symphonic Rock", L"Slow Rock", L"Big Band", L"Chorus", L"Easy Listening",
	L"Acoustic", L"Humour", L"Speech", L"Chanson", L"Opera", L"Chamber Music", L"Sonata",
	L"Symphony", L"Booty Bass", L"Primus", L"Porn Groove", L"Satire", L"Slow Jam",
	L"Club", L"Tango", L"Samba", L"Folklore", L"Ballad", L"Power Ballad", L"Rhythmic Soul",
	L"Freestyle", L"Duet", L"Punk Rock", L"Drum Solo", L"A Cappella", L"Euro-House",
	L"Dance Hall", L"Goa", L"Drum & Bass", L"Club-House", L"Hardcore", L"Terror", L"Indie",
	L"BritPop", L"Afro-Punk", L"Polsk Punk", L"Beat", L"Christian Gangsta Rap",
	L"Heavy Metal", L"Black Metal", L"Crossover", L"Contemporary Christian",
	L"Christian Rock", L"Merengue", L"Salsa", L"Thrash Metal", L"Anime", L"JPop",
	L"Synthpop", L"Abstract", L"Art Rock", L"Baroque", L"Bhangra", L"Big Beat",
	L"Breakbeat", L"Chillout", L"Downtempo", L"Dub", L"EBM", L"Eclectic", L"Electro",
	L"Electroclash", L"Emo", L"Experimental", L"Garage", L"Global", L"IDM", L"Illbient",
	L"Industro-Goth", L"Jam Band", L"Krautrock", L"Leftfield", L"Lounge", L"Math Rock",
	L"New Romantic", L"Nu-Breakz", L"Post-Punk", L"Post-Rock", L"Psytrance", L"Shoegaze",
	L"Space Rock", L"Trop Rock", L"World Music", L"Neoclassical", L"Audiobook",
	L"Audio Theatre", L"Neue Deutsche Welle", L"Podcast", L"Indie Rock", L"G-Funk",
	L"Dubstep", L"Garage Rock", L"Psybient", L"Glam Rock", L"Dream Pop", L"Merseybeat",
	L"K-Pop", L"Chiptune", L"Grime", L"Grindcore", L"Indietronic", L"Indietronica",
	L"Jazz Rock", L"Jazz Fusion", L"Post-Punk Revival", L"Electronica", L"Psychill",
	L"Ethnotronic", L"Americana", L"Ambient Dub", L"Digital Dub", L"Chillwave", L"Stoner Rock",
	L"Slowcore", L"Softcore", L"Flamenco", L"Hi-NRG", L"Ethereal", L"Drone", L"Doom Metal",
	L"Doom Jazz", L"Mainstream", L"Glitch", L"Balearic", L"Modern Classical", L"Mod",
	L"Contemporary Classical", L"Psybreaks", L"Psystep", L"Psydub", L"Chillstep", L"Berlin School",
	L"Future Jazz", L"Djent", L"Musique Concrète", L"Electroacoustic", L"Folktronica", L"Texas Country", L"Red Dirt",
	L"Arabic", L"Asian", L"Bachata", L"Bollywood", L"Cajun", L"Calypso", L"Creole", L"Darkstep", L"Jewish", L"Reggaeton", L"Smooth Jazz",
	L"Soca", L"Spiritual", L"Turntablism", L"Zouk", L"Neofolk", L"Nu Jazz",
};

size_t numGenres = sizeof(id3v1_genres)/sizeof(*id3v1_genres);

ID3v1::ID3v1()
{
	title[0]=0;
	artist[0]=0;
	album[0]=0;
	comment[0]=0;
	year[0]=0;
	genre=255;
	track=0;
	hasData=false;
	dirty=false;
	title[30]=0;
	artist[30]=0;
	album[30]=0;
	comment[30]=0;
	year[4]=0;
}

int ID3v1::Decode(const void *data)
{
	const char *fbuf = (const char *)data;
	ptrdiff_t x;

	hasData = false;
	title[0] = artist[0] = album[0] = year[0] = comment[0] = 0;
	genre = 255; track = 0;

	if (memcmp(fbuf, "TAG", 3))
	{
		return 1;
	}
	memcpy(title, fbuf + 3, 30); x = 29; while (x >= 0 && title[x] == ' ') x--; title[x + 1] = 0;
	memcpy(artist, fbuf + 33, 30); x = 29; while (x >= 0 && artist[x] == ' ') x--; artist[x + 1] = 0;
	memcpy(album, fbuf + 63, 30); x = 29; while (x >= 0 && album[x] == ' ') x--; album[x + 1] = 0;
	memcpy(year, fbuf + 93, 4); x = 3; while (x >= 0 && year[x] == ' ') x--; year[x + 1] = 0;
	memcpy(comment, fbuf + 97, 30); x = 29; while (x >= 0 && comment[x] == ' ') x--; comment[x + 1] = 0;
	if (fbuf[97 + 28] == 0 && fbuf[97 + 28 + 1] != 0) track = fbuf[97 + 28 + 1];
	genre = ((unsigned char *)fbuf)[127];
	hasData = 1;
	return 0;
}

int ID3v1::Encode(void *data)
{
	if (!hasData)
		return 1;
	char *fbuf = (char *)data;
	size_t x;
	fbuf[0] = 'T';fbuf[1] = 'A';fbuf[2] = 'G';
	if (title) strncpy(fbuf + 3, title, 30); for (x = 3 + strlen(title); x < 33; x ++) fbuf[x] = 0;
	if (artist) strncpy(fbuf + 33, artist, 30); for (x = 33 + strlen(artist); x < 63; x ++) fbuf[x] = 0;
	if (album) strncpy(fbuf + 63, album, 30); for (x = 63 + strlen(album); x < 93; x ++) fbuf[x] = 0;
	if (year) strncpy(fbuf + 93, year, 4); for (x = 93 + strlen(year); x < 97; x ++) fbuf[x] = 0;
	if (comment) strncpy(fbuf + 97, comment, 30); for (x = 97 + strlen(comment); x < 127; x ++) fbuf[x] = 0;
	if (track)
	{
		fbuf[97 + 28] = 0;
		fbuf[97 + 28 + 1] = track;
	}
	((unsigned char *)fbuf)[127] = genre;
	return 0;
}

#define ID3V1_CODEPAGE ((config_read_mode==READ_LOCAL)?CP_ACP:28591)
int ID3v1::GetString(const char *tag, wchar_t *data, int dataLen)
{
	if (!hasData)
		return 0;

	if (!_stricmp(tag, "title"))
	{
		MultiByteToWideCharSZ(ID3V1_CODEPAGE, 0, title, -1, data, dataLen);
		return 1;
	}
	else if (!_stricmp(tag, "artist"))
	{
		MultiByteToWideCharSZ(ID3V1_CODEPAGE, 0, artist, -1, data, dataLen);
		return 1;
	}
	else if (!_stricmp(tag, "album"))
	{
		MultiByteToWideCharSZ(ID3V1_CODEPAGE, 0, album, -1, data, dataLen);
		return 1;
	}
	else if (!_stricmp(tag, "comment"))
	{
		MultiByteToWideCharSZ(ID3V1_CODEPAGE, 0, comment, -1, data, dataLen);
		return 1;
	}
	else if (!_stricmp(tag, "year"))
	{
		MultiByteToWideCharSZ(ID3V1_CODEPAGE, 0, year, -1, data, dataLen);
		return 1;
	}
	else if (!_stricmp(tag, "genre"))
	{
		if (genre >= numGenres)	return -1;
		StringCchCopyW(data, dataLen, id3v1_genres[genre]);
		return 1;
	}
	else if (!_stricmp(tag, "track"))
	{
		if (track == 0)	return -1;
		StringCchPrintfW(data, dataLen, L"%u", track);
		return 1;
	}
	else
		return 0;
}

int ID3v1::SetString(const char *tag, const wchar_t *data)
{
	if (!_stricmp(tag, "title"))
		WideCharToMultiByteSZ(ID3V1_CODEPAGE, 0, data, -1, title, 31, 0 ,0);
	else if (!_stricmp(tag, "artist"))
		WideCharToMultiByteSZ(ID3V1_CODEPAGE, 0, data, -1, artist, 31, 0 ,0);
	else if (!_stricmp(tag, "album"))
		WideCharToMultiByteSZ(ID3V1_CODEPAGE, 0, data, -1, album, 31, 0 ,0);
	else if (!_stricmp(tag, "comment"))
		WideCharToMultiByteSZ(ID3V1_CODEPAGE, 0, data, -1, comment, 31, 0 ,0);
	else if (!_stricmp(tag, "year"))
		WideCharToMultiByteSZ(ID3V1_CODEPAGE, 0, data, -1, year, 5, 0 ,0);
	else if (!_stricmp(tag, "genre"))
	{
		genre=255;
		if (data)
		{
			for (size_t i=0;i<numGenres;i++)
			{
				if (!_wcsicmp(id3v1_genres[i], data))
				{
					genre= (unsigned char)i;
				}
			}
		}
	}
	else if (!_stricmp(tag, "track"))
	{
		int t = _wtoi(data);
		if(t > 255) track = 0;
		else track = t;
	}
	else
		return 0;

	dirty=true;
	hasData = 1;
	return 1;
}

void ID3v1::Clear()
{
	hasData=false;
	dirty=true;
	//clear data
	title[0]=artist[0]=album[0]=comment[0]=year[0]=0;
	genre = track = 0;
}