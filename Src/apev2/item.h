#ifndef NULLSOFT_APEV2_ITEM_H
#define NULLSOFT_APEV2_ITEM_H

#include <bfc/platform/types.h>

namespace APEv2
{
enum
{
	ITEM_KEY_COMPARE_CASE_INSENSITIVE = 0,
	ITEM_KEY_COMPARE_CASE_SENSITIVE = 1,
};
class Item
{
public:
	Item();
	~Item();
	void Retain();
	void Release();

	/* If successful, puts incremented data pointer in new_data, and new data size remaining in new_len */
	int Read(void *data, size_t len, void **new_data, size_t *new_len);

	int Encode(void *data, size_t len);
	size_t EncodeSize();

	bool IsReadOnly();
	bool IsString();
	bool KeyMatch(const char *key, int compare=ITEM_KEY_COMPARE_CASE_INSENSITIVE);
	int Get(void **data, size_t *len);
	int GetKey(const char **tag);
	int Set(const void *data, size_t len, int dataType);
	int SetKey(const char *tag);

private:
	size_t refCount;
	uint32_t flags;
	char *key;
	void *value;
	uint32_t len;
};
}
#endif
