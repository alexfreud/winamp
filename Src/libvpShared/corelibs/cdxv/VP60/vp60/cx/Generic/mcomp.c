/****************************************************************************
*
*   Module Title :     mcomp.c
*
*   Description  :     Motion compensation functions.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include <math.h>       // For Abs()
#include "mcomp.h"
#include "compdll.h"

/****************************************************************************
*  Imports
****************************************************************************/
extern INT32 *XX_LUT;

extern void VP6_PredictFiltered(PB_INSTANCE *pbi,UINT8 *SrcPtr,INT32 mx,INT32 my,INT32 MvShift) ;

/****************************************************************************
*  Macros
****************************************************************************/
#define HP_THRESH       0

// bias towards cheaper motion vectors should be tied to cpi->MVErrorPerBit 
// but isn't at least not yet.  setting this to 0 says don't bias at all
#define MVEPBSAD_MULT		1
#define MVEPBSAD_RSHIFT		2
#define MVEPBSAD_RSHIFT2	14

/****************************************************************************
*  Exports.
****************************************************************************/
UINT32  TotError = 0;
UINT32  ErrCount = 0;

UINT8  FilteredBlock[256];

/****************************************************************************
 *
 *  ROUTINE       : InitDSMotionCompensation
 *
 *  INPUTS        : CP_INSTANCE *cpi     : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Initialises data structures used by the diamond search.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void InitDSMotionCompensation ( CP_INSTANCE *cpi )
{
    int i;
    int Len;
    int SearchSite = 0;
    int LineStepY = (INT32)cpi->pb.Configuration.YStride;

    // How many search stages are there.
    cpi->DSMVSearchSteps = 0;

    // Generate offsets for 4 search sites per step.
    Len = (MAX_MV_EXTENT + 1)/4;				

    while ( Len>0 )
    {
        // Another step.
        cpi->DSMVSearchSteps += 1;

        // Compute offsets for search sites.
        cpi->DSMVOffsetX[SearchSite]   = 0;
        cpi->DSMVOffsetY[SearchSite++] = -Len;

        cpi->DSMVOffsetX[SearchSite]   = -Len;
        cpi->DSMVOffsetY[SearchSite++] = 0;
        
        cpi->DSMVOffsetX[SearchSite]   = Len;
        cpi->DSMVOffsetY[SearchSite++] = 0;
        
        cpi->DSMVOffsetX[SearchSite]   = 0;
        cpi->DSMVOffsetY[SearchSite++] = Len;

        // Contract.
        Len /= 2;
    }

    // Compute pixel index offsets.
    for ( i=SearchSite-1; i>=0; i-- )
        cpi->DSMVPixelOffsetY[i] = (cpi->DSMVOffsetY[i]*LineStepY) + cpi->DSMVOffsetX[i];
}

/****************************************************************************
 * 
 *  ROUTINE       :  InitMotionCompensation
 *
 *  INPUTS        :  CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       :  None.    
 *
 *  RETURNS       :  void
 *
 *  FUNCTION      :  Initialises motion compensation data structures. 
 *
 *  SPECIAL NOTES :  None. 
 *
 ****************************************************************************/
void InitMotionCompensation ( CP_INSTANCE *cpi )
{
    int i;
    int Len;
    int SearchSite = 0; 
    int LineStepY = (INT32)cpi->pb.Configuration.YStride;

    // How many search stages are there.
    cpi->MVSearchSteps = 0;
 
    // Set up offsets arrays used in fractional pel searches
    cpi->SubPixelXOffset[0] = 0;
    cpi->SubPixelXOffset[1] = 0;
    cpi->SubPixelXOffset[2] = -1;
    cpi->SubPixelXOffset[3] = 1;
    cpi->SubPixelXOffset[4] = 0;
    cpi->SubPixelXOffset[5] = -1;
    cpi->SubPixelXOffset[6] = 1;
    cpi->SubPixelXOffset[7] = -1;
    cpi->SubPixelXOffset[8] = 1;

    cpi->SubPixelYOffset[0] = 0;
    cpi->SubPixelYOffset[1] = -1;
    cpi->SubPixelYOffset[2] = 0;
    cpi->SubPixelYOffset[3] = 0;
    cpi->SubPixelYOffset[4] = 1;
    cpi->SubPixelYOffset[5] = -1;
    cpi->SubPixelYOffset[6] = -1;
    cpi->SubPixelYOffset[7] = 1;
    cpi->SubPixelYOffset[8] = 1;

    // Generate offsets for 8 search sites per step.
    Len = (MAX_MV_EXTENT + 1)/4;				
    while ( Len>0 )
    {
        // Another step.
        cpi->MVSearchSteps += 1;

        // Compute offsets for search sites.
        cpi->MVOffsetX[SearchSite]   = -Len;
        cpi->MVOffsetY[SearchSite++] = -Len;
        cpi->MVOffsetX[SearchSite]   = 0;
        cpi->MVOffsetY[SearchSite++] = -Len;
        cpi->MVOffsetX[SearchSite]   = Len;
        cpi->MVOffsetY[SearchSite++] = -Len;
        cpi->MVOffsetX[SearchSite]   = -Len;
        cpi->MVOffsetY[SearchSite++] = 0;
        cpi->MVOffsetX[SearchSite]   = Len;
        cpi->MVOffsetY[SearchSite++] = 0;
        cpi->MVOffsetX[SearchSite]   = -Len;
        cpi->MVOffsetY[SearchSite++] = Len;
        cpi->MVOffsetX[SearchSite]   = 0;
        cpi->MVOffsetY[SearchSite++] = Len;
        cpi->MVOffsetX[SearchSite]   = Len;
        cpi->MVOffsetY[SearchSite++] = Len;

        // Contract.
        Len /= 2;
    }

    // Compute pixel index offsets.
    for ( i=SearchSite-1; i>=0; i-- )
        cpi->MVPixelOffsetY[i] = (cpi->MVOffsetY[i]*LineStepY) + cpi->MVOffsetX[i];

    // set up search sites for 5 region Diamond search    
    InitDSMotionCompensation(cpi);

    // Initialize the function pointers for block motion search
    // and fractional pixel motion search
    cpi->FindMvViaSearch        = FindMvVia3StepSearch;
    cpi->FindBestHalfPixelMv    = FindBestFractionalPixelStep;
    cpi->FindBestQuarterPixelMv = FindBestFractionalPixelStep;
}

/****************************************************************************
 * 
 *  ROUTINE       :  GetMBFrameVerticalVariance
 *
 *  INPUTS        :  CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       :  None.    
 *
 *  RETURNS       :  UINT32: vertical variance for a macroblock.
 *
 *  FUNCTION      :  Computes the vertical variance by summing the local
 *                   2 pixel variances throughout the MB.
 *
 *  SPECIAL NOTES :  The difference between the last two rows in a 
 *                   macro-block are not accounted for!
 *
 ****************************************************************************/
UINT32 GetMBFrameVerticalVariance ( CP_INSTANCE *cpi )
{
    int i, j;
    UINT32 x, y, z;
    UINT32  MBVariance = 0;
    PB_INSTANCE *pbi = &cpi->pb;
//    UINT8 *SrcPtr = &cpi->yuv1ptr[pbi->mbi.Source];
    UINT8 *SrcPtr = &cpi->yuv1ptr[pbi->mbi.blockDxInfo[0].Source];
    INT32 SourceStride = pbi->Configuration.VideoFrameWidth;
    INT32 Pitch2 = SourceStride*2;

    for ( i=0; i<7; i++ )
    {
        for ( j=0; j<16; j++ )
        {
            x = SrcPtr[j];
            y = SrcPtr[j+SourceStride];
            z = SrcPtr[j+Pitch2 ];
            MBVariance +=(x-y)*(x-y) + (y-z)*(y-z);
        }
        SrcPtr += Pitch2;
    }
    return MBVariance;
}

/****************************************************************************
 * 
 *  ROUTINE       :  GetMBFieldVerticalVariance
 *
 *  INPUTS        :  CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       :  None.    
 *
 *  RETURNS       :  UINT32: field vertical variance for a macroblock.
 *
 *  FUNCTION      :  Computes the vertical variance by summing the local
 *                   2 pixel variances within two independent fields
 *                   throughout the MB.
 *
 *  SPECIAL NOTES :  None.
 *
 ****************************************************************************/
UINT32 GetMBFieldVerticalVariance ( CP_INSTANCE *cpi )
{
    int i,j;
    UINT32  x, y, z, w;
    UINT32  MBFieldVariance = 0;
    PB_INSTANCE *pbi = &cpi->pb;
    //UINT8 *SrcPtr = &cpi->yuv1ptr[pbi->mbi.Source];
    UINT8 *SrcPtr = &cpi->yuv1ptr[pbi->mbi.blockDxInfo[0].Source];
    INT32 SourceStride = pbi->Configuration.VideoFrameWidth;
    INT32 Pitch2 = SourceStride*2;

    for ( i=0; i<7; i++ )
    {
        for ( j=0; j<16; j++ )
        { 
            x = SrcPtr[j];
            y = SrcPtr[j+SourceStride];
            z = SrcPtr[j+Pitch2 ];
            w = SrcPtr[j+Pitch2 + SourceStride];
            MBFieldVariance +=(x-z)*(x-z) + (y-w)*(y-w);
        }
        SrcPtr += Pitch2;
    }
    return MBFieldVariance;
}

