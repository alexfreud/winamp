#pragma once
#include "header.h"

namespace ID3v2_21
{
#pragma pack(push, 1)
	struct ExtendedHeaderData
	{
		uint32_t size;
	};
#pragma pack(pop)

	class ExtendedHeaderBase
	{
	public:
		ExtendedHeaderBase(const ID3v2::Header &_tagHeader);
		int Parse(const void *_data, size_t len, size_t *bytes_read);
		enum
		{
			SIZE=4,
		};
	protected:
		uint32_t Size() const;
		void *data;
		size_t data_size;
		ExtendedHeaderData headerData;
		const ID3v2::Header &tagHeader;
	};
}

namespace ID3v2_3
{
	class ExtendedHeader : public ID3v2_21::ExtendedHeaderBase
	{
	public:
		ExtendedHeader(const ID3v2::Header &_tagHeader);
	};
}

namespace ID3v2_4
{
	class ExtendedHeader : public ID3v2_21::ExtendedHeaderBase
	{
	public:
		ExtendedHeader(const ID3v2::Header &_tagHeader);
		int Parse(const void *_data, size_t len, size_t *bytes_read);

	protected:
		uint32_t Size() const;

	};
}
