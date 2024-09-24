#pragma once

#include "field.h"
#include "Vfs.h"
#include <stdio.h>
#include "../nu/PtrMap.h"
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif
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

protected:
	void Undelete(void);

	int InsertionPoint; // TODO: benski> might be able to pass this into WriteFields/WriteIndex
	int ref_count;
	typedef nu::PtrDeque2<Field> FieldList;
	FieldList Fields;
};


class Record : public RecordBase
{
public:
	Record(int RecordPos, int insertionPoint, VFILE *FileHandle,Table *p);
	bool InCache() { return ref_count > 1; }

#ifdef __APPLE__
	ColumnField *GetColumnByName(CFStringRef name);
#else
	ColumnField *GetColumnByName(const wchar_t *name);
#endif
	int WriteFields(Table *ParentTable, int RecordIndex);
	int WriteIndex(Table *ParentTable, int RecordIndex);
	void Delete(Table *ParentTable, int RecordIndex);
	typedef bool (*FieldsWalker)(Record *record, Field *entry, void *context);
	NDE_API void WalkFields(FieldsWalker callback, void *context);
};