/****************************************************************************
 * 
 *  ROUTINE       : GetReconReferencePoints
 *  
 *  INPUTS        : PB_INSTANCE *pbi     : Pointer to decoder instance.
 *                  UINT8 *BufferPointer : Pointer to refernce point in reference image.
 *                  MOTION_VECTOR *MV    : Motion vector to be used.
 *						
 *  OUTPUTS       : UINT8 **ReconPtr1    : Pointer-to-pointer to first block in ref frame.
 *                  UINT8 **ReconPtr2    : Pointer-to-pointer to second block in ref frame.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Computes pointers to two blocks in the reference frame
 *                  that bracket the fractional pixel position specified in MV.
 *                  These two blocks will later be used to interpolate
 *                  the prediction block.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void GetReconReferencePoints
( 
    PB_INSTANCE *pbi,
    UINT8 *BufferPointer,
    UINT8 **ReconPtr1,
    UINT8 **ReconPtr2,
    MOTION_VECTOR *MV
)
{
	INT32  mVx,  mVy;
    INT32  ModX, ModY;

    // Calculate full pixel motion vector position 
    if ( MV->x >= 0 )
        mVx = (MV->x >> Y_MVSHIFT);
    else 
        mVx = -((-MV->x) >> Y_MVSHIFT);

    if ( MV->y >= 0 )
        mVy = (MV->y >> Y_MVSHIFT);
    else
        mVy = -((-MV->y) >> Y_MVSHIFT);

	// Calculate the first pointer.
	*ReconPtr1 = BufferPointer + (pbi->mbi.blockDxInfo[0].FrameReconStride * mVy) + mVx;

    // Calculate the second pointer
    *ReconPtr2 = *ReconPtr1;
	ModX = (MV->x & Y_MVMODMASK);
	ModY = (MV->y & Y_MVMODMASK);
    
    if ( ModX )
	{
		if ( MV->x > 0 )
			*ReconPtr2 += 1;
		else
			*ReconPtr2 -= 1;
	}

    if ( ModY )
	{
		if ( MV->y > 0 )
			*ReconPtr2 += pbi->mbi.blockDxInfo[0].CurrentReconStride;
		else
			*ReconPtr2 -= pbi->mbi.blockDxInfo[0].CurrentReconStride;
	}
}

/****************************************************************************
 *
 *  ROUTINE       : GetInterErrQPel
 *
 *  INPUTS        : PB_INSTANCE *pbi    : Pointer to decoder instance.
 *                  UINT8 *NewDataPtr   : Pointer to source block.
 *                  UINT32 SourceStride : Stride for NewDataPtr.
 *                  UINT8 *RefDataPtr1  : Pointer to block position in reference frame.
 *                  UINT8 *RefDataPtr2  : Pointer to block position in reference frame.
 *                  INT32 ReconStride   : Size of the block.
 *                  MOTION_VECTOR *MV   : Best MV found for block in reference frame.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Variance of the prediction error (scaled by 2^12)
 *
 *  FUNCTION      : Calculates scaled prediction error variance for the
 *                  QPel interpolated block.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
UINT32 GetInterErrQPel
(
    PB_INSTANCE *pbi,
    UINT8 *NewDataPtr,
    INT32 SourceStride,
    UINT8 *RefDataPtr1,
    UINT8 *RefDataPtr2,
    INT32 ReconStride,
    MOTION_VECTOR *MV
)
{
    INT32 ModX, ModY;

    // Compute fractional MV offsets (to 1/8 point precision as required by FilterBlock)
    ModX = (MV->x & Y_MVMODMASK) << 1;
    ModY = (MV->y & Y_MVMODMASK) << 1;

    // FilterBlockBil_8 filters the input data to produce an 8x8 Qpel precision prediction block.
	FilterBlockBil_8 ( RefDataPtr1, RefDataPtr2, FilteredBlock, ReconStride, ModX, ModY );

    // Compute and return population variance as mis-match metric.
	return GetInterError ( NewDataPtr, SourceStride, FilteredBlock, FilteredBlock, 8 );
}

/****************************************************************************
 *
 *  ROUTINE       : GetInterError2
 *
 *  INPUTS        : PB_INSTANCE *pbi   : Pointer to decoder instance.
 *                  UINT8 *NewDataPtr  : Pointer to current block.
 *                  UINT8 *RefDataPtr1 : Pointer to reference block.
 *                  MOTION_VECTOR *MV  : Pointer to motion vector.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Error variance.
 *
 *  FUNCTION      : Calculates a difference error score between two blocks.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
UINT32 GetInterError2 ( PB_INSTANCE *pbi, UINT8 *NewDataPtr, UINT8 *RefDataPtr1, MOTION_VECTOR *MV )
{
	UINT8 *ReconDataPtr1;
	UINT8 *ReconDataPtr2;
	UINT32 err = 0;

	// Get the reference pointers for the motion vector
	GetReconReferencePoints( pbi, RefDataPtr1, &ReconDataPtr1, &ReconDataPtr2, MV );

	// Calculate the variance error score for the vector
	if ( (MV->x & Y_MVMODMASK) || (MV->y & Y_MVMODMASK) )
	{
		err = GetInterErrQPel ( pbi, NewDataPtr, pbi->mbi.blockDxInfo[0].CurrentSourceStride, ReconDataPtr1, ReconDataPtr2, pbi->mbi.blockDxInfo[0].CurrentReconStride, MV);
	}
	else
	{
		err = GetInterError ( NewDataPtr, pbi->mbi.blockDxInfo[0].CurrentSourceStride, ReconDataPtr1, ReconDataPtr2, pbi->mbi.blockDxInfo[0].CurrentReconStride ); 
	}
	return err;
}

/****************************************************************************
 *
 *  ROUTINE       : GetInterError2_slow
 *
 *  INPUTS        : PB_INSTANCE *pbi   : Pointer to decoder instance.
 *                  UINT8 *NewDataPtr  : Pointer to current block.
 *                  UINT8 *RefDataPtr1 : Pointer to reference block.
 *                  MOTION_VECTOR *MV  : Pointer to motion vector.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Error variance.
 *
 *  FUNCTION      : Calculates a difference error score between two blocks.
 *
 *  SPECIAL NOTES : This function works on a loop filtered version of 
 *					the data.
 *
 ****************************************************************************/
UINT32 GetInterError2_slow ( PB_INSTANCE *pbi, UINT8 *NewDataPtr, UINT8 *RefDataPtr1, MOTION_VECTOR *MV )
{
    INT32  ModX, ModY;
	UINT32 err = 0;
	UINT8 *TempPtr1 = pbi->LoopFilteredBlock + (2*16+2);
	UINT8 *TempPtr2 = TempPtr1;

	// This function produces a filtered copy of the appropriate part of the 
	// reconstruction buffer in pbi->LoopFilteredBlock[].
	VP6_PredictFiltered ( pbi, RefDataPtr1, MV->x, MV->y, Y_MVSHIFT );

	// Pull off the fractional bits
	ModX = (MV->x & Y_MVMODMASK);
	ModY = (MV->y & Y_MVMODMASK);

	// Update the second reference pointer inrespect of the fractional X bits.
    if ( ModX )
	{
		if ( MV->x >= 0 )
			TempPtr2 += 1;
		else
			TempPtr2 -= 1;
	}

	// Update the second reference pointer inrespect of the fractional Y bits.
    if ( ModY )
	{
		if ( MV->y > 0 )
			TempPtr2 += 16;
		else
			TempPtr2 -= 16;
	}

	//  If any of the fractional bits are set use GetInterErrQPel() else GetInterError()
	if ( ModX || ModY )
		err = GetInterErrQPel(pbi,NewDataPtr,pbi->mbi.blockDxInfo[0].CurrentSourceStride, TempPtr1,TempPtr2,16, MV );
	else
		err = GetInterError(NewDataPtr,pbi->mbi.blockDxInfo[0].CurrentSourceStride, TempPtr1,TempPtr2,16);

	return err;
}

/****************************************************************************
 *
 *  ROUTINE       : GetInterErr
 *
 *  INPUTS        : UINT8 *NewDataPtr  : Pointer to current block.
 *                  INT32 SourceStride : Stride for NewDataPtr block.
 *                  UINT8 *RefDataPtr1 : Pointer to reference block.
 *                  UINT8 *RefDataPtr2 : Pointer to reference block.
 *                  INT32 ReconStride  : Stride for RefDataPtr1 & RefDataPtr2.
 *
 *  OUTPUTS       : None
 *
 *  RETURNS       : UINT32: Error variance (scaled by 2^12).
 *
 *  FUNCTION      : Calculates the variance of the difference between the
 *                  NewDataPtr block and the average of the RefDataPtr1 & 
 *                  RefDataPtr2 blocks.
 *
 *  SPECIAL NOTES : Computed error variance is multiplied by 2^12 (4096).
 *
 ****************************************************************************/
