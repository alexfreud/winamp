/****************************************************************************
*
*   Module Title :     vp60dxv.c
*
*   Description  :     Defines the entry point for the console application.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include <math.h>   // For Abs()
#include "pbdll.h"

#include "duck_mem.h" /* interface to memory manager */
#include "dxl_plugin.h" /* interface to dxv */
#include "duck_dxl.h"

#include <stddef.h>

/****************************************************************************
*  Macros
****************************************************************************/
#ifdef _MSC_VER 
#pragma warning(disable:4055)
#endif 

#define VP60_FOURCC DXL_MKFOURCC( 'V', 'P', '6', '0')
#define VP61_FOURCC DXL_MKFOURCC( 'V', 'P', '6', '1')
#define VP62_FOURCC DXL_MKFOURCC( 'V', 'P', '6', '2')
extern int VPX_GetSizeOfPixel(dxvBitDepth bd);
extern void *VPX_GetBlitter(dxvBlitQuality bq, dxvBitDepth bd);

/****************************************************************************
*  Typedefs
****************************************************************************/
typedef unsigned long FourCC;

typedef struct  // YUV buffer configuration structure
{
	int   YWidth;
	int   YHeight;
	int   YStride;

	int   UVWidth;
	int   UVHeight;
	int   UVStride;

	char *YBuffer;
	char *UBuffer;
	char *VBuffer;

	char *uvStart;
	int   uvDstArea;
	int   uvUsedArea;
} DXV_YUV_BUFFER_CONFIG;

/* define an xImage structure based on the core xImage struct */
typedef struct tXImageCODEC
{
	FourCC myFourCC;
	DXV_YUV_BUFFER_CONFIG FrameBuffer;
	PB_INSTANCE *myPBI;
	int owned;
	int decompressedOnce;

} vp60_XIMAGE, *vp60_XIMAGE_HANDLE;

typedef void ((*vp6BLIT_FUNC)(unsigned char *, int, YUV_BUFFER_CONFIG *));
//typedef void ((*vp6_VSCREEN_FUNC)(void));

/****************************************************************************
*  Modul Statics
****************************************************************************/

/****************************************************************************
*  Forward declarationss
****************************************************************************/
void vp60_SetParameter(DXL_XIMAGE_HANDLE src,int Command, uintptr_t Parameter );

/****************************************************************************
*  Imports
****************************************************************************/
extern void VP6_VPInitLibrary(void);
extern void VP6_VPDeInitLibrary(void);
extern void VP6_readTSC(unsigned long *tsc);

int vp60_getWH(DXL_XIMAGE_HANDLE src, int *w, int *h)
{
	vp60_XIMAGE_HANDLE thisAlgorithmBase = (vp60_XIMAGE_HANDLE)DXL_GetAlgorithmBasePtr(src);
	*w = thisAlgorithmBase->myPBI->Configuration.VideoFrameWidth;
	*h = thisAlgorithmBase->myPBI->Configuration.VideoFrameHeight;
	return DXL_OK;
}
#if 0
/****************************************************************************
* 
*  ROUTINE       :     vp60_GetInfo
*
*  INPUTS        :     unsigned char *source :
*                      
*  OUTPUTS       :     FrameInfo *frameInfo  :
*
*  RETURNS       :     void
*
*  FUNCTION      :     
*
*  SPECIAL NOTES :     None. 
*
****************************************************************************/
void vp60_GetInfo ( unsigned char *source, FrameInfo *frameInfo )
{
	// Is the frame and inter frame or a key frame 
	frameInfo->KeyFrame = !(source[0] > 0x7f);
	frameInfo->Quality = source[0] >> 2;
	if ( frameInfo->KeyFrame )
		frameInfo->Version = ((source[2]>>3) & 0x1f );
	else
		frameInfo->Version = 0;

	frameInfo->vp30Flag = (int)source[1];
}
#endif

