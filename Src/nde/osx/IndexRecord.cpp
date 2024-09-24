/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

IndexRecord Class

--------------------------------------------------------------------------- */

#include "nde.h"
#include <stdio.h>

IndexRecord::IndexRecord(int RecordPos, int insertionPoint, VFILE *TableHandle, Table *ParentTable)
{
	InsertionPoint = insertionPoint;
	if (RecordPos != 0)
	{
		int n=0;
		uint32_t ThisPos = RecordPos;
		while (ThisPos)
		{
			if (n >= 128)
				break;
			Vfseek(TableHandle, ThisPos, SEEK_SET);
			Field Entry (ThisPos);
			Field *TypedEntry = Entry.ReadField(ParentTable, ThisPos, &ThisPos);

			if (!TypedEntry) break; // some db error?

			AddField(TypedEntry);
			//			ThisPos = TypedEntry->GetNextFieldPos();
			n++;
		}
	}
}


void IndexRecord::BuildCollaboration()
{
	for (FieldList::iterator itr=Fields.begin();itr!=Fields.end();itr++)
	{
		IndexField *p = (IndexField *)*itr;
		if (p->next)
			p->index->Colaborate((IndexField *)p->next);
		else
			p->index->Colaborate((IndexField *)*Fields.begin());
	}
}

bool IndexRecord::NeedFix()
{
	for (FieldList::iterator itr=Fields.begin();itr!=Fields.end();itr++)
	{
		IndexField *p = (IndexField *)*itr;
		if (p->index->NeedFix())
			return true;
	}
	return false;
}

#ifdef _WIN32
IndexField *IndexRecord::GetIndexByName(const wchar_t *name)
{
	for (FieldList::iterator itr=Fields.begin();itr!=Fields.end();itr++)
	{
		IndexField *p = (IndexField *)*itr;
		if (!_wcsicmp(p->GetIndexName(), name))
			return p;
	}
	return NULL;
}
#else
IndexField *IndexRecord::GetIndexByName(CFStringRef name)
{
	for (FieldList::iterator itr=Fields.begin();itr!=Fields.end();itr++)
	{
		IndexField *p = (IndexField *)*itr;
		if (CFStringCompare(p->GetIndexName(), name, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
			return p;
	}
	return NULL;
}
#endif

bool IndexRecord::CheckIndexing(int v)
{
	int i = v;
	for (FieldList::iterator itr=Fields.begin();itr!=Fields.end();itr++)
	{
		IndexField *p = (IndexField *)*itr;
		v = p->index->GetCooperative(v);
	}
	return v == i;
}


void IndexRecord::SetModified(BOOL modifiedVal)
{
	for (FieldList::iterator itr=Fields.begin();itr!=Fields.end();itr++)
	{
		IndexField *p = (IndexField *)*itr;
		p->index->Modified = modifiedVal;
	}
}

Field *RecordBase::GetField(unsigned char ID)
{
	for (FieldList::iterator itr=Fields.begin();itr!=Fields.end();itr++)
	{
		IndexField *p = (IndexField *)*itr;
		if (p->GetFieldId() == ID)
			return p;
	}
	return NULL;
}

void RecordBase::AddField(Field *field)
{
	if (!field)	return;
	if (GetField(field->ID))
		return;

	Fields.push_back(field);
}

int IndexRecord::WriteFields(Table *ParentTable)
{
	Field *previous = 0;
	for (FieldList::iterator itr=Fields.begin();itr!=Fields.end();itr++)
	{
		IndexField *p = (IndexField *)*itr;
		p->WriteField(ParentTable, previous, (Field *)p->next);
		previous = p;
	}
	return WriteIndex(ParentTable);
}


int IndexRecord::WriteIndex(Table *ParentTable)
{
	int P=0;
	if (!Fields.empty())
		P=(*Fields.begin())->GetFieldPos();
	return ParentTable->index->Update(INDEX_RECORD_NUM, P, this, FALSE);
}

void RecordBase::RemoveField(Field *field)
{
	if (!field)
		return;
	Fields.erase(field);
	delete field;
}

void IndexRecord::WalkFields(FieldsWalker callback, void *context)
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

