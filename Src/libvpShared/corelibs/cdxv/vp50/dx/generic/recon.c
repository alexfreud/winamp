/****************************************************************************
*
*   Module Title :     recon.c
*
*   Description  :     reconstruction code
*
*    AUTHOR      :     jimb b
*
*****************************************************************************
*   Revision History
*
*   1.19 JBB 18 Mar 01 Reorganized code created this file
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include "pbdll.h"
#include "codec_common_interface.h"
#include <string.h>

/****************************************************************************
*  Explicit imports
*****************************************************************************
*/        
extern void AverageBlockBicubic_C( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 ReconPixelsPerLine);
extern void NewAverageBlock( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 ReconPixelsPerLine);
extern void UvAverageBlock( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 ReconPixelsPerLine, INT8 ModX, INT8 ModY );

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        

/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/


/****************************************************************************
*  Exported Functions
*****************************************************************************
*/


/****************************************************************************
*  Module Statics
*****************************************************************************
*/              
#define MIN(a, b) ( ( a < b ) ? a : b )
#define Mod8(a) ( ((a) & 7))

/****************************************************************************
 * 
 *  ROUTINE       :     PredictFilteredBlock
 *
 *  INPUTS        :     
 *                      
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     try to build an 8x8 block motion prediction block. If
 *                      the block is copied across a block boundary attempt 
 *                      to eliminate the internal block border by applying the
 *                      loop filter internally to the block
 *
 *  SPECIAL NOTES :     
 *
 *  ERRORS        :     None.
 *
 *****************************************************************************/
void PredictFiltered
(
	PB_INSTANCE *pbi,
	UINT8 *SrcPtr,
	INT32 mx,
	INT32 my,
	INT32 MvShift
) 
{
    
    INT32  BoundaryX, BoundaryY; 
    INT32  mVx, mVy;
	UINT32 ReconIndex = 0;
	MACROBLOCK_INFO *mbi=&pbi->mbi;
    
	UINT8  TempPtr1 = 2 * 16 + 2;
	UINT32 TempPtr2 = TempPtr1;
	UINT8 *TempBuffer = pbi->LoopFilteredBlock;

	// Calculate full pixel motion vector position 
    if(mx > 0 )
        mVx = (mx >> MvShift);
    else 
        mVx = -((-mx) >> MvShift);

    if(my > 0 )
        mVy = (my >> MvShift);
    else
        mVy = -((-my) >> MvShift);

	// calculate offset in last frame matching motion vector
	ReconIndex += mbi->FrameReconStride * mVy + mVx;

	// give our selves a border of 2 extra pixel on all sides (for loop filter and half pixel moves)
	ReconIndex -= 2 * mbi->CurrentReconStride;
	ReconIndex -= 2;

	// copy the 12x12 region starting from reconpixel index into our temp buffer.
    Copy12x12( SrcPtr + ReconIndex, TempBuffer, mbi->CurrentReconStride, 16);

	// calculate block border position for x
	BoundaryX = (8 - Mod8(mVx))&7;
  
	// calculate block border position for y
	BoundaryY = (8 - Mod8(mVy))&7;

	// apply the loop filter at the horizontal boundary we selected
    if(BoundaryX)
		FilteringHoriz_12(
			pbi	->quantizer->FrameQIndex, 
			TempBuffer + 2 + BoundaryX, 
			16);

	// apply the loop filter at the vertical boundary we selected
    if(BoundaryY)
		FilteringVert_12(
			pbi->quantizer->FrameQIndex, 
			TempBuffer + 2 * 16 + BoundaryY * 16, 
			16);

}

/****************************************************************************
 * 
 *  ROUTINE       :     PredictFilteredBlock
 *
 *  INPUTS        :     
 *                      
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     try to build an 8x8 block motion prediction block. If
 *                      the block is copied across a block boundary attempt 
 *                      to eliminate the internal block border by applying the
 *                      loop filter internally to the block
 *
 *  SPECIAL NOTES :     
 *
 *  ERRORS        :     None.
 *
 *****************************************************************************/
#define AVERAGE_ROUTINE AverageBlock
//#define AVERAGE_ROUTINE AverageBlockBicubic_C
//#define AVERAGE_ROUTINE NewAverageBlock

//#define UV_AVERAGE_ROUTINE AverageBlock
#define UV_AVERAGE_ROUTINE UvAverageBlock