UINT32 GetInterErr
(
    UINT8 * NewDataPtr,
    INT32 SourceStride,
    UINT8 * RefDataPtr1,
    UINT8 * RefDataPtr2,
    INT32 ReconStride 
)
{
    UINT32  i;
    INT32   XSum=0;
    INT32   XXSum=0;
    INT32   DiffVal;
    INT32   AbsRefOffset = abs((int)(RefDataPtr1 - RefDataPtr2));

    // Mode of interpolation chosen based upon on the offset of the second reference pointer
    if ( AbsRefOffset == 0 )
    {
        for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
        {
            DiffVal = ((int)NewDataPtr[0]) - (int)RefDataPtr1[0];
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[1]) - (int)RefDataPtr1[1];
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[2]) - (int)RefDataPtr1[2];
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[3]) - (int)RefDataPtr1[3];
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[4]) - (int)RefDataPtr1[4];
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[5]) - (int)RefDataPtr1[5];
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[6]) - (int)RefDataPtr1[6];
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[7]) - (int)RefDataPtr1[7];
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            // Step to next row of block.
            NewDataPtr += SourceStride;
            RefDataPtr1 += ReconStride;
        }
    }
    // Simple two reference interpolation
    else
    {
        for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
        {
            DiffVal = ((int)NewDataPtr[0]) - (((int)RefDataPtr1[0] + (int)RefDataPtr2[0]) / 2);
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[1]) - (((int)RefDataPtr1[1] + (int)RefDataPtr2[1]) / 2);
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[2]) - (((int)RefDataPtr1[2] + (int)RefDataPtr2[2]) / 2);
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[3]) - (((int)RefDataPtr1[3] + (int)RefDataPtr2[3]) / 2);
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[4]) - (((int)RefDataPtr1[4] + (int)RefDataPtr2[4]) / 2);
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[5]) - (((int)RefDataPtr1[5] + (int)RefDataPtr2[5]) / 2);
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[6]) - (((int)RefDataPtr1[6] + (int)RefDataPtr2[6]) / 2);
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            DiffVal = ((int)NewDataPtr[7]) - (((int)RefDataPtr1[7] + (int)RefDataPtr2[7]) / 2);
            XSum += DiffVal;
            XXSum += XX_LUT[DiffVal];

            // Step to next row of block.
            NewDataPtr += SourceStride;
            RefDataPtr1 += ReconStride;
            RefDataPtr2 += ReconStride;
        }
    }

    // Compute and return population variance as mis-match metric.
    return (( (XXSum<<6) - XSum*XSum ));
}

/****************************************************************************
 *
 *  ROUTINE       :  GetSumAbsDiffs
 *
 *  INPUTS        :  UINT8 *NewDataPtr  : Pointer to current block.
 *                   INT32 SourceStride : Stride for NewDataPtr block.
 *                   UINT8 *RefDataPtr  : Pointer to reference block.
 *                   INT32 ReconStride  : Stride for RefDataPtr.
 *                   UINT32 ErrorSoFar  : Error for MB so far.
 *                   UINT32 BestSoFar   : Best error found so far.
 *
 *  OUTPUTS       :  None.
 *
 *  RETURNS       :  Sum absolute differences
 *
 *  FUNCTION      :  Calculates the sum of the absolute differences.
 *
 *  SPECIAL NOTES :  ErrorSoFar represents the prediction error sum for
 *                   those blocks within the current MB that have been predicted.
 *                   BestSoFar is used as an early bail-out condition.
 *
 ****************************************************************************/
UINT32 GetSumAbsDiffs
(
    UINT8 * NewDataPtr,
    INT32 SourceStride,
    UINT8  * RefDataPtr,
    INT32 ReconStride,
    UINT32 ErrorSoFar,
    UINT32 BestSoFar
)
{
    UINT32  i;
    UINT32  DiffVal = ErrorSoFar;

    for ( i=0; i < BLOCK_HEIGHT_WIDTH; i++ )
    {
        DiffVal += AbsX_LUT[ ((int)NewDataPtr[0]) - ((int)RefDataPtr[0]) ];
        DiffVal += AbsX_LUT[ ((int)NewDataPtr[1]) - ((int)RefDataPtr[1]) ];
        DiffVal += AbsX_LUT[ ((int)NewDataPtr[2]) - ((int)RefDataPtr[2]) ];
        DiffVal += AbsX_LUT[ ((int)NewDataPtr[3]) - ((int)RefDataPtr[3]) ];
        DiffVal += AbsX_LUT[ ((int)NewDataPtr[4]) - ((int)RefDataPtr[4]) ];
        DiffVal += AbsX_LUT[ ((int)NewDataPtr[5]) - ((int)RefDataPtr[5]) ];
        DiffVal += AbsX_LUT[ ((int)NewDataPtr[6]) - ((int)RefDataPtr[6]) ];
        DiffVal += AbsX_LUT[ ((int)NewDataPtr[7]) - ((int)RefDataPtr[7]) ];

        if ( DiffVal > BestSoFar )
            break;

        // Step to next row of block.
        NewDataPtr += SourceStride;
        RefDataPtr += ReconStride;
    }
    return DiffVal;
}

/****************************************************************************
 *
 *  ROUTINE       :  GetHalfPixelSumAbsDiffs
 *
 *  INPUTS        :  UINT8 *SrcData     : Pointer to current block.
 *                   INT32 SourceStride : Stride for NewDataPtr block.
 *                   UINT8 *RefDataPtr1 : Pointer to first reference block.
 *                   UINT8 *RefDataPtr2 : Pointer to second reference block.
 *                   INT32 ReconStride  : Stride for RefDataPtr1 & RefDataPtr2.
 *                   UINT32 ErrorSoFar  : Error for MB so far.
 *                   UINT32 BestSoFar   : Best error found so far.
 *
 *  OUTPUTS       :  None.
 *
 *  RETURNS       :  UINT32: Sum absolute differences at 1/2 pixel accuracy.
 *
 *  FUNCTION      :  Calculates the sum of the absolute differences against
 *                   half pixel interpolated references.
 *
 *  SPECIAL NOTES :  ErrorSoFar represents the prediction error sum for
 *                   those blocks within the current MB that have been predicted.
 *                   BestSoFar is used as an early bail-out condition.
 *
 ****************************************************************************/
UINT32 GetHalfPixelSumAbsDiffs
(
    UINT8 * SrcData,
    INT32 SourceStride,
    UINT8 * RefDataPtr1,
    UINT8 * RefDataPtr2,
    INT32 ReconStride,
    UINT32 ErrorSoFar,
    UINT32 BestSoFar
)
{

    UINT32  i;
    UINT32  DiffVal = ErrorSoFar;

    for ( i=0; i < BLOCK_HEIGHT_WIDTH; i++ )
    {
        DiffVal += AbsX_LUT[ ((int)SrcData[0]) - (((int)RefDataPtr1[0] + (int)RefDataPtr2[0]) / 2) ];
        DiffVal += AbsX_LUT[ ((int)SrcData[1]) - (((int)RefDataPtr1[1] + (int)RefDataPtr2[1]) / 2) ];
        DiffVal += AbsX_LUT[ ((int)SrcData[2]) - (((int)RefDataPtr1[2] + (int)RefDataPtr2[2]) / 2) ];
        DiffVal += AbsX_LUT[ ((int)SrcData[3]) - (((int)RefDataPtr1[3] + (int)RefDataPtr2[3]) / 2) ];
        DiffVal += AbsX_LUT[ ((int)SrcData[4]) - (((int)RefDataPtr1[4] + (int)RefDataPtr2[4]) / 2) ];
        DiffVal += AbsX_LUT[ ((int)SrcData[5]) - (((int)RefDataPtr1[5] + (int)RefDataPtr2[5]) / 2) ];
        DiffVal += AbsX_LUT[ ((int)SrcData[6]) - (((int)RefDataPtr1[6] + (int)RefDataPtr2[6]) / 2) ];
        DiffVal += AbsX_LUT[ ((int)SrcData[7]) - (((int)RefDataPtr1[7] + (int)RefDataPtr2[7]) / 2) ];

        if ( DiffVal > BestSoFar )
            break;

        // Step to next row of block.
        SrcData += SourceStride;
        RefDataPtr1 += ReconStride;
        RefDataPtr2 += ReconStride;
    }
    return DiffVal;

}
/****************************************************************************
 *
 *  ROUTINE       :     GetIntraErrorC
 *
 *  INPUTS        :     UINT8 *DataPtr      : Pointer to intra block.
 *                      INT32  SourceStride : Block stride.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Intra frame block variance (scaled by 2^12)
 *
 *  FUNCTION      :     Calculates a variance score for the block.
 *
 *  SPECIAL NOTES :     Computed variance value is scaled by 2^12 (4096).
 *
 ****************************************************************************/
