#include "main.h"
#include "ml_local.h"
#include "..\..\General\gen_ml/ml.h"
#include "../replicant/nu/ns_wc.h"
#include "../nde/nde.h"
#include "../replicant/nu/AutoWide.h"
#include "../nu/AutoWideFn.h"
#include "editquery.h"
#include <time.h>

int queryEditOther(HWND hwnd, const char *query, const char *viewname, int mode);

extern int m_item_mode;
extern wchar_t m_item_name[256];
extern wchar_t m_item_query[1024];

LRESULT IPC_GetFileRatingW(INT_PTR fileName)
{
	int rating = 0;
	const wchar_t *filename = (const wchar_t *)fileName;
	if (!filename) return 0;
	if (g_table)
	{
		EnterCriticalSection(&g_db_cs);

		wchar_t filename2[MAX_PATH] = {0}; // full lfn path if set
		nde_scanner_t s = NDE_Table_CreateScanner(g_table);
		int found=FindFileInDatabase(s, MAINTABLE_ID_FILENAME, filename, filename2);

		if (found)
		{
			nde_field_t f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_RATING);
			if (f && NDE_Field_GetType(f) == FIELD_INTEGER)
			{
				rating = min(5, max(0, NDE_IntegerField_GetValue(f)));
			}
		}

		NDE_Table_DestroyScanner(g_table, s);

		LeaveCriticalSection(&g_db_cs);
	}
	return rating;
}

LRESULT IPC_SetFileRatingW(INT_PTR file_rating)
{
	const wchar_t *filename;
	int rating;
	int found = 0;

	file_set_ratingW *data = (file_set_ratingW*)file_rating;
	if (!data) return 0;

	filename = data->fileName;
	rating = min(5, max(0, data->newRating));

	if (!filename) return 0;
	if (g_table)
	{
		EnterCriticalSection(&g_db_cs);
		wchar_t filename2[MAX_PATH] = {0}; // full lfn path if set

		nde_scanner_t s = NDE_Table_CreateScanner(g_table);
		int found=FindFileInDatabase(s, MAINTABLE_ID_FILENAME, filename, filename2);
		if (!found)
		{
			int guess = -1, meta = -1, rec = 1;
			autoscan_add_directory(filename, &guess, &meta, &rec, 1); // use this folder's guess/meta options
			if (guess == -1) guess = g_config->ReadInt(L"guessmode", 0);
			if (meta == -1) meta = g_config->ReadInt(L"usemetadata", 1);
			addFileToDb(filename, 0, meta, guess, 1, (int)time(NULL));
		}
		if (filename2[0] && !found)
		{
			if (NDE_Scanner_LocateFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, filename2)) found = 2;
		}
		if (!found)
		{
			if (NDE_Scanner_LocateFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, filename)) found = 1;
		}
		if (found)
		{
			NDE_Scanner_Edit(s);
			db_setFieldInt(s, MAINTABLE_ID_RATING, rating);
			NDE_Scanner_Post(s);
			if (g_config->ReadInt(L"writeratings", 0))
			{
				wchar_t buf[64] = {0};
				if (rating > 0)
				{
					wsprintfW(buf, L"%d", rating);
				}
				else
					buf[0] = 0;
				updateFileInfo(filename, DB_FIELDNAME_rating, buf);
				SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_WRITE_EXTENDED_FILE_INFO);
			}
		}

		NDE_Table_DestroyScanner(g_table, s);

		LeaveCriticalSection(&g_db_cs);
	}
	return found != 0;
}


LRESULT IPC_GetFileRating(INT_PTR fileName)
{
	return IPC_GetFileRatingW((INT_PTR)(wchar_t *)AutoWide((const char *)fileName));
}

LRESULT IPC_SetFileRating(INT_PTR file_rating)
{
	file_set_rating *data = (file_set_rating*)file_rating;
	if (!data) return 0;
	file_set_ratingW dataW;
	AutoWide wideFn(data->fileName);
	dataW.fileName = wideFn;
	dataW.newRating = data->newRating;
	return IPC_SetFileRatingW((INT_PTR)&dataW);
}

int m_calling_getfileinfo;
int getFileInfo(const char *filename, char *metadata, char *dest, int len)
{
  m_calling_getfileinfo=1;
  dest[0]=0;
  extendedFileInfoStruct efis={
    filename,
    metadata,
    dest,
    len,
  };
  int r = SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efis,IPC_GET_EXTENDED_FILE_INFO); //will return 1 if wa2 supports this IPC call
  m_calling_getfileinfo=0;
  return r;
}

