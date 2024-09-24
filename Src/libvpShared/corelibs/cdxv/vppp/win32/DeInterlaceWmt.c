/**************************************************************************** 
 *
 *   Module Title :     DeInterlaceWmt.c
 *
 *   Description  :     DeInterlace
 *
 ***************************************************************************/

/****************************************************************************
*  Header Frames
****************************************************************************/
#include "postp.h"

/****************************************************************************
*  Module constants.
****************************************************************************/        
#if defined(_WIN32_WCE)
#pragma pack(16)
short Eight2s[] = { 2, 2, 2, 2, 2, 2, 2, 2 }; 
#pragma pack()
#else
__declspec(align(16)) short Eight2s[] = { 2, 2, 2, 2, 2, 2, 2, 2 }; 
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
void WmtFastDeInterlace
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
#if defined(_WIN32_WCE)
	return;
#else
    
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
            pxor        xmm7,       xmm7

WmtDeInterlaceLoop:            
            movq        xmm0,       QWORD ptr [esi + ecx]
            movq        xmm1,       QWORD ptr [eax + ecx]

            punpcklbw   xmm0,       xmm7
            movq        xmm2,       QWORD ptr [edx + ecx]

            punpcklbw   xmm1,       xmm7
            paddw       xmm0,       Eight2s

            psllw       xmm1,       1
            punpcklbw   xmm2,       xmm7

            paddw       xmm0,       xmm1
            paddw       xmm0,       xmm2

            psraw       xmm0,       2
            packuswb    xmm0,       xmm7

            movq        QWORD ptr [edi+ecx],   xmm0
            add         ecx,        8        

            cmp         ecx,        ebx
            jl          WmtDeInterlaceLoop
            
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
    
    //copy the last line
    CurrentSrcPtr += Stride;
    CurrentDstPtr += Stride;
    memcpy ( CurrentDstPtr, CurrentSrcPtr, Width );
#endif
  
}
