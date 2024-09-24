#include "APEv2Metadata.h"
#include "metadata/MetadataKeys.h"
#include "nu/ByteReader.h"
#include "nswasabi/ReferenceCounted.h"
#include <stdlib.h>
#include <stdio.h>

static inline bool TestFlag(int flags, int flag_to_check)
{
	if (flags & flag_to_check)
		return true;
	return false;
}

api_metadata *APEv2Metadata::metadata_api=0;

APEv2Metadata::APEv2Metadata()
{
	apev2_tag=0;
}

APEv2Metadata::~APEv2Metadata()
{
}

int APEv2Metadata::Initialize(api_metadata *metadata_api)
{
	APEv2Metadata::metadata_api = metadata_api;
	return NErr_Success;
}

int APEv2Metadata::Initialize(nsapev2_tag_t tag)
{
	apev2_tag = tag;
	return NErr_Success;
}


/* ifc_metadata implementation */
int APEv2Metadata::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	if (!apev2_tag)
		return NErr_Unknown;

	switch (field)
	{
	case MetadataKeys::TRACK_GAIN:
		return NSAPEv2_Tag_GetString(apev2_tag, "REPLAYGAIN_TRACK_GAIN", index, value);
	case MetadataKeys::TRACK_PEAK:
		return NSAPEv2_Tag_GetString(apev2_tag, "REPLAYGAIN_TRACK_PEAK", index, value);
	case MetadataKeys::ALBUM_GAIN:
		return NSAPEv2_Tag_GetString(apev2_tag, "REPLAYGAIN_ALBUM_GAIN", index, value);
	case MetadataKeys::ALBUM_PEAK:
		return NSAPEv2_Tag_GetString(apev2_tag, "REPLAYGAIN_ALBUM_PEAK", index, value);
	}

	return NErr_Unknown;
}

int APEv2Metadata::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	if (!apev2_tag)
		return NErr_Unknown;

	return NErr_Unknown;
}

