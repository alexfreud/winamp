/* ////////////////////////////////// "ippm.h" /////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2001-2009 Intel Corporation. All Rights Reserved.
//
//                  Intel(R) Performance Primitives
//                  Small Matrices Processing(ippMX)
//
*/

#if !defined( __IPPM_H__ ) || defined( _OWN_BLDPCS )
#define __IPPM_H__

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

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmGetLibVersion
//  Purpose:    get the library version
//  Returns:    the structure of information about version
//              of ippm library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/

IPPAPI(const IppLibraryVersion*, ippmGetLibVersion,(void) )

/* /////////////////////////////////////////////////////////////////////////////
//                   Utility functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmExtract
//  Purpose:    performs ROI extraction
//  Parameters:
//    pSrc, ppSrc       pointer to the source object or array of objects
//    srcStride0        stride between the objects in the source array
//    srcStride1        stride between the rows in the source matrix
//    srcStride2        stride between the elements in the source object
//    srcRoiShift       ROI shift for the source object
//    pDst              pointer to the destination array
//    len               vector length
//    width             matrix width
//    height            matrix height
//    count             number of objects in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmExtract_v_32f,   (const Ipp32f*  pSrc,  int srcStride2,
                                              Ipp32f*  pDst,  int len))
IPPAPI(IppStatus, ippmExtract_v_64f,   (const Ipp64f*  pSrc,  int srcStride2,
                                              Ipp64f*  pDst,  int len))

IPPAPI(IppStatus, ippmExtract_v_32f_P, (const Ipp32f** ppSrc, int srcRoiShift,
                                              Ipp32f*  pDst,  int len))
IPPAPI(IppStatus, ippmExtract_v_64f_P, (const Ipp64f** ppSrc, int srcRoiShift,
                                              Ipp64f*  pDst,  int len))

IPPAPI(IppStatus, ippmExtract_va_32f,  (const Ipp32f*  pSrc,  int srcStride0, int srcStride2,
                                              Ipp32f*  pDst,  int len, int count))
IPPAPI(IppStatus, ippmExtract_va_64f,  (const Ipp64f*  pSrc,  int srcStride0, int srcStride2,
                                              Ipp64f*  pDst,  int len, int count))

IPPAPI(IppStatus, ippmExtract_va_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                              Ipp32f*  pDst,  int len, int count))
IPPAPI(IppStatus, ippmExtract_va_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                              Ipp64f*  pDst,  int len, int count))

IPPAPI(IppStatus, ippmExtract_va_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride2,
                                              Ipp32f*  pDst,  int len, int count))
IPPAPI(IppStatus, ippmExtract_va_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride2,
                                              Ipp64f*  pDst,  int len, int count))

IPPAPI(IppStatus, ippmExtract_m_32f,   (const Ipp32f*  pSrc,  int srcStride1, int srcStride2,
                                              Ipp32f*  pDst,  int width, int height))
IPPAPI(IppStatus, ippmExtract_m_64f,   (const Ipp64f*  pSrc,  int srcStride1, int srcStride2,
                                              Ipp64f*  pDst,  int width, int height))

IPPAPI(IppStatus, ippmExtract_m_32f_P, (const Ipp32f** ppSrc, int srcRoiShift,
                                              Ipp32f*  pDst,  int width, int height))
IPPAPI(IppStatus, ippmExtract_m_64f_P, (const Ipp64f** ppSrc, int srcRoiShift,
                                              Ipp64f*  pDst,  int width, int height))

IPPAPI(IppStatus, ippmExtract_t_32f,   (const Ipp32f*  pSrc,  int srcStride1, int srcStride2,
                                              Ipp32f*  pDst,  int width, int height))
IPPAPI(IppStatus, ippmExtract_t_64f,   (const Ipp64f*  pSrc,  int srcStride1, int srcStride2,
                                              Ipp64f*  pDst,  int width, int height))

IPPAPI(IppStatus, ippmExtract_t_32f_P, (const Ipp32f** ppSrc, int srcRoiShift,
                                              Ipp32f*  pDst,  int width, int height))
IPPAPI(IppStatus, ippmExtract_t_64f_P, (const Ipp64f** ppSrc, int srcRoiShift,
                                              Ipp64f*  pDst,  int width, int height))

IPPAPI(IppStatus, ippmExtract_ma_32f,  (const Ipp32f*  pSrc,  int srcStride0, int srcStride1, int srcStride2,
                                              Ipp32f*  pDst,  int width, int height, int count))
IPPAPI(IppStatus, ippmExtract_ma_64f,  (const Ipp64f*  pSrc,  int srcStride0, int srcStride1, int srcStride2,
                                              Ipp64f*  pDst,  int width, int height, int count))

IPPAPI(IppStatus, ippmExtract_ma_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                              Ipp32f*  pDst,  int width, int height, int count))
IPPAPI(IppStatus, ippmExtract_ma_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                              Ipp64f*  pDst,  int width, int height, int count))

IPPAPI(IppStatus, ippmExtract_ma_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                              Ipp32f*  pDst,  int width, int height, int count))
IPPAPI(IppStatus, ippmExtract_ma_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                              Ipp64f*  pDst,  int width, int height, int count))

IPPAPI(IppStatus, ippmExtract_ta_32f,  (const Ipp32f*  pSrc, int srcStride0, int srcStride1, int srcStride2,
                                              Ipp32f*  pDst, int width, int height, int count))
IPPAPI(IppStatus, ippmExtract_ta_64f,  (const Ipp64f*  pSrc, int srcStride0, int srcStride1, int srcStride2,
                                              Ipp64f*  pDst, int width, int height, int count))

IPPAPI(IppStatus, ippmExtract_ta_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                              Ipp32f*  pDst,  int width, int height, int count))
IPPAPI(IppStatus, ippmExtract_ta_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                              Ipp64f*  pDst,  int width, int height, int count))

IPPAPI(IppStatus, ippmExtract_ta_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                              Ipp32f*  pDst,  int width, int height, int count))
IPPAPI(IppStatus, ippmExtract_ta_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                              Ipp64f*  pDst,  int width, int height, int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmCopy
//  Purpose:    copy data from object of any type to another object of any type
//  Parameters:
//    pSrc, ppSrc       pointer to the source object or array of objects
//    srcStride0        stride between the objects in the source array
//    srcStride1        stride between the rows in the source matrix
//    srcStride2        stride between the elements in the source object
//    srcRoiShift       ROI shift for the source object
//    pDst, ppDst       pointer to the destination object or array of objects
//    dstStride0        stride between the objects in the destination array
//    dstStride1        stride between the rows in the source matrix
//    dstStride2        stride between the elements in the destination object
//    dstRoiShift       ROI shift for the destination object
//    len               vector length
//    width             matrix width
//    height            matrix height
//    count             number of objects in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmCopy_va_32f_SS,(const Ipp32f*  pSrc,  int srcStride0,  int srcStride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride2,
                                            int len, int count))
IPPAPI(IppStatus, ippmCopy_va_64f_SS,(const Ipp64f*  pSrc,  int srcStride0,  int srcStride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride2,
                                            int len, int count))

IPPAPI(IppStatus, ippmCopy_va_32f_SP,(const Ipp32f*  pSrc,  int srcStride0,  int srcStride2,
                                            Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                            int len, int count))
IPPAPI(IppStatus, ippmCopy_va_64f_SP,(const Ipp64f*  pSrc,  int srcStride0,  int srcStride2,
                                            Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                            int len, int count))

IPPAPI(IppStatus, ippmCopy_va_32f_SL,(const Ipp32f*  pSrc,  int srcStride0,  int srcStride2,
                                            Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                            int len, int count))
IPPAPI(IppStatus, ippmCopy_va_64f_SL,(const Ipp64f*  pSrc,  int srcStride0,  int srcStride2,
                                            Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                            int len, int count))

IPPAPI(IppStatus, ippmCopy_va_32f_LS,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride2,
                                            int len, int count))
IPPAPI(IppStatus, ippmCopy_va_64f_LS,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride2,
                                            int len, int count))

IPPAPI(IppStatus, ippmCopy_va_32f_PS,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride2,
                                            int len, int count))
IPPAPI(IppStatus, ippmCopy_va_64f_PS,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride2,
                                            int len, int count))

IPPAPI(IppStatus, ippmCopy_va_32f_LP,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride2,
                                            Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                            int len, int count))
IPPAPI(IppStatus, ippmCopy_va_64f_LP,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride2,
                                            Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                            int len, int count))

IPPAPI(IppStatus, ippmCopy_va_32f_LL,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride2,
                                            Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                            int len, int count))
IPPAPI(IppStatus, ippmCopy_va_64f_LL,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride2,
                                            Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                            int len, int count))

IPPAPI(IppStatus, ippmCopy_va_32f_PP,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                            Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                            int len, int count))
IPPAPI(IppStatus, ippmCopy_va_64f_PP,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                            Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                            int len, int count))

IPPAPI(IppStatus, ippmCopy_va_32f_PL,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                            Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                            int len, int count))
IPPAPI(IppStatus, ippmCopy_va_64f_PL,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                            Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                            int len, int count))

IPPAPI(IppStatus, ippmCopy_ma_32f_SS,(const Ipp32f*  pSrc,  int srcStride0, int srcStride1, int srcStride2,
                                            Ipp32f*  pDst,  int dstStride0, int dstStride1, int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmCopy_ma_64f_SS,(const Ipp64f*  pSrc,  int srcStride0, int srcStride1, int srcStride2,
                                            Ipp64f*  pDst,  int dstStride0, int dstStride1, int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmCopy_ma_32f_SP,(const Ipp32f*  pSrc,  int srcStride0, int srcStride1, int srcStride2,
                                            Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmCopy_ma_64f_SP,(const Ipp64f*  pSrc,  int srcStride0, int srcStride1, int srcStride2,
                                            Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmCopy_ma_32f_SL,(const Ipp32f*  pSrc,  int srcStride0, int srcStride1, int srcStride2,
                                            Ipp32f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmCopy_ma_64f_SL,(const Ipp64f*  pSrc,  int srcStride0, int srcStride1, int srcStride2,
                                            Ipp64f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmCopy_ma_32f_LS,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                            Ipp32f*  pDst,  int dstStride0, int dstStride1, int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmCopy_ma_64f_LS,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                            Ipp64f*  pDst,  int dstStride0, int dstStride1, int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmCopy_ma_32f_PS,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                            Ipp32f*  pDst,  int dstStride0, int dstStride1, int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmCopy_ma_64f_PS,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                            Ipp64f*  pDst,  int dstStride0, int dstStride1, int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmCopy_ma_32f_LP,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                            Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmCopy_ma_64f_LP,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                            Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmCopy_ma_32f_LL,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                            Ipp32f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmCopy_ma_64f_LL,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                            Ipp64f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmCopy_ma_32f_PP,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                            Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmCopy_ma_64f_PP,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                            Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmCopy_ma_32f_PL,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                            Ipp32f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmCopy_ma_64f_PL,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                            Ipp64f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                            int width, int height, int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmLoadIdentity
//  Purpose:    initializes identity matrix
//  Parameters:
//    pDst, ppDst       pointer to the destination matrix or array of matrices
//    dstStride0        stride between the matrices in the destination array
//    dstStride1        stride between the rows in the destination matrix
//    dstStride2        stride between the elements in the destination matrix
//    dstRoiShift       ROI shift for the destination matrix
//    width             matrix width
//    height            matrix height
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmLoadIdentity_ma_32f,  (Ipp32f*  pDst,  int dstStride0, int dstStride1, int dstStride2,
                                             int width, int height, int count))
IPPAPI(IppStatus, ippmLoadIdentity_ma_64f,  (Ipp64f*  pDst,  int dstStride0, int dstStride1, int dstStride2,
                                             int width, int height, int count))

IPPAPI(IppStatus, ippmLoadIdentity_ma_32f_P,(Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                             int width, int height, int count))
IPPAPI(IppStatus, ippmLoadIdentity_ma_64f_P,(Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                             int width, int height, int count))

IPPAPI(IppStatus, ippmLoadIdentity_ma_32f_L,(Ipp32f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                             int width, int height, int count))
IPPAPI(IppStatus, ippmLoadIdentity_ma_64f_L,(Ipp64f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                             int width, int height, int count))

/* /////////////////////////////////////////////////////////////////////////////
//                   Vector Algebra functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmSaxpy
//  Purpose:    performs the "saxpy" operation on vectors
//  Parameters:
//    pSrc1, ppSrc1     pointer to the first source vector or array of vectors
//    src1Stride0       stride between the vectors in the first source array
//    src1Stride2       stride between the elements in the first source vector
//    src1RoiShift      ROI shift for the first source vector
//    scale             multiplier
//    pSrc2, ppSrc2     pointer to the second source vector or array of vectors
//    src2Stride0       stride between the vectors in the second source array
//    src2Stride2       stride between the elements in the second source vector
//    src2RoiShift      ROI shift for the second source vector
//    pDst, ppDst       pointer to the destination vector or array of vectors
//    dstStride0        stride between the vectors in the destination array
//    dstStride2        stride between the elements in the destination vector
//    dstRoiShift       ROI shift for the destination vector
//    len               vector length
//    count             number of vectors in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmSaxpy_vv_32f,   (const Ipp32f* pSrc1, int src1Stride2, Ipp32f scale,
                                       const Ipp32f* pSrc2, int src2Stride2,
                                             Ipp32f* pDst,  int dstStride2,
                                             int len))
IPPAPI(IppStatus, ippmSaxpy_vv_64f,   (const Ipp64f* pSrc1, int src1Stride2, Ipp64f scale,
                                       const Ipp64f* pSrc2, int src2Stride2,
                                             Ipp64f* pDst,  int dstStride2,
                                             int len))

IPPAPI(IppStatus, ippmSaxpy_vv_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, Ipp32f scale,
                                       const Ipp32f** ppSrc2, int src2RoiShift,
                                             Ipp32f** ppDst,  int dstRoiShift,
                                             int len))
IPPAPI(IppStatus, ippmSaxpy_vv_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, Ipp64f scale,
                                       const Ipp64f** ppSrc2, int src2RoiShift,
                                             Ipp64f** ppDst,  int dstRoiShift,
                                             int len))

IPPAPI(IppStatus, ippmSaxpy_vva_32f,  (const Ipp32f* pSrc1, int src1Stride2, Ipp32f scale,
                                       const Ipp32f* pSrc2, int src2Stride0, int src2Stride2,
                                             Ipp32f* pDst,  int dstStride0,  int dstStride2,
                                             int len, int count))
IPPAPI(IppStatus, ippmSaxpy_vva_64f,  (const Ipp64f* pSrc1, int src1Stride2, Ipp64f scale,
                                       const Ipp64f* pSrc2, int src2Stride0, int src2Stride2,
                                             Ipp64f* pDst,  int dstStride0,  int dstStride2,
                                             int len, int count))

IPPAPI(IppStatus, ippmSaxpy_vva_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, Ipp32f scale,
                                       const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                             Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                             int len, int count))
IPPAPI(IppStatus, ippmSaxpy_vva_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, Ipp64f scale,
                                       const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                             Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                             int len, int count))

IPPAPI(IppStatus, ippmSaxpy_vva_32f_L,(const Ipp32f*  pSrc1,  int src1Stride2, Ipp32f scale,
                                       const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                             Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                             int len, int count))
IPPAPI(IppStatus, ippmSaxpy_vva_64f_L,(const Ipp64f*  pSrc1,  int src1Stride2, Ipp64f scale,
                                       const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                             Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                             int len, int count))

IPPAPI(IppStatus, ippmSaxpy_vav_32f,  (const Ipp32f*  pSrc1,  int src1Stride0, int src1Stride2, Ipp32f scale,
                                       const Ipp32f*  pSrc2,  int src2Stride2,
                                             Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                             int len, int count))
IPPAPI(IppStatus, ippmSaxpy_vav_64f,  (const Ipp64f* pSrc1, int src1Stride0, int src1Stride2, Ipp64f scale,
                                       const Ipp64f* pSrc2, int src2Stride2,
                                             Ipp64f* pDst,  int dstStride0,  int dstStride2,
                                             int len, int count))

IPPAPI(IppStatus, ippmSaxpy_vav_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0, Ipp32f scale,
                                        const Ipp32f** ppSrc2, int src2RoiShift,
                                              Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                              int len, int count))
IPPAPI(IppStatus, ippmSaxpy_vav_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0, Ipp64f scale,
                                        const Ipp64f** ppSrc2, int src2RoiShift,
                                              Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                              int len, int count))

IPPAPI(IppStatus, ippmSaxpy_vav_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride2, Ipp32f scale,
                                        const Ipp32f*  pSrc2,  int src2Stride2,
                                              Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                              int len, int count))
IPPAPI(IppStatus, ippmSaxpy_vav_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride2, Ipp64f scale,
                                        const Ipp64f*  pSrc2,  int src2Stride2,
                                              Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                              int len, int count))

IPPAPI(IppStatus, ippmSaxpy_vava_32f,  (const Ipp32f*  pSrc1,  int src1Stride0,  int src1Stride2, Ipp32f scale,
                                        const Ipp32f*  pSrc2,  int src2Stride0,  int src2Stride2,
                                              Ipp32f*  pDst,   int dstStride0,   int dstStride2,
                                              int len, int count))
IPPAPI(IppStatus, ippmSaxpy_vava_64f,  (const Ipp64f*  pSrc1,  int src1Stride0,  int src1Stride2, Ipp64f scale,
                                        const Ipp64f*  pSrc2,  int src2Stride0,  int src2Stride2,
                                              Ipp64f*  pDst,   int dstStride0,   int dstStride2,
                                              int len, int count))

IPPAPI(IppStatus, ippmSaxpy_vava_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0, Ipp32f scale,
                                        const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                              Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                              int len, int count))
IPPAPI(IppStatus, ippmSaxpy_vava_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0, Ipp64f scale,
                                        const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                              Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                              int len, int count))

IPPAPI(IppStatus, ippmSaxpy_vava_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride2, Ipp32f scale,
                                        const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                              Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                              int len, int count))
IPPAPI(IppStatus, ippmSaxpy_vava_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride2, Ipp64f scale,
                                        const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                              Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                              int len, int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmAdd
//  Purpose:    adds constant or vector to another vector
//  Parameters:
//    pSrc1, ppSrc1     pointer to the first source vector or array of vectors
//    src1Stride0       stride between the vectors in the first source array
//    src1Stride2       stride between the elements in the first source vector
//    src1RoiShift      ROI shift for the first source vector
//    pSrc2, ppSrc2     pointer to the second source vector or array of vectors
//    src2Stride0       stride between the vectors in the second source array
//    src2Stride2       stride between the elements in the second source vector
//    src2RoiShift      ROI shift for the second source vector
//    val               constant
//    pVal, ppVal       pointer to the source array of constants
//    valStride0        stride between the constants in the source array of constants
//    valRoiShift       ROI shift for the source array of constants
//    pDst, ppDst       pointer to the destination vector or array of vectors
//    dstStride0        stride between the vectors in the destination array
//    dstStride2        stride between the elements in the destination vector
//    dstRoiShift       ROI shift for the destination vector
//    len               vector length
//    count             number of vectors (constants) in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmAdd_vc_32f,   (const Ipp32f*  pSrc,  int srcStride2, Ipp32f val,
                                           Ipp32f*  pDst,  int dstStride2,
                                           int len))
IPPAPI(IppStatus, ippmAdd_vc_64f,   (const Ipp64f*  pSrc,  int srcStride2, Ipp64f val,
                                           Ipp64f*  pDst,  int dstStride2,
                                           int len))

IPPAPI(IppStatus, ippmAdd_vc_32f_P, (const Ipp32f** ppSrc, int srcRoiShift, Ipp32f val,
                                           Ipp32f** ppDst, int dstRoiShift,
                                           int len))
IPPAPI(IppStatus, ippmAdd_vc_64f_P, (const Ipp64f** ppSrc, int srcRoiShift, Ipp64f val,
                                           Ipp64f** ppDst, int dstRoiShift,
                                           int len))

IPPAPI(IppStatus, ippmAdd_vac_32f,  (const Ipp32f*  pSrc,  int srcStride0, int srcStride2, Ipp32f val,
                                           Ipp32f*  pDst,  int dstStride0, int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmAdd_vac_64f,  (const Ipp64f*  pSrc,  int srcStride0, int srcStride2, Ipp64f val,
                                           Ipp64f*  pDst,  int dstStride0, int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmAdd_vac_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0, Ipp32f val,
                                           Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                           int len, int count))
IPPAPI(IppStatus, ippmAdd_vac_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0, Ipp64f val,
                                           Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                           int len, int count))

IPPAPI(IppStatus, ippmAdd_vac_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride2, Ipp32f val,
                                           Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmAdd_vac_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride2, Ipp64f val,
                                           Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmAdd_vaca_32f,( const Ipp32f* pSrc, int srcStride0, int srcStride2,
                                     const Ipp32f* pVal, int valStride0,
                                           Ipp32f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmAdd_vaca_64f,( const Ipp64f* pSrc, int srcStride0, int srcStride2,
                                     const Ipp64f* pVal, int valStride0,
                                           Ipp64f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmAdd_vaca_32f_P,( const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                       const Ipp32f*  pVal , int valStride0,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmAdd_vaca_64f_P,( const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                       const Ipp64f*  pVal , int valStride0,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmAdd_vaca_32f_L,( const Ipp32f** ppSrc, int srcRoiShift, int srcStride2,
                                       const Ipp32f** ppVal, int valRoiShift,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmAdd_vaca_64f_L,( const Ipp64f** ppSrc, int srcRoiShift, int srcStride2,
                                       const Ipp64f** ppVal, int valRoiShift,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmAdd_vca_32f, ( const Ipp32f* pSrc, int srcStride2,
                                     const Ipp32f* pVal, int valStride0,
                                           Ipp32f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmAdd_vca_64f, ( const Ipp64f* pSrc, int srcStride2,
                                     const Ipp64f* pVal, int valStride0,
                                           Ipp64f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmAdd_vca_32f_P, ( const Ipp32f** ppSrc, int srcRoiShift,
                                       const Ipp32f*  pVal , int valStride0,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmAdd_vca_64f_P, ( const Ipp64f** ppSrc, int srcRoiShift,
                                       const Ipp64f*  pVal , int valStride0,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmAdd_vca_32f_L, ( const Ipp32f*  pSrc,  int srcStride2,
                                       const Ipp32f** ppVal, int valRoiShift,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmAdd_vca_64f_L, ( const Ipp64f*  pSrc,  int srcStride2,
                                       const Ipp64f** ppVal, int valRoiShift,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmAdd_vv_32f,   (const Ipp32f*  pSrc1, int src1Stride2,
                                     const Ipp32f*  pSrc2, int src2Stride2,
                                           Ipp32f*  pDst,  int dstStride2,
                                           int len))
IPPAPI(IppStatus, ippmAdd_vv_64f,   (const Ipp64f*  pSrc1, int src1Stride2,
                                     const Ipp64f*  pSrc2, int src2Stride2,
                                           Ipp64f*  pDst,  int dstStride2,
                                           int len))

IPPAPI(IppStatus, ippmAdd_vv_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                     const Ipp32f** ppSrc2, int src2RoiShift,
                                           Ipp32f** ppDst,  int dstRoiShift,
                                           int len))
IPPAPI(IppStatus, ippmAdd_vv_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                     const Ipp64f** ppSrc2, int src2RoiShift,
                                           Ipp64f** ppDst,  int dstRoiShift,
                                           int len))

IPPAPI(IppStatus, ippmAdd_vav_32f,  (const Ipp32f*  pSrc1,  int src1Stride0, int src1Stride2,
                                     const Ipp32f*  pSrc2,  int src2Stride2,
                                           Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmAdd_vav_64f,  (const Ipp64f*  pSrc1,  int src1Stride0, int src1Stride2,
                                     const Ipp64f*  pSrc2,  int src2Stride2,
                                           Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmAdd_vav_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                     const Ipp32f** ppSrc2, int src2RoiShift,
                                           Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                           int len, int count))
IPPAPI(IppStatus, ippmAdd_vav_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                     const Ipp64f** ppSrc2, int src2RoiShift,
                                           Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                           int len, int count))

IPPAPI(IppStatus, ippmAdd_vav_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride2,
                                     const Ipp32f*  pSrc2,  int src2Stride2,
                                           Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmAdd_vav_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride2,
                                     const Ipp64f*  pSrc2,  int src2Stride2,
                                           Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmAdd_vava_32f, (const Ipp32f*  pSrc1,  int src1Stride0, int src1Stride2,
                                     const Ipp32f*  pSrc2,  int src2Stride0, int src2Stride2,
                                           Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmAdd_vava_64f, (const Ipp64f*  pSrc1,  int src1Stride0, int src1Stride2,
                                     const Ipp64f*  pSrc2,  int src2Stride0, int src2Stride2,
                                           Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmAdd_vava_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int len, int count))
IPPAPI(IppStatus, ippmAdd_vava_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int len, int count))

IPPAPI(IppStatus, ippmAdd_vava_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride2,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                            int len, int count))
IPPAPI(IppStatus, ippmAdd_vava_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride2,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                            int len, int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmSub
//  Purpose:    subtracts constant or vector from another vector
//  Parameters:
//    pSrc1, ppSrc1     pointer to the first source vector or array of vectors
//    src1Stride0       stride between the vectors in the first source array
//    src1Stride2       stride between the elements in the first source vector
//    src1RoiShift      ROI shift for the first source vector
//    pSrc2, ppSrc2     pointer to the second source vector or array of vectors
//    src2Stride0       stride between the vectors in the second source array
//    src2Stride2       stride between the elements in the second source vector
//    src2RoiShift      ROI shift for the second source vector
//    val               constant
//    pVal, ppVal       pointer to the source array of constants
//    valStride0        stride between the constants in the source array of constants
//    valRoiShift       ROI shift for the source array of constants
//    pDst, ppDst       pointer to the destination vector or array of vectors
//    dstStride0        stride between the vectors in the destination array
//    dstStride2        stride between the elements in the destination vector
//    dstRoiShift       ROI shift for the destination vector
//    len               vector length
//    count             number of vectors (constants) in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmSub_vc_32f,   (const Ipp32f*  pSrc,  int srcStride2, Ipp32f val,
                                           Ipp32f*  pDst,  int dstStride2,
                                           int len))
IPPAPI(IppStatus, ippmSub_vc_64f,   (const Ipp64f*  pSrc,  int srcStride2, Ipp64f val,
                                           Ipp64f*  pDst,  int dstStride2,
                                           int len))

IPPAPI(IppStatus, ippmSub_vc_32f_P, (const Ipp32f** ppSrc, int srcRoiShift, Ipp32f val,
                                           Ipp32f** ppDst, int dstRoiShift,
                                           int len))
IPPAPI(IppStatus, ippmSub_vc_64f_P, (const Ipp64f** ppSrc, int srcRoiShift, Ipp64f val,
                                           Ipp64f** ppDst, int dstRoiShift,
                                           int len))

IPPAPI(IppStatus, ippmSub_cv_32f,   (const Ipp32f*  pSrc,  int srcStride2, Ipp32f val,
                                           Ipp32f*  pDst,  int dstStride2,
                                           int len))
IPPAPI(IppStatus, ippmSub_cv_64f,   (const Ipp64f*  pSrc,  int srcStride2, Ipp64f val,
                                           Ipp64f*  pDst,  int dstStride2,
                                           int len))

IPPAPI(IppStatus, ippmSub_cv_32f_P, (const Ipp32f** ppSrc, int srcRoiShift, Ipp32f val,
                                           Ipp32f** ppDst, int dstRoiShift,
                                           int len))
IPPAPI(IppStatus, ippmSub_cv_64f_P, (const Ipp64f** ppSrc, int srcRoiShift, Ipp64f val,
                                           Ipp64f** ppDst, int dstRoiShift,
                                           int len))

IPPAPI(IppStatus, ippmSub_vac_32f,  (const Ipp32f*  pSrc,  int srcStride0, int srcStride2, Ipp32f val,
                                           Ipp32f*  pDst,  int dstStride0, int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmSub_vac_64f,  (const Ipp64f*  pSrc,  int srcStride0, int srcStride2, Ipp64f val,
                                           Ipp64f*  pDst,  int dstStride0, int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmSub_vac_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0, Ipp32f val,
                                           Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                           int len, int count))
IPPAPI(IppStatus, ippmSub_vac_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0, Ipp64f val,
                                           Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                           int len, int count))

IPPAPI(IppStatus, ippmSub_vac_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride2, Ipp32f val,
                                           Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmSub_vac_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride2, Ipp64f val,
                                           Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmSub_cva_32f,  (const Ipp32f*  pSrc,  int srcStride0, int srcStride2, Ipp32f val,
                                           Ipp32f*  pDst,  int dstStride0, int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmSub_cva_64f,  (const Ipp64f*  pSrc,  int srcStride0, int srcStride2, Ipp64f val,
                                           Ipp64f*  pDst,  int dstStride0, int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmSub_cva_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0, Ipp32f val,
                                           Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                           int len, int count))
IPPAPI(IppStatus, ippmSub_cva_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0, Ipp64f val,
                                           Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                           int len, int count))

IPPAPI(IppStatus, ippmSub_cva_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride2, Ipp32f val,
                                           Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmSub_cva_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride2, Ipp64f val,
                                           Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmSub_vaca_32f,( const Ipp32f* pSrc, int srcStride0, int srcStride2,
                                     const Ipp32f* pVal, int valStride0,
                                           Ipp32f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmSub_vaca_64f,( const Ipp64f* pSrc, int srcStride0, int srcStride2,
                                     const Ipp64f* pVal, int valStride0,
                                           Ipp64f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmSub_vaca_32f_P,( const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                       const Ipp32f*  pVal , int valStride0,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_vaca_64f_P,( const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                       const Ipp64f*  pVal , int valStride0,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_vaca_32f_L,( const Ipp32f** ppSrc, int srcRoiShift, int srcStride2,
                                       const Ipp32f** ppVal, int valRoiShift,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_vaca_64f_L,( const Ipp64f** ppSrc, int srcRoiShift, int srcStride2,
                                       const Ipp64f** ppVal, int valRoiShift,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_vca_32f, ( const Ipp32f* pSrc, int srcStride2,
                                     const Ipp32f* pVal, int valStride0,
                                           Ipp32f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmSub_vca_64f, ( const Ipp64f* pSrc, int srcStride2,
                                     const Ipp64f* pVal, int valStride0,
                                           Ipp64f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmSub_vca_32f_P, ( const Ipp32f** ppSrc, int srcRoiShift,
                                       const Ipp32f*  pVal , int valStride0,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_vca_64f_P, ( const Ipp64f** ppSrc, int srcRoiShift,
                                       const Ipp64f*  pVal , int valStride0,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_vca_32f_L, ( const Ipp32f*  pSrc,  int srcStride2,
                                       const Ipp32f** ppVal, int valRoiShift,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_vca_64f_L, ( const Ipp64f*  pSrc,  int srcStride2,
                                       const Ipp64f** ppVal, int valRoiShift,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_cava_32f,( const Ipp32f* pSrc, int srcStride0, int srcStride2,
                                     const Ipp32f* pVal, int valStride0,
                                           Ipp32f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmSub_cava_64f,( const Ipp64f* pSrc, int srcStride0, int srcStride2,
                                     const Ipp64f* pVal, int valStride0,
                                           Ipp64f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmSub_cava_32f_P,( const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                       const Ipp32f*  pVal , int valStride0,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_cava_64f_P,( const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                       const Ipp64f*  pVal , int valStride0,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_cava_32f_L,( const Ipp32f** ppSrc, int srcRoiShift, int srcStride2,
                                       const Ipp32f** ppVal, int valRoiShift,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_cava_64f_L,( const Ipp64f** ppSrc, int srcRoiShift, int srcStride2,
                                       const Ipp64f** ppVal, int valRoiShift,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_cav_32f, ( const Ipp32f* pSrc, int srcStride2,
                                     const Ipp32f* pVal, int valStride0,
                                           Ipp32f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmSub_cav_64f, ( const Ipp64f* pSrc, int srcStride2,
                                     const Ipp64f* pVal, int valStride0,
                                           Ipp64f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmSub_cav_32f_P, ( const Ipp32f** ppSrc, int srcRoiShift,
                                       const Ipp32f*  pVal , int valStride0,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_cav_64f_P, ( const Ipp64f** ppSrc, int srcRoiShift,
                                       const Ipp64f*  pVal , int valStride0,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_cav_32f_L, ( const Ipp32f*  pSrc,  int srcStride2,
                                       const Ipp32f** ppVal, int valRoiShift,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_cav_64f_L, ( const Ipp64f*  pSrc,  int srcStride2,
                                       const Ipp64f** ppVal, int valRoiShift,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmSub_vv_32f,   (const Ipp32f*  pSrc1, int src1Stride2,
                                     const Ipp32f*  pSrc2, int src2Stride2,
                                           Ipp32f*  pDst,  int dstStride2,
                                           int len))
IPPAPI(IppStatus, ippmSub_vv_64f,   (const Ipp64f*  pSrc1, int src1Stride2,
                                     const Ipp64f*  pSrc2, int src2Stride2,
                                           Ipp64f*  pDst,  int dstStride2,
                                           int len))

IPPAPI(IppStatus, ippmSub_vv_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                     const Ipp32f** ppSrc2, int src2RoiShift,
                                           Ipp32f** ppDst,  int dstRoiShift,
                                           int len))
IPPAPI(IppStatus, ippmSub_vv_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                     const Ipp64f** ppSrc2, int src2RoiShift,
                                           Ipp64f** ppDst,  int dstRoiShift,
                                           int len))

IPPAPI(IppStatus, ippmSub_vav_32f,  (const Ipp32f*  pSrc1,  int src1Stride0, int src1Stride2,
                                     const Ipp32f*  pSrc2,  int src2Stride2,
                                           Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmSub_vav_64f,  (const Ipp64f*  pSrc1,  int src1Stride0, int src1Stride2,
                                     const Ipp64f*  pSrc2,  int src2Stride2,
                                           Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmSub_vav_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                     const Ipp32f** ppSrc2, int src2RoiShift,
                                           Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                           int len, int count))
IPPAPI(IppStatus, ippmSub_vav_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                     const Ipp64f** ppSrc2, int src2RoiShift,
                                           Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                           int len, int count))

IPPAPI(IppStatus, ippmSub_vav_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride2,
                                     const Ipp32f*  pSrc2,  int src2Stride2,
                                           Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmSub_vav_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride2,
                                     const Ipp64f*  pSrc2,  int src2Stride2,
                                           Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmSub_vva_32f,  (const Ipp32f*  pSrc1,  int src1Stride2,
                                     const Ipp32f*  pSrc2,  int src2Stride0, int src2Stride2,
                                           Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmSub_vva_64f,  (const Ipp64f*  pSrc1,  int src1Stride2,
                                     const Ipp64f*  pSrc2,  int src2Stride0, int src2Stride2,
                                           Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmSub_vva_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift,
                                     const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                           Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                           int len, int count))
IPPAPI(IppStatus, ippmSub_vva_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift,
                                     const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                           Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                           int len, int count))

IPPAPI(IppStatus, ippmSub_vva_32f_L,(const Ipp32f*  pSrc1,  int src1Stride2,
                                     const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                           Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmSub_vva_64f_L,(const Ipp64f*  pSrc1,  int src1Stride2,
                                     const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                           Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmSub_vava_32f, (const Ipp32f*  pSrc1,  int src1Stride0, int src1Stride2,
                                     const Ipp32f*  pSrc2,  int src2Stride0, int src2Stride2,
                                           Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmSub_vava_64f, (const Ipp64f*  pSrc1,  int src1Stride0, int src1Stride2,
                                     const Ipp64f*  pSrc2,  int src2Stride0, int src2Stride2,
                                           Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmSub_vava_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int len, int count))
IPPAPI(IppStatus, ippmSub_vava_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int len, int count))

IPPAPI(IppStatus, ippmSub_vava_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride2,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                            int len, int count))
IPPAPI(IppStatus, ippmSub_vava_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride2,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                            int len, int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmMul_vc
//  Purpose:    multiplies vector by constant
//  Parameters:
//    pSrc, ppSrc       pointer to the source vector or array of vectors
//    srcStride0        stride between the vectors in the source array
//    srcStride2        stride between the elements in the source vector
//    srcRoiShift       ROI shift for the source vector
//    val               constant
//    pVal, ppVal       pointer to the source array of constants
//    valStride0        stride between the constants in the source array of constants
//    valRoiShift       ROI shift for the source array of constants
//    pDst, ppDst       pointer to the destination vector or array of vectors
//    dstStride0        stride between the vectors in the destination array
//    dstStride2        stride between the elements in the destination vector
//    dstRoiShift       ROI shift for the destination vector
//    len               vector length
//    count             number of vectors (constants) in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmMul_vc_32f,   (const Ipp32f*  pSrc,  int srcStride2, Ipp32f val,
                                           Ipp32f*  pDst,  int dstStride2,
                                           int len))
IPPAPI(IppStatus, ippmMul_vc_64f,   (const Ipp64f*  pSrc,  int srcStride2, Ipp64f val,
                                           Ipp64f*  pDst,  int dstStride2,
                                           int len))

IPPAPI(IppStatus, ippmMul_vc_32f_P, (const Ipp32f** ppSrc, int srcRoiShift, Ipp32f val,
                                           Ipp32f** ppDst, int dstRoiShift,
                                           int len))
IPPAPI(IppStatus, ippmMul_vc_64f_P, (const Ipp64f** ppSrc, int srcRoiShift, Ipp64f val,
                                           Ipp64f** ppDst, int dstRoiShift,
                                           int len))

IPPAPI(IppStatus, ippmMul_vac_32f,  (const Ipp32f*  pSrc,  int srcStride0, int srcStride2, Ipp32f val,
                                           Ipp32f*  pDst,  int dstStride0, int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmMul_vac_64f,  (const Ipp64f*  pSrc,  int srcStride0, int srcStride2, Ipp64f val,
                                           Ipp64f*  pDst,  int dstStride0, int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmMul_vac_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0, Ipp32f val,
                                           Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                           int len, int count))
IPPAPI(IppStatus, ippmMul_vac_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0, Ipp64f val,
                                           Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                           int len, int count))

IPPAPI(IppStatus, ippmMul_vac_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride2, Ipp32f val,
                                           Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                           int len, int count))
IPPAPI(IppStatus, ippmMul_vac_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride2, Ipp64f val,
                                           Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                           int len, int count))

IPPAPI(IppStatus, ippmMul_vaca_32f,( const Ipp32f* pSrc, int srcStride0, int srcStride2,
                                     const Ipp32f* pVal, int valStride0,
                                           Ipp32f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmMul_vaca_64f,( const Ipp64f* pSrc, int srcStride0, int srcStride2,
                                     const Ipp64f* pVal, int valStride0,
                                           Ipp64f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmMul_vaca_32f_P,( const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                       const Ipp32f*  pVal , int valStride0,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmMul_vaca_64f_P,( const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                       const Ipp64f*  pVal , int valStride0,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmMul_vaca_32f_L,( const Ipp32f** ppSrc, int srcRoiShift, int srcStride2,
                                       const Ipp32f** ppVal, int valRoiShift,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmMul_vaca_64f_L,( const Ipp64f** ppSrc, int srcRoiShift, int srcStride2,
                                       const Ipp64f** ppVal, int valRoiShift,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmMul_vca_32f, ( const Ipp32f* pSrc, int srcStride2,
                                     const Ipp32f* pVal, int valStride0,
                                           Ipp32f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmMul_vca_64f, ( const Ipp64f* pSrc, int srcStride2,
                                     const Ipp64f* pVal, int valStride0,
                                           Ipp64f* pDst, int dstStride0, int dstStride2,
                                           int len, int count ))

IPPAPI(IppStatus, ippmMul_vca_32f_P, ( const Ipp32f** ppSrc, int srcRoiShift,
                                       const Ipp32f*  pVal , int valStride0,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmMul_vca_64f_P, ( const Ipp64f** ppSrc, int srcRoiShift,
                                       const Ipp64f*  pVal , int valStride0,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                             int len, int count ))

IPPAPI(IppStatus, ippmMul_vca_32f_L, ( const Ipp32f*  pSrc,  int srcStride2,
                                       const Ipp32f** ppVal, int valRoiShift,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

IPPAPI(IppStatus, ippmMul_vca_64f_L, ( const Ipp64f*  pSrc,  int srcStride2,
                                       const Ipp64f** ppVal, int valRoiShift,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride2,
                                             int len, int count ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmCrossProduct
//  Purpose:    computes cross product of two 3D vectors
//  Parameters:
//    pSrc1, ppSrc1     pointer to the first source vector or array of vectors
//    src1Stride0       stride between the vectors in the first source array
//    src1Stride2       stride between the elements in the first source vector
//    src1RoiShift      ROI shift for the first source vector
//    pSrc2, ppSrc2     pointer to the second source vector or array of vectors
//    src2Stride0       stride between the vectors in the second source array
//    src2Stride2       stride between the elements in the second source vector
//    src2RoiShift      ROI shift for the second source vector
//    pDst, ppDst       pointer to the destination vector or array of vectors
//    dstStride0        stride between the vectors in the destination array
//    dstStride2        stride between the elements in the destination vector
//    dstRoiShift       ROI shift for the destination vector
//    count             number of vectors in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmCrossProduct_vv_32f,    (const Ipp32f*  pSrc1,  int src1Stride2,
                                               const Ipp32f*  pSrc2,  int src2Stride2,
                                                     Ipp32f*  pDst,   int dstStride2))
IPPAPI(IppStatus, ippmCrossProduct_vv_64f,    (const Ipp64f*  pSrc1,  int src1Stride2,
                                               const Ipp64f*  pSrc2,  int src2Stride2,
                                                     Ipp64f*  pDst,   int dstStride2))

IPPAPI(IppStatus, ippmCrossProduct_vv_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                               const Ipp32f** ppSrc2, int src2RoiShift,
                                                     Ipp32f** ppDst,  int dstRoiShift))
IPPAPI(IppStatus, ippmCrossProduct_vv_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                               const Ipp64f** ppSrc2, int src2RoiShift,
                                                     Ipp64f** ppDst,  int dstRoiShift))

IPPAPI(IppStatus, ippmCrossProduct_vva_32f,   (const Ipp32f*  pSrc1,  int src1Stride2,
                                               const Ipp32f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                     Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                                     int count))
IPPAPI(IppStatus, ippmCrossProduct_vva_64f,   (const Ipp64f*  pSrc1,  int src1Stride2,
                                               const Ipp64f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                     Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                                     int count))

IPPAPI(IppStatus, ippmCrossProduct_vva_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                               const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                     Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                                     int count))
IPPAPI(IppStatus, ippmCrossProduct_vva_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                               const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                     Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                                     int count))

IPPAPI(IppStatus, ippmCrossProduct_vva_32f_L, (const Ipp32f*  pSrc1,  int src1Stride2,
                                               const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                     Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                                     int count))
IPPAPI(IppStatus, ippmCrossProduct_vva_64f_L, (const Ipp64f*  pSrc1,  int src1Stride2,
                                               const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                     Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                                     int count))

IPPAPI(IppStatus, ippmCrossProduct_vav_32f,   (const Ipp32f*  pSrc1,  int src1Stride0, int src1Stride2,
                                               const Ipp32f*  pSrc2,  int src2Stride2,
                                                     Ipp32f*  pDst,   int dstStride0, int dstStride2,
                                                     int count))
IPPAPI(IppStatus, ippmCrossProduct_vav_64f,   (const Ipp64f*  pSrc1,  int src1Stride0, int src1Stride2,
                                               const Ipp64f*  pSrc2,  int src2Stride2,
                                                     Ipp64f*  pDst,   int dstStride0, int dstStride2,
                                                     int count))

IPPAPI(IppStatus, ippmCrossProduct_vav_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                               const Ipp32f** ppSrc2, int src2RoiShift,
                                                     Ipp32f** ppDst,  int dstRoiShift, int dstStride0,
                                                     int count))
IPPAPI(IppStatus, ippmCrossProduct_vav_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                               const Ipp64f** ppSrc2, int src2RoiShift,
                                                     Ipp64f** ppDst,  int dstRoiShift, int dstStride0,
                                                     int count))

IPPAPI(IppStatus, ippmCrossProduct_vav_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride2,
                                               const Ipp32f*  pSrc2,  int src2Stride2,
                                                     Ipp32f** ppDst,  int dstRoiShift, int dstStride2,
                                                     int count))
IPPAPI(IppStatus, ippmCrossProduct_vav_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride2,
                                               const Ipp64f*  pSrc2,  int src2Stride2,
                                                     Ipp64f** ppDst,  int dstRoiShift, int dstStride2,
                                                     int count))

IPPAPI(IppStatus, ippmCrossProduct_vava_32f,  (const Ipp32f* pSrc1, int src1Stride0, int src1Stride2,
                                               const Ipp32f* pSrc2, int src2Stride0, int src2Stride2,
                                                     Ipp32f* pDst,  int dstStride0,  int dstStride2,
                                                     int count))
IPPAPI(IppStatus, ippmCrossProduct_vava_64f,  (const Ipp64f* pSrc1, int src1Stride0, int src1Stride2,
                                               const Ipp64f* pSrc2, int src2Stride0, int src2Stride2,
                                                     Ipp64f* pDst,  int dstStride0,  int dstStride2,
                                                     int count))

IPPAPI(IppStatus, ippmCrossProduct_vava_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                               const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                     Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                                     int count))
IPPAPI(IppStatus, ippmCrossProduct_vava_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                               const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                     Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                                     int count))

IPPAPI(IppStatus, ippmCrossProduct_vava_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride2,
                                               const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                     Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                                     int count))
IPPAPI(IppStatus, ippmCrossProduct_vava_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride2,
                                               const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                     Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                                     int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmDotProduct
//  Purpose:    computes dot product of two vectors
//  Parameters:
//    pSrc1, ppSrc1     pointer to the first source vector or array of vectors
//    src1Stride0       stride between the vectors in the first source array
//    src1Stride2       stride between the elements in the first source vector
//    src1RoiShift      ROI shift for the first source vector
//    pSrc2, ppSrc2     pointer to the second source vector or array of vectors
//    src2Stride0       stride between the vectors in the second source array
//    src2Stride2       stride between the elements in the second source vector
//    src2RoiShift      ROI shift for the second source vector
//    pDst              pointer to the destination value or array of values
//    len               vector length
//    count             number of vectors in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmDotProduct_vv_32f,    (const Ipp32f*  pSrc1,  int src1Stride2,
                                             const Ipp32f*  pSrc2,  int src2Stride2,
                                                   Ipp32f*  pDst,   int len))
IPPAPI(IppStatus, ippmDotProduct_vv_64f,    (const Ipp64f*  pSrc1,  int src1Stride2,
                                             const Ipp64f*  pSrc2,  int src2Stride2,
                                                   Ipp64f*  pDst,   int len))

IPPAPI(IppStatus, ippmDotProduct_vv_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                             const Ipp32f** ppSrc2, int src2RoiShift,
                                                   Ipp32f*  pDst,   int len))
IPPAPI(IppStatus, ippmDotProduct_vv_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                             const Ipp64f** ppSrc2, int src2RoiShift,
                                                   Ipp64f*  pDst,  int len))

IPPAPI(IppStatus, ippmDotProduct_vav_32f,   (const Ipp32f*  pSrc1,  int src1Stride0, int src1Stride2,
                                             const Ipp32f*  pSrc2,  int src2Stride2,
                                                   Ipp32f*  pDst,   int len, int count))
IPPAPI(IppStatus, ippmDotProduct_vav_64f,   (const Ipp64f*  pSrc1,  int src1Stride0, int src1Stride2,
                                             const Ipp64f*  pSrc2,  int src2Stride2,
                                                   Ipp64f*  pDst,   int len, int count))

IPPAPI(IppStatus, ippmDotProduct_vav_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                             const Ipp32f** ppSrc2, int src2RoiShift,
                                                   Ipp32f*  pDst,   int len, int count))
IPPAPI(IppStatus, ippmDotProduct_vav_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                             const Ipp64f** ppSrc2, int src2RoiShift,
                                                   Ipp64f*  pDst,   int len, int count))

IPPAPI(IppStatus, ippmDotProduct_vav_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride2,
                                             const Ipp32f*  pSrc2,  int src2Stride2,
                                                   Ipp32f*  pDst,   int len, int count))
IPPAPI(IppStatus, ippmDotProduct_vav_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride2,
                                             const Ipp64f*  pSrc2,  int src2Stride2,
                                                   Ipp64f*  pDst,  int len, int count))

IPPAPI(IppStatus, ippmDotProduct_vava_32f,  (const Ipp32f* pSrc1, int src1Stride0, int src1Stride2,
                                             const Ipp32f* pSrc2, int src2Stride0, int src2Stride2,
                                                   Ipp32f* pDst,  int len, int count))
IPPAPI(IppStatus, ippmDotProduct_vava_64f,  (const Ipp64f* pSrc1, int src1Stride0, int src1Stride2,
                                             const Ipp64f* pSrc2, int src2Stride0, int src2Stride2,
                                                   Ipp64f* pDst,  int len, int count))

IPPAPI(IppStatus, ippmDotProduct_vava_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                             const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                   Ipp32f*  pDst,   int len, int count))
IPPAPI(IppStatus, ippmDotProduct_vava_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                             const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                   Ipp64f*  pDst,   int len, int count))

IPPAPI(IppStatus, ippmDotProduct_vava_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride2,
                                             const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                   Ipp32f*  pDst,   int len, int count))
IPPAPI(IppStatus, ippmDotProduct_vava_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride2,
                                             const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                   Ipp64f*  pDst,   int len, int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmL2Norm
//  Purpose:    computes vector's L2 norm
//  Parameters:
//    pSrc, ppSrc       pointer to the source vector or array of vectors
//    srcStride0        stride between the vectors in the source array
//    srcStride2        stride between the elements in the source vector
//    srcRoiShift       ROI shift for the source vector
//    pDst              pointer to the destination value or array of values
//    len               vector length
//    count             number of vectors in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmL2Norm_v_32f,   (const Ipp32f*  pSrc, int srcStride2,
                                             Ipp32f*  pDst, int len))
IPPAPI(IppStatus, ippmL2Norm_v_64f,   (const Ipp64f*  pSrc, int srcStride2,
                                             Ipp64f*  pDst, int len))

IPPAPI(IppStatus, ippmL2Norm_v_32f_P, (const Ipp32f** ppSrc, int srcRoiShift,
                                             Ipp32f*  pDst,  int len))
IPPAPI(IppStatus, ippmL2Norm_v_64f_P, (const Ipp64f** ppSrc, int srcRoiShift,
                                             Ipp64f*  pDst,  int len))

IPPAPI(IppStatus, ippmL2Norm_va_32f,  (const Ipp32f*  pSrc,  int srcStride0, int srcStride2,
                                             Ipp32f*  pDst,  int len, int count))
IPPAPI(IppStatus, ippmL2Norm_va_64f,  (const Ipp64f*  pSrc,  int srcStride0, int srcStride2,
                                             Ipp64f*  pDst,  int len, int count))

IPPAPI(IppStatus, ippmL2Norm_va_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                             Ipp32f*  pDst,  int len, int count))
IPPAPI(IppStatus, ippmL2Norm_va_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                             Ipp64f*  pDst,  int len, int count))

IPPAPI(IppStatus, ippmL2Norm_va_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride2,
                                             Ipp32f*  pDst,  int len, int count))
IPPAPI(IppStatus, ippmL2Norm_va_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride2,
                                             Ipp64f*  pDst,  int len, int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmLComb
//  Purpose:    composes linear combination of two vectors
//  Parameters:
//    pSrc1, ppSrc1     pointer to the first source vector or array of vectors
//    src1Stride0       stride between the vectors in the first source array
//    src1Stride2       stride between the elements in the first source vector
//    src1RoiShift      ROI shift for the first source vector
//    scale1            first multiplier
//    pSrc2, ppSrc2     pointer to the second source vector or array of vectors
//    src2Stride0       stride between the vectors in the second source array
//    src2Stride2       stride between the elements in the second source vector
//    src2RoiShift      ROI shift for the second source vector
//    scale2            second multiplier
//    pDst, ppDst       pointer to the destination vector or array of vectors
//    dstStride0        stride between the vectors in the destination array
//    dstStride2        stride between the elements in the destination vector
//    dstRoiShift       ROI shift for the destination vector
//    len               vector length
//    count             number of vectors in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmLComb_vv_32f,    (const Ipp32f*  pSrc1,  int src1Stride2, Ipp32f scale1,
                                        const Ipp32f*  pSrc2,  int src2Stride2, Ipp32f scale2,
                                              Ipp32f*  pDst,   int dstStride2,
                                              int len))
IPPAPI(IppStatus, ippmLComb_vv_64f,    (const Ipp64f*  pSrc1,  int src1Stride2, Ipp64f scale1,
                                        const Ipp64f*  pSrc2,  int src2Stride2, Ipp64f scale2,
                                              Ipp64f*  pDst,   int dstStride2,
                                              int len))

IPPAPI(IppStatus, ippmLComb_vv_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift, Ipp32f scale1,
                                        const Ipp32f** ppSrc2, int src2RoiShift, Ipp32f scale2,
                                              Ipp32f** ppDst,  int dstRoiShift,
                                              int len))
IPPAPI(IppStatus, ippmLComb_vv_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift, Ipp64f scale1,
                                        const Ipp64f** ppSrc2, int src2RoiShift, Ipp64f scale2,
                                              Ipp64f** ppDst,  int dstRoiShift,
                                              int len))

IPPAPI(IppStatus, ippmLComb_vav_32f,   (const Ipp32f*  pSrc1,  int src1Stride0, int src1Stride2, Ipp32f scale1,
                                        const Ipp32f*  pSrc2,  int src2Stride2, Ipp32f scale2,
                                              Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                              int len, int count))
IPPAPI(IppStatus, ippmLComb_vav_64f,   (const Ipp64f*  pSrc1,  int src1Stride0, int src1Stride2, Ipp64f scale1,
                                        const Ipp64f*  pSrc2,  int src2Stride2, Ipp64f scale2,
                                              Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                              int len, int count))

IPPAPI(IppStatus, ippmLComb_vav_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0, Ipp32f scale1,
                                        const Ipp32f** ppSrc2, int src2RoiShift, Ipp32f scale2,
                                              Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                              int len, int count))
IPPAPI(IppStatus, ippmLComb_vav_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0, Ipp64f scale1,
                                        const Ipp64f** ppSrc2, int src2RoiShift, Ipp64f scale2,
                                              Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                              int len, int count))

IPPAPI(IppStatus, ippmLComb_vav_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride2, Ipp32f scale1,
                                        const Ipp32f*  pSrc2,  int src2Stride2,  Ipp32f scale2,
                                              Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                              int len, int count))
IPPAPI(IppStatus, ippmLComb_vav_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride2, Ipp64f scale1,
                                        const Ipp64f*  pSrc2,  int src2Stride2,  Ipp64f scale2,
                                              Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                              int len, int count))

IPPAPI(IppStatus, ippmLComb_vava_32f,  (const Ipp32f*  pSrc1,  int src1Stride0, int src1Stride2, Ipp32f scale1,
                                        const Ipp32f*  pSrc2,  int src2Stride0, int src2Stride2, Ipp32f scale2,
                                              Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                              int len, int count))
IPPAPI(IppStatus, ippmLComb_vava_64f,  (const Ipp64f*  pSrc1,  int src1Stride0, int src1Stride2, Ipp64f scale1,
                                        const Ipp64f*  pSrc2,  int src2Stride0, int src2Stride2, Ipp64f scale2,
                                              Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                              int len, int count))

IPPAPI(IppStatus, ippmLComb_vava_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0, Ipp32f scale1,
                                        const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0, Ipp32f scale2,
                                              Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                              int len, int count))
IPPAPI(IppStatus, ippmLComb_vava_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0, Ipp64f scale1,
                                        const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0, Ipp64f scale2,
                                              Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                              int len, int count))

IPPAPI(IppStatus, ippmLComb_vava_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride2, Ipp32f scale1,
                                        const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2, Ipp32f scale2,
                                              Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                              int len, int count))
IPPAPI(IppStatus, ippmLComb_vava_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride2, Ipp64f scale1,
                                        const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2, Ipp64f scale2,
                                              Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                              int len, int count))

/* /////////////////////////////////////////////////////////////////////////////
//                   Matrix Algebra functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmTranspose
//  Purpose:    performs matrix transposition
//  Parameters:
//    pSrc, ppSrc       pointer to the source matrix or array of matrices
//    srcStride0        stride between the matrices in the source array
//    srcStride1        stride between the rows in the source matrix
//    srcStride2        stride between the elements in the source matrix
//    srcRoiShift       ROI shift for the source matrix
//    width             source matrix width
//    height            source matrix height
//    pDst, ppDst       pointer to the destination matrix or array of matrices
//    dstStride0        stride between the matrices in the destination array
//    dstStride1        stride between the rows in the destination matrix
//    dstStride2        stride between the elements in the destination matrix
//    dstRoiShift       ROI shift for the destination matrix
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmTranspose_m_32f,   (const Ipp32f* pSrc, int srcStride1, int srcStride2,
                                                int width, int height,
                                                Ipp32f* pDst, int dstStride1, int dstStride2))
IPPAPI(IppStatus, ippmTranspose_m_64f,   (const Ipp64f* pSrc, int srcStride1, int srcStride2,
                                                int width, int height,
                                                Ipp64f* pDst, int dstStride1, int dstStride2))

IPPAPI(IppStatus, ippmTranspose_m_32f_P, (const Ipp32f** ppSrc, int srcRoiShift,
                                                int width, int height,
                                                Ipp32f** ppDst, int dstRoiShift))
IPPAPI(IppStatus, ippmTranspose_m_64f_P, (const Ipp64f** ppSrc, int srcRoiShift,
                                                int width, int height,
                                                Ipp64f** ppDst, int dstRoiShift))

IPPAPI(IppStatus, ippmTranspose_ma_32f,  (const Ipp32f* pSrc, int srcStride0, int srcStride1, int srcStride2,
                                                int width, int height,
                                                Ipp32f* pDst, int dstStride0, int dstStride1, int dstStride2,
                                                int count))
IPPAPI(IppStatus, ippmTranspose_ma_64f,  (const Ipp64f* pSrc, int srcStride0, int srcStride1, int srcStride2,
                                                int width, int height,
                                                Ipp64f* pDst, int dstStride0, int dstStride1, int dstStride2,
                                                int count))

IPPAPI(IppStatus, ippmTranspose_ma_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                                int width, int height,
                                                Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                                int count))
IPPAPI(IppStatus, ippmTranspose_ma_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                                int width, int height,
                                                Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                                int count))

IPPAPI(IppStatus, ippmTranspose_ma_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                                int width, int height,
                                                Ipp32f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                                int count))
IPPAPI(IppStatus, ippmTranspose_ma_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                                int width, int height,
                                                Ipp64f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                                int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmInvert
//  Purpose:    performs matrix inverse
//  Parameters:
//    pSrc, ppSrc       pointer to the source matrix or array of matrices
//    srcStride0        stride between the matrices in the source array
//    srcStride1        stride between the rows in the source matrix
//    srcStride2        stride between the elements in the source matrix
//    srcRoiShift       ROI shift for the source matrix
//    pBuffer           pointer to the allocated buffer with the specified size for internal
//                      computations, must be at least equal to widthHeight2+widthHeight
//    pDst, ppDst       pointer to the destination matrix or array of matrices
//    dstStride0        stride between the matrices in the destination array
//    dstStride1        stride between the rows in the destination matrix
//    dstStride2        stride between the elements in the destination matrix
//    dstRoiShift       ROI shift for the destination matrix
//    widthHeight       size of the square matrix
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsSingularErr       matrix is singular
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmInvert_m_32f,   (const Ipp32f* pSrc, int srcStride1, int srcStride2,
                                             Ipp32f* pBuffer,
                                             Ipp32f* pDst, int dstStride1, int dstStride2,
                                             int widthHeight))
IPPAPI(IppStatus, ippmInvert_m_64f,   (const Ipp64f* pSrc, int srcStride1, int srcStride2,
                                             Ipp64f* pBuffer,
                                             Ipp64f* pDst, int dstStride1, int dstStride2,
                                             int widthHeight))

IPPAPI(IppStatus, ippmInvert_m_32f_P, (const Ipp32f** ppSrc, int srcRoiShift,
                                             Ipp32f*  pBuffer,
                                             Ipp32f** ppDst, int dstRoiShift,
                                             int widthHeight ))
IPPAPI(IppStatus, ippmInvert_m_64f_P, (const Ipp64f** ppSrc, int srcRoiShift,
                                             Ipp64f*  pBuffer,
                                             Ipp64f** ppDst, int dstRoiShift,
                                             int widthHeight ))

IPPAPI(IppStatus, ippmInvert_ma_32f,  (const Ipp32f* pSrc, int srcStride0, int srcStride1, int srcStride2,
                                             Ipp32f* pBuffer,
                                             Ipp32f* pDst, int dstStride0, int dstStride1, int dstStride2,
                                             int widthHeight, int count))
IPPAPI(IppStatus, ippmInvert_ma_64f,  (const Ipp64f* pSrc, int srcStride0, int srcStride1, int srcStride2,
                                             Ipp64f* pBuffer,
                                             Ipp64f* pDst, int dstStride0, int dstStride1, int dstStride2,
                                             int widthHeight, int count))

IPPAPI(IppStatus, ippmInvert_ma_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                             Ipp32f* pBuffer,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                             int widthHeight, int count))
IPPAPI(IppStatus, ippmInvert_ma_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                             Ipp64f* pBuffer,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                             int widthHeight, int count))

IPPAPI(IppStatus, ippmInvert_ma_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                             Ipp32f*  pBuffer,
                                             Ipp32f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                             int widthHeight, int count))
IPPAPI(IppStatus, ippmInvert_ma_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                             Ipp64f*  pBuffer,
                                             Ipp64f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                             int widthHeight, int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmFrobNorm
//  Purpose:    performs matrix's Frobenius norm
//  Parameters:
//    pSrc, ppSrc       pointer to the source matrix or array of matrices
//    srcStride0        stride between the matrices in the source array
//    srcStride1        stride between the rows in the source matrix
//    srcStride2        stride between the elements in the source matrix
//    srcRoiShift       ROI shift for the source matrix
//    width             matrix width
//    height            matrix height
//    pDst              pointer to the destination value or array of values
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsSqrtNegArg        indicates a warning that a sum of all the squared elements in the source
//                            matrix has a negative value
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmFrobNorm_m_32f,   (const Ipp32f* pSrc, int srcStride1, int srcStride2,
                                               int width, int height,
                                               Ipp32f* pDst))
IPPAPI(IppStatus, ippmFrobNorm_m_64f,   (const Ipp64f* pSrc, int srcStride1, int srcStride2,
                                               int width, int height,
                                               Ipp64f* pDst))

IPPAPI(IppStatus, ippmFrobNorm_m_32f_P, (const Ipp32f** ppSrc, int srcRoiShift,
                                               int width, int height,
                                               Ipp32f* pDst))
IPPAPI(IppStatus, ippmFrobNorm_m_64f_P, (const Ipp64f** ppSrc, int srcRoiShift,
                                               int width, int height,
                                               Ipp64f* pDst))

IPPAPI(IppStatus, ippmFrobNorm_ma_32f,  (const Ipp32f* pSrc, int srcStride0, int srcStride1, int srcStride2,
                                               int width, int height,
                                               Ipp32f* pDst,
                                               int count))
IPPAPI(IppStatus, ippmFrobNorm_ma_64f,  (const Ipp64f* pSrc, int srcStride0, int srcStride1, int srcStride2,
                                               int width, int height,
                                               Ipp64f* pDst,
                                               int count))

IPPAPI(IppStatus, ippmFrobNorm_ma_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                               int width, int height,
                                               Ipp32f* pDst,
                                               int count))
IPPAPI(IppStatus, ippmFrobNorm_ma_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                               int width, int height,
                                               Ipp64f* pDst,
                                               int count))

IPPAPI(IppStatus, ippmFrobNorm_ma_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                               int width, int height,
                                               Ipp32f* pDst,
                                               int count))
IPPAPI(IppStatus, ippmFrobNorm_ma_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                               int width, int height,
                                               Ipp64f* pDst,
                                               int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmDet
//  Purpose:    performs matrix's determinant
//  Parameters:
//    pSrc, ppSrc       pointer to the source matrix or array of matrices
//    srcStride0        stride between the matrices in the source array
//    srcStride1        stride between the rows in the source matrix
//    srcStride2        stride between the elements in the source matrix
//    srcRoiShift       ROI shift for the source matrix
//    widthHeight       size of the square matrix
//    pBuffer           pointer to the allocated buffer with the specified size for internal
//                      computations; must be at least equal to widthHeight*(1+widthHeight)
//    pDst              pointer to the destination value or array of values
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmDet_m_32f,   (const Ipp32f*  pSrc, int srcStride1, int srcStride2,
                                          int widthHeight, Ipp32f* pBuffer,
                                          Ipp32f*  pDst))
IPPAPI(IppStatus, ippmDet_m_64f,   (const Ipp64f*  pSrc, int srcStride1, int srcStride2,
                                          int widthHeight, Ipp64f* pBuffer,
                                          Ipp64f*  pDst))

IPPAPI(IppStatus, ippmDet_m_32f_P, (const Ipp32f** ppSrc, int srcRoiShift,
                                          int widthHeight, Ipp32f* pBuffer,
                                          Ipp32f*  pDst))
IPPAPI(IppStatus, ippmDet_m_64f_P, (const Ipp64f** ppSrc, int srcRoiShift,
                                          int widthHeight, Ipp64f* pBuffer,
                                          Ipp64f*  pDst))

IPPAPI(IppStatus, ippmDet_ma_32f, (const Ipp32f*   pSrc, int srcStride0, int srcStride1, int srcStride2,
                                          int widthHeight, Ipp32f* pBuffer,
                                          Ipp32f*  pDst, int count))
IPPAPI(IppStatus, ippmDet_ma_64f, (const Ipp64f*   pSrc, int srcStride0, int srcStride1, int srcStride2,
                                          int widthHeight, Ipp64f* pBuffer,
                                          Ipp64f*  pDst, int count))

IPPAPI(IppStatus, ippmDet_ma_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                          int widthHeight, Ipp32f* pBuffer,
                                          Ipp32f*  pDst,  int count))
IPPAPI(IppStatus, ippmDet_ma_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                          int widthHeight, Ipp64f* pBuffer,
                                          Ipp64f*  pDst,  int count))

IPPAPI(IppStatus, ippmDet_ma_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                          int widthHeight, Ipp32f* pBuffer,
                                          Ipp32f*  pDst,  int count))
IPPAPI(IppStatus, ippmDet_ma_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                          int widthHeight, Ipp64f* pBuffer,
                                          Ipp64f*  pDst,  int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmTrace
//  Purpose:    performs matrix's trace
//  Parameters:
//    pSrc, ppSrc       pointer to the source matrix or array of matrices
//    srcStride0        stride between the matrices in the source array
//    srcStride1        stride between the rows in the source matrix
//    srcStride2        stride between the elements in the source matrix
//    srcRoiShift       ROI shift for the source matrix
//    widthHeight       size of the square matrix
//    pDst              pointer to the destination value or array of values
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmTrace_m_32f,   (const Ipp32f*  pSrc,   int srcStride1, int srcStride2,
                                            int widthHeight, Ipp32f* pDst))
IPPAPI(IppStatus, ippmTrace_m_64f,   (const Ipp64f*  pSrc,   int srcStride1, int srcStride2,
                                            int widthHeight, Ipp64f* pDst))

IPPAPI(IppStatus, ippmTrace_m_32f_P, (const Ipp32f** ppSrc,  int srcRoiShift,
                                            int widthHeight, Ipp32f* pDst))
IPPAPI(IppStatus, ippmTrace_m_64f_P, (const Ipp64f** ppSrc,  int srcRoiShift,
                                            int widthHeight, Ipp64f* pDst))

IPPAPI(IppStatus, ippmTrace_ma_32f, (const Ipp32f*   pSrc,   int srcStride0, int srcStride1, int srcStride2,
                                           int widthHeight,  Ipp32f* pDst,   int count))
IPPAPI(IppStatus, ippmTrace_ma_64f, (const Ipp64f*   pSrc,   int srcStride0, int srcStride1, int srcStride2,
                                           int widthHeight,  Ipp64f* pDst,   int count))

IPPAPI(IppStatus, ippmTrace_ma_32f_P,(const Ipp32f** ppSrc,  int srcRoiShift, int srcStride0,
                                            int widthHeight, Ipp32f* pDst,    int count))
IPPAPI(IppStatus, ippmTrace_ma_64f_P,(const Ipp64f** ppSrc,  int srcRoiShift, int srcStride0,
                                            int widthHeight, Ipp64f* pDst,    int count))

IPPAPI(IppStatus, ippmTrace_ma_32f_L,(const Ipp32f** ppSrc,  int srcRoiShift, int srcStride1, int srcStride2,
                                            int widthHeight, Ipp32f* pDst,    int count))
IPPAPI(IppStatus, ippmTrace_ma_64f_L,(const Ipp64f** ppSrc,  int srcRoiShift, int srcStride1, int srcStride2,
                                            int widthHeight, Ipp64f* pDst,    int count))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmMul_mc
//  Purpose:    multiplies matrix by a constant
//  Parameters:
//    pSrc, ppSrc       pointer to the source matrix or array of matrices
//    srcStride0        stride between the matrices in the source array
//    srcStride1        stride between the rows in the source matrix
//    srcStride2        stride between the elements in the source matrix
//    srcRoiShift       ROI shift for the source matrix
//    val               multiplier
//    pDst, ppDst       pointer to the destination matrix or array of matrices
//    dstStride0        stride between the matrices in the destination array
//    dstStride1        stride between the rows in the destination matrix
//    dstStride2        stride between the elements in the destination matrix
//    dstRoiShift       ROI shift for the destination matrix
//    width             width of the destination matrix
//    height            height of the destination matrix
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmMul_mc_32f,   (const Ipp32f*  pSrc, int srcStride1, int srcStride2,
                                           Ipp32f   val,
                                           Ipp32f*  pDst, int dstStride1, int dstStride2,
                                           int width, int height))
IPPAPI(IppStatus, ippmMul_mc_64f,   (const Ipp64f*  pSrc, int srcStride1, int srcStride2,
                                           Ipp64f   val,
                                           Ipp64f*  pDst, int dstStride1, int dstStride2,
                                           int width, int height))

IPPAPI(IppStatus, ippmMul_mc_32f_P, (const Ipp32f** ppSrc, int srcRoiShift,
                                           Ipp32f   val,
                                           Ipp32f** ppDst, int dstRoiShift,
                                           int width, int height))
IPPAPI(IppStatus, ippmMul_mc_64f_P, (const Ipp64f** ppSrc, int srcRoiShift,
                                           Ipp64f   val,
                                           Ipp64f** ppDst, int dstRoiShift,
                                           int width, int height))

IPPAPI(IppStatus, ippmMul_tc_32f,   (const Ipp32f*  pSrc, int srcStride1, int srcStride2,
                                           Ipp32f   val,
                                           Ipp32f*  pDst, int dstStride1, int dstStride2,
                                           int width, int height))
IPPAPI(IppStatus, ippmMul_tc_64f,   (const Ipp64f*  pSrc, int srcStride1, int srcStride2,
                                           Ipp64f   val,
                                           Ipp64f*  pDst, int dstStride1, int dstStride2,
                                           int width, int height))

IPPAPI(IppStatus, ippmMul_tc_32f_P, (const Ipp32f** ppSrc, int srcRoiShift,
                                           Ipp32f   val,
                                           Ipp32f** ppDst, int dstRoiShift,
                                           int width, int height))
IPPAPI(IppStatus, ippmMul_tc_64f_P, (const Ipp64f** ppSrc, int srcRoiShift,
                                           Ipp64f   val,
                                           Ipp64f** ppDst, int dstRoiShift,
                                           int width, int height))

IPPAPI(IppStatus, ippmMul_mac_32f,  (const Ipp32f*  pSrc, int srcStride0, int srcStride1, int srcStride2,
                                           Ipp32f   val,
                                           Ipp32f*  pDst, int dstStride0, int dstStride1, int dstStride2,
                                           int width, int height, int count))
IPPAPI(IppStatus, ippmMul_mac_64f,  (const Ipp64f*  pSrc, int srcStride0, int srcStride1, int srcStride2,
                                           Ipp64f   val,
                                           Ipp64f*  pDst, int dstStride0, int dstStride1, int dstStride2,
                                           int width, int height, int count))

IPPAPI(IppStatus, ippmMul_mac_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                           Ipp32f   val,
                                           Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                           int width, int height, int count))
IPPAPI(IppStatus, ippmMul_mac_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                           Ipp64f   val,
                                           Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                           int width, int height, int count))

IPPAPI(IppStatus, ippmMul_mac_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                           Ipp32f   val,
                                           Ipp32f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                           int width, int height, int count))
IPPAPI(IppStatus, ippmMul_mac_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                           Ipp64f   val,
                                           Ipp64f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                           int width, int height, int count))

IPPAPI(IppStatus, ippmMul_tac_32f,  (const Ipp32f*  pSrc, int srcStride0, int srcStride1, int srcStride2,
                                           Ipp32f   val,
                                           Ipp32f*  pDst, int dstStride0, int dstStride1, int dstStride2,
                                           int width, int height, int count))
IPPAPI(IppStatus, ippmMul_tac_64f,  (const Ipp64f*  pSrc, int srcStride0, int srcStride1, int srcStride2,
                                           Ipp64f   val,
                                           Ipp64f*  pDst, int dstStride0, int dstStride1, int dstStride2,
                                           int width, int height, int count))

IPPAPI(IppStatus, ippmMul_tac_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                           Ipp32f   val,
                                           Ipp32f** ppDst, int dstRoiShift, int dstStride0,
                                           int width, int height, int count))
IPPAPI(IppStatus, ippmMul_tac_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                           Ipp64f   val,
                                           Ipp64f** ppDst, int dstRoiShift, int dstStride0,
                                           int width, int height, int count))

IPPAPI(IppStatus, ippmMul_tac_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                           Ipp32f   val,
                                           Ipp32f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                           int width, int height, int count))
IPPAPI(IppStatus, ippmMul_tac_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                           Ipp64f   val,
                                           Ipp64f** ppDst, int dstRoiShift, int dstStride1, int dstStride2,
                                           int width, int height, int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmMul_mv
//  Purpose:    multiplies matrix by vector
//  Parameters:
//    pSrc1, ppSrc1     pointer to the source matrix or array of matrices
//    src1Stride0       stride between the matrices in the source matrix array
//    src1Stride1       stride between the rows in the source matrix
//    src1Stride2       stride between the elements in the source matrix
//    src1RoiShift      ROI shift for the source matrix
//    src1Width         matrix width
//    src1Height        matrix height
//    pSrc2, ppSrc2     pointer to the source vector or array of vectors
//    src2Stride0       stride between the vectors in the source vector array
//    src2Stride2       stride between the elements in the source vector
//    src2RoiShift      ROI shift for the source vector
//    src2Len           length of the source vector
//    pDst, ppDst       pointer to the destination vector or array of vectors
//    dstStride0        stride between the vectors in the destination array
//    dstStride2        stride between the elements in the destination vector
//    dstRoiShift       ROI shift for the destination vector
//    count             number of objects in the array
//  Return:
//    ippStsNullPtrErr         pointer(s) to the data is NULL
//    ippStsSizeErr            data size value is less or equal zero
//    ippStsCountMatrixErr     count value is less or equal zero
//    ippStsSizeMatchMatrixErr unsuitable sizes of the source matrices
//    ippStsStrideMatrixErr    stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr  RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr              otherwise
*/

