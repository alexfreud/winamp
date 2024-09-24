#include "item.h"
#include "flags.h"
#include "util.h"
#include <strsafe.h>
#include <stdint.h>

/*
http://wiki.hydrogenaudio.org/index.php?title=APE_Tag_Item
*/

APEv2::Item::Item()
{
	refCount=1;
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

void APEv2::Item::Retain()
{
	refCount++;
}

void APEv2::Item::Release()
{
	if (--refCount == 0)
		delete this;
}

int APEv2::Item::Read(void *_data, size_t datalen, void **new_data, size_t *new_len)
{
	char *data = (char *)_data;

	if (datalen < 4)
		return APEV2_TOO_SMALL;
	memcpy(&len, data, 4);
	len = ATON32(len);
	data+=4;
	datalen-=4;

	if (datalen < 4)
		return APEV2_TOO_SMALL;
	memcpy(&flags, data, 4);
	flags = ATON32(flags);
	data+=4;
	datalen-=4;

	uint32_t key_len=0;
	for (uint32_t i=0;i<datalen;i++)
	{
		if (data[i] == 0)
		{
			key_len=i;
			break;
		}
	}

	if (key_len == datalen)
		return APEV2_TOO_SMALL;

	if (key_len == 0)
		return APEV2_FAILURE;

	if (key)
	{
		free(key);
		key = 0;
	}
	key = (char *)calloc(key_len+1, sizeof(char));
	if (key)
	{
		StringCchCopyA(key, key_len+1, data);
		datalen-=(key_len+1);
		data+=(key_len+1);

		if (datalen < len)
		{
			free(key);
			key = 0;
			return APEV2_TOO_SMALL;
		}

		if (value)
		{
			free(value);
			value = 0;
		}
		value = (char *)calloc(len, sizeof(char));
		if (value)
		{
			memcpy(value, data, len);
			datalen-=len;
			data+=len;
			*new_len = datalen;
			*new_data=data;
			return APEV2_SUCCESS;
		}
		else
		{
			free(key);
			return APEV2_FAILURE;
		}
	}
	else
		return APEV2_FAILURE;
}

bool APEv2::Item::IsReadOnly()
{
	return flags & FLAG_READONLY;
}

bool APEv2::Item::IsString()
{
	return (flags & MASK_ITEM_TYPE) == FLAG_ITEM_TEXT;
}

bool APEv2::Item::KeyMatch(const char *key_to_compare, int compare)
{
	if (!key || !*key)
		return false;

	switch (compare)
	{
	case ITEM_KEY_COMPARE_CASE_INSENSITIVE:
		return !_stricmp(key_to_compare, key);
	case ITEM_KEY_COMPARE_CASE_SENSITIVE:
		return !strcmp(key_to_compare, key);
	default:
		return false;
	}
}

int APEv2::Item::Get(void **data, size_t *datalen)
{
	if (!value || !len)
		return APEV2_FAILURE;
	*data = value;
	*datalen = len;
	return APEV2_SUCCESS;
}

int APEv2::Item::Set(const void *data, size_t datalen, int dataType)
{
	if (!data || !datalen)
		return APEV2_FAILURE;

	// set data type for this item
	flags &= ~MASK_ITEM_TYPE;
	flags |= dataType;

	free(value);
	value = malloc(datalen);
	len=(uint32_t)datalen;
	memcpy(value, data, len);
	return APEV2_SUCCESS;
}

int APEv2::Item::SetKey(const char *tag)
{
	if (!tag || !*tag)
		return APEV2_FAILURE;

	free(key);
	key = _strdup(tag);
	return APEV2_SUCCESS;
}

int APEv2::Item::GetKey(const char **tag)
{
	if (!key)
		return APEV2_FAILURE;
	*tag = key;
	return APEV2_SUCCESS;
}

size_t APEv2::Item::EncodeSize()
{
	return 4 /* size */ + 4 /* flags */ + (key && *key ? strlen(key) : 0) + 1 /* NULL separator */ + len;
}

int APEv2::Item::Encode(void *data, size_t datalen)
{
	if (!key || !value || !len)
		return APEV2_FAILURE;

	if (datalen < EncodeSize())
		return APEV2_TOO_SMALL;

	int8_t *ptr = (int8_t *)data;

	// write data length
	int32_t _len = NTOA32(len);
	memcpy(ptr, &_len, sizeof(_len));
	ptr+=sizeof(_len);
	datalen-=sizeof(_len);

	// write flags
	int32_t _flags = NTOA32(flags);
	memcpy(ptr, &_flags, sizeof(_flags));
	ptr+=sizeof(_flags);
	datalen-=sizeof(_flags);

	// write key and null terminator
	if (StringCchCopyExA((char *)ptr, datalen, key, (char **) &ptr, &datalen, 0) != S_OK)
		return APEV2_FAILURE;
	// account for null separator
	ptr++;
	datalen--;

	// write data
	memcpy(ptr, value, len);
	return APEV2_SUCCESS;
}