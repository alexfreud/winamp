/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

Field Class Prototypes

--------------------------------------------------------------------------- */

#ifndef __FIELD_H
#define __FIELD_H

#ifdef __ANDROID__
#include <foundation/types.h> // TODO
#else
#include <bfc/platform/types.h>
#endif

class Table;
#include <stdio.h>

#define PUT_INT(y) data[pos]=(uint8_t)(y&255); data[pos+1]=(uint8_t)((y>>8)&255); data[pos+2]=(uint8_t)((y>>16)&255); data[pos+3]=(uint8_t)((y>>24)&255)
#define GET_INT() (int)(data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
#define PUT_SHORT(y) data[pos]=(uint8_t)(y&255); data[pos+1]=(uint8_t)((y>>8)&255);
#define GET_SHORT() (uint16_t)(data[pos]|(data[pos+1]<<8))
#define PUT_PINT(y) data[*pos]=(uint8_t)(y&255); data[*pos+1]=(uint8_t)((y>>8)&255); data[*pos+2]=(uint8_t)((y>>16)&255); data[*pos+3]=(uint8_t)((y>>24)&255)
extern float GET_FLOAT(const uint8_t *data, size_t pos);
extern void PUT_FLOAT(float f, uint8_t *data, size_t pos);
extern void PUT_BINARY(uint8_t *dest, const uint8_t *src, size_t size, size_t pos);
extern void GET_BINARY(uint8_t *dest, const uint8_t *src, size_t size, size_t pos);
#define PUT_CHAR(y) data[pos]=y
#define GET_CHAR() data[pos]
#define CHECK_CHAR(l) { if (l < 1) return; l--; }
#define CHECK_INT(l) { if (l < 4) return; l-=4; }
#define CHECK_INT64(l) { if (l < 8) return; l-=8; }
#define CHECK_SHORT(l) { if (l < 2) return; l-=2; }
#define CHECK_BIN(l, size) { if (l < size) return; l-=size; }

#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(x) { \
	if (!(x)) \
	OutputDebugString("Assertion failed "); \
}

#define _CHECK_CHAR(l) { ASSERT(l >= 1); l-=1; }
#define _CHECK_INT(l) { ASSERT(l >= 4); l-=4; }
#define _CHECK_SHORT(l) { ASSERT(l >= 2); l-=2; }
#define _CHECK_BIN(l, size) { ASSERT(l >= size); l-=size; }

class Field //: public wa::lists::node_ptr
{
public:
	Field();
	virtual ~Field();
	virtual Field *Clone(Table *pTable);
	unsigned char GetFieldId(void)
	{
		return ID;
	}
	int GetType();
	size_t GetTotalSize();
	Field(int FieldPos);
	Field *ReadField(Table *pTable, int pos, uint32_t *next_position=0);
	Field *ReadField(Table *pTable, int pos, bool readTyped, uint32_t *next_position=0);
	void WriteField(Table *pTable, Field *previous_field, Field *next_field);
	virtual void ReadTypedData(const uint8_t *, size_t /*len*/) { };
	virtual void WriteTypedData(uint8_t *, size_t) { };
	virtual size_t GetDataSize(void)
	{
		return 0;
	}
	virtual int Compare(Field * /*Entry*/)
	{
		return 0;
	}
	virtual int Starts(Field * /*Entry*/)
	{
		return 0;
	}
	virtual int Contains(Field * /*Entry*/)
	{
		return 0;
	}
	virtual bool ApplyFilter(Field * /*Data*/, int /*flags*/) 
	{
		return false;
	}
	uint32_t GetFieldPos(void);
	void InitField(void);
	union
	{
		struct
		{
			uint8_t ID;
			uint8_t Type;
		};
		uint32_t _dummy_union_thingy;
	};
protected:
	uint32_t Pos;

	uint32_t MaxSizeOnDisk;
};

#endif