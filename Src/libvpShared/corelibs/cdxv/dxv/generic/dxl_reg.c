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


/********

  DXL_REG.C - functions for registration of "Blit" functions
  (C)1996 The Duck Corporation

********/
#include <assert.h>
#include <dxl_main.h>
#include <stdio.h>
#include <string.h>
#include "duck_mem.h"

typedef struct tBlitStruct {
	blitFunc setup, blit, exit;
} DXL_BLITTER, DXL_BLITTER_HANDLE;

static int nextBlitter = 1;
                                  /**************/
static DXL_BLITTER blitTable[32][DXL_MAX_IFORMATS];
static signed char blitTranslateTable[MAX_BQUALITIES][MAX_CDEPTHS];

static void nullBlitter(void){}

static DXL_INTERNAL_FORMAT iFormats[] = {
	DXL_NULL_IFORMAT,
	DXL_NULL_IFORMAT,
	DXL_NULL_IFORMAT,
	DXL_NULL_IFORMAT,
	DXL_NULL_IFORMAT,
	DXL_NULL_IFORMAT,
	DXL_NULL_IFORMAT,
	DXL_NULL_IFORMAT
};
DXL_INTERNAL_FORMAT dxl_GetFOURCCInternalFormat(unsigned long fourcc)
{
	int aHandle = dxl_GetAlgHandle(fourcc);

	if (aHandle != -1)
		return iFormats[aHandle];

	return DXL_NULL_IFORMAT;
}

int dxl_RegisterInternalFormat(int xHandle, DXL_INTERNAL_FORMAT xFormat)
{
	iFormats[xHandle] = xFormat;

	return DXL_OK;
}

DXL_BLIT_FORMAT DXL_ReserveBlitter(void)
{
	if (nextBlitter >= 32)
		return -1; /*DXL_EXCEEDED_MAX_BLITTERS;*/

	return nextBlitter++;
}

DXL_BLIT_FORMAT DXL_OverrideBlitter(enum BLITQUALITY bq,enum BITDEPTH bd)
{
    if(blitTranslateTable[bq][bd] == (signed char)-1)
	    blitTranslateTable[bq][bd] =  (char)DXL_ReserveBlitter();

    return blitTranslateTable[bq][bd];
}

int DXL_RegisterBlitter(DXL_BLIT_FORMAT dFormat, DXL_INTERNAL_FORMAT sFormat, 
						blitFunc blit, blitFunc setup, blitFunc exit)
{
	if ((dFormat >= nextBlitter) || (sFormat >= DXL_MAX_IFORMATS))
		return -1; /*DXL_INVALID_BLIT_FORMAT;*/

	blitTable[dFormat][sFormat].setup = setup;
	blitTable[dFormat][sFormat].exit = exit;
	blitTable[dFormat][sFormat].blit = blit;

	return 0; /*DXL_OK;*/
}

DXL_INTERNAL_FORMAT DXL_GetXImageInternalFormat(DXL_XIMAGE_HANDLE xImage,
												DXL_VSCREEN_HANDLE vScreen)
{
	int ret;

	ret = xImage->internalFormat(xImage,vScreen);

	if (ret == DXL_NULL_IFORMAT)
	{
		return (DXL_INTERNAL_FORMAT )
			dxl_GetFOURCCInternalFormat(DXL_GetXImageFOURCC(xImage));
	}
	return (DXL_INTERNAL_FORMAT ) ret;
}

DXL_INTERNAL_FORMAT DXL_GetVScreenInternalFormat(DXL_VSCREEN_HANDLE vScreen)
{
	if (vScreen->bd == DXRGB16){
		return DXL_LINE16;
	}else if (vScreen->bd == DXRGB8||vScreen->bd == DXHALFTONE8){
		return DXL_LINE8;
	}else
		return (DXL_INTERNAL_FORMAT) -1;
}

blitFunc DXL_GetVBlitFunc(DXL_VSCREEN_HANDLE src,DXL_VSCREEN_HANDLE dst)
{    
	return blitTable[DXL_GetVScreenBlitFormat(dst)]
		[DXL_GetVScreenInternalFormat(src)].blit;
}

