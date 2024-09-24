/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2009 Intel Corporation. All Rights Reserved.
//
//
//                  Intel(R) Performance Primitives
//                  Color Conversion Library (ippCC)
//
*/
#if !defined( __IPPCC_H__ ) || defined( _OWN_BLDPCS )
#define __IPPCC_H__

#if defined (_WIN32_WCE) && defined (_M_IX86) && defined (__stdcall)
  #define _IPP_STDCALL_CDECL
  #undef __stdcall
#endif

#ifndef __IPPDEFS_H__
  #include "ippdefs.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined( _OWN_BLDPCS )

typedef enum {
    ippDitherNone,
    ippDitherFS,
    ippDitherJJN,
    ippDitherStucki,
    ippDitherBayer
} IppiDitherType;

#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                   Functions declarations
////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////// */


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippccGetLibVersion
//  Purpose:    getting of the library version
//  Returns:    the structure of information about version
//              of ippCC library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
IPPAPI( const IppLibraryVersion*, ippccGetLibVersion, (void) )

/* /////////////////////////////////////////////////////////////////////////////
//                   Color Space  Conversion Functions
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiCbYCr422ToBGR_709HDTV_8u_C2C3R
//  Purpose:    Converts a UYVY image to the BGR24 image
//  Name:       ippiCbYCr422ToBGR_709HDTV_8u_C2C4R
//  Purpose:    Converts a UYVY image to the BGRA image
//  Name:       ippiBGRToCbYCr422_709HDTV_8u_C3C2R
//  Purpose:    Converts a BGR24 image to the UYVY image
//  Name:       ippiBGRToCbYCr422_709HDTV_8u_AC4C2R
//  Purpose:    Converts a BGRA  image to the UYVY image
//  Name:       ippiYCbCr420ToBGR_709HDTV_8u_P3C4R
//  Purpose:    Converts a I420(IYUV) image to the BGRA image
//  Name:       ippiBGRToYCbCr420_709HDTV_8u_AC4P3R
//  Purpose:    Converts a BGRA image to the I420(IYUV) image
//  Parameters:
//     pSrc     Pointer to the source image (for pixel-order data).An array of
//              pointers to separate source color planes (for plane-order data)
//     pDst     Pointer to the destination image (for pixel-order data).An array
//              of pointers to separate destination color planes (for plane-order data)
//     roiSize  Size of source and destination ROI in pixels
//     srcStep  Step in bytes through the source image to jump on the next line
//     dstStep  Step in bytes through the destination image to jump on the next line
//  Returns:
//     ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//     ippStsSizeErr     roiSize has field with zero or negative value
//     ippStsNoErr       No errors
//  Reference:
//      Jack Keith
//      Video Demystified: A Handbook for the Digital Engineer, 2nd ed.
//      1996.pp.(42-43)
//    YCbCr <> RGB ITU_R BT.709 (HDTV) where R'G'B' has a range of 16-235
//       Y has a range of 16 - 235. Cb,Cr have a range of 16 - 240
//       Y  =  0.2126*R' + 0.7152*G' + 0.0722*B'
//       Cb = -0.117*R'  - 0.394*G'  + 0.511 * B' + 128
//       Cr =  0.511*R'  - 0.464*G'  - 0.047 * B' + 128
//       Cb = 0.5389 (B' - Y )
//       Cr = 0.6350 (R' - Y )
//       Digital representation.
//       Quantization level assignment:
//         - Video data:        1 through 254
//         - Timing reference:  0 and 255
//       R' = Y + 1.540*(Cr - 128 )
//       G' = Y - 0.459*(Cr - 128 ) - 0.183*( Cb - 128 )
//       B' = Y + 1.816*(Cb - 128)
*/
IPPAPI(IppStatus, ippiCbYCr422ToBGR_709HDTV_8u_C2C3R,(const Ipp8u* pSrc,int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiCbYCr422ToBGR_709HDTV_8u_C2C4R,(const Ipp8u* pSrc,int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp8u aval ))
IPPAPI(IppStatus, ippiBGRToCbYCr422_709HDTV_8u_C3C2R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiBGRToCbYCr422_709HDTV_8u_AC4C2R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToBGR_709HDTV_8u_P3C4R, (const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst   , int dstStep   , IppiSize roiSize, Ipp8u aval))
IPPAPI(IppStatus, ippiBGRToYCbCr420_709HDTV_8u_AC4P3R,(const Ipp8u* pSrc   , int srcStep   , Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Parameters:
//     pSrc     Pointer to the source image (for pixel-order data).An array of
//              pointers to separate source color planes (for plane-order data)
//     pDst     Pointer to the destination image (for pixel-order data).An array
//              of pointers to separate destination color planes (for plane-order data)
//     roiSize  Size of source and destination ROI in pixels
//     srcStep  Step in bytes through the source image to jump on the next line
//     dstStep  Step in bytes through the destination image to jump on the next line
//  Returns:
//     ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//     ippStsSizeErr     roiSize has field with zero or negative value
//     ippStsNoErr       No errors
//  Reference:
//      Jack Keith
//      Video Demystified: A Handbook for the Digital Engineer, 2nd ed.
//      1996.pp.(42-43)
//    YCbCr <> RGB ITU_R BT.709 (Computer System Considerations) where R'G'B' has a range of 0-255
//    Y  =  0.183*R' + 0.614*G' + 0.062*B' + 16
//    Cb = -0.101*R' - 0.338*G' + 0.439*B' + 128
//    Cr =  0.439*R' - 0.399*G' - 0.040*B' + 128
//    R' = 1.164*(Y - 16) + 1.793*(Cr - 128 )
//    G' = 1.164*(Y - 16) - 0.534*(Cr - 128 )- 0.213*( Cb - 128 )
//    B' = 1.164*(Y - 16) + 2.115*(Cb - 128 )
*/

  IPPAPI(IppStatus, ippiBGRToYCbCr420_709CSC_8u_AC4P3R,(const Ipp8u*  pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
  IPPAPI(IppStatus, ippiBGRToYCrCb420_709CSC_8u_AC4P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))

  IPPAPI(IppStatus, ippiBGRToYCbCr420_709CSC_8u_C3P3R,(const Ipp8u*  pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
  IPPAPI(IppStatus, ippiBGRToYCrCb420_709CSC_8u_C3P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))

  IPPAPI(IppStatus, ippiYCbCr420ToBGR_709CSC_8u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp8u* pDst, int dstStep, IppiSize roiSize))
  IPPAPI(IppStatus, ippiYCbCrToBGR_709CSC_8u_P3C3R,( const Ipp8u* pSrc[3],int srcStep,Ipp8u* pDst,int dstStep,IppiSize roiSize ))
  IPPAPI(IppStatus, ippiYCbCrToBGR_709CSC_8u_P3C4R,( const Ipp8u* pSrc[3],int srcStep,Ipp8u* pDst,int dstStep,IppiSize roiSize, Ipp8u aval))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//  ippiBGRToCbYCr422_8u_AC4C2R,       ippiCbYCr422ToBGR_8u_C2C4R,
//  ippiYCbCr411ToBGR_8u_P3C3R
//  ippiYCbCr411ToBGR_8u_P3C4R
//
//  ippiRGBToCbYCr422_8u_C3C2R,        ippiCbYCr422ToRGB_8u_C2C3R,
//  ippiRGBToCbYCr422Gamma_8u_C3C2R,
//  ippiYCbCr422ToRGB_8u_P3C3R
//
//  ippiRGBToYCbCr422_8u_C3C2R,        ippiYCbCr422ToRGB_8u_C2C3R,
//  ippiRGBToYCbCr422_8u_P3C2R,        ippiYCbCr422ToRGB_8u_C2P3R,
//
//  ippiRGBToYCbCr420_8u_C3P3R,        ippiYCbCr420ToRGB_8u_P3C3R,
//  ippiYCbCr420ToBGR_8u_P3C3R,
//
//  ippiYCbCr422ToRGB565_8u16u_C2C3R,  ippiYCbCr422ToBGR565_8u16u_C2C3R,
//  ippiYCbCr422ToRGB555_8u16u_C2C3R,  ippiYCbCr422ToBGR555_8u16u_C2C3R,
//  ippiYCbCr422ToRGB444_8u16u_C2C3R,  ippiYCbCr422ToBGR444_8u16u_C2C3R,
//
//  ippiYCbCrToRGB565_8u16u_P3C3R,     ippiYCbCrToBGR565_8u16u_P3C3R,
//  ippiYCbCrToRGB444_8u16u_P3C3R,     ippiYCbCrToBGR444_8u16u_P3C3R,
//  ippiYCbCrToRGB555_8u16u_P3C3R,     ippiYCbCrToBGR555_8u16u_P3C3R,
//
//  ippiYCbCr420ToRGB565_8u16u_P3C3R,  ippiYCbCr420ToBGR565_8u16u_P3C3R
//  ippiYCbCr420ToRGB555_8u16u_P3C3R,  ippiYCbCr420ToBGR555_8u16u_P3C3R,
//  ippiYCbCr420ToRGB444_8u16u_P3C3R,  ippiYCbCr420ToBGR444_8u16u_P3C3R,
//
//  ippiYCbCr422ToRGB565_8u16u_P3C3R,  ippiYCbCr422ToBGR565_8u16u_P3C3R,
//  ippiYCbCr422ToRGB555_8u16u_P3C3R,  ippiYCbCr422ToBGR555_8u16u_P3C3R,
//  ippiYCbCr422ToRGB444_8u16u_P3C3R,  ippiYCbCr422ToBGR444_8u16u_P3C3R,
//
//  ippiRGBToYCrCb422_8u_C3C2R,        ippiYCrCb422ToRGB_8u_C2C3R,
//  ippiRGBToYCrCb422_8u_P3C2R,        ippiYCrCb422ToRGB_8u_C2P3R,
//
//
//  Purpose:    Converts an RGB(BGR) image to the YCbCr (CbYCr, YCrCb) image and vice versa.
//  Parameters:
//     pSrc     Pointer to the source image (for pixel-order data).An array of
//              pointers to separate source color planes (for plane-order data)
//     pDst     Pointer to the destination image (for pixel-order data).An array
                of pointers to separate destination color planes (for plane-order data)
//     roiSize  Size of source and destination ROI in pixels
//     srcStep  Step in bytes through the source image to jump on the next line
//     dstStep  Step in bytes through the destination image to jump on the next line
//  Returns:
//     ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//     ippStsSizeErr     roiSize has field with zero or negative value
//     ippStsNoErr       No errors
//  Reference:
//      Jack Keith
//      Video Demystified: A Handbook for the Digital Engineer, 2nd ed.
//      1996.pp.(42-43)
//
//  The YCbCr color space was developed as part of Recommendation ITU-R BT.601
//  (formerly CCI 601). Y is defined to have a nominal range of 16 to 235;
//  Cb and Cr are defined to have a range of 16 to 240, with 128 equal to zero.
//  The function ippiRGBToYCbCr422_8u_P3C2R uses 4:2:2 sampling format. For every
//  two  horizontal Y samples, there is one Cb and Cr sample.
//  Each pixel in the input RGB image is of 24 bit depth. Each pixel in the
//  output YCbCr image is of 16 bit depth.
//  Sequence of samples in the YCbCr422 image is
//             Y0Cb0Y1Cr0,Y2Cb1Y3Cr1,...
//  Sequence of samples in the CbYCr422 image is:
//             Cb0Y0CrY1,Cb1Y2Cr1Y3,...
//  All functions operate on the gamma-corrected RGB (R'G'B') images
//  (except ippiRGBToCbYCrGamma_8u_C3C2R, see below) with pixel values
//  in the range 0 .. 255, as is commonly found in computer system.
//  Conversion is performed according to the following equations:
//
//       Y  =  0.257*R' + 0.504*G' + 0.098*B' + 16
//       Cb = -0.148*R' - 0.291*G' + 0.439*B' + 128
//       Cr =  0.439*R' - 0.368*G' - 0.071*B' + 128
//
//       R' = 1.164*(Y - 16) + 1.596*(Cr - 128 )
//       G' = 1.164*(Y - 16) - 0.813*(Cr - 128 )- 0.392*( Cb - 128 )
//       B' = 1.164*(Y - 16) + 2.017*(Cb - 128 )
//
//   Note that for the YCbCr-to-RGB equations, the RGB values must be saturated
//   at the 0 and 255 levels due to occasional excursions outside the nominal
//   YCbCr ranges.
//   ippiRGBToCbYCr422Gamma_8u_C3C2R function additionally performs gamma-correction, there is
//   sample down filter(1/4,1/2,1/4).
*/

IPPAPI(IppStatus, ippiCbYCr422ToBGR_8u_C2C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp8u aval))

IPPAPI(IppStatus, ippiBGRToCbYCr422_8u_AC4C2R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiYCbCr411ToBGR_8u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp8u* pDst,
       int dstStep, IppiSize roiSize))

IPPAPI(IppStatus,ippiYCbCr411ToBGR_8u_P3C4R,(const Ipp8u* pSrc[3],int srcStep[3],
       Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp8u aval))

IPPAPI(IppStatus, ippiCbYCr422ToRGB_8u_C2C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRGBToCbYCr422Gamma_8u_C3C2R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRGBToCbYCr422_8u_C3C2R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiYCbCr422ToRGB_8u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],
       Ipp8u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRGBToYCbCr422_8u_C3C2R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToRGB_8u_C2C3R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRGBToYCbCr422_8u_P3C2R,(const Ipp8u* pSrc[3], int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToRGB_8u_C2P3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst[3], int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiYCbCr420ToBGR_8u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],
       Ipp8u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiYCbCr420ToRGB_8u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYCbCr420_8u_C3P3R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst[3],int dstStep[3], IppiSize roiSize))

IPPAPI(IppStatus, ippiYCbCr422ToRGB565_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToBGR565_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiYCbCr422ToRGB555_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToBGR555_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))


