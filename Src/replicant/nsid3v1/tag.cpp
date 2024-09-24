#include "tag.h"
#include <string.h> // for strnlen

#include "nsid3v1.h"

#include "foundation/error.h"
#include "nu/ByteReader.h"
#include "nu/ByteWriter.h"

#ifdef __APPLE__

inline static size_t id3v1_strnlen(const char *s, size_t n)
{
	const char *p=(const char *)memchr(s, 0, n);
	return p?p-s:n;
}

#else // __APPLE__

inline static size_t id3v1_strnlen(const char *s, size_t n)
{
	return strnlen(s, n);
}

#endif


ID3v1::Tag::Tag()
{
}

void ID3v1::Tag::New()
{
	header[0]='T';
	header[1]='A';
	header[2]='G';
	memset(title, 0, 30);
	memset(artist, 0, 30);
	memset(album, 0, 30);
	memset(year, 0, 30);
	memset(comment, 0, 30);
	track=0;
	genre=0;
}

// Deprecated, bytereader class can now handle n-number of bytes
static void GetAndFillNumberOfBytes(bytereader_value_t byte_reader, int bytes, const void *data, char *destination)
{
	for (int i = 0; i < bytes; i++)
	{
		destination[i] = bytereader_read_u8(&byte_reader);
	}
}

const char *ID3v1::Tag::GetHeader(void) const			{ return header; }
const char *ID3v1::Tag::GetTitle(void) const			{ return title; }
const char *ID3v1::Tag::GetArtist(void) const			{ return artist; }
const char *ID3v1::Tag::GetAlbum(void) const			{ return album; }
const char *ID3v1::Tag::GetYear(void) const				{ return year; }
const char *ID3v1::Tag::GetComment(void) const			{ return comment; }
uint8_t ID3v1::Tag::GetTrack(void) const	{ return track; }
uint8_t ID3v1::Tag::GetGenre(void) const	{ return genre; }

//unsigned int ID3v1::Tag::GetTitleLength(void)	{ return ( strlen(title) <= 30 ) ? strlen(title) : 30; }
size_t ID3v1::Tag::GetHeaderLength(void) const	{ return id3v1_strnlen(header, 3); }
size_t ID3v1::Tag::GetTitleLength(void) const	{ return id3v1_strnlen(title, 30); }
size_t ID3v1::Tag::GetArtistLength(void) const	{ return id3v1_strnlen(artist, 30); }
size_t ID3v1::Tag::GetAlbumLength(void) const	{ return id3v1_strnlen(album, 30); }
size_t ID3v1::Tag::GetYearLength(void) const	{ return id3v1_strnlen(year, 4); }
size_t ID3v1::Tag::GetCommentLength(void) const	{ return id3v1_strnlen(comment, 30); }

int ID3v1::Tag::Parse(const void *data, size_t len)
{
	if (len < 128)
		return NErr_NeedMoreData;

	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, len);

	// Get header
	bytereader_read_n(&byte_reader, header, 3);

	// Get title
	bytereader_read_n(&byte_reader, title, 30);

	// Get artist
	bytereader_read_n(&byte_reader, artist, 30);

	// Get album
	bytereader_read_n(&byte_reader, album, 30);

	// Get year
	bytereader_read_n(&byte_reader, year, 4);

	// Get comments
	bytereader_read_n(&byte_reader, comment, 30);

	// Get genre
	genre = bytereader_read_u8(&byte_reader);

	// Check for the presence of track # inside of the comments field at offset position 29 & 30
	if (comment[28] == 0 && comment[29] != 0)
	{
		track = comment[29];
	}
	else
		track = 0;

	return NErr_Success;
}

/* copies source_bytes characters to destination, and fills up the remaining (up to destination_length) with null */
static void id3v1_strncpyn(char *destination, size_t destination_length, const char *source, size_t source_bytes)
{
	// make sure we don't write too much
	if (source_bytes > destination_length)
		source_bytes = destination_length;

	memcpy(destination, source, source_bytes);
	memset(destination+source_bytes, 0, destination_length-source_bytes); // zero remainder of string
}

void ID3v1::Tag::SetTitle(const char *new_title, size_t length)
{
	id3v1_strncpyn(title, 30, new_title, length);
}

void ID3v1::Tag::SetArtist(const char *new_artist, size_t length)
{
	id3v1_strncpyn(artist, 30, new_artist, length);
}

void ID3v1::Tag::SetAlbum(const char *new_album, size_t length)
{
	id3v1_strncpyn(album, 30, new_album, length);
}

void ID3v1::Tag::SetYear(const char *new_year, size_t length)
{
	id3v1_strncpyn(year, 4, new_year, length);
}

void ID3v1::Tag::SetComment(const char *new_comment, size_t length)
{
	id3v1_strncpyn(comment, 28, new_comment, length);
}

void ID3v1::Tag::SetTrack(uint8_t new_track)
{
	track = new_track;
}

void ID3v1::Tag::SetGenre(uint8_t new_genre)
{
	genre = new_genre;
}

int ID3v1::Tag::Serialize(void *data, size_t len)
{
	if (len < 128)
		return NErr_Insufficient;

	bytewriter_s byte_writer;
	bytewriter_init(&byte_writer, data, len);

	bytewriter_write_n(&byte_writer, header, 3);
	bytewriter_write_n(&byte_writer, title, 30);
	bytewriter_write_n(&byte_writer, artist, 30);
	bytewriter_write_n(&byte_writer, album, 30);
	bytewriter_write_n(&byte_writer, year, 4);
	bytewriter_write_n(&byte_writer, comment, 28);
	bytewriter_write_u8(&byte_writer, 0);
	bytewriter_write_u8(&byte_writer, track);
	bytewriter_write_u8(&byte_writer, genre);

	return NErr_Success;
}
