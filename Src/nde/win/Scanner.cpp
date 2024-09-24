/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

Scanner Class

--------------------------------------------------------------------------- */

#include "../nde.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "BinaryField.h"
#include "Binary32Field.h"
#include <assert.h>
//---------------------------------------------------------------------------
Scanner::Scanner(Table *parentTable)
{
	disable_date_resolution=0;
	index = NULL;
	pTable = parentTable;
	Edition=FALSE;

	lastLocateIndex = NULL;
	lastLocateId = -1;
	lastLocateIdx = -1;
	lastLocateFrom = -128;
	lastLocateFieldClone = NULL;
	CurrentRecord=NULL;
	CurrentRecordIdx=0;
	iModified = FALSE;
	FiltersOK = FALSE;
	
	search_any = FALSE;
	ResultPtr = 0;

	/*inMatchJoins = 0;
	lastJoinCache = 0;
	invalidJoinCache = 1;*/
	last_query = NULL;
	last_query_failed = FALSE;
	token = NULL;
	in_query_parser = 0;
}

//---------------------------------------------------------------------------
Scanner::~Scanner()
{
	for ( StringField *l_string_field : search_strings )
		delete l_string_field;

	search_strings.clear();

	if ( CurrentRecord )
		CurrentRecord->Release();

	if ( lastLocateFieldClone )
	{
		delete lastLocateFieldClone;
	}

	Query_CleanUp();

	if ( token )
		ndestring_release( token );

	ndestring_release( last_query );
}

//---------------------------------------------------------------------------
Record *Scanner::GetRecord(int Idx)
{
	int Ptr;
	Ptr = index->Get(Idx);

	Record *r = pTable->RowCache_Get(Ptr);
	if (r)
	{
		return r;
	}

	r = new Record(Ptr, Idx, pTable->Handle, pTable);
	pTable->RowCache_Add(r, Ptr);
	return r;
}

//---------------------------------------------------------------------------
void Scanner::GetCurrentRecord(void)
{
	if (CurrentRecord) CurrentRecord->Release();
	CurrentRecord = NULL;

	//invalidJoinCache = 1;
	if (Eof() || Bof()) return;
	CurrentRecord = GetRecord(CurrentRecordIdx);

}

//---------------------------------------------------------------------------
void Scanner::GetRecordById(int Id, BOOL checkFilters)
{
	CurrentRecordIdx=max(min(index->NEntries, Id+NUM_SPECIAL_RECORDS), 0);
	GetCurrentRecord();
	if (!checkFilters || MatchFilters())
		return;
	Next();
}

//---------------------------------------------------------------------------
void Scanner::First(int *killswitch)
{
	if (last_query_failed) return;
	GetRecordById(0);
	if (!MatchFilters() && !Eof())
		Next(killswitch);
}

//---------------------------------------------------------------------------
int Scanner::Next(int *killswitch)
{
	if (last_query_failed) return 0;

	while (!Eof() && !Bof())
	{
		CurrentRecordIdx++;
		GetCurrentRecord();
		if (MatchFilters())
			break;
		else
		{
			if ((killswitch && *killswitch))
				return 0;
		}
	}
	return 1;
}

//---------------------------------------------------------------------------
int Scanner::Previous(int *killswitch)
{
	if (last_query_failed) return 0;

	while (CurrentRecordIdx >= NUM_SPECIAL_RECORDS)
	{
		CurrentRecordIdx--;
		GetCurrentRecord();
		if (MatchFilters())
			break;
		else
		{
			if ((killswitch && *killswitch))
				return 0;
		}
	}
	return 1;
}

//---------------------------------------------------------------------------
void Scanner::Last(int *killswitch)
{
	if (last_query_failed) return;
	GetRecordById(index->NEntries-NUM_SPECIAL_RECORDS-1); // -3 here because 1)GetRecordById is public, so -NUM_SPECIAL_RECORDS, and 2)last entry is nentries-1, so -1
	if (!MatchFilters() && !Bof())
		Previous(killswitch);
	if (CurrentRecordIdx < NUM_SPECIAL_RECORDS)
	{
		CurrentRecordIdx = index->NEntries;
		GetCurrentRecord(); // will only delete current record if it exists
	}
}

//---------------------------------------------------------------------------
BOOL Scanner::Eof(void)
{
	if (last_query_failed) return TRUE;
	return CurrentRecordIdx >= index->NEntries;
}

//---------------------------------------------------------------------------
BOOL Scanner::Bof(void)
{
	if (last_query_failed) return TRUE;
	return CurrentRecordIdx < NUM_SPECIAL_RECORDS;
}

