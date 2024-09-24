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


#ifndef _duck_dxl_h
#define _duck_dxl_h


/******************************************************************************\
*                                                                              *
* duck_dxl.h  -   TrueMotion include file for decompression libraries          *
*                                                                              *
*                 Version:      6.0.0                                          *
*                 Created:      3/3/98                                         *
*                                                                              *
\******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/* enumerated data types */

typedef enum BLITQUALITY {
	DXBLIT_SAME = 0,        /* Blit directly, w/o stretching */
	DXBLIT_R1,
	DXBLIT_R2,
	DXBLIT_STRETCH,         /* double horizontally, skip lines vertically */
	DXBLIT_R3,
	DXBLIT_STRETCH_BRIGHT,  /* double horizontally, interpolate vertically */
	DXBLIT_R4,
	DXBLIT_R5,
	DXBLIT_R6,
	DXBLIT_NONE,
	DXBLITMAX
} dxvBlitQuality ;

typedef enum BITDEPTH { 
    DXRGBNULL = 0, 
    DXRGB8 = 1, 
    
	DXRGB16_555 = 2, 
    DXRGB24 = 3, 
    DXRGB_UNUSED = 4,
    DXRGB16VESA = 5,
    DXRGB8VESA = 6,
    DXRGB16_565 = 7,

    DXYUY2 = 8, 
    DXYVU9 = 9, 
    DXYV12 = 10, 
    DXUYVY = 11, 
    
	DXRGB32 = 12, 
    DXRGB16VESA_565 = 13, 
	DXHALFTONE8 =14,
	DXMAX
} dxvBitDepth ;

#define DXRGB16	DXRGB16_555
#define DXRGB24CHAR DXRGB24

typedef enum OFFSETXY { 
	DXL_ABSOLUTE = 0, 
	DXL_RELATIVE 
} dxvOffsetMode;

typedef enum IMAGETYPE {
	DXL_INTRAFRAME = 0, 
	DXL_INTERFRAME, 
	DXL_SPRITE 
} dxvImageType;

typedef enum DXL_ERR{
    DXL_LOW_ERR = -32000,
    DXL_INVALID_REQUEST = -8,
    DXL_VERSION_CONFLICT = -7,
    DXL_INVALID_DATA = -7,
    DXL_INVALID_BLIT = -6,
    DXL_BAD_DATA = -5,
    DXL_ALLOC_FAILED = -4,
    DXL_NULL_FRAME = -3, 
    DXL_NULLSOURCE = -2, 
    DXL_NOTINUSE = -1, 
    DXL_OK = 0,
    DXL_HOLD_FRAME = 1
} dxvError ;

typedef enum BGMODE   /* sprite drawing modes 
                         v1.0.2 supports NORM & NO_BACKGROUND */{ 
    NORM = 0,         /* normal sprite mode, blend edges w/background */
    NO_BACKGROUND = 1,/* transparant sprite mode 1, 
                         sets all background data transparant no blending */
    NORM_TRANS = 2,   /* transparant sprite mode 2, 
                         blend edges (alphas) w/separate background buffer, 
                         set sprite background to trans */
    RGB_OPAQUE = 3,   /* blend edges to sprColor, set background to sprColor*/
    RGB_TRANS = 4     /* blend edges w/sprColor, set background to trans */ 
} dxvBackgroundMode ;

/*********************************************************/

/* definition of data handles */                                         

typedef struct vScreen *DXL_VSCREEN_HANDLE;
typedef struct tXImage *DXL_XIMAGE_HANDLE;
typedef struct tXAudioSource *DXL_XAUDIOSRC_HANDLE;
typedef struct tAudioBuff *DXL_AUDIODST_HANDLE;

/* main video decompression init, exit and query */

int DXL_InitVideo(int maxScreens,int maxImages);
void DXL_ExitVideo(void); 

/*get pointer to NULL terminated 
  array of supported fourCCs */
unsigned long *DXL_GetFourCCList(void);

/* create an xImage (decompressor) object,
	based on the provided compressed data */
DXL_XIMAGE_HANDLE DXL_CreateXImage(
	unsigned char *data
	); 

int DXL_SetXImageCSize(DXL_XIMAGE_HANDLE xImage, int compressedSize);
int DXL_InitVideoEx(int lmaxScreens, int lmaxImages);

/* create an xImage (decompressor) object 
    of a specific type */
DXL_XIMAGE_HANDLE DXL_CreateXImageOfType(
	unsigned char *data,
	unsigned long fccType
	);

/* destroy the specified xImage */
void DXL_DestroyXImage(DXL_XIMAGE_HANDLE src);

/* feed the xImage new data, get ready to decompress */
int DXL_AlterXImageData(
	DXL_XIMAGE_HANDLE src,
	unsigned char *ptrData
	);