UINT32 GetIntraErrorC ( UINT8 *DataPtr, INT32 SourceStride )
{
    UINT32  i;
    UINT32  XSum=0;
    UINT32  XXSum=0;
    UINT8   *DiffPtr;

    DiffPtr = DataPtr;

    // Loop expanded out for speed.
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {
        // Examine alternate pixel locations.
        XSum  += DiffPtr[0];
        XXSum += XX_LUT[DiffPtr[0]];
        XSum  += DiffPtr[1];
        XXSum += XX_LUT[DiffPtr[1]];
        XSum  += DiffPtr[2];
        XXSum += XX_LUT[DiffPtr[2]];
        XSum  += DiffPtr[3];
        XXSum += XX_LUT[DiffPtr[3]];
        XSum  += DiffPtr[4];
        XXSum += XX_LUT[DiffPtr[4]];
        XSum  += DiffPtr[5];
        XXSum += XX_LUT[DiffPtr[5]];
        XSum  += DiffPtr[6];
        XXSum += XX_LUT[DiffPtr[6]];
        XSum  += DiffPtr[7];
        XXSum += XX_LUT[DiffPtr[7]];

        // Step to next row of block.
        DiffPtr += SourceStride;
    }

    // Compute population variance as mis-match metric.
    return ((XXSum<<6) - XSum*XSum);
}

/****************************************************************************
 *
 *  ROUTINE       :  GetSumAbsDiffs16
 *
 *  INPUTS        :  UINT8 *SrcPtr      : Pointer to current block.
 *                   INT32 SourceStride : Stride for SrcPtr block.
 *                   UINT8 *RefPtr      : Pointer to reference block.
 *                   INT32 ReconStride  : Stride for RefPtr.
 *                   UINT32 ErrorSoFar  : Error for MB so far (NOT USED).
 *                   UINT32 BestSoFar   : Best error found so far (NOT USED).
 *
 *  OUTPUTS       :  None.
 *
 *  RETURNS       :  UINT32: SAD for the 16x16 block
 *
 *  FUNCTION      :  Calculates the sum of the absolute differences for 
 *                   the 16x16 block.
 *
 *  SPECIAL NOTES :  None.
 *
 ****************************************************************************/
UINT32 GetSumAbsDiffs16
(
    UINT8 *SrcPtr,
    INT32 SourceStride,
    UINT8 *RefPtr,
    INT32 ReconStride,
    UINT32 ErrorSoFar,
    UINT32 BestSoFar
)
{
    UINT32 Error = 0;

    Error = GetSAD ( SrcPtr,                  SourceStride, RefPtr,                 ReconStride, Error, HUGE_ERROR );
    Error = GetSAD ( SrcPtr+8,                SourceStride, RefPtr+8,               ReconStride, Error, HUGE_ERROR );
    Error = GetSAD ( SrcPtr+8*SourceStride,   SourceStride, RefPtr+8*ReconStride,   ReconStride, Error, HUGE_ERROR );
    Error = GetSAD ( SrcPtr+8*SourceStride+8, SourceStride, RefPtr+8*ReconStride+8, ReconStride, Error, HUGE_ERROR );
    return Error;
}

/****************************************************************************
 *
 *  ROUTINE       :     GetHalfPixelSumAbsDiffs16
 *
 *  INPUTS        :  UINT8 *SrcPtr      : Pointer to current block.
 *                   INT32 SourceStride : Stride for SrcPtr block.
 *                   UINT8 *RefPtr       : Pointer to first reference block.
 *                   UINT8 *RefPtr2      : Pointer to second reference block.
 *                   UINT32 ReconStride  : Stride for RefPtr & RefPtr2. 
 *                   INT32  ErrorSoFar   : Error for MB so far (NOT USED).
 *                   INT32  BestSoFar    : Best error found so far (NOT USED).
 *
 *  OUTPUTS       :  None.
 *
 *  RETURNS       :  UINT32: SAD at 1/2 pixel accuracy.
 *
 *  FUNCTION      :  Calculates the sum of the absolute differences between
 *                   the block pointed to by SrcPtr and the half pixel 
 *                   interpolation block created from RefPtr & RefPtr2.
 *
 *  SPECIAL NOTES :  None.
 *
 ****************************************************************************/
UINT32 GetHalfPixelSumAbsDiffs16
(
    UINT8 *SrcPtr,
    INT32 SourceStride,
    UINT8 *RefPtr,
    UINT8 *RefPtr2,
    INT32 ReconStride,
    UINT32 ErrorSoFar,
    UINT32 BestSoFar
)
{
    UINT32 Error = 0;

    Error = GetSadHalfPixel ( SrcPtr, SourceStride, RefPtr, RefPtr2, ReconStride, Error, HUGE_ERROR );

    Error = GetSadHalfPixel ( SrcPtr+8,SourceStride, RefPtr+8, RefPtr2+8, ReconStride, Error, HUGE_ERROR );

    Error = GetSadHalfPixel ( SrcPtr+8*SourceStride, SourceStride, RefPtr+8*ReconStride ,
        RefPtr2+8*ReconStride, ReconStride, Error, HUGE_ERROR );

    Error = GetSadHalfPixel( SrcPtr+8*SourceStride+8, SourceStride,
        RefPtr+8*ReconStride+8, RefPtr2+8*ReconStride+8,
        ReconStride, Error, HUGE_ERROR );

    return Error;
}

/****************************************************************************
 *
 *  ROUTINE       :  GetMBIntraError
 *
 *  INPUTS        :  CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       :  None.
 *
 *  RETURNS       :  UINT32: Intra-frame variance for the MB.
 *
 *  FUNCTION      :  Calculates the intra-frame variance for the MB.
 *
 *  SPECIAL NOTES :  Only considers the four Y blocks in the MB (chroma
 *                   ignored).
 *
 ****************************************************************************/
UINT32 GetMBIntraError ( CP_INSTANCE *cpi )
{
    UINT32  i;
    UINT32 IntraError = 0;
    PB_INSTANCE *pbi = &cpi->pb;

    // Add together the intra errors for the four Y blocks in the MB
    for ( i=0; i<4; i++ )
        IntraError += GetIntraError( &cpi->yuv1ptr[pbi->mbi.blockDxInfo[i].Source], pbi->mbi.blockDxInfo[i].CurrentSourceStride );
    return IntraError;
}

/****************************************************************************
 *
 *  ROUTINE       :  GetMBInterError
 *
 *  INPUTS        :  CP_INSTANCE *cpi   : Pointer to encoder instance.
 *                   UINT8 *SrcPtr      : Pointer to first block.
 *                   UINT8 *RefPtr      : Pointer to second block.
 *                   MOTION_VECTOR *MV  : Motion vector to be used.
 *
 *  OUTPUTS       :  UINT32 *BlockError : Array to hold individual block variances.
 *
 *  RETURNS       :  UINT32: Inter-frame variance for the MB (scaled by 2^12).
 *
 *  FUNCTION      :  Calculates the variance of the difference between
 *                   the MB pointed to by SrcPtr & the MB found by
 *                   applying MV to RefPtr.
 *
 *  SPECIAL NOTES :  Variance is scaled by 2^12 (4096). Choma is ignored
 *                   when computing the variance.
 *
 ****************************************************************************/
UINT32 GetMBInterError
(
    CP_INSTANCE *cpi,
    UINT8 *SrcPtr,
    UINT8 *RefPtr,
    MOTION_VECTOR *MV,
    UINT32 *BlockError
)
{
    int i;
    PB_INSTANCE *pbi = &cpi->pb;

    for ( i=0; i<4; i++ )
    {
        // Select either GetInterError2 which does not do loop filtering
		// or GetInterError2_slow which does based on speed and profile
		// constraints. 
		if( (cpi->pb.UseLoopFilter == NO_LOOP_FILTER) || 
			(cpi->Speed > 8) || 
			(cpi->pb.VpProfile == SIMPLE_PROFILE) )
		{
	        BlockError[i] = GetInterError2 ( pbi,
		            &SrcPtr[pbi->mbi.blockDxInfo[i].Source],
				    &RefPtr[pbi->mbi.blockDxInfo[i].thisRecon],
				    MV );
		}
		else
		{
	        BlockError[i] = GetInterError2_slow ( pbi,
		            &SrcPtr[pbi->mbi.blockDxInfo[i].Source],
				    &RefPtr[pbi->mbi.blockDxInfo[i].thisRecon],
				    MV );
		}

    }
    return BlockError[0]+BlockError[1]+BlockError[2]+BlockError[3];
}

/****************************************************************************
 *
 *  ROUTINE       : FindMvVia3StepSearch
 *
 *  INPUTS        : CP_INSTANCE *cpi     : Pointer to encoder instance.
 *                  CODING_MODE	Mode     : Coding mode for the block.
 *                  UINT8 *SrcPtr        : Pointer to source block.
 *                  UINT8 *RefPtr        : Pointer to block position in reference frame.
 *                  UINT32 BlockSize     : Size of the block.
 *
 *  OUTPUTS       : MOTION_VECTOR *MV    : Best MV found for block in reference frame.
 *                  UINT8 **BestBlockPtr : Pointer-to-pointer to best blockin ref frame.
 *
 *  RETURNS       : UINT32: SAD error of the best matching block.
 *
 *  FUNCTION      : Finds block in reference frame that best matches the SrcPtr 
 *                  block using a hierarchical search.
 *
 *  SPECIAL NOTES : The actual number of steps in the search varies depending
 *                  on the maximum possible MV size. Motion vectors are
 *                  stored in 1/4 pixel units.
 *
 ****************************************************************************/
