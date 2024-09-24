/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//       Copyright (c) 1999-2009 Intel Corporation. All Rights Reserved.
//
//                Intel(R) Integrated Performance Primitives
//                           Image Processing
//
*/

#if !defined( __IPPI_H__ ) || defined( _OWN_BLDPCS )
#define __IPPI_H__

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
    ippAlphaOver,
    ippAlphaIn,
    ippAlphaOut,
    ippAlphaATop,
    ippAlphaXor,
    ippAlphaPlus,
    ippAlphaOverPremul,
    ippAlphaInPremul,
    ippAlphaOutPremul,
    ippAlphaATopPremul,
    ippAlphaXorPremul,
    ippAlphaPlusPremul
} IppiAlphaType;

struct DeconvFFTState_32f_C1R;
typedef struct DeconvFFTState_32f_C1R IppiDeconvFFTState_32f_C1R;

struct DeconvFFTState_32f_C3R;
typedef struct DeconvFFTState_32f_C3R IppiDeconvFFTState_32f_C3R;

struct DeconvLR_32f_C1R;
typedef struct DeconvLR_32f_C1R IppiDeconvLR_32f_C1R;

struct DeconvLR_32f_C3R;
typedef struct DeconvLR_32f_C3R IppiDeconvLR_32f_C3R;

typedef enum {
    ippiFilterBilateralGauss = 100,
    ippiFilterBilateralGaussFast = 101
} IppiFilterBilateralType;

struct FilterBilateralSpec;
typedef struct FilterBilateralSpec IppiFilterBilateralSpec;

typedef enum {
    ippResizeFilterHann,
    ippResizeFilterLanczos
} IppiResizeFilterType;

struct ResizeFilterState;
typedef struct ResizeFilterState IppiResizeFilterState;
#endif /* _OWN_BLDPCS */




/* /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                   Functions declarations
////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////// */


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiGetLibVersion
//  Purpose:    gets the version of the library
//  Returns:    structure containing information about the current version of
//  the Intel IPP library for image processing
//  Parameters:
//
//  Notes:      there is no need to release the returned structure
*/
IPPAPI( const IppLibraryVersion*, ippiGetLibVersion, (void) )


/* /////////////////////////////////////////////////////////////////////////////
//                   Memory Allocation Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiMalloc
//  Purpose:    allocates memory with 32-byte aligned pointer for ippIP images,
//              every line of the image is aligned due to the padding characterized
//              by pStepBytes
//  Parameter:
//    widthPixels   width of image in pixels
//    heightPixels  height of image in pixels
//    pStepBytes    pointer to the image step, it is an output parameter
//                  calculated by the function
//
//  Returns:    pointer to the allocated memory or NULL if out of memory or wrong parameters
//  Notes:      free the allocated memory using the function ippiFree only
*/

IPPAPI( Ipp8u*,   ippiMalloc_8u_C1,    ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp16u*,  ippiMalloc_16u_C1,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp16s*,  ippiMalloc_16s_C1,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32s*,  ippiMalloc_32s_C1,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32f*,  ippiMalloc_32f_C1,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32sc*, ippiMalloc_32sc_C1,  ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32fc*, ippiMalloc_32fc_C1,  ( int widthPixels, int heightPixels, int* pStepBytes ) )

IPPAPI( Ipp8u*,   ippiMalloc_8u_C2,    ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp16u*,  ippiMalloc_16u_C2,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp16s*,  ippiMalloc_16s_C2,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32s*,  ippiMalloc_32s_C2,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32f*,  ippiMalloc_32f_C2,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32sc*, ippiMalloc_32sc_C2,  ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32fc*, ippiMalloc_32fc_C2,  ( int widthPixels, int heightPixels, int* pStepBytes ) )

IPPAPI( Ipp8u*,   ippiMalloc_8u_C3,    ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp16u*,  ippiMalloc_16u_C3,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp16s*,  ippiMalloc_16s_C3,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32s*,  ippiMalloc_32s_C3,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32f*,  ippiMalloc_32f_C3,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32sc*, ippiMalloc_32sc_C3,  ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32fc*, ippiMalloc_32fc_C3,  ( int widthPixels, int heightPixels, int* pStepBytes ) )

IPPAPI( Ipp8u*,   ippiMalloc_8u_C4,    ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp16u*,  ippiMalloc_16u_C4,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp16s*,  ippiMalloc_16s_C4,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32s*,  ippiMalloc_32s_C4,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32f*,  ippiMalloc_32f_C4,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32sc*, ippiMalloc_32sc_C4,  ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32fc*, ippiMalloc_32fc_C4,  ( int widthPixels, int heightPixels, int* pStepBytes ) )

IPPAPI( Ipp8u*,   ippiMalloc_8u_AC4,   ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp16u*,  ippiMalloc_16u_AC4,  ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp16s*,  ippiMalloc_16s_AC4,  ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32s*,  ippiMalloc_32s_AC4,  ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32f*,  ippiMalloc_32f_AC4,  ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32sc*, ippiMalloc_32sc_AC4, ( int widthPixels, int heightPixels, int* pStepBytes ) )
IPPAPI( Ipp32fc*, ippiMalloc_32fc_AC4, ( int widthPixels, int heightPixels, int* pStepBytes ) )


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFree
//  Purpose:    frees memory allocated by the ippiMalloc functions
//  Parameter:
//    ptr       pointer to the memory allocated by the ippiMalloc functions
//
//  Notes:      use this function to free memory allocated by ippiMalloc
*/
IPPAPI( void, ippiFree, (void* ptr) )


/* ///////////////////////////////////////////////////////////////////////////////////////
//                  Arithmetic Functions
///////////////////////////////////////////////////////////////////////////// */
/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiAdd_8u_C1RSfs,  ippiAdd_8u_C3RSfs,  ippiAdd_8u_C4RSfs,  ippiAdd_8u_AC4RSfs,
//         ippiAdd_16s_C1RSfs, ippiAdd_16s_C3RSfs, ippiAdd_16s_C4RSfs, ippiAdd_16s_AC4RSfs,
//         ippiAdd_16u_C1RSfs, ippiAdd_16u_C3RSfs, ippiAdd_16u_C4RSfs, ippiAdd_16u_AC4RSfs,
//         ippiSub_8u_C1RSfs,  ippiSub_8u_C3RSfs,  ippiSub_8u_C4RSfs,  ippiSub_8u_AC4RSfs,
//         ippiSub_16s_C1RSfs, ippiSub_16s_C3RSfs, ippiSub_16s_C4RSfs, ippiSub_16s_AC4RSfs,
//         ippiSub_16u_C1RSfs, ippiSub_16u_C3RSfs, ippiSub_16u_C4RSfs, ippiSub_16u_AC4RSfs,
//         ippiMul_8u_C1RSfs,  ippiMul_8u_C3RSfs,  ippiMul_8u_C4RSfs,  ippiMul_8u_AC4RSfs,
//         ippiMul_16s_C1RSfs, ippiMul_16s_C3RSfs, ippiMul_16s_C4RSfs, ippiMul_16s_AC4RSfs
//         ippiMul_16u_C1RSfs, ippiMul_16u_C3RSfs, ippiMul_16u_C4RSfs, ippiMul_16u_AC4RSfs
//
//  Purpose:    Adds, subtracts, or multiplies pixel values of two
//              source images and places the scaled result in the destination image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            Width or height of images is less than or equal to zero
//
//  Parameters:
//    pSrc1, pSrc2             Pointers to the source images
//    src1Step, src2Step       Steps through the source images
//    pDst                     Pointer to the destination image
//    dstStep                  Step through the destination image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiAdd_8u_C1RSfs,   (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2,
                                        int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiAdd_8u_C3RSfs,   (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2,
                                        int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiAdd_8u_C4RSfs,   (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2,
                                        int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiAdd_8u_AC4RSfs,  (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2,
                                        int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16s_C1RSfs,  (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2,
                                        int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16s_C3RSfs,  (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2,
                                        int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16s_C4RSfs,  (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2,
                                        int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16s_AC4RSfs, (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2,
                                        int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16u_C1RSfs,  (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2,
                                        int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16u_C3RSfs,  (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2,
                                        int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16u_C4RSfs,  (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2,
                                        int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16u_AC4RSfs, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2,
                                        int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiSub_8u_C1RSfs,   (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2,
                                        int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiSub_8u_C3RSfs,   (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2,
                                        int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiSub_8u_C4RSfs,   (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2,
                                        int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiSub_8u_AC4RSfs,  (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2,
                                        int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiSub_16s_C1RSfs,  (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2,
                                        int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiSub_16s_C3RSfs,  (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2,
                                        int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiSub_16s_C4RSfs,  (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2,
                                        int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiSub_16s_AC4RSfs, (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2,
                                        int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiSub_16u_C1RSfs,  (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2,
                                        int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiSub_16u_C3RSfs,  (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2,
                                        int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiSub_16u_C4RSfs,  (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2,
                                        int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiSub_16u_AC4RSfs, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2,
                                        int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiMul_8u_C1RSfs,   (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2,
                                        int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiMul_8u_C3RSfs,   (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2,
                                        int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiMul_8u_C4RSfs,   (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2,
                                        int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiMul_8u_AC4RSfs,  (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2,
                                        int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiMul_16s_C1RSfs,  (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2,
                                        int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiMul_16s_C3RSfs,  (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2,
                                        int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiMul_16s_C4RSfs,  (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2,
                                        int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiMul_16s_AC4RSfs, (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2,
                                        int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiMul_16u_C1RSfs,  (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2,
                                        int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiMul_16u_C3RSfs,  (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2,
                                        int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiMul_16u_C4RSfs,  (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2,
                                        int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))
IPPAPI(IppStatus, ippiMul_16u_AC4RSfs, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2,
                                        int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                        int scaleFactor))

/* //////////////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiAddC_8u_C1IRSfs,  ippiAddC_8u_C3IRSfs,  ippiAddC_8u_C4IRSfs,   ippiAddC_8u_AC4IRSfs,
//        ippiAddC_16s_C1IRSfs, ippiAddC_16s_C3IRSfs, ippiAddC_16s_C4IRSfs,  ippiAddC_16s_AC4IRSfs,
//        ippiAddC_16u_C1IRSfs, ippiAddC_16u_C3IRSfs, ippiAddC_16u_C4IRSfs,  ippiAddC_16u_AC4IRSfs,
//        ippiSubC_8u_C1IRSfs,  ippiSubC_8u_C3IRSfs,  ippiSubC_8u_C4IRSfs,   ippiSubC_8u_AC4IRSfs,
//        ippiSubC_16s_C1IRSfs, ippiSubC_16s_C3IRSfs, ippiSubC_16s_C4IRSfs,  ippiSubC_16s_AC4IRSfs,
//        ippiSubC_16u_C1IRSfs, ippiSubC_16u_C3IRSfs, ippiSubC_16u_C4IRSfs,  ippiSubC_16u_AC4IRSfs,
//        ippiMulC_8u_C1IRSfs,  ippiMulC_8u_C3IRSfs,  ippiMulC_8u_C4IRSfs,   ippiMulC_8u_AC4IRSfs,
//        ippiMulC_16s_C1IRSfs, ippiMulC_16s_C3IRSfs, ippiMulC_16s_C4IRSfs,  ippiMulC_16s_AC4IRSfs
//        ippiMulC_16u_C1IRSfs, ippiMulC_16u_C3IRSfs, ippiMulC_16u_C4IRSfs,  ippiMulC_16u_AC4IRSfs
//
//  Purpose:    Adds, subtracts, or multiplies pixel values of an image and a constant
//              and places the scaled results in the same image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         Pointer is NULL
//    ippStsSizeErr            Width or height of an image is less than or equal to zero
//
//  Parameters:
//    value                    Constant value (constant vector for multi-channel images)
//    pSrcDst                  Pointer to the image
//    srcDstStep               Step through the image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiAddC_8u_C1IRSfs,   (Ipp8u value, Ipp8u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_8u_C3IRSfs,   (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_8u_C4IRSfs,   (const Ipp8u value[4], Ipp8u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_8u_AC4IRSfs,  (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16s_C1IRSfs,  (Ipp16s value, Ipp16s* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16s_C3IRSfs,  (const Ipp16s value[3], Ipp16s* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16s_C4IRSfs,  (const Ipp16s value[4], Ipp16s* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16s_AC4IRSfs, (const Ipp16s value[3], Ipp16s* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16u_C1IRSfs,  (Ipp16u value, Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16u_C3IRSfs,  (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16u_C4IRSfs,  (const Ipp16u value[4], Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16u_AC4IRSfs, (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_8u_C1IRSfs,   (Ipp8u value, Ipp8u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_8u_C3IRSfs,   (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_8u_C4IRSfs,   (const Ipp8u value[4], Ipp8u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_8u_AC4IRSfs,  (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16s_C1IRSfs,  (Ipp16s value, Ipp16s* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16s_C3IRSfs,  (const Ipp16s value[3], Ipp16s* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16s_C4IRSfs,  (const Ipp16s value[4], Ipp16s* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16s_AC4IRSfs, (const Ipp16s value[3], Ipp16s* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16u_C1IRSfs,  (Ipp16u value, Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16u_C3IRSfs,  (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16u_C4IRSfs,  (const Ipp16u value[4], Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16u_AC4IRSfs, (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_8u_C1IRSfs,   (Ipp8u value, Ipp8u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_8u_C3IRSfs,   (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_8u_C4IRSfs,   (const Ipp8u value[4], Ipp8u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_8u_AC4IRSfs,  (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16s_C1IRSfs,  (Ipp16s value, Ipp16s* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16s_C3IRSfs,  (const Ipp16s value[3], Ipp16s* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16s_C4IRSfs,  (const Ipp16s value[4], Ipp16s* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16s_AC4IRSfs, (const Ipp16s value[3], Ipp16s* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16u_C1IRSfs,  (Ipp16u value, Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16u_C3IRSfs,  (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16u_C4IRSfs,  (const Ipp16u value[4], Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16u_AC4IRSfs, (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiAddC_8u_C1RSfs,  ippiAddC_8u_C3RSfs,  ippiAddC_8u_C4RSfs   ippiAddC_8u_AC4RSfs,
//        ippiAddC_16s_C1RSfs, ippiAddC_16s_C3RSfs, ippiAddC_16s_C4RSfs, ippiAddC_16s_AC4RSfs,
//        ippiAddC_16u_C1RSfs, ippiAddC_16u_C3RSfs, ippiAddC_16u_C4RSfs, ippiAddC_16u_AC4RSfs,
//        ippiSubC_8u_C1RSfs,  ippiSubC_8u_C3RSfs,  ippiSubC_8u_C4RSfs,  ippiSubC_8u_AC4RSfs,
//        ippiSubC_16s_C1RSfs, ippiSubC_16s_C3RSfs, ippiSubC_16s_C4RSfs, ippiSubC_16s_AC4RSfs,
//        ippiSubC_16u_C1RSfs, ippiSubC_16u_C3RSfs, ippiSubC_16u_C4RSfs, ippiSubC_16u_AC4RSfs,
//        ippiMulC_8u_C1RSfs,  ippiMulC_8u_C3RSfs,  ippiMulC_8u_C4RSfs,  ippiMulC_8u_AC4RSfs,
//        ippiMulC_16s_C1RSfs, ippiMulC_16s_C3RSfs, ippiMulC_16s_C4RSfs, ippiMulC_16s_AC4RSfs
//        ippiMulC_16u_C1RSfs, ippiMulC_16u_C3RSfs, ippiMulC_16u_C4RSfs, ippiMulC_16u_AC4RSfs
//
//  Purpose:    Adds, subtracts, or multiplies pixel values of a source image
//              and a constant, and places the scaled results in the destination image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            Width or height of images is less than or equal to zero
//
//  Parameters:
//    value                    Constant value (constant vector for multi-channel images)
//    pSrc                     Pointer to the source image
//    srcStep                  Step through the source image
//    pDst                     Pointer to the destination image
//    dstStep                  Step through the destination image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiAddC_8u_C1RSfs,   (const Ipp8u* pSrc, int srcStep, Ipp8u value, Ipp8u* pDst,
                                         int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_8u_C3RSfs,   (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3],
                                         Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiAddC_8u_C4RSfs,   (const Ipp8u* pSrc, int srcStep, const Ipp8u value[4],
                                         Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiAddC_8u_AC4RSfs,  (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3],
                                         Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16s_C1RSfs,  (const Ipp16s* pSrc, int srcStep, Ipp16s value, Ipp16s* pDst,
                                         int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16s_C3RSfs,  (const Ipp16s* pSrc, int srcStep, const Ipp16s value[3],
                                         Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16s_C4RSfs,  (const Ipp16s* pSrc, int srcStep, const Ipp16s value[4],
                                         Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16s_AC4RSfs, (const Ipp16s* pSrc, int srcStep, const Ipp16s value[3],
                                         Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16u_C1RSfs,  (const Ipp16u* pSrc, int srcStep, Ipp16u value, Ipp16u* pDst,
                                         int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16u_C3RSfs,  (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3],
                                         Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16u_C4RSfs,  (const Ipp16u* pSrc, int srcStep, const Ipp16u value[4],
                                         Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16u_AC4RSfs, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3],
                                         Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiSubC_8u_C1RSfs,   (const Ipp8u* pSrc, int srcStep, Ipp8u value, Ipp8u* pDst,
                                         int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_8u_C3RSfs,   (const Ipp8u* pSrc, int srcStep ,const Ipp8u value[3],
                                         Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiSubC_8u_C4RSfs,   (const Ipp8u* pSrc, int srcStep, const Ipp8u value[4],
                                         Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiSubC_8u_AC4RSfs,  (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3],
                                         Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16s_C1RSfs,  (const Ipp16s* pSrc, int srcStep, Ipp16s value, Ipp16s* pDst,
                                         int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16s_C3RSfs,  (const Ipp16s* pSrc, int srcStep, const Ipp16s value[3],
                                         Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16s_C4RSfs,  (const Ipp16s* pSrc, int srcStep, const Ipp16s value[4],
                                         Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16s_AC4RSfs, (const Ipp16s* pSrc, int srcStep, const Ipp16s value[3],
                                         Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16u_C1RSfs,  (const Ipp16u* pSrc, int srcStep, Ipp16u value, Ipp16u* pDst,
                                         int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16u_C3RSfs,  (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3],
                                         Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16u_C4RSfs,  (const Ipp16u* pSrc, int srcStep, const Ipp16u value[4],
                                         Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16u_AC4RSfs, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3],
                                         Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiMulC_8u_C1RSfs,   (const Ipp8u* pSrc, int srcStep, Ipp8u value, Ipp8u* pDst,
                                         int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_8u_C3RSfs,   (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3],
                                         Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiMulC_8u_C4RSfs,   (const Ipp8u* pSrc, int srcStep, const Ipp8u value[4],
                                         Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiMulC_8u_AC4RSfs,  (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3],
                                         Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16s_C1RSfs,  (const Ipp16s* pSrc, int srcStep, Ipp16s value, Ipp16s* pDst,
                                         int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16s_C3RSfs,  (const Ipp16s* pSrc, int srcStep, const Ipp16s value[3],
                                         Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16s_C4RSfs,  (const Ipp16s* pSrc, int srcStep, const Ipp16s value[4],
                                         Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16s_AC4RSfs, (const Ipp16s* pSrc, int srcStep, const Ipp16s value[3],
                                         Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16u_C1RSfs,  (const Ipp16u* pSrc, int srcStep, Ipp16u value, Ipp16u* pDst,
                                         int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16u_C3RSfs,  (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3],
                                         Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16u_C4RSfs,  (const Ipp16u* pSrc, int srcStep, const Ipp16u value[4],
                                         Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16u_AC4RSfs, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3],
                                         Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                         int scaleFactor))

/* ////////////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiAdd_8u_C1IRSfs,  ippiAdd_8u_C3IRSfs,  ippiAdd_8u_C4IRSfs,  ippiAdd_8u_AC4IRSfs,
//        ippiAdd_16s_C1IRSfs, ippiAdd_16s_C3IRSfs, ippiAdd_16s_C4IRSfs, ippiAdd_16s_AC4IRSfs,
//        ippiAdd_16u_C1IRSfs, ippiAdd_16u_C3IRSfs, ippiAdd_16u_C4IRSfs, ippiAdd_16u_AC4IRSfs,
//        ippiSub_8u_C1IRSfs,  ippiSub_8u_C3IRSfs,  ippiSub_8u_C4IRSfs,  ippiSub_8u_AC4IRSfs,
//        ippiSub_16s_C1IRSfs, ippiSub_16s_C3IRSfs, ippiSub_16s_C4IRSfs  ippiSub_16s_AC4IRSfs,
//        ippiSub_16u_C1IRSfs, ippiSub_16u_C3IRSfs, ippiSub_16u_C4IRSfs  ippiSub_16u_AC4IRSfs,
//        ippiMul_8u_C1IRSfs,  ippiMul_8u_C3IRSfs,  ippiMul_8u_C4IRSfs,  ippiMul_8u_AC4IRSfs,
//        ippiMul_16s_C1IRSfs, ippiMul_16s_C3IRSfs, ippiMul_16s_C4IRSfs, ippiMul_16s_AC4IRSfs
//        ippiMul_16u_C1IRSfs, ippiMul_16u_C3IRSfs, ippiMul_16u_C4IRSfs, ippiMul_16u_AC4IRSfs
//
//  Purpose:    Adds, subtracts, or multiplies pixel values of two source images
//              and places the scaled results in the first source image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            Width or height of images is less than or equal to zero
//
//  Parameters:
//    pSrc                     Pointer to the second source image
//    srcStep                  Step through the second source image
//    pSrcDst                  Pointer to the first source/destination image
//    srcDstStep               Step through the first source/destination image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiAdd_8u_C1IRSfs,   (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus, ippiAdd_8u_C3IRSfs,   (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_8u_C4IRSfs,   (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_8u_AC4IRSfs,  (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16s_C1IRSfs,  (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16s_C3IRSfs,  (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16s_C4IRSfs,  (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16s_AC4IRSfs, (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16u_C1IRSfs,  (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16u_C3IRSfs,  (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16u_C4IRSfs,  (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16u_AC4IRSfs, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_8u_C1IRSfs,   (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_8u_C3IRSfs,   (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_8u_C4IRSfs,   (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_8u_AC4IRSfs,  (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16s_C1IRSfs,  (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16s_C3IRSfs,  (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16s_C4IRSfs,  (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16s_AC4IRSfs, (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16u_C1IRSfs,  (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16u_C3IRSfs,  (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16u_C4IRSfs,  (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16u_AC4IRSfs, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_8u_C1IRSfs,   (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_8u_C3IRSfs,   (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_8u_C4IRSfs,   (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_8u_AC4IRSfs,  (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16s_C1IRSfs,  (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16s_C3IRSfs,  (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16s_C4IRSfs,  (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16s_AC4IRSfs, (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16u_C1IRSfs,  (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16u_C3IRSfs,  (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16u_C4IRSfs,  (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16u_AC4IRSfs, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////////
//  Name: ippiAddC_32f_C1R, ippiAddC_32f_C3R, ippiAddC_32f_C4R,  ippiAddC_32f_AC4R,
//        ippiSubC_32f_C1R, ippiSubC_32f_C3R, ippiSubC_32f_C4R,  ippiSubC_32f_AC4R,
//        ippiMulC_32f_C1R, ippiMulC_32f_C3R, ippiMulC_32f_C4R,  ippiMulC_32f_AC4R
//
//  Purpose:    Adds, subtracts, or multiplies pixel values of a source image
//              and a constant, and places the results in a destination image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            Width or height of images is less than or equal to zero
//
//  Parameters:
//    value                    The constant value for the specified operation
//    pSrc                     Pointer to the source image
//    srcStep                  Step through the source image
//    pDst                     Pointer to the destination image
//    dstStep                  Step through the destination image
//    roiSize                  Size of the ROI
*/

IPPAPI(IppStatus, ippiAddC_32f_C1R,  (const Ipp32f* pSrc, int srcStep, Ipp32f value, Ipp32f* pDst,
                                      int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAddC_32f_C3R,  (const Ipp32f* pSrc, int srcStep, const Ipp32f value[3],
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAddC_32f_C4R,  (const Ipp32f* pSrc, int srcStep, const Ipp32f value[4],
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAddC_32f_AC4R, (const Ipp32f* pSrc, int srcStep, const Ipp32f value[3],
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32f_C1R,  (const Ipp32f* pSrc, int srcStep, Ipp32f value, Ipp32f* pDst,
                                      int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32f_C3R,  (const Ipp32f* pSrc, int srcStep, const Ipp32f value[3],
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32f_C4R,  (const Ipp32f* pSrc, int srcStep, const Ipp32f value[4],
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32f_AC4R, (const Ipp32f* pSrc, int srcStep, const Ipp32f value[3],
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32f_C1R,  (const Ipp32f* pSrc, int srcStep, Ipp32f value, Ipp32f* pDst,
                                      int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32f_C3R,  (const Ipp32f* pSrc, int srcStep, const Ipp32f value[3],
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32f_C4R,  (const Ipp32f* pSrc, int srcStep, const Ipp32f value[4],
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32f_AC4R, (const Ipp32f* pSrc, int srcStep, const Ipp32f value[3],
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize))

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiAddC_32f_C1IR, ippiAddC_32f_C3IR, ippiAddC_32f_C4IR, ippiAddC_32f_AC4IR,
//        ippiSubC_32f_C1IR, ippiSubC_32f_C3IR, ippiSubC_32f_C4IR, ippiSubC_32f_AC4IR,
//        ippiMulC_32f_C1IR, ippiMulC_32f_C3IR, ippiMulC_32f_C4IR, ippiMulC_32f_AC4IR
//
//  Purpose:    Adds, subtracts, or multiplies pixel values of an image
//              and a constant, and places the results in the same image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         Pointer is NULL
//    ippStsSizeErr            Width or height of an image is less than or equal to zero
//
//  Parameters:
//    value                    The constant value for the specified operation
//    pSrcDst                  Pointer to the image
//    srcDstStep               Step through the image
//    roiSize                  Size of the ROI
*/

IPPAPI(IppStatus, ippiAddC_32f_C1IR,  (Ipp32f value, Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiAddC_32f_C3IR,  (const Ipp32f value[3], Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiAddC_32f_C4IR,  (const Ipp32f value[4], Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiAddC_32f_AC4IR, (const Ipp32f value[3], Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32f_C1IR,  (Ipp32f value, Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32f_C3IR,  (const Ipp32f value[3], Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32f_C4IR,  (const Ipp32f value[4], Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32f_AC4IR, (const Ipp32f value[3], Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32f_C1IR,  (Ipp32f value, Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32f_C3IR,  (const Ipp32f value[3], Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32f_C4IR,  (const Ipp32f value[4], Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32f_AC4IR, (const Ipp32f value[3], Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiAdd_32f_C1IR, ippiAdd_32f_C3IR, ippiAdd_32f_C4IR, ippiAdd_32f_AC4IR,
//        ippiSub_32f_C1IR, ippiSub_32f_C3IR, ippiSub_32f_C4IR, ippiSub_32f_AC4IR,
//        ippiMul_32f_C1IR, ippiMul_32f_C3IR, ippiMul_32f_C4IR, ippiMul_32f_AC4IR
//
//  Purpose:    Adds, subtracts, or multiplies pixel values of two source images
//              and places the results in the first image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            Width or height of images is less than or equal to zero
//
//  Parameters:
//    pSrc                     Pointer to the second source image
//    srcStep                  Step through the second source image
//    pSrcDst                  Pointer to the  first source/destination image
//    srcDstStep               Step through the first source/destination image
//    roiSize                  Size of the ROI
*/

IPPAPI(IppStatus, ippiAdd_32f_C1IR,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAdd_32f_C3IR,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAdd_32f_C4IR,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAdd_32f_AC4IR, (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32f_C1IR,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32f_C3IR,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32f_C4IR,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32f_AC4IR, (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32f_C1IR,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32f_C3IR,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32f_C4IR,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32f_AC4IR, (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////
//  Name: ippiAdd_32f_C1R, ippiAdd_32f_C3R, ippiAdd_32f_C4R, ippiAdd_32f_AC4R,
//        ippiSub_32f_C1R, ippiSub_32f_C3R, ippiSub_32f_C4R, ippiSub_32f_AC4R,
//        ippiMul_32f_C1R, ippiMul_32f_C3R, ippiMul_32f_C4R, ippiMul_32f_AC4R
//
//  Purpose:    Adds, subtracts, or multiplies pixel values of two
//              source images and places the results in a destination image.
//
//  Returns:
//    ippStsNoErr            OK
//    ippStsNullPtrErr       One of the pointers is NULL
//    ippStsSizeErr          Width or height of images is less than or equal to zero
//
//  Parameters:
//    pSrc1, pSrc2           Pointers to the source images
//    src1Step, src2Step     Steps through the source images
//    pDst                   Pointer to the destination image
//    dstStep                Step through the destination image

//    roiSize                Size of the ROI
*/

IPPAPI(IppStatus, ippiAdd_32f_C1R,  (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2,
                                     int src2Step, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAdd_32f_C3R,  (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2,
                                     int src2Step, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAdd_32f_C4R,  (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2,
                                     int src2Step, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAdd_32f_AC4R, (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2,
                                     int src2Step, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32f_C1R,  (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2,
                                     int src2Step, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32f_C3R,  (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2,
                                     int src2Step, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32f_C4R,  (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2,
                                     int src2Step, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32f_AC4R, (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2,
                                     int src2Step, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32f_C1R,  (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2,
                                     int src2Step, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32f_C3R,  (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2,
                                     int src2Step, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32f_C4R,  (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2,
                                     int src2Step, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32f_AC4R, (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2,
                                     int src2Step, Ipp32f* pDst, int dstStep, IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiComplement_32s_C1IR
//
//  Purpose:    Converts negative integer number from complement to
//              direct code reserving the sign in the upper bit.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         Pointer is NULL
//    ippStsStepErr            Step is less than or equal to zero
//    ippStsStrideErr          Step is less than the width of an image
//
//  Parameters:
//    pSrcDst                  Pointer to the source and destination image
//    srcdstStep               Step in bytes through the image
//    roiSize                  Size of the ROI
*/

IPPAPI (IppStatus, ippiComplement_32s_C1IR, ( Ipp32s* pSrcDst, int srcdstStep, IppiSize roiSize ))


/* //////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDiv_32f_C1R, ippiDiv_32f_C3R ippiDiv_32f_C4R ippiDiv_32f_AC4R
//
//  Purpose:    Divides pixel values of an image by pixel values of another image
//              and places the results in a destination image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field with zero or negative value
//    ippStsStepErr            At least one step value is less than or equal to zero
//    ippStsDivByZero          A warning that a divisor value is zero, the function
//                             execution is continued.
//                             If a dividend is equal to zero, then the result is NAN_32F;
//                             if it is greater than zero, then the result is INF_32F,
//                             if it is less than zero, then the result is INF_NEG_32F
//
//  Parameters:
//    pSrc1                    Pointer to the divisor source image
//    src1Step                 Step through the divisor source image
//    pSrc2                    Pointer to the dividend source image
//    src2Step                 Step through the dividend source image
//    pDst                     Pointer to the destination image
//    dstStep                  Step through the destination image
//    roiSize                  Size of the ROI
*/

IPPAPI(IppStatus, ippiDiv_32f_C1R,    (const Ipp32f* pSrc1, int src1Step,
                                       const Ipp32f* pSrc2, int src2Step,
                                             Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDiv_32f_C3R,    (const Ipp32f* pSrc1, int src1Step,
                                       const Ipp32f* pSrc2, int src2Step,
                                             Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDiv_32f_C4R,    (const Ipp32f* pSrc1, int src1Step,
                                       const Ipp32f* pSrc2, int src2Step,
                                             Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDiv_32f_AC4R,    (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                             Ipp32f* pDst, int dstStep, IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDiv_16s_C1RSfs, ippiDiv_8u_C1RSfs, ippiDiv_16u_C1RSfs,
//              ippiDiv_16s_C3RSfs, ippiDiv_8u_C3RSfs, ippiDiv_16u_C3RSfs,
//              ippiDiv_16s_C4RSfs, ippiDiv_8u_C4RSfs, ippiDiv_16u_C4RSfs,
//              ippiDiv_16s_AC4RSfs,ippiDiv_8u_AC4RSfs,ippiDiv_16u_AC4RSfs
//
//  Purpose:    Divides pixel values of an image by pixel values of
//              another image and places the scaled results in a destination
//              image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field with zero or negative value
//    ippStsStepErr            At least one step value is less than or equal to zero
//    ippStsDivByZero          A warning that a divisor value is zero, the function
//                             execution is continued.
//                    If a dividend is equal to zero, then the result is zero;
//                    if it is greater than zero, then the result is IPP_MAX_16S, or IPP_MAX_8U, or IPP_MAX_16U
//                    if it is less than zero (for 16s), then the result is IPP_MIN_16S
//
//  Parameters:
//    pSrc1                    Pointer to the divisor source image
//    src1Step                 Step through the divisor source image
//    pSrc2                    Pointer to the dividend source image
//    src2Step                 Step through the dividend source image
//    pDst                     Pointer to the destination image
//    dstStep                  Step through the destination image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiDiv_16s_C1RSfs, (const Ipp16s* pSrc1, int src1Step,
                                       const Ipp16s* pSrc2, int src2Step,
                                             Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16s_C3RSfs, (const Ipp16s* pSrc1, int src1Step,
                                       const Ipp16s* pSrc2, int src2Step,
                                             Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16s_C4RSfs, (const Ipp16s* pSrc1, int src1Step,
                                       const Ipp16s* pSrc2, int src2Step,
                                       Ipp16s* pDst, int dstStep, IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_16s_AC4RSfs, (const Ipp16s* pSrc1, int src1Step,
                                       const Ipp16s* pSrc2, int src2Step,
                                       Ipp16s* pDst, int dstStep, IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_8u_C1RSfs,  (const Ipp8u* pSrc1, int src1Step,
                                       const Ipp8u* pSrc2, int src2Step,
                                             Ipp8u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_8u_C3RSfs,  (const Ipp8u* pSrc1, int src1Step,
                                       const Ipp8u* pSrc2, int src2Step,
                                             Ipp8u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_8u_C4RSfs, (const Ipp8u* pSrc1, int src1Step,
                                       const Ipp8u* pSrc2, int src2Step,
                                       Ipp8u* pDst, int dstStep, IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_8u_AC4RSfs, (const Ipp8u* pSrc1, int src1Step,
                                       const Ipp8u* pSrc2, int src2Step,
                                       Ipp8u* pDst, int dstStep, IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_16u_C1RSfs, (const Ipp16u* pSrc1, int src1Step,
                                       const Ipp16u* pSrc2, int src2Step,
                                             Ipp16u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16u_C3RSfs, (const Ipp16u* pSrc1, int src1Step,
                                       const Ipp16u* pSrc2, int src2Step,
                                             Ipp16u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16u_C4RSfs, (const Ipp16u* pSrc1, int src1Step,
                                       const Ipp16u* pSrc2, int src2Step,
                                       Ipp16u* pDst, int dstStep, IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_16u_AC4RSfs, (const Ipp16u* pSrc1, int src1Step,
                                       const Ipp16u* pSrc2, int src2Step,
                                       Ipp16u* pDst, int dstStep, IppiSize roiSize, int ScaleFactor))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDivC_32f_C1R, ippiDivC_32f_C3R
//              ippiDivC_32f_C4R, ippiDivC_32f_AC4R
//
//  Purpose:    Divides pixel values of a source image by a constant
//              and places the results in a destination image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field with zero or negative value
//    ippStsStepErr            step value is less than or equal to zero
//    ippStsDivByZeroErr       The constant is equal to zero
//
//  Parameters:
//    value                    The constant divisor
//    pSrc                     Pointer to the source image
//    pDst                     Pointer to the destination image
//    roiSize                  Size of the ROI
*/

IPPAPI(IppStatus, ippiDivC_32f_C1R,    (const Ipp32f* pSrc, int srcStep, Ipp32f value,
                                        Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDivC_32f_C3R,    (const Ipp32f* pSrc, int srcStep, const Ipp32f value[3],
                                        Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDivC_32f_C4R, (const Ipp32f* pSrc, int srcStep, const Ipp32f val[4],
                                     Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDivC_32f_AC4R,(const Ipp32f* pSrc, int srcStep, const Ipp32f val[3],
                                     Ipp32f* pDst, int dstStep, IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDivC_16s_C1RSfs, ippiDivC_8u_C1RSfs, ippiDivC_16u_C1RSfs,
//              ippiDivC_16s_C3RSfs, ippiDivC_8u_C3RSfs, ippiDivC_16u_C3RSfs,
//              ippiDivC_16s_C4RSfs, ippiDivC_8u_C4RSfs, ippiDivC_16u_C4RSfs,
//              ippiDivC_16s_AC4RSfs,ippiDivC_8u_AC4RSfs,ippiDivC_16u_AC4RSfs
//
//  Purpose:    Divides pixel values of a source image by a constant
//              and places the scaled results in a destination image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field with zero or negative value
//    ippStsStepErr            Step value is less than or equal to zero
//    ippStsDivByZeroErr       The constant is equal to zero
//
//  Parameters:
//    value                    Constant divisor
//    pSrc                     Pointer to the source image
//    pDst                     Pointer to the destination image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiDivC_16s_C1RSfs, (const Ipp16s* pSrc, int srcStep, Ipp16s value,
                                        Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16s_C3RSfs, (const Ipp16s* pSrc, int srcStep, const Ipp16s value[3],
                                        Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16s_C4RSfs,  (const Ipp16s* pSrc, int srcStep, const Ipp16s value[4],
                                        Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16s_AC4RSfs,  (const Ipp16s* pSrc, int srcStep, const Ipp16s value[3],
                                        Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_8u_C1RSfs,  (const Ipp8u* pSrc, int srcStep, Ipp8u value,
                                        Ipp8u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_8u_C3RSfs,  (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3],
                                        Ipp8u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_8u_C4RSfs,  (const Ipp8u* pSrc, int srcStep, const Ipp8u value[4],
                                        Ipp8u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_8u_AC4RSfs,  (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3],
                                        Ipp8u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16u_C1RSfs, (const Ipp16u* pSrc, int srcStep, Ipp16u value,
                                        Ipp16u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16u_C3RSfs, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3],
                                        Ipp16u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16u_C4RSfs,  (const Ipp16u* pSrc, int srcStep, const Ipp16u value[4],
                                        Ipp16u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16u_AC4RSfs,  (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3],
                                        Ipp16u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDiv_32f_C1IR, ippiDiv_32f_C3IR ippiDiv_32f_C4IR ippiDiv_32f_AC4IR
//
//  Purpose:    Divides pixel values of an image by pixel values of
//              another image and places the results in the dividend source
//              image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field with zero or negative value
//    ippStsStepErr            At least one step value is less than or equal to zero
//    ippStsDivByZero          A warning that a divisor value is zero, the function
//                             execution is continued.
//                             If a dividend is equal to zero, then the result is NAN_32F;
//                             if it is greater than zero, then the result is INF_32F,
//                             if it is less than zero, then the result is INF_NEG_32F
//
//  Parameters:
//    pSrc                     Pointer to the divisor source image
//    srcStep                  Step through the divisor source image
//    pSrcDst                  Pointer to the dividend source/destination image
//    srcDstStep               Step through the dividend source/destination image
//    roiSize                  Size of the ROI
*/

IPPAPI(IppStatus, ippiDiv_32f_C1IR,    (const Ipp32f* pSrc, int srcStep,
                                              Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDiv_32f_C3IR,    (const Ipp32f* pSrc, int srcStep,
                                              Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDiv_32f_C4IR,    (const Ipp32f* pSrc, int srcStep,
                                              Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDiv_32f_AC4IR,    (const Ipp32f* pSrc, int srcStep,
                                              Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDiv_16s_C1IRSfs, ippiDiv_8u_C1IRSfs, ippiDiv_16u_C1IRSfs,
//              ippiDiv_16s_C3IRSfs, ippiDiv_8u_C3IRSfs, ippiDiv_16u_C3IRSfs,
//              ippiDiv_16s_C4IRSfs, ippiDiv_8u_C4IRSfs, ippiDiv_16u_C4IRSfs,
//              ippiDiv_16s_AC4IRSfs,ippiDiv_8u_AC4IRSfs,ippiDiv_16u_AC4IRSfs
//
//  Purpose:    Divides pixel values of an image by pixel values of
//              another image and places the scaled results in the dividend
//              source image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field with zero or negative value
//    ippStsStepErr            At least one step value is less than or equal to zero
//    ippStsDivByZero          A warning that a divisor value is zero, the function
//                             execution is continued.
//                    If a dividend is equal to zero, then the result is zero;
//                    if it is greater than zero, then the result is IPP_MAX_16S, or IPP_MAX_8U, or IPP_MAX_16U
//                    if it is less than zero (for 16s), then the result is IPP_MIN_16S
//
//  Parameters:
//    pSrc                     Pointer to the divisor source image
//    srcStep                  Step through the divisor source image
//    pSrcDst                  Pointer to the dividend source/destination image
//    srcDstStep               Step through the dividend source/destination image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiDiv_16s_C1IRSfs, (const Ipp16s* pSrc, int srcStep,
                                              Ipp16s* pSrcDst, int srcDstStep,
                                              IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16s_C3IRSfs, (const Ipp16s* pSrc, int srcStep,
                                              Ipp16s* pSrcDst, int srcDstStep,
                                              IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16s_C4IRSfs, (const Ipp16s* pSrc, int srcStep,
                                              Ipp16s* pSrcDst, int srcDstStep,
                                              IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_16s_AC4IRSfs, (const Ipp16s* pSrc, int srcStep,
                                              Ipp16s* pSrcDst, int srcDstStep,
                                              IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_8u_C1IRSfs,  (const Ipp8u* pSrc, int srcStep,
                                              Ipp8u* pSrcDst, int srcDstStep,
                                              IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_8u_C3IRSfs,  (const Ipp8u* pSrc, int srcStep,
                                              Ipp8u* pSrcDst, int srcDstStep,
                                              IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_8u_C4IRSfs, (const Ipp8u* pSrc, int srcStep,
                                             Ipp8u* pSrcDst, int srcDstStep,
                                             IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_8u_AC4IRSfs, (const Ipp8u* pSrc, int srcStep,
                                             Ipp8u* pSrcDst, int srcDstStep,
                                             IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_16u_C1IRSfs, (const Ipp16u* pSrc, int srcStep,
                                              Ipp16u* pSrcDst, int srcDstStep,
                                              IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16u_C3IRSfs, (const Ipp16u* pSrc, int srcStep,
                                              Ipp16u* pSrcDst, int srcDstStep,
                                              IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16u_C4IRSfs, (const Ipp16u* pSrc, int srcStep,
                                              Ipp16u* pSrcDst, int srcDstStep,
                                              IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_16u_AC4IRSfs, (const Ipp16u* pSrc, int srcStep,
                                              Ipp16u* pSrcDst, int srcDstStep,
                                              IppiSize roiSize, int ScaleFactor))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDivC_32f_C1IR, ippiDivC_32f_C3IR,
//              ippiDivC_32f_C4IR, ippiDivC_32f_AC4IR
//
//  Purpose:    Divides pixel values of a source image by a constant
//              and places the results in the same image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         The pointer is NULL
//    ippStsSizeErr            The roiSize has a field with zero or negative value
//    ippStsStepErr            The step value is less than or equal to zero
//    ippStsDivByZeroErr       The constant is equal to zero
//
//  Parameters:
//    value                    The constant divisor
//    pSrcDst                  Pointer to the source/destination image
//    srcDstStep               Step through the source/destination image
//    roiSize                  Size of the ROI
*/

IPPAPI(IppStatus, ippiDivC_32f_C1IR,    (Ipp32f value, Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDivC_32f_C3IR,    (const Ipp32f value[3], Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDivC_32f_C4IR, (const Ipp32f val[4], Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDivC_32f_AC4IR, (const Ipp32f val[3], Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDivC_16s_C1IRSfs, ippiDivC_8u_C1IRSfs, ippiDivC_16u_C1IRSfs,
//              ippiDivC_16s_C3IRSfs, ippiDivC_8u_C3IRSfs, ippiDivC_16u_C3IRSfs,
//              ippiDivC_16s_C4IRSfs, ippiDivC_8u_C4IRSfs, ippiDivC_16u_C4IRSfs,
//              ippiDivC_16s_AC4IRSfs,ippiDivC_8u_AC4IRSfs,ippiDivC_16u_AC4IRSfs
//
//  Purpose:    Divides pixel values of a source image by a constant
//              and places the scaled results in the same image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         The pointer is NULL
//    ippStsSizeErr            The roiSize has a field with zero or negative value
//    ippStsStepErr            The step value is less than or equal to zero
//    ippStsDivByZeroErr       The constant is equal to zero
//
//  Parameters:
//    value                    The constant divisor
//    pSrcDst                  Pointer to the source/destination image
//    srcDstStep               Step through the source/destination image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiDivC_16s_C1IRSfs, (Ipp16s value, Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16s_C3IRSfs, (const Ipp16s value[3], Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16s_C4IRSfs, (const Ipp16s val[4], Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDivC_16s_AC4IRSfs, (const Ipp16s val[3], Ipp16s* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDivC_8u_C1IRSfs,  (Ipp8u value, Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_8u_C3IRSfs,  (const Ipp8u value[3], Ipp8u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_8u_C4IRSfs, (const Ipp8u val[4], Ipp8u* pSrcDst,
                                        int srcDstStep, IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDivC_8u_AC4IRSfs, (const Ipp8u val[3], Ipp8u* pSrcDst,
                                        int srcDstStep, IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDivC_16u_C1IRSfs, (Ipp16u value, Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16u_C3IRSfs, (const Ipp16u value[3], Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16u_C4IRSfs, (const Ipp16u val[4], Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int ScaleFactor))
IPPAPI(IppStatus, ippiDivC_16u_AC4IRSfs, (const Ipp16u val[3], Ipp16u* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int ScaleFactor))


/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiAbs_16s_C1R
//              ippiAbs_16s_C3R
//              ippiAbs_16s_C4R
//              ippiAbs_16s_AC4R
//              ippiAbs_32f_C1R
//              ippiAbs_32f_C3R
//              ippiAbs_32f_C4R
//              ippiAbs_32f_AC4R
//
//              ippiAbs_16s_C1IR
//              ippiAbs_16s_C3IR
//              ippiAbs_16s_C4IR
//              ippiAbs_16s_AC4IR
//              ippiAbs_32f_C1IR
//              ippiAbs_32f_C3IR
//              ippiAbs_32f_C4IR
//              ippiAbs_32f_AC4IR
//
//  Purpose:    computes absolute value of each pixel of a source image and
//              places results in the destination image;
//              for in-place flavors - in the same source image
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of the pointers is NULL
//   ippStsSizeErr     The roiSize has a field with negative or zero value
//
//  Parameters:
//   pSrc       pointer to the source image
//   srcStep    step through the source image
//   pDst       pointer to the destination image
//   dstStep    step through the destination image
//   pSrcDst    pointer to the source/destination image (for in-place function)
//   srcDstStep step through the source/destination image (for in-place function)
//   roiSize    size of the ROI
*/
IPPAPI(IppStatus,ippiAbs_16s_C1R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_16s_C3R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_16s_AC4R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_32f_C1R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_32f_C3R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_32f_AC4R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_16s_C1IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_16s_C3IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_16s_AC4IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_32f_C3IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_16s_C4R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_32f_C4R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_16s_C4IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiAbs_32f_C4IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))


/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiSqr_8u_C1RSfs
//              ippiSqr_8u_C3RSfs
//              ippiSqr_8u_AC4RSfs
//              ippiSqr_8u_C4RSfs
//              ippiSqr_16u_C1RSfs
//              ippiSqr_16u_C3RSfs
//              ippiSqr_16u_AC4RSfs
//              ippiSqr_16u_C4RSfs
//              ippiSqr_16s_C1RSfs
//              ippiSqr_16s_C3RSfs
//              ippiSqr_16s_AC4RSfs
//              ippiSqr_16s_C4RSfs
//              ippiSqr_32f_C1R
//              ippiSqr_32f_C3R
//              ippiSqr_32f_AC4R
//              ippiSqr_32f_C4R
//
//              ippiSqr_8u_C1IRSfs
//              ippiSqr_8u_C3IRSfs
//              ippiSqr_8u_AC4IRSfs
//              ippiSqr_8u_C4IRSfs
//              ippiSqr_16u_C1IRSfs
//              ippiSqr_16u_C3IRSfs
//              ippiSqr_16u_AC4IRSfs
//              ippiSqr_16u_C4IRSfs
//              ippiSqr_16s_C1IRSfs
//              ippiSqr_16s_C3IRSfs
//              ippiSqr_16s_AC4IRSfs
//              ippiSqr_16s_C4IRSfs
//              ippiSqr_32f_C1IR
//              ippiSqr_32f_C3IR
//              ippiSqr_32f_AC4IR
//              ippiSqr_32f_C4IR
//
//  Purpose:    squares pixel values of an image and
//              places results in the destination image;
//              for in-place flavors - in  the same image
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of the pointers is NULL
//   ippStsSizeErr     The roiSize has a field with negative or zero value
//
//  Parameters:
//   pSrc       pointer to the source image
//   srcStep    step through the source image
//   pDst       pointer to the destination image
//   dstStep    step through the destination image
//   pSrcDst    pointer to the source/destination image (for in-place function)
//   srcDstStep step through the source/destination image (for in-place function)
//   roiSize    size of the ROI
//   scaleFactor scale factor
*/

IPPAPI(IppStatus,ippiSqr_8u_C1RSfs,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_8u_C3RSfs,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_8u_AC4RSfs,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_8u_C4RSfs,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16u_C1RSfs,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16u_C3RSfs,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16u_AC4RSfs,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16u_C4RSfs,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16s_C1RSfs,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16s_C3RSfs,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16s_AC4RSfs,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16s_C4RSfs,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_32f_C1R, (const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiSqr_32f_C3R, (const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiSqr_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiSqr_32f_C4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiSqr_8u_C1IRSfs, (Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_8u_C3IRSfs, (Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_8u_AC4IRSfs,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_8u_C4IRSfs,(Ipp8u* pSrcDst, int srcDstStep,

       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16u_C1IRSfs, (Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16u_C3IRSfs, (Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16u_AC4IRSfs,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16u_C4IRSfs,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16s_C1IRSfs, (Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16s_C3IRSfs, (Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16s_AC4IRSfs,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_16s_C4IRSfs,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqr_32f_C1IR, (Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiSqr_32f_C3IR, (Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiSqr_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiSqr_32f_C4IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize))


/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiSqrt_8u_C1RSfs
//              ippiSqrt_8u_C3RSfs
//              ippiSqrt_8u_AC4RSfs
//              ippiSqrt_16u_C1RSfs
//              ippiSqrt_16u_C3RSfs
//              ippiSqrt_16u_AC4RSfs
//              ippiSqrt_16s_C1RSfs
//              ippiSqrt_16s_C3RSfs
//              ippiSqrt_16s_AC4RSfs
//              ippiSqrt_32f_C1R
//              ippiSqrt_32f_C3R
//              ippiSqrt_32f_AC4R
//
//              ippiSqrt_8u_C1IRSfs
//              ippiSqrt_8u_C3IRSfs
//              ippiSqrt_8u_AC4IRSfs
//              ippiSqrt_16u_C1IRSfs
//              ippiSqrt_16u_C3IRSfs
//              ippiSqrt_16u_AC4IRSfs
//              ippiSqrt_16s_C1IRSfs
//              ippiSqrt_16s_C3IRSfs
//              ippiSqrt_16s_AC4IRSfs
//              ippiSqrt_32f_C1IR
//              ippiSqrt_32f_C3IR
//              ippiSqrt_32f_AC4IR
//              ippiSqrt_32f_C4IR
//  Purpose:    computes square roots of pixel values of a source image and
//              places results in the destination image;
//              for in-place flavors - in the same image
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of pointers is NULL
//   ippStsSizeErr     The roiSize has a field with negative or zero value
//   ippStsSqrtNegArg  Source image pixel has a negative value
//
//  Parameters:
//   pSrc       pointer to the source image
//   srcStep    step through the source image
//   pDst       pointer to the destination image
//   dstStep    step through the destination image
//   pSrcDst    pointer to the source/destination image (for in-place function)
//   srcDstStep step through the source/destination image (for in-place function)
//   roiSize    size of the ROI
//   scaleFactor scale factor
*/
IPPAPI(IppStatus,ippiSqrt_8u_C1RSfs,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_8u_C3RSfs,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_8u_AC4RSfs,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_16u_C1RSfs,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_16u_C3RSfs,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_16u_AC4RSfs,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_16s_C1RSfs,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_16s_C3RSfs,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_16s_AC4RSfs,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_32f_C1R, (const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiSqrt_32f_C3R, (const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiSqrt_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiSqrt_8u_C1IRSfs, (Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_8u_C3IRSfs, (Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_8u_AC4IRSfs,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_16u_C1IRSfs, (Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_16u_C3IRSfs, (Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_16u_AC4IRSfs,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_16s_C1IRSfs, (Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_16s_C3IRSfs, (Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_16s_AC4IRSfs,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus,ippiSqrt_32f_C1IR, (Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiSqrt_32f_C3IR, (Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiSqrt_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize))
IPPAPI(IppStatus,ippiSqrt_32f_C4IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize))



/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//  ippiLn_32f_C1IR   ippiLn_16s_C1IRSfs  ippiLn_8u_C1IRSfs ippiLn_16u_C1IRSfs
//  ippiLn_32f_C3IR   ippiLn_16s_C3IRSfs  ippiLn_8u_C3IRSfs ippiLn_16u_C3IRSfs
//  ippiLn_32f_C1R    ippiLn_16s_C1RSfs   ippiLn_8u_C1RSfs  ippiLn_16u_C1RSfs
//  ippiLn_32f_C3R    ippiLn_16s_C3RSfs   ippiLn_8u_C3RSfs  ippiLn_16u_C3RSfs
//  Purpose:
//     computes the natural logarithm of each pixel values of a source image
//     and places the results in the destination image;
//     for in-place flavors - in the same image
//  Parameters:
//    pSrc         Pointer to the source image.
//    pDst         Pointer to the destination image.
//    pSrcDst      Pointer to the source/destination image for in-place functions.
//    srcStep      Step through the source image.
//    dstStep      Step through the destination image.
//    srcDstStep   Step through the source/destination image for in-place functions.
//    roiSize      Size of the ROI.
//    scaleFactor  Scale factor for integer data.
//  Returns:
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsSizeErr       The roiSize has a field with negative or zero value
//    ippStsStepErr       One of the step values is less than or equal to zero
//    ippStsLnZeroArg     The source pixel has a zero value
//    ippStsLnNegArg      The source pixel has a negative value
//    ippStsNoErr         otherwise
*/

IPPAPI(IppStatus, ippiLn_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLn_32f_C3IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLn_32f_C1R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst,
                                                       int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLn_32f_C3R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst,
                                                       int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiLn_16s_C1IRSfs,(Ipp16s* pSrcDst, int srcDstStep,
                                    IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiLn_16s_C3IRSfs,(Ipp16s* pSrcDst, int srcDstStep,
                                    IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiLn_16s_C1RSfs, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst,

                                     int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiLn_16s_C3RSfs, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst,
                                     int dstStep, IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus,ippiLn_16u_C1RSfs,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst,
                                        int dstStep, IppiSize roiSize, int ScalFact))
IPPAPI(IppStatus,ippiLn_16u_C3RSfs,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst,
                                        int dstStep, IppiSize roiSize, int ScalFact))


IPPAPI(IppStatus, ippiLn_8u_C1IRSfs,(Ipp8u* pSrcDst, int srcDstStep,
                                    IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiLn_8u_C3IRSfs,(Ipp8u* pSrcDst, int srcDstStep,
                                    IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiLn_8u_C1RSfs, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst,
                                     int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiLn_8u_C3RSfs, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst,
                                     int dstStep, IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus,ippiLn_16u_C1IRSfs,(Ipp16u* pSrcDst, int srcDstStep,
                                        IppiSize roiSize, int ScalFact))
IPPAPI(IppStatus,ippiLn_16u_C3IRSfs,(Ipp16u* pSrcDst, int srcDstStep,
                                        IppiSize roiSize, int ScalFact))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//  ippiExp_32f_C1IR   ippiExp_16s_C1IRSfs  ippiExp_8u_C1IRSfs ippiExp_16u_C1IRSfs
//  ippiExp_32f_C3IR   ippiExp_16s_C3IRSfs  ippiExp_8u_C3IRSfs ippiExp_16u_C3IRSfs
//  ippiExp_32f_C1R    ippiExp_16s_C1RSfs   ippiExp_8u_C1RSfs  ippiExp_16u_C1RSfs
//  ippiExp_32f_C3R    ippiExp_16s_C3RSfs   ippiExp_8u_C3RSfs  ippiExp_16u_C3RSfs
//  Purpose:
//     computes the exponential of pixel values in a source image
//  Parameters:
//    pSrc         Pointer to the source image.
//    pDst         Pointer to the destination image.
//    pSrcDst      Pointer to the source/destination image for in-place functions.
//    srcStep      Step through the source image.
//    dstStep      Step through the in destination image.
//    srcDstStep   Step through the source/destination image for in-place functions.
//    roiSize      Size of the ROI.
//    scaleFactor  Scale factor for integer data.

//  Returns:
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsSizeErr       The roiSize has a field with negative or zero value
//    ippStsStepErr       One of the step values is less than or equal to zero
//    ippStsNoErr         otherwise
*/


IPPAPI(IppStatus, ippiExp_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiExp_32f_C3IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiExp_32f_C1R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst,
                                                       int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiExp_32f_C3R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst,
                                                       int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiExp_16s_C1IRSfs,(Ipp16s* pSrcDst, int srcDstStep,
                                    IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiExp_16s_C3IRSfs,(Ipp16s* pSrcDst, int srcDstStep,
                                    IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiExp_16s_C1RSfs, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst,
                                     int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiExp_16s_C3RSfs, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst,
                                     int dstStep, IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus,ippiExp_16u_C1IRSfs,(Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int sFact))
IPPAPI(IppStatus,ippiExp_16u_C3IRSfs,(Ipp16u* pSrcDst, int srcDstStep,
                                          IppiSize roiSize, int sFact))


IPPAPI(IppStatus, ippiExp_8u_C1IRSfs,(Ipp8u* pSrcDst, int srcDstStep,
                                    IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiExp_8u_C3IRSfs,(Ipp8u* pSrcDst, int srcDstStep,
                                    IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiExp_8u_C1RSfs, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst,
                                     int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiExp_8u_C3RSfs, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst,
                                     int dstStep, IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus,ippiExp_16u_C1RSfs,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst,
                                          int dstStep, IppiSize roiSize, int sFact))
IPPAPI(IppStatus,ippiExp_16u_C3RSfs,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst,
                                          int dstStep, IppiSize roiSize, int sFact))

/* /////////////////////////////////////////////////////////////////////////////
//                  Arithmetic Functions Operating on Complex Data
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAddC_32fc_C1R, ippiAddC_32fc_C3R, ippiAddC_32fc_AC4R,
//              ippiSubC_32fc_C1R, ippiSubC_32fc_C3R, ippiSubC_32fc_AC4R,
//              ippiMulC_32fc_C1R, ippiMulC_32fc_C3R, ippiMulC_32fc_AC4R
//              ippiDivC_32fc_C1R, ippiDivC_32fc_C3R, ippiDivC_32fc_AC4R
//
//  Purpose:    Adds, subtracts, multiplies, or divides pixel values of an image
//              and a constant and places the results in the destination image.
//
//  Returns:
//    ippStsNoErr          OK
//    ippStsNullPtrErr     One of the pointers is NULL
//    ippStsSizeErr        The roiSize has a field with negative or zero value
//    ippStsStepErr        One of the step values is less than or equal to zero
//    ippStsDivByZeroErr   The constant is equal to zero (for division)
//
//  Parameters:
//    value                The constant value (constant vector for multi-channel images)
//    pSrc                 Pointer to the source image
//    srcStep              Step through the source image
//    pDst                 Pointer to the destination image
//    dstStep              Step through the destination image
//    roiSize              Size of the ROI
*/

IPPAPI(IppStatus, ippiAddC_32fc_C1R,  (const Ipp32fc* pSrc, int srcStep, Ipp32fc value, Ipp32fc* pDst,
                                      int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAddC_32fc_C3R,  (const Ipp32fc* pSrc, int srcStep, const Ipp32fc value[3],
                                      Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAddC_32fc_AC4R, (const Ipp32fc* pSrc, int srcStep, const Ipp32fc value[3],
                                      Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32fc_C1R,  (const Ipp32fc* pSrc, int srcStep, Ipp32fc value, Ipp32fc* pDst,
                                      int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32fc_C3R,  (const Ipp32fc* pSrc, int srcStep, const Ipp32fc value[3],
                                      Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32fc_AC4R, (const Ipp32fc* pSrc, int srcStep, const Ipp32fc value[3],
                                      Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32fc_C1R,  (const Ipp32fc* pSrc, int srcStep, Ipp32fc value, Ipp32fc* pDst,
                                      int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32fc_C3R,  (const Ipp32fc* pSrc, int srcStep, const Ipp32fc value[3],
                                      Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32fc_AC4R, (const Ipp32fc* pSrc, int srcStep, const Ipp32fc value[3],
                                      Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDivC_32fc_C1R,  (const Ipp32fc* pSrc,
                                      int srcStep, Ipp32fc value, Ipp32fc* pDst,
                                      int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDivC_32fc_C3R,  (const Ipp32fc* pSrc,
                                      int srcStep, const Ipp32fc value[3], Ipp32fc* pDst,
                                      int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDivC_32fc_AC4R, (const Ipp32fc* pSrc,
                                      int srcStep, const Ipp32fc value[3], Ipp32fc* pDst,
                                      int dstStep, IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAddC_32fc_C1IR, ippiAddC_32fc_C3IR, ippiAddC_32fc_AC4IR,
//              ippiSubC_32fc_C1IR, ippiSubC_32fc_C3IR, ippiSubC_32fc_AC4IR,
//              ippiMulC_32fc_C1IR, ippiMulC_32fc_C3IR, ippiMulC_32fc_AC4IR
//              ippiDivC_32fc_C1IR, ippiDivC_32fc_C3IR, ippiDivC_32fc_AC4IR
//
//  Purpose:    Adds, subtracts, multiplies, or divides pixel values of an image
//              and a constant and places the results in the same image.
//
//  Returns:
//    ippStsNoErr            OK
//    ippStsNullPtrErr       The pointer pSrcDst is NULL
//    ippStsSizeErr          The roiSize has a field with negative or zero value
//    ippStsStepErr          The step value is less than or equal to zero
//    ippStsDivByZeroErr     The constant is equal to zero (for division)
//
//  Parameters:
//    value                  The constant value (constant vector for multi-channel images)
//    pSrcDst                Pointer to the image
//    srcDstStep             Step through the image
//    roiSize                Size of the ROI
*/
IPPAPI(IppStatus, ippiAddC_32fc_C1IR,  (Ipp32fc value, Ipp32fc* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiAddC_32fc_C3IR,  (const Ipp32fc value[3], Ipp32fc* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiAddC_32fc_AC4IR, (const Ipp32fc value[3], Ipp32fc* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32fc_C1IR,  (Ipp32fc value, Ipp32fc* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32fc_C3IR,  (const Ipp32fc value[3], Ipp32fc* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiSubC_32fc_AC4IR, (const Ipp32fc value[3], Ipp32fc* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32fc_C1IR,  (Ipp32fc value, Ipp32fc* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32fc_C3IR,  (const Ipp32fc value[3], Ipp32fc* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiMulC_32fc_AC4IR, (const Ipp32fc value[3], Ipp32fc* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiDivC_32fc_C1IR,  (Ipp32fc value, Ipp32fc* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiDivC_32fc_C3IR,  (const Ipp32fc value[3], Ipp32fc* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))
IPPAPI(IppStatus, ippiDivC_32fc_AC4IR, (const Ipp32fc value[3], Ipp32fc* pSrcDst, int srcDstStep,
                                       IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAdd_32fc_C1IR, ippiAdd_32fc_C3IR, ippiAdd_32fc_AC4IR,
//              ippiSub_32fc_C1IR, ippiSub_32fc_C3IR, ippiSub_32fc_AC4IR,
//              ippiMul_32fc_C1IR, ippiMul_32fc_C3IR, ippiMul_32fc_AC4IR
//              ippiDiv_32fc_C1IR, ippiDiv_32fc_C3IR, ippiDiv_32fc_AC4IR
//
//  Purpose:    Adds, subtracts, multiplies, or divides pixel values of two
//              source images and places the results in the first source image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            The roiSize has a field with negative or zero value
//    ippStsStepErr            Any of the step values is less than or equal to zero
//    ippStsDivByZero          For division only - a warning that a divisor value (pixel value
//                             of the second image) equals zero, the function execution is continued.
//                               If a dividend is equal to zero, then the result is NAN_32F;
//                               if it is greater than zero, then the result is INF_32F,
//                               if it is less than zero, then the result is INF_NEG_32F
//
//  Parameters:
//    pSrc                     Pointer to the second source image
//    srcStep                  Step through the second source image
//    pSrcDst                  Pointer to the first source/destination image
//    srcDstStep               Step through the first source/destination image
//    roiSize                  Size of the ROI
*/

IPPAPI(IppStatus, ippiAdd_32fc_C1IR,  (const Ipp32fc* pSrc, int srcStep, Ipp32fc* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAdd_32fc_C3IR,  (const Ipp32fc* pSrc, int srcStep, Ipp32fc* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAdd_32fc_AC4IR, (const Ipp32fc* pSrc, int srcStep, Ipp32fc* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32fc_C1IR,  (const Ipp32fc* pSrc, int srcStep, Ipp32fc* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32fc_C3IR,  (const Ipp32fc* pSrc, int srcStep, Ipp32fc* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32fc_AC4IR, (const Ipp32fc* pSrc, int srcStep, Ipp32fc* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32fc_C1IR,  (const Ipp32fc* pSrc, int srcStep, Ipp32fc* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32fc_C3IR,  (const Ipp32fc* pSrc, int srcStep, Ipp32fc* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32fc_AC4IR, (const Ipp32fc* pSrc, int srcStep, Ipp32fc* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDiv_32fc_C1IR,  (const Ipp32fc* pSrc, int srcStep, Ipp32fc* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDiv_32fc_C3IR,  (const Ipp32fc* pSrc, int srcStep, Ipp32fc* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDiv_32fc_AC4IR, (const Ipp32fc* pSrc, int srcStep, Ipp32fc* pSrcDst,
                                      int srcDstStep, IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAdd_32fc_C1R, ippiAdd_32fc_C3R, ippiAdd_32fc_AC4R,
//              ippiSub_32fc_C1R, ippiSub_32fc_C3R, ippiSub_32fc_AC4R,
//              ippiMul_32fc_C1R, ippiMul_32fc_C3R, ippiMul_32fc_AC4R
//              ippiDiv_32fc_C1R, ippiDiv_32fc_C3R, ippiDiv_32fc_AC4R
//
//  Purpose:    Adds, subtracts, multiplies, or divides pixel values of two
//              source images and places the results in the destination image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            The roiSize has a field with negative or zero value
//    ippStsStepErr            Any of the step values is less than or equal to zero
//    ippStsDivByZero          For division only - a warning that a divisor value (pixel value
//                             of the second image) equals zero, the function execution is continued.
//                               If a dividend is equal to zero, then the result is NAN_32F;
//                               if it is greater than zero, then the result is INF_32F,
//                               if it is less than zero, then the result is INF_NEG_32F
//
//  Parameters:
//    pSrc1, pSrc2             Pointers to the first and second source images
//    src1Step, src2Step       Step through the first and second source images
//    pDst                     Pointer to the destination image
//    dstStep                  Step through the destination image
//    roiSize                  Size of the ROI
*/

IPPAPI(IppStatus, ippiAdd_32fc_C1R,  (const Ipp32fc* pSrc1, int src1Step, const Ipp32fc* pSrc2,
                                     int src2Step, Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAdd_32fc_C3R,  (const Ipp32fc* pSrc1, int src1Step, const Ipp32fc* pSrc2,
                                     int src2Step, Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAdd_32fc_AC4R, (const Ipp32fc* pSrc1, int src1Step, const Ipp32fc* pSrc2,
                                     int src2Step, Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32fc_C1R,  (const Ipp32fc* pSrc1, int src1Step, const Ipp32fc* pSrc2,
                                     int src2Step, Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32fc_C3R,  (const Ipp32fc* pSrc1, int src1Step, const Ipp32fc* pSrc2,
                                     int src2Step, Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiSub_32fc_AC4R, (const Ipp32fc* pSrc1, int src1Step, const Ipp32fc* pSrc2,
                                     int src2Step, Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32fc_C1R,  (const Ipp32fc* pSrc1, int src1Step, const Ipp32fc* pSrc2,
                                     int src2Step, Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32fc_C3R,  (const Ipp32fc* pSrc1, int src1Step, const Ipp32fc* pSrc2,
                                     int src2Step, Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMul_32fc_AC4R, (const Ipp32fc* pSrc1, int src1Step, const Ipp32fc* pSrc2,
                                     int src2Step, Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDiv_32fc_C1R,  (const Ipp32fc* pSrc1, int src1Step, const Ipp32fc* pSrc2,
                                     int src2Step, Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDiv_32fc_C3R,  (const Ipp32fc* pSrc1, int src1Step, const Ipp32fc* pSrc2,
                                     int src2Step, Ipp32fc* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiDiv_32fc_AC4R, (const Ipp32fc* pSrc1, int src1Step, const Ipp32fc* pSrc2,
                                     int src2Step, Ipp32fc* pDst, int dstStep, IppiSize roiSize))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAdd_16sc_C1IRSfs, ippiAdd_16sc_C3IRSfs, ippiAdd_16sc_AC4IRSfs,
//              ippiSub_16sc_C1IRSfs, ippiSub_16sc_C3IRSfs, ippiSub_16sc_AC4IRSfs,
//              ippiMul_16sc_C1IRSfs, ippiMul_16sc_C3IRSfs, ippiMul_16sc_AC4IRSfs,
//              ippiDiv_16sc_C1IRSfs, ippiDiv_16sc_C3IRSfs, ippiDiv_16sc_AC4IRSfs
//
//  Purpose:    Adds, subtracts, multiplies, or divides pixel values of two
//              source images and places the results in the first source image.
//
//  Returns:
//    ippStsNoErr              OK
//    iippStsNullPtrErr        One of the pointers is NULL
//    ippStsSizeErr            The roiSize has a field with negative or zero value
//    ippStsStepErr            Any of the step values is less than or equal to zero
//    ippStsDivByZero          For division only - a warning that a divisor value (pixel value
//                             of the second image) equals zero, the function execution is continued.
//                               If a dividend is equal to zero, then the result is zero;
//                               if it is greater than zero, then the result is zero,
//                               if it is less than zero, then the result is zero
//
//  Parameters:
//    pSrc                     Pointer to the source image
//    srcStep                  Step through the source image
//    pSrcDst                  Pointer to the source and destination image
//    srcDstStep               Step through the source and destination image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiAdd_16sc_C1IRSfs,  (const Ipp16sc* pSrc, int srcStep, Ipp16sc* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16sc_C3IRSfs,  (const Ipp16sc* pSrc, int srcStep, Ipp16sc* pSrcDst,
                                         int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16sc_AC4IRSfs, (const Ipp16sc* pSrc, int srcStep, Ipp16sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16sc_C1IRSfs,  (const Ipp16sc* pSrc, int srcStep, Ipp16sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16sc_C3IRSfs,  (const Ipp16sc* pSrc, int srcStep, Ipp16sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16sc_AC4IRSfs, (const Ipp16sc* pSrc, int srcStep, Ipp16sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16sc_C1IRSfs,  (const Ipp16sc* pSrc, int srcStep, Ipp16sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16sc_C3IRSfs,  (const Ipp16sc* pSrc, int srcStep, Ipp16sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16sc_AC4IRSfs, (const Ipp16sc* pSrc, int srcStep, Ipp16sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16sc_C1IRSfs,  (const Ipp16sc* pSrc, int srcStep, Ipp16sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16sc_C3IRSfs,  (const Ipp16sc* pSrc, int srcStep, Ipp16sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16sc_AC4IRSfs, (const Ipp16sc* pSrc, int srcStep, Ipp16sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAdd_16sc_C1RSfs, ippiAdd_16sc_C3RSfs, ippiAdd_16sc_AC4RSfs,
//              ippiSub_16sc_C1RSfs, ippiSub_16sc_C3RSfs, ippiSub_16sc_AC4RSfs,
//              ippiMul_16sc_C1RSfs, ippiMul_16sc_C3RSfs, ippiMul_16sc_AC4RSfs,
//              ippiDiv_16sc_C1RSfs, ippiDiv_16sc_C3RSfs, ippiDiv_16sc_AC4RSfs
//
//  Purpose:    Adds, subtracts, multiplies, or divides pixel values of two
//              source images and places the results in the destination image.
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            The roiSize has a field with negative or zero value
//    ippStsStepErr            One of the step values is less than or equal to zero
//    ippStsDivByZero          For division only - a warning that a divisor value (pixel value
//                             of the second image) equals zero, the function execution is continued.
//                               If a dividend is equal to zero, then the result is zero;
//                               if it is greater than zero, then the result is zero,
//                               if it is less than zero, then the result is zero
//  Parameters:
//    pSrc1, pSrc2             Pointers to source images
//    src1Step, src2Step       Steps through the source images
//    pDst                     Pointer to the destination image
//    dstStep                  Step through the destination image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiAdd_16sc_C1RSfs,  (const Ipp16sc* pSrc1, int src1Step, const Ipp16sc* pSrc2,
                                        int src2Step, Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16sc_C3RSfs,  (const Ipp16sc* pSrc1, int src1Step, const Ipp16sc* pSrc2,
                                        int src2Step, Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_16sc_AC4RSfs, (const Ipp16sc* pSrc1, int src1Step, const Ipp16sc* pSrc2,
                                        int src2Step, Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16sc_C1RSfs,  (const Ipp16sc* pSrc1, int src1Step, const Ipp16sc* pSrc2,
                                        int src2Step, Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16sc_C3RSfs,  (const Ipp16sc* pSrc1, int src1Step, const Ipp16sc* pSrc2,
                                        int src2Step, Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_16sc_AC4RSfs, (const Ipp16sc* pSrc1, int src1Step, const Ipp16sc* pSrc2,
                                        int src2Step, Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16sc_C1RSfs,  (const Ipp16sc* pSrc1, int src1Step, const Ipp16sc* pSrc2,
                                        int src2Step, Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16sc_C3RSfs,  (const Ipp16sc* pSrc1, int src1Step, const Ipp16sc* pSrc2,
                                        int src2Step, Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_16sc_AC4RSfs, (const Ipp16sc* pSrc1, int src1Step, const Ipp16sc* pSrc2,
                                        int src2Step, Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16sc_C1RSfs,  (const Ipp16sc* pSrc1, int src1Step, const Ipp16sc* pSrc2,
                                        int src2Step, Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16sc_C3RSfs,  (const Ipp16sc* pSrc1, int src1Step, const Ipp16sc* pSrc2,
                                        int src2Step, Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_16sc_AC4RSfs, (const Ipp16sc* pSrc1, int src1Step, const Ipp16sc* pSrc2,
                                        int src2Step, Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAdd_32sc_C1IRSfs, ippiAdd_32sc_C3IRSfs, ippiAdd_32sc_AC4IRSfs,
//              ippiSub_32sc_C1IRSfs, ippiSub_32sc_C3IRSfs, ippiSub_32sc_AC4IRSfs,
//              ippiMul_32sc_C1IRSfs, ippiMul_32sc_C3IRSfs, ippiMul_32sc_AC4IRSfs,
//              ippiDiv_32sc_C1IRSfs, ippiDiv_32sc_C3IRSfs, ippiDiv_32sc_AC4IRSfs
//
//  Purpose:    Adds, subtracts, multiplies, or divides pixel values of two
//              source images and places the results in the first source image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            The roiSize has a field with negative or zero value
//    ippStsStepErr            One of the step values is less than or equal to zero
//    ippStsDivByZero          For division only - a warning that a divisor value (pixel value
//                             of the second image) equals zero, the function execution is continued.
//                               If a dividend is equal to zero, then the result is zero;
//                               if it is greater than zero, then the result is IPP_MAX_32S,
//                               if it is less than zero, then the result is IPP_MIN_32S
//  Parameters:
//    pSrc                     Pointer to the second source image
//    srcStep                  Step through the second source image
//    pSrcDst                  Pointer to the first source/destination image
//    srcDstStep               Step through the first source/destination image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiAdd_32sc_C1IRSfs,  (const Ipp32sc* pSrc, int srcStep, Ipp32sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_32sc_C3IRSfs,  (const Ipp32sc* pSrc, int srcStep, Ipp32sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_32sc_AC4IRSfs, (const Ipp32sc* pSrc, int srcStep, Ipp32sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_32sc_C1IRSfs,  (const Ipp32sc* pSrc, int srcStep, Ipp32sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_32sc_C3IRSfs,  (const Ipp32sc* pSrc, int srcStep, Ipp32sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_32sc_AC4IRSfs, (const Ipp32sc* pSrc, int srcStep, Ipp32sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_32sc_C1IRSfs,  (const Ipp32sc* pSrc, int srcStep, Ipp32sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_32sc_C3IRSfs,  (const Ipp32sc* pSrc, int srcStep, Ipp32sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_32sc_AC4IRSfs, (const Ipp32sc* pSrc, int srcStep, Ipp32sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_32sc_C1IRSfs,  (const Ipp32sc* pSrc, int srcStep, Ipp32sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_32sc_C3IRSfs,  (const Ipp32sc* pSrc, int srcStep, Ipp32sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_32sc_AC4IRSfs, (const Ipp32sc* pSrc, int srcStep, Ipp32sc* pSrcDst,
                                          int srcDstStep, IppiSize roiSize, int scaleFactor))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAdd_32sc_C1RSfs, ippiAdd_32sc_C3RSfs, ippiAdd_32sc_AC4RSfs,
//              ippiSub_32sc_C1RSfs, ippiSub_32sc_C3RSfs, ippiSub_32sc_AC4RSfs,
//              ippiMul_32sc_C1RSfs, ippiMul_32sc_C3RSfs, ippiMul_32sc_AC4RSfs,
//              ippiDiv_32sc_C1RSfs, ippiDiv_32sc_C3RSfs, ippiDiv_32sc_AC4RSfs
//
//  Purpose:    Adds, subtracts, multiplies, or divides pixel values of two
//              source images and places the results in the destination image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            The roiSize has a field with negative or zero value
//    ippStsStepErr            One of the step values is less than or equal to zero
//    ippStsDivByZero          For division only - a warning that a divisor value (pixel value
//                             of the second image) equals zero, the function execution is continued.
//                               If a dividend is equal to zero, then the result is zero;
//                               if it is greater than zero, then the result is IPP_MAX_32S,
//                               if it is less than zero, then the result is IPP_MIN_32S
//  Parameters:
//    pSrc1, pSrc2             Pointers to source images
//    src1Step, src2Step       The steps of the source images
//    pDst                     The pointer to the destination image
//    dstStep                  The step of the destination image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiAdd_32sc_C1RSfs,  (const Ipp32sc* pSrc1, int src1Step, const Ipp32sc* pSrc2,
                                         int src2Step, Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_32sc_C3RSfs,  (const Ipp32sc* pSrc1, int src1Step, const Ipp32sc* pSrc2,
                                         int src2Step, Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAdd_32sc_AC4RSfs, (const Ipp32sc* pSrc1, int src1Step, const Ipp32sc* pSrc2,
                                         int src2Step, Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_32sc_C1RSfs,  (const Ipp32sc* pSrc1, int src1Step, const Ipp32sc* pSrc2,
                                         int src2Step, Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_32sc_C3RSfs,  (const Ipp32sc* pSrc1, int src1Step, const Ipp32sc* pSrc2,
                                         int src2Step, Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSub_32sc_AC4RSfs, (const Ipp32sc* pSrc1, int src1Step, const Ipp32sc* pSrc2,
                                         int src2Step, Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_32sc_C1RSfs,  (const Ipp32sc* pSrc1, int src1Step, const Ipp32sc* pSrc2,
                                         int src2Step, Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_32sc_C3RSfs,  (const Ipp32sc* pSrc1, int src1Step, const Ipp32sc* pSrc2,
                                         int src2Step, Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMul_32sc_AC4RSfs, (const Ipp32sc* pSrc1, int src1Step, const Ipp32sc* pSrc2,
                                         int src2Step, Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_32sc_C1RSfs,  (const Ipp32sc* pSrc1, int src1Step, const Ipp32sc* pSrc2,
                                         int src2Step, Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_32sc_C3RSfs,  (const Ipp32sc* pSrc1, int src1Step, const Ipp32sc* pSrc2,

                                        int src2Step, Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_32sc_AC4RSfs, (const Ipp32sc* pSrc1, int src1Step, const Ipp32sc* pSrc2,
                                        int src2Step, Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAddC_16sc_C1IRSfs, ippiAddC_16sc_C3IRSfs, ippiAddC_16sc_AC4IRSfs,
//              ippiSubC_16sc_C1IRSfs, ippiSubC_16sc_C3IRSfs, ippiSubC_16sc_AC4IRSfs,
//              ippiMulC_16sc_C1IRSfs, ippiMulC_16sc_C3IRSfs, ippiMulC_16sc_AC4IRSfs,
//              ippiDivC_16sc_C1IRSfs, ippiDivC_16sc_C3IRSfs, ippiDivC_16sc_AC4IRSfs
//
//  Purpose:    Adds, subtracts, multiplies, or divides pixel values of an image
//              and a constant and places the results in the same image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         The pointer is NULL
//    ippStsSizeErr            The roiSize has a field with negative or zero value
//    ippStsStepErr            The step value is less than or equal to zero
//    ippStsDivByZeroErr       The constant is equal to zero (for division)
//
//  Parameters:
//    value                  The constant value (constant vector for multi-channel images)
//    pSrcDst                Pointer to the image
//    srcDstStep             Step through the image
//    roiSize                Size of the ROI
//    scaleFactor            Scale factor
*/
IPPAPI(IppStatus, ippiAddC_16sc_C1IRSfs,  (Ipp16sc value, Ipp16sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16sc_C3IRSfs,  (const Ipp16sc value[3], Ipp16sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16sc_AC4IRSfs, (const Ipp16sc value[3], Ipp16sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16sc_C1IRSfs,  (Ipp16sc value, Ipp16sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16sc_C3IRSfs,  (const Ipp16sc value[3], Ipp16sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16sc_AC4IRSfs, (const Ipp16sc value[3], Ipp16sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16sc_C1IRSfs,  (Ipp16sc value, Ipp16sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16sc_C3IRSfs,  (const Ipp16sc value[3], Ipp16sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16sc_AC4IRSfs, (const Ipp16sc value[3], Ipp16sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16sc_C1IRSfs,  (Ipp16sc value, Ipp16sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16sc_C3IRSfs,  (const Ipp16sc value[3], Ipp16sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16sc_AC4IRSfs, (const Ipp16sc value[3], Ipp16sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))

/* //////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAddC_16sc_C1RSfs, ippiAddC_16sc_C3RSfs, ippiAddC_16sc_AC4RSfs,
//              ippiSubC_16sc_C1RSfs, ippiSubC_16sc_C3RSfs, ippiSubC_16sc_AC4RSfs,
//              ippiMulC_16sc_C1RSfs, ippiMulC_16sc_C3RSfs, ippiMulC_16sc_AC4RSfs
//              ippiDivC_16sc_C1RSfs, ippiDivC_16sc_C3RSfs, ippiDivC_16sc_AC4RSfs
//
//
//  Purpose:    Adds, subtracts, multiplies, or divides pixel values of an image
//              and a constant and places the results in the destination image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            The roiSize has a field with negative or zero value
//    ippStsStepErr            One of the step values is less than or equal to zero
//    ippStsDivByZeroErr       The constant is equal to zero (for division)
//
//  Parameters:
//    value                    The constant value (constant vector for multi-channel images)
//    pSrc                     Pointer to the source image
//    srcStep                  Step through the source image
//    pDst                     Pointer to the destination image
//    dstStep                  Step through the destination image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiAddC_16sc_C1RSfs,  (const Ipp16sc* pSrc, int srcStep, Ipp16sc value, Ipp16sc* pDst,
                                          int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16sc_C3RSfs,  (const Ipp16sc* pSrc, int srcStep, const Ipp16sc value[3],
                                          Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_16sc_AC4RSfs, (const Ipp16sc* pSrc, int srcStep, const Ipp16sc value[3],
                                          Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16sc_C1RSfs,  (const Ipp16sc* pSrc, int srcStep, Ipp16sc value, Ipp16sc* pDst,
                                          int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16sc_C3RSfs,  (const Ipp16sc* pSrc, int srcStep, const Ipp16sc value[3],
                                          Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_16sc_AC4RSfs, (const Ipp16sc* pSrc, int srcStep, const Ipp16sc value[3],
                                          Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16sc_C1RSfs,  (const Ipp16sc* pSrc, int srcStep, Ipp16sc value, Ipp16sc* pDst,
                                          int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16sc_C3RSfs,  (const Ipp16sc* pSrc, int srcStep, const Ipp16sc value[3],
                                          Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_16sc_AC4RSfs, (const Ipp16sc* pSrc, int srcStep, const Ipp16sc value[3],
                                          Ipp16sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16sc_C1RSfs,  (const Ipp16sc* pSrc,
                                          int srcStep, Ipp16sc value, Ipp16sc* pDst,
                                          int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16sc_C3RSfs,  (const Ipp16sc* pSrc,
                                          int srcStep, const Ipp16sc value[3], Ipp16sc* pDst,
                                          int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_16sc_AC4RSfs, (const Ipp16sc* pSrc,
                                          int srcStep, const Ipp16sc value[3], Ipp16sc* pDst,
                                          int dstStep, IppiSize roiSize, int scaleFactor))


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAddC_32sc_C1IRSfs, ippiAddC_32sc_C3IRSfs, ippiAddC_32sc_AC4IRSfs,
//              ippiSubC_32sc_C1IRSfs, ippiSubC_32sc_C3IRSfs, ippiSubC_32sc_AC4IRSfs,
//              ippiMulC_32sc_C1IRSfs, ippiMulC_32sc_C3IRSfs, ippiMulC_32sc_AC4IRSfs,
//              ippiDivC_32sc_C1IRSfs, ippiDivC_32sc_C3IRSfs, ippiDivC_32sc_AC4IRSfs
//
//  Purpose:    Adds, subtracts, multiplies, or divides pixel values of an image
//              and a constant and places the results in the same image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         The pointer is NULL
//    ippStsSizeErr            The roiSize has a field with negative or zero value
//    ippStsStepErr            The step value is less than or equal to zero
//    ippStsDivByZeroErr       The constant is equal to zero (for division)
//
//  Parameters:
//    value                    The constant value (constant vector for multi-channel images)
//    pSrcDst                  Pointer to the image
//    srcDstStep               Step through the image
//    roiSize                  Size of the ROI
//    scaleFactor              Scale factor
*/
IPPAPI(IppStatus, ippiAddC_32sc_C1IRSfs,  (Ipp32sc value, Ipp32sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_32sc_C3IRSfs,  (const Ipp32sc value[3], Ipp32sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_32sc_AC4IRSfs, (const Ipp32sc value[3], Ipp32sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_32sc_C1IRSfs,  (Ipp32sc value, Ipp32sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_32sc_C3IRSfs,  (const Ipp32sc value[3], Ipp32sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_32sc_AC4IRSfs, (const Ipp32sc value[3], Ipp32sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_32sc_C1IRSfs,  (Ipp32sc value, Ipp32sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_32sc_C3IRSfs,  (const Ipp32sc value[3], Ipp32sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_32sc_AC4IRSfs, (const Ipp32sc value[3], Ipp32sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_32sc_C1IRSfs,  (Ipp32sc value, Ipp32sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_32sc_C3IRSfs,  (const Ipp32sc value[3], Ipp32sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_32sc_AC4IRSfs, (const Ipp32sc value[3], Ipp32sc* pSrcDst, int srcDstStep,
                                           IppiSize roiSize, int scaleFactor))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAddC_32sc_C1RSfs, ippiAddC_32sc_C3RSfs, ippiAddC_32sc_AC4RSfs,
//              ippiSubC_32sc_C1RSfs, ippiSubC_32sc_C3RSfs, ippiSubC_32sc_AC4RSfs,
//              ippiMulC_32sc_C1RSfs, ippiMulC_32sc_C3RSfs, ippiMulC_32sc_AC4RSfs,
//              ippiDivC_32sc_C1RSfs, ippiDivC_32sc_C3RSfs, ippiDivC_32sc_AC4RSfs
//
//  Purpose:    Adds, subtracts, multiplies, or divides pixel values of an image
//              and a constant and places the results in the destination image.
//
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            The roiSize has a field with negative or zero value
//    ippStsStepErr            Any of the step values is less than or equal to zero
//    ippStsDivByZeroErr       The constant is equal to zero (for division)
//
//
//  Parameters:
//    value                    The constant value (constant vector for multi-channel images)
//    pSrc                     Pointer to the source image
//    srcStep                  Step through the source image
//    pDst                     Pointer to the destination image
//    dstStep                  Step through the destination image
//    roiSize                  ROI
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiAddC_32sc_C1RSfs,  (const Ipp32sc* pSrc, int srcStep, Ipp32sc value, Ipp32sc* pDst,
                                          int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_32sc_C3RSfs,  (const Ipp32sc* pSrc, int srcStep, const Ipp32sc value[3],
                                          Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiAddC_32sc_AC4RSfs, (const Ipp32sc* pSrc, int srcStep, const Ipp32sc value[3],
                                          Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_32sc_C1RSfs,  (const Ipp32sc* pSrc, int srcStep, Ipp32sc value, Ipp32sc* pDst,
                                          int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_32sc_C3RSfs,  (const Ipp32sc* pSrc, int srcStep, const Ipp32sc value[3],
                                          Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiSubC_32sc_AC4RSfs, (const Ipp32sc* pSrc, int srcStep, const Ipp32sc value[3],
                                          Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_32sc_C1RSfs,  (const Ipp32sc* pSrc, int srcStep, Ipp32sc value, Ipp32sc* pDst,
                                          int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_32sc_C3RSfs,  (const Ipp32sc* pSrc, int srcStep, const Ipp32sc value[3],
                                          Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulC_32sc_AC4RSfs, (const Ipp32sc* pSrc, int srcStep, const Ipp32sc value[3],
                                          Ipp32sc* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_32sc_C1RSfs,  (const Ipp32sc* pSrc,
                                          int srcStep, Ipp32sc value, Ipp32sc* pDst,
                                          int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_32sc_C3RSfs,  (const Ipp32sc* pSrc,
                                          int srcStep, const Ipp32sc value[3], Ipp32sc* pDst,
                                          int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiDivC_32sc_AC4RSfs, (const Ipp32sc* pSrc,
                                          int srcStep, const Ipp32sc value[3], Ipp32sc* pDst,
                                          int dstStep, IppiSize roiSize, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////////////////////////
//                      Multiplication with Scaling
///////////////////////////////////////////////////////////////////////////////////////////////// */
/*
//  Names:              ippiMulScale, ippiMulCScale
//
//  Purpose:            Multiplies pixel values of two images (MulScale),
//                      or pixel values of an image by a constant (MulScaleC) and scales the products
//
//  Parameters:
//   value              The constant value (constant vector for multi-channel images)
//   pSrc               Pointer to the source image
//   srcStep            Step through the source image
//   pSrcDst            Pointer to the source/destination image (in-place operations)
//   srcDstStep         Step through the source/destination image (in-place operations)
//   pSrc1              Pointer to the first source image
//   src1Step           Step through the first source image
//   pSrc2              Pointer to the second source image
//   src2Step           Step through the second source image
//   pDst               Pointer to the destination image
//   dstStep            Step through the destination image
//   roiSize            Size of the image ROI
//
//  Returns:
//   ippStsNullPtrErr   One of the pointers is NULL
//   ippStsStepErr      One of the step values is less than or equal to zero
//   ippStsSizeErr      The roiSize has a field with negative or zero value
//   ippStsNoErr        otherwise
*/

IPPAPI(IppStatus, ippiMulScale_8u_C1R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_8u_C3R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_8u_C4R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_8u_AC4R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_8u_C1IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_8u_C3IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_8u_C4IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_8u_AC4IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp8u value, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_8u_C3R, (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_8u_C4R, (const Ipp8u* pSrc, int srcStep, const Ipp8u value[4], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_8u_AC4R, (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_8u_C1IR, (Ipp8u value, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_8u_C3IR, (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_8u_C4IR, (const Ipp8u value[4], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_8u_AC4IR, (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiMulScale_16u_C1R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_16u_C3R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_16u_C4R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_16u_AC4R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_16u_C1IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_16u_C3IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_16u_C4IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulScale_16u_AC4IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_16u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp16u value, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_16u_C3R, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_16u_C4R, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[4], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_16u_AC4R, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_16u_C1IR, (Ipp16u value, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_16u_C3IR, (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_16u_C4IR, (const Ipp16u value[4], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulCScale_16u_AC4IR, (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//              Dot product of two images
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiDotProd
//  Purpose:        Computes the dot product of two images
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsStepErr      One of the step values is equal to zero
//  Parameters:
//    pSrc1       Pointer to the first source image.
//    src1Step    Step in bytes through the first source image
//    pSrc2       Pointer to the second source image.
//    src2Step    Step in bytes through the  source image
//    roiSize     Size of the source image ROI.
//    pDp         Pointer to the result (one-channel data) or array (multi-channel data) containing computed dot products of channel values of pixels in the source images.
//    hint        Option to select the algorithmic implementation of the function
//  Notes:
*/
IPPAPI(IppStatus, ippiDotProd_8u64f_C1R,(const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, IppiSize roiSize, Ipp64f *pDp))
IPPAPI(IppStatus, ippiDotProd_8s64f_C1R,(const Ipp8s* pSrc1, int src1Step, const Ipp8s* pSrc2, int src2Step, IppiSize roiSize, Ipp64f *pDp))
IPPAPI(IppStatus, ippiDotProd_16u64f_C1R,(const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, IppiSize roiSize, Ipp64f *pDp))
IPPAPI(IppStatus, ippiDotProd_16s64f_C1R,(const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2, int src2Step, IppiSize roiSize, Ipp64f *pDp))
IPPAPI(IppStatus, ippiDotProd_32u64f_C1R,(const Ipp32u* pSrc1, int src1Step, const Ipp32u* pSrc2, int src2Step, IppiSize roiSize, Ipp64f *pDp))
IPPAPI(IppStatus, ippiDotProd_32s64f_C1R,(const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, IppiSize roiSize, Ipp64f *pDp))
IPPAPI(IppStatus, ippiDotProd_32f64f_C1R,(const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2, int src2Step, IppiSize roiSize, Ipp64f *pDp, IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiDotProd_8u64f_C3R,(const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3]))
IPPAPI(IppStatus, ippiDotProd_8s64f_C3R,(const Ipp8s* pSrc1, int src1Step, const Ipp8s* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3]))
IPPAPI(IppStatus, ippiDotProd_16u64f_C3R,(const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3]))
IPPAPI(IppStatus, ippiDotProd_16s64f_C3R,(const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3]))
IPPAPI(IppStatus, ippiDotProd_32u64f_C3R,(const Ipp32u* pSrc1, int src1Step, const Ipp32u* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3]))
IPPAPI(IppStatus, ippiDotProd_32s64f_C3R,(const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3]))
IPPAPI(IppStatus, ippiDotProd_32f64f_C3R,(const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiDotProd_8u64f_C4R,(const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[4]))
IPPAPI(IppStatus, ippiDotProd_8s64f_C4R,(const Ipp8s* pSrc1, int src1Step, const Ipp8s* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[4]))
IPPAPI(IppStatus, ippiDotProd_16u64f_C4R,(const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[4]))
IPPAPI(IppStatus, ippiDotProd_16s64f_C4R,(const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[4]))
IPPAPI(IppStatus, ippiDotProd_32u64f_C4R,(const Ipp32u* pSrc1, int src1Step, const Ipp32u* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[4]))
IPPAPI(IppStatus, ippiDotProd_32s64f_C4R,(const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[4]))
IPPAPI(IppStatus, ippiDotProd_32f64f_C4R,(const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[4], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiDotProd_8u64f_AC4R,(const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3]))
IPPAPI(IppStatus, ippiDotProd_8s64f_AC4R,(const Ipp8s* pSrc1, int src1Step, const Ipp8s* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3]))
IPPAPI(IppStatus, ippiDotProd_16u64f_AC4R,(const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3]))
IPPAPI(IppStatus, ippiDotProd_16s64f_AC4R,(const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3]))
IPPAPI(IppStatus, ippiDotProd_32u64f_AC4R,(const Ipp32u* pSrc1, int src1Step, const Ipp32u* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3]))
IPPAPI(IppStatus, ippiDotProd_32s64f_AC4R,(const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3]))
IPPAPI(IppStatus, ippiDotProd_32f64f_AC4R,(const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2, int src2Step, IppiSize roiSize, Ipp64f pDp[3], IppHintAlgorithm hint))

/* /////////////////////////////////////////////////////////////////////////////
//              Dot product of taps vector and columns,
//                  which are placed in stripe of rows
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDotProdCol_32f_L2
//
//  Purpose:    Calculates the dot product of taps vector and columns,
//      which are placed in stripe of rows; useful for external vertical
//      filtering pipeline implementation.
//
//  Parameters:
//    ppSrcRow              pointer to set of rows
//    pTaps                 pointer to taps vector
//    tapsLen               taps length and (equal to number of rows)
//    pDst                  pointer where to store the result row
//    width                 width of source and destination rows
//
//  Returns:
//    ippStsNoErr           OK
//    ippStsNullPtrErr      one of the pointers is NULL
//    ippStsSizeErr         width is less than or equal to zero
*/
IPPAPI(IppStatus, ippiDotProdCol_32f_L2, (
        const Ipp32f *const   ppSrcRow[],
        const Ipp32f         *pTaps,
              int             tapsLen,
              Ipp32f         *pDst,
              int             width))



/* /////////////////////////////////////////////////////////////////////////////
//              Vector Multiplication of Images in RCPack2D Format
///////////////////////////////////////////////////////////////////////////// */
/*  Name:               ippiMulPack, ippiMulPackConj
//
//  Purpose:            Multiplies pixel values of two images in RCPack2D format
//                      and store the result also in PCPack2D format
//
//  Returns:
//      ippStsNoErr       No errors
//      ippStsNullPtrErr  One of the pointers is NULL
//      ippStsStepErr     One of the step values is zero or negative
//      ippStsSizeErr     The roiSize has a field with negative or zero value
//
//  Parameters:
//      pSrc            Pointer to the source image for in-place operation
//      pSrcDst         Pointer to the source/destination image for in-place operation
//      srcStep         Step through the source image for in-place operation
//      srcDstStep      Step through the source/destination image for in-place operation
//      pSrc1           Pointer to the first source image
//      src1Step        Step through the first source image
//      pSrc2           Pointer to the second source image
//      src1Step        Step through the second source image
//      pDst            Pointer to the destination image
//      dstStep         Step through the destination image
//      roiSize         Size of the source and destination ROI
//      scaleFactor     Scale factor
//
//  Notes:              Both in-place and not-in-place operations are supported
//                      ippiMulPackConj functions are only for float data
*/

IPPAPI(IppStatus, ippiMulPack_16s_C1IRSfs, (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst, int srcDstStep,
                                        IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_16s_C3IRSfs, (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst, int srcDstStep,
                                        IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_16s_C4IRSfs, (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst, int srcDstStep,
                                        IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_16s_AC4IRSfs, (const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst, int srcDstStep,
                                        IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_16s_C1RSfs, (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2, int src2Step,
                                        Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_16s_C3RSfs, (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2, int src2Step,
                                        Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_16s_C4RSfs, (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2, int src2Step,
                                        Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_16s_AC4RSfs, (const Ipp16s* pSrc1, int src1Step, const Ipp16s* pSrc2, int src2Step,
                                        Ipp16s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus, ippiMulPack_32s_C1IRSfs, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep,
                                        IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_32s_C3IRSfs, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep,
                                        IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_32s_C4IRSfs, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep,
                                        IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_32s_AC4IRSfs, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep,
                                        IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_32s_C1RSfs, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step,
                                        Ipp32s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_32s_C3RSfs, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step,
                                        Ipp32s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_32s_C4RSfs, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step,
                                        Ipp32s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))
IPPAPI(IppStatus, ippiMulPack_32s_AC4RSfs, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step,
                                        Ipp32s* pDst, int dstStep, IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus, ippiMulPack_32f_C1IR, (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPack_32f_C3IR, (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPack_32f_C4IR, (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPack_32f_AC4IR, (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPack_32f_C1R, (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2, int src2Step,
                                        Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPack_32f_C3R, (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2, int src2Step,
                                        Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPack_32f_C4R, (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2, int src2Step,
                                        Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPack_32f_AC4R, (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2, int src2Step,
                                        Ipp32f* pDst, int dstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiMulPackConj_32f_C1IR, (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPackConj_32f_C3IR, (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPackConj_32f_C4IR, (const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPackConj_32f_AC4IR,(const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPackConj_32f_C1R, (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2, int src2Step,
                                            Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPackConj_32f_C3R, (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2, int src2Step,
                                            Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPackConj_32f_C4R, (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2, int src2Step,
                                            Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiMulPackConj_32f_AC4R, (const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2, int src2Step,
                                            Ipp32f* pDst, int dstStep, IppiSize roiSize))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiPackToCplxExtend
//
//  Purpose:        Converts an image in RCPack2D format to a complex data image.
//
//  Returns:
//      ippStsNoErr            No errors
//      ippStsNullPtrErr       pSrc == NULL, or pDst == NULL
//      ippStsStepErr          One of the step values is less zero or negative
//      ippStsSizeErr          The srcSize has a field with zero or negative value
//
//  Parameters:
//    pSrc        Pointer to the source image data (point to pixel (0,0))
//    srcSize     Size of the source image
//    srcStep     Step through  the source image
//    pDst        Pointer to the destination image
//    dstStep     Step through the destination image
//  Notes:
*/

IPPAPI (IppStatus, ippiPackToCplxExtend_32s32sc_C1R, (const Ipp32s* pSrc,
        IppiSize srcSize, int srcStep,
        Ipp32sc* pDst, int dstStep ))

IPPAPI (IppStatus, ippiPackToCplxExtend_32f32fc_C1R, (const Ipp32f* pSrc,
        IppiSize srcSize, int srcStep,
        Ipp32fc* pDst, int dstStep ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiCplxExtendToPack
//
//  Purpose:        Converts an image in complex data format to RCPack2D image.
//
//  Returns:
//      ippStsNoErr            No errors
//      ippStsNullPtrErr       pSrc == NULL, or pDst == NULL
//      ippStsStepErr          One of the step values is less zero or negative
//      ippStsSizeErr          The srcSize has a field with zero or negative value
//
//  Parameters:
//    pSrc        Pointer to the source image data (point to pixel (0,0))
//    srcSize     Size of the source image
//    srcStep     Step through  the source image
//    pDst        Pointer to the destination image
//    dstStep     Step through the destination image
//  Notes:
*/
IPPAPI(IppStatus,ippiCplxExtendToPack_32fc32f_C1R,(const Ipp32fc* pSrc, int srcStep, IppiSize srcSize,
                                                                           Ipp32f* pDst, int dstStep ))

IPPAPI(IppStatus,ippiCplxExtendToPack_32sc32s_C1R,(const Ipp32sc* pSrc, int srcStep, IppiSize srcSize,
                                                                           Ipp32s* pDst, int dstStep ))

IPPAPI(IppStatus,ippiCplxExtendToPack_16sc16s_C1R,(const Ipp16sc* pSrc, int srcStep, IppiSize srcSize,
                                                                           Ipp16s* pDst, int dstStep ))


IPPAPI(IppStatus,ippiCplxExtendToPack_32fc32f_C3R,(const Ipp32fc* pSrc, int srcStep, IppiSize srcSize,
                                                                           Ipp32f* pDst, int dstStep ))

IPPAPI(IppStatus,ippiCplxExtendToPack_32sc32s_C3R,(const Ipp32sc* pSrc, int srcStep, IppiSize srcSize,
                                                                           Ipp32s* pDst, int dstStep ))

IPPAPI(IppStatus,ippiCplxExtendToPack_16sc16s_C3R,(const Ipp16sc* pSrc, int srcStep, IppiSize srcSize,
                                                                           Ipp16s* pDst, int dstStep ))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:  ippiPhasePack_32f_C1R
//          ippiPhasePack_32f_C3R
//          ippiPhasePack_16s_C1RSfs
//          ippiPhasePack_16s_C3RSfs
//  Purpose:
//    Computes  the phase (in radians) of elements of an image in RCPack2D packed format.
//  Parameters:
//    pSrc         Pointer to the source complex image in Pack2D format
//    srcStep      Step through the source image
//    pDst         Pointer to the destination image
//    dstStep      Step through the destination image
//    dstRoiSize   Size of the ROI of destination image
//    scaleFactor  Scale factor (only for integer data)
//  Returns:
//    ippStsNullPtrErr    pSrc or pDst is NULL
//    ippStsSizeErr       The width or height of images is less than or equal to zero
//    ippStsStepErr       srcStep or dstStep is less than or equal to zero
//    ippStsNoErr         Otherwise
*/

IPPAPI(IppStatus, ippiPhasePack_32f_C1R,(const Ipp32f* pSrc, int srcStep,
                                               Ipp32f* pDst, int dstStep,
                                                      IppiSize dstRoiSize))

IPPAPI(IppStatus, ippiPhasePack_32f_C3R,(const Ipp32f* pSrc, int srcStep,
                                               Ipp32f* pDst, int dstStep,
                                                      IppiSize dstRoiSize))

IPPAPI(IppStatus, ippiPhasePack_32s_C1RSfs,(const Ipp32s* pSrc, int srcStep,
                                                  Ipp32s* pDst, int dstStep,
                                           IppiSize dstRoiSize, int scaleFactor))

IPPAPI(IppStatus, ippiPhasePack_32s_C3RSfs,(const Ipp32s* pSrc, int srcStep,
                                                  Ipp32s* pDst, int dstStep,
                                           IppiSize dstRoiSize, int scaleFactor))

IPPAPI(IppStatus, ippiPhasePack_16s_C1RSfs,(const Ipp16s* pSrc, int srcStep,
                                                  Ipp16s* pDst, int dstStep,
                                           IppiSize dstRoiSize, int scaleFactor))

IPPAPI(IppStatus, ippiPhasePack_16s_C3RSfs,(const Ipp16s* pSrc, int srcStep,

                                                  Ipp16s* pDst, int dstStep,
                                           IppiSize dstRoiSize, int scaleFactor))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:  ippiMagnitudePack_32f_C1R
//          ippiMagnitudePack_32f_C3R
//          ippiMagnitudePack_32s_C1RSfs
//          ippiMagnitudePack_32s_C3RSfs
//          ippiMagnitudePack_16s_C1RSfs
//          ippiMagnitudePack_16s_C3RSfs
//  Purpose:
//    Computes magnitude of elements of an image in RCPack2D packed format.
//  Parameters:
//    pSrc        Pointer to the source image in Pack2D format
//    srcStep     Step through the source image
//    pDst        Pointer to the destination image to store the magnitude components
//    dstStep     Step through the destination image
//    dstRoiSize  Size of the destination ROI
//    scaleFactor Scale factor (only for integer data)
//  Returns:
//    ippStsNullPtrErr    pSrc or pDst is NULL
//    ippStsSizeErr       The width or height of images is less than or equal to zero
//    ippStsStepErr       srcStep or dstStep is less than or equal to zero
//    ippStsNoErr         Otherwise
*/

IPPAPI(IppStatus, ippiMagnitudePack_32f_C1R,(const Ipp32f* pSrc, int srcStep,
                                                   Ipp32f* pDst, int dstStep,
                                                           IppiSize dstRoiSize))

IPPAPI(IppStatus, ippiMagnitudePack_32f_C3R,(const Ipp32f* pSrc, int srcStep,
                                                   Ipp32f* pDst, int dstStep,
                                                           IppiSize dstRoiSize))

IPPAPI(IppStatus, ippiMagnitudePack_16s_C1RSfs,(const Ipp16s* pSrc, int srcStep,
                                                      Ipp16s* pDst, int dstStep,
                                               IppiSize dstRoiSize, int scaleFactor))

IPPAPI(IppStatus, ippiMagnitudePack_16s_C3RSfs,(const Ipp16s* pSrc, int srcStep,
                                                      Ipp16s* pDst, int dstStep,
                                               IppiSize dstRoiSize, int scaleFactor))

IPPAPI(IppStatus, ippiMagnitudePack_32s_C1RSfs,(const Ipp32s* pSrc, int srcStep,
                                                   Ipp32s* pDst, int dstStep,
                                               IppiSize dstRoiSize, int scaleFactor))

IPPAPI(IppStatus, ippiMagnitudePack_32s_C3RSfs,(const Ipp32s* pSrc, int srcStep,
                                                   Ipp32s* pDst, int dstStep,
                                               IppiSize dstRoiSize, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:  ippiMagnitude_32fc32f_C1R
//          ippiMagnitude_32fc32f_C3R
//          ippiMagnitude_32sc32s_C1RSfs
//          ippiMagnitude_32sc32s_C3RSfs
//          ippiMagnitude_16sc16s_C1RSfs
//          ippiMagnitude_16sc16s_C3RSfs
//          ippiMagnitude_16uc16u_C1RSfs
//          ippiMagnitude_16uc16u_C3RSfs
//  Purpose:
//    Computes magnitude of elements of a complex data image.
//  Parameters:
//    pSrc        Pointer to the source image in common complex data format
//    srcStep     Step through the source image
//    pDst        Pointer to the destination image to store magnitude components
//    dstStep     Step through the destination image
//    roiSize     Size of the ROI
//    scaleFactor Scale factor (only for integer data)
//  Returns:
//    ippStsNullPtrErr    pSrc or pDst is NULL
//    ippStsSizeErr       The width or height of images is less than or equal to zero
//    ippStsStepErr       srcStep or dstStep is less than or equal to zero
//    ippStsNoErr         Otherwise
*/

IPPAPI(IppStatus, ippiMagnitude_32fc32f_C1R,(const Ipp32fc* pSrc, int srcStep,
                                                    Ipp32f* pDst, int dstStep,
                                                            IppiSize roiSize))

IPPAPI(IppStatus, ippiMagnitude_32fc32f_C3R,(const Ipp32fc* pSrc, int srcStep,
                                                    Ipp32f* pDst, int dstStep,
                                                            IppiSize roiSize))

IPPAPI(IppStatus, ippiMagnitude_16sc16s_C1RSfs,(const Ipp16sc* pSrc, int srcStep,
                                                      Ipp16s*  pDst, int dstStep,
                                                   IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus, ippiMagnitude_16sc16s_C3RSfs,(const Ipp16sc* pSrc, int srcStep,
                                                      Ipp16s* pDst,  int dstStep,
                                                   IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus, ippiMagnitude_32sc32s_C1RSfs,(const Ipp32sc* pSrc, int srcStep,
                                                    Ipp32s* pDst, int dstStep,
                                                   IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus, ippiMagnitude_32sc32s_C3RSfs,(const Ipp32sc* pSrc, int srcStep,
                                                    Ipp32s* pDst, int dstStep,
                                                   IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus, ippiMagnitude_16uc16u_C1RSfs,(const Ipp16uc* pSrc, int srcStep,
                                                      Ipp16u*  pDst, int dstStep,
                                                      IppiSize roiSize, int ScalFact))

IPPAPI(IppStatus, ippiMagnitude_16uc16u_C3RSfs,(const Ipp16uc* pSrc, int srcStep,
                                                      Ipp16u*  pDst, int dstStep,
                                                      IppiSize roiSize, int ScalFact))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:   ippiPhase_32fc32f_C1R
//           ippiPhase_32fc32f_C3R
//           ippiPhase_16sc16s_C1RSfs
//           ippiPhase_16sc16s_C3RSfs
//           ippiPhase_16uc16u_C1RSfs
//           ippiPhase_16uc16u_C3RSfs
//  Purpose:
//    Computes the phase (in radians) of elements of a complex data image
//  Parameters:
//    pSrc         Pointer to the source image in common complex data format
//    srcStep      Step through the source image
//    pDst         Pointer to the destination image to store the phase components
//    dstStep      Step through the destination image
//    roiSize      Size of the ROI
//    scaleFactor  Scale factor (only for integer data)
//  Returns:
//    ippStsNullPtrErr    pSrc or pDst is NULL
//    ippStsSizeErr       The width or height of images is less than or equal to zero
//    ippStsStepErr       srcStep or dstStep is less than or equal to zero
//    ippStsNoErr         Otherwise
*/

IPPAPI(IppStatus, ippiPhase_32fc32f_C1R,(const Ipp32fc* pSrc, int srcStep,
                                               Ipp32f* pDst, int dstStep,
                                                          IppiSize roiSize))

IPPAPI(IppStatus, ippiPhase_32fc32f_C3R,(const Ipp32fc* pSrc, int srcStep,
                                               Ipp32f* pDst, int dstStep,
                                                           IppiSize roiSize))

IPPAPI(IppStatus, ippiPhase_32sc32s_C1RSfs,(const Ipp32sc* pSrc, int srcStep,
                                                  Ipp32s*  pDst, int dstStep,
                                               IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus, ippiPhase_32sc32s_C3RSfs,(const Ipp32sc* pSrc, int srcStep,
                                                  Ipp32s* pDst,  int dstStep,
                                               IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus, ippiPhase_16sc16s_C1RSfs,(const Ipp16sc* pSrc, int srcStep,
                                                  Ipp16s*  pDst, int dstStep,
                                               IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus, ippiPhase_16sc16s_C3RSfs,(const Ipp16sc* pSrc, int srcStep,
                                                  Ipp16s* pDst,  int dstStep,
                                               IppiSize roiSize, int scaleFactor))

IPPAPI(IppStatus, ippiPhase_16uc16u_C1RSfs,(const Ipp16uc* pSrc, int srcStep,
                                                  Ipp16u*  pDst, int dstStep,
                                                  IppiSize roiSize, int ScalFact))

IPPAPI(IppStatus, ippiPhase_16uc16u_C3RSfs,(const Ipp16uc* pSrc, int srcStep,
                                                  Ipp16u*  pDst, int dstStep,
                                                  IppiSize roiSize, int ScalFact))


/* /////////////////////////////////////////////////////////////////////////////
//                  Alpha Compositing Operations
///////////////////////////////////////////////////////////////////////////// */
/*
//  Contents:
//      ippiAlphaPremul_8u_AC4R,  ippiAlphaPremul_16u_AC4R
//      ippiAlphaPremul_8u_AC4IR, ippiAlphaPremul_16u_AC4IR
//      ippiAlphaPremul_8u_AP4R,  ippiAlphaPremul_16u_AP4R
//      ippiAlphaPremul_8u_AP4IR, ippiAlphaPremul_16u_AP4IR
//   Pre-multiplies pixel values of an image by its alpha values.

//      ippiAlphaPremulC_8u_AC4R,  ippiAlphaPremulC_16u_AC4R
//      ippiAlphaPremulC_8u_AC4IR, ippiAlphaPremulC_16u_AC4IR
//      ippiAlphaPremulC_8u_AP4R,  ippiAlphaPremulC_16u_AP4R
//      ippiAlphaPremulC_8u_AP4IR, ippiAlphaPremulC_16u_AP4IR
//      ippiAlphaPremulC_8u_C4R,   ippiAlphaPremulC_16u_C4R
//      ippiAlphaPremulC_8u_C4IR,  ippiAlphaPremulC_16u_C4IR
//      ippiAlphaPremulC_8u_C3R,   ippiAlphaPremulC_16u_C3R
//      ippiAlphaPremulC_8u_C3IR,  ippiAlphaPremulC_16u_C3IR
//      ippiAlphaPremulC_8u_C1R,   ippiAlphaPremulC_16u_C1R
//      ippiAlphaPremulC_8u_C1IR,  ippiAlphaPremulC_16u_C1IR
//   Pre-multiplies pixel values of an image by constant alpha values.
//
//      ippiAlphaComp_8u_AC4R, ippiAlphaComp_16u_AC4R
//      ippiAlphaComp_8u_AC1R, ippiAlphaComp_16u_AC1R
//   Combines two images using alpha values of both images
//
//      ippiAlphaCompC_8u_AC4R, ippiAlphaCompC_16u_AC4R
//      ippiAlphaCompC_8u_AP4R, ippiAlphaCompC_16u_AP4R
//      ippiAlphaCompC_8u_C4R,  ippiAlphaCompC_16u_C4R
//      ippiAlphaCompC_8u_C3R,  ippiAlphaCompC_16u_C3R
//      ippiAlphaCompC_8u_C1R,  ippiAlphaCompC_16u_C1R
//   Combines two images using constant alpha values
//
//  Types of compositing operation (alphaType)
//      OVER   ippAlphaOver   ippAlphaOverPremul
//      IN     ippAlphaIn     ippAlphaInPremul
//      OUT    ippAlphaOut    ippAlphaOutPremul
//      ATOP   ippAlphaATop   ippAlphaATopPremul
//      XOR    ippAlphaXor    ippAlphaXorPremul
//      PLUS   ippAlphaPlus   ippAlphaPlusPremul
//
//  Type  result pixel           result pixel (Premul)    result alpha
//  OVER  aA*A+(1-aA)*aB*B         A+(1-aA)*B             aA+(1-aA)*aB
//  IN    aA*A*aB                  A*aB                   aA*aB
//  OUT   aA*A*(1-aB)              A*(1-aB)               aA*(1-aB)
//  ATOP  aA*A*aB+(1-aA)*aB*B      A*aB+(1-aA)*B          aA*aB+(1-aA)*aB
//  XOR   aA*A*(1-aB)+(1-aA)*aB*B  A*(1-aB)+(1-aA)*B      aA*(1-aB)+(1-aA)*aB
//  PLUS  aA*A+aB*B                A+B                    aA+aB
//      Here 1 corresponds significance VAL_MAX, multiplication is performed
//      with scaling
//          X * Y => (X * Y) / VAL_MAX
//      and VAL_MAX is the maximum presentable pixel value:
//          VAL_MAX == IPP_MAX_8U  for 8u
//          VAL_MAX == IPP_MAX_16U for 16u
*/

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiAlphaPremul_8u_AC4R,  ippiAlphaPremul_16u_AC4R
//                  ippiAlphaPremul_8u_AC4IR, ippiAlphaPremul_16u_AC4IR
//                  ippiAlphaPremul_8u_AP4R,  ippiAlphaPremul_16u_AP4R
//                  ippiAlphaPremul_8u_AP4IR, ippiAlphaPremul_16u_AP4IR
//
//  Purpose:        Pre-multiplies pixel values of an image by its alpha values
//                  for 4-channel images
//               For channels 1-3
//                      dst_pixel = (src_pixel * src_alpha) / VAL_MAX
//               For alpha-channel (channel 4)
//                      dst_alpha = src_alpha
//  Parameters:
//     pSrc         Pointer to the source image for pixel-order data,
//                  array of pointers to separate source color planes for planar data
//     srcStep      Step through the source image
//     pDst         Pointer to the destination image for pixel-order data,
//                  array of pointers to separate destination color planes for planar data
//     dstStep      Step through the destination image
//     pSrcDst      Pointer to the source/destination image, or array of pointers
//                  to separate source/destination color planes for in-place functions
//     srcDstStep   Step through the source/destination image for in-place functions
//     roiSize      Size of the source and destination ROI
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pSrc == NULL, or pDst == NULL, or pSrcDst == NULL
//     ippStsSizeErr          The roiSize has a field with negative or zero value
*/

IPPAPI (IppStatus, ippiAlphaPremul_8u_AC4R,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u* pDst, int dstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremul_16u_AC4R,
                   ( const Ipp16u* pSrc, int srcStep,
                     Ipp16u* pDst, int dstStep,
                     IppiSize roiSize ))

IPPAPI (IppStatus, ippiAlphaPremul_8u_AC4IR,
                   ( Ipp8u* pSrcDst, int srcDstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremul_16u_AC4IR,
                   ( Ipp16u* pSrcDst, int srcDstStep,
                     IppiSize roiSize ))


IPPAPI (IppStatus, ippiAlphaPremul_8u_AP4R,
                   ( const Ipp8u* const pSrc[4], int srcStep,
                     Ipp8u* const pDst[4], int dstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremul_16u_AP4R,
                   ( const Ipp16u* const pSrc[4], int srcStep,
                     Ipp16u* const pDst[4], int dstStep,
                     IppiSize roiSize ))

IPPAPI (IppStatus, ippiAlphaPremul_8u_AP4IR,
                   ( Ipp8u* const pSrcDst[4], int srcDstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremul_16u_AP4IR,
                   ( Ipp16u* const pSrcDst[4], int srcDstStep,
                     IppiSize roiSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiAlphaPremulC_8u_AC4R,  ippiAlphaPremulC_16u_AC4R
//                  ippiAlphaPremulC_8u_AC4IR, ippiAlphaPremulC_16u_ACI4R
//                  ippiAlphaPremulC_8u_AP4R,  ippiAlphaPremulC_16u_AP4R
//                  ippiAlphaPremulC_8u_AP4IR, ippiAlphaPremulC_16u_API4R
//
//  Purpose:        Pre-multiplies pixel values of an image by constant alpha values
//                  for 4-channel images
//               For channels 1-3
//                      dst_pixel = (src_pixel * const_alpha) / VAL_MAX
//               For alpha-channel (channel 4)
//                      dst_alpha = const_alpha
//  Parameters:
//     pSrc         Pointer to the source image for pixel-order data,
//                  array of pointers to separate source color planes for planar data
//     srcStep      Step through the source image
//     pDst         Pointer to the destination image for pixel-order data,
//                  array of pointers to separate destination color planes for planar data
//     dstStep      Step through the destination image
//     pSrcDst      Pointer to the source/destination image, or array of pointers
//                  to separate source/destination color planes for in-place functions
//     srcDstStep   Step through the source/destination image for in-place functions
//     alpha        The constant alpha value
//     roiSize      Size of the source and destination ROI
//  Returns:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pSrc == NULL, or pDst == NULL, or pSrcDst == NULL
//     ippStsSizeErr          The roiSize has a field with negative or zero value
//
//  Notes:          Value becomes 0 <= alpha <= VAL_MAX
*/

IPPAPI (IppStatus, ippiAlphaPremulC_8u_AC4R,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u alpha,
                     Ipp8u* pDst, int dstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremulC_16u_AC4R,
                   ( const Ipp16u* pSrc, int srcStep,
                     Ipp16u alpha,
                     Ipp16u* pDst, int dstStep,
                     IppiSize roiSize ))

IPPAPI (IppStatus, ippiAlphaPremulC_8u_AC4IR,
                   ( Ipp8u alpha,

                     Ipp8u* pSrcDst, int srcDstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremulC_16u_AC4IR,
                   ( Ipp16u alpha,
                     Ipp16u* pSrcDst, int srcDstStep,
                     IppiSize roiSize ))

IPPAPI (IppStatus, ippiAlphaPremulC_8u_AP4R,
                   ( const Ipp8u* const pSrc[4], int srcStep,
                     Ipp8u alpha,
                     Ipp8u* const pDst[4], int dstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremulC_16u_AP4R,
                   ( const Ipp16u* const pSrc[4], int srcStep,
                     Ipp16u alpha,
                     Ipp16u* const pDst[4], int dstStep,
                     IppiSize roiSize ))

IPPAPI (IppStatus, ippiAlphaPremulC_8u_AP4IR,
                   ( Ipp8u alpha,
                     Ipp8u* const pSrcDst[4], int srcDstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremulC_16u_AP4IR,
                   ( Ipp16u alpha,
                     Ipp16u* const pSrcDst[4], int srcDstStep,
                     IppiSize roiSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiAlphaPremulC_8u_C4R,  ippiAlphaPremulC_16u_C4R
//                  ippiAlphaPremulC_8u_C4IR, ippiAlphaPremulC_16u_C4IR
//
//  Purpose:        Pre-multiplies pixel values of an image by constant alpha values
//                  for 4-channel images:
//                      dst_pixel = (src_pixel * const_alpha) / VAL_MAX
//  Parameters:
//     pSrc         Pointer to the source image
//     srcStep      Step through the source image
//     pDst         Pointer to the destination image
//     dstStep      Step through the destination image
//     pSrcDst      Pointer to the source/destination image for in-place functions
//     srcDstStep   Step through the source/destination image for in-place functions
//     alpha        The constant alpha value
//     roiSize      Size of the source and destination ROI
//  Returns:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pSrc == NULL, or pDst == NULL, or pSrcDst == NULL
//     ippStsSizeErr          The roiSize has a field with negative or zero value
//
//  Notes:          Value becomes 0 <= alpha <= VAL_MAX
*/

IPPAPI (IppStatus, ippiAlphaPremulC_8u_C4R,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u alpha,
                     Ipp8u* pDst, int dstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremulC_16u_C4R,
                   ( const Ipp16u* pSrc, int srcStep,
                     Ipp16u alpha,
                     Ipp16u* pDst, int dstStep,
                     IppiSize roiSize ))

IPPAPI (IppStatus, ippiAlphaPremulC_8u_C4IR,
                   ( Ipp8u alpha,
                     Ipp8u* pSrcDst, int srcDstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremulC_16u_C4IR,
                   ( Ipp16u alpha,
                     Ipp16u* pSrcDst, int srcDstStep,
                     IppiSize roiSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiAlphaPremulC_8u_C3R,  ippiAlphaPremulC_16u_C3R
//                  ippiAlphaPremulC_8u_C3IR, ippiAlphaPremulC_16u_C3IR
//  Purpose:        Pre-multiplies pixel values of an image by constant alpha values
//                  for 3-channel images:
//                      dst_pixel = (src_pixel * const_alpha) / VAL_MAX
//  Parameters:
//     pSrc         Pointer to the source image
//     srcStep      Step through the source image
//     pDst         Pointer to the destination image
//     dstStep      Step through the destination image
//     pSrcDst      Pointer to the source/destination image for in-place functions
//     srcDstStep   Step through the source/destination image for in-place functions
//     alpha        The constant alpha value
//     roiSize      Size of the source and destination ROI
//  Returns:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pSrc == NULL, or pDst == NULL, or pSrcDst == NULL
//     ippStsSizeErr          The roiSize has a field with negative or zero value
//
//  Notes:          Value becomes 0 <= alpha <= VAL_MAX
*/

IPPAPI (IppStatus, ippiAlphaPremulC_8u_C3R,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u alpha,
                     Ipp8u* pDst, int dstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremulC_16u_C3R,
                   ( const Ipp16u* pSrc, int srcStep,
                     Ipp16u alpha,
                     Ipp16u* pDst, int dstStep,
                     IppiSize roiSize ))

IPPAPI (IppStatus, ippiAlphaPremulC_8u_C3IR,
                   ( Ipp8u alpha,
                     Ipp8u* pSrcDst, int srcDstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremulC_16u_C3IR,
                   ( Ipp16u alpha,
                     Ipp16u* pSrcDst, int srcDstStep,
                     IppiSize roiSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiAlphaPremulC_8u_C1R,  ippiAlphaPremulC_16u_C1R
//                  ippiAlphaPremulC_8u_C1IR, ippiAlphaPremulC_16u_C1IR
//  Purpose:        Pre-multiplies pixel values of an image by constant alpha values
//                  for 1-channel images:
//                      dst_pixel = (src_pixel * const_alpha) / VAL_MAX
//  Parameters:
//     pSrc         Pointer to the source image
//     srcStep      Step through the source image
//     pDst         Pointer to the destination image
//     dstStep      Step through the destination image
//     pSrcDst      Pointer to the source/destination image for in-place functions
//     srcDstStep   Step through the source/destination image for in-place functions
//     alpha        The constant alpha value
//     roiSize      Size of the source and destination ROI
//  Returns:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pSrc == NULL, or pDst == NULL, or pSrcDst == NULL
//     ippStsSizeErr          The roiSize has a field with negative or zero value
//
//  Notes:          Value becomes 0 <= alpha <= VAL_MAX
*/


IPPAPI (IppStatus, ippiAlphaPremulC_8u_C1R,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u alpha,
                     Ipp8u* pDst, int dstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremulC_16u_C1R,
                   ( const Ipp16u* pSrc, int srcStep,
                     Ipp16u alpha,
                     Ipp16u* pDst, int dstStep,
                     IppiSize roiSize ))

IPPAPI (IppStatus, ippiAlphaPremulC_8u_C1IR,
                   ( Ipp8u alpha,
                     Ipp8u* pSrcDst, int srcDstStep,
                     IppiSize roiSize ))
IPPAPI (IppStatus, ippiAlphaPremulC_16u_C1IR,
                   ( Ipp16u alpha,
                     Ipp16u* pSrcDst, int srcDstStep,
                     IppiSize roiSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiAlphaComp_8u_AC4R, ippiAlphaComp_16u_AC4R
//                  ippiAlphaComp_8s_AC4R, ippiAlphaComp_16s_AC4R
//                  ippiAlphaComp_32s_AC4R,ippiAlphaComp_32u_AC4R
//                  ippiAlphaComp_8u_AP4R, ippiAlphaComp_16u_AP4R
//
//  Purpose:        Combines two 4-channel images using alpha values of both images
//
//  Parameters:
//     pSrc1        Pointer to the first source image for pixel-order data,
//                  array of pointers to separate source color planes for planar data
//     src1Step     Step through the first source image
//     pSrc2        Pointer to the second source image for pixel-order data,
//                  array of pointers to separate source color planes for planar data
//     src2Step     Step through the second source image
//     pDst         Pointer to the destination image for pixel-order data,
//                  array of pointers to separate destination color planes for planar data
//     dstStep      Step through the destination image
//     roiSize      Size of the source and destination ROI
//     alphaType    The type of composition to perform
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pSrc1== NULL, or pSrc2== NULL, or pDst == NULL
//     ippStsSizeErr          The roiSize has a field with negative or zero value
//     ippStsAlphaTypeErr     The alphaType is incorrect
//     Note:                  Result is wrong, if Alpha < 0 for signed types
*/

IPPAPI (IppStatus, ippiAlphaComp_8u_AC4R,
                   ( const Ipp8u* pSrc1, int src1Step,

                     const Ipp8u* pSrc2, int src2Step,
                     Ipp8u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))
IPPAPI (IppStatus, ippiAlphaComp_16u_AC4R,
                   ( const Ipp16u* pSrc1, int src1Step,
                     const Ipp16u* pSrc2, int src2Step,
                     Ipp16u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))


IPPAPI (IppStatus, ippiAlphaComp_8s_AC4R,
                   ( const Ipp8s* pSrc1, int src1Step,
                     const Ipp8s* pSrc2, int src2Step,
                     Ipp8s* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaComp_16s_AC4R,
                   ( const Ipp16s* pSrc1, int src1Step,
                     const Ipp16s* pSrc2, int src2Step,
                     Ipp16s* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaComp_32s_AC4R,
                   ( const Ipp32s* pSrc1, int src1Step,
                     const Ipp32s* pSrc2, int src2Step,
                     Ipp32s* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaComp_32u_AC4R,
                   ( const Ipp32u* pSrc1, int src1Step,
                     const Ipp32u* pSrc2, int src2Step,
                     Ipp32u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaComp_32f_AC4R,
                   ( const Ipp32f* pSrc1, int src1Step,
                     const Ipp32f* pSrc2, int src2Step,
                     Ipp32f* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaComp_8u_AP4R,
                   ( const Ipp8u* const pSrc1[4], int src1Step,
                     const Ipp8u* const pSrc2[4], int src2Step,
                     Ipp8u* const pDst[4], int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaComp_16u_AP4R,
                   ( const Ipp16u* const pSrc1[4], int src1Step,
                     const Ipp16u* const pSrc2[4], int src2Step,
                     Ipp16u* const pDst[4], int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiAlphaComp_8u_AC1R, ippiAlphaComp_16u_AC1R
//                  ippiAlphaComp_8s_AC1R, ippiAlphaComp_16s_AC1R
//                  ippiAlphaComp_32s_AC1R, ippiAlphaComp_32u_AC1R
//  Purpose:        Combines two 1-channel images using alpha values of both images
//
//  Parameters:
//     pSrc1        Pointer to the first source image
//     src1Step     Step through the first source image
//     pSrc2        Pointer to the second source image
//     src2Step     Step through the second source image
//     pDst         Pointer to the destination image
//     dstStep      Step through the destination image
//     roiSize      Size of the source and destination ROI
//     alphaType    The type of composition to perform
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pSrc1== NULL, or pSrc2== NULL, or pDst == NULL
//     ippStsSizeErr          The roiSize has a field with negative or zero value
//     ippStsAlphaTypeErr     The alphaType is incorrect
//     Note:                  Result is wrong, if Alpha < 0 for signed types
*/

IPPAPI (IppStatus, ippiAlphaComp_8u_AC1R,
                   ( const Ipp8u* pSrc1, int src1Step,
                     const Ipp8u* pSrc2, int src2Step,
                     Ipp8u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))
IPPAPI (IppStatus, ippiAlphaComp_16u_AC1R,
                   ( const Ipp16u* pSrc1, int src1Step,
                     const Ipp16u* pSrc2, int src2Step,
                     Ipp16u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaComp_8s_AC1R,
                   ( const Ipp8s* pSrc1, int src1Step,
                     const Ipp8s* pSrc2, int src2Step,
                     Ipp8s* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))
IPPAPI (IppStatus, ippiAlphaComp_16s_AC1R,
                   ( const Ipp16s* pSrc1, int src1Step,
                     const Ipp16s* pSrc2, int src2Step,
                     Ipp16s* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaComp_32s_AC1R,
                   ( const Ipp32s* pSrc1, int src1Step,
                     const Ipp32s* pSrc2, int src2Step,
                     Ipp32s* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaComp_32u_AC1R,
                   ( const Ipp32u* pSrc1, int src1Step,
                     const Ipp32u* pSrc2, int src2Step,
                     Ipp32u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaComp_32f_AC1R,
                   ( const Ipp32f* pSrc1, int src1Step,
                     const Ipp32f* pSrc2, int src2Step,
                     Ipp32f* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiAlphaCompC_8u_AC4R, ippiAlphaCompC_16u_AC4R
//                  ippiAlphaCompC_8u_AP4R, ippiAlphaCompC_16u_AP4R
//
//  Purpose:        Combines two 4-channel images using constant alpha values
//
//  Parameters:
//     pSrc1        Pointer to the first source image for pixel-order data,
//                  array of pointers to separate source color planes for planar data
//     src1Step     Step through the first source image
//     pSrc2        Pointer to the second source image for pixel-order data,
//                  array of pointers to separate source color planes for planar data
//     src2Step     Step through the second source image
//     pDst         Pointer to the destination image for pixel-order data,
//                  array of pointers to separate destination color planes for planar data
//     dstStep      Step through the destination image
//     roiSize      Size of the source and destination ROI
//     alpha1       The constant alpha value for the first source image
//     alpha2       The constant alpha value for the second source image
//     alphaType    The type of composition to perform
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pSrc1== NULL, or pSrc2== NULL, or pDst == NULL
//     ippStsSizeErr          The roiSize has a field with negative or zero value
//     ippStsAlphaTypeErr     The alphaType is incorrect
//
//  Notes:          Alpha-channel values (channel 4) remain without modifications
//                  Value becomes 0 <= alphaA <= VAL_MAX
//                                0 <= alphaB <= VAL_MAX
*/

IPPAPI (IppStatus, ippiAlphaCompC_8u_AC4R,
                   ( const Ipp8u* pSrc1, int src1Step,
                     Ipp8u alpha1,
                     const Ipp8u* pSrc2, int src2Step,
                     Ipp8u alpha2,
                     Ipp8u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))
IPPAPI (IppStatus, ippiAlphaCompC_16u_AC4R,
                   ( const Ipp16u* pSrc1, int src1Step,
                     Ipp16u alpha1,
                     const Ipp16u* pSrc2, int src2Step,
                     Ipp16u alpha2,
                     Ipp16u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaCompC_8u_AP4R,
                   ( const Ipp8u* const pSrc1[4], int src1Step,
                     Ipp8u alpha1,
                     const Ipp8u* const pSrc2[4], int src2Step,
                     Ipp8u alpha2,
                     Ipp8u* const pDst[4], int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))
IPPAPI (IppStatus, ippiAlphaCompC_16u_AP4R,
                   ( const Ipp16u* const pSrc1[4], int src1Step,
                     Ipp16u alpha1,
                     const Ipp16u* const pSrc2[4], int src2Step,
                     Ipp16u alpha2,
                     Ipp16u* const pDst[4], int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiAlphaCompC_8u_C4R, ippiAlphaCompC_16u_C4R
//
//  Purpose:        Combines two 4-channel images using constant alpha values
//
//  Parameters:
//     pSrc1        Pointer to the first source image
//     src1Step     Step through the first source image
//     pSrc2        Pointer to the second source image
//     src2Step     Step through the second source image
//     pDst         Pointer to the destination image
//     dstStep      Step through the destination image
//     roiSize      Size of the source and destination ROI
//     alpha1       The constant alpha value for the first source image
//     alpha2       The constant alpha value for the second source image
//     alphaType    The type of composition to perform
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pSrc1== NULL, or pSrc2== NULL, or pDst == NULL
//     ippStsSizeErr          The roiSize has a field with negative or zero value
//     ippStsAlphaTypeErr     The alphaType is incorrect
//
//  Notes:          Value becomes 0 <= alphaA <= VAL_MAX
//                                0 <= alphaB <= VAL_MAX
*/

IPPAPI (IppStatus, ippiAlphaCompC_8u_C4R,
                   ( const Ipp8u* pSrc1, int src1Step,
                     Ipp8u alpha1,
                     const Ipp8u* pSrc2, int src2Step,

                     Ipp8u alpha2,
                     Ipp8u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))
IPPAPI (IppStatus, ippiAlphaCompC_16u_C4R,
                   ( const Ipp16u* pSrc1, int src1Step,
                     Ipp16u alpha1,
                     const Ipp16u* pSrc2, int src2Step,
                     Ipp16u alpha2,
                     Ipp16u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiAlphaCompC_8u_C3R, ippiAlphaCompC_16u_C3R
//  Purpose:        Combines two 3-channel images using constant alpha values
//  Parameters:
//     pSrc1        Pointer to the first source image
//     src1Step     Step through the first source image
//     pSrc2        Pointer to the second source image
//     src2Step     Step through the second source image
//     pDst         Pointer to the destination image
//     dstStep      Step through the destination image
//     roiSize      Size of the source and destination ROI
//     alpha1       The constant alpha value for the first source image
//     alpha2       The constant alpha value for the second source image
//     alphaType    The type of composition to perform
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pSrc1== NULL, or pSrc2== NULL, or pDst == NULL
//     ippStsSizeErr          The roiSize has a field with negative or zero value
//     ippStsAlphaTypeErr     The alphaType is incorrect
//
//  Notes:          Value becomes 0 <= alphaA <= VAL_MAX
//                                0 <= alphaB <= VAL_MAX
*/

IPPAPI (IppStatus, ippiAlphaCompC_8u_C3R,
                   ( const Ipp8u* pSrc1, int src1Step,
                     Ipp8u alpha1,
                     const Ipp8u* pSrc2, int src2Step,
                     Ipp8u alpha2,
                     Ipp8u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))
IPPAPI (IppStatus, ippiAlphaCompC_16u_C3R,
                   ( const Ipp16u* pSrc1, int src1Step,
                     Ipp16u alpha1,
                     const Ipp16u* pSrc2, int src2Step,
                     Ipp16u alpha2,
                     Ipp16u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiAlphaCompC_8u_C1R, ippiAlphaCompC_16u_C1R
//                  ippiAlphaCompC_8s_C1R, ippiAlphaCompC_16s_C1R
//                  ippiAlphaCompC_32s_C1R, ippiAlphaCompC_32u_C1R
//  Purpose:        Combines two 1-channel images using constant alpha values
//  Parameters:
//     pSrc1        Pointer to the first source image
//     src1Step     Step through the first source image
//     pSrc2        Pointer to the second source image
//     src2Step     Step through the second source image
//     pDst         Pointer to the destination image
//     dstStep      Step through the destination image
//     roiSize      Size of the source and destination ROI
//     alpha1       The constant alpha value for the first source image
//     alpha2       The constant alpha value for the second source image
//     alphaType    The type of composition to perform
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pSrc1== NULL, or pSrc2== NULL, or pDst == NULL
//     ippStsSizeErr          The roiSize has a field with negative or zero value
//     ippStsAlphaTypeErr     The alphaType is incorrect
//
//  Notes:          Value becomes 0 <= alphaA <= VAL_MAX
//                                0 <= alphaB <= VAL_MAX
*/

IPPAPI (IppStatus, ippiAlphaCompC_8u_C1R,
                   ( const Ipp8u* pSrc1, int src1Step,
                     Ipp8u alpha1,
                     const Ipp8u* pSrc2, int src2Step,
                     Ipp8u alpha2,
                     Ipp8u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))
IPPAPI (IppStatus, ippiAlphaCompC_16u_C1R,
                   ( const Ipp16u* pSrc1, int src1Step,
                     Ipp16u alpha1,
                     const Ipp16u* pSrc2, int src2Step,
                     Ipp16u alpha2,
                     Ipp16u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaCompC_8s_C1R,
                   ( const Ipp8s* pSrc1, int src1Step,
                     Ipp8s alpha1,
                     const Ipp8s* pSrc2, int src2Step,
                     Ipp8s alpha2,
                     Ipp8s* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))
IPPAPI (IppStatus, ippiAlphaCompC_16s_C1R,
                   ( const Ipp16s* pSrc1, int src1Step,
                     Ipp16s alpha1,
                     const Ipp16s* pSrc2, int src2Step,
                     Ipp16s alpha2,
                     Ipp16s* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))


IPPAPI (IppStatus, ippiAlphaCompC_32s_C1R,
                   ( const Ipp32s* pSrc1, int src1Step,
                     Ipp32s alpha1,
                     const Ipp32s* pSrc2, int src2Step,
                     Ipp32s alpha2,
                     Ipp32s* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaCompC_32u_C1R,
                   ( const Ipp32u* pSrc1, int src1Step,
                     Ipp32u alpha1,
                     const Ipp32u* pSrc2, int src2Step,
                     Ipp32u alpha2,
                     Ipp32u* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))

IPPAPI (IppStatus, ippiAlphaCompC_32f_C1R,
                   ( const Ipp32f* pSrc1, int src1Step,
                     Ipp32f alpha1,
                     const Ipp32f* pSrc2, int src2Step,
                     Ipp32f alpha2,
                     Ipp32f* pDst, int dstStep,
                     IppiSize roiSize,
                     IppiAlphaType alphaType ))




/* /////////////////////////////////////////////////////////////////////////////
//                  Linear Transform Operations
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//                  Definitions for FFT Functions
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )

struct FFT2DSpec_C_32fc;
typedef struct FFT2DSpec_C_32fc IppiFFTSpec_C_32fc;
struct FFT2DSpec_R_32f;
typedef struct FFT2DSpec_R_32f IppiFFTSpec_R_32f;

struct FFT2DSpec_R_32s;
typedef struct FFT2DSpec_R_32s IppiFFTSpec_R_32s;

#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
//                  FFT Context Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFFTInitAlloc
//  Purpose:    Creates and initializes the FFT context structure
//  Parameters:
//     orderX     Base-2 logarithm of the number of samples in FFT (width)
//     orderY     Base-2 logarithm of the number of samples in FFT (height)
//     flag       Flag to choose the results normalization factors
//     hint       Option to select the algorithmic implementation of the transform
//                function
//     pFFTSpec   Pointer to the pointer to the FFT context structure
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pFFTSpec == NULL
//     ippStsFftOrderErr      FFT order value is illegal
//     ippStsFFTFlagErr       flag has an illegal value
//     ippStsMemAllocErr      Memory allocation fails
*/

IPPAPI (IppStatus, ippiFFTInitAlloc_C_32fc,
                   ( IppiFFTSpec_C_32fc** pFFTSpec,
                     int orderX, int orderY, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippiFFTInitAlloc_R_32f,
                   ( IppiFFTSpec_R_32f** pFFTSpec,
                     int orderX, int orderY, int flag, IppHintAlgorithm hint ))

IPPAPI (IppStatus, ippiFFTInitAlloc_R_32s,
                   ( IppiFFTSpec_R_32s** pFFTSpec,
                     int orderX, int orderY, int flag, IppHintAlgorithm hint ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFFTFree
//  Purpose:    Deallocates memory used by the FFT context structure
//  Parameters:
//     pFFTSpec  Pointer to the FFT context structure
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pFFTSpec == NULL
//     ippStsContextMatchErr  Invalid context structure
*/

IPPAPI (IppStatus, ippiFFTFree_C_32fc, ( IppiFFTSpec_C_32fc* pFFTSpec ))
IPPAPI (IppStatus, ippiFFTFree_R_32f, ( IppiFFTSpec_R_32f*  pFFTSpec ))

IPPAPI (IppStatus, ippiFFTFree_R_32s, ( IppiFFTSpec_R_32s*  pFFTSpec ))


/* /////////////////////////////////////////////////////////////////////////////
//                  FFT Buffer Size
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFFTGetBufSize
//  Purpose:    Computes the size of an external FFT work buffer (in bytes)
//  Parameters:
//     pFFTSpec  Pointer to the FFT context structure
//     pSize      Pointer to the size of the external buffer
//  Returns:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pFFTSpec == NULL or pSize == NULL
//     ippStsContextMatchErr  bad context identifier
*/

IPPAPI (IppStatus, ippiFFTGetBufSize_C_32fc,
                   ( const IppiFFTSpec_C_32fc* pFFTSpec, int* pSize ))
IPPAPI (IppStatus, ippiFFTGetBufSize_R_32f,
                   ( const IppiFFTSpec_R_32f* pFFTSpec, int* pSize ))

IPPAPI (IppStatus, ippiFFTGetBufSize_R_32s,
                   ( const IppiFFTSpec_R_32s* pFFTSpec, int* pSize ))

/* /////////////////////////////////////////////////////////////////////////////
//                  FFT Transforms
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFFTFwd, ippiFFTInv
//  Purpose:    Performs forward or inverse FFT of an image
//  Parameters:
//     pFFTSpec   Pointer to the FFT context structure
//     pSrc       Pointer to the source image
//     srcStep    Step through the source image
//     pDst       Pointer to the destination image
//     dstStep    Step through the destination image
//     pSrcDst    Pointer to the source/destination image (in-place)
//     srcDstStep Step through the source/destination image (in-place)
//     pBuffer    Pointer to the external work buffer
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pFFTSpec == NULL, or
//                            pSrc == NULL, or pDst == NULL
//     ippStsStepErr          srcStep or dstStep value is zero or negative
//     ippStsContextMatchErr  Invalid context structure
//     ippStsMemAllocErr      Memory allocation error
*/

IPPAPI (IppStatus, ippiFFTFwd_CToC_32fc_C1R,
                   ( const Ipp32fc* pSrc, int srcStep,
                     Ipp32fc* pDst, int dstStep,
                     const IppiFFTSpec_C_32fc* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTInv_CToC_32fc_C1R,
                   ( const Ipp32fc* pSrc, int srcStep,
                     Ipp32fc* pDst, int dstStep,
                     const IppiFFTSpec_C_32fc* pFFTSpec,
                     Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippiFFTFwd_RToPack_32f_C1R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTFwd_RToPack_32f_C3R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTFwd_RToPack_32f_C4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTFwd_RToPack_32f_AC4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippiFFTInv_PackToR_32f_C1R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTInv_PackToR_32f_C3R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTInv_PackToR_32f_C4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTInv_PackToR_32f_AC4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippiFFTFwd_RToPack_8u32s_C1RSfs,
                   ( const Ipp8u *pSrc, int srcStep,
                     Ipp32s *pDst, int dstStep,
                     const IppiFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))
IPPAPI (IppStatus, ippiFFTFwd_RToPack_8u32s_C3RSfs,
                   ( const Ipp8u *pSrc, int srcStep,
                     Ipp32s *pDst, int dstStep,
                     const IppiFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))
IPPAPI (IppStatus, ippiFFTFwd_RToPack_8u32s_C4RSfs,
                   ( const Ipp8u *pSrc, int srcStep,
                     Ipp32s *pDst, int dstStep,
                     const IppiFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))
IPPAPI (IppStatus, ippiFFTFwd_RToPack_8u32s_AC4RSfs,
                   ( const Ipp8u *pSrc, int srcStep,
                     Ipp32s *pDst, int dstStep,
                     const IppiFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))

IPPAPI (IppStatus, ippiFFTInv_PackToR_32s8u_C1RSfs,
                   ( const Ipp32s *pSrc, int srcStep,
                     Ipp8u *pDst, int dstStep,
                     const IppiFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))
IPPAPI (IppStatus, ippiFFTInv_PackToR_32s8u_C3RSfs,
                   ( const Ipp32s *pSrc, int srcStep,
                     Ipp8u *pDst, int dstStep,
                     const IppiFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))
IPPAPI (IppStatus, ippiFFTInv_PackToR_32s8u_C4RSfs,
                   ( const Ipp32s *pSrc, int srcStep,
                     Ipp8u *pDst, int dstStep,
                     const IppiFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))
IPPAPI (IppStatus, ippiFFTInv_PackToR_32s8u_AC4RSfs,
                   ( const Ipp32s *pSrc, int srcStep,
                     Ipp8u *pDst, int dstStep,
                     const IppiFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))

IPPAPI (IppStatus, ippiFFTFwd_CToC_32fc_C1IR,
                   ( Ipp32fc* pSrcDst, int srcDstStep,
                     const IppiFFTSpec_C_32fc* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTInv_CToC_32fc_C1IR,
                   ( Ipp32fc* pSrcDst, int srcDstStep,
                     const IppiFFTSpec_C_32fc* pFFTSpec,
                     Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippiFFTFwd_RToPack_32f_C1IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTFwd_RToPack_32f_C3IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTFwd_RToPack_32f_C4IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTFwd_RToPack_32f_AC4IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippiFFTInv_PackToR_32f_C1IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTInv_PackToR_32f_C3IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTInv_PackToR_32f_C4IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiFFTInv_PackToR_32f_AC4IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiFFTSpec_R_32f* pFFTSpec,
                     Ipp8u* pBuffer ))

/* /////////////////////////////////////////////////////////////////////////////
//                  Definitions for DFT Functions
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )

struct DFT2DSpec_C_32fc;
typedef struct DFT2DSpec_C_32fc IppiDFTSpec_C_32fc;
struct DFT2DSpec_R_32f;
typedef struct DFT2DSpec_R_32f IppiDFTSpec_R_32f;

struct DFT2DSpec_R_32s;
typedef struct DFT2DSpec_R_32s IppiDFTSpec_R_32s;

#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
//                  DFT Context Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDFTInitAlloc
//  Purpose:      Creates and initializes the DFT context structure
//  Parameters:
//     roiSize    Size of the ROI
//     flag       Flag to choose the results normalization factors
//     hint       Option to select the algorithmic implementation of the transform
//                function
//     pDFTSpec   Pointer to the pointer to the DFT context structure
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pDFTSpec == NULL
//     ippStsSizeErr          roiSize has a field with zero or negative value
//     ippStsFFTFlagErr       Illegal value of the flag
//     ippStsMemAllocErr      Memory allocation error
*/

IPPAPI (IppStatus, ippiDFTInitAlloc_C_32fc,
                   ( IppiDFTSpec_C_32fc** pDFTSpec,
                     IppiSize roiSize, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippiDFTInitAlloc_R_32f,
                   ( IppiDFTSpec_R_32f** pDFTSpec,
                     IppiSize roiSize, int flag, IppHintAlgorithm hint ))

IPPAPI (IppStatus, ippiDFTInitAlloc_R_32s,
                   ( IppiDFTSpec_R_32s** pDFTSpec,
                     IppiSize roiSize, int flag, IppHintAlgorithm hint ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDFTFree
//  Purpose:    Deallocates memory used by the DFT context structure
//  Parameters:
//     pDFTSpec       Pointer to the DFT context structure
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pDFTSpec == NULL
//     ippStsContextMatchErr  Invalid context structure
*/

IPPAPI (IppStatus, ippiDFTFree_C_32fc, ( IppiDFTSpec_C_32fc* pDFTSpec ))
IPPAPI (IppStatus, ippiDFTFree_R_32f, ( IppiDFTSpec_R_32f*  pDFTSpec ))
IPPAPI (IppStatus, ippiDFTFree_R_32s, ( IppiDFTSpec_R_32s*  pFFTSpec ))


/* /////////////////////////////////////////////////////////////////////////////
//                  DFT Buffer Size
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDFTGetBufSize
//  Purpose:    Computes the size of the external DFT work buffer (in bytes)
//  Parameters:
//     pDFTSpec   Pointer to the DFT context structure
//     pSize      Pointer to the size of the buffer
//  Returns:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pDFTSpec == NULL or pSize == NULL
//     ippStsContextMatchErr  Invalid context structure
*/

IPPAPI (IppStatus, ippiDFTGetBufSize_C_32fc,
                   ( const IppiDFTSpec_C_32fc* pDFTSpec, int* pSize ))
IPPAPI (IppStatus, ippiDFTGetBufSize_R_32f,
                   ( const IppiDFTSpec_R_32f* pDFTSpec, int* pSize ))

IPPAPI (IppStatus, ippiDFTGetBufSize_R_32s,
                   ( const IppiDFTSpec_R_32s* pDFTSpec, int* pSize ))


/* /////////////////////////////////////////////////////////////////////////////
//                  DFT Transforms
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDFTFwd, ippiDFTInv
//  Purpose:    Performs forward or inverse DFT of an image
//  Parameters:
//     pDFTSpec    Pointer to the DFT context structure
//     pSrc        Pointer to source image
//     srcStep     Step through the source image
//     pDst        Pointer to the destination image
//     dstStep     Step through the destination image
//     pSrcDst     Pointer to the source/destination image (in-place)
//     srcDstStep  Step through the source/destination image (in-place)
//     pBuffer     Pointer to the external work buffer
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pDFTSpec == NULL, or
//                            pSrc == NULL, or pDst == NULL
//     ippStsStepErr          srcStep or dstStep value is zero or negative
//     ippStsContextMatchErr  Invalid context structure
//     ippStsMemAllocErr      Memory allocation error
*/

IPPAPI (IppStatus, ippiDFTFwd_CToC_32fc_C1R,
                   ( const Ipp32fc* pSrc, int srcStep,
                     Ipp32fc* pDst, int dstStep,
                     const IppiDFTSpec_C_32fc* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTInv_CToC_32fc_C1R,
                   ( const Ipp32fc* pSrc, int srcStep,
                     Ipp32fc* pDst, int dstStep,
                     const IppiDFTSpec_C_32fc* pDFTSpec,
                     Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippiDFTFwd_RToPack_32f_C1R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTFwd_RToPack_32f_C3R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTFwd_RToPack_32f_C4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTFwd_RToPack_32f_AC4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippiDFTInv_PackToR_32f_C1R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTInv_PackToR_32f_C3R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTInv_PackToR_32f_C4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTInv_PackToR_32f_AC4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippiDFTFwd_RToPack_8u32s_C1RSfs,
                   ( const Ipp8u *pSrc, int srcStep,
                     Ipp32s *pDst, int dstStep,
                     const IppiDFTSpec_R_32s* pDFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))
IPPAPI (IppStatus, ippiDFTFwd_RToPack_8u32s_C3RSfs,
                   ( const Ipp8u *pSrc, int srcStep,
                     Ipp32s *pDst, int dstStep,
                     const IppiDFTSpec_R_32s* pDFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))
IPPAPI (IppStatus, ippiDFTFwd_RToPack_8u32s_C4RSfs,
                   ( const Ipp8u *pSrc, int srcStep,
                     Ipp32s *pDst, int dstStep,
                     const IppiDFTSpec_R_32s* pDFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))
IPPAPI (IppStatus, ippiDFTFwd_RToPack_8u32s_AC4RSfs,
                   ( const Ipp8u *pSrc, int srcStep,
                     Ipp32s *pDst, int dstStep,
                     const IppiDFTSpec_R_32s* pDFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))

IPPAPI (IppStatus, ippiDFTInv_PackToR_32s8u_C1RSfs,
                   ( const Ipp32s *pSrc, int srcStep,
                     Ipp8u *pDst, int dstStep,
                     const IppiDFTSpec_R_32s* pDFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))
IPPAPI (IppStatus, ippiDFTInv_PackToR_32s8u_C3RSfs,
                   ( const Ipp32s *pSrc, int srcStep,
                     Ipp8u *pDst, int dstStep,
                     const IppiDFTSpec_R_32s* pDFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))
IPPAPI (IppStatus, ippiDFTInv_PackToR_32s8u_C4RSfs,
                   ( const Ipp32s *pSrc, int srcStep,
                     Ipp8u *pDst, int dstStep,
                     const IppiDFTSpec_R_32s* pDFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))
IPPAPI (IppStatus, ippiDFTInv_PackToR_32s8u_AC4RSfs,
                   ( const Ipp32s *pSrc, int srcStep,
                     Ipp8u *pDst, int dstStep,
                     const IppiDFTSpec_R_32s* pDFTSpec,
                     int scaleFactor, Ipp8u *pBuffer ))

IPPAPI (IppStatus, ippiDFTFwd_CToC_32fc_C1IR,
                   ( Ipp32fc* pSrcDst, int srcDstStep,
                     const IppiDFTSpec_C_32fc* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTInv_CToC_32fc_C1IR,
                   ( Ipp32fc* pSrcDst, int srcDstStep,
                     const IppiDFTSpec_C_32fc* pDFTSpec,
                     Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippiDFTFwd_RToPack_32f_C1IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTFwd_RToPack_32f_C3IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTFwd_RToPack_32f_C4IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTFwd_RToPack_32f_AC4IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippiDFTInv_PackToR_32f_C1IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTInv_PackToR_32f_C3IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTInv_PackToR_32f_C4IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDFTInv_PackToR_32f_AC4IR,
                   ( Ipp32f* pSrcDst, int srcDstStep,
                     const IppiDFTSpec_R_32f* pDFTSpec,
                     Ipp8u* pBuffer ))

/* /////////////////////////////////////////////////////////////////////////////
//                  Definitions for DCT Functions
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )

struct DCT2DFwdSpec_32f;
typedef struct DCT2DFwdSpec_32f IppiDCTFwdSpec_32f;
struct DCT2DInvSpec_32f;
typedef struct DCT2DInvSpec_32f IppiDCTInvSpec_32f;

#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
//                  DCT Context Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDCTFwdInitAlloc, ippiDCTInvInitAlloc
//  Purpose:    Creates and initializes the forward/inverse DCT context structure
//  Parameters:
//     roiSize    Size of the ROI
//     hint       Option to select the algorithmic implementation of the transform
//                function
//     pDCTSpec   Pointer to the pointer to the DCT context structure
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pDCTSpec == NULL
//     ippStsSizeErr          roiSize has a field with zero or negative value
//     ippStsMemAllocErr      Memory allocation error
*/

IPPAPI (IppStatus, ippiDCTFwdInitAlloc_32f,
                   ( IppiDCTFwdSpec_32f** pDCTSpec,
                     IppiSize roiSize, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippiDCTInvInitAlloc_32f,
                   ( IppiDCTInvSpec_32f** pDCTSpec,
                     IppiSize roiSize, IppHintAlgorithm hint ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDCTFwdFree, ippiDCTInvFree
//  Purpose:    Frees memory used by the forward/inverse DCT context structure
//  Parameters:
//     pDCTSpec  Pointer to the forward/inverse DCT context structure
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pDCTSpec == NULL
//     ippStsContextMatchErr  Invalid context structure
*/

IPPAPI (IppStatus, ippiDCTFwdFree_32f, ( IppiDCTFwdSpec_32f*  pDCTSpec ))
IPPAPI (IppStatus, ippiDCTInvFree_32f, ( IppiDCTInvSpec_32f*  pDCTSpec ))


/* /////////////////////////////////////////////////////////////////////////////
//                  DCT Buffer Size
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDCTFwdGetBufSize, ippiDCTInvGetBufSize
//  Purpose:    Computes the size of the external forward/inverse DCT work buffer
//              (in bytes)
//  Parameters:
//     pDCTSpec   Pointer to the external forward/inverse DCT context structure
//     pSize      Pointer to the size of the buffer
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pDCTSpec == NULL, or pSize == NULL
//     ippStsContextMatchErr  Invalid context structure
*/

IPPAPI (IppStatus, ippiDCTFwdGetBufSize_32f,
                   ( const IppiDCTFwdSpec_32f* pDCTSpec, int* pSize ))
IPPAPI (IppStatus, ippiDCTInvGetBufSize_32f,
                   ( const IppiDCTInvSpec_32f* pDCTSpec, int* pSize ))


/* /////////////////////////////////////////////////////////////////////////////
//                  DCT Transforms
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDCTFwd, ippiDCTInv
//  Purpose:    Performs forward or inverse DCT of an image
//  Parameters:
//     pDCTSpec   Pointer to the DCT context structure
//     pSrc       Pointer to the source image
//     srcStep    Step through the source image
//     pDst       Pointer to the destination image
//     dstStep    Step through the destination image
//     pBuffer    Pointer to the work buffer
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       pDCTSpec == NULL, or
//                            pSrc == NULL, or pDst == NULL
//     ippStsStepErr          srcStep or dstStep value is zero or negative
//     ippStsContextMatchErr  Invalid context structure
//     ippStsMemAllocErr      memory allocation error
*/

IPPAPI (IppStatus, ippiDCTFwd_32f_C1R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDCTFwdSpec_32f* pDCTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDCTFwd_32f_C3R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDCTFwdSpec_32f* pDCTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDCTFwd_32f_C4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDCTFwdSpec_32f* pDCTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDCTFwd_32f_AC4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDCTFwdSpec_32f* pDCTSpec,
                     Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippiDCTInv_32f_C1R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDCTInvSpec_32f* pDCTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDCTInv_32f_C3R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDCTInvSpec_32f* pDCTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDCTInv_32f_C4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDCTInvSpec_32f* pDCTSpec,
                     Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippiDCTInv_32f_AC4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep,
                     const IppiDCTInvSpec_32f* pDCTSpec,
                     Ipp8u* pBuffer ))

/* /////////////////////////////////////////////////////////////////////////////
//                  8x8 DCT Transforms
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiDCT8x8Fwd_16s_C1, ippiDCT8x8Fwd_16s_C1I
//             ippiDCT8x8Inv_16s_C1, ippiDCT8x8Inv_16s_C1I
//             ippiDCT8x8Fwd_16s_C1R
//             ippiDCT8x8Inv_16s_C1R
//  Purpose:   Performs forward or inverse DCT in the 8x8 buffer for 16s data
//
//  Parameters:
//     pSrc       Pointer to the source buffer
//     pDst       Pointer to the destination buffer
//     pSrcDst    Pointer to the source and destination buffer (in-place operations)
//     srcStep    Step through the source image (operations with ROI)
//     dstStep    Step through the destination image (operations with ROI)
//  Returns:
//     ippStsNoErr         No errors
//     ippStsNullPtrErr    One of the pointers is NULL
//     ippStsStepErr       srcStep or dstStep value is zero or negative
//  Notes:
//     Source data for inverse DCT functions must be the result of the forward DCT
//     of data from the range [-256,255]
*/

IPPAPI (IppStatus, ippiDCT8x8Fwd_16s_C1,  ( const Ipp16s* pSrc, Ipp16s* pDst ))
IPPAPI (IppStatus, ippiDCT8x8Inv_16s_C1,  ( const Ipp16s* pSrc, Ipp16s* pDst ))

IPPAPI (IppStatus, ippiDCT8x8Fwd_16s_C1I, ( Ipp16s* pSrcDst ))
IPPAPI (IppStatus, ippiDCT8x8Inv_16s_C1I, ( Ipp16s* pSrcDst ))

IPPAPI (IppStatus, ippiDCT8x8Fwd_16s_C1R,
                           ( const Ipp16s* pSrc, int srcStep, Ipp16s* pDst ))
IPPAPI (IppStatus, ippiDCT8x8Inv_16s_C1R,
                           ( const Ipp16s* pSrc, Ipp16s* pDst, int dstStep ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiDCT8x8Inv_2x2_16s_C1, ippiDCT8x8Inv_2x2_16s_C1I
//             ippiDCT8x8Inv_4x4_16s_C1, ippiDCT8x8Inv_4x4_16s_C1I
//  Purpose:   Performs inverse DCT of nonzero elements in the top left quadrant
//             of size 2x2 or 4x4 in the 8x8 buffer
//  Parameters:
//     pSrc       Pointer to the source buffer
//     pDst       Pointer to the destination buffer
//     pSrcDst    Pointer to the source/destination buffer (in-place operations)
//  Returns:
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       one of the pointers is NULL
//  Notes:
//     Source data for these functions must be the result of the forward DCT
//     of data from the range [-256,255]
*/

IPPAPI (IppStatus, ippiDCT8x8Inv_2x2_16s_C1,  ( const Ipp16s* pSrc, Ipp16s* pDst ))
IPPAPI (IppStatus, ippiDCT8x8Inv_4x4_16s_C1,  ( const Ipp16s* pSrc, Ipp16s* pDst ))

IPPAPI (IppStatus, ippiDCT8x8Inv_2x2_16s_C1I, ( Ipp16s* pSrcDst ))
IPPAPI (IppStatus, ippiDCT8x8Inv_4x4_16s_C1I, ( Ipp16s* pSrcDst ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiDCT8x8To2x2Inv_16s_C1, ippiDCT8x8To2x2Inv_16s_C1I
//             ippiDCT8x8To4x4Inv_16s_C1, ippiDCT8x8To4x4Inv_16s_C1I
//  Purpose:   Inverse Discrete Cosine Transform 8x8 for 16s data and
//             downsampling of the result from 8x8 to 2x2 or 4x4 by averaging
//  Arguments:
//     pSrc     - pointer to start of source buffer
//     pDst     - pointer to start of destination buffer
//     pSrcDst  - pointer to start of in-place buffer
//  Returns:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       one of pointers are NULL
//  Notes:
//     Source data for these functions must be the result of the forward DCT
//     of data from the range [-256,255]
*/

IPPAPI (IppStatus, ippiDCT8x8To2x2Inv_16s_C1,  ( const Ipp16s* pSrc, Ipp16s* pDst ))
IPPAPI (IppStatus, ippiDCT8x8To4x4Inv_16s_C1,  ( const Ipp16s* pSrc, Ipp16s* pDst ))

IPPAPI (IppStatus, ippiDCT8x8To2x2Inv_16s_C1I, ( Ipp16s* pSrcDst ))
IPPAPI (IppStatus, ippiDCT8x8To4x4Inv_16s_C1I, ( Ipp16s* pSrcDst ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiDCT8x8Inv_A10_16s_C1, ippiDCT8x8Inv_A10_16s_C1I
//  Purpose:   Performs inverse DCT in the 8x8 buffer for 10 bits 16s data
//
//  Parameters:
//     pSrc       Pointer to the source buffer
//     pDst       Pointer to the destination buffer
//  Returns:
//     ippStsNoErr         No errors
//     ippStsNullPtrErr    One of the pointers is NULL
//  Notes:
//     Source data for these functions must be the result of the forward DCT
//     of data from the range [-512,511]
*/

IPPAPI (IppStatus, ippiDCT8x8Inv_A10_16s_C1,  ( const Ipp16s* pSrc, Ipp16s* pDst ))
IPPAPI (IppStatus, ippiDCT8x8Inv_A10_16s_C1I, ( Ipp16s* pSrcDst ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiDCT8x8Fwd_8u16s_C1R
//             ippiDCT8x8Inv_16s8u_C1R
//  Purpose:   Performs forward and inverse DCT in 8x8 buffer
//             for 16s data with conversion from/to 8u
//  Parameters:
//     pSrc      Pointer to the source buffer
//     pDst      Pointer to the destination buffer
//     srcStep   Step through the source image
//     dstStep   Step through the destination image
//  Returns:
//     ippStsNoErr        No errors
//     ippStsNullPtrErr   One of pointers is NULL
//     ippStsStepErr      srcStep or dstStep value is zero or negative
*/

IPPAPI (IppStatus, ippiDCT8x8Fwd_8u16s_C1R,
                   ( const Ipp8u* pSrc, int srcStep, Ipp16s* pDst ))

IPPAPI (IppStatus, ippiDCT8x8Inv_16s8u_C1R,
                   ( const Ipp16s* pSrc, Ipp8u* pDst, int dstStep ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiDCT8x8FwdLS_8u16s_C1R
//  Purpose:   Performs forward DCT in 8x8 buffer for 16s data
//             with conversion from 8u and level shift
//  Parameters:
//     pSrc      Pointer to start of source buffer
//     pDst      Pointer to start of destination buffer
//     srcStep   Step the source buffer
//     addVal    Constant value adding before DCT (level shift)
//  Returns:
//     ippStsNoErr         No errors
//     ippStsNullPtrErr    one of the pointers is NULL
//     ippStsStepErr       srcStep value is zero or negative
*/

IPPAPI (IppStatus, ippiDCT8x8FwdLS_8u16s_C1R,
                   ( const Ipp8u* pSrc, int srcStep, Ipp16s* pDst,
                     Ipp16s addVal ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiDCT8x8InvLSClip_16s8u_C1R
//  Purpose:   Performs inverse DCT in 8x8 buffer for 16s data
//             with level shift, clipping and conversion to 8u
//  Parameters:
//     pSrc      Pointer to the source buffer
//     pDst      Pointer to the destination buffer
//     dstStep   Step through the destination image
//     addVal    Constant value adding after DCT (level shift)
//     clipDown  Constant value for clipping (MIN)
//     clipUp    Constant value for clipping (MAX)
//  Returns:
//     ippStsNoErr           No errors
//     ippStsNullPtrErr      One of the pointers is NULL
//     ippStsStepErr         dstStep value is zero or negative
*/

IPPAPI (IppStatus, ippiDCT8x8InvLSClip_16s8u_C1R,
                   ( const Ipp16s* pSrc, Ipp8u* pDst, int dstStep,
                     Ipp16s addVal, Ipp8u clipDown, Ipp8u clipUp ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiDCT8x8Fwd_32f_C1, ippiDCT8x8Fwd_32f_C1I
//             ippiDCT8x8Inv_32f_C1, ippiDCT8x8Inv_32f_C1I
//  Purpose:   Performs forward or inverse DCT in the 8x8 buffer for 32f data
//
//  Parameters:
//     pSrc       Pointer to the source buffer
//     pDst       Pointer to the destination buffer
//     pSrcDst    Pointer to the source and destination buffer (in-place operations)
//  Returns:
//     ippStsNoErr         No errors
//     ippStsNullPtrErr    One of the pointers is NULL
*/

IPPAPI (IppStatus, ippiDCT8x8Fwd_32f_C1,  ( const Ipp32f* pSrc, Ipp32f* pDst ))
IPPAPI (IppStatus, ippiDCT8x8Inv_32f_C1,  ( const Ipp32f* pSrc, Ipp32f* pDst ))

IPPAPI (IppStatus, ippiDCT8x8Fwd_32f_C1I, ( Ipp32f* pSrcDst ))
IPPAPI (IppStatus, ippiDCT8x8Inv_32f_C1I, ( Ipp32f* pSrcDst ))




/* /////////////////////////////////////////////////////////////////////////////
//          Wavelet Transform Functions for User Filter Banks
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )

struct iWTFwdSpec_32f_C1R;
typedef struct iWTFwdSpec_32f_C1R IppiWTFwdSpec_32f_C1R;

struct iWTInvSpec_32f_C1R;
typedef struct iWTInvSpec_32f_C1R IppiWTInvSpec_32f_C1R;

struct iWTFwdSpec_32f_C3R;
typedef struct iWTFwdSpec_32f_C3R IppiWTFwdSpec_32f_C3R;

struct iWTInvSpec_32f_C3R;
typedef struct iWTInvSpec_32f_C3R IppiWTInvSpec_32f_C3R;

#endif /* _OWN_BLDPCS */


/* //////////////////////////////////////////////////////////////////////
// Name:       ippiWTFwdInitAlloc_32f_C1R
//             ippiWTFwdInitAlloc_32f_C3R
// Purpose:    Allocates and initializes
//                the forward wavelet transform context structure.
// Parameters:
//   pSpec        Pointer to pointer to allocated and initialized
//                 context structure;
//   pTapsLow     Pointer to lowpass filter taps;
//   lenLow       Length of lowpass filter;
//   anchorLow    Anchor position of lowpass filter;
//   pTapsHigh    Pointer to highpass filter taps;
//   lenHigh      Length of highpass filter;
//   anchorHigh   Anchor position of highpass filter.
//
// Returns:
//   ippStsNoErr        OK;
//   ippStsNullPtrErr   One of the pointers is NULL;
//   ippStsSizeErr      lenLow or lenHigh is less than 2;
//   ippStsAnchorErr    anchorLow or anchorHigh is less than zero;
//   ippStsMemAllocErr  Memory allocation failure for the context structure.
//
*/
IPPAPI (IppStatus, ippiWTFwdInitAlloc_32f_C1R, (IppiWTFwdSpec_32f_C1R** pSpec,
        const Ipp32f* pTapsLow,  int lenLow,  int anchorLow,
        const Ipp32f* pTapsHigh, int lenHigh, int anchorHigh))

IPPAPI (IppStatus, ippiWTFwdInitAlloc_32f_C3R, (IppiWTFwdSpec_32f_C3R** pSpec,
        const Ipp32f* pTapsLow,  int lenLow,  int anchorLow,
        const Ipp32f* pTapsHigh, int lenHigh, int anchorHigh))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippiWTFwdFree_32f_C1R
//              ippiWTFwdFree_32f_C3R
//
// Purpose:     Deallocates memory used by the
//               forward wavelet transform context structure.
//
// Parameters:
//   pSpec      Pointer to the context structure.
//
// Returns:
//   ippStsNoErr             OK;
//   ippStsNullPtrErr        Pointer to the context structure is NULL;
//   ippStsContextMatchErr   Invalid context structure.
//
*/
IPPAPI (IppStatus, ippiWTFwdFree_32f_C1R, (IppiWTFwdSpec_32f_C1R* pSpec))
IPPAPI (IppStatus, ippiWTFwdFree_32f_C3R, (IppiWTFwdSpec_32f_C3R* pSpec))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippiWTFwdGetBufSize_C1R
//              ippiWTFwdGetBufSize_C3R
//
// Purpose:     Computes the size of external work buffer for forward
//              wavelet transform
//
// Parameters:
//   pSpec     Pointer to the context structure.
//   pSize      Pointer to the variable that will receive the size of work buffer
//              required for forward wavelet transform.
//
// Returns:
//   ippStsNoErr            OK;
//   ippStsNullPtrErr       One of the pointers is NULL;
//   ippStsContextMatchErr  Invalid context structure.
//
// Notes:      if pointer to context structure is NULL,
//                      ippStsNullPtrErr will be returned.
*/
IPPAPI (IppStatus, ippiWTFwdGetBufSize_C1R,
                   ( const IppiWTFwdSpec_32f_C1R* pSpec, int* pSize ))

IPPAPI (IppStatus, ippiWTFwdGetBufSize_C3R,
                   ( const IppiWTFwdSpec_32f_C3R* pSpec, int* pSize ))



/* //////////////////////////////////////////////////////////////////////
// Name:        ippiWTFwd_32f_C1R
//              ippiWTFwd_32f_C3R
//
// Purpose:     Performs wavelet decomposition of an image.
//
// Parameters:
//   pSrc         Pointer to source image ROI;
//   srcStep      Step in bytes through the source image;
//   pApproxDst   Pointer to destination "approximation" image ROI;
//   approxStep   Step in bytes through the destination approximation image;
//   pDetailXDst  Pointer to the destination "horizontal details" image ROI;
//   detailXStep  Step in bytes through the destination horizontal detail image;
//   pDetailYDst  Pointer to the destination "vertical details" image ROI;
//   detailYStep  Step in bytes through the destination "vertical details" image;
//   pDetailXYDst Pointer to the destination "diagonal details" image ROI;
//   detailXYStep Step in bytes through the destination "diagonal details" image;
//   dstRoiSize   ROI size for all destination images.
//   pSpec        Pointer to the context structure.
//
// Returns:
//   ippStsNoErr            OK;
//   ippStsNullPtrErr       One of pointers is NULL;
//   ippStsSizeErr          dstRoiSize has a field with zero or negative value;
//   ippStsContextMatchErr  Invalid context structure.
//
// Notes:
//   No any fixed borders extension (wrap, symm.) will be applied!
//   Source image must have valid and accessible border data outside of ROI.
//
//   Only the same ROI sizes for destination images are supported.
//
//   Source ROI size should be calculated by the following rule:
//          srcRoiSize.width  = 2 * dstRoiSize.width;
//          srcRoiSize.height = 2 * dstRoiSize.height.
//
//   Conventional tokens for destination images have next meaning:
//    "Approximation"     - image obtained by vertical
//                              and horizontal lowpass filtering.
//    "Horizontal detail" - image obtained by vertical highpass
//                              and horizontal lowpass filtering.
//    "Vertical detail"   - image obtained by vertical lowpass
//                              and horizontal highpass filtering.
//    "Diagonal detail"   - image obtained by vertical
//                              and horizontal highpass filtering.
//   These tokens are used only for identification convenience.
//
//
*/
IPPAPI (IppStatus, ippiWTFwd_32f_C1R, (const Ipp32f* pSrc,  int srcStep,
        Ipp32f* pApproxDst,   int approxStep,
        Ipp32f* pDetailXDst,  int detailXStep,
        Ipp32f* pDetailYDst,  int detailYStep,
        Ipp32f* pDetailXYDst, int detailXYStep,
        IppiSize dstRoiSize, const IppiWTFwdSpec_32f_C1R* pSpec,
        Ipp8u* pBuffer))

IPPAPI (IppStatus, ippiWTFwd_32f_C3R, (const Ipp32f* pSrc,  int srcStep,
        Ipp32f* pApproxDst,   int approxStep,
        Ipp32f* pDetailXDst,  int detailXStep,
        Ipp32f* pDetailYDst,  int detailYStep,
        Ipp32f* pDetailXYDst, int detailXYStep,
        IppiSize dstRoiSize, const IppiWTFwdSpec_32f_C3R* pSpec,
        Ipp8u* pBuffer))


/* //////////////////////////////////////////////////////////////////////
// Name:       ippiWTInvInitAlloc_32f_C1R
//             ippiWTInvInitAlloc_32f_C3R
// Purpose:    Allocates and initializes
//                the inverse wavelet transform context structure.
// Parameters:
//   pSpec       Pointer to pointer to an allocated and initialized
//                 context structure;
//   pTapsLow    Pointer to lowpass filter taps;
//   lenLow      Length of the lowpass filter;
//   anchorLow   Anchor position of the lowpass filter;
//   pTapsHigh   Pointer to highpass filter taps;
//   lenHigh     Length of the highpass filter;
//   anchorHigh  Anchor position of the highpass filter.
//
// Returns:
//   ippStsNoErr       OK;
//   ippStsNullPtrErr  One of the pointers is NULL;
//   ippStsSizeErr     lenLow or lenHigh is less than 2;
//   ippStsAnchorErr   anchorLow or anchorHigh is less than zero;
//   ippStsMemAllocErr Memory allocation failure for the context structure.
//
// Notes:   anchor position value should be given for upsampled data.
//
*/
IPPAPI (IppStatus, ippiWTInvInitAlloc_32f_C1R, (IppiWTInvSpec_32f_C1R** pSpec,
        const Ipp32f* pTapsLow,  int lenLow,  int anchorLow,
        const Ipp32f* pTapsHigh, int lenHigh, int anchorHigh))

IPPAPI (IppStatus, ippiWTInvInitAlloc_32f_C3R, (IppiWTInvSpec_32f_C3R** pSpec,
        const Ipp32f* pTapsLow,  int lenLow,  int anchorLow,
        const Ipp32f* pTapsHigh, int lenHigh, int anchorHigh))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippiWTInvFree_32f_C1R
//              ippiWTInvFree_32f_C3R
//
// Purpose:     Deallocates memory used by
//                 the inverse wavelet transform context structure.
//
// Parameters:
//   pSpec      Pointer to the context structure.
//
// Returns:
//   ippStsNoErr            OK;
//   ippStsNullPtrErr       Pointer to the context structure is NULL;
//   ippStsContextMatchErr  Invalid context structure.
//
*/
IPPAPI (IppStatus, ippiWTInvFree_32f_C1R, (IppiWTInvSpec_32f_C1R* pSpec))
IPPAPI (IppStatus, ippiWTInvFree_32f_C3R, (IppiWTInvSpec_32f_C3R* pSpec))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippiWTInvGetBufSize_C1R
//              ippiWTInvGetBufSize_C3R
//
// Purpose:     Computes the size of external work buffer for
//                  an inverse wavelet transform.
//
// Parameters:
//   pSpec    Pointer to the context structure.
//   pSize    Pointer to the variable that will receive the size of work buffer.
//
// Returns:
//   ippStsNoErr            OK;
//   ippStsNullPtrErr       One of the pointers is NULL;
//   ippStsContextMatchErr  Invalid context structure.
//
// Notes:      if pointer to context structure is NULL,
//                      ippStsNullPtrErr will be returned.
*/
IPPAPI (IppStatus, ippiWTInvGetBufSize_C1R,
                   ( const IppiWTInvSpec_32f_C1R* pSpec, int* pSize ))

IPPAPI (IppStatus, ippiWTInvGetBufSize_C3R,
                   ( const IppiWTInvSpec_32f_C3R* pSpec, int* pSize ))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippiWTInv_32f_C1R
//              ippiWTInv_32f_C3R
//
// Purpose:     Performs wavelet reconstruction of an image.
//
// Parameters:
//   pApproxSrc    Pointer to the source "approximation" image ROI;
//   approxStep    Step in bytes through the source approximation image;
//   pDetailXSrc   Pointer to the source "horizontal details" image ROI;
//   detailXStep   Step in bytes through the source horizontal detail image;
//   pDetailYSrc   Pointer to the source "vertical details" image ROI;
//   detailYStep   Step in bytes through the source "vertical details" image;
//   pDetailXYSrc  Pointer to the source "diagonal details" image ROI;
//   detailXYStep  Step in bytes through the source "diagonal details" image;
//   srcRoiSize    ROI size for all source images.
//   pDst          Pointer to the destination image ROI;
//   dstStep       Step in bytes through the destination image;
//   pSpec         Pointer to the context structure;
//   pBuffer       Pointer to the allocated buffer for intermediate operations.
//
// Returns:
//   ippStsNoErr            OK;
//   ippStsNullPtrErr       One of the pointers is NULL;
//   ippStsSizeErr          srcRoiSize has a field with zero or negative value;
//   ippStsContextMatchErr  Invalid context structure.
//
// Notes:
//   No any fixed borders extension (wrap, symm.) will be applied! Source
//    images must have valid and accessible border data outside of ROI.
//
//   Only the same ROI size for source images supported. Destination ROI size
//     should be calculated by next rule:
//          dstRoiSize.width  = 2 * srcRoiSize.width;
//          dstRoiSize.height = 2 * srcRoiSize.height.
//
//
//   Monikers for the source images are in accordance with decomposition destination.
//
//
*/
IPPAPI (IppStatus, ippiWTInv_32f_C1R, (
        const Ipp32f* pApproxSrc,   int approxStep,
        const Ipp32f* pDetailXSrc,  int detailXStep,
        const Ipp32f* pDetailYSrc,  int detailYStep,
        const Ipp32f* pDetailXYSrc, int detailXYStep,
        IppiSize srcRoiSize, Ipp32f* pDst,  int dstStep,
        const IppiWTInvSpec_32f_C1R* pSpec, Ipp8u* pBuffer))

IPPAPI (IppStatus, ippiWTInv_32f_C3R, (
        const Ipp32f* pApproxSrc,   int approxStep,
        const Ipp32f* pDetailXSrc,  int detailXStep,
        const Ipp32f* pDetailYSrc,  int detailYStep,
        const Ipp32f* pDetailXYSrc, int detailXYStep,
        IppiSize srcRoiSize, Ipp32f* pDst,  int dstStep,
        const IppiWTInvSpec_32f_C3R* pSpec, Ipp8u* pBuffer))




/* /////////////////////////////////////////////////////////////////////////////
//                   Image resampling functions
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )

typedef enum
{
    ippResampleCatmullRom
} IppiResampleType;


struct ResampleRowSpec_32f;
typedef struct ResampleRowSpec_32f IppiResampleRowSpec_32f;

#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiResampleRowGetSize_32f
//
//  Purpose:    calculate the size of IppiResampleRowSpec_32f structure
//   and size of working buffer
//
//  Returns:
//    ippStsNoErr           OK
//    ippStsNullPtrErr      one of pointers is NULL
//    ippStsOutOfRangeErr   one or more SampleWidth is incorrect
//
//  Parameters:
//    srcSampleWidth        distance between neighbor pixels of a source row
//                                                         on the reference grid
//    dstSampleWidth        distance between neighbor pixels of a destination
//                                                     row on the reference grid
//    pSpecSize             pointer where to store the calculated
//                                               Spec structure size (in bytes)
//    pBufSize              pointer where to store buffer size (in bytes)
*/
IPPAPI(IppStatus, ippiResampleRowGetSize_32f, (
                            int  srcSampleWidth,
                            int  dstSampleWidth,
                            int *pSpecSize,
                            int *pBufSize))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiResampleRowInit_32f
//
//  Purpose:    initialize the IppiRowSpec structure by resampling parameters
//
//  Returns:
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pRowSpec is NULL pointer
//    ippStsBadArgErr       incorrect or unsupported filter type
//    ippStsOutOfRangeErr   one or more SampleWidth is incorrect
//
//  Parameters:
//    filterType            type of interpolation filter using for resampling
//    srcSampleWidth        distance between neighbor pixels of a source row
//                                                         on the reference grid
//    dstSampleWidth        distance between neighbor pixels of a destination
//                                                     row on the reference grid
//    pRowSpec              pointer where to store initialized structure
*/
IPPAPI(IppStatus, ippiResampleRowInit_32f, (
                            IppiResampleRowSpec_32f *pRowSpec,
                            IppiResampleType         filterType,
                            int                      srcSampleWidth,
                            int                      dstSampleWidth))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiResampleRowGetBorderWidth_32f
//
//  Purpose:    get the width of left and right borders to be applied around
//              the source buffer for ippiResampleRow (for version requiring
//              borders around source buffer)
//
//  Returns:
//    ippStsNoErr           OK
//    ippStsNullPtrErr      One of the pointers is NULL
//    ippStsBadArgErr       pointed IppiResampleRowSpec_32f is corrupted
//
//  Parameters:
//    pRowSpec              pointer to initialized IppiResampleRowSpec_32f
//    pLeftWidth            pointer where to store left border width (in pixels)
//    pRightWidth           pointer where to store right border width (in pixels)
*/
IPPAPI(IppStatus, ippiResampleRowGetBorderWidth_32f, (
                      const IppiResampleRowSpec_32f *pRowSpec,
                            int                     *pLeftWidth,
                            int                     *pRightWidth))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiResampleRowReplicateBorder_32f_C1R, ippiResampleRow_32f_C1
//
//  Purpose:    Resample rows with specified parameters.
//              ippiResampleRowReplicateBorder_32f_C1R function require borders
//                  around source buffer
//              ippiResampleRow_32f_C1 function replicate borders
//
//  Returns:
//    ippStsNoErr           OK
//    ippStsNullPtrErr      One of the pointers is NULL
//    ippStsBadArgErr       pointed IppiResampleRowSpec_32f is corrupted
//    ippStsOutOfRangeErr   xOriginRefGrid is out of range
//    ippStsSizeErr         sizeRefGrid is wrong
//
//  Parameters:
//    pSrc                  pointer to the source row origin
//    srcLineStep           distance in bytes between starts of consecutive lines
//                                                            in the source image
//    pDst                  pointer to the destination row origin
//    dstLineStep           distance in bytes between starts of consecutive lines
//                                                       in the destination image
//    xOriginRefGrid        X coordinate of the image origin at reference grid
//    sizeRefGrid           size of image in terms of  reference grid
//    pRowSpec              pointer to initialized IppiResampleRowSpec_32f
//    pBuffer               pointer to work buffer
*/

IPPAPI(IppStatus, ippiResampleRowReplicateBorder_32f_C1R, (
                      const Ipp32f *pSrc, int srcLineStep,
                            Ipp32f *pDst, int dstLineStep,
                            int                      xOriginRefGrid,
                            IppiSize                 sizeRefGrid,
                      const IppiResampleRowSpec_32f *pRowSpec,
                            Ipp8u                   *pBuffer))

IPPAPI(IppStatus, ippiResampleRow_32f_C1, (
                      const Ipp32f *pSrc, int srcLineStep,
                            Ipp32f *pDst, int dstLineStep,
                            int                      xOriginRefGrid,
                            IppiSize                 sizeRefGrid,
                      const IppiResampleRowSpec_32f *pRowSpec,
                            Ipp8u                   *pBuffer))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippiDecimateFilterRow_8u_C1R,
//                      ippiDecimateFilterColumn_8u_C1R
//  Purpose:            Decimate the image with specified filters
//                      in horizontal or vertical directions
//  Parameters:
//    pSrc              source image data
//    srcStep           step in source image
//    srcRoiSize        region of interest of source image
//    pDst              resultant image data
//    dstStep           step in destination image
//    fraction          they specify fractions of decimating
//  Returns:
//    ippStsNoErr       no errors
//    ippStsNullPtrErr  one of the pointers is NULL
//    ippStsStepErr     one of the step values is zero or negative
//    ippStsSizeErr     srcRoiSize has a field with negative or zero value
//    ippStsDecimateFractionErr (fraction != ippPolyphase_1_2) &&
//                              (fraction != ippPolyphase_3_5) &&
//                              (fraction != ippPolyphase_2_3) &&
//                              (fraction != ippPolyphase_7_10) &&
//                              (fraction != ippPolyphase_3_4)
*/

IPPAPI(IppStatus, ippiDecimateFilterRow_8u_C1R, (
    const Ipp8u* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp8u* pDst, int dstStep, IppiFraction fraction))
IPPAPI(IppStatus, ippiDecimateFilterColumn_8u_C1R, (
    const Ipp8u* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp8u* pDst, int dstStep, IppiFraction fraction))


/* /////////////////////////////////////////////////////////////////////////////
//                   Geometric Transform functions
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )
typedef enum {
    ippAxsHorizontal,
    ippAxsVertical,
    ippAxsBoth
} IppiAxis;


#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:        ippiMirror
//
//  Purpose:     Mirrors an image about a horizontal
//               or vertical axis, or both
//
//  Context:
//
//  Returns:     IppStatus
//    ippStsNoErr         No errors
//    ippStsNullPtrErr    pSrc == NULL, or pDst == NULL
//    ippStsSizeErr,      roiSize has a field with zero or negative value
//    ippStsMirrorFlipErr (flip != ippAxsHorizontal) &&
//                        (flip != ippAxsVertical) &&
//                        (flip != ippAxsBoth)
//
//  Parameters:
//    pSrc       Pointer to the source image
//    srcStep    Step through the source image
//    pDst       Pointer to the destination image
//    dstStep    Step through the destination image
//    pSrcDst    Pointer to the source/destination image (in-place flavors)
//    srcDstStep Step through the source/destination image (in-place flavors)
//    roiSize    Size of the ROI
//    flip       Specifies the axis to mirror the image about:
//                 ippAxsHorizontal     horizontal axis,
//                 ippAxsVertical       vertical axis,
//                 ippAxsBoth           both horizontal and vertical axes
//
//  Notes:
//
*/

IPPAPI(IppStatus, ippiMirror_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                      IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_8u_C3R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                      IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_8u_AC4R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                       IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_8u_C4R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                                       IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_8u_C1IR, (Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_8u_C3IR, (Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_8u_AC4IR, (Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_8u_C4IR, (Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))

IPPAPI(IppStatus, ippiMirror_16u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
                                       IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_16u_C3R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
                                       IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_16u_AC4R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
                                        IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_16u_C4R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
                                                        IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_16u_C1IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_16u_C3IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_16u_AC4IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI (IppStatus, ippiMirror_16u_C4IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))

IPPAPI(IppStatus, ippiMirror_32s_C1R, (const Ipp32s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
                                       IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_32s_C3R, (const Ipp32s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
                                       IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_32s_AC4R, (const Ipp32s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
                                        IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_32s_C4R, (const Ipp32s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
                                                        IppiSize roiSize, IppiAxis flip ) )

IPPAPI(IppStatus, ippiMirror_32s_C1IR, (Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_32s_C3IR, (Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_32s_AC4IR, (Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_32s_C4IR, (Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))

IPPAPI(IppStatus, ippiMirror_16s_C1R, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
                                       IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_16s_C3R, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
                                       IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_16s_AC4R, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
                                        IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_16s_C4R, (const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
                                                        IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_16s_C1IR, (Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_16s_C3IR, (Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_16s_AC4IR, (Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI (IppStatus, ippiMirror_16s_C4IR, (Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))

IPPAPI(IppStatus, ippiMirror_32f_C1R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                       IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_32f_C3R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                       IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_32f_AC4R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                        IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_32f_C4R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                                        IppiSize roiSize, IppiAxis flip ) )
IPPAPI(IppStatus, ippiMirror_32f_C1IR, (Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_32f_C3IR, (Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_32f_AC4IR, (Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))
IPPAPI(IppStatus, ippiMirror_32f_C4IR, (Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, IppiAxis flip))


/* /////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:           ippiRemap
//  Purpose:   Transforms the source image by remapping its pixels
//                  dst[i,j] = src[xMap[i,j], yMap[i,j]]
//  Context:
//  Returns:             IppStatus
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            srcRoi or dstRoiSize has a field with zero or negative value
//    ippStsStepErr            One of the step values is zero or negative
//    ippStsInterpolateErr     interpolation has an illegal value
//
//  Parameters:
//    pSrc          Pointer to the source image (point to pixel (0,0)). An array
//                  of pointers to each plane of the source image for planar data.
//    srcSize       Size of the source image.
//    srcStep       Step through the source image
//    srcRoi        Region if interest in the source image.
//    pxMap         Pointer to image with x coordinates of map.
//    xMapStep      The step in xMap image
//    pyMap         The pointer to image with y coordinates of map.
//    yMapStep      The step in yMap image
//    pDst          Pointer to the destination image. An array of pointers
//                  to each plane of the destination image for planar data.
//    dstStep       Step through the destination image
//    dstRoiSize    Size of the destination ROI
//    interpolation The type of interpolation to perform for image resampling
//                  The following types are currently supported:
//              IPPI_INTER_NN     Nearest neighbor interpolation.
//              IPPI_INTER_LINEAR Linear interpolation.
//              IPPI_INTER_CUBIC  Cubic interpolation.
//  Notes:
*/

IPPAPI(IppStatus, ippiRemap_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep,
    const Ipp32f* pyMap, int yMapStep, Ipp8u* pDst, int dstStep,
    IppiSize dstRoiSize, int interpolation))

IPPAPI(IppStatus, ippiRemap_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep,
    const Ipp32f* pyMap, int yMapStep, Ipp8u* pDst, int dstStep,
    IppiSize dstRoiSize, int interpolation))

IPPAPI(IppStatus, ippiRemap_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep,
    const Ipp32f* pyMap, int yMapStep, Ipp8u* pDst, int dstStep,
    IppiSize dstRoiSize, int interpolation))

IPPAPI(IppStatus, ippiRemap_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep, const Ipp32f* pyMap,
    int yMapStep, Ipp8u* const pDst[3], int dstStep, IppiSize dstRoi, int interpolation))

IPPAPI(IppStatus, ippiRemap_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep,
    const Ipp32f* pyMap, int yMapStep, Ipp8u* pDst, int dstStep,
    IppiSize dstRoiSize, int interpolation))

IPPAPI(IppStatus, ippiRemap_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep, const Ipp32f* pyMap,
    int yMapStep, Ipp8u* const pDst[4], int dstStep, IppiSize dstRoi, int interpolation))

IPPAPI(IppStatus, ippiRemap_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep,
    const Ipp32f* pyMap, int yMapStep, Ipp32f* pDst, int dstStep,
    IppiSize dstRoiSize, int interpolation))

IPPAPI(IppStatus, ippiRemap_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep,
    const Ipp32f* pyMap, int yMapStep, Ipp32f* pDst, int dstStep,
    IppiSize dstRoiSize, int interpolation))

IPPAPI(IppStatus, ippiRemap_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep,
    const Ipp32f* pyMap, int yMapStep, Ipp32f* pDst, int dstStep,
    IppiSize dstRoiSize, int interpolation))

IPPAPI(IppStatus, ippiRemap_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep, const Ipp32f* pyMap,
    int yMapStep, Ipp32f* const pDst[3], int dstStep, IppiSize dstRoi, int interpolation))

IPPAPI(IppStatus, ippiRemap_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep,
    const Ipp32f* pyMap, int yMapStep, Ipp32f* pDst, int dstStep,
    IppiSize dstRoiSize, int interpolation))

IPPAPI(IppStatus, ippiRemap_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep, const Ipp32f* pyMap,
    int yMapStep, Ipp32f* const pDst[4], int dstStep, IppiSize dstRoi, int interpolation))

IPPAPI(IppStatus, ippiRemap_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep,
    const Ipp32f* pyMap, int yMapStep, Ipp16u* pDst, int dstStep,
    IppiSize dstRoiSize, int interpolation))

IPPAPI(IppStatus, ippiRemap_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep,
    const Ipp32f* pyMap, int yMapStep, Ipp16u* pDst, int dstStep,
    IppiSize dstRoiSize, int interpolation))

IPPAPI(IppStatus, ippiRemap_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep,
    const Ipp32f* pyMap, int yMapStep, Ipp16u* pDst, int dstStep,
    IppiSize dstRoiSize, int interpolation))

IPPAPI(IppStatus, ippiRemap_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep, const Ipp32f* pyMap,
    int yMapStep, Ipp16u* const pDst[3], int dstStep, IppiSize dstRoi, int interpolation))

IPPAPI(IppStatus, ippiRemap_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep,
    const Ipp32f* pyMap, int yMapStep, Ipp16u* pDst, int dstStep,
    IppiSize dstRoiSize, int interpolation))

IPPAPI(IppStatus, ippiRemap_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize,
    int srcStep, IppiRect srcRoi, const Ipp32f* pxMap, int xMapStep, const Ipp32f* pyMap,
    int yMapStep, Ipp16u* const pDst[4], int dstStep, IppiSize dstRoi, int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:               ippiResize
//  Purpose:            Resize source image by xFactor and yFactor
//  Parameters:
//    pSrc              source image data
//    srcSize           size of src
//    srcStep           step in src
//    srcROI            region of interest of src
//    pDst              resultant image data
//    dstStep           step in dst
//    dstROI            region of interest of dst
//    xFactor, yFactor  they specify fractions of resizing
//    interpolation     type of interpolation to perform for resampling the input image:
//                      IPPI_INTER_NN      nearest neighbour interpolation
//                      IPPI_INTER_LINEAR  linear interpolation
//                      IPPI_INTER_CUBIC   cubic convolution interpolation
//                      IPPI_INTER_LANCZOS interpolation by 3-lobed Lanczos-windowed sinc function
//                      IPPI_INTER_SUPER   supersampling interpolation
//  Returns:
//    ippStsNoErr             no errors
//    ippStsNullPtrErr        pSrc == NULL or pDst == NULL
//    ippStsSizeErr           width or height of images is less or equal zero
//    ippStsWrongIntersectROI srcRoi hasn`t intersection with the source image, no operation
//    ippStsResizeNoOperationErr one of the output image dimensions is less than 1 pixel
//    ippStsResizeFactorErr   xFactor or yFactor is less or equal zero
//    ippStsInterpolationErr  (interpolation != IPPI_INTER_NN) &&
//                            (interpolation != IPPI_INTER_LINEAR) &&
//                            (interpolation != IPPI_INTER_CUBIC) &&
//                            (interpolation != IPPI_INTER_LANCZOS) &&
//                            (interpolation != IPPI_INTER_SUPER)
//  Notes:
//    not in-place
*/

IPP_DEPRECATED("use ippiResizeSqrPixel_8u_C1R function instead of this one") \
IPPAPI(IppStatus, ippiResize_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_8u_C3R function instead of this one") \
IPPAPI(IppStatus, ippiResize_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_8u_C4R function instead of this one") \
IPPAPI(IppStatus, ippiResize_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_8u_AC4R function instead of this one") \
IPPAPI(IppStatus, ippiResize_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_8u_P3R function instead of this one") \
IPPAPI(IppStatus, ippiResize_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp8u* const pDst[3], int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_8u_P4R function instead of this one") \
IPPAPI(IppStatus, ippiResize_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp8u* const pDst[4], int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_16u_C1R function instead of this one") \
IPPAPI(IppStatus, ippiResize_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_16u_C3R function instead of this one") \
IPPAPI(IppStatus, ippiResize_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_16u_C4R function instead of this one") \
IPPAPI(IppStatus, ippiResize_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_16u_AC4R function instead of this one") \
IPPAPI(IppStatus, ippiResize_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_16u_P3R function instead of this one") \
IPPAPI(IppStatus, ippiResize_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp16u* const pDst[3], int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_16u_P4R function instead of this one") \
IPPAPI(IppStatus, ippiResize_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp16u* const pDst[4], int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_32f_C1R function instead of this one") \
IPPAPI(IppStatus, ippiResize_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_32f_C3R function instead of this one") \
IPPAPI(IppStatus, ippiResize_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_32f_C4R function instead of this one") \
IPPAPI(IppStatus, ippiResize_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_32f_AC4R function instead of this one") \
IPPAPI(IppStatus, ippiResize_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_32f_P3R function instead of this one") \
IPPAPI(IppStatus, ippiResize_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp32f* const pDst[3], int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_32f_P4R function instead of this one") \
IPPAPI(IppStatus, ippiResize_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp32f* const pDst[4], int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:               ippiResizeCenter
//  Purpose:            Resize source image from defined point by xFactor and yFactor
//  Parameters:
//    pSrc              source image data
//    srcSize           size of src
//    srcStep           step in src
//    srcROI            region of interest of src
//    pDst              resultant image data
//    dstStep           step in dst
//    dstROI            region of interest of dst
//    xFactor, yFactor  they specify fractions of resizing
//    interpolation     type of interpolation to perform for resampling the input image:
//                      IPPI_INTER_NN      nearest neighbour interpolation
//                      IPPI_INTER_LINEAR  linear interpolation
//                      IPPI_INTER_CUBIC   cubic convolution interpolation
//                      IPPI_INTER_LANCZOS interpolation by 3-lobed Lanczos-windowed sinc function
//                      IPPI_INTER_SUPER   supersampling interpolation
//  Returns:
//    ippStsNoErr             no errors
//    ippStsNullPtrErr        pSrc == NULL or pDst == NULL
//    ippStsSizeErr           width or height of images is less or equal zero
//    ippStsWrongIntersectROI srcRoi hasn`t intersection with the source image, no operation
//    ippStsResizeNoOperationErr one of the output image dimensions is less than 1 pixel
//    ippStsResizeFactorErr   xFactor or yFactor is less or equal zero
//    ippStsInterpolationErr  (interpolation != IPPI_INTER_NN) &&
//                            (interpolation != IPPI_INTER_LINEAR) &&
//                            (interpolation != IPPI_INTER_CUBIC) &&
//                            (interpolation != IPPI_INTER_LANCZOS) &&
//                            (interpolation != IPPI_INTER_SUPER)
//  Notes:
//    not in-place
*/

IPP_DEPRECATED("use ippiResizeSqrPixel_8u_C1R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_8u_C3R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_8u_C4R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_8u_AC4R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_8u_P3R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp8u* const pDst[3], int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_8u_P4R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp8u* const pDst[4], int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_16u_C1R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_16u_C3R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_16u_C4R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_16u_AC4R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_16u_P3R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp16u* const pDst[3], int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_16u_P4R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp16u* const pDst[4], int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_32f_C1R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_32f_C3R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_32f_C4R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_32f_AC4R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_32f_P3R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp32f* const pDst[3], int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel_32f_P4R function instead of this one") \
IPPAPI(IppStatus, ippiResizeCenter_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp32f* const pDst[4], int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor,
                                            double xCenter, double yCenter, int interpolation))


/* ///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Name:               ippiGetResizeFract
//  Context:            Recalculate resize factors for farther tile processing
//  Parameters:
//    srcSize           size of src
//    srcROI            region of interest of src
//    xFactor, yFactor  they specify fractions of resizing
//    xFr, yFr          pointers to modified factors
//    interpolation     type of interpolation to perform for resampling the input image:
//                      IPPI_INTER_NN      nearest neighbour interpolation
//                      IPPI_INTER_LINEAR  linear interpolation
//                      IPPI_INTER_CUBIC   cubic convolution interpolation
//                      IPPI_INTER_LANCZOS interpolation by 3-lobed Lanczos-windowed sinc function
//  Returns:
//    ippStsNoErr             no errors
//    ippStsSizeErr           width or height of images is less or equal zero
//    ippStsWrongIntersectROI srcRoi hasn`t intersection with the source image, no operation
//    ippStsResizeFactorErr   xFactor or yFactor is less or equal zero
//    ippStsInterpolationErr  (interpolation != IPPI_INTER_NN) &&
//                            (interpolation != IPPI_INTER_LINEAR) &&
//                            (interpolation != IPPI_INTER_CUBIC) &&
//                            (interpolation != IPPI_INTER_LANCZOS)
*/

IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiGetResizeFract, (IppiSize srcSize, IppiRect srcRoi, double xFactor, double yFactor,
                                       double* xFr, double* yFr, int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:               ippiResizeShift
//  Context:            Resize source image by xFactor and yFactor for tile processing
//  Parameters:
//    pSrc              source image data
//    srcSize           size of src
//    srcStep           step in src
//    srcROI            region of interest of src
//    pDst              resultant image data (size of tile)
//    dstStep           step in dst
//    dstROI            region of interest of dst
//    xFr, yFr          they specify fractions of tile resizing
//    xShift, yShift    they specify offsets(double) for tile processing
//    interpolation     type of interpolation to perform for resampling the input image:
//                      IPPI_INTER_NN      nearest neighbour interpolation
//                      IPPI_INTER_LINEAR  linear interpolation
//                      IPPI_INTER_CUBIC   cubic convolution interpolation
//                      IPPI_INTER_LANCZOS interpolation by 3-lobed Lanczos-windowed sinc function
//  Returns:
//    ippStsNoErr             no errors
//    ippStsNullPtrErr        pSrc == NULL or pDst == NULL
//    ippStsSizeErr           width or height of images is less or equal zero
//    ippStsResizeFactorErr   xFactor or yFactor is less or equal zero
//    ippStsInterpolationErr  (interpolation != IPPI_INTER_NN) &&
//                            (interpolation != IPPI_INTER_LINEAR) &&
//                            (interpolation != IPPI_INTER_CUBIC) &&
//                            (interpolation != IPPI_INTER_LANCZOS)
//  Notes:
//    not in-place
//    without supersampling interpolation
*/

IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp8u* const pDst[3], int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp8u* const pDst[4], int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp16u* const pDst[3], int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp16u* const pDst[4], int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp32f* const pDst[3], int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))
IPP_DEPRECATED("use ippiResizeSqrPixel functions for tile processing") \
IPPAPI(IppStatus, ippiResizeShift_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                       Ipp32f* const pDst[4], int dstStep, IppiSize dstRoiSize,
                                       double xFr, double yFr, double xShift, double yShift, int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:               ippiResizeYUV422_8u_C2R
//  Purpose:            Resize source image by xFactor and yFactor
//  Parameters:
//    pSrc              source image data
//    srcSize           size of src
//    srcStep           step in src
//    srcROI            region of interest of src
//    pDst              resultant image data
//    dstStep           step in dst
//    dstROI            region of interest of dst
//    xFactor, yFactor  they specify fractions of resizing
//    interpolation     type of interpolation to perform for resampling the input image:
//                      IPPI_INTER_NN      nearest neighbour interpolation
//                      IPPI_INTER_LINEAR  linear interpolation
//                      IPPI_INTER_CUBIC   cubic convolution interpolation
//  Returns:
//    ippStsNoErr             no errors
//    ippStsNullPtrErr        pSrc == NULL or pDst == NULL
//    ippStsSizeErr           width or height of images is less or equal zero,
//                            width of src image is odd,
//                            width of region of interest of src is odd,
//                            xoffset of region of interest of src is odd,
//                            width of region of interest of dst is less two
//    ippStsResizeNoOperationErr one of the output image dimensions is less than 1 pixel
//    ippStsResizeFactorErr   xFactor or yFactor is less or equal zero
//    ippStsInterpolationErr  (interpolation != IPPI_INTER_NN) &&
//                            (interpolation != IPPI_INTER_LINEAR) &&
//                            (interpolation != IPPI_INTER_CUBIC)
//  Notes:
//    YUY2 pixel format (Y0U0Y1V0,Y2U1Y3V1,.. or Y0Cb0Y1Cr0,Y2Cb1Y3Cr1,..)
//    not in-place
*/

IPPAPI(IppStatus, ippiResizeYUV422_8u_C2R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                            Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                            double xFactor, double yFactor, int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:               ippiResizeSqrPixelGetBufSize
//  Purpose:            Computes the size of an external work buffer (in bytes)
//  Parameters:
//    dstSize           size of destination image
//    nChannel          number of channels
//    interpolation     type of interpolation to perform for resizing the input image:
//                        IPPI_INTER_NN      nearest neighbor interpolation
//                        IPPI_INTER_LINEAR  linear interpolation
//                        IPPI_INTER_CUBIC   cubic polynomial interpolation
//                        IPPI_INTER_LANCZOS interpolation by Lanczos3-windowed sinc function
//                      including two-parameter cubic filters:
//                        IPPI_INTER_CUBIC2P_BSPLINE      B-spline filter (1, 0)
//                        IPPI_INTER_CUBIC2P_CATMULLROM   Catmull-Rom filter (0, 1/2)
//                        IPPI_INTER_CUBIC2P_B05C03       special filter with parameters (1/2, 3/10)
//    pSize             pointer to the external buffer`s size
//  Returns:
//    ippStsNoErr             no errors
//    ippStsNullPtrErr        pSize == NULL
//    ippStsSizeErr           width or height of dst is less or equal zero
//    ippStsNumChannelsErr    number of channels is not 1, 3 or 4
//    ippStsInterpolationErr  (interpolation != IPPI_INTER_NN) &&
//                            (interpolation != IPPI_INTER_LINEAR) &&
//                            (interpolation != IPPI_INTER_CUBIC) &&
//                            (interpolation != IPPI_INTER_CUBIC2P_BSPLINE) &&
//                            (interpolation != IPPI_INTER_CUBIC2P_CATMULLROM) &&
//                            (interpolation != IPPI_INTER_CUBIC2P_B05C03) &&
//                            (interpolation != IPPI_INTER_LANCZOS)
*/

IPP_DEPRECATED("use ippiResizeGetBufSize function instead of this one") \
IPPAPI(IppStatus, ippiResizeSqrPixelGetBufSize, (
       IppiSize dstSize, int nChannel, int interpolation, int* pSize))

/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:               ippiResizeGetBufSize
//  Purpose:            Computes the size of an external work buffer (in bytes)
//  Parameters:
//    srcROI            region of interest of source image
//    dstROI            region of interest of destination image
//    nChannel          number of channels
//    interpolation     type of interpolation to perform for resizing the input image:
//                        IPPI_INTER_NN      Nearest Neighbor interpolation
//                        IPPI_INTER_LINEAR  Linear interpolation
//                        IPPI_INTER_CUBIC   Cubic polynomial interpolation
//                        IPPI_INTER_LANCZOS Interpolation by Lanczos3-windowed sinc function
//                        IPPI_INTER_SUPER   Super Sampling method
//                      including two-parameter cubic filters:
//                        IPPI_INTER_CUBIC2P_BSPLINE    B-spline filter (1, 0)
//                        IPPI_INTER_CUBIC2P_CATMULLROM Catmull-Rom filter (0, 1/2)
//                        IPPI_INTER_CUBIC2P_B05C03     Special filter with parameters (1/2, 3/10)
//                      including special feature for smoothing:
//                        IPPI_SMOOTH_EDGE   Edges smoothing in addition to one of general methods
//                        IPPI_SUBPIXEL_EDGE Edges handling in addition to one of general methods
//    pSize             pointer to the external buffer`s size
//  Returns:
//    ippStsNoErr             no errors
//    ippStsNullPtrErr        pSize == NULL
//    ippStsSizeErr           width or height of images is less or equal zero
//    ippStsNumChannelsErr    number of channels is not 1, 3 or 4
//    ippStsInterpolationErr  interpolation has an illegal value
*/

IPPAPI(IppStatus, ippiResizeGetBufSize, (
       IppiRect srcROI, IppiRect dstROI, int nChannel, int interpolation, int* pSize))

/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:               ippiResizeSqrPixel
//  Purpose:            Performs RESIZE transform of the source image by xFactor and yFactor
//                            |X'|   |xFactor    0   |   |X|   |xShift|
//                            |  | = |               | * | | + |      |
//                            |Y'|   |   0    yFactor|   |Y|   |yShift|
//  Parameters:
//    pSrc              source image data
//    srcSize           size of source image
//    srcStep           step in source image
//    srcROI            region of interest of source image
//    pDst              resultant image data
//    dstStep           step in destination image
//    dstROI            region of interest of destination image
//    xFactor, yFactor  they specify fractions of resizing
//    xShift, yShift    they specify shifts of resizing:
//    interpolation     type of interpolation to perform for resizing the input image:
//                        IPPI_INTER_NN      Nearest Neighbor interpolation
//                        IPPI_INTER_LINEAR  Linear interpolation
//                        IPPI_INTER_CUBIC   Cubic polynomial interpolation
//                        IPPI_INTER_LANCZOS Interpolation by Lanczos3-windowed sinc function
//                        IPPI_INTER_SUPER   Super Sampling method
//                      including two-parameter cubic filters:
//                        IPPI_INTER_CUBIC2P_BSPLINE    B-spline filter (1, 0)
//                        IPPI_INTER_CUBIC2P_CATMULLROM Catmull-Rom filter (0, 1/2)
//                        IPPI_INTER_CUBIC2P_B05C03     Special filter with parameters (1/2, 3/10)
//                      including special feature for smoothing:
//                        IPPI_SMOOTH_EDGE    Edges smoothing in addition to one of the above methods
//                        IPPI_SUBPIXEL_EDGE Edges handling in addition to one of general methods
//    pBuffer           pointer to work buffer
//  Returns:
//    ippStsNoErr             no errors
//    ippStsNullPtrErr        pSrc == NULL or pDst == NULL or pBuffer == NULL
//    ippStsSizeErr           width or height of images is less or equal zero
//    ippStsWrongIntersectROI srcRoi has not intersection with the source image, no operation
//    ippStsResizeFactorErr   xFactor or yFactor is less or equal zero
//    ippStsInterpolationErr  interpolation has an illegal value
*/

IPPAPI(IppStatus, ippiResizeSqrPixel_8u_C1R, (
       const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp8u* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_8u_C3R, (
       const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp8u* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_8u_C4R, (
       const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp8u* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_8u_AC4R, (
       const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp8u* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_8u_P3R, (
       const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp8u* const pDst[3], int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_8u_P4R, (
       const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp8u* const pDst[4], int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_16u_C1R, (
       const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp16u* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_16u_C3R, (
       const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp16u* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_16u_C4R, (
       const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp16u* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_16u_AC4R, (
       const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp16u* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_16u_P3R, (
       const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp16u* const pDst[3], int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_16u_P4R, (
       const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp16u* const pDst[4], int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_16s_C1R, (
       const Ipp16s* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp16s* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_16s_C3R, (
       const Ipp16s* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp16s* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_16s_C4R, (
       const Ipp16s* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp16s* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_16s_AC4R, (
       const Ipp16s* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp16s* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_16s_P3R, (
       const Ipp16s* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp16s* const pDst[3], int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_16s_P4R, (
       const Ipp16s* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp16s* const pDst[4], int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_32f_C1R, (
       const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp32f* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_32f_C3R, (
       const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp32f* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_32f_C4R, (
       const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp32f* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_32f_AC4R, (
       const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp32f* pDst, int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_32f_P3R, (
       const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp32f* const pDst[3], int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))
IPPAPI(IppStatus, ippiResizeSqrPixel_32f_P4R, (
       const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcROI,
       Ipp32f* const pDst[4], int dstStep, IppiRect dstROI,
       double xFactor, double yFactor, double xShift, double yShift,
       int interpolation, Ipp8u *pBuffer))


/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:               ippiSuperSamplingGetBufSize
//  Purpose:            Computes the size of an external work buffer (in bytes)
//  Parameters:
//    srcRoiSize        region of interest of source image
//    dstRoiSize        region of interest of destination image
//    nChannel          number of channels
//    pSize             pointer to the external buffer`s size
//  Returns:
//    ippStsNoErr       no errors
//    ippStsNullPtrErr  pSize == NULL
//    ippStsSizeErr     one of the image dimensions has zero or negative value
//                      or any dimension of src less than appropriate dimension of dst
//    ippStsNumChannelsErr number of channels is not 1 or 3 or 4
*/

IPPAPI(IppStatus, ippiSuperSamplingGetBufSize, (
    IppiSize srcRoiSize, IppiSize dstRoiSize, int nChannel, int *pSize))

/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:               ippiSuperSampling
//  Purpose:            Performs RESIZE transform using Super Sampling method
//  Parameters:
//    pSrc              source image data
//    srcStep           step in source image
//    srcRoiSize        region of interest of source image
//    pDst              resultant image data
//    dstStep           step in destination image
//    dstRoiSize        region of interest of destination image
//    pBuffer           pointer to work buffer
//  Returns:
//    ippStsNoErr       no errors
//    ippStsNullPtrErr  pSrc == NULL or pDst == NULL or pBuffer == NULL
//    ippStsStepErr     srcStep or dstStep has a zero or negative value
//    ippStsSizeErr     one of the image dimensions has zero or negative value
//                      or any dimension of src less than appropriate dimension of dst
*/

IPPAPI(IppStatus, ippiSuperSampling_8u_C1R, (
    const Ipp8u* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_8u_C3R, (
    const Ipp8u* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_8u_C4R, (
    const Ipp8u* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_8u_AC4R, (
    const Ipp8u* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_8u_P3R, (
    const Ipp8u* const pSrc[3], int srcStep, IppiSize srcRoiSize,
    Ipp8u* const pDst[3], int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_8u_P4R, (
    const Ipp8u* const pSrc[4], int srcStep, IppiSize srcRoiSize,
    Ipp8u* const pDst[4], int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_16u_C1R, (
    const Ipp16u* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_16u_C3R, (
    const Ipp16u* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_16u_C4R, (
    const Ipp16u* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_16u_AC4R, (
    const Ipp16u* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_16u_P3R, (
    const Ipp16u* const pSrc[3], int srcStep, IppiSize srcRoiSize,
    Ipp16u* const pDst[3], int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_16u_P4R, (
    const Ipp16u* const pSrc[4], int srcStep, IppiSize srcRoiSize,
    Ipp16u* const pDst[4], int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_16s_C1R, (
    const Ipp16s* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp16s* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_16s_C3R, (
    const Ipp16s* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp16s* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_16s_C4R, (
    const Ipp16s* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp16s* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_16s_AC4R, (
    const Ipp16s* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp16s* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_16s_P3R, (
    const Ipp16s* const pSrc[3], int srcStep, IppiSize srcRoiSize,
    Ipp16s* const pDst[3], int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_16s_P4R, (
    const Ipp16s* const pSrc[4], int srcStep, IppiSize srcRoiSize,
    Ipp16s* const pDst[4], int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_32f_C1R, (
    const Ipp32f* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_32f_C3R, (
    const Ipp32f* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_32f_C4R, (
    const Ipp32f* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_32f_AC4R, (
    const Ipp32f* pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_32f_P3R, (
    const Ipp32f* const pSrc[3], int srcStep, IppiSize srcRoiSize,
    Ipp32f* const pDst[3], int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))
IPPAPI(IppStatus, ippiSuperSampling_32f_P4R, (
    const Ipp32f* const pSrc[4], int srcStep, IppiSize srcRoiSize,
    Ipp32f* const pDst[4], int dstStep, IppiSize dstRoiSize,
    Ipp8u* pBuffer))


/* /////////////////////////////////////////////////////////////////////////////
//                     Resize filter function
// /////////////////////////////////////////////////////////////////////////////
//
//  Name:               ippiResizeFilterGetSize_8u_C1R
//  Purpose:            Computes pState size for resize filter (in bytes)
//  Parameters:
//    srcRoiSize        region of interest of source image
//    dstRoiSize        region of interest of destination image
//    filter            type of resize filter
//    pSize             pointer to State size
//  Returns:
//    ippStsNoErr       no errors
//    ippStsSizeErr     width or height of images is less or equal to zero
//    ippStsNotSupportedModeErr filter type is not supported
//    ippStsNullPtrErr  pointer to buffer size is NULL
*/

IPPAPI(IppStatus, ippiResizeFilterGetSize_8u_C1R, (
    IppiSize srcRoiSize, IppiSize dstRoiSize, IppiResizeFilterType filter, Ipp32u *pSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippiResizeFilterInit_8u_C1R
//  Purpose:            Initialization of State for resize filter
//  Parameters:
//    pState            pointer to State
//    srcRoiSize        region of interest of source image
//    dstRoiSize        region of interest of destination image
//    filter            type of resize filter
//  Returns:
//    ippStsNoErr       no errors
//    ippStsNullPtrErr  pointer to Spec is NULL
//    ippStsSizeErr     width or height of images is less or equal to zero
//    ippStsNotSupportedModeErr filter type is not supported
*/

IPPAPI(IppStatus, ippiResizeFilterInit_8u_C1R, (
    IppiResizeFilterState *pState, IppiSize srcRoiSize, IppiSize dstRoiSize, IppiResizeFilterType filter))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippiResizeFilter_8u_C1R
//  Purpose:            Performs RESIZE transform using generic filter
//  Parameters:
//    pSrc              source image data
//    srcStep           step in source image
//    srcRoiSize        region of interest of source image
//    pDst              resultant image data
//    dstStep           step in destination image
//    dstRoiSize        region of interest of destination image
//    pState            pointer to filter state
//  Return:
//    ippStsNoErr       no errors
//    ippStsNullPtrErr  pSrc == NULL or pDst == NULL or pState == NULL
//    ippStsStepErr     srcStep or dstStep is less than or equal to zero
//    ippStsSizeErr     width or height of images is less or equal to zero
//    ippStsContextMatchErr invalid context structure
*/

IPPAPI(IppStatus, ippiResizeFilter_8u_C1R, (
    const Ipp8u *pSrc, int srcStep, IppiSize srcRoiSize,
    Ipp8u *pDst, int dstStep, IppiSize dstRoiSize, IppiResizeFilterState *pState))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ppiGetAffineBound
//  Purpose:        Computes the bounding rectangle of the transformed image ROI.
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr   OK
//  Parameters:
//      srcRoi      Source image ROI.
//      coeffs      The affine transform matrix
//                  |X'|   |a11 a12| |X| |a13|
//                  |  | = |       |*| |+|   |
//                  |Y'|   |a21 a22| |Y| |a23|
//      bound       Resultant bounding rectangle
//  Notes:
*/

IPPAPI(IppStatus, ippiGetAffineBound, (IppiRect srcRoi,double bound[2][2], const double coeffs[2][3]))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiGetAffineQuad
//  Purpose:    Computes coordinates of the quadrangle to which a source ROI is mapped
//  Context:
//  Returns:        IppStatus.
//    ippStsNoErr   OK
//  Parameters:
//      srcRoi         Source image ROI.
//      coeffs      The affine transform matrix
//                  |X'|   |a11 a12| |X| |a13|
//                  |  | = |       |*| |+|   |
//                  |Y'|   |a21 a22| |Y| |a23|
//      quadr       Resultant quadrangle
//  Notes:
*/


IPPAPI(IppStatus, ippiGetAffineQuad, (IppiRect srcRoi, double quad[4][2], const double coeffs[2][3]))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiGetAffineTransform
//  Purpose:   Computes coefficients to transform a source ROI to a given quadrangle.
//  Context:
//  Returns:        IppStatus.
//    ippStsNoErr   OK
//  Parameters:
//      srcRoi      Source image ROI.
//      coeffs      The resultant affine transform matrix
//                  |X'|   |a11 a12| |X| |a13|
//                  |  | = |       |*| |+|   |
//                  |Y'|   |a21 a22| |Y| |a23|
//      quad        Vertex coordinates of the quadrangle
//  Notes: The function computes the coordinates of the 4th vertex of the quadrangle
//         that uniquely depends on the three other (specified) vertices.
//         If the computed coordinates are not equal to the ones specified in quad,
//         the function returns the warning message and continues operation with the computed values
*/

IPPAPI(IppStatus, ippiGetAffineTransform, (IppiRect srcRoi, const double quad[4][2], double coeffs[2][3]))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiWarpAffine
//  Purpose:   Performs affine transform of the source image.
//                  |X'|   |a11 a12| |X| |a13|
//                  |  | = |       |*| |+|   |
//                  |Y'|   |a21 a22| |Y| |a23|
//  Context:
//  Returns:                IppStatus
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pSrc or pDst is NULL
//    ippStsSizeErr         One of the image dimensions has zero or negative value
//    ippStsInterpolateErr  interpolation has an illegal value
//  Parameters:
//      pSrc        Pointer to the source image data (point to pixel (0,0))
//      srcSize     Size of the source image
//      srcStep     Step through the source image
//      srcRoi      Region of interest in the source image
//      pDst        Pointer to  the destination image (point to pixel (0,0))
//      dstStep     Step through the destination image
//      dstRoi      Region of interest in the destination image.
//      coeffs      The affine transform matrix
//      interpolation  The type of interpolation to perform for resampling
//                  the input image. Possible values:
//                  IPPI_INTER_NN       Nearest neighbor interpolation
//                  IPPI_INTER_LINEAR   Linear interpolation
//                  IPPI_INTER_CUBIC    Cubic convolution interpolation
//                  IPPI_SMOOTH_EDGE    smoothed edges
//  Notes:
*/

IPPAPI(IppStatus, ippiWarpAffine_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffine_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiWarpAffineBack
//  Purpose:    Performs an inverse affine transform of an image.
//                  |X'|   |a11 a12| |X| |a13|
//                  |  | = |       |*| |+|   |
//                  |Y'|   |a21 a22| |Y| |a23|
//  Context:
//  Returns:                IppStatus
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pSrc or pDst is NULL
//    ippStsSizeErr         One of the image dimensions has zero or negative value
//    ippStsInterpolateErr  interpolation has an illegal value
//  Parameters:
//      pSrc        Pointer to the source image data (point to pixel (0,0))
//      srcSize     Size of the source image
//      srcStep     Step through the source image
//      srcRoi      Region of interest in the source image
//      pDst        Pointer to  the destination image (point to pixel (0,0)).
//      dstStep     Step through the destination image
//      dstRoi      Region of interest in the destination image
//      coeffs      The affine transform matrix
//      interpolation The type of interpolation to perform for resampling
//                  the input image. Possible values:
//                  IPPI_INTER_NN       Nearest neighbor interpolation.
//                  IPPI_INTER_LINEAR   Linear interpolation.
//                  IPPI_INTER_CUBIC    Cubic convolution interpolation.
//  Notes:
*/

IPPAPI(IppStatus, ippiWarpAffineBack_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineBack_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[2][3], int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiWarpAffineQuad
//  Purpose:  Performs affine transform of the given source quadrangle
//            to the specified destination quadrangle
//                  |X'|   |a11 a12| |X| |a13|
//                  |  | = |       |*| |+|   |
//                  |Y'|   |a21 a22| |Y| |a23|
//  Context:
//  Returns:                IppStatus
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pSrc or pDst is NULL
//    ippStsSizeErr         One of the image dimensions has zero or negative value
//    ippStsStepErr         srcStep or dstStep has a zero or negative value
//    ippStsInterpolateErr  interpolation has an illegal value
//  Parameters:
//      pSrc        Pointer to the source image data (point to pixel (0,0))
//      srcSize     Size of the source image
//      srcStep     Step through the source image
//      srcRoi      Region of interest in the source image
//      srcQuad     Given quadrangle in the source image
//      pDst        Pointer to  the destination image (point to pixel (0,0)).
//      dstStep     Step through the destination image
//      dstRoi      Region of interest in the destination image.
//      dstQuad     Given quadrangle in the destination image
//      interpolation The type of interpolation to perform for resampling
//                  the input image. Possible values:
//                  IPPI_INTER_NN       Nearest neighbor interpolation.
//                  IPPI_INTER_LINEAR   Linear interpolation.
//                  IPPI_INTER_CUBIC    Cubic convolution interpolation.
//                 +IPPI_SMOOTH_EDGE    Edges smoothing in addition to one of the
//                                      above methods
//  Notes: The function computes the coordinates of the 4th vertex of the quadrangle
//         that uniquely depends on the three other (specified) vertices.
//         If the computed coordinates are not equal to the ones specified in quad,
//         the function returns the warning message and continues operation with the computed values
*/

IPPAPI(IppStatus, ippiWarpAffineQuad_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* const pDst[3], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* const pDst[4], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* const pDst[3], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* const pDst[4], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* const pDst[3], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpAffineQuad_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* const pDst[4], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiRotate
//  Purpose:  Rotates an image around (0, 0) by specified angle + shifts it.
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pSrc or pDst is NULL
//    ippStsSizeErr         One of the image dimensions has zero or negative value
//    ippStsStepErr         srcStep or dstStep has a zero or negative value
//    ippStsInterpolateErr  interpolation has an illegal value
//  Parameters:
//      pSrc          Pointer to the source image data (point to pixel (0,0))
//      srcSize       Size of the source image
//      srcStep       Step through the source image
//      srcRoi        Region of interest in the source image
//      pDst          Pointer to  the destination image (point to pixel (0,0)).
//      dstStep       Step through the destination image
//      dstRoi        Region of interest in the destination image.
//      angle         The angle of clockwise rotation in degrees
//      xShif, yShift The shift along the corresponding axis
//      interpolation The type of interpolation to perform for resampling
//                  the input image. Possible values:
//                  IPPI_INTER_NN       Nearest neighbor interpolation.
//                  IPPI_INTER_LINEAR   Linear interpolation.
//                  IPPI_INTER_CUBIC    Cubic convolution interpolation.
//                 +IPPI_SMOOTH_EDGE    Edges smoothing in addition to one of the
//                                      above methods
//  Notes:
*/

IPPAPI(IppStatus, ippiRotate_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[3], int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[4], int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[3], int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[4], int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[3], int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiRotate_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[4], int dstStep, IppiRect dstRoi, double angle, double xShift, double yShift, int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiAddRotateShift
//  Purpose:   Calculates shifts for ippiRotate function to rotate an image
//             around the specified center (xCenter, yCenter) with arbitrary shifts.
//  Context:
//  Returns:        IppStatus.
//    ippStsNoErr           OK
//    ippStsNullPtrErr      One of pointers to the output data is NULL
//  Parameters:
//    xCenter, yCenter    Coordinates of the center of rotation
//    angle               The angle of clockwise rotation, degrees
//    xShift, yShift      Pointers to the shift values
//  Notes:
*/

IPPAPI(IppStatus, ippiAddRotateShift, (double xCenter, double yCenter, double angle,
                                       double *xShift, double *yShift))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiGetRotateShift
//  Purpose:      Calculates shifts for ippiRotate function to rotate an image
//                around the specified center (xCenter, yCenter)
//  Context:
//  Returns:        IppStatus.
//    ippStsNoErr          OK
//    ippStsNullPtrErr     One of the pointers to the output data is NULL
//  Parameters:
//     xCenter, yCenter    Coordinates of the center of rotation
//     angle               The angle of clockwise rotation, degrees
//     xShift, yShift      Pointers to the shift values
//  Notes:
*/

IPPAPI(IppStatus, ippiGetRotateShift, (double xCenter, double yCenter, double angle,
                                       double *xShift, double *yShift))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiGetRotateQuad
//  Purpose:    Computes the quadrangle to which the source ROI would be mapped
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr      OK
//  Parameters:
//      srcRoi          Source image ROI.
//      angle           The angle of rotation in degrees
//      xShift, yShift  The shift along the corresponding axis
//      quad            Output array with vertex coordinates of the quadrangle

//  Notes:
*/

IPPAPI(IppStatus, ippiGetRotateQuad, (IppiRect srcRoi, double quad[4][2], double angle, double xShift, double yShift))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiGetRotateBound
//  Purpose: Computes the bounding rectangle for the transformed image ROI
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr      OK
//  Parameters:
//      srcRoi          Source image ROI.
//      angle           The angle of rotation in degrees
//      xShift, yShift  The shift along the corresponding axis
//      bound           Output array with vertex coordinates of the bounding rectangle
//  Notes:
*/

IPPAPI(IppStatus, ippiGetRotateBound, (IppiRect srcRoi, double bound[2][2], double angle, double xShift, double yShift))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiRotateCenter
//  Purpose:        Rotates an image about an arbitrary center.
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pSrc or pDst is NULL
//    ippStsSizeErr         One of the image dimensions has zero or negative value
//    ippStsStepErr         srcStep or dstStep has a zero or negative value
//    ippStsInterpolateErr  interpolation has an illegal value
//  Parameters:
//      pSrc             Pointer to the source image data (point to pixel (0,0))
//      srcSize          Size of the source image
//      srcStep          Step through the source image
//      srcRoi           Region of interest in the source image
//      pDst             Pointer to  the destination image (point to pixel (0,0)).
//      dstStep          Step through the destination image
//      dstRoi           Region of interest in the destination image.
//      angle            The angle of clockwise rotation in degrees
//      xCenter, yCenter Center of rotation coordinates
//      interpolation    The type of interpolation to perform for resampling
//                       the input image. Possible values:
//                       IPPI_INTER_NN       Nearest neighbor interpolation.
//                       IPPI_INTER_LINEAR   Linear interpolation.
//                       IPPI_INTER_CUBIC    Cubic convolution interpolation.
//                       IPPI_SMOOTH_EDGE    Edges smoothing in addition to one of the
//                                           above methods
//  Notes:
*/

IPPAPI(IppStatus, ippiRotateCenter_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[3], int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[4], int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[3], int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[4], int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[3], int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))
IPPAPI(IppStatus, ippiRotateCenter_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[4], int dstStep, IppiRect dstRoi, double angle, double xCenter, double yCenter, int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiShear
//  Purpose:  Performs shear transform of the source image
//                  |X'|   |1  Shx| |X|
//                  |  | = |      |*| |
//                  |Y'|   |Shy  1| |Y|
//  Context:
//  Returns:        IppStatus.
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pSrc or pDst is NULL
//    ippStsSizeErr         One of the image dimensions has zero or negative value
//    ippStsStepErr         srcStep or dstStep has a zero or negative value
//    ippStsInterpolateErr  interpolation has an illegal value
//  Parameters:
//      pSrc           Pointer to the source image data (point to pixel (0,0))
//      srcSize        Size of the source image
//      srcStep        Step through the source image
//      srcRoi         Region of interest in the source image
//      pDst           Pointer to  the destination image (point to pixel (0,0)).
//      dstStep        Step through the destination image
//      dstRoi         Region of interest in the destination image.
//      xShear, yShear Coefficients of the shearing transform
//      xShif, yShift  The shift along the corresponding axis
//      interpolation  The type of interpolation to perform for resampling
//                     the input image. Possible values:
//                  IPPI_INTER_NN       Nearest neighbor interpolation
//                  IPPI_INTER_LINEAR   Linear interpolation
//                  IPPI_INTER_CUBIC    Cubic convolution interpolation
//                  IPPI_SMOOTH_EDGE    Edges smoothing in addition to one of the
//                                      above methods
//  Notes:
*/

IPPAPI(IppStatus, ippiShear_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[3], int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[4], int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[3], int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[4], int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[3], int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))
IPPAPI(IppStatus, ippiShear_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[4], int dstStep, IppiRect dstRoi, double xShear, double yShear, double xShift, double yShift, int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiGetShearQuad
//  Purpose:    Computes the quadrangle to which the source ROI would be mapped
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr   OK
//  Parameters:
//      srcRoi          Source image ROI
//      xShear, yShear  The coefficients of the shear transform
//      xShift, yShift  The shift along the corresponding axis
//      quad            Output array with vertex coordinates of the quadrangle
//  Notes:
*/


IPPAPI(IppStatus, ippiGetShearQuad, (IppiRect srcRoi, double quad[4][2], double xShear, double yShear, double xShift, double yShift))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiGetShearBound
//  Purpose:  Computes the bounding rectangle for the transformed image ROI
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr   OK
//  Parameters:
//      srcRoi          Source image ROI
//      xShear, yShear  The coefficients of the shear transform
//      xShift, yShift  The shift along the corresponding axis
//      bound           Output array with vertex coordinates of the bounding rectangle
//  Notes:
*/

IPPAPI(IppStatus, ippiGetShearBound, (IppiRect srcRoi, double bound[2][2], double xShear, double yShear, double xShift, double yShift))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiGetPerspectiveBound
//  Purpose:  Computes the bounding rectangle for the transformed image ROI
//  Context:
//  Returns:        IppStatus.
//    ippStsNoErr   OK
//  Parameters:
//      srcRoi  Source image ROI.
//      coeffs  The perspective transform matrix
//                     a11*j + a12*i + a13
//                 x = -------------------
//                     a31*j + a32*i + a33
//
//                     a21*j + a22*i + a23
//                 y = -------------------
//                     a31*j + a32*i + a33
//      bound   Output array with vertex coordinates of the bounding rectangle
//  Notes:
*/

IPPAPI(IppStatus, ippiGetPerspectiveBound, (IppiRect srcRoi, double bound[2][2], const double coeffs[3][3]))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiGetPerspectiveQuad
//  Purpose:    Computes the quadrangle to which the source ROI would be mapped
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr   OK
//  Parameters:
//      srcRoi    Source image ROI
//      coeffs    The perspective transform matrix
//                     a11*j + a12*i + a13
//                 x = -------------------
//                     a31*j + a32*i + a33
//
//                     a21*j + a22*i + a23
//                 y = -------------------
//                     a31*j + a32*i + a33
//      quadr     Output array with vertex coordinates of the quadrangle
//  Notes:
*/

IPPAPI(IppStatus, ippiGetPerspectiveQuad, (IppiRect srcRoi, double quad[4][2], const double coeffs[3][3]))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiGetPerspectiveTransform
//  Purpose:  Computes perspective transform matrix to transform the source ROI
//            to the given quadrangle
//  Context:
//  Returns:        IppStatus.
//    ippStsNoErr   OK
//  Parameters:
//      srcRoi   Source image ROI.
//      coeffs   The resultant perspective transform matrix
//                     a11*j + a12*i + a13
//                 x = -------------------
//                     a31*j + a32*i + a33
//
//                     a21*j + a22*i + a23
//                 y = -------------------
//                     a31*j + a32*i + a33
//      quad     Vertex coordinates of the quadrangle
//  Notes:
*/

IPPAPI(IppStatus, ippiGetPerspectiveTransform, (IppiRect srcRoi, const double quad[4][2], double coeffs[3][3]))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiWarpPerspective
//  Purpose:   Performs perspective warping of an image
//                     a11*j + a12*i + a13
//                 x = -------------------
//                     a31*j + a32*i + a33
//
//                     a21*j + a22*i + a23
//                 y = -------------------
//                     a31*j + a32*i + a33
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pSrc or pDst is NULL
//    ippStsSizeErr         One of the image dimensions has zero or negative value
//    ippStsStepErr         srcStep or dstStep has a zero or negative value
//    ippStsInterpolateErr  interpolation has an illegal value
//  Parameters:
//      pSrc        Pointer to the source image data (point to pixel (0,0))
//      srcSize     Size of the source image
//      srcStep     Step through the source image
//      srcRoi      Region of interest in the source image
//      pDst        Pointer to  the destination image (point to pixel (0,0))
//      dstStep     Step through the destination image
//      dstRoi      Region of interest in the destination image
//      coeffs      The perspective transform matrix
//      interpolation  The type of interpolation to perform for resampling
//                  the input image. Possible values:
//                  IPPI_INTER_NN       Nearest neighbor interpolation
//                  IPPI_INTER_LINEAR   Linear interpolation
//                  IPPI_INTER_CUBIC    Cubic convolution interpolation
//                  IPPI_SMOOTH_EDGE    Edges smoothing in addition to one of the
//                                      above methods
//  Notes:
*/

IPPAPI(IppStatus, ippiWarpPerspective_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspective_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiWarpPerspectiveBack
//  Purpose:   Performs an inverse perspective warping of an image
//                     a11*j + a12*i + a13
//                 x = -------------------
//                     a31*j + a32*i + a33
//
//                     a21*j + a22*i + a23
//                 y = -------------------
//                     a31*j + a32*i + a33
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pSrc or pDst is NULL
//    ippStsSizeErr         One of the image dimensions has zero or negative value
//    ippStsStepErr         srcStep or dstStep has a zero or negative value
//    ippStsInterpolateErr  interpolation has an illegal value
//  Parameters:
//      pSrc        Pointer to the source image data (point to pixel (0,0))
//      srcSize     Size of the source image
//      srcStep     Step through the source image
//      srcRoi      Region of interest in the source image
//      pDst        Pointer to  the destination image (point to pixel (0,0))
//      dstStep     Step through the destination image
//      dstRoi      Region of interest in the destination image
//      coeffs      The perspective transform matrix
//      interpolation  The type of interpolation to perform for resampling
//                  the input image. Possible values:
//                  IPPI_INTER_NN       Nearest neighbor interpolation
//                  IPPI_INTER_LINEAR   Linear interpolation
//                  IPPI_INTER_CUBIC    Cubic convolution interpolation
//  Notes:
*/

IPPAPI(IppStatus, ippiWarpPerspectiveBack_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveBack_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[3][3], int interpolation))


/* ///////////////////////////////////////////////////////////////////////////////
//  Name:           ippiWarpPerspectiveQuad
//  Purpose:  Performs perspective warping of an arbitrary quadrangle in the source
//            image to the quadrangle in the destination image
//                     a11*j + a12*i + a13
//                 x = -------------------
//                     a31*j + a32*i + a33
//
//                     a21*j + a22*i + a23
//                 y = -------------------
//                     a31*j + a32*i + a33
//  Context:
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pSrc or pDst is NULL
//    ippStsSizeErr         One of the image dimensions has zero or negative value
//    ippStsStepErr         srcStep or dstStep has a zero or negative value
//    ippStsInterpolateErr  interpolation has an illegal value
//  Parameters:
//      pSrc        Pointer to the source image data (point to the pixel (0,0))
//      srcSize     Size of the source image
//      srcStep     Step through the source image
//      srcRoi      Region of interest in the source image
//      srcQuad     Vertex coordinates of a given quadrangle in the source image
//      pDst        Pointer to  the destination image (point to the pixel (0,0))
//      dstStep     Step through the destination image
//      dstRoi      Region of interest in the destination image
//      dstQuad     Vertex coordinates of the given quadrangle in the destination image
//      interpolation  The type of interpolation to perform for resampling
//                  the input image. Possible values:
//                  IPPI_INTER_NN       Nearest neighbor interpolation
//                  IPPI_INTER_LINEAR   Linear interpolation
//                  IPPI_INTER_CUBIC    Cubic convolution interpolation
//  Notes:
*/

IPPAPI(IppStatus, ippiWarpPerspectiveQuad_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* const pDst[3], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* const pDst[4], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* const pDst[3], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* const pDst[4], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* const pDst[3], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpPerspectiveQuad_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* const pDst[4], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiGetBilinearBound
//  Purpose:  Computes the bounding rectangle for the transformed image ROI
//  Context:
//  Returns:        IppStatus.
//    ippStsNoErr   OK
//  Parameters:
//      srcRoi  Source image ROI.
//      coeffs  The bilinear transform matrix
//                  |X|   |a11|      |a12 a13| |J|   |a14|
//                  | | = |   |*JI + |       |*| | + |   |
//                  |Y|   |a21|      |a22 a23| |I|   |a24|
//      bound   Output array with vertex coordinates of the bounding rectangle
//  Notes:
*/

IPPAPI(IppStatus, ippiGetBilinearBound, (IppiRect srcRoi, double bound[2][2], const double coeffs[2][4]))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiGetBilinearQuad
//  Purpose:   Computes the quadrangle to which the source ROI would be mapped
//  Context:
//  Returns:        IppStatus.
//    ippStsNoErr   OK
//  Parameters:
//      srcRoi   Source image ROI.
//      coeffs   The bilinear transform matrix
//                  |X|   |a11|      |a12 a13| |J|   |a14|
//                  | | = |   |*JI + |       |*| | + |   |
//                  |Y|   |a21|      |a22 a23| |I|   |a24|
//      quadr    Output array with vertex coordinates of the quadrangle
//  Notes:
*/

IPPAPI(IppStatus, ippiGetBilinearQuad, (IppiRect srcRoi, double quad[4][2], const double coeffs[2][4]))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiGetBilinearTransform
//  Purpose:  Computes bilinear transform matrix to transform the source ROI
//            to the given quadrangle
//  Context:
//  Returns:        IppStatus.
//    ippStsNoErr        OK
//  Parameters:
//      srcRoi         Source image ROI.
//      coeffs      The resultant bilinear transform matrix
//                  |X|   |a11|      |a12 a13| |J|   |a14|
//                  | | = |   |*JI + |       |*| | + |   |
//                  |Y|   |a21|      |a22 a23| |I|   |a24|
//      quad        Vertex coordinates of the quadrangle
//  Notes:
*/

IPPAPI(IppStatus, ippiGetBilinearTransform, (IppiRect srcRoi,const double quad[4][2], double coeffs[2][4]))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiWarpBilinear
//  Purpose:  Performs bilinear warping of an image
//                  |X|   |a11|      |a12 a13| |J|   |a14|
//                  | | = |   |*JI + |       |*| | + |   |
//                  |Y|   |a21|      |a22 a23| |I|   |a24|
//  Context:
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pSrc or pDst is NULL
//    ippStsSizeErr         One of the image dimensions has zero or negative value
//    ippStsStepErr         srcStep or dstStep has a zero or negative value
//    ippStsInterpolateErr  interpolation has an illegal value
//  Parameters:
//      pSrc        Pointer to the source image data (point to pixel (0,0))
//      srcSize     Size of the source image
//      srcStep     Step through the source image
//      srcRoi      Region of interest in the source image
//      pDst        Pointer to  the destination image (point to pixel (0,0))
//      dstStep     Step through the destination image
//      dstRoi      Region of interest in the destination image
//      coeffs      The bilinear transform matrix
//      interpolation  The type of interpolation to perform for resampling
//                  the input image. Possible values:
//                  IPPI_INTER_NN       Nearest neighbor interpolation
//                  IPPI_INTER_LINEAR   Linear interpolation
//                  IPPI_INTER_CUBIC    Cubic convolution interpolation
//                  IPPI_SMOOTH_EDGE    Edges smoothing in addition to one of the
//                                      above methods
//  Notes:
*/

IPPAPI(IppStatus, ippiWarpBilinear_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinear_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiWarpBilinearBack
//  Purpose:  Performs an inverse bilinear warping of an image
//                  |X|   |a11|      |a12 a13| |J|   |a14|
//                  | | = |   |*JI + |       |*| | + |   |
//                  |Y|   |a21|      |a22 a23| |I|   |a24|
//  Context:
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pSrc or pDst is NULL
//    ippStsSizeErr         One of the image dimensions has zero or negative value
//    ippStsStepErr         srcStep or dstStep has a zero or negative value
//    ippStsInterpolateErr  interpolation has an illegal value
//  Parameters:
//      pSrc        Pointer to the source image data (point to pixel (0,0))
//      srcSize     Size of the source image
//      srcStep     Step through the source image
//      srcRoi      Region of interest in the source image
//      pDst        Pointer to  the destination image (point to pixel (0,0))
//      dstStep     Step through the destination image
//      dstRoi      Region of interest in the destination image
//      coeffs      The bilinear transform matrix
//      interpolation  The type of interpolation to perform for resampling
//                     the input image. Possible values:
//                  IPPI_INTER_NN       Nearest neighbor interpolation
//                  IPPI_INTER_LINEAR   Linear interpolation
//                  IPPI_INTER_CUBIC    Cubic convolution interpolation
//  Notes:
*/

IPPAPI(IppStatus, ippiWarpBilinearBack_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp8u* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp32f* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[3], int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearBack_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, Ipp16u* const pDst[4], int dstStep, IppiRect dstRoi, const double coeffs[2][4], int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiWarpBilinearQuad
//  Purpose:  Performs bilinear warping of an arbitrary quadrangle in the source
//            image to the quadrangle in the destination image
//                  |X|   |a11|      |a12 a13| |J|   |a14|
//                  | | = |   |*JI + |       |*| | + |   |
//                  |Y|   |a21|      |a22 a23| |I|   |a24|
//  Context:
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pSrc or pDst is NULL
//    ippStsSizeErr         One of the image dimensions has zero or negative value
//    ippStsStepErr         srcStep or dstStep has a zero or negative value
//    ippStsInterpolateErr  interpolation has an illegal value
//  Parameters:
//      pSrc        Pointer to the source image data (point to pixel (0,0))
//      srcSize     Size of the source image
//      srcStep     Step through the source image
//      srcRoi      Region of interest in the source image
//      srcQuad     A given quadrangle in the source image
//      pDst        Pointer to  the destination image (point to pixel (0,0))
//      dstStep     Step through the destination image
//      dstRoi      Region of interest in the destination image
//      dstQuad     A given quadrangle in the destination image
//      interpolation  The type of interpolation to perform for resampling
//                  the input image. Possible values:
//                  IPPI_INTER_NN       Nearest neighbor interpolation
//                  IPPI_INTER_LINEAR   Linear interpolation
//                  IPPI_INTER_CUBIC    Cubic convolution interpolation
//                  IPPI_SMOOTH_EDGE    Edges smoothing in addition to one of the
//                                      above methods
//  Notes:
*/

IPPAPI(IppStatus, ippiWarpBilinearQuad_8u_C1R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_8u_C3R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_8u_AC4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_8u_P3R, (const Ipp8u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* const pDst[3], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_8u_C4R, (const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_8u_P4R, (const Ipp8u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp8u* const pDst[4], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_32f_C1R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_32f_C3R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_32f_AC4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_32f_P3R, (const Ipp32f* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* const pDst[3], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_32f_C4R, (const Ipp32f* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_32f_P4R, (const Ipp32f* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp32f* const pDst[4], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_16u_C1R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_16u_C3R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_16u_AC4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_16u_P3R, (const Ipp16u* const pSrc[3], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* const pDst[3], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_16u_C4R, (const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* pDst, int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))
IPPAPI(IppStatus, ippiWarpBilinearQuad_16u_P4R, (const Ipp16u* const pSrc[4], IppiSize srcSize, int srcStep, IppiRect srcRoi, const double srcQuad[4][2], Ipp16u* const pDst[4], int dstStep, IppiRect dstRoi, const double dstQuad[4][2], int interpolation))


/* /////////////////////////////////////////////////////////////////////////////
//                   Statistic functions
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )
struct MomentState64f;
struct MomentState64s;

typedef struct MomentState64f IppiMomentState_64f;
typedef struct MomentState64s IppiMomentState_64s;

typedef Ipp64f IppiHuMoment_64f[7];
typedef Ipp64s IppiHuMoment_64s[7];
#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiMomentInitAlloc()
//
//  Purpose:   Allocates memory and initializes MomentState structure
//
//  Returns:
//    ippStsMemAllocErr Memory allocation failure
//    ippStsNoErr       No errors
//
//  Parameters:
//    hint     Option to specify the computation algorithm
//    pState   Pointer to the MomentState structure
*/
IPPAPI(IppStatus, ippiMomentInitAlloc_64f, (IppiMomentState_64f** pState, IppHintAlgorithm hint))
IPPAPI(IppStatus, ippiMomentInitAlloc_64s, (IppiMomentState_64s** pState, IppHintAlgorithm hint))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiMomentFree()
//
//  Purpose:   Deallocates the MomentState structure
//
//  Returns:
//    ippStsNullPtrErr       pState==NULL
//    ippStsContextMatchErr  pState->idCtx != idCtxMoment
//    ippStsNoErr            No errors
//
//  Parameters:
//    pState   Pointer to the MomentState structure
//
*/
IPPAPI (IppStatus, ippiMomentFree_64f, (IppiMomentState_64f* pState))
IPPAPI (IppStatus, ippiMomentFree_64s, (IppiMomentState_64s* pState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiMomentGetStateSize_64s
//
//  Purpose:   Computes the size of the external buffer for the state
//             structure ippiMomentsState_64s in bytes
//
//  Returns:
//    ippStsNoErr         OK
//    ippStsNullPtrErr    pSize==NULL
//  Parameters:
//    hint                Option to specify the computation algorithm
//    pSize               Pointer to the value of the buffer size
//                        of the structure ippiMomentState_64s.
*/
IPPAPI(IppStatus,  ippiMomentGetStateSize_64s,
                   (IppHintAlgorithm hint, int * pSize))


/* ////////////////////////////////////////////////////////////////////////////////////
//  Name:           ippiMomentInit64s
//
//  Purpose:        Initializes ippiMomentState_64s structure (without memory allocation)
//
//  Returns:
//    ippStsNoErr   No errors
//
//  Parameters:
//    pState        Pointer to the MomentState structure
//    hint          Option to specify the computation algorithm
*/
IPPAPI (IppStatus, ippiMomentInit_64s,
                   (IppiMomentState_64s* pState, IppHintAlgorithm hint))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiMoments
//
//  Purpose:   Computes statistical moments of an image
//
//  Returns:
//    ippStsContextMatchErr   pState->idCtx != idCtxMoment
//    ippStsNullPtrErr        (pSrc == NULL) or (pState == NULL)
//    ippStsStepErr           pSrcStep <0
//    ippStsSizeErr           (roiSize.width  <1) or (roiSize.height <1)
//    ippStsNoErr             No errors
//
//  Parameters:
//    pSrc     Pointer to the source image
//    srcStep  Step in bytes through the source image
//    roiSize  Size of the source ROI
//    pState   Pointer to the MomentState structure
//
//  Notes:
//    These functions compute moments of order 0 to 3 only
//
*/
IPPAPI(IppStatus,ippiMoments64f_8u_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64f* pCtx))
IPPAPI(IppStatus,ippiMoments64f_8u_C3R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64f* pCtx))
IPPAPI(IppStatus,ippiMoments64f_8u_AC4R,(const Ipp8u* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64f* pCtx))

IPPAPI(IppStatus,ippiMoments64f_32f_C1R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64f* pCtx))
IPPAPI(IppStatus,ippiMoments64f_32f_C3R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64f* pCtx))
IPPAPI(IppStatus,ippiMoments64f_32f_AC4R,(const Ipp32f* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64f* pCtx))

IPPAPI(IppStatus,ippiMoments64s_8u_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64s* pCtx))
IPPAPI(IppStatus,ippiMoments64s_8u_C3R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64s* pCtx))
IPPAPI(IppStatus,ippiMoments64s_8u_AC4R,(const Ipp8u* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64s* pCtx))

IPPAPI(IppStatus,ippiMoments64f_16u_C1R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64f* pCtx))
IPPAPI(IppStatus,ippiMoments64f_16u_C3R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64f* pCtx))
IPPAPI(IppStatus,ippiMoments64f_16u_AC4R,(const Ipp16u* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64f* pCtx))

IPPAPI(IppStatus,ippiMoments64s_16u_C1R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64s* pCtx))
IPPAPI(IppStatus,ippiMoments64s_16u_C3R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64s* pCtx))
IPPAPI(IppStatus,ippiMoments64s_16u_AC4R,(const Ipp16u* pSrc, int srcStep, IppiSize roiSize, IppiMomentState_64s* pCtx))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiGetSpatialMoment()
//             ippiGetCentralMoment()
//
//  Purpose:   Retrieves the value of the image spatial/central moment.
//
//  Returns:
//    ippStsNullPtrErr      (pState == NULL) or (pValue == NULL)
//    ippStsContextMatchErr pState->idCtx != idCtxMoment
//    ippStsSizeErr         (mOrd+nOrd) >3 or
//                          (nChannel<0) or (nChannel>=pState->nChannelInUse)
//    ippStsNoErr           No errors
//
//  Parameters:
//    pState      Pointer to the MomentState structure
//    mOrd        m- Order (X direction)
//    nOrd        n- Order (Y direction)
//    nChannel    Channel number
//    roiOffset   Offset of the ROI origin (ippiGetSpatialMoment ONLY!)
//    pValue      Pointer to the retrieved moment value
//    scaleFactor Factor to scale the moment value (for integer data)
//
//  NOTE:
//    ippiGetSpatialMoment uses Absolute Coordinates (left-top image has 0,0).
//
*/
IPPAPI(IppStatus,ippiGetSpatialMoment_64f,(const IppiMomentState_64f* pState,
                                       int mOrd, int nOrd, int nChannel,
                                       IppiPoint roiOffset, Ipp64f* pValue))
IPPAPI(IppStatus,ippiGetCentralMoment_64f,(const IppiMomentState_64f* pState,
                                       int mOrd, int nOrd, int nChannel,
                                       Ipp64f* pValue))

IPPAPI(IppStatus,ippiGetSpatialMoment_64s,(const IppiMomentState_64s* pState,
                                       int mOrd, int nOrd, int nChannel,
                                       IppiPoint roiOffset, Ipp64s* pValue, int scaleFactor))
IPPAPI(IppStatus,ippiGetCentralMoment_64s,(const IppiMomentState_64s* pState,
                                       int mOrd, int nOrd, int nChannel,
                                       Ipp64s* pValue, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiGetNormalizedSpatialMoment()
//             ippiGetNormalizedCentralMoment()
//
//  Purpose:   Retrieves the normalized value of the image spatial/central moment.
//
//  Returns:
//    ippStsNullPtrErr      (pState == NULL) or (pValue == NULL)
//    ippStsContextMatchErr pState->idCtx != idCtxMoment
//    ippStsSizeErr         (mOrd+nOrd) >3 or
//                          (nChannel<0) or (nChannel>=pState->nChannelInUse)
//    ippStsMoment00ZeroErr mm[0][0] < IPP_EPS52
//    ippStsNoErr           No errors
//
//  Parameters:
//    pState      Pointer to the MomentState structure
//    mOrd        m- Order (X direction)
//    nOrd        n- Order (Y direction)
//    nChannel    Channel number
//    roiOffset   Offset of the ROI origin (ippiGetSpatialMoment ONLY!)
//    pValue      Pointer to the normalized moment value
//    scaleFactor Factor to scale the moment value (for integer data)
//
*/
IPPAPI(IppStatus,ippiGetNormalizedSpatialMoment_64f,(const IppiMomentState_64f* pState,
                                   int mOrd, int nOrd, int nChannel,
                                   IppiPoint roiOffset, Ipp64f* pValue))
IPPAPI(IppStatus,ippiGetNormalizedCentralMoment_64f,(const IppiMomentState_64f* pState,
                                   int mOrd, int nOrd, int nChannel,
                                   Ipp64f* pValue))

IPPAPI(IppStatus,ippiGetNormalizedSpatialMoment_64s,(const IppiMomentState_64s* pState,
                                   int mOrd, int nOrd, int nChannel,
                                   IppiPoint roiOffset, Ipp64s* pValue, int scaleFactor))
IPPAPI(IppStatus,ippiGetNormalizedCentralMoment_64s,(const IppiMomentState_64s* pState,
                                   int mOrd, int nOrd, int nChannel,
                                   Ipp64s* pValue, int scaleFactor))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiGetHuMoments()
//
//  Purpose:   Retrieves image Hu moment invariants.
//
//  Returns:
//    ippStsNullPtrErr      (pState == NULL) or (pHu == NULL)
//    ippStsContextMatchErr pState->idCtx != idCtxMoment
//    ippStsSizeErr         (nChannel<0) or (nChannel>=pState->nChannelInUse)
//    ippStsMoment00ZeroErr mm[0][0] < IPP_EPS52
//    ippStsNoErr           No errors
//
//  Parameters:
//    pState      Pointer to the MomentState structure
//    nChannel    Channel number
//    pHm         Pointer to the array of the Hu moment invariants
//    scaleFactor Factor to scale the moment value (for integer data)
//
//  Notes:
//    We consider Hu moments up to the 7-th order only
*/
IPPAPI(IppStatus,ippiGetHuMoments_64f,(const IppiMomentState_64f* pState,
                                   int nChannel, IppiHuMoment_64f pHm))
IPPAPI(IppStatus,ippiGetHuMoments_64s,(const IppiMomentState_64s* pState,
                                   int nChannel, IppiHuMoment_64s pHm, int scaleFactor))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiNorm_Inf
//  Purpose:        computes the C-norm of pixel values of the image: n = MAX |src1|
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value
//  Parameters:
//    pSrc        Pointer to the source image.
//    srcStep     Step through the source image
//    roiSize     Size of the source ROI.
//    pValue      Pointer to the computed norm (one-channel data)
//    value       Array of the computed norms for each channel (multi-channel data)
//  Notes:
*/

IPPAPI(IppStatus, ippiNorm_Inf_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNorm_Inf_8u_C3R, (const Ipp8u* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_Inf_8u_AC4R, (const Ipp8u* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_Inf_8u_C4R, (const Ipp8u* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNorm_Inf_16s_C1R, (const Ipp16s* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNorm_Inf_16s_C3R, (const Ipp16s* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_Inf_16s_AC4R, (const Ipp16s* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_Inf_16s_C4R, (const Ipp16s* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNorm_Inf_16u_C1R, (const Ipp16u* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNorm_Inf_16u_C3R, (const Ipp16u* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_Inf_16u_AC4R, (const Ipp16u* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_Inf_16u_C4R, (const Ipp16u* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNorm_Inf_32s_C1R, (const Ipp32s* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f* value))

IPPAPI(IppStatus, ippiNorm_Inf_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNorm_Inf_32f_C3R, (const Ipp32f* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_Inf_32f_AC4R, (const Ipp32f* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_Inf_32f_C4R, (const Ipp32f* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f value[4]))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiNorm_L1
//  Purpose:        computes the L1-norm of pixel values of the image: n = SUM |src1|
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value
//  Parameters:
//    pSrc        Pointer to the source image.
//    srcStep     Step through the source image
//    roiSize     Size of the source ROI.
//    pValue      Pointer to the computed norm (one-channel data)
//    value       Array of the computed norms for each channel (multi-channel data)
//    hint        Option to specify the computation algorithm
//  Notes:
*/

IPPAPI(IppStatus, ippiNorm_L1_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNorm_L1_8u_C3R, (const Ipp8u* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_L1_8u_AC4R, (const Ipp8u* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_L1_8u_C4R, (const Ipp8u* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNorm_L1_16s_C1R, (const Ipp16s* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNorm_L1_16s_C3R, (const Ipp16s* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_L1_16s_AC4R, (const Ipp16s* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_L1_16s_C4R, (const Ipp16s* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNorm_L1_16u_C1R, (const Ipp16u* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNorm_L1_16u_C3R, (const Ipp16u* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_L1_16u_AC4R, (const Ipp16u* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_L1_16u_C4R, (const Ipp16u* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNorm_L1_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f* pValue, IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNorm_L1_32f_C3R, (const Ipp32f* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f value[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNorm_L1_32f_AC4R, (const Ipp32f* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNorm_L1_32f_C4R, (const Ipp32f* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[4], IppHintAlgorithm hint))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiNorm_L2
//  Purpose:        computes the L2-norm of pixel values of the image: n = SQRT(SUM |src1|^2)
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value
//  Parameters:
//    pSrc        Pointer to the source image.
//    srcStep     Step through the source image
//    roiSize     Size of the source ROI.
//    pValue      Pointer to the computed norm (one-channel data)
//    value       Array of the computed norms for each channel (multi-channel data)
//    hint        Option to specify the computation algorithm
//  Notes:
//    simple mul is better than table for P6 family
*/

IPPAPI(IppStatus, ippiNorm_L2_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNorm_L2_8u_C3R, (const Ipp8u* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_L2_8u_AC4R, (const Ipp8u* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_L2_8u_C4R, (const Ipp8u* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNorm_L2_16s_C1R, (const Ipp16s* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNorm_L2_16s_C3R, (const Ipp16s* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_L2_16s_AC4R, (const Ipp16s* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_L2_16s_C4R, (const Ipp16s* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNorm_L2_16u_C1R, (const Ipp16u* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNorm_L2_16u_C3R, (const Ipp16u* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_L2_16u_AC4R, (const Ipp16u* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNorm_L2_16u_C4R, (const Ipp16u* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNorm_L2_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f* pValue, IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNorm_L2_32f_C3R, (const Ipp32f* pSrc, int srcStep,
                                      IppiSize roiSize, Ipp64f value[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNorm_L2_32f_AC4R, (const Ipp32f* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNorm_L2_32f_C4R, (const Ipp32f* pSrc, int srcStep,
                                       IppiSize roiSize, Ipp64f value[4], IppHintAlgorithm hint))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiNormDiff_Inf
//  Purpose:        computes the C-norm of pixel values of two images: n = MAX |src1 - src2|
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr         OK
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsSizeErr       roiSize has a field with zero or negative value
//  Parameters:
//    pSrc1, pSrc2        Pointers to the source images.
//    src1Step, src2Step  Steps in bytes through the source images
//    roiSize             Size of the source ROI.
//    pValue              Pointer to the computed norm (one-channel data)
//    value               Array of the computed norms for each channel (multi-channel data)
//  Notes:
*/

IPPAPI(IppStatus, ippiNormDiff_Inf_8u_C1R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormDiff_Inf_8u_C3R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_Inf_8u_AC4R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_Inf_8u_C4R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormDiff_Inf_16s_C1R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormDiff_Inf_16s_C3R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_Inf_16s_AC4R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_Inf_16s_C4R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormDiff_Inf_16u_C1R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormDiff_Inf_16u_C3R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_Inf_16u_AC4R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_Inf_16u_C4R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormDiff_Inf_32f_C1R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormDiff_Inf_32f_C3R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_Inf_32f_AC4R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_Inf_32f_C4R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiNormDiff_L1
//  Purpose:        computes the L1-norm of pixel values of two images: n = SUM |src1 - src2|
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr         OK
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsSizeErr       roiSize has a field with zero or negative value
//  Parameters:
//    pSrc1, pSrc2        Pointers to the source images.
//    src1Step, src2Step  Steps in bytes through the source images
//    roiSize             Size of the source ROI.
//    pValue              Pointer to the computed norm (one-channel data)
//    value               Array of the computed norms for each channel (multi-channel data)
//    hint                Option to specify the computation algorithm
//  Notes:
*/

IPPAPI(IppStatus, ippiNormDiff_L1_8u_C1R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormDiff_L1_8u_C3R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_L1_8u_AC4R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_L1_8u_C4R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormDiff_L1_16s_C1R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormDiff_L1_16s_C3R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_L1_16s_AC4R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_L1_16s_C4R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormDiff_L1_16u_C1R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormDiff_L1_16u_C3R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_L1_16u_AC4R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_L1_16u_C4R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormDiff_L1_32f_C1R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue, IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNormDiff_L1_32f_C3R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNormDiff_L1_32f_AC4R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNormDiff_L1_32f_C4R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4], IppHintAlgorithm hint))

/* /////////////////////////////////////////////////////////////////////////////////
//  Name:           ippiNormDiff_L2
//  Purpose:        computes the L2-norm of pixel values of two images:
//                    n = SQRT(SUM |src1 - src2|^2)
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr         OK
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsSizeErr       roiSize has a field with zero or negative value
//  Parameters:
//    pSrc1, pSrc2        Pointers to the source images.
//    src1Step, src2Step  Steps in bytes through the source images
//    roiSize             Size of the source ROI.
//    pValue              Pointer to the computed norm (one-channel data)
//    value               Array of the computed norms for each channel (multi-channel data)
//    hint                Option to specify the computation algorithm
//  Notes:
*/

IPPAPI(IppStatus, ippiNormDiff_L2_8u_C1R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormDiff_L2_8u_C3R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_L2_8u_AC4R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_L2_8u_C4R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormDiff_L2_16s_C1R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormDiff_L2_16s_C3R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_L2_16s_AC4R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_L2_16s_C4R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormDiff_L2_16u_C1R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormDiff_L2_16u_C3R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_L2_16u_AC4R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormDiff_L2_16u_C4R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormDiff_L2_32f_C1R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue, IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNormDiff_L2_32f_C3R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNormDiff_L2_32f_AC4R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNormDiff_L2_32f_C4R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4], IppHintAlgorithm hint))

/* //////////////////////////////////////////////////////////////////////////////////////////
//  Name:           ippiNormRel_Inf
//  Purpose:        computes the relative error for the C-norm of pixel values of two images:
//                      n = MAX |src1 - src2| / MAX |src2|
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr         OK
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsSizeErr       roiSize has a field with zero or negative value
//    ippStsDivByZero     MAX |src2| == 0
//  Parameters:
//    pSrc1, pSrc2        Pointers to the source images.
//    src1Step, src2Step  Steps in bytes through the source images
//    roiSize             Size of the source ROI.
//    pValue              Pointer to the computed norm (one-channel data)
//    value               Array of the computed norms for each channel (multi-channel data)
//  Notes:
*/

IPPAPI(IppStatus, ippiNormRel_Inf_8u_C1R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormRel_Inf_8u_C3R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_Inf_8u_AC4R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_Inf_8u_C4R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormRel_Inf_16s_C1R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormRel_Inf_16s_C3R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_Inf_16s_AC4R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_Inf_16s_C4R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormRel_Inf_16u_C1R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormRel_Inf_16u_C3R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_Inf_16u_AC4R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_Inf_16u_C4R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormRel_Inf_32f_C1R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormRel_Inf_32f_C3R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_Inf_32f_AC4R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_Inf_32f_C4R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

/* /////////////////////////////////////////////////////////////////////////////////////////
//  Name:           ippiNormRel_L1
//  Purpose:        computes the relative error for the 1-norm of pixel values of two images:
//                      n = SUM |src1 - src2| / SUM |src2|
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr         OK
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsSizeErr       roiSize has a field with zero or negative value
//    ippStsDivByZero     SUM |src2| == 0
//  Parameters:
//    pSrc1, pSrc2        Pointers to the source images.
//    src1Step, src2Step  Steps in bytes through the source images
//    roiSize             Size of the source ROI.
//    pValue              Pointer to the computed norm (one-channel data)
//    value               Array of the computed norms for each channel (multi-channel data)
//    hint                Option to specify the computation algorithm
//  Notes:
*/

IPPAPI(IppStatus, ippiNormRel_L1_8u_C1R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormRel_L1_8u_C3R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_L1_8u_AC4R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_L1_8u_C4R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormRel_L1_16s_C1R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormRel_L1_16s_C3R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_L1_16s_AC4R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_L1_16s_C4R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormRel_L1_16u_C1R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormRel_L1_16u_C3R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_L1_16u_AC4R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_L1_16u_C4R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormRel_L1_32f_C1R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue, IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNormRel_L1_32f_C3R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNormRel_L1_32f_AC4R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNormRel_L1_32f_C4R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4], IppHintAlgorithm hint))

/* //////////////////////////////////////////////////////////////////////////////////////////
//  Name:           ippiNormRel_L2
//  Purpose:        computes the relative error for the L2-norm of pixel values of two images:
//                      n = SQRT(SUM |src1 - src2|^2 / SUM |src2|^2)
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr         OK
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsSizeErr       roiSize has a field with zero or negative value
//    ippStsDivByZero     SUM |src2|^2 == 0
//  Parameters:
//    pSrc1, pSrc2        Pointers to the source images.
//    src1Step, src2Step  Steps in bytes through the source images
//    roiSize             Size of the source ROI.
//    pValue              Pointer to the computed norm (one-channel data)
//    value               Array of the computed norms for each channel (multi-channel data)
//    hint                Option to specify the computation algorithm
//  Notes:
*/

IPPAPI(IppStatus, ippiNormRel_L2_8u_C1R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormRel_L2_8u_C3R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_L2_8u_AC4R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_L2_8u_C4R, (const Ipp8u* pSrc1, int src1Step,
                                        const Ipp8u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormRel_L2_16s_C1R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormRel_L2_16s_C3R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_L2_16s_AC4R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_L2_16s_C4R, (const Ipp16s* pSrc1, int src1Step,
                                        const Ipp16s* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormRel_L2_16u_C1R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue))

IPPAPI(IppStatus, ippiNormRel_L2_16u_C3R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_L2_16u_AC4R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3]))

IPPAPI(IppStatus, ippiNormRel_L2_16u_C4R, (const Ipp16u* pSrc1, int src1Step,
                                        const Ipp16u* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4]))

IPPAPI(IppStatus, ippiNormRel_L2_32f_C1R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f* pValue, IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNormRel_L2_32f_C3R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNormRel_L2_32f_AC4R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiNormRel_L2_32f_C4R, (const Ipp32f* pSrc1, int src1Step,
                                        const Ipp32f* pSrc2, int src2Step,
                                        IppiSize roiSize, Ipp64f value[4], IppHintAlgorithm hint))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiSum
//  Purpose:        computes the sum of image pixel values
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value
//  Parameters:
//    pSrc        Pointer to the source image.
//    srcStep     Step in bytes through the source image
//    roiSize     Size of the source image ROI.
//    pSum        Pointer to the result (one-channel data)
//    sum         Array containing the results (multi-channel data)
//    hint        Option to select the algorithmic implementation of the function
//  Notes:
*/

IPPAPI(IppStatus, ippiSum_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                                   IppiSize roiSize, Ipp64f* pSum))

IPPAPI(IppStatus, ippiSum_8u_C3R, (const Ipp8u* pSrc, int srcStep,
                                   IppiSize roiSize, Ipp64f sum[3]))

IPPAPI(IppStatus, ippiSum_8u_AC4R, (const Ipp8u* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f sum[3]))

IPPAPI(IppStatus, ippiSum_8u_C4R, (const Ipp8u* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f sum[4]))

IPPAPI(IppStatus, ippiSum_16s_C1R, (const Ipp16s* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f* pSum))

IPPAPI(IppStatus, ippiSum_16s_C3R, (const Ipp16s* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f sum[3]))

IPPAPI(IppStatus, ippiSum_16s_AC4R, (const Ipp16s* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f sum[3]))

IPPAPI(IppStatus, ippiSum_16s_C4R, (const Ipp16s* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f sum[4]))

IPPAPI(IppStatus, ippiSum_16u_C1R, (const Ipp16u* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f* pSum))

IPPAPI(IppStatus, ippiSum_16u_C3R, (const Ipp16u* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f sum[3]))

IPPAPI(IppStatus, ippiSum_16u_AC4R, (const Ipp16u* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f sum[3]))

IPPAPI(IppStatus, ippiSum_16u_C4R, (const Ipp16u* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f sum[4]))

IPPAPI(IppStatus, ippiSum_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f* pSum, IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiSum_32f_C3R, (const Ipp32f* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f sum[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiSum_32f_AC4R, (const Ipp32f* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f sum[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiSum_32f_C4R, (const Ipp32f* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f sum[4], IppHintAlgorithm hint))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiMean
//  Purpose:        computes the mean of image pixel values
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value.
//  Parameters:
//    pSrc        Pointer to the source image.
//    srcStep     Step in bytes through the source image
//    roiSize     Size of the source ROI.
//    pMean       Pointer to the result (one-channel data)
//    mean        Array containing the results (multi-channel data)
//    hint        Option to select the algorithmic implementation of the function
//  Notes:
*/

IPPAPI(IppStatus, ippiMean_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                                   IppiSize roiSize, Ipp64f* pMean))

IPPAPI(IppStatus, ippiMean_8u_C3R, (const Ipp8u* pSrc, int srcStep,
                                   IppiSize roiSize, Ipp64f mean[3]))

IPPAPI(IppStatus, ippiMean_8u_AC4R, (const Ipp8u* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f mean[3]))

IPPAPI(IppStatus, ippiMean_8u_C4R, (const Ipp8u* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f mean[4]))

IPPAPI(IppStatus, ippiMean_16s_C1R, (const Ipp16s* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f* pMean))

IPPAPI(IppStatus, ippiMean_16s_C3R, (const Ipp16s* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f mean[3]))

IPPAPI(IppStatus, ippiMean_16s_AC4R, (const Ipp16s* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f mean[3]))

IPPAPI(IppStatus, ippiMean_16s_C4R, (const Ipp16s* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f mean[4]))

IPPAPI(IppStatus, ippiMean_16u_C1R, (const Ipp16u* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f* pMean))

IPPAPI(IppStatus, ippiMean_16u_C3R, (const Ipp16u* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f mean[3]))

IPPAPI(IppStatus, ippiMean_16u_AC4R, (const Ipp16u* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f mean[3]))

IPPAPI(IppStatus, ippiMean_16u_C4R, (const Ipp16u* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f mean[4]))

IPPAPI(IppStatus, ippiMean_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f* pMean, IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiMean_32f_C3R, (const Ipp32f* pSrc, int srcStep,
                                    IppiSize roiSize, Ipp64f mean[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiMean_32f_AC4R, (const Ipp32f* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f mean[3], IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiMean_32f_C4R, (const Ipp32f* pSrc, int srcStep,
                                     IppiSize roiSize, Ipp64f mean[4], IppHintAlgorithm hint))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//      ippiQualityIndex_8u_C1R,     ippiQualityIndex_32f_C1R,
//      ippiQualityIndex_8u_C3R,     ippiQualityIndex_32f_C3R,
//      ippiQualityIndex_8u_AC4R,    ippiQualityIndex_32f_AC4R.
//
//  Purpose: ippiQualityIndex() function calculates the Universal Image Quality
//           Index. Instead of traditional error summation methods, the
//           proposed index is designed by modeling any image distortion as a
//           combination of three factors: loss of correlation, luminance
//           distortion, and contrast distortion. The dynamic range of the index
//           is [-1.0, 1.0].
//
//  Parameters:
//      pSrc1          Pointer to the 1st source image ROI;
//      src1Step       Step in bytes through the 1 source image buffer;
//      pSrc2          Pointer to the 2nd source image ROI;
//      src2Step       Step in bytes through the 2nd source image buffer;
//      roiSize        Size of the 1st and 2nd source images ROI in pixels;
//      pQualityIndex  Pointer where to store the calculated Universal
//                      Image Quality Index;
//
//  Returns:
//   ippStsNoErr        OK
//   ippStsNullPtrErr   One of the pointers to pSrc1, pSrc2 or
//                                                       pQualityIndex is NULL;
//   ippStsSizeErr      roiSize has a field with zero or negative value;
//   ippStsStepErr      One of the src1Step or src2Step is less than or
//                                                               equal to zero;
//   ippStsMemAllocErr  Memory allocation for internal buffers fails.
*/
IPPAPI( IppStatus, ippiQualityIndex_8u32f_C1R,( const Ipp8u* pSrc1,
              int src1Step, const Ipp8u* pSrc2, int src2Step, IppiSize roiSize,
                                                       Ipp32f* pQualityIndex ))
IPPAPI( IppStatus, ippiQualityIndex_8u32f_C3R,( const Ipp8u* pSrc1,
              int src1Step, const Ipp8u* pSrc2, int src2Step, IppiSize roiSize,
                                                     Ipp32f pQualityIndex[3] ))
IPPAPI( IppStatus, ippiQualityIndex_8u32f_AC4R,( const Ipp8u* pSrc1,
              int src1Step, const Ipp8u* pSrc2, int src2Step, IppiSize roiSize,
                                                     Ipp32f pQualityIndex[3] ))
IPPAPI( IppStatus, ippiQualityIndex_32f_C1R,( const Ipp32f* pSrc1,
             int src1Step, const Ipp32f* pSrc2, int src2Step, IppiSize roiSize,
                                                       Ipp32f* pQualityIndex ))
IPPAPI( IppStatus, ippiQualityIndex_32f_C3R,( const Ipp32f* pSrc1,
             int src1Step, const Ipp32f* pSrc2, int src2Step, IppiSize roiSize,
                                                     Ipp32f pQualityIndex[3] ))
IPPAPI( IppStatus, ippiQualityIndex_32f_AC4R,( const Ipp32f* pSrc1,
             int src1Step, const Ipp32f* pSrc2, int src2Step, IppiSize roiSize,
                                                     Ipp32f pQualityIndex[3] ))

IPPAPI( IppStatus, ippiQualityIndex_16u32f_C1R,( const Ipp16u* pSrc1,
              int src1Step, const Ipp16u* pSrc2, int src2Step, IppiSize roiSize,
                                                       Ipp32f* pQualityIndex ))
IPPAPI( IppStatus, ippiQualityIndex_16u32f_C3R,( const Ipp16u* pSrc1,
              int src1Step, const Ipp16u* pSrc2, int src2Step, IppiSize roiSize,
                                                     Ipp32f pQualityIndex[3] ))
IPPAPI( IppStatus, ippiQualityIndex_16u32f_AC4R,( const Ipp16u* pSrc1,
              int src1Step, const Ipp16u* pSrc2, int src2Step, IppiSize roiSize,
                                                     Ipp32f pQualityIndex[3] ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiHistogramRange
//  Purpose:        computes the intensity histogram of an image
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr           OK
//    ippStsNullPtrErr      One of the pointers is NULL
//    ippStsSizeErr         roiSize has a field with zero or negative value
//    ippStsMemAllocErr     There is not enough memory for the inner histogram
//    ippStsHistoNofLevelsErr  Number of levels is less than 2
//  Parameters:
//    pSrc        Pointer to the source image.
//    srcStep     Step in bytes through the source image
//    roiSize     Size of the source ROI.
//    pHist       Pointer to the computed histogram.
//    pLevels     Pointer to the array of level values.
//    nLevels     Number of levels
//  Notes:
*/
IPPAPI(IppStatus, ippiHistogramRange_8u_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiHistogramRange_8u_C3R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiHistogramRange_8u_AC4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiHistogramRange_8u_C4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[4],  const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiHistogramRange_16s_C1R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiHistogramRange_16s_C3R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3],const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiHistogramRange_16s_AC4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3],const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiHistogramRange_16s_C4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[4],const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiHistogramRange_16u_C1R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiHistogramRange_16u_C3R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3],const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiHistogramRange_16u_AC4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3],const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiHistogramRange_16u_C4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[4],const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiHistogramRange_32f_C1R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist, const Ipp32f* pLevels, int nLevels))
IPPAPI(IppStatus, ippiHistogramRange_32f_C3R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3],  const Ipp32f* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiHistogramRange_32f_AC4R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3], const Ipp32f* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiHistogramRange_32f_C4R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[4],  const Ipp32f* pLevels[4], int nLevels[4]))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiHistogramEven
//  Purpose:        Computes the intensity histogram of an image
//                  using equal bins - even histogram
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value
//    ippStsMemAllocErr  There is not enough memory for the inner histogram
//    ippStsHistoNofLevelsErr Number of levels is less 2
//  Parameters:
//    pSrc        Pointer to the source image.
//    srcStep     Step in bytes through the source image
//    roiSize     Size of the source ROI.
//    pHist       Pointer to the computed histogram.
//    pLevels     Pointer to the array of level values.
//    nLevels     Number of levels
//    lowerLevel  Lower level boundary
//    upperLevel  Upper level boundary
//  Notes:
*/
IPPAPI(IppStatus, ippiHistogramEven_8u_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist, Ipp32s* pLevels, int nLevels, Ipp32s lowerLevel, Ipp32s upperLevel))
IPPAPI(IppStatus, ippiHistogramEven_8u_C3R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3], Ipp32s* pLevels[3], int nLevels[3], Ipp32s lowerLevel[3], Ipp32s upperLevel[3]))
IPPAPI(IppStatus, ippiHistogramEven_8u_AC4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3], Ipp32s* pLevels[3], int nLevels[3], Ipp32s lowerLevel[3], Ipp32s upperLevel[3]))
IPPAPI(IppStatus, ippiHistogramEven_8u_C4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[4], Ipp32s* pLevels[4], int nLevels[4], Ipp32s lowerLevel[4], Ipp32s upperLevel[4]))
IPPAPI(IppStatus, ippiHistogramEven_16s_C1R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist, Ipp32s* pLevels, int nLevels, Ipp32s lowerLevel, Ipp32s upperLevel))
IPPAPI(IppStatus, ippiHistogramEven_16s_C3R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3], Ipp32s* pLevels[3], int nLevels[3], Ipp32s lowerLevel[3], Ipp32s upperLevel[3]))
IPPAPI(IppStatus, ippiHistogramEven_16s_AC4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3], Ipp32s* pLevels[3], int nLevels[3], Ipp32s lowerLevel[3], Ipp32s upperLevel[3]))
IPPAPI(IppStatus, ippiHistogramEven_16s_C4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[4], Ipp32s* pLevels[4], int nLevels[4], Ipp32s lowerLevel[4], Ipp32s upperLevel[4]))
IPPAPI(IppStatus, ippiHistogramEven_16u_C1R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist, Ipp32s* pLevels, int nLevels, Ipp32s lowerLevel, Ipp32s upperLevel))
IPPAPI(IppStatus, ippiHistogramEven_16u_C3R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3], Ipp32s* pLevels[3], int nLevels[3], Ipp32s lowerLevel[3], Ipp32s upperLevel[3]))
IPPAPI(IppStatus, ippiHistogramEven_16u_AC4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[3], Ipp32s* pLevels[3], int nLevels[3], Ipp32s lowerLevel[3], Ipp32s upperLevel[3]))
IPPAPI(IppStatus, ippiHistogramEven_16u_C4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp32s* pHist[4], Ipp32s* pLevels[4], int nLevels[4], Ipp32s lowerLevel[4], Ipp32s upperLevel[4]))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiLUT, ippiLUT_Linear, ippiLUT_Cubic
//  Purpose:        Performs intensity transformation of an image
//                  using lookup table (LUT) without interpolation or
//                  using lookup table (LUT) with linear interpolation or
//                  using lookup table (LUT) with cubic interpolation
//  Parameters:
//    pSrc          pointer to the source image
//    srcStep       step in bytes through the source image
//    pDst          pointer to the destination image
//    dstStep       step in bytes through the destination image
//    pSrcDst       pointer to the destination image (inplace case)
//    srcDstStep    step in bytes through the destination image (inplace case)
//    roiSize       size of the source and destination ROI
//    pValues       pointer to the array of intensity values
//    pLevels       pointer to the array of level values
//    nLevels       number of levels
//  Returns:
//    ippStsNoErr           no errors
//    ippStsNullPtrErr      one of the pointers is NULL
//    ippStsSizeErr         roiSize has a field with zero or negative value
//    ippStsMemAllocErr     there is not enough memory for the inner histogram
//    ippStsLUTNofLevelsErr number of levels is less 2
//  Notes:
*/

IPPAPI(IppStatus, ippiLUT_8u_C1R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_8u_C3R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_8u_C4R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_8u_AC4R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_8u_C1IR,(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_8u_C3IR,(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_8u_C4IR,(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_8u_AC4IR,(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_16s_C1R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_16s_C3R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_16s_C4R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_16s_AC4R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_16s_C1IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_16s_C3IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_16s_C4IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_16s_AC4IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_16u_C1R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_16u_C3R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_16u_C4R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_16u_AC4R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_16u_C1IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_16u_C3IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_16u_C4IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_32f_C1R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f* pValues, const Ipp32f* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_32f_C3R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f* pValues[3], const Ipp32f* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_32f_C4R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f* pValues[4], const Ipp32f* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_32f_AC4R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f* pValues[3], const Ipp32f* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32f* pValues, const Ipp32f* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_32f_C3IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32f* pValues[3], const Ipp32f* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_32f_C4IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32f* pValues[4], const Ipp32f* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32f* pValues[3], const Ipp32f* pLevels[3], int nLevels[3]))

IPPAPI(IppStatus, ippiLUT_Linear_8u_C1R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Linear_8u_C3R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_8u_C4R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Linear_8u_AC4R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_8u_C1IR,(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Linear_8u_C3IR,(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_8u_C4IR,(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Linear_8u_AC4IR,(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_16s_C1R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Linear_16s_C3R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_16s_C4R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Linear_16s_AC4R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_16s_C1IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Linear_16s_C3IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_16s_C4IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Linear_16s_AC4IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_16u_C1R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Linear_16u_C3R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_16u_C4R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Linear_16u_AC4R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_16u_C1IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Linear_16u_C3IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_16u_C4IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Linear_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_32f_C1R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f* pValues, const Ipp32f* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Linear_32f_C3R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f* pValues[3], const Ipp32f* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_32f_C4R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f* pValues[4], const Ipp32f* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Linear_32f_AC4R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f* pValues[3], const Ipp32f* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32f* pValues, const Ipp32f* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Linear_32f_C3IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32f* pValues[3], const Ipp32f* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Linear_32f_C4IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32f* pValues[4], const Ipp32f* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Linear_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32f* pValues[3], const Ipp32f* pLevels[3], int nLevels[3]))

IPPAPI(IppStatus, ippiLUT_Cubic_8u_C1R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Cubic_8u_C3R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_8u_C4R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Cubic_8u_AC4R,(const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_8u_C1IR,(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Cubic_8u_C3IR,(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_8u_C4IR,(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Cubic_8u_AC4IR,(Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_16s_C1R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Cubic_16s_C3R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_16s_C4R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Cubic_16s_AC4R,(const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_16s_C1IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Cubic_16s_C3IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_16s_C4IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Cubic_16s_AC4IR,(Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_16u_C1R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Cubic_16u_C3R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_16u_C4R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Cubic_16u_AC4R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_16u_C1IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues, const Ipp32s* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Cubic_16u_C3IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_16u_C4IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[4], const Ipp32s* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Cubic_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32s* pValues[3], const Ipp32s* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_32f_C1R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f* pValues, const Ipp32f* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Cubic_32f_C3R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f* pValues[3], const Ipp32f* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_32f_C4R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f* pValues[4], const Ipp32f* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Cubic_32f_AC4R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f* pValues[3], const Ipp32f* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32f* pValues, const Ipp32f* pLevels, int nLevels))
IPPAPI(IppStatus, ippiLUT_Cubic_32f_C3IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32f* pValues[3], const Ipp32f* pLevels[3], int nLevels[3]))
IPPAPI(IppStatus, ippiLUT_Cubic_32f_C4IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32f* pValues[4], const Ipp32f* pLevels[4], int nLevels[4]))
IPPAPI(IppStatus, ippiLUT_Cubic_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize, const Ipp32f* pValues[3], const Ipp32f* pLevels[3], int nLevels[3]))


/* ////////////////////////////////////////////////////////////////////////////
//  Names:       ippiLUTPalette
//  Purpose:     intensity transformation of image using the palette lookup table pTable
//  Parameters:
//    pSrc       pointer to the source image
//    srcStep    line offset in input data in bytes
//    alphaValue constant alpha channel
//    pDst       pointer to the destination image
//    dstStep    line offset in output data in bytes
//    roiSize    size of source ROI in pixels
//    pTable     pointer to palette table of size 2^nBitSize or
//               array of pointers to each channel
//    nBitSize   number of valid bits in the source image
//               (range [1,8] for 8u source images and range [1,16] for 16u source images)
//  Returns:
//    ippStsNoErr         no errors
//    ippStsNullPtrErr    pSrc == NULL or pDst == NULL or pTable == NULL
//    ippStsSizeErr       width or height of ROI is less or equal zero
//    ippStsOutOfRangeErr nBitSize is out of range
//  Notes:
*/
IPPAPI(IppStatus, ippiLUTPalette_16u32u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp32u* pDst, int dstStep,
       IppiSize roiSize, const Ipp32u* pTable, int nBitSize))
IPPAPI(IppStatus, ippiLUTPalette_16u24u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
       IppiSize roiSize, const Ipp8u* pTable, int nBitSize))
IPPAPI(IppStatus, ippiLUTPalette_16u8u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
       IppiSize roiSize, const Ipp8u* pTable, int nBitSize))
IPPAPI(IppStatus, ippiLUTPalette_8u32u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp32u* pDst, int dstStep,
       IppiSize roiSize, const Ipp32u* pTable, int nBitSize))
IPPAPI(IppStatus, ippiLUTPalette_8u24u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
       IppiSize roiSize, const Ipp8u* pTable, int nBitSize))
IPPAPI(IppStatus, ippiLUTPalette_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
       IppiSize roiSize, const Ipp8u* pTable, int nBitSize))
IPPAPI(IppStatus, ippiLUTPalette_16u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
       IppiSize roiSize, const Ipp16u* pTable, int nBitSize))
IPPAPI(IppStatus, ippiLUTPalette_8u_C3R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
       IppiSize roiSize, const Ipp8u* const pTable[3], int nBitSize))
IPPAPI(IppStatus, ippiLUTPalette_16u_C3R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
       IppiSize roiSize, const Ipp16u* const pTable[3], int nBitSize))
IPPAPI(IppStatus, ippiLUTPalette_8u_C4R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
       IppiSize roiSize, const Ipp8u* const pTable[4], int nBitSize))
IPPAPI(IppStatus, ippiLUTPalette_16u_C4R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
       IppiSize roiSize, const Ipp16u* const pTable[4], int nBitSize))
IPPAPI(IppStatus, ippiLUTPalette_8u_AC4R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
       IppiSize roiSize, const Ipp8u* const pTable[3], int nBitSize))
IPPAPI(IppStatus, ippiLUTPalette_16u_AC4R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
       IppiSize roiSize, const Ipp16u* const pTable[3], int nBitSize))

IPPAPI(IppStatus, ippiLUTPalette_8u_C3A0C4R, (const Ipp8u* pSrc, int srcStep, int alphaValue, Ipp8u* pDst, int dstStep,
       IppiSize roiSize, const Ipp8u* const pTable[3], int nBitSize))
IPPAPI(IppStatus, ippiLUTPalette_16u_C3A0C4R, (const Ipp16u* pSrc, int srcStep, int alphaValue, Ipp16u* pDst, int dstStep,
       IppiSize roiSize, const Ipp16u* const pTable[3], int nBitSize))
IPPAPI(IppStatus, ippiLUTPaletteSwap_8u_C3A0C4R, (const Ipp8u* pSrc, int srcStep, int alphaValue, Ipp8u* pDst, int dstStep,
       IppiSize roiSize, const Ipp8u* const pTable[3], int nBitSize))
IPPAPI(IppStatus, ippiLUTPaletteSwap_16u_C3A0C4R, (const Ipp16u* pSrc, int srcStep, int alphaValue, Ipp16u* pDst, int dstStep,
       IppiSize roiSize, const Ipp16u* const pTable[3], int nBitSize))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:                ippiCountInRange
//
//  Purpose:  Computes the number of pixels with intensity values within the given range
//
//  Returns:             IppStatus
//      ippStsNoErr       No errors
//      ippStsNullPtrErr  pSrc == NULL
//      ippStsStepErr     srcStep is less than or equal to zero
//      ippStsSizeErr     roiSize has a field with zero or negative value
//      ippStsRangeErr    lowerBound is greater than upperBound
//
//  Parameters:
//      pSrc             Pointer to the source buffer
//      roiSize          Size of the source ROI
//      srcStep          Step through the source image buffer
//      counts           Number of pixels within the given intensity range
//      lowerBound       Lower limit of the range
//      upperBound       Upper limit of the range
*/

IPPAPI(IppStatus, ippiCountInRange_8u_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize,
                                            int* counts, Ipp8u lowerBound, Ipp8u upperBound))
IPPAPI(IppStatus, ippiCountInRange_8u_C3R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize,
                                            int counts[3], Ipp8u lowerBound[3], Ipp8u upperBound[3]))
IPPAPI(IppStatus, ippiCountInRange_8u_AC4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize,
                                            int counts[3], Ipp8u lowerBound[3], Ipp8u upperBound[3]))
IPPAPI(IppStatus, ippiCountInRange_32f_C1R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize,
                                            int* counts, Ipp32f lowerBound, Ipp32f upperBound))
IPPAPI(IppStatus, ippiCountInRange_32f_C3R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize,
                                            int counts[3], Ipp32f lowerBound[3], Ipp32f upperBound[3]))
IPPAPI(IppStatus, ippiCountInRange_32f_AC4R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize,
                                            int counts[3], Ipp32f lowerBound[3], Ipp32f upperBound[3]))


/* ///////////////////////////////////////////////////////////////////////////
//             Non-linear Filters
/////////////////////////////////////////////////////////////////////////// */

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilterMedianHoriz_8u_C1R
//              ippiFilterMedianHoriz_8u_C3R
//              ippiFilterMedianHoriz_8u_AC4R
//              ippiFilterMedianHoriz_16s_C1R
//              ippiFilterMedianHoriz_16s_C3R
//              ippiFilterMedianHoriz_16s_AC4R
//              ippiFilterMedianHoriz_8u_C4R
//              ippiFilterMedianHoriz_16s_C4R
//              ippiFilterMedianHoriz_16u_C1R
//              ippiFilterMedianHoriz_16u_C3R
//              ippiFilterMedianHoriz_16u_AC4R
//              ippiFilterMedianHoriz_16u_C4R

//  Purpose:  Performs horizontal median filtering
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  pSrc or pDst is NULL
//   ippStsSizeErr     dstRoiSize has a field with zero or negative value
//   ippStsStepErr     srcStep or dstStep has zero or negative value
//   ippStsMaskSizeErr Illegal value of mask
//
//  Parameters:
//   pSrc        Pointer to the source image
//   srcStep     Step through the source image
//   pDst        Pointer to the destination image
//   dstStep     Step through the destination image
//   dstRoiSize  Size of the destination ROI
//   mask        Type of the filter mask
*/
IPPAPI(IppStatus,ippiFilterMedianHoriz_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianHoriz_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianHoriz_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianHoriz_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianHoriz_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianHoriz_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianHoriz_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianHoriz_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianHoriz_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianHoriz_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianHoriz_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianHoriz_16u_C4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilterMedianVert_8u_C1R
//              ippiFilterMedianVert_8u_C3R
//              ippiFilterMedianVert_8u_AC4R
//              ippiFilterMedianVert_16s_C1R
//              ippiFilterMedianVert_16s_C3R
//              ippiFilterMedianVert_16s_AC4R
//              ippiFilterMedianVert_8u_C4R
//              ippiFilterMedianVert_16s_C4R
//              ippiFilterMedianVert_16u_C1R
//              ippiFilterMedianVert_16u_C3R
//              ippiFilterMedianVert_16u_AC4R
//              ippiFilterMedianVert_16u_C4R
//  Purpose: Performs vertical median filtering
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  pSrc or pDst is NULL
//   ippStsSizeErr     dstRoiSize has a field with zero or negative value
//   ippStsStepErr     srcStep or dstStep has zero or negative value
//   ippStsMaskSizeErr Illegal value of mask
//
//  Parameters:
//   pSrc        Pointer to the source image
//   srcStep     Step through the source image
//   pDst        Pointer to the destination image
//   dstStep     Step through the destination image
//   dstRoiSize  Size of the destination ROI
//   mask        Type of the filter mask
*/
IPPAPI(IppStatus,ippiFilterMedianVert_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianVert_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianVert_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianVert_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianVert_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianVert_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianVert_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianVert_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianVert_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianVert_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianVert_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianVert_16u_C4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilterMedian_8u_C1R
//              ippiFilterMedian_8u_C3R
//              ippiFilterMedian_8u_AC4R
//              ippiFilterMedian_16s_C1R
//              ippiFilterMedian_16s_C3R
//              ippiFilterMedian_16s_AC4R
//              ippiFilterMedian_8u_C4R
//              ippiFilterMedian_16s_C4R
//              ippiFilterMedian_16u_C1R
//              ippiFilterMedian_16u_C3R
//              ippiFilterMedian_16u_AC4R
//              ippiFilterMedian_16u_C4R

//  Purpose:  Filters an image using a box median filter
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  pSrc or pDst is NULL
//   ippStsSizeErr     dstRoiSize has a field with zero or negative value
//   ippStsStepErr     srcStep or dstStep has zero or negative value
//   ippStsMaskSizeErr maskSize has a field with zero, negative, or even value
//   ippStsAnchorErr   anchor is outside the mask
//
//  Parameters:
//   pSrc        Pointer to the source image
//   srcStep     Step through the source image
//   pDst        Pointer to the destination image
//   dstStep     Step through the destination image
//   dstRoiSize  Size of the destination ROI
//   maskSize    Size of the mask in pixels
//   anchor      Anchor cell specifying the mask alignment with respect to
//               the position of input pixel
*/
IPPAPI(IppStatus,ippiFilterMedian_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMedian_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMedian_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMedian_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMedian_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMedian_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMedian_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMedian_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMedian_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMedian_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMedian_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMedian_16u_C4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMedian_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilterMedianCross_8u_C1R
//              ippiFilterMedianCross_8u_C3R
//              ippiFilterMedianCross_8u_AC4R
//              ippiFilterMedianCross_16s_C1R
//              ippiFilterMedianCross_16s_C3R
//              ippiFilterMedianCross_16s_AC4R
//              ippiFilterMedianCross_16u_C1R
//              ippiFilterMedianCross_16u_C3R
//              ippiFilterMedianCross_16u_AC4R
//  Purpose:  Filters an image using a cross median filter
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  pSrc or pDst is NULL
//   ippStsSizeErr     dstRoiSize has a field with zero or negative value
//   ippStsStepErr     srcStep or dstStep has zero or negative value
//   ippStsMaskSizeErr Illegal value of mask
//
//  Parameters:
//   pSrc        Pointer to the source image
//   srcStep     Step through the source image
//   pDst        Pointer to the destination image
//   dstStep     Step through the destination image
//   dstRoiSize  Size of the destination ROI
//   mask        Type of the filter mask
*/
IPPAPI(IppStatus,ippiFilterMedianCross_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianCross_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianCross_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianCross_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianCross_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianCross_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianCross_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianCross_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianCross_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilterMedianColor_8u_C3R
//              ippiFilterMedianColor_8u_AC4R
//              ippiFilterMedianColor_16s_C3R
//              ippiFilterMedianColor_16s_AC4R
//              ippiFilterMedianColor_32f_C3R
//              ippiFilterMedianColor_32f_AC4R
//  Purpose:  Filters an image using a box color median filter
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  pSrc or pDst is NULL
//   ippStsSizeErr     dstRoiSize has a field with zero or negative value
//   ippStsStepErr     srcStep or dstStep has zero or negative value
//   ippStsMaskSizeErr Illegal value of mask
//
//  Parameters:
//   pSrc        Pointer to the source image
//   srcStep     Step through the source image
//   pDst        Pointer to the destination image
//   dstStep     Step through the destination image
//   dstRoiSize  Size of the destination ROI
//   mask        Type of the filter mask
*/
IPPAPI(IppStatus,ippiFilterMedianColor_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianColor_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianColor_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianColor_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianColor_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterMedianColor_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiMaskSize mask))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilterMedianWeightedCenter3x3_8u_C1R

//  Purpose:  Filter an image using a median filter with kernel size 3x3 and
//            enlarged weight of central pixel
//  Returns:
//   ippStsNoErr            OK
//   ippStsNullPtrErr       pSrc or pDst is NULL
//   ippStsSizeErr          dstRoiSize has a field with zero or negative value
//   ippStsStepErr          srcStep or dstStep has zero or negative value
//   ippStsWeightErr        weight of central Pixel has zero or negative value
//   ippStsEvenMedianWeight weight of central Pixel has even value
//
//  Parameters:
//   pSrc        Pointer to the source image
//   srcStep     Step through the source image
//   pDst        Pointer to the destination image
//   dstStep     Step through the destination image
//   dstRoiSize  Size of the destination ROI
//   weight      Weight of central pixel
*/
IPPAPI(IppStatus,ippiFilterMedianWeightedCenter3x3_8u_C1R,(const Ipp8u* pSrc,
       int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, int weight))


/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilterMax_8u_C1R
//              ippiFilterMax_8u_C3R
//              ippiFilterMax_8u_AC4R
//              ippiFilterMax_8u_C4R
//              ippiFilterMax_16s_C1R
//              ippiFilterMax_16s_C3R
//              ippiFilterMax_16s_AC4R
//              ippiFilterMax_16s_C4R
//              ippiFilterMax_16u_C1R
//              ippiFilterMax_16u_C3R
//              ippiFilterMax_16u_AC4R
//              ippiFilterMax_16u_C4R
//              ippiFilterMax_32f_C1R
//              ippiFilterMax_32f_C3R
//              ippiFilterMax_32f_AC4R
//              ippiFilterMax_32f_C4R
//  Purpose:  Applies the "max" filter to an image
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  pSrc or pDst is NULL
//   ippStsSizeErr     dstRoiSize has a field with zero or negative value
//   ippStsStepErr     srcStep or dstStep has zero or negative value
//   ippStsMaskSizeErr maskSize has a field with zero, or negative value
//   ippStsAnchorErr   anchor is outside the mask
//
//  Parameters:
//   pSrc        Pointer to the source image
//   srcStep     Step through the source image
//   pDst        Pointer to the destination image
//   dstStep     Step through the destination image
//   pSrcDst     Pointer to the source/destination image (in-place flavors)
//   srcDstStep  Step through the source/destination image (in-place flavors)
//   dstRoiSize  Size of the destination ROI
//   roiSize     Size of the source/destination ROI (in-place flavors)
//   maskSize    Size of the mask in pixels
//   anchor      Anchor cell specifying the mask alignment with respect to
//               the position of input pixel
*/
IPPAPI(IppStatus,ippiFilterMax_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMax_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMax_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMax_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))

IPPAPI(IppStatus,ippiFilterMax_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMax_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMax_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMax_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))

IPPAPI(IppStatus,ippiFilterMax_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMax_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMax_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMax_16u_C4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))

IPPAPI(IppStatus,ippiFilterMax_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMax_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMax_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMax_32f_C4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilterMin_8u_C1R
//              ippiFilterMin_8u_C3R
//              ippiFilterMin_8u_AC4R
//              ippiFilterMin_8u_C4R
//              ippiFilterMin_16s_C1R
//              ippiFilterMin_16s_C3R
//              ippiFilterMin_16s_AC4R
//              ippiFilterMin_16s_C4R
//              ippiFilterMin_16u_C1R
//              ippiFilterMin_16u_C3R
//              ippiFilterMin_16u_AC4R
//              ippiFilterMin_16u_C4R
//              ippiFilterMin_32f_C1R
//              ippiFilterMin_32f_C3R
//              ippiFilterMin_32f_AC4R
//              ippiFilterMin_32f_C4R
//  Purpose:  Applies the "min" filter to an image
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  pSrc or pDst is NULL
//   ippStsSizeErr     dstRoiSize has a field with zero or negative value
//   ippStsStepErr     srcStep or dstStep has zero or negative value
//   ippStsMaskSizeErr maskSize has a field with zero, or negative value
//   ippStsAnchorErr   anchor is outside the mask
//
//  Parameters:
//   pSrc        Pointer to the source image
//   srcStep     Step through the source image
//   pDst        Pointer to the destination image
//   dstStep     Step through the destination image
//   dstRoiSize  Size of the destination ROI
//   maskSize    Size of the mask in pixels
//   anchor      Anchor cell specifying the mask alignment with respect to
//               the position of input pixel
*/
IPPAPI(IppStatus,ippiFilterMin_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMin_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMin_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMin_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))

IPPAPI(IppStatus,ippiFilterMin_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMin_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMin_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMin_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))

IPPAPI(IppStatus,ippiFilterMin_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMin_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMin_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMin_16u_C4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))

IPPAPI(IppStatus,ippiFilterMin_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMin_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMin_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))
IPPAPI(IppStatus,ippiFilterMin_32f_C4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
       IppiPoint anchor))


/* ///////////////////////////////////////////////////////////////////////////
//             Linear Filters
/////////////////////////////////////////////////////////////////////////// */
/* ///////////////////////////////////////////////////////////////////////////
// Name:   ippiFilterBox_8u_<desc>R,  ippiFilterBox_8u_<desc>IR
//         ippiFilterBox_16u_<desc>R, ippiFilterBox_16u_<desc>IR
//         ippiFilterBox_16s_<desc>R, ippiFilterBox_16s_<desc>IR
//         ippiFilterBox_32f_<desc>R, ippiFilterBox_32f_<desc>IR
//           <desc>  C1|C3|C4|AC4   (descriptor)
// Purpose:             Blurs an image using a simple box filter
// Parameters:
//   pSrc               pointer to the source image
//   srcStep            step in the source image
//   pDst               pointer to the destination image
//   dstStep            step in the destination image
//   pSrcDst            pointer to the source/destination image (in-place flavors)
//   srcDstStep         step in the source/destination image (in-place flavors)
//   dstRoiSize         size of the destination ROI
//   roiSize            size of the source/destination ROI (in-place flavors)
//   maskSize           size of the mask in pixels
//   anchor             the [x,y] coordinates of the anchor cell in the kernel
// Returns:
//   ippStsNoErr        No errors
//   ippStsNullPtrErr   pSrc == NULL or pDst == NULL or pSrcDst == NULL
//   ippStsStepErr      one of the step values is zero or negative
//   ippStsSizeErr      dstRoiSize or roiSize has a field with zero or negative value
//   ippStsMaskSizeErr  maskSize has a field with zero or negative value
//   ippStsAnchorErr    anchor is outside the mask
//   ippStsMemAllocErr  memory allocation error
*/

IPPAPI(IppStatus, ippiFilterBox_8u_C1R, (const Ipp8u* pSrc,int srcStep,
       Ipp8u* pDst, int dstStep,IppiSize dstRoiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_8u_C3R, (const Ipp8u* pSrc,int srcStep,
       Ipp8u* pDst, int dstStep,IppiSize dstRoiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_8u_C4R, (const Ipp8u* pSrc,int srcStep,
       Ipp8u* pDst, int dstStep,IppiSize dstRoiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_8u_AC4R, (const Ipp8u* pSrc,int srcStep,
       Ipp8u* pDst, int dstStep,IppiSize dstRoiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16u_C1R, (const Ipp16u* pSrc,int srcStep,
       Ipp16u* pDst, int dstStep,IppiSize dstRoiSize,IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16u_C3R, (const Ipp16u* pSrc,int srcStep,
       Ipp16u* pDst, int dstStep,IppiSize dstRoiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16u_C4R, (const Ipp16u* pSrc,int srcStep,
       Ipp16u* pDst, int dstStep,IppiSize dstRoiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16u_AC4R, (const Ipp16u* pSrc,int srcStep,
       Ipp16u* pDst, int dstStep,IppiSize dstRoiSize,IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16s_C1R, (const Ipp16s* pSrc,int srcStep,
       Ipp16s* pDst, int dstStep,IppiSize dstRoiSize,IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16s_C3R, (const Ipp16s* pSrc,int srcStep,
       Ipp16s* pDst, int dstStep,IppiSize dstRoiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16s_C4R, (const Ipp16s* pSrc,int srcStep,
       Ipp16s* pDst, int dstStep,IppiSize dstRoiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16s_AC4R, (const Ipp16s* pSrc,int srcStep,
       Ipp16s* pDst, int dstStep,IppiSize dstRoiSize,IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_32f_C1R, (const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep,IppiSize dstRoiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_32f_C3R, (const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep,IppiSize dstRoiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_32f_C4R, (const Ipp32f* pSrc,int srcStep,
       Ipp32f* pDst, int dstStep,IppiSize dstRoiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_32f_AC4R, (const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep,IppiSize dstRoiSize, IppiSize maskSize, IppiPoint anchor))

IPPAPI(IppStatus, ippiFilterBox_8u_C1IR, (Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_8u_C3IR, (Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_8u_C4IR, (Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize,IppiSize maskSize,IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_8u_AC4IR, (Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16u_C1IR, (Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16u_C3IR, (Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16u_C4IR, (Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize,IppiSize maskSize,IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16u_AC4IR, (Ipp16u* pSrc, int srcDstStep,
       IppiSize roiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16s_C1IR, (Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16s_C3IR, (Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16s_C4IR, (Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize,IppiSize maskSize,IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_16s_AC4IR, (Ipp16s* pSrc, int srcDstStep,
       IppiSize roiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_32f_C1IR, (Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_32f_C3IR, (Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, IppiSize maskSize, IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_32f_C4IR, (Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize,IppiSize maskSize,IppiPoint anchor))
IPPAPI(IppStatus, ippiFilterBox_32f_AC4IR, (Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, IppiSize maskSize, IppiPoint anchor))

/* ///////////////////////////////////////////////////////////////////////////
// Name:   ippiSumWindowRow_8u32f_<desc>R,  ippiSumWindowColumn_8u32f_<desc>R
//         ippiSumWindowRow_16u32f_<desc>R, ippiSumWindowColumn_16u32f_<desc>R
//         ippiSumWindowRow_16s32f_<desc>R, ippiSumWindowColumn_16s32f_<desc>R
//           <desc>  C1|C3|C4   (descriptor)
// Purpose:             Sums pixel values in the row or column mask applied to the image
// Parameters:
//   pSrc               pointer to the source image
//   srcStep            step in the source image
//   pDst               pointer to the destination image
//   dstStep            step in the destination image
//   dstRoiSize         size of the destination ROI
//   maskSize           size of the horizontal or vertical mask in pixels
//   anchor             the anchor cell
// Returns:
//   ippStsNoErr        No errors
//   ippStsNullPtrErr   pSrc == NULL or pDst == NULL or pSrcDst == NULL
//   ippStsSizeErr      dstRoiSize has a field with zero or negative value
//   ippStsMaskSizeErr  maskSize is zero or negative value
//   ippStsAnchorErr    anchor is outside the mask
//   ippStsMemAllocErr  memory allocation error (ippiSumWindowColumn only)
*/

IPPAPI(IppStatus,ippiSumWindowRow_8u32f_C1R, (const Ipp8u* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowRow_8u32f_C3R, (const Ipp8u* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowRow_8u32f_C4R, (const Ipp8u* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowRow_16u32f_C1R, (const Ipp16u* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowRow_16u32f_C3R, (const Ipp16u* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowRow_16u32f_C4R, (const Ipp16u* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowRow_16s32f_C1R, (const Ipp16s* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowRow_16s32f_C3R, (const Ipp16s* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowRow_16s32f_C4R, (const Ipp16s* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))

IPPAPI(IppStatus,ippiSumWindowColumn_8u32f_C1R, (const Ipp8u* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowColumn_8u32f_C3R, (const Ipp8u* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowColumn_8u32f_C4R, (const Ipp8u* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowColumn_16u32f_C1R, (const Ipp16u* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowColumn_16u32f_C3R, (const Ipp16u* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowColumn_16u32f_C4R, (const Ipp16u* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowColumn_16s32f_C1R, (const Ipp16s* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowColumn_16s32f_C3R, (const Ipp16s* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))
IPPAPI(IppStatus,ippiSumWindowColumn_16s32f_C4R, (const Ipp16s* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, int maskSize, int anchor))


/* ///////////////////////////////////////////////////////////////////////////
//             Filters with Fixed Kernel
/////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilterPrewittHoriz_8u_C1R
//              ippiFilterPrewittHoriz_8u_C3R
//              ippiFilterPrewittHoriz_8u_AC4R
//              ippiFilterPrewittHoriz_8u_C4R
//              ippiFilterPrewittHoriz_16s_C1R
//              ippiFilterPrewittHoriz_16s_C3R
//              ippiFilterPrewittHoriz_16s_AC4R
//              ippiFilterPrewittHoriz_16s_C4R
//              ippiFilterPrewittHoriz_32f_C1R
//              ippiFilterPrewittHoriz_32f_C3R
//              ippiFilterPrewittHoriz_32f_AC4R
//              ippiFilterPrewittHoriz_32f_C4R
//              ippiFilterPrewittVert_8u_C1R
//              ippiFilterPrewittVert_8u_C3R
//              ippiFilterPrewittVert_8u_AC4R
//              ippiFilterPrewittVert_8u_C4R
//              ippiFilterPrewittVert_16s_C1R
//              ippiFilterPrewittVert_16s_C3R
//              ippiFilterPrewittVert_16s_AC4R
//              ippiFilterPrewittVert_16s_C4R
//              ippiFilterPrewittVert_32f_C1R
//              ippiFilterPrewittVert_32f_C3R
//              ippiFilterPrewittVert_32f_AC4R
//              ippiFilterPrewittVert_32f_C4R
//              ippiFilterSobelHoriz_8u_C1R
//              ippiFilterSobelHoriz_8u_C3R
//              ippiFilterSobelHoriz_8u_AC4R
//              ippiFilterSobelHoriz_8u_C4R
//              ippiFilterSobelHoriz_16s_C1R
//              ippiFilterSobelHoriz_16s_C3R
//              ippiFilterSobelHoriz_16s_AC4R
//              ippiFilterSobelHoriz_16s_C4R
//              ippiFilterSobelHoriz_32f_C1R
//              ippiFilterSobelHoriz_32f_C3R
//              ippiFilterSobelHoriz_32f_AC4R
//              ippiFilterSobelHoriz_32f_C4R
//              ippiFilterSobelVert_8u_C1R
//              ippiFilterSobelVert_8u_C3R
//              ippiFilterSobelVert_8u_AC4R
//              ippiFilterSobelVert_8u_C4R
//              ippiFilterSobelVert_16s_C1R
//              ippiFilterSobelVert_16s_C3R
//              ippiFilterSobelVert_16s_AC4R
//              ippiFilterSobelVert_16s_C4R
//              ippiFilterSobelVert_32f_C1R
//              ippiFilterSobelVert_32f_C3R
//              ippiFilterSobelVert_32f_AC4R
//              ippiFilterSobelVert_32f_C4R
//              ippiFilterRobertsDown_8u_C1R
//              ippiFilterRobertsDown_8u_C3R
//              ippiFilterRobertsDown_8u_AC4R
//              ippiFilterRobertsDown_16s_C1R
//              ippiFilterRobertsDown_16s_C3R
//              ippiFilterRobertsDown_16s_AC4R
//              ippiFilterRobertsDown_32f_C1R
//              ippiFilterRobertsDown_32f_C3R
//              ippiFilterRobertsDown_32f_AC4R
//              ippiFilterRobertsUp_8u_C1R
//              ippiFilterRobertsUp_8u_C3R
//              ippiFilterRobertsUp_8u_AC4R
//              ippiFilterRobertsUp_16s_C1R
//              ippiFilterRobertsUp_16s_C3R
//              ippiFilterRobertsUp_16s_AC4R
//              ippiFilterRobertsUp_32f_C1R
//              ippiFilterRobertsUp_32f_C3R
//              ippiFilterRobertsUp_32f_AC4R
//              ippiFilterSharpen_8u_C1R
//              ippiFilterSharpen_8u_C3R
//              ippiFilterSharpen_8u_AC4R
//              ippiFilterSharpen_8u_C4R
//              ippiFilterSharpen_16s_C1R
//              ippiFilterSharpen_16s_C3R
//              ippiFilterSharpen_16s_AC4R
//              ippiFilterSharpen_16s_C4R
//              ippiFilterSharpen_32f_C1R
//              ippiFilterSharpen_32f_C3R
//              ippiFilterSharpen_32f_AC4R
//              ippiFilterSharpen_32f_C4R
//              ippiFilterScharrVert_8u16s_C1R
//              ippiFilterScharrVert_8s16s_C1R
//              ippiFilterScharrVert_32f_C1R
//              ippiFilterScharrHoriz_8u16s_C1R
//              ippiFilterScharrHoriz_8s16s_C1R
//              ippiFilterScharrHoriz_32f_C1R
//
//  Purpose:  Perform linear filtering of an image using one of
//            predefined convolution kernels (3x3):
//
//                                1  1  1
//              PrewittHoriz      0  0  0
//                               -1 -1 -1
//
//
//                               -1  0  1
//              PrewittVert      -1  0  1
//                               -1  0  1
//
//
//                                1  2  1
//              SobelHoriz        0  0  0
//                               -1 -2 -1
//
//
//                               -1  0  1
//              SobelVert        -2  0  2
//                               -1  0  1
//
//
//                                0  0  0
//              RobetsDown        0  1  0
//                                0  0 -1
//
//
//                                0  0  0
//              RobertsUp         0  1  0
//                               -1  0  0
//
//
//                               -1 -1  1
//              Sharpen          -1 16  1  X  1/8
//                               -1 -1  1
//
//
//                                3  0  -3
//              ScharrVert       10  0 -10
//                                3  0  -3
//
//
//                                3  10  3
//              ScharrHoriz       0   0  0
//                               -3 -10 -3
//
//
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  pSrc or pDst is NULL
//   ippStsSizeErr     roiSize has a field with zero or negative value
//   ippStsStepErr     srcStep or dstStep has zero or negative value
//
//  Parameters:
//   pSrc       Pointer to the source image
//   srcStep    Step through the source image
//   pDst       Pointer to the destination image
//   dstStep    Step through the destination image
//   roiSize    size of the ROI
*/
IPPAPI(IppStatus,ippiFilterPrewittVert_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittVert_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittVert_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittVert_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittVert_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittVert_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittVert_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittVert_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittVert_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittVert_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittVert_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittVert_32f_C4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittHoriz_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittHoriz_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittHoriz_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittHoriz_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittHoriz_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittHoriz_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittHoriz_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittHoriz_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittHoriz_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittHoriz_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittHoriz_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterPrewittHoriz_32f_C4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelVert_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelVert_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelVert_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelVert_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelVert_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelVert_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelVert_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelVert_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelVert_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelVert_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelVert_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelVert_32f_C4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelHoriz_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelHoriz_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelHoriz_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelHoriz_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelHoriz_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelHoriz_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelHoriz_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelHoriz_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelHoriz_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelHoriz_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelHoriz_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSobelHoriz_32f_C4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsUp_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsUp_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsUp_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsUp_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsUp_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsUp_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsUp_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsUp_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsUp_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsDown_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsDown_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsDown_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsDown_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsDown_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsDown_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsDown_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsDown_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterRobertsDown_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSharpen_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSharpen_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSharpen_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSharpen_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSharpen_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSharpen_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSharpen_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSharpen_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSharpen_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSharpen_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSharpen_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterSharpen_32f_C4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiFilterScharrVert_8u16s_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiFilterScharrHoriz_8u16s_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiFilterScharrVert_8s16s_C1R,(const Ipp8s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiFilterScharrHoriz_8s16s_C1R,(const Ipp8s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiFilterScharrVert_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize ))
IPPAPI(IppStatus,ippiFilterScharrHoriz_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize ))


/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilterLaplace_8u_C1R
//              ippiFilterLaplace_8u_C3R
//              ippiFilterLaplace_8u_AC4R
//              ippiFilterLaplace_8u_C4R
//              ippiFilterLaplace_16s_C1R
//              ippiFilterLaplace_16s_C3R
//              ippiFilterLaplace_16s_AC4R
//              ippiFilterLaplace_16s_C4R
//              ippiFilterLaplace_32f_C1R
//              ippiFilterLaplace_32f_C3R
//              ippiFilterLaplace_32f_AC4R
//              ippiFilterLaplace_32f_C4R
//              ippiFilterGauss_8u_C1R
//              ippiFilterGauss_8u_C3R
//              ippiFilterGauss_8u_AC4R
//              ippiFilterGauss_8u_C4R
//              ippiFilterGauss_16s_C1R
//              ippiFilterGauss_16s_C3R
//              ippiFilterGauss_16s_AC4R
//              ippiFilterGauss_16s_C4R
//              ippiFilterGauss_16u_C1R
//              ippiFilterGauss_16u_C3R
//              ippiFilterGauss_16u_AC4R
//              ippiFilterGauss_16u_C4R
//              ippiFilterGauss_32f_C1R
//              ippiFilterGauss_32f_C3R
//              ippiFilterGauss_32f_AC4R
//              ippiFilterGauss_32f_C4R
//              ippiFilterLowpass_8u_C1R
//              ippiFilterLowpass_8u_C3R
//              ippiFilterLowpass_8u_AC4R
//              ippiFilterLowpass_16s_C1R
//              ippiFilterLowpass_16s_C3R
//              ippiFilterLowpass_16s_AC4R
//              ippiFilterLowpass_16u_C1R
//              ippiFilterLowpass_16u_C3R
//              ippiFilterLowpass_16u_AC4R
//              ippiFilterLowpass_32f_C1R
//              ippiFilterLowpass_32f_C3R
//              ippiFilterLowpass_32f_AC4R
//              ippiFilterHipass_8u_C1R
//              ippiFilterHipass_8u_C3R
//              ippiFilterHipass_8u_AC4R
//              ippiFilterHipass_8u_C4R
//              ippiFilterHipass_16s_C1R
//              ippiFilterHipass_16s_C3R
//              ippiFilterHipass_16s_AC4R
//              ippiFilterHipass_16s_C4R
//              ippiFilterHipass_32f_C1R
//              ippiFilterHipass_32f_C3R
//              ippiFilterHipass_32f_AC4R
//              ippiFilterHipass_32f_C4R
//              ippiFilterSobelVert_8u16s_C1R
//              ippiFilterSobelVert_8s16s_C1R
//              ippiFilterSobelVertMask_32f_C1R
//              ippiFilterSobelHoriz_8u16s_C1R
//              ippiFilterSobelHoriz_8s16s_C1R
//              ippiFilterSobelHorizMask_32f_C1R
//              ippiFilterSobelVertSecond_8u16s_C1R
//              ippiFilterSobelVertSecond_8s16s_C1R
//              ippiFilterSobelVertSecond_32f_C1R
//              ippiFilterSobelHorizSecond_8u16s_C1R
//              ippiFilterSobelHorizSecond_8s16s_C1R
//              ippiFilterSobelHorizSecond_32f_C1R
//              ippiFilterSobelCross_8u16s_C1R
//              ippiFilterSobelCross_8s16s_C1R
//              ippiFilterSobelCross_32f_C1R
//
//  Purpose:   Perform linear filtering of an image using one of
//             predefined convolution kernels (3x3 or 5x5):
//
//                               -1 -1  1
//              Laplace (3x3)    -1  8  1
//                               -1 -1  1
//
//
//                                1  2  1
//              Gauss (3x3)       2  4  2  X  1/16
//                                1  2  1
//
//
//                                1  1  1
//              Lowpass (3x3)     1  1  1  X  1/9
//                                1  1  1
//
//
//                               -1 -1 -1
//              Hipass (3x3 )    -1  8 -1
//                               -1 -1 -1
//
//
//                               -1  0  1
//              SobelVert (3x3)  -2  0  2
//                               -1  0  1
//
//
//                                1  2  1
//              SobelHoriz (3x3)  0  0  0
//                               -1 -2 -1
//
//
//                                       1 -2  1
//              SobelVertSecond (3x3)    2 -4  2
//                                       1 -2  1
//
//
//                                       1  2  1
//              SobelHorizSecond (3x3)  -2 -4 -2
//                                       1  2  1
//
//
//                               -1  0  1
//              SobelCross (3x3)  0  0  0
//                                1  0 -1
//
//
//                               -1 -3 -4 -3 -1
//                               -3  0  6  0 -3
//              Laplace (5x5)    -4  6 20  6 -4
//                               -3  0  6  0 -3
//                               -1 -3 -4 -3 -1
//
//                                2   7  12   7   2
//                                7  31  52  31   7
//              Gauss (5x5)      12  52 127  52  12  X  1/571
//                                7  31  52  31   7
//                                2   7  12   7   2
//
//                                1 1 1 1 1
//                                1 1 1 1 1
//              Lowpass (5x5)     1 1 1 1 1  X  1/25
//                                1 1 1 1 1
//                                1 1 1 1 1
//

//                               -1 -1 -1 -1 -1
//                               -1 -1 -1 -1 -1
//              Hipass (5x5)     -1 -1 24 -1 -1
//                               -1 -1 -1 -1 -1
//                               -1 -1 -1 -1 -1
//
//                               -1  -2   0   2   1
//                               -4  -8   0   8   4
//              SobelVert (5x5)  -6 -12   0  12   6
//                               -4  -8   0   8   4
//                               -1  -2   0   2   1
//
//                                1   4   6   4   1
//                                2   8  12   8   2
//              SobelHoriz (5x5)  0   0   0   0   0
//                               -2  -8 -12  -8  -4
//                               -1  -4  -6  -4  -1
//
//                                       1   0  -2   0   1
//                                       4   0  -8   0   4
//              SobelVertSecond (5x5)    6   0 -12   0   6
//                                       4   0  -8   0   4
//                                       1   0  -2   0   1
//
//                                       1   4   6   4   1
//                                       0   0   0   0   0
//              SobelVertHoriz (5x5)    -2  -8 -12  -8  -2
//                                       0   0   0   0   0
//                                       1   4   6   4   1
//
//                               -1  -2   0   2   1
//                               -2  -4   0   4   2
//              SobelCross (5x5)  0   0   0   0   0
//                                2   4   0  -4  -2
//                                1   2   0  -2  -1
//
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  pSrc or pDst is NULL
//   ippStsSizeErr     roiSize has a field with zero or negative value
//   ippStsStepErr     srcStep or dstStep has zero or negative value
//   ippStsMaskSizeErr Illegal mask value
//
//  Parameters:
//   pSrc       Pointer to the source image
//   srcStep    Step through the source image
//   pDst       Pointer to the destination image
//   dstStep    Step through the destination image
//   roiSize    size of the ROI
//   mask       Filter mask
*/
IPPAPI(IppStatus,ippiFilterLaplace_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLaplace_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLaplace_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLaplace_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLaplace_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLaplace_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLaplace_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLaplace_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLaplace_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLaplace_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLaplace_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLaplace_32f_C4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_16u_C4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterGauss_32f_C4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterHipass_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterHipass_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterHipass_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterHipass_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterHipass_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterHipass_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterHipass_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterHipass_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterHipass_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterHipass_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterHipass_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterHipass_32f_C4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLowpass_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLowpass_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLowpass_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLowpass_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLowpass_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLowpass_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLowpass_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLowpass_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLowpass_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLowpass_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLowpass_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLowpass_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))
IPPAPI(IppStatus,ippiFilterLaplace_8u16s_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterLaplace_8s16s_C1R,(const Ipp8s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelVert_8u16s_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelHoriz_8u16s_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelVertSecond_8u16s_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelHorizSecond_8u16s_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelCross_8u16s_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelVert_8s16s_C1R,(const Ipp8s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelHoriz_8s16s_C1R,(const Ipp8s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelVertSecond_8s16s_C1R,(const Ipp8s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelHorizSecond_8s16s_C1R,(const Ipp8s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelCross_8s16s_C1R,(const Ipp8s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelVertMask_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelHorizMask_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelVertSecond_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelHorizSecond_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))
IPPAPI(IppStatus,ippiFilterSobelCross_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask ))


/* ///////////////////////////////////////////////////////////////////////////
//             General Linear Filters
/////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////////
//   Names:     ippiFilter_8u_C1R
//              ippiFilter_8u_C3R
//              ippiFilter_8u_C4R
//              ippiFilter_8u_AC4R
//              ippiFilter_16s_C1R    ippiFilter_16u_C1R
//              ippiFilter_16s_C3R    ippiFilter_16u_C3R
//              ippiFilter_16s_C4R    ippiFilter_16u_C4R
//              ippiFilter_16s_AC4R   ippiFilter_16u_AC4R
//
//  Purpose:    Filters an image using a general integer rectangular kernel
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of the pointers is NULL
//   ippStsSizeErr     dstRoiSize or kernelSize has a field with zero or negative value
//   ippStsDivisorErr  divisor value is zero, function execution is interrupted
//
//  Parameters:
//      pSrc        Pointer to the source buffer
//      srcStep     Step in bytes through the source image buffer
//      pDst        Pointer to the destination buffer
//      dstStep     Step in bytes through the destination image buffer
//      dstRoiSize  Size of the source and destination ROI in pixels
//      pKernel     Pointer to the kernel values ( 32s kernel )
//      kernelSize  Size of the rectangular kernel in pixels.
//      anchor      Anchor cell specifying the rectangular kernel alignment
//                  with respect to the position of the input pixel
//      divisor     The integer value by which the computed result is divided.
*/

IPPAPI( IppStatus, ippiFilter_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor ))
IPPAPI( IppStatus, ippiFilter_8u_C3R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor ))
IPPAPI( IppStatus, ippiFilter_8u_C4R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor ))
IPPAPI( IppStatus, ippiFilter_8u_AC4R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor ))
IPPAPI( IppStatus, ippiFilter_16s_C1R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor ))
IPPAPI( IppStatus, ippiFilter_16s_C3R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor ))
IPPAPI( IppStatus, ippiFilter_16s_C4R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor ))
IPPAPI( IppStatus, ippiFilter_16s_AC4R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor ))
IPPAPI( IppStatus, ippiFilter_16u_C1R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor ))
IPPAPI( IppStatus, ippiFilter_16u_C3R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor ))
IPPAPI( IppStatus, ippiFilter_16u_C4R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor ))
IPPAPI( IppStatus, ippiFilter_16u_AC4R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor ))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilter32f_8u_C1R
//              ippiFilter32f_8u_C3R
//              ippiFilter32f_8u_C4R
//              ippiFilter32f_8u_AC4R
//              ippiFilter32f_16s_C1R    ippiFilter32f_16u_C1R
//              ippiFilter32f_16s_C3R    ippiFilter32f_16u_C3R
//              ippiFilter32f_16s_C4R    ippiFilter32f_16u_C4R
//              ippiFilter32f_16s_AC4R   ippiFilter32f_16u_AC4R
//              ippiFilter32f_8u16s_C1R
//              ippiFilter32f_8u16s_C3R
//              ippiFilter32f_8u16s_C4R
//              ippiFilter32f_8s16s_C1R
//              ippiFilter32f_8s16s_C3R
//              ippiFilter32f_8s16s_C4R
//              ippiFilter32f_8s_C1R
//              ippiFilter32f_8s_C3R
//              ippiFilter32f_8s_C4R
//              ippiFilter32f_32s_C1R
//              ippiFilter32f_32s_C3R
//              ippiFilter32f_32s_C4R
//              ippiFilter_32f_C1R
//              ippiFilter_32f_C3R
//              ippiFilter_32f_C4R
//              ippiFilter_32f_AC4R
//  Purpose:    Filters an image that consists of integer data with use of
//              the rectangular kernel of floating-point values.
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of the pointers is NULL
//   ippStsSizeErr     dstRoiSize or kernelSize has a field with zero or negative value
//
//  Parameters:
//      pSrc            Pointer to the source buffer
//      srcStep         Step in bytes through the source image buffer
//      pDst            Pointer to the destination buffer
//      dstStep         Step in bytes through the destination image buffer
//      dstRoiSize      Size of the source and destination ROI in pixels
//      pKernel         Pointer to the kernel values ( 32f kernel )
//      kernelSize      Size of the rectangular kernel in pixels.
//      anchor          Anchor cell specifying the rectangular kernel alignment
//                      with respect to the position of the input pixel
*/
IPPAPI( IppStatus, ippiFilter32f_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_8u_C3R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_8u_C4R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_8u_AC4R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_16s_C1R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_16s_C3R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_16s_C4R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_16s_AC4R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_16u_C1R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_16u_C3R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_16u_C4R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_16u_AC4R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
        Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter_32f_C3R, ( const Ipp32f* pSrc, int srcStep,
        Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter_32f_C4R, ( const Ipp32f* pSrc, int srcStep,
        Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter_32f_AC4R, ( const Ipp32f* pSrc, int srcStep,
        Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_8u16s_C1R, ( const Ipp8u* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_8u16s_C3R, ( const Ipp8u* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_8u16s_C4R, ( const Ipp8u* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_8s16s_C1R, ( const Ipp8s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_8s16s_C3R, ( const Ipp8s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_8s16s_C4R, ( const Ipp8s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_8s_C1R, ( const Ipp8s* pSrc, int srcStep,
        Ipp8s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_8s_C3R, ( const Ipp8s* pSrc, int srcStep,
        Ipp8s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_8s_C4R, ( const Ipp8s* pSrc, int srcStep,
        Ipp8s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_32s_C1R, ( const Ipp32s* pSrc, int srcStep,
        Ipp32s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_32s_C3R, ( const Ipp32s* pSrc, int srcStep,
        Ipp32s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))
IPPAPI( IppStatus, ippiFilter32f_32s_C4R, ( const Ipp32s* pSrc, int srcStep,
        Ipp32s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor ))

/* ////////////////////////////////////////////////////////////////////////////
//                Separable Filters
//////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////////
//   Names:     ippiFilterColumn_8u_C1R
//              ippiFilterColumn_8u_C3R
//              ippiFilterColumn_8u_C4R
//              ippiFilterColumn_8u_AC4R
//              ippiFilterColumn_16s_C1R    ippiFilterColumn_16u_C1R
//              ippiFilterColumn_16s_C3R    ippiFilterColumn_16u_C3R
//              ippiFilterColumn_16s_C4R    ippiFilterColumn_16u_C4R
//              ippiFilterColumn_16s_AC4R   ippiFilterColumn_16u_AC4R
//
//  Purpose:    Filters an image using a spatial 32s kernel consisting of a
//              single column
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of the pointers is NULL
//   ippStsSizeErr     dstRoiSize has a field with zero or negative value, or
//                     kernelSize value is zero or negative
//   ippStsDivisorErr  divisor value is zero, function execution is interrupted
//
//  Parameters:
//      pSrc        Pointer to the source buffer
//      srcStep     Step in bytes through the source image buffer
//      pDst        Pointer to the destination buffer
//      dstStep     Step in bytes through the destination image buffer
//      dstRoiSize  Size of the source and destination ROI in pixels
//      pKernel     Pointer to the column kernel values ( 32s kernel )
//      kernelSize  Size of the column kernel in pixels.
//      yAnchor     Anchor cell specifying the kernel vertical alignment with
//                  respect to the position of the input pixel
//      divisor     The integer value by which the computed result is divided.
*/
IPPAPI( IppStatus, ippiFilterColumn_8u_C1R, ( const Ipp8u* pSrc,
        int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32s* pKernel, int kernelSize, int yAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterColumn_8u_C3R, ( const Ipp8u* pSrc,
        int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32s* pKernel, int kernelSize, int yAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterColumn_8u_C4R, ( const Ipp8u* pSrc,
        int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32s* pKernel, int kernelSize, int yAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterColumn_8u_AC4R, ( const Ipp8u* pSrc,
        int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32s* pKernel, int kernelSize, int yAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterColumn_16s_C1R, ( const Ipp16s* pSrc,
        int srcStep, Ipp16s* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32s* pKernel, int kernelSize, int yAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterColumn_16s_C3R, ( const Ipp16s* pSrc,
        int srcStep, Ipp16s* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32s* pKernel, int kernelSize, int yAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterColumn_16s_C4R, ( const Ipp16s* pSrc,
        int srcStep, Ipp16s* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32s* pKernel, int kernelSize, int yAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterColumn_16s_AC4R, ( const Ipp16s* pSrc,
        int srcStep, Ipp16s* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32s* pKernel, int kernelSize, int yAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterColumn_16u_C1R, ( const Ipp16u* pSrc,
        int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32s* pKernel, int kernelSize, int yAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterColumn_16u_C3R, ( const Ipp16u* pSrc,
        int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32s* pKernel, int kernelSize, int yAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterColumn_16u_C4R, ( const Ipp16u* pSrc,
        int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32s* pKernel, int kernelSize, int yAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterColumn_16u_AC4R, ( const Ipp16u* pSrc,
        int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32s* pKernel, int kernelSize, int yAnchor, int divisor ))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilterColumn32f_8u_C1R
//              ippiFilterColumn32f_8u_C3R
//              ippiFilterColumn32f_8u_C4R
//              ippiFilterColumn32f_8u_AC4R
//              ippiFilterColumn32f_16s_C1R    ippiFilterColumn32f_16u_C1R
//              ippiFilterColumn32f_16s_C3R    ippiFilterColumn32f_16u_C3R
//              ippiFilterColumn32f_16s_C4R    ippiFilterColumn32f_16u_C4R
//              ippiFilterColumn32f_16s_AC4R   ippiFilterColumn32f_16u_AC4R
//              ippiFilterColumn_32f_C1R
//              ippiFilterColumn_32f_C3R
//              ippiFilterColumn_32f_C4R
//              ippiFilterColumn_32f_AC4R
//
//  Purpose:    Filters an image using a spatial 32f kernel consisting of a
//              single column
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  Some of pointers to pSrc, pDst or pKernel are NULL
//   ippStsSizeErr     dstRoiSize has a field with zero or negative value, or
//                     kernelSize value is zero or negative
//
//  Parameters:
//      pSrc        Pointer to the source buffer
//      srcStep     Step in bytes through the source image buffer
//      pDst        Pointer to the destination buffer
//      dstStep     Step in bytes through the destination image buffer
//      dstRoiSize  Size of the source and destination ROI in pixels
//      pKernel     Pointer to the column kernel values ( 32f kernel )
//      kernelSize  Size of the column kernel in pixels.
//      yAnchor     Anchor cell specifying the kernel vertical alignment with
//                  respect to the position of the input pixel
*/
IPPAPI( IppStatus, ippiFilterColumn32f_8u_C1R, ( const Ipp8u* pSrc,
        int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn32f_8u_C3R, ( const Ipp8u* pSrc,
        int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn32f_8u_C4R, ( const Ipp8u* pSrc,
        int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn32f_8u_AC4R, ( const Ipp8u* pSrc,
        int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn32f_16s_C1R, ( const Ipp16s* pSrc,
        int srcStep, Ipp16s* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn32f_16s_C3R, ( const Ipp16s* pSrc,
        int srcStep, Ipp16s* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn32f_16s_C4R, ( const Ipp16s* pSrc,
        int srcStep, Ipp16s* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn32f_16s_AC4R, ( const Ipp16s* pSrc,
        int srcStep, Ipp16s* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn32f_16u_C1R, ( const Ipp16u* pSrc,
        int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn32f_16u_C3R, ( const Ipp16u* pSrc,
        int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn32f_16u_C4R, ( const Ipp16u* pSrc,
        int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn32f_16u_AC4R, ( const Ipp16u* pSrc,
        int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn_32f_C1R, ( const Ipp32f* pSrc,
        int srcStep, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn_32f_C3R, ( const Ipp32f* pSrc,
        int srcStep, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn_32f_C4R, ( const Ipp32f* pSrc,
        int srcStep, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))
IPPAPI( IppStatus, ippiFilterColumn_32f_AC4R, ( const Ipp32f* pSrc,
        int srcStep, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
        const Ipp32f* pKernel, int kernelSize, int yAnchor ))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilterRow_8u_C1R
//              ippiFilterRow_8u_C3R
//              ippiFilterRow_8u_C4R
//              ippiFilterRow_8u_AC4R
//              ippiFilterRow_16s_C1R    ippiFilterRow_16u_C1R
//              ippiFilterRow_16s_C3R    ippiFilterRow_16u_C3R
//              ippiFilterRow_16s_C4R    ippiFilterRow_16u_C4R
//              ippiFilterRow_16s_AC4R   ippiFilterRow_16u_AC4R
//
//  Purpose:   Filters an image using a spatial 32s kernel consisting of a
//             single row
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of the pointers is NULL
//   ippStsSizeErr     dstRoiSize has a field with zero or negative value, or
//                     kernelSize value is zero or negative
//   ippStsDivisorErr  divisor value is zero, function execution is interrupted
//
//  Parameters:
//      pSrc        Pointer to the source buffer
//      srcStep     Step in bytes through the source image buffer
//      pDst        Pointer to the destination buffer
//      dstStep     Step in bytes through the destination image buffer
//      dstRoiSize  Size of the source and destination ROI in pixels
//      pKernel     Pointer to the row kernel values ( 32s kernel )
//      kernelSize  Size of the row kernel in pixels.
//      xAnchor     Anchor cell specifying the kernel horizontal alignment with
//                  respect to the position of the input pixel.
//      divisor     The integer value by which the computed result is divided.
*/
IPPAPI( IppStatus, ippiFilterRow_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        int kernelSize, int xAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterRow_8u_C3R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        int kernelSize, int xAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterRow_8u_C4R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        int kernelSize, int xAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterRow_8u_AC4R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        int kernelSize, int xAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterRow_16s_C1R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        int kernelSize, int xAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterRow_16s_C3R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        int kernelSize, int xAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterRow_16s_C4R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        int kernelSize, int xAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterRow_16s_AC4R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        int kernelSize, int xAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterRow_16u_C1R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        int kernelSize, int xAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterRow_16u_C3R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        int kernelSize, int xAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterRow_16u_C4R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        int kernelSize, int xAnchor, int divisor ))
IPPAPI( IppStatus, ippiFilterRow_16u_AC4R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        int kernelSize, int xAnchor, int divisor ))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiFilterRow32f_8u_C1R
//              ippiFilterRow32f_8u_C3R
//              ippiFilterRow32f_8u_C4R
//              ippiFilterRow32f_8u_AC4R
//              ippiFilterRow32f_16s_C1R    ippiFilterRow32f_16u_C1R
//              ippiFilterRow32f_16s_C3R    ippiFilterRow32f_16u_C3R
//              ippiFilterRow32f_16s_C4R    ippiFilterRow32f_16u_C4R
//              ippiFilterRow32f_16s_AC4R   ippiFilterRow32f_16u_AC4R
//              ippiFilterRow_32f_C1R
//              ippiFilterRow_32f_C3R
//              ippiFilterRow_32f_C4R
//              ippiFilterRow_32f_AC4R
//
//  Purpose:   Filters an image using a spatial 32f kernel consisting of a
//             single row
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of the pointers is NULL
//   ippStsSizeErr     dstRoiSize has a field with zero or negative value, or
//                     kernelSize value is zero or negative
//
//  Parameters:
//      pSrc        Pointer to the source buffer;
//      srcStep     Step in bytes through the source image buffer;
//      pDst        Pointer to the destination buffer;
//      dstStep     Step in bytes through the destination image buffer;
//      dstRoiSize  Size of the source and destination ROI in pixels;
//      pKernel     Pointer to the row kernel values ( 32f kernel );
//      kernelSize  Size of the row kernel in pixels;
//      xAnchor     Anchor cell specifying the kernel horizontal alignment with
//                  respect to the position of the input pixel.
*/
IPPAPI( IppStatus, ippiFilterRow32f_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow32f_8u_C3R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow32f_8u_C4R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow32f_8u_AC4R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow32f_16s_C1R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow32f_16s_C3R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow32f_16s_C4R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow32f_16s_AC4R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow32f_16u_C1R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow32f_16u_C3R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow32f_16u_C4R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow32f_16u_AC4R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
        Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow_32f_C3R, ( const Ipp32f* pSrc, int srcStep,
        Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow_32f_C4R, ( const Ipp32f* pSrc, int srcStep,
        Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))
IPPAPI( IppStatus, ippiFilterRow_32f_AC4R, ( const Ipp32f* pSrc, int srcStep,
        Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        int kernelSize, int xAnchor ))


/* /////////////////////////////////////////////////////////////////////////////
//                  Wiener Filters
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//      ippiFilterWienerGetBufferSize,
//  Purpose: Computes the size of the external buffer for Wiener filter
//
//      ippiFilterWiener_8u_C1R,  ippiFilterWiener_16s_C1R,
//      ippiFilterWiener_8u_C3R,  ippiFilterWiener_16s_C3R,
//      ippiFilterWiener_8u_C4R,  ippiFilterWiener_16s_C4R,
//      ippiFilterWiener_8u_AC4R, ippiFilterWiener_16s_AC4R,
//      ippiFilterWiener_32f_C1R,
//      ippiFilterWiener_32f_C3R,
//      ippiFilterWiener_32f_C4R,
//      ippiFilterWiener_32f_AC4R.
//
//  Purpose: Performs two-dimensional adaptive noise-removal
//           filtering of an image using Wiener filter.
//
//  Parameters:
//      pSrc        Pointer to the source image ROI;
//      srcStep     Step in bytes through the source image buffer;
//      pDst        Pointer to the destination image ROI;
//      dstStep     Step in bytes through the destination image buffer;
//      dstRoiSize  Size of the destination ROI in pixels;
//      maskSize    Size of the rectangular local pixel neighborhood (mask);
//      anchor      Anchor cell specifying the mask alignment
//                           with respect to the position of the input pixel;
//      noise       Noise level value or array of the noise level values for
//                                                       multi-channel image;
//      pBuffer     Pointer to the external work buffer;
//      pBufferSize Pointer to the computed value of the external buffer size;
//      channels    Number of channels in the image ( 1, 3, or 4 ).
//
//  Returns:
//   ippStsNoErr           OK
//   ippStsNumChannelsErr  channels is not 1, 3, or 4
//   ippStsNullPtrErr      One of the pointers is NULL;
//   ippStsSizeErr         dstRoiSize has a field with zero or negative value
//   ippStsMaskSizeErr     maskSize has a field with zero or negative value
//   ippStsNoiseRangeErr   One of the noise values is less than 0
//                                                         or greater than 1.0;
*/

IPPAPI( IppStatus, ippiFilterWienerGetBufferSize,( IppiSize dstRoiSize,
                          IppiSize maskSize, int channels, int* pBufferSize ))
IPPAPI( IppStatus, ippiFilterWiener_8u_C1R,( const Ipp8u* pSrc, int srcStep,
              Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
                           IppiPoint anchor, Ipp32f noise[1], Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilterWiener_8u_C3R,( const Ipp8u* pSrc, int srcStep,
              Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
                           IppiPoint anchor, Ipp32f noise[3], Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilterWiener_8u_AC4R,( const Ipp8u* pSrc, int srcStep,
              Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
                           IppiPoint anchor, Ipp32f noise[3], Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilterWiener_8u_C4R,( const Ipp8u* pSrc, int srcStep,
              Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
                           IppiPoint anchor, Ipp32f noise[4], Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilterWiener_16s_C1R,( const Ipp16s* pSrc, int srcStep,
             Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
                           IppiPoint anchor, Ipp32f noise[1], Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilterWiener_16s_C3R,( const Ipp16s* pSrc, int srcStep,
             Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
                           IppiPoint anchor, Ipp32f noise[3], Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilterWiener_16s_AC4R,( const Ipp16s* pSrc, int srcStep,
             Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
                           IppiPoint anchor, Ipp32f noise[3], Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilterWiener_16s_C4R,( const Ipp16s* pSrc, int srcStep,
             Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
                           IppiPoint anchor, Ipp32f noise[4], Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilterWiener_32f_C1R,( const Ipp32f* pSrc, int srcStep,
             Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
                           IppiPoint anchor, Ipp32f noise[1], Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilterWiener_32f_C3R,( const Ipp32f* pSrc, int srcStep,
             Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
                           IppiPoint anchor, Ipp32f noise[3], Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilterWiener_32f_AC4R,( const Ipp32f* pSrc, int srcStep,
             Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
                           IppiPoint anchor, Ipp32f noise[3], Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilterWiener_32f_C4R,( const Ipp32f* pSrc, int srcStep,
             Ipp32f* pDst, int dstStep, IppiSize dstRoiSize, IppiSize maskSize,
                           IppiPoint anchor, Ipp32f noise[4], Ipp8u* pBuffer ))

/* ////////////////////////////////////////////////////////////////////////////
//   Names:     ippiConvFull_32f_C1R
//              ippiConvFull_32f_C3R
//              ippiConvFull_32f_AC4R
//              ippiConvFull_16s_C1R
//              ippiConvFull_16s_C3R
//              ippiConvFull_16s_AC4R
//              ippiConvFull_8u_C1R
//              ippiConvFull_8u_C3R
//              ippiConvFull_8u_AC4R
//
//  Purpose: Performs full 2-D convolution of matrices (images). If IppiSize's
//           of matrices are Wa*Ha and Wb*Hb correspondingly, then the
//           IppiSize of the resulting matrix (image) will be
//              (Wa+Wb-1)*(Ha+Hb-1).
//           If the resulting IppiSize > CRITERION, then convolution is done
//             using 2D FFT.
//
//  Returns:
//      ippStsNoErr       OK;
//      ippStsNullPtrErr  One of the pointers pSrc1, pSrc2, pDst is NULL;
//      ippStsSizeErr     src1Size, src2Size has at least one field with
//                                                zero or negative value;
//      ippStsStepErr     One of the step values is zero or negative;
///     ippStsDivisorErr  divisor value is zero, function execution is interrupted;
//      ippStsMemAllocErr Memory allocation error.
//
//  Parameters:
//      pSrc1       Pointer to the source buffer 1;
//      src1Step    Step in bytes through the source image buffer 1;
//      src1Size    Size of the source buffer 1 in pixels;
//      pSrc2       Pointer to the source buffer 2;
//      src2Step    Step in bytes through the source image buffer 2;
//      Src2Size    Size of the source buffer 2 in pixels;
//      pDst        Pointer to the destination buffer;
//      dstStep     Step in bytes through the destination image buffer;
//      divisor     The integer value by which the computed result is divided
//                  (in case of 8u or 16s data).
*/

IPPAPI( IppStatus, ippiConvFull_32f_C1R,( const Ipp32f* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp32f* pSrc2, int src2Step, IppiSize src2Size,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiConvFull_32f_C3R,( const Ipp32f* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp32f* pSrc2, int src2Step, IppiSize src2Size,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiConvFull_32f_AC4R,( const Ipp32f* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp32f* pSrc2, int src2Step, IppiSize src2Size,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiConvFull_16s_C1R,( const Ipp16s* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp16s* pSrc2, int src2Step, IppiSize src2Size,
        Ipp16s* pDst, int dstStep, int divisor ))
IPPAPI( IppStatus, ippiConvFull_16s_C3R,( const Ipp16s* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp16s* pSrc2, int src2Step, IppiSize src2Size,
        Ipp16s* pDst, int dstStep, int divisor ))
IPPAPI( IppStatus, ippiConvFull_16s_AC4R,( const Ipp16s* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp16s* pSrc2, int src2Step, IppiSize src2Size,
        Ipp16s* pDst, int dstStep, int divisor ))
IPPAPI( IppStatus, ippiConvFull_8u_C1R,( const Ipp8u* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp8u* pSrc2, int src2Step, IppiSize src2Size,
        Ipp8u* pDst, int dstStep, int divisor ))
IPPAPI( IppStatus, ippiConvFull_8u_C3R,( const Ipp8u* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp8u* pSrc2, int src2Step, IppiSize src2Size,
        Ipp8u* pDst, int dstStep, int divisor ))
IPPAPI( IppStatus, ippiConvFull_8u_AC4R,( const Ipp8u* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp8u* pSrc2, int src2Step, IppiSize src2Size,
        Ipp8u* pDst, int dstStep, int divisor ))



/* ////////////////////////////////////////////////////////////////////////////
//   Names:     ippiConvValid_32f_C1R
//              ippiConvValid_32f_C3R
//              ippiConvValid_32f_AC4R
//              ippiConvValid_16s_C1R
//              ippiConvValid_16s_C3R
//              ippiConvValid_16s_AC4R
//              ippiConvValid_8u_C1R
//              ippiConvValid_8u_C3R
//              ippiConvValid_8u_AC4R
//
//  Purpose: Performs the VALID 2-D convolution of matrices (images).
//           If IppiSize's of matrices (images) are Wa*Ha and Wb*Hb
//           correspondingly, then the IppiSize of the resulting matrix
//           (image) will be (|Wa-Wb|+1)*(|Ha-Hb|+1).
//           If the smallest image IppiSize > CRITERION, then convolution
//           is done using 2D FFT.
//
//  Returns:
//      ippStsNoErr       OK;
//      ippStsNullPtrErr  One of the pointers pSrc1, pSrc2, pDst is NULL;
//      ippStsSizeErr     src1Size, src2Size has at least one field with
//                                                zero or negative value;
//      ippStsStepErr     One of the step values is zero or negative;
///     ippStsDivisorErr  divisor value is zero, function execution is interrupted;
//      ippStsMemAllocErr Memory allocation error.
//
//  Parameters:
//      pSrc1       Pointer to the source buffer 1;
//      src1Step    Step in bytes through the source image buffer 1;
//      src1Size    Size of the source buffer 1 in pixels;
//      pSrc2       Pointer to the source buffer 2;
//      src2Step    Step in bytes through the source image buffer 2;
//      src2Size    Size of the source buffer 2 in pixels;
//      pDst        Pointer to the destination buffer;
//      dstStep     Step in bytes through the destination image buffer;
//      divisor     The integer value by which the computed result is divided
//                  (in case of 8u or 16s data).
*/

IPPAPI( IppStatus, ippiConvValid_32f_C1R,( const Ipp32f* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp32f* pSrc2, int src2Step, IppiSize src2Size,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiConvValid_32f_C3R,( const Ipp32f* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp32f* pSrc2, int src2Step, IppiSize src2Size,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiConvValid_32f_AC4R,( const Ipp32f* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp32f* pSrc2, int src2Step, IppiSize src2Size,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiConvValid_16s_C1R,( const Ipp16s* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp16s* pSrc2, int src2Step, IppiSize src2Size,
        Ipp16s* pDst, int dstStep, int divisor ))
IPPAPI( IppStatus, ippiConvValid_16s_C3R,( const Ipp16s* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp16s* pSrc2, int src2Step, IppiSize src2Size,
        Ipp16s* pDst, int dstStep, int divisor ))
IPPAPI( IppStatus, ippiConvValid_16s_AC4R,( const Ipp16s* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp16s* pSrc2, int src2Step, IppiSize src2Size,
        Ipp16s* pDst, int dstStep, int divisor ))
IPPAPI( IppStatus, ippiConvValid_8u_C1R,( const Ipp8u* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp8u* pSrc2, int src2Step, IppiSize src2Size,
        Ipp8u* pDst, int dstStep, int divisor ))
IPPAPI( IppStatus, ippiConvValid_8u_C3R,( const Ipp8u* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp8u* pSrc2, int src2Step, IppiSize src2Size,
        Ipp8u* pDst, int dstStep, int divisor ))
IPPAPI( IppStatus, ippiConvValid_8u_AC4R,( const Ipp8u* pSrc1, int src1Step,
        IppiSize src1Size, const Ipp8u* pSrc2, int src2Step, IppiSize src2Size,
        Ipp8u* pDst, int dstStep, int divisor ))



/* //////////////////////////////////////////////////////////////////////////////////////
//                   Image Proximity Measures
////////////////////////////////////////////////////////////////////////////////////// */
/*///////////////////////////////////////////////////////////////////////////////////////
//  Names:
//      ippiCrossCorrFull_Norm_32f_C1R,         ippiCrossCorrSame_Norm_32f_C1R,
//      ippiCrossCorrFull_Norm_32f_C3R,         ippiCrossCorrSame_Norm_32f_C3R,
//      ippiCrossCorrFull_Norm_32f_C4R,         ippiCrossCorrSame_Norm_32f_C4R,
//      ippiCrossCorrFull_Norm_32f_AC4R,        ippiCrossCorrSame_Norm_32f_AC4R,
//      ippiCrossCorrFull_Norm_8u_C1RSfs,       ippiCrossCorrSame_Norm_8u_C1RSfs,
//      ippiCrossCorrFull_Norm_8u_C3RSfs,       ippiCrossCorrSame_Norm_8u_C3RSfs,
//      ippiCrossCorrFull_Norm_8u_C4RSfs,       ippiCrossCorrSame_Norm_8u_C4RSfs,
//      ippiCrossCorrFull_Norm_8u_AC4RSfs,      ippiCrossCorrSame_Norm_8u_AC4RSfs,
//      ippiCrossCorrFull_Norm_8u32f_C1R,       ippiCrossCorrSame_Norm_8u32f_C1R,
//      ippiCrossCorrFull_Norm_8u32f_C3R,       ippiCrossCorrSame_Norm_8u32f_C3R,
//      ippiCrossCorrFull_Norm_8u32f_C4R,       ippiCrossCorrSame_Norm_8u32f_C4R,
//      ippiCrossCorrFull_Norm_8u32f_AC4R,      ippiCrossCorrSame_Norm_8u32f_AC4R,
//      ippiCrossCorrFull_Norm_8s32f_C1R,       ippiCrossCorrSame_Norm_8s32f_C1R,
//      ippiCrossCorrFull_Norm_8s32f_C3R,       ippiCrossCorrSame_Norm_8s32f_C3R,
//      ippiCrossCorrFull_Norm_8s32f_C4R,       ippiCrossCorrSame_Norm_8s32f_C4R,
//      ippiCrossCorrFull_Norm_8s32f_AC4R,      ippiCrossCorrSame_Norm_8s32f_AC4R,
//      ippiCrossCorrFull_Norm_16u32f_C1R,      ippiCrossCorrSame_Norm_16u32f_C1R,
//      ippiCrossCorrFull_Norm_16u32f_C3R,      ippiCrossCorrSame_Norm_16u32f_C3R,
//      ippiCrossCorrFull_Norm_16u32f_C4R,      ippiCrossCorrSame_Norm_16u32f_C4R,
//      ippiCrossCorrFull_Norm_16u32f_AC4R,     ippiCrossCorrSame_Norm_16u32f_AC4R,
//
//      ippiCrossCorrValid_Norm_32f_C1R,
//      ippiCrossCorrValid_Norm_32f_C3R,
//      ippiCrossCorrValid_Norm_32f_C4R,
//      ippiCrossCorrValid_Norm_32f_AC4R,
//      ippiCrossCorrValid_Norm_8u_C1RSfs,
//      ippiCrossCorrValid_Norm_8u_C3RSfs,
//      ippiCrossCorrValid_Norm_8u_C4RSfs,
//      ippiCrossCorrValid_Norm_8u_AC4RSfs,
//      ippiCrossCorrValid_Norm_8u32f_C1R,
//      ippiCrossCorrValid_Norm_8u32f_C3R,
//      ippiCrossCorrValid_Norm_8u32f_C4R,
//      ippiCrossCorrValid_Norm_8u32f_AC4R.
//      ippiCrossCorrValid_Norm_8s32f_C1R,
//      ippiCrossCorrValid_Norm_8s32f_C3R,
//      ippiCrossCorrValid_Norm_8s32f_C4R,
//      ippiCrossCorrValid_Norm_8s32f_AC4R.
//
//  Purpose: Computes normalized cross-correlation between
//           an image and a template (another image).
//           The cross-correlation values are the image similarity measures: the
//           higher cross-correlation at a particular pixel, the more
//           similarity between the template and the image in the neighborhood
//           of the pixel. If IppiSize's of image and template are Wa * Ha and
//           Wb * Hb correspondingly, then the IppiSize of the resulting
//           matrix with normalized cross-correlation coefficients will be
//           a) in case of 'Full' suffix:
//              ( Wa + Wb - 1 )*( Ha + Hb - 1 ).
//           b) in case of 'Same' suffix:
//              ( Wa )*( Ha ).
//           c) in case of 'Valid' suffix:
//              ( Wa - Wb + 1 )*( Ha - Hb + 1 ).
//  Notice:
//           suffix 'R' (ROI) means only scanline alignment (srcStep), in
//           'Same' and 'Full' cases no any requirements for data outside
//           the ROI - it's assumed that template and source images are zero padded.
//
//  Parameters:
//      pSrc        Pointer to the source image ROI;
//      srcStep     Step in bytes through the source image buffer;
//      srcRoiSize  Size of the source ROI in pixels;
//      pTpl        Pointer to the template ( feature ) image ROI;
//      tplStep     Step in bytes through the template image buffer;
//      tplRoiSize  Size of the template ROI in pixels;
//      pDst        Pointer to the destination buffer;
//      dstStep     Step in bytes through the destination image buffer;
//      scaleFactor Scale factor value ( integer output data ).
//
//  Returns:
//   ippStsNoErr        OK
//   ippStsNullPtrErr   One of the pointers to pSrc, pDst or pTpl is NULL;
//   ippStsSizeErr      srcRoiSize or tplRoiSize has a field with zero or
//                      negative value,
//                      or srcRoiSize has a field with value smaller than value
//                      of the corresponding field of tplRoiSize;
//   ippStsStepErr      One of the step values is less than or equal to zero;
//   ippStsMemAllocErr  Memory allocation for internal buffers fails.
*/

IPPAPI( IppStatus, ippiCrossCorrFull_Norm_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_32f_C3R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_32f_AC4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_8u32f_C3R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_8u32f_AC4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_8u_C1RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_8u_C3RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_8u_AC4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_16u32f_C1R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_16u32f_C3R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_16u32f_AC4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_16u32f_C4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_32f_C4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_8u32f_C4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_8s32f_C1R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_8s32f_C3R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_8s32f_C4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_8s32f_AC4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_Norm_8u_C4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))

IPPAPI( IppStatus, ippiCrossCorrValid_Norm_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_32f_C3R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_32f_AC4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_8u32f_C3R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_8u32f_AC4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_8u_C1RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_8u_C3RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_8u_AC4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_32f_C4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_8u32f_C4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_8s32f_C1R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_8s32f_C3R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_8s32f_C4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_8s32f_AC4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_8u_C4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_16u32f_C1R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_16u32f_C3R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_16u32f_AC4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_16u32f_C4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))

IPPAPI( IppStatus, ippiCrossCorrSame_Norm_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_32f_C3R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_32f_AC4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_8u32f_C3R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_8u32f_AC4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_8u_C1RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_8u_C3RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_8u_AC4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_32f_C4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_8u32f_C4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_8s32f_C1R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_8s32f_C3R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_8s32f_C4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_8s32f_AC4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_8u_C4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_16u32f_C1R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_16u32f_C3R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_16u32f_AC4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_Norm_16u32f_C4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))

/* /////////////////////////////////////////////////////////////////////////////////
//  Names:
//      ippiCrossCorrFull_NormLevel_32f_C1R,    ippiCrossCorrSame_NormLevel_32f_C1R,
//      ippiCrossCorrFull_NormLevel_32f_C3R,    ippiCrossCorrSame_NormLevel_32f_C3R,
//      ippiCrossCorrFull_NormLevel_32f_C4R,    ippiCrossCorrSame_NormLevel_32f_C4R,
//      ippiCrossCorrFull_NormLevel_32f_AC4R,   ippiCrossCorrSame_NormLevel_32f_AC4R,
//      ippiCrossCorrFull_NormLevel_8u_C1RSfs,  ippiCrossCorrSame_NormLevel_8u_C1RSfs,
//      ippiCrossCorrFull_NormLevel_8u_C3RSfs,  ippiCrossCorrSame_NormLevel_8u_C3RSfs,
//      ippiCrossCorrFull_NormLevel_8u_C4RSfs,  ippiCrossCorrSame_NormLevel_8u_C4RSfs,
//      ippiCrossCorrFull_NormLevel_8u_AC4RSfs, ippiCrossCorrSame_NormLevel_8u_AC4RSfs,
//      ippiCrossCorrFull_NormLevel_8u32f_C1R,  ippiCrossCorrSame_NormLevel_8u32f_C1R,
//      ippiCrossCorrFull_NormLevel_8u32f_C3R,  ippiCrossCorrSame_NormLevel_8u32f_C3R,
//      ippiCrossCorrFull_NormLevel_8u32f_C4R,  ippiCrossCorrSame_NormLevel_8u32f_C4R,
//      ippiCrossCorrFull_NormLevel_8u32f_AC4R, ippiCrossCorrSame_NormLevel_8u32f_AC4R,
//      ippiCrossCorrFull_NormLevel_8s32f_C1R,  ippiCrossCorrSame_NormLevel_8s32f_C1R,
//      ippiCrossCorrFull_NormLevel_8s32f_C3R,  ippiCrossCorrSame_NormLevel_8s32f_C3R,
//      ippiCrossCorrFull_NormLevel_8s32f_C4R,  ippiCrossCorrSame_NormLevel_8s32f_C4R,
//      ippiCrossCorrFull_NormLevel_8s32f_AC4R, ippiCrossCorrSame_NormLevel_8s32f_AC4R,
//      ippiCrossCorrFull_NormLevel_16u32f_C1R, ippiCrossCorrSame_NormLevel_16u32f_C1R,
//      ippiCrossCorrFull_NormLevel_16u32f_C3R, ippiCrossCorrSame_NormLevel_16u32f_C3R,
//      ippiCrossCorrFull_NormLevel_16u32f_C4R, ippiCrossCorrSame_NormLevel_16u32f_C4R,
//      ippiCrossCorrFull_NormLevel_16u32f_AC4R,ippiCrossCorrSame_NormLevel_16u32f_AC4R,
//
//      ippiCrossCorrValid_NormLevel_32f_C1R,
//      ippiCrossCorrValid_NormLevel_32f_C3R,
//      ippiCrossCorrValid_NormLevel_32f_C4R,
//      ippiCrossCorrValid_NormLevel_32f_AC4R,
//      ippiCrossCorrValid_NormLevel_8u_C1RSfs,
//      ippiCrossCorrValid_NormLevel_8u_C3RSfs,
//      ippiCrossCorrValid_NormLevel_8u_C4RSfs,
//      ippiCrossCorrValid_NormLevel_8u_AC4RSfs,
//      ippiCrossCorrValid_NormLevel_8u32f_C1R,
//      ippiCrossCorrValid_NormLevel_8u32f_C3R,
//      ippiCrossCorrValid_NormLevel_8u32f_C4R,
//      ippiCrossCorrValid_NormLevel_8u32f_AC4R,
//      ippiCrossCorrValid_NormLevel_8s32f_C1R,
//      ippiCrossCorrValid_NormLevel_8s32f_C3R,
//      ippiCrossCorrValid_NormLevel_8s32f_C4R,
//      ippiCrossCorrValid_NormLevel_8s32f_AC4R.
//      ippiCrossCorrValid_NormLevel_16u32f_C1R,
//      ippiCrossCorrValid_NormLevel_16u32f_C3R,
//      ippiCrossCorrValid_NormLevel_16u32f_C4R,
//      ippiCrossCorrValid_NormLevel_16u32f_AC4R.
//
//  Purpose: Computes normalized correlation coefficient between an image
//           and a template.
//           ippiCrossCorr_NormLevel() function allows you to compute the
//           cross-correlation of an image and a template (another image).
//           The cross-correlation values are image similarity measures: the
//           higher cross-correlation at a particular pixel, the more
//           similarity between the template and the image in the neighborhood
//           of the pixel. If IppiSize's of image and template are Wa * Ha and
//           Wb * Hb correspondingly, then the IppiSize of the resulting
//           matrix with normalized cross-correlation coefficients will be
//           a) in case of 'Full' suffix:
//              ( Wa + Wb - 1 )*( Ha + Hb - 1 ).
//           b) in case of 'Same' suffix:
//              ( Wa )*( Ha ).
//           c) in case of 'Valid' suffix:
//              ( Wa - Wb + 1 )*( Ha - Hb + 1 ).
//  Notice:
//           suffix 'R' (ROI) means only scanline alignment (srcStep), in
//           'Same' and 'Full' cases no any requirements for data outstand
//           the ROI - it's assumes that template and src are zero padded.
//           The difference from ippiCrossCorr_Norm() functions is the using
//           of Zero Mean image and Template to avoid brightness impact.
//           (Before the calculation of the cross-correlation coefficients,
//           the mean of the image in the region under the feature is subtracted
//           from every image pixel; the same for the template.)
//
//  Parameters:
//      pSrc        Pointer to the source image ROI;
//      srcStep     Step in bytes through the source image buffer;
//      srcRoiSize  Size of the source ROI in pixels;
//      pTpl        Pointer to the template ( feature ) image ROI;
//      tplStep     Step in bytes through the template image buffer;
//      tplRoiSize  Size of the template ROI in pixels;
//      pDst        Pointer to the destination buffer;
//      dstStep     Step in bytes through the destination image buffer;
//      scaleFactor Scale factor value ( integer output data ).
//
//  Returns:
//   ippStsNoErr        OK
//   ippStsNullPtrErr   One of the pointers to pSrc, pDst or pTpl is NULL;
//   ippStsSizeErr      srcRoiSize or tplRoiSize has a field with zero or
//                      negative value,
//                      or srcRoiSize has a field with value smaller than value
//                      of the corresponding field of tplRoiSize;
//   ippStsStepErr      One of the step values is less than or equal to zero;
//   ippStsMemAllocErr  Memory allocation for internal buffers fails.
*/

IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_32f_C3R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_32f_C4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_32f_AC4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_8u32f_C3R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_8u32f_C4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_8u32f_AC4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_8s32f_C1R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_8s32f_C3R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_8s32f_C4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_8s32f_AC4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_8u_C1RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_8u_C3RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_8u_C4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_8u_AC4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_16u32f_C1R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_16u32f_C3R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_16u32f_AC4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrFull_NormLevel_16u32f_C4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))

IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_32f_C3R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_32f_C4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_32f_AC4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_8u32f_C3R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_8u32f_C4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_8u32f_AC4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_8s32f_C1R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_8s32f_C3R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_8s32f_C4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_8s32f_AC4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_8u_C1RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_8u_C3RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_8u_C4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_8u_AC4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_16u32f_C1R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_16u32f_C3R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_16u32f_AC4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_16u32f_C4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))

IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_32f_C3R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_32f_C4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_32f_AC4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_8u32f_C3R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_8u32f_C4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_8u32f_AC4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_8s32f_C1R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_8s32f_C3R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_8s32f_C4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_8s32f_AC4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_8u_C1RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_8u_C3RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_8u_C4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_8u_AC4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_16u32f_C1R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_16u32f_C3R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_16u32f_AC4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrSame_NormLevel_16u32f_C4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))


/* //////////////////////////////////////////////////////////////////////////////////
//  Names:
//      ippiSqrDistanceFull_Norm_32f_C1R,    ippiSqrDistanceSame_Norm_32f_C1R,
//      ippiSqrDistanceFull_Norm_32f_C3R,    ippiSqrDistanceSame_Norm_32f_C3R,
//      ippiSqrDistanceFull_Norm_32f_C4R,    ippiSqrDistanceSame_Norm_32f_C4R,
//      ippiSqrDistanceFull_Norm_32f_AC4R,   ippiSqrDistanceSame_Norm_32f_AC4R,
//      ippiSqrDistanceFull_Norm_8u_C1RSfs,  ippiSqrDistanceSame_Norm_8u_C1RSfs,
//      ippiSqrDistanceFull_Norm_8u_C3RSfs,  ippiSqrDistanceSame_Norm_8u_C3RSfs,
//      ippiSqrDistanceFull_Norm_8u_C4RSfs,  ippiSqrDistanceSame_Norm_8u_C4RSfs,
//      ippiSqrDistanceFull_Norm_8u_AC4RSfs, ippiSqrDistanceSame_Norm_8u_AC4RSfs,
//      ippiSqrDistanceFull_Norm_8u32f_C1R,  ippiSqrDistanceSame_Norm_8u32f_C1R,
//      ippiSqrDistanceFull_Norm_8u32f_C3R,  ippiSqrDistanceSame_Norm_8u32f_C3R,
//      ippiSqrDistanceFull_Norm_8u32f_C4R,  ippiSqrDistanceSame_Norm_8u32f_C4R,
//      ippiSqrDistanceFull_Norm_8u32f_AC4R, ippiSqrDistanceSame_Norm_8u32f_AC4R,
//      ippiSqrDistanceFull_Norm_8s32f_C1R,  ippiSqrDistanceSame_Norm_8s32f_C1R,
//      ippiSqrDistanceFull_Norm_8s32f_C3R,  ippiSqrDistanceSame_Norm_8s32f_C3R,
//      ippiSqrDistanceFull_Norm_8s32f_C4R,  ippiSqrDistanceSame_Norm_8s32f_C4R,
//      ippiSqrDistanceFull_Norm_8s32f_AC4R, ippiSqrDistanceSame_Norm_8s32f_AC4R,
//      ippiSqrDistanceFull_Norm_16u32f_C1R, ippiSqrDistanceSame_Norm_16u32f_C1R,
//      ippiSqrDistanceFull_Norm_16u32f_C3R, ippiSqrDistanceSame_Norm_16u32f_C3R,
//      ippiSqrDistanceFull_Norm_16u32f_C4R, ippiSqrDistanceSame_Norm_16u32f_C4R,
//      ippiSqrDistanceFull_Norm_16u32f_AC4R,ippiSqrDistanceSame_Norm_16u32f_AC4R,
//
//      ippiSqrDistanceValid_Norm_32f_C1R,
//      ippiSqrDistanceValid_Norm_32f_C3R,
//      ippiSqrDistanceValid_Norm_32f_C4R,
//      ippiSqrDistanceValid_Norm_32f_AC4R,
//      ippiSqrDistanceValid_Norm_8u_C1RSfs,
//      ippiSqrDistanceValid_Norm_8u_C3RSfs,
//      ippiSqrDistanceValid_Norm_8u_C4RSfs,
//      ippiSqrDistanceValid_Norm_8u_AC4RSfs,
//      ippiSqrDistanceValid_Norm_8u32f_C1R,
//      ippiSqrDistanceValid_Norm_8u32f_C3R,
//      ippiSqrDistanceValid_Norm_8u32f_C4R,
//      ippiSqrDistanceValid_Norm_8u32f_AC4R.
//      ippiSqrDistanceValid_Norm_16u32f_C1R,
//      ippiSqrDistanceValid_Norm_16u32f_C3R,
//      ippiSqrDistanceValid_Norm_16u32f_C4R,
//      ippiSqrDistanceValid_Norm_16u32f_AC4R.
//      ippiSqrDistanceValid_Norm_8s32f_C1R,
//      ippiSqrDistanceValid_Norm_8s32f_C3R,
//      ippiSqrDistanceValid_Norm_8s32f_C4R,
//      ippiSqrDistanceValid_Norm_8s32f_AC4R.
//
//  Purpose: Computes normalized Euclidean distance, or Sum of Squared
//           Distance (SSD) of an image and a template (another image).
//               The SSD values are image similarity measures: the smaller
//           value of SSD at a particular pixel, the more similarity between
//           the template and the image in the neighborhood of the pixel.
//               If IppiSize's of image and template are Wa * Ha and
//           Wb * Hb correspondingly, then the IppiSize of the resulting
//           matrix with normalized SSD coefficients will be
//           a) in case of 'Full' suffix:
//              ( Wa + Wb - 1 )*( Ha + Hb - 1 ).
//           b) in case of 'Same' suffix:
//              ( Wa )*( Ha ).
//           c) in case of 'Valid' suffix:
//              ( Wa - Wb + 1 )*( Ha - Hb + 1 ).
//  Notice:
//           suffix 'R' (ROI) means only scanline alignment (srcStep), in
//           'Same' and 'Full' cases no any requirements for data outstand
//           the ROI - it's assumed that template and source images are zero padded.
//
//  Parameters:
//      pSrc        Pointer to the source image ROI;
//      srcStep     Step in bytes through the source image buffer;
//      srcRoiSize  Size of the source ROI in pixels;
//      pTpl        Pointer to the template ( feature ) image ROI;
//      tplStep     Step in bytes through the template image buffer;
//      tplRoiSize  Size of the template ROI in pixels;
//      pDst        Pointer to the destination buffer;
//      dstStep     Step in bytes through the destination image buffer;
//      scaleFactor Scale factor value ( integer output data ).
//
//  Returns:
//   ippStsNoErr        OK
//   ippStsNullPtrErr   One of the pointers to pSrc, pDst or pTpl is NULL;
//   ippStsSizeErr      srcRoiSize or tplRoiSize has a field with zero or
//                      negative value,
//                      or srcRoiSize has a field with value smaller than value
//                      of the corresponding field of tplRoiSize;
//   ippStsStepErr      One of the step values is less than or equal to zero;
//   ippStsMemAllocErr  Memory allocation for internal buffers fails.
*/

IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_32f_C3R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_32f_AC4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_8u32f_C3R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_8u32f_AC4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_8u_C1RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_8u_C3RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_8u_AC4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))

IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_32f_C3R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_32f_AC4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_8u32f_C3R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_8u32f_AC4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_8u_C1RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_8u_C3RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_8u_AC4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))

IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_32f_C1R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_32f_C3R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_32f_AC4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_8u32f_C1R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_8u32f_C3R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_8u32f_AC4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_8u_C1RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_8u_C3RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_8u_AC4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))

IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_32f_C4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_8u32f_C4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_8s32f_C1R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_8s32f_C3R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_8s32f_C4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_8s32f_AC4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_8u_C4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_16u32f_C1R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_16u32f_C3R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_16u32f_AC4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceFull_Norm_16u32f_C4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))

IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_32f_C4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_8u32f_C4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_8s32f_C1R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_8s32f_C3R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_8s32f_C4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_8s32f_AC4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_8u_C4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_16u32f_C1R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_16u32f_C3R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_16u32f_AC4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_16u32f_C4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))

IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_32f_C4R, ( const Ipp32f* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_8u32f_C4R, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_8s32f_C1R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_8s32f_C3R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_8s32f_C4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_8s32f_AC4R, ( const Ipp8s* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_8u_C4RSfs, ( const Ipp8u* pSrc, int srcStep,
        IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep, IppiSize tplRoiSize,
        Ipp8u* pDst, int dstStep, int scaleFactor ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_16u32f_C1R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_16u32f_C3R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_16u32f_AC4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiSqrDistanceSame_Norm_16u32f_C4R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
/*
///////////////////////   Version without normalization ///////////////////////
//
//  Names:
//      ippiCrossCorrValid_32f_C1R
//      ippiCrossCorrValid_8u32f_C1R
//      ippiCrossCorrValid_8s32f_C1R
//      ippiCrossCorrValid_16u32f_C1R
//
//  Arguments:
//      pSrc        - pointer to the source image ROI;
//      srcStep     - step in bytes through the source image buffer;
//      srcRoiSize  - size of the source ROI in pixels;
//      pTpl        - pointer to the template ( feature ) image ROI;
//      tplStep     - step in bytes through the template image buffer;
//      tplRoiSize  - size of the template ROI in pixels;
//      pDst        - pointer to the destination buffer;
//      dstStep     - step in bytes through the destination image buffer;
//
//  Return:
//   ippStsNoErr        - Ok
//   ippStsNullPtrErr   - at least one of the pointers to pSrc, pDst or pTpl is NULL;
//   ippStsSizeErr      - at least one of the sizes of srcRoiSize or tplRoiSize is less or equal zero,
//                        or at least one of the sizes of srcRoiSize is smaller than the corresponding
//                        size of the tplRoiSize;
//   ippStsStepErr      - at least one of the srcStep, tplStep or dstStep is less or equal zero;
//   ippStsMemAllocErr  - an error occurs during allocation memory for internal buffers.
//
//  Purpose: ippiCrossCorr() function allows you to compute the
//           cross-correlation of an image and a template (another image).
//           The cross-correlation values are image similarity measures: the
//           higher cross-correlation at a particular pixel, the more
//           similarity between the template and the image in the neighborhood
//           of the pixel.
//
//           If IppiSize's of image and template are Wa * Ha and
//           Wb * Hb correspondingly, then the IppiSize of the resulting
//           matrice with coefficients will be
//
//                      ( Wa - Wb + 1 )*( Ha - Hb + 1 )
//
//           ('Valid' case in MatLab notation)
//  Notice:
//           suffix 'R' (ROI) means only scanline alingment (srcStep), in
//           'Same' and 'Full' cases no any requirements for data outstand
//           the ROI - it's assumes that template and src are zero padded.
*/
IPPAPI( IppStatus, ippiCrossCorrValid_32f_C1R, ( const Ipp32f* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp32f* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_8u32f_C1R, ( const Ipp8u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp8u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_8s32f_C1R, ( const Ipp8s* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp8s* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
IPPAPI( IppStatus, ippiCrossCorrValid_16u32f_C1R, ( const Ipp16u* pSrc,
        int srcStep, IppiSize srcRoiSize, const Ipp16u* pTpl, int tplStep,
        IppiSize tplRoiSize, Ipp32f* pDst, int dstStep ))
/* /////////////////////////////////////////////////////////////////////////////
//                   Threshold operations
///////////////////////////////////////////////////////////////////////////// */

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiThreshold_8u_C1R
//              ippiThreshold_8u_C3R
//              ippiThreshold_8u_AC4R
//              ippiThreshold_16s_C1R
//              ippiThreshold_16s_C3R
//              ippiThreshold_16s_AC4R
//              ippiThreshold_32f_C1R
//              ippiThreshold_32f_C3R
//              ippiThreshold_32f_AC4R
//              ippiThreshold_8u_C1IR
//              ippiThreshold_8u_C3IR
//              ippiThreshold_8u_AC4IR
//              ippiThreshold_16s_C1IR
//              ippiThreshold_16s_C3IR
//              ippiThreshold_16s_AC4IR
//              ippiThreshold_32f_C1IR
//              ippiThreshold_32f_C3IR
//              ippiThreshold_32f_AC4IR
//              ippiThreshold_16u_C1R
//              ippiThreshold_16u_C3R
//              ippiThreshold_16u_AC4R
//              ippiThreshold_16u_C1IR
//              ippiThreshold_16u_C3IR
//              ippiThreshold_16u_AC4IR
//
//  Purpose:    Performs thresholding of an image using the specified level

//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of the pointers is NULL
//   ippStsSizeErr     roiSize has a field with zero or negative value
//   ippStsStepErr     One of the step values is zero or negative
//
//  Parameters:
//   pSrc       Pointer to the source image
//   srcStep    Step through the source image
//   pDst       Pointer to the destination image
//   dstStep    Step through the destination image
//   pSrcDst    Pointer to the source/destination image (in-place flavors)
//   srcDstStep Step through the source/destination image (in-place flavors)
//   roiSize    Size of the ROI
//   threshold  Threshold level value (array of values for multi-channel data)
//   ippCmpOp   Comparison mode, possible values:
//                ippCmpLess     - less than,
//                ippCmpGreater  - greater than
*/
IPPAPI(IppStatus,ippiThreshold_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp8u threshold,
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, Ipp16s threshold,
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f threshold,
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_8u_C1IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp8u threshold, IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_16s_C1IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16s threshold, IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp32f threshold, IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_8u_C3IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_16s_C3IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_32f_C3IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_8u_AC4IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_16s_AC4IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, Ipp16u threshold,
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_16u_C1IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16u threshold, IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_16u_C3IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[3], IppCmpOp ippCmpOp))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiThreshold_GT_8u_C1R
//              ippiThreshold_GT_8u_C3R
//              ippiThreshold_GT_8u_AC4R
//              ippiThreshold_GT_16s_C1R
//              ippiThreshold_GT_16s_C3R
//              ippiThreshold_GT_16s_AC4R
//              ippiThreshold_GT_32f_C1R
//              ippiThreshold_GT_32f_C3R
//              ippiThreshold_GT_32f_AC4R
//              ippiThreshold_GT_8u_C1IR
//              ippiThreshold_GT_8u_C3IR
//              ippiThreshold_GT_8u_AC4IR
//              ippiThreshold_GT_16s_C1IR
//              ippiThreshold_GT_16s_C3IR
//              ippiThreshold_GT_16s_AC4IR
//              ippiThreshold_GT_32f_C1IR
//              ippiThreshold_GT_32f_C3IR
//              ippiThreshold_GT_32f_AC4IR
//              ippiThreshold_GT_16u_C1R
//              ippiThreshold_GT_16u_C3R
//              ippiThreshold_GT_16u_AC4R
//              ippiThreshold_GT_16u_C1IR
//              ippiThreshold_GT_16u_C3IR
//              ippiThreshold_GT_16u_AC4IR
//
//  Purpose:   Performs threshold operation using the comparison "greater than"
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of the pointers is NULL
//   ippStsSizeErr     roiSize has a field with zero or negative value
//   ippStsStepErr     One of the step values is zero or negative
//
//  Parameters:
//   pSrc       Pointer to the source image
//   srcStep    Step through the source image
//   pDst       Pointer to the destination image
//   dstStep    Step through the destination image
//   pSrcDst    Pointer to the source/destination image (in-place flavors)
//   srcDstStep Step through the source/destination image (in-place flavors)
//   roiSize    Size of the ROI
//   threshold  Threshold level value (array of values for multi-channel data)
*/
IPPAPI(IppStatus,ippiThreshold_GT_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp8u threshold))
IPPAPI(IppStatus,ippiThreshold_GT_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, Ipp16s threshold))
IPPAPI(IppStatus,ippiThreshold_GT_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f threshold))
IPPAPI(IppStatus,ippiThreshold_GT_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_8u_C1IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp8u threshold))
IPPAPI(IppStatus,ippiThreshold_GT_16s_C1IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16s threshold))
IPPAPI(IppStatus,ippiThreshold_GT_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp32f threshold))
IPPAPI(IppStatus,ippiThreshold_GT_8u_C3IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_16s_C3IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_32f_C3IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_8u_AC4IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_16s_AC4IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, Ipp16u threshold))
IPPAPI(IppStatus,ippiThreshold_GT_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_16u_C1IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16u threshold))
IPPAPI(IppStatus,ippiThreshold_GT_16u_C3IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_GT_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[3]))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiThreshold_LT_8u_C1R
//              ippiThreshold_LT_8u_C3R
//              ippiThreshold_LT_8u_AC4R
//              ippiThreshold_LT_16s_C1R
//              ippiThreshold_LT_16s_C3R
//              ippiThreshold_LT_16s_AC4R
//              ippiThreshold_LT_32f_C1R
//              ippiThreshold_LT_32f_C3R
//              ippiThreshold_LT_32f_AC4R
//              ippiThreshold_LT_8u_C1IR
//              ippiThreshold_LT_8u_C3IR
//              ippiThreshold_LT_8u_AC4IR
//              ippiThreshold_LT_16s_C1IR
//              ippiThreshold_LT_16s_C3IR
//              ippiThreshold_LT_16s_AC4IR
//              ippiThreshold_LT_32f_C1IR
//              ippiThreshold_LT_32f_C3IR
//              ippiThreshold_LT_32f_AC4IR
//              ippiThreshold_LT_16u_C1R
//              ippiThreshold_LT_16u_C3R
//              ippiThreshold_LT_16u_AC4R
//              ippiThreshold_LT_16u_C1IR
//              ippiThreshold_LT_16u_C3IR
//              ippiThreshold_LT_16u_AC4IR
//
//  Purpose:  Performs threshold operation using the comparison "less than"
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of the pointers is NULL
//   ippStsSizeErr     roiSize has a field with zero or negative value
//   ippStsStepErr     One of the step values is zero or negative
//
//  Parameters:
//   pSrc       Pointer to the source image
//   srcStep    Step through the source image
//   pDst       Pointer to the destination image
//   dstStep    Step through the destination image
//   pSrcDst    Pointer to the source/destination image (in-place flavors)
//   srcDstStep Step through the source/destination image (in-place flavors)
//   roiSize    Size of the ROI
//   threshold  Threshold level value (array of values for multi-channel data)
//   ippCmpOp   Comparison mode, possible values:
//                ippCmpLess     - less than
//                ippCmpGreater  - greater than
*/
IPPAPI(IppStatus,ippiThreshold_LT_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp8u threshold))
IPPAPI(IppStatus,ippiThreshold_LT_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, Ipp16s threshold))
IPPAPI(IppStatus,ippiThreshold_LT_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f threshold))
IPPAPI(IppStatus,ippiThreshold_LT_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_8u_C1IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp8u threshold))
IPPAPI(IppStatus,ippiThreshold_LT_16s_C1IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16s threshold))
IPPAPI(IppStatus,ippiThreshold_LT_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp32f threshold))
IPPAPI(IppStatus,ippiThreshold_LT_8u_C3IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_16s_C3IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_32f_C3IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_8u_AC4IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_16s_AC4IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, Ipp16u threshold))
IPPAPI(IppStatus,ippiThreshold_LT_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_16u_C1IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16u threshold))
IPPAPI(IppStatus,ippiThreshold_LT_16u_C3IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[3]))
IPPAPI(IppStatus,ippiThreshold_LT_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[3]))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiThreshold_Val_8u_C1R
//              ippiThreshold_Val_8u_C3R
//              ippiThreshold_Val_8u_AC4R
//              ippiThreshold_Val_16s_C1R
//              ippiThreshold_Val_16s_C3R
//              ippiThreshold_Val_16s_AC4R
//              ippiThreshold_Val_32f_C1R
//              ippiThreshold_Val_32f_C3R
//              ippiThreshold_Val_32f_AC4R
//              ippiThreshold_Val_8u_C1IR
//              ippiThreshold_Val_8u_C3IR
//              ippiThreshold_Val_8u_AC4IR
//              ippiThreshold_Val_16s_C1IR
//              ippiThreshold_Val_16s_C3IR
//              ippiThreshold_Val_16s_AC4IR
//              ippiThreshold_Val_32f_C1IR
//              ippiThreshold_Val_32f_C3IR
//              ippiThreshold_Val_32f_AC4IR
//              ippiThreshold_Val_16u_C1R
//              ippiThreshold_Val_16u_C3R
//              ippiThreshold_Val_16u_AC4R
//              ippiThreshold_Val_16u_C1IR
//              ippiThreshold_Val_16u_C3IR
//              ippiThreshold_Val_16u_AC4IR
//
//  Purpose:  Performs thresholding of pixel values: pixels that satisfy
//            the compare conditions are set to a specified value
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of the pointers is NULL
//   ippStsSizeErr     roiSize has a field with zero or negative value
//   ippStsStepErr     One of the step values is zero or negative
//
//  Parameters:
//   pSrc       Pointer to the source image
//   srcStep    Step through the source image
//   pDst       Pointer to the destination image
//   dstStep    Step through the destination image
//   pSrcDst    Pointer to the source/destination image (in-place flavors)
//   srcDstStep Step through the source/destination image (in-place flavors)
//   roiSize    Size of the ROI
//   threshold  Threshold level value (array of values for multi-channel data)
//   value      The output value (array or values for multi-channel data)
//   ippCmpOp      comparison mode, ippCmpLess or ippCmpGreater
*/
IPPAPI(IppStatus,ippiThreshold_Val_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp8u threshold,
       Ipp8u value, IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, Ipp16s threshold,
       Ipp16s value, IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f threshold,
       Ipp32f value, IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[3],
       const Ipp8u value[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[3],
       const Ipp16s value[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[3],
       const Ipp32f value[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[3],
       const Ipp8u value[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[3],
       const Ipp16s value[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[3],
       const Ipp32f value[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_8u_C1IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp8u threshold, Ipp8u value, IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_16s_C1IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16s threshold, Ipp16s value, IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp32f threshold, Ipp32f value, IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_8u_C3IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[3], const Ipp8u value[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_16s_C3IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[3], const Ipp16s value[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_32f_C3IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[3], const Ipp32f value[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_8u_AC4IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[3], const Ipp8u value[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_16s_AC4IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[3], const Ipp16s value[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[3], const Ipp32f value[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, Ipp16u threshold,
       Ipp16u value, IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[3],
       const Ipp16u value[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[3],
       const Ipp16u value[3], IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_16u_C1IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16u threshold, Ipp16u value, IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_16u_C3IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[3], const Ipp16u value[3],
       IppCmpOp ippCmpOp))
IPPAPI(IppStatus,ippiThreshold_Val_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[3], const Ipp16u value[3],
       IppCmpOp ippCmpOp))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiThreshold_GTVal_8u_C1R
//              ippiThreshold_GTVal_8u_C3R
//              ippiThreshold_GTVal_8u_AC4R
//              ippiThreshold_GTVal_16s_C1R
//              ippiThreshold_GTVal_16s_C3R
//              ippiThreshold_GTVal_16s_AC4R
//              ippiThreshold_GTVal_32f_C1R
//              ippiThreshold_GTVal_32f_C3R
//              ippiThreshold_GTVal_32f_AC4R
//              ippiThreshold_GTVal_8u_C1IR
//              ippiThreshold_GTVal_8u_C3IR
//              ippiThreshold_GTVal_8u_AC4IR
//              ippiThreshold_GTVal_16s_C1IR
//              ippiThreshold_GTVal_16s_C3IR
//              ippiThreshold_GTVal_16s_AC4IR
//              ippiThreshold_GTVal_32f_C1IR
//              ippiThreshold_GTVal_32f_C3IR
//              ippiThreshold_GTVal_32f_AC4IR
//              ippiThreshold_GTVal_8u_C4R
//              ippiThreshold_GTVal_16s_C4R
//              ippiThreshold_GTVal_32f_C4R
//              ippiThreshold_GTVal_8u_C4IR
//              ippiThreshold_GTVal_16s_C4IR
//              ippiThreshold_GTVal_32f_C4IR
//              ippiThreshold_GTVal_16u_C1R
//              ippiThreshold_GTVal_16u_C3R
//              ippiThreshold_GTVal_16u_AC4R
//              ippiThreshold_GTVal_16u_C1IR
//              ippiThreshold_GTVal_16u_C3IR
//              ippiThreshold_GTVal_16u_AC4IR
//              ippiThreshold_GTVal_16u_C4R
//              ippiThreshold_GTVal_16u_C4IR
//
//  Purpose:  Performs thresholding of pixel values: pixels that are
//            greater than threshold, are set to a specified value
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of the pointers is NULL
//   ippStsSizeErr     roiSize has a field with zero or negative value
//   ippStsStepErr     One of the step values is zero or negative
//
//  Parameters:
//   pSrc       Pointer to the source image
//   srcStep    Step through the source image
//   pDst       Pointer to the destination image
//   dstStep    Step through the destination image
//   pSrcDst    Pointer to the source/destination image (in-place flavors)
//   srcDstStep Step through the source/destination image (in-place flavors)
//   roiSize    Size of the ROI
//   threshold  Threshold level value (array of values for multi-channel data)
//   value      The output value (array or values for multi-channel data)
*/
IPPAPI(IppStatus,ippiThreshold_GTVal_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp8u threshold,
       Ipp8u value))
IPPAPI(IppStatus,ippiThreshold_GTVal_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, Ipp16s threshold,
       Ipp16s value))
IPPAPI(IppStatus,ippiThreshold_GTVal_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f threshold,
       Ipp32f value))
IPPAPI(IppStatus,ippiThreshold_GTVal_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[3],
       const Ipp8u value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[3],
       const Ipp16s value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[3],
       const Ipp32f value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[3],
       const Ipp8u value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[3],
       const Ipp16s value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[3],
       const Ipp32f value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_8u_C1IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp8u threshold, Ipp8u value))
IPPAPI(IppStatus,ippiThreshold_GTVal_16s_C1IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16s threshold, Ipp16s value))
IPPAPI(IppStatus,ippiThreshold_GTVal_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp32f threshold, Ipp32f value))
IPPAPI(IppStatus,ippiThreshold_GTVal_8u_C3IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[3], const Ipp8u value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_16s_C3IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[3], const Ipp16s value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_32f_C3IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[3], const Ipp32f value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_8u_AC4IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[3], const Ipp8u value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_16s_AC4IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[3], const Ipp16s value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[3], const Ipp32f value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[4],
       const Ipp8u value[4]))
IPPAPI(IppStatus,ippiThreshold_GTVal_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[4],
       const Ipp16s value[4]))
IPPAPI(IppStatus,ippiThreshold_GTVal_32f_C4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[4],
       const Ipp32f value[4]))
IPPAPI(IppStatus,ippiThreshold_GTVal_8u_C4IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[4], const Ipp8u value[4]))
IPPAPI(IppStatus,ippiThreshold_GTVal_16s_C4IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[4], const Ipp16s value[4]))
IPPAPI(IppStatus,ippiThreshold_GTVal_32f_C4IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[4], const Ipp32f value[4]))
IPPAPI(IppStatus,ippiThreshold_GTVal_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, Ipp16u threshold,
       Ipp16u value))
IPPAPI(IppStatus,ippiThreshold_GTVal_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[3],
       const Ipp16u value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[3],
       const Ipp16u value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_16u_C1IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16u threshold, Ipp16u value))
IPPAPI(IppStatus,ippiThreshold_GTVal_16u_C3IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[3], const Ipp16u value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[3], const Ipp16u value[3]))
IPPAPI(IppStatus,ippiThreshold_GTVal_16u_C4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[4],
       const Ipp16u value[4]))
IPPAPI(IppStatus,ippiThreshold_GTVal_16u_C4IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[4], const Ipp16u value[4]))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiThreshold_LTVal_8u_C1R
//              ippiThreshold_LTVal_8u_C3R
//              ippiThreshold_LTVal_8u_AC4R
//              ippiThreshold_LTVal_16s_C1R
//              ippiThreshold_LTVal_16s_C3R
//              ippiThreshold_LTVal_16s_AC4R
//              ippiThreshold_LTVal_32f_C1R
//              ippiThreshold_LTVal_32f_C3R
//              ippiThreshold_LTVal_32f_AC4R
//              ippiThreshold_LTVal_8u_C1IR
//              ippiThreshold_LTVal_8u_C3IR
//              ippiThreshold_LTVal_8u_AC4IR
//              ippiThreshold_LTVal_16s_C1IR
//              ippiThreshold_LTVal_16s_C3IR
//              ippiThreshold_LTVal_16s_AC4IR
//              ippiThreshold_LTVal_32f_C1IR
//              ippiThreshold_LTVal_32f_C3IR
//              ippiThreshold_LTVal_32f_AC4IR
//              ippiThreshold_LTVal_8u_C4R
//              ippiThreshold_LTVal_16s_C4R
//              ippiThreshold_LTVal_32f_C4R
//              ippiThreshold_LTVal_8u_C4IR
//              ippiThreshold_LTVal_16s_C4IR
//              ippiThreshold_LTVal_32f_C4IR
//              ippiThreshold_LTVal_16u_C1R
//              ippiThreshold_LTVal_16u_C3R
//              ippiThreshold_LTVal_16u_AC4R
//              ippiThreshold_LTVal_16u_C1IR
//              ippiThreshold_LTVal_16u_C3IR
//              ippiThreshold_LTVal_16u_AC4IR
//              ippiThreshold_LTVal_16u_C4R
//              ippiThreshold_LTVal_16u_C4IR
//
//  Purpose:  Performs thresholding of pixel values: pixels that are
//            less than threshold, are set to a specified value
//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  One of the pointers is NULL
//   ippStsSizeErr     roiSize has a field with zero or negative value
//   ippStsStepErr     One of the step values is zero or negative
//
//  Parameters:
//   pSrc       Pointer to the source image
//   srcStep    Step through the source image
//   pDst       Pointer to the destination image
//   dstStep    Step through the destination image
//   pSrcDst    Pointer to the source/destination image (in-place flavors)
//   srcDstStep Step through the source/destination image (in-place flavors)
//   roiSize    Size of the ROI
//   threshold  Threshold level value (array of values for multi-channel data)
//   value      The output value (array or values for multi-channel data)
*/
IPPAPI(IppStatus,ippiThreshold_LTVal_8u_C1R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp8u threshold,
       Ipp8u value))
IPPAPI(IppStatus,ippiThreshold_LTVal_16s_C1R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, Ipp16s threshold,
       Ipp16s value))
IPPAPI(IppStatus,ippiThreshold_LTVal_32f_C1R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f threshold,
       Ipp32f value))
IPPAPI(IppStatus,ippiThreshold_LTVal_8u_C3R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[3],
       const Ipp8u value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_16s_C3R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[3],
       const Ipp16s value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_32f_C3R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[3],
       const Ipp32f value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_8u_AC4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[3],
       const Ipp8u value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_16s_AC4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[3],
       const Ipp16s value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_32f_AC4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[3],
       const Ipp32f value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_8u_C1IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp8u threshold, Ipp8u value))
IPPAPI(IppStatus,ippiThreshold_LTVal_16s_C1IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16s threshold, Ipp16s value))
IPPAPI(IppStatus,ippiThreshold_LTVal_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp32f threshold, Ipp32f value))
IPPAPI(IppStatus,ippiThreshold_LTVal_8u_C3IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[3], const Ipp8u value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_16s_C3IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[3], const Ipp16s value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_32f_C3IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[3], const Ipp32f value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_8u_AC4IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[3], const Ipp8u value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_16s_AC4IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[3], const Ipp16s value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[3], const Ipp32f value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_8u_C4R,(const Ipp8u* pSrc, int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u threshold[4],
       const Ipp8u value[4]))
IPPAPI(IppStatus,ippiThreshold_LTVal_16s_C4R,(const Ipp16s* pSrc, int srcStep,
       Ipp16s* pDst, int dstStep, IppiSize roiSize, const Ipp16s threshold[4],
       const Ipp16s value[4]))
IPPAPI(IppStatus,ippiThreshold_LTVal_32f_C4R,(const Ipp32f* pSrc, int srcStep,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, const Ipp32f threshold[4],
       const Ipp32f value[4]))
IPPAPI(IppStatus,ippiThreshold_LTVal_8u_C4IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp8u threshold[4], const Ipp8u value[4]))
IPPAPI(IppStatus,ippiThreshold_LTVal_16s_C4IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s threshold[4], const Ipp16s value[4]))
IPPAPI(IppStatus,ippiThreshold_LTVal_32f_C4IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f threshold[4], const Ipp32f value[4]))
IPPAPI(IppStatus,ippiThreshold_LTVal_16u_C1R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, Ipp16u threshold,
       Ipp16u value))
IPPAPI(IppStatus,ippiThreshold_LTVal_16u_C3R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[3],
       const Ipp16u value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_16u_AC4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[3],
       const Ipp16u value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_16u_C1IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16u threshold, Ipp16u value))
IPPAPI(IppStatus,ippiThreshold_LTVal_16u_C3IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[3], const Ipp16u value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[3], const Ipp16u value[3]))
IPPAPI(IppStatus,ippiThreshold_LTVal_16u_C4R,(const Ipp16u* pSrc, int srcStep,
       Ipp16u* pDst, int dstStep, IppiSize roiSize, const Ipp16u threshold[4],
       const Ipp16u value[4]))
IPPAPI(IppStatus,ippiThreshold_LTVal_16u_C4IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u threshold[4], const Ipp16u value[4]))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippiThreshold_LTValGTVal_8u_C1R
//              ippiThreshold_LTValGTVal_8u_C3R
//              ippiThreshold_LTValGTVal_8u_AC4R
//              ippiThreshold_LTValGTVal_16s_C1R
//              ippiThreshold_LTValGTVal_16s_C3R
//              ippiThreshold_LTValGTVal_16s_AC4R
//              ippiThreshold_LTValGTVal_32f_C1R
//              ippiThreshold_LTValGTVal_32f_C3R
//              ippiThreshold_LTValGTVal_32f_AC4R
//              ippiThreshold_LTValGTVal_16u_C1R
//              ippiThreshold_LTValGTVal_16u_C3R
//              ippiThreshold_LTValGTVal_16u_AC4R
//
//  Purpose:    Performs double thresholding of pixel values
//  Returns:
//   ippStsNoErr        OK
//   ippStsNullPtrErr   One of the pointers is NULL
//   ippStsSizeErr      roiSize has a field with zero or negative value
//   ippStsThresholdErr thresholdLT > thresholdGT
//   ippStsStepErr      One of the step values is zero or negative
//
//  Parameters:
///  Parameters:
//   pSrc        Pointer to the source image
//   srcStep     Step through the source image
//   pDst        Pointer to the destination image
//   dstStep     Step through the destination image
//   pSrcDst     Pointer to the source/destination image (in-place flavors)
//   srcDstStep  Step through the source/destination image (in-place flavors)
//   roiSize     Size of the ROI
//   thresholdLT Lower threshold value (array of values for multi-channel data)
//   valueLT     Lower output value (array or values for multi-channel data)
//   thresholdGT Upper threshold value (array of values for multi-channel data)
//   valueGT     Upper output value (array or values for multi-channel data)
*/
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_8u_C1R,(const Ipp8u* pSrc,int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp8u thresholdLT,
       Ipp8u valueLT, Ipp8u thresholdGT, Ipp8u valueGT))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_16s_C1R,(const Ipp16s* pSrc,
       int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize,
       Ipp16s thresholdLT, Ipp16s valueLT, Ipp16s thresholdGT,
       Ipp16s valueGT))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_32f_C1R,(const Ipp32f* pSrc,
       int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize,
       Ipp32f thresholdLT, Ipp32f valueLT, Ipp32f thresholdGT,
       Ipp32f valueGT))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_8u_C3R,(const Ipp8u* pSrc,int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u thresholdLT[3],
       const Ipp8u valueLT[3], const Ipp8u thresholdGT[3],
       const Ipp8u valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_16s_C3R,(const Ipp16s* pSrc,
       int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize,
       const Ipp16s thresholdLT[3], const Ipp16s valueLT[3],
       const Ipp16s thresholdGT[3], const Ipp16s valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_32f_C3R,(const Ipp32f* pSrc,
       int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize,
       const Ipp32f thresholdLT[3], const Ipp32f valueLT[3],
       const Ipp32f thresholdGT[3], const Ipp32f valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_8u_AC4R,(const Ipp8u* pSrc,int srcStep,
       Ipp8u* pDst, int dstStep, IppiSize roiSize, const Ipp8u thresholdLT[3],
       const Ipp8u valueLT[3], const Ipp8u thresholdGT[3],
       const Ipp8u valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_16s_AC4R,(const Ipp16s* pSrc,
       int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize,
       const Ipp16s thresholdLT[3], const Ipp16s valueLT[3],
       const Ipp16s thresholdGT[3], const Ipp16s valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_32f_AC4R,(const Ipp32f* pSrc,
       int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize,
       const Ipp32f thresholdLT[3], const Ipp32f valueLT[3],
       const Ipp32f thresholdGT[3], const Ipp32f valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_8u_C1IR,(Ipp8u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp8u thresholdLT, Ipp8u valueLT, Ipp8u thresholdGT,
       Ipp8u valueGT))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_16s_C1IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16s thresholdLT, Ipp16s valueLT, Ipp16s thresholdGT,
       Ipp16s valueGT))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp32f thresholdLT, Ipp32f valueLT, Ipp32f thresholdGT,
       Ipp32f valueGT))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_8u_C3IR,(Ipp8u* pSrcDst,int srcDstStep,
       IppiSize roiSize, const Ipp8u thresholdLT[3], const Ipp8u valueLT[3],
       const Ipp8u thresholdGT[3], const Ipp8u valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_16s_C3IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s thresholdLT[3], const Ipp16s valueLT[3],
       const Ipp16s thresholdGT[3], const Ipp16s valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_32f_C3IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize,  const Ipp32f thresholdLT[3], const Ipp32f valueLT[3],
       const Ipp32f thresholdGT[3], const Ipp32f valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_8u_AC4IR,(Ipp8u* pSrcDst,int srcDstStep,
       IppiSize roiSize, const Ipp8u thresholdLT[3], const Ipp8u valueLT[3],
       const Ipp8u thresholdGT[3], const Ipp8u valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_16s_AC4IR,(Ipp16s* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16s thresholdLT[3], const Ipp16s valueLT[3],
       const Ipp16s thresholdGT[3], const Ipp16s valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp32f thresholdLT[3], const Ipp32f valueLT[3],
       const Ipp32f thresholdGT[3], const Ipp32f valueGT[3]))

IPPAPI(IppStatus,ippiThreshold_LTValGTVal_16u_C1R,(const Ipp16u* pSrc,
       int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize,
       Ipp16u thresholdLT, Ipp16u valueLT, Ipp16u thresholdGT,
       Ipp16u valueGT))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_16u_C3R,(const Ipp16u* pSrc,
       int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize,
       const Ipp16u thresholdLT[3], const Ipp16u valueLT[3],
       const Ipp16u thresholdGT[3], const Ipp16u valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_16u_AC4R,(const Ipp16u* pSrc,
       int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize,
       const Ipp16u thresholdLT[3], const Ipp16u valueLT[3],
       const Ipp16u thresholdGT[3], const Ipp16u valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_16u_C1IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, Ipp16u thresholdLT, Ipp16u valueLT, Ipp16u thresholdGT,
       Ipp16u valueGT))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_16u_C3IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u thresholdLT[3], const Ipp16u valueLT[3],
       const Ipp16u thresholdGT[3], const Ipp16u valueGT[3]))
IPPAPI(IppStatus,ippiThreshold_LTValGTVal_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep,
       IppiSize roiSize, const Ipp16u thresholdLT[3], const Ipp16u valueLT[3],
       const Ipp16u thresholdGT[3], const Ipp16u valueGT[3]))

/*F/////////////////////////////////////////////////////////////////////////////////
//  Name: ippiComputeThreshold_Otsu_8u_C1R
//
//  Purpose: Calculate Otsu theshold value of images
//    Return:
//      ippStsNoErr              Ok
//      ippStsNullPtrErr         One of pointers is NULL
//      ippStsSizeErr            The width or height of images is less or equal zero
//      ippStsStepErr            The steps in images is less ROI
//    Parameters:
//      pSrc                     Pointer to image
//      srcStep                  Image step
//      roiSize                  Size of image ROI
//      pThreshold               Returned Otsu theshold value
//
//F*/

IPPAPI (IppStatus, ippiComputeThreshold_Otsu_8u_C1R, (const Ipp8u* pSrc, int srcStep,
        IppiSize roiSize, Ipp8u* pThreshold))

/* /////////////////////////////////////////////////////////////////////////////
//                   Convert and Initialization functions
///////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiCopyManaged
//
//  Purpose:  copy pixel values from the source image to the destination  image
//
//
//  Returns:
//    ippStsNullPtrErr  One of the pointers is NULL
//    ippStsSizeErr     roiSize has a field with zero or negative value
//    ippStsNoErr       OK
//
//  Parameters:
//    pSrc              Pointer  to the source image buffer
//    srcStep           Step in bytes through the source image buffer
//    pDst              Pointer to the  destination image buffer
//    dstStep           Step in bytes through the destination image buffer
//    roiSize           Size of the ROI
//    flags             The logic sum of tags sets type of copying.
//                      (IPP_TEMPORAL_COPY,IPP_NONTEMPORAL_STORE etc.)
*/

IPPAPI( IppStatus, ippiCopyManaged_8u_C1R,
                   ( const Ipp8u* pSrc, int srcStep,
                           Ipp8u* pDst, int dstStep,
                      IppiSize roiSize, int flags ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiCopy
//
//  Purpose:  copy pixel values from the source image to the destination  image
//
//
//  Returns:
//    ippStsNullPtrErr  One of the pointers is NULL
//    ippStsSizeErr     roiSize has a field with zero or negative value
//    ippStsNoErr       OK
//
//  Parameters:
//    pSrc              Pointer  to the source image buffer
//    srcStep           Step in bytes through the source image buffer
//    pDst              Pointer to the  destination image buffer
//    dstStep           Step in bytes through the destination image buffer
//    roiSize           Size of the ROI
//    pMask             Pointer to the mask image buffer
//    maskStep          Step in bytes through the mask image buffer
*/

IPPAPI( IppStatus, ippiCopy_8u_C3C1R,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_8u_C1C3R,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_8u_C4C1R,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_8u_C1C4R,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_8u_C3CR,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_8u_C4CR,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_8u_AC4C3R,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_8u_C3AC4R,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_8u_C1R,
                    ( const Ipp8u* pSrc, int srcStep,
                      Ipp8u* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_8u_C3R,
                    ( const Ipp8u* pSrc, int srcStep,
                      Ipp8u* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_8u_C4R,
                    ( const Ipp8u* pSrc, int srcStep,
                      Ipp8u* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_8u_AC4R,
                    ( const Ipp8u* pSrc, int srcStep,
                      Ipp8u* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_8u_C1MR,
                    ( const Ipp8u* pSrc, int srcStep,
                      Ipp8u* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_8u_C3MR,
                    ( const Ipp8u* pSrc, int srcStep,
                      Ipp8u* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_8u_C4MR,
                    ( const Ipp8u* pSrc, int srcStep,
                      Ipp8u* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_8u_AC4MR,
                    ( const Ipp8u* pSrc, int srcStep,
                      Ipp8u* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI( IppStatus, ippiCopy_16s_C3C1R,
                   ( const Ipp16s* pSrc, int srcStep,
                     Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16s_C1C3R,
                   ( const Ipp16s* pSrc, int srcStep,
                     Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16s_C4C1R,
                   ( const Ipp16s* pSrc, int srcStep,
                     Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16s_C1C4R,
                   ( const Ipp16s* pSrc, int srcStep,
                     Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16s_C3CR,
                   ( const Ipp16s* pSrc, int srcStep,
                     Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16s_C4CR,
                   ( const Ipp16s* pSrc, int srcStep,
                     Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16s_AC4C3R,
                   ( const Ipp16s* pSrc, int srcStep,
                     Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16s_C3AC4R,
                   ( const Ipp16s* pSrc, int srcStep,
                     Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16s_C1R,
                    ( const Ipp16s* pSrc, int srcStep,
                      Ipp16s* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16s_C3R,
                    ( const Ipp16s* pSrc, int srcStep,
                      Ipp16s* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16s_C4R,
                    ( const Ipp16s* pSrc, int srcStep,
                      Ipp16s* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16s_AC4R,
                    ( const Ipp16s* pSrc, int srcStep,
                      Ipp16s* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16s_C1MR,
                    ( const Ipp16s* pSrc, int srcStep,
                      Ipp16s* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_16s_C3MR,
                    ( const Ipp16s* pSrc, int srcStep,
                      Ipp16s* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_16s_C4MR,
                    ( const Ipp16s* pSrc, int srcStep,
                      Ipp16s* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_16s_AC4MR,
                    ( const Ipp16s* pSrc, int srcStep,
                      Ipp16s* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI( IppStatus, ippiCopy_32f_C3C1R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32f_C1C3R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32f_C4C1R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32f_C1C4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32f_C3CR,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32f_C4CR,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32f_AC4C3R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32f_C3AC4R,
                   ( const Ipp32f* pSrc, int srcStep,
                     Ipp32f* pDst, int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32f_C1R,
                    ( const Ipp32f* pSrc, int srcStep,
                      Ipp32f* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32f_C3R,
                    ( const Ipp32f* pSrc, int srcStep,
                      Ipp32f* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32f_C4R,
                    ( const Ipp32f* pSrc, int srcStep,
                      Ipp32f* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32f_AC4R,
                    ( const Ipp32f* pSrc, int srcStep,
                      Ipp32f* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32f_C1MR,
                    ( const Ipp32f* pSrc, int srcStep,
                      Ipp32f* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_32f_C3MR,
                    ( const Ipp32f* pSrc, int srcStep,
                      Ipp32f* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_32f_C4MR,
                    ( const Ipp32f* pSrc, int srcStep,
                      Ipp32f* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_32f_AC4MR,
                    ( const Ipp32f* pSrc, int srcStep,
                      Ipp32f* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_8u_C3P3R, ( const Ipp8u* pSrc, int srcStep,
                    Ipp8u* const pDst[3], int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_8u_P3C3R, (const  Ipp8u* const pSrc[3],
                            int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_8u_C4P4R, ( const Ipp8u* pSrc, int srcStep,
                    Ipp8u* const pDst[4], int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_8u_P4C4R, (const  Ipp8u* const pSrc[4],
                            int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16s_C3P3R, ( const Ipp16s* pSrc, int srcStep,
                    Ipp16s* const pDst[3], int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16s_P3C3R, (const  Ipp16s* const pSrc[3],
                            int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16s_C4P4R, ( const Ipp16s* pSrc, int srcStep,
                    Ipp16s* const pDst[4], int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16s_P4C4R, (const  Ipp16s* const pSrc[4],
                            int srcStep, Ipp16s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32f_C3P3R, ( const Ipp32f* pSrc, int srcStep,
                    Ipp32f* const pDst[3], int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32f_P3C3R, (const  Ipp32f* const pSrc[3],
                            int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32f_C4P4R, ( const Ipp32f* pSrc, int srcStep,
                    Ipp32f* const pDst[4], int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32f_P4C4R, (const  Ipp32f* const pSrc[4],
                            int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32s_C3C1R,
                   ( const Ipp32s* pSrc, int srcStep,
                     Ipp32s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32s_C1C3R,
                   ( const Ipp32s* pSrc, int srcStep,
                     Ipp32s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32s_C4C1R,
                   ( const Ipp32s* pSrc, int srcStep,
                     Ipp32s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32s_C1C4R,
                   ( const Ipp32s* pSrc, int srcStep,
                     Ipp32s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32s_C3CR,
                   ( const Ipp32s* pSrc, int srcStep,
                     Ipp32s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32s_C4CR,
                   ( const Ipp32s* pSrc, int srcStep,
                     Ipp32s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32s_AC4C3R,
                   ( const Ipp32s* pSrc, int srcStep,
                     Ipp32s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_32s_C3AC4R,
                   ( const Ipp32s* pSrc, int srcStep,
                     Ipp32s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32s_C1R,
                    ( const Ipp32s* pSrc, int srcStep,
                      Ipp32s* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32s_C3R,
                    ( const Ipp32s* pSrc, int srcStep,
                      Ipp32s* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32s_C4R,
                    ( const Ipp32s* pSrc, int srcStep,
                      Ipp32s* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32s_AC4R,
                    ( const Ipp32s* pSrc, int srcStep,
                      Ipp32s* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32s_C1MR,
                    ( const Ipp32s* pSrc, int srcStep,
                      Ipp32s* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_32s_C3MR,
                    ( const Ipp32s* pSrc, int srcStep,
                      Ipp32s* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_32s_C4MR,
                    ( const Ipp32s* pSrc, int srcStep,
                      Ipp32s* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_32s_AC4MR,
                    ( const Ipp32s* pSrc, int srcStep,
                      Ipp32s* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_32s_C3P3R, ( const Ipp32s* pSrc, int srcStep,
                    Ipp32s* const pDst[3], int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32s_P3C3R, (const  Ipp32s* const pSrc[3],
                            int srcStep, Ipp32s* pDst, int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32s_C4P4R, ( const Ipp32s* pSrc, int srcStep,
                    Ipp32s* const pDst[4], int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_32s_P4C4R, (const  Ipp32s* const pSrc[4],
                            int srcStep, Ipp32s* pDst, int dstStep, IppiSize roiSize ))

IPPAPI( IppStatus, ippiCopy_16u_C3C1R,
                   ( const Ipp16u* pSrc, int srcStep,
                     Ipp16u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16u_C1C3R,
                   ( const Ipp16u* pSrc, int srcStep,
                     Ipp16u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16u_C4C1R,
                   ( const Ipp16u* pSrc, int srcStep,
                     Ipp16u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16u_C1C4R,
                   ( const Ipp16u* pSrc, int srcStep,
                     Ipp16u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16u_C3CR,
                   ( const Ipp16u* pSrc, int srcStep,
                     Ipp16u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16u_C4CR,
                   ( const Ipp16u* pSrc, int srcStep,
                     Ipp16u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16u_AC4C3R,
                   ( const Ipp16u* pSrc, int srcStep,
                     Ipp16u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI( IppStatus, ippiCopy_16u_C3AC4R,
                   ( const Ipp16u* pSrc, int srcStep,
                     Ipp16u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16u_C1R,
                    ( const Ipp16u* pSrc, int srcStep,
                      Ipp16u* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16u_C3R,
                    ( const Ipp16u* pSrc, int srcStep,
                      Ipp16u* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16u_C4R,
                    ( const Ipp16u* pSrc, int srcStep,
                      Ipp16u* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16u_AC4R,
                    ( const Ipp16u* pSrc, int srcStep,
                      Ipp16u* pDst, int dstStep,IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16u_C1MR,
                    ( const Ipp16u* pSrc, int srcStep,
                      Ipp16u* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_16u_C3MR,
                    ( const Ipp16u* pSrc, int srcStep,
                      Ipp16u* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_16u_C4MR,
                    ( const Ipp16u* pSrc, int srcStep,
                      Ipp16u* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiCopy_16u_AC4MR,
                    ( const Ipp16u* pSrc, int srcStep,
                      Ipp16u* pDst, int dstStep,IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))

IPPAPI ( IppStatus, ippiCopy_16u_C3P3R, ( const Ipp16u* pSrc, int srcStep,
                    Ipp16u* const pDst[3], int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16u_P3C3R, (const  Ipp16u* const pSrc[3],
                            int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16u_C4P4R, ( const Ipp16u* pSrc, int srcStep,
                    Ipp16u* const pDst[4], int dstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiCopy_16u_P4C4R, (const  Ipp16u* const pSrc[4],
                            int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiCopyReplicateBorder
//
//  Purpose:   Copies pixel values between two buffers and adds
//             the replicated border pixels.
//
//  Returns:
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsSizeErr       1). srcRoiSize or dstRoiSize has a field with negative or zero value
//                        2). topBorderHeight or leftBorderWidth is less than zero
//                        3). dstRoiSize.width < srcRoiSize.width + leftBorderWidth
//                        4). dstRoiSize.height < srcRoiSize.height + topBorderHeight
//    ippStsStepErr       srcStep or dstStep is less than or equal to zero
//    ippStsNoErr         OK
//
//  Parameters:
//    pSrc                Pointer  to the source image buffer
//    srcStep             Step in bytes through the source image
//    pDst                Pointer to the  destination image buffer
//    dstStep             Step in bytes through the destination image
//    scrRoiSize          Size of the source ROI in pixels
//    dstRoiSize          Size of the destination ROI in pixels
//    topBorderHeight     Height of the top border in pixels
//    leftBorderWidth     Width of the left border in pixels
*/

IPPAPI (IppStatus, ippiCopyReplicateBorder_8u_C1R,
            ( const Ipp8u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp8u* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_8u_C3R,
            ( const Ipp8u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp8u* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_8u_AC4R,
            ( const Ipp8u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp8u* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_8u_C4R,
            ( const Ipp8u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp8u* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_16s_C1R,
            ( const Ipp16s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16s* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_16s_C3R,
            ( const Ipp16s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16s* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_16s_AC4R,
            ( const Ipp16s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16s* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_16s_C4R,
            ( const Ipp16s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16s* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_32s_C1R,
            ( const Ipp32s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32s* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_32s_C3R,
            ( const Ipp32s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32s* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_32s_AC4R,
            ( const Ipp32s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32s* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_32s_C4R,
            ( const Ipp32s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32s* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )

IPPAPI ( IppStatus, ippiCopyReplicateBorder_8u_C1IR,
                    ( const Ipp8u* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_8u_C3IR,
                    ( const Ipp8u* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_8u_AC4IR,
                    ( const Ipp8u* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_8u_C4IR,
                    ( const Ipp8u* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )

IPPAPI ( IppStatus, ippiCopyReplicateBorder_16s_C1IR,
                    ( const Ipp16s* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_16s_C3IR,
                    ( const Ipp16s* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_16s_AC4IR,
                    ( const Ipp16s* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_16s_C4IR,
                    ( const Ipp16s* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )

IPPAPI ( IppStatus, ippiCopyReplicateBorder_32s_C1IR,
                    ( const Ipp32s* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_32s_C3IR,
                    ( const Ipp32s* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_32s_AC4IR,
                    ( const Ipp32s* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_32s_C4IR,
                    ( const Ipp32s* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )

IPPAPI ( IppStatus, ippiCopyReplicateBorder_16u_C1IR,
                    ( const Ipp16u* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_16u_C3IR,
                    ( const Ipp16u* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_16u_AC4IR,
                    ( const Ipp16u* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_16u_C4IR,
                    ( const Ipp16u* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )

IPPAPI (IppStatus, ippiCopyReplicateBorder_16u_C1R,
            ( const Ipp16u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16u* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_16u_C3R,
            ( const Ipp16u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16u* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_16u_AC4R,
            ( const Ipp16u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16u* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_16u_C4R,
            ( const Ipp16u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16u* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )

IPPAPI (IppStatus, ippiCopyReplicateBorder_32f_C1R,
            ( const Ipp32f* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32f* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_32f_C3R,
            ( const Ipp32f* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32f* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_32f_AC4R,
            ( const Ipp32f* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32f* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI (IppStatus, ippiCopyReplicateBorder_32f_C4R,
            ( const Ipp32f* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32f* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )

IPPAPI ( IppStatus, ippiCopyReplicateBorder_32f_C1IR,
                    ( const Ipp32f* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_32f_C3IR,
                    ( const Ipp32f* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_32f_AC4IR,
                    ( const Ipp32f* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )
IPPAPI ( IppStatus, ippiCopyReplicateBorder_32f_C4IR,
                    ( const Ipp32f* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiCopyConstBorder
//
//  Purpose:    Copies pixel values between two buffers and adds
//              the border pixels with constant value.
//
//  Returns:
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      1). srcRoiSize or dstRoiSize has a field with negative or zero value
//                       2). topBorderHeight or leftBorderWidth is less than zero
//                       3). dstRoiSize.width < srcRoiSize.width + leftBorderWidth
//                       4). dstRoiSize.height < srcRoiSize.height + topBorderHeight
//    ippStsStepErr      srcStep or dstStep is less than or equal to zero
//    ippStsNoErr        OK
//
//  Parameters:
//    pSrc               Pointer  to the source image buffer
//    srcStep            Step in bytes through the source image
//    pDst               Pointer to the  destination image buffer
//    dstStep            Step in bytes through the destination image
//    srcRoiSize         Size of the source ROI in pixels
//    dstRoiSize         Size of the destination ROI in pixels
//    topBorderHeight    Height of the top border in pixels
//    leftBorderWidth    Width of the left border in pixels
//    value              Constant value to assign to the border pixels
*/

IPPAPI (IppStatus, ippiCopyConstBorder_8u_C1R,
            ( const Ipp8u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp8u* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            Ipp8u value ) )
IPPAPI (IppStatus, ippiCopyConstBorder_8u_C3R,
            ( const Ipp8u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp8u* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp8u value[3] ) )
IPPAPI (IppStatus, ippiCopyConstBorder_8u_AC4R,
            ( const Ipp8u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp8u* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp8u value[3] ) )
IPPAPI (IppStatus, ippiCopyConstBorder_8u_C4R,
            ( const Ipp8u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp8u* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp8u value[4] ) )
IPPAPI (IppStatus, ippiCopyConstBorder_16s_C1R,
            ( const Ipp16s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16s* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            Ipp16s value ) )
IPPAPI (IppStatus, ippiCopyConstBorder_16s_C3R,
            ( const Ipp16s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16s* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp16s value[3] ) )
IPPAPI (IppStatus, ippiCopyConstBorder_16s_AC4R,
            ( const Ipp16s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16s* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp16s value[3] ) )
IPPAPI (IppStatus, ippiCopyConstBorder_16s_C4R,
            ( const Ipp16s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16s* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp16s value[4] ) )
IPPAPI (IppStatus, ippiCopyConstBorder_32s_C1R,
            ( const Ipp32s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32s* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            Ipp32s value ) )
IPPAPI (IppStatus, ippiCopyConstBorder_32s_C3R,
            ( const Ipp32s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32s* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp32s value[3] ) )
IPPAPI (IppStatus, ippiCopyConstBorder_32s_AC4R,
            ( const Ipp32s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32s* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp32s value[3] ) )
IPPAPI (IppStatus, ippiCopyConstBorder_32s_C4R,
            ( const Ipp32s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32s* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp32s value[4] ) )

IPPAPI (IppStatus, ippiCopyConstBorder_16u_C1R,
            ( const Ipp16u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16u* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            Ipp16u value ) )
IPPAPI (IppStatus, ippiCopyConstBorder_16u_C3R,
            ( const Ipp16u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16u* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp16u value[3] ) )
IPPAPI (IppStatus, ippiCopyConstBorder_16u_AC4R,
            ( const Ipp16u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16u* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp16u value[3] ) )
IPPAPI (IppStatus, ippiCopyConstBorder_16u_C4R,
            ( const Ipp16u* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp16u* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp16u value[4] ) )

IPPAPI (IppStatus, ippiCopyConstBorder_32f_C1R,
            ( const Ipp32f* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32f* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            Ipp32f value ) )
IPPAPI (IppStatus, ippiCopyConstBorder_32f_C3R,
            ( const Ipp32f* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32f* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp32f value[3] ) )
IPPAPI (IppStatus, ippiCopyConstBorder_32f_AC4R,
            ( const Ipp32f* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32f* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp32f value[3] ) )
IPPAPI (IppStatus, ippiCopyConstBorder_32f_C4R,
            ( const Ipp32f* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32f* pDst,  int dstStep, IppiSize dstRoiSize,
                            int topBorderHeight, int leftBorderWidth,
                            const Ipp32f value[4] ) )


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiCopyWrapBorder
//
//  Purpose:    Copies pixel values between two buffers and adds the border pixels.
//
//  Returns:
//    ippStsNullPtrErr    One of the pointers is NULL
//    ippStsSizeErr       1). srcRoiSize or dstRoiSize has a field with negative or zero value
//                        2). topBorderHeight or leftBorderWidth is less than zero
//                        3). dstRoiSize.width < srcRoiSize.width + leftBorderWidth
//                        4). dstRoiSize.height < srcRoiSize.height + topBorderHeight
//    ippStsStepErr       srcStep or dstStep is less than or equal to zero
//    ippStsNoErr         OK
//
//  Parameters:
//    pSrc                Pointer  to the source image buffer
//    srcStep             Step in bytes through the source image
//    pDst                Pointer to the  destination image buffer
//    dstStep             Step in bytes through the destination image
//    scrRoiSize          Size of the source ROI in pixels
//    dstRoiSize          Size of the destination ROI in pixels
//    topBorderHeight     Height of the top border in pixels
//    leftBorderWidth     Width of the left border in pixels
*/

IPPAPI (IppStatus, ippiCopyWrapBorder_32s_C1R,
            ( const Ipp32s* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32s* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI ( IppStatus, ippiCopyWrapBorder_32s_C1IR,
                    ( const Ipp32s* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )

IPPAPI (IppStatus, ippiCopyWrapBorder_32f_C1R,
            ( const Ipp32f* pSrc,  int srcStep, IppiSize srcRoiSize,
                    Ipp32f* pDst,  int dstStep, IppiSize dstRoiSize,
                    int topBorderHeight, int leftBorderWidth ) )
IPPAPI ( IppStatus, ippiCopyWrapBorder_32f_C1IR,
                    ( const Ipp32f* pSrc,  int srcDstStep,
                            IppiSize srcRoiSize, IppiSize dstRoiSize,
                            int topBorderHeight, int leftborderwidth ) )

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiDup
//
//  Purpose:  Duplication pixel values from the source image
//            to the correspondent pixels in all channels
//            of the destination  image.
//
//  Returns:
//    ippStsNullPtrErr  One of the pointers is NULL
//    ippStsSizeErr     roiSize has a field with zero or negative value
//    ippStsNoErr       OK
//
//  Parameters:
//    pSrc              Pointer  to the source image buffer
//    srcStep           Step in bytes through the source image buffer
//    pDst              Pointer to the  destination image buffer
//    dstStep           Step in bytes through the destination image buffer
//    roiSize           Size of the ROI
*/

IPPAPI( IppStatus, ippiDup_8u_C1C3R,
                   ( const Ipp8u* pSrc, int srcStep,
                     Ipp8u* pDst, int dstStep, IppiSize roiSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiSet
//
//  Purpose:    Sets pixels in the image buffer to a constant value
//
//  Returns:
//    ippStsNullPtrErr  One of pointers is NULL
//    ippStsSizeErr     roiSize has a field with negative or zero value
//    ippStsNoErr       OK
//
//  Parameters:
//    value      Constant value assigned to each pixel in the image buffer
//    pDst       Pointer to the destination image buffer
//    dstStep    Step in bytes through the destination image buffer
//    roiSize    Size of the ROI
//    pMask      Pointer to the mask image buffer
//    maskStep   Step in bytes through the mask image buffer
*/

IPPAPI ( IppStatus, ippiSet_8u_C1R,
                    ( Ipp8u value, Ipp8u* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_8u_C3CR,
                    ( Ipp8u value, Ipp8u* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_8u_C4CR,
                    ( Ipp8u value, Ipp8u* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_8u_C3R,
                    ( const Ipp8u value[3], Ipp8u* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_8u_C4R,
                    ( const Ipp8u value[4], Ipp8u* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_8u_AC4R,
                    ( const Ipp8u value[3], Ipp8u* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_8u_C1MR,
                    ( Ipp8u value, Ipp8u* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_8u_C3MR,
                    ( const Ipp8u value[3], Ipp8u* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))

IPPAPI ( IppStatus, ippiSet_8u_C4MR,
                    ( const Ipp8u value[4], Ipp8u* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_8u_AC4MR,
                    ( const Ipp8u value[3], Ipp8u* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_16s_C1R,
                    ( Ipp16s value, Ipp16s* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_16s_C3CR,
                    ( Ipp16s value, Ipp16s* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_16s_C4CR,
                    ( Ipp16s value, Ipp16s* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_16s_C3R,
                    ( const Ipp16s value[3], Ipp16s* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_16s_C4R,
                    ( const Ipp16s value[4], Ipp16s* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_16s_AC4R,
                    ( const Ipp16s value[3], Ipp16s* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_16s_C1MR,
                    ( Ipp16s value, Ipp16s* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_16s_C3MR,
                    ( const Ipp16s value[3], Ipp16s* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_16s_C4MR,
                    ( const Ipp16s value[4], Ipp16s* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_16s_AC4MR,
                    ( const Ipp16s value[3], Ipp16s* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_32f_C1R,
                    ( Ipp32f value, Ipp32f* pDst, int dstStep,
                      IppiSize roiSize ))

IPPAPI ( IppStatus, ippiSet_32f_C3CR,
                    ( Ipp32f value, Ipp32f* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_32f_C4CR,
                    ( Ipp32f value, Ipp32f* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_32f_C3R,
                    ( const Ipp32f value[3], Ipp32f* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_32f_C4R,
                    ( const Ipp32f value[4], Ipp32f* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_32f_AC4R,
                    ( const Ipp32f value[3], Ipp32f* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_32f_C1MR,
                    ( Ipp32f value, Ipp32f* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_32f_C3MR,
                    ( const Ipp32f value[3], Ipp32f* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_32f_C4MR,
                    ( const Ipp32f value[4], Ipp32f* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_32f_AC4MR,
                    ( const Ipp32f value[3], Ipp32f* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_32s_C1R,
                    ( Ipp32s value, Ipp32s* pDst, int dstStep,
                      IppiSize roiSize ))

IPPAPI ( IppStatus, ippiSet_32s_C3CR,
                    ( Ipp32s value, Ipp32s* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_32s_C4CR,
                    ( Ipp32s value, Ipp32s* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_32s_C3R,
                    ( const Ipp32s value[3], Ipp32s* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_32s_C4R,
                    ( const Ipp32s value[4], Ipp32s* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_32s_AC4R,
                    ( const Ipp32s value[3], Ipp32s* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_32s_C1MR,
                    ( Ipp32s value, Ipp32s* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_32s_C3MR,
                    ( const Ipp32s value[3], Ipp32s* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_32s_C4MR,
                    ( const Ipp32s value[4], Ipp32s* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_32s_AC4MR,
                    ( const Ipp32s value[3], Ipp32s* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))

IPPAPI ( IppStatus, ippiSet_16u_C1R,
                    ( Ipp16u value, Ipp16u* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_16u_C3CR,
                    ( Ipp16u value, Ipp16u* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_16u_C4CR,
                    ( Ipp16u value, Ipp16u* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_16u_C3R,
                    ( const Ipp16u value[3], Ipp16u* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_16u_C4R,
                    ( const Ipp16u value[4], Ipp16u* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_16u_AC4R,
                    ( const Ipp16u value[3], Ipp16u* pDst, int dstStep,
                      IppiSize roiSize ))
IPPAPI ( IppStatus, ippiSet_16u_C1MR,
                    ( Ipp16u value, Ipp16u* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_16u_C3MR,
                    ( const Ipp16u value[3], Ipp16u* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_16u_C4MR,
                    ( const Ipp16u value[4], Ipp16u* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))
IPPAPI ( IppStatus, ippiSet_16u_AC4MR,
                    ( const Ipp16u value[3], Ipp16u* pDst, int dstStep,
                      IppiSize roiSize,
                      const Ipp8u* pMask, int maskStep ))

/* //////////////////////////////////////////////////////////////////////////////////
//  Name:  ippiAddRandUniform_Direct_8u_C1IR,  ippiAddRandUniform_Direct_8u_C3IR,
//         ippiAddRandUniform_Direct_8u_C4IR,  ippiAddRandUniform_Direct_8u_AC4IR,
//         ippiAddRandUniform_Direct_16s_C1IR, ippiAddRandUniform_Direct_16s_C3IR,
//         ippiAddRandUniform_Direct_16s_C4IR, ippiAddRandUniform_Direct_16s_AC4IR,
//         ippiAddRandUniform_Direct_32f_C1IR, ippiAddRandUniform_Direct_32f_C3IR,
//         ippiAddRandUniform_Direct_32f_C4IR, ippiAddRandUniform_Direct_32f_AC4IR
//         ippiAddRandUniform_Direct_16u_C1IR, ippiAddRandUniform_Direct_16u_C3IR,
//         ippiAddRandUniform_Direct_16u_C4IR, ippiAddRandUniform_Direct_16u_AC4IR,
//
//  Purpose:    Generates pseudo-random samples with uniform distribution and adds them
//              to an image.
//
//  Returns:
//    ippStsNoErr          OK
//    ippStsNullPtrErr     One of the pointers is NULL
//    ippStsSizeErr        roiSize has a field with zero or negative value
//    ippStsStepErr        The step in image is less than or equal to zero
//
//  Parameters:
//    pSrcDst              Pointer to the image
//    srcDstStep           Step in bytes through the image
//    roiSize              ROI size
//    low                  The lower bounds of the uniform distributions range
//    high                 The upper bounds of the uniform distributions range
//    pSeed                Pointer to the seed value for the pseudo-random number
//                          generator
*/

IPPAPI(IppStatus, ippiAddRandUniform_Direct_8u_C1IR,   (Ipp8u* pSrcDst, int srcDstStep,
                                                        IppiSize roiSize, Ipp8u low, Ipp8u high,
                                                        unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandUniform_Direct_8u_C3IR,   (Ipp8u* pSrcDst, int srcDstStep,
                                                        IppiSize roiSize, Ipp8u low, Ipp8u high,
                                                        unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandUniform_Direct_8u_C4IR,   (Ipp8u* pSrcDst, int srcDstStep,
                                                        IppiSize roiSize, Ipp8u low, Ipp8u high,
                                                        unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandUniform_Direct_8u_AC4IR,  (Ipp8u* pSrcDst, int srcDstStep,
                                                        IppiSize roiSize, Ipp8u low, Ipp8u high,
                                                        unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandUniform_Direct_16s_C1IR,  (Ipp16s* pSrcDst, int srcDstStep,
                                                        IppiSize roiSize, Ipp16s low, Ipp16s high,
                                                        unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandUniform_Direct_16s_C3IR,  (Ipp16s* pSrcDst, int srcDstStep,
                                                        IppiSize roiSize, Ipp16s low, Ipp16s high,
                                                        unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandUniform_Direct_16s_C4IR,  (Ipp16s* pSrcDst, int srcDstStep,
                                                        IppiSize roiSize, Ipp16s low, Ipp16s high,
                                                        unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandUniform_Direct_16s_AC4IR, (Ipp16s* pSrcDst, int srcDstStep,
                                                        IppiSize roiSize, Ipp16s low, Ipp16s high,
                                                        unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandUniform_Direct_32f_C1IR,  (Ipp32f* pSrcDst, int srcDstStep,
                                                        IppiSize roiSize, Ipp32f low, Ipp32f high,
                                                        unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandUniform_Direct_32f_C3IR,  (Ipp32f* pSrcDst, int srcDstStep,
                                                        IppiSize roiSize, Ipp32f low, Ipp32f high,
                                                        unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandUniform_Direct_32f_C4IR,  (Ipp32f* pSrcDst, int srcDstStep,
                                                        IppiSize roiSize, Ipp32f low, Ipp32f high,
                                                        unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandUniform_Direct_32f_AC4IR, (Ipp32f* pSrcDst, int srcDstStep,
                                                        IppiSize roiSize, Ipp32f low, Ipp32f high,
                                                        unsigned int* pSeed))

IPPAPI(IppStatus,ippiAddRandUniform_Direct_16u_C1IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize,
                                                           Ipp16u low, Ipp16u high, unsigned int* pSeed))
IPPAPI(IppStatus,ippiAddRandUniform_Direct_16u_C3IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize,
                                                           Ipp16u low, Ipp16u high, unsigned int* pSeed))
IPPAPI(IppStatus,ippiAddRandUniform_Direct_16u_C4IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize,
                                                           Ipp16u low, Ipp16u high, unsigned int* pSeed))
IPPAPI(IppStatus,ippiAddRandUniform_Direct_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize,
                                                           Ipp16u low, Ipp16u high, unsigned int* pSeed))

/* ////////////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAddRandGauss_Direct_8u_C1IR,  ippiAddRandGauss_Direct_8u_C3IR,
//              ippiAddRandGauss_Direct_8u_C4IR,  ippiAddRandGauss_Direct_8u_AC4IR
//              ippiAddRandGauss_Direct_16s_C1IR, ippiAddRandGauss_Direct_16s_C3IR,
//              ippiAddRandGauss_Direct_16s_C4IR, ippiAddRandGauss_Direct_16s_AC4IR,
//              ippiAddRandGauss_Direct_32f_C1IR, ippiAddRandGauss_Direct_32f_C3IR,
//              ippiAddRandGauss_Direct_32f_C4IR, ippiAddRandGauss_Direct_32f_AC4IR
//              ippiAddRandGauss_Direct_16u_C1IR, ippiAddRandGauss_Direct_16u_C3IR,
//              ippiAddRandGauss_Direct_16u_C4IR, ippiAddRandGauss_Direct_16u_AC4IR,
//
//  Purpose:    Generates pseudo-random samples with normal distribution and adds them
//              to an image.
//
//  Returns:
//    ippStsNoErr           OK
//    ippStsNullPtrErr      One of the pointers is NULL
//    ippStsSizeErr         roiSize has a field with zero or negative value
//    ippStsStepErr         The step value is less than or equal to zero
//
//  Parameters:
//    pSrcDst               Pointer to the image
//    srcDstStep            Step in bytes through the image
//    roiSize               ROI size
//    mean                  The mean of the normal distribution
//    stdev                 The standard deviation of the normal distribution
//    pSeed                 Pointer to the seed value for the pseudo-random number
//                             generator
*/


IPPAPI(IppStatus, ippiAddRandGauss_Direct_8u_C1IR,   (Ipp8u* pSrcDst, int srcDstStep,
                                                      IppiSize roiSize, Ipp8u mean, Ipp8u stdev,
                                                      unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandGauss_Direct_8u_C3IR,   (Ipp8u* pSrcDst, int srcDstStep,
                                                      IppiSize roiSize, Ipp8u mean, Ipp8u stdev,
                                                      unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandGauss_Direct_8u_C4IR,   (Ipp8u* pSrcDst, int srcDstStep,
                                                      IppiSize roiSize, Ipp8u mean, Ipp8u stdev,
                                                      unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandGauss_Direct_8u_AC4IR,  (Ipp8u* pSrcDst, int srcDstStep,
                                                      IppiSize roiSize, Ipp8u mean, Ipp8u stdev,
                                                      unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandGauss_Direct_16s_C1IR,  (Ipp16s* pSrcDst, int srcDstStep,
                                                      IppiSize roiSize, Ipp16s mean, Ipp16s stdev,
                                                      unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandGauss_Direct_16s_C3IR,  (Ipp16s* pSrcDst, int srcDstStep,
                                                      IppiSize roiSize, Ipp16s mean, Ipp16s stdev,
                                                      unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandGauss_Direct_16s_C4IR,  (Ipp16s* pSrcDst, int srcDstStep,
                                                      IppiSize roiSize, Ipp16s mean, Ipp16s stdev,
                                                      unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandGauss_Direct_16s_AC4IR, (Ipp16s* pSrcDst, int srcDstStep,
                                                      IppiSize roiSize, Ipp16s mean, Ipp16s stdev,
                                                      unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandGauss_Direct_32f_C1IR,  (Ipp32f* pSrcDst, int srcDstStep,
                                                      IppiSize roiSize, Ipp32f mean, Ipp32f stdev,
                                                      unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandGauss_Direct_32f_C3IR,  (Ipp32f* pSrcDst, int srcDstStep,
                                                      IppiSize roiSize, Ipp32f mean, Ipp32f stdev,
                                                      unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandGauss_Direct_32f_C4IR,  (Ipp32f* pSrcDst, int srcDstStep,
                                                      IppiSize roiSize, Ipp32f mean, Ipp32f stdev,
                                                      unsigned int* pSeed))
IPPAPI(IppStatus, ippiAddRandGauss_Direct_32f_AC4IR, (Ipp32f* pSrcDst, int srcDstStep,
                                                      IppiSize roiSize, Ipp32f mean, Ipp32f stdev,
                                                      unsigned int* pSeed))

IPPAPI(IppStatus,ippiAddRandGauss_Direct_16u_C1IR, (Ipp16u* pSrcDst,int srcDstStep,IppiSize roiSize,
                                                     Ipp16u mean, Ipp16u stdev, unsigned int* pSeed))
IPPAPI(IppStatus,ippiAddRandGauss_Direct_16u_C3IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize,
                                                      Ipp16u mean, Ipp16u stdev, unsigned int* pSeed))
IPPAPI(IppStatus,ippiAddRandGauss_Direct_16u_C4IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize,
                                                      Ipp16u mean, Ipp16u stdev, unsigned int* pSeed))
IPPAPI(IppStatus,ippiAddRandGauss_Direct_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize,
                                                      Ipp16u mean, Ipp16u stdev, unsigned int* pSeed))

/* ////////////////////////////////////////////////////////////////////////////////////
//  Name:               ippiImageJaehne
//  Purpose:            Creates Jaenne's test image
//  Returns:
//    ippStsNoErr       No error
//    ippStsNullPtrErr  pDst pointer is NULL
//    ippStsSizeErr     roiSize has a field with zero or negative value, or
//                      srcDstStep has a zero or negative value
//  Parameters:
//    pDst              Pointer to the destination buffer
//    DstStep           Step in bytes through the destination buffer
//    roiSize           Size of the destination image ROI in pixels
//  Notes:
//                      Dst(x,y,) = A*Sin(0.5*IPP_PI* (x2^2 + y2^2) / roiSize.height),
//                      x variables from 0 to roi.width-1,
//                      y variables from 0 to roi.height-1,
//                      x2 = (x-roi.width+1)/2.0 ,   y2 = (y-roi.height+1)/2.0 .
//                      A is the constant value depends on the image type being created.
*/

IPPAPI(IppStatus, ippiImageJaehne_8u_C1R, (Ipp8u* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_8u_C3R, (Ipp8u* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_8s_C1R, (Ipp8s* pDst, int DstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiImageJaehne_8s_C3R, (Ipp8s* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_16u_C1R, (Ipp16u* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_16u_C3R, (Ipp16u* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_16s_C1R, (Ipp16s* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_16s_C3R, (Ipp16s* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_32s_C1R, (Ipp32s* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_32s_C3R, (Ipp32s* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_32f_C1R, (Ipp32f* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_32f_C3R, (Ipp32f* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_8u_C4R,  (Ipp8u* pDst,  int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_8s_C4R,  (Ipp8s* pDst,  int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_16u_C4R, (Ipp16u* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_16s_C4R, (Ipp16s* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_32s_C4R, (Ipp32s* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_32f_C4R, (Ipp32f* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_8u_AC4R, (Ipp8u* pDst,  int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_8s_AC4R, (Ipp8s* pDst,  int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_16u_AC4R,(Ipp16u* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_16s_AC4R,(Ipp16s* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_32s_AC4R,(Ipp32s* pDst, int DstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiImageJaehne_32f_AC4R,(Ipp32f* pDst, int DstStep, IppiSize roiSize))


/* /////////////////////////////////////////////////////////////////////////
//  Name:               ippiImageRamp
//  Purpose:            Creates an ippi test image with an intensity ramp
//  Returns:
//    ippStsNoErr       No error
//    ippStsNullPtrErr  pDst pointer is NULL
//    ippStsSizeErr     roiSize has a field with zero or negative value, or
//                      srcDstStep has a zero or negative value
//  Parameters:
//    pDst              Pointer to the destination buffer
//    DstStep           Step in bytes through the destination buffer
//    roiSize           Size of the destination image ROI in pixels
//    offset            Offset value
//    slope             Slope coefficient
//    axis              Specifies the direction of the image intensity ramp,
//                      possible values:
//                        ippAxsHorizontal   in X-direction,
//                        ippAxsVertical     in Y-direction,
//                        ippAxsBoth         in both X and Y-directions.
//  Notes:              Dst(x,y) = offset + slope * x   (if ramp for X-direction)
//                      Dst(x,y) = offset + slope * y   (if ramp for Y-direction)
//                      Dst(x,y) = offset + slope * x*y (if ramp for X,Y-direction)
*/
IPPAPI(IppStatus, ippiImageRamp_8u_C1R, (Ipp8u* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_8u_C3R, (Ipp8u* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_8s_C1R, (Ipp8s* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_8s_C3R, (Ipp8s* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_16u_C1R, (Ipp16u* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_16u_C3R, (Ipp16u* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_16s_C1R, (Ipp16s* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_16s_C3R, (Ipp16s* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_32s_C1R, (Ipp32s* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_32s_C3R, (Ipp32s* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_32f_C1R, (Ipp32f* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_32f_C3R, (Ipp32f* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_8u_C4R, (Ipp8u* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_8s_C4R, (Ipp8s* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_16u_C4R,(Ipp16u* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_16s_C4R,(Ipp16s* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_32s_C4R,(Ipp32s* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_32f_C4R,(Ipp32f* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_8u_AC4R, (Ipp8u* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_8s_AC4R, (Ipp8s* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_16u_AC4R,(Ipp16u* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_16s_AC4R,(Ipp16s* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_32s_AC4R,(Ipp32s* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))
IPPAPI(IppStatus, ippiImageRamp_32f_AC4R,(Ipp32f* pDst, int DstStep, IppiSize roiSize, float offset, float slope, IppiAxis axis))


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiConvert
//
//  Purpose:    Converts pixel values of an image from one bit depth to another
//
//  Returns:
//    ippStsNullPtrErr      One of the pointers is NULL
//    ippStsSizeErr         roiSize has a field with zero or negative value
//    ippStsStepErr         srcStep or dstStep has zero or negative value
//    ippStsNoErr           OK
//
//  Parameters:
//    pSrc                  Pointer  to the source image
//    srcStep               Step through the source image
//    pDst                  Pointer to the  destination image
//    dstStep               Step in bytes through the destination image
//    roiSize               Size of the ROI
//    roundMode             Rounding mode, ippRndZero or ippRndNear
*/
IPPAPI ( IppStatus, ippiConvert_8u16u_C1R,
        (const Ipp8u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u16u_C3R,
        (const Ipp8u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u16u_AC4R,
        (const Ipp8u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u16u_C4R,
        (const Ipp8u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16u8u_C1R,
        ( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16u8u_C3R,
        ( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16u8u_AC4R,
        ( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16u8u_C4R,
        ( const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u16s_C1R,
        (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u16s_C3R,
        (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u16s_AC4R,
        (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u16s_C4R,
        (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16s8u_C1R,
        ( const Ipp16s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16s8u_C3R,
        ( const Ipp16s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16s8u_AC4R,
        ( const Ipp16s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16s8u_C4R,
        ( const Ipp16s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u32f_C1R,
        (const Ipp8u* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u32f_C3R,
        (const Ipp8u* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u32f_AC4R,
        (const Ipp8u* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u32f_C4R,
        (const Ipp8u* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_32f8u_C1R,
        ( const Ipp32f* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_32f8u_C3R,
        ( const Ipp32f* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_32f8u_AC4R,
        ( const Ipp32f* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_32f8u_C4R,
        ( const Ipp32f* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_16s32f_C1R,
        (const Ipp16s* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16s32f_C3R,
        (const Ipp16s* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16s32f_AC4R,
        (const Ipp16s* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16s32f_C4R,
        (const Ipp16s* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_32f16s_C1R,
        ( const Ipp32f* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_32f16s_C3R,
        ( const Ipp32f* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_32f16s_AC4R,
        ( const Ipp32f* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_32f16s_C4R,
        ( const Ipp32f* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_8s32f_C1R,
        (const Ipp8s* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8s32f_C3R,
        (const Ipp8s* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8s32f_AC4R,
        (const Ipp8s* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8s32f_C4R,
        (const Ipp8s* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_32f8s_C1R,
        ( const Ipp32f* pSrc, int srcStep, Ipp8s* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_32f8s_C3R,
        ( const Ipp32f* pSrc, int srcStep, Ipp8s* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_32f8s_AC4R,
        ( const Ipp32f* pSrc, int srcStep, Ipp8s* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_32f8s_C4R,
        ( const Ipp32f* pSrc, int srcStep, Ipp8s* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_16u32f_C1R,
        (const Ipp16u* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16u32f_C3R,
        (const Ipp16u* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16u32f_AC4R,
        (const Ipp16u* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16u32f_C4R,
        (const Ipp16u* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_32f16u_C1R,
        ( const Ipp32f* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_32f16u_C3R,
        ( const Ipp32f* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_32f16u_AC4R,
        ( const Ipp32f* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_32f16u_C4R,
        ( const Ipp32f* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
          IppiSize roiSize, IppRoundMode roundMode ))
IPPAPI ( IppStatus, ippiConvert_8u32s_C1R,
        (const Ipp8u* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u32s_C3R,
        (const Ipp8u* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u32s_AC4R,
        (const Ipp8u* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8u32s_C4R,
        (const Ipp8u* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_32s8u_C1R,
        ( const Ipp32s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_32s8u_C3R,
        ( const Ipp32s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_32s8u_AC4R,
        ( const Ipp32s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_32s8u_C4R,
        ( const Ipp32s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8s32s_C1R,
        (const Ipp8s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8s32s_C3R,
        (const Ipp8s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8s32s_AC4R,
        (const Ipp8s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_8s32s_C4R,
        (const Ipp8s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_32s8s_C1R,
        ( const Ipp32s* pSrc, int srcStep, Ipp8s* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_32s8s_C3R,
        ( const Ipp32s* pSrc, int srcStep, Ipp8s* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_32s8s_AC4R,
        ( const Ipp32s* pSrc, int srcStep, Ipp8s* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_32s8s_C4R,
        ( const Ipp32s* pSrc, int srcStep, Ipp8s* pDst, int dstStep,
          IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16u32s_C1R,
        (const Ipp16u* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16u32s_C3R,
        (const Ipp16u* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16u32s_AC4R,
        (const Ipp16u* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16u32s_C4R,
        (const Ipp16u* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))

IPPAPI ( IppStatus, ippiConvert_16s32s_C1R,
        (const Ipp16s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16s32s_C3R,
        (const Ipp16s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16s32s_AC4R,
        (const Ipp16s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiConvert_16s32s_C4R,
        (const Ipp16s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))

IPPAPI ( IppStatus, ippiConvert_8s8u_C1Rs,
        (const Ipp8s*  pSrc, int srcStep, Ipp8u*  pDst, int dstStep, 
         IppiSize roi))

IPPAPI ( IppStatus, ippiConvert_8s16s_C1R,
        (const Ipp8s*  pSrc, int srcStep, Ipp16s* pDst, int dstStep, 
         IppiSize roi))

IPPAPI ( IppStatus, ippiConvert_8s16u_C1Rs, 
        (const Ipp8s*  pSrc, int srcStep, Ipp16u* pDst, int dstStep, 
         IppiSize roi))

IPPAPI ( IppStatus, ippiConvert_8s32u_C1Rs,
        (const Ipp8s*  pSrc, int srcStep, Ipp32u* pDst, int dstStep, 
         IppiSize roi))
IPPAPI ( IppStatus, ippiConvert_8u8s_C1RSfs,
        (const Ipp8u*  pSrc, int srcStep, Ipp8s*  pDst, int dstStep, 
         IppiSize roi,IppRoundMode rndMode, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_16s8s_C1RSfs,  
        (const Ipp16s* pSrc, int srcStep, Ipp8s*  pDst, int dstStep, 
         IppiSize roi,IppRoundMode rndMode, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_16s16u_C1Rs, 
        (const Ipp16s* pSrc, int srcStep, Ipp16u* pDst, int dstStep, 
         IppiSize roi))

IPPAPI ( IppStatus, ippiConvert_16s32u_C1Rs, 
        (const Ipp16s* pSrc, int srcStep, Ipp32u* pDst, int dstStep, 
         IppiSize roi))

IPPAPI ( IppStatus, ippiConvert_16u8s_C1RSfs,  
        (const Ipp16u* pSrc, int srcStep, Ipp8s*  pDst, int dstStep, 
         IppiSize roi,IppRoundMode rndMode, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_16u16s_C1RSfs, 
        (const Ipp16u* pSrc, int srcStep, Ipp16s* pDst, 
         int dstStep, IppiSize roi,IppRoundMode rndMode, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_16u32u_C1R, 
        (const Ipp16u* pSrc, int srcStep, Ipp32u* pDst, int dstStep, 
         IppiSize roi))

IPPAPI ( IppStatus, ippiConvert_32s16s_C1RSfs, 
        (const Ipp32s* pSrc, int srcStep, Ipp16s* pDst, int dstStep, 
         IppiSize roi,IppRoundMode rndMode, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_32s16u_C1RSfs, 
        (const Ipp32s* pSrc, int srcStep, Ipp16u* pDst, int dstStep,    
         IppiSize roi,IppRoundMode rndMode, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_32s32u_C1Rs, 
        (const Ipp32s* pSrc, int srcStep, Ipp32u* pDst, int dstStep, 
         IppiSize roi))

IPPAPI ( IppStatus, ippiConvert_32s32f_C1R, 
        (const Ipp32s* pSrc, int srcStep, Ipp32f* pDst, int dstStep, 
         IppiSize roi))

IPPAPI ( IppStatus, ippiConvert_32u8s_C1RSfs,  
        (const Ipp32u* pSrc, int srcStep, Ipp8s*  pDst, int dstStep, 
         IppiSize roi,IppRoundMode rndMode, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_32u8u_C1RSfs,  
        (const Ipp32u* pSrc, int srcStep, Ipp8u*  pDst, int dstStep, 
         IppiSize roi,IppRoundMode rndMode, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_32u16s_C1RSfs, 
        (const Ipp32u* pSrc, int srcStep, Ipp16s* pDst, int dstStep, 
         IppiSize roi,IppRoundMode rndMode, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_32u16u_C1RSfs, 
        (const Ipp32u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, 
         IppiSize roi,IppRoundMode rndMode, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_32u32s_C1RSfs, 
        (const Ipp32u* pSrc, int srcStep, Ipp32s* pDst, int dstStep, 
         IppiSize roi,IppRoundMode rndMode, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_32u32f_C1R, 
        (const Ipp32u* pSrc, int srcStep, Ipp32f* pDst, int dstStep, 
         IppiSize roi))

IPPAPI ( IppStatus, ippiConvert_32f32u_C1RSfs, 
        (const Ipp32f* pSrc, int srcStep, Ipp32u* pDst, int dstStep, 
         IppiSize roi,IppRoundMode rndMode, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_32f8s_C1RSfs,  
        (const Ipp32f* pSrc, int srcStep, Ipp8s*  pDst, int dstStep, 
         IppiSize roi, IppRoundMode round, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_32f8u_C1RSfs,  
        (const Ipp32f* pSrc, int srcStep, Ipp8u*  pDst, int dstStep, 
         IppiSize roi, IppRoundMode round, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_32f16s_C1RSfs, 
        (const Ipp32f* pSrc, int srcStep, Ipp16s* pDst, int dstStep, 
         IppiSize roi, IppRoundMode round, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_32f16u_C1RSfs, 
        (const Ipp32f* pSrc, int srcStep, Ipp16u* pDst, int dstStep, 
         IppiSize roi, IppRoundMode round, int scaleFactor))

IPPAPI ( IppStatus, ippiConvert_32f32s_C1RSfs, 
        (const Ipp32f* pSrc, int srcStep, Ipp32s* pDst, int dstStep, 
         IppiSize roi, IppRoundMode round, int scaleFactor))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiConvert_1u8u_C1R
//
//  Purpose:    Converts a bitonal image to an 8u grayscale image
//
//  Returns:
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value,
//                       or srcBitOffset is less than zero
//    ippStsStepErr      srcStep or dstStep has a negative or zero value
//    ippStsNoErr        OK
//
//  Parameters:
//    pSrc               Pointer  to the source image
//    srcStep            Step through the source image
//    srcBitOffset       Offset in the first byte of the source image row
//    pDst               Pointer to the  destination image
//    dstStep            Step through the destination image
//    roiSize            Size of the ROI
*/

IPPAPI ( IppStatus, ippiConvert_1u8u_C1R,
        ( const Ipp8u* pSrc, int srcStep, int srcBitOffset,
                Ipp8u* pDst, int dstStep, IppiSize roiSize ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiConvert_8u1u_C1R
//
//  Purpose:    Converts an 8u grayscale image to a bitonal image
//
//  Returns:
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value,
//                       or dstBitOffset is less than zero
//    ippStsStepErr      srcStep or dstStep has a negative or zero value
//    ippStsMemAllocErr  Memory allocation fails
//    ippStsNoErr        OK
//
//  Parameters:
//    pSrc               Pointer  to the source image
//    srcStep            Step through the source image
//    pDst               Pointer to the  destination image
//    dstStep            Step through the destination image
//    dstBitOffset       Offset in the first byte of the destination image row
//    roiSize            Size of the ROI
//    threshold          Threshold level for Stucki's dithering.
*/

IPPAPI ( IppStatus, ippiConvert_8u1u_C1R,( const Ipp8u* pSrc, int srcStep,
                Ipp8u* pDst, int dstStep, int dstBitOffset, IppiSize roiSize,Ipp8u threshold))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:            ippiPolarToCart
//
//  Purpose:     Converts an image in the polar coordinate form to Cartesian
//               coordinate form
//  Parameters:
//   pSrcMagn            Pointer to the source image plane containing magnitudes
//   pSrcPhase           Pointer to the source image plane containing phase values
//   srcStep             Step through the source image
//   pDst                Pointer to the destination image
//   dstStep             Step through the destination image
//   roiSize             Size of the ROI
//  Return:
//   ippStsNullPtrErr    One of the pointers is NULL
//   ippStsSizeErr       height or width of the image is less than 1
//   ippStsStepErr,      if srcStep <= 0 or
//                          dstStep <= 0
//   ippStsNoErr         No errors
*/

IPPAPI(IppStatus,ippiPolarToCart_32f32fc_P2C1R,( const Ipp32f *pSrcMagn, const Ipp32f *pSrcPhase,
                                                 int srcStep, Ipp32fc *pDst, int dstStep, IppiSize roiSize ))

IPPAPI(IppStatus, ippiPolarToCart_32fc_C1R,(const Ipp32f* pSrcMagn, const Ipp32f* pSrcPhase, int srcStep,
                                                              IppiSize roiSize, Ipp32fc *pDst , int dstStep))

IPPAPI(IppStatus, ippiPolarToCart_32sc_C1R,(const Ipp32s* pSrcMagn, const Ipp32s* pSrcPhase, int srcStep,
                                             int PhaseFixedPoint, IppiSize roiSize, Ipp32sc *pDst , int dstStep))

IPPAPI(IppStatus, ippiPolarToCart_16sc_C1R,(const Ipp16s* pSrcMagn, const Ipp16s* pSrcPhase, int srcStep,
                                             int PhaseFixedPoint, IppiSize roiSize, Ipp16sc *pDst , int dstStep))

IPPAPI(IppStatus, ippiPolarToCart_32fc_C3R,(const Ipp32f* pSrcMagn, const Ipp32f* pSrcPhase, int srcStep,
                                                                  IppiSize roiSize, Ipp32fc *pDst , int dstStep))

IPPAPI(IppStatus, ippiPolarToCart_32sc_C3R,(const Ipp32s* pSrcMagn, const Ipp32s* pSrcPhase, int srcStep,
                                             int PhaseFixedPoint, IppiSize roiSize, Ipp32sc *pDst , int dstStep))

IPPAPI(IppStatus, ippiPolarToCart_16sc_C3R,(const Ipp16s* pSrcMagn, const Ipp16s* pSrcPhase, int srcStep,
                                             int PhaseFixedPoint, IppiSize roiSize, Ipp16sc *pDst , int dstStep))


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiSwapChannels
//
//  Purpose:    Changes the order of channels of the image
//              The function performs operation for each pixel:
//                  pDst[0] = pSrc[ dstOrder[0] ]
//                  pDst[1] = pSrc[ dstOrder[1] ]
//                  pDst[2] = pSrc[ dstOrder[2] ]
//
//  Returns:
//    ippStsNullPtrErr      One of the pointers is NULL
//    ippStsSizeErr         roiSize has a field with zero or negative value
//    ippStsStepErr         One of the step values is less than or equal to zero
//    ippStsChannelOrderErr dstOrder is out of the range,
//                           it should be: dstOrder[3] = { 0..2, 0..2, 0..2 }
//    ippStsNoErr           OK
//
//  Parameters:
//    pSrc           Pointer  to the source image
//    srcStep        Step in bytes through the source image
//    pDst           Pointer to the  destination image
//    dstStep        Step in bytes through the destination image
//    pSrcDst        Pointer to the source/destination image (in-place flavors)
//    srcDstStep     Step through the source/destination image (in-place flavors)
//    roiSize        Size of the ROI
//    dstOrder       The order of channels in the destination image
*/
IPPAPI ( IppStatus, ippiSwapChannels_8u_C3R,
       ( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_8u_AC4R,
       ( const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_16u_C3R,
       ( const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
         IppiSize roiSize, const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_16u_AC4R,
       ( const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
         IppiSize roiSize, const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_16s_C3R,
       ( const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
         IppiSize roiSize, const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_16s_AC4R,
       ( const Ipp16s* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
         IppiSize roiSize, const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_32s_C3R,
       ( const Ipp32s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize, const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_32s_AC4R,
       ( const Ipp32s* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize, const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_32f_C3R,
       ( const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize, const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_32f_AC4R,
       ( const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize, const int dstOrder[3] ))

IPPAPI ( IppStatus, ippiSwapChannels_8u_C3IR,
       ( Ipp8u* pSrcDst, int srcDstStep,
             IppiSize roiSize, const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_8u_C4IR,
       ( Ipp8u* pSrcDst, int srcDstStep,
             IppiSize roiSize, const int dstOrder[4] ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiSwapChannels C3C4R,C4C3R
//
//  Purpose:    Changes the order of channels of the image
//              The function performs operation for each pixel:
//  a) C3C4R.
//    if(dstOrder[i] < 3) dst[i] = src[dstOrder[i]];
//    if(dstOrder[i] == 3) dst[i] = val;
//    if(dstOrder[i] > 3) dst[i] does not change;
//    i = 0,1,2,3
//  b) C4C3R.
//    dst[0] = src [dstOrder[0]];
//    dst[1] = src [dstOrder[1]];
//    dst[2] = src [dstOrder[2]];
//
//  Returns:
//    ippStsNullPtrErr      One of the pointers is NULL
//    ippStsSizeErr         roiSize has a field with zero or negative value
//    ippStsChannelOrderErr dstOrder is out of the range, it should be:
//                            a) C3C4R.
//                              dstOrder[i] => 0, i = 0,1,2,3.
//                            b) C4C3R.
//                              0 <= dstOrder[i] <= 3, i = 0,1,2.
//    ippStsNoErr     OK
//
//  Parameters:
//    pSrc           Pointer  to the source image
//    srcStep        Step in bytes through the source image
//    pDst           Pointer to the  destination image
//    dstStep        Step in bytes through the destination image
//    roiSize        Size of the ROI
//    dstOrder       The order of channels in the destination image
//    val            Constant value for C3C4R
*/
IPPAPI ( IppStatus, ippiSwapChannels_8u_C3C4R,
                    ( const Ipp8u* pSrc, int srcStep,
                            Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   const int dstOrder[4], Ipp8u val ))
IPPAPI ( IppStatus, ippiSwapChannels_8u_C4C3R,
                    ( const Ipp8u* pSrc, int srcStep,
                            Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_16s_C3C4R,
                    ( const Ipp16s* pSrc, int srcStep,
                            Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                    const int dstOrder[4], Ipp16s val ))
IPPAPI ( IppStatus, ippiSwapChannels_16s_C4C3R,
                    ( const Ipp16s* pSrc, int srcStep,
                            Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                    const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_16u_C3C4R,
                    ( const Ipp16u* pSrc, int srcStep,
                            Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                    const int dstOrder[4], Ipp16u val ))
IPPAPI ( IppStatus, ippiSwapChannels_16u_C4C3R,
                    ( const Ipp16u* pSrc, int srcStep,
                            Ipp16u* pDst, int dstStep, IppiSize roiSize,
                                    const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_32s_C3C4R,
                    ( const Ipp32s* pSrc, int srcStep,
                            Ipp32s* pDst, int dstStep, IppiSize roiSize,
                                    const int dstOrder[4], Ipp32s val ))
IPPAPI ( IppStatus, ippiSwapChannels_32s_C4C3R,
                    ( const Ipp32s* pSrc, int srcStep,
                            Ipp32s* pDst, int dstStep, IppiSize roiSize,
                                    const int dstOrder[3] ))
IPPAPI ( IppStatus, ippiSwapChannels_32f_C3C4R,
                    ( const Ipp32f* pSrc, int srcStep,
                            Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                    const int dstOrder[4], Ipp32f val ))
IPPAPI ( IppStatus, ippiSwapChannels_32f_C4C3R,
                    ( const Ipp32f* pSrc, int srcStep,
                            Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                    const int dstOrder[3] ))


/* /////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiScale
//
//  Purpose:   Scales pixel values of an image and converts them to another bit depth
//              dst = a + b * src;
//              a = type_min_dst - b * type_min_src;
//              b = (type_max_dst - type_min_dst) / (type_max_src - type_min_src).
//
//  Returns:
//    ippStsNullPtrErr      One of the pointers is NULL
//    ippStsSizeErr         roiSize has a field with zero or negative value
//    ippStsStepErr         One of the step values is less than or equal to zero
//    ippStsScaleRangeErr   Input data bounds are incorrect (vMax - vMin <= 0)
//    ippStsNoErr           OK
//
//  Parameters:
//    pSrc            Pointer  to the source image
//    srcStep         Step through the source image
//    pDst            Pointer to the  destination image
//    dstStep         Step through the destination image
//    roiSize         Size of the ROI
//    vMin, vMax      Minimum and maximum values of the input data (32f).
//    hint            Option to select the algorithmic implementation:
//                        1). hint == ippAlgHintAccurate
//                                  - accuracy e-8, but slowly;
//                        2). hint == ippAlgHintFast,
//                                 or ippAlgHintNone
//                                  - accuracy e-3, but quickly.
*/
IPPAPI ( IppStatus, ippiScale_8u16u_C1R,
        (const Ipp8u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiScale_8u16s_C1R,
        (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiScale_8u32s_C1R,
        (const Ipp8u* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiScale_8u32f_C1R,
        (const Ipp8u* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize, Ipp32f vMin, Ipp32f vMax ))
IPPAPI ( IppStatus, ippiScale_8u16u_C3R,
        (const Ipp8u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiScale_8u16s_C3R,
        (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiScale_8u32s_C3R,
        (const Ipp8u* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiScale_8u32f_C3R,
        (const Ipp8u* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize, Ipp32f vMin, Ipp32f vMax ))
IPPAPI ( IppStatus, ippiScale_8u16u_AC4R,
        (const Ipp8u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiScale_8u16s_AC4R,
        (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiScale_8u32s_AC4R,
        (const Ipp8u* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiScale_8u32f_AC4R,
        (const Ipp8u* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize, Ipp32f vMin, Ipp32f vMax ))
IPPAPI ( IppStatus, ippiScale_8u16u_C4R,
        (const Ipp8u* pSrc, int srcStep, Ipp16u* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiScale_8u16s_C4R,
        (const Ipp8u* pSrc, int srcStep, Ipp16s* pDst, int dstStep,
         IppiSize roiSize ))

IPPAPI ( IppStatus, ippiScale_8u32s_C4R,
        (const Ipp8u* pSrc, int srcStep, Ipp32s* pDst, int dstStep,
         IppiSize roiSize ))
IPPAPI ( IppStatus, ippiScale_8u32f_C4R,
        (const Ipp8u* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
         IppiSize roiSize, Ipp32f vMin, Ipp32f vMax ))
IPPAPI ( IppStatus, ippiScale_16u8u_C1R,
        (const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, IppHintAlgorithm hint ))
IPPAPI ( IppStatus, ippiScale_16s8u_C1R,
        (const Ipp16s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, IppHintAlgorithm hint ))
IPPAPI ( IppStatus, ippiScale_32s8u_C1R,
        (const Ipp32s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, IppHintAlgorithm hint ))
IPPAPI ( IppStatus, ippiScale_32f8u_C1R,
        (const Ipp32f* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, Ipp32f vMin, Ipp32f vMax ))
IPPAPI ( IppStatus, ippiScale_16u8u_C3R,
        (const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, IppHintAlgorithm hint ))
IPPAPI ( IppStatus, ippiScale_16s8u_C3R,
        (const Ipp16s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, IppHintAlgorithm hint ))
IPPAPI ( IppStatus, ippiScale_32s8u_C3R,
        (const Ipp32s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, IppHintAlgorithm hint ))
IPPAPI ( IppStatus, ippiScale_32f8u_C3R,
        (const Ipp32f* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, Ipp32f vMin, Ipp32f vMax ))
IPPAPI ( IppStatus, ippiScale_16u8u_AC4R,
        (const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, IppHintAlgorithm hint ))
IPPAPI ( IppStatus, ippiScale_16s8u_AC4R,
        (const Ipp16s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, IppHintAlgorithm hint ))
IPPAPI ( IppStatus, ippiScale_32s8u_AC4R,
        (const Ipp32s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, IppHintAlgorithm hint ))
IPPAPI ( IppStatus, ippiScale_32f8u_AC4R,
        (const Ipp32f* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, Ipp32f vMin, Ipp32f vMax ))
IPPAPI ( IppStatus, ippiScale_16u8u_C4R,
        (const Ipp16u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, IppHintAlgorithm hint ))
IPPAPI ( IppStatus, ippiScale_16s8u_C4R,
        (const Ipp16s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, IppHintAlgorithm hint ))
IPPAPI ( IppStatus, ippiScale_32s8u_C4R,
        (const Ipp32s* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, IppHintAlgorithm hint ))
IPPAPI ( IppStatus, ippiScale_32f8u_C4R,
        (const Ipp32f* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
         IppiSize roiSize, Ipp32f vMin, Ipp32f vMax ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiMin
//  Purpose:        computes the minimum of image pixel values
//  Returns:        IppStatus
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value
//  Parameters:
//    pSrc        Pointer to the source image.
//    srcStep     Step through the source image
//    roiSize     Size of the source image ROI.
//    pMin        Pointer to the result (C1)
//    min         Array containing results (C3, AC4, C4)
*/

IPPAPI(IppStatus, ippiMin_8u_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u* pMin))
IPPAPI(IppStatus, ippiMin_8u_C3R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u min[3]))
IPPAPI(IppStatus, ippiMin_8u_AC4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u min[3]))
IPPAPI(IppStatus, ippiMin_8u_C4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u min[4]))

IPPAPI(IppStatus, ippiMin_16s_C1R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s* pMin))
IPPAPI(IppStatus, ippiMin_16s_C3R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s min[3]))
IPPAPI(IppStatus, ippiMin_16s_AC4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s min[3]))
IPPAPI(IppStatus, ippiMin_16s_C4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s min[4]))

IPPAPI(IppStatus, ippiMin_16u_C1R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u* pMin))
IPPAPI(IppStatus, ippiMin_16u_C3R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u min[3]))
IPPAPI(IppStatus, ippiMin_16u_AC4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u min[3]))
IPPAPI(IppStatus, ippiMin_16u_C4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u min[4]))

IPPAPI(IppStatus, ippiMin_32f_C1R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f* pMin))
IPPAPI(IppStatus, ippiMin_32f_C3R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f min[3]))
IPPAPI(IppStatus, ippiMin_32f_AC4R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f min[3]))
IPPAPI(IppStatus, ippiMin_32f_C4R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f min[4]))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiMinIndx
//  Purpose:        computes the minimum of image pixel values and retrieves
//                  the x and y coordinates of pixels with this value
//  Returns:        IppStatus
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value
//  Parameters:
//    pSrc        Pointer to the source image.
//    srcStep     Step in bytes through the source image
//    roiSize     Size of the source image ROI.
//    pMin        Pointer to the result (C1)
//    min         Array of the results (C3, AC4, C4)
//    pIndexX     Pointer to the x coordinate of the pixel with min value (C1)
//    pIndexY     Pointer to the y coordinate of the pixel with min value (C1)
//    indexX      Array containing the x coordinates of the pixel with min value (C3, AC4, C4)
//    indexY      Array containing the y coordinates of the pixel with min value (C3, AC4, C4)
*/

IPPAPI(IppStatus, ippiMinIndx_8u_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u* pMin, int* pIndexX, int* pIndexY))
IPPAPI(IppStatus, ippiMinIndx_8u_C3R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u min[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMinIndx_8u_AC4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u min[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMinIndx_8u_C4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u min[4], int indexX[4], int indexY[4]))

IPPAPI(IppStatus, ippiMinIndx_16s_C1R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s* pMin, int* pIndexX, int* pIndexY))
IPPAPI(IppStatus, ippiMinIndx_16s_C3R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s min[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMinIndx_16s_AC4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s min[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMinIndx_16s_C4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s min[4], int indexX[4], int indexY[4]))

IPPAPI(IppStatus, ippiMinIndx_16u_C1R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u* pMin, int* pIndexX, int* pIndexY))
IPPAPI(IppStatus, ippiMinIndx_16u_C3R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u min[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMinIndx_16u_AC4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u min[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMinIndx_16u_C4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u min[4], int indexX[4], int indexY[4]))

IPPAPI(IppStatus, ippiMinIndx_32f_C1R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f* pMin, int* pIndexX, int* pIndexY))
IPPAPI(IppStatus, ippiMinIndx_32f_C3R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f min[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMinIndx_32f_AC4R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f min[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMinIndx_32f_C4R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f min[4], int indexX[4], int indexY[4]))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiMax
//  Purpose:        computes the maximum of image pixel values
//  Returns:        IppStatus
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value
//  Parameters:
//    pSrc        Pointer to the source image.
//    srcStep     Step in bytes through the source image
//    roiSize     Size of the source image ROI.
//    pMax        Pointer to the result (C1)
//    max         Array containing the results (C3, AC4, C4)
*/

IPPAPI(IppStatus, ippiMax_8u_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u* pMax))
IPPAPI(IppStatus, ippiMax_8u_C3R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u max[3]))
IPPAPI(IppStatus, ippiMax_8u_AC4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u max[3]))
IPPAPI(IppStatus, ippiMax_8u_C4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u max[4]))

IPPAPI(IppStatus, ippiMax_16s_C1R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s* pMax))
IPPAPI(IppStatus, ippiMax_16s_C3R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s max[3]))
IPPAPI(IppStatus, ippiMax_16s_AC4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s max[3]))
IPPAPI(IppStatus, ippiMax_16s_C4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s max[4]))

IPPAPI(IppStatus, ippiMax_16u_C1R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u* pMax))
IPPAPI(IppStatus, ippiMax_16u_C3R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u max[3]))
IPPAPI(IppStatus, ippiMax_16u_AC4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u max[3]))
IPPAPI(IppStatus, ippiMax_16u_C4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u max[4]))

IPPAPI(IppStatus, ippiMax_32f_C1R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f* pMax))
IPPAPI(IppStatus, ippiMax_32f_C3R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f max[3]))
IPPAPI(IppStatus, ippiMax_32f_AC4R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f max[3]))
IPPAPI(IppStatus, ippiMax_32f_C4R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f max[4]))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiMaxIndx
//  Purpose:        computes the maximum of image pixel values and retrieves
//                  the x and y coordinates of pixels with this value
//  Returns:        IppStatus
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value
//  Parameters:
//    pSrc        Pointer to the source image.
//    srcStep     Step in bytes through the source image
//    roiSize     Size of the source image ROI.
//    pMax        Pointer to the result (C1)
//    max         Array of the results (C3, AC4, C4)
//    pIndexX     Pointer to the x coordinate of the pixel with max value (C1)
//    pIndexY     Pointer to the y coordinate of the pixel with max value (C1)
//    indexX      Array containing the x coordinates of the pixel with max value (C3, AC4, C4)
//    indexY      Array containing the y coordinates of the pixel with max value (C3, AC4, C4)
*/

IPPAPI(IppStatus, ippiMaxIndx_8u_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u* pMax, int* pIndexX, int* pIndexY))
IPPAPI(IppStatus, ippiMaxIndx_8u_C3R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u max[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMaxIndx_8u_AC4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u max[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMaxIndx_8u_C4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u max[4], int indexX[4], int indexY[4]))

IPPAPI(IppStatus, ippiMaxIndx_16s_C1R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s* pMax, int* pIndexX, int* pIndexY))
IPPAPI(IppStatus, ippiMaxIndx_16s_C3R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s max[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMaxIndx_16s_AC4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s max[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMaxIndx_16s_C4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s max[4], int indexX[4], int indexY[4]))

IPPAPI(IppStatus, ippiMaxIndx_16u_C1R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u* pMax, int* pIndexX, int* pIndexY))
IPPAPI(IppStatus, ippiMaxIndx_16u_C3R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u max[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMaxIndx_16u_AC4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u max[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMaxIndx_16u_C4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u max[4], int indexX[4], int indexY[4]))

IPPAPI(IppStatus, ippiMaxIndx_32f_C1R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f* pMax, int* pIndexX, int* pIndexY))
IPPAPI(IppStatus, ippiMaxIndx_32f_C3R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f max[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMaxIndx_32f_AC4R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f max[3], int indexX[3], int indexY[3]))
IPPAPI(IppStatus, ippiMaxIndx_32f_C4R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f max[4], int indexX[4], int indexY[4]))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiMinMax
//  Purpose:        computes the minimum and maximum of image pixel value
//  Returns:        IppStatus
//    ippStsNoErr        OK
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsSizeErr      roiSize has a field with zero or negative value
//  Parameters:
//    pSrc        Pointer to the source image
//    srcStep     Step in bytes through the source image
//    roiSize     Size of the source image ROI.
//    pMin, pMax  Pointers to the results (C1)
//    min, max    Arrays containing the results (C3, AC4, C4)
*/

IPPAPI(IppStatus, ippiMinMax_8u_C1R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u* pMin, Ipp8u* pMax))
IPPAPI(IppStatus, ippiMinMax_8u_C3R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u min[3], Ipp8u max[3]))
IPPAPI(IppStatus, ippiMinMax_8u_AC4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u min[3], Ipp8u max[3]))
IPPAPI(IppStatus, ippiMinMax_8u_C4R, (const Ipp8u* pSrc, int srcStep, IppiSize roiSize, Ipp8u min[4], Ipp8u max[4]))

IPPAPI(IppStatus, ippiMinMax_16s_C1R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s* pMin, Ipp16s* pMax))
IPPAPI(IppStatus, ippiMinMax_16s_C3R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s min[3], Ipp16s max[3]))
IPPAPI(IppStatus, ippiMinMax_16s_AC4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s min[3], Ipp16s max[3]))
IPPAPI(IppStatus, ippiMinMax_16s_C4R, (const Ipp16s* pSrc, int srcStep, IppiSize roiSize, Ipp16s min[4], Ipp16s max[4]))

IPPAPI(IppStatus, ippiMinMax_16u_C1R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u* pMin, Ipp16u* pMax))
IPPAPI(IppStatus, ippiMinMax_16u_C3R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u min[3], Ipp16u max[3]))
IPPAPI(IppStatus, ippiMinMax_16u_AC4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u min[3], Ipp16u max[3]))
IPPAPI(IppStatus, ippiMinMax_16u_C4R, (const Ipp16u* pSrc, int srcStep, IppiSize roiSize, Ipp16u min[4], Ipp16u max[4]))

IPPAPI(IppStatus, ippiMinMax_32f_C1R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f* pMin, Ipp32f* pMax))
IPPAPI(IppStatus, ippiMinMax_32f_C3R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f min[3], Ipp32f max[3]))
IPPAPI(IppStatus, ippiMinMax_32f_AC4R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f min[3], Ipp32f max[3]))
IPPAPI(IppStatus, ippiMinMax_32f_C4R, (const Ipp32f* pSrc, int srcStep, IppiSize roiSize, Ipp32f min[4], Ipp32f max[4]))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:              ippiMinEvery, ippiMaxEvery
//  Purpose:            calculation min/max value for every element of two images
//  Parameters:
//   pSrc               pointer to input image
//   pSrcDst            pointer to input/output image
//    srcStep           Step in bytes through the source image
//    roiSize           Size of the source image ROI.
//  Return:
//   ippStsNullPtrErr   pointer(s) to the data is NULL
//   ippStsSizeErr      roiSize has a field with zero or negative value
//   ippStsNoErr        otherwise
*/

IPPAPI(IppStatus,ippiMaxEvery_8u_C1IR,(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_8u_C1IR,(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMaxEvery_16s_C1IR,(const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_16s_C1IR,(const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMaxEvery_16u_C1IR,(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_16u_C1IR,(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMaxEvery_32f_C1IR,(const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_32f_C1IR,(const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))

IPPAPI(IppStatus,ippiMaxEvery_8u_C3IR,(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_8u_C3IR,(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMaxEvery_16s_C3IR,(const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_16s_C3IR,(const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMaxEvery_16u_C3IR,(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_16u_C3IR,(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMaxEvery_32f_C3IR,(const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_32f_C3IR,(const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))

IPPAPI(IppStatus,ippiMaxEvery_8u_C4IR,(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_8u_C4IR,(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMaxEvery_16s_C4IR,(const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_16s_C4IR,(const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMaxEvery_16u_C4IR,(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_16u_C4IR,(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMaxEvery_32f_C4IR,(const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_32f_C4IR,(const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))

IPPAPI(IppStatus,ippiMaxEvery_8u_AC4IR,(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_8u_AC4IR,(const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMaxEvery_16s_AC4IR,(const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_16s_AC4IR,(const Ipp16s* pSrc, int srcStep, Ipp16s* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMaxEvery_16u_AC4IR,(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_16u_AC4IR,(const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMaxEvery_32f_AC4IR,(const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus,ippiMinEvery_32f_AC4IR,(const Ipp32f* pSrc, int srcStep, Ipp32f* pSrcDst,
                                                                   int srcDstStep, IppiSize roiSize))


/* /////////////////////////////////////////////////////////////////////////////////////////////////
//                      Logical Operations and Shift Functions
///////////////////////////////////////////////////////////////////////////////////////////////// */
/*
//  Names:          ippiAnd, ippiAndC, ippiOr, ippiOrC, ippiXor, ippiXorC, ippiNot,
//  Purpose:        Performs corresponding bitwise logical operation between pixels of two image
//                  (AndC/OrC/XorC  - between pixel of the source image and a constant)
//
//  Names:          ippiLShiftC, ippiRShiftC
//  Purpose:        Shifts bits in each pixel value to the left and right
//  Parameters:
//   value         1) The constant value to be ANDed/ORed/XORed with each pixel of the source,
//                     constant vector for multi-channel images;
//                 2) The number of bits to shift, constant vector for multi-channel images.
//   pSrc          Pointer to the source image
//   srcStep       Step through the source image
//   pSrcDst       Pointer to the source/destination image (in-place flavors)
//   srcDstStep    Step through the source/destination image (in-place flavors)
//   pSrc1         Pointer to first source image
//   src1Step      Step through first source image
//   pSrc2         Pointer to second source image
//   src2Step      Step through second source image
//   pDst          Pointer to the destination image
//   dstStep       Step in destination image
//   roiSize       Size of the ROI
//
//  Returns:
//   ippStsNullPtrErr   One of the pointers is NULL
//   ippStsStepErr      One of the step values is less than or equal to zero
//   ippStsSizeErr      roiSize has a field with zero or negative value
//   ippStsShiftErr     Shift's value is less than zero
//   ippStsNoErr        No errors
*/

IPPAPI(IppStatus, ippiAnd_8u_C1R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_8u_C3R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_8u_C4R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_8u_AC4R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_8u_C1IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_8u_C3IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_8u_C4IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_8u_AC4IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp8u value, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_8u_C3R, (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_8u_C4R, (const Ipp8u* pSrc, int srcStep, const Ipp8u value[4], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_8u_AC4R, (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_8u_C1IR, (Ipp8u value, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_8u_C3IR, (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_8u_C4IR, (const Ipp8u value[4], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_8u_AC4IR, (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_16u_C1R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_16u_C3R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_16u_C4R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_16u_AC4R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_16u_C1IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_16u_C3IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_16u_C4IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_16u_AC4IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_16u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp16u value, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_16u_C3R, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_16u_C4R, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[4], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_16u_AC4R, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_16u_C1IR, (Ipp16u value, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_16u_C3IR, (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_16u_C4IR, (const Ipp16u value[4], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_16u_AC4IR, (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_32s_C1R, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_32s_C3R, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_32s_C4R, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_32s_AC4R, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_32s_C1IR, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_32s_C3IR, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_32s_C4IR, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAnd_32s_AC4IR, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_32s_C1R, (const Ipp32s* pSrc, int srcStep, Ipp32s value, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_32s_C3R, (const Ipp32s* pSrc, int srcStep, const Ipp32s value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_32s_C4R, (const Ipp32s* pSrc, int srcStep, const Ipp32s value[4], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_32s_AC4R, (const Ipp32s* pSrc, int srcStep, const Ipp32s value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_32s_C1IR, (Ipp32s value, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_32s_C3IR, (const Ipp32s value[3], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_32s_C4IR, (const Ipp32s value[4], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiAndC_32s_AC4IR, (const Ipp32s value[3], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiOr_8u_C1R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_8u_C3R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_8u_C4R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_8u_AC4R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_8u_C1IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_8u_C3IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_8u_C4IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_8u_AC4IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp8u value, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_8u_C3R, (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_8u_C4R, (const Ipp8u* pSrc, int srcStep, const Ipp8u value[4], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_8u_AC4R, (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_8u_C1IR, (Ipp8u value, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_8u_C3IR, (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_8u_C4IR, (const Ipp8u value[4], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_8u_AC4IR, (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_16u_C1R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_16u_C3R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_16u_C4R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_16u_AC4R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_16u_C1IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_16u_C3IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_16u_C4IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_16u_AC4IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_16u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp16u value, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_16u_C3R, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_16u_C4R, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[4], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_16u_AC4R, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_16u_C1IR, (Ipp16u value, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_16u_C3IR, (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_16u_C4IR, (const Ipp16u value[4], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_16u_AC4IR, (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_32s_C1R, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_32s_C3R, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_32s_C4R, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_32s_AC4R, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_32s_C1IR, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_32s_C3IR, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_32s_C4IR, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOr_32s_AC4IR, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_32s_C1R, (const Ipp32s* pSrc, int srcStep, Ipp32s value, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_32s_C3R, (const Ipp32s* pSrc, int srcStep, const Ipp32s value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_32s_C4R, (const Ipp32s* pSrc, int srcStep, const Ipp32s value[4], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_32s_AC4R, (const Ipp32s* pSrc, int srcStep, const Ipp32s value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_32s_C1IR, (Ipp32s value, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_32s_C3IR, (const Ipp32s value[3], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_32s_C4IR, (const Ipp32s value[4], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiOrC_32s_AC4IR, (const Ipp32s value[3], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiXor_8u_C1R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_8u_C3R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_8u_C4R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_8u_AC4R, (const Ipp8u* pSrc1, int src1Step, const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_8u_C1IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_8u_C3IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_8u_C4IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_8u_AC4IR, (const Ipp8u* pSrc, int srcStep, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp8u value, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_8u_C3R, (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_8u_C4R, (const Ipp8u* pSrc, int srcStep, const Ipp8u value[4], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_8u_AC4R, (const Ipp8u* pSrc, int srcStep, const Ipp8u value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_8u_C1IR, (Ipp8u value, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_8u_C3IR, (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_8u_C4IR, (const Ipp8u value[4], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_8u_AC4IR, (const Ipp8u value[3], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_16u_C1R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_16u_C3R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_16u_C4R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_16u_AC4R, (const Ipp16u* pSrc1, int src1Step, const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_16u_C1IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_16u_C3IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_16u_C4IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_16u_AC4IR, (const Ipp16u* pSrc, int srcStep, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_16u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp16u value, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_16u_C3R, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_16u_C4R, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[4], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_16u_AC4R, (const Ipp16u* pSrc, int srcStep, const Ipp16u value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_16u_C1IR, (Ipp16u value, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_16u_C3IR, (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_16u_C4IR, (const Ipp16u value[4], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_16u_AC4IR, (const Ipp16u value[3], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_32s_C1R, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_32s_C3R, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_32s_C4R, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_32s_AC4R, (const Ipp32s* pSrc1, int src1Step, const Ipp32s* pSrc2, int src2Step, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_32s_C1IR, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_32s_C3IR, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_32s_C4IR, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXor_32s_AC4IR, (const Ipp32s* pSrc, int srcStep, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_32s_C1R, (const Ipp32s* pSrc, int srcStep, Ipp32s value, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_32s_C3R, (const Ipp32s* pSrc, int srcStep, const Ipp32s value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_32s_C4R, (const Ipp32s* pSrc, int srcStep, const Ipp32s value[4], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_32s_AC4R, (const Ipp32s* pSrc, int srcStep, const Ipp32s value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_32s_C1IR, (Ipp32s value, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_32s_C3IR, (const Ipp32s value[3], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_32s_C4IR, (const Ipp32s value[4], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiXorC_32s_AC4IR, (const Ipp32s value[3], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiNot_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiNot_8u_C3R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiNot_8u_C4R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiNot_8u_AC4R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiNot_8u_C1IR, (Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiNot_8u_C3IR, (Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiNot_8u_C4IR, (Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiNot_8u_AC4IR, (Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiLShiftC_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp32u value, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_8u_C3R, (const Ipp8u* pSrc, int srcStep, const Ipp32u value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_8u_C4R, (const Ipp8u* pSrc, int srcStep, const Ipp32u value[4], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_8u_AC4R, (const Ipp8u* pSrc, int srcStep, const Ipp32u value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_8u_C1IR, (Ipp32u value, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_8u_C3IR, (const Ipp32u value[3], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_8u_C4IR, (const Ipp32u value[4], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_8u_AC4IR, (const Ipp32u value[3], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_16u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp32u value, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_16u_C3R, (const Ipp16u* pSrc, int srcStep, const Ipp32u value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_16u_C4R, (const Ipp16u* pSrc, int srcStep, const Ipp32u value[4], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_16u_AC4R, (const Ipp16u* pSrc, int srcStep, const Ipp32u value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_16u_C1IR, (Ipp32u value, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_16u_C3IR, (const Ipp32u value[3], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_16u_C4IR, (const Ipp32u value[4], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_16u_AC4IR, (const Ipp32u value[3], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_32s_C1R, (const Ipp32s* pSrc, int srcStep, Ipp32u value, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_32s_C3R, (const Ipp32s* pSrc, int srcStep, const Ipp32u value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_32s_C4R, (const Ipp32s* pSrc, int srcStep, const Ipp32u value[4], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_32s_AC4R, (const Ipp32s* pSrc, int srcStep, const Ipp32u value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_32s_C1IR, (Ipp32u value, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_32s_C3IR, (const Ipp32u value[3], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_32s_C4IR, (const Ipp32u value[4], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiLShiftC_32s_AC4IR, (const Ipp32u value[3], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))

IPPAPI(IppStatus, ippiRShiftC_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp32u value, Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8u_C3R, (const Ipp8u* pSrc, int srcStep, const Ipp32u value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8u_C4R, (const Ipp8u* pSrc, int srcStep, const Ipp32u value[4], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8u_AC4R, (const Ipp8u* pSrc, int srcStep, const Ipp32u value[3], Ipp8u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8u_C1IR, (Ipp32u value, Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8u_C3IR, (const Ipp32u value[3], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8u_C4IR, (const Ipp32u value[4], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8u_AC4IR, (const Ipp32u value[3], Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8s_C1R, (const Ipp8s* pSrc, int srcStep, Ipp32u value, Ipp8s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8s_C3R, (const Ipp8s* pSrc, int srcStep, const Ipp32u value[3], Ipp8s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8s_C4R, (const Ipp8s* pSrc, int srcStep, const Ipp32u value[4], Ipp8s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8s_AC4R, (const Ipp8s* pSrc, int srcStep, const Ipp32u value[3], Ipp8s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8s_C1IR, (Ipp32u value, Ipp8s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8s_C3IR, (const Ipp32u value[3], Ipp8s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8s_C4IR, (const Ipp32u value[4], Ipp8s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_8s_AC4IR, (const Ipp32u value[3], Ipp8s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp32u value, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16u_C3R, (const Ipp16u* pSrc, int srcStep, const Ipp32u value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16u_C4R, (const Ipp16u* pSrc, int srcStep, const Ipp32u value[4], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16u_AC4R, (const Ipp16u* pSrc, int srcStep, const Ipp32u value[3], Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16u_C1IR, (Ipp32u value, Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16u_C3IR, (const Ipp32u value[3], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16u_C4IR, (const Ipp32u value[4], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16u_AC4IR, (const Ipp32u value[3], Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16s_C1R, (const Ipp16s* pSrc, int srcStep, Ipp32u value, Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16s_C3R, (const Ipp16s* pSrc, int srcStep, const Ipp32u value[3], Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16s_C4R, (const Ipp16s* pSrc, int srcStep, const Ipp32u value[4], Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16s_AC4R, (const Ipp16s* pSrc, int srcStep, const Ipp32u value[3], Ipp16s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16s_C1IR, (Ipp32u value, Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16s_C3IR, (const Ipp32u value[3], Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16s_C4IR, (const Ipp32u value[4], Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_16s_AC4IR, (const Ipp32u value[3], Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_32s_C1R, (const Ipp32s* pSrc, int srcStep, Ipp32u value, Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_32s_C3R, (const Ipp32s* pSrc, int srcStep, const Ipp32u value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_32s_C4R, (const Ipp32s* pSrc, int srcStep, const Ipp32u value[4], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_32s_AC4R, (const Ipp32s* pSrc, int srcStep, const Ipp32u value[3], Ipp32s* pDst, int dstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_32s_C1IR, (Ipp32u value, Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_32s_C3IR, (const Ipp32u value[3], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_32s_C4IR, (const Ipp32u value[4], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI(IppStatus, ippiRShiftC_32s_AC4IR, (const Ipp32u value[3], Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize))


/* /////////////////////////////////////////////////////////////////////////////////////////////////
//                              Compare Operations
///////////////////////////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippiCompare
//                  ippiCompareC
//  Purpose:  Compares pixel values of two images, or pixel values of an image to a constant
//            value using the following compare conditions: <, <=, ==, >, >= ;
//  Names:          ippiCompareEqualEps
//                  ippiCompareEqualEpsC
//  Purpose:  Compares 32f images for being equal, or equal to a given value within given tolerance
//  Context:
//
//  Returns:        IppStatus
//    ippStsNoErr        No errors
//    ippStsNullPtrErr   One of the pointers is NULL
//    ippStsStepErr      One of the step values is less than or equal to zero
//    ippStsSizeErr      roiSize has a field with zero or negative value
//    ippStsEpsValErr    eps is negative
//
//  Parameters:
//    pSrc1         Pointer to the first source image;
//    src1Step      Step through the first source image;
//    pSrc2         Pointer to the second source image data;
//    src2Step      Step through the second source image;
//    pDst          Pointer to destination image data;
//    dstStep       Step in destination image;
//    roiSize       Size of the ROI;
//    ippCmpOp      Compare operation to be used
//    value         Value (array of values for multi-channel image) to compare
//                  each pixel to
//    eps           The tolerance value
//
//  Notes:
*/

IPPAPI (IppStatus, ippiCompare_8u_C1R, ( const Ipp8u* pSrc1, int src1Step,
                                         const Ipp8u* pSrc2, int src2Step,
                                               Ipp8u* pDst,  int dstStep,
                                         IppiSize roiSize,   IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompare_8u_C3R, ( const Ipp8u* pSrc1, int src1Step,
                                         const Ipp8u* pSrc2, int src2Step,
                                               Ipp8u* pDst,  int dstStep,
                                         IppiSize roiSize,   IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompare_8u_AC4R, (const Ipp8u* pSrc1, int src1Step,
                                         const Ipp8u* pSrc2, int src2Step,
                                               Ipp8u* pDst,  int dstStep,
                                         IppiSize roiSize,   IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompare_8u_C4R, ( const Ipp8u* pSrc1, int src1Step,
                                         const Ipp8u* pSrc2, int src2Step,
                                               Ipp8u* pDst,  int dstStep,
                                         IppiSize roiSize,   IppCmpOp ippCmpOp))

IPPAPI (IppStatus, ippiCompareC_8u_C1R,(const Ipp8u* pSrc, int srcStep, Ipp8u value,
                                              Ipp8u* pDst, int dstStep,
                                        IppiSize roiSize,  IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompareC_8u_C3R,(const Ipp8u* pSrc, int srcStep, const Ipp8u value[3],
                                              Ipp8u* pDst, int dstStep,
                                        IppiSize roiSize,  IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompareC_8u_AC4R,( const Ipp8u* pSrc, int srcStep, const Ipp8u value[3],
                                                Ipp8u* pDst, int dstStep,
                                          IppiSize roiSize,  IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompareC_8u_C4R,(const Ipp8u* pSrc, int srcStep, const Ipp8u value[4],
                                              Ipp8u* pDst, int dstStep,
                                        IppiSize roiSize,  IppCmpOp ippCmpOp))

IPPAPI (IppStatus, ippiCompare_16s_C1R, ( const Ipp16s* pSrc1, int src1Step,
                                          const Ipp16s* pSrc2, int src2Step,
                                                Ipp8u*  pDst,  int dstStep,
                                          IppiSize roiSize,    IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompare_16s_C3R, ( const Ipp16s* pSrc1, int src1Step,
                                          const Ipp16s* pSrc2, int src2Step,
                                                Ipp8u*  pDst,  int dstStep,
                                          IppiSize roiSize,    IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompare_16s_AC4R, (const Ipp16s* pSrc1, int src1Step,
                                          const Ipp16s* pSrc2, int src2Step,
                                                Ipp8u*  pDst,  int dstStep,
                                          IppiSize roiSize,    IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompare_16s_C4R, ( const Ipp16s* pSrc1, int src1Step,
                                          const Ipp16s* pSrc2, int src2Step,
                                                Ipp8u*  pDst,  int dstStep,
                                          IppiSize roiSize,    IppCmpOp ippCmpOp))

IPPAPI (IppStatus, ippiCompareC_16s_C1R, ( const Ipp16s* pSrc, int srcStep, Ipp16s value,
                                                Ipp8u*  pDst,  int dstStep,
                                           IppiSize roiSize,   IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompareC_16s_C3R, ( const Ipp16s* pSrc, int srcStep, const Ipp16s value[3],
                                                 Ipp8u*  pDst, int dstStep,
                                           IppiSize roiSize,   IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompareC_16s_AC4R, ( const Ipp16s* pSrc, int srcStep, const Ipp16s value[3],
                                                  Ipp8u*  pDst, int dstStep,
                                            IppiSize roiSize,   IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompareC_16s_C4R, ( const Ipp16s* pSrc, int srcStep, const Ipp16s value[4],
                                                 Ipp8u*  pDst, int dstStep,
                                           IppiSize roiSize,   IppCmpOp ippCmpOp))

IPPAPI (IppStatus, ippiCompare_16u_C1R, ( const Ipp16u* pSrc1, int src1Step,
                                          const Ipp16u* pSrc2, int src2Step,
                                                Ipp8u*  pDst,  int dstStep,
                                          IppiSize roiSize,    IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompare_16u_C3R, ( const Ipp16u* pSrc1, int src1Step,
                                          const Ipp16u* pSrc2, int src2Step,
                                                Ipp8u*  pDst,  int dstStep,
                                          IppiSize roiSize,    IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompare_16u_AC4R, (const Ipp16u* pSrc1, int src1Step,
                                          const Ipp16u* pSrc2, int src2Step,
                                                Ipp8u*  pDst,  int dstStep,
                                          IppiSize roiSize,    IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompare_16u_C4R, ( const Ipp16u* pSrc1, int src1Step,
                                          const Ipp16u* pSrc2, int src2Step,
                                                Ipp8u*  pDst,  int dstStep,
                                          IppiSize roiSize,    IppCmpOp ippCmpOp))

IPPAPI (IppStatus, ippiCompareC_16u_C1R, ( const Ipp16u* pSrc, int srcStep, Ipp16u value,
                                                Ipp8u*  pDst,  int dstStep,
                                           IppiSize roiSize,   IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompareC_16u_C3R, ( const Ipp16u* pSrc, int srcStep, const Ipp16u value[3],
                                                 Ipp8u*  pDst, int dstStep,
                                           IppiSize roiSize,   IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompareC_16u_AC4R, ( const Ipp16u* pSrc, int srcStep, const Ipp16u value[3],
                                                  Ipp8u*  pDst, int dstStep,
                                            IppiSize roiSize,   IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompareC_16u_C4R, ( const Ipp16u* pSrc, int srcStep, const Ipp16u value[4],
                                                 Ipp8u*  pDst, int dstStep,
                                           IppiSize roiSize,   IppCmpOp ippCmpOp))

IPPAPI (IppStatus, ippiCompare_32f_C1R, ( const Ipp32f* pSrc1, int src1Step,
                                          const Ipp32f* pSrc2, int src2Step,
                                                 Ipp8u*  pDst, int dstStep,
                                          IppiSize roiSize,    IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompare_32f_C3R, ( const Ipp32f* pSrc1, int src1Step,
                                          const Ipp32f* pSrc2, int src2Step,
                                                 Ipp8u*  pDst, int dstStep,
                                          IppiSize roiSize,    IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompare_32f_AC4R,( const Ipp32f* pSrc1, int src1Step,
                                          const Ipp32f* pSrc2, int src2Step,
                                                 Ipp8u*  pDst, int dstStep,
                                          IppiSize roiSize,    IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompare_32f_C4R, ( const Ipp32f* pSrc1, int src1Step,
                                          const Ipp32f* pSrc2, int src2Step,

                                                 Ipp8u*  pDst, int dstStep,
                                          IppiSize roiSize,    IppCmpOp ippCmpOp))

IPPAPI (IppStatus, ippiCompareC_32f_C1R, ( const Ipp32f* pSrc, int srcStep, Ipp32f value,
                                                 Ipp8u*  pDst, int dstStep,
                                           IppiSize roiSize,   IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompareC_32f_C3R, ( const Ipp32f* pSrc, int srcStep, const Ipp32f value[3],
                                                 Ipp8u*  pDst, int dstStep,
                                           IppiSize roiSize,   IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompareC_32f_AC4R, ( const Ipp32f* pSrc, int srcStep, const Ipp32f value[3],
                                                  Ipp8u*  pDst, int dstStep,
                                            IppiSize roiSize,   IppCmpOp ippCmpOp))
IPPAPI (IppStatus, ippiCompareC_32f_C4R, ( const Ipp32f* pSrc, int srcStep, const Ipp32f value[4],
                                                 Ipp8u*  pDst, int dstStep,
                                           IppiSize roiSize,   IppCmpOp ippCmpOp))

IPPAPI(IppStatus,ippiCompareEqualEps_32f_C1R,(const Ipp32f* pSrc1, int src1Step,
                                              const Ipp32f* pSrc2, int src2Step,
                                                    Ipp8u*  pDst,  int dstStep,
                                              IppiSize roiSize,    Ipp32f eps))
IPPAPI(IppStatus,ippiCompareEqualEps_32f_C3R,(const Ipp32f* pSrc1, int src1Step,
                                              const Ipp32f* pSrc2, int src2Step,
                                                    Ipp8u*  pDst,  int dstStep,
                                              IppiSize roiSize,    Ipp32f eps))
IPPAPI(IppStatus,ippiCompareEqualEps_32f_AC4R,(const Ipp32f* pSrc1,int src1Step,
                                               const Ipp32f* pSrc2,int src2Step,
                                                     Ipp8u*  pDst,  int dstStep,
                                               IppiSize roiSize,   Ipp32f eps))
IPPAPI(IppStatus,ippiCompareEqualEps_32f_C4R,(const Ipp32f* pSrc1, int src1Step,
                                              const Ipp32f* pSrc2, int src2Step,
                                                    Ipp8u*  pDst,  int dstStep,
                                              IppiSize roiSize,    Ipp32f eps))

IPPAPI(IppStatus,ippiCompareEqualEpsC_32f_C1R,(const Ipp32f* pSrc, int srcStep, Ipp32f value,
                                                    Ipp8u*   pDst,  int dstStep,
                                               IppiSize roiSize,   Ipp32f eps))
IPPAPI(IppStatus,ippiCompareEqualEpsC_32f_C3R,(const Ipp32f* pSrc, int srcStep,const Ipp32f value[3],
                                                     Ipp8u*  pDst, int dstStep,
                                               IppiSize roiSize,   Ipp32f eps))
IPPAPI(IppStatus,ippiCompareEqualEpsC_32f_AC4R,(const Ipp32f* pSrc, int srcStep,const Ipp32f value[3],
                                                      Ipp8u*  pDst, int dstStep,
                                                IppiSize roiSize,   Ipp32f eps))
IPPAPI(IppStatus,ippiCompareEqualEpsC_32f_C4R,(const Ipp32f* pSrc, int srcStep,const Ipp32f value[4],
                                                     Ipp8u*  pDst, int dstStep,
                                               IppiSize roiSize,   Ipp32f eps))

/* /////////////////////////////////////////////////////////////////////////////////////////////////
//                 Morphological Operations
///////////////////////////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiErode3x3_8u_C1R()    ippiDilate3x3_8u_C1R()
//             ippiErode3x3_8u_C3R()    ippiDilate3x3_8u_C3R()
//             ippiErode3x3_8u_AC4R()   ippiDilate3x3_8u_AC4R()
//             ippiErode3x3_8u_C4R()    ippiDilate3x3_8u_C4R()
//
//             ippiErode3x3_32f_C1R()   ippiDilate3x3_32f_C1R()
//             ippiErode3x3_32f_C3R()   ippiDilate3x3_32f_C3R()
//             ippiErode3x3_32f_AC4R()  ippiDilate3x3_32f_AC4R()
//             ippiErode3x3_32f_C4R()   ippiDilate3x3_32f_C4R()
//
//  Purpose:   Performs not in-place erosion/dilation using a 3x3 mask
//
//  Returns:
//    ippStsNullPtrErr   pSrc == NULL or pDst == NULL
//    ippStsStepErr      srcStep <= 0 or dstStep <= 0
//    ippStsSizeErr      roiSize has a field with zero or negative value
//    ippStsStrideErr    (2+roiSize.width)*nChannels*sizeof(item) > srcStep or
//                       (2+roiSize.width)*nChannels*sizeof(item) > dstStep
//    ippStsNoErr        No errors
//
//  Parameters:
//    pSrc          Pointer to the source image ROI
//    srcStep       Step (bytes) through the source image
//    pDst          Pointer to the destination image ROI
//    dstStep       Step (bytes) through the destination image
//    roiSize       Size of the ROI
*/
IPPAPI (IppStatus, ippiErode3x3_8u_C1R,   (const Ipp8u*  pSrc, int srcStep, Ipp8u*  pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_8u_C3R,   (const Ipp8u*  pSrc, int srcStep, Ipp8u*  pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_8u_AC4R,  (const Ipp8u*  pSrc, int srcStep, Ipp8u*  pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_8u_C4R,   (const Ipp8u*  pSrc, int srcStep, Ipp8u*  pDst, int dstStep, IppiSize roiSize))

IPPAPI (IppStatus, ippiDilate3x3_8u_C1R,  (const Ipp8u*  pSrc, int srcStep, Ipp8u*  pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_8u_C3R,  (const Ipp8u*  pSrc, int srcStep, Ipp8u*  pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_8u_AC4R, (const Ipp8u*  pSrc, int srcStep, Ipp8u*  pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_8u_C4R,  (const Ipp8u*  pSrc, int srcStep, Ipp8u*  pDst, int dstStep, IppiSize roiSize))

IPPAPI (IppStatus, ippiErode3x3_32f_C1R,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_32f_C3R,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_32f_AC4R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_32f_C4R,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize))

IPPAPI (IppStatus, ippiDilate3x3_32f_C1R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_32f_C3R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_32f_AC4R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_32f_C4R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize))


IPPAPI (IppStatus, ippiErode3x3_16u_C1R,  (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_16u_C3R,  (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_16u_C4R,  (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_16u_AC4R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize))

IPPAPI (IppStatus, ippiDilate3x3_16u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_16u_C3R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_16u_AC4R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_16u_C4R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiErode3x3_8u_C1IR()    ippiDilate3x3_8u_C1IR()
//             ippiErode3x3_8u_C3IR()    ippiDilate3x3_8u_C3IR()
//             ippiErode3x3_8u_AC4IR()   ippiDilate3x3_8u_AC4IR()
//             ippiErode3x3_8u_C4IR()    ippiDilate3x3_8u_C4IR()
//
//             ippiErode3x3_32f_C1IR()   ippiDilate3x3_32f_C1IR()
//             ippiErode3x3_32f_C3IR()   ippiDilate3x3_32f_C3IR()
//             ippiErode3x3_32f_AC4IR()  ippiDilate3x3_32f_AC4IR()
//             ippiErode3x3_32f_C4IR()   ippiDilate3x3_32f_C4IR()
//
//  Purpose:   Performs in-place erosion/dilation using a 3x3 mask
//
//  Returns:
//    ippStsNullPtrErr    pSrcDst == NULL
//    ippStsStepErr       srcDstStep <= 0
//    ippStsSizeErr       roiSize.width  <1 or
//                        roiSize.height <1
//    ippStsStrideErr     (2+roiSize.width)*nChannels*sizeof(item) > srcDstStep
//    ippStsMemAllocErr   Memory allocation fails
//    ippStsNoErr         No errors
//
//  Parameters:
//    pSrcDst     Pointer to the source/destination image IROI
//    srcDstStep  Step (bytes) through the source/destination image
//    roiSize     Size of the ROI
*/
IPPAPI (IppStatus, ippiErode3x3_8u_C1IR,   (Ipp8u*  pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_8u_C3IR,   (Ipp8u*  pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_8u_AC4IR,  (Ipp8u*  pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_8u_C4IR,   (Ipp8u*  pSrcDst, int srcDstStep, IppiSize roiSize))

IPPAPI (IppStatus, ippiDilate3x3_8u_C1IR,  (Ipp8u*  pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_8u_C3IR,  (Ipp8u*  pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_8u_AC4IR, (Ipp8u*  pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_8u_C4IR,  (Ipp8u*  pSrcDst, int srcDstStep, IppiSize roiSize))

IPPAPI (IppStatus, ippiErode3x3_32f_C1IR,  (Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_32f_C3IR,  (Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_32f_AC4IR, (Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_32f_C4IR,  (Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))

IPPAPI (IppStatus, ippiDilate3x3_32f_C1IR, (Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_32f_C3IR, (Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_32f_C4IR, (Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize))


IPPAPI (IppStatus, ippiErode3x3_16u_C1IR,  (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_16u_C3IR,  (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_16u_C4IR,  (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiErode3x3_16u_AC4IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))

IPPAPI (IppStatus, ippiDilate3x3_16u_C1IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_16u_C3IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_16u_C4IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))
IPPAPI (IppStatus, ippiDilate3x3_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiErode_8u_C1R()   ippiDilate_8u_C1R()
//             ippiErode_8u_C3R()   ippiDilate_8u_C3R()
//             ippiErode_8u_C4R()   ippiDilate_8u_C4R()
//             ippiErode_8u_AC4R()  ippiDilate_8u_AC4R()
//
//             ippiErode_32f_C1R()  ippiDilate_32f_C1R()
//             ippiErode_32f_C3R()  ippiDilate_32f_C3R()
//             ippiErode_32f_C4R()  ippiDilate_32f_C4R()
//             ippiErode_32f_AC4R() ippiDilate_32f_AC4R()
//
//  Purpose:   Performs not in-place erosion/dilation using an arbitrary mask
//
//  Returns:
//    ippStsNullPtrErr,   if pSrc == NULL or
//                           pDst == NULL or
//                           pMask== NULL
//    ippStsStepErr,      if srcStep <= 0 or
//                           dstStep <= 0
//    ippStsSizeErr,      if dstRoiSize.width  <1 or
//                           dstRoiSize.height <1
//    ippStsSizeErr,      if maskSize.width  <1 or
//                           maskSize.height <1
//    ippStsAnchorErr,    if (0>anchor.x)||(anchor.x>=maskSize.width) or
//                           (0>anchor.y)||(anchor.y>=maskSize.height)
//    ippStsStrideErr,    if (maskSize.width-1+dstRoiSize.width)*nChannels*sizeof(item)) > srcStep or
//                           (maskSize.width-1+dstRoiSize.width)*nChannels*sizeof(item)) > dstStep
//    ippStsMemAllocErr,  if can not allocate memory
//    ippStsZeroMaskValuesErr, if all values of the mask are zero
//    ippStsNoErr,        if no errors
//
//  Parameters:
//    pSrc          pointer to the source image ROI
//    srcStep       source image scan-line size (bytes)
//    pDst          pointer to the target image ROI
//    dstStep       target image scan-line size (bytes)
//    dstRoiSize    size of ROI
//    pMask         pointer to the mask
//    maskSize      size of mask
//    anchor        position of the anchor
*/
IPPAPI (IppStatus, ippiErode_8u_C1R,   (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_8u_C3R,   (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_8u_C4R,   (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_8u_AC4R,  (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))

IPPAPI (IppStatus, ippiDilate_8u_C1R,  (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_8u_C3R,  (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_8u_C4R,  (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_8u_AC4R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))

IPPAPI (IppStatus, ippiErode_32f_C1R,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_32f_C3R,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_32f_C4R,  (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_32f_AC4R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))

IPPAPI (IppStatus, ippiDilate_32f_C1R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_32f_C3R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_32f_C4R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_32f_AC4R,(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize dstRoiSize,
                             const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))


IPPAPI (IppStatus, ippiErode_16u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                       const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_16u_C3R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                       const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_16u_C4R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                       const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_16u_AC4R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                       const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))

IPPAPI (IppStatus, ippiDilate_16u_C1R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                        const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_16u_C3R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                        const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_16u_C4R, (const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                        const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_16u_AC4R,(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize dstRoiSize,
                                        const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:      ippiErode_8u_C1IR()   ippiDilate_8u_C1IR()
//             ippiErode_8u_C3IR()   ippiDilate_8u_C3IR()
//             ippiErode_8u_AC4IR()  ippiDilate_8u_AC4IR()
//
//             ippiErode_32f_C1IR()  ippiDilate_32f_C1IR()
//             ippiErode_32f_C3IR()  ippiDilate_32f_C3IR()
//             ippiErode_32f_AC4IR() ippiDilate_32f_AC4IR()
//
//  Purpose:   Performs in-place erosion/dilation using an arbitrary mask
//
//  Returns:
//    ippStsNullPtrErr,   if pSrcDst == NULL or
//                           pMask== NULL
//    ippStsStepErr,      if srcDstStep <= 0
//    ippStsSizeErr,      if dstRoiSize.width  <1 or
//                           dstRoiSize.height <1
//    ippStsSizeErr,      if maskSize.width  <1 or
//                           maskSize.height <1
//    ippStsAnchorErr,    if (0>anchor.x)||(anchor.x>=maskSize.width) or
//                           (0>anchor.y)||(anchor.y>=maskSize.height)
//    ippStsStrideErr,    if (maskSize.width-1+dstRoiSize.width)*nChannels*sizeof(item)) > srcDstStep
//    ippStsMemAllocErr,  if can not allocate memory
//    ippStsZeroMaskValuesErr, if all values of the mask are zero
//    ippStsNoErr,        if no errors
//
//  Parameters:
//    pSrcDst       pointer to the source image ROI
//    srcDstStep    source image scan-line size (bytes)
//    dstRoiSize    size of ROI
//    pMask         pointer to the mask
//    maskSize      size of mask
//    anchor        position of the anchor
*/
IPPAPI (IppStatus, ippiErode_8u_C1IR,   (Ipp8u* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                              const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_8u_C3IR,   (Ipp8u* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                              const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_8u_AC4IR,  (Ipp8u* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                              const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))

IPPAPI (IppStatus, ippiDilate_8u_C1IR,  (Ipp8u* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                              const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_8u_C3IR,  (Ipp8u* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                              const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_8u_AC4IR, (Ipp8u* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                              const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))

IPPAPI (IppStatus, ippiErode_32f_C1IR,  (Ipp32f* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                              const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_32f_C3IR,  (Ipp32f* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                              const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_32f_AC4IR, (Ipp32f* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                              const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))

IPPAPI (IppStatus, ippiDilate_32f_C1IR, (Ipp32f* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                              const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_32f_C3IR, (Ipp32f* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                              const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_32f_AC4IR,(Ipp32f* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                              const Ipp8u*  pMask, IppiSize maskSize, IppiPoint anchor))

IPPAPI (IppStatus, ippiErode_16u_C1IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                                   const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_16u_C3IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                                   const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiErode_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                                   const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))

IPPAPI (IppStatus, ippiDilate_16u_C1IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                                    const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_16u_C3IR, (Ipp16u* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                                    const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))
IPPAPI (IppStatus, ippiDilate_16u_AC4IR,(Ipp16u* pSrcDst, int srcDstStep, IppiSize dstRoiSize,
                                    const Ipp8u* pMask, IppiSize maskSize, IppiPoint anchor))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:
//    ippiZigzagInv8x8_16s_C1
//    ippiZigzagFwd8x8_16s_C1
//
//  Purpose:
//    Converts a natural order  to zigzag in an 8x8 block (forward function),
//    converts a zigzag order to natural  in a 8x8 block (inverse function)
//
//  Parameter:
//    pSrc   Pointer to the source block
//    pDst   Pointer to the destination block
//
//  Returns:
//    ippStsNoErr      No errors
//    ippStsNullPtrErr One of the pointers is NULL
//
*/

IPPAPI(IppStatus, ippiZigzagInv8x8_16s_C1,(const Ipp16s* pSrc, Ipp16s* pDst))
IPPAPI(IppStatus, ippiZigzagFwd8x8_16s_C1, (const Ipp16s* pSrc, Ipp16s* pDst))


/* /////////////////////////////////////////////////////////////////////////////
//                         Windowing functions
//
//  Note: to obtain the window coefficients you have apply the corresponding
//        function to the image with all pixel values set to 1 (this image can
//        be created, for example, calling function ippiSet(1,x,n))
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:            ippiWinBartlett,   ippiWinBartlettSep
//  Purpose:   Applies Bartlett windowing function to an image
//  Parameters:
//    pSrc             Pointer to the source image
//    srcStep          Step through the source image
//    pDst             Pointer to the destination image
//    dstStep          Step through the destination image
//    pSrcDst          Pointer to the source/destination image (in-place flavors)
//    srcDstStep       Step through the source/destination image (in-place flavors)
//    roiSize          Size of the ROI
//  Returns:
//    ippStsNullPtrErr One of the pointers is NULL
//    ippStsSizeErr    roiSize has a field with value less than 3
//    ippStsNoErr      No errors
*/

IPPAPI(IppStatus, ippiWinBartlett_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep,
                                                          IppiSize roiSize))

IPPAPI(IppStatus, ippiWinBartlett_8u_C1IR, (Ipp8u* pSrcDst, int srcDstStep,
                                                          IppiSize roiSize))

IPPAPI(IppStatus, ippiWinBartlettSep_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep,
                                                          IppiSize roiSize))

IPPAPI(IppStatus, ippiWinBartlettSep_8u_C1IR, (Ipp8u* pSrcDst, int srcDstStep,
                                                          IppiSize roiSize))

IPPAPI(IppStatus, ippiWinBartlett_32f_C1R,(const Ipp32f* pSrc, int srcStep,
                                                 Ipp32f* pDst, int dstStep,
                                                          IppiSize roiSize))

IPPAPI(IppStatus, ippiWinBartlett_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                                                 Ipp8u* pDst, int dstStep,
                                                          IppiSize roiSize))

IPPAPI(IppStatus, ippiWinBartlettSep_32f_C1R,(const Ipp32f* pSrc, int srcStep,
                                                    Ipp32f* pDst, int dstStep,
                                                          IppiSize roiSize))

IPPAPI(IppStatus, ippiWinBartlettSep_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                                                    Ipp8u* pDst, int dstStep,
                                                          IppiSize roiSize))

IPPAPI(IppStatus, ippiWinBartlett_16u_C1IR, (Ipp16u* pSrcDst, int srcDstStep,
                                                               IppiSize roiSize))
IPPAPI(IppStatus, ippiWinBartlettSep_16u_C1IR,(Ipp16u* pSrcDst, int srcDstStep,
                                                               IppiSize roiSize))
IPPAPI(IppStatus, ippiWinBartlett_16u_C1R, (const Ipp16u* pSrc, int srcStep,
                                                  Ipp16u* pDst, int dstStep,
                                                               IppiSize roiSize))
IPPAPI(IppStatus, ippiWinBartlettSep_16u_C1R,(const Ipp16u* pSrc, int srcStep,
                                                    Ipp16u* pDst, int dstStep,
                                                               IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:            ippiWinHamming,    ippiWinHammimgSep
//  Purpose:
//  Parameters:
//    pSrc             Pointer to the source image
//    srcStep          Step through the source image
//    pDst             Pointer to the destination image
//    dstStep          Step through the destination image
//    pSrcDst          Pointer to the source/destination image (in-place flavors)
//    srcDstStep       Step through the source/destination image (in-place flavors)
//    roiSize          Size of the ROI
//  Returns:
//    ippStsNullPtrErr One of the pointers is NULL
//    ippStsSizeErr    roiSize has a field with value less than 3
//    ippStsNoErr      No errors
*/

IPPAPI(IppStatus, ippiWinHamming_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep,
                                                           IppiSize roiSize))

IPPAPI(IppStatus, ippiWinHamming_8u_C1IR, (Ipp8u* pSrcDst, int srcDstStep,
                                                           IppiSize roiSize))

IPPAPI(IppStatus, ippiWinHammingSep_32f_C1IR,(Ipp32f* pSrcDst, int srcDstStep,
                                                           IppiSize roiSize))

IPPAPI(IppStatus, ippiWinHammingSep_8u_C1IR, (Ipp8u* pSrcDst, int srcDstStep,
                                                           IppiSize roiSize))

IPPAPI(IppStatus, ippiWinHamming_32f_C1R,(const Ipp32f* pSrc, int srcStep,
                                                 Ipp32f* pDst, int dstStep,
                                                           IppiSize roiSize))

IPPAPI(IppStatus, ippiWinHamming_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                                                 Ipp8u* pDst, int dstStep,
                                                           IppiSize roiSize))

IPPAPI(IppStatus, ippiWinHammingSep_32f_C1R,(const Ipp32f* pSrc, int srcStep,
                                                    Ipp32f* pDst, int dstStep,
                                                           IppiSize roiSize))

IPPAPI(IppStatus, ippiWinHammingSep_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                                                    Ipp8u* pDst, int dstStep,
                                                           IppiSize roiSize))

IPPAPI(IppStatus, ippiWinHamming_16u_C1IR, (Ipp16u* pSrcDst, int srcDstStep,
                                                               IppiSize roiSize))
IPPAPI(IppStatus, ippiWinHammingSep_16u_C1IR,(Ipp16u* pSrcDst, int srcDstStep,
                                                               IppiSize roiSize))
IPPAPI(IppStatus, ippiWinHamming_16u_C1R,(const Ipp16u* pSrc, int srcStep,
                                                Ipp16u* pDst, int dstStep,
                                                               IppiSize roiSize))
IPPAPI(IppStatus, ippiWinHammingSep_16u_C1R,(const Ipp16u* pSrc, int srcStep,
                                                   Ipp16u* pDst, int dstStep,
                                                               IppiSize roiSize))

/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:        ippiTranspose
//
//  Purpose:     Transposing an image
//
//  Returns:     IppStatus
//    ippStsNoErr         No errors
//    ippStsNullPtrErr    pSrc == NULL, or pDst == NULL
//    ippStsSizeErr       roiSize has a field with zero or negative value,
//                        and roiSize.width != roiSize.height (in-place flavors)
//
//  Parameters:
//    pSrc       Pointer to the source image
//    srcStep    Step through the source image
//    pDst       Pointer to the destination image
//    dstStep    Step through the destination image
//    pSrcDst    Pointer to the source/destination image (in-place flavors)
//    srcDstStep Step through the source/destination image (in-place flavors)
//    roiSize    Size of the ROI
//
//  Notes: Parameters roiSize.width and roiSize.height are defined for the source image.
//
*/

IPPAPI ( IppStatus, ippiTranspose_8u_C1R,
                    ( const Ipp8u* pSrc, int srcStep,
                            Ipp8u* pDst, int dstStep,
                                    IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_8u_C3R,
                    ( const Ipp8u* pSrc, int srcStep,
                            Ipp8u* pDst, int dstStep,
                                    IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_8u_C4R,
                    ( const Ipp8u* pSrc, int srcStep,
                            Ipp8u* pDst, int dstStep,
                                    IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_8u_C1IR,
                    ( Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_8u_C3IR,
                    ( Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_8u_C4IR,
                    ( Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_16u_C1R,
                    ( const Ipp16u* pSrc, int srcStep,
                            Ipp16u* pDst, int dstStep,
                                     IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_16u_C3R,
                    ( const Ipp16u* pSrc, int srcStep,
                            Ipp16u* pDst, int dstStep,
                                     IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_16u_C4R,
                    ( const Ipp16u* pSrc, int srcStep,
                            Ipp16u* pDst, int dstStep,
                                     IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_16u_C1IR,
                    ( Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_16u_C3IR,
                    ( Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_16u_C4IR,
                    ( Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_32s_C1R,
                    ( const Ipp32s* pSrc, int srcStep,
                            Ipp32s* pDst, int dstStep,
                                     IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_32s_C3R,
                    ( const Ipp32s* pSrc, int srcStep,
                            Ipp32s* pDst, int dstStep,
                                     IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_32s_C4R,
                    ( const Ipp32s* pSrc, int srcStep,
                            Ipp32s* pDst, int dstStep,
                                     IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_32s_C1IR,
                    ( Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_32s_C3IR,
                    ( Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_32s_C4IR,
                    ( Ipp32s* pSrcDst, int srcDstStep, IppiSize roiSize ))

IPPAPI ( IppStatus, ippiTranspose_16s_C1R,
                    ( const Ipp16s* pSrc, int srcStep,
                            Ipp16s* pDst, int dstStep,
                                     IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_16s_C3R,
                    ( const Ipp16s* pSrc, int srcStep,
                            Ipp16s* pDst, int dstStep,
                                     IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_16s_C4R,
                    ( const Ipp16s* pSrc, int srcStep,
                            Ipp16s* pDst, int dstStep,
                                     IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_16s_C1IR,
                    ( Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_16s_C3IR,
                    ( Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_16s_C4IR,
                    ( Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize ))


IPPAPI ( IppStatus, ippiTranspose_32f_C1R,
                    ( const Ipp32f* pSrc, int srcStep,
                            Ipp32f* pDst, int dstStep,
                                     IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_32f_C3R,
                    ( const Ipp32f* pSrc, int srcStep,
                            Ipp32f* pDst, int dstStep,
                                     IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_32f_C4R,
                    ( const Ipp32f* pSrc, int srcStep,
                            Ipp32f* pDst, int dstStep,
                                     IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_32f_C1IR,
                    ( Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_32f_C3IR,
                    ( Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize ))
IPPAPI ( IppStatus, ippiTranspose_32f_C4IR,
                    ( Ipp32f* pSrcDst, int srcDstStep, IppiSize roiSize ))


/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:        ippiDeconvFFTInitAlloc_32f_C*R
//
//  Purpose:    Creates and initializes the Deconvolution context structure
//
//  Returns:     IppStatus
//    ippStsNoErr         No errors
//    ippStsNullPtrErr    pDeconvFFTState == NULL, or pKernel == NULL
//    ippStsSizeErr       kernelSize less or equal to zero
//                        kernelSize great than 2^FFTorder,
//    ippStsBadArgErr     threshold less or equal to zero
//
//  Parameters:
//    pDeconvFFTState     Pointer to the created Deconvolution context structure
//    pKernel             Pointer to the kernel array
//    kernelSize          Size of kernel
//    FFTorder            Order of created FFT structure
//    threshold           Threshold level value (for except dividing to zero)
//
*/

IPPAPI ( IppStatus, ippiDeconvFFTInitAlloc_32f_C1R, (IppiDeconvFFTState_32f_C1R**
        pDeconvFFTState, const Ipp32f* pKernel, int kernelSize, int FFTorder, Ipp32f threshold))

IPPAPI ( IppStatus, ippiDeconvFFTInitAlloc_32f_C3R, (IppiDeconvFFTState_32f_C3R**
        pDeconvFFTState, const Ipp32f* pKernel, int kernelSize, int FFTorder, Ipp32f threshold))

/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:        ippiDeconvFFT_32f_C*R
//
//  Purpose:    Perform deconvolution for source image using FFT
//
//  Returns:     IppStatus
//    ippStsNoErr          No errors
//    ippStsNullPtrErr     pDeconvFFTState == NULL, or pSrc == NULL or pDst == NULL
//    ippStsSizeErr        roi.width or roi.height less or equal to zero
//                         roi.width or roi.height great than (2^FFTorder-kernelSize)
//    ippStsStepErr        srcstep or dststep less than roi.width multiplied by type size
//    ippStsNotEvenStepErr Indicates an error condition if one of step values for floating-point
//                          images cannot be divided by 4.
//
//  Parameters:
//    pSrc                Pointer to the source image
//    srcStep             Step in bytes in the source image
//    pDst                Pointer to the destination image
//    dstStep             Step in bytes in the destination image
//    roi                 Size of the image ROI in pixels.
//    pDeconvFFTState     Pointer to the Deconvolution context structure
//
*/
IPPAPI ( IppStatus, ippiDeconvFFT_32f_C1R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst,
        int dstStep, IppiSize roi, IppiDeconvFFTState_32f_C1R* pDeconvFFTState))

IPPAPI ( IppStatus, ippiDeconvFFT_32f_C3R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst,
        int dstStep, IppiSize roi, IppiDeconvFFTState_32f_C3R* pDeconvFFTState))


/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:        ippiDeconvFFTFree_32f_C*R
//
//  Purpose:    Deallocates memory used by the Deconvolution context structure
//
//  Returns:     IppStatus
//    ippStsNoErr          No errors
//    ippStsNullPtrErr     pDeconvFFTState == NULL
//
//  Parameters:
//    pDeconvFFTState      Pointer to the Deconvolution context structure
//
*/

IPPAPI ( IppStatus, ippiDeconvFFTFree_32f_C1R, (IppiDeconvFFTState_32f_C1R* pDeconvFFTState))

IPPAPI ( IppStatus, ippiDeconvFFTFree_32f_C3R, (IppiDeconvFFTState_32f_C3R* pDeconvFFTState))


/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:        ippiDeconvLRInitAlloc_32f_C*R
//
//  Purpose:    Creates and initializes the Lucy-Richardson Deconvolution context structure
//
//  Returns:     IppStatus
//    ippStsNoErr         No errors
//    ippStsNullPtrErr    pDeconvLR == NULL, or pKernel == NULL
//    ippStsSizeErr       kernelSize less or equal to zero
//                        kernelSize great than maxroi.width or maxroi.height,
//                        maxroi.height or maxroi.width less or equal to zero
//    ippStsBadArgErr     threshold less or equal to zero
//
//  Parameters:
//    pDeconvLR           Pointer to the created Lucy-Richardson Deconvolution context structure
//    pKernel             Pointer to the kernel array
//    kernelSize          Size of kernel
//    maxroi              Maximum size of the image ROI in pixels.
//    threshold           Threshold level value (for except dividing to zero)
//
*/

IPPAPI ( IppStatus, ippiDeconvLRInitAlloc_32f_C1R, (IppiDeconvLR_32f_C1R** pDeconvLR,
        const Ipp32f* pKernel, int kernelSize, IppiSize maxroi, Ipp32f threshold))

IPPAPI ( IppStatus, ippiDeconvLRInitAlloc_32f_C3R, (IppiDeconvLR_32f_C3R** pDeconvLR,
        const Ipp32f* pKernel, int kernelSize, IppiSize maxroi, Ipp32f threshold))

/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:        ippiDeconvLR_32f_C*R
//
//  Purpose:    Perform deconvolution for source image using Lucy-Richardson algorithm
//
//  Returns:     IppStatus
//    ippStsNoErr          No errors
//    ippStsNullPtrErr     pDeconvLR == NULL, or pSrc == NULL or pDst == NULL
//    ippStsSizeErr        roi.width or roi.height less or equal to zero
//                         roi.width  great than (maxroi.width-kernelSize)
//                         roi.height great than (maxroi.height-kernelSize)
//    ippStsStepErr        srcstep or dststep less than roi.width multiplied by type size
//    ippStsNotEvenStepErr Indicates an error condition if one of step values for floating-point
//                          images cannot be divided by 4
//    ippStsBadArgErr      number of iterations less or equal to zero
//
//  Parameters:
//    pSrc                Pointer to the source image
//    srcStep             Step in bytes in the source image
//    pDst                Pointer to the destination image
//    dstStep             Step in bytes in the destination image
//    roi                 Size of the image ROI in pixels
//    numiter             Number of algorithm iteration
//    pDeconvLR           Pointer to the Lucy-Richardson Deconvolution context structure
//
*/

IPPAPI ( IppStatus, ippiDeconvLR_32f_C1R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
        IppiSize roi, int numiter, IppiDeconvLR_32f_C1R* pDeconvLR))

IPPAPI ( IppStatus, ippiDeconvLR_32f_C3R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
        IppiSize roi, int numiter, IppiDeconvLR_32f_C3R* pDeconvLR))

/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:        ippiDeconvLRFree_32f_C*R
//
//  Purpose:    Deallocates memory used by the Lucy-Richardson Deconvolution context structure
//
//  Returns:     IppStatus
//    ippStsNoErr          No errors
//    ippStsNullPtrErr     pDeconvLR == NULL
//
//  Parameters:
//    pDeconvLR            Pointer to the Lucy-Richardson Deconvolution context structure
//
*/
IPPAPI ( IppStatus, ippiDeconvLRFree_32f_C1R, (IppiDeconvLR_32f_C1R* pDeconvLR))

IPPAPI ( IppStatus, ippiDeconvLRFree_32f_C3R, (IppiDeconvLR_32f_C3R* pDeconvLR))

/* /////////////////////////////////////////////////////////////////////////////
//
//  Names:        ippiCompColorKey_8u_C1R
//                ippiCompColorKey_8u_C3R
//                ippiCompColorKey_8u_C4R
//                ippiCompColorKey_16s_C1R
//                ippiCompColorKey_16s_C3R
//                ippiCompColorKey_16s_C4R
//                ippiCompColorKey_16u_C1R
//                ippiCompColorKey_16u_C3R
//                ippiCompColorKey_16u_C4R
//
//  Purpose:    Perform alpha blending with transparent background.
//
//  Returns:     IppStatus
//     ippStsNoErr            No errors
//     ippStsNullPtrErr       One of the pointers is NULL
//     ippStsSizeErr          The roiSize has a field with negative or zero value
//     ippStsStepErr          One of steps is less than or equal to zero
//     ippStsAlphaTypeErr     Unsupported type of composition (for ippiAlphaCompColorKey)
//
//  Parameters:
//    pSrc1, pSrc2           Pointers to the source images
//    src1Step, src2Step     Steps through the source images
//    pDst                   Pointer to the destination image
//    dstStep                Step through the destination image
//    roiSize                Size of the image ROI
//    colorKey               Color value (array of values for multi-channel data)
//    alphaType              The type of composition to perform (for ippiAlphaCompColorKey)
//
*/
IPPAPI (IppStatus, ippiCompColorKey_8u_C1R, (const Ipp8u* pSrc1, int src1Step,
                                             const Ipp8u* pSrc2, int src2Step,
                                                   Ipp8u* pDst,  int dstStep,
                                             IppiSize roiSize, Ipp8u colorKey))

IPPAPI (IppStatus, ippiCompColorKey_8u_C3R, (const Ipp8u* pSrc1, int src1Step,
                                             const Ipp8u* pSrc2, int src2Step,
                                                   Ipp8u* pDst,  int dstStep,
                                             IppiSize roiSize, Ipp8u colorKey[3]))

IPPAPI (IppStatus, ippiCompColorKey_8u_C4R, (const Ipp8u* pSrc1, int src1Step,
                                             const Ipp8u* pSrc2, int src2Step,
                                                   Ipp8u* pDst,  int dstStep,
                                             IppiSize roiSize, Ipp8u colorKey[4]))

IPPAPI (IppStatus, ippiCompColorKey_16u_C1R, (const Ipp16u* pSrc1, int src1Step,
                                              const Ipp16u* pSrc2, int src2Step,
                                                    Ipp16u* pDst,  int dstStep,
                                              IppiSize roiSize, Ipp16u colorKey))

IPPAPI (IppStatus, ippiCompColorKey_16u_C3R, (const Ipp16u* pSrc1, int src1Step,
                                              const Ipp16u* pSrc2, int src2Step,
                                                     Ipp16u* pDst,  int dstStep,
                                              IppiSize roiSize,   Ipp16u colorKey[3]))

IPPAPI (IppStatus, ippiCompColorKey_16u_C4R, (const Ipp16u* pSrc1, int src1Step,
                                              const Ipp16u* pSrc2, int src2Step,
                                                  Ipp16u* pDst,  int dstStep,
                                              IppiSize roiSize, Ipp16u colorKey[4]))

IPPAPI (IppStatus, ippiCompColorKey_16s_C1R, (const Ipp16s* pSrc1, int src1Step,
                                              const Ipp16s* pSrc2, int src2Step,
                                                    Ipp16s* pDst,  int dstStep,
                                              IppiSize roiSize, Ipp16s colorKey))

IPPAPI (IppStatus, ippiCompColorKey_16s_C3R, (const Ipp16s* pSrc1, int src1Step,
                                              const Ipp16s* pSrc2, int src2Step,
                                                    Ipp16s* pDst,  int dstStep,
                                              IppiSize roiSize, Ipp16s colorKey[3]))

IPPAPI (IppStatus, ippiCompColorKey_16s_C4R, (const Ipp16s* pSrc1, int src1Step,
                                              const Ipp16s* pSrc2, int src2Step,
                                                    Ipp16s* pDst,  int dstStep,
                                              IppiSize roiSize, Ipp16s colorkey[4]))
IPPAPI (IppStatus, ippiAlphaCompColorKey_8u_AC4R, ( const Ipp8u* pSrc1, int src1Step,Ipp8u alpha1,
                                              const Ipp8u* pSrc2, int src2Step, Ipp8u alpha2,
                                              Ipp8u* pDst,  int dstStep,
                                              IppiSize roiSize,   Ipp8u colorKey[4],IppiAlphaType alphaType ))


/* ///////////////////////////////////////////////////////////////////////////
//                     Median filter function
// ///////////////////////////////////////////////////////////////////////////
// Name:
//   ippiMedian_8u_P3C1R
//
// Purpose:
//   Median of three images.
//
//   For each pixel (x, y) in the ROI:
//   pDst[x + y*dstStep] = MEDIAN(pSrc[0][x + y*srcStep],
//                                pSrc[1][x + y*srcStep],
//                                pSrc[2][x + y*srcStep]);
//
// Parameters:
//   pSrc       Pointer to three source images.
//   srcStep    Step in bytes through source images.
//   pDst       Pointer to the destination image.
//   dstStep    Step in bytes through the destination image buffer.
//   size       Size of the ROI in pixels.
//
// Returns:
//   ippStsNoErr        Indicates no error. Any other value indicates an error or a warning.
//   ippStsNullPtrErr   Indicates an error if one of the specified pointers is NULL.
//   ippStsSizeErr      Indicates an error condition if size has a field with zero or negative value.
//
*/
IPPAPI(IppStatus, ippiMedian_8u_P3C1R, (const Ipp8u *pSrc[3],
                                        int srcStep,
                                        Ipp8u *pDst,
                                        int dstStep,
                                        IppiSize size))


/* /////////////////////////////////////////////////////////////////////////////
//                     De-interlacing filter function
// /////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////
//
//  Name:               ippiDeinterlaceFilterCAVT_8u_C1R
//  Purpose:            Performs de-interlacing of two-field image
//                      using content adaptive vertical temporal (CAVT) filtering
//  Parameters:
//    pSrc              pointer to the source image (frame)
//    srcStep           step of the source pointer in bytes
//    pDst              pointer to the destination image (frame)
//    dstStep           step of the destination pointer in bytes
//    threshold         threshold level value
//    roiSize           size of the source and destination ROI
//  Returns:
//    ippStsNoErr       no errors
//    ippStsNullPtrErr  pSrc == NULL or pDst == NULL
//    ippStsSizeErr     width of roi is less or equal zero or
//                      height of roi is less 8 or odd
*/

IPPAPI(IppStatus, ippiDeinterlaceFilterCAVT_8u_C1R,
    (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep, Ipp16u threshold, IppiSize roiSize))




/* /////////////////////////////////////////////////////////////////////////////
//                     Bilateral filter function
// /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFilterBilateralGetBufSize_8u_C1R
//  Purpose:    to define buffer size for bilateral filter
//  Parameters:
//    filter        type of bilateral filter
//    maxDstRoiSize maximum roi size of destination image what will be applied
//                  for processing
//    maxKernelSize maximum size of kernel what will be applied for processing
//    pBufferSize   pointer to buffer size
//  Return:
//    ippStsNoErr               OK
//    ippStsNullPtrErr          pointer to buffer size is NULL
//    ippStsSizeErr             size of maxDstRoiSize is less or equal 0
//    ippStsMaskSizeErr         size of maxKernelSize is less or equal 0
//    ippStsNotSupportedModeErr bilateral filter type is not supported
*/
IPPAPI(IppStatus,ippiFilterBilateralGetBufSize_8u_C1R,
       ( IppiFilterBilateralType filter, IppiSize maxDstRoiSize,
       IppiSize maxKernelSize, int *pBufferSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFilterBilateralInit_8u_C1R
//  Purpose:    initialization of Spec for bilateral filter
//  Parameters:
//    filter            type of bilateral filter
//    valSquareSigma    square of Sigma for factor function for pixel value
//    posSquareSigma    square of Sigma for factor function for pixel position
//    stepInKernel      processing step in kernel
//    pSpec             pointer to Spec
//  Return:
//    ippStsNoErr               OK
//    ippStsNullPtrErr          pointer to Spec is NULL
//    ippStsMaskSizeErr         size of maxKernelSize is less or equal 0
//    ippStsBadArgErr           valSquareSigma, posSquareSigma or stepInKernel
//                              is less or equal 0
//    ippStsNotSupportedModeErr bilateral filter type is not supported
*/
IPPAPI(IppStatus,ippiFilterBilateralInit_8u_C1R,( IppiFilterBilateralType filter,
       IppiSize maxKernelSize, Ipp32f valSquareSigma, Ipp32f posSquareSigma,
       int stepInKernel, IppiFilterBilateralSpec *pSpec ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFilterBilateral_8u_C1R
//  Purpose:    bilateral filter
//  Parameters:
//    pSrc         Pointer to the source image
//    srcStep      Step through the source image
//    pDst         Pointer to the destination image
//    dstStep      Step through the destination image
//    dstRoiSize   Size of the destination ROI
//    kernelSize   Size of the filter kernel
//    pSpec        Pointer to filter spec
//  Return:
//    ippStsNoErr           OK
//    ippStsNullPtrErr      pointer to Src, Dst or Specc is NULL
//    ippStsSizeErr         size of dstRoiSize is less or equal 0
//    ippStsMaskSizeErr     size of kernelSize is less or equal 0
//    ippStsStepErr         step of src or dst is less or equal 0
//    ippStsContextMatchErr filter Spec is not match
*/
IPPAPI(IppStatus,ippiFilterBilateral_8u_C1R,( const Ipp8u *pSrc, int srcStep,
       Ipp8u *pDst, int dstStep, IppiSize dstRoiSize, IppiSize kernelSize,
       IppiFilterBilateralSpec *pSpec))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:      ippiFilterGetBufSize_64f_C1R
//  Purpose:   Get size of temporal buffer
//  Parameters:
//      kernelSize      Size of the rectangular kernel in pixels.
//      roiWidth        Width of ROI
//      pSize           Pointer to the size of work buffer
//  Returns:
//   ippStsNoErr        Ok
//   ippStsNullPtrErr   pSize is NULL
//   ippStsSizeErr      Some size of kernelSize or roiWidth less or equal zero
//  Remark:             Function may return zero size of buffer.
*/
IPPAPI( IppStatus, ippiFilterGetBufSize_64f_C1R, (IppiSize  kernelSize, int roiWidth, int* pSize))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFilter64f_8u_C1R
//  Purpose:    Filters an image using a general float rectangular kernel
//  Parameters:
//      pSrc            Pointer to the source buffer
//      srcStep         Step in bytes through the source image buffer
//      pDst            Pointer to the destination buffer
//      dstStep         Step in bytes through the destination image buffer
//      dstRoiSize      Size of the source and destination ROI in pixels
//      pKernel         Pointer to the kernel values ( 64f kernel )
//      kernelSize      Size of the rectangular kernel in pixels.
//      anchor          Anchor cell specifying the rectangular kernel alignment
//                      with respect to the position of the input pixel
//      pBuffer         Pointer to work buffer
//  Returns:
//   ippStsNoErr        Ok
//   ippStsNullPtrErr   Some of pointers to pSrc, pDst or pKernel are NULL or
//                      pBuffer is null but GetBufSize returned non zero size
//   ippStsSizeErr      Some size of dstRoiSize or kernalSize less or equal zero
//   ippStsStepErr      srcStep is less than (roiWidth + kernelWidth - 1) * sizeof(Ipp64f) or
//                      dstStep is less than  roiWidth * sizeof(Ipp64f)
*/

IPPAPI( IppStatus, ippiFilter_64f_C1R, ( const Ipp64f* pSrc, int srcStep,
        Ipp64f* pDst, int dstStep, IppiSize dstRoiSize, const Ipp64f* pKernel,
        IppiSize kernelSize, IppiPoint anchor, Ipp8u* pBuffer ))

/* ///////////////////////////////////////////////////////////////////////////
//             General Linear Filters with rounding
// ////////////////////////////////////////////////////////////////////////////
//   Names:     ippiFilterRoundGetBufSize16s_8u_C1R
//              ippiFilterRoundGetBufSize16s_8u_C3R
//              ippiFilterRoundGetBufSize16s_8u_C4R
//              ippiFilterRoundGetBufSize16s_8u_AC4R
//              ippiFilterRoundGetBufSize32s_16u_C1R
//              ippiFilterRoundGetBufSize32s_16u_C3R
//              ippiFilterRoundGetBufSize32s_16u_C4R
//              ippiFilterRoundGetBufSize32s_16u_AC4R
//              ippiFilterRoundGetBufSize32s_16s_C1R
//              ippiFilterRoundGetBufSize32s_16s_C3R
//              ippiFilterRoundGetBufSize32s_16s_C4R
//              ippiFilterRoundGetBufSize32s_16s_AC4R
//              ippiFilterRoundGetBufSize32f_8u_C3R
//              ippiFilterRoundGetBufSize32f_8u_C3R
//              ippiFilterRoundGetBufSize32f_8u_C4R
//              ippiFilterRoundGetBufSize32f_8u_AC4R
//              ippiFilterRoundGetBufSize32f_16u_C1R
//              ippiFilterRoundGetBufSize32f_16u_C3R
//              ippiFilterRoundGetBufSize32f_16u_C4R
//              ippiFilterRoundGetBufSize32f_16u_AC4R
//              ippiFilterRoundGetBufSize32f_16s_C1R
//              ippiFilterRoundGetBufSize32f_16s_C3R
//              ippiFilterRoundGetBufSize32f_16s_C4R
//              ippiFilterRoundGetBufSize32f_16s_AC4R

//  Purpose:    Filters an image using a general integer rectangular kernel
//              with rounding(ippRndZero, ippRndNear, ippRndFinancial)
//  Purpose:    Get size of temporal buffer
//      kernelSize      Size of the rectangular kernel in pixels.
//      roiWidth        Width of ROI
//      pSize           Pointer to the size of work buffer
//  Returns:
//   ippStsNoErr        Ok
//   ippStsNullPtrErr   pSize is NULL
//   ippStsSizeErr      Some size of kernelSize or roiWidth less or equal zero
//  Remark:             Function may return zero size of buffer.
*/
IPPAPI( IppStatus, ippiFilterRoundGetBufSize16s_8u_C1R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize16s_8u_C3R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize16s_8u_C4R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize16s_8u_AC4R,(IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32s_16u_C1R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32s_16u_C3R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32s_16u_C4R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32s_16u_AC4R,(IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32s_16s_C1R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32s_16s_C3R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32s_16s_C4R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32s_16s_AC4R,(IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32f_8u_C1R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32f_8u_C3R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32f_8u_C4R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32f_8u_AC4R,(IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32f_16u_C1R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32f_16u_C3R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32f_16u_C4R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32f_16u_AC4R,(IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32f_16s_C1R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32f_16s_C3R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32f_16s_C4R, (IppiSize  kernelSize, int roiWidth, int* pSize))
IPPAPI( IppStatus, ippiFilterRoundGetBufSize32f_16s_AC4R,(IppiSize  kernelSize, int roiWidth, int* pSize))
/*
//
//  Purpose:    Filters an image using a general integer rectangular kernel
//   Names:     ippiFilter_Round16s_8u_C1R
//              ippiFilter_Round16s_8u_C3R
//              ippiFilter_Round16s_8u_C4R
//              ippiFilter_Round16s_8u_AC4R
//              ippiFilter_Round32s_16u_C1R
//              ippiFilter_Round32s_16u_C3R
//              ippiFilter_Round32s_16u_C4R
//              ippiFilter_Round32s_16u_AC4R
//              ippiFilter_Round32s_16s_C1R
//              ippiFilter_Round32s_16s_C3R
//              ippiFilter_Round32s_16s_C4R
//              ippiFilter_Round32s_16s_AC4R
//  Purpose:    Filters an image using a general integer rectangular kernel
//              with rounding(ippRndZero, ippRndNear, ippRndFinancial)
//  Parameters:
//      pSrc           Pointer to the source buffer
//      srcStep        Step in bytes through the source image buffer
//      pDst           Pointer to the destination buffer
//      dstStep        Step in bytes through the destination image buffer
//      dstRoiSize     Size of the source and destination ROI in pixels
//      pKernel        Pointer to the kernel values ( 16s(32s) kernel )
//      kernelSize     Size of the rectangular kernel in pixels.
//      anchor         Anchor cell specifying the rectangular kernel alignment
//                     with respect to the position of the input pixel
//      divisor        The integer value by which the computed result is divided.
//      roundMode      Rounding mode (ippRndZero, ippRndNear or ippRndFinancial)
//      pBuffer        Pointer to the temporary data

//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  Some of pointers to pSrc, pDst or pKernel are NULL or
//                     pBuffer is null but GetBufSize returned non zero size
//   ippStsSizeErr     dstRoiSize or kernelSize has a field with zero or negative value
//   ippStsDivisorErr  divisor value is zero, function execution is interrupted
//   ippStsStepErr      srcStep is less than (roiWidth + kernelWidth - 1) * sizeof(Ipp64f) or
//                      dstStep is less than  roiWidth * sizeof(*pSrc)
//   ippStsRoundModeNotSupportedErr Unsupported round mode
*/
IPPAPI( IppStatus, ippiFilter_Round16s_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp16s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round16s_8u_C3R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp16s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round16s_8u_C4R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp16s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round16s_8u_AC4R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp16s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32s_16u_C1R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32s_16u_C3R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32s_16u_C4R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32s_16u_AC4R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32s_16s_C1R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32s_16s_C3R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32s_16s_C4R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32s_16s_AC4R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32s* pKernel,
        IppiSize kernelSize, IppiPoint anchor, int divisor, IppRoundMode roundMode, Ipp8u* pBuffer ))
/*
//
//  Purpose:    Filters an image using a general integer rectangular kernel
//   Names:     ippiFilter_Round32f_8u_C1R
//              ippiFilter_Round32f_8u_C3R
//              ippiFilter_Round32f_8u_C4R
//              ippiFilter_Round32f_8u_AC4R
//              ippiFilter_Round32f_16u_C1R
//              ippiFilter_Round32f_16u_C3R
//              ippiFilter_Round32f_16u_C4R
//              ippiFilter_Round32f_16u_AC4R
//              ippiFilter_Round32f_16s_C1R
//              ippiFilter_Round32f_16s_C3R
//              ippiFilter_Round32f_16s_C4R
//              ippiFilter_Round32f_16s_AC4R
//  Purpose:    Filters an image using a general integer rectangular kernel
//              with rounding(ippRndZero, ippRndNear, ippRndFinancial)
//  Parameters:
//      pSrc           Pointer to the source buffer
//      srcStep        Step in bytes through the source image buffer
//      pDst           Pointer to the destination buffer
//      dstStep        Step in bytes through the destination image buffer
//      dstRoiSize     Size of the source and destination ROI in pixels
//      pKernel        Pointer to the kernel values ( 32f kernel )
//      kernelSize     Size of the rectangular kernel in pixels.
//      anchor         Anchor cell specifying the rectangular kernel alignment
//                     with respect to the position of the input pixel
//      roundMode      Rounding mode (ippRndZero, ippRndNear or ippRndFinancial)
//      pBuffer        Pointer to the temporary data

//  Returns:
//   ippStsNoErr       OK
//   ippStsNullPtrErr  Some of pointers to pSrc, pDst or pKernel are NULL or
//                     pBuffer is null but GetBufSize returned non zero size
//   ippStsSizeErr     dstRoiSize or kernelSize has a field with zero or negative value
//   ippStsStepErr      srcStep is less than (roiWidth + kernelWidth - 1) * sizeof(Ipp64f) or
//                      dstStep is less than  roiWidth * sizeof(*pSrc)
//   ippStsRoundModeNotSupportedErr Unsupported round mode
*/
IPPAPI( IppStatus, ippiFilter_Round32f_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32f_8u_C3R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32f_8u_C4R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32f_8u_AC4R, ( const Ipp8u* pSrc, int srcStep,
        Ipp8u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32f_16u_C1R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor,  IppRoundMode roundMode, Ipp8u* pBuffer))
IPPAPI( IppStatus, ippiFilter_Round32f_16u_C3R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor,  IppRoundMode roundMode, Ipp8u* pBuffer))
IPPAPI( IppStatus, ippiFilter_Round32f_16u_C4R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor,  IppRoundMode roundMode, Ipp8u* pBuffer))
IPPAPI( IppStatus, ippiFilter_Round32f_16u_AC4R, ( const Ipp16u* pSrc, int srcStep,
        Ipp16u* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor,  IppRoundMode roundMode, Ipp8u* pBuffer))

IPPAPI( IppStatus, ippiFilter_Round32f_16s_C1R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32f_16s_C3R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32f_16s_C4R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor, IppRoundMode roundMode, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippiFilter_Round32f_16s_AC4R, ( const Ipp16s* pSrc, int srcStep,
        Ipp16s* pDst, int dstStep, IppiSize dstRoiSize, const Ipp32f* pKernel,
        IppiSize kernelSize, IppiPoint anchor, IppRoundMode roundMode, Ipp8u* pBuffer ))

/*
//  Purpose:    Divides pixel values of an image by pixel values of
//              another image with three rounding modes (ippRndZero,ippRndNear,ippRndFinancial)
//              and places the scaled results in a destination
//              image.
//  Name:       ippiDiv_Round_16s_C1RSfs, ippiDiv_Round_8u_C1RSfs, ippiDiv_Round_16u_C1RSfs,
//              ippiDiv_Round_16s_C3RSfs, ippiDiv_Round_8u_C3RSfs, ippiDiv_Round_16u_C3RSfs,
//              ippiDiv_Round_16s_C4RSfs, ippiDiv_Round_8u_C4RSfs, ippiDiv_Round_16u_C4RSfs,
//              ippiDiv_Round_16s_AC4RSfs, ippiDiv_Round_8u_AC4RSfs, ippiDiv_Round_16u_AC4RSfs,
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field with zero or negative value
//    ippStsStepErr            At least one step value is less than or equal to zero
//    ippStsDivByZero          A warning that a divisor value is zero, the function
//                             execution is continued.
//                    If a dividend is equal to zero, then the result is zero;
//                    if it is greater than zero, then the result is IPP_MAX_16S, or IPP_MAX_8U, or IPP_MAX_16U
//                    if it is less than zero (for 16s), then the result is IPP_MIN_16S
//   ippStsRoundModeNotSupportedErr Unsupported round mode
//
//
//  Parameters:
//    pSrc1                    Pointer to the divisor source image
//    src1Step                 Step through the divisor source image
//    pSrc2                    Pointer to the dividend source image
//    src2Step                 Step through the dividend source image
//    pDst                     Pointer to the destination image
//    dstStep                  Step through the destination image
//    roiSize                  Size of the ROI
//    rndMode           Rounding mode (ippRndZero, ippRndNear or ippRndFinancial)
//    scaleFactor              Scale factor
*/

IPPAPI(IppStatus, ippiDiv_Round_16s_C1RSfs, (const Ipp16s* pSrc1, int src1Step,
       const Ipp16s* pSrc2, int src2Step,    Ipp16s* pDst, int dstStep, IppiSize roiSize,
       IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16s_C3RSfs, (const Ipp16s* pSrc1, int src1Step,
       const Ipp16s* pSrc2, int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
       IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16s_C4RSfs, (const Ipp16s* pSrc1, int src1Step,
       const Ipp16s* pSrc2, int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
       IppRoundMode rndMode, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16s_AC4RSfs, (const Ipp16s* pSrc1, int src1Step,
       const Ipp16s* pSrc2, int src2Step, Ipp16s* pDst, int dstStep, IppiSize roiSize,
       IppRoundMode rndMode, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_8u_C1RSfs,  (const Ipp8u* pSrc1, int src1Step,
       const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
       IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_8u_C3RSfs,  (const Ipp8u* pSrc1, int src1Step,
       const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
       IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_8u_C4RSfs, (const Ipp8u* pSrc1, int src1Step,
       const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
       IppRoundMode rndMode, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_8u_AC4RSfs, (const Ipp8u* pSrc1, int src1Step,
       const Ipp8u* pSrc2, int src2Step, Ipp8u* pDst, int dstStep, IppiSize roiSize,
       IppRoundMode rndMode, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16u_C1RSfs, (const Ipp16u* pSrc1, int src1Step,
       const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
       IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16u_C3RSfs, (const Ipp16u* pSrc1, int src1Step,
       const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
       IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16u_C4RSfs, (const Ipp16u* pSrc1, int src1Step,
       const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
        IppRoundMode rndMode, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16u_AC4RSfs, (const Ipp16u* pSrc1, int src1Step,
       const Ipp16u* pSrc2, int src2Step, Ipp16u* pDst, int dstStep, IppiSize roiSize,
        IppRoundMode rndMode, int ScaleFactor))
/*
//  Purpose:    Divides pixel values of an image by pixel values of
//              another image with three rounding modes (ippRndZero,ippRndNear,ippRndFinancial)
//              and places the scaled results in a destination
//              image.
//  Name:       ippiDiv_Round_16s_C1IRSfs, ippiDiv_Round_8u_C1IRSfs, ippiDiv_Round_16u_C1IRSfs,
//              ippiDiv_Round_16s_C3IRSfs, ippiDiv_Round_8u_C3IRSfs, ippiDiv_Round_16u_C3IRSfs,
//              ippiDiv_Round_16s_C4IRSfs, ippiDiv_Round_8u_C4IRSfs, ippiDiv_Round_16u_C4IRSfs,
//              ippiDiv_Round_16s_AC4IRSfs, ippiDiv_Round_8u_AC4IRSfs, ippiDiv_Round_16u_AC4IRSfs,
//  Parameters:
//    pSrc                     Pointer to the divisor source image
//    srcStep                  Step through the divisor source image
//    pSrcDst                  Pointer to the dividend source/destination image
//    srcDstStep               Step through the dividend source/destination image
//    roiSize                  Size of the ROI
//    rndMode           Rounding mode (ippRndZero, ippRndNear or ippRndFinancial)
//    scaleFactor              Scale factor
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field with zero or negative value
//    ippStsStepErr            At least one step value is less than or equal to zero
//    ippStsDivByZero          A warning that a divisor value is zero, the function
//                             execution is continued.
//                    If a dividend is equal to zero, then the result is zero;
//                    if it is greater than zero, then the result is IPP_MAX_16S, or IPP_MAX_8U, or IPP_MAX_16U
//                    if it is less than zero (for 16s), then the result is IPP_MIN_16S
//   ippStsRoundModeNotSupportedErr Unsupported round mode
*/
IPPAPI(IppStatus, ippiDiv_Round_16s_C1IRSfs, (const Ipp16s* pSrc, int srcStep,
         Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize,
       IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16s_C3IRSfs, (const Ipp16s* pSrc, int srcStep,
         Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize,
       IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16s_C4IRSfs, (const Ipp16s* pSrc, int srcStep,
         Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize,
       IppRoundMode rndMode, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16s_AC4IRSfs, (const Ipp16s* pSrc, int srcStep,
         Ipp16s* pSrcDst, int srcDstStep, IppiSize roiSize,
       IppRoundMode rndMode, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_8u_C1IRSfs,  (const Ipp8u* pSrc, int srcStep,
         Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize,
       IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_8u_C3IRSfs,  (const Ipp8u* pSrc, int srcStep,
         Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize,
       IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_8u_C4IRSfs, (const Ipp8u* pSrc, int srcStep,
         Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize,
       IppRoundMode rndMode, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_8u_AC4IRSfs, (const Ipp8u* pSrc, int srcStep,
         Ipp8u* pSrcDst, int srcDstStep, IppiSize roiSize,
       IppRoundMode rndMode, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16u_C1IRSfs, (const Ipp16u* pSrc, int srcStep,
         Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize,
       IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16u_C3IRSfs, (const Ipp16u* pSrc, int srcStep,
         Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize,
       IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16u_C4IRSfs, (const Ipp16u* pSrc, int srcStep,
         Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize,
        IppRoundMode rndMode, int ScaleFactor))
IPPAPI(IppStatus, ippiDiv_Round_16u_AC4IRSfs, (const Ipp16u* pSrc, int srcStep,
         Ipp16u* pSrcDst, int srcDstStep, IppiSize roiSize,
        IppRoundMode rndMode, int ScaleFactor))

/*
//  Purpose:    Evaluates per-pixel SSIM equation on filtered images.
//              Intended to be uses as part of a wSSIM evaluation pipeline according to the following reference: 
//              Z. Wang, A. C. Bovik, H. R. Sheikh and E. P. Simoncelli, "Image quality assessment: From error
//              visibility to structural similarity," IEEE TIP, vol. 13, no. 4, pp. 600-612, Apr. 2004.
//              Link: h__p://www.ece.uwaterloo.ca/~z70wang/publications/ssim.html
//
//    pDst[i] = (2*F(I1)[i]*F(I2)[i]+C1)/(F(I1)[i]^2+F(I2)[i]^2+C1)*
//              *(2*F(I1*I2)-2*F(I1)[i]*F(I2)[i]+C2)/(F(I1^2)+F(I1^2)-F(I1)[i]^2-F(I2)[i]^2+C2)
//
//  Parameters:
//    pSrc1                    Filtered first source image F(I1)
//    src1Step                 Step through the first source image
//    pSrc2                    Filtered second source image F(I2)
//    src2Step                 Step through the second source image
//    pSrc3                    Filtered squared first source image F(I1^2)
//    src3Step                 Step through the squared first source image
//    pSrc4                    Filtered squared second source image F(I2^2)
//    src4Step                 Step through the squared second source image
//    pSrc5                    Filtered product of the first and second source images F(I1*I2)
//    src5Step                 Step through the product of the first and second source images
//    pDst                     Pointer to the unweighted per-pixel SSIM indexes array
//    dstStep                  Step through the unweighted per-pixel SSIM indexes array
//    roiSize                  Size of the ROI
//    C1                       First predefined algorithm constant
//    C2                       Second predefined algorithm constant
//  Returns:
//    ippStsNoErr              OK
//    ippStsNullPtrErr         One of the pointers is NULL
//    ippStsSizeErr            roiSize has a field with zero or negative value
//    ippStsStepErr            At least one step value is less than or equal to zero
//    ippStsBadArgErr          Incorrect hint value
*/

IPPAPI( IppStatus, ippiSSIM_32f_C1R,( const Ipp32f* pSrc1, int src1Step, const Ipp32f* pSrc2, int src2Step,
       const Ipp32f* pSrc3, int src3Step, const Ipp32f* pSrc4, int src4Step, const Ipp32f* pSrc5, int src5Step,
       Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f C1, Ipp32f C2, IppHintAlgorithm hint))


#ifdef __cplusplus
}
#endif

#if defined (_IPP_STDCALL_CDECL)
  #undef  _IPP_STDCALL_CDECL
  #define __stdcall __cdecl
#endif

#endif /* __IPPI_H__ */
/* ////////////////////////// End of file "ippi.h" ////////////////////////// */