/* explicitly alter attributes of an xImage */
DXL_XIMAGE_HANDLE DXL_AlterXImage(
	DXL_XIMAGE_HANDLE ,
	unsigned char *ptrData,
	int xImType,
	dxvBitDepth bitDepth ,
	int maxWidth,int maxHeight 
	);

/* get xImage target bounding rectangle */
int DXL_GetXImageDirtyXYWH(
	DXL_XIMAGE_HANDLE ,
	int *,int *,int *,int *
	);

/* force decompressor to refresh all destination pixels */
int DXL_SetXImageBlitAll(
	DXL_XIMAGE_HANDLE xImage
	);

/* if available, get pointer to internal frameBuffer */
void *DXL_GetXImageFrameBuffer(
	DXL_XIMAGE_HANDLE src
	);

long DXL_GetXImageCSize(
	DXL_XIMAGE_HANDLE src
	);

/* change x,y, offset at which to decompress image */
int DXL_MoveXImage(
	DXL_XIMAGE_HANDLE src,
	dxvOffsetMode mode, 
	int x,int y
	);

/* set sprite transparency mode (SegaSaturn only) */
int DXL_AlterXImageSpriteModes(
	DXL_XIMAGE_HANDLE src,
	int sprMode, int sprColor
	);

/* get application specified x,y offset, 
	and overall decompressed width and height */
int DXL_GetXImageXYWH(
	DXL_XIMAGE_HANDLE src,
	int *x,int *y,int *w, int *h
	);

int DXL_GetXImageType(DXL_XIMAGE_HANDLE src);
int DXL_GetXImageColorDepth(DXL_XIMAGE_HANDLE src);
int DXL_IsXImageKeyFrame(DXL_XIMAGE_HANDLE src);

/* decompress and blit as a single process */
int DXL_dxImageToVScreen(
	DXL_XIMAGE_HANDLE src, 
	DXL_VSCREEN_HANDLE dst
	);

/* compatibility check prior between 
	decompressor and destination */
int DXL_CheckdxImageToVScreen(
	DXL_XIMAGE_HANDLE src,
	DXL_VSCREEN_HANDLE dst
	);

/* blit from xImage internal "working area" to vScreen */
int DXL_BlitXImageToVScreen(
	DXL_XIMAGE_HANDLE src, 
	DXL_VSCREEN_HANDLE dst
	);

/* vscreen management functions */

/* create a virtual screen target for decompression (renderer?) */
DXL_VSCREEN_HANDLE DXL_CreateVScreen(
	unsigned char *addr,
	dxvBitDepth colorMode, 
	short bytePitch,
	short height
	);

/* alter attributes of an existing vScreen */
int DXL_AlterVScreen(
	DXL_VSCREEN_HANDLE dst, 
	unsigned char *addr,
	dxvBitDepth colorMode,
	int bytePitch,
	int height
	);
	

void DXL_VScreenSetInfoDotsFlag(DXL_VSCREEN_HANDLE vScreen, int showDots);
	

/* alter clipping rectangle of vScreen */
/* not supported by all decompressors */
int DXL_AlterVScreenClip(
	DXL_VSCREEN_HANDLE dst,
	int x,int y,
	int w,int h
	);

/* alter viewport rectangle of vScreen */
/* width/height not supported by all decompressors */
int DXL_AlterVScreenView(
	DXL_VSCREEN_HANDLE dst,
	int x,int y,
	int w,int h
	);

/* destroy a vScreen object/struct */
void DXL_DestroyVScreen(
	DXL_VSCREEN_HANDLE dst
	);

/* set blit mode/quality of a vScreen 
   same (normal), stretch (black lined)
   stretch bright (stretched w/interpolation) */    
int DXL_SetVScreenBlitQuality(
	DXL_VSCREEN_HANDLE dest,
	dxvBlitQuality bq
	);

/* get vScreens current blit mode/quality */
dxvBlitQuality DXL_GetVScreenBlitQuality(
	DXL_VSCREEN_HANDLE
	);

/* alter spite background associated with a vscreen */
/* used only by SegaSaturn for hardware sprite support */
int DXL_AlterVScreenBackground(
	DXL_VSCREEN_HANDLE ,
	unsigned char *,
	dxvBitDepth bd ,int ,int ,int ,int 
	);

/* set DOS VESA mode for vScreen (DOS only) */
int DXL_AlterVScreenVESAMode(
	DXL_VSCREEN_HANDLE ,
	int vesaMode
	);

/* set physical screen to vScreen's vesa mode */
int DXL_ActivateVScreenVESAMode(DXL_VSCREEN_HANDLE);

/* get vScreen (generally physical) vesa mode */
int DXL_GetVScreenVESAMode(DXL_VSCREEN_HANDLE );

void DXL_EraseVScreen(DXL_VSCREEN_HANDLE );
void DXL_BlackLineVScreen(DXL_VSCREEN_HANDLE );

