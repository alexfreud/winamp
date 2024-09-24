#include "header_wav.h"
#include <windows.h>
#include "header_avi.h"
#include <mmsystem.h>
#include <bfc/platform/types.h>
#pragma pack(1)

struct WAVEfmt
{
	unsigned __int16 encoding;
	unsigned __int16 numChannels;
	unsigned __int32 sampleRate;
	unsigned __int32 avgBytesPerSec;
	unsigned __int16 blockAlign;
  unsigned __int16 bitsPerSample;
};
#pragma pack()

#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )				\
	( (long)(unsigned char)(ch0) | ( (long)(unsigned char)(ch1) << 8 ) |	\
	  ( (long)(unsigned char)(ch2) << 16 ) | ( (long)(unsigned char)(ch3) << 24 ) )
#endif /* mmioFOURCC */

HeaderWav::HeaderWav(bool bAllowHttpConnection /* = false */)
		: bAllowHttp(bAllowHttpConnection), fh(INVALID_HANDLE_VALUE)
{}



int HeaderWav::getInfos(const wchar_t *filename, bool checkMetadata)
{
	unsigned __int32 bytesPerSecond=0;
	{
		fh = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (fh == INVALID_HANDLE_VALUE) return 0;
	}

	int riffid = read_dword();
	if (riffid != mmioFOURCC('R', 'I', 'F', 'F'))
	{
		myfclose();
		return 0;
	}

	unsigned __int32 filesize = read_dword();
	int aviid = read_dword();
	if (aviid != mmioFOURCC('W', 'A', 'V', 'E'))
	{
		myfclose();
		return 0;
	}

	while (1)
	{
		int id = read_dword();
		if (!id) break;

		if (id == mmioFOURCC('L', 'I', 'S', 'T'))
		{
			unsigned __int32 len = read_dword() - 4;
			id = read_dword();

			switch (id)
			{
			case mmioFOURCC('I', 'N', 'F', 'O'):
				{
					if (!checkMetadata)
					{
						if (len)
							myfseek(len, FILE_CURRENT);
						break;
					}

					while (len)
					{
						int infoId = read_dword();
						unsigned __int32 infoSize = read_dword();
						unsigned __int32 chunksize = (infoSize + 1) & (~1);
						len -= (chunksize + 8);
						switch (infoId)
						{
							// TODO: IYER for year, but is it string or number?
							// TODO: ITRK for track, but is it string or number?
						case mmioFOURCC('I', 'C', 'O', 'M'):
							{
								composer = (wchar_t*)calloc(infoSize + 1, sizeof(wchar_t));
								composer[infoSize] = 0;
								myfread(composer, infoSize, 1);
								chunksize -= infoSize;
							}
							break;
						case mmioFOURCC('I', 'P', 'U', 'B'):
							{
								publisher = (wchar_t*)calloc(infoSize + 1, sizeof(wchar_t));
								publisher[infoSize] = 0;
								myfread(publisher, infoSize, 1);
								chunksize -= infoSize;
							}
							break;
						case mmioFOURCC('I', 'A', 'L', 'B'):
							{
								album = (wchar_t*)calloc(infoSize + 1, sizeof(wchar_t));
								album[infoSize] = 0;
								myfread(album, infoSize, 1);
								chunksize -= infoSize;
							}
							break;
						case mmioFOURCC('I', 'G', 'N', 'R'):
							{
								genre = (wchar_t*)calloc(infoSize + 1, sizeof(wchar_t));
								genre[infoSize] = 0;
								myfread(genre, infoSize, 1);
								chunksize -= infoSize;
							}
							break;
						case mmioFOURCC('I', 'C', 'M', 'T'):
							{
								comment = (wchar_t*)calloc(infoSize + 1, sizeof(wchar_t));
								comment[infoSize] = 0;
								myfread(comment, infoSize, 1);
								chunksize -= infoSize;
							}
							break;
						case mmioFOURCC('I', 'A', 'R', 'T'):
							{
								artist = (wchar_t*)calloc(infoSize + 1, sizeof(wchar_t));
								artist[infoSize] = 0;
								myfread(artist, infoSize, 1);
								chunksize -= infoSize;
							}
							break;
						case mmioFOURCC('I', 'N', 'A', 'M'):
							{
								title = (wchar_t*)calloc(infoSize + 1, sizeof(wchar_t));
								title[infoSize] = 0;
								myfread(title, infoSize, 1);
								chunksize -= infoSize;
							}
							break;
						}
						if (chunksize > 0) myfseek(chunksize, FILE_CURRENT);

					}
				}
				break;
			}
			continue;
		}

		unsigned __int32 size2 = read_dword();
		unsigned __int32 chunksize = (size2 + 1) & (~1);
		switch (id)
		{
		case mmioFOURCC('f', 'm', 't', ' '):
			{
				WAVEfmt fmt;
				size_t l = min(size2, sizeof(fmt));
				myfread(&fmt, l, 1);
				chunksize -= (unsigned __int32)l;
				has_audio=true;
				audio_nch=fmt.numChannels;
				audio_bps=fmt.bitsPerSample;
				audio_srate=fmt.sampleRate;
				bytesPerSecond=fmt.avgBytesPerSec;
			}
			break;
		case mmioFOURCC('d','a','t','a'):
			{
				if (bytesPerSecond)
					length = MulDiv(1000, chunksize, bytesPerSecond);
			}
			break;

		}
		if (chunksize > 0) myfseek(chunksize, FILE_CURRENT);
	}

	myfclose();
	return 1;
}

size_t HeaderWav::myfread(void *buffer, size_t size, size_t count)
{
	DWORD bytesRead = 0;
	ReadFile(fh, buffer,(DWORD)(size*count), &bytesRead, NULL);
	return bytesRead;
}


int HeaderWav::myfclose()
{
	return CloseHandle(fh);
}


// Note; Only supports SEEK_CUR for http (which is what is used)
int HeaderWav::myfseek(long offset, DWORD origin)
{
	return SetFilePointer(fh, offset, 0, origin);

}

/* TODO:
__int64 myFileSeek (HANDLE hf, __int64 distance, DWORD MoveMethod)
{
   LARGE_INTEGER li;

   li.QuadPart = distance;

   li.LowPart = SetFilePointer (hf, li.LowPart, &li.HighPart, MoveMethod);

   if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
   {
      li.QuadPart = -1;
   }

   return li.QuadPart;
}
*/