/**************************************************************************** 
 *
 *   Module Title :     DeInterlaceWmt.c
 *
 *   Description  :     DeInterlace Routines
 *
 ***************************************************************************/

/****************************************************************************
*  Header Files
****************************************************************************/
#include "postp.h"

/****************************************************************************
*  Module constants.
****************************************************************************/        

#if defined(_WIN32_WCE)
#pragma pack(16)
short four2s[] = { 2, 2, 2, 2 }; 
#pragma pack()
#else
__declspec(align(16)) short four2s[] = { 2, 2, 2, 2 }; 
#endif

/****************************************************************************
 * 
 *  ROUTINE       : WmtFastDeInterlace
 *
 *  INPUTS        : UINT8 *SrcPtr : Pointer to input frame.
 *                  UINT8 *DstPtr : Pointer to output frame.
 *                  INT32 Width   : Width of frame in pixels.
 *                  INT32 Height  : Height of frame in pixels.
 *                  INT32 Stride  : Stride of images.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a 3 tap filter vertically to remove interlacing
 *                  artifacts.
 *
 *  SPECIAL NOTES : This function use a three tap filter [1, 2, 1] to blur
 *                  veritically in an interlaced frame. This function assumes:
 *                  1) Buffers SrcPtr and DstPtr point to have the same geometery,
 *                  2) SrcPtr and DstPtr can _not_ be same.
 *
 ****************************************************************************/
void MmxFastDeInterlace
(
    UINT8 *SrcPtr,
    UINT8 *DstPtr,
    INT32 Width,
    INT32 Height,
    INT32 Stride
)
{
    INT32 i;  
    UINT8 *CurrentSrcPtr = SrcPtr;
    UINT8 *CurrentDstPtr = DstPtr;
    
    // Always copy the first line
    memcpy ( CurrentDstPtr, CurrentSrcPtr, Width );

    for ( i=1; i<Height-1; i++ )
    {
        CurrentDstPtr += Stride;

        __asm
        {
            mov         esi,        [CurrentSrcPtr]
            mov         edi,        [CurrentDstPtr]
            
            xor         ecx,        ecx 
            mov         edx,        [Stride]

            lea         eax,        [esi + edx]
            lea         edx,        [eax + edx]

            mov         ebx,        [Width]
            pxor        mm7,        mm7

MmxDeInterlaceLoop:            
            movq        mm0,       QWORD ptr [esi + ecx]                // line -1
            movq        mm1,       QWORD ptr [eax + ecx]                // line  0

            movq        mm3,        mm0                                 // line -1
            punpcklbw   mm0,        mm7                                 // line -1 low
            
            movq        mm2,        QWORD ptr [edx + ecx]               // line 1
            punpckhbw   mm3,        mm7                                 // line -1 high


            movq        mm4,        mm1                                 // line 0 
            punpcklbw   mm1,        mm7                                 // line 0 low

            paddw       mm0,        four2s                              // line -1 low + 2s
            paddw       mm3,        four2s                              // line -1 high + 2s

            punpckhbw   mm4,        mm7                                 // line 0 high
            psllw       mm1,        1                                   // line 0 * 2

            psllw       mm4,        1                                   // line 0 * 2
            movq        mm5,        mm2                                 // line 1

            punpcklbw   mm2,        mm7                                 // line 1 low
            paddw       mm0,        mm1                                 // line -1 + line 0 * 2

            paddw       mm3,        mm4                                 // line -1 + line 0 * 2
            punpckhbw   mm5,        mm7                                 // line 1 high
            
            paddw       mm0,        mm2                                 // -1 + 0 * 2 + 1
            paddw       mm3,        mm5                                 // -1 + 0 * 2 + 1

            psraw       mm0,        2                                   // >> 2
            psraw       mm3,        2                                   // >> 2
            
            packuswb    mm0,        mm3

            movq        QWORD ptr [edi+ecx],   mm0
            add         ecx,        8        

            cmp         ecx,        ebx
            jl          MmxDeInterlaceLoop
            
        }
        CurrentSrcPtr += Stride;
        /*
        for(j=0;j<Width;j++)
        {
            x0 = PrevSrcPtr[j];
            x1 = (CurrentSrcPtr[j]<<1);
            x2 = NextSrcPtr[j];
            CurrentDstPtr[j] = (UINT8)( (x0 + x1 + x2)>>2 );
        }
        */
    }
    
    // copy the last line
    CurrentSrcPtr += Stride;
    CurrentDstPtr += Stride;
    memcpy ( CurrentDstPtr, CurrentSrcPtr, Width );
}
