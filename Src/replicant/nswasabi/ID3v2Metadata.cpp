#include "ID3v2Metadata.h"
#include "metadata/MetadataKeys.h"
#include "nswasabi/ReferenceCounted.h"
#include <stdlib.h>
#include <stdio.h>

api_metadata *ID3v2Metadata::metadata_api=0;

static inline bool TestFlag(int flags, int flag_to_check)
{
	if (flags & flag_to_check)
		return true;
	return false;
}

ID3v2Metadata::ID3v2Metadata()
{
	id3v2_tag=0;

#ifdef __APPLE__
	number_formatter = NULL;
#endif
}

ID3v2Metadata::~ID3v2Metadata()
{
#ifdef __APPLE__	
	if (NULL != number_formatter)
		CFRelease(number_formatter);
#endif	
}

int ID3v2Metadata::Initialize(api_metadata *metadata_api)
{
	ID3v2Metadata::metadata_api = metadata_api;
	return NErr_Success;
}

int ID3v2Metadata::Initialize(nsid3v2_tag_t tag)
{
	id3v2_tag = tag;

	return NErr_Success;
}

int ID3v2Metadata::GetGenre(int index, nx_string_t *value)
{
	nx_string_t genre=0;
	int ret = NSID3v2_Tag_Text_Get(id3v2_tag, NSID3V2_FRAME_CONTENTTYPE, &genre, 0);
	if (ret != NErr_Success)
		return ret;

	if (index > 0)
		return NErr_EndOfEnumeration;

	if (genre)
	{
		*value = genre;
#ifdef _WIN32
		// parse the (##) out of it
		wchar_t *tmp = genre->string;
		while (*tmp == ' ') tmp++;
		if (!wcsncmp(tmp, L"(RX)", 4))
		{
			*value = NXStringCreateFromUTF8("Remix");
			NXStringRelease(genre);
			if (*value)
				return NErr_Success;
			else
				return NErr_OutOfMemory;
		}
		else if (!wcsncmp(tmp, L"(CR)", 4))
		{
			*value = NXStringCreateFromUTF8("Cover");
			NXStringRelease(genre);
			if (*value)
				return NErr_Success;
			else
				return NErr_OutOfMemory;
		}

		if (*tmp == '(' || (*tmp >= '0' && *tmp <= '9')) // both (%d) and %d forms
		{
			int noparam = 0;

			if (*tmp == '(') tmp++;
			else noparam = 1;
			size_t genre_index = _wtoi(tmp);
			int cnt = 0;
			while (*tmp >= '0' && *tmp <= '9') cnt++, tmp++;
			while (*tmp == ' ') tmp++;

			if (((!*tmp && noparam) || (!noparam && *tmp == ')')) && cnt > 0)
			{
				if (genre_index < 256 && metadata_api)
				{
					int ret = metadata_api->GetGenre(genre_index, value);
					if (ret == NErr_Success)
					{
						NXStringRetain(*value);
						NXStringRelease(genre);
						return ret;
					}
				}				
			}
		}
#elif defined(__APPLE__)
		int ret = NErr_Success;
		
		CFMutableStringRef mutable_genre = CFStringCreateMutableCopy(NULL, 0, genre);
		CFStringTrimWhitespace(mutable_genre);
		
		CFIndex mutable_genre_length = CFStringGetLength(mutable_genre);
		
		if (kCFCompareEqualTo == CFStringCompareWithOptionsAndLocale(mutable_genre, 
																	CFSTR("(RX)"), 
																	CFRangeMake(0, mutable_genre_length), 
																	0, 
																	NULL))
		{
			NXStringRelease(genre);
			*value = CFSTR("Remix");
			ret = NErr_Success;
		}
		else if (kCFCompareEqualTo == CFStringCompareWithOptionsAndLocale(mutable_genre, 
																		 CFSTR("(CR)"), 
																		CFRangeMake(0, mutable_genre_length), 
																		 0, 
																		 NULL))
		{
			NXStringRelease(genre);
			*value = CFSTR("Cover");
			ret = NErr_Success;
		}
		else
		{
			CFStringTrim(mutable_genre, CFSTR("("));
			CFStringTrim(mutable_genre, CFSTR(")"));
			mutable_genre_length = CFStringGetLength(mutable_genre);
			if (mutable_genre_length > 0 
				&& mutable_genre_length < 4)
			{
				if (NULL == number_formatter)
				{
					CFLocaleRef locale = CFLocaleCreate(NULL, CFSTR("en_US_POSIX"));
					number_formatter = CFNumberFormatterCreate(NULL, locale, kCFNumberFormatterDecimalStyle);
					CFRelease(locale);
				}
				
				SInt8 genre_index;
				CFRange number_range = CFRangeMake(0, mutable_genre_length);
				if (NULL != number_formatter
					&& false != CFNumberFormatterGetValueFromString(number_formatter, 
																	mutable_genre, 
																	&number_range, 
																	kCFNumberSInt8Type,
																	&genre_index)
					&& number_range.length == mutable_genre_length
					&& number_range.location == 0)
				{
					
					if (genre_index >= 0
						&& genre_index < 256 
						&& metadata_api)
					{
						int ret = metadata_api->GetGenre(genre_index, value);
						if (ret == NErr_Success)
						{
							NXStringRetain(*value);
							NXStringRelease(genre);
						}
						ret = NErr_Success;
					}
				}
			}
		}
		
		CFRelease(mutable_genre);
		return ret;
#elif defined(__linux__)
		char *tmp = genre->string;
		while (*tmp == ' ') tmp++;

		if (!strncmp(tmp, "(RX)", 4))
		{
			NXStringRelease(genre);
			return NXStringCreateWithUTF8(value, "Remix");
		}
		else if (!strncmp(tmp, "(CR)", 4))
		{
			NXStringRelease(genre);
			return NXStringCreateWithUTF8(value, "Cover");
		}

		if (*tmp == '(' || (*tmp >= '0' && *tmp <= '9')) // both (%d) and %d forms
		{
			int noparam = 0;

			if (*tmp == '(') tmp++;
			else noparam = 1;
			size_t genre_index = atoi(tmp);
			int cnt = 0;
			while (*tmp >= '0' && *tmp <= '9') cnt++, tmp++;
			while (*tmp == ' ') tmp++;

			if (((!*tmp && noparam) || (!noparam && *tmp == ')')) && cnt > 0)
			{
				if (genre_index < 256 && metadata_api)
				{
					int ret = metadata_api->GetGenre(genre_index, value);
					if (ret == NErr_Success)
					{
						NXStringRetain(*value);
						NXStringRelease(genre);
						return ret;
					}
				}	
			}
		}
#else
#error port me!
#endif
	}
	return NErr_Success;
}

