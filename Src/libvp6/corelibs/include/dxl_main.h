//==========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1999 - 2001  On2 Technologies Inc. All Rights Reserved.
//
//--------------------------------------------------------------------------


#ifndef _dxl_main_h
#define _dxl_main_h

#if defined(__cplusplus)
extern "C" {
#endif

struct vScreen;
struct tXImage;

struct tAudioBuff;
struct tXAudioSource;

#if defined(__cplusplus)
}
#endif




#if defined(_WIN32_WCE)
#ifndef NULL
#define NULL 0
#endif
#endif

#include "dkpltfrm.h"
//#include "duktypes.h"
#include "duck_dxl.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define TMRTType 5

#define validate(x) {if (!x) return (int ) DXL_NULLSOURCE; if (!x->dkFlags.inUse) return (int ) DXL_NOTINUSE ;}

typedef void *blitFunc;   
/*typedef void (*blitFunc2)(DXL_XIMAGE_HANDLE,DXL_VSCREEN_HANDLE);   */

typedef int DXL_BLIT_FORMAT;

enum DKOBJECTTYPE { 
    DXUNUSED = 0, 
    DXXIMAGE = 1, 
    DXVSCREEN = 2
};

enum COLORDEPTH { 
    PALETTE8	= 0, 
    RGB555		= 1, 
    RGB555A		= 2, 
    RGB888		= 3,
    RGBA8888	= 4
};

typedef struct tagflgs {
	unsigned inUse : 1;
	unsigned DXed : 1;
	unsigned clutOwner: 1;
	unsigned doCompleteBlit : 1;
	unsigned keyFrame : 1;
	unsigned nullFrame : 1;
	unsigned interframe : 1;
	unsigned logo : 1;
	unsigned allocated : 1;
} dkInfoFlags;

typedef struct vflgs {
	unsigned clipped : 1;
	unsigned showInfoDots : 1;
} vFlags;

typedef struct frameheader {
	unsigned char	hdrSize;
	unsigned char	Type;
	unsigned char	DeltaSet;
	unsigned char	Table;
	unsigned short Ysize;
	unsigned short Xsize;
	unsigned short	CheckSum;
	unsigned char	CVersion;
	unsigned char	metaType;
	unsigned char	Frameinfo;
	unsigned char	Control;
	unsigned short xoff,yoff,width,height;
} FRAMEHEADER;

typedef struct DXINFOSTRUCT{                     
	int imwidth;
	int imheight; 
	int blockpower;
	int lpbmione;
	int block2x;
	unsigned char *vectbl;
	int hinterp;
	int interframe;
	int iskeyframe;
	int sprite;
	int bitcnt;
	int hdrSize; 
	int drawing;
	int fmt;          
	FRAMEHEADER f;
	int algorithm;
} dxInfoStruct;

/*
	base "class" for xImage(s):

  	enum DKOBJECTTYPE dkObjectType; // type of object
	dkInfoFlags dkFlags;			// universal flags
	enum COLORDEPTH cDepth;			// colorDepth
	short imWidth,imHeight;			// internal width & height
	short x,y,w,h;					// location and dx'd dimensions
	unsigned char *addr;			// pointer to compressed data
	DXL_VSCREEN_HANDLE lVScreen;	// last know destination
	DXL_XIMAGE_HANDLE (*create)(void);		// creator (constructor)
	DXL_XIMAGE_HANDLE (*recreate)(void);	// recreate base w/h/type/etc.
	int (*destroy)(void);			// destroyer (destructor)
	int (*seedData)(void);			// reseed with new compressed data
	int (*dx)(void);				// decompress (to vScreen)
	int (*blit)(void);				// blit from internal state
*/

/*
	char *(*perfStats)(DXL_XIMAGE_HANDLE, char *storage); \
*/

typedef struct profilePack_t
{
	UINT64 dxClocks;
	UINT64 profileStart;
	UINT64 profileEnd;
	int frameCount;

} DXL_PROFILEPACK;


#define xImageBaseStruct \
	enum DKOBJECTTYPE dkObjectType; \
	dkInfoFlags dkFlags; \
	enum COLORDEPTH colorDepth; \
	short imWidth,imHeight; \
	short x,y,w,h; \
	unsigned char *addr; \
	DXL_VSCREEN_HANDLE lVScreen; \
	enum BITDEPTH *bdPrefs; \
	DXL_XIMAGE_HANDLE (*create)(void *); \
	DXL_XIMAGE_HANDLE (*recreate)(DXL_XIMAGE_HANDLE,void *,int,int,int,int); \
	int (*destroy)(DXL_XIMAGE_HANDLE); \
	int (*seedData)(DXL_XIMAGE_HANDLE); \
	int (*dx)(DXL_XIMAGE_HANDLE, DXL_VSCREEN_HANDLE); \
	int (*blit)(DXL_XIMAGE_HANDLE, DXL_VSCREEN_HANDLE); \
    int (*internalFormat)(DXL_XIMAGE_HANDLE, DXL_VSCREEN_HANDLE); \
    int (*verify)(DXL_XIMAGE_HANDLE, DXL_VSCREEN_HANDLE); \
	int fSize; \
	long (*GetXImageCSize)(DXL_XIMAGE_HANDLE); \
	void *(*getFrameBuffer)(DXL_XIMAGE_HANDLE); \
	void (*setParameter)(DXL_XIMAGE_HANDLE, int , unsigned long );\
	DXL_PROFILEPACK prof
	


typedef struct tXImage{  
	xImageBaseStruct;
} DXL_XIMAGE;

