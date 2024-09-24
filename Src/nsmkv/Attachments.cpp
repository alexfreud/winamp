#include "Attachments.h"
#include "read.h"
#include "global_elements.h"

static uint64_t ReadAttachedFile(nsmkv::MKVReader *reader, uint64_t size, nsmkv::AttachedFile &attached_file)
{
	uint64_t total_bytes_read=0;
	while (size)
	{
		ebml_node node;
		uint64_t bytes_read = read_ebml_node(reader, &node);

		if (bytes_read == 0)
			return 0;

		// benski> checking bytes_read and node.size separately prevents possible integer overflow attack
		if (bytes_read > size)
			return 0;
		total_bytes_read+=bytes_read;
		size-=bytes_read;

		if (node.size > size)
			return 0;
		total_bytes_read+=node.size;
		size-=node.size;

		switch(node.id)
		{
		case mkv_attachments_filename:
			{
				char *utf8 = 0;
				if (read_utf8(reader, node.size, &utf8) == 0)
					return 0;
				if (utf8)
					printf("Filename: %s\n", utf8);
				attached_file.Own(attached_file.filename, utf8);
			}
			break;
		case mkv_attachments_filemimetype:
			{
				char *utf8 = 0;
				if (read_utf8(reader, node.size, &utf8) == 0)
					return 0;
				if (utf8)
					printf("File MIME Type: %s\n", utf8);
				attached_file.Own(attached_file.mime_type, utf8);
			}
			break;
		case mkv_attachments_filedata:
			{
				printf("File Data: binary size %I64u\n", node.size);
				reader->Skip(node.size);
			}
			break;
		case mkv_attachments_fileuid:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;
				
				printf("File UID: %I64x\n", val);
				attached_file.file_uid = val;
			}
			break;
		default:
			nsmkv::ReadGlobal(reader, node.id, node.size);
		}
	}
	return total_bytes_read;
}

uint64_t nsmkv::ReadAttachment(nsmkv::MKVReader *reader, uint64_t size, nsmkv::Attachments &attachments)
{
	uint64_t total_bytes_read=0;
	while (size)
	{
		ebml_node node;
		uint64_t bytes_read = read_ebml_node(reader, &node);

		if (bytes_read == 0)
			return 0;

		// benski> checking bytes_read and node.size separately prevents possible integer overflow attack
		if (bytes_read > size)
			return 0;
		total_bytes_read+=bytes_read;
		size-=bytes_read;

		if (node.size > size)
			return 0;
		total_bytes_read+=node.size;
		size-=node.size;

		switch(node.id)
		{
		case mkv_attachments_attachedfile:
			{
				printf("Attachmented File\\n");
				nsmkv::AttachedFile attached_file;
				ReadAttachedFile(reader, node.size, attached_file);
			}
			break;
		default:
			ReadGlobal(reader, node.id, node.size);
		}
	}
	return total_bytes_read;
}
