#pragma once
#include "foundation/types.h"
#include "nu/PtrDeque.h"
#include "item.h"
#include "flags.h"
#include "header.h"

namespace APEv2
{
class Tag
{
public:
	Tag();
	~Tag();

	/* Parsing */
	int Parse(const APEv2::Header *header, const void *data, size_t len);

	/* Retrieving Data */
	int GetItem(const char *key, unsigned int index, Item **item, int compare=ITEM_KEY_COMPARE_CASE_INSENSITIVE) const;
	int GetItemAtIndex(unsigned int index, Item **) const;
	int GetData(const char *tag, unsigned int index, const void **data, size_t *data_len, int compare=ITEM_KEY_COMPARE_CASE_INSENSITIVE) const;
	int EnumerateItems(const Item *start, Item **item) const;
	int FindItemByKey(const char *key, Item **item, int compare=ITEM_KEY_COMPARE_CASE_INSENSITIVE) const;
	bool IsReadOnly() const;
	int GetItemCount(size_t *count) const;
	int GetFlags(uint32_t *flags) const;

	/* Setting Data */
	int AddItem(APEv2::Item *new_item);
	int SetFlags(uint32_t flags, uint32_t mask);

	/* Removing Data */
	void Clear();
	void Remove(const char *key, unsigned int starting_index, int compare=ITEM_KEY_COMPARE_CASE_INSENSITIVE); // removes all items matching a key, but skips the first 'starting_index' items
	void RemoveItem(Item *item);
	/* Serializing */
	size_t EncodeSize() const;
	int Encode(void *data, size_t len) const;	

private: /* methods */
	int ParseData(const void *data, size_t len); /* helper function, call with data pointing to beginning of items block (skip header), and length without footer. */

private: /* data */
	typedef nu::PtrDeque<Item> ItemList;
	ItemList items;
	uint32_t flags;
};
}

