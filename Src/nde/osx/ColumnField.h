/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

ColumnField Class Prototypes
Mac OS X implementation
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
	ColumnField(unsigned char FieldID, CFStringRef FieldName, unsigned char FieldType, Table *parentTable);
	ColumnField();
	~ColumnField();

	/* Field implementation */
	virtual void ReadTypedData(const uint8_t *, size_t len);
	virtual void WriteTypedData(uint8_t *, size_t len);
	virtual size_t GetDataSize(void);
	virtual int Compare(Field *Entry);

	/* ColumnField methods */
	void InitField(void);
	void SetDataType(unsigned char type);
	bool IsSearchableField() const;
	void SetSearchable(bool val);
	unsigned char GetDataType(void);
	CFStringRef	GetFieldName(void);

protected:
	bool searchable;
	CFStringRef Name;
	unsigned char MyType;
};

#endif