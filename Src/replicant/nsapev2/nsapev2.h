#pragma once

#include "foundation/types.h"
#include "nx/nxstring.h"

#ifdef __ANDROID__
#include "android/nsapev2.h"
#elif defined(_WIN32)
#include "windows/nsapev2.h"
#elif defined(__linux__)
#include "linux/nsapev2.h"
#elif defined (__APPLE__)
#include "osx/nsapev2.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct nsapev2_header_struct_t { } *nsapev2_header_t;
	typedef struct nsapev2_tag_struct_t { } *nsapev2_tag_t;
	typedef struct nsapev2_item_struct_t { } *nsapev2_item_t;

	// must be exactly 32 bytes
	NSAPEV2_EXPORT int NSAPEv2_Header_Valid(const void *header_data);
	NSAPEV2_EXPORT int NSAPEv2_Header_Create(nsapev2_header_t *header, const void *header_data, size_t header_len);
	NSAPEV2_EXPORT int NSAPEv2_Header_TagSize(const nsapev2_header_t header, uint32_t *tag_size);
	NSAPEV2_EXPORT int NSAPEv2_Header_HasHeader(nsapev2_header_t header);
	NSAPEV2_EXPORT int NSAPEv2_Header_Destroy(nsapev2_header_t header);


	// currently, this function makes a copy of any necessary data.  in the future, it would be better
	// to make another version of this function that "borrows" your data
	// if you can guarantee that the memory will outlive the nsapev2_tag_t object
	NSAPEV2_EXPORT int NSAPEv2_Tag_Create(nsapev2_tag_t *tag, const nsapev2_header_t header, const void *bytes, size_t bytes_len);
	NSAPEV2_EXPORT int NSAPEv2_Tag_New(nsapev2_tag_t *tag);
	NSAPEV2_EXPORT int NSAPEv2_Tag_SerializedSize(const nsapev2_tag_t tag, size_t *bytes);
	NSAPEV2_EXPORT int NSAPEv2_Tag_Serialize(const nsapev2_tag_t t, void *data, size_t bytes);
	NSAPEV2_EXPORT int NSAPEv2_Tag_Destroy(nsapev2_tag_t tag);

	NSAPEV2_EXPORT int NSAPEv2_Tag_GetFlags(nsapev2_tag_t tag, uint32_t *flags);
	NSAPEV2_EXPORT int NSAPEv2_Tag_SetFlags(nsapev2_tag_t tag, uint32_t flags, uint32_t mask);

	NSAPEV2_EXPORT int NSAPEv2_Tag_GetString(const nsapev2_tag_t tag, const char *key, unsigned int index, nx_string_t *value);
	NSAPEV2_EXPORT int NSAPEv2_Tag_SetString(nsapev2_tag_t tag, const char *key, unsigned int index, nx_string_t value);

	NSAPEV2_EXPORT int NSAPEv2_Tag_GetBinary(const nsapev2_tag_t tag, const char *key, unsigned int index, const void **bytes, size_t *length);
	NSAPEV2_EXPORT int NSAPEv2_Tag_SetBinary(nsapev2_tag_t tag, const char *key, unsigned int index, const void *bytes, size_t length);
	NSAPEV2_EXPORT int NSAPEv2_Tag_SetArtwork(nsapev2_tag_t t, const char *key, unsigned int index, const char *filename, const void *bytes, size_t length);
	/* Items */
	// flags
	static const uint32_t nsapev2_item_read_write = 0;
	static const uint32_t nsapev2_item_read_only = 1;

	static const uint32_t nsapev2_item_type_utf8 = 0;
	static const uint32_t nsapev2_item_type_binary = 1 << 1;
	static const uint32_t nsapev2_item_type_locator = 2 << 1;

	// mask flags with these to check the different fields
	static const uint32_t nsapev2_item_type_mask = (1 << 2) | (1 << 1);
	static const uint32_t nsapev2_item_read_mask = 1 << 0;

	NSAPEV2_EXPORT int NSAPEv2_Tag_EnumerateItems(const nsapev2_tag_t tag, const nsapev2_item_t start, nsapev2_item_t *next);
	NSAPEV2_EXPORT int NSAPEv2_Tag_GetItemCount(const nsapev2_tag_t t, size_t *count);
	NSAPEV2_EXPORT int NSAPEv2_Tag_GetItemAtIndex(const nsapev2_tag_t t, unsigned int index, nsapev2_item_t *item);
	NSAPEV2_EXPORT int NSAPEv2_Item_GetInformation(const nsapev2_item_t item, const char **key, uint32_t *flags);
	NSAPEV2_EXPORT int NSAPEv2_Item_GetString(const nsapev2_item_t item, nx_string_t *value);
	NSAPEV2_EXPORT int NSAPEv2_Item_GetBinary(const nsapev2_item_t item, const void **bytes, size_t *length);
	NSAPEV2_EXPORT int NSAPEv2_Item_SetKey(nsapev2_item_t item, const char *key);
	NSAPEV2_EXPORT int NSAPEv2_Item_SetString(nsapev2_item_t item, nx_string_t value);
	NSAPEV2_EXPORT int NSAPEv2_Tag_RemoveItem(nsapev2_tag_t tag, nsapev2_item_t item);
	NSAPEV2_EXPORT int NSAPEv2_Tag_AddItem(nsapev2_tag_t tag, nsapev2_item_t *item);
#ifdef __cplusplus
}
#endif
