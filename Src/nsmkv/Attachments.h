#pragma once
#include <bfc/platform/types.h>
#include "mkv_reader.h"

// Attachments
const uint32_t mkv_segment_attachments = 0x941a469;
const uint32_t mkv_attachments_attachedfile = 0x21a7;
const uint32_t mkv_attachments_filename = 0x66e;
const uint32_t mkv_attachments_filemimetype = 0x660;
const uint32_t mkv_attachments_filedata =0x65c;
const uint32_t mkv_attachments_fileuid = 0x6ae;

namespace nsmkv
{
	class AttachedFile
	{
	public:
		AttachedFile()
		{
			file_uid=0;
			filename=0;
			mime_type=0;
		}
		~AttachedFile()
		{
			free(filename);
			free(mime_type);
		}
		void Own(char *&field, char *value)
		{
			if (field)
				free(field);
			field = value;
		}

		uint64_t file_uid;
		char *filename;
		char *mime_type;
	};

	class Attachments
	{
	public:
	};

	uint64_t ReadAttachment(MKVReader *reader, uint64_t size, nsmkv::Attachments &attachments);
}