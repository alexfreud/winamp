/****************************************************************************
*
*   Module Title :     vp5dxv.c
*
*   Description  :     VP50 interface to DXV.
*
*    AUTHOR      :     SJL
*
*****************************************************************************
*   Revision History
*
*   1.03 SJL 17/10/02  Up the version to 1.0.0.3, added new dxv interface
*   1.02 YWX 30/09/02  Up the version to 1.0.0.2, added support of scaling
*   1.01 YWX 19/09/02  Fixed bug in blit and up the version to 1.0.0.1
*	1.00 SJL 17/06/02  Base
*
*****************************************************************************
*/
//#include <stdlib.h> 

#include "duck_mem.h" /* interface to memory manager */
#include "dxl_plugin.h" /* interface to dxv */

#include "pbdll.h"


const char* VP6LIBVERSION="ON2 VP6 Decode Library for MAC Version 1.0.0.3";

typedef unsigned int FourCC;
 
#define VP60_FOURCC DXL_MKFOURCC( 'V', 'P', '6', '0')
#define VP61_FOURCC DXL_MKFOURCC( 'V', 'P', '6', '1')


static dxvBitDepth bitDepths[] = 
{
	DXYV12,DXRGBNULL
};


void vp60_SetParameter(DXL_XIMAGE_HANDLE src,int Command, unsigned int Parameter );

extern void VP6_VPInitLibrary(void);
extern void VP6_VPDeInitLibrary(void);

#include "duck_dxl.h"

#if 0
typedef struct tFrameInfo
{
    int KeyFrame;
    int Version;
    int Quality;
    int vp30Flag;
} FrameInfo;

void 
vp60_GetInfo(unsigned char * source, FrameInfo * frameInfo)
{

    // Is the frame and inter frame or a key frame 
    frameInfo->KeyFrame = !(source[0] > 0x7f);
    frameInfo->Quality = source[0] >> 2;
    if(frameInfo->KeyFrame) 
        frameInfo->Version = ((source[2]>>3) & 0x1f );
    else
        frameInfo->Version = 0;

    frameInfo->vp30Flag = (int)source[1];

}
#endif

// YUV buffer configuration structure
typedef struct
{
    int     YWidth;
    int     YHeight;
    int     YStride;

    int     UVWidth;
    int     UVHeight;
    int     UVStride;

    char *  YBuffer;
    char *  UBuffer;
    char *  VBuffer;

	char *  uvStart;
    int uvDstArea;
    int uvUsedArea;

} DXV_YUV_BUFFER_CONFIG;

/* define an algorithm base container */
typedef struct tXImageCODEC
{
	FourCC myFourCC;
	DXV_YUV_BUFFER_CONFIG FrameBuffer;
	PB_INSTANCE *myPBI;
	int owned;
} vp60_XIMAGE, *vp60_XIMAGE_HANDLE;


typedef void ((*VP6BLIT_FUNC)(unsigned char *, int, YUV_BUFFER_CONFIG *));
//typedef void ((*vp6_VSCREEN_FUNC)(void));

/****************************************************************************
 * 
 *  ROUTINE       :     vp50_decompress
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     
 *
 *  SPECIAL NOTES :     
 *
 ****************************************************************************/
#include "huffman.h"

