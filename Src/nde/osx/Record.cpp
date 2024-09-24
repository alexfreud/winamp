/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

Record Class

--------------------------------------------------------------------------- */

//#include "record.h"
#include "nde.h"
#include <stdio.h>

void RecordBase::Retain()
{
	ref_count++;
}

void RecordBase::Release()
{
	if (--ref_count == 0)
		delete this;
}

RecordBase::RecordBase()
{
	ref_count = 1;
	InsertionPoint = 0;
}

//---------------------------------------------------------------------------
Record::Record(int RecordPos,  int insertionPoint, VFILE *TableHandle, Table *ParentTable)
{
	InsertionPoint = insertionPoint;
	Record *columns = ParentTable->GetColumns();
	int max=columns ? columns->Fields.size() : 128;
	int n=0;
	if (RecordPos != 0)
	{
		uint32_t ThisPos = RecordPos;
		while (ThisPos)
		{
			if (n >= max)
				break;
			Vfseek(TableHandle, ThisPos, SEEK_SET);
			Field Entry (ThisPos);
			Field *TypedEntry = Entry.ReadField(ParentTable, ThisPos, &ThisPos);

			if (!TypedEntry) break; // some db error?

			AddField(TypedEntry);
			n++;
		}
	}
}

//---------------------------------------------------------------------------
RecordBase::~RecordBase()
{
	Fields.deleteAll();
}


#ifdef _WIN32
ColumnField *Record::GetColumnByName(const wchar_t *name)
{
	for (FieldList::iterator itr=Fields.begin();itr!=Fields.end();itr++)
	{
		ColumnField *p = (ColumnField *)*itr;
		if (!_wcsicmp(p->GetFieldName(), name))
			return p;
	}
	return NULL;
}
#elif defined(__APPLE__)
ColumnField *Record::GetColumnByName(CFStringRef name)
{
	for (FieldList::iterator itr=Fields.begin();itr!=Fields.end();itr++)
	{
		ColumnField *p = (ColumnField *)*itr;
		if (CFStringCompare(p->GetFieldName(), name, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
			return p;
	}
	return NULL;
}
#endif


//---------------------------------------------------------------------------
int Record::WriteFields(Table *ParentTable, int RecordIndex)
{
	Field *previous = 0;
	for (FieldList::iterator itr=Fields.begin();itr!=Fields.end();itr++)
	{
		Field *p = *itr;
		p->WriteField(ParentTable, previous, (Field *)p->next);
		previous = p;
	}
	return WriteIndex(ParentTable, RecordIndex);
}

//---------------------------------------------------------------------------
int Record::WriteIndex(Table *ParentTable, int RecordIndex)
{
	int P=0;
	if (RecordIndex == NEW_RECORD)
		RecordIndex = ParentTable->index->Insert(InsertionPoint);
	if (!Fields.empty())
	{
		Field *f = *Fields.begin();
		P=f->GetFieldPos();
	}
	return ParentTable->index->Update(RecordIndex, P, this, FALSE);
}

//---------------------------------------------------------------------------
void Record::Delete(Table *ParentTable, int RecordIndex)
{
	ParentTable->index->Delete(RecordIndex, ParentTable->index->Get(RecordIndex), this);
}

void Record::WalkFields(FieldsWalker callback, void *context)
{
	if (callback)
	{
		for (FieldList::iterator itr=Fields.begin();itr!=Fields.end();itr++)
		{
			if (!callback(this, *itr, context))
				break;
		}
	}
}



