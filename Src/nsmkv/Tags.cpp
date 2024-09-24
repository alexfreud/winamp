#include "Tags.h"
#include "global_elements.h"
#include "read.h"

nsmkv::Tags::Tags(void)
{
}

nsmkv::Tags::~Tags(void)
{
}

static uint64_t ReadSimpleTag(FILE *f, uint64_t size)
{
	uint64_t total_bytes_read=0;
	while (size)
	{
		ebml_node node;
		uint64_t bytes_read = read_ebml_node((nsmkv::MKVReader*)f, &node);

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
		case mkv_tags_tag_simpletag_tagname:
			{
				char *utf8 = 0;
				if (node.size && read_utf8((nsmkv::MKVReader*)f, node.size, &utf8) == 0)
					return 0;
				if (utf8)
					printf("    Tag Name: %s\n", utf8);
				free(utf8);
			}
			break;
		case mkv_tags_tag_simpletag_tagstring:
			{
				char *utf8 = 0;
				if (node.size && read_utf8((nsmkv::MKVReader*)f, node.size, &utf8) == 0)
					return 0;
				if (utf8)
					printf("    Tag String: %s\n", utf8);
				free(utf8);
			}
			break;
		case mkv_tags_tag_simpletag_taglanguage:
			{
				char *utf8 = 0;
				if (node.size && read_utf8((nsmkv::MKVReader*)f, node.size, &utf8) == 0)
					return 0;
				if (utf8)
					printf("    Tag Language: %s\n", utf8);
				free(utf8);
			}
			break;
		case mkv_tags_tag_simpletag_tagdefault:
			{
				uint64_t val;
				if (read_unsigned((nsmkv::MKVReader*)f, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Tag Default: %I64u\n", val);
#endif
			}
			break;
		default:
			nsmkv::ReadGlobal((nsmkv::MKVReader*)f, node.id, node.size);
		}
	}
	return total_bytes_read;
}

static uint64_t ReadTarget(FILE *f, uint64_t size)
{
	uint64_t total_bytes_read=0;
	while (size)
	{
		ebml_node node;
		uint64_t bytes_read = read_ebml_node((nsmkv::MKVReader*)f, &node);

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
		case mkv_tags_tag_target_targettypevalue:
			{
				uint64_t val;
				if (read_unsigned((nsmkv::MKVReader*)f, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Target Type Value: %I64u\n", val);
#endif
			}
			break;
		default:
			nsmkv::ReadGlobal((nsmkv::MKVReader*)f, node.id, node.size);
		}
	}
	return total_bytes_read;
}

static uint64_t ReadTag(FILE *f, uint64_t size)
{
	uint64_t total_bytes_read=0;
	while (size)
	{
		ebml_node node;
		uint64_t bytes_read = read_ebml_node((nsmkv::MKVReader*)f, &node);

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
		case mkv_tags_tag_target:
			{
				ReadTarget(f, node.size);
			}
			break;
		case mkv_tags_tag_simpletag:
			{
				ReadSimpleTag(f, node.size);
			}
			break;
		default:
			nsmkv::ReadGlobal((nsmkv::MKVReader*)f, node.id, node.size);
		}
	}
	return total_bytes_read;
}

uint64_t nsmkv::ReadTags(FILE *f, uint64_t size, nsmkv::Tags &tags)
{
	uint64_t total_bytes_read=0;
	while (size)
	{
		ebml_node node;
		uint64_t bytes_read = read_ebml_node((nsmkv::MKVReader*)f, &node);

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
		case mkv_tags_tag:
			{
				ReadTag(f, node.size);
			}
			break;
		default:
			nsmkv::ReadGlobal((nsmkv::MKVReader*)f, node.id, node.size);
		}
	}
	return total_bytes_read;
}