static int 
vp60_decompress(DXL_XIMAGE_HANDLE src, DXL_VSCREEN_HANDLE vScreen)
{

	int retVal;
	vp60_XIMAGE_HANDLE thisAlgorithmBase = (vp60_XIMAGE_HANDLE)DXL_GetAlgorithmBasePtr(src);
	unsigned char *cAddr;
	int cSize;		
	int w, h;

	
	DXL_GetXImageXYWH(src, NULL, NULL, &w, &h);
			


	// if we have a compressed frame decompress it ( otherwise we'll just redo
	// the scaling and postprocessing from the last frame )
    cAddr = DXL_GetXImageCDataAddr(src);
	cSize = DXL_GetXImageCSize(src);
	
    if(cAddr)
    {
		if((cSize != 0) && (cAddr[0]>=1 || cAddr[1]>=1 || cAddr[2] >=1))
		{
			// decode the frame 
			retVal = VP6_DecodeFrameToYUV(thisAlgorithmBase->myPBI, (char *)cAddr, cSize, w, h);
			if(retVal != 0 )
			{
	            if(retVal == -1)
		            return DXL_VERSION_CONFLICT;
			    else
				    return DXL_BAD_DATA;
			}
		}
    }


	if (vScreen) /* if there is a vScreen, blit to it */
	{
        unsigned char * ptrScrn;
        short thisPitch, vsHeight;
        dxvBlitQuality bq; 
        dxvBitDepth bd;
        VP6BLIT_FUNC blitter;
        
        DXL_GetVScreenAttributes(vScreen, (void **)&ptrScrn, &bq, &bd, &thisPitch, &vsHeight);

		if(ptrScrn)
        { 
    		int x, y, pSize;
            int viewX, viewY;

			DXL_GetVScreenView(vScreen, &viewX, &viewY, NULL, NULL);

			/* get a frame pointer to the scaled and postprocessed reconstructed buffer */
		    VP6_GetYUVConfig(thisAlgorithmBase->myPBI, (YUV_BUFFER_CONFIG *) &(thisAlgorithmBase->FrameBuffer));
			
          	pSize = VPX_GetSizeOfPixel(bd);

			DXL_GetXImageXYWH(src, &x, &y, NULL, NULL);

		    /* remember to offset if requested */
		    y += viewY;
		    x += viewX; 

	        ptrScrn += (x * pSize) + (y * thisPitch);

            /* setup ptrs so we can work backwards through Paul's frame buffers */
            #if 1
            thisAlgorithmBase->FrameBuffer.YBuffer = thisAlgorithmBase->FrameBuffer.YBuffer + 
                    ((thisAlgorithmBase->FrameBuffer.YHeight - 1) * 
                     (thisAlgorithmBase->FrameBuffer.YStride));

			thisAlgorithmBase->FrameBuffer.UBuffer = thisAlgorithmBase->FrameBuffer.UBuffer +
                    ((thisAlgorithmBase->FrameBuffer.UVHeight - 1) * 
                     (thisAlgorithmBase->FrameBuffer.UVStride));
			
            thisAlgorithmBase->FrameBuffer.VBuffer = thisAlgorithmBase->FrameBuffer.VBuffer +
                    ((thisAlgorithmBase->FrameBuffer.UVHeight - 1) * 
                     (thisAlgorithmBase->FrameBuffer.UVStride));
            #endif
            
            if((bd != DXYUY2) && (bd != DXYV12))
            {
                if(bq == DXBLIT_STRETCH)
                {
                    thisPitch *= 2;
                }
            }

            if(bd == DXYV12 || bd == DXI420)
            {
				if(thisPitch < 0)
				{
					thisAlgorithmBase->FrameBuffer.uvStart = (char *) (ptrScrn + abs(thisPitch) + abs(thisPitch) * h/4 + thisPitch/2 );
					thisAlgorithmBase->FrameBuffer.uvDstArea = abs((thisPitch * h)/4);
					thisAlgorithmBase->FrameBuffer.uvUsedArea = 0;
				}
				else
				{
					thisAlgorithmBase->FrameBuffer.uvStart = (char *) (ptrScrn + (thisPitch * h));
					thisAlgorithmBase->FrameBuffer.uvDstArea = ((thisPitch * h)/4);
					thisAlgorithmBase->FrameBuffer.uvUsedArea = ((thisPitch * thisAlgorithmBase->FrameBuffer.UVHeight)/2);
				}

            }

			blitter = (VP6BLIT_FUNC)VPX_GetBlitter(bq, bd);
			
			if ((void *)blitter != (void *)-1) 
			{
            	blitter(ptrScrn, thisPitch, (YUV_BUFFER_CONFIG *)(&thisAlgorithmBase->FrameBuffer));
            }
            else
            {
            	return DXL_INVALID_BLIT;
            }


        }
	}

	return DXL_OK;
}

