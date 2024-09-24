#include "tag.h"
#include "header.h"
#include "flags.h"
#include "nu/ns_wc.h"
#include <limits.h>
#include <strsafe.h>
#include <stdint.h>

/*
http://wiki.hydrogenaudio.org/index.php?title=APEv2_specification
*/

APEv2::Tag::Tag()
{
	flags = 0; // default to writing just a footer
}

int APEv2::Tag::Parse(const void *_data, size_t len)
{
	char *data = (char *)_data;

	if (len < Header::SIZE)
		return APEV2_TOO_SMALL;

	char *headerStart = data+(len-Header::SIZE);

	Header footer(headerStart);
	if (footer.Valid()
		&& !(len == Header::SIZE && footer.IsHeader())) // if all we have is this footer, we should check that it's not really a header
	{
		len -= Header::SIZE;
		char *dataStart = data;
		if (footer.HasHeader())
		{
			// TODO: validate header
			dataStart+= Header::SIZE;
			len-=Header::SIZE;
		}
		flags = footer.GetFlags();
		flags &= ~FLAG_HEADER_NO_FOOTER; // winamp 5.54 had this flag reversed, so let's correct it
		return ParseData(dataStart, len);
	}

	// ok, we didn't have a footer, so let's see if we have a header
	headerStart = data;
	Header header(headerStart);
	if (header.Valid())
	{
		len -= Header::SIZE;
		char *dataStart = data + Header::SIZE;
		if (header.HasFooter())
		{
			// TODO: validate footer
			// benski> cut... we got here because we didn't have a footer.
			//len-=Header::SIZE;
		}
		flags = header.GetFlags();
		flags |= FLAG_HEADER_NO_FOOTER; // winamp 5.54 had this flag reversed, so let's correct it
		return ParseData(dataStart, len);
	}

	return APEV2_FAILURE;
}

int APEv2::Tag::ParseData(const void *data, size_t len)
{
	char *dataStart = (char *)data;
	while (len)
	{
		Item *item = new Item;
		size_t new_len = 0;
		void *new_data = 0;
		int ret = item->Read(dataStart, len, &new_data, &new_len);
		if (ret == APEV2_SUCCESS)
		{
			len = new_len;
			dataStart = (char *)new_data;
			items.push_back(item);
		}
		else
		{
			item->Release();
			return ret;
		}
	}
	return APEV2_SUCCESS;
}

int APEv2::Tag::GetString(const char *tag, wchar_t *data, size_t dataLen)
{
	/* our UTF-8 to wchar_t conversion function wants an int for size, so we should explicitly check the size */
	if (dataLen > INT_MAX)
		return APEV2_FAILURE;

	for ( APEv2::Item *l_item : items )
	{
		/* check if it's a string first, and then match the key next (will be faster) */
		if ( l_item->IsString() && l_item->KeyMatch(tag, ITEM_KEY_COMPARE_CASE_INSENSITIVE))
		{
			void *item_data = 0;
			size_t item_dataLen = 0;
			if ( l_item->Get(&item_data, &item_dataLen) == APEV2_SUCCESS && item_dataLen < INT_MAX)
			{
				int signed_len = static_cast<int>(item_dataLen); // we checked against INT_MAX above so this conversion is safe
				if (MultiByteToWideCharSZ(CP_UTF8, 0, (LPCSTR)item_data, signed_len, data, (int)dataLen) != 0)
					return APEV2_SUCCESS;
			}
			return APEV2_NO_DATA;
		}
	}

	return APEV2_KEY_NOT_FOUND;
}

int APEv2::Tag::SetItemData(APEv2::Item *item, const wchar_t *data)
{
	int utf16_data_cch = (int)wcslen(data);
	int item_dataLen = WideCharToMultiByte(CP_UTF8, 0, data, utf16_data_cch, 0, 0, NULL, NULL);
	if (!item_dataLen)
		return APEV2_FAILURE;

	char *item_data = (char *)calloc(item_dataLen, sizeof(char));
	if (item_data)
	{
		if (!WideCharToMultiByte(CP_UTF8, 0, data, utf16_data_cch, item_data, item_dataLen, NULL, NULL))
		{
			free(item_data);
			return APEV2_FAILURE;
		}

		item->Set(item_data, item_dataLen, FLAG_ITEM_TEXT);
		free(item_data);
		return APEV2_SUCCESS;
	}
	return APEV2_FAILURE;
}

int APEv2::Tag::SetString(const char *tag, const wchar_t *data)
{
	for (auto it = items.begin(); it != items.end(); it++)
	{
		APEv2::Item* item = *it;
		if (item->KeyMatch(tag, ITEM_KEY_COMPARE_CASE_INSENSITIVE))
		{
			if (data && *data)
			{
				return SetItemData(item, data);
			}
			else
			{
				item->Release();
				it = items.erase(it);
				return APEV2_SUCCESS;
			}
		}
	}

	if (data && *data)
	{
		/* Not already in the list, so we need to add it */
		Item *newItem = new Item;

		if (newItem->SetKey(tag) == APEV2_SUCCESS && SetItemData(newItem, data) == APEV2_SUCCESS)
		{
			items.push_back(newItem);
		}
		else
		{
			newItem->Release();
			return APEV2_FAILURE;
		}
	}

	return APEV2_SUCCESS;
}

