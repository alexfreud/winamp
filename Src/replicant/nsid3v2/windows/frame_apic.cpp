#include "nsid3v2.h"
#include "nsid3v2/header.h"
#include "nsid3v2/tag.h"
#include "nsid3v2/frame_utils.h"
#include <api/memmgr/api_memmgr.h>
#include <strsafe.h>

struct ParsedPicture
{
	uint8_t encoding; // 0 - iso-8859-1, 1 - UTF16LE, 2 - UTF16BE, 3 - UTF8
	const char *mime_type;
	size_t mime_cch;
	uint8_t picture_type;
	union 
	{
		const char *as8;
		const wchar_t *as16;
	} description_data;
	size_t description_cch;
	const void *picture_data;
	size_t picture_bytes;
};

static int ParsePicture(const void *data, size_t data_len, ParsedPicture &parsed)
{
	const uint8_t *data8 = (const uint8_t *)data;
	parsed.encoding = data8[0];
	parsed.mime_type = (const char *)&data8[1];
	data_len--;
	ParseDescription(parsed.mime_type, data_len, parsed.mime_cch);
	parsed.picture_type = data8[2+parsed.mime_cch];
	data_len--;

	switch(parsed.encoding)
	{
	case 0: // ISO-8859-1
		ParseDescription(parsed.description_data.as8, parsed.description_cch, data_len);
		parsed.picture_data = parsed.description_data.as8 + parsed.description_cch + 1;
		parsed.picture_bytes = data_len;
		return NErr_Success;
	case 1: // UTF-16
		ParseDescription(parsed.description_data.as16, parsed.description_cch, data_len, parsed.encoding);
		parsed.picture_data = parsed.description_data.as8 + parsed.description_cch + 1;
		parsed.picture_bytes = data_len;
		return NErr_Success;

	case 2: // UTF-16 BE
		ParseDescription(parsed.description_data.as16, parsed.description_cch, data_len, parsed.encoding);
		parsed.picture_data = parsed.description_data.as8 + parsed.description_cch + 1;
		parsed.picture_bytes = data_len;
		return NErr_Success;
	case 3: // UTF-8
		ParseDescription(parsed.description_data.as8, parsed.description_cch, data_len);
		parsed.picture_data = parsed.description_data.as8 + parsed.description_cch + 1;
		parsed.picture_bytes = data_len;
		return NErr_Success;
	}
	return NErr_NotImplemented;
}

int NSID3v2_Tag_APIC_GetPicture(const nsid3v2_tag_t t, uint8_t picture_type, void *_memmgr, wchar_t **mime_type, void **picture_data, size_t *picture_bytes)
{
	api_memmgr *memmgr = (api_memmgr *)_memmgr;
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	const ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_PICTURE);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedPicture parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParsePicture(data, data_len, parsed) == NErr_Success && parsed.picture_type == picture_type)
		{
			const char *type = strchr(parsed.mime_type, '/');

			if (type && *type)
			{
				type++;
				int typelen = MultiByteToWideChar(CP_ACP, 0, type, -1, 0, 0);
				*mime_type = (wchar_t *)memmgr->sysMalloc(typelen * sizeof(wchar_t));
				MultiByteToWideChar(CP_ACP, 0, type, -1, *mime_type, typelen);
			}
			else
				*mime_type = 0; // unknown!

			*picture_bytes = parsed.picture_bytes;
			*picture_data = memmgr->sysMalloc(parsed.picture_bytes);
			memcpy(*picture_data, parsed.picture_data, parsed.picture_bytes);
			return NErr_Success;
		}
		frame = tag->FindNextFrame(frame);
	}

	return NErr_Error;
}


int NSID3v2_Tag_APIC_GetFirstPicture(const nsid3v2_tag_t t, void *_memmgr, wchar_t **mime_type, void **picture_data, size_t *picture_bytes)
{
	api_memmgr *memmgr = (api_memmgr *)_memmgr;
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	const ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_PICTURE);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedPicture parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParsePicture(data, data_len, parsed) == NErr_Success)
		{
			const char *type = strchr(parsed.mime_type, '/');

			if (type && *type)
			{
				type++;
				int typelen = MultiByteToWideChar(CP_ACP, 0, type, -1, 0, 0);
				*mime_type = (wchar_t *)memmgr->sysMalloc(typelen * sizeof(wchar_t));
				MultiByteToWideChar(CP_ACP, 0, type, -1, *mime_type, typelen);
			}
			else
				*mime_type = 0; // unknown!

			*picture_bytes = parsed.picture_bytes;
			*picture_data = memmgr->sysMalloc(parsed.picture_bytes);
			memcpy(*picture_data, parsed.picture_data, parsed.picture_bytes);
			return NErr_Success;
		}
		frame = tag->FindNextFrame(frame);
	}

	return NErr_Error;
}

int NSID3v2_Tag_APIC_GetFrame(const nsid3v2_tag_t t, uint8_t picture_type, nsid3v2_frame_t *f)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	const ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_PICTURE);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedPicture parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParsePicture(data, data_len, parsed) == NErr_Success && parsed.picture_type == picture_type)
		{
			*f = (nsid3v2_frame_t)frame;
			return NErr_Success;
		}
		frame = tag->FindNextFrame(frame);
	}

	return NErr_Error;
}

int NSID3v2_Tag_APIC_GetFirstFrame(const nsid3v2_tag_t t, nsid3v2_frame_t *f)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	const ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_PICTURE);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedPicture parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParsePicture(data, data_len, parsed) == NErr_Success)
		{
			*f = (nsid3v2_frame_t)frame;
			return NErr_Success;
		}
		frame = tag->FindNextFrame(frame);
	}

	return NErr_Error;
}