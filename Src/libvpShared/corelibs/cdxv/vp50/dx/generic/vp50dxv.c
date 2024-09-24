/* 
 dxvmpg.cpp : Defines the entry point for the console application.
*/ 
#include <stdlib.h> 

#include "dkpltfrm.h" /* platform specifics */
#include "duktypes.h" /* predefined general types used at duck */

#include "duck_mem.h" /* interface to memory manager */
#include "dxl_main.h" /* interface to dxv */
#include "pbdll.h"

typedef unsigned long FourCC;
 
#define VP50_FOURCC DXL_MKFOURCC( 'V', 'P', '5', '0')
void vp50_SetParameter(DXL_XIMAGE_HANDLE src,int Command, unsigned long Parameter );

extern void vp3SetBlit(void);
extern void VP5_VPInitLibrary(void);
extern void VP5_VPDeInitLibrary(void);
#ifdef _MSC_VER 
#pragma warning(disable:4055)
#endif

#include "duck_dxl.h"
extern void VP5_readTSC(unsigned long *tsc);

void vp50_GetInfo(unsigned char * source, FrameInfo * frameInfo)
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

/* define an xImage structure based on the core xImage struct */
typedef struct tXImageCODEC
{
	xImageBaseStruct;
	FourCC myFourCC;
	DXV_YUV_BUFFER_CONFIG FrameBuffer;
	PB_INSTANCE *myPBI;
	int owned;
	
} vp50_XIMAGE,*vp50_XIMAGE_HANDLE;

static dxvBitDepth bitDepths[] = 
{
	DXRGB32,DXRGB24,DXRGB16,DXRGBNULL
};


typedef void ((*vp5BLIT_FUNC)(unsigned char *, int, YUV_BUFFER_CONFIG *));
typedef void ((*vp5_VSCREEN_FUNC)(void));


