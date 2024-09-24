#include "header.h"
#include "read.h"
#include "global_elements.h"

#ifdef WA_VALIDATE
extern uint64_t max_id_length;
extern uint64_t max_size_length;
#endif

// returns bytes read.  0 means EOF
uint64_t nsmkv::ReadHeader(nsmkv::MKVReader *reader, uint64_t size, nsmkv::Header &header)
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
		case mkv_header_doctype:
			{
				char *utf8=0;
				if (read_utf8(reader, node.size, &utf8) == 0)
					return 0;

				header.OwnDocType(utf8);
#ifdef WA_VALIDATE
				header.doctype_found = true;
				printf("  DocType: %s\n", header.doctype);
#endif
			}
			break;
		case mkv_header_doctype_version:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

				header.doctype_version = val;
#ifdef WA_VALIDATE
				header.doctype_version_found = true;
				printf("  DocType Version: %I64u\n", header.doctype_version);
#endif
			}
			break;
		case mkv_header_doctype_read_version:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

				header.doctype_read_version = val;
#ifdef WA_VALIDATE
				header.doctype_read_version_found = true;
				printf("  DocType Read Version: %I64u\n", header.doctype_read_version);
#endif
			}
			break;
		case mkv_header_ebml_version:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

				header.ebml_version = val;
#ifdef WA_VALIDATE
				header.ebml_version_found = true;
				printf("  EBML Version: %I64u\n", header.ebml_version);
#endif
			}
			break;
		case mkv_header_ebml_read_version:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

				header.ebml_read_version = val;
#ifdef WA_VALIDATE
				header.ebml_read_version_found = true;
				printf("  EBML Read Version: %I64u\n", header.ebml_read_version);
#endif
			}
			break;
		case mkv_header_ebml_max_id_length:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

				header.ebml_max_id_length = val;
#ifdef WA_VALIDATE
				max_id_length = val;
				header.ebml_max_id_length_found = true;
				printf("  EBML Max ID Length: %I64u\n", header.ebml_max_id_length);
#endif
			}
			break;
		case mkv_header_ebml_max_size_length:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

				header.ebml_max_size_length = val;
#ifdef WA_VALIDATE
				max_size_length = val;
				header.ebml_max_size_length_found = true;
				printf("  EBML Max Size Length: %I64u\n", header.ebml_max_size_length);
#endif	

			}
			break;
		default:
			{
				if (ReadGlobal(reader, node.id, node.size) == 0)
					return 0;
			}
		}
	}

	return total_bytes_read;
}
