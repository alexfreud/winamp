/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2001-2009 Intel Corporation. All Rights Reserved.
//
//          Intel(R) Integrated Performance Primitives
//                      Core (ippCore)
//
*/

#if !defined( __IPPCORE_H__ ) || defined( _OWN_BLDPCS )
#define __IPPCORE_H__

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

/* /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                   Functions declarations
////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////// */


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippGetLibVersion
//  Purpose:    getting of the library version
//  Returns:    the structure of information about version
//              of ippcore library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
IPPAPI( const IppLibraryVersion*, ippGetLibVersion, (void) )


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippGetStatusString
//  Purpose:    convert the library status code to a readable string
//  Parameters:
//    StsCode   IPP status code
//  Returns:    pointer to string describing the library status code
//
//  Notes:      don't free the pointer
*/
IPPAPI( const char*, ippGetStatusString, ( IppStatus StsCode ) )


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippGetCpuType
//  Purpose:    detects Intel(R) processor
//  Parameter:  none
//  Return:     IppCpuType
//
*/

IPPAPI( IppCpuType, ippGetCpuType, (void) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippGetCpuClocks
//  Purpose:    reading of time stamp counter (TSC) register value
//  Returns:    TSC value
//
//  Note:      An hardware exception is possible if TSC reading is not supported by
/              the current chipset
*/

IPPAPI( Ipp64u, ippGetCpuClocks, (void) )


/* ///////////////////////////////////////////////////////////////////////////
//  Names:  ippSetFlushToZero,
//          ippSetDenormAreZero.
//
//  Purpose: ippSetFlushToZero enables or disables the flush-to-zero mode,
//           ippSetDenormAreZero enables or disables the denormals-are-zeros
//           mode.
//
//  Arguments:
//     value       - !0 or 0 - set or clear the corresponding bit of MXCSR
//     pUMask      - pointer to user store current underflow exception mask
//                   ( may be NULL if don't want to store )
//
//  Return:
//   ippStsNoErr              - Ok
//   ippStsCpuNotSupportedErr - the mode is not supported
*/

IPPAPI( IppStatus, ippSetFlushToZero, ( int value, unsigned int* pUMask ))
IPPAPI( IppStatus, ippSetDenormAreZeros, ( int value ))



/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippAlignPtr
//  Purpose:    pointer aligning
//  Returns:    aligned pointer
//
//  Parameter:
//    ptr        - pointer
//    alignBytes - number of bytes to align
//
*/
IPPAPI( void*, ippAlignPtr, ( void * ptr, int alignBytes ) )

/* /////////////////////////////////////////////////////////////////////////////
//                   Functions to allocate and free memory
///////////////////////////////////////////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippMalloc
//  Purpose:    32-byte aligned memory allocation
//  Parameter:
//    len       number of bytes
//  Returns:    pointer to allocated memory
//
//  Notes:      the memory allocated by ippMalloc has to be free by ippFree
//              function only.
*/

IPPAPI( void*, ippMalloc,  (int length) )


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippFree
//  Purpose:    free memory allocated by the ippMalloc function
//  Parameter:
//    ptr       pointer to the memory allocated by the ippMalloc function
//
//  Notes:      use the function to free memory allocated by ippMalloc
*/
IPPAPI( void, ippFree, (void* ptr) )


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippStaticInit
//  Purpose:    Automatic switching to best for current cpu library code using.
//  Returns:
//   ippStsNoErr       - the best code (static) successfully set
//   ippStsNonIntelCpu - px version (static) of code was set
//   ippStsNoOperationInDll - function does nothing in the dynamic version of the library
//
//  Parameter:  nothing
//
//  Notes:      At the moment of this function execution no any other IPP function
//              has to be working
*/
IPPAPI( IppStatus, ippStaticInit, ( void ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippInit
//  Purpose:    Automatic switching to best for current cpu library code using.
//  Returns:
//   ippStsNoErr
//
//  Parameter:  nothing
//
//  Notes:      At the moment of this function execution no any other IPP function
//              has to be working
*/
IPPAPI( IppStatus, ippInit, ( void ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippStaticInitCpu
//  Purpose:    switching to user defined target cpu library code using
//
//  Returns:
//   ippStsNoErr       - required target cpu library code is successfully set
//   ippStsCpuMismatch - required target cpu library can't be set, the previous
//                       set is used
//   ippStsNoOperationInDll - function does nothing in the dynamic version of the library
//
//  Parameter:  IppCpuType
//
//  Notes:      At the moment of this function execution no any other IPP function
//              has to be working
*/
IPP_DEPRECATED("use ippInitCpu function instead of this one")\
IPPAPI( IppStatus, ippStaticInitCpu, ( IppCpuType cpu ) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippInitCpu
//  Purpose:    switching to user defined target cpu library code using
//
//  Returns:
//   ippStsNoErr       - required target cpu library code is successfully set
//   ippStsCpuMismatch - required target cpu library can't be set, the previous
//                       set is used
//  Parameter:  IppCpuType
//
//  Notes:      At the moment of this function execution no any other IPP function
//              has to be working
*/
IPPAPI( IppStatus, ippInitCpu, ( IppCpuType cpu ) )


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippEnableCpu
//  Purpose:    allows dispatching CPU specific IPP library. It doesn't dispatch the code. 
//              The following call ippInit(), if it follows ippEnableCpu, does that if application 
//              is executed on CPU enabled platform or CPU simulator. 
//              Introducing this function in IPP 6.1 was needed to allow IPP users run the codes 
//              with Intel(R) AVX instructions in the case they have an Intel AVX-enabled hardware 
//              or simulator
//
//  Returns:
//   ippStsNoErr
//
//  Parameter:  IppCpuType
//  
//  Motes: The ippInit() call without calling ippEnableCpu() as well as ippEnableCpu 
//         without ippInit() call will not dispatch AVX code, even if you run code on Intel AVX 
//         enabled platform. Call the functions sequentially: 
//         ippEnableCpu(ippCpuAVX); ippInit(); to dispatch IPP AVX code. 
//         The function works and could be useful for ippCpuAVX processor type only.
//
*/
IPPAPI( IppStatus, ippEnableCpu, ( IppCpuType cpu ) )


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippGetCpuFreqMhz
//
//  Purpose:    the function estimates cpu frequency and returns
//              its value in MHz as a integer
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         null pointer to the freq value
//    ippStsSizeErr            wrong num of tries, internal var
//  Arguments:
//    pMhz                     pointer to the integer to write
//                             cpu freq value estimated
//
//  Notes:      no exact value is guaranteed, the value could
//              vary with cpu workloading
*/

IPPAPI(IppStatus, ippGetCpuFreqMhz, ( int* pMhz ) )

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippSetNumThreads
//
//  Purpose:
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNoOperation        For static library internal threading is not supported
//    ippStsSizeErr            Desired number of threads less or equal zero
//
//  Arguments:
//    numThr                   Desired number of threads
*/
IPPAPI( IppStatus, ippSetNumThreads, ( int numThr ) )

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippGetNumThreads
//
//  Purpose:
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Pointer to numThr is Null
//    ippStsNoOperation        For static library internal threading is not supported
//                             and return value is always == 1
//
//  Arguments:
//    pNumThr                  Pointer to memory location where to store current numThr
*/
IPPAPI( IppStatus, ippGetNumThreads, (int* pNumThr) )

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippGetMaxCacheSizeB
//
//  Purpose:  Detects maximal from the sizes of L2 or L3 in bytes
//
//  Return:
//    ippStsNullPtrErr         The result's pointer is NULL.
//    ippStsNotSupportedCpu    The cpu is not supported.
//    ippStsUnknownCacheSize   The cpu is supported, but the size of the cache is unknown.
//    ippStsNoErr              Ok
//
//  Arguments:
//    pSizeByte                Pointer to the result
//
//  Note:
//    1). Intel(R) processors are supported only.
//    2). Intel(R) Itanium(R) processors and platforms with Intel XScale(R) technology are unsupported
//    3). For unsupported processors the result is "0",
//        and the return status is "ippStsNotSupportedCpu".
//    4). For supported processors the result is "0",
//        and the return status is "ippStsUnknownCacheSize".
//        if sizes of the cache is unknown.
//
*/
IPPAPI( IppStatus, ippGetMaxCacheSizeB, ( int* pSizeByte ) )

/*
//  Name:       ippGetNumCoresOnDie
//  Purpose:    to distinguish MultiCore processors from other
//  Returns:    number of cores
//
*/
IPPAPI( int, ippGetNumCoresOnDie,( void ))

/*
//  Name:       ippGetCpuFeatures
//  Purpose:    Detects CPU features.
//  Parameters:
//    pFeaturesMask   Pointer to the features mask.
//                    Nonzero value of bit means the corresponding feature is supported.
//                    Features mask values are defined in the ippdefs.h
//                      [ 0] - MMX     ( ippCPUID_MMX   )
//                      [ 1] - SSE     ( ippCPUID_SSE   )
//                      [ 2] - SSE2    ( ippCPUID_SSE2  )
//                      [ 3] - SSE3    ( ippCPUID_SSE3  )
//                      [ 4] - SSSE3   ( ippCPUID_SSSE3 )
//                      [ 5] - MOVBE   ( ippCPUID_MOVBE )
//                      [ 6] - SSE41   ( ippCPUID_SSE41 )
//                      [ 7] - SSE42   ( ippCPUID_SSE42 )
//                      [8:63] - Reserved
//
//    pCpuidInfoRegs  Pointer to the 4-element vector.
//                    Result of CPUID.1 are stored in this vector.
//                      [0] - register EAX
//                      [1] - register EBX
//                      [2] - register ECX
//                      [3] - register EDX
//                    If pointer pCpuidInfoRegs is set to NULL, registers are not stored.
//
//  Returns:
//    ippStsNullPtrErr         The pointer to the features mask (pFeaturesMask) is NULL.
//    ippStsNotSupportedCpu    CPU is not supported.
//    ippStsNoErr              Ok
//
//  Note: Only IA-32 and Intel(R) 64 are supported
*/
IPPAPI( IppStatus, ippGetCpuFeatures, ( Ipp64u* pFeaturesMask,
                                        Ipp32u  pCpuidInfoRegs[4] ) )



/* /////////////////////////////////////////////////////////////////////////////
//                   i18n functions to operate with Message Catalogs
///////////////////////////////////////////////////////////////////////////// */

#if !defined( _OWN_BLDPCS )
   DECLARE_IPPCONTEXT( IppMsgCatalog );
   #if defined (_WIN32)
   typedef unsigned short* IppMsg;
   #else
     typedef char* IppMsg;
   #endif
#endif /* _OWN_BLDPCS */

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippMessageCatalogOpenI18n
//  Purpose:    Opens i18n Message Catalog
//  Parameters:
//
//  Returns:
//    ippStsMemAllocErr
//    ippStsNullPtrErr
//    ippStsI18nUnsupportedErr
//    ippStsI18nMsgCatalogOpenErr
//    ippStsNoErr
//
//  Notes: function allocates memory, this memory has to be freed by ippMessageCatalogCloseI18n
//         ippMessageCatalogCloseI18n It should be used, despite of the returned status code
*/
IPPAPI( IppStatus, ippMessageCatalogOpenI18n, ( IppMsgCatalog** pMsgCatalog ) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippMessageCatalogCloseI18n
//  Purpose:    Closes i18n Message Catalog, which was opened by ippMessageCatalogOpenI18n
//  Parameters:
//
//  Returns:
//  ippStsContextMatchErr
//  ippStsNullPtrErr
//  ippStsI18nMsgCatalogCloseErr
//  ippStsI18nUnsupportedErr
//
//  Notes:
*/
IPPAPI( IppStatus, ippMessageCatalogCloseI18n, ( IppMsgCatalog* pMsgCatalog ) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippGetMessageStatusI18n
//  Purpose:    gets localized message corresponding to IppStatus
//  Parameters:
//  Returns:
//    ippStsMemAllocErr
//    ippStsNullPtrErr
//    ippStsContextMatchErr
//    ippStsI18nMsgCatalogInvalid
//    ippStsI18nGetMessageFail
//    ippStsUnknownStatusCodeErr
//
//  Notes:
//
*/
IPPAPI( IppStatus, ippGetMessageStatusI18n, ( const IppMsgCatalog* pMsgCatalog, IppStatus StsCode, IppMsg* pMsg ) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippStatusToMessageIdI18n
//  Purpose:    transforms of IPP status to message ID for Message Catalog
//  Parameters:
//    StsCode   IPP status code
//
//  Returns:    Message ID for i18n catalogs
//
//  Notes:      Function is useful for direct Message Catalogs access
//
*/
IPPAPI( Ipp32u, ippStatusToMessageIdI18n, ( IppStatus StsCode ) )


#if defined (_IPP_STDCALL_CDECL)
  #undef  _IPP_STDCALL_CDECL
  #define __stdcall __cdecl
#endif

#ifdef __cplusplus
}
#endif

#endif /* __IPPCORE_H__ */
/* ////////////////////////////// End of file /////////////////////////////// */
