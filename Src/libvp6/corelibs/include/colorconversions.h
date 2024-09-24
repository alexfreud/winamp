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


#ifndef COLORCONVERSIONS_H
#define COLORCONVERSIONS_H

#define ScaleFactor      0x8000
#define ShiftFactor      15

#define PixelsPerBlock      4
#define PixelsPerBlockShift 2

void CC_RGB32toYV12_C( unsigned char *RGBABuffer, int ImageWidth, int ImageHeight,
                       unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );

void CC_RGB24toYV12_C( unsigned char *RGBBuffer, int ImageWidth, int ImageHeight,
                       unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );

void CC_UYVYtoYV12_C( unsigned char *UYVYBuffer, int ImageWidth, int ImageHeight,
                      unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch, int DstPitch );

void CC_YUY2toYV12_C( unsigned char *YUY2Buffer, int ImageWidth, int ImageHeight,
                      unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch, int DstPitch );

void CC_YVYUtoYV12_C( unsigned char *YVYUBuffer, int ImageWidth, int ImageHeight,
                      unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );

#endif /* COLORCONVERSIONS_H */