LRESULT IPC_GetFileInfo(INT_PTR param)
{
	if (!param) return 0;
	itemRecord *ent = (itemRecord *)param;
	if (!ent->filename || ent->filename && !*ent->filename) return 0;

	char filename2[MAX_PATH] = {0}; // full lfn path if set
	makeFilename2(ent->filename, filename2, ARRAYSIZE(filename2));

	if (!g_table) openDb();
	// first, check to see if it's in the db
	if (g_table)
	{
		EnterCriticalSection(&g_db_cs);

		nde_scanner_t s = NDE_Table_CreateScanner(g_table);

		if (NDE_Scanner_LocateFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, (AutoWideFn)(ent->filename)) ||
		    (filename2[0] && NDE_Scanner_LocateFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, (AutoWideFn)(filename2))))
		{
			nde_field_t f;
			// found it db, yay!
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_TITLE);
			if (!ent->title) ent->title = f ? AutoCharDup(NDE_StringField_GetString(f)) : 0;
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_ALBUM);
			if (!ent->album) ent->album = f ? AutoCharDup(NDE_StringField_GetString(f)) : 0;
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_ARTIST);
			if (!ent->artist) ent->artist = f ? AutoCharDup(NDE_StringField_GetString(f)) : 0;
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_GENRE);
			if (!ent->genre) ent->genre = f ? AutoCharDup(NDE_StringField_GetString(f)) : 0;
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_TRACKNB);
			if (ent->track <= 0) ent->track = f ? NDE_IntegerField_GetValue(f) : -1;
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_YEAR);
			if (ent->year <= 0) ent->year = f ? NDE_IntegerField_GetValue(f) : -1;
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_LENGTH);
			if (ent->length <= 0) ent->length = f ? NDE_IntegerField_GetValue(f) : -1;
			//ent->extended_info=NULL;//for now

			NDE_Table_DestroyScanner(g_table, s);
			LeaveCriticalSection(&g_db_cs);
			return 0;
		}
		NDE_Table_DestroyScanner(g_table, s);
		LeaveCriticalSection(&g_db_cs);
	}
	// if not, run the same shit as if we were adding it to the db!

	char *fnp = filename2[0] ? filename2 : ent->filename; // fnp is the file we will run through the guesswork

	char tmp[1024] = {0, };
	if (getFileInfo(fnp, "title", tmp, ARRAYSIZE(tmp) - 1))
	{
		if (tmp[0] && !ent->title)
		{
			ent->title = _strdup(tmp);
		}
		if (!ent->artist)
		{
			getFileInfo(ent->filename, "artist", tmp, ARRAYSIZE(tmp) - 1);
			if (tmp[0])
			{
				ent->artist = _strdup(tmp);
			}
		}
		if (!ent->album)
		{
			getFileInfo(ent->filename, "album", tmp, ARRAYSIZE(tmp) - 1);
			if (tmp[0])
			{
				ent->album = _strdup(tmp);
			}
		}
		if (ent->year <= 0)
		{
			getFileInfo(ent->filename, "year", tmp, ARRAYSIZE(tmp) - 1);
			if (tmp[0] && !strstr(tmp, "__") && !strstr(tmp, "/") && !strstr(tmp, "\\") && !strstr(tmp, "."))
			{
				char *p = tmp;
				while (p && *p)
				{
					if (*p == '_') *p = '0';
					p++;
				}
				int y = atoi(tmp);
				if (y != 0) ent->year = y;
			}
		}
		if (!ent->genre)
		{
			getFileInfo(ent->filename, "genre", tmp, ARRAYSIZE(tmp) - 1);
			if (tmp[0])
			{
				ent->genre = _strdup(tmp);
			}
		}
		if (ent->track <= 0)
		{
			getFileInfo(ent->filename, "track", tmp, ARRAYSIZE(tmp) - 1);
			if (tmp[0])
			{
				ent->track = atoi(tmp);
			}
		}
		if (ent->length <= 0)
		{
			getFileInfo(ent->filename, "length", tmp, ARRAYSIZE(tmp) - 1);
			if (tmp[0])
			{
				ent->length = atoi(tmp) / 1000;
			}
		}
	}
	if (!ent->title && !ent->artist && !ent->album && !ent->track)
	{
		int tn = 0;
		wchar_t *artist = 0, *album = 0, *title = 0;
		wchar_t *guessbuf = guessTitles(AutoWide(filename2[0] ? filename2 : ent->filename), &tn, &artist, &album, &title);
		if (guessbuf)
		{
			if (!ent->title && title)
			{
				ent->title = AutoCharDup(title);
			}
			if (!ent->artist && artist)
			{
				ent->artist = AutoCharDup(artist);
			}
			if (!ent->album && album)
			{
				ent->album = AutoCharDup(album);
			}
			if (ent->track <= 0 && tn)
			{
				ent->track = tn;
			}
			free(guessbuf);
		}
	}
	return 0;
}

