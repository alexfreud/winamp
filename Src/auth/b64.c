/* /////////////////////////////////////////////////////////////////////////////
 * File:        b64.c
 *
 * Purpose:     Implementation file for the b64 library
 *
 * Created:     18th October 2004
 * Updated:     9th February 2008
 *
 * Home:        http://synesis.com.au/software/
 *
 * Copyright (c) 2004-2008, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer. 
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the names of
 *   any contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////////// */


/** \file b64.c Implementation file for the b64 library
 */

/* /////////////////////////////////////////////////////////////////////////////
 * Version information
 */

#ifndef B64_DOCUMENTATION_SKIP_SECTION
# define B64_VER_C_B64_MAJOR    1
# define B64_VER_C_B64_MINOR    2
# define B64_VER_C_B64_REVISION 2
# define B64_VER_C_B64_EDIT     16
#endif /* !B64_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////////
 * Includes
 */

#include "b64.h"

#include <assert.h>
#include <string.h>

/* /////////////////////////////////////////////////////////////////////////////
 * Constants and definitions
 */

#ifndef B64_DOCUMENTATION_SKIP_SECTION
# define NUM_PLAIN_DATA_BYTES        (3)
# define NUM_ENCODED_DATA_BYTES      (4)
#endif /* !B64_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////////
 * Macros
 */

#ifndef NUM_ELEMENTS
# define NUM_ELEMENTS(x)        (sizeof(x) / sizeof(x[0]))
#endif /* !NUM_ELEMENTS */

/* /////////////////////////////////////////////////////////////////////////////
 * Warnings
 */

#if defined(_MSC_VER) && \
    _MSC_VER < 1000
# pragma warning(disable : 4705)
#endif /* _MSC_VER < 1000 */

/* /////////////////////////////////////////////////////////////////////////////
 * Data
 */

static const char           b64_chars[] =   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const signed char    b64_indexes[]   =   
{
    /* 0 - 31 / 0x00 - 0x1f */
        -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1
    /* 32 - 63 / 0x20 - 0x3f */
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, 62, -1, -1, -1, 63  /* ... , '+', ... '/' */
    ,   52, 53, 54, 55, 56, 57, 58, 59  /* '0' - '7' */
    ,   60, 61, -1, -1, -1, -1, -1, -1  /* '8', '9', ... */
    /* 64 - 95 / 0x40 - 0x5f */
    ,   -1, 0,  1,  2,  3,  4,  5,  6   /* ..., 'A' - 'G' */
    ,   7,  8,  9,  10, 11, 12, 13, 14  /* 'H' - 'O' */
    ,   15, 16, 17, 18, 19, 20, 21, 22  /* 'P' - 'W' */
    ,   23, 24, 25, -1, -1, -1, -1, -1  /* 'X', 'Y', 'Z', ... */
    /* 96 - 127 / 0x60 - 0x7f */
    ,   -1, 26, 27, 28, 29, 30, 31, 32  /* ..., 'a' - 'g' */
    ,   33, 34, 35, 36, 37, 38, 39, 40  /* 'h' - 'o' */
    ,   41, 42, 43, 44, 45, 46, 47, 48  /* 'p' - 'w' */
    ,   49, 50, 51, -1, -1, -1, -1, -1  /* 'x', 'y', 'z', ... */

    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  

    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  

    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  

    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
    ,   -1, -1, -1, -1, -1, -1, -1, -1  
};

/* /////////////////////////////////////////////////////////////////////////////
 * Helper functions
 */

/** This function reads in 3 bytes at a time, and translates them into 4
 * characters.
 */
