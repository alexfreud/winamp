/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

Table Class
Apple Mac OS X implementation
--------------------------------------------------------------------------- */
#include "../nde.h"
#include <stdio.h>
#include <string.h>
#include "../CRC.H"

const char *tSign="NDETABLE";

//---------------------------------------------------------------------------
Table::Table(const char *TableName, const char *Idx, BOOL Create, Database *_db, BOOL _Cached)
: columns_cached(false), use_row_cache(false), Scanner(this)
{
	Handle = 0; 
	memset(column_ids, FIELD_UNKNOWN, 255);
	Cached = _Cached;
	db = _db;
	AutoCreate = Create;
	Name = strdup(TableName);
	IdxName = strdup(Idx);
	Init();
}

//---------------------------------------------------------------------------
void Table::Init()
{
	numErrors = 0;
	Scanners = new LinkedList();
	// benski> cut: Handle=NULL;
	IdxHandle=NULL;
	FieldsRecord=NULL;
	IndexList=NULL;	
	GLocateUpToDate = FALSE;
}

//---------------------------------------------------------------------------
Table::~Table()
{
	Reset();
	if (Handle) // Reset doesn't completely destroy Handle
			Vfdestroy(Handle);
	Handle = 0;
	
	free(Name);
	free(IdxName);
}

//---------------------------------------------------------------------------
void Table::Reset() 
{
	if (IndexList) IndexList->Release();
	IndexList=0;
	if (FieldsRecord) FieldsRecord->Release();
	FieldsRecord=0;
	delete Scanners;
	Scanners=0;
	if (Handle)
		Vfclose(Handle); // close (but don't destroy) to keep mutex open.
	if (IdxHandle)
		Vfdestroy(IdxHandle);
	IdxHandle = 0;

	for (RowCache::iterator itr=rowCache.begin();itr!=rowCache.end();itr++)
	{
		if (itr->second)
			itr->second->Release();
	}
	rowCache.clear();

	memset(column_ids, FIELD_UNKNOWN, 255);
	columns_cached=false;
}


struct IndexNewWalkerContext
{
	IndexNewWalkerContext(Table *_table)
	{
		N = -1;
		table = _table;
	}
	int N;
	Table *table;
};

bool Table::IndexNewWalker(IndexRecord *record, Field *entry, void *context_in)
{
	IndexNewWalkerContext *context = (IndexNewWalkerContext *)context_in;
	IndexField *p = (IndexField *)entry;
	p->index = new Index(context->table->IdxHandle, p->ID, context->N++, p->Type, FALSE, 0, context->table);
	return true;
}

