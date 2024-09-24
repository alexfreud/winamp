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


#ifndef MISCASM_H
#define MISCASM_H

void CC_RGB32toYV12_MMXLU( unsigned char *RGBABuffer, int ImageWidth, int ImageHeight,
                           unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer );

#endif MISCASM_H
