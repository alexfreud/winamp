/****************************************************************************
* 
*   Module Title :     Transform.c
*
*   Description  :     DCT transform & inverse transform functions.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include <math.h>           // For Abs()
#include "type_aliases.h"
#include "codec_common.h"

/****************************************************************************
 * 
 *  ROUTINE       : SUB8
 *
 *  INPUTS        : UINT8 *FiltPtr     : Pointer to 8x8 source block.
 *                  UINT8 *ReconPtr    : Pointer to 8x8 block to be subtracted from FiltPtr.
 *                  UINT8 *old_ptr1    : NOT USED.
 *                  UINT8 *new_ptr1    : NOT USED.
 *                  INT32 SourceStride : Stride of FiltPtr.
 *                  INT32 ReconStride  : Stride of ReconPtr.
 *
 *  OUTPUTS       : INT16 *DctInputPtr : Pointer to 8x8 array to hold difference.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Does a pixel-by-pixel subtraction of the two 8x8 blocks
 *                  and stores the results in DctInputPtr.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void SUB8 
( 
    UINT8 *FiltPtr,
    UINT8 *ReconPtr,
    INT16 *DctInputPtr,
    UINT8 *old_ptr1,     /* NOT USED */
    UINT8 *new_ptr1,     /* NOT USED */
    INT32 SourceStride,
    INT32 ReconStride
)
{
    int i;

    // Loop unrolled to improve speed...
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {
        DctInputPtr[0] = (INT16)((int)(FiltPtr[0]) - ((int)ReconPtr[0]) );
        DctInputPtr[1] = (INT16)((int)(FiltPtr[1]) - ((int)ReconPtr[1]) );
        DctInputPtr[2] = (INT16)((int)(FiltPtr[2]) - ((int)ReconPtr[2]) );
        DctInputPtr[3] = (INT16)((int)(FiltPtr[3]) - ((int)ReconPtr[3]) );
        DctInputPtr[4] = (INT16)((int)(FiltPtr[4]) - ((int)ReconPtr[4]) );
        DctInputPtr[5] = (INT16)((int)(FiltPtr[5]) - ((int)ReconPtr[5]) );
        DctInputPtr[6] = (INT16)((int)(FiltPtr[6]) - ((int)ReconPtr[6]) );
        DctInputPtr[7] = (INT16)((int)(FiltPtr[7]) - ((int)ReconPtr[7]) );

        // Next row...
        FiltPtr     += SourceStride;
        ReconPtr    += ReconStride;
        DctInputPtr += BLOCK_HEIGHT_WIDTH;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : Sub8_128
 *
 *  INPUTS        : UINT8 *FiltPtr     : Pointer to 8x8 source block.
 *                  UINT8 *old_ptr1    : NOT USED.
 *                  UINT8 *new_ptr1    : NOT USED.
 *                  INT32 SourceStride : Stride of FiltPtr.
 *
 *  OUTPUTS       : INT16 *DctInputPtr : Pointer to 8x8 array to hold modified block.
 *
 *  RETURNS       : None.
 *
 *  FUNCTION      : Subtracts the value 128 from each pixel value in the
 *                  input block FiltPtr.
 *
 *  SPECIAL NOTES : Used when coding a block in INTRA mode to convert the
 *                  pixel range (0,255) to (-128,127). This reduces the 
 *                  internal precision required by the DCT transform.
 *
 ****************************************************************************/
void SUB8_128
( 
    UINT8 *FiltPtr,
    INT16 *DctInputPtr,
    UINT8 *old_ptr1,    /* NOT USED */
    UINT8 *new_ptr1,    /* NOT USED */
    INT32 SourceStride 
)
{
    int i;

    // Loop unrolled to improve speed...
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {
        DctInputPtr[0] = (INT16)((int)(FiltPtr[0]) - 128);
        DctInputPtr[1] = (INT16)((int)(FiltPtr[1]) - 128);
        DctInputPtr[2] = (INT16)((int)(FiltPtr[2]) - 128);
        DctInputPtr[3] = (INT16)((int)(FiltPtr[3]) - 128);
        DctInputPtr[4] = (INT16)((int)(FiltPtr[4]) - 128);
        DctInputPtr[5] = (INT16)((int)(FiltPtr[5]) - 128);
        DctInputPtr[6] = (INT16)((int)(FiltPtr[6]) - 128);
        DctInputPtr[7] = (INT16)((int)(FiltPtr[7]) - 128);
        
        // Next row...
        FiltPtr     += SourceStride;
        DctInputPtr += BLOCK_HEIGHT_WIDTH;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : SUB8AV2
 *
 *  INPUTS        : UINT8 *FiltPtr     : Pointer to 8x8 source block.
 *                  UINT8 *ReconPtr1   : Pointer to first 8x8 reference block.
 *                  UINT8 *ReconPtr2   : Pointer to second 8x8 reference block.
 *                  UINT8 *old_ptr1    : NOT USED.
 *                  UINT8 *new_ptr1    : NOT USED.
 *                  INT32 SourceStride : Stride of FiltPtr.
 *                  INT32 ReconStride  : Stride of ReconPtr1 & ReconPtr2.
 *
 *  OUTPUTS       : INT16 *DctInputPtr : Pointer to 8x8 array to hold difference.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Subtracts the average of the two reconstruction blocks
 *                  from the FiltPtr block.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void SUB8AV2
( 
    UINT8 *FiltPtr,
    UINT8 *ReconPtr1,
    UINT8 *ReconPtr2,
    INT16 *DctInputPtr,
    UINT8 *old_ptr1,    /* NOT USED */
    UINT8 *new_ptr1,    /* NOT USED */
    INT32 SourceStride,
    INT32 ReconStride 
)
{
    int i;

    // Loop unrolled to improve speed...
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {   
        DctInputPtr[0] = (INT16)((int)(FiltPtr[0]) - (((int)ReconPtr1[0] + (int)ReconPtr2[0]) / 2) );
        DctInputPtr[1] = (INT16)((int)(FiltPtr[1]) - (((int)ReconPtr1[1] + (int)ReconPtr2[1]) / 2) );
        DctInputPtr[2] = (INT16)((int)(FiltPtr[2]) - (((int)ReconPtr1[2] + (int)ReconPtr2[2]) / 2) );
        DctInputPtr[3] = (INT16)((int)(FiltPtr[3]) - (((int)ReconPtr1[3] + (int)ReconPtr2[3]) / 2) );
        DctInputPtr[4] = (INT16)((int)(FiltPtr[4]) - (((int)ReconPtr1[4] + (int)ReconPtr2[4]) / 2) );
        DctInputPtr[5] = (INT16)((int)(FiltPtr[5]) - (((int)ReconPtr1[5] + (int)ReconPtr2[5]) / 2) );
        DctInputPtr[6] = (INT16)((int)(FiltPtr[6]) - (((int)ReconPtr1[6] + (int)ReconPtr2[6]) / 2) );
        DctInputPtr[7] = (INT16)((int)(FiltPtr[7]) - (((int)ReconPtr1[7] + (int)ReconPtr2[7]) / 2) );
              
        // Next row...
        FiltPtr     += SourceStride;
        ReconPtr1   += ReconStride;
        ReconPtr2   += ReconStride;
        DctInputPtr += BLOCK_HEIGHT_WIDTH;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : AllZeroDctData
 *
 *  INPUTS        : Q_LIST_ENTRY *QuantList : Array of quantized DCT coefficients.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : BOOL: TRUE if all quantized DCT coeffs are zero, FALSE otherwise.
 *
 *  FUNCTION      : Checks for case where all DCT data will be zero.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
BOOL AllZeroDctData ( Q_LIST_ENTRY * QuantList )
{
    UINT32 i;

    for ( i=0; i<64; i++ )
        if ( QuantList[i] != 0 )
            return FALSE;
    return TRUE;
}

/****************************************************************************
 * 
 *  ROUTINE       : Sub8Filtered
 *
 *  INPUTS        : UINT8 *FiltPtr     : Pointer to 8x8 source block.
 *                  UINT8 *ReconPtr    : Pointer to 8x8 block to be subtracted from FiltPtr.
 *                  INT32 SourceStride : Stride of FiltPtr.
 *                  INT32 ReconStride  : Stride of ReconPtr.
 *                  INT32 *Kernel      : Pointer to filter taps to filter source.
 *
 *  OUTPUTS       : INT16 *DctInputPtr : Pointer to 8x8 array to hold difference.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Does a pixel-by-pixel subtraction of the two 8x8 blocks
 *                  and stores the results in DctInputPtr. However, at any
 *                  pixel if the difference exceeds 4 then a 3x3 filter is 
 *                  applied to the source block before doing the subtraction.
 *
 *  SPECIAL NOTES : The Kernel actually has 10 entries, the first 9 are the
 *                  taps of the 3x3 filter, the last is the filter normalization
 *                  factor. 
 *
 ****************************************************************************/
void Sub8Filtered
(  
    UINT8 *FiltPtr, 
    UINT8 *ReconPtr, 
    INT16 *DctInputPtr, 
    INT32 SourceStride, 
    INT32 ReconStride, 
    INT32 *Kernel 
)
{
    int i,j;
	INT32 Tmp;
	INT32 Diff;
	UINT8 *SrcPtr;

    // Loop unrolled to improve speed...
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {
		for ( j=0; j<BLOCK_HEIGHT_WIDTH; j++ )
		{
			Diff = (INT32)((INT32)FiltPtr[j] - (INT32)ReconPtr[j]);
			
            if ( abs( Diff ) > 4 )
			{
                // Filter source

                // Top row of filter...
				SrcPtr = &FiltPtr[j-SourceStride];
				Tmp  = (UINT32)SrcPtr[-1] * Kernel[0];
				Tmp += (UINT32)SrcPtr[0] * Kernel[1];
				Tmp += (UINT32)SrcPtr[1] * Kernel[2];
				
                // Middle row of filter...
                SrcPtr = &FiltPtr[j];
				Tmp += (UINT32)SrcPtr[-1] * Kernel[3];
				Tmp +=  (UINT32)SrcPtr[0] * Kernel[4];
				Tmp += (UINT32)SrcPtr[1] * Kernel[5];
				
                // Bottom row of filter...
                SrcPtr = &FiltPtr[j+SourceStride];
				Tmp += (UINT32)SrcPtr[-1] * Kernel[6];
				Tmp += (UINT32)SrcPtr[0] * Kernel[7];
				Tmp += (UINT32)SrcPtr[1] * Kernel[8];
				
                // Normalize filter output...
                Tmp = Tmp / Kernel[9];

                // Subtract...
				Tmp = (Tmp - (INT32)ReconPtr[j]);

				// Dcide whether to use filtered or unfiltered result...
                if ( abs(Tmp)+4 < abs(Diff) )
					DctInputPtr[j] = (INT16)Tmp;
				else  
					DctInputPtr[j] = (INT16)Diff;
			}
			else
				DctInputPtr[j] = (INT16)Diff;
		}

        // Next row...
        FiltPtr     += SourceStride;
        ReconPtr    += ReconStride;
        DctInputPtr += BLOCK_HEIGHT_WIDTH;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : Sub8_128Filtered
 *
 *  INPUTS        : UINT8 *FiltPtr     : Pointer to 8x8 source block.
 *                  INT32 SourceStride : Stride of FiltPtr.
 *                  INT32 *Kernel      : Pointer to filter taps to filter source.
 *
 *  OUTPUTS       : INT16 *DctInputPtr : Pointer to 8x8 array to hold difference.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a 3x3 filter to the source data and then subtracts
 *                  128 from each pixel value. The resulting block is stored in
 *                  DctInputPtr.
 *
 *  SPECIAL NOTES : The Kernel actually has 10 entries, the first 9 are the
 *                  taps of the 3x3 filter, the last is the filter normalization
 *                  factor. 
 *
 ****************************************************************************/
void Sub8_128Filtered
(  
    UINT8 *FiltPtr,
    INT16 *DctInputPtr,
    INT32 SourceStride,
    INT32 *Kernel
)
{
    int   i, j;
	INT32 Tmp;
	UINT8 *SrcPtr;

    // Loop unrolled to improve speed...
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {
		for ( j=0; j<BLOCK_HEIGHT_WIDTH; j++ )
		{
            // Filter source

            // Top row of filter...
			SrcPtr = &FiltPtr[j-SourceStride];
			Tmp  = (UINT32)SrcPtr[-1] * Kernel[0];
			Tmp += (UINT32)SrcPtr[0] * Kernel[1];
			Tmp += (UINT32)SrcPtr[1] * Kernel[2];
			
            // Middle row of filter...
            SrcPtr = &FiltPtr[j];
			Tmp += (UINT32)SrcPtr[-1] * Kernel[3];
			Tmp +=  (UINT32)SrcPtr[0] * Kernel[4];
			Tmp += (UINT32)SrcPtr[1] * Kernel[5];
			
            // Bottom row of filter...
            SrcPtr = &FiltPtr[j+SourceStride];
			Tmp += (UINT32)SrcPtr[-1] * Kernel[6];
			Tmp += (UINT32)SrcPtr[0] * Kernel[7];
			Tmp += (UINT32)SrcPtr[1] * Kernel[8];
			
            // Normalize filter output...
            Tmp = Tmp / Kernel[9];

            // Subtract...
			DctInputPtr[j] = (INT16)(Tmp - (INT32)128);
		}

        // Next row...
        FiltPtr     += SourceStride;
        DctInputPtr += BLOCK_HEIGHT_WIDTH;
    }
}
