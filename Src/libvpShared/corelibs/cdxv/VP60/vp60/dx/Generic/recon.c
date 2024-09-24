/****************************************************************************
*
*   Module Title :     recon.c
*
*   Description  :     Frame reconstruction functions.
*
****************************************************************************/
#define STRICT              /* Strict type checking. */

/****************************************************************************
*  Header Files
****************************************************************************/
#include <math.h>
#include "pbdll.h"

/****************************************************************************
*  Macros
****************************************************************************/              
#define TMAX 6
#define TMIN 1

#define Mod8(a) ((a) & 7)

/*************************************************************************** 
 *
 *  ROUTINE       :     Var16Point
 *
 *  INPUTS        :     UINT8 *DataPtr     : Pointer to data block.
 *                      INT32 SourceStride : Block stride.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     UINT32: Calculated 16-point variance (no scaling).
 *
 *  FUNCTION      :     Calculates variance for the 8x8 block *BUT* only samples
 *                      every second pixel in every second row of the block. In
 *                      other words for the 8x8 block only 16 sample points are used.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
UINT32 Var16Point ( UINT8 *DataPtr, INT32 SourceStride )
{
    UINT32  i;
    UINT32  XSum=0;
    UINT32  XXSum=0;
    UINT8   *DiffPtr;

    // Loop expanded out for speed.
    DiffPtr = DataPtr;

    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i+=2 )
    {
        // Examine alternate pixel locations.
        XSum += DiffPtr[0];
        XXSum += DiffPtr[0] * DiffPtr[0];
        XSum += DiffPtr[2];
        XXSum += DiffPtr[2] * DiffPtr[2];
        XSum += DiffPtr[4];
        XXSum += DiffPtr[4] * DiffPtr[4];
        XSum += DiffPtr[6];
        XXSum += DiffPtr[6] * DiffPtr[6];

        // Step to next row of block.
        DiffPtr += (SourceStride << 1);
    }

    // Compute population variance as mis-match metric.
    return (( (XXSum<<4) - XSum*XSum ) ) >> 8;
}

/*************************************************************************** 
 *
 *  ROUTINE       :     DiffVar16Point
 *
 *  INPUTS        :     UINT8 *DataPtr     : Pointer to data block.
 *                      INT32 SourceStride : Block stride.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     UINT32: Calculated 16-point variance (no scaling).
 *
 *  FUNCTION      :     Calculates a variance for 16 data values.
 *						Each data value is the absolute difference between a pair of samples
 *                      one line and one column apart 
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
UINT32 DiffVar16Point ( UINT8 *DataPtr, INT32 SourceStride )
{
    UINT32  i;
	INT32   X;
    UINT32  XSum=0;
    UINT32  XXSum=0;
    UINT8   *DiffPtr;
    UINT8   *DiffPtr2;

    // Loop expanded out for speed.
    DiffPtr = DataPtr;
	DiffPtr2 = DataPtr + SourceStride + 1;

    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i+=2 )
    {
        // Examine alternate pixel locations.
		X = abs( DiffPtr[0] - DiffPtr2[0]);
        XSum += X;
        XXSum += X * X;

		X = abs( DiffPtr[2] - DiffPtr2[2]);
        XSum += X;
        XXSum += X * X;

		X = abs( DiffPtr[4] - DiffPtr2[4]);
        XSum += X;
        XXSum += X * X;

		X = abs( DiffPtr[6] - DiffPtr2[6]);
        XSum += X;
        XXSum += X * X;

        // Step to next row of block.
        DiffPtr += (SourceStride << 1);
		DiffPtr2 += (SourceStride << 1);
    }

    // Compute population variance as mis-match metric.
    return (( (XXSum<<4) - XSum*XSum ) ) >> 8;
}

/****************************************************************************
 * 
 *  ROUTINE       :     InitLoopDeringThresholds
 *
 *  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Initialise thresholds used in the prediction/loop 
 *                      deringing filter.
 *
 *  SPECIAL NOTES :     
 *
 *****************************************************************************/
