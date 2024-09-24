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

---------------------------------------------------------------------------
*/

#ifndef __STRINGFIELD_H
#define __STRINGFIELD_H

#include <CoreFoundation/CFString.h>

class StringField : public Field
{
	
public:
	StringField();
	~StringField();
	
	StringField(CFStringRef Str);
	CFStringRef GetString(); // CFRetain this if you need to keep it for a while
	void SetNDEString(CFStringRef str);

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
	
	CFStringRef String;

};

#endif