int APEv2Metadata::Metadata_GetReal(int field, unsigned int index, double *value)
{
	if (!apev2_tag)
		return NErr_Unknown;

	int ret;
	nx_string_t str;
	switch (field)
	{
	case MetadataKeys::TRACK_GAIN:
		ret = NSAPEv2_Tag_GetString(apev2_tag, "REPLAYGAIN_TRACK_GAIN", index, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	case MetadataKeys::TRACK_PEAK:
		ret = NSAPEv2_Tag_GetString(apev2_tag, "REPLAYGAIN_TRACK_PEAK", index, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	case MetadataKeys::ALBUM_GAIN:
		ret = NSAPEv2_Tag_GetString(apev2_tag, "REPLAYGAIN_ALBUM_GAIN", index, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	case MetadataKeys::ALBUM_PEAK:
		ret = NSAPEv2_Tag_GetString(apev2_tag, "REPLAYGAIN_ALBUM_PEAK", index, &str);
		if (ret == NErr_Success)
		{
			ret = NXStringGetDoubleValue(str, value);
			NXStringRelease(str);
		}
		return ret;
	}
	return NErr_Unknown;
}

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

static const char *APEv2_GetMIME(const char *extension)
{
	if (!extension)
		return 0;

	if (strcasecmp(extension, "JPG") == 0 || strcasecmp(extension, "JPEG") == 0)
		return "image/jpeg";
	if (strcasecmp(extension, "PNG") == 0)
		return "image/png";
	if (strcasecmp(extension, "GIF") == 0)
		return "image/gif";
	if (strcasecmp(extension, "BMP") == 0)
		return "image/bmp";

	return 0;
}

static int APEv2_ParseArt(const void *bytes, size_t length, artwork_t *out_data, data_flags_t flags)
{
	if (out_data)
	{
		nx_data_t data=0;
		if (flags != DATA_FLAG_NONE)
		{
			bytereader_s byte_reader;
			bytereader_init(&byte_reader, bytes, length);
			if (bytereader_size(&byte_reader) == 0)
				return NErr_Insufficient;

			const char *description_start = (const char *)bytereader_pointer(&byte_reader);
			const char *extension_start=0;
			uint8_t byte;
			do
			{
				if (bytereader_size(&byte_reader) == 0)
					return NErr_Insufficient;
				byte = bytereader_read_u8(&byte_reader);
				if (byte == '.') // found extension
				{
					extension_start = (const char *)bytereader_pointer(&byte_reader);
				}		
			} while (byte && bytereader_size(&byte_reader));

			size_t length = bytereader_size(&byte_reader);

			if (length == 0)
				return NErr_Empty;

			if (TestFlag(flags, DATA_FLAG_DATA))
			{
				int ret = NXDataCreate(&data, bytereader_pointer(&byte_reader), length);
				if (ret != NErr_Success)
					return ret;
			}
			else
			{
				int ret = NXDataCreateEmpty(&data);
				if (ret != NErr_Success)
					return ret;
			}

			if (TestFlag(flags, DATA_FLAG_DESCRIPTION))
			{
				ReferenceCountedNXString description;
				size_t length;
				if (extension_start)
					length = (size_t)extension_start - (size_t)description_start - 1;
				else
					length = (size_t)bytereader_pointer(&byte_reader) - (size_t)description_start - 1;

				if (length)
				{
					int ret = NXStringCreateWithBytes(&description, description_start, length, nx_charset_utf8);
					if (ret != NErr_Success)
					{
						NXDataRelease(data);
						return ret;
					}
					NXDataSetDescription(data, description);
				}
			}

			if (TestFlag(flags, DATA_FLAG_MIME))
			{
				ReferenceCountedNXString mime_type;
				const char *mime_string = APEv2_GetMIME(extension_start);
				if (mime_string)
				{
					int ret = NXStringCreateWithUTF8(&mime_type, mime_string);
					if (ret != NErr_Success)
					{
						NXDataRelease(data);
						return ret;
					}
				}
			}
		}
		out_data->data = data;
		/* we don't know these */
		out_data->height=0;
		out_data->width=0;
	}
	return NErr_Success;
}

int APEv2Metadata::Metadata_GetArtwork(int field, unsigned int index, artwork_t *data, data_flags_t flags)
{
	if (!apev2_tag)
		return NErr_Unknown;

	int ret;
	const void *bytes;
	size_t length;
	switch(field)
	{
	case MetadataKeys::ALBUM:
		ret = NSAPEv2_Tag_GetBinary(apev2_tag, "Cover Art (front)", index, &bytes, &length);
		if (ret == NErr_Success)
			return APEv2_ParseArt(bytes, length, data, flags);
		return ret;
	}
	return NErr_Unknown;
}

int APEv2Metadata::MetadataEditor_SetField(int field, unsigned int index, nx_string_t value)
{
	switch (field)
	{
	case MetadataKeys::TRACK_GAIN:
		return NSAPEv2_Tag_SetString(apev2_tag, "REPLAYGAIN_TRACK_GAIN", index, value);
	case MetadataKeys::TRACK_PEAK:
		return NSAPEv2_Tag_SetString(apev2_tag, "REPLAYGAIN_TRACK_PEAK", index, value);
	case MetadataKeys::ALBUM_GAIN:
		return NSAPEv2_Tag_SetString(apev2_tag, "REPLAYGAIN_ALBUM_GAIN", index, value);
	case MetadataKeys::ALBUM_PEAK:
		return NSAPEv2_Tag_SetString(apev2_tag, "REPLAYGAIN_ALBUM_PEAK", index, value);
	}
	return NErr_Unknown;
}

int APEv2Metadata::MetadataEditor_SetInteger(int field, unsigned int index, int64_t value)
{
	return NErr_Unknown;
}

int APEv2Metadata::MetadataEditor_SetReal(int field, unsigned int index, double value)
{
	// TODO: but we need NXStringCreateFromDouble which I don't feel like writing right now
	return NErr_Unknown;
}

static void APEv2_GetFilenameForMIME(char *filename, const char *type, nx_string_t mime_type)
{
	if (mime_type)
	{
		if (NXStringKeywordCompareWithCString(mime_type, "image/jpeg") == NErr_True	|| NXStringKeywordCompareWithCString(mime_type, "image/jpg") == NErr_True)
			sprintf(filename, "%s.jpeg", type);
		else if (NXStringKeywordCompareWithCString(mime_type, "image/png") == NErr_True)
			sprintf(filename, "%s.png", type);
		if (NXStringKeywordCompareWithCString(mime_type, "image/gif") == NErr_True)
			sprintf(filename, "%s.gif", type);
		if (NXStringKeywordCompareWithCString(mime_type, "image/bmp") == NErr_True)
			sprintf(filename, "%s.bmp", type);
		else
			sprintf(filename, "%s.jpg", type); // TODO: perhaps we could use whatever is after image/
	}
	else
		sprintf(filename, "%s.jpg", type); // ehh, just guess
}

int APEv2Metadata::MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *data, data_flags_t flags)
{
	ReferenceCountedNXString mime_type;
	const void *bytes = 0;
	size_t length = 0;
	switch(field)
	{
	case MetadataKeys::ALBUM:
		if (data && NXDataGet(data->data, &bytes, &length) == NErr_Success)
		{
			char filename[256] = {0};
			NXDataGetMIME(data->data, &mime_type);
			APEv2_GetFilenameForMIME(filename, "cover", mime_type); /* TODO: perhaps use description, instead? */
			return NSAPEv2_Tag_SetArtwork(apev2_tag, "Cover Art (front)", index, filename, bytes, length);
		}
		else
			return NSAPEv2_Tag_SetArtwork(apev2_tag, "Cover Art (front)", index, 0, 0, 0);
	}
	return NErr_Unknown;
}
