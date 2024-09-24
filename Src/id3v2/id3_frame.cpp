// The authors have released ID3Lib as Public Domain (PD) and claim no copyright,
// patent or other intellectual property protection in this work. This means that
// it may be modified, redistributed and used in commercial and non-commercial
// software and hardware without restrictions. ID3Lib is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
//
// The ID3Lib authors encourage improvements and optimisations to be sent to the
// ID3Lib coordinator, currently Dirk Mahoney (dirk@id3.org). Approved
// submissions may be altered, and will be included and released under these terms.
//
// Mon Nov 23 18:34:01 1998
// improved/optimized/whatever 10/30/00 JF


#include <string.h>
#include "id3_tag.h"


ID3_Frame::ID3_Frame(ID3_FrameID id)
{
	luint lwordsForFields = 0;

	version = ID3_TAGVERSION;
	revision = ID3_TAGREVISION;
	numFields = 0;
	fields = NULL;
	groupingID[0] = 0;
	encryptionID[0] = 0;
	compression = false;

	lwordsForFields = (((luint) ID3FN_LASTFIELDID) - 1) / (sizeof (luint) * 8);

	if ((((luint) ID3FN_LASTFIELDID) - 1) % (sizeof (luint) * 8) != 0)
		lwordsForFields++;

	if (fieldBits = (bitset)calloc(lwordsForFields, sizeof(*fieldBits)))
	{
		for (luint i = 0; i < lwordsForFields; i++)
			fieldBits[i] = 0;
	}
	else
		ID3_THROW (ID3E_NoMemory);

	SetID (id);
}


ID3_Frame::~ID3_Frame (void)
{
	Clear();

	if (fieldBits)
		free(fieldBits);
}


void ID3_Frame::Clear (void)
{
	if (numFields && fields)
	{
		for (luint i = 0; i < numFields; i++)
			delete fields[i];

		free(fields);

		fields = NULL;
		numFields = 0;
		hasChanged = true;
	}

	return ;
}


void ID3_Frame::SetID (ID3_FrameID id)
{
	ID3_FrameDef *info;

	Clear();

	if (id != ID3FID_NOFRAME)
	{
		if (info = ID3_FindFrameDef (id))
		{
			frameID = id;

			numFields = 0;

			while (info->fieldDefs[numFields].id != ID3FN_NOFIELD)
				numFields++;

			if ((fields = (ID3_Field **)calloc(numFields, sizeof(ID3_Field*))) == NULL)
				ID3_THROW (ID3E_NoMemory);
			else
			{
				for (luint i = 0; i < numFields; i++)
				{
					if ((fields[i] = new ID3_Field) == NULL)
						ID3_THROW (ID3E_NoMemory);
					else
					{
						fields[i]->name = info->fieldDefs[i].id;
						fields[i]->type = info->fieldDefs[i].type;
						fields[i]->fixedLength = info->fieldDefs[i].fixedLength;
						fields[i]->ioVersion = info->fieldDefs[i].version;
						fields[i]->ioRevision = info->fieldDefs[i].revision;
						fields[i]->control = info->fieldDefs[i].control;
						fields[i]->flags = info->fieldDefs[i].flags;

						// tell the frame that this field is present
						BS_SET (fieldBits, fields[i]->name);
					}
				}

				hasChanged = true;
			}
		}
		else
			ID3_THROW (ID3E_InvalidFrameID);
	}

	return ;
}


ID3_FrameID ID3_Frame::GetID (void)
{
	return frameID;
}


void ID3_Frame::SetVersion (uchar ver, uchar rev)
{
	if (version != ver || revision != rev)
		hasChanged = true;

	version = ver;
	revision = rev;

	return ;
}


lsint ID3_Frame::FindField (ID3_FieldID fieldName)
{
	if (BS_ISSET (fieldBits, fieldName))
	{
		lsint num = 0;
		while (num < (lsint) numFields)
		{
			if (fields[num]->name == fieldName) return num;
			num++;
		}
		return -1;
	}
	return 0;
}


ID3_Field& ID3_Frame::Field (ID3_FieldID fieldName)
{
	luint fieldNum = FindField (fieldName);

	if (fieldNum == -1)
		ID3_THROW (ID3E_FieldNotFound);

	return *fields[fieldNum];
}


void ID3_Frame::UpdateFieldDeps (void)
{
	for (luint i = 0; i < numFields; i++)
	{
		if (fields[i]->flags & ID3FF_ADJUSTEDBY)
		{
			switch (fields[i]->type)
			{
			case ID3FTY_BITFIELD:
				{
					//luint value = 0;

					// now find the field on which this
					// field is dependent and get a copy
					// of the value of that field.
					// then adjust the fixedLength of this
					// field to that value / 8.
				}
				break;
			}
		}
	}

	return ;
}


void ID3_Frame::UpdateStringTypes(void)
{
	for (luint i = 0; i < numFields; i++)
	{
		if (fields[i]->flags & ID3FF_ADJUSTENC)
		{
			ID3_TextEnc enc;
			ID3_FieldType newType;

			enc = (ID3_TextEnc) Field(ID3FN_TEXTENC).Get();

			switch (enc)
			{
			case ID3TE_ASCII:
				newType = ID3FTY_ASCIISTRING;
				break;

			case ID3TE_UNICODE:
				newType = ID3FTY_UNICODESTRING;
				break;

			case ID3TE_UTF8:
				newType = ID3FTY_UTF8STRING;
				break;

			default:
				newType = ID3FTY_ASCIISTRING;
				break;
			}

			fields[i]->type = newType;
		}
	}
}


luint ID3_Frame::Size(void)
{
	luint bytesUsed = 0;
	ID3_FrameHeader header;

	header.SetVersion (version, revision);
	bytesUsed = header.Size();

	if (strlen (encryptionID))
		bytesUsed++;

	if (strlen (groupingID))
		bytesUsed++;

	// this call is to tell the string fields
	// what they should be rendered/parsed as
	// (ASCII or Unicode)
	UpdateStringTypes();

	for (luint i = 0; i < numFields; i++)
	{
		fields[i]->SetVersion (version, revision);
		bytesUsed += fields[i]->BinSize();
	}

	return bytesUsed;
}


bool ID3_Frame::HasChanged (void)
{
	if (hasChanged) return hasChanged;

	for (luint i = 0; i < numFields; i++)
	{
		bool changed = fields[i]->HasChanged();
		if (changed) return changed;
	}
	return 0;
}
