#include "nsapev2.h"
#include "nsapev2/header.h"
#include "nsapev2/tag.h"
#include <new>

int NSAPEv2_Header_Valid(const void *header_data)

{
	if (APEv2::Header(header_data).Valid())
		return NErr_Success;
	else
		return NErr_False;
}

int NSAPEv2_Header_Create(nsapev2_header_t *header, const void *header_data, size_t header_len)
{
	if (header_len < 10 || NSAPEv2_Header_Valid(header_data) != NErr_Success)
		return NErr_Error;

	nsapev2_header_t new_header = (nsapev2_header_t)new (std::nothrow) APEv2::Header(header_data);
	if (!new_header)
		return NErr_OutOfMemory;
	*header = new_header;
	return NErr_Success;
}

int NSAPEv2_Header_TagSize(const nsapev2_header_t h, uint32_t *tag_size)
{
	const APEv2::Header *header = (const APEv2::Header *)h;
	if (!header)
		return NErr_NullPointer;

	*tag_size = header->TagSize();
	return NErr_Success;
}

int NSAPEv2_Header_HasHeader(nsapev2_header_t h)
{
	const APEv2::Header *header = (const APEv2::Header *)h;
	if (!header)
		return NErr_NullPointer;

	if (header->HasHeader())
		return NErr_Success;
	else
		return NErr_False;
}

int NSAPEv2_Header_Destroy(nsapev2_header_t h)
{		 
	const APEv2::Header *header = (const APEv2::Header *)h;
	if (!header)
		return NErr_NullPointer;

	delete header;
	return NErr_Success;
}

int NSAPEv2_Tag_Create(nsapev2_tag_t *t, const nsapev2_header_t h, const void *bytes, size_t bytes_len)
{
	const APEv2::Header *header = (const APEv2::Header *)h;
	if (!header)
		return NErr_NullPointer;

	APEv2::Tag *tag = new (std::nothrow) APEv2::Tag;
	if (!tag)
		return NErr_OutOfMemory;

	int ret = tag->Parse(header, bytes, bytes_len);
	if (ret != NErr_Success)
	{
		delete tag;
		return ret;
	}

	*t = (nsapev2_tag_t)tag;
	return ret;
}

int NSAPEv2_Tag_New(nsapev2_tag_t *t)
{
	APEv2::Header *new_header = new (std::nothrow) APEv2::Header();
	if (!new_header)
		return NErr_OutOfMemory;

	APEv2::Tag *tag = new (std::nothrow) APEv2::Tag;
	if (!tag)
	{
		delete new_header;
		return NErr_OutOfMemory;
	}

	*t = (nsapev2_tag_t)tag;
	return NErr_Success;
}

int NSAPEv2_Tag_SerializedSize(const nsapev2_tag_t t, size_t *bytes)
{	
	const APEv2::Tag *tag = (const APEv2::Tag *)t;
	if (!tag)
		return NErr_BadParameter;

	*bytes = tag->EncodeSize();
	return NErr_Success;
}

int NSAPEv2_Tag_Serialize(const nsapev2_tag_t t, void *data, size_t bytes)
{	
	const APEv2::Tag *tag = (const APEv2::Tag *)t;
	if (!tag)
		return NErr_BadParameter;

	return tag->Encode(data, bytes);	
}

int NSAPEv2_Tag_Destroy(nsapev2_tag_t t)
{
	APEv2::Tag *tag = (APEv2::Tag *)t;
	if (!tag)
		return NErr_NullPointer;
	delete tag;
	return NErr_Success;
}

int NSAPEv2_Tag_GetFlags(nsapev2_tag_t t, uint32_t *flags)
{
	APEv2::Tag *tag = (APEv2::Tag *)t;
	if (!tag)
		return NErr_BadParameter;

	return tag->GetFlags(flags);
}

int NSAPEv2_Tag_SetFlags(nsapev2_tag_t t, uint32_t flags, uint32_t mask)
{
	APEv2::Tag *tag = (APEv2::Tag *)t;
	if (!tag)
		return NErr_BadParameter;

	return tag->SetFlags(flags, mask);
}

