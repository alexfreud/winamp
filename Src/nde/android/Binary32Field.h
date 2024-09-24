/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 Binary32Field Class Prototypes

 Android (linux) implementation
--------------------------------------------------------------------------- */

#ifndef __NDE_BINARY32FIELD_H
#define __NDE_BINARY32FIELD_H

#include "BinaryField.h"
#include <foundation/types.h>

class Binary32Field : public BinaryField
{
protected:
	virtual void ReadTypedData(const uint8_t *, size_t len);
	virtual void WriteTypedData(uint8_t *, size_t len);
	virtual size_t GetDataSize(void);
	void InitField(void);

public:
	Binary32Field(const uint8_t *Data, size_t len);
	Binary32Field();
};

#endif
