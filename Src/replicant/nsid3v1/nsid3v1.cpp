#include "nsid3v1.h"
#include "tag.h"

#include "foundation/error.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <new>


int NSID3v1_Header_Valid(const void *data, size_t len)  
{
	// return NErr_True or NErr_False depending on whether or not you detect that it's valid ID3v1	
	if (memcmp(data, "TAG", 3) == 0)
	{
		return NErr_True;
	}

	return NErr_False;
}

int NSID3v1_Tag_Create(const void *data, size_t len, nsid3v1_tag_t *out_tag)
{
	ID3v1::Tag *tag = new (std::nothrow) ID3v1::Tag();
	if (!tag)
		return NErr_OutOfMemory;

	int ret = tag->Parse(data, len);
	if (ret != NErr_Success)
		return ret;
	
	*out_tag = (nsid3v1_tag_t)tag;
		
	return NErr_Success;
}

int NSID3v1_Tag_New(nsid3v1_tag_t *out_tag)
{
	ID3v1::Tag *tag = new (std::nothrow) ID3v1::Tag();
	if (!tag)
		return NErr_OutOfMemory;

	tag->New();

	*out_tag = (nsid3v1_tag_t)tag;
		
	return NErr_Success;
}

int NSID3v1_Tag_Serialize(const nsid3v1_tag_t t, void *data, size_t len)
{
	ID3v1::Tag *tag = (ID3v1::Tag *)t;
	if (!tag)
		return NErr_BadParameter;

	return tag->Serialize(data, len);
}

int NSID3v1_Tag_Destroy(nsid3v1_tag_t t)
{
	ID3v1::Tag *tag = (ID3v1::Tag *)t;
	if (!tag)
		return NErr_BadParameter;
	delete tag;
	return NErr_Success;
}


// Field getters
int NSID3v1_Field_Text_Get(const nsid3v1_tag_t t, const int field, nx_string_t *out_value)
{
	const ID3v1::Tag *tag = (const ID3v1::Tag *)t;
	char track_num[4] = { 0, 0, 0 };

	if (tag)
	{
		switch (field)
		{
		case NSID3V1_TITLE:
			return NSID3v1_Get_Title(t, out_value);
		case NSID3V1_ARTIST:
			return NSID3v1_Get_Artist(t, out_value);
		case NSID3V1_ALBUM:
			return NSID3v1_Get_Album(t, out_value);
		case NSID3V1_YEAR:
			return NSID3v1_Get_Year(t, out_value);
		case NSID3V1_COMMENT:
			return NSID3v1_Get_Comment(t, out_value);
		case NSID3V1_TRACK:
			return NSID3v1_Get_Track(t, out_value);
		//case NSID3V1_GENRE:
			//return NSID3v1_Get_Genre(t, out_value);
		default:
			return NErr_Unknown;
		}
	}
	else
		return NErr_Empty;
}

int NSID3v1_Get_Title(nsid3v1_tag_t t, nx_string_t *value)
{
	const ID3v1::Tag *tag = (const ID3v1::Tag *)t;
	if (tag)
	{
		size_t value_length = tag->GetTitleLength();
		if (value_length > 0)
			return NXStringCreateWithBytes(value, tag->GetTitle(), value_length, nx_charset_latin1);
		else
			return NErr_Empty;
		return NErr_Success;
	}
	return NErr_Empty;
}

int NSID3v1_Get_Artist(nsid3v1_tag_t t, nx_string_t *value)
{
	const ID3v1::Tag *tag = (const ID3v1::Tag *)t;
	if (tag)
	{
		size_t value_length = tag->GetArtistLength();
		if (value_length > 0)
			return NXStringCreateWithBytes(value, tag->GetArtist(), value_length, nx_charset_latin1);
		else
			return NErr_Empty;
		return NErr_Success;
	}
	return NErr_Empty;
}

int NSID3v1_Get_Album(nsid3v1_tag_t t, nx_string_t *value)
{
	const ID3v1::Tag *tag = (const ID3v1::Tag *)t;
	if (tag)
	{
		size_t value_length = tag->GetAlbumLength();
		if (value_length > 0)
			return NXStringCreateWithBytes(value, tag->GetAlbum(), value_length, nx_charset_latin1);
		else
			return NErr_Empty;
		return NErr_Success;
	}
	return NErr_Empty;
}

int NSID3v1_Get_Year(nsid3v1_tag_t t, nx_string_t *value)
{
	const ID3v1::Tag *tag = (const ID3v1::Tag *)t;
	if (tag)
	{
		size_t value_length = tag->GetYearLength();
		if (value_length > 0)
			return NXStringCreateWithBytes(value, tag->GetYear(), value_length, nx_charset_latin1);
		else
			return NErr_Empty;
	}
	return NErr_Empty;
}

int NSID3v1_Get_Comment(nsid3v1_tag_t t, nx_string_t *value)
{
	const ID3v1::Tag *tag = (const ID3v1::Tag *)t;
	if (tag)
	{
		size_t value_length = tag->GetCommentLength();
		if (value_length > 0)
			return NXStringCreateWithBytes(value, tag->GetComment(), value_length, nx_charset_latin1);
		else
			return NErr_Empty;
	}
	return NErr_Empty;
}

