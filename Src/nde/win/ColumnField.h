/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

ColumnField Class Prototypes

--------------------------------------------------------------------------- */

#ifndef __COLUMNFIELD_H
#define __COLUMNFIELD_H

#include "Field.h"
#include "LinkedList.h"
#include "Table.h"

#include "Scanner.h"

class ColumnField : public Field
{
public:
	ColumnField(unsigned char FieldID, const wchar_t *FieldName, unsigned char FieldType, Table *parentTable);
	ColumnField();
	~ColumnField();
	virtual void ReadTypedData(const uint8_t *, size_t len);
	virtual void WriteTypedData(uint8_t *, size_t len);
	virtual size_t GetDataSize(void);
	virtual int Compare(Field *Entry);
	void InitField(void);

	bool IsSearchableField() const;
	void SetSearchable(bool val);

	unsigned char GetDataType(void);
	void SetDataType(unsigned char type);

	wchar_t *GetFieldName(void); // not const because it's an NDE string
	void SetFieldName(wchar_t *name);

protected:
	bool searchable;
	wchar_t *Name;
	unsigned char MyType;
};

#endif