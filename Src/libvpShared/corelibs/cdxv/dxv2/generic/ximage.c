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
#include "../include/dxl_plugin.h" 
#include <ctype.h>  /* toupper */

static CREATE_FUNC creator[NUM_ALG];
static unsigned int fourCC[NUM_ALG];

static DXL_OBJECT_VERSION thisVersion = 0x01000001;

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


typedef struct tXImageBase
{  
	dkInfoFlags dkFlags;
	//short x,y,w,h;
	unsigned char *addr;
	enum BITDEPTH *bdPrefs;
	CREATE_FUNC create;
	RECREATE_FUNC recreate;
	DESTROY_FUNC destroy;
    SEND_VMSG_FUNC sendVideoMessage;
	DX_FUNC dx;
	int fSize;
	SET_PARAMETER_FUNC setParameter;
	GET_PARAMETER_FUNC getParameter;
} DXL_XIMAGE_BASE;

typedef struct tXImage
{  
	DXL_OBJECT_VERSION version;
	
	DXL_XIMAGE_BASE * xImageBasePtr;
	
	DXL_HANDLE algorithmBasePtr;
} DXL_XIMAGE;

#define validateXImage(x) \
{ \
	if (!x) \
		return (int) DXL_NULLSOURCE; \
	if (!x->xImageBasePtr->dkFlags.inUse) \
		return (int) DXL_NOTINUSE; \
}

//	if (!x->version != thisVersion) 
//		return (int) DXL_INVALID_DATA; 

static //inline 
unsigned int toUpperFOURCC(unsigned int type)
{

    return (
            (toupper((char)((type >> 24) & 0xff)) << 24) |
            (toupper((char)((type >> 16) & 0xff)) << 16) |
            (toupper((char)((type >> 8) & 0xff)) << 8) |
            toupper((char)((type >> 0) & 0xff))
            );
}

int 
DXL_SetXImageCSize(DXL_XIMAGE_HANDLE src, int temp)
{
    if(src == NULL) 
    	return DXL_NOTINUSE;

	src->xImageBasePtr->fSize = temp;

	return DXL_OK;
}

int 
DXL_GetXImageCSize(DXL_XIMAGE_HANDLE src)
{
    if(src == NULL) 
    	return 0;

    return src->xImageBasePtr->fSize;
}

unsigned char * 
DXL_GetXImageCDataAddr(DXL_XIMAGE_HANDLE src)
{
    if(src == NULL) 
    	return NULL;

	return src->xImageBasePtr->addr;
}
/*
int 
DXL_MoveXImage(DXL_XIMAGE_HANDLE src, enum OFFSETXY mode, int x, int y)
{
    validateXImage(src);
    
    if (mode != DXL_RELATIVE)
    {
        src->xImageBasePtr->x = 0;
        src->xImageBasePtr->y = 0;
    }
    src->xImageBasePtr->x += (short) x;
    src->xImageBasePtr->y += (short) y;

    return DXL_OK;
}
*/
int 
DXL_AlterXImageData(DXL_XIMAGE_HANDLE src, unsigned char *data)
{                    
    validateXImage(src);

    src->xImageBasePtr->addr = data;       
    src->xImageBasePtr->dkFlags.DXed = 0;

    if (data == NULL) 
        return DXL_OK;

    return DXL_OK;
}

int 
DXL_IsXImageKeyFrame(DXL_XIMAGE_HANDLE src)
{
    validateXImage(src);

    return src->xImageBasePtr->dkFlags.keyFrame;
}


void 
DXL_DestroyXImage(DXL_XIMAGE_HANDLE src)
{
    if (src != NULL)
    {
        if (src->xImageBasePtr->dkFlags.inUse)
        {
            src->xImageBasePtr->destroy(src);
        }

    	duck_free(src->xImageBasePtr);
    	duck_free(src);
    }
}