int m_calling_getfileinfoW;
static int getFileInfoW(const wchar_t *filename, const wchar_t *metadata, wchar_t *dest, int len)
{
	m_calling_getfileinfoW=1;
	dest[0]=0;
	extendedFileInfoStructW efis={
		filename,
		metadata,
		dest,
		len,
	};
	int r = SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efis,IPC_GET_EXTENDED_FILE_INFOW); //will return 1 if wa2 supports this IPC call
	m_calling_getfileinfoW=0;
	return r;
}

LRESULT IPC_GetFileInfoW(INT_PTR param)
{
	if (!param) return 0;
	itemRecordW *ent = (itemRecordW *)param;
	if (!ent->filename || ent->filename && !*ent->filename) return 0;

	wchar_t filename2[MAX_PATH] = {0}; // full lfn path if set
	makeFilename2W(ent->filename, filename2, ARRAYSIZE(filename2));

	if (!g_table) openDb();
	// first, check to see if it's in the db
	if (g_table)
	{
		EnterCriticalSection(&g_db_cs);

		nde_scanner_t s = NDE_Table_CreateScanner(g_table);

		if (NDE_Scanner_LocateFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, ent->filename) ||
		    (filename2[0] && NDE_Scanner_LocateFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, filename2)))
		{
			nde_field_t f;
			// found it db, yay!
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_TITLE);
			if (!ent->title) ent->title = f ? NDE_StringField_GetString(f) : 0;
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_ALBUM);
			if (!ent->album) ent->album = f ? NDE_StringField_GetString(f) : 0;
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_ARTIST);
			if (!ent->artist) ent->artist = f ? NDE_StringField_GetString(f) : 0;
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_GENRE);
			if (!ent->genre) ent->genre = f ? NDE_StringField_GetString(f) : 0;
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_TRACKNB);
			if (ent->track <= 0) ent->track = f ? NDE_IntegerField_GetValue(f) : -1;
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_YEAR);
			if (ent->year <= 0) ent->year = f ? NDE_IntegerField_GetValue(f) : -1;
			f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_LENGTH);
			if (ent->length <= 0) ent->length = f ? NDE_IntegerField_GetValue(f) : -1;
			//ent->extended_info=NULL;//for now

			NDE_Table_DestroyScanner(g_table, s);
			LeaveCriticalSection(&g_db_cs);
			return 0;
		}
		NDE_Table_DestroyScanner(g_table, s);
		LeaveCriticalSection(&g_db_cs);
	}
	// if not, run the same shit as if we were adding it to the db!

	wchar_t *fnp = filename2[0] ? filename2 : ent->filename; // fnp is the file we will run through the guesswork

	wchar_t tmp[1024] = {0, };
	if (getFileInfoW(fnp, DB_FIELDNAME_title, tmp, ARRAYSIZE(tmp) - 1))
	{
		if (tmp[0] && !ent->title)
		{
			ent->title = _wcsdup(tmp);
		}
		if (!ent->artist)
		{
			getFileInfoW(ent->filename, DB_FIELDNAME_artist, tmp, ARRAYSIZE(tmp) - 1);
			if (tmp[0])
			{
				ent->artist = _wcsdup(tmp);
			}
		}
		if (!ent->album)
		{
			getFileInfoW(ent->filename, DB_FIELDNAME_album, tmp, ARRAYSIZE(tmp) - 1);
			if (tmp[0])
			{
				ent->album = _wcsdup(tmp);
			}
		}
		if (ent->year <= 0)
		{
			getFileInfoW(ent->filename, DB_FIELDNAME_year, tmp, ARRAYSIZE(tmp) - 1);
			if (tmp[0] && !wcsstr(tmp, L"__") && !wcsstr(tmp, L"/") && !wcsstr(tmp, L"\\") && !wcsstr(tmp, L"."))
			{
				wchar_t *p = tmp;
				while (p && *p)
				{
					if (*p == L'_') *p = L'0';
					p++;
				}
				int y = _wtoi(tmp);
				if (y != 0) ent->year = y;
			}
		}
		if (!ent->genre)
		{
			getFileInfoW(ent->filename, DB_FIELDNAME_genre, tmp, ARRAYSIZE(tmp) - 1);
			if (tmp[0])
			{
				ent->genre = _wcsdup(tmp);
			}
		}
		if (ent->track <= 0)
		{
			getFileInfoW(ent->filename, DB_FIELDNAME_track, tmp, ARRAYSIZE(tmp) - 1);
			if (tmp[0])
			{
				ent->track = _wtoi(tmp);
			}
		}
		if (ent->length <= 0)
		{
			getFileInfoW(ent->filename, DB_FIELDNAME_length, tmp, ARRAYSIZE(tmp) - 1);
			if (tmp[0])
			{
				ent->length = _wtoi(tmp) / 1000;
			}
		}
	}
	if (!ent->title && !ent->artist && !ent->album && !ent->track)
	{
		int tn = 0;
		wchar_t *artist = 0, *album = 0, *title = 0;
		wchar_t *guessbuf = guessTitles(filename2[0] ? filename2 : ent->filename, &tn, &artist, &album, &title);
		if (guessbuf)
		{
			if (!ent->title && title)
			{
				ent->title = _wcsdup(title);
			}
			if (!ent->artist && artist)
			{
				ent->artist = _wcsdup(artist);
			}
			if (!ent->album && album)
			{
				ent->album = _wcsdup(album);
			}
			if (ent->track <= 0 && tn)
			{
				ent->track = tn;
			}
			free(guessbuf);
		}
	}
	return 0;
}

