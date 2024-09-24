#pragma once

#include "../nde.h"
#include "../Field.h"
#include "Vfs.h"
#include <stdio.h>
#include <deque>

class ColumnField;

class RecordBase
{
public:
	RecordBase();
	virtual ~RecordBase();
	Field *GetField(unsigned char ID);
	void Retain();
	void Release();
	void RemoveField(Field *field);
	void AddField(Field *field);
	Field* GetLastField()
	{
		if (Fields.empty())
		{
			return nullptr;
		}
		return Fields[Fields.size() - 1];
	}
protected:
	void Undelete(void);

	int InsertionPoint; // TODO: benski> might be able to pass this into WriteFields/WriteIndex
	int ref_count;
	typedef std::deque<Field*> FieldList;
	FieldList Fields;
};

class Record : public RecordBase
{
public:
	Record(int RecordPos, int insertionPoint, VFILE *FileHandle,Table *p);
	bool InCache() { return ref_count > 1; }

	ColumnField *GetColumnByName(const wchar_t *name);
	int WriteFields(Table *ParentTable, int RecordIndex);
	int WriteIndex(Table *ParentTable, int RecordIndex);
	void Delete(Table *ParentTable, int RecordIndex);
	typedef bool (__cdecl *FieldsWalker)(Record *record, Field *entry, void *context);
	NDE_API void WalkFields(FieldsWalker callback, void *context);
};