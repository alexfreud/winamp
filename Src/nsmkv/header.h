#pragma once
#include <bfc/platform/types.h>
#include "mkv_reader.h"

// IDs
// these are slightly different from the matroska spec because we specify
// values after vint decoding and they specify before
const uint32_t mkv_header=0xa45dfa3;
const uint32_t mkv_header_ebml_version=0x286;
const uint32_t mkv_header_ebml_read_version=0x2f7;
const uint32_t mkv_header_ebml_max_id_length=0x2f2;
const uint32_t mkv_header_ebml_max_size_length=0x2f3;
const uint32_t mkv_header_doctype=0x282;
const uint32_t mkv_header_doctype_read_version=0x285;
const uint32_t mkv_header_doctype_version=0x287;

namespace nsmkv
{
	class Header
	{
	public:
		// defaults provided as per spec for matroska
		// *_found variables indicate whether the field was found in the file
		Header() :
			ebml_version(1), 
			ebml_read_version(1),
			ebml_max_id_length(4),
			ebml_max_size_length(8),
			doctype(0),
			doctype_version(1),
			doctype_read_version(1),
			ebml_header_found(false)
#ifdef WA_VALIDATE
			,
			ebml_version_found(false),
			ebml_read_version_found(false),
			ebml_max_id_length_found(false),
			ebml_max_size_length_found(false),
			doctype_version_found(false),
			doctype_found(false),
			doctype_read_version_found(false)
#endif
		{
		}
		~Header()
		{
			if (doctype)
				free(doctype);
		}
		void OwnDocType(char *_doctype)
		{
			if (doctype)
				free(doctype);
			doctype = _doctype;
		}

		uint64_t ebml_version;
		uint64_t ebml_read_version;
		uint64_t ebml_max_id_length;
		uint64_t ebml_max_size_length;
		char *doctype;
		uint64_t doctype_version;
		uint64_t doctype_read_version;
		bool ebml_header_found;

#ifdef WA_VALIDATE
		bool ebml_version_found;
		bool ebml_read_version_found;
		bool ebml_max_id_length_found;
		bool doctype_found;
		bool ebml_max_size_length_found;
		bool doctype_version_found;
		bool doctype_read_version_found;
#endif
	};

	// returns bytes read.  0 means EOF
	uint64_t ReadHeader(nsmkv::MKVReader *reader, uint64_t size, nsmkv::Header &header);
};