DXL_INTERNAL_FORMAT vp50_GetXImageInternalFormat(DXL_XIMAGE_HANDLE xImage,
												DXL_VSCREEN_HANDLE vScreen)
{
	(void) vScreen;
	(void) xImage;
	return YV12;
}
int vp50_blit(PB_INSTANCE *pbi,DXL_XIMAGE_HANDLE src, DXL_VSCREEN_HANDLE vScreen,DXV_YUV_BUFFER_CONFIG *FrameBuffer,int x, int y )
{
    if(vScreen && ((void *)(src->internalFormat) != NULL)) {
        /* get your hamdy damdy((c)1997 Duck North) registered blitter setup */
        vScreen->blitSetup = DXL_GetBlitSetupFunc(src,vScreen);
        vScreen->blitExit = DXL_GetBlitExitFunc(src,vScreen);
        vScreen->blitter = DXL_GetBlitFunc(src, vScreen); 

        if (vScreen->blitter ==  (void *) -1)
            return DXL_INVALID_BLIT;
    }

	if (vScreen) /* if there is a vScreen, blit to it */
	{
		if (vScreen->addr)
        { 
    		int pSize;
            int w,h;
            unsigned char *ptrScrn;
            int thisPitch = vScreen->pitch;
			unsigned int duration;
			unsigned int starttsc,endtsc;

			/* get a frame pointer to the scaled and postprocessed reconstructed buffer */
		    VP5_GetYUVConfig(pbi, (YUV_BUFFER_CONFIG *) FrameBuffer);

            pSize = DXL_GetVScreenSizeOfPixel(vScreen);

		    /* remember to offset if requested */
		    y += vScreen->viewY;           
		    x += vScreen->viewX ;

            /* for planar destinations */
            w = vScreen->viewW;//pitch;
            h = vScreen->height;

			if(w != FrameBuffer->YWidth)
			{
				FrameBuffer->YWidth = w;
				FrameBuffer->UVWidth = (w+1)/2;
			}
			if(h != FrameBuffer->YHeight)
			{
				FrameBuffer->YHeight = h;
				FrameBuffer->UVHeight = (h+1)/2;
			}
		    ptrScrn = vScreen->addr;
	        ptrScrn += (x * pSize) + (y * thisPitch);

            /* setup ptrs so we can work backwards through Paul's frame buffers */
            FrameBuffer->YBuffer = FrameBuffer->YBuffer + 
                    ((FrameBuffer->YHeight - 1) * 
                     (FrameBuffer->YStride));

			FrameBuffer->UBuffer = FrameBuffer->UBuffer +
                    ((FrameBuffer->UVHeight - 1) * 
                     (FrameBuffer->UVStride));
			
            FrameBuffer->VBuffer = FrameBuffer->VBuffer +
                    ((FrameBuffer->UVHeight - 1) * 
                     (FrameBuffer->UVStride));


            if((vScreen->bd != DXYUY2) && (vScreen->bd != DXYV12))
            {
                if(vScreen->bq == DXBLIT_STRETCH)
                {
                    thisPitch *= 2;
                }
            }

            if(vScreen->bd == DXYV12||vScreen->bd == DXI420)
            {
				if(thisPitch < 0)
				{
					FrameBuffer->uvStart = (char *) (ptrScrn + abs(thisPitch) + abs(thisPitch) * h/4 + thisPitch/2 );
					FrameBuffer->uvDstArea = abs((thisPitch * h)/4);
					FrameBuffer->uvUsedArea = 0;
				}
				else
				{
					FrameBuffer->uvStart = (char *) (ptrScrn + (thisPitch * h));
					FrameBuffer->uvDstArea = (((thisPitch+1)/2) * (( h+1)/2));
					FrameBuffer->uvUsedArea = (((thisPitch+1)/2) * FrameBuffer->UVHeight);
				}

				// Temporary fix for Scott Kludge Kludge Kludge !!!!!!!!!
				// ptrScrn -= thisPitch; // fixes a bug in assembly code for some reason the buttnutt is adding pitch to Y buffer
            }

			/* if a blitter hasn't been set up set one up ! */
            if (vScreen->blitSetup != (void *)-1) 
                ((vp5_VSCREEN_FUNC)vScreen->blitSetup)();

			/* if its still not set up return that it failed */
            if ((vp5BLIT_FUNC)vScreen->blitter == (vp5BLIT_FUNC)-1)
                return DXL_INVALID_BLIT;

			/* blit the screen */
			
			VP5_readTSC(&starttsc);
			if(pbi->Configuration.Interlaced==1 && (vScreen->bd != DXYV12 && vScreen->bd != DXI420))
			{
				int ypitch = FrameBuffer->YStride;
				int uvpitch = FrameBuffer->UVStride; 
		
				FrameBuffer->YStride <<= 1;
				FrameBuffer->YHeight >>= 1;
				FrameBuffer->UVStride <<= 1;
				FrameBuffer->UVHeight >>= 1;

				ptrScrn+=thisPitch;
				FrameBuffer->YBuffer -= ypitch;
				FrameBuffer->UBuffer -= uvpitch;
				FrameBuffer->VBuffer -= uvpitch;
	            ((vp5BLIT_FUNC)vScreen->blitter)(ptrScrn, thisPitch*2, (YUV_BUFFER_CONFIG *)(FrameBuffer));

				ptrScrn-=thisPitch;
				FrameBuffer->YBuffer += ypitch;
				FrameBuffer->UBuffer += uvpitch;
				FrameBuffer->VBuffer += uvpitch;
	            ((vp5BLIT_FUNC)vScreen->blitter)(ptrScrn, thisPitch*2, (YUV_BUFFER_CONFIG *)(FrameBuffer));

			}
			else
			{
	            ((vp5BLIT_FUNC)vScreen->blitter)(ptrScrn, thisPitch, (YUV_BUFFER_CONFIG *)(FrameBuffer));
			}
			VP5_readTSC(&endtsc);

			duration = ( endtsc - starttsc ) / (pbi->ProcessorFrequency) ;
			if( pbi->avgBlitTime == 0)
			{
				pbi->avgBlitTime = duration;
			}
			else
			{
		
				pbi->avgBlitTime = (7*pbi->avgBlitTime + duration)>>3;
			}

			/* blitter cleanup ?*/
            if ((vp5BLIT_FUNC)vScreen->blitExit != (vp5BLIT_FUNC)-1) 
                ((vp5_VSCREEN_FUNC)vScreen->blitExit)();

        }
	}
	return DXL_OK;
}


static int vp50_decompress(vp50_XIMAGE_HANDLE src, DXL_VSCREEN_HANDLE vScreen)
{

	// if we have a compressed frame decompress it ( otherwise we'll just redo
	// the scaling and postprocessing from the last frame )
    if (src->addr)
    {

		if( src->fSize != 0 && (src->addr[0]>=1 || src->addr[1]>=1 || src->addr[2] >=1))
		{
			// decode the frame 
			int retVal= VP5_DecodeFrameToYUV(
				src->myPBI,
		        (char *)src->addr, 
				src->fSize, 
				src->imWidth, 
				src->imHeight);

			if(retVal != 0 )
			{
	            if(retVal == -1)
		            return DXL_VERSION_CONFLICT;
			    else
				    return DXL_BAD_DATA;
			}
		}
    }
		VP5_GetYUVConfig(src->myPBI, (YUV_BUFFER_CONFIG *) &src->FrameBuffer);
	return DXL_OK;
}

/* 
  close down a decompressor, releasing the wilk decompressor, 
  the xImage (decompressor), and the intermediate vScreen (surface)
*/

static int vp50_xImageDestroy(vp50_XIMAGE_HANDLE xThis)
{
	if (xThis)
	{
		if(xThis->owned)
	        VP5_StopDecoder(&(xThis->myPBI));
		duck_free(xThis);
	}

	return DXL_OK;
}