LRESULT IPC_FreeFileInfo(INT_PTR param)
{
	itemRecord *ent = (itemRecord *)param;
	if (!param) return 0;
	if (ent->album)
	{
		free(ent->album); ent->album = NULL;
	}
	if (ent->artist)
	{
		free(ent->artist); ent->artist = NULL;
	}
	if (ent->comment)
	{
		free(ent->comment); ent->comment = NULL;
	}
	if (ent->genre)
	{
		free(ent->genre); ent->genre = NULL;
	}
	if (ent->title)
	{
		free(ent->title); ent->title = NULL;
	}
	//if (ent->extended_info) { free(ent->extended_info); ent->extended_info= NULL; }

	return 0;
}

LRESULT IPC_FreeFileInfoW(INT_PTR param)
{
	itemRecordW *ent = (itemRecordW *)param;
	if (!param) return 0;
	if (ent->album)
	{
		free(ent->album); ent->album = NULL;
	}
	if (ent->artist)
	{
		free(ent->artist); ent->artist = NULL;
	}
	if (ent->comment)
	{
		free(ent->comment); ent->comment = NULL;
	}
	if (ent->genre)
	{
		free(ent->genre); ent->genre = NULL;
	}
	if (ent->title)
	{
		free(ent->title); ent->title = NULL;
	}
	//if (ent->extended_info) { free(ent->extended_info); ent->extended_info= NULL; }

	return 0;
}

LRESULT IPC_EditView(INT_PTR param)
{
	static char dummyNameChar[256];
	static char dummyQueryChar[1024];
	ml_editview *mev = (ml_editview *)param;
	if (mev == NULL) return 0;
	if (queryEditOther(mev->dialog_parent, mev->query, mev->name, mev->mode))
	{
		mev->mode = m_item_mode;
		WideCharToMultiByteSZ(CP_ACP, 0, m_item_name, -1, dummyNameChar, 256, 0, 0);
		mev->name = dummyNameChar;
		WideCharToMultiByteSZ(CP_ACP, 0, m_item_query, -1, dummyQueryChar, 1024, 0, 0);
		mev->query = dummyQueryChar;
	}
	else
	{
		mev->dialog_parent = NULL;
		mev->mode = -1;
		mev->name = NULL;
		mev->query = NULL;
		return 0;
	}
	return 1;
}