/****************************************************************************
 * 
 *  ROUTINE       :     vp60_xImageDestroy
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     close down a decompressor, releasing the wilk decompressor, 
 *                      the xImage (decompressor), and the intermediate vScreen (surface)
 *
 *  SPECIAL NOTES :     
 *
 ****************************************************************************/
static int 
vp60_xImageDestroy(DXL_XIMAGE_HANDLE src)
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
 *  ROUTINE       :     vp50_xImageReCreate
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     
 *
 *  SPECIAL NOTES : 
 *                  called during initialization and/or when xImage (decompressor)
 *                  attributes change, note that nImage and src are actually
 *                  synonymous and should be cleared out a bit (to say the least!)
 *
 *
 *                  !!!!!!
 *                  This function should be prepared to get data that is NOT of the 
 *                  type native to the decoder,  It should do it's best to verify it 
 *                  as valid data and should clean up after itself and return NULL
 *                  if it doesn't recognize the format of the data
 *
 ****************************************************************************/
static void * 
vp60_xImageReCreate(DXL_XIMAGE_HANDLE src, unsigned char *data, int type, enum BITDEPTH bitDepth, int w, int h)
{  
	vp60_XIMAGE_HANDLE thisAlgorithmBase = (vp60_XIMAGE_HANDLE)DXL_GetAlgorithmBasePtr(src); 

	(void) bitDepth;
   

    if ( 
    		(type != VP60_FOURCC) && 
    		(type != VP61_FOURCC) && 
    		(type !=  DXL_MKFOURCC( 'V', 'P', '6', '2'))  
    	) 
		return NULL;

	thisAlgorithmBase->myFourCC = type;

    /* create new PBI */
    if ( !VP6_StartDecoder( &(thisAlgorithmBase->myPBI), w, h ) )
    {
		vp60_xImageDestroy ( src );
        thisAlgorithmBase = NULL;
    }
    else
    {
	    thisAlgorithmBase->owned = 1;
    }


    return (DXL_HANDLE)thisAlgorithmBase;
}

/****************************************************************************
 * 
 *  ROUTINE       :     vp50_xImageCreate
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     
 *
 *  SPECIAL NOTES :     in this "glue" case, just calls through to the create function. 
 *
 ****************************************************************************/
static DXL_HANDLE 
vp60_xImageCreate (DXL_XIMAGE_HANDLE src, unsigned char *data)
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

	return (DXL_HANDLE)thisAlgorithmBase;
}

/****************************************************************************
 * 
 *  ROUTINE       :     vp50_Init
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
int 
vp60_Init(void)
{
    DXL_RegisterXImage((CREATE_FUNC) vp60_xImageCreate, VP60_FOURCC);

    DXL_RegisterXImage((CREATE_FUNC) vp60_xImageCreate, VP61_FOURCC);
    
    DXL_RegisterXImage((CREATE_FUNC) vp60_xImageCreate,  DXL_MKFOURCC( 'V', 'P', '6', '2'));
    

    vp3SetBlit();

	/* initialize all the global variables */
	VP6_VPInitLibrary();
	
	return DXL_OK;
}

/****************************************************************************
 * 
 *  ROUTINE       :     vp60_Exit
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     main exit routine, called during DXL_ExitVideo() 
 *                      clean up any global information if necessary
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
int 
vp60_Exit(void)
{
	VP6_VPDeInitLibrary();

	return DXL_OK;
}
/****************************************************************************
 * 
 *  ROUTINE       :     
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void 
vp60_SetParameter(DXL_XIMAGE_HANDLE src, int Command, uinptr_t Parameter)
{
	vp60_XIMAGE_HANDLE thisAlgorithmBase = (vp60_XIMAGE_HANDLE)DXL_GetAlgorithmBasePtr(src); 

	VP6_SetPbParam(thisAlgorithmBase->myPBI, (PB_COMMAND_TYPE) Command, Parameter );
}
