/****************************************************************************
*
*   Module Title :     CWmtFunctions.c
*
*   Description  :     Encoder system dependant functions
*
*    AUTHOR      :     Paul Wilkins
*
****************************************************************************/

/****************************************************************************
*  Header Files
****************************************************************************/
#include <math.h>
#include "compdll.h"
#include <assert.h>

/****************************************************************************
*  Macros
****************************************************************************/
#pragma warning(disable:4799)

#define FILTER_WEIGHT 128
#define FILTER_SHIFT    7

/****************************************************************************
*  Imports
****************************************************************************/
extern XMMGetSAD ( UINT8 *NewDataPtr,
                   INT32 PixelsPerLine,
                   UINT8 *RefDataPtr,
                   INT32 RefPixelsPerLine,
                   UINT32 ErrorSoFar,
                   UINT32 BestSoFar );

extern UINT32 GetSumAbsDiffs16(UINT8 * SrcPtr,INT32 SourceStride,UINT8  * RefPtr,INT32 ReconStride,UINT32 ErrorSoFar,UINT32 BestSoFar);
extern INT16  BilinearFilters_wmt[8][16];

/****************************************************************************
*  Module Statics
****************************************************************************/
static __declspec(align(16)) short rd[] = { 64, 64, 64, 64, 64, 64, 64, 64 };


/****************************************************************************
 * 
 *  ROUTINE       : WmtGetSumAbsDiffs16
 *
 *  INPUTS        : UINT8 *SrcPtr       : Pointer to input 16x16 block.
 *                  INT32  SourceStride : Stride of input block.
 *                  UINT8 *RefPtr       : Pointer to reference 16x16 block.
 *                  INT32  ReconStride  : Stride of reference block.
 *                  UINT32 ErrorSoFar   : Accumulated error to date.
 *                  UINT32 BestSoFar    : Best error found so far.
 *                  
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: SAD.
 *
 *  FUNCTION      : Calculate the Sum of Absolute difference on 16x16 pixels.
 *
 *  SPECIAL NOTES : The function assumes the SrcPtr is aligned on 16 bytes,
 *                  RefPtr can be aligned any byte boundary.
 *
 ****************************************************************************/
UINT32 WmtGetSumAbsDiffs16 
(
    UINT8 *SrcPtr,
    INT32  SourceStride,
    UINT8 *RefPtr,
    INT32  ReconStride,
    UINT32 ErrorSoFar,
    UINT32 BestSoFar
)
{

    UINT32 Error = 0;
    __asm
    {
        mov         esi,    SrcPtr                      ;
        mov         edi,    RefPtr                      ; 

        mov         ecx,    SourceStride                ;
        mov         edx,    ReconStride                 ;
        
        movdqu      xmm0,   [edi]                       ;   Row0 Ref
        lea         eax,    [esi + 2 * ecx ]            ;   Calculate Row3 Source address

        psadbw      xmm0,   [esi]                       ;   Row0 SAD
        lea         ebx,    [edi + 2 * edx ]            ;   Calculate Row3 Ref address

        movdqu      xmm1,   [edi + edx]                 ;   Row1 Ref
        add         eax,    ecx                         ;   Calculate Row3 Source address    

        psadbw      xmm1,   [esi + ecx]                 ;   Row1 SAD
        add         ebx,    edx                         ;   Calculate Row3 Ref address

        movdqu      xmm2,   [edi + 2 * edx ]            ;   Row2 Ref
        paddw       xmm0,   xmm1                        ;   Row0 sad + Row1 sad

        psadbw      xmm2,   [esi + 2 * ecx ]            ;   Row2 Sad
        lea         esi,    [eax + 2 * ecx ]            ;   Calculate Row6 Source address    

        lea         edi,    [ebx + 2 * edx ]            ;   Calculate Row6 Ref address
        movdqu      xmm3,   [ebx]                       ;   Row3 Ref
        
        add         esi,    ecx                         ;   Calculate Row6 Source address    
        psadbw      xmm3,   [eax]                       ;   Row3 SAD
        
        add         edi,    edx                         ;   Calculate Row6 Ref address
        movdqu      xmm4,   [ebx + edx]                 ;   Row4 Ref

        paddw       xmm2,   xmm3                        ;   Row2 Sad + Row3 Sad
        psadbw      xmm4,   [eax + ecx]                 ;   Row4 Sad

        movdqu      xmm5,   [ebx + 2 * edx]             ;   Row5 Ref
        paddd       xmm0,   xmm2                        ;   Row0 + Row1 + Row2 + Row3 SAD

        psadbw      xmm5,   [eax + 2 * ecx]             ;   Row5 SAD
        movdqu      xmm6,   [edi]                       ;   Row6 Ref

        paddw       xmm4,   xmm5                        ;   Row4 + Row5 SAD
        psadbw      xmm6,   [esi]                       ;   Row6 SAD

        movdqu      xmm7,   [edi + edx ]                ;   Row7 Ref
        paddd       xmm0,   xmm4                        ;   Row0 1 2 3 4 5 

        psadbw      xmm7,   [esi + ecx]                 ;   Row7 Sad
        
        lea         esi,    [esi + 2* ecx]              ;   calculate Row8 source address
        paddw       xmm7,   xmm6                        ;   Row7 + Row6 Sad
        
        lea         edi,    [edi + 2* edx]              ;   calculate Row8 source address        
        paddd       xmm7,   xmm0                        ; 

        // next eight row
        movdqu      xmm0,   [edi]                       ;   Row0 Ref
        lea         eax,    [esi + 2 * ecx ]            ;   Calculate Row3 Source address

        psadbw      xmm0,   [esi]                       ;   Row0 SAD
        lea         ebx,    [edi + 2 * edx ]            ;   Calculate Row3 Ref address

        movdqu      xmm1,   [edi + edx]                 ;   Row1 Ref
        add         eax,    ecx                         ;   Calculate Row3 Source address    

        psadbw      xmm1,   [esi + ecx]                 ;   Row1 SAD
        add         ebx,    edx                         ;   Calculate Row3 Ref address

        movdqu      xmm2,   [edi + 2 * edx ]            ;   Row2 Ref
        paddw       xmm0,   xmm1                        ;   Row0 sad + Row1 sad

        psadbw      xmm2,   [esi + 2 * ecx ]            ;   Row2 Sad
        lea         esi,    [eax + 2 * ecx ]            ;   Calculate Row6 Source address    

        lea         edi,    [ebx + 2 * edx ]            ;   Calculate Row6 Ref address
        movdqu      xmm3,   [ebx]                       ;   Row3 Ref
        
        add         esi,    ecx                         ;   Calculate Row6 Source address    
        psadbw      xmm3,   [eax]                       ;   Row3 SAD
        
        add         edi,    edx                         ;   Calculate Row6 Ref address
        movdqu      xmm4,   [ebx + edx]                 ;   Row4 Ref

        paddw       xmm2,   xmm3                        ;   Row2 Sad + Row3 Sad
        psadbw      xmm4,   [eax + ecx]                 ;   Row4 Sad

        movdqu      xmm5,   [ebx + 2 * edx]             ;   Row5 Ref
        paddd       xmm0,   xmm2                        ;   Row0 + Row1 + Row2 + Row3 SAD

        psadbw      xmm5,   [eax + 2 * ecx]             ;   Row5 SAD
        movdqu      xmm6,   [edi]                       ;   Row6 Ref

        paddw       xmm4,   xmm5                        ;   Row4 + Row5 SAD
        psadbw      xmm6,   [esi]                       ;   Row6 SAD

        paddd       xmm0,   xmm4                        ;   Row0 1 2 3 4 5 

        movdqu      xmm3,   [edi + edx ]                ;   Row7 Ref
        psadbw      xmm3,   [esi + ecx ]                ;   Row7 Sad

        paddw       xmm3,   xmm6                        ;
        paddd       xmm0,   xmm3                        ;   Sum of 16 row sad
        
        paddd       xmm7,   xmm0;                       ; 

        movdq2q     mm0,    xmm7                        ;   lower q
        psrldq      xmm7,   8                           ;  

        movdq2q     mm1,    xmm7                        ;    High Q
        paddd       mm0,    mm1                         ;

        movd        Error, mm0               

    }
    return Error;
}

/****************************************************************************
 * 
 *  ROUTINE       : WmtGetHalfPixelSumAbsDiffs16
 *
 *  INPUTS        : UINT8 *SrcPtr       : Pointer to input 16x16 block.
 *                  INT32  SourceStride : Stride of input block.
 *                  UINT8 *RefPtr       : Pointer to first reference 16x16 block.
 *                  UINT8 *RefPtr2      : Pointer to second reference 16x16 block.
 *                  INT32  ReconStride  : Stride of reference blocks.
 *                  UINT32 ErrorSoFar   : Accumulated error to date.
 *                  UINT32 BestSoFar    : Best error found so far.
 *                  
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: SAD.
 *
 *  FUNCTION      : Calculates the Sum of Absolute differences between a 16x16 
 *                  pixel MB and the average of two 16x16 pixel references.
 *
 *  SPECIAL NOTES : The function assumes the SrcPtr is aligned on 16 bytes,
 *                  RefPtr & RefPtr2 can be aligned any byte boundary.
 *
 ****************************************************************************/