LRESULT IPC_EditQuery(INT_PTR param)
{
	static char staticQuery[4096];
	ml_editquery *meq = (ml_editquery *)param;
	if (meq == NULL) return 0;
	wchar_t querybuf[4096] = {0};
	const wchar_t *newQuery = editQuery(meq->dialog_parent, AutoWide(meq->query), querybuf, 4096);
	WideCharToMultiByteSZ(CP_ACP, 0, newQuery, -1, staticQuery, 4096, 0, 0);
	meq->query = staticQuery;
	return meq->query != NULL;
}

bool FindFileInDB(nde_scanner_t s, const wchar_t *filename)
{
	wchar_t filename2[MAX_PATH] = {0}; // full lfn path if set
	return !!FindFileInDatabase(s, MAINTABLE_ID_FILENAME, filename, filename2);
}

INT_PTR HandleIpcMessage(INT_PTR msg, INT_PTR param)
{
	switch (msg)
	{
		/////////////////////// database API
	case ML_IPC_DB_RUNQUERYW:
	case ML_IPC_DB_RUNQUERY_SEARCHW:
	case ML_IPC_DB_RUNQUERY_FILENAMEW:
	case ML_IPC_DB_RUNQUERY_INDEXW:
		if (!g_table) openDb();
		if (param && g_table)
		{
			mlQueryStructW *p = (mlQueryStructW *)param;
			itemRecordListW *obj = &p->results;
			int failed = 0;

			EnterCriticalSection(&g_db_cs);

			nde_scanner_t s = NDE_Table_CreateScanner(g_table);
			if (msg == ML_IPC_DB_RUNQUERY_SEARCHW)
			{
				if (p->query)
				{
					GayStringW gs;
					makeQueryStringFromText(&gs, p->query);
					if (gs.Get() && gs.Get()[0]) NDE_Scanner_Query(s, gs.Get());
				}
				NDE_Scanner_First(s);
			}
			else if (msg == ML_IPC_DB_RUNQUERY_INDEXW)
			{
				failed=1;
			}
			else if (msg == ML_IPC_DB_RUNQUERY_FILENAMEW)
			{
				failed = 1;
				if (p->query)
				{
					if (NDE_Scanner_LocateFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, p->query))
					{
						failed = 0;
					}
				}
			}
			else
			{
				if (p->query) 
					NDE_Scanner_Query(s, p->query);
				NDE_Scanner_First(s);
			}

			if (!failed)
			{
				int cnt = 0;
				do
				{
					nde_field_t f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_FILENAME);
					if (!f) break;

					allocRecordList(obj, obj->Size + 1);
					if (!obj->Alloc) break;

					obj->Items[obj->Size].filename = NDE_StringField_GetString(f);
					ndestring_retain(obj->Items[obj->Size].filename);
					ScannerRefToObjCacheNFNW(s, obj, true);

					// this makes 0 give unlimited results, 1 give 1, and so on.
					if (++cnt == p->max_results || msg == ML_IPC_DB_RUNQUERY_FILENAMEW
					    /*|| msg == ML_IPC_DB_RUNQUERY_INDEX*/) break;

				}
				while (NDE_Scanner_Next(s));
			}

			NDE_Table_DestroyScanner(g_table, s);
			LeaveCriticalSection(&g_db_cs);

			compactRecordList(obj);

			return failed ? -1 : 1;
		}
		return -1;

	case ML_IPC_DB_RUNQUERY:
	case ML_IPC_DB_RUNQUERY_SEARCH:
	case ML_IPC_DB_RUNQUERY_FILENAME:
	case ML_IPC_DB_RUNQUERY_INDEX:
		{
			mlQueryStruct *p = (mlQueryStruct*)param;
			mlQueryStructW pW;
			AutoWide wideQuery(p->query);
			pW.query = wideQuery;
			pW.max_results = p->max_results;
			memset(&pW.results, 0, sizeof(pW.results));
			INT_PTR res = HandleIpcMessage(msg+0x1000, (INT_PTR)&pW); // convienently the W messages are 0x1000 higher
			itemRecordList *obj = &p->results;
			// append new results to old results
			for (int i=0;i!=pW.results.Size;i++)
			{
				allocRecordList(obj, obj->Size + 1);
				if (!obj->Alloc) break;
				convertRecord(&obj->Items[obj->Size++], &pW.results.Items[i]);
			}
			freeRecordList(&pW.results);
			return res;
		}
	case ML_IPC_DB_FREEQUERYRESULTS:
		if (param)
		{
			mlQueryStruct *p = (mlQueryStruct*)param;
			freeRecordList(&p->results);
			return 1;
		}
		return -1;
	case ML_IPC_DB_FREEQUERYRESULTSW:
		if (param)
		{
			mlQueryStructW *p = (mlQueryStructW*)param;
			freeRecordList(&p->results);
			return 1;
		}
		return -1;
	case ML_IPC_DB_REMOVEITEM:
	case ML_IPC_DB_REMOVEITEMW:
		if (!g_table) openDb();
		if (param && g_table)
		{
			wchar_t *filename = (((msg == ML_IPC_DB_REMOVEITEMW) ? (wchar_t*)param : AutoWideFn((char*)param)));
			int ret = ( RemoveFileFromDB(filename) == 0 ) ? 1 : -2;		// Change the return value from the stadard 0 == success, > 0 == fail
			return ret;
		}
		return -1;
	case ML_IPC_DB_UPDATEITEMW:
	case ML_IPC_DB_ADDORUPDATEITEMW:
		if (!g_table) openDb();
		if (param && g_table)
		{
			int ret = 1;
			itemRecordW *item = (itemRecordW*)param;
			if (!item->filename) return -1;

			EnterCriticalSection(&g_db_cs);
			nde_scanner_t s = NDE_Table_CreateScanner(g_table);

			if (FindFileInDB(s, item->filename))
			{
				NDE_Scanner_Edit(s);
			}
			else
			{
				if (msg == ML_IPC_DB_UPDATEITEMW) ret = -2;
				else
				{
					// new file
					NDE_Scanner_New(s);
					db_setFieldStringW(s, MAINTABLE_ID_FILENAME, item->filename);
					db_setFieldInt(s, MAINTABLE_ID_DATEADDED, (int)time(NULL));
				}
			}
			if (ret == 1)
			{
				// update applicable members
				if (item->title) db_setFieldStringW(s, MAINTABLE_ID_TITLE, item->title);
				if (item->album) db_setFieldStringW(s, MAINTABLE_ID_ALBUM, item->album);
				if (item->artist) db_setFieldStringW(s, MAINTABLE_ID_ARTIST, item->artist);
				if (item->comment) db_setFieldStringW(s, MAINTABLE_ID_COMMENT, item->comment);
				if (item->genre) db_setFieldStringW(s, MAINTABLE_ID_GENRE, item->genre);
				if (item->albumartist) db_setFieldStringW(s, MAINTABLE_ID_ALBUMARTIST, item->albumartist);
				if (item->replaygain_album_gain) db_setFieldStringW(s, MAINTABLE_ID_ALBUMGAIN, item->replaygain_album_gain);
				if (item->replaygain_track_gain) db_setFieldStringW(s, MAINTABLE_ID_TRACKGAIN, item->replaygain_track_gain);
				if (item->publisher) db_setFieldStringW(s, MAINTABLE_ID_PUBLISHER, item->publisher);
				if (item->composer) db_setFieldStringW(s, MAINTABLE_ID_COMPOSER, item->composer);
				if (item->category) db_setFieldStringW(s, MAINTABLE_ID_CATEGORY, item->category);
				if (item->year >= 0) db_setFieldInt(s, MAINTABLE_ID_YEAR, item->year);
				if (item->track >= 0) db_setFieldInt(s, MAINTABLE_ID_TRACKNB, item->track);
				if (item->tracks >= 0) db_setFieldInt(s, MAINTABLE_ID_TRACKS, item->tracks);
				if (item->length >= 0) db_setFieldInt(s, MAINTABLE_ID_LENGTH, item->length);
				if (item->rating >= 0) 
				{
					db_setFieldInt(s, MAINTABLE_ID_RATING, item->rating);
					if (g_config->ReadInt(L"writeratings", 0))
					{
						wchar_t buf[64] = {0};
						if (item->rating > 0)
						{
							wsprintfW(buf, L"%d", item->rating);
						}
						else
							buf[0] = 0;
						updateFileInfo(item->filename, DB_FIELDNAME_rating, buf);
						SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_WRITE_EXTENDED_FILE_INFO);
					}
				}
				if (item->lastplay >= 0) db_setFieldInt(s, MAINTABLE_ID_LASTPLAY, (int)item->lastplay);
				if (item->lastupd >= 0) db_setFieldInt(s, MAINTABLE_ID_LASTUPDTIME, (int)item->lastupd);
				if (item->filetime >= 0) db_setFieldInt(s, MAINTABLE_ID_FILETIME, (int)item->filetime);
				if (item->filesize >= 0)
				{
					// changed in 5.64 to use the 'realsize' if it's available, otherwise map to filesize scaled to bytes (was stored as kb previously)
					const wchar_t *realsize = getRecordExtendedItem(item, L"realsize");
					if (realsize) db_setFieldInt64(s, MAINTABLE_ID_FILESIZE, _wtoi64(realsize));
					else db_setFieldInt(s, MAINTABLE_ID_FILESIZE, item->filesize * 1024);
				}
				if (item->bitrate >= 0) db_setFieldInt(s, MAINTABLE_ID_BITRATE, item->bitrate);
				if (item->type >= 0) db_setFieldInt(s, MAINTABLE_ID_TYPE, item->type);
				if (item->disc >= 0) db_setFieldInt(s, MAINTABLE_ID_DISC, item->disc);
				if (item->discs >= 0) db_setFieldInt(s, MAINTABLE_ID_DISCS, item->discs);
				if (item->bpm >= 0) db_setFieldInt(s, MAINTABLE_ID_BPM, item->bpm);
				// give a default playcnt of 0 if we're creating a new one and the caller
				// didn't specify a playcnt
				if (item->playcount >= 0)
					db_setFieldInt(s, MAINTABLE_ID_PLAYCOUNT, item->playcount);
				else
					db_setFieldInt(s, MAINTABLE_ID_PLAYCOUNT, 0);

				for (int x = 0; x < NUM_EXTRA_COLSW; x ++)
				{
					wchar_t *at = getRecordExtendedItem(item, extra_strsW[x]); // can't use _fast here because it's not our itemRecordW
					if (at)
					{
						switch (extra_idsW[x])
						{
						case MAINTABLE_ID_LOSSLESS:
						case MAINTABLE_ID_PODCASTPUBDATE:
						case MAINTABLE_ID_ISPODCAST:
						case MAINTABLE_ID_WIDTH:
						case MAINTABLE_ID_HEIGHT:
							if (*at)
								db_setFieldInt(s, extra_idsW[x], _wtoi(at));
							break;
						case MAINTABLE_ID_GRACENOTEFILEID:
						case MAINTABLE_ID_GRACENOTEEXTDATA:
						case MAINTABLE_ID_PODCASTCHANNEL:
						case MAINTABLE_ID_CODEC:
						case MAINTABLE_ID_DIRECTOR:
						case MAINTABLE_ID_PRODUCER:
							db_setFieldStringW(s, extra_idsW[x], at);
							break;
						}
					}
				}

				NDE_Scanner_Post(s);
				g_table_dirty++;
			}
			NDE_Table_DestroyScanner(g_table, s);
			LeaveCriticalSection(&g_db_cs);

			return ret;
		}
		return -1;

	case ML_IPC_DB_UPDATEITEM:
	case ML_IPC_DB_ADDORUPDATEITEM:
		{
			itemRecord *item = (itemRecord*)param;
			if (!item || !item->filename) 
				return -1;
			itemRecordW wideRecord;
			memset(&wideRecord, 0, sizeof(wideRecord));
			convertRecord(&wideRecord, item);
			INT_PTR res = HandleIpcMessage(msg+0x1000, (INT_PTR)&wideRecord); // unicode message values are convienantly 0x1000 higher here
			freeRecord(&wideRecord);
			return res;
		}
	
	case ML_IPC_DB_UPDATEFILEW:
	case ML_IPC_DB_ADDORUPDATEFILEW:
		if (!g_table) openDb();
		if (param && g_table)
		{
			LMDB_FILE_ADD_INFOW *fi = (LMDB_FILE_ADD_INFOW *)param;
			if (!fi || !fi->fileName) return -1;

			int guess  = (fi->gues_mode == -1) ? g_config->ReadInt(L"guessmode", 0) : fi->gues_mode;
			int meta = (fi->meta_mode == -1) ? g_config->ReadInt(L"usemetadata", 1) : fi->meta_mode;
			return addFileToDb(fi->fileName, msg == ML_IPC_DB_UPDATEFILEW ? 1 : 0, meta, guess);
		}
		return -1;
	case ML_IPC_DB_UPDATEFILE:
	case ML_IPC_DB_ADDORUPDATEFILE:
		if (!g_table) openDb();
		if (param && g_table)
		{
			LMDB_FILE_ADD_INFO *fi = (LMDB_FILE_ADD_INFO*)param;
			if (!fi || !fi->fileName) return -1;

			int guess  = (fi->gues_mode == -1) ? g_config->ReadInt(L"guessmode", 0) : fi->gues_mode;
			int meta = (fi->meta_mode == -1) ? g_config->ReadInt(L"usemetadata", 1) : fi->meta_mode;
			return addFileToDb(AutoWide(fi->fileName), msg == ML_IPC_DB_UPDATEFILE ? 1 : 0, meta, guess);
		}
		return -1;
	case ML_IPC_DB_SYNCDB:
		if (!g_table) openDb();
		if (g_table && g_table_dirty)
		{
			EnterCriticalSection(&g_db_cs);
			g_table_dirty = 0;
			NDE_Table_Sync(g_table);
			LeaveCriticalSection(&g_db_cs);
			return 1;
		}
		return -1;

	case ML_IPC_GET_FILE_RATINGW:		return IPC_GetFileRatingW(param);
	case ML_IPC_SET_FILE_RATINGW:		return IPC_SetFileRatingW(param);
	case ML_IPC_GET_FILE_RATING:		return IPC_GetFileRating(param);
	case ML_IPC_SET_FILE_RATING:		return IPC_SetFileRating(param);
	case ML_IPC_FREEFILEINFO:			return IPC_FreeFileInfo(param);
	case ML_IPC_FREEFILEINFOW:			return IPC_FreeFileInfoW(param);
	case ML_IPC_GETFILEINFO:			return IPC_GetFileInfo(param);
	case ML_IPC_GETFILEINFOW:			return IPC_GetFileInfoW(param);
	case ML_IPC_PLAY_PLAYLIST:
	case ML_IPC_LOAD_PLAYLIST:
	{
		// play/load the playlist passed as param
		queryItem *item = m_query_list[param];
		if (item)
		{
			wchar_t configDir[MAX_PATH] = {0};
			PathCombineW(configDir, g_viewsDir, item->metafn);
			C_Config viewconf(configDir);
			main_playQuery(&viewconf, item->query, 0, msg == ML_IPC_PLAY_PLAYLIST);
		}
	}
	break;
	case ML_IPC_EDITVIEW:			return IPC_EditView(param);
	case ML_IPC_EDITQUERY:			return IPC_EditQuery(param);
	case ML_IPC_SMARTVIEW_COUNT:
		return m_query_list.size();
	case ML_IPC_SMARTVIEW_INFO:
		if (param)
		{
			mlSmartViewInfo * v = (mlSmartViewInfo *)param;
			if (v->size < sizeof(mlSmartViewInfo)) break;
			if (v->smartViewNum >= m_query_list.size()) break;
			
			auto it = m_query_list.begin();
			while (v->smartViewNum--)
			{
				it++;
			}
			queryItem* q = it->second; //(m_query_list.begin()+v->smartViewNum)->second;

			if (!q) break;
			if (!q->name || !q->query) break;
			v->treeItemId = it->first; //(m_query_list.begin()+v->smartViewNum)->first;
			v->iconImgIndex = q->imgIndex;
			v->mode = q->mode;
			lstrcpynW(v->smartViewName,q->name,sizeof(v->smartViewName)/sizeof(wchar_t));
			lstrcpynW(v->smartViewQuery,q->query,sizeof(v->smartViewQuery)/sizeof(wchar_t));
			return 1;
		}
		break;
	case ML_IPC_SMARTVIEW_ADD:
		if (param)
		{
			mlSmartViewInfo * v = (mlSmartViewInfo *)param;
			if (v->size < sizeof(mlSmartViewInfo)) break;
			v->treeItemId = addQueryItem(v->smartViewName,v->smartViewQuery,v->mode,0,NULL,v->iconImgIndex, v->smartViewNum);
			saveQueryTree();
			return 1;
		}
		break;
	}
	return 0;
}