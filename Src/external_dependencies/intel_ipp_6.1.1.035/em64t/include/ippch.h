/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2002-2009 Intel Corporation. All Rights Reserved.
//
//
//                  Intel(R) Integrated Performance Primitives
//                  String Manipulations Library (ippCH)
//
*/
#if !defined( __IPPCH_H__ ) || defined( _OWN_BLDPCS )
#define __IPPCH_H__

#ifndef __IPPDEFS_H__
  #include "ippdefs.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined( _OWN_BLDPCS )

#if defined (_WIN32_WCE) && defined (_M_IX86) && defined (__stdcall)
  #define _IPP_STDCALL_CDECL
  #undef __stdcall
#endif


typedef struct {
 void *pFind;
 int lenFind;
} IppRegExpFind;

struct RegExpState;
typedef struct RegExpState IppRegExpState;

typedef enum {
    ippFmtASCII = 0,
    ippFmtUTF8
} IppRegExpFormat;

#endif /* _OWN_BLDPCS */


/* /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                   Functions declarations
////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////// */


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippchGetLibVersion
//  Purpose:    getting of the library version
//  Returns:    the structure of information about version
//              of ippCH library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
IPPAPI( const IppLibraryVersion*, ippchGetLibVersion, (void) )

/* /////////////////////////////////////////////////////////////////////////////
//                String Functions
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFind_8u        ippsFind_16u
//              ippsFindC_8u       ippsFindC_16u
//              ippsFindRev_8u     ippsFindRev_16u
//              ippsFindRevC_8u    ippsFindRevC_16u
//
//  Purpose:    Finds the match for string of elements or single element
//              within source string in direct or reverse direction
//
//  Arguments:
//     pSrc    - pointer to the source string
//     len     - source string length
//     pFind   - pointer to the searching string
//     lenFind - searching string length
//     valFind - searching element
//     pIndex  - pointer to the result index:
//               *pIndex = index of first occurrence ;
//               *pIndex = -1 if no match;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, pFind or pIndex are NULL
//   ippStsLengthErr   len or lenFind are negative
*/
IPPAPI (IppStatus, ippsFind_8u,  (const Ipp8u* pSrc, int len,
                                  const Ipp8u* pFind, int lenFind,
                                  int *pIndex))
IPPAPI (IppStatus, ippsFind_16u, (const Ipp16u* pSrc, int len,
                                  const Ipp16u* pFind, int lenFind,
                                  int *pIndex))
IPPAPI (IppStatus, ippsFindC_8u,  (const Ipp8u* pSrc, int len,
                                   Ipp8u valFind, int *pIndex))
IPPAPI (IppStatus, ippsFindC_16u, (const Ipp16u* pSrc, int len,
                                   Ipp16u valFind, int *pIndex))

IPPAPI (IppStatus, ippsFindRev_8u,  (const Ipp8u* pSrc, int len,
                                     const Ipp8u* pFind, int lenFind,
                                     int *pIndex))
IPPAPI (IppStatus, ippsFindRev_16u, (const Ipp16u* pSrc, int len,
                                     const Ipp16u* pFind, int lenFind,
                                     int *pIndex))
IPPAPI (IppStatus, ippsFindRevC_8u,  (const Ipp8u* pSrc, int len,
                                      Ipp8u valFind, int *pIndex))
IPPAPI (IppStatus, ippsFindRevC_16u, (const Ipp16u* pSrc, int len,
                                      Ipp16u valFind, int *pIndex))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFind_Z_8u        ippsFind_Z_16u
//              ippsFindC_Z_8u       ippsFindC_Z_16u
//
//  Purpose:    Finds the match for zero-ended string of elements or single element
//              within source zero-ended string in direct or reverse direction
//
//  Arguments:
//     pSrcZ   - pointer to the source zero-ended string
//     pFindZ  - pointer to the searching zero-ended string
//     valFind - searching element
//     pIndex  - pointer to the result index:
//               *pIndex = index of first occurrence;
//               *pIndex = -1 if no match;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrcZ, pFindZ or pIndex are NULL
*/
IPPAPI (IppStatus, ippsFind_Z_8u,   (const Ipp8u* pSrcZ,
                                     const Ipp8u* pFindZ, int *pIndex))
IPPAPI (IppStatus, ippsFind_Z_16u,  (const Ipp16u* pSrcZ,
                                     const Ipp16u* pFindZ, int *pIndex))
IPPAPI (IppStatus, ippsFindC_Z_8u,  (const Ipp8u* pSrcZ,
                                     Ipp8u valFind, int *pIndex))
IPPAPI (IppStatus, ippsFindC_Z_16u, (const Ipp16u* pSrcZ,
                                     Ipp16u valFind, int *pIndex))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsCompare_8u        ippsCompare_16u
