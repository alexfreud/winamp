#include "values.h"

uint8_t ID3v2::Values::ValidHeaderMask(uint8_t version, uint8_t revision)
{
	switch(version)
	{
	case 2:
		if (revision == 1)
			return 0xE0;
		else
			return 0xC0;
	case 4:
		return 0xF0; /* 11110000 */
	case 3:
		return 0xE0; /* 11100000 */
	default:
		return 0;
	}
}

bool ID3v2::Values::KnownVersion(uint8_t version, uint8_t revision)
{
	if (version > Values::MAX_VERSION)
		return false;

	if (version < Values::MIN_VERSION)
		return false;

	return true;
}

uint8_t ID3v2::Values::ExtendedHeaderFlag(uint8_t version, uint8_t revision)
{
	switch(version)
	{
	case 2:
		if (revision == 1)
			return (1 << 6);
		else
			return 0;

	case 3:
	case 4:
		return (1 << 6);

	default: 
		return 0;
	}
}
