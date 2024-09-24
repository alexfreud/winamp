/****************************************************************************
*
*   Module Title :     OptFunctions.c
*
*   Description  :     Encoder system dependant functions.
*
****************************************************************************/

/****************************************************************************
*  Header Files
****************************************************************************/
#include "compdll.h"
#include "math.h"
/****************************************************************************
*  Macros
****************************************************************************/
#pragma warning(disable:4799)

#define FILTER_WEIGHT 128
#define FILTER_SHIFT  7

/****************************************************************************
*  Module Statics
****************************************************************************/
static __declspec(align(16)) short rd[] = { 64, 64, 64, 64, 64, 64, 64, 64 };

/****************************************************************************
*  Imports
****************************************************************************/
extern INT16  BilinearFilters_mmx[8][16];

/****************************************************************************
 *
 *  ROUTINE       : MmxGetSAD
 *
 *  INPUTS        : UINT8 *NewDataPtr       : Pointer to first input data array.
 *                  INT32  PixelsPerLine    : Length of line for NewDataPtr.
 *                  UINT8 *RefDataPtr       : Pointer to second input data array.
 *                  INT32  RefPixelsPerLine : Length of line for RefDataPtr.
 *                  INT32  ErrorSoFar       : Error accumulated before this call.
 *                  INT32  BestSoFar        : (NOT USED).
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : INT32: SAD for the two blocks.
 *
 *  FUNCTION      : Calculates the sum of the absolute differences for 
 *                  the two blocks.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
INT32 MmxGetSAD
( 
    UINT8 *NewDataPtr, 
    INT32 PixelsPerLine,
    UINT8 *RefDataPtr,
    INT32 RefPixelsPerLine,
    INT32 ErrorSoFar, 
    INT32 BestSoFar 
)
{
    INT32   DiffVal = ErrorSoFar;
    INT16   DiffAcc[4] = { 0, 0, 0, 0};     // MMX accumulator.

    // MMX code for SAD.
__asm
    {
        pxor        mm6, mm6                    ; Blank mmx6
        pxor        mm7, mm7                    ; Blank mmx7

        mov         eax,dword ptr [NewDataPtr]  ; Load base addresses
        mov         ebx,dword ptr [RefDataPtr]
        mov         ecx,dword ptr [PixelsPerLine]
        mov         edx,dword ptr [RefPixelsPerLine]

        // Row 1
        movq        mm0, [eax]                  ; Copy eight bytes to mm0
        movq        mm1, [ebx]                  ; Copy eight bytes to mm1
        movq        mm2, mm0                    ; Take copy of MM0

        psubusb     mm0, mm1                    ; A-B to MM0
        psubusb     mm1, mm2                    ; B-A to MM1
        por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

        movq        mm1, mm0                    ; keep a copy
        punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
        paddw       mm7, mm0                    ; accumulate difference...
        punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
        add         eax,ecx                     ; Inc pointer into the new data
        paddw       mm7, mm1                    ; accumulate difference...
        add         ebx,edx                     ; Inc pointer into ref data

        // Row 2
        movq        mm0, [eax]                  ; Copy eight bytes to mm0
        movq        mm1, [ebx]                  ; Copy eight bytes to mm1
        movq        mm2, mm0                    ; Take copy of MM0

        psubusb     mm0, mm1                    ; A-B to MM0
        psubusb     mm1, mm2                    ; B-A to MM1
        por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

        movq        mm1, mm0                    ; keep a copy
        punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
        paddw       mm7, mm0                    ; accumulate difference...
        punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
        add         eax,ecx                     ; Inc pointer into the new data
        paddw       mm7, mm1                    ; accumulate difference...
        add         ebx,edx                     ; Inc pointer into ref data

        // Row 3
        movq        mm0, [eax]                  ; Copy eight bytes to mm0
        movq        mm1, [ebx]                  ; Copy eight bytes to mm1
        movq        mm2, mm0                    ; Take copy of MM0

        psubusb     mm0, mm1                    ; A-B to MM0
        psubusb     mm1, mm2                    ; B-A to MM1
        por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

        movq        mm1, mm0                    ; keep a copy
        punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
        paddw       mm7, mm0                    ; accumulate difference...
        punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
        add         eax,ecx                     ; Inc pointer into the new data
        paddw       mm7, mm1                    ; accumulate difference...
        add         ebx,edx                     ; Inc pointer into ref data

        // Row 4
        movq        mm0, [eax]                  ; Copy eight bytes to mm0
        movq        mm1, [ebx]                  ; Copy eight bytes to mm1
        movq        mm2, mm0                    ; Take copy of MM0

        psubusb     mm0, mm1                    ; A-B to MM0
        psubusb     mm1, mm2                    ; B-A to MM1
        por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

        movq        mm1, mm0                    ; keep a copy
        punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
        paddw       mm7, mm0                    ; accumulate difference...
        punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
        add         eax,ecx                     ; Inc pointer into the new data
        paddw       mm7, mm1                    ; accumulate difference...
        add         ebx,edx                     ; Inc pointer into ref data

        // Row 5
        movq        mm0, [eax]                  ; Copy eight bytes to mm0
        movq        mm1, [ebx]                  ; Copy eight bytes to mm1
        movq        mm2, mm0                    ; Take copy of MM0

        psubusb     mm0, mm1                    ; A-B to MM0
        psubusb     mm1, mm2                    ; B-A to MM1
        por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

        movq        mm1, mm0                    ; keep a copy
        punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
        paddw       mm7, mm0                    ; accumulate difference...
        punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
        add         eax,ecx                     ; Inc pointer into the new data
        paddw       mm7, mm1                    ; accumulate difference...
        add         ebx,edx                     ; Inc pointer into ref data

        // Row 6
        movq        mm0, [eax]                  ; Copy eight bytes to mm0
        movq        mm1, [ebx]                  ; Copy eight bytes to mm1
        movq        mm2, mm0                    ; Take copy of MM0

        psubusb     mm0, mm1                    ; A-B to MM0
        psubusb     mm1, mm2                    ; B-A to MM1
        por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

        movq        mm1, mm0                    ; keep a copy
        punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
        paddw       mm7, mm0                    ; accumulate difference...
        punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
        add         eax,ecx                     ; Inc pointer into the new data
        paddw       mm7, mm1                    ; accumulate difference...
        add         ebx,edx                     ; Inc pointer into ref data

        // Row 7
        movq        mm0, [eax]                  ; Copy eight bytes to mm0
        movq        mm1, [ebx]                  ; Copy eight bytes to mm1
        movq        mm2, mm0                    ; Take copy of MM0

        psubusb     mm0, mm1                    ; A-B to MM0
        psubusb     mm1, mm2                    ; B-A to MM1
        por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

        movq        mm1, mm0                    ; keep a copy
        punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
        paddw       mm7, mm0                    ; accumulate difference...
        punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
        add         eax,ecx                     ; Inc pointer into the new data
        paddw       mm7, mm1                    ; accumulate difference...
        add         ebx,edx                     ; Inc pointer into ref data

        // Row 8
        movq        mm0, [eax]                  ; Copy eight bytes to mm0
        movq        mm1, [ebx]                  ; Copy eight bytes to mm1
        movq        mm2, mm0                    ; Take copy of MM0

        psubusb     mm0, mm1                    ; A-B to MM0
        psubusb     mm1, mm2                    ; B-A to MM1
        por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

        movq        mm1, mm0                    ; keep a copy
        punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
        paddw       mm7, mm0                    ; accumulate difference...
        punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
        paddw       mm7, mm1                    ; accumulate difference...

        movq        DWORD PTR [DiffAcc], mm7    ; copy back accumulated results into normal memory
//      emms                                    ; Clear the MMX state.
    }

    //  Accumulate the 4 resulting word values.
    DiffVal += DiffAcc[0] + DiffAcc[1] + DiffAcc[2] + DiffAcc[3];

    return DiffVal;
}

/****************************************************************************
 *
 *  ROUTINE       : MmxGetHalfPixelSAD
 *
 *  INPUTS        : UINT8 *SrcData          : Pointer to first input data array.
 *                  INT32  PixelsPerLine    : Length of line for NewDataPtr.
 *                  UINT8 *RefDataPtr1      : Pointer to first reference data array.
 *                  UINT8 *RefDataPtr2      : Pointer to second reference data array.
 *                  INT32  RefPixelsPerLine : Length of line for RefDataPtr1/2.
 *                  INT32  ErrorSoFar       : Error accumulated before this call.
 *                  INT32  BestSoFar        : (NOT USED).
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
INT32 MmxGetHalfPixelSAD
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
    INT32   RefOffset = (int)(RefDataPtr1 - RefDataPtr2);
    INT16   DiffAcc[4] = { 0, 0, 0, 0 };     // MMX accumulator.

    if ( RefOffset == 0 )
    {
        // Simple case as for non 0.5 pixel
        DiffVal += MmxGetSAD ( SrcData, PixelsPerLine, RefDataPtr1, RefPixelsPerLine, ErrorSoFar, BestSoFar );
    }
    else
    {
__asm
        // MMX code for SAD.
        {
            pxor        mm6, mm6                    ; Blank mmx6
            pxor        mm7, mm7                    ; Blank mmx7

            mov         eax,dword ptr [SrcData]     ; Load base addresses and line increment
            mov         ebx,dword ptr [RefDataPtr1]
            mov         ecx,dword ptr [RefDataPtr2]
            mov         edx,dword ptr [PixelsPerLine]
            mov         esi,dword ptr [RefPixelsPerLine]

            // Row 1
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            paddw       mm1, mm2                    ; Add word values together.
            punpckhbw   mm4, mm6
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            paddw       mm3, mm4                    ; Add word values together.
            movq        mm0, [eax]                  ; Copy eight of src data to mm0
            psrlw       mm3, 1                      ; Devide by two (shift right 1)
            movq        mm2, mm0                    ; Take copy of MM0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data for SAD
            psubusb     mm0, mm1                    ; A-B to MM0
            psubusb     mm1, mm2                    ; B-A to MM1
            por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

            movq        mm1, mm0                    ; keep a copy
            punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
            paddw       mm7, mm0                    ; accumulate difference...
            punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
            add         eax,edx                     ; Inc pointer into the src data
            paddw       mm7, mm1                    ; accumulate difference...
            add         ebx,esi                     ; Inc pointer into ref1
            add         ecx,esi                     ; Inc pointer into ref2

            // Row 2
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2


            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            paddw       mm1, mm2                    ; Add word values together.
            punpckhbw   mm4, mm6
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            paddw       mm3, mm4                    ; Add word values together.
            movq        mm0, [eax]                  ; Copy eight of src data to mm0
            psrlw       mm3, 1                      ; Devide by two (shift right 1)
            movq        mm2, mm0                    ; Take copy of MM0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data for SAD
            psubusb     mm0, mm1                    ; A-B to MM0
            psubusb     mm1, mm2                    ; B-A to MM1
            por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

            movq        mm1, mm0                    ; keep a copy
            punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
            paddw       mm7, mm0                    ; accumulate difference...
            punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
            add         eax,edx                     ; Inc pointer into the src data
            paddw       mm7, mm1                    ; accumulate difference...
            add         ebx,esi                     ; Inc pointer into ref1
            add         ecx,esi                     ; Inc pointer into ref2

            // Row 3
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            paddw       mm1, mm2                    ; Add word values together.
            punpckhbw   mm4, mm6
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            paddw       mm3, mm4                    ; Add word values together.
            movq        mm0, [eax]                  ; Copy eight of src data to mm0
            psrlw       mm3, 1                      ; Devide by two (shift right 1)
            movq        mm2, mm0                    ; Take copy of MM0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data for SAD
            psubusb     mm0, mm1                    ; A-B to MM0
            psubusb     mm1, mm2                    ; B-A to MM1
            por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

            movq        mm1, mm0                    ; keep a copy
            punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
            paddw       mm7, mm0                    ; accumulate difference...
            punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
            add         eax,edx                     ; Inc pointer into the src data
            paddw       mm7, mm1                    ; accumulate difference...
            add         ebx,esi                     ; Inc pointer into ref1
            add         ecx,esi                     ; Inc pointer into ref2

            // Row 4
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            paddw       mm1, mm2                    ; Add word values together.
            punpckhbw   mm4, mm6
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            paddw       mm3, mm4                    ; Add word values together.
            movq        mm0, [eax]                  ; Copy eight of src data to mm0
            psrlw       mm3, 1                      ; Devide by two (shift right 1)
            movq        mm2, mm0                    ; Take copy of MM0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data for SAD
            psubusb     mm0, mm1                    ; A-B to MM0
            psubusb     mm1, mm2                    ; B-A to MM1
            por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

            movq        mm1, mm0                    ; keep a copy
            punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
            paddw       mm7, mm0                    ; accumulate difference...
            punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
            add         eax,edx                     ; Inc pointer into the src data
            paddw       mm7, mm1                    ; accumulate difference...
            add         ebx,esi                     ; Inc pointer into ref1
            add         ecx,esi                     ; Inc pointer into ref2

            // Row 5
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            paddw       mm1, mm2                    ; Add word values together.
            punpckhbw   mm4, mm6
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            paddw       mm3, mm4                    ; Add word values together.
            movq        mm0, [eax]                  ; Copy eight of src data to mm0
            psrlw       mm3, 1                      ; Devide by two (shift right 1)
            movq        mm2, mm0                    ; Take copy of MM0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data for SAD
            psubusb     mm0, mm1                    ; A-B to MM0
            psubusb     mm1, mm2                    ; B-A to MM1
            por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

            movq        mm1, mm0                    ; keep a copy
            punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
            paddw       mm7, mm0                    ; accumulate difference...
            punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
            add         eax,edx                     ; Inc pointer into the src data
            paddw       mm7, mm1                    ; accumulate difference...
            add         ebx,esi                     ; Inc pointer into ref1
            add         ecx,esi                     ; Inc pointer into ref2

            // Row 6
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            paddw       mm1, mm2                    ; Add word values together.
            punpckhbw   mm4, mm6
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            paddw       mm3, mm4                    ; Add word values together.
            movq        mm0, [eax]                  ; Copy eight of src data to mm0
            psrlw       mm3, 1                      ; Devide by two (shift right 1)
            movq        mm2, mm0                    ; Take copy of MM0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data for SAD
            psubusb     mm0, mm1                    ; A-B to MM0
            psubusb     mm1, mm2                    ; B-A to MM1
            por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

            movq        mm1, mm0                    ; keep a copy
            punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
            paddw       mm7, mm0                    ; accumulate difference...
            punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
            add         eax,edx                     ; Inc pointer into the src data
            paddw       mm7, mm1                    ; accumulate difference...
            add         ebx,esi                     ; Inc pointer into ref1
            add         ecx,esi                     ; Inc pointer into ref2

            // Row 7
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            paddw       mm1, mm2                    ; Add word values together.
            punpckhbw   mm4, mm6
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            paddw       mm3, mm4                    ; Add word values together.
            movq        mm0, [eax]                  ; Copy eight of src data to mm0
            psrlw       mm3, 1                      ; Devide by two (shift right 1)
            movq        mm2, mm0                    ; Take copy of MM0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data for SAD
            psubusb     mm0, mm1                    ; A-B to MM0
            psubusb     mm1, mm2                    ; B-A to MM1
            por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

            movq        mm1, mm0                    ; keep a copy
            punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
            paddw       mm7, mm0                    ; accumulate difference...
            punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
            add         eax,edx                     ; Inc pointer into the src data
            paddw       mm7, mm1                    ; accumulate difference...
            add         ebx,esi                     ; Inc pointer into ref1
            add         ecx,esi                     ; Inc pointer into ref2

            // Row 8
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            paddw       mm1, mm2                    ; Add word values together.
            punpckhbw   mm4, mm6
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            paddw       mm3, mm4                    ; Add word values together.
            movq        mm0, [eax]                  ; Copy eight of src data to mm0
            psrlw       mm3, 1                      ; Devide by two (shift right 1)
            movq        mm2, mm0                    ; Take copy of MM0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data for SAD
            psubusb     mm0, mm1                    ; A-B to MM0
            psubusb     mm1, mm2                    ; B-A to MM1
            por         mm0, mm1                    ; OR MM0 and MM1 gives abs differences in MM0

            movq        mm1, mm0                    ; keep a copy
            punpcklbw   mm0, mm6                    ; unpack to higher precision for accumulation
            paddw       mm7, mm0                    ; accumulate difference...
            punpckhbw   mm1, mm6                    ; unpack high four bytes to higher precision
            paddw       mm7, mm1                    ; accumulate difference...

            movq        DWORD PTR [DiffAcc], mm7    ; copy back accumulated results into normal memory
        }

        //  Accumulate the 4 word values in DiffAcc
        DiffVal += DiffAcc[0] + DiffAcc[1] + DiffAcc[2] + DiffAcc[3];
    }
    return DiffVal;
}

/****************************************************************************
 *
 *  ROUTINE       : MmxGetInterErr
 *
 *  INPUTS        : UINT8 *NewDataPtr       : Pointer to first input data array.
 *                  INT32  PixelsPerLine    : Length of line for NewDataPtr.
 *                  UINT8 *RefDataPtr1      : Pointer to first reference data array.
 *                  UINT8 *RefDataPtr2      : Pointer to second reference data array.
 *                  INT32  RefPixelsPerLine : Length of line for RefDataPtr1/2.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Error variance.
 *
 *  FUNCTION      : Calculates a difference error score for two blocks.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
UINT32 MmxGetInterErr
( 
    UINT8 *NewDataPtr, 
    INT32 PixelsPerLine, 
    UINT8 *RefDataPtr1, 
    UINT8 *RefDataPtr2, 
    INT32 RefPixelsPerLine 
)
{
    UINT32  XSum  = 0;
    UINT32  XXSum = 0;
    INT16   MmxXSum[4]  = { 0, 0, 0, 0 };      // XSum accumulators
    INT32   MmxXXSum[2] = { 0, 0 };            // XXSum accumulators

    INT32   AbsRefOffset = abs( (int)(RefDataPtr1 - RefDataPtr2) );

    // Mode of interpolation chosen based upon on the offset of the second reference pointer
    if ( AbsRefOffset == 0 )
    {
        __asm
        {
            pxor        mm5, mm5                    ; Blank mmx6
            pxor        mm6, mm6                    ; Blank mmx7
            pxor        mm7, mm7                    ; Blank mmx7

            mov         eax,dword ptr [NewDataPtr]  ; Load base addresses
            mov         ebx,dword ptr [RefDataPtr1]
            mov         ecx,dword ptr [PixelsPerLine]
            mov         edx,dword ptr [RefPixelsPerLine]

            // Row 1
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm1, [ebx]                  ; Copy eight bytes to mm1
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpcklbw   mm1, mm6
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         ebx,edx                     ; Inc pointer into ref data
            add         eax,ecx                     ; Inc pointer into the new data
            movq        mm1, [ebx]                  ; Copy eight bytes to mm1
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            // Row 2
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            punpcklbw   mm1, mm6
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         ebx,edx                     ; Inc pointer into ref data
            add         eax,ecx                     ; Inc pointer into the new data
            movq        mm1, [ebx]                  ; Copy eight bytes to mm1
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            // Row 3
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            punpcklbw   mm1, mm6
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         ebx,edx                     ; Inc pointer into ref data
            add         eax,ecx                     ; Inc pointer into the new data
            movq        mm1, [ebx]                  ; Copy eight bytes to mm1
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            // Row 4
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            punpcklbw   mm1, mm6
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         ebx,edx                     ; Inc pointer into ref data
            add         eax,ecx                     ; Inc pointer into the new data
            movq        mm1, [ebx]                  ; Copy eight bytes to mm1
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            // Row 5
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            punpcklbw   mm1, mm6
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         ebx,edx                     ; Inc pointer into ref data
            add         eax,ecx                     ; Inc pointer into the new data
            movq        mm1, [ebx]                  ; Copy eight bytes to mm1
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            // Row 6
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            punpcklbw   mm1, mm6
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         ebx,edx                     ; Inc pointer into ref data
            add         eax,ecx                     ; Inc pointer into the new data
            movq        mm1, [ebx]                  ; Copy eight bytes to mm1
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            // Row 7
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            punpcklbw   mm1, mm6
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         ebx,edx                     ; Inc pointer into ref data
            add         eax,ecx                     ; Inc pointer into the new data
            movq        mm1, [ebx]                  ; Copy eight bytes to mm1
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            // Row 8
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            punpcklbw   mm1, mm6
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         ebx,edx                     ; Inc pointer into ref data
            add         eax,ecx                     ; Inc pointer into the new data
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
        }

        // Now accumulate the final results.
        XSum  = MmxXSum[0] + MmxXSum[1] + MmxXSum[2] + MmxXSum[3];
        XXSum = MmxXXSum[0] + MmxXXSum[1];
    }
    // Simple half pixel reference data
    else
    {
__asm
        {
            pxor        mm5, mm5                    ; Blank mmx6
            pxor        mm6, mm6                    ; Blank mmx7
            pxor        mm7, mm7                    ; Blank mmx7

            mov         eax,dword ptr [NewDataPtr]  ; Load base addresses
            mov         ebx,dword ptr [RefDataPtr1]
            mov         ecx,dword ptr [RefDataPtr2]
            mov         edx,dword ptr [PixelsPerLine]
            mov         esi,dword ptr [RefPixelsPerLine]

            // Row 1
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            paddw       mm1, mm2                    ; Add word values together.
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            punpckhbw   mm4, mm6
            paddw       mm3, mm4                    ; Add word values together.
            psrlw       mm3, 1                      ; Devide by two (shift right 1)

            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpcklbw   mm1, mm6
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
            add         eax,edx                     ; Inc pointer into the new data
            add         ebx,esi                     ; Inc pointer into ref data
            add         ecx,esi                     ; Inc pointer into ref2 data

            // Row 2
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            paddw       mm1, mm2                    ; Add word values together.
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            punpckhbw   mm4, mm6
            paddw       mm3, mm4                    ; Add word values together.
            psrlw       mm3, 1                      ; Devide by two (shift right 1)

            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpcklbw   mm1, mm6
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
            add         eax,edx                     ; Inc pointer into the new data
            add         ebx,esi                     ; Inc pointer into ref data
            add         ecx,esi                     ; Inc pointer into ref2 data

            // Row 3
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            paddw       mm1, mm2                    ; Add word values together.
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            punpckhbw   mm4, mm6
            paddw       mm3, mm4                    ; Add word values together.
            psrlw       mm3, 1                      ; Devide by two (shift right 1)

            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpcklbw   mm1, mm6
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
            add         eax,edx                     ; Inc pointer into the new data
            add         ebx,esi                     ; Inc pointer into ref data
            add         ecx,esi                     ; Inc pointer into ref2 data

            // Row 4
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            paddw       mm1, mm2                    ; Add word values together.
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            punpckhbw   mm4, mm6
            paddw       mm3, mm4                    ; Add word values together.
            psrlw       mm3, 1                      ; Devide by two (shift right 1)

            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpcklbw   mm1, mm6
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
            add         eax,edx                     ; Inc pointer into the new data
            add         ebx,esi                     ; Inc pointer into ref data
            add         ecx,esi                     ; Inc pointer into ref2 data

            // Row 5
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            paddw       mm1, mm2                    ; Add word values together.
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            punpckhbw   mm4, mm6
            paddw       mm3, mm4                    ; Add word values together.
            psrlw       mm3, 1                      ; Devide by two (shift right 1)

            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpcklbw   mm1, mm6
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
            add         eax,edx                     ; Inc pointer into the new data
            add         ebx,esi                     ; Inc pointer into ref data
            add         ecx,esi                     ; Inc pointer into ref2 data

            // Row 6
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            paddw       mm1, mm2                    ; Add word values together.
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            punpckhbw   mm4, mm6
            paddw       mm3, mm4                    ; Add word values together.
            psrlw       mm3, 1                      ; Devide by two (shift right 1)

            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpcklbw   mm1, mm6
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
            add         eax,edx                     ; Inc pointer into the new data
            add         ebx,esi                     ; Inc pointer into ref data
            add         ecx,esi                     ; Inc pointer into ref2 data

            // Row 7
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            paddw       mm1, mm2                    ; Add word values together.
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            punpckhbw   mm4, mm6
            paddw       mm3, mm4                    ; Add word values together.
            psrlw       mm3, 1                      ; Devide by two (shift right 1)

            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpcklbw   mm1, mm6
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory

            add         eax,edx                     ; Inc pointer into the new data
            add         ebx,esi                     ; Inc pointer into ref data
            add         ecx,esi                     ; Inc pointer into ref2 data

            // Row 8
            movq        mm1, [ebx]                  ; Copy eight bytes from each of ref 1 and ref 2.
            movq        mm2, [ecx]
            movq        mm3, mm1                    ; Take copies.
            movq        mm4, mm2

            punpcklbw   mm1, mm6                    ; unpack low four bytes to higher precision
            punpcklbw   mm2, mm6
            paddw       mm1, mm2                    ; Add word values together.
            psrlw       mm1, 1                      ; Devide by two (shift right 1)
            punpckhbw   mm3, mm6                    ; unpack high four bytes to higher precision
            punpckhbw   mm4, mm6
            paddw       mm3, mm4                    ; Add word values together.
            psrlw       mm3, 1                      ; Devide by two (shift right 1)

            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            packuswb    mm1, mm3                    ; Repack to give 1/2 pixel averaged reference data
            movq        mm2, mm0                    ; Take copies
            movq        mm3, mm1                    ; Take copies

            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpcklbw   mm1, mm6
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            punpckhbw   mm3, mm6
            psubsw      mm0, mm1                    ; A-B (low order) to MM0
            psubsw      mm2, mm3                    ; A-B (high order) to MM2

            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5

            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            movq        DWORD PTR [MmxXSum], mm5    ; copy back accumulated results into normal memory
            movq        DWORD PTR [MmxXXSum], mm7   ; copy back accumulated results into normal memory
        }

        // Now accumulate the final results.
        XSum  = MmxXSum[0] + MmxXSum[1] + MmxXSum[2] + MmxXSum[3];
        XXSum = MmxXXSum[0] + MmxXXSum[1];
    }

    // Compute and return population variance as mis-match metric.
    return ( ((XXSum << 6) - XSum*XSum ) );
}

/****************************************************************************
 *
 *  ROUTINE       : MmxGetIntraError
 *
 *  INPUTS        : UINT8 *DataPtr       : Pointer to input block.
 *                  INT32  PixelsPerLine : Length of line for input block.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Block variance.
 *
 *  FUNCTION      : Calculates a variance score for the block.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
UINT32 MmxGetIntraError ( UINT8 *DataPtr, INT32 PixelsPerLine )
{
    UINT8   *DiffPtr;
    UINT32  XSum  = 0;
    UINT32  XXSum = 0;

    // Loop expanded out for speed.
    DiffPtr = DataPtr;

    __asm
    {
            pxor        mm5, mm5                    ; Blank mmx6
            pxor        mm6, mm6                    ; Blank mmx7
            pxor        mm7, mm7                    ; Blank mmx7

            mov         eax,dword ptr [DiffPtr] ; Load base addresses
            mov         ecx,dword ptr [PixelsPerLine]

            // Row 1
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5
            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         eax,ecx                     ; Inc pointer into the new data
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7


            // Row 2
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5
            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         eax,ecx                     ; Inc pointer into ref data
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            // Row 3
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5
            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         eax,ecx                     ; Inc pointer into ref data
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7


            // Row 4
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5
            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         eax,ecx                     ; Inc pointer into ref data
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            // Row 5
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5
            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         eax,ecx                     ; Inc pointer into ref data
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            // Row 6
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5
            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         eax,ecx                     ; Inc pointer into ref data
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            // Row 7
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5
            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         eax,ecx                     ; Inc pointer into ref data
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            // Row 8
            movq        mm0, [eax]                  ; Copy eight bytes to mm0
            movq        mm2, mm0                    ; Take copies
            punpcklbw   mm0, mm6                    ; unpack to higher precision
            punpckhbw   mm2, mm6                    ; unpack to higher precision
            paddw       mm5, mm0                    ; accumulate differences in mm5
            paddw       mm5, mm2                    ; accumulate differences in mm5
            pmaddwd     mm0, mm0                    ; square and accumulate
            pmaddwd     mm2, mm2                    ; square and accumulate
            add         eax,ecx                     ; Inc pointer into ref data
            paddd       mm7, mm0                    ; accumulate in mm7
            paddd       mm7, mm2                    ; accumulate in mm7

            movq        mm4, mm5                    ;
            punpcklwd   mm5, mm6
            punpckhwd   mm4, mm6
            movq        mm0, mm7
            paddw       mm5, mm4

            punpckhdq   mm0, mm6
            punpckldq   mm7, mm6
            movq        mm4, mm5
            paddd       mm0, mm7
            punpckhdq   mm4, mm6
            punpckldq   mm5, mm6
            movd        DWORD PTR [XXSum], mm0
            paddw       mm4, mm5
            movd        DWORD ptr [XSum], mm4
    }

    // Compute population variance as mis-match metric.
    return ( (XXSum<<6) - XSum*XSum );
}

/****************************************************************************
 *
 *  ROUTINE       : MmxGetMBFrameVertVar
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Vertical variance for frame.
 *
 *  FUNCTION      : Computes the vertical variance for a macroblock based
 *                  upon the sum of the local 2 pixel variances within
 *                  the entire frame.
 *
 *  SPECIAL NOTES : The difference between the last two rows in a MB
 *                  are not accounted for!
 *
 ****************************************************************************/
