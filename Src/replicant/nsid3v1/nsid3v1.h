#pragma once

#include "nx/nxstring.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NSID3V1_EXPORT
	typedef struct nsid3v1_tag_struct_t { } *nsid3v1_tag_t;


	
	// Basic methods
	NSID3V1_EXPORT int NSID3v1_Header_Valid(const void *data, size_t len);
	NSID3V1_EXPORT int NSID3v1_Tag_Create(const void *data, size_t len, nsid3v1_tag_t *out_tag);
	NSID3V1_EXPORT int NSID3v1_Tag_New(nsid3v1_tag_t *out_tag);
	// len must be >= 128.  ALWAYS writes 128 bytes if successful
	NSID3V1_EXPORT int NSID3v1_Tag_Serialize(const nsid3v1_tag_t t, void *data, size_t len);
	NSID3V1_EXPORT int NSID3v1_Tag_Destroy(nsid3v1_tag_t t);

	// Generic getters
	//NSID3V1_EXPORT int NSID3v1_Field_Text_Get(const nsid3v1_tag_t *t, const int field, nx_string_t *out_value);
	NSID3V1_EXPORT int NSID3v1_Field_Text_Get(const nsid3v1_tag_t t, const int field, nx_string_t *out_value);
	
	// Specific field getters as text
	NSID3V1_EXPORT int NSID3v1_Get_Title(const nsid3v1_tag_t t, nx_string_t *value);
	NSID3V1_EXPORT int NSID3v1_Get_Artist(const nsid3v1_tag_t t, nx_string_t *value);
	NSID3V1_EXPORT int NSID3v1_Get_Album(const nsid3v1_tag_t t, nx_string_t *value);
	NSID3V1_EXPORT int NSID3v1_Get_Year(const nsid3v1_tag_t t, nx_string_t *value);
	NSID3V1_EXPORT int NSID3v1_Get_Comment(const nsid3v1_tag_t t, nx_string_t *value);
	NSID3V1_EXPORT int NSID3v1_Get_Track(const nsid3v1_tag_t t, nx_string_t *value);

	// Specific field getters as integers
	NSID3V1_EXPORT int NSID3v1_Int_Get_Year(const nsid3v1_tag_t t, unsigned int *value);
	NSID3V1_EXPORT int NSID3v1_Int_Get_Track(const nsid3v1_tag_t t, uint8_t *value);
	NSID3V1_EXPORT int NSID3v1_Int_Get_Genre(const nsid3v1_tag_t t, uint8_t *value);

	// Specific field setters as text
	NSID3V1_EXPORT int NSID3v1_Set_Title(nsid3v1_tag_t t, nx_string_t value);
	NSID3V1_EXPORT int NSID3v1_Set_Artist(nsid3v1_tag_t t, nx_string_t value);
	NSID3V1_EXPORT int NSID3v1_Set_Album(nsid3v1_tag_t t, nx_string_t value);
	NSID3V1_EXPORT int NSID3v1_Set_Year(nsid3v1_tag_t t, nx_string_t value);
	NSID3V1_EXPORT int NSID3v1_Set_Comment(nsid3v1_tag_t t, nx_string_t value);
	NSID3V1_EXPORT int NSID3v1_Set_Track(nsid3v1_tag_t t, nx_string_t value);

	// Specific field setters as integers
	NSID3V1_EXPORT int NSID3v1_Int_Set_Year(nsid3v1_tag_t t, unsigned int value);
	NSID3V1_EXPORT int NSID3v1_Int_Set_Track(nsid3v1_tag_t t, uint8_t value);
	NSID3V1_EXPORT int NSID3v1_Int_Set_Genre(nsid3v1_tag_t t, uint8_t value);
	
	// field types for ID3V1.0
	enum
	{
		NSID3V1_TAG,
		NSID3V1_TITLE,
		NSID3V1_ARTIST,
		NSID3V1_ALBUM,
		NSID3V1_YEAR,
		NSID3V1_COMMENT,
		NSID3V1_TRACK,		// ID3V1.1
		NSID3V1_GENRE,
	};
	
#ifdef __cplusplus
}
#endif
