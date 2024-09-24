#include "mldbApi.h"
#include "main.h"
#include <strsafe.h>

itemRecordW *MLDBAPI::GetFile(const wchar_t *filename)
{
	itemRecordW *result = 0;
	if (filename)
	{
		openDb(); // just in case it's not opened yet (this function will return immediately if it's already open)
		if (g_table)
		{
			EnterCriticalSection(&g_db_cs);
			nde_scanner_t s = NDE_Table_CreateScanner(g_table);

			if (s)
			{
				if (NDE_Scanner_LocateFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, filename))
				{
					nde_field_t f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_FILENAME);
					if (f)
					{
						result = (itemRecordW *)_aligned_malloc(sizeof(itemRecordW), 16);
						if (result)
						{
							result->filename = NDE_StringField_GetString(f);
							ndestring_retain(result->filename);
							ScannerRefToObjCacheNFNW(s, result, true);
						}
					}
				}
				NDE_Table_DestroyScanner(g_table, s);
			}
			LeaveCriticalSection(&g_db_cs);
		}
	}
	return result;
}

itemRecordW *MLDBAPI::GetFileIf(const wchar_t *filename, const wchar_t *query)
{
	itemRecordW *result = 0;
	if (filename)
	{
		openDb(); // just in case it's not opened yet (this function will return immediately if it's already open)
		if (g_table)
		{
			EnterCriticalSection(&g_db_cs);
			nde_scanner_t s = NDE_Table_CreateScanner(g_table);

			if (s)
			{
				NDE_Scanner_Query(s, query);
				if (NDE_Scanner_LocateFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, filename))
				{
					nde_field_t f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_FILENAME);
					if (f)
					{
						result = (itemRecordW *)_aligned_malloc(sizeof(itemRecordW), 16);
						if (result)
						{
							result->filename = NDE_StringField_GetString(f);
							ndestring_retain(result->filename);
							ScannerRefToObjCacheNFNW(s, result, true);
						}
					}
				}
				NDE_Table_DestroyScanner(g_table, s);
			}
			LeaveCriticalSection(&g_db_cs);
		}
	}
	return result;
}

itemRecordListW *MLDBAPI::GetAlbum(const wchar_t *albumname, const wchar_t *albumartist)
{
	wchar_t query[4096] = {0}; // hope it's big enough
	if (albumartist && albumname)
	{
		StringCchPrintfW(query, 4096, L"((albumartist isempty and artist=\"%s\") or albumartist=\"%s\") and album=\"%s\"", albumartist, albumartist, albumname);
		return Query(query);
	}
	else if (albumname)
	{
		StringCchPrintfW(query, 4096, L"album=\"%s\"", albumname);
		return Query(query);
	}
	return 0;
}

itemRecordListW *MLDBAPI::Query(const wchar_t *query)
{
	itemRecordListW *result = 0;

	if (query)
	{
		openDb(); // just in case it's not opened yet (this function will return immediately if it's already open)
		if (g_table)
		{
			EnterCriticalSection(&g_db_cs);
			nde_scanner_t s = NDE_Table_CreateScanner(g_table);
			if (s)
			{
				NDE_Scanner_Query(s, query);
				NDE_Scanner_First(s);

				do
				{
					nde_field_t f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_FILENAME);
					if (!f) break;

					if (!result)
						result = (itemRecordListW *)calloc(1, sizeof(itemRecordListW));

					if (!result)
						break;

					allocRecordList(result, result->Size + 1);
					if (!result->Alloc) break;

					result->Items[result->Size].filename = NDE_StringField_GetString(f);
					ndestring_retain(result->Items[result->Size].filename);
					ScannerRefToObjCacheNFNW(s, result, true);
				}
				while (NDE_Scanner_Next(s));

				NDE_Table_DestroyScanner(g_table, s);
			}
			LeaveCriticalSection(&g_db_cs);
		}
	}
	return result;
}

