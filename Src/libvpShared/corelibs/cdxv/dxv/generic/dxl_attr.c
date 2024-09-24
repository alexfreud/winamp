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


#include "dxl_main.h"                                            

int DXL_SetVScreenBlitQuality(DXL_VSCREEN_HANDLE dst, enum BLITQUALITY blitquality)                  
{
	int oldBQ;
	
	validate(dst);

	oldBQ = dst->bq;
	dst->bq = blitquality;

	return oldBQ;
}

enum BLITQUALITY DXL_GetVScreenBlitQuality(DXL_VSCREEN_HANDLE dst)                  
{
	if (dst)	{
		return dst->bq;
	}
	return DXBLIT_SAME;
}
