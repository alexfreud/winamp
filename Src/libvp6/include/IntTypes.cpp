#include "IntTypes.hpp"
#pragma warning(disable:4710)
#pragma warning(push,3)
#include <iostream>
#pragma warning(pop)

using std::ostream;


ostream& operator<<(ostream& os, IntTypes::int64_t i)
{
    char buf[65];

    _i64toa(i, buf, 10);

    return os << buf;
}

ostream& operator<<(ostream& os, IntTypes::uint64_t i)
{
    char buf[65];

    _ui64toa(i, buf, 10);

    return os << buf;
}

ostream& operator<<(ostream& os, IntTypes::uint32_t i)
{
    return os << static_cast<unsigned int>(i);
}


ostream& operator<<(ostream& os, IntTypes::uint16_t i)
{
    char buf[33];

    _ultoa(i, buf, 10);

    return os << buf;
}