itemRecordListW *MLDBAPI::QueryLimit(const wchar_t *query, unsigned int limit)
{
	itemRecordListW *result = 0;
	if (limit == 0)
		return 0;
	if (query)
	{
		openDb(); // just in case it's not opened yet (this function will return immediately if it's already open)
		if (g_table)
		{
			EnterCriticalSection(&g_db_cs);
			nde_scanner_t s = NDE_Table_CreateScanner(g_table);
			if (s)
			{
				NDE_Scanner_Query(s, query);
				NDE_Scanner_First(s);

				do
				{
					nde_field_t f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_FILENAME);
					if (!f) break;

					if (!result)
					{
						result = (itemRecordListW *)calloc(1, sizeof(itemRecordListW));
						if (!result)
							break;

						allocRecordList(result, limit);
						if (!result->Alloc) 
							break;
					}			

					result->Items[result->Size].filename = NDE_StringField_GetString(f);
					ndestring_retain(result->Items[result->Size].filename);
					ScannerRefToObjCacheNFNW(s, result, true);
				}
				while (result->Size < (int)limit && NDE_Scanner_Next(s));

				NDE_Table_DestroyScanner(g_table, s);
			}
			LeaveCriticalSection(&g_db_cs);
		}
	}
	return result;
}

void MLDBAPI::FreeRecord(itemRecordW *record)
{
	freeRecord(record);
}

void MLDBAPI::FreeRecordList(itemRecordListW *recordList)
{
	freeRecordList(recordList);
}

bool FindFileInDB(nde_scanner_t s, const wchar_t *filename);
void MLDBAPI::SetField(const wchar_t *filename, const char *field, const wchar_t *value)
{
	openDb();
	if (g_table)
	{
		EnterCriticalSection(&g_db_cs);
		nde_scanner_t s = NDE_Table_CreateScanner(g_table);
		if (s)
		{
			if (FindFileInDB(s, filename))
			{				
				if (value && value[0])
				{
					NDE_Scanner_Edit(s);
					nde_field_t f = NDE_Scanner_GetFieldByName(s, field);
					if (!f) 
						f=NDE_Scanner_NewFieldByName(s, field);

					if (f)
						NDE_StringField_SetString(f, value);
				}
				else
				{
					nde_field_t f = NDE_Scanner_GetFieldByName(s, field);
					if (f)
						NDE_Scanner_DeleteField(s, f);
				}
				NDE_Scanner_Post(s);
				g_table_dirty++;
			}
			NDE_Table_DestroyScanner(g_table, s);
		}
		LeaveCriticalSection(&g_db_cs);
	}
}

void MLDBAPI::SetFieldInteger(const wchar_t *filename, const char *field, int value)
{
	openDb();
	if (g_table)
	{
		EnterCriticalSection(&g_db_cs);
		nde_scanner_t s = NDE_Table_CreateScanner(g_table);
		if (s)
		{
			if (FindFileInDB(s, filename))
			{				
				NDE_Scanner_Edit(s);
				nde_field_t f = NDE_Scanner_GetFieldByName(s, field);
				if (!f) 
					f=NDE_Scanner_NewFieldByName(s, field);

				if (f)
					NDE_IntegerField_SetValue(f, value);

				NDE_Scanner_Post(s);
				g_table_dirty++;
			}
			NDE_Table_DestroyScanner(g_table, s);
		}
		LeaveCriticalSection(&g_db_cs);
	}
}

void MLDBAPI::SetFieldInt128(const wchar_t *filename, const char *field, uint8_t value[16])
{
	openDb();
	if (g_table)
	{
		EnterCriticalSection(&g_db_cs);
		nde_scanner_t s = NDE_Table_CreateScanner(g_table);
		if (s)
		{
			if (FindFileInDB(s, filename))
			{				
				if (value)
				{
					NDE_Scanner_Edit(s);
					nde_field_t f = NDE_Scanner_GetFieldByName(s, field);
					if (!f) 
						f=NDE_Scanner_NewFieldByName(s, field);

					if (f)
						NDE_Int128Field_SetValue(f, value);
				}
				else
				{
					nde_field_t f = NDE_Scanner_GetFieldByName(s, field);
					if (f)
						NDE_Scanner_DeleteField(s, f);
				}
				NDE_Scanner_Post(s);
				g_table_dirty++;
			}
			NDE_Table_DestroyScanner(g_table, s);
		}
		LeaveCriticalSection(&g_db_cs);
	}
}