int NSAPEv2_Tag_GetString(const nsapev2_tag_t t, const char *key, unsigned int index, nx_string_t *value)
{
	const APEv2::Tag *tag = (const APEv2::Tag *)t;
	if (!tag)
		return NErr_Empty;

	const void *data;
	size_t data_len;
	int ret = tag->GetData(key, index, &data, &data_len);
	if (ret != NErr_Success)
		return ret;

	return NXStringCreateWithBytes(value, data, data_len, nx_charset_utf8);
}

int NSAPEv2_Tag_SetString(nsapev2_tag_t t, const char *key, unsigned int index, nx_string_t value)
{
	APEv2::Tag *tag = (APEv2::Tag *)t;
	if (!tag)
		return NErr_BadParameter;

	if (value)
	{
		APEv2::Item *item;
		int ret = tag->GetItem(key, index, &item);
		if (ret == NErr_Success)
		{
			return item->Set(value);
		}
		else
		{
			/* no such item, need to make a new one */
			item = new (std::nothrow) APEv2::Item;
			if (!item)
				return NErr_OutOfMemory;

			ret = item->SetKey(key);
			if (ret != NErr_Success)
			{
				delete item;
				return ret;
			}

			ret = item->Set(value);
			if (ret != NErr_Success)
			{
				delete item;
				return ret;
			}

			ret = tag->AddItem(item);
			if (ret != NErr_Success)
			{
				delete item;
				return ret;
			}

			return NErr_Success;
		}
	}
	else
	{
		tag->Remove(key, index);
		return NErr_Success;
	}
}

int NSAPEv2_Tag_GetBinary(const nsapev2_tag_t t, const char *key, unsigned int index, const void **bytes, size_t *length)
{
	const APEv2::Tag *tag = (const APEv2::Tag *)t;
	if (!tag)
		return NErr_Empty;

	return tag->GetData(key, index, bytes, length);
}

int NSAPEv2_Tag_SetBinary(nsapev2_tag_t t, const char *key, unsigned int index, const void *bytes, size_t length)
{
	APEv2::Tag *tag = (APEv2::Tag *)t;
	if (!tag)
		return NErr_BadParameter;

	if (bytes)
	{
		APEv2::Item *item;
		int ret = tag->GetItem(key, index, &item);
		if (ret == NErr_Success)
		{
			return item->Set(bytes, length, nsapev2_item_type_binary);
		}
		else
		{
			/* no such item, need to make a new one */
			item = new (std::nothrow) APEv2::Item;
			if (!item)
				return NErr_OutOfMemory;

			ret = item->SetKey(key);
			if (ret != NErr_Success)
			{
				delete item;
				return ret;
			}

			ret = item->Set(bytes, length, nsapev2_item_type_binary);
			if (ret != NErr_Success)
			{
				delete item;
				return ret;
			}

			ret = tag->AddItem(item);
			if (ret != NErr_Success)
			{
				delete item;
				return ret;
			}

			return NErr_Success;
		}
	}
	else
	{
		tag->Remove(key, index);
		return NErr_Success;
	}
}

static int SetArtwork(APEv2::Item *item, const char *filename, const void *bytes, size_t length)
{

	size_t filename_length = filename?strlen(filename):0;
	size_t total_length = filename_length 
		+ 1 /* null separator */
		+ length;

	void *item_data;
	int ret = item->New(total_length, nsapev2_item_type_binary, &item_data);
	if (ret != NErr_Success)
		return ret;

	bytewriter_s byte_writer;
	bytewriter_init(&byte_writer, item_data, total_length);
	bytewriter_write_n(&byte_writer, filename, filename_length);
	bytewriter_write_u8(&byte_writer, 0);
	bytewriter_write_n(&byte_writer, bytes, length);

	return NErr_Success;
}