static int ID3v2_GetText(nsid3v2_tag_t id3v2_tag, int frame_enum, unsigned int index, nx_string_t *value)
{
	if (!id3v2_tag)
		return NErr_Empty;

	nsid3v2_frame_t frame;
	int ret = NSID3v2_Tag_GetFrame(id3v2_tag, frame_enum, &frame);
	if (ret != NErr_Success)
		return ret;

	if (index > 0)
		return NErr_EndOfEnumeration;

	return NSID3v2_Frame_Text_Get(frame, value, 0);
}

static int ID3v2_GetTXXX(nsid3v2_tag_t id3v2_tag, const char *description, unsigned int index, nx_string_t *value)
{
	if (!id3v2_tag)
		return NErr_Empty;

	nsid3v2_frame_t frame;
	int ret = NSID3v2_Tag_TXXX_Find(id3v2_tag, description, &frame, 0);
	if (ret != NErr_Success)
		return ret;

	if (index > 0)
		return NErr_EndOfEnumeration;

	return NSID3v2_Frame_UserText_Get(frame, 0, value, 0);
}

static int ID3v2_GetComments(nsid3v2_tag_t id3v2_tag, const char *description, unsigned int index, nx_string_t *value)
{
	if (!id3v2_tag)
		return NErr_Empty;

	nsid3v2_frame_t frame;
	int ret = NSID3v2_Tag_Comments_Find(id3v2_tag, description, &frame, 0);
	if (ret != NErr_Success)
		return ret;

	if (index > 0)
		return NErr_EndOfEnumeration;

	return NSID3v2_Frame_Comments_Get(frame, 0, 0, value, 0);
}