/****************************************************************************
* 
*  ROUTINE       :  vp60_decompress
*
*  INPUTS        :  vp60_XIMAGE_HANDLE src     :
*                   DXL_VSCREEN_HANDLE vScreen :
*
*  OUTPUTS       :  None.
*
*  RETURNS       :  int:
*
*  FUNCTION      :  
*
*  SPECIAL NOTES :  None. 
*
****************************************************************************/
int vp60_decompress ( DXL_XIMAGE_HANDLE src)
{
	vp60_XIMAGE_HANDLE thisAlgorithmBase = (vp60_XIMAGE_HANDLE)DXL_GetAlgorithmBasePtr(src);
	unsigned char *cAddr;
	int cSize;

	cAddr = DXL_GetXImageCDataAddr(src);
	cSize = DXL_GetXImageCSize(src);

	// if we have a compressed frame decompress it ( otherwise we'll just redo
	// the scaling and postprocessing from the last frame )
	if (cAddr)
	{
		if( cSize != 0 && (cAddr[0]>=1 || cAddr[1]>=1 || cAddr[2] >=1))
		{


			// decode the frame 
			int retVal = VP6_DecodeFrameToYUV (
				thisAlgorithmBase->myPBI,
				(char *)cAddr, 
				cSize);

			if ( retVal != 0 )
			{
				if ( retVal == -1)
					return DXL_VERSION_CONFLICT;
				else
					return DXL_BAD_DATA;
			}
			thisAlgorithmBase->decompressedOnce = 1;

		}
	}

	//CT>removed blit for size 
	VP6_GetYUVConfig(thisAlgorithmBase->myPBI, (YUV_BUFFER_CONFIG *) &thisAlgorithmBase->FrameBuffer);


	return DXL_OK;   
}

/****************************************************************************
* 
*  ROUTINE       :  vp60_xImageDestroy
*
*  INPUTS        :  vp60_XIMAGE_HANDLE xThis     :
*
*  OUTPUTS       :  None.
*
*  RETURNS       :  int:
*
*  FUNCTION      :  Closes decoder and releases resources.
*
*  SPECIAL NOTES :  None. 
*
****************************************************************************/
static int vp60_xImageDestroy ( DXL_XIMAGE_HANDLE src )
{
	vp60_XIMAGE_HANDLE thisAlgorithmBase = (vp60_XIMAGE_HANDLE)DXL_GetAlgorithmBasePtr(src);

	if(thisAlgorithmBase)
	{
		if ( thisAlgorithmBase->owned )
			VP6_StopDecoder ( &(thisAlgorithmBase->myPBI) );
		duck_free ( thisAlgorithmBase );
	}
	return DXL_OK;
}

/****************************************************************************
* 
*  ROUTINE       :  vp60_xImageReCreate
*
*  INPUTS        :  unsigned char *data :
*
*  OUTPUTS       :  None.
*
*  RETURNS       :  DXL_XIMAGE_HANDLE:
*
*  FUNCTION      :  
*
*  SPECIAL NOTES :  Called during initialization and/or when xImage
*                   (decompressor) attributes change, note that nImage and
*                   src are actually synonymous and should be cleared out
*                   a bit (to say the least!)
*
*                   NOTE:
*                   This function should be prepared to get data that is
*                   NOT of the type native to the decoder,  It should do
*                   it's best to verify it as valid data and should clean
*                   up after itself and return NULL if it doesn't recognize
*                   the format of the data.
*
****************************************************************************/
static DXL_HANDLE vp60_xImageReCreate
(
 DXL_XIMAGE_HANDLE src,
 unsigned char *data,
 int type,
 enum BITDEPTH bitDepth,
 int w,
 int h
 )
{  
	vp60_XIMAGE_HANDLE thisAlgorithmBase = (vp60_XIMAGE_HANDLE)DXL_GetAlgorithmBasePtr(src); 

	(void) bitDepth;

	if ( (type != VP60_FOURCC) && (type != VP61_FOURCC) && (type != VP62_FOURCC) ) 
		return NULL;


	/* create new PBI */
	if ( !VP6_StartDecoder( &(thisAlgorithmBase->myPBI), w, h ) )
	{
		vp60_xImageDestroy ( src );
		thisAlgorithmBase = NULL;
	}
	else
	{
		thisAlgorithmBase->owned = 1;
		thisAlgorithmBase->decompressedOnce = 0;
	}	

	return (DXL_HANDLE)thisAlgorithmBase;
}

