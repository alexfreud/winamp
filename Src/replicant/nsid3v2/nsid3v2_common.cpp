#include "nsid3v2/nsid3v2.h"
#include "nsid3v2/header.h"
#include "nsid3v2/tag.h"
#include <string.h> // for memcmp
#include <new>

int NSID3v2_Header_Valid(const void *header_data)
{
	ID3v2::Header header(header_data);
	if (header.Valid())
		return NErr_Success;
	else
		return NErr_False;
}

int NSID3v2_Header_FooterValid(const void *footer_data)
{
	ID3v2::Header footer(footer_data);
	if (footer.FooterValid())
		return NErr_Success;
	else
		return NErr_False;
}

int NSID3v2_Header_Create(nsid3v2_header_t *h, const void *header_data, size_t header_len)
{
	if (header_len < 10)
		return NErr_NeedMoreData;

	ID3v2::Header header(header_data);
	if (!header.Valid())
		return NErr_Error;

	nsid3v2_header_t new_header = (nsid3v2_header_t)new (std::nothrow) ID3v2::Header(header);
	if (!new_header)
		return NErr_OutOfMemory;
	*h = new_header;
	return NErr_Success;
}

int NSID3v2_Header_New(nsid3v2_header_t *h, uint8_t version, uint8_t revision)
{
	nsid3v2_header_t new_header = (nsid3v2_header_t)new (std::nothrow) ID3v2::Header(version, revision);
	if (!new_header)
		return NErr_OutOfMemory;
	*h = new_header;
	return NErr_Success;
}

int NSID3v2_Header_FooterCreate(nsid3v2_header_t *f, const void *footer_data, size_t footer_len)
{
	if (footer_len < 10)
		return NErr_NeedMoreData;

	ID3v2::Header footer(footer_data);
	if (!footer.FooterValid())
		return NErr_Error;

	nsid3v2_header_t new_header = (nsid3v2_header_t)new (std::nothrow) ID3v2::Header(footer);
	if (!new_header)
		return NErr_OutOfMemory;
	*f = new_header;
	return NErr_Success;
}

int NSID3v2_Header_TagSize(const nsid3v2_header_t h, uint32_t *tag_size)
{
	const ID3v2::Header *header = (const ID3v2::Header *)h;
	if (!header)
		return NErr_NullPointer;

	*tag_size = header->TagSize();
	return NErr_Success;
}

int NSID3v2_Header_HasFooter(const nsid3v2_header_t h)
{
	const ID3v2::Header *header = (const ID3v2::Header *)h;
	if (!header)
		return NErr_NullPointer;

	if (header->HasFooter())
		return NErr_Success;
	else
		return NErr_False;
}

int NSID3v2_Header_Destroy(nsid3v2_header_t h)
{
	const ID3v2::Header *header = (const ID3v2::Header *)h;
	if (!header)
		return NErr_NullPointer;

	delete header;
	return NErr_Success;
}

/*
================== Tag ================== 
=                                       =
========================================= 
*/

int NSID3v2_Tag_Create(nsid3v2_tag_t *t, const nsid3v2_header_t h, const void *bytes, size_t bytes_len)
{
	const ID3v2::Header *header = (const ID3v2::Header *)h;
	if (!header)
		return NErr_NullPointer;

	switch(header->GetVersion())
	{
	case 2:
		{
			ID3v2_2::Tag *tag = new (std::nothrow) ID3v2_2::Tag(*header);
			if (!tag)
				return NErr_OutOfMemory;
			tag->Parse(bytes, bytes_len);
			*t = (nsid3v2_tag_t)tag;
			return NErr_Success;
		}
	case 3:
		{
			ID3v2_3::Tag *tag = new (std::nothrow) ID3v2_3::Tag(*header);
			if (!tag)
				return NErr_OutOfMemory;
			tag->Parse(bytes, bytes_len);
			*t = (nsid3v2_tag_t)tag;
			return NErr_Success;
		}
	case 4:
		{
			ID3v2_4::Tag *tag = new (std::nothrow) ID3v2_4::Tag(*header);
			if (!tag)
				return NErr_OutOfMemory;
			tag->Parse(bytes, bytes_len);
			*t = (nsid3v2_tag_t)tag;
			return NErr_Success;
		}
	default:
		return NErr_NotImplemented;
	}
}

