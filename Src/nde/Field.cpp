/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

Field Class

--------------------------------------------------------------------------- */

#include "field.h"
#include "vfs.h"
#include "table.h"
#include "nde.h"
#include "dbutils.h"

#ifdef WIN32
#include <malloc.h>
#elif defined(__APPLE__)
#include <alloca.h>
#endif

//---------------------------------------------------------------------------
void PUT_BINARY(uint8_t *dest, const uint8_t *src, size_t size, size_t pos)
{
	if (src && dest && size > 0)
		memcpy(dest+pos, src, size);
}

//---------------------------------------------------------------------------
void GET_BINARY(uint8_t *dest, const uint8_t *src, size_t size, size_t pos) 
{
	if (dest && src && size > 0)
		memcpy(dest, src+pos, size);
}

//---------------------------------------------------------------------------
void PUT_FLOAT(float f, uint8_t *data, size_t pos)
{
	unsigned int y = *(const unsigned int *)&f;
	data[pos]=(unsigned char)(y&255); data[pos+1]=(unsigned char)((y>>8)&255); data[pos+2]=(unsigned char)((y>>16)&255); data[pos+3]=(unsigned char)((y>>24)&255);
}

//---------------------------------------------------------------------------
float GET_FLOAT(const uint8_t *data, size_t pos)
{
	int a = data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24);
	float f = *(const float *)&a;
	return f;
}

//---------------------------------------------------------------------------
Field::Field(int FieldPos)
{
	InitField();
	Pos = FieldPos;
}

//---------------------------------------------------------------------------
void Field::InitField(void)
{
	Type = FIELD_UNKNOWN;
	Pos = 0;
	ID = 0;
	MaxSizeOnDisk = 0;
}

//---------------------------------------------------------------------------
Field::Field()
{
	InitField();
}

//---------------------------------------------------------------------------
Field::~Field()
{
}

//---------------------------------------------------------------------------
Field *Field::ReadField(Table *pTable, int pos, uint32_t *next_position)
{
	return ReadField(pTable, pos, false, next_position);
}

//---------------------------------------------------------------------------
Field *Field::ReadField(Table *pTable, int pos, bool Quick, uint32_t *next_position)
{
	VFILE *HTable = pTable->Handle;
	int newPos = pos;
	int rType = FIELD_REDIRECTOR;
	unsigned char oType;

	if (!Vflock(HTable, false))
		return 0;

	while  (rType == FIELD_REDIRECTOR)
	{
		Vfseek(HTable, Pos, SEEK_SET);
		if (Vfread(&ID, sizeof(ID), HTable) != 1 || 
			Vfread(&oType, sizeof(oType), HTable) != 1) 
		{
			Vfunlock(HTable, false);
			return NULL;
		}
		if (oType == FIELD_REDIRECTOR)
		{
			if (Vfread(&newPos, sizeof(newPos), HTable) != 1) 
			{
				Vfunlock(HTable, false);
				return NULL;
			}
			Pos = newPos;
		}
		rType = oType;
	}
	if (Quick)
		Vfseek(HTable, sizeof(MaxSizeOnDisk), SEEK_CUR);
	else
	{
		if (Vfread(&MaxSizeOnDisk, sizeof(MaxSizeOnDisk), HTable) != 1) 
		{
			Vfunlock(HTable, false);
			return NULL;
		}
	}
	uint32_t NextFieldPos = 0;
	if (Vfread(&NextFieldPos, sizeof(NextFieldPos), HTable) != 1) 
	{
		Vfunlock(HTable, false);
		return NULL;
	}
	if (next_position) *next_position = NextFieldPos;
	if (Quick)
	{
		Vfunlock(HTable, false);
		return this;
	}

	uint32_t PreviousFieldPos = 0;
	if (Vfread(&PreviousFieldPos, sizeof(PreviousFieldPos), HTable) != 1) 
	{
		Vfunlock(HTable, false);
		return NULL;
	}

	Field *O=NULL;
	O = TranslateObject(oType, pTable);
	if (O)
	{
		O->ID = ID;
		O->Type = oType;
		O->Pos = Pos;
		O->MaxSizeOnDisk = MaxSizeOnDisk;
		uint8_t *data = NULL;
		if (HTable->cached && MaxSizeOnDisk > VFILE_INC) 
		{
			pTable->IncErrorCount();
			MaxSizeOnDisk = (uint32_t)GetDataSize();
			O->MaxSizeOnDisk = MaxSizeOnDisk;
		}
		else if (HTable->cached)
		{
			data = HTable->data+HTable->ptr; // benski> uber-hack
			Vfseek(HTable, MaxSizeOnDisk, SEEK_CUR);
		}
		else
		{
			data = (uint8_t *)_malloca(MaxSizeOnDisk);
			Vfread(data, MaxSizeOnDisk, HTable);
		}

		if (data) 
		{
			O->ReadTypedData(data, MaxSizeOnDisk);
			if (!HTable->cached)
			{
				_freea(data);
			}
		}
		Vfunlock(HTable, false);
		return O;
	}
	Vfunlock(HTable, false);
	return NULL;
}

