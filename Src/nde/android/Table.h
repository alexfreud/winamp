/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 Table Class Prototypes
 Android (linux) implementation
--------------------------------------------------------------------------- */

#ifndef __TABLE_H
#define __TABLE_H

#include <stdio.h>
#include "Scanner.h"
#include <map>
#include "IndexRecord.h"

class Table : private Scanner
{
public:
	// TODO: move these back to protected
	VFILE *Handle;
	using Scanner::index;
	bool use_row_cache;
	bool GLocateUpToDate;
private:
	void Init();
	void Reset();

private:
	LinkedList *Scanners;

protected:
	char *Name;
	char *IdxName;

	VFILE *IdxHandle;
	int AutoCreate;
	Record *FieldsRecord;
	IndexRecord *IndexList;
	Database *db;
	int Cached;
	int numErrors;
	using Scanner::Edition;
	bool columns_cached;
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
	Table(const char *TableName, const char *IdxName, int Create, Database *db, int Cached);
	~Table();
	bool Open();
	void Close();

	// Columns
	ColumnField *NewColumn(unsigned char Id, const char *name, unsigned char type, bool indexUniques);

	void DeleteColumn(ColumnField *field); // todo
	void DeleteColumnByName(const char *name); // todo
	void DeleteColumnById(unsigned char Id); // todo
	void PostColumns(void);
	NDE_API Record *GetColumns(void);
	ColumnField *GetColumnByName(const char *FieldName);
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
	bool LocateByIdEx(int Id, int From, Field *field, int comp_mode);

	// Indexes
	void AddIndexByName(const char *FieldName, const char *KeyName);
	void AddIndexById(unsigned char Id, const char *KeyName);

	void WalkIndices(IndexWalker callback, void *context);

	IndexField *GetIndexByName(const char *name);
	IndexField *GetIndexById(unsigned char Id);
	using Scanner::SetWorkingIndexByName;
	using Scanner::SetWorkingIndexById;
	bool CheckIndexing(void);
	void DropIndexByName(const char *desc);
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
	void SetGlobalLocateUpToDate(bool is);

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