UINT32 MmxGetMBFrameVertVar ( CP_INSTANCE *cpi )
{
    UINT32 FrameError;
    INT32 Stride  = cpi->pb.Configuration.VideoFrameWidth;
//    UINT8 *SrcPtr = &cpi->yuv1ptr[cpi->pb.mbi.Source];
//sjlhack
    UINT8 *SrcPtr = &cpi->yuv1ptr[cpi->pb.mbi.blockDxInfo[0].Source];

    __asm
    {
        mov         ecx,    [Stride]
        mov         eax,    DWORD PTR [SrcPtr]
        
        pxor        mm7,    mm7                     ; clear mm7
        pxor        mm6,    mm6                     ; clear mm6
        
        mov         edx,    7                       ; 

MmxGetMBFrameVertVarLoop:

        movq        mm1,    [eax]                   ; 00 01 02 03 04 05 06 07
        movq        mm0,    [eax+ecx]               ; 10 11 12 13 14 15 16 17
        
        movq        mm3,    mm0                     ; copy of 00 01 02 03 04 05 06 07
        punpcklbw   mm0,    mm7                     ; xx 00 xx 01 xx 02 xx 03

        punpckhbw   mm3,    mm7                     ; xx 04 xx 05 xx 06 xx 07
        movq        mm2,    [eax+ecx*2]             ; 20 21 22 23 24 25 26 27

        movq        mm4,    mm1                     ; 10 11 12 13 14 15 16 17
        punpcklbw   mm1,    mm7                     ; xx 10 xx 11 xx 12 xx 13
        
        punpckhbw   mm4,    mm7                     ; xx 14 xx 15 xx 16 xx 17
        movq        mm5,    mm2                     ; 20 21 22 23 24 25 26 27

        punpcklbw   mm2,    mm7                     ; xx 20 xx 21 xx 22 xx 23
        psubw       mm1,    mm0                     ; difference between 0, 1  low four
        
        pmaddwd     mm1,    mm1                     ; SD between 0, 1  low four
        psubw       mm4,    mm3                     ; difference bwtween 0, 1   high four
        
        pmaddwd     mm4,    mm4                     ; SD between 0, 1  high foru
        punpckhbw   mm5,    mm7                     ; xx 24 xx 25 xx 26 xx 27

        psubw       mm2,    mm0                     ; difference between 0, 2   low four
        pmaddwd     mm2,    mm2                     ; sd between 0, 2   low four
        
        psubw       mm5,    mm3                     ; difference between 0, 2   high four
        pmaddwd     mm5,    mm5                     ; sd between 0, 2   high four

        paddd       mm1,    mm4                     ;
        paddd       mm2,    mm5                     ; 

        paddd       mm6,    mm1                     ;
        paddd       mm6,    mm2                     ; accumlated in mm6

        // done with the low eight

        movq        mm1,    8[eax]                   ; 00 01 02 03 04 05 06 07
        movq        mm0,    8[eax+ecx]               ; 10 11 12 13 14 15 16 17
        
        movq        mm3,    mm0                     ; copy of 00 01 02 03 04 05 06 07
        punpcklbw   mm0,    mm7                     ; xx 00 xx 01 xx 02 xx 03

        punpckhbw   mm3,    mm7                     ; xx 04 xx 05 xx 06 xx 07
        movq        mm2,    8[eax+ecx*2]             ; 20 21 22 23 24 25 26 27

        movq        mm4,    mm1                     ; 10 11 12 13 14 15 16 17
        punpcklbw   mm1,    mm7                     ; xx 10 xx 11 xx 12 xx 13
        
        punpckhbw   mm4,    mm7                     ; xx 14 xx 15 xx 16 xx 17
        movq        mm5,    mm2                     ; 20 21 22 23 24 25 26 27

        punpcklbw   mm2,    mm7                     ; xx 20 xx 21 xx 22 xx 23
        psubw       mm1,    mm0                     ; difference between 0, 1  low four
        
        pmaddwd     mm1,    mm1                     ; SD between 0, 1  low four
        psubw       mm4,    mm3                     ; difference bwtween 0, 1   high four
        
        pmaddwd     mm4,    mm4                     ; SD between 0, 1  high foru
        punpckhbw   mm5,    mm7                     ; xx 24 xx 25 xx 26 xx 27

        psubw       mm2,    mm0                     ; difference between 0, 2   low four
        pmaddwd     mm2,    mm2                     ; sd between 0, 2   low four
        
        psubw       mm5,    mm3                     ; difference between 0, 2   high four
        pmaddwd     mm5,    mm5                     ; sd between 0, 2   high four

        paddd       mm1,    mm4                     ;
        paddd       mm2,    mm5                     ; 

        paddd       mm6,    mm1                     ;
        paddd       mm6,    mm2                     ; accumlated in mm6

        lea         eax,    [eax + ecx *2]          ; skip one line
        sub         edx,    1

        jnz         MmxGetMBFrameVertVarLoop
       
        movq        mm0,    mm6
        psrlq       mm0,    32

        paddd       mm0,    mm6
        movd        [FrameError], mm0
    }

    return FrameError;
}

