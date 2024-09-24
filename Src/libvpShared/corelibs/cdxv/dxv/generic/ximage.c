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



int DXL_SetXImageCSize(DXL_XIMAGE_HANDLE src, int temp)
{
	src->fSize = temp;

	return temp;
}

void DXL_DestroyXImage(DXL_XIMAGE_HANDLE src)
{

	{
#pragma warning(disable:4210) //nonstandard extension used : function given file scope
		void DXL_AccurateTime(UINT64* time);
#pragma warning(default:4210) //nonstandard extension used : function given file scope

		UINT64 clocksTotal;
		double ratio, ratio2;
		DXL_AccurateTime(&src->prof.profileEnd);
		clocksTotal = (src->prof.profileEnd - src->prof.profileStart);
		if (clocksTotal)
		{
			ratio = src->prof.dxClocks * 1.0 / clocksTotal;
			ratio2 = (double)(src->prof.dxClocks / src->prof.frameCount);
		}
	}


	if (src != NULL)
	{
		if (src->dkFlags.inUse)
		{
			src->destroy(src);
		}
	}
}

int DXL_MoveXImage(DXL_XIMAGE_HANDLE src,enum OFFSETXY mode, int x,int y)
{
	validate(src);

	if (mode != DXL_RELATIVE){
		src->x = 0;
		src->y = 0;
	}
	src->x = (short)(src->x + x);
	src->y = (short)(src->y + y);
	return DXL_OK;
}

int DXL_AlterXImageData(DXL_XIMAGE_HANDLE src, unsigned char *data)
{                    
	validate(src);

	src->addr = data;       
	src->dkFlags.DXed = 0;

	if (data == NULL) return DXL_OK;

	if (src->seedData)
		return src->seedData(src);
	else
		return 0;
}

int DXL_GetXImageXYWH(DXL_XIMAGE_HANDLE src,int *x,int *y,int *w, int *h)
{
	validate(src);

	*x = src->x;
	*y = src->y;
	*w = src->w;
	*h = src->h;

	return DXL_OK;
}

int DXL_IsXImageKeyFrame(DXL_XIMAGE_HANDLE src)
{
	validate(src);

	return src->dkFlags.keyFrame;
}

/* typedef DXL_XIMAGE_HANDLE (*createFunc)(unsigned char *data);   */
#define NUM_ALG 16
static createFunc creator[NUM_ALG];
static unsigned long fourCC[NUM_ALG];

DXL_XIMAGE_HANDLE DXL_CreateXImage(unsigned char *data)
{
	int i;
	DXL_XIMAGE_HANDLE nImage = NULL;

	for(i = 0; i < NUM_ALG; i++){
		if (fourCC[i]){
			nImage = creator[i](data);
			if ( nImage )
				break;
		}else
			break;
	}

	if (nImage) 
	{
		nImage->dkFlags.inUse = 1;
		nImage->addr = data;
		nImage->create = (struct tXImage *(__cdecl *)(void *))creator[i];
	}

	return nImage;
}




DXL_XIMAGE_HANDLE DXL_CreateXImageOfType(unsigned char *data,unsigned long type)
{
	int i;
	DXL_XIMAGE_HANDLE nImage = NULL;

	for(i = 0; i < NUM_ALG; i++){
		if (fourCC[i] == type){
			nImage = creator[i](data);
			if ( nImage )
				break;
		}
	}

	if (nImage) 
	{
		nImage->dkFlags.inUse = 1;
		nImage->addr = data;
		nImage->prof.profileStart = 0;
		nImage->prof.dxClocks = 0;
		nImage->prof.frameCount = 0;
	}

	return nImage;
}




DXL_XIMAGE_HANDLE DXL_CreateXImageFromBMI(
	unsigned char *data, 
	unsigned long fcc, 
	DK_BITMAPINFOHEADER *srcAndDest  /* There will always be two Obiwan */
	)
{
	int i;
	DXL_XIMAGE_HANDLE nImage = NULL;

	for(i = 0; i < NUM_ALG; i++){
		if (fourCC[i] == fcc){
			nImage = creator[i]((unsigned char *) srcAndDest);
			if ( nImage )
				break;
		}
	}

	if (nImage) 
	{
		nImage->dkFlags.inUse = 1;
		nImage->addr = data;
		duck_memset(&nImage->prof,0,sizeof(DXL_PROFILEPACK)); /* probably redundent */
	}

	return nImage;
}






