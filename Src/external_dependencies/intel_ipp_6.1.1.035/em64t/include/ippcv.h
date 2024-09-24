/* ///////////////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright (c) 1999-2009 Intel Corporation. All Rights Reserved.
//
//              Intel(R) Integrated Performance Primitives
//                       Computer Vision (ippCV)
//
*/

#if !defined( __IPPCV_H__ ) || defined( _OWN_BLDPCS )
#define __IPPCV_H__

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

typedef enum _IppiKernelType {
    ippKernelSobel     =  0,
    ippKernelScharr    =  1
} IppiKernelType;

typedef enum _IppiNorm {
    ippiNormInf = 0,
    ippiNormL1 = 1,
    ippiNormL2 = 2,
    ippiNormFM = 3
} IppiNorm;

struct ipcvMorphState;
typedef struct ipcvMorphState IppiMorphState;

struct ipcvMorphAdvState;
typedef struct ipcvMorphAdvState IppiMorphAdvState;

struct ipcvMorphGrayState_8u;
typedef struct ipcvMorphGrayState_8u IppiMorphGrayState_8u;

struct ipcvMorphGrayState_32f;
typedef struct ipcvMorphGrayState_32f IppiMorphGrayState_32f;

struct ipcvConvState;
typedef struct ipcvConvState IppiConvState;

typedef struct _IppiConnectedComp {
    Ipp64f   area;    /*  area of the segmented component  */
    Ipp64f   value[3];/*  gray scale value of the segmented component  */
    IppiRect rect;    /*  bounding rectangle of the segmented component  */
} IppiConnectedComp;

struct PyramidState;
typedef struct PyramidState IppiPyramidState;

typedef IppiPyramidState IppiPyramidDownState_8u_C1R;
typedef IppiPyramidState IppiPyramidDownState_16u_C1R;
typedef IppiPyramidState IppiPyramidDownState_32f_C1R;
typedef IppiPyramidState IppiPyramidDownState_8u_C3R;
typedef IppiPyramidState IppiPyramidDownState_16u_C3R;
typedef IppiPyramidState IppiPyramidDownState_32f_C3R;
typedef IppiPyramidState IppiPyramidUpState_8u_C1R;
typedef IppiPyramidState IppiPyramidUpState_16u_C1R;
typedef IppiPyramidState IppiPyramidUpState_32f_C1R;
typedef IppiPyramidState IppiPyramidUpState_8u_C3R;
typedef IppiPyramidState IppiPyramidUpState_16u_C3R;
typedef IppiPyramidState IppiPyramidUpState_32f_C3R;


typedef struct _IppiPyramid {
    Ipp8u         **pImage;
    IppiSize      *pRoi;
    Ipp64f        *pRate;
    int           *pStep;
    Ipp8u         *pState;
    int            level;
} IppiPyramid;

struct OptFlowPyrLK;
typedef struct OptFlowPyrLK IppiOptFlowPyrLK;

typedef IppiOptFlowPyrLK IppiOptFlowPyrLK_8u_C1R;
typedef IppiOptFlowPyrLK IppiOptFlowPyrLK_16u_C1R;
typedef IppiOptFlowPyrLK IppiOptFlowPyrLK_32f_C1R;

struct ipcvHaarClassifier_32f;
typedef struct ipcvHaarClassifier_32f IppiHaarClassifier_32f;

struct ipcvHaarClassifier_32s;
typedef struct ipcvHaarClassifier_32s IppiHaarClassifier_32s;

struct ipcvFGHistogramState_8u_C1R;
typedef struct ipcvFGHistogramState_8u_C1R IppFGHistogramState_8u_C1R;

struct ipcvFGHistogramState_8u_C3R;
typedef struct ipcvFGHistogramState_8u_C3R IppFGHistogramState_8u_C3R;

struct ipcvFGGaussianState_8u_C1R;
typedef struct ipcvFGGaussianState_8u_C1R IppFGGaussianState_8u_C1R;

struct ipcvFGGaussianState_8u_C3R;
typedef struct ipcvFGGaussianState_8u_C3R IppFGGaussianState_8u_C3R;

typedef struct _IppFGHistogramModel {  /* for C3 image */
    int                valueQuant;     /* 64 */
    int                changeQuant;    /* 32 */
    int                valueUse;       /* 30 */
    int                valueAll;       /* 50 */
    int                changeUse;      /* 50 */
    int                changeAll;      /* 80 */
    Ipp32f             updBGChange;    /* 0.0046 */
    Ipp32f             updBGRef;       /* 0.0010 */
    int                numFrame;
    float              detectionRate;
    float              brightnessDistortion;
    int                shadowBG;
} IppFGHistogramModel;

typedef struct _IppFGGaussianModel {
    int                numGauss;
    Ipp32f             priorBack;
    Ipp32f             updBGProb;
    int                winSize;
    int                numFrame;
    float              detectionRate;
    float              brightnessDistortion;
    int                shadowBG;
} IppFGGaussianModel;

typedef enum _IppiInpaintFlag {
    IPP_INPAINT_TELEA    =  0,
    IPP_INPAINT_NS       =  1
} IppiInpaintFlag;

struct ipcvInpaintState_8u;
typedef struct ipcvInpaintState_8u IppiInpaintState_8u_C1R;
typedef struct ipcvInpaintState_8u IppiInpaintState_8u_C3R;

#endif /* _OWN_BLDPCS */

#define IPP_SEGMENT_QUEUE     0x01
#define IPP_SEGMENT_DISTANCE  0x02
#define IPP_SEGMENT_BORDER_4  0x40
#define IPP_SEGMENT_BORDER_8  0x80

#define IPP_TRUNC(a,b) ((a)&~((b)-1))
#define IPP_APPEND(a,b) (((a)+(b)-1)&~((b)-1))

/* ///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//                   Functions declarations
//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////// */

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippcvGetLibVersion
//
//  Purpose:    getting of the library version
//
//  Returns:    the structure of information about  version of ippcv library
//
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
IPPAPI( const IppLibraryVersion*, ippcvGetLibVersion, (void) )


/****************************************************************************************\
*                               Copy with Subpixel Precision                             *
\****************************************************************************************/


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiCopySubpix_8u_C1R,              ippiCopySubpix_16u_C1R,
//        ippiCopySubpix_8u16u_C1R_Sfs,       ippiCopySubpix_16u32f_C1R,
//        ippiCopySubpix_8u32f_C1R,           ippiCopySubpix_32f_C1R
//
//  Purpose:   copies source image to destination image with interpolation
//
//  Returns:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     Pointer to source image
//    srcStep                  Step in source image
//    pDst                     Pointer to destination image
//    dstStep                  Step in destination image
//    roiSize                  Source and destination image ROI size.
//    dx                       x coeff of linear interpolation
//    dy                       y coeff of linear interpolation
//    scaleFactor              Output scale factor, >= 0
//
//  Notes:
//F*/

IPPAPI(IppStatus, ippiCopySubpix_8u_C1R,        (const Ipp8u* pSrc, int srcStep,
                  Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp32f dx, Ipp32f dy))

IPPAPI(IppStatus, ippiCopySubpix_8u16u_C1R_Sfs, (const Ipp8u* pSrc, int srcStep,
                  Ipp16u* pDst, int dstStep, IppiSize roiSize, Ipp32f dx, Ipp32f dy, int scaleFactor))

IPPAPI(IppStatus, ippiCopySubpix_8u32f_C1R,     (const Ipp8u* pSrc, int srcStep,
                  Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f dx, Ipp32f dy))

IPPAPI(IppStatus, ippiCopySubpix_16u_C1R,       (const Ipp16u* pSrc, int srcStep,
                  Ipp16u* pDst, int dstStep, IppiSize roiSize, Ipp32f dx, Ipp32f dy))

IPPAPI(IppStatus, ippiCopySubpix_16u32f_C1R,    (const Ipp16u* pSrc, int srcStep,
                  Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f dx, Ipp32f dy))

IPPAPI(IppStatus, ippiCopySubpix_32f_C1R,       (const Ipp32f* pSrc, int srcStep,
                  Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f dx, Ipp32f dy))


/*F////////////////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiCopySubpixIntersect_8u_C1R,              ippiCopySubpixIntersect_16u_C1R,
//        ippiCopySubpixIntersect_8u16u_C1R_Sfs,       ippiCopySubpixIntersect_16u32f_C1R,
//        ippiCopySubpixIntersect_8u32f_C1R,           ippiCopySubpixIntersect_32f_C1R
//
//  Purpose:   finds intersection of centered window in the source image and copies
//             in to destination image with the border
//             border pixel are taken from the source image or replicated if they are outside it
//
//  Returns:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     Pointer to source image
//    srcStep                  Step in source image
//    srcRoiSize               Source image ROI size.
//    pDst                     Pointer to destination image
//    dstStep                  Step in destination image
//    dstRoiSize               Destination image ROI size.
//    point                    Center of dst window in src image (subpixel)
//    pMin                     Top left corner of dst filled part
//    pMax                     Bottom right corner of dst filled part
//    scaleFactor              Output scale factor, >= 0
//
//  Notes:                     For integer point.x or point.y pixels from the last row
//                             or column are not copied. Branches are possible.
//F*/

IPPAPI(IppStatus, ippiCopySubpixIntersect_8u_C1R,        (const Ipp8u* pSrc, int srcStep,
                  IppiSize srcRoiSize, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                  IppiPoint_32f point, IppiPoint *pMin, IppiPoint *pMax))

IPPAPI(IppStatus, ippiCopySubpixIntersect_8u16u_C1R_Sfs, (const Ipp8u* pSrc, int srcStep,
                  IppiSize srcRoiSize, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                  IppiPoint_32f point, IppiPoint *pMin, IppiPoint *pMax, int scaleFactor))

IPPAPI(IppStatus, ippiCopySubpixIntersect_8u32f_C1R,     (const Ipp8u* pSrc, int srcStep,
                  IppiSize srcRoiSize, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                  IppiPoint_32f point, IppiPoint *pMin, IppiPoint *pMax))

IPPAPI(IppStatus, ippiCopySubpixIntersect_16u_C1R,       (const Ipp16u* pSrc, int srcStep,
                  IppiSize srcRoiSize, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                  IppiPoint_32f point, IppiPoint *pMin, IppiPoint *pMax))

IPPAPI(IppStatus, ippiCopySubpixIntersect_16u32f_C1R,    (const Ipp16u* pSrc, int srcStep,
                  IppiSize srcRoiSize, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                  IppiPoint_32f point, IppiPoint *pMin, IppiPoint *pMax))

IPPAPI(IppStatus, ippiCopySubpixIntersect_32f_C1R,       (const Ipp32f* pSrc, int srcStep,
                  IppiSize srcRoiSize, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                  IppiPoint_32f point, IppiPoint *pMin, IppiPoint *pMax))


/****************************************************************************************\
*                                     Line sampling                                      *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiSampleLine_8u_C1R, ippiSampleLine_8u_C3R,
//           ippiSampleLine_16u_C1R, ippiSampleLine_16u_C3R,
//           ippiSampleLine_32f_C1R, ippiSampleLine_32f_C3R,
//
//  Purpose: Reads values of pixels on the raster
//           line between two given points and write them to buffer.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsOutOfRangeErr      At least one of the points is outside the image ROI.
//
//  Parameters:
//    pSrc                     Source image
//    srcStep                  Its step
//    roiSize                  ROI size
//    pBuffer                  Pointer to buffer where the pixels are stored.
//                             It must have size >= max(abs(pt2.y - pt1.y)+1,
//                                                      abs(pt2.x - pt1.x)+1)*
//                                                  <size_of_pixel>.
//    pt1                      Starting point of the line segment.
//                             The pixel value will be stored to buffer first.
//    pt2                      Ending point of the line segment.
//                             The pixel value will be stored to buffer last.
//F*/

IPPAPI(IppStatus, ippiSampleLine_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
                                           IppiSize roiSize, Ipp8u* pDst,
                                           IppiPoint pt1, IppiPoint pt2 ))

IPPAPI(IppStatus, ippiSampleLine_8u_C3R, ( const Ipp8u* pSrc, int srcStep,
                                           IppiSize roiSize, Ipp8u* pDst,
                                           IppiPoint pt1, IppiPoint pt2 ))

IPPAPI(IppStatus, ippiSampleLine_16u_C1R, ( const Ipp16u* pSrc, int srcStep,
                                           IppiSize roiSize, Ipp16u* pDst,
                                           IppiPoint pt1, IppiPoint pt2 ))

IPPAPI(IppStatus, ippiSampleLine_16u_C3R, ( const Ipp16u* pSrc, int srcStep,
                                           IppiSize roiSize, Ipp16u* pDst,
                                           IppiPoint pt1, IppiPoint pt2 ))

IPPAPI(IppStatus, ippiSampleLine_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
                                            IppiSize roiSize, Ipp32f* pDst,
                                            IppiPoint pt1, IppiPoint pt2 ))

IPPAPI(IppStatus, ippiSampleLine_32f_C3R, ( const Ipp32f* pSrc, int srcStep,
                                            IppiSize roiSize, Ipp32f* pDst,
                                            IppiPoint pt1, IppiPoint pt2 ))


/****************************************************************************************\
*                                    Accumulation                                        *
\****************************************************************************************/

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAdd_8u32f_C1IR,   ippiAdd_8s32f_C1IR,
//              ippiAdd_16u32f_C1IR,
//              ippiAdd_8u32f_C1IMR,  ippiAdd_8s32f_C1IMR,
//              ippiAdd_16u32f_C1IMR, ippiAdd_32f_C1IMR
//
//  Purpose:    Add image to accumulator.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            Step is too small to fit image.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Arguments:
//    pSrc                     Pointer to source image
//    srcStep                  Step in the source image
//    pMask                    Pointer to mask
//    maskStep                 Step in the mask image
//    pSrcDst                  Pointer to accumulator image
//    srcDstStep               Step in the accumulator image
//    roiSize                  Image size
*/

IPPAPI(IppStatus, ippiAdd_8u32f_C1IR, (const Ipp8u*  pSrc, int srcStep,
                                       Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize ))

IPPAPI(IppStatus, ippiAdd_8s32f_C1IR, (const Ipp8s*  pSrc, int srcStep,
                                       Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize ))

IPPAPI(IppStatus, ippiAdd_16u32f_C1IR, (const Ipp16u*  pSrc, int srcStep,
                                        Ipp32f* pSrcDst, int srcDstStep,
                                        IppiSize roiSize ))


IPPAPI(IppStatus, ippiAdd_8u32f_C1IMR,(const Ipp8u*  pSrc, int srcStep,
                                       const Ipp8u* pMask, int maskStep,
                                       Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize ))

IPPAPI(IppStatus, ippiAdd_8s32f_C1IMR,(const Ipp8s*  pSrc, int srcStep,
                                       const Ipp8u* pMask, int maskStep,
                                       Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize ))

IPPAPI(IppStatus, ippiAdd_16u32f_C1IMR,(const Ipp16u*  pSrc, int srcStep,
                                        const Ipp8u* pMask, int maskStep,
                                        Ipp32f* pSrcDst, int srcDstStep,
                                        IppiSize roiSize ))

IPPAPI(IppStatus, ippiAdd_32f_C1IMR,  (const Ipp32f* pSrc, int srcStep,
                                       const Ipp8u* pMask, int maskStep,
                                       Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize ))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiAddSquare_8u32f_C1IR,   ippiAddSquare_8s32f_C1IR,
//          ippiAddSquare_16u32f_C1IR,  ippiAddSquare_32f_C1IR,
//          ippiAddSquare_8u32f_C1IMR,  ippiAddSquare_8s32f_C1IMR,
//          ippiAddSquare_16u32f_C1IMR, ippiAddSquare_32f_C1IMR
//
//  Purpose:    Add squared image (i.e. multiplied by itself) to accumulator.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            Step is too small to fit image.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Arguments:
//    pSrc                     Pointer to source image
//    srcStep                  Step in the source image
//    pMask                    Pointer to mask
//    maskStep                 Step in the mask image
//    pSrcDst                  Pointer to accumulator image
//    srcDstStep               Step in the accumulator image
//    roiSize                  Image size
*/

