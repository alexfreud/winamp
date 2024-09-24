#pragma once
#include "foundation/types.h"
#include "nx/nxstring.h"
#include "nu/PtrDeque.h"
#include "nu/ByteWriter.h"
#include "nu/ByteReader.h"
namespace APEv2
{
	enum
	{
		ITEM_KEY_COMPARE_CASE_INSENSITIVE = 0,
		ITEM_KEY_COMPARE_CASE_SENSITIVE = 1,
	};

	class Item : public nu::PtrDequeNode
	{
	public:
		Item();
		~Item();

		/* If successful, puts incremented data pointer in new_data, and new data size remaining in new_len */
		int Read(bytereader_t byte_reader);

		int Encode(bytewriter_t byte_writer) const;
		size_t EncodeSize() const;

		bool IsReadOnly();
		bool KeyMatch(const char *key, int compare=ITEM_KEY_COMPARE_CASE_INSENSITIVE);
		int Get(const void **data, size_t *len) const;
		int GetKey(const char **tag) const;
		int Set(nx_string_t value);
		int Set(const void *data, size_t len, int dataType);
		int SetKey(const char *tag);
		int New(size_t datalen, int data_type, void **bytes);
		uint32_t GetFlags() const;

	private:
		uint32_t flags;
		char *key;
		void *value;
		uint32_t len;
	};
}