/****************************************************************************
 *
 *  ROUTINE       : MmxGetMBFieldVertVar
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Vertical variance for fields within a frame.
 *
 *  FUNCTION      : Computes the vertical variance for a macroblock based
 *                  upon the sum of the local 2 pixel variances within
 *                  the two fields of a frame.
 *
 *  SPECIAL NOTES : The difference between the last two rows in a MB
 *                  are not accounted for!
 *
 ****************************************************************************/
UINT32 MmxGetMBFieldVertVar ( CP_INSTANCE *cpi )
{
    UINT32 FieldError;
    INT32 Stride  = cpi->pb.Configuration.VideoFrameWidth;
//    UINT8 *SrcPtr = &cpi->yuv1ptr[cpi->pb.mbi.Source];
//sjlhack
    UINT8 *SrcPtr = &cpi->yuv1ptr[cpi->pb.mbi.blockDxInfo[0].Source];

    __asm
    {
        mov         ecx,    [Stride]
        mov         eax,    DWORD PTR [SrcPtr]
        
        pxor        mm7,    mm7                     ; clear mm7
        pxor        mm6,    mm6                     ; clear mm6
        
        mov         edx,    7                       ; 

MmxGetMBFieldVertVarLoop:

        movq        mm1,    [eax]                   ; 00 01 02 03 04 05 06 07
        movq        mm0,    [eax+ecx*2]             ; 10 11 12 13 14 15 16 17

        movq        mm2,    mm0                     ; 00 01 02 03 04 05 06 07
        punpcklbw   mm0,    mm7                     ; xx 00 xx 01 xx 02 xx 03

        movq        mm3,    mm1                     ; 10 11 12 13 14 15 16 17
        punpckhbw   mm2,    mm7                     ; xx 04 xx 05 xx 06 xx 07

        punpcklbw   mm1,    mm7                     ; xx 10 xx 11 xx 12 xx 13
        punpckhbw   mm3,    mm7                     ; xx 14 xx 15 xx 16 xx 17

        psubw       mm0,    mm1                     ; diff between 0    1 low four
        pmaddwd     mm0,    mm0                     ; SD   between 0    1 low four
        
        psubw       mm2,    mm3                     ; diff between 0    1 high four
        pmaddwd     mm2,    mm2                     ; SD between   0    1 high four

        paddd       mm0,    mm2
        paddd       mm6,    mm0

        movq        mm1,    8[eax]                   ; 00 01 02 03 04 05 06 07
        movq        mm0,    8[eax+ecx*2]             ; 10 11 12 13 14 15 16 17

        movq        mm2,    mm0                     ; 00 01 02 03 04 05 06 07
        punpcklbw   mm0,    mm7                     ; xx 00 xx 01 xx 02 xx 03

        movq        mm3,    mm1                     ; 10 11 12 13 14 15 16 17
        punpckhbw   mm2,    mm7                     ; xx 04 xx 05 xx 06 xx 07

        punpcklbw   mm1,    mm7                     ; xx 10 xx 11 xx 12 xx 13
        punpckhbw   mm3,    mm7                     ; xx 14 xx 15 xx 16 xx 17

        psubw       mm0,    mm1                     ; diff between 0    1 low four
        pmaddwd     mm0,    mm0                     ; SD   between 0    1 low four
        
        psubw       mm2,    mm3                     ; diff between 0    1 high four
        pmaddwd     mm2,    mm2                     ; SD between   0    1 high four

        paddd       mm0,    mm2
        paddd       mm6,    mm0

        lea         eax,    [eax+ecx]

        movq        mm1,    [eax]                   ; 00 01 02 03 04 05 06 07
        movq        mm0,    [eax+ecx*2]             ; 10 11 12 13 14 15 16 17

        movq        mm2,    mm0                     ; 00 01 02 03 04 05 06 07
        punpcklbw   mm0,    mm7                     ; xx 00 xx 01 xx 02 xx 03

        movq        mm3,    mm1                     ; 10 11 12 13 14 15 16 17
        punpckhbw   mm2,    mm7                     ; xx 04 xx 05 xx 06 xx 07

        punpcklbw   mm1,    mm7                     ; xx 10 xx 11 xx 12 xx 13
        punpckhbw   mm3,    mm7                     ; xx 14 xx 15 xx 16 xx 17

        psubw       mm0,    mm1                     ; diff between 0    1 low four
        pmaddwd     mm0,    mm0                     ; SD   between 0    1 low four
        
        psubw       mm2,    mm3                     ; diff between 0    1 high four
        pmaddwd     mm2,    mm2                     ; SD between   0    1 high four

        paddd       mm0,    mm2
        paddd       mm6,    mm0

        movq        mm1,    8[eax]                   ; 00 01 02 03 04 05 06 07
        movq        mm0,    8[eax+ecx*2]             ; 10 11 12 13 14 15 16 17

        movq        mm2,    mm0                     ; 00 01 02 03 04 05 06 07
        punpcklbw   mm0,    mm7                     ; xx 00 xx 01 xx 02 xx 03

        movq        mm3,    mm1                     ; 10 11 12 13 14 15 16 17
        punpckhbw   mm2,    mm7                     ; xx 04 xx 05 xx 06 xx 07

        punpcklbw   mm1,    mm7                     ; xx 10 xx 11 xx 12 xx 13
        punpckhbw   mm3,    mm7                     ; xx 14 xx 15 xx 16 xx 17

        psubw       mm0,    mm1                     ; diff between 0    1 low four
        pmaddwd     mm0,    mm0                     ; SD   between 0    1 low four
        
        psubw       mm2,    mm3                     ; diff between 0    1 high four
        pmaddwd     mm2,    mm2                     ; SD between   0    1 high four

        paddd       mm0,    mm2
        paddd       mm6,    mm0

        lea         eax,    [eax + ecx ]            ; skip one line
        sub         edx,    1

        jnz         MmxGetMBFieldVertVarLoop
       
        movq        mm0,    mm6
        psrlq       mm0,    32

        paddd       mm0,    mm6
        movd        [FieldError], mm0
    }

    return FieldError;

}

