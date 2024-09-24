#pragma once

/* include any platform-specific stuff, first */
#ifdef __ANDROID__
#include "android/nsid3v2.h"
#elif defined(_WIN32)
#include "windows/nsid3v2.h"
#elif defined(__linux__)
#include "linux/nsid3v2.h"
#elif defined(__APPLE__)
#include "osx/nsid3v2.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
	// must be exactly 10 bytes
	NSID3V2_EXPORT int NSID3v2_Header_Valid(const void *header_data);
	NSID3V2_EXPORT int NSID3v2_Header_FooterValid(const void *footer_data);
	NSID3V2_EXPORT int NSID3v2_Header_Create(nsid3v2_header_t *header, const void *header_data, size_t header_len);
	NSID3V2_EXPORT int NSID3v2_Header_New(nsid3v2_header_t *h, uint8_t version, uint8_t revision);

	/* Creates from footer data instead of header data */
	NSID3V2_EXPORT int NSID3v2_Header_FooterCreate(nsid3v2_header_t *footer, const void *footer_data, size_t footer_len);
	NSID3V2_EXPORT int NSID3v2_Header_TagSize(const nsid3v2_header_t header, uint32_t *tag_size);
	NSID3V2_EXPORT int NSID3v2_Header_HasFooter(const nsid3v2_header_t header);
	NSID3V2_EXPORT int NSID3v2_Header_Destroy(nsid3v2_header_t header);

	// currently, this function makes a copy of any necessary data.  in the future, it would be better
	// to make another version of this function that "borrows" your data
	// if you can guarantee that the memory will outlive the nsid3v2_tag_t object
	NSID3V2_EXPORT int NSID3v2_Tag_Create(nsid3v2_tag_t *tag, const nsid3v2_header_t header, const void *bytes, size_t bytes_len);
	NSID3V2_EXPORT int NSID3v2_Tag_Destroy(nsid3v2_tag_t tag);

	NSID3V2_EXPORT int NSID3v2_Tag_GetFrame(const nsid3v2_tag_t tag, int frame_enum, nsid3v2_frame_t *frame);
	NSID3V2_EXPORT int NSID3v2_Tag_GetNextFrame(const nsid3v2_tag_t tag, const nsid3v2_frame_t start_frame, nsid3v2_frame_t *frame);
	NSID3V2_EXPORT int NSID3v2_Tag_RemoveFrame(nsid3v2_tag_t tag, nsid3v2_frame_t frame);
	NSID3V2_EXPORT int NSID3v2_Tag_CreateFrame(nsid3v2_tag_t tag, int frame_enum, int flags, nsid3v2_frame_t *frame);
	NSID3V2_EXPORT int NSID3v2_Tag_AddFrame(nsid3v2_tag_t tag, nsid3v2_frame_t frame);
	NSID3V2_EXPORT int NSID3v2_Tag_EnumerateFrame(const nsid3v2_tag_t tag, nsid3v2_frame_t position, nsid3v2_frame_t *frame);

	NSID3V2_EXPORT int NSID3v2_Tag_GetInformation(nsid3v2_tag_t tag, uint8_t *version, uint8_t *revision, int *flags);

	/* 
	get specific information out of a tag.  returns the first one found that matches the requirements.
	*/
	enum
	{
		NSID3V2_TEXT_SYSTEM=1, // use system code page instead of ISO-8859-1
	};
	NSID3V2_EXPORT int NSID3v2_Tag_TXXX_Find(const nsid3v2_tag_t tag, const char *description, nsid3v2_frame_t *out_frame, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Tag_ID_Find(const nsid3v2_tag_t tag, const char *owner, nsid3v2_frame_t *out_frame, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Tag_Comments_Find(const nsid3v2_tag_t tag, const char *description, nsid3v2_frame_t *out_frame, int text_flags);

	NSID3V2_EXPORT int NSID3v2_Tag_Text_Get(const nsid3v2_tag_t tag, int frame_enum, nx_string_t *value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Tag_TXXX_Get(const nsid3v2_tag_t tag, const char *description, nx_string_t *value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Tag_Popularimeter_GetRatingPlaycount(const nsid3v2_tag_t tag, const char *email, uint8_t *rating, uint64_t *playcount);
	NSID3V2_EXPORT int NSID3v2_Tag_Comments_Get(const nsid3v2_tag_t tag, const char *description, char language[3], nx_string_t *value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Tag_WXXX_Get(const nsid3v2_tag_t tag, const char *description, nx_string_t *value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Tag_ID_Get(const nsid3v2_tag_t tag, const char *owner, const void **id_data, size_t *length, int text_flags);

	NSID3V2_EXPORT int NSID3v2_Frame_GetInformation(nsid3v2_frame_t frame, int *type, int *flags);

	NSID3V2_EXPORT int NSID3v2_Frame_Text_Get(const nsid3v2_frame_t frame, nx_string_t *value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Frame_UserText_Get(const nsid3v2_frame_t frame, nx_string_t *description, nx_string_t *value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Frame_Private_Get(const nsid3v2_frame_t frame, nx_string_t *description, const void **data, size_t *length);
	NSID3V2_EXPORT int NSID3v2_Frame_Object_Get(const nsid3v2_frame_t frame, nx_string_t *mime, nx_string_t *filename, nx_string_t *description, const void **out_data, size_t *length, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Frame_Popularity_Get(nsid3v2_frame_t frame, nx_string_t *email, uint8_t *rating, uint64_t *playcount, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Frame_Picture_Get(const nsid3v2_frame_t frame, nx_string_t *mime, uint8_t *picture_type, nx_string_t *description, const void **picture_data, size_t *length, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Frame_ID_Get(nsid3v2_frame_t frame, nx_string_t *owner, const void **id_data, size_t *length, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Frame_Comments_Get(const nsid3v2_frame_t frame, nx_string_t *description, char language[3], nx_string_t *value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Frame_URL_Get(const nsid3v2_frame_t frame, nx_string_t *value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Frame_UserURL_Get(const nsid3v2_frame_t frame, nx_string_t *description, nx_string_t *value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Frame_Binary_Get(nsid3v2_frame_t frame, const void **binary, size_t *length);

	NSID3V2_EXPORT int NSID3v2_Frame_GetIdentifier(nsid3v2_frame_t frame, const char **identifier);

	enum
	{
		SerializedSize_Padding      = (0x1 << 0), // write 'padding_size' extra 0's to the file
		SerializedSize_AbsoluteSize = (0x2 << 0), // 'padding_size' represents the final total tag size
		SerializedSize_BlockSize    = (0x3 << 0), // 'padding_size' represents a block size.  the total tag size will be a multiple of 'padding_size'			
		SerializedSize_PaddingMask  = (0x3 << 0),

		// note that setting NEITHER of this will preserve whatever setting was originally used in the tag (or individual frames for ID3v2.4)
		Serialize_Unsynchronize     = (0x1 << 2), // force the tag to be unsynchronized, even if it wasn't originally
		Serialize_NoUnsynchronize   = (0x2 << 2), // disable all unsynchronization, even if the tag was originally unsynchronized
		Serialize_UnsynchronizeMask = (0x3 << 2),

		Serialize_NoCompression   = (0x1 << 4), // disables all compression
		Serialize_CompressionMask = (0x1 << 4), 
	};

	NSID3V2_EXPORT int NSID3v2_Tag_SerializedSize(nsid3v2_tag_t tag, uint32_t *length, uint32_t padding_size, int flags);
	NSID3V2_EXPORT int NSID3v2_Tag_Serialize(nsid3v2_tag_t tag, void *data, uint32_t len, int flags);

	NSID3V2_EXPORT int NSID3v2_Tag_Text_Set(nsid3v2_tag_t tag, int frame_enum, nx_string_t value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Tag_TXXX_Set(nsid3v2_tag_t tag, const char *description, nx_string_t value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Tag_Comments_Set(nsid3v2_tag_t tag, const char *description, const char language[3], nx_string_t value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Tag_ID_Set(nsid3v2_tag_t tag, const char *owner, const void *id_data, size_t length, int text_flags);

	NSID3V2_EXPORT int NSID3v2_Frame_Text_Set(nsid3v2_frame_t frame, nx_string_t value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Frame_UserText_Set(nsid3v2_frame_t frame, const char *description, nx_string_t value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Frame_Comments_Set(nsid3v2_frame_t frame, const char *description, const char language[3], nx_string_t value, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Frame_ID_Set(nsid3v2_frame_t frame, const char *owner, const void *id_data, size_t length, int text_flags);
	NSID3V2_EXPORT int NSID3v2_Frame_Picture_Set(nsid3v2_frame_t frame, nx_string_t mime, uint8_t picture_type, nx_string_t description, const void *picture_data, size_t length, int text_flags);
	enum
	{
		NSID3V2_FRAME_PICTURE, // APIC
		NSID3V2_FRAME_COMMENTS, // COMM
		NSID3V2_FRAME_POPULARIMETER, // POPM
		NSID3V2_FRAME_ALBUM, // TALB
		NSID3V2_FRAME_BPM, // TBPM
		NSID3V2_FRAME_COMPOSER, // TCOM
		NSID3V2_FRAME_CONTENTTYPE, // TCON
		NSID3V2_FRAME_COPYRIGHT, // TCOP
		NSID3V2_FRAME_DATE, // TDAT
		NSID3V2_FRAME_PLAYLISTDELAY, // TDLY
		NSID3V2_FRAME_RECORDINGTIME, // TDRC
		NSID3V2_FRAME_ENCODEDBY, // TENC
		NSID3V2_FRAME_LYRICIST, // TEXT
		NSID3V2_FRAME_FILETYPE, // TFLT
		NSID3V2_FRAME_TIME, // TIME
		NSID3V2_FRAME_CONTENTGROUP, // TIT1
		NSID3V2_FRAME_TITLE, // TIT2		
		NSID3V2_FRAME_SUBTITLE, // TIT3
		NSID3V2_FRAME_KEY, // TKEY
		NSID3V2_FRAME_LANGUAGE, // TLAN
		NSID3V2_FRAME_LENGTH, // TLEN
		NSID3V2_FRAME_MEDIATYPE, // TMED
		NSID3V2_FRAME_MOOD, // TMOO
		NSID3V2_FRAME_ORIGINALALBUM, // TOAL

		NSID3V2_FRAME_ORIGINALARTIST, // TOPE		

		NSID3V2_FRAME_LEADARTIST, // TPE1
		NSID3V2_FRAME_BAND, // TPE2
		NSID3V2_FRAME_CONDUCTOR, // TPE3
		NSID3V2_FRAME_REMIXER, // TPE4
		NSID3V2_FRAME_PARTOFSET, // TPOS
		NSID3V2_FRAME_PUBLISHER, // TPUB
		NSID3V2_FRAME_TRACK, // TRCK
		NSID3V2_FRAME_RECORDINGDATES, // TRDA

		NSID3V2_FRAME_ISRC, // TSRC
		NSID3V2_FRAME_ENCODERSETTINGS, // TSSE
		NSID3V2_FRAME_YEAR, // TYER

		NSID3V2_FRAME_USER_TEXT, // TXXX

		NSID3V2_FRAME_ID, // UFID

	};

	/* frame types */
	enum
	{
		NSID3V2_FRAMETYPE_UNKNOWN,
		NSID3V2_FRAMETYPE_TEXT,
		NSID3V2_FRAMETYPE_USERTEXT,
		NSID3V2_FRAMETYPE_COMMENTS,
		NSID3V2_FRAMETYPE_URL,
		NSID3V2_FRAMETYPE_USERURL,
		NSID3V2_FRAMETYPE_PRIVATE,
		NSID3V2_FRAMETYPE_OBJECT,
		NSID3V2_FRAMETYPE_POPULARITY,
		NSID3V2_FRAMETYPE_PICTURE,
		NSID3V2_FRAMETYPE_ID,
	};

	/* these DO NOT map to the actual flag bitmasks, they are only for API usage! */
	enum
	{
		NSID3V2_TAGFLAG_EXTENDED_HEADER = 1<<1,
		NSID3V2_TAGFLAG_UNSYNCHRONIZED = 1<<2,
		NSID3V2_TAGFLAG_HASFOOTER = 1<<3,

		NSID3V2_FRAMEFLAG_TAG_ALTER_PRESERVATION = 1<<1,
		NSID3V2_FRAMEFLAG_FILE_ALTER_PRESERVATION = 1<<2,
		NSID3V2_FRAMEFLAG_ENCRYPTED = 1<<3,
		NSID3V2_FRAMEFLAG_COMPRESSED = 1<<4,
		NSID3V2_FRAMEFLAG_GROUPED = 1<<5,
		NSID3V2_FRAMEFLAG_READONLY =1<<6,
		NSID3V2_FRAMEFLAG_UNSYNCHRONIZED =1<<7,
		NSID3V2_FRAMEFLAG_DATALENGTHINDICATED =1<<8,
	};

#ifdef __cplusplus
}
#endif