// only one of value1 or value2 should be non-NULL
static int SplitSlash(nx_string_t track, nx_string_t *value1, nx_string_t *value2)
{
	char track_utf8[64];
	size_t bytes_copied;
	int ret;
	ret = NXStringGetBytes(&bytes_copied, track, track_utf8, 64, nx_charset_utf8, nx_string_get_bytes_size_null_terminate);
	if (ret == NErr_Success)
	{
		size_t len = strcspn(track_utf8, "/");

		if (value2)
		{
			const char *second = &track_utf8[len];
			if (*second)
				second++;
			
			if (!*second)
				return NErr_Empty;

			return NXStringCreateWithUTF8(value2, second);
		}
		else
		{
			if (len == 0)
				return NErr_Empty;
			return NXStringCreateWithBytes(value1, track_utf8, len, nx_charset_utf8);
		}

		return NErr_Success;						
	}

	return ret;
}

/* ifc_metadata implementation */
int ID3v2Metadata::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	if (!id3v2_tag)
		return NErr_Unknown;

	int ret;

	switch (field)
	{
	case MetadataKeys::ARTIST:
		return ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_LEADARTIST, index, value);

	case MetadataKeys::ALBUM_ARTIST:
		ret = ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_BAND, index, value); /* Windows Media Player style */
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			return ret;

		ret = ID3v2_GetTXXX(id3v2_tag, "ALBUM ARTIST", index, value); /* foobar 2000 style */
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			return ret;

		ret = ID3v2_GetTXXX(id3v2_tag, "ALBUMARTIST", index, value); /* mp3tag style */
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			return ret;

		return ID3v2_GetTXXX(id3v2_tag, "Band", index, value); /* audacity style */

	case MetadataKeys::ALBUM:
		return ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_ALBUM, index, value);

	case MetadataKeys::TITLE:
		return ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_TITLE, index, value);

	case MetadataKeys::GENRE: 
		return GetGenre(index, value);

	case MetadataKeys::TRACK:
		{
			ReferenceCountedNXString track;
			ret = ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_TRACK, index, &track);
			if (ret == NErr_Success)
				return SplitSlash(track, value, 0);

			return ret;			
		}
		break;

			case MetadataKeys::TRACKS:
{
			ReferenceCountedNXString track;
			ret = ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_TRACK, index, &track);
			if (ret == NErr_Success)
				return SplitSlash(track, 0, value);

			return ret;			
		}
		break;

	case MetadataKeys::YEAR:
		ret = ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_RECORDINGTIME, index, value);
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			return ret;

		return ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_YEAR, index, value);

	case MetadataKeys::DISC:
		{
			ReferenceCountedNXString track;
			ret = ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_PARTOFSET, index, &track);
			if (ret == NErr_Success)
				return SplitSlash(track, value, 0);

			return ret;			
		}
		break;

	case MetadataKeys::DISCS:
		{
			ReferenceCountedNXString track;
			ret = ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_PARTOFSET, index, &track);
			if (ret == NErr_Success)
				return SplitSlash(track, 0, value);

			return ret;			
		}
		break;

	case MetadataKeys::COMPOSER:
		return ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_COMPOSER, index, value);

	case MetadataKeys::PUBLISHER:
		return ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_PUBLISHER, index, value);

	case MetadataKeys::BPM:
		return ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_BPM, index, value);

	case MetadataKeys::COMMENT:
		return ID3v2_GetComments(id3v2_tag, "", index, value);
		// TODO case MetadataKeys::PLAY_COUNT:
		// TODO case MetadataKeys::RATING:

	case MetadataKeys::TRACK_GAIN:
		return ID3v2_GetTXXX(id3v2_tag, "replaygain_track_gain", index, value);

	case MetadataKeys::TRACK_PEAK:
		return ID3v2_GetTXXX(id3v2_tag, "replaygain_track_peak", index, value);

	case MetadataKeys::ALBUM_GAIN:
		return ID3v2_GetTXXX(id3v2_tag, "replaygain_album_gain", index, value);

	case MetadataKeys::ALBUM_PEAK:
		return ID3v2_GetTXXX(id3v2_tag, "replaygain_album_peak", index, value);
	}

	return NErr_Unknown;
}