IPPAPI(IppStatus, ippmMul_mv_32f,    (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride2, int src2Len,
                                            Ipp32f*  pDst,  int dstStride2))
IPPAPI(IppStatus, ippmMul_mv_64f,    (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride2, int src2Len,
                                            Ipp64f*  pDst,  int dstStride2))

IPPAPI(IppStatus, ippmMul_mv_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift))
IPPAPI(IppStatus, ippmMul_mv_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift))

IPPAPI(IppStatus, ippmMul_tv_32f,    (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride2, int src2Len,
                                            Ipp32f*  pDst,  int dstStride2))
IPPAPI(IppStatus, ippmMul_tv_64f,    (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride2, int src2Len,
                                            Ipp64f*  pDst,  int dstStride2))

IPPAPI(IppStatus, ippmMul_tv_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift))
IPPAPI(IppStatus, ippmMul_tv_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift))

IPPAPI(IppStatus, ippmMul_mva_32f,   (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride2, int src2Len,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride2, int count))
IPPAPI(IppStatus, ippmMul_mva_64f,   (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride2, int src2Len,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride2, int count))

IPPAPI(IppStatus, ippmMul_mva_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0, int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,  int count))
IPPAPI(IppStatus, ippmMul_mva_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0, int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,  int count))