int NSID3v1_Get_Track(nsid3v1_tag_t t, nx_string_t *value)
{
	const ID3v1::Tag *tag = (const ID3v1::Tag *)t;
	

	if (tag)
	{
		unsigned char track = tag->GetTrack();
		if (track > 0)
			return NXStringCreateWithUInt64(value, track);
		else
			return NErr_Empty;
		return NErr_Success;
	}
	return NErr_Empty;
}

int NSID3v1_Int_Get_Year(nsid3v1_tag_t t, unsigned int *value)
{
	const ID3v1::Tag *tag = (const ID3v1::Tag *)t;
	if (tag)
	{
		char year[5];
		memcpy(year, tag->GetYear(), 4);
		year[4]=0;
		*value = strtoul(year, 0, 10);
		return NErr_Success;
	}
	return NErr_Empty;
}

int NSID3v1_Int_Get_Track(nsid3v1_tag_t t, uint8_t *value)
{
	const ID3v1::Tag *tag = (const ID3v1::Tag *)t;
	if (tag)
	{
		*value = tag->GetTrack();
		return NErr_Success;
	}
	return NErr_Empty;
}

int NSID3v1_Int_Get_Genre(nsid3v1_tag_t t, uint8_t *value)
{
	const ID3v1::Tag *tag = (const ID3v1::Tag *)t;
	if (tag)
	{
		*value = tag->GetGenre();
		return NErr_Success;
	}
	return NErr_Empty;
}

/* ================= setters ================= */
typedef void (ID3v1::Tag::*Setter)(const char *, size_t length);
template <size_t limit>
static int SetFromString(ID3v1::Tag *tag, Setter setter, nx_string_t value)
{
	if (!value)
	{
		(tag->*setter)(0, 0);
		return NErr_Success;
	}

	char temp[limit];
	size_t bytes_copied;
	int ret = NXStringGetBytes(&bytes_copied, value, temp, limit, nx_charset_latin1, 0);
	if (ret != NErr_Success)
		return ret;

	(tag->*setter)(temp, bytes_copied);
	return NErr_Success;
}

int NSID3v1_Set_Title(nsid3v1_tag_t t, nx_string_t value)
{
	ID3v1::Tag *tag = (ID3v1::Tag *)t;
	if (!t)
		return NErr_BadParameter;
	return SetFromString<30>(tag, &ID3v1::Tag::SetTitle, value);	
}

int NSID3v1_Set_Artist(nsid3v1_tag_t t, nx_string_t value)
{
	ID3v1::Tag *tag = (ID3v1::Tag *)t;
	if (!t)
		return NErr_BadParameter;
	return SetFromString<30>(tag, &ID3v1::Tag::SetArtist, value);
}

int NSID3v1_Set_Album(nsid3v1_tag_t t, nx_string_t value)
{
	ID3v1::Tag *tag = (ID3v1::Tag *)t;
	if (!t)
		return NErr_BadParameter;

	return SetFromString<30>(tag, &ID3v1::Tag::SetAlbum, value);
}

int NSID3v1_Set_Year(nsid3v1_tag_t t, nx_string_t value)
{
	ID3v1::Tag *tag = (ID3v1::Tag *)t;
	if (!t)
		return NErr_BadParameter;

	return SetFromString<4>(tag, &ID3v1::Tag::SetYear, value);	
}

int NSID3v1_Set_Comment(nsid3v1_tag_t t, nx_string_t value)
{
	ID3v1::Tag *tag = (ID3v1::Tag *)t;
	if (!t)
		return NErr_BadParameter;

	return SetFromString<28>(tag, &ID3v1::Tag::SetComment, value);	
}

int NSID3v1_Set_Track(nsid3v1_tag_t t, nx_string_t value)
{
	ID3v1::Tag *tag = (ID3v1::Tag *)t;
	if (!t)
		return NErr_BadParameter;

	if (!value)
	{
		tag->SetTrack(0);
		return NErr_Success;
	}

	int temp=0;
	int ret = NXStringGetIntegerValue(value, &temp);
	if (ret != NErr_Success)
		return ret;

	if (temp < 0 || temp > 255)
		return NErr_ParameterOutOfRange;

	tag->SetTrack((uint8_t)temp);
	return NErr_Success;
}

int NSID3v1_Int_Set_Year(nsid3v1_tag_t t, unsigned int value)
{
	ID3v1::Tag *tag = (ID3v1::Tag *)t;
	if (!t)
		return NErr_BadParameter;

	if (value > 9999)
		return NErr_ParameterOutOfRange;

	char temp[5];
	sprintf(temp, "%u", value);
	tag->SetYear(temp, 4);
	
	return NErr_Success;
}

int NSID3v1_Int_Set_Track(nsid3v1_tag_t t, uint8_t value)
{
	ID3v1::Tag *tag = (ID3v1::Tag *)t;
	if (!t)
		return NErr_BadParameter;

	tag->SetTrack(value);
	return NErr_Success;
}


int NSID3v1_Int_Set_Genre(nsid3v1_tag_t t, uint8_t value)
{
		ID3v1::Tag *tag = (ID3v1::Tag *)t;
	if (!t)
		return NErr_BadParameter;

	tag->SetGenre(value);
	return NErr_Success;
}