UINT32 WmtGetHalfPixelSumAbsDiffs16
(
    UINT8 *SrcPtr,
    INT32  SourceStride,
    UINT8 *RefPtr,
    UINT8 *RefPtr2,
    INT32  ReconStride,
    UINT32 ErrorSoFar,
    UINT32 BestSoFar
)
{
    UINT32 Error = 0;
    
    if ( RefPtr == RefPtr2 )
    {
        Error = GetSumAbsDiffs16 ( SrcPtr, SourceStride, RefPtr, ReconStride, 0, 0 );
    }
    else
    {
        __asm
        {
            mov         esi,        SrcPtr;
            mov         edi,        RefPtr;

            mov         eax,        RefPtr2;
            mov         ecx,        SourceStride;

            mov         edx,        ReconStride;
            pxor        xmm7,       xmm7;

            mov         ebx,         16;
            pxor        xmm6,       xmm6;

LoopWmtHalfSad:

            movdqu      xmm0,       [edi]                   ;   Read 16 bytes from Ref
            movdqu      xmm1,       [eax]                   ;   Read 16 bytes from Ref2
                        
            movdqa      xmm2,       xmm0                    ;   copy 
            punpcklbw   xmm0,       xmm7                    ;   Low 8 bytes from Ref

            movdqa      xmm3,       xmm1                    ;   copy
            punpcklbw   xmm1,       xmm7                    ;   Low 8 bytes from Ref2

            paddw       xmm0,       xmm1                    ;   Add low 8 bytes
            punpckhbw   xmm2,       xmm7                    ;   High 8 bytes from Ref

            psraw       xmm0,       1                       ;   average of Low 8 bytes Ref and Ref2
            punpckhbw   xmm3,       xmm7                    ;   High 8 bytes from Ref2

            add         eax,        edx                     ;   Next line of Ref1
            paddw       xmm2,       xmm3                    ;   Add high 8 bytes

            add         edi,        edx                     ;   Next line of Ref2
            psraw       xmm2,       1                       ;   Average of high 8 bytes

            packuswb    xmm0,       xmm2                    ;   pack the average back into bytes
            psadbw      xmm0,       [esi]                   ;   sad 

            add         esi,        ecx                     ;   next line of source
            dec         ebx                                 ;
            
            paddd       xmm6,       xmm0                    ;   accumulate the sad
            jnz         LoopWmtHalfSad

            movdq2q     mm0,        xmm6                    ;   
            psrldq      xmm6,       8                       ;

            movdq2q     mm1,        xmm6                    ;
            paddd       mm0,        mm1                     ;

            movd        Error,      mm0                     ;

        }                
    }
    return Error;
}