DXL_XIMAGE_HANDLE DXL_CreateXImageOfType(unsigned char *data, unsigned int type)
{
    int i;
    DXL_XIMAGE_HANDLE nImage = NULL;
    
    /* alloc our ximage */
    nImage = (DXL_XIMAGE_HANDLE) duck_calloc(1, sizeof(DXL_XIMAGE), DMEM_GENERAL);
    if(nImage == NULL) 
    {
    	return NULL;
    }

    nImage->version = thisVersion;

    /* alloc our generic ximage base */
    nImage->xImageBasePtr = (DXL_XIMAGE_BASE *) duck_calloc(1, sizeof(DXL_XIMAGE_BASE), DMEM_GENERAL);
    if(nImage->xImageBasePtr == NULL) 
    {
    	duck_free(nImage);
    	return NULL;
    }

    /* clear out just in case calloc does not really work */
    nImage->algorithmBasePtr = NULL;
    

    /*
    //convert fourCC to uppercase, fixes problem with calls to DXV with
    //lowercase fourCC's
    */
    type = toUpperFOURCC(type);


	/* try to match the fourcc to a registered algorithm */
    for(i = 0; i < NUM_ALG; i++)
    {
        if(fourCC[i] == type)
        {
            if(nImage->algorithmBasePtr = creator[i](nImage, data))
            {
                nImage->xImageBasePtr->create = creator[i];
                break;
            }
        }
    }

	/* was a valid registered alogrith found ? */
    if(nImage->algorithmBasePtr == NULL) 
    {
    	/* nope, so we are going to bail */
    	duck_free(nImage->xImageBasePtr);
    	duck_free(nImage);

    	return NULL;
	}
	
    nImage->xImageBasePtr->dkFlags.inUse = 1;
    nImage->xImageBasePtr->addr = data;

    return nImage;
}


unsigned int *
DXL_GetFourCCList(void)
{
	/*********
		return a list of all supported fourccs
	*********/
	return fourCC;
}


int 
DXL_GetAlgHandle(unsigned int fourcc)
{
	/*********
		search through the fourcc table to find a dx'er's index
	*********/
	int i;

    for (i = 0; i < NUM_ALG; i++)
		if (fourCC[i] == fourcc) 
            return i;

	return DXL_NOTINUSE;
}


unsigned int 
DXL_GetXImageFOURCC(DXL_XIMAGE_HANDLE src)
{
	/*********
		find an ximages fourcc (by comparing creator functions)
	*********/
	int i;

    for (i = 0; i < NUM_ALG; i++)
		if (creator[i] == (CREATE_FUNC) src->xImageBasePtr->create) 
        {
			return fourCC[i];
        }

	return 0;
}

int 
DXL_dxImageToVScreen(DXL_XIMAGE_HANDLE src, DXL_VSCREEN_HANDLE dst)
{
    int dxvCode; 

	validateXImage(src);

    /* 
        after a ximage is created, it must always be altered....  this check will 
        catch programmers who do not follow the api
    */
    if(!src->xImageBasePtr->dkFlags.allocated)
        return DXL_NOTINUSE;

    if(!src->xImageBasePtr->dx)
        return DXL_NOTINUSE;

	//if(!src->xImageBasePtr->addr)
	//	return DXL_HOLD_FRAME;

	dxvCode = src->xImageBasePtr->dx(src, dst);

    return dxvCode;
}

/*-------------------------------------------------------------------

-------------------------------------------------------------------*/
int 
DXL_InitVideo(void)
{
    /* this will force the internal fourcc and creator arrays to be set to 0 */
	DXL_RegisterXImage(NULL, 0L);

	return DXL_OK;
}


void 
DXL_ExitVideo(void)
{                                     

}


DXL_XIMAGE_HANDLE 
DXL_AlterXImage(DXL_XIMAGE_HANDLE src, unsigned char *data, int type,
    				enum BITDEPTH bitDepth, int width, int height)
{
    type = toUpperFOURCC(type);

    if (src == NULL)
	{
		if(type) /* if type specified, try using it as the fourcc */
			src = DXL_CreateXImageOfType(data,type);

		if (src == NULL) /* if still null, give up */
			return NULL;
	}

    /* no way to recreate, assume create is good enough */
    if (!src->xImageBasePtr->recreate) 
        return src;


 	src->xImageBasePtr->addr = data;

	src->algorithmBasePtr = src->xImageBasePtr->recreate(src, data, type, bitDepth, width, height);