/* copy one vScreen to another */
/* provides support for offscreen compositing,
   16 bit and 8 bit modes only */
int DXL_BlitVScreenToVScreen(
	DXL_VSCREEN_HANDLE fromVScreen,
	DXL_VSCREEN_HANDLE toVScreen
	);

/* get attributes of the vScreen */
int DXL_GetVScreenAttributes(
    DXL_VSCREEN_HANDLE vScreen,
    void **addr, 
    dxvBlitQuality *bq, 
    short *pitch, 
    short *height
	);   

char *DXL_GetXImageStats(DXL_XIMAGE_HANDLE xImage,char *storage);


/* get vScreen's current viewport rectangle
	a viewport represents an x,y, offset and 
	a clipping width and height */        
int DXL_GetVScreenView(
	DXL_VSCREEN_HANDLE dst,
	int *x,int *y,int *w,int *h
	);

/* get vScreen's current clipping rectangle */        
int DXL_GetVScreenClip(
	DXL_VSCREEN_HANDLE dst,
	int *x,int *y,int *w,int *h
	);

/* provide Color lookup tables for 8 bit support */
int DXL_SetVScreenCLUTs(
	DXL_VSCREEN_HANDLE vScr, 
	unsigned char *clpt, 
	unsigned char *clpt2,
	int exp
	);

/* return the palette currently used */
int DXL_GetBitDepthPalette(dxvBitDepth colorMode,
	unsigned char **pal);

/* relinquish color lookup table structures */
void DXL_ResetVScreenCLUTs(
	DXL_VSCREEN_HANDLE vScr
	);

/* check to see if a blit mode is supported */

int DXL_CheckVScreenBlit(DXL_VSCREEN_HANDLE dst,unsigned long fourcc);
int DXL_CheckVScreenXImageBlit(DXL_VSCREEN_HANDLE dst,DXL_XIMAGE_HANDLE src);

/* audio function prototypes */

int DXL_InitAudio(int srcs,int dsts);
void DXL_ExitAudio(void);

typedef struct tDKWAVEFORM *DKWAVEFORMPTR; /* place holder pointer */

/* create a compressed audio source (decompressor) */
DXL_XAUDIOSRC_HANDLE DXL_CreateXAudioSrc(
	DKWAVEFORMPTR wv, 
	unsigned char *addr,int length
	);

/* feed an audio decompressor new data (w/o decompressing) */
void DXL_AlterXAudioData(
	DXL_XAUDIOSRC_HANDLE xSource,
	unsigned char *addr,int length
	);

/* destroy an audio decompressor */
void DXL_DestroyXAudioSrc(DXL_XAUDIOSRC_HANDLE xSource);
        
/* create a audio destination description */
DXL_AUDIODST_HANDLE DXL_CreateAudioDst(
    void *addrL,void *addrR,
    int length,int bitDepth,
    int numChannels,
    int sampleRate
	);

/* change characteristics of audio destination descriptor */
void DXL_AlterAudioDst(
    DXL_AUDIODST_HANDLE dst,
    void *addrL,void *addrR,
    int length,int bitDepth,
    int numChannels,
    int sampleRate
	);

/* destroy audio destination description struct */
void DXL_DestroyAudioDst(DXL_AUDIODST_HANDLE dst);

/* decompress up to maxSamplesToDecompress */
/* function returns the actual number of 
	samples decompressed */
int DXL_dxAudio(
	DXL_XAUDIOSRC_HANDLE src,
	DXL_AUDIODST_HANDLE dst,
	int maxSamplesToDecompress
	);

/* windows 95 dll system abstraction functions */

/* set memory allocator function */
void DXV_Setmalloc(
	void *(*mallocFuncPtr)(unsigned int size)
	);

/* set cleared memory allocator function */
void DXV_Setcalloc(
	void *(*callocFuncPtr)(unsigned int size, unsigned int number)
	);

/*set memory free function */
void DXV_Setfree(
	void (*freeFuncPtr)(void *)
	);


/* can only have a max of 32 cpu specific features */
typedef enum tCPU_FEATURES 
{
    NO_FEATURES = 0,
    MMX_SUPPORTED = 1
} CPU_FEATURES;

CPU_FEATURES DXL_GetCPUFeatures(void);
unsigned long DXL_GetXImageFOURCC(DXL_XIMAGE_HANDLE src);


void vp31_SetParameter(DXL_XIMAGE_HANDLE src,int Command, unsigned long Parameter );



//int DXL_RegisteredAudioDXerGet(DXL_AUDIO_REGISTRATION *oneRegistration);

typedef struct tFrameInfo
{
    int KeyFrame;
    int Version;
    int Quality;
    int vp30Flag;
} FrameInfo;

void vp31_GetInfo(unsigned char * source, FrameInfo * frameInfo);

#if defined(__cplusplus)
}
#endif

#endif /* include guards */