/****************************************************************************
 *
 *  ROUTINE       : WmtGetHalfPixelSAD
 *
 *  INPUTS        : UINT8 *SrcData          : Pointer to input 16x16 block.
 *                  INT32  PixelsPerLine    : Stride of input block.
 *                  UINT8 *RefDataPtr1      : Pointer to first reference 16x16 block.
 *                  UINT8 *RefDataPtr2      : Pointer to second reference 16x16 block.
 *                  INT32  RefPixelsPerLine : Stride of reference blocks.
 *                  INT32  ErrorSoFar       : Accumulated error to date.
 *                  INT32  BestSoFar        : Best error found so far.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : INT32: SAD at 1/2 pixel accuracy.
 *
 *  FUNCTION      : Calculates the sum of the absolute differences against
 *                  half pixel interpolated references.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
INT32 WmtGetHalfPixelSAD
( 
    UINT8 *SrcData, 
    INT32 PixelsPerLine,
    UINT8 *RefDataPtr1,
    UINT8 *RefDataPtr2,
    INT32 RefPixelsPerLine,
    INT32 ErrorSoFar, 
    INT32 BestSoFar 
)
{
    INT32   DiffVal = ErrorSoFar;
    INT16   DiffAcc[4] = { 0, 0, 0, 0 };     // MMX accumulator.
    INT32   RefOffset = (int)(RefDataPtr1 - RefDataPtr2);

    if ( RefOffset == 0 )
    {
        // Simple case as for non 0.5 pixel
        DiffVal += XMMGetSAD ( SrcData, PixelsPerLine, RefDataPtr1, RefPixelsPerLine, ErrorSoFar, BestSoFar );
    }
    else
    {
        // WMT Code for HalfPixelSAD
        __asm
        {
            mov         eax,        dword ptr [SrcData]         // Get Src Pointer
            pxor        xmm6,       xmm6                        // clear mm6

            mov         ebx,        dword ptr [RefDataPtr1]     // Get Reference pointers
            pxor        xmm7,       xmm7

            mov         edx,        dword ptr [PixelsPerLine]   // Width
            mov         ecx,        dword ptr [RefDataPtr2]

            mov         esi,        edx                         // width
            mov         edx,        dword ptr [RefPixelsPerLine]   // Src Pitch

            // Row 1 and 2
            movq        xmm1,       QWORD ptr [ebx]             // Eight bytes from ref 1
            movq        xmm2,       QWORD ptr [ecx]             // Eight Bytes from ref 2

            punpcklbw   xmm1,       xmm6                        // unpack ref1 to shorts
            movq        xmm3,       QWORD ptr [ebx+edx]         // Eight bytes from ref 1

            punpcklbw   xmm2,       xmm6                        // unpack ref2 to shorts
            movq        xmm4,       QWORD ptr [ecx+edx]         // Eight bytes from ref 2

            punpcklbw   xmm3,       xmm6                        // unpack to shorts
            paddw       xmm1,       xmm2                        // Add short values together.

            punpcklbw   xmm4,       xmm6                        // unpack to shorts
            psrlw       xmm1,       1                           // Devided by two (shift right 1)

            paddw       xmm3,       xmm4                        // add short values togethter
            movq        xmm0,       QWORD PTR [eax]             // Copy eight of src data to xmm0

            psrlw       xmm3,       1                           // divided by 2
            punpcklbw   xmm0,       xmm6                        // unpack to shorts

            movq        xmm5,       QWORD PTR [eax+esi]         // get the source
            movdqa      xmm2,       xmm0                        // make a copy of xmm0

            punpcklbw   xmm5,       xmm6                        // unpack to shorts
            psubusw     xmm0,       xmm1                        // A-B to xmm0

            movdqa      xmm4,       xmm5                        // make a copy
            psubusw     xmm1,       xmm2                        // B-A to xmm1

            psubusw     xmm5,       xmm3                        // A-B to xmm5
            psubusw     xmm3,       xmm4                        // B-A to mm1

            por         xmm0,       xmm1                        // abs differences
            por         xmm5,       xmm3                        // abs differences

            paddw       xmm7,       xmm0                        // accumulate difference...
            paddw       xmm7,       xmm5                        // accumulate difference...

            lea         ebx,        [ebx+edx*2]                 // two line below
            lea         ecx,        [ecx+edx*2]                 // two line below

            lea         eax,        [eax+esi*2]                 // two line below

            // Row 3 and 4
            movq        xmm1,       QWORD PTR [ebx]             // Eight bytes from ref 1
            movq        xmm2,       QWORD PTR [ecx]             // Eight Bytes from ref 2

            punpcklbw   xmm1,       xmm6                        // unpack ref1 to shorts
            movq        xmm3,       QWORD PTR [ebx+edx]         // Eight bytes from ref 1

            punpcklbw   xmm2,       xmm6                        // unpack ref2 to shorts
            movq        xmm4,       QWORD PTR [ecx+edx]         // Eight bytes from ref 2

            punpcklbw   xmm3,       xmm6                        // unpack to shorts
            paddw       xmm1,       xmm2                        // Add short values together.

            punpcklbw   xmm4,       xmm6                        // unpack to shorts
            psrlw       xmm1,       1                           // Devided by two (shift right 1)

            paddw       xmm3,       xmm4                        // add short values togethter
            movq        xmm0,       QWORD PTR [eax]             // Copy eight of src data to xmm0

            psrlw       xmm3,       1                           // divided by 2
            punpcklbw   xmm0,       xmm6                        // unpack to shorts

            movq        xmm5,       QWORD PTR [eax+esi]         // get the source
            movdqa      xmm2,       xmm0                        // make a copy of xmm0

            punpcklbw   xmm5,       xmm6                        // unpack to shorts
            psubusw     xmm0,       xmm1                        // A-B to xmm0

            movdqa      xmm4,       xmm5                        // make a copy
            psubusw     xmm1,       xmm2                        // B-A to xmm1

            psubusw     xmm5,       xmm3                        // A-B to xmm5
            psubusw     xmm3,       xmm4                        // B-A to mm1

            por         xmm0,       xmm1                        // abs differences
            por         xmm5,       xmm3                        // abs differences

            paddw       xmm7,       xmm0                        // accumulate difference...
            paddw       xmm7,       xmm5                        // accumulate difference...

            lea         ebx,        [ebx+edx*2]                 // two line below
            lea         ecx,        [ecx+edx*2]                 // two line below

            lea         eax,        [eax+esi*2]                 // two line below

            // Row 5 and 6
            movq        xmm1,       QWORD PTR [ebx]             // Eight bytes from ref 1
            movq        xmm2,       QWORD PTR [ecx]             // Eight Bytes from ref 2

            punpcklbw   xmm1,       xmm6                        // unpack ref1 to shorts
            movq        xmm3,       QWORD PTR [ebx+edx]         // Eight bytes from ref 1

            punpcklbw   xmm2,       xmm6                        // unpack ref2 to shorts
            movq        xmm4,       QWORD PTR [ecx+edx]         // Eight bytes from ref 2

            punpcklbw   xmm3,       xmm6                        // unpack to shorts
            paddw       xmm1,       xmm2                        // Add short values together.

            punpcklbw   xmm4,       xmm6                        // unpack to shorts
            psrlw       xmm1,       1                           // Devided by two (shift right 1)

            paddw       xmm3,       xmm4                        // add short values togethter
            movq        xmm0,       QWORD PTR [eax]             // Copy eight of src data to xmm0

            psrlw       xmm3,       1                           // divided by 2
            punpcklbw   xmm0,       xmm6                        // unpack to shorts

            movq        xmm5,       QWORD PTR [eax+esi]         // get the source
            movdqa      xmm2,       xmm0                        // make a copy of xmm0

            punpcklbw   xmm5,       xmm6                        // unpack to shorts
            psubusw     xmm0,       xmm1                        // A-B to xmm0

            movdqa      xmm4,       xmm5                        // make a copy
            psubusw     xmm1,       xmm2                        // B-A to xmm1

            psubusw     xmm5,       xmm3                        // A-B to xmm5
            psubusw     xmm3,       xmm4                        // B-A to mm1

            por         xmm0,       xmm1                        // abs differences
            por         xmm5,       xmm3                        // abs differences

            paddw       xmm7,       xmm0                        // accumulate difference...
            paddw       xmm7,       xmm5                        // accumulate difference...

            lea         ebx,        [ebx+edx*2]                 // two line below
            lea         ecx,        [ecx+edx*2]                 // two line below


            lea         eax,        [eax+esi*2]                 // two line below

            // Row 7 and 8
            movq        xmm1,       QWORD PTR [ebx]             // Eight bytes from ref 1
            movq        xmm2,       QWORD PTR [ecx]             // Eight Bytes from ref 2

            punpcklbw   xmm1,       xmm6                        // unpack ref1 to shorts
            movq        xmm3,       QWORD PTR [ebx+edx]         // Eight bytes from ref 1

            punpcklbw   xmm2,       xmm6                        // unpack ref2 to shorts
            movq        xmm4,       QWORD PTR [ecx+edx]         // Eight bytes from ref 2

            punpcklbw   xmm3,       xmm6                        // unpack to shorts
            paddw       xmm1,       xmm2                        // Add short values together.

            punpcklbw   xmm4,       xmm6                        // unpack to shorts
            psrlw       xmm1,       1                           // Devided by two (shift right 1)

            paddw       xmm3,       xmm4                        // add short values togethter
            movq        xmm0,       QWORD PTR [eax]                     // Copy eight of src data to xmm0

            psrlw       xmm3,       1                           // divided by 2
            punpcklbw   xmm0,       xmm6                        // unpack to shorts

            movq        xmm5,       QWORD PTR [eax+esi]         // get the source
            movdqa      xmm2,       xmm0                        // make a copy of xmm0

            punpcklbw   xmm5,       xmm6                        // unpack to shorts
            psubusw     xmm0,       xmm1                        // A-B to xmm0

            movdqa      xmm4,       xmm5                        // make a copy
            psubusw     xmm1,       xmm2                        // B-A to xmm1

            psubusw     xmm5,       xmm3                        // A-B to xmm5
            psubusw     xmm3,       xmm4                        // B-A to mm1

            por         xmm0,       xmm1                        // abs differences
            por         xmm5,       xmm3                        // abs differences

            paddw       xmm7,       xmm0                        // accumulate difference...
            paddw       xmm7,       xmm5                        // accumulate difference...

            // add the value to gether
            movdqa      xmm0,       xmm7                        // low four words
            psrldq      xmm7,       8                           // shift 64 bits

            paddw       xmm0,       xmm7                        // add
            movq        QWORD PTR [DiffAcc], xmm0   ; copy back accumulated results into normal memory

        }

        //  Accumulate the 4 word values in DiffAcc
        DiffVal += DiffAcc[0] + DiffAcc[1] + DiffAcc[2] + DiffAcc[3];
    }

    return DiffVal;
}

/****************************************************************************
 *
 *  ROUTINE       : WmtGetIntraError
 *
 *  INPUTS        : UINT8 *DataPtr       : Pointer to input block.
 *                  INT32  PixelsPerLine : Stride of input block.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Intra frame variance.
 *
 *  FUNCTION      : Calculates the variance of the block.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
UINT32 WmtGetIntraError ( UINT8 *DataPtr, INT32 PixelsPerLine )
{
    UINT32  XSum;
    UINT32  XXSum;
    UINT8   *DiffPtr;

    // Loop expanded out for speed.
    DiffPtr = DataPtr;

    __asm
    {
            pxor        xmm5,   xmm5                    ; Blank mmx6
            pxor        xmm6,   xmm6                    ; Blank mmx7
            pxor        xmm7,   xmm7                    ; Blank mmx7

            mov         eax,    dword ptr [DiffPtr]     ; Load base addresses
            mov         ecx,    dword ptr [PixelsPerLine]

            // Row 1
            movq        xmm0,   QWORD ptr [eax]         ; Copy eight bytes to xmm0;
            punpcklbw   xmm0,   xmm6
            paddw       xmm5,   xmm0
            pmaddwd     xmm0,   xmm0
            paddd       xmm7,   xmm0
            
            // Row 2
            movq        xmm1,   QWORD ptr [eax+ecx]     ; Copy eight bytes to xmm0;
            punpcklbw   xmm1,   xmm6
            paddw       xmm5,   xmm1
            pmaddwd     xmm1,   xmm1
            paddd       xmm7,   xmm1

            // Row 3    
            movq        xmm2,   QWORD ptr [eax+ecx * 2]     ; Copy eight bytes to xmm0;
            add         eax,    ecx
            punpcklbw   xmm2,   xmm6
            paddw       xmm5,   xmm2
            pmaddwd     xmm2,   xmm2
            paddd       xmm7,   xmm2
            lea         eax,    [eax+ecx*2]
            
            // Row 4
            movq        xmm0,   QWORD ptr [eax]         ; Copy eight bytes to xmm0;
            punpcklbw   xmm0,   xmm6
            paddw       xmm5,   xmm0
            pmaddwd     xmm0,   xmm0
            paddd       xmm7,   xmm0

            // Row 5
            movq        xmm1,   QWORD ptr [eax+ecx]     ; Copy eight bytes to xmm0;
            punpcklbw   xmm1,   xmm6
            paddw       xmm5,   xmm1
            pmaddwd     xmm1,   xmm1
            paddd       xmm7,   xmm1

            // Row 6    
            movq        xmm2,   QWORD ptr [eax+ecx * 2]     ; Copy eight bytes to xmm0;
            add         eax,    ecx
            punpcklbw   xmm2,   xmm6
            paddw       xmm5,   xmm2
            pmaddwd     xmm2,   xmm2
            paddd       xmm7,   xmm2
            lea         eax,    [eax+ecx*2]

            // Row 7
            movq        xmm0,   QWORD ptr [eax]         ; Copy eight bytes to xmm0;
            punpcklbw   xmm0,   xmm6
            paddw       xmm5,   xmm0
            pmaddwd     xmm0,   xmm0
            paddd       xmm7,   xmm0

            // Row 8
            movq        xmm1,   QWORD ptr [eax+ecx]     ; Copy eight bytes to xmm0;
            punpcklbw   xmm1,   xmm6
            paddw       xmm5,   xmm1
            pmaddwd     xmm1,   xmm1
            paddd       xmm7,   xmm1
        
            movdqa      xmm4,   xmm5
            punpcklwd   xmm5,   xmm6

            punpckhwd   xmm4,   xmm6
            movdqa      xmm0,   xmm7

            paddw       xmm5,   xmm4
            punpckldq   xmm7,   xmm6

            punpckhdq   xmm0,   xmm6
            movdqa      xmm4,   xmm5

            paddd       xmm0,   xmm7
            punpckldq   xmm4,   xmm6
            punpckhdq   xmm5,   xmm6
            paddw       xmm4,   xmm5

            movdqa      xmm5,   xmm4
            movdqa      xmm7,   xmm0

            psrldq      xmm5,   8;
            psrldq      xmm7,   8;
            
            paddw       xmm4,   xmm5
            paddd       xmm0,   xmm7
            
            movd        DWORD PTR [XXSum], xmm0
            movd        DWORD ptr [XSum], xmm4
    }
    // Compute population variance as mis-match metric.
    return ( ((XXSum<<6) - XSum*XSum) );
}

/****************************************************************************
 *
 *  ROUTINE       : WmtGetInterErr
 *
 *  INPUTS        : UINT8 *NewDataPtr       : Pointer to input block.
 *                  INT32  PixelsPerLine    : Stride of input block.
 *                  UINT8 *RefDataPtr1      : Pointer to first reference block.
 *                  UINT8 *RefDataPtr2      : Pointer to second reference block.
 *                  INT32  RefPixelsPerLine : Stride of reference blocks.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: SAD at 1/2 pixel accuracy.
 *
 *  FUNCTION      : Calculates the variance of the difference between a block
 *                  and the half-pixel interpolated average of two reference blocks.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
UINT32 WmtGetInterErr
( 
    UINT8 *NewDataPtr, 
    INT32 PixelsPerLine, 
    UINT8 *RefDataPtr1, 
    UINT8 *RefDataPtr2, 
    INT32 RefPixelsPerLine 
)
{
    UINT32  XSum; 
    UINT32  XXSum;
    INT16   MmxXSum[4];  
    INT32   MmxXXSum[2]; 

    // Mode of interpolation chosen based upon on the offset of the second reference pointer
    if ( RefDataPtr1 == RefDataPtr2 )
    {
        __asm
        {
            mov         eax,        NewDataPtr                  // Load base addresses
            pxor        xmm5,       xmm5                        // Clear Xmm5

            mov         ebx,        RefDataPtr1                 // Ref1
            pxor        xmm6,       xmm6                        // Clear Xmm6


            mov         ecx,        PixelsPerLine               // Get Width
            pxor        xmm7,       xmm7                        // Clear Xmm7

            mov         edx,        RefPixelsPerLine            // Get Pitch

            // Row 1 and Row 2
            movq        xmm0,       QWORD PTR [eax]             // Copy eight bytes to xmm0
            movq        xmm1,       QWORD PTR [ebx]             // Copy eight bytes to xmm1

            punpcklbw   xmm0,       xmm6                        // unpack to higher precision
            movq        xmm3,       QWORD Ptr [eax+ecx]         // Copy eight Bytes to xmm3

            punpcklbw   xmm1,       xmm6                        // unpack to shorts
            movq        xmm4,       QWORD ptr [ebx+edx]         // Copy eight Bytes to xmm4

            punpcklbw   xmm3,       xmm6                        // unpack to shorts
            psubsw      xmm0,       xmm1                        // A-B to xmm0

            punpcklbw   xmm4,       xmm6                        // unpack to shorts
            paddw       xmm5,       xmm0                        // accumulate differences in xmm5

            psubsw      xmm3,       xmm4                        // A-B to xmm3
            paddw       xmm5,       xmm3                        // accumulate the differences

            pmaddwd     xmm0,       xmm0                        // square and accumulate
            pmaddwd     xmm3,       xmm3                        // square and accumulate

            lea         ebx,        [ebx+edx*2]                 // mov forward two lines
            lea         eax,        [eax+ecx*2]                 // mov forward two lines

            paddd       xmm7,       xmm0                        // accumulate in xmm7
            paddd       xmm7,       xmm3                        // accumulate in xmm7

            // Row 3 and Row 4
            movq        xmm0,       QWORD PTR [eax]             // Copy eight bytes to xmm0
            movq        xmm1,       QWORD PTR [ebx]             // Copy eight bytes to xmm1

            punpcklbw   xmm0,       xmm6                        // unpack to higher precision
            movq        xmm3,       QWORD Ptr [eax+ecx]         // Copy eight Bytes to xmm3

            punpcklbw   xmm1,       xmm6                        // unpack to shorts
            movq        xmm4,       QWORD ptr [ebx+edx]         // Copy eight Bytes to xmm4

            punpcklbw   xmm3,       xmm6                        // unpack to shorts
            psubsw      xmm0,       xmm1                        // A-B to xmm0

            punpcklbw   xmm4,       xmm6                        // unpack to shorts
            paddw       xmm5,       xmm0                        // accumulate differences in xmm5

            psubsw      xmm3,       xmm4                        // A-B to xmm3
            paddw       xmm5,       xmm3                        // accumulate the differences

            pmaddwd     xmm0,       xmm0                        // square and accumulate
            pmaddwd     xmm3,       xmm3                        // square and accumulate

            lea         ebx,        [ebx+edx*2]                 // mov forward two lines
            lea         eax,        [eax+ecx*2]                 // mov forward two lines

            paddd       xmm7,       xmm0                        // accumulate in xmm7
            paddd       xmm7,       xmm3                        // accumulate in xmm7

            // Row 5 and Row6
            movq        xmm0,       QWORD PTR [eax]             // Copy eight bytes to xmm0
            movq        xmm1,       QWORD PTR [ebx]             // Copy eight bytes to xmm1

            punpcklbw   xmm0,       xmm6                        // unpack to higher precision
            movq        xmm3,       QWORD Ptr [eax+ecx]         // Copy eight Bytes to xmm3

            punpcklbw   xmm1,       xmm6                        // unpack to shorts
            movq        xmm4,       QWORD ptr [ebx+edx]         // Copy eight Bytes to xmm4

            punpcklbw   xmm3,       xmm6                        // unpack to shorts
            psubsw      xmm0,       xmm1                        // A-B to xmm0

            punpcklbw   xmm4,       xmm6                        // unpack to shorts
            paddw       xmm5,       xmm0                        // accumulate differences in xmm5

            psubsw      xmm3,       xmm4                        // A-B to xmm3
            paddw       xmm5,       xmm3                        // accumulate the differences

            pmaddwd     xmm0,       xmm0                        // square and accumulate
            pmaddwd     xmm3,       xmm3                        // square and accumulate

            lea         ebx,        [ebx+edx*2]                 // mov forward two lines
            lea         eax,        [eax+ecx*2]                 // mov forward two lines

            paddd       xmm7,       xmm0                        // accumulate in xmm7
            paddd       xmm7,       xmm3                        // accumulate in xmm7

            // Row 7 and Row 8
            movq        xmm0,       QWORD PTR [eax]             // Copy eight bytes to xmm0
            movq        xmm1,       QWORD PTR [ebx]             // Copy eight bytes to xmm1

            punpcklbw   xmm0,       xmm6                        // unpack to higher precision
            movq        xmm3,       QWORD Ptr [eax+ecx]         // Copy eight Bytes to xmm3

            punpcklbw   xmm1,       xmm6                        // unpack to shorts
            movq        xmm4,       QWORD ptr [ebx+edx]         // Copy eight Bytes to xmm4

            punpcklbw   xmm3,       xmm6                        // unpack to shorts
            psubsw      xmm0,       xmm1                        // A-B to xmm0

            punpcklbw   xmm4,       xmm6                        // unpack to shorts
            paddw       xmm5,       xmm0                        // accumulate differences in xmm5

            psubsw      xmm3,       xmm4                        // A-B to xmm3
            paddw       xmm5,       xmm3                        // accumulate the differences

            pmaddwd     xmm0,       xmm0                        // square and accumulate
            pmaddwd     xmm3,       xmm3                        // square and accumulate

            paddd       xmm7,       xmm0                        // accumulate in xmm7
            paddd       xmm7,       xmm3                        // accumulate in xmm7


            movdqa      xmm0,       xmm5
            movdqa      xmm1,       xmm7

            psrldq      xmm5,       8
            psrldq      xmm7,       8

            paddw       xmm0,       xmm5
            paddd       xmm1,       xmm7


            movq        QWORD PTR [MmxXSum], xmm0   ; copy back accumulated results into normal memory
            movq        QWORD PTR [MmxXXSum], xmm1  ; copy back accumulated results into normal memory

        }

        // Now accumulate the final results.
        XSum = MmxXSum[0] + MmxXSum[1] + MmxXSum[2] + MmxXSum[3];
        XXSum = MmxXXSum[0] + MmxXXSum[1];
    }
    // Simple half pixel reference data
    else
    {
        __asm
        {

            mov         eax,        NewDataPtr                  // Load base addresses
            pxor        xmm5,       xmm5                        // Clear Xmm5

            mov         ebx,        RefDataPtr1                 // Ref1
            pxor        xmm6,       xmm6                        // Clear Xmm6


            mov         ecx,        PixelsPerLine               // Get Width
            pxor        xmm7,       xmm7                        // Clear Xmm7

            mov         esi,        RefDataPtr2                 // Ref 2
            mov         edx,        RefPixelsPerLine            // Get Pitch


            // Row 1 and Row 2
            movq        xmm1,       QWORD PTR [ebx]             // Copy eight bytes from each of ref 1
            movq        xmm2,       QWORD PTR [esi]             // Copy eight bytes from each of ref 2

            punpcklbw   xmm1,       xmm6                        // unpack to shorts
            movq        xmm3,       QWORD PTR [ebx+edx]         // Copy eight bytes from each of ref 1

            punpcklbw   xmm2,       xmm6                        // unpack to shorts
            movq        xmm4,       QWORD PTR [esi+edx]         // Copy eight bytes from each of ref 2

            punpcklbw   xmm3,       xmm6                        // unpack to shorts
            paddw       xmm1,       xmm2                        // Add word values together.

            punpcklbw   xmm4,       xmm6                        // unpack to shorts
            psrlw       xmm1,       1                           // Devide by two (shift right 1)

            paddw       xmm3,       xmm4                        // add word values together
            movq        xmm0,       QWORD PTR [eax]             // copy eight source bytes to xmm2

            psrlw       xmm3,       1                           // divided by two
            movq        xmm2,       QWORD PTR [eax+ecx]         // copy eight source bytes to xmm2

            punpcklbw   xmm0,       xmm6                        // unpack to words
            punpcklbw   xmm2,       xmm6                        // unpack to words

            psubsw      xmm0,       xmm1                        // the difference
            psubsw      xmm2,       xmm3                        // the difference

            paddw       xmm5,       xmm0                        // accumulate the difference
            paddw       xmm5,       xmm2                        // accumulate the difference

            pmaddwd     xmm0,       xmm0                        // square and accumulate
            pmaddwd     xmm2,       xmm2                        // square and accumulate

            lea         eax,        [eax+ecx*2]
            lea         ebx,        [ebx+edx*2]

            lea         esi,        [esi+edx*2]
            paddd       xmm7,       xmm0                        // accumulate in mm7

            paddd       xmm7,       xmm2                        // accumulate in mm7


            // Row 3 and Row 4
            movq        xmm1,       QWORD PTR [ebx]             // Copy eight bytes from each of ref 1
            movq        xmm2,       QWORD PTR [esi]             // Copy eight bytes from each of ref 2

            punpcklbw   xmm1,       xmm6                        // unpack to shorts
            movq        xmm3,       QWORD PTR [ebx+edx]         // Copy eight bytes from each of ref 1

            punpcklbw   xmm2,       xmm6                        // unpack to shorts
            movq        xmm4,       QWORD PTR [esi+edx]         // Copy eight bytes from each of ref 2

            punpcklbw   xmm3,       xmm6                        // unpack to shorts
            paddw       xmm1,       xmm2                        // Add word values together.

            punpcklbw   xmm4,       xmm6                        // unpack to shorts
            psrlw       xmm1,       1                           // Devide by two (shift right 1)

            paddw       xmm3,       xmm4                        // add word values together
            movq        xmm0,       QWORD PTR [eax]             // copy eight source bytes to xmm2

            psrlw       xmm3,       1                           // divided by two
            movq        xmm2,       QWORD PTR [eax+ecx]         // copy eight source bytes to xmm2

            punpcklbw   xmm0,       xmm6                        // unpack to words
            punpcklbw   xmm2,       xmm6                        // unpack to words

            psubsw      xmm0,       xmm1                        // the difference
            psubsw      xmm2,       xmm3                        // the difference

            paddw       xmm5,       xmm0                        // accumulate the difference
            paddw       xmm5,       xmm2                        // accumulate the difference

            pmaddwd     xmm0,       xmm0                        // square and accumulate
            pmaddwd     xmm2,       xmm2                        // square and accumulate

            lea         eax,        [eax+ecx*2]
            lea         ebx,        [ebx+edx*2]

            lea         esi,        [esi+edx*2]
            paddd       xmm7,       xmm0                        // accumulate in mm7

            paddd       xmm7,       xmm2                        // accumulate in mm7


            // Row 5 and Row 6
            movq        xmm1,       QWORD PTR [ebx]             // Copy eight bytes from each of ref 1
            movq        xmm2,       QWORD PTR [esi]             // Copy eight bytes from each of ref 2

            punpcklbw   xmm1,       xmm6                        // unpack to shorts
            movq        xmm3,       QWORD PTR [ebx+edx]         // Copy eight bytes from each of ref 1

            punpcklbw   xmm2,       xmm6                        // unpack to shorts
            movq        xmm4,       QWORD PTR [esi+edx]         // Copy eight bytes from each of ref 2

            punpcklbw   xmm3,       xmm6                        // unpack to shorts
            paddw       xmm1,       xmm2                        // Add word values together.

            punpcklbw   xmm4,       xmm6                        // unpack to shorts
            psrlw       xmm1,       1                           // Devide by two (shift right 1)

            paddw       xmm3,       xmm4                        // add word values together
            movq        xmm0,       QWORD PTR [eax]             // copy eight source bytes to xmm2

            psrlw       xmm3,       1                           // divided by two
            movq        xmm2,       QWORD PTR [eax+ecx]         // copy eight source bytes to xmm2

            punpcklbw   xmm0,       xmm6                        // unpack to words
            punpcklbw   xmm2,       xmm6                        // unpack to words

            psubsw      xmm0,       xmm1                        // the difference
            psubsw      xmm2,       xmm3                        // the difference

            paddw       xmm5,       xmm0                        // accumulate the difference
            paddw       xmm5,       xmm2                        // accumulate the difference

            pmaddwd     xmm0,       xmm0                        // square and accumulate
            pmaddwd     xmm2,       xmm2                        // square and accumulate

            lea         eax,        [eax+ecx*2]
            lea         ebx,        [ebx+edx*2]

            lea         esi,        [esi+edx*2]
            paddd       xmm7,       xmm0                        // accumulate in mm7

            paddd       xmm7,       xmm2                        // accumulate in mm7


            // Row 7 and Row 8
            movq        xmm1,       QWORD PTR [ebx]             // Copy eight bytes from each of ref 1
            movq        xmm2,       QWORD PTR [esi]             // Copy eight bytes from each of ref 2

            punpcklbw   xmm1,       xmm6                        // unpack to shorts
            movq        xmm3,       QWORD PTR [ebx+edx]         // Copy eight bytes from each of ref 1

            punpcklbw   xmm2,       xmm6                        // unpack to shorts
            movq        xmm4,       QWORD PTR [esi+edx]         // Copy eight bytes from each of ref 2

            punpcklbw   xmm3,       xmm6                        // unpack to shorts
            paddw       xmm1,       xmm2                        // Add word values together.

            punpcklbw   xmm4,       xmm6                        // unpack to shorts
            psrlw       xmm1,       1                           // Devide by two (shift right 1)

            paddw       xmm3,       xmm4                        // add word values together
            movq        xmm0,       QWORD PTR [eax]             // copy eight source bytes to xmm2

            psrlw       xmm3,       1                           // divided by two
            movq        xmm2,       QWORD PTR [eax+ecx]         // copy eight source bytes to xmm2

            punpcklbw   xmm0,       xmm6                        // unpack to words
            punpcklbw   xmm2,       xmm6                        // unpack to words

            psubsw      xmm0,       xmm1                        // the difference
            psubsw      xmm2,       xmm3                        // the difference

            paddw       xmm5,       xmm0                        // accumulate the difference
            paddw       xmm5,       xmm2                        // accumulate the difference

            pmaddwd     xmm0,       xmm0                        // square and accumulate
            pmaddwd     xmm2,       xmm2                        // square and accumulate

            paddd       xmm7,       xmm0                        // accumulate in mm7
            paddd       xmm7,       xmm2                        // accumulate in mm7

            movdqa      xmm0,       xmm5
            movdqa      xmm1,       xmm7

            psrldq      xmm5,       8
            psrldq      xmm7,       8

            paddw       xmm0,       xmm5
            paddd       xmm1,       xmm7


            movq        QWORD Ptr [MmxXSum],    xmm0            // copy back accumulated results into normal memory
            movq        QWORD Ptr [MmxXXSum],   xmm1            // copy back accumulated results into normal memory

        }

        // Now accumulate the final results.
        XSum = MmxXSum[0] + MmxXSum[1] + MmxXSum[2] + MmxXSum[3];
        XXSum = MmxXXSum[0] + MmxXXSum[1];
    }

    // Compute and return population variance as mis-match metric.
    return ( ((XXSum << 6) - XSum*XSum ) );
}

/****************************************************************************
 *
 *  ROUTINE       : WmtGetMBFrameVertVar
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Vertical variance for the entire frame.
 *
 *  FUNCTION      : Calculates the vertical variance for a frame based
 *                  upon the sum of the local 2 pixel variances within
 *                  the entire frame.
 *
 *  SPECIAL NOTE  : The difference between the last two rows in a macro-
 *                  block are not accounted for!
 *
 ****************************************************************************/