//---------------------------------------------------------------------------
Field *Scanner::GetFieldByName(const wchar_t *FieldName)
{
	ColumnField *header = pTable->GetColumnByName(FieldName);
	if (header)
	{
		unsigned char Idx = header->ID;
		return GetFieldById(Idx);
	}
	return NULL;
}

//---------------------------------------------------------------------------
Field *Scanner::GetFieldById(unsigned char Id)
{
	if (!CurrentRecord)
		return NULL;
	Field *field = CurrentRecord->GetField(Id);
	if (field)
	{
		int column_type = pTable->GetColumnType(Id);
		if (!CompatibleFields(field->GetType(), column_type))
		{
			return NULL;
		}
	}
	return field;
}

//---------------------------------------------------------------------------
Field *Scanner::NewFieldByName(const wchar_t *fieldName, unsigned char Perm)
{
	ColumnField *header = pTable->GetColumnByName(fieldName);
	if (header)
	{
		unsigned char Id = header->ID;
		Field *field = NewFieldById(Id, Perm);
		return field;
	}
	return NULL;
}

//---------------------------------------------------------------------------
Field *Scanner::NewFieldById(unsigned char Id, unsigned char Perm)
{
	ColumnField *field = GetColumnById(Id);
	if (!field)
		return NULL;
	Field *O=GetFieldById(Id);
	if (O) return O;
	switch (field->GetDataType())
	{
		case FIELD_STRING:
			O = new StringField();
			break;
		case FIELD_INTEGER:
			O = new IntegerField();
			break;
		case FIELD_INT64:
			O = new Int64Field();
			break;
		case FIELD_INT128:
			O = new Int128Field();
			break;
		case FIELD_DATETIME:
			if (disable_date_resolution)
				O= new StringField();
			else
				O = new DateTimeField();
			break;
		case FIELD_LENGTH:
			O = new LengthField();
			break;
		case FIELD_FILENAME:
			O = new FilenameField();
			break;
		case FIELD_BINARY:
			O = new BinaryField();
			break;
		case FIELD_BINARY32:
			O = new Binary32Field();
			break;
		default:
			MessageBox(NULL, L"unknown field type for id", L"debug", 0);
			O = new Field();
			break;
	}
	O->Type = field->GetDataType();
	O->ID = Id;
	CurrentRecord->AddField(O);
	return O;
}

Field *Scanner::NewFieldByType(unsigned char Type, unsigned char Id, unsigned char Perm)
{
	Field *O = NULL;
	switch (Type)
	{
		case FIELD_STRING:
			O = new StringField();
			break;
		case FIELD_INTEGER:
			O = new IntegerField();
			break;
		case FIELD_INT64:
			O = new Int64Field();
			break;
		case FIELD_INT128:
			O = new Int128Field();
			break;
		case FIELD_DATETIME:
			if (disable_date_resolution)
				O= new StringField();
			else
				O = new DateTimeField();
			break;
		case FIELD_LENGTH:
			O = new LengthField();
			break;
		case FIELD_FILENAME:
			O = new FilenameField();
			break;
		case FIELD_BINARY:
			O = new BinaryField();
			break;
		case FIELD_BINARY32:
			O = new Binary32Field();
			break;
		default:
			MessageBox(NULL, L"unknown field type for id", L"debug", 0);
			O = new Field();
			break;
	}
	O->Type = Type;
	O->ID = Id;
	CurrentRecord->AddField(O);
	return O;
}

//---------------------------------------------------------------------------
void Scanner::Post(void)
{
	if (!CurrentRecord) return;
	/*if (CurrentRecord->RecordIndex == NEW_RECORD)
	NEntries++;*/
	if (pTable->use_row_cache && CurrentRecordIdx != NEW_RECORD)
	{
		int Ptr = index->Get(CurrentRecordIdx);
		pTable->RowCache_Remove(Ptr);
	}
	CurrentRecordIdx = CurrentRecord->WriteFields(pTable, CurrentRecordIdx);
	Edition=FALSE;
	if (pTable->use_row_cache)
	{
		int Ptr = index->Get(CurrentRecordIdx);
		pTable->RowCache_Add(CurrentRecord, Ptr);
	}
}

//---------------------------------------------------------------------------
void Scanner::New(void)
{
	if (CurrentRecord) CurrentRecord->Release();
	CurrentRecord = NULL;

	CurrentRecord = new Record(0, index->NEntries, pTable->Handle, pTable);
	CurrentRecordIdx = NEW_RECORD;
	Edition = TRUE;
}

//---------------------------------------------------------------------------
void Scanner::Insert(void)
{
	if (CurrentRecord) CurrentRecord->Release();
	CurrentRecord = new Record(0, CurrentRecordIdx, pTable->Handle, pTable);
	CurrentRecordIdx = NEW_RECORD;
	Edition=TRUE;
}

