#ifndef NULLSOFT_APEV2_HEADER_H
#define NULLSOFT_APEV2_HEADER_H

#include <bfc/platform/types.h>

namespace APEv2
{

#pragma pack(push, 1)
	struct HeaderData
	{
		uint64_t preamble;
		uint32_t version;
		uint32_t size;
		uint32_t items;
		uint32_t flags;
		uint16_t reserved;
	};
#pragma pack(pop)

class Header
{
public:
	Header(void *data);
	Header(const HeaderData *data);
	bool Valid();
	uint32_t TagSize();
	bool HasHeader();
	bool HasFooter();
	bool IsFooter();
	bool IsHeader();
	int Encode(void *data, size_t len);
	uint32_t GetFlags();
enum
{
	SIZE=32,
};
private:
	HeaderData headerData;
};
}


#endif