UINT32 WmtGetMBFrameVertVar ( CP_INSTANCE *cpi )
{
    UINT32 FrameError;
    INT32 Stride  = cpi->pb.Configuration.VideoFrameWidth;
//    UINT8 *SrcPtr = &cpi->yuv1ptr[cpi->pb.mbi.Source];
//sjlhack
    UINT8 *SrcPtr = &cpi->yuv1ptr[cpi->pb.mbi.blockDxInfo[0].Source];

    __asm
    {
        mov         ecx,    DWORD PTR [Stride]
        mov         eax,    DWORD PTR [SrcPtr]
        
        pxor        xmm7,   xmm7
        pxor        xmm6,   xmm6

        mov         edx,    7

WmtGetMBFrameVertVarLoop:

        movdqa      xmm1,   [eax]               ; 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
        movdqa      xmm0,   [eax+ecx]           ; 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f
            
        movdqa      xmm3,   xmm0                ; 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
        punpcklbw   xmm0,   xmm7                ; xx 00 xx 01 xx 02 xx 03 xx 04 xx 05 xx 06 xx 07
        
        movdqa      xmm4,   xmm1                ; 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f
        punpckhbw   xmm3,   xmm7                ; xx 08 xx 09 xx 0a xx 0b xx 0c xx 0d xx 0e xx 0f

        movdqa      xmm2,   [eax+ecx*2]         ; 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f
        punpcklbw   xmm1,   xmm7                ; xx 10 xx 11 xx 12 xx 13 xx 14 xx 15 xx 16 xx 17

        movdqa      xmm5,   xmm2                ; 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f
        punpckhbw   xmm4,   xmm7                ; xx 18 xx 19 xx 1a xx 1b xx 1c xx 1d xx 1e xx 1f

        psubw       xmm1,   xmm0                ; difference between 0 1    low eight
        pmaddwd     xmm1,   xmm1                ; SD         between 0 1    low eight

        punpcklbw   xmm2,   xmm7                ; xx 20 xx 21 xx 22 xx 23 xx 24 xx 25 xx 26 xx 27
        psubw       xmm4,   xmm3                ; difference between 0 1    high four

        pmaddwd     xmm4,   xmm4                ; SD         between 0 1    high four
        punpckhbw   xmm5,   xmm7                ; xx 28 xx 29 xx 2a xx 2b xx 2c xx 2d xx 2e xx 2f

        psubw       xmm2,   xmm0                ; difference between 0 2    low eight
        pmaddwd     xmm2,   xmm2                ; SD         between 0 2    low eight

        psubw       xmm5,   xmm3                ; difference between 0 2    High eight
        pmaddwd     xmm5,   xmm5                ; SD         between 0 2    High eight

        paddd       xmm1,   xmm4
        paddd       xmm2,   xmm5                

        paddd       xmm6,   xmm1                ; accumlated in xmm6
        paddd       xmm6,   xmm2                ; xx xx xx s0 xx xx xx s1 xx xx xx s2 xx xx xx s3

        lea         eax,    [eax+ecx*2]
        sub         edx,    1

        jnz         WmtGetMBFrameVertVarLoop

        movdqa      xmm0,   xmm6                ; xx xx xx s0 xx xx xx s1 xx xx xx s2 xx xx xx s3
        punpckldq   xmm6,   xmm7                ; xx xx xx xx xx xx xx s0 xx xx xx xx xx xx xx s2

        punpckhdq   xmm0,   xmm7                ; xx xx xx xx xx xx xx s1 xx xx xx xx xx xx xx s3
        paddd       xmm0,   xmm6                ; xx xx xx xx xx xx xxs01 xx xx xx xx xx xx xxs23

        movdqa      xmm6,   xmm0                ; xx xx xx xx xx xx xxs01 xx xx xx xx xx xx xxs23
        psrldq      xmm0,   8;                  ; xx xx xx xx xx xx xx 23 xx xx xx xx xx xx xx xx

        paddd       xmm0,   xmm6                 
        movd        [FrameError], xmm0
    }

    return FrameError;
}