//---------------------------------------------------------------------------
BOOL Table::Open(void)
{
	BOOL Valid;
	int justcreated = 0;

	if (!Handle)
		Handle = Vfnew(Name, "r+b", Cached);
	if (!Handle) return FALSE;
	if (!Vflock(Handle))
	{
		Vfdestroy(Handle);
		Handle = 0;
		return FALSE;
	}

	Handle = Vfopen(Handle, Name, "r+b", Cached);
	IdxHandle = Vfopen(0, IdxName, "r+b", TRUE);
	Valid = (Handle && IdxHandle);

	// unlock
	if (Valid || !AutoCreate)
	{
		//if (Handle)
			//Vfunlock(Handle);
	}
	else
	{
		if (Handle)
		{
			Vfclose(Handle);
			if (IdxHandle)
				Vfdestroy(IdxHandle);
			IdxHandle = 0;
		}
		else
		{
			if (IdxHandle)
				Vfdestroy(IdxHandle);
			IdxHandle = 0;
			Handle = Vfnew(Name, "w+b", Cached);
			if (!Vflock(Handle))
			{
				Vfdestroy(Handle);
				return FALSE;
			}
		}

		Handle = Vfopen(Handle, Name, "w+b", Cached);
		IdxHandle = Vfopen(0, IdxName, "w+b", TRUE);
		Valid = (Handle && IdxHandle);

		if (Valid)
		{
			Vfwrite(__TABLE_SIGNATURE__, strlen(__TABLE_SIGNATURE__), Handle);
			Vfwrite(__INDEX_SIGNATURE__, strlen(__TABLE_SIGNATURE__), IdxHandle);
			// TODO bensk> change if NUM_SPECIAL_RECORDS ever increases
			int v=NUM_SPECIAL_RECORDS;//strlen(__TABLE_SIGNATURE__);
			Vfwrite(&v, sizeof(v), IdxHandle);
			//    v = 0; fwrite(&v, sizeof(v), 1, IdxHandle);
			v = -1; Vfwrite(&v, sizeof(v), IdxHandle); // write ID
			v = 0; 
			for (int i=0;i<NUM_SPECIAL_RECORDS;i++)
			{
				Vfwrite(&v, sizeof(v), IdxHandle);
				Vfwrite(&v, sizeof(v), IdxHandle);
			}
			Sync();
			justcreated = 1;
		}
	}


	if (!Valid)
	{
		if (Handle) Vfdestroy(Handle);
		if (IdxHandle) Vfdestroy(IdxHandle);
		Handle = NULL;
		IdxHandle = NULL;
	}
	else
	{
		int Ptr;

		char test1[9]={0,};
		char test2[9]={0,};

		Vfseek(Handle, 0, SEEK_SET);
		Vfread(test1, strlen(__TABLE_SIGNATURE__), Handle);
		Vfseek(IdxHandle, 0, SEEK_SET);
		Vfread(test2, strlen(__INDEX_SIGNATURE__), IdxHandle);
		test1[8]=0;
		test2[8]=0;
		if (strcmp(test1, __TABLE_SIGNATURE__) || strcmp(test2, __INDEX_SIGNATURE__))
		{
			if (Handle) Vfdestroy(Handle);
			Handle = 0;
			if (IdxHandle) Vfdestroy(IdxHandle);
			IdxHandle = 0;
			return FALSE;
		}

		// Load default index
		IndexField *field;

		field = new IndexField(PRIMARY_INDEX, -1, -1, CFSTR("None"));
		field->index = new Index(IdxHandle, PRIMARY_INDEX, -1, -1, FALSE, 0, this);

		// Get indexes
		Ptr = field->index->Get(INDEX_RECORD_NUM);
		IndexList = new IndexRecord(Ptr, INDEX_RECORD_NUM, Handle, this);
		if (!IndexList)
		{
			delete field;
			if (Handle) Vfdestroy(Handle);
			Handle = 0;
			if (IdxHandle) Vfdestroy(IdxHandle);
			IdxHandle = 0;
			return FALSE;
		}

		// Init them
		IndexNewWalkerContext newContext(this);
		IndexList->WalkFields(IndexNewWalker, &newContext);

		// Add default in case its not there (if it is it won't be added by addfield)
		IndexList->AddField(field);

		// Get the default index (whether loaded or preloaded)
		Scanner::index = ((IndexField*)IndexList->GetField(PRIMARY_INDEX))->index;

		// If it's different from preloaded, delete preloaded
		if (field->index != Scanner::index)
		{
			delete field;
			field=0;
		}

		// Set up colaboration
		IndexList->BuildCollaboration();

		// Get columns
		Ptr = Scanner::index->Get(FIELDS_RECORD_NUM);
		FieldsRecord = new Record(Ptr, FIELDS_RECORD_NUM, Handle, this);
		if (!FieldsRecord)
		{
			IndexList->Release();
			IndexList=0;
			if (Handle) Vfdestroy(Handle);
			Handle = 0;
			if (IdxHandle) Vfdestroy(IdxHandle);
			IdxHandle = 0;
			return FALSE;
		}

		// update the column cache
		FieldsRecord->WalkFields(BuildColumnCache, this);
		columns_cached=true;
	}

#if 0 // TODO
	if (Valid && !justcreated)
	{
		if (IndexList->NeedFix())
			Compact();
	}
#endif
	if (Valid) First();
	if (Handle)
		Vfunlock(Handle);
	return Valid;
}

//---------------------------------------------------------------------------
void Table::Close(void)
{
	int v=0;

	if (Handle && IndexList && Vflock(Handle, 0))
	{
		IndexList->WalkFields(IndexWriteWalker, 0);
	}

	delete Scanners;
	Scanners = NULL;

	Vsync(Handle);
	if (IdxHandle)
	{
		Vfdestroy(IdxHandle);
		IdxHandle = NULL;
		v |= 2;
	}
	if (Handle)
	{
		Vfdestroy(Handle);
		Handle = NULL;
		v |= 1;
	}

	if (v != 3)
		return;

}

bool Table::IndexWriteWalker(IndexRecord *record, Field *entry, void *context)
{
	IndexField *field = (IndexField *)entry;
	field->index->WriteIndex();
	return true;
}

