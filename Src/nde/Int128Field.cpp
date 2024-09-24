/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 Int128Field Class
Field data layout:
[16 bytes] value
--------------------------------------------------------------------------- */

#include "nde.h"
#include "Int128Field.h"
#include <time.h>

//---------------------------------------------------------------------------
Int128Field::Int128Field(void *data)
{
	InitField();
	Type = FIELD_INT128;
	memcpy(Value, data, 16);
}

//---------------------------------------------------------------------------
void Int128Field::InitField(void)
{
	Type = FIELD_INT128;
	memset(Value, 0, 16);
}

//---------------------------------------------------------------------------
Int128Field::Int128Field()
{
	InitField();
}

//---------------------------------------------------------------------------
Int128Field::~Int128Field()
{
}

//---------------------------------------------------------------------------
void Int128Field::ReadTypedData(const uint8_t *data, size_t len)
{
	if (len < 16) return;
	memcpy(Value, data, 16);
}

//---------------------------------------------------------------------------
void Int128Field::WriteTypedData(uint8_t *data, size_t len)
{

	if (len < 16) return;
	memcpy(data, Value, 16);
}

//---------------------------------------------------------------------------
void *Int128Field::GetValue(void)
{
	return Value;
}

//---------------------------------------------------------------------------
void Int128Field::SetValue(const void *Val)
{
	memcpy(Value, Val, 16);
}

//---------------------------------------------------------------------------
size_t Int128Field::GetDataSize(void)
{
	return 16;
}

//---------------------------------------------------------------------------
int Int128Field::Compare(Field *Entry)
{
	if (!Entry) return -1;
	return memcmp(Value, ((Int128Field*)Entry)->GetValue(), 16);
}

static char zerobuf[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//---------------------------------------------------------------------------
bool Int128Field::ApplyFilter(Field *Data, int op)
{
	void *p = ((Int128Field *)Data)->GetValue();
	void *d = Value;
	if (!p)
		p = zerobuf;
	if (!d)
		d = zerobuf;
	bool r;
	switch (op)
	{
		case FILTER_EQUALS:
			r = !memcmp(d, p, 16);
			break;
		case FILTER_CONTAINS:
			r = !memcmp(d, p, 16);
			break;
		case FILTER_ABOVE:
			r = (memcmp(d, p, 16) > 0);
			break;
		case FILTER_BELOW:
			r = (memcmp(d, p, 16) < 0);
			break;
		case FILTER_BELOWOREQUAL:
			r = (memcmp(d, p, 16) <= 0);
			break;
		case FILTER_ABOVEOREQUAL:
			r = (memcmp(d, p, 16) >= 0);
			break;
		case FILTER_ISEMPTY:
			r = !d || (!memcmp(d, zerobuf, 16));
			break;
		case FILTER_ISNOTEMPTY:
			r = d && (memcmp(d, zerobuf, 16));
			break;
		default:
			r = true;
			break;
	}
	return r;
}