static size_t b64_encode_(  unsigned char const *src
                        ,   size_t              srcSize
                        ,   char *const         dest
                        ,   size_t              destLen
                        ,   unsigned            lineLen
                        ,   B64_RC              *rc)
{
    size_t  total   =   ((srcSize + (NUM_PLAIN_DATA_BYTES - 1)) / NUM_PLAIN_DATA_BYTES) * NUM_ENCODED_DATA_BYTES;

    assert(NULL != rc);
    *rc = B64_RC_OK;

    if(lineLen > 0)
    {
        unsigned    numLines    =   (unsigned int)(total + (lineLen - 1)) / lineLen;

        total += 2 * (numLines - 1);
    }

    if(NULL == dest)
    {
        return total;
    }
    else if(destLen < total)
    {
        *rc = B64_RC_INSUFFICIENT_BUFFER;

        return 0;
    }
    else
    {
        char    *p      =   dest;
        char    *end    =   dest + destLen;
        size_t  len     =   0;

        for(; NUM_PLAIN_DATA_BYTES <= srcSize; srcSize -= NUM_PLAIN_DATA_BYTES)
        {
            char    characters[NUM_ENCODED_DATA_BYTES] = {0};

            /* 
             * 
             * |       0       |       1       |       2       |
             *
             * |               |               |               |
             * |       |       |       |       |       |       |
             * |   |   |   |   |   |   |   |   |   |   |   |   |
             * | | | | | | | | | | | | | | | | | | | | | | | | |
             * 
             * |     0     |     1     |     2     |     3     |
             * 
             */

            /* characters[0] is the 6 left-most bits of src[0] */
            characters[0] = (char)((src[0] & 0xfc) >> 2);
            /* characters[0] is the right-most 2 bits of src[0] and the left-most 4 bits of src[1] */
            characters[1] = (char)(((src[0] & 0x03) << 4) + ((src[1] & 0xf0) >> 4));
            /* characters[0] is the right-most 4 bits of src[1] and the 2 left-most bits of src[2] */
            characters[2] = (char)(((src[1] & 0x0f) << 2) + ((src[2] & 0xc0) >> 6));
            /* characters[3] is the right-most 6 bits of src[2] */
            characters[3] = (char)(src[2] & 0x3f);

#ifndef __WATCOMC__
            assert(characters[0] >= 0 && characters[0] < 64);
            assert(characters[1] >= 0 && characters[1] < 64);
            assert(characters[2] >= 0 && characters[2] < 64);
            assert(characters[3] >= 0 && characters[3] < 64);
#endif /* __WATCOMC__ */

            src += NUM_PLAIN_DATA_BYTES;
            *p++ = b64_chars[(unsigned char)characters[0]];
            assert(NULL != strchr(b64_chars, *(p-1)));
            ++len;
            assert(len != lineLen);

            *p++ = b64_chars[(unsigned char)characters[1]];
            assert(NULL != strchr(b64_chars, *(p-1)));
            ++len;
            assert(len != lineLen);

            *p++ = b64_chars[(unsigned char)characters[2]];
            assert(NULL != strchr(b64_chars, *(p-1)));
            ++len;
            assert(len != lineLen);

            *p++ = b64_chars[(unsigned char)characters[3]];
            assert(NULL != strchr(b64_chars, *(p-1)));

            if( ++len == lineLen &&
                p != end)
            {
                *p++ = '\r';
                *p++ = '\n';
                len = 0;
            }
        }

        if(0 != srcSize)
        {
            /* Deal with the overspill, by boosting it up to three bytes (using 0s)
             * and then appending '=' for any missing characters.
             *
             * This is done into a temporary buffer, so we can call ourselves and
             * have the output continue to be written direct to the destination.
             */

            unsigned char   dummy[NUM_PLAIN_DATA_BYTES] = {0};
            size_t          i;

            for(i = 0; i < srcSize; ++i)
            {
                dummy[i] = *src++;
            }

            for(; i < NUM_PLAIN_DATA_BYTES; ++i)
            {
                dummy[i] = '\0';
            }

            b64_encode_(&dummy[0], NUM_PLAIN_DATA_BYTES, p, NUM_ENCODED_DATA_BYTES * (1 + 2), 0, rc);

            for(p += 1 + srcSize; srcSize++ < NUM_PLAIN_DATA_BYTES; )
            {
                *p++ = '=';
            }
        }

        return total;
    }
}

/** This function reads in a character string in 4-character chunks, and writes 
 * out the converted form in 3-byte chunks to the destination.
 */
