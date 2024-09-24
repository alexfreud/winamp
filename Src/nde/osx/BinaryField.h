/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 BinaryField Class Prototypes

--------------------------------------------------------------------------- */

#ifndef __BINARYFIELD_H
#define __BINARYFIELD_H

#include "Field.h"
#include <bfc/platform/types.h>
class BinaryField : public Field
{
protected:
	virtual void ReadTypedData(const uint8_t *, size_t len);
	virtual void WriteTypedData(uint8_t *, size_t len);
	virtual size_t GetDataSize(void);
	virtual int Compare(Field *Entry);
	virtual bool ApplyFilter(Field *Data, int flags);
	void InitField(void);
	CFDataRef Data;

public:
	~BinaryField();
	BinaryField(const uint8_t *Data, int len);
	BinaryField();
	const uint8_t *GetData(size_t *len);
	void SetData(const uint8_t *Data, size_t len);
	CFDataRef GetCFData();
};

#endif
