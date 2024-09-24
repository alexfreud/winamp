#ifndef FOURCC_HPP
#define FOURCC_HPP

#include <iosfwd>
#include <cstring>


#if defined(__POWERPC__) || defined(__APPLE__) || defined(__MERKS__)
using namespace std;
#endif

class FourCC
{
public:  

    FourCC();
    FourCC(const char*);
    explicit FourCC(unsigned long);

    bool operator==(const FourCC&) const;
    bool operator!=(const FourCC&) const;

    bool operator==(const char*) const;
    bool operator!=(const char*) const;

    operator unsigned long() const;
    unsigned long asLong() const;

    FourCC& operator=(unsigned long);

    char operator[](int) const;

    std::ostream& put(std::ostream&) const;

    bool printable() const;

private:

    union
    {
        char code[4];
        unsigned long codeAsLong;
    };

};


inline FourCC::FourCC()
{
}

inline FourCC::FourCC(unsigned long x)
    : codeAsLong(x)
{
}

inline FourCC::FourCC(const char* str)
{
    memcpy(code, str, 4);
}


inline bool FourCC::operator==(const FourCC& rhs) const
{
    return codeAsLong == rhs.codeAsLong;
}

inline bool FourCC::operator!=(const FourCC& rhs) const
{
    return !operator==(rhs);
}

inline bool FourCC::operator==(const char* rhs) const
{
    return (memcmp(code, rhs, 4) == 0);
}

inline bool FourCC::operator!=(const char* rhs) const
{
    return !operator==(rhs);
}


inline FourCC::operator unsigned long() const
{
    return codeAsLong;
}

inline unsigned long FourCC::asLong() const
{
    return codeAsLong;
}

inline char FourCC::operator[](int i) const
{
    return code[i];
}

inline FourCC& FourCC::operator=(unsigned long val)
{
    codeAsLong = val;
    return *this;
}

inline std::ostream& operator<<(std::ostream& os, const FourCC& rhs)
{
    return rhs.put(os);
}

#endif