//
//  Purpose:    Compares two strings element-by-element
//
//  Arguments:
//     pSrc1   - pointer to the first string
//     pSrc2   - pointer to the second string
//     len     - string length to compare
//     pResult - pointer to the result:
//               *pResult =  0 if src1 == src2;
//               *pResult = >0 if src1 >  src2;
//               *pResult = <0 if src1 <  src2;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc1, pSrc2 or pResult are NULL
//   ippStsLengthErr   len is negative
*/
IPPAPI (IppStatus, ippsCompare_8u,  (const Ipp8u* pSrc1, const Ipp8u* pSrc2,
                                     int len, int *pResult))
IPPAPI (IppStatus, ippsCompare_16u, (const Ipp16u* pSrc1, const Ipp16u* pSrc2,
                                     int len, int *pResult))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsEqual_8u        ippsEqual_16u
//
//  Purpose:    Compares two strings element-by-element
//
//  Arguments:
//     pSrc1   - pointer to the first string
//     pSrc2   - pointer to the second string
//     len     - string length to compare
//     pResult - pointer to the result:
//               *pResult =  1 if src1 == src2;
//               *pResult =  0 if src1 != src2;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc1, pSrc2 or pResult are NULL
//   ippStsLengthErr   len is negative
*/
IPPAPI (IppStatus, ippsEqual_8u,  (const Ipp8u* pSrc1, const Ipp8u* pSrc2,
                                   int len, int *pResult))
IPPAPI (IppStatus, ippsEqual_16u, (const Ipp16u* pSrc1, const Ipp16u* pSrc2,
                                   int len, int *pResult))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsTrimC_8u_I        ippsTrimC_16u_I
