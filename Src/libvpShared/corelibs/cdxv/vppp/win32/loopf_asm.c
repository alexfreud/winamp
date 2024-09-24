/****************************************************************************
*
*   Module Title :     loopf_asm.c
*
*   Description  :     Optimized version of the loop filter.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Frames
****************************************************************************/
#include <stdio.h>
#include <memory.h>
#include "postp.h"

/****************************************************************************
*  Macros
****************************************************************************/
#pragma warning (disable:4799)
#pragma warning (disable:4731)

#define LIMIT_OFFSET        0
#define FOURONES_OFFSET     8
#define LFABS_OFFSET        16
#define TRANS_OFFSET        24

/****************************************************************************
 * 
 *  ROUTINE       : SetupBoundingValueArray_ForMMX
 *
 *  INPUTS        : POSTPROC_INSTANCE *pbi : Pointer to post-processing instance.
 *                  INT32 FLimit           : Filter limiting value.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : INT32*: Pointer to bounding value array.
 *
 *  FUNCTION      : Sets up bounding value array used in filtering operations.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
INT32 *SetupBoundingValueArray_ForMMX ( POSTPROC_INSTANCE *pbi, INT32 FLimit )
{
    INT32 *BoundingValuePtr;

    /* Since the FiltBoundingValue array is currently only used in the generic */
    /* version, we are going to reuse this memory for our own purposes.        */
    /* 2 longs for limit, 2 longs for _4ONES, 2 longs for LFABS_MMX, and       */
    /* 8 longs for temp work storage                                           */
   BoundingValuePtr = (INT32 *)((UINT32)(&pbi->FiltBoundingValue[256]) & 0xffffffe0);    

    // expand for mmx code
    BoundingValuePtr[0] = BoundingValuePtr[1] = FLimit * 0x00010001;
    BoundingValuePtr[2] = BoundingValuePtr[3] = 0x00010001;
    BoundingValuePtr[4] = BoundingValuePtr[5] = 0x00040004;

    return BoundingValuePtr;
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterHoriz_MMX
 *
 *  INPUTS        : POSTPROC_INSTANCE *pbi  : Pointer to post-processing instance.
 *                  UINT8 *PixelPtr         : Pointer to input frame.
 *                  INT32 LineLength        : Length of line in input frame.
 *                  INT32 *BoundingValuePtr : Pointer to bouning value array.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a loop filter to the vertical edge (i.e. horizontally).
 *
 *  SPECIAL NOTES : This version attempts to fix the DC_misalign stalls.
 *
 ****************************************************************************/
