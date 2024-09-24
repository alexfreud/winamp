/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 Table Class Prototypes
 Windows implementation
--------------------------------------------------------------------------- */

#ifndef __TABLE_H
#define __TABLE_H

#include <stdio.h>
//#include <io.h>
#include "../Scanner.h"
#include <map>
#include "../IndexRecord.h"
#include <assert.h>

class Table : private Scanner
{
public:
	// TODO: move these back to protected
	VFILE *Handle = NULL;
	using Scanner::index;
	bool use_row_cache   = false;
	BOOL GLocateUpToDate = FALSE;

private:
	void Init();
	void Reset();

private:
	LinkedList *Scanners;

protected:
	wchar_t *Name;
	wchar_t *IdxName;

	VFILE        *IdxHandle = NULL;
	BOOL          AutoCreate;
	Record       *FieldsRecord = NULL;
	IndexRecord  *IndexList = NULL;
	Database     *db;
	BOOL          Cached;
	int           numErrors = 0;
	using Scanner::Edition;
	bool          columns_cached = false;
	unsigned char column_ids[256];
	typedef std::map<int, Record*> RowCache;
	RowCache rowCache;

	// Tables
	static bool Compact_ColumnWalk(Record *record, Field *entry, void *context_in);
	static bool Compact_ColumnWalk2(Record *record, Field *entry, void *context_in);
	static bool Compact_IndexWalk(Table *table, IndexField *entry, void *context);
	static bool IndexWriteWalker(IndexRecord *record, Field *entry, void *context);
	static bool IndexWalkerThunk(IndexRecord *record, Field *entry, void *context);
	static bool IndexNewWalker(IndexRecord *record, Field *entry, void *context);
	static bool BuildColumnCache(Record *record, Field *entry, void *context);

public:
	typedef bool (*IndexWalker)(Table *table, IndexField *entry, void *context);
	Table(const wchar_t *TableName, const wchar_t *IdxName, BOOL Create, Database *db, BOOL Cached);
	~Table();
	BOOL Open(void);
	void Close(void);

	// Columns
	ColumnField *NewColumn(unsigned char Id, const wchar_t *name, unsigned char type, BOOL indexUniques);

	void DeleteColumn(ColumnField *field); // todo
	void DeleteColumnByName(const wchar_t *name); // todo
	void DeleteColumnById(unsigned char Id); // todo
	void PostColumns(void);
	NDE_API Record *GetColumns(void);
	ColumnField *GetColumnByName(const wchar_t *FieldName);
	ColumnField *GetColumnById(unsigned char Idx);
	unsigned char GetColumnType(unsigned char Id);

	// Fields
	using Scanner::NewFieldByName;
	using Scanner::NewFieldById;
	using Scanner::GetFieldByName;
	using Scanner::GetFieldById;
	using Scanner::DeleteField;
	using Scanner::DeleteFieldByName;
	using Scanner::DeleteFieldById;

	// Records
	using Scanner::First;
	using Scanner::Last;
	using Scanner::Next;
	using Scanner::Previous;
	using Scanner::Eof;
	using Scanner::Bof;
	using Scanner::New;
	using Scanner::Insert;
	using Scanner::Edit;
	using Scanner::Cancel;
	using Scanner::Post;
	using Scanner::Delete;
	using Scanner::GetRecordsCount;
	using Scanner::GetRecordById;
	using Scanner::GetRecordId;
	void Sync(void);
	using Scanner::LocateByName;
	using Scanner::LocateById;
	BOOL LocateByIdEx(int Id, int From, Field *field, int comp_mode);

	// Indexes
	void AddIndexByName(const wchar_t *FieldName, const wchar_t *KeyName);
	void AddIndexById(unsigned char Id, const wchar_t *KeyName);

	void WalkIndices(IndexWalker callback, void *context);

	IndexField *GetIndexByName(const wchar_t *name);
	IndexField *GetIndexById(unsigned char Id);
	using Scanner::SetWorkingIndexByName;
	using Scanner::SetWorkingIndexById;
	NDE_API BOOL CheckIndexing(void);
	void DropIndexByName(const wchar_t *desc);
	void DropIndexById(unsigned char Id);
	void DropIndex(IndexField *Ptr);
	void IndexModified(void);

	// Filters
	using Scanner::AddFilterByName;
	using Scanner::AddFilterById;
	using Scanner::AddFilterOp;
	using Scanner::RemoveFilters;

	// Scanners
	Scanner *NewScanner();
	Scanner *GetDefaultScanner();
	void DeleteScanner(Scanner *scan);

	// Misc
	using Scanner::FragmentationLevel;
	void Compact(int *progress = NULL);
	void SetGlobalLocateUpToDate(BOOL is);

	// Row Cache
	void RowCache_Delete(int position);
	void RowCache_Remove(int position);
	void RowCache_Add(Record *record, int position);
	Record *RowCache_Get(int position);
	NDE_API void EnableRowCache();

	// Searching
	void SetFieldSearchableById(unsigned char field_id, bool searchable);

	int HasErrors()
	{
		return numErrors > 0;
	}
	int NumErrors()
	{
		return numErrors;
	}
	void IncErrorCount()
	{
		numErrors++;
	}
};

#endif