IPPAPI(IppStatus, ippmMul_mva_32f_L, (const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2, int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,  int count))
IPPAPI(IppStatus, ippmMul_mva_64f_L, (const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2, int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,  int count))

IPPAPI(IppStatus, ippmMul_tva_32f,   (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride2, int src2Len,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride2,  int count))
IPPAPI(IppStatus, ippmMul_tva_64f,   (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride2, int src2Len,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride2,  int count))

IPPAPI(IppStatus, ippmMul_tva_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0, int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,  int count))
IPPAPI(IppStatus, ippmMul_tva_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0, int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,  int count))

IPPAPI(IppStatus, ippmMul_tva_32f_L, (const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2, int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,  int count))
IPPAPI(IppStatus, ippmMul_tva_64f_L, (const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2, int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,  int count))

IPPAPI(IppStatus, ippmMul_mav_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride2, int src2Len,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride2, int count))
IPPAPI(IppStatus, ippmMul_mav_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride2, int src2Len,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride2, int count))

IPPAPI(IppStatus, ippmMul_mav_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0, int count))
IPPAPI(IppStatus, ippmMul_mav_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0, int count))

IPPAPI(IppStatus, ippmMul_mav_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f*  pSrc2,  int src2Stride2,  int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride2, int count))
IPPAPI(IppStatus, ippmMul_mav_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f*  pSrc2,  int src2Stride2,  int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride2, int count))