void FilterHoriz_MMX
(
    POSTPROC_INSTANCE *pbi, 
    UINT8 *PixelPtr, 
    INT32 LineLength, 
    INT32 *BoundingValuePtr
)
{
	(void) pbi;

    /* A somewhat optimized MMX version of the left edge filter. */
    __asm 
    {
        mov         eax,[BoundingValuePtr]
        mov         edx,[LineLength]            //stride

        mov         ebx,[PixelPtr]
        mov         ecx,[LineLength]            //stride

        movd        mm0,[ebx + -2]              //xx xx xx xx 01 00 xx xx
    ;-

        movd        mm4,[ebx + 2]               //xx xx xx xx xx xx 03 02
        psrld       mm0,16                      //xx xx xx xx 00 00 01 00

        movd        mm1,[ebx + ecx + -2]        //xx xx xx xx 11 10 xx xx
        punpcklwd   mm0,mm4                     //xx xx xx xx 03 02 01 00

        movd        mm4,[ebx + ecx + 2]         //xx xx xx xx xx xx 13 12
        psrld       mm1,16                      //xx xx xx xx 00 00 11 10

        punpcklwd   mm1,mm4                     //xx xx xx xx 13 12 11 10
        lea         edx,[edx + edx*2]           //stride * 3

        movd        mm2,[ebx + ecx*2 + -2]      //xx xx xx xx 21 20 xx xx
        punpcklbw   mm0,mm1                     //13 03 12 02 11 01 10 00

        movd        mm4,[ebx + ecx*2 + 2]       //xx xx xx xx xx xx 23 22
        psrld       mm2,16                      //xx xx xx xx 00 00 21 20

        movd        mm1,[ebx + edx + -2]        //xx xx xx xx 31 30 xx xx
        punpcklwd   mm2,mm4                     //xx xx xx xx 23 22 21 20

        movd        mm4,[ebx + edx + 2]         //xx xx xx xx xx xx 33 32
        psrld       mm1,16                      //xx xx xx xx 00 00 31 30

        punpcklwd   mm1,mm4                     //xx xx xx xx 33 32 31 30
        pxor        mm4,mm4

        punpcklbw   mm2,mm1                     //33 23 32 22 31 21 30 20
        movq        mm1,mm0

        punpcklwd   mm0,mm2                     //31 21 11 01 30 20 10 00
        lea         ebx,[ebx + ecx*4]           //base + (stride * 4)

        punpckhwd   mm1,mm2                     //33 23 13 03 32 22 12 02
        movq        mm6,mm0                     //xx xx xx xx 30 20 10 00

        movq        [eax + TRANS_OFFSET + 0],mm0
        movq        mm2,mm1

        movq        [eax + TRANS_OFFSET + 8],mm1
        psrlq       mm0,32                      //xx xx xx xx 31 21 11 01

;-----------
        movd        mm7,[ebx + -2]              //xx xx xx xx 41 40 xx xx
        punpcklbw   mm1,mm4                     //convert to words

        movd        mm4,[ebx + 2]               //xx xx xx xx xx xx 43 42
        psrld       mm7,16                      //xx xx xx xx 00 00 41 40

        movd        mm5,[ebx + ecx + -2]        //xx xx xx xx 51 50 xx xx
        punpcklwd   mm7,mm4                     //xx xx xx xx 43 42 41 40

        movd        mm4,[ebx + ecx + 2]         //xx xx xx xx xx xx 53 52
        psrld       mm5,16

        punpcklwd   mm5,mm4
        pxor        mm4,mm4

        punpcklbw   mm0,mm4
;-

        psrlq       mm2,32                      //xx xx xx xx 33 23 13 03
        psubw       mm1,mm0                     //x = p[0] - p[ms]

        punpcklbw   mm7,mm5                     //53 43 52 42 51 41 50 40
        movq        mm3,mm1
;-------------------
        punpcklbw   mm6,mm4
        paddw       mm3,mm1

        punpcklbw   mm2,mm4
        paddw       mm1,mm3

        paddw       mm1,[eax + LFABS_OFFSET]    //x += LoopFilterAdjustBeforeShift
        psubw       mm6,mm2

        movd        mm2,[ebx + ecx*2 + -2]      //xx xx xx xx 61 60 xx xx
        paddw       mm6,mm1

        movd        mm4,[ebx + ecx*2 + 2]       //xx xx xx xx xx xx 63 62
        psrld       mm2,16

        movd        mm5,[ebx + edx + -2]        //xx xx xx xx 71 70 xx xx
        punpcklwd   mm2,mm4                     //xx xx xx xx 63 62 61 60

        movd        mm4,[ebx + edx + 2]         //xx xx xx xx xx xx 73 72
        psrld       mm5,16                      //xx xx xx xx 00 00 71 70

        mov         ebx,[PixelPtr]              //restore PixelPtr
        punpcklwd   mm5,mm4                     //xx xx xx xx 73 72 71 70

        psraw       mm6,3                       //values to be clipped
        pxor        mm4,mm4

        punpcklbw   mm2,mm5                     //73 63 72 62 71 61 70 60
        movq        mm5,mm7                     //53 43 52 42 51 41 50 40

        movq        mm1,mm6
        punpckhwd   mm5,mm2                     //73 63 53 43 72 62 52 42


        movq        [eax + TRANS_OFFSET + 24],mm5   //save for later
        punpcklwd   mm7,mm2                     //71 61 51 41 70 60 50 40

        movq        [eax + TRANS_OFFSET + 16],mm7   //save for later
        psraw       mm6,15

        movq        mm2,[eax + LIMIT_OFFSET]        //get the limit value
        movq        mm0,mm7                         //xx xx xx xx 70 60 50 41

        psrlq       mm7,32                          //xx xx xx xx 71 61 51 41
        pxor        mm1,mm6

        psubsw      mm1,mm6                         //abs(i)
        punpcklbw   mm5,mm4

        por         mm6,[eax + FOURONES_OFFSET]     //now have -1 or 1 
        movq        mm3,mm2

        punpcklbw   mm7,mm4
        psubw       mm3,mm1                         //limit - abs(i)

        movq        mm4,mm3
        psraw       mm3,15

        push        ebp                        
    ;-

        psubw       mm5,mm7                         //x = p[0] - p[ms]
        pxor        mm4,mm3

        psubsw      mm4,mm3                         //abs(limit - abs(i))
        pxor        mm3,mm3

        movq        mm1,[eax + TRANS_OFFSET + 28]  //xx xx xx xx 73 63 53 43
        psubusw     mm2,mm4                     //limit - abs(limit - abs(i))

        punpcklbw   mm0,mm3
        movq        mm7,mm5

        paddw       mm7,mm5
        pmullw      mm2,mm6                     //new y -- wait 3 cycles

        punpcklbw   mm1,mm3
        paddw       mm5,mm7

        paddw       mm5,[eax + LFABS_OFFSET]             //x += LoopFilterAdjustBeforeShift
        psubw       mm0,mm1

        paddw       mm0,mm5
        pxor        mm6,mm6     

        movd        mm7,[eax + TRANS_OFFSET + 8]  //xx xx xx xx 32 22 12 02
        psraw       mm0,3                       //values to be clipped

        movd        mm3,[eax + TRANS_OFFSET + 4]  //xx xx xx xx 31 21 11 01
        punpcklbw   mm7,mm6

        psubw       mm7,mm2                     //p[ms] + y
        punpcklbw   mm3,mm6

        paddw       mm3,mm2                     //p[0] - y
        packuswb    mm7,mm7                     //clamp[ p[ms] + y]

        packuswb    mm3,mm3                     //clamp[ p[0] - y]
        movq        mm1,mm0

        movq        mm2,[eax + LIMIT_OFFSET]                 //get the limit value
        psraw       mm0,15

        //values to write out
        punpcklbw   mm3,mm7                     //32 31 22 21 12 11 02 01                    
        movq        mm7,mm0                     //save sign

        movd        ebp,mm3                     //12 11 02 01
        pxor        mm1,mm0

        //xor bp,bp

        mov         WORD PTR[ebx + 1],bp                //02 01
        psubsw      mm1,mm0                     //abs(i)

        shr         ebp,16
        movq        mm5,mm2

        mov         WORD PTR[ebx + ecx + 1],bp
        psrlq       mm3,32                      //xx xx xx xx 32 31 22 21

        por         mm7,[eax + FOURONES_OFFSET]                //now have -1 or 1 
        psubw       mm5,mm1                     //limit - abs(i)

        movd        ebp,mm3                     //32 31 22 21
        movq        mm4,mm5

        mov         [ebx + ecx*2 + 1],bp
        psraw       mm5,15

        shr         ebp,16
        pxor        mm4,mm5

        mov         [ebx + edx + 1],bp
        psubsw      mm4,mm5                     //abs(limit - abs(i))

        movd        mm5,[eax + TRANS_OFFSET + 24]  //xx xx xx xx 72 62 52 42
        psubusw     mm2,mm4                     //limit - abs(limit - abs(i))

        pmullw      mm2,mm7                     //new y
        pxor        mm6,mm6

        movd        mm3,[eax + TRANS_OFFSET + 20]  //xx xx xx xx 71 61 51 41
        punpcklbw   mm5,mm6

        lea         ebx,[ebx + ecx*4]
        punpcklbw   mm3,mm6

        paddw       mm3,mm2                     //p[ms] + y
        psubw       mm5,mm2                     //p[0] - y

        packuswb    mm3,mm3                     //clamp[ p[ms] + y]
        pop         ebp
    ;-

//
//NOTE: optimize the following somehow
//
        packuswb    mm5,mm5                     //clamp[ p[0] - y]
    ;-
        punpcklbw   mm3,mm5                     //72 71 62 61 52 51 42 41
    ;-

        movd        eax,mm3                     //52 51 42 41
        psrlq       mm3,32                      //xx xx xx xx 72 71 62 61

        mov         [ebx + 1],ax
    ;-
        shr         eax,16
    ;-

        mov         [ebx + ecx + 1],ax
    ;-


        movd        eax,mm3
    ;-

        mov         [ebx + ecx*2 + 1],ax
    ;-

        shr         eax,16
    ;-

        mov         [ebx + edx + 1],ax
    ;-
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterVert_MMX
 *
 *  INPUTS        : POSTPROC_INSTANCE *pbi  : Pointer to post-processing instance.
 *                  UINT8 *PixelPtr         : Pointer to input frame.
 *                  INT32 LineLength        : Length of line in input frame.
 *                  INT32 *BoundingValuePtr : Pointer to bouning value array.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a loop filter to the horizontal edge (i.e. vertically).
 *
 *  SPECIAL NOTES : This version attempts to fix the DC_misalign stalls.
 *
 ****************************************************************************/
void FilterVert_MMX
(
    POSTPROC_INSTANCE *pbi, 
    UINT8 *PixelPtr, 
    INT32 LineLength, 
    INT32 *BoundingValuePtr
)
{
    INT32 ms = -LineLength;
	(void) pbi;

    /* A somewhat optimized MMX version of the top edge filter. */
    __asm 
    {
        mov         eax,[BoundingValuePtr]
    ;-

        mov         ebx,[PixelPtr]
        mov         ecx,[ms]                    //negative stride

        movd        mm1,[ebx + 0]               //p[0]   
        pxor        mm4,mm4

        movd        mm0,[ebx + ecx]             //get row above -- p[ms]
        punpcklbw   mm1,mm4                     //convert to words

        mov         edx,[LineLength]
        punpcklbw   mm0,mm4

        movd        mm6,[ebx + ecx*2]           //p[ms2]
        psubw       mm1,mm0                     //x = p[0] - p[ms]

        movq        mm2,[ebx + edx]             //p[stride]
        movq        mm3,mm1

        punpcklbw   mm6,mm4
        paddw       mm3,mm1

        punpcklbw   mm2,mm4
        paddw       mm1,mm3

        paddw       mm1,[eax + LFABS_OFFSET]             //x += LoopFilterAdjustBeforeShift
        psubw       mm6,mm2

        movq        mm2,[eax + LIMIT_OFFSET]                 //get the limit value
        paddw       mm6,mm1

        movd        mm5,[ebx + 4]               //p[0]   
        psraw       mm6,3                       //values to be clipped

        movq        mm1,mm6
        psraw       mm6,15

        movd        mm7,[ebx + ecx + 4]         //p[ms]
        pxor        mm1,mm6

        psubsw      mm1,mm6                     //abs(i)
        pxor        mm0,mm0

        punpcklbw   mm5,mm0
        movq        mm3,mm2

        por         mm6,[eax + FOURONES_OFFSET]                //now have -1 or 1 
        punpcklbw   mm7,mm0

        psubw       mm3,mm1                     //limit - abs(i)
        psubw       mm5,mm7                     //x = p[0] - p[ms]

        movq        mm4,mm3
        psraw       mm3,15

        movd        mm0,[ebx + ecx*2 + 4]       //p[ms2]
        pxor        mm4,mm3

        movd        mm1,[ebx + edx +4]          //p[stride]
        psubsw      mm4,mm3                     //abs(limit - abs(i))

        pxor        mm3,mm3
        psubusw     mm2,mm4                     //limit - abs(limit - abs(i))

        punpcklbw   mm0,mm3
        movq        mm7,mm5

        paddw       mm7,mm5
        pmullw      mm2,mm6                     //new y -- wait 3 cycles

        punpcklbw   mm1,mm3
        paddw       mm5,mm7

        paddw       mm5,[eax + LFABS_OFFSET]             //x += LoopFilterAdjustBeforeShift
        psubw       mm0,mm1

        paddw       mm0,mm5
        pxor        mm6,mm6     

        movd        mm7,[ebx + 0]               //p[0]   
        psraw       mm0,3                       //values to be clipped

        movd        mm3,[ebx + ecx]             //get row above -- p[ms]
        punpcklbw   mm7,mm6

        psubw       mm7,mm2                     //p[ms] + y
        punpcklbw   mm3,mm6

        paddw       mm3,mm2                     //p[0] - y
        packuswb    mm7,mm7                     //clamp[ p[ms] + y]

        packuswb    mm3,mm3                     //clamp[ p[0] - y]
        movq        mm1,mm0

        movd        [ebx + 0],mm7               //write p[0]
        psraw       mm0,15

        movq        mm7,mm0                     //save sign
        pxor        mm1,mm0

;
;
        movq        mm2,[eax + LIMIT_OFFSET]                 //get the limit value
;
;

        psubsw      mm1,mm0                     //abs(i)
        movq        mm5,mm2

        por         mm7,[eax + FOURONES_OFFSET]                //now have -1 or 1 
        psubw       mm5,mm1                     //limit - abs(i)

        movq        mm4,mm5
        psraw       mm5,15

        movd        [ebx + ecx],mm3             //write p[ms]
        pxor        mm4,mm5

        psubsw      mm4,mm5                     //abs(limit - abs(i))
        pxor        mm6,mm6

        movd        mm5,[ebx + 4]               //p[0]  
        psubusw     mm2,mm4                     //limit - abs(limit - abs(i))

        movd        mm3,[ebx + ecx + 4]         //p[ms]
        pmullw      mm2,mm7                     //new y

        punpcklbw   mm5,mm6
    ;-

        punpcklbw   mm3,mm6
    ;-

        paddw       mm3,mm2                     //p[ms] + y
        psubw       mm5,mm2                     //p[0] - y

        packuswb    mm3,mm3                     //clamp[ p[ms] + y]
    ;-

        packuswb    mm5,mm5                     //clamp[ p[0] - y]
    ;-

        movd        [ebx + ecx + 4],mm3         //write p[ms]
;

        movd        [ebx + 4],mm5               //write p[0]
    }
}