static int IncSafe(const char *&value, size_t &value_length, size_t increment_length)
{
	/* eat leading spaces */
	while (*value == ' ' && value_length) 
	{
		value++;
		value_length--;
	}

	if (increment_length > value_length)
		return NErr_NeedMoreData;

	value += increment_length;
	value_length -= increment_length;
	/* eat trailing spaces */
	while (*value == ' ' && value_length) 
	{
		value++;
		value_length--;
	}

	return NErr_Success;
}

static int SplitSlashInteger(nx_string_t track, unsigned int *value1, unsigned int *value2)
{
	char track_utf8[64];
	size_t bytes_copied;
	int ret;
	ret = NXStringGetBytes(&bytes_copied, track, track_utf8, 64, nx_charset_utf8, nx_string_get_bytes_size_null_terminate);
	if (ret == NErr_Success)
	{
		size_t len = strcspn(track_utf8, "/");

		if (track_utf8[len])
			*value2 = strtoul(&track_utf8[len+1], 0, 10);
		else
			*value2 = 0;

		track_utf8[len]=0;
		*value1 = strtoul(track_utf8, 0, 10);

		return NErr_Success;						
	}

	return ret;
}

int ID3v2Metadata::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	if (!id3v2_tag)
		return NErr_Unknown;

	switch(field)
	{
	case MetadataKeys::TRACK:
		{
			ReferenceCountedNXString track;
			int ret;
			ret = ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_TRACK, index, &track);
			if (ret == NErr_Success)
			{
				unsigned int itrack, itracks;
				ret = SplitSlashInteger(track, &itrack, &itracks);
				if (ret == NErr_Success)
				{
					if (itrack == 0)
						return NErr_Empty;

					*value = itrack;
					return NErr_Success;

				}
			}				
			return ret;			
		}
		break;

	case MetadataKeys::TRACKS:
		{
			ReferenceCountedNXString track;
			int ret;
			ret = ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_TRACK, index, &track);
			if (ret == NErr_Success)
			{
				unsigned int itrack, itracks;
				ret = SplitSlashInteger(track, &itrack, &itracks);
				if (ret == NErr_Success)
				{
					if (itracks == 0)
						return NErr_Empty;

					*value = itracks;
					return NErr_Success;
				}
			}				
			return ret;			
		}
		break;

	case MetadataKeys::DISC:
		{
			ReferenceCountedNXString track;
			int ret;
			ret = ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_PARTOFSET, index, &track);
			if (ret == NErr_Success)
			{
				unsigned int idisc, idiscs;
				ret = SplitSlashInteger(track, &idisc, &idiscs);
				if (ret == NErr_Success)
				{
					if (idisc == 0)
						return NErr_Empty;

					*value = idisc;
					return NErr_Success;

				}
			}				
			return ret;			
		}
		break;

	case MetadataKeys::DISCS:
		{
			ReferenceCountedNXString track;
			int ret;
			ret = ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_PARTOFSET, index, &track);
			if (ret == NErr_Success)
			{
				unsigned int idisc, idiscs;
				ret = SplitSlashInteger(track, &idisc, &idiscs);
				if (ret == NErr_Success)
				{
					if (idiscs == 0)
						return NErr_Empty;

					*value = idiscs;
					return NErr_Success;

				}
			}				
			return ret;			
		}
		break;

	case MetadataKeys::BPM:
		{
			ReferenceCountedNXString bpm;
			int ret;
			ret = ID3v2_GetText(id3v2_tag, NSID3V2_FRAME_BPM, index, &bpm);
			if (ret == NErr_Success)
			{
				/* TODO: benski> implement NXStringGetInt64Value */
				int value32;
				ret = NXStringGetIntegerValue(bpm, &value32);
				if (ret != NErr_Success)
					return ret;
				*value = value32;
				return NErr_Success;
			}
			return ret;
		}
	case MetadataKeys::PREGAP:
		{
			ReferenceCountedNXString str;
			char language[3];
			int ret = NSID3v2_Tag_Comments_Get(id3v2_tag, "iTunSMPB", language, &str, 0);
			if (ret == NErr_Success)
			{
				if (index > 0)
					return NErr_EndOfEnumeration;

				const char *itunsmpb;
				size_t itunsmpb_length;
				char temp[64] = {0};
				if (NXStringGetCString(str, temp, sizeof(temp)/sizeof(*temp), &itunsmpb, &itunsmpb_length) == NErr_Success)
				{
					/* skip first set of meaningless values */
					if (IncSafe(itunsmpb, itunsmpb_length, 8) == NErr_Success && itunsmpb_length >= 8)
					{
						/* read pre-gap */
						*value = strtoul(itunsmpb, 0, 16);
						return NErr_Success;
					}	
				}
				return NErr_Error;
			}
			else
				return ret;

		}
	case MetadataKeys::POSTGAP:
		{
			ReferenceCountedNXString str;
			char language[3];
			int ret = NSID3v2_Tag_Comments_Get(id3v2_tag, "iTunSMPB", language, &str, 0);
			if (ret == NErr_Success)
			{
				if (index > 0)
					return NErr_EndOfEnumeration;

				const char *itunsmpb;
				size_t itunsmpb_length;
				char temp[64] = {0};
				if (NXStringGetCString(str, temp, sizeof(temp)/sizeof(*temp), &itunsmpb, &itunsmpb_length) == NErr_Success)
				{
					/* two separate calls so we can skip spaces properly */
					if (IncSafe(itunsmpb, itunsmpb_length, 8) == NErr_Success && itunsmpb_length >= 8
						&& IncSafe(itunsmpb, itunsmpb_length, 8) == NErr_Success && itunsmpb_length >= 8)
					{
						*value = strtoul(itunsmpb, 0, 16);
						return NErr_Success;
					}

				}	
				return NErr_Error;
			}
			else
				return ret;

		}
	}
	return NErr_Unknown;
}