/****************************************************************************
 *
 *  ROUTINE       : FilterBlock2dBil_SAD_mmx
 *
 *  INPUTS        : UINT8 *SrcPtr           : Pointer to input block.
 *                  INT32 SrcStride         : Stride for input block.
 *                  UINT8 *RefPtr           : Pointer to reference block.
 *                  UINT32 SrcPixelsPerLine : Stride for reference block.
 *                  INT16 *HFilter          : Pointer to horizontal filter taps.
 *                  INT16 *VFilter          : Pointer to vertical filter taps.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: SAD error.
 *
 *  FUNCTION      : Produces a filtered fractional block in 2-D
 *  				using bilinear filters and calculate the SAD.
 *
 *  SPECIAL NOTES : The difference between the last two rows in a MB
 *                  are not accounted for!
 *
 ****************************************************************************/
_inline UINT32 FilterBlock2dBil_SAD_mmx 
(
    UINT8 *SrcPtr, 
    INT32 SrcStride, 
    UINT8 *RefPtr, 
    UINT32 SrcPixelsPerLine, 
    INT16 *HFilter, 
    INT16 *VFilter 
)
{

    UINT32 Error=0;
    __asm
    {
        mov         eax,        HFilter             ; 
        mov         edi,        SrcPtr              ; 

        mov         esi,        RefPtr              ;
        mov         ecx,        8            ;

        mov         edx,        SrcPixelsPerLine    ;
               
        movq        mm1,        [eax]               ;
        movq        mm2,        [eax+16]            ;
        
        mov         eax,        VFilter             ;       
        pxor         mm0,        mm0                ;

        // get the first horizontal line done       ;
        movq        mm3,        [esi]               ; xx 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14
        movq        mm4,        mm3                 ; make a copy of current line
        
        punpcklbw   mm3,        mm0                 ; xx 00 01 02 03 04 05 06
        punpckhbw   mm4,        mm0                 ;

        pmullw      mm3,        mm1                 ;
        pmullw      mm4,        mm1                 ;

        movq        mm5,        [esi+1]             ;
        movq        mm6,        mm5                 ;

        punpcklbw   mm5,        mm0                 ;
        punpckhbw   mm6,        mm0                 ;

        pmullw      mm5,        mm2                 ;
        pmullw      mm6,        mm2                 ;

        paddw       mm3,        mm5                 ;
        paddw       mm4,        mm6                 ;
        
        paddw       mm3,        rd                  ; xmm3 += round value
        psraw       mm3,        FILTER_SHIFT        ; xmm3 /= 128

        paddw       mm4,        rd                  ;
        psraw       mm4,        FILTER_SHIFT        ;
        
        movq        mm7,        mm3                 ;
        packuswb    mm7,        mm4                 ;


        add         esi,        edx                 ; next line
NextRow:
        movq        mm3,        [esi]               ; xx 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14
        movq        mm4,        mm3                 ; make a copy of current line
        
        punpcklbw   mm3,        mm0                 ; xx 00 01 02 03 04 05 06
        punpckhbw   mm4,        mm0                 ;

        pmullw      mm3,        mm1                 ;
        pmullw      mm4,        mm1                 ;

        movq        mm5,        [esi+1]             ;
        movq        mm6,        mm5                 ;

        punpcklbw   mm5,        mm0                 ;
        punpckhbw   mm6,        mm0                 ;

        pmullw      mm5,        mm2                 ;
        pmullw      mm6,        mm2                 ;

        paddw       mm3,        mm5                 ;
        paddw       mm4,        mm6                 ;
        
        movq        mm5,        mm7                 ;
        movq        mm6,        mm7                 ;                

        punpcklbw   mm5,        mm0                 ;
        punpckhbw   mm6,        mm0

        pmullw      mm5,        [eax]               ;
        pmullw      mm6,        [eax]               ;
        
        paddw       mm3,        rd                  ; xmm3 += round value
        psraw       mm3,        FILTER_SHIFT        ; xmm3 /= 128

        paddw       mm4,        rd                  ;
        psraw       mm4,        FILTER_SHIFT        ;
        
        movq        mm7,        mm3                 ;
        packuswb    mm7,        mm4                 ;    
        

        pmullw      mm3,        [eax+16]            ;
        pmullw      mm4,        [eax+16]            ;

        paddw       mm3,        mm5                 ;
        paddw       mm4,        mm6                 ;
        
        
        paddw       mm3,        rd                  ; xmm3 += round value
        psraw       mm3,        FILTER_SHIFT        ; xmm3 /= 128

        paddw       mm4,        rd                  ;
        psraw       mm4,        FILTER_SHIFT        ;
               
        packuswb    mm3,        mm4                                         
        movq        mm4,        [edi]               ;
        
        psadbw      mm3,        mm4                 ;
        movd        mm4,        Error               ;

        paddd       mm3,        mm4                 ;
        movd        Error,      mm3                 ;        
        
        add         esi,        edx                 ; next line
        add         edi,        SrcStride           ;                   ; 

        dec         ecx                             ;
        jne         NextRow                         
    }
    return Error;
}