void InitLoopDeringThresholds ( PB_INSTANCE *pbi )
{
	UINT32 i;

	pbi->DrCutOff = 64;
	for ( i=0; i<pbi->DrCutOff; i++ )
		pbi->DrThresh[255 - i] = ((TMAX * pbi->DrCutOff) - ((TMAX - TMIN) * i)) / pbi->DrCutOff;

	for ( i=pbi->DrCutOff; i<255; i++ )
		pbi->DrThresh[255 - i] = TMIN;
}

/****************************************************************************
 * 
 *  ROUTINE       :     LoopDeringBlock
 *
 *  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
 *                      UINT8 *SrcPtr    : Pointer to block to be deringed.
 *                      UINT32 Stride    : Stride for input block data.
 *                      UINT32 Width     : Block width.
 *                      UINT32 Height    : Block height.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Applies a thresholded dering/smoothing filter to a block
 *						of data.
 *
 *  SPECIAL NOTES :     
 *
 *****************************************************************************/
void LoopDeringBlock 
( 
    PB_INSTANCE *pbi, 
    UINT8 *SrcPtr, 
    UINT32 Stride, 
    UINT32 Width, 
    UINT32 Height 
)
{
	UINT32 i,j;

	UINT8 *DataPtr0;
	UINT8 *DataPtr1;
	UINT8 *DataPtr2;
	UINT8  TmpBuffer[16];		// TBD only one value needed... clean up code

	INT32  ADiff1;
	INT32  ADiff2;

	INT32  Sum;
	INT32  Thresh;

	UINT8  Min = 255;
	UINT8  Max = 0;

	// Look for the min and max value in the block
	DataPtr1 = SrcPtr;
	for ( i=0; i<Height; i++ )
	{
		for ( j=0; j<Width; j++ )
		{
			if ( *DataPtr1 < Min )
				Min = *DataPtr1;
			if ( *DataPtr1 > Max )
				Max = *DataPtr1;

			DataPtr1++;
		}
		DataPtr1 = (DataPtr1 - Width) + Stride;
	}

	// Now choose the dering threshold
	if ( pbi->DrThresh[255 - Min] > pbi->DrThresh[Max] )
		Thresh = pbi->DrThresh[255 - Min];
	else
		Thresh = pbi->DrThresh[Max];

	// Threshold bigger for bigger range
	Thresh += ((Max - Min) >> 5);

	// Horizontal dering
	DataPtr1 = SrcPtr; 
	for ( i=0; i<Height; i++ )
	{
		for ( j=0; j<Width; j++ )
		{
			ADiff1 = abs( (INT32)DataPtr1[j] - (INT32)DataPtr1[j-1] );
			ADiff2 = abs( (INT32)DataPtr1[j] - (INT32)DataPtr1[j+1] );

			Sum = DataPtr1[j] + DataPtr1[j];

			if ( ADiff1 <= Thresh )
				Sum += DataPtr1[j-1];
			else
				Sum += DataPtr1[j];

			if ( ADiff2 <= Thresh )
				Sum += DataPtr1[j+1];
			else
				Sum += DataPtr1[j];
			
			Sum = (Sum + 2) >> 2;

			TmpBuffer[j] = Sum;
		}

		// Copy back the filtered line
		memcpy ( DataPtr1, TmpBuffer, Width );

		// Next line
		DataPtr1 += Stride;
	}

	// Vertical dering
	for ( i=0; i<Width; i++ )
	{
		DataPtr1 = SrcPtr + i;
		DataPtr0 = DataPtr1 - Stride;
		DataPtr2 = DataPtr1 + Stride;

		for ( j=0; j<Height; j++ )
		{
			ADiff1 = abs( (INT32)*DataPtr1 - (INT32)*DataPtr0 );
			ADiff2 = abs( (INT32)*DataPtr1 - (INT32)*DataPtr2 );

			Sum = *DataPtr1 + *DataPtr1;

			if ( ADiff1 <= Thresh ) 
				Sum += *DataPtr0;
			else
				Sum += *DataPtr1;

			if ( ADiff2 <= Thresh )
				Sum += *DataPtr2;
			else
				Sum += *DataPtr1;
			
			Sum = (Sum + 2) >> 2;

			TmpBuffer[j] = Sum;

			DataPtr0 += Stride;
			DataPtr1 += Stride;
			DataPtr2 += Stride;
		}

		// Copy back the filtered data
		DataPtr1 = SrcPtr + i;
		for ( j=0; j<Height; j++ )
		{
			*DataPtr1 = TmpBuffer[j];
			DataPtr1 += Stride;
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_PredictFiltered
 *
 *  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
 *                      UINT8 *SrcPtr    : Pointer to block to be filtered.
 *	                    INT32 mx         :
 *	                    INT32 my         :
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Build an 8x8 motion prediction block. If the block is 
 *                      copied across a block boundary, attempt to eliminate 
 *                      the internal border by applying the loop filter internally.
 *
 *  SPECIAL NOTES :     None.
 *
 *****************************************************************************/
void VP6_PredictFiltered
(
	PB_INSTANCE *pbi,
	UINT8 *SrcPtr,
	INT32 mx,
	INT32 my,
    UINT32 bp
) 
{
    INT32  mVx, mVy;
	INT32  ReconIndex;
	MACROBLOCK_INFO *mbi=&pbi->mbi;
    
	UINT8 *TempBuffer = pbi->LoopFilteredBlock;

	INT32  BoundaryX, BoundaryY; 

	// Calculate full pixel motion vector position 
    if(mx > 0 )
        mVx = (mx >> pbi->mbi.blockDxInfo[bp].MvShift);
    else 
        mVx = -((-mx) >> pbi->mbi.blockDxInfo[bp].MvShift);

    if(my > 0 )
        mVy = (my >> pbi->mbi.blockDxInfo[bp].MvShift);
    else
        mVy = -((-my) >> pbi->mbi.blockDxInfo[bp].MvShift);

	// calculate offset in last frame matching motion vector
	ReconIndex = mbi->blockDxInfo[bp].FrameReconStride * mVy + mVx;

	// Give our selves a border of 2 extra pixel on all sides (for loop filter and half pixel moves)
	ReconIndex -= 2 * mbi->blockDxInfo[bp].CurrentReconStride;
	ReconIndex -= 2;

	// copy the 12x12 region starting from reconpixel index into our temp buffer.
    Copy12x12( SrcPtr + ReconIndex, TempBuffer, mbi->blockDxInfo[bp].CurrentReconStride, 16);

	// What sort of loop filtering are we doing
	// Dering loop filter is mandated to OFF in the current bitstream#
    //if ( pbi->UseLoopFilter == LOOP_FILTER_DERING )
	if ( FALSE )
	{
		// Apply prediction.loop dering filter
		LoopDeringBlock( pbi, &TempBuffer[16+1], 16, 10, 10 );
	}
	else
	{
		// calculate block border position for x
		BoundaryX = (8 - Mod8(mVx))&7;

		// calculate block border position for y
		BoundaryY = (8 - Mod8(mVy))&7;

		// apply the loop filter at the horizontal boundary we selected
		if(BoundaryX)
			FilteringHoriz_12(
				pbi->quantizer->FrameQIndex, 
				TempBuffer + 2 + BoundaryX, 
				16); 

		// apply the loop filter at the vertical boundary we selected
		if (BoundaryY)
			FilteringVert_12(
				pbi->quantizer->FrameQIndex, 
				TempBuffer + 2 * 16 + BoundaryY * 16, 
				16);
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     PredictFilteredBlock
 *
 *  INPUTS        :     PB_INSTANCE *pbi  : Pointer to decoder instance.
 *                      INT16 *OutputPtr  : Pointer to output data.
 *                      BLOCK_POSITION bp : Position of block within MB.
 *                      
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Build an 8x8 motion prediction block. If the block is 
 *                      copied across a block boundary, attempt to eliminate 
 *                      the internal border by applying the loop filter internally.
 *
 *  SPECIAL NOTES :     
 *
 *****************************************************************************/
void VP6_PredictFilteredBlock 
(
	PB_INSTANCE *pbi,
	INT16 *OutputPtr,
	UINT32 bp  
) 
{
    UINT8 *SrcPtr;
	UINT8 *TempBuffer;
	UINT32 TempPtr1;
	UINT32 TempPtr2;
	INT32  ModX, ModY;
	UINT32 IVar;
	UINT32 BicMvSizeLimit;
	UINT32 Stride;

    UINT32 MvShift = pbi->mbi.blockDxInfo[bp].MvShift; //pbi->mbi.MvShift;
    UINT32 MvModMask = pbi->mbi.blockDxInfo[bp].MvModMask; //pbi->mbi.MvModMask;

    // Which buffer are we working on?
    SrcPtr = pbi->LastFrameRecon;
    if ( VP6_Mode2Frame[pbi->mbi.Mode] == 2 ) 
    {
        SrcPtr = pbi->GoldenFrame;
    }

    // No loop filtering in simple profile
	if ( pbi->VpProfile == SIMPLE_PROFILE || (pbi->UseLoopFilter == NO_LOOP_FILTER) )
	{
	    INT32  mVx, mVy;
		INT32  mx = pbi->mbi.Mv[bp].x;
		INT32  my = pbi->mbi.Mv[bp].y;

        // Mask off fractional pel bits.
	    ModX = (mx & MvModMask);
	    ModY = (my & MvModMask); 

		// Calculate full pixel motion vector position 
        mx += (MvModMask&(mx>>31));
        my += (MvModMask&(my>>31));
        
		mVx = (mx >> MvShift);
		mVy = (my >> MvShift);

		// Set up a pointer into the recon buffer
		TempBuffer = SrcPtr + pbi->mbi.blockDxInfo[bp].thisRecon + (pbi->mbi.blockDxInfo[bp].FrameReconStride * mVy + mVx);
		Stride = pbi->mbi.blockDxInfo[bp].CurrentReconStride;
		TempPtr1 = TempPtr2 = 0;
	}
	else
	{
		// Loop filter the block
		VP6_PredictFiltered( pbi, SrcPtr + pbi->mbi.blockDxInfo[bp].thisRecon, pbi->mbi.Mv[bp].x, pbi->mbi.Mv[bp].y, bp );
		TempBuffer = pbi->LoopFilteredBlock;
		Stride = 16;
		TempPtr1 = 2*16+2;		// Offset into the 12x12 loop filtered buffer
		TempPtr2 = TempPtr1;
        
        // Mask off fractional pel bits.
	    ModX = (pbi->mbi.Mv[bp].x & MvModMask);
	    ModY = (pbi->mbi.Mv[bp].y & MvModMask); 
	}

    // determine if we have a fractional pixel move in the x direction
	if ( ModX )
	{
		TempPtr2 += ( pbi->mbi.Mv[bp].x > 0 )*2 -1;        
	}

	// handle fractional pixel motion in Y
	if ( ModY )
	{
        TempPtr2 += (( pbi->mbi.Mv[bp].y > 0 ) * 2 - 1)*Stride;
	}
 
	// put the results back into the real reconstruction buffer
    if ( TempPtr1 != TempPtr2 ) 
	{
		// The FilterBlock selects a filter based upon a ModX and ModY value that are at 1/8 point 
		// precision. Because U and V are subsampled the vector is already at the right precision 
		// for U and V but for Y we have to multiply by 2.
		if ( bp < 4 )
		{
			// Filterblock expects input at 1/8 pel resolution (hence << 1 for Y)
			ModX = ModX << 1;
			ModY = ModY << 1; 

			// Select the filtering mode
			if ( pbi->VpProfile == SIMPLE_PROFILE )
			{
				// Simple profile always uses bilinear filtering for speed
				FilterBlock( &TempBuffer[TempPtr1], &TempBuffer[TempPtr2], (unsigned short *)OutputPtr, Stride, ModX, ModY, FALSE, 14 );
			}
			else if ( pbi->PredictionFilterMode == AUTO_SELECT_PM )
			{
				//  Work out the Mv size limit for selecting bicubic
				if ( pbi->PredictionFilterMvSizeThresh > 0 )
					BicMvSizeLimit = (1 << (pbi->PredictionFilterMvSizeThresh - 1)) << 2;			 // Convert to a value in 1/4 pel units
				else
					BicMvSizeLimit = ((MAX_MV_EXTENT >> 1) + 1) << 2;								 // Unrestricted

				// Only use bicubic on shortish vectors
				if ( ( pbi->PredictionFilterMvSizeThresh != 0 ) &&
					 ( ( (UINT32)abs(pbi->mbi.Mv[bp].x) > BicMvSizeLimit ) || ( (UINT32)abs(pbi->mbi.Mv[bp].y) > BicMvSizeLimit ) ) )
				{
					FilterBlock( &TempBuffer[TempPtr1], &TempBuffer[TempPtr2], (unsigned short *)OutputPtr, Stride, ModX, ModY, FALSE, pbi->PredictionFilterAlpha);
				}
			    // Should we use a variance test for bicubic as well
				else if ( pbi->PredictionFilterVarThresh != 0 )
				{
					IVar = Var16Point( &TempBuffer[TempPtr1], Stride );
					FilterBlock( &TempBuffer[TempPtr1], &TempBuffer[TempPtr2], (unsigned short *)OutputPtr, Stride, ModX, ModY, (IVar >= pbi->PredictionFilterVarThresh), pbi->PredictionFilterAlpha );
				}
				else
				{
					FilterBlock( &TempBuffer[TempPtr1], &TempBuffer[TempPtr2], (unsigned short *)OutputPtr, Stride, ModX, ModY, TRUE, pbi->PredictionFilterAlpha );
				}
			}
			else  
				FilterBlock( &TempBuffer[TempPtr1], &TempBuffer[TempPtr2], (unsigned short *)OutputPtr, Stride, ModX, ModY, (pbi->PredictionFilterMode == BICUBIC_ONLY_PM), pbi->PredictionFilterAlpha );
		}
		else
		{
			FilterBlock( &TempBuffer[TempPtr1], &TempBuffer[TempPtr2], (unsigned short *)OutputPtr, Stride, ModX, ModY, FALSE, pbi->PredictionFilterAlpha );
		}
	}
	// No fractional pels
    else
        UnpackBlock(&TempBuffer[TempPtr1], OutputPtr, Stride );

}

/****************************************************************************
 * 
 *  ROUTINE       :     ReconstructBlock
 *
 *  INPUTS        :     PB_INSTANCE *pbi  : Pointer to decoder instance.
 *                      BLOCK_POSITION bp : Position of block within MB.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Reconstructs the coded block depending on coding mode.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_ReconstructBlock ( PB_INSTANCE *pbi, BLOCK_POSITION bp )
{
    UINT32 thisRecon = pbi->mbi.blockDxInfo[bp].thisRecon;

	// Action depends on decode mode.
	if ( pbi->mbi.Mode == CODE_INTER_NO_MV )       // Inter with no motion vector
	{
		ReconInter( pbi->TmpDataBuffer, 
                    (UINT8 *)&pbi->ThisFrameRecon[thisRecon], 
			        (UINT8 *)&pbi->LastFrameRecon[thisRecon], 
			        (INT16 *)pbi->ReconDataBuffer[bp], 
                    pbi->mbi.blockDxInfo[bp].CurrentReconStride);
		
	}
	else if ( VP6_ModeUsesMC[pbi->mbi.Mode] )          // The mode uses a motion vector.
	{
		// For the compressor we did this already ( possible optimization).
		VP6_PredictFilteredBlock( pbi, pbi->TmpDataBuffer,bp);

		ReconBlock( pbi->TmpDataBuffer,
			        (INT16 *)pbi->ReconDataBuffer[bp],
			        (UINT8 *)&pbi->ThisFrameRecon[thisRecon],
			        pbi->mbi.blockDxInfo[bp].CurrentReconStride );
	}
	else if ( pbi->mbi.Mode == CODE_USING_GOLDEN )     // Golden frame with motion vector
	{
		// Reconstruct the pixel data using the golden frame reconstruction and change data
		ReconInter( pbi->TmpDataBuffer, 
                    (UINT8 *)&pbi->ThisFrameRecon[thisRecon], 
			        (UINT8 *)&pbi->GoldenFrame[thisRecon], 
			        (INT16 *)pbi->ReconDataBuffer[bp], 
                    pbi->mbi.blockDxInfo[bp].CurrentReconStride );
	}
	else                                            // Simple Intra coding
	{
		// Get the pixel index for the first pixel in the fragment.
		ReconIntra( pbi->TmpDataBuffer, 
                    (UINT8 *)&pbi->ThisFrameRecon[thisRecon], 
                    (UINT16 *)pbi->ReconDataBuffer[bp], 
                    pbi->mbi.blockDxInfo[bp].CurrentReconStride );
	}
}
