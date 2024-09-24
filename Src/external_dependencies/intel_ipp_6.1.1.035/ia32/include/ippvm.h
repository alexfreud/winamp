/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2002-2009 Intel Corporation. All Rights Reserved.
//
//              Intel(R) Integrated Performance Primitives
//                          Vector Math (ippVM)
//
*/

#if !defined( __IPPVM_H__ ) || defined( _OWN_BLDPCS )
#define __IPPVM_H__

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

#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                   Functions declarations
////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////// */


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippvmGetLibVersion
//  Purpose:    getting of the library version
//  Returns:    the structure of information about version
//              of ippVM library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/

IPPAPI( const IppLibraryVersion*, ippvmGetLibVersion, (void) )


IPPAPI( IppStatus, ippsAbs_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAbs_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsAdd_32f_A24, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAdd_64f_A53, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsSub_32f_A24, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSub_64f_A53, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsMul_32f_A24, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsMul_64f_A53, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))


IPPAPI( IppStatus, ippsInv_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInv_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInv_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInv_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInv_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInv_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsDiv_32f_A11, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsDiv_32f_A21, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsDiv_32f_A24, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsDiv_64f_A26, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsDiv_64f_A50, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsDiv_64f_A53, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsSqrt_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSqrt_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSqrt_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSqrt_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSqrt_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSqrt_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsInvSqrt_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInvSqrt_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInvSqrt_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInvSqrt_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInvSqrt_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInvSqrt_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsCbrt_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCbrt_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCbrt_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCbrt_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCbrt_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCbrt_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsInvCbrt_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInvCbrt_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInvCbrt_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInvCbrt_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInvCbrt_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsInvCbrt_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsPow_32f_A11, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow_32f_A21, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow_32f_A24, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow_64f_A26, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow_64f_A50, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow_64f_A53, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsPow2o3_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow2o3_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow2o3_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow2o3_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow2o3_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow2o3_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsPow3o2_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow3o2_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow3o2_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow3o2_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow3o2_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow3o2_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsSqr_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSqr_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsPowx_32f_A11, (const Ipp32f a[],const Ipp32f b,Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPowx_32f_A21, (const Ipp32f a[],const Ipp32f b,Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPowx_32f_A24, (const Ipp32f a[],const Ipp32f b,Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPowx_64f_A26, (const Ipp64f a[],const Ipp64f b,Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPowx_64f_A50, (const Ipp64f a[],const Ipp64f b,Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsPowx_64f_A53, (const Ipp64f a[],const Ipp64f b,Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsExp_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsExp_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsExp_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsExp_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsExp_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsExp_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsExpm1_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsExpm1_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsExpm1_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsExpm1_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsExpm1_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsExpm1_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsLn_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLn_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLn_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLn_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLn_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLn_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsLog10_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog10_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog10_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog10_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog10_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog10_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsLog1p_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog1p_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog1p_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog1p_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog1p_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog1p_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsCos_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCos_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCos_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCos_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCos_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCos_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsSin_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSin_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSin_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSin_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSin_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSin_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsSinCos_32f_A11, (const Ipp32f a[],Ipp32f r1[],Ipp32f r2[],Ipp32s n))
IPPAPI( IppStatus, ippsSinCos_32f_A21, (const Ipp32f a[],Ipp32f r1[],Ipp32f r2[],Ipp32s n))
IPPAPI( IppStatus, ippsSinCos_32f_A24, (const Ipp32f a[],Ipp32f r1[],Ipp32f r2[],Ipp32s n))
IPPAPI( IppStatus, ippsSinCos_64f_A26, (const Ipp64f a[],Ipp64f r1[],Ipp64f r2[],Ipp32s n))
IPPAPI( IppStatus, ippsSinCos_64f_A50, (const Ipp64f a[],Ipp64f r1[],Ipp64f r2[],Ipp32s n))
IPPAPI( IppStatus, ippsSinCos_64f_A53, (const Ipp64f a[],Ipp64f r1[],Ipp64f r2[],Ipp32s n))

IPPAPI( IppStatus, ippsTan_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsTan_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsTan_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsTan_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsTan_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsTan_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsAcos_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcos_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcos_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcos_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcos_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcos_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsAsin_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsin_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsin_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsin_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsin_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsin_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsAtan_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsAtan2_32f_A11, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan2_32f_A21, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan2_32f_A24, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan2_64f_A26, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan2_64f_A50, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan2_64f_A53, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsCosh_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCosh_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCosh_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCosh_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCosh_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCosh_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsSinh_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSinh_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSinh_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSinh_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSinh_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsSinh_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsTanh_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsTanh_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsTanh_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsTanh_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsTanh_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsTanh_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsAcosh_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcosh_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcosh_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcosh_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcosh_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcosh_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsAsinh_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsinh_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsinh_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsinh_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsinh_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsinh_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsAtanh_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtanh_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtanh_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtanh_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtanh_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtanh_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsErf_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErf_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErf_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErf_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErf_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErf_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsErfInv_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfInv_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfInv_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfInv_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfInv_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfInv_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsErfc_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfc_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfc_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfc_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfc_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfc_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsErfcInv_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfcInv_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfcInv_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfcInv_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfcInv_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsErfcInv_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsCdfNorm_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCdfNorm_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCdfNorm_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCdfNorm_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCdfNorm_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCdfNorm_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsCdfNormInv_32f_A11, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCdfNormInv_32f_A21, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCdfNormInv_32f_A24, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCdfNormInv_64f_A26, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCdfNormInv_64f_A50, (const Ipp64f a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCdfNormInv_64f_A53, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsHypot_32f_A11, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsHypot_32f_A21, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsHypot_32f_A24, (const Ipp32f a[],const Ipp32f b[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsHypot_64f_A26, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsHypot_64f_A50, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsHypot_64f_A53, (const Ipp64f a[],const Ipp64f b[],Ipp64f r[],Ipp32s n))



IPPAPI( IppStatus, ippsAbs_32fc_A11, (const Ipp32fc a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAbs_32fc_A21, (const Ipp32fc a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAbs_32fc_A24, (const Ipp32fc a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAbs_64fc_A26, (const Ipp64fc a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAbs_64fc_A50, (const Ipp64fc a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsAbs_64fc_A53, (const Ipp64fc a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsArg_32fc_A11, (const Ipp32fc a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsArg_32fc_A21, (const Ipp32fc a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsArg_32fc_A24, (const Ipp32fc a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsArg_64fc_A26, (const Ipp64fc a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsArg_64fc_A50, (const Ipp64fc a[],Ipp64f r[],Ipp32s n))
IPPAPI( IppStatus, ippsArg_64fc_A53, (const Ipp64fc a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsAdd_32fc_A24, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAdd_64fc_A53, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsSub_32fc_A24, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSub_64fc_A53, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsMul_32fc_A11, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsMul_32fc_A21, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsMul_32fc_A24, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsMul_64fc_A26, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsMul_64fc_A50, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsMul_64fc_A53, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsDiv_32fc_A11, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsDiv_32fc_A21, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsDiv_32fc_A24, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsDiv_64fc_A26, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsDiv_64fc_A50, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsDiv_64fc_A53, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsCIS_32fc_A11, (const Ipp32f a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCIS_32fc_A21, (const Ipp32f a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCIS_32fc_A24, (const Ipp32f a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCIS_64fc_A26, (const Ipp64f a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCIS_64fc_A50, (const Ipp64f a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCIS_64fc_A53, (const Ipp64f a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsConj_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsConj_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsMulByConj_32fc_A11, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsMulByConj_32fc_A21, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsMulByConj_32fc_A24, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsMulByConj_64fc_A26, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsMulByConj_64fc_A50, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsMulByConj_64fc_A53, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsCos_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCos_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCos_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCos_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCos_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCos_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsSin_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSin_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSin_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSin_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSin_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSin_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsTan_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsTan_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsTan_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsTan_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsTan_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsTan_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsCosh_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCosh_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCosh_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCosh_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCosh_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsCosh_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsSinh_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSinh_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSinh_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSinh_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSinh_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSinh_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsTanh_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsTanh_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsTanh_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsTanh_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsTanh_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsTanh_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsAcos_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcos_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcos_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcos_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcos_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcos_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsAsin_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsin_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsin_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsin_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsin_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsin_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsAtan_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtan_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsAcosh_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcosh_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcosh_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcosh_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcosh_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAcosh_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsAsinh_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsinh_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsinh_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsinh_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsinh_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAsinh_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsAtanh_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtanh_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtanh_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtanh_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtanh_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsAtanh_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsExp_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsExp_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsExp_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsExp_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsExp_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsExp_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsLn_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsLn_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsLn_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsLn_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsLn_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsLn_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsLog10_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog10_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog10_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog10_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog10_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsLog10_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsSqrt_32fc_A11, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSqrt_32fc_A21, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSqrt_32fc_A24, (const Ipp32fc a[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSqrt_64fc_A26, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSqrt_64fc_A50, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsSqrt_64fc_A53, (const Ipp64fc a[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsPow_32fc_A11, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow_32fc_A21, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow_32fc_A24, (const Ipp32fc a[],const Ipp32fc b[],Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow_64fc_A26, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow_64fc_A50, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsPow_64fc_A53, (const Ipp64fc a[],const Ipp64fc b[],Ipp64fc r[],Ipp32s n))

IPPAPI( IppStatus, ippsPowx_32fc_A11, (const Ipp32fc a[],const Ipp32fc b,Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsPowx_32fc_A21, (const Ipp32fc a[],const Ipp32fc b,Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsPowx_32fc_A24, (const Ipp32fc a[],const Ipp32fc b,Ipp32fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsPowx_64fc_A26, (const Ipp64fc a[],const Ipp64fc b,Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsPowx_64fc_A50, (const Ipp64fc a[],const Ipp64fc b,Ipp64fc r[],Ipp32s n))
IPPAPI( IppStatus, ippsPowx_64fc_A53, (const Ipp64fc a[],const Ipp64fc b,Ipp64fc r[],Ipp32s n))



IPPAPI( IppStatus, ippsFloor_32f, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsFloor_64f, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsCeil_32f, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsCeil_64f, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsTrunc_32f, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsTrunc_64f, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsRound_32f, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsRound_64f, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsRint_32f, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsRint_64f, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsNearbyInt_32f, (const Ipp32f a[],Ipp32f r[],Ipp32s n))
IPPAPI( IppStatus, ippsNearbyInt_64f, (const Ipp64f a[],Ipp64f r[],Ipp32s n))

IPPAPI( IppStatus, ippsModf_32f, (const Ipp32f a[],Ipp32f r1[],Ipp32f r2[],Ipp32s n))
IPPAPI( IppStatus, ippsModf_64f, (const Ipp64f a[],Ipp64f r1[],Ipp64f r2[],Ipp32s n))

#ifdef __cplusplus
}
#endif

#if defined (_IPP_STDCALL_CDECL)
  #undef  _IPP_STDCALL_CDECL
  #define __stdcall __cdecl
#endif

#endif /* __IPPVM_H__ */
/* ////////////////////////////// End of file /////////////////////////////// */