UINT32 FindMvVia3StepSearch
(
    CP_INSTANCE *cpi,
	CODING_MODE	Mode,
    UINT8 *SrcPtr,
    UINT8 *RefPtr,
    MOTION_VECTOR *MV,
    UINT8 **BestBlockPtr,
    UINT32 BlockSize
)
{
    INT32  i;
    INT32  step;
	UINT32 EstMvBits;			// Actualy bits * 64
    INT32  SourceStride;
    INT32  ReconStride;
	INT32  FirstStepOffset;
	MOTION_VECTOR DifferentialVector;
    INT32  x=0, y=0;
    INT32  SearchSite=0;
    UINT32 Error = 0;
    UINT32 MinError = HUGE_ERROR;
	INT32  MvOffsetX = 0;
	INT32  MvOffsetY = 0;
    UINT8  *CandidateBlockPtr = NULL;
    PB_INSTANCE *pbi = &cpi->pb;
    UINT32 (*GetSad)( UINT8 * SrcPtr, INT32 SourceStride, UINT8  * RefPtr, INT32 ReconStride, UINT32 ErrorSoFar, UINT32 BestSoFar  );

	// Work out if we will code the vector relative to 0,0 or nearest
	if ( Mode == CODE_INTER_PLUS_MV )
	{
		if ( pbi->mbi.NearestMvIndex < MAX_NEAREST_ADJ_INDEX )
		{
			MvOffsetX = pbi->mbi.NearestInterMVect.x;
			MvOffsetY = pbi->mbi.NearestInterMVect.y;
		}
	}
	else	// Golden frame
	{
		if ( pbi->mbi.NearestGMvIndex < MAX_NEAREST_ADJ_INDEX )
		{
			MvOffsetX = pbi->mbi.NearestGoldMVect.x;
			MvOffsetY = pbi->mbi.NearestGoldMVect.y;
		}
	}

    if (BlockSize == 8)
    {
        GetSad = GetSAD;
//sjlhack -- always assuming y plane
        SourceStride = pbi->mbi.blockDxInfo[0].CurrentSourceStride;
        ReconStride  = pbi->mbi.blockDxInfo[0].CurrentReconStride;
    }
    else
    {
        // get sad 16 function works for a whole macroblock interlaced only if pixels per line
        // works frame wise
        GetSad = GetSAD16;
        ReconStride  = pbi->Configuration.YStride;
        SourceStride = pbi->Configuration.VideoFrameWidth;
    }
    
    // Check the 0,0 candidate.
    Error = GetSad( SrcPtr, SourceStride, RefPtr, ReconStride, 0, HUGE_ERROR );

    MinError = Error;
    *BestBlockPtr = RefPtr;
    x = 0;
    y = 0;
    MV->x = 0;
    MV->y = 0;

	// Set up control of how many steps to take and size of first step
    // For larger images use a longer initial step and hence more search steps
    if ( cpi->pb.Configuration.VideoFrameWidth >= 480 )
    {
		BOOL LongVectorsAllowed= TRUE; 

		if ( LongVectorsAllowed  &&
		     ( (MvOffsetX >= 48) || (MvOffsetX <= -48) || (MvOffsetY >= 48) || (MvOffsetY <= -48) )  )
		{
			FirstStepOffset = 0;
		}
		else if ( (MvOffsetX >= 16) || (MvOffsetX <= -16) || (MvOffsetY >= 16) || (MvOffsetY <= -16) )
			FirstStepOffset = 1;
		else 
			FirstStepOffset = 2;
	}			
    else if ( cpi->pb.Configuration.VideoFrameWidth >= 320 )
	{
		if ( (MvOffsetX >= 16) || (MvOffsetX <= -16) || (MvOffsetY >= 16) || (MvOffsetY <= -16) )
			FirstStepOffset = 1;
		else 
			FirstStepOffset = 2;
	}
    else
	{
		if ( (MvOffsetX >= 16) || (MvOffsetX <= -16) || (MvOffsetY >= 16) || (MvOffsetY <= -16) )
			FirstStepOffset = 1;
		else
			FirstStepOffset = 2;
	}
	SearchSite = FirstStepOffset * 8;

    // Proceed through the appropriate number of steps.
    for (  step=FirstStepOffset; step<cpi->MVSearchSteps; step++ )
    {
        // Search the 8-neighbours at distance pertinent to current step.
        for ( i=0; i<8; i++ )
        {
            // Set pointer to next candidate matching block.
            CandidateBlockPtr = RefPtr + cpi->MVPixelOffsetY[SearchSite];

            // Get the block error score.
            Error = GetSad( SrcPtr, SourceStride, CandidateBlockPtr,ReconStride,0, MinError );

			// Calculate differential vector in Qpel units
			DifferentialVector.x = (4 * (MV->x + cpi->MVOffsetX[SearchSite])) -	MvOffsetX;
			DifferentialVector.y = (4 * (MV->y + cpi->MVOffsetY[SearchSite])) - MvOffsetY;

			EstMvBits = cpi->EstMvCostPtrX[DifferentialVector.x]
				      + cpi->EstMvCostPtrY[DifferentialVector.y];

			Error += (EstMvBits * MVEPBSAD_MULT)>>MVEPBSAD_RSHIFT;
			Error += (EstMvBits * Error)>>MVEPBSAD_RSHIFT2;

            if ( Error < MinError )
            {
                // Remember best match.
                MinError = Error;
                *BestBlockPtr = CandidateBlockPtr;

                // Where is it.
                x = MV->x + cpi->MVOffsetX[SearchSite];
                y = MV->y + cpi->MVOffsetY[SearchSite];
            }

            // Move to next search location.
            SearchSite += 1;
        }

        // Move to best location this step.
        RefPtr = *BestBlockPtr;
        MV->x = x;
        MV->y = y;
    }

    // Factor vectors to 1/4 pixel resoultion.
    MV->x = (MV->x * 4);
    MV->y = (MV->y * 4);

	TotError += MinError;
	ErrCount++;

    return MinError;
}

/****************************************************************************
 *
 *  ROUTINE       : FindMvViaExhaustSearch
 *
 *  INPUTS        : CP_INSTANCE *cpi     : Pointer to encoder instance.
 *                  CODING_MODE	Mode     : Coding mode for the block.
 *                  UINT8 *SrcPtr        : Pointer to source block.
 *                  UINT8 *RefPtr        : Pointer to block position in reference frame.
 *                  UINT32 BlockSize     : Size of the block.
 *
 *  OUTPUTS       : MOTION_VECTOR *MV    : Best MV found for block in reference frame.
 *                  UINT8 **BestBlockPtr : Pointer-to-pointer to best blockin ref frame.
 *
 *  RETURNS       : UINT32: SAD error of the best matching block.
 *
 *  FUNCTION      : Finds block in reference frame that best matches the SrcPtr 
 *                  block using an exhaustive search.
 *
 *  SPECIAL NOTES : Motion vectors are stored in 1/4 pixel units.
 *                  
 ****************************************************************************/
