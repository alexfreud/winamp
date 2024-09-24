#include "main.h"
#include "ml_local.h"
#include "api_mldb.h"
#include <commctrl.h>
#include "resource.h"
#include "../replicant/nu/ns_wc.h"
#include "../nde/nde.h"
#include "../Agave/Language/api_language.h"
#include "..\..\General\gen_ml/config.h"
#include "..\..\General\gen_ml/gaystring.h"
#include "time.h"
#include "../winamp/in2.h"
#include "../Winamp/strutil.h"
#include <shlwapi.h>
#include <strsafe.h>


bool skipTitleInfo=false;

static int getFileInfoW(const wchar_t *filename, const wchar_t *metadata, wchar_t *dest, size_t len)
{
	dest[0]=0;
	return AGAVE_API_METADATA->GetExtendedFileInfo(filename, metadata, dest, len);
}

static time_t FileTimeToUnixTime(FILETIME *ft)
{
  ULARGE_INTEGER end;
  memcpy(&end,ft,sizeof(end));
  end.QuadPart -= 116444736000000000;
  end.QuadPart /= 10000000; // 100ns -> seconds
  return (time_t)end.QuadPart;
}

void makeFilename2W(const wchar_t *filename, wchar_t *filename2, int filename2_len)
{
	filename2[0]=0;
	GetLongPathNameW(filename, filename2, filename2_len);
	if (!_wcsicmp(filename,filename2)) filename2[0]=0;
}

void makeFilename2(const char *filename, char *filename2, int filename2_len)
{
	filename2[0]=0;
	GetLongPathNameA(filename, filename2, filename2_len);
	if (!stricmp(filename,filename2)) filename2[0]=0;
}

