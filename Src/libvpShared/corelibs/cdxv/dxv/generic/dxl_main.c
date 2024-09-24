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

static DXL_VSCREEN_HANDLE vScreens = NULL;
static int maxScreens;

int preallocVScreens(int lmaxScreens)
{
	(void) lmaxScreens;  // not used
#if PRE_ALLOCATE
    vScreens = (DXL_VSCREEN_HANDLE)duck_calloc(maxScreens = lmaxScreens,sizeof(DXL_VSCREEN),DMEM_GENERAL);
    
	if (vScreens == NULL) 
		return DXL_ALLOC_FAILED;
#endif
	return DXL_OK;
}

void freeVScreens(void)
{                                     
#if PRE_ALLOCATE
    int i;
    
    if (vScreens)
	{
        for(i = 0; i < maxScreens; i++)
            DXL_DestroyVScreen(&vScreens[i]);
        duck_free(vScreens);
    }
#endif
}
        
DXL_VSCREEN_HANDLE vScreenCreate(void)
{
	DXL_VSCREEN_HANDLE nScreen;

#if PRE_ALLOCATE
	if (vScreens)
	{
		int i;
		nScreen = vScreens;
    
		for(i=0; i < maxScreens; i++,nScreen++)
			if (!nScreen->dkFlags.inUse) break;

		if (i < maxScreens) 
			return nScreen;
    }
#endif

    nScreen = (DXL_VSCREEN_HANDLE)duck_calloc(1,sizeof(DXL_VSCREEN),DMEM_GENERAL);
	
	if (nScreen)
		nScreen->dkFlags.allocated = 1;

    return nScreen;
}
