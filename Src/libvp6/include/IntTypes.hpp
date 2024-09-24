#ifndef INTTYPES_HPP
#define INTTYPES_HPP

#include <iosfwd>

namespace IntTypes 
{
    typedef __int8 int8_t;
    typedef unsigned __int8 uint8_t;

    typedef __int16 int16_t;
    typedef unsigned __int16 uint16_t;

    typedef __int32 int32_t;
    typedef unsigned __int32 uint32_t;

    typedef __int64 int64_t;
    typedef unsigned __int64 uint64_t;

}

std::ostream& operator<<(std::ostream&, IntTypes::int64_t);
std::ostream& operator<<(std::ostream&, IntTypes::uint64_t);
std::ostream& operator<<(std::ostream&, IntTypes::uint32_t);
std::ostream& operator<<(std::ostream&, IntTypes::uint16_t);

#endif