IPPAPI(IppStatus, ippiYCbCr422ToRGB444_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToBGR444_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiYCbCrToBGR565_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB565_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToBGR444_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB444_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToBGR555_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB555_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiYCbCr420ToBGR565_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToRGB565_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToBGR555_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToRGB555_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToBGR444_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToRGB444_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiYCbCr422ToBGR565_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToRGB565_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToBGR555_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToRGB555_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToBGR444_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToRGB444_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRGBToYCrCb422_8u_C3C2R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCrCb422ToRGB_8u_C2C3R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYCrCb422_8u_P3C2R,(const Ipp8u* pSrc[3], int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCrCb422ToRGB_8u_C2P3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst[3], int dstStep, IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name: ippiYCbCrToBGR(RGB)565(555,444)Dither_8u16u_C3R
//        ippiYCbCrToBGR(RGB)565(555,444)Dither_8u16u_P3C3R
//        ippiYCbCr422ToBGR(RGB)565(555,444)Dither_8u16u_P3C3R
//        ippiYCbCr420ToBGR(RGB)565(555,444)Dither_8u16u_P3C3R
//        ippiYUV420ToBGR(RGB)565(555,444)Dither_8u16u_P3C3R
//  Purpose:
//      Converts a YCbCr(YUV) image to the 16-bit per pixel BGR(RGB) image with dithering.
//  Parameters:
//     pSrc   Pointer to the source image (for pixel-order data).An array of pointers
//            to separate source color planes (for plane-order data)
//     pDst   Pointer to the destination image (for pixel-order data).An array of pointers
//            to separate destination color planes (for plane-order data)
//     roiSize  Size of the source and destination ROI in pixels.
//     srcStep  Step in bytes through the source image to jump on the next line
//     dstStep  Step in bytes through the destination image to jump on the next line
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  After color conversion bit reduction is performed using Bayer's dithering algorithm
*/

IPPAPI(IppStatus, ippiYCbCrToBGR565Dither_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB565Dither_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToBGR555Dither_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB555Dither_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToBGR444Dither_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB444Dither_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToBGR565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToBGR555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToBGR444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToBGR565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToRGB565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToBGR555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToRGB555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToBGR444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToRGB444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToBGR565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToRGB565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToBGR555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToRGB555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToBGR444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToRGB444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToRGB555Dither_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiYCbCr422ToBGR555Dither_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToRGB565Dither_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToBGR565Dither_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToRGB444Dither_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToBGR444Dither_8u16u_C2C3R,(const Ipp8u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiYUV420ToBGR444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToRGB444Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToBGR555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToRGB555Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToBGR565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToRGB565Dither_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiBGR565ToYUV420_16u8u_C3P3R/ippiBGR555ToYUV420_16u8u_C3P3R
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsSizeErr         if roiSize.width < 2 or if roiSize.height < 0
//  Arguments:
//    pSrc          Pointer to the source image
//    srcStep       Step through the source image
//    pDst          An array of pointers  to separate destination color planes.
//    dstStep       An array of step in bytes through the destination planes
//    roiSize       region of interest to be processed, in pixels
*/
IPPAPI(IppStatus, ippiBGR565ToYUV420_16u8u_C3P3R,( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGR555ToYUV420_16u8u_C3P3R,( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr422ToBGR_8u_C2C4R / ippiYCbCr422ToBGR_8u_C2C3R
//  Purpose:    Converts a YUY2 image to the BGRA / RGB24 image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr        One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 2 or if roiSize.height < 1
//
//  Arguments:
//    pSrc                     pointer to the source image
//    srcStep                  step for the source image
//    pDst                     pointer to the destination image
//    dstStep                  step for the destination image
//    roiSize                 region of interest to be processed, in pixels
*/
IPPAPI(IppStatus, ippiYCbCr422ToBGR_8u_C2C4R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst,int dstStep, IppiSize roiSize, Ipp8u aval  ))
IPPAPI(IppStatus, ippiYCbCr422ToBGR_8u_C2C3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst,int dstStep, IppiSize roiSize  ))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCrToBGR_8u_P3C4R / ippiYCbCrToBGR_8u_P3C3R
//  Purpose:    Converts a P444 image to the BGRA / RGB24 image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr        One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4 or if roiSize.height < 1
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pDst                     pointer to the destination image
//    dstStep                  step for the destination image
//    roiSize                 region of interest to be processed, in pixels
*/
IPPAPI(IppStatus, ippiYCbCrToBGR_8u_P3C4R,( const Ipp8u* pSrc[3],int srcStep,Ipp8u* pDst,int dstStep,IppiSize roiSize, Ipp8u aval  ))
IPPAPI(IppStatus, ippiYCbCrToBGR_8u_P3C3R,( const Ipp8u* pSrc[3],int srcStep,Ipp8u* pDst,int dstStep,IppiSize roiSize ))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr411ToBGR565_8u16u_P3C3R / ippiYCbCr411ToBGR555_8u16u_P3C3R
//  Purpose:    Converts a P411 image to the RGB565 / RGB555 image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr        One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4 or if roiSize.height < 1
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pDst                     pointer to the destination image
//    dstStep                  step for the destination image
//     roiSize                 region of interest to be processed, in pixels
*/
IPPAPI(IppStatus, ippiYCbCr411ToBGR565_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst,int dstStep,IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCbCr411ToBGR555_8u16u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp16u* pDst,int dstStep,IppiSize roiSize ))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiBGRToYCbCr411_8u_C3P3R/ippiBGRToYCbCr411_8u_AC4P3R/ippiBGR565ToYCbCr411_16u8u_C3P3R/ippiBGR555ToYCbCr411_16u8u_C3P3R
//  Purpose:    Converts a RGB24/RGBA/RGB565/RGB565 image to the P411 image
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsSizeErr         if roiSize.width < 4 or if roiSize.height < 1
//  Arguments:
//    pSrc          Pointer to the source image
//    srcStep       Step through the source image
//    pDst          An array of pointers  to separate destination color planes.
//    dstStep       An array of step in bytes through the destination planes
//    roiSize       region of interest to be processed, in pixels
*/
IPPAPI(IppStatus, ippiBGRToYCbCr411_8u_C3P3R,   (const Ipp8u*  pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGRToYCbCr411_8u_AC4P3R,  (const Ipp8u*  pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGR565ToYCbCr411_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGR555ToYCbCr411_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiBGRToYCbCr422_8u_C3P3R/ippiBGRToYCbCr422_8u_AC4P3R/ippiBGR565ToYCbCr422_16u8u_C3P3R/ippiBGR555ToYCbCr422_16u8u_C3P3R
//  Purpose:    Converts a RGB24/RGBA/RGB565/RGB565 image to the P422 image
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsSizeErr         if roiSize.width < 2 or if roiSize.height < 1
//  Arguments:
//    pSrc          Pointer to the source image
//    srcStep       Step through the source image
//    pDst          An array of pointers  to separate destination color planes.
//    dstStep       An array of step in bytes through the destination planes
//    roiSize       region of interest to be processed, in pixels
*/
IPPAPI(IppStatus, ippiBGRToYCbCr422_8u_C3P3R,   (const Ipp8u*  pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))
IPPAPI(IppStatus, ippiBGRToYCbCr422_8u_AC4P3R,  (const Ipp8u*  pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))
IPPAPI(IppStatus, ippiBGR565ToYCbCr422_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))
IPPAPI(IppStatus, ippiBGR555ToYCbCr422_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiBGRToYCbCr420_8u_C3P3R/ippiBGRToYCbCr420_8u_AC4P3R/ippiBGR565ToYCbCr420_16u8u_C3P3R/ippiBGR555ToYCbCr420_16u8u_C3P3R
//  Purpose:    Converts a RGB24/RGBA/RGB565/RGB565 image to the IYUV image
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsSizeErr         if roiSize.width < 2 or if roiSize.height < 0
//  Arguments:
//    pSrc          Pointer to the source image
//    srcStep       Step through the source image
//    pDst          An array of pointers  to separate destination color planes.
//    dstStep       An array of step in bytes through the destination planes
//    roiSize       region of interest to be processed, in pixels
*/
IPPAPI(IppStatus, ippiBGRToYCbCr420_8u_C3P3R,   (const Ipp8u*  pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGRToYCbCr420_8u_AC4P3R,  (const Ipp8u*  pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGR565ToYCbCr420_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGR555ToYCbCr420_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiBGRToYCbCr422_8u_C3C2R/ippiBGRToYCbCr422_8u_AC4C2R/ippiBGR555ToYCbCr422_16u8u_C3C2R/ippiBGR565ToYCbCr422_16u8u_C3C2R
//  Purpose:    Converts a RGB24/RGBA/RGB565/RGB565 image to the YUY2 image
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsSizeErr         if roiSize.width < 2 or if roiSize.height < 1
//  Arguments:
//    pSrc          Pointer to the source image
//    srcStep       Step through the source image
//    pDst          An array of pointers  to separate destination color planes.
//    dstStep       An array of step in bytes through the destination planes
//    roiSize       region of interest to be processed, in pixels
*/
IPPAPI(IppStatus, ippiBGRToYCbCr422_8u_C3C2R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGRToYCbCr422_8u_AC4C2R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGR555ToYCbCr422_16u8u_C3C2R,( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGR565ToYCbCr422_16u8u_C3C2R,( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiBGRToYCrCb420_8u_C3P3R/ippiBGRToYCrCb420_8u_AC4P3R/ippiBGR555ToYCrCb420_16u8u_C3P3R/ippiBGR565ToYCrCb420_16u8u_C3P3R
//  Purpose:    Converts a RGB24/RGBA/RGB565/RGB565 image to the YV12 image
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsSizeErr         if roiSize.width < 2 or if roiSize.height < 2
//  Arguments:
//    pSrc          Pointer to the source image
//    srcStep       Step through the source image
//    pDst          An array of pointers  to separate destination color planes.
//    dstStep       An array of step in bytes through the destination planes
//    roiSize       region of interest to be processed, in pixels
*/
IPPAPI(IppStatus, ippiBGRToYCrCb420_8u_C3P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGRToYCrCb420_8u_AC4P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGR555ToYCrCb420_16u8u_C3P3R,( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGR565ToYCrCb420_16u8u_C3P3R,( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiRGBToYCrCb420_8u_AC4P3R
//  Purpose:    Converts a RGBA image to the YV12 image
//  Return:
//    ippStsNoErr            Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsDoubleSize    Indicates a warning if roiSize is not a multiple of 2.
//  Arguments:
//    pSrc            Pointer to the source image ROI.
//    srcStep        Distance in bytes between starts of consecutive lines in the source image.
//    pDst            An array of pointers to ROI in the separate planes of the destination image.
//    dstStep        Distance in bytes between starts of consecutive lines in the destination image.
//    roiSize Size of the source and destination ROI in pixels.
*/
IPPAPI(IppStatus, ippiRGBToYCrCb420_8u_AC4P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiYCrCb420ToRGB_8u_P3C4R  and  ippiYCbCr420ToBGR_8u_P3C4R
//  Purpose:    Converts a YV12 image to the a RGBA image and converts a IYUV image to the a BGRA
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsDoubleSize    Indicates a warning if roiSize is not a multiple of 2.
//  Arguments:
//    pSrc           An array of pointers to ROI in separate planes of the source image.
//    srcStep      An array of distances in bytes between starts of consecutive lines in the source image planes.
//    pDst           Pointer to the destination image ROI.
//    dstStep      Distance in bytes between starts of consecutive lines in the destination image.
//   roiSize         Size of the source and destination ROI in pixels.
//    aval           Constant value to create the fourth channel.
*/
IPPAPI(IppStatus, ippiYCbCr420ToBGR_8u_P3C4R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp8u* pDst, int dstStep, IppiSize roiSize,  Ipp8u aval))
IPPAPI(IppStatus, ippiYCrCb420ToRGB_8u_P3C4R,(const Ipp8u* pSrc[3],int srcStep[3],Ipp8u* pDst, int dstStep, IppiSize roiSize,  Ipp8u aval ))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiRGBToYCbCr_8u_AC4P3R and ippiRGBToYCbCr_8u_C3P3R
//  Purpose:    Converts a RGBA and RGB image to the YUV 4:4:4 image
//  Return:
//    ippStsNoErr            Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//  Arguments:
//    pSrc          Pointer to the source image ROI for pixel-order image. An array of pointers to ROI in each separate source color planes for planar images.
//    srcStep      Distance in bytes between starts of consecutive lines in the source image.
//   pDst            Pointer to the destination image ROI. An array of pointers to ROI in the separate destination color planes for planar images.
//   dstStep       Distance in bytes between starts of consecutive lines in the destination image.
//   roiSize        Size of the source and destination ROI in pixels.
*/
IPPAPI(IppStatus, ippiRGBToYCbCr_8u_AC4P3R,( const Ipp8u* pSrc  , int srcStep, Ipp8u* pDst[3], int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYCbCr_8u_C3P3R,( const Ipp8u* pSrc   , int srcStep, Ipp8u* pDst[3], int dstStep, IppiSize roiSize))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiYCbCrToRGB_8u_P3C4R
//  Purpose:    Converts a YUV 4:4:4 image to the a RGBA image
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    Arguments:
//    pSrc             An array of pointers to ROI in each separate source color planes.
//    srcStep         Distance in bytes between starts of consecutive lines in the source image.
//    pDst             Pointer to the destination image ROI.
//    dstStep         Distance in bytes between starts of consecutive lines in the destination image.
//    roiSize          Size of the source and destination ROI in pixels.
//    aval               Constant value to create the fourth channel.
*/
IPPAPI(IppStatus, ippiYCbCrToRGB_8u_P3C4R,( const Ipp8u* pSrc[3], int srcStep, Ipp8u* pDst   , int dstStep, IppiSize roiSize, Ipp8u aval ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr422_8u_P3C2R,     ippiYCbCr422_8u_C2P3R
//
//  Purpose:    Converts 422 planar image to 2-channel pixel-order
//              image and vice versa.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            Width of first plain 422-image less than 2(4)
//                             or height equal zero
//
//  Parameters:
//    pSrc[3]                  Array of pointers to the source image planes
//    srcStep[3]               Array of steps through the source image planes
//    pDst[3]                  Array of pointers to the destination image planes
//    dstStep[3]               Array of steps through the destination image planes
//    pSrc                     Pointer to the source pixel-order image
//    srcStep                  Step through the source pixel-order image
//    pDst                     Pointer to the destination pixel-order image
//    dstStep                  Step through the destination pixel-order image
//    roiSize                  Size of the ROI
*/
IPPAPI (IppStatus, ippiYCbCr422_8u_P3C2R,  ( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst, int dstStep, IppiSize  roiSize))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCrCb420ToBGR_Filter_8u_P3C4R
//  Purpose:    Converts a YV12 image to the RGB32 image
//  Return:
//    ippStsNoErr           Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsSizeErr         if roiSize.width < 2 or if roiSize.height < 2
//    ippStsDoubleSize      sizes of image is not multiples of 2
//  Arguments:
//    pDst          Pointer to the destination image
//    dstStep       Step through the destination image
//    pSrc          An array of pointers  to separate source color planes.
//    srcStep       An array of step in bytes through the source planes
//    roiSize       Region of interest to be processed, in pixels
//    aval          Constant value to create the fourth channel.
*/
IPPAPI(IppStatus, ippiYCrCb420ToBGR_Filter_8u_P3C4R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp8u aval ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiJoin420_8u_P2C2R, ippiJoin420_Filter_8u_P2C2R
//
//  Purpose:    Converts 420 two-plane image to 2-channel pixel-order image,
//              ippiJoin420_Filter additionally performs deinterlace filtering NV12 to YUY2
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field with zero or negative value
//
//  Parameters:
//    pSrcY                    Pointer to the source image Y plane.
//    srcYStep                 Step through the source image Y plane.
//    pSrcCbCr                 Pointer to the source image interleaved chrominance plane.
//    srcCbCrStep              Step through the source image CbCr plane.
//    pDst                     Pointer to the destination image
//    dstStep                  Step through the destination image
//    roiSize                  Size of the ROI, height and width should be multiple of 2.
//    layout                   Slice layout (for deinterlace filter).
//             Possible values:
//                IPP_UPPER  - the first slice.
//                IPP_CENTER - the middle slices.
//                IPP_LOWER  - the last   slice.
//                IPP_LOWER && IPP_UPPER && IPP_CENTER - image is not sliced.
//  Notes:
//    Source 4:2:0 two-plane image format ( NV12 ):
//    all Y (pSrcY) samples are found first in memory as an array of
//    unsigned char with an even number of lines memory alignment),
//    followed immediately by an array(pSrcCbCr) of unsigned char
//    containing interleaved U and V samples (such that if addressed as a little-endian
//    WORD type, U would be in the LSBs and V would be in the MSBs).
//
//    Sequence of samples in the destination 4:2:2 pixel-order two-channel image (YUY2):
//    Y0U0Y1V0,Y2U1Y3V1,...
//
//    The function ippiJoin420_Filter_8u_P2C2R usually operates on the sliced images
//     ( the height of slices should be a multiple of 16).
//
*/

IPPAPI(IppStatus, ippiYCbCr420ToYCbCr422_8u_P2C2R,(const Ipp8u* pSrcY, int srcYStep,const Ipp8u* pSrcCbCr,
                  int srcCbCrStep, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToYCbCr422_Filter_8u_P2C2R,(const Ipp8u* pSrcY, int srcYStep,const Ipp8u* pSrcCbCr,
        int srcCbCrStep, Ipp8u* pDst, int dstStep, IppiSize roiSize,int layout))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiSplit420_8u_P2P3R, ippiSplit420_Filter_8u_P2P3R
//
//  Purpose:    Converts NV12 two-plane image to YV12 three-plane image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field with zero or negative value
//
//  Parameters:
//    pSrcY                    Pointer to the source image Y plane.
//    srcYStepY                Step through the source image Y plane.
//    pSrcCbCr                 Pointer to the source image CbCr plane.
//    srcCbCrStep              Step through the source image CbCr plane.
//    pDst[3]                  Array of pointers to the destination image planes
//    dstStep[3]               Array of steps through the destination image planes
//    roiSize                  Size of the ROI, should be multiple of 2.
//    layout                   Slice layout (for deinterlace filter).
//             Possible values:
//                IPP_UPPER  - the first slice.
//                IPP_CENTER - the middle slices.
//                IPP_LOWER  - the last   slice.
//                IPP_LOWER && IPP_UPPER && IPP_CENTER - image is not sliced.
//  Notes:
//    Source 4:2:0 two-plane image format (NV12):
//    all Y (pSrcY) samples are found first in memory as an array of
//    unsigned char with an even number of lines memory alignment),
//    followed immediately by an array(pSrcCbCr) of unsigned char
//    containing interleaved U and V samples (such that if addressed as a little-endian
//    WORD type, U would be in the LSBs and V would be in the MSBs).
//
//    Destination 4:2:0 three-plane image format(YV12 ):
//    the order of the pointers to destination images - Y V U.
//
//    The function ippiSplit420_Filter_8u_P2P3R usually operates on the sliced images
//    ( the height of slices should be a multiple of 16).
*/


IPPAPI(IppStatus, ippiYCbCr420ToYCrCb420_Filter_8u_P2P3R,(const Ipp8u* pSrcY, int srcYStep,const Ipp8u* pSrcCbCr,
                  int srcCbCrStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize,int layout))
IPPAPI(IppStatus, ippiYCbCr420ToYCrCb420_8u_P2P3R,(const Ipp8u* pSrcY, int srcYStep,const Ipp8u* pSrcCbCr,
                  int srcCbCrStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYUToYU422_8u_C2P2R, ippiUYToYU422_8u_C2P2R
//  Purpose:    Converts 2-channel YUY2,UYVY images to the 2-plane NV12 image
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field value less than 2
//
//  Parameters:
//    pDstY                    Pointer to the destination image Y plane.
//    dstYStep                 The step through the destination image Y plane.
//    pDstCbCr                 Pointer to the destination image CbCr plane.
//    dstCbCrStep              The step through the destination image CbCr plane.
//    pSrc                     Pointer to the source image
//    srcStep                  Step through the source image
//    roiSize                  Size of the ROI, should be multiple of 2.
//  Notes:
//    for ippiYUToYU422_8u_C2P2R sequence of bytes in the source image is( YUY2 ):
//                   Y0U0Y1V0,Y2U1Y3V1,...
//    for ippiUYToYU422_8u_C2P2R sequence of bytes in the destination image is( UYVY ):
//                   U0Y0V0Y1,U1Y2V1Y3,...
//    Sequence of bytes in the destination image is( NV12 ):
//        Y plane    Y0Y1Y2Y3
//       UV plane    U0V0U1V1
*/

IPPAPI(IppStatus, ippiYCbCr422ToYCbCr420_8u_C2P2R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDstY, int dstYStep,Ipp8u* pDstCbCr,int dstCbCrStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiCbYCr422ToYCbCr420_8u_C2P2R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDstY, int dstYStep,Ipp8u* pDstCbCr,int dstCbCrStep, IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr422ToYCrCb420_8u_C2P3R, ippiCbYCr422ToYCrCb420_8u_C2P3R
//  Purpose:    Converts 2-channel YUY2,UYVY images to the 3-plane YV12 image
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field value less than 2
//
//  Parameters:
//    pSrc          Pointer to the source image
//    srcStep       Step through the source image
//    roiSize       Size of the ROI, should be multiple of 2.
//    pDst          An array of pointers  to separate destination color planes.
//    dstStep       An array of step in bytes through the destination planes
//  Notes:
//    for ippiYUToYV422_8u_C2P3R sequence of bytes in the source image is( YUY2 ):
//                   Y0U0Y1V0,Y2U1Y3V1,...
//    for ippiUYToYV422_8u_C2P3R sequence of bytes in the destination image is( UYVY ):
//                   U0Y0V0Y1,U1Y2V1Y3,...
//    Sequence of planes in the destination image is( YV12 ):  Y V U
*/
IPPAPI(IppStatus, ippiYCbCr422ToYCrCb420_8u_C2P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3],int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiCbYCr422ToYCrCb420_8u_C2P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3],int dstStep[3], IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYUToUY422_8u_C2R, ippiUYToYU422_8u_C2R
//  Purpose:    Converts a 2-channel YUY2 image to the UYVY image and vice versa
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field value less than 2
//
//  Parameters:
//    pSrc                     Pointer to the source image
//    srcStep                  Step through the source i mage
//    roiSize                  Size of the ROI, roiSize.width should be multiple of 2
//    pDst                     Pointer to the  destination image
//    dstStep                  Step through the destination image
//  Notes:
//    sequence of bytes in the source image for ippiYUToUY422_8u_C2R and in the destination image for ippiUYToYU422_8u_C2P2R is( YUY2 ):
//                   Y0U0Y1V0,Y2U1Y3V1,...
//    sequence of bytes in the destination image for ippiUYToYU422_8u_C2R and in the source image for ippiUYToYU422_8u_C2P2R is( UYVY ):
//                   U0Y0V0Y1,U1Y2V1Y3,...
*/
IPPAPI(IppStatus, ippiYCbCr422ToCbYCr422_8u_C2R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiCbYCr422ToYCbCr422_8u_C2R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYVToUY420_8u_P3C2R, ippiYVToYU420_8u_P3C2R
//  Purpose:    Converts a 3-plane YV12 image to 2-channel YUY2,UYVY images
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field value less than 2
//
//  Parameters:
//    pSrc                     An array of three pointers to source image planes
//    srcStep                  An array of steps through the source image planes
//    roiSize                  Size of the ROI, should be multiple 2.
//    pDst                     Pointer to the  destination image
//    dstStep                  Step through the destination image
//  Notes:
//    Sequence of planes in the source image is( YV12 ):  Y V U
//    for ippiYVToUY420_8u_P3C2R sequence of bytes in the destination image is( YUY2 ):
//                   Y0U0Y1V0,Y2U1Y3V1,...
//    for ippiUYToYU422_8u_C2P2R sequence of bytes in the destination image is( UYVY ):
//                   U0Y0V0Y1,U1Y2V1Y3,...
*/

IPPAPI(IppStatus, ippiYCrCb420ToCbYCr422_8u_P3C2R,( const Ipp8u* pSrc[3],int srcStep[3], Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCrCb420ToYCbCr422_8u_P3C2R,( const Ipp8u* pSrc[3],int srcStep[3], Ipp8u* pDst, int dstStep, IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiRGBToYCbCr422_8u_C3P3R/ippiYCbCr422ToBGR_8u_P3C3R
//  PVCS ID 9600
//  Purpose:    Converts a RGB24/YUY2 image to the YUY2/RGB24 image
//  Return:
//    ippStsNoErr            Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsDoubleSize    Indicates a warning if roiSize is not a multiple of 2.
//  Arguments:
//   pSrc        Pointer to the source image ROI for pixel-order image. An array of pointers to ROI in each separate source color planes for planar images.
//   srcStep     Distance in bytes between starts of consecutive lines in the source image.
//   pDst        Pointer to the destination image ROI. An array of pointers to ROI in the separate destination color and alpha planes for planar images.
//   dstStep     Distance in bytes between starts of consecutive lines in the destination image.
//   roiSize     Size of the source and destination ROI in pixels.
*/
IPPAPI(IppStatus, ippiRGBToYCbCr422_8u_C3P3R,(const Ipp8u* pSrc   , int srcStep   , Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422ToBGR_8u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst   , int dstStep   , IppiSize roiSize))



/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCrCb420ToYCbCr420_8u_P3P2R
//  Purpose:    Converts a 3-plane YV12 image to the 2-plane NV12 image
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field value less than 2
//
//  Parameters:
//    pSrc                     An array of three pointers to source image planes
//    srcStep                  An array of steps through the source image planes
//    roiSize                  Size of the ROI, should be multiple 2.
//    pDstY                    Pointer to the destination image Y plane.
//    dstYStep                 Step through the destination image Y plane.
//    pDstCbCr                 Pointer to the destination image CbCr plane.
//    dstCbCrStep              Step through the destination image CbCr plane.
//  Notes:
//    Sequence of planes in the source image is( YV12 ):  Y V U
//    Sequence of bytes in the destination image is( NV12 ):
//        Y plane    Y0Y1Y2Y3
//       UV plane    U0V0U1V1
*/
IPPAPI(IppStatus, ippiYCrCb420ToYCbCr420_8u_P3P2R,( const Ipp8u* pSrc[3],int srcStep[3], Ipp8u* pDstY, int dstYStep,Ipp8u* pDstCbCr,int dstCbCrStep, IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYUToUY420_8u_P2C2R
//  Purpose:    Converts a 2-plane NV12 image to the 2-channel UYVY image
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field value less than 2
//
//  Parameters:
//    pSrcY                    Pointer to the source image Y plane.
//    srcYStep                 Step through the source image Y plane.
//    pSrcCbCr                 Pointer to the source image CbCr plane.
//    srcCbCrStep              Step through the source image CbCr plane.
//    pDst                     Pointer to the destination image
//    dstStep                  Step through the destination image
//    roiSize                  Size of the ROI, should be multiple of 2.
//  Notes:
//    Sequence of bytes in the source image is( NV12 ):
//        Y plane    Y0Y1Y2Y3
//       UV plane    U0V0U1V1
//    for ippiUYToYU422_8u_C2P2R sequence of bytes in the destination image is( UYVY ):
//                   U0Y0V0Y1,U1Y2V1Y3,...
*/
IPPAPI(IppStatus, ippiYCbCr420ToCbYCr422_8u_P2C2R,( const Ipp8u* pSrcY, int srcYStep,const Ipp8u* pSrcCbCr, int srcCbCrStep, Ipp8u* pDst, int dstStep, IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr422ToYCbCr422_8u_C2P3R
//  Purpose:    Converts a YUY2 image to the P422 image
//  Name:       ippiYCrCb422ToYCbCr422_8u_C2P3R
//  Purpose:    Converts a YVYU image to the P422 image
//  Name:       ippiCbYCr422ToYCbCr422_8u_C2P3R
//  Purpose:    Converts a UYVY image to the P422 image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 2
//
//  Arguments:
//    pSrc                     pointer to the source image
//    srcStep                  step for the source image
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 2.
*/
IPPAPI(IppStatus, ippiYCbCr422_8u_C2P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCrCb422ToYCbCr422_8u_C2P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiCbYCr422ToYCbCr422_8u_C2P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr422ToYCbCr420_8u_C2P3R
//  Purpose:    Converts a 2-channel YUY2 image to the I420(IYUV) image
//  Name:       ippiCbYCr422ToYCbCr420_8u_C2P3R
//  Purpose:    Converts a 2-channel YUY2 image to the I420(IYUV) image
//  Name:       ippiYCrCb422ToYCbCr420_8u_C2P3R
//  Purpose:    Converts a 2-channel YVYU image to the I420(IYUV) image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 2 || roiSize.height < 2
//
//  Arguments:
//    pSrc                     pointer to the source image
//    srcStep                  step for the source image
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width and roiSize.height  should be multiple 2.
*/
IPPAPI(IppStatus, ippiYCbCr422ToYCbCr420_8u_C2P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiCbYCr422ToYCbCr420_8u_C2P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCrCb422ToYCbCr420_8u_C2P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr422ToYCbCr411_8u_C2P3R
//  Purpose:    Converts a YUY2 image to the P411 image
//  Name:       ippiCbYCr422ToYCbCr411_8u_C2P3R
//  Purpose:    Converts a YUY2 image to the P411 image
//  Name:       ippiYCrCb422ToYCbCr411_8u_C2P3R
//  Purpose:    Converts a YVYU image to the P411 image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4
//
//  Arguments:
//    pSrc                     pointer to the source image
//    srcStep                  step for the source image
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 4.
*/
IPPAPI(IppStatus, ippiYCbCr422ToYCbCr411_8u_C2P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiCbYCr422ToYCbCr411_8u_C2P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCrCb422ToYCbCr411_8u_C2P3R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiYCbCr422ToYCbCr420_8u_P3P2R
//  Purpose:    Converts a P422 image to the NV12 image
//  Name:    ippiYCbCr420_8u_P3P2R
//  Purpose:    Converts a IYUV image to the NV12 image
//  Name:    ippiYCbCr420ToYCbCr411_8u_P3P2R
//  Purpose:    Converts a IYUV image to the NV11 image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr        One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 2 || roiSize.height < 2
//
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pDstY                    pointer to the source Y plane
//    dstYStep                 step  for the destination Y plane
//    pDstCbCr                 pointer to the destination CbCr plane
//    dstCbCrStep              step  for the destination CbCr plane
//    roiSize                  region of interest to be processed, in pixels
//  Notes:
//    roiSize.width and roiSize.height should be multiple 2.
*/
IPPAPI(IppStatus, ippiYCbCr422ToYCbCr420_8u_P3P2R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDstY,
        int dstYStep,Ipp8u* pDstCbCr, int dstCbCrStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCbCr420_8u_P3P2R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDstY,
        int dstYStep,Ipp8u* pDstCbCr, int dstCbCrStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCbCr420ToYCbCr411_8u_P3P2R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDstY,
        int dstYStep,Ipp8u* pDstCbCr, int dstCbCrStep, IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiYCbCr422ToYCbCr411_8u_P3P2R
//  Purpose:    Converts a P422 image to the NV11 image
//  Name:    ippiYCrCb420ToYCbCr411_8u_P3P2R
//  Purpose:    Converts a YV12 image to the NV11 image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr        One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4 || roiSize.height < 2
//
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pDstY                    pointer to the source Y plane
//    dstYStep                 step  for the destination Y plane
//    pDstCbCr                 pointer to the destination CbCr plane
//    dstCbCrStep              step  for the destination CbCr plane
//    roiSize                  region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 4.
//    roiSize.height should be multiple 2.
*/
IPPAPI(IppStatus, ippiYCbCr422ToYCbCr411_8u_P3P2R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDstY,
        int dstYStep,Ipp8u* pDstCbCr, int dstCbCrStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCrCb420ToYCbCr411_8u_P3P2R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDstY,
        int dstYStep,Ipp8u* pDstCbCr, int dstCbCrStep, IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr422ToYCbCr420_8u_P3R
//  Purpose:    Converts a P422 image to the I420(IYUV)image
//  Name:       ippiYCbCr420ToYCbCr422_8u_P3R
//  Purpose:    Converts a IYUV image to the P422 image
//  Name:       ippiYCrCb420ToYCbCr422_8u_P3R
//  Purpose:    Converts a YV12 image to the P422 image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr        One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 2 || roiSize.height < 2
//
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width and roiSize.height  should be multiple 2.
*/
IPPAPI(IppStatus, ippiYCbCr422ToYCbCr420_8u_P3R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCbCr420ToYCbCr422_8u_P3R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCrCb420ToYCbCr422_8u_P3R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))
/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr420ToYCbCr422_Filter_8u_P3R
//  Purpose:    Converts a IYUV image to the P422 image.
//  Name:       ippiYCrCb420ToYCbCr422_Filter_8u_P3R
//  Purpose:    Converts a YV12 image to the P422 image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 2 || roiSize.height < 8
//
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 2.
//    roiSize.height should be multiple 8.
//    We use here Catmull-Rom interpolation.
*/
IPPAPI(IppStatus, ippiYCbCr420ToYCbCr422_Filter_8u_P3R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCrCb420ToYCbCr422_Filter_8u_P3R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))
/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr420ToYCbCr422_Filter_8u_P2P3R
//  Purpose:    Converts a NV12 image to the P422 image.
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 2 || roiSize.height < 8
//
//  Arguments:
//    pSrcY                    pointer to the source Y plane
//    srcYStep                 step  for the source Y plane
//    pSrcCbCr                 pointer to the source CbCr plane
//    srcCbCrStep              step  for the source CbCr plane
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 2.
//    roiSize.height should be multiple 8.
//    We use here Catmull-Rom interpolation.
*/
IPPAPI(IppStatus, ippiYCbCr420ToYCbCr422_Filter_8u_P2P3R,(const Ipp8u* pSrcY,int srcYStep,const Ipp8u* pSrcCbCr, int srcCbCrStep,
       Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr420ToYCbCr422_8u_P2P3R
//  Purpose:    Converts a NV12 image to the P422 image.
//  Name:       ippiYCbCr420_8u_P2P3R
//  Purpose:    Converts a NV12 image to the I420(IYUV) image.
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 2 || roiSize.height < 2
//
//  Arguments:
//    pSrcY                    pointer to the source Y plane
//    srcYStep                 step  for the source Y plane
//    pSrcCbCr                 pointer to the source CbCr plane
//    srcCbCrStep              step  for the source CbCr plane
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 2.
//    roiSize.height should be multiple 2.
*/
IPPAPI(IppStatus, ippiYCbCr420ToYCbCr422_8u_P2P3R,(const Ipp8u* pSrcY,int srcYStep,const Ipp8u* pSrcCbCr, int srcCbCrStep,
Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))

IPPAPI(IppStatus, ippiYCbCr420_8u_P2P3R,(const Ipp8u* pSrcY,int srcYStep,const Ipp8u* pSrcCbCr, int srcCbCrStep,
Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr420ToYCbCr411_8u_P2P3R
//  Purpose:    Converts a NV12 image to the P411 image.
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4 || roiSize.height < 2
//
//  Arguments:
//    pSrcY                    pointer to the source Y plane
//    srcYStep                 step  for the source Y plane
//    pSrcCbCr                 pointer to the source CbCr plane
//    srcCbCrStep              step  for the source CbCr plane
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 4.
//    roiSize.height should be multiple 2.
*/
IPPAPI(IppStatus, ippiYCbCr420ToYCbCr411_8u_P2P3R,(const Ipp8u* pSrcY,int srcYStep,const Ipp8u* pSrcCbCr, int srcCbCrStep,
Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr411ToYCbCr422_8u_P3C2R
//  Purpose:    Converts a P411 image to the YUY2 image.
//  Name:       ippiYCbCr411ToYCrCb422_8u_P3C2R
//  Purpose:    Converts a P411 image to the YVYU image.
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4
//
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pSrc                     pointer to the destination image
//    srcStep                  step for the destination image
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 4.
*/
IPPAPI(IppStatus, ippiYCbCr411ToYCbCr422_8u_P3C2R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst,
        int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCbCr411ToYCrCb422_8u_P3C2R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst,
        int dstStep, IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr411ToYCbCr422_8u_P3R
//  Purpose:    Converts a P411 image to the P422 image
//  Name:       ippiYCbCr411ToYCrCb422_8u_P3R
//  Purpose:    Converts a P411 image to the YVYU image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr        One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4
//
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width should be multiple 4.
*/
IPPAPI(IppStatus, ippiYCbCr411ToYCbCr422_8u_P3R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCbCr411ToYCrCb422_8u_P3R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr411ToYCbCr420_8u_P3R
//  Purpose:    Converts a P411 image to the I420(IYUV) image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr        One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4 || roiSize.height < 2
//
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 4.
//    roiSize.height should be multiple 2.
*/
IPPAPI(IppStatus, ippiYCbCr411ToYCbCr420_8u_P3R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiYCbCr411ToYCbCr420_8u_P3P2R
//  Purpose:    Converts a P411 image to the NV12 image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4 || roiSize.height < 2
//
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pDstY                    pointer to the source Y plane
//    dstYStep                 step  for the destination Y plane
//    pDstCbCr                 pointer to the destination CbCr plane
//    dstCbCrStep              step  for the destination CbCr plane
//    roiSize                  region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 4.
//    roiSize.height should be multiple 2.
*/
IPPAPI(IppStatus, ippiYCbCr411ToYCbCr420_8u_P3P2R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDstY,
        int dstYStep,Ipp8u* pDstCbCr, int dstCbCrStep, IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiYCbCr411ToYCbCr411_8u_P3P2R
//  Purpose:    Converts a P411 image to the NV11 image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4
//
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pDstY                    pointer to the source Y plane
//    dstYStep                 step  for the destination Y plane
//    pDstCbCr                 pointer to the destination CbCr plane
//    dstCbCrStep              step  for the destination CbCr plane
//    roiSize                  region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 4.
*/
IPPAPI(IppStatus, ippiYCbCr411_8u_P3P2R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDstY,
        int dstYStep,Ipp8u* pDstCbCr, int dstCbCrStep, IppiSize roiSize ))

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr411ToYCbCr422_8u_P2C2R
//  Purpose:    Converts a NV11 image to the YUY2 image.
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4
//
//  Arguments:
//    pSrcY                    pointer to the source Y plane
//    srcYStep                 step  for the source Y plane
//    pSrcCbCr                 pointer to the source CbCr plane
//    srcCbCrStep              step  for the source CbCr plane
//    pSrc                     pointer to the destination image
//    srcStep                  step for the destination image
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 4.
*/
IPPAPI(IppStatus, ippiYCbCr411ToYCbCr422_8u_P2C2R,(const Ipp8u* pSrcY,int srcYStep,const Ipp8u* pSrcCbCr, int srcCbCrStep,
Ipp8u* pDst, int dstStep, IppiSize roiSize ))

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr411ToYCbCr422_8u_P2P3R
//  Purpose:    Converts a NV11 image to the P422 image.
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4
//
//  Arguments:
//    pSrcY                    pointer to the source Y plane
//    srcYStep                 step  for the source Y plane
//    pSrcCbCr                 pointer to the source CbCr plane
//    srcCbCrStep              step  for the source CbCr plane
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 4.
*/
IPPAPI(IppStatus, ippiYCbCr411ToYCbCr422_8u_P2P3R,(const Ipp8u* pSrcY,int srcYStep,const Ipp8u* pSrcCbCr, int srcCbCrStep,
Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr411ToYCrCb420_8u_P2P3R
//  Purpose:    Converts a NV11 image to the YV12 image.
//  Name:       ippiYCbCr411ToYCbCr420_8u_P2P3R
//  Purpose:    Converts a NV11 image to the I420(IYUV) image.
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4 || roiSize.height < 2
//
//  Arguments:
//    pSrcY                    pointer to the source Y plane
//    srcYStep                 step  for the source Y plane
//    pSrcCbCr                 pointer to the source CbCr plane
//    srcCbCrStep              step  for the source CbCr plane
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 4.
//    roiSize.height should be multiple 2.
*/
IPPAPI(IppStatus, ippiYCbCr411ToYCrCb420_8u_P2P3R,(const Ipp8u* pSrcY,int srcYStep,const Ipp8u* pSrcCbCr, int srcCbCrStep,
Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCbCr411ToYCbCr420_8u_P2P3R,(const Ipp8u* pSrcY,int srcYStep,const Ipp8u* pSrcCbCr, int srcCbCrStep,
Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr411_8u_P2P3R
//  Purpose:    Converts a NV11 image to the P411 image.
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4
//
//  Arguments:
//    pSrcY                    pointer to the source Y plane
//    srcYStep                 step  for the source Y plane
//    pSrcCbCr                 pointer to the source CbCr plane
//    srcCbCrStep              step  for the source CbCr plane
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 4.
*/
IPPAPI(IppStatus, ippiYCbCr411_8u_P2P3R,(const Ipp8u* pSrcY,int srcYStep,const Ipp8u* pSrcCbCr, int srcCbCrStep,
Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr422ToYCbCr411_8u_C2P2R
//  Purpose:    Converts a YUY2 image to the NV11 image YUY2ToNV11
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 4
//
//  Arguments:
//    pSrc                     pointer to the source image
//    srcStep                  step for the source image
//    pDstY                    pointer to the source Y plane
//    dstYStep                 step  for the destination Y plane
//    pDstCbCr                 pointer to the destination CbCr plane
//    dstCbCrStep              step  for the destination CbCr plane
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width  should be multiple 4.
*/
IPPAPI(IppStatus, ippiYCbCr422ToYCbCr411_8u_C2P2R,( const Ipp8u* pSrc, int srcStep,  Ipp8u* pDstY,
        int dstYStep,Ipp8u* pDstCbCr, int dstCbCrStep, IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr422ToYCbCr411_8u_P3R
//  Purpose:    Converts a P422 image to the P411 image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr        One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 2 || roiSize.height < 2
//
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pDst                     array of pointers to the components of the destination image
//    dstStep                  array of steps values for every component
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width and roiSize.height  should be multiple 2.
*/
IPPAPI(IppStatus, ippiYCbCr422ToYCbCr411_8u_P3R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst[3],
        int dstStep[3], IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr422ToYCrCb422_8u_P3C2R
//  Purpose:    Converts a P422 image to the YVYU image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr        One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 2
//
//  Arguments:
//    pSrc                     array of pointers to the components of the source image
//    srcStep                  array of step values for every component
//    pSrc                     pointer to the destination image
//    srcStep                  step for the destination image
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width and should be multiple 2.
*/
IPPAPI(IppStatus, ippiYCbCr422ToYCrCb422_8u_P3C2R,( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst,
        int dstStep, IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCbCr422ToYCrCb422_8u_C2R
//  Purpose:    Converts a YUY2 image to the YVYU image
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr        One or more pointers are NULL
//    ippStsSizeErr            if roiSize.width < 2
//
//  Arguments:
//    pSrc                     pointer to the source image
//    srcStep                  step for the source image
//    pSrc                     pointer to the destination image
//    srcStep                  step for the destination image
//     roiSize                 region of interest to be processed, in pixels
//  Notes:
//    roiSize.width should be multiple 2.
*/
IPPAPI(IppStatus, ippiYCbCr422ToYCrCb422_8u_C2R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst,
        int dstStep, IppiSize roiSize ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name: ippiRGBToYCbCr_8u_C3R,       ippiYCbCrToRGB_8u_C3R.
//        ippiRGBToYCbCr_8u_AC4R,      ippiYCbCrToRGB_8u_AC4R.
//        ippiRGBToYCbCr_8u_P3R,       ippiYCbCrToRGB_8u_P3R.
//        ippiYCbCrToRGB_8u_P3C3R
//        ippiYCbCrToBGR444_8u16u_C3R, ippiYCbCrToRGB444_8u16u_C3R,
//        ippiYCbCrToBGR555_8u16u_C3R, ippiYCbCrToRGB555_8u16u_C3R,
//        ippiYCbCrToBGR565_8u16u_C3R, ippiYCbCrToRGB565_8u16u_C3R,

//  Purpose:    Convert an RGB(BGR) image to and from YCbCr color model
//  Parameters:
//     pSrc   Pointer to the source image (for pixel-order data).An array of pointers
//            to separate source color planes (in case of plane-order data)
//     pDst   Pointer to the resultant image (for pixel-order data).An array of pointers
//            to separate source color planes (in case of plane-order data)
//     roiSize Size of the ROI in pixels.
//     srcStep Step in bytes through the source image to jump on the next line
//     dstStep Step in bytes through the destination image to jump on the next line
//  Returns:
//           ippStsNullPtrErr  src == NULL or dst == NULL
//           ippStsStepErr,    srcStep or dstStep is less than or equal to zero
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  Reference:
//      Jack Keith
//      Video Demystified: A Handbook for the Digital Engineer, 2nd ed.
//      1996.pp.(42-43)
//
//  The YCbCr color space was developed as part of Recommendation ITU-R BT.601
//  (formerly CCI 601). Y is defined to have a nominal range of 16 to 235;
//  Cb and Cr are defined to have a range of 16 to 240, with 128 equal to zero.
//  If the gamma-corrected RGB(R'G'B') image has a range 0 .. 255, as is commonly
//  found in computer system (and in our library), the following equations may be
//  used:
//
//       Y  =  0.257*R' + 0.504*G' + 0.098*B' + 16
//       Cb = -0.148*R' - 0.291*G' + 0.439*B' + 128
//       Cr =  0.439*R' - 0.368*G' - 0.071*B' + 128
//
//       R' = 1.164*(Y - 16) + 1.596*(Cr - 128 )
//       G' = 1.164*(Y - 16) - 0.813*(Cr - 128 )- 0.392*( Cb - 128 )
//       B' = 1.164*(Y - 16) + 2.017*(Cb - 128 )
//
//   Note that for the YCbCr-to-RGB equations, the RGB values must be saturated
//   at the 0 and 255 levels due to occasional excursions outside the nominal
//   YCbCr ranges.
//
*/
IPPAPI(IppStatus, ippiRGBToYCbCr_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYCbCr_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYCbCr_8u_P3R,(const Ipp8u*  pSrc[3], int srcStep,
       Ipp8u* pDst[3], int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB_8u_P3C3R,(const Ipp8u*  pSrc[3],int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB_8u_P3R,(const Ipp8u*  pSrc[3],int srcStep,
       Ipp8u* pDst[3], int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToBGR444_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB444_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToBGR555_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB555_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToBGR565_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCrToRGB565_8u16u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiRGBToYUV_8u_C3R,  ippiYUVToRGB_8u_C3R.
//              ippiRGBToYUV_8u_AC4R, ippiYUVToRGB_8u_AC4R.
//              ippiRGBToYUV_8u_P3R,  ippiYUVToRGB_8u_P3R.
//              ippiRGBToYUV_8u_C3P3R,ippiYUVToRGB_8u_P3C3R.
//  Purpose:    Converts an RGB image to the YUV color model and vice versa.
//  Parameters:
//     pSrc   Pointer to the source image (for pixel-order data).An array of pointers
//            to separate source color planes (for plane-order data)
//     pDst   Pointer to the destination image (for pixel-order data).An array of
//            pointers to separate destination color planes (for of plane-order data)
//     roiSize   Size of ROI in pixels.
//     srcStep   Step in bytes through the source image to jump on the next line
//     dstStep   Step in bytes through the destination image to jump on the next line
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsStepErr,    srcStep or dstStep is less than or equal to zero
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  Reference:
//      Jack Keith
//      Video Demystified: A Handbook for the Digital Engineer, 2nd ed.
//      1996.pp.(40-41)
//
//     The YUV color space is the basic color space used by the PAL, NTSC, and
//  SECAM composite color video standards.
//
//  These functions operate with gamma-corrected images.
//  The basic equations for conversion between gamma-corrected RGB(R'G'B')and YUV are:
//
//       Y' =  0.299*R' + 0.587*G' + 0.114*B'
//       U  = -0.147*R' - 0.289*G' + 0.436*B' = 0.492*(B' - Y' )
//       V  =  0.615*R' - 0.515*G' - 0.100*B' = 0.877*(R' - Y' )
//
//       R' = Y' + 1.140 * V
//       G' = Y' - 0.394 * U - 0.581 * V
//       B' = Y' + 2.032 * U
//
//     For digital RGB values in the range [0 .. 255], Y has a range [0..255],
//   U a range [-112 .. +112], and V a range [-157..+157].
//
//   These equations are usually scaled to simplify the implementation in an actual
//   NTSC or PAL digital encoder or decoder.
//
*/
/* Pixel to Pixel */
IPPAPI(IppStatus, ippiRGBToYUV_8u_C3R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUVToRGB_8u_C3R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
/* Pixel to Pixel */
IPPAPI(IppStatus, ippiRGBToYUV_8u_AC4R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUVToRGB_8u_AC4R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
/* Plane to Plane */
IPPAPI(IppStatus, ippiRGBToYUV_8u_P3R,(const Ipp8u*  pSrc[3], int srcStep ,
       Ipp8u* pDst[3], int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUVToRGB_8u_P3R,(const Ipp8u*  pSrc[3], int srcStep,
       Ipp8u* pDst[3], int dstStep, IppiSize roiSize))
/* Pixel to Plane */
IPPAPI(IppStatus, ippiRGBToYUV_8u_C3P3R,( const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst[3], int dstStep, IppiSize roiSize))
/* Plane to Pixel */
IPPAPI(IppStatus, ippiYUVToRGB_8u_P3C3R,(const Ipp8u*  pSrc[3], int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))


/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiBGRToYUV420_8u_AC4P3R
//  PVCS ID 7635
//  Return:
//    ippStsNoErr         Ok
//    ippStsNullPtrErr    one or more pointers are NULL
//    ippStsDoubleSize    Indicates a warning if roiSize is not a multiple of 2.
//  Arguments:
//    pSrc          Pointer to the source image ROI.
//    srcStep       Distance in bytes between starts of consecutive lines in the source image.
//    pDst          An array of pointers to ROI in the separate planes of the destination image.
//    dstStep       Distance in bytes between starts of consecutive lines in the destination image.
//    roiSize       Size of the source and destination ROI in pixels.
*/
IPPAPI(IppStatus, ippiBGRToYUV420_8u_AC4P3R,  (const Ipp8u*  pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiRGBToYUV_8u_AC4P4R
//  PVCS ID 8868
//  Return:
//    ippStsNoErr            Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//  Arguments:
//   pSrc         Pointer to the source image ROI for pixel-order image.
//   srcStep      Distance in bytes between starts of consecutive lines in the source image.
//   pDst         Pointer to the destination image ROI. An array of pointers to ROI in the separate destination color and alpha planes for planar images.
//   dstStep      Distance in bytes between starts of consecutive lines in the destination image.
//   roiSize      Size of the source and destination ROI in pixels.
*/
IPPAPI(IppStatus, ippiRGBToYUV_8u_AC4P4R,( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[4], int dstStep, IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiRGB565ToYUV420_16u8u_C3P3R/ippiRGB565ToYUV422_16u8u_C3P3R
//  PVCS ID 8910
//  Return:
//    ippStsNoErr            Ok
//    ippStsNullPtrErr      one or more pointers are NULL
//    ippStsDoubleSize    Indicates a warning if roiSize is not a multiple of 2.
//  Arguments:
//   pSrc        Pointer to the source image ROI for pixel-order image.
//   srcStep     Distance in bytes between starts of consecutive lines in the source image.
//   pDst        An array of pointers to ROI in the separate destination color and alpha planes for planar images.
//   dstStep     Distance in bytes between starts of consecutive lines in the destination image.
//   roiSize     Size of the source and destination ROI in pixels.
*/
IPPAPI(IppStatus, ippiRGB565ToYUV420_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))
IPPAPI(IppStatus, ippiRGB565ToYUV422_16u8u_C3P3R,(const Ipp16u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiRGBToYUV422_8u_C3P3R,  ippiYUV422ToRGB_8u_P3C3R.
//              ippiRGBToYUV422_8u_P3R,    ippiYUV422ToRGB_8u_P3R.
//              ippiRGBToYUV420_8u_C3P3R,  ippiYUV420ToRGB_8u_P3C3R.
//              ippiRGBToYUV422_8u_C3C2R,  ippiYUV422ToRGB_8u_C2C3R.
//         ippiYUV420ToBGR565_8u16u_P3C3R,
//         ippiYUV420ToBGR555_8u16u_P3C3R,
//         ippiYUV420ToBGR444_8u16u_P3C3R,
//         ippiYUV420ToRGB565_8u16u_P3C3R,
//         ippiYUV420ToRGB555_8u16u_P3C3R,
//         ippiYUV420ToRGB444_8u16u_P3C3R.

//  Purpose:    Converts an RGB (BGR) image to the YUV color model with 4:2:2 or
//              4:2:0 sampling and vice versa.
//  Parameters:
//     pSrc  Pointer to the source image (for pixel-order data).An array of pointers
//           to separate source color planes (for plane-order data)
//     pDst  Pointer to the destination image (for pixel-order data).An array of pointers
//           to separate destination color planes (for plane-order data)
//     roiSize   Size of the ROI in pixels.
//     srcStep   Step in bytes through the source image to jump on the next line(for pixel-order data).
//               An array of step values for the separate source color planes (for plane-order data).
//     dstStep   Step in bytes through destination image to jump on the next line(for pixel-order data).
//               An array of step values for the separate destination color planes (for plane-order data).
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsStepErr     srcStep or dstStep is less than or equal to zero
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  Reference:
//      Jack Keith
//      Video Demystified: A Handbook for the Digital Engineer, 2nd ed.
//      1996.pp.(40-41)
//
//     The YUV color space is the basic color space used by the PAL , NTSC , and
//  SECAM composite color video standards.
//
//  The functions operate with 4:2:2 and 4:2:0 sampling formats.
//    4:2:2 uses the horizontal-only 2:1 reduction of U and V,
//    4:2:0 uses 2:1 reduction of U and V in both the vertical and
//    horizontal directions.
//
//  These functions operate with gamma-corrected images.
//  The basic equations for conversion between gamma-corrected RGB(R'G'B')and YUV are:
//
//       Y' =  0.299*R' + 0.587*G' + 0.114*B'
//       U  = -0.147*R' - 0.289*G' + 0.436*B' = 0.492*(B' - Y' )
//       V  =  0.615*R' - 0.515*G' - 0.100*B' = 0.877*(R' - Y' )
//
//       R' = Y' + 1.140 * V
//       G' = Y' - 0.394 * U - 0.581 * V
//       B' = Y' + 2.032 * U
//
//     For digital RGB values with the range [0 .. 255], Y has a range [0..255],
//   U a range [-112 .. +112], and V a range [-157..+157].
//

//   These equations are usually scaled to simplify the implementation in an actual
//   NTSC or PAL digital encoder or decoder.
//
*/
IPPAPI(IppStatus, ippiYUV420ToRGB_8u_P3AC4R,(const Ipp8u*  pSrc[3],int srcStep[3],Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV422ToRGB_8u_P3AC4R,(const Ipp8u*  pSrc[3],int srcStep[3],Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYUV422_8u_C3P3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst[3],int dstStep[3], IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV422ToRGB_8u_P3C3R,(const Ipp8u* pSrc[3],int srcStep[3],
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYUV422_8u_P3R,(const Ipp8u*  pSrc[3], int srcStep ,
       Ipp8u* pDst[3], int dstStep[3],IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV422ToRGB_8u_P3R,(const Ipp8u* pSrc[3],
       int srcStep[3],Ipp8u* pDst[3], int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYUV420_8u_C3P3R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToRGB_8u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToBGR_8u_P3C3R,(const Ipp8u*       pSrc[3], int srcStep[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYUV420_8u_P3R,(const Ipp8u* pSrc[3], int srcStep ,
       Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToRGB_8u_P3R,(const Ipp8u* pSrc[3],int srcStep[3],
       Ipp8u* pDst[3], int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYUV422_8u_C3C2R,(const Ipp8u* pSrc, int srcStep ,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV422ToRGB_8u_C2C3R,(const Ipp8u* pSrc,int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToBGR565_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToBGR555_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToBGR444_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToRGB565_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToRGB555_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYUV420ToRGB444_8u16u_P3C3R,(const Ipp8u* pSrc[3], int srcStep[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiRGBToYUV422_8u_P3,   ippiYUV422ToRGB_8u_P3.
//              ippiRGBToYUV422_8u_C3P3, ippiYUV422ToRGB_8u_P3C3.
//              ippiRGBToYUV420_8u_C3P3, ippiYUV420ToRGB_8u_P3C3.
//              ippiRGBToYUV420_8u_P3,   ippiYUV420ToRGB_8u_P3.
//  Purpose:    Converts an RGB image to YUV color model with 4:2:2 and
//              4:2:0 sampling and vice versa.
//  Parameters:
//     pSrc   Pointer to the source image (for pixel-order data).An array of pointers
//            to the separate source color planes (for plane-order data)
//     pDst   Pointer to the destination image (for pixel-order data).An array of pointers
//            to the separate destination color planes (for plane-order data)
//     imgSize   Size of the source and destination images in pixels
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsStepErr,    srcStep or dstStep is less than or equal to zero
//           ippStsSizeErr     imgSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  Reference:
//      Jack Keith
//      Video Demystified: A Handbook for the Digital Engineer, 2nd ed.
//      1996.pp.(40-41)
//
//     The YUV color space is the basic color space used by the PAL , NTSC , and
//  SECAM composite color video standards.
//
//  The functions operate with 4:2:2 and 4:2:0 sampling formats.
//    4:2:2 uses the horizontal-only 2:1 reduction of U and V,
//    4:2:0 uses 2:1 reduction of U and V in both the vertical and
//    horizontal directions.
//
//  These functions operate with gamma-corrected images.
//  The basic equations to convert between gamma-corrected RGB(R'G'B')and YUV are:
//
//       Y' =  0.299*R' + 0.587*G' + 0.114*B'
//       U  = -0.147*R' - 0.289*G' + 0.436*B' = 0.492*(B' - Y' )
//       V  =  0.615*R' - 0.515*G' - 0.100*B' = 0.877*(R' - Y' )
//
//       R' = Y' + 1.140 * V
//       G' = Y' - 0.394 * U - 0.581 * V
//       B' = Y' + 2.032 * U
//
//   For digital RGB values with the range [0 .. 255], Y has the range [0..255],
//   U the range [-112 .. +112],V the range [-157..+157].
//
//   These equations are usually scaled to simplify the implementation in an actual
//   NTSC or PAL digital encoder or decoder.
//
*/
/* Plane to Plane */
IPPAPI(IppStatus, ippiRGBToYUV422_8u_P3,(const Ipp8u* pSrc[3], Ipp8u* pDst[3], IppiSize imgSize))
IPPAPI(IppStatus, ippiYUV422ToRGB_8u_P3,(const Ipp8u* pSrc[3], Ipp8u* pDst[3], IppiSize imgSize))
/* Pixel to Plane */
IPPAPI(IppStatus, ippiRGBToYUV422_8u_C3P3,(const Ipp8u* pSrc, Ipp8u* pDst[3], IppiSize imgSize))
/* Plane to Pixel */
IPPAPI(IppStatus, ippiYUV422ToRGB_8u_P3C3,(const Ipp8u* pSrc[3],Ipp8u* pDst, IppiSize imgSize ))
/* Pixel to Plane */
IPPAPI(IppStatus, ippiRGBToYUV420_8u_C3P3,(const Ipp8u* pSrc, Ipp8u* pDst[3], IppiSize imgSize))
/* Plane to Pixel */
IPPAPI(IppStatus, ippiYUV420ToRGB_8u_P3C3,(const Ipp8u* pSrc[3], Ipp8u* pDst, IppiSize imgSize))
/* Plane to Plane */
IPPAPI(IppStatus, ippiRGBToYUV420_8u_P3,(const Ipp8u* pSrc[3], Ipp8u* pDst[3], IppiSize imgSize))
IPPAPI(IppStatus, ippiYUV420ToRGB_8u_P3,(const Ipp8u* pSrc[3], Ipp8u* pDst[3], IppiSize imgSize))


/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCoCgToBGR_16s8u_P3C3R  ippiYCoCgToBGR_16s8u_P3C4R
//  Purpose:    Converts a YCoCg image to the RGB24/RGB32 image.
//  Name:       ippiYCoCgToBGR_16s8u_P3C3R  ippiBGRToYCoCg_8u16s_C4P3R
//  Purpose:    Converts a RGB24/RGB32 image to the YCoCg image.
//  Name:       ippiYCoCgToSBGR_16s_P3C3R  ippiSBGRToYCoCg_16s_C3P3R
//  Purpose:    Converts a YCoCg image to the scRGB48/scRGB64 image.
//  Name:       ippi_SC_BGRToYCoCg_16s_C3P3R  ippi_SC_BGRToYCoCg_16s_C4P3R
//  Purpose:    Converts a scRGB48/scRGB64 image to the YCoCg image.
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//
//  Arguments:
//    pYCC                     array of pointers to the components of the YCoCg image
//    yccStep                  step for every Y,Co,Cg component
//    pBGR                     Pointer to the BGR image (for pixel-order data).
//    bgrStep                  step  for the BGR image.
//    roiSize                  region of interest to be processed, in pixels
//  Notes:
//    Y  = (( R + G*2 + B) + 2 ) / 4
//   Co  = (( R -       B) + 1 ) / 2
//   Cg  = ((-R + G*2 - B) + 2 ) / 4
//
//    R  =  Y + Co - Cg
//    G  =  Y +      Cg
//    B  =  Y - Co - Cg
//    scRGB allows negative values and values above 1.0
*/
IPPAPI(IppStatus,ippiYCoCgToBGR_16s8u_P3C3R,(const Ipp16s* pYCC[3],int yccStep,Ipp8u* pBGR,int bgrStep,IppiSize roiSize))
IPPAPI(IppStatus,ippiBGRToYCoCg_8u16s_C3P3R,(const Ipp8u*  pBGR, int bgrStep, Ipp16s* pYCC [3], int yccStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiYCoCgToBGR_16s8u_P3C4R,(const Ipp16s* pYCC[3],int yccStep,Ipp8u* pBGR,int bgrStep,IppiSize roiSize, Ipp8u aval))
IPPAPI(IppStatus,ippiBGRToYCoCg_8u16s_C4P3R,(const Ipp8u*  pBGR, int bgrStep, Ipp16s* pYCC [3], int yccStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiYCoCgToSBGR_16s_P3C3R ,(const Ipp16s* pYCC[3], int yccStep, Ipp16s*  pBGR, int bgrStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiSBGRToYCoCg_16s_C3P3R ,(const Ipp16s* pBGR, int bgrStep, Ipp16s*  pYCC[3], int yccStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiYCoCgToSBGR_16s_P3C4R ,(const Ipp16s* pYCC[3], int yccStep, Ipp16s*  pBGR   , int bgrStep, IppiSize roiSize, Ipp16s aval ))
IPPAPI(IppStatus,ippiSBGRToYCoCg_16s_C4P3R ,(const Ipp16s* pBGR   , int bgrStep, Ipp16s*  pYCC[3], int yccStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiYCoCgToSBGR_32s16s_P3C3R,(const Ipp32s* pYCC[3], int yccStep, Ipp16s*  pBGR   , int bgrStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiSBGRToYCoCg_16s32s_C3P3R,(const Ipp16s* pBGR   , int bgrStep, Ipp32s*  pYCC[3], int yccStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiYCoCgToSBGR_32s16s_P3C4R,(const Ipp32s* pYCC[3], int yccStep, Ipp16s*  pBGR   , int bgrStep, IppiSize roiSize, Ipp16s aval ))
IPPAPI(IppStatus,ippiSBGRToYCoCg_16s32s_C4P3R,(const Ipp16s* pBGR   , int bgrStep, Ipp32s*  pYCC[3], int yccStep, IppiSize roiSize ))

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiYCoCgToBGR_Rev_16s8u_P3C3R  ippiYCoCgToBGR_Rev_16s8u_P3C4R
//  Purpose:    Converts a YCoCg-R image to the scRGB48/scRGB64 image.
//  Name:       ippiBGRToYCoCg_Rev_8u16s_C3P3R  ippiBGRToYCoCg_Rev_8u16s_C4P3R
//  Purpose:    Converts a scRGB48/scRGB64 image to the YCoCg-R image.
//  Name:       ippiYCoCgToSBGR_Rev_16s_P3C3R  ippiYCoCgToSBGR_Rev_16s_P3C4R
//  Purpose:    Converts a YCoCg-R image to the scRGB48/scRGB64 image.
//  Name:       ippiSBGRToYCoCg_Rev_16s_C3P3R  ippiSBGRToYCoCg_Rev_16s_C4P3R
//  Purpose:    Converts a scRGB48/scRGB64 image to the YCoCg-R image.
//              Where: YCoCg-R  - Reversible transform.
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One or more pointers are NULL
//
//  Arguments:
//    pYCC                     array of pointers to the components of the YCoCg image
//    yccStep                  step for every Y,Co,Cg component
//    pBGR                     Pointer to the BGR image (for pixel-order data).
//    bgrStep                  step  for the BGR image.
//    roiSize                  region of interest to be processed, in pixels
//  Notes:
//    Co =  R  -  B
//    t  =  B  + (Co >> 1)
//    Cg =  G  -  t
//    Y  =  t  + (Cg >> 1)
//
//    t  =  Y  - (Cg >> 1)
//    G  =  Cg + t
//    B  =  t  - (Co >> 1)
//    R  =  B  +  Co
//    If each of the RGB channels are integer values with
//    an N-bit range, then the luma channel Y requires N bits,
//    and the chroma channels require N+1 bits.
*/
IPPAPI(IppStatus, ippiYCoCgToBGR_Rev_16s8u_P3C3R,(const Ipp16s* pYCC[3], int yccStep, Ipp8u*   pBGR, int bgrStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGRToYCoCg_Rev_8u16s_C3P3R,(const Ipp8u*  pBGR, int bgrStep, Ipp16s*  pYCC[3], int yccStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCoCgToBGR_Rev_16s8u_P3C4R,(const Ipp16s* pYCC[3], int yccStep, Ipp8u*   pBGR, int bgrStep, IppiSize roiSize,Ipp8u aval ))
IPPAPI(IppStatus, ippiBGRToYCoCg_Rev_8u16s_C4P3R,(const Ipp8u*  pBGR, int bgrStep, Ipp16s*  pYCC[3], int yccStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCoCgToSBGR_Rev_16s_P3C3R ,(const Ipp16s* pYCC[3], int yccStep, Ipp16s*  pBGR, int bgrStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiSBGRToYCoCg_Rev_16s_C3P3R ,(const Ipp16s* pBGR, int bgrStep, Ipp16s*  pYCC[3], int yccStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCoCgToSBGR_Rev_16s_P3C4R ,(const Ipp16s* pYCC[3], int yccStep, Ipp16s*  pBGR, int bgrStep, IppiSize roiSize,Ipp16s aval ))
IPPAPI(IppStatus, ippiSBGRToYCoCg_Rev_16s_C4P3R ,(const Ipp16s* pBGR, int bgrStep, Ipp16s*  pYCC[3], int yccStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiSBGRToYCoCg_Rev_16s32s_C3P3R,(const Ipp16s* pBGR, int bgrStep, Ipp32s*  pYCC[3], int yccStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiYCoCgToSBGR_Rev_32s16s_P3C4R,(const Ipp32s* pYCC[3], int yccStep, Ipp16s*  pBGR, int bgrStep, IppiSize roiSize, Ipp16s aval ))
IPPAPI(IppStatus, ippiYCoCgToSBGR_Rev_32s16s_P3C3R,(const Ipp32s* pYCC[3], int yccStep, Ipp16s*  pBGR, int bgrStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiSBGRToYCoCg_Rev_16s32s_C4P3R,(const Ipp16s* pBGR, int bgrStep, Ipp32s*  pYCC[3], int yccStep, IppiSize roiSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiRGBToGray
//  Purpose:    Converts an RGB image to gray scale (fixed coefficients)
//  Parameters:
//     pSrc     Pointer to the source image , points to point(0,0)
//     pDst     Pointer to the destination image , points to point(0,0)
//     roiSize  Size of the ROI in pixels. Since the function performs point
//          operations (without a border), the ROI may be the whole image.
//     srcStep  Step in bytes through the source image to jump on the next line
//     dstStep  Step in bytes through the destination image to jump on the next line
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  Reference:
//      Jack Keith
//      Video Demystified: A Handbook for the Digital Engineer, 2nd ed.
//      1996.p.(82)
//
//  The transform coefficients of equation below correspond to the standard for
//  NTSC red, green and blue CRT phosphors (1953) that are standardized in the
//  ITU-R Recommendation BT. 601-2 (formerly CCIR Rec. 601-2).
//
//  The basic equation to compute non-linear video luma (monochrome) from non-linear
//  (gamma-corrected) RGB(R'G'B') is:
//
//  Y' = 0.299 * R' + 0.587 * G' + 0.114 * B';
//
//
*/
IPPAPI(IppStatus,ippiRGBToGray_8u_C3C1R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize))
IPPAPI(IppStatus,ippiRGBToGray_16u_C3C1R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep,IppiSize roiSize))
IPPAPI(IppStatus,ippiRGBToGray_16s_C3C1R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep,IppiSize roiSize))
IPPAPI(IppStatus,ippiRGBToGray_32f_C3C1R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep,IppiSize roiSize))
IPPAPI(IppStatus,ippiRGBToGray_8u_AC4C1R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize))
IPPAPI(IppStatus,ippiRGBToGray_16u_AC4C1R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep,IppiSize roiSize))
IPPAPI(IppStatus,ippiRGBToGray_16s_AC4C1R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep,IppiSize roiSize))
IPPAPI(IppStatus,ippiRGBToGray_32f_AC4C1R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep,IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiColorToGray
//  Purpose:    Converts an RGB image to gray scale (custom coefficients)
//  Parameters:
//     pSrc      Pointer to the source image , points to point(0,0)
//     pDst      Pointer to the destination image , points to point(0,0)
//     roiSize   Size of the ROI in pixels. Since the function performs point
//               operations (without a border), the ROI may be the whole image.
//     srcStep   Step in bytes through the source image to jump on the next line
//     dstStep   Step in bytes through the destination image to jump on the next line
//     coeffs[3] User-defined vector of coefficients.
//                 The sum of the coefficients should be less than or equal to 1
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//
//  The following equation is used to convert an RGB image to gray scale:
//
//   Y = coeffs[0] * R + coeffs[1] * G + coeffs[2] * B;
//
//
*/
IPPAPI(IppStatus,ippiColorToGray_8u_C3C1R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,const Ipp32f coeffs[3]))
IPPAPI(IppStatus,ippiColorToGray_16u_C3C1R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep,IppiSize roiSize,const Ipp32f coeffs[3]))
IPPAPI(IppStatus,ippiColorToGray_16s_C3C1R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep,IppiSize roiSize,const Ipp32f coeffs[3]))
IPPAPI(IppStatus,ippiColorToGray_32f_C3C1R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep,IppiSize roiSize,const Ipp32f coeffs[3]))

IPPAPI(IppStatus,ippiColorToGray_8u_AC4C1R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,const Ipp32f coeffs[3]))
IPPAPI(IppStatus,ippiColorToGray_16u_AC4C1R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep,IppiSize roiSize,const Ipp32f coeffs[3]))
IPPAPI(IppStatus,ippiColorToGray_16s_AC4C1R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep,IppiSize roiSize,const Ipp32f coeffs[3]))
IPPAPI(IppStatus,ippiColorToGray_32f_AC4C1R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep,IppiSize roiSize,const Ipp32f coeffs[3]))


/* ////////////////////////////////////////////////////////////////////////////
//  Name:     ippiRGBToHLS,     ippiHLSToRGB
//            ippiBGRToHLS,     ippiHLSToBGR
//  Purpose:    Converts an RGB(BGR) image to the HLS color model and vice versa
//  Parameters:
//     pSrc      Pointer to the source image , points to point(0,0)
//     pDst      Pointer to the destination image , points to point(0,0)
//     roiSize   Size of the ROI in pixels. Since the function performs point
//               operations (without a border), the ROI may be the whole image.
//     srcStep   Step in bytes through the source image to jump on the next line
//     dstStep   Step in bytes through the destination image to jump on the next line
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsStepErr     srcStep or dstStep is less than or equal to zero
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  RGB and HLS values for the 32f data type should be in the range [0..1]
//  Reference:
//      David F.Rogers
//      Procedural Elements for Computer Graphics
//      1985.pp.(403-406)
//
//       H is the hue red at 0 degrees, which has range [0 .. 360 degrees],
//       L is the lightness,
//       S is the saturation,
//
//       The RGB to HLS conversion algorithm in pseudo code:
//   Lightness:
//      M1 = max(R,G,B); M2 = max(R,G,B); L = (M1+M2)/2
//   Saturation:
//      if M1 = M2 then // achromatic case
//          S = 0
//          H = 0
//      else // chromatics case
//          if L <= 0.5 then
//               S = (M1-M2) / (M1+M2)
//          else
//               S = (M1-M2) / (2-M1-M2)
//   Hue:
//      Cr = (M1-R) / (M1-M2)
//      Cg = (M1-G) / (M1-M2)
//      Cb = (M1-B) / (M1-M2)
//      if R = M2 then H = Cb - Cg
//      if G = M2 then H = 2 + Cr - Cb
//      if B = M2 then H = 4 + Cg - Cr
//      H = 60*H
//      if H < 0 then H = H + 360
//
//      The HSL to RGB conversion algorithm in pseudo code:
//      if L <= 0.5 then
//           M2 = L *(1 + S)
//      else
//           M2 = L + S - L * S
//      M1 = 2 * L - M2
//      if S = 0 then
//         R = G = B = L
//      else
//          h = H + 120
//          if h > 360 then
//              h = h - 360

//          if h  <  60 then
//              R = ( M1 + ( M2 - M1 ) * h / 60)
//          else if h < 180 then
//              R = M2
//          else if h < 240 then
//              R = M1 + ( M2 - M1 ) * ( 240 - h ) / 60
//          else
//              R = M1
//          h = H
//          if h  <  60 then
//              G = ( M1 + ( M2 - M1 ) * h / 60
//          else if h < 180 then
//              G = M2
//          else if h < 240 then
//              G = M1 + ( M2 - M1 ) * ( 240 - h ) / 60
//          else
//              G  = M1
//          h = H - 120
//          if h < 0 then
//              h += 360
//          if h  <  60 then
//              B = ( M1 + ( M2 - M1 ) * h / 60
//          else if h < 180 then
//              B = M2
//          else if h < 240 then
//              B = M1 + ( M2 - M1 ) * ( 240 - h ) / 60
//          else
//              B = M1
//
//    H,L,S,R,G,B are scaled to the range:
//             [0..1]                      for the 32f depth,
//             [0..IPP_MAX_8u]             for the 8u depth,
//             [0..IPP_MAX_16u]            for the 16u depth,
//             [IPP_MIN_16S..IPP_MAX_16s]  for the 16s depth.
//
*/
IPPAPI(IppStatus, ippiBGRToHLS_8u_AC4R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToHLS_8u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToRGB_8u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRGBToHLS_8u_AC4R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToRGB_8u_AC4R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRGBToHLS_16s_C3R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToRGB_16s_C3R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToHLS_16s_AC4R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToRGB_16s_AC4R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRGBToHLS_16u_C3R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToRGB_16u_C3R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToHLS_16u_AC4R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToRGB_16u_AC4R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRGBToHLS_32f_C3R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToRGB_32f_C3R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToHLS_32f_AC4R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToRGB_32f_AC4R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiBGRToHLS_8u_AP4R, (const Ipp8u*  pSrc[4], int srcStep, Ipp8u* pDst[4], int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiBGRToHLS_8u_AP4C4R, (const Ipp8u*  pSrc[4], int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiBGRToHLS_8u_AC4P4R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[4], int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiBGRToHLS_8u_P3R, (const Ipp8u*  pSrc[3], int srcStep, Ipp8u* pDst[3], int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiBGRToHLS_8u_P3C3R, (const Ipp8u*  pSrc[3], int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiBGRToHLS_8u_C3P3R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToBGR_8u_AP4R, (const Ipp8u*  pSrc[4], int srcStep, Ipp8u* pDst[4], int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToBGR_8u_AP4C4R, (const Ipp8u*  pSrc[4], int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToBGR_8u_AC4P4R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[4], int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToBGR_8u_P3R, (const Ipp8u*  pSrc[3], int srcStep, Ipp8u* pDst[3], int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToBGR_8u_P3C3R, (const Ipp8u*  pSrc[3], int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHLSToBGR_8u_C3P3R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst[3], int dstStep, IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:        ippiRGBToHSV,   ippiHSVToRGB
//  Purpose:    Converts an RGB image to the HSV color model and vice versa
//  Parameters:
//     pSrc      Pointer to the source image , points to point(0,0)
//     pDst      Pointer to the destination image , points to point(0,0)
//     roiSize   Size of the ROI in pixels.
//     srcStep   Step in bytes through the source image to jump on the next line
//     dstStep   Step in bytes through the destination image to jump on the next line
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsStepErr     srcStep or dstStep is less than or equal to zero
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  Reference:
//      David F.Rogers
//      Procedural Elements for Computer Graphics
//      1985.pp.(401-403)
//
//       H is the hue red at 0 degrees, which has range [0 .. 360 degrees],
//       S is the saturation,
//       V is the value
//       The RGB to HSV conversion algorithm in pseudo code:
//   Value:
//      V = max(R,G,B);
//   Saturation:
//      temp = min(R,G,B);
//      if V = 0 then // achromatic case
//          S = 0
//          H = 0
//      else // chromatics case
//          S = (V - temp)/V
//   Hue:
//      Cr = (V - R) / (V - temp)
//      Cg = (V - G) / (V - temp)
//      Cb = (V - B) / (V - temp)
//      if R = V then H = Cb - Cg
//      if G = V then H = 2 + Cr - Cb
//      if B = V then H = 4 + Cg - Cr
//      H = 60*H
//      if H < 0 then H = H + 360
//
//      The HSV to RGB conversion algorithm in pseudo code:
//      if S = 0 then
//         R = G = B = V
//      else
//          if H = 360 then
//              H = 0
//          else
//              H = H/60
//           I = floor(H)
//           F = H - I;
//           M = V * ( 1 - S);
//           N = V * ( 1 - S * F);
//           K = V * ( 1 - S * (1 - F));
//           if(I == 0)then{ R = V;G = K;B = M;}
//           if(I == 1)then{ R = N;G = V;B = M;}
//           if(I == 2)then{ R = M;G = V;B = K;}
//           if(I == 3)then{ R = M;G = N;B = V;}
//           if(I == 4)then{ R = K;G = M;B = V;}
//           if(I == 5)then{ R = V;G = M;B = N;}
//
//           in the range [0..IPP_MAX_8u ] for the 8u depth,
//           in the range [0..IPP_MAX_16u] for the 16u depth,
//
*/
IPPAPI(IppStatus, ippiRGBToHSV_8u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHSVToRGB_8u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToHSV_8u_AC4R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHSVToRGB_8u_AC4R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRGBToHSV_16u_C3R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHSVToRGB_16u_C3R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToHSV_16u_AC4R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiHSVToRGB_16u_AC4R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:     ippiRGBToYCC,   ippiYCCToRGB
//  Purpose:    Converts an RGB image to the YCC color model and vice versa.
//  Parameters:
//    pSrc          Pointer to the source image ROI
//    srcStep       Step through the source image (bytes)
//    pDst          Pointer to the destination image ROI
//    dstStep       Step through the destination image (bytes)
//    dstRoiSize    size of the ROI
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsStepErr     srcStep or dstStep is less than or equal to zero
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  Reference:
//      Jack Keith
//      Video Demystified: a Handbook for the Digital Engineer, 2nd ed.
//      1996.pp.(46-47)
//
//  The basic equations to convert gamma-corrected R'G'B' image to YCC are:
//
//   RGB data is transformed into PhotoYCC data:
//    Y  =  0.299*R' + 0.587*G' + 0.114*B'
//    C1 = -0.299*R' - 0.587*G' + 0.886*B' = B'- Y
//    C2 =  0.701*R' - 0.587*G' - 0.114*B' = R'- Y
//   Y,C1,C2 are quantized and limited to the range [0..1]
//    Y  = 1. / 1.402 * Y
//    C1 = 111.4 / 255. * C1 + 156. / 255.
//    C2 = 135.64 /255. * C2 + 137. / 255.
//
//  Conversion of PhotoYCC data to RGB data for CRT computer display:
//
//   normal luminance and chrominance data are recovered
//    Y  = 1.3584 * Y
//    C1 = 2.2179 * (C1 - 156./255.)
//    C2 = 1.8215 * (C2 - 137./255.)
//   PhotoYCC data is transformed into RGB data
//    R' = L + C2
//    G' = L - 0.194*C1 - 0.509*C2
//    B' = L + C1
//    Where:  Y -  luminance; and C1, C2  - two chrominance values.
//
//  Equations are given above in assumption that the Y, C1, C2, R, G, and B
//   values are in the range [0..1].
//   Y, C1, C2, R, G, B - are scaled to the range:
//             [0..1]                      for the 32f depth,
//             [0..IPP_MAX_8u]             for the 8u depth,
//             [0..IPP_MAX_16u]            for the 16u depth,
//             [IPP_MIN_16s..IPP_MAX_16s]  for the 16s depth.
*/

IPPAPI(IppStatus, ippiRGBToYCC_8u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCCToRGB_8u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYCC_8u_AC4R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCCToRGB_8u_AC4R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRGBToYCC_16u_C3R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCCToRGB_16u_C3R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYCC_16u_AC4R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCCToRGB_16u_AC4R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRGBToYCC_16s_C3R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCCToRGB_16s_C3R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYCC_16s_AC4R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCCToRGB_16s_AC4R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRGBToYCC_32f_C3R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCCToRGB_32f_C3R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToYCC_32f_AC4R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCCToRGB_32f_AC4R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiRGBToXYZ,     ippiXYZToRGB
//  Purpose:    Converts an RGB image to the XYZ color model and vice versa.
//  Parameters:
//    pSrc          Pointer to the source image ROI
//    srcStep       Step through the source image (bytes)
//    pDst          Pointer to the destination image ROI
//    dstStep       Step through the destination image (bytes)
//    dstRoiSize    size of the ROI
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsStepErr     srcStep or dstStep is less than or equal to zero
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  Reference:
//      David F. Rogers
//      Procedural Elements for Computer Graphics.
//      1985.
//
//  The basic equations to convert between Rec. 709 RGB (with its D65 white point) and CIE XYZ are:
//
//       X =  0.412453 * R + 0.35758 * G + 0.180423 * B
//       Y =  0.212671 * R + 0.71516 * G + 0.072169 * B
//       Z =  0.019334 * R + 0.119193* G + 0.950227 * B
//
//       R = 3.240479 * X - 1.53715  * Y  - 0.498535 * Z
//       G =-0.969256 * X + 1.875991 * Y  + 0.041556 * Z
//       B = 0.055648 * X - 0.204043 * Y  + 1.057311 * Z
//  Equations are given above in assumption that the X,Y,Z,R,G, and B
//   values are in the range [0..1].
//   Y, C1, C2, R, G, B - are scaled to the range:
//           [0..1]                      for the 32f depth,
//           [0..IPP_MAX_8u]             for the 8u depth,
//           [0..IPP_MAX_16u]            for the 16u depth,
//           [IPP_MIN_16s..IPP_MAX_16s]  for the 16s depth.
//
*/
IPPAPI(IppStatus, ippiRGBToXYZ_8u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXYZToRGB_8u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToXYZ_8u_AC4R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXYZToRGB_8u_AC4R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToXYZ_16u_C3R,(const Ipp16u* pSrc, int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXYZToRGB_16u_C3R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToXYZ_16u_AC4R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXYZToRGB_16u_AC4R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToXYZ_16s_C3R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiXYZToRGB_16s_C3R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToXYZ_16s_AC4R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXYZToRGB_16s_AC4R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToXYZ_32f_C3R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXYZToRGB_32f_C3R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToXYZ_32f_AC4R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXYZToRGB_32f_AC4R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiRGBToLUV,     ippiLUVToRGB
//  Purpose:    Converts an RGB image to the LUV color model and vice versa.
//  Parameters:
//    pSrc          Pointer to the source image ROI
//    srcStep       Step through the source image (bytes)
//    pDst          Pointer to the destination image ROI
//    dstStep       Step through the destination image (bytes)
//    dstRoiSize    size of the ROI
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsStepErr     srcStep or dstStep is less than or equal to zero
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoErr       No errors
//  Reference:
//     Computer Graphics: Principles and Practices. James D. Foley... [et al]. 2nd edition.
//     Addison-Wesley, 1990.p.(584)
//
//    At first an RGB image is converted to the XYZ format image (see the functions
//    ippiRGBToXYZ), then to the CIELUV with the white point D65 and CIE chromaticity
//    coordinates of white point (xn,yn) = (0.312713, 0.329016), and Yn = 1.0 - the luminance of white point.
//
//       L = 116. * (Y/Yn)**1/3. - 16.
//       U = 13. * L * ( u - un )
//       V = 13. * L * ( v - vn )
//      These are quantized and limited to the 8-bit range of 0 to 255.
//       L =   L * 255. / 100.
//       U = ( U + 134. ) * 255. / 354.
//       V = ( V + 140. ) * 255. / 256.
//       where:
//       u' = 4.*X / (X + 15.*Y + 3.*Z)
//       v' = 9.*Y / (X + 15.*Y + 3.*Z)
//       un = 4.*xn / ( -2.*xn + 12.*yn + 3. )
//       vn = 9.*yn / ( -2.*xn + 12.*yn + 3. ).
//       xn, yn is the CIE chromaticity coordinates of white point.
//       Yn = 255. is the luminance of white point.
//
//       The L component values are in the range [0..100], the U component values are
//       in the range [-134..220], and the V component values are in the range [-140..122].
//
//      The CIELUV to RGB conversion is performed as following. At first
//      a LUV image is converted to the XYZ image
//       L  =   L * 100./ 255.
//       U  = ( U * 354./ 255.) - 134.
//       V  = ( V * 256./ 255.) - 140.
//       u = U / ( 13.* L ) + un
//       v = V / ( 13.* L ) + vn
//       Y = (( L + 16. ) / 116. )**3.
//       Y *= Yn
//       X =  -9.* Y * u / (( u - 4.)* v - u * v )
//       Z = ( 9.*Y - 15*v*Y - v*X ) / 3. * v
//       where:
//       un = 4.*xn / ( -2.*xn + 12.*yn + 3. )
//       vn = 9.*yn / ( -2.*xn + 12.*yn + 3. ).
//       xn, yn is the CIE chromaticity coordinates of white point.
//       Yn = 255. is the luminance of white point.
//
//     Then the XYZ image is converted to the RGB image (see the functions
//     ippiXYZToRGB).
//
*/

IPPAPI(IppStatus, ippiRGBToLUV_8u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLUVToRGB_8u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToLUV_8u_AC4R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLUVToRGB_8u_AC4R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToLUV_16u_C3R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLUVToRGB_16u_C3R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToLUV_16u_AC4R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLUVToRGB_16u_AC4R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToLUV_16s_C3R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLUVToRGB_16s_C3R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToLUV_16s_AC4R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLUVToRGB_16s_AC4R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToLUV_32f_C3R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLUVToRGB_32f_C3R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRGBToLUV_32f_AC4R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLUVToRGB_32f_AC4R,(const Ipp32f* pSrc,int srcStep,Ipp32f* pDst, int dstStep, IppiSize roiSize))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiBGRToLab_8u_C3R    and ippiLabToBGR_8u_C3R
//              ippiBGRToLab_8u16u_C3R and ippiLabToBGR_16u8u_C3R
//  Purpose:    Converts an RGB image to CIE Lab color model and vice-versa
//  Parameters:
//    pSrc          Pointer to the source image ROI
//    srcStep       Step through the source  image (bytes)
//    pDst          Pointer to the destination image ROI
//    dstStep       Step through the destination image (bytes)
//    roiSize       Size of the ROI
//  Returns:
//     ippStsNullPtrErr  if src == NULL or dst == NULL
//     ippStsSizeErr     if imgSize.width <= 0 || imgSize.height <= 0
//     ippStsNoErr       otherwise
//  Reference:
//     Computer graphics: principles and practices. James D. Foley... [et al.]. 2nd ed.
//     Addison-Wesley, c1990.p.(584)
//
//    At first an RGB image is converted to the XYZ color model (see the function
//    ippRGBToXYZ_8u_C3R), then to the CIELab with the white point D65 and CIE chromaticity
//    coordinates of white point (xn,yn) = (0.312713, 0.329016)
//    L = 116. *((Y/Yn)^(1/3)) - 16    for Y/Yn >  0.008856
//    L = 903.3*(Y/Yn)                 for Y/Yn <= 0.008856
//    a = 500. * (f(X/Xn) - f(Y/Yn))
//    b = 200. * (f(Y/Yn) - f(Z/Zn))
//    where f(t)=t^(1/3)               for t >  0.008856
//    f(t)=7.787*t+16/116              for t <= 0.008856
//    These values are quantized and scaled to the 8-bit range of 0 to 255 for ippiBGRToLab_8u_C3R.
//    L =   L * 255. / 100.
//    a = (a + 128.)
//    b = (a + 128.)
//    and they are quantized and scaled to the 16-bit range of 0 to 65535 for ippiBGRToLab_8u16u_C3R
//    L =  L * 65535. / 100.
//    a = (a + 128.)* 255
//    b = (a + 128.)* 255
//    where:
//      normalizing multipliers
//    Yn = 1.0      * 255
//    Xn = 0.950455 * 255
//    Zn = 1.088753 * 255
//
//    L component values are in the range [0..100], a and b component values are
//    in the range [-128..127].
//
//    The CIELab to RGB conversion is performed as follows. At first
//    a Lab image is converted to the XYZ image
//      for ippiLabToBGR_8u_C3R
//    L =  L * 100./ 255.
//    a = (a - 128.)
//    b = (a - 128.)
//      or for ippiLabToBGR_16u8u_C3R
//    L =  L * 100./ 65535.
//    a = (a / 255 - 128.)
//    b = (b / 255 - 128.)
//    X = Xn * ( P + a / 500 )^3
//    Y = Yn * P^3
//    Z = Zn * ( P - b / 200 )^3
//    where P = (L + 16) / 116
//    Then the XYZ image is converted to the RGB color model (see the function
//     ippXYZToRGB_8u_C3R).
//
*/

IPPAPI(IppStatus, ippiBGRToLab_8u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst,int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLabToBGR_8u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst,int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiBGRToLab_8u16u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp16u* pDst,int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLabToBGR_16u8u_C3R,(const Ipp16u* pSrc,int srcStep,Ipp8u* pDst,int dstStep, IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippReduceBits
//  Purpose:    Reduces the bit resolution of an image.
//  Parameters:
//     pSrc      Pointer to the source image .
//     pDst      Pointer to the destination image .
//     roiSize   Size of ROI in pixels.
//     srcStep   Step in bytes through the source image to jump on the next line
//     dstStep   Step in bytes through the destination image to jump on the next line
//     noise     The number specifying the amount of noise added (as a percentage of a range [0..100])
//     levels    The number of output levels for halftoning (dithering)[2.. MAX_LEVELS],
//               where  MAX_LEVELS is  0x01 << depth, and depth is depth of the destination image
//     dtype     The type of dithering to be used. The following types are supported:
//        ippDitherNone     no dithering is done
//        ippDitherStucki   Stucki's dithering algorithm
//        ippDitherFS       Floid-Steinberg's dithering algorithm
//        ippDitherJJN      Jarvice-Judice-Ninke's dithering algorithm
//        ippDitherBayer    Bayer's dithering algorithm
//      RGB  values for the 32f data type should be in the range [0..1]
//  Returns:
//           ippStsNullPtrErr  pSrc == NULL, or pDst == NULL
//           ippStsStepErr     srcStep or dstStep is less than or equal to zero
//           ippStsSizeErr     roiSize has a field with zero or negative value
//           ippStsNoiseValErr noise has illegal value
//           ippStsDitherLevelsErr  levels value is out of admissible range
//           ippStsNoErr       No errors
*/

IPPAPI(IppStatus, ippiReduceBits_8u_C1R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_8u_C3R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_8u_AC4R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_8u_C4R,(const Ipp8u* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))

IPPAPI(IppStatus, ippiReduceBits_16u_C1R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_16u_C3R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_16u_AC4R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_16u_C4R,(const Ipp16u* pSrc,int srcStep,Ipp16u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))

IPPAPI(IppStatus, ippiReduceBits_16u8u_C1R,(const Ipp16u* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_16u8u_C3R,(const Ipp16u* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_16u8u_AC4R,(const Ipp16u* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_16u8u_C4R,(const Ipp16u* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))

IPPAPI(IppStatus, ippiReduceBits_16s_C1R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_16s_C3R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_16s_AC4R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_16s_C4R,(const Ipp16s* pSrc,int srcStep,Ipp16s* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))

IPPAPI(IppStatus, ippiReduceBits_16s8u_C1R,(const Ipp16s* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_16s8u_C3R,(const Ipp16s* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_16s8u_AC4R,(const Ipp16s* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_16s8u_C4R,(const Ipp16s* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))

IPPAPI(IppStatus, ippiReduceBits_32f8u_C1R,(const Ipp32f* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_32f8u_C3R,(const Ipp32f* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_32f8u_AC4R,(const Ipp32f* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_32f8u_C4R,(const Ipp32f* pSrc,int srcStep,Ipp8u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))

IPPAPI(IppStatus, ippiReduceBits_32f16u_C1R,(const Ipp32f* pSrc,int srcStep,Ipp16u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_32f16u_C3R,(const Ipp32f* pSrc,int srcStep,Ipp16u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_32f16u_AC4R,(const Ipp32f* pSrc,int srcStep,Ipp16u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_32f16u_C4R,(const Ipp32f* pSrc,int srcStep,Ipp16u* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))

IPPAPI(IppStatus, ippiReduceBits_32f16s_C1R,(const Ipp32f* pSrc,int srcStep,Ipp16s* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_32f16s_C3R,(const Ipp32f* pSrc,int srcStep,Ipp16s* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_32f16s_AC4R,(const Ipp32f* pSrc,int srcStep,Ipp16s* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))
IPPAPI(IppStatus, ippiReduceBits_32f16s_C4R,(const Ipp32f* pSrc,int srcStep,Ipp16s* pDst, int dstStep,IppiSize roiSize,
       int noise, IppiDitherType dtype, int levels))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiColorTwist
//
//  Purpose:    Applies a color-twist matrix to an image.
//              |R|   | t11 t12 t13 t14 |   |r|
//              |G| = | t21 t22 t23 t24 | * |g|
//              |B|   | t31 t32 t33 t34 |   |b|
//
//               R = t11*r + t12*g + t13*b + t14
//               G = t21*r + t22*g + t23*b + t24
//               B = t31*r + t32*g + t33*b + t34
//
//  Returns:
//    ippStsNullPtrErr      One of the pointers is NULL
//    ippStsSizeErr         roiSize has a field with zero or negative value
//    ippStsStepErr         One of the step values is zero or negative
//    ippStsNoErr           OK
//
//  Parameters:
//    pSrc            Pointer to the source image
//    srcStep         Step through the source image
//    pDst            Pointer to the  destination image
//    dstStep         Step through the destination image
//    pSrcDst         Pointer to the source/destination image (in-place flavors)
//    srcDstStep      Step through the source/destination image (in-place flavors)
//    roiSize         Size of the ROI
//    twist           An array of color-twist matrix elements
*/
IPPAPI ( IppStatus, ippiColorTwist32f_8s_C3R, ( const Ipp8s* pSrc, int srcStep, Ipp8s* pDst, int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_8s_C3IR, ( Ipp8s* pSrcDst, int srcDstStep, IppiSize roiSize,
                    const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_8s_AC4R, ( const Ipp8s* pSrc, int srcStep, Ipp8s* pDst, int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_8s_AC4IR, ( Ipp8s* pSrcDst, int srcDstStep, IppiSize roiSize,
                    const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_8s_P3R, ( const Ipp8s* pSrc[3], int srcStep,
                    Ipp8s* pDst[3], int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_8s_IP3R, ( Ipp8s* pSrcDst[3], int srcDstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_8u_C3R, ( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_8u_C3IR, ( Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize,
                    const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_8u_AC4R, ( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_8u_AC4IR, ( Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize,
                    const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_8u_P3R, ( const Ipp8u* pSrc[3], int srcStep,
                    Ipp8u* pDst[3], int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_8u_IP3R, ( Ipp8u* pSrcDst[3], int srcDstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_16u_C3R, ( const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_16u_C3IR, ( Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize,
                    const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_16u_AC4R, ( const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_16u_AC4IR, ( Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize,
                    const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_16u_P3R, ( const Ipp16u* pSrc[3], int srcStep,
                    Ipp16u* pDst[3], int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_16u_IP3R, ( Ipp16u* pSrcDst[3], int srcDstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_16s_C3R, ( const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_16s_C3IR, ( Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize,
                    const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_16s_AC4R, ( const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_16s_AC4IR, ( Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize,
                    const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_16s_P3R, ( const Ipp16s* pSrc[3], int srcStep,
                    Ipp16s* pDst[3], int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist32f_16s_IP3R, ( Ipp16s* pSrcDst[3], int srcDstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist_32f_C3R, ( const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist_32f_C3IR, ( Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize,
                    const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist_32f_AC4R, ( const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist_32f_AC4IR, ( Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize,
                    const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist_32f_P3R, ( const Ipp32f* pSrc[3], int srcStep,
                    Ipp32f* pDst[3], int dstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))
IPPAPI ( IppStatus, ippiColorTwist_32f_IP3R, ( Ipp32f* pSrcDst[3], int srcDstStep,
                    IppiSize roiSize, const Ipp32f twist[3][4] ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiColorTwist_32f_C4R
//
//  Purpose:    Applies a color-twist matrix to an image.
//              |R|   | t11 t12 t13 t14 |   |r|
//              |G| = | t21 t22 t23 t24 | * |g|
//              |B|   | t31 t32 t33 t34 |   |b|
//              |W|   | t41 t42 t43 t44 |   |w|
//               R = t11*r + t12*g + t13*b + t14*w
//               G = t21*r + t22*g + t23*b + t24*w
//               B = t31*r + t32*g + t33*b + t34*w
//               W = t41*r + t42*g + t43*b + t44*w
//
//  Returns:
//    ippStsNullPtrErr      One of the pointers is NULL
//    ippStsSizeErr         roiSize has a field with zero or negative value
//    ippStsStepErr         One of the step values is zero or negative
//    ippStsNoErr           OK
//
//  Parameters:
//    pSrc                  Pointer  to the source image
//    srcStep               Step through the source image
//    pDst                  Pointer to the  destination image
//    dstStep               Step through the destination image
//    roiSize               Size of the ROI
//    twist                 An array of color-twist matrix elements
*/

IPPAPI ( IppStatus, ippiColorTwist_32f_C4R,( const Ipp32f* pSrc, int srcStep,Ipp32f* pDst, int dstStep,
           IppiSize roiSize, const Ipp32f twist[4][4]))


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiGammaFwd,      ippiGammaInv
//
//  Purpose:  Performs gamma-correction of an RGB image (ippiGammaFwd);
//            converts a gamma-corrected RGB image back to the original (ippiGammaInv).
//                  1). Gamma-correction:
//                      for R,G,B < 0.018
//                          R' = 4.5 * R
//                          G' = 4.5 * G
//                          B' = 4.5 * B
//                      for R,G,B >= 0.018
//                          R' = 1.099 * (R**0.45) - 0.099
//                          G' = 1.099 * (G**0.45) - 0.099
//                          B' = 1.099 * (B**0.45) - 0.099
//
//                  2). Conversion to the original:
//                      for R',G',B' < 0.0812
//                          R = R' / 4.5
//                          G = G' / 4.5
//                          B = B' / 4.5
//                      for R',G',B' >= 0.0812
//                          R = (( R' + 0.099 ) / 1.099 )** 1 / 0.45
//                          G = (( G' + 0.099 ) / 1.099 )** 1 / 0.45
//                          B = (( B' + 0.099 ) / 1.099 )** 1 / 0.45
//
//                  Note: example for range[0,1].
//
//  Returns:
//    ippStsNullPtrErr      One of the pointers is NULL
//    ippStsSizeErr         roiSize has a field with zero or negative value
//    ippStsStepErr         One of the step values is less than or equal to zero
//    ippStsGammaRangeErr   vMax - vMin <= 0 (for 32f)
//    ippStsNoErr           OK
//
//  Parameters:
//    pSrc                  Pointer  to the source image (pixel-order data). An array
//                          of pointers to separate source color planes (planar data)
//    srcStep               Step through the source image
//    pDst                  Pointer to the  destination image (pixel-order data). An array
//                          of pointers to separate destination color planes (planar data)
//    dstStep               Step through the destination image
//    pSrcDst               Pointer to the source/destination image (in-place flavors)
//    srcDstStep            Step through the source/destination image (in-place flavors)
//    roiSize               Size of the ROI
//    vMin, vMax            Minimum and maximum values of the input 32f data.
*/
IPPAPI(IppStatus,ippiGammaFwd_8u_C3R, ( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaFwd_8u_C3IR, ( Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaInv_8u_C3R, ( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaInv_8u_C3IR, ( Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaFwd_8u_AC4R, ( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaFwd_8u_AC4IR, ( Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaInv_8u_AC4R, ( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaInv_8u_AC4IR, ( Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaFwd_8u_P3R, ( const Ipp8u* pSrc[3], int srcStep, Ipp8u* pDst[3], int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaFwd_8u_IP3R, ( Ipp8u* pSrcDst[3], int srcDstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaInv_8u_P3R, ( const Ipp8u* pSrc[3], int srcStep, Ipp8u* pDst[3], int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaInv_8u_IP3R, ( Ipp8u* pSrcDst[3], int srcDstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaFwd_16u_C3R, ( const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaFwd_16u_C3IR, ( Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaInv_16u_C3R, ( const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaInv_16u_C3IR, ( Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaFwd_16u_AC4R, ( const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaFwd_16u_AC4IR, ( Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaInv_16u_AC4R, ( const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaInv_16u_AC4IR, ( Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaFwd_16u_P3R, ( const Ipp16u* pSrc[3], int srcStep, Ipp16u* pDst[3], int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaFwd_16u_IP3R, ( Ipp16u* pSrcDst[3], int srcDstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaInv_16u_P3R, ( const Ipp16u* pSrc[3], int srcStep, Ipp16u* pDst[3], int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaInv_16u_IP3R, ( Ipp16u* pSrcDst[3], int srcDstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiGammaFwd_32f_C3R, ( const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f vMin, Ipp32f vMax  ))
IPPAPI(IppStatus,ippiGammaFwd_32f_C3IR, ( Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, Ipp32f vMin, Ipp32f vMax ))
IPPAPI(IppStatus,ippiGammaInv_32f_C3R, ( const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f vMin, Ipp32f vMax  ))
IPPAPI(IppStatus,ippiGammaInv_32f_C3IR, ( Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, Ipp32f vMin, Ipp32f vMax ))
IPPAPI(IppStatus,ippiGammaFwd_32f_AC4R, ( const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f vMin, Ipp32f vMax  ))
IPPAPI(IppStatus,ippiGammaFwd_32f_AC4IR, ( Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, Ipp32f vMin, Ipp32f vMax ))
IPPAPI(IppStatus,ippiGammaInv_32f_AC4R, ( const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f vMin, Ipp32f vMax  ))
IPPAPI(IppStatus,ippiGammaInv_32f_AC4IR, ( Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, Ipp32f vMin, Ipp32f vMax ))
IPPAPI(IppStatus,ippiGammaFwd_32f_P3R, ( const Ipp32f* pSrc[3], int srcStep, Ipp32f* pDst[3], int dstStep, IppiSize roiSize, Ipp32f vMin, Ipp32f vMax  ))
IPPAPI(IppStatus,ippiGammaFwd_32f_IP3R, ( Ipp32f* pSrcDst[3], int srcDstStep, IppiSize roiSize, Ipp32f vMin, Ipp32f vMax  ))
IPPAPI(IppStatus,ippiGammaInv_32f_P3R, ( const Ipp32f* pSrc[3], int srcStep, Ipp32f* pDst[3], int dstStep, IppiSize roiSize, Ipp32f vMin, Ipp32f vMax  ))
IPPAPI(IppStatus,ippiGammaInv_32f_IP3R, ( Ipp32f* pSrcDst[3], int srcDstStep, IppiSize roiSize, Ipp32f vMin, Ipp32f vMax  ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:     ippiToneMapLinear_32f8u_C1R ippiToneMapMean_32f8u_C1R
//  Purpose:   Convert HDRI into the LDRI. In the other words they map a dst Image into the range [0..255]
//
// Input Arguments:
//    pSrc     - pointer to the HDRI.
//    srcStep  - step through the source image
//  Output Arguments:
//    pDst     - pointer to the LDRI.
//    dstStep  - step through the source image
//  Returns:
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value
//  Notes:
//          ippiToneMapLinear_32f_C1RI - applies Linear Scale-Factor method to src image.
//          ippiToneMapMean_32f_C1RI   - applies Mean Value method to src image.
//
*/
IPPAPI(IppStatus,ippiToneMapLinear_32f8u_C1R,(const Ipp32f* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiToneMapMean_32f8u_C1R,  (const Ipp32f* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:     ippiYCbCr420To411_8u_P3R  ippiYCbCr420To411_8u_P3R  ippiYCbCr1620To420_8u_P3R
//  Purpose:   Converts an YCbCr420 to YCbCr411 image,
//                         YCbCr411 to YCbCr420 image,
//                         YCbCr420 to YCbCr1620 image,
//                         YCbCr1620 to YCbCr420 image.
//
// Input Arguments:
//    pSrc     - pointer to the source image.
//    srcStep  - step through the source image
//    roiSize  - Size of the ROI
//  Output Arguments:
//    pDst     - pointer to the destination image.
//    dstStep  - step through the destination image
//  Returns:
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value
//
*/
IPPAPI(IppStatus, ippiYCbCr420To411_8u_P3R, ( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize) )
IPPAPI(IppStatus, ippiYCbCr411To420_8u_P3R, ( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize) )
IPPAPI(IppStatus, ippiYCbCr420To1620_8u_P3R, ( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize) )
IPPAPI(IppStatus, ippiYCbCr1620To420_8u_P3R, ( const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize) )

/* /////////////////////////////////////////////////////////////////////////////
//
//  Names:        ippiRGBToRGB565_8u16u_C3R
//                ippiBGRToBGR565_8u16u_C3R
//
//  Purpose:      Converts RGB(BGR) image to RGB565(BGR565) image and vice versa
//
//  Returns:
//    ippStsNoErr         No errors
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsSizeErr       The roiSize has a field with negative or zero value
//    ippStsStepErr       One of steps is less than or equal to zero
//
//  Parameters:
//    pSrc                Pointers to the source images
//    srcStep             Steps through the source image
//    pDst                Pointer to the destination image
//    dstStep             Step through the destination image
//    roiSize             Size of the image ROI
//
*/

IPPAPI(IppStatus, ippiRGBToRGB565_8u16u_C3R,(const Ipp8u*  pSrc, int srcStep, Ipp16u*  pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGRToBGR565_8u16u_C3R,(const Ipp8u*  pSrc, int srcStep, Ipp16u*  pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiRGB565ToRGB_16u8u_C3R,(const Ipp16u* pSrc, int srcStep,  Ipp8u*  pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiBGR565ToBGR_16u8u_C3R,(const Ipp16u* pSrc, int srcStep,  Ipp8u*  pDst, int dstStep, IppiSize roiSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:        ippiCFAToRGB_8u_C1C3R
//                ippiCFAToRGB_16u_C1C3R
//
//  Purpose:      Bayer transform. Converts color filter array image to RGB image
//  Returns:
//    ippStsNoErr         No errors
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsBadArgErr     Wrong value of grid
//    ippStsSizeErr       The srcSize.width<2 or the srcSize.height<2 or
//                        the roiSize has a field with negative or zero value
//  Parameters:
//    pSrc                Pointers to the source image
//    srcRoi              ROI in source image(top left corner and size)
//    srcSize             Size of source image
//    srcStep             Steps through the source image
//    pDst                Pointer to the destination image
//    dstStep             Step through the destination image
//    grid                Type of baeyers grid
//    interpolation       Reserved, must be 0
//
*/

#if !defined( _OWN_BLDPCS )
typedef enum {
  ippiBayerBGGR,
  ippiBayerRGGB,
  ippiBayerGBRG,
  ippiBayerGRBG
} IppiBayerGrid;
#endif /* _OWN_BLDPCS */

IPPAPI(IppStatus, ippiCFAToRGB_8u_C1C3R,(const Ipp8u* pSrc, IppiRect srcRoi, IppiSize srcSize, int srcStep,
   Ipp8u* pDst, int dstStep, IppiBayerGrid grid, int interpolation))
IPPAPI(IppStatus, ippiCFAToRGB_16u_C1C3R,(const Ipp16u* pSrc, IppiRect srcRoi, IppiSize srcSize, int srcStep,
   Ipp16u* pDst, int dstStep, IppiBayerGrid grid, int interpolation))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiCbYCr422ToYCbCr420_Interlace_8u_C2P3R,
//              ippiYCbCr420ToCbYCr422_Interlace_8u_P3C2R
//              ippiYCbCr422To420_Interlace_8u_P3R
//              ippiYCbCr420To422_Interlace_8u_P3R
//  Purpose:    Converts 2-channel interlaced UYVY, YUYV C2 and P3 images
//              to the P3 YUV420 image and vice versa
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize.width less than 2 or roiSize.height less than 4
//    ippStsDoubleSize         roiSize.width  is not multiples of 2 or
//                             roiSize.height is not multiples of 4
//
//  Parameters:
//    pSrc          Pointer(an array of pointers) to the source image
//    srcStep       Step(an array of steps) through the source image
//    roiSize       Size of the ROI, should be multiple of 2(width) and 4(height)
//    pDst          Pointer(an array of pointers)  to destination image.
//    dstStep       Step(an array of steps) through the destination image
//  Notes:
//    UYVU422 to YUV420 uses following formulas,
//       Yd = Ys
//       U(V)d0 = (3*U(V)s0 +     U(V)s2 + 2) / 4
//       U(V)d1 = (  U(V)s1 + 3 * U(V)s3 + 2) / 4,
//       where    Us0(1), Us2(3) are interleaved adjacent lines
//
//    YUV420 to UYVU422 uses following formulas,
//       U(V)d0 = (5*U(V)s0 + 3*U(V)s2 + 4) / 8;
//       U(V)d1 = (7*U(V)s1 +   U(V)s3 + 4) / 8;
//       U(V)d2 = (  U(V)s0 + 7*U(V)s2 + 4) / 8;
//       U(V)d3 = (3*U(V)s1 + 5*U(V)s3 + 4) / 8;
//
//
//
*/
IPPAPI(IppStatus, ippiCbYCr422ToYCbCr420_Interlace_8u_C2P3R,(const Ipp8u* pSrc,
                int srcStep,Ipp8u* pDst[3],int dstStep[3], IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr422To420_Interlace_8u_P3R,(const Ipp8u* pSrc[3],
           int srcStep[3], Ipp8u* pDst[3], int dstStep[3], IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420ToCbYCr422_Interlace_8u_P3C2R,(const Ipp8u* pSrc[3],
               int   srcStep[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiYCbCr420To422_Interlace_8u_P3R,(const Ipp8u* pSrc[3],
          int srcStep[3], Ipp8u* pDst[3],  int dstStep[3], IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:        ippiDemosaicAHD_8u_C1C3R
//                ippiDemosaicAHD_16u_C1C3R
//
//  Purpose:      Converts color filter array image to RGB image wih AHD algorithm.
//                Reference: K. Hirakawa, T.W. Parks, "Adaptive Homogeneity-Directed Demosaicing Algorithm,"
//                IEEE Trans. Image Processing, March 2005,
//                Link: h__p://www.accidentalmark.com/research/papers/Hirakawa05MNdemosaicTIP.pdf
//
//  Returns:
//    ippStsNoErr         No errors
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsBadArgErr     Wrong value of grid
//    ippStsSizeErr       The srcSize.width<5 or the srcSize.height<5 or
//                        the roiSize has a field with negative or zero value
//  Parameters:
//    pSrc                Pointers to the source image
//    srcRoi              ROI in source image(top left corner and size)
//    srcSize             Size of source image
//    srcStep             Steps through the source image
//    pDst                Pointer to the destination image
//    dstStep             Step through the destination image
//    grid                Type of baeyers grid
//    pTmp                Pointer to the temporary image of (srcRoi.width + 6, 30) size
//    tmpStep             Steps through the temporary image
//
*/
IPPAPI(IppStatus, ippiDemosaicAHD_8u_C1C3R,(const Ipp8u* pSrc, IppiRect srcRoi, IppiSize srcSize, int srcStep,
   Ipp8u* pDst, int dstStep, IppiBayerGrid grid, Ipp8u* pTmp, int tmpStep))
IPPAPI(IppStatus, ippiDemosaicAHD_16u_C1C3R,(const Ipp16u* pSrc, IppiRect srcRoi, IppiSize srcSize, int srcStep,
   Ipp16u* pDst, int dstStep, IppiBayerGrid grid, Ipp16u* pTmp, int tmpStep))


/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippiRGBToYCoCg_8u_C3P3R   
//
//  Purpose:
//    RGB to YCoCg color conversion
//
//  Parameter:
//    pSrc      pointer to the input RGB data
//    srcStep   line offset in input data
//    pDst      pointer to pointers to the output YCoCg data.
//    dstStep   line offset in output data
//    roiSize   ROI size
//
//  Returns:
//    IppStatus
//
*/
IPPAPI(IppStatus, ippiRGBToYCoCg_8u_C3P3R,(
  const Ipp8u*   pSrc,
        int      srcStep,
        Ipp8u*   pDst[3],
        int      dstStep,
        IppiSize roi))


/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippiYCoCgToRGB_8u_P3C3R   
//
//  Purpose:
//    YCoCg to RGB  color conversion
//
//  Parameter:
//    pSrc      pointer to pointers to the input YCoCg data.
//    srcStep   line offset in input data
//    pDst      pointer to the output RGB data.
//    dstStep   line offset in output data
//    roiSize   ROI size
//
//  Returns:
//    IppStatus
//
*/
IPPAPI(IppStatus, ippiYCoCgToRGB_8u_P3C3R,(
  const Ipp8u*   pSrc[3],
        int      srcStep,
        Ipp8u*   pDst,
        int      dstStep,
        IppiSize roi))


#ifdef __cplusplus
}
#endif

#if defined (_IPP_STDCALL_CDECL)
  #undef  _IPP_STDCALL_CDECL
  #define __stdcall __cdecl
#endif

#endif /* __IPPCC_H__ */
/* ////////////////////////////// End of file /////////////////////////////// */

