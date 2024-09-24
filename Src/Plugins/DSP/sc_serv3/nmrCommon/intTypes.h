#ifndef intTypes_H_
#define intTypes_H_

#if defined(__APPLE_CC__) | defined(PLATFORM_BSD)
#include <machine/endian.h>
#else
#ifndef WIN32
#include <endian.h>
#endif
#endif

#ifndef WIN32
// these are defined under windows
typedef char 	__int8;
typedef short 	__int16;
typedef int 	__int32;
typedef long long __int64;
#endif

typedef unsigned char 	__uint8;
typedef unsigned short 	__uint16;
typedef unsigned int 	__uint32;
typedef unsigned long long __uint64;

#ifdef __cplusplus


#undef ByteSwap32
#undef ByteSwap16
#undef ByteSwap
#undef bs16
#undef bs32
#undef bs
#undef bs16s
#undef bs16u
#undef bs32s
#undef bs32u

inline __uint32 ByteSwap32u (__uint32 nLongNumber)
{
   return (((nLongNumber&0x000000FF)<<24)+((nLongNumber&0x0000FF00)<<8)+
   ((nLongNumber&0x00FF0000)>>8)+((nLongNumber&0xFF000000)>>24));
}

inline __uint16 ByteSwap16u (__uint16 nValue)
{
   return (((nValue>> 8)) | (nValue << 8));

}

inline __int16 ByteSwap16s(__int16 nValue) { return ByteSwap16u(nValue); }
inline __int32 ByteSwap32s(__int32 nValue) { return ByteSwap32u(nValue); }

inline __uint16 ByteSwap(__uint16 nValue) { return ByteSwap16u(nValue); }
inline __uint32 ByteSwap(__uint32 nValue) { return ByteSwap32u(nValue); }
inline __int16 ByteSwap(__int16 nValue) { return ByteSwap16u(nValue); }
inline __int32 ByteSwap(__int32 nValue) { return ByteSwap32u(nValue); }

#ifdef __BYTE_ORDER

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define bs16(x) (x)
#define bs16s(x) (x)
#define bs16u(x) (x)
#define bs32(x) (x)
#define bs32s(x) (x)
#define bs32u(x) (x)
#define bs(x) (x)
#else
#if __BYTE_ORDER == __BIG_ENDIAN
#define bs16(x) ByteSwap16u(x)
#define bs16s(x) ByteSwap16s(x)
#define bs16u(x) ByteSwap16u(x)
#define bs32(x) ByteSwap32u(x)
#define bs32s(x) ByteSwap32s(x)
#define bs32u(x) ByteSwap32u(x)
#define bs(x) ByteSwap(x)
#else
//#error "No endian defined (__BYTE_ORDER)"
#endif
#endif

#else

#ifdef BYTE_ORDER

#if BYTE_ORDER == LITTLE_ENDIAN
#define bs16(x) (x)
#define bs16s(x) (x)
#define bs16u(x) (x)
#define bs32(x) (x)
#define bs32s(x) (x)
#define bs32u(x) (x)
#define bs(x) (x)
#else
#if BYTE_ORDER == BIG_ENDIAN
#define bs16(x) ByteSwap16u(x)
#define bs16s(x) ByteSwap16s(x)
#define bs16u(x) ByteSwap16u(x)
#define bs32(x) ByteSwap32u(x)
#define bs32s(x) ByteSwap32s(x)
#define bs32u(x) ByteSwap32u(x)
#define bs(x) ByteSwap(x)
#else
//#error "No endian defined (BYTE_ORDER)"
#endif
#endif

#else

//#error "Neither __BYTE_ORDER or BYTE_ORDER is defined"
#endif
#endif

#endif

#endif
