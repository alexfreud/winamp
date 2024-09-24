/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

IndexRecord Class

--------------------------------------------------------------------------- */

#include "../nde.h"
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
	for (FieldList::iterator itr = Fields.begin(); itr != Fields.end(); itr++)
	{
		IndexField *p = (IndexField *)(* itr);
		if ((itr + 1) != Fields.end())
			p->index->Colaborate((IndexField *)(*(itr+1)));
		else
			p->index->Colaborate((IndexField *)*(Fields.begin()));
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

Field *RecordBase::GetField( unsigned char ID )
{
	for ( FieldList::iterator itr = Fields.begin(); itr != Fields.end(); itr++ )
	{
		IndexField *p = (IndexField *)*itr;
		if ( p->GetFieldId() == ID )
			return p;
	}

	return NULL;
}

void RecordBase::AddField(Field *field)
{
	if (!field)
		return;

	if (GetField(field->ID))
		return;

	Fields.push_back(field);
}

int IndexRecord::WriteFields( Table *ParentTable )
{
	Field      *l_previous_field = NULL;
	IndexField *l_index_field    = NULL;

	for ( FieldList::iterator itr = Fields.begin(); itr != Fields.end(); itr++ )
	{
		l_index_field = (IndexField *)(* itr);

		Field* nextField = (itr + 1) != Fields.end() ? *(itr + 1) : nullptr;

		//l_index_field->WriteField(ParentTable, l_previous_field, (Field*)l_index_field->next);
		l_index_field->WriteField(ParentTable, l_previous_field, nextField);

		l_previous_field = l_index_field;
	}

	return WriteIndex( ParentTable );
}


int IndexRecord::WriteIndex( Table *ParentTable )
{
	int P = 0;

	if ( !Fields.empty() )
		P = ( *Fields.begin() )->GetFieldPos();

	return ParentTable->index->Update( INDEX_RECORD_NUM, P, this, FALSE );
}

void RecordBase::RemoveField(Field *field)
{
	if (!field)
		return;

	//Fields.erase(field);
	//delete field;

	for (auto it = Fields.begin(); it != Fields.end(); it++)
	{
		Field* f = *it;
		if (f == field)
		{
			Fields.erase(it);
			delete f;
			break;
		}
	}
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