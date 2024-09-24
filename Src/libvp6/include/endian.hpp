#if !defined(ENDIAN_HPP)
#define ENDIAN_HPP
//______________________________________________________________________________
//
//  endian.hpp
//  Facilitate endian conversions.

//namespace xxx
//{

    //--------------------------------------
    inline
    bool big_endian()
    {
        long lTest = 1;

        return (reinterpret_cast<char*>(&lTest)[0] == 0);
    }

    //--------------------------------------
    inline
    short swap_endian(short i16)
    {
        return i16 << 8 & 0xff00
            | i16 >> 8 & 0x00ff;
    }

    //--------------------------------------
    inline
    unsigned short swap_endian(unsigned short ui16)
    {
        return ui16 << 8 & 0xff00
            | ui16 >> 8 & 0x00ff;
    }

    //--------------------------------------
    inline
    unsigned int swap_endian(unsigned int i32)
    {
        return i32 << 24 & 0xff000000
            | i32 << 8 & 0x00ff0000
            | i32 >> 8 & 0x0000ff00
            | i32 >> 24 & 0x000000ff;
    }

    //--------------------------------------
    inline
    int swap_endian(int ui32)
    {
        return ui32 << 24 & 0xff000000
            | ui32 << 8 & 0x00ff0000
            | ui32 >> 8 & 0x0000ff00
            | ui32 >> 24 & 0x000000ff;
    }

    //--------------------------------------
    inline
    short big_endian(short i16Native)
    {
        if (big_endian())
        {
            return i16Native;
        }

        return swap_endian(i16Native);
    }

    //--------------------------------------
    inline
    unsigned short big_endian(unsigned short ui16Native)
    {
        if (big_endian())
        {
            return ui16Native;
        }

        return swap_endian(ui16Native);
    }

    //--------------------------------------
    inline
    unsigned int big_endian(unsigned int i32Native)
    {
        if (big_endian())
        {
            return i32Native;
        }

        return swap_endian(i32Native);
    }

    //--------------------------------------
    inline
    int big_endian(int ui32Native)
    {
        if (big_endian())
        {
            return ui32Native;
        }

        return swap_endian(ui32Native);
    }

    //--------------------------------------
    inline
    short little_endian(short i16Native)
    {
        if (!big_endian())
        {
            return i16Native;
        }

        return swap_endian(i16Native);
    }

    //--------------------------------------
    inline
    unsigned short little_endian(unsigned short ui16Native)
    {
        if (!big_endian())
        {
            return ui16Native;
        }

        return swap_endian(ui16Native);
    }

    //--------------------------------------
    inline
    unsigned int little_endian(unsigned int i32Native)
    {
        if (!big_endian())
        {
            return i32Native;
        }

        return swap_endian(i32Native);
    }

    //--------------------------------------
    inline
    int little_endian(int ui32Native)
    {
        if (!big_endian())
        {
            return ui32Native;
        }

        return swap_endian(ui32Native);
    }

    //--------------------------------------
    inline
    short native_endian(short i16, bool bBigEndian)
    {
        if (big_endian() != bBigEndian)
        {
            return swap_endian(i16);
        }

        return i16;
    }

    //--------------------------------------
    inline
    unsigned short native_endian(unsigned short ui16, bool bBigEndian)
    {
        if (big_endian() != bBigEndian)
        {
            return swap_endian(ui16);
        }

        return ui16;
    }

    //--------------------------------------
    inline
    unsigned int native_endian(unsigned int i32, bool bBigEndian)
    {
        if (big_endian() != bBigEndian)
        {
            return swap_endian(i32);
        }

        return i32;
    }

    //--------------------------------------
    inline
    int native_endian(int ui32, bool bBigEndian)
    {
        if (big_endian() != bBigEndian)
        {
            return swap_endian(ui32);
        }

        return ui32;
    }

//}  //  namespace xxx

#endif  //  ENDIAN_HPP