//---------------------------------------------------------------------------
void Scanner::Delete(void)
{
	if (CurrentRecord)
	{
		if (pTable->use_row_cache && CurrentRecordIdx != NEW_RECORD)
		{
			int Ptr = index->Get(CurrentRecordIdx);
			pTable->RowCache_Delete(Ptr);
		}
		CurrentRecord->Delete(pTable, CurrentRecordIdx);
	}
	if (Eof())
		Previous();
	GetRecordById(CurrentRecordIdx-NUM_SPECIAL_RECORDS);
}

//---------------------------------------------------------------------------
int Scanner::GetRecordId(void)
{
	return CurrentRecordIdx != NEW_RECORD ? CurrentRecordIdx-NUM_SPECIAL_RECORDS : CurrentRecordIdx;
}

//---------------------------------------------------------------------------
void Scanner::Edit(void)
{
	if (Edition) return;
	if (!CurrentRecord)
		return;
	/*Field *f = (Field *)CurrentRecord->Fields->GetHead();
	while (f)
	{
	f->SubtableRecord = INVALID_RECORD;
	f = (Field *)f->GetNext();
	}*/

	if (CurrentRecord->InCache()) // if it's in the cache
	{
		// benski> make copy of CurrentRecord, outside the cache
		int Ptr = index->Get(CurrentRecordIdx);
		pTable->RowCache_Remove(Ptr);
		Record *r = new Record(Ptr, CurrentRecordIdx, pTable->Handle, pTable);
		CurrentRecord->Release();
		CurrentRecord = r;
	}

	Edition = TRUE;
}


//---------------------------------------------------------------------------
void Scanner::Cancel(void)
{
	Edition = FALSE;
	GetCurrentRecord();
}

//---------------------------------------------------------------------------
BOOL Scanner::LocateByName(const wchar_t *col, int From, Field *field, int *nskip)
{
	ColumnField *f = pTable->GetColumnByName(col);
	if (!f)
		return NULL;
	return LocateById(f->GetFieldId(), From, field, nskip);
}

//---------------------------------------------------------------------------
void Scanner::CacheLastLocate(int Id, int From, Field *field, Index *i, int j)
{
	lastLocateId = Id;
	lastLocateFrom = From;
	if (lastLocateFieldClone)
	{
		delete lastLocateFieldClone;
		lastLocateFieldClone = NULL;
	}
	lastLocateFieldClone = field->Clone(pTable);
	lastLocateIndex = i;
	i->locateUpToDate = TRUE;
	pTable->SetGlobalLocateUpToDate(TRUE);
	lastLocateIdx = j;
}


