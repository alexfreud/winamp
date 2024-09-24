/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 Int64Field Class Prototypes

--------------------------------------------------------------------------- */

#ifndef NULLSOFT_NDE_INT64FIELD_H
#define NULLSOFT_NDE_INT64FIELD_H

#include "Field.h"

class Int64Field : public Field
{
protected:
	virtual void ReadTypedData(const uint8_t *, size_t len);
	virtual void WriteTypedData(uint8_t *, size_t len);
	virtual size_t GetDataSize(void);
	virtual int Compare(Field *Entry);
	virtual bool ApplyFilter(Field *Data, int op);
	void InitField(void);
	int64_t Value;

public:
	~Int64Field();
	Int64Field(int64_t);
	Int64Field();
	int64_t GetValue(void);
	void SetValue(int64_t);
};



#endif