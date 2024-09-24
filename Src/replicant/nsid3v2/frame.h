#pragma once
#include "frameheader.h"
#include "nu/PtrDeque.h"

namespace ID3v2
{
	class Frame : public nu::PtrDequeNode
	{
	public:
		virtual ~Frame();
		int NewData(size_t new_len, void **data, size_t *data_len);
		int GetData(const void **data, size_t *data_len) const;
		size_t GetDataSize() const;
		virtual const int8_t *GetIdentifier() const=0;
		virtual unsigned int GetVersion() const=0;
		
		virtual bool Encrypted() const;
		virtual bool Compressed() const;
		virtual bool Grouped() const;
		virtual bool ReadOnly() const;
		virtual bool FrameUnsynchronised() const;
		virtual bool DataLengthIndicated() const;
		virtual bool TagAlterPreservation() const;
		virtual bool FileAlterPreservation() const;
	protected:
		Frame();
		void *data;
		size_t data_size; /* REAL size, might be different from header.headerData.size */
	};
}

namespace ID3v2_2
{
	class Frame : public ID3v2::Frame
	{
	public:
		Frame(const ID3v2::Header &_header, const int8_t *id, int flags); // creates an empty frame with a given ID
		Frame(const ID3v2_2::FrameHeader &_header);
		int Parse(const void *_data, size_t len, size_t *read);
		int SerializedSize(uint32_t *length, const ID3v2::Header &tag_header, int flags) const;
		// there is enough room guaranteed to be present because it will be checked with SerializedSize()
		int Serialize(void *data, uint32_t *written, const ID3v2::Header &tag_header, int flags) const;
		const int8_t *GetIdentifier() const;
		unsigned int GetVersion() const { return 2; }
	private:
		ID3v2_2::FrameHeader header;
	};
}


namespace ID3v2_3
{
	class Frame : public ID3v2::Frame
	{
	public:
		Frame(const ID3v2::Header &_header, const int8_t *id, int flags); // creates an empty frame with a given ID
		Frame(const ID3v2_3::FrameHeader &_header);
		int Parse(const void *_data, size_t len, size_t *read);
		
		int SerializedSize(uint32_t *length, const ID3v2::Header &tag_header, int flags) const;
		// there is enough room guaranteed to be present because it will be checked with SerializedSize()
		int Serialize(void *data, uint32_t *written, const ID3v2::Header &tag_header, int flags) const;

		const int8_t *GetIdentifier() const;
		unsigned int GetVersion() const { return 3; }
		virtual bool Encrypted() const;
		virtual bool Compressed() const;
		virtual bool Grouped() const;
		virtual bool ReadOnly() const;
		virtual bool TagAlterPreservation() const;
		virtual bool FileAlterPreservation() const;

	private:
		ID3v2_3::FrameHeader header;
		uint8_t group_identity;
		/* helper function
		reads num_bytes from input into output, dealing with re-synchronization and length checking 
		increments bytes_read value by number of input bytes read (different from num_bytes when data is unsynchronized
		decrements input_len by bytes read
		decrements output_len by bytes written
		*/
		bool ReadData(void *output, const void *&input, size_t &input_len, size_t &frame_len, size_t num_bytes, size_t *bytes_read) const;
	};
}

namespace ID3v2_4
{
	class Frame : public ID3v2::Frame
	{
	public:
		Frame(const ID3v2::Header &_header, const int8_t *id, int flags); // creates an empty frame with a given ID
		Frame(const ID3v2_4::FrameHeader &_header);
		int Parse(const void *_data, size_t len, size_t *read);
		int SerializedSize(uint32_t *length, const ID3v2::Header &tag_header, int flags) const;
		int Serialize(void *data, uint32_t *written, const ID3v2::Header &tag_header, int flags) const;
		const int8_t *GetIdentifier() const;
		unsigned int GetVersion() const { return 4; }
		virtual bool Encrypted() const;
		virtual bool Compressed() const;
		virtual bool Grouped() const;
		virtual bool ReadOnly() const;
		virtual bool FrameUnsynchronised() const;
		virtual bool DataLengthIndicated() const;
		virtual bool TagAlterPreservation() const;
		virtual bool FileAlterPreservation() const;
	private:
		ID3v2_4::FrameHeader header;
		uint8_t group_identity;
		/* helper function
		reads num_bytes from input into output, dealing with re-synchronization and length checking 
		increments bytes_read value by number of input bytes read (different from num_bytes when data is unsynchronized
		decrements input_len by bytes read
		decrements output_len by bytes written
		*/
		bool ReadData(void *output, const void *&input, size_t &input_len, size_t &frame_len, size_t num_bytes, size_t *bytes_read) const;
	};
}
