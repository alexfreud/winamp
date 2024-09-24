/* 
---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
---------------------------------------------------------------------------
*/

/* 
---------------------------------------------------------------------------

StringField Class Prototypes

Android (linux) implementation
---------------------------------------------------------------------------
*/

#ifndef __STRINGFIELD_H
#define __STRINGFIELD_H
#include "../NDEString.h"

class StringField : public Field
{
public:
	StringField();
	~StringField();
	

	StringField(const char *Str, int strkind=STRING_IS_WCHAR);
	char *GetString(void);
	void SetString(const char *Str);
	void SetNDEString(char *Str);

protected:
	virtual void ReadTypedData(const uint8_t *, size_t len);
	virtual void WriteTypedData(uint8_t *, size_t len);
	virtual size_t GetDataSize(void);
	virtual int Compare(Field *Entry);
	virtual int Starts(Field *Entry);
	virtual int Contains(Field *Entry);

	virtual bool ApplyFilter(Field *Data, int op);
	virtual Field *Clone(Table *pTable);
	void InitField(void);
	
	char *String;
};

#endif

