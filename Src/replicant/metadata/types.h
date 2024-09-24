#pragma once
#include "foundation/types.h"
#include "nx/nxdata.h"

typedef int data_flags_t;
enum
{
	DATA_FLAG_NONE=0,
	DATA_FLAG_DATA=(1<<0),
	DATA_FLAG_SOURCE_INFORMATION=(1<<1),
	DATA_FLAG_MIME=(1<<2),
	DATA_FLAG_DESCRIPTION=(1<<3),
	DATA_FLAG_ALL=DATA_FLAG_DATA|DATA_FLAG_SOURCE_INFORMATION|DATA_FLAG_MIME|DATA_FLAG_DESCRIPTION,
};

class artwork_t
{
public:
	artwork_t()
	{
		data=0;
		width=0;
		height=0;
	}

	~artwork_t()
	{
		NXDataRelease(data);
	}

	nx_data_t data;
	uint32_t width;
	uint32_t height;
};