	/* was a valid registered alogrith found ? */
    if(src->algorithmBasePtr == NULL) 
    {
    	/* nope, so we are going to bail */
    	duck_free(src->xImageBasePtr);
    	duck_free(src);

    	return NULL;
	}

    
	src->xImageBasePtr->dkFlags.allocated = 1;

    return src;
}


int 
DXL_SetParameter(DXL_XIMAGE_HANDLE src, int Command, unsigned int Parameter )
{
	if (src == NULL) 
		return DXL_NULLSOURCE;

	if (src->xImageBasePtr == NULL) 
		return DXL_NULLSOURCE;

	if(src->xImageBasePtr->setParameter == NULL)
		return DXL_NULLSOURCE;

	src->xImageBasePtr->setParameter(src, Command, Parameter);

    return DXL_OK;
}

int 
DXL_GetParameter(DXL_XIMAGE_HANDLE src, int Command, unsigned int Parameter )
{
	if (src == NULL) 
		return DXL_NULLSOURCE;

	if (src->xImageBasePtr == NULL) 
		return DXL_NULLSOURCE;

	if(src->xImageBasePtr->getParameter == NULL)
   		return DXL_NULLSOURCE;

	return src->xImageBasePtr->getParameter(src, Command, Parameter);
}

DXL_HANDLE
DXL_GetAlgorithmBasePtr(DXL_XIMAGE_HANDLE src) 
{
	return src->algorithmBasePtr;
}

int
DXL_SendVideoMessage(DXL_XIMAGE_HANDLE src, void *msgHandle, unsigned int msgSize)
{
	validateXImage(src);

   	if(src->xImageBasePtr->sendVideoMessage != NULL)
		return src->xImageBasePtr->sendVideoMessage(src, msgHandle, msgSize);

    return DXL_OK;
}



/*-------------------------------------------------------------------
	CALLBACK REGISTRATION SECTION
-------------------------------------------------------------------*/
int 
DXL_RegisterXImage(CREATE_FUNC myCreator, unsigned int fourcc)
{
    int i;
    
    /* special case -- a fourcc of zero will set the creator and fourcc arrays to 0 */
    if (!fourcc)
    {
		duck_memset(creator, 0, sizeof(creator));
		duck_memset(fourCC, 0, sizeof(fourCC));
        return DXL_OK;
    }
            
    for (i = 0; i < NUM_ALG; i++)
    {
        if (!fourCC[i])
        {
            creator[i] = myCreator;
			fourCC[i] = fourcc;

            return i;
        }
    }
    return DXL_NOTINUSE;
}

int 
DXL_RegisterXImageRecreate(DXL_XIMAGE_HANDLE src, RECREATE_FUNC thisFunc)
{
    src->xImageBasePtr->recreate = thisFunc;

    return DXL_OK;
}

int 
DXL_RegisterXImageDestroy(DXL_XIMAGE_HANDLE src, DESTROY_FUNC thisFunc)
{
    src->xImageBasePtr->destroy = thisFunc;

    return DXL_OK;
}

int 
DXL_RegisterXImageDx(DXL_XIMAGE_HANDLE src, DX_FUNC thisFunc)
{
    src->xImageBasePtr->dx = thisFunc;

    return DXL_OK;
}

int 
DXL_RegisterXImageSetParameter(DXL_XIMAGE_HANDLE src, SET_PARAMETER_FUNC thisFunc)
{
    src->xImageBasePtr->setParameter = thisFunc;

    return DXL_OK;
}

int 
DXL_RegisterXImageGetParameter(DXL_XIMAGE_HANDLE src, GET_PARAMETER_FUNC thisFunc)
{
    src->xImageBasePtr->getParameter = thisFunc;

    return DXL_OK;
}

int 
DXL_RegisterXImageSendVideoMessage(DXL_XIMAGE_HANDLE src, SEND_VMSG_FUNC thisFunc)
{
    src->xImageBasePtr->sendVideoMessage = thisFunc;

    return DXL_OK;
}

/*-------------------------------------------------------------------

-------------------------------------------------------------------*/

