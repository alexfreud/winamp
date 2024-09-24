#include <windows.h>
#include "header_avi.h"
#include <mmsystem.h>
#include <bfc/platform/types.h>

#pragma pack(1)


typedef struct
{
	DWORD	dwMicroSecPerFrame;	// frame display rate (or 0L)
	DWORD	dwMaxBytesPerSec;	// max. transfer rate
	DWORD	dwPaddingGranularity;	// pad to multiples of this
	// size; normally 2K.
	DWORD	dwFlags;		// the ever-present flags
	DWORD	dwTotalFrames;		// # frames in file
	DWORD	dwInitialFrames;
	DWORD	dwStreams;
	DWORD	dwSuggestedBufferSize;

	DWORD	dwWidth;
	DWORD	dwHeight;

	DWORD	dwReserved[4];
}
MainAVIHeader;

typedef struct
{
	FOURCC	fccType;
	FOURCC	fccHandler;
	DWORD	dwFlags;	/* Contains AVITF_* flags */
	WORD	wPriority;
	WORD	wLanguage;
	DWORD	dwInitialFrames;
	DWORD	dwScale;
	DWORD	dwRate;	/* dwRate / dwScale == samples/second */
	DWORD	dwStart;
	DWORD	dwLength; /* In units above... */
	DWORD	dwSuggestedBufferSize;
	DWORD	dwQuality;
	DWORD	dwSampleSize;
	RECT	rcFrame;
}
AVIStreamHeader;

#pragma pack()

#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )				\
	( (long)(unsigned char)(ch0) | ( (long)(unsigned char)(ch1) << 8 ) |	\
	  ( (long)(unsigned char)(ch2) << 16 ) | ( (long)(unsigned char)(ch3) << 24 ) )
#endif /* mmioFOURCC */

HeaderAvi::HeaderAvi(bool bAllowHttpConnection /* = false */)
		: bAllowHttp(bAllowHttpConnection), fh(INVALID_HANDLE_VALUE)
{}

HeaderAvi::~HeaderAvi()
{}

int HeaderAvi::getInfos(const wchar_t *filename, bool checkMetadata)
{
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
	if (aviid != mmioFOURCC('A', 'V', 'I', ' '))
	{
		myfclose();
		return 0;
	}

	int last_fccType = 0;
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
			case mmioFOURCC('m', 'o', 'v', 'i'):
				{
					// MOVI header
					//if (!checkMetadata) // might have metadata at the end of the file, so we gotta keep going
					//{
						myfclose();
						return 1;
					//}
					len = (len + 1) & (~1);
					myfseek(len, FILE_CURRENT);
				}
				break;
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
		size_t chunksize = (size2 + 1) & (~1);
		switch (id)
		{
		case mmioFOURCC('a', 'v', 'i', 'h'):
			{
				MainAVIHeader ah;
				size_t l = min(size2, sizeof(ah));
				myfread(&ah, l, 1);
				chunksize -= l;
				video_h = ah.dwHeight; video_w = ah.dwWidth;
				if (video_h && video_w) has_video = true;
			}
			break;
		case mmioFOURCC('s', 't', 'r', 'h'):
			{
				AVIStreamHeader ash;
				size_t l = min(size2, sizeof(ash));
				myfread(&ash, l, 1);
				chunksize -= l;
				last_fccType = ash.fccType;
				if (last_fccType == mmioFOURCC('v', 'i', 'd', 's'))
				{
					float frametime = (float)ash.dwScale / ((float)ash.dwRate / 1000);
					length = (int)((float)ash.dwLength * frametime);
				}
			}
			break;
		case mmioFOURCC('s', 't', 'r', 'f'):
			{
				if (last_fccType == mmioFOURCC('v', 'i', 'd', 's'))
				{
				}
				else if (last_fccType == mmioFOURCC('a', 'u', 'd', 's'))
				{
					WAVEFORMATEX wfe;
					size_t l = min(chunksize, sizeof(wfe));
					myfread(&wfe, sizeof(wfe), 1);
					audio_bps = wfe.wBitsPerSample;
					audio_srate = wfe.nSamplesPerSec;
					audio_nch = wfe.nChannels;
					if (!audio_bps) audio_bps = 16;
					has_audio = true;
					chunksize -= l;
				}
			}
			break;
		}
		if (chunksize > 0) myfseek((long)chunksize, FILE_CURRENT);
	}

	myfclose();
	return 1;
}

size_t HeaderAvi::myfread(void *buffer, size_t size, size_t count)
{
	DWORD bytesRead = 0;
	ReadFile(fh, buffer, (DWORD)(size*count), &bytesRead, NULL);
	return bytesRead;
}

int HeaderAvi::myfclose()
{
	return CloseHandle(fh);
}

// Note; Only supports SEEK_CUR for http (which is what is used)
int HeaderAvi::myfseek(long offset, DWORD origin)
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