int APEv2::Tag::EnumValue(size_t i, const char **tag, wchar_t *data, size_t dataLen)
{
	/* our UTF-8 to wchar_t conversion function wants an int for size, so we should explicitly check the size */
	if (dataLen > INT_MAX)
		return APEV2_FAILURE;

	if (i >= items.size())
		return APEV2_END_OF_ITEMS;

	APEv2::Item *item = items[i];
	item->GetKey(tag);

	if (!data || !dataLen) // if they don't want the data, go ahead and return now
		return APEV2_SUCCESS;

	data[0]=0;
	void *item_data = 0;
	size_t item_dataLen = 0;
	if (item->IsString())
	{
		if (item->Get(&item_data, &item_dataLen) == APEV2_SUCCESS && item_dataLen < INT_MAX)
		{
			int signed_len = static_cast<int>(item_dataLen); // we checked against INT_MAX above so this conversion is safe
			if (MultiByteToWideCharSZ(CP_UTF8, 0, (LPCSTR)item_data, signed_len, data, (int)dataLen) == 0)
				*data=0;
		}
	}
	else
	{
		// TODO: benski> hmmm
		StringCchCopyW(data, dataLen, L"[TODO: deal with binary]");
	}

	return APEV2_SUCCESS;
}

void APEv2::Tag::Clear()
{
	for ( APEv2::Item *l_item : items )
	{
		l_item->Release();
	}

	items.clear();
}

int APEv2::Tag::RemoveItem(size_t index)
{
	if (index >= items.size())
		return APEV2_END_OF_ITEMS;

	APEv2::Item *item = items[index];
	items.erase(items.begin() + index);
	item->Release();
	return APEV2_SUCCESS;
}

size_t APEv2::Tag::EncodeSize()
{
	size_t totalSize=0;

	if (flags & FLAG_HEADER_HAS_HEADER)
		totalSize+=Header::SIZE;

	for ( APEv2::Item *l_item : items )
	{
		totalSize += l_item->EncodeSize();
	}

	if (!(flags & FLAG_HEADER_NO_FOOTER))
		totalSize+=Header::SIZE;

	return totalSize;
}

int APEv2::Tag::Encode(void *data, size_t len)
{
	size_t totalSize=0;
	int8_t *ptr = (int8_t *)data;

	if (flags & FLAG_HEADER_HAS_HEADER)
	{
		HeaderData headerData = {0};
		headerData.size= (uint32_t)totalSize;
		if (!(flags & FLAG_HEADER_NO_FOOTER))
			headerData.size += Header::SIZE;
		headerData.items = (uint32_t)items.size();

		headerData.flags = (flags & FLAG_HEADER_ENCODE_MASK)|FLAG_HEADER_IS_HEADER;

		Header header(&headerData);
		int ret = header.Encode(ptr, len);
		if (ret != APEV2_SUCCESS)
			return ret;
		ptr += Header::SIZE;
		len -= Header::SIZE;
	}

	for ( APEv2::Item *l_item : items )
	{
		int ret = l_item->Encode(ptr, len);
		if (ret!= APEV2_SUCCESS)
			return ret;
		size_t itemSize = l_item->EncodeSize();
		len-=itemSize;
		ptr+=itemSize;
		totalSize+=itemSize;
	}

	if (!(flags & FLAG_HEADER_NO_FOOTER))
	{
		HeaderData footerData = {0};
		footerData.size= (uint32_t)totalSize + Header::SIZE;
		footerData.items = (uint32_t)items.size();
		footerData.flags = (flags & FLAG_HEADER_ENCODE_MASK);

		Header footer(&footerData);
		int ret = footer.Encode(ptr, len);
		if (ret != APEV2_SUCCESS)
			return ret;
	}
	return APEV2_SUCCESS;
}

int APEv2::Tag::SetKeyValueByIndex(size_t index, const char *key, const wchar_t *data)
{
	if (index >= items.size())
		return APEV2_END_OF_ITEMS;

	// TODO: check for duplicate key

	items[index]->SetKey(key);
	return SetItemData(items[index], data);
}

APEv2::Item *APEv2::Tag::AddItem()
{
	Item *newItem = new Item;
	items.push_back(newItem);
	return newItem;
}

int APEv2::Tag::SetFlags(uint32_t newflags, uint32_t mask)
{
	flags = (flags & ~mask) | newflags;
	return APEV2_SUCCESS;
}

int APEv2::Tag::FindItemByKey(const char *key, size_t *index, int compare)
{
	for (size_t i=0;i!=items.size();i++)
	{
		if (items[i]->KeyMatch(key, compare))
		{
			*index = i;
			return APEV2_SUCCESS;
		}
	}
	return APEV2_KEY_NOT_FOUND;
}

size_t APEv2::Tag::GetNumItems()
{
	return items.size();
}

bool APEv2::Tag::IsReadOnly()
{
	return flags & FLAG_READONLY;
}

bool APEv2::Tag::IsItemReadOnly(size_t index)
{
	if (index >= items.size())
		return true;
	return items[index]->IsReadOnly();
}