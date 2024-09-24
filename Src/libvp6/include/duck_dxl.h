#ifndef _duck_dxl_h
#define _duck_dxl_h


#include "duck_bmp.h"


/******************************************************************************\
<table BGCOLOR=#FFC0C0 border=1 WIDTH=100% ><tr><td><b>                                                                              
  duck_dxl.h  </b></td><td><b> 	TrueMotion include file for decompression libraries </b>
                                                                           
</td></tr><tr><td>&nbsp</td><td>	Version:      6.0.0  
</td></tr><tr><td>&nbsp</td><td>  	Updated:      $Date: 2011/06/29 19:50:29 $                                        
</td></tr><tr><td>&nbsp</td><td>  	Copyright (c) 1994-98, The Duck Corp. All rights reserved.
</td></tr><tr><td>Important Objects</td><td>The On2 Decompression services tries to abstract the various objects
used to decompress and render both audio and video. This allows the overall API to flex and accomodate new 
decompression schemes and new destinations.
</td></tr><tr><td>DXL_XIMAGE_HANDLE</td><td>Abstract container object used to organize and control compressed
video.
</td></tr><tr><td>DXL_VSCREEN_HANDLE</td><td>Abstract container object used to organize and control display of
uncompressed video to a surface.
</td></tr><tr><td>DXL_XAUDIOSRC_HANDLE</td><td>Abstract container object used to organize and control 
compressed audio.
</td></tr><tr><td>DXL_AUDIODST_HANDLE</td><td>Abstract container object used to organize and control 
rendering / playing of uncompressed audio.
</td></tr>
</table>
******************************************************************************/

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
    DXI420 = 15, 
	
	DXYVYU = 16,

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
    DXL_HARDWARE_ERROR = -16002,
    DXL_HARDWARE_NOT_INITED = -16001,
    DXL_HARDWARE_BUFFER_FULL = -16000,
    DXL_INVALID_REQUEST = -9,
    DXL_VERSION_CONFLICT = -8,
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

/* main video decompression init, exit and query */


/*@
@Name 			DXL_InitVideo
@Description  		Initialize Video decompression services		
@Return value		DXL_OK on success. -1 unable to initialize library, insufficient memory available.
@*/
int DXL_InitVideo(
int maxScreens,		/* max Number of VScreens to allow. Outdated. Please pass zero ! */
int maxImages		/* max Number of xImages  to allow. Outdated. Please pass zero ! */
);




/*@
@Name			DXL_ExitVideo
@Description		Exit and de-initialize video decompression library. Release any allocated data structures. 
Always destroy xImages before calling this routine to avoid memory leaks.
@Return value		none
@*/
void DXL_ExitVideo(void); 


/*get pointer to NULL terminated 
  array of supported fourCCs */
unsigned long *DXL_GetFourCCList(void);


/*@
@Name  			DXL_CreateXImage
@Description 		Create an xImage (decompressor) object, based on the  compressed data provided.
@Return value 		returns a DXL_XIMAGE_HANDLE object ... also known as an xImage, or compressed image
@*/
DXL_XIMAGE_HANDLE DXL_CreateXImage(
unsigned char *data	/* compressed data */
); 




/*@
@Name			DXL_SetXImageCSize
@Description		Set the size of the current compressed frame		
@Return value		echo back the compressed image size		
@*/
int DXL_SetXImageCSize(
DXL_XIMAGE_HANDLE xImage, 	/* compressed image handle */
int compressedSize		/* compressed image size */
);




/*@
@Name 			DXL_CreateXImageOfType
@Description		Create an xImage (decompressor) object  of a requested type based on a FOURCC. 
Allocates buffers and initializes structures needed to decompress a source-compressed xImage.
@Return value		Handle to xImage created by this call , null if cannot create.
@*/
DXL_XIMAGE_HANDLE DXL_CreateXImageOfType(
unsigned char *data,    	/* pointer to compressed image data */
unsigned long fccType		/* FOURCC code indicating type of compressed image data */
);



