#pragma once
#include "foundation/types.h"
#include "header.h"


namespace ID3v2
{
	class FrameHeader
	{
	protected:
		FrameHeader(const ID3v2::Header &_header);
		const ID3v2::Header &tagHeader;
	};
}

namespace ID3v2_2
{

	struct FrameHeaderData
	{
		int8_t id[4]; // ID3v2.2 uses 3 bytes but we add a NULL for the last to make it easier
		uint8_t size[3]; // 24 bit size field
	};

	class FrameHeader : public ID3v2::FrameHeader
	{
	public:
		FrameHeader(const ID3v2_2::FrameHeader &frame_header, const ID3v2::Header &_header);
		FrameHeader(const ID3v2::Header &_header, const int8_t *id, int flags);
		FrameHeader(const ID3v2::Header &_header, const void *data);
		bool IsValid() const;
		bool Unsynchronised() const;
		uint32_t FrameSize() const;
		const int8_t *GetIdentifier() const;
		void SetSize(uint32_t data_size);
		int SerializedSize(uint32_t *written) const;
		int Serialize(void *data) const;
		enum
		{
			SIZE=6,
		};
	private:
		FrameHeaderData frameHeaderData;
	};
}

namespace ID3v2_3
{

	class FrameHeaderBase : public ID3v2::FrameHeader
	{
	public:
		int SerializedSize(uint32_t *written) const;
		int Serialize(void *data, uint32_t *written) const;
		bool IsValid() const;
		const int8_t *GetIdentifier() const;
		enum
		{
			SIZE=10,
		};

	protected:
		FrameHeaderBase(const ID3v2_3::FrameHeaderBase &frame_header_base, const ID3v2::Header &_header);
		FrameHeaderBase(const ID3v2::Header &_header);
		FrameHeaderBase(const ID3v2::Header &_header, const int8_t *id, int flags);

		int8_t id[4];
		uint32_t size;
		uint8_t flags[2];
	};

	class FrameHeader : public ID3v2_3::FrameHeaderBase
	{
	public:
		FrameHeader(const ID3v2_3::FrameHeader &frame_header, const ID3v2::Header &_header);
		FrameHeader(const ID3v2::Header &_header, const int8_t *id, int flags);
		FrameHeader(const ID3v2::Header &_header, const void *data);
		
		uint32_t FrameSize() const;
		bool Encrypted() const;
		bool Compressed() const;
		bool Grouped() const;
		bool ReadOnly() const;
		bool Unsynchronised() const;
		bool TagAlterPreservation() const;
		bool FileAlterPreservation() const;

		void ClearCompressed();
		/* sets a new size, given a data size.  this function might add additional size (grouped, compressed, etc) */
		void SetSize(uint32_t data_size);
		
	private:

	};
}

namespace ID3v2_4
{
	class FrameHeader : public ID3v2_3::FrameHeaderBase
	{
	public:
		FrameHeader(const ID3v2_4::FrameHeader &frame_header, const ID3v2::Header &_header);
		FrameHeader(const ID3v2::Header &_header, const int8_t *id, int flags);
		FrameHeader(const ID3v2::Header &_header, const void *data);
		/* size to read from disk */
		uint32_t FrameSize() const;
		bool Encrypted() const;
		bool Compressed() const;
		bool Grouped() const;
		bool ReadOnly() const;
		bool Unsynchronised() const;
		bool FrameUnsynchronised() const;
		bool DataLengthIndicated() const;
		bool TagAlterPreservation() const;
		bool FileAlterPreservation() const;

		void ClearUnsynchronized();
		void ClearCompressed();
		/* sets a new size, given a data size.  this function might add additional size (grouped, compressed, etc) */
		void SetSize(uint32_t data_size);
		int SerializedSize(uint32_t *written) const;
		int Serialize(void *data, uint32_t *written) const;
	};
}