//
//  Purpose:    Deletes an odd symbol at the end and the beginning of a string
//              in-place
//
//  Arguments:
//     pSrcDst - pointer to the string
//     pLen    - pointer to the string length:
//               *pLen = source length on input;
//               *pLen = destination length on output;
//     odd     - odd symbol
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrcDst or pLen are NULL
//   ippStsLengthErr   *pLen is negative
*/
IPPAPI (IppStatus, ippsTrimC_8u_I,  (Ipp8u* pSrcDst, int* pLen, Ipp8u odd ))
IPPAPI (IppStatus, ippsTrimC_16u_I, (Ipp16u* pSrcDst, int* pLen, Ipp16u odd ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsTrimC_8u        ippsTrimC_16u
//
//  Purpose:    Deletes an odd symbol at the end and the beginning of a string
//
//  Arguments:
//     pSrc    - pointer to the source string
//     srcLen  - source string length
//     odd     - odd symbol
//     pDst    - pointer to the destination string
//     pDstLen - pointer to the destination string length:
//               *pDstLen doesn't use as input value;
//               *pDstLen = destination length on output;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrcDst, pDst or pDstLen are NULL
//   ippStsLengthErr   srcLen is negative
*/
IPPAPI (IppStatus, ippsTrimC_8u,  (const Ipp8u* pSrc, int srcLen, Ipp8u odd,
                                   Ipp8u* pDst, int* pDstLen ))
IPPAPI (IppStatus, ippsTrimC_16u, (const Ipp16u* pSrc, int srcLen, Ipp16u odd,
                                   Ipp16u* pDst, int* pDstLen ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:          ippsUppercase_16u_I
//                 ippsLowercase_16u_I
//                 ippsUppercase_16u
//                 ippsLowercase_16u
//
//  Purpose:    Forms an uppercase or lowercase version of the Unicode string
//
//  Arguments:
//     pSrc    - pointer to the source string
//     pDst    - pointer to the destination string
//     pSrcDst - pointer to the string for in-place operation
//     len     - string length
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, pDst or pSrcDst are NULL;
//   ippStsLengthErr   len is negative;
*/

IPPAPI (IppStatus, ippsUppercase_16u_I,( Ipp16u* pSrcDst, int len ))
IPPAPI (IppStatus, ippsLowercase_16u_I,( Ipp16u* pSrcDst, int len ))
IPPAPI (IppStatus, ippsUppercase_16u, (const Ipp16u* pSrc, Ipp16u* pDst, int len))
IPPAPI (IppStatus, ippsLowercase_16u, (const Ipp16u* pSrc, Ipp16u* pDst, int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsUppercaseLatin_8u_I   ippsUppercaseLatin_16u_I
//              ippsLowercaseLatin_8u_I   ippsLowercaseLatin_16u_I
//              ippsLowercaseLatin_8u     ippsUppercaseLatin_16u
//              ippsUppercaseLatin_8u     ippsLowercaseLatin_16u
//
//  Purpose:    Forms an uppercase or lowercase version of the ASCII string
//
//  Arguments:
//     pSrc    - pointer to the source string
//     pDst    - pointer to the destination string
//     pSrcDst - pointer to the string for in-place operation
//     len     - string length
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, pDst or pSrcDst are NULL;
//   ippStsLengthErr   len is negative;
*/
IPPAPI (IppStatus, ippsUppercaseLatin_8u_I, ( Ipp8u* pSrcDst, int len ))
IPPAPI (IppStatus, ippsLowercaseLatin_8u_I, ( Ipp8u* pSrcDst, int len ))
IPPAPI (IppStatus, ippsUppercaseLatin_16u_I,( Ipp16u* pSrcDst, int len ))
IPPAPI (IppStatus, ippsLowercaseLatin_16u_I,( Ipp16u* pSrcDst, int len ))
IPPAPI (IppStatus, ippsLowercaseLatin_8u,  (const Ipp8u* pSrc, Ipp8u* pDst, int len))
IPPAPI (IppStatus, ippsUppercaseLatin_8u,  (const Ipp8u* pSrc, Ipp8u* pDst, int len))
IPPAPI (IppStatus, ippsUppercaseLatin_16u, (const Ipp16u* pSrc, Ipp16u* pDst, int len))
IPPAPI (IppStatus, ippsLowercaseLatin_16u, (const Ipp16u* pSrc, Ipp16u* pDst, int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsHash_8u32u        ippsHash_16u32u
//
//  Purpose:    Calculates hashed value so that different strings yield different
//              values:
//              for (i=0; i<len; i++) hash = (hash << 1) ^ src[i];
//
//  Arguments:
//     pSrc     - pointer to the source string
//     len      - source string length
//     pHashVal - pointer to the result value
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc or pHashVal are NULL
//   ippStsLengthErr   len is negative
*/

IPPAPI (IppStatus, ippsHash_8u32u,  (const Ipp8u* pSrc, int len, Ipp32u* pHashVal ))
IPPAPI (IppStatus, ippsHash_16u32u, (const Ipp16u* pSrc, int len, Ipp32u* pHashVal ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsHashSJ2_8u32u         ippsHashSJ2_16u32u
//
//  Purpose:    Calculates hashed value so that different strings yield different
//              values:
//              for (i=0; i<len; i++) hash =
//
//  Arguments:
//     pSrc     - pointer to the source string
//     len      - source string length
//     pHashVal - pointer to the result value
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc or pHashVal are NULL
//   ippStsLengthErr   len is negative
*/

IPPAPI (IppStatus,ippsHashSJ2_8u32u, (const Ipp8u* pSrc, int len, Ipp32u* pHashVal))
IPPAPI (IppStatus,ippsHashSJ2_16u32u, (const Ipp16u* pSrc, int len, Ipp32u* pHashVal))
/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsHashMSCS_8u32u        ippsHashMSCS_16u32u
//
//  Purpose:    Calculates hashed value so that different strings yield different
//              values:
//              for (i=0; i<len; i++) hash =
//
//  Arguments:
//     pSrc     - pointer to the source string
//     len      - source string length
//     pHashVal - pointer to the result value
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc or pHashVal are NULL
//   ippStsLengthErr   len is negative
*/

IPPAPI (IppStatus,ippsHashMSCS_8u32u, (const Ipp8u* pSrc, int len, Ipp32u* pHashVal))
IPPAPI (IppStatus,ippsHashMSCS_16u32u, (const Ipp16u* pSrc, int len, Ipp32u* pHashVal))




/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConcat_8u   ippsConcat_16u
//
//  Purpose:    Concatenates two strings together
//
//  Arguments:
//     pSrc1   - pointer to the first source string
//     len1    - first source string length
//     pSrc2   - pointer to the second source string
//     len2    - second source string length
//     pDst    - pointer to the destination string
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc1, pSrc2 or pDst are NULL
//   ippStsLengthErr   len1 or len2 are negative
*/
IPPAPI (IppStatus, ippsConcat_8u,  (const Ipp8u* pSrc1, int len1,
                                    const Ipp8u* pSrc2, int len2,
                                    Ipp8u* pDst))
IPPAPI (IppStatus, ippsConcat_16u, (const Ipp16u* pSrc1, int len1,
                                    const Ipp16u* pSrc2, int len2,
                                    Ipp16u* pDst))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConcat_8u_D2L   ippsConcat_16u_D2L
//
//  Purpose:    Concatenates several strings together
//
//  Arguments:
//     pSrc    - pointer to the array of source strings
//     srcLen  - pointer to the array of source strings' lengths
//     numSrc  - number of source strings
//     pDst    - pointer to the destination string
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, srcLen or pDst are NULL;
//                     pSrc[i] is NULL for i < numSrc
//   ippStsLengthErr   srcLen[i] is negative for i < numSrc
//   ippStsSizeErr     numSrc is not positive
*/
IPPAPI (IppStatus, ippsConcat_8u_D2L,  (const Ipp8u* const pSrc[], const int srcLen[], int numSrc,
                                        Ipp8u* pDst ))
IPPAPI (IppStatus, ippsConcat_16u_D2L, (const Ipp16u* const pSrc[], const int srcLen[], int numSrc,
                                        Ipp16u* pDst ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConcatC_8u_D2L   ippsConcatC_16u_D2L
//
//  Purpose:    Concatenates several strings together and separates them
//              by the symbol delimiter
//
//  Arguments:
//     pSrc    - pointer to the array of source strings
//     srcLen  - pointer to the array of source strings' lengths
//     numSrc  - number of source strings
//     delim   - delimiter
//     pDst    - pointer to the destination string
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, srcLen or pDst are NULL;
//                     pSrc[i] is NULL for i < numSrc
//   ippStsLengthErr   srcLen[i] is negative for i < numSrc
//   ippStsSizeErr     numSrc is not positive
*/
IPPAPI (IppStatus, ippsConcatC_8u_D2L,  (const Ipp8u* const pSrc[], const int srcLen[], int numSrc,
                                         Ipp8u delim, Ipp8u* pDst ))
IPPAPI (IppStatus, ippsConcatC_16u_D2L, (const Ipp16u* const pSrc[], const int srcLen[], int numSrc,
                                         Ipp16u delim, Ipp16u* pDst ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsSplitC_8u_D2L   ippsSplitC_16u_D2L
//
//  Purpose:    Splits source string to several destination strings
//              using the symbol delimiter; all delimiters are significant,
//              in the case of double delimiter empty string is inserted.
//
//  Arguments:
//     pSrc    - pointer to the source string
//     srcLen  - source string length
//     delim   - delimiter
//     pDst    - pointer to the array of destination strings
//     dstLen  - pointer to the array of destination strings' lengths
//     pNumDst - pointer to the number of destination strings:
//               *pNumDst = initial number of destination strings on input;
//               *pNumDst = number of splitted strings on output;
//
//  Return:
//     ippStsNoErr       Ok
//  ERRORS:
//     ippStsNullPtrErr  pSrc, pDst, dstLen or pNumDst are NULL;
//                       pDst[i] is NULL for i < number of splitted strings
//     ippStsLengthErr   srcLen is negative;
//                       dstLen[i] is negative for i < number of splitted strings
//     ippStsSizeErr     *pNumDst is not positive
//  WARNINGS:
//     ippStsOvermuchStrings  the initial number of destination strings is less
//                            than the number of splitted strings;
//                            number of destination strings is truncated to
//                            initial number in this case
//     ippStsOverlongString   the length of one of destination strings is less than
//                            length of corresponding splitted string;
//                            splitted string is truncated to destination length
//                            in this case
*/
IPPAPI (IppStatus, ippsSplitC_8u_D2L,  (const Ipp8u* pSrc, int srcLen, Ipp8u delim,
                                        Ipp8u* pDst[], int dstLen[], int* pNumDst))
IPPAPI (IppStatus, ippsSplitC_16u_D2L, (const Ipp16u* pSrc, int srcLen, Ipp16u delim,
                                        Ipp16u* pDst[], int dstLen[], int* pNumDst))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFindCAny_8u
//              ippsFindCAny_16u
//              ippsFindRevCAny_8u
//              ippsFindRevCAny_16u
//
//  Purpose:    Reports the index of the first/last occurrence in
//              the vector of any value in a specified array.
//
//  Arguments:
//     pSrc    - The pointer of vector to find.
//     len     - The length of the vector.
//     pAnyOf  - A pointer of array containing one or more values to seek.
//     lenFind - The length of array.
//     pIndex  - The positive integer index of the first occurrence in
//               the vector where any value in pAnyOf was found;
//               otherwise, -1 if no value in pAnyOf was found.
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  Any of pointers is NULL.
//   ippStsLengthErr   len or lenAnyOf are negative.
*/
IPPAPI (IppStatus, ippsFindCAny_8u, ( const Ipp8u* pSrc, int len,
        const Ipp8u* pAnyOf, int lenAnyOf, int* pIndex ))
IPPAPI (IppStatus, ippsFindCAny_16u, ( const Ipp16u* pSrc, int len,
        const Ipp16u* pAnyOf, int lenAnyOf, int* pIndex ))
IPPAPI (IppStatus, ippsFindRevCAny_8u, ( const Ipp8u* pSrc, int len,
        const Ipp8u* pAnyOf, int lenAnyOf, int* pIndex ))
IPPAPI (IppStatus, ippsFindRevCAny_16u, ( const Ipp16u* pSrc, int len,
        const Ipp16u* pAnyOf, int lenAnyOf, int* pIndex ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsReplaceC_8u
//              ippsReplaceC_16u
//
//  Purpose:    Replaces all occurrences of a specified value in
//              the vector with another specified value.
//
//  Arguments:
//     pSrc    - The pointer of vector to replace.
//     pDst    - The ponter of replaced vector.
//     len     - The length of the vector.
//     oldVal  - A value to be replaced.
//     newVal  - A value to replace all occurrences of oldVal.
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  Any of pointers is NULL.
//   ippStsLengthErr   len is negative.
*/
IPPAPI (IppStatus, ippsReplaceC_8u, ( const Ipp8u* pSrc, Ipp8u* pDst, int len,
        Ipp8u oldVal, Ipp8u newVal ))
IPPAPI (IppStatus, ippsReplaceC_16u, ( const Ipp16u* pSrc, Ipp16u* pDst, int len,
        Ipp16u oldVal, Ipp16u newVal ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsTrimCAny_8u
//              ippsTrimCAny_16u
//              ippsTrimEndCAny_8u
//              ippsTrimEndCAny_16u
//              ippsTrimStartCAny_8u
//              ippsTrimStartCAny_16u
//
//  Purpose:    Removes all occurrences of a set of specified values
//              from:
//                TrimCAny  - the beginning and end of the vector.
//                TrimEndCAny   - the end of the vector.
//                TrimStartCAny - the beginning of the vector.
//
//  Arguments:
//     pSrc    - The pointer of src vector to remove.
//     srcLen  - The length of the src vector.
//     pTrim   - An array of values to be removed.
//     trimLen - The length of the array values.
//     pDst    - The pointer of dst vector to result save.
//     pDstLen - The result length of the dst vector.
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  Any of pointers is NULL.
//   ippStsLengthErr   srcLen or trimLen are negative.
//
//  Note:
//   The length of the pDst should be sufficient;
//   if values not found, *pDstLen = srcLen.
*/
IPPAPI (IppStatus, ippsTrimCAny_8u, ( const Ipp8u* pSrc, int srcLen,
        const Ipp8u* pTrim, int trimLen, Ipp8u* pDst, int* pDstLen ))
IPPAPI (IppStatus, ippsTrimCAny_16u, ( const Ipp16u* pSrc, int srcLen,
        const Ipp16u* pTrim, int trimLen, Ipp16u* pDst, int* pDstLen ))
IPPAPI (IppStatus, ippsTrimEndCAny_8u, ( const Ipp8u* pSrc, int srcLen,
        const Ipp8u* pTrim, int trimLen, Ipp8u* pDst, int* pDstLen ))
IPPAPI (IppStatus, ippsTrimEndCAny_16u, ( const Ipp16u* pSrc, int srcLen,
        const Ipp16u* pTrim, int trimLen, Ipp16u* pDst, int* pDstLen ))
IPPAPI (IppStatus, ippsTrimStartCAny_8u, ( const Ipp8u* pSrc, int srcLen,
        const Ipp8u* pTrim, int trimLen, Ipp8u* pDst, int* pDstLen ))
IPPAPI (IppStatus, ippsTrimStartCAny_16u, ( const Ipp16u* pSrc, int srcLen,
        const Ipp16u* pTrim, int trimLen, Ipp16u* pDst, int* pDstLen ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsCompareIgnoreCase_16u
//
//  Purpose:    Compares two Unicode strings element-by-element
//
//  Arguments:
//     pSrc1   - pointer to the first string
//     pSrc2   - pointer to the second string
//     len     - string length to compare
//     pResult - pointer to the result:
//               *pResult = 0 if src1 == src2;
//               *pResult > 0 if src1 >  src2;
//               *pResult < 0 if src1 <  src2;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc1, pSrc2 or pResult is NULL
//   ippStsLengthErr   len is negative
*/
IPPAPI (IppStatus, ippsCompareIgnoreCase_16u, (const Ipp16u* pSrc1, const Ipp16u* pSrc2, int len,
                                               int *pResult))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsCompareIgnoreCaseLatin_8u
//              ippsCompareIgnoreCaseLatin_16u
//
//  Purpose:    Compares two ASCII strings element-by-element
//
//  Arguments:
//     pSrc1   - pointer to the first string
//     pSrc2   - pointer to the second string
//     len     - string length to compare
//     pResult - pointer to the result:
//               *pResult = 0 if src1 == src2;
//               *pResult > 0 if src1 >  src2;
//               *pResult < 0 if src1 <  src2;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc1, pSrc2 or pResult is NULL
//   ippStsLengthErr   len is negative
*/
IPPAPI (IppStatus, ippsCompareIgnoreCaseLatin_8u,  (const Ipp8u* pSrc1, const Ipp8u* pSrc2, int len,
                                                    int *pResult))
IPPAPI (IppStatus, ippsCompareIgnoreCaseLatin_16u, (const Ipp16u* pSrc1, const Ipp16u* pSrc2,
                                                    int len, int *pResult))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsInsert_8u_I       ippsInsert_16u_I
//              ippsInsert_8u         ippsInsert_16u
//
//  Purpose:    Inserts one string at a specified index position in other string
//
//  Arguments:
//     pSrc       - pointer to the source string
//     srcLen     - source string length
//     pInsert    - pointer to the string to be inserted
//     insertLen  - length of the string to be inserted
//     pDst       - pointer to the destination string
//     pSrcDst    - pointer to the string for in-place operation
//     pSrcDstLen - pointer to the string length:
//                 *pSrcDstLen = source length on input;
//                 *pSrcDstLen = destination length on output;
//     startIndex - index of start position
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, pInsert, pDst, pSrcDst or pSrcDstLen is NULL
//   ippStsLengthErr   srcLen, insertLen, *pSrcDstLen or startIndex is negative Or
//                     startIndex is greater than srcLen or *pSrcDstLen
*/
IPPAPI (IppStatus, ippsInsert_8u_I,  (const Ipp8u* pInsert, int insertLen, Ipp8u* pSrcDst,
                                      int* pSrcDstLen, int startIndex))
IPPAPI (IppStatus, ippsInsert_16u_I, (const Ipp16u* pInsert, int insertLen, Ipp16u* pSrcDst,
                                      int* pSrcDstLen, int startIndex))
IPPAPI (IppStatus, ippsInsert_8u,    (const Ipp8u* pSrc, int srcLen, const Ipp8u* pInsert,
                                      int insertLen, Ipp8u* pDst, int startIndex))
IPPAPI (IppStatus, ippsInsert_16u,   (const Ipp16u* pSrc, int srcLen, const Ipp16u* pInsert,
                                      int insertLen, Ipp16u* pDst, int startIndex))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsRemove_8u_I       ippsRemove_16u_I
//              ippsRemove_8u         ippsRemove_16u
//
//  Purpose:    Deletes a specified number of characters from the string
//              beginning at a specified position.
//
//  Arguments:
//     pSrc       - pointer to the source string
//     srcLen     - source string length
//     pDst       - pointer to the destination string
//     pSrcDst    - pointer to the string for in-place operation
//     pSrcDstLen - pointer to the string length:
//                 *pSrcDstLen = source length on input;
//                 *pSrcDstLen = destination length on output;
//     startIndex - index of start position
//     len        - number of characters to be deleted
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, pDst, pSrcDst or pSrcDstLen are NULL
//   ippStsLengthErr   srcLen, *pSrcDstLen, len or startIndex is negative Or
//                     (startIndex + len) is greater than srcLen or *pSrcDstLen
*/
IPPAPI (IppStatus, ippsRemove_8u_I,  (Ipp8u* pSrcDst, int* pSrcDstLen, int startIndex, int len))
IPPAPI (IppStatus, ippsRemove_16u_I, (Ipp16u* pSrcDst, int* pSrcDstLen, int startIndex, int len))
IPPAPI (IppStatus, ippsRemove_8u,    (const Ipp8u* pSrc, int srcLen, Ipp8u* pDst, int startIndex,
                                      int len))
IPPAPI (IppStatus, ippsRemove_16u,   (const Ipp16u* pSrc, int srcLen, Ipp16u* pDst, int startIndex,
                                      int len))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpInitAlloc
// Purpose:             Allocates necessary memory, compiles a pattern into the
//                      internal form consideration corresponding options and
//                      writes it into the pRegExpState
//
// Parameters:
//    pPattern          Pointer to the pattern of regular expression
//    pOptions          Pointer to options for compiling and executing
//                      regular expression (possible values 'i','s','m','x','g')
//                      It should be NULL if no options are required.
//    pRegExpState      Pointer to the structure containing internal form of
//                      a regular expression.
//    pErrOffset        Pointer to offset into the pattern if compiling is break
//
// Return:
//    ippStsNoErr               No errors
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsMemAllocErr         Can't allocate memory for pRegExpState
//    ippStsRegExpOptionsErr    Options are incorrect
//    ippStsRegExpQuantifierErr Error caused by using wrong quantifier
//    ippStsRegExpGroupingErr   Error caused by using wrong grouping
//    ippStsRegExpBackRefErr    Error caused by using wrong back reference
//    ippStsRegExpChClassErr    Error caused by using wrong character class
//    ippStsRegExpMetaChErr     Error caused by using wrong metacharacter
//
*/
IPPAPI(IppStatus, ippsRegExpInitAlloc, ( const char* pPattern, const char* pOptions,
                                         IppRegExpState** ppRegExpState, int* pErrOffset ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpGetSize
// Purpose:             Computes the size of necessary memory (in bytes) for
//                      structure containing internal form of regular expression
//
// Parameters:
//    pPattern          Pointer to the pattern of regular expression
//    pRegExpStateSize  Pointer to the computed size of structure containing
//                      internal form of regular expression
//
// Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsRegExpGetSize, ( const char* pPattern, int* pRegExpStateSize ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpInit
// Purpose:             Compiles a pattern into the internal form consideration
//                      corresponding options and writes it into the pRegExpState
//
// Parameters:
//    pPattern          Pointer to the pattern of regular expression
//    pOptions          Pointer to options for compiling and executing
//                      regular expression (possible values 'i','s','m','x','g')
//                      It should be NULL if no options are required.
//    pRegExpState      Pointer to the structure containing internal form of
//                      a regular expression.
//    pErrOffset        Pointer to offset into the pattern if compiling is break
//
// Return:
//    ippStsNoErr               No errors
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsRegExpOptionsErr    Options are incorrect
//    ippStsRegExpQuantifierErr Error caused by using wrong quantifier
//    ippStsRegExpGroupingErr   Error caused by using wrong grouping
//    ippStsRegExpBackRefErr    Error caused by using wrong back reference
//    ippStsRegExpChClassErr    Error caused by using wrong character class
//    ippStsRegExpMetaChErr     Error caused by using wrong metacharacter
//
*/
IPPAPI(IppStatus, ippsRegExpInit, ( const char* pPattern, const char* pOptions,
                                    IppRegExpState*  pRegExpState, int* pErrOffset ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpSetMatchLimit
// Purpose:             Changes initial value of the matches kept in stack
//
// Parameters:
//    matchLimit        New value of the matches kept in stack
//    pRegExpState      Pointer to the structure containing internal form of
//                      a regular expression
//
// Return:
//    ippStsNullPtrErr  Pointer is NULL
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsRegExpSetMatchLimit, ( int matchLimit, IppRegExpState* pRegExpState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpSetFormat
// Purpose:             Changes initial value of the matches kept in stack
//
// Parameters:
//    fmt               New source encoding mode
//    pRegExpState      Pointer to the structure containing internal form of
//                      a regular expression
//
// Return:
//    ippStsNullPtrErr  Pointer is NULL.
//    ippStsRangeErr    When
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsRegExpSetFormat, ( IppRegExpFormat fmt, IppRegExpState* pRegExpState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpFree
// Purpose:             Frees allocated memory for the structure containing
//                      internal form of regular expression
//
// Parameters:
//    pRegExpState      Pointer to the structure containing internal form of
//                      a regular expression.
//
*/
IPPAPI(void, ippsRegExpFree, ( IppRegExpState* pRegExpState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpFind_8u
// Purpose:             Looks for the occurrences of the substrings matching
//                      the specified regular expression.
//
// Parameters:
//    pSrc              Pointer to the source string
//    srcLen            Number of elements in the source string.
//    pRegExpState      Pointer to the structure containing internal form of
//                      a regular expression
//    pFind             Array of pointers to the matching substrings
//    pNumFind          Size of the array pFind on input,
//                      number of matching substrings on output.
//
// Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Length of the source vector is less zero or or
//                              pNumFind is less than or equal to 0
//    ippStsRegExpErr           The structure pRegExpState contains wrong data
//    ippStsRegExpMatchLimitErr The match limit has been exhausted
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsRegExpFind_8u, ( const Ipp8u* pSrc, int srcLen, IppRegExpState* pRegExpState,
                                       IppRegExpFind* pFind, int* pNumFind ))


#if !defined( _OWN_BLDPCS )

struct RegExpMultiState;
typedef struct RegExpMultiState IppRegExpMultiState;

typedef struct {
   Ipp32u regexpDoneFlag;
   Ipp32u regexpID;
   Ipp32s numMultiFind;
   IppStatus status;
   IppRegExpFind* pFind;
} IppRegExpMultiFind;

#endif /* _OWN_BLDPCS */

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpMultiGetSize
// Purpose:             Computes the size of necessary memory (in bytes) for
//                      structure containing internal form of multi patterns search engine.
//
// Parameters:
//    maxPatterns       Maximum number of pattern.
//    pSize             Pointer to the computed size of structure containing
//                      internal form of search engine
//
// Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     When maxPatterns is less or equal 0.
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsRegExpMultiGetSize, ( Ipp32u maxPatterns, int *pSize))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpMultiInit
// Purpose:             Initialize internal form of multi patterns search engine.
//
// Parameters:
//    maxPatterns       Maximum number of pattern.
//    pState            Pointer to the structure containing internal form of
//                      multi patterns search engine.
//
// Return:
//    ippStsNoErr       No errors
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     When maxPatterns is less or equal 0.
//
*/
IPPAPI(IppStatus, ippsRegExpMultiInit, ( IppRegExpMultiState* pState, Ipp32u maxPatterns))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpMultiInitAlloc
// Purpose:             Allocates necessary memory, initialize internal form of multi patterns search engine
//
// Parameters:
//    maxPatterns       Maximum number of pattern.
//    pState            Double pointer to the structure containing internal form of
//                      multi patterns search engine.
//
// Return:
//    ippStsNoErr       No errors
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     When maxPatterns is less or equal 0.
*/
IPPAPI(IppStatus, ippsRegExpMultiInitAlloc, ( IppRegExpMultiState** ppState, Ipp32u maxPatterns))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpMultiFree
// Purpose:             Frees allocated memory for the structure containing
//                      internal form of multi patterns search engine
//
// Parameters:
//    pState            Pointer to the structure containing internal form of
//                      multi patterns search engine.
//
*/
IPPAPI(void, ippsRegExpMultiFree, (IppRegExpMultiState* pState))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpMulti*
// Purpose:             Controls multi patterns database. Add, remove or modify patterns.
//
// Parameters:
//    pRegExpState      Pointer to the structure containing internal form of a
//                      compiled regular expression.
//    regexpID          Pattern ID. 0 is invalid ID.
//    pState            Pointer to the structure containing internal form of
//                      multi patterns search engine.
//
// Return:
//    ippStsNoErr       No errors
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     When ID is equal 0.
//    ippStsMemAllocErr When number of patterns exceeded its maximum value.
//
*/
IPPAPI(IppStatus, ippsRegExpMultiAdd, ( const IppRegExpState* pRegExpState, Ipp32u regexpID, IppRegExpMultiState* pState))
IPPAPI(IppStatus, ippsRegExpMultiModify, ( const IppRegExpState* pRegExpState, Ipp32u regexpID, IppRegExpMultiState* pState))
IPPAPI(IppStatus, ippsRegExpMultiDelete, (Ipp32u regexpID, IppRegExpMultiState* pState))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpMultiFind_8u
// Purpose:             Looks for the occurrences of the substrings matching
//                      the specified patterns.
//
// Parameters:
//    pSrc              Pointer to the source string
//    srcLen            Number of elements in the source string.
//    pState            Pointer to the structure containing internal form of
//                      multi patterns search engine
//    pDstMultiFind     Array of pointers to the matching patterns
//
// Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Length of the source vector is less or equal zero.
//    ippStsNoErr               No errors
//
*/
IPPAPI(IppStatus, ippsRegExpMultiFind_8u, ( const Ipp8u* pSrc, int srcLen, IppRegExpMultiFind *pDstMultiFind, const IppRegExpMultiState* pState))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsConvertUTF
// Purpose:             Convert UTF16BE or UTF16LE format to UTF8 and vice versa.
//
// Parameters:
//    pSrc     Pointer to the input vector.
//    pSrcLen  Length of the pSrc vector on input and its used length on output.
//    BEFlag   Flag to indicate UTF16BE format. 0 means UTF16LE format.
//    pDst     Pointer to the output vector.
//    pDstLen  Length of the pDst vector on input and its used length on output.
//
// Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsNoErr               No errors
*/

IPPAPI(IppStatus, ippsConvertUTF_8u16u,( const Ipp8u* pSrc, Ipp32u *pSrcLen, Ipp16u* pDst, Ipp32u *pDstLen, int BEFlag))
IPPAPI(IppStatus, ippsConvertUTF_16u8u,( const Ipp16u* pSrc, Ipp32u *pSrcLen, Ipp8u* pDst, Ipp32u *pDstLen, int BEFlag))

#if !defined( _OWN_BLDPCS )

struct RegExpMultiState;
typedef struct RegExpReplaceState IppRegExpReplaceState;
#endif /* _OWN_BLDPCS */

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpReplaceGetSize
// Purpose:             Computes the size the find and replace engine memory.
//
// Parameters:
//    pSrcReplacement   Pointer to the input null-terminated replace pattern.
//    pSize             Pointer to the computed size of the replace engine memory.
//
// Return:
//    ippStsNullPtrErr  pSize pointer is NULL
//    ippStsNoErr       No errors
//
*/
IPPAPI(IppStatus, ippsRegExpReplaceGetSize, ( const Ipp8u* pSrcReplacement, Ipp32u *pSize))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpReplaceInit
// Purpose:             Initialize internal form of find and replace engine.
//
// Parameters:
//    pSrcReplacement   Pointer to the input null-terminated replace pattern.
//    pReplaceState     Pointer to the memory allocated for the find and replace
//                      engine structure of size  after ippsRegExpReplaceGetSize function.
//
// Return:
//    ippStsNoErr       No errors
//    ippStsNullPtrErr  pReplaceState pointer is NULL
//
*/
IPPAPI(IppStatus, ippsRegExpReplaceInit, ( const Ipp8u* pSrcReplacement, IppRegExpReplaceState *pReplaceState))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpReplace_8u
// Purpose:             Performs find and replace.
//
// Parameters:
//    pSrc            Pointer to the source string.
//    pSrcLenOffset   Length of the pSrc vector on input and its used length on output.
//    pRegExpState    Pointer to the compiled pattern structure.
//    pReplaceState   Pointer to the memory allocated for the find and replace engine structure.
//    pDst            Pointer to the destination string.
//    pDstLen         Length of the pDst vector on input and its used length on output.
//    pFind           Array of pointers to the matching substrings.
//    pNumFind        Size of the array pFind on input, number of matching substrings on output.
//
// Return:
//    ippStsNoErr       Indicates no error.
//    ippStsNullPtrErr  One or several pointer(s) is NULL.
//    ippStsSizeErr     Indicates an error when value in pSrcLen or pDstLen is less or equal to zero.
//
*/
IPPAPI(IppStatus, ippsRegExpReplace_8u, ( const Ipp8u* pSrc, int *pSrcLenOffset, Ipp8u *pDst, int *pDstLen, IppRegExpFind* pFind, int* pNumFind, IppRegExpState* pRegExpState,
       IppRegExpReplaceState *pReplaceState ))

#if defined (_IPP_STDCALL_CDECL)
  #undef  _IPP_STDCALL_CDECL
  #define __stdcall __cdecl
#endif

#ifdef __cplusplus
}
#endif

#endif /* __IPPCH_H__ */
/* ////////////////////////////// End of file /////////////////////////////// */
