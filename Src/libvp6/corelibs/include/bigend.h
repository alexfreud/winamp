#ifndef _bigend_h
#define _bigend_h

#if defined(__cplusplus)
extern "C" {
#endif

#define invert2(x) ( (((x)>>8)&0x00ff) | (((x)<<8)&0xff00) )
#define invert4(x) ( ((invert2(x)&0x0000ffff)<<16) | (invert2((x>>16))&0x0000ffff) )

#define highByte(x) (unsigned char)x
#define mid2Byte(x) (unsigned char)(x >> 8)
#define mid1Byte(x) (unsigned char)(x >> 16)
#define lowByte(x) (unsigned char)(x >> 24)

#define SWAPENDS 1

#if defined(__cplusplus)
}
#endif
#endif
