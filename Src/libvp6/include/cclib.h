#ifndef _CCLIB_H
#define _CCLIB_H
#include "cpuidlib.h"

#ifdef __cplusplus
extern "C"
{
#else
#if !defined(bool)
    typedef int bool;
#endif
#endif

int InitCCLib( PROCTYPE CpuType );

void DeInitCCLib( void );

extern void (*RGB32toYV12)( unsigned char *RGBABuffer, int ImageWidth, int ImageHeight,
                            unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );

extern void (*RGB24toYV12)( unsigned char *RGBBuffer, int ImageWidth, int ImageHeight,
                            unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );

extern void (*UYVYtoYV12)( unsigned char *UYVYBuffer, int ImageWidth, int ImageHeight,
                           unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );

extern void (*YUY2toYV12)( unsigned char *UYVYBuffer, int ImageWidth, int ImageHeight,
                           unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );

extern void (*YVYUtoYV12)( unsigned char *YVYUBuffer, int ImageWidth, int ImageHeight,
                           unsigned char *YBuffer, unsigned char *UBuffer, unsigned char *VBuffer, int SrcPitch,int DstPitch );

extern void RGB24toYV12F
(
    unsigned char *RGBBuffer,
    int ImageWidth,
    int ImageHeight,
    unsigned char *YBuffer,
    unsigned char *UBuffer,
    unsigned char *VBuffer,
    int SrcPitch,
    int DstPitch
);

extern void RGB32toYV12F
(
    unsigned char *RGBBuffer,
    int ImageWidth,
    int ImageHeight,
    unsigned char *YBuffer,
    unsigned char *UBuffer,
    unsigned char *VBuffer,
    int SrcPitch,
    int DstPitch
);

extern void UYVYtoYV12F
(
    unsigned char *UYVYBuffer,
    int ImageWidth,
    int ImageHeight,
    unsigned char *YBuffer,
    unsigned char *UBuffer,
    unsigned char *VBuffer,
    int SrcPitch,
    int DstPitch
);

extern void YUY2toYV12F
(
    unsigned char *YUY2Buffer,
    int ImageWidth,
    int ImageHeight,
    unsigned char *YBuffer,
    unsigned char *UBuffer,
    unsigned char *VBuffer,
    int SrcPitch,
    int DstPitch
);

extern void YVYUtoYV12F
(
    unsigned char *YVYUBuffer,
    int ImageWidth,
    int ImageHeight,
    unsigned char *YBuffer,
    unsigned char *UBuffer,
    unsigned char *VBuffer,
    int SrcPitch,
    int DstPitch
);

/*
 * Macros to make it easier to call the needed functions
 */
#define CC_RGB32toYV12( _RGBABuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer ) \
        (*RGB32toYV12)( _RGBABuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer, _ImageWidth*4, _ImageWidth )

#define CC_RGB24toYV12( _RGBBuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer ) \
        (*RGB24toYV12)( _RGBBuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer, _ImageWidth*3, _ImageWidth )

#define CC_UYVYtoYV12( _UYVYBuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer ) \
        (*UYVYtoYV12)( _UYVYBuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer, _ImageWidth*2, _ImageWidth )

#define CC_YUY2toYV12( _YUY2Buffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer ) \
        (*YUY2toYV12)( _YUY2Buffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer, _ImageWidth*2, _ImageWidth )

#define CC_YVYUtoYV12( _YVYUBuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer ) \
        (*YVYUtoYV12)( _YVYUBuffer, _ImageWidth, _ImageHeight, _YBuffer, _UBuffer, _VBuffer, _ImageWidth*2, _ImageWidth )

// super generic rgb to yuv color conversion can handle any rgb to yuv conversion
// provided r,g,b components are 1 byte apiece, and that the resulting y is 1 byte
extern void ConvertRGBtoYUV(
    const unsigned char* const pucSourceR, const unsigned char* const pucSourceG, const unsigned char* const pucSourceB,
    int width, int height, int rgb_step, int rgb_pitch,
    unsigned char* const pucDestY, unsigned char* const pucDestU, unsigned char* const pucDestV,
    int uv_width_shift, int uv_height_shift,
    int y_step, int y_pitch,int uv_step,int uv_pitch);

extern void ConvertRGBtoYUVI(
    const unsigned char* const pucSourceR, const unsigned char* const pucSourceG, const unsigned char* const pucSourceB,
    int iWidth, int iHeight, int iStepRGB, int iStrideRGB,
    unsigned char* const pucDestY, unsigned char* const pucDestU, unsigned char* const pucDestV,
    int uv_width_shift, int uv_height_shift,
    int iStepY, int iStrideY, int iStepUV, int iStrideUV);

//  This assumes 3 byte RGB data with the same component ordering in the source and destination
extern void ConvertRGBtoRGB(const unsigned char* const pucSource, long lWidth, long lHeight, long lStepIn, long lStrideIn,
    unsigned char* const pucDest, long lStepOut, long lStrideOut);

extern void ConvertYUVtoRGB(
    const unsigned char* const pucYPlane, const unsigned char* const pucUPlane, const unsigned char* const pucVPlane,
    long lWidth, long lHeight,
    long uv_width_shift, long uv_height_shift,  //  not used, should both be set to 1
    long lStepY, long lStrideY,long lStepUV, long lStrideUV,
    unsigned char* const pucRPlane, unsigned char* const pucGPlane, unsigned char* const pucBPlane,
    long lStepRGB, long lStrideRGB);

extern void ConvertYUVItoRGB(
    const unsigned char* const pucYPlane, const unsigned char* const pucUPlane, const unsigned char* const pucVPlane,
    long lWidth, long lHeight,
    long uv_width_shift, long uv_height_shift,  //  not used, should both be set to 1
    long lStepY, long lStrideY,long lStepUV, long lStrideUV,
    unsigned char* const pucRPlane, unsigned char* const pucGPlane, unsigned char* const pucBPlane,
    long lStepRGB, long lStrideRGB);

extern void ConvertYUVtoRGB2(
    const unsigned char* const pucYPlane, const unsigned char* const pucUPlane, const unsigned char* const pucVPlane,
    long lWidth, long lHeight,
    long uv_width_shift, long uv_height_shift,  //  not used, should both be set to 1
    long lStepY, long lStrideY,long lStepUV, long lStrideUV,
    unsigned char* const pucRPlane, unsigned char* const pucGPlane, unsigned char* const pucBPlane,
    long lStepRGB, long lStrideRGB,
    bool bSupersampleHoriz, bool bSupersampleVert);

extern void ConvertYUVItoRGB2(
    const unsigned char* const pucYPlane, const unsigned char* const pucUPlane, const unsigned char* const pucVPlane,
    long lWidth, long lHeight,
    long uv_width_shift, long uv_height_shift,  //  not used, should both be set to 1
    long lStepY, long lStrideY,long lStepUV, long lStrideUV,
    unsigned char* const pucRPlane, unsigned char* const pucGPlane, unsigned char* const pucBPlane,
    long lStepRGB, long lStrideRGB,
    bool bSupersampleHoriz, bool bSupersampleVert);

//  This assumes packed planar data
extern void ConvertYUVtoYUV(const unsigned char* const pucYIn, const unsigned char* const pucUV1In, const unsigned char* const pucUV2In,
    long m_lWidth, long m_lHeight,
    unsigned char* const pucYOut, unsigned char* const pucUV1Out, unsigned char* const pucUV2Out);

//  This assumes packed planar data
extern void ConvertYUVtoYUVI(const unsigned char* const pucYIn, const unsigned char* const pucUV1In, const unsigned char* const pucUV2In,
    long m_lWidth, long m_lHeight,
    unsigned char* const pucYOut, unsigned char* const pucUV1Out, unsigned char* const pucUV2Out);

//  This assumes packed planar data
extern void ConvertYUVItoYUV(const unsigned char* const pucYIn, const unsigned char* const pucUV1In, const unsigned char* const pucUV2In,
    long m_lWidth, long m_lHeight,
    unsigned char* const pucYOut, unsigned char* const pucUV1Out, unsigned char* const pucUV2Out);

#ifdef __cplusplus
}
#endif
#endif /* _CCLIB_H */