//---------------------------------------------------------------------------
void Table::Sync(void)
{
	if (!Vflock(Handle))
		return;

	if (IndexList)
		IndexList->WalkFields(IndexWriteWalker, 0);

	int err=0;
	if (!err && Handle) err|=Vsync(Handle);
	if (!err && IdxHandle) err|=Vsync(IdxHandle);
	
	Vfunlock(Handle);
}

//---------------------------------------------------------------------------
ColumnField *Table::NewColumn(unsigned char FieldID, CFStringRef FieldName, unsigned char FieldType, BOOL indexUnique)
{
	columns_cached=false; // if they start writing new columns, kill the columns cache until they PostColumns()
	ColumnField *f = GetColumnById(FieldID);
	if (f) {
		int t = f->GetDataType();
		if (t != FieldType) {
			if (CompatibleFields(t, FieldType))
			{
				f->SetDataType(FieldType);
				goto aok;
			}
		}
		return NULL;
	}
aok:
	if (GetColumnByName(FieldName))
		return NULL;
	ColumnField *field = new ColumnField(FieldID, FieldName, FieldType, this);
	column_ids[FieldID]=FieldType;
	FieldsRecord->AddField(field);
	return field;
}

void Table::SetFieldSearchableById(unsigned char field_id, bool searchable)
{
	ColumnField *column = GetColumnById(field_id);
	if (column)
		column->SetSearchable(searchable);
	if (searchable)
	{
		search_fields.insert(field_id);
	}
	else
	{
		search_fields.erase(field_id);
	}
}

//---------------------------------------------------------------------------
bool Table::BuildColumnCache(Record *record, Field *entry, void *context)
{
	Table *table = (Table *)context;
	ColumnField *column = (ColumnField *)entry;
	unsigned char field_id=column->GetFieldId();
	table->column_ids[field_id] = column->GetDataType();

	if (column->IsSearchableField())
	{
		table->search_fields.insert(field_id);
	}
	else
	{
		table->search_fields.erase(field_id);
	}
	return true;
}

//---------------------------------------------------------------------------
void Table::PostColumns(void)
{
	FieldsRecord->WriteFields(this, FIELDS_RECORD_NUM);
	memset(column_ids, FIELD_UNKNOWN, 255);
	FieldsRecord->WalkFields(BuildColumnCache, this);
	columns_cached=true;
}

unsigned char Table::GetColumnType(unsigned char Id)
{
	if (columns_cached)
	{
		return column_ids[Id];
	}
	else
	{
		return GetColumnById(Id)->GetDataType();
	}
}

//---------------------------------------------------------------------------
IndexField *Table::GetIndexByName(CFStringRef name)
{
	if (!IndexList)
		return NULL;
	return IndexList->GetIndexByName(name);
}

//---------------------------------------------------------------------------
IndexField *Table::GetIndexById(unsigned char Id)
{
	if (!IndexList)
		return NULL;
	return (IndexField *)IndexList->GetField(Id);
}

//---------------------------------------------------------------------------
void Table::AddIndexByName(CFStringRef name, CFStringRef desc)
{
	ColumnField *header = GetColumnByName(name);
	if (header)
	{
		unsigned char Idx = header->ID;
		AddIndexById(Idx, desc);
	}
}

//---------------------------------------------------------------------------
void Table::AddIndexById(unsigned char Id, CFStringRef desc)
{
	if (GetIndexById(Id)) return;
	ColumnField *col = GetColumnById(Id);
	if (!col)
		return;
	IndexField *newindex = new IndexField(Id, IndexList->GetColumnCount(), col->GetDataType(), desc);
	newindex->index = new Index(IdxHandle, Id, IndexList->GetColumnCount(), col->GetDataType(), TRUE, Scanner::index->NEntries, this);
	IndexList->AddField(newindex);

	IndexField *previous = (IndexField *)newindex->prev;
	previous->index->Colaborate(newindex);
	IndexField *primary_index = (IndexField *)IndexList->GetField(PRIMARY_INDEX);
	newindex->index->Colaborate(primary_index);

	previous->index->Propagate();

	IndexList->WriteFields(this);
}

//---------------------------------------------------------------------------
BOOL Table::CheckIndexing(void)
{
	if (IndexList->GetColumnCount() == 0) return TRUE;

	for (int i=0;i<Scanner::index->NEntries;i++)
	{		
		if (!IndexList->CheckIndexing(i))
			return FALSE;
	}
	return TRUE;
}

struct IndexWalkerThunkContext
{
	void *context;
	Table *_this;
	Table::IndexWalker callback;
};

