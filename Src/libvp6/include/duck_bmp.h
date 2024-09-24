#ifndef _duck_bmp_h
#define _duck_bmp_h

#include "dkpltfrm.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct tDKBITMAP_old
{                                                   
	unsigned long	ulFormatTag;
	unsigned long	usWidth;			/* width */
	unsigned long	usHeight;			/* height */
    unsigned long	bmWidthBytes;          
    unsigned short	bmPlanes;
    unsigned short  usDepth;
    unsigned int	ulHandler;

} DKBITMAP_old;   /* Depricated please ! */


/* This is the REAL BITMAP */
typedef struct tDKBITMAP {
	unsigned long	bmType;
	unsigned long	bmWidth;
	unsigned long	bmHeight;
	unsigned long	bmWidthBytes;
	unsigned short	bmPlanes;
	unsigned short	bmBitsPixel;
	void*       	bmBits;
} DKBITMAP;



#if !defined(DWORD)
#define  DWORD unsigned int
#endif

#if !defined(WORD)
#define WORD unsigned short
#endif




typedef struct DK_BITMAPINFOHEADER_t
{
    DWORD			biSize;
    DWORD			biWidth;
    DWORD			biHeight;
    WORD			biPlanes;
    WORD			biBitCount;
    DWORD			biCompression;
    DWORD			biSizeImage;
    DWORD			biXPelsPerMeter;
    DWORD			biYPelsPerMeter;
    DWORD			biClrUsed;
    DWORD			biClrImportant;

	DWORD			fccHandler;          /* hopefully this never matters */
	DWORD			dxFlavor;
} DK_BITMAPINFOHEADER;


static int DK_BITMAPINFOHEADER_REFLECT[ ]  = { 4,4,4,2,2, 4,4,4,4,4,4,4,4 };



#undef WORD
#undef DWORD




#if defined(__cplusplus)
}
#endif
#endif