static size_t b64_decode_(  char const      *src
                        ,   size_t          srcLen
                        ,   unsigned char   *dest
                        ,   size_t          destSize
                        ,   unsigned        flags
                        ,   char const      **badChar
                        ,   B64_RC          *rc)
{
    const size_t    wholeChunks     =   (srcLen / NUM_ENCODED_DATA_BYTES);
    const size_t    remainderBytes  =   (srcLen % NUM_ENCODED_DATA_BYTES);
    size_t          maxTotal        =   (wholeChunks + (0 != remainderBytes)) * NUM_PLAIN_DATA_BYTES;
    unsigned char   *dest_          =   dest;

    ((void)remainderBytes);

    assert(NULL != badChar);
    assert(NULL != rc);

    *badChar    =   NULL;
    *rc         =   B64_RC_OK;

    if(NULL == dest)
    {
        return maxTotal;
    }
    else if(destSize < maxTotal)
    {
        *rc = B64_RC_INSUFFICIENT_BUFFER;

        return 0;
    }
    else
    {
        /* Now we iterate through the src, collecting together four characters
         * at a time from the Base-64 alphabet, until the end-point is reached.
         *
         * 
         */

        char const          *begin      =   src;
        char const  *const  end         =   begin + srcLen;
        size_t              currIndex   =   0;
        size_t              numPads     =   0;
        signed char         indexes[NUM_ENCODED_DATA_BYTES] = {0};    /* 4 */

        for(; begin != end; ++begin)
        {
            const char  ch  =   *begin;

            if('=' == ch)
            {
                assert(currIndex < NUM_ENCODED_DATA_BYTES);

                indexes[currIndex++] = '\0';

                ++numPads;
            }
            else
            {
                /* NOTE: Had to rename 'index' to 'ix', due to name clash with GCC on 64-bit Linux. */
                signed char ix   =   b64_indexes[(unsigned char)ch];

                if(-1 == ix)
                {
                    switch(ch)
                    {
                        case    ' ':
                        case    '\t':
                        case    '\b':
                        case    '\v':
                            if(B64_F_STOP_ON_UNEXPECTED_WS & flags)
                            {
                                *rc         =   B64_RC_DATA_ERROR;
                                *badChar    =   begin;
                                return 0;
                            }
                            else
                            {
                                /* Fall through */
                            }
                        case    '\r':
                        case    '\n':
                            continue;
                        default:
                            if(B64_F_STOP_ON_UNKNOWN_CHAR & flags)
                            {
                                *rc         =   B64_RC_DATA_ERROR;
                                *badChar    =   begin;
                                return 0;
                            }
                            else
                            {
                                continue;
                            }
                    }
                }
                else
                {
                    numPads = 0;

                    assert(currIndex < NUM_ENCODED_DATA_BYTES);

                    indexes[currIndex++] = ix;
                }
            }

            if(NUM_ENCODED_DATA_BYTES == currIndex)
            {
                unsigned char   bytes[NUM_PLAIN_DATA_BYTES] = {0};        /* 3 */

                bytes[0] = (unsigned char)((indexes[0] << 2) + ((indexes[1] & 0x30) >> 4));

                currIndex = 0;

                *dest++ = bytes[0];
                if(2 != numPads)
                {
                    bytes[1] = (unsigned char)(((indexes[1] & 0xf) << 4) + ((indexes[2] & 0x3c) >> 2));

                    *dest++ = bytes[1];

                    if(1 != numPads)
                    {
                        bytes[2] = (unsigned char)(((indexes[2] & 0x3) << 6) + indexes[3]);

                        *dest++ = bytes[2];
                    }
                }
                if(0 != numPads)
                {
                    break;
                }
            }
        }

        return (size_t)(dest - dest_);
    }
}

/* /////////////////////////////////////////////////////////////////////////////
 * API functions
 */

size_t b64_encode(void const *src, size_t srcSize, char *dest, size_t destLen)
{
    /* Use Null Object (Variable) here for rc, so do not need to check
     * elsewhere.
     */
    B64_RC  rc_;

    return b64_encode_((unsigned char const*)src, srcSize, dest, destLen, 0, &rc_);
}

size_t b64_encode2( void const  *src
                ,   size_t      srcSize
                ,   char        *dest
                ,   size_t      destLen
                ,   unsigned    flags
                ,   int         lineLen /* = -1 */
                ,   B64_RC      *rc     /* = NULL */)
{
    /* Use Null Object (Variable) here for rc, so do not need to check
     * elsewhere
     */
    B64_RC  rc_;
    if(NULL == rc)
    {
        rc = &rc_;
    }

    switch(B64_F_LINE_LEN_MASK & flags)
    {
        case    B64_F_LINE_LEN_USE_PARAM:
            if(lineLen >= 0)
            {
                break;
            }
            /* Fall through to 64 */
        case    B64_F_LINE_LEN_64:
            lineLen = 64;
            break;
        case    B64_F_LINE_LEN_76:
            lineLen = 76;
            break;
        default:
            assert(!"Bad line length flag specified to b64_encode2()");
        case    B64_F_LINE_LEN_INFINITE:
            lineLen = 0;
            break;
    }

    assert(0 == (lineLen % 4));

    return b64_encode_((unsigned char const*)src, srcSize, dest, destLen, (unsigned)lineLen, rc);
}

