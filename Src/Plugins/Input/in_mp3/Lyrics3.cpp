#include <windows.h>
#include "Lyrics3.h"
#include "config.h"
#include <strsafe.h>

// http://www.id3.org/Lyrics3v2

Lyrics3::Lyrics3()
{
	artist=0;
	title=0;
	album=0;
	hasData=false;
	dirty=false;
}

Lyrics3::~Lyrics3()
{
	free(artist);
	free(title);
	free(album);
}

static wchar_t *CopyField(const uint8_t *buffer, uint32_t size)
{
	int converted = MultiByteToWideChar(28591, 0,(LPCSTR)buffer, size, 0, 0);
	wchar_t *str = (wchar_t *)calloc((converted+1), sizeof(wchar_t));
	if (str)
	{
		converted = MultiByteToWideChar(28591, 0, (LPCSTR)buffer, size, str, converted);
		str[converted]=0;
	}

	return str;
}

int Lyrics3::Decode(const void *data, size_t datalen)
{
	if (!config_parse_lyrics3)
		return 1;

	if (memcmp(data, "LYRICSBEGIN", 11) == 0)
	{
		hasData = true;

		datalen-=11;
		uint8_t *buffer = (uint8_t *)data+11;

		while (datalen > 8)
		{
			uint8_t fid[4] = {0};
			uint8_t sizeT[6] = {0};
			uint32_t size;
			fid[3] = 0;
			sizeT[5] = 0;

			memcpy(fid, buffer, 3);
			buffer+=3; datalen-=3;

			memcpy(sizeT, buffer, 5);
			buffer+=5; datalen-=5;

			size = strtoul((char *)sizeT, 0, 10);

			if (datalen >= size)
			{
				/*if ( memcmp(fid, "IND", 3) == 0) // the IND field
				{
					if ( buff2[ posn + 8 + 1 ] == '1')
						stampsUsed = true;
				}
				else */
				if (memcmp(fid, "ETT", 3) == 0) // the TITLE field
				{
					title = CopyField(buffer, size);
				}
				else if (strcmp((char *) fid, "EAR") == 0) // the ARTIST field
				{
					artist = CopyField(buffer, size);
				}
				else if (strcmp((char *) fid, "EAL") == 0)  // the ALBUM field
				{
					album = CopyField(buffer, size);
				}
				/*else if ( strcmp((char *) fid, "LYR") == 0) 							// the LYRICS field
				{
					char *text;
					luint newSize;

					newSize = ID3_CRLFtoLF((char *) & buff2[ posn + 8 ], size);

					if ( stampsUsed)
						newSize = ID3_StripTimeStamps((char *) & buff2[ posn + 8 ], newSize);

					if ( text = (char*)malloc(newSize + 1))
					{
						text[ newSize ] = 0;

						memcpy( text, &buff2[ posn + 8 ], newSize);

						ID3_AddLyrics( this, text);

						free(text);
					}
					else
						ID3_THROW( ID3E_NoMemory);
				}*/

				datalen-=size;
				buffer+=size;
			}
			else
				break;
		}
		return 0;
	}
	return 1;

}

int Lyrics3::GetString(const char *tag, wchar_t *data, int dataLen)
{
	if (!hasData)
		return 0;

	if (!_stricmp(tag, "title"))
	{
		if (title && *title)
		{
			StringCchCopyW(data, dataLen, title);
			return 1;
		}
		return -1;
	}
	else if (!_stricmp(tag, "artist"))
	{
		if (artist && *artist)
		{
			StringCchCopyW(data, dataLen, artist);
			return 1;
		}
		return -1;
	}
	else if (!_stricmp(tag, "album"))
	{
		if (album && *album)
		{
			StringCchCopyW(data, dataLen, album);
			return 1;
		}
		return -1;
	}

	return 0;
}

int Lyrics3::SetString(const char *tag, const wchar_t *data)
{
	int ret=0;
	if (!_stricmp(tag, "title"))
	{
		if (title) free(title);
		title = _wcsdup(data);
		ret = 1;
	}
	else if (!_stricmp(tag, "artist"))
	{
		if ( artist ) free(artist);
		artist = _wcsdup(data);
		ret = 1;
	}
	else if (!_stricmp(tag, "album"))
	{
		if ( album ) free(album);
		album = _wcsdup(data);
		ret = 1;
	}
	
	if(ret)
	{
		hasData=true;
	}
	return ret;
}

void Lyrics3::Clear()
{
	free(artist); artist=0;
	free(album); album=0;
	free(title); title=0;
	dirty=true;
	hasData=false;
}