bool Table::IndexWalkerThunk(IndexRecord *record, Field *entry, void *context_in)
{
	IndexWalkerThunkContext *context = (IndexWalkerThunkContext *)context_in;
	return context->callback(context->_this, (IndexField *)entry, context->context);
}

//---------------------------------------------------------------------------
void Table::WalkIndices(IndexWalker callback, void *context)
{
	if (IndexList && callback)
	{
		IndexWalkerThunkContext walkerContext = { context, this, callback };
		IndexList->WalkFields(IndexWalkerThunk, &walkerContext);
	}
}

//---------------------------------------------------------------------------
void Table::DropIndex(IndexField *Ptr)
{
	if (!Ptr || Ptr->Type != FIELD_INDEX) return;
	if (Scanner::index == Ptr->index)
	{
		Scanner::index = ((IndexField*)IndexList->GetField(PRIMARY_INDEX))->index;

		IndexList->BuildCollaboration();
	}
	IndexList->RemoveField(Ptr);
	if (Scanner::index->SecIndex == Ptr)
		Scanner::index->SecIndex = 0;
	IndexList->SetModified(TRUE);
	IndexList->WriteFields(this);
}

//---------------------------------------------------------------------------
void Table::DropIndexByName(CFStringRef desc)
{
	IndexField *indx = GetIndexByName(desc);
	if (CFStringCompare(desc, CFSTR("None"), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
		return;
	
	if (indx)
		DropIndex(indx);
}

//---------------------------------------------------------------------------
void Table::DropIndexById(unsigned char Id)
{
	if (!IndexList)
		return;
	if (Id == (unsigned char)PRIMARY_INDEX) return;
	IndexField *indx=(IndexField *)IndexList->GetField(Id);
	if (indx)
		DropIndex(indx);
}



//---------------------------------------------------------------------------
BOOL Table::LocateByIdEx(int Id, int From, Field *field, int comp_mode)
{
	return Scanner::LocateByIdEx(Id, From, field, NULL, comp_mode);
}

//---------------------------------------------------------------------------
Record *Table::GetColumns(void)
{
	if (!FieldsRecord)
		return NULL;
	return FieldsRecord;
}

//---------------------------------------------------------------------------
Scanner *Table::NewScanner()
{
	Scanner *s = new Scanner(this);
	/*if (Scanners->GetNElements() > 0)*/
	s->index = Scanner::index;
	Scanners->AddEntry(s, TRUE);
	return s;
}

//---------------------------------------------------------------------------
Scanner *Table::GetDefaultScanner(void)
{
	return this;
}

//---------------------------------------------------------------------------
void Table::DeleteScanner(Scanner *scan)
{
	if (!scan) return;
		Scanners->RemoveEntry(scan);
}

//---------------------------------------------------------------------------
void Table::IndexModified(void)
{
	Scanner *s = (Scanner *)Scanners->GetHead();
	while (s)
	{
		s->IndexModified();
		s = (Scanner *)s->GetNext();
	}
}

//---------------------------------------------------------------------------
void Table::SetGlobalLocateUpToDate(BOOL is) {
	GLocateUpToDate = is;
}

ColumnField *Table::GetColumnById(unsigned char Idx)
{
	if (!FieldsRecord)
		return NULL;
	return (ColumnField *)FieldsRecord->GetField(Idx);
}

ColumnField *Table::GetColumnByName(CFStringRef FieldName)
{
	return FieldsRecord->GetColumnByName(FieldName);
}

void Table::RowCache_Delete(int position)
{
	if (use_row_cache)
	{
		RowCache::iterator found = rowCache.find(position);
		if (found != rowCache.end())
		{
			if (found->second)
			found->second->Release();
			rowCache.erase(found);
		}
	}
}

void Table::RowCache_Remove(int position)
{
	if (use_row_cache)
	{
		
		Record *&row = rowCache[position];
		if (row)
		{
			row->Release();
		}

		row = 0;
		}
}

void Table::RowCache_Add(Record *record, int position)
{
	if (use_row_cache)
	{
		record->Retain();

		Record *&row = rowCache[position];
		if (row)
		{
			row->Release();
		}

		row = record;
	}
}

Record *Table::RowCache_Get(int position)
{
	if (!use_row_cache)
		return 0;
	Record *row = rowCache[position];
	if (row)
		row->Retain();
	return row;
}

void Table::EnableRowCache()
{
	use_row_cache=true;
}