int DXL_RegisterXImage(createFunc myCreator,unsigned long fourcc, DXL_INTERNAL_FORMAT xFormat)
{
	int i;

	if (!fourcc){
		duck_memset(creator,0,sizeof(creator));
		duck_memset(fourCC,0,sizeof(fourCC));
		return 0;
	}

	for (i = 0; i < sizeof(fourCC)/sizeof(unsigned long);i++){
		if (!fourCC[i]){
			creator[i] = myCreator;
			fourCC[i] = fourcc;
			dxl_RegisterInternalFormat(i, xFormat);
			return i;
		}
	}
	return -1;
}

unsigned long *DXL_GetFourCCList(void)
{
	/*********
	return a list of all supported fourccs
	*********/
	return fourCC;
}


int dxl_GetAlgHandle(unsigned long fourcc)
{
	/*********
	search through the fourcc table to find a dx'er's index
	*********/
	int i;

	for (i = 0; i < sizeof(fourCC)/sizeof(unsigned long);i++)
		if (fourCC[i] == fourcc) return i;

	return -1;
}


unsigned long DXL_GetXImageFOURCC(DXL_XIMAGE_HANDLE src)
{
	/*********
	find an ximages fourcc (by comparing creator functions)
	*********/
	int i;

	for (i = 0; i < sizeof(fourCC)/sizeof(unsigned long);i++)
		if (creator[i] == (createFunc)src->create) 
			return fourCC[i];

	return 0L;
}

unsigned char *DXL_GetDestAddress(DXL_XIMAGE_HANDLE src, DXL_VSCREEN_HANDLE dst)
{
	/*********
	get the address within the vscreen to start writing at
	*********/
	unsigned char *scrnDest = (unsigned char *)0L;
	int x,y;

	y = dst->viewY + src->y;           
	x = dst->viewX + src->x;

	scrnDest = (unsigned char *) dst->addr;
	scrnDest += (x * DXL_GetVScreenSizeOfPixel(dst)) + (y * dst->pitch);

	return scrnDest;
}

int DXL_dxImageToVScreen(DXL_XIMAGE_HANDLE src, DXL_VSCREEN_HANDLE dst)
{
	int dxvCode; 
	validate(src);

	if (!src->dx)
		return -1;

#pragma warning(disable:4054) // typecase from function pointer to data pointer
	if(dst && ((void *)(src->internalFormat) != NULL)) {
		/* get your hamdy damdy((c)1997 Duck North) registered blitter setup */
		dst->blitSetup = DXL_GetBlitSetupFunc(src,dst);
		dst->blitExit = DXL_GetBlitExitFunc(src,dst);
		dst->blitter = DXL_GetBlitFunc(src, dst); 

		if (dst->blitter ==  (void *) -1)
			return DXL_INVALID_BLIT;
	}
#pragma warning(default:4054) // typecase from function pointer to data pointer

	//	if (!src->addr)
	//		return 1;

#if 1  /* we want to profile ... this should constitute no performance hit to profile */
	{
		UINT64 timerStart;
		UINT64 timerEnd;

		void DXL_AccurateTime(UINT64* time);
		DXL_AccurateTime(&timerStart);

		if (src->prof.profileStart == 0)
			src->prof.profileStart = timerStart;	
		dxvCode = src->dx(src,dst);
		DXL_AccurateTime(&timerEnd);
		src->prof.dxClocks += (timerEnd - timerStart);
		src->prof.frameCount += 1;
	}
#else
	dxvCode = src->dx(src,dst);
#endif


	return dxvCode;
}


long DXL_GetXImageCSize(DXL_XIMAGE_HANDLE src)
{
	if (src == NULL) return -1;

	if (!src->GetXImageCSize)
		return -2;

	return(src->GetXImageCSize(src));
}

/***********************************************/

DXL_XIMAGE_HANDLE DXL_AlterXImage(DXL_XIMAGE_HANDLE src,
																	unsigned char *data,int type,
																	enum BITDEPTH bitDepth,int width,int height)
{
	if (src == NULL)
	{
		if (type) /* if type specified, try using it as the fourcc */
			src = DXL_CreateXImageOfType(data,type);

		if (src == NULL) /* if still null, try creating it blind from the data */
			src = DXL_CreateXImage(data);

		if (src == NULL) /* if still null, give up */
			return NULL;
	}

	if (!src->recreate) /* no way to recreate, assume create is good enough */
		return src;

	return(src->recreate(src,data,type,bitDepth,width,height));
}


void DXL_SetParameter(DXL_XIMAGE_HANDLE src, int Command, unsigned long Parameter )
{
	src->setParameter(src,Command,Parameter);
}