/****************************************************************************
 *
 *  ROUTINE       : WmtGetMBFieldVertVar
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Vertical variance for the entire frame.
 *
 *  FUNCTION      : Calculates the vertical variance for a frame based
 *                  upon the sum of the local 2 pixel variances within
 *                  the individual fields of the frame.
 *
 *  SPECIAL NOTE  : The difference between the last two rows in a macro-
 *                  block are not accounted for!
 *
 ****************************************************************************/
UINT32 WmtGetMBFieldVertVar( CP_INSTANCE *cpi )
{
    UINT32 FieldError;
    INT32 Stride = cpi->pb.Configuration.VideoFrameWidth;
//    UINT8 *SrcPtr = &cpi->yuv1ptr[cpi->pb.mbi.Source];
//sjlhack
    UINT8 *SrcPtr = &cpi->yuv1ptr[cpi->pb.mbi.blockDxInfo[0].Source];

    __asm
    {
        mov         ecx,    DWORD PTR [Stride]
        mov         eax,    DWORD PTR [SrcPtr]
        
        pxor        xmm7,   xmm7
        pxor        xmm6,   xmm6

        mov         edx,    7

WmtGetMBFieldVertVarLoop:

        movdqa      xmm1,   [eax]               ; 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
        movdqa      xmm0,   [eax+ecx*2]         ; 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f
            
        movdqa      xmm2,   xmm0                ; 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
        punpcklbw   xmm0,   xmm7                ; xx 00 xx 01 xx 02 xx 03 xx 04 xx 05 xx 06 xx 07
        
        movdqa      xmm3,   xmm1                ; 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f
        punpckhbw   xmm2,   xmm7                ; xx 08 xx 09 xx 0a xx 0b xx 0c xx 0d xx 0e xx 0f

        punpcklbw   xmm1,   xmm7                ; xx 20 xx 21 xx 22 xx 23 xx 24 xx 25 xx 26 xx 27
        punpckhbw   xmm3,   xmm7                ; xx 28 xx 29 xx 2a xx 2b xx 2c xx 2d xx 2e xx 2f


        psubw       xmm0,   xmm1;
        pmaddwd     xmm0,   xmm0;
        
        psubw       xmm2,   xmm3;
        pmaddwd     xmm2,   xmm2

        paddd       xmm0,   xmm2;
        lea         eax,    [eax + ecx]

        movdqa      xmm2,   [eax]
        movdqa      xmm3,   [eax + ecx*2]

        movdqa      xmm4,   xmm2    ;
        punpcklbw   xmm2,   xmm7

        movdqa      xmm5,   xmm3
        punpckhbw   xmm4,   xmm7

        punpcklbw   xmm3,   xmm7
        punpckhbw   xmm5,   xmm7

        psubw       xmm2,   xmm3
        pmaddwd     xmm2,   xmm2

        psubw       xmm4,   xmm5
        pmaddwd     xmm4,   xmm4

        paddd       xmm2,   xmm4
        paddd       xmm0,   xmm2

        paddd       xmm6,   xmm0

        lea         eax,    [eax+ecx]
        sub         edx,    1

        jnz         WmtGetMBFieldVertVarLoop

        movdqa      xmm0,   xmm6                ; xx xx xx s0 xx xx xx s1 xx xx xx s2 xx xx xx s3
        punpckldq   xmm6,   xmm7                ; xx xx xx xx xx xx xx s0 xx xx xx xx xx xx xx s2

        punpckhdq   xmm0,   xmm7                ; xx xx xx xx xx xx xx s1 xx xx xx xx xx xx xx s3
        paddd       xmm0,   xmm6                ; xx xx xx xx xx xx xxs01 xx xx xx xx xx xx xxs23

        movdqa      xmm6,   xmm0                ; xx xx xx xx xx xx xxs01 xx xx xx xx xx xx xxs23
        psrldq      xmm0,   8;                  ; xx xx xx xx xx xx xx 23 xx xx xx xx xx xx xx xx

        paddd       xmm0,   xmm6                 
        movd        [FieldError], xmm0
    }

    return FieldError;
}

