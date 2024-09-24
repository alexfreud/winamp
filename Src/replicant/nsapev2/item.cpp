#include "item.h"
#include "flags.h"
#include "util.h"
#include "nu/ByteWriter.h"
#include "nu/strsafe.h"
#include "nu/ByteReader.h"
#include "nsapev2/nsapev2.h"
#include <stdlib.h>

/*
http://wiki.hydrogenaudio.org/index.php?title=APE_Tag_Item

Item layout:
[0-3] length of value field (little endian)
[4-7] flags (little endian)
[null terminated] key
[length] value
*/

APEv2::Item::Item()
{
	len=0;
	flags=0;
	key=0;
	value=0;
}

APEv2::Item::~Item()
{
	free(key);
	free(value);
}

int APEv2::Item::Read(bytereader_t byte_reader)
{
	if (bytereader_size(byte_reader) < 8)
		return NErr_NeedMoreData;

	/* read fixed-size fields */
	len = bytereader_read_u32_le(byte_reader);
	flags = bytereader_read_u32_le(byte_reader);

	/* find the null terminator */
	size_t key_len = bytereader_find_zero(byte_reader);

	/* make sure we didn't hit the end of our buffer */
	if (key_len == bytereader_size(byte_reader))
		return NErr_Insufficient;

	/* check for empty key and also check for integer overflow */
	if (key_len == 0 || key_len+1 == 0)
		return NErr_Error;

	key = (char *)malloc(key_len+1);
	if (key)
	{
		bytereader_read_n(byte_reader, key, key_len+1); /* read key and terminator*/

		if (bytereader_size(byte_reader) < len) /* make sure we have room for the value! */
		{
			free(key);
			key=0;	
			return NErr_NeedMoreData;
		}

		value = (char *)malloc(len);
		if (value)
		{
			bytereader_read_n(byte_reader, value, len); /* read value */
			return NErr_Success;
		}
		else
		{
			free(key);
			key=0;
			return NErr_OutOfMemory;
		}
	}
	else
		return NErr_OutOfMemory;

}

bool APEv2::Item::IsReadOnly()
{
	return flags & FLAG_READONLY;
}

bool APEv2::Item::KeyMatch(const char *key_to_compare, int compare)
{
	if (!key || !*key)
		return false;

	switch (compare)
	{
	case ITEM_KEY_COMPARE_CASE_INSENSITIVE:
#ifdef _WIN32
		return !_stricmp(key_to_compare, key);
#else
		return !strcasecmp(key_to_compare, key);
#endif
	case ITEM_KEY_COMPARE_CASE_SENSITIVE:
		return !strcmp(key_to_compare, key);
	default:
		return false;
	}
}

int APEv2::Item::Get(const void **data, size_t *datalen) const
{
	if (!value || !len)
		return NErr_Empty;
	*data = value;
	*datalen = len;
	return NErr_Success;
}

int APEv2::Item::Set(nx_string_t string)
{
	if (!value)
		return NErr_BadParameter;

	flags &= ~nsapev2_item_type_mask;
	flags |= nsapev2_item_type_utf8;

	size_t bytes;
	int ret = NXStringGetBytesSize(&bytes, string, nx_charset_utf8, 0);
	if (ret != NErr_DirectPointer && ret != NErr_Success)
		return ret;
	
	void *new_value = malloc(bytes);
	if (!new_value)
		return NErr_OutOfMemory;
	
	size_t bytes_copied;
	ret = NXStringGetBytes(&bytes_copied, string, new_value, bytes, nx_charset_utf8, 0);
	if (ret != NErr_Success)
	{
		free(new_value);
		return ret;
	}

	free(value);
	value=new_value;
	len=(uint32_t)bytes_copied;
	return NErr_Success;
}

int APEv2::Item::Set(const void *data, size_t datalen, int data_type)
{
	if (!data || !datalen)
		return NErr_Error;

	// set data type for this item
	flags &= ~nsapev2_item_type_mask;
	flags |= data_type;

	void *new_value = realloc(value, datalen);
	if (!new_value)
		return NErr_OutOfMemory;

	value=new_value;
	len=(uint32_t)datalen;
	memcpy(value, data, len);
	return NErr_Success;
}

	int APEv2::Item::New(size_t datalen, int data_type, void **bytes)
	{
		if (!datalen)
			return NErr_Error;

		// set data type for this item
		flags &= ~nsapev2_item_type_mask;
		flags |= data_type;

		void *new_value = realloc(value, datalen);
		if (!new_value)
			return NErr_OutOfMemory;

		value=new_value;

		len=(uint32_t)datalen;
		*bytes = value;
		return NErr_Success;
	}

int APEv2::Item::SetKey(const char *tag)
{
	if (!tag || !*tag)
		return NErr_Error;

	char *new_key = strdup(tag);
	if (!new_key)
		return NErr_OutOfMemory;

	free(key);
	key = new_key;
	return NErr_Success;
}

int APEv2::Item::GetKey(const char **tag) const
{
	if (!key)
		return NErr_Error;
	*tag = key;
	return NErr_Success;
}

size_t APEv2::Item::EncodeSize() const
{
	return 4 /* size */ + 4 /* flags */ + strlen(key) + 1 /* NULL separator */ + len;
}

uint32_t APEv2::Item::GetFlags() const
{
	return flags;
}

int APEv2::Item::Encode(bytewriter_t byte_writer) const
{
	if (!key || !value || !len)
		return NErr_Error;

	if (bytewriter_size(byte_writer) < EncodeSize())
		return NErr_Insufficient;

	bytewriter_write_u32_le(byte_writer, len);
	bytewriter_write_u32_le(byte_writer, flags);
	bytewriter_write_n(byte_writer, key, strlen(key) + 1);
	bytewriter_write_n(byte_writer, value, len);

	return NErr_Success;
}

