/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2004-2009 Intel Corporation. All Rights Reserved.
//
//                  Intel(R) Performance Primitives
//                  Data Compression Library (ippDC)
//
*/
#if !defined( __IPPDC_H__ ) || defined( _OWN_BLDPCS )
#define __IPPDC_H__

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

/********************* Data Structures and Macro ****************************/

#if !defined( _OWN_BLDPCS )

/*
//             VLC
*/

typedef struct {
  Ipp32s value;  /* current value */
  Ipp32s code;   /* the real bits code for the index */
  Ipp32s length; /* the bit length of the value */
} IppsVLCTable_32s;

#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                   Functions declarations
////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////// */


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippdcGetLibVersion
//  Purpose:    getting of the library version
//  Returns:    the structure of information about version
//              of ippDC library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
IPPAPI( const IppLibraryVersion*, ippdcGetLibVersion, (void) )

/* Run Length Encoding */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeRLE_8u
//  Purpose:            Performs the RLE encoding
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pSrcLen           Pointer to the length of source vector on input,
//                      pointer to the size of remainder on output
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output.
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsEncodeRLE_8u, ( Ipp8u** ppSrc, int* pSrcLen,
                                      Ipp8u* pDst, int* pDstLen ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeRLE_8u
//  Purpose:            Performs the RLE decoding
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pSrcLen           Pointer to the length of source vector on input,
//                      pointer to the size of remainder on output
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output.
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsSrcDataErr          The source vector contains unsupported data
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsDecodeRLE_8u, ( Ipp8u** ppSrc, int* pSrcLen,
                                      Ipp8u* pDst, int* pDstLen ))