int NSID3v2_Tag_Destroy(nsid3v2_tag_t t)
{
	ID3v2::Tag *tag = (ID3v2::Tag *)t;
	delete tag;
	return NErr_Success;
}

int NSID3v2_Tag_RemoveFrame(nsid3v2_tag_t t, nsid3v2_frame_t f)
{
	ID3v2::Tag *tag = (ID3v2::Tag *)t;
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	tag->RemoveFrame(frame);
	return NErr_Success;
}

int NSID3v2_Tag_CreateFrame(nsid3v2_tag_t t, int frame_enum, int flags, nsid3v2_frame_t *f)
{
	ID3v2::Tag *tag = (ID3v2::Tag *)t;
	*f = (nsid3v2_frame_t)tag->NewFrame(frame_enum, flags);
	return NErr_Success;
}

int NSID3v2_Tag_AddFrame(nsid3v2_tag_t t, nsid3v2_frame_t f)
{
	ID3v2::Tag *tag = (ID3v2::Tag *)t;
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	tag->AddFrame(frame);
	return NErr_Success;
}

int NSID3v2_Tag_GetFrame(const nsid3v2_tag_t t, int frame_enum, nsid3v2_frame_t *frame)
{
	ID3v2::Tag *tag = (ID3v2::Tag *)t;
	ID3v2::Frame *found_frame = tag->FindFirstFrame(frame_enum);
	if (found_frame)
	{
		*frame = (nsid3v2_frame_t)found_frame;
		return NErr_Success;
	}
	else
		return NErr_Empty;
}

int NSID3v2_Tag_GetNextFrame(const nsid3v2_tag_t t, const nsid3v2_frame_t f, nsid3v2_frame_t *frame)
{
	ID3v2::Tag *tag = (ID3v2::Tag *)t;
	ID3v2::Frame *start_frame = (ID3v2::Frame *)f;

	ID3v2::Frame *found_frame = tag->FindNextFrame(start_frame);
	if (found_frame)
	{
		*frame = (nsid3v2_frame_t)found_frame;
		return NErr_Success;
	}
	else
		return NErr_EndOfEnumeration;
}

int NSID3v2_Frame_Binary_Get(nsid3v2_frame_t f, const void **binary, size_t *length)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	if (!frame)
		return NErr_BadParameter;

	return frame->GetData(binary, length);
}

int NSID3v2_Tag_EnumerateFrame(const nsid3v2_tag_t t, nsid3v2_frame_t p, nsid3v2_frame_t *f)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	const ID3v2::Frame *frame = tag->EnumerateFrame((const ID3v2::Frame *)p);
	*f = (nsid3v2_frame_t)frame;
	if (frame)
	{
		return NErr_Success;
	}
	else
	{
		return NErr_Error;
	}
}

int NSID3v2_Tag_GetInformation(nsid3v2_tag_t t, uint8_t *version, uint8_t *revision, int *flags)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	if (!tag)
		return NErr_BadParameter;

	*version = tag->GetVersion();
	*revision = tag->GetRevision();

	int local_flags=0;
	if (tag->HasExtendedHeader())
		local_flags |= NSID3V2_TAGFLAG_EXTENDED_HEADER;

	if (tag->Unsynchronised())
		local_flags |= NSID3V2_TAGFLAG_UNSYNCHRONIZED;
	if (tag->HasFooter())
		local_flags |= NSID3V2_TAGFLAG_HASFOOTER;

	*flags = local_flags;

	return NErr_Success;
}

int NSID3v2_Tag_SerializedSize(nsid3v2_tag_t t, uint32_t *length, uint32_t padding_size, int flags)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	if (!tag)
		return NErr_BadParameter;

	return tag->SerializedSize(length, padding_size, flags);
}