UINT32 FindMvViaExhaustSearch
(
    CP_INSTANCE *cpi,
	CODING_MODE	Mode,
    UINT8 *SrcPtr,
    UINT8 *RefPtr,
    MOTION_VECTOR *MV,
    UINT8 **BestBlockPtr,
    UINT32 BlockSize
)
{
    INT32  i,j;
    UINT32 Error;
	UINT32 EstMvBits;			  // Actualy bits * 64
	INT32  MvMaxExtent; 
	INT32  HalfMvMaxExtent; 
    INT32  SourceStride;
    INT32  ReconStride;
	MOTION_VECTOR ThisMv;
	MOTION_VECTOR DifferentialVector;
	INT32  MvOffsetX = 0;
	INT32  MvOffsetY = 0;
    UINT32 MinError = HUGE_ERROR;
    UINT8  *CandidateBlockPtr=NULL;
    PB_INSTANCE *pbi = &cpi->pb;
    UINT32 (*GetSad)( UINT8 * SrcPtr, INT32 SourceStride, UINT8  * RefPtr, INT32 ReconStride, UINT32 ErrorSoFar, UINT32 BestSoFar  );

	// Work out if we will code the vector relative to 0,0 or nearest
	if ( Mode == CODE_INTER_PLUS_MV )
	{
		if ( pbi->mbi.NearestMvIndex < MAX_NEAREST_ADJ_INDEX )
		{
			MvOffsetX = pbi->mbi.NearestInterMVect.x;
			MvOffsetY = pbi->mbi.NearestInterMVect.y;
		}
	}
	else	// Golden frame
	{
		if ( pbi->mbi.NearestGMvIndex < MAX_NEAREST_ADJ_INDEX )
		{
			MvOffsetX = pbi->mbi.NearestGoldMVect.x;
			MvOffsetY = pbi->mbi.NearestGoldMVect.y;
		}
	}

    // For larger images allow a longer search
	// NOTE: 
    // MvOffsetX and MvOffsetY are in 1/4 pel units.
    if ( cpi->pb.Configuration.VideoFrameWidth >= 480 )
	{
	    MvMaxExtent =  63;				
	}
    else if ( cpi->pb.Configuration.VideoFrameWidth >= 320 )
	{
		// Consider the length of the nearest X and Y
        MvMaxExtent =  31;				
	}
    else
        MvMaxExtent =  31;				

	HalfMvMaxExtent =  MvMaxExtent/2;

    if (BlockSize == 8)
    {
        GetSad = GetSAD;
//sjlhack -- always assuming y plane
        SourceStride = pbi->mbi.blockDxInfo[0].CurrentSourceStride;
        ReconStride  = pbi->mbi.blockDxInfo[0].CurrentReconStride;
    }
    else
    {
        // get sad 16 function works for a whole macroblock interlaced only if pixels per line
        // works frame wise
        GetSad = GetSAD16;
        ReconStride  = pbi->Configuration.YStride;
        SourceStride = pbi->Configuration.VideoFrameWidth;
    }

    RefPtr = RefPtr - (HalfMvMaxExtent * pbi->Configuration.YStride) - HalfMvMaxExtent;

    // Search each pixel alligned site
    for ( i=0; i<(INT32)MvMaxExtent; i++ )
    {
        // Starting position in row
        CandidateBlockPtr = RefPtr;

        for ( j=0; j<(INT32)MvMaxExtent; j++ )
        {
			// *4 converts to 1/4 pixel resolution
			ThisMv.x = 4 * (j - HalfMvMaxExtent);
			ThisMv.y = 4 * (i - HalfMvMaxExtent);

			// Get the block error score.
            Error = GetSad( SrcPtr, SourceStride, CandidateBlockPtr, ReconStride,0, HUGE_ERROR );

			// Should we code relative to 0,0 or nearest
		    DifferentialVector.x = ThisMv.x - MvOffsetX;
			DifferentialVector.y = ThisMv.y - MvOffsetY;

			EstMvBits = cpi->EstMvCostPtrX[DifferentialVector.x]
				      + cpi->EstMvCostPtrY[DifferentialVector.y];

			Error += (EstMvBits * MVEPBSAD_MULT)>>MVEPBSAD_RSHIFT;
			Error += (EstMvBits * Error)>>MVEPBSAD_RSHIFT2;

            // Was this the best so far
            if ( Error < MinError )
            {
                MinError = Error;
                *BestBlockPtr = CandidateBlockPtr;
                MV->x = ThisMv.x;
                MV->y = ThisMv.y;
            }

            // Move the the next site
            CandidateBlockPtr++;
        }

        // Move on to the next row.
        RefPtr += pbi->Configuration.YStride;
    }
    return MinError;
}


/****************************************************************************
 *
 *  ROUTINE       : FindBestFractionalPixelStep
 *
 *  INPUTS        : CP_INSTANCE *cpi     : Pointer to encoder instance.
 *                  CODING_MODE	Mode     : Coding mode for the block.
 *                  UINT8 *SrcPtr        : Pointer to source block.
 *                  UINT8 *RefPtr        : Pointer to block position in reference frame.
 *                  UINT32 BlockSize     : Size of the block.
 *                  UINT32 *MinError     : Pointer to best error found to date.
 *                  UINT8 BitShift       : Number of its to shift the MV components
 *                                         by (depending whether 1/2 or 1/4 pel search)
 *
 *  OUTPUTS       : MOTION_VECTOR *MV    : Best MV found for block in reference frame.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Finds the best fractional (1/2 or 1/4) pixel MV that
 *                  gives the best matching block in the refernce frame.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void FindBestFractionalPixelStep
(
    CP_INSTANCE *cpi,
	CODING_MODE	Mode,
    UINT8 *SrcPtr,
    UINT8 *RefPtr,
    MOTION_VECTOR *MV,
    UINT32 BlockSize,
    UINT32 *MinError,
	UINT8  BitShift
)
{
    UINT32 i, j;
    UINT32 nBlocks;
    INT32  ModX, ModY;
	UINT32 EstMvBits;					// bits * 64
    INT32  SourceStride;
    INT32  ReconStride;
    INT32  BlockOffset[4];
    UINT8 *SourceBlock[4];
    UINT8 *RefDataPtr1;
    UINT8 *RefDataPtr2;
	MOTION_VECTOR DifferentialVector;

    UINT32 Error = 0;
    UINT8  BestOffset = 0;
	INT32  MvOffsetX = 0;
	INT32  MvOffsetY = 0;
    MOTION_VECTOR TmpVector = {0, 0};
    PB_INSTANCE *pbi = &cpi->pb;

	// Work out if we will code the vector relative to 0,0 or nearest
	if ( Mode == CODE_INTER_PLUS_MV )
	{
		if ( pbi->mbi.NearestMvIndex < MAX_NEAREST_ADJ_INDEX )
		{
			MvOffsetX = pbi->mbi.NearestInterMVect.x;
			MvOffsetY = pbi->mbi.NearestInterMVect.y;
		}
	}
	else	// Golden frame
	{
		if ( pbi->mbi.NearestGMvIndex < MAX_NEAREST_ADJ_INDEX )
		{
			MvOffsetX = pbi->mbi.NearestGoldMVect.x;
			MvOffsetY = pbi->mbi.NearestGoldMVect.y;
		}
	}

//sjlhack -- always assuming y plane
    SourceStride = pbi->mbi.blockDxInfo[0].CurrentSourceStride;
    ReconStride = pbi->mbi.blockDxInfo[0].CurrentReconStride;
    if (BlockSize == 8)
    {  
        // Only 1 block to process
        nBlocks = 1;
        BlockOffset[0] = 0;
        SourceBlock[0] = SrcPtr;
    }
    else
    {
        // 4 8x8s to process--may be interlaced!
		nBlocks = 4;
		if ( pbi->mbi.Interlaced == 1 )
		{
			SourceBlock[0] = SrcPtr;
			SourceBlock[1] = SrcPtr + 8;
			SourceBlock[2] = SrcPtr + pbi->Configuration.VideoFrameWidth;
			SourceBlock[3] = SourceBlock[2] + 8;

			BlockOffset[0] = 0;
			BlockOffset[1] = 8;
			BlockOffset[2] = pbi->Configuration.YStride - 8;
			BlockOffset[3] = 8;
		}
		else
		{
			SourceBlock[0] = SrcPtr;
			SourceBlock[1] = SrcPtr + 8;
			SourceBlock[2] = SrcPtr + (8*pbi->Configuration.VideoFrameWidth);
			SourceBlock[3] = SourceBlock[2] + 8;

			BlockOffset[0] = 0;
			BlockOffset[1] = 8;
			BlockOffset[2] = (8 * pbi->Configuration.YStride) - 8;
			BlockOffset[3] = 8;
		}
    }

    // Examine eight positions around a central position
    for ( i = 1; i < 9; i++ )
	{
        // MV holds best mv in 1/4 pixel units
		TmpVector.x = MV->x + (cpi->SubPixelXOffset[i] << BitShift);
		TmpVector.y = MV->y + (cpi->SubPixelYOffset[i] << BitShift);

		// Get the two reference pointers for the motion vector
		GetReconReferencePoints( pbi, RefPtr, &RefDataPtr1, &RefDataPtr2, &TmpVector );
        
        // Filter number is based on 1/8th pixel positions
		ModX = (TmpVector.x & Y_MVMODMASK) << 1;
		ModY = (TmpVector.y & Y_MVMODMASK) << 1;

        // Ptr1 & Ptr2 are current frame and fractional pel filtered block respectively
        Error = 0;

        for ( j=0; j<nBlocks; j++ )
        {
            //UINT32 error1, error2;
            RefDataPtr1 += BlockOffset[j];
            RefDataPtr2 += BlockOffset[j];

            Error += FiltBlockBilGetSad(SourceBlock[j], SourceStride, RefDataPtr1, RefDataPtr2, ReconStride, ModX, ModY,HUGE_ERROR);
        }
		
        // Should we code relative to 0,0 or nearest
		DifferentialVector.x = TmpVector.x - MvOffsetX;
		DifferentialVector.y = TmpVector.y - MvOffsetY;

		EstMvBits = cpi->EstMvCostPtrX[DifferentialVector.x]
				  + cpi->EstMvCostPtrY[DifferentialVector.y];

		Error += (EstMvBits * MVEPBSAD_MULT)>>MVEPBSAD_RSHIFT;
		Error += (EstMvBits * Error)>>MVEPBSAD_RSHIFT2;

		if ( Error < *MinError )
		{
			BestOffset = (UINT8)i;
			*MinError = Error;
		}
	}

    // Set the returned vector
    MV->x += (cpi->SubPixelXOffset[BestOffset] << BitShift);
    MV->y += (cpi->SubPixelYOffset[BestOffset] << BitShift);

    return;
}

/****************************************************************************
 *
 *  ROUTINE       : GetMBMVInterError
 *
 *  INPUTS        : CP_INSTANCE *cpi     : Pointer to encoder instance.
 *                  CODING_MODE	Mode     : Coding mode for the block.
 *                  UINT8 *RefPtr        : Pointer to block position in reference frame.
 *
 *  OUTPUTS       : MOTION_VECTOR *MV    : Best MV found for block in reference frame.
 *                  UINT32 *TempErrors   : Array to hold variances of individual Y-blocks.
 *
 *  RETURNS       : UINT32: Prediction error variance for best matching block.
 *
 *  FUNCTION      : Calculates a MB MV using a heirachical search.
 *
 *  SPECIAL NOTES : Returned variance is scaled by 2^12 (4096).
 *
 ****************************************************************************/