IPPAPI(IppStatus, ippmMul_tav_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride2, int src2Len,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride2, int count))
IPPAPI(IppStatus, ippmMul_tav_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride2, int src2Len,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride2, int count))

IPPAPI(IppStatus, ippmMul_tav_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0, int count))
IPPAPI(IppStatus, ippmMul_tav_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0, int count))

IPPAPI(IppStatus, ippmMul_tav_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f*  pSrc2,  int src2Stride2,  int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride2, int count))
IPPAPI(IppStatus, ippmMul_tav_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f*  pSrc2,  int src2Stride2,  int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride2, int count))

IPPAPI(IppStatus, ippmMul_mava_32f,  (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride2, int src2Len,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride2,  int count))
IPPAPI(IppStatus, ippmMul_mava_64f,  (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride2, int src2Len,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride2,  int count))

IPPAPI(IppStatus, ippmMul_mava_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0, int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0, int count))
IPPAPI(IppStatus, ippmMul_mava_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0, int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0, int count))

IPPAPI(IppStatus, ippmMul_mava_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2, int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,  int count))
IPPAPI(IppStatus, ippmMul_mava_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2, int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,  int count))

IPPAPI(IppStatus, ippmMul_tava_32f,  (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride2, int src2Len,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride2,  int count))
IPPAPI(IppStatus, ippmMul_tava_64f,  (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride2, int src2Len,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride2,  int count))

