/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 Int128Field Class Prototypes

--------------------------------------------------------------------------- */

#ifndef NULLSOFT_NDE_INT128FIELD_H
#define NULLSOFT_NDE_INT128FIELD_H

#include "Field.h"

class Int128Field : public Field
{
protected:
	virtual void ReadTypedData(const uint8_t *, size_t len);
	virtual void WriteTypedData(uint8_t *, size_t len);
	virtual size_t GetDataSize(void);
	virtual int Compare(Field *Entry);
	virtual bool ApplyFilter(Field *Data, int op);
	void InitField(void);
	char Value[16];
public:
	~Int128Field();
	Int128Field(void *value);
	Int128Field();
	void *GetValue(void);
	void SetValue(const void *value);
};



#endif