UINT32 GetMBMVInterError
(
    CP_INSTANCE *cpi,
	CODING_MODE	Mode,
    UINT8 *RefFramePtr,
    MOTION_VECTOR *MV,
    UINT32 *TempErrors
)
{
    UINT32  MinError;
    UINT32  InterMVError = 0;

    PB_INSTANCE *pbi=&cpi->pb;
//sjlhack -- always assuming y plane
    UINT8   *SrcPtr = &cpi->yuv1ptr[pbi->mbi.blockDxInfo[0].Source];
    UINT8   *RefPtr = &RefFramePtr[pbi->mbi.blockDxInfo[0].thisRecon];

    UINT8   *BestBlockPtr=NULL;
    
    MinError = cpi->FindMvViaSearch ( cpi, Mode, SrcPtr,RefPtr,MV, &BestBlockPtr,16);
        
 	if ( MinError > HP_THRESH )
	   cpi->FindBestHalfPixelMv ( cpi, Mode, SrcPtr, RefPtr,  MV,  16, &MinError, 1 );
    
    if ( MinError > HP_THRESH )
	   cpi->FindBestQuarterPixelMv( cpi, Mode, SrcPtr, RefPtr,  MV,  16, &MinError, 0 );
    
#if defined(_MSC_VER)
	ClearSysState();
#endif

    // Get the error score for the chosen 1/2 pixel offset as a variance.
    InterMVError = GetMBInterError( cpi, cpi->yuv1ptr, RefFramePtr, MV, TempErrors );

    // Return score of best matching block.
    return InterMVError;
}

/****************************************************************************
 *
 *  ROUTINE       : GetMBMVExhaustiveSearch
 *
 *  INPUTS        : CP_INSTANCE *cpi     : Pointer to encoder instance.
 *                  CODING_MODE	Mode     : Coding mode for the block.
 *                  UINT8 *RefPtr        : Pointer to block position in reference frame.
 *
 *  OUTPUTS       : MOTION_VECTOR *MV    : Best MV found for block in reference frame.
 *                  UINT32 *TempErrors   : Array to hold variances of individual Y-blocks.
 *
 *  RETURNS       : UINT32: Prediction error variance for best matching block.
 *
 *  FUNCTION      : Calculates a MB MV using an exhaustive search.
 *
 *  SPECIAL NOTES : Returned variance is scaled by 2^12 (4096).
 *
 ****************************************************************************/
UINT32 GetMBMVExhaustiveSearch
(
    CP_INSTANCE *cpi,
	CODING_MODE	Mode,
    UINT8 *RefFramePtr,
    MOTION_VECTOR *MV,    
    UINT32 *TempErrors
)
{
    UINT32  MinError;
    UINT32  InterMVError = 0;

    PB_INSTANCE *pbi=&cpi->pb;
//sjlhack -- always assuming y plane
    UINT8   *SrcPtr = &cpi->yuv1ptr[pbi->mbi.blockDxInfo[0].Source];
    UINT8   *RefPtr = &RefFramePtr[pbi->mbi.blockDxInfo[0].thisRecon];
    UINT8   *BestBlockPtr=NULL;

	MinError = FindMvViaExhaustSearch( cpi, Mode, SrcPtr,RefPtr,MV,  &BestBlockPtr,16);

    if ( MinError > HP_THRESH )
		cpi->FindBestHalfPixelMv ( cpi, Mode, SrcPtr, RefPtr,  MV,  16, &MinError, 1 );
    
	if ( MinError > HP_THRESH )
	    cpi->FindBestQuarterPixelMv( cpi, Mode, SrcPtr, RefPtr,  MV,  16, &MinError, 0 );
    
#if defined(_MSC_VER)
	ClearSysState();
#endif

    // Get the error score for the chosen 1/2 pixel offset as a variance.
    InterMVError = GetMBInterError( cpi, cpi->yuv1ptr, RefFramePtr, MV, TempErrors );

    // Return score of best matching block.
    return InterMVError;
}

/****************************************************************************
 *
 *  ROUTINE       : GetBMVExhaustiveSearch
 *
 *  INPUTS        : CP_INSTANCE *cpi  : Pointer to encoder instance.
 *                  UINT8 *RefPtr     : Pointer to block position in reference frame.
 *
 *  OUTPUTS       : MOTION_VECTOR *MV : Best MV found for block in reference frame.
 *
 *  RETURNS       : UINT32: Prediction error variance for best matching block.
 *
 *  FUNCTION      : Calculates a MV for an 8x8 Y block using an exhaustive search.
 *
 *  SPECIAL NOTES : Returned variance is scaled by 2^12 (4096).
 *
 ****************************************************************************/
UINT32 GetBMVExhaustiveSearch ( CP_INSTANCE *cpi, UINT8 *RefFramePtr, MOTION_VECTOR *MV, UINT32 bp )
{
    UINT32  MinError;
    UINT32  InterMVError = 0;

    PB_INSTANCE *pbi = &cpi->pb;

    UINT8   *SrcPtr = &cpi->yuv1ptr[pbi->mbi.blockDxInfo[bp].Source];
    UINT8   *RefPtr = &RefFramePtr[pbi->mbi.blockDxInfo[bp].thisRecon];
    UINT8   *BestBlockPtr = NULL;

	MinError = FindMvViaExhaustSearch( cpi, CODE_INTER_PLUS_MV, SrcPtr,RefPtr,MV, &BestBlockPtr,8);

	if ( MinError > HP_THRESH )
		cpi->FindBestHalfPixelMv ( cpi, CODE_INTER_PLUS_MV, SrcPtr, RefPtr,  MV, 8, &MinError, 1 );
    
	if ( MinError > HP_THRESH )
		cpi->FindBestQuarterPixelMv( cpi, CODE_INTER_PLUS_MV, SrcPtr, RefPtr,  MV, 8, &MinError, 0 );
    
    InterMVError = GetInterError2( pbi, SrcPtr, RefPtr, MV );

    // Return score of best matching block.
    return InterMVError;
}

/****************************************************************************
 *
 *  ROUTINE       : GetBMVSearch
 *
 *  INPUTS        : CP_INSTANCE *cpi   : Pointer to encoder instance.
 *                  UINT8 *RefFramePtr : Pointer to block position in reference frame.
 *
 *  OUTPUTS       : MOTION_VECTOR *MV  : Best MV found for block in reference frame.
 *
 *  RETURNS       : UINT32: Prediction error variance for best matching block.
 *
 *  FUNCTION      : Calculates a MV for an 8x8 Y block using an exhaustive search.
 *
 *  SPECIAL NOTES : Returned variance is scaled by 2^12 (4096).
 *
 ****************************************************************************/
UINT32 GetBMVSearch ( CP_INSTANCE *cpi, UINT8 *RefFramePtr, MOTION_VECTOR *MV, UINT32 bp )
{
    UINT32  MinError;
    UINT32  InterMVError = 0;

    PB_INSTANCE *pbi=&cpi->pb;

    UINT8   *SrcPtr = &cpi->yuv1ptr[pbi->mbi.blockDxInfo[bp].Source];
    UINT8   *RefPtr = &RefFramePtr[pbi->mbi.blockDxInfo[bp].thisRecon];
    UINT8   *BestBlockPtr=NULL;


    MinError = cpi->FindMvViaSearch( cpi, CODE_INTER_PLUS_MV, SrcPtr,RefPtr, MV, &BestBlockPtr, 8);
    
    
	if ( MinError > HP_THRESH )
		cpi->FindBestHalfPixelMv ( cpi, CODE_INTER_PLUS_MV, SrcPtr, RefPtr,  MV, 8, &MinError, 1 );

    
	if ( MinError > HP_THRESH )
		cpi->FindBestQuarterPixelMv( cpi, CODE_INTER_PLUS_MV, SrcPtr, RefPtr,  MV, 8, &MinError, 0 );
    

    InterMVError = GetInterError2( pbi, SrcPtr, RefPtr, MV );


    // Return score of best matching block.
    return InterMVError;
}