//---------------------------------------------------------------------------
BOOL Scanner::LocateById(int Id, int From, Field *field, int *nskip)
{
	return LocateByIdEx(Id, From, field, nskip, COMPARE_MODE_EXACT);
}
//---------------------------------------------------------------------------
BOOL Scanner::LocateByIdEx(int Id, int From, Field *field, int *nskip, int comp_mode)
{
	IndexField *i = pTable->GetIndexById(Id);
	Field *compField=NULL;
	int j=0;
	int n=0;
	Field *cfV=NULL;

	if (index->NEntries == NUM_SPECIAL_RECORDS)
		return FALSE;

	int success = FALSE;

	if (nskip) *nskip=0;
	// I know this is stupid but.... May be do something later
	switch (comp_mode)
	{
	case COMPARE_MODE_CONTAINS:
		while (1)
		{
			success = FALSE;
			if (!i)
			{
				// No index for this column. Using slow locate, enumerates the database, still faster than what the user
				// can do since we have access to QuickFindField which only read field headers
				// in order to locate the field we have to compare. user could only read the entire record.
				if (From == FIRST_RECORD)
					From = NUM_SPECIAL_RECORDS;
				else
					From+=(NUM_SPECIAL_RECORDS+1);
				if (From == lastLocateFrom && Id == lastLocateId && field->Contains(lastLocateFieldClone)==0 && index == lastLocateIndex && (index->locateUpToDate || pTable->GLocateUpToDate))
				{
					if (CurrentRecordIdx != lastLocateIdx) GetRecordById(lastLocateIdx-NUM_SPECIAL_RECORDS, FALSE);
					success = TRUE;
					goto nextiter_1;
				}
				for (j=From;j<index->NEntries;j++)
				{
					compField = index->QuickFindField(Id, index->Get(j));
					cfV = /*(compField && compField->Type == FIELD_PRIVATE) ? ((PrivateField *)compField)->myField :*/ compField;
					if (!field->Contains(cfV))
					{
						if (compField)
						{
							delete compField;
						}
						if (CurrentRecordIdx != j) GetRecordById(j-NUM_SPECIAL_RECORDS, FALSE);
						CacheLastLocate(Id, From, field, index, j);
						success = TRUE;
						goto nextiter_1;
					}
					delete compField;
				}
				success = FALSE;
				goto nextiter_1;
			}
			else
			{
				// Index available. Using fast locate. nfetched=log2(nrecords) for first locate, 1 more fetch per locate on same criteria
				int p = 0;
				if (From == FIRST_RECORD) From = NUM_SPECIAL_RECORDS;
				else From = index->TranslateIndex(From+NUM_SPECIAL_RECORDS, i->index)+1;
				if (From == lastLocateFrom && Id == lastLocateId && field->Contains(lastLocateFieldClone)==0 && index == lastLocateIndex && (i->index->locateUpToDate || pTable->GLocateUpToDate))
				{
					if (CurrentRecordIdx != lastLocateIdx) GetRecordById(lastLocateIdx-NUM_SPECIAL_RECORDS, FALSE);
					success = TRUE;
					goto nextiter_1;
				}
				if (From >= index->NEntries)
				{
					return FALSE;
				}
				compField = i->index->QuickFindField(Id, i->index->Get(From));
				cfV = /*(compField && compField->Type == FIELD_PRIVATE) ? ((PrivateField *)compField)->myField :*/ compField;
				if (field->Contains(cfV) == 0)
				{
					delete compField;
					compField = NULL;
					n = i->index->TranslateIndex(From, index);
					if (CurrentRecordIdx != n) GetRecordById(n-NUM_SPECIAL_RECORDS, FALSE);
					CacheLastLocate(Id, From, field, i->index, n);
					success = TRUE;
					goto nextiter_1;
				}
				delete compField;
				compField = NULL;
				p = i->index->QuickFindEx(Id, field, From, comp_mode);
				if (p != FIELD_NOT_FOUND)
				{
					n = (index->GetId() == Id) ? p : i->index->TranslateIndex(p, index);
					if (CurrentRecordIdx != n) GetRecordById(n-NUM_SPECIAL_RECORDS, FALSE);
					CacheLastLocate(Id, From, field, i->index, n);
					success = TRUE;
					goto nextiter_1;
				}
			}
nextiter_1: // eek
			if (success)
			{
				if (!MatchFilters() && !Eof())
				{
					From = GetRecordId();
					if (nskip) (*nskip)++;
				}
				else break;
			}
			else break;
		}

		break;
	case COMPARE_MODE_EXACT:
		while (1)
		{
			success = FALSE;
			if (!i)
			{
				// No index for this column. Using slow locate, enumerates the database, still faster than what the user
				// can do since we have access to QuickFindField which only read field headers
				// in order to locate the field we have to compare. user could only read the entire record.
				if (From == FIRST_RECORD)
					From = NUM_SPECIAL_RECORDS;
				else
					From+=(NUM_SPECIAL_RECORDS+1);
				if (From == lastLocateFrom && Id == lastLocateId && field->Compare(lastLocateFieldClone)==0 && index == lastLocateIndex && (index->locateUpToDate || pTable->GLocateUpToDate))
				{
					if (CurrentRecordIdx != lastLocateIdx) GetRecordById(lastLocateIdx-NUM_SPECIAL_RECORDS, FALSE);
					success = TRUE;
					goto nextiter_2;
				}
				for (j=From;j<index->NEntries;j++)
				{
					compField = index->QuickFindField(Id, index->Get(j));
					cfV = /*(compField && compField->Type == FIELD_PRIVATE) ? ((PrivateField *)compField)->myField :*/ compField;
					if (!field->Compare(cfV))
					{
						delete compField;
						compField = NULL;
						if (CurrentRecordIdx != j) GetRecordById(j-NUM_SPECIAL_RECORDS, FALSE);
						CacheLastLocate(Id, From, field, index, j);
						success = TRUE;
						goto nextiter_2;
					}
					delete compField;
					compField = NULL;
				}
				success = FALSE;
				goto nextiter_2;
			}
			else
			{
				// Index available. Using fast locate. nfetched=log2(nrecords) for first locate, 1 more fetch per locate on same criteria
				int p;
				if (From == FIRST_RECORD) From = NUM_SPECIAL_RECORDS;
				else From = index->TranslateIndex(From+NUM_SPECIAL_RECORDS, i->index)+1;
				if (From == lastLocateFrom && Id == lastLocateId && field->Compare(lastLocateFieldClone)==0 && index == lastLocateIndex && (i->index->locateUpToDate || pTable->GLocateUpToDate))
				{
					if (CurrentRecordIdx != lastLocateIdx) GetRecordById(lastLocateIdx-NUM_SPECIAL_RECORDS, FALSE);
					success = TRUE;
					goto nextiter_2;
				}
				if (From >= index->NEntries)
				{
					return FALSE;
				}
				compField = i->index->QuickFindField(Id, i->index->Get(From));
				cfV = /*(compField && compField->Type == FIELD_PRIVATE) ? ((PrivateField *)compField)->myField :*/ compField;
				if (field->Compare(cfV) == 0)
				{
					if (compField)
					{
						delete compField;
						compField = NULL;
					}
					n = i->index->TranslateIndex(From, index);
					if (CurrentRecordIdx != n) GetRecordById(n-NUM_SPECIAL_RECORDS, FALSE);
					CacheLastLocate(Id, From, field, i->index, n);
					success = TRUE;
					goto nextiter_2;
				}
				delete compField;
				p = i->index->QuickFindEx(Id, field, From, comp_mode);
				if (p != FIELD_NOT_FOUND)
				{
					n = (index->GetId() == Id) ? p : i->index->TranslateIndex(p, index);
					if (CurrentRecordIdx != n) GetRecordById(n-NUM_SPECIAL_RECORDS, FALSE);
					CacheLastLocate(Id, From, field, i->index, n);
					success = TRUE;
					goto nextiter_2;
				}
			}
nextiter_2: // eek
			if (success)
			{
				if (!MatchFilters() && !Eof())
				{
					From = GetRecordId();
					if (nskip) (*nskip)++;
				}
				else break;
			}
			else break;
		}
		break;
	case COMPARE_MODE_STARTS:
		while (1)
		{
			success = FALSE;
			if (!i)
			{
				// No index for this column. Using slow locate, enumerates the database, still faster than what the user
				// can do since we have access to QuickFindField which only read field headers
				// in order to locate the field we have to compare. user could only read the entire record.
				if (From == FIRST_RECORD)
					From = NUM_SPECIAL_RECORDS;
				else
					From+=(NUM_SPECIAL_RECORDS+1);
				if (From == lastLocateFrom && Id == lastLocateId && field->Starts(lastLocateFieldClone)==0 && index == lastLocateIndex && (index->locateUpToDate || pTable->GLocateUpToDate))
				{
					if (CurrentRecordIdx != lastLocateIdx) GetRecordById(lastLocateIdx-NUM_SPECIAL_RECORDS, FALSE);
					success = TRUE;
					goto nextiter_3;
				}
				for (j=From;j<index->NEntries;j++)
				{
					compField = index->QuickFindField(Id, index->Get(j));
					cfV = /*(compField && compField->Type == FIELD_PRIVATE) ? ((PrivateField *)compField)->myField :*/ compField;
					if (!field->Starts(cfV))
					{
						if (compField)
						{
							delete compField;
							compField = NULL;
						}
						if (CurrentRecordIdx != j) GetRecordById(j-NUM_SPECIAL_RECORDS, FALSE);
						CacheLastLocate(Id, From, field, index, j);
						success = TRUE;
						goto nextiter_3;
					}
					if (compField)
					{
						delete compField;
						compField = NULL;
					}
				}
				success = FALSE;
				goto nextiter_3;
			}
			else
			{
				// Index available. Using fast locate. nfetched=log2(nrecords) for first locate, 1 more fetch per locate on same criteria
				int p = 0;
				if (From == FIRST_RECORD) From = NUM_SPECIAL_RECORDS;
				else From = index->TranslateIndex(From+NUM_SPECIAL_RECORDS, i->index)+1;
				if (From == lastLocateFrom && Id == lastLocateId && field->Starts(lastLocateFieldClone)==0 && index == lastLocateIndex && (i->index->locateUpToDate || pTable->GLocateUpToDate))
				{
					if (CurrentRecordIdx != lastLocateIdx) GetRecordById(lastLocateIdx-NUM_SPECIAL_RECORDS, FALSE);
					success = TRUE;
					goto nextiter_3;
				}
				if (From >= index->NEntries)
				{
					return FALSE;
				}
				compField = i->index->QuickFindField(Id, i->index->Get(From));
				cfV = /*(compField && compField->Type == FIELD_PRIVATE) ? ((PrivateField *)compField)->myField :*/ compField;
				if (field->Starts(cfV) == 0)
				{
					delete compField;
					compField = NULL;
					n = i->index->TranslateIndex(From, index);
					if (CurrentRecordIdx != n) GetRecordById(n-NUM_SPECIAL_RECORDS, FALSE);
					CacheLastLocate(Id, From, field, i->index, n);
					success = TRUE;
					goto nextiter_3;
				}
				delete compField;
				compField = NULL;
				p = i->index->QuickFindEx(Id, field, From, comp_mode);
				if (p != FIELD_NOT_FOUND)
				{
					n = (index->GetId() == Id) ? p : i->index->TranslateIndex(p, index);
					if (CurrentRecordIdx != n) GetRecordById(n-NUM_SPECIAL_RECORDS, FALSE);
					CacheLastLocate(Id, From, field, i->index, n);
					success = TRUE;
					goto nextiter_3;
				}
			}
nextiter_3: // eek
			if (success)
			{
				if (!MatchFilters() && !Eof())
				{
					From = GetRecordId();
					if (nskip) (*nskip)++;
				}
				else break;
			}
			else break;
		}
		break;
	}

	return success;
}

