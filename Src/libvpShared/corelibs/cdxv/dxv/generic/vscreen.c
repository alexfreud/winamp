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


#include "duck_mem.h"
#include "dxl_main.h"                                            
#include <assert.h>
/***********************************************/

int DXL_GetVScreenSizeOfPixel(DXL_VSCREEN_HANDLE vSc)
{
    switch (vSc->bd){
        case DXRGB8: 
		case DXHALFTONE8:
        case DXRGB8VESA:            
            return 1;
        case DXRGB16_555:
        case DXRGB16_565:
        case DXRGB16VESA:
        case DXYUY2:
        case DXUYVY:
            return 2;
        case DXRGB24:
            return 3;
        case DXRGB32:
			return 4;
		default:
			return -1;
    }
}

void DXL_DestroyVScreen(DXL_VSCREEN_HANDLE dst)
{
    if (dst != NULL){
        dst->dkFlags.inUse = 0;
        dst->addr = NULL;
		if (dst->dkFlags.allocated)
			duck_free(dst);
    }
}

int DXL_AlterVScreen(DXL_VSCREEN_HANDLE dst, unsigned char *addr,enum BITDEPTH bd, int p,int h) 
{       
    validate(dst);

    if (addr != NULL) dst->addr = addr;

    if (bd != DXRGBNULL) dst->bd = bd;

    if (p != -1) dst->pitch = (short) p;
    
    if (h != -1) dst->height = (short) h;

    return DXL_OK;
}           

int DXL_AlterVScreenView(DXL_VSCREEN_HANDLE dst,int x,int y,int w,int h)
{
    validate(dst);

	if (x > -1)	dst->viewX = (short)x;// & 0xfffe;
    if (y > -1)	dst->viewY = (short)y;
    if (w > -1) dst->viewW = (short)w;// & 0xfffe;
    if (h > -1) dst->viewH = (short)h;

    return DXL_OK;
}   
        
DXL_VSCREEN_HANDLE DXL_CreateVScreen(unsigned char *addr, enum BITDEPTH bd, short p,short h)
{
#pragma warning(disable: 4210) // nonstandard extension used : function given file scope
	DXL_VSCREEN_HANDLE vScreenCreate(void);
#pragma warning(default: 4210) // nonstandard extension used : function given file scope

	DXL_VSCREEN_HANDLE nScreen = vScreenCreate();

	if (!nScreen) return NULL;

	nScreen->dkFlags.inUse = 1;
	nScreen->blitFormat = -1;

	DXL_AlterVScreen(nScreen, addr, bd, p, h);

	nScreen->bx = nScreen->by = 0;
	nScreen->bAddr = NULL;
	nScreen->bq = DXBLIT_SAME;

	return nScreen;
}

int DXL_GetVScreenView(DXL_VSCREEN_HANDLE dst,int *x,int *y,int *w,int *h)
{
	validate(dst);

	*x = dst->viewX;
	*y = dst->viewY;
	*w = dst->viewW;
	*h = dst->viewH;

	return DXL_OK;
}



int DXL_GetVScreenAttributes(
    DXL_VSCREEN_HANDLE vScreen,
    void **addr, 
    dxvBlitQuality *bq, 
    dxvBitDepth *bd,
    short *pitch, 
    short *height
	)
{
	if (addr)
	{
		*addr = (void *) (vScreen->addr);
	}
	else
	{
		assert(0);
	}


	if (bq)
	{
		*bq = vScreen->bq;
	}
	else
	{
		assert(0);
	}



	if (bd)
	{
		*bd = vScreen->bd;
	}
	else
	{
		assert(0);
	}


	if (pitch)
	{
		*pitch = vScreen->pitch;
	}
	else
	{
		assert(0);
	}



	if (height)
	{
		*height = vScreen->height;
	}
	else
	{
		assert(0);
	}


	return 0;
}  /* end get attributes */