static __int64 FileSize64(HANDLE file)
{
	LARGE_INTEGER position;
	position.QuadPart=0;
	position.LowPart = GetFileSize(file, (LPDWORD)&position.HighPart); 	

	if (position.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
		return INVALID_FILE_SIZE;
	else
		return position.QuadPart;
}

static void GetFileSizeAndTime(const wchar_t *filename, __int64 *file_size, time_t *file_time)
{
	WIN32_FILE_ATTRIBUTE_DATA file_data;
	if (GetFileAttributesExW(filename, GetFileExInfoStandard, &file_data) == FALSE)
	{
		// GetFileAttributesEx failed. that sucks, let's try something else
		HANDLE hFile=CreateFileW(filename,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			FILETIME lt = {0};
			if (GetFileTime(hFile,NULL,NULL,&lt))
			{
				*file_time=FileTimeToUnixTime(&lt);
			}
			*file_size=FileSize64(hFile);
			CloseHandle(hFile);
		}
	}
	else
	{
		// success
		*file_time = FileTimeToUnixTime(&file_data.ftLastWriteTime);
		LARGE_INTEGER size64;
		size64.LowPart = file_data.nFileSizeLow;
		size64.HighPart = file_data.nFileSizeHigh;
		*file_size = size64.QuadPart;
	}
}

int FindFileInDatabase(nde_scanner_t s, int fieldId, const wchar_t *filename, wchar_t alternate[MAX_PATH])
{
	alternate[0]=0;

	makeFilename2W(filename,alternate,MAX_PATH);
	if (alternate[0])
	{
		if (NDE_Scanner_LocateFilename(s, fieldId, FIRST_RECORD, alternate))
		{
			return 2;
		}
	}
	if (NDE_Scanner_LocateFilename(s, fieldId, FIRST_RECORD, filename)) 
		return 1;

	return 0;
}

static inline void GetOptionalField(nde_scanner_t s, const wchar_t *filename, const wchar_t *field, unsigned char field_id)
{
	wchar_t tmp[1024]={0};
	if (getFileInfoW(filename,field,tmp,sizeof(tmp)/sizeof(wchar_t)))
	{
		if(tmp[0])
		{
			tmp[1023]=0; // just in case
			db_setFieldStringW(s, field_id,tmp); 
		}
		else
		{
			db_removeField(s, field_id);
		}
	}
}

static inline void GetOptionalFieldInt(nde_scanner_t s, const wchar_t *filename, const wchar_t *field, unsigned char field_id)
{
	wchar_t tmp[128]={0};
	if (getFileInfoW(filename,field,tmp,sizeof(tmp)/sizeof(wchar_t)))
	{
		if(tmp[0])
		{
			tmp[127]=0; // just in case
			db_setFieldInt(s,field_id,_wtoi(tmp));
		}
		else
		{
			db_removeField(s, field_id);
		}
	}
}

static inline void GetNonBlankFieldInt(nde_scanner_t s, const wchar_t *filename, const wchar_t *field, unsigned char field_id)
{
	wchar_t tmp[128]={0};
	if (getFileInfoW(filename,field,tmp,sizeof(tmp)/sizeof(wchar_t)))
	{
		if(tmp[0])
		{
			tmp[127]=0; // just in case
			db_setFieldInt(s,field_id,_wtoi(tmp));
		}
	}
}


// return values
//  0 - error
//  1 - success
// -2 - record not found (in update only mode)  
int addFileToDb(const wchar_t *filename, int onlyupdate, int use_metadata, int guess_mode, int playcnt, int lastplay, bool force) 
{
	if (!_wcsicmp(PathFindExtensionW(filename), L".cda"))
    return 0;

	__int64 file_size=INVALID_FILE_SIZE;
	time_t file_time=0;
	GetFileSizeAndTime(filename, &file_size, &file_time);
	if (file_size == INVALID_FILE_SIZE || file_size == 0)
		return 0;

	openDb(); // just in case it's not opened yet (this function will return immediately if it's already open)
	EnterCriticalSection(&g_db_cs);

	nde_scanner_t s = NDE_Table_CreateScanner(g_table);

	wchar_t filename2[MAX_PATH] = {0}; // full lfn path if set
	int found = FindFileInDatabase(s, MAINTABLE_ID_FILENAME, filename, filename2);

	if (found)		// For updating
	{
		// if an update wasn't forced, see if the file's timestamp or filesize have changed
		if (!force && NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_FILESIZE))
		{
			nde_field_t f=NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_FILETIME);
			if (f && file_time <= NDE_IntegerField_GetValue(f))
			{
				f=NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_LASTUPDTIME);
				if (f && file_time <= NDE_IntegerField_GetValue(f))
				{
					NDE_Table_DestroyScanner(g_table, s);
					LeaveCriticalSection(&g_db_cs);
					return 1;
				}
			}
		}
		NDE_Scanner_Edit(s);
		if (found == 1 && filename2[0]) db_setFieldStringW(s,MAINTABLE_ID_FILENAME,filename2); // if we have a better filename, update it
		nde_field_t f=NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_PLAYCOUNT);
		int cnt = f?NDE_IntegerField_GetValue(f):0;
		if (!cnt)
			db_setFieldInt(s,MAINTABLE_ID_PLAYCOUNT,0);
	}
	else	// Adding an entry from scratch
	{
		if (onlyupdate) 
		{
			NDE_Table_DestroyScanner(g_table, s);
			LeaveCriticalSection(&g_db_cs);

			// Issue a wasabi system callback after we have successfully updated a file in the ml database
			WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_UPDATED_EXTERNAL, (size_t)filename, 0);
			return -2;
		}
		// new file
		NDE_Scanner_New(s);
		db_setFieldStringW(s,MAINTABLE_ID_FILENAME,filename2[0]?filename2:filename);
		db_setFieldInt(s,MAINTABLE_ID_PLAYCOUNT,playcnt);
		if (lastplay)
			db_setFieldInt(s,MAINTABLE_ID_LASTPLAY,lastplay);
		db_setFieldInt(s,MAINTABLE_ID_DATEADDED, (int)time(NULL));
	}

	int hasttitle=0, hastartist=0, hastalbum=0, hasttrack=0;
	int hastyear=0, hastlength=0;
	int hasdisc=0, hasalbumartist=0;
	int hasttype=0, hasdiscs=0, hastracks=0, hasbitrate=0;
	int the_length_sec=0;
	wchar_t m_artist[1024] = {0}, tmp[1024] = {0};

	if(getFileInfoW(filename, DB_FIELDNAME_type, tmp, sizeof(tmp) / sizeof(wchar_t)) )
	{
		if(tmp[0]) { int type=_wtoi(tmp); db_setFieldInt(s,MAINTABLE_ID_TYPE,type); hasttype++; }
	}

	if (getFileInfoW(filename, DB_FIELDNAME_length,tmp,sizeof(tmp)/sizeof(wchar_t)))
	{
		if(tmp[0]) { db_setFieldInt(s,MAINTABLE_ID_LENGTH,the_length_sec=(_wtoi(tmp)/1000)); hastlength++; }
	}

	if (getFileInfoW(filename, DB_FIELDNAME_bitrate,tmp,sizeof(tmp)/sizeof(wchar_t)))
	{
		if(tmp[0]) { db_setFieldInt(s,MAINTABLE_ID_BITRATE,_wtoi(tmp)); hasbitrate++; }
	}

	if(use_metadata && getFileInfoW(filename, DB_FIELDNAME_title,tmp,sizeof(tmp)/sizeof(wchar_t)))
	{
		if(tmp[0])
		{
			db_setFieldStringW(s,MAINTABLE_ID_TITLE,tmp);
			hasttitle++;
		}

		getFileInfoW( filename, DB_FIELDNAME_artist, tmp, sizeof( tmp ) / sizeof( wchar_t ) );

		if(tmp[0])
		{
			StringCchCopyW(m_artist, 1024, tmp);
			db_setFieldStringW(s,MAINTABLE_ID_ARTIST,tmp);
			hastartist++;
		}

		getFileInfoW(filename, DB_FIELDNAME_album,tmp,sizeof(tmp)/sizeof(wchar_t));

		if(tmp[0])
		{
			db_setFieldStringW(s,MAINTABLE_ID_ALBUM,tmp);
			hastalbum++;
		}

		GetOptionalField(s, filename, DB_FIELDNAME_comment, MAINTABLE_ID_COMMENT);

		getFileInfoW(filename, DB_FIELDNAME_year,tmp,sizeof(tmp)/sizeof(wchar_t));
		if(tmp[0] && !wcsstr(tmp,L"__") && !wcsstr(tmp,L"/") && !wcsstr(tmp,L"\\") && !wcsstr(tmp,L".")) { 
			wchar_t *p=tmp;
			while (p && *p)
			{
				if (*p == L'_') *p=L'0';
				p++;
			}
			int y=_wtoi(tmp);
			if(y!=0) { db_setFieldInt(s,MAINTABLE_ID_YEAR,_wtoi(tmp));  hastyear++; }
		}
		GetOptionalField(s, filename, DB_FIELDNAME_genre, MAINTABLE_ID_GENRE);

		getFileInfoW(filename, DB_FIELDNAME_track,tmp,sizeof(tmp)/sizeof(wchar_t));
		if(tmp[0])
		{
			int track, tracks;
			ParseIntSlashInt(tmp, &track, &tracks);
			if (track > 0)
			{
				db_setFieldInt(s,MAINTABLE_ID_TRACKNB,track); 
				hasttrack++; 
			}
			if (tracks > 0)
			{
				db_setFieldInt(s,MAINTABLE_ID_TRACKS,tracks); 
				hastracks++; 
			}
		}
		getFileInfoW(filename, DB_FIELDNAME_disc,tmp,sizeof(tmp)/sizeof(wchar_t));
		if(tmp[0])
		{
			int disc, discs;
			ParseIntSlashInt(tmp, &disc, &discs);
			if (disc > 0)
			{
				db_setFieldInt(s,MAINTABLE_ID_DISC,disc); 
				hasdisc++; 
			}
			if (discs > 0)
			{
				db_setFieldInt(s,MAINTABLE_ID_DISCS,discs); 
				hasdiscs++; 
			}
		}
		getFileInfoW(filename, DB_FIELDNAME_albumartist,tmp,sizeof(tmp)/sizeof(wchar_t));
		if(tmp[0]) { db_setFieldStringW(s,MAINTABLE_ID_ALBUMARTIST,tmp); hasalbumartist++; }
		GetOptionalField(s, filename, DB_FIELDNAME_publisher,             MAINTABLE_ID_PUBLISHER);
		GetOptionalField(s, filename, DB_FIELDNAME_composer,              MAINTABLE_ID_COMPOSER);
		GetOptionalField(s, filename, DB_FIELDNAME_replaygain_album_gain, MAINTABLE_ID_ALBUMGAIN);
		GetOptionalField(s, filename, DB_FIELDNAME_replaygain_track_gain, MAINTABLE_ID_TRACKGAIN);
		GetOptionalFieldInt(s, filename, DB_FIELDNAME_bpm, MAINTABLE_ID_BPM);
		GetOptionalField(s, filename, DB_FIELDNAME_GracenoteFileID, MAINTABLE_ID_GRACENOTEFILEID);
		GetOptionalField(s, filename, DB_FIELDNAME_GracenoteExtData, MAINTABLE_ID_GRACENOTEEXTDATA);
		GetOptionalFieldInt(s, filename, DB_FIELDNAME_lossless, MAINTABLE_ID_LOSSLESS);
		GetOptionalField(s, filename, DB_FIELDNAME_category, MAINTABLE_ID_CATEGORY);
		GetOptionalField(s, filename, DB_FIELDNAME_codec, MAINTABLE_ID_CODEC);
		GetOptionalField(s, filename, DB_FIELDNAME_director, MAINTABLE_ID_DIRECTOR);
		GetOptionalField(s, filename, DB_FIELDNAME_producer, MAINTABLE_ID_PRODUCER);
		GetOptionalFieldInt(s, filename, DB_FIELDNAME_width, MAINTABLE_ID_WIDTH);
		GetOptionalFieldInt(s, filename, DB_FIELDNAME_height, MAINTABLE_ID_HEIGHT);
		if (g_config->ReadInt(L"writeratings", 0))
			GetOptionalFieldInt(s, filename, DB_FIELDNAME_rating, MAINTABLE_ID_RATING);
		else
			GetNonBlankFieldInt(s, filename, DB_FIELDNAME_rating, MAINTABLE_ID_RATING);
		GetOptionalField(s, filename, L"mime", MAINTABLE_ID_MIMETYPE);
	}

	int guessmode = guess_mode;
	if (guessmode != 2 && ((!hasttitle) + (!hastartist) + (!hastalbum) + (!hasttrack) >= (g_guessifany ? 1 : 4)))
	{
		int tn = 0;
		wchar_t *artist = 0, *album = 0, *title = 0, *guessbuf = 0;

		if (guessmode==1)
		{
			guessbuf = _wcsdup(filename2[0] ? filename2 : filename);

			wchar_t *p=scanstr_backW(guessbuf, L"\\/.", guessbuf);
			if (*p == '.') 
			{
				*p = 0;
				p = scanstr_backW(guessbuf, L"\\/", guessbuf);
			}

			if (p > guessbuf)
			{
				*p = 0;
				title = p+1;
				p=scanstr_backW(guessbuf, L"\\/", guessbuf);
				if (p > guessbuf)
				{
					*p = 0;
					album = p+1;
					p=scanstr_backW(guessbuf,L"\\/", guessbuf);
					if (p > guessbuf)
					{
						*p = 0;
						artist = p+1;
					}
				}
			}
		} 
		else 
			guessbuf = guessTitles(filename2[0] ? filename2 : filename, &tn, &artist, &album, &title);

		if (guessbuf)
		{
			if (!hasttitle && title) { hasttitle++; db_setFieldStringW(s,MAINTABLE_ID_TITLE,title); }
			if (!hastartist && artist) { hastartist++; db_setFieldStringW(s,MAINTABLE_ID_ARTIST,artist); StringCbCopyW(m_artist, sizeof(m_artist), artist); }
			if (!hastalbum && album) { hastalbum++; db_setFieldStringW(s,MAINTABLE_ID_ALBUM,album); }
			if (!hasttrack && tn) { hasttrack++; db_setFieldInt(s,MAINTABLE_ID_TRACKNB,tn); }
			free(guessbuf);
		}
	}

	if (!hastlength || !hasttitle) 
	{
		// try to query length and title using older GetFileInfo input plugin API
		wchar_t ft[1024] = {0};
		basicFileInfoStructW bi={0};
		bi.filename=filename2[0]?filename2:filename;
		bi.length=-1;
		bi.title=ft;
		bi.titlelen=1024;
		skipTitleInfo=true;
		LeaveCriticalSection(&g_db_cs); // benski> not actually sure if this is safe, but it prevents a deadlock
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&bi,IPC_GET_BASIC_FILE_INFOW);
		EnterCriticalSection(&g_db_cs);
		skipTitleInfo=false;
		if (!hastlength && bi.length >= 0)
		{
			hastlength=1;
			db_setFieldInt(s,MAINTABLE_ID_LENGTH,the_length_sec=bi.length);
		}
		if (!hasttitle && ft[0])
		{
			hasttitle=1;
			db_setFieldStringW(s,MAINTABLE_ID_TITLE,ft);
		}
	}

	// set up default (empty) strings/values
	if (!hasttitle) // title=filename
	{
		wchar_t *p = PathFindFileNameW(filename), *dup = _wcsdup(p);
		if (dup)
		{
			PathRemoveExtensionW(dup);
			db_setFieldStringW(s,MAINTABLE_ID_TITLE,dup);
			free(dup);
		}
	}

	if (!hastartist) db_removeField(s,MAINTABLE_ID_ARTIST);
	if (!hastalbum) db_removeField(s,MAINTABLE_ID_ALBUM);
	if (!hasttrack) db_removeField(s,MAINTABLE_ID_TRACKNB);
	if (!hastracks) db_removeField(s,MAINTABLE_ID_TRACKS);
	if (!hastyear) db_removeField(s,MAINTABLE_ID_YEAR);
	if (!hastlength) db_removeField(s,MAINTABLE_ID_LENGTH);
	if (!hasttype) db_setFieldInt(s,MAINTABLE_ID_TYPE,0); //audio
	if (!hasdisc) db_removeField(s, MAINTABLE_ID_DISC);
	if (!hasdiscs) db_removeField(s, MAINTABLE_ID_DISCS);
	if (!hasalbumartist)
	{
		if (hastartist && g_config->ReadInt(L"artist_as_albumartist", 1))
			db_setFieldStringW(s, MAINTABLE_ID_ALBUMARTIST, m_artist);
		else 
			db_removeField(s, MAINTABLE_ID_ALBUMARTIST);
	}

	if (file_size != INVALID_FILE_SIZE)
	{
		db_setFieldInt64(s,MAINTABLE_ID_FILESIZE, file_size);
	}
	else db_removeField(s,MAINTABLE_ID_FILESIZE);

	db_setFieldInt(s,MAINTABLE_ID_LASTUPDTIME, (int)time(NULL));
	db_setFieldInt(s,MAINTABLE_ID_FILETIME, (int)file_time);

	if (!hasbitrate && the_length_sec)
	{
		__int64 br =(file_size*8LL) / (__int64)the_length_sec;
		br /= 1000;
		db_setFieldInt(s,MAINTABLE_ID_BITRATE,(int)br);
	}

	NDE_Scanner_Post(s);
	g_table_dirty++;

	NDE_Table_DestroyScanner(g_table, s);

	LeaveCriticalSection(&g_db_cs);

	if (found)
	{
		// Issue a wasabi system callback after we have successfully updated a file in the ml database
		WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_UPDATED, (size_t)filename, 0);
	}
	else
	{
		// Issue a wasabi system callback after we have successfully added a file in the ml database
		WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_ADDED, (size_t)filename, 0);
	}

	return 1;
}