//---------------------------------------------------------------------------
void Scanner::DeleteFieldByName(const wchar_t *name)
{
	ColumnField *header = pTable->GetColumnByName(name);
	if (header)
	{
		unsigned char Idx = header->ID;
		DeleteFieldById(Idx);
	}
	return;
}


//---------------------------------------------------------------------------
void Scanner::DeleteFieldById(unsigned char Id)
{
	Field *field = CurrentRecord->GetField(Id);
	if (!field) return;
	CurrentRecord->RemoveField(field);
}

//---------------------------------------------------------------------------
void Scanner::DeleteField(Field *field)
{
	if (!field) return;
	CurrentRecord->RemoveField(field);
}

static bool TotalSizeCalculator(Record *record, Field *f, void *context)
{
	size_t *totalSize = (size_t *)context;
	*totalSize += f->GetTotalSize();
	return true;
}

//---------------------------------------------------------------------------
float Scanner::FragmentationLevel(void)
{
	int oldP = GetRecordId();
	size_t totalSize=0;

	if (CurrentRecord)
	{
		if (CurrentRecord) CurrentRecord->Release();
		CurrentRecord = NULL;
		CurrentRecordIdx = 0;
	}

	for (int i=0;i<index->NEntries;i++)
	{
		Record *r = GetRecord(i);
		if (r)
		{
			r->WalkFields(TotalSizeCalculator, &totalSize);
			r->Release();
		}
	}
	GetRecordById(oldP);
	Vfseek(pTable->Handle, 0, SEEK_END);
	return (((float)(Vftell(pTable->Handle)-strlen(__TABLE_SIGNATURE__)) / (float)totalSize) - 1) * 100;
}