/****************************************************************************
* 
*  ROUTINE       :  vp60_xImageCreate
*
*  INPUTS        :  unsigned char *data :
*
*  OUTPUTS       :  None.
*
*  RETURNS       :  DXL_XIMAGE_HANDLE:
*
*  FUNCTION      :  
*
*  SPECIAL NOTES :  In this "glue" case, just calls through to the 
*                   create function. 
*
****************************************************************************/
static DXL_HANDLE vp60_xImageCreate (DXL_XIMAGE_HANDLE src, unsigned char *data)
{
	//	return vp60_xImageReCreate(src, data, VP60_FOURCC, (enum BITDEPTH ) 0, 320, 240);

	vp60_XIMAGE_HANDLE thisAlgorithmBase = (vp60_XIMAGE_HANDLE)DXL_GetAlgorithmBasePtr(src); 

	/* create a new xImage, specific to this type of decoder, 
	(see "vp60_XIMAGE" struct above and dxl_main.h) */
		thisAlgorithmBase = (vp60_XIMAGE_HANDLE)duck_calloc ( 1, sizeof(vp60_XIMAGE), DMEM_GENERAL );
	if (thisAlgorithmBase == NULL) 
		return NULL;


	DXL_RegisterXImageRecreate(src, (RECREATE_FUNC) vp60_xImageReCreate);

	DXL_RegisterXImageDestroy(src, (DESTROY_FUNC) vp60_xImageDestroy);

	DXL_RegisterXImageDx(src, (DX_FUNC) vp60_decompress);

	DXL_RegisterXImageSetParameter(src, (SET_PARAMETER_FUNC) vp60_SetParameter);

	thisAlgorithmBase->myFourCC = VP60_FOURCC;

	thisAlgorithmBase->decompressedOnce = 0;
	return (DXL_HANDLE)thisAlgorithmBase;
}

/****************************************************************************
* 
*  ROUTINE       :  vp60_Init
*
*  INPUTS        :  None.
*
*  OUTPUTS       :  None.
*
*  RETURNS       :  int
*
*  FUNCTION      :  
*
*  SPECIAL NOTES :  
*
****************************************************************************/
int vp60_Init ( void )
{
	DXL_RegisterXImage((CREATE_FUNC) vp60_xImageCreate, VP60_FOURCC);
	DXL_RegisterXImage((CREATE_FUNC) vp60_xImageCreate, VP61_FOURCC);
	DXL_RegisterXImage((CREATE_FUNC) vp60_xImageCreate, VP62_FOURCC);

	/* initialize all the global variables */
	VP6_VPInitLibrary();

	return DXL_OK;
}

/****************************************************************************
* 
*  ROUTINE       :  vp60_Exit
*
*  INPUTS        :  None.
*
*  OUTPUTS       :  None.
*
*  RETURNS       :  int
*
*  FUNCTION      :  Main exit routine, called during DXL_ExitVideo()
*                   clean up any global information if necessary.
*                   
*  SPECIAL NOTES :  None. 
*
****************************************************************************/
int vp60_Exit(void)
{
	VP6_VPDeInitLibrary();

	return DXL_OK;
}

/****************************************************************************
* 
*  ROUTINE       :  vp60_SetParameter
*
*  INPUTS        :  DXL_XIMAGE_HANDLE src   :
*                   int Command             :
*                   unsigned long Parameter :
*
*  OUTPUTS       :  None.
*
*  RETURNS       :  void
*
*  FUNCTION      :  
*                   
*                   
*  SPECIAL NOTES :  None. 
*
****************************************************************************/
void vp60_SetParameter(DXL_XIMAGE_HANDLE src, int Command, uintptr_t Parameter)
{
	vp60_XIMAGE_HANDLE thisAlgorithmBase = (vp60_XIMAGE_HANDLE)DXL_GetAlgorithmBasePtr(src); 

	if ( Command == PBC_SET_PBSTRUCT )
	{
		if ( thisAlgorithmBase->owned )
			VP6_StopDecoder ( &(thisAlgorithmBase->myPBI) );

		thisAlgorithmBase->owned = 0;
		thisAlgorithmBase->myPBI = (PB_INSTANCE *) Parameter;
	}
	else
		VP6_SetPbParam( thisAlgorithmBase->myPBI, (PB_COMMAND_TYPE)Command, Parameter );
}

//CT:
typedef	struct {
	unsigned char*	baseAddr;
	long			rowBytes;
} YV12_PLANE;

typedef	struct {
	YV12_PLANE	y;
	YV12_PLANE	u;
	YV12_PLANE	v;
} YV12_PLANES;

void GetImageBufs(DXL_XIMAGE_HANDLE x, YV12_PLANES *p)
{
	//  vp60_XIMAGE_HANDLE xim=(vp60_XIMAGE_HANDLE)x;
	vp60_XIMAGE_HANDLE xim = (vp60_XIMAGE_HANDLE)DXL_GetAlgorithmBasePtr(x);

	p->y.baseAddr=(unsigned char *)xim->FrameBuffer.YBuffer;
	p->u.baseAddr=(unsigned char *)xim->FrameBuffer.UBuffer;
	p->v.baseAddr=(unsigned char *)xim->FrameBuffer.VBuffer;
	p->y.rowBytes=xim->FrameBuffer.YStride;
	p->u.rowBytes=xim->FrameBuffer.UVStride;
	p->v.rowBytes=xim->FrameBuffer.UVStride;
}
