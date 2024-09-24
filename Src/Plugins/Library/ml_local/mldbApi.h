#pragma once

#include "api_mldb.h"

class MLDBAPI : public api_mldb
{
public:
	itemRecordW *GetFile(const wchar_t *filename);
	itemRecordW *GetFileIf(const wchar_t *filename, const wchar_t *query);
	itemRecordListW *GetAlbum(const wchar_t *albumname, const wchar_t *albumartist);
	itemRecordListW *Query(const wchar_t *query);
	itemRecordListW *QueryLimit(const wchar_t *query, unsigned int limit);

	void SetField(const wchar_t *filename, const char *field, const wchar_t *value);
	void SetFieldInteger(const wchar_t *filename, const char *field, int value);
	void SetFieldInt128(const wchar_t *filename, const char *field, uint8_t value[16]);
	void Sync();

	int AddFile(const wchar_t *filename);

	void FreeRecord(itemRecordW *record);
	void FreeRecordList(itemRecordListW *recordList);
	int RemoveFile(const wchar_t *filename);

	/* wrappers around ndestring */
	void RetainString(wchar_t *str);
	void ReleaseString(wchar_t *str);
	wchar_t *DuplicateString(const wchar_t *str);

		int GetMaxInteger(const char *field, int *max);
protected:
	RECVS_DISPATCH;
};

