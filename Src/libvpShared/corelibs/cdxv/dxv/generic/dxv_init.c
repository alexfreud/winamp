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


#if defined(DISPLAYDIB)
#if DISPLAYDIB
int globalIsDIB,globalDIBWidth,globalDIBHeight;
#endif
#endif    

extern int preallocVScreens(int lmaxScreens);
extern void freeVScreens(void);


int DXL_InitVideo(int lmaxScreens,int lmaxImages)
{
	(void)lmaxImages; // Not Used;
/*    registerDuckBlitters(); */
	resetBlitters();

//	DXL_RegisterXImage(NULL,0L,(DXL_INTERNAL_FORMAT ) 0);

	preallocVScreens(lmaxScreens);
	return DXL_OK;
}


void DXL_ExitVideo(void)
{                                     
    freeVScreens();
}