/*@
@Name 			DXL_CreateXImageFromBMI
@Description		Create an xImage (decompressor) object  of a requested type based on a FOURCC. 
Allocates buffers and initializes structures needed to decompress a source-compressed xImage.
@Return value		Handle to xImage created by this call , null if cannot create.
@*/
DXL_XIMAGE_HANDLE DXL_CreateXImageFromBMI(
unsigned char *data,    					/* pointer to compressed image data */
unsigned long biCompression,				/* biCompression from BMIH */
DK_BITMAPINFOHEADER *srcAndDst				/* BMI data from AVI file or elsewhere */
);




/*@
@Name 		DXL_DestroyXImage 
@Description  	destroy the specified xImage
@Return value	void
@*/
void DXL_DestroyXImage(
DXL_XIMAGE_HANDLE src		/* handle to compressed image */
);



/*@
@Name 		DXL_AlterXImageData
@Description	Provides a compressed source with new data to decompress. New xImage attributes can be 
queried any time after changing the address of the compressed data with this function.	
@Return value  	DXL_OK on success or negative error code. -3 indicates that the pointer was passed in as null. 
Some compression applications (such as Adobe Premiere) use this to indicate that the new frame is the same as 
the previous frame, and the previous frame is to be held.
@*/
int DXL_AlterXImageData(		
DXL_XIMAGE_HANDLE src,		/* handle to compressed image source (xImage) */			
unsigned char *ptrData		/* pointer to compressed video data to be associated with xImage */
);



/*@
@Name 		DXL_AlterXImage
@Description    Explicitly alter attributes of an xImage. The use of this funtion 
may affect the state of the xImage's frame buffer. During interframe compression, this can result 
in corruption of the decompressed image. Make sure to use this function only prior to decompressing a keyframe.
@Return value 	handle to compressed image, or null if error.
@*/
DXL_XIMAGE_HANDLE DXL_AlterXImage(
DXL_XIMAGE_HANDLE xImage,	/* handle to compressed image */
unsigned char *ptrData,		/* pointer to compressed video data. */
int xImType,			/* new xImage type (DXL_INTRAFRAME, DXL_INTERFRAME). */
dxvBitDepth bitDepth ,    	/* bitdepth of decompressed data */
int maxWidth,			/* width of decompressed image */
int maxHeight 			/* height of decompressed image */
);




/*@
@Name 			DXL_GetXImageCSize
@Description 		Get xImage compressed size
@Return value   	returns the compressed size
@*/
long DXL_GetXImageCSize(
DXL_XIMAGE_HANDLE src		/* handle to compressed image */
);



/*@
@Name			DXL_GetXImageXYWH
@Description  		get application specified x,y offset, and overall decompressed width and height. 
x and y offsets are legacy fields, ignore. 		
@Return value  		DXL_OK on success, or negative error code.	
@*/
int DXL_GetXImageXYWH(
	DXL_XIMAGE_HANDLE src,			/* the xImage Handle. */
	int *x,int *y,int *w, int *h		/* x,y,w,h - addresses for offsets and dimensions. */
	);


/*@
@Name 			DXL_IsXImageKeyFrame
@Description   		Check keyframe status of current image.
Use DXL_AlterXImageData to set the current frame and the use this call to detect keyframe status.
@Return value  		return 1 if this xImage is a keyFrame, 0 if not a keyframe. 
@*/
int DXL_IsXImageKeyFrame(
	DXL_XIMAGE_HANDLE src   /* handle to compressed image */
);