int ID3v2Metadata::Metadata_GetReal(int field, unsigned int index, double *value)
{
	if (!id3v2_tag)
		return NErr_Unknown;

	int ret;
	nx_string_t str;
	switch (field)
	{
	case MetadataKeys::TRACK_GAIN:
		ret = ID3v2_GetTXXX(id3v2_tag, "replaygain_track_gain", index, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	case MetadataKeys::TRACK_PEAK:
		ret = ID3v2_GetTXXX(id3v2_tag, "replaygain_track_peak", index, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	case MetadataKeys::ALBUM_GAIN:
		ret = ID3v2_GetTXXX(id3v2_tag, "replaygain_album_gain", index, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	case MetadataKeys::ALBUM_PEAK:
		ret = ID3v2_GetTXXX(id3v2_tag, "replaygain_album_peak", index, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	}
	return NErr_Unknown;
}

static int ArtLookupType(uint8_t *id3v2_type, int metadata_key)
{
	switch(metadata_key)
	{
	case MetadataKeys::ALBUM:
		*id3v2_type = 3;
		return NErr_Success;
	}
	return NErr_Unknown;
}

static int NXStringCreateWithMIME(nx_string_t *mime_type, nx_string_t in)
{
	if (!mime_type)
		return NErr_Success;

	char temp[128];
	size_t copied;
	int ret = NXStringGetBytes(&copied, in, temp, 128, nx_charset_ascii, nx_string_get_bytes_size_null_terminate);
	if (ret != NErr_Success)
		return ret;

	if (strstr(temp, "/") != 0)
	{
		*mime_type = NXStringRetain(in);
		return NErr_Success;
	}
	else
	{
		char temp2[128];
#ifdef _WIN32
		_snprintf(temp2, 127, "image/%s", temp);
#else
		snprintf(temp2, 127, "image/%s", temp);
#endif
		temp2[127]=0;

		return NXStringCreateWithUTF8(mime_type, temp2);
	}
}

int ID3v2Metadata::Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	if (!id3v2_tag)
		return NErr_Unknown;

	uint8_t id3v2_picture_type;
	int ret = ArtLookupType(&id3v2_picture_type, field);
	if (ret != NErr_Success)
		return ret;

	if (!id3v2_tag)
		return NErr_Empty;

	bool found_one=false;
	nsid3v2_frame_t frame=0;
	ret = NSID3v2_Tag_GetFrame(id3v2_tag, NSID3V2_FRAME_PICTURE, &frame);
	if (ret != NErr_Success)
		return ret;

	for (;;)
	{
		uint8_t this_type;
		if (NSID3v2_Frame_Picture_Get(frame, 0, &this_type, 0, 0, 0, 0) == NErr_Success && (this_type == id3v2_picture_type || (id3v2_picture_type == 3 && this_type == 0)))
		{
			found_one=true;
			if (index == 0)
			{

				if (artwork)
				{
					nx_data_t data=0;

					if (flags != DATA_FLAG_NONE)
					{
						const void *picture_data;
						size_t picture_length;
						ReferenceCountedNXString mime_local, description;

						ret = NSID3v2_Frame_Picture_Get(frame, TestFlag(flags, DATA_FLAG_MIME)?(&mime_local):0, &this_type, TestFlag(flags, DATA_FLAG_DESCRIPTION)?(&description):0, &picture_data, &picture_length, 0);
						if (ret != NErr_Success)
							return ret;

						if (TestFlag(flags, DATA_FLAG_DATA))
						{
							ret = NXDataCreate(&data, picture_data, picture_length);
							if (ret != NErr_Success)
								return ret;
						}
						else
						{
							ret = NXDataCreateEmpty(&data);
							if (ret != NErr_Success)
								return ret;
						}

						if (mime_local)
						{
							ReferenceCountedNXString mime_type;
							ret = NXStringCreateWithMIME(&mime_type, mime_local);
							if (ret != NErr_Success)
							{
								NXDataRelease(data);
								return ret;
							}
							NXDataSetMIME(data, mime_type);
						}

						if (description)
						{
							NXDataSetDescription(data, description);
						}
					}
					artwork->data = data;	
					/* id3v2 doesn't store height and width, so zero these */
					artwork->width=0;
					artwork->height=0;
				}
				return NErr_Success;
			}
			else
			{
				index--; // keep looking
			}
		}

		if (NSID3v2_Tag_GetNextFrame(id3v2_tag, frame, &frame) != NErr_Success)
		{
			if (found_one)
				return NErr_EndOfEnumeration;
			else
				return NErr_Empty;
		}
	}
}

static int SetText(nsid3v2_tag_t id3v2_tag, int frame_id, unsigned int index, nx_string_t value)
{
	if (index > 0)
		return NErr_Success;

	if (!value)
	{
		nsid3v2_frame_t frame;
		if (NSID3v2_Tag_GetFrame(id3v2_tag, frame_id, &frame) == NErr_Success)
		{
			for(;;)
			{
				nsid3v2_frame_t next;
				int ret = NSID3v2_Tag_GetNextFrame(id3v2_tag, frame, &next);
				NSID3v2_Tag_RemoveFrame(id3v2_tag, frame);
				if (ret != NErr_Success)
					break;
				frame=next;
			}
		}
		return NErr_Success;
	}
	else
	{
		return NSID3v2_Tag_Text_Set(id3v2_tag, frame_id, value, 0);
	}
}

static int SetTXXX(nsid3v2_tag_t id3v2_tag, const char *description, unsigned int index, nx_string_t value,  int text_flags)
{
	if (index > 0)
		return NErr_EndOfEnumeration;

	if (!value)
	{
		nsid3v2_frame_t frame;
		for(;;)
		{
			if (NSID3v2_Tag_TXXX_Find(id3v2_tag, description, &frame, text_flags) == NErr_Success)
				NSID3v2_Tag_RemoveFrame(id3v2_tag, frame);
			else
				return NErr_Success;
		}
	}
	else
	{
		return NSID3v2_Tag_TXXX_Set(id3v2_tag, description, value, 0);
	}
}

static int SetComments(nsid3v2_tag_t id3v2_tag, const char *description, unsigned int index, nx_string_t value,  int text_flags)
{
	if (index > 0)
		return NErr_EndOfEnumeration;

	if (!value)
	{
		nsid3v2_frame_t frame;
		for(;;)
		{
			if (NSID3v2_Tag_Comments_Find(id3v2_tag, description, &frame, text_flags) == NErr_Success)
				NSID3v2_Tag_RemoveFrame(id3v2_tag, frame);
			else
				return NErr_Success;
		}
	}
	else
	{
		return NSID3v2_Tag_Comments_Set(id3v2_tag, description, "\0\0\0", value, 0);
	}
}

int ID3v2Metadata::MetadataEditor_SetField(int field, unsigned int index, nx_string_t value)
{
	int ret;

	switch (field)
	{
	case MetadataKeys::ARTIST:
		return SetText(id3v2_tag, NSID3V2_FRAME_LEADARTIST, index, value);

	case MetadataKeys::ALBUM_ARTIST:
		ret = SetText(id3v2_tag, NSID3V2_FRAME_BAND, index, value);
		/* delete some of the alternates */
		SetTXXX(id3v2_tag, "ALBUM ARTIST", index, 0, 0); /* foobar 2000 style */
		SetTXXX(id3v2_tag, "ALBUMARTIST", index, 0, 0); /* mp3tag style */

		if (!value) /* this might be a valid field, so only delete it if we're specifically deleting album artist (because otherwise, if it's here it's going to get picked up by GetField */
			SetTXXX(id3v2_tag, "Band", index, 0, 0); /* audacity style */

		return ret;

	case MetadataKeys::ALBUM:
		return SetText(id3v2_tag, NSID3V2_FRAME_ALBUM, index, value);

	case MetadataKeys::TITLE:
		return SetText(id3v2_tag, NSID3V2_FRAME_TITLE, index, value);

	case MetadataKeys::GENRE: 
		return SetText(id3v2_tag, NSID3V2_FRAME_CONTENTTYPE, index, value);

	case MetadataKeys::YEAR:
		/* try to set "newer" style TDRC, first */
		ret = SetText(id3v2_tag, NSID3V2_FRAME_RECORDINGTIME, index, value);
		if (ret == NErr_Success)
		{
			/* if it succeeded, remove the older TYER tag */
			SetText(id3v2_tag, NSID3V2_FRAME_RECORDINGTIME, index, 0);
			return ret;
		}

		/* fall back to using TYER */
		return SetText(id3v2_tag, NSID3V2_FRAME_RECORDINGTIME, index, 0);

	case MetadataKeys::TRACK:
		return SetText(id3v2_tag, NSID3V2_FRAME_TRACK, index, value);

	case MetadataKeys::DISC:
		return SetText(id3v2_tag, NSID3V2_FRAME_PARTOFSET, index, value);

	case MetadataKeys::COMPOSER:
		return SetText(id3v2_tag, NSID3V2_FRAME_COMPOSER, index, value);

	case MetadataKeys::PUBLISHER:
		return SetText(id3v2_tag, NSID3V2_FRAME_PUBLISHER, index, value);

	case MetadataKeys::BPM:
		return SetText(id3v2_tag, NSID3V2_FRAME_BPM, index, value);

	case MetadataKeys::COMMENT:
		return SetComments(id3v2_tag, "", index, value, 0);

	case MetadataKeys::TRACK_GAIN:
		return SetTXXX(id3v2_tag, "replaygain_track_gain", index, value, 0);

	case MetadataKeys::TRACK_PEAK:
		return SetTXXX(id3v2_tag, "replaygain_track_peak", index, value, 0);

	case MetadataKeys::ALBUM_GAIN:
		return SetTXXX(id3v2_tag, "replaygain_album_gain", index, value, 0);

	case MetadataKeys::ALBUM_PEAK:
		return SetTXXX(id3v2_tag, "replaygain_album_peak", index, value, 0);
	}

	return NErr_Unknown;
}

static int ID3v2_SetPicture(nsid3v2_frame_t frame, uint8_t id3v2_picture_type, artwork_t *artwork, data_flags_t flags)
{
	int ret;

	const void *picture_data;
	size_t picture_length;
	ret = NXDataGet(artwork->data, &picture_data, &picture_length);
	if (ret != NErr_Success)
		return ret;
	ReferenceCountedNXString mime_type, description;
	if (TestFlag(flags, DATA_FLAG_MIME))
		NXDataGetMIME(artwork->data, &mime_type);
	if (TestFlag(flags, DATA_FLAG_DESCRIPTION))
		NXDataGetDescription(artwork->data, &description);
	return NSID3v2_Frame_Picture_Set(frame, mime_type, id3v2_picture_type, description, picture_data, picture_length, 0);			
}

int ID3v2Metadata::MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	uint8_t id3v2_picture_type;
	int ret = ArtLookupType(&id3v2_picture_type, field);
	if (ret != NErr_Success)
		return ret;


	nsid3v2_frame_t frame=0;
	ret = NSID3v2_Tag_GetFrame(id3v2_tag, NSID3V2_FRAME_PICTURE, &frame);
	if (ret != NErr_Success)
	{
		if (artwork && artwork->data)
		{
			/* create a new one and store */
			int ret = NSID3v2_Tag_CreateFrame(id3v2_tag, NSID3V2_FRAME_PICTURE, 0, &frame);
			if (ret == NErr_Success)
			{
				ret = ID3v2_SetPicture(frame, id3v2_picture_type, artwork, flags);
				if (ret == NErr_Success)
				{
					ret = NSID3v2_Tag_AddFrame(id3v2_tag, frame);
					if (ret != NErr_Success)
					{
						NSID3v2_Tag_RemoveFrame(id3v2_tag, frame);
					}
				}
			}
			return ret;
		}
		else
			return NErr_Success;
	}

	for (;;)
	{
		/* iterate now, because we might delete the current frame */
		nsid3v2_frame_t next_frame=0;
		if (NSID3v2_Tag_GetNextFrame(id3v2_tag, frame, &next_frame) != NErr_Success)
			next_frame=0; /* just in case */

		uint8_t this_type;
		if (NSID3v2_Frame_Picture_Get(frame, 0, &this_type, 0, 0, 0, 0) == NErr_Success && (this_type == id3v2_picture_type || (id3v2_picture_type == 3 && this_type == 0)))
		{
			if (index == 0)
			{
				if (artwork && artwork->data)
				{
					return ID3v2_SetPicture(frame, id3v2_picture_type, artwork, flags);
				}
				else
				{
					NSID3v2_Tag_RemoveFrame(id3v2_tag, frame);
				}
			}
			else
			{
				index--; // keep looking
			}
		}

		if (!next_frame)
		{
			if (!artwork || !artwork->data)
				return NErr_Success;
			else
			{
				/* create a new one and store */
				int ret = NSID3v2_Tag_CreateFrame(id3v2_tag, NSID3V2_FRAME_PICTURE, 0, &frame);
				if (ret != NErr_Success)
					return ret;

				ret = ID3v2_SetPicture(frame, id3v2_picture_type, artwork, flags);
				if (ret != NErr_Success)
					return ret;
				ret = NSID3v2_Tag_AddFrame(id3v2_tag, frame);
				if (ret != NErr_Success)
				{
					NSID3v2_Tag_RemoveFrame(id3v2_tag, frame);
				}
				return ret;
			}
		}
		frame = next_frame;
	}

	return NErr_NotImplemented; 
}