/* 
  called during initialization and/or when xImage (decompressor)
  attributes change, note that nImage and src are actually
  synonymous and should be cleared out a bit (to say the least!)


  !!!!!!
  This function should be prepared to get data that is NOT of the 
  type native to the decoder,  It should do it's best to verify it 
  as valid data and should clean up after itself and return NULL
  if it doesn't recognize the format of the data
*/
static DXL_XIMAGE_HANDLE vp50_xImageCreate(unsigned char *data);
static DXL_XIMAGE_HANDLE vp50_xImageReCreate(vp50_XIMAGE_HANDLE src,unsigned char *data,
	int type,enum BITDEPTH bitDepth,int w,int h)
{  
	(void) bitDepth;
    if (type != VP50_FOURCC) 
		return NULL;

	if (src != NULL)	/* if an xImage/decompressor already exists, destroy it */
		vp50_xImageDestroy(src);

	/* create a new xImage, specific to this type of decoder, 
        (see "vp50_XIMAGE" struct above and dxl_main.h) */

	src = (vp50_XIMAGE_HANDLE)duck_calloc(1,sizeof(vp50_XIMAGE),DMEM_GENERAL);

	if (!src) 
        return NULL;

//	duck_memset(nImage,0,sizeof(vp50_XIMAGE));

	/* set up the "vtable" of interface calls */
    src->create =  (DXL_XIMAGE_HANDLE (*)(void *)) vp50_xImageCreate;
    src->recreate =  (DXL_XIMAGE_HANDLE (*)(DXL_XIMAGE_HANDLE,void *,int,int,int,int)) vp50_xImageReCreate;

	src->destroy = (int (*)(DXL_XIMAGE_HANDLE))vp50_xImageDestroy;
	src->dx = (int (*)(DXL_XIMAGE_HANDLE, DXL_VSCREEN_HANDLE)) vp50_decompress;
	src->blit = NULL; /* there is no interleaved blitter for vp5x files */
	src->setParameter = vp50_SetParameter;

#if !KLUDGE_FOR_NEIL
    src->internalFormat = (int (*)(DXL_XIMAGE_HANDLE, DXL_VSCREEN_HANDLE)) vp50_GetXImageInternalFormat; 
#endif
	src->bdPrefs = bitDepths; /* plug in the list of prefered bit depths */

    src->addr = data;
    src->dkFlags.inUse = 1;

	src->imWidth = src->w = (short) (w ? w : 320);
	src->imHeight = src->h = (short) (h ? h : 240);

	src->myFourCC = VP50_FOURCC;
  
    /* create new PBI */
    if(!VP5_StartDecoder( &(src->myPBI), src->imWidth, src->imHeight ))
    {
		vp50_xImageDestroy(src);
        src = NULL;
    }
	src->owned = 1;

    return (DXL_XIMAGE_HANDLE ) src;
}

/* in this "glue" case, just calls through to the create function */

static DXL_XIMAGE_HANDLE vp50_xImageCreate(unsigned char *data)
{
	return vp50_xImageReCreate(NULL, data, VP50_FOURCC, (enum BITDEPTH ) 0,0,0);
}

int vp50_Init(void)
{

    DXL_RegisterXImage( 
		(DXL_XIMAGE_HANDLE (*)(unsigned char *)) vp50_xImageCreate,
		VP50_FOURCC, 
        YV12 
		);


	/* initialize all the global variables */
	VP5_VPInitLibrary();

	return DXL_OK;
}

/* 
    main exit routine, called during DXL_ExitVideo() 
    clean up any global information if necessary
*/

int vp50_Exit(void)
{
	VP5_VPDeInitLibrary();

	return DXL_OK;
}

void vp50_SetParameter(DXL_XIMAGE_HANDLE src,int Command, unsigned long Parameter )
{
	if(Command == PBC_SET_PBSTRUCT)
	{

		if(((vp50_XIMAGE_HANDLE) src)->owned)
	        VP5_StopDecoder(&(((vp50_XIMAGE_HANDLE) src)->myPBI));

		((vp50_XIMAGE_HANDLE) src)->owned = 0;
		((vp50_XIMAGE_HANDLE) src)->myPBI= (PB_INSTANCE *) Parameter;

	}
	else
		VP5_SetPbParam( ((vp50_XIMAGE_HANDLE) src)->myPBI, (PB_COMMAND_TYPE) Command, (UINT32) Parameter );
}

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
  vp50_XIMAGE_HANDLE xim=(vp50_XIMAGE_HANDLE)x;
  p->y.baseAddr=(unsigned char *)xim->FrameBuffer.YBuffer;
  p->u.baseAddr=(unsigned char *)xim->FrameBuffer.UBuffer;
  p->v.baseAddr=(unsigned char *)xim->FrameBuffer.VBuffer;
  p->y.rowBytes=xim->FrameBuffer.YStride;
  p->u.rowBytes=xim->FrameBuffer.UVStride;
  p->v.rowBytes=xim->FrameBuffer.UVStride;
}