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


#include "../include/duck_dxl.h"
#include "duck_mem.h"
#include "../include/dxl_plugin.h"         
typedef struct tagflgs 
{
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

typedef struct vScreen
{
	DXL_OBJECT_VERSION version;

	unsigned char *_addr;
	unsigned char *laddr;	/* address of destination and what it was the last time */

	enum BITDEPTH bd;		/* format of destination */
	enum BLITQUALITY bq;	/* blit translation mode */

	short pitch, height;	/* pitch and height of dest */        
      
	short viewX,viewY;		/* offset/clipping viewport within destination */
	short viewW,viewH;

	dkInfoFlags dkFlags;

} DXL_VSCREEN;

#define validate(x) \
{ \
	if (!x) \
		return (int) DXL_NULLSOURCE; \
	if (!x->dkFlags.inUse) \
		return (int) DXL_NOTINUSE; \
}

/***********************************************/

DXL_VSCREEN_HANDLE 
vScreenCreate(void)
{
	DXL_VSCREEN_HANDLE nScreen;

    nScreen = (DXL_VSCREEN_HANDLE)duck_calloc(1,sizeof(DXL_VSCREEN),DMEM_GENERAL);
	if (nScreen)
		nScreen->dkFlags.allocated = 1;

    return nScreen;
}

int 
DXL_SetVScreenBlitQuality(DXL_VSCREEN_HANDLE dst, enum BLITQUALITY blitquality)                  
{
	int oldBQ;
	
	validate(dst);

	oldBQ = dst->bq;
	dst->bq = blitquality;

	return oldBQ;
}

void 
DXL_DestroyVScreen(DXL_VSCREEN_HANDLE dst)
{
    if (dst != NULL){
        dst->dkFlags.inUse = 0;
        dst->_addr = NULL;
		if (dst->dkFlags.allocated)
			duck_free(dst);
    }
}

int 
DXL_AlterVScreen(DXL_VSCREEN_HANDLE dst, unsigned char *_addr, enum BITDEPTH bd, int p, int h) 
{       
    validate(dst);

    if (_addr != NULL) dst->_addr = _addr;

    if (bd != DXRGBNULL) dst->bd = bd;

    if (p != -1) dst->pitch = (short) p;
    
    if (h != -1) dst->height = (short) h;

    return DXL_OK;
}           

int 
DXL_AlterVScreenView(DXL_VSCREEN_HANDLE dst,int x,int y,int w,int h)
{
    validate(dst);

	if (x > -1)	dst->viewX = (short)x;
    if (y > -1)	dst->viewY = (short)y;
    if (w > -1) dst->viewW = (short)w;
    if (h > -1) dst->viewH = (short)h;

    return DXL_OK;
}   
        
DXL_VSCREEN_HANDLE 
DXL_CreateVScreen(unsigned char *_addr, enum BITDEPTH bd, short p,short h)
{
	DXL_VSCREEN_HANDLE vScreenCreate(void);
	DXL_VSCREEN_HANDLE nScreen = vScreenCreate();

	if (!nScreen) 
		return NULL;

	nScreen->dkFlags.inUse = 1;

	DXL_AlterVScreen(nScreen, _addr, bd, p, h);

	return nScreen;
}

int DXL_GetVScreenView(DXL_VSCREEN_HANDLE dst,int *x,int *y,int *w,int *h)
{
	validate(dst);

    if(x)
	    *x = dst->viewX;
    if(y)
	    *y = dst->viewY;
    if(w)
	    *w = dst->viewW;
    if(h)
    	*h = dst->viewH;

	return DXL_OK;
}



int DXL_GetVScreenAttributes(DXL_VSCREEN_HANDLE dst, void **_addr, dxvBlitQuality *bq, dxvBitDepth *bd, short *pitch, short *height)
{
	validate(dst);

    if(_addr)
        *_addr = dst->_addr;

    if(bq)
        *bq = dst->bq;
    
    if(bd)
        *bd = dst->bd;

    if(pitch)
        *pitch = dst->pitch;
    
    if(height)
        *height = dst->height;

    return DXL_OK;
}   