int NSID3v2_Tag_Serialize(nsid3v2_tag_t t, void *data, uint32_t len, int flags)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	if (!tag)
		return NErr_BadParameter;

	return tag->Serialize(data, len, flags);
}
/*
================= Frame ================= 
=                                       =
========================================= 
*/

int NSID3v2_Frame_GetIdentifier(nsid3v2_frame_t f, const char **identifier)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	if (!frame)
		return NErr_BadParameter;

	*identifier = (const char *)frame->GetIdentifier();
	return NErr_Success;
}


int NSID3v2_Frame_GetInformation(nsid3v2_frame_t f, int *type, int *flags)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	if (!frame)
		return NErr_BadParameter;

	const char *identifier=0;
	int ret = NSID3v2_Frame_GetIdentifier(f, &identifier);
	if (ret != NErr_Success)
		return ret;

	/* TODO: make a method to get the version from the frame */

	/* ok this is a bit of hack job */
	if (!memcmp(identifier, "TXX", 4) || !memcmp(identifier, "TXXX", 4))
	{
		*type = NSID3V2_FRAMETYPE_USERTEXT;
	}
	else if (!memcmp(identifier, "COM", 4) || !memcmp(identifier, "COMM", 4))
	{
		*type = NSID3V2_FRAMETYPE_COMMENTS;
	}
	else if (identifier[0] == 'T') /* check for text */
	{
		*type = NSID3V2_FRAMETYPE_TEXT;
	}		
	else if (!memcmp(identifier, "WXX", 4) || !memcmp(identifier, "WXXX", 4))
	{
		*type = NSID3V2_FRAMETYPE_USERURL;
	}
	else if (identifier[0] == 'W') /* check for URL */
	{
		*type = NSID3V2_FRAMETYPE_URL;
	}		
	else if (!memcmp(identifier, "PRIV", 4))
	{
		*type = NSID3V2_FRAMETYPE_PRIVATE;
	}
	else if (!memcmp(identifier, "GEO", 4) || !memcmp(identifier, "GEOB", 4))
	{
		*type = NSID3V2_FRAMETYPE_OBJECT;
	}
	else if (!memcmp(identifier, "POP", 4) || !memcmp(identifier, "POPM", 4))
	{
		*type = NSID3V2_FRAMETYPE_POPULARITY;
	}
	else if (!memcmp(identifier, "PIC", 4) || !memcmp(identifier, "APIC", 4))
	{
		*type = NSID3V2_FRAMETYPE_PICTURE;
	}	
	else if (!memcmp(identifier, "UFI", 4) || !memcmp(identifier, "UFID", 4))
	{
		*type = NSID3V2_FRAMETYPE_ID;
	}		
	else
	{
		*type = NSID3V2_FRAMETYPE_UNKNOWN;
	}

	if (flags)
	{
		int local_flags=0;
		if (frame->TagAlterPreservation())
			local_flags |= NSID3V2_FRAMEFLAG_TAG_ALTER_PRESERVATION;
		if (frame->FileAlterPreservation())
			local_flags |= NSID3V2_FRAMEFLAG_FILE_ALTER_PRESERVATION;
		if (frame->Encrypted())
			local_flags |= NSID3V2_FRAMEFLAG_ENCRYPTED;
		if (frame->Compressed())
			local_flags |= NSID3V2_FRAMEFLAG_COMPRESSED;
		if (frame->Grouped())
			local_flags |= NSID3V2_FRAMEFLAG_GROUPED;
		if (frame->ReadOnly())
			local_flags |= NSID3V2_FRAMEFLAG_READONLY;
		if (frame->FrameUnsynchronised())
			local_flags |= NSID3V2_FRAMEFLAG_UNSYNCHRONIZED;
		if (frame->DataLengthIndicated())
			local_flags |= NSID3V2_FRAMEFLAG_DATALENGTHINDICATED;
		*flags = local_flags;
	}

	return NErr_Success;
}

