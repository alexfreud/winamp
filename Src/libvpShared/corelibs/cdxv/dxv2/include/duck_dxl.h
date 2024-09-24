#ifndef _duck_dxl_h
#define _duck_dxl_h


/******************************************************************************\
<table BGCOLOR=#FFC0C0 border=1 WIDTH=100% ><tr><td><b>                                                                              
  duck_dxl.h  </b></td><td><b> 	TrueMotion include file for decompression libraries </b>
                                                                           
</td></tr><tr><td>&nbsp</td><td>	Version:      6.0.0  
</td></tr><tr><td>&nbsp</td><td>  	Created:      3/3/98                                         
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
	DXMAX
} dxvBitDepth ;

#define DXRGB16	DXRGB16_555
#define DXRGB24CHAR DXRGB24

typedef enum OFFSETXY { 
	DXL_ABSOLUTE = 0, 
	DXL_RELATIVE 
} dxvOffsetMode;

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


enum IMAGETYPE { whgfw_X=0 };  /* MEW */
enum BGMODE     { kjhdkj_X=0 }; /* MEW */


/*********************************************************/

/* definition of data handles */                                         

typedef struct vScreen *DXL_VSCREEN_HANDLE;
typedef struct tXImage *DXL_XIMAGE_HANDLE;


/* main video decompression init, exit and query */


/*@
@Name 			DXL_InitVideo
@Description  		Initialize Video decompression services		
@Return value		DXL_OK on success.
@*/
int DXL_InitVideo(
void
);


/*@
@Name			DXL_ExitVideo
@Description		shutdown video decompression services.
@Return value		none
@*/
void DXL_ExitVideo(void); 


/*get pointer to NULL terminated 
  array of supported fourCCs */
unsigned int *DXL_GetFourCCList(void);


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
@Return value		handle to xImage created by this call .
@*/
DXL_XIMAGE_HANDLE DXL_CreateXImageOfType(
unsigned char *data,    	/* pointer to compressed data */
unsigned int fccType		/* FOURCC style code indicating type of compressed data */
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
@Description	feed the xImage new data, get ready to decompress 	
@Return value  	DXL_OK on success
@*/
int DXL_AlterXImageData(		
DXL_XIMAGE_HANDLE src,		/* xImage, handle to compressed data */			
unsigned char *ptrData		/* latest data to be associated with xImage */
);



/*@
@Name 		DXL_AlterXImage
@Description    explicitly alter attributes of an xImage 
@Return value 	handle to compressed image
@*/
DXL_XIMAGE_HANDLE DXL_AlterXImage(
DXL_XIMAGE_HANDLE src,		/* handle to compressed image */
unsigned char *ptrData,		/* pointer to compressed data. */
int xImType,			/* code for compress data type. */
dxvBitDepth bitDepth ,    	/* bitdepth of decompressed data */
int maxWidth,			/* width of decompressed image */
int maxHeight 			/* height of decompressed image */
);


unsigned char * 
DXL_GetXImageCDataAddr(DXL_XIMAGE_HANDLE src);



/*@
@Name 			DXL_GetXImageCSize
@Description 		Get xImage compressed size
@Return value   	returns the compressed size
@*/
int DXL_GetXImageCSize(
DXL_XIMAGE_HANDLE src		/* handle to compressed image */
);



/*@
@Name			DXL_GetXImageXYWH
@Description  		get application specified x,y offset, and overall decompressed width and height. 
x and y offsets are legacy fields, ignore. 		
@Return value  		DXL_OK on success	
@*/
int DXL_GetXImageXYWH(
	DXL_XIMAGE_HANDLE src,			/* the xImage Handle. */
	int *x,int *y,int *w, int *h		/* x,y,w,h */
	);


/*@
@Name 			DXL_IsXImageKeyFrame
@Description   		return whether this xImage is a keyFrame.
@Return value  		return whether this xImage is a keyFrame.
@*/
int DXL_IsXImageKeyFrame(
	DXL_XIMAGE_HANDLE src   /* handle to compressed image */
);



/*@
@Name 			DXL_dxImageToVScreen
@Description		decompress and blit as a single process 
@Return value		DXL_OK on success.
@*/
int DXL_dxImageToVScreen(
	DXL_XIMAGE_HANDLE src, 		/* xImage handle. */
	DXL_VSCREEN_HANDLE dst		/* handle to destination surface */
	);


/* vscreen management functions */

/*@
@Name 		DXL_CreateVScreen
@Description	create a virtual screen for rendering, storing decompressed video.		
@Return value	returns a DXL_VSCREEN_HANDLE
@*/
DXL_VSCREEN_HANDLE DXL_CreateVScreen(
	unsigned char *addr,		/* The address where pixel data should be written */
	dxvBitDepth colorMode, 		/* Determines the colorspace and color depth of VScreen */
	short bytePitch,		/* offset from one raster to the next */
	short height			/* number of rasters in a VScreen */
	);



/*@
@Name 		DXL_AlterVScreen
@Description	Alter address and attributes associated with a vscreen. 			
@Return value	returns a DXL_VSCREEN_HANDLE
@*/
int DXL_AlterVScreen(
	DXL_VSCREEN_HANDLE dst, 	/* handle to a VScreen */
	unsigned char *addr,		/* The address where pixel data should be written */
	dxvBitDepth colorMode,		/* Determines the colorspace and color depth of VScreen */
	int bytePitch,			/* offset from one raster to the next */
	int height			/* number of rasters in a VScreen */
	);
	

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


/* get attributes of the vScreen */
int DXL_GetVScreenAttributes(
    DXL_VSCREEN_HANDLE vScreen,
    void **addr, 
    dxvBlitQuality *bq, 
    dxvBitDepth *bd,
    short *pitch, 
    short *height
	);   

/* get vScreen's current viewport rectangle
	a viewport represents an x,y, offset and 
	a clipping width and height */        
int DXL_GetVScreenView(
	DXL_VSCREEN_HANDLE dst,
	int *x,int *y,int *w,int *h
	);

/* pass a parameter to the decompressor */
int  DXL_SetParameter(
	DXL_XIMAGE_HANDLE src, 
	int Command, 
	unsigned int Parameter 
	);

unsigned int DXL_GetXImageFOURCC(DXL_XIMAGE_HANDLE src);

/* Temporary hack to dxv to allow calls to get info (jbb) */
/*
typedef struct tFrameInfo
{
    int KeyFrame;
    int Version;
    int Quality;
    int vp30Flag;
} FrameInfo;
*/

#if defined(__cplusplus)
}
#endif

#endif /* include guards */
