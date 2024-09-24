/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2007-2009 Intel Corporation. All Rights Reserved.
//
//                  Intel(R) Performance Primitives
//                  Data Integrity Primitives (ippDI)
//
*/

#if !defined( __IPPDI_H__ ) || defined( _OWN_BLDPCS )
#define __IPPDI_H__


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
//  Name:       ippdiGetLibVersion
//  Purpose:    getting of the library version
//  Returns:    the structure of information about version of ippDI library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
IPPAPI( const IppLibraryVersion*, ippdiGetLibVersion, (void) )


/* /////////////////////////////////////////////////////////////////////////////
//
// GF(2^m) extension of elementary GF(2)
//
*/
#if !defined( _OWN_BLDPCS )
   typedef struct _GF8  IppsGFSpec_8u;
#endif /* _OWN_BLDPCS */

IPPAPI(IppStatus, ippsGFGetSize_8u,(int gfDegree, int* pSize))
IPPAPI(IppStatus, ippsGFInit_8u,(int gfDegree, const Ipp8u* pPolynomial, IppsGFSpec_8u* pGF))

IPPAPI(IppStatus, ippsGFAdd_8u,(Ipp8u srcA, Ipp8u srcB, Ipp8u* pDstR, const IppsGFSpec_8u* pGF))
IPPAPI(IppStatus, ippsGFSub_8u,(Ipp8u srcA, Ipp8u srcB, Ipp8u* pDstR, const IppsGFSpec_8u* pGF))
IPPAPI(IppStatus, ippsGFMul_8u,(Ipp8u srcA, Ipp8u srcB, Ipp8u* pDstR, const IppsGFSpec_8u* pGF))
IPPAPI(IppStatus, ippsGFDiv_8u,(Ipp8u srcA, Ipp8u srcB, Ipp8u* pDstR, const IppsGFSpec_8u* pGF))
IPPAPI(IppStatus, ippsGFPow_8u,(Ipp8u srcA, int srcPwr, Ipp8u* pDstR, const IppsGFSpec_8u* pGF))
IPPAPI(IppStatus, ippsGFInv_8u,(Ipp8u srcA,        Ipp8u* pDstR,  const IppsGFSpec_8u* pGF))
IPPAPI(IppStatus, ippsGFNeg_8u,(Ipp8u srcA,        Ipp8u* pDstR,  const IppsGFSpec_8u* pGF))
IPPAPI(IppStatus, ippsGFLogAlpha_8u,(Ipp8u srcA,   Ipp8u* pDstPwr,const IppsGFSpec_8u* pGF))
IPPAPI(IppStatus, ippsGFExpAlpha_8u,(Ipp8u srcPwr, Ipp8u* pDdstR, const IppsGFSpec_8u* pGF))


/* /////////////////////////////////////////////////////////////////////////////
//
// polynomials over GF(2^m)
//
*/
#if !defined( _OWN_BLDPCS )
   typedef struct _PolyGF8 IppsPoly_GF8u;
#endif /* _OWN_BLDPCS */

IPPAPI(IppStatus, ippsPolyGFGetSize_8u,(int maxDegree, int* pSize))
IPPAPI(IppStatus, ippsPolyGFInit_8u,(const IppsGFSpec_8u* pGF, int maxDegree,
                                    IppsPoly_GF8u* pPoly))

IPPAPI(IppStatus, ippsPolyGFSetCoeffs_8u,(const Ipp8u* pCoeff,int degree,
                                    IppsPoly_GF8u* pPoly))
IPPAPI(IppStatus, ippsPolyGFSetDegree_8u,(int degree,
                                    IppsPoly_GF8u* pPoly))
IPPAPI(IppStatus, ippsPolyGFCopy_8u,(const IppsPoly_GF8u* pPolyA, IppsPoly_GF8u* pPolyB))

IPPAPI(IppStatus, ippsPolyGFGetRef_8u,(Ipp8u** const pDstCoeff, int* pDstDegree,
                                    IppsGFSpec_8u** const pDstGF,
                                    const IppsPoly_GF8u* pPoly))

IPPAPI(IppStatus, ippsPolyGFAdd_8u,(const IppsPoly_GF8u* pSrcA, const IppsPoly_GF8u* pSrcB,
                                          IppsPoly_GF8u* pDstR))
IPPAPI(IppStatus, ippsPolyGFSub_8u,(const IppsPoly_GF8u* pSrcA, const IppsPoly_GF8u* pSrcB,
                                          IppsPoly_GF8u* pSrcR))
IPPAPI(IppStatus, ippsPolyGFMul_8u,(const IppsPoly_GF8u* pSrcA, const IppsPoly_GF8u* pSrcB,
                                          IppsPoly_GF8u* pDstR))
