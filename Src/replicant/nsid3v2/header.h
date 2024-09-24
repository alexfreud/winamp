#pragma once

#include "foundation/types.h"

namespace ID3v2
{
	class Header
	{
	public:
		Header();
		Header(uint8_t version, uint8_t revision);
		Header(const void *data);
		Header(const Header *copy, uint32_t new_size);
		void Parse(const void *data);
		int Serialize(void *data);
		int SerializeAsHeader(void *data);
		int SerializeAsFooter(void *data);
		/* Does this seem like a valid ID3v2 header? */
		bool Valid() const;
		bool FooterValid() const;
		/* how much space the tag occupies on disk */
		uint32_t TagSize() const;
		bool HasExtendedHeader() const;
		uint8_t GetVersion() const;
		uint8_t GetRevision() const;
		bool Unsynchronised() const;
		bool HasFooter() const;
		void SetFooter(bool footer);
		enum
		{
			SIZE=10,
		};

		void ClearExtendedHeader();
		void ClearUnsynchronized();
	private:

		uint8_t marker[3];
		uint8_t version;
		uint8_t revision;
		uint8_t flags;
		uint32_t size;
	};
}
