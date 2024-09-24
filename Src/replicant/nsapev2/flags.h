#pragma once
#include "foundation/error.h"
namespace APEv2
{
	/*
	http://wiki.hydrogenaudio.org/index.php?title=Ape_Tags_Flags
	*/
	enum
	{
		/* flags for header or item */
		FLAG_READONLY = 1,

		/* header/footer specific flags */
		FLAG_HEADER_HAS_HEADER = (1 << 31),
		FLAG_HEADER_NO_FOOTER = (1 << 30),
		FLAG_HEADER_IS_HEADER = (1 << 29),
		FLAG_HEADER_ENCODE_MASK = FLAG_READONLY|FLAG_HEADER_HAS_HEADER|FLAG_HEADER_NO_FOOTER,
	};

}