IPPAPI(IppStatus, ippmMul_tava_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0, int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,  int count))
IPPAPI(IppStatus, ippmMul_tava_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0, int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,  int count))

IPPAPI(IppStatus, ippmMul_tava_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2, int src2Len,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,  int count))
IPPAPI(IppStatus, ippmMul_tava_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2, int src2Len,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,  int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmMul_mm
//  Purpose:    multiplies matrix by another matrix
//  Parameters:
//    pSrc1, ppSrc1     pointer to the first source matrix or array of matrices
//    src1Stride0       stride between the matrices in the first source matrix array
//    src1Stride1       stride between the rows in the first source matrix
//    src1Stride2       stride between the elements in the first source matrix
//    src1RoiShift      ROI shift for the first source matrix
//    src1Width         width of the first source matrix
//    src1Height        height of the first source matrix
//    pSrc2, ppSrc2     pointer to the second source matrix or array of matrices
//    src2Stride0       stride between the matrices in the second source matrix array
//    src2Stride1       stride between the rows in the second source matrix
//    src2Stride2       stride between the elements in the second source matrix
//    src2RoiShift      ROI shift for the second source matrix
//    src2Width         width of the second source matrix
//    src2Height        height of the second source matrix
//    pDst, ppDst       pointer to the destination matrix or array of matrices
//    dstStride0        stride between the matrices in the destination array
//    dstStride1        stride between the rows in the destination matrix
//    dstStride2        stride between the elements in the destination matrix
//    dstRoiShift       ROI shift for the destination matrix
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr         pointer(s) to the data is NULL
//    ippStsSizeErr            data size value is less or equal zero
//    ippStsSizeMatchMatrixErr unsuitable sizes of the source matrices
//    ippStsCountMatrixErr     count value is less or equal zero
//    ippStsStrideMatrixErr    stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr  RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr              otherwise
*/

IPPAPI(IppStatus, ippmMul_mm_32f,    (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride1,  int dstStride2))
IPPAPI(IppStatus, ippmMul_mm_64f,    (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride1,  int dstStride2))

IPPAPI(IppStatus, ippmMul_mm_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift))
IPPAPI(IppStatus, ippmMul_mm_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift))

IPPAPI(IppStatus, ippmMul_tm_32f,    (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride1,  int dstStride2))
IPPAPI(IppStatus, ippmMul_tm_64f,    (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride1,  int dstStride2))

IPPAPI(IppStatus, ippmMul_tm_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift))
IPPAPI(IppStatus, ippmMul_tm_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift))

IPPAPI(IppStatus, ippmMul_mt_32f,    (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride1,  int dstStride2))
IPPAPI(IppStatus, ippmMul_mt_64f,    (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride1,  int dstStride2))

IPPAPI(IppStatus, ippmMul_mt_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift))
IPPAPI(IppStatus, ippmMul_mt_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift))

IPPAPI(IppStatus, ippmMul_tt_32f,    (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride1,  int dstStride2))
IPPAPI(IppStatus, ippmMul_tt_64f,    (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride1,  int dstStride2))

IPPAPI(IppStatus, ippmMul_tt_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift))
IPPAPI(IppStatus, ippmMul_tt_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift))

IPPAPI(IppStatus, ippmMul_mma_32f,   (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_mma_64f,   (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_mma_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))
IPPAPI(IppStatus, ippmMul_mma_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))

IPPAPI(IppStatus, ippmMul_mma_32f_L, (const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_mma_64f_L, (const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_tma_32f,   (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_tma_64f,   (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_tma_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))
IPPAPI(IppStatus, ippmMul_tma_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))

IPPAPI(IppStatus, ippmMul_tma_32f_L, (const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_tma_64f_L, (const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_mta_32f,   (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_mta_64f,   (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_mta_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))
IPPAPI(IppStatus, ippmMul_mta_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))

IPPAPI(IppStatus, ippmMul_mta_32f_L, (const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_mta_64f_L, (const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_tta_32f,   (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_tta_64f,   (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_tta_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))
IPPAPI(IppStatus, ippmMul_tta_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))

IPPAPI(IppStatus, ippmMul_tta_32f_L, (const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_tta_64f_L, (const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_mam_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_mam_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_mam_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))
IPPAPI(IppStatus, ippmMul_mam_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))

IPPAPI(IppStatus, ippmMul_mam_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f*  pSrc2,  int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_mam_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f*  pSrc2,  int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_tam_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_tam_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_tam_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))
IPPAPI(IppStatus, ippmMul_tam_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))

IPPAPI(IppStatus, ippmMul_tam_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f*  pSrc2,  int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_tam_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f*  pSrc2,  int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_mat_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_mat_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_mat_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))
IPPAPI(IppStatus, ippmMul_mat_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))

IPPAPI(IppStatus, ippmMul_mat_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f*  pSrc2,  int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_mat_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f*  pSrc2,  int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_tat_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_tat_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_tat_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))
IPPAPI(IppStatus, ippmMul_tat_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))

IPPAPI(IppStatus, ippmMul_tat_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f*  pSrc2,  int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_tat_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f*  pSrc2,  int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_mama_32f,  (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_mama_64f,  (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_mama_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))
IPPAPI(IppStatus, ippmMul_mama_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))

IPPAPI(IppStatus, ippmMul_mama_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_mama_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_tama_32f,  (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_tama_64f,  (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_tama_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))
IPPAPI(IppStatus, ippmMul_tama_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))

IPPAPI(IppStatus, ippmMul_tama_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_tama_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_mata_32f,  (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_mata_64f,  (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_mata_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))
IPPAPI(IppStatus, ippmMul_mata_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))

IPPAPI(IppStatus, ippmMul_mata_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_mata_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_tata_32f,  (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_tata_64f,  (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                            int src1Width,  int src1Height,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            int src2Width,  int src2Height,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int count))

IPPAPI(IppStatus, ippmMul_tata_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))
IPPAPI(IppStatus, ippmMul_tata_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int count))

IPPAPI(IppStatus, ippmMul_tata_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))
IPPAPI(IppStatus, ippmMul_tata_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                            int src1Width,   int src1Height,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            int src2Width,   int src2Height,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmAdd_mm
//  Purpose:    adds matrix to another matrix
//  Parameters:
//    pSrc1, ppSrc1     pointer to the first source matrix or array of matrices
//    src1Stride0       stride between the matrices in the first source matrix array
//    src1Stride1       stride between the rows in the first source matrix
//    src1Stride2       stride between the elements in the first source matrix
//    src1RoiShift      ROI shift for the first source matrix
//    pSrc2, ppSrc2     pointer to the second source matrix or array of matrices
//    src2Stride0       stride between the matrices in the second source matrix array
//    src2Stride1       stride between the rows in the second source matrix
//    src2Stride2       stride between the elements in the second source matrix
//    src2RoiShift      ROI shift for the second source matrix
//    pDst, ppDst       pointer to the destination matrix or array of matrices
//    dstStride0        stride between the matrices in the destination array
//    dstStride1        stride between the rows in the destination matrix
//    dstStride2        stride between the elements in the destination matrix
//    dstRoiShift       ROI shift for the destination matrix
//    width             width of the destination matrix
//    height            height of the destination matrix
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/
IPPAPI(IppStatus, ippmAdd_mm_32f,    (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))
IPPAPI(IppStatus, ippmAdd_mm_64f,    (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))

IPPAPI(IppStatus, ippmAdd_mm_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,
                                            int width, int height))
IPPAPI(IppStatus, ippmAdd_mm_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,
                                            int width, int height))

IPPAPI(IppStatus, ippmAdd_tm_32f,    (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))
IPPAPI(IppStatus, ippmAdd_tm_64f,    (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))

IPPAPI(IppStatus, ippmAdd_tm_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,
                                            int width, int height))
IPPAPI(IppStatus, ippmAdd_tm_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,
                                            int width, int height))

IPPAPI(IppStatus, ippmAdd_tt_32f,    (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))
IPPAPI(IppStatus, ippmAdd_tt_64f,    (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))

IPPAPI(IppStatus, ippmAdd_tt_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,
                                            int width, int height))
IPPAPI(IppStatus, ippmAdd_tt_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,
                                            int width, int height))

IPPAPI(IppStatus, ippmAdd_mam_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_mam_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_mam_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_mam_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_mam_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_mam_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_tam_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_tam_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_tam_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_tam_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_tam_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_tam_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_mat_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_mat_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_mat_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_mat_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_mat_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_mat_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_tat_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_tat_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_tat_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_tat_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_tat_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_tat_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_mama_32f,  (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_mama_64f,  (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_mama_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_mama_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_mama_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_mama_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_tama_32f,  (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_tama_64f,  (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_tama_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_tama_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_tama_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_tama_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_tata_32f,  (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_tata_64f,  (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_tata_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_tata_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmAdd_tata_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmAdd_tata_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmSub_mm
//  Purpose:    substracts matrix from another matrix
//  Parameters:
//    pSrc1, ppSrc1     pointer to the first source matrix or array of matrices
//    src1Stride0       stride between the matrices in the first source matrix array
//    src1Stride1       stride between the rows in the first source matrix
//    src1Stride2       stride between the elements in the first source matrix
//    src1RoiShift      ROI shift for the first source matrix
//    pSrc2, ppSrc2     pointer to the second source matrix or array of matrices
//    src2Stride0       stride between the matrices in the second source matrix array
//    src2Stride1       stride between the rows in the second source matrix
//    src2Stride2       stride between the elements in the second source matrix
//    src2RoiShift      ROI shift for the second source matrix
//    pDst, ppDst       pointer to the destination matrix or array of matrices
//    dstStride0        stride between the matrices in the destination array
//    dstStride1        stride between the rows in the destination matrix
//    dstStride2        stride between the elements in the destination matrix
//    dstRoiShift       ROI shift for the destination matrix
//    width             width of the destination matrix
//    height            height of the destination matrix
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/
IPPAPI(IppStatus, ippmSub_mm_32f,    (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))
IPPAPI(IppStatus, ippmSub_mm_64f,    (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))

IPPAPI(IppStatus, ippmSub_mm_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,
                                            int width, int height))
IPPAPI(IppStatus, ippmSub_mm_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,
                                            int width, int height))

IPPAPI(IppStatus, ippmSub_tm_32f,    (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))
IPPAPI(IppStatus, ippmSub_tm_64f,    (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))

IPPAPI(IppStatus, ippmSub_tm_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,
                                            int width, int height))
IPPAPI(IppStatus, ippmSub_tm_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,
                                            int width, int height))

IPPAPI(IppStatus, ippmSub_mt_32f,    (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))
IPPAPI(IppStatus, ippmSub_mt_64f,    (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))

IPPAPI(IppStatus, ippmSub_mt_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,
                                            int width, int height))
IPPAPI(IppStatus, ippmSub_mt_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,
                                            int width, int height))

IPPAPI(IppStatus, ippmSub_tt_32f,    (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))
IPPAPI(IppStatus, ippmSub_tt_64f,    (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride1,  int dstStride2,
                                            int width, int height))

IPPAPI(IppStatus, ippmSub_tt_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,
                                            int width, int height))
IPPAPI(IppStatus, ippmSub_tt_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,
                                            int width, int height))