/*@
@Name 			DXL_dxImageToVScreen
@Description		Decompress and blit as a single process, according to current source and destination attributes. 
Passing 0 can be used to skip a blit in order to reduce CPU load as needed (synchronization).
@Return value		DXL_OK on success. 1 means place-holder frame. Negative means error. 
@*/
int DXL_dxImageToVScreen(
	DXL_XIMAGE_HANDLE xImage, 		/* Handle to compresse image source (xImage). */
	DXL_VSCREEN_HANDLE dst			/* Handle to destination surface (vScreen). Null means decompress without blit. */
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

/*@
@Name 		DXL_CreateVScreen
@Description	Create a virtual screen for rendering, storing decompressed video.		
@Return value	returns a DXL_VSCREEN_HANDLE, or null if none available.
@*/
DXL_VSCREEN_HANDLE DXL_CreateVScreen(
	unsigned char *addr,		/* The address where pixel data should be written */
	dxvBitDepth colorMode, 		/* Determines the colorspace and color depth of VScreen */
	short bytePitch,		/* Offset from one raster to the next measured in bytes. */
	short height			/* Number of rasters in a VScreen */
	);



/*@
@Name 		DXL_AlterVScreen
@Description	Alter address and attributes associated with a vscreen. 			
@Return value	0 for success or negatibe error code.
@*/
int DXL_AlterVScreen(
	DXL_VSCREEN_HANDLE dst, 	/* handle to a VScreen */
	unsigned char *addr,		/* The address where pixel data should be written, or null for no change. */
	dxvBitDepth colorMode,		/* Determines the colorspace and color depth of VScreen, or -1 for no change. */
	int bytePitch,			/* offset from one raster to the next measured in bytes, or 0 for no change. */
	int height			/* number of rasters in a VScreen, or 0 for no change.  */
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


/*@
@Name		DXL_DestroyVScreen
@Description	Destroy a vScreen object/struct.
@Return value	None
@*/
void DXL_DestroyVScreen(
	DXL_VSCREEN_HANDLE dst		/* handle to virtual screen destination */
);




/*@
@Name		DXL_SetVScreenBlitQuality
@Description 	set blit-quality of a vScreen same (normal), stretch (black lined) 
stretch bright (stretched w/interpolation)
@Return value	return prior blit-quality value.
@*/    
int DXL_SetVScreenBlitQuality(
	DXL_VSCREEN_HANDLE dest,	/* handle to vScreen */
	dxvBlitQuality bq		/* new blit-quality value */
);




/*@
@Name		DXL_GetVScreenBlitQuality
@Description	Get vScreens current blit-quality. Blit-quality determines if and how stretching should occur during the blit.
@Return value	returns member of enum called dxvBlitQuality, blit-quality value. 
BLIT_SAME is direct transfer; BLIT_STRETCH does double wide pixels and raster skipping; BLIT_STRETCH_BRIGHT stretches in both horizontal and vertical directions.
@*/
dxvBlitQuality DXL_GetVScreenBlitQuality(
	DXL_VSCREEN_HANDLE	/* handle to vScreen. */
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
    dxvBitDepth *bd,
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

/* pass a parameter to the decompressor */
void DXL_SetParameter(
	DXL_XIMAGE_HANDLE src, 
	int Command, 
	unsigned long Parameter 
	);

/* can only have a max of 32 cpu specific features */
typedef enum tCPU_FEATURES 
{
    NO_FEATURES = 0,
    MMX_SUPPORTED = 1
} CPU_FEATURES;

CPU_FEATURES DXL_GetCPUFeatures(void);
unsigned long DXL_GetXImageFOURCC(DXL_XIMAGE_HANDLE src);


/* pass a parameter to the decompressor */
void DXL_SetParameter(
	DXL_XIMAGE_HANDLE src, 
	int Command, 
	unsigned long Parameter 
	);



/* Temporary hack to dxv to allow calls to get info (jbb) */
typedef struct tFrameInfo
{
    int KeyFrame;
    int Version;
    int Quality;
    int vp30Flag;
} FrameInfo;





/* define this in case we need to interogate before bailing out */
typedef struct bmiChunk_t
{
	unsigned long biCompression;
	unsigned char biBitCount;
	unsigned char biPlanes;
	dxvBitDepth bd;

} BMIMapping;


extern BMIMapping DXL_BMIMap[];


#if !defined(DXL_MKFOURCC)
#define DXL_MKFOURCC( ch0, ch1, ch2, ch3 ) \
		( (unsigned long)(unsigned char)(ch0) | ( (unsigned long)(unsigned char)(ch1) << 8 ) |    \
		( (unsigned long)(unsigned char)(ch2) << 16 ) | ( (unsigned long)(unsigned char)(ch3) << 24 ) )

#endif


/* src xImage must actually be a DXL_CODEC_HANDLE */
/* you will need a dxvvfw.lib in order to utilize this prototype for now */
int DXL_ReportBestBMIMatch(DXL_XIMAGE_HANDLE src, BMIMapping** map, int *maxMaps, int doConsoleReport);

/* have DXV print DXV/ICM mapping to HTML table */
void DXL_ReportBMIMapping(char *filename);




void vp31_GetInfo(unsigned char * source, FrameInfo * frameInfo);

#if defined(__cplusplus)
}
#endif

#endif /* include guards */