blitFunc DXL_GetVBlitSetupFunc(DXL_VSCREEN_HANDLE src,DXL_VSCREEN_HANDLE dst)
{    
	return blitTable[DXL_GetVScreenBlitFormat(dst)]
		[DXL_GetVScreenInternalFormat(src)].setup;
}

blitFunc DXL_GetBlitFunc(DXL_XIMAGE_HANDLE xImage,DXL_VSCREEN_HANDLE vScreen)
{   
	DXL_BLIT_FORMAT i = DXL_GetVScreenBlitFormat(vScreen);
	DXL_INTERNAL_FORMAT j = DXL_GetXImageInternalFormat(xImage,vScreen);

	if(i == -1)
		return (blitFunc)-1;

	if(j == DXL_NULL_IFORMAT) 
#pragma warning(disable:4054) // typecase from function pointer to data pointer
		return (blitFunc)nullBlitter;
#pragma warning(default:4054) // typecase from function pointer to data pointer
	else
		return blitTable[i][j].blit;
}

void *DXL_GetBlitSetupFunc(DXL_XIMAGE_HANDLE xImage,DXL_VSCREEN_HANDLE vScreen)
{    
	return blitTable[DXL_GetVScreenBlitFormat(vScreen)]
		[DXL_GetXImageInternalFormat(xImage,vScreen)].setup;
}

void *DXL_GetBlitExitFunc(DXL_XIMAGE_HANDLE xImage,DXL_VSCREEN_HANDLE vScreen)
{    
	return blitTable[DXL_GetVScreenBlitFormat(vScreen)]
		[DXL_GetXImageInternalFormat(xImage,vScreen)].exit;
}
 
DXL_BLIT_FORMAT DXL_GetVScreenBlitFormat(DXL_VSCREEN_HANDLE vScreen)
{   enum BLITQUALITY bq;

	if (vScreen->blitFormat != (signed char)-1)
		return vScreen->blitFormat;
		
	bq = DXL_GetVScreenBlitQuality(vScreen);

	return blitTranslateTable[bq]
		[vScreen->bd];
}

void resetBlitters(void)
{    
	nextBlitter = 0;

	duck_memset(blitTable,-1,sizeof(blitTable));
	duck_memset(blitTranslateTable,-1,sizeof(blitTranslateTable));
}



int DXL_CheckFCCToVScreenFormat(unsigned long FCC,enum BITDEPTH format, enum BLITQUALITY bq)
{
	DXL_XIMAGE_HANDLE src;
	DXL_VSCREEN_HANDLE dst;
	int ret = DXL_INVALID_BLIT;

	src = DXL_CreateXImageOfType(NULL,FCC);	
	assert(src != NULL);

	if (src)
	{
		dst = DXL_CreateVScreen(
			(unsigned char *)0xDEADBEEF, format, 1280,480);

		assert(dst != NULL);
		if (dst)
		{
			dst->bq = bq;
			ret = DXL_CheckdxImageToVScreen(src, dst);
			DXL_DestroyVScreen(dst);
		}
		DXL_DestroyXImage(src);
	}
	return ret;
}

int DXL_CheckVScreenXImageBlit(DXL_VSCREEN_HANDLE dst,DXL_XIMAGE_HANDLE src)
{
    validate(src);

    if (!src->dx)
        return -1;
	
	if (!dst) return -1;

	if (src->verify != NULL)
		return(src->verify(src,dst));

#pragma warning(disable:4054) // typecase from function pointer to data pointer
    if((void *)(src->internalFormat) != NULL){
        dst->blitter = DXL_GetBlitFunc(src, dst); 

        if ((dst->blitter !=  (void *) -1) && (dst->blitter !=  nullBlitter))
			return DXL_OK;
    }
#pragma warning(default:4054) // typecase from function pointer to data pointer
	return DXL_INVALID_BLIT;
}

int DXL_CheckVScreenBlit(DXL_VSCREEN_HANDLE dst,unsigned long fourcc)
{
	return DXL_CheckFCCToVScreenFormat(fourcc,dst->bd, dst->bq);
}

int DXL_CheckdxImageToVScreen(DXL_XIMAGE_HANDLE src, DXL_VSCREEN_HANDLE dst)
{
	return DXL_CheckVScreenXImageBlit( dst, src);
}