/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

ColumnField Class Prototypes

Android (linux) implementation
--------------------------------------------------------------------------- */

#ifndef __COLUMNFIELD_H
#define __COLUMNFIELD_H

#include "../Field.h"
#include "../LinkedList.h"
#include "Table.h"

#include "Scanner.h"

class ColumnField : public Field
{
public:
	ColumnField(unsigned char FieldID, const char *FieldName, unsigned char FieldType, Table *parentTable);
	ColumnField();
	~ColumnField();
	virtual void ReadTypedData(const uint8_t *, size_t len);
	virtual void WriteTypedData(uint8_t *, size_t len);
	virtual size_t GetDataSize(void);
	virtual int Compare(Field *Entry);
	void InitField(void);
	void SetDataType(unsigned char type);
	bool IsSearchableField() const;
	void SetSearchable(bool val);
public:
	unsigned char GetDataType(void);
	char *GetFieldName(void); // not const because it's an NDE string

protected:
	bool searchable;
	char *Name;
	unsigned char MyType;
};

#endif