//---------------------------------------------------------------------------
int Scanner::GetRecordsCount(void)
{
	if (index)
		return index->NEntries-NUM_SPECIAL_RECORDS;
	else
		return 0;
}

//---------------------------------------------------------------------------
BOOL Scanner::SetWorkingIndexById(unsigned char Id)
{
	IndexField *indx = pTable->GetIndexById(Id);
	int v = CurrentRecordIdx;
	if (indx)
	{
		if (!Eof() && !Bof())
		{
			IndexField *f = index->SecIndex;
			v = index->GetCooperative(CurrentRecordIdx);
			while (f != indx)
			{
				v = f->index->GetCooperative(v);
				f = f->index->SecIndex;
			}
		}
		index = indx->index;
		CurrentRecordIdx = v;
		GetCurrentRecord();
	}
	return (indx != NULL);
}

//---------------------------------------------------------------------------
BOOL Scanner::SetWorkingIndexByName(const wchar_t *desc)
{
	IndexField *indx = pTable->GetIndexByName(desc);
	if (indx)
		return SetWorkingIndexById(indx->ID);
	else
		return SetWorkingIndexById(-1);
}

//---------------------------------------------------------------------------
void Scanner::IndexModified(void)
{
	iModified = TRUE;
}

//---------------------------------------------------------------------------
void Scanner::ClearDirtyBit(void)
{
	iModified = FALSE;
}

//---------------------------------------------------------------------------
Table *Scanner::GetTable(void)
{
	return pTable;
}

//---------------------------------------------------------------------------
ColumnField *Scanner::GetColumnByName(const wchar_t *FieldName)
{
	return pTable->GetColumnByName(FieldName);
}
//---------------------------------------------------------------------------
ColumnField *Scanner::GetColumnById(unsigned char Idx)
{
	return pTable->GetColumnById(Idx);
}

//---------------------------------------------------------------------------
int Scanner::AddFilterByName(const wchar_t *name, Field *Data, unsigned char Op)
{
	ColumnField *f = pTable->GetColumnByName(name);
	if (f)
		return AddFilterById(f->GetFieldId(), Data, Op);
	return ADDFILTER_FAILED;
}