void MLDBAPI::Sync()
{
	openDb();
	if (g_table)
	{
		EnterCriticalSection(&g_db_cs);
		NDE_Table_Sync(g_table);
		g_table_dirty=0;
		LeaveCriticalSection(&g_db_cs);
	}
}

int MLDBAPI::AddFile(const wchar_t *filename)
{
	int guess = g_config->ReadInt(L"guessmode", 0);
	int meta = g_config->ReadInt(L"usemetadata", 1);
	return addFileToDb(filename, 0, meta, guess);
}

int MLDBAPI::RemoveFile(const wchar_t *filename)
{
	return RemoveFileFromDB(filename);
}

void MLDBAPI::RetainString(wchar_t *str)
{
	ndestring_retain(str);
}

void MLDBAPI::ReleaseString(wchar_t *str)
{
	ndestring_release(str);
}

wchar_t *MLDBAPI::DuplicateString(const wchar_t *str)
{
	return ndestring_wcsdup(str);
}

int MLDBAPI::GetMaxInteger(const char *field, int *max)
{
	openDb();
	if (!g_table)
		return 1;

	EnterCriticalSection(&g_db_cs);
	nde_field_t f = NDE_Table_GetColumnByName(g_table, field);
	if (!f)
	{
		LeaveCriticalSection(&g_db_cs);
		return 1;
	}

	unsigned char field_id = NDE_ColumnField_GetFieldID(f);
	nde_scanner_t s = NDE_Table_CreateScanner(g_table);
	if (s)
	{
		int maximum_so_far=0;
		NDE_Scanner_Query(s, L"");
		NDE_Scanner_First(s);
		do
		{
			nde_field_t f = NDE_Scanner_GetFieldByID(s, field_id);
			if (f) 
			{
				int this_value = NDE_IntegerField_GetValue(f);
				if (this_value > maximum_so_far)
					maximum_so_far = this_value;
			}

		} while (NDE_Scanner_Next(s) && !NDE_Scanner_EOF(s));

		NDE_Table_DestroyScanner(g_table, s);
		*max=maximum_so_far;
		LeaveCriticalSection(&g_db_cs);
		return 0;
	}
	else
	{
		LeaveCriticalSection(&g_db_cs);
		return 1;
	}	
}

#define CBCLASS MLDBAPI
START_DISPATCH;
CB(API_MLDB_GETFILE, GetFile)
CB(API_MLDB_GETFILEIF, GetFileIf)
CB(API_MLDB_GETALBUM, GetAlbum)
CB(API_MLDB_QUERY, Query)
CB(API_MLDB_QUERYLIMIT, QueryLimit)
VCB(API_MLDB_FREERECORD, FreeRecord)
VCB(API_MLDB_FREERECORDLIST, FreeRecordList)
VCB(API_MLDB_SETFIELD, SetField)
VCB(API_MLDB_SETFIELDINT, SetFieldInteger)
VCB(API_MLDB_SETFIELDINT128, SetFieldInt128)
VCB(API_MLDB_SYNC, Sync)
CB(API_MLDB_ADDFILE, AddFile)
CB(API_MLDB_REMOVEFILE, RemoveFile)
VCB(API_MLDB_RETAINSTRING, RetainString)
VCB(API_MLDB_RELEASESTRING, ReleaseString)
CB(API_MLDB_DUPLICATESTRING, DuplicateString)
CB(API_MLDB_GETMAXINTEGER, GetMaxInteger)
END_DISPATCH;
#undef CBCLASS

