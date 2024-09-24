#pragma once

#include "nsid3v1.h"

class nstest;

namespace ID3v1
{
	class Tag
	{
		friend class nstest;
	public:
		Tag();
		int Parse(const void *data, size_t len);
		void New(); // creates an empty (but valid) tag
	
		// Member value getters
		const char *GetHeader(void) const;
		const char *GetTitle(void) const;
		const char *GetArtist(void) const;
		const char *GetAlbum(void) const;
		const char *GetYear(void) const;
		const char *GetComment(void) const;
		uint8_t GetTrack(void) const;
		uint8_t GetGenre(void) const;

		// Member length getters
		size_t GetHeaderLength(void) const;
		size_t GetTitleLength(void) const;
		size_t GetArtistLength(void) const;
		size_t GetAlbumLength(void) const;
		size_t GetYearLength(void) const;
		size_t GetCommentLength(void) const;

		void SetTitle(const char *title, size_t length);
		void SetArtist(const char *artist, size_t length);
		void SetAlbum(const char *album, size_t length);
		void SetYear(const char *year, size_t length);
		void SetComment(const char *comment, size_t length);
		void SetTrack(uint8_t track);
		void SetGenre(uint8_t genre);

		int Serialize(void *data, size_t len);
	protected:
		char header[3];
		char title[30];
		char artist[30];
		char album[30];
		char year[4];
		char comment[30];			// Bytes 29 & 30 can contain 0 & genre respectivly, ID3V1.1
		uint8_t track;
		uint8_t genre;
	};
}
