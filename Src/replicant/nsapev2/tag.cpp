#include "tag.h"
#include "header.h"
#include "flags.h"
#ifdef _WIN32
#include "../nu/ns_wc.h"
#include "nu/strsafe.h"
#endif
#include <limits.h>

#include <new>
#include "nu/ByteWriter.h"

/*
http://wiki.hydrogenaudio.org/index.php?title=APEv2_specification
*/

APEv2::Tag::Tag()
{
	flags = 0; // default to writing just a footer
}

APEv2::Tag::~Tag()
{
	items.deleteAll();
}

/* Parsing */
int APEv2::Tag::Parse(const APEv2::Header *header, const void *_data, size_t len)
{
	flags = header->GetFlags();

	if (header->IsFooter())
	{
		flags &= ~FLAG_HEADER_NO_FOOTER; // winamp 5.54 had this flag reversed, so let's correct it
		if (header->HasHeader())
		{
			// TODO: validate header
			_data = (const uint8_t *)_data + 32;
			len -= 32;
		}
		len -= 32; /* footer is counted in the size */
		return ParseData(_data, len);
	}
	else /* IsHeader() */
	{
		if (header->HasFooter())
		{
			// TODO: validate footer
			len -= 32;
		}
		return ParseData(_data, len);
	}
}

int APEv2::Tag::ParseData(const void *data, size_t len)
{
	bytereader_s byte_reader;
	bytereader_init(&byte_reader, data, len);

	while (bytereader_size(&byte_reader))
	{
		Item *item = new (std::nothrow) Item;
		if (!item)
			return NErr_OutOfMemory;

		int ret = item->Read(&byte_reader);
		if (ret == NErr_Success)
		{
			items.push_back(item);
		}
		else
		{
			delete item;
			return ret;
		}
	}
	return NErr_Success;
}

/* Retrieving Data */
int APEv2::Tag::GetItem(const char *key, unsigned int index, Item **item, int compare) const
{
	unsigned int i=0;
	for (ItemList::iterator itr=items.begin();itr!=items.end();itr++)
	{
		/* check if it's a string first, and then match the key next (will be faster) */
		if (itr->KeyMatch(key, compare))
		{
			if (i++ < index)
				continue;
	
			*item = *itr;
			return NErr_Success;
		}
	}

	if (i > index) /* if we found the key once, but the index was too high */
		return NErr_EndOfEnumeration;
	return NErr_Empty;
}

int APEv2::Tag::GetItemAtIndex(unsigned int index, Item **item) const
{
	unsigned int i=0;
	for (ItemList::iterator itr=items.begin();itr!=items.end();itr++)
	{
		if (i++ < index)
			continue;

		*item = *itr;
		return NErr_Success;
	}

	return NErr_EndOfEnumeration;
}

int APEv2::Tag::GetData(const char *key, unsigned int index, const void **data, size_t *data_len, int compare) const
{
	Item *item=0;
	int ret = GetItem(key, index, &item, compare);
	if (ret != NErr_Success)
		return ret;

	return item->Get(data, data_len);
}

int APEv2::Tag::EnumerateItems(const Item *start, Item **item) const
{
	Item *next_item = 0;
	if (!start)
	{
		next_item = items.front();
	}
	else
	{
		next_item = static_cast<APEv2::Item *>(start->next);
	}
	*item = next_item;
	if (next_item)
		return NErr_Success;
	else if (start)
			return NErr_EndOfEnumeration;
	else
		return NErr_Empty;
}

int APEv2::Tag::FindItemByKey(const char *key, Item **item, int compare) const
{
	for (ItemList::iterator itr=items.begin();itr!=items.end();itr++)
	{
		if (itr->KeyMatch(key, compare))
		{
			*item = *itr;
			return NErr_Success;
		}
	}
	return NErr_Unknown;
}

bool APEv2::Tag::IsReadOnly() const
{
	return flags & FLAG_READONLY;
}

int APEv2::Tag::GetItemCount(size_t *count) const
{
	*count = items.size();
	return NErr_Success;
}

int APEv2::Tag::GetFlags(uint32_t *flags) const
{
	*flags = this->flags;
	return NErr_Success;
}

/* Setting Data */
int APEv2::Tag::AddItem(APEv2::Item *new_item)
{
	items.push_back(new_item);
	return NErr_Success;
}

int APEv2::Tag::SetFlags(uint32_t newflags, uint32_t mask)
{
  flags = (flags & ~mask) | newflags;
	return NErr_Success;
}

/* Removing Data */
void APEv2::Tag::Clear()
{
	items.deleteAll();
}

void APEv2::Tag::Remove(const char *key, unsigned int starting_index, int compare)
{
	for (ItemList::iterator itr=items.begin();itr!=items.end();)
	{
		ItemList::iterator next = itr;
		next++;
		APEv2::Item *item = *itr;

		if (item->KeyMatch(key, compare))
		{
			if (starting_index)
			{
				starting_index--;
			}
			else
			{
				items.erase(item);
				delete item;				
			}
		}
		itr=next;
	}
}

void APEv2::Tag::RemoveItem(Item *item)
{
	items.erase(item);
	delete item;
}

/* Serializing */
size_t APEv2::Tag::EncodeSize() const
{
	size_t total_size=0;

	if (flags & FLAG_HEADER_HAS_HEADER)
		total_size+=Header::SIZE;

	for (ItemList::iterator itr=items.begin();itr!=items.end();itr++)
	{
		total_size += itr->EncodeSize();
	}

	if (!(flags & FLAG_HEADER_NO_FOOTER))
		total_size+=Header::SIZE;

	return total_size;
}

int APEv2::Tag::Encode(void *data, size_t len) const
{
	bytewriter_s byte_writer;
	bytewriter_init(&byte_writer, data, len);

	if (flags & FLAG_HEADER_HAS_HEADER)
	{
		Header header;
		header.SetSize((uint32_t)len - Header::SIZE);
		header.SetItems((uint32_t)items.size());
		header.SetFlags((flags & FLAG_HEADER_ENCODE_MASK)|FLAG_HEADER_IS_HEADER);

		int ret = header.Encode(&byte_writer);
		if (ret != NErr_Success)
			return ret;
	}

	for (ItemList::iterator itr=items.begin();itr!=items.end();itr++)
	{
		int ret = itr->Encode(&byte_writer);
		if (ret!= NErr_Success)
			return ret;
	}

	if (!(flags & FLAG_HEADER_NO_FOOTER))
	{
		Header footer;
		
		if (flags & FLAG_HEADER_HAS_HEADER)
			footer.SetSize((uint32_t)len - Header::SIZE);
		else
			footer.SetSize((uint32_t)len);
		footer.SetItems((uint32_t)items.size());
		footer.SetFlags((flags & FLAG_HEADER_ENCODE_MASK));

		int ret = footer.Encode(&byte_writer);
		if (ret != NErr_Success)
			return ret;
	}
	return NErr_Success;
}
