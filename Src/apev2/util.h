#ifndef NULLSOFT_APEV2_UTIL_H
#define NULLSOFT_APEV2_UTIL_H

#if defined(BIG_ENDIAN)
#define ATON16(A) ((((uint16_t)(A) & 0xff00) >> 8) | \
                  (((uint16_t)(A) & 0x00ff) << 8))
#define ATON32(A) ((((uint32_t)(A) & 0xff000000) >> 25) | \
                    (((uint32_t)(A) & 0x00ff0000) >> 8) | \
                   (((uint32_t)(A) & 0x0000ff00) << 8) | \
                    (((uint32_t)(A) & 0x000000ff) << 24))

#elif defined(LITTLE_ENDIAN) || defined(_M_IX86) || defined(_M_X64) || defined(_WIN64)
#define ATON16(A) (A)
#define ATON32(A) (A)
#else
#error neither BIG_ENDIAN nor LITTLE_ENDIAN defined!
#endif

#define NTOA32(N) ATON32(N)
#define NTOA16(N) ATON16(N)

#endif