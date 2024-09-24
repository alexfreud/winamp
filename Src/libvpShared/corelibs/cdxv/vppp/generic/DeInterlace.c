/**************************************************************************** 
 *
 *   Module Title :     DeInterlace.c
 *
 *   Description  :     De-Interlace routines.
 *
 ***************************************************************************/

/****************************************************************************
*  Header Files
****************************************************************************/
#include <memory.h>
#include "type_aliases.h"

/****************************************************************************
 * 
 *  ROUTINE       : CFastDeInterlace
 *
 *  INPUTS        : UINT8 *SrcPtr : Pointer to input image.
 *                  UINT8 *DstPtr : Pointer to output image.
 *                  INT32 Width   : Image width.
 *                  INT32 Height  : Image height.
 *                  INT32 Stride  : Image stride.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a 3-tap filter vertically to remove interlacing
 *                  artifacts.
 *
 *  SPECIAL NOTES : This function use a three tap filter [1, 2, 1] to blur 
 *                  veritically in an interlaced frame. This function assumes:
 *                  1) SrcPtr & DstPtr buffers have the same geometry.
 *                  2) SrcPtr != DstPtr.
 *
 ****************************************************************************/
void CFastDeInterlace
(
    UINT8 *SrcPtr,
    UINT8 *DstPtr,
    INT32 Width,
    INT32 Height,
    INT32 Stride
)
{
    INT32  i, j;
    UINT32 x0, x1, x2;
    UINT8 *PrevSrcPtr, *NextSrcPtr;
    UINT8 *CurrentSrcPtr = SrcPtr;
    UINT8 *CurrentDstPtr = DstPtr;
    
    // Always copy the first line
    memcpy ( CurrentDstPtr, CurrentSrcPtr, Width );

    for ( i=1; i<Height-1; i++ )
    {
        PrevSrcPtr     = CurrentSrcPtr;
        CurrentSrcPtr += Stride;
        NextSrcPtr     = CurrentSrcPtr + Stride;
        CurrentDstPtr += Stride;

        for ( j=0; j<Width; j++ )
        {
            x0 = PrevSrcPtr[j];
            x1 = (CurrentSrcPtr[j]<<1);
            x2 = NextSrcPtr[j];
            CurrentDstPtr[j] = (UINT8)( (x0 + x1 + x2 + 2)>>2 );
        }
    }
    
    // Copy the last line
    CurrentSrcPtr += Stride;
    CurrentDstPtr += Stride;
    memcpy ( CurrentDstPtr, CurrentSrcPtr, Width );
}