//---------------------------------------------------------------------------
int Scanner::AddFilterById(unsigned char Id, Field *Data, unsigned char Op)
{
	ColumnField *f = pTable->GetColumnById(Id);
	if (f)
	{
		Filter *filter = new Filter(Data, f->GetFieldId(), Op);
		FilterList.AddEntry(filter, TRUE);
	}
	else
		return ADDFILTER_FAILED;

	if (in_query_parser) return 1;
	return CheckFilters();
}

//---------------------------------------------------------------------------
int Scanner::AddFilterOp(unsigned char Op)
{
	Filter *filter = new Filter(Op);
	FilterList.AddEntry(filter, TRUE);
	if (in_query_parser) return 1;
	return CheckFilters();
}

//---------------------------------------------------------------------------
Filter *Scanner::GetLastFilter(void)
{
	if (FilterList.GetNElements() == 0) return NULL;
	return (Filter *)FilterList.GetFoot();
}

//---------------------------------------------------------------------------
void Scanner::RemoveFilters(void)
{
	last_query_failed = FALSE;
	while (FilterList.GetNElements() > 0)
		FilterList.RemoveEntry(FilterList.GetHead());
	FiltersOK = FALSE;
}

//---------------------------------------------------------------------------
BOOL Scanner::CheckFilters(void)
{
	int f=0;
	FiltersOK = FALSE;
	if (FilterList.GetNElements() == 0) // Should never happen
		return FILTERS_INVALID;
	Filter *filter = (Filter *)FilterList.GetHead();
	while (filter)
	{
		if (f == 256) return FILTERS_INVALID;
		int op = filter->GetOp();
		if (filter->Data() || op == FILTER_ISEMPTY || op == FILTER_ISNOTEMPTY)
			f++;
		else
		{
			if (op != FILTER_NOT)
				f--;
		}
		if (f == 0) return FILTERS_INVALID;
		filter = (Filter *)filter->GetNext();
	}

	if (f == 1)
	{
		FiltersOK = TRUE;
		return FILTERS_COMPLETE;
	}
	return FILTERS_INCOMPLETE;
}

//---------------------------------------------------------------------------
static bool EmptyMeansTrue(int op)
{
	return op == FILTER_ISEMPTY
		|| op == FILTER_NOTEQUALS
		|| op == FILTER_NOTCONTAINS;
}

//---------------------------------------------------------------------------
bool Scanner::MatchFilter(Filter *filter)
{
	Field *field = GetFieldById(filter->GetId());
	int op = filter->GetOp();
	Field * f = filter->Data();
	/* old behaviour
	if (!field)
	return EmptyMeansTrue(op);
	else if (op == FILTER_ISEMPTY)
	return FALSE;
	else if (op == FILTER_ISNOTEMPTY)
	return TRUE;
	*/

	// new behaviour
	if (!field)
	{
		// if field is empty and we're doing an equals op, match if f is also empty
		if (op == FILTER_EQUALS && f) return f->ApplyFilter(f,FILTER_ISEMPTY);
		if (op == FILTER_NOTEQUALS && f) return f->ApplyFilter(f,FILTER_ISNOTEMPTY);
		return EmptyMeansTrue(op);
	}
	// no need to check for op == FILTER_ISEMPTY, the fields now handle that

	return field->ApplyFilter(f, op);
}

struct Results
{
	Results(){
		value = false;
		calculated = false;
		filter = 0;
	}

	void operator=(bool _val)
	{
		calculated=true;
		value=_val;
	}

	bool Calc(Scanner *scanner)
	{
		if (!calculated)
		{
#if 0 /* if we want to do field-to-field comparisons */
			Field *compare_column = filter->Data();
			if (compare_column && compare_column->Type == FIELD_COLUMN)
			{
				Field *field = scanner->GetFieldById(filter->GetId());
				Field *compare_field = scanner->GetFieldById(compare_column->ID);
				int op = filter->GetOp();
				value = field->ApplyFilter(compare_field, op);
				calculated=true;
				return value;
			}
#endif
			value = scanner->MatchFilter(filter);
			calculated=true;
		}
		return value;
	}

	void SetFilter(Filter *_filter)
	{
		calculated=false;
		filter=_filter;
	}
private:
	bool value;
	bool calculated;
	Filter *filter;
};

bool Scanner::MatchSearch(const SearchFields &fields, StringField *search_field)
{
	for (SearchFields::const_iterator itr = fields.begin(); itr != fields.end();itr++)
	{
		Field *f = GetFieldById(*itr);
		if (f && f->Contains(search_field))
		{
			return true;
		}
	}
	return false; // if none of the fields matched the search strings, then bail out
}

