#ifndef NULLSOFT_IN_MP3_APEV2_H
#define NULLSOFT_IN_MP3_APEV2_H

#include "../apev2/tag.h"

class APE : private APEv2::Tag
{
public:
	APE();
	bool HasData() { return hasData; }
	bool IsDirty() { return dirty; }
	void ResetDirty() { dirty=0; hasData = true; }
	void Clear();
	void MarkClear();
	int Decode(const void *data, size_t len);

	// return -1 for empty, 1 for OK, 0 for "don't understand tag name"
	int GetString(const char *tag, wchar_t *data, int dataLen);
	int SetString(const char *tag, const wchar_t *data);

	int AddKeyValue(const char *key, const wchar_t *data);
	int SetKeyValueByIndex(size_t index, const char *key, const wchar_t *data);
	int RemoveItem(size_t index);

	int AddItem();
	

	static const char *MapApeKeyToWinampKey(const char *ape_key);
	static const wchar_t *MapApeKeyToWinampKeyW(const char *ape_key);
	static const char *MapWinampKeyToApeKey(const char *winamp_key);

	using APEv2::Tag::EnumValue;
	using APEv2::Tag::EncodeSize;
	using APEv2::Tag::Encode;
	using APEv2::Tag::FindItemByKey;
	using APEv2::Tag::GetNumItems;
	using APEv2::Tag::IsItemReadOnly;
	using APEv2::Tag::SetFlags;

private:
	bool hasData;
	bool dirty;
};


#endif