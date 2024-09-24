#ifndef NULLSOFT_APEV2_TAG_H
#define NULLSOFT_APEV2_TAG_H

#include <bfc/platform/types.h>
#include <vector>
#include "item.h"
#include "flags.h"

namespace APEv2
{
class Tag
{
public:
	Tag();

	/* Parsing */
	int Parse(const void *data, size_t len);

	/* Retrieving Data */
	int GetString(const char *tag, wchar_t *data, size_t dataLen);
	int EnumValue(size_t i, const char **tag, wchar_t *data, size_t dataLen);
	int FindItemByKey(const char *key, size_t *index, int compare=ITEM_KEY_COMPARE_CASE_INSENSITIVE);
	size_t GetNumItems();
	bool IsReadOnly();
	bool IsItemReadOnly(size_t index);

	/* Setting Data */
	Item *AddItem();
	int SetString(const char *tag, const wchar_t *data);
	int SetKeyValueByIndex(size_t index, const char *key, const wchar_t *data);
	int SetFlags(uint32_t flags, uint32_t mask);

	/* Removing Data */
	void Clear();
	int RemoveItem(size_t index);
	int RemoveItem(const char *tag);

	/* Serializing */
	size_t EncodeSize();
	int Encode(void *data, size_t len);
	

private: /* methods */
	int ParseData(const void *data, size_t len); /* helper function, call with data pointing to beginning of items block (skip header), and length without footer. */

protected:
	int SetItemData(APEv2::Item *item, const wchar_t *data);

private: /* data */
	std::vector<APEv2::Item*> items;
	uint32_t flags;
};
}

#endif