IPPAPI(IppStatus, ippsPolyGFMod_8u,(const IppsPoly_GF8u* pSrcA, const IppsPoly_GF8u* pSrcB,
                                          IppsPoly_GF8u* pDstR))
IPPAPI(IppStatus, ippsPolyGFDiv_8u,(const IppsPoly_GF8u* pSrcDividend, const IppsPoly_GF8u* pSrcDivisor,
                                          IppsPoly_GF8u* pDstQuotient,
                                          IppsPoly_GF8u* pDstReminder))
IPPAPI(IppStatus, ippsPolyGFShlC_8u,(const IppsPoly_GF8u* pSrc, int nShift,
                                           IppsPoly_GF8u* pDst))
IPPAPI(IppStatus, ippsPolyGFShrC_8u,(const IppsPoly_GF8u* pSrc, int nShift,
                                           IppsPoly_GF8u* pDst))

IPPAPI(IppStatus, ippsPolyGFIrreducible_8u,(const IppsPoly_GF8u* pSrc, IppBool* pIsIrreducible))
IPPAPI(IppStatus, ippsPolyGFPrimitive_8u,(const IppsPoly_GF8u* pSrc, IppBool isIrreducible, IppBool* pIsPrimitive))

IPPAPI(IppStatus, ippsPolyGFValue_8u,(const IppsPoly_GF8u* pSrc, Ipp8u srcE, Ipp8u* pDstValue))
IPPAPI(IppStatus, ippsPolyGFDerive_8u,(const IppsPoly_GF8u* pSrc, IppsPoly_GF8u* pDst))
IPPAPI(IppStatus, ippsPolyGFRoots_8u,(const IppsPoly_GF8u* pSrc,
                                      Ipp8u* pRoot, int* nRoots))

IPPAPI(IppStatus, ippsPolyGFGCD_8u,(const IppsPoly_GF8u* pSrcA, const IppsPoly_GF8u* pSrcB,
                                          IppsPoly_GF8u* pDstGCD))



/* /////////////////////////////////////////////////////////////////////////////
//
// RS encoder
//
*/
#if !defined( _OWN_BLDPCS )
   typedef struct _RSencodeGF8   IppsRSEncodeSpec_8u;
#endif /* _OWN_BLDPCS */

IPPAPI(IppStatus, ippsRSEncodeGetSize_8u,(int codeLen, int dataLen, int* pSize))
IPPAPI(IppStatus, ippsRSEncodeInit_8u,(int codeLen, int dataLen, const IppsGFSpec_8u* pGF, Ipp8u root,
                                       IppsRSEncodeSpec_8u* pRS))

IPPAPI(IppStatus, ippsRSEncodeGetBufferSize_8u,(const IppsRSEncodeSpec_8u* pRS, int* pSize))

IPPAPI(IppStatus, ippsRSEncode_8u,(const Ipp8u* pSrc,
                                         Ipp8u* pDst,
                                   const IppsRSEncodeSpec_8u* pRS,
                                         Ipp8u* pBuffer))


/* /////////////////////////////////////////////////////////////////////////////
//
// RS decoder
//
*/
#if !defined( _OWN_BLDPCS )
   typedef struct _RSdecodeGF8  IppsRSDecodeSpec_8u;
#endif /* _OWN_BLDPCS */

IPPAPI(IppStatus, ippsRSDecodeGetSize_8u,(int codeLen, int dataLen, int* pSize))
IPPAPI(IppStatus, ippsRSDecodeInit_8u,(int codeLen, int dataLen, const IppsGFSpec_8u* pGF, Ipp8u root,
                                       IppsRSDecodeSpec_8u* pRS))

IPPAPI(IppStatus, ippsRSDecodeBMGetBufferSize_8u,(const IppsRSDecodeSpec_8u* pRS, int* pSize))
IPPAPI(IppStatus, ippsRSDecodeEEGetBufferSize_8u,(const IppsRSDecodeSpec_8u* pRS, int* pSize))

IPPAPI(IppStatus, ippsRSDecodeBM_8u,(const int *pErasureList, int erasureListLen,
                                     Ipp8u* pSrcDstCodeWord,
                                     const IppsRSDecodeSpec_8u* pRS,
                                     Ipp8u* pBuffer))

IPPAPI(IppStatus, ippsRSDecodeEE_8u,(const int *pErasureList, int erasureListLen,
                                     Ipp8u* pSrcDstCodeWord,
                                     const IppsRSDecodeSpec_8u* pRS,
                                     Ipp8u* pBuffer))


#ifdef __cplusplus
}
#endif


#if defined (_IPP_STDCALL_CDECL)
  #undef  _IPP_STDCALL_CDECL
  #define __stdcall __cdecl
#endif


#endif /* __IPPDI_H__ */
