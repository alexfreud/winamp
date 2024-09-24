#include "apev2.h"

APE::APE()
{
	hasData=false;
	dirty=false;
}

int APE::Decode(const void *data, size_t len)
{
	if (APEv2::Tag::Parse(data, len) == APEv2::APEV2_SUCCESS)
	{
		hasData=true;
		return 1;
	}

	return 0;
}

// return -1 for empty, 1 for OK, 0 for "don't understand metadata name"
int APE::GetString(const char *metadata, wchar_t *data, int dataLen)
{
	if (!hasData)
		return 0;

	if (!_stricmp(metadata, "replaygain_track_gain")
	    || !_stricmp(metadata, "replaygain_track_peak")
	    || !_stricmp(metadata, "replaygain_album_gain")
	    || !_stricmp(metadata, "replaygain_album_peak"))
	{
		if (APEv2::Tag::GetString(metadata, data, dataLen) == APEv2::APEV2_SUCCESS)
			return 1;
		return -1;
	}
	else
	{
		const char *ape_key = MapWinampKeyToApeKey(metadata);
		if (ape_key)
		{
			if (APEv2::Tag::GetString(ape_key, data, dataLen) == APEv2::APEV2_SUCCESS)
				return 1;
			return -1;
		}
	}

	return 0;
}

int APE::SetString(const char *metadata, const wchar_t *data)
{
	if (!_stricmp(metadata, "replaygain_track_gain")
	    || !_stricmp(metadata, "replaygain_track_peak")
	    || !_stricmp(metadata, "replaygain_album_gain")
	    || !_stricmp(metadata, "replaygain_album_peak"))
	{
		APEv2::Tag::SetString(metadata, data);
		dirty=true;
		hasData=true;
		return 1;
	}
	else
	{
		const char *ape_key = MapWinampKeyToApeKey(metadata);
		if (ape_key)
		{
			APEv2::Tag::SetString(ape_key, data);
			dirty=true;
			hasData=true;
			return 1;
		}
	}
	return 0;
}

void APE::Clear()
{
	APEv2::Tag::Clear();
	dirty=true;
	hasData=false;
}

void APE::MarkClear()
{
	dirty=true;
	hasData=false;
}

int APE::SetKeyValueByIndex(size_t index, const char *key, const wchar_t *data)
{
	dirty=true;
	return APEv2::Tag::SetKeyValueByIndex(index, key, data);
}

int APE::RemoveItem(size_t index)
{
	dirty=true;
	return APEv2::Tag::RemoveItem(index);
}

int APE::AddItem()
{
	dirty=true;
	hasData=true;
	APEv2::Tag::AddItem();
	return APEv2::APEV2_SUCCESS;
}

struct ApeKeyMapping
{
	const char *ape_key;
	const char *winamp_key;
	const wchar_t *winamp_keyW;
};

static ApeKeyMapping apeKeyMapping[] =
{
	{ "Track", "track", L"track" },
	{"Album", "album", L"album" },
	{"Artist", "artist", L"artist" },
	{"Comment", "comment", L"comment" },
	{"Year", "year", L"year" },
	{"Genre", "genre", L"genre" },
	{"Title", "title", L"title"},
	{"Composer", "composer", L"composer"},
	{"Performer", "performer", L"performer"},
	{"Album artist", "albumartist", L"albumartist"},
};

const wchar_t *APE::MapApeKeyToWinampKeyW(const char *ape_key)
{
	size_t num_mappings = sizeof(apeKeyMapping)/sizeof(apeKeyMapping[0]);
	for (size_t i=0;i!=num_mappings;i++)
	{
		if (!_stricmp(ape_key, apeKeyMapping[i].ape_key))
			return apeKeyMapping[i].winamp_keyW;
	}
	return NULL;
}

const char *APE::MapApeKeyToWinampKey(const char *ape_key)
{
	size_t num_mappings = sizeof(apeKeyMapping)/sizeof(apeKeyMapping[0]);
	for (size_t i=0;i!=num_mappings;i++)
	{
		if (!_stricmp(ape_key, apeKeyMapping[i].ape_key))
			return apeKeyMapping[i].winamp_key;
	}
	return NULL;
}

const char *APE::MapWinampKeyToApeKey(const char *winamp_key)
{
	size_t num_mappings = sizeof(apeKeyMapping)/sizeof(apeKeyMapping[0]);
	for (size_t i=0;i!=num_mappings;i++)
	{
		if (!_stricmp(winamp_key, apeKeyMapping[i].winamp_key))
			return apeKeyMapping[i].ape_key;
	}
	return NULL;
}

int APE::AddKeyValue(const char *key, const wchar_t *data)
{
	dirty=true;
	hasData=true;
	APEv2::Item *newItem = APEv2::Tag::AddItem();
	newItem->SetKey(key);
	return APEv2::Tag::SetItemData(newItem, data);
}