void PredictFilteredBlock
(
	PB_INSTANCE *pbi,
	INT16* OutputPtr,
	BLOCK_POSITION bp 
) 
{
	MACROBLOCK_INFO *mbi=&pbi->mbi;
    
    UINT8 *SrcPtr;

	UINT8 *TempBuffer = pbi->LoopFilteredBlock;

	UINT32 TempPtr1 = 2*16+2;
	UINT32 TempPtr2 = TempPtr1;
	INT8   ModX, ModY;

    // Which buffer are we working on?
    if ( VP5_Mode2Frame[pbi->mbi.Mode] == 2 ) 
    {
        SrcPtr = pbi->GoldenFrame;
    }
    else
    {
        SrcPtr = pbi->LastFrameRecon;
    }

	PredictFiltered( pbi, SrcPtr+mbi->Recon, pbi->mbi.Mv[bp].x, pbi->mbi.Mv[bp].y, pbi->mbi.MvShift) ;

    // determine if we have a half pixel move in the x direction
    if(pbi->mbi.Mv[bp].x & pbi->mbi.MvModMask)
	{
		if ( pbi->mbi.Mv[bp].x > 0 )
		{
			TempPtr2 += 1;
		}
		else
		{
			TempPtr2 -= 1;
		}
	}

	// handle half pixel motion in Y
    if(pbi->mbi.Mv[bp].y & pbi->mbi.MvModMask)
	{
		if ( pbi->mbi.Mv[bp].y > 0 )
		{
			TempPtr2 += 16;
		}
		else
		{
			TempPtr2 -= 16;
		}
	}
 
	// put the results back into the real reconstruction buffer
    if (TempPtr1!=TempPtr2) 
	{
		if ( bp < 4 )
			AVERAGE_ROUTINE(&TempBuffer[TempPtr1], &TempBuffer[TempPtr2], (unsigned short *)OutputPtr, 16);
		else
		{
			ModX = pbi->mbi.Mv[bp].x & 0x03;
			ModY = pbi->mbi.Mv[bp].y & 0x03;

			//UV_AVERAGE_ROUTINE(&TempBuffer[TempPtr1], &TempBuffer[TempPtr2], (unsigned short *)OutputPtr, 16, ModX, ModY );
			AverageBlock(&TempBuffer[TempPtr1], &TempBuffer[TempPtr2], (unsigned short *)OutputPtr, 16);
		}
	}
    else
        UnpackBlock(&TempBuffer[TempPtr1], OutputPtr, 16);

}

#ifndef RECONSTRUCTMBATONCE
/****************************************************************************
 * 
 *  ROUTINE       :     ReconstructBlock
 *
 *  INPUTS        :     
 *						
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Codes a DCT block
 *
 *                      Motion vectors and modes asumed to be defined at the MB level.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ReconstructBlock
( 
	PB_INSTANCE *pbi, 
	BLOCK_POSITION bp
)
{
	
	// Action depends on decode mode.
	if ( pbi->mbi.Mode == CODE_INTER_NO_MV )       // Inter with no motion vector
	{
		ReconInter( pbi->TmpDataBuffer, (UINT8 *)&pbi->ThisFrameRecon[pbi->mbi.Recon], 
			(UINT8 *)&pbi->LastFrameRecon[pbi->mbi.Recon], 
			pbi->ReconDataBuffer, pbi->mbi.CurrentReconStride);
		
	}
	else if ( VP5_ModeUsesMC[pbi->mbi.Mode] )          // The mode uses a motion vector.
	{
		// For the compressor we did this already ( possible optimization).
		PredictFilteredBlock( pbi, pbi->TmpDataBuffer,bp);

		ReconBlock( 
			pbi->TmpDataBuffer,
			pbi->ReconDataBuffer,
			(UINT8 *)&pbi->ThisFrameRecon[pbi->mbi.Recon],
			pbi->mbi.CurrentReconStride );
	}
	else if ( pbi->mbi.Mode == CODE_USING_GOLDEN )     // Golden frame with motion vector
	{
		// Reconstruct the pixel data using the golden frame reconstruction and change data
		ReconInter( pbi->TmpDataBuffer, (UINT8 *)&pbi->ThisFrameRecon[pbi->mbi.Recon], 
			(UINT8 *)&pbi->GoldenFrame[ pbi->mbi.Recon ], 
			pbi->ReconDataBuffer, pbi->mbi.CurrentReconStride );
	}
	else                                            // Simple Intra coding
	{
		// Get the pixel index for the first pixel in the fragment.
		ReconIntra( pbi->TmpDataBuffer, (UINT8 *)&pbi->ThisFrameRecon[pbi->mbi.Recon], (UINT16 *)pbi->ReconDataBuffer, pbi->mbi.CurrentReconStride );
	}
}

#endif

/************************************************************************** * 
 *  ROUTINE       :     CopyBlock
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Copies a block from source to destination
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

void CopyBlockC(unsigned char *src, unsigned char *dest, unsigned int srcstride)
{
	unsigned char * s = src;
	unsigned char * d = dest;
	unsigned int stride = srcstride;

	int j;
    for ( j = 0; j < 8; j++ )
	{
		((UINT32*)d)[0] = ((UINT32*)s)[0];
		((UINT32*)d)[1] = ((UINT32*)s)[1];
		s+=stride;
		d+=stride;
	}
}