int NSAPEv2_Tag_SetArtwork(nsapev2_tag_t t, const char *key, unsigned int index, const char *filename, const void *bytes, size_t length)
{
	APEv2::Tag *tag = (APEv2::Tag *)t;
	if (!tag)
		return NErr_BadParameter;

	if (bytes)
	{

		APEv2::Item *item;
		int ret = tag->GetItem(key, index, &item);
		if (ret == NErr_Success)
		{
			return SetArtwork(item, filename, bytes, length);
		}
		else
		{
			/* no such item, need to make a new one */
			item = new (std::nothrow) APEv2::Item;
			if (!item)
				return NErr_OutOfMemory;

			ret = item->SetKey(key);
			if (ret != NErr_Success)
			{
				delete item;
				return ret;
			}

			ret = SetArtwork(item, filename, bytes, length);
			if (ret != NErr_Success)
			{
				delete item;
				return ret;
			}

			ret = tag->AddItem(item);
			if (ret != NErr_Success)
			{
				delete item;
				return ret;
			}

			return NErr_Success;
		}
	}
	else
	{
		tag->Remove(key, index);
		return NErr_Success;
	}
}

int NSAPEv2_Tag_EnumerateItems(const nsapev2_tag_t t, const nsapev2_item_t s, nsapev2_item_t *next)
{
	const APEv2::Tag *tag = (const APEv2::Tag *)t;
	if (!tag)
		return NErr_NullPointer;

	const APEv2::Item *start = (const APEv2::Item *)s;
	return tag->EnumerateItems(start, (APEv2::Item **)next);
}

int NSAPEv2_Tag_GetItemCount(const nsapev2_tag_t t, size_t *count)
{
	const APEv2::Tag *tag = (const APEv2::Tag *)t;
	if (!tag)
		return NErr_NullPointer;

	return tag->GetItemCount(count);
}


int NSAPEv2_Tag_GetItemAtIndex(const nsapev2_tag_t t, unsigned int index, nsapev2_item_t *item)
{
	APEv2::Tag *tag = (APEv2::Tag *)t;
	if (!tag)
		return NErr_NullPointer;

	return tag->GetItemAtIndex(index, (APEv2::Item **)item);
}

int NSAPEv2_Item_GetInformation(const nsapev2_item_t i, const char **key, uint32_t *flags)
{
	const APEv2::Item *item = (const APEv2::Item *)i;
	if (!item)
		return NErr_NullPointer;

	if (key)
	{
		int ret = item->GetKey(key);
		if (ret != NErr_Success)
			return ret;
	}

	if (flags)
		*flags = item->GetFlags();
	return NErr_Success;

}

int NSAPEv2_Item_GetString(const nsapev2_item_t i, nx_string_t *value)
{
	const APEv2::Item *item = (const APEv2::Item *)i;
	if (!item)
		return NErr_BadParameter;

	const void *data;
	size_t data_len;
	int ret = item->Get(&data, &data_len);
	if (ret != NErr_Success)
		return ret;

	return NXStringCreateWithBytes(value, data, data_len, nx_charset_utf8);
}

int NSAPEv2_Item_GetBinary(const nsapev2_item_t i, const void **bytes, size_t *length)
{
	const APEv2::Item *item = (const APEv2::Item *)i;
	if (!item)
		return NErr_BadParameter;

	return item->Get(bytes, length);
}

int NSAPEv2_Item_SetKey(nsapev2_item_t i, const char *key)
{
	APEv2::Item *item = (APEv2::Item *)i;
	if (!item)
		return NErr_BadParameter;

	return item->SetKey(key);
}

int NSAPEv2_Item_SetString(nsapev2_item_t i, nx_string_t value)
{
	APEv2::Item *item = (APEv2::Item *)i;
	if (!item)
		return NErr_BadParameter;

	return item->Set(value);
}

int NSAPEv2_Tag_RemoveItem(nsapev2_tag_t t, nsapev2_item_t i)
{
	APEv2::Tag *tag = (APEv2::Tag *)t;
	if (!tag)
		return NErr_BadParameter;

	APEv2::Item *item = (APEv2::Item *)i;
	if (!item)
		return NErr_BadParameter;

	tag->RemoveItem(item);
	return NErr_Success;
}

int NSAPEv2_Tag_AddItem(nsapev2_tag_t t, nsapev2_item_t *i)
{
	APEv2::Tag *tag = (APEv2::Tag *)t;
	if (!tag)
		return NErr_BadParameter;

	APEv2::Item **item = (APEv2::Item **)i;
	if (!item)
		return NErr_NullPointer;

	*item = new (std::nothrow) APEv2::Item;
	if (!*item)
		return NErr_OutOfMemory;

	return tag->AddItem(*item);
}