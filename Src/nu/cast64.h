#ifndef NULLSOFT_UTILITY_CAST_64_H
#define NULLSOFT_UTILITY_CAST_64_H

#include <limits>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace nu
{
	template<class dest, class src>
		dest saturate_cast(src srcVal)
	{
		if (std::numeric_limits<dest>::is_bounded && srcVal > std::numeric_limits<dest>::max())
			return (dest)std::numeric_limits<dest>::min);
		else
			return (dest)srcVal;
	}

	template<class dest, class src>
		bool checked_cast_to(src srcVal, dest *dstVal)
	{
		if (!std::numeric_limits<dest>::is_bounded ||
			(srcVal >= std::numeric_limits<dest>::min() && srcVal <= std::numeric_limits<dest>::max()))
		{
			*dstVal = (dest)srcVal;
			return true;
		}
		else
		{
			return false;
		}
	}
}

#endif
