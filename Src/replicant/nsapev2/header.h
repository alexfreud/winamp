#pragma once
#include "foundation/types.h"
#include "nu/ByteWriter.h"
class nstest;

namespace APEv2
{
	class Header
	{
		friend class ::nstest;
	public:
		Header();
		Header(const void *data);
		void SetSize(uint32_t size) { this->size = size; }
		void SetFlags(uint32_t flags) { this->flags = flags; }
		void SetItems(uint32_t items) { this->items = items; }
		bool Valid() const;
		uint32_t TagSize() const;
		bool HasHeader() const;
		bool HasFooter() const;
		bool IsFooter() const;
		bool IsHeader() const;
		int Encode(bytewriter_t byte_writer) const;
		uint32_t GetFlags() const;
		enum
		{
			SIZE=32,
		};
	private:
		uint8_t preamble[8];
		uint32_t version;
		uint32_t size;
		uint32_t items;
		uint32_t flags;
		uint64_t reserved;
	};
}