/****************************************************************************
 *
 *  ROUTINE       : FilterBlock2dBil_SAD_wmt
 *
 *  INPUTS        : UINT8 *SrcPtr           : Pointer to source block.
 *                  INT32 SrcStride         : Stride of source block.
 *                  UINT8 *RefPtr           : Pointer to reference block.
 *                  UINT32 SrcPixelsPerLine : Number of pels per line in source.
 *                  INT16 *HFilter          : Pointer to array of horizontal filter taps.
 *                  INT16 *VFilter          : Pointer to array of vertical filter taps.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: SAD.
 *
 *  FUNCTION      : Produces a filtered fractional block prediction in 2-D
 *   				using bi-linear filters and calculates the SAD.
 *
 *  SPECIAL NOTE  : The difference between the last two rows in a macro-
 *                  block are not accounted for!
 *
 ****************************************************************************/
_inline UINT32 FilterBlock2dBil_SAD_wmt
( 
    UINT8 *SrcPtr, 
    INT32 SrcStride, 
    UINT8 *RefPtr, 
    UINT32 SrcPixelsPerLine, 
    INT16 *HFilter, 
    INT16 *VFilter 
)
{
    UINT32 Error;

    __asm
    {
        mov         eax,        HFilter             ; 
        mov         edi,        SrcPtr              ; 

        mov         esi,        RefPtr              ;
        mov         ecx,        8            ;

        mov         edx,        SrcPixelsPerLine    ;
               
        movdqa      xmm1,       [eax]               ;
        movdqa      xmm2,       [eax+16]            ;
        
        mov         eax,        VFilter             ;       
        pxor        xmm0,       xmm0                ;

        // get the first horizontal line done       ;
        movdqu      xmm3,       [esi]               ; xx 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14
        movdqa      xmm4,       xmm3                ; make a copy of current line
        
        punpcklbw   xmm3,       xmm0                ; xx 00 01 02 03 04 05 06
        psrldq      xmm4,       1                   ; 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 xx        
        
        pmullw      xmm3,       xmm1                ;        
        punpcklbw   xmm4,       xmm0                ; 00 01 02 03 04 05 06 07

        pmullw      xmm4,       xmm2                ;
        paddw       xmm3,       xmm4                ;   

        paddw       xmm3,       rd                  ; 
        psraw       xmm3,       FILTER_SHIFT        ; ready for output
        
        movdqa      xmm5,       xmm3                ;
        pxor        mm7,        mm7

        add         esi,        edx                 ; next line
NextRow:
        pmullw      xmm5,       [eax]               ; 
        movdqu      xmm3,       [esi]               ; xx 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14

        movdqa      xmm4,       xmm3                ; make a copy of current line        
        punpcklbw   xmm3,       xmm0                ; xx 00 01 02 03 04 05 06

        psrldq      xmm4,       1                   ; 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 xx                
        pmullw      xmm3,       xmm1                ;        
        punpcklbw   xmm4,       xmm0                ; 00 01 02 03 04 05 06 07

        movdqa      xmm6,       xmm5                ; 
        pmullw      xmm4,       xmm2                ;

        paddw       xmm3,       xmm4                ;   
        paddw       xmm3,       rd                  ; 

        psraw       xmm3,       FILTER_SHIFT        ; ready for output
        movdqa      xmm5,       xmm3                ; make a copy for the next row
        
        pmullw      xmm3,       [eax+16]            ; 
        paddw       xmm6,       xmm3                ;
        

        paddw       xmm6,       rd                  ; xmm6 += round value
        psraw       xmm6,       FILTER_SHIFT        ; xmm6 /= 128

        packuswb    xmm6,       xmm0                ; pack and unpack to saturate
        movdq2q     mm0,        xmm6

        movq        mm1,        [edi]               ;
        psadbw      mm0,        mm1                 ;
        
        paddd       mm7,        mm0
        
        add         esi,        edx                 ; next line
        add         edi,        SrcStride           ;                   ; 

        dec         ecx                             ;
        jne         NextRow                         
        
        movd        Error,      mm7;

    }
    return  Error;
}

