#include "ID3v1Metadata.h"
#include "metadata/MetadataKeys.h"
#include <stdlib.h>

api_metadata *ID3v1Metadata::metadata_api=0;

ID3v1Metadata::ID3v1Metadata()
{
	id3v1_tag=0;
}

ID3v1Metadata::~ID3v1Metadata()
{
}

int ID3v1Metadata::Initialize(api_metadata *metadata_api)
{
	ID3v1Metadata::metadata_api = metadata_api;
	return NErr_Success;
}

int ID3v1Metadata::Initialize(nsid3v1_tag_t tag)
{
	id3v1_tag = tag;
	this->metadata_api = metadata_api;
	return NErr_Success;
}

/* ifc_metadata implementation */
int ID3v1Metadata::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	if (!id3v1_tag)
		return NErr_Unknown;

	switch (field)
	{
	case MetadataKeys::TITLE:
		return index?NErr_EndOfEnumeration:NSID3v1_Get_Title(id3v1_tag, value);
	case MetadataKeys::ARTIST:
		return index?NErr_EndOfEnumeration:NSID3v1_Get_Artist(id3v1_tag, value);
	case MetadataKeys::ALBUM:
		return index?NErr_EndOfEnumeration:NSID3v1_Get_Album(id3v1_tag, value);
	case MetadataKeys::YEAR:
		return index?NErr_EndOfEnumeration:NSID3v1_Get_Year(id3v1_tag, value);
	case MetadataKeys::COMMENT:
		return index?NErr_EndOfEnumeration:NSID3v1_Get_Comment(id3v1_tag, value);
	case MetadataKeys::TRACK:
		return index?NErr_EndOfEnumeration:NSID3v1_Get_Track(id3v1_tag, value);
	case MetadataKeys::GENRE: 
		{
			if (!metadata_api)
				return NErr_Unknown;
			if (index > 0)
				return NErr_EndOfEnumeration;

			uint8_t genre_id;
			int ret = NSID3v1_Int_Get_Genre(id3v1_tag, &genre_id);
			if (ret != NErr_Success)
				return ret;

			nx_string_t genre;
			ret = metadata_api->GetGenre(genre_id, &genre);
			if (ret == NErr_Success)
			{
				*value = NXStringRetain(genre);
				return NErr_Success;
			}
			else if (ret == NErr_Unknown)
			{
				return NErr_Empty;
			}
			else
			{
				return ret;
			}
		}
	}

	return NErr_Unknown;
}

int ID3v1Metadata::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	if (!id3v1_tag)
		return NErr_Unknown;

	switch (field)
	{
	case MetadataKeys::YEAR:
		{
			if (index > 0)
				return NErr_EndOfEnumeration;
			unsigned int year;
			int ret = NSID3v1_Int_Get_Year(id3v1_tag, &year);
			if (ret == NErr_Success)
				*value = (int64_t)year;
			return ret;
		}
	case MetadataKeys::TRACK:
		{
			if (index > 0)
				return NErr_EndOfEnumeration;
			uint8_t track;
			int ret = NSID3v1_Int_Get_Track(id3v1_tag, &track);
			if (ret == NErr_Success)
				*value = (int64_t)track;
			return ret;
		}
	}
	return NErr_Unknown;
}

int ID3v1Metadata::Metadata_GetReal(int field, unsigned int index, double *value)
{
	if (!id3v1_tag)
		return NErr_Unknown;

	return NErr_Unknown;
}

int ID3v1Metadata::MetadataEditor_SetField(int field, unsigned int index, nx_string_t value)
{
	if (!id3v1_tag)
		return NErr_NullPointer;

	switch (field)
	{
	case MetadataKeys::TITLE:
		return index?NErr_EndOfEnumeration:NSID3v1_Set_Title(id3v1_tag, value);
	case MetadataKeys::ARTIST:
		return index?NErr_EndOfEnumeration:NSID3v1_Set_Artist(id3v1_tag, value);
	case MetadataKeys::ALBUM:
		return index?NErr_EndOfEnumeration:NSID3v1_Set_Album(id3v1_tag, value);
	case MetadataKeys::YEAR:
		return index?NErr_EndOfEnumeration:NSID3v1_Set_Year(id3v1_tag, value);
	case MetadataKeys::COMMENT:
		return index?NErr_EndOfEnumeration:NSID3v1_Set_Comment(id3v1_tag, value);
	case MetadataKeys::TRACK:
		return index?NErr_EndOfEnumeration:NSID3v1_Set_Track(id3v1_tag, value);

	case MetadataKeys::GENRE: 
		{
			if (!metadata_api)
				return NErr_Unknown;
			if (index > 0)
				return NErr_EndOfEnumeration;

			uint8_t genre_id;
			int ret = metadata_api->GetGenreID(value, &genre_id);
			if (ret == NErr_Success)
				return NSID3v1_Int_Set_Genre(id3v1_tag, genre_id);
			else
				return NSID3v1_Int_Set_Genre(id3v1_tag, 0xFF);
		}
		

	}

	return NErr_Unknown;
}

int ID3v1Metadata::MetadataEditor_SetInteger(int field, unsigned int index, int64_t value)
{
	if (!id3v1_tag)
		return NErr_NullPointer;

	if (index != 0) 
		return NErr_EndOfEnumeration;

	switch (field)
	{
	case MetadataKeys::YEAR:
		return NSID3v1_Int_Set_Year(id3v1_tag, (unsigned int)value);
	case MetadataKeys::TRACK:
		if (value < 0 || value > 255)
			return NErr_ParameterOutOfRange;
		return NSID3v1_Int_Set_Track(id3v1_tag, (uint8_t)value);
	case MetadataKeys::GENRE:
		if (value < 0 || value > 255)
			return NErr_ParameterOutOfRange;
		return NSID3v1_Int_Set_Genre(id3v1_tag, (uint8_t)value);
	}

	return NErr_Unknown;
}

#undef DESCRIPTION