bool Scanner::MatchSearches()
{
	// no search means always match
	if (search_strings.empty())
		return true;

	Scanner *s = pTable->GetDefaultScanner(); // kind of a hack but gets around private member variables
	const SearchFields &fields = s->search_fields;
	if (search_any)
	{
		for ( StringField *l_string_field : search_strings )
		{
			if ( MatchSearch( fields, l_string_field ) == true )
				return true;
		}

		return false; // we'll only get here if no search strings matched
	}
	else
	{ // normal search (subsequent terms do further filtering
		for ( StringField *l_string_field : search_strings )
		{
			if ( MatchSearch( fields, l_string_field ) == false )
				return false;
		}

		return true; // we'll only get here if all search strings have a match
	}
}

//---------------------------------------------------------------------------
bool Scanner::MatchFilters(void)
{
	if (!FiltersOK || FilterList.GetNElements() == 0)
	{
		//  return MatchJoins();
		return true;
	}

	//if (!MatchSearches())
	//return false;

	ResultPtr = 0;

	Results resultTable[256];

	Filter *filter = (Filter *)FilterList.GetHead();
	while (filter)
	{
		int op = filter->GetOp();
		if (filter->Data() || op == FILTER_ISEMPTY || op == FILTER_ISNOTEMPTY)
			resultTable[ResultPtr++].SetFilter(filter);
		else
			switch (op)
			{
				case FILTER_AND:
					if (ResultPtr > 1)
						resultTable[ResultPtr-2] = resultTable[ResultPtr-2].Calc(this) && resultTable[ResultPtr-1].Calc(this);
					ResultPtr--;
					break;
				case FILTER_OR:
					if (ResultPtr > 1)
						resultTable[ResultPtr-2] = resultTable[ResultPtr-2].Calc(this) || resultTable[ResultPtr-1].Calc(this);
					ResultPtr--;
					break;
				case FILTER_NOT:
					if (ResultPtr > 0)
						resultTable[ResultPtr-1] = !resultTable[ResultPtr-1].Calc(this);
					break;
			}
		filter = (Filter *)filter->GetNext();
	}

	if (ResultPtr != 1) // Should never happen, case already discarded by CheckFilters
	{
		FiltersOK = FALSE;
		return TRUE;
	}

	if (!resultTable[0].Calc(this)) return 0;
	//  return MatchJoins();
	//return FALSE;
	return MatchSearches();
}

void Scanner::WalkFilters(FilterWalker callback, void *context)
{
	if (callback)
	{
		LinkedListEntry *entry = FilterList.GetHead();
		while (entry)
		{
			if (!callback(this, (Filter *)entry, context))
				break;
			entry = entry->Next;
		}
	}
}

void Scanner::Search(const wchar_t *search_string)
{
	// first, clear existing search terms
	for ( StringField *l_string_field : search_strings )
		delete l_string_field;

	search_strings.clear();

	if (search_string && *search_string == L'*' && search_string[1] == L' ')
	{
		search_any=true;
		search_string += 2;
	}
	else
		search_any=false;

	if (search_string)
	{
		while (search_string && *search_string)
		{
			while (search_string && (*search_string && (*search_string == ' ' || *search_string == '\t')))
				search_string++;

			const wchar_t *end=search_string;

			wchar_t c = *search_string;
			if (c == L'\"') // a quoted string
			{
				end++;
				search_string++;
				while (end && *end && *end != '\"')
					end++;

				if (search_string && *search_string) // make sure it's not just a quote by itself
				{
					if (*end == 0) // no terminating quotes
					{
						wchar_t *search_term = ndestring_wcsndup(search_string, end-search_string);
						search_strings.push_back(new StringField(search_term, STRING_IS_NDESTRING));
					}
					else if (end > (search_string+1)) // at least one character in the quotes
					{
						wchar_t *search_term = ndestring_wcsndup(search_string, end-search_string-1);
						search_strings.push_back(new StringField(search_term, STRING_IS_NDESTRING));
						end++;
					}
				}
				search_string=end;
			}
			else if (c)
			{
				while (end && *end && *end != ' ' && *end != '\t')
					end++;

				wchar_t *search_term = ndestring_wcsndup(search_string, end-search_string-1);
				search_strings.push_back(new StringField(search_term, STRING_IS_NDESTRING));
			}
		}
	}
}

void Scanner::WalkFields(Record::FieldsWalker callback, void *context)
{
	if (CurrentRecord)
		CurrentRecord->WalkFields(callback, context);
}