typedef struct tXImage1{  
	xImageBaseStruct;

	/********** TM1 specific follows **********/
	enum IMAGETYPE imType; 

	unsigned char *lineBuffer;  
	int lineBufferSize;
	
	unsigned long *chromaBuffer;
	int chromaBufferSize;

	short dxCount; /* number of lines left to decompress */
	short lw,lh;

	enum BGMODE sprMode;
	short sprColor;	/* sprite mode and color for blending */

    dxInfoStruct dxInfo;
} DXL_XIMAGE_1,*DXL_XIMAGE_1HANDLE;

typedef struct vScreen{
	enum DKOBJECTTYPE dkObjectType;
	unsigned char *addr,*laddr;	/* address of destination and what it was the last time */
	unsigned char *bAddr,*bOffsetAddr;	/* address of sprite background */
	enum BITDEPTH bd;		/* format of destination */
	enum BLITQUALITY bq;	/* blit translation mode */
	short pitch,height;		/* pitch and height of dest */        
	short bx,by,bPitch;		/* x,y, and pitch of background */        
	short viewX,viewY;		/* offset/clipping viewport within destination */
	short viewW,viewH;
	short clipX,clipY;		/* clipping rectangle within viewport */
	short clipW,clipH;
	dkInfoFlags dkFlags;
	DXL_XIMAGE_HANDLE lXImage; /* last XImage decompressed here, useful for smart blitting */
	unsigned char *clut1,*clut2;
	DXL_BLIT_FORMAT blitFormat;  

    void *blitSetup;
    void *blitter;
    void *blitExit;

	int vesaMode;
	unsigned char *drawAddr;
	short drawW,drawH;
	vFlags flags;

} DXL_VSCREEN;

/* private functions */
int decodeHeader(void *data,register dxInfoStruct *dxInfo);


#define MAX_CDEPTHS DXMAX
#define MAX_BQUALITIES DXBLITMAX

typedef enum tDXL_INTERNAL_FORMAT {
	DXL_NULL_IFORMAT = -1,
	DXL_LINE16 = 0,
	DXL_LINE16i = 1,
	DXL_LINE16hi = 2,
	DXL_LINE16spr = 3,
	DXL_LINE8 = 4,
    TM2_BLOCK24 = 5,
    TM1_24 = 6,
    TORQ_YUY2 = 7,
    TORQ_YUY2hi = 8,
    YV12 = 9,
    SWET_YUV = 10,
	DXL_MAX_IFORMATS
} DXL_INTERNAL_FORMAT;

DXL_BLIT_FORMAT DXL_ReserveBlitter(void);
DXL_BLIT_FORMAT DXL_OverrideBlitter(enum BLITQUALITY bq,enum BITDEPTH bd);
int DXL_RegisterBlitter(DXL_BLIT_FORMAT dFormat, DXL_INTERNAL_FORMAT , 
						blitFunc blit, blitFunc setup, blitFunc exit);

blitFunc DXL_GetBlitFunc(DXL_XIMAGE_HANDLE ,DXL_VSCREEN_HANDLE );
blitFunc DXL_GetBlitSetupFunc(DXL_XIMAGE_HANDLE ,DXL_VSCREEN_HANDLE );
blitFunc DXL_GetBlitExitFunc(DXL_XIMAGE_HANDLE ,DXL_VSCREEN_HANDLE );

blitFunc DXL_GetVBlitFunc(DXL_VSCREEN_HANDLE ,DXL_VSCREEN_HANDLE );
blitFunc DXL_GetVBlitSetupFunc(DXL_VSCREEN_HANDLE ,DXL_VSCREEN_HANDLE );
blitFunc DXL_GetVBlitExitFunc(DXL_VSCREEN_HANDLE ,DXL_VSCREEN_HANDLE );

DXL_BLIT_FORMAT DXL_GetVScreenBlitFormat(DXL_VSCREEN_HANDLE );
DXL_INTERNAL_FORMAT DXL_GetXImageInternalFormat(DXL_XIMAGE_HANDLE ,DXL_VSCREEN_HANDLE );

DXL_INTERNAL_FORMAT DXL_GetVScreenInternalFormat(DXL_VSCREEN_HANDLE vScreen);

int dxl_GetAlgHandle(unsigned long fourcc);
int dxl_RegisterInternalFormat(int xHandle, DXL_INTERNAL_FORMAT xFormat);

int DXL_VScreenInfoDots(DXL_XIMAGE_HANDLE src, DXL_VSCREEN_HANDLE dst);

int DXL_GetVScreenSizeOfPixel(DXL_VSCREEN_HANDLE );
unsigned char *DXL_GetDestAddress(DXL_XIMAGE_HANDLE src, DXL_VSCREEN_HANDLE dst);

int DXL_GetXImageOffset(DXL_XIMAGE_HANDLE,int *,int *);

typedef DXL_XIMAGE_HANDLE (*createFunc)(unsigned char *data);
int DXL_RegisterXImage(createFunc creator,unsigned long fourcc,DXL_INTERNAL_FORMAT iForm);

void registerDuckBlitters(void);
void resetBlitters(void);

void dxv_logo( unsigned char *dst, int width, int height, int pitch,
					   enum BITDEPTH format, int dci, enum BLITQUALITY bq);

void dxl_24c(void *compaddr, void *scrnaddr,
	int dstPitch,int iskeyframe,
	int hinterp,int doublesize,
	int scrnwidth,int scrnheight,
	int blockpower, int block2x,
	int forceheight, char *lastdecomp,
	char *lastcdecomp);

#define DXL_MKFOURCC( ch0, ch1, ch2, ch3 ) \
		( (unsigned long)(unsigned char)(ch0) | ( (unsigned long)(unsigned char)(ch1) << 8 ) |    \
		( (unsigned long)(unsigned char)(ch2) << 16 ) | ( (unsigned long)(unsigned char)(ch3) << 24 ) )

#if defined(__cplusplus)
}
#endif

#endif