/****************************************************************************
 *
 *  ROUTINE       : FindMvViaDiamondSearch
 *
 *  INPUTS        : CP_INSTANCE *cpi     : Pointer to encoder instance.
 *                  CODING_MODE Mode     : Coding mode for the block.
 *                  UINT8 *SrcPtr        : Pointer to block in source image.
 *                  UINT8 *RefPtr        : Pointer to block in reference image.
 *                  UINT32 BlockSize     : Size of block.
 *
 *  OUTPUTS       : MOTION_VECTOR *MV    : Motion vector of best block found.
 *                  UINT8 **BestBlockPtr : Pointer-to-pointer of best block found.
 *
 *  RETURNS       : UINT32: SAD for the best matching block found.
 *
 *  FUNCTION      : Calculates a MV using a diamond search.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
UINT32 FindMvViaDiamondSearch
(
    CP_INSTANCE *cpi,
    CODING_MODE Mode,
    UINT8 *SrcPtr,
    UINT8 *RefPtr,
    MOTION_VECTOR *MV,
    UINT8 **BestBlockPtr,
    UINT32 BlockSize
)
{
    INT32  i;
    INT32  step;
	UINT32 EstMvBits;			// Actualy bits * 64
    INT32  SourceStride;
    INT32  ReconStride;
	INT32  FirstStepOffset;
    MOTION_VECTOR DifferentialVector;

    INT32  x=0, y=0;
    UINT32 Error = 0;
    UINT32 MinError = HUGE_ERROR;
	INT32  MvOffsetX = 0;
	INT32  MvOffsetY = 0;
    INT32  SearchSite = 0;
    UINT8  *CandidateBlockPtr = NULL;
    PB_INSTANCE *pbi = &cpi->pb;
    UINT32 (*GetSad)( UINT8 * SrcPtr, INT32 SourceStride, UINT8  * RefPtr, INT32 ReconStride, UINT32 ErrorSoFar, UINT32 BestSoFar  );

	// Work out if we will code the vector relative to 0,0 or nearest
	if ( Mode == CODE_INTER_PLUS_MV )
	{
		if ( pbi->mbi.NearestMvIndex < MAX_NEAREST_ADJ_INDEX )
		{
			MvOffsetX = pbi->mbi.NearestInterMVect.x;
			MvOffsetY = pbi->mbi.NearestInterMVect.y;
		}
	}
	else	// Golden frame
	{
		if ( pbi->mbi.NearestGMvIndex < MAX_NEAREST_ADJ_INDEX )
		{
			MvOffsetX = pbi->mbi.NearestGoldMVect.x;
			MvOffsetY = pbi->mbi.NearestGoldMVect.y;
		}
	}


    if ( BlockSize == 8 )
    {
        GetSad = GetSAD;
//sjlhack -- always assuming y plane
        SourceStride = pbi->mbi.blockDxInfo[0].CurrentSourceStride;
        ReconStride  = pbi->mbi.blockDxInfo[0].CurrentReconStride;
    }
    else
    {
        // get sad 16 function works for a whole macroblock interlaced only if pixels per line
        // works frame wise
        GetSad = GetSAD16;
        ReconStride  = pbi->Configuration.YStride;
        SourceStride = pbi->Configuration.VideoFrameWidth;
    }
    
    // Check the 0,0 candidate.
    Error = GetSad( SrcPtr, SourceStride, RefPtr, ReconStride, 0, HUGE_ERROR );

    MinError = Error;
    *BestBlockPtr = RefPtr;
    x = 0;
    y = 0;
    MV->x = 0;
    MV->y = 0;

	// Set up control of how many steps to take and size of first step
    // For larger images use a longer initial step and hence more search steps
    if ( cpi->pb.Configuration.VideoFrameWidth >= 480 )
    {
		BOOL LongVectorsAllowed = TRUE; 

		if ( LongVectorsAllowed  &&
		     ( (MvOffsetX >= 48) || (MvOffsetX <= -48) || (MvOffsetY >= 48) || (MvOffsetY <= -48) )  )
		{
			FirstStepOffset = 0;
		}
		else if ( (MvOffsetX >= 16) || (MvOffsetX <= -16) || (MvOffsetY >= 16) || (MvOffsetY <= -16) )
			FirstStepOffset = 1;
		else 
			FirstStepOffset = 2;
	}			
    else if ( cpi->pb.Configuration.VideoFrameWidth >= 320 )
	{
		if ( (MvOffsetX >= 16) || (MvOffsetX <= -16) || (MvOffsetY >= 16) || (MvOffsetY <= -16) )
			FirstStepOffset = 1;
		else 
			FirstStepOffset = 2;
	}
    else
	{
		if ( (MvOffsetX >= 16) || (MvOffsetX <= -16) || (MvOffsetY >= 16) || (MvOffsetY <= -16) )
			FirstStepOffset = 1;
		else
			FirstStepOffset = 2;
	}

	SearchSite = FirstStepOffset * 4;

    // Proceed through N-steps.
    for (  step=FirstStepOffset; step<cpi->DSMVSearchSteps; step++ )
    {
        // Search the 4-neighbours at distance pertinent to current step.
        for ( i=0; i<4; i++ )
        {
            // Set pointer to next candidate matching block.
            CandidateBlockPtr = RefPtr + cpi->DSMVPixelOffsetY[SearchSite];

            // Get the block error score.
            Error = GetSad( SrcPtr, SourceStride, CandidateBlockPtr,ReconStride,0, MinError );

			// Calculate differential vector in Qpel units
			DifferentialVector.x = (4 * (MV->x + cpi->MVOffsetX[SearchSite])) -	MvOffsetX;
			DifferentialVector.y = (4 * (MV->y + cpi->MVOffsetY[SearchSite])) - MvOffsetY;

			EstMvBits = cpi->EstMvCostPtrX[DifferentialVector.x]
				      + cpi->EstMvCostPtrY[DifferentialVector.y];

			Error += (EstMvBits * MVEPBSAD_MULT)>>MVEPBSAD_RSHIFT;
			Error += (EstMvBits * Error)>>MVEPBSAD_RSHIFT2;

            if ( Error < MinError )
            {
                // Remember best match.
                MinError = Error;
                *BestBlockPtr = CandidateBlockPtr;

                // Where is it.
                x = MV->x + cpi->DSMVOffsetX[SearchSite];
                y = MV->y + cpi->DSMVOffsetY[SearchSite];
            }

            // Move to next search location.
            SearchSite += 1;
        }

        // Move to best location this step.
        RefPtr = *BestBlockPtr;
        MV->x = x;
        MV->y = y;
    }
    // Factor vectors to 1/4 pixel resoultion.
    MV->x = (MV->x * 4);
    MV->y = (MV->y * 4);

	TotError += MinError;
	ErrCount++;

    return MinError;
}

/****************************************************************************
 *
 *  ROUTINE       : SkipFractionalPixelStep
 *
 *  INPUTS        : CP_INSTANCE *cpi     : Pointer to encoder instance (NOT USED).
 *                  CODING_MODE	Mode     : Coding mode for the block (NOT USED).
 *                  UINT8 *SrcPtr        : Pointer to source block (NOT USED).
 *                  UINT8 *RefPtr        : Pointer to block position in reference frame (NOT USED).
 *                  UINT32 BlockSize     : Size of the block (NOT USED).
 *                  UINT32 *MinError     : Pointer to best error found to date (NOT USED).
 *                  UINT8 BitShift       : Number of its to shift the MV components 
 *                                         by (depending whether 1/2 or 1/4 pel search)(NOT USED).
 *
 *  OUTPUTS       : MOTION_VECTOR *MV    : Best MV found for block in reference frame.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Stub function to avoid fractional pixel MV search.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void SkipFractionalPixelStep
(
    CP_INSTANCE *cpi,
	CODING_MODE	Mode,
    UINT8 *SrcPtr,
    UINT8 *RefPtr,
    MOTION_VECTOR *MV,
    UINT32 BlockSize,
    UINT32 *MinError,
	UINT8  BitShift
)
{
    // stub function 
    return;
}

/****************************************************************************
 *
 *  ROUTINE       : FiltBlockBilGetSad_C
 *
 *  INPUTS        : UINT8 *SrcPtr       : Pointer to source block.
 *                  INT32 SrcStride     : Stride of source image.
 *                  UINT8 *ReconPtr1    : Pointer to first block position in reference frame.
 *                  UINT8 *ReconPtr2    : Pointer to second block position in reference frame.
 *                  INT32 PixelsPerLine : Pixels in line of frame containing ReconPtr1/2.
 *                  INT32 ModX          : Fractional part of MV x-component.
 *                  INT32 ModY          : Fractional part of MV x-component.
 *                  UINT32 BestSoFar    : Best error found to date.
 *
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: SAD of the filtered block prediction error.
 *
 *  FUNCTION      : Produces a filtered fractional pel prediction block
 *  				using bi-linear filters and calculates the SAD of
 *                  the prediction error.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
UINT32 FiltBlockBilGetSad_C
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
    // AWG This array name masks array of same name at file scope!!! BEWARE!!!
    UINT8 FilteredBlock[256];

    FilterBlockBil_8 ( ReconPtr1, ReconPtr2, FilteredBlock, PixelsPerLine, ModX, ModY );    
    return GetSAD (  SrcPtr, SrcStride, FilteredBlock, 8, 0, BestSoFar );
}
