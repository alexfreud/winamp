#pragma once
#include "foundation/types.h"

/* benski>
This is where we encapsulate all data.
Everything is implemented by a function that accepts a version and revision.
*/
namespace ID3v2
{
	namespace Values
	{
		enum
		{
			MIN_VERSION = 2,
			MAX_VERSION = 4,
		};

		bool KnownVersion(uint8_t version, uint8_t revision);
		uint8_t ValidHeaderMask(uint8_t version, uint8_t revision);
		uint8_t ExtendedHeaderFlag(uint8_t version, uint8_t revision);
	}
};