/****************************************************************************
 *
 *  ROUTINE       : FilterBlock1d_vb8_SAD_wmt
 *
 *  INPUTS        : UINT8 *SrcPtr        : Pointer to source block.
 *                  INT32 SrcStride      : Stride of source block.
 *                  UINT8 *RefPtr        : Pointer to reference block.
 *                  UINT32 PixelsPerLine : Number of pels per line in source.
 *                  UINT32 FilterStep    : Pointer to array of horizontal filter taps.
 *                  INT16 *Filter        : Pointer to array of filter taps.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: SAD.
 *
 *  FUNCTION      : Produces a filtered fractional block vertically 
 *   				using bi-linear filters and calculates the SAD.
 *
 *  SPECIAL NOTE  : The difference between the last two rows in a macro-
 *                  block are not accounted for!
 *
 ****************************************************************************/
_inline UINT32 FilterBlock1d_vb8_SAD_wmt
( 
    UINT8 *SrcPtr, 
    INT32 SrcStride, 
    UINT8 *RefPtr, 
    UINT32 PixelsPerLine, 
    UINT32 PixelStep, 
    INT16 *Filter 
)
{
    UINT32 Error;
    __asm

    {

        mov         edi,        Filter
        movdqa      xmm1,       [edi]               ; xmm3 *= kernel 0 modifiers.
        movdqa      xmm2,       [edi + 16]          ; xmm3 *= kernel 0 modifiers.

        mov         edi,        SrcPtr
		mov			esi,        RefPtr
        
        mov         ecx,        8                   ;

        mov         edx,        SrcStride
        mov         eax,        PixelsPerLine;
        
        pxor        mm7,        mm7
		pxor		xmm0,       xmm0                ; xmm0 = 00000000

nextrow:
        movdqu		xmm3,       [esi]               ; xmm3 = p0..p16
        punpcklbw   xmm3,       xmm0                ; xmm3 = p0..p8
        pmullw      xmm3,       xmm1                ; xmm3 *= kernel 0 modifiers.

        movdqu		xmm4,       [esi + eax ]         ; xmm4 = p0..p16
        punpcklbw   xmm4,       xmm0                ; xmm4 = p0..p8
        pmullw      xmm4,       xmm2                ; xmm4 *= kernel 1 modifiers.
        paddw       xmm3,       xmm4                ; xmm3 += xmm4

        paddw       xmm3,       rd                  ; xmm3 += round value
        psraw       xmm3,       FILTER_SHIFT        ; xmm3 /= 128
        packuswb    xmm3,       xmm0                ; pack and unpack to saturate

        movdq2q     mm0,        xmm3
        movq        mm1,        [edi]               ;
        
        psadbw      mm0,        mm1                 ;
        paddd       mm7,        mm0
        
        // the subsequent iterations repeat 3 out of 4 of these reads.  Since the 
        // recon block should be in cache this shouldn't cost much.  Its obviously 
        // avoidable!!!. 
        add         esi,        eax
        add         edi,        edx 

        dec         ecx                             ; decrement count
        jnz         nextrow                         ; next row

        movd        Error,      mm7       

    }
    return Error;
}

/****************************************************************************
 *
 *  ROUTINE       : FilterBlock1d_hb8_SAD_wmt
 *
 *  INPUTS        : UINT8 *SrcPtr           : Pointer to source block.
 *                  INT32 SrcStride         : Stride of source block.
 *                  UINT8 *RefPtr           : Pointer to reference block.
 *                  UINT32 SrcPixelsPerLine : Number of pels per line in source.
 *                  UINT32 FilterStep       : Offset to nest pixel in input image.
 *                  INT16 *Filter           : Pointer to array of filter taps.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: SAD.
 *
 *  FUNCTION      : Produces a filtered fractional block horizontally 
 *   				using bi-linear filters and calculates the SAD.
 *
 *  SPECIAL NOTE  : The difference between the last two rows in a macro-
 *                  block are not accounted for!
 *
 ****************************************************************************/
_inline UINT32 FilterBlock1d_hb8_SAD_wmt
(
    UINT8 *SrcPtr, 
    INT32 SrcStride, 
    UINT8 *RefPtr, 
    UINT32 SrcPixelsPerLine, 
    UINT32 PixelStep, 
    INT16 *Filter 
)
{
    UINT32 Error = 0;

    __asm
    {

        mov         edi,        Filter
        movdqa      xmm1,       [edi]               ; xmm3 *= kernel 0 modifiers.
        movdqa      xmm2,       [edi + 16]          ; xmm3 *= kernel 0 modifiers.

        mov         edi,        SrcPtr
		mov			esi,        RefPtr
		
        pxor		xmm0,       xmm0                ; xmm0 = 00000000
        pxor        mm7,        mm7                 ; mm7 = 0
        
        mov         ecx,        8                   ;

        mov         edx,        SrcStride
        mov         eax,        SrcPixelsPerLine;

nextrow:
        movdqu		xmm3,       [esi]               ; xmm3 = p-1..p14    
        movdqu      xmm5,       xmm3                ; xmm4 = p-1..p14

        punpcklbw   xmm3,       xmm0                ; xmm3 = p-1..p6
        pmullw      xmm3,       xmm1                ; xmm3 *= kernel 0 modifiers.

        psrldq      xmm5,       1                   ; xmm4 = p0..p13
        punpcklbw   xmm5,       xmm0                ; xmm5 = p0..p7
        
        pmullw      xmm5,       xmm2                ; xmm5 *= kernel 1 modifiers
        paddw       xmm3,       xmm5                ; xmm3 += xmm5

        paddw       xmm3,       rd                  ; xmm3 += round value
        psraw       xmm3,       FILTER_SHIFT        ; xmm3 /= 128
        
        packuswb    xmm3,       xmm0                ; pack and unpack to saturate
        
        movdq2q     mm0,        xmm3
        movq        mm1,        [edi]               ; read src
    
        psadbw      mm0,        mm1                 ;
        paddd       mm7,        mm0

        add         esi,        eax                 ; next line
        add         edi,        edx                 ; 

        dec         ecx                             ; decrement count
        jnz         nextrow                         ; next row

        movd        Error,        mm7;
    }
    return Error;
}
                         