size_t b64_decode(char const *src, size_t srcLen, void *dest, size_t destSize)
{
    /* Use Null Object (Variable) here for rc and badChar, so do not need to
     * check elsewhere.
     */
    char const  *badChar_;
    B64_RC      rc_;

    return b64_decode_(src, srcLen, (unsigned char*)dest, destSize, B64_F_STOP_ON_NOTHING, &badChar_, &rc_);
}

size_t b64_decode2( char const  *src
                ,   size_t      srcLen
                ,   void        *dest
                ,   size_t      destSize
                ,   unsigned    flags
                ,   char const  **badChar   /* = NULL */
                ,   B64_RC      *rc         /* = NULL */)
{
    char const      *badChar_;
    B64_RC          rc_;

    /* Use Null Object (Variable) here for rc and badChar, so do not need to
     * check elsewhere.
     */
    if(NULL == badChar)
    {
        badChar = &badChar_;
    }
    if(NULL == rc)
    {
        rc = &rc_;
    }

    return b64_decode_(src, srcLen, (unsigned char*)dest, destSize, flags, badChar, rc);
}

/* ////////////////////////////////////////////////////////////////////////// */

#ifdef B64_DOCUMENTATION_SKIP_SECTION
struct b64ErrorString_t_
#else /* !B64_DOCUMENTATION_SKIP_SECTION */
typedef struct b64ErrorString_t_    b64ErrorString_t_;
struct b64ErrorString_t_
#endif /* !B64_DOCUMENTATION_SKIP_SECTION */
{
    int         code;   /*!< The error code. */
    char const  *str;   /*!< The string.        */
    size_t      len;    /*!< The string length. */
};



#define SEVERITY_STR_DECL(rc, desc)                                                         \
                                                                                            \
    static const char               s_str##rc[] =   desc;                                   \
    static const b64ErrorString_t_  s_rct##rc = { rc, s_str##rc, NUM_ELEMENTS(s_str##rc) - 1 }


#define SEVERITY_STR_ENTRY(rc)                                                          \
                                                                                        \
    &s_rct##rc


static char const *b64_LookupCodeA_(int code, b64ErrorString_t_ const **mappings, size_t cMappings, size_t *len)
{
    /* Use Null Object (Variable) here for len, so do not need to check
     * elsewhere.
     */
    size_t  len_;

    if(NULL == len)
    {
        len = &len_;
    }

    /* Checked, indexed search. */
    if( code >= 0 &&
        code < B64_max_RC_value)
    {
        if(code == mappings[code]->code)
        {
            return (*len = mappings[code]->len, mappings[code]->str);
        }
    }

    /* Linear search. Should only be needed if order in
     * b64_LookupErrorStringA_() messed up.
     */
    { size_t i; for(i = 0; i < cMappings; ++i)
    {
        if(code == mappings[i]->code)
        {
            return (*len = mappings[i]->len, mappings[i]->str);
        }
    }}

    return (*len = 0, "");
}

static char const *b64_LookupErrorStringA_(int error, size_t *len)
{
    SEVERITY_STR_DECL(B64_RC_OK                     ,   "Operation was successful"                                              );
    SEVERITY_STR_DECL(B64_RC_INSUFFICIENT_BUFFER    ,   "The given translation buffer was not of sufficient size"               );
    SEVERITY_STR_DECL(B64_RC_TRUNCATED_INPUT        ,   "The input did not represent a fully formed stream of octet couplings"  );
    SEVERITY_STR_DECL(B64_RC_DATA_ERROR             ,   "Invalid data"                                                          );

    static const b64ErrorString_t_    *s_strings[] = 
    {
        SEVERITY_STR_ENTRY(B64_RC_OK),
        SEVERITY_STR_ENTRY(B64_RC_INSUFFICIENT_BUFFER),
        SEVERITY_STR_ENTRY(B64_RC_TRUNCATED_INPUT),
        SEVERITY_STR_ENTRY(B64_RC_DATA_ERROR),
    };

    return b64_LookupCodeA_(error, s_strings, NUM_ELEMENTS(s_strings), len);
}

char const *b64_getErrorString(B64_RC code)
{
    return b64_LookupErrorStringA_((int)code, NULL);
}

size_t b64_getErrorStringLength(B64_RC code)
{
    size_t  len;

    return (b64_LookupErrorStringA_((int)code, &len), len);
}

/* ////////////////////////////////////////////////////////////////////////// */
