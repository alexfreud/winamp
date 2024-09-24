#pragma once

#include <bfc/dispatch.h>
#include "..\..\General\gen_ml/ml.h"

class api_mldb : public Dispatchable
{
protected:
	api_mldb() {}
	~api_mldb() {}
public:
	itemRecordW *GetFile(const wchar_t *filename);
	itemRecordW *GetFileIf(const wchar_t *filename, const wchar_t *query); // returns the item record for a filename, but also checks against the passed query
	itemRecordListW *GetAlbum(const wchar_t *albumname, const wchar_t *albumartist);
	itemRecordListW *Query(const wchar_t *query);
	itemRecordListW *QueryLimit(const wchar_t *query, unsigned int limit);

	void SetField(const wchar_t *filename, const char *field, const wchar_t *value);
	void SetFieldInteger(const wchar_t *filename, const char *field, int value);
	void SetFieldInt128(const wchar_t *filename, const char *field, uint8_t value[16]);
	void Sync();

	void FreeRecord(itemRecordW *record);
	void FreeRecordList(itemRecordListW *recordList);

	int AddFile(const wchar_t *filename);
	int RemoveFile(const wchar_t *filename);

	/* wrappers around ndestring */
	void RetainString(wchar_t *str);
	void ReleaseString(wchar_t *str);
	wchar_t *DuplicateString(const wchar_t *str);

	int GetMaxInteger(const char *field, int *max);

	DISPATCH_CODES
	{
		API_MLDB_GETFILE = 10,
		API_MLDB_GETFILEIF = 11,
		API_MLDB_GETALBUM = 20,
		API_MLDB_QUERY = 30,
		API_MLDB_QUERYLIMIT = 31,
		API_MLDB_FREERECORD = 40,
		API_MLDB_FREERECORDLIST = 50,
		API_MLDB_SETFIELD = 60,
		API_MLDB_SETFIELDINT = 61,
		API_MLDB_SETFIELDINT128 = 62,
		API_MLDB_SYNC = 70,
		API_MLDB_ADDFILE = 80,
		API_MLDB_REMOVEFILE = 90,
		API_MLDB_RETAINSTRING = 100,
		API_MLDB_RELEASESTRING = 110,
		API_MLDB_DUPLICATESTRING = 120,
		API_MLDB_GETMAXINTEGER = 130,
	};

	typedef struct
	{
		time_t played;
		int count;
	} played_info;

	enum 
	{
		// System callbacks for api_mldb
		SYSCALLBACK = MK4CC('m','l','d','b'),	// Unique identifier for mldb_api callbacks
		MLDB_FILE_ADDED = 10,					// param1 = filename, param2 = (not used), Callback event for when a new file is added to the local mldb
		MLDB_FILE_REMOVED_PRE = 20,				// param1 = filename, param2 = (not used), Callback event for when a file is removed from the local mldb (before it happens)
		MLDB_FILE_REMOVED_POST = 25,			// param1 = filename, param2 = (not used), Callback event for when a file is removed from the local mldb (after it happens)
		MLDB_FILE_UPDATED = 30,					// param1 = filename, param2 = (not used), Callback event for when a file is modified in the local mldb
		MLDB_FILE_UPDATED_EXTERNAL = 35,		// param1 = filename, param2 = (not used), Callback event for when a file is modified and is not in the local mldb
		MLDB_CLEARED = 40,						// param1 = filenames, param2 = count, Callback event for when the local mldb is cleared (useful so removed is not triggered for all files)
		MLDB_FILE_PLAYED = 50,					// param1 = filename, param2 = played_info, Callback event for when a file is tracked as playing in the local mldb
		MLDB_FILE_GET_CLOUD_STATUS = 60,		// param1 = filename, param2 = HMENU*, Callback event for when ml_local needs to show a cloud status menu (returned by the handler in param2)
		MLDB_FILE_PROCESS_CLOUD_STATUS = 65,	// param1 = menu_item, param2 = int*, Callback event for when ml_local needs to process the result of a cloud status menu
	};
};

inline itemRecordW *api_mldb::GetFile(const wchar_t *filename)
{
	return _call(API_MLDB_GETFILE, (itemRecordW *)0, filename);
}

inline itemRecordW *api_mldb::GetFileIf(const wchar_t *filename, const wchar_t *query)
{
	return _call(API_MLDB_GETFILEIF, (itemRecordW *)0, filename, query);
}

inline itemRecordListW *api_mldb::GetAlbum(const wchar_t *albumname, const wchar_t *albumartist)
{
	return _call(API_MLDB_GETALBUM, (itemRecordListW *)0, albumname, albumartist);
}

inline itemRecordListW *api_mldb::Query(const wchar_t *query)
{
	return _call(API_MLDB_QUERY, (itemRecordListW *)0, query);
}

inline itemRecordListW *api_mldb::QueryLimit(const wchar_t *query, unsigned int limit)
{
	return _call(API_MLDB_QUERYLIMIT, (itemRecordListW *)0, query, limit);
}

inline void api_mldb::FreeRecord(itemRecordW *record)
{
	_voidcall(API_MLDB_FREERECORD, record);
}

inline void api_mldb::FreeRecordList(itemRecordListW *recordList)
{
	_voidcall(API_MLDB_FREERECORDLIST, recordList);
}

inline void api_mldb::SetField(const wchar_t *filename, const char *field, const wchar_t *value)
{
	_voidcall(API_MLDB_SETFIELD, filename, field, value);
}

inline void api_mldb::SetFieldInteger(const wchar_t *filename, const char *field, int value)
{
	_voidcall(API_MLDB_SETFIELDINT, filename, field, value);
}

inline void api_mldb::SetFieldInt128(const wchar_t *filename, const char *field, uint8_t value[16])
{
	_voidcall(API_MLDB_SETFIELDINT128, filename, field, value);
}

inline void api_mldb::Sync()
{
	_voidcall(API_MLDB_SYNC);
}

inline int api_mldb::AddFile(const wchar_t *filename)
{
	return _call(API_MLDB_ADDFILE, (int)0, filename);
}

inline int api_mldb::RemoveFile(const wchar_t *filename)
{
	return _call(API_MLDB_REMOVEFILE, (int)0, filename);
}

inline void api_mldb::RetainString(wchar_t *str)
{
	_voidcall(API_MLDB_RETAINSTRING, str);
}

inline void api_mldb::ReleaseString(wchar_t *str)
{
	_voidcall(API_MLDB_RELEASESTRING, str);
}

inline wchar_t *api_mldb::DuplicateString(const wchar_t *str)
{
	return _call(API_MLDB_DUPLICATESTRING, (wchar_t *)0, str);
}

inline int api_mldb::GetMaxInteger(const char *field, int *max)
{
	return _call(API_MLDB_GETMAXINTEGER, (int)1, field, max);
}

// {5A94DABC-E19A-4a12-9AA8-852D8BF06532}
static const GUID mldbApiGuid = 
{ 0x5a94dabc, 0xe19a, 0x4a12, { 0x9a, 0xa8, 0x85, 0x2d, 0x8b, 0xf0, 0x65, 0x32 } };