/* Move To Front */
#if !defined ( _OWN_BLDPCS )
struct MTFState_8u;
typedef struct MTFState_8u IppMTFState_8u;
#endif

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsMTFInitAlloc_8u
// Purpose:             Allocates necessary memory and initializes structure for
//                      the MTF transform
//
// Parameters:
//    pMTFState         Pointer to the structure containing parameters for
//                       the MTF transform
//
// Return:
//    ippStsNullPtrErr  Pointer to structure is NULL
//    ippMemAllocErr    Can't allocate memory for pMTFState
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsMTFInitAlloc_8u, ( IppMTFState_8u** ppMTFState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsMTFInit_8u
// Purpose:             Initializes parameters for the MTF transform
//
// Parameters:
//    pMTFState         Pointer to the structure containing parameters for
//                      the MTF transform
//
// Return:
//    ippStsNullPtrErr  Pointer to structure is NULL
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsMTFInit_8u, ( IppMTFState_8u* pMTFState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsMTFGetSize_8u
// Purpose:             Computes the size of necessary memory (in bytes) for
//                      structure of the MTF transform
//
// Parameters:
//    pMTFStateSize     Pointer to the computed size of structure
//
// Return:
//    ippStsNullPtrErr  Pointer is NULL
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsMTFGetSize_8u, ( int* pMTFStateSize ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsMTFFree_8u
// Purpose:             Frees allocated memory for MTF transform structure
//
// Parameters:
//    pMTFState         Pointer to the structure containing parameters for
//                      the MTF transform
//
*/
IPPAPI(void, ippsMTFFree_8u, ( IppMTFState_8u* pMTFState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsMTFFwd_8u
//  Purpose:            Performs the forward MTF transform
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pDst              Pointer to the destination vector
//    len               Length of source/destination vectors
//    pMTFState         Pointer to the structure containing parameters for
//                      the MTF transform
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of the source vector is less or equal zero
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsMTFFwd_8u, ( const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                   IppMTFState_8u* pMTFState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsMTFInv_8u
//  Purpose:            Performs the inverse MTF transform
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pDst              Pointer to the destination vector
//    len               Length of source/destination vectors
//    pMTFState         Pointer to the structure containing parameters for
//                      the MTF transform
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of the source vector is less or equal zero
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsMTFInv_8u, ( const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                   IppMTFState_8u* pMTFState ))

/* Burrows - Wheeler Transform */
/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsBWTFwdGetSize_8u
// Purpose:             Computes the size of necessary memory (in bytes) for
//                      additional buffer for the forward BWT transform
//
// Parameters:
//    wndSize           Window size for the BWT transform
//    pBWTFwdBuffSize   Pointer to the computed size of buffer
//
// Return:
//    ippStsNullPtrErr  Pointer is NULL
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsBWTFwdGetSize_8u, ( int wndSize, int* pBWTFwdBuffSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsBWTFwd_8u
//  Purpose:            Performs the forward BWT transform
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pDst              Pointer to the destination vector
//    len               Length of source/destination vectors
//    index             Pointer to the index of first position for
//                      the inverse BWT transform
//    pBWTFwdBuff       Pointer to the additional buffer
//
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of source/destination vectors is less or equal zero
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsBWTFwd_8u, ( const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                   int* index, Ipp8u* pBWTFwdBuff ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsBWTInvGetSize_8u
// Purpose:             Computes the size of necessary memory (in bytes) for
//                      additional buffer for the inverse BWT transform
//
// Parameters:
//    wndSize           Window size for the BWT transform
//    pBWTInvBuffSize   Pointer to the computed size of buffer
//
// Return:
//    ippStsNullPtrErr  Pointer is NULL
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsBWTInvGetSize_8u, ( int wndSize, int* pBWTInvBuffSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsBWTInv_8u
//  Purpose:            Performs the inverse BWT transform
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pDst              Pointer to the destination vector
//    len               Length of source/destination vectors
//    index             Index of first position for the inverse BWT transform
//    pBWTInvBuff       Pointer to the additional buffer
//
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of source/destination vectors is less or
//                      equal zero or index greater or equal srcLen
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsBWTInv_8u, ( const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                   int index, Ipp8u* pBWTInvBuff ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsBWTGetSize_SmallBlock_8u
// Purpose:             Computes the size of necessary memory (in bytes) for
//                      additional buffer for the forward/inverse BWT transform
//
// Parameters:
//    wndSize           Window size for the BWT transform
//    pBWTBuffSize      Pointer to the computed size of buffer
//
// Return:
//    ippStsNullPtrErr  Pointer is NULL
//    ippStsSizeErr     wndSize less or equal 0 or more than 32768
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsBWTGetSize_SmallBlock_8u, ( int wndSize, int* pBuffSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsBWTFwd_SmallBlock_8u
//  Purpose:            Performs the forward BWT transform. This function is
//                      destined for processing of small blocks <= 32768
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pDst              Pointer to the destination vector
//    len               Length of source/destination vectors
//    index             Pointer to the index of first position for
//                      the inverse BWT transform
//    pBWTBuff          Pointer to the additional buffer
//
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of vectors is less or equal 0 or more than 32768
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsBWTFwd_SmallBlock_8u, ( const Ipp8u* pSrc, Ipp8u* pDst,
                                              int len, int* index,
                                              Ipp8u* pBWTBuff ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsBWTInv_SmallBlock_8u
//  Purpose:            Performs the inverse BWT transform. This function is
//                      destined for processing of small blocks <= 32768
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pDst              Pointer to the destination vector
//    len               Length of source/destination vectors
//    index             Index of first position for the inverse BWT transform
//    pBWTBuff          Pointer to the additional buffer
//
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of source/destination vectors is less or
//                      equal 0 or more than 32768 or index greater or equal srcLen
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsBWTInv_SmallBlock_8u, ( const Ipp8u* pSrc, Ipp8u* pDst,
                                              int len, int index,
                                              Ipp8u* pBWTBuff ))

/* Huffman Coding */
#if !defined ( _OWN_BLDPCS )
struct HuffState_8u;
typedef struct HuffState_8u IppHuffState_8u;
#endif

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsEncodeHuffInit_8u
// Purpose:             Initializes structure for Huffman encoding
//
// Parameters:
//    freqTable         Table of symbols' frequencies
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//
// Return:
//    ippStsNullPtrErr        One or several pointer(s) is NULL
//    ippStsFreqTableErr      Invalid freqTable
//    ippStsMaxLenHuffCodeErr Max length of Huffman code more expected
//    ippStsNoErr             No errors
//
*/
IPPAPI(IppStatus, ippsEncodeHuffInit_8u, ( const int freqTable[256],
                                           IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsHuffGetSize_8u
// Purpose:             Computes the size of necessary memory (in bytes) for
//                      structure of Huffman coding
//
// Parameters:
//    pHuffStateSize    Pointer to the computed size of structure
//
// Return:
//    ippStsNullPtrErr  Pointer is NULL
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsHuffGetSize_8u, ( int* pHuffStateSize ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsEncodeHuffInitAlloc_8u
// Purpose:             Allocates necessary memory and initializes structure for
//                      Huffman encoding
//
// Parameters:
//    freqTable         Table of symbols' frequencies
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//
// Return:
//    ippStsNullPtrErr        One or several pointer(s) is NULL
//    ippMemAllocErr          Can't allocate memory for pHuffState
//    ippStsFreqTableErr      Invalid freqTable
//    ippStsMaxLenHuffCodeErr Max length of Huffman code more expected
//    ippStsNoErr             No errors
//
*/
IPPAPI(IppStatus, ippsEncodeHuffInitAlloc_8u, ( const int freqTable[256],
                                                IppHuffState_8u** ppHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsHuffFree_8u
// Purpose:             Frees allocated memory for Huffman coding structure
//
// Parameters:
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//
*/
IPPAPI(void, ippsHuffFree_8u, ( IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeHuffOne_8u
//  Purpose:            Performs Huffman encoding of the one source element
//
//  Parameters:
//    src               Source element
//    pDst              Pointer to the destination vector
//    dstOffsetBits     Offset in the destination vector, starting with high bit
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//  Return:
//    ippStsNullPtrErr      One or several pointer(s) is NULL
//    ippStsCodeLenTableErr Invalid codeLenTable
//    ippStsSizeErr         dstOffsetBits less than 0 or more than 7
//    ippStsNoErr           No errors
//
*/
IPPAPI(IppStatus, ippsEncodeHuffOne_8u, ( Ipp8u src, Ipp8u* pDst, int dstOffsetBits,
                                          IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeHuff_8u
//  Purpose:            Performs Huffman encoding
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    srcLen            Length of source vector
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the resulting length of the destination vector
//                      on output.
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of the source vector is less or equal zero
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsEncodeHuff_8u, ( const Ipp8u* pSrc, int srcLen,
                                       Ipp8u* pDst, int* pDstLen,
                                       IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeHuffFinal_8u
//  Purpose:            Flushes remainder after Huffman encoding
//
//  Parameters:
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the resulting length of the destination vector
//                      on output.
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsEncodeHuffFinal_8u, ( Ipp8u* pDst, int* pDstLen,
                                            IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsHuffGetLenCodeTable_8u
//  Purpose:            Gives back the table with lengths of Huffman codes from
//                      pHuffState
//
//  Parameters:
//    codeLenTable      Destination table with lengths of Huffman codes
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsHuffGetLenCodeTable_8u, ( int codeLenTable[256],
                                                IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsDecodeHuffInit_8u
// Purpose:             Initializes structure for Huffman decoding
//
// Parameters:
//    codeLenTable      Table with lengths of Huffman codes
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//
// Return:
//    ippStsNullPtrErr      One or several pointer(s) is NULL
//    ippStsCodeLenTableErr Invalid codeLenTable
//    ippStsNoErr           No errors
//
*/
IPPAPI(IppStatus, ippsDecodeHuffInit_8u, ( const int codeLenTable[256],
                                           IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsDecodeHuffInitAlloc_8u
// Purpose:             Allocates necessary memory and initializes structure for
//                      Huffman decoding
//
// Parameters:
//    codeLenTable      Table with lengths of Huffman codes
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//
// Return:
//    ippStsNullPtrErr      One or several pointer(s) is NULL
//    ippMemAllocErr        Can't allocate memory for pHuffState
//    ippStsCodeLenTableErr Invalid codeLenTable
//    ippStsNoErr           No errors
//
*/
IPPAPI(IppStatus, ippsDecodeHuffInitAlloc_8u, ( const int codeLenTable[256],
                                                IppHuffState_8u** ppHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeHuffOne_8u
//  Purpose:            Performs Huffman decoding of the one destination element
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    srcOffsetBits     Offset in the source vector, starting with high bit
//    pDst              Pointer to the destination vector
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     srcOffsetBits less than 0 or more than 7
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsDecodeHuffOne_8u, ( const Ipp8u* pSrc, int srcOffsetBits,
                                          Ipp8u* pDst, IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeHuff_8u
//  Purpose:            Performs Huffman decoding
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    srcLen            Length of source vector
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the expected size of destination vector on input,
//                      pointer to the resulting length of the destination vector
//                      on output.
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of the source vector is less or equal zero
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsDecodeHuff_8u, ( const Ipp8u* pSrc, int srcLen,
                                       Ipp8u* pDst, int* pDstLen,
                                       IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsHuffGetDstBuffSize_8u
// Purpose:             Computes the size of necessary memory (in bytes) for
//                      the destination buffer (for Huffman encoding/decoding)
//
// Parameters:
//    codeLenTable      Table with lengths of Huffman codes
//    srcLen            Length of source vector
//    pEncDstBuffSize   Pointer to the computed size of the destination buffer
//                      for Huffman encoding (value returns if pointer isn't NULL)
//    pDecDstBuffSize   Pointer to the computed size of the destination buffer
//                      for Huffman decoding (value returns if pointer isn't NULL)
//
// Return:
//    ippStsNullPtrErr      Pointer to codeLenTable is NULL
//    ippStsCodeLenTableErr Invalid codeLenTable
//    ippStsNoErr           No errors
//
*/
IPPAPI(IppStatus, ippsHuffGetDstBuffSize_8u, ( const int codeLenTable[256], int srcLen,
                                               int* pEncDstBuffSize, int* pDecDstBuffSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsHuffLenCodeTablePack_8u
//  Purpose:            Packs the table with lengths of Huffman codes
//
//  Parameters:
//    codeLenTable      Table with lengths of Huffman codes
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output.
//
//  Return:
//    ippStsNullPtrErr      One or several pointer(s) is NULL
//    ippStsSizeErr         Length of the destination vector is less, equal zero or
//                          less expected
//    ippStsCodeLenTableErr Invalid codeLenTable
//    ippStsNoErr           No errors
//
*/
IPPAPI(IppStatus, ippsHuffLenCodeTablePack_8u,   ( const int codeLenTable[256],
                                                   Ipp8u* pDst, int* pDstLen ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsHuffLenCodeTableUnpack_8u
//  Purpose:            Unpacks the table with lengths of Huffman codes
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pSrcLen           Pointer to the length of source vector on input,
//                      pointer to the resulting length of the source vector
//    codeLenTable      Table with lengths of Huffman codes
//
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of the source vector is less, equal zero or
//                      less expected
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsHuffLenCodeTableUnpack_8u, ( const Ipp8u* pSrc, int* pSrcLen,
                                                   int codeLenTable[256] ))

/*  Generalized Interval Transform (GIT) functions */
#if !defined ( _OWN_BLDPCS )
struct GITState_8u;
typedef struct GITState_8u IppGITState_8u;
typedef enum {
    ippGITNoStrategy,
    ippGITLeftReorder,
    ippGITRightReorder,
    ippGITFixedOrder
} IppGITStrategyHint;
#endif

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeGITGetSize_8u
//  Purpose:            Finds out size of GIT internal encoding state structure
//                      in bytes
//
//  Parameters:
//    maxSrcLen         Max length of source vector
//    maxDstLen         Max length of destination vector
//    pGITStateSize     Pointer to the size of GIT internal encoding state
//  Return:
//    ippStsNullPtrErr  Pointer to GITStateSize is NULL
//    ippStsSizeErr     Bad length arguments
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsEncodeGITGetSize_8u, ( int maxSrcLen, int maxDstLen,
                                             int* pGITStateSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeGITInit_8u
//  Purpose:            Initializes the GIT internal encoding state
//
//  Parameters:
//    maxSrcLen         Max length of source vector
//    maxDstLen         Max length of destination vector
//    pGITState         Pointer to memory allocated for GIT internal encoding
//                      state
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Bad size arguments
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsEncodeGITInit_8u, (int maxSrcLen, int maxDstLen,
                                         IppGITState_8u* ppGITState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeGITInitAlloc_8u
//  Purpose:            Allocates and Initializes the GIT internal encoding state
//
//  Parameters:
//    maxSrcLen         Max length of source vector
//    maxDstLen         Max length of destination vector
//    ppGITState        Pointer to pointer to GIT internal encoding state
//  Return:
//    ippStsSizeErr     Bad length arguments
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsEncodeGITInitAlloc_8u, (int maxSrcLen, int maxDstLen,
                                              IppGITState_8u** ppGITState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeGIT_8u
//  Purpose:            Performs GIT encoding
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    srcLen            Length of source vector
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the length of destination vector
//    strategyHint      Strategy hint for lexicorgaphical reordering
//    pGITState         Pointer to GIT internal encoding state
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Source vector is too long, more than the value of
//                      maxSrcLen parameter passed to ippsGITEncodeGetSize_8u
//                      or ippsGITEncodeInitAlloc_8u
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsEncodeGIT_8u, (const Ipp8u* pSrc, int srcLen, Ipp8u* pDst,
                                     int* pDstLen,
                                     IppGITStrategyHint strategyHint,
                                     IppGITState_8u* pGITState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeGITGetSize_8u
//  Purpose:            Finds out size of GIT internal decoding state structure
//                      in bytes
//
//  Parameters:
//    maxSrcLen         Max length of source vector
//    pGITStateSize     Pointer to the size of GIT internal decoding state
//  Return:
//    ippStsNullPtrErr  Pointer to GITStateSize is NULL
//    ippStsSizeErr     Bad length arguments
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsDecodeGITGetSize_8u, (int maxSrcLen, int* pGITStateSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeGITInit_8u
//  Purpose:            Initializes the GIT internal decoding state
//
//  Parameters:
//    maxSrcLen         Max length of source vector
//    maxDstLen         Max length of destination vector
//    pGITState         Pointer to memory allocated for GIT internal decoding
//                      state
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Bad size arguments
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsDecodeGITInit_8u, (int maxDstLen, IppGITState_8u* pGITState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeGITInitAlloc_8u
//  Purpose:            Allocates and Initializes the GIT internal decoding state
//
//  Parameters:
//    maxSrcLen         Max length of source vector
//    maxDstLen         Max length of destination vector
//    ppGITState        Pointer to pointer to GIT internal decoding state
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Bad length arguments
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsDecodeGITInitAlloc_8u, (int maxSrcLen, int maxDstLen,
                                              IppGITState_8u** ppGITState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeGIT_8u
//  Purpose:            Performs GIT decoding
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    srcLen            Length of source vector
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the length of destination vector
//    strategyHint      Strategy hint for lexicorgaphical reordering
//    pGITState         Pointer to GIT internal decoding state
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Not enough memory allocated for destination buffer
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsDecodeGIT_8u, (const Ipp8u* pSrc, int srcLen, Ipp8u* pDst, int* pDstLen,
                                     IppGITStrategyHint strategyHint,
                                     IppGITState_8u* pGITState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsGITFree_8u
//  Purpose:            Frees the GIT internal decoding state
//
//  Parameters:
//    pGITState         Pointer to the GIT internal state
//
*/
IPPAPI(void, ippsGITFree_8u, (IppGITState_8u* pGITState))

/* Ziv Lempel Storer Szymanski (LZSS) functions */
#if !defined ( _OWN_BLDPCS )
struct LZSSState_8u;
typedef struct LZSSState_8u IppLZSSState_8u;
#endif

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsLZSSGetSize_8u
//  Purpose:            Finds out size of LZSS internal state structure in bytes
//
//  Parameters:
//    pLZSSStateSize    Pointer to the size of LZSS internal encoding state
//  Return:
//    ippStsNullPtrErr  Pointer to LZSSStateSize is NULL
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsLZSSGetSize_8u, ( int* pLZSSStateSize ))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZSSInit_8u
//  Purpose:            Initializes the LZSS internal state for encoding
//
//  Parameters:
//    pLZSSState        Pointer to memory allocated for LZSS internal state
//  Return:
//    ippStsNullPtrErr  Pointer to internal LZSS state structure is NULL
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsEncodeLZSSInit_8u, ( IppLZSSState_8u* pLZSSState ))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZSSInitAlloc_8u
//  Purpose:            Allocates and Initializes the LZSS internal state for encoding
//
//  Parameters:
//    ppLZSSState       Double pointer to LZSS internal state
//  Return:
//    ippStsNullPtrErr     Double pointer to internal LZSS state structure is NULL
//    ippStsMemAllocErr    Error occurred during memory allocation
//    ippStsNoErr          No errors
//
*/
IPPAPI(IppStatus, ippsEncodeLZSSInitAlloc_8u, ( IppLZSSState_8u** ppLZSSState ))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZSS_8u
//  Purpose:            Performs LZSS encoding
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector
//    ppDst             Double pointer to the destination vector
//    pDstLen           Pointer to the length of destination vector
//    pLZSSState        Pointer to LZSS internal state for encoding
//  Return:
//    ippStsNullPtrErr           One or several pointer(s) is NULL
//    ippStsSizeErr              Bad length arguments
//    ippStsDstSizeLessExpected  Destination buffer is full
//    ippStsNoErr                No errors
//
*/
IPPAPI(IppStatus, ippsEncodeLZSS_8u, ( Ipp8u** ppSrc, int* pSrcLen, Ipp8u** ppDst, int* pDstLen,
                                    IppLZSSState_8u* pLZSSState ))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZSSFlush_8u
//  Purpose:            Flushes the last few bits from the bit stream and alignes
//                      output data on the byte boundary
//
//  Parameters:
//    ppDst             Double pointer to the destination vector
//    pDstLen           Pointer to the length of destination vector
//    pLZSSState        Pointer to the LZSS internal state for encoding
//  Return:
//    ippStsNullPtrErr           One or several pointer(s) is NULL
//    ippStsSizeErr              Bad length arguments
//    ippStsDstSizeLessExpected  Destination buffer is full
//    ippStsNoErr                No errors
*/
IPPAPI(IppStatus, ippsEncodeLZSSFlush_8u, (Ipp8u** ppDst, int* pDstLen, IppLZSSState_8u* pLZSSState))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZSSInit_8u
//  Purpose:            Initializes the LZSS internal state for decoding
//
//  Parameters:
//    pLZSSState        Pointer to memory allocated for LZSS internal state
//  Return:
//    ippStsNullPtrErr  Pointer to internal LZSS state structure is NULL
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsDecodeLZSSInit_8u, ( IppLZSSState_8u* pLZSSState ))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZSSInitAlloc_8u
//  Purpose:            Allocates and Initializes the LZSS internal state for decoding
//
//  Parameters:
//    ppLZSSState       Double pointer to LZSS internal state for decoding
//  Return:
//    ippStsNullPtrErr     Double pointer to internal LZSS state structure is NULL
//    ippStsMemAllocErr    Error occurred during memory allocation
//    ippStsNoErr          No errors
//
*/
IPPAPI(IppStatus, ippsDecodeLZSSInitAlloc_8u, ( IppLZSSState_8u** ppLZSSState))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZSS_8u
//  Purpose:            Performs LZSS decoding
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector
//    ppDst             Double pointer to the destination vector
//    pDstLen           Pointer to the length of destination vector
//    pLZSSState        Pointer to LZSS internal state
//  Return:
//    ippStsNullPtrErr           One or several pointer(s) is NULL
//    ippStsSizeErr              Bad length arguments
//    ippStsDstSizeLessExpected  Destination buffer is full
//    ippStsNoErr                No errors
//
*/
IPPAPI(IppStatus, ippsDecodeLZSS_8u, ( Ipp8u** ppSrc, int* pSrcLen, Ipp8u** ppDst,
                                    int* pDstLen, IppLZSSState_8u* pLZSSState ))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsLZSSFree_8u
//  Purpose:            Frees the LZSS internal state
//
//  Parameters:
//    pLZSSState        Pointer to the LZSS internal state for decoding
//
*/
IPPAPI(void, ippsLZSSFree_8u, ( IppLZSSState_8u* pLZSSState ))

/* rfc1950, 1951, 1952 - compatible functions */

#if !defined ( _OWN_BLDPCS )
struct LZ77State_8u;
typedef struct LZ77State_8u IppLZ77State_8u;
typedef enum{
   IppLZ77FastCompr,
   IppLZ77AverageCompr,
   IppLZ77BestCompr
} IppLZ77ComprLevel;
typedef enum{
   IppLZ77NoChcksm,
   IppLZ77Adler32,
   IppLZ77CRC32
} IppLZ77Chcksm;
typedef enum {
   IppLZ77NoFlush,
   IppLZ77SyncFlush,
   IppLZ77FullFlush,
   IppLZ77FinishFlush
} IppLZ77Flush;
typedef struct IppLZ77Pairs_16u {
   Ipp16u length;
   Ipp16u offset;
} IppLZ77Pair;
typedef enum {
   IppLZ77StatusInit,
   IppLZ77StatusLZ77Process,
   IppLZ77StatusHuffProcess,
   IppLZ77StatusFinal
} IppLZ77DeflateStatus;
typedef enum {
  IppLZ77UseFixed,
  IppLZ77UseDynamic,
  IppLZ77UseStored
} IppLZ77HuffMode;
typedef enum {
  IppLZ77InflateStatusInit,
  IppLZ77InflateStatusHuffProcess,
  IppLZ77InflateStatusLZ77Process,
  IppLZ77InflateStatusFinal
} IppLZ77InflateStatus;
#endif

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77GetSize
//  Purpose:            Computes the size of the internal encoding structure.
//
//  Parameters:
//   pLZ77VLCStateSize  Pointer to the size of the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pointer pLZ77VLCStateSize is NULL.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77GetSize_8u, (int* pLZ77StateSize) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77Init
//  Purpose:            Initializes the internal encoding structure.
//
//  Parameters:
//   comprLevel         Compression level.
//   checksum           Algorithm to compute the checksum for input data.
//   pLZ77State         Pointer to memory allocated for the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//   ippStsBadArgErr    Indicates an error when the checksum or comprLevel parameter
//                      has an illegal value.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77Init_8u, (IppLZ77ComprLevel comprLevel,
                                         IppLZ77Chcksm checksum, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77InitAlloc
//  Purpose:            Allocates memory and initializes the internal encoding structure.
//
//  Parameters:
//   comprLevel         Compression level.
//   checksum           Algorithm to compute the checksum for input data.
//   ppLZ77State        Double pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the ppLZ77State pointer is NULL.
//   ippStsBadArgErr    Indicates an error when the checksum or comprLevel parameter
//                      has an illegal value.
//   ippStsMemAlloc     Indicates an error when memory allocation fails.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77InitAlloc_8u, (IppLZ77ComprLevel comprLevel,
                                              IppLZ77Chcksm checksum, IppLZ77State_8u** ppLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77
//  Purpose:            Performs LZ77 encoding.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//
//  Note: This function searches for substring matches using the LZ77 algorithm.
//        The technique of sliding window support is compatible with rfc1951.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77_8u, (Ipp8u** ppSrc, int* pSrcLen, IppLZ77Pair** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77SelectHuffMode
//  Purpose:            Takes the best decision about the optimal coding strategy
//                      (use fixed Huffman coding or dynamic Huffman coding).
//
//  Parameters:
//   pSrc               Pointer to the source vector.
//   srcLen             Length of the source vector.
//   pHuffMode          Pointer to the value of coding strategy.
//   pLZ77State         Pointer to memory allocated for the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77SelectHuffMode_8u, (IppLZ77Pair* pSrc, int srcLen,
                                                    IppLZ77HuffMode* pHuffMode,
                                                    IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77FixedHuff
//  Purpose:            Performs fixed Huffman coding of the LZ77 output.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//   ippStsStreamEnd            Indicates a warning when the stream ends. This warning can
//                              be returned only when the flush value is FINISH.
//
//  Note: This function produces the rfc1951 compatible code for the LZ77 output.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77FixedHuff_8u, (IppLZ77Pair** ppSrc, int* pSrcLen, Ipp8u** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77DynamicHuff
//  Purpose:            Performs dynamic Huffman coding of the LZ77 output.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//   ippStsStreamEnd            Indicates a warning when the stream ends. This warning can
//                              be returned only when the flush value is FINISH.
//
//  Note: This function produces the rfc1951 compatible code for the LZ77 output.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77DynamicHuff_8u, (IppLZ77Pair** ppSrc, int* pSrcLen, Ipp8u** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77StoredBlock
//  Purpose:            Transmits the block without compression.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77StoredBlock_8u, (Ipp8u** ppSrc, int* pSrcLen, Ipp8u** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77Flush
//  Purpose:            Performs writing the service information (accumulated
//                      checksum and total length of input data stream) in order
//                      to achieve the ZLIB/GZIP data format compatibility.
//
//  Parameters:
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//
//  Note: This is a service function which is necessary for achieving compatibility with
//        the rfc1950, rfc1951, rfc1952 describing ZLIB/GZIP data format.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77Flush_8u, (Ipp8u** ppDst, int* pDstLen,
                                                 IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77GetPairs
//  Purpose:            Reads the pointer to the pair buffer, it's length and current index
//                      from the internal state structure for encoding.
//
//  Parameters:
//   ppPairs            Double pointer to a variable of ippLZ77Pair type.
//   pPairsInd          Pointer to the current index in the pair buffer
//   pPairsLen          Pointer to the length of pair buffer
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State or ppPairs pointer is NULL.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77GetPairs_8u, (IppLZ77Pair** ppPairs, int* pPairsInd,
                                              int* pPairsLen, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77SetPairs
//  Purpose:            Writes the pointer to the pair buffer, it's length and current index
//                      to the internal state structure for encoding.
//
//  Parameters:
//   pPairs             Pointer to a variable of ippLZ77Pair type.
//   pairsInd           Current index in the pair buffer
//   pairsLen           Length of pair buffer
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State or pPairs pointer is NULL.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77SetPairs_8u, (IppLZ77Pair* pPairs, int pairsInd,
                                             int pairsLen, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77GetStatus
//  Purpose:            Reads the encoding status value from the internal state
//                      structure for encoding.
//
//  Parameters:
//   pDeflateStatus     Pointer to a variable of ippLZ77DeflateStatus type.
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State or pDeflateStatus pointer is NULL.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77GetStatus_8u, (IppLZ77DeflateStatus* pDeflateStatus,
                                              IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77SetStatus
//  Purpose:            Writes the encoding status value to the internal state
//                      structure for encoding.
//
//  Parameters:
//   deflateStatus      Variable of ippLZ77DeflateStatus type.
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//   ippStsBadArgErr    Indicates an error when the deflateStatus parameter has an illegal value.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77SetStatus_8u, (IppLZ77DeflateStatus deflateStatus,
                                              IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77Reset
//  Purpose:            Resets the internal state structure for encoding.
//
//  Parameters:
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//
*/
IPPAPI( IppStatus, ippsEncodeLZ77Reset_8u, (IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77GetSize
//  Purpose:            Computes the size of the internal decoding structure.
//
//  Parameters:
//   pLZ77StateSize     Pointer to the size of the internal decoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pointer pLZ77StateSize is NULL.
//
*/
IPPAPI( IppStatus, ippsDecodeLZ77GetSize_8u, (int* pLZ77StateSize) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77Init
//  Purpose:            Initializes the internal decoding structure.
//
//  Parameters:
//   checksum           Algorithm to compute the checksum for output data.
//   pLZ77State         Pointer to memory allocated for the internal decoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//   ippStsBadArgErr    Indicates an error when the checksum parameter
//                      has an illegal value.
//
*/
IPPAPI( IppStatus, ippsDecodeLZ77Init_8u, (IppLZ77Chcksm checksum, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77InitAlloc
//  Purpose:            Allocates memory and initializes the internal encoding structure.
//
//  Parameters:
//   checksum           Algorithm to compute the checksum for output data.
//   ppLZ77State        Double pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the ppLZ77State pointer is NULL.
//   ippStsBadArgErr    Indicates an error when the checksum parameter has an illegal value.
//   ippStsMemAlloc     Indicates an error when memory allocation fails.
//
*/
IPPAPI( IppStatus, ippsDecodeLZ77InitAlloc_8u, (IppLZ77Chcksm checksum, IppLZ77State_8u** ppLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77
//  Purpose:            Performs LZ77 decoding.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//   ippStsStreamEnd            Indicates a warning when the stream ends.
//
//  Note: The technique of LZ77 sliding window support is compatible with rfc1951.
//
*/
IPPAPI( IppStatus, ippsDecodeLZ77_8u, (IppLZ77Pair** ppSrc, int* pSrcLen, Ipp8u** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77GetBlockType
//  Purpose:            Decodes the type of the block from the DEFLATE format.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   pHuffMode          Pointer to the value of coding mode.
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when SrcLen is less than zero.
//   ippStsSrcSizeLessExpected  Indicates a warning when the source buffer is less than expected.
//                              (Internal bit stream and source vector do not contain enough bits to decode
//                              the type of the block)
//
*/
IPPAPI( IppStatus, ippsDecodeLZ77GetBlockType_8u, (Ipp8u** ppSrc, int* pSrcLen,
                                                  IppLZ77HuffMode* pHuffMode,
                                                  IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77FixedHuff
//  Purpose:            Performs fixed Huffman decoding of the rfc1951 compatible code.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsSrcSizeLessExpected  Indicates a warning when the source buffer is less than expected
//                              (end of block marker is not decoded).
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//   ippStsStreamEnd            Indicates a warning when the stream ends.
//
//  Note: This function decodes the rfc1951 compatible code.
//
*/
IPPAPI( IppStatus, ippsDecodeLZ77FixedHuff_8u, (Ipp8u** ppSrc, int* pSrcLen, IppLZ77Pair** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77DynamicHuff
//  Purpose:            Performs dynamic Huffman decoding of the rfc1951 compatible code.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsWrongBlockType       Indicates a warning when the type of the block is not dynamic Huffman type.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsSrcSizeLessExpected  Indicates a warning when the source buffer is less than expected
//                              (end of block marker is not decoded).
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//   ippStsStreamEnd            Indicates a warning when the stream ends.
//
//  Note: This function decodes the rfc1951 compatible code.
//
*/
IPPAPI( IppStatus, ippsDecodeLZ77DynamicHuff_8u, (Ipp8u** ppSrc, int* pSrcLen, IppLZ77Pair** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77StoredBlock
//  Purpose:            Performs decoding of the block transmitted without compression.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsWrongBlockType       Indicates a warning when the type of the block is not of
//                              the "stored without compression type" type.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsSrcSizeLessExpected  Indicates a warning when the source buffer is less than expected
//                              (end of block marker is not decoded).
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//
//
*/
IPPAPI( IppStatus, ippsDecodeLZ77StoredBlock_8u, (Ipp8u** ppSrc, int* pSrcLen, Ipp8u** ppDst,
                                     int* pDstLen, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77StoredHuff_8u
//  Purpose:            Performs copying the data to the output buffer of pairs
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector
//    ppDst             Double pointer to the destination vector of pairs
//    pDstLen           Pointer to the length of destination vector of pairs
//    pLZ77State        Pointer to the internal state
//
//  Return:
//   ippStsNoErr                Indicates no error
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL
//   ippStsWrongBlockType       Indicates a warning when the type of the block is not of
//                              the "stored without compression type" type
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero
//   ippStsSrcSizeLessExpected  Indicates a warning when the source buffer is less than expected
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full
//
*/

IPPAPI( IppStatus, ippsDecodeLZ77StoredHuff_8u, (Ipp8u** ppSrc, int* pSrcLen, IppLZ77Pair** ppDst,
                                     int* pDstLen, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77GetPairs
//  Purpose:            Reads the pointer to the pair buffer, it's length and current index
//                      from the internal state structure for decoding.
//
//  Parameters:
//   ppPairs            Double pointer to a variable of ippLZ77Pair type.
//   pPairsInd          Pointer to the current index in the pair buffer
//   pPairsLen          Pointer to the length of pair buffer
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State or ppPairs pointer is NULL.
//
*/
IPPAPI( IppStatus, ippsDecodeLZ77GetPairs_8u, (IppLZ77Pair** ppPairs, int* pPairsInd,
                                              int* pPairsLen, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77SetPairs
//  Purpose:            Writes the pointer to the pair buffer, it's length and current index
//                      to the internal state structure for decoding.
//
//  Parameters:
//   pPairs             Pointer to a variable of ippLZ77Pair type.
//   pairsInd           Current index in the pair buffer
//   pairsLen           Length of pair buffer
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State or pPairs pointer is NULL.
//
*/
IPPAPI( IppStatus, ippsDecodeLZ77SetPairs_8u, (IppLZ77Pair* pPairs, int pairsInd,
                                             int pairsLen, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77GetStatus
//  Purpose:            Reads the decoding status value from the internal state
//                      structure for decoding.
//
//  Parameters:
//   pInflateStatus     Pointer to a variable of ippLZ77InflateStatus type.
//   pLZ77State         Pointer to the internal structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State or pInflateStatus pointer is NULL.
//
*/
IPPAPI( IppStatus, ippsDecodeLZ77GetStatus_8u, ( IppLZ77InflateStatus* pInflateStatus,
                                               IppLZ77State_8u* pLZ77State ) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77SetStatus
//  Purpose:            Writes the decoding status value to the internal state
//                      structure for decoding.
//
//  Parameters:
//   inflateStatus      Variable of ippLZ77InflateStatus type.
//   pLZ77State         Pointer to the internal structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//   ippStsBadArgErr    Indicates an error when the inflateStatus parameter has an illegal value.
//
*/
IPPAPI( IppStatus, ippsDecodeLZ77SetStatus_8u, ( IppLZ77InflateStatus inflateStatus,
                                               IppLZ77State_8u* pLZ77State ) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77SetDictionary_8u, ippsDecodeLZ77SetDictionary_8u
//  Purpose:            Presets the dictionary for encoding/decoding.
//
//  Parameters:
//    pDictionary       Pointer to the dictionary vector
//    dictLen           Length of dictionary vector
//    pLZ77State        Pointer to the internal state
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsEncodeLZ77SetDictionary_8u, ( Ipp8u* pDictionary, int dictLen, IppLZ77State_8u* pLZ77State ))
IPPAPI(IppStatus, ippsDecodeLZ77SetDictionary_8u, ( Ipp8u* pDictionary, int dictLen, IppLZ77State_8u* pLZ77State ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77Reset
//  Purpose:            Resets the internal state structure for decoding.
//
//  Parameters:
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//
*/
IPPAPI( IppStatus, ippsDecodeLZ77Reset_8u, (IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77FixedHuffFull_8u
//  Purpose:            Performs the decoding of fixed huffman rfc1951 compatible format
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector
//    ppDst             Double pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output
//    flush             Flush mode
//    pLZ77State        Pointer to internal decoding state
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsSrcSizeLessExpected The end of block symbol not decoded, so size of source vector less expected
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsStreamEnd           The end of stream symbol decoded
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsDecodeLZ77FixedHuffFull_8u, (Ipp8u** ppSrc, int* pSrcLen, Ipp8u** ppDst, int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77DynamicHuffFull_8u
//  Purpose:            Performs the decoding of dynamic huffman rfc1951 compatible format
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector
//    ppDst             Double pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output
//    flush             Flush mode
//    pLZ77State        Pointer to internal decoding state
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsSrcSizeLessExpected The end of block symbol not decoded, so size of source vector less expected
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsStreamEnd           The end of stream symbol decoded
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsDecodeLZ77DynamicHuffFull_8u, (Ipp8u** ppSrc, int* pSrcLen, Ipp8u** ppDst, int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77CopyState_8u
//  Purpose:            Performs copying the fields of internal state structure
//
//  Parameters:
//    pLZ77StateSrc        Pointer to the internal state for copying from
//    pLZ77StateDst        Pointer to the internal state for copying to
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsDecodeLZ77CopyState_8u,   ( IppLZ77State_8u* pLZ77StateSrc, IppLZ77State_8u* pLZ77StateDst ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsLZ77Free
//  Purpose:            Frees the internal state structure for encoding or decoding.
//
//  Parameters:
//   pLZ77State         Pointer to the internal decoding structure.
//
*/
IPPAPI( void, ippsLZ77Free_8u, (IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsAdler32
//  Purpose:            Computes the adler32(ITUT V.42) checksum for the source vector.
//
//  Parameters:
//   pSrc               Pointer to the source vector.
//   srcLen             Length of the source vector.
//   pAdler32           Pointer to the checksum value.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pSrc pointer is NULL.
//   ippStsSizeErr      Indicates an error when the length of the source vector is less
//                      than or equal to zero.
//
*/
IPPAPI( IppStatus, ippsAdler32_8u, (const Ipp8u* pSrc, int srcLen, Ipp32u* pAdler32) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsCRC32
//  Purpose:            Computes the CRC32(ITUT V.42) checksum for the source vector.
//
//  Parameters:
//   pSrc               Pointer to the source vector.
//   srcLen             Length of the source vector.
//   pCRC32             Pointer to the checksum value.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pSrc pointer is NULL.
//   ippStsSizeErr      Indicates an error when the length of the source vector is less
//                      than or equal to zero.
//
*/
IPPAPI( IppStatus, ippsCRC32_8u, (const Ipp8u* pSrc, int srcLen, Ipp32u* pCRC32) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsCRC32C
//  Purpose:            Computes the CRC32C (the polinomial 0x11EDC6F41) value
//                      for the source vector.
//                      Reference: "Optimization of cyclic redundancy-check 
//                      codes with 24 and 32 parity bits". Castagnoli, G.;
//                      Brauer, S.; Herrmann, M.; Communications, 
//                      IEEE Transactions on Volume 41, Issue 6,
//                      June 1993 Page(s):883 - 892.
//
//  Parameters:
//   pSrc               Pointer to the source vector
//   srcLen             Length of the source vector
//   pCRC32C            Pointer to the CRC32C value
//  Return:
//   ippStsNoErr        No errors
//   ippStsNullPtrErr   One or several pointer(s) is NULL
//   ippStsSizeErr      Length of the source vector is equal zero
//
*/
IPPAPI( IppStatus, ippsCRC32C_8u, (const Ipp8u* pSrc, Ipp32u srcLen, Ipp32u* pCRC32C) )

#if !defined ( _OWN_BLDPCS )
typedef struct IppInflateState {
  const Ipp8u* pWindow;          /* pointer to the sliding window */
                                 /* (the dictionary for the LZ77 algorithm) */
  unsigned int winSize;          /* size of the sliding window */
  unsigned int tableType;        /* type of Huffman code tables */
                                 /* (for example, 0 - tables for Fixed Huffman codes) */
  unsigned int tableBufferSize;  /* (ENOUGH = 2048) * (sizeof(code) = 4) - */
                                 /* sizeof(IppInflateState) */
} IppInflateState;

typedef enum { /* this type is used as a translator of the inflate_mode type from zlib */
  ippTYPE,
  ippLEN,
  ippLENEXT
} IppInflateMode;
#endif

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsInflateBuildHuffTable
//  Purpose:            Builds literal/length and distance Huffman code table for
//                      decoding a block that was compressed with usage dynamic Huffman codes
//                      according to the "deflate" format (rfc1951)
//
//  Parameters:
//    pCodeLens         Pointer to the common array with literal/length and distance
//                      Huffman code lengths
//    nLitCodeLens      Number of literal/length Huffman code lengths
//    nDistCodeLens     Number of distance Huffman code lengths
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     nLitCodeLens is greater than 286, or nLitCodeLens is greater than 30
//                      (according to rfc1951)
//    ippStsSrcDataErr  Invalid literal/length and distance set has been met
//                      in the common lengths array
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsInflateBuildHuffTable, ( const Ipp16u* pCodeLens,
                                               unsigned int nLitCodeLens,
                                               unsigned int nDistCodeLens,
                                               IppInflateState *pIppInflateState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsInflate_8u
//  Purpose:            Decodes of the "deflate" format (rfc1951)
//                      according to the type of Huffman code tables
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of the source vector
//    pCode             Pointer to the bit buffer
//    pCodeLenBits      Number of valid bits in the bit buffer
//    winIdx            Index of the sliding window start position
//    ppDst             Double pointer to the destination vector
//    pDstLen           Pointer to the length of the destination vector
//    dstIdx            Index of the current position in the destination vector
//    pMode             Pointer to the current decode mode
//    pIppInflateState  Pointer to the structure that contains decode parameters
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     codeLenBits is greater than 32, or
//                      winIdx is greater than pIppInflateState->winSize, or
//                      dstIdx is greater than dstLen
//    ippStsSrcDataErr  Invalid literal/length and distance set has been met
//                      during decoding
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsInflate_8u, ( Ipp8u** ppSrc, unsigned int* pSrcLen,
                                    Ipp32u* pCode, unsigned int* pCodeLenBits,
                                    unsigned int winIdx,
                                    Ipp8u** ppDst, unsigned int* pDstLen, unsigned int dstIdx,
                                    IppInflateMode* pMode, IppInflateState *pIppInflateState ))

#if !defined( _OWN_BLDPCS )

typedef struct {
  Ipp16u freq;
  Ipp16u code;
} IppDeflateFreqTable;

#endif /* _OWN_BLDPCS */

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDeflateLZ77_8u
//  Purpose:            Perform LZ77 encoding according to
//                      the compression level
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of the source vector
//    pSrcIdx           Pointer to the index of the current position in
//                      the source vector. This parameter is used by 
//                      the function for correlation current possition of 
//                      the source vector and indexes in the hash tables.
//                      The normalization of this index and the hash tables 
//                      must only be done every 2GB of the source data
//                      instead of 64K (the zlib approach)
//    pWindow           Pointer to the sliding window, which is used as
//                      the dictionary for LZ77 encoding
//    winSize           Size of the window and the hash prev table
//    pHashHead         Pointer to heads of the hash chains. This table is
//                      initialized with (-winSize) value for correct processing
//                      of the first bytes of the source vector
//    pHashPrev         Pointer to links to older strings with the same
//                      hash index
//    hashSize          Size of the hash head table
//    pLitFreqTable     Pointer to the literals/lengths frequency table
//    pDistFreqTable    Pointer to the distances frequency table
//    pLitDst           Pointer to the literals/lengths destination vector
//    pDistDst          Pointer to the distances destination vector
//    pDstLen           Pointer to the length of the destination vectors
//    comprLevel        Compression level. It is like the zlib compression level
//    flush             Flush value
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsNoErr       No errors
//
*/

IPPAPI( IppStatus, ippsDeflateLZ77_8u, (
                     const Ipp8u** ppSrc, Ipp32u* pSrcLen, Ipp32u* pSrcIdx,
                     const Ipp8u* pWindow, Ipp32u winSize,
                     Ipp32s* pHashHead, Ipp32s* pHashPrev, Ipp32u hashSize,
                     IppDeflateFreqTable pLitFreqTable[286],
                     IppDeflateFreqTable pDistFreqTable[30],
                     Ipp8u* pLitDst, Ipp16u* pDistDst, Ipp32u* pDstLen,
                     int comprLevel, IppLZ77Flush flush ) )


#if !defined( _OWN_BLDPCS )

typedef struct {
  Ipp16u code;
  Ipp16u len;
} IppDeflateHuffCode;

#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
//  Name:              ippsDeflateHuff_8u
//  Purpose:           Performs Huffman encoding
//
//  Parameters:
//    pLitSrc          Pointer to the literals/lengths source vector
//    pDistSrc         Pointer to the distances source vector
//    pSrcLen          Pointer to the length of the source vectors
//    pCode            Pointer to the bit buffer
//    pCodeLenBits     Pointer to the number of valid bits in the bit buffer
//    pLitHuffCodes    Pointer to the literals/lengths Huffman codes
//    pDistHuffCodes   Pointer to the distances Huffman codes
//    pDst             Pointer to the destination vector
//    pDstIdx          Pointer to the index in the destination vector, the zlib
//                     uses the knowingly sufficient intermediate buffer for
//                     the Huffman encoding, so we need to know indexes of
//                     the first (input parameter) and the last (output parameter)
//                     symbols, which are written by the function
//  Return:
//    ippStsNullPtrErr One or several pointer(s) is NULL
//    ippStsNoErr      No errors
//
*/
IPPAPI( IppStatus, ippsDeflateHuff_8u, (
          const Ipp8u* pLitSrc, const Ipp16u* pDistSrc, Ipp32u srcLen,
          Ipp16u* pCode, Ipp32u* pCodeLenBits,
          IppDeflateHuffCode pLitHuffCodes[286],
          IppDeflateHuffCode pDistHuffCodes[30],
          Ipp8u* pDst, Ipp32u* pDstIdx ) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:              ippsDeflateDictionarySet_8u
//  Purpose:           Presets the user's dictionary for LZ77 encoding
//
//  Parameters:
//    pDictSrc         Pointer to the user's dictionary
//    dictLen          Length of the user's dictionary
//    pHashHeadDst     Pointer to heads of the hash chains
//    hashSize         Size of the hash head table
//    pHashPrevDst     Pointer to links to older strings with the same
//                     hash index
//    pWindowDst       Pointer to the sliding window, which is used as
//                     the dictionary for LZ77 encoding
//    winSize          Size of the window and the hash prev table in elements
//    comprLevel       Compression level. It is like the zlib compression level
//  Return:
//    ippStsNullPtrErr One or several pointer(s) is NULL
//    ippStsNoErr      No errors
//
*/
IPPAPI( IppStatus, ippsDeflateDictionarySet_8u, (
          const Ipp8u* pDictSrc, Ipp32u dictLen,
          Ipp32s* pHashHeadDst, Ipp32u hashSize, Ipp32s* pHashPrevDst,
          Ipp8u* pWindowDst, Ipp32u winSize, int comprLevel ) )

/* bzip2 - compatible functions */

#if !defined ( _OWN_BLDPCS )
struct RLEState_BZ2;
typedef struct RLEState_BZ2 IppRLEState_BZ2;
#endif

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsRLEGetSize_BZ2_8u
//  Purpose:            Calculates the size of internal state for bzip2-specific RLE.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    pRLEStateSize             Pointer to the size of internal state for bzip2-specific RLE
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsRLEGetSize_BZ2_8u,    ( int* pRLEStateSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeRLEInit_BZ2_8u
//  Purpose:            Initializes the elements of the bzip2-specific internal state for RLE.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    pRLEState         Pointer to internal state structure for bzip2 specific RLE
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsEncodeRLEInit_BZ2_8u,    ( IppRLEState_BZ2* pRLEState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeRLEInitAlloc_BZ2_8u
//  Purpose:            Allocates the memory and initializes the elements of the bzip2-specific internal state for RLE.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    ppRLEState        Double pointer to internal state structure for bzip2 specific RLE
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsEncodeRLEInitAlloc_BZ2_8u, ( IppRLEState_BZ2** ppRLEState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeRLE_BZ2_8u
//  Purpose:            Performs the RLE encoding with thresholding = 4.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output
//    pRLEState         Pointer to internal state structure for bzip2 specific RLE
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsEncodeRLE_BZ2_8u,    ( Ipp8u** ppSrc, int* pSrcLen, Ipp8u* pDst, int* pDstLen, IppRLEState_BZ2* pRLEState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeRLEFlush_BZ2_8u
//  Purpose:            Performs flushing the rest of data after RLE encoding with thresholding = 4.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output
//    pRLEState         Pointer to internal state structure for bzip2 specific RLE
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsEncodeRLEFlush_BZ2_8u,   ( Ipp8u* pDst, int* pDstLen, IppRLEState_BZ2* pRLEState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsRLEGetInUseTable_8u
//  Purpose:            Service function: gets the pointer to the inUse vector from internal state
//                      of type IppRLEState_BZ2. Specific function for bzip2 compatibility.
//
//  Parameters:
//    inUse             Pointer to the inUse vector
//    pRLEState         Pointer to internal state structure for bzip2 specific RLE
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsRLEGetInUseTable_8u,    ( Ipp8u inUse[256], IppRLEState_BZ2* pRLEState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsRLEFree_BZ2_8u
//  Purpose:            Frees the bzip2-specific internal state for RLE.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    pRLEState         Pointer to internal state structure for bzip2 specific RLE.
//
*/

IPPAPI(void, ippsRLEFree_BZ2_8u,    ( IppRLEState_BZ2* pRLEState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeRLE_BZ2_8u
//  Purpose:            Performs the RLE decoding with thresholding = 4.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector on input,
//                      pointer to the size of remainder on output
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output.
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsDecodeRLE_BZ2_8u,    (Ipp8u** ppSrc, int* pSrcLen, Ipp8u* pDst, int* pDstLen ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeZ1Z2_BZ2_8u16u
//  Purpose:            Performs the Z1Z2 encoding.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector on input,
//                      pointer to the size of remainder on output
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output.
//    freqTable[258]    Table of frequencies collected for alphabet symbols.
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsEncodeZ1Z2_BZ2_8u16u, ( Ipp8u** ppSrc, int* pSrcLen, Ipp16u* pDst, int* pDstLen, int freqTable[258] ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeZ1Z2_BZ2_16u8u
//  Purpose:            Performs the Z1Z2 decoding.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector on input,
//                      pointer to the size of remainder on output
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output.
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsDecodeZ1Z2_BZ2_16u8u, ( Ipp16u** ppSrc, int* pSrcLen, Ipp8u* pDst, int* pDstLen ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsReduceDictionary_8u_I
//  Purpose:            Performs the dictionary reducing.
//
//  Parameters:
//    inUse[256]        Table of 256 values of Ipp8u type.
//    pSrcDst           Pointer to the source/destination vector
//    srcDstLen         Length of source/destination vector.
//    pSizeDictionary   Pointer to the size of dictionary on input and to the size
//                      of reduced dictionary on output.
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsReduceDictionary_8u_I,   ( const Ipp8u inUse[256], Ipp8u* pSrcDst, int srcDstLen, int* pSizeDictionary ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsExpandDictionary_8u_I
//  Purpose:            Performs the dictionary expanding.
//
//  Parameters:
//    inUse[256]        Table of 256 values of Ipp8u type.
//    pSrcDst           Pointer to the source/destination vector
//    srcDstLen         Length of source/destination vector.
//    sizeDictionary    The size of reduced dictionary on input.
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsExpandDictionary_8u_I,   ( const Ipp8u inUse[256], Ipp8u* pSrcDst, int srcDstLen, int sizeDictionary ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsCRC32_BZ2_8u
//  Purpose:            Performs the CRC32 checksum calculation according to the direct algorithm, which is used in bzip2.
//
//  Parameters:
//    pSrc              Pointer to the source data vector
//    srcLen            The length of source vector
//    pCRC32            Pointer to the value of accumulated CRC32
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Length of the source vector is less or equal zero
//    ippStsNoErr               No errors
//
*/

IPPAPI(IppStatus, ippsCRC32_BZ2_8u,    ( const Ipp8u* pSrc, int srcLen, Ipp32u* pCRC32 ))

#if !defined ( _OWN_BLDPCS )
struct EncodeHuffState_BZ2;
typedef struct EncodeHuffState_BZ2 IppEncodeHuffState_BZ2;

struct DecodeHuffState_BZ2;
typedef struct DecodeHuffState_BZ2 IppDecodeHuffState_BZ2;
#endif

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeHuffGetSize_BZ2_16u8u
//  Purpose:            Calculates the size of internal state for bzip2-specific Huffman coding.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    wndSize                          Size of the block to be processed
//    pEncodeHuffStateSize             Pointer to the size of internal state for bzip2-specific Huffman coding
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsEncodeHuffGetSize_BZ2_16u8u,   ( int wndSize, int* pEncodeHuffStateSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeHuffInit_BZ2_16u8u
//  Purpose:            Initializes the elements of the bzip2-specific internal state for Huffman coding.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    sizeDictionary     The size of the dictionary
//    freqTable          Table of frequencies of symbols
//    pSrc               Pointer to the source vector
//    srcLen             Length of the source vector
//    pEncodeHuffState   Pointer to internal state structure for bzip2 specific Huffman coding
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsEncodeHuffInit_BZ2_16u8u,      ( int sizeDictionary, const int freqTable[258], const Ipp16u* pSrc, int srcLen,
                                                       IppEncodeHuffState_BZ2* pEncodeHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeHuffInitAlloc_BZ2_16u8u
//  Purpose:            Allocates the memory and initializes the elements of the bzip2-specific internal state for Huffman coding.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    wndSize            The size of the input block in bytes
//    sizeDictionary     The size of the dictionary
//    freqTable          Table of frequencies of symbols
//    pSrc               Pointer to the source vector
//    srcLen             Length of the source vector
//    pEncodeHuffState   Pointer to internal state structure for bzip2 specific Huffman coding
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsEncodeHuffInitAlloc_BZ2_16u8u, ( int wndSize, int sizeDictionary, const int freqTable[258],
                                                       const Ipp16u* pSrc, int srcLen, IppEncodeHuffState_BZ2** ppEncodeHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsPackHuffContext_BZ2_16u8u
//  Purpose:            Performs the bzip2-specific encoding of Huffman context.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    pCode             Pointer to the bit buffer
//    pCodeLenBits      Number of valid bits in the bit buffer
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output
//    pEncodeHuffState  Pointer to internal state structure for bzip2 specific Huffman coding
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsPackHuffContext_BZ2_16u8u,     ( Ipp32u* pCode, int* pCodeLenBits, Ipp8u* pDst, int* pDstLen,
                                                       IppEncodeHuffState_BZ2* pEncodeHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeHuff_BZ2_16u8u
//  Purpose:            Performs the bzip2-specific Huffman encoding.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    pCode             Pointer to the bit buffer
//    pCodeLenBits      Number of valid bits in the bit buffer
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output
//    pEncodeHuffState  Pointer to internal state structure for bzip2 specific Huffman coding
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsEncodeHuff_BZ2_16u8u,          ( Ipp32u* pCode, int* pCodeLenBits, Ipp16u** ppSrc, int* pSrcLen,
                                                       Ipp8u* pDst, int* pDstLen, IppEncodeHuffState_BZ2* pEncodeHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeHuffFree_BZ2_16u8u
//  Purpose:            Frees the bzip2-specific internal state for Huffman coding.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    pEncodeHuffState         Pointer to internal state structure for bzip2 specific Huffman coding.
//
*/
IPPAPI(void,      ippsEncodeHuffFree_BZ2_16u8u,            ( IppEncodeHuffState_BZ2* pEncodeHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeHuffGetSize_BZ2_8u16u
//  Purpose:            Calculates the size of internal state for bzip2-specific Huffman decoding.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    wndSize                    Size of the block to be processed
//    pDecodeHuffStateSize       Pointer to the size of internal state for bzip2-specific Huffman decoding
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsDecodeHuffGetSize_BZ2_8u16u,   ( int wndSize, int* pDecodeHuffStateSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeHuffInit_BZ2_8u16u
//  Purpose:            Initializes the elements of the bzip2-specific internal state for Huffman decoding.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    sizeDictionary           The size of the dictionary
//    pDecodeHuffState         Pointer to internal state structure for bzip2 specific Huffman decoding
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsDecodeHuffInit_BZ2_8u16u,      ( int sizeDictionary, IppDecodeHuffState_BZ2* pDecodeHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeHuffInitAlloc_BZ2_8u16u
//  Purpose:            Allocates the memory and initializes the elements of the bzip2-specific internal state for Huffman decoding.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    wndSize            The size of the input block in bytes
//    sizeDictionary     The size of the dictionary
//    pDecodeHuffState         Pointer to internal state structure for bzip2 specific Huffman coding
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsDecodeHuffInitAlloc_BZ2_8u16u, ( int wndSize, int sizeDictionary, IppDecodeHuffState_BZ2** ppDecodeHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsUnpackHuffContext_BZ2_8u16u
//  Purpose:            Performs the bzip2-specific decoding of Huffman context.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    pCode                   Pointer to the bit buffer
//    pCodeLenBits            Number of valid bits in the bit buffer
//    pSrc                    Pointer to the destination vector
//    pSrcLen                 Pointer to the size of destination buffer on input,
//                            pointer to the resulting length of the destination vector
//                            on output
//    pDecodeHuffState        Pointer to internal state structure for bzip2 specific Huffman decoding.
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsUnpackHuffContext_BZ2_8u16u,   ( Ipp32u* pCode, int* pCodeLenBits, Ipp8u** ppSrc, int* pSrcLen,
                                                       IppDecodeHuffState_BZ2* pDecodeHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeHuff_BZ2_8u16u
//  Purpose:            Performs the bzip2-specific Huffman decoding.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    pCode                   Pointer to the bit buffer
//    pCodeLenBits            Number of valid bits in the bit buffer
//    ppSrc                   Double pointer to the source vector
//    pSrcLen                 Pointer to the length of source vector
//    pDst                    Pointer to the destination vector
//    pDstLen                 Pointer to the size of destination buffer on input,
//                            pointer to the resulting length of the destination vector
//                            on output
//    pDecodeHuffState        Pointer to internal state structure for bzip2 specific Huffman decoding.
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsDecodeHuff_BZ2_8u16u,          ( Ipp32u* pCode, int* pCodeLenBits, Ipp8u** ppSrc, int* pSrcLen,
                                                       Ipp16u* pDst, int* pDstLen, IppDecodeHuffState_BZ2* pDecodeHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeHuffFree_BZ2_8u16u
//  Purpose:            Frees the bzip2-specific internal state for Huffman decoding.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//  pDecodeHuffState    Pointer to internal state structure for bzip2 specific Huffman coding.
//
*/
IPPAPI(void,      ippsDecodeHuffFree_BZ2_8u16u,      ( IppDecodeHuffState_BZ2* pDecodeHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsVLCEncodeFree_32s
//  Purpose:            Frees memory allocated for internal VLCDecode structure.
//
//  Arguments:
//     pVLCSpec         Pointer to pointer to VLCEncoder specification structure.
//
//  Return:
//
*/
IPPAPI(void, ippsVLCEncodeFree_32s, (IppsVLCEncodeSpec_32s* pVLCSpec))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCEncodeInitAlloc_32s
//  Purpose:    ippsVLCEncodeInitAlloc_32s allocates and initializes the size
//              for internal VLCEncode structure on the base of Variable Length Code table.
//
//  Arguments:
//     pInputTable                  pointer to input table.
//     inputTableSize               size of this table.
//     ppVLCSpec                    pointer to pointer to VLCEncoder specification structure.
//
//  Return:
//     ippStsNoErr                  Indicates no error.
//     ippStsNullPtrErr             Indicates an error when one or more pointers
//                                  passed to the function is NULL.
//     ippStsVLCInputDataErr        Indicates an error when incorrect input is used.
//     ippStsMemAllocErr            Indicates an error when memory for VLCEncoder
//                                  specification structure was not allocated.
//
*/
IPPAPI(IppStatus, ippsVLCEncodeInitAlloc_32s, (const IppsVLCTable_32s* pInputTable, int inputTableSize, IppsVLCEncodeSpec_32s** ppVLCSpec))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCEncodeInit_32s
//  Purpose:    ippsVLCEncodeInit_32s initializes the size for internal VLCEncode
//              structure on the base of Variable Length Code table.
//
//  Arguments:
//     pInputTable                  pointer to input table.
//     inputTableSize               size of this table.
//     pVLCSpec                     pointer to VLCEncoder specification structure.
//
//  Return:
//     ippStsNoErr                  Indicates no error.
//     ippStsNullPtrErr             Indicates an error when one or more pointers
//                                  passed to the function is NULL.
//     ippStsVLCInputDataErr        Indicates an error when incorrect input is used.
//
*/
IPPAPI(IppStatus, ippsVLCEncodeInit_32s, (const IppsVLCTable_32s* pInputTable, int inputTableSize, IppsVLCEncodeSpec_32s* pVLCSpec))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCEncodeGetSize_32s
//  Purpose:    ippsVLCEncodeGetSize_32s calculates the size for internal VLCEncode
//              structure on the base of Variable Length Code table.
//
//  Arguments:
//     pInputTable                  pointer to input table.
//     inputTableSize               size of this table.
//     pSize                        pointer to size of VLCEncoder specification structure.
//
//  Return:
//     ippStsNoErr                  Indicates no error.
//     ippStsNullPtrErr             Indicates an error when one or more pointers
//                                  passed to the function is NULL.
//     ippStsVLCInputDataErr        Indicates an error when incorrect input is used.
//
*/
IPPAPI(IppStatus, ippsVLCEncodeGetSize_32s, (const IppsVLCTable_32s* pInputTable, int inputTableSize, Ipp32s* pSize))

/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:       ippsVLCEncodeBlock_16s1u
//  Purpose:    ippsVLCEncodeBlock_16s1u encodes dstLen elements from the source
//              data pSrc and stores the result in the destination buffer *ppDst.
//              In addition, the function advances pointer *ppDst on the number
//              successfully written bytes and stores the actual it in ppDst.
//              The functions updates *pSrcBitsOffset to the value of the actual
//              bit offset in the source buffer *ppDst.
//
//  Arguments:
//     pSrc                         pointer to source values array.
//     srcLen                       size of value is array pSrc.
//     ppDst                        pointer to pointer to destination bitstream.
//     pDstBitsOffset               pointer to in/out bit offset in pDst.
//     pVLCSpec                     pointer to VLCEncoder specification structure.
//
//  Return:
//     ippStsNoErr                  Indicates no error.
//     ippStsNullPtrErr             Indicates an error when one or more pointers
//                                  passed to the function is NULL.
//     ippStsVLCInputDataErr        Indicates an error when incorrect input is used.
//
*/
IPPAPI(IppStatus, ippsVLCEncodeBlock_16s1u, (const Ipp16s* pSrc, int srcLen, Ipp8u** ppDst, int* pDstBitsOffset, const IppsVLCEncodeSpec_32s* pVLCSpec))

/* /////////////////////////////////////////////////////////////////////////////
//
//  Name:       ippsVLCEncodeOne_16s1u
//  Purpose:    ippsVLCEncodeOne_16s1u uses VLC table specified for
//              ippsVLCEncodeInitAlloc_32s function or for ippsVLCEncodeInit_32s function.
//
//  Arguments:
//     src                          source value.
//     ppDst                        pointer to pointer to destination bitstream.
//     pDstBitsOffset               pointer to in/out bit offset in pDst.
//     pVLCSpec                     pointer to VLCEncoder specification structure.
//
//  Return:
//     ippStsNoErr                  Indicates no error.
//     ippStsNullPtrErr             Indicates an error when one or more pointers
//                                  passed to the function is NULL.
//     ippStsVLCInputDataErr        Indicates an error when incorrect input is used.
//
*/
IPPAPI(IppStatus, ippsVLCEncodeOne_16s1u, (Ipp16s src, Ipp8u** pDst, int* pDstBitsOffset, const IppsVLCEncodeSpec_32s* pVLCSpec))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCCountBits_16s32s
//  Purpose:    ippsVLCCountBits_16s32s calculates number of bits necessary for encoding
//              source data in pSrc  using variable Length Codes specified by pInputTable
//              in the function ippsVLCEncodeInitAlloc_32s or in ippsVLCEncodeInit_32s.
//
//  Arguments:
//     pVLCSpec                     pointer to pointer to VLCEncoder specification structure.
//
//  Return:
//     ippStsNoErr                  Indicates no error.
//     ippStsNullPtrErr             Indicates an error when one or more pointers
//                                  passed to the function is NULL.
//     ippStsVLCInputDataErr        Indicates an error when incorrect input is used.
//
*/
IPPAPI(IppStatus, ippsVLCCountBits_16s32s, (const Ipp16s* pSrc, int srcLen, Ipp32s* pCountBits, const IppsVLCEncodeSpec_32s* pVLCSpec))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCDecodeGetSize_32s
//  Purpose:    calculates the size for internal VLCDecode structure on
//              the base of input parameters
//
//  Arguments:
//     pInputTable                   pointer to input table.
//     inputTableSize                size of this table
//     pSubTablesSizes               sizes of subTables
//     numSubTables                  num of Subtabtes.
//     pSize                         pointer to size of VLCDecoder specification
//                                   structure
//
//  Return:
//     ippStsNoErr                   Indicates no error.
//     ippStsNullPtrErr              Indicates an error when one or more pointers
//                                   passed to the function is NULL.
//     ippStsVLCUsrTblCodeLengthErr  Indicates an error when 1) the maximal length
//                                   of codec in the input table exceeds 32;
//                                   2) when any size in of subtables in
//                                   pSubTablesSizes is less than one;
//                                   3)when sum of this values is less than the
//                                   maximal length of codes in the input table.
//
*/
IPPAPI(IppStatus, ippsVLCDecodeGetSize_32s,(const IppsVLCTable_32s *pInputTable,
                                            int inputTableSize,
                                            Ipp32s *pSubTablesSizes,
                                            int numSubTables,
                                            Ipp32s *pSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCDecodeInit_32s
//  Purpose:    initializes internal VLCDecode structure
//              based on the input parameters
//
//  Arguments:
//     pInputTable                   pointer to input table.
//     inputTableSize                size of this table
//     pSubTablesSizes               sizes of subTables
//     numSubTables                  num of Subtabtes.
//     pVLCSpec                      pointer to VLCDecoder specification structure
//
//  Return:
//     ippStsNoErr                   Indicates no error.
//     ippStsNullPtrErr              Indicates an error when one or more pointers
//                                   passed to the function is NULL.
//     ippStsVLCUsrTblCodeLengthErr  Indicates an error when 1) the maximal length
//                                   of codec in the input table exceeds 32;
//                                   2) when any size in of subtables in
//                                   pSubTablesSizes is less than one;
//                                   3)when sum of this values is less than the
//                                   maximal length of codes in the input table.
//
*/
IPPAPI(IppStatus, ippsVLCDecodeInit_32s,(const IppsVLCTable_32s *pInputTable,
                                         int inputTableSize,
                                         Ipp32s *pSubTablesSizes,
                                         int numSubTables,
                                         IppsVLCDecodeSpec_32s *pVLCSpec))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCDecodeInitAlloc_32s
//  Purpose:    allocates and initializes internal VLCDecode structure
//              based on the input parameters
//
//  Arguments:
//     pInputTable                   pointer to input table.
//     inputTableSize                size of this table
//     pSubTablesSizes               sizes of subTables
//     numSubTables                  num of Subtabtes.
//     ppVLCSpec                     pointer to pointer VLCDecoder specification structure
//
//  Return:
//     ippStsNoErr                   Indicates no error.
//     ippStsNullPtrErr              Indicates an error when one or more pointers
//                                   passed to the function is NULL.
//     ippStsVLCUsrTblCodeLengthErr  Indicates an error when 1) the maximal length
//                                   of codec in the input table exceeds 32;
//                                   2) when any size in of subtables in
//                                   pSubTablesSizes is less than one;
//                                   3)when sum of this values is less than the
//                                   maximal length of codes in the input table.
//     ippStsMemAllocErr             Indicates an error when memory for VLCDecoder
//                                   specification structure was not allocated.
//
*/
IPPAPI(IppStatus, ippsVLCDecodeInitAlloc_32s,(const IppsVLCTable_32s *pInputTable,
                                              int inputTableSize,
                                              Ipp32s *pSubTablesSizes,
                                              int numSubTables,
                                              IppsVLCDecodeSpec_32s **ppVLCSpec))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCDecodeFree_32s
//  Purpose:    frees memory allocated for internal VLCDecode structure
//
//  Arguments:
//     pVLCSpec         pointer to VLCDecoder specification structure
//
//  Return:
//
*/
IPPAPI(void, ippsVLCDecodeFree_32s, (IppsVLCDecodeSpec_32s *pVLCSpec))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCDecodeBlock_1u16s
//  Purpose:    decodes a dstLen element encoded by VLC code from the source data
//              *ppSrc with *pSrcBitsOffset bits offset and stores the result in
//              the destination *pDst
//
//  Arguments:
//     ppSrc            pointer to pointer to source input bitstream
//     pSrcBitsOffset   pointer to in/out source stream bit offset position
//     pDst             decoded values array
//     dstLen           number of values to decode into array pDst
//     pVLCSpec         pointer to VLCDecoder specification structure
//
//  Return:
//     ippStsNoErr            Indicates no error.
//     ippStsNullPtrErr       Indicates an error when one or more pointers passed to
//                            the function is NULL.
//     ippStsVLCInputDataErr  Indicates an error when incorrect input is used.
//                            It can indicate that bitstream contain code that is not
//                            specified inside the used table.
//     ippStsBitOffsetErr     Indicate an error when offset less than 0 or more than 7.
//     ippStsContextMatchErr  Indicate an error when pVLCSpec struct was not created by
//                            ippsVLCDecodeInit_32s or ippsVLCDecodeInitAlloc_32s functions.
//
*/
IPPAPI(IppStatus, ippsVLCDecodeBlock_1u16s,(Ipp8u **ppSrc,
                                            int *pSrcBitsOffset,
                                            Ipp16s *pDst,
                                            int dstLen,
                                            const IppsVLCDecodeSpec_32s *pVLCSpec))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCDecodeOne_1u16s
//  Purpose:    decodes a single element encoded by VLC code from the source data
//              *ppSrc with *pSrcBitsOffset bits offset and stores the result in
//              the destination *pDst
//
//  Arguments:
//     ppSrc            pointer to pointer to source input bitstream
//     pSrcBitsOffset   pointer to in/out source stream bit offset position
//     pDst             decoded value
//     pVLCSpec         pointer to VLCDecoder specification structure
//
//  Return:
//     ippStsNoErr            Indicates no error.
//     ippStsNullPtrErr       Indicates an error when one or more pointers passed to
//                            the function is NULL.
//     ippStsVLCInputDataErr  Indicates an error when incorrect input is used.
//                            It can indicate that bitstream contain code that is not
//                            specified inside the used table.
//     ippStsBitOffsetErr     Indicate an error when offset less than 0 or more than 7.
//     ippStsContextMatchErr  Indicate an error when pVLCSpec struct was not created by
//                            ippsVLCDecodeInit_32s or ippsVLCDecodeInitAlloc_32s functions.
//
*/
IPPAPI(IppStatus, ippsVLCDecodeOne_1u16s,(Ipp8u **ppSrc,
                                          int *pSrcBitsOffset,
                                          Ipp16s *pDst,
                                          const IppsVLCDecodeSpec_32s *pVLCSpec))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCDecodeUTupleGetSize_32s
//  Purpose:    calculates the size for internal VLCDecode structure on
//              the base of input parameters
//
//  Arguments:
//     pInputTable                   pointer to input table.
//     inputTableSize                size of this table
//     pSubTablesSizes               sizes of subTables
//     numSubTables                  num of Subtabtes.
//     numElements                   number of elements in tuple
//     numValueBit                   size of one element value, bit.
//     pSize                         pointer to size of VLCDecoder specification
//                                   structure
//
//  Return:
//     ippStsNoErr                   Indicates no error.
//     ippStsNullPtrErr              Indicates an error when one or more pointers
//                                   passed to the function is NULL.
//     ippStsVLCUsrTblCodeLengthErr  Indicates an error when 1) the maximal length
//                                   of codec in the input table exceeds 32;
//                                   2) when any size in of subtables in
//                                   pSubTablesSizes is less than one;
//                                   3)when sum of this values is less than the
//                                   maximal length of codes in the input table.
//
*/

IPPAPI(IppStatus, ippsVLCDecodeUTupleGetSize_32s,(const IppsVLCTable_32s  *pInputTable,
                                                  int               inputTableSize,
                                                  Ipp32s            *pSubTablesSizes,
                                                  int               numSubTables,
                                                  int               numElements,
                                                  int               numValueBit,
                                                  Ipp32s            *pSize))

/*******************************************************************/

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCDecodeUTupleInit_32s
//  Purpose:    initializes internal VLCDecode structure
//              based on the input parameters
//
//  Arguments:
//     pInputTable                   pointer to input table.
//     inputTableSize                size of this table
//     pSubTablesSizes               sizes of subTables
//     numSubTables                  num of Subtabtes.
//     numElements                   number of elements in tuple
//     numValueBit                   size of one element value, bit.
//     pVLCSpec                      pointer to VLCDecoder specification structure
//
//  Return:
//     ippStsNoErr                   Indicates no error.
//     ippStsNullPtrErr              Indicates an error when one or more pointers
//                                   passed to the function is NULL.
//     ippStsVLCUsrTblCodeLengthErr  Indicates an error when 1) the maximal length
//                                   of codec in the input table exceeds 32;
//                                   2) when any size in of subtables in
//                                   pSubTablesSizes is less than one;
//                                   3)when sum of this values is less than the
//                                   maximal length of codes in the input table.
//
*/

IPPAPI(IppStatus, ippsVLCDecodeUTupleInit_32s,(const  IppsVLCTable_32s      *pInputTable,
                                                int                   inputTableSize,
                                                Ipp32s                *pSubTablesSizes,
                                                int                   numSubTables,
                                                int                   numElements,
                                                int                   numValueBit,
                                                IppsVLCDecodeUTupleSpec_32s *pVLCSpec))

/*******************************************************************/

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCDecodeUTupleInitAlloc_32s
//  Purpose:    allocates and initializes internal VLCDecode structure
//              based on the input parameters
//
//  Arguments:
//     pInputTable                   pointer to input table.
//     inputTableSize                size of this table
//     pSubTablesSizes               sizes of subTables
//     numSubTables                  num of Subtabtes.
//     numElements                   number of elements in tuple
//     numValueBit                   size of one element value, bit.
//     ppVLCSpec                     pointer to pointer VLCDecoder specification structure
//
//  Return:
//     ippStsNoErr                   Indicates no error.
//     ippStsNullPtrErr              Indicates an error when one or more pointers
//                                   passed to the function is NULL.
//     ippStsVLCUsrTblCodeLengthErr  Indicates an error when 1) the maximal length
//                                   of codec in the input table exceeds 32;
//                                   2) when any size in of subtables in
//                                   pSubTablesSizes is less than one;
//                                   3)when sum of this values is less than the
//                                   maximal length of codes in the input table.
//     ippStsMemAllocErr             Indicates an error when memory for VLCDecoder
//                                   specification structure was not allocated.
//
*/

IPPAPI(IppStatus, ippsVLCDecodeUTupleInitAlloc_32s,(const IppsVLCTable_32s      *pInputTable,
                                                    int                   inputTableSize,
                                                    Ipp32s                *pSubTablesSizes,
                                                    int                   numSubTables,
                                                    int                   numElements,
                                                    int                   numValueBit,
                                                    IppsVLCDecodeUTupleSpec_32s **ppVLCSpec))

/*******************************************************************/

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCDecodeUTupleFree_32s
//  Purpose:    frees memory allocated for internal VLCDecode structure
//
//  Arguments:
//     pVLCSpec         pointer to VLCDecoder specification structure
//
//  Return:
//
*/

IPPAPI(void, ippsVLCDecodeUTupleFree_32s,(IppsVLCDecodeUTupleSpec_32s *pVLCSpec))



/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCDecodeUTupleBlock_1u16s
//  Purpose:    decodes a dstLen element encoded by VLC code from the source data
//              *ppSrc with *pSrcBitsOffset bits offset and stores the result in
//              the destination *pDst
//
//  Arguments:
//     ppSrc            pointer to pointer to source input bitstream
//     pSrcBitsOffset   pointer to in/out source stream bit offset position
//     pDst             decoded values array
//     dstLen           number of values to decode into array pDst
//     pVLCSpec         pointer to VLCDecoder specification structure
//
//  Return:
//     ippStsNoErr            Indicates no error.
//     ippStsNullPtrErr       Indicates an error when one or more pointers passed to
//                            the function is NULL.
//     ippStsVLCInputDataErr  Indicates an error when incorrect input is used.
//                            It can indicate that bitstream contain code that is not
//                            specified inside the used table.
//     ippStsBitOffsetErr     Indicate an error when offset less than 0 or more than 7.
//     ippStsContextMatchErr  Indicate an error when pVLCSpec struct was not created by
//                            ippsVLCDecodeInit_32s or ippsVLCDecodeInitAlloc_32s functions.
//
*/

IPPAPI(IppStatus, ippsVLCDecodeUTupleBlock_1u16s,(Ipp8u                       **ppSrc,
                                            int                         *pSrcBitsOffset,
                                            Ipp16s                      *pDst,
                                            int                         dstLen,
                                            const IppsVLCDecodeUTupleSpec_32s *pVLCSpec))

/*******************************************************************/

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsVLCDecodeUTupleOne_1u16s
//  Purpose:    decodes a single element encoded by VLC code from the source data
//              *ppSrc with *pSrcBitsOffset bits offset and stores the result in
//              the destination *pDst
//
//  Arguments:
//     ppSrc            pointer to pointer to source input bitstream
//     pSrcBitsOffset   pointer to in/out source stream bit offset position
//     pDst             decoded value
//     pVLCSpec         pointer to VLCDecoder specification structure
//
//  Return:
//     ippStsNoErr            Indicates no error.
//     ippStsNullPtrErr       Indicates an error when one or more pointers passed to
//                            the function is NULL.
//     ippStsVLCInputDataErr  Indicates an error when incorrect input is used.
//                            It can indicate that bitstream contain code that is not
//                            specified inside the used table.
//     ippStsBitOffsetErr     Indicate an error when offset less than 0 or more than 7.
//     ippStsContextMatchErr  Indicate an error when pVLCSpec struct was not created by
//                            ippsVLCDecodeInit_32s or ippsVLCDecodeInitAlloc_32s functions.
//
*/

IPPAPI(IppStatus, ippsVLCDecodeUTupleOne_1u16s,(Ipp8u                        **ppSrc,
                                           int                          *pSrcBitsOffset,
                                           Ipp16s                       *pDst,
                                           const IppsVLCDecodeUTupleSpec_32s  *pVLCSpec))
/**************************************************
IPP LZO Definitions
***************************************************/
#if !defined ( _OWN_BLDPCS )
typedef enum {
    IppLZO1XST,      /* Single-threaded, generic LZO-compatible*/
    IppLZO1XMT      /* Multi-threaded */
} IppLZOMethod ;
struct LZOState_8u;
typedef struct LZOState_8u IppLZOState_8u;
#endif

/*******************************************************************/

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsEncodeLZOGetSize
//  Purpose:    returns structure size necessary for compression
//
//  Arguments:
//     method           LZO method to be used during compression
//     maxInputLen      maximum length of input buffer, which will be processed by Encode
//     pSize            pointer to size variable
//
//  Return:
//      ippStsBadArgErr          illegal method
//      ippStsNullPtrErr         NULL pointer detected
//      ippStsNoErr              no error
//
*/
IPPAPI(IppStatus, ippsEncodeLZOGetSize, (IppLZOMethod method, Ipp32u maxInputLen, Ipp32u *pSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsEncodeLZO_8u
//  Purpose:    compresses specified input buffer
//
//  Arguments:
//      pSrc                   input data address
//      srcLen                 input data length
//      pDst                   output buffer address
//      pDstLen                pointer to resulting length variable, must contain output buffer length upon start
//      pLZOState              pointer to IppLZOState structure variable
//
//  Return:
//      ippStsNullPtrErr            one of the pointers is NULL
//      ippStsDstSizeLessExpected   output buffer is too short for compressed data
//      ippStsNoErr                 no error detected
//
*/
IPPAPI(IppStatus, ippsEncodeLZO_8u, (const Ipp8u *pSrc, Ipp32u srcLen, Ipp8u *pDst, Ipp32u *pDstLen, IppLZOState_8u *pLZOState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsEncodeLZOInit
//  Purpose:    initializes IppLZOSate_8u structure
//
//  Arguments:
//      method                  LZO compression method desired
//      maxInputLen             maximum length of input buffer, which will be processed by Encode
//      pLZOState               pointer to IppLZOState structure variable
//
//  Return:
//      ippStsNullPtrErr            one of the pointers is NULL
//      ippStsBadArgErr             illegal method
//      ippStsNoErr                 no error detected
//
*/
IPPAPI(IppStatus, ippsEncodeLZOInit_8u, (IppLZOMethod method, Ipp32u maxInputLen, IppLZOState_8u *pLZOState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsDecodeLZO_8u
//      Purpose:    decompresses specified input buffer to output buffer, returns decompressed data length
//  Name:       ippsDecodeLZOSafe_8u
//      Purpose:    decompresses specified input buffer to output buffer with checking output buffer boundaries, returns decompressed data length
//
//  Arguments:
//      pSrc                  pointer to input buffer
//      srcLen                input data length
//      pDst                  pointer to output buffer
//      pDstLen               pointer to output data length variable. Initially contains output buffer length
//
//  Return:
//      ippStsNullPtrErr            one of the pointers is NULL
//      ippStsDstSizeLessExpected   output buffer is too short for compressed data
//      ippStsSrcSizeLessExpected   input buffer data is not complete, i.e. no EOF found
//      ippStsBrokenLzoStream       ippsDecodeLZOSafe_8u detected output buffer boundary violation
//      ippStsNoErr                 no error detected
//
*/
IPPAPI(IppStatus, ippsDecodeLZO_8u, (const Ipp8u *pSrc, Ipp32u srcLen, Ipp8u *pDst, Ipp32u *pDstLen))
IPPAPI(IppStatus, ippsDecodeLZOSafe_8u, (const Ipp8u *pSrc, Ipp32u srcLen, Ipp8u *pDst, Ipp32u *pDstLen))

#ifdef __cplusplus
}
#endif

#if defined (_IPP_STDCALL_CDECL)
  #undef  _IPP_STDCALL_CDECL
  #define __stdcall __cdecl
#endif

#endif /* __IPPDC_H__ */
/* ////////////////////////////// End of file /////////////////////////////// */

