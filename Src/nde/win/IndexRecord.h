#pragma once
#include <stdio.h>
#include "Record.h"

class IndexRecord : public RecordBase
{
public:
	IndexRecord(int RecordPos, int insertionPoint, VFILE *FileHandle,Table *p);
	void BuildCollaboration();
	bool NeedFix();
	IndexField *GetIndexByName(const wchar_t *name);
	int GetColumnCount() { return (int)(Fields.size() - 1); }
	bool CheckIndexing(int v);
	int WriteFields(Table *ParentTable);
	int WriteIndex(Table *ParentTable);
	typedef bool (*FieldsWalker)(IndexRecord *record, Field *entry, void *context);
	void WalkFields(FieldsWalker callback, void *context);
};