IPPAPI(IppStatus, ippiAddSquare_8u32f_C1IR, (const Ipp8u*  pSrc, int srcStep,
                                             Ipp32f* pSrcDst, int srcDstStep,
                                             IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddSquare_8s32f_C1IR, (const Ipp8s*  pSrc, int srcStep,
                                             Ipp32f* pSrcDst, int srcDstStep,
                                             IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddSquare_16u32f_C1IR, (const Ipp16u*  pSrc, int srcStep,
                                              Ipp32f* pSrcDst, int srcDstStep,
                                              IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddSquare_32f_C1IR,   (const Ipp32f* pSrc, int srcStep,
                                             Ipp32f* pSrcDst, int srcDstStep,
                                             IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddSquare_8u32f_C1IMR,(const Ipp8u* pSrc, int srcStep,
                                             const Ipp8u* pMask, int maskStep,
                                             Ipp32f* pSrcDst, int srcDstStep,
                                             IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddSquare_8s32f_C1IMR,(const Ipp8s* pSrc, int srcStep,
                                             const Ipp8u* pMask, int maskStep,
                                             Ipp32f* pSrcDst, int srcDstStep,
                                             IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddSquare_16u32f_C1IMR,(const Ipp16u* pSrc, int srcStep,
                                              const Ipp8u* pMask, int maskStep,
                                              Ipp32f* pSrcDst, int srcDstStep,
                                              IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddSquare_32f_C1IMR,  (const Ipp32f* pSrc, int srcStep,
                                             const Ipp8u* pMask, int maskStep,
                                             Ipp32f* pSrcDst, int srcDstStep,
                                             IppiSize roiSize ))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiAddProduct_8u32f_C1IR,   ippiAddProduct_8s32f_C1IR,
//        ippiAddProduct_16u32f_C1IR,  ippiAddProduct_32f_C1IR,
//        ippiAddProduct_8u32f_C1IMR,  ippiAddProduct_8s32f_C1IMR,
//        ippiAddProduct_16u32f_C1IMR, ippiAddProduct_32f_C1IMR
//
//  Purpose:  Add product of two images to accumulator.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            Step is too small to fit image.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Arguments:
//    pSrc1                    Pointer to first source image
//    src1Step                 Step in the first source image
//    pSrc2                    Pointer to second source image
//    src2Step                 Step in the second source image
//    pMask                    Pointer to mask
//    maskStep                 Step in the mask image
//    pSrcDst                  Pointer to accumulator image
//    srcDstStep               Step in the accumulator image
//    roiSize                  Image size
*/

IPPAPI(IppStatus, ippiAddProduct_8u32f_C1IR, (const Ipp8u*  pSrc1, int src1Step,
                                              const Ipp8u*  pSrc2, int src2Step,
                                              Ipp32f* pSrcDst, int srcDstStep,
                                              IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddProduct_8s32f_C1IR, (const Ipp8s*  pSrc1, int src1Step,
                                              const Ipp8s*  pSrc2, int src2Step,
                                              Ipp32f* pSrcDst, int srcDstStep,
                                              IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddProduct_16u32f_C1IR, (const Ipp16u*  pSrc1, int src1Step,
                                               const Ipp16u*  pSrc2, int src2Step,
                                               Ipp32f* pSrcDst, int srcDstStep,
                                               IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddProduct_32f_C1IR,   (const Ipp32f* pSrc1, int src1Step,
                                              const Ipp32f* pSrc2, int src2Step,
                                              Ipp32f* pSrcDst, int srcDstStep,
                                              IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddProduct_8u32f_C1IMR,(const Ipp8u*  pSrc1, int src1Step,
                                              const Ipp8u*  pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              Ipp32f* pSrcDst, int srcDstStep,
                                              IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddProduct_8s32f_C1IMR,(const Ipp8s*  pSrc1, int src1Step,
                                              const Ipp8s*  pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              Ipp32f* pSrcDst, int srcDstStep,
                                              IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddProduct_16u32f_C1IMR,(const Ipp16u*  pSrc1, int src1Step,
                                               const Ipp16u*  pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               Ipp32f* pSrcDst, int srcDstStep,
                                               IppiSize roiSize ))

IPPAPI(IppStatus, ippiAddProduct_32f_C1IMR,  (const Ipp32f* pSrc1, int src1Step,
                                              const Ipp32f* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              Ipp32f* pSrcDst, int srcDstStep,
                                              IppiSize roiSize ))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiAddWeighted_8u32f_C1IR,  ippiAddWeighted_8s32f_C1IR,
//        ippiAddWeighted_16u32f_C1IR, ippiAddWeighted_32f_C1IR,
//        ippiAddWeighted_8u32f_C1IMR, ippiAddWeighted_8s32f_C1IMR,
//        ippiAddWeighted_16u32f_C1IMR,ippiAddWeighted_32f_C1IMR
//        ippiAddWeighted_32f_C1R
//
//  Purpose:  Add image, multiplied by alpha, to accumulator, multiplied by (1 - alpha).
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            Step is too small to fit image.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Arguments:
//    pSrc1                    Pointer to first source image
//    src1Step                 Step in the first source image
//    pSrc2                    Pointer to second source image
//    src2Step                 Step in the second source image
//    pMask                    Pointer to mask
//    maskStep                 Step in the mask image
//    pSrcDst                  Pointer to accumulator image
//    srcDstStep               Step in the accumulator image
//    pDst                     Pointer to destination image
//    dstStep                  Step in the destination image
//    roiSize                  Image size
//    alpha                    Weight of source image
*/

IPPAPI(IppStatus, ippiAddWeighted_8u32f_C1IR, (const Ipp8u*  pSrc, int srcStep,
                                               Ipp32f* pSrcDst, int srcDstStep,
                                               IppiSize roiSize, Ipp32f alpha ))

IPPAPI(IppStatus, ippiAddWeighted_8s32f_C1IR, (const Ipp8s*  pSrc, int srcStep,
                                               Ipp32f* pSrcDst, int srcDstStep,
                                               IppiSize roiSize, Ipp32f alpha ))

IPPAPI(IppStatus, ippiAddWeighted_16u32f_C1IR, (const Ipp16u*  pSrc, int srcStep,
                                                Ipp32f* pSrcDst, int srcDstStep,
                                                IppiSize roiSize, Ipp32f alpha ))

IPPAPI(IppStatus, ippiAddWeighted_32f_C1IR,   (const Ipp32f* pSrc, int srcStep,
                                               Ipp32f* pSrcDst, int srcDstStep,
                                               IppiSize roiSize, Ipp32f alpha ))

IPPAPI(IppStatus, ippiAddWeighted_8u32f_C1IMR,(const Ipp8u* pSrc, int srcStep,
                                               const Ipp8u* pMask, int maskStep,
                                               Ipp32f* pSrcDst, int srcDstStep,
                                               IppiSize roiSize, Ipp32f alpha ))

IPPAPI(IppStatus, ippiAddWeighted_8s32f_C1IMR,(const Ipp8s* pSrc, int srcStep,
                                               const Ipp8u* pMask, int maskStep,
                                               Ipp32f* pSrcDst, int srcDstStep,
                                               IppiSize roiSize, Ipp32f alpha ))

IPPAPI(IppStatus, ippiAddWeighted_16u32f_C1IMR,(const Ipp16u* pSrc, int srcStep,
                                                const Ipp8u* pMask, int maskStep,
                                                Ipp32f* pSrcDst, int srcDstStep,
                                                IppiSize roiSize, Ipp32f alpha ))

IPPAPI(IppStatus, ippiAddWeighted_32f_C1IMR,  (const Ipp32f* pSrc, int srcStep,
                                               const Ipp8u* pMask, int maskStep,
                                               Ipp32f* pSrcDst, int srcDstStep,
                                               IppiSize roiSize, Ipp32f alpha ))

IPPAPI(IppStatus, ippiAddWeighted_32f_C1R, ( const Ipp32f*  pSrc1, int src1Step,
                                             const Ipp32f*  pSrc2, int src2Step,
                                              Ipp32f* pDst, int dstStep,
                                              IppiSize roiSize, Ipp32f alpha ))


/****************************************************************************************\
*                                 Absolute difference                                    *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiAbsDiff_8u_C1R, ippiAbsDiff_8u_C3R, ippiAbsDiff_16u_C1R, ippiAbsDiff_32f_C1R,
//
//  Purpose: Calculate absolute difference between corresponding pixels of the two images
//           or between image pixels and scalar.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc1                    Source image
//    src1Step                 Its step
//    pSrc2                    Second source image
//    src2Step                 Its step
//    pDst                     Destination image
//    dstStep                  Its step
//    roiSize                  ROI size
//F*/

IPPAPI(IppStatus, ippiAbsDiff_8u_C1R, ( const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus, ippiAbsDiff_8u_C3R, ( const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        Ipp8u* pDst, int dstStep, IppiSize roiSize ))

IPPAPI(IppStatus, ippiAbsDiff_16u_C1R, ( const Ipp16u* pSrc1, int src1Step,
                                         const Ipp16u* pSrc2, int src2Step,
                                         Ipp16u* pDst, int dstStep, IppiSize roiSize ))

IPPAPI(IppStatus, ippiAbsDiff_32f_C1R, ( const Ipp32f* pSrc1, int src1Step,
                                         const Ipp32f* pSrc2, int src2Step,
                                         Ipp32f* pDst, int dstStep, IppiSize roiSize ))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiAbsDiffC_8u_C1R, ippiAbsDiffC_16u_C1R, ippiAbsDiffC_32f_C1R,
//
//  Purpose: Calculate absolute difference between image pixels and scalar.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     Source image
//    srcStep                  Its step
//    pDst                     Destination image: dst(x,y) = abs(src(x,y) - value)
//    dstStep                  Its step
//    roiSize                      ROI size
//    value                    Scalar value to compare with. For 8u function
//                             If scalar is not within [0,255], it is clipped
//                             ( value = value < 0 ? 0 : value > 255 ? 255 : value )
//F*/

IPPAPI(IppStatus, ippiAbsDiffC_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
                                         Ipp8u* pDst, int dstStep,
                                         IppiSize roiSize, int value ))

IPPAPI(IppStatus, ippiAbsDiffC_16u_C1R, ( const Ipp16u* pSrc, int srcStep,
                                          Ipp16u* pDst, int dstStep,
                                          IppiSize roiSize, int value ))

IPPAPI(IppStatus, ippiAbsDiffC_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
                                          Ipp32f* pDst, int dstStep,
                                          IppiSize roiSize, Ipp32f value ))


/****************************************************************************************\
*                                Morphological Operations                                *
\****************************************************************************************/

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphologyGetSize_8u_C1R,            ippiMorphologyGetSize_32f_C1R,
//          ippiMorphologyGetSize_8u_C3R,            ippiMorphologyGetSize_32f_C3R,
//          ippiMorphologyGetSize_8u_C4R,            ippiMorphologyGetSize_32f_C4R
//
//  Purpose:  Gets size of internal state of morphological operation.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of image or width or height of structuring
//                             element is less or equal zero.
//
//  Arguments:
//    roiWidth                 Width of image ROI in pixels
//    pMask                    Pointer to structuring element (mask)
//    maskSize                 Size of structuring element
//    pSize                    Pointer to state length (GetSize)
*/

IPPAPI(IppStatus, ippiMorphologyGetSize_8u_C1R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize, int* pSize))
IPPAPI(IppStatus, ippiMorphologyGetSize_8u_C3R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize, int* pSize))
IPPAPI(IppStatus, ippiMorphologyGetSize_8u_C4R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize, int* pSize))
IPPAPI(IppStatus, ippiMorphologyGetSize_32f_C1R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize, int* pSize))
IPPAPI(IppStatus, ippiMorphologyGetSize_32f_C3R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize, int* pSize))
IPPAPI(IppStatus, ippiMorphologyGetSize_32f_C4R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize, int* pSize))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphologyInit_8u_C1R,               ippiMorphologyInit_32f_C1R,
//          ippiMorphologyInit_8u_C3R,               ippiMorphologyInit_32f_C3R,
//          ippiMorphologyInit_8u_C4R,               ippiMorphologyInit_32f_C4R,
//
//  Purpose:  Initialize internal state of morphological operation.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of image or width or height of structuring
//                             element is less or equal zero.
//    ippStsAnchorErr          Anchor point is outside the structuring element
//
//  Arguments:
//    roiWidth                 Width of image ROI in pixels
//    pMask                    Pointer to structuring element (mask)
//    maskSize                 Size of structuring element
//    anchor                   Anchor of the structuring element
//    pState                   Pointer to morphological state (Init)
*/

IPPAPI(IppStatus, ippiMorphologyInit_8u_C1R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                              IppiPoint anchor, IppiMorphState* pState))
IPPAPI(IppStatus, ippiMorphologyInit_8u_C3R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                              IppiPoint anchor, IppiMorphState* pState))
IPPAPI(IppStatus, ippiMorphologyInit_8u_C4R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                              IppiPoint anchor, IppiMorphState* pState))
IPPAPI(IppStatus, ippiMorphologyInit_32f_C1R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                               IppiPoint anchor, IppiMorphState* pState))
IPPAPI(IppStatus, ippiMorphologyInit_32f_C3R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                               IppiPoint anchor, IppiMorphState* pState))
IPPAPI(IppStatus, ippiMorphologyInit_32f_C4R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                               IppiPoint anchor, IppiMorphState* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphologyInitAlloc_8u_C1R,          ippiMorphologyInitAlloc_32f_C1R,
//          ippiMorphologyInitAlloc_8u_C3R,          ippiMorphologyInitAlloc_32f_C3R,
//          ippiMorphologyInitAlloc_8u_C4R,          ippiMorphologyInitAlloc_32f_C4R
//
//  Purpose:  Allocate buffers and initialize internal state of morphological operation.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of image or width or height of structuring
//                             element is less or equal zero.
//    ippStsAnchorErr          Anchor point is outside the structuring element
//    ippStsMemAllocErr        Memory allocation error
//
//  Arguments:
//    roiWidth                 Width of image ROI in pixels
//    pMask                    Pointer to structuring element (mask)
//    maskSize                 Size of structuring element
//    anchor                   Anchor of the structuring element
//    ppState                  Double pointer to morphological state (InitAlloc)
*/

IPPAPI(IppStatus, ippiMorphologyInitAlloc_8u_C1R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                                   IppiPoint anchor, IppiMorphState** ppState))
IPPAPI(IppStatus, ippiMorphologyInitAlloc_8u_C3R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                                   IppiPoint anchor, IppiMorphState** ppState))
IPPAPI(IppStatus, ippiMorphologyInitAlloc_8u_C4R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                                   IppiPoint anchor, IppiMorphState** ppState))
IPPAPI(IppStatus, ippiMorphologyInitAlloc_32f_C1R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                                    IppiPoint anchor, IppiMorphState** ppState))
IPPAPI(IppStatus, ippiMorphologyInitAlloc_32f_C3R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                                    IppiPoint anchor, IppiMorphState** ppState))
IPPAPI(IppStatus, ippiMorphologyInitAlloc_32f_C4R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                                    IppiPoint anchor, IppiMorphState** ppState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphologyFree
//
//  Purpose:  Releases buffers, allocated by ippiMorphologyInitAlloc
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//
//  Arguments:
//    morphState               Pointer to morphological state.
*/

IPPAPI(IppStatus, ippiMorphologyFree,(IppiMorphState* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiDilateBorderReplicate_8u_C1R,    ippiDilateBorderReplicate_8u_C3R,
//          ippiDilateBorderReplicate_8u_C4R,    ippiDilateBorderReplicate_32f_C1R,
//          ippiDilateBorderReplicate_32f_C3R,   ippiDilateBorderReplicate_32f_C4R
//
//          ippiErodeBorderReplicate_8u_C1R,     ippiErodeBorderReplicate_8u_C3R,
//          ippiErodeBorderReplicate_8u_C4R,     ippiErodeBorderReplicate_32f_C1R,
//          ippiErodeBorderReplicate_32f_C3R,    ippiErodeBorderReplicate_32f_C4R,
//
//  Purpose:    Perform erosion/dilation of image arbitrary shape structuring element.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The ROI width or height is less than 1
//                             or ROI width is bigger than ROI width in state
//    ippStsStepErr            Step is too small to fit image.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsBadArgErr          Bad border type
//
//  Arguments:
//    pSrc                     The pointer to source image
//    srcStep                  The step in source image
//    pDst                     The pointer to destination image
//    dstStep                  The step in destination image
//    roiSize                  ROI size
//    borderType               Type of border (ippBorderRepl now)
//    pState                   Pointer to morphological operation state
*/

IPPAPI(IppStatus, ippiDilateBorderReplicate_8u_C1R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
IPPAPI(IppStatus, ippiDilateBorderReplicate_8u_C3R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
IPPAPI(IppStatus, ippiDilateBorderReplicate_8u_C4R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
IPPAPI(IppStatus, ippiDilateBorderReplicate_32f_C1R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
IPPAPI(IppStatus, ippiDilateBorderReplicate_32f_C3R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
IPPAPI(IppStatus, ippiDilateBorderReplicate_32f_C4R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))

IPPAPI(IppStatus, ippiErodeBorderReplicate_8u_C1R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
IPPAPI(IppStatus, ippiErodeBorderReplicate_8u_C3R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
IPPAPI(IppStatus, ippiErodeBorderReplicate_8u_C4R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
IPPAPI(IppStatus, ippiErodeBorderReplicate_32f_C1R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
IPPAPI(IppStatus, ippiErodeBorderReplicate_32f_C3R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
IPPAPI(IppStatus, ippiErodeBorderReplicate_32f_C4R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))


/****************************************************************************************\
*                       Advanced Morphological Operations                                *
\****************************************************************************************/


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphAdvGetSize_8u_C1R,            ippiMorphAdvGetSize_32f_C1R,
//          ippiMorphAdvGetSize_8u_C3R,            ippiMorphAdvGetSize_32f_C3R,
//          ippiMorphAdvGetSize_8u_C4R,            ippiMorphAdvGetSize_32f_C4R
//
//  Purpose:  Gets size of internal state of advanced morphological operation.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of image or width or height of structuring
//                             element is less or equal zero.
//
//  Arguments:
//    roiSize                  Maximal image ROI in pixels
//    pMask                    Pointer to structuring element (mask)
//    maskSize                 Size of structuring element
//    pSize                    Pointer to state length (GetSize)
*/

IPPAPI(IppStatus, ippiMorphAdvGetSize_8u_C1R,(IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, int* stateSize))
IPPAPI(IppStatus, ippiMorphAdvGetSize_8u_C3R,(IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, int* stateSize))
IPPAPI(IppStatus, ippiMorphAdvGetSize_8u_C4R,(IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, int* stateSize))
IPPAPI(IppStatus, ippiMorphAdvGetSize_32f_C1R,(IppiSize roiSize, const Ipp8u* pMask,
                                                      IppiSize maskSize, int* stateSize))
IPPAPI(IppStatus, ippiMorphAdvGetSize_32f_C3R,(IppiSize roiSize, const Ipp8u* pMask,
                                                      IppiSize maskSize, int* stateSize))
IPPAPI(IppStatus, ippiMorphAdvGetSize_32f_C4R,(IppiSize roiSize, const Ipp8u* pMask,
                                                      IppiSize maskSize, int* stateSize))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphAdvInit_8u_C1R,               ippiMorphAdvInit_32f_C1R,
//          ippiMorphAdvInit_8u_C3R,               ippiMorphAdvInit_32f_C3R,
//          ippiMorphAdvInit_8u_C4R,               ippiMorphAdvInit_32f_C4R,
//
//  Purpose:  Initialize internal state of advanced morphological operation.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of image or width or height of structuring
//                             element is less or equal zero.
//    ippStsAnchorErr          Anchor point is outside the structuring element
//
//  Arguments:
//    pState                   Pointer to morphological state (Init)
//    roiSize                  Maximal image ROI in pixels
//    pMask                    Pointer to structuring element (mask)
//    maskSize                 Size of structuring element
//    anchor                   Anchor of the structuring element
*/

IPPAPI(IppStatus, ippiMorphAdvInit_8u_C1R,(IppiMorphAdvState* morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiMorphAdvInit_8u_C3R,(IppiMorphAdvState* morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiMorphAdvInit_8u_C4R,(IppiMorphAdvState* morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiMorphAdvInit_32f_C1R,(IppiMorphAdvState* morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiMorphAdvInit_32f_C3R,(IppiMorphAdvState* morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiMorphAdvInit_32f_C4R,(IppiMorphAdvState* morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                IppiSize maskSize, IppiPoint anchor))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphAdvInitAlloc_8u_C1R,          ippiMorphAdvInitAlloc_32f_C1R,
//          ippiMorphAdvInitAlloc_8u_C3R,          ippiMorphAdvInitAlloc_32f_C3R,
//          ippiMorphAdvInitAlloc_8u_C4R,          ippiMorphAdvInitAlloc_32f_C4R
//
//  Purpose:  Allocate buffers and initialize internal state of advanced morphological operation.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of image or width or height of structuring
//                             element is less or equal zero.
//    ippStsAnchorErr          Anchor point is outside the structuring element
//    ippStsMemAllocErr        Memory allocation error
//
//  Arguments:
//    ppState                  Double pointer to morphological state (InitAlloc)
//    roiSize                  Maximal image ROI in pixels
//    pMask                    Pointer to structuring element (mask)
//    maskSize                 Size of structuring element
//    anchor                   Anchor of the structuring element
*/

IPPAPI(IppStatus, ippiMorphAdvInitAlloc_8u_C1R,(IppiMorphAdvState** morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiMorphAdvInitAlloc_8u_C3R,(IppiMorphAdvState** morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiMorphAdvInitAlloc_8u_C4R,(IppiMorphAdvState** morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiMorphAdvInitAlloc_32f_C1R,(IppiMorphAdvState** morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiMorphAdvInitAlloc_32f_C3R,(IppiMorphAdvState** morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiMorphAdvInitAlloc_32f_C4R,(IppiMorphAdvState** morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, IppiPoint anchor))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphAdvFree
//
//  Purpose:  Releases buffers, allocated by rippiMorphAdvInitAlloc
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Null pointer to pointer to morphological state.
//
//  Arguments:
//    pState               double pointer to morphological state.
*/

IPPAPI(IppStatus, ippiMorphAdvFree,(IppiMorphAdvState* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphCloseBorder_8u_C1R,             ippiMorphCloseBorder_8u_C3R,
//          ippiMorphCloseBorder_8u_C4R,             ippiMorphCloseBorder_32f_C1R,
//          ippiMorphCloseBorder_32f_C3R,            ippiMorphCloseBorder_32f_C4R
//
//          ippiMorphOpenBorder_8u_C1R,              ippiMorphOpenBorder_8u_C3R,
//          ippiMorphOpenBorder_8u_C4R,              ippiMorphOpenBorder_32f_C1R,
//          ippiMorphOpenBorder_32f_C3R,             ippiMorphOpenBorder_32f_C4R,
//
//          ippiMorphTophatBorder_8u_C1R,            ippiMorphTophatBorder_8u_C3R,
//          ippiMorphTophatBorder_8u_C4R,            ippiMorphTophatBorder_32f_C1R,
//          ippiMorphTophatBorder_32f_C3R,           ippiMorphTophatBorder_32f_C4R,
//
//          ippiMorphBlackhatBorder_8u_C1R,          ippiMorphBlackhatBorder_8u_C3R,
//          ippiMorphBlackhatBorder_8u_C4R,          ippiMorphBlackhatBorder_32f_C1R,
//          ippiMorphBlackhatBorder_32f_C3R,         ippiMorphBlackhatBorder_32f_C4R,
//
//          ippiMorphGradientBorder_8u_C1R,     ippiMorphGradientBorder_8u_C3R,
//          ippiMorphGradientBorder_8u_C4R,     ippiMorphGradientBorder_32f_C1R,
//          ippiMorphGradientBorder_32f_C3R,    ippiMorphGradientBorder_32f_C4R,
//
//  Purpose:    Perform advanced morphology of image arbitrary shape structuring element.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The ROI width or height is less than 1
//                             or ROI width is bigger than ROI width in state
//    ippStsStepErr            Step is too small to fit image.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsBadArgErr          Bad border type
//
//  Arguments:
//    pSrc                     The pointer to source image
//    srcStep                  The step in source image
//    pDst                     The pointer to destination image
//    dstStep                  The step in destination image
//    roiSize                  ROI size
//    borderType               Type of border (ippBorderRepl now)
//    pState                   Pointer to morphological operation state
*/

IPPAPI(IppStatus, ippiMorphOpenBorder_8u_C1R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphOpenBorder_8u_C3R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphOpenBorder_8u_C4R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphOpenBorder_32f_C1R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphOpenBorder_32f_C3R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphOpenBorder_32f_C4R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))

IPPAPI(IppStatus, ippiMorphCloseBorder_8u_C1R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphCloseBorder_8u_C3R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphCloseBorder_8u_C4R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphCloseBorder_32f_C1R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphCloseBorder_32f_C3R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphCloseBorder_32f_C4R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))

IPPAPI(IppStatus, ippiMorphTophatBorder_8u_C1R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphTophatBorder_8u_C3R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphTophatBorder_8u_C4R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphTophatBorder_32f_C1R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphTophatBorder_32f_C3R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphTophatBorder_32f_C4R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))

IPPAPI(IppStatus, ippiMorphBlackhatBorder_8u_C1R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphBlackhatBorder_8u_C3R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphBlackhatBorder_8u_C4R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphBlackhatBorder_32f_C1R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphBlackhatBorder_32f_C3R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphBlackhatBorder_32f_C4R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))

IPPAPI(IppStatus, ippiMorphGradientBorder_8u_C1R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphGradientBorder_8u_C3R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphGradientBorder_8u_C4R,(
                const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphGradientBorder_32f_C1R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphGradientBorder_32f_C3R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))
IPPAPI(IppStatus, ippiMorphGradientBorder_32f_C4R,(
                const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                IppiSize roiSize, IppiBorderType borderType, IppiMorphAdvState* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphGrayGetSize_8u_C1R,            ippiMorphGrayGetSize_32f_C1R
//
//  Purpose:  Gets size of internal state of gray-kernel morphological operation.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of image or width or height of structuring
//                             element is less or equal zero.
//
//  Arguments:
//    roiSize                  Maximal image ROI in pixels
//    pMask                    Pointer to structuring element
//    maskSize                 Size of structuring element
//    pSize                    Pointer to state length
*/

IPPAPI(IppStatus, ippiMorphGrayGetSize_8u_C1R,(IppiSize roiSize, const Ipp32s* pMask, IppiSize maskSize, int* pSize))

IPPAPI(IppStatus, ippiMorphGrayGetSize_32f_C1R,(IppiSize roiSize, const Ipp32f* pMask, IppiSize maskSize, int* pSize))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphGrayInit_8u_C1R,               ippiMorphGrayInit_32f_C1R
//
//  Purpose:  Initialize internal state of gray-scale morphological operation.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of image or width or height of structuring
//                             element is less or equal zero.
//    ippStsAnchorErr          Anchor point is outside the structuring element
//
//  Arguments:
//    roiSize                  Maximal image roiSize in pixels
//    pMask                    Pointer to structuring element (mask)
//    maskSize                 Size of structuring element
//    anchor                   Anchor of the structuring element
//    pState                   Pointer to morphological state (Init)
*/
IPPAPI(IppStatus, ippiMorphGrayInit_8u_C1R,(IppiMorphGrayState_8u* pState, IppiSize roiSize, const Ipp32s* pMask,
                                              IppiSize maskSize, IppiPoint anchor))

IPPAPI(IppStatus, ippiMorphGrayInit_32f_C1R,(IppiMorphGrayState_32f* pState, IppiSize roiSize, const Ipp32f* pMask,
                                              IppiSize maskSize, IppiPoint anchor))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphGrayInitAlloc_8u_C1R,          ippiMorphGrayInitAlloc_32f_C1R
//
//  Purpose:  Allocate buffers and initialize internal state of gray-scale morphological operation.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of image or width or height of structuring
//                             element is less or equal zero.
//    ippStsAnchorErr          Anchor point is outside the structuring element
//    ippStsMemAllocErr        Memory allocation error
//
//  Arguments:
//    roiSize                  Maximal image roiSize in pixels
//    pMask                    Pointer to structuring element (mask)
//    maskSize                 Size of structuring element
//    anchor                   Anchor of the structuring element
//    ppState                  Double pointer to morphological state (InitAlloc)
*/
IPPAPI(IppStatus, ippiMorphGrayInitAlloc_8u_C1R,(IppiMorphGrayState_8u** ppState, IppiSize roiSize, const Ipp32s* pMask,
                                                   IppiSize maskSize, IppiPoint anchor))

IPPAPI(IppStatus, ippiMorphGrayInitAlloc_32f_C1R,(IppiMorphGrayState_32f** ppState, IppiSize roiSize, const Ipp32f* pMask,
                                                   IppiSize maskSize, IppiPoint anchor))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphGrayFree_8u_C1R,       ippiMorphGrayFree_32f_C1R
//
//  Purpose:  Releases buffers, allocated by rippiMorphGrayInitAlloc
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Null pointer to pointer to morphological state.
//
//  Arguments:
//    pState                   Double pointer to morphological state.
*/

IPPAPI(IppStatus, ippiMorphGrayFree_8u_C1R,(IppiMorphGrayState_8u* pState))

IPPAPI(IppStatus, ippiMorphGrayFree_32f_C1R,(IppiMorphGrayState_32f* pState))


/*F///////////////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiGrayDilateBorder_8u_C1R,    ippiGrayDilateBorder_32f_C1R,
//          ippiGrayErodeBorder_8u_C1R,     ippiGrayErodeBorder_32f_C1R
//
//  Purpose:    Perform erosion/dilation of image with gray-scale structuring element.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The ROI width or height is less than 1
//                             or ROI width is bigger than ROI width in state
//    ippStsStepErr            Step is too small to fit image.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsBadArgErr          Bad border type
//
//  Arguments:
//    pSrc                     The pointer to source image
//    srcStep                  The step in source image
//    pDst                     The pointer to destination image
//    dstStep                  The step in destination image
//    roiSize                  ROI size
//    border                   Type of border (ippBorderRepl now)
//    pState                   Pointer to morphological operation state
*/
IPPAPI(IppStatus, ippiGrayErodeBorder_8u_C1R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType border, IppiMorphGrayState_8u* pState))

IPPAPI(IppStatus, ippiGrayErodeBorder_32f_C1R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType border, IppiMorphGrayState_32f* pState))

IPPAPI(IppStatus, ippiGrayDilateBorder_8u_C1R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType border, IppiMorphGrayState_8u* pState))

IPPAPI(IppStatus, ippiGrayDilateBorder_32f_C1R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType border, IppiMorphGrayState_32f* pState))


/*F/////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiMorphReconstructGetBufferSize_8u_C1,    ippiMorphReconstructGetBufferSize_8u_C1
//
//  Purpose:   returns buffer size for morphological reconstruction
//
//  Returns:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    roiSize                  The maximal ROI size.
//    pBufSize                 The pointer to the buffer size.
//
//  Notes:
//F*/

IPPAPI(IppStatus, ippiMorphReconstructGetBufferSize_8u_C1,(IppiSize roiSize, int *pSize))

IPPAPI(IppStatus, ippiMorphReconstructGetBufferSize_32f_C1,(IppiSize roiSize, int *pSize))


/*F/////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiMorphReconstructDilate_8u_C1IR,              ippiMorphReconstructErode_8u_C1IR,
//        ippiMorphReconstructDilate_32f_C1IR,             ippiMorphReconstructErode_32f_C1IR
//
//  Purpose:   performs morphological reconstruction of pSrcDst under/above pSrc
//
//  Returns:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     The pointer to source above/under image
//    srcStep                  The step in source image
//    pSrcDst                  The pointer to image to reconstruct
//    srcDstStep               The step in destination image
//    roiSize                  The source and destination image ROI size.
//    norm                     The norm type for dilation
//                                  ippiNormInf = Linf norm (8-connectivity)
//                                  ippiNormL1  = L1 norm   (4-connectivity)
//    pBuffer                  The pointer to working buffer
//
//  Notes:
//F*/

IPPAPI(IppStatus, ippiMorphReconstructDilate_8u_C1IR, (const Ipp8u* pSrc, int srcStep,
       Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, Ipp8u *pBuf, IppiNorm norm))

IPPAPI(IppStatus, ippiMorphReconstructErode_8u_C1IR, (const Ipp8u* pSrc, int srcStep,
       Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, Ipp8u *pBuf, IppiNorm norm))

IPPAPI(IppStatus, ippiMorphReconstructDilate_32f_C1IR, (const Ipp32f* pSrc, int srcStep,
       Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, Ipp32f *pBuf, IppiNorm norm))

IPPAPI(IppStatus, ippiMorphReconstructErode_32f_C1IR, (const Ipp32f* pSrc, int srcStep,
       Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, Ipp32f *pBuf, IppiNorm norm))


/****************************************************************************************\
*                                   Min/Max Filters                                      *
\****************************************************************************************/


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiFilterMinGetBufferSize_8u_C1R,       ippiFilterMaxGetBufferSize_8u_C1R,
//          ippiFilterMinGetBufferSize_32f_C1R,      ippiFilterMaxGetBufferSize_32f_C1R,
//          ippiFilterMinGetBufferSize_8u_C3R,       ippiFilterMaxGetBufferSize_8u_C3R,
//          ippiFilterMinGetBufferSize_32f_C3R,      ippiFilterMaxGetBufferSize_32f_C3R,
//          ippiFilterMinGetBufferSize_8u_C4R,       ippiFilterMaxGetBufferSize_8u_C4R,
//          ippiFilterMinGetBufferSize_32f_C4R,      ippiFilterMaxGetBufferSize_32f_C4R
//
//  Purpose:    Calculate buffer size for morphology operations with rectangular kernel
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsMaskSizeErr        Wrong mask size
//
//  Parameters:
//    roiWidth                 The image ROI width
//    maskSize                 The mask size
//    pBufferSize              The pointer to the buffer size
*/

IPPAPI(IppStatus, ippiFilterMinGetBufferSize_8u_C1R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

IPPAPI(IppStatus, ippiFilterMaxGetBufferSize_8u_C1R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

IPPAPI(IppStatus, ippiFilterMinGetBufferSize_32f_C1R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

IPPAPI(IppStatus, ippiFilterMaxGetBufferSize_32f_C1R, (int roiWidth, IppiSize maskSize, int *pBufferSize))


IPPAPI(IppStatus, ippiFilterMinGetBufferSize_8u_C3R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

IPPAPI(IppStatus, ippiFilterMaxGetBufferSize_8u_C3R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

IPPAPI(IppStatus, ippiFilterMinGetBufferSize_32f_C3R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

IPPAPI(IppStatus, ippiFilterMaxGetBufferSize_32f_C3R, (int roiWidth, IppiSize maskSize, int *pBufferSize))


IPPAPI(IppStatus, ippiFilterMinGetBufferSize_8u_C4R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

IPPAPI(IppStatus, ippiFilterMaxGetBufferSize_8u_C4R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

IPPAPI(IppStatus, ippiFilterMinGetBufferSize_32f_C4R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

IPPAPI(IppStatus, ippiFilterMaxGetBufferSize_32f_C4R, (int roiWidth, IppiSize maskSize, int *pBufferSize))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiFilterMaxBorderReplicate_8u_C1R,       ippiFilterMinBorderReplicate_8u_C1R,
//          ippiFilterMaxBorderReplicate_32f_C1R,      ippiFilterMinBorderReplicate_32f_C1R
//          ippiFilterMaxBorderReplicate_8u_C3R,       ippiFilterMinBorderReplicate_8u_C3R,
//          ippiFilterMaxBorderReplicate_32f_C3R,      ippiFilterMinBorderReplicate_32f_C3R
//          ippiFilterMaxBorderReplicate_8u_C4R,       ippiFilterMinBorderReplicate_8u_C4R,
//          ippiFilterMaxBorderReplicate_32f_C4R,      ippiFilterMinBorderReplicate_32f_C4R
//
//  Purpose:    Perform morphology operations with rectangular kernel
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsMaskSizeErr        Wrong mask size
//    ippStsAnchorErr          Anchor is outside the mask size.
//
//  Parameters:
//    pSrc                     The pointer to the source image
//    srcStep                  The step in the source image
//    pDst                     The pointer to the destination image
//    dstStep                  The step in the destination image
//    roiSize                  The image ROI size
//    maskSize                 The mask size
//    anchor                   The anchor position
//    pBuffer                  The pointer to the working buffer
//F*/

IPPAPI(IppStatus, ippiFilterMinBorderReplicate_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiFilterMaxBorderReplicate_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiFilterMinBorderReplicate_32f_C1R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiFilterMaxBorderReplicate_32f_C1R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))


IPPAPI(IppStatus, ippiFilterMinBorderReplicate_8u_C3R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiFilterMaxBorderReplicate_8u_C3R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiFilterMinBorderReplicate_32f_C3R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiFilterMaxBorderReplicate_32f_C3R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))


IPPAPI(IppStatus, ippiFilterMinBorderReplicate_8u_C4R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiFilterMaxBorderReplicate_8u_C4R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiFilterMinBorderReplicate_32f_C4R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiFilterMaxBorderReplicate_32f_C4R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))


/****************************************************************************************\
*                                   Separable Filters                                    *
\****************************************************************************************/


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiFilterRowBorderPipelineGetBufferSize_8u16s_C1R,   ippiFilterRowBorderPipelineGetBufferSize_8u16s_C3R
//         ippiFilterRowBorderPipelineGetBufferSize_16s_C1R,     ippiFilterRowBorderPipelineGetBufferSize_16s_C3R
//         ippiFilterRowBorderPipelineGetBufferSize_Low_16s_C1R, ippiFilterRowBorderPipelineGetBufferSize_Low_16s_C3R
//         ippiFilterRowBorderPipelineGetBufferSize_32f_C1R,     ippiFilterRowBorderPipelineGetBufferSize_32f_C3R
//
//  Purpose:    Get size of external buffer.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image or kernel size are less or equal zero
//
//  Parameters:
//    roiSize                  The image ROI size
//    kernelSize               The size of the kernel
//    pBufferSize              The pointer to the buffer size
*/

IPPAPI(IppStatus, ippiFilterRowBorderPipelineGetBufferSize_8u16s_C1R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterRowBorderPipelineGetBufferSize_8u16s_C3R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterRowBorderPipelineGetBufferSize_16s_C1R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterRowBorderPipelineGetBufferSize_16s_C3R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterRowBorderPipelineGetBufferSize_Low_16s_C1R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterRowBorderPipelineGetBufferSize_Low_16s_C3R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterRowBorderPipelineGetBufferSize_32f_C1R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterRowBorderPipelineGetBufferSize_32f_C3R, (IppiSize roiSize, int kernelSize, int* pBufferSize))


/*F///////////////////////////////////////////////////////////////////////////////////////////
//  Name:      ippiFilterRowBorderPipeline_8u16s_C1R,  ippiFilterRowBorderPipeline_8u16s_C3R
//             ippiFilterRowBorderPipeline_16s_C1R,    ippiFilterRowBorderPipeline_16s_C3R
//             ippiFilterRowBorderPipeline_Low_16s_C1R, ippiFilterRowBorderPipeline_Low_16s_C3R
//             ippiFilterRowBorderPipeline_32f_C1R,    ippiFilterRowBorderPipeline_32f_C3R
//
//  Purpose:   Convolves source image rows with the row kernel
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsAnchorErr          The anchor outside the kernel
//    ippStsBadArgErr          Wrong border type or zero divisor
//
//  Parameters:
//    pSrc                     The pointer to the source image
//    srcStep                  The step in the source image
//    ppDst                    The double pointer to the destination image
//    roiSize                  The image ROI size
//    pKernel                  The pointer to the kernel
//    kernelSize               The size of the kernel
//    xAnchor                  The anchor value , (0<=xAnchor<kernelSize)
//    borderType               The type of the border
//    borderValue              The value for the constant border
//    divisor                  The value to divide output pixels by , (for integer functions)
//    pBuffer                  The pointer to the working buffer
//    Notes:                   The output is the doulble pointer to support the circle buffer
//F*/

IPPAPI(IppStatus, ippiFilterRowBorderPipeline_8u16s_C1R, (const Ipp8u* pSrc, int srcStep, Ipp16s** ppDst,
                                        IppiSize roiSize, const Ipp16s* pKernel, int kernelSize, int xAnchor,
                                        IppiBorderType borderType, Ipp8u borderValue, int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterRowBorderPipeline_8u16s_C3R, (const Ipp8u* pSrc, int srcStep, Ipp16s** ppDst,
                                        IppiSize roiSize, const Ipp16s* pKernel, int kernelSize, int xAnchor,
                                        IppiBorderType borderType, Ipp8u borderValue[3], int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterRowBorderPipeline_16s_C1R, (const Ipp16s* pSrc, int srcStep, Ipp16s** ppDst,
                                        IppiSize roiSize, const Ipp16s* pKernel, int kernelSize, int xAnchor,
                                        IppiBorderType borderType, Ipp16s borderValue, int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterRowBorderPipeline_16s_C3R, (const Ipp16s* pSrc, int srcStep, Ipp16s** ppDst,
                                        IppiSize roiSize, const Ipp16s* pKernel, int kernelSize, int xAnchor,
                                        IppiBorderType borderType, Ipp16s borderValue[3], int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterRowBorderPipeline_Low_16s_C1R, (const Ipp16s* pSrc, int srcStep, Ipp16s** ppDst,
                                        IppiSize roiSize, const Ipp16s* pKernel, int kernelSize, int xAnchor,
                                        IppiBorderType borderType, Ipp16s borderValue, int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterRowBorderPipeline_Low_16s_C3R, (const Ipp16s* pSrc, int srcStep, Ipp16s** ppDst,
                                        IppiSize roiSize, const Ipp16s* pKernel, int kernelSize, int xAnchor,
                                        IppiBorderType borderType, Ipp16s borderValue[3], int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterRowBorderPipeline_32f_C1R, (const Ipp32f* pSrc, int srcStep, Ipp32f** ppDst,
                                        IppiSize roiSize, const Ipp32f* pKernel, int kernelSize, int xAnchor,
                                        IppiBorderType borderType, Ipp32f borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterRowBorderPipeline_32f_C3R, (const Ipp32f* pSrc, int srcStep, Ipp32f** ppDst,
                                        IppiSize roiSize, const Ipp32f* pKernel, int kernelSize, int xAnchor,
                                        IppiBorderType borderType, Ipp32f borderValue[3], Ipp8u* pBuffer))


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:      ippiFilterColumnPipelineGetBufferSize_16s_C1R,       ippiFilterColumnPipelineGetBufferSize_16s_C3R
//             ippiFilterColumnPipelineGetBufferSize_Low_16s_C1R,   ippiFilterColumnPipelineGetBufferSize_Low_16s_C3R
//             ippiFilterColumnPipelineGetBufferSize_16s8u_C1R,     ippiFilterColumnPipelineGetBufferSize_16s8u_C3R
//             ippiFilterColumnPipelineGetBufferSize_16s8s_C1R,     ippiFilterColumnPipelineGetBufferSize_16s8s_C3R
//             ippiFilterColumnPipelineGetBufferSize_32f_C1R,       ippiFilterColumnPipelineGetBufferSize_32f_C3R
//
//  Purpose:    Get size of external buffer.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image or kernel size are less or equal zero
//
//  Parameters:
//    roiSize                  The image ROI size
//    kernelSize               The size of the kernel
//    pBufferSize              The pointer to the buffer size
*/


IPPAPI(IppStatus, ippiFilterColumnPipelineGetBufferSize_16s_C1R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterColumnPipelineGetBufferSize_16s_C3R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterColumnPipelineGetBufferSize_Low_16s_C1R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterColumnPipelineGetBufferSize_Low_16s_C3R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterColumnPipelineGetBufferSize_16s8u_C1R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterColumnPipelineGetBufferSize_16s8u_C3R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterColumnPipelineGetBufferSize_16s8s_C1R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterColumnPipelineGetBufferSize_16s8s_C3R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterColumnPipelineGetBufferSize_32f_C1R, (IppiSize roiSize, int kernelSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterColumnPipelineGetBufferSize_32f_C3R, (IppiSize roiSize, int kernelSize, int* pBufferSize))


/*F///////////////////////////////////////////////////////////////////////////////////////////
//  Name:      ippiFilterColumnPipeline_16s_C1R,     ippiFilterColumnPipeline_16s_C3R
//             ippiFilterColumnPipeline_Low_16s_C1R, ippiFilterColumnPipeline_Low_16s_C3R
//             ippiFilterColumnPipeline_16s8u_C1R,   ippiFilterColumnPipeline_16s8u_C3R
//             ippiFilterColumnPipeline_16s8s_C1R,   ippiFilterColumnPipeline_16s8s_C3R
//             ippiFilterColumnPipeline_32f_C1R,     ippiFilterColumnPipeline_32f_C3R
//  Purpose:   Convolves source image rows with the row kernel
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsBadArgErr          Zero divisor
//
//  Parameters:
//    ppSrc                    The double pointer to the source image
//    pDst                     The pointer to the destination image
//    dstStep                  The step in the destination image
//    roiSize                  The image ROI size
//    pKernel                  The pointer to the kernel
//    kernelSize               The size of the kernel
//    divisor                  The value to divide output pixels by , (for integer functions)
//    pBuffer                  The pointer to the working buffer
//    Notes:                   The input is the doulble pointer to support the circle buffer
//F*/

IPPAPI(IppStatus, ippiFilterColumnPipeline_16s_C1R, (const Ipp16s** ppSrc, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                           const Ipp16s* pKernel, int kernelSize, int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterColumnPipeline_16s_C3R, (const Ipp16s** ppSrc, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                           const Ipp16s* pKernel, int kernelSize, int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterColumnPipeline_Low_16s_C1R, (const Ipp16s** ppSrc, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                           const Ipp16s* pKernel, int kernelSize, int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterColumnPipeline_Low_16s_C3R, (const Ipp16s** ppSrc, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                           const Ipp16s* pKernel, int kernelSize, int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterColumnPipeline_16s8u_C1R, (const Ipp16s** ppSrc, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                           const Ipp16s* pKernel, int kernelSize, int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterColumnPipeline_16s8u_C3R, (const Ipp16s** ppSrc, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                           const Ipp16s* pKernel, int kernelSize, int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterColumnPipeline_16s8s_C1R, (const Ipp16s** ppSrc, Ipp8s* pDst, int dstStep, IppiSize roiSize,
                                           const Ipp16s* pKernel, int kernelSize, int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterColumnPipeline_16s8s_C3R, (const Ipp16s** ppSrc, Ipp8s* pDst, int dstStep, IppiSize roiSize,
                                           const Ipp16s* pKernel, int kernelSize, int divisor, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterColumnPipeline_32f_C1R, (const Ipp32f** ppSrc, Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                           const Ipp32f* pKernel, int kernelSize, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterColumnPipeline_32f_C3R, (const Ipp32f** ppSrc, Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                           const Ipp32f* pKernel, int kernelSize, Ipp8u* pBuffer))


/****************************************************************************************\
*                                   Fixed Filters                                        *
\****************************************************************************************/


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiFilterScharrHorizGetBufferSize_8u16s_C1R,       ippiFilterScharrHorizGetBufferSize_32f_C1R,
//          ippiFilterScharrVertGetBufferSize_8u16s_C1R,        ippiFilterScharrVertGetBufferSize_32f_C1R,
//          ippiFilterSobelHorizGetBufferSize_8u16s_C1R,        ippiFilterSobelHorizGetBufferSize_32f_C1R,
//          ippiFilterSobelVertGetBufferSize_8u16s_C1R,         ippiFilterSobelVertGetBufferSize_32f_C1R,
//          ippiFilterSobelNegVertGetBufferSize_8u16s_C1R,      ippiFilterSobelNegVertGetBufferSize_32f_C1R,
//          ippiFilterSobelHorizSecondGetBufferSize_8u16s_C1R,  ippiFilterSobelHorizSecondGetBufferSize_32f_C1R,
//          ippiFilterSobelVertSecondGetBufferSize_8u16s_C1R,   ippiFilterSobelVertSecondGetBufferSize_32f_C1R,
//          ippiFilterSobelCrossGetBufferSize_8u16s_C1R,        ippiFilterSobelCrossGetBufferSize_32f_C1R,
//          ippiFilterLaplacianGetBufferSize_8u16s_C1R,         ippiFilterLaplacianGetBufferSize_32f_C1R,
//          ippiFilterLowpassGetBufferSize_8u_C1R,              ippiFilterLowpassGetBufferSize_32f_C1R,
//          ippiFilterScharrHorizGetBufferSize_8u8s_C1R,
//          ippiFilterScharrVertGetBufferSize_8u8s_C1R,
//          ippiFilterSobelHorizGetBufferSize_8u8s_C1R,
//          ippiFilterSobelVertGetBufferSize_8u8s_C1R,
//          ippiFilterSobelNegVertGetBufferSize_8u8s_C1R,
//          ippiFilterSobelHorizSecondGetBufferSize_8u8s_C1R,
//          ippiFilterSobelVertSecondGetBufferSize_8u8s_C1R,
//          ippiFilterSobelCrossGetBufferSize_8u8s_C1R,
//          ippiFilterLaplacianGetBufferSize_8u8s_C1R,
//
//
//  Purpose:    Perform convolution operation with fixed kernels 3x3 and 5x5
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsMaskSizeErr        Wrong mask size
//
//  Parameters:
//    roiSize                  The image ROI size
//    mask                     The mask size
//    pBufferSize              The pointer to the buffer size
*/

IPPAPI(IppStatus, ippiFilterScharrHorizGetBufferSize_8u16s_C1R,     (IppiSize roiSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterScharrVertGetBufferSize_8u16s_C1R,      (IppiSize roiSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelHorizGetBufferSize_8u16s_C1R,      (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelVertGetBufferSize_8u16s_C1R,       (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelNegVertGetBufferSize_8u16s_C1R,    (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelHorizSecondGetBufferSize_8u16s_C1R,(IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelVertSecondGetBufferSize_8u16s_C1R, (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelCrossGetBufferSize_8u16s_C1R,      (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterLaplacianGetBufferSize_8u16s_C1R,       (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))


IPPAPI(IppStatus, ippiFilterScharrHorizGetBufferSize_8u8s_C1R,     (IppiSize roiSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterScharrVertGetBufferSize_8u8s_C1R,      (IppiSize roiSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelHorizGetBufferSize_8u8s_C1R,      (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelVertGetBufferSize_8u8s_C1R,       (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelNegVertGetBufferSize_8u8s_C1R,    (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelHorizSecondGetBufferSize_8u8s_C1R,(IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelVertSecondGetBufferSize_8u8s_C1R, (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelCrossGetBufferSize_8u8s_C1R,      (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterLaplacianGetBufferSize_8u8s_C1R,       (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))


IPPAPI(IppStatus, ippiFilterLowpassGetBufferSize_8u_C1R,            (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))


IPPAPI(IppStatus, ippiFilterScharrHorizGetBufferSize_32f_C1R,     (IppiSize roiSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterScharrVertGetBufferSize_32f_C1R,      (IppiSize roiSize, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelHorizGetBufferSize_32f_C1R,      (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelVertGetBufferSize_32f_C1R,       (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelNegVertGetBufferSize_32f_C1R,    (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelHorizSecondGetBufferSize_32f_C1R,(IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelVertSecondGetBufferSize_32f_C1R, (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterSobelCrossGetBufferSize_32f_C1R,      (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterLaplacianGetBufferSize_32f_C1R,       (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

IPPAPI(IppStatus, ippiFilterLowpassGetBufferSize_32f_C1R,         (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiFilterScharrHorizBorder_8u16s_C1R,       ippiFilterScharrHorizBorder_32f_C1R
//          ippiFilterScharrVertBorder_8u16s_C1R,        ippiFilterScharrVertBorder_32f_C1R
//          ippiFilterSobelHorizBorder_8u16s_C1R,        ippiFilterSobelHorizBorder_32f_C1R
//          ippiFilterSobelVertBorder_8u16s_C1R,         ippiFilterSobelVertBorder_32f_C1R
//          ippiFilterSobelNegVertBorder_8u16s_C1R,      ippiFilterSobelNegVertBorder_32f_C1R
//          ippiFilterSobelHorizSecondBorder_8u16s_C1R,  ippiFilterSobelHorizSecondBorder_32f_C1R
//          ippiFilterSobelVertSecondBorder_8u16s_C1R,   ippiFilterSobelVertSecondBorder_32f_C1R
//          ippiFilterSobelCrossBorder_8u16s_C1R,        ippiFilterSobelCrossBorder_32f_C1R
//          ippiFilterLaplacianBorder_8u16s_C1R,         ippiFilterLaplacianBorder_32f_C1R
//          ippiFilterLowpassBorder_8u_C1R,              ippiFilterLowpassBorder_32f_C1R,
//          ippiFilterScharrHorizBorder_8u8s_C1R,
//          ippiFilterScharrVertBorder_8u8s_C1R,
//          ippiFilterSobelHorizBorder_8u8s_C1R,
//          ippiFilterSobelVertBorder_8u8s_C1R,
//          ippiFilterSobelNegVertBorder_8u8s_C1R,
//          ippiFilterSobelHorizSecondBorder_8u8s_C1R,
//          ippiFilterSobelVertSecondBorder_8u8s_C1R,
//          ippiFilterSobelCrossBorder_8u8s_C1R,
//          ippiFilterLaplacianBorder_8u8s_C1R
//
//  Purpose:    Perform convolution operation with fixed kernels 3x3 and 5x5
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsMaskSizeErr        Wrong mask size
//    ippStsBadArgErr          Wrong border type or zero divisor
//
//  Parameters:
//    pSrc                     The pointer to the source image
//    srcStep                  The step in the source image
//    pDst                     The pointer to the destination image
//    dstStep                  The step in the destination image
//    roiSize                  The image ROI size
//    mask                     The mask size
//    borderType               The type of the border
//    borderValue              The value for the constant border
//    pBuffer                  The pointer to the working buffer
//    divisor                  The value to divide output pixels by , (for integer functions)
//F*/

IPPAPI(IppStatus, ippiFilterScharrHorizBorder_8u16s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                      IppiBorderType borderType, Ipp8u borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterScharrVertBorder_8u16s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                      IppiBorderType borderType, Ipp8u borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelHorizBorder_8u16s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelVertBorder_8u16s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelNegVertBorder_8u16s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelHorizSecondBorder_8u16s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelVertSecondBorder_8u16s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelCrossBorder_8u16s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterLaplacianBorder_8u16s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterLowpassBorder_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterScharrHorizBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterScharrVertBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelHorizBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelVertBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelNegVertBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelHorizSecondBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelVertSecondBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelCrossBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterLaplacianBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))


IPPAPI(IppStatus, ippiFilterScharrHorizBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                      IppiBorderType borderType, Ipp32f borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterScharrVertBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                      IppiBorderType borderType, Ipp32f borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelHorizBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp32f  borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelVertBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp32f  borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelNegVertBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp32f  borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelHorizSecondBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp32f  borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelVertSecondBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp32f  borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterSobelCrossBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp32f  borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterLowpassBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp32f  borderValue, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiFilterLaplacianBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp32f  borderValue, Ipp8u* pBuffer))


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiGenSobelKernel_16s,       ippiGenSobelKernel_32f
//
//  Purpose:    Generate kernel for Sobel differential operator
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The size of kernel is less or equal zero
//    ippStsBadArgErr          derivative order is less than 0
//
//  Parameters:
//    pDst                     The pointer to the destination kernel
//    kernelSize               The kernel size, odd
//    dx                       The order of derivative (0<=dx<kernelSize)
//    sign                     Reverse signs in sign < 0
*/

IPPAPI (IppStatus, ippiGenSobelKernel_16s, (Ipp16s* pDst, int kernelSize, int dx, int sign))

IPPAPI (IppStatus, ippiGenSobelKernel_32f, (Ipp32f *pDst, int kernelSize, int dx, int sign))


/****************************************************************************************\
*                                   Image Integrals                                     *
\****************************************************************************************/


/*F/////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiIntegral_8u32s_C1R,             ippiIntegral_8u32f_C1R,
//        ippiSqrIntegral_8u32s64f_C1R,       ippiSqrIntegral_8u32f64f_C1R,
//        ippiTiltedIntegral_8u32s_C1R,       ippiTiltedIntegral_8u32f_C1R
//        ippiTiltedSqrIntegral_8u32s64f_C1R, ippiTiltedSqrIntegral_8u32f64f_C1R,
//        ippiSqrIntegral_8u32s_C1R,          ippiTiltedSqrIntegral_8u32s_C1R
//
//  Purpose:   calculates pixel sum on subimage
//
//  Returns:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     The pointer to source image
//    srcStep                  The step in source image
//    pDst                     The pointer to destination integral image
//    dstStep                  The step in destination image
//    pSq                      The pointer to destination square integral image
//    sqStep                   The step in destination image
//    roiSize                  The source and destination image ROI size.
//    val                      The value to add to pDst image pixels.
//    valSqr                   The value to add to pSq image pixels.
//
//  Notes:
//F*/

IPPAPI(IppStatus, ippiIntegral_8u32s_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp32s* pDst, int dstStep, IppiSize roiSize, Ipp32s val))

IPPAPI(IppStatus, ippiTiltedIntegral_8u32s_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp32s* pDst, int dstStep, IppiSize roiSize, Ipp32s val))

IPPAPI(IppStatus, ippiSqrIntegral_8u32s_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp32s* pDst, int dstStep, Ipp32s* pSqr, int sqrStep, IppiSize roi,
                  Ipp32s val, Ipp32s valSqr))

IPPAPI(IppStatus, ippiTiltedSqrIntegral_8u32s_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp32s* pDst, int dstStep, Ipp32s* pSqr, int sqrStep, IppiSize roi,
                  Ipp32s val, Ipp32s valSqr))

IPPAPI(IppStatus, ippiSqrIntegral_8u32s64f_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp32s* pDst, int dstStep, Ipp64f* pSqr, int sqrStep, IppiSize roiSize,
                  Ipp32s val, Ipp64f valSqr))

IPPAPI(IppStatus, ippiTiltedSqrIntegral_8u32s64f_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp32s* pDst, int dstStep, Ipp64f* pSqr, int sqrStep, IppiSize roiSize,
                  Ipp32s val, Ipp64f valSqr))


IPPAPI(IppStatus, ippiIntegral_8u32f_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f val))

IPPAPI(IppStatus, ippiTiltedIntegral_8u32f_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f val))

IPPAPI(IppStatus, ippiSqrIntegral_8u32f64f_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp32f* pDst, int dstStep, Ipp64f* pSqr, int sqrStep, IppiSize roiSize,
                  Ipp32f val, Ipp64f valSqr))

IPPAPI(IppStatus, ippiTiltedSqrIntegral_8u32f64f_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp32f* pDst, int dstStep, Ipp64f* pSqr, int sqrStep, IppiSize roiSize,
                  Ipp32f val, Ipp64f valSqr))


/****************************************************************************************\
*                                 Image Mean and Variance                                *
\****************************************************************************************/


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiMean_8u_C1MR,  ippiMean_8s_C1MR,  ippiMean_16u_C1MR,  ippiMean_32f_C1MR,
//        ippiMean_8u_C3CMR, ippiMean_8s_C3CMR, ippiMean_16u_C3CMR, ippiMean_32f_C3CMR
//
//  Purpose:  Find mean value for selected region
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsCOIErr             COI index is illegal (coi<1 || coi>3)
//
//  Parameters:
//    pSrc                     Pointer to image
//    srcStep                  Image step
//    pMask                    Pointer to mask image
//    maskStep                 Step in the mask image
//    roiSize                  Size of image ROI
//    coi                      Index of color channel (1..3) (if color image)
//    pMean                    Returned mean value
//
//  Notes:
//F*/

IPPAPI( IppStatus, ippiMean_8u_C1MR, ( const Ipp8u* pSrc, int srcStep,
                                       const Ipp8u* pMask, int maskStep,
                                       IppiSize roiSize, Ipp64f* pMean ))

IPPAPI( IppStatus, ippiMean_8u_C3CMR, ( const Ipp8u* pSrc, int srcStep,
                                        const Ipp8u* pMask, int maskStep,
                                        IppiSize roiSize, int coi, Ipp64f* pMean ))

IPPAPI( IppStatus, ippiMean_8s_C1MR, ( const Ipp8s* pSrc, int srcStep,
                                       const Ipp8u* pMask, int maskStep,
                                       IppiSize roiSize, Ipp64f* pMean ))

IPPAPI( IppStatus, ippiMean_8s_C3CMR, ( const Ipp8s* pSrc, int srcStep,
                                        const Ipp8u* pMask, int maskStep,
                                        IppiSize roiSize, int coi, Ipp64f* pMean ))

IPPAPI( IppStatus, ippiMean_16u_C1MR, ( const Ipp16u* pSrc, int srcStep,
                                        const Ipp8u* pMask, int maskStep,
                                        IppiSize roiSize, Ipp64f* pMean ))

IPPAPI( IppStatus, ippiMean_16u_C3CMR, ( const Ipp16u* pSrc, int srcStep,
                                         const Ipp8u* pMask, int maskStep,
                                         IppiSize roiSize, int coi, Ipp64f* pMean ))

IPPAPI( IppStatus, ippiMean_32f_C1MR, ( const Ipp32f* pSrc, int srcStep,
                                        const Ipp8u* pMask, int maskStep,
                                        IppiSize roiSize, Ipp64f* pMean ))

IPPAPI( IppStatus, ippiMean_32f_C3CMR, ( const Ipp32f* pSrc, int srcStep,
                                         const Ipp8u* pMask, int maskStep,
                                         IppiSize roiSize, int coi, Ipp64f* pMean ))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiMean_StdDev_8u_C1R,   ippiMean_StdDev_8s_C1R,
//        ippiMean_StdDev_16u_C1R,  ippiMean_StdDev_32f_C1R,
//        ippiMean_StdDev_8u_C3CR,  ippiMean_StdDev_8s_C3CR,
//        ippiMean_StdDev_16u_C3CR, ippiMean_StdDev_32f_C3CR
//
//  Purpose:  Find mean and standard deviation values for selected region
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsCOIErr             COI index is illegal (coi<1 || coi>3)
//
//  Parameters:
//    pSrc                     Pointer to image
//    srcStep                  Image step
//    roiSize                  Size of image ROI
//    coi                      Index of color channel (1..3) (if color image)
//    pMean                    Returned mean value
//    pStdDev                  Returned standard deviation
//
//  Notes:
//F*/

IPPAPI( IppStatus, ippiMean_StdDev_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
                                             IppiSize roiSize,
                                             Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_8u_C3CR, ( const Ipp8u* pSrc, int srcStep,
                                              IppiSize roiSize, int coi,
                                              Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_8s_C1R, ( const Ipp8s* pSrc, int srcStep,
                                             IppiSize roiSize,
                                             Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_8s_C3CR, ( const Ipp8s* pSrc, int srcStep,
                                              IppiSize roiSize, int coi,
                                              Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_16u_C1R, ( const Ipp16u* pSrc, int srcStep,
                                              IppiSize roiSize,
                                              Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_16u_C3CR, ( const Ipp16u* pSrc, int srcStep,
                                               IppiSize roiSize, int coi,
                                               Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
                                              IppiSize roiSize,
                                              Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_32f_C3CR, ( const Ipp32f* pSrc, int srcStep,
                                               IppiSize roiSize, int coi,
                                               Ipp64f* pMean, Ipp64f* pStdDev ))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiMean_StdDev_8u_C1MR,   ippiMean_StdDev_8s_C1MR,
//        ippiMean_StdDev_16u_C1MR,  ippiMean_StdDev_32f_C1MR,
//        ippiMean_StdDev_8u_C3CMR,  ippiMean_StdDev_8s_C3CMR,
//        ippiMean_StdDev_16u_C3CMR, ippiMean_StdDev_32f_C3CMR
//
//  Purpose:  Find mean and standard deviation values for selected region
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     Pointer to image
//    srcStep                  Image step
//    pMask                    Pointer to mask image
//    maskStep                 Step in the mask image
//    roiSize                  Size of image ROI
//    coi                      Index of color channel (1..3) (if color image)
//    pMean                    Returned mean value
//    pStdDev                  Returned standard deviation
//
//  Notes:
//F*/

IPPAPI( IppStatus, ippiMean_StdDev_8u_C1MR, ( const Ipp8u* pSrc, int srcStep,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,
                                              Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_8u_C3CMR, ( const Ipp8u* pSrc, int srcStep,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi,
                                               Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_8s_C1MR, ( const Ipp8s* pSrc, int srcStep,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,
                                              Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_8s_C3CMR, ( const Ipp8s* pSrc, int srcStep,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi,
                                               Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_16u_C1MR, ( const Ipp16u* pSrc, int srcStep,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,
                                              Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_16u_C3CMR, ( const Ipp16u* pSrc, int srcStep,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi,
                                               Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_32f_C1MR, ( const Ipp32f* pSrc, int srcStep,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize,
                                               Ipp64f* pMean, Ipp64f* pStdDev ))

IPPAPI( IppStatus, ippiMean_StdDev_32f_C3CMR, ( const Ipp32f* pSrc, int srcStep,
                                                const Ipp8u* pMask, int maskStep,
                                                IppiSize roiSize, int coi,
                                                Ipp64f* pMean, Ipp64f* pStdDev ))


/****************************************************************************************\
*                                   Variance on Window                                   *
\****************************************************************************************/


/*F/////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiRectStdDev_32s32f_C1R,           ippiRectStdDev_32f_C1R
//        ippiTiltedRectStdDev_32s32f_C1R,     ippiTiltedRectStdDev_32f_C1R
//        ippiRectStdDev_32s_C1RSfs,           ippiTiltedRectStdDev_32s_C1RSfs
//
//  Purpose:   Calculates standard deviation on rectangular window
//
//  Returns:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     The pointer to source image of integrals
//    srcStep                  The step in source image
//    pSqr                     The pointer to destination square integral image
//    sqrStep                  The step in destination image
//    pDst                     The pointer to destination image
//    dstStep                  The step in destination image
//    roiSize                  The destination image ROI size.
//    rect                     The rectangular window for standard deviation calculation.
//    scaleFactor              Output scale factor
//
//  Notes:
//F*/

IPPAPI(IppStatus, ippiRectStdDev_32s_C1RSfs,  (const Ipp32s* pSrc, int srcStep,
                         const Ipp32s* pSqr, int sqrStep, Ipp32s* pDst, int dstStep, IppiSize roi,
                         IppiRect rect, int scaleFactor))

IPPAPI(IppStatus, ippiRectStdDev_32s32f_C1R,  (const Ipp32s* pSrc, int srcStep,
                         const Ipp64f* pSqr, int sqrStep, Ipp32f* pDst, int dstStep,
                         IppiSize roiSize, IppiRect rect))

IPPAPI(IppStatus, ippiTiltedRectStdDev_32s32f_C1R,  (const Ipp32s* pSrc, int srcStep,
                         const Ipp64f* pSqr, int sqrStep, Ipp32f* pDst, int dstStep,
                         IppiSize roiSize, IppiRect rect))


IPPAPI(IppStatus, ippiTiltedRectStdDev_32s_C1RSfs,  (const Ipp32s* pSrc, int srcStep,
                         const Ipp32s* pSqr, int sqrStep, Ipp32s* pDst, int dstStep, IppiSize roi,
                         IppiRect rect, int scaleFactor))

IPPAPI(IppStatus, ippiRectStdDev_32f_C1R,  (const Ipp32f* pSrc, int srcStep,
                         const Ipp64f* pSqr, int sqrStep, Ipp32f* pDst, int dstStep,
                         IppiSize roiSize, IppiRect rect))

IPPAPI(IppStatus, ippiTiltedRectStdDev_32f_C1R,  (const Ipp32f* pSrc, int srcStep,
                         const Ipp64f* pSqr, int sqrStep, Ipp32f* pDst, int dstStep,
                         IppiSize roiSize, IppiRect rect))


/****************************************************************************************\
*                                   Image Minimum and Maximum                            *
\****************************************************************************************/


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiMinMaxIndx_8u_C1R,   ippiMinMaxIndx_8s_C1R,
//        ippiMinMaxIndx_16u_C1R,  ippiMinMaxIndx_32f_C1R,
//        ippiMinMaxIndx_8u_C3CR,  ippiMinMaxIndx_8s_C3CR,
//        ippiMinMaxIndx_16u_C3CR, ippiMinMaxIndx_32f_C3CR,
//
//  Purpose:  Finds minimum and maximum values in the image and their coordinates
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     Pointer to image
//    srcStep                  Image step
//    roiSize                  Size of image ROI
//    coi                      Index of color channel (1..3) (if color image)
//    pMinVal                  Pointer to minimum value
//    pMaxVal                  Pointer to maximum value
//    pMinIndex                Minimum's coordinates
//    pMaxIndex                Maximum's coordinates
//
//  Notes:
//    Any of output parameters is optional
//F*/

IPPAPI(IppStatus, ippiMinMaxIndx_8u_C1R,( const Ipp8u* pSrc, int srcStep, IppiSize roiSize,
                                          Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                          IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI(IppStatus, ippiMinMaxIndx_8u_C3CR,( const Ipp8u* pSrc, int srcStep,
                                           IppiSize roiSize, int coi,
                                           Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                           IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI(IppStatus, ippiMinMaxIndx_8s_C1R,( const Ipp8s* pSrc, int step, IppiSize roiSize,
                                          Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                          IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI(IppStatus, ippiMinMaxIndx_8s_C3CR,( const Ipp8s* pSrc, int step, IppiSize roiSize,
                                           int coi, Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                           IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI(IppStatus, ippiMinMaxIndx_16u_C1R,( const Ipp16u* pSrc, int srcStep, IppiSize roiSize,
                                           Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                           IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI(IppStatus, ippiMinMaxIndx_16u_C3CR,( const Ipp16u* pSrc, int srcStep,
                                            IppiSize roiSize, int coi,
                                            Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                            IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI(IppStatus, ippiMinMaxIndx_32f_C1R,( const Ipp32f* pSrc, int step, IppiSize roiSize,
                                           Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                           IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI(IppStatus, ippiMinMaxIndx_32f_C3CR,( const Ipp32f* pSrc, int step, IppiSize roiSize,
                                            int coi, Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                            IppiPoint* pMinIndex, IppiPoint* pMaxIndex))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiMinMaxIndx_8u_C1MR,   ippiMinMaxIndx_8s_C1MR,
//        ippiMinMaxIndx_16u_C1MR,  ippiMinMaxIndx_32f_C1MR,
//        ippiMinMaxIndx_8u_C3CMR,  ippiMinMaxIndx_8s_C3CMR,
//        ippiMinMaxIndx_16u_C3CMR, ippiMinMaxIndx_32f_C3CMR,
//
//  Purpose:  Finds minimum and maximum values in the image and their coordinates
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     Pointer to image
//    srcStep                  Image step
//    pMask                    Pointer to mask image
//    maskStep                 Step in the mask image
//    roiSize                  Size of image ROI
//    coi                      Index of color channel (1..3) (if color image)
//    pMinVal                  Pointer to minimum value
//    pMaxVal                  Pointer to maximum value
//    pMinIndex                Minimum's coordinates
//    pMaxIndex                Maximum's coordinates
//
//  Notes:
//    Any of output parameters is optional
//F*/

IPPAPI(IppStatus, ippiMinMaxIndx_8u_C1MR,( const Ipp8u* pSrc, int srcStep,
                                           const Ipp8u* pMask, int maskStep,
                                           IppiSize roiSize,
                                           Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                           IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI( IppStatus, ippiMinMaxIndx_8u_C3CMR, ( const Ipp8u* pSrc, int srcStep,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize, int coi,
                                              Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                              IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI( IppStatus, ippiMinMaxIndx_8s_C1MR, ( const Ipp8s* pSrc, int srcStep,
                                             const Ipp8u* pMask, int maskStep,
                                             IppiSize roiSize,
                                             Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                             IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI( IppStatus, ippiMinMaxIndx_8s_C3CMR, ( const Ipp8s* pSrc, int srcStep,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize, int coi,
                                              Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                              IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI(IppStatus, ippiMinMaxIndx_16u_C1MR,( const Ipp16u* pSrc, int srcStep,
                                            const Ipp8u* pMask, int maskStep,
                                            IppiSize roiSize,
                                            Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                            IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI( IppStatus, ippiMinMaxIndx_16u_C3CMR, ( const Ipp16u* pSrc, int srcStep,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi,
                                               Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                               IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI( IppStatus, ippiMinMaxIndx_32f_C1MR, ( const Ipp32f* pSrc, int srcStep,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,
                                              Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                              IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

IPPAPI( IppStatus, ippiMinMaxIndx_32f_C3CMR, ( const Ipp32f* pSrc, int srcStep,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi,
                                               Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                               IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))


/****************************************************************************************\
*                                     Image Norms                                        *
\****************************************************************************************/


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Names: ippiNorm_Inf_8u_C1MR,       ippiNorm_Inf_8s_C1MR,
//         ippiNorm_Inf_16u_C1MR,      ippiNorm_Inf_32f_C1MR,
//         ippiNorm_Inf_8u_C3CMR,      ippiNorm_Inf_8s_C3CMR,
//         ippiNorm_Inf_16u_C3CMR,     ippiNorm_Inf_32f_C3CMR,
//         ippiNormDiff_Inf_8u_C1MR,   ippiNormDiff_Inf_8s_C1MR,
//         ippiNormDiff_Inf_16u_C1MR,  ippiNormDiff_Inf_32f_C1MR,
//         ippiNormDiff_Inf_8u_C3CMR,  ippiNormDiff_Inf_8s_C3CMR,
//         ippiNormDiff_Inf_16u_C3CMR, ippiNormDiff_Inf_32f_C3CMR,
//         ippiNormRel_Inf_8u_C1MR,    ippiNormRel_Inf_8s_C1MR,
//         ippiNormRel_Inf_16u_C1MR,   ippiNormRel_Inf_32f_C1MR,
//         ippiNormRel_Inf_8u_C3CMR,   ippiNormRel_Inf_8s_C3CMR,
//         ippiNormRel_Inf_16u_C3CMR,  ippiNormRel_Inf_32f_C3CMR,
//
//         ippiNorm_L1_8u_C1MR,        ippiNorm_L1_8s_C1MR,
//         ippiNorm_L1_16u_C1MR,       ippiNorm_L1_32f_C1MR,
//         ippiNorm_L1_8u_C3CMR,       ippiNorm_L1_8s_C3CMR,
//         ippiNorm_L1_16u_C3CMR,      ippiNorm_L1_32f_C3CMR,
//         ippiNormDiff_L1_8u_C1MR,    ippiNormDiff_L1_8s_C1MR,
//         ippiNormDiff_L1_16u_C1MR,   ippiNormDiff_L1_32f_C1MR,
//         ippiNormDiff_L1_8u_C3CMR,   ippiNormDiff_L1_8s_C3CMR,
//         ippiNormDiff_L1_16u_C3CMR,  ippiNormDiff_L1_32f_C3CMR,
//         ippiNormRel_L1_8u_C1MR,     ippiNormRel_L1_8s_C1MR,
//         ippiNormRel_L1_16u_C1MR,    ippiNormRel_L1_32f_C1MR,
//         ippiNormRel_L1_8u_C3CMR,    ippiNormRel_L1_8s_C3CMR,
//         ippiNormRel_L1_16u_C3CMR,   ippiNormRel_L1_32f_C3CMR,
//
//         ippiNorm_L2_8u_C1MR,        ippiNorm_L2_8s_C1MR,
//         ippiNorm_L2_16u_C1MR,       ippiNorm_L2_32f_C1MR,
//         ippiNorm_L2_8u_C3CMR,       ippiNorm_L2_8s_C3CMR,
//         ippiNorm_L2_16u_C3CMR,      ippiNorm_L2_32f_C3CMR,
//         ippiNormDiff_L2_8u_C1MR,    ippiNormDiff_L2_8s_C1MR,
//         ippiNormDiff_L2_16u_C1MR,   ippiNormDiff_L2_32f_C1MR,
//         ippiNormDiff_L2_8u_C3CMR,   ippiNormDiff_L2_8s_C3CMR,
//         ippiNormDiff_L2_16u_C3CMR,  ippiNormDiff_L2_32f_C3CMR,
//         ippiNormRel_L2_8u_C1MR,     ippiNormRel_L2_8s_C1MR,
//         ippiNormRel_L2_16u_C1MR,    ippiNormRel_L2_32f_C1MR,
//         ippiNormRel_L2_8u_C3CMR,    ippiNormRel_L2_8s_C3CMR,
//         ippiNormRel_L2_16u_C3CMR,   ippiNormRel_L2_32f_C3CMR
//
//  Purpose: Calculates ordinary, differential or relative norms of one or two images
//           in an arbitrary image region.
//
//  Returns:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc, pSrc1              Pointers to source and mask images
//    pSrc2, pMask
//    srcStep, src1Step        Their steps
//    src2Step, maskStep
//    roiSize                  Their size or ROI size
//    coi                      COI index (1..3) (if 3-channel images)
//    pNorm                    The pointer to calculated norm
//
//  Notes:
//F*/

/* ///////////////////////////////// 8uC1 flavor ////////////////////////////////////// */

IPPAPI( IppStatus, ippiNorm_Inf_8u_C1MR, ( const Ipp8u* pSrc, int srcStep,
                                           const Ipp8u* pMask,int maskStep,
                                           IppiSize roiSize, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_Inf_8s_C1MR, ( const Ipp8s* pSrc, int srcStep,
                                           const Ipp8u* pMask,int maskStep,
                                           IppiSize roiSize, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_Inf_16u_C1MR, ( const Ipp16u* pSrc, int srcStep,
                                            const Ipp8u* pMask,int maskStep,
                                            IppiSize roiSize, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_Inf_32f_C1MR, ( const Ipp32f* pSrc, int srcStep,
                                            const Ipp8u* pMask, int maskStep,
                                            IppiSize roiSize, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_Inf_8u_C3CMR, ( const Ipp8u* pSrc, int srcStep,
                                            const Ipp8u* pMask,int maskStep,
                                            IppiSize roiSize, int coi, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_Inf_8s_C3CMR, ( const Ipp8s* pSrc, int srcStep,
                                            const Ipp8u* pMask,int maskStep,
                                            IppiSize roiSize, int coi, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_Inf_16u_C3CMR, ( const Ipp16u* pSrc, int srcStep,
                                             const Ipp8u* pMask,int maskStep,
                                             IppiSize roiSize, int coi, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_Inf_32f_C3CMR, ( const Ipp32f* pSrc, int srcStep,
                                             const Ipp8u* pMask, int maskStep,
                                             IppiSize roiSize, int coi, Ipp64f* pNorm ) )


IPPAPI( IppStatus, ippiNormDiff_Inf_8u_C1MR, ( const Ipp8u* pSrc1, int src1Step,
                                               const Ipp8u* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormDiff_Inf_8s_C1MR, ( const Ipp8s* pSrc1, int src1Step,
                                               const Ipp8s* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormDiff_Inf_16u_C1MR, ( const Ipp16u* pSrc1, int src1Step,
                                                const Ipp16u* pSrc2, int src2Step,
                                                const Ipp8u* pMask, int maskStep,
                                                IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormDiff_Inf_32f_C1MR, ( const Ipp32f* pSrc1, int src1Step,
                                                const Ipp32f* pSrc2, int src2Step,
                                                const Ipp8u* pMask,  int maskStep,
                                                IppiSize roiSize,    Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormDiff_Inf_8u_C3CMR, (const Ipp8u* pSrc1, int src1Step,
                                               const Ipp8u* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormDiff_Inf_8s_C3CMR, (const Ipp8s* pSrc1, int src1Step,
                                               const Ipp8s* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormDiff_Inf_16u_C3CMR, (const Ipp16u* pSrc1, int src1Step,
                                                const Ipp16u* pSrc2, int src2Step,
                                                const Ipp8u* pMask, int maskStep,
                                                IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormDiff_Inf_32f_C3CMR, (const Ipp32f* pSrc1, int src1Step,
                                                const Ipp32f* pSrc2, int src2Step,
                                                const Ipp8u* pMask,  int maskStep,
                                                IppiSize roiSize, int coi, Ipp64f* pNorm ))


IPPAPI( IppStatus, ippiNormRel_Inf_8u_C1MR, ( const Ipp8u* pSrc1, int src1Step,
                                              const Ipp8u* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormRel_Inf_8s_C1MR, ( const Ipp8s* pSrc1, int src1Step,
                                              const Ipp8s* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormRel_Inf_16u_C1MR, ( const Ipp16u* pSrc1, int src1Step,
                                               const Ipp16u* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormRel_Inf_32f_C1MR, ( const Ipp32f* pSrc1, int src1Step,
                                               const Ipp32f* pSrc2, int src2Step,
                                               const Ipp8u* pMask,  int maskStep,
                                               IppiSize roiSize,    Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormRel_Inf_8u_C3CMR, ( const Ipp8u* pSrc1, int src1Step,
                                               const Ipp8u* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormRel_Inf_8s_C3CMR, ( const Ipp8s* pSrc1, int src1Step,
                                               const Ipp8s* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormRel_Inf_16u_C3CMR, ( const Ipp16u* pSrc1, int src1Step,
                                                const Ipp16u* pSrc2, int src2Step,
                                                const Ipp8u* pMask, int maskStep,
                                                IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormRel_Inf_32f_C3CMR, ( const Ipp32f* pSrc1, int src1Step,
                                                const Ipp32f* pSrc2, int src2Step,
                                                const Ipp8u* pMask,  int maskStep,
                                                IppiSize roiSize, int coi, Ipp64f* pNorm ))


IPPAPI( IppStatus, ippiNorm_L1_8u_C1MR, ( const Ipp8u* pSrc, int srcStep,
                                          const Ipp8u* pMask,int maskStep,
                                          IppiSize roiSize, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L1_8s_C1MR, ( const Ipp8s* pSrc, int srcStep,
                                          const Ipp8u* pMask,int maskStep,
                                          IppiSize roiSize, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L1_16u_C1MR, ( const Ipp16u* pSrc, int srcStep,
                                           const Ipp8u* pMask,int maskStep,
                                           IppiSize roiSize, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L1_32f_C1MR, ( const Ipp32f* pSrc, int srcStep,
                                           const Ipp8u* pMask, int maskStep,
                                           IppiSize roiSize, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L1_8u_C3CMR, ( const Ipp8u* pSrc, int srcStep,
                                           const Ipp8u* pMask,int maskStep,
                                           IppiSize roiSize, int coi, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L1_8s_C3CMR, ( const Ipp8s* pSrc, int srcStep,
                                           const Ipp8u* pMask,int maskStep,
                                           IppiSize roiSize, int coi, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L1_16u_C3CMR, ( const Ipp16u* pSrc, int srcStep,
                                            const Ipp8u* pMask,int maskStep,
                                            IppiSize roiSize, int coi, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L1_32f_C3CMR, ( const Ipp32f* pSrc, int srcStep,
                                            const Ipp8u* pMask, int maskStep,
                                            IppiSize roiSize, int coi, Ipp64f* pNorm ) )


IPPAPI( IppStatus, ippiNormDiff_L1_8u_C1MR, ( const Ipp8u* pSrc1, int src1Step,
                                              const Ipp8u* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormDiff_L1_8s_C1MR, ( const Ipp8s* pSrc1, int src1Step,
                                              const Ipp8s* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormDiff_L1_16u_C1MR, ( const Ipp16u* pSrc1, int src1Step,
                                               const Ipp16u* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormDiff_L1_32f_C1MR, ( const Ipp32f* pSrc1, int src1Step,
                                               const Ipp32f* pSrc2, int src2Step,
                                               const Ipp8u* pMask,  int maskStep,
                                               IppiSize roiSize,    Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormDiff_L1_8u_C3CMR, ( const Ipp8u* pSrc1, int src1Step,
                                               const Ipp8u* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormDiff_L1_8s_C3CMR, ( const Ipp8s* pSrc1, int src1Step,
                                               const Ipp8s* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormDiff_L1_16u_C3CMR, ( const Ipp16u* pSrc1, int src1Step,
                                                const Ipp16u* pSrc2, int src2Step,
                                                const Ipp8u* pMask, int maskStep,
                                                IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormDiff_L1_32f_C3CMR, ( const Ipp32f* pSrc1, int src1Step,
                                                const Ipp32f* pSrc2, int src2Step,
                                                const Ipp8u* pMask,  int maskStep,
                                                IppiSize roiSize, int coi, Ipp64f* pNorm ))


IPPAPI( IppStatus, ippiNormRel_L1_8u_C1MR, ( const Ipp8u* pSrc1, int src1Step,
                                             const Ipp8u* pSrc2, int src2Step,
                                             const Ipp8u* pMask, int maskStep,
                                             IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormRel_L1_8s_C1MR, ( const Ipp8s* pSrc1, int src1Step,
                                             const Ipp8s* pSrc2, int src2Step,
                                             const Ipp8u* pMask, int maskStep,
                                             IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormRel_L1_16u_C1MR, ( const Ipp16u* pSrc1, int src1Step,
                                              const Ipp16u* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormRel_L1_32f_C1MR, ( const Ipp32f* pSrc1, int src1Step,
                                              const Ipp32f* pSrc2, int src2Step,
                                              const Ipp8u* pMask,  int maskStep,
                                              IppiSize roiSize,    Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormRel_L1_8u_C3CMR, ( const Ipp8u* pSrc1, int src1Step,
                                              const Ipp8u* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormRel_L1_8s_C3CMR, ( const Ipp8s* pSrc1, int src1Step,
                                              const Ipp8s* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormRel_L1_16u_C3CMR, ( const Ipp16u* pSrc1, int src1Step,
                                               const Ipp16u* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormRel_L1_32f_C3CMR, ( const Ipp32f* pSrc1, int src1Step,
                                               const Ipp32f* pSrc2, int src2Step,
                                               const Ipp8u* pMask,  int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))


IPPAPI( IppStatus, ippiNorm_L2_8u_C1MR, ( const Ipp8u* pSrc, int srcStep,
                                          const Ipp8u* pMask,int maskStep,
                                          IppiSize roiSize, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L2_8s_C1MR, ( const Ipp8s* pSrc, int srcStep,
                                          const Ipp8u* pMask,int maskStep,
                                          IppiSize roiSize, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L2_16u_C1MR, ( const Ipp16u* pSrc, int srcStep,
                                           const Ipp8u* pMask,int maskStep,
                                           IppiSize roiSize, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L2_32f_C1MR, ( const Ipp32f* pSrc, int srcStep,
                                           const Ipp8u* pMask, int maskStep,
                                           IppiSize roiSize, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L2_8u_C3CMR, ( const Ipp8u* pSrc, int srcStep,
                                           const Ipp8u* pMask,int maskStep,
                                           IppiSize roiSize, int coi, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L2_8s_C3CMR, ( const Ipp8s* pSrc, int srcStep,
                                           const Ipp8u* pMask,int maskStep,
                                           IppiSize roiSize, int coi, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L2_16u_C3CMR, ( const Ipp16u* pSrc, int srcStep,
                                            const Ipp8u* pMask,int maskStep,
                                            IppiSize roiSize, int coi, Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNorm_L2_32f_C3CMR, ( const Ipp32f* pSrc, int srcStep,
                                            const Ipp8u* pMask, int maskStep,
                                            IppiSize roiSize, int coi, Ipp64f* pNorm ) )


IPPAPI( IppStatus, ippiNormDiff_L2_8u_C1MR, ( const Ipp8u* pSrc1, int src1Step,
                                              const Ipp8u* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormDiff_L2_8s_C1MR, ( const Ipp8s* pSrc1, int src1Step,
                                              const Ipp8s* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormDiff_L2_16u_C1MR, ( const Ipp16u* pSrc1, int src1Step,
                                               const Ipp16u* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormDiff_L2_32f_C1MR, ( const Ipp32f* pSrc1, int src1Step,
                                               const Ipp32f* pSrc2, int src2Step,
                                               const Ipp8u* pMask,  int maskStep,
                                               IppiSize roiSize,    Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormDiff_L2_8u_C3CMR, ( const Ipp8u* pSrc1, int src1Step,
                                               const Ipp8u* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormDiff_L2_8s_C3CMR, ( const Ipp8s* pSrc1, int src1Step,
                                               const Ipp8s* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormDiff_L2_16u_C3CMR, ( const Ipp16u* pSrc1, int src1Step,
                                                const Ipp16u* pSrc2, int src2Step,
                                                const Ipp8u* pMask, int maskStep,
                                                IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormDiff_L2_32f_C3CMR, ( const Ipp32f* pSrc1, int src1Step,
                                                const Ipp32f* pSrc2, int src2Step,
                                                const Ipp8u* pMask,  int maskStep,
                                                IppiSize roiSize, int coi, Ipp64f* pNorm ))


IPPAPI( IppStatus, ippiNormRel_L2_8u_C1MR, ( const Ipp8u* pSrc1, int src1Step,
                                             const Ipp8u* pSrc2, int src2Step,
                                             const Ipp8u* pMask, int maskStep,
                                             IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormRel_L2_8s_C1MR, ( const Ipp8s* pSrc1, int src1Step,
                                             const Ipp8s* pSrc2, int src2Step,
                                             const Ipp8u* pMask, int maskStep,
                                             IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormRel_L2_16u_C1MR, ( const Ipp16u* pSrc1, int src1Step,
                                              const Ipp16u* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,   Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormRel_L2_32f_C1MR, ( const Ipp32f* pSrc1, int src1Step,
                                              const Ipp32f* pSrc2, int src2Step,
                                              const Ipp8u* pMask,  int maskStep,
                                              IppiSize roiSize,    Ipp64f* pNorm ) )

IPPAPI( IppStatus, ippiNormRel_L2_8u_C3CMR, ( const Ipp8u* pSrc1, int src1Step,
                                              const Ipp8u* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormRel_L2_8s_C3CMR, ( const Ipp8s* pSrc1, int src1Step,
                                              const Ipp8s* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormRel_L2_16u_C3CMR, ( const Ipp16u* pSrc1, int src1Step,
                                               const Ipp16u* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

IPPAPI( IppStatus, ippiNormRel_L2_32f_C3CMR, ( const Ipp32f* pSrc1, int src1Step,
                                               const Ipp32f* pSrc2, int src2Step,
                                               const Ipp8u* pMask,  int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))


/****************************************************************************************\
*                                   Edge/Corner detection                                *
\****************************************************************************************/


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiCannyGetSize
//
//  Purpose: Calculates size of temporary buffer, required to run Canny function.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Pointer bufferSize is NULL
//    ippStsSizeErr            roiSize has a field with zero or negative value
//
//  Parameters:
//    roiSize                  Size of image ROI in pixel
//    bufferSize               Pointer to the variable that returns the size of the temporary buffer
//F*/

IPPAPI( IppStatus, ippiCannyGetSize, ( IppiSize roiSize, int* bufferSize ))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiCanny_16s8u_C1IR,     ippiCanny_32f8u_C1IR
//
//  Purpose: Creates binary image of source's image edges,
//                using derivatives of the first order.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsBadArgErr          Bad thresholds
//
//  Parameters:
//    pSrcDx                   Pointers to the source image ( first derivatives  with respect to X )
//    srcDxStep                Step in bytes through the source image pSrcDx
//    pSrcDy                   Pointers to the source image ( first derivatives  with respect to Y )
//    srcDyStep                Step in bytes through the source image pSrcDy
//
//    roiSize                  Size of the source images ROI in pixels
//    lowThresh                Low threshold for edges detection
//    highThresh               Upper threshold for edges detection
//    pBuffer                  Pointer to the pre-allocated temporary buffer, which size can be
//                             calculated using ippiCannyGetSize function
//F*/

IPPAPI(IppStatus, ippiCanny_16s8u_C1R, ( Ipp16s* pSrcDx, int srcDxStep,
                                         Ipp16s* pSrcDy, int srcDyStep,
                                         Ipp8u*  pDstEdges, int dstEdgeStep,
                                         IppiSize roiSize,
                                         Ipp32f  lowThresh,
                                         Ipp32f  highThresh,
                                         Ipp8u*  pBuffer ))

IPPAPI(IppStatus, ippiCanny_32f8u_C1R, ( Ipp32f* pSrcDx, int srcDxStep,
                                         Ipp32f* pSrcDy, int srcDyStep,
                                         Ipp8u*  pDstEdges, int dstEdgeStep,
                                         IppiSize roiSize,
                                         Ipp32f  lowThresh,
                                         Ipp32f  highThresh,
                                         Ipp8u*  pBuffer ))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiEigenValsVecsGetBufferSize_8u32f_C1R, ippiEigenValsVecsGetBufferSize_32f_C1R
//
//  Purpose: Calculates size of temporary buffer, required to run one of EigenValsVecs***
//           functions.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width is less or equal zero or bad window size
//
//  Parameters:
//    roiSize                  roiSize size in pixels
//    apertureSize             Linear size of derivative filter aperture
//    avgWindow                Linear size of averaging window
//    bufferSize               Output parameter. Calculated buffer size.
//F*/

IPPAPI(IppStatus, ippiEigenValsVecsGetBufferSize_8u32f_C1R, ( IppiSize roiSize,int apertureSize,
                                              int avgWindow, int* bufferSize ))
IPPAPI(IppStatus, ippiEigenValsVecsGetBufferSize_32f_C1R, ( IppiSize roiSize,int apertureSize,
                                              int avgWindow, int* bufferSize ))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiEigenValsVecs_8u32f_C1R, ippiEigenValsVecs_32f_C1R
//
//  Purpose: Calculate both eigen values and eigen vectors of 2x2 autocorrelation
//           gradient matrix for every pixel. Can be used for sophisticated
//           edge and corner detection
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//                             or bad window size
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     Source image
//    srcStep                  Its step
//    pEigenVV                 Image, which is 6 times wider that source image,
//                             filled with 6-tuples:
//                             (eig_val1, eig_val2, eig_vec1_x, eig_vec1_y,
//                             eig_vec2_x, eig_vec2_y)
//    eigStep                  Output image step
//    roiSize                  ROI size
//    kernType                 Kernel type (Scharr 3x3 or Sobel 3x3, 5x5)
//    apertureSize             Linear size of derivative filter aperture
//    avgWindow                Linear size of averaging window
//    pBuffer                  Preallocated temporary buffer, which size can be calculated
//                             using ippiEigenValsVecsGetSize function
//F*/

IPPAPI(IppStatus, ippiEigenValsVecs_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
                                                 Ipp32f* pEigenVV, int eigStep,
                                                 IppiSize roiSize, IppiKernelType kernType,
                                                 int apertureSize, int avgWindow, Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiEigenValsVecs_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
                                               Ipp32f* pEigenVV, int eigStep,
                                               IppiSize roiSize, IppiKernelType kernType,
                                               int apertureSize, int avgWindow, Ipp8u* pBuffer ))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiMinEigenValGetBufferSize_8u32f_C1R, ippiMinEigenValGetBufferSize_32f_C1R
//
//  Purpose: Calculates size of temporary buffer, required to run one of MinEigenVal***
//           functions.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width is less or equal zero or bad window size
//
//  Parameters:
//    roiSize                  roiSize size in pixels
//    apertureSize             Linear size of derivative filter aperture
//    avgWindow                Linear size of averaging window
//    bufferSize               Output parameter. Calculated buffer size.
//F*/

IPPAPI(IppStatus, ippiMinEigenValGetBufferSize_8u32f_C1R, ( IppiSize roiSize, int apertureSize,
                                            int avgWindow, int* bufferSize ))

IPPAPI(IppStatus, ippiMinEigenValGetBufferSize_32f_C1R, ( IppiSize roiSize, int apertureSize,
                                            int avgWindow, int* bufferSize ))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiMinEigenVal_8u32f_C1R, ippiMinEigenVal_32f_C1R
//
//  Purpose: Calculate minimal eigen value of 2x2 autocorrelation gradient matrix
//           for every pixel. Pixels with relatively large minimal eigen values
//           are strong corners on the picture.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//                             or bad window size
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     Source image
//    srcStep                  Its step
//    pMinEigenVal             Image, filled with minimal eigen values for every pixel
//    minValStep               Its step
//    roiSize                  ROI size
//    kernType                 Kernel type (Scharr 3x3 or Sobel 3x3, 5x5)
//    apertureSize             Linear size of derivative filter aperture
//    avgWindow                Linear size of averaging window
//    pBuffer                  Preallocated temporary buffer, which size can be calculated
//                             using ippiMinEigenValGetSize function
//F*/

IPPAPI(IppStatus, ippiMinEigenVal_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
                                               Ipp32f* pMinEigenVal, int minValStep,
                                               IppiSize roiSize, IppiKernelType kernType,
                                               int apertureSize, int avgWindow, Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiMinEigenVal_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
                                             Ipp32f* pMinEigenVal, int minValStep,
                                             IppiSize roiSize, IppiKernelType kernType,
                                             int apertureSize, int avgWindow, Ipp8u* pBuffer ))


/****************************************************************************************\
*                                   Distance Transform                                   *
\****************************************************************************************/

/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiTrueDistanceTransformGetBufferSize_8u16u_C1RSfs,
//         ippiTrueDistanceTransformGetBufferSize_8u32f_C1R
//
//  Purpose:    Get size of external buffer.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            Indicates an error condition if roiSize has a field with zero
//                             or negative value.
//
//  Parameters:
//    roiSize                  The image ROI size
//    pBufferSize              The pointer to the buffer size
*/

IPPAPI(IppStatus, ippiTrueDistanceTransformGetBufferSize_8u16u_C1RSfs, (IppiSize roiSize, int* pBufferSize))

IPPAPI(IppStatus, ippiTrueDistanceTransformGetBufferSize_8u32f_C1R, (IppiSize roiSize, int* pBufferSize))

/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiDistanceTransform_3x3_8u32f_C1R, ippiDistanceTransform_5x5_8u32f_C1R,
//           ippiDistanceTransform_3x3_8u16u_C1R, ippiDistanceTransform_5x5_8u16u_C1R,
//           ippiDistanceTransform_3x3_8u_C1R,    ippiDistanceTransform_5x5_8u_C1R,
//           ippiDistanceTransform_3x3_8u_C1IR,   ippiDistanceTransform_5x5_8u_C1IR,
//           ippiTrueDistanceTransform_8u16u_C1RSfs,
//           ippiTrueDistanceTransform_8u32f_C1R
//
//  Purpose: For every non-zero pixel in the source image, the functions calculate
//           distance between that pixel and nearest zero pixel.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsCoeffErr           Zero mask coefficient
//
//  Parameters:
//    pSrc                     Source image
//    pSrcDst                  Pointer to the input and output image
//    srcStep                  Its step
//    pDst                     Output image with distances
//    dstStep                  Its step
//    roiSize                  ROI size
//    pMetrics                 Array that determines metrics used.
//    scaleFactor              Scale factor
//    pBuffer                  The pointer to the working buffer
//
//F*/

IPPAPI(IppStatus, ippiDistanceTransform_3x3_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
                                                         Ipp32f* pDst, int dstStep,
                                                         IppiSize roiSize, Ipp32f* pMetrics ))

IPPAPI(IppStatus, ippiDistanceTransform_5x5_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
                                                         Ipp32f* pDst, int dstStep,
                                                         IppiSize roiSize, Ipp32f* pMetrics ))

IPPAPI(IppStatus, ippiDistanceTransform_3x3_8u16u_C1R, ( const Ipp8u* pSrc, int srcStep,
                                                         Ipp16u* pDst, int dstStep,
                                                         IppiSize roiSize, Ipp32s* pMetrics ))

IPPAPI(IppStatus, ippiDistanceTransform_5x5_8u16u_C1R, ( const Ipp8u* pSrc, int srcStep,
                                                         Ipp16u* pDst, int dstStep,
                                                         IppiSize roiSize, Ipp32s* pMetrics ))

IPPAPI(IppStatus, ippiDistanceTransform_3x3_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
                                                      Ipp8u* pDst, int dstStep,
                                                      IppiSize roiSize, Ipp32s* pMetrics ))

IPPAPI(IppStatus, ippiDistanceTransform_5x5_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
                                                      Ipp8u* pDst, int dstStep,
                                                      IppiSize roiSize, Ipp32s* pMetrics ))


IPPAPI(IppStatus, ippiDistanceTransform_3x3_8u_C1IR, ( Ipp8u* pSrcDst, int srcDstStep,
                                                       IppiSize roiSize, Ipp32s* pMetrics ))

IPPAPI(IppStatus, ippiDistanceTransform_5x5_8u_C1IR, ( Ipp8u* pSrcDst, int srcDstStep,
                                                       IppiSize roiSize, Ipp32s* pMetrics ))


IPPAPI(IppStatus, ippiTrueDistanceTransform_8u16u_C1RSfs, ( const Ipp8u* pSrc, int srcStep,
                                                         Ipp16u* pDst, int dstStep,
                                                         IppiSize roiSize, int scaleFactor,
                                                         Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiTrueDistanceTransform_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
                                                         Ipp32f* pDst, int dstStep,
                                                         IppiSize roiSize, Ipp8u* pBuffer ))

/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiGetDistanceTransformMask_32f,    ippiGetDistanceTransformMask_32s
//           ippiGetDistanceTransformMask (deprecated name of ippiGetDistanceTransformMask_32f)
//
//  Purpose: Calculates optimal mask for given type of metrics and given mask size
//
//  Return:
//    ippStsOk                 Succeed
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsBadArgErr          Bad kernel size or norm or maskType
//
//  Parameters:
//    kerSize                  Kernel size (3,5)
//    norm                     Norm type (L1,L2,Inf)
//    maskType                 Type of distance:
//                                30 - 3x3 aperture for infinify norm,
//                                31 - 3x3 aperture for L1 norm,
//                                32 - 3x3 aperture for L2 norm,
//                                50 - 5x5 aperture for infinify norm,
//                                51 - 5x5 aperture for L1 norm,
//                                52 - 5x5 aperture for L2 norm
//    pMetrics                 Pointer to resultant metrics
//F*/

IPPAPI( IppStatus, ippiGetDistanceTransformMask_32f, ( int kerSize, IppiNorm norm, Ipp32f* pMetrics ))

IPPAPI( IppStatus, ippiGetDistanceTransformMask_32s, ( int kerSize, IppiNorm norm, Ipp32s* pMetrics ))

IPPAPI( IppStatus, ippiGetDistanceTransformMask,     ( int maskType, Ipp32f* pMetrics ))


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiFastMarchingGetBufferSize_8u32f_C1R
//
//  Purpose:    Get size of external buffer.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image or kernel size are less or equal zero
//
//  Parameters:
//    roiSize                  The image ROI size
//    pBufferSize              The pointer to the buffer size
*/

IPPAPI(IppStatus, ippiFastMarchingGetBufferSize_8u32f_C1R, (IppiSize roiSize, int* pBufferSize))


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiFastMarching_8u32f_C1R
//
//  Purpose:    Calculate distance transform by fast marching method
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image or kernel size are less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     Source image
//    srcStep                  Its step
//    pDst                     Output image with distances
//    dstStep                  Its step
//    roiSize                  The image ROI size
//    radius                   The radius of external neighborhood
//    pBuffer                  Pointer to working buffer
//
//  Note: dst = min((src1+src1+sqrt(2-(src1-src2)**2))/2,min(src1,src2)+1)
//        for four neighbour pairs
*/

IPPAPI(IppStatus, ippiFastMarching_8u32f_C1R, (const Ipp8u* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                      IppiSize roiSize, Ipp32f radius, Ipp8u* pBuffer))


/****************************************************************************************\
*                                      Flood Fill                                        *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiFloodFillGetSize_4Con, ippiFloodFillGetSize_8Con
//           ippiFloodFillGetSize_Grad4Con, ippiFloodFillGetSize_Grad8Con
//
//  Purpose: The functions calculate size of temporary buffer, required to run
//           one of the corresponding flood fill functions.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//
//  Parameters:
//    roiSize                  ROI size
//    pBufSize                 Temporary buffer size
//F*/

IPPAPI( IppStatus, ippiFloodFillGetSize, ( IppiSize roiSize, int* pBufSize ))

IPPAPI( IppStatus, ippiFloodFillGetSize_Grad, ( IppiSize roiSize, int* pBufSize ))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Names:   ippiFloodFill_4Con_8u_C1IR,      ippiFloodFill_4Con_32f_C1IR,
//           ippiFloodFill_8Con_8u_C1IR,      ippiFloodFill_8Con_32f_C1IR,
//           ippiFloodFill_Grad4Con_8u_C1IR,  ippiFloodFill_Grad4Con_32f_C1IR,
//           ippiFloodFill_Grad8Con_8u_C1IR,  ippiFloodFill_Grad8Con_32f_C1IR
//           ippiFloodFill_Range4Con_8u_C1IR, ippiFloodFill_Range4Con_32f_C1IR,
//           ippiFloodFill_Range8Con_8u_C1IR, ippiFloodFill_Range8Con_32f_C1IR
//           ippiFloodFill_4Con_8u_C3IR,      ippiFloodFill_4Con_32f_C3IR,
//           ippiFloodFill_8Con_8u_C3IR,      ippiFloodFill_8Con_32f_C3IR,
//           ippiFloodFill_Grad4Con_8u_C3IR,  ippiFloodFill_Grad4Con_32f_C3IR,
//           ippiFloodFill_Grad8Con_8u_C3IR,  ippiFloodFill_Grad8Con_32f_C3IR
//           ippiFloodFill_Range4Con_8u_C3IR, ippiFloodFill_Range4Con_32f_C3IR,
//           ippiFloodFill_Range8Con_8u_C3IR, ippiFloodFill_Range8Con_32f_C3IR
//           ippiFloodFill_4Con_16u_C1IR,
//           ippiFloodFill_8Con_16u_C1IR,
//           ippiFloodFill_Grad4Con_16u_C1IR,
//           ippiFloodFill_Grad8Con_16u_C1IR,
//           ippiFloodFill_Range4Con_16u_C1IR,
//           ippiFloodFill_Range8Con_16u_C1IR,
//           ippiFloodFill_4Con_16u_C3IR,
//           ippiFloodFill_8Con_16u_C3IR,
//           ippiFloodFill_Grad4Con_16u_C3IR,
//           ippiFloodFill_Grad8Con_16u_C3IR,
//           ippiFloodFill_Range4Con_16u_C3IR,
//           ippiFloodFill_Range8Con_16u_C3IR,
//
//  Purpose: The functions fill the seed pixel enewValirons inside which all pixel
//           values are equal to (first 4 funcs) or not far from each other (the others).
//
//  Return:
//    ippStsNoErr              Ok.
//    ippStsNullPtrErr         One of pointers is NULL.
//    ippStsSizeErr            The width or height of images is less or equal zero.
//    ippStsStepErr            The steps in images are too small.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsOutOfRangeErr      Indicates an error condition if the seed point is out of ROI.
//
//  Parameters:
//    pImage                   Pointer to ROI of initial image (in the beginning)
//                             which is "repainted" during the function action,
//    imageStep                Full string length of initial image (in bytes),
//    roi                      Size of image ROI,
//    seed                     Coordinates of the seed point inside image ROI,
//    newVal                   Value to fill with for one-channel data,
//    pNewVal                  Pointer to the vector containing values to fill with
//                             for three-channel data,
//    minDelta                 Minimum difference between neighbor pixels for one-channel data,
//    maxDelta                 Maximum difference between neighbor pixels for one-channel data,
//    pMinDelta                Pointer to the minimum differences between neighbor pixels for
//                             three-channel images,
//    pMaxDelta                Pointer to the maximum differences between neighbor pixels for
//                             three-channel images,
//    pRegion                  Pointer to repainted region properties structure,
//    pBuffer                  Buffer needed for calculations (its size must be
//                             calculated by ippiFloodFillGetSize_Grad function).
//
//  Notes:   This function uses a rapid non-recursive algorithm.
//F*/

IPPAPI( IppStatus, ippiFloodFill_4Con_8u_C1IR, ( Ipp8u*  pImage, int imageStep,
                                                 IppiSize roiSize, IppiPoint seed,
                                                 Ipp8u newVal, IppiConnectedComp* pRegion,
                                                 Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_8Con_8u_C1IR, ( Ipp8u*  pImage, int imageStep,
                                                 IppiSize roiSize, IppiPoint seed,
                                                 Ipp8u newVal, IppiConnectedComp* pRegion,
                                                 Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_4Con_16u_C1IR, ( Ipp16u*  pImage, int imageStep,
                                                 IppiSize roiSize, IppiPoint seed,
                                                 Ipp16u newVal, IppiConnectedComp* pRegion,
                                                 Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_8Con_16u_C1IR, ( Ipp16u*  pImage, int imageStep,
                                                 IppiSize roiSize, IppiPoint seed,
                                                 Ipp16u newVal, IppiConnectedComp* pRegion,
                                                 Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_4Con_32f_C1IR, ( Ipp32f* pImage, int imageStep,
                                                  IppiSize roiSize, IppiPoint seed,
                                                  Ipp32f newVal, IppiConnectedComp* pRegion,
                                                  Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_8Con_32f_C1IR, ( Ipp32f* pImage, int imageStep,
                                                  IppiSize roiSize, IppiPoint seed,
                                                  Ipp32f newVal, IppiConnectedComp* pRegion,
                                                  Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Grad4Con_8u_C1IR, ( Ipp8u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp8u newVal, Ipp8u minDelta, Ipp8u maxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Grad8Con_8u_C1IR, ( Ipp8u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp8u newVal, Ipp8u minDelta, Ipp8u maxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Grad4Con_16u_C1IR, ( Ipp16u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp16u newVal, Ipp16u minDelta, Ipp16u maxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Grad8Con_16u_C1IR, ( Ipp16u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp16u newVal, Ipp16u minDelta, Ipp16u maxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Grad4Con_32f_C1IR, ( Ipp32f* pImage, int imageStep,
                                                      IppiSize roiSize, IppiPoint seed,
                                                      Ipp32f newVal, Ipp32f minDelta, Ipp32f maxDelta,
                                                      IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Grad8Con_32f_C1IR, ( Ipp32f* pImage, int imageStep,
                                                      IppiSize roiSize, IppiPoint seed,
                                                      Ipp32f newVal, Ipp32f minDelta, Ipp32f maxDelta,
                                                      IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Range4Con_8u_C1IR, ( Ipp8u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp8u newVal, Ipp8u minDelta, Ipp8u maxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Range8Con_8u_C1IR, ( Ipp8u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp8u newVal, Ipp8u minDelta, Ipp8u maxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Range4Con_16u_C1IR, ( Ipp16u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp16u newVal, Ipp16u minDelta, Ipp16u maxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Range8Con_16u_C1IR, ( Ipp16u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp16u newVal, Ipp16u minDelta, Ipp16u maxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Range4Con_32f_C1IR, ( Ipp32f* pImage, int imageStep,
                                                      IppiSize roiSize, IppiPoint seed,
                                                      Ipp32f newVal, Ipp32f minDelta, Ipp32f maxDelta,
                                                      IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Range8Con_32f_C1IR, ( Ipp32f* pImage, int imageStep,
                                                      IppiSize roiSize, IppiPoint seed,
                                                      Ipp32f newVal, Ipp32f minDelta, Ipp32f maxDelta,
                                                      IppiConnectedComp* pRegion, Ipp8u* pBuffer ))


IPPAPI( IppStatus, ippiFloodFill_4Con_8u_C3IR, ( Ipp8u*  pImage, int imageStep,
                                                 IppiSize roiSize, IppiPoint seed,
                                                 Ipp8u *pNewVal, IppiConnectedComp* pRegion,
                                                 Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_8Con_8u_C3IR, ( Ipp8u*  pImage, int imageStep,
                                                 IppiSize roiSize, IppiPoint seed,
                                                 Ipp8u *pNewVal, IppiConnectedComp* pRegion,
                                                 Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_4Con_16u_C3IR, ( Ipp16u*  pImage, int imageStep,
                                                 IppiSize roiSize, IppiPoint seed,
                                                 Ipp16u *pNewVal, IppiConnectedComp* pRegion,
                                                 Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_8Con_16u_C3IR, ( Ipp16u*  pImage, int imageStep,
                                                 IppiSize roiSize, IppiPoint seed,
                                                 Ipp16u *pNewVal, IppiConnectedComp* pRegion,
                                                 Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_4Con_32f_C3IR, ( Ipp32f* pImage, int imageStep,
                                                  IppiSize roiSize, IppiPoint seed,
                                                  Ipp32f *pNewVal, IppiConnectedComp* pRegion,
                                                  Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_8Con_32f_C3IR, ( Ipp32f* pImage, int imageStep,
                                                  IppiSize roiSize, IppiPoint seed,
                                                  Ipp32f *pNewVal, IppiConnectedComp* pRegion,
                                                  Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Grad4Con_8u_C3IR, ( Ipp8u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp8u *pNewVal, Ipp8u *pMinDelta, Ipp8u *pMaxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Grad8Con_8u_C3IR, ( Ipp8u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp8u *pNewVal, Ipp8u *pMinDelta, Ipp8u *pMaxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Grad4Con_16u_C3IR, ( Ipp16u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp16u *pNewVal, Ipp16u *pMinDelta, Ipp16u *pMaxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Grad8Con_16u_C3IR, ( Ipp16u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp16u *pNewVal, Ipp16u *pMinDelta, Ipp16u *pMaxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Grad4Con_32f_C3IR, ( Ipp32f* pImage, int imageStep,
                                                      IppiSize roiSize, IppiPoint seed,
                                                      Ipp32f *pNewVal, Ipp32f *pMinDelta, Ipp32f *pMaxDelta,
                                                      IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Grad8Con_32f_C3IR, ( Ipp32f* pImage, int imageStep,
                                                      IppiSize roiSize, IppiPoint seed,
                                                      Ipp32f *pNewVal, Ipp32f *pMinDelta, Ipp32f *pMaxDelta,
                                                      IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Range4Con_8u_C3IR, ( Ipp8u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp8u *pNewVal, Ipp8u *pMinDelta, Ipp8u *pMaxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Range8Con_8u_C3IR, ( Ipp8u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp8u *pNewVal, Ipp8u *pMinDelta, Ipp8u *pMaxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Range4Con_16u_C3IR, ( Ipp16u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp16u *pNewVal, Ipp16u *pMinDelta, Ipp16u *pMaxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Range8Con_16u_C3IR, ( Ipp16u*  pImage, int imageStep,
                                                     IppiSize roiSize, IppiPoint seed,
                                                     Ipp16u *pNewVal, Ipp16u *pMinDelta, Ipp16u *pMaxDelta,
                                                     IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Range4Con_32f_C3IR, ( Ipp32f* pImage, int imageStep,
                                                      IppiSize roiSize, IppiPoint seed,
                                                      Ipp32f *pNewVal, Ipp32f *pMinDelta, Ipp32f *pMaxDelta,
                                                      IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

IPPAPI( IppStatus, ippiFloodFill_Range8Con_32f_C3IR, ( Ipp32f* pImage, int imageStep,
                                                      IppiSize roiSize, IppiPoint seed,
                                                      Ipp32f *pNewVal, Ipp32f *pMinDelta, Ipp32f *pMaxDelta,
                                                      IppiConnectedComp* pRegion, Ipp8u* pBuffer ))

/****************************************************************************************\
*                                      Motion Templates                                  *
\****************************************************************************************/

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:      ippiUpdateMotionHistory_8u32f_C1IR,  ippiUpdateMotionHistory_16u32f_C1IR
//             ippiUpdateMotionHistory_32f_C1IR
//
//  Purpose:   Sets motion history image (MHI) pixels to the current time stamp
//             when the corrensonding pixels in the silhoette image are non zero.
//             Else (silhouette pixels are zero) MHI pixels are
//             cleared if their values are too small (less than timestamp - mhiDuration),
//             i.e. they were updated far ago last time. Else MHI pixels remain unchanged.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsOutOfRangeErr      Maximal duration is negative
//
//  Arguments:
//    pSilhouette              The pointer to silhouette image
//    silhStep                 The step in silhouette image
//    pMHI                     The pointer to motion history image
//    mhiStep                  The step in mhi image
//    roiSize                  ROI size
//    timestamp                Current time stamp (milliseconds)
//    mhiDuration              Maximal duration of motion track (milliseconds)
*/

IPPAPI(IppStatus, ippiUpdateMotionHistory_8u32f_C1IR,
                        ( const Ipp8u* pSilhouette, int silhStep,
                          Ipp32f* pMHI, int mhiStep, IppiSize roiSize,
                          Ipp32f timestamp, Ipp32f mhiDuration ))

IPPAPI(IppStatus, ippiUpdateMotionHistory_16u32f_C1IR,
                        ( const Ipp16u* pSilhouette, int silhStep,
                          Ipp32f* pMHI, int mhiStep, IppiSize roiSize,
                          Ipp32f timestamp, Ipp32f mhiDuration ))

IPPAPI(IppStatus, ippiUpdateMotionHistory_32f_C1IR,
                        ( const Ipp32f* pSilhouette, int silhStep,
                          Ipp32f* pMHI, int mhiStep, IppiSize roiSize,
                          Ipp32f timestamp, Ipp32f mhiDuration ))


/****************************************************************************************\
*                                        Optical Flow                                    *
\****************************************************************************************/


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiOpticalFlowPyrLKInitAlloc_8u_C1R, ippiOpticalFlowPyrLKInitAlloc_16u_C1R,
//              ippiOpticalFlowPyrLKInitAlloc_32f_C1R
//
//  Purpose:    allocates memory and initializes a structure for pyramidal L-K algorithm
//
//  Return:
//    ippStsNoErr              Indicates no error. Any other value indicates an error or a warning.
//    ippStsNullPtrErr         Indicates an error if ppState is NULL.
//    ippStsSizeErr            Indicates an error condition if roiSize has a field with zero
//                               or negative value or if winSize is equal to or less than 0.
//    ippStsMemAllocErr        Memory allocation error
//
//  Arguments:
//    pState                   Pointer to initialized structure
//    roi                      Maximal image ROI
//    winSize                  Size of search window (2*winSize+1)
//    hint                     Option to select the algorithmic implementation of the function
*/

IPPAPI(IppStatus, ippiOpticalFlowPyrLKInitAlloc_8u_C1R, (IppiOptFlowPyrLK_8u_C1R** ppState, IppiSize roiSize,
                         int winSize, IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiOpticalFlowPyrLKInitAlloc_16u_C1R, (IppiOptFlowPyrLK_16u_C1R** ppState, IppiSize roiSize,
                         int winSize, IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiOpticalFlowPyrLKInitAlloc_32f_C1R, (IppiOptFlowPyrLK_32f_C1R** ppState, IppiSize roiSize,
                         int winSize, IppHintAlgorithm hint))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiOpticalFlowPyrLKFree_8u_C1R,      ippiOpticalFlowPyrLKFree_16u_C1R,
//              ippiOpticalFlowPyrLKFree_32f_C1R
//
//  Purpose:    Free structure for pyramidal L-K algorithm
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//
//  Arguments:
//    pState                   Pointer to initialized structure
*/

IPPAPI(IppStatus, ippiOpticalFlowPyrLKFree_8u_C1R, (IppiOptFlowPyrLK_8u_C1R* pState))

IPPAPI(IppStatus, ippiOpticalFlowPyrLKFree_16u_C1R, (IppiOptFlowPyrLK_16u_C1R* pState))

IPPAPI(IppStatus, ippiOpticalFlowPyrLKFree_32f_C1R, (IppiOptFlowPyrLK_32f_C1R* pState))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiOpticalFlowPyrLK_8u_C1R, ippiOpticalFlowPyrLK_16u_C1R,
//        ippiOpticalFlowPyrLK_32f_C1R
//
//  Purpose:
//    Pyramidal version of Lucas - Kanade method of optical flow calculation
//
//  Returns:
//    ippStsNoErr              Indicates no error. Any other value indicates an error or a warning
//    ippStsNullPtrErr         Indicates an error if one of the specified pointer is NULL
//    ippStsSizeErr            Indicates an error condition if numFeat or winSize has zero or
//                               negative value.
//    ippStsBadArgErr          Indicates an error condition if maxLev or threshold has negative
//                               value, or maxIter has zero or negative value.
//
//  Arguments:
//    pPyr1                    Pointer to the first image pyramid (time t)
//    pPyr2                    Pointer to the second image pyramid (time t+dt)
//    pPrev                    Array of points, for which the flow needs to be found
//    pNext                    Array of new positions of pPrev points
//    pError                   Array of differences between pPrev and pNext points
//    pStatus                  Array of result indicator (0 - not calculated)
//    numFeat                  Number of points to calculate optical flow
//    winSize                  Size of search window (2*winSize+1)
//    maxLev                   Pyramid level to start the operation
//    maxIter                  Maximum number of algorithm iterations for each pyramid level
//    threshold                Threshold value to stop new position search
//    pState                   Pointer to structure
//
//    Notes:  For calculating spatial derivatives 3x3 Scharr operator is used.
//            The values of pixels beyond the image are determined using replication mode.
//F*/

IPPAPI(IppStatus, ippiOpticalFlowPyrLK_8u_C1R, (IppiPyramid *pPyr1, IppiPyramid *pPyr2,
                         const IppiPoint_32f *pPrev, IppiPoint_32f *pNext, Ipp8s *pStatus, Ipp32f *pError,
                         int numFeat, int winSize, int maxLev, int maxIter, Ipp32f threshold,
                         IppiOptFlowPyrLK_8u_C1R *pState))

IPPAPI(IppStatus, ippiOpticalFlowPyrLK_16u_C1R, (IppiPyramid *pPyr1, IppiPyramid *pPyr2,
                         const IppiPoint_32f *pPrev, IppiPoint_32f *pNext, Ipp8s *pStatus, Ipp32f *pError,
                         int numFeat, int winSize, int maxLev, int maxIter, Ipp32f threshold,
                         IppiOptFlowPyrLK_16u_C1R *pState))

IPPAPI(IppStatus, ippiOpticalFlowPyrLK_32f_C1R, (IppiPyramid *pPyr1, IppiPyramid *pPyr2,
                         const IppiPoint_32f *pRrev, IppiPoint_32f *pNext, Ipp8s *pStatus, Ipp32f *pError,
                         int numFeat, int winSize, int maxLev, int maxIter, Ipp32f threshold,
                         IppiOptFlowPyrLK_32f_C1R *pState))


/****************************************************************************************\
*                                    Gaussian Pyramids                                   *
\****************************************************************************************/


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiPyrUpGetBufSize_Gauss5x5, ippiPyrDownGetBufSize_Gauss5x5
//
//  Purpose:    Calculates cyclic buffer size for pyramids.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         The pbufSize pointer is NULL
//    ippStsSizeErr            The value of roiWidth is zerro or negative
//    ippStsDataTypeErr        The dataType is not Ipp8u, Ipp8s or Ipp32f
//    ippStsNumChannensErr     The channels is not 1 or 3
//
//  Arguments:
//    roiWidth                 Width of image ROI in pixels
//    dataType                 Data type of the source image
//    channels                 Number of image channels
//    pbufSize                 Pointer to the variable that return the size of the temporary buffer.
*/

IPPAPI(IppStatus, ippiPyrUpGetBufSize_Gauss5x5, (int roiWidth, IppDataType dataType,
                                                 int channels, int* bufSize))

IPPAPI(IppStatus, ippiPyrDownGetBufSize_Gauss5x5, (int roiWidth, IppDataType dataType,
                                                   int channels, int* bufSize))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiPyrDown_Gauss5x5_8u_C1R, ippiPyrDown_Gauss5x5_8u_C3R,
//              ippiPyrDown_Gauss5x5_8s_C1R, ippiPyrDown_Gauss5x5_8s_C3R,
//              ippiPyrDown_Gauss5x5_32f_C1R, ippiPyrDown_Gauss5x5_32f_C3R,
//
//              ippiPyrUp_Gauss5x5_8u_C1R, ippiPyrUp_Gauss5x5_8u_C3R,
//              ippiPyrUp_Gauss5x5_8s_C1R, ippiPyrUp_Gauss5x5_8s_C3R,
//              ippiPyrUp_Gauss5x5_32f_C1R, ippiPyrUp_Gauss5x5_32f_C3R,
//
//  Purpose:    Perform downsampling/upsampling of the image with 5x5 gaussian.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero.
//    ippStsStepErr            Step is too small to fit image.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Arguments:
//    pSrc                     Pointer to source image
//    srcStep                  Step in bytes through the source image
//    pDst                     Pointer to destination image
//    dstStep                  Step in bytes through the destination image
//    roiSize                  Size of the source image ROI in pixel. Destination image width and
//                             height will be twice large (PyrUp)
//                             or twice smaller (PyrDown)
//    pBuffer                  Pointer to the the temporary buffer of the size calculated by
//                             ippPyrUpGetSize_Gauss_5x5 or ippPyrDownGetSize_Gauss_5x5
*/

IPPAPI(IppStatus, ippiPyrUp_Gauss5x5_8u_C1R,  (const Ipp8u* pSrc, int srcStep,
                                               Ipp8u* pDst, int dstStep,
                                               IppiSize roiSize, Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiPyrUp_Gauss5x5_8u_C3R,  (const Ipp8u* pSrc, int srcStep,
                                               Ipp8u* pDst, int dstStep,
                                               IppiSize roiSize, Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiPyrUp_Gauss5x5_8s_C1R,  (const Ipp8s* pSrc, int srcStep,
                                               Ipp8s* pDst, int dstStep,
                                               IppiSize roiSize, Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiPyrUp_Gauss5x5_8s_C3R,  (const Ipp8s* pSrc, int srcStep,
                                               Ipp8s* pDst, int dstStep,
                                               IppiSize roiSize, Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiPyrUp_Gauss5x5_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                               Ipp32f* pDst, int dstStep,
                                               IppiSize roiSize, Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiPyrUp_Gauss5x5_32f_C3R, (const Ipp32f* pSrc, int srcStep,
                                               Ipp32f* pDst, int dstStep,
                                               IppiSize roiSize, Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiPyrDown_Gauss5x5_8u_C1R,  (const Ipp8u* pSrc, int srcStep,
                                                 Ipp8u* pDst, int dstStep,
                                                 IppiSize roiSize, Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiPyrDown_Gauss5x5_8u_C3R,  (const Ipp8u* pSrc, int srcStep,
                                                 Ipp8u* pDst, int dstStep,
                                                 IppiSize roiSize, Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiPyrDown_Gauss5x5_8s_C1R,  (const Ipp8s* pSrc, int srcStep,
                                                 Ipp8s* pDst, int dstStep,
                                                 IppiSize roiSize, Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiPyrDown_Gauss5x5_8s_C3R,  (const Ipp8s* pSrc, int srcStep,
                                                 Ipp8s* pDst, int dstStep,
                                                 IppiSize roiSize, Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiPyrDown_Gauss5x5_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                                 Ipp32f* pDst, int dstStep,
                                                 IppiSize roiSize, Ipp8u* pBuffer ))

IPPAPI(IppStatus, ippiPyrDown_Gauss5x5_32f_C3R, (const Ipp32f* pSrc, int srcStep,
                                                 Ipp32f* pDst, int dstStep,
                                                 IppiSize roiSize, Ipp8u* pBuffer ))


/****************************************************************************************\
*                                    Universal Pyramids                                  *
\****************************************************************************************/


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiPyramidInitAlloc, ippiPyramidFree
//
//  Purpose:    Initializes structure for pyramids, calculates ROI for layers,
//              allocates images for layers.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsBadArgErr          Bad rate or level
//    ippStsMemAllocErr        Memory allocation error
//
//  Arguments:
//    ppPyr                    Pointer to the pointer to the pyramid structure.
//    pPyr                     Pointer to the pyramid structure.
//    level                    Maximal number pyramid level.
//    roiSize                  Lowest level image ROI size.
//    rate                     Neighbour levels ratio (1<rate<=10)
*/
IPPAPI(IppStatus, ippiPyramidInitAlloc,(IppiPyramid** ppPyr, int level, IppiSize roiSize, Ipp32f rate))
IPPAPI(IppStatus, ippiPyramidFree,(IppiPyramid* pPyr))

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiPyramidLayerDownInitAlloc_8u_C1R,   ippiPyramidLayerDownInitAlloc_8u_C3R
//              ippiPyramidLayerDownInitAlloc_16u_C1R,  ippiPyramidLayerDownInitAlloc_16u_C3R
//              ippiPyramidLayerDownInitAlloc_32f_C1R,  ippiPyramidLayerDownInitAlloc_32f_C3R
//              ippiPyramidLayerUpInitAlloc_8u_C1R,     ippiPyramidLayerUpInitAlloc_8u_C3R
//              ippiPyramidLayerUpInitAlloc_16u_C1R,    ippiPyramidLayerUpInitAlloc_16u_C3R
//              ippiPyramidLayerUpInitAlloc_32f_C1R,    ippiPyramidLayerUpInitAlloc_32f_C3R
//
//  Purpose:    Initializes structure for pyramid layer calculation
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsBadArgErr          Bad mode, rate or kernel size
//    ippStsMemAllocErr        Memory allocation error
//
//  Arguments:
//    ppState                  Pointer to the pointer to initialized structure
//    srcRoi                   Source image ROI size.
//    dstRoi                   Destination image ROI size.
//    rate                     Neighbour levels ratio (1<rate<4)
//    pKernel                  Separable symmetric kernel of odd length
//    kerSize                  Kernel size
//    mode                     IPPI_INTER_LINEAR - bilinear interpolation
*/

IPPAPI(IppStatus, ippiPyramidLayerDownInitAlloc_8u_C1R, (IppiPyramidDownState_8u_C1R**  ppState, IppiSize srcRoi,
                         Ipp32f rate, Ipp16s* pKernel, int kerSize, int mode))
IPPAPI(IppStatus, ippiPyramidLayerDownInitAlloc_16u_C1R,(IppiPyramidDownState_16u_C1R** ppState, IppiSize srcRoi,
                         Ipp32f rate, Ipp16s* pKernel, int kerSize, int mode))
IPPAPI(IppStatus, ippiPyramidLayerDownInitAlloc_32f_C1R,(IppiPyramidDownState_32f_C1R** ppState, IppiSize srcRoi,
                         Ipp32f rate, Ipp32f* pKernel, int kerSize, int mode))
IPPAPI(IppStatus, ippiPyramidLayerDownInitAlloc_8u_C3R, (IppiPyramidDownState_8u_C3R**  ppState, IppiSize srcRoi,
                         Ipp32f rate, Ipp16s* pKernel, int kerSize, int mode))
IPPAPI(IppStatus, ippiPyramidLayerDownInitAlloc_16u_C3R,(IppiPyramidDownState_16u_C3R** ppState, IppiSize srcRoi,
                         Ipp32f rate, Ipp16s* pKernel, int kerSize, int mode))
IPPAPI(IppStatus, ippiPyramidLayerDownInitAlloc_32f_C3R,(IppiPyramidDownState_32f_C3R** ppState, IppiSize srcRoi,
                         Ipp32f rate, Ipp32f* pKernel, int kerSize, int mode))

IPPAPI(IppStatus, ippiPyramidLayerUpInitAlloc_8u_C1R, (IppiPyramidUpState_8u_C1R**  ppState, IppiSize dstRoi,
                         Ipp32f rate, Ipp16s* pKernel, int kerSize, int mode))
IPPAPI(IppStatus, ippiPyramidLayerUpInitAlloc_16u_C1R,(IppiPyramidUpState_16u_C1R** ppState, IppiSize dstRoi,
                         Ipp32f rate, Ipp16s* pKernel, int kerSize, int mode))
IPPAPI(IppStatus, ippiPyramidLayerUpInitAlloc_32f_C1R,(IppiPyramidUpState_32f_C1R** ppState, IppiSize dstRoi,
                         Ipp32f rate, Ipp32f* pKernel, int kerSize, int mode))
IPPAPI(IppStatus, ippiPyramidLayerUpInitAlloc_8u_C3R, (IppiPyramidUpState_8u_C3R**  ppState, IppiSize dstRoi,
                         Ipp32f rate, Ipp16s* pKernel, int kerSize, int mode))
IPPAPI(IppStatus, ippiPyramidLayerUpInitAlloc_16u_C3R,(IppiPyramidUpState_16u_C3R** ppState, IppiSize dstRoi,
                         Ipp32f rate, Ipp16s* pKernel, int kerSize, int mode))
IPPAPI(IppStatus, ippiPyramidLayerUpInitAlloc_32f_C3R,(IppiPyramidUpState_32f_C3R** ppState, IppiSize dstRoi,
                         Ipp32f rate, Ipp32f* pKernel, int kerSize, int mode))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiPyramidLayerDownFree_8u_C1R,   ippiPyramidLayerDownFree_8u_C3R
//              ippiPyramidLayerDownFree_16u_C1R,  ippiPyramidLayerDownFree_16u_C3R
//              ippiPyramidLayerDownFree_32f_C1R,  ippiPyramidLayerDownFree_32f_C3R
//              ippiPyramidLayerUpFree_8u_C1R,     ippiPyramidLayerUpFree_8u_C3R
//              ippiPyramidLayerUpFree_16u_C1R,    ippiPyramidLayerUpFree_16u_C3R
//              ippiPyramidLayerUpFree_32f_C1R,    ippiPyramidLayerUpFree_32f_C3R
//
//  Purpose:    Initializes structure for pyramid layer calculation
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//
//  Arguments:
//    pState                   Pointer to initialized structure
*/

IPPAPI(IppStatus, ippiPyramidLayerDownFree_8u_C1R, (IppiPyramidDownState_8u_C1R*  pState))
IPPAPI(IppStatus, ippiPyramidLayerDownFree_16u_C1R,(IppiPyramidDownState_16u_C1R* pState))
IPPAPI(IppStatus, ippiPyramidLayerDownFree_32f_C1R,(IppiPyramidDownState_32f_C1R* pState))
IPPAPI(IppStatus, ippiPyramidLayerDownFree_8u_C3R, (IppiPyramidDownState_8u_C3R*  pState))
IPPAPI(IppStatus, ippiPyramidLayerDownFree_16u_C3R,(IppiPyramidDownState_16u_C3R* pState))
IPPAPI(IppStatus, ippiPyramidLayerDownFree_32f_C3R,(IppiPyramidDownState_32f_C3R* pState))

IPPAPI(IppStatus, ippiPyramidLayerUpFree_8u_C1R, (IppiPyramidUpState_8u_C1R*  pState))
IPPAPI(IppStatus, ippiPyramidLayerUpFree_16u_C1R,(IppiPyramidUpState_16u_C1R* pState))
IPPAPI(IppStatus, ippiPyramidLayerUpFree_32f_C1R,(IppiPyramidUpState_32f_C1R* pState))
IPPAPI(IppStatus, ippiPyramidLayerUpFree_8u_C3R, (IppiPyramidUpState_8u_C3R*  pState))
IPPAPI(IppStatus, ippiPyramidLayerUpFree_16u_C3R,(IppiPyramidUpState_16u_C3R* pState))
IPPAPI(IppStatus, ippiPyramidLayerUpFree_32f_C3R,(IppiPyramidUpState_32f_C3R* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiGetPyramidDownROI, ippiGetPyramidUpROI
//
//  Purpose:    Calculate possible size of destination ROI.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            Wrong src roi
//    ippStsBadArgErr          Wrong rate
//
//  Arguments:
//    srcRoi                   Source image ROI size.
//    pDstRoi                  Pointer to destination image ROI size (down).
//    pDstRoiMin               Pointer to minimal destination image ROI size (up).
//    pDstRoiMax               Pointer to maximal destination image ROI size (up).
//    rate                     Neighbour levels ratio (1<rate<=10)
//
//  Notes:                     For up case destination size belongs to interval
//                             max((int)((float)((src-1)*rate)),src+1)<=dst<=
//                             max((int)((float)(src)*rate)),src+1)
*/

IPPAPI(IppStatus, ippiGetPyramidDownROI,(IppiSize srcRoi, IppiSize *pDstRoi, Ipp32f rate))
IPPAPI(IppStatus, ippiGetPyramidUpROI,(IppiSize srcRoi, IppiSize *pDstRoiMin, IppiSize *pDstRoiMax, Ipp32f rate))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiPyramidLayerDown_8u_C1R, ippiPyramidLayerDown_16u_C1R, ippiPyramidLayerDown_32f_C1R
//              ippiPyramidLayerDown_8u_C3R, ippiPyramidLayerDown_16u_C3R, ippiPyramidLayerDown_32f_C3R
//              ippiPyramidLayerUp_8u_C1R,   ippiPyramidLayerUp_16u_C1R,   ippiPyramidLayerUp_32f_C1R
//              ippiPyramidLayerUp_8u_C3R,   ippiPyramidLayerUp_16u_C3R,   ippiPyramidLayerUp_32f_C3R
//
//  Purpose:    Perform downsampling/upsampling of the image with 5x5 gaussian.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of the specified pointers is NULL
//    ippStsSizeErr            The srcRoiSize or dstRoiSize has a fild with zero or negativ value
//    ippStsStepErr            The steps in images are too small
//    ippStsBadArgErr          pState->rate has wrong value
//    ippStsNotEvenStepErr     One of the step values is not divisibly by 4 for floating-point
//                             images, or by 2 for short-integer images.
//  Arguments:
//    pSrc                     Pointer to the source image
//    srcStep                  Step in byte through the source image
//    srcRoiSize               Size of the source image ROI in pixel.
//    dstRoiSize               Size of the destination image ROI in pixel.
//    pDst                     Pointer to destination image
//    dstStep                  Step in byte through the destination image
//    pState                   Pointer to the pyramid layer structure
*/

IPPAPI(IppStatus, ippiPyramidLayerDown_8u_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize srcRoiSize,
                         Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiPyramidDownState_8u_C1R* pState))
IPPAPI(IppStatus, ippiPyramidLayerDown_8u_C3R, (const Ipp8u* pSrc, int srcStep, IppiSize srcRoiSize,
                         Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiPyramidDownState_8u_C3R* pState))
IPPAPI(IppStatus, ippiPyramidLayerDown_16u_C1R,(const Ipp16u* pSrc, int srcStep, IppiSize srcRoiSize,
                         Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiPyramidDownState_16u_C1R* pState))
IPPAPI(IppStatus, ippiPyramidLayerDown_16u_C3R,(const Ipp16u* pSrc, int srcStep, IppiSize srcRoiSize,
                         Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiPyramidDownState_16u_C3R* pState))
IPPAPI(IppStatus, ippiPyramidLayerDown_32f_C1R,(const Ipp32f* pSrc, int srcStep, IppiSize srcRoiSize,
                         Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiPyramidDownState_32f_C1R* pState))
IPPAPI(IppStatus, ippiPyramidLayerDown_32f_C3R,(const Ipp32f* pSrc, int srcStep, IppiSize srcRoiSize,
                         Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiPyramidDownState_32f_C3R* pState))

IPPAPI(IppStatus, ippiPyramidLayerUp_8u_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize srcRoiSize,
                         Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiPyramidUpState_8u_C1R* pState))
IPPAPI(IppStatus, ippiPyramidLayerUp_8u_C3R, (const Ipp8u* pSrc, int srcStep, IppiSize srcRoiSize,
                         Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiPyramidUpState_8u_C3R* pState))
IPPAPI(IppStatus, ippiPyramidLayerUp_16u_C1R,(const Ipp16u* pSrc, int srcStep, IppiSize srcRoiSize,
                         Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiPyramidUpState_16u_C1R* pState))
IPPAPI(IppStatus, ippiPyramidLayerUp_16u_C3R,(const Ipp16u* pSrc, int srcStep, IppiSize srcRoiSize,
                         Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiPyramidUpState_16u_C3R* pState))
IPPAPI(IppStatus, ippiPyramidLayerUp_32f_C1R,(const Ipp32f* pSrc, int srcStep, IppiSize srcRoiSize,
                         Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiPyramidUpState_32f_C1R* pState))
IPPAPI(IppStatus, ippiPyramidLayerUp_32f_C3R,(const Ipp32f* pSrc, int srcStep, IppiSize srcRoiSize,
                         Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiPyramidUpState_32f_C3R* pState))


/****************************************************************************************\
*                                     Haar Classifier                                    *
\****************************************************************************************/


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:      ippiHaarClassifierInitAlloc_32f,  ippiTiltedHaarClassifierInitAlloc_32f
//             ippiHaarClassifierInitAlloc_32s,  ippiTiltedHaarClassifierInitAlloc_32s
//
//  Purpose:   Allocates and initializes memory for the stage of the Haar classifier
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The number of classifiers or features is less or equal zero
//    ippStsbadArgErr          The bad feature rectangular
//    ippStsMemAllocErr        Memory allocation error
//
//  Arguments:
//    pState                   The pointer to the pointer to the Haar classifier structure.
//    pFeature                 The pointer to the array of features.
//    pWeight                  The pointer to the array of feature weights.
//    pThreshold               The pointer to the array of classifier thresholds [length].
//    pVal1, pVal2             Pointers to arrays of classifier results [length].
//    pNum                     The pointer to the array of classifier lengths [length].
//    length                   The number of classifiers in the stage.
//
//  Notes:  For integer version feature weights pWeight are in Q0, classifier thresholds
//          pThreshold are in QT (see ApplyHaarClassifier), pVal1 and pVal2 are scale as
//          stage thresholds threshold of ApplyHaarClassifier function
*/

IPPAPI(IppStatus, ippiHaarClassifierInitAlloc_32f, (IppiHaarClassifier_32f **pState,
                 const IppiRect* pFeature, const Ipp32f* pWeight, const Ipp32f* pThreshold,
                 const Ipp32f* pVal1, const Ipp32f* pVal2, const int* pNum, int length))

IPPAPI(IppStatus, ippiTiltedHaarClassifierInitAlloc_32f, (IppiHaarClassifier_32f **pState,
                 const IppiRect* pFeature, const Ipp32f* pWeight, const Ipp32f* pThreshold,
                 const Ipp32f* pVal1, const Ipp32f* pVal2, const int* pNum, int length))

IPPAPI(IppStatus, ippiHaarClassifierInitAlloc_32s, (IppiHaarClassifier_32s **pState,
                 const IppiRect* pFeature, const Ipp32s* pWeight, const Ipp32s* pThreshold,
                 const Ipp32s* pVal1, const Ipp32s* pVal2, const int* pNum, int length))

IPPAPI(IppStatus, ippiTiltedHaarClassifierInitAlloc_32s, (IppiHaarClassifier_32s **pState,
                 const IppiRect* pFeature, const Ipp32s* pWeight, const Ipp32s* pThreshold,
                 const Ipp32s* pVal1, const Ipp32s* pVal2, const int* pNum, int length))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiHaarClassifierFree_32f,         ippiHaarClassifierFree_32s
//
//  Purpose:    Free structure for Haar classifier
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//
//  Arguments:
//    pState                   Pointer to the structure to be freed
*/

IPPAPI(IppStatus, ippiHaarClassifierFree_32f,(IppiHaarClassifier_32f *pState))

IPPAPI(IppStatus, ippiHaarClassifierFree_32s,(IppiHaarClassifier_32s *pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:      ippiTiltHaarFeatures_32f,  ippiTiltHaarFeatures_32s
//
//  Purpose:   Tilts marked feature on -45 degree
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//
//  Arguments:
//    pMask                    The mask of feature to tilt.
//    flag                     1 - left bottom  -45 degree
//                             0 - left top     +45 degree
//    pState                   The pointer to the Haar classifier structure.
//
//  Notes:  The mask length is equal to the number of classifiers in the classifier
//          If pMask[i] != 0 i-th feature is tilted
//          Classifiers with tilted features require two input integral images and
//          can be used by rippiApplyMixedHaarClassifier functions
*/

IPPAPI(IppStatus, ippiTiltHaarFeatures_32f, (const Ipp8u *pMask, int flag, IppiHaarClassifier_32f *pState))

IPPAPI(IppStatus, ippiTiltHaarFeatures_32s, (const Ipp8u *pMask, int flag, IppiHaarClassifier_32s *pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiGetHaarClassifierSize_32f,      ippiGetHaarClassifierSize_32s
//
//  Purpose:    Returns the size of the Haar classifier.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//
//  Arguments:
//    pState    Pointer to the Haar classifier structure.
//    pSize        Pointer to the returned value of Haar classifier size.
*/

IPPAPI(IppStatus, ippiGetHaarClassifierSize_32f, (IppiHaarClassifier_32f* pState, IppiSize* pSize))

IPPAPI(IppStatus, ippiGetHaarClassifierSize_32s, (IppiHaarClassifier_32s* pState, IppiSize* pSize))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:      ippiApplyHaarClassifier_32f_C1R,    ippiApplyMixedHaarClassifier_32f_C1R,
//             ippiApplyHaarClassifier_32s32f_C1R, ippiApplyMixedHaarClassifier_32s32f_C1R,
//             ippiApplyHaarClassifier_32s_C1RSfs, ippiApplyMixedHaarClassifier_32s_C1RSfs
//
//  Purpose:   Applies the stage of Haar classifiers to the image
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The number of classifiers or features is less or equal zero
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Arguments:
//    pSrc                     The pointer  to the source image of integrals.
//    srcStep                  The step in bytes through the source image.
//    pNorm                    The pointer  to the source image of norm factors.
//    normStep                 The step  in bytes through the image of norm factors.
//    pMask                    The pointer  to the source and destination image of classification decisions.
//    maskStep                 The step  in bytes through the image of classification decisions.
//    pPositive                The pointer to the number of positive decisions.
//    roiSize                  The size of source and destination images ROI in pixels.
//    threshold                The stage threshold value.
//    pState                   The pointer to the Haar classifier structure.
//    scaleFactor              Scale factor for classifier threshold*norm, <= 0
*/

IPPAPI(IppStatus, ippiApplyHaarClassifier_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                         const Ipp32f* pNorm, int normStep, Ipp8u* pMask, int maskStep,
                         IppiSize roiSize, int *pPositive, Ipp32f threshold,
                         IppiHaarClassifier_32f *pState))

IPPAPI(IppStatus, ippiApplyHaarClassifier_32s32f_C1R, (const Ipp32s* pSrc, int srcStep,
                         const Ipp32f* pNorm, int normStep, Ipp8u* pMask, int maskStep,
                         IppiSize roiSize, int *pPositive, Ipp32f threshold,
                         IppiHaarClassifier_32f *pState))

IPPAPI(IppStatus, ippiApplyHaarClassifier_32s_C1RSfs, (const Ipp32s* pSrc, int srcStep,
                         const Ipp32s* pNorm, int normStep, Ipp8u* pMask, int maskStep,
                         IppiSize roiSize, int *pPositive, Ipp32s threshold,
                         IppiHaarClassifier_32s *pState, int scaleFactor))

IPPAPI(IppStatus, ippiApplyMixedHaarClassifier_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                         const Ipp32f* pTilt, int tiltStep, const Ipp32f* pNorm, int normStep,
                         Ipp8u* pMask, int maskStep, IppiSize roiSize, int *pPositive, Ipp32f threshold,
                         IppiHaarClassifier_32f *pState))

IPPAPI(IppStatus, ippiApplyMixedHaarClassifier_32s32f_C1R, (const Ipp32s* pSrc, int srcStep,
                         const Ipp32s* pTilt, int tiltStep, const Ipp32f* pNorm, int normStep,
                         Ipp8u* pMask, int maskStep, IppiSize roiSize, int *pPositive, Ipp32f threshold,
                         IppiHaarClassifier_32f *pState))

IPPAPI(IppStatus, ippiApplyMixedHaarClassifier_32s_C1RSfs, (const Ipp32s* pSrc, int srcStep,
                         const Ipp32s* pTilt, int tiltStep, const Ipp32s* pNorm, int normStep,
                         Ipp8u* pMask, int maskStep, IppiSize roiSize, int *pPositive, Ipp32s threshold,
                         IppiHaarClassifier_32s *pState, int scaleFactor))


/****************************************************************************************\
*                              Correction of Camera Distortions                          *
\****************************************************************************************/


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiUndistortGetSize
//
//  Purpose: calculate the buffer size for Undistort functions
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//
//  Parameters:
//    roiSize                  Maximal image size
//    pBufsize                 Pointer to work buffer size
//
//  Notes:
//F*/

IPPAPI(IppStatus, ippiUndistortGetSize, (IppiSize roiSize, int *pBufsize))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiCreateMapCameraUndistort_32f_C1R
//
//  Purpose: initialize x and y maps for undistortion by ippiRemap function
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         pxMap or pyMap is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsBadArgErr          Bad fx or fy
//
//  Parameters:
//    pxMap                    Pointer to x map (result, free by ippiFree)
//    xStep                    Pointer to x map row step (result)
//    pyMap                    Pointer to x map (result, free by ippiFree)
//    yStep                    Pointer to x map row step (result)
//    roiSize                  Maximal image size
//    fx, fy                   Focal lengths
//    cx, cy                   Coordinates of principal point
//    k1, k2                   Coeffs of radial distortion
//    p1, p2                   Coeffs of tangential distortion
//    pBuffer                  Pointer to work buffer
//
//  Notes:
//    fx, fy != 0
//F*/

IPPAPI(IppStatus, ippiCreateMapCameraUndistort_32f_C1R, (Ipp32f *pxMap, int xStep,
                  Ipp32f *pyMap, int yStep, IppiSize roiSize, Ipp32f fx, Ipp32f fy, Ipp32f cx, Ipp32f cy,
                  Ipp32f k1, Ipp32f k2, Ipp32f p1, Ipp32f p2, Ipp8u *pBuffer))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiUndistortRadial_8u_C1R,  ippiUndistortRadial_8u_C3R,
//           ippiUndistortRadial_16u_C1R, ippiUndistortRadial_16u_C3R
//           ippiUndistortRadial_32f_C1R, ippiUndistortRadial_32f_C3R
//
//  Purpose: correct camera distortion
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         pSrc or pDst is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsBadArgErr          Bad fx or fy
//
//  Parameters:
//    pSrc                     Source image
//    srcStep                  Step in source image
//    pDst                     Pointer to destination image
//    dstStep                  Step in destination image
//    roiSize                  Source and destination image ROI size.
//    fx, fy                   Focal lengths
//    cx, cy                   Coordinates of principal point
//    k1, k2                   Coeffs of radial distortion
//    pBuffer                  Pointer to work buffer
//
//  Notes:
//F*/

IPPAPI(IppStatus, ippiUndistortRadial_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                         Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp32f fx, Ipp32f fy,
                         Ipp32f cx, Ipp32f cy, Ipp32f k1, Ipp32f k2, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiUndistortRadial_16u_C1R, (const Ipp16u* pSrc, int srcStep,
                         Ipp16u* pDst, int dstStep, IppiSize roiSize, Ipp32f fx, Ipp32f fy,
                         Ipp32f cx, Ipp32f cy, Ipp32f k1, Ipp32f k2, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiUndistortRadial_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                         Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f fx, Ipp32f fy,
                         Ipp32f cx, Ipp32f cy, Ipp32f k1, Ipp32f k2, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiUndistortRadial_8u_C3R, (const Ipp8u* pSrc, int srcStep,
                         Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp32f fx, Ipp32f fy,
                         Ipp32f cx, Ipp32f cy, Ipp32f k1, Ipp32f k2, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiUndistortRadial_16u_C3R, (const Ipp16u* pSrc, int srcStep,
                         Ipp16u* pDst, int dstStep, IppiSize roiSize, Ipp32f fx, Ipp32f fy,
                         Ipp32f cx, Ipp32f cy, Ipp32f k1, Ipp32f k2, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiUndistortRadial_32f_C3R, (const Ipp32f* pSrc, int srcStep,
                         Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f fx, Ipp32f fy,
                         Ipp32f cx, Ipp32f cy, Ipp32f k1, Ipp32f k2, Ipp8u *pBuffer))


/*F/////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiGradientColorToGray_8u_C3C1R,
//           ippiGradientColorToGray_16u_C3C1R,
//           ippiGradientColorToGray_32f_C3C1R
//
//  Purpose: Calculate gray gradient from 3-channel gradient image.
//
//  Return:
//    ippStsNoErr         Ok
//    ippStsNullPtrErr    One of pointers is NULL
//    ippStsSizeErr       The width or height of images is less or equal zero
//    ippStsStepErr       The steps in images are too small
//
//  Parameters:
//    pSrc                The source image
//    srcStep             Its step
//    pDst                The destination image
//    dstStep             Its step
//    roiSize             ROI size
//    norm                The norm type
//                                  rippiNormInf = max(|a|,|b|,|c|)
//                                  rippiNormL1  = (|a|+|b|+|c|)/3
//                                  rippiNormL2  = sqrt((a*a+b*b+c*c)/3)
//
//  Note:                 For integer flavors, the result is scaled to the full range of the
//                        destination data type
//F*/

IPPAPI(IppStatus, ippiGradientColorToGray_8u_C3C1R, (const Ipp8u* pSrc, int srcStep,
                 Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiNorm norm))

IPPAPI(IppStatus, ippiGradientColorToGray_16u_C3C1R, (const Ipp16u* pSrc, int srcStep,
                 Ipp16u* pDst, int dstStep, IppiSize roiSize, IppiNorm norm))

IPPAPI(IppStatus, ippiGradientColorToGray_32f_C3C1R, (const Ipp32f* pSrc, int srcStep,
                 Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiNorm norm))


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiInpaintInitAlloc_8u_C1R,    ippiInpaintInitAlloc_8u_C3R
//
//  Purpose:    allocates memory and initializes a structure for direct inpainting algorithm
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image  are less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element
//    ippStsBadArgErr          Wrong radius or flags
//
//  Parameters:
//    ppState                  Pointer to pointer to inpainting structure
//    pDist                    Distance image
//    distStep                 Its step
//    pMask                    Distance image
//    maskStep                 Its step
//    roiSize                  The image ROI size
//    radius                   Neighborhood radius (dist<=radius pixels are processed)
//    flags                    Inpainting flags
//                                IPP_INPAINT_TELEA  Telea algorithm is used
//                                IPP_INPAINT_NS     Navier-Stokes equation is used
*/

IPPAPI(IppStatus, ippiInpaintInitAlloc_8u_C1R, (IppiInpaintState_8u_C1R** ppState, const Ipp32f* pDist,
                                                int distStep, const Ipp8u* pMask, int maskStep,
                                                IppiSize roiSize, Ipp32f radius, IppiInpaintFlag flags))

IPPAPI(IppStatus, ippiInpaintInitAlloc_8u_C3R, (IppiInpaintState_8u_C3R** ppState, const Ipp32f* pDist,
                                                int distStep, const Ipp8u* pMask, int maskStep,
                                                IppiSize roiSize, Ipp32f radius, IppiInpaintFlag flags))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiInpaintFree_8u_C1R,      ippiInpaintFree_8u_C3R
//
//  Purpose:    Free structure for inpainting algorithm
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//
//  Arguments:
//    pState                   Pointer to structure to free
*/

IPPAPI(IppStatus, ippiInpaintFree_8u_C1R, (IppiInpaintState_8u_C1R* pState))

IPPAPI(IppStatus, ippiInpaintFree_8u_C3R, (IppiInpaintState_8u_C3R* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiInpaint_8u_C1R,    ippiInpaint_8u_C3R
//
//  Purpose:    restores damaged image area by direct inpainting
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image  are less or equal zero or different from initalloc
//    ippStsStepErr            The steps in images are too small
//
//  Parameters:
//    pSrc                     Source image
//    srcStep                  Its step
//    pDst                     Destination image
//    dstStep                  Its step
//    roiSize                  The image ROI size
//    pState                   Pointer to inpainting structure
*/

IPPAPI(IppStatus, ippiInpaint_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                       IppiSize roiSize, IppiInpaintState_8u_C1R* pState))

IPPAPI(IppStatus, ippiInpaint_8u_C3R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                       IppiSize roiSize, IppiInpaintState_8u_C1R* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiSegmentGradientGetBufferSize_8u_C1R
//         ippiSegmentGradientGetBufferSize_8u_C3R
//
//  Purpose:    Get size of external buffer.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image or kernel size are less or equal zero
//
//  Parameters:
//    roiSize                  The image ROI size
//    pBufSize                 The pointer to the buffer size
*/

IPPAPI(IppStatus, ippiSegmentGradientGetBufferSize_8u_C1R, (IppiSize roiSize, int* pBufSize))

IPPAPI(IppStatus, ippiSegmentGradientGetBufferSize_8u_C3R, (IppiSize roiSize, int* pBufSize))


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiSegmentGradient_8u_C1R, ippiSegmentGradient_8u_C3R
//
//  Purpose:    Draw bounds between image segments
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image or kernel size are less or equal zero
//    ippStsStepErr            The steps in images are too small
//
//  Parameters:
//    pSrc                     Source image
//    srcStep                  Its step
//    pMarker                  Source and destination marker image
//    markerStep               Its step
//    roiSize                  Image ROI size
//    norm                     Type of norm to form the mask for maximum search:
//                                rippiNormInf   Infinity norm (8-connectivity, 3x3 rectangular mask).
//                                rippiNormL1    L1 norm (4-connectivity, 3x3 cross mask).
//    flag                     Flags
//                                IPP_SEGMENT_BORDER_4 Draw L1 segment borders.
//                                IPP_SEGMENT_BORDER_8 Draw Linf segment borders.
//    pBuffer                  Pointer to working buffer
*/

IPPAPI(IppStatus, ippiSegmentGradient_8u_C3IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pMarker,
                  int markerStep, IppiSize roiSize, IppiNorm norm, int flags, Ipp8u* pBuffer))


IPPAPI(IppStatus, ippiSegmentGradient_8u_C1IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pMarker,
                  int markerStep, IppiSize roiSize, IppiNorm norm, int flags, Ipp8u* pBuffer))


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiSegmentWatershedGetBufferSize_8u_C1R, ippiSegmentWatershedGetBufferSize_8u16u_C1R
//
//  Purpose:    Get size of external buffer.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image or kernel size are less or equal zero
//
//  Parameters:
//    roiSize                  The image ROI size
//    pBufferSize              The pointer to the buffer size
*/

IPPAPI(IppStatus, ippiSegmentWatershedGetBufferSize_8u_C1R, (IppiSize roiSize, int *pBufSize))

IPPAPI(IppStatus, ippiSegmentWatershedGetBufferSize_8u16u_C1R, (IppiSize roiSize, int *pBufSize))


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiSegmentWatershed_8u_C1IR,  ippiSegmentWatershed_8u16u_C1IR
//
//  Purpose:    Draw bounds between image segments
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image or kernel size are less or equal zero
//    ippStsStepErr            The steps in images are too small
//
//  Parameters:
//    pSrc                     Source image
//    srcStep                  Its step
//    pMarker                  Source and destination marker image
//    markerStep               Its step
//    roiSize                  Image ROI size
//    norm                     Type of norm to form the mask for maximum search:
//                                rippiNormInf   Infinity norm (8-connectivity, 3x3 rectangular mask).
//                                rippiNormL1    L1 norm (4-connectivity, 3x3 cross mask).
//                                rippiNormL2    approximation of L2 norm  (8-connectivity, 3x3 mask 11,15).
//                                rippiNormFM    Fast marching distance (4-connectivity).
//    flag                     Flags
//                                IPP_SEGMENT_QUEUE    Via priority queue.
//                                IPP_SEGMENT_DISTANCE Via distance transform.
//                                IPP_SEGMENT_BORDER_4 Draw L1 segment borders.
//                                IPP_SEGMENT_BORDER_8 Draw Linf segment borders.
//    pBuffer                  Pointer to working buffer
*/

IPPAPI(IppStatus, ippiSegmentWatershed_8u_C1IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pMarker, int markerStep, IppiSize roiSize,
                                                 IppiNorm norm, int flag, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiSegmentWatershed_8u16u_C1IR, (const Ipp8u* pSrc, int srcStep, Ipp16u* pMarker, int markerStep, IppiSize roiSize,
                                                 IppiNorm norm, int flag, Ipp8u* pBuffer))


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiBoundSegments_8u_C1IR,  ippiBoundSegments_16u_C1IR
//
//  Purpose:    Draw bounds between image segments
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image or kernel size are less or equal zero
//    ippStsStepErr            The steps in images are too small
//
//  Parameters:
//    pMarker                  Source and destination marker image
//    markerStep               Its step
//    roiSize                  Image ROI size
//    norm                     Type of norm to form the mask for maximum search:
//                                ippiNormInf   Infinity norm (8-connectivity, 3x3 rectangular mask).
//                                ippiNormL1    L1 norm (4-connectivity, 3x3 cross mask).
*/

IPPAPI(IppStatus, ippiBoundSegments_8u_C1IR, (Ipp8u* pMarker, int markerStep, IppiSize roiSize,
                                              Ipp8u val, IppiNorm norm))

IPPAPI(IppStatus, ippiBoundSegments_16u_C1IR, (Ipp16u* pMarker, int markerStep, IppiSize roiSize,
                                               Ipp16u val, IppiNorm norm))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:    ippiLabelMarkersGetBufferSize_8u_C1R, ippiLabelMarkersGetBufferSize_16u_C1R
//
//  Purpose: The functions calculate size of temporary buffer, required to run
//           marker labeling function
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//
//  Parameters:
//    roiSize                  ROI size
//    pBufSize                 Temporary buffer size
//F*/

IPPAPI(IppStatus, ippiLabelMarkersGetBufferSize_8u_C1R, (IppiSize roiSize, int* pBufSize))

IPPAPI(IppStatus, ippiLabelMarkersGetBufferSize_16u_C1R, (IppiSize roiSize, int* pBufSize))


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiLabelMarkers_8u_C1IR, ippiLabelMarkers_16u_C1IR
//
//  Purpose:    Labels connected non-zero components with different label values
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image or kernel size are less or equal zero
//    ippStsStepErr            The steps in images are too small
//
//  Parameters:
//    pMarker                  Source and destination marker image
//    markerStep               Its step
//    roiSize                  Image ROI size
//    minLabel                 Minimal label value > 0
//    maxLabel                 Maximal label value < 255
//    norm                     Type of norm to form the mask for maximum search:
//                                ippiNormL1    L1 norm (4-connectivity, 3x3 cross mask).
//                                ippiNormInf   Infinity norm (8-connectivity, 3x3 rectangular mask).
//    pNumber                  Pointer to number of markers
//    pBuffer                  Pointer to working buffer
*/

IPPAPI(IppStatus, ippiLabelMarkers_8u_C1IR, (Ipp8u* pMarker, int markerStep, IppiSize roiSize,
                         int minLabel, int maxLabel, IppiNorm norm, int *pNumber, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippiLabelMarkers_16u_C1IR, (Ipp16u* pMarker, int markerStep, IppiSize roiSize,
                         int minLabel, int maxLabel, IppiNorm norm, int *pNumber, Ipp8u* pBuffer))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippibFastArctan
//
//  Purpose:    Given "X" and "Y" images, calculates "angle" image
//              (i.e. atan(y/x)). Resultant angles are in degrees.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The length is less or equal zero
//
//  Arguments:
//    pSrcY                    Pointer to source "Y" image
//    pSrcX                    Pointer to source "X" image
//    pDst                     Pointer to "angle" image
//    length                   Vector length
//
//  Note:
//    For current version angle precision is ~0.1 degree
*/

IPPAPI(IppStatus, ippibFastArctan_32f, ( const Ipp32f*  pSrcY, const Ipp32f*  pSrcX,
                                         Ipp32f* pDst, int length ))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundHistogramInitAlloc
//
//  Purpose:    Allocates and initializes a state structure for
//              foreground/background segmentation using histograms
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsBadArgErr          Wrong value of pModel field
//
//  Arguments:
//    pSrc                     Source image
//    srcStep                  Step in source image
//    roiSize                  Source image ROI size.
//    pModel                   Pointer to the structure of the histogram statistical model.
//    pState                   Pointer to the pointer to the segmentation state structure.
*/

IPPAPI(IppStatus, ippiForegroundHistogramInitAlloc_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
                 IppiSize roiSize, IppFGHistogramModel* pModel, IppFGHistogramState_8u_C1R** pState))

IPPAPI(IppStatus, ippiForegroundHistogramInitAlloc_8u_C3R, ( const Ipp8u* pSrc, int srcStep,
                 IppiSize roiSize, IppFGHistogramModel* pModel, IppFGHistogramState_8u_C3R** pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundHistogramFree
//
//  Purpose:    Frees memory allocated for the foreground/background segmentation structure
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//
//  Arguments:
//    pState                   Pointer to the structure to be freed
*/

IPPAPI(IppStatus, ippiForegroundHistogramFree_8u_C1R, (IppFGHistogramState_8u_C1R* pState))

IPPAPI(IppStatus, ippiForegroundHistogramFree_8u_C3R, (IppFGHistogramState_8u_C3R* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundHistogram
//
//  Purpose:    Calculates foreground mask using histograms
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsStepErr            The steps in images are too small
//
//  Arguments:
//    pSrc                     Source image
//    srcStep                  Step in source image
//    pMask                    Pointer to mask
//    maskStep                 Step in the mask image
//    roiSize                  Source image ROI size.
//    pState                   Pointer to the pointer to the segmentation state structure.
*/

IPPAPI(IppStatus, ippiForegroundHistogram_8u_C1R, (const Ipp8u* pSrc, int srcStep,
       Ipp8u* pMask, int maskStep, IppiSize roiSize, IppFGHistogramState_8u_C1R* pState))

IPPAPI(IppStatus, ippiForegroundHistogram_8u_C3R, (const Ipp8u* pSrc, int srcStep,
       Ipp8u* pMask, int maskStep, IppiSize roiSize, IppFGHistogramState_8u_C3R* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundHistogramUpdate
//
//  Purpose:    Updates histogram statistical model for foreground segmentation
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsStepErr            The steps in images are too small
//
//  Arguments:
//    pSrc                     Source image
//    srcStep                  Step in source image
//    pMask                    Pointer to mask
//    maskStep                 Step in the mask image
//    pRef                     Pointer to reference image
//    refStep                  Step in the reference image
//    roiSize                  Source image ROI size.
//    pState                   Pointer to the pointer to the segmentation state structure.
*/

IPPAPI(IppStatus, ippiForegroundHistogramUpdate_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                  Ipp8u* pMask, int maskStep, Ipp8u* pRef, int refStep, IppiSize roiSize,
                  IppFGHistogramState_8u_C1R* pState))

IPPAPI(IppStatus, ippiForegroundHistogramUpdate_8u_C3R, (const Ipp8u* pSrc, int srcStep,
                  Ipp8u* pMask, int maskStep, Ipp8u* pRef, int refStep, IppiSize roiSize,
                  IppFGHistogramState_8u_C3R* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundGaussianInitAlloc
//
//  Purpose:    Allocates and initializes a state structure for
//              foreground/background segmentation using Gaussian mixtures
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsStepErr            The steps in images are too small
//
//  Arguments:
//    pSrc                     Source image
//    srcStep                  Step in source image
//    roiSize                  Source image ROI size.
//    pModel                   Pointer to the structure of the gaussian model.
//    pState                   Pointer to the pointer to the segmentation state structure.
*/

IPPAPI(IppStatus, ippiForegroundGaussianInitAlloc_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
                 IppiSize roiSize, IppFGGaussianModel* pModel, IppFGGaussianState_8u_C1R** pState))

IPPAPI(IppStatus, ippiForegroundGaussianInitAlloc_8u_C3R, ( const Ipp8u* pSrc, int srcStep,
                 IppiSize roiSize, IppFGGaussianModel* pModel, IppFGGaussianState_8u_C3R** pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundGaussianFree
//
//  Purpose:    Frees memory allocated for the foreground/background segmentation structure
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//
//  Arguments:
//    pState                   Pointer to the structure to be freed
*/

IPPAPI(IppStatus, ippiForegroundGaussianFree_8u_C1R, (IppFGGaussianState_8u_C1R* pState))

IPPAPI(IppStatus, ippiForegroundGaussianFree_8u_C3R, (IppFGGaussianState_8u_C3R* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundGaussian
//
//  Purpose:    Calculates foreground mask using Gaussians
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsStepErr            The steps in images are too small
//
//  Arguments:
//    pSrc                     Source image
//    srcStep                  Step in source image
//    pRef                     Pointer to reference image
//    refStep                  Step in the reference image
//    pDst                     Pointer to destination image
//    dstStep                  Step in the destination image
//    roiSize                  Source image ROI size.
//    pState                   Pointer to the pointer to the segmentation state structure.
*/

IPPAPI(IppStatus, ippiForegroundGaussian_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                  Ipp8u* pRef, int refStep, Ipp8u* pDst, int dstStep,
                  IppiSize roiSize, IppFGGaussianState_8u_C1R* pState))

IPPAPI(IppStatus, ippiForegroundGaussian_8u_C3R, (const Ipp8u* pSrc, int srcStep,
                  Ipp8u* pRef, int refStep, Ipp8u* pDst, int dstStep,
                  IppiSize roiSize, IppFGGaussianState_8u_C3R* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFilterGaussGetBufferSize
//
//  Purpose:    Computes the size of the working buffer for the Gaussian filter
//
//  Return:
//     ippStsNoErr      Ok. Any other value indicates an error or a warning.
//     ippStsNullPtrErr One of the pointers is NULL.
//     ippStsSizeErr    roiSize has a field with zero or negative value .
//     ippStsBadArgErr  KernelSize is odd or is less than 3.
//
//  Arguments:
//     roiSize          Maximal size of the image ROI in pixels.
//     kernelSize       Size of the Gaussian kernel (odd, greater or equal to 3).
//     pBufferSize      Pointer to the computed size of the buffer:
*/

IPPAPI(IppStatus, ippiFilterGaussGetBufferSize_32f_C1R,(IppiSize roiSize, int KernelSize,
                                                 int* pBufferSize))

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFilterGaussBorder
//
//  Purpose:    Applies Gaussian filter with borders
//
//  Return:
//     ippStsNoErr      Ok. Any other value indicates an error or a warning.
//     ippStsNullPtrErr One of the specified pointers is NULL.
//     ippStsSizeErr    roiSize has a field with zero or negative value.
//     ippStsStepErr    srcStep is less than roiSize.width * <pixelSize>,
//                              or dstStep is less than roiSize.width * <pixelSize>.
//     ippStsNotEvenStepErr One of the step values is not divisible by 4 for floating-point images.
//     ippStsBadArgErr  kernelSize is less than 3 or sigma is less or equal than 0.
//
//  Arguments:
//     pSrc             Pointer to the source image ROI.
//     srcStep          Distance in bytes between starts of consecutive lines in the source image.
//     pDst             Pointer to the destination image ROI.
//     dstStep          Distance in bytes between starts of consecutive lines in the destination image.
//     roiSize          Size of the source and destination image ROI.
//     kernelSize       Size of the Gaussian kernel (odd, greater or equal to 3).
//     sigma            Standard deviation of the Gaussian kernel.
//     pBuffer          Pointer to the working buffer.
*/

IPPAPI(IppStatus, ippiFilterGaussBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,Ipp32f* pDst, int dstStep,
                                           IppiSize roiSize,int KernelSize,Ipp32f sigma,
                                           IppiBorderType borderType, Ipp32f borderValue,
                                           Ipp8u* pBuffer))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFindPeaks3x3GetBufferSize
//
//  Purpose:    Computes the size of the working buffer for the peak search
//
//  Return:
//     ippStsNoErr      Ok. Any other value indicates an error or a warning.
//     ippStsNullPtrErr Indicates an error condition if the pointer pBufferSize is NULL.
//     ippStsSizeErr    Indicates an error condition if maskSize has a field with zero
//                      or negative value, or if roiWidth is less than 1.
//
//  Arguments:
//     roiWidth         Maximal image width (in pixels).
//     pBufferSize      Pointer to the computed size of the buffer.
*/

IPPAPI(IppStatus, ippiFindPeaks3x3GetBufferSize_32f_C1R,(int roiWidth, int* pBufferSize))

IPPAPI(IppStatus, ippiFindPeaks3x3GetBufferSize_32s_C1R,(int roiWidth, int* pBufferSize))

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFindPeaks3x3
//
//  Purpose:    Finds coordinates of peaks (maximums or minimums) with absolute value
//              exceeding threshold value
//
//  Return:
//     ippStsNoErr      Ok. Any other value indicates an error or a warning.
//     ippStsNullPtrErr One of the specified pointers is NULL.
//     ippStsSizeErr    Indicates an error condition if roiSize has a field
//                      with zero or negative value or if maxPeakCount is less
//                      than or equal to zero, or if Border is less than 1 or
//                      greater than 0.5* roiSize.width or. greater than 0.5* roiSize.hight
//     ippStsStepErr    Indicates an error condition if srcStep is less than roiSize.width * <pixelSize>
//     ippStsNotEvenStepErr Indicates an error condition if one of the step values is
//                      not divisible by 4 for floating-point or 32-bit integer images.
//     ippStsBadArgErr  Indicates an error condition if norm value is wrong.
//
//  Arguments:
//     pSrc             Pointer to the source image ROI.
//     srcStep          Distance in bytes between starts of consecutive lines in the source image.
//     roiSize          Size of the image ROI in pixels.
//     threshold        Threshold value.
//     pPeak            Pointer to the maximum coordinates [maxPeakCount].
//     maxPeakCount     Maximal number of peaks.
//     pPeakCount       Number of detected peaks.
//     norm             Type of norm to form the mask for maximum search:
//                      ippiNormInf Infinity norm (8-connectivity, 3x3 rectangular mask).
//                      ippiNormL1 L1 norm (4-connectivity, 3x3 cross mask).
//     Border           Border value, only pixel with distance from the edge of the image
//                      greater than Border are processed.
//     pBuffer          Pointer to the working buffer.
*/

IPPAPI(IppStatus, ippiFindPeaks3x3_32f_C1R,(const Ipp32f* pSrc, int srcStep, IppiSize roiSize,
                                   Ipp32f threshold, IppiPoint* pPeak, int maxPeakCount,
                                   int* pPeakCount, IppiNorm norm, int Border, Ipp8u* pBuffer))


IPPAPI(IppStatus, ippiFindPeaks3x3_32s_C1R,(const Ipp32s* pSrc, int srcStep, IppiSize roiSize,
                                   Ipp32s threshold, IppiPoint* pPeak, int maxPeakCount,
                                   int* pPeakCount, IppiNorm norm, int Border, Ipp8u* pBuffer))

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:      ippiHoughLineGetSize_8u_C1R()
//
//
//  Purpose:   Calculate the size of temporary buffer for ippiHoughLine_8u32f_C1R function.
//
//  Arguments:
//    roiSize                  The size of source ROI.
//    delta                    Discretization step, delta.rho - distance resolution in pixels, delta.theta - angle resolution in radians
//    maxLineCount             The size of detected line buffer
//    pBufSize                 Pointer to the computed size of the temporary buffer
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         pBufSize is NULL
//    ippStsSizeErr            The roiSize or delta has a field with zero or negative value
//    ippStsOverflow           The size of buffer too big. Overflow.
//
*/

IPPAPI(IppStatus, ippiHoughLineGetSize_8u_C1R, (IppiSize roiSize, IppPointPolar delta, int maxLineCount, int* pBufSize))

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:      ippiHoughLine_8u32f_C1R()
//
//
//  Purpose:   Perform Hough transform algorithm
//
//
//  Arguments:
//    pSrc                     Pointer to the source image ROI
//    srcStep                  Distance in bytes between starts of consecutive lines in the source image
//    roiSize                  The size of source ROI.
//    delta                    Discretization step, delta.rho - distance resolution in pixels, delta.theta - angle resolution in radians
//    threshold                Threshold for a line is detected (if accumulator value > threshold)
//    pLine                    Pointer to output array of detected lines
//    maxLineCount             Size of output array pLine in elements
//    pLineCount               Number of detected lines.  If founded more than maxLineCount lines than function returns "ippStsDstSizeLessExpected" status
//    pBuffer                  Pointer to the pre-allocated temporary buffer
//
//  Return:
//    ippStsNoErr               Ok
//    ippStsNullPtrErr          pSrc or pLine or pLineCount or pBuffer is NULL
//    ippStsStepErr             srcStep is not valid
//    ippStsSizeErr             roiSize has a field with zero or negative value or maxLineCount is zero or negative
//    ippStsBadArgErr           threshold or is less than or equal to zero or delta.rho less 0 or more ROI width+height or delta.theta less 0 or more PI
//    ippStsDstSizeLessExpected Ok, but lines detected more than maxLineCount
//
*/

IPPAPI(IppStatus, ippiHoughLine_8u32f_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, IppPointPolar delta, int threshold,
       IppPointPolar* pLine, int maxLineCount, int* pLineCount, Ipp8u* pBuffer))

/*
//  Name:      ippiHoughLine_Region_8u32f_C1R
//
//
//  Purpose:   Perform Hough transform algorithm in defined region
//
//
//  Arguments:
//    pSrc                     Pointer to the source image ROI
//    srcStep                  Distance in bytes between starts of consecutive lines in the source image
//    roiSize                  The size of source ROI.
//    pLine                    Pointer to output array of detected lines
//    dstRoi                   Bottom left and top right corners of searched lines. All founded lines must be
//                             in this area. (line[n].rho>=dstRoi[0].rho && line[n].theta>=dstRoi[0].theta &&
//                                            line[n].rho<=dstRoi[1].rho && line[n].theta<=dstRoi[1].theta)
//    maxLineCount             Size of output array pLine in elements
//    pLineCount               Number of detected lines.  If founded more than maxLineCount lines than function returns "ippStsDstSizeLessExpected" status
//    delta                    Discretization step, delta.rho - distance resolution in pixels, delta.theta - angle resolution in radians
//    threshold                Threshold for a line is detected (if accumulator value > threshold)
//    pBuffer                  Pointer to the pre-allocated temporary buffer
//
//  Return:
//    ippStsNoErr               Ok
//    ippStsNullPtrErr          pSrc or pLine or pLineCount or pBuffer is NULL
//    ippStsStepErr             srcStep is not valid
//    ippStsSizeErr             roiSize has a field with zero or negative value or maxLineCount is zero or negative
//    ippStsBadArgErr           threshold or is less than or equal to zero or delta.rho less 0 or more ROI width+height or
//                              delta.theta less 0 or more PI or dstRoi[0].rho more dstRoi[1].rho or dstRoi[0].theta more dstRoi[1].theta
//    ippStsDstSizeLessExpected Ok, but lines detected more than maxLineCount
//
*/

IPPAPI(IppStatus, ippiHoughLine_Region_8u32f_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize,
       IppPointPolar* pLine, IppPointPolar dstRoi[2], int maxLineCount, int* pLineCount, IppPointPolar delta, int threshold, Ipp8u* pBuffer))


#if !defined( _OWN_BLDPCS )
typedef struct _IppiSRHNSpec_PSF3x3 IppiSRHNSpec_PSF3x3;
typedef struct _IppiSRHNSpec_PSF2x2 IppiSRHNSpec_PSF2x2;
#endif

/* ///////////////////////////////////////////////////////////////////////////
//  Name:    ippiSRHNInitAlloc_PSF3x3,
//           ippiSRHNInitAlloc_PSF2x2
//
//  Purpose: Allocate and initialize the internal PSF structures.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Some of the pointers are NULL
//    ippStsSizeErr            The table size is incorrect
//  Arguments:
//    ppPSF                    Double pointer to the created PSF structure
//    pcTab                    Input table with coefficients
//    tabSize                  The number of elements in cTab
*/

IPPAPI(IppStatus, ippiSRHNInitAlloc_PSF3x3, (IppiSRHNSpec_PSF3x3** ppPSF,
   const Ipp32f pcTab[][9], int tabSize))

IPPAPI(IppStatus, ippiSRHNInitAlloc_PSF2x2, (IppiSRHNSpec_PSF2x2** ppPSF,
   const Ipp32f pcTab[][4], int tabSize))


/* ///////////////////////////////////////////////////////////////////////////
//  Name:    ippiSRHNFree_PSF3x3,
//           ippiSRHNFree_PSF2x2
//
//  Purpose: Deallocate PSF structures
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         The PSF pointer is NULL
//  Arguments:
//    pPSF                     Pointer to the PSF structure
*/

IPPAPI(IppStatus, ippiSRHNFree_PSF3x3, (IppiSRHNSpec_PSF3x3* pPSF))
IPPAPI(IppStatus, ippiSRHNFree_PSF2x2, (IppiSRHNSpec_PSF2x2* pPSF))


/* ///////////////////////////////////////////////////////////////////////////
//  Name:    ippiSRHNCalcResidual_PSF3x3_8u32f_C1R,
//           ippiSRHNCalcResidual_PSF2x2_8u32f_C1R,
//           ippiSRHNCalcResidual_PSF3x3_16u32f_C1R,
//           ippiSRHNCalcResidual_PSF2x2_16u32f_C1R,
//
//  Purpose: Compute residuals for likelihood part of target function
//           or its gradient.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Some of the pointers are NULL's
//    ippStsSizeErr            The length is negative
//    ippStsStepErr            hiResStep is too small or
//                             is not divisible by sizeof(pHiRes[0])
//
//  Arguments:
//    pHiRes                   Pointer to high-resolution image
//    pHiResStep               High-resolution image step
//    pLowRes                  Array of pixel values from low-resolution images
//    pOfs                     Array of offsets of 3x3 areas
//                             inside high-resolution image
//    pCoeff                   Array of indices in cTab
//    pResidual                The output array
//    len                      Length of lowRes, ofs, coeff and residual arrays
//    pPSF                     Previously created PSF table for 3x3 or 2x2 case
*/

IPPAPI(IppStatus, ippiSRHNCalcResidual_PSF3x3_8u32f_C1R, (
    const Ipp32f* pHiRes, int hiResStep, const Ipp8u* pLowRes,
    const Ipp32s* pOfs, const Ipp16u* pCoeff, Ipp32f* pResidual,
    int len, const IppiSRHNSpec_PSF3x3* pPSF))

IPPAPI(IppStatus, ippiSRHNCalcResidual_PSF2x2_8u32f_C1R, (
    const Ipp32f* pHiRes, int hiResStep, const Ipp8u* pLowRes,
    const Ipp32s* pOfs, const Ipp16u* pCoeff, Ipp32f* pResidual,
    int len, const IppiSRHNSpec_PSF2x2* pPSF))

IPPAPI(IppStatus, ippiSRHNCalcResidual_PSF3x3_16u32f_C1R, (
    const Ipp32f* pHiRes, int hiResStep, const Ipp16u* pLowRes,
    const Ipp32s* pOfs, const Ipp16u* pCoeff, Ipp32f* pResidual,
    int len, const IppiSRHNSpec_PSF3x3* pPSF))

IPPAPI(IppStatus, ippiSRHNCalcResidual_PSF2x2_16u32f_C1R, (
    const Ipp32f* pHiRes, int hiResStep, const Ipp16u* pLowRes,
    const Ipp32s* pOfs, const Ipp16u* pCoeff, Ipp32f* pResidual,
    int len, const IppiSRHNSpec_PSF2x2* pPSF))


/* ///////////////////////////////////////////////////////////////////////////
//  Name:    ippiSRHNUpdateGradient_PSF3x3_32f_C1R,
//           ippiSRHNUpdateGradient_PSF2x2_32f_C1R
//
//  Purpose: Update likelihood part of the gradient
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Some of the pointers are NULL's
//    ippStsSizeErr            Length is negative
//    ippStsStepErr            gradStep is too small or is not divisible by
//                             sizeof(pGrad[0])
//
//  Arguments:
//    pGrad                    Pointer to gradient
//    gradStep                 The gradient step
//    pOfs                     Array of offsets of 3x3 or 2x2 areas
//                             inside high-resolution image
//    pCoeff                   Array of indices in cTab
//    pRob                     Array of computed robust function derivatives
//    pWeight                  Array of indices in wTab
//    len                      Length of ofs, coeff, errVal and weight arrays
//    pwTab                    A table of confidence weights
//    pPSF                     PSF structure
*/

IPPAPI(IppStatus, ippiSRHNUpdateGradient_PSF3x3_32f_C1R, (
    Ipp32f* pGrad, int gradStep, const Ipp32s* pOfs, const Ipp16u* pCoeff,
    const Ipp32f* pRob, const Ipp8u* pWeight, int len, const Ipp32f* pwTab,
    const IppiSRHNSpec_PSF3x3* pPSF))

IPPAPI(IppStatus, ippiSRHNUpdateGradient_PSF2x2_32f_C1R, (
    Ipp32f* pGrad, int gradStep, const Ipp32s* pOfs, const Ipp16u* pCoeff,
    const Ipp32f* pRob, const Ipp8u* pWeight, int len, const Ipp32f* pwTab,
    const IppiSRHNSpec_PSF2x2* pPSF))


#if defined __cplusplus
}
#endif

#if defined (_IPP_STDCALL_CDECL)
  #undef  _IPP_STDCALL_CDECL
  #define __stdcall __cdecl
#endif

#endif /* __IPPCV_H__ */
/* ////////////////////////// End of file "ippCV.h" ////////////////////////// */