IPPAPI(IppStatus, ippmSub_mma_32f,   (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mma_64f,   (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mma_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mma_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mma_32f_L, (const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mma_64f_L, (const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tma_32f,   (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tma_64f,   (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tma_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tma_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tma_32f_L, (const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tma_64f_L, (const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mta_32f,   (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mta_64f,   (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mta_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mta_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mta_32f_L, (const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mta_64f_L, (const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tta_32f,   (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tta_64f,   (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tta_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tta_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tta_32f_L, (const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tta_64f_L, (const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mam_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mam_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mam_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mam_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mam_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mam_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tam_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tam_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tam_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tam_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tam_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tam_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mat_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mat_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mat_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mat_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mat_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mat_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tat_32f,   (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tat_64f,   (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tat_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tat_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tat_32f_L, (const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tat_64f_L, (const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2,  int src2Stride1,  int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mama_32f,  (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mama_64f,  (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mama_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mama_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mama_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mama_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tama_32f,  (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tama_64f,  (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tama_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tama_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tama_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tama_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mata_32f,  (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mata_64f,  (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mata_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mata_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_mata_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_mata_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tata_32f,  (const Ipp32f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp32f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp32f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tata_64f,  (const Ipp64f*  pSrc1, int src1Stride0, int src1Stride1, int src1Stride2,
                                      const Ipp64f*  pSrc2, int src2Stride0, int src2Stride1, int src2Stride2,
                                            Ipp64f*  pDst,  int dstStride0,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tata_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tata_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                            int width, int height, int count))

IPPAPI(IppStatus, ippmSub_tata_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp32f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))
IPPAPI(IppStatus, ippmSub_tata_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                      const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride1, int src2Stride2,
                                            Ipp64f** ppDst,  int dstRoiShift,  int dstStride1,  int dstStride2,
                                            int width, int height, int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmGaxpy_mv
//  Purpose:    performs the "gaxpy" operation on matrix
//  Parameters:
//    pSrc1, ppSrc1     pointer to the source matrix or array of matrices
//    src1Stride0       stride between the matrices in the source matrix array
//    src1Stride1       stride between the rows in the source matrix
//    src1Stride2       stride between the elements in the source matrix
//    src1RoiShift      ROI shift for the source matrix
//    src1Width         matrix width
//    src1Height        matrix height
//    pSrc2, ppSrc2     pointer to the first source vector or array of vectors
//    src2Stride0       stride between the vectors in the first source vector array
//    src2Stride2       stride between the elements in the first source vector
//    src2RoiShift      ROI shift for the first source vector
//    src2Len           length of the first source vector
//    pSrc3, ppSrc3     pointer to the second source vector or array of vectors
//    src3Stride0       stride between the vectors in the second source vector array
//    src3Stride2       stride between the elements in the second source vector
//    src3RoiShift      ROI shift for the second source vector
//    src3Len           length of the second source vector
//    pDst, ppDst       pointer to the destination vector or array of vectors
//    dstStride0        stride between the vectors in the destination array
//    dstStride2        stride between the elements in the destination vector
//    dstRoiShift       ROI shift for the destination vector
//    count             number of objects in the array
//  Return:
//    ippStsNullPtrErr         pointer(s) to the data is NULL
//    ippStsSizeErr            data size value is less or equal zero
//    ippStsSizeMatchMatrixErr unsuitable sizes of the source matrices
//    ippStsCountMatrixErr     count value is less or equal zero
//    ippStsStrideMatrixErr    stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr  RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr              otherwise
*/

IPPAPI(IppStatus, ippmGaxpy_mv_32f,   (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                             int src1Width,  int src1Height,
                                       const Ipp32f*  pSrc2, int src2Stride2, int src2Len,
                                       const Ipp32f*  pSrc3, int src3Stride2, int src3Len,
                                             Ipp32f*  pDst,  int dstStride2))
IPPAPI(IppStatus, ippmGaxpy_mv_64f,   (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                             int src1Width,  int src1Height,
                                       const Ipp64f*  pSrc2, int src2Stride2, int src2Len,
                                       const Ipp64f*  pSrc3, int src3Stride2, int src3Len,
                                             Ipp64f*  pDst,  int dstStride2))

IPPAPI(IppStatus, ippmGaxpy_mv_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                             int src1Width,   int src1Height,
                                       const Ipp32f** ppSrc2, int src2RoiShift, int src2Len,
                                       const Ipp32f** ppSrc3, int src3RoiShift, int src3Len,
                                             Ipp32f** ppDst,  int dstRoiShift))
IPPAPI(IppStatus, ippmGaxpy_mv_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                             int src1Width,   int src1Height,
                                       const Ipp64f** ppSrc2, int src2RoiShift, int src2Len,
                                       const Ipp64f** ppSrc3, int src3RoiShift, int src3Len,
                                             Ipp64f** ppDst,  int dstRoiShift))

IPPAPI(IppStatus, ippmGaxpy_mva_32f,   (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                             int src1Width,  int src1Height,
                                       const Ipp32f*  pSrc2, int src2Stride0, int src2Stride2, int src2Len,
                                       const Ipp32f*  pSrc3, int src3Stride0, int src3Stride2, int src3Len,
                                             Ipp32f*  pDst,  int dstStride0,  int dstStride2, int count))
IPPAPI(IppStatus, ippmGaxpy_mva_64f,  (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                             int src1Width,  int src1Height,
                                       const Ipp64f*  pSrc2, int src2Stride0, int src2Stride2, int src2Len,
                                       const Ipp64f*  pSrc3, int src3Stride0, int src3Stride2, int src3Len,
                                             Ipp64f*  pDst,  int dstStride0,  int dstStride2, int count))

IPPAPI(IppStatus, ippmGaxpy_mva_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift,
                                             int src1Width,   int src1Height,
                                       const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0, int src2Len,
                                       const Ipp32f** ppSrc3, int src3RoiShift, int src3Stride0, int src3Len,
                                             Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,  int count))
IPPAPI(IppStatus, ippmGaxpy_mva_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift,
                                             int src1Width,   int src1Height,
                                       const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0, int src2Len,
                                       const Ipp64f** ppSrc3, int src3RoiShift, int src3Stride0, int src3Len,
                                             Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,  int count))

IPPAPI(IppStatus, ippmGaxpy_mva_32f_L,(const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                             int src1Width,   int src1Height,
                                       const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2, int src2Len,
                                       const Ipp32f** ppSrc3, int src3RoiShift, int src3Stride2, int src3Len,
                                             Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,  int count))
IPPAPI(IppStatus, ippmGaxpy_mva_64f_L,(const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                             int src1Width,   int src1Height,
                                       const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2, int src2Len,
                                       const Ipp64f** ppSrc3, int src3RoiShift, int src3Stride2, int src3Len,
                                             Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,  int count))

IPPAPI(IppStatus, ippmGaxpy_mvav_32f, (const Ipp32f*  pSrc1, int src1Stride1, int src1Stride2,
                                             int src1Width,  int src1Height,
                                       const Ipp32f*  pSrc2, int src2Stride0, int src2Stride2, int src2Len,
                                       const Ipp32f*  pSrc3, int src3Stride2, int src3Len,
                                             Ipp32f*  pDst,  int dstStride0,  int dstStride2, int count))
IPPAPI(IppStatus, ippmGaxpy_mvav_64f, (const Ipp64f*  pSrc1, int src1Stride1, int src1Stride2,
                                             int src1Width,  int src1Height,
                                       const Ipp64f*  pSrc2, int src2Stride0, int src2Stride2, int src2Len,
                                       const Ipp64f*  pSrc3, int src3Stride2, int src3Len,
                                             Ipp64f*  pDst,  int dstStride0,  int dstStride2, int count))

IPPAPI(IppStatus, ippmGaxpy_mvav_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift,
                                              int src1Width,   int src1Height,
                                        const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0, int src2Len,
                                        const Ipp32f** ppSrc3, int src3RoiShift, int src3Len,
                                              Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,  int count))
IPPAPI(IppStatus, ippmGaxpy_mvav_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift,
                                              int src1Width,   int src1Height,
                                        const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0, int src2Len,
                                        const Ipp64f** ppSrc3, int src3RoiShift, int src3Len,
                                              Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,  int count))

IPPAPI(IppStatus, ippmGaxpy_mvav_32f_L,(const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                              int src1Width,   int src1Height,
                                        const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2, int src2Len,
                                        const Ipp32f*  pSrc3,  int src3Stride2,  int src3Len,
                                              Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,  int count))
IPPAPI(IppStatus, ippmGaxpy_mvav_64f_L,(const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                              int src1Width,   int src1Height,
                                        const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2, int src2Len,
                                        const Ipp64f*  pSrc3,  int src3Stride2,  int src3Len,
                                              Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,  int count))

/* /////////////////////////////////////////////////////////////////////////////
//                   Linear System Solution functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmLUDecomp
//  Purpose:    decomposes square matrix into product of upper and
//              lower triangular matrices
//  Parameters:
//    pSrc, ppSrc       pointer to the source matrix or array of matrices
//    srcStride0        stride between the matrices in the source array
//    srcStride1        stride between the rows in the source matrix
//    srcStride2        stride between the elements in the source matrix
//    srcRoiShift       ROI shift for the source matrix
//    pDstIndex         Pointer to array of pivot indices, where row i interchanges with row index(i).
//                      The array size must be more than or equal to widthHeight.
//                      If the operation is performed on an array of matrices
//                      the size of indices' array must be more than or equal to count*widthHeight.
//    pDst, ppDst       pointer to the destination matrix or array of matrices
//    dstStride0        stride between the matrices in the destination array
//    dstStride1        stride between the rows in the destination matrix
//    dstStride2        stride between the elements in the destination matrix
//    dstRoiShift       ROI shift for the destination matrix
//    widthHeight       size of the square matrix
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsSingularErr       matrix is singular
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmLUDecomp_m_32f,   (const Ipp32f*  pSrc,  int srcStride1, int srcStride2,
                                               int*     pDstIndex,
                                               Ipp32f*  pDst,  int dstStride1, int dstStride2,
                                               int widthHeight))
IPPAPI(IppStatus, ippmLUDecomp_m_64f,   (const Ipp64f*  pSrc,  int srcStride1, int srcStride2,
                                               int*     pDstIndex,
                                               Ipp64f*  pDst,  int dstStride1, int dstStride2,
                                               int widthHeight))

IPPAPI(IppStatus, ippmLUDecomp_m_32f_P, (const Ipp32f** ppSrc, int srcRoiShift,
                                               int*     pDstIndex,
                                               Ipp32f** ppDst, int dstRoiShift,
                                               int widthHeight))
IPPAPI(IppStatus, ippmLUDecomp_m_64f_P, (const Ipp64f** ppSrc, int srcRoiShift,
                                               int*     pDstIndex,
                                               Ipp64f** ppDst, int dstRoiShift,
                                               int widthHeight))

IPPAPI(IppStatus, ippmLUDecomp_ma_32f,  (const Ipp32f*  pSrc,   int srcStride0, int srcStride1, int srcStride2,
                                               int*     pDstIndex,
                                               Ipp32f*  pDst,   int dstStride0, int dstStride1, int dstStride2,
                                               int widthHeight, int count))
IPPAPI(IppStatus, ippmLUDecomp_ma_64f,  (const Ipp64f*  pSrc,   int srcStride0, int srcStride1, int srcStride2,
                                               int*     pDstIndex,
                                               Ipp64f*  pDst,   int dstStride0, int dstStride1, int dstStride2,
                                               int widthHeight, int count))

IPPAPI(IppStatus, ippmLUDecomp_ma_32f_P,(const Ipp32f** ppSrc,  int srcRoiShift, int srcStride0,
                                               int*     pDstIndex,
                                               Ipp32f** ppDst,  int dstRoiShift, int dstStride0,
                                               int widthHeight, int count))
IPPAPI(IppStatus, ippmLUDecomp_ma_64f_P,(const Ipp64f** ppSrc,  int srcRoiShift, int srcStride0,
                                               int*     pDstIndex,
                                               Ipp64f** ppDst,  int dstRoiShift, int dstStride0,
                                               int widthHeight, int count))

IPPAPI(IppStatus, ippmLUDecomp_ma_32f_L,(const Ipp32f** ppSrc,  int srcRoiShift, int srcStride1, int srcStride2,
                                               int*     pDstIndex,
                                               Ipp32f** ppDst,  int dstRoiShift, int dstStride1, int dstStride2,
                                               int widthHeight, int count))
IPPAPI(IppStatus, ippmLUDecomp_ma_64f_L,(const Ipp64f** ppSrc,  int srcRoiShift, int srcStride1, int srcStride2,
                                               int*     pDstIndex,
                                               Ipp64f** ppDst,  int dstRoiShift, int dstStride1, int dstStride2,
                                               int widthHeight, int count))

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippmLUBackSubst
//  Purpose:    solves system of linear equations with LU-factored
//              square matrix
//  Parameters:
//    pSrc1, ppSrc1     Pointer to the source matrix or array of matrices.
//                      Matrix must be a result of calling LUDecomp.
//    src1Stride0       stride between the matrices in the source array
//    src1Stride1       stride between the rows in the source matrix
//    src1Stride2       stride between the elements in the source matrix
//    src1RoiShift      ROI shift for the source matrix
//    pSrcIndex         Pointer to array of pivot indices.
//                      This array must be a result of calling LUDecomp.
//                      The array size must be more than or equal to widthHeight.
//                      If the operation is performed on an array of matrices
//                      the size of indices' array must be more than or equal to count*widthHeight.
//    pSrc2, ppSrc2     pointer to the source vector or array of vectors
//    src2Stride0       stride between the vectors in the source vector array
//    src2Stride2       stride between the elements in the source vector
//    src2RoiShift      ROI shift for the source vector
//    pDst, ppDst       pointer to the destination vector or array of vectors
//    dstStride0        stride between the vectors in the destination array
//    dstStride2        stride between the elements in the destination vector
//    dstRoiShift       ROI shift for the destination vector
//    widthHeight       size of the square matrix and vectors
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmLUBackSubst_mv_32f,    (const Ipp32f*  pSrc1,  int src1Stride1, int src1Stride2,
                                                    int* pSrcIndex,
                                              const Ipp32f*  pSrc2,  int src2Stride2,
                                                    Ipp32f*  pDst,   int dstStride2,
                                                    int widthHeight))
IPPAPI(IppStatus, ippmLUBackSubst_mv_64f,    (const Ipp64f*  pSrc1,  int src1Stride1, int src1Stride2,
                                                    int* pSrcIndex,
                                              const Ipp64f*  pSrc2,  int src2Stride2,
                                                    Ipp64f*  pDst,   int dstStride2,
                                                    int widthHeight))

IPPAPI(IppStatus, ippmLUBackSubst_mv_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                                    int* pSrcIndex,
                                              const Ipp32f** ppSrc2, int src2RoiShift,
                                                    Ipp32f** ppDst,  int dstRoiShift,
                                                    int widthHeight))
IPPAPI(IppStatus, ippmLUBackSubst_mv_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                                    int* pSrcIndex,
                                              const Ipp64f** ppSrc2, int src2RoiShift,
                                                    Ipp64f** ppDst,  int dstRoiShift,
                                                    int widthHeight))

IPPAPI(IppStatus, ippmLUBackSubst_mva_32f,   (const Ipp32f*  pSrc1,  int src1Stride1, int src1Stride2,
                                                    int* pSrcIndex,
                                              const Ipp32f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                    Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                                    int widthHeight, int count))
IPPAPI(IppStatus, ippmLUBackSubst_mva_64f,   (const Ipp64f*  pSrc1,  int src1Stride1, int src1Stride2,
                                                    int* pSrcIndex,
                                              const Ipp64f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                    Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                                    int widthHeight, int count))

IPPAPI(IppStatus, ippmLUBackSubst_mva_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                                    int* pSrcIndex,
                                              const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                    Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                                    int widthHeight, int count))
IPPAPI(IppStatus, ippmLUBackSubst_mva_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                                    int* pSrcIndex,
                                              const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                    Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                                    int widthHeight, int count))

IPPAPI(IppStatus, ippmLUBackSubst_mva_32f_L, (const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                                    int* pSrcIndex,
                                              const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                    Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                                    int widthHeight, int count))
IPPAPI(IppStatus, ippmLUBackSubst_mva_64f_L, (const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                                    int* pSrcIndex,
                                              const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                    Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                                    int widthHeight, int count))

IPPAPI(IppStatus, ippmLUBackSubst_mava_32f,  (const Ipp32f*  pSrc1,  int src1Stride0, int src1Stride1, int src1Stride2,
                                                    int* pSrcIndex,
                                              const Ipp32f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                    Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                                    int widthHeight, int count))
IPPAPI(IppStatus, ippmLUBackSubst_mava_64f,  (const Ipp64f*  pSrc1,  int src1Stride0, int src1Stride1, int src1Stride2,
                                                    int* pSrcIndex,
                                              const Ipp64f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                    Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                                    int widthHeight, int count))

IPPAPI(IppStatus, ippmLUBackSubst_mava_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                                    int* pSrcIndex,
                                              const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                    Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                                    int widthHeight, int count))
IPPAPI(IppStatus, ippmLUBackSubst_mava_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                                    int* pSrcIndex,
                                              const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                    Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                                    int widthHeight, int count))

IPPAPI(IppStatus, ippmLUBackSubst_mava_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                                    int* pSrcIndex,
                                              const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                    Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                                    int widthHeight, int count))
IPPAPI(IppStatus, ippmLUBackSubst_mava_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                                    int* pSrcIndex,
                                              const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                    Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                                    int widthHeight, int count))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmCholeskyDecomp
//  Purpose:    performs Cholesky decomposition of a symmetric
//              positive definite square matrix
//  Parameters:
//    pSrc, ppSrc       pointer to the source matrix or array of matrices
//    srcStride0        stride between the matrices in the source array
//    srcStride1        stride between the rows in the source matrix
//    srcStride2        stride between the elements in the source matrix
//    srcRoiShift       ROI shift for the source matrix
//    pDst, ppDst       pointer to the destination matrix or array of matrices
//    dstStride0        stride between the matrices in the destination array
//    dstStride1        stride between the rows in the destination matrix
//    dstStride2        stride between the elements in the destination matrix
//    dstRoiShift       ROI shift for the destination matrix
//    widthHeight       size of the square matrix
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNotPosDefErr      returns an error when the source matrix is not positive definite
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmCholeskyDecomp_m_32f,   (const Ipp32f*  pSrc,  int srcStride1, int srcStride2,
                                                     Ipp32f*  pDst,  int dstStride1, int dstStride2,
                                                     int widthHeight))
IPPAPI(IppStatus, ippmCholeskyDecomp_m_64f,   (const Ipp64f*  pSrc,  int srcStride1, int srcStride2,
                                                     Ipp64f*  pDst,  int dstStride1, int dstStride2,
                                                     int widthHeight))

IPPAPI(IppStatus, ippmCholeskyDecomp_m_32f_P, (const Ipp32f** ppSrc, int srcRoiShift,
                                                     Ipp32f** ppDst, int dstRoiShift,
                                                     int widthHeight))
IPPAPI(IppStatus, ippmCholeskyDecomp_m_64f_P, (const Ipp64f** ppSrc, int srcRoiShift,
                                                     Ipp64f** ppDst, int dstRoiShift,
                                                     int widthHeight))

IPPAPI(IppStatus, ippmCholeskyDecomp_ma_32f,  (const Ipp32f*  pSrc,   int srcStride0, int srcStride1, int srcStride2,
                                                     Ipp32f*  pDst,   int dstStride0, int dstStride1, int dstStride2,
                                                     int widthHeight, int count))
IPPAPI(IppStatus, ippmCholeskyDecomp_ma_64f,  (const Ipp64f*  pSrc,   int srcStride0, int srcStride1, int srcStride2,
                                                     Ipp64f*  pDst,   int dstStride0, int dstStride1, int dstStride2,
                                                     int widthHeight, int count))

IPPAPI(IppStatus, ippmCholeskyDecomp_ma_32f_P,(const Ipp32f** ppSrc,  int srcRoiShift, int srcStride0,
                                                     Ipp32f** ppDst,  int dstRoiShift, int dstStride0,
                                                     int widthHeight, int count))
IPPAPI(IppStatus, ippmCholeskyDecomp_ma_64f_P,(const Ipp64f** ppSrc,  int srcRoiShift, int srcStride0,
                                                     Ipp64f** ppDst,  int dstRoiShift, int dstStride0,
                                                     int widthHeight, int count))

IPPAPI(IppStatus, ippmCholeskyDecomp_ma_32f_L,(const Ipp32f** ppSrc,  int srcRoiShift, int srcStride1, int srcStride2,
                                                     Ipp32f** ppDst,  int dstRoiShift, int dstStride1, int dstStride2,
                                                     int widthHeight, int count))
IPPAPI(IppStatus, ippmCholeskyDecomp_ma_64f_L,(const Ipp64f** ppSrc,  int srcRoiShift, int srcStride1, int srcStride2,
                                                     Ipp64f** ppDst,  int dstRoiShift, int dstStride1, int dstStride2,
                                                     int widthHeight, int count))

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippmCholeskyBackSubst
//  Purpose:    solves system of linear equations using the Cholesky
//              triangular factor
//  Parameters:
//    pSrc1, ppSrc1     Pointer to the source matrix or array of matrices.
//                      Matrix must be a result of calling CholeskyDecomp.
//    src1Stride0       stride between the matrices in the source array
//    src1Stride1       stride between the rows in the source matrix
//    src1Stride2       stride between the elements in the source matrix
//    src1RoiShift      ROI shift for the source matrix
//    pSrc2, ppSrc2     pointer to the source vector or array of vectors
//    src2Stride0       stride between the vectors in the source vector array
//    src2Stride2       stride between the elements in the source vector
//    src2RoiShift      ROI shift for the source vector
//    pDst, ppDst       pointer to the destination vector or array of vectors
//    dstStride0        stride between the vectors in the destination array
//    dstStride2        stride between the elements in the destination vector
//    dstRoiShift       ROI shift for the destination vector
//    widthHeight       size of the square matrix and vectors
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/

IPPAPI(IppStatus, ippmCholeskyBackSubst_mv_32f,    (const Ipp32f*  pSrc1,  int src1Stride1, int src1Stride2,
                                                    const Ipp32f*  pSrc2,  int src2Stride2,
                                                          Ipp32f*  pDst,   int dstStride2,
                                                          int widthHeight))
IPPAPI(IppStatus, ippmCholeskyBackSubst_mv_64f,    (const Ipp64f*  pSrc1,  int src1Stride1, int src1Stride2,
                                                    const Ipp64f*  pSrc2,  int src2Stride2,
                                                          Ipp64f*  pDst,   int dstStride2,
                                                          int widthHeight))

IPPAPI(IppStatus, ippmCholeskyBackSubst_mv_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                                    const Ipp32f** ppSrc2, int src2RoiShift,
                                                          Ipp32f** ppDst,  int dstRoiShift,
                                                          int widthHeight))
IPPAPI(IppStatus, ippmCholeskyBackSubst_mv_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                                    const Ipp64f** ppSrc2, int src2RoiShift,
                                                          Ipp64f** ppDst,  int dstRoiShift,
                                                          int widthHeight))

IPPAPI(IppStatus, ippmCholeskyBackSubst_mva_32f,   (const Ipp32f*  pSrc1,  int src1Stride1, int src1Stride2,
                                                    const Ipp32f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                          Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                                          int widthHeight, int count))
IPPAPI(IppStatus, ippmCholeskyBackSubst_mva_64f,   (const Ipp64f*  pSrc1,  int src1Stride1, int src1Stride2,
                                                    const Ipp64f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                          Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                                          int widthHeight, int count))

IPPAPI(IppStatus, ippmCholeskyBackSubst_mva_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                                    const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                          Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                                          int widthHeight, int count))
IPPAPI(IppStatus, ippmCholeskyBackSubst_mva_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                                    const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                          Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                                          int widthHeight, int count))

IPPAPI(IppStatus, ippmCholeskyBackSubst_mva_32f_L, (const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                                    const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                          Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                                          int widthHeight, int count))
IPPAPI(IppStatus, ippmCholeskyBackSubst_mva_64f_L, (const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                                    const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                          Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                                          int widthHeight, int count))

IPPAPI(IppStatus, ippmCholeskyBackSubst_mava_32f,  (const Ipp32f*  pSrc1,  int src1Stride0, int src1Stride1, int src1Stride2,
                                                    const Ipp32f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                          Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                                          int widthHeight, int count))
IPPAPI(IppStatus, ippmCholeskyBackSubst_mava_64f,  (const Ipp64f*  pSrc1,  int src1Stride0, int src1Stride1, int src1Stride2,
                                                    const Ipp64f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                          Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                                          int widthHeight, int count))

IPPAPI(IppStatus, ippmCholeskyBackSubst_mava_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                                    const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                          Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                                          int widthHeight, int count))
IPPAPI(IppStatus, ippmCholeskyBackSubst_mava_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                                    const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                          Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                                          int widthHeight, int count))

IPPAPI(IppStatus, ippmCholeskyBackSubst_mava_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                                    const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                          Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                                          int widthHeight, int count))
IPPAPI(IppStatus, ippmCholeskyBackSubst_mava_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                                    const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                          Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                                          int widthHeight, int count))

/* /////////////////////////////////////////////////////////////////////////////
//                  Least  Squares Problem functions
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippmQRDecomp
//  Purpose:    Computes the QR decomposition for the given matrix
//  Parameters:
//    pSrc, ppSrc       pointer to the source matrix or array of matrices
//    srcStride0        stride between the matrices in the source array
//    srcStride1        stride between the rows in the source matrix
//    srcStride2        stride between the elements in the source matrix
//    srcRoiShift       ROI shift for the source matrix
//    pBuffer           pointer to auxiliary array, the size of the array must be
//                      more than or equal to height
//    pDst, ppDst       pointer to the destination matrix or array of matrices
//    dstStride0        stride between the matrices in the destination array
//    dstStride1        stride between the rows in the destination matrix
//    dstStride2        stride between the elements in the destination matrix
//    dstRoiShift       ROI shift for the destination matrix
//    width             matrix width
//    height            matrix height
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr         pointer(s) to the data is NULL
//    ippStsSizeErr            data size value is less or equal zero
//    ippStsSizeMatchMatrixErr unsuitable sizes of the source matrices
//    ippStsCountMatrixErr     count value is less or equal zero
//    ippStsStrideMatrixErr    stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr  RoiShift value is negative or not dividend to size of data type
//    ippStsDivByZeroErr       returns an error when the source matrix has an incomplete column rank
//    ippStsNoErr              otherwise
*/

IPPAPI(IppStatus, ippmQRDecomp_m_32f,   (const Ipp32f*  pSrc,  int srcStride1, int srcStride2,
                                               Ipp32f*  pBuffer,
                                               Ipp32f*  pDst,  int dstStride1, int dstStride2,
                                               int width, int height))
IPPAPI(IppStatus, ippmQRDecomp_m_64f,   (const Ipp64f*  pSrc,  int srcStride1, int srcStride2,
                                               Ipp64f*  pBuffer,
                                               Ipp64f*  pDst,  int dstStride1, int dstStride2,
                                               int width, int height))

IPPAPI(IppStatus, ippmQRDecomp_m_32f_P, (const Ipp32f** ppSrc, int srcRoiShift,
                                               Ipp32f*  pBuffer,
                                               Ipp32f** ppDst, int dstRoiShift,
                                               int width, int height))
IPPAPI(IppStatus, ippmQRDecomp_m_64f_P, (const Ipp64f** ppSrc, int srcRoiShift,
                                               Ipp64f*  pBuffer,
                                               Ipp64f** ppDst, int dstRoiShift,
                                               int width, int height))

IPPAPI(IppStatus, ippmQRDecomp_ma_32f,  (const Ipp32f*  pSrc,   int srcStride0, int srcStride1, int srcStride2,
                                               Ipp32f*  pBuffer,
                                               Ipp32f*  pDst,   int dstStride0, int dstStride1, int dstStride2,
                                               int width, int height, int count))
IPPAPI(IppStatus, ippmQRDecomp_ma_64f,  (const Ipp64f*  pSrc,   int srcStride0, int srcStride1, int srcStride2,
                                               Ipp64f*  pBuffer,
                                               Ipp64f*  pDst,   int dstStride0, int dstStride1, int dstStride2,
                                               int width, int height, int count))

IPPAPI(IppStatus, ippmQRDecomp_ma_32f_P,(const Ipp32f** ppSrc,  int srcRoiShift, int srcStride0,
                                               Ipp32f*  pBuffer,
                                               Ipp32f** ppDst,  int dstRoiShift, int dstStride0,
                                               int width, int height, int count))
IPPAPI(IppStatus, ippmQRDecomp_ma_64f_P,(const Ipp64f** ppSrc,  int srcRoiShift, int srcStride0,
                                               Ipp64f*  pBuffer,
                                               Ipp64f** ppDst,  int dstRoiShift, int dstStride0,
                                               int width, int height, int count))

IPPAPI(IppStatus, ippmQRDecomp_ma_32f_L,(const Ipp32f** ppSrc,  int srcRoiShift, int srcStride1, int srcStride2,
                                               Ipp32f*  pBuffer,
                                               Ipp32f** ppDst,  int dstRoiShift, int dstStride1, int dstStride2,
                                               int width, int height, int count))
IPPAPI(IppStatus, ippmQRDecomp_ma_64f_L,(const Ipp64f** ppSrc,  int srcRoiShift, int srcStride1, int srcStride2,
                                               Ipp64f*  pBuffer,
                                               Ipp64f** ppDst,  int dstRoiShift, int dstStride1, int dstStride2,
                                               int width, int height, int count))

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippmQRBackSubst
//  Purpose:    Solves least squares problem for QR-decomposed matrix
//  Parameters:
//    pSrc1, ppSrc1     pointer to the source matrix or array of matrices,
//                      matrix must be a result of calling QRDecomp
//    src1Stride0       stride between the matrices in the source array
//    src1Stride1       stride between the rows in the source matrix
//    src1Stride2       stride between the elements in the source matrix
//    src1RoiShift      ROI shift for the source matrix
//    pBuffer           pointer to auxiliary array, the size of the array must be
//                      more than or equal to height
//    pSrc2, ppSrc2     pointer to the source vector or array of vectors
//    src2Stride0       stride between the vectors in the source vector array
//    src2Stride2       stride between the elements in the source vector
//    src2RoiShift      ROI shift for the source vector
//    pDst, ppDst       pointer to the destination vector or array of vectors
//    dstStride0        stride between the vectors in the destination array
//    dstStride2        stride between the elements in the destination vector
//    dstRoiShift       ROI shift for the destination vector
//    width             matrix width
//    height            matrix height
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr         pointer(s) to the data is NULL
//    ippStsSizeErr            data size value is less or equal zero
//    ippStsSizeMatchMatrixErr unsuitable sizes of the source matrices
//    ippStsCountMatrixErr     count value is less or equal zero
//    ippStsStrideMatrixErr    stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr  RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr              otherwise
*/

IPPAPI(IppStatus, ippmQRBackSubst_mv_32f,    (const Ipp32f*  pSrc1,  int src1Stride1, int src1Stride2,
                                                    Ipp32f*  pBuffer,
                                              const Ipp32f*  pSrc2,  int src2Stride2,
                                                    Ipp32f*  pDst,   int dstStride2,
                                                    int width, int height))
IPPAPI(IppStatus, ippmQRBackSubst_mv_64f,    (const Ipp64f*  pSrc1,  int src1Stride1, int src1Stride2,
                                                    Ipp64f*  pBuffer,
                                              const Ipp64f*  pSrc2,  int src2Stride2,
                                                    Ipp64f*  pDst,   int dstStride2,
                                                    int width, int height))

IPPAPI(IppStatus, ippmQRBackSubst_mv_32f_P,  (const Ipp32f** ppSrc1, int src1RoiShift,
                                                    Ipp32f*  pBuffer,
                                              const Ipp32f** ppSrc2, int src2RoiShift,
                                                    Ipp32f** ppDst,  int dstRoiShift,
                                                    int width, int height))
IPPAPI(IppStatus, ippmQRBackSubst_mv_64f_P,  (const Ipp64f** ppSrc1, int src1RoiShift,
                                                    Ipp64f*  pBuffer,
                                              const Ipp64f** ppSrc2, int src2RoiShift,
                                                    Ipp64f** ppDst,  int dstRoiShift,
                                                    int width, int height))

IPPAPI(IppStatus, ippmQRBackSubst_mva_32f,   (const Ipp32f*  pSrc1,  int src1Stride1, int src1Stride2,
                                                    Ipp32f*  pBuffer,
                                              const Ipp32f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                    Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                                    int width, int height, int count))
IPPAPI(IppStatus, ippmQRBackSubst_mva_64f,   (const Ipp64f*  pSrc1,  int src1Stride1, int src1Stride2,
                                                    Ipp64f*  pBuffer,
                                              const Ipp64f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                    Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                                    int width, int height, int count))

IPPAPI(IppStatus, ippmQRBackSubst_mva_32f_P, (const Ipp32f** ppSrc1, int src1RoiShift,
                                                    Ipp32f*  pBuffer,
                                              const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                    Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                                    int width, int height, int count))
IPPAPI(IppStatus, ippmQRBackSubst_mva_64f_P, (const Ipp64f** ppSrc1, int src1RoiShift,
                                                    Ipp64f*  pBuffer,
                                              const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                    Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                                    int width, int height, int count))

IPPAPI(IppStatus, ippmQRBackSubst_mva_32f_L, (const Ipp32f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                                    Ipp32f*  pBuffer,
                                              const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                    Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                                    int width, int height, int count))
IPPAPI(IppStatus, ippmQRBackSubst_mva_64f_L, (const Ipp64f*  pSrc1,  int src1Stride1,  int src1Stride2,
                                                    Ipp64f*  pBuffer,
                                              const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                    Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                                    int width, int height, int count))

IPPAPI(IppStatus, ippmQRBackSubst_mava_32f,  (const Ipp32f*  pSrc1,  int src1Stride0, int src1Stride1, int src1Stride2,
                                                    Ipp32f*  pBuffer,
                                              const Ipp32f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                    Ipp32f*  pDst,   int dstStride0,  int dstStride2,
                                                    int width, int height, int count))
IPPAPI(IppStatus, ippmQRBackSubst_mava_64f,  (const Ipp64f*  pSrc1,  int src1Stride0, int src1Stride1, int src1Stride2,
                                                    Ipp64f*  pBuffer,
                                              const Ipp64f*  pSrc2,  int src2Stride0, int src2Stride2,
                                                    Ipp64f*  pDst,   int dstStride0,  int dstStride2,
                                                    int width, int height, int count))

IPPAPI(IppStatus, ippmQRBackSubst_mava_32f_P,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride0,
                                                    Ipp32f*  pBuffer,
                                              const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                    Ipp32f** ppDst,  int dstRoiShift,  int dstStride0,
                                                    int width, int height, int count))
IPPAPI(IppStatus, ippmQRBackSubst_mava_64f_P,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride0,
                                                    Ipp64f*  pBuffer,
                                              const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride0,
                                                    Ipp64f** ppDst,  int dstRoiShift,  int dstStride0,
                                                    int width, int height, int count))

IPPAPI(IppStatus, ippmQRBackSubst_mava_32f_L,(const Ipp32f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                                    Ipp32f*  pBuffer,
                                              const Ipp32f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                    Ipp32f** ppDst,  int dstRoiShift,  int dstStride2,
                                                    int width, int height, int count))
IPPAPI(IppStatus, ippmQRBackSubst_mava_64f_L,(const Ipp64f** ppSrc1, int src1RoiShift, int src1Stride1, int src1Stride2,
                                                    Ipp64f*  pBuffer,
                                              const Ipp64f** ppSrc2, int src2RoiShift, int src2Stride2,
                                                    Ipp64f** ppDst,  int dstRoiShift,  int dstStride2,
                                                    int width, int height, int count))

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippmEigenValuesVectorsSym
//  Purpose:    Find eigenvalues and eigenvectors for real symmetric matrices
//  Parameters:
//    pSrc1, ppSrc1     pointer to the source matrix or array of matrices
//    src1Stride0       stride between the matrices in the source array
//    src1Stride1       stride between the rows in the source matrix
//    src1Stride2       stride between the elements in the source matrix
//    src1RoiShift      ROI shift for the source matrix
//    pBuffer           pointer to auxiliary array, the size of the array must be
//                      more than or equal to widthHeight*widthHeight
//    pDstVectors, ppDstVectors  pointer to the destination matrix or array of matrices
//                               which columns are eigenvectors
//    dstStride0        stride between the matrices in the destination array
//    dstStride1        stride between the rows in the destination matrix
//    dstStride2        stride between the elements in the destination matrix
//    dstRoiShift       ROI shift for the destination matrix
//    pDstValues        pointer to the destination dense array of the eigenvalues
//    widthHeight       square matrix size
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/
IPPAPI(IppStatus, ippmEigenValuesVectorsSym_m_32f,  (const Ipp32f* pSrc, int srcStride1, int srcStride2,
                                                           Ipp32f* pBuffer,
                                                           Ipp32f* pDstVectors, int dstStride1, int dstStride2,
                                                           Ipp32f* pDstValues,  int widthHeight))
IPPAPI(IppStatus, ippmEigenValuesVectorsSym_m_64f,  (const Ipp64f* pSrc, int srcStride1, int srcStride2,
                                                           Ipp64f* pBuffer,
                                                           Ipp64f* pDstVectors, int dstStride1, int dstStride2,
                                                           Ipp64f* pDstValues,  int widthHeight))

IPPAPI(IppStatus, ippmEigenValuesVectorsSym_m_32f_P,(const Ipp32f** ppSrc, int srcRoiShift,
                                                           Ipp32f*  pBuffer,
                                                           Ipp32f** ppDstVectors, int dstRoiShift,
                                                           Ipp32f*  pDstValues,   int widthHeight))
IPPAPI(IppStatus, ippmEigenValuesVectorsSym_m_64f_P,(const Ipp64f** ppSrc, int srcRoiShift,
                                                           Ipp64f*  pBuffer,
                                                           Ipp64f** ppDstVectors, int dstRoiShift,
                                                           Ipp64f*  pDstValues,   int widthHeight))

IPPAPI(IppStatus, ippmEigenValuesVectorsSym_ma_32f,  (const Ipp32f* pSrc, int srcStride0, int srcStride1, int srcStride2,
                                                            Ipp32f* pBuffer,
                                                            Ipp32f* pDstVectors, int dstStride0, int dstStride1, int dstStride2,
                                                            Ipp32f* pDstValues,  int widthHeight, int count))
IPPAPI(IppStatus, ippmEigenValuesVectorsSym_ma_64f,  (const Ipp64f* pSrc, int srcStride0, int srcStride1, int srcStride2,
                                                            Ipp64f* pBuffer,
                                                            Ipp64f* pDstVectors, int dstStride0, int dstStride1, int dstStride2,
                                                            Ipp64f* pDstValues,  int widthHeight, int count))

IPPAPI(IppStatus, ippmEigenValuesVectorsSym_ma_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                                            Ipp32f*  pBuffer,
                                                            Ipp32f** ppDstVectors, int dstRoiShift, int dstStride0,
                                                            Ipp32f*  pDstValues,   int widthHeight, int count))
IPPAPI(IppStatus, ippmEigenValuesVectorsSym_ma_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                                            Ipp64f*  pBuffer,
                                                            Ipp64f** ppDstVectors, int dstRoiShift, int dstStride0,
                                                            Ipp64f*  pDstValues,   int widthHeight, int count))

IPPAPI(IppStatus, ippmEigenValuesVectorsSym_ma_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                                            Ipp32f*  pBuffer,
                                                            Ipp32f** ppDstVectors, int dstRoiShift, int dstStride1, int dstStride2,
                                                            Ipp32f*  pDstValues,   int widthHeight, int count))
IPPAPI(IppStatus, ippmEigenValuesVectorsSym_ma_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                                            Ipp64f*  pBuffer,
                                                            Ipp64f** ppDstVectors, int dstRoiShift, int dstStride1, int dstStride2,
                                                            Ipp64f*  pDstValues,   int widthHeight, int count))

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippmEigenValuesSym
//  Purpose:    Find eigenvalues for real symmetric matrices
//  Parameters:
//    pSrc1, ppSrc1     pointer to the source matrix or array of matrices
//    src1Stride0       stride between the matrices in the source array
//    src1Stride1       stride between the rows in the source matrix
//    src1Stride2       stride between the elements in the source matrix
//    src1RoiShift      ROI shift for the source matrix
//    pBuffer           pointer to auxiliary array, the size of the array must be
//                      more than or equal to widthHeight*widthHeight
//    pDstValues        pointer to the destination dense array of the eigenvalues
//    widthHeight       square matrix size
//    count             number of matrices in the array
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsNoErr             otherwise
*/
IPPAPI(IppStatus, ippmEigenValuesSym_m_32f,  (const Ipp32f* pSrc, int srcStride1, int srcStride2,
                                                    Ipp32f* pBuffer,
                                                    Ipp32f* pDstValues, int widthHeight))
IPPAPI(IppStatus, ippmEigenValuesSym_m_64f,  (const Ipp64f* pSrc, int srcStride1, int srcStride2,
                                                    Ipp64f* pBuffer,
                                                    Ipp64f* pDstValues, int widthHeight))

IPPAPI(IppStatus, ippmEigenValuesSym_m_32f_P,(const Ipp32f** ppSrc, int srcRoiShift,
                                                    Ipp32f*  pBuffer,
                                                    Ipp32f*  pDstValues, int widthHeight))
IPPAPI(IppStatus, ippmEigenValuesSym_m_64f_P,(const Ipp64f** ppSrc, int srcRoiShift,
                                                    Ipp64f*  pBuffer,
                                                    Ipp64f*  pDstValues, int widthHeight))

IPPAPI(IppStatus, ippmEigenValuesSym_ma_32f,  (const Ipp32f* pSrc, int srcStride0, int srcStride1, int srcStride2,
                                                     Ipp32f* pBuffer,
                                                     Ipp32f* pDstValues, int widthHeight, int count))
IPPAPI(IppStatus, ippmEigenValuesSym_ma_64f,  (const Ipp64f* pSrc, int srcStride0, int srcStride1, int srcStride2,
                                                     Ipp64f* pBuffer,
                                                     Ipp64f* pDstValues, int widthHeight, int count))

IPPAPI(IppStatus, ippmEigenValuesSym_ma_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                                     Ipp32f*  pBuffer,
                                                     Ipp32f*  pDstValues, int widthHeight, int count))
IPPAPI(IppStatus, ippmEigenValuesSym_ma_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                                     Ipp64f*  pBuffer,
                                                     Ipp64f*  pDstValues, int widthHeight, int count))

IPPAPI(IppStatus, ippmEigenValuesSym_ma_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                                     Ipp32f*  pBuffer,
                                                     Ipp32f*  pDstValues, int widthHeight, int count))
IPPAPI(IppStatus, ippmEigenValuesSym_ma_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                                     Ipp64f*  pBuffer,
                                                     Ipp64f*  pDstValues, int widthHeight, int count))

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippmEigenValuesVectors
//  Purpose:    Find eigenvalues, the right eigenvectors and (or) the left eigenvectors
//              for real general (nonsymmetrical) matrices
//  Parameters:
//    pSrc, ppSrc       pointer to the source matrix or array of matrices
//    srcStride0        stride between the matrices in the source array
//    srcStride1        stride between the rows in the source matrix
//    srcStride2        stride between the elements in the source matrix
//    srcRoiShift       ROI shift for the source matrix
//    pDstVectorsRight, ppDstVectorsRight  pointer to the destination matrix
//                      or array of matrices which columns are right eigenvectors
//    dstRightStride0   stride between the matrices in the destination array of the right eigenvectors
//    dstRightStride1   stride between the rows in the destination matrix of the right eigenvectors
//    dstRightStride2   stride between the elements in the destination matrix of the right eigenvectors
//    dstRightRoiShift  ROI shift for the destination matrix of the right eigenvectors
//    pDstVectorsLeft, ppDstVectorsLeft  pointer to the destination matrix
//                      or array of matrices which columns are Left eigenvectors
//    dstLeftStride0    stride between the matrices in the destination array of the Left eigenvectors
//    dstLeftStride1    stride between the rows in the destination matrix of the Left eigenvectors
//    dstLeftStride2    stride between the elements in the destination matrix of the Left eigenvectors
//    dstLeftRoiShift   ROI shift for the destination matrix of the Left eigenvectors
//    pDstValuesRe      pointer to the destination specified dense array which contains
//                      real parts of eigenvalues
//    pDstValuesIm      pointer to the destination specified dense array which contains
//                      imaginary parts of eigenvalues
//    widthHeight       square matrix size
//    count             number of matrices in the array
//    pBuffer           pointer to the allocated buffer with the specified size for internal
//                      computations. Buffer size should be computed by the function
//                      ippmEigenValuesVectorsBufSize

//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsSingularErr       any of the input matrices is singular
//    ippStsConvergeErr       algorithm does not converge
//    ippStsNoErr             otherwise
*/
IPPAPI(IppStatus, ippmEigenValuesVectors_m_32f,
    (const Ipp32f* pSrc, int srcStride1, int srcStride2,
           Ipp32f* pDstVectorsRight, int dstRightStride1, int dstRightStride2,
           Ipp32f* pDstVectorsLeft,  int dstLeftStride1,  int dstLeftStride2,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectors_m_64f,
    (const Ipp64f* pSrc, int srcStride1, int srcStride2,
           Ipp64f* pDstVectorsRight, int dstRightStride1, int dstRightStride2,
           Ipp64f* pDstVectorsLeft,  int dstLeftStride1,  int dstLeftStride2,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectors_m_32f_P,
    (const Ipp32f** ppSrc, int srcRoiShift,
           Ipp32f** ppDstVectorsRight, int dstRightRoiShift,
           Ipp32f** ppDstVectorsLeft,  int dstLeftRoiShift,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectors_m_64f_P,
     (const Ipp64f** ppSrc, int srcRoiShift,
            Ipp64f** ppDstVectorsRight, int dstRightRoiShift,
            Ipp64f** ppDstVectorsLeft,  int dstLeftRoiShift,
            Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
            int widthHeight, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectorsRight_m_32f,
    (const Ipp32f* pSrc, int srcStride1, int srcStride2,
           Ipp32f* pDstVectorsRight, int dstRightStride1, int dstRightStride2,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectorsRight_m_64f,
    (const Ipp64f* pSrc, int srcStride1, int srcStride2,
           Ipp64f* pDstVectorsRight, int dstRightStride1, int dstRightStride2,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectorsRight_m_32f_P,
    (const Ipp32f** ppSrc, int srcRoiShift,
           Ipp32f** ppDstVectorsRight, int dstRightRoiShift,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectorsRight_m_64f_P,
    (const Ipp64f** ppSrc, int srcRoiShift,
           Ipp64f** ppDstVectorsRight, int dstRightRoiShift,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectorsLeft_m_32f,
    (const Ipp32f* pSrc, int srcStride1, int srcStride2,
           Ipp32f* pDstVectorsLeft, int dstLeftStride1, int dstLeftStride2,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectorsLeft_m_64f,
    (const Ipp64f* pSrc, int srcStride1, int srcStride2,
           Ipp64f* pDstVectorsLeft, int dstLeftStride1, int dstLeftStride2,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectorsLeft_m_32f_P,
    (const Ipp32f** ppSrc, int srcRoiShift,
           Ipp32f** ppDstVectorsLeft, int dstLeftRoiShift,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectorsLeft_m_64f_P,
    (const Ipp64f** ppSrc, int srcRoiShift,
           Ipp64f** ppDstVectorsLeft, int dstLeftRoiShift,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectors_ma_32f,
    (const Ipp32f* pSrc, int srcStride0, int srcStride1, int srcStride2,
           Ipp32f* pDstVectorsRight, int dstRightStride0, int dstRightStride1, int dstRightStride2,
           Ipp32f* pDstVectorsLeft,  int dstLeftStride0,  int dstLeftStride1,  int dstLeftStride2,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectors_ma_64f,
    (const Ipp64f* pSrc, int srcStride0, int srcStride1, int srcStride2,
           Ipp64f* pDstVectorsRight, int dstRightStride0, int dstRightStride1, int dstRightStride2,
           Ipp64f* pDstVectorsLeft,  int dstLeftStride0,  int dstLeftStride1,  int dstLeftStride2,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectors_ma_32f_P,
    (const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
           Ipp32f** ppDstVectorsRight, int dstRightRoiShift, int dstRightStride0,
           Ipp32f** ppDstVectorsLeft,  int dstLeftRoiShift,  int dstLeftStride0,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectors_ma_64f_P,
    (const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
           Ipp64f** ppDstVectorsRight, int dstRightRoiShift, int dstRightStride0,
           Ipp64f** ppDstVectorsLeft,  int dstLeftRoiShift,  int dstLeftStride0,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectors_ma_32f_L,
    (const Ipp32f** ppSrc, int srcRoiShift, int srcStride1,  int srcStride2,
           Ipp32f** ppDstVectorsRight, int dstRightRoiShift, int dstRightStride1, int dstRightStride2,
           Ipp32f** ppDstVectorsLeft,  int dstLeftRoiShift,  int dstLeftStride1,  int dstLeftStride2,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectors_ma_64f_L,
    (const Ipp64f** ppSrc, int srcRoiShift, int srcStride1,  int srcStride2,
           Ipp64f** ppDstVectorsRight, int dstRightRoiShift, int dstRightStride1, int dstRightStride2,
           Ipp64f** ppDstVectorsLeft,  int dstLeftRoiShift,  int dstLeftStride1,  int dstLeftStride2,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectorsRight_ma_32f,
    (const Ipp32f* pSrc, int srcStride0, int srcStride1, int srcStride2,
           Ipp32f* pDstVectorsRight, int dstRightStride0, int dstRightStride1, int dstRightStride2,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectorsRight_ma_64f,
    (const Ipp64f* pSrc, int srcStride0, int srcStride1, int srcStride2,
           Ipp64f* pDstVectorsRight, int dstRightStride0, int dstRightStride1, int dstRightStride2,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectorsRight_ma_32f_P,
    (const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
           Ipp32f** ppDstVectorsRight, int dstRightRoiShift, int dstRightStride0,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectorsRight_ma_64f_P,
    (const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
           Ipp64f** ppDstVectorsRight, int dstRightRoiShift, int dstRightStride0,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectorsRight_ma_32f_L,
    (const Ipp32f** ppSrc, int srcRoiShift, int srcStride1,  int srcStride2,
           Ipp32f** ppDstVectorsRight, int dstRightRoiShift, int dstRightStride1, int dstRightStride2,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectorsRight_ma_64f_L,
    (const Ipp64f** ppSrc, int srcRoiShift, int srcStride1,  int srcStride2,
           Ipp64f** ppDstVectorsRight, int dstRightRoiShift, int dstRightStride1, int dstRightStride2,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectorsLeft_ma_32f,
    (const Ipp32f* pSrc, int srcStride0, int srcStride1, int srcStride2,
           Ipp32f* pDstVectorsLeft,  int dstLeftStride0,  int dstLeftStride1,  int dstLeftStride2,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectorsLeft_ma_64f,
    (const Ipp64f* pSrc, int srcStride0, int srcStride1, int srcStride2,
           Ipp64f* pDstVectorsLeft,  int dstLeftStride0,  int dstLeftStride1,  int dstLeftStride2,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectorsLeft_ma_32f_P,
    (const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
           Ipp32f** ppDstVectorsLeft,  int dstLeftRoiShift,  int dstLeftStride0,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectorsLeft_ma_64f_P,
    (const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
           Ipp64f** ppDstVectorsLeft,  int dstLeftRoiShift,  int dstLeftStride0,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValuesVectorsLeft_ma_32f_L,
    (const Ipp32f** ppSrc, int srcRoiShift, int srcStride1,  int srcStride2,
           Ipp32f** ppDstVectorsLeft,  int dstLeftRoiShift,  int dstLeftStride1,  int dstLeftStride2,
           Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValuesVectorsLeft_ma_64f_L,
    (const Ipp64f** ppSrc, int srcRoiShift, int srcStride1,  int srcStride2,
           Ipp64f** ppDstVectorsLeft,  int dstLeftRoiShift,  int dstLeftStride1,  int dstLeftStride2,
           Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
           int widthHeight, int count, Ipp8u* pBuffer))
/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippmEigenValuesVectors
//  Purpose:    Find eigenvalues for real general (nonsymmetrical) matrices
//  Parameters:
//    pSrc, ppSrc       pointer to the source matrix or array of matrices
//    srcStride0        stride between the matrices in the source array
//    srcStride1        stride between the rows in the source matrix
//    srcStride2        stride between the elements in the source matrix
//    srcRoiShift       ROI shift for the source matrix
//    pDstValuesRe      pointer to the destination specified dense array which contains
//                      real parts of eigenvalues
//    pDstValuesIm      pointer to the destination specified dense array which contains
//                      imaginary parts of eigenvalues
//    widthHeight       square matrix size
//    count             number of matrices in the array
//    pBuffer           pointer to the allocated buffer with the specified size for internal
//                      computations. Buffer size should be computed by the function
//                      ippmEigenValuesBufSize

//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsRoiShiftMatrixErr RoiShift value is negative or not dividend to size of data type
//    ippStsSingularErr       any of the input matrices is singular
//    ippStsConvergeErr       algorithm does not converge
//    ippStsNoErr             otherwise
*/
IPPAPI(IppStatus, ippmEigenValues_m_32f,(const Ipp32f* pSrc, int srcStride1, int srcStride2,
                                               Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
                                               int widthHeight, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValues_m_64f,(const Ipp64f* pSrc, int srcStride1, int srcStride2,
                                               Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
                                               int widthHeight, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValues_m_32f_P,(const Ipp32f** ppSrc, int srcRoiShift,
                                                 Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
                                                 int widthHeight, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValues_m_64f_P,(const Ipp64f** ppSrc, int srcRoiShift,
                                                 Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
                                                 int widthHeight, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValues_ma_32f,(const Ipp32f* pSrc, int srcStride0, int srcStride1, int srcStride2,
                                                Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
                                                int widthHeight, int count, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValues_ma_64f,(const Ipp64f* pSrc, int srcStride0, int srcStride1, int srcStride2,
                                                Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
                                                int widthHeight, int count, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValues_ma_32f_P,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride0,
                                                  Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
                                                  int widthHeight, int count, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValues_ma_64f_P,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride0,
                                                  Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
                                                  int widthHeight, int count, Ipp8u* pBuffer))

IPPAPI(IppStatus, ippmEigenValues_ma_32f_L,(const Ipp32f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                                  Ipp32f* pDstValuesRe, Ipp32f* pDstValuesIm,
                                                  int widthHeight, int count, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippmEigenValues_ma_64f_L,(const Ipp64f** ppSrc, int srcRoiShift, int srcStride1, int srcStride2,
                                                  Ipp64f* pDstValuesRe, Ipp64f* pDstValuesIm,
                                                  int widthHeight, int count, Ipp8u* pBuffer))
/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippmEigenValuesVectorsGetBufSize & ippmEigenValuesGetBufSize
//  Purpose:    Computes the work buffer size for functions EigenValuesVectors and EigenValues
//
//  Parameters:
//    widthHeight       square matrix size
//    pSizeBytes        Pointer to the work buffer size value in bytes
//
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsSizeErr           data size value is less or equal zero
//    ippStsNoErr             otherwise
*/
IPPAPI(IppStatus, ippmEigenValuesVectorsGetBufSize_32f,(int widthHeight, int* pSizeBytes))

IPPAPI(IppStatus, ippmEigenValuesVectorsGetBufSize_64f,(int widthHeight, int* pSizeBytes))

IPPAPI(IppStatus, ippmEigenValuesGetBufSize_32f, (int widthHeight, int* pSizeBytes))

IPPAPI(IppStatus, ippmEigenValuesGetBufSize_64f, (int widthHeight, int* pSizeBytes))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippmAffineTransform3DH_mva
//  Purpose:    Performs arbitrary affine transformation on array of 3D vectors
//              in Homogeneous coordinate space
//
//  Parameters:
//    pSrc1                 Pointer to the source matrix 4x4
//    src1Stride1           Stride between the rows in the source matrix.
//    src1Stride2           Stride between the elements in the source matrix.
//    pSrc2                 Pointer to the source array of 3D vectors
//    src2Stride0           Stride between the vectors in the source 3D vector array
//    src2Stride2           Stride between the elements in the source 3D vector
//    pDst                  Pointer to the destination array of 3D vectors
//    dstStride0            Stride between the 3D vectors in the destination array
//    dstStride2            Stride between the elements in the destination 3D vector
//    count                 Number of 3D vectors in the array
//
//  Return:
//    ippStsNullPtrErr        pointer(s) to the data is NULL
//    ippStsCountMatrixErr    count value is less or equal zero
//    ippStsStrideMatrixErr   stride value is not positive or not dividend to size of data type
//    ippStsDivByZeroErr      W is equal to 0 for one 3D vector at least.
//    ippStsNoErr             otherwise
*/
IPPAPI(IppStatus, ippmAffineTransform3DH_mva_32f,(const Ipp32f* pSrc1, int src1Stride1, int src1Stride2,
                                                  const Ipp32f* pSrc2, int src2Stride0, int src2Stride2,
                                                    Ipp32f* pDst,  int dstStride0,  int dstStride2,
                                                    int count))
#if defined __cplusplus
}
#endif

#if defined (_IPP_STDCALL_CDECL)
  #undef  _IPP_STDCALL_CDECL
  #define __stdcall __cdecl
#endif

#endif /* __IPPM_H__ */

/* //////////////////////// End of file "ippm.h" ////////////////////////// */
