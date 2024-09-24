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


#ifndef ASMCOLORCONVERSIONS_H
#define ASMCOLORCONVERSIONS_H

void CC_RGB32toYV12_MMX( unsigned char *RGBABuffer, int ImageWidth, int ImageHeight,
                         unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );

void CC_RGB32toYV12_XMM( unsigned char *RGBABuffer, int ImageWidth, int ImageHeight,
                         unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );

void CC_RGB24toYV12_MMX( unsigned char *RGBBuffer, int ImageWidth, int ImageHeight,
                         unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );

void CC_RGB24toYV12_XMM( unsigned char *RGBBuffer, int ImageWidth, int ImageHeight,
                         unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );

void CC_UYVYtoYV12_MMX( unsigned char *UYVYBuffer, int ImageWidth, int ImageHeight,
                        unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch, int DstPitch );

void CC_UYVYtoYV12_XMM( unsigned char *UYVYBuffer, int ImageWidth, int ImageHeight,
                        unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch, int DstPitch );

void CC_YUY2toYV12_MMX( unsigned char *YUY2Buffer, int ImageWidth, int ImageHeight,
                        unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch, int DstPitch );

void CC_YUY2toYV12_XMM( unsigned char *YUY2Buffer, int ImageWidth, int ImageHeight,
                        unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch, int DstPitch );

void CC_YVYUtoYV12_MMX( unsigned char *YVYUBuffer, int ImageWidth, int ImageHeight,
                        unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );

void CC_YVYUtoYV12_XMM( unsigned char *YVYUBuffer, int ImageWidth, int ImageHeight,
                        unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );


#endif /* ASMCOLORCONVERSIONS_H */