//---------------------------------------------------------------------------
void Field::WriteField(Table *pTable, Field *previous_field, Field *next_field)
{
	VFILE *HTable = pTable->Handle;
	if (Pos == -1) return;
	size_t data_size = GetDataSize();
	if (HTable->cached && MaxSizeOnDisk > VFILE_INC) {
		pTable->IncErrorCount();
		MaxSizeOnDisk = (uint32_t)data_size;
	}
	
	if (Pos == 0 || (uint32_t)data_size > MaxSizeOnDisk)
	{
		MaxSizeOnDisk = (uint32_t)data_size;
		uint32_t newPos = AllocNewPos(HTable);
		if (Pos != 0)
		{
			unsigned char v = 0;
			Vfseek(HTable, Pos, SEEK_SET);
			Vfwrite(&v, sizeof(v), HTable);
			v = FIELD_REDIRECTOR;
			Vfwrite(&v, sizeof(v),  HTable);
			Vfwrite(&newPos, sizeof(newPos), HTable);
		}
		Pos = newPos;
		if (previous_field)
		{
			//previous_field->NextFieldPos = Pos;
			if (previous_field->Pos)
			{
				Vfseek(HTable, previous_field->Pos+sizeof(ID)+sizeof(MaxSizeOnDisk)+sizeof(Type), SEEK_SET);
				Vfwrite(&Pos, sizeof(Pos), HTable);
			}
		}
		if (next_field)
		{
			if (next_field->Pos)
			{
				Vfseek(HTable, next_field->Pos+sizeof(ID)+sizeof(uint32_t/*NextFieldPos*/)+sizeof(Type)+sizeof(MaxSizeOnDisk), SEEK_SET);
				Vfwrite(&Pos, sizeof(Pos), HTable);
			}
		}
	}

	uint32_t PreviousFieldPos = 0, NextFieldPos = 0;
	if (previous_field) PreviousFieldPos = previous_field->GetFieldPos(); else PreviousFieldPos = NULL;
	if (next_field) NextFieldPos = next_field->GetFieldPos(); else NextFieldPos = NULL;

	Vfseek(HTable, Pos, SEEK_SET);
	Vfwrite(&ID, sizeof(ID), HTable);
	Vfwrite(&Type, sizeof(Type), HTable);
	Vfwrite(&MaxSizeOnDisk, sizeof(MaxSizeOnDisk), HTable);
	Vfwrite(&NextFieldPos, sizeof(NextFieldPos), HTable);
	Vfwrite(&PreviousFieldPos, sizeof(PreviousFieldPos), HTable);
	uint8_t *data = (unsigned char*)_malloca(MaxSizeOnDisk);
	WriteTypedData(data, MaxSizeOnDisk);
	Vfwrite(data, MaxSizeOnDisk, HTable);
	_freea(data);
}

//---------------------------------------------------------------------------
uint32_t Field::GetFieldPos(void)
{
	return Pos;
}

//---------------------------------------------------------------------------
Field *Field::Clone(Table *pTable)
{
	Field *clone = TranslateObject(Type, pTable);
	size_t size = GetDataSize();
	uint8_t *data = (unsigned char*)_malloca(size);
	WriteTypedData(data, size);
	clone->ReadTypedData(data, size);
	if (data) _freea(data);
	clone->Type = Type;
	//clone->HTable = HTable;
	clone->Pos = FIELD_CLONE;
	clone->ID = ID;
	//clone->NextFieldPos = 0;
	clone->MaxSizeOnDisk=(uint32_t)size;
	return clone;
}

//---------------------------------------------------------------------------
int Field::GetType() {
	return Type;    
}

size_t Field::GetTotalSize()
{
	return GetDataSize() + sizeof(ID)+sizeof(Type)+sizeof(MaxSizeOnDisk)+sizeof(uint32_t/*NextFieldPos*/)+sizeof(uint32_t /*PreviousFieldPos*/);
}