/****************************************************************************
 *
 *  ROUTINE       : FilterBlock1d_vb8_SAD_mmx
 *
 *  INPUTS        : UINT8 *SrcPtr           : Pointer to input block.
 *                  INT32 SrcStride         : Stride for input block.
 *                  UINT8 *RefPtr           : Pointer to reference block.
 *                  UINT32 PixelsPerLine    : Stride for reference block.
 *                  UINT32 PixelStep        : Offset to move to next pixel in input.
 *                  INT16 *Filter           : Pointer to filter taps.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: SAD error.
 *
 *  FUNCTION      : Applies 1-D vertical bi-linear filter to input block.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
_inline UINT32 FilterBlock1d_vb8_SAD_mmx
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
        movq        mm1,       [edi]                ; mm3 *= kernel 0 modifiers.
        movq        mm2,       [edi + 16]            ; mm3 *= kernel 0 modifiers.

        mov         edi,        SrcPtr
		mov			esi,        RefPtr
        
        mov         ecx,        8                   ;

        mov         edx,        SrcStride
        mov         eax,        PixelsPerLine;
        
        pxor        mm7,        mm7
		pxor		mm0,       mm0                  ; mm0 = 00000000

nextrow:
        movq		mm3,        [esi]               ; mm3 = p0..p7
        movq        mm4,        mm3                 ; mm4 = p0..p7
        
        punpcklbw   mm3,        mm0                 ; mm3 = p0..p3
        punpckhbw   mm4,        mm0                 ; mm4 = p4..p7
        
        pmullw      mm3,        mm1                 ; mm3 *= kernel 0 modifiers.
        pmullw      mm4,        mm1                 ; mm4 *= kernel 0 modifiers.

        movq        mm5,        [esi + eax]         ; 
        movq        mm6,        mm5                 ;

        punpcklbw   mm5,        mm0                 ;
        punpckhbw   mm6,        mm0                 ;

        pmullw      mm5,        mm2                 ;
        pmullw      mm6,        mm2                 ;

        paddw       mm3,        mm5                 ;
        paddw       mm4,        mm6                 ;

        paddw       mm3,        rd                  ; xmm3 += round value
        psraw       mm3,        FILTER_SHIFT        ; xmm3 /= 128

        paddw       mm4,        rd                  ;
        psraw       mm4,        FILTER_SHIFT        ;


        packuswb    mm3,        mm4                 ; pack and unpack to saturate
        movq        mm5,        [edi]               ;
        
        psadbw      mm3,        mm5                 ;
        paddd       mm7,        mm3
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
 *  ROUTINE       : FilterBlock1d_hb8_SAD_mmx
 *
 *  INPUTS        : UINT8 *SrcPtr           : Pointer to input block.
 *                  INT32 SrcStride         : Stride for input block.
 *                  UINT8 *RefPtr           : Pointer to reference block.
 *                  UINT32 SrcPixelsPerLine : Stride for reference block.
 *                  UINT32 PixelStep        : Offset to move to next pixel in input.
 *                  INT16 *Filter           : Pointer to filter taps.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: SAD error.
 *
 *  FUNCTION      : Applies 1-D horizontal bi-linear filter to input block.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
_inline UINT32 FilterBlock1d_hb8_SAD_mmx
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
        movq        mm1,        [edi]               ; xmm3 *= kernel 0 modifiers.
        movq        mm2,        [edi + 16]          ; xmm3 *= kernel 0 modifiers.

        mov         edi,        SrcPtr
		mov			esi,        RefPtr
		
        pxor		mm0,        mm0                 ; mm0 = 00000000
        pxor        mm7,        mm7                 ; mm7 = 0
        
        mov         ecx,        8                   ;

        mov         edx,        SrcStride
        mov         eax,        SrcPixelsPerLine;

nextrow:
        movq		mm3,        [esi]               ; mm3 = p-1..p6
        movq        mm4,        mm3                 ; make a copy
            
        punpcklbw   mm3,        mm0                 ;                      
        pmullw      mm3,        mm1                 ;

        movq        mm5,        [esi+1]             ;mm5 = p0 ..... p7
        punpckhbw   mm4,        mm0            

        pmullw      mm4,        mm1                 ;        
        movq        mm6,        mm5                 ;

        punpcklbw   mm5,        mm0                 ; mm5 = p0..p7
        pmullw      mm5,        mm2                 ;

        punpckhbw   mm6,        mm0                 ;
        pmullw      mm6,        mm2                 ;

        paddw       mm3,        mm5                 ;            
        paddw       mm4,        mm6                 ;

        paddw       mm3,        rd                  ; xmm3 += round value
        psraw       mm3,        FILTER_SHIFT        ; xmm3 /= 128

        paddw       mm4,        rd                  ;
        psraw       mm4,        FILTER_SHIFT        ;
        
        packuswb    mm3,        mm4                 ; pack and unpack to saturate
        
        movq        mm5,        [edi]               ; read src  
        psadbw      mm3,        mm5                 ;
        paddd       mm7,        mm3

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
 *  ROUTINE       : FiltBlockBilGetSad_mmx
 *
 *  INPUTS        : UINT8 *SrcPtr        : Pointer to input block.
 *                  INT32 SrcStride      : Stride for input block.
 *                  UINT8 *ReconPtr1     : Pointer to first reference block.
 *                  UINT8 *ReconPtr2     : Pointer to second reference block.
 *                  UINT32 PixelsPerLine : Stride for reference block.
 *                  INT32 ModX           : Fractional part of x-component of MV.
 *                  INT32 ModY           : Fractional part of x-component of MV.
 *                  UINT32 BestSoFar     : Best error found so far.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: SAD error.
 *
 *  FUNCTION      : Applies 2-D bi-linear filter to get prediction block
 *                  and computes SAD for prediction error.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
UINT32 FiltBlockBilGetSad_mmx
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
	if ( diff<0 ) 
	{								// swap pointers so ReconPtr1 smaller
		UINT8 *temp = ReconPtr1;
		ReconPtr1   = ReconPtr2;
		ReconPtr2   = temp;
		diff = (int)(ReconPtr2-ReconPtr1);
	}

	if ( diff==1 )
		Error = FilterBlock1d_hb8_SAD_mmx ( SrcPtr, SrcStride, ReconPtr1, PixelsPerLine, 1, BilinearFilters_mmx[ModX] );
	else if (diff == (int)(PixelsPerLine) )				// Fractional pixel in vertical only
		Error = FilterBlock1d_vb8_SAD_mmx ( SrcPtr, SrcStride, ReconPtr1, PixelsPerLine, PixelsPerLine, BilinearFilters_mmx[ModY] );
	else if(diff == (int)(PixelsPerLine - 1))			// ReconPtr1 is Top right
        Error = FilterBlock2dBil_SAD_mmx ( SrcPtr, SrcStride, ReconPtr1-1, PixelsPerLine, BilinearFilters_mmx[ModX], BilinearFilters_mmx[ModY] );        
	else if(diff == (int)(PixelsPerLine + 1) )			// ReconPtr1 is Top left
        Error = FilterBlock2dBil_SAD_mmx ( SrcPtr, SrcStride, ReconPtr1, PixelsPerLine, BilinearFilters_mmx[ModX], BilinearFilters_mmx[ModY] );		
    return Error;
}
