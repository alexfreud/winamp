/* ////////////////////////////////// "ipps.h" /////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//         Copyright(c) 1999-2009 Intel Corporation. All Rights Reserved.
//
//                 Intel(R) Integrated Performance Primitives
//                      Signal Processing (ippSP)
//
*/

#if !defined( __IPPS_H__ ) || defined( _OWN_BLDPCS )
#define __IPPS_H__

#ifndef __IPPDEFS_H__
#include "ippdefs.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined (_WIN32_WCE) && defined (_M_IX86) && defined (__stdcall)
  #define _IPP_STDCALL_CDECL
  #undef __stdcall
#endif


#if !defined( _OWN_BLDPCS )
typedef struct {
    int left;
    int right;
} IppsROI;
#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsGetLibVersion
//  Purpose:    get the library version
//  Parameters:
//  Returns:    pointer to structure describing version of the ipps library
//
//  Notes:      don't free the pointer
*/
IPPAPI( const IppLibraryVersion*, ippsGetLibVersion, (void) )

/* /////////////////////////////////////////////////////////////////////////////
//                   Functions to allocate and free memory
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsMalloc
//  Purpose:    32-byte aligned memory allocation
//  Parameter:
//    len       number of elements (according to their type)
//  Returns:    pointer to allocated memory
//
//  Notes:      the memory allocated by ippsMalloc has to be free by ippsFree
//              function only.
*/

IPPAPI( Ipp8u*,   ippsMalloc_8u,  (int len) )
IPPAPI( Ipp16u*,  ippsMalloc_16u, (int len) )
IPPAPI( Ipp32u*,  ippsMalloc_32u, (int len) )
IPPAPI( Ipp8s*,   ippsMalloc_8s,  (int len) )
IPPAPI( Ipp16s*,  ippsMalloc_16s, (int len) )
IPPAPI( Ipp32s*,  ippsMalloc_32s, (int len) )
IPPAPI( Ipp64s*,  ippsMalloc_64s, (int len) )

IPPAPI( Ipp32f*,  ippsMalloc_32f, (int len) )
IPPAPI( Ipp64f*,  ippsMalloc_64f, (int len) )

IPPAPI( Ipp8sc*,  ippsMalloc_8sc,  (int len) )
IPPAPI( Ipp16sc*, ippsMalloc_16sc, (int len) )
IPPAPI( Ipp32sc*, ippsMalloc_32sc, (int len) )
IPPAPI( Ipp64sc*, ippsMalloc_64sc, (int len) )
IPPAPI( Ipp32fc*, ippsMalloc_32fc, (int len) )
IPPAPI( Ipp64fc*, ippsMalloc_64fc, (int len) )


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFree
//  Purpose:    free memory allocated by the ippsMalloc functions
//  Parameter:
//    ptr       pointer to the memory allocated by the ippsMalloc functions
//
//  Notes:      use the function to free memory allocated by ippsMalloc_*
*/
IPPAPI( void, ippsFree, (void* ptr) )



/* /////////////////////////////////////////////////////////////////////////////
//                   Vector Initialization functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsCopy
//  Purpose:    copy data from source to destination vector
//  Parameters:
//    pSrc        pointer to the input vector
//    pDst        pointer to the output vector
//    len         length of the vectors, number of items
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           length of the vectors is less or equal zero
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippsCopy_8u,( const Ipp8u* pSrc, Ipp8u* pDst, int len ))
IPPAPI(IppStatus, ippsCopy_16s,( const Ipp16s* pSrc, Ipp16s* pDst, int len ))
IPPAPI(IppStatus, ippsCopy_16sc,( const Ipp16sc* pSrc, Ipp16sc* pDst, int len ))
IPPAPI(IppStatus, ippsCopy_32f,( const Ipp32f* pSrc, Ipp32f* pDst, int len ))
IPPAPI(IppStatus, ippsCopy_32fc,( const Ipp32fc* pSrc, Ipp32fc* pDst, int len ))
IPPAPI(IppStatus, ippsCopy_64f,( const Ipp64f* pSrc, Ipp64f* pDst, int len ))
IPPAPI(IppStatus, ippsCopy_64fc,( const Ipp64fc* pSrc, Ipp64fc* pDst, int len ))
IPPAPI(IppStatus, ippsCopy_32s,( const Ipp32s* pSrc, Ipp32s* pDst, int len ))
IPPAPI(IppStatus, ippsCopy_32sc,( const Ipp32sc* pSrc, Ipp32sc* pDst, int len ))
IPPAPI(IppStatus, ippsCopy_64s,( const Ipp64s* pSrc, Ipp64s* pDst, int len ))
IPPAPI(IppStatus, ippsCopy_64sc,( const Ipp64sc* pSrc, Ipp64sc* pDst, int len ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsPackBits_32u8u
//  Purpose:    Packing of bit's data from source to destination vector
//  Parameters:
//    pSrcBit       pointer to the input vector with data bits
//    pSrcBits      pointer to the input vector that specifies number of data bits
//    srcLen        length of the pSrcBit and pSrcBits vectors, number of items
//    pDst          pointer to the output vector
//    dstBitOffset  offset in the first byte of the destination vector
//    pDstLenBit    pointer to the length of the output vector, number of bits
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           length of the vectors is less or equal zero
//    ippStsSizeErr           dstBitOffset is less zero
//
//    ippStsOverlongString    Warning: pSrcBits[i] < 0, or pSrcBits[i] > 32.
//
//    ippStsNoErr             otherwise
*/

IPPAPI( IppStatus, ippsPackBits_32u8u,( const Ipp32u* pSrcBit,
                                        const int*    pSrcBits,
                                              int     srcLen,
                                              Ipp8u*  pDst,
                                              int     dstBitOffset,
                                              int*    pDstLenBit ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsCopy_1u
//  Purpose:    copy bit's data from source to destination vector
//  Parameters:
//    pSrc          pointer to the input vector
//    srcBitOffset  offset in the first byte of the source vector
//    pDst          pointer to the output vector
//    dstBitOffset  offset in the first byte of the destination vector
//    len           length of the vectors, number of bits
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           length of the vectors is less or equal zero
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippsCopy_1u,
      ( const Ipp8u* pSrc, int srcBitOffset, Ipp8u* pDst, int dstBitOffset, int len ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsMove
//  Purpose:    The ippsMove function copies "len" elements from src to dst.
//              If some regions of the source area and the destination overlap,
//              ippsMove ensures that the original source bytes in the overlapping
//              region are copied before being overwritten.
//
//  Parameters:
//    pSrc        pointer to the input vector
//    pDst        pointer to the output vector
//    len         length of the vectors, number of items
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           length of the vectors is less or equal zero
//    ippStsNoErr             otherwise
*/

IPPAPI ( IppStatus, ippsMove_8u,
                    ( const Ipp8u* pSrc, Ipp8u* pDst, int len ))
IPPAPI ( IppStatus, ippsMove_16s,
                    ( const Ipp16s* pSrc, Ipp16s* pDst, int len ))
IPPAPI ( IppStatus, ippsMove_16sc,
                    ( const Ipp16sc* pSrc, Ipp16sc* pDst, int len ))
IPPAPI ( IppStatus, ippsMove_32f,
                    ( const Ipp32f* pSrc, Ipp32f* pDst, int len ))
IPPAPI ( IppStatus, ippsMove_32fc,
                    ( const Ipp32fc* pSrc, Ipp32fc* pDst, int len ))
IPPAPI ( IppStatus, ippsMove_64f,
                    ( const Ipp64f* pSrc, Ipp64f* pDst, int len ))
IPPAPI ( IppStatus, ippsMove_64fc,
                    ( const Ipp64fc* pSrc, Ipp64fc* pDst, int len ))
IPPAPI ( IppStatus, ippsMove_32s,
                    ( const Ipp32s* pSrc, Ipp32s* pDst, int len ))
IPPAPI ( IppStatus, ippsMove_32sc,
                    ( const Ipp32sc* pSrc, Ipp32sc* pDst, int len ))
IPPAPI ( IppStatus, ippsMove_64s,
                    ( const Ipp64s* pSrc, Ipp64s* pDst, int len ))
IPPAPI ( IppStatus, ippsMove_64sc,
                    ( const Ipp64sc* pSrc, Ipp64sc* pDst, int len ))



/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsZero
//  Purpose:    set elements of the vector to zero of corresponding type
//  Parameters:
//    pDst       pointer to the destination vector
//    len        length of the vectors
//  Return:
//    ippStsNullPtrErr        pointer to the vector is NULL
//    ippStsSizeErr           length of the vectors is less or equal zero
//    ippStsNoErr             otherwise
*/

IPPAPI ( IppStatus, ippsZero_8u,( Ipp8u* pDst, int len ))
IPPAPI ( IppStatus, ippsZero_16s,( Ipp16s* pDst, int len ))
IPPAPI ( IppStatus, ippsZero_16sc,( Ipp16sc* pDst, int len ))
IPPAPI ( IppStatus, ippsZero_32f,( Ipp32f* pDst, int len ))
IPPAPI ( IppStatus, ippsZero_32fc,( Ipp32fc* pDst, int len ))
IPPAPI ( IppStatus, ippsZero_64f,( Ipp64f* pDst, int len ))
IPPAPI ( IppStatus, ippsZero_64fc,( Ipp64fc* pDst, int len ))
IPPAPI ( IppStatus, ippsZero_32s,( Ipp32s* pDst, int len ))
IPPAPI ( IppStatus, ippsZero_32sc,( Ipp32sc* pDst, int len ))
IPPAPI ( IppStatus, ippsZero_64s,( Ipp64s* pDst, int len ))
IPPAPI ( IppStatus, ippsZero_64sc,( Ipp64sc* pDst, int len ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsSet
//  Purpose:    set elements of the destination vector to the value
//  Parameters:
//    val        value to set the elements of the vector
//    pDst       pointer to the destination vector
//    len        length of the vectors
//  Return:
//    ippStsNullPtrErr        pointer to the vector is NULL
//    ippStsSizeErr           length of the vector is less or equal zero
//    ippStsNoErr             otherwise
*/

IPPAPI ( IppStatus, ippsSet_8u,( Ipp8u val, Ipp8u* pDst, int len ))
IPPAPI ( IppStatus, ippsSet_16s,( Ipp16s val, Ipp16s* pDst, int len ))
IPPAPI ( IppStatus, ippsSet_16sc,( Ipp16sc val, Ipp16sc* pDst, int len ))
IPPAPI ( IppStatus, ippsSet_32s,( Ipp32s val, Ipp32s* pDst, int len ))
IPPAPI ( IppStatus, ippsSet_32sc,( Ipp32sc val, Ipp32sc* pDst, int len ))
IPPAPI ( IppStatus, ippsSet_32f,( Ipp32f val, Ipp32f* pDst, int len ))
IPPAPI ( IppStatus, ippsSet_32fc,( Ipp32fc val, Ipp32fc* pDst, int len ))
IPPAPI ( IppStatus, ippsSet_64s,( Ipp64s val, Ipp64s* pDst, int len ))
IPPAPI ( IppStatus, ippsSet_64sc,( Ipp64sc val, Ipp64sc* pDst, int len ))
IPPAPI ( IppStatus, ippsSet_64f,( Ipp64f val, Ipp64f* pDst, int len ))
IPPAPI ( IppStatus, ippsSet_64fc,( Ipp64fc val, Ipp64fc* pDst, int len ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippsRandUniform_Direct_16s, ippsRandUniform_Direct_32f, ippsRandUniform_Direct_64f
//
//  Purpose:    Makes pseudo-random samples with a uniform distribution and places them in
//              the vector.
//
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         The pointer to vector is NULL
//    ippStsBadSizeErr         The length of the vector is less or equal zero
//
//  Arguments:
//    low                      The lower bounds of the uniform distributions range.
//    high                     The upper bounds of the uniform distributions range.
//    pSeed                    The pointer to the seed value used by the pseudo-random number
//                             generation algorithm.
//    pSrcDst                  The pointer to vector
//    len                      Vector's length
*/

IPPAPI(IppStatus, ippsRandUniform_Direct_16s, (Ipp16s* pDst, int len, Ipp16s low, Ipp16s high,
                                               unsigned int* pSeed))
IPPAPI(IppStatus, ippsRandUniform_Direct_32f, (Ipp32f* pDst, int len, Ipp32f low, Ipp32f high,
                                               unsigned int* pSeed))
IPPAPI(IppStatus, ippsRandUniform_Direct_64f, (Ipp64f* pDst, int len, Ipp64f low, Ipp64f high,
                                               unsigned int* pSeed))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippsRandGauss_Direct_16s, ippsRandGauss_Direct_32f, ippsRandGauss_Direct_64f
//
//  Purpose:    Makes pseudo-random samples with a Normal distribution distribution and places
//              them in the vector.
//
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         The pointer to vector is NULL
//    ippStsBadSizeErr         The length of the vector is less or equal zero
//
//  Arguments:
//    mean                     The mean of the Normal distribution.
//    stdev                    The standard deviation of the Normal distribution.
//    pSeed                    The pointer to the seed value used by the pseudo-random number
//                             generation algorithm.
//    pSrcDst                  The pointer to vector
//    len                      Vector's length
*/

IPPAPI(IppStatus, ippsRandGauss_Direct_16s, (Ipp16s* pDst, int len, Ipp16s mean, Ipp16s stdev,
                                            unsigned int* pSeed))
IPPAPI(IppStatus, ippsRandGauss_Direct_32f, (Ipp32f* pDst, int len, Ipp32f mean, Ipp32f stdev,
                                            unsigned int* pSeed))
IPPAPI(IppStatus, ippsRandGauss_Direct_64f, (Ipp64f* pDst, int len, Ipp64f mean, Ipp64f stdev,
                                            unsigned int* pSeed))

/* ///////////////////////////////////////////////////////////////////////// */
#if !defined( _OWN_BLDPCS )

struct RandUniState_8u;
struct RandUniState_16s;
struct RandUniState_32f;

typedef struct RandUniState_8u IppsRandUniState_8u;
typedef struct RandUniState_16s IppsRandUniState_16s;
typedef struct RandUniState_32f IppsRandUniState_32f;

struct RandGaussState_8u;
struct RandGaussState_16s;
struct RandGaussState_32f;

typedef struct RandGaussState_8u IppsRandGaussState_8u;
typedef struct RandGaussState_16s IppsRandGaussState_16s;
typedef struct RandGaussState_32f IppsRandGaussState_32f;

#endif

/* /////////////////////////////////////////////////////////////////////////
// Name:                ippsRandUniformInitAlloc_8u,  ippsRandUniformInitAlloc_16s,
//                      ippsRandUniformInitAlloc_32f
// Purpose:             Allocate and initialize parameters for the generator
//                      of noise with uniform distribution.
// Returns:
// Parameters:
//    pRandUniState     A pointer to the structure containing parameters for the
//                      generator of noise.
//    low               The lower bounds of the uniform distribution's range.
//    high              The upper bounds of the uniform distribution's range.
//    seed              The seed value used by the pseudo-random number generation
//                      algorithm.
//
// Returns:
//    ippStsNullPtrErr  pRandUniState==NULL
//    ippMemAllocErr    Can not allocate random uniform state
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsRandUniformInitAlloc_8u, (IppsRandUniState_8u** pRandUniState,
                                   Ipp8u low, Ipp8u high, unsigned int seed))

IPPAPI(IppStatus, ippsRandUniformInitAlloc_16s, (IppsRandUniState_16s** pRandUniState,
                                   Ipp16s low, Ipp16s high, unsigned int seed))

IPPAPI(IppStatus, ippsRandUniformInitAlloc_32f, (IppsRandUniState_32f** pRandUniState,
                                   Ipp32f low, Ipp32f high, unsigned int seed))

/* /////////////////////////////////////////////////////////////////////////
// Name:                     ippsRandUniform_8u,  ippsRandUniform_16s,
//                           ippsRandUniform_32f
// Purpose:                  Makes pseudo-random samples with a uniform distribution
//                           and places them in the vector.
// Parameters:
//    pDst                   The pointer to vector
//    len                    Vector's length
//    pRandUniState          A pointer to the structure containing parameters for the
//                           generator of noise
// Returns:
//    ippStsNullPtrErr       pRandUniState==NULL
//    ippStsContextMatchErr  pState->idCtx != idCtxRandUni
//    ippStsNoErr            No errors
*/

IPPAPI(IppStatus, ippsRandUniform_8u,  (Ipp8u* pDst,  int len, IppsRandUniState_8u* pRandUniState))
IPPAPI(IppStatus, ippsRandUniform_16s, (Ipp16s* pDst, int len, IppsRandUniState_16s* pRandUniState))
IPPAPI(IppStatus, ippsRandUniform_32f, (Ipp32f* pDst, int len, IppsRandUniState_32f* pRandUniState))

/* /////////////////////////////////////////////////////////////////////////
// Name:                     ippsRandUniformFree_8u, ippsRandUniformFree_16s
//                           ippsRandUniformFree_32f
// Purpose:                  Close random uniform state
//
// Parameters:
//    pRandUniState          Pointer to the random uniform state
//
// Returns:
//    ippStsNullPtrErr       pState==NULL
//    ippStsContextMatchErr  pState->idCtx != idCtxRandUni
//    ippStsNoErr,           No errors
*/

IPPAPI (IppStatus, ippsRandUniformFree_8u,  (IppsRandUniState_8u* pRandUniState))
IPPAPI (IppStatus, ippsRandUniformFree_16s, (IppsRandUniState_16s* pRandUniState))
IPPAPI (IppStatus, ippsRandUniformFree_32f, (IppsRandUniState_32f* pRandUniState))


/* //////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// Name:                ippsRandGaussInitAlloc_8u,  ippsRandGaussInitAlloc_16s,
//                      ippsRandGaussInitAlloc_32f
// Purpose:             Allocate and initialize parameters for the generator of noise.
// Returns:
// Parameters:
//    pRandGaussState   A pointer to the structure containing parameters for the
//                      generator of noise.
//    mean              The mean of the normal distribution.
//    stdDev            The standard deviation of the normal distribution.
//    seed              The seed value used by the pseudo-random number
//
// Returns:
//    ippStsNullPtrErr  pRandGaussState==NULL
//    ippMemAllocErr    Can not allocate normal random state
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsRandGaussInitAlloc_8u,   (IppsRandGaussState_8u** pRandGaussState,
                                   Ipp8u mean, Ipp8u stdDev, unsigned int seed))

IPPAPI(IppStatus, ippsRandGaussInitAlloc_16s, (IppsRandGaussState_16s** pRandGaussState,
                                   Ipp16s mean, Ipp16s stdDev, unsigned int seed))

IPPAPI(IppStatus, ippsRandGaussInitAlloc_32f, (IppsRandGaussState_32f** pRandGaussState,
                                   Ipp32f mean, Ipp32f stdDev, unsigned int seed))

/* /////////////////////////////////////////////////////////////////////////
// Name:                     ippsRandGauss_8u,  ippsRandGauss_16s,
//                           ippsRandGauss_32f
// Purpose:                  Makes pseudo-random samples with a normal distribution
//                           and places them in the vector.
// Parameters:
//    pDst                   The pointer to vector
//    len                    Vector's length
//    pRandUniState          A pointer to the structure containing parameters
//                           for the generator of noise
//    ippStsContextMatchErr  pState->idCtx != idCtxRandGauss
// Returns:
//    ippStsNullPtrErr       pRandGaussState==NULL
//    ippStsNoErr            No errors
*/

IPPAPI(IppStatus, ippsRandGauss_8u,  (Ipp8u* pDst,  int len, IppsRandGaussState_8u*  pRandGaussState))
IPPAPI(IppStatus, ippsRandGauss_16s, (Ipp16s* pDst, int len, IppsRandGaussState_16s* pRandGaussState))
IPPAPI(IppStatus, ippsRandGauss_32f, (Ipp32f* pDst, int len, IppsRandGaussState_32f* pRandGaussState))

/* /////////////////////////////////////////////////////////////////////////
// Name:                     ippsRandGaussFree_8u, ippsRandGaussFree_16s,
//                           ippsRandGaussFree_32f
// Purpose:                  Close random normal state
//
// Parameters:
//    pRandUniState          Pointer to the random normal state
//
// Returns:
//    ippStsNullPtrErr       pState==NULL
//    ippStsContextMatchErr  pState->idCtx != idCtxRandGauss
//    ippStsNoErr,           No errors
*/

IPPAPI (IppStatus, ippsRandGaussFree_8u,  (IppsRandGaussState_8u*  pRandGaussState))
IPPAPI (IppStatus, ippsRandGaussFree_16s, (IppsRandGaussState_16s* pRandGaussState))
IPPAPI (IppStatus, ippsRandGaussFree_32f, (IppsRandGaussState_32f* pRandGaussState))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippsRandGaussGetSize_16s
//
//  Purpose:    Gaussian sequence generator state variable size -
//              computes the size,in bytes,
//              of the state variable structure ippsRandGaussState_16s.
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         pRandGaussStateSize==NULL
//  Arguments:
//    pRandGaussStateSize      pointer to the computed values of the size
//                             of the structure ippsRandGaussState_16s.
*/
IPPAPI(IppStatus, ippsRandGaussGetSize_16s, (int * pRandGaussStateSize))

/* //////////////////////////////////////////////////////////////////////////////////
// Name:                ippsRandGaussInit_16s
// Purpose:             Initializes the Gaussian sequence generator state structure with
//                      given parameters (mean, variance, seed).
// Parameters:
//    pRandGaussState   A pointer to the structure containing parameters for the
//                      generator of noise.
//    mean              Mean of the normal distribution.
//    stdDev            Standard deviation of the normal distribution.
//    seed              Seed value used by the pseudo-random number generator
//
// Returns:
//    ippStsNullPtrErr  pRandGaussState==NULL
//    ippMemAllocErr    Can not allocate normal random state
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsRandGaussInit_16s, (IppsRandGaussState_16s* pRandGaussState,
                                   Ipp16s mean, Ipp16s stdDev, unsigned int seed))


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippsRandUniformGetSize_16s
//
//  Purpose:    Uniform sequence generator state variable size -
//              computes the size,in bytes,
//              of the state variable structure ippsRandIniState_16s.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         pRandUniformStateSize==NULL
//  Arguments:
//    pRandGaussStateSize      pointer to the computed value of the size
//                             of the structure ippsRandUniState_16s.
*/
IPPAPI(IppStatus, ippsRandUniformGetSize_16s, (int * pRandUniformStateSize))


/* //////////////////////////////////////////////////////////////////////////////////
// Name:                ippsRandUniformInit_16s
// Purpose:             Initializes the uniform sequence generator state structure with
//                      given parameters (boundaries, seed)
// Parameters:
//    pRandUniState     Pointer to the structure containing parameters for the
//                      generator of noise.
//    low               Lower bound of the uniform distribution's range.
//    high              Upper bounds of the uniform distribution's range.
//    seed              Seed value used by the pseudo-random number generation
//                      algorithm.
//
*/
IPPAPI(IppStatus, ippsRandUniformInit_16s, (IppsRandUniState_16s* pRandUniState,
                                   Ipp16s low, Ipp16s high, unsigned int seed))




/* /////////////////////////////////////////////////////////////////////////
//  Name:               ippsVectorJaehne
//  Purpose:            creates Jaehne vector
//
//  Parameters:
//    pDst              the pointer to the destination vector
//    len               length of the vector
//    magn              magnitude of the signal
//
//  Return:
//    ippStsNoErr       indicates no error
//    ippStsNullPtrErr  indicates an error when the pDst pointer is NULL
//    ippStsBadSizeErr  indicates an error when len is less or equal zero
//    ippStsJaehneErr   indicates an error when magnitude value is negative
//
//  Notes:              pDst[n] = magn*sin(0.5*pi*n^2/len), n=0,1,2,..len-1.
//
*/
IPPAPI (IppStatus, ippsVectorJaehne_8u,  (Ipp8u*  pDst, int len, Ipp8u magn))
IPPAPI (IppStatus, ippsVectorJaehne_8s,  (Ipp8s*  pDst, int len, Ipp8s magn))
IPPAPI (IppStatus, ippsVectorJaehne_16u, (Ipp16u* pDst, int len, Ipp16u magn))
IPPAPI (IppStatus, ippsVectorJaehne_16s, (Ipp16s* pDst, int len, Ipp16s magn))
IPPAPI (IppStatus, ippsVectorJaehne_32u, (Ipp32u* pDst, int len, Ipp32u magn))
IPPAPI (IppStatus, ippsVectorJaehne_32s, (Ipp32s* pDst, int len, Ipp32s magn))
IPPAPI (IppStatus, ippsVectorJaehne_32f, (Ipp32f* pDst, int len, Ipp32f magn))
IPPAPI (IppStatus, ippsVectorJaehne_64f, (Ipp64f* pDst, int len, Ipp64f magn))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippsTone_Direct
//  Purpose:        generates a tone
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   Some of pointers to input or output data are NULL
//    ippStsSizeErr      The length of vector is less or equal zero
//    ippStsToneMagnErr  The magn value is less than or equal to zero
//    ippStsToneFreqErr  The rFreq value is less than 0 or greater than or equal to 0.5
//                       for real tone and 1.0 for complex tone
//    ippStsTonePhaseErr The phase value is less 0 or greater or equal 2*PI
//  Parameters:
//    magn            Magnitude of the tone; that is, the maximum value
//                    attained by the wave
//    rFreq           Frequency of the tone relative to the sampling
//                    frequency. It must be in range [0.0, 0.5) for real, and
//                    [0.0, 1.0) for complex tone
//    pPhase          Phase of the tone relative to a cosinewave. It must
//                    be in range [0.0, 2*PI).
//    pDst            Pointer to the destination vector.
//    len             Length of the vector
//    hint            Suggests using specific code
//  Notes:
//    for real:  pDst[i] = magn * cos(IPP_2PI * rfreq * i + phase);
//    for cplx:  pDst[i].re = magn * cos(IPP_2PI * rfreq * i + phase);
//               pDst[i].im = magn * sin(IPP_2PI * rfreq * i + phase);
*/


IPPAPI(IppStatus, ippsTone_Direct_32f, (Ipp32f* pDst, int len, float magn,
                                float rFreq, float* pPhase, IppHintAlgorithm hint))
IPPAPI(IppStatus, ippsTone_Direct_32fc, (Ipp32fc* pDst, int len, float magn,
                                float rFreq, float* pPhase, IppHintAlgorithm hint))
IPPAPI(IppStatus, ippsTone_Direct_64f, (Ipp64f* pDst, int len, double magn,
                               double rFreq, double* pPhase, IppHintAlgorithm hint))
IPPAPI(IppStatus, ippsTone_Direct_64fc, (Ipp64fc* pDst, int len, double magn,
                               double rFreq, double* pPhase, IppHintAlgorithm hint))
IPPAPI(IppStatus, ippsTone_Direct_16s, (Ipp16s* pDst, int len, Ipp16s magn,
                                float rFreq, float* pPhase, IppHintAlgorithm hint))
IPPAPI(IppStatus, ippsTone_Direct_16sc, (Ipp16sc* pDst, int len, Ipp16s magn,
                                float rFreq, float* pPhase, IppHintAlgorithm hint))

#if !defined ( _OWN_BLDPCS )
struct ToneState_16s;
typedef struct ToneState_16s IppToneState_16s;
#endif


/*
//  Name:                ippsToneInitAllocQ15_16s
//  Purpose:             Allocates memory for the structure IppToneState_16s,
//                       initializes it with a set of cosinwave parameters (magnitude,
//                       frequency, phase).
//  Context:
//  Returns:             IppStatus
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   Double pointer to pToneState is NULL
//    ippStsToneMagnErr  The magn value is less than or equal to zero
//    ippStsToneFreqErr  The freqQ15 value is less than 0 or greater than 16383
//    ippStsTonePhaseErr The phaseQ15 value is less than 0 or greater than 205886
//  Parameters:
//    **pToneState       Double pointer to the structure IppToneState_16s.
//    magn               Magnitude of the tone; that is, the maximum value
//                       attained by the wave.
//    rFreqQ15           Frequency of the tone relative to the sampling
//                       frequency. It must be between 0 and 16383
//    phaseQ15           Phase of the tone relative to a cosinewave. It must
//                       be between 0 and 205886.
//  Notes:
*/
IPPAPI(IppStatus, ippsToneInitAllocQ15_16s, (IppToneState_16s** pToneState,
                                             Ipp16s magn, Ipp16s rFreqQ15, Ipp32s phaseQ15))

/*
//  Name:                ippsToneFree_16s
//  Purpose:             Frees memory, which was allocated
//                       for the structure IppToneState_16s.
//  Context:
//  Returns:             IppStatus
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   Pointer to pToneState is NULL
//  Parameters:
//    *pToneState        Pointer to the structure IppToneState_16s.
//  Notes:
*/
IPPAPI(IppStatus, ippsToneFree, (IppToneState_16s* pToneState))

/*
//  Name:                ippsToneGetStateSizeQ15_16s
//  Purpose:             Computes the size, in bytes, of the structure IppToneState_16s
//  Context:
//  Returns:             IppStatus
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   Pointer to pToneState size is NULL
//  Parameters:
//    *pToneStateSize    Pointer to the computed value of the size
//                       of the structure IppToneState_16s.
//  Notes:
*/
IPPAPI(IppStatus, ippsToneGetStateSizeQ15_16s, (int* pToneStateSize))

/*
//  Name:                ippsToneInitQ15_16s
//  Purpose:             initializes the structure IppToneState_16s with
//                       given set of cosinewave parameters (magnitude,
//                       frequency, phase)
//  Context:
//  Returns:             IppStatus
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   Pointer to pToneState is NULL
//    ippStsToneMagnErr  The magn value is less than or equal to zero
//    ippStsToneFreqErr  The rFreqQ15 value is less than 0 or greater 16383
//    ippStsTonePhaseErr The phaseQ15 value is less than 0 or greater 205886
//  Parameters:
//    *pToneState        Pointer to the structure IppToneState_16s.
//    magn               Magnitude of the tone; that is, the maximum value
//                       attained by the wave.
//    rFreqQ15           Frequency of the tone relative to the sampling
//                       frequency. It must be between 0 and 16383
//    phaseQ15           Phase of the tone relative to a cosinewave. It must
//                       be between 0 and 205886.
//  Notes:
*/
IPPAPI(IppStatus, ippsToneInitQ15_16s, (IppToneState_16s* pToneState, Ipp16s magn,
                                        Ipp16s rFreqQ15, Ipp32s phaseQ15))

/*
//  Name:                ippsToneQ15_16s
//  Purpose:             generates a tone
//  Context:
//  Returns:             IppStatus
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   One of the specified pointers is NULL
//    ippStsSizeErr      len is less than or equal to 0
//  Parameters:
//    pDst               Pointer to the destination vector.
//    len                Length of the vector
//    *pToneState        Pointer to the structure IppToneState_16s.
//  Notes:
*/

IPPAPI(IppStatus, ippsToneQ15_16s, (Ipp16s* pDst, int len, IppToneState_16s* pToneState))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippsTriangle_Direct
//  Purpose:        generates a Triangle
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   Some of pointers to input or output data are NULL
//    ippStsSizeErr       The length of vector is less or equal zero
//    ippStsTrnglMagnErr  The magn value is less or equal to zero
//    ippStsTrnglFreqErr  The rfreq value is less 0 or greater or equal 0.5
//    ippStsTrnglPhaseErr The phase value is less 0 or greater or equal 2*PI
//    ippStsTrnglAsymErr  The asym value is less -PI or greater or equal PI
//  Parameters:
//    magn            Magnitude of the Triangle; that is, the maximum value
//                    attained by the wave
//    rFreq           Frequency of the Triangle relative to the sampling
//                    frequency. It must be in range [0.0, 0.5)
//    pPhase          POinter to the phase of the Triangle relative to acosinewave. It must
//                    be in range [0.0, 2*PI)
//    asym            Asymmetry of a triangle. It must be in range [-PI,PI).
//    pDst            Pointer to destination vector.
//    len             Length of the vector
*/


IPPAPI(IppStatus, ippsTriangle_Direct_64f, (Ipp64f* pDst, int len, double magn,
                                double rFreq, double asym, double* pPhase))
IPPAPI(IppStatus, ippsTriangle_Direct_64fc, (Ipp64fc* pDst, int len, double magn,
                                double rFreq, double asym, double* pPhase))
IPPAPI(IppStatus, ippsTriangle_Direct_32f, (Ipp32f* pDst, int len, float magn,
                                float rFreq, float asym, float* pPhase))
IPPAPI(IppStatus, ippsTriangle_Direct_32fc, (Ipp32fc* pDst, int len, float magn,
                                float rFreq, float asym, float* pPhase))
IPPAPI(IppStatus, ippsTriangle_Direct_16s, (Ipp16s* pDst, int len, Ipp16s magn,
                                float rFreq, float asym, float* pPhase))
IPPAPI(IppStatus, ippsTriangle_Direct_16sc, (Ipp16sc* pDst, int len, Ipp16s magn,
                                float rFreq, float asym, float* pPhase))

#if !defined ( _OWN_BLDPCS )
/* IPP Context triangle identification */
struct TriangleState_16s;
typedef struct TriangleState_16s IppTriangleState_16s;
#endif

/*
//  Name:                ippsTriangleInitAllocQ15_16s
//  Purpose:             Allocates memory for the structure IppTriangleState_16s,
//                       initializes it with a set of wave parameters (magnitude,
//                       frequency, phase, asymmetry).
//  Context:
//  Returns:             IppStatus
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   Double pointer to pTriangleState is NULL
//    ippStsTriangleMagnErr  The magn value is less than or equal to zero
//    ippStsTriangleFreqErr  The freqQ15 value is less than 0 or greater than 16383
//    ippStsTriangleAsymErr  The phaseQ15 value is less than 0 or greater than 205886
//    ippStsTrianglePhaseErr The asymQ15 value is less than -102943 or greater than 102943
//  Parameters:
//    **pTriangleState   Double pointer to the structure IppTriangleState_16s.
//    magn               Magnitude of the Triangle; that is, the maximum value
//                       attained by the wave.
//    rFreqQ15           Frequency of the Triangle relative to the sampling
//                       frequency. It must be between 0 and 16383
//    phaseQ15           Phase of the Triangle relative to a wave. It must
//                       be between 0 and 205886.
//    asymQ15            Asymmetry of the Triangle relative to a wave. It must
//                       be between -102943 and 102943.
//  Notes:
*/
IPPAPI(IppStatus, ippsTriangleInitAllocQ15_16s, (IppTriangleState_16s** pTriangleState,
                         Ipp16s magn, Ipp16s rFreqQ15, Ipp32s phaseQ15, Ipp32s asymQ15))



/*
//  Name:                ippsTriangleFree_16s
//  Purpose:             Frees memory, which was allocated
//                       for the structure IppTriangleState_16s.
//  Context:
//  Returns:             IppStatus
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   Pointer to pTriangleState is NULL
//  Parameters:
//    *pTriangleState    Pointer to the structure IppTriangleState_16s.
//  Notes:
*/
IPPAPI(IppStatus, ippsTriangleFree, (IppTriangleState_16s* pTriangleState))



/*
//  Name:                ippsTriangleGetStateSizeQ15_16s
//  Purpose:             Computes the size, in bytes, of the structure IppTriangleState_16s
//  Context:
//  Returns:             IppStatus
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   Pointer to pTriangleState size is NULL
//  Parameters:
//    *pTriangleStateSize Pointer to the computed value of the size
//                        of the structure IppTriangleState_16s.
//  Notes:
*/
IPPAPI(IppStatus, ippsTriangleGetStateSizeQ15_16s, (int* pTriangleStateSize))

/*
//  Name:                ippsTriangleInitQ15_16s
//  Purpose:             Initializes the structure IppTriangleState_16s
//                       with a given set of cosinewave parameters (magnitude,
//                       frequency, phase)
//  Context:
//  Returns:               IppStatus
//    ippStsNoErr          Ok
//    ippStsNullPtrErr     The pointer to pTriangleState is NULL
//    ippStsTrngleMagnErr  The magn value is less than or equal to zero
//    ippStsTrngleFreqErr  The freqQ15 value is less than 0 or greater than 16383
//    ippStsTrnglePhaseErr The phaseQ15 value is less than 0 or greater than 205886
//    ippStsTrngleAsymErr  The asymQ15 value is less than -102943 or greater than 102943
//  Parameters:
//    *pTriangleState    Pointer to the structure IppTriangleState_16s.
//    magn               Magnitude of the Triangle; that is, the maximum value
//                       attained by the wave.
//    rFreqQ15           Frequency of the Triangle relative to the sampling
//                       frequency. It must be between 0 and 16383
//    phaseQ15           Phase of the Triangle relative to a wave. It must
//                       be between 0 and 205886.
//    asymQ15            Asymmetry of the Triangle relative to a wave. It must
//                       be between -102943 and 102943.

//  Notes:
*/
IPPAPI(IppStatus, ippsTriangleInitQ15_16s, (IppTriangleState_16s* pTriangleState,
                  Ipp16s magn, Ipp16s rFreqQ15, Ipp32s phaseQ15, Ipp32s asymQ15))


/*
//  Name:                ippsTriangleQ15_16s
//  Purpose:             generates a Triangle
//  Context:
//  Returns:             IppStatus
//    ippStsNoErr        Ok
//  Parameters:
//    pDst               The pointer to destination vector.
//    len                The length of vector
//    *pTriangleState    Pointer to the structure IppTriangleState_16s.
//  Notes:
*/
IPPAPI(IppStatus, ippsTriangleQ15_16s, (Ipp16s* pDst, int len, IppTriangleState_16s* pTriangleState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippsToneQ15_Direct_16s
//  Purpose:        generates a tone
//  Context:
//  Returns:             IppStatus
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   The pointer to the destination vector is NULL
//    ippStsSizeErr      The length of the vector is less than or equal to zero
//    ippStsToneMagnErr  The magn value is less than or equal to zero
//    ippStsToneFreqErr  The rFreqQ15 value is less than 0 or greater than 16383
//    ippStsTonePhaseErr The phaseQ15 value is less than 0 or greater than 205886

//  Parameters:
//    pDst          Pointer to the destination vector.
//    len           Length of the vector
//    magn          Magnitude of the tone; that is, the maximum value
//                  attained by the wave.It must be between 0 and 32767
//    rFreqQ15      Frequency of the tone relative to the sampling
//                  frequency. It must be between 0 and 16383
//    phaseQ15      Phase of the tone relative to a cosinewave. It must
//                  be between 0 and 205886.
*/
IPPAPI(IppStatus, ippsToneQ15_Direct_16s, (Ipp16s* pDst, int len,
                                          Ipp16s magn, Ipp16s rFreqQ15, Ipp32s phaseQ15))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippsTriangleQ15_Direct_16s
//  Purpose:        generates a Triangle
//  Context:
//  Returns:                 IppStatus
//    ippStsNoErr            Ok
//    ippStsNullPtrErr       The pointer to the destination vector is NULL
//    ippStsSizeErr          The length of the vector is less than or equal to zero
//    ippStsTriangleMagnErr  The magn value is less than or equal to zero
//    ippStsTriangleFreqErr  The rFfreqQ15 value is less than 0 or greater than 16383
//    ippStsTriangleAsymErr  The asymQ15 value is less than 0 or greater than 205886
//    ippStsTrianglePhaseErr The phaseQ15 value is less than -102943 or greater than 102943
//  Parameters:
//    pDst          Pointer to the destination vector.
//    len           Length of the vector
//    mag           Magnitude of the Triangle; that is, the maximum value
//                  attained by the wave. It must be between 0 and 32767.
//    rFreqQ15      Frequency of the Triangle relative to the sampling
//                  frequency. It must be between 0 and 16383
//    phaseQ15      The phase of the Triangle relative to a wave. It must
//                  be between 0 and 205886.
//    asymQ15       The asymmetry of the Triangle relative to a wave. It must
//                  be between -102943 and 102943.
//  Notes:
*/
IPPAPI(IppStatus, ippsTriangleQ15_Direct_16s, ( Ipp16s* pDst, int len,
                                                Ipp16s magn, Ipp16s rFreqQ15,
                                                Ipp32s phaseQ15, Ipp32s asymQ15))



/* /////////////////////////////////////////////////////////////////////////
//  Name:               ippsVectorRamp_8u,  ippsVectorRamp_8s,
//                      ippsVectorRamp_16u, ippsVectorRamp_16s,
//                      ippsVectorRamp_32u, ippsVectorRamp_32s,
//                      ippsVectorRamp_32f, ippsVectorRamp_64f
//  Purpose:            Creates ramp vector
//
//  Parameters:
//    pDst              A pointer to the destination vector
//    len               Vector's length
//    offset            Offset value
//    slope             Slope coefficient
//
//  Return:
//    ippStsNoErr       No error
//    ippStsNullPtrErr  pDst pointer is NULL
//    ippStsBadSizeErr  Vector's length is less or equal zero
//    ippStsNoErr       No error
//
//  Notes:              Dst[n] = offset + slope * n
//
*/
IPPAPI (IppStatus, ippsVectorRamp_8u,  (Ipp8u*  pDst, int len, float offset, float slope))
IPPAPI (IppStatus, ippsVectorRamp_8s,  (Ipp8s*  pDst, int len, float offset, float slope))
IPPAPI (IppStatus, ippsVectorRamp_16u, (Ipp16u* pDst, int len, float offset, float slope))
IPPAPI (IppStatus, ippsVectorRamp_16s, (Ipp16s* pDst, int len, float offset, float slope))
IPPAPI (IppStatus, ippsVectorRamp_32u, (Ipp32u* pDst, int len, float offset, float slope))
IPPAPI (IppStatus, ippsVectorRamp_32s, (Ipp32s* pDst, int len, float offset, float slope))
IPPAPI (IppStatus, ippsVectorRamp_32f, (Ipp32f* pDst, int len, float offset, float slope))
IPPAPI (IppStatus, ippsVectorRamp_64f, (Ipp64f* pDst, int len, float offset, float slope))



/* /////////////////////////////////////////////////////////////////////////////
//                   Convert functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsReal
//  Purpose:    form vector with real part of the input complex vector
//  Parameters:
//    pSrc       pointer to the input complex vector
//    pDstRe     pointer to the output vector to store the real part
//    len        length of the vectors, number of items
//  Return:
//    ippStsNullPtrErr       pointer(s) to the data is NULL
//    ippStsSizeErr          length of the vectors is less or equal zero
//    ippStsNoErr            otherwise
*/

IPPAPI(IppStatus, ippsReal_64fc,(const Ipp64fc* pSrc, Ipp64f* pDstRe, int len))
IPPAPI(IppStatus, ippsReal_32fc,(const Ipp32fc* pSrc, Ipp32f* pDstRe, int len))
IPPAPI(IppStatus, ippsReal_16sc,(const Ipp16sc* pSrc, Ipp16s* pDstRe, int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsImag
//  Purpose:    form vector with imaginary part of the input complex vector
//  Parameters:
//    pSrc       pointer to the input complex vector
//    pDstRe     pointer to the output vector to store the real part
//    len        length of the vectors, number of items
//  Return:
//    ippStsNullPtrErr       pointer(s) to the data is NULL
//    ippStsSizeErr          length of the vectors is less or equal zero
//    ippStsNoErr            otherwise
*/

IPPAPI(IppStatus, ippsImag_64fc,(const Ipp64fc* pSrc, Ipp64f* pDstIm, int len))
IPPAPI(IppStatus, ippsImag_32fc,(const Ipp32fc* pSrc, Ipp32f* pDstIm, int len))
IPPAPI(IppStatus, ippsImag_16sc,(const Ipp16sc* pSrc, Ipp16s* pDstIm, int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsCplxToReal
//  Purpose:    form the real and imaginary parts of the input complex vector
//  Parameters:
//    pSrc       pointer to the input complex vector
//    pDstRe     pointer to output vector to store the real part
//    pDstIm     pointer to output vector to store the imaginary part
//    len        length of the vectors, number of items
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           length of the vectors is less or equal zero
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippsCplxToReal_64fc,( const Ipp64fc* pSrc, Ipp64f* pDstRe,
       Ipp64f* pDstIm, int len ))
IPPAPI(IppStatus, ippsCplxToReal_32fc,( const Ipp32fc* pSrc, Ipp32f* pDstRe,
       Ipp32f* pDstIm, int len ))
IPPAPI(IppStatus, ippsCplxToReal_16sc,( const Ipp16sc* pSrc, Ipp16s* pDstRe,
       Ipp16s* pDstIm, int len ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsRealToCplx
//  Purpose:    form complex vector from the real and imaginary components
//  Parameters:
//    pSrcRe     pointer to the input vector with real part, may be NULL
//    pSrcIm     pointer to the input vector with imaginary part, may be NULL
//    pDst       pointer to the output complex vector
//    len        length of the vectors
//  Return:
//    ippStsNullPtrErr        pointer to the destination data is NULL
//    ippStsSizeErr           length of the vectors is less or equal zero
//    ippStsNoErr             otherwise
//
//  Notes:      one of the two input pointers may be NULL. In this case
//              the corresponding values of the output complex elements is 0
*/

IPPAPI(IppStatus, ippsRealToCplx_64f,( const Ipp64f* pSrcRe,
       const Ipp64f* pSrcIm, Ipp64fc* pDst, int len ))
IPPAPI(IppStatus, ippsRealToCplx_32f,( const Ipp32f* pSrcRe,
       const Ipp32f* pSrcIm, Ipp32fc* pDst, int len ))
IPPAPI(IppStatus, ippsRealToCplx_16s,( const Ipp16s* pSrcRe,
       const Ipp16s* pSrcIm, Ipp16sc* pDst, int len ))




/* /////////////////////////////////////////////////////////////////////////////
//  Names:       ippsConj, ippsConjFlip
//  Purpose:     complex conjugate data vector
//  Parameters:
//    pSrc               pointer to the input vector
//    pDst               pointer to the output vector
//    len                length of the vectors
//  Return:
//    ippStsNullPtrErr      pointer(s) to the data is NULL
//    ippStsSizeErr         length of the vectors is less or equal zero
//    ippStsNoErr           otherwise
//  Notes:
//    the ConjFlip version conjugates and stores result in reverse order
*/

IPPAPI ( IppStatus, ippsConj_64fc_I, ( Ipp64fc* pSrcDst, int len ))
IPPAPI ( IppStatus, ippsConj_32fc_I, ( Ipp32fc* pSrcDst, int len ))
IPPAPI ( IppStatus, ippsConj_16sc_I, ( Ipp16sc* pSrcDst, int len ))
IPPAPI ( IppStatus, ippsConj_64fc,
                    ( const Ipp64fc* pSrc, Ipp64fc* pDst, int len ))
IPPAPI ( IppStatus, ippsConj_32fc,
                    ( const Ipp32fc* pSrc, Ipp32fc* pDst, int len ))
IPPAPI ( IppStatus, ippsConj_16sc,
                    ( const Ipp16sc* pSrc, Ipp16sc* pDst, int len ))
IPPAPI ( IppStatus, ippsConjFlip_64fc,
                    ( const Ipp64fc* pSrc, Ipp64fc* pDst, int len ))
IPPAPI ( IppStatus, ippsConjFlip_32fc,
                    ( const Ipp32fc* pSrc, Ipp32fc* pDst, int len ))
IPPAPI ( IppStatus, ippsConjFlip_16sc,
                    ( const Ipp16sc* pSrc, Ipp16sc* pDst, int len ))
IPPAPI ( IppStatus, ippsConjCcs_64fc_I,
                    ( Ipp64fc* pSrcDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjCcs_32fc_I,
                    ( Ipp32fc* pSrcDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjCcs_16sc_I,
                    ( Ipp16sc* pSrcDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjCcs_64fc,
                    ( const Ipp64f* pSrc, Ipp64fc* pDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjCcs_32fc,
                    ( const Ipp32f* pSrc, Ipp32fc* pDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjCcs_16sc,
                    ( const Ipp16s* pSrc, Ipp16sc* pDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjPack_64fc_I,
                    ( Ipp64fc* pSrcDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjPack_32fc_I,
                    ( Ipp32fc* pSrcDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjPack_16sc_I,
                    ( Ipp16sc* pSrcDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjPack_64fc,
                    ( const Ipp64f* pSrc, Ipp64fc* pDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjPack_32fc,
                    ( const Ipp32f* pSrc, Ipp32fc* pDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjPack_16sc,
                    ( const Ipp16s* pSrc, Ipp16sc* pDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjPerm_64fc_I,
                    ( Ipp64fc* pSrcDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjPerm_32fc_I,
                    ( Ipp32fc* pSrcDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjPerm_16sc_I,
                    ( Ipp16sc* pSrcDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjPerm_64fc,
                    ( const Ipp64f* pSrc, Ipp64fc* pDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjPerm_32fc,
                    ( const Ipp32f* pSrc, Ipp32fc* pDst, int lenDst ))
IPPAPI ( IppStatus, ippsConjPerm_16sc,
                    ( const Ipp16s* pSrc, Ipp16sc* pDst, int lenDst ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConvert
//  Purpose:    Converts integer data to floating point data
//  Parameters:
//    pSrc        pointer to integer data to be converted
//    pDst        pointer to the destination vector
//    len         length of the vectors
//  Return:
//    ippStsNullPtrErr    pointer(s) to the data is NULL
//    ippStsSizeErr       length of the vectors is less or equal zero
//    ippStsNoErr         otherwise
*/
IPPAPI(IppStatus,ippsConvert_8s16s,(const Ipp8s* pSrc,Ipp16s* pDst,int len))
IPPAPI(IppStatus,ippsConvert_16s32s,(const Ipp16s* pSrc, Ipp32s* pDst, int len))
IPPAPI(IppStatus,ippsConvert_32s16s,(const Ipp32s* pSrc, Ipp16s* pDst, int len))
IPPAPI(IppStatus,ippsConvert_8s32f,(const Ipp8s* pSrc,Ipp32f* pDst,int len))
IPPAPI(IppStatus,ippsConvert_8u32f,(const Ipp8u* pSrc,Ipp32f* pDst,int len))
IPPAPI(IppStatus,ippsConvert_16s32f,(const Ipp16s* pSrc,Ipp32f* pDst,int len))
IPPAPI(IppStatus,ippsConvert_16u32f,(const Ipp16u* pSrc,Ipp32f* pDst,int len))
IPPAPI(IppStatus,ippsConvert_32s64f,(const Ipp32s* pSrc,Ipp64f* pDst,int len))
IPPAPI(IppStatus,ippsConvert_32s32f,(const Ipp32s* pSrc,Ipp32f* pDst,int len))
IPPAPI(IppStatus,ippsConvert_64s64f,(const Ipp64s* pSrc,Ipp64f* pDst, Ipp32u len))
IPPAPI(IppStatus,ippsConvert_16s8s_Sfs,(const Ipp16s* pSrc, Ipp8s* pDst,
       Ipp32u len, IppRoundMode rndMode, int scaleFactor))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConvert
//  Purpose:    convert floating point data to integer data
//  Parameters:
//    pSrc         pointer to the input floating point data to be converted
//    pDst         pointer to destination vector
//    len          length of the vectors
//    rndMode      Rounding mode which can be ippRndZero or ippRndNear
//    scaleFactor  scale factor value
//  Return:
//    ippStsNullPtrErr    pointer(s) to the data NULL
//    ippStsSizeErr       length of the vectors is less or equal zero
//    ippStsNoErr         otherwise
//  Note:
//    an out-of-range result will be saturated
*/

IPPAPI(IppStatus,ippsConvert_32f8s_Sfs,(const Ipp32f* pSrc, Ipp8s* pDst,
       int len, IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus,ippsConvert_32f8u_Sfs,(const Ipp32f* pSrc, Ipp8u* pDst,
       int len, IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus,ippsConvert_32f16s_Sfs,(const Ipp32f* pSrc, Ipp16s* pDst,
       int len, IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus,ippsConvert_32f16u_Sfs,(const Ipp32f* pSrc, Ipp16u* pDst,
       int len, IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus,ippsConvert_64f32s_Sfs,(const Ipp64f* pSrc, Ipp32s* pDst,
       int len, IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus,ippsConvert_32f32s_Sfs,(const Ipp32f* pSrc, Ipp32s* pDst,
       int len, IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus,ippsConvert_64f16s_Sfs,(const Ipp64f* pSrc, Ipp16s* pDst,
       int len, IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus,ippsConvert_64f64s_Sfs,(const Ipp64f* pSrc, Ipp64s* pDst,
       Ipp32u len, IppRoundMode rndMode, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConvert_32f64f
//  Purpose:    Converts floating point data Ipp32f
//              to floating point data Ipp64f
//  Parameters:
//    pSrc          pointer to the input vector
//    pDst          pointer to the output vector
//    len           length of the vectors
//  Return:
//    ippStsNullPtrErr    pointer(s) to the data is NULL
//    ippStsSizeErr       length of the vectors is less or equal zero
//    ippStsNoErr         otherwise
*/

IPPAPI ( IppStatus, ippsConvert_32f64f,
       ( const Ipp32f* pSrc, Ipp64f* pDst, int len ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConvert_64f32f
//  Purpose:    Converts floating point data Ipp64f
//              to floating point data Ipp32f
//  Parameters:
//    pSrc          pointer to the input vector
//    pDst          pointer to the output vector
//    len           length of the vectors
//  Return:
//    ippStsNullPtrErr    pointer(s) to the data is NULL
//    ippStsSizeErr       length of the vectors is less or equal zero
//    ippStsNoErr         otherwise
//  Note:
//    an out-of-range result will be saturated
*/

IPPAPI ( IppStatus, ippsConvert_64f32f,
       ( const Ipp64f* pSrc, Ipp32f* pDst, int len ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConvert
//  Purpose:    Converts integer data to floating point data
//  Parameters:
//    pSrc          pointer to integer data to be converted
//    pDst          pointer to the destination vector
//    len           length of the vectors
//    scaleFactor   scale factor value
//  Return:
//    ippStsNullPtrErr    pointer(s) to the data is NULL
//    ippStsSizeErr       length of the vectors is less or equal zero
//    ippStsNoErr         otherwise
*/

IPPAPI ( IppStatus, ippsConvert_16s32f_Sfs,
        ( const Ipp16s* pSrc, Ipp32f* pDst, int len, int scaleFactor ))
IPPAPI ( IppStatus, ippsConvert_16s64f_Sfs,
        ( const Ipp16s* pSrc, Ipp64f* pDst, int len, int scaleFactor ))
IPPAPI ( IppStatus, ippsConvert_32s32f_Sfs,
        ( const Ipp32s* pSrc, Ipp32f* pDst, int len, int scaleFactor ))
IPPAPI ( IppStatus, ippsConvert_32s64f_Sfs,
        ( const Ipp32s* pSrc, Ipp64f* pDst, int len, int scaleFactor ))
IPPAPI( IppStatus, ippsConvert_32s16s_Sfs,
                   ( const Ipp32s* pSrc, Ipp16s* pDst, int len,
                       int scaleFactor ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConvert
//  Purpose:    Converts 24u data to 32u or 32f data.
//              Converts 32u or 32f data to 24u data.
//              Converts 24s data to 32s or 32f data.
//              Converts 32s or 32f data to 24s data.
//  Parameters:
//    pSrc          pointer to the input vector
//    pDst          pointer to the output vector
//    len           length of the vectors
//    scaleFactor   scale factor value
//  Return:
//    ippStsNullPtrErr    pointer(s) to the data is NULL
//    ippStsSizeErr       length of the vectors is less or equal zero
//    ippStsNoErr         otherwise
*/

IPPAPI( IppStatus, ippsConvert_24u32u,
                   ( const Ipp8u* pSrc, Ipp32u* pDst, int len ))
IPPAPI( IppStatus, ippsConvert_32u24u_Sfs,
                   ( const Ipp32u* pSrc, Ipp8u* pDst, int len,
                       int scaleFactor ))
IPPAPI( IppStatus, ippsConvert_24u32f,
                   ( const Ipp8u* pSrc, Ipp32f* pDst, int len ))
IPPAPI( IppStatus, ippsConvert_32f24u_Sfs,
                   ( const Ipp32f* pSrc, Ipp8u* pDst, int len,
                       int scaleFactor ))
IPPAPI( IppStatus, ippsConvert_24s32s,
                   ( const Ipp8u* pSrc, Ipp32s* pDst, int len ))
IPPAPI( IppStatus, ippsConvert_32s24s_Sfs,
                   ( const Ipp32s* pSrc, Ipp8u* pDst, int len,
                       int scaleFactor ))
IPPAPI( IppStatus, ippsConvert_24s32f,
                   ( const Ipp8u* pSrc, Ipp32f* pDst, int len ))
IPPAPI( IppStatus, ippsConvert_32f24s_Sfs,
                   ( const Ipp32f* pSrc, Ipp8u* pDst, int len,
                       int scaleFactor ))


#if !defined( _OWN_BLDPCS )
typedef Ipp16s Ipp16f;
#endif

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConvert_16s16f
//  Purpose:    Converts integer data to floating point data
//  Parameters:
//    pSrc        pointer to integer data to be converted
//    pDst        pointer to the destination vector
//    len         length of the vectors
//    rndMode      Rounding mode which can be ippRndZero or ippRndNear
//  Return:
//    ippStsNullPtrErr    pointer(s) to the data is NULL
//    ippStsSizeErr       length of the vectors is less or equal zero
//    ippStsNoErr         otherwise
*/

IPPAPI(IppStatus,ippsConvert_16s16f,(const Ipp16s* pSrc,Ipp16f* pDst,int len,IppRoundMode rndMode))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConvert_16f16s_Sfs
//  Purpose:    convert floating point data to integer data
//  Parameters:
//    pSrc         pointer to the input floating point data to be converted
//    pDst         pointer to destination vector
//    len          length of the vectors
//    rndMode      Rounding mode which can be ippRndZero or ippRndNear
//    scaleFactor  scale factor value
//  Return:
//    ippStsNullPtrErr    pointer(s) to the data NULL
//    ippStsSizeErr       length of the vectors is less or equal zero
//    ippStsNoErr         otherwise
//  Note:
//    an out-of-range result will be saturated
*/
IPPAPI(IppStatus,ippsConvert_16f16s_Sfs,(const Ipp16f* pSrc,Ipp16s* pDst,int len,IppRoundMode rndMode,int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConvert_32f16f
//  Purpose:    Converts floating point data Ipp32f
//              to floating point data Ipp16f
//  Parameters:
//    pSrc          pointer to the input vector
//    pDst          pointer to the output vector
//    len           length of the vectors
//    rndMode       Rounding mode which can be ippRndZero or ippRndNear
//  Return:
//    ippStsNullPtrErr    pointer(s) to the data is NULL
//    ippStsSizeErr       length of the vectors is less or equal zero
//    ippStsNoErr         otherwise
*/
IPPAPI(IppStatus,ippsConvert_32f16f,(const Ipp32f* pSrc,Ipp16f* pDst,int len,IppRoundMode rndMode))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConvert_16f32f
//  Purpose:    Converts floating point data Ipp16f
//              to floating point data Ipp32f
//  Parameters:
//    pSrc          pointer to the input vector
//    pDst          pointer to the output vector
//    len           length of the vectors
Return:
//    ippStsNullPtrErr    pointer(s) to the data is NULL
//    ippStsSizeErr       length of the vectors is less or equal zero
//    ippStsNoErr         otherwise
*/
IPPAPI(IppStatus,ippsConvert_16f32f,(const Ipp16f* pSrc,Ipp32f* pDst,int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConvert
//  Purpose:    convert integer data to integer data
//  Parameters:
//    pSrc         pointer to the input integer data to be converted
//    pDst         pointer to destination vector
//    len          length of the vectors
//    rndMode      Rounding mode which can be ippRndZero or ippRndNear
//    scaleFactor  scale factor value
//  Return:
//    ippStsNullPtrErr    pointer(s) to the data NULL
//    ippStsSizeErr       length of the vectors is less or equal zero
//    ippStsNoErr         otherwise
//  Note:
//    an out-of-range result will be saturated
*/

IPPAPI(IppStatus,ippsConvert_64s32s_Sfs,(const Ipp64s* pSrc, Ipp32s* pDst,
       int len, IppRoundMode rndMode, int scaleFactor))



/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsThreshold
//  Purpose:    execute threshold operation on every element of the vector
//  Parameters:
//    level      level of the threshold operation
//    pSrcDst    pointer to the vector for in-place operation
//    pSrc       pointer to the input vector
//    pDst       pointer to the output vector
//    len        length of the vectors
//    relOp      comparison mode, cmpLess or cmpGreater
//  Return:
//    ippStsNullPtrErr          pointer(s) to the data is NULL
//    ippStsSizeErr             length of the vectors is less or equal zero
//    ippStsThreshNegLevelErr   negative level value in complex operation
//    ippStsBadArgErr           relOp is no cmpLess and no cmpGreater
//    ippStsNoErr               otherwise
//  Notes:
//  real data
//    cmpLess    : pDst[n] = pSrc[n] < level ? level : pSrc[n];
//    cmpGreater : pDst[n] = pSrc[n] > level ? level : pSrc[n];
//  complex data
//    cmpLess    : pDst[n] = abs(pSrc[n]) < level ? pSrc[n]*k : pSrc[n];
//    cmpGreater : pDst[n] = abs(pSrc[n]) > level ? pSrc[n]*k : pSrc[n];
//    where k = level / abs(pSrc[n]);
*/

IPPAPI(IppStatus,ippsThreshold_32f_I,( Ipp32f* pSrcDst, int len,
       Ipp32f level, IppCmpOp relOp ))
IPPAPI(IppStatus,ippsThreshold_32fc_I,( Ipp32fc* pSrcDst, int len,
       Ipp32f level, IppCmpOp relOp ))
IPPAPI(IppStatus,ippsThreshold_64f_I,( Ipp64f* pSrcDst, int len,
       Ipp64f level, IppCmpOp relOp ))
IPPAPI(IppStatus,ippsThreshold_64fc_I,( Ipp64fc* pSrcDst, int len,
       Ipp64f level, IppCmpOp relOp ))
IPPAPI(IppStatus,ippsThreshold_16s_I,( Ipp16s* pSrcDst, int len,
       Ipp16s level, IppCmpOp relOp ))
IPPAPI(IppStatus,ippsThreshold_16sc_I,( Ipp16sc* pSrcDst, int len,
       Ipp16s level,  IppCmpOp relOp ))

IPPAPI(IppStatus,ippsThreshold_32f,( const Ipp32f* pSrc, Ipp32f* pDst,
       int len, Ipp32f level, IppCmpOp relOp ))
IPPAPI(IppStatus,ippsThreshold_32fc,( const Ipp32fc* pSrc, Ipp32fc* pDst,
       int len, Ipp32f level, IppCmpOp relOp ))
IPPAPI(IppStatus,ippsThreshold_64f,( const Ipp64f* pSrc, Ipp64f* pDst,
       int len, Ipp64f level, IppCmpOp relOp ))
IPPAPI(IppStatus,ippsThreshold_64fc,( const Ipp64fc* pSrc, Ipp64fc* pDst,
       int len, Ipp64f level, IppCmpOp relOp ))
IPPAPI(IppStatus,ippsThreshold_16s,( const Ipp16s* pSrc, Ipp16s* pDst,
       int len, Ipp16s level, IppCmpOp relOp ))
IPPAPI(IppStatus,ippsThreshold_16sc,( const Ipp16sc* pSrc, Ipp16sc* pDst,
       int len, Ipp16s level, IppCmpOp relOp))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsThresholdLT
//              ippsThresholdGT
//  Purpose:    execute threshold operation on every element of the vector,
//              "less than" for ippsThresoldLT
//              "greater than for ippsThresholdGT
//  Parameters:
//    pSrcDst    pointer to the vector for in-place operation
//    pSrc       pointer to the input vector
//    pDst       pointer to the output vector
//    len         length of the vectors
//    level      level of the threshold operation
//  Return:
//    ippStsNullPtrErr          pointer(s) to the data is NULL
//    ippStsSizeErr             length of the vectors is less or equal zero
//    ippStsThreshNegLevelErr   negative level value in complex operation
//    ippStsNoErr               otherwise
*/
IPPAPI(IppStatus,ippsThreshold_LT_32f_I,( Ipp32f* pSrcDst, int len,
       Ipp32f level ))
IPPAPI(IppStatus,ippsThreshold_LT_32fc_I,( Ipp32fc* pSrcDst, int len,
       Ipp32f level ))
IPPAPI(IppStatus,ippsThreshold_LT_64f_I,( Ipp64f* pSrcDst, int len,
       Ipp64f level ))
IPPAPI(IppStatus,ippsThreshold_LT_64fc_I,( Ipp64fc* pSrcDst, int len,
       Ipp64f level ))
IPPAPI(IppStatus,ippsThreshold_LT_16s_I,( Ipp16s* pSrcDst, int len,
       Ipp16s level ))
IPPAPI(IppStatus,ippsThreshold_LT_16sc_I,( Ipp16sc* pSrcDst, int len,
       Ipp16s level ))
IPPAPI(IppStatus,ippsThreshold_LT_32f,( const Ipp32f* pSrc, Ipp32f* pDst,
       int len, Ipp32f level ))
IPPAPI(IppStatus,ippsThreshold_LT_32fc,( const Ipp32fc* pSrc, Ipp32fc* pDst,
       int len, Ipp32f level ))
IPPAPI(IppStatus,ippsThreshold_LT_64f,( const Ipp64f* pSrc, Ipp64f* pDst,
       int len, Ipp64f level ))
IPPAPI(IppStatus,ippsThreshold_LT_64fc,( const Ipp64fc* pSrc, Ipp64fc* pDst,
       int len, Ipp64f level ))
IPPAPI(IppStatus,ippsThreshold_LT_16s,( const Ipp16s* pSrc, Ipp16s* pDst,
       int len, Ipp16s level ))
IPPAPI(IppStatus,ippsThreshold_LT_16sc,( const Ipp16sc* pSrc, Ipp16sc* pDst,
       int len, Ipp16s level ))

IPPAPI(IppStatus,ippsThreshold_LT_32s_I,(Ipp32s* pSrcDst,int len,Ipp32s level))
IPPAPI(IppStatus,ippsThreshold_LT_32s,(const Ipp32s* pSrc,Ipp32s* pDst,int len,Ipp32s level))

IPPAPI(IppStatus,ippsThreshold_GT_32f_I,( Ipp32f* pSrcDst, int len,
       Ipp32f level ))
IPPAPI(IppStatus,ippsThreshold_GT_32fc_I,( Ipp32fc* pSrcDst, int len,
       Ipp32f level ))
IPPAPI(IppStatus,ippsThreshold_GT_64f_I,( Ipp64f* pSrcDst, int len,
       Ipp64f level ))
IPPAPI(IppStatus,ippsThreshold_GT_64fc_I,( Ipp64fc* pSrcDst, int len,
       Ipp64f level ))
IPPAPI(IppStatus,ippsThreshold_GT_16s_I,( Ipp16s* pSrcDst, int len,
       Ipp16s level ))
IPPAPI(IppStatus,ippsThreshold_GT_16sc_I,( Ipp16sc* pSrcDst, int len,
       Ipp16s level ))
IPPAPI(IppStatus,ippsThreshold_GT_32f,( const Ipp32f* pSrc, Ipp32f* pDst,
       int len, Ipp32f level ))
IPPAPI(IppStatus,ippsThreshold_GT_32fc,( const Ipp32fc* pSrc, Ipp32fc* pDst,
       int len, Ipp32f level ))
IPPAPI(IppStatus,ippsThreshold_GT_64f,( const Ipp64f* pSrc, Ipp64f* pDst,
       int len, Ipp64f level ))
IPPAPI(IppStatus,ippsThreshold_GT_64fc,( const Ipp64fc* pSrc, Ipp64fc* pDst,
       int len, Ipp64f level ))
IPPAPI(IppStatus,ippsThreshold_GT_16s,( const Ipp16s* pSrc, Ipp16s* pDst,
       int len, Ipp16s level ))
IPPAPI(IppStatus,ippsThreshold_GT_16sc,( const Ipp16sc* pSrc, Ipp16sc* pDst,
       int len, Ipp16s level ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsThreshold_LTAbs
//              ippsThreshold_GTAbs
//  Purpose:    execute threshold by abolute value operation on every element
//              of the vector
//              "less than" for ippsThresold_LTAbs
//              "greater than for ippsThreshold_GTAbs
//  Parameters:
//    pSrcDst    pointer to the vector for in-place operation
//    pSrc       pointer to the input vector
//    pDst       pointer to the output vector
//    len         length of the vectors
//    level      level of the threshold operation
//  Return:
//    ippStsNullPtrErr          pointer(s) to the data is NULL
//    ippStsSizeErr             length of the vectors is less or equal zero
//    ippStsThreshNegLevelErr   negative level value in complex operation
//    ippStsNoErr               otherwise
*/
IPPAPI(IppStatus,ippsThreshold_LTAbs_32f,(const Ipp32f* pSrc, Ipp32f *pDst,
       int len, Ipp32f level))
IPPAPI(IppStatus,ippsThreshold_LTAbs_64f,(const Ipp64f* pSrc, Ipp64f *pDst,
       int len, Ipp64f level))
IPPAPI(IppStatus,ippsThreshold_LTAbs_16s,(const Ipp16s* pSrc, Ipp16s *pDst,
       int len, Ipp16s level))
IPPAPI(IppStatus,ippsThreshold_LTAbs_32s,(const Ipp32s* pSrc, Ipp32s *pDst,
       int len, Ipp32s level))
IPPAPI(IppStatus,ippsThreshold_LTAbs_32f_I,(Ipp32f *pSrcDst,
       int len, Ipp32f level))
IPPAPI(IppStatus,ippsThreshold_LTAbs_64f_I,(Ipp64f *pSrcDst,
       int len, Ipp64f level))
IPPAPI(IppStatus,ippsThreshold_LTAbs_16s_I,(Ipp16s *pSrcDst,
       int len, Ipp16s level))
IPPAPI(IppStatus,ippsThreshold_LTAbs_32s_I,(Ipp32s *pSrcDst,
       int len, Ipp32s level))
IPPAPI(IppStatus,ippsThreshold_GTAbs_32f,(const Ipp32f* pSrc, Ipp32f *pDst,
       int len, Ipp32f level))
IPPAPI(IppStatus,ippsThreshold_GTAbs_64f,(const Ipp64f* pSrc, Ipp64f *pDst,
       int len, Ipp64f level))
IPPAPI(IppStatus,ippsThreshold_GTAbs_16s,(const Ipp16s* pSrc, Ipp16s *pDst,
       int len, Ipp16s level))
IPPAPI(IppStatus,ippsThreshold_GTAbs_32s,(const Ipp32s* pSrc, Ipp32s *pDst,
       int len, Ipp32s level))
IPPAPI(IppStatus,ippsThreshold_GTAbs_32f_I,(Ipp32f *pSrcDst,
       int len, Ipp32f level))
IPPAPI(IppStatus,ippsThreshold_GTAbs_64f_I,(Ipp64f *pSrcDst,
       int len, Ipp64f level))
IPPAPI(IppStatus,ippsThreshold_GTAbs_16s_I,(Ipp16s *pSrcDst,
       int len, Ipp16s level))
IPPAPI(IppStatus,ippsThreshold_GTAbs_32s_I,(Ipp32s *pSrcDst,
       int len, Ipp32s level))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsThresholdLTValue
//              ippsThresholdGTValue
//  Purpose:    execute threshold operation on every element of the vector with
//              replace on value,
//              "less than" for ippsThresoldLTValue
//              "greater than for ippsThresholdGTValue
//  Parameters:
//    pSrcDst    pointer to the vector for in-place operation
//    pSrc       pointer to the input vector
//    pDst       pointer to the output vector
//    len         length of the vectors
//    level      level of the threshold operation
//    value      value of replace
//  Return:
//    ippStsNullPtrErr          pointer(s) to the data is NULL
//    ippStsSizeErr             length of the vectors is less or equal zero
//    ippStsThreshNegLevelErr   negative level value in complex operation
//    ippStsNoErr               otherwise
*/
IPPAPI(IppStatus,ippsThreshold_LTVal_32f_I,( Ipp32f* pSrcDst, int len,
       Ipp32f level, Ipp32f value ))
IPPAPI(IppStatus,ippsThreshold_LTVal_32fc_I,( Ipp32fc* pSrcDst, int len,
       Ipp32f level, Ipp32fc value ))
IPPAPI(IppStatus,ippsThreshold_LTVal_64f_I,( Ipp64f* pSrcDst, int len,
       Ipp64f level, Ipp64f value ))
IPPAPI(IppStatus,ippsThreshold_LTVal_64fc_I,( Ipp64fc* pSrcDst, int len,
       Ipp64f level, Ipp64fc value ))
IPPAPI(IppStatus,ippsThreshold_LTVal_16s_I,( Ipp16s* pSrcDst, int len,
       Ipp16s level, Ipp16s value ))
IPPAPI(IppStatus,ippsThreshold_LTVal_16sc_I,( Ipp16sc* pSrcDst, int len,
       Ipp16s level, Ipp16sc value ))
IPPAPI(IppStatus,ippsThreshold_LTVal_32f,( const Ipp32f* pSrc, Ipp32f* pDst,
       int len, Ipp32f level, Ipp32f value ))
IPPAPI(IppStatus,ippsThreshold_LTVal_32fc,( const Ipp32fc* pSrc, Ipp32fc* pDst,
       int len, Ipp32f level, Ipp32fc value ))
IPPAPI(IppStatus,ippsThreshold_LTVal_64f,( const Ipp64f* pSrc, Ipp64f* pDst,
       int len, Ipp64f level, Ipp64f value ))
IPPAPI(IppStatus,ippsThreshold_LTVal_64fc,( const Ipp64fc* pSrc, Ipp64fc* pDst,
       int len, Ipp64f level, Ipp64fc value ))
IPPAPI(IppStatus,ippsThreshold_LTVal_16s,( const Ipp16s* pSrc, Ipp16s* pDst,
       int len, Ipp16s level, Ipp16s value ))
IPPAPI(IppStatus,ippsThreshold_LTVal_16sc,( const Ipp16sc* pSrc, Ipp16sc* pDst,
       int len, Ipp16s level, Ipp16sc value ))
IPPAPI(IppStatus,ippsThreshold_GTVal_32f_I,( Ipp32f* pSrcDst, int len,
       Ipp32f level, Ipp32f value ))
IPPAPI(IppStatus,ippsThreshold_GTVal_32fc_I,( Ipp32fc* pSrcDst, int len,
       Ipp32f level, Ipp32fc value ))
IPPAPI(IppStatus,ippsThreshold_GTVal_64f_I,( Ipp64f* pSrcDst, int len,
       Ipp64f level, Ipp64f value ))
IPPAPI(IppStatus,ippsThreshold_GTVal_64fc_I,( Ipp64fc* pSrcDst, int len,
       Ipp64f level, Ipp64fc value ))
IPPAPI(IppStatus,ippsThreshold_GTVal_16s_I,( Ipp16s* pSrcDst, int len,
       Ipp16s level, Ipp16s value ))
IPPAPI(IppStatus,ippsThreshold_GTVal_16sc_I,( Ipp16sc* pSrcDst, int len,
       Ipp16s level, Ipp16sc value ))
IPPAPI(IppStatus,ippsThreshold_GTVal_32f,( const Ipp32f* pSrc, Ipp32f* pDst,
       int len, Ipp32f level, Ipp32f value ))
IPPAPI(IppStatus,ippsThreshold_GTVal_32fc,( const Ipp32fc* pSrc, Ipp32fc* pDst,
       int len, Ipp32f level, Ipp32fc value ))
IPPAPI(IppStatus,ippsThreshold_GTVal_64f,( const Ipp64f* pSrc, Ipp64f* pDst,
       int len, Ipp64f level, Ipp64f value ))
IPPAPI(IppStatus,ippsThreshold_GTVal_64fc,( const Ipp64fc* pSrc, Ipp64fc* pDst,
       int len, Ipp64f level, Ipp64fc value ))
IPPAPI(IppStatus,ippsThreshold_GTVal_16s,( const Ipp16s* pSrc, Ipp16s* pDst,
       int len, Ipp16s level, Ipp16s value ))
IPPAPI(IppStatus,ippsThreshold_GTVal_16sc,( const Ipp16sc* pSrc, Ipp16sc* pDst,
       int len, Ipp16s level, Ipp16sc value ))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsThresholdLTInv
//
//  Purpose:    replace elements of vector values by their inversion after
//              threshold operation
//  Parameters:
//    level      level of threshold operation
//    pSrcDst    pointer to the vector in in-place operation
//    pSrc       pointer to the source vector
//    pDst       pointer to the destination vector
//    len        length of the vectors
//  Return:
//    ippStsNullPtrErr              pointer(s) to the data is NULL
//    ippStsSizeErr                 length of the vector is less or equal zero
//    ippStsThreshNegLevelErr       negative level value
//    ippStsInvZero                 level value and source element value are zero
//    ippStsNoErr                   otherwise
*/

IPPAPI(IppStatus,ippsThreshold_LTInv_32f_I,(Ipp32f* pSrcDst,int len,Ipp32f level))
IPPAPI(IppStatus,ippsThreshold_LTInv_32fc_I,(Ipp32fc* pSrcDst,int len,Ipp32f level))
IPPAPI(IppStatus,ippsThreshold_LTInv_64f_I,(Ipp64f* pSrcDst,int len,Ipp64f level))
IPPAPI(IppStatus,ippsThreshold_LTInv_64fc_I,(Ipp64fc* pSrcDst,int len,Ipp64f level))

IPPAPI(IppStatus,ippsThreshold_LTInv_32f,(const Ipp32f* pSrc,Ipp32f* pDst,int len,Ipp32f level))
IPPAPI(IppStatus,ippsThreshold_LTInv_32fc,(const Ipp32fc* pSrc,Ipp32fc* pDst,int len,Ipp32f level))
IPPAPI(IppStatus,ippsThreshold_LTInv_64f,(const Ipp64f* pSrc,Ipp64f* pDst,int len,Ipp64f level))
IPPAPI(IppStatus,ippsThreshold_LTInv_64fc,(const Ipp64fc* pSrc,Ipp64fc* pDst,int len,Ipp64f level))

/* ///////////////////////////////////////////////////////////////////////////// */


IPPAPI(IppStatus,ippsThreshold_LTValGTVal_32f_I,( Ipp32f* pSrcDst, int len,
       Ipp32f levelLT, Ipp32f valueLT, Ipp32f levelGT, Ipp32f valueGT ))
IPPAPI(IppStatus,ippsThreshold_LTValGTVal_64f_I,( Ipp64f* pSrcDst, int len,
       Ipp64f levelLT, Ipp64f valueLT, Ipp64f levelGT, Ipp64f valueGT ))
IPPAPI(IppStatus,ippsThreshold_LTValGTVal_32f,( const Ipp32f* pSrc,
       Ipp32f* pDst, int len, Ipp32f levelLT, Ipp32f valueLT, Ipp32f levelGT,
       Ipp32f valueGT ))
IPPAPI(IppStatus,ippsThreshold_LTValGTVal_64f,( const Ipp64f* pSrc,
       Ipp64f* pDst, int len, Ipp64f levelLT, Ipp64f valueLT, Ipp64f levelGT,
       Ipp64f valueGT ))

IPPAPI(IppStatus,ippsThreshold_LTValGTVal_16s_I,( Ipp16s* pSrcDst, int len,
       Ipp16s levelLT, Ipp16s valueLT, Ipp16s levelGT, Ipp16s valueGT ))
IPPAPI(IppStatus,ippsThreshold_LTValGTVal_16s,( const Ipp16s* pSrc,
       Ipp16s* pDst, int len, Ipp16s levelLT, Ipp16s valueLT, Ipp16s levelGT,
       Ipp16s valueGT ))



IPPAPI(IppStatus,ippsThreshold_GT_32s_I,(Ipp32s* pSrcDst,int len,Ipp32s level))

IPPAPI(IppStatus,ippsThreshold_GT_32s,(const Ipp32s* pSrc,Ipp32s* pDst,int len,Ipp32s level))

IPPAPI(IppStatus,ippsThreshold_LTValGTVal_32s_I,( Ipp32s* pSrcDst, int len,
       Ipp32s levelLT, Ipp32s valueLT, Ipp32s levelGT, Ipp32s valueGT ))

IPPAPI(IppStatus,ippsThreshold_LTValGTVal_32s,( const Ipp32s* pSrc,
       Ipp32s* pDst, int len, Ipp32s levelLT, Ipp32s valueLT, Ipp32s levelGT, Ipp32s valueGT ))





/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsCartToPolar
//
//  Purpose:    Convert cartesian coordinate to polar. Input data are formed as
//              a complex vector.
//
//  Parameters:
//   pSrc          an input complex vector
//   pDstMagn      an output vector to store the magnitude components
//   pDstPhase     an output vector to store the phase components (in radians)
//   len           a length of the array
//  Return:
//   ippStsNoErr           Ok
//   ippStsNullPtrErr      Some of pointers to input or output data are NULL
//   ippStsSizeErr         The length of the arrays is less or equal zero
//
*/

IPPAPI(IppStatus, ippsCartToPolar_32fc,(const Ipp32fc* pSrc, Ipp32f* pDstMagn,
                                                   Ipp32f* pDstPhase, int len))
IPPAPI(IppStatus, ippsCartToPolar_64fc,(const Ipp64fc* pSrc, Ipp64f* pDstMagn,
                                                   Ipp64f* pDstPhase, int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsCartToPolar
//
//  Purpose:    Convert cartesian coordinate to polar. Input data are formed as
//              two real vectors.
//
//  Parameters:
//   pSrcRe       an input vector containing the coordinates X
//   pSrcIm       an input vector containing the coordinates Y
//   pDstMagn     an output vector to store the magnitude components
//   pDstPhase    an output vector to store the phase components (in radians)
//   len          a length of the array
//  Return:
//   ippStsNoErr           Ok
//   ippStsNullPtrErr      Some of pointers to input or output data are NULL
//   ippStsSizeErr         The length of the arrays is less or equal zero
//
*/

IPPAPI(IppStatus, ippsCartToPolar_32f,(const Ipp32f* pSrcRe, const Ipp32f*
                        pSrcIm, Ipp32f* pDstMagn, Ipp32f* pDstPhase, int len))
IPPAPI(IppStatus, ippsCartToPolar_64f,(const Ipp64f* pSrcRe, const Ipp64f*
                        pSrcIm, Ipp64f* pDstMagn, Ipp64f* pDstPhase, int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsPolarToCart
//
//  Purpose:    Convert polar coordinate to cartesian. Output data are formed as
//              a complex vector.
//
//  Parameters:
//   pDstMagn      an input vector containing the magnitude components
//   pDstPhase     an input vector containing the phase components(in radians)
//   pDst          an output complex vector to store the cartesian coordinates
//   len           a length of the arrays
//  Return:
//   ippStsNoErr           Ok
//   ippStsNullPtrErr      Some of pointers to input or output data are NULL
//   ippStsSizeErr         The length of the arrays is less or equal zero
//
*/

IPPAPI(IppStatus, ippsPolarToCart_32fc,(const Ipp32f* pSrcMagn,
                  const Ipp32f* pSrcPhase, Ipp32fc* pDst, int len))
IPPAPI(IppStatus, ippsPolarToCart_64fc,(const Ipp64f* pSrcMagn,
                  const Ipp64f* pSrcPhase, Ipp64fc* pDst, int len))

IPPAPI(IppStatus, ippsPolarToCart_32sc,(const Ipp32s* pSrcMagn,
                  const Ipp32s* pSrcPhase, int phaseFixedPoint, Ipp32sc* pDst, int len))

IPPAPI(IppStatus, ippsPolarToCart_16sc,(const Ipp16s* pSrcMagn,
                  const Ipp16s* pSrcPhase, int phaseFixedPoint, Ipp16sc* pDst, int len))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsPolarToCart
//
//  Purpose:    Convert polar coordinate to cartesian. Output data are formed as
//              two real vectors.
//
//  Parameters:
//   pDstMagn     an input vector containing the magnitude components
//   pDstPhase    an input vector containing the phase components(in radians)
//   pSrcRe       an output complex vector to store the coordinates X
//   pSrcIm       an output complex vector to store the coordinates Y
//   len          a length of the arrays
//  Return:
//   ippStsNoErr           Ok
//   ippStsNullPtrErr      Some of pointers to input or output data are NULL
//   ippStsSizeErr         The length of the arrays is less or equal zero
//
*/

IPPAPI(IppStatus, ippsPolarToCart_32f,(const Ipp32f* pSrcMagn,
                  const Ipp32f* pSrcPhase, Ipp32f* pDstRe, Ipp32f* pDstIm, int len))
IPPAPI(IppStatus, ippsPolarToCart_64f,(const Ipp64f* pSrcMagn,
                  const Ipp64f* pSrcPhase, Ipp64f* pDstRe, Ipp64f* pDstIm, int len))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsCartToPolar
//
//  Purpose:    Convert cartesian coordinate to polar. Input data are formed as
//              a complex vector.
//
//  Parameters:
//   pSrc              an input complex vector
//   pDstMagn          an output vector to store the magnitude components
//   pDstPhase         an output vector to store the phase components (in radians)
//   len               a length of the array
//   magnScaleFactor   scale factor of the magnitude companents
//   phaseScaleFactor  scale factor of the phase companents
//  Return:
//   ippStsNoErr           Ok
//   ippStsNullPtrErr      Some of pointers to input or output data are NULL
//   ippStsSizeErr         The length of the arrays is less or equal zero
//
*/
IPPAPI(IppStatus, ippsCartToPolar_16sc_Sfs, (const Ipp16sc* pSrc, Ipp16s* pDstMagn, Ipp16s* pDstPhase, int len, int magnScaleFactor, int phaseScaleFactor))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsPolarToCart
//
//  Purpose:    Convert polar coordinate to cartesian. Output data are formed as
//              a complex vector.
//
//  Parameters:
//   pDstMagn          an input vector containing the magnitude components
//   pDstPhase         an input vector containing the phase components(in radians)
//   pDst              an output complex vector to store the cartesian coordinates
//   len               a length of the arrays
//   magnScaleFactor   scale factor of the magnitude companents
//   phaseScaleFactor  scale factor of the phase companents
//  Return:
//   ippStsNoErr           Ok
//   ippStsNullPtrErr      Some of pointers to input or output data are NULL
//   ippStsSizeErr         The length of the arrays is less or equal zero
//
*/
IPPAPI(IppStatus, ippsPolarToCart_16sc_Sfs, (const Ipp16s* pSrcMagn, const Ipp16s* pSrcPhase, Ipp16sc* pDst, int len, int magnScaleFactor, int phaseScaleFactor))


/* /////////////////////////////////////////////////////////////////////////////
//                          Companding functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsALawToLin
//  Purpose:    convert from A-Law to linear PCM value
//  Parameters:
//    pSrc        pointer to the input vector containing A-Law values
//    pDst        pointer to the output vector for store linear PCM values
//    len         length of the vectors, number of items
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           length of the vectors is less or equal zero
//    ippStsNoErr             otherwise
*/
IPPAPI(IppStatus, ippsALawToLin_8u32f, (const Ipp8u* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsALawToLin_8u16s, (const Ipp8u* pSrc, Ipp16s* pDst, int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsMuLawToLin
//  Purpose:    convert from Mu-Law to linear PCM value
//  Parameters:
//    pSrc        pointer to the input vector containing Mu-Law values
//    pDst        pointer to the output vector for store linear PCM values
//    len         length of the vectors, number of items
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           length of the vectors is less or equal zero
//    ippStsNoErr             otherwise
*/
IPPAPI(IppStatus, ippsMuLawToLin_8u32f, (const Ipp8u* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsMuLawToLin_8u16s, (const Ipp8u* pSrc, Ipp16s* pDst, int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsLinToALaw
//  Purpose:    convert from linear PCM to A-Law value
//  Parameters:
//    pSrc        pointer to the input vector containing linear PCM values
//    pDst        pointer to the output vector for store A-Law values
//    len         length of the vectors, number of items
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           length of the vectors is less or equal zero
//    ippStsNoErr             otherwise
*/
IPPAPI(IppStatus, ippsLinToALaw_32f8u, (const Ipp32f* pSrc, Ipp8u* pDst, int len))
IPPAPI(IppStatus, ippsLinToALaw_16s8u, (const Ipp16s* pSrc, Ipp8u* pDst, int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsMuLawToLin
//  Purpose:    convert from linear PCM to Mu-Law value
//  Parameters:
//    pSrc        pointer to the input vector containing linear PCM values
//    pDst        pointer to the output vector for store Mu-Law values
//    len         length of the vectors, number of items
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           length of the vectors is less or equal zero
//    ippStsNoErr             otherwise
*/
IPPAPI(IppStatus, ippsLinToMuLaw_32f8u, (const Ipp32f* pSrc, Ipp8u* pDst, int len))
IPPAPI(IppStatus, ippsLinToMuLaw_16s8u, (const Ipp16s* pSrc, Ipp8u* pDst, int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsALawToMuLaw, ippsMuLawToALaw
//  Purpose:    convert from A-Law to Mu-Law and vice-versa
//  Parameters:
//    pSrc        pointer to the input vector containing A-Law or Mu-Law values
//    pDst        pointer to the output vector for store Mu-Law or A-Law values
//    len         length of the vectors, number of items
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           length of the vectors is less or equal zero
//    ippStsNoErr             otherwise
*/
IPPAPI(IppStatus, ippsALawToMuLaw_8u,  (const Ipp8u* pSrc, Ipp8u* pDst, int len))
IPPAPI(IppStatus, ippsMuLawToALaw_8u,   (const Ipp8u* pSrc, Ipp8u* pDst, int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//    ippsPreemphasize_32f
//  Purpose:
//    Compute the preemphasizes a single precision real signal.
//  Parameters:
//    pSrcDst  pointer to the vector for in-place operation.
//    len      length of  the input vector.
//    val      The multiplier to be used in the preemphasis difference equation
//             y(n) = x(n) - a * x(n-1)  where y  is the preemphasized  output
//             and x is the input. Usually a value  of 0.95  is  used for speech
//             audio  signals.
//  Return:
//    ippStsNoErr         Ok
//    ippStsNullPtrErr    Some of pointers to input or output data are NULL
//    ippStsSizeErr       The length of the arrays is less or equal zero
*/
IPPAPI(IppStatus, ippsPreemphasize_32f,(Ipp32f* pSrcDst, int len, Ipp32f val))
IPPAPI(IppStatus, ippsPreemphasize_16s,(Ipp16s* pSrcDst, int len, Ipp32f val))



/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFlip
//  Purpose:    dst[i] = src[len-i-1], i=0..len-1
//  Parameters:
//    pSrc      pointer to the input vector
//    pDst      pointer to the output vector
//    len       length of the vectors, number of items
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           length of the vectors is less or equal zero
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippsFlip_8u,( const Ipp8u* pSrc, Ipp8u* pDst, int len ))
IPPAPI(IppStatus, ippsFlip_8u_I,( Ipp8u* pSrcDst, int len ))
IPPAPI(IppStatus, ippsFlip_16u,( const Ipp16u* pSrc, Ipp16u* pDst, int len ))
IPPAPI(IppStatus, ippsFlip_16u_I,( Ipp16u* pSrcDst, int len ))
IPPAPI(IppStatus, ippsFlip_32f,( const Ipp32f* pSrc, Ipp32f* pDst, int len ))
IPPAPI(IppStatus, ippsFlip_32f_I,( Ipp32f* pSrcDst, int len ))
IPPAPI(IppStatus, ippsFlip_64f,( const Ipp64f* pSrc, Ipp64f* pDst, int len ))
IPPAPI(IppStatus, ippsFlip_64f_I,( Ipp64f* pSrcDst, int len ))

IPPAPI(IppStatus, ippsFlip_32fc,( const Ipp32fc* pSrc, Ipp32fc* pDst, int len ))
IPPAPI(IppStatus, ippsFlip_32fc_I,( Ipp32fc* pSrcDst, int len ))
IPPAPI(IppStatus, ippsFlip_64fc,( const Ipp64fc* pSrc, Ipp64fc* pDst, int len ))
IPPAPI(IppStatus, ippsFlip_64fc_I,( Ipp64fc* pSrcDst, int len ))


/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippsUpdateLinear_16s32s_I
//  Purpose:    Calc Update Linear value
//  Return:
//   IPP_NO_ERR                 Ok
//   IPP_NULL_PTR_ERR           Pointer to pSrc or pointer to pSrcDst is NULL
//   IPP_BADSIZE_ERR            The length of the array is less or equal zero
//  Parameters:
//   pSrc           pointer to vector
//   len            a length of the array
//   pSrcDst        pointer to input and output
//   srcShiftRight  shiftright of src (0<=srcShiftRight<=15)
//   alpha          weight
//   hint           code specific use hints
//
*/
IPPAPI(IppStatus,ippsUpdateLinear_16s32s_I,(const Ipp16s* pSrc,int len,
       Ipp32s* pSrcDst, int srcShiftRight,Ipp16s alpha, IppHintAlgorithm hint))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippsUpdatePower_16s32s_I
//  Purpose:    Calc Update Power value
//  Return:
//   IPP_NO_ERR                 Ok
//   IPP_NULL_PTR_ERR           Pointer to pSrc or pointer to pSrcDst is NULL
//   IPP_BADSIZE_ERR            The length of the array is less or equal zero
//  Parameters:
//   pSrc           pointer to vector
//   len            a length of the array
//   pSrcDst        pointer to input and output
//   srcShiftRight  shiftright of src (0<=srcShiftRight<=31)
//   alpha          weight
//   hint           code specific use hints
//
*/
IPPAPI(IppStatus,ippsUpdatePower_16s32s_I,(const Ipp16s* pSrc,int len,
       Ipp32s* pSrcDst, int srcShiftRight,Ipp16s alpha, IppHintAlgorithm hint))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsJoin_32f16s_D2L
//  Purpose:    Join of vectors.
//  Parameters:
//      pSrc        pointer to pointers to the input vectors
//      pDst        pointer to the output vector
//      nChannels   number of channels
//      chanlen     length of the channel
//  Return:
//      ippStsNullPtrErr        pointer(s) to the data is NULL
//      ippStsSizeErr           nChannels or chanlen are less or equal zero
//      ippStsNoErr             otherwise
//
*/

IPPAPI( IppStatus, ippsJoin_32f16s_D2L, ( const Ipp32f** pSrc,
                  int nChannels, int chanLen, Ipp16s* pDst ) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsJoinScaled_32f16s_D2L
//              ippsJoinScaled_32f24s_D2L
//
//  Purpose:    Join of vectors.
//  Parameters:
//      pSrc        pointer to pointers to the input vectors
//      pDst        pointer to the output vector
//      nChannels   number of channels
//      chanlen     length of the channel
//  Return:
//      ippStsNullPtrErr        pointer(s) to the data is NULL
//      ippStsSizeErr           nChannels or chanlen are less or equal zero
//      ippStsNoErr             otherwise
//
//      Note: Default region of the src data is [-1.0,1.0].
*/

IPPAPI( IppStatus, ippsJoinScaled_32f16s_D2L,
        ( const Ipp32f** pSrc, int nChannels, int chanLen, Ipp16s* pDst ) )
IPPAPI( IppStatus, ippsJoinScaled_32f24s_D2L,
        ( const Ipp32f** pSrc, int nChannels, int chanLen, Ipp8u* pDst ) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsSplitScaled_16s32f_D2L
//              ippsSplitScaled_24s32f_D2L
//
//  Purpose:    Split of vector.
//  Parameters:
//      pSrc        pointer to the input vector
//      pDst        pointer to pointers to the output vectors
//      nChannels   number of channels
//      chanlen     length of the channel
//  Return:
//      ippStsNullPtrErr        pointer(s) to the data is NULL
//      ippStsSizeErr           nChannels or chanlen are less or equal zero
//      ippStsNoErr             otherwise
//
//      Note: Region of the dst data is [-1.0,1.0].
*/

IPPAPI( IppStatus, ippsSplitScaled_16s32f_D2L,
        ( const Ipp16s* pSrc, Ipp32f** pDst, int nChannels, int chanLen ) )
IPPAPI( IppStatus, ippsSplitScaled_24s32f_D2L,
        ( const Ipp8u* pSrc, Ipp32f** pDst, int nChannels, int chanLen ) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsSwapBytes
//  Purpose:    switches from a "big endian" order to the "little endian" order and vice-versa
//  Parameters:
//    pSrc                 pointer to the source vector
//    pSrcDst              pointer to the source/destination vector
//    pDst                 pointer to the destination vector
//    len                  length of the vectors
//  Return:
//    ippStsNullPtrErr     pointer to the vector is NULL
//    ippStsSizeErr        length of the vectors is less or equal zero
//    ippStsNoErr          otherwise
*/

IPPAPI(IppStatus, ippsSwapBytes_16u_I, ( Ipp16u* pSrcDst, int len ))
IPPAPI(IppStatus, ippsSwapBytes_24u_I, ( Ipp8u*  pSrcDst, int len ))
IPPAPI(IppStatus, ippsSwapBytes_32u_I, ( Ipp32u* pSrcDst, int len ))
IPPAPI(IppStatus, ippsSwapBytes_64u_I, ( Ipp64u* pSrcDst, int len ))

IPPAPI(IppStatus, ippsSwapBytes_16u,   ( const Ipp16u* pSrc, Ipp16u* pDst, int len ))
IPPAPI(IppStatus, ippsSwapBytes_24u,   ( const Ipp8u*  pSrc, Ipp8u*  pDst, int len ))
IPPAPI(IppStatus, ippsSwapBytes_32u,   ( const Ipp32u* pSrc, Ipp32u* pDst, int len ))
IPPAPI(IppStatus, ippsSwapBytes_64u,   ( const Ipp64u* pSrc, Ipp64u* pDst, int len ))

/* /////////////////////////////////////////////////////////////////////////////
//                  Arithmetic functions
///////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////////
//  Names:       ippsAdd, ippsSub, ippsMul
//
//  Purpose:    add, subtract and multiply operations upon every element of
//              the source vector
//  Arguments:
//    pSrc                 pointer to the source vector
//    pSrcDst              pointer to the source/destination vector
//    pSrc1                pointer to the first source vector
//    pSrc2                pointer to the second source vector
//    pDst                 pointer to the destination vector
//    len                  length of the vectors
//    scaleFactor          scale factor value
//  Return:
//    ippStsNullPtrErr     pointer(s) to the data is NULL
//    ippStsSizeErr        length of the vectors is less or equal zero
//    ippStsNoErr          otherwise
//  Note:
//    AddC(X,v,Y)    :  Y[n] = X[n] + v
//    MulC(X,v,Y)    :  Y[n] = X[n] * v
//    SubC(X,v,Y)    :  Y[n] = X[n] - v
//    SubCRev(X,v,Y) :  Y[n] = v - X[n]
//    Sub(X,Y)       :  Y[n] = Y[n] - X[n]
//    Sub(X,Y,Z)     :  Z[n] = Y[n] - X[n]
*/

IPPAPI(IppStatus, ippsAddC_16s_I,     (Ipp16s  val, Ipp16s*  pSrcDst, int len))
IPPAPI(IppStatus, ippsSubC_16s_I,     (Ipp16s  val, Ipp16s*  pSrcDst, int len))
IPPAPI(IppStatus, ippsMulC_16s_I,     (Ipp16s  val, Ipp16s*  pSrcDst, int len))
IPPAPI(IppStatus, ippsAddC_32f_I,     (Ipp32f  val, Ipp32f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsAddC_32fc_I,    (Ipp32fc val, Ipp32fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsSubC_32f_I,     (Ipp32f  val, Ipp32f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsSubC_32fc_I,    (Ipp32fc val, Ipp32fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsSubCRev_32f_I,  (Ipp32f  val, Ipp32f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsSubCRev_32fc_I, (Ipp32fc val, Ipp32fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsMulC_32f_I,     (Ipp32f  val, Ipp32f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsMulC_32fc_I,    (Ipp32fc val, Ipp32fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsAddC_64f_I,     (Ipp64f  val, Ipp64f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsAddC_64fc_I,    (Ipp64fc val, Ipp64fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsSubC_64f_I,     (Ipp64f  val, Ipp64f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsSubC_64fc_I,    (Ipp64fc val, Ipp64fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsSubCRev_64f_I,  (Ipp64f  val, Ipp64f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsSubCRev_64fc_I, (Ipp64fc val, Ipp64fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsMulC_64f_I,     (Ipp64f  val, Ipp64f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsMulC_64fc_I,    (Ipp64fc val, Ipp64fc* pSrcDst, int len))

IPPAPI(IppStatus, ippsMulC_32f16s_Sfs,    (const Ipp32f*  pSrc, Ipp32f  val,
       Ipp16s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMulC_Low_32f16s,    (const Ipp32f*  pSrc, Ipp32f  val,
       Ipp16s*  pDst, int len))


IPPAPI(IppStatus, ippsAddC_8u_ISfs,      (Ipp8u   val, Ipp8u*   pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubC_8u_ISfs,      (Ipp8u   val, Ipp8u*   pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubCRev_8u_ISfs,   (Ipp8u   val, Ipp8u*   pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsMulC_8u_ISfs,      (Ipp8u   val, Ipp8u*   pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsAddC_16s_ISfs,     (Ipp16s  val, Ipp16s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubC_16s_ISfs,     (Ipp16s  val, Ipp16s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsMulC_16s_ISfs,     (Ipp16s  val, Ipp16s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsAddC_16sc_ISfs,    (Ipp16sc val, Ipp16sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubC_16sc_ISfs,    (Ipp16sc val, Ipp16sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsMulC_16sc_ISfs,    (Ipp16sc val, Ipp16sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubCRev_16s_ISfs,  (Ipp16s  val, Ipp16s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubCRev_16sc_ISfs, (Ipp16sc val, Ipp16sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsAddC_32s_ISfs,     (Ipp32s  val, Ipp32s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsAddC_32sc_ISfs,    (Ipp32sc val, Ipp32sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubC_32s_ISfs,     (Ipp32s  val, Ipp32s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubC_32sc_ISfs,    (Ipp32sc val, Ipp32sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubCRev_32s_ISfs,  (Ipp32s  val, Ipp32s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubCRev_32sc_ISfs, (Ipp32sc val, Ipp32sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsMulC_32s_ISfs,     (Ipp32s  val, Ipp32s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsMulC_32sc_ISfs,    (Ipp32sc val, Ipp32sc* pSrcDst,
       int len, int scaleFactor))

IPPAPI(IppStatus, ippsAddC_32f,     (const Ipp32f*  pSrc, Ipp32f  val,
       Ipp32f*  pDst, int len))
IPPAPI(IppStatus, ippsAddC_32fc,    (const Ipp32fc* pSrc, Ipp32fc val,
       Ipp32fc* pDst, int len))
IPPAPI(IppStatus, ippsSubC_32f,     (const Ipp32f*  pSrc, Ipp32f  val,
       Ipp32f*  pDst, int len))
IPPAPI(IppStatus, ippsSubC_32fc,    (const Ipp32fc* pSrc, Ipp32fc val,
       Ipp32fc* pDst, int len))
IPPAPI(IppStatus, ippsSubCRev_32f,  (const Ipp32f*  pSrc, Ipp32f  val,
       Ipp32f*  pDst, int len))
IPPAPI(IppStatus, ippsSubCRev_32fc, (const Ipp32fc* pSrc, Ipp32fc val,
       Ipp32fc* pDst, int len))
IPPAPI(IppStatus, ippsMulC_32f,     (const Ipp32f*  pSrc, Ipp32f  val,
       Ipp32f*  pDst, int len))
IPPAPI(IppStatus, ippsMulC_32fc,    (const Ipp32fc* pSrc, Ipp32fc val,
       Ipp32fc* pDst, int len))
IPPAPI(IppStatus, ippsAddC_64f,     (const Ipp64f*  pSrc, Ipp64f  val,
       Ipp64f*  pDst, int len))
IPPAPI(IppStatus, ippsAddC_64fc,    (const Ipp64fc* pSrc, Ipp64fc val,
       Ipp64fc* pDst, int len))
IPPAPI(IppStatus, ippsSubC_64f,     (const Ipp64f*  pSrc, Ipp64f  val,
       Ipp64f*  pDst, int len))
IPPAPI(IppStatus, ippsSubC_64fc,    (const Ipp64fc* pSrc, Ipp64fc val,
       Ipp64fc* pDst, int len))
IPPAPI(IppStatus, ippsSubCRev_64f,  (const Ipp64f*  pSrc, Ipp64f  val,
       Ipp64f*  pDst, int len))
IPPAPI(IppStatus, ippsSubCRev_64fc, (const Ipp64fc* pSrc, Ipp64fc val,
       Ipp64fc* pDst, int len))
IPPAPI(IppStatus, ippsMulC_64f,     (const Ipp64f*  pSrc, Ipp64f  val,
       Ipp64f*  pDst, int len))
IPPAPI(IppStatus, ippsMulC_64fc,    (const Ipp64fc* pSrc, Ipp64fc val,
       Ipp64fc* pDst, int len))

IPPAPI(IppStatus, ippsAddC_8u_Sfs,     (const Ipp8u*   pSrc, Ipp8u   val,
       Ipp8u*   pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubC_8u_Sfs,     (const Ipp8u*   pSrc, Ipp8u   val,
       Ipp8u*   pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubCRev_8u_Sfs,  (const Ipp8u*   pSrc, Ipp8u   val,
       Ipp8u*   pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMulC_8u_Sfs,     (const Ipp8u*   pSrc, Ipp8u   val,
       Ipp8u*   pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsAddC_16s_Sfs,    (const Ipp16s*  pSrc, Ipp16s  val,
       Ipp16s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsAddC_16sc_Sfs,   (const Ipp16sc* pSrc, Ipp16sc val,
       Ipp16sc* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubC_16s_Sfs,    (const Ipp16s*  pSrc, Ipp16s  val,
       Ipp16s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubC_16sc_Sfs,   (const Ipp16sc* pSrc, Ipp16sc val,
       Ipp16sc* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubCRev_16s_Sfs, (const Ipp16s*  pSrc, Ipp16s  val,
       Ipp16s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubCRev_16sc_Sfs,(const Ipp16sc* pSrc, Ipp16sc val,
       Ipp16sc* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMulC_16s_Sfs,    (const Ipp16s*  pSrc, Ipp16s  val,
       Ipp16s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMulC_16sc_Sfs,   (const Ipp16sc* pSrc, Ipp16sc val,
       Ipp16sc* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsAddC_32s_Sfs,    (const Ipp32s*  pSrc, Ipp32s  val,
       Ipp32s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsAddC_32sc_Sfs,   (const Ipp32sc* pSrc, Ipp32sc val,
       Ipp32sc* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubC_32s_Sfs,    (const Ipp32s*  pSrc, Ipp32s  val,
       Ipp32s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubC_32sc_Sfs,   (const Ipp32sc* pSrc, Ipp32sc val,
       Ipp32sc* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubCRev_32s_Sfs, (const Ipp32s*  pSrc, Ipp32s  val,
       Ipp32s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubCRev_32sc_Sfs,(const Ipp32sc* pSrc, Ipp32sc val,
       Ipp32sc* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMulC_32s_Sfs,    (const Ipp32s*  pSrc, Ipp32s  val,
       Ipp32s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMulC_32sc_Sfs,   (const Ipp32sc* pSrc, Ipp32sc val,
       Ipp32sc* pDst, int len, int scaleFactor))

IPPAPI(IppStatus, ippsAdd_16s_I,  (const Ipp16s*  pSrc,
       Ipp16s*  pSrcDst, int len))
IPPAPI(IppStatus, ippsSub_16s_I,  (const Ipp16s*  pSrc,
       Ipp16s*  pSrcDst, int len))
IPPAPI(IppStatus, ippsMul_16s_I,  (const Ipp16s*  pSrc,
       Ipp16s*  pSrcDst, int len))
IPPAPI(IppStatus, ippsAdd_32f_I,  (const Ipp32f*  pSrc,
       Ipp32f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsAdd_32fc_I, (const Ipp32fc* pSrc,
       Ipp32fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsSub_32f_I,  (const Ipp32f*  pSrc,
       Ipp32f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsSub_32fc_I, (const Ipp32fc* pSrc,
       Ipp32fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsMul_32f_I,  (const Ipp32f*  pSrc,
       Ipp32f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsMul_32fc_I, (const Ipp32fc* pSrc,
       Ipp32fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsAdd_64f_I,  (const Ipp64f*  pSrc,
       Ipp64f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsAdd_64fc_I, (const Ipp64fc* pSrc,
       Ipp64fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsSub_64f_I,  (const Ipp64f*  pSrc,
       Ipp64f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsSub_64fc_I, (const Ipp64fc* pSrc,
       Ipp64fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsMul_64f_I,  (const Ipp64f*  pSrc,
       Ipp64f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsMul_64fc_I, (const Ipp64fc* pSrc,
       Ipp64fc* pSrcDst, int len))


IPPAPI(IppStatus, ippsAdd_8u_ISfs,    (const Ipp8u*   pSrc, Ipp8u*   pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSub_8u_ISfs,    (const Ipp8u*   pSrc, Ipp8u*   pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_8u_ISfs,    (const Ipp8u*   pSrc, Ipp8u*   pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsAdd_16s_ISfs,   (const Ipp16s*  pSrc, Ipp16s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsAdd_16sc_ISfs,  (const Ipp16sc* pSrc, Ipp16sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSub_16s_ISfs,   (const Ipp16s*  pSrc, Ipp16s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSub_16sc_ISfs,  (const Ipp16sc* pSrc, Ipp16sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_16s_ISfs,   (const Ipp16s*  pSrc, Ipp16s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_16sc_ISfs,  (const Ipp16sc* pSrc, Ipp16sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsAdd_32s_ISfs,   (const Ipp32s*  pSrc, Ipp32s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsAdd_32sc_ISfs,  (const Ipp32sc* pSrc, Ipp32sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSub_32s_ISfs,   (const Ipp32s*  pSrc, Ipp32s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsSub_32sc_ISfs,  (const Ipp32sc* pSrc, Ipp32sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_32s_ISfs,   (const Ipp32s*  pSrc, Ipp32s*  pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_32sc_ISfs,  (const Ipp32sc* pSrc, Ipp32sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsAdd_8u16u,  (const Ipp8u*   pSrc1, const Ipp8u*   pSrc2,
       Ipp16u*  pDst, int len))
IPPAPI(IppStatus, ippsMul_8u16u,  (const Ipp8u*   pSrc1, const Ipp8u*   pSrc2,
       Ipp16u*  pDst, int len))
IPPAPI(IppStatus, ippsAdd_16s,    (const Ipp16s*  pSrc1, const Ipp16s*  pSrc2,
       Ipp16s*  pDst, int len))
IPPAPI(IppStatus, ippsSub_16s,    (const Ipp16s*  pSrc1, const Ipp16s*  pSrc2,
       Ipp16s*  pDst, int len))
IPPAPI(IppStatus, ippsMul_16s,    (const Ipp16s*  pSrc1, const Ipp16s*  pSrc2,
       Ipp16s*  pDst, int len))
IPPAPI(IppStatus, ippsAdd_16u,    (const Ipp16u*  pSrc1, const Ipp16u*  pSrc2,
       Ipp16u*  pDst, int len))
IPPAPI(IppStatus, ippsAdd_32u,    (const Ipp32u*  pSrc1, const Ipp32u*  pSrc2,
       Ipp32u*  pDst, int len))
IPPAPI(IppStatus, ippsAdd_16s32f, (const Ipp16s*  pSrc1, const Ipp16s*  pSrc2,
       Ipp32f*  pDst, int len))
IPPAPI(IppStatus, ippsSub_16s32f, (const Ipp16s*  pSrc1, const Ipp16s*  pSrc2,
       Ipp32f*  pDst, int len))
IPPAPI(IppStatus, ippsMul_16s32f, (const Ipp16s*  pSrc1, const Ipp16s*  pSrc2,
       Ipp32f*  pDst, int len))
IPPAPI(IppStatus, ippsAdd_32f,    (const Ipp32f*  pSrc1, const Ipp32f*  pSrc2,
       Ipp32f*  pDst, int len))
IPPAPI(IppStatus, ippsAdd_32fc,   (const Ipp32fc* pSrc1, const Ipp32fc* pSrc2,
       Ipp32fc* pDst, int len))
IPPAPI(IppStatus, ippsSub_32f,    (const Ipp32f*  pSrc1, const Ipp32f*  pSrc2,
       Ipp32f*  pDst, int len))
IPPAPI(IppStatus, ippsSub_32fc,   (const Ipp32fc* pSrc1, const Ipp32fc* pSrc2,
       Ipp32fc* pDst, int len))
IPPAPI(IppStatus, ippsMul_32f,    (const Ipp32f*  pSrc1, const Ipp32f*  pSrc2,
       Ipp32f*  pDst, int len))
IPPAPI(IppStatus, ippsMul_32fc,   (const Ipp32fc* pSrc1, const Ipp32fc* pSrc2,
       Ipp32fc* pDst, int len))
IPPAPI(IppStatus, ippsAdd_64f,    (const Ipp64f*  pSrc1, const Ipp64f*  pSrc2,
       Ipp64f*  pDst, int len))
IPPAPI(IppStatus, ippsAdd_64fc,   (const Ipp64fc* pSrc1, const Ipp64fc* pSrc2,
       Ipp64fc* pDst, int len))
IPPAPI(IppStatus, ippsSub_64f,    (const Ipp64f*  pSrc1, const Ipp64f*  pSrc2,
       Ipp64f*  pDst, int len))
IPPAPI(IppStatus, ippsSub_64fc,   (const Ipp64fc* pSrc1, const Ipp64fc* pSrc2,
       Ipp64fc* pDst, int len))
IPPAPI(IppStatus, ippsMul_64f,    (const Ipp64f*  pSrc1, const Ipp64f*  pSrc2,
       Ipp64f*  pDst, int len))
IPPAPI(IppStatus, ippsMul_64fc,   (const Ipp64fc* pSrc1, const Ipp64fc* pSrc2,
       Ipp64fc* pDst, int len))

IPPAPI(IppStatus, ippsAdd_8u_Sfs,     (const Ipp8u*   pSrc1, const Ipp8u*   pSrc2,
       Ipp8u*   pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSub_8u_Sfs,     (const Ipp8u*   pSrc1, const Ipp8u*   pSrc2,
       Ipp8u*   pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_8u_Sfs,     (const Ipp8u*   pSrc1, const Ipp8u*   pSrc2,
       Ipp8u*   pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsAdd_16s_Sfs,    (const Ipp16s*  pSrc1, const Ipp16s*  pSrc2,
       Ipp16s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsAdd_16sc_Sfs,   (const Ipp16sc* pSrc1, const Ipp16sc* pSrc2,
       Ipp16sc* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSub_16s_Sfs,    (const Ipp16s*  pSrc1, const Ipp16s*  pSrc2,
       Ipp16s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSub_16sc_Sfs,   (const Ipp16sc* pSrc1, const Ipp16sc* pSrc2,
       Ipp16sc* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_16s_Sfs,    (const Ipp16s*  pSrc1, const Ipp16s*  pSrc2,
       Ipp16s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_16sc_Sfs,   (const Ipp16sc* pSrc1, const Ipp16sc* pSrc2,
       Ipp16sc* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_16s32s_Sfs, (const Ipp16s*  pSrc1, const Ipp16s*  pSrc2,
       Ipp32s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsAdd_32s_Sfs,    (const Ipp32s*  pSrc1, const Ipp32s*  pSrc2,
       Ipp32s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsAdd_64s_Sfs,    (const Ipp64s*  pSrc1, const Ipp64s*  pSrc2,
       Ipp64s*  pDst, Ipp32u len, int scaleFactor))
IPPAPI(IppStatus, ippsAdd_32sc_Sfs,   (const Ipp32sc* pSrc1, const Ipp32sc* pSrc2,
       Ipp32sc* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSub_32s_Sfs,    (const Ipp32s*  pSrc1, const Ipp32s*  pSrc2,
       Ipp32s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSub_32sc_Sfs,   (const Ipp32sc* pSrc1, const Ipp32sc* pSrc2,
       Ipp32sc* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_32s_Sfs,    (const Ipp32s*  pSrc1, const Ipp32s*  pSrc2,
       Ipp32s*  pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_32sc_Sfs,   (const Ipp32sc* pSrc1, const Ipp32sc* pSrc2,
       Ipp32sc* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_16u16s_Sfs, (const Ipp16u* pSrc1, const Ipp16s* pSrc2,
       Ipp16s* pDst, int len, int scaleFactor))


IPPAPI(IppStatus, ippsMul_32s32sc_ISfs, (const Ipp32s* pSrc, Ipp32sc* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_32s32sc_Sfs,  (const Ipp32s* pSrc1, const Ipp32sc* pSrc2,
       Ipp32sc* pDst, int len, int scaleFactor))

IPPAPI(IppStatus, ippsMul_Low_32s_Sfs, ( const Ipp32s* pSrc1, const Ipp32s* pSrc2,
       Ipp32s* pDst, int len, int scaleFactor))

IPPAPI(IppStatus, ippsMul_32f32fc_I, (const Ipp32f* pSrc, Ipp32fc* pSrcDst,
       int len))
IPPAPI(IppStatus, ippsMul_32f32fc, (const Ipp32f* pSrc1, const Ipp32fc* pSrc2,
       Ipp32fc* pDst, int len))

IPPAPI(IppStatus, ippsAdd_16s32s_I, (const Ipp16s* pSrc, Ipp32s* pSrcDst, int len))

IPPAPI(IppStatus, ippsAddC_16u_ISfs, (Ipp16u val, Ipp16u* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsAddC_16u_Sfs, (const Ipp16u* pSrc, Ipp16u val, Ipp16u* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsAdd_16u_ISfs, (const Ipp16u* pSrc, Ipp16u* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsAdd_16u_Sfs, (const Ipp16u* pSrc1, const Ipp16u* pSrc2, Ipp16u* pDst, int len, int scaleFactor))

IPPAPI(IppStatus, ippsSubC_16u_ISfs, (Ipp16u val, Ipp16u* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubC_16u_Sfs, (const Ipp16u* pSrc, Ipp16u val, Ipp16u* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubCRev_16u_ISfs, (Ipp16u val, Ipp16u* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSubCRev_16u_Sfs, (const Ipp16u* pSrc, Ipp16u val, Ipp16u* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSub_16u_ISfs, (const Ipp16u* pSrc, Ipp16u* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsSub_16u_Sfs, (const Ipp16u* pSrc1, const Ipp16u* pSrc2, Ipp16u* pDst, int len, int scaleFactor))

IPPAPI(IppStatus, ippsMulC_16u_ISfs, (Ipp16u val, Ipp16u* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMulC_16u_Sfs, (const Ipp16u* pSrc, Ipp16u val, Ipp16u* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_16u_ISfs, (const Ipp16u* pSrc, Ipp16u* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMul_16u_Sfs, (const Ipp16u* pSrc1, const Ipp16u* pSrc2, Ipp16u* pDst, int len, int scaleFactor))

IPPAPI(IppStatus, ippsMulC_64s_ISfs, (Ipp64s val, Ipp64s*  pSrcDst, Ipp32u len, int scaleFactor))
IPPAPI(IppStatus, ippsMulC_64f64s_ISfs, (Ipp64f val, Ipp64s*  pSrcDst, Ipp32u len, int scaleFactor))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsAddProduct
//  Purpose:    multiplies elements of two source vectors and adds product to
//              the accumulator vector
//  Parameters:
//    pSrc1                pointer to the first source vector
//    pSrc2                pointer to the second source vector
//    pSrcDst              pointer to the source/destination (accumulator) vector
//    len                  length of the vectors
//    scaleFactor          scale factor value
//  Return:
//    ippStsNullPtrErr     pointer to the vector is NULL
//    ippStsSizeErr        length of the vectors is less or equal zero
//    ippStsNoErr          otherwise
//
//  Notes:                 pSrcDst[n] = pSrcDst[n] + pSrc1[n] * pSrc2[n], n=0,1,2,..len-1.
*/

IPPAPI(IppStatus, ippsAddProduct_16s_Sfs,    ( const Ipp16s* pSrc1, const Ipp16s* pSrc2,
                                               Ipp16s* pSrcDst, int len, int scaleFactor ))
IPPAPI(IppStatus, ippsAddProduct_16s32s_Sfs, ( const Ipp16s* pSrc1, const Ipp16s* pSrc2,
                                               Ipp32s* pSrcDst, int len, int scaleFactor ))
IPPAPI(IppStatus, ippsAddProduct_32s_Sfs,    ( const Ipp32s* pSrc1, const Ipp32s* pSrc2,
                                               Ipp32s* pSrcDst, int len, int scaleFactor ))
IPPAPI(IppStatus, ippsAddProduct_32f,        ( const Ipp32f* pSrc1, const Ipp32f* pSrc2,
                                               Ipp32f* pSrcDst, int len ))
IPPAPI(IppStatus, ippsAddProduct_64f,        ( const Ipp64f* pSrc1, const Ipp64f* pSrc2,
                                               Ipp64f* pSrcDst, int len ))

IPPAPI(IppStatus, ippsAddProduct_32fc,       ( const Ipp32fc* pSrc1, const Ipp32fc* pSrc2,
                                               Ipp32fc* pSrcDst, int len ))
IPPAPI(IppStatus, ippsAddProduct_64fc,       ( const Ipp64fc* pSrc1, const Ipp64fc* pSrc2,
                                               Ipp64fc* pSrcDst, int len ))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsSqr
//  Purpose:    compute square value for every element of the source vector
//  Parameters:
//    pSrcDst          pointer to the source/destination vector
//    pSrc             pointer to the input vector
//    pDst             pointer to the output vector
//    len              length of the vectors
//   scaleFactor       scale factor value
//  Return:
//    ippStsNullPtrErr    pointer(s) the source data NULL
//    ippStsSizeErr       length of the vectors is less or equal zero
//    ippStsNoErr         otherwise
*/
IPPAPI(IppStatus,ippsSqr_32f_I,(Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus,ippsSqr_32fc_I,(Ipp32fc* pSrcDst, int len))
IPPAPI(IppStatus,ippsSqr_64f_I,(Ipp64f* pSrcDst, int len))
IPPAPI(IppStatus,ippsSqr_64fc_I,(Ipp64fc* pSrcDst, int len))

IPPAPI(IppStatus,ippsSqr_32f,(const Ipp32f* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus,ippsSqr_32fc,(const Ipp32fc* pSrc, Ipp32fc* pDst, int len))
IPPAPI(IppStatus,ippsSqr_64f,(const Ipp64f* pSrc, Ipp64f* pDst, int len))
IPPAPI(IppStatus,ippsSqr_64fc,(const Ipp64fc* pSrc, Ipp64fc* pDst, int len))

IPPAPI(IppStatus,ippsSqr_16s_ISfs,(Ipp16s* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus,ippsSqr_16sc_ISfs,(Ipp16sc* pSrcDst, int len, int scaleFactor))

IPPAPI(IppStatus,ippsSqr_16s_Sfs,(const Ipp16s* pSrc, Ipp16s* pDst, int len,
                                  int scaleFactor))
IPPAPI(IppStatus,ippsSqr_16sc_Sfs,(const Ipp16sc* pSrc, Ipp16sc* pDst, int len,
                                   int scaleFactor))
IPPAPI(IppStatus,ippsSqr_8u_ISfs,(Ipp8u* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus,ippsSqr_8u_Sfs,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                 int scaleFactor))
IPPAPI(IppStatus,ippsSqr_16u_ISfs,(Ipp16u* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus,ippsSqr_16u_Sfs,(const Ipp16u* pSrc, Ipp16u* pDst, int len,
                                  int scaleFactor))


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDiv
//
//  Purpose:    divide every element of the source vector by the scalar value
//              or by corresponding element of the second source vector
//  Arguments:
//    val               the divisor value
//    pSrc              pointer to the divisor source vector
//    pSrc1             pointer to the divisor source vector
//    pSrc2             pointer to the dividend source vector
//    pDst              pointer to the destination vector
//    pSrcDst           pointer to the source/destination vector
//    len               vector's length, number of items
//    scaleFactor       scale factor parameter value
//  Return:
//    ippStsNullPtrErr     pointer(s) to the data vector is NULL
//    ippStsSizeErr        length of the vector is less or equal zero
//    ippStsDivByZeroErr   the scalar divisor value is zero
//    ippStsDivByZero      Warning status if an element of divisor vector is
//                      zero. If the dividend is zero than result is
//                      NaN, if the dividend is not zero than result
//                      is Infinity with correspondent sign. The
//                      execution is not aborted. For the integer operation
//                      zero instead of NaN and the corresponding bound
//                      values instead of Infinity
//    ippStsNoErr          otherwise
//  Note:
//    DivC(v,X,Y)  :    Y[n] = X[n] / v
//    DivC(v,X)    :    X[n] = X[n] / v
//    Div(X,Y)     :    Y[n] = Y[n] / X[n]
//    Div(X,Y,Z)   :    Z[n] = Y[n] / X[n]
*/

IPPAPI(IppStatus, ippsDiv_32f, (const Ipp32f* pSrc1, const Ipp32f* pSrc2,
       Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsDiv_32fc, (const Ipp32fc* pSrc1, const Ipp32fc* pSrc2,
       Ipp32fc* pDst, int len))
IPPAPI(IppStatus, ippsDiv_64f, (const Ipp64f* pSrc1, const Ipp64f* pSrc2,
       Ipp64f* pDst, int len))
IPPAPI(IppStatus, ippsDiv_64fc, (const Ipp64fc* pSrc1, const Ipp64fc* pSrc2,
       Ipp64fc* pDst, int len))

IPPAPI(IppStatus, ippsDiv_16s_Sfs, (const Ipp16s* pSrc1, const Ipp16s* pSrc2,
       Ipp16s* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsDiv_8u_Sfs, (const Ipp8u* pSrc1, const Ipp8u* pSrc2,
       Ipp8u* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsDiv_16sc_Sfs, (const Ipp16sc* pSrc1,
       const Ipp16sc* pSrc2, Ipp16sc* pDst, int len, int scaleFactor))

IPPAPI(IppStatus, ippsDivC_32f, (const Ipp32f* pSrc, Ipp32f val,
       Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsDivC_32fc, (const Ipp32fc* pSrc, Ipp32fc val,
       Ipp32fc* pDst, int len))
IPPAPI(IppStatus, ippsDivC_64f, (const Ipp64f* pSrc, Ipp64f val,
       Ipp64f* pDst, int len))
IPPAPI(IppStatus, ippsDivC_64fc, (const Ipp64fc* pSrc, Ipp64fc val,
       Ipp64fc* pDst, int len))

IPPAPI(IppStatus, ippsDivC_16s_Sfs, (const Ipp16s* pSrc, Ipp16s val,
       Ipp16s* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsDivC_8u_Sfs, (const Ipp8u* pSrc, Ipp8u val,
       Ipp8u* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsDivC_16sc_Sfs, (const Ipp16sc* pSrc, Ipp16sc val,
       Ipp16sc* pDst, int len, int scaleFactor))

IPPAPI(IppStatus, ippsDiv_32f_I, (const Ipp32f* pSrc,
       Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus, ippsDiv_32fc_I, (const Ipp32fc* pSrc,
       Ipp32fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsDiv_64f_I, (const Ipp64f* pSrc,
       Ipp64f* pSrcDst, int len))
IPPAPI(IppStatus, ippsDiv_64fc_I, (const Ipp64fc* pSrc,
       Ipp64fc* pSrcDst, int len))

IPPAPI(IppStatus, ippsDiv_16s_ISfs, (const Ipp16s* pSrc, Ipp16s* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsDiv_8u_ISfs, (const Ipp8u* pSrc, Ipp8u* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsDiv_16sc_ISfs, (const Ipp16sc* pSrc, Ipp16sc* pSrcDst,
       int len, int scaleFactor))

IPPAPI(IppStatus, ippsDiv_32s_Sfs, (const Ipp32s* pSrc1, const Ipp32s* pSrc2,
       Ipp32s* pDst, int len, int scaleFactor))

IPPAPI(IppStatus, ippsDiv_32s_ISfs, (const Ipp32s* pSrc, Ipp32s* pSrcDst,
       int len, int ScaleFactor))

IPPAPI(IppStatus, ippsDiv_32s16s_Sfs, (const Ipp16s* pSrc1, const Ipp32s* pSrc2,
       Ipp16s* pDst, int len, int scaleFactor))

IPPAPI(IppStatus, ippsDivC_32f_I, (Ipp32f val, Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus, ippsDivC_32fc_I, (Ipp32fc val, Ipp32fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsDivC_64f_I, (Ipp64f val, Ipp64f* pSrcDst, int len))
IPPAPI(IppStatus, ippsDivC_64fc_I, (Ipp64fc val, Ipp64fc* pSrcDst, int len))

IPPAPI(IppStatus, ippsDivC_16s_ISfs, (Ipp16s val, Ipp16s* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsDivC_8u_ISfs, (Ipp8u val, Ipp8u* pSrcDst,
       int len, int scaleFactor))
IPPAPI(IppStatus, ippsDivC_16sc_ISfs, (Ipp16sc val, Ipp16sc* pSrcDst,
       int len, int scaleFactor))

IPPAPI(IppStatus, ippsDivCRev_16u, (const Ipp16u* pSrc, Ipp16u val,
       Ipp16u* pDst, int len))
IPPAPI(IppStatus, ippsDivCRev_32f, (const Ipp32f* pSrc, Ipp32f val,
      Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsDivCRev_16u_I, (Ipp16u val, Ipp16u* pSrcDst, int len))
IPPAPI(IppStatus, ippsDivCRev_32f_I, (Ipp32f val, Ipp32f* pSrcDst, int len))

IPPAPI(IppStatus, ippsDivC_16u_ISfs, (Ipp16u val, Ipp16u* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsDivC_16u_Sfs, (const Ipp16u* pSrc, Ipp16u val, Ipp16u* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsDiv_16u_ISfs, (const Ipp16u* pSrc, Ipp16u* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsDiv_16u_Sfs, (const Ipp16u* pSrc1, const Ipp16u* pSrc2, Ipp16u* pDst, int len, int scaleFactor))
IPPAPI( IppStatus, ippsDivC_64s_ISfs, ( Ipp64s val, Ipp64s* pSrcDst, Ipp32u len, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsSqrt
//  Purpose:    compute square root value for every element of the source vector
//   pSrc                 pointer to the source vector
//   pDst                 pointer to the destination vector
//   pSrcDst              pointer to the source/destination vector
//   len                  length of the vector(s), number of items
//   scaleFactor          scale factor value
//  Return:
//   ippStsNullPtrErr        pointer to vector is NULL
//   ippStsSizeErr           length of the vector is less or equal zero
//   ippStsSqrtNegArg        negative value in real sequence
//   ippStsNoErr             otherwise
*/
IPPAPI(IppStatus,ippsSqrt_32f_I,(Ipp32f* pSrcDst,int len))
IPPAPI(IppStatus,ippsSqrt_32fc_I,(Ipp32fc* pSrcDst,int len))
IPPAPI(IppStatus,ippsSqrt_64f_I,(Ipp64f* pSrcDst,int len))
IPPAPI(IppStatus,ippsSqrt_64fc_I,(Ipp64fc* pSrcDst,int len))

IPPAPI(IppStatus,ippsSqrt_32f,(const Ipp32f* pSrc,Ipp32f* pDst,int len))
IPPAPI(IppStatus,ippsSqrt_32fc,(const Ipp32fc* pSrc,Ipp32fc* pDst,int len))
IPPAPI(IppStatus,ippsSqrt_64f,(const Ipp64f* pSrc,Ipp64f* pDst,int len))
IPPAPI(IppStatus,ippsSqrt_64fc,(const Ipp64fc* pSrc,Ipp64fc* pDst,int len))

IPPAPI(IppStatus,ippsSqrt_16s_ISfs,(Ipp16s* pSrcDst,int len,int scaleFactor))
IPPAPI(IppStatus,ippsSqrt_16sc_ISfs,(Ipp16sc* pSrcDst,int len,int scaleFactor))

IPPAPI(IppStatus,ippsSqrt_16s_Sfs,(const Ipp16s* pSrc,Ipp16s* pDst,int len,
                                   int scaleFactor))
IPPAPI(IppStatus,ippsSqrt_16sc_Sfs,(const Ipp16sc* pSrc,Ipp16sc* pDst,int len,
                                    int scaleFactor))

IPPAPI(IppStatus,ippsSqrt_64s_ISfs,(Ipp64s* pSrcDst,int len,int scaleFactor))
IPPAPI(IppStatus,ippsSqrt_64s_Sfs,(const Ipp64s* pSrc,Ipp64s* pDst,int len,
                                   int scaleFactor))

IPPAPI(IppStatus,ippsSqrt_8u_ISfs,(Ipp8u* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus,ippsSqrt_8u_Sfs,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                  int scaleFactor))
IPPAPI(IppStatus,ippsSqrt_16u_ISfs,(Ipp16u* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus,ippsSqrt_16u_Sfs,(const Ipp16u* pSrc, Ipp16u* pDst, int len,
                                   int scaleFactor))

IPPAPI(IppStatus,ippsSqrt_32s16s_Sfs,(const Ipp32s* pSrc,Ipp16s* pDst,
        int len, int scaleFactor))
IPPAPI(IppStatus,ippsSqrt_64s16s_Sfs,(const Ipp64s* pSrc,Ipp16s* pDst,
       int len, int scaleFactor))






/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsCubrt
//  Purpose:    Compute cube root of every elements of the source vector
//  Parameters:
//   pSrc                 pointer to the source vector
//   pDst                 pointer to the destination vector
//   len                  length of the vector(s)
//   ScaleFactor          scale factor value
//  Return:
//   ippStsNullPtrErr        pointer(s) to the data vector is NULL
//   ippStsSizeErr           length of the vector(s) is less or equal 0
//   ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippsCubrt_32s16s_Sfs, ( const Ipp32s* pSrc, Ipp16s* pDst, int Len, int sFactor))
IPPAPI(IppStatus, ippsCubrt_32f, ( const Ipp32f* pSrc, Ipp32f* pDst, int Len))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsAbs
//  Purpose:    compute absolute value of each element of the source vector
//  Parameters:
//   pSrcDst            pointer to the source/destination vector
//   pSrc               pointer to the source vector
//   pDst               pointer to the destination vector
//   len                length of the vector(s), number of items
//  Return:
//   ippStsNullPtrErr      pointer(s) to data vector is NULL
//   ippStsSizeErr         length of a vector is less or equal 0
//   ippStsNoErr           otherwise
*/
IPPAPI(IppStatus,ippsAbs_32f_I,(Ipp32f* pSrcDst,int len))
IPPAPI(IppStatus,ippsAbs_64f_I,(Ipp64f* pSrcDst,int len))
IPPAPI(IppStatus,ippsAbs_16s_I,(Ipp16s* pSrcDst,int len))

IPPAPI(IppStatus,ippsAbs_32f,(const Ipp32f* pSrc, Ipp32f* pDst,int len))
IPPAPI(IppStatus,ippsAbs_64f,(const Ipp64f* pSrc, Ipp64f* pDst,int len))
IPPAPI(IppStatus,ippsAbs_16s,(const Ipp16s* pSrc, Ipp16s* pDst,int len))

IPPAPI(IppStatus,ippsAbs_32s_I,(Ipp32s* pSrcDst,int len))
IPPAPI(IppStatus,ippsAbs_32s,(const Ipp32s* pSrc, Ipp32s* pDst,int len))



/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsMagnitude
//  Purpose:    compute magnitude of every complex element of the source
//  Parameters:
//   pSrcDst            pointer to the source/destination vector
//   pSrc               pointer to the source vector
//   pDst               pointer to the destination vector
//   len                length of the vector(s), number of items
//   scaleFactor        scale factor value
//  Return:
//   ippStsNullPtrErr      pointer(s) to data vector is NULL
//   ippStsSizeErr         length of a vector is less or equal 0
//   ippStsNoErr           otherwise
//  Notes:
//         dst = sqrt( src.re^2 + src.im^2 )
*/
IPPAPI(IppStatus,ippsMagnitude_32fc,   (const Ipp32fc* pSrc,Ipp32f* pDst,int len))
IPPAPI(IppStatus,ippsMagnitude_64fc,   (const Ipp64fc* pSrc,Ipp64f* pDst,int len))
IPPAPI(IppStatus,ippsMagnitude_16sc32f,(const Ipp16sc* pSrc,Ipp32f* pDst,int len))
IPPAPI(IppStatus,ippsMagnitude_16sc_Sfs,(const Ipp16sc* pSrc,Ipp16s* pDst,
                                    int len,int scaleFactor))
IPPAPI(IppStatus,ippsMagnitude_32f,(const Ipp32f* pSrcRe,const Ipp32f* pSrcIm,
                               Ipp32f* pDst,int len))
IPPAPI(IppStatus,ippsMagnitude_64f,(const Ipp64f* pSrcRe,const Ipp64f* pSrcIm,
                               Ipp64f* pDst,int len))
IPPAPI(IppStatus,ippsMagnitude_16s_Sfs,(const Ipp16s* pSrcRe,const Ipp16s* pSrcIm,
                               Ipp16s* pDst,int len,int scaleFactor))
IPPAPI(IppStatus,ippsMagnitude_32sc_Sfs,(const Ipp32sc* pSrc,Ipp32s* pDst,
                                    int len,int scaleFactor))

IPPAPI(IppStatus,ippsMagnitude_16s32f,(const Ipp16s* pSrcRe, const Ipp16s* pSrcIm,
                                                             Ipp32f* pDst, int len))

IPPAPI(IppStatus, ippsMagSquared_32sc32s_Sfs, (const Ipp32sc* pSrc, Ipp32s* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsMagSquared_32fc64f, (const Ipp32fc* pSrc, Ipp64f* pDst, int len))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsExp
//  Purpose:    compute exponent value for all elements of the source vector
//  Parameters:
//   pSrcDst            pointer to the source/destination vector
//   pSrc               pointer to the source vector
//   pDst               pointer to the destination vector
//   len                length of the vector(s)
//   scaleFactor        scale factor value
//  Return:
//   ippStsNullPtrErr      pointer(s) to the data vector is NULL
//   ippStsSizeErr         length of the vector(s) is less or equal 0
//   ippStsNoErr           otherwise
*/
IPPAPI(IppStatus, ippsExp_32f_I,(Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus, ippsExp_64f_I,(Ipp64f* pSrcDst, int len))
IPPAPI(IppStatus, ippsExp_16s_ISfs,(Ipp16s* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsExp_32s_ISfs,(Ipp32s* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsExp_64s_ISfs,(Ipp64s* pSrcDst, int len, int scaleFactor))

IPPAPI(IppStatus, ippsExp_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsExp_64f, (const Ipp64f* pSrc, Ipp64f* pDst, int len))
IPPAPI(IppStatus, ippsExp_16s_Sfs,(const Ipp16s* pSrc, Ipp16s* pDst, int len,
   int scaleFactor))
IPPAPI(IppStatus, ippsExp_32s_Sfs,(const Ipp32s* pSrc, Ipp32s* pDst, int len,
   int scaleFactor))
IPPAPI(IppStatus, ippsExp_64s_Sfs,(const Ipp64s* pSrc, Ipp64s* pDst, int len,
   int scaleFactor))

IPPAPI(IppStatus, ippsExp_32f64f,(const Ipp32f* pSrc, Ipp64f* pDst, int len))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsLn
//  Purpose:    compute natural logarithm of every elements of the source vector
//  Parameters:
//   pSrcDst              pointer to the source/destination vector
//   pSrc                 pointer to the source vector
//   pDst                 pointer to the destination vector
//   len                  length of the vector(s)
//   ScaleFactor          scale factor value
//  Return:
//   ippStsNullPtrErr        pointer(s) to the data vector is NULL
//   ippStsSizeErr           length of the vector(s) is less or equal 0
//   ippStsLnZeroArg         zero value in the source vector
//   ippStsLnNegArg          negative value in the source vector
//   ippStsNoErr             otherwise
//  Notes:
//                Ln( x<0 ) = NaN
//                Ln( 0 ) = -Inf
*/

IPPAPI(IppStatus, ippsLn_32f_I,(Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus, ippsLn_64f_I,(Ipp64f* pSrcDst, int len))

IPPAPI(IppStatus, ippsLn_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsLn_64f, (const Ipp64f* pSrc, Ipp64f* pDst, int len))
IPPAPI(IppStatus, ippsLn_64f32f,(const Ipp64f* pSrc, Ipp32f* pDst, int len))

IPPAPI(IppStatus, ippsLn_16s_ISfs,(Ipp16s* pSrcDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsLn_16s_Sfs,(const Ipp16s* pSrc, Ipp16s* pDst, int len, int scaleFactor))

IPPAPI(IppStatus, ippsLn_32s16s_Sfs, ( const Ipp32s* pSrc, Ipp16s* pDst, int Len, int scaleFactor))

IPPAPI(IppStatus, ippsLn_32s_ISfs,( Ipp32s* pSrcDst, int Len, int scaleFactor))
IPPAPI(IppStatus, ippsLn_32s_Sfs, ( const Ipp32s* pSrc, Ipp32s* pDst, int Len, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ipps10Log10_32s_ISfs
//              ipps10Log10_32s_Sfs
//
//  Purpose:    compute decimal logarithm multiplied by 10 of every elements
//              of the source vector (for integer only).
//
//  Parameters:
//   pSrcDst              pointer to the source/destination vector
//   pSrc                 pointer to the source vector
//   pDst                 pointer to the destination vector
//   Len                  length of the vector(s)
//   ScaleFactor          scale factor value
//  Return:
//   ippStsNullPtrErr     pointer(s) to the data vector is NULL
//   ippStsSizeErr        length of the vector(s) is less or equal 0
//   ippStsLnZeroArg      zero value in the source vector
//   ippStsLnNegArg       negative value in the source vector
//   ippStsNoErr          otherwise
//
*/
IPPAPI(IppStatus, ipps10Log10_32s_ISfs,( Ipp32s* pSrcDst, int Len, int scaleFactor))
IPPAPI(IppStatus, ipps10Log10_32s_Sfs, ( const Ipp32s* pSrc, Ipp32s* pDst, int Len, int scaleFactor))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsSumLn
//  Purpose:    computes sum of natural logarithm every elements of the source vector
//  Parameters:
//   pSrc                 pointer to the source vector
//   pSum                 pointer to the result
//   len                  length of the vector
//  Return:
//   ippStsNullPtrErr     pointer(s) to the data vector is NULL
//   ippStsSizeErr        length of the vector(s) is less or equal 0
//   ippStsLnZeroArg      zero value in the source vector
//   ippStsLnNegArg       negative value in the source vector
//   ippStsNoErr          otherwise
*/


IPPAPI(IppStatus, ippsSumLn_32f,(const Ipp32f* pSrc, int len, Ipp32f* pSum))
IPPAPI(IppStatus, ippsSumLn_64f,(const Ipp64f* pSrc, int len, Ipp64f* pSum))
IPPAPI(IppStatus, ippsSumLn_32f64f,(const Ipp32f* pSrc, int len, Ipp64f* pSum))
IPPAPI(IppStatus, ippsSumLn_16s32f,(const Ipp16s* pSrc, int len, Ipp32f* pSum))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippsSortAscend, ippsSortDescend
//
//  Purpose:    Execute sorting of all elemens of the vector.
//              ippsSortAscend  is sorted in increasing order.
//              ippsSortDescend is sorted in decreasing order.
//  Arguments:
//    pSrcDst              pointer to the source/destination vector
//    len                  length of the vector
//  Return:
//    ippStsNullPtrErr     pointer to the data is NULL
//    ippStsSizeErr        length of the vector is less or equal zero
//    ippStsNoErr          otherwise
*/

IPPAPI(IppStatus, ippsSortAscend_8u_I,  (Ipp8u*  pSrcDst, int len))
IPPAPI(IppStatus, ippsSortAscend_16s_I, (Ipp16s* pSrcDst, int len))
IPPAPI(IppStatus, ippsSortAscend_16u_I, (Ipp16u* pSrcDst, int len))
IPPAPI(IppStatus, ippsSortAscend_32s_I, (Ipp32s* pSrcDst, int len))
IPPAPI(IppStatus, ippsSortAscend_32f_I, (Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus, ippsSortAscend_64f_I, (Ipp64f* pSrcDst, int len))

IPPAPI(IppStatus, ippsSortDescend_8u_I,  (Ipp8u*  pSrcDst, int len))
IPPAPI(IppStatus, ippsSortDescend_16s_I, (Ipp16s* pSrcDst, int len))
IPPAPI(IppStatus, ippsSortDescend_16u_I, (Ipp16u* pSrcDst, int len))
IPPAPI(IppStatus, ippsSortDescend_32s_I, (Ipp32s* pSrcDst, int len))
IPPAPI(IppStatus, ippsSortDescend_32f_I, (Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus, ippsSortDescend_64f_I, (Ipp64f* pSrcDst, int len))

IPPAPI(IppStatus,ippsSortIndexAscend_8u_I, (Ipp8u*  pSrcDst, int* pDstIdx, int len ))
IPPAPI(IppStatus,ippsSortIndexAscend_16s_I,(Ipp16s* pSrcDst, int* pDstIdx, int len ))
IPPAPI(IppStatus,ippsSortIndexAscend_16u_I,(Ipp16u* pSrcDst, int* pDstIdx, int len ))
IPPAPI(IppStatus,ippsSortIndexAscend_32s_I,(Ipp32s* pSrcDst, int* pDstIdx, int len ))
IPPAPI(IppStatus,ippsSortIndexAscend_32f_I,(Ipp32f* pSrcDst, int* pDstIdx, int len ))
IPPAPI(IppStatus,ippsSortIndexAscend_64f_I,(Ipp64f* pSrcDst, int* pDstIdx, int len ))

IPPAPI(IppStatus,ippsSortIndexDescend_8u_I, (Ipp8u*  pSrcDst, int* pDstIdx, int len ))
IPPAPI(IppStatus,ippsSortIndexDescend_16s_I,(Ipp16s* pSrcDst, int* pDstIdx, int len ))
IPPAPI(IppStatus,ippsSortIndexDescend_16u_I,(Ipp16u* pSrcDst, int* pDstIdx, int len ))
IPPAPI(IppStatus,ippsSortIndexDescend_32s_I,(Ipp32s* pSrcDst, int* pDstIdx, int len ))
IPPAPI(IppStatus,ippsSortIndexDescend_32f_I,(Ipp32f* pSrcDst, int* pDstIdx, int len ))
IPPAPI(IppStatus,ippsSortIndexDescend_64f_I,(Ipp64f* pSrcDst, int* pDstIdx, int len ))

/* /////////////////////////////////////////////////////////////////////////////////////
//  Names:      ippsSortRadixAscend, ippsSortRadixDescend
//
//  Purpose:    Rearrange elements of input vector using radix sort algorithm.
//              ippsSortRadixAscend  - sorts input array in increasing order
//              ippsSortRadixDescend - sorts input array in decreasing order
//
//  Arguments:
//    pSrcDst              pointer to the source/destination vector
//    pTmp                 pointer to the temporary vector (same size as input one)
//    len                  length of the vectors
//  Return:
//    ippStsNullPtrErr     pointer to the data or temporary placeholders is NULL
//    ippStsSizeErr        length of the vectors is less or equal zero
//    ippStsNoErr          otherwise
*/

IPPAPI(IppStatus, ippsSortRadixAscend_32f_I, (Ipp32f *pSrcDst, Ipp32f *pTmp, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixAscend_32s_I, (Ipp32s *pSrcDst, Ipp32s *pTmp, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixAscend_32u_I, (Ipp32u *pSrcDst, Ipp32u *pTmp, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixAscend_16s_I, (Ipp16s *pSrcDst, Ipp16s *pTmp, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixAscend_16u_I, (Ipp16u *pSrcDst, Ipp16u *pTmp, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixAscend_8u_I,  (Ipp8u  *pSrcDst, Ipp8u  *pTmp, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixAscend_64f_I, (Ipp64f *pSrcDst, Ipp64f *pTmp, Ipp32s len))

IPPAPI(IppStatus, ippsSortRadixDescend_32f_I, (Ipp32f *pSrcDst, Ipp32f *pTmp, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixDescend_32s_I, (Ipp32s *pSrcDst, Ipp32s *pTmp, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixDescend_32u_I, (Ipp32u *pSrcDst, Ipp32u *pTmp, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixDescend_16s_I, (Ipp16s *pSrcDst, Ipp16s *pTmp, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixDescend_16u_I, (Ipp16u *pSrcDst, Ipp16u *pTmp, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixDescend_8u_I,  (Ipp8u  *pSrcDst, Ipp8u  *pTmp, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixDescend_64f_I, (Ipp64f *pSrcDst, Ipp64f *pTmp, Ipp32s len))

/* /////////////////////////////////////////////////////////////////////////////////////
//  Names:      ippsSortRadixIndexAscend, ippsSortRadixIndexDescend
//
//  Purpose:    Indirectly sorts possibly sparse input vector, using indexes.
//              For a dense input array the following will be true:
//
//              ippsSortRadixIndexAscend  - pSrc[pDstIndx[i-1]] <= pSrc[pDstIndx[i]];
//              ippsSortRadixIndexDescend - pSrc[pDstIndx[i]] <= pSrc[pDstIndx[i-1]];
//
//  Arguments:
//    pSrc                 pointer to the first element of a sparse input vector;
//    srcStrideBytes       step between two consecutive elements of input vector in bytes;
//    pDstIndx             pointer to the output indexes vector;
//    pTmpIndx             pointer to the temporary indexes vector (same size as output one);
//    len                  length of the vectors;
//  Return:
//    ippStsNullPtrErr     pointer to the data or temporary placeholders is NULL
//    ippStsSizeErr        length of the vectors is less or equal zero
//    ippStsNoErr          otherwise
*/

IPPAPI(IppStatus, ippsSortRadixIndexAscend_32f, (const Ipp32f * const pSrc, Ipp32s srcStrideBytes,
                                                Ipp32s *pDstIndx, Ipp32s *pTmpIndx, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixIndexAscend_32s, (const Ipp32s * const pSrc, Ipp32s srcStrideBytes,
                                                Ipp32s *pDstIndx, Ipp32s *pTmpIndx, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixIndexAscend_32u, (const Ipp32u * const pSrc, Ipp32s srcStrideBytes,
                                                Ipp32s *pDstIndx, Ipp32s *pTmpIndx, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixIndexAscend_16s, (const Ipp16s * const pSrc, Ipp32s srcStrideBytes,
                                                Ipp32s *pDstIndx, Ipp32s *pTmpIndx, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixIndexAscend_16u, (const Ipp16u * const pSrc, Ipp32s srcStrideBytes,
                                                Ipp32s *pDstIndx, Ipp32s *pTmpIndx, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixIndexAscend_8u,  (const Ipp8u  * const pSrc, Ipp32s srcStrideBytes,
                                                Ipp32s *pDstIndx, Ipp32s *pTmpIndx, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixIndexDescend_32f, (const Ipp32f * const pSrc, Ipp32s srcStrideBytes,
                                                Ipp32s *pDstIndx, Ipp32s *pTmpIndx, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixIndexDescend_32s, (const Ipp32s * const pSrc, Ipp32s srcStrideBytes,
                                                Ipp32s *pDstIndx, Ipp32s *pTmpIndx, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixIndexDescend_32u, (const Ipp32u * const pSrc, Ipp32s srcStrideBytes,
                                                Ipp32s *pDstIndx, Ipp32s *pTmpIndx, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixIndexDescend_16s, (const Ipp16s * const pSrc, Ipp32s srcStrideBytes,
                                                Ipp32s *pDstIndx, Ipp32s *pTmpIndx, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixIndexDescend_16u, (const Ipp16u * const pSrc, Ipp32s srcStrideBytes,
                                                Ipp32s *pDstIndx, Ipp32s *pTmpIndx, Ipp32s len))
IPPAPI(IppStatus, ippsSortRadixIndexDescend_8u,  (const Ipp8u  * const pSrc, Ipp32s srcStrideBytes,
                                                Ipp32s *pDstIndx, Ipp32s *pTmpIndx, Ipp32s len))

/* /////////////////////////////////////////////////////////////////////////////
//                  Vector Measures Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsSum
//  Purpose:    sum all elements of the source vector
//  Parameters:
//   pSrc                pointer to the source vector
//   pSum                pointer to the result
//   len                 length of the vector
//   scaleFactor         scale factor value
//  Return:
//   ippStsNullPtrErr       pointer to the vector or result is NULL
//   ippStsSizeErr          length of the vector is less or equal 0
//   ippStsNoErr            otherwise
*/
IPPAPI(IppStatus,ippsSum_32f, (const Ipp32f*  pSrc,int len, Ipp32f* pSum,
       IppHintAlgorithm hint))
IPPAPI(IppStatus,ippsSum_64f, (const Ipp64f*  pSrc,int len, Ipp64f* pSum))
IPPAPI(IppStatus,ippsSum_32fc,(const Ipp32fc* pSrc,int len, Ipp32fc* pSum,
       IppHintAlgorithm hint))
IPPAPI(IppStatus,ippsSum_16s32s_Sfs, (const Ipp16s*  pSrc, int len,
                                       Ipp32s*  pSum, int scaleFactor))
IPPAPI(IppStatus,ippsSum_16sc32sc_Sfs,(const Ipp16sc* pSrc, int len,
                                       Ipp32sc* pSum, int scaleFactor))
IPPAPI(IppStatus,ippsSum_16s_Sfs, (const Ipp16s*  pSrc, int len,
                                       Ipp16s*  pSum, int scaleFactor))
IPPAPI(IppStatus,ippsSum_16sc_Sfs, (const Ipp16sc* pSrc, int len,
                                       Ipp16sc* pSum, int scaleFactor))
IPPAPI(IppStatus,ippsSum_32s_Sfs, (const Ipp32s*  pSrc, int len,
                                       Ipp32s*  pSum, int scaleFactor))

IPPAPI(IppStatus,ippsSum_64fc,(const Ipp64fc* pSrc,int len, Ipp64fc* pSum))



/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsMean
//  Purpose:    compute average value of all elements of the source vector
//  Parameters:
//   pSrc                pointer to the source vector
//   pMean               pointer to the result
//   len                 length of the source vector
//   scaleFactor         scale factor value
//  Return:
//   ippStsNullPtrErr       pointer(s) to the vector or the result is NULL
//   ippStsSizeErr          length of the vector is less or equal 0
//   ippStsNoErr            otherwise
*/
IPPAPI(IppStatus,ippsMean_32f, (const Ipp32f*  pSrc,int len,Ipp32f*  pMean,
       IppHintAlgorithm hint))
IPPAPI(IppStatus,ippsMean_32fc,(const Ipp32fc* pSrc,int len,Ipp32fc* pMean,
       IppHintAlgorithm hint))
IPPAPI(IppStatus,ippsMean_64f, (const Ipp64f*  pSrc,int len,Ipp64f*  pMean))
IPPAPI(IppStatus,ippsMean_16s_Sfs, (const Ipp16s*  pSrc,int len,
                                    Ipp16s*  pMean,int scaleFactor))
IPPAPI(IppStatus,ippsMean_16sc_Sfs,(const Ipp16sc* pSrc,int len,
                                    Ipp16sc* pMean,int scaleFactor))
IPPAPI(IppStatus,ippsMean_64fc,(const Ipp64fc* pSrc,int len,Ipp64fc* pMean))
IPPAPI(IppStatus,ippsMean_32s_Sfs,(const Ipp32s *pSrc,int len,Ipp32s *pMean,
       int scaleFactor ) )


/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsStdDev
//  Purpose:    compute standard deviation value of all elements of the vector
//  Parameters:
//   pSrc               pointer to the vector
//   len                length of the vector
//   pStdDev            pointer to the result
//   scaleFactor        scale factor value
//  Return:
//   ippStsNoErr           Ok
//   ippStsNullPtrErr      pointer to the vector or the result is NULL
//   ippStsSizeErr         length of the vector is less than 2
//  Functionality:
//         std = sqrt( sum( (x[n] - mean(x))^2, n=0..len-1 ) / (len-1) )
*/
IPPAPI(IppStatus,ippsStdDev_32f,(const Ipp32f* pSrc,int len,Ipp32f* pStdDev,
       IppHintAlgorithm hint))
IPPAPI(IppStatus,ippsStdDev_64f,(const Ipp64f* pSrc,int len,Ipp64f* pStdDev))

IPPAPI(IppStatus,ippsStdDev_16s32s_Sfs,(const Ipp16s* pSrc,int len,
                                        Ipp32s* pStdDev,int scaleFactor))
IPPAPI(IppStatus,ippsStdDev_16s_Sfs,(const Ipp16s* pSrc,int len,
                                     Ipp16s* pStdDev,int scaleFactor))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsMeanStdDev
//  Purpose:    compute standard deviation value and mean value
//              of all elements of the vector
//  Parameters:
//   pSrc               pointer to the vector
//   len                length of the vector
//   pStdDev            pointer to the result
//   pMean              pointer to the result
//   scaleFactor        scale factor value
//  Return:
//   ippStsNoErr           Ok
//   ippStsNullPtrErr      pointer to the vector or the result is NULL
//   ippStsSizeErr         length of the vector is less than 2
//  Functionality:
//         std = sqrt( sum( (x[n] - mean(x))^2, n=0..len-1 ) / (len-1) )
*/
IPPAPI(IppStatus,ippsMeanStdDev_32f,(const Ipp32f* pSrc, int len, Ipp32f* pMean, Ipp32f* pStdDev,
                                                                            IppHintAlgorithm hint))
IPPAPI(IppStatus,ippsMeanStdDev_64f,(const Ipp64f* pSrc, int len, Ipp64f* pMean, Ipp64f* pStdDev))

IPPAPI(IppStatus,ippsMeanStdDev_16s32s_Sfs,(const Ipp16s* pSrc,int len, Ipp32s* pMean, Ipp32s* pStdDev,
                                                                                    int scaleFactor))
IPPAPI(IppStatus,ippsMeanStdDev_16s_Sfs,(const Ipp16s* pSrc,int len, Ipp16s* pMean, Ipp16s* pStdDev,
                                                                                    int scaleFactor))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsMax
//  Purpose:    find maximum value among all elements of the source vector
//  Parameters:
//   pSrc                 pointer to the source vector
//   pMax                 pointer to the result
//   len                  length of the vector
//  Return:
//   ippStsNullPtrErr        pointer(s) to the vector or the result is NULL
//   ippStsSizeErr           length of the vector is less or equal 0
//   ippStsNoErr             otherwise
*/
IPPAPI(IppStatus,ippsMax_32f,(const Ipp32f* pSrc,int len,Ipp32f* pMax))
IPPAPI(IppStatus,ippsMax_64f,(const Ipp64f* pSrc,int len,Ipp64f* pMax))
IPPAPI(IppStatus,ippsMax_16s,(const Ipp16s* pSrc,int len,Ipp16s* pMax))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:       ippsMaxIndx
//
//  Purpose:    find element with max value and return the value and the index
//  Parameters:
//   pSrc           pointer to the input vector
//   len            length of the vector
//   pMax           address to place max value found
//   pIndx          address to place index found, may be NULL
//  Return:
//   ippStsNullPtrErr        pointer(s) to the data is NULL
//   ippStsSizeErr           length of the vector is less or equal zero
//   ippStsNoErr             otherwise
*/

IPPAPI ( IppStatus, ippsMaxIndx_16s,
         ( const Ipp16s* pSrc, int len, Ipp16s* pMax, int* pIndx ))
IPPAPI ( IppStatus, ippsMaxIndx_32f,
         ( const Ipp32f* pSrc, int len, Ipp32f* pMax, int* pIndx ))
IPPAPI ( IppStatus, ippsMaxIndx_64f,
         ( const Ipp64f* pSrc, int len, Ipp64f* pMax, int* pIndx ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsMin
//  Purpose:    find minimum value among all elements of the source vector
//  Parameters:
//   pSrc                 pointer to the source vector
//   pMin                 pointer to the result
//   len                  length of the vector
//  Return:
//   ippStsNullPtrErr        pointer(s) to the vector or the result is NULL
//   ippStsSizeErr           length of the vector is less or equal 0
//   ippStsNoErr             otherwise
*/
IPPAPI(IppStatus,ippsMin_32f,(const Ipp32f* pSrc,int len,Ipp32f* pMin))
IPPAPI(IppStatus,ippsMin_64f,(const Ipp64f* pSrc,int len,Ipp64f* pMin))
IPPAPI(IppStatus,ippsMin_16s,(const Ipp16s* pSrc,int len,Ipp16s* pMin))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:       ippsMinIndx
//
//  Purpose:    find element with min value and return the value and the index
//  Parameters:
//   pSrc           pointer to the input vector
//   len            length of the vector
//   pMin           address to place min value found
//   pIndx          address to place index found, may be NULL
//  Return:
//   ippStsNullPtrErr        pointer(s) to the data is NULL
//   ippStsSizeErr           length of the vector is less or equal zero
//   ippStsNoErr             otherwise
*/
IPPAPI ( IppStatus, ippsMinIndx_16s,
         ( const Ipp16s* pSrc, int len, Ipp16s* pMin, int* pIndx ))
IPPAPI ( IppStatus, ippsMinIndx_32f,
         ( const Ipp32f* pSrc, int len, Ipp32f* pMin, int* pIndx ))
IPPAPI ( IppStatus, ippsMinIndx_64f,
         ( const Ipp64f* pSrc, int len, Ipp64f* pMin, int* pIndx ))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:              ippsMinEvery, ippsMaxEvery
//  Purpose:            calculation min/max value for every element of two vectors
//  Parameters:
//   pSrc               pointer to input vector
//   pSrcDst            pointer to input/output vector
//   len                vector's length
//  Return:
//   ippStsNullPtrErr      pointer(s) to the data is NULL
//   ippStsSizeErr         vector`s length is less or equal zero
//   ippStsNoErr           otherwise
*/

IPPAPI(IppStatus, ippsMinEvery_16s_I, (const Ipp16s* pSrc, Ipp16s* pSrcDst, int len))
IPPAPI(IppStatus, ippsMinEvery_32s_I, (const Ipp32s* pSrc, Ipp32s* pSrcDst, int len))
IPPAPI(IppStatus, ippsMinEvery_32f_I, (const Ipp32f* pSrc, Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus, ippsMaxEvery_16s_I, (const Ipp16s* pSrc, Ipp16s* pSrcDst, int len))
IPPAPI(IppStatus, ippsMaxEvery_32s_I, (const Ipp32s* pSrc, Ipp32s* pSrcDst, int len))
IPPAPI(IppStatus, ippsMaxEvery_32f_I, (const Ipp32f* pSrc, Ipp32f* pSrcDst, int len))

IPPAPI(IppStatus, ippsMinEvery_64f_I, (const Ipp64f* pSrc, Ipp64f* pSrcDst, Ipp32u len))

IPPAPI(IppStatus, ippsMaxEvery_8u_I, (const Ipp8u*  pSrc, Ipp8u*  pSrcDst, int len ))
IPPAPI(IppStatus, ippsMinEvery_8u_I, (const Ipp8u*  pSrc, Ipp8u*  pSrcDst, int len ))
IPPAPI(IppStatus, ippsMaxEvery_16u_I,(const Ipp16u* pSrc, Ipp16u* pSrcDst, int len ))
IPPAPI(IppStatus, ippsMinEvery_16u_I,(const Ipp16u* pSrc, Ipp16u* pSrcDst, int len ))


IPPAPI(IppStatus, ippsMinMax_64f,(const Ipp64f* pSrc, int len, Ipp64f* pMin, Ipp64f* pMax))
IPPAPI(IppStatus, ippsMinMax_32f,(const Ipp32f* pSrc, int len, Ipp32f* pMin, Ipp32f* pMax))
IPPAPI(IppStatus, ippsMinMax_32s,(const Ipp32s* pSrc, int len, Ipp32s* pMin, Ipp32s* pMax))
IPPAPI(IppStatus, ippsMinMax_32u,(const Ipp32u* pSrc, int len, Ipp32u* pMin, Ipp32u* pMax))
IPPAPI(IppStatus, ippsMinMax_16s,(const Ipp16s* pSrc, int len, Ipp16s* pMin, Ipp16s* pMax))
IPPAPI(IppStatus, ippsMinMax_16u,(const Ipp16u* pSrc, int len, Ipp16u* pMin, Ipp16u* pMax))
IPPAPI(IppStatus, ippsMinMax_8u, (const Ipp8u*  pSrc, int len, Ipp8u*  pMin, Ipp8u*  pMax))


IPPAPI(IppStatus, ippsMinMaxIndx_64f,(const Ipp64f* pSrc, int len, Ipp64f* pMin, int* pMinIndx,
                                                                   Ipp64f* pMax, int* pMaxIndx))
IPPAPI(IppStatus, ippsMinMaxIndx_32f,(const Ipp32f* pSrc, int len, Ipp32f* pMin, int* pMinIndx,
                                                                   Ipp32f* pMax, int* pMaxIndx))
IPPAPI(IppStatus, ippsMinMaxIndx_32s,(const Ipp32s* pSrc, int len, Ipp32s* pMin, int* pMinIndx,
                                                                   Ipp32s* pMax, int* pMaxIndx))
IPPAPI(IppStatus, ippsMinMaxIndx_32u,(const Ipp32u* pSrc, int len, Ipp32u* pMin, int* pMinIndx,
                                                                   Ipp32u* pMax, int* pMaxIndx))
IPPAPI(IppStatus, ippsMinMaxIndx_16s,(const Ipp16s* pSrc, int len, Ipp16s* pMin, int* pMinIndx,
                                                                   Ipp16s* pMax, int* pMaxIndx))
IPPAPI(IppStatus, ippsMinMaxIndx_16u,(const Ipp16u* pSrc, int len, Ipp16u* pMin, int* pMinIndx,
                                                                   Ipp16u* pMax, int* pMaxIndx))
IPPAPI(IppStatus, ippsMinMaxIndx_8u, (const Ipp8u*  pSrc, int len, Ipp8u*  pMin, int* pMinIndx,
                                                                   Ipp8u*  pMax, int* pMaxIndx))




IPPAPI(IppStatus, ippsMin_32s, (const Ipp32s* pSrc, int len, Ipp32s* pMin))

IPPAPI(IppStatus, ippsMax_32s, (const Ipp32s* pSrc, int len, Ipp32s* pMax))

IPPAPI(IppStatus, ippsMinIndx_32s, (const Ipp32s* pSrc, int len, Ipp32s* pMin, int* pIndx))

IPPAPI(IppStatus, ippsMaxIndx_32s, (const Ipp32s* pSrc, int len, Ipp32s* pMax, int* pIndx))



IPPAPI(IppStatus, ippsMinAbs_16s, (const Ipp16s* pSrc, int len, Ipp16s* pMinAbs))

IPPAPI(IppStatus, ippsMaxAbs_16s, (const Ipp16s* pSrc, int len, Ipp16s* pMaxAbs))

IPPAPI(IppStatus, ippsMinAbsIndx_16s, (const Ipp16s* pSrc, int len, Ipp16s* pMinAbs, int* pIndx))

IPPAPI(IppStatus, ippsMaxAbsIndx_16s, (const Ipp16s* pSrc, int len, Ipp16s* pMaxAbs, int* pIndx))



IPPAPI(IppStatus, ippsMinAbs_32s, (const Ipp32s* pSrc, int len, Ipp32s* pMinAbs))

IPPAPI(IppStatus, ippsMaxAbs_32s, (const Ipp32s* pSrc, int len, Ipp32s* pMaxAbs))

IPPAPI(IppStatus, ippsMinAbsIndx_32s, (const Ipp32s* pSrc, int len, Ipp32s* pMinAbs, int* pIndx))

IPPAPI(IppStatus, ippsMaxAbsIndx_32s, (const Ipp32s* pSrc, int len, Ipp32s* pMaxAbs, int* pIndx))





/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//    ippsPhase_64fc
//    ippsPhase_32fc
//    ippsPhase_16sc_Sfs
//    ippsPhase_16sc32f
//  Purpose:
//    Compute the phase (in radians) of complex vector elements.
//  Parameters:
//    pSrcRe    - an input complex vector
//    pDst      - an output vector to store the phase components;
//    len       - a length of the arrays.
//    scaleFactor   - a scale factor of output results (only for integer data)
//  Return:
//    ippStsNoErr               Ok
//    ippStsNullPtrErr          Some of pointers to input or output data are NULL
//    ippStsBadSizeErr          The length of the arrays is less or equal zero
*/
IPPAPI(IppStatus, ippsPhase_64fc,(const Ipp64fc* pSrc, Ipp64f* pDst, int len))
IPPAPI(IppStatus, ippsPhase_32fc,(const Ipp32fc* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsPhase_16sc32f,(const Ipp16sc* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsPhase_16sc_Sfs,(const Ipp16sc* pSrc, Ipp16s* pDst, int len,
                                                                int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//    ippsPhase_64f
//    ippsPhase_32f
//    ippsPhase_16s_Sfs
//    ippsPhase_16s32f
//  Purpose:
//    Compute the phase of complex data formed as two real vectors.
//  Parameters:
//    pSrcRe    - an input vector containing a real part of complex data
//    pSrcIm    - an input vector containing an imaginary part of complex data
//    pDst      - an output vector to store the phase components
//    len       - a length of the arrays.
//    scaleFactor   - a scale factor of output results (only for integer data)
//  Return:
//    ippStsNoErr               Ok
//    ippStsNullPtrErr          Some of pointers to input or output data are NULL
//    ippStsBadSizeErr          The length of the arrays is less or equal zero
*/
IPPAPI(IppStatus, ippsPhase_64f,(const Ipp64f* pSrcRe, const Ipp64f* pSrcIm,
                                                            Ipp64f* pDst, int len))
IPPAPI(IppStatus, ippsPhase_32f,(const Ipp32f* pSrcRe, const Ipp32f* pSrcIm,
                                                            Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsPhase_16s_Sfs,(const Ipp16s* pSrcRe, const Ipp16s* pSrcIm,
                                           Ipp16s* pDst, int len, int scaleFactor))
IPPAPI(IppStatus, ippsPhase_16s32f,(const Ipp16s* pSrcRe, const Ipp16s* pSrcIm,
                                                            Ipp32f* pDst, int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//    ippsPhase_32sc_Sfs
//  Purpose:
//    Compute the phase (in radians) of complex vector elements.
//  Parameters:
//    pSrcRe    - an input complex vector
//    pDst      - an output vector to store the phase components;
//    len       - a length of the arrays.
//    scaleFactor   - a scale factor of output results (only for integer data)
//  Return:
//    ippStsNoErr               Ok
//    ippStsNullPtrErr          Some of pointers to input or output data are NULL
//    ippStsBadSizeErr          The length of the arrays is less or equal zero
*/
IPPAPI(IppStatus, ippsPhase_32sc_Sfs,(const Ipp32sc* pSrc, Ipp32s* pDst, int len,
                                                                int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//      ippsMaxOrder_64f
//      ippsMaxOrder_32f
//      ippsMaxOrder_32s
//      ippsMaxOrder_16s
//  Purpose:
//     Determines the maximal number of binary digits for data representation.
//  Parameters:
//    pSrc     The pointer on input signal vector.
//    pOrder   Pointer to result value.
//    len      The  length of  the input vector.
//  Return:
//    ippStsNoErr         Ok
//    ippStsNullPtrErr    Some of pointers to input or output data are NULL
//    ippStsSizeErr       The length of the arrays is less or equal zero
//    ippStsNanArg        If not a number is met in a input value
*/

IPPAPI(IppStatus, ippsMaxOrder_64f,(const Ipp64f* pSrc, int len, int* pOrder))
IPPAPI(IppStatus, ippsMaxOrder_32f,(const Ipp32f* pSrc, int len, int* pOrder))
IPPAPI(IppStatus, ippsMaxOrder_32s,(const Ipp32s* pSrc, int len, int* pOrder))
IPPAPI(IppStatus, ippsMaxOrder_16s,(const Ipp16s* pSrc, int len, int* pOrder))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsArctan
//
//  Purpose: compute arctangent value for all elements of the source vector
//
//  Return:
//   stsNoErr           Ok
//   stsNullPtrErr      Some of pointers to input or output data are NULL
//   stsBadSizeErr      The length of the arrays is less or equal zero
//
//  Parameters:
//   pSrcDst            pointer to the source/destination vector
//   pSrc               pointer to the source vector
//   pDst               pointer to the destination vector
//   len                a length of the array
//
*/

IPPAPI(IppStatus, ippsArctan_32f_I,(      Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus, ippsArctan_32f,  (const Ipp32f* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsArctan_64f_I,(      Ipp64f* pSrcDst, int len))
IPPAPI(IppStatus, ippsArctan_64f,  (const Ipp64f* pSrc, Ipp64f* pDst, int len))




/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippsFindNearestOne
//  Purpose:        Searches the table for an element closest to the reference value
//                  and returns its value and index
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   At least one of the specified pointers is NULL
//    ippStsSizeErr      The length of the table is less than or equal to zero
//  Parameters:
//    inpVal        reference Value
//    pOutVal       pointer to the found value
//    pOutIndx      pointer to the found index
//    pTable        table for search
//    tblLen        length of the table
//  Notes:
//                  The table should contain monotonically increasing values
*/

IPPAPI(IppStatus, ippsFindNearestOne_16u, (Ipp16u inpVal, Ipp16u* pOutVal, int* pOutIndex, const Ipp16u *pTable, int tblLen))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:           ippsFindNearest
//  Purpose:        Searches the table for elements closest to the reference values
//                  and the their indexes
//  Context:
//  Returns:        IppStatus
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   At least one of the specified pointers is NULL
//    ippStsSizeErr      The length of table or pVals is less than or equal to zero
//  Parameters:
//    pVals         pointer to the reference values vector
//    pOutVals      pointer to the vector with the found values
//    pOutIndexes   pointer to the array with indexes of the found elements
//    len           length of the input vector
//    pTable        table for search
//    tblLen        length of the table
//  Notes:
//                  The table should contain monotonically increasing values
*/


IPPAPI(IppStatus, ippsFindNearest_16u, (const Ipp16u* pVals, Ipp16u* pOutVals, int* pOutIndexes, int len, const Ipp16u *pTable, int tblLen))


/* /////////////////////////////////////////////////////////////////////////////
//                      Vector logical functions
///////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////////
//  Names:              ippsAnd, ippsOr, ippsXor, ippsNot, ippsLShiftC, ippsRShiftC
//  Purpose:            logical operations and vector shifts
//  Parameters:
//   val                1) value to be ANDed/ORed/XORed with each element of the vector (And, Or, Xor);
//                      2) position`s number which vector elements to be SHIFTed on (ShiftC)
//   pSrc               pointer to input vector
//   pSrcDst            pointer to input/output vector
//   pSrc1              pointer to first input vector
//   pSrc2              pointer to second input vector
//   pDst               pointer to output vector
//   len                vector's length
//  Return:
//   ippStsNullPtrErr      pointer(s) to the data is NULL
//   ippStsSizeErr         vector`s length is less or equal zero
//   ippStsShiftErr        shift`s value is less zero
//   ippStsNoErr           otherwise
*/

IPPAPI(IppStatus, ippsAndC_8u_I, (Ipp8u val, Ipp8u* pSrcDst, int len))
IPPAPI(IppStatus, ippsAndC_8u, (const Ipp8u* pSrc, Ipp8u val, Ipp8u* pDst, int len))
IPPAPI(IppStatus, ippsAndC_16u_I, (Ipp16u val, Ipp16u* pSrcDst, int len))
IPPAPI(IppStatus, ippsAndC_16u, (const Ipp16u* pSrc, Ipp16u val, Ipp16u* pDst, int len))
IPPAPI(IppStatus, ippsAndC_32u_I, (Ipp32u val, Ipp32u* pSrcDst, int len))
IPPAPI(IppStatus, ippsAndC_32u, (const Ipp32u* pSrc, Ipp32u val, Ipp32u* pDst, int len))
IPPAPI(IppStatus, ippsAnd_8u_I, (const Ipp8u* pSrc, Ipp8u* pSrcDst, int len))
IPPAPI(IppStatus, ippsAnd_8u, (const Ipp8u* pSrc1, const Ipp8u* pSrc2, Ipp8u* pDst, int len))
IPPAPI(IppStatus, ippsAnd_16u_I, (const Ipp16u* pSrc, Ipp16u* pSrcDst, int len))
IPPAPI(IppStatus, ippsAnd_16u, (const Ipp16u* pSrc1, const Ipp16u* pSrc2, Ipp16u* pDst, int len))
IPPAPI(IppStatus, ippsAnd_32u_I, (const Ipp32u* pSrc, Ipp32u* pSrcDst, int len))
IPPAPI(IppStatus, ippsAnd_32u, (const Ipp32u* pSrc1, const Ipp32u* pSrc2, Ipp32u* pDst, int len))

IPPAPI(IppStatus, ippsOrC_8u_I, (Ipp8u val, Ipp8u* pSrcDst, int len))
IPPAPI(IppStatus, ippsOrC_8u, (const Ipp8u* pSrc, Ipp8u val, Ipp8u* pDst, int len))
IPPAPI(IppStatus, ippsOrC_16u_I, (Ipp16u val, Ipp16u* pSrcDst, int len))
IPPAPI(IppStatus, ippsOrC_16u, (const Ipp16u* pSrc, Ipp16u val, Ipp16u* pDst, int len))
IPPAPI(IppStatus, ippsOrC_32u_I, (Ipp32u val, Ipp32u* pSrcDst, int len))
IPPAPI(IppStatus, ippsOrC_32u, (const Ipp32u* pSrc, Ipp32u val, Ipp32u* pDst, int len))
IPPAPI(IppStatus, ippsOr_8u_I, (const Ipp8u* pSrc, Ipp8u* pSrcDst, int len))
IPPAPI(IppStatus, ippsOr_8u, (const Ipp8u* pSrc1, const Ipp8u* pSrc2, Ipp8u* pDst, int len))
IPPAPI(IppStatus, ippsOr_16u_I, (const Ipp16u* pSrc, Ipp16u* pSrcDst, int len))
IPPAPI(IppStatus, ippsOr_16u, (const Ipp16u* pSrc1, const Ipp16u* pSrc2, Ipp16u* pDst, int len))
IPPAPI(IppStatus, ippsOr_32u_I, (const Ipp32u* pSrc, Ipp32u* pSrcDst, int len))
IPPAPI(IppStatus, ippsOr_32u, (const Ipp32u* pSrc1, const Ipp32u* pSrc2, Ipp32u* pDst, int len))

IPPAPI(IppStatus, ippsXorC_8u_I, (Ipp8u val, Ipp8u* pSrcDst, int len))
IPPAPI(IppStatus, ippsXorC_8u, (const Ipp8u* pSrc, Ipp8u val, Ipp8u* pDst, int len))
IPPAPI(IppStatus, ippsXorC_16u_I, (Ipp16u val, Ipp16u* pSrcDst, int len))
IPPAPI(IppStatus, ippsXorC_16u, (const Ipp16u* pSrc, Ipp16u val, Ipp16u* pDst, int len))
IPPAPI(IppStatus, ippsXorC_32u_I, (Ipp32u val, Ipp32u* pSrcDst, int len))
IPPAPI(IppStatus, ippsXorC_32u, (const Ipp32u* pSrc, Ipp32u val, Ipp32u* pDst, int len))
IPPAPI(IppStatus, ippsXor_8u_I, (const Ipp8u* pSrc, Ipp8u* pSrcDst, int len))
IPPAPI(IppStatus, ippsXor_8u, (const Ipp8u* pSrc1, const Ipp8u* pSrc2, Ipp8u* pDst, int len))
IPPAPI(IppStatus, ippsXor_16u_I, (const Ipp16u* pSrc, Ipp16u* pSrcDst, int len))
IPPAPI(IppStatus, ippsXor_16u, (const Ipp16u* pSrc1, const Ipp16u* pSrc2, Ipp16u* pDst, int len))
IPPAPI(IppStatus, ippsXor_32u_I, (const Ipp32u* pSrc, Ipp32u* pSrcDst, int len))
IPPAPI(IppStatus, ippsXor_32u, (const Ipp32u* pSrc1, const Ipp32u* pSrc2, Ipp32u* pDst, int len))

IPPAPI(IppStatus, ippsNot_8u_I, (Ipp8u* pSrcDst, int len))
IPPAPI(IppStatus, ippsNot_8u, (const Ipp8u* pSrc, Ipp8u* pDst, int len))
IPPAPI(IppStatus, ippsNot_16u_I, (Ipp16u* pSrcDst, int len))
IPPAPI(IppStatus, ippsNot_16u, (const Ipp16u* pSrc, Ipp16u* pDst, int len))
IPPAPI(IppStatus, ippsNot_32u_I, (Ipp32u* pSrcDst, int len))
IPPAPI(IppStatus, ippsNot_32u, (const Ipp32u* pSrc, Ipp32u* pDst, int len))

IPPAPI(IppStatus, ippsLShiftC_8u_I, (int val, Ipp8u* pSrcDst, int len))
IPPAPI(IppStatus, ippsLShiftC_8u, (const Ipp8u* pSrc, int val, Ipp8u* pDst, int len))
IPPAPI(IppStatus, ippsLShiftC_16u_I, (int val, Ipp16u* pSrcDst, int len))
IPPAPI(IppStatus, ippsLShiftC_16u, (const Ipp16u* pSrc, int val, Ipp16u* pDst, int len))
IPPAPI(IppStatus, ippsLShiftC_16s_I, (int val, Ipp16s* pSrcDst, int len))
IPPAPI(IppStatus, ippsLShiftC_16s, (const Ipp16s* pSrc, int val, Ipp16s* pDst, int len))
IPPAPI(IppStatus, ippsLShiftC_32s_I, (int val, Ipp32s* pSrcDst, int len))
IPPAPI(IppStatus, ippsLShiftC_32s, (const Ipp32s* pSrc, int val, Ipp32s* pDst, int len))

IPPAPI(IppStatus, ippsRShiftC_8u_I, (int val, Ipp8u* pSrcDst, int len))
IPPAPI(IppStatus, ippsRShiftC_8u, (const Ipp8u* pSrc, int val, Ipp8u* pDst, int len))
IPPAPI(IppStatus, ippsRShiftC_16u_I, (int val, Ipp16u* pSrcDst, int len))
IPPAPI(IppStatus, ippsRShiftC_16u, (const Ipp16u* pSrc, int val, Ipp16u* pDst, int len))
IPPAPI(IppStatus, ippsRShiftC_16s_I, (int val, Ipp16s* pSrcDst, int len))
IPPAPI(IppStatus, ippsRShiftC_16s, (const Ipp16s* pSrc, int val, Ipp16s* pDst, int len))
IPPAPI(IppStatus, ippsRShiftC_32s_I, (int val, Ipp32s* pSrcDst, int len))
IPPAPI(IppStatus, ippsRShiftC_32s, (const Ipp32s* pSrc, int val, Ipp32s* pDst, int len))



/* /////////////////////////////////////////////////////////////////////////////
//                  Dot Product Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDotProd
//  Purpose:    compute Dot Product value
//  Arguments:
//     pSrc1               pointer to the source vector
//     pSrc2               pointer to the another source vector
//     len                 vector's length, number of items
//     pDp                 pointer to the result
//     scaleFactor         scale factor value
//  Return:
//     ippStsNullPtrErr       pointer(s) pSrc pDst is NULL
//     ippStsSizeErr          length of the vectors is less or equal 0
//     ippStsNoErr            otherwise
//  Notes:
//     the functions don't conjugate one of the source vectors
*/

IPPAPI(IppStatus, ippsDotProd_32f, (const Ipp32f* pSrc1,
       const Ipp32f* pSrc2, int len, Ipp32f* pDp))
IPPAPI(IppStatus, ippsDotProd_32fc,(const Ipp32fc* pSrc1,
       const Ipp32fc* pSrc2, int len, Ipp32fc* pDp))
IPPAPI(IppStatus, ippsDotProd_32f32fc,(const Ipp32f* pSrc1,
       const Ipp32fc* pSrc2, int len, Ipp32fc* pDp))

IPPAPI(IppStatus, ippsDotProd_64f, (const Ipp64f* pSrc1,
       const Ipp64f* pSrc2, int len, Ipp64f* pDp))
IPPAPI(IppStatus, ippsDotProd_64fc,(const Ipp64fc* pSrc1,
       const Ipp64fc* pSrc2, int len, Ipp64fc* pDp))
IPPAPI(IppStatus, ippsDotProd_64f64fc,(const Ipp64f* pSrc1,
       const Ipp64fc* pSrc2, int len, Ipp64fc* pDp))

IPPAPI(IppStatus, ippsDotProd_16s_Sfs, (const Ipp16s* pSrc1,
       const Ipp16s* pSrc2, int len, Ipp16s* pDp, int scaleFactor))
IPPAPI(IppStatus, ippsDotProd_16sc_Sfs,(const Ipp16sc* pSrc1,
       const Ipp16sc* pSrc2, int len, Ipp16sc* pDp, int scaleFactor))
IPPAPI(IppStatus, ippsDotProd_16s16sc_Sfs, (const Ipp16s* pSrc1,
       const Ipp16sc* pSrc2, int len, Ipp16sc* pDp, int scaleFactor))

IPPAPI(IppStatus, ippsDotProd_16s64s, (const Ipp16s*  pSrc1,
       const Ipp16s*  pSrc2, int len, Ipp64s*  pDp))
IPPAPI(IppStatus, ippsDotProd_16sc64sc, (const Ipp16sc* pSrc1,
       const Ipp16sc* pSrc2, int len, Ipp64sc* pDp))
IPPAPI(IppStatus, ippsDotProd_16s16sc64sc,(const Ipp16s*  pSrc1,
       const Ipp16sc* pSrc2, int len, Ipp64sc* pDp))

IPPAPI(IppStatus, ippsDotProd_16s32f, (const Ipp16s*  pSrc1,
       const Ipp16s*  pSrc2, int len, Ipp32f*  pDp))
IPPAPI(IppStatus, ippsDotProd_16sc32fc, (const Ipp16sc* pSrc1,
       const Ipp16sc* pSrc2, int len, Ipp32fc* pDp))
IPPAPI(IppStatus, ippsDotProd_16s16sc32fc,(const Ipp16s*  pSrc1,
       const Ipp16sc* pSrc2, int len, Ipp32fc* pDp))

IPPAPI ( IppStatus, ippsDotProd_32f64f,
        ( const Ipp32f* pSrc1, const Ipp32f* pSrc2, int len, Ipp64f* pDp ))
IPPAPI ( IppStatus, ippsDotProd_32fc64fc,
        ( const Ipp32fc* pSrc1, const Ipp32fc* pSrc2, int len, Ipp64fc* pDp ))
IPPAPI ( IppStatus, ippsDotProd_32f32fc64fc,
        ( const Ipp32f* pSrc1, const Ipp32fc* pSrc2, int len, Ipp64fc* pDp ))


IPPAPI ( IppStatus, ippsDotProd_16s32s_Sfs,
        ( const Ipp16s* pSrc1, const Ipp16s* pSrc2,
          int len, Ipp32s* pDp, int scaleFactor ))
IPPAPI ( IppStatus, ippsDotProd_16sc32sc_Sfs,
        ( const Ipp16sc* pSrc1, const Ipp16sc* pSrc2,
          int len, Ipp32sc* pDp, int scaleFactor ))
IPPAPI ( IppStatus, ippsDotProd_16s16sc32sc_Sfs,
        ( const Ipp16s* pSrc1, const Ipp16sc* pSrc2,
          int len, Ipp32sc* pDp, int scaleFactor ))

IPPAPI ( IppStatus, ippsDotProd_32s_Sfs,
        ( const Ipp32s* pSrc1, const Ipp32s* pSrc2,
          int len, Ipp32s* pDp, int scaleFactor ))
IPPAPI ( IppStatus, ippsDotProd_32sc_Sfs,
        ( const Ipp32sc* pSrc1, const Ipp32sc* pSrc2,
          int len, Ipp32sc* pDp, int scaleFactor ))
IPPAPI ( IppStatus, ippsDotProd_32s32sc_Sfs,
        ( const Ipp32s* pSrc1, const Ipp32sc* pSrc2,
          int len, Ipp32sc* pDp, int scaleFactor ))
IPPAPI ( IppStatus, ippsDotProd_16s32s32s_Sfs,
        ( const Ipp16s* pSrc1, const Ipp32s* pSrc2,
          int len, Ipp32s* pDp, int scaleFactor ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//          ippsPowerSpectr_64fc
//          ippsPowerSpectr_32fc
//          ippsPowerSpectr_16sc_Sfs
//          ippsPowerSpectr_16sc32f
//  Purpose:
//    Compute the power spectrum of complex vector
//  Parameters:
//    pSrcRe    - pointer to the real part of input vector.
//    pSrcIm    - pointer to the image part of input vector.
//    pDst      - pointer to the result.
//    len       - vector length.
//    scaleFactor   - scale factor for rezult (only for integer data).
//  Return:
//   ippStsNullPtrErr  indicates that one or more pointers to the data is NULL.
//   ippStsSizeErr     indicates that vector length is less or equal zero.
//   ippStsNoErr       otherwise.
*/



IPPAPI(IppStatus, ippsPowerSpectr_64fc,(const Ipp64fc* pSrc, Ipp64f* pDst, int len))
IPPAPI(IppStatus, ippsPowerSpectr_32fc,(const Ipp32fc* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsPowerSpectr_16sc_Sfs,(const Ipp16sc* pSrc, Ipp16s* pDst,
                                                              int len, int scaleFactor))
IPPAPI(IppStatus, ippsPowerSpectr_16sc32f, (const Ipp16sc* pSrc, Ipp32f* pDst,
                                                                           int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//          ippsPowerSpectr_64f
//          ippsPowerSpectr_32f
//          ippsPowerSpectr_16s_Sfs
//          ippsPowerSpectr_16s32f
//  Purpose:
//    Compute the power spectrum of complex data formed as two real vectors
//  Parameters:
//    pSrcRe    - pointer to the real part of input vector.
//    pSrcIm    - pointer to the image part of input vector.
//    pDst      - pointer to the result.
//    len       - vector length.
//    scaleFactor   - scale factor for rezult (only for integer data).
//  Return:
//   ippStsNullPtrErr  indicates that one or more pointers to the data is NULL.
//   ippStsSizeErr     indicates that vector length is less or equal zero.
//   ippStsNoErr       otherwise.
*/

IPPAPI(IppStatus, ippsPowerSpectr_64f,(const Ipp64f* pSrcRe, const Ipp64f* pSrcIm,
                                                          Ipp64f* pDst, int len))

IPPAPI(IppStatus, ippsPowerSpectr_32f,(const Ipp32f* pSrcRe, const Ipp32f* pSrcIm,
                                                          Ipp32f* pDst, int len))

IPPAPI(IppStatus, ippsPowerSpectr_16s_Sfs,(const Ipp16s* pSrcRe, const Ipp16s* pSrcIm,
                                                Ipp16s* pDst, int len, int scaleFactor))

IPPAPI(IppStatus, ippsPowerSpectr_16s32f, (const Ipp16s* pSrcRe, const Ipp16s* pSrcIm,
                                                             Ipp32f* pDst, int len))

/* /////////////////////////////////////////////////////////////////////////////
//                  Linear Transform
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//    ippsNormalize_64fc
//    ippsNormalize_32fc
//    ippsNormalize_16sc_Sfs
//  Purpose:
//    Complex vector normalization using offset and division method.
//  Parameters:
//    pSrc      - an input complex vector
//    pDst      - an output complex vector
//    len       - a length of the arrays.
//    vsub      - complex a subtrahend
//    vdiv      - denominator
//    scaleFactor   - a scale factor of output results (only for integer data)
//  Return:
//    ippStsNoErr            Ok
//    ippStsNullPtrErr       Some of pointers to input or output data are NULL
//    ippStsSizeErr          The length of the arrays is less or equal zero
//    ippStsDivByZeroErr     denominator equal zero or less than float
//                           format minimum
*/
IPPAPI(IppStatus, ippsNormalize_64fc,(const Ipp64fc* pSrc, Ipp64fc* pDst,
       int len, Ipp64fc vsub, Ipp64f vdiv))
IPPAPI(IppStatus, ippsNormalize_32fc,(const Ipp32fc* pSrc, Ipp32fc* pDst,
       int len, Ipp32fc vsub, Ipp32f vdiv))
IPPAPI(IppStatus, ippsNormalize_16sc_Sfs,(const Ipp16sc* pSrc, Ipp16sc* pDst,
       int len, Ipp16sc vsub, int vdiv, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:
//    ippsNormalize_64f
//    ippsNormalize_32f
//    ippsNormalize_16s_Sfs
//  Purpose:
//    Normalize elements of real vector with the help of offset and division.
//  Parameters:
//    pSrc      - an input vector of real data
//    pDst      - an output vector of real data
//    len       - a length of the arrays.
//    vsub      - subtrahend
//    vdiv      - denominator
//    scaleFactor   - a scale factor of output results (only for integer data)
//  Return:
//    ippStsNoErr               Ok
//    ippStsNullPtrErr          Some of pointers to input or output data are NULL
//    ippStsSizeErr             The length of the arrays is less or equal zero
//    ippStsDivByZeroErr        denominator equal zero or less than float
//                           format minimum
*/
IPPAPI(IppStatus, ippsNormalize_64f,(const Ipp64f* pSrc, Ipp64f* pDst, int len,
       Ipp64f vsub, Ipp64f vdiv))
IPPAPI(IppStatus, ippsNormalize_32f,(const Ipp32f* pSrc, Ipp32f* pDst, int len,
       Ipp32f vsub, Ipp32f vdiv))
IPPAPI(IppStatus, ippsNormalize_16s_Sfs,(const Ipp16s* pSrc, Ipp16s* pDst,
       int len, Ipp16s vsub, int vdiv, int scaleFactor ))


/* /////////////////////////////////////////////////////////////////////////////
//                  Definitions for FFT Functions
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )

typedef struct FFTSpec_C_32fc   IppsFFTSpec_C_32fc;
typedef struct FFTSpec_C_32f    IppsFFTSpec_C_32f;
typedef struct FFTSpec_R_32f    IppsFFTSpec_R_32f;

typedef struct FFTSpec_C_64fc   IppsFFTSpec_C_64fc;
typedef struct FFTSpec_C_64f    IppsFFTSpec_C_64f;
typedef struct FFTSpec_R_64f    IppsFFTSpec_R_64f;

typedef struct FFTSpec_C_16sc   IppsFFTSpec_C_16sc;
typedef struct FFTSpec_C_16s    IppsFFTSpec_C_16s;
typedef struct FFTSpec_R_16s    IppsFFTSpec_R_16s;

typedef struct FFTSpec_C_32sc   IppsFFTSpec_C_32sc;
typedef struct FFTSpec_C_32s    IppsFFTSpec_C_32s;
typedef struct FFTSpec_R_32s    IppsFFTSpec_R_32s;

typedef struct FFTSpec_R_16s32s IppsFFTSpec_R_16s32s;

#endif /* _OWN_BLDPCS */

/* /////////////////////////////////////////////////////////////////////////////
//                  FFT Get Size Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFFTGetSize_C, ippsFFTGetSize_R
//  Purpose:    get sizes of the FFTSpec and buffers (on bytes)
//  Arguments:
//     order           - base-2 logarithm of the number of samples in FFT
//     flag            - normalization flag
//     hint            - code specific use hints
//     pSpecSize       - where write size of FFTSpec
//     pSpecBufferSize - where write size of buffer for FFTInit functions
//     pBufferSize     - where write size of buffer for FFT calculation
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pSpecSize == NULL or pSpecBufferSize == NULL or
//                            pBufferSize == NULL
//     ippStsFftOrderErr      bad the order value
//     ippStsFftFlagErr       bad the normalization flag value
*/

IPPAPI (IppStatus, ippsFFTGetSize_C_32fc,
                   ( int order, int flag, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetSize_C_32f,
                   ( int order, int flag, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetSize_R_32f,
                   ( int order, int flag, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))

IPPAPI (IppStatus, ippsFFTGetSize_C_64fc,
                   ( int order, int flag, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetSize_C_64f,
                   ( int order, int flag, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetSize_R_64f,
                   ( int order, int flag, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))

IPPAPI (IppStatus, ippsFFTGetSize_C_16sc,
                   ( int order, int flag, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetSize_C_16s,
                   ( int order, int flag, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetSize_R_16s,
                   ( int order, int flag, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))

IPPAPI (IppStatus, ippsFFTGetSize_C_32sc,
                   ( int order, int flag, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetSize_C_32s,
                   ( int order, int flag, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetSize_R_32s,
                   ( int order, int flag, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))

IPPAPI (IppStatus, ippsFFTGetSize_R_16s32s,
                   ( int order, int flag, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))

/* /////////////////////////////////////////////////////////////////////////////
//                  FFT Context Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFFTInit_C, ippsFFTInit_R
//  Purpose:    initialize of FFT context
//  Arguments:
//     order       - base-2 logarithm of the number of samples in FFT
//     flag        - normalization flag
//     hint        - code specific use hints
//     ppFFTSpec   - where write pointer to new context
//     pSpec       - pointer to area for FFTSpec
//     pSpecBuffer - pointer to work buffer
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       ppFFTSpec == NULL or
//                            pSpec == NULL or pSpecBuffer == NULL
//     ippStsFftOrderErr      bad the order value
//     ippStsFftFlagErr       bad the normalization flag value
*/

IPPAPI (IppStatus, ippsFFTInit_C_32fc,
                   ( IppsFFTSpec_C_32fc** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))
IPPAPI (IppStatus, ippsFFTInit_C_32f,
                   ( IppsFFTSpec_C_32f** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))
IPPAPI (IppStatus, ippsFFTInit_R_32f,
                   ( IppsFFTSpec_R_32f** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))

IPPAPI (IppStatus, ippsFFTInit_C_64fc,
                   ( IppsFFTSpec_C_64fc** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))
IPPAPI (IppStatus, ippsFFTInit_C_64f,
                   ( IppsFFTSpec_C_64f** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))
IPPAPI (IppStatus, ippsFFTInit_R_64f,
                   ( IppsFFTSpec_R_64f** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))

IPPAPI (IppStatus, ippsFFTInit_C_16sc,
                   ( IppsFFTSpec_C_16sc** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))
IPPAPI (IppStatus, ippsFFTInit_C_16s,
                   ( IppsFFTSpec_C_16s** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))
IPPAPI (IppStatus, ippsFFTInit_R_16s,
                   ( IppsFFTSpec_R_16s** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))

IPPAPI (IppStatus, ippsFFTInit_C_32sc,
                   ( IppsFFTSpec_C_32sc** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))
IPPAPI (IppStatus, ippsFFTInit_C_32s,
                   ( IppsFFTSpec_C_32s** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))
IPPAPI (IppStatus, ippsFFTInit_R_32s,
                   ( IppsFFTSpec_R_32s** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))

IPPAPI (IppStatus, ippsFFTInit_R_16s32s,
                   ( IppsFFTSpec_R_16s32s** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFFTInitAlloc_C, ippsFFTInitAlloc_R
//  Purpose:    create and initialize of FFT context
//  Arguments:
//     order     - base-2 logarithm of the number of samples in FFT
//     flag      - normalization flag
//     hint      - code specific use hints
//     ppFFTSpec - where write pointer to new context
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       ppFFTSpec == NULL
//     ippStsFftOrderErr      bad the order value
//     ippStsFftFlagErr       bad the normalization flag value
//     ippStsMemAllocErr      memory allocation error
*/

IPPAPI (IppStatus, ippsFFTInitAlloc_C_32fc,
                   ( IppsFFTSpec_C_32fc** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsFFTInitAlloc_C_32f,
                   ( IppsFFTSpec_C_32f** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsFFTInitAlloc_R_32f,
                   ( IppsFFTSpec_R_32f** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint ))

IPPAPI (IppStatus, ippsFFTInitAlloc_C_64fc,
                   ( IppsFFTSpec_C_64fc** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsFFTInitAlloc_C_64f,
                   ( IppsFFTSpec_C_64f** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsFFTInitAlloc_R_64f,
                   ( IppsFFTSpec_R_64f** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint ))

IPPAPI (IppStatus, ippsFFTInitAlloc_C_16sc,
                   ( IppsFFTSpec_C_16sc** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsFFTInitAlloc_C_16s,
                   ( IppsFFTSpec_C_16s** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsFFTInitAlloc_R_16s,
                   ( IppsFFTSpec_R_16s** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint ))

IPPAPI (IppStatus, ippsFFTInitAlloc_C_32sc,
                   ( IppsFFTSpec_C_32sc** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsFFTInitAlloc_C_32s,
                   ( IppsFFTSpec_C_32s** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsFFTInitAlloc_R_32s,
                   ( IppsFFTSpec_R_32s** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint ))

IPPAPI (IppStatus, ippsFFTInitAlloc_R_16s32s,
                   ( IppsFFTSpec_R_16s32s** ppFFTSpec,
                     int order, int flag, IppHintAlgorithm hint ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFFTFree_C, ippsFFTFree_R
//  Purpose:    delete FFT context
//  Arguments:
//     pFFTSpec - pointer to FFT context to be deleted
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pFFTSpec == NULL
//     ippStsContextMatchErr  bad context identifier
*/

IPPAPI (IppStatus, ippsFFTFree_C_32fc,   ( IppsFFTSpec_C_32fc* pFFTSpec ))
IPPAPI (IppStatus, ippsFFTFree_C_32f,    ( IppsFFTSpec_C_32f*  pFFTSpec ))
IPPAPI (IppStatus, ippsFFTFree_R_32f,    ( IppsFFTSpec_R_32f*  pFFTSpec ))

IPPAPI (IppStatus, ippsFFTFree_C_64fc,   ( IppsFFTSpec_C_64fc* pFFTSpec ))
IPPAPI (IppStatus, ippsFFTFree_C_64f,    ( IppsFFTSpec_C_64f*  pFFTSpec ))
IPPAPI (IppStatus, ippsFFTFree_R_64f,    ( IppsFFTSpec_R_64f*  pFFTSpec ))

IPPAPI (IppStatus, ippsFFTFree_C_16sc,   ( IppsFFTSpec_C_16sc* pFFTSpec ))
IPPAPI (IppStatus, ippsFFTFree_C_16s,    ( IppsFFTSpec_C_16s*  pFFTSpec ))
IPPAPI (IppStatus, ippsFFTFree_R_16s,    ( IppsFFTSpec_R_16s*  pFFTSpec ))

IPPAPI (IppStatus, ippsFFTFree_C_32sc,   ( IppsFFTSpec_C_32sc* pFFTSpec ))
IPPAPI (IppStatus, ippsFFTFree_C_32s,    ( IppsFFTSpec_C_32s*  pFFTSpec ))
IPPAPI (IppStatus, ippsFFTFree_R_32s,    ( IppsFFTSpec_R_32s*  pFFTSpec ))

IPPAPI (IppStatus, ippsFFTFree_R_16s32s, ( IppsFFTSpec_R_16s32s* pFFTSpec ))

/* /////////////////////////////////////////////////////////////////////////////
//                  FFT Buffer Size
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFFTGetBufSize_C, ippsFFTGetBufSize_R
//  Purpose:    get size of the FFT work buffer (on bytes)
//  Arguments:
//     pFFTSpec    - pointer to the FFT structure
//     pBufferSize - Pointer to the FFT work buffer size value
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pFFTSpec == NULL or pBufferSize == NULL
//     ippStsContextMatchErr  bad context identifier
*/

IPPAPI (IppStatus, ippsFFTGetBufSize_C_32fc,
                   ( const IppsFFTSpec_C_32fc* pFFTSpec, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetBufSize_C_32f,
                   ( const IppsFFTSpec_C_32f*  pFFTSpec, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetBufSize_R_32f,
                   ( const IppsFFTSpec_R_32f*  pFFTSpec, int* pBufferSize ))

IPPAPI (IppStatus, ippsFFTGetBufSize_C_64fc,
                   ( const IppsFFTSpec_C_64fc* pFFTSpec, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetBufSize_C_64f,
                   ( const IppsFFTSpec_C_64f*  pFFTSpec, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetBufSize_R_64f,
                   ( const IppsFFTSpec_R_64f*  pFFTSpec, int* pBufferSize ))

IPPAPI (IppStatus, ippsFFTGetBufSize_C_16sc,
                   ( const IppsFFTSpec_C_16sc* pFFTSpec, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetBufSize_C_16s,
                   ( const IppsFFTSpec_C_16s*  pFFTSpec, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetBufSize_R_16s,
                   ( const IppsFFTSpec_R_16s*  pFFTSpec, int* pBufferSize ))

IPPAPI (IppStatus, ippsFFTGetBufSize_C_32sc,
                   ( const IppsFFTSpec_C_32sc* pFFTSpec, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetBufSize_C_32s,
                   ( const IppsFFTSpec_C_32s*  pFFTSpec, int* pBufferSize ))
IPPAPI (IppStatus, ippsFFTGetBufSize_R_32s,
                   ( const IppsFFTSpec_R_32s*  pFFTSpec, int* pBufferSize ))

IPPAPI (IppStatus, ippsFFTGetBufSize_R_16s32s,
                   ( const IppsFFTSpec_R_16s32s* pFFTSpec, int* pBufferSize ))

/* /////////////////////////////////////////////////////////////////////////////
//                  FFT Complex Transforms
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFFTFwd_CToC, ippsFFTInv_CToC
//  Purpose:    compute forward and inverse FFT of the complex signal
//  Arguments:
//     pFFTSpec - pointer to FFT context
//     pSrc     - pointer to source complex signal
//     pDst     - pointer to destination complex signal
//     pSrcRe   - pointer to real      part of source signal
//     pSrcIm   - pointer to imaginary part of source signal
//     pDstRe   - pointer to real      part of destination signal
//     pDstIm   - pointer to imaginary part of destination signal
//     pSrcDSt  - pointer to complex signal
//     pSrcDstRe- pointer to real      part of signal
//     pSrcDstIm- pointer to imaginary part of signal
//     pBuffer  - pointer to work buffer
//     scaleFactor
//              - scale factor for output result
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pFFTSpec == NULL or
//                            pSrc == NULL or pDst == NULL or
//                            pSrcRe == NULL or pSrcIm == NULL or
//                            pDstRe == NULL or pDstIm == NULL or
//     ippStsContextMatchErr  bad context identifier
//     ippStsMemAllocErr      memory allocation error
*/

IPPAPI (IppStatus, ippsFFTFwd_CToC_32fc,
                   ( const Ipp32fc* pSrc, Ipp32fc* pDst,
                     const IppsFFTSpec_C_32fc* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_32fc,
                   ( const Ipp32fc* pSrc, Ipp32fc* pDst,
                     const IppsFFTSpec_C_32fc* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_CToC_32f,
                   ( const Ipp32f* pSrcRe, const Ipp32f* pSrcIm,
                     Ipp32f* pDstRe, Ipp32f* pDstIm,
                     const IppsFFTSpec_C_32f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_32f,
                   ( const Ipp32f* pSrcRe, const Ipp32f* pSrcIm,
                     Ipp32f* pDstRe, Ipp32f* pDstIm,
                     const IppsFFTSpec_C_32f* pFFTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_CToC_32fc_I,
                   ( Ipp32fc* pSrcDst,
                     const IppsFFTSpec_C_32fc* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_32fc_I,
                   ( Ipp32fc* pSrcDst,
                     const IppsFFTSpec_C_32fc* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_CToC_32f_I,
                   ( Ipp32f* pSrcDstRe, Ipp32f* pSrcDstIm,
                     const IppsFFTSpec_C_32f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_32f_I,
                   ( Ipp32f* pSrcDstRe, Ipp32f* pSrcDstIm,
                     const IppsFFTSpec_C_32f* pFFTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_CToC_64fc,
                   ( const Ipp64fc* pSrc, Ipp64fc* pDst,
                     const IppsFFTSpec_C_64fc* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_64fc,
                   ( const Ipp64fc* pSrc, Ipp64fc* pDst,
                     const IppsFFTSpec_C_64fc* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_CToC_64f,
                   ( const Ipp64f* pSrcRe, const Ipp64f* pSrcIm,
                     Ipp64f* pDstRe, Ipp64f* pDstIm,
                     const IppsFFTSpec_C_64f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_64f,
                   ( const Ipp64f* pSrcRe, const Ipp64f* pSrcIm,
                     Ipp64f* pDstRe, Ipp64f* pDstIm,
                     const IppsFFTSpec_C_64f* pFFTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_CToC_64fc_I,
                   ( Ipp64fc* pSrcDst,
                     const IppsFFTSpec_C_64fc* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_64fc_I,
                   ( Ipp64fc* pSrcDst,
                     const IppsFFTSpec_C_64fc* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_CToC_64f_I,
                   ( Ipp64f* pSrcDstRe, Ipp64f* pSrcDstIm,
                     const IppsFFTSpec_C_64f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_64f_I,
                   ( Ipp64f* pSrcDstRe, Ipp64f* pSrcDstIm,
                     const IppsFFTSpec_C_64f* pFFTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_CToC_16sc_Sfs,
                   ( const Ipp16sc* pSrc, Ipp16sc* pDst,
                     const IppsFFTSpec_C_16sc* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_16sc_Sfs,
                   ( const Ipp16sc* pSrc, Ipp16sc* pDst,
                     const IppsFFTSpec_C_16sc* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_CToC_16s_Sfs,
                   ( const Ipp16s* pSrcRe, const Ipp16s* pSrcIm,
                     Ipp16s* pDstRe, Ipp16s* pDstIm,
                     const IppsFFTSpec_C_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_16s_Sfs,
                   ( const Ipp16s* pSrcRe, const Ipp16s* pSrcIm,
                     Ipp16s* pDstRe, Ipp16s* pDstIm,
                     const IppsFFTSpec_C_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_CToC_16sc_ISfs,
                   ( Ipp16sc* pSrcDst,
                     const IppsFFTSpec_C_16sc* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_16sc_ISfs,
                   ( Ipp16sc* pSrcDst,
                     const IppsFFTSpec_C_16sc* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_CToC_16s_ISfs,
                   ( Ipp16s* pSrcDstRe, Ipp16s* pSrcDstIm,
                     const IppsFFTSpec_C_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_16s_ISfs,
                   ( Ipp16s* pSrcDstRe, Ipp16s* pSrcDstIm,
                     const IppsFFTSpec_C_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_CToC_32sc_Sfs,
                   ( const Ipp32sc* pSrc, Ipp32sc* pDst,
                     const IppsFFTSpec_C_32sc* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_32sc_Sfs,
                   ( const Ipp32sc* pSrc, Ipp32sc* pDst,
                     const IppsFFTSpec_C_32sc* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_CToC_32s_Sfs,
                   ( const Ipp32s* pSrcRe, const Ipp32s* pSrcIm,
                     Ipp32s* pDstRe, Ipp32s* pDstIm,
                     const IppsFFTSpec_C_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_32s_Sfs,
                   ( const Ipp32s* pSrcRe, const Ipp32s* pSrcIm,
                     Ipp32s* pDstRe, Ipp32s* pDstIm,
                     const IppsFFTSpec_C_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_CToC_32sc_ISfs,
                   ( Ipp32sc* pSrcDst,
                     const IppsFFTSpec_C_32sc* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_32sc_ISfs,
                   ( Ipp32sc* pSrcDst,
                     const IppsFFTSpec_C_32sc* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_CToC_32s_ISfs,
                   ( Ipp32s* pSrcDstRe, Ipp32s* pSrcDstIm,
                     const IppsFFTSpec_C_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CToC_32s_ISfs,
                   ( Ipp32s* pSrcDstRe, Ipp32s* pSrcDstIm,
                     const IppsFFTSpec_C_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))

/* /////////////////////////////////////////////////////////////////////////////
//                  FFT Real Packed Transforms
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFFTFwd_RToPerm, ippsFFTFwd_RToPack, ippsFFTFwd_RToCCS
//              ippsFFTInv_PermToR, ippsFFTInv_PackToR, ippsFFTInv_CCSToR
//  Purpose:    compute forward and inverse FFT of real signal
//              using Perm, Pack or Ccs packed format
//  Arguments:
//     pFFTSpec - pointer to FFT context
//     pSrc     - pointer to source signal
//     pDst     - pointer to destination signal
//     pSrcDst  - pointer to signal
//     pBuffer  - pointer to work buffer
//     scaleFactor
//              - scale factor for output result
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pFFTSpec == NULL or
//                            pSrc == NULL or pDst == NULL
//     ippStsContextMatchErr  bad context identifier
//     ippStsMemAllocErr      memory allocation error
*/

IPPAPI (IppStatus, ippsFFTFwd_RToPerm_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsFFTSpec_R_32f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToPack_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsFFTSpec_R_32f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToCCS_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsFFTSpec_R_32f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PermToR_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsFFTSpec_R_32f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PackToR_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsFFTSpec_R_32f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CCSToR_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsFFTSpec_R_32f* pFFTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_RToPerm_32f_I,
                   ( Ipp32f* pSrcDst,
                     const IppsFFTSpec_R_32f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToPack_32f_I,
                   ( Ipp32f* pSrcDst,
                     const IppsFFTSpec_R_32f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToCCS_32f_I,
                   ( Ipp32f* pSrcDst,
                     const IppsFFTSpec_R_32f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PermToR_32f_I,
                   ( Ipp32f* pSrcDst,
                     const IppsFFTSpec_R_32f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PackToR_32f_I,
                   ( Ipp32f* pSrcDst,
                     const IppsFFTSpec_R_32f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CCSToR_32f_I,
                   ( Ipp32f* pSrcDst,
                     const IppsFFTSpec_R_32f* pFFTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_RToPerm_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsFFTSpec_R_64f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToPack_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsFFTSpec_R_64f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToCCS_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsFFTSpec_R_64f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PermToR_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsFFTSpec_R_64f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PackToR_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsFFTSpec_R_64f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CCSToR_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsFFTSpec_R_64f* pFFTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_RToPerm_64f_I,
                   ( Ipp64f* pSrcDst,
                     const IppsFFTSpec_R_64f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToPack_64f_I,
                   ( Ipp64f* pSrcDst,
                     const IppsFFTSpec_R_64f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToCCS_64f_I,
                   ( Ipp64f* pSrcDst,
                     const IppsFFTSpec_R_64f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PermToR_64f_I,
                   ( Ipp64f* pSrcDst,
                     const IppsFFTSpec_R_64f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PackToR_64f_I,
                   ( Ipp64f* pSrcDst,
                     const IppsFFTSpec_R_64f* pFFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CCSToR_64f_I,
                   ( Ipp64f* pSrcDst,
                     const IppsFFTSpec_R_64f* pFFTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_RToPerm_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsFFTSpec_R_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToPack_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsFFTSpec_R_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToCCS_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsFFTSpec_R_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PermToR_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsFFTSpec_R_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PackToR_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsFFTSpec_R_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CCSToR_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsFFTSpec_R_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_RToPerm_16s_ISfs,
                   ( Ipp16s* pSrcDst,
                     const IppsFFTSpec_R_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToPack_16s_ISfs,
                   ( Ipp16s* pSrcDst,
                     const IppsFFTSpec_R_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToCCS_16s_ISfs,
                   ( Ipp16s* pSrcDst,
                     const IppsFFTSpec_R_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PermToR_16s_ISfs,
                   ( Ipp16s* pSrcDst,
                     const IppsFFTSpec_R_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PackToR_16s_ISfs,
                   ( Ipp16s* pSrcDst,
                     const IppsFFTSpec_R_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CCSToR_16s_ISfs,
                   ( Ipp16s* pSrcDst,
                     const IppsFFTSpec_R_16s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_RToPerm_32s_Sfs,
                   ( const Ipp32s* pSrc, Ipp32s* pDst,
                     const IppsFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToPack_32s_Sfs,
                   ( const Ipp32s* pSrc, Ipp32s* pDst,
                     const IppsFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToCCS_32s_Sfs,
                   ( const Ipp32s* pSrc, Ipp32s* pDst,
                     const IppsFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PermToR_32s_Sfs,
                   ( const Ipp32s* pSrc, Ipp32s* pDst,
                     const IppsFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PackToR_32s_Sfs,
                   ( const Ipp32s* pSrc, Ipp32s* pDst,
                     const IppsFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CCSToR_32s_Sfs,
                   ( const Ipp32s* pSrc, Ipp32s* pDst,
                     const IppsFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_RToPerm_32s_ISfs,
                   ( Ipp32s* pSrcDst,
                     const IppsFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToPack_32s_ISfs,
                   ( Ipp32s* pSrcDst,
                     const IppsFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTFwd_RToCCS_32s_ISfs,
                   ( Ipp32s* pSrcDst,
                     const IppsFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PermToR_32s_ISfs,
                   ( Ipp32s* pSrcDst,
                     const IppsFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_PackToR_32s_ISfs,
                   ( Ipp32s* pSrcDst,
                     const IppsFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CCSToR_32s_ISfs,
                   ( Ipp32s* pSrcDst,
                     const IppsFFTSpec_R_32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsFFTFwd_RToCCS_16s32s_Sfs,
                   ( const Ipp16s* pSrc, Ipp32s* pDst,
                     const IppsFFTSpec_R_16s32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsFFTInv_CCSToR_32s16s_Sfs,
                   ( const Ipp32s* pSrc, Ipp16s* pDst,
                     const IppsFFTSpec_R_16s32s* pFFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))

/* /////////////////////////////////////////////////////////////////////////////
//                  Definitions for DFT Functions
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )

typedef struct DFTSpec_C_16sc       IppsDFTSpec_C_16sc;
typedef struct DFTSpec_C_16s        IppsDFTSpec_C_16s;
typedef struct DFTSpec_R_16s        IppsDFTSpec_R_16s;

typedef struct DFTSpec_C_32fc       IppsDFTSpec_C_32fc;
typedef struct DFTSpec_C_32f        IppsDFTSpec_C_32f;
typedef struct DFTSpec_R_32f        IppsDFTSpec_R_32f;

typedef struct DFTSpec_C_64fc       IppsDFTSpec_C_64fc;
typedef struct DFTSpec_C_64f        IppsDFTSpec_C_64f;
typedef struct DFTSpec_R_64f        IppsDFTSpec_R_64f;

typedef struct DFTOutOrdSpec_C_32fc IppsDFTOutOrdSpec_C_32fc;

typedef struct DFTOutOrdSpec_C_64fc IppsDFTOutOrdSpec_C_64fc;

#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
//                  DFT Context Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDFTInitAlloc_C, ippsDFTInitAlloc_R
//  Purpose:    create and initialize of DFT context
//  Arguments:
//     length   - number of samples in DFT
//     flag     - normalization flag
//     hint     - code specific use hints
//     pDFTSpec - where write pointer to new context
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pDFTSpec == NULL
//     ippStsSizeErr          bad the length value
//     ippStsFFTFlagErr       bad the normalization flag value
//     ippStsMemAllocErr      memory allocation error
*/

IPPAPI (IppStatus, ippsDFTInitAlloc_C_16sc,
                   ( IppsDFTSpec_C_16sc** pDFTSpec,
                     int length, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsDFTInitAlloc_C_16s,
                   ( IppsDFTSpec_C_16s** pDFTSpec,
                     int length, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsDFTInitAlloc_R_16s,
                   ( IppsDFTSpec_R_16s** pDFTSpec,
                     int length, int flag, IppHintAlgorithm hint ))

IPPAPI (IppStatus, ippsDFTInitAlloc_C_32fc,
                   ( IppsDFTSpec_C_32fc** pDFTSpec,
                     int length, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsDFTInitAlloc_C_32f,
                   ( IppsDFTSpec_C_32f** pDFTSpec,
                     int length, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsDFTInitAlloc_R_32f,
                   ( IppsDFTSpec_R_32f** pDFTSpec,
                     int length, int flag, IppHintAlgorithm hint ))

IPPAPI (IppStatus, ippsDFTInitAlloc_C_64fc,
                   ( IppsDFTSpec_C_64fc** pDFTSpec,
                     int length, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsDFTInitAlloc_C_64f,
                   ( IppsDFTSpec_C_64f** pDFTSpec,
                     int length, int flag, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsDFTInitAlloc_R_64f,
                   ( IppsDFTSpec_R_64f** pDFTSpec,
                     int length, int flag, IppHintAlgorithm hint ))

IPPAPI (IppStatus, ippsDFTOutOrdInitAlloc_C_32fc,
                   ( IppsDFTOutOrdSpec_C_32fc** pDFTSpec,
                     int length, int flag, IppHintAlgorithm hint ))

IPPAPI (IppStatus, ippsDFTOutOrdInitAlloc_C_64fc,
                   ( IppsDFTOutOrdSpec_C_64fc** pDFTSpec,
                     int length, int flag, IppHintAlgorithm hint ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDFTFree_C, ippsDFTFree_R
//  Purpose:    delete DFT context
//  Arguments:
//     pDFTSpec - pointer to DFT context to be deleted
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pDFTSpec == NULL
//     ippStsContextMatchErr  bad context identifier
*/

IPPAPI (IppStatus, ippsDFTFree_C_16sc, ( IppsDFTSpec_C_16sc* pDFTSpec ))
IPPAPI (IppStatus, ippsDFTFree_C_16s,  ( IppsDFTSpec_C_16s*  pDFTSpec ))
IPPAPI (IppStatus, ippsDFTFree_R_16s,  ( IppsDFTSpec_R_16s*  pDFTSpec ))

IPPAPI (IppStatus, ippsDFTFree_C_32fc, ( IppsDFTSpec_C_32fc* pDFTSpec ))
IPPAPI (IppStatus, ippsDFTFree_C_32f,  ( IppsDFTSpec_C_32f*  pDFTSpec ))
IPPAPI (IppStatus, ippsDFTFree_R_32f,  ( IppsDFTSpec_R_32f*  pDFTSpec ))

IPPAPI (IppStatus, ippsDFTFree_C_64fc, ( IppsDFTSpec_C_64fc* pDFTSpec ))
IPPAPI (IppStatus, ippsDFTFree_C_64f,  ( IppsDFTSpec_C_64f*  pDFTSpec ))
IPPAPI (IppStatus, ippsDFTFree_R_64f,  ( IppsDFTSpec_R_64f*  pDFTSpec ))

IPPAPI (IppStatus, ippsDFTOutOrdFree_C_32fc, ( IppsDFTOutOrdSpec_C_32fc* pDFTSpec ))

IPPAPI (IppStatus, ippsDFTOutOrdFree_C_64fc, ( IppsDFTOutOrdSpec_C_64fc* pDFTSpec ))


/* /////////////////////////////////////////////////////////////////////////////
//                  DFT Buffer Size
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDFTGetBufSize_C, ippsDFTGetBufSize_R
//  Purpose:    get size of the DFT work buffer (on bytes)
//  Arguments:
//     pDFTSpec - pointer to DFT context
//     pSize     - where write size of buffer
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pDFTSpec == NULL or pSize == NULL
//     ippStsContextMatchErr  bad context identifier
*/

IPPAPI (IppStatus, ippsDFTGetBufSize_C_16sc,
                   ( const IppsDFTSpec_C_16sc* pDFTSpec, int* pSize ))
IPPAPI (IppStatus, ippsDFTGetBufSize_C_16s,
                   ( const IppsDFTSpec_C_16s*  pDFTSpec, int* pSize ))
IPPAPI (IppStatus, ippsDFTGetBufSize_R_16s,
                   ( const IppsDFTSpec_R_16s*  pDFTSpec, int* pSize ))

IPPAPI (IppStatus, ippsDFTGetBufSize_C_32fc,
                   ( const IppsDFTSpec_C_32fc* pDFTSpec, int* pSize ))
IPPAPI (IppStatus, ippsDFTGetBufSize_C_32f,
                   ( const IppsDFTSpec_C_32f*  pDFTSpec, int* pSize ))
IPPAPI (IppStatus, ippsDFTGetBufSize_R_32f,
                   ( const IppsDFTSpec_R_32f*  pDFTSpec, int* pSize ))

IPPAPI (IppStatus, ippsDFTGetBufSize_C_64fc,
                   ( const IppsDFTSpec_C_64fc* pDFTSpec, int* pSize ))
IPPAPI (IppStatus, ippsDFTGetBufSize_C_64f,
                   ( const IppsDFTSpec_C_64f*  pDFTSpec, int* pSize ))
IPPAPI (IppStatus, ippsDFTGetBufSize_R_64f,
                   ( const IppsDFTSpec_R_64f*  pDFTSpec, int* pSize ))

IPPAPI (IppStatus, ippsDFTOutOrdGetBufSize_C_32fc,
                   ( const IppsDFTOutOrdSpec_C_32fc* pDFTSpec, int* size ))

IPPAPI (IppStatus, ippsDFTOutOrdGetBufSize_C_64fc,
                   ( const IppsDFTOutOrdSpec_C_64fc* pDFTSpec, int* size ))


/* /////////////////////////////////////////////////////////////////////////////
//                  DFT Complex Transforms
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDFTFwd_CToC, ippsDFTInv_CToC
//  Purpose:    compute forward and inverse DFT of the complex signal
//  Arguments:
//     pDFTSpec - pointer to DFT context
//     pSrc     - pointer to source complex signal
//     pDst     - pointer to destination complex signal
//     pSrcRe   - pointer to real      part of source signal
//     pSrcIm   - pointer to imaginary part of source signal
//     pDstRe   - pointer to real      part of destination signal
//     pDstIm   - pointer to imaginary part of destination signal
//     pBuffer  - pointer to work buffer
//     scaleFactor
//              - scale factor for output result
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pDFTSpec == NULL or
//                            pSrc == NULL or pDst == NULL or
//                            pSrcRe == NULL or pSrcIm == NULL or
//                            pDstRe == NULL or pDstIm == NULL or
//     ippStsContextMatchErr  bad context identifier
//     ippStsMemAllocErr      memory allocation error
*/

IPPAPI (IppStatus, ippsDFTFwd_CToC_16sc_Sfs,
                   ( const Ipp16sc* pSrc, Ipp16sc* pDst,
                     const IppsDFTSpec_C_16sc* pDFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_CToC_16sc_Sfs,
                   ( const Ipp16sc* pSrc, Ipp16sc* pDst,
                     const IppsDFTSpec_C_16sc* pDFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTFwd_CToC_16s_Sfs,
                   ( const Ipp16s* pSrcRe, const Ipp16s* pSrcIm,
                     Ipp16s* pDstRe, Ipp16s* pDstIm,
                     const IppsDFTSpec_C_16s* pDFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_CToC_16s_Sfs,
                   ( const Ipp16s* pSrcRe, const Ipp16s* pSrcIm,
                     Ipp16s* pDstRe, Ipp16s* pDstIm,
                     const IppsDFTSpec_C_16s* pDFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsDFTFwd_CToC_32fc,
                   ( const Ipp32fc* pSrc, Ipp32fc* pDst,
                     const IppsDFTSpec_C_32fc* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_CToC_32fc,
                   ( const Ipp32fc* pSrc, Ipp32fc* pDst,
                     const IppsDFTSpec_C_32fc* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTFwd_CToC_32f,
                   ( const Ipp32f* pSrcRe, const Ipp32f* pSrcIm,
                     Ipp32f* pDstRe, Ipp32f* pDstIm,
                     const IppsDFTSpec_C_32f* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_CToC_32f,
                   ( const Ipp32f* pSrcRe, const Ipp32f* pSrcIm,
                     Ipp32f* pDstRe, Ipp32f* pDstIm,
                     const IppsDFTSpec_C_32f* pDFTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsDFTFwd_CToC_64fc,
                   ( const Ipp64fc* pSrc, Ipp64fc* pDst,
                     const IppsDFTSpec_C_64fc* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_CToC_64fc,
                   ( const Ipp64fc* pSrc, Ipp64fc* pDst,
                     const IppsDFTSpec_C_64fc* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTFwd_CToC_64f,
                   ( const Ipp64f* pSrcRe, const Ipp64f* pSrcIm,
                     Ipp64f* pDstRe, Ipp64f* pDstIm,
                     const IppsDFTSpec_C_64f* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_CToC_64f,
                   ( const Ipp64f* pSrcRe, const Ipp64f* pSrcIm,
                     Ipp64f* pDstRe, Ipp64f* pDstIm,
                     const IppsDFTSpec_C_64f* pDFTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsDFTOutOrdFwd_CToC_32fc,
                   ( const Ipp32fc* pSrc, Ipp32fc* pDst,
                     const IppsDFTOutOrdSpec_C_32fc* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTOutOrdInv_CToC_32fc,
                   ( const Ipp32fc* pSrc, Ipp32fc* pDst,
                     const IppsDFTOutOrdSpec_C_32fc* pDFTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsDFTOutOrdFwd_CToC_64fc,
                   ( const Ipp64fc* pSrc, Ipp64fc* pDst,
                     const IppsDFTOutOrdSpec_C_64fc* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTOutOrdInv_CToC_64fc,
                   ( const Ipp64fc* pSrc, Ipp64fc* pDst,
                     const IppsDFTOutOrdSpec_C_64fc* pDFTSpec, Ipp8u* pBuffer ))


/* /////////////////////////////////////////////////////////////////////////////
//                  DFT Real Packed Transforms
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDFTFwd_RToPerm, ippsDFTFwd_RToPack, ippsDFTFwd_RToCCS
//              ippsDFTInv_PermToR, ippsDFTInv_PackToR, ippsDFTInv_CCSToR
//  Purpose:    compute forward and inverse DFT of real signal
//              using Perm, Pack or Ccs packed format
//  Arguments:
//     pDFTSpec - pointer to DFT context
//     pSrc     - pointer to source signal
//     pDst     - pointer to destination signal
//     pBuffer  - pointer to work buffer
//     scaleFactor
//              - scale factor for output result
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pDFTSpec == NULL or
//                            pSrc == NULL or pDst == NULL
//     ippStsContextMatchErr  bad context identifier
//     ippStsMemAllocErr      memory allocation error
*/

IPPAPI (IppStatus, ippsDFTFwd_RToPerm_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsDFTSpec_R_16s* pDFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTFwd_RToPack_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsDFTSpec_R_16s* pDFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTFwd_RToCCS_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsDFTSpec_R_16s* pDFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_PermToR_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsDFTSpec_R_16s* pDFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_PackToR_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsDFTSpec_R_16s* pDFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_CCSToR_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsDFTSpec_R_16s* pDFTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsDFTFwd_RToPerm_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsDFTSpec_R_32f* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTFwd_RToPack_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsDFTSpec_R_32f* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTFwd_RToCCS_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsDFTSpec_R_32f* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_PermToR_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsDFTSpec_R_32f* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_PackToR_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsDFTSpec_R_32f* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_CCSToR_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsDFTSpec_R_32f* pDFTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsDFTFwd_RToPerm_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsDFTSpec_R_64f* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTFwd_RToPack_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsDFTSpec_R_64f* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTFwd_RToCCS_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsDFTSpec_R_64f* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_PermToR_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsDFTSpec_R_64f* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_PackToR_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsDFTSpec_R_64f* pDFTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDFTInv_CCSToR_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsDFTSpec_R_64f* pDFTSpec, Ipp8u* pBuffer ))


/* /////////////////////////////////////////////////////////////////////////////
//              Vector multiplication in RCPack and in RCPerm formats
///////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////////
//  Names:              ippsMulPack, ippsMulPerm
//  Purpose:            multiply two vectors stored in RCPack and RCPerm formats
//  Parameters:
//   pSrc               pointer to input vector (in-place case)
//   pSrcDst            pointer to output vector (in-place case)
//   pSrc1              pointer to first input vector
//   pSrc2              pointer to second input vector
//   pDst               pointer to output vector
//   length             vector's length
//   scaleFactor        scale factor
//  Return:
//   ippStsNullPtrErr      pointer(s) to the data is NULL
//   ippStsSizeErr         vector`s length is less or equal zero
//   ippStsNoErr           otherwise
*/

IPPAPI(IppStatus, ippsMulPack_16s_ISfs, (const Ipp16s* pSrc, Ipp16s* pSrcDst, int length, int scaleFactor))
IPPAPI(IppStatus, ippsMulPerm_16s_ISfs, (const Ipp16s* pSrc, Ipp16s* pSrcDst, int length, int scaleFactor))
IPPAPI(IppStatus, ippsMulPack_32f_I, (const Ipp32f* pSrc, Ipp32f* pSrcDst, int length))
IPPAPI(IppStatus, ippsMulPerm_32f_I, (const Ipp32f* pSrc, Ipp32f* pSrcDst, int length))
IPPAPI(IppStatus, ippsMulPack_64f_I, (const Ipp64f* pSrc, Ipp64f* pSrcDst, int length))
IPPAPI(IppStatus, ippsMulPerm_64f_I, (const Ipp64f* pSrc, Ipp64f* pSrcDst, int length))
IPPAPI(IppStatus, ippsMulPack_16s_Sfs, (const Ipp16s* pSrc1, const Ipp16s* pSrc2, Ipp16s* pDst, int length, int scaleFactor))
IPPAPI(IppStatus, ippsMulPerm_16s_Sfs, (const Ipp16s* pSrc1, const Ipp16s* pSrc2, Ipp16s* pDst, int length, int scaleFactor))
IPPAPI(IppStatus, ippsMulPack_32f, (const Ipp32f* pSrc1, const Ipp32f* pSrc2, Ipp32f* pDst, int length))
IPPAPI(IppStatus, ippsMulPerm_32f, (const Ipp32f* pSrc1, const Ipp32f* pSrc2, Ipp32f* pDst, int length))
IPPAPI(IppStatus, ippsMulPack_64f, (const Ipp64f* pSrc1, const Ipp64f* pSrc2, Ipp64f* pDst, int length))
IPPAPI(IppStatus, ippsMulPerm_64f, (const Ipp64f* pSrc1, const Ipp64f* pSrc2, Ipp64f* pDst, int length))

/* ////////////////////////////////////////////////////////////////////////////
//  Names:              ippsMulPackConj
//  Purpose:            multiply on a complex conjugate vector and store in RCPack format
//  Parameters:
//   pSrc               pointer to input vector (in-place case)
//   pSrcDst            pointer to output vector (in-place case)
//   length             vector's length
//  Return:
//   ippStsNullPtrErr      pointer(s) to the data is NULL
//   ippStsSizeErr         vector`s length is less or equal zero
//   ippStsNoErr           otherwise
*/

IPPAPI(IppStatus, ippsMulPackConj_32f_I, (const Ipp32f* pSrc, Ipp32f* pSrcDst, int length))
IPPAPI(IppStatus, ippsMulPackConj_64f_I, (const Ipp64f* pSrc, Ipp64f* pSrcDst, int length))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:        ippsGoertz
//  Purpose:      compute DFT for single frequency (Goertzel algorithm)
//  Parameters:
//    freq                 single relative frequency value [0, 1.0)
//    pSrc                 pointer to the input vector
//    len                  length of the vector
//    pVal                 pointer to the DFT result value computed
//    scaleFactor          scale factor value
//  Return:
//    ippStsNullPtrErr        pointer to the data is NULL
//    ippStsSizeErr           length of the vector is less or equal zero
//    ippStsRelFreqErr        frequency value out of range
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippsGoertz_32fc, (const Ipp32fc* pSrc, int len, Ipp32fc* pVal, Ipp32f rFreq))
IPPAPI(IppStatus, ippsGoertz_64fc, (const Ipp64fc* pSrc, int len, Ipp64fc* pVal, Ipp64f rFreq))

IPPAPI(IppStatus, ippsGoertz_16sc_Sfs, (const Ipp16sc* pSrc, int len, Ipp16sc* pVal, Ipp32f rFreq, int scaleFactor))

IPPAPI(IppStatus, ippsGoertz_32f, (const Ipp32f* pSrc, int len, Ipp32fc* pVal, Ipp32f rFreq))
IPPAPI(IppStatus, ippsGoertz_16s_Sfs, (const Ipp16s* pSrc, int len, Ipp16sc* pVal, Ipp32f rFreq, int scaleFactor))
IPPAPI(IppStatus, ippsGoertz_64f, (const Ipp64f* pSrc, int len, Ipp64fc* pVal, Ipp64f rFreq))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:        ippsGoertzTwo
//  Purpose:      compute DFT for dual frequency (Goertzel algorithm)
//  Parameters:
//    freq                 pointer to two relative frequency values [0, 1.0)
//    pSrc                 pointer to the input vector
//    len                  length of the vector
//    pVal                 pointer to the DFT result value computed
//    scaleFactor          scale factor value
//  Return:
//    ippStsNullPtrErr        pointer to the data is NULL
//    ippStsSizeErr           length of the vector is less or equal zero
//    ippStsRelFreqErr        frequency values out of range
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippsGoertzTwo_32fc, (const Ipp32fc* pSrc, int len,
       Ipp32fc val[2], const Ipp32f rFreq[2] ))
IPPAPI(IppStatus, ippsGoertzTwo_64fc, (const Ipp64fc* pSrc, int len,
       Ipp64fc val[2], const Ipp64f rFreq[2] ))
IPPAPI(IppStatus, ippsGoertzTwo_16sc_Sfs, (const Ipp16sc* pSrc, int len,
       Ipp16sc val[2], const Ipp32f rFreq[2], int scaleFactor))
IPPAPI(IppStatus, ippsGoertzTwo_32f, (const Ipp32f* pSrc, int len,
       Ipp32fc val[2], const Ipp32f rFreq[2] ))
IPPAPI(IppStatus, ippsGoertzTwo_64f, (const Ipp64f* pSrc, int len,
       Ipp64fc val[2], const Ipp64f rFreq[2] ))



/* /////////////////////////////////////////////////////////////////////////////
//  Names:        ippsGoertzQ15
//  Purpose:      compute DFT for single frequency (Goertzel algorithm)
//  Parameters:
//    rFreqQ15             single relative frequency value [0, 32767]
//    pSrc                 pointer to the input vector
//    len                  length of the vector
//    pVal                 pointer to the DFT result value computed
//    scaleFactor          scale factor value
//  Return:
//    ippStsNullPtrErr        pointer to the data is NULL
//    ippStsSizeErr           length of the vector is less or equal zero
//    ippStsRelFreqErr        frequency value out of range
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippsGoertzQ15_16sc_Sfs,
                  ( const Ipp16sc* pSrc, int len, Ipp16sc* pVal,
                    Ipp16s rFreqQ15, int scaleFactor ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:        ippsGoertzTwoQ15
//  Purpose:      compute DFT for dual frequency (Goertzel algorithm)
//  Parameters:
//    rFreqQ15             pointer to two relative frequency values [0, 32767]
//    pSrc                 pointer to the input vector
//    len                  length of the vector
//    pVal                 pointer to the DFT result value computed
//    scaleFactor          scale factor value
//  Return:
//    ippStsNullPtrErr        pointer to the data is NULL
//    ippStsSizeErr           length of the vector is less or equal zero
//    ippStsRelFreqErr        frequency values out of range
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippsGoertzTwoQ15_16sc_Sfs,
                  ( const Ipp16sc* pSrc, int len, Ipp16sc pVal[2],
                    const Ipp16s rFreqQ15[2], int scaleFactor ))


/* /////////////////////////////////////////////////////////////////////////////
//                  Definitions for DCT Functions
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )

typedef struct DCTFwdSpec_16s IppsDCTFwdSpec_16s;
typedef struct DCTInvSpec_16s IppsDCTInvSpec_16s;

typedef struct DCTFwdSpec_32f IppsDCTFwdSpec_32f;
typedef struct DCTInvSpec_32f IppsDCTInvSpec_32f;

typedef struct DCTFwdSpec_64f IppsDCTFwdSpec_64f;
typedef struct DCTInvSpec_64f IppsDCTInvSpec_64f;

#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
//                  DCT Get Size Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDCTFwdGetSize, ippsDCTInvGetSize
//  Purpose:    get sizes of the DCTSpec and buffers (on bytes)
//  Arguments:
//     len             - number of samples in DCT
//     hint            - code specific use hints
//     pSpecSize       - where write size of DCTSpec
//     pSpecBufferSize - where write size of buffer for DCTInit functions
//     pBufferSize     - where write size of buffer for DCT calculation
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pSpecSize == NULL or pSpecBufferSize == NULL or
//                            pBufferSize == NULL
//     ippStsSizeErr          bad the len value
*/

IPPAPI (IppStatus, ippsDCTFwdGetSize_16s,
                   ( int len, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))
IPPAPI (IppStatus, ippsDCTInvGetSize_16s,
                   ( int len, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))

IPPAPI (IppStatus, ippsDCTFwdGetSize_32f,
                   ( int len, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))
IPPAPI (IppStatus, ippsDCTInvGetSize_32f,
                   ( int len, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))

IPPAPI (IppStatus, ippsDCTFwdGetSize_64f,
                   ( int len, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))
IPPAPI (IppStatus, ippsDCTInvGetSize_64f,
                   ( int len, IppHintAlgorithm hint,
                     int* pSpecSize, int* pSpecBufferSize, int* pBufferSize ))


/* /////////////////////////////////////////////////////////////////////////////
//                  DCT Context Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDCTFwdInit, ippsDCTInvInit
//  Purpose:    initialize of DCT context
//  Arguments:
//     len         - number of samples in DCT
//     hint        - code specific use hints
//     ppDCTSpec   - where write pointer to new context
//     pSpec       - pointer to area for DCTSpec
//     pSpecBuffer - pointer to work buffer
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       ppDCTSpec == NULL or
//                            pSpec == NULL or pMemInit == NULL
//     ippStsSizeErr          bad the len value
*/

IPPAPI (IppStatus, ippsDCTFwdInit_16s,
                   ( IppsDCTFwdSpec_16s** ppDCTSpec,
                     int len, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))
IPPAPI (IppStatus, ippsDCTInvInit_16s,
                   ( IppsDCTInvSpec_16s** ppDCTSpec,
                     int len, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))

IPPAPI (IppStatus, ippsDCTFwdInit_32f,
                   ( IppsDCTFwdSpec_32f** ppDCTSpec,
                     int len, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))
IPPAPI (IppStatus, ippsDCTInvInit_32f,
                   ( IppsDCTInvSpec_32f** ppDCTSpec,
                     int len, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))

IPPAPI (IppStatus, ippsDCTFwdInit_64f,
                   ( IppsDCTFwdSpec_64f** ppDCTSpec,
                     int len, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))
IPPAPI (IppStatus, ippsDCTInvInit_64f,
                   ( IppsDCTInvSpec_64f** ppDCTSpec,
                     int len, IppHintAlgorithm hint,
                     Ipp8u* pSpec, Ipp8u* pSpecBuffer ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDCTFwdInitAlloc, ippsDCTInvInitAlloc
//  Purpose:    create and initialize of DCT context
//  Arguments:
//     len       - number of samples in DCT
//     hint      - code specific use hints
//     ppDCTSpec - where write pointer to new context
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       ppDCTSpec == NULL
//     ippStsSizeErr          bad the len value
//     ippStsMemAllocErr      memory allocation error
*/

IPPAPI (IppStatus, ippsDCTFwdInitAlloc_16s,
                   ( IppsDCTFwdSpec_16s** ppDCTSpec,
                     int len, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsDCTInvInitAlloc_16s,
                   ( IppsDCTInvSpec_16s** ppDCTSpec,
                     int len, IppHintAlgorithm hint ))

IPPAPI (IppStatus, ippsDCTFwdInitAlloc_32f,
                   ( IppsDCTFwdSpec_32f** ppDCTSpec,
                     int len, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsDCTInvInitAlloc_32f,
                   ( IppsDCTInvSpec_32f** ppDCTSpec,
                     int len, IppHintAlgorithm hint ))

IPPAPI (IppStatus, ippsDCTFwdInitAlloc_64f,
                   ( IppsDCTFwdSpec_64f** ppDCTSpec,
                     int len, IppHintAlgorithm hint ))
IPPAPI (IppStatus, ippsDCTInvInitAlloc_64f,
                   ( IppsDCTInvSpec_64f** ppDCTSpec,
                     int len, IppHintAlgorithm hint ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDCTFwdFree, ippsDCTInvFree
//  Purpose:    delete DCT context
//  Arguments:
//     pDCTSpec - pointer to DCT context to be deleted
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pDCTSpec == NULL
//     ippStsContextMatchErr  bad context identifier
*/

IPPAPI (IppStatus, ippsDCTFwdFree_16s, ( IppsDCTFwdSpec_16s*  pDCTSpec ))
IPPAPI (IppStatus, ippsDCTInvFree_16s, ( IppsDCTInvSpec_16s*  pDCTSpec ))

IPPAPI (IppStatus, ippsDCTFwdFree_32f, ( IppsDCTFwdSpec_32f*  pDCTSpec ))
IPPAPI (IppStatus, ippsDCTInvFree_32f, ( IppsDCTInvSpec_32f*  pDCTSpec ))

IPPAPI (IppStatus, ippsDCTFwdFree_64f, ( IppsDCTFwdSpec_64f*  pDCTSpec ))
IPPAPI (IppStatus, ippsDCTInvFree_64f, ( IppsDCTInvSpec_64f*  pDCTSpec ))


/* /////////////////////////////////////////////////////////////////////////////
//                  DCT Buffer Size
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDCTFwdGetBufSize, ippsDCTInvGetBufSize
//  Purpose:    get size of the DCT work buffer (on bytes)
//  Arguments:
//     pDCTSpec    - pointer to the DCT structure
//     pBufferSize - pointer to the DCT work buffer size value
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pDCTSpec == NULL or pSize == NULL
//     ippStsContextMatchErr  bad context identifier
*/

IPPAPI (IppStatus, ippsDCTFwdGetBufSize_16s,
                   ( const IppsDCTFwdSpec_16s* pDCTSpec, int* pBufferSize ))
IPPAPI (IppStatus, ippsDCTInvGetBufSize_16s,
                   ( const IppsDCTInvSpec_16s* pDCTSpec, int* pBufferSize ))

IPPAPI (IppStatus, ippsDCTFwdGetBufSize_32f,
                   ( const IppsDCTFwdSpec_32f* pDCTSpec, int* pBufferSize ))
IPPAPI (IppStatus, ippsDCTInvGetBufSize_32f,
                   ( const IppsDCTInvSpec_32f* pDCTSpec, int* pBufferSize ))

IPPAPI (IppStatus, ippsDCTFwdGetBufSize_64f,
                   ( const IppsDCTFwdSpec_64f* pDCTSpec, int* pBufferSize ))
IPPAPI (IppStatus, ippsDCTInvGetBufSize_64f,
                   ( const IppsDCTInvSpec_64f* pDCTSpec, int* pBufferSize ))


/* /////////////////////////////////////////////////////////////////////////////
//                  DCT Transforms
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDCTFwd, ippsDCTInv
//  Purpose:    compute forward and inverse DCT of signal
//  Arguments:
//     pDCTSpec - pointer to DCT context
//     pSrc     - pointer to source signal
//     pDst     - pointer to destination signal
//     pSrcDst  - pointer to signal
//     pBuffer  - pointer to work buffer
//     scaleFactor
//              - scale factor for output result
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pDCTSpec == NULL or
//                            pSrc == NULL or pDst == NULL or pSrcDst == NULL
//     ippStsContextMatchErr  bad context identifier
//     ippStsMemAllocErr      memory allocation error
*/

IPPAPI (IppStatus, ippsDCTFwd_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsDCTFwdSpec_16s* pDCTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDCTInv_16s_Sfs,
                   ( const Ipp16s* pSrc, Ipp16s* pDst,
                     const IppsDCTInvSpec_16s* pDCTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsDCTFwd_16s_ISfs,
                   ( Ipp16s* pSrcDst,
                     const IppsDCTFwdSpec_16s* pDCTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDCTInv_16s_ISfs,
                   ( Ipp16s* pSrcDst,
                     const IppsDCTInvSpec_16s* pDCTSpec,
                     int scaleFactor, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsDCTFwd_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsDCTFwdSpec_32f* pDCTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDCTInv_32f,
                   ( const Ipp32f* pSrc, Ipp32f* pDst,
                     const IppsDCTInvSpec_32f* pDCTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsDCTFwd_32f_I,
                   ( Ipp32f* pSrcDst,
                     const IppsDCTFwdSpec_32f* pDCTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDCTInv_32f_I,
                   ( Ipp32f* pSrcDst,
                     const IppsDCTInvSpec_32f* pDCTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsDCTFwd_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsDCTFwdSpec_64f* pDCTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDCTInv_64f,
                   ( const Ipp64f* pSrc, Ipp64f* pDst,
                     const IppsDCTInvSpec_64f* pDCTSpec, Ipp8u* pBuffer ))

IPPAPI (IppStatus, ippsDCTFwd_64f_I,
                   ( Ipp64f* pSrcDst,
                     const IppsDCTFwdSpec_64f* pDCTSpec, Ipp8u* pBuffer ))
IPPAPI (IppStatus, ippsDCTInv_64f_I,
                   ( Ipp64f* pSrDst,
                     const IppsDCTInvSpec_64f* pDCTSpec, Ipp8u* pBuffer ))


/* /////////////////////////////////////////////////////////////////////////////
//          Wavelet Transform Functions for Fixed Filter Banks
///////////////////////////////////////////////////////////////////////////// */
/* //////////////////////////////////////////////////////////////////////
// Name:       ippsWTHaar
// Purpose:    one level Haar Wavelet Transform
// Arguments:
//   pSrc        - source vector;
//   len         - length of source vector;
//   pDstLow     - coarse "low frequency" component destination;
//   pDstHigh    - detail "high frequency" component destination;
//   pSrcLow     - coarse "low frequency" component source;
//   pSrcHigh    - detail "high frequency" component source;
//   pDst        - destination vector;
//   scaleFactor - scale factor value
//  Return:
//   ippStsNullPtrErr    pointer(s) to the data vector is NULL
//   ippStsSizeErr       the length is less or equal zero
//   ippStsNoErr         otherwise
*/

IPPAPI (IppStatus, ippsWTHaarFwd_8s,
                   ( const Ipp8s* pSrc, int len,
                        Ipp8s* pDstLow, Ipp8s* pDstHigh ))
IPPAPI (IppStatus, ippsWTHaarFwd_16s,
                   ( const Ipp16s* pSrc, int len,
                        Ipp16s* pDstLow, Ipp16s* pDstHigh ))
IPPAPI (IppStatus, ippsWTHaarFwd_32s,
                   ( const Ipp32s* pSrc, int len,
                        Ipp32s* pDstLow, Ipp32s* pDstHigh ))
IPPAPI (IppStatus, ippsWTHaarFwd_64s,
                   ( const Ipp64s* pSrc, int len,
                        Ipp64s* pDstLow, Ipp64s* pDstHigh ))
IPPAPI (IppStatus, ippsWTHaarFwd_32f,
                   ( const Ipp32f* pSrc, int len,
                        Ipp32f* pDstLow, Ipp32f* pDstHigh ))
IPPAPI (IppStatus, ippsWTHaarFwd_64f,
                   ( const Ipp64f* pSrc, int len,
                        Ipp64f* pDstLow, Ipp64f* pDstHigh ))

IPPAPI (IppStatus, ippsWTHaarFwd_8s_Sfs,
                   ( const Ipp8s* pSrc, int len,
                        Ipp8s* pDstLow, Ipp8s* pDstHigh, int scaleFactor))
IPPAPI (IppStatus, ippsWTHaarFwd_16s_Sfs,
                   ( const Ipp16s* pSrc, int len,
                        Ipp16s* pDstLow, Ipp16s* pDstHigh, int scaleFactor ))
IPPAPI (IppStatus, ippsWTHaarFwd_32s_Sfs,
                   ( const Ipp32s* pSrc, int len,
                        Ipp32s* pDstLow, Ipp32s* pDstHigh, int scaleFactor ))
IPPAPI (IppStatus, ippsWTHaarFwd_64s_Sfs,
                   ( const Ipp64s* pSrc, int len,
                        Ipp64s* pDstLow, Ipp64s* pDstHigh, int scaleFactor ))

IPPAPI (IppStatus, ippsWTHaarInv_8s,
                   ( const Ipp8s* pSrcLow, const Ipp8s* pSrcHigh,
                                Ipp8s* pDst, int len ))
IPPAPI (IppStatus, ippsWTHaarInv_16s,
                   ( const Ipp16s* pSrcLow, const Ipp16s* pSrcHigh,
                                Ipp16s* pDst, int len ))
IPPAPI (IppStatus, ippsWTHaarInv_32s,
                   ( const Ipp32s* pSrcLow, const Ipp32s* pSrcHigh,
                                Ipp32s* pDst, int len ))
IPPAPI (IppStatus, ippsWTHaarInv_64s,
                   ( const Ipp64s* pSrcLow, const Ipp64s* pSrcHigh,
                                Ipp64s* pDst, int len ))
IPPAPI (IppStatus, ippsWTHaarInv_32f,
                   ( const Ipp32f* pSrcLow, const Ipp32f* pSrcHigh,
                                Ipp32f* pDst, int len ))
IPPAPI (IppStatus, ippsWTHaarInv_64f,
                   ( const Ipp64f* pSrcLow, const Ipp64f* pSrcHigh,
                                Ipp64f* pDst, int len ))

IPPAPI (IppStatus, ippsWTHaarInv_8s_Sfs,
                   ( const Ipp8s* pSrcLow, const Ipp8s* pSrcHigh,
                                Ipp8s* pDst, int len, int scaleFactor ))
IPPAPI (IppStatus, ippsWTHaarInv_16s_Sfs,
                   ( const Ipp16s* pSrcLow, const Ipp16s* pSrcHigh,
                                Ipp16s* pDst, int len, int scaleFactor ))
IPPAPI (IppStatus, ippsWTHaarInv_32s_Sfs,
                   ( const Ipp32s* pSrcLow, const Ipp32s* pSrcHigh,
                                Ipp32s* pDst, int len, int scaleFactor ))
IPPAPI (IppStatus, ippsWTHaarInv_64s_Sfs,
                   ( const Ipp64s* pSrcLow, const Ipp64s* pSrcHigh,
                              Ipp64s* pDst, int len, int scaleFactor ))


/* /////////////////////////////////////////////////////////////////////////////
//          Wavelet Transform Fucntions for User Filter Banks
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )

struct sWTFwdState_32f;
typedef struct sWTFwdState_32f IppsWTFwdState_32f;

struct sWTFwdState_8s32f;
typedef struct sWTFwdState_8s32f  IppsWTFwdState_8s32f;

struct sWTFwdState_8u32f;
typedef struct sWTFwdState_8u32f  IppsWTFwdState_8u32f;

struct sWTFwdState_16s32f;
typedef struct sWTFwdState_16s32f IppsWTFwdState_16s32f;

struct sWTFwdState_16u32f;
typedef struct sWTFwdState_16u32f IppsWTFwdState_16u32f;

struct sWTInvState_32f;
typedef struct sWTInvState_32f    IppsWTInvState_32f;

struct sWTInvState_32f8s;
typedef struct sWTInvState_32f8s  IppsWTInvState_32f8s;

struct sWTInvState_32f8u;
typedef struct sWTInvState_32f8u  IppsWTInvState_32f8u;

struct sWTInvState_32f16s;
typedef struct sWTInvState_32f16s IppsWTInvState_32f16s;

struct sWTInvState_32f16u;
typedef struct sWTInvState_32f16u IppsWTInvState_32f16u;

#endif /* _OWN_BLDPCS */


/* //////////////////////////////////////////////////////////////////////
// Name:        ippsWTFwdInitAlloc_32f, ippsWTFwdInitAlloc_8s32f,
//              ippsWTFwdInitAlloc_8u32f, ippsWTFwdInitAlloc_16s32f,
//              ippsWTFwdInitAlloc_16u32f
//
// Purpose:     Allocate and initialize
//                forward wavelet transform pState structure.
// Parameters:
//   pState    - pointer to pointer to allocated and initialized
//                pState structure.
//   pTapsLow  - pointer to lowpass filter taps;
//   lenLow    - length of lowpass filter;
//   offsLow   - input delay of lowpass filter;
//   pTapsHigh - pointer to highpass filter taps;
//   lenHigh   - length of highpass filter;
//   offsHigh  - input delay of highpass filter;
//
// Returns:
//   ippStsNoErr        - Ok;
//   ippStsNullPtrErr   - pointer to filter taps are NULL
//                          or pointer to pState structure is NULL;
//   ippStsSizeErr      - filter length is less or equal zero;
//   ippStsWtOffsetErr  - filter delay is less than (-1).
//
// Notes:   filter input delay minimum value is (-1) that corresponds to
//            downsampling phase equal 1 (first sample excluded,
//            second included and so on);
*/
IPPAPI (IppStatus, ippsWTFwdInitAlloc_32f, (IppsWTFwdState_32f** pState,
        const Ipp32f* pTapsLow,  int lenLow,  int offsLow,
        const Ipp32f* pTapsHigh, int lenHigh, int offsHigh))

IPPAPI (IppStatus, ippsWTFwdInitAlloc_8s32f, (IppsWTFwdState_8s32f** pState,
        const Ipp32f* pTapsLow,  int lenLow,  int offsLow,
        const Ipp32f* pTapsHigh, int lenHigh, int offsHigh))

IPPAPI (IppStatus, ippsWTFwdInitAlloc_8u32f, (IppsWTFwdState_8u32f** pState,
        const Ipp32f* pTapsLow,  int lenLow,  int offsLow,
        const Ipp32f* pTapsHigh, int lenHigh, int offsHigh))

IPPAPI (IppStatus, ippsWTFwdInitAlloc_16s32f, (IppsWTFwdState_16s32f** pState,
        const Ipp32f* pTapsLow,  int lenLow,  int offsLow,
        const Ipp32f* pTapsHigh, int lenHigh, int offsHigh))

IPPAPI (IppStatus, ippsWTFwdInitAlloc_16u32f, (IppsWTFwdState_16u32f** pState,
        const Ipp32f* pTapsLow,  int lenLow,  int offsLow,
        const Ipp32f* pTapsHigh, int lenHigh, int offsHigh))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippsWTFwdSetDlyLine_32f, ippsWTFwdSetDlyLine_8s32f,
//              ippsWTFwdSetDlyLine_8u32f, ippsWTFwdSetDlyLine_16s32f,
//              ippsWTFwdSetDlyLine_16u32f
//
// Purpose:     The function copies the pointed vectors to internal delay lines.
//
// Parameters:
//   pState   - pointer to pState structure;
//   pDlyLow  - pointer to delay line for lowpass filtering;
//   pDlyHigh - pointer to delay line for highpass filtering.
//
// Returns:
//   ippStsNoErr            - Ok;
//   ippStsNullPtrErr       - some of pointers pDlyLow
//                              or pDlyHigh vectors are NULL;
//   ippStspStateMatchErr   - mismatch pState structure.
//
// Notes: lengths of delay lines:
//          len(pDlyLow)  = lenLow  + offsLow  - 1;
//          len(pDlyHigh) = lenHigh + offsHigh - 1;
//  lenLow, offsLow, lenHigh, offsHigh - parameters
//    for ippsWTFwdInitAlloc function.
*/
IPPAPI (IppStatus, ippsWTFwdSetDlyLine_32f, (IppsWTFwdState_32f* pState,
        const Ipp32f* pDlyLow, const Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTFwdSetDlyLine_8s32f, (IppsWTFwdState_8s32f* pState,
        const Ipp32f* pDlyLow, const Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTFwdSetDlyLine_8u32f, (IppsWTFwdState_8u32f* pState,
        const Ipp32f* pDlyLow, const Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTFwdSetDlyLine_16s32f, (IppsWTFwdState_16s32f* pState,
        const Ipp32f* pDlyLow, const Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTFwdSetDlyLine_16u32f, (IppsWTFwdState_16u32f* pState,
        const Ipp32f* pDlyLow, const Ipp32f* pDlyHigh))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippsWTFwdGetDlyLine_32f, ippsWTFwdGetDlyLine_8s32f,
//              ippsWTFwdGetDlyLine_8u32f, ippsWTFwdGetDlyLine_16s32f,
//              ippsWTFwdGetDlyLine_16u32f
//
// Purpose:     The function copies data from interanl delay lines
//                to the pointed vectors.
// Parameters:
//   pState   - pointer to pState structure;
//   pDlyLow  - pointer to delay line for lowpass filtering;
//   pDlyHigh - pointer to delay line for highpass filtering.
//
// Returns:
//   ippStsNoErr            - Ok;
//   ippStsNullPtrErr       - some of pointers pDlyLow
//                              or pDlyHigh vectors are NULL;
//   ippStspStateMatchErr   - mismatch pState structure.
//
// Notes: lengths of delay lines:
//          len(pDlyLow)  = lenLow  + offsLow  - 1;
//          len(pDlyHigh) = lenHigh + offsHigh - 1;
//  lenLow, offsLow, lenHigh, offsHigh - parameters
//    for ippsWTFwdInitAlloc function.
*/
IPPAPI (IppStatus, ippsWTFwdGetDlyLine_32f, (IppsWTFwdState_32f* pState,
        Ipp32f* pDlyLow, Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTFwdGetDlyLine_8s32f, (IppsWTFwdState_8s32f* pState,
        Ipp32f* pDlyLow, Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTFwdGetDlyLine_8u32f, (IppsWTFwdState_8u32f* pState,
        Ipp32f* pDlyLow, Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTFwdGetDlyLine_16s32f, (IppsWTFwdState_16s32f* pState,
        Ipp32f* pDlyLow, Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTFwdGetDlyLine_16u32f, (IppsWTFwdState_16u32f* pState,
        Ipp32f* pDlyLow, Ipp32f* pDlyHigh))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippsWTFwd_32f, ippsWTFwd_16s32f, ippsWTFwd_16u32f,
//              ippsWTFwd_8s32f, ippsWTFwd_8u32f
//
// Purpose:     Forward wavelet transform.
//
// Parameters:
//   pSrc     - pointer to source block of data;
//   pDstLow  - pointer to destination block of
//                "low-frequency" component;
//   pDstHigh - pointer to destination block of
//                "high-frequency" component;
//   dstLen   - length of destination;
//   pState    - pointer to pState structure.
//
//  Returns:
//   ippStsNoErr            - Ok;
//   ippStsNullPtrErr       - some of pointers to pSrc, pDstLow
//                              or pDstHigh vectors are NULL;
//   ippStsSizeErr          - the length is less or equal zero;
//   ippStspStateMatchErr    - mismatch pState structure.
//
// Notes:      source block length must be 2 * dstLen.
*/
IPPAPI (IppStatus, ippsWTFwd_32f, (const Ipp32f* pSrc,
        Ipp32f* pDstLow, Ipp32f* pDstHigh, int dstLen,
        IppsWTFwdState_32f* pState))

IPPAPI (IppStatus, ippsWTFwd_8s32f, (const Ipp8s* pSrc,
        Ipp32f* pDstLow, Ipp32f* pDstHigh, int dstLen,
        IppsWTFwdState_8s32f* pState))

IPPAPI (IppStatus, ippsWTFwd_8u32f, (const Ipp8u* pSrc,
        Ipp32f* pDstLow, Ipp32f* pDstHigh, int dstLen,
        IppsWTFwdState_8u32f* pState))

IPPAPI (IppStatus, ippsWTFwd_16s32f, (const Ipp16s* pSrc,
        Ipp32f* pDstLow, Ipp32f* pDstHigh, int dstLen,
        IppsWTFwdState_16s32f* pState))

IPPAPI (IppStatus, ippsWTFwd_16u32f, (
        const Ipp16u* pSrc, Ipp32f* pDstLow, Ipp32f* pDstHigh, int dstLen,
        IppsWTFwdState_16u32f* pState))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippsWTFwdFree_32f, ippsWTFwdFree_8s32f, ippsWTFwdFree_8u32f,
//              ippsWTFwdFree_16s32f, ippsWTFwdFree_16u32f
//
// Purpose:     Free and Deallocate forward wavelet transofrm pState structure.
//
// Parameters:
//   IppsWTFwdState_32f *pState - pointer to pState structure.
//
//  Returns:
//   ippStsNoErr            - Ok;
//   ippStsNullPtrErr       - Pointer to pState structure is NULL;
//   ippStspStateMatchErr   - Mismatch pState structure.
//
// Notes:      if pointer to pState is NULL, ippStsNoErr will be returned.
*/
IPPAPI (IppStatus, ippsWTFwdFree_32f, (IppsWTFwdState_32f* pState))

IPPAPI (IppStatus, ippsWTFwdFree_8s32f, (IppsWTFwdState_8s32f* pState))

IPPAPI (IppStatus, ippsWTFwdFree_8u32f, (IppsWTFwdState_8u32f* pState))

IPPAPI (IppStatus, ippsWTFwdFree_16s32f, (IppsWTFwdState_16s32f* pState))

IPPAPI (IppStatus, ippsWTFwdFree_16u32f, (IppsWTFwdState_16u32f* pState))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippsWTInvInitAlloc_32f,   ippsWTInvInitAlloc_32f8s,
//              ippsWTInvInitAlloc_32f8u, ippsWTInvInitAlloc_32f16s,
//              ippsWTInvInitAlloc_32f16u
//
// Purpose:     Allocate and initialize
//                inverse wavelet transform pState structure.
// Parameters:
//   pState    - pointer to pointer to allocated and initialized
//                pState structure.
//   pTapsLow  - pointer to lowpass filter taps;
//   lenLow    - length of lowpass filter;
//   offsLow   - input delay of lowpass filter;
//   pTapsHigh - pointer to highpass filter taps;
//   lenHigh   - length of highpass filter;
//   offsHigh  - input delay of highpass filter;
//
// Returns:
//   ippStsNoErr        - Ok;
//   ippStsNullPtrErr   - pointer to filter taps are NULL
//                          or pointer to pState structure is NULL;
//   ippStsSizeErr      - filter length is less or equal zero;
//   ippStsWtOffsetErr  - filter delay is less than (-1).
//
// Notes:       filter output delay minimum value is 0 that corresponds to
//             upsampling phase equal 0 (first sample included,
//                                          second sample is zero and so on);
//              pointer to returned error status may be NULL if no error
//             diagnostic required.
*/
IPPAPI (IppStatus, ippsWTInvInitAlloc_32f, (IppsWTInvState_32f** pState,
        const Ipp32f* pTapsLow,  int lenLow,  int offsLow,
        const Ipp32f* pTapsHigh, int lenHigh, int offsHigh))

IPPAPI (IppStatus, ippsWTInvInitAlloc_32f8s, (IppsWTInvState_32f8s** pState,
        const Ipp32f* pTapsLow,  int lenLow,  int offsLow,
        const Ipp32f* pTapsHigh, int lenHigh, int offsHigh))

IPPAPI (IppStatus, ippsWTInvInitAlloc_32f8u, (IppsWTInvState_32f8u** pState,
        const Ipp32f* pTapsLow,  int lenLow,  int offsLow,
        const Ipp32f* pTapsHigh, int lenHigh, int offsHigh))

IPPAPI (IppStatus, ippsWTInvInitAlloc_32f16s, (IppsWTInvState_32f16s** pState,
        const Ipp32f* pTapsLow,  int lenLow,  int offsLow,
        const Ipp32f* pTapsHigh, int lenHigh, int offsHigh))

IPPAPI (IppStatus, ippsWTInvInitAlloc_32f16u, (IppsWTInvState_32f16u** pState,
        const Ipp32f* pTapsLow,  int lenLow,  int offsLow,
        const Ipp32f* pTapsHigh, int lenHigh, int offsHigh))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippsWTInvSetDlyLine_32f, ippsWTInvSetDlyLine_32f8s,
//              ippsWTInvSetDlyLine_32f8u, ippsWTInvSetDlyLine_32f16s,
//              ippsWTInvSetDlyLine_32f16u
//
// Purpose:     The function copies the pointed vectors to internal delay lines.
//
// Parameters:
//   pState   - pointer to pState structure;
//   pDlyLow  - pointer to delay line for lowpass filtering;
//   pDlyHigh - pointer to delay line for highpass filtering.
//
// Returns:
//   ippStsNoErr            - Ok;
//   ippStsNullPtrErr       - some of pointers pDlyLow
//                              or pDlyHigh vectors are NULL;
//   ippStspStateMatchErr   - mismatch pState structure.
//
// Notes: lengths of delay lines (as "C" expression):
//          len(pDlyLow)  = (lenLow   + offsLow  - 1) / 2;
//          len(pDlyHigh) = (lenHigh  + offsHigh - 1) / 2;
//  lenLow, offsLow, lenHigh, offsHigh - parameters
//    for ippsWTInvInitAlloc function.
*/
IPPAPI (IppStatus, ippsWTInvSetDlyLine_32f, (IppsWTInvState_32f* pState,
        const Ipp32f* pDlyLow, const Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTInvSetDlyLine_32f8s, (IppsWTInvState_32f8s* pState,
        const Ipp32f* pDlyLow, const Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTInvSetDlyLine_32f8u, (IppsWTInvState_32f8u* pState,
        const Ipp32f* pDlyLow, const Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTInvSetDlyLine_32f16s, (IppsWTInvState_32f16s* pState,
        const Ipp32f* pDlyLow, const Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTInvSetDlyLine_32f16u, (IppsWTInvState_32f16u* pState,
        const Ipp32f* pDlyLow, const Ipp32f* pDlyHigh))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippsWTInvGetDlyLine_32f, ippsWTInvGetDlyLine_32f8s,
//              ippsWTInvGetDlyLine_32f8u, ippsWTInvGetDlyLine_32f16s,
//              ippsWTInvGetDlyLine_32f16u
//
// Purpose:     The function copies data from interanl delay lines
//                to the pointed vectors.
// Parameters:
//   pState   - pointer to pState structure;
//   pDlyLow  - pointer to delay line for lowpass filtering;
//   pDlyHigh - pointer to delay line for highpass filtering.
//
// Returns:
//   ippStsNoErr            - Ok;
//   ippStsNullPtrErr       - some of pointers pDlyLow
//                              or pDlyHigh vectors are NULL;
//   ippStspStateMatchErr    - mismatch pState structure.
//
// Notes: lengths of delay lines (as "C" expression):
//          len(pDlyLow)  = (lenLow   + offsLow  - 1) / 2;
//          len(pDlyHigh) = (lenHigh  + offsHigh - 1) / 2;
//  lenLow, offsLow, lenHigh, offsHigh - parameters
//    for ippsWTInvInitAlloc function.
*/
IPPAPI (IppStatus, ippsWTInvGetDlyLine_32f, (IppsWTInvState_32f* pState,
        Ipp32f* pDlyLow, Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTInvGetDlyLine_32f8s, (IppsWTInvState_32f8s* pState,
        Ipp32f* pDlyLow, Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTInvGetDlyLine_32f8u, (IppsWTInvState_32f8u* pState,
        Ipp32f* pDlyLow, Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTInvGetDlyLine_32f16s, (IppsWTInvState_32f16s* pState,
        Ipp32f* pDlyLow, Ipp32f* pDlyHigh))

IPPAPI (IppStatus, ippsWTInvGetDlyLine_32f16u, (IppsWTInvState_32f16u* pState,
        Ipp32f* pDlyLow, Ipp32f* pDlyHigh))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippsWTInv_32f, ippsWTInv_32f16s, ippsWTInv_32f16u,
//              ippsWTInv_32f8s, ippsWTInv_32f8u
//
// Purpose:     Inverse wavelet transform.
//
// Parameters:
//   srcLow  - pointer to source block of
//               "low-frequency" component;
//   srcHigh - pointer to source block of
//               "high-frequency" component;
//   dstLen  - length of components.
//   dst     - pointer to destination block of
//               reconstructed data;
//   pState  - pointer to pState structure;
//
//  Returns:
//   ippStsNoErr            - Ok;
//   ippStsNullPtrErr       - some of pointers to pDst pSrcLow
//                              or pSrcHigh vectors are NULL;
//   ippStsSizeErr          - the length is less or equal zero;
//   ippStspStateMatchErr    - mismatch pState structure.
//
// Notes:      destination block length must be 2 * srcLen.
*/

IPPAPI (IppStatus, ippsWTInv_32f, (
        const Ipp32f* pSrcLow, const Ipp32f* pSrcHigh, int srcLen, Ipp32f* pDst,
        IppsWTInvState_32f* pState))

IPPAPI (IppStatus, ippsWTInv_32f8s, (
        const Ipp32f* pSrcLow, const Ipp32f* pSrcHigh, int srcLen, Ipp8s* pDst,
        IppsWTInvState_32f8s* pState))

IPPAPI (IppStatus, ippsWTInv_32f8u, (
        const Ipp32f* pSrcLow, const Ipp32f* pSrcHigh, int srcLen, Ipp8u* pDst,
        IppsWTInvState_32f8u* pState))

IPPAPI (IppStatus, ippsWTInv_32f16s, (
        const Ipp32f* pSrcLow, const Ipp32f* pSrcHigh, int srcLen, Ipp16s* pDst,
        IppsWTInvState_32f16s* pState))

IPPAPI (IppStatus, ippsWTInv_32f16u, (
        const Ipp32f* pSrcLow, const Ipp32f* pSrcHigh, int srcLen, Ipp16u* pDst,
        IppsWTInvState_32f16u* pState))


/* //////////////////////////////////////////////////////////////////////
// Name:        ippsWTInvFree_32f, ippsWTInvFree_32f8s, ippsWTInvFree_32f8u,
//              ippsWTInvFree_32f16s, ippsWTInvFree_32f16u
//
// Purpose:     Free and Deallocate inverse wavelet transofrm pState structure.
//
// Parameters:
//   IppsWTInvState_32f *pState - pointer to pState structure.
//
//  Returns:
//   ippStsNoErr            - Ok;
//   ippStsNullPtrErr       - Pointer to pState structure is NULL;
//   ippStspStateMatchErr   - Mismatch pState structure.
//
// Notes:      if pointer to pState is NULL, ippStsNoErr will be returned.
*/
IPPAPI (IppStatus, ippsWTInvFree_32f, (IppsWTInvState_32f* pState))

IPPAPI (IppStatus, ippsWTInvFree_32f8s, (IppsWTInvState_32f8s* pState))

IPPAPI (IppStatus, ippsWTInvFree_32f8u, (IppsWTInvState_32f8u* pState))

IPPAPI (IppStatus, ippsWTInvFree_32f16s, (IppsWTInvState_32f16s* pState))

IPPAPI (IppStatus, ippsWTInvFree_32f16u, (IppsWTInvState_32f16u* pState))



/* /////////////////////////////////////////////////////////////////////////////
//                  Filtering
///////////////////////////////////////////////////////////////////////////// */


/* /////////////////////////////////////////////////////////////////////////////
//                  Convolution functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConv
//  Purpose:    Linear Convolution of 1D signals
//  Parameters:
//      pSrc1                pointer to the first source vector
//      pSrc2                pointer to the second source vector
//      lenSrc1              length of the first source vector
//      lenSrc2              length of the second source vector
//      pDst                 pointer to the destination vector
//  Returns:    IppStatus
//      ippStsNullPtrErr        pointer(s) to the data is NULL
//      ippStsSizeErr           length of the vectors is less or equal zero
//      ippStsMemAllocErr       no memory for internal buffers
//      ippStsNoErr             otherwise
//  Notes:
//          Length of the destination data vector is lenSrc1+lenSrc2-1.
//          The input signal are exchangeable because of
//          commutative convolution property.
//          Some other values may be returned by FFT transform functions
*/

IPPAPI(IppStatus, ippsConv_32f, ( const Ipp32f* pSrc1, int lenSrc1,
       const Ipp32f* pSrc2, int lenSrc2, Ipp32f* pDst))
IPPAPI(IppStatus, ippsConv_16s_Sfs, ( const Ipp16s* pSrc1, int lenSrc1,
       const Ipp16s* pSrc2, int lenSrc2, Ipp16s* pDst, int scaleFactor))
IPPAPI( IppStatus, ippsConv_64f,( const Ipp64f* pSrc1, int lenSrc1,
        const Ipp64f* pSrc2, int lenSrc2, Ipp64f* pDst))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConvBiased_32f
//  Purpose:    Linear Convolution of 1D signals whith a bias.
//  Parameters:
//      pSrc1               pointer to the first source vector
//      pSrc2               pointer to the second source vector
//      lenSrc1             length of the first source vector
//      lenSrc2             length of the second source vector
//      pDst                pointer to the destination vector
//      lenDst              length of the destination vector
//      bias
//  Returns:    IppStatus
//      ippStsNullPtrErr        pointer(s) to the data is NULL
//      ippStsSizeErr           length of the vectors is less or equal zero
//      ippStsNoErr             otherwise
*/

IPPAPI ( IppStatus, ippsConvBiased_32f,
                    ( const Ipp32f *pSrc1, int len1,
                      const Ipp32f *pSrc2, int len2,
                            Ipp32f *pDst, int lenDst, int bias ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsConvCyclic
//  Purpose:    Cyclic Convolution of 1D signals of fixed size
//  Parameters: the pointers to data of fixed size
//  Returns:    IppStatus
//                ippStsNoErr    parameters are not checked
//  Notes:
//          The length of the convolution is given in the function name.
*/

IPPAPI(IppStatus, ippsConvCyclic8x8_32f,( const Ipp32f* x,
       const Ipp32f* h, Ipp32f* y ))
IPPAPI(IppStatus, ippsConvCyclic8x8_16s_Sfs,( const Ipp16s* x,
       const Ipp16s* h, Ipp16s* y, int scaleFactor ))
IPPAPI(IppStatus, ippsConvCyclic4x4_32f32fc,( const Ipp32f* x,
       const Ipp32fc* h, Ipp32fc* y ))



/* /////////////////////////////////////////////////////////////////////////////
//                     IIR filters (float and double taps versions)
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )

struct IIRState_32f;
typedef struct IIRState_32f IppsIIRState_32f;

struct IIRState_32fc;
typedef struct IIRState_32fc IppsIIRState_32fc;

struct IIRState32f_16s;
typedef struct IIRState32f_16s IppsIIRState32f_16s;

struct IIRState32fc_16sc;
typedef struct IIRState32fc_16sc IppsIIRState32fc_16sc;

struct IIRState_64f;
typedef struct IIRState_64f IppsIIRState_64f;

struct IIRState_64fc;
typedef struct IIRState_64fc IppsIIRState_64fc;

struct IIRState64f_32f;
typedef struct IIRState64f_32f IppsIIRState64f_32f;

struct IIRState64fc_32fc;
typedef struct IIRState64fc_32fc IppsIIRState64fc_32fc;

struct IIRState64f_32s;
typedef struct IIRState64f_32s IppsIIRState64f_32s;

struct IIRState64fc_32sc;
typedef struct IIRState64fc_32sc IppsIIRState64fc_32sc;

struct IIRState64f_16s;
typedef struct IIRState64f_16s IppsIIRState64f_16s;

struct IIRState64fc_16sc;
typedef struct IIRState64fc_16sc IppsIIRState64fc_16sc;

struct IIRState32s_16s;
typedef struct IIRState32s_16s IppsIIRState32s_16s;

struct IIRState32sc_16sc;
typedef struct IIRState32sc_16sc IppsIIRState32sc_16sc;


#endif /* _OWN_BLDPCS */

/* /////////////////////////////////////////////////////////////////////////////
//  Initialize context
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:     ippsIIRInitAlloc, ippsIIRFree
//  Purpose:       initialize context arbitrary order IIR filter
//  Parameters:
//      ppState     - double pointer to filter context
//      pState      - pointer to filter context
//      pTaps       - pointer to filter coefficients
//      order       - arbitrary filter order
//      pDelay      - pointer to delay line data, can be NULL
//  Return: IppStatus
//      ippStsMemAllocErr    - memory allocation error
//      ippStsNullPtrErr     - pointer(s) to the data is NULL
//      ippStsIIROrderErr    - filter order < 0
//      ippStsDivByZeroErr   - A(0) is zero
//      ippStsContextMatchErr  - wrong context identifier
//      ippStsNoErr          - otherwise
//  Order of the coefficients in the input taps buffer:
//     B(0),B(1),B(2)..,B(order);
//     A(0),A(1),A(2)..,A(order);
//     . . .
//  Note:
//      A(0) != 0
//      ippsIIRClose function works for both AR and BQ contexts
*/

IPPAPI(IppStatus, ippsIIRInitAlloc_32f, (IppsIIRState_32f** ppState,
       const Ipp32f* pTaps, int order, const Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc_32fc, (IppsIIRState_32fc** ppState,
       const Ipp32fc* pTaps, int order, const Ipp32fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc32f_16s, (IppsIIRState32f_16s** ppState,
       const Ipp32f* pTaps, int order, const Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc32fc_16sc, (IppsIIRState32fc_16sc** ppState,
       const Ipp32fc* pTaps, int order, const Ipp32fc* pDlyLine))

IPPAPI(IppStatus, ippsIIRInitAlloc_64f, (IppsIIRState_64f** ppState,
       const Ipp64f* pTaps, int order, const Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc_64fc, (IppsIIRState_64fc** ppState,
       const Ipp64fc* pTaps, int order, const Ipp64fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc64f_32f, (IppsIIRState64f_32f** ppState,
       const Ipp64f* pTaps, int order, const Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc64fc_32fc, (IppsIIRState64fc_32fc** ppState,
       const Ipp64fc* pTaps, int order, const Ipp64fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc64f_32s, (IppsIIRState64f_32s** ppState,
       const Ipp64f* pTaps, int order, const Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc64fc_32sc, (IppsIIRState64fc_32sc** ppState,
       const Ipp64fc* pTaps, int order, const Ipp64fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc64f_16s, (IppsIIRState64f_16s** ppState,
       const Ipp64f* pTaps, int order, const Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc64fc_16sc, (IppsIIRState64fc_16sc** ppState,
       const Ipp64fc* pTaps, int order, const Ipp64fc* pDlyLine))

IPPAPI(IppStatus, ippsIIRFree_32f, (IppsIIRState_32f* pState))
IPPAPI(IppStatus, ippsIIRFree_32fc, (IppsIIRState_32fc* pState))
IPPAPI(IppStatus, ippsIIRFree32f_16s, (IppsIIRState32f_16s* pState))
IPPAPI(IppStatus, ippsIIRFree32fc_16sc, (IppsIIRState32fc_16sc* pState))

IPPAPI(IppStatus, ippsIIRFree_64f, (IppsIIRState_64f* pState))
IPPAPI(IppStatus, ippsIIRFree_64fc, (IppsIIRState_64fc* pState))
IPPAPI(IppStatus, ippsIIRFree64f_32f, (IppsIIRState64f_32f* pState))
IPPAPI(IppStatus, ippsIIRFree64fc_32fc, (IppsIIRState64fc_32fc* pState))
IPPAPI(IppStatus, ippsIIRFree64f_32s, (IppsIIRState64f_32s* pState))
IPPAPI(IppStatus, ippsIIRFree64fc_32sc, (IppsIIRState64fc_32sc* pState))
IPPAPI(IppStatus, ippsIIRFree64f_16s, (IppsIIRState64f_16s* pState))
IPPAPI(IppStatus, ippsIIRFree64fc_16sc, (IppsIIRState64fc_16sc* pState))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:     ippsIIRInitAlloc_BiQuad
//  Purpose:   initialize biquad numBq-section filter
//  Parameters:
//      ppState     - double pointer to filter context
//      pTaps       - pointer to filter coefficients
//      numBq       - number biquads of BQ filter
//      pDelay      - pointer to delay line data, can be NULL
//  Return: IppStatus
//      ippStsMemAllocErr  - memory allocation error
//      ippStsNullPtrErr   - pointer(s) ppState or pTaps is NULL
//      ippStsIIROrderErr  - numBq <= 0
//      ippStsDivByZeroErr - A(n,0) or B(n,0) is zero
//      ippStsNoErr        - otherwise
//
//  Order of the coefficients in the input taps buffer:
//     B(0,0),B(0,1),B(0,2),A(0,0),A(0,1),A(0,2);
//     B(1,0),B(1,1),B(1,2),A(1,0),A(1,1),A(1,2);
//     . . .
//  Notice:
//      A(n,0) != 0 and B(n,0) != 0
*/

IPPAPI(IppStatus, ippsIIRInitAlloc_BiQuad_32f, (IppsIIRState_32f** ppState,
       const Ipp32f* pTaps, int numBq, const Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc_BiQuad_32fc, (IppsIIRState_32fc** ppState,
       const Ipp32fc* pTaps, int numBq, const Ipp32fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc32f_BiQuad_16s, (IppsIIRState32f_16s** ppState,
       const Ipp32f* pTaps, int numBq, const Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc32fc_BiQuad_16sc, (IppsIIRState32fc_16sc** ppState,
       const Ipp32fc* pTaps, int numBq, const Ipp32fc* pDlyLine))

IPPAPI(IppStatus, ippsIIRInitAlloc_BiQuad_64f, (IppsIIRState_64f** ppState,
       const Ipp64f* pTaps, int numBq, const Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc_BiQuad_64fc, (IppsIIRState_64fc** ppState,
       const Ipp64fc* pTaps, int numBq, const Ipp64fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc64f_BiQuad_32f, (IppsIIRState64f_32f** ppState,
       const Ipp64f* pTaps, int numBq, const Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc64fc_BiQuad_32fc, (IppsIIRState64fc_32fc** ppState,
       const Ipp64fc* pTaps, int numBq, const Ipp64fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc64f_BiQuad_32s, (IppsIIRState64f_32s** ppState,
       const Ipp64f* pTaps, int numBq, const Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc64fc_BiQuad_32sc, (IppsIIRState64fc_32sc** ppState,
       const Ipp64fc* pTaps, int numBq, const Ipp64fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc64f_BiQuad_16s, (IppsIIRState64f_16s** ppState,
       const Ipp64f* pTaps, int numBq, const Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc64fc_BiQuad_16sc, (IppsIIRState64fc_16sc** ppState,
       const Ipp64fc* pTaps, int numBq, const Ipp64fc* pDlyLine))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:     ippsIIRInitAlloc_BiQuad_DF1
//  Purpose:   initialize biquad numBq-section DF1 filter
//  Parameters:
//      ppState     - double pointer to filter context
//      pTaps       - pointer to filter coefficients
//      numBq       - number biquads of BQ filter
//      pDelay      - pointer to delay line data, can be NULL
//  Return: IppStatus
//      ippStsMemAllocErr  - memory allocation error
//      ippStsNullPtrErr   - pointer(s) ppState or pTaps is NULL
//      ippStsIIROrderErr  - numBq <= 0
//      ippStsDivByZeroErr - A(n,0) or B(n,0) is zero
//      ippStsNoErr        - otherwise
//
//  Order of the coefficients in the input taps buffer:
//    pTaps = B[0,0], B[0,1], B[0,2], A[0,0], A[0,1], A[0,2],
//            B[1,0], B[1,1], B[1,2], A[1,0], A[1,1], A[1,2],
//            ...
//            B[n,0], B[n,1], B[n,2], A[n,0], A[n,1], A[n,2]
//            ( n = numBq-1 );
//
//    pDLyLine = X[0,-2], X[0,-1], Y[0,-2], Y[0,-1],
//               X[1,-2], X[1,-1], Y[1,-2], Y[1,-1],
//               ...
//               X[n,-2], X[n,-1], Y[n,-2], Y[n,-1],
//               ( n = numBq-1 );
//     . . .
//  Notice:
//      A(n,0) != 0
//
//    Y[i] = X[i-2]*B[0,2] + X[i-1]*B[0,1] + X[i]*B[0,0] -
//           - Y[i-1]*A[0,1] - Y[i-2]*A[0,2];
//    Z[i] = Y[i];
//    Y[i] = Z[i-2]*B[1,2] + Z[i-1]*B[1,1] + Z[i]*B[1,0] -
//             - Y[i-1]*A[1,1] - Y[i-2]*A[1,2];
//    Z[i] = Y[i];
//    ...
//    Y[i] = Z[i-2]*B[j,2] + Z[i-1]*B[j,1] + Z[i]*B[j,0] -
//             - Y[i-1]*A[j,1] - Y[i-2]*A[j,2];
//    Z[i] = Y[i];
//    ...
//    Y[i] = Z[i-2]*B[n,2] + Z[i-1]*B[n,1] + Z[i]*B[n,0] -
//             - Y[i-1]*A[n,1] - Y[i-2]*A[n,2]; (n=numBq-1)
//    pDst[i] = Y[i];
*/
IPPAPI( IppStatus, ippsIIRInitAlloc_BiQuad_DF1_32f,( IppsIIRState_32f** pState,
                   const Ipp32f* pTaps, int numBq, const Ipp32f* pDlyLine ))
IPPAPI(IppStatus, ippsIIRInitAlloc64f_BiQuad_DF1_32s, (IppsIIRState64f_32s** ppState,
       const Ipp64f* pTaps, int numBq, const Ipp32s* pDlyLine))



/* /////////////////////////////////////////////////////////////////////////////
//  Work with Delay Line
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsIIRGetDlyLine, ippsIIRSetDlyLine
//  Purpose:    set and get delay line
//  Parameters:
//      pState              - pointer to IIR filter context
//      pDelay              - pointer to delay line to be set
//  Return:
//      ippStsContextMatchErr  - wrong context identifier
//      ippStsNullPtrErr       - pointer(s) pState or pDelay is NULL
//      ippStsNoErr            - otherwise
*/

IPPAPI(IppStatus, ippsIIRGetDlyLine_32f, (const IppsIIRState_32f* pState, Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine_32f, (IppsIIRState_32f* pState, const Ipp32f* pDlyLine))

IPPAPI(IppStatus, ippsIIRGetDlyLine_32fc, (const IppsIIRState_32fc* pState, Ipp32fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine_32fc, (IppsIIRState_32fc* pState, const Ipp32fc* pDlyLine))

IPPAPI(IppStatus, ippsIIRGetDlyLine32f_16s, (const IppsIIRState32f_16s* pState, Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine32f_16s, (IppsIIRState32f_16s* pState, const Ipp32f* pDlyLine))

IPPAPI(IppStatus, ippsIIRGetDlyLine32fc_16sc, (const IppsIIRState32fc_16sc* pState, Ipp32fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine32fc_16sc, (IppsIIRState32fc_16sc* pState, const Ipp32fc* pDlyLine))

IPPAPI(IppStatus, ippsIIRGetDlyLine_64f, (const IppsIIRState_64f* pState, Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine_64f, (IppsIIRState_64f* pState, const Ipp64f* pDlyLine))

IPPAPI(IppStatus, ippsIIRGetDlyLine_64fc, (const IppsIIRState_64fc* pState, Ipp64fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine_64fc, (IppsIIRState_64fc* pState, const Ipp64fc* pDlyLine))

IPPAPI(IppStatus, ippsIIRGetDlyLine64f_32f, (const IppsIIRState64f_32f* pState, Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine64f_32f, (IppsIIRState64f_32f* pState, const Ipp64f* pDlyLine))

IPPAPI(IppStatus, ippsIIRGetDlyLine64fc_32fc, (const IppsIIRState64fc_32fc* pState, Ipp64fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine64fc_32fc, (IppsIIRState64fc_32fc* pState, const Ipp64fc* pDlyLine))

IPPAPI(IppStatus, ippsIIRGetDlyLine64f_32s, (const IppsIIRState64f_32s* pState, Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine64f_32s, (IppsIIRState64f_32s* pState, const Ipp64f* pDlyLine))

IPPAPI(IppStatus, ippsIIRGetDlyLine64fc_32sc, (const IppsIIRState64fc_32sc* pState, Ipp64fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine64fc_32sc, (IppsIIRState64fc_32sc* pState, const Ipp64fc* pDlyLine))

IPPAPI(IppStatus, ippsIIRGetDlyLine64f_16s, (const IppsIIRState64f_16s* pState, Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine64f_16s, (IppsIIRState64f_16s* pState, const Ipp64f* pDlyLine))

IPPAPI(IppStatus, ippsIIRGetDlyLine64fc_16sc, (const IppsIIRState64fc_16sc* pState, Ipp64fc* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine64fc_16sc, (IppsIIRState64fc_16sc* pState, const Ipp64fc* pDlyLine))

IPPAPI(IppStatus, ippsIIRGetDlyLine64f_DF1_32s, (const IppsIIRState64f_32s* pState, Ipp32s* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine64f_DF1_32s, (IppsIIRState64f_32s* pState, const Ipp32s* pDlyLine))


/* /////////////////////////////////////////////////////////////////////////////
//  Filtering
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:           ippsIIROne
//  Purpose:         IIR filter with float or double taps. One sample operation
//  Parameters:
//      pState              - pointer to IIR filter context
//      src                 - input sample
//      pDstVal             - output sample
//      scaleFactor         - scale factor value
//  Return:
//      ippStsContextMatchErr  - wrong context identifier
//      ippStsNullPtrErr       - pointer(s) to the data is NULL
//      ippStsNoErr            - otherwise
//
//  Note: Don't modify scaleFactor value unless context is changed
*/

IPPAPI(IppStatus, ippsIIROne_32f, (Ipp32f src, Ipp32f* pDstVal, IppsIIRState_32f* pState))
IPPAPI(IppStatus, ippsIIROne_32fc, (Ipp32fc src, Ipp32fc* pDstVal, IppsIIRState_32fc* pState))

IPPAPI(IppStatus, ippsIIROne32f_16s_Sfs, (Ipp16s src, Ipp16s* pDstVal, IppsIIRState32f_16s* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIROne32fc_16sc_Sfs, (Ipp16sc src, Ipp16sc* pDstVal, IppsIIRState32fc_16sc* pState, int scaleFactor))

IPPAPI(IppStatus, ippsIIROne_64f, (Ipp64f src, Ipp64f* pDstVal, IppsIIRState_64f* pState))
IPPAPI(IppStatus, ippsIIROne_64fc, (Ipp64fc src, Ipp64fc* pDstVal, IppsIIRState_64fc* pState))

IPPAPI(IppStatus, ippsIIROne64f_32f, (Ipp32f src, Ipp32f* pDstVal, IppsIIRState64f_32f* pState))
IPPAPI(IppStatus, ippsIIROne64fc_32fc, (Ipp32fc src, Ipp32fc* pDstVal, IppsIIRState64fc_32fc* pState))

IPPAPI(IppStatus, ippsIIROne64f_32s_Sfs, (Ipp32s src, Ipp32s* pDstVal, IppsIIRState64f_32s* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIROne64fc_32sc_Sfs, (Ipp32sc src, Ipp32sc* pDstVal, IppsIIRState64fc_32sc* pState, int scaleFactor))

IPPAPI(IppStatus, ippsIIROne64f_16s_Sfs, (Ipp16s src, Ipp16s* pDstVal, IppsIIRState64f_16s* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIROne64fc_16sc_Sfs, (Ipp16sc src, Ipp16sc* pDstVal, IppsIIRState64fc_16sc* pState, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:         ippsIIR
//  Purpose:       IIR filter with float or double taps. Vector filtering
//  Parameters:
//      pState              - pointer to filter context
//      pSrcDst             - pointer to input/output vector in in-place ops
//      pSrc                - pointer to input vector
//      pDst                - pointer to output vector
//      len                 - length of the vectors
//      scaleFactor         - scale factor value
//  Return:
//      ippStsContextMatchErr  - wrong context identifier
//      ippStsNullPtrErr       - pointer(s) to the data is NULL
//      ippStsSizeErr          - length of the vectors <= 0
//      ippStsNoErr            - otherwise
//
//  Note: Don't modify scaleFactor value unless context is changed
*/

IPPAPI(IppStatus, ippsIIR_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int len,
       IppsIIRState_32f* pState))
IPPAPI(IppStatus, ippsIIR_32f_I, (Ipp32f* pSrcDst, int len, IppsIIRState_32f* pState))
IPPAPI(IppStatus, ippsIIR_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int len,
       IppsIIRState_32fc* pState))
IPPAPI(IppStatus, ippsIIR_32fc_I, (Ipp32fc* pSrcDst, int len, IppsIIRState_32fc* pState))

IPPAPI(IppStatus, ippsIIR32f_16s_Sfs, (const Ipp16s* pSrc, Ipp16s* pDst, int len,
       IppsIIRState32f_16s* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIR32f_16s_ISfs, (Ipp16s* pSrcDst, int len,
       IppsIIRState32f_16s* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIR32fc_16sc_Sfs, (const Ipp16sc* pSrc, Ipp16sc* pDst, int len,
       IppsIIRState32fc_16sc* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIR32fc_16sc_ISfs, (Ipp16sc* pSrcDst, int len,
       IppsIIRState32fc_16sc* pState, int scaleFactor))

IPPAPI(IppStatus, ippsIIR_64f, (const Ipp64f* pSrc, Ipp64f* pDst, int len,
       IppsIIRState_64f* pState))
IPPAPI(IppStatus, ippsIIR_64f_I, (Ipp64f* pSrcDst, int len, IppsIIRState_64f* pState))
IPPAPI(IppStatus, ippsIIR_64fc, (const Ipp64fc* pSrc, Ipp64fc* pDst, int len,
       IppsIIRState_64fc* pState))
IPPAPI(IppStatus, ippsIIR_64fc_I, (Ipp64fc* pSrcDst, int len, IppsIIRState_64fc* pState))

IPPAPI(IppStatus, ippsIIR64f_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int len,
       IppsIIRState64f_32f* pState))
IPPAPI(IppStatus, ippsIIR64f_32f_I, (Ipp32f* pSrcDst, int len, IppsIIRState64f_32f* pState))
IPPAPI(IppStatus, ippsIIR64fc_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int len,
       IppsIIRState64fc_32fc* pState))
IPPAPI(IppStatus, ippsIIR64fc_32fc_I, (Ipp32fc* pSrcDst, int len, IppsIIRState64fc_32fc* pState))

IPPAPI(IppStatus, ippsIIR64f_32s_Sfs, (const Ipp32s* pSrc, Ipp32s* pDst, int len,
       IppsIIRState64f_32s* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIR64f_32s_ISfs, (Ipp32s* pSrcDst, int len,
       IppsIIRState64f_32s* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIR64fc_32sc_Sfs, (const Ipp32sc* pSrc, Ipp32sc* pDst, int len,
       IppsIIRState64fc_32sc* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIR64fc_32sc_ISfs, (Ipp32sc* pSrcDst, int len,
       IppsIIRState64fc_32sc* pState, int scaleFactor))

IPPAPI(IppStatus, ippsIIR64f_16s_Sfs, (const Ipp16s* pSrc, Ipp16s* pDst, int len,
       IppsIIRState64f_16s* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIR64f_16s_ISfs, (Ipp16s* pSrcDst, int len,
       IppsIIRState64f_16s* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIR64fc_16sc_Sfs, (const Ipp16sc* pSrc, Ipp16sc* pDst, int len,
       IppsIIRState64fc_16sc* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIR64fc_16sc_ISfs, (Ipp16sc* pSrcDst, int len,
       IppsIIRState64fc_16sc* pState, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:         ippsIIR_32f_P, ippsIIR64f_32s_P
//  Purpose:       IIR filter for multi-channel data. Vector filtering.
//  Parameters:
//      ppSrc               - pointer to array of pointers to source vectors
//      ppDst               - pointer to array of pointers to destination vectors
//      ppSrcDst            - pointer to array of source/destination vectors in in-place ops
//      len                 - length of the vectors
//      nChannels           - number of processing channels
//      ppState             - pointer to array of filter contexts
//  Return:
//      ippStsContextMatchErr  - wrong context identifier
//      ippStsNullPtrErr       - pointer(s) to the data is NULL
//      ippStsSizeErr          - length of the vectors <= 0
//      ippStsChannelErr       - number of processing channels <= 0
//      ippStsNoErr            - otherwise
//
*/
IPPAPI( IppStatus, ippsIIR_32f_P,( const Ipp32f **ppSrc, Ipp32f **ppDst, int len,
       int nChannels, IppsIIRState_32f **ppState ))
IPPAPI( IppStatus, ippsIIR_32f_IP,( Ipp32f **ppSrcDst, int len,
       int nChannels, IppsIIRState_32f **ppState ))
IPPAPI(IppStatus, ippsIIR64f_32s_PSfs, (const Ipp32s **ppSrc, Ipp32s **ppDst, int len,
       int nChannels, IppsIIRState64f_32s **ppState, int *pScaleFactor))
IPPAPI(IppStatus, ippsIIR64f_32s_IPSfs, (Ipp32s **ppSrcDst, int len,
       int nChannels, IppsIIRState64f_32s **ppState, int *pScaleFactor))



/* /////////////////////////////////////////////////////////////////////////////
//                     IIR filters (integer taps version)
///////////////////////////////////////////////////////////////////////////// */


/* /////////////////////////////////////////////////////////////////////////////
//  Initialize context
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:         ippsIIRInitAlloc, ippsIIRInitAlloc_BiQuad, ippsIIRFree
//  Purpose:       create and initialize IIR context for AR filter
//  Parameters:
//      ppState     - double pointer to filter context
//      pState      - pointer to filter context
//      pTaps       - pointer to filter coefficients
//      order       - arbitrary filter order
//      tapsFactor  - scale factor for Ipp32s context taps
//      numBq       - number of biquads in BQ filter
//      pDelay      - pointer to delay line, may be NULL
//  Return:
//      ippStsNoErr        - Ok
//      ippStsMemAllocErr  - memory allocate error
//      ippStsNullPtrErr   - pointer(s) to ppState, pState or pTaps is NULL
//      ippStsIIROrderErr  - filter order < 0 or numBq <= 0
//      ippStsDivByZeroErr - A(0) or A(n,0) or B(n,0) is zero
//
//  the Ipp32s taps from the source Ipp32f taps and taps factor
//  may be prepared by this way, for example
//
//   ippsAbs_64f( taps, tmp, 6 );
//   ippsMax_64f( tmp, 6, &tmax );
//
//   tapsfactor = 0;
//   if( tmax > IPP_MAX_32S )
//      while( (tmax/=2) > IPP_MAX_32S ) ++tapsfactor;
//   else
//      while( (tmax*=2) < IPP_MAX_32S ) --tapsfactor;
//
//   if( tapsfactor > 0 )
//      ippsDivC_64f_I( (float)(1<<(++tapsfactor)), taps, 6 );
//   else if( tapsfactor < 0 )
//      ippsMulC_64f_I( (float)(1<<(-(tapsfactor))), taps, 6 );
//
//   ippsConvert_64f32s_Sfs ( taps, taps32s, 6, ippRndNear, 0 );
//
//  Order of coefficients is:
//     B(0),B(1),...,B(order),A(0),A(1),...,A(order)
//  A(0) != 0
*/

IPPAPI(IppStatus, ippsIIRInitAlloc32s_16s, (IppsIIRState32s_16s** ppState,
       const Ipp32s* pTaps, int order, int tapsFactor, const Ipp32s* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc32s_16s32f, (IppsIIRState32s_16s** ppState,
       const Ipp32f* pTaps, int order, const Ipp32s* pDlyLine))

IPPAPI(IppStatus, ippsIIRInitAlloc32sc_16sc, (IppsIIRState32sc_16sc** ppState,
       const Ipp32sc* pTaps, int order, int tapsFactor, const Ipp32sc* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc32sc_16sc32fc, (IppsIIRState32sc_16sc** ppState,
       const Ipp32fc* pTaps, int order, const Ipp32sc* pDlyLine))

IPPAPI(IppStatus, ippsIIRInitAlloc32s_BiQuad_16s, (IppsIIRState32s_16s** ppState,
       const Ipp32s* pTaps, int numBq, int tapsFactor, const Ipp32s* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc32s_BiQuad_16s32f, (IppsIIRState32s_16s** ppState,
       const Ipp32f* pTaps, int numBq, const Ipp32s* pDlyLine))

IPPAPI(IppStatus, ippsIIRInitAlloc32sc_BiQuad_16sc, (IppsIIRState32sc_16sc** ppState,
       const Ipp32sc* pTaps, int numBq, int tapsFactor, const Ipp32sc* pDlyLine))
IPPAPI(IppStatus, ippsIIRInitAlloc32sc_BiQuad_16sc32fc, (IppsIIRState32sc_16sc** ppState,
       const Ipp32fc* pTaps, int numBq, const Ipp32sc* pDlyLine))

IPPAPI(IppStatus, ippsIIRFree32s_16s, (IppsIIRState32s_16s* pState))
IPPAPI(IppStatus, ippsIIRFree32sc_16sc, (IppsIIRState32sc_16sc* pState))


/* /////////////////////////////////////////////////////////////////////////////
//  Work with Delay Line
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsIIRGetDlyLine, ippsIIRSetDlyLine
//  Purpose:    set and get delay line
//  Parameters:
//      pState              - pointer to IIR filter context
//      pDelay              - pointer to delay line to be set
//  Return:
//      ippStsContextMatchErr  - wrong context identifier
//      ippStsNullPtrErr       - pointer(s) to the data is NULL
//      ippStsNoErr            - otherwise
*/

IPPAPI(IppStatus, ippsIIRGetDlyLine32s_16s, (const IppsIIRState32s_16s* pState, Ipp32s* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine32s_16s, (IppsIIRState32s_16s* pState, const Ipp32s* pDlyLine))

IPPAPI(IppStatus, ippsIIRGetDlyLine32sc_16sc, (const IppsIIRState32sc_16sc* pState, Ipp32sc* pDlyLine))
IPPAPI(IppStatus, ippsIIRSetDlyLine32sc_16sc, (IppsIIRState32sc_16sc* pState, const Ipp32sc* pDlyLine))

/* /////////////////////////////////////////////////////////////////////////////
//  Filtering
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:         ippsIIROne
//  Purpose:       IIR filter. One sample operation
//  Parameters:
//      pState              - pointer to the filter context
//      src                 - the input sample
//      pDstVal             - pointer to the output sample
//      scaleFactor         - scale factor value
//  Return:
//      ippStsContextMatchErr  - wrong context
//      ippStsNullPtrErr       - pointer(s) to pState or pDstVal is NULL
//      ippStsNoErr            - otherwise
//
//  Note: Don't modify scaleFactor value unless context is changed
*/

IPPAPI(IppStatus, ippsIIROne32s_16s_Sfs, (Ipp16s src, Ipp16s* pDstVal, IppsIIRState32s_16s* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIROne32sc_16sc_Sfs, (Ipp16sc src, Ipp16sc* pDstVal, IppsIIRState32sc_16sc* pState, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:        ippsIIR
//  Purpose:      IIR filter. Vector filtering
//  Parameters:
//      pState              - pointer to the filter context
//      pSrc                - pointer to input data
//      pSrcDst             - pointer to input/ouput data
//      pDst                - pointer to output data
//      len                 - length of the vectors
//      scaleFactor         - scale factor value
//  Return:
//      ippStsContextMatchErr  - wrong context
//      ippStsNullPtrErr       - pointer(s) pState or pSrc or pDst is NULL
//      ippStsSizeErr          - length of the vectors <= 0
//      ippStsNoErr            - otherwise
//
//  Note: Don't modify scaleFactor value unless context is changed
*/

IPPAPI(IppStatus, ippsIIR32s_16s_Sfs, (const Ipp16s* pSrc, Ipp16s* pDst, int len,
       IppsIIRState32s_16s* pState, int scaleFactor))
IPPAPI(IppStatus, ippsIIR32sc_16sc_Sfs, (const Ipp16sc* pSrc, Ipp16sc* pDst, int len,
       IppsIIRState32sc_16sc* pState, int scaleFactor))

IPPAPI(IppStatus, ippsIIR32s_16s_ISfs, (Ipp16s* pSrcDst, int len, IppsIIRState32s_16s* pState,
       int scaleFactor))
IPPAPI(IppStatus, ippsIIR32sc_16sc_ISfs, (Ipp16sc* pSrcDst, int len, IppsIIRState32sc_16sc* pState,
       int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:  ippsIIR_Direct_16s, ippsIIR_Direct_16s_I,
//          ippsIIROne_Direct_16s, ippsIIROne_Direct_16s_I,
//          ippsIIR_BiQuadDirect_16s, ippsIIR_BiQuadDirect_16s_I,
//          ippsIIROne_BiQuadDirect_16s, ippsIIROne_BiQuadDirect_16s_I.
//  Purpose: IIR filter with 16s taps. One sample (with suffix One), or vector
//           operation, direct (without State structure) form. Suffix "BiQuad"
//           means numBq-section filter, else the arbitrary coefficients IIR
//           filter.
//  Parameters:
//      pSrc        - pointer to the input array.
//      src         - input sample in 'One' case.
//      pDst        - pointer to the output array.
//      pDstVal     - pointer to the output sample in 'One' case.
//      pSrcDst     - pointer to the input and output array for the in-place
//                                                                   operation.
//      pSrcDstVal  - pointer to the input and output sample for in-place
//                                                     operation in 'One' case.
//      pTaps       - pointer to filter coefficients
//      order       - arbitrary filter order
//      numBq       - number biquads of BQ filter
//      pDlyLine    - pointer to delay line data
//  Return: IppStatus
//      ippStsNullPtrErr    - pointer(s) to the data is NULL
//      ippStsIIROrderErr   - filter order < 0
//      ippStsScaleRangeErr - if A(0) < 0, see "Note..."
//      ippStsMemAllocErr   - memory allocation error
//      ippStsSizeErr       - length of the vectors <= 0
//      ippStsNoErr         - otherwise
//
//  Order of the coefficients in the input taps buffer for the arbitrary
//                                                                      filter:
//     B(0),B(1),B(2)..,B(order);
//     A(0),A(1),A(2)..,A(order);
//     . . .
//  Note:
//      A(0) >= 0, and means the scale factor (not divisor !) for all the
//                                                                  other taps.
//  Order of the coefficients in the input taps buffer for BiQuad-section
//                                                                      filter:
//     B(0,0),B(0,1),B(0,2),A(0,0),A(0,1),A(0,2);
//     B(1,0),B(1,1),B(1,2),A(1,0),A(1,1),A(1,2);
//     ........
//  Note:
//      A(0,0) >= 0, A(1,0) >= 0..., and means the scale factor (not divisor !)
//      for all the other taps of each section.
*/

IPPAPI( IppStatus, ippsIIR_Direct_16s,( const Ipp16s* pSrc, Ipp16s* pDst,
                 int len, const Ipp16s* pTaps, int order, Ipp32s* pDlyLine ))
IPPAPI( IppStatus, ippsIIR_Direct_16s_I,( Ipp16s* pSrcDst, int len,
                          const Ipp16s* pTaps, int order, Ipp32s* pDlyLine ))
IPPAPI( IppStatus, ippsIIROne_Direct_16s,( Ipp16s src, Ipp16s* pDstVal,
                          const Ipp16s* pTaps, int order, Ipp32s* pDlyLine ))
IPPAPI( IppStatus, ippsIIROne_Direct_16s_I,( Ipp16s* pSrcDst,
                          const Ipp16s* pTaps, int order, Ipp32s* pDlyLine ))

IPPAPI( IppStatus, ippsIIR_BiQuadDirect_16s,( const Ipp16s* pSrc, Ipp16s* pDst,
             int len, const Ipp16s* pTaps, int numBq, Ipp32s* pDlyLine ))
IPPAPI( IppStatus, ippsIIR_BiQuadDirect_16s_I,( Ipp16s* pSrcDst, int len,
                     const Ipp16s * pTaps, int numBq, Ipp32s* pDlyLine ))
IPPAPI( IppStatus, ippsIIROne_BiQuadDirect_16s,( Ipp16s src, Ipp16s* pDstVal,
                      const Ipp16s* pTaps, int numBq, Ipp32s* pDlyLine ))
IPPAPI( IppStatus, ippsIIROne_BiQuadDirect_16s_I,( Ipp16s* pSrcDstVal,
                      const Ipp16s* pTaps, int numBq, Ipp32s* pDlyLine ))



/* ////////////////////////////////////////////////////////////////////////////
//          Initialize IIR state with external memory buffer
//////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////////
//  Name:         ippsIIRGetStateSize, ippsIIRGetStateSize_BiQuad,
//                ippsIIRGetStateSize_BiQuad_DF1_32f,
//                ippsIIRInit, ippsIIRInit_BiQuad,
//                ippsIIRInit_BiQuad_DF1_32f
//
//  Purpose:      ippsIIRGetStateSize - calculates the size of the IIR State
//                                                                   structure;
//                ippsIIRInit - initialize IIR state - set taps and delay line
//                using external memory buffer;
//  Parameters:
//      pTaps       - pointer to the filter coefficients;
//      order       - order of the filter;
//      numBq       - order of the filter;
//      pDlyLine    - pointer to the delay line values, can be NULL;
//      ppState     - double pointer to the IIR state created or NULL;
//      tapsFactor  - scaleFactor for taps (integer version);
//      pBufferSize - pointer where to store the calculated IIR State structure
//                                                             size (in bytes);
//   Return:
//      status      - status value returned, its value are
//         ippStsNullPtrErr       - pointer(s) to the data is NULL
//         ippStsIIROrderErr      - order <= 0 or numBq < 1
//         ippStsNoErr            - otherwise
*/

/* ******************************** 32s_16s ******************************** */
IPPAPI( IppStatus, ippsIIRGetStateSize32s_16s,( int order, int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize32sc_16sc,( int order,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize32s_BiQuad_16s,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize32sc_BiQuad_16sc,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRInit32s_16s,( IppsIIRState32s_16s** ppState,
                                const Ipp32s* pTaps, int order, int tapsFactor,
                                         const Ipp32s* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit32sc_16sc,( IppsIIRState32sc_16sc** ppState,
                               const Ipp32sc* pTaps, int order, int tapsFactor,
                                        const Ipp32sc* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit32s_BiQuad_16s,( IppsIIRState32s_16s** ppState,
                                const Ipp32s* pTaps, int numBq, int tapsFactor,
                                         const Ipp32s* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit32sc_BiQuad_16sc,(
                   IppsIIRState32sc_16sc** ppState, const Ipp32sc* pTaps,
             int numBq, int tapsFactor, const Ipp32sc* pDlyLine, Ipp8u* pBuf ))

/* ****************************** 32s_16s32f ******************************* */
IPPAPI( IppStatus, ippsIIRGetStateSize32s_16s32f,( int order,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize32sc_16sc32fc,( int order,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize32s_BiQuad_16s32f,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize32sc_BiQuad_16sc32fc,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRInit32s_16s32f,( IppsIIRState32s_16s** ppState,
         const Ipp32f* pTaps, int order, const Ipp32s* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit32sc_16sc32fc,( IppsIIRState32sc_16sc** ppState,
                                               const Ipp32fc* pTaps, int order,
                                        const Ipp32sc* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit32s_BiQuad_16s32f,( IppsIIRState32s_16s** ppState,
                                                const Ipp32f* pTaps, int numBq,
                                         const Ipp32s* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit32sc_BiQuad_16sc32fc,(
                          IppsIIRState32sc_16sc** ppState, const Ipp32fc* pTaps,
                             int numBq, const Ipp32sc* pDlyLine, Ipp8u* pBuf ))
/* ********************************** 32f ********************************** */
IPPAPI( IppStatus, ippsIIRGetStateSize_32f,( int order, int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize_32fc,( int order, int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize_BiQuad_32f,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize_BiQuad_DF1_32f,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize_BiQuad_32fc,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRInit_32f,( IppsIIRState_32f** ppState,
         const Ipp32f* pTaps, int order, const Ipp32f* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit_32fc,( IppsIIRState_32fc** ppState,
       const Ipp32fc* pTaps, int order, const Ipp32fc* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit_BiQuad_32f,( IppsIIRState_32f** ppState,
         const Ipp32f* pTaps, int numBq, const Ipp32f* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit_BiQuad_DF1_32f,( IppsIIRState_32f** pState,
         const Ipp32f* pTaps, int numBq, const Ipp32f* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit_BiQuad_32fc,( IppsIIRState_32fc** ppState,
       const Ipp32fc* pTaps, int numBq, const Ipp32fc* pDlyLine, Ipp8u* pBuf ))
/* ******************************** 32f_16s ******************************** */
IPPAPI( IppStatus, ippsIIRGetStateSize32f_16s,( int order, int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize32fc_16sc,( int order,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize32f_BiQuad_16s,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize32fc_BiQuad_16sc,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRInit32f_16s,( IppsIIRState32f_16s** ppState,
         const Ipp32f* pTaps, int order, const Ipp32f* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit32fc_16sc,( IppsIIRState32fc_16sc** ppState,
       const Ipp32fc* pTaps, int order, const Ipp32fc* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit32f_BiQuad_16s,( IppsIIRState32f_16s** ppState,
         const Ipp32f* pTaps, int numBq, const Ipp32f* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit32fc_BiQuad_16sc,( IppsIIRState32fc_16sc** ppState,
       const Ipp32fc* pTaps, int numBq, const Ipp32fc* pDlyLine, Ipp8u* pBuf ))
/* ********************************** 64f ********************************** */
IPPAPI( IppStatus, ippsIIRGetStateSize_64f,( int order, int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize_64fc,( int order, int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize_BiQuad_64f,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize_BiQuad_64fc,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRInit_64f,( IppsIIRState_64f** ppState,
         const Ipp64f* pTaps, int order, const Ipp64f* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit_64fc,( IppsIIRState_64fc** ppState,
       const Ipp64fc* pTaps, int order, const Ipp64fc* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit_BiQuad_64f,( IppsIIRState_64f** ppState,
         const Ipp64f* pTaps, int numBq, const Ipp64f* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit_BiQuad_64fc,( IppsIIRState_64fc** ppState,
       const Ipp64fc* pTaps, int numBq, const Ipp64fc* pDlyLine, Ipp8u* pBuf ))
/* ******************************** 64f_16s ******************************** */
IPPAPI( IppStatus, ippsIIRGetStateSize64f_16s,( int order, int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize64fc_16sc,( int order,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize64f_BiQuad_16s,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize64fc_BiQuad_16sc,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRInit64f_16s,( IppsIIRState64f_16s** ppState,
         const Ipp64f* pTaps, int order, const Ipp64f* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit64fc_16sc,( IppsIIRState64fc_16sc** ppState,
       const Ipp64fc* pTaps, int order, const Ipp64fc* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit64f_BiQuad_16s,( IppsIIRState64f_16s** ppState,
         const Ipp64f* pTaps, int numBq, const Ipp64f* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit64fc_BiQuad_16sc,( IppsIIRState64fc_16sc** ppState,
       const Ipp64fc* pTaps, int numBq, const Ipp64fc* pDlyLine, Ipp8u* pBuf ))
/* ******************************** 64f_32s ******************************** */
IPPAPI( IppStatus, ippsIIRGetStateSize64f_32s,( int order, int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize64fc_32sc,( int order,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize64f_BiQuad_32s,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize64f_BiQuad_DF1_32s,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize64fc_BiQuad_32sc,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRInit64f_32s,( IppsIIRState64f_32s** ppState,
         const Ipp64f* pTaps, int order, const Ipp64f* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit64fc_32sc,( IppsIIRState64fc_32sc** ppState,
       const Ipp64fc* pTaps, int order, const Ipp64fc* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit64f_BiQuad_32s,( IppsIIRState64f_32s** ppState,
         const Ipp64f* pTaps, int numBq, const Ipp64f* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit64f_BiQuad_DF1_32s,( IppsIIRState64f_32s** ppState,
         const Ipp64f* pTaps, int numBq, const Ipp32s* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit64fc_BiQuad_32sc,( IppsIIRState64fc_32sc** ppState,
       const Ipp64fc* pTaps, int numBq, const Ipp64fc* pDlyLine, Ipp8u* pBuf ))
/* ******************************** 64f_32f ******************************** */
IPPAPI( IppStatus, ippsIIRGetStateSize64f_32f,( int order, int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize64fc_32fc,( int order,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize64f_BiQuad_32f,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRGetStateSize64fc_BiQuad_32fc,( int numBq,
                                                            int *pBufferSize ))
IPPAPI( IppStatus, ippsIIRInit64f_32f,( IppsIIRState64f_32f** ppState,
         const Ipp64f* pTaps, int order, const Ipp64f* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit64fc_32fc,( IppsIIRState64fc_32fc** ppState,
       const Ipp64fc* pTaps, int order, const Ipp64fc* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit64f_BiQuad_32f,( IppsIIRState64f_32f** ppState,
         const Ipp64f* pTaps, int numBq, const Ipp64f* pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsIIRInit64fc_BiQuad_32fc,( IppsIIRState64fc_32fc** ppState,
       const Ipp64fc* pTaps, int numBq, const Ipp64fc* pDlyLine, Ipp8u* pBuf ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:              ippsIIRSetTaps
//  Purpose:            set new IIR taps values to state
//  Parameters:
//      pTaps       -   pointer to new IIR taps
//      pState      -   pointer to the IIR filter state
//      tapsFactor  -   scaleFactor for taps (integer version only)
//  Return:
//      ippStsContextMatchErr  -   wrong state identifier
//      ippStsNullPtrErr       -   pointer(s) to the data is NULL
//      ippStsNoErr            -   otherwise
*/
IPPAPI( IppStatus, ippsIIRSetTaps_32f,( const Ipp32f *pTaps,
                                                    IppsIIRState_32f* pState ))
IPPAPI( IppStatus, ippsIIRSetTaps_32fc,( const Ipp32fc *pTaps,
                                                   IppsIIRState_32fc* pState ))
IPPAPI( IppStatus, ippsIIRSetTaps32f_16s,( const Ipp32f *pTaps,
                                                 IppsIIRState32f_16s* pState ))
IPPAPI( IppStatus, ippsIIRSetTaps32fc_16sc,( const Ipp32fc *pTaps,
                                               IppsIIRState32fc_16sc* pState ))
IPPAPI( IppStatus, ippsIIRSetTaps32s_16s,( const Ipp32s *pTaps,
                                 IppsIIRState32s_16s* pState, int tapsFactor ))
IPPAPI( IppStatus, ippsIIRSetTaps32sc_16sc,( const Ipp32sc *pTaps,
                               IppsIIRState32sc_16sc* pState, int tapsFactor ))
IPPAPI( IppStatus, ippsIIRSetTaps32s_16s32f,( const Ipp32f *pTaps,
                                                 IppsIIRState32s_16s* pState ))
IPPAPI( IppStatus, ippsIIRSetTaps32sc_16sc32fc,( const Ipp32fc *pTaps,
                                               IppsIIRState32sc_16sc* pState ))
IPPAPI( IppStatus, ippsIIRSetTaps_64f,( const Ipp64f *pTaps,
                                                    IppsIIRState_64f* pState ))
IPPAPI( IppStatus, ippsIIRSetTaps_64fc,( const Ipp64fc *pTaps,
                                                   IppsIIRState_64fc* pState ))
IPPAPI( IppStatus, ippsIIRSetTaps64f_32f,( const Ipp64f *pTaps,
                                                 IppsIIRState64f_32f* pState ))
IPPAPI( IppStatus, ippsIIRSetTaps64fc_32fc,( const Ipp64fc *pTaps,
                                               IppsIIRState64fc_32fc* pState ))
IPPAPI( IppStatus, ippsIIRSetTaps64f_32s,( const Ipp64f *pTaps,
                                                 IppsIIRState64f_32s* pState ))
IPPAPI( IppStatus, ippsIIRSetTaps64fc_32sc,( const Ipp64fc *pTaps,
                                               IppsIIRState64fc_32sc* pState ))
IPPAPI( IppStatus, ippsIIRSetTaps64f_16s,( const Ipp64f *pTaps,
                                                 IppsIIRState64f_16s* pState ))
IPPAPI( IppStatus, ippsIIRSetTaps64fc_16sc,( const Ipp64fc *pTaps,
                                               IppsIIRState64fc_16sc* pState ))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsIIRGenLowpass_64f, ippsIIRGenHighpass_64f
//
//  Purpose:    This function computes the highpass and lowpass IIR filter coefficients
//
//  Parameters:
//      rFreq             cut off frequency (0 < rFreq < 0.5)
//
//      ripple            possible ripple in pass band for ippChebyshev1 type of filter
//
//      order             the order of future filter (1 <= order <= 12)
//
//      pTaps             pointer to the array which specifies
//                        the filter coefficients
//
//      filterType        type of required filter (ippButterworth or ippChebyshev1)
//
//
//  Return:
//   ippStsNullPtrErr           the null pointer to taps[] array pass to function
//   ippStsIIRPassbandRippleErr the ripple in passband for Chebyshev1 design is less zero,
//                              equal to zero or greater than 29
//   ippStsFilterFrequencyErr   the cut of frequency of filter is less zero, equal to zero
//                              or greater than 0.5
//   ippStsIIRGenOrderErr       the order of an IIR filter for design them is less than one
//                              or greater than 12
//   ippStsNoErr                otherwise
//
*/

IPPAPI(IppStatus, ippsIIRGenLowpass_64f, ( Ipp64f rFreq, Ipp64f ripple, int order, Ipp64f* pTaps, IppsIIRFilterType filterType))

IPPAPI(IppStatus, ippsIIRGenHighpass_64f,( Ipp64f rFreq, Ipp64f ripple, int order, Ipp64f* pTaps, IppsIIRFilterType filterType))


/* /////////////////////////////////////////////////////////////////////////////
//                     FIR filters (float and double taps versions)
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )

struct FIRState_32f;
typedef struct FIRState_32f IppsFIRState_32f;

struct FIRState_32fc;
typedef struct FIRState_32fc IppsFIRState_32fc;

struct FIRState32f_16s;
typedef struct FIRState32f_16s IppsFIRState32f_16s;

struct FIRState32fc_16sc;
typedef struct FIRState32fc_16sc IppsFIRState32fc_16sc;

struct FIRState_64f;
typedef struct FIRState_64f IppsFIRState_64f;

struct FIRState_64fc;
typedef struct FIRState_64fc IppsFIRState_64fc;

struct FIRState64f_32f;
typedef struct FIRState64f_32f IppsFIRState64f_32f;

struct FIRState64fc_32fc;
typedef struct FIRState64fc_32fc IppsFIRState64fc_32fc;

struct FIRState64f_32s;
typedef struct FIRState64f_32s IppsFIRState64f_32s;

struct FIRState64fc_32sc;
typedef struct FIRState64fc_32sc IppsFIRState64fc_32sc;

struct FIRState64f_16s;
typedef struct FIRState64f_16s IppsFIRState64f_16s;

struct FIRState64fc_16sc;
typedef struct FIRState64fc_16sc IppsFIRState64fc_16sc;

struct FIRState32s_16s;
typedef struct FIRState32s_16s IppsFIRState32s_16s;

struct FIRState32sc_16sc;
typedef struct FIRState32sc_16sc IppsFIRState32sc_16sc;

struct FIRState_32s;
typedef struct FIRState_32s IppsFIRState_32s;

struct FIRState_16s;
typedef struct FIRState_16s IppsFIRState_16s;

#endif /* _OWN_BLDPCS */

/* /////////////////////////////////////////////////////////////////////////////
//  Initialize FIR state
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:         ippsFIRInitAlloc, ippsFIRMRInitAlloc, ippsFIRFree
//  Purpose:      create and initialize FIR state - set taps and delay line
//                and close it
//  Parameters:
//      pTaps       - pointer to the filter coefficients
//      tapsLen     - number of coefficients
//      pDlyLine    - pointer to the delay line values, can be NULL
//      state       - pointer to the FIR state created or NULL;
//   Return:
//      status      - status value returned, its value are
//         ippStsMemAllocErr      - memory allocation error
//         ippStsNullPtrErr       - pointer(s) to the data is NULL
//         ippStsFIRLenErr        - tapsLen <= 0
//         ippStsFIRMRFactorErr   - factor <= 0
//         ippStsFIRMRPhaseErr    - phase < 0 || factor <= phase
//         ippStsContextMatchErr  - wrong state identifier
//         ippStsNoErr            - otherwise
*/

IPPAPI(IppStatus, ippsFIRInitAlloc_32f, (IppsFIRState_32f** pState,
        const Ipp32f* pTaps, int tapsLen, const Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc_32f, (IppsFIRState_32f** pState,
        const Ipp32f* pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsFIRInitAlloc_32fc, (IppsFIRState_32fc** pState,
        const Ipp32fc* pTaps, int tapsLen, const Ipp32fc* pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc_32fc, (IppsFIRState_32fc** pState,
        const Ipp32fc* pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp32fc* pDlyLine))

IPPAPI(IppStatus, ippsFIRInitAlloc32f_16s, (IppsFIRState32f_16s** pState,
        const Ipp32f* pTaps, int tapsLen, const Ipp16s* pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc32f_16s, (IppsFIRState32f_16s** pState,
        const Ipp32f* pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp16s* pDlyLine))
IPPAPI(IppStatus, ippsFIRInitAlloc32fc_16sc, (IppsFIRState32fc_16sc** pState,
        const Ipp32fc* pTaps, int tapsLen, const Ipp16sc* pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc32fc_16sc, (IppsFIRState32fc_16sc** pState,
        const Ipp32fc* pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp16sc* pDlyLine))

IPPAPI(IppStatus, ippsFIRInitAlloc_64f, (IppsFIRState_64f** pState,
        const Ipp64f* pTaps, int tapsLen, const Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc_64f, (IppsFIRState_64f** pState,
        const Ipp64f* pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsFIRInitAlloc_64fc, (IppsFIRState_64fc** pState,
        const Ipp64fc* pTaps, int tapsLen, const Ipp64fc* pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc_64fc, (IppsFIRState_64fc** pState,
        const Ipp64fc* pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp64fc* pDlyLine))

IPPAPI(IppStatus, ippsFIRInitAlloc64f_32f, (IppsFIRState64f_32f** pState,
        const Ipp64f* pTaps, int tapsLen, const Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc64f_32f, (IppsFIRState64f_32f** pState,
        const Ipp64f* pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsFIRInitAlloc64fc_32fc, (IppsFIRState64fc_32fc** pState,
        const Ipp64fc* pTaps, int tapsLen, const Ipp32fc* pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc64fc_32fc, (IppsFIRState64fc_32fc** pState,
        const Ipp64fc* pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp32fc* pDlyLine))

IPPAPI(IppStatus, ippsFIRInitAlloc64f_32s, (IppsFIRState64f_32s** pState,
        const Ipp64f* pTaps, int tapsLen, const Ipp32s* pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc64f_32s, (IppsFIRState64f_32s** pState,
        const Ipp64f* pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp32s* pDlyLine))
IPPAPI(IppStatus, ippsFIRInitAlloc64fc_32sc, (IppsFIRState64fc_32sc** pState,
        const Ipp64fc* pTaps, int tapsLen, const Ipp32sc* pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc64fc_32sc, (IppsFIRState64fc_32sc** pState,
        const Ipp64fc* pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp32sc* pDlyLine))

IPPAPI(IppStatus, ippsFIRInitAlloc64f_16s, (IppsFIRState64f_16s** pState,
        const Ipp64f* pTaps, int tapsLen, const Ipp16s* pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc64f_16s, (IppsFIRState64f_16s** pState,
        const Ipp64f* pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp16s* pDlyLine))
IPPAPI(IppStatus, ippsFIRInitAlloc64fc_16sc, (IppsFIRState64fc_16sc** pState,
        const Ipp64fc* pTaps, int tapsLen, const Ipp16sc* pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc64fc_16sc, (IppsFIRState64fc_16sc** pState,
        const Ipp64fc* pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp16sc* pDlyLine))

IPPAPI(IppStatus, ippsFIRFree_32f, (IppsFIRState_32f* pState))

IPPAPI(IppStatus, ippsFIRFree_32fc, (IppsFIRState_32fc* pState))

IPPAPI(IppStatus, ippsFIRFree32f_16s, (IppsFIRState32f_16s* pState))

IPPAPI(IppStatus, ippsFIRFree32fc_16sc, (IppsFIRState32fc_16sc* pState))

IPPAPI(IppStatus, ippsFIRFree_64f, (IppsFIRState_64f* pState))

IPPAPI(IppStatus, ippsFIRFree_64fc, (IppsFIRState_64fc* pState))

IPPAPI(IppStatus, ippsFIRFree64f_32f, (IppsFIRState64f_32f* pState))

IPPAPI(IppStatus, ippsFIRFree64fc_32fc, (IppsFIRState64fc_32fc* pState))

IPPAPI(IppStatus, ippsFIRFree64f_32s, (IppsFIRState64f_32s* pState))

IPPAPI(IppStatus, ippsFIRFree64fc_32sc, (IppsFIRState64fc_32sc* pState))

IPPAPI(IppStatus, ippsFIRFree64f_16s, (IppsFIRState64f_16s* pState))

IPPAPI(IppStatus, ippsFIRFree64fc_16sc, (IppsFIRState64fc_16sc* pState))

/* ////////////////////////////////////////////////////////////////////////////
//          Initialize FIR state with external memory buffer
//////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////////
//  Name:         ippsFIRGetStateSize, ippsFIRMRGetStateSize,
//                ippsFIRInit, ippsFIRMRInit
//  Purpose:      ippsFIRGetStateSize - calculates the size of the FIR State
//                                                                   structure;
//                ippsFIRInit - initialize FIR state - set taps and delay line
//                using external memory buffer;
//  Parameters:
//      pTaps       - pointer to the filter coefficients;
//      tapsLen     - number of coefficients;
//      pDlyLine    - pointer to the delay line values, can be NULL;
//      ppState     - pointer to the FIR state created or NULL;
//      upFactor    - multi-rate up factor;
//      upPhase     - multi-rate up phase;
//      downFactor  - multi-rate down factor;
//      downPhase   - multi-rate down phase;
//      pStateSize  - pointer where to store the calculated FIR State structure
//                                                             size (in bytes);
//   Return:
//      status      - status value returned, its value are
//         ippStsNullPtrErr       - pointer(s) to the data is NULL
//         ippStsFIRLenErr        - tapsLen <= 0
//         ippStsFIRMRFactorErr   - factor <= 0
//         ippStsFIRMRPhaseErr    - phase < 0 || factor <= phase
//         ippStsNoErr            - otherwise
*/

/* ******************************** 32s_16s ******************************** */
IPPAPI( IppStatus, ippsFIRGetStateSize32s_16s,( int tapsLen, int* pStateSize ))
IPPAPI( IppStatus, ippsFIRInit32s_16s,( IppsFIRState32s_16s** ppState,
      const Ipp32s *pTaps, int tapsLen, int tapsFactor, const Ipp16s *pDlyLine,
                                                             Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize32s_16s,( int tapsLen, int upFactor,
                                            int downFactor, int *pStateSize ))
IPPAPI( IppStatus, ippsFIRMRInit32s_16s,( IppsFIRState32s_16s** ppState,
   const Ipp32s *pTaps, int tapsLen, int tapsFactor, int upFactor, int upPhase,
         int downFactor, int downPhase, const Ipp16s *pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRInit32sc_16sc,( IppsFIRState32sc_16sc** ppState,
                             const Ipp32sc *pTaps, int tapsLen, int tapsFactor,
                                       const Ipp16sc *pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize32sc_16sc,( int tapsLen, int upFactor,
                                            int downFactor, int* pStateSize ))
IPPAPI( IppStatus, ippsFIRMRInit32sc_16sc,( IppsFIRState32sc_16sc** ppState,
  const Ipp32sc *pTaps, int tapsLen, int tapsFactor, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp16sc *pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRGetStateSize32sc_16sc32fc,( int tapsLen,
                                                            int *pStateSize ))
/* ****************************** 32s_16s32f ******************************* */
IPPAPI( IppStatus, ippsFIRGetStateSize32s_16s32f,( int tapsLen,
                                                            int* pStateSize ))
IPPAPI( IppStatus, ippsFIRInit32s_16s32f,( IppsFIRState32s_16s** ppState,
      const Ipp32f *pTaps, int tapsLen, const Ipp16s *pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize32s_16s32f,( int tapsLen, int upFactor,
                                            int downFactor, int *pStateSize ))
IPPAPI( IppStatus, ippsFIRMRInit32s_16s32f,( IppsFIRState32s_16s** ppState,
                   const Ipp32f *pTaps, int tapsLen, int upFactor, int upPhase,
         int downFactor, int downPhase, const Ipp16s *pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRGetStateSize32sc_16sc,( int tapsLen,
                                                            int *pStateSize ))
IPPAPI( IppStatus, ippsFIRInit32sc_16sc32fc,( IppsFIRState32sc_16sc** ppState,
    const Ipp32fc *pTaps, int tapsLen, const Ipp16sc *pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize32sc_16sc32fc,( int tapsLen,
                              int upFactor, int downFactor, int *pStateSize ))
IPPAPI( IppStatus, ippsFIRMRInit32sc_16sc32fc,( IppsFIRState32sc_16sc** ppState,
                  const Ipp32fc *pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp16sc *pDlyLine, Ipp8u* pBuffer ))
/* ********************************** 32f ********************************** */
IPPAPI( IppStatus, ippsFIRInit_32f,( IppsFIRState_32f** ppState,
   const Ipp32f *pTaps, int tapsLen, const Ipp32f *pDlyLine, Ipp8u *pBuffer ))
IPPAPI( IppStatus, ippsFIRInit_32fc,( IppsFIRState_32fc** ppState,
 const Ipp32fc *pTaps, int tapsLen, const Ipp32fc *pDlyLine, Ipp8u *pBuffer ))
IPPAPI( IppStatus, ippsFIRGetStateSize_32f,( int tapsLen, int *pBufferSize ))
IPPAPI( IppStatus, ippsFIRGetStateSize_32fc,( int tapsLen, int *pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRInit_32f,( IppsFIRState_32f** ppState,
                   const Ipp32f* pTaps, int tapsLen, int upFactor, int upPhase,
      int downFactor, int downPhase, const Ipp32f* pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize_32f,( int tapsLen, int upFactor,
                                           int downFactor, int *pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize_32fc,( int tapsLen, int upFactor,
                                           int downFactor, int *pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRInit_32fc,( IppsFIRState_32fc** ppState,
                  const Ipp32fc *pTaps, int tapsLen, int upFactor, int upPhase,
     int downFactor, int downPhase, const Ipp32fc *pDlyLine, Ipp8u* pBuffer ))
/* ******************************** 32f_16s ******************************** */
IPPAPI( IppStatus, ippsFIRGetStateSize32f_16s,( int tapsLen,
                                                           int* pBufferSize ))
IPPAPI( IppStatus, ippsFIRInit32f_16s,( IppsFIRState32f_16s** ppState,
   const Ipp32f *pTaps, int tapsLen, const Ipp16s *pDlyLine, Ipp8u* pBuffer ))
IPPAPI(IppStatus, ippsFIRGetStateSize32fc_16sc, ( int tapsLen,
                                                           int *pBufferSize ))
IPPAPI(IppStatus, ippsFIRInit32fc_16sc, (IppsFIRState32fc_16sc** ppState,
 const Ipp32fc *pTaps, int tapsLen, const Ipp16sc *pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize32f_16s,( int tapsLen, int upFactor,
                                           int downFactor, int* pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRInit32f_16s,( IppsFIRState32f_16s** ppState,
                   const Ipp32f* pTaps, int tapsLen, int upFactor, int upPhase,
      int downFactor, int downPhase, const Ipp16s* pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize32fc_16sc,( int tapsLen, int upFactor,
                                           int downFactor, int* pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRInit32fc_16sc,( IppsFIRState32fc_16sc** ppState,
                  const Ipp32fc *pTaps, int tapsLen, int upFactor, int upPhase,
     int downFactor, int downPhase, const Ipp16sc *pDlyLine, Ipp8u* pBuffer ))
/* ********************************** 64f ********************************** */
IPPAPI( IppStatus, ippsFIRInit_64f,( IppsFIRState_64f** ppState,
   const Ipp64f *pTaps, int tapsLen, const Ipp64f *pDlyLine, Ipp8u *pBuffer ))
IPPAPI( IppStatus, ippsFIRInit_64fc,( IppsFIRState_64fc** ppState,
 const Ipp64fc *pTaps, int tapsLen, const Ipp64fc *pDlyLine, Ipp8u *pBuffer ))
IPPAPI( IppStatus, ippsFIRGetStateSize_64f,( int tapsLen, int *pBufferSize ))
IPPAPI( IppStatus, ippsFIRGetStateSize_64fc,( int tapsLen, int *pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRInit_64f,( IppsFIRState_64f** ppState,
                   const Ipp64f* pTaps, int tapsLen, int upFactor, int upPhase,
      int downFactor, int downPhase, const Ipp64f* pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize_64f,( int tapsLen, int upFactor,
                                           int downFactor, int *pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize_64fc,( int tapsLen, int upFactor,
                                           int downFactor, int *pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRInit_64fc,( IppsFIRState_64fc** ppState,
                  const Ipp64fc *pTaps, int tapsLen, int upFactor, int upPhase,
     int downFactor, int downPhase, const Ipp64fc *pDlyLine, Ipp8u* pBuffer ))
/* ******************************** 64f_16s ******************************** */
IPPAPI( IppStatus, ippsFIRGetStateSize64f_16s,( int tapsLen,
                                                           int* pBufferSize ))
IPPAPI( IppStatus, ippsFIRInit64f_16s,( IppsFIRState64f_16s** ppState,
   const Ipp64f *pTaps, int tapsLen, const Ipp16s *pDlyLine, Ipp8u* pBuffer ))
IPPAPI(IppStatus, ippsFIRGetStateSize64fc_16sc, ( int tapsLen,
                                                           int *pBufferSize ))
IPPAPI(IppStatus, ippsFIRInit64fc_16sc, (IppsFIRState64fc_16sc** ppState,
 const Ipp64fc *pTaps, int tapsLen, const Ipp16sc *pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize64f_16s,( int tapsLen, int upFactor,
                                           int downFactor, int* pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRInit64f_16s,( IppsFIRState64f_16s** ppState,
                   const Ipp64f* pTaps, int tapsLen, int upFactor, int upPhase,
      int downFactor, int downPhase, const Ipp16s* pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize64fc_16sc,( int tapsLen, int upFactor,
                                           int downFactor, int* pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRInit64fc_16sc,( IppsFIRState64fc_16sc** ppState,
                  const Ipp64fc *pTaps, int tapsLen, int upFactor, int upPhase,
     int downFactor, int downPhase, const Ipp16sc *pDlyLine, Ipp8u* pBuffer ))
/* ******************************** 64f_32s ******************************** */
IPPAPI( IppStatus, ippsFIRGetStateSize64f_32s,( int tapsLen,
                                                           int* pBufferSize ))
IPPAPI( IppStatus, ippsFIRInit64f_32s,( IppsFIRState64f_32s** ppState,
   const Ipp64f *pTaps, int tapsLen, const Ipp32s *pDlyLine, Ipp8u* pBuffer ))
IPPAPI(IppStatus, ippsFIRGetStateSize64fc_32sc, ( int tapsLen,
                                                           int *pBufferSize ))
IPPAPI(IppStatus, ippsFIRInit64fc_32sc, (IppsFIRState64fc_32sc** ppState,
 const Ipp64fc *pTaps, int tapsLen, const Ipp32sc *pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize64f_32s,( int tapsLen, int upFactor,
                                           int downFactor, int* pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRInit64f_32s,( IppsFIRState64f_32s** ppState,
                   const Ipp64f* pTaps, int tapsLen, int upFactor, int upPhase,
      int downFactor, int downPhase, const Ipp32s* pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize64fc_32sc,( int tapsLen, int upFactor,
                                           int downFactor, int* pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRInit64fc_32sc,( IppsFIRState64fc_32sc** ppState,
                  const Ipp64fc *pTaps, int tapsLen, int upFactor, int upPhase,
     int downFactor, int downPhase, const Ipp32sc *pDlyLine, Ipp8u* pBuffer ))
/* ******************************** 64f_32f ******************************** */
IPPAPI( IppStatus, ippsFIRGetStateSize64f_32f,( int tapsLen,
                                                           int* pBufferSize ))
IPPAPI( IppStatus, ippsFIRInit64f_32f,( IppsFIRState64f_32f** ppState,
   const Ipp64f *pTaps, int tapsLen, const Ipp32f *pDlyLine, Ipp8u* pBuffer ))
IPPAPI(IppStatus, ippsFIRGetStateSize64fc_32fc, ( int tapsLen,
                                                           int *pBufferSize ))
IPPAPI(IppStatus, ippsFIRInit64fc_32fc, (IppsFIRState64fc_32fc** ppState,
 const Ipp64fc *pTaps, int tapsLen, const Ipp32fc *pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize64f_32f,( int tapsLen, int upFactor,
                                           int downFactor, int* pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRInit64f_32f,( IppsFIRState64f_32f** ppState,
                   const Ipp64f* pTaps, int tapsLen, int upFactor, int upPhase,
      int downFactor, int downPhase, const Ipp32f* pDlyLine, Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize64fc_32fc,( int tapsLen, int upFactor,
                                           int downFactor, int* pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRInit64fc_32fc,( IppsFIRState64fc_32fc** ppState,
                  const Ipp64fc *pTaps, int tapsLen, int upFactor, int upPhase,
     int downFactor, int downPhase, const Ipp32fc *pDlyLine, Ipp8u* pBuffer ))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:              ippsFIRGetTaps
//  Purpose:            get FIR taps value from state
//  Parameters:
//      pTaps       -   pointer to buffer to get FIR taps
//      pState      -   pointer to the FIR filter state
//  Return:
//      ippStsContextMatchErr  -   wrong state identifier
//      ippStsNullPtrErr       -   pointer(s) to the data is NULL
//      ippStsNoErr            -   otherwise
*/

IPPAPI(IppStatus, ippsFIRGetTaps_32f, (const IppsFIRState_32f* pState, Ipp32f* pTaps))
IPPAPI(IppStatus, ippsFIRGetTaps_32fc, (const IppsFIRState_32fc* pState, Ipp32fc* pTaps))

IPPAPI(IppStatus, ippsFIRGetTaps32f_16s, (const IppsFIRState32f_16s* pState, Ipp32f* pTaps))
IPPAPI(IppStatus, ippsFIRGetTaps32fc_16sc, (const IppsFIRState32fc_16sc* pState, Ipp32fc* pTaps))

IPPAPI(IppStatus, ippsFIRGetTaps_64f, (const IppsFIRState_64f* pState, Ipp64f* pTaps))
IPPAPI(IppStatus, ippsFIRGetTaps_64fc, (const IppsFIRState_64fc* pState, Ipp64fc* pTaps))

IPPAPI(IppStatus, ippsFIRGetTaps64f_32f, (const IppsFIRState64f_32f* pState, Ipp64f* pTaps))
IPPAPI(IppStatus, ippsFIRGetTaps64fc_32fc, (const IppsFIRState64fc_32fc* pState, Ipp64fc* pTaps))

IPPAPI(IppStatus, ippsFIRGetTaps64f_32s, (const IppsFIRState64f_32s* pState, Ipp64f* pTaps))
IPPAPI(IppStatus, ippsFIRGetTaps64fc_32sc, (const IppsFIRState64fc_32sc* pState, Ipp64fc* pTaps))

IPPAPI(IppStatus, ippsFIRGetTaps64f_16s, (const IppsFIRState64f_16s* pState, Ipp64f* pTaps))
IPPAPI(IppStatus, ippsFIRGetTaps64fc_16sc, (const IppsFIRState64fc_16sc* pState, Ipp64fc* pTaps))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:              ippsFIRGSetTaps
//  Purpose:            set FIR taps value to state
//  Parameters:
//      pTaps       -   pointer to buffer to set FIR taps
//      pState      -   pointer to the FIR filter state
//  Return:
//      ippStsContextMatchErr  -   wrong state identifier
//      ippStsNullPtrErr       -   pointer(s) to the data is NULL
//      ippStsNoErr            -   otherwise
*/

IPPAPI( IppStatus, ippsFIRSetTaps_32f,( const Ipp32f *pTaps,
                                                    IppsFIRState_32f* pState ))
IPPAPI( IppStatus, ippsFIRSetTaps_32fc,( const Ipp32fc *pTaps,
                                                   IppsFIRState_32fc* pState ))
IPPAPI( IppStatus, ippsFIRSetTaps32f_16s,( const Ipp32f *pTaps,
                                                 IppsFIRState32f_16s* pState ))
IPPAPI( IppStatus, ippsFIRSetTaps32fc_16sc,( const Ipp32fc *pTaps,
                                               IppsFIRState32fc_16sc* pState ))
IPPAPI( IppStatus, ippsFIRSetTaps32s_16s,( const Ipp32s *pTaps,
                                 IppsFIRState32s_16s* pState, int tapsFactor ))
IPPAPI( IppStatus, ippsFIRSetTaps32sc_16sc,( const Ipp32sc *pTaps,
                               IppsFIRState32sc_16sc* pState, int tapsFactor ))
IPPAPI( IppStatus, ippsFIRSetTaps32s_16s32f,( const Ipp32f *pTaps,
                                                 IppsFIRState32s_16s* pState ))
IPPAPI( IppStatus, ippsFIRSetTaps32sc_16sc32fc,( const Ipp32fc *pTaps,
                                               IppsFIRState32sc_16sc* pState ))
IPPAPI( IppStatus, ippsFIRSetTaps_64f,( const Ipp64f *pTaps,
                                                    IppsFIRState_64f* pState ))
IPPAPI( IppStatus, ippsFIRSetTaps_64fc,( const Ipp64fc *pTaps,
                                                   IppsFIRState_64fc* pState ))
IPPAPI( IppStatus, ippsFIRSetTaps64f_32f,( const Ipp64f *pTaps,
                                                 IppsFIRState64f_32f* pState ))
IPPAPI( IppStatus, ippsFIRSetTaps64fc_32fc,( const Ipp64fc *pTaps,
                                               IppsFIRState64fc_32fc* pState ))
IPPAPI( IppStatus, ippsFIRSetTaps64f_32s,( const Ipp64f *pTaps,
                                                 IppsFIRState64f_32s* pState ))
IPPAPI( IppStatus, ippsFIRSetTaps64fc_32sc,( const Ipp64fc *pTaps,
                                               IppsFIRState64fc_32sc* pState ))
IPPAPI( IppStatus, ippsFIRSetTaps64f_16s,( const Ipp64f *pTaps,
                                                 IppsFIRState64f_16s* pState ))
IPPAPI( IppStatus, ippsFIRSetTaps64fc_16sc,( const Ipp64fc *pTaps,
                                               IppsFIRState64fc_16sc* pState ))



/* /////////////////////////////////////////////////////////////////////////////
//  Work with Delay Line
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:           ippsFIRGetDlyLine, ippsFIRSetDlyLine
//  Purpose:         set and get delay line
//  Parameters:
//      pDlyLine            - pointer to delay line
//      pState              - pointer to the filter state
//  Return:
//      ippStsContextMatchErr  - wrong state identifier
//      ippStsNullPtrErr       - pointer(s) to the data is NULL
//      ippStsNoErr            - otherwise
//  Note: pDlyLine may be NULL
*/

IPPAPI(IppStatus, ippsFIRGetDlyLine_32f, (const IppsFIRState_32f* pState, Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine_32f, (IppsFIRState_32f* pState, const Ipp32f* pDlyLine))

IPPAPI(IppStatus, ippsFIRGetDlyLine_32fc, (const IppsFIRState_32fc* pState, Ipp32fc* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine_32fc, (IppsFIRState_32fc* pState, const Ipp32fc* pDlyLine))

IPPAPI(IppStatus, ippsFIRGetDlyLine32f_16s, (const IppsFIRState32f_16s* pState, Ipp16s* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine32f_16s, (IppsFIRState32f_16s* pState, const Ipp16s* pDlyLine))

IPPAPI(IppStatus, ippsFIRGetDlyLine32fc_16sc, (const IppsFIRState32fc_16sc* pState, Ipp16sc* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine32fc_16sc, (IppsFIRState32fc_16sc* pState, const Ipp16sc* pDlyLine))

IPPAPI(IppStatus, ippsFIRGetDlyLine_64f, (const IppsFIRState_64f* pState, Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine_64f, (IppsFIRState_64f* pState, const Ipp64f* pDlyLine))

IPPAPI(IppStatus, ippsFIRGetDlyLine_64fc, (const IppsFIRState_64fc* pState, Ipp64fc* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine_64fc, (IppsFIRState_64fc* pState, const Ipp64fc* pDlyLine))

IPPAPI(IppStatus, ippsFIRGetDlyLine64f_32f, (const IppsFIRState64f_32f* pState, Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine64f_32f, (IppsFIRState64f_32f* pState, const Ipp32f* pDlyLine))

IPPAPI(IppStatus, ippsFIRGetDlyLine64fc_32fc, (const IppsFIRState64fc_32fc* pState, Ipp32fc* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine64fc_32fc, (IppsFIRState64fc_32fc* pState, const Ipp32fc* pDlyLine))

IPPAPI(IppStatus, ippsFIRGetDlyLine64f_32s, (const IppsFIRState64f_32s* pState, Ipp32s* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine64f_32s, (IppsFIRState64f_32s* pState, const Ipp32s* pDlyLine))

IPPAPI(IppStatus, ippsFIRGetDlyLine64fc_32sc, (const IppsFIRState64fc_32sc* pState, Ipp32sc* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine64fc_32sc, (IppsFIRState64fc_32sc* pState, const Ipp32sc* pDlyLine))

IPPAPI(IppStatus, ippsFIRGetDlyLine64f_16s, (const IppsFIRState64f_16s* pState, Ipp16s* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine64f_16s, (IppsFIRState64f_16s* pState, const Ipp16s* pDlyLine))

IPPAPI(IppStatus, ippsFIRGetDlyLine64fc_16sc, (const IppsFIRState64fc_16sc* pState, Ipp16sc* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine64fc_16sc, (IppsFIRState64fc_16sc* pState, const Ipp16sc* pDlyLine))

/* /////////////////////////////////////////////////////////////////////////////
//  Filtering
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:         ippsFIROne
//  Purpose:       FIR filter. One point filtering
//  Parameters:
//      src            - input sample
//      pDstVal        - output sample
//      pState         - pointer to the filter state
//      scaleFactor    - scale factor value
//  Return:
//      ippStsContextMatchErr  - wrong state identifier
//      ippStsNullPtrErr       - pointer(s) to the data is NULL
//      ippStsNoErr            - otherwise
*/

IPPAPI(IppStatus, ippsFIROne_32f, (Ipp32f src, Ipp32f* pDstVal, IppsFIRState_32f* pState))
IPPAPI(IppStatus, ippsFIROne_32fc, (Ipp32fc src, Ipp32fc* pDstVal, IppsFIRState_32fc* pState))

IPPAPI(IppStatus, ippsFIROne32f_16s_Sfs, (Ipp16s src, Ipp16s* pDstVal,
        IppsFIRState32f_16s* pState, int scaleFactor ))
IPPAPI(IppStatus, ippsFIROne32fc_16sc_Sfs, (Ipp16sc src, Ipp16sc* pDstVal,
        IppsFIRState32fc_16sc* pState,  int scaleFactor ))

IPPAPI(IppStatus, ippsFIROne_64f, (Ipp64f src, Ipp64f* pDstVal, IppsFIRState_64f* pState))
IPPAPI(IppStatus, ippsFIROne_64fc, (Ipp64fc src, Ipp64fc* pDstVal, IppsFIRState_64fc* pState))

IPPAPI(IppStatus, ippsFIROne64f_32f, (Ipp32f src, Ipp32f* pDstVal, IppsFIRState64f_32f* pState))
IPPAPI(IppStatus, ippsFIROne64fc_32fc, (Ipp32fc src, Ipp32fc* pDstVal, IppsFIRState64fc_32fc* pState))

IPPAPI(IppStatus, ippsFIROne64f_32s_Sfs, (Ipp32s src, Ipp32s* pDstVal,
        IppsFIRState64f_32s* pState, int scaleFactor ))
IPPAPI(IppStatus, ippsFIROne64fc_32sc_Sfs, (Ipp32sc src, Ipp32sc* pDstVal,
        IppsFIRState64fc_32sc* pState,  int scaleFactor ))

IPPAPI(IppStatus, ippsFIROne64f_16s_Sfs, (Ipp16s src, Ipp16s* pDstVal,
        IppsFIRState64f_16s* pState,  int scaleFactor ))
IPPAPI(IppStatus, ippsFIROne64fc_16sc_Sfs, (Ipp16sc src, Ipp16sc* pDstVal,
         IppsFIRState64fc_16sc* pState, int scaleFactor ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:         ippsFIR
//  Purpose:       FIR filter. Vector filtering
//  Parameters:
//      pSrcDst     - pointer to the input/output vector in in-place operation
//      pSrc        - pointer to the input vector
//      pDst        - pointer to the output vector
//      numIters    - number iterations (for single-rate equal length data vector)
//      pState      - pointer to the filter state
//      scaleFactor - scale factor value
//  Return:
//      ippStsContextMatchErr  - wrong state identifier
//      ippStsNullPtrErr       - pointer(s) to the data is NULL
//      ippStsSizeErr          - numIters is less or equal zero
//      ippStsNoErr            - otherwise
//  Note: for Multi-Rate filtering
//          length pSrc = numIters*downFactor
//          length pDst = numIters*upFactor
//          for inplace functions max this values
*/

IPPAPI(IppStatus, ippsFIR_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int numIters,
        IppsFIRState_32f* pState))
IPPAPI(IppStatus, ippsFIR_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int numIters,
        IppsFIRState_32fc* pState))

IPPAPI(IppStatus, ippsFIR32f_16s_Sfs, (const Ipp16s* pSrc, Ipp16s* pDst, int numIters,
        IppsFIRState32f_16s* pState, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR32fc_16sc_Sfs, (const Ipp16sc* pSrc, Ipp16sc* pDst, int numIters,
        IppsFIRState32fc_16sc* pState, int scaleFactor ))

IPPAPI(IppStatus, ippsFIR_32f_I, (Ipp32f* pSrcDst, int numIters,
        IppsFIRState_32f* pState))
IPPAPI(IppStatus, ippsFIR_32fc_I, (Ipp32fc* pSrcDst, int numIters,
        IppsFIRState_32fc* pState))

IPPAPI(IppStatus, ippsFIR32f_16s_ISfs, (Ipp16s* pSrcDst, int numIters,
        IppsFIRState32f_16s* pState, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR32fc_16sc_ISfs, (Ipp16sc* pSrcDst, int numIters,
        IppsFIRState32fc_16sc* pState, int scaleFactor ))

IPPAPI(IppStatus, ippsFIR_64f, (const Ipp64f* pSrc, Ipp64f* pDst, int numIters,
        IppsFIRState_64f* pState))
IPPAPI(IppStatus, ippsFIR_64fc, (const Ipp64fc* pSrc, Ipp64fc* pDst, int numIters,
        IppsFIRState_64fc* pState))

IPPAPI(IppStatus, ippsFIR_64f_I, (Ipp64f* pSrcDst, int numIters,
        IppsFIRState_64f* pState))
IPPAPI(IppStatus, ippsFIR_64fc_I, (Ipp64fc* pSrcDst, int numIters,
        IppsFIRState_64fc* pState))

IPPAPI(IppStatus, ippsFIR64f_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int numIters,
        IppsFIRState64f_32f* pState))
IPPAPI(IppStatus, ippsFIR64fc_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int numIters,
        IppsFIRState64fc_32fc* pState))

IPPAPI(IppStatus, ippsFIR64f_32f_I, (Ipp32f* pSrcDst, int numIters,
        IppsFIRState64f_32f* pState))
IPPAPI(IppStatus, ippsFIR64fc_32fc_I, (Ipp32fc* pSrcDst, int numIters,
        IppsFIRState64fc_32fc* pState))

IPPAPI(IppStatus, ippsFIR64f_32s_Sfs, (const Ipp32s* pSrc, Ipp32s* pDst, int numIters,
        IppsFIRState64f_32s* pState, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR64fc_32sc_Sfs, (const Ipp32sc* pSrc, Ipp32sc* pDst, int numIters,
        IppsFIRState64fc_32sc* pState, int scaleFactor ))

IPPAPI(IppStatus, ippsFIR64f_32s_ISfs, (Ipp32s* pSrcDst, int numIters,
        IppsFIRState64f_32s* pState,  int scaleFactor ))
IPPAPI(IppStatus, ippsFIR64fc_32sc_ISfs, (Ipp32sc* pSrcDst, int numIters,
        IppsFIRState64fc_32sc* pState,  int scaleFactor ))

IPPAPI(IppStatus, ippsFIR64f_16s_Sfs, (const Ipp16s* pSrc, Ipp16s* pDst, int numIters,
        IppsFIRState64f_16s* pState,  int scaleFactor ))
IPPAPI(IppStatus, ippsFIR64fc_16sc_Sfs, (const Ipp16sc* pSrc, Ipp16sc* pDst, int numIters,
        IppsFIRState64fc_16sc* pState, int scaleFactor ))

IPPAPI(IppStatus, ippsFIR64f_16s_ISfs, (Ipp16s* pSrcDst, int numIters,
        IppsFIRState64f_16s* pState, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR64fc_16sc_ISfs, (Ipp16sc* pSrcDst, int numIters,
        IppsFIRState64fc_16sc* pState, int scaleFactor ))

/* /////////////////////////////////////////////////////////////////////////////
//                     FIR filters (integer taps version)
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//  Initialize State
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:       ippsFIRInitAlloc, ippsFIRMRInitAlloc, ippsFIRFree
//  Purpose:     create and initialize FIR state, set taps and delay line
//  Parameters:
//      pTaps          - pointer to the filter coefficients
//      tapsLen        - number of coefficients
//      tapsFactor     - scale factor of Ipp32s taps
//      pDlyLine       - pointer delay line, may be NULL
//      state          - pointer to the state created or NULL
//  Return:
//      status         - status returned, its values are
//          ippStsMemAllocErr  - memory allocation error
//          ippStsNullPtrErr   - pointer(s) to the data is NULL
//          ippStsFIRLenErr    - tapsLen <= 0
//          ippStsFIRMRFactorErr   - factor <= 0
//          ippStsFIRMRPhaseErr    - phase < 0 || factor <= phase
//          ippStsNoErr        - otherwise
//  Notes:   pTaps and tapsFactor for Ipp32s calculate as follows
//
//          Ipp64f mpy = 1.0;
//          Ipp32f pFTaps[tapsLen];     // true values of the coefficients
//          Ipp32s pTaps[tapsLen];      // values to be pass to integer FIR
//
//          ... calculate coefficients, filling pFTaps ...
//
//          max = MAX(abs(pFTaps[i]));   for i = 0..tapsLen-1
//
//          tapsFactor = 0;
//          if (max > IPP_MAX_32S) {
//              while (max > IPP_MAX_32S) {
//                  tapsFactor++;
//                  max *= 0.5;
//                  mpy *= 0.5;
//              }
//          } else {
//              while (max < IPP_MAX_32S && tapsFactor > -17) {
//                  tapsFactor--;
//                  max += max;
//                  mpy += mpy;
//              }
//              tapsFactor++;
//              mpy *= 0.5;
//          }
//
//          for (i = 0; i < tapsLen; i++)
//              if (pFTaps[i] < 0)
//                  pSTaps[i] = (Ipp32s)(mpy*pFTaps[i]-0.5);
//              else
//                  pSTaps[i] = (Ipp32s)(mpy*pFTaps[i]+0.5);
*/
IPPAPI(IppStatus, ippsFIRInitAlloc32s_16s, (IppsFIRState32s_16s** pState,
        const Ipp32s *pTaps, int tapsLen, int tapsFactor, const Ipp16s *pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc32s_16s, (IppsFIRState32s_16s** pState,
        const Ipp32s *pTaps, int tapsLen, int tapsFactor, int upFactor,
        int upPhase, int downFactor, int downPhase, const Ipp16s *pDlyLine))
IPPAPI(IppStatus, ippsFIRInitAlloc32s_16s32f, (IppsFIRState32s_16s** pState,
        const Ipp32f *pTaps, int tapsLen, const Ipp16s *pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc32s_16s32f, (IppsFIRState32s_16s **pState,
        const Ipp32f *pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp16s *pDlyLine))
IPPAPI(IppStatus, ippsFIRInitAlloc32sc_16sc, (IppsFIRState32sc_16sc** pState,
        const Ipp32sc *pTaps, int tapsLen, int tapsFactor, const Ipp16sc *pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc32sc_16sc, (IppsFIRState32sc_16sc** pState,
        const Ipp32sc *pTaps, int tapsLen, int tapsFactor, int upFactor,
        int upPhase, int downFactor, int downPhase, const Ipp16sc *pDlyLine))
IPPAPI(IppStatus, ippsFIRInitAlloc32sc_16sc32fc, (IppsFIRState32sc_16sc** pState,
        const Ipp32fc *pTaps, int tapsLen, const Ipp16sc *pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc32sc_16sc32fc, (IppsFIRState32sc_16sc** pState,
        const Ipp32fc *pTaps, int tapsLen, int upFactor, int upPhase,
        int downFactor, int downPhase, const Ipp16sc *pDlyLine))

IPPAPI(IppStatus, ippsFIRFree32s_16s, (IppsFIRState32s_16s *pState))

IPPAPI(IppStatus, ippsFIRFree32sc_16sc, (IppsFIRState32sc_16sc *pState))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:              ippsFIRGetTaps
//  Purpose:            get FIR taps value from state
//  Parameters:
//      pTaps       -   pointer to buffer to get FIR taps
//      pState      -   pointer to the FIR filter state
//  Return:
//      ippStsContextMatchErr  -   wrong state identifier
//      ippStsNullPtrErr       -   pointer(s) to the data is NULL
//      ippStsNoErr            -   otherwise
*/

IPPAPI(IppStatus, ippsFIRGetTaps32s_16s, (const IppsFIRState32s_16s* pState,
        Ipp32s* pTaps, int* tapsFactor))
IPPAPI(IppStatus, ippsFIRGetTaps32sc_16sc, (const IppsFIRState32sc_16sc* pState,
        Ipp32sc* pTaps, int* tapsFactor))
IPPAPI(IppStatus, ippsFIRGetTaps32s_16s32f, (const IppsFIRState32s_16s* pState,
        Ipp32f* pTaps))
IPPAPI(IppStatus, ippsFIRGetTaps32sc_16sc32fc, (const IppsFIRState32sc_16sc* pState,
        Ipp32fc* pTaps))


/* /////////////////////////////////////////////////////////////////////////////
//  Work with Delay Line
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:             ippsFIRGetDlyLine, ippsFIRSetDlyLine
//  Purpose:           set and get delay line
//  Parameters:
//      pDlyLine       - pointer to the delay line
//      pState         - pointer to the FIR filter state
//  Return:
//      ippStsContextMatchErr  -   wrong state identifier
//      ippStsNullPtrErr       -   pointer(s) to the data is NULL
//      ippStsNoErr            -   otherwise
//  Note: pDlyLine may be NULL
*/

IPPAPI(IppStatus, ippsFIRGetDlyLine32s_16s, (const IppsFIRState32s_16s* pState,
        Ipp16s* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine32s_16s, (IppsFIRState32s_16s* pState,
        const Ipp16s* pDlyLine))
IPPAPI(IppStatus, ippsFIRGetDlyLine32sc_16sc, (const IppsFIRState32sc_16sc* pState,
        Ipp16sc* pDlyLine))
IPPAPI(IppStatus, ippsFIRSetDlyLine32sc_16sc, (IppsFIRState32sc_16sc* pState,
        const Ipp16sc* pDlyLine))

/* /////////////////////////////////////////////////////////////////////////////
//  Filtering
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:            ippsFIROne, ippsFIROne
//  Purpose:          FIR filter with integer taps. One sample filtering
//  Parameters:
//      src            - input sample
//      pDstVal        - pointer to the output sample
//      pState         - pointer to the FIR filter state
//      scaleFactor    - scale factor value
//  Return:
//      ippStsContextMatchErr  - wrong state identifier
//      ippStsNullPtrErr       - pointer(s) to the data is NULL
//      ippStsNoErr            - otherwise
*/
IPPAPI(IppStatus, ippsFIROne32s_16s_Sfs, (Ipp16s src, Ipp16s *pDstVal,
        IppsFIRState32s_16s *pState, int scaleFactor ))
IPPAPI(IppStatus, ippsFIROne32sc_16sc_Sfs, (Ipp16sc src, Ipp16sc *pDstVal,
        IppsFIRState32sc_16sc *pState, int scaleFactor ))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:        ippsFIR
//  Purpose:      FIR filter with integer taps. Vector filtering
//  Parameters:
//      pSrc          - pointer to the input vector
//      pDst          - pointer to the output vector
//      pSrcDst       - pointer to input/output vector in in-place operation
//      numIters      - number iterations (for single-rate equal length data vector)
//      pState        - pointer to the filter state
//      scaleFactor   - scale factor value
//  Return:
//      ippStsContextMatchErr  - wrong State identifier
//      ippStsNullPtrErr       - pointer(s) to the data is NULL
//      ippStsSizeErr          - numIters <= 0
//      ippStsNoErr            - otherwise
//  Note: for Multi-Rate filtering
//          length pSrc = numIters*downFactor
//          length pDst = numIters*upFactor
//          for inplace functions max this values
*/

IPPAPI(IppStatus, ippsFIR32s_16s_Sfs, (const Ipp16s *pSrc, Ipp16s *pDst,
        int numIters, IppsFIRState32s_16s *pState, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR32sc_16sc_Sfs, (const Ipp16sc *pSrc, Ipp16sc *pDst,
        int numIters, IppsFIRState32sc_16sc *pState, int scaleFactor ))

IPPAPI(IppStatus, ippsFIR32s_16s_ISfs, (Ipp16s *pSrcDst, int numIters,
        IppsFIRState32s_16s *pState, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR32sc_16sc_ISfs, (Ipp16sc *pSrcDst, int numIters,
        IppsFIRState32sc_16sc *pState, int scaleFactor ))

IPPAPI( IppStatus, ippsFIRInitAlloc_32s,( IppsFIRState_32s** pState,
                   const Ipp32s *pTaps, int tapsLen, const Ipp32s *pDlyLine ))
IPPAPI( IppStatus, ippsFIRGetStateSize_32s,( int tapsLen, int* pBufferSize ))
IPPAPI( IppStatus, ippsFIRInit_32s,( IppsFIRState_32s** pState,
      const Ipp32s *pTaps, int tapsLen, const Ipp32s *pDlyLine, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsFIRSetTaps_32s,( const Ipp32s *pTaps,
                                                   IppsFIRState_32s* pState ))
IPPAPI( IppStatus, ippsFIRGetTaps_32s,( const IppsFIRState_32s* pState,
                                                              Ipp32s* pTaps ))
IPPAPI( IppStatus, ippsFIROne_32s_Sfs,( Ipp32s src, Ipp32s *pDstVal,
                                  IppsFIRState_32s *pState, int scaleFactor ))
IPPAPI( IppStatus, ippsFIR_32s_Sfs,( const Ipp32s *pSrc, Ipp32s *pDst,
                    int numIters, IppsFIRState_32s *pState, int scaleFactor ))
IPPAPI( IppStatus, ippsFIR_32s_ISfs,( Ipp32s *pSrcDst, int numIters,
                                  IppsFIRState_32s *pState, int scaleFactor ))
IPPAPI( IppStatus, ippsFIRFree_32s,( IppsFIRState_32s *pState ))

IPPAPI( IppStatus, ippsFIRGetStateSize_16s,( int tapsLen, int* pStateSize ))
IPPAPI( IppStatus, ippsFIRMRGetStateSize_16s,( int tapsLen, int upFactor,
                                            int downFactor, int *pStateSize ))
IPPAPI( IppStatus, ippsFIRInit_16s,( IppsFIRState_16s** ppState,
      const Ipp16s *pTaps, int tapsLen, int tapsFactor, const Ipp16s *pDlyLine,
                                                            Ipp8u* pBuffer ))
IPPAPI( IppStatus, ippsFIRMRInit_16s,( IppsFIRState_16s** ppState,
   const Ipp16s *pTaps, int tapsLen, int tapsFactor, int upFactor, int upPhase,
         int downFactor, int downPhase, const Ipp16s *pDlyLine, Ipp8u* pBuffer ))
IPPAPI(IppStatus, ippsFIRInitAlloc_16s, (IppsFIRState_16s** pState,
        const Ipp16s *pTaps, int tapsLen, int tapsFactor, const Ipp16s *pDlyLine))
IPPAPI(IppStatus, ippsFIRMRInitAlloc_16s, (IppsFIRState_16s** pState,
        const Ipp16s *pTaps, int tapsLen, int tapsFactor, int upFactor,
        int upPhase, int downFactor, int downPhase, const Ipp16s *pDlyLine))
IPPAPI( IppStatus, ippsFIRSetTaps_16s,( const Ipp16s *pTaps,
                                 IppsFIRState_16s* pState, int tapsFactor ))
IPPAPI( IppStatus, ippsFIRGetTaps_16s,( const IppsFIRState_16s* pState,
                                              Ipp16s* pTaps, int* tapsFactor ))
IPPAPI( IppStatus, ippsFIRSetDlyLine_16s,( IppsFIRState_16s *pState,
                                                      const Ipp16s *pDlyLine ))
IPPAPI( IppStatus, ippsFIRGetDlyLine_16s,(
                         const IppsFIRState_16s *pState, Ipp16s *pDlyLine ))
IPPAPI( IppStatus, ippsFIROne_16s_Sfs,( Ipp16s src, Ipp16s *pDstVal,
                               IppsFIRState_16s *pState,  int scaleFactor ))
IPPAPI(IppStatus, ippsFIR_16s_Sfs, (const Ipp16s *pSrc, Ipp16s *pDst,
        int numIters, IppsFIRState_16s *pState, int scaleFactor ))
IPPAPI( IppStatus, ippsFIR_16s_ISfs,( Ipp16s *pSrcDst, int numIters,
                                IppsFIRState_16s *pState, int scaleFactor ))
IPPAPI(IppStatus, ippsFIRFree_16s, (IppsFIRState_16s *pState))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:        ippsFIRStream
//  Purpose:      FIR stream filter with integer taps. Vector filtering
//                It does not have delay line.
//  Parameters:
//      pSrc          - pointer to the input vector
//      pDst          - pointer to the output vector
//      numIters      - number iterations (for single-rate equal length data vector)
//      pState        - pointer to the filter state
//      scaleFactor   - scale factor value
//  Return:
//      ippStsContextMatchErr  - wrong State identifier
//      ippStsNullPtrErr       - pointer(s) to the data is NULL
//      ippStsSizeErr          - numIters <= 0
//      ippStsNoErr            - otherwise
//  Note: for Multi-Rate filtering
//          length pSrc = numIters*downFactor
//          length pDst = numIters*upFactor
//          for inplace functions max this values
*/

IPPAPI( IppStatus, ippsFIRStreamGetStateSize_16s,( int tapsLen, int* pStateSize ))
IPPAPI( IppStatus, ippsFIRMRStreamGetStateSize_16s,( int tapsLen, int upFactor,
                                            int downFactor, int *pStateSize ))
IPPAPI( IppStatus, ippsFIRStreamInit_16s,( IppsFIRState_16s** pState,
      const Ipp16s *pTaps, int tapsLen, int tapsFactor,  IppRoundMode rndMode, Ipp8u* pBuf ))
IPPAPI( IppStatus, ippsFIRMRStreamInit_16s,( IppsFIRState_16s** ppState,
   const Ipp16s *pTaps, int tapsLen, int tapsFactor, int upFactor, int upPhase,
         int downFactor, int downPhase, IppRoundMode rndMode, Ipp8u* pBuffer ))
IPPAPI(IppStatus, ippsFIRStreamInitAlloc_16s, (IppsFIRState_16s** pState,
        const Ipp16s *pTaps, int tapsLen, int tapsFactor, IppRoundMode rndMode))
IPPAPI(IppStatus, ippsFIRMRStreamInitAlloc_16s, (IppsFIRState_16s** pState,
        const Ipp16s *pTaps, int tapsLen, int tapsFactor, int upFactor,
        int upPhase, int downFactor, int downPhase, IppRoundMode rndMode))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:        ippsFIRStream
//  Purpose:      FIR stream filter with float taps. Vector filtering
//                It does not have delay line.
//  Parameters:
//      pSrc          - pointer to the input vector
//      pDst          - pointer to the output vector
//      numIters      - number iterations (for single-rate equal length data vector)
//      pState        - pointer to the filter state
//      scaleFactor   - scale factor value//  Return:
//      ippStsContextMatchErr  - wrong State identifier
//      ippStsNullPtrErr       - pointer(s) to the data is NULL
//      ippStsSizeErr          - numIters <= 0
//      ippStsNoErr            - otherwise
//  Note: for Multi-Rate filtering
//          length pSrc = numIters*downFactor
//          length pDst = numIters*upFactor
//          for inplace functions max this values
*/
IPPAPI( IppStatus, ippsFIRStreamGetStateSize_32f,( int tapsLen, int *pBufferSize ))
IPPAPI( IppStatus, ippsFIRMRStreamGetStateSize_32f,( int tapsLen, int upFactor,
                                             int downFactor, int *pBufferSize ))
IPPAPI( IppStatus, ippsFIRStreamInit_32f,( IppsFIRState_32f** pState,
    const Ipp32f *pTaps, int tapsLen, Ipp8u *pBuffer ))
IPPAPI( IppStatus, ippsFIRMRStreamInit_32f,( IppsFIRState_32f** pState,
    const Ipp32f *pTaps, int tapsLen, int upFactor, int upPhase,
                  int downFactor, int downPhase, Ipp8u *pBuffer ))
IPPAPI( IppStatus, ippsFIRStreamInitAlloc_32f,( IppsFIRState_32f** pState,
                    const Ipp32f *pTaps, int tapsLen ))
IPPAPI( IppStatus, ippsFIRMRStreamInitAlloc_32f,( IppsFIRState_32f** pState,
                   const Ipp32f* pTaps, int tapsLen, int upFactor, int upPhase,
                       int downFactor, int downPhase ))

/* /////////////////////////////////////////////////////////////////////////////
//                  FIR LMS filters
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//   Names:     ippsFIRLMSOne_Direct
//   Purpose:   direct form of a FIR LMS filter. One point operation.
//   Parameters:
//      src          source signal sample
//      refval       desired signal sample
//      pTapsInv     FIR taps coefficient values to be fitted
//      tapsLen      number of the taps
//      pDlyLine     pointer to the delay line values
//      pDlyIndex    pointer to the current index of delay line
//      mu           adaptation step
//      muQ15        adaptation step, integer version
//                   muQ15 = (int)(mu * (1<<15) + 0.5f)
//      pDstVal      where write output sample to
//   Return:
//      ippStsNullPtrErr  pointer the the data is null
//      ippStsSizeErr     the taps length is equal or less zero
//      ippStsNoErr       otherwise
//   Note: adaptation error value has been deleted from the parameter
//         list because it can be computed as (refval - dst).
//         taps array is inverted, delay line is of double size = tapsLen * 2
*/
IPPAPI(IppStatus, ippsFIRLMSOne_Direct_32f,( Ipp32f src, Ipp32f refval,
       Ipp32f* pDstVal, Ipp32f* pTapsInv, int tapsLen, float mu, Ipp32f* pDlyLine,
       int* pDlyIndex ))

IPPAPI(IppStatus, ippsFIRLMSOne_Direct32f_16s,( Ipp16s src, Ipp16s refval,
       Ipp16s* pDstVal, Ipp32f* pTapsInv, int tapsLen, float mu, Ipp16s* pDlyLine,
       int* pDlyIndex ))

IPPAPI(IppStatus, ippsFIRLMSOne_DirectQ15_16s,( Ipp16s src, Ipp16s refval,
       Ipp16s* pDstVal, Ipp32s* pTapsInv, int tapsLen, int muQ15, Ipp16s* pDlyLine,
       int* pDlyIndex ))


/* context oriented functions */
#if !defined( _OWN_BLDPCS )

  struct FIRLMSState_32f;
  typedef struct FIRLMSState_32f IppsFIRLMSState_32f;

  struct FIRLMSState32f_16s;
  typedef struct FIRLMSState32f_16s IppsFIRLMSState32f_16s;

#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
//   Names:      ippsFIRLMS
//   Purpose:    LMS filtering with context use
//   Parameters:
//      pState    pointer to the state
//      pSrc      pointer to the source signal
//      pRef      pointer to the desired signal
//      pDst      pointer to the output signal
//      len       length of the signals
//      mu        adaptation step
//   Return:
//      ippStsNullPtrErr       pointer to the data is null
//      ippStsSizeErr          the length of signals is equal or less zero
//      ippStsContextMatchErr    wrong state identifier
//      ippStsNoErr            otherwise
*/
IPPAPI(IppStatus, ippsFIRLMS_32f,( const Ipp32f* pSrc, const Ipp32f* pRef,
       Ipp32f* pDst, int len, float mu, IppsFIRLMSState_32f* pState ))

IPPAPI(IppStatus, ippsFIRLMS32f_16s,( const Ipp16s* pSrc, const Ipp16s* pRef,
       Ipp16s* pDst, int len, float mu, IppsFIRLMSState32f_16s* pStatel ))


/* /////////////////////////////////////////////////////////////////////////////
//   Names:       ippsFIRLMSInitAlloc, ippsFIRLMSFree
//   Purpose:     LMS initialization functions
//   Parameters:
//      pTaps     pointer to the taps values. May be null
//      tapsLen   number of the taps
//      pDlyLine  pointer to the delay line. May be null
//      dlyLineIndex  current index value for the delay line
//      pState    address of pointer to the state returned
//   Return:
//      ippStsNullPtrErr       pointer is null
//      ippStsContextMatchErr    wrong state identifier
//      ippStsNoErr            otherwise
*/

IPPAPI(IppStatus, ippsFIRLMSInitAlloc_32f,( IppsFIRLMSState_32f** pState,
   const Ipp32f* pTaps, int tapsLen, const Ipp32f* pDlyLine, int dlyLineIndex ))

IPPAPI(IppStatus, ippsFIRLMSInitAlloc32f_16s,( IppsFIRLMSState32f_16s** pState,
   const Ipp32f* pTaps, int tapsLen, const Ipp16s* pDlyLine, int dlyLineIndex ))

IPPAPI(IppStatus, ippsFIRLMSFree_32f,( IppsFIRLMSState_32f* pState))
IPPAPI(IppStatus, ippsFIRLMSFree32f_16s,( IppsFIRLMSState32f_16s* pState))

/* /////////////////////////////////////////////////////////////////////////////
//   Names:        ippsFIRLMSGetTaps
//   Purpose:      get taps values
//   Parameters:
//      pstate          pointer to the state
//      pTaps           pointer to the array to store the taps values
//   Return:
//      ippStsNullPtrErr   pointer to the data is null
//      ippStsNoErr        otherwise
*/

IPPAPI(IppStatus, ippsFIRLMSGetTaps_32f,( const IppsFIRLMSState_32f* pState,
       Ipp32f* pOutTaps ))
IPPAPI(IppStatus, ippsFIRLMSGetTaps32f_16s,( const IppsFIRLMSState32f_16s* pState,
       Ipp32f* pOutTaps ))

/* /////////////////////////////////////////////////////////////////////////////
//   Names:       ippsFIRLMSGetDlyl, ippsFIRLMSSetDlyl
//   Purpose:     set or get delay line
//   Parameters:
//      pState         pointer to the state structure
//      pDlyLine       pointer to the delay line of the single size = tapsLen
//      pDlyLineIndex  pointer to get the current delay line index
//   Return:
//      ippStsNullPtrErr       pointer to the data is null
//      ippStsContextMatchErr    wrong state identifier
//      ippStsNoErr            otherwise
*/

IPPAPI(IppStatus, ippsFIRLMSGetDlyLine_32f,( const IppsFIRLMSState_32f* pState,
   Ipp32f* pDlyLine, int* pDlyLineIndex ))
IPPAPI(IppStatus, ippsFIRLMSGetDlyLine32f_16s,( const IppsFIRLMSState32f_16s* pState,
   Ipp16s* pDlyLine, int* pDlyLineIndex ))

IPPAPI(IppStatus, ippsFIRLMSSetDlyLine_32f,( IppsFIRLMSState_32f* pState,
   const Ipp32f* pDlyLine, int dlyLineIndex ))
IPPAPI(IppStatus, ippsFIRLMSSetDlyLine32f_16s,( IppsFIRLMSState32f_16s* pState,
   const Ipp16s* pDlyLine, int dlyLineIndex ))


/* /////////////////////////////////////////////////////////////////////////////
//                  FIR LMS MR filters
///////////////////////////////////////////////////////////////////////////// */

/* context oriented functions */
#if !defined( _OWN_BLDPCS )

  struct FIRLMSMRState32s_16s;
  typedef struct FIRLMSMRState32s_16s IppsFIRLMSMRState32s_16s;

  struct FIRLMSMRState32sc_16sc;
  typedef struct FIRLMSMRState32sc_16sc IppsFIRLMSMRState32sc_16sc;

#endif /* _OWN_BLDPCS */

/* /////////////////////////////////////////////////////////////////////////////
//   Names:      ippsFIRLMSMROne, ippsFIRLMSMROneVal
//   Purpose:    LMS MR filtering with context use
//   Parameters:
//      val       the source signal last value to update delay line
//      pDstVal   pointer to the output signal value
//      pState    pointer to the state
//   Return:
//      ippStsNullPtrErr        pointer to the data is null
//      ippStsContextMatchErr   wrong state identifier
//      ippStsNoErr             otherwise
*/
IPPAPI( IppStatus, ippsFIRLMSMROne32s_16s,( Ipp32s* pDstVal,
                                            IppsFIRLMSMRState32s_16s* pState ))
IPPAPI( IppStatus, ippsFIRLMSMROneVal32s_16s,( Ipp16s val, Ipp32s* pDstVal,
                                            IppsFIRLMSMRState32s_16s* pState ))

IPPAPI( IppStatus, ippsFIRLMSMROne32sc_16sc,( Ipp32sc* pDstVal,
                                            IppsFIRLMSMRState32sc_16sc* pState ))
IPPAPI( IppStatus, ippsFIRLMSMROneVal32sc_16sc,( Ipp16sc val, Ipp32sc* pDstVal,
                                            IppsFIRLMSMRState32sc_16sc* pState ))

/* /////////////////////////////////////////////////////////////////////////////
//   Names:       ippsFIRLMSMRInitAlloc, ippsFIRLMSMRFree
//   Purpose:     LMS MR initialization functions
//   Parameters:
//      pState        address of pointer to the state returned
//      pTaps         pointer to the taps values. May be null
//      tapsLen       number of the taps
//      pDlyLine      pointer to the delay line. May be null
//      dlyLineIndex  current index value for the delay line
//      dlyStep       sample down factor
//      updateDly     update delay in samples
//      mu            adaptation step
//   Return:
//      ippStsNullPtrErr       pointer is null
//      ippStsContextMatchErr  wrong state identifier
//      ippStsNoErr            otherwise
*/

IPPAPI( IppStatus, ippsFIRLMSMRInitAlloc32s_16s,( IppsFIRLMSMRState32s_16s** pState,
   const Ipp32s* pTaps, int tapsLen, const Ipp16s* pDlyLine, int dlyLineIndex,
   int dlyStep, int updateDly, int mu ))
IPPAPI( IppStatus, ippsFIRLMSMRFree32s_16s,( IppsFIRLMSMRState32s_16s* pState ))

IPPAPI( IppStatus, ippsFIRLMSMRInitAlloc32sc_16sc,( IppsFIRLMSMRState32sc_16sc** pState,
   const Ipp32sc* pTaps, int tapsLen, const Ipp16sc* pDlyLine, int dlyLineIndex,
   int dlyStep, int updateDly, int mu ))
IPPAPI( IppStatus, ippsFIRLMSMRFree32sc_16sc,( IppsFIRLMSMRState32sc_16sc* pState ))

/* /////////////////////////////////////////////////////////////////////////////
//   Names:        ippsFIRLMSMRGetTaps, ippsFIRLMSMRSetTaps,
//                 ippsFIRLMSMRGetTapsPointer
//   Purpose:      get & set taps values
//   Parameters:
//      pState     pointer to the state
//      pOutTaps   pointer to the array to store the taps values
//      pInTaps    pointer to the taps values. May be null
//      pTaps      pointer to the state taps values. For direct access
//   Return:
//      ippStsNullPtrErr       pointer to the data is null
//      ippStsContextMatchErr  wrong state identifier
//      ippStsNoErr            otherwise
*/

IPPAPI( IppStatus, ippsFIRLMSMRSetTaps32s_16s,( IppsFIRLMSMRState32s_16s* pState,
                                        const Ipp32s* pInTaps ))
IPPAPI( IppStatus, ippsFIRLMSMRGetTaps32s_16s,( IppsFIRLMSMRState32s_16s* pState,
                                           Ipp32s* pOutTaps ))
IPPAPI( IppStatus, ippsFIRLMSMRGetTapsPointer32s_16s,( IppsFIRLMSMRState32s_16s* pState,
                                           Ipp32s** pTaps ))

IPPAPI( IppStatus, ippsFIRLMSMRSetTaps32sc_16sc,( IppsFIRLMSMRState32sc_16sc* pState,
                                        const Ipp32sc* pInTaps ))
IPPAPI( IppStatus, ippsFIRLMSMRGetTaps32sc_16sc,( IppsFIRLMSMRState32sc_16sc* pState,
                                           Ipp32sc* pOutTaps ))
IPPAPI( IppStatus, ippsFIRLMSMRGetTapsPointer32sc_16sc,(
                        IppsFIRLMSMRState32sc_16sc* pState, Ipp32sc** pTaps ))

/* /////////////////////////////////////////////////////////////////////////////
//   Names:       ippsFIRLMSMRGetDlyLine, ippsFIRLMSMRSetDlyLine,
//                ippsFIRLMSMRGetDlyVal
//   Purpose:     set or get delay line, or get one delay line value from
//                specified position
//   Parameters:
//      pState          pointer to the state structure
//      pInDlyLine      pointer to the delay line of the (see state definition)
//                          size = tapsLen * dlyStep + updateDly (may be null)
//      pOutDlyLine     pointer to the delay line of the (see state definition)
//                      size = tapsLen * dlyStep + updateDly
//      pOutDlyLineIndex  pointer to get the current delay line index
//      dlyLineIndex    current index value for the delay line
//      index           to get one value posted into delay line "index" iterations ago
//   Return:
//      ippStsNullPtrErr       pointer to the data is null
//      ippStsContextMatchErr  wrong state identifier
//      ippStsNoErr            otherwise
*/

IPPAPI( IppStatus, ippsFIRLMSMRSetDlyLine32s_16s,( IppsFIRLMSMRState32s_16s* pState,
                                        const Ipp16s* pInDlyLine, int dlyLineIndex ))
IPPAPI( IppStatus, ippsFIRLMSMRGetDlyLine32s_16s,( IppsFIRLMSMRState32s_16s* pState,
                                        Ipp16s* pOutDlyLine, int* pOutDlyIndex ))
IPPAPI( IppStatus, ippsFIRLMSMRGetDlyVal32s_16s,( IppsFIRLMSMRState32s_16s* pState,
                                        Ipp16s* pOutVal, int index ))
IPPAPI( IppStatus, ippsFIRLMSMRSetDlyLine32sc_16sc,( IppsFIRLMSMRState32sc_16sc* pState,
                                        const Ipp16sc* pInDlyLine, int dlyLineIndex ))
IPPAPI( IppStatus, ippsFIRLMSMRGetDlyLine32sc_16sc,( IppsFIRLMSMRState32sc_16sc* pState,
                                        Ipp16sc* pOutDlyLine, int* pOutDlyLineIndex ))
IPPAPI( IppStatus, ippsFIRLMSMRGetDlyVal32sc_16sc,( IppsFIRLMSMRState32sc_16sc* pState,
                                        Ipp16sc* pOutVal, int index ))

/* /////////////////////////////////////////////////////////////////////////////
//   Names:       ippsFIRLMSMRPutVal
//   Purpose:     put one value to the delay line
//   Parameters:
//      val       the source signal last value to update delay line
//      pState    pointer to the state structure
//   Return:
//      ippStsNullPtrErr       pointer to the data is null
//      ippStsContextMatchErr  wrong state identifier
//      ippStsNoErr            otherwise
*/

IPPAPI( IppStatus, ippsFIRLMSMRPutVal32s_16s,( Ipp16s val,
                                            IppsFIRLMSMRState32s_16s* pState ))
IPPAPI( IppStatus, ippsFIRLMSMRPutVal32sc_16sc,( Ipp16sc val,
                                            IppsFIRLMSMRState32sc_16sc* pState ))

/* /////////////////////////////////////////////////////////////////////////////
//   Names:       ippsFIRLMSMRSetMu
//   Purpose:     set new adaptation step
//   Parameters:
//      pState    pointer to the state structure
//      mu        new adaptation step
//   Return:
//      ippStsNullPtrErr       pointer to the data is null
//      ippStsContextMatchErr  wrong state identifier
//      ippStsNoErr            otherwise
*/

IPPAPI( IppStatus, ippsFIRLMSMRSetMu32s_16s,( IppsFIRLMSMRState32s_16s* pState,
                                           const int mu ))
IPPAPI( IppStatus, ippsFIRLMSMRSetMu32sc_16sc,( IppsFIRLMSMRState32sc_16sc* pState,
                                           const int mu ))

/* /////////////////////////////////////////////////////////////////////////////
//   Names:       ippsFIRLMSMRUpdateTaps
//   Purpose:     recalculation of taps using Least Mean Square alg
//   Parameters:
//      ErrVal    difference between output and reference signal
//      pState    pointer to the state structure
//   Return:
//      ippStsNullPtrErr       pointer to the data is null
//      ippStsContextMatchErr  wrong state identifier
//      ippStsNoErr            otherwise
*/

IPPAPI( IppStatus, ippsFIRLMSMRUpdateTaps32s_16s,( Ipp32s ErrVal,
                                            IppsFIRLMSMRState32s_16s* pState ))
IPPAPI( IppStatus, ippsFIRLMSMRUpdateTaps32sc_16sc,( Ipp32sc ErrVal,
                                            IppsFIRLMSMRState32sc_16sc* pState ))




/* /////////////////////////////////////////////////////////////////////////////
//                     FIR filters (direct version)
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//   Names:      ippsFIROne_Direct
//   Purpose:    Directly filters a single sample through a FIR filter.
//   Parameters:
//      src            input sample
//      pDstVal        pointer to the output sample
//      pSrcDstVal     pointer to the input and output sample for in-place operation.
//      pTaps          pointer to the array containing the taps values,
//                       the number of elements in the array is tapsLen
//      tapsLen        number of elements in the array containing the taps values.
//      tapsFactor     scale factor for the taps of Ipp32s data type
//                               (for integer versions only).
//      pDlyLine       pointer to the array containing the delay line values,
//                        the number of elements in the array is 2*tapsLen
//      pDlyLineIndex  pointer to the current delay line index
//      scaleFactor    integer scaling factor value
//   Return:
//      ippStsNullPtrErr       pointer(s) to data arrays is(are) NULL
//      ippStsFIRLenErr        tapsLen is less than or equal to 0
//      ippStsNoErr            otherwise
*/

IPPAPI(IppStatus, ippsFIROne_Direct_32f, (Ipp32f src, Ipp32f* pDstVal, const Ipp32f* pTaps, int tapsLen,
        Ipp32f* pDlyLine, int* pDlyLineIndex))
IPPAPI(IppStatus, ippsFIROne_Direct_32fc, (Ipp32fc src, Ipp32fc* pDstVal, const Ipp32fc* pTaps, int tapsLen,
        Ipp32fc* pDlyLine, int* pDlyLineIndex))

IPPAPI(IppStatus, ippsFIROne_Direct_32f_I, (Ipp32f* pSrcDstVal, const Ipp32f* pTaps, int tapsLen,
        Ipp32f* pDlyLine, int* pDlyLineIndex))
IPPAPI(IppStatus, ippsFIROne_Direct_32fc_I, (Ipp32fc* pSrcDstVal, const Ipp32fc* pTaps, int tapsLen,
        Ipp32fc* pDlyLine, int* pDlyLineIndex))

IPPAPI(IppStatus, ippsFIROne32f_Direct_16s_Sfs, (Ipp16s src, Ipp16s* pDstVal, const Ipp32f* pTaps, int tapsLen,
        Ipp16s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIROne32fc_Direct_16sc_Sfs, (Ipp16sc src, Ipp16sc* pDstVal, const Ipp32fc* pTaps, int tapsLen,
        Ipp16sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIROne32f_Direct_16s_ISfs, (Ipp16s* pSrcDstVal, const Ipp32f* pTaps, int tapsLen,
        Ipp16s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIROne32fc_Direct_16sc_ISfs, (Ipp16sc* pSrcDstVal, const Ipp32fc* pTaps, int tapsLen,
        Ipp16sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIROne_Direct_64f, (Ipp64f src, Ipp64f* pDstVal, const Ipp64f* pTaps, int tapsLen,
        Ipp64f* pDlyLine, int* pDlyLineIndex))
IPPAPI(IppStatus, ippsFIROne_Direct_64fc, (Ipp64fc src, Ipp64fc* pDstVal, const Ipp64fc* pTaps, int tapsLen,
        Ipp64fc* pDlyLine, int* pDlyLineIndex))

IPPAPI(IppStatus, ippsFIROne_Direct_64f_I, (Ipp64f* pSrcDstVal, const Ipp64f* pTaps, int tapsLen,
        Ipp64f* pDlyLine, int* pDlyLineIndex))
IPPAPI(IppStatus, ippsFIROne_Direct_64fc_I, (Ipp64fc* pSrcDstVal, const Ipp64fc* pTaps, int tapsLen,
        Ipp64fc* pDlyLine, int* pDlyLineIndex))

IPPAPI(IppStatus, ippsFIROne64f_Direct_32f, (Ipp32f src, Ipp32f* pDstVal, const Ipp64f* pTaps, int tapsLen,
        Ipp32f* pDlyLine, int* pDlyLineIndex))
IPPAPI(IppStatus, ippsFIROne64fc_Direct_32fc, (Ipp32fc src, Ipp32fc* pDstVal, const Ipp64fc* pTaps, int tapsLen,
        Ipp32fc* pDlyLine, int* pDlyLineIndex))

IPPAPI(IppStatus, ippsFIROne64f_Direct_32f_I, (Ipp32f* pSrcDstVal, const Ipp64f* pTaps, int tapsLen,
        Ipp32f* pDlyLine, int* pDlyLineIndex))
IPPAPI(IppStatus, ippsFIROne64fc_Direct_32fc_I, (Ipp32fc* pSrcDstVal, const Ipp64fc* pTaps, int tapsLen,
        Ipp32fc* pDlyLine, int* pDlyLineIndex))

IPPAPI(IppStatus, ippsFIROne64f_Direct_32s_Sfs, (Ipp32s src, Ipp32s* pDstVal, const Ipp64f* pTaps, int tapsLen,
        Ipp32s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIROne64fc_Direct_32sc_Sfs, (Ipp32sc src, Ipp32sc* pDstVal, const Ipp64fc* pTaps, int tapsLen,
        Ipp32sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIROne64f_Direct_32s_ISfs, (Ipp32s* pSrcDstVal, const Ipp64f* pTaps, int tapsLen,
        Ipp32s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIROne64fc_Direct_32sc_ISfs, (Ipp32sc* pSrcDstVal, const Ipp64fc* pTaps, int tapsLen,
        Ipp32sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIROne64f_Direct_16s_Sfs, (Ipp16s src, Ipp16s* pDstVal, const Ipp64f* pTaps, int tapsLen,
        Ipp16s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIROne64fc_Direct_16sc_Sfs, (Ipp16sc src, Ipp16sc* pDstVal, const Ipp64fc* pTaps, int tapsLen,
        Ipp16sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIROne64f_Direct_16s_ISfs, (Ipp16s* pSrcDstVal, const Ipp64f* pTaps, int tapsLen,
        Ipp16s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIROne64fc_Direct_16sc_ISfs, (Ipp16sc* pSrcDstVal, const Ipp64fc* pTaps, int tapsLen,
        Ipp16sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIROne32s_Direct_16s_Sfs, (Ipp16s src, Ipp16s* pDstVal,
        const Ipp32s* pTaps, int tapsLen, int tapsFactor,
        Ipp16s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIROne32sc_Direct_16sc_Sfs, (Ipp16sc src, Ipp16sc* pDstVal,
        const Ipp32sc* pTaps, int tapsLen, int tapsFactor,
        Ipp16sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIROne32s_Direct_16s_ISfs, (Ipp16s* pSrcDstVal,
        const Ipp32s* pTaps, int tapsLen, int tapsFactor,
        Ipp16s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIROne32sc_Direct_16sc_ISfs, (Ipp16sc* pSrcDstVal,
        const Ipp32sc* pTaps, int tapsLen, int tapsFactor,
        Ipp16sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

/* ///////////////////////////////////////////////////////////////////////////////////////////
//   Names:      ippsFIR_Direct
//   Purpose:    Directly filters a block of samples through a single-rate FIR filter.
//   Parameters:
//      pSrc           pointer to the input array
//      pDst           pointer to the output array
//      pSrcDst        pointer to the input and output array for in-place operation.
//      numIters       number of samples in the input array
//      pTaps          pointer to the array containing the taps values,
//                       the number of elements in the array is tapsLen
//      tapsLen        number of elements in the array containing the taps values.
//      tapsFactor     scale factor for the taps of Ipp32s data type
//                               (for integer versions only).
//      pDlyLine       pointer to the array containing the delay line values,
//                        the number of elements in the array is 2*tapsLen
//      pDlyLineIndex  pointer to the current delay line index
//      scaleFactor    integer scaling factor value
//   Return:
//      ippStsNullPtrErr       pointer(s) to data arrays is(are) NULL
//      ippStsFIRLenErr        tapsLen is less than or equal to 0
//      ippStsSizeErr          numIters is less than or equal to 0
//      ippStsNoErr            otherwise
*/

IPPAPI(IppStatus, ippsFIR_Direct_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int numIters, const Ipp32f* pTaps, int tapsLen,
        Ipp32f* pDlyLine, int* pDlyLineIndex))
IPPAPI(IppStatus, ippsFIR_Direct_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int numIters, const Ipp32fc* pTaps, int tapsLen,
        Ipp32fc* pDlyLine, int* pDlyLineIndex))

IPPAPI(IppStatus, ippsFIR_Direct_32f_I, (Ipp32f* pSrcDst, int numIters, const Ipp32f* pTaps, int tapsLen,
        Ipp32f* pDlyLine, int* pDlyLineIndex))
IPPAPI(IppStatus, ippsFIR_Direct_32fc_I, (Ipp32fc* pSrcDst, int numIters, const Ipp32fc* pTaps, int tapsLen,
        Ipp32fc* pDlyLine, int* pDlyLineIndex))

IPPAPI(IppStatus, ippsFIR32f_Direct_16s_Sfs, (const Ipp16s* pSrc, Ipp16s* pDst, int numIters, const Ipp32f* pTaps, int tapsLen,
        Ipp16s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR32fc_Direct_16sc_Sfs, (const Ipp16sc* pSrc, Ipp16sc* pDst, int numIters, const Ipp32fc* pTaps, int tapsLen,
        Ipp16sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIR32f_Direct_16s_ISfs, (Ipp16s* pSrcDst, int numIters, const Ipp32f* pTaps, int tapsLen,
        Ipp16s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR32fc_Direct_16sc_ISfs, (Ipp16sc* pSrcDst, int numIters, const Ipp32fc* pTaps, int tapsLen,
        Ipp16sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIR_Direct_64f, (const Ipp64f* pSrc, Ipp64f* pDst, int numIters, const Ipp64f* pTaps, int tapsLen,
        Ipp64f* pDlyLine, int* pDlyLineIndex))
IPPAPI(IppStatus, ippsFIR_Direct_64fc, (const Ipp64fc* pSrc, Ipp64fc* pDst, int numIters, const Ipp64fc* pTaps, int tapsLen,
        Ipp64fc* pDlyLine, int* pDlyLineIndex))

IPPAPI(IppStatus, ippsFIR_Direct_64f_I, (Ipp64f* pSrcDst, int numIters, const Ipp64f* pTaps, int tapsLen,
        Ipp64f* pDlyLine, int* pDlyLineIndex))
IPPAPI(IppStatus, ippsFIR_Direct_64fc_I, (Ipp64fc* pSrcDst, int numIters, const Ipp64fc* pTaps, int tapsLen,
        Ipp64fc* pDlyLine, int* pDlyLineIndex))

IPPAPI(IppStatus, ippsFIR64f_Direct_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int numIters, const Ipp64f* pTaps, int tapsLen,
        Ipp32f* pDlyLine, int* pDlyLineIndex))
IPPAPI(IppStatus, ippsFIR64fc_Direct_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int numIters, const Ipp64fc* pTaps, int tapsLen,
        Ipp32fc* pDlyLine, int* pDlyLineIndex))

IPPAPI(IppStatus, ippsFIR64f_Direct_32f_I, (Ipp32f* pSrcDst, int numIters, const Ipp64f* pTaps, int tapsLen,
        Ipp32f* pDlyLine, int* pDlyLineIndex))
IPPAPI(IppStatus, ippsFIR64fc_Direct_32fc_I, (Ipp32fc* pSrcDst, int numIters, const Ipp64fc* pTaps, int tapsLen,
        Ipp32fc* pDlyLine, int* pDlyLineIndex))

IPPAPI(IppStatus, ippsFIR64f_Direct_32s_Sfs, (const Ipp32s* pSrc, Ipp32s* pDst, int numIters, const Ipp64f* pTaps, int tapsLen,
        Ipp32s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR64fc_Direct_32sc_Sfs, (const Ipp32sc* pSrc, Ipp32sc* pDst, int numIters, const Ipp64fc* pTaps, int tapsLen,
        Ipp32sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIR64f_Direct_32s_ISfs, (Ipp32s* pSrcDst, int numIters, const Ipp64f* pTaps, int tapsLen,
        Ipp32s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR64fc_Direct_32sc_ISfs, (Ipp32sc* pSrcDst, int numIters, const Ipp64fc* pTaps, int tapsLen,
        Ipp32sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIR64f_Direct_16s_Sfs, (const Ipp16s* pSrc, Ipp16s* pDst, int numIters, const Ipp64f* pTaps, int tapsLen,
        Ipp16s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR64fc_Direct_16sc_Sfs, (const Ipp16sc* pSrc, Ipp16sc* pDst, int numIters, const Ipp64fc* pTaps, int tapsLen,
        Ipp16sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIR64f_Direct_16s_ISfs, (Ipp16s* pSrcDst, int numIters, const Ipp64f* pTaps, int tapsLen,
        Ipp16s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR64fc_Direct_16sc_ISfs, (Ipp16sc* pSrcDst, int numIters, const Ipp64fc* pTaps, int tapsLen,
        Ipp16sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIR32s_Direct_16s_Sfs, (const Ipp16s* pSrc, Ipp16s* pDst, int numIters,
        const Ipp32s* pTaps, int tapsLen, int tapsFactor,
        Ipp16s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR32sc_Direct_16sc_Sfs, (const Ipp16sc* pSrc, Ipp16sc* pDst, int numIters,
        const Ipp32sc* pTaps, int tapsLen, int tapsFactor,
        Ipp16sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

IPPAPI(IppStatus, ippsFIR32s_Direct_16s_ISfs, (Ipp16s* pSrcDst, int numIters,
        const Ipp32s* pTaps, int tapsLen, int tapsFactor,
        Ipp16s* pDlyLine, int* pDlyLineIndex, int scaleFactor ))
IPPAPI(IppStatus, ippsFIR32sc_Direct_16sc_ISfs, (Ipp16sc* pSrcDst, int numIters,
        const Ipp32sc* pTaps, int tapsLen, int tapsFactor,
        Ipp16sc* pDlyLine, int* pDlyLineIndex, int scaleFactor ))

/* ///////////////////////////////////////////////////////////////////////////////////////////
//   Names:      ippsFIRMR_Direct
//   Purpose:    Directly filters a block of samples through a multi-rate FIR filter.
//   Parameters:
//      pSrc           pointer to the input array
//      pDst           pointer to the output array
//      pSrcDst        pointer to the input and output array for in-place operation.
//      numIters       number of iterations in the input array
//      pTaps          pointer to the array containing the taps values,
//                       the number of elements in the array is tapsLen
//      tapsLen        number of elements in the array containing the taps values.
//      tapsFactor     scale factor for the taps of Ipp32s data type
//                               (for integer versions only).
//      pDlyLine       pointer to the array containing the delay line values
//      upFactor       up-sampling factor
//      downFactor     down-sampling factor
//      upPhase        up-sampling phase
//      downPhase      down-sampling phase
//      scaleFactor    integer scaling factor value
//   Return:
//      ippStsNullPtrErr       pointer(s) to data arrays is(are) NULL
//      ippStsFIRLenErr        tapsLen is less than or equal to 0
//      ippStsSizeErr          numIters is less than or equal to 0
//      ippStsFIRMRFactorErr   upFactor (downFactor) is less than or equal to 0
//      ippStsFIRMRPhaseErr    upPhase (downPhase) is negative,
//                                       or less than or equal to upFactor (downFactor).
//      ippStsNoErr            otherwise
*/


IPPAPI(IppStatus, ippsFIRMR_Direct_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int numIters,
        const Ipp32f* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsFIRMR_Direct_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int numIters,
        const Ipp32fc* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp32fc* pDlyLine))

IPPAPI(IppStatus, ippsFIRMR_Direct_32f_I, (Ipp32f* pSrcDst, int numIters,
        const Ipp32f* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsFIRMR_Direct_32fc_I, (Ipp32fc* pSrcDst, int numIters,
        const Ipp32fc* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp32fc* pDlyLine))

IPPAPI(IppStatus, ippsFIRMR32f_Direct_16s_Sfs, (const Ipp16s* pSrc, Ipp16s* pDst, int numIters,
        const Ipp32f* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp16s* pDlyLine, int scaleFactor))
IPPAPI(IppStatus, ippsFIRMR32fc_Direct_16sc_Sfs, (const Ipp16sc* pSrc, Ipp16sc* pDst, int numIters,
        const Ipp32fc* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp16sc* pDlyLine, int scaleFactor))

IPPAPI(IppStatus, ippsFIRMR32f_Direct_16s_ISfs, (Ipp16s* pSrcDst, int numIters,
        const Ipp32f* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp16s* pDlyLine, int scaleFactor))
IPPAPI(IppStatus, ippsFIRMR32fc_Direct_16sc_ISfs, (Ipp16sc* pSrcDst, int numIters,
        const Ipp32fc* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp16sc* pDlyLine, int scaleFactor))

IPPAPI(IppStatus, ippsFIRMR_Direct_64f, (const Ipp64f* pSrc, Ipp64f* pDst, int numIters,
        const Ipp64f* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsFIRMR_Direct_64fc, (const Ipp64fc* pSrc, Ipp64fc* pDst, int numIters,
        const Ipp64fc* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp64fc* pDlyLine))

IPPAPI(IppStatus, ippsFIRMR_Direct_64f_I, (Ipp64f* pSrcDst, int numIters,
        const Ipp64f* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp64f* pDlyLine))
IPPAPI(IppStatus, ippsFIRMR_Direct_64fc_I, (Ipp64fc* pSrcDst, int numIters,
        const Ipp64fc* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp64fc* pDlyLine))

IPPAPI(IppStatus, ippsFIRMR64f_Direct_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int numIters,
        const Ipp64f* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsFIRMR64fc_Direct_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int numIters,
        const Ipp64fc* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp32fc* pDlyLine))

IPPAPI(IppStatus, ippsFIRMR64f_Direct_32f_I, (Ipp32f* pSrcDst, int numIters,
        const Ipp64f* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp32f* pDlyLine))
IPPAPI(IppStatus, ippsFIRMR64fc_Direct_32fc_I, (Ipp32fc* pSrcDst, int numIters,
        const Ipp64fc* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp32fc* pDlyLine))

IPPAPI(IppStatus, ippsFIRMR64f_Direct_32s_Sfs, (const Ipp32s* pSrc, Ipp32s* pDst, int numIters,
        const Ipp64f* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp32s* pDlyLine, int scaleFactor))
IPPAPI(IppStatus, ippsFIRMR64fc_Direct_32sc_Sfs, (const Ipp32sc* pSrc, Ipp32sc* pDst, int numIters,
        const Ipp64fc* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp32sc* pDlyLine, int scaleFactor))

IPPAPI(IppStatus, ippsFIRMR64f_Direct_32s_ISfs, (Ipp32s* pSrcDst, int numIters,
        const Ipp64f* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp32s* pDlyLine, int scaleFactor))
IPPAPI(IppStatus, ippsFIRMR64fc_Direct_32sc_ISfs, (Ipp32sc* pSrcDst, int numIters,
        const Ipp64fc* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp32sc* pDlyLine, int scaleFactor))

IPPAPI(IppStatus, ippsFIRMR64f_Direct_16s_Sfs, (const Ipp16s* pSrc, Ipp16s* pDst, int numIters,
        const Ipp64f* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp16s* pDlyLine, int scaleFactor))
IPPAPI(IppStatus, ippsFIRMR64fc_Direct_16sc_Sfs, (const Ipp16sc* pSrc, Ipp16sc* pDst, int numIters,
        const Ipp64fc* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp16sc* pDlyLine, int scaleFactor))

IPPAPI(IppStatus, ippsFIRMR64f_Direct_16s_ISfs, (Ipp16s* pSrcDst, int numIters,
        const Ipp64f* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp16s* pDlyLine, int scaleFactor))
IPPAPI(IppStatus, ippsFIRMR64fc_Direct_16sc_ISfs, (Ipp16sc* pSrcDst, int numIters,
        const Ipp64fc* pTaps, int tapsLen, int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp16sc* pDlyLine, int scaleFactor))

IPPAPI(IppStatus, ippsFIRMR32s_Direct_16s_Sfs, (const Ipp16s* pSrc, Ipp16s* pDst, int numIters,
        const Ipp32s* pTaps, int tapsLen, int tapsFactor,
        int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp16s* pDlyLine, int scaleFactor))
IPPAPI(IppStatus, ippsFIRMR32sc_Direct_16sc_Sfs, (const Ipp16sc* pSrc, Ipp16sc* pDst, int numIters,
        const Ipp32sc* pTaps, int tapsLen, int tapsFactor,
        int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp16sc* pDlyLine, int scaleFactor))

IPPAPI(IppStatus, ippsFIRMR32s_Direct_16s_ISfs, (Ipp16s* pSrcDst, int numIters,
        const Ipp32s* pTaps, int tapsLen, int tapsFactor,
        int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp16s* pDlyLine, int scaleFactor))
IPPAPI(IppStatus, ippsFIRMR32sc_Direct_16sc_ISfs, (Ipp16sc* pSrcDst, int numIters,
        const Ipp32sc* pTaps, int tapsLen, int tapsFactor,
        int upFactor,int upPhase, int downFactor, int downPhase,
        Ipp16sc* pDlyLine, int scaleFactor))


/* ////////////////////////////////////////////////////////////////////////////
//  Names:      ippsFIR_Direct_16s_Sfs,
//              ippsFIR_Direct_16s_ISfs,
//              ippsFIROne_Direct_16s_Sfs,
//              ippsFIROne_Direct_16s_ISfs.
//  Purpose:    Directly filters a block of samples (or one sample in 'One'
//              case) through a single-rate FIR filter with fixed point taps
//              ( Q15 ).
//   Parameters:
//      pSrc            pointer to the input array.
//      src             input sample in 'One' case.
//      pDst            pointer to the output array.
//      pDstVal         pointer to the output sample in 'One' case.
//      pSrcDst         pointer to the input and output array for in-place
//                      operation.
//      pSrcDstVal      pointer to the input and output sample for in-place
//                      operation in 'One' case.
//      numIters        number of samples in the input array.
//      pTapsQ15        pointer to the array containing the taps values,
//                      the number of elements in the array is tapsLen.
//      tapsLen         number of elements in the array containing the taps
//                      values.
//      pDlyLine        pointer to the array containing the delay line values,
//                      the number of elements in the array is 2 * tapsLen.
//      pDlyLineIndex   pointer to the current delay line index.
//      scaleFactor     integer scaling factor value.
//   Return:
//      ippStsNullPtrErr       pointer(s) to data arrays is(are) NULL.
//      ippStsFIRLenErr        tapsLen is less than or equal to 0.
//      ippStsSizeErr          sampLen is less than or equal to 0.
//      ippStsDlyLineIndexErr  current delay line index is greater or equal
//                             tapsLen, or less than 0.
//      ippStsNoErr            otherwise.
*/

IPPAPI( IppStatus, ippsFIR_Direct_16s_Sfs,( const Ipp16s* pSrc, Ipp16s* pDst,
          int numIters, const Ipp16s* pTapsQ15, int tapsLen, Ipp16s* pDlyLine,
                                      int* pDlyLineIndex, int scaleFactor ))

IPPAPI( IppStatus, ippsFIR_Direct_16s_ISfs,( Ipp16s* pSrcDst, int numIters,
                       const Ipp16s* pTapsQ15, int tapsLen, Ipp16s* pDlyLine,
                                      int* pDlyLineIndex, int scaleFactor ))

IPPAPI( IppStatus, ippsFIROne_Direct_16s_Sfs,( Ipp16s src, Ipp16s* pDstVal,
                       const Ipp16s* pTapsQ15, int tapsLen, Ipp16s* pDlyLine,
                                      int* pDlyLineIndex, int scaleFactor ))

IPPAPI( IppStatus, ippsFIROne_Direct_16s_ISfs,( Ipp16s* pSrcDstVal,
                      const Ipp16s* pTapsQ15, int tapsLen, Ipp16s * pDlyLine,
                                      int* pDlyLineIndex, int scaleFactor ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsFIRGenLowpass_64f, ippsFIRGenHighpass_64f, ippsFIRGenBandpass_64f
//              ippsFIRGenBandstop_64f

//  Purpose:    This function computes the lowpass FIR filter coefficients
//              by windowing of ideal (infinite) filter coefficients segment
//
//  Parameters:
//      rfreq             cut off frequency (0 < rfreq < 0.5)
//
//      taps              pointer to the array which specifies
//                        the filter coefficients;
//
//      tapsLen           the number of taps in taps[] array (tapsLen>=5);
//
//      winType           the ippWindowType switch variable,
//                        which specifies the smoothing window type;
//
//      doNormal          if doNormal=0 the functions calculates
//                        non-normalized sequence of filter coefficients,
//                        in other cases the sequence of coefficients
//                        will be normalized.
//  Return:
//   ippStsNullPtrErr     the null pointer to taps[] array pass to function
//   ippStsSizeErr        the length of coefficient's array is less than five
//   ippStsSizeErr        the low or high frequency isn't satisfy
//                                    the condition 0 < rLowFreq < 0.5
//   ippStsNoErr          otherwise
//
*/

IPPAPI(IppStatus, ippsFIRGenLowpass_64f, (Ipp64f rfreq, Ipp64f* taps, int tapsLen,
                                            IppWinType winType, IppBool doNormal))


IPPAPI(IppStatus, ippsFIRGenHighpass_64f, (Ipp64f rfreq, Ipp64f* taps, int tapsLen,
                                             IppWinType winType, IppBool doNormal))


IPPAPI(IppStatus, ippsFIRGenBandpass_64f, (Ipp64f rLowFreq, Ipp64f rHighFreq, Ipp64f* taps,
                                     int tapsLen, IppWinType winType, IppBool doNormal))


IPPAPI(IppStatus, ippsFIRGenBandstop_64f, (Ipp64f rLowFreq, Ipp64f rHighFreq, Ipp64f* taps,
                                     int tapsLen, IppWinType winType, IppBool doNormal))

/* /////////////////////////////////////////////////////////////////////////////
//                  Windowing functions
//  Note: to create the window coefficients you have to make two calls
//        Set(1,x,n) and Win(x,n)
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:            ippsWinBartlett
//  Parameters:
//   pSrcDst          pointer to the vector
//   len              length of the vector, window size
//  Return:
//   ippStsNullPtrErr    pointer to the vector is NULL
//   ippStsSizeErr       length of the vector is less 3
//   ippStsNoErr         otherwise
*/

IPPAPI(IppStatus, ippsWinBartlett_16s_I, (Ipp16s* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBartlett_16sc_I, (Ipp16sc* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBartlett_32f_I, (Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBartlett_32fc_I, (Ipp32fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBartlett_16s, (const Ipp16s* pSrc, Ipp16s* pDst, int len))
IPPAPI(IppStatus, ippsWinBartlett_16sc, (const Ipp16sc* pSrc, Ipp16sc* pDst, int len))
IPPAPI(IppStatus, ippsWinBartlett_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsWinBartlett_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int len))
IPPAPI(IppStatus, ippsWinBartlett_64f, (const Ipp64f*  pSrc, Ipp64f*  pDst, int len))
IPPAPI(IppStatus, ippsWinBartlett_64fc,(const Ipp64fc* pSrc, Ipp64fc* pDst, int len))
IPPAPI(IppStatus, ippsWinBartlett_64f_I, (Ipp64f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBartlett_64fc_I,(Ipp64fc* pSrcDst, int len))


/* /////////////////////////////////////////////////////////////////////////////
//  Names:            ippsWinHann
//  Parameters:
//   pSrcDst          pointer to the vector
//   len              length of the vector, window size
//  Return:
//   ippStsNullPtrErr    pointer to the vector is NULL
//   ippStsSizeErr       length of the vector is less 3
//   ippStsNoErr         otherwise
//  Functionality:    0.5*(1-cos(2*pi*n/(N-1)))
*/

IPPAPI(IppStatus, ippsWinHann_16s_I, (Ipp16s* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinHann_16sc_I, (Ipp16sc* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinHann_32f_I, (Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinHann_32fc_I, (Ipp32fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinHann_16s, (const Ipp16s* pSrc, Ipp16s* pDst, int len))
IPPAPI(IppStatus, ippsWinHann_16sc, (const Ipp16sc* pSrc, Ipp16sc* pDst, int len))
IPPAPI(IppStatus, ippsWinHann_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsWinHann_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int len))

IPPAPI(IppStatus, ippsWinHann_64f_I,     (Ipp64f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsWinHann_64fc_I,    (Ipp64fc* pSrcDst, int len))

IPPAPI(IppStatus, ippsWinHann_64f,     (const Ipp64f*  pSrc, Ipp64f*  pDst, int len))
IPPAPI(IppStatus, ippsWinHann_64fc,    (const Ipp64fc* pSrc, Ipp64fc* pDst, int len))



/* /////////////////////////////////////////////////////////////////////////////
//  Names:            ippsWinHamming
//  Parameters:
//   pSrcDst          pointer to the vector
//   len              length of the vector, window size
//  Return:
//   ippStsNullPtrErr    pointer to the vector is NULL
//   ippStsSizeErr       length of the vector is less 3
//   ippStsNoErr         otherwise
*/

IPPAPI(IppStatus, ippsWinHamming_16s_I, (Ipp16s* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinHamming_16sc_I, (Ipp16sc* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinHamming_32f_I, (Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinHamming_32fc_I, (Ipp32fc* pSrcDst, int len))

IPPAPI(IppStatus, ippsWinHamming_16s, (const Ipp16s* pSrc, Ipp16s* pDst, int len))
IPPAPI(IppStatus, ippsWinHamming_16sc, (const Ipp16sc* pSrc, Ipp16sc* pDst, int len))
IPPAPI(IppStatus, ippsWinHamming_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsWinHamming_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int len))

IPPAPI(IppStatus, ippsWinHamming_64f,  (const Ipp64f*  pSrc, Ipp64f*  pDst, int len))
IPPAPI(IppStatus, ippsWinHamming_64fc, (const Ipp64fc* pSrc, Ipp64fc* pDst, int len))
IPPAPI(IppStatus, ippsWinHamming_64f_I,  (Ipp64f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsWinHamming_64fc_I, (Ipp64fc* pSrcDst, int len))



/* /////////////////////////////////////////////////////////////////////////////
//  Names:            ippsWinBlackman
//  Purpose:          multiply vector by Blackman windowing function
//  Parameters:
//   pSrcDst          pointer to the vector
//   len              length of the vector, window size
//   alpha            adjustable parameter associated with the
//                    Blackman windowing equation
//   alphaQ15         scaled (scale factor 15) version of the alpha
//   scaleFactor      scale factor of the output signal
//  Return:
//   ippStsNullPtrErr    pointer to the vector is NULL
//   ippStsSizeErr       length of the vector is less 3, for Opt it's 4
//   ippStsNoErr         otherwise
//  Notes:
//     parameter alpha value
//         WinBlackmaStd   : -0.16
//         WinBlackmaOpt   : -0.5 / (1+cos(2*pi/(len-1)))
*/

IPPAPI(IppStatus, ippsWinBlackmanQ15_16s_ISfs, (Ipp16s* pSrcDst, int len,
                                             int alphaQ15, int scaleFactor))

IPPAPI(IppStatus, ippsWinBlackmanQ15_16s_I, (Ipp16s* pSrcDst, int len, int alphaQ15))
IPPAPI(IppStatus, ippsWinBlackmanQ15_16sc_I, (Ipp16sc* pSrcDst, int len, int alphaQ15))
IPPAPI(IppStatus, ippsWinBlackman_16s_I, (Ipp16s* pSrcDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinBlackman_16sc_I, (Ipp16sc* pSrcDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinBlackman_32f_I, (Ipp32f* pSrcDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinBlackman_32fc_I, (Ipp32fc* pSrcDst, int len, float alpha))

IPPAPI(IppStatus, ippsWinBlackmanQ15_16s, (const Ipp16s* pSrc, Ipp16s* pDst, int len, int alphaQ15))
IPPAPI(IppStatus, ippsWinBlackmanQ15_16sc, (const Ipp16sc* pSrc, Ipp16sc* pDst, int len, int alphaQ15))
IPPAPI(IppStatus, ippsWinBlackman_16s, (const Ipp16s* pSrc, Ipp16s* pDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinBlackman_16sc, (const Ipp16sc* pSrc, Ipp16sc* pDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinBlackman_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinBlackman_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int len, float alpha))

IPPAPI(IppStatus, ippsWinBlackmanStd_16s_I, (Ipp16s* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanStd_16sc_I, (Ipp16sc* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanStd_32f_I, (Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanStd_32fc_I, (Ipp32fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanOpt_16s_I, (Ipp16s* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanOpt_16sc_I, (Ipp16sc* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanOpt_32f_I, (Ipp32f* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanOpt_32fc_I, (Ipp32fc* pSrcDst, int len))

IPPAPI(IppStatus, ippsWinBlackmanStd_16s, (const Ipp16s* pSrc, Ipp16s* pDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanStd_16sc, (const Ipp16sc* pSrc, Ipp16sc* pDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanStd_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanStd_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanOpt_16s, (const Ipp16s* pSrc, Ipp16s* pDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanOpt_16sc, (const Ipp16sc* pSrc, Ipp16sc* pDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanOpt_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanOpt_32fc, (const Ipp32fc* pSrc, Ipp32fc* pDst, int len))

IPPAPI(IppStatus, ippsWinBlackman_64f_I, (Ipp64f*  pSrcDst, int len, double alpha))
IPPAPI(IppStatus, ippsWinBlackman_64fc_I,(Ipp64fc* pSrcDst, int len, double alpha))

IPPAPI(IppStatus, ippsWinBlackman_64f, (const Ipp64f*  pSrc, Ipp64f*  pDst, int len, double alpha))
IPPAPI(IppStatus, ippsWinBlackman_64fc,(const Ipp64fc* pSrc, Ipp64fc* pDst, int len, double alpha))

IPPAPI(IppStatus, ippsWinBlackmanStd_64f_I, (Ipp64f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanStd_64fc_I,(Ipp64fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanStd_64f, (const Ipp64f*  pSrc, Ipp64f*  pDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanStd_64fc,(const Ipp64fc* pSrc, Ipp64fc* pDst, int len))

IPPAPI(IppStatus, ippsWinBlackmanOpt_64f_I, (Ipp64f*  pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanOpt_64fc_I,(Ipp64fc* pSrcDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanOpt_64f, (const Ipp64f*  pSrc, Ipp64f*  pDst, int len))
IPPAPI(IppStatus, ippsWinBlackmanOpt_64fc,(const Ipp64fc* pSrc, Ipp64fc* pDst, int len))



/* /////////////////////////////////////////////////////////////////////////////
//  Names:            ippsWinKaiser
//  Purpose:          multiply vector by Kaiser windowing function
//  Parameters:
//   pSrcDst          pointer to the vector
//   len              length of the vector, window size
//   alpha            adjustable parameter associated with the
//                    Kaiser windowing equation
//   alphaQ15         scaled (scale factor 15) version of the alpha
//  Return:
//   ippStsNullPtrErr    pointer to the vector is NULL
//   ippStsSizeErr       length of the vector is less 1
//   ippStsHugeWinErr    window in function is huge
//   ippStsNoErr         otherwise
*/

IPPAPI(IppStatus, ippsWinKaiser_16s,    (const Ipp16s* pSrc, Ipp16s* pDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinKaiser_16s_I,  (Ipp16s* pSrcDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinKaiserQ15_16s, (const Ipp16s* pSrc, Ipp16s* pDst, int len, int alphaQ15))
IPPAPI(IppStatus, ippsWinKaiserQ15_16s_I,(Ipp16s* pSrcDst, int len, int alphaQ15))
IPPAPI(IppStatus, ippsWinKaiser_16sc,   (const Ipp16sc* pSrc, Ipp16sc* pDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinKaiser_16sc_I, (Ipp16sc* pSrcDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinKaiserQ15_16sc,(const Ipp16sc* pSrc, Ipp16sc* pDst, int len, int alphaQ15))
IPPAPI(IppStatus, ippsWinKaiserQ15_16sc_I,(Ipp16sc* pSrcDst, int len, int alphaQ15))
IPPAPI(IppStatus, ippsWinKaiser_32f,    (const Ipp32f* pSrc, Ipp32f* pDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinKaiser_32f_I,  (Ipp32f* pSrcDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinKaiser_32fc,   (const Ipp32fc* pSrc, Ipp32fc* pDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinKaiser_32fc_I, (Ipp32fc* pSrcDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinKaiser_64f,    (const Ipp64f* pSrc, Ipp64f* pDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinKaiser_64f_I,  (Ipp64f* pSrcDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinKaiser_64fc_I, (Ipp64fc* pSrcDst, int len, float alpha))
IPPAPI(IppStatus, ippsWinKaiser_64fc,   (const Ipp64fc* pSrc, Ipp64fc* pDst, int len, float alpha))

/* /////////////////////////////////////////////////////////////////////////////
//                  Median filter
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsFilterMedian
//  Purpose:    filter source data by the Median Filter
//  Parameters:
//   pSrcDst             pointer to the source vector
//   pSrc                pointer to the source vector
//   pDst                pointer to the destination vector
//   len                 length of the vector(s)
//   maskSize            median mask size (odd)
//  Return:
//   ippStsNullPtrErr              pointer(s) to the data is NULL
//   ippStsSizeErr                 length of the vector(s) is less or equal zero
//   ippStsEvenMedianMaskSize      median mask size is even warning
//   ippStsNoErr                   otherwise
//  Notes:
//      - if len is even than len=len-1
//      - value of not existed point equals to the last point value,
//        for example, x[-1]=x[0] or x[len]=x[len-1]
*/
IPPAPI(IppStatus,ippsFilterMedian_32f_I,(Ipp32f* pSrcDst,int len,int maskSize))
IPPAPI(IppStatus,ippsFilterMedian_64f_I,(Ipp64f* pSrcDst,int len,int maskSize))
IPPAPI(IppStatus,ippsFilterMedian_16s_I,(Ipp16s* pSrcDst,int len,int maskSize))
IPPAPI(IppStatus,ippsFilterMedian_8u_I,(Ipp8u* pSrcDst,int len,int maskSize))

IPPAPI(IppStatus,ippsFilterMedian_32f,(const Ipp32f* pSrc, Ipp32f *pDst,
                                       int len, int maskSize ))
IPPAPI(IppStatus,ippsFilterMedian_64f,(const Ipp64f* pSrc, Ipp64f *pDst,
                                       int len, int maskSize ))
IPPAPI(IppStatus,ippsFilterMedian_16s,(const Ipp16s* pSrc, Ipp16s *pDst,
                                       int len, int maskSize ))
IPPAPI(IppStatus,ippsFilterMedian_8u,(const Ipp8u* pSrc, Ipp8u *pDst,
                                      int len, int maskSize ))

IPPAPI(IppStatus,ippsFilterMedian_32s_I,(Ipp32s* pSrcDst,int len,int maskSize))
IPPAPI(IppStatus,ippsFilterMedian_32s,(const Ipp32s* pSrc, Ipp32s *pDst,
                                       int len, int maskSize ))


/* /////////////////////////////////////////////////////////////////////////////
//                  Statistic functions
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//  Name:            ippsNorm
//  Purpose:         calculate norm of vector
//     Inf   - calculate C-norm of vector:  n = MAX |src1|
//     L1    - calculate L1-norm of vector: n = SUM |src1|
//     L2    - calculate L2-norm of vector: n = SQRT(SUM |src1|^2)
//     L2Sqr - calculate L2-norm of vector: n = SUM |src1|^2
//  Parameters:
//    pSrc           source data pointer
//    len            length of vector
//    pNorm          pointer to result
//    scaleFactor    scale factor value
//  Returns:
//    ippStsNoErr       Ok
//    ippStsNullPtrErr  Some of pointers to input or output data are NULL
//    ippStsSizeErr     The length of vector is less or equal zero
//  Notes:
*/
IPPAPI(IppStatus, ippsNorm_Inf_16s32f, (const Ipp16s* pSrc, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNorm_Inf_16s32s_Sfs, (const Ipp16s* pSrc, int len, Ipp32s* pNorm, int scaleFactor))
IPPAPI(IppStatus, ippsNorm_Inf_32f, (const Ipp32f* pSrc, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNorm_Inf_64f, (const Ipp64f* pSrc, int len, Ipp64f* pNorm))
IPPAPI(IppStatus, ippsNorm_L1_16s32f, (const Ipp16s* pSrc, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNorm_L1_16s32s_Sfs, (const Ipp16s* pSrc, int len, Ipp32s* pNorm, int scaleFactor))
IPPAPI(IppStatus, ippsNorm_L1_32f, (const Ipp32f* pSrc, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNorm_L1_64f, (const Ipp64f* pSrc, int len, Ipp64f* pNorm))
IPPAPI(IppStatus, ippsNorm_L2_16s32f, (const Ipp16s* pSrc, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNorm_L2_16s32s_Sfs, (const Ipp16s* pSrc, int len, Ipp32s* pNorm, int scaleFactor))
IPPAPI(IppStatus, ippsNorm_L2_32f, (const Ipp32f* pSrc, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNorm_L2_64f, (const Ipp64f* pSrc, int len, Ipp64f* pNorm))

IPPAPI(IppStatus, ippsNorm_Inf_32fc32f,(const Ipp32fc* pSrc, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNorm_Inf_64fc64f,(const Ipp64fc* pSrc, int len, Ipp64f* pNorm))
IPPAPI(IppStatus, ippsNorm_L1_32fc64f, (const Ipp32fc* pSrc, int len, Ipp64f* pNorm))
IPPAPI(IppStatus, ippsNorm_L1_64fc64f, (const Ipp64fc* pSrc, int len, Ipp64f* pNorm))
IPPAPI(IppStatus, ippsNorm_L2_32fc64f, (const Ipp32fc* pSrc, int len, Ipp64f* pNorm))
IPPAPI(IppStatus, ippsNorm_L2_64fc64f, (const Ipp64fc* pSrc, int len, Ipp64f* pNorm))

IPPAPI(IppStatus, ippsNorm_L1_16s64s_Sfs, (const Ipp16s* pSrc, int len, Ipp64s* pNorm, int scaleFactor))
IPPAPI(IppStatus, ippsNorm_L2Sqr_16s64s_Sfs, (const Ipp16s* pSrc, int len, Ipp64s* pNorm, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:            ippsNormDiff
//  Purpose:         calculate norm of vectors
//     Inf   - calculate C-norm of vectors:  n = MAX |src1-src2|
//     L1    - calculate L1-norm of vectors: n = SUM |src1-src2|
//     L2    - calculate L2-norm of vectors: n = SQRT(SUM |src1-src2|^2)
//     L2Sqr - calculate L2-norm of vectors: n = SUM |src1-src2|^2
//  Parameters:
//    pSrc1, pSrc2   source data pointers
//    len            length of vector
//    pNorm          pointer to result
//    scaleFactor    scale factor value
//  Returns:
//    ippStsNoErr       Ok
//    ippStsNullPtrErr  Some of pointers to input or output data are NULL
//    ippStsSizeErr     The length of vector is less or equal zero
//  Notes:
*/
IPPAPI(IppStatus, ippsNormDiff_Inf_16s32f, (const Ipp16s* pSrc1, const Ipp16s* pSrc2, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNormDiff_Inf_16s32s_Sfs, (const Ipp16s* pSrc1, const Ipp16s* pSrc2, int len, Ipp32s* pNorm, int scaleFactor))
IPPAPI(IppStatus, ippsNormDiff_Inf_32f, (const Ipp32f* pSrc1, const Ipp32f* pSrc2, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNormDiff_Inf_64f, (const Ipp64f* pSrc1, const Ipp64f* pSrc2, int len, Ipp64f* pNorm))
IPPAPI(IppStatus, ippsNormDiff_L1_16s32f, (const Ipp16s* pSrc1, const Ipp16s* pSrc2, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNormDiff_L1_16s32s_Sfs, (const Ipp16s* pSrc1, const Ipp16s* pSrc2, int len, Ipp32s* pNorm, int scaleFactor))
IPPAPI(IppStatus, ippsNormDiff_L1_32f, (const Ipp32f* pSrc1, const Ipp32f* pSrc2, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNormDiff_L1_64f, (const Ipp64f* pSrc1, const Ipp64f* pSrc2, int len, Ipp64f* pNorm))
IPPAPI(IppStatus, ippsNormDiff_L2_16s32f, (const Ipp16s* pSrc1, const Ipp16s* pSrc2, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNormDiff_L2_16s32s_Sfs, (const Ipp16s* pSrc1, const Ipp16s* pSrc2, int len, Ipp32s* pNorm, int scaleFactor))
IPPAPI(IppStatus, ippsNormDiff_L2_32f, (const Ipp32f* pSrc1, const Ipp32f* pSrc2, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNormDiff_L2_64f, (const Ipp64f* pSrc1, const Ipp64f* pSrc2, int len, Ipp64f* pNorm))

IPPAPI(IppStatus, ippsNormDiff_Inf_32fc32f,(const Ipp32fc* pSrc1, const Ipp32fc* pSrc2, int len, Ipp32f* pNorm))
IPPAPI(IppStatus, ippsNormDiff_Inf_64fc64f,(const Ipp64fc* pSrc1, const Ipp64fc* pSrc2, int len, Ipp64f* pNorm))
IPPAPI(IppStatus, ippsNormDiff_L1_32fc64f,(const Ipp32fc* pSrc1, const Ipp32fc* pSrc2, int len, Ipp64f* pNorm))
IPPAPI(IppStatus, ippsNormDiff_L1_64fc64f,(const Ipp64fc* pSrc1, const Ipp64fc* pSrc2, int len, Ipp64f* pNorm))
IPPAPI(IppStatus, ippsNormDiff_L2_32fc64f,(const Ipp32fc* pSrc1, const Ipp32fc* pSrc2, int len, Ipp64f* pNorm))
IPPAPI(IppStatus, ippsNormDiff_L2_64fc64f,(const Ipp64fc* pSrc1, const Ipp64fc* pSrc2, int len, Ipp64f* pNorm))

IPPAPI(IppStatus, ippsNormDiff_L1_16s64s_Sfs, (const Ipp16s* pSrc1, const Ipp16s* pSrc2, int len, Ipp64s* pNorm, int scaleFactor))
IPPAPI(IppStatus, ippsNormDiff_L2Sqr_16s64s_Sfs, (const Ipp16s* pSrc1, const Ipp16s* pSrc2, int len, Ipp64s* pNorm, int scaleFactor))


/* /////////////////////////////////////////////////////////////////////////////
//                Cross Correlation Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsCrossCorr_32f,  ippsCrossCorr_64f,
//              ippsCrossCorr_32fc, ippsCrossCorr_64fc
//
//  Purpose:    Calculate Cross Correlation
//
//  Arguments:
//     pSrc1   - pointer to the vector_1 source
//     src1Len - vector_1 source length
//     pSrc2   - pointer to the vector_2 source
//     src2Len - vector_2 source length
//     pDst    - pointer to the cross correlation
//     dstLen  - length of cross-correlation
//     lowLag  - cross-correlation lowest lag
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  either pSrc1 or(and) pSrc2 are NULL
//   ippStsSizeErr     vector's length is not positive
//
*/
IPPAPI(IppStatus, ippsCrossCorr_32f, (const Ipp32f* pSrc1, int src1Len, const Ipp32f* pSrc2, int src2Len, Ipp32f* pDst,  int dstLen, int lowLag))
IPPAPI(IppStatus, ippsCrossCorr_64f, (const Ipp64f* pSrc1, int src1Len, const Ipp64f* pSrc2, int src2Len, Ipp64f* pDst,  int dstLen, int lowLag))
IPPAPI(IppStatus, ippsCrossCorr_32fc,(const Ipp32fc* pSrc1, int src1Len, const Ipp32fc* pSrc2, int src2Len, Ipp32fc* pDst,  int dstLen, int lowLag))
IPPAPI(IppStatus, ippsCrossCorr_64fc,(const Ipp64fc* pSrc1, int src1Len, const Ipp64fc* pSrc2, int src2Len, Ipp64fc* pDst,  int dstLen, int lowLag))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsCrossCorr_16s_Sfs
//              ippsCrossCorr_16s64s
//
//  Purpose:    Calculate Cross Correlation and Scale Result (with saturate)
//
//  Arguments:
//     pSrc1   - pointer to the vector_1 source
//     src1Len - vector_1 source length
//     pSrc2   - pointer to the vector_2 source
//     src2Len - vector_2 source length
//     pDst    - pointer to the cross correlation
//     dstLen  - length of cross-correlation
//     lowLag  - cross-correlation lowest lag
//     scaleFactor - scale factor value
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  either pSrc1 or(and) pSrc2 are NULL
//   ippStsSizeErr     vector's length is not positive
//
*/
IPPAPI(IppStatus, ippsCrossCorr_16s_Sfs, (const Ipp16s* pSrc1, int src1Len, const Ipp16s* pSrc2, int src2Len,
                                                   Ipp16s* pDst, int dstLen, int lowLag, int scaleFactor))
IPPAPI(IppStatus, ippsCrossCorr_16s64s, (const Ipp16s* pSrc1, int src1Len, const Ipp16s* pSrc2, int src2Len,
                                                                    Ipp64s* pDst, int dstLen, int lowLag))
/* /////////////////////////////////////////////////////////////////////////////
//                AutoCorrelation Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Names:  ippsAutoCorr_32f,  ippsAutoCorr_NormA_32f,  ippsAutoCorr_NormB_32f,
//          ippsAutoCorr_64f,  ippsAutoCorr_NormA_64f,  ippsAutoCorr_NormB_64f,
//          ippsAutoCorr_32fc, ippsAutoCorr_NormA_32fc, ippsAutoCorr_NormB_32fc,
//          ippsAutoCorr_64fc, ippsAutoCorr_NormA_64fc, ippsAutoCorr_NormB_64fc,
//
//  Purpose:    Calculate the autocorrelation,
//              without suffix NormX specifies that the normal autocorrelation to be
//              computed;
//              suffix NormA specifies that the biased autocorrelation to be
//              computed (the resulting values are to be divided on srcLen);
//              suffix NormB specifies that the unbiased autocorrelation to be
//              computed (the resulting values are to be divided on ( srcLen - n ),
//              where "n" means current iteration).
//
//  Arguments:
//     pSrc   - pointer to the source vector
//     srcLen - source vector length
//     pDst   - pointer to the auto-correlation result vector
//     dstLen - length of auto-correlation
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  either pSrc or(and) pDst are NULL
//   ippStsSizeErr     vector's length is not positive
*/

IPPAPI(IppStatus, ippsAutoCorr_32f, ( const Ipp32f* pSrc, int srcLen, Ipp32f* pDst, int dstLen ))
IPPAPI(IppStatus, ippsAutoCorr_NormA_32f, ( const Ipp32f* pSrc, int srcLen, Ipp32f* pDst, int dstLen ))
IPPAPI(IppStatus, ippsAutoCorr_NormB_32f, ( const Ipp32f* pSrc, int srcLen, Ipp32f* pDst, int dstLen ))
IPPAPI(IppStatus, ippsAutoCorr_64f, ( const Ipp64f* pSrc, int srcLen, Ipp64f* pDst, int dstLen ))
IPPAPI(IppStatus, ippsAutoCorr_NormA_64f, ( const Ipp64f* pSrc, int srcLen, Ipp64f* pDst, int dstLen ))
IPPAPI(IppStatus, ippsAutoCorr_NormB_64f, ( const Ipp64f* pSrc, int srcLen, Ipp64f* pDst, int dstLen ))
IPPAPI(IppStatus, ippsAutoCorr_32fc,( const Ipp32fc* pSrc, int srcLen, Ipp32fc* pDst, int dstLen ))
IPPAPI(IppStatus, ippsAutoCorr_NormA_32fc,( const Ipp32fc* pSrc, int srcLen, Ipp32fc* pDst, int dstLen ))
IPPAPI(IppStatus, ippsAutoCorr_NormB_32fc,( const Ipp32fc* pSrc, int srcLen, Ipp32fc* pDst, int dstLen ))
IPPAPI(IppStatus, ippsAutoCorr_64fc,( const Ipp64fc* pSrc, int srcLen, Ipp64fc* pDst, int dstLen ))
IPPAPI(IppStatus, ippsAutoCorr_NormA_64fc,( const Ipp64fc* pSrc, int srcLen, Ipp64fc* pDst, int dstLen ))
IPPAPI(IppStatus, ippsAutoCorr_NormB_64fc,( const Ipp64fc* pSrc, int srcLen, Ipp64fc* pDst, int dstLen ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:  ippsAutoCorr_16s_Sfs,
//          ippsAutoCorr_NormA_16s_Sfs,
//          ippsAutoCorr_NormB_16s_Sfs
//
//  Purpose:    Calculate the autocorrelation,
//              without suffix NormX specifies that the normal autocorrelation to be
//              computed;
//              suffix NormA specifies that the biased autocorrelation to be
//              computed (the resulting values are to be divided on srcLen);
//              suffix NormB specifies that the unbiased autocorrelation to be
//              computed (the resulting values are to be divided on ( srcLen - n ),
//              where n means current iteration).
//
//  Arguments:
//     pSrc   - pointer to the source vector
//     srcLen - source vector length
//     pDst   - pointer to the auto-correlation result vector
//     dstLen - length of auto-correlation
//     scaleFactor - scale factor value
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  either pSrc or(and) pDst are NULL
//   ippStsSizeErr     vector's length is not positive
*/

IPPAPI(IppStatus, ippsAutoCorr_16s_Sfs,( const Ipp16s* pSrc, int srcLen, Ipp16s* pDst,
                                                        int dstLen, int scaleFactor ))
IPPAPI(IppStatus, ippsAutoCorr_NormA_16s_Sfs,( const Ipp16s* pSrc, int srcLen, Ipp16s* pDst,
                                                        int dstLen, int scaleFactor ))
IPPAPI(IppStatus, ippsAutoCorr_NormB_16s_Sfs,( const Ipp16s* pSrc, int srcLen, Ipp16s* pDst,
                                                        int dstLen, int scaleFactor ))



/* /////////////////////////////////////////////////////////////////////////////
//                  Sampling functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsSampleUp
//  Purpose:    upsampling, i.e. expansion of input vector to get output vector
//              by simple adding zeroes between input elements
//  Parameters:
//   pSrc   (in)   pointer to the input vector
//   pDst   (in)   pointer to the output vector
//   srcLen (in)   length of input vector
//   dstLen (out)  pointer to the length of output vector
//   factor (in)   the number of output elements, corresponding to one element
//                 of input vector.
//   phase(in-out) pointer to value, that is the position (0, ..., factor-1) of
//                 element from input vector in the group of factor elements of
//                 output vector. Out value is ready to continue upsampling with
//                 the same factor (out = in).
//
//  Return:
//   ippStsNullPtrErr        one or several pointers pSrc, pDst, dstLen or phase
//                         is NULL
//   ippStsSizeErr           length of input vector is less or equal zero
//   ippStsSampleFactorErr   factor <= 0
//   ippStsSamplePhaseErr    *phase < 0 or *phase >= factor
//   ippStsNoErr             otherwise
*/
IPPAPI ( IppStatus, ippsSampleUp_32f, (const Ipp32f* pSrc, int  srcLen,
                                             Ipp32f* pDst, int* dstLen,
                                             int factor,   int* phase))
IPPAPI ( IppStatus, ippsSampleUp_32fc, (const Ipp32fc* pSrc, int  srcLen,
                                              Ipp32fc* pDst, int* dstLen,
                                              int factor,   int* phase))
IPPAPI ( IppStatus, ippsSampleUp_64f, (const Ipp64f* pSrc, int  srcLen,
                                             Ipp64f* pDst, int* dstLen,
                                             int factor,   int* phase))
IPPAPI ( IppStatus, ippsSampleUp_64fc, (const Ipp64fc* pSrc, int  srcLen,
                                              Ipp64fc* pDst, int* dstLen,
                                              int factor,   int* phase))
IPPAPI ( IppStatus, ippsSampleUp_16s, (const Ipp16s* pSrc, int  srcLen,
                                             Ipp16s* pDst, int* dstLen,
                                             int factor,   int* phase))
IPPAPI ( IppStatus, ippsSampleUp_16sc, (const Ipp16sc* pSrc, int  srcLen,
                                              Ipp16sc* pDst, int* dstLen,
                                              int factor,   int* phase))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsSampleDown
//  Purpose:    subsampling, i.e. only one of "factor" elements of input vector
//              are placed to output vector
//  Parameters:
//   pSrc   (in)   pointer to the input vector
//   pDst   (in)   pointer to the output vector
//   srcLen (in)   length of input vector
//   dstLen (out)  pointer to the length of output vector
//   factor (in)   the number of input elements, corresponding to one element
//                 of output vector.
//   phase(in-out) pointer to value, that is the position (0, ..., factor-1) of
//                 chosen element in the group of "factor" elements. Out value
//                 of *phase is ready to continue subsampling with the same
//                 factor.
//
//  Return:
//   ippStsNullPtrErr        one or several pointers pSrc, pDst, dstLen or phase
//                        is NULL
//   ippStsSizeErr           length of input vector is less or equal zero
//   ippStsSampleFactorErr   factor <= 0
//   ippStsSamplePhaseErr    *phase < 0 or *phase >=factor
//   ippStsNoErr             otherwise
*/
IPPAPI ( IppStatus, ippsSampleDown_32f, (const Ipp32f* pSrc, int  srcLen,
                                               Ipp32f* pDst, int* dstLen,
                                               int factor,   int* phase))
IPPAPI ( IppStatus, ippsSampleDown_32fc, (const Ipp32fc* pSrc, int  srcLen,
                                                Ipp32fc* pDst, int* dstLen,
                                                int factor,   int* phase))
IPPAPI ( IppStatus, ippsSampleDown_64f, (const Ipp64f* pSrc, int  srcLen,
                                               Ipp64f* pDst, int* dstLen,
                                               int factor,   int* phase))
IPPAPI ( IppStatus, ippsSampleDown_64fc, (const Ipp64fc* pSrc, int  srcLen,
                                                Ipp64fc* pDst, int* dstLen,
                                                int factor,   int* phase))
IPPAPI ( IppStatus, ippsSampleDown_16s, (const Ipp16s* pSrc, int  srcLen,
                                               Ipp16s* pDst, int* dstLen,
                                               int factor,   int* phase))
IPPAPI ( IppStatus, ippsSampleDown_16sc, (const Ipp16sc* pSrc, int  srcLen,
                                                Ipp16sc* pDst, int* dstLen,
                                                int factor,   int* phase))



/* ///////////////////////////////////////////////////////////////////////////
//  Names:      ippsGetVarPointDV_16sc
//  Purpose:    Fills the array VariantPoint with information about 8
//              (if State = 32,64) or 4 (if State = 16) closest to the
//              refPoint complex points (stores the indexes in the
//              offset table and errors between refPoint and the
//              current point)
//  Return:
//  ippStsNoErr         Ok
//  ippStsNullPtrErr    Any of the specified pointers is NULL
//  Parameters:
//  pSrc            pointer to the reference point in format 9:7
//  pDst            pointer to the closest to the reference point left
//                  and bottom comlexpoint in format 9:7
//  pVariantPoint   pointer to the array where the information is stored
//  pLabel          pointer to the labels table
//  state           number of states of the convolution coder
*/
IPPAPI(IppStatus,ippsGetVarPointDV_16sc,(const Ipp16sc *pSrc,Ipp16sc *pDst,
       Ipp16sc *pVariantPoint,const Ipp8u *pLabel,int state))


/* ///////////////////////////////////////////////////////////////////////////
//  Names:      ippsCalcStatesDV_16sc
//  Purpose:    Computes possible states of the Viterbi decoder
//  Return:
//  ippStsNoErr         OK
//  ippStsNullPtrErr    Any of the specified pointers is NULL
//  Parameters:
//  pPathError          pointer to the table of path error metrics
//  pNextState          pointer to the next state table
//  pBranchError        pointer to the branch error table
//  pCurrentSubsetPoint pointer to the current 4D subset
//  pPathTable          pointer to the Viterbi path table
//  state               number of states of the convolution coder
//  presentIndex        start index in Viterbi Path table
*/
IPPAPI(IppStatus,ippsCalcStatesDV_16sc,(const Ipp16u *pathError,
       const Ipp8u *pNextState, Ipp16u *pBranchError,
       const Ipp16s *pCurrentSubsetPoint, Ipp16s *pPathTable,
       int state,int presentIndex))

/* ///////////////////////////////////////////////////////////////////////////
//  Names:      ippsBuildSymblTableDV4D_16s
//  Purpose:    Fills the array with an information of possible 4D symbols
//  Return:
//  ippStsNoErr         OK
//  ippStsNullPtrErr    Any of the specified pointers is NULL
//  Parameters:
//  pVariantPoint       pointer to the array of possible 2D symbols
//  pCurrentSubsetPoint pointer to the current array of 4D symbols
//  state               number of states of the convolution coder
//  bitInversion        bit Inversion
*/
IPPAPI(IppStatus,ippsBuildSymblTableDV4D_16sc,(const Ipp16sc *pVariantPoint,
       Ipp16sc *pCurrentSubsetPoint,int state,int bitInversion ))

/* ///////////////////////////////////////////////////////////////////////////
//  Names:      ippsUpdatePathMetricsDV_16u
//  Purpose:    Searches for the minimum path metric and updates states of the decoder
//  Return:
//  ippStsNoErr         OK
//  ippStsNullPtrErr    Any of the specified pointers is NULL
//  Parameters:
//  pBranchError        pointer to the branch error table
//  pMinPathError       pointer to the current minimum path error metric
//  pMinSost            pointer to the state with minimum path metric
//  pPathError          pointer to table of path error metrics
//  state               number of states of the convolution coder
*/
IPPAPI(IppStatus,ippsUpdatePathMetricsDV_16u,(Ipp16u *pBranchError,
       Ipp16u *pMinPathError,Ipp8u *pMinSost,Ipp16u *pPathError,int state))


/* /////////////////////////////////////////////////////////////////////////////
//                  Definitions for Hilbert Functions
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )

struct HilbertSpec_32f32fc;
typedef struct HilbertSpec_32f32fc IppsHilbertSpec_32f32fc;

struct HilbertSpec_16s32fc;
typedef struct HilbertSpec_16s32fc IppsHilbertSpec_16s32fc;

struct HilbertSpec_16s16sc;
typedef struct HilbertSpec_16s16sc IppsHilbertSpec_16s16sc;

#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
//                  Hilbert Context Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsHilbertInitAlloc_32f32fc, ippsHilbertFree_32f32fc,
//              ippsHilbertInitAlloc_16s32fc, ippsHilbertFree_16s32fc,
//              ippsHilbertInitAlloc_16s16sc, ippsHilbertFree_16s16sc
//  Purpose:    create, initialize and delete Hilbert context
//  Arguments:
//     pSpec    - where write pointer to new context
//     length   - number of samples in Hilbert
//     hint     - code specific use hints (DFT)
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pSpec == NULL
//     ippStsSizeErr          bad the length value
//     ippStsContextMatchErr  bad context identifier
//     ippStsMemAllocErr      memory allocation error
*/

IPPAPI(IppStatus, ippsHilbertInitAlloc_32f32fc, (IppsHilbertSpec_32f32fc **pSpec,
                                                 int length, IppHintAlgorithm hint))
IPPAPI(IppStatus, ippsHilbertInitAlloc_16s32fc, (IppsHilbertSpec_16s32fc **pSpec,
                                                 int length, IppHintAlgorithm hint))
IPPAPI(IppStatus, ippsHilbertInitAlloc_16s16sc, (IppsHilbertSpec_16s16sc **pSpec,
                                                 int length, IppHintAlgorithm hint))
IPPAPI(IppStatus, ippsHilbertFree_32f32fc, (IppsHilbertSpec_32f32fc *pSpec))
IPPAPI(IppStatus, ippsHilbertFree_16s32fc, (IppsHilbertSpec_16s32fc *pSpec))
IPPAPI(IppStatus, ippsHilbertFree_16s16sc, (IppsHilbertSpec_16s16sc *pSpec))


/* /////////////////////////////////////////////////////////////////////////////
//                  Hilbert Transform Functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsHilbert_32f32fc,
//              ippsHilbert_16s32fc,
//              ippsHilbert_16s16sc_Sfs
//  Purpose:    compute Hilbert transform of the real signal
//  Arguments:
//     pSrc     - pointer to source real signal
//     pDst     - pointer to destination complex signal
//     pSpec    - pointer to Hilbert context
//     scaleFactor - scale factor for output signal
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pointer(s) to the data is NULL
//     ippStsContextMatchErr  bad context identifier
//     ippStsMemAllocErr      memory allocation error
*/

IPPAPI(IppStatus, ippsHilbert_32f32fc, (const Ipp32f *pSrc, Ipp32fc *pDst,
                                        IppsHilbertSpec_32f32fc *pSpec))
IPPAPI(IppStatus, ippsHilbert_16s32fc, (const Ipp16s *pSrc, Ipp32fc *pDst,
                                        IppsHilbertSpec_16s32fc *pSpec))
IPPAPI(IppStatus, ippsHilbert_16s16sc_Sfs, (const Ipp16s *pSrc, Ipp16sc *pDst,
                                        IppsHilbertSpec_16s16sc *pSpec, int scaleFactor))

#if !defined( _OWN_BLDPCS )
struct FIRSparseState_32f;
typedef struct FIRSparseState_32f IppsFIRSparseState_32f;

struct IIRSparseState_32f;
typedef struct IIRSparseState_32f IppsIIRSparseState_32f;

#endif /* _OWN_BLDPCS */


/* ////////////////////////////////////////////////////////////////////////////
//  Name:         ippsFIRSparseGetStateSize,
//                ippsFIRSparseInit
//  Purpose:      ippsFIRSparseGetStateSize - calculates the size of the FIRSparse
//                                            State  structure;
//                ippsFIRSparseInit - initialize FIRSparse state - set non-zero taps,
//                their positions and delay line using external memory buffer;
//  Parameters:
//      pNZTaps     - pointer to the non-zero filter coefficients;
//      pNZTapPos   - pointer to the positions of non-zero filter coefficients;
//      nzTapsLen   - number of non-zero coefficients;
//      pDlyLine    - pointer to the delay line values, can be NULL;
//      pState      - pointer to the FIRSparse state created or NULL;
//      order       - order of FIRSparse filter
//      pStateSize  - pointer where to store the calculated FIRSparse State
//                    structuresize (in bytes);
//   Return:
//      status      - status value returned, its value are
//         ippStsNullPtrErr       - pointer(s) to the data is NULL
//         ippStsFIRLenErr        - nzTapsLen <= 0
//         ippStsSparseErr        - non-zero tap positions are not in ascending order,
//                                  negative or repeated.
//         ippStsNoErr            - otherwise
*/

IPPAPI( IppStatus, ippsFIRSparseGetStateSize_32f,( int nzTapsLen,
    int order, int *pStateSize ))

IPPAPI( IppStatus, ippsFIRSparseInit_32f,( IppsFIRSparseState_32f** pState,
    const Ipp32f *pNZTaps, const Ipp32s* pNZTapPos, int nzTapsLen,
    const Ipp32f *pDlyLine, Ipp8u *pBuffer ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:         ippsIIRSparseGetStateSize,
//                ippsIIRSparseInit
//  Purpose:      ippsIIRSparseGetStateSize - calculates the size of the
//                                            IIRSparse State structure;
//                ippsIIRSparseInit - initialize IIRSparse state - set non-zero taps,
//                their positions and delay line using external memory buffer;
//  Parameters:
//      pNZTaps     - pointer to the non-zero filter coefficients;
//      pNZTapPos   - pointer to the positions of non-zero filter coefficients;
//      nzTapsLen1,
//      nzTapsLen2  - number of non-zero coefficients according to the IIRSparseformula;
//      pDlyLine    - pointer to the delay line values, can be NULL;
//      pState      - pointer to the IIR state created or NULL;
//      pStateSize  - pointer where to store the calculated IIR State structure
//                                                             size (in bytes);
//   Return:
//      status      - status value returned, its value are
//         ippStsNullPtrErr       - pointer(s) to the data is NULL
//         ippStsIIROrderErr      - nzTapsLen1 <= 0 or nzTapsLen2 < 0
//         ippStsSparseErr        - non-zero tap positions are not in ascending order,
//                                  negative or repeated.
//         ippStsNoErr            - otherwise
*/

IPPAPI( IppStatus, ippsIIRSparseGetStateSize_32f,( int nzTapsLen1, int nzTapsLen2,
        int order1, int order2, int *pStateSize ))

IPPAPI( IppStatus, ippsIIRSparseInit_32f,( IppsIIRSparseState_32f** pState,
        const Ipp32f* pNZTaps, const Ipp32s* pNZTapPos, int nzTapsLen1,
        int nzTapsLen2, const Ipp32f* pDlyLine, Ipp8u* pBuf ))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:         ippsFIRSparse
//  Purpose:       FIRSparse filter with float taps. Vector filtering
//  Parameters:
//      pSrc        - pointer to the input vector
//      pDst        - pointer to the output vector
//      len         - length data vector
//      pState      - pointer to the filter state
//  Return:
//      ippStsNullPtrErr       - pointer(s) to the data is NULL
//      ippStsSizeErr          - length of the vectors <= 0
//      ippStsNoErr            - otherwise
*/

IPPAPI(IppStatus, ippsFIRSparse_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int len,
        IppsFIRSparseState_32f* pState))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:         ippsIIRSparse
//  Purpose:       IIRSparse filter with float taps. Vector filtering
//  Parameters:
//      pSrc                - pointer to input vector
//      pDst                - pointer to output vector
//      len                 - length of the vectors
//      pState              - pointer to the filter state
//  Return:
//      ippStsNullPtrErr       - pointer(s) to the data is NULL
//      ippStsSizeErr          - length of the vectors <= 0
//      ippStsNoErr            - otherwise
*/

IPPAPI(IppStatus, ippsIIRSparse_32f, (const Ipp32f* pSrc, Ipp32f* pDst, int len,
       IppsIIRSparseState_32f* pState))



/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsAddProductC
//  Purpose:    multiplies elements of of a vector by a constant and adds product to
//              the accumulator vector
//  Parameters:
//    pSrc                 pointer to the source vector
//    val                  constant value
//    pSrcDst              pointer to the source/destination (accumulator) vector
//    len                  length of the vectors
//  Return:
//    ippStsNullPtrErr     pointer to the vector is NULL
//    ippStsSizeErr        length of the vectors is less or equal zero
//    ippStsNoErr          otherwise
//
//  Notes:                 pSrcDst[n] = pSrcDst[n] + pSrc[n] * val, n=0,1,2,..len-1.
*/

IPPAPI(IppStatus, ippsAddProductC_32f,       ( const Ipp32f* pSrc, const Ipp32f val,
                                               Ipp32f* pSrcDst, int len ))
/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippsSumWindow_8u32f      ippsSumWindow_16s32f
//  Purpose:
//  Return:
//    ippStsNoErr        Ok
//    ippStsNullPtrErr   one or more pointers are NULL
//    ippStsMaskSizeErr  maskSize has a field with zero, or negative value
//  Arguments:
//   pSrc        Pointer to the source vector
//   pDst        Pointer to the destination vector
//   maskSize    Size of the mask in pixels
*/
IPPAPI(IppStatus,ippsSumWindow_8u32f ,(const Ipp8u*  pSrc,Ipp32f* pDst,int len, int maskSize ))
IPPAPI(IppStatus,ippsSumWindow_16s32f,(const Ipp16s* pSrc,Ipp32f* pDst,int len, int maskSize ))

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:  ippsCountInRange_32s
//  Purpose: Computes the number of vector elements falling within the specified range.
//  Return:
//     ippStsNoErr       No errors, it's OK
//     ippStsNullPtrErr  Either pSrc or pCounts equals to zero.
//     ippStsLengthErr   The vector's length is less than or equals to zero.
//  Arguments:
//     pSrc              A pointer to the source vector.
//     len               Number of the vector elements.
//     pCounts           A pointer to the output result.
//     lowerBound        The upper boundary of the range.
//     uppreBound        The lower boundary of the range.
*/
IPPAPI( IppStatus, ippsCountInRange_32s, ( const Ipp32s* pSrc, int len, int* pCounts,
       Ipp32s lowerBound, Ipp32s upperBound ) )


/*
//  Purpose:            Creates ramp vector
//
//  Parameters:
//    pDst              A pointer to the destination vector
//    len               Vector's length
//    offset            Offset value
//    slope             Slope coefficient
//
//  Return:
//    ippStsNoErr       No error
//    ippStsNullPtrErr  pDst pointer is NULL
//    ippStsBadSizeErr  Vector's length is less or equal zero
//    ippStsNoErr       No error
//
//  Notes:              Dst[n] = offset + slope * n
//
*/

IPPAPI(IppStatus,  ippsVectorSlope_8u, ( Ipp8u*  pDst, int len, Ipp32f offset, Ipp32f slope ))
IPPAPI(IppStatus,  ippsVectorSlope_8s, ( Ipp8s*  pDst, int len, Ipp32f offset, Ipp32f slope ))
IPPAPI(IppStatus, ippsVectorSlope_16u, ( Ipp16u* pDst, int len, Ipp32f offset, Ipp32f slope ))
IPPAPI(IppStatus, ippsVectorSlope_16s, ( Ipp16s* pDst, int len, Ipp32f offset, Ipp32f slope ))
IPPAPI(IppStatus, ippsVectorSlope_32u, ( Ipp32u* pDst, int len, Ipp64f offset, Ipp64f slope ))
IPPAPI(IppStatus, ippsVectorSlope_32s, ( Ipp32s* pDst, int len, Ipp64f offset, Ipp64f slope ))
IPPAPI(IppStatus, ippsVectorSlope_32f, ( Ipp32f* pDst, int len, Ipp32f offset, Ipp32f slope ))
IPPAPI(IppStatus, ippsVectorSlope_64f, ( Ipp64f* pDst, int len, Ipp64f offset, Ipp64f slope ))

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDiv_Round_8u_Sfs
//              ippsDiv_Round_16u_Sfs
//              ippsDiv_Round_16s_Sfs
//              ippsDiv_Round_8u_ISfs
//              ippsDiv_Round_16u_ISfs
//              ippsDiv_Round_16s_ISfs
//
//
//  Purpose:    divide every element of the source vector by the scalar value
//              or by corresponding element of the second source vector and
//              round to zero, near or financial
//  Arguments:
//    val               the divisor value
//    pSrc              pointer to the divisor source vector
//    pSrc1             pointer to the divisor source vector
//    pSrc2             pointer to the dividend source vector
//    pDst              pointer to the destination vector
//    pSrcDst           pointer to the source/destination vector
//    len               vector's length, number of items
//    rndMode           Rounding mode (ippRndZero, ippRndNear or ippRndFinancial)
//    scaleFactor       scale factor parameter value
//  Return:
//    ippStsNullPtrErr     pointer(s) to the data vector is NULL
//    ippStsSizeErr        length of the vector is less or equal zero
//    ippStsDivByZeroErr   the scalar divisor value is zero
//    ippStsDivByZero      Warning status if an element of divisor vector is
//                      zero. If the dividend is zero than result is
//                      NaN, if the dividend is not zero than result
//                      is Infinity with correspondent sign. The
//                      execution is not aborted. For the integer operation
//                      zero instead of NaN and the corresponding bound
//                      values instead of Infinity
//   ippStsRoundModeNotSupportedErr Unsupported round mode
//   ippStsNoErr          otherwise

//  Note:
//    DivC(v,X,Y)  :    Y[n] = X[n] / v
//    DivC(v,X)    :    X[n] = X[n] / v
//    Div(X,Y)     :    Y[n] = Y[n] / X[n]
//    Div(X,Y,Z)   :    Z[n] = Y[n] / X[n]
*/
IPPAPI(IppStatus, ippsDiv_Round_8u_Sfs, (const Ipp8u* pSrc1, const Ipp8u* pSrc2,
       Ipp8u* pDst, int len, IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippsDiv_Round_16s_Sfs, (const Ipp16s* pSrc1, const Ipp16s* pSrc2,
       Ipp16s* pDst, int len, IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippsDiv_Round_16u_Sfs, (const Ipp16u* pSrc1, const Ipp16u* pSrc2,
       Ipp16u* pDst, int len, IppRoundMode rndMode, int scaleFactor))

IPPAPI(IppStatus, ippsDiv_Round_8u_ISfs, (const Ipp8u* pSrc, Ipp8u* pSrcDst,
       int len, IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippsDiv_Round_16s_ISfs, (const Ipp16s* pSrc, Ipp16s* pSrcDst,
       int len, IppRoundMode rndMode, int scaleFactor))
IPPAPI(IppStatus, ippsDiv_Round_16u_ISfs, (const Ipp16u* pSrc, Ipp16u* pSrcDst,
       int len, IppRoundMode rndMode, int scaleFactor))

/* /////////////////////////////////////////////////////////////////////////////
//  Names:      ippsDemodulateFM_CToR_16s
//  Purpose:    FM demodulate
//  Return:
//   IPP_NO_ERR                 Ok
//   IPP_NULL_PTR_ERR           Some of pointers are NULL
//   IPP_BADSIZE_ERR            The length of the array is less or equal zero
//  Parameters:
//   pSrcRe     pointer to input vector of Real data
//   pSrcIm     pointer to input vector of Image data
//   pDst       pointer to output vector
//   len        a length of the array
//   pDlyPoint  pointer to earlier data
*/
IPPAPI(IppStatus, ippsDemodulateFM_CToR_16s,( const Ipp16s* pSrcRe,
       const Ipp16s *pSrcIm, Ipp16s *pDst, int len, Ipp16sc *pDlyPoint ))



/* ///////////////////////////////////////////////////////////////////////////
//  Name:    ippsCauchy_32f_I, ippsCauchyD_32f_I, ippsCauchyDD2_32f_I
//
//  Purpose: Compute Cauchy robust error function or its 1st, 2nd derivatives
//
//  Return:
//    ippStsNoErr         Ok
//    ippStsNullPtrErr    Some of the pointers are NULL's
//    ippStsSizeErr       Length is negative
//    ippStsOutOfRangeErr C is negative
//
//  Arguments:
//    pSrcDst          The input/output array of function arguments/values.
//    pD2F             The array of values of the 2nd derivative.
//    len              Length of srcdst and d2fval
//    C                Cauchy function parameter
*/

IPPAPI(IppStatus, ippsCauchy_32f_I, (Ipp32f* pSrcDst, int len, Ipp32f C))
IPPAPI(IppStatus, ippsCauchyD_32f_I, (Ipp32f* pSrcDst, int len, Ipp32f C))
IPPAPI(IppStatus, ippsCauchyDD2_32f_I, (Ipp32f* pSrcDst, Ipp32f* pD2F, int len, Ipp32f C))

/* ///////////////////////////////////////////////////////////////////////////
//  Name:              ippsZeroCrossing_*
//  Purpose:           Counts the zero-cross measure for the input signal.
//
//  Parameters:
//    pSrc             Pointer to the input signal [len].
//    len              Number of elements in the input signal.
//    pValZCR          Pointer to the result value.
//    zcType           Zero crossing measure type.
//  Return:
//    ippStsNoErr      Indicates no error.
//    ippStsNullPtrErr Indicates an error when the pSrc or pRes pointer is null.
//    ippStsRangeErr   Indicates an error when zcType is not equal to 
//                     ippZCR, ippZCXor or ippZCC  
*/

IPPAPI(IppStatus, ippsZeroCrossing_16s32f,(const Ipp16s* pSrc, Ipp32u len,
       Ipp32f* pValZCR, IppsZCType zcType))
IPPAPI(IppStatus, ippsZeroCrossing_32f,   (const Ipp32f* pSrc, Ipp32u len,
       Ipp32f* pValZCR, IppsZCType zcType))


/* /////////////////////////////////////////////////////////////////////////////
//     The library was generated by Spiral, developed at Carnegie Mellon
//     University, http://www.spiral.net
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  Name:       ippgenGetLibVersion
//  Purpose:    getting of the library version
//  Returns:    the structure of information about version of ippGEN library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
IPPAPI( const IppLibraryVersion*, ippgenGetLibVersion, (void) )

#if !defined( _OWN_BLDPCS )
  typedef void ippgDCT4Spec_32f;
  typedef void ippgDCT4Spec_64f;
#endif /* _OWN_BLDPCS */

IPPAPI(IppStatus, ippgDCT4Free_64f, (ippgDCT4Spec_64f *spec) )
IPPAPI(IppStatus, ippgDCT4GetSize_64f, (int len, int *pSize))
IPPAPI(IppStatus, ippgDCT4Init_64f, (ippgDCT4Spec_64f **pSpec, int len, Ipp8u *pMem) )
IPPAPI(IppStatus, ippgDCT4InitAlloc_64f, (ippgDCT4Spec_64f **pSpec, int len) )
IPPAPI(IppStatus, ippgDCT4_64f, (const Ipp64f *pSrc, Ipp64f *pDst, const ippgDCT4Spec_64f *spec) )

IPPAPI(IppStatus, ippgDCT4Free_32f, (ippgDCT4Spec_32f *spec) )
IPPAPI(IppStatus, ippgDCT4GetSize_32f, (int len, int *pSize))
IPPAPI(IppStatus, ippgDCT4Init_32f, (ippgDCT4Spec_32f **pSpec, int len, Ipp8u *pMem) )
IPPAPI(IppStatus, ippgDCT4InitAlloc_32f, (ippgDCT4Spec_32f **pSpec, int len) )
IPPAPI(IppStatus, ippgDCT4_32f, (const Ipp32f *pSrc, Ipp32f *pDst, const ippgDCT4Spec_32f *spec) )


/* /////////////////////////////////////////////////////////////////////////////
//              Walsh Hadamard transforms of fixed size
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippgWHT
//  Purpose:    compute Walsh Hadamard transform of a signal
//  Note:       Walsh Hadamard transform is involutary (self-inverse)
//  Normalization choices: IPP_NODIV_BY_ANY, IPP_DIV_BY_SQRTN,
//                         IPP_DIV_FWD_BY_N is same as IPP_DIV_INV_BY_N

//  Arguments:
//     pSrc     - pointer to source complex signal
//     pDst     - pointer to destination complex signal
//     order    - number of samples in the input signal pSrc is 2^order
//                valid values for order are [ 1 .. 13 ]
//     flag     - normalization flag
//
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pSrc == NULL or pDst == NULL or
//     ippStsFftFlagErr       bad the normalization flag value
*/
IPPAPI(IppStatus, ippgWHTGetBufferSize_32f, (int order, Ipp32u *pBufferSize) )
IPPAPI(IppStatus, ippgWHTGetBufferSize_64f, (int order, Ipp32u *pBufferSize) )

IPPAPI(IppStatus, ippgWHT_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int order, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int order, int flag, Ipp8u *pBuffer) )

IPPAPI(IppStatus, ippgWHT_1_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_1_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag, Ipp8u *pBuffer) )

IPPAPI(IppStatus, ippgWHT_2_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_2_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag, Ipp8u *pBuffer) )

IPPAPI(IppStatus, ippgWHT_3_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_3_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag, Ipp8u *pBuffer) )

IPPAPI(IppStatus, ippgWHT_4_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_4_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag, Ipp8u *pBuffer) )

IPPAPI(IppStatus, ippgWHT_5_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_5_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag, Ipp8u *pBuffer) )

IPPAPI(IppStatus, ippgWHT_6_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_6_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag, Ipp8u *pBuffer) )

IPPAPI(IppStatus, ippgWHT_7_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_7_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag, Ipp8u *pBuffer) )

IPPAPI(IppStatus, ippgWHT_8_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_8_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag, Ipp8u *pBuffer) )

IPPAPI(IppStatus, ippgWHT_9_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_9_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag, Ipp8u *pBuffer) )

IPPAPI(IppStatus, ippgWHT_10_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_10_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag, Ipp8u *pBuffer) )

IPPAPI(IppStatus, ippgWHT_11_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_11_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag, Ipp8u *pBuffer) )

IPPAPI(IppStatus, ippgWHT_12_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_12_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag, Ipp8u *pBuffer) )

IPPAPI(IppStatus, ippgWHT_13_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag, Ipp8u *pBuffer) )
IPPAPI(IppStatus, ippgWHT_13_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag, Ipp8u *pBuffer) )

/* /////////////////////////////////////////////////////////////////////////////
//               DFT Complex Transforms of fixed size
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippgDFTFwd_CToC, ippsDFTInv_CToC
//  Purpose:    compute forward and inverse DFT of a complex signal
//  Normalization choices: IPP_NODIV_BY_ANY, IPP_DIV_BY_SQRTN,
//                         IPP_DIV_FWD_BY_N, IPP_DIV_INV_BY_N

//  Arguments:
//     pSrc     - pointer to source complex signal
//     pDst     - pointer to destination complex signal
//     length   - number of samples in the input signal pSrc
//                valid values for length are [ 2 .. 64 ]
//     flag     - normalization flag
//
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pSrc == NULL or pDst == NULL or
//     ippStsFftFlagErr       bad the normalization flag value
*/
IPPAPI(IppStatus, ippgDFTFwd_CToC_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int length, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_2_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_2_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_2_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_2_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_3_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_3_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_3_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_3_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_4_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_4_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_4_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_4_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_5_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_5_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_5_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_5_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_6_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_6_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_6_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_6_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_7_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_7_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_7_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_7_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_8_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_8_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_8_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_8_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_9_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_9_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_9_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_9_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_10_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_10_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_10_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_10_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_11_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_11_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_11_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_11_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_12_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_12_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_12_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_12_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_13_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_13_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_13_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_13_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_14_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_14_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_14_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_14_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_15_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_15_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_15_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_15_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_16_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_16_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_16_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_16_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_17_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_17_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_17_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_17_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_18_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_18_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_18_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_18_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_19_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_19_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_19_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_19_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_20_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_20_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_20_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_20_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_21_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_21_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_21_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_21_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_22_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_22_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_22_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_22_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_23_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_23_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_23_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_23_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_24_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_24_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_24_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_24_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_25_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_25_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_25_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_25_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_26_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_26_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_26_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_26_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_27_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_27_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_27_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_27_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_28_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_28_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_28_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_28_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_29_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_29_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_29_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_29_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_30_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_30_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_30_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_30_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_31_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_31_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_31_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_31_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_32_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_32_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_32_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_32_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_33_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_33_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_33_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_33_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_34_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_34_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_34_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_34_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_35_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_35_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_35_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_35_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_36_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_36_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_36_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_36_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_37_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_37_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_37_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_37_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_38_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_38_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_38_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_38_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_39_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_39_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_39_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_39_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_40_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_40_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_40_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_40_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_41_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_41_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_41_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_41_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_42_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_42_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_42_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_42_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_43_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_43_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_43_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_43_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_44_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_44_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_44_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_44_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_45_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_45_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_45_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_45_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_46_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_46_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_46_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_46_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_47_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_47_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_47_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_47_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_48_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_48_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_48_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_48_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_49_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_49_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_49_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_49_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_50_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_50_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_50_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_50_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_51_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_51_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_51_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_51_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_52_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_52_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_52_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_52_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_53_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_53_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_53_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_53_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_54_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_54_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_54_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_54_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_55_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_55_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_55_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_55_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_56_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_56_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_56_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_56_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_57_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_57_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_57_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_57_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_58_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_58_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_58_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_58_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_59_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_59_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_59_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_59_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_60_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_60_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_60_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_60_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_61_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_61_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_61_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_61_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_62_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_62_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_62_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_62_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_63_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_63_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_63_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_63_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_CToC_64_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_CToC_64_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_64_32fc, (const Ipp32fc *pSrc, Ipp32fc *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CToC_64_64fc, (const Ipp64fc *pSrc, Ipp64fc *pDst, int flag) )

/* /////////////////////////////////////////////////////////////////////////////
//               DFT Real Transforms of fixed size
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippgDFTFwd_RToCCS, ippgDFTFwd_RToPack, ippgDFTFwd_RToPerm,
//              ippsDFTInv_CCSToR, ippgDFTInv_PackToR, ippgDFTInv_PermToR,
//  Purpose:    compute forward and inverse DFT of a real signal
//  Normalization choices: IPP_NODIV_BY_ANY, IPP_DIV_BY_SQRTN,
//                         IPP_DIV_FWD_BY_N, IPP_DIV_INV_BY_N

//  Arguments:
//     pSrc     - pointer to source complex signal
//     pDst     - pointer to destination complex signal
//     length   - number of samples in the input signal pSrc
//                valid values for length are [ 2 .. 64 ]
//     flag     - normalization flag
//
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pSrc == NULL or pDst == NULL or
//     ippStsFftFlagErr       bad the normalization flag value
*/
IPPAPI(IppStatus, ippgDFTFwd_RToPack_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int length, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int length, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_2_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_2_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_2_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_2_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_2_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_2_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_2_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_2_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_2_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_2_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_2_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_2_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_3_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_3_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_3_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_3_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_3_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_3_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_3_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_3_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_3_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_3_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_3_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_3_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_4_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_4_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_4_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_4_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_4_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_4_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_4_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_4_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_4_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_4_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_4_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_4_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_5_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_5_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_5_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_5_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_5_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_5_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_5_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_5_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_5_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_5_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_5_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_5_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_6_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_6_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_6_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_6_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_6_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_6_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_6_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_6_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_6_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_6_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_6_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_6_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_7_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_7_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_7_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_7_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_7_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_7_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_7_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_7_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_7_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_7_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_7_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_7_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_8_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_8_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_8_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_8_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_8_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_8_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_8_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_8_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_8_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_8_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_8_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_8_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_9_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_9_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_9_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_9_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_9_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_9_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_9_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_9_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_9_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_9_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_9_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_9_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_10_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_10_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_10_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_10_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_10_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_10_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_10_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_10_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_10_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_10_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_10_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_10_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_11_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_11_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_11_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_11_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_11_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_11_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_11_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_11_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_11_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_11_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_11_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_11_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_12_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_12_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_12_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_12_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_12_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_12_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_12_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_12_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_12_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_12_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_12_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_12_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_13_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_13_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_13_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_13_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_13_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_13_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_13_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_13_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_13_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_13_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_13_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_13_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_14_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_14_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_14_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_14_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_14_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_14_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_14_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_14_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_14_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_14_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_14_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_14_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_15_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_15_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_15_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_15_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_15_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_15_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_15_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_15_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_15_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_15_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_15_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_15_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_16_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_16_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_16_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_16_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_16_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_16_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_16_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_16_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_16_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_16_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_16_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_16_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_17_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_17_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_17_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_17_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_17_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_17_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_17_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_17_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_17_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_17_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_17_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_17_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_18_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_18_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_18_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_18_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_18_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_18_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_18_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_18_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_18_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_18_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_18_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_18_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_19_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_19_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_19_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_19_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_19_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_19_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_19_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_19_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_19_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_19_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_19_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_19_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_20_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_20_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_20_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_20_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_20_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_20_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_20_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_20_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_20_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_20_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_20_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_20_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_21_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_21_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_21_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_21_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_21_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_21_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_21_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_21_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_21_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_21_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_21_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_21_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_22_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_22_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_22_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_22_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_22_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_22_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_22_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_22_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_22_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_22_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_22_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_22_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_23_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_23_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_23_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_23_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_23_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_23_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_23_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_23_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_23_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_23_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_23_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_23_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_24_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_24_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_24_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_24_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_24_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_24_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_24_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_24_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_24_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_24_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_24_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_24_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_25_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_25_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_25_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_25_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_25_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_25_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_25_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_25_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_25_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_25_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_25_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_25_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_26_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_26_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_26_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_26_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_26_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_26_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_26_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_26_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_26_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_26_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_26_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_26_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_27_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_27_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_27_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_27_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_27_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_27_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_27_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_27_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_27_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_27_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_27_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_27_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_28_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_28_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_28_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_28_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_28_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_28_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_28_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_28_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_28_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_28_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_28_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_28_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_29_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_29_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_29_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_29_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_29_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_29_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_29_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_29_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_29_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_29_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_29_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_29_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_30_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_30_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_30_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_30_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_30_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_30_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_30_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_30_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_30_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_30_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_30_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_30_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_31_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_31_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_31_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_31_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_31_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_31_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_31_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_31_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_31_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_31_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_31_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_31_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_32_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_32_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_32_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_32_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_32_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_32_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_32_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_32_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_32_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_32_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_32_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_32_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_33_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_33_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_33_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_33_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_33_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_33_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_33_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_33_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_33_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_33_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_33_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_33_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_34_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_34_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_34_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_34_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_34_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_34_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_34_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_34_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_34_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_34_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_34_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_34_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_35_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_35_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_35_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_35_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_35_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_35_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_35_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_35_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_35_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_35_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_35_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_35_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_36_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_36_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_36_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_36_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_36_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_36_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_36_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_36_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_36_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_36_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_36_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_36_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_37_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_37_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_37_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_37_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_37_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_37_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_37_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_37_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_37_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_37_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_37_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_37_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_38_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_38_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_38_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_38_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_38_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_38_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_38_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_38_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_38_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_38_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_38_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_38_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_39_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_39_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_39_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_39_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_39_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_39_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_39_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_39_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_39_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_39_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_39_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_39_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_40_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_40_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_40_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_40_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_40_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_40_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_40_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_40_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_40_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_40_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_40_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_40_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_41_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_41_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_41_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_41_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_41_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_41_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_41_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_41_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_41_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_41_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_41_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_41_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_42_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_42_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_42_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_42_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_42_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_42_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_42_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_42_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_42_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_42_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_42_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_42_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_43_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_43_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_43_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_43_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_43_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_43_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_43_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_43_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_43_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_43_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_43_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_43_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_44_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_44_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_44_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_44_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_44_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_44_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_44_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_44_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_44_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_44_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_44_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_44_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_45_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_45_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_45_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_45_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_45_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_45_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_45_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_45_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_45_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_45_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_45_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_45_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_46_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_46_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_46_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_46_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_46_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_46_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_46_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_46_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_46_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_46_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_46_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_46_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_47_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_47_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_47_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_47_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_47_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_47_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_47_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_47_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_47_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_47_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_47_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_47_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_48_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_48_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_48_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_48_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_48_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_48_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_48_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_48_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_48_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_48_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_48_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_48_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_49_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_49_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_49_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_49_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_49_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_49_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_49_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_49_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_49_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_49_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_49_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_49_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_50_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_50_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_50_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_50_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_50_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_50_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_50_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_50_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_50_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_50_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_50_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_50_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_51_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_51_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_51_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_51_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_51_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_51_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_51_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_51_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_51_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_51_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_51_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_51_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_52_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_52_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_52_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_52_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_52_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_52_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_52_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_52_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_52_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_52_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_52_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_52_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_53_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_53_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_53_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_53_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_53_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_53_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_53_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_53_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_53_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_53_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_53_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_53_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_54_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_54_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_54_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_54_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_54_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_54_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_54_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_54_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_54_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_54_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_54_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_54_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_55_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_55_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_55_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_55_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_55_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_55_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_55_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_55_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_55_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_55_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_55_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_55_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_56_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_56_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_56_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_56_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_56_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_56_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_56_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_56_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_56_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_56_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_56_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_56_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_57_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_57_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_57_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_57_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_57_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_57_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_57_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_57_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_57_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_57_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_57_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_57_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_58_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_58_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_58_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_58_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_58_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_58_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_58_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_58_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_58_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_58_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_58_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_58_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_59_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_59_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_59_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_59_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_59_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_59_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_59_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_59_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_59_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_59_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_59_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_59_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_60_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_60_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_60_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_60_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_60_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_60_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_60_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_60_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_60_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_60_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_60_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_60_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_61_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_61_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_61_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_61_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_61_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_61_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_61_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_61_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_61_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_61_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_61_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_61_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_62_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_62_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_62_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_62_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_62_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_62_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_62_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_62_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_62_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_62_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_62_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_62_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_63_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_63_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_63_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_63_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_63_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_63_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_63_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_63_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_63_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_63_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_63_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_63_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTFwd_RToPack_64_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPack_64_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_64_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToPerm_64_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_64_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTFwd_RToCCS_64_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgDFTInv_PackToR_64_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PackToR_64_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_64_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_PermToR_64_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_64_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgDFTInv_CCSToR_64_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

/* /////////////////////////////////////////////////////////////////////////////
//              Discrete Hartley Transforms of fixed size
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippgHartley
//  Purpose:    compute Hartley transform of a real signal
//  Note:       Hartley transform is involutary (self-inverse)
//  Normalization choices: IPP_NODIV_BY_ANY, IPP_DIV_BY_SQRTN,
//                         IPP_DIV_FWD_BY_N is same as IPP_DIV_INV_BY_N

//  Arguments:
//     pSrc     - pointer to source complex signal
//     pDst     - pointer to destination complex signal
//     length   - number of samples in the input signal pSrc
//                valid values for length are [ 2 .. 64 ]
//     flag     - normalization flag
//
//  Return:
//     ippStsNoErr            no errors
//     ippStsNullPtrErr       pSrc == NULL or pDst == NULL or
//     ippStsFftFlagErr       bad the normalization flag value
*/
IPPAPI(IppStatus, ippgHartley_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int length, int flag) )
IPPAPI(IppStatus, ippgHartley_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int length, int flag) )

IPPAPI(IppStatus, ippgHartley_2_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_2_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_3_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_3_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_4_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_4_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_5_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_5_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_6_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_6_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_7_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_7_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_8_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_8_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_9_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_9_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_10_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_10_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_11_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_11_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_12_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_12_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_13_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_13_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_14_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_14_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_15_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_15_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_16_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_16_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_17_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_17_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_18_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_18_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_19_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_19_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_20_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_20_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_21_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_21_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_22_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_22_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_23_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_23_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_24_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_24_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_25_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_25_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_26_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_26_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_27_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_27_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_28_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_28_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_29_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_29_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_30_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_30_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_31_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_31_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_32_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_32_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_33_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_33_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_34_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_34_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_35_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_35_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_36_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_36_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_37_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_37_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_38_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_38_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_39_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_39_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_40_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_40_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_41_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_41_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_42_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_42_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_43_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_43_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_44_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_44_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_45_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_45_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_46_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_46_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_47_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_47_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_48_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_48_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_49_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_49_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_50_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_50_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_51_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_51_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_52_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_52_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_53_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_53_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_54_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_54_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_55_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_55_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_56_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_56_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_57_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_57_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_58_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_58_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_59_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_59_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_60_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_60_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_61_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_61_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_62_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_62_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_63_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_63_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )

IPPAPI(IppStatus, ippgHartley_64_32f, (const Ipp32f *pSrc, Ipp32f *pDst, int flag) )
IPPAPI(IppStatus, ippgHartley_64_64f, (const Ipp64f *pSrc, Ipp64f *pDst, int flag) )


#if defined (_IPP_STDCALL_CDECL)
  #undef  _IPP_STDCALL_CDECL
  #define __stdcall __cdecl
#endif


#ifdef __cplusplus
}
#endif

#endif /* __IPPS_H__ */
/* ////////////////////////// End of file "ipps.h" ////////////////////////// */