/****************************************************************************
 *
 *  ROUTINE       : FiltBlockBilGetSad_wmt
 *
 *  INPUTS        : UINT8 *SrcPtr        : Pointer to source block.
 *                  INT32 SrcStride      : Stride of source block.
 *                  UINT8 *ReconPtr1     : Pointer to first reference block.
 *                  UINT8 *ReconPtr2     : Pointer to second reference block.
 *                  UINT32 PixelsPerLine : Number of pels per line in source.
 *                  UINT32 FilterStep    : Offset to nest pixel in input image.
 *                  INT32  ModX          : Fraction part of MV x-component.
 *                  INT32  ModY          : Fraction part of MV y-component.
 *                  UINT32 BestSoFar     : Best error found so far.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: SAD.
 *
 *  FUNCTION      : Produces a filtered fractional pel block using
 *   				bi-linear filters and calculates the SAD.
 *
 *  SPECIAL NOTE  : The difference between the last two rows in a macro-
 *                  block are not accounted for!
 *
 ****************************************************************************/
UINT32 FiltBlockBilGetSad_wmt
(
    UINT8 *SrcPtr,
    INT32 SrcStride,
    UINT8 *ReconPtr1,
    UINT8 *ReconPtr2,
    INT32 PixelsPerLine,
    INT32 ModX, 
    INT32 ModY,
    UINT32 BestSoFar
)
{
    INT32  diff;
    UINT32 Error;

    // swap pointers so ReconPtr1 smaller (above, left, above-right or above-left )
	diff = ReconPtr2-ReconPtr1;
	
    // The ModX and ModY arguments are the bottom three bits of the signed motion vector components (at 1/8th pel precision).
	// This works out to be what we want... despite the pointer swapping that goes on below.
	// For example... if the X component of the vector is a +ve ModX = X%8.
	//                if the X component of the vector is a -ve ModX = 8+(X%8) where X%8 is in the range -7 to -1.
	if ( diff < 0 ) 
	{											// swap pointers so ReconPtr1 smaller
		UINT8 *temp = ReconPtr1;
		ReconPtr1   = ReconPtr2;
		ReconPtr2   = temp;
		diff        = (int)(ReconPtr2-ReconPtr1);
	}

	if( diff==1 )
		Error = FilterBlock1d_hb8_SAD_wmt(SrcPtr, SrcStride, ReconPtr1, PixelsPerLine, 1, BilinearFilters_wmt[ModX] );
	else if (diff == (int)(PixelsPerLine) )				// Fractional pixel in vertical only
		Error = FilterBlock1d_vb8_SAD_wmt(SrcPtr, SrcStride, ReconPtr1, PixelsPerLine, PixelsPerLine, BilinearFilters_wmt[ModY]);
	else if(diff == (int)(PixelsPerLine - 1))			// ReconPtr1 is Top right
        Error = FilterBlock2dBil_SAD_wmt( SrcPtr, SrcStride, ReconPtr1-1, PixelsPerLine, BilinearFilters_wmt[ModX], BilinearFilters_wmt[ModY] );        
	else if(diff == (int)(PixelsPerLine + 1) )			// ReconPtr1 is Top left
        Error = FilterBlock2dBil_SAD_wmt( SrcPtr, SrcStride, ReconPtr1, PixelsPerLine, BilinearFilters_wmt[ModX], BilinearFilters_wmt[ModY] );		
    
    return Error;
}


/****************************************************************************
 * 
 *  ROUTINE       : WmtComputeBlockReconError
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Variance for the block (Scaled by 2^6)
 *
 *  FUNCTION      : Computes a reconstruction error variance for a block.
 *
 *  SPECIAL NOTES : The variance value returned is scaled by a factor
 *                  2^6 (i.e.64). 
 *
 ****************************************************************************/

UINT32 WmtComputeBlockReconError ( CP_INSTANCE *cpi, UINT32 bp )
{
    INT32  XXSum;
	INT32  MaxXXDiff;

	UINT8 *NewDataPtr   = &cpi->yuv1ptr[cpi->pb.mbi.blockDxInfo[bp].Source];
	UINT8 *RefDataPtr1  = &cpi->pb.ThisFrameRecon[cpi->pb.mbi.blockDxInfo[bp].thisRecon];

    INT32  SourceStride = cpi->pb.mbi.blockDxInfo[bp].CurrentSourceStride;
	INT32  ReconStride  = cpi->pb.mbi.blockDxInfo[bp].CurrentReconStride;
    __asm
    {

        mov         esi,        NewDataPtr
        mov         edi,        RefDataPtr1

        mov         eax,        SourceStride
        mov         edx,        ReconStride

        lea         ecx,        [esi+eax*8]

        pxor        xmm7,       xmm7            
        pxor        xmm6,       xmm6

        pxor        xmm5,       xmm5
        
WmtReconErrorLoop:
        movq        xmm0,       QWORD ptr [esi]         // s0 s1 s2 s3 s4 s5 s6 s7 xx xx xx xx xx xx xx xx
        movq        xmm1,       QWORD ptr [edi]         // r0 r1 r2 r3 r4 r5 r6 r7 xx xx xx xx xx xx xx xx

        movdqa      xmm2,       xmm0                    //  make a copy
        movdqa      xmm3,       xmm1                    //  make a copy

        psubusb     xmm0,       xmm1                    //
        psubusb     xmm3,       xmm2                    //

        por         xmm0,       xmm3                    // abs( d0 d1 d2 d3 d4 d5 d6 d7 xx xx xx xx xx xx xx xx )
        movdqa      xmm2,       xmm0                    // make a copy

        punpcklbw   xmm0,       xmm7                    // abs ( xxd0 xxd1 xxd2 xxd3 xxd4 xxd5 xxd6 xxd7)       
        punpcklbw   xmm2,       xmm7                    // abs ( xxd0 xxd1 xxd2 xxd3 xxd4 xxd5 xxd6 xxd7)       

        movdqa      xmm1,       xmm2                    // abs ( xxd0 xxd1 xxd2 xxd3 xxd4 xxd5 xxd6 xxd7)       
        pmaddwd      xmm0,       xmm0                    // 
    
        punpcklwd   xmm1,       xmm7                    //  xxxx xxd0 xxxx xxd1 xxxx xxd2 xxxx xxd3
        punpckhwd   xmm2,       xmm7                    //  xxxx xxd4 xxxx xxd5 xxxx xxd6 xxxx xxd7
        
        pmaxsw      xmm1,       xmm2                    //  xxxx xxM0 xxxx xxM1 xxxx xxM2 xxxx xxM3
        movdqa      xmm2,       xmm1                    //  xxxx xxM0 xxxx xxM1 xxxx xxM2 xxxx xxM3

        punpckldq   xmm1,       xmm7                    //  xxxx xxxx xxxx xxM0 xxxx xxxx xxxx xxM1
        punpckhdq   xmm2,       xmm7                    //  xxxx xxxx xxxx xxM2 xxxx xxxx xxxx xxM3
        
        pmaxsw      xmm1,       xmm2                    //  xxxx xxxx xxxx max0 xxxx xxxx xxxx max1
        movdqa      xmm2,       xmm1                    //  xxxx xxxx xxxx max0 xxxx xxxx xxxx max1

        psrldq      xmm1,       8                       //  xxxx xxxx xxxx xxxx xxxx xxxx xxxx max0
        pmaxsw      xmm1,       xmm2                    //  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx MMAX

        pmaxsw      xmm5,       xmm1                    //  
        paddd       xmm6,       xmm0                    // 

        lea         esi,        [esi+eax]
        lea         edi,        [edi+edx]               //

        cmp         ecx,        esi
        jne         WmtReconErrorLoop

        movd        MaxXXDiff,  xmm5                    // get the max
                
        movdqa      xmm4,       xmm6                    // xxxx xxs0 xxxx xxs1 xxxx xxs2 xxxx xxs3
        psrldq      xmm4,       8                       // xxxx xxs2 xxxx xxs3 xxxx xxxx xxxx xxxx                      

        paddd       xmm6,       xmm4                    // xxxx s0s2 xxxx s1s3 xxxxxxxxxxxxxxxxxxx
        movdqa      xmm4,       xmm6                    // xxxx s0s2 xxxx s1s3 xxxxxxxxxxxxxxxxxxx

        psrldq      xmm4,       4                       // xxxx s1s3 xxxxxxxxxxxxxxxxxxxxxxxxxxxxx
        paddd       xmm6,       xmm4                    //      0123
        
        movd        XXSum,      xmm6
        

    }
	return  (UINT32)(XXSum + (2 * MaxXXDiff*MaxXXDiff)) << 6;
}

