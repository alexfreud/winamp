/****************************************************************************
*
*   Module Title :     SCAN_YUV
*
*   Description  :     Content analysis and scoring functions for YUV 411. .
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
*   1.12 PGW 27 Apr 01 Changes to use last frame coded list passed in from codec.
*   1.11 PGW 28 Feb 01 Removal of requirement for a seperate pre-processor output buffer.
*   1.10 PGW 04 Oct 00 Bug fixes to SadPass2() and changes to how it is called.
*					   Changes to ConsolidateDiffScanResults()
*   1.09 PGW 29 Aug 00 Correction to defaults in SetVcapLevelOffset()
*   1.08 JBB 03 Aug 00 Cleaned up a bit (memset full buffer)
*                      Fixed Problem with Pak Filter wrapping over edges
*   1.07 PGW 24 Jul 00 Added column scan funtion. Experiment with PAK off.
*                      Tweaks to filter thresholds.
*   1.06 PGW 10 Jul 00 Changes to RowDiffScan() to reduce number of conditionals.
*   1.05 PGW 22/06/00  Filtering threshold tweaks.
*   1.04 JBB 30/05/00  Removed hard coded size limits
*	1.03 YX	 13/04/00  Comment out some if() testings 
*   1.02 PGW 16/03/00  Changes to SetVcapLevelOffset() to provide 
*                      various pre-set filter levels. 
*   1.01 PGW 12/07/99  Changes to reduce uneccessary dependancies. 
*   1.00 PGW 14/06/99  Configuration baseline
*
*****************************************************************************
*/						

/****************************************************************************
*  Header Frames
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "preproc.h"

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        

#define MIN_STEP_THRESH 6


#define SCORE_MULT_LOW    0.5
#define SCORE_MULT_MEDIUM 2.0
#define SCORE_MULT_HIGH   4

/****************************************************************************
*  Explicit Imports
*****************************************************************************
*/


extern void ClearMmxState(PP_INSTANCE *ppi);

/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/

UINT32 LineLengthScores[ MAX_SEARCH_LINE_LEN + 1 ] = { 0, 0, 0, 0, 2, 4, 12, 24 };
UINT32 BodyNeighbourScore = 8;
double DiffDevisor = 0.0625; // 1/16
UINT8  LineSearchTripTresh = 16;
double LowVarianceThresh = 200.0;

/****************************************************************************
*  Foreward References
*****************************************************************************
*/              

BOOL RowSadScan( PP_INSTANCE *ppi, UINT8 * YuvPtr1, UINT8 * YuvPtr2, INT8 *  DispFragPtr );
BOOL ColSadScan( PP_INSTANCE *ppi, UINT8 * YuvPtr1, UINT8 * YuvPtr2, INT8 *  DispFragPtr );

void RowDiffScan( PP_INSTANCE *ppi, UINT8 * YuvPtr1, UINT8 * YuvPtr2, 
                  INT16 * YUVDiffPtr, UINT8 * bits_map_ptr, 
                  INT8 * SgcPtr, INT8  * DispFragPtr, 
				  UINT8 * FDiffPixels, INT32 * RowDiffsPtr, 
                  UINT8 * ChLocalsPtr,  BOOL EdgeRow );

void SadPass2( PP_INSTANCE *ppi, INT32 RowNumber, INT8 *  DispFragPtr );

void ConsolidateDiffScanResults( PP_INSTANCE *ppi, UINT8 * FDiffPixels, INT8 * SgcScores, INT8 * DispFragPtr1 );

void RowChangedLocalsScan( PP_INSTANCE *ppi, UINT8 * PixelMapPtr, UINT8 * ChLocalsPtr, INT8 * DispFragPtr,
                           UINT8   RowType );


void NoiseScoreRow( PP_INSTANCE *ppi, UINT8  * PixelMapPtr, UINT8 * ChLocalsPtr, 
				    INT16  * YUVDiffsPtr, 
                    UINT8  * PixelNoiseScorePtr, 
                    UINT32 * FragScorePtr, 
					INT8   * DispFragPtr,
                    INT32  * RowDiffsPtr );

void PrimaryEdgeScoreRow( PP_INSTANCE *ppi, 
						  UINT8  * ChangedLocalsPtr, INT16 * YUVDiffsPtr, 
                          UINT8  * PixelNoiseScorePtr, 
                          UINT32 * FragScorePtr,
						  INT8   * DispFragPtr,
                          UINT8    RowType );

void LineSearchScoreRow( PP_INSTANCE *ppi, 
						 UINT8  * ChangedLocalsPtr, INT16 * YUVDiffsPtr, 
                         UINT8  * PixelNoiseScorePtr, 
                         UINT32 * FragScorePtr, 
						 INT8   * DispFragPtr,
                         INT32    RowNumber );

UINT8 LineSearchScorePixel( PP_INSTANCE *ppi, UINT8 * ChangedLocalsPtr, INT32 RowNumber, INT32 ColNumber );
void PixelLineSearch( PP_INSTANCE *ppi, UINT8 * ChangedLocalsPtr, INT32 RowNumber, INT32 ColNumber, UINT8 direction, UINT32 * line_length );
double GetLocalVarianceMultiplier( PP_INSTANCE *ppi, INT16 * YUVDiffPtr, UINT32 PlaneLineLength );

//void  RowCopy( PP_INSTANCE *ppi, UINT32 BlockMapIndex );
UINT8 ApplyPakLowPass( PP_INSTANCE *ppi, UINT8 * SrcPtr );

/****************************************************************************
*  Module Statics
*****************************************************************************
*/              


/****************************************************************************
 * 
 *  ROUTINE       :     InitScanMapArrays
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Initialise the display and score maps
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void InitScanMapArrays(PP_INSTANCE *ppi)
{
	INT32 i;
	UINT8 StepThresh;

    /* Clear down the fragment level map arrays for the current frame. */                     
    memset( ppi->FragScores, 0, ppi->ScanFrameFragments * sizeof(UINT32) );
    memset( ppi->SameGreyDirPixels, 0, ppi->ScanFrameFragments );
    memset( ppi->FragDiffPixels, 0, ppi->ScanFrameFragments );
    memset( (void *)ppi->RowChangedPixels, 0, 3* ppi->ScanConfig.VideoFrameHeight * sizeof(INT32) );

    // Clear down blocks coded worspace.
    memset( ppi->ScanDisplayFragments, BLOCK_NOT_CODED, ppi->ScanFrameFragments );

	// Threshold used in setting up ppi->NoiseScoreBoostTable[]
	StepThresh = (UINT8)(ppi->SRFGreyThresh >> 1);
	if ( StepThresh < MIN_STEP_THRESH )
		StepThresh = MIN_STEP_THRESH;
	ppi->SrfThresh = (int)ppi->SRFGreyThresh;

	// Set up various tables used to tweak pixel score values and scoring rules 
	// based upon absolute value of a pixel change
	for ( i = 0; i < 256; i++ )
	{
		// Score multiplier table indexed by absolute difference.
		ppi->AbsDiff_ScoreMultiplierTable[i] = (double)i * DiffDevisor;
		if ( ppi->AbsDiff_ScoreMultiplierTable[i] < SCORE_MULT_LOW )
			ppi->AbsDiff_ScoreMultiplierTable[i] = SCORE_MULT_LOW;
		else if ( ppi->AbsDiff_ScoreMultiplierTable[i] > SCORE_MULT_HIGH )
			ppi->AbsDiff_ScoreMultiplierTable[i] = SCORE_MULT_HIGH;

		// Table that facilitates a relaxation of the changed locals rules in
		// NoiseScoreRow() for pixels that have changed by a large amount.
		if ( i < (ppi->SrfThresh + StepThresh) )
			ppi->NoiseScoreBoostTable[i] = 0;
		else if ( i < (ppi->SrfThresh + (StepThresh * 4)) )
			ppi->NoiseScoreBoostTable[i] = 1;
		else if ( i < (ppi->SrfThresh + (StepThresh * 6)) )
			ppi->NoiseScoreBoostTable[i] = 2;
		else
			ppi->NoiseScoreBoostTable[i] = 3;

	}

	// Set various other threshold parameters.

	// Set variables that control access to the line search algorithms.
	LineSearchTripTresh = 16;
	if ( LineSearchTripTresh > ppi->PrimaryBlockThreshold )
		LineSearchTripTresh = (UINT8)(ppi->PrimaryBlockThreshold + 1);

	// Adjust line search length if block threshold low
	ppi->MaxLineSearchLen = MAX_SEARCH_LINE_LEN;
	while ( (ppi->MaxLineSearchLen > 0) && (LineLengthScores[ppi->MaxLineSearchLen-1] > ppi->PrimaryBlockThreshold) )
		ppi->MaxLineSearchLen -= 1;

    // Initialise the level, srf and PAK threshold table pointers..
    ppi->SrfThreshTablePtr = &(ppi->SrfThreshTable[255]);
    ppi->SgcThreshTablePtr = &(ppi->SgcThreshTable[255]);
    ppi->SrfPakThreshTablePtr = &(ppi->SrfPakThreshTable[255]);

}


/****************************************************************************
 * 
 *  ROUTINE       :     AnalysePlane
 *
 *  INPUTS        :     PlanePtr0/1     Pointers to the first pixel in the plane 
 *                                       for source and reference images  
 *                      FragArrayOffset  Start offset in fragment arrays.
 *                      PWidth           Width of an image plane in pixels.
 *                      PHeight          Height of image plane in pixels
 *                      PStride          Plane stride (the number to be added to 
 *                                       a pixel index to get to the corresponding 
 *                                       pixel in the next line (can be different 
 *                                       from PWidth))
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Analyses and filters the image plane defined by the inputs.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void AnalysePlane( PP_INSTANCE *ppi, UINT8 * PlanePtr0, UINT8 * PlanePtr1, UINT32 FragArrayOffset, UINT32 PWidth, UINT32 PHeight, UINT32 PStride ) 
{
    UINT8  * RawPlanePtr0;
    UINT8  * RawPlanePtr1;

    INT16  * YUVDiffsPtr;
    INT16  * YUVDiffsPtr1;
    INT16  * YUVDiffsPtr2;
        
    UINT32 FragIndex;
    UINT32 ScoreFragIndex1;
    UINT32 ScoreFragIndex2;
    UINT32 ScoreFragIndex3;
    UINT32 ScoreFragIndex4;

    BOOL   UpdatedOrCandidateBlocks = FALSE;
    
    UINT8  * ChLocalsPtr0;
    UINT8  * ChLocalsPtr1;
    UINT8  * ChLocalsPtr2;

	UINT8  * PixelsChangedPtr0;
	UINT8  * PixelsChangedPtr1;

    UINT8  * PixelScoresPtr1;
    UINT8  * PixelScoresPtr2;
//	UINT8  * PixelScoresPtr4;

	INT8   * DispFragPtr0;
	INT8   * DispFragPtr1;
	INT8   * DispFragPtr2;

    UINT32 * FragScoresPtr1;
    UINT32 * FragScoresPtr2;

    INT32  * RowDiffsPtr;
    INT32  * RowDiffsPtr1;
    INT32  * RowDiffsPtr2;

    INT32  i,j; 

	INT32  RowNumber1;
	INT32  RowNumber2;
	INT32  RowNumber3;
	INT32  RowNumber4;

    BOOL   EdgeRow;
    INT32  LineSearchRowNumber = 0;
	
	// Variables used as temporary stores for frequently used values.
	INT32  Row0Mod3;
	INT32  Row1Mod3;
	INT32  Row2Mod3;
	INT32  BlockRowPixels;


    /* Set pixel difference threshold */
	if ( FragArrayOffset == 0 )
	{
		/* Luminance */
		ppi->LevelThresh = (int)ppi->SgcLevelThresh;
        ppi->NegLevelThresh = -ppi->LevelThresh;

		ppi->SrfThresh = (int)ppi->SRFGreyThresh;
        ppi->NegSrfThresh = -ppi->SrfThresh;

 	    // Scores correction for Y pixels.
        ppi->YUVPlaneCorrectionFactor = 1.0;

		ppi->BlockThreshold = ppi->PrimaryBlockThreshold;
		ppi->BlockSgcThresh = ppi->SgcThresh;
	}
	else
	{
		/* Chrominance */
		ppi->LevelThresh = (int)ppi->SuvcLevelThresh;
        ppi->NegLevelThresh = -ppi->LevelThresh;

        ppi->SrfThresh = (int)ppi->SRFColThresh;
        ppi->NegSrfThresh = -ppi->SrfThresh;

		// Scores correction for UV pixels.
        ppi->YUVPlaneCorrectionFactor = 1.5;

		// Block threholds different for subsampled U and V blocks
		ppi->BlockThreshold = (UINT32)(ppi->PrimaryBlockThreshold / ppi->UVBlockThreshCorrection);
		ppi->BlockSgcThresh = (UINT32)(ppi->SgcThresh / ppi->UVSgcCorrection);
	}

    // Initialise the SRF thresh table and pointer.
    memset( ppi->SrfThreshTable, 1, 512 );
    for ( i = ppi->NegSrfThresh; i <= ppi->SrfThresh; i++ )
    {
        ppi->SrfThreshTablePtr[i] = 0;
    }
    
    // Initialise the PAK thresh table.
    for ( i = -255; i <= 255; i++ )
    {
		if ( ppi->SrfThreshTablePtr[i] && (i <= ppi->HighChange) && (i >= ppi->NegHighChange) )
            ppi->SrfPakThreshTablePtr[i] = 1;
        else
            ppi->SrfPakThreshTablePtr[i] = 0;
    }

    // Initialise the SGc lookup table
    for ( i = -255; i <= 255; i++ )
    {
        if ( i <= ppi->NegLevelThresh )
            ppi->SgcThreshTablePtr[i] = -1;
        else if ( i >= ppi->LevelThresh )
            ppi->SgcThreshTablePtr[i] = 1;
        else
            ppi->SgcThreshTablePtr[i] = 0;
    }

    // Set up plane dimension variables
    ppi->PlaneHFragments = PWidth / ppi->HFragPixels;
    ppi->PlaneVFragments = PHeight / ppi->VFragPixels;
    ppi->PlaneWidth = PWidth;
    ppi->PlaneHeight = PHeight;
    ppi->PlaneStride = PStride;

    // Set up local pointers into the raw image data.
    RawPlanePtr0 = (UINT8 *)PlanePtr0;
    RawPlanePtr1 = (UINT8 *)PlanePtr1;
   
    // Note size and endo points for circular buffers.
    ppi->YuvDiffsCircularBufferSize = YDIFF_CB_ROWS * ppi->PlaneWidth;
    ppi->ChLocalsCircularBufferSize = CHLOCALS_CB_ROWS * ppi->PlaneWidth;
	ppi->PixelMapCircularBufferSize = PMAP_CB_ROWS * ppi->PlaneWidth;

    // Set high change thresh where PAK not needed;
    ppi->HighChange = ppi->SrfThresh * 4;
    ppi->NegHighChange = -ppi->HighChange;

    // Set up row difference pointers.
    RowDiffsPtr = ppi->RowChangedPixels;
    RowDiffsPtr1 = ppi->RowChangedPixels;
    RowDiffsPtr2 = ppi->RowChangedPixels;

	BlockRowPixels = ppi->PlaneWidth * ppi->VFragPixels;

    for ( i = 0; i < (ppi->PlaneVFragments + 4); i++ )
    {
		RowNumber1 = (i - 1);
		RowNumber2 = (i - 2);
		RowNumber3 = (i - 3);
		RowNumber4 = (i - 4);

		// Pre calculate some frequently used values
		Row0Mod3 = i % 3;
		Row1Mod3 = RowNumber1 % 3;
		Row2Mod3 = RowNumber2 % 3;

        //  For row diff scan last two iterations are invalid
        if ( i < ppi->PlaneVFragments )
        {
		    FragIndex = (i * ppi->PlaneHFragments) + FragArrayOffset;
            YUVDiffsPtr = &ppi->yuv_differences[Row0Mod3 * BlockRowPixels];
            
			PixelsChangedPtr0 = (UINT8 *)(&ppi->PixelChangedMap[Row0Mod3 * BlockRowPixels]);
			DispFragPtr0 =  &ppi->ScanDisplayFragments[FragIndex];

            ChLocalsPtr0 = (UINT8 *)(&ppi->ChLocals[Row0Mod3 * BlockRowPixels]);

        }

        // Set up the changed locals pointer to trail behind by one row of fragments.
        if ( i > 0 )
        {
            // For last iteration the ch locals and noise scans are invalid
            if ( RowNumber1 < ppi->PlaneVFragments )
            {
                ScoreFragIndex1 = (RowNumber1 * ppi->PlaneHFragments) + FragArrayOffset;
          
                ChLocalsPtr1 = (UINT8 *)(&ppi->ChLocals[Row1Mod3 * BlockRowPixels]);
				PixelsChangedPtr1 = (UINT8 *)(&ppi->PixelChangedMap[(Row1Mod3) * BlockRowPixels]);

				PixelScoresPtr1 = &ppi->PixelScores[(RowNumber1 % 4) * BlockRowPixels];

                YUVDiffsPtr1 = &ppi->yuv_differences[Row1Mod3 * BlockRowPixels];
                FragScoresPtr1 = &ppi->FragScores[ScoreFragIndex1];
				DispFragPtr1 = &ppi->ScanDisplayFragments[ScoreFragIndex1];

            }

            if ( RowNumber2 >= 0 )
            {
                ScoreFragIndex2 = (RowNumber2 * ppi->PlaneHFragments) + FragArrayOffset;
                ChLocalsPtr2 = (UINT8 *)(&ppi->ChLocals[Row2Mod3 * BlockRowPixels]);
                YUVDiffsPtr2 = &ppi->yuv_differences[Row2Mod3 * BlockRowPixels];

				PixelScoresPtr2 = &ppi->PixelScores[(RowNumber2 % 4) * BlockRowPixels];

                FragScoresPtr2 =  &ppi->FragScores[ScoreFragIndex2];
				DispFragPtr2 = &ppi->ScanDisplayFragments[ScoreFragIndex2];
            }
            else
            {
                ChLocalsPtr2 = NULL;
            }
        }
        else
        {
            ChLocalsPtr1 = NULL;
            ChLocalsPtr2 = NULL;
        }

		// Fast break out test for obvious yes and no cases in this row of blocks
		if ( i < ppi->PlaneVFragments )
		{
			UpdatedOrCandidateBlocks = RowSadScan( ppi, RawPlanePtr0, RawPlanePtr1, DispFragPtr0 );
			if( ColSadScan( ppi, RawPlanePtr0, RawPlanePtr1, DispFragPtr0 ) )
				UpdatedOrCandidateBlocks = TRUE;

//			SadPass2( ppi, i, DispFragPtr0 );
		}
        else	// ????? Not needed now as we always do RowSadScan etc.
        {
            // Make sure we still call other functions if RowSadScan() etc. disabled
            UpdatedOrCandidateBlocks = TRUE;
        }

		// Consolidation and fast break ot tests at Row 1 level
		if ( (i > 0) && (RowNumber1 < ppi->PlaneVFragments) )
		{
			// Mark as coded any candidate block that lies adjacent to a coded block.
			SadPass2( ppi, RowNumber1, DispFragPtr1 );

			// Check results of diff scan in last set of blocks. 
		    // Eliminate NO cases and add in +SGC cases
			ConsolidateDiffScanResults( ppi, &ppi->FragDiffPixels[ScoreFragIndex1], &ppi->SameGreyDirPixels[ScoreFragIndex1], DispFragPtr1 );
		}

        for ( j = 0; j < ppi->VFragPixels; j++ )
        {
            // Last two iterations do not apply
            if ( i < ppi->PlaneVFragments )
            {
                /* Is the current fragment at an edge. */
                EdgeRow = ( ( (i == 0) && (j == 0) ) ||
                            ( (i == (ppi->PlaneVFragments - 1)) && (j == (ppi->VFragPixels - 1)) ) );

                // Clear the arrays that will be used for the changed pixels maps
                memset( PixelsChangedPtr0, 0, ppi->PlaneWidth );

                // Difference scan and map each row
                if ( UpdatedOrCandidateBlocks )
                {
                    // Scan the row for interesting differences
					// Also clear the array that will be used for changed locals map
                    RowDiffScan( ppi, RawPlanePtr0, RawPlanePtr1, 
                                 YUVDiffsPtr, PixelsChangedPtr0, 
                                 &ppi->SameGreyDirPixels[FragIndex], 
                                 DispFragPtr0, &ppi->FragDiffPixels[FragIndex], 
							     RowDiffsPtr, ChLocalsPtr0, EdgeRow);
                }
                else
                {
					// Clear the array that will be used for changed locals map
					memset( ChLocalsPtr0, 0, ppi->PlaneWidth );
                }

                // The actual image plane pointers must be incremented by stride as this may be 
                // different (more) than the plane width. Our own internal buffers use ppi->PlaneWidth.
                RawPlanePtr0 += ppi->PlaneStride;
                RawPlanePtr1 += ppi->PlaneStride;
				PixelsChangedPtr0 += ppi->PlaneWidth;
                ChLocalsPtr0 += ppi->PlaneWidth;
                YUVDiffsPtr += ppi->PlaneWidth;
                RowDiffsPtr++;
            }

            // Run behind calculating the changed locals data and noise scores.
            if ( ChLocalsPtr1 != NULL )
            {
                // Last few iterations do not apply
                if ( RowNumber1 < ppi->PlaneVFragments )
                {
                    // Blank the next row in the pixel scores data structure.
	                memset( PixelScoresPtr1, 0, ppi->PlaneWidth );

                    // Don't bother doing anything if there are no changed pixels in this row
                    if ( *RowDiffsPtr1 )
                    {
					    // Last valid row is a special case
                        if ( i < ppi->PlaneVFragments )
                            RowChangedLocalsScan( ppi, PixelsChangedPtr1, ChLocalsPtr1, DispFragPtr1, (UINT8)( (((i-1)==0) && (j==0)) ? FIRST_ROW : NOT_EDGE_ROW) );
                        else    
                            RowChangedLocalsScan( ppi, PixelsChangedPtr1, ChLocalsPtr1, DispFragPtr1, (UINT8)((j==(ppi->VFragPixels-1)) ? LAST_ROW : NOT_EDGE_ROW) );

                        NoiseScoreRow( ppi, PixelsChangedPtr1, ChLocalsPtr1, YUVDiffsPtr1,
                                       PixelScoresPtr1, FragScoresPtr1, DispFragPtr1, RowDiffsPtr1 );
                    }

                    ChLocalsPtr1 += ppi->PlaneWidth;
					PixelsChangedPtr1 += ppi->PlaneWidth;
                    YUVDiffsPtr1 += ppi->PlaneWidth;
                    PixelScoresPtr1 += ppi->PlaneWidth;
                    RowDiffsPtr1 ++;
                }

                // Run edge enhancement algorithms
                if ( RowNumber2 < ppi->PlaneVFragments )
                {
					if ( ChLocalsPtr2 != NULL )
					{
						// Don't bother doing anything if there are no changed pixels in this row
						if ( *RowDiffsPtr2 )
						{
							if ( RowNumber1 < ppi->PlaneVFragments )
							{
								PrimaryEdgeScoreRow( ppi, ChLocalsPtr2, YUVDiffsPtr2,
													 PixelScoresPtr2, FragScoresPtr2, DispFragPtr2,
													 (UINT8)( (((i-2)==0) && (j==0)) ? FIRST_ROW : NOT_EDGE_ROW)  );
							}
							else
							{
								// Edge enhancement
								PrimaryEdgeScoreRow( ppi, ChLocalsPtr2, YUVDiffsPtr2,
													 PixelScoresPtr2, FragScoresPtr2, DispFragPtr2,
													 (UINT8)((j==(ppi->VFragPixels-1)) ? LAST_ROW : NOT_EDGE_ROW) );
							}

							// Recursive line search
							LineSearchScoreRow( ppi, ChLocalsPtr2, YUVDiffsPtr2,
												PixelScoresPtr2, FragScoresPtr2, DispFragPtr2,
												LineSearchRowNumber );
						}

						ChLocalsPtr2 += ppi->PlaneWidth;
						YUVDiffsPtr2 += ppi->PlaneWidth;
						PixelScoresPtr2 += ppi->PlaneWidth;
						LineSearchRowNumber += 1;
						RowDiffsPtr2 ++;
					}
				}
            }
        }

		// BAR algorithm
		if ( (RowNumber3 >= 0) && (RowNumber3 < ppi->PlaneVFragments) )
		{
            ScoreFragIndex3 = (RowNumber3 * ppi->PlaneHFragments) + FragArrayOffset;
			RowBarEnhBlockMap(ppi,  &ppi->FragScores[ScoreFragIndex3], 
			   				   &ppi->SameGreyDirPixels[ScoreFragIndex3],
							   &ppi->ScanDisplayFragments[ScoreFragIndex3],
							   &ppi->BarBlockMap[(RowNumber3 % 3) * ppi->PlaneHFragments],
							   RowNumber3 );
		}

		// BAR copy back and "ppi->SRF filtering" or "pixel copy back"
		if ( (RowNumber4 >= 0) && (RowNumber4 < ppi->PlaneVFragments) )
		{
            // BAR copy back stage must lag by one more row to avoid BAR blocks
			// being used in BAR descisions.
            ScoreFragIndex4 = (RowNumber4 * ppi->PlaneHFragments) + FragArrayOffset;

			BarCopyBack(ppi, &ppi->ScanDisplayFragments[ScoreFragIndex4],
						&ppi->BarBlockMap[(RowNumber4 % 3) * ppi->PlaneHFragments]);

/*
            // "Apply ppi->SRF filtering to" or "copy back" pixels.
			PixelScoresPtr4 = &ppi->PixelScores[(RowNumber4 % 4) * BlockRowPixels];
*/
            // Copy over the data from any blocks marked for update into the output buffer.
            //RowCopy(ppi, ScoreFragIndex4);
		}
    }
}


/****************************************************************************
 * 
 *  ROUTINE       :     RowSadScan
 *
 *  INPUTS        :     UINT8  * YuvPtr1, YuvPtr2 
 *								 Pointers into current and previous frame
 *
 *  OUTPUTS       :		INT8   * DispFragPtr
 *								 Fragment update map (-1 = ???, 0 = No, >0 = Yes)
 *
 *  RETURNS       :     TRUE if row contains Candidate or coded blocsk else FALSE
 *
 *  FUNCTION      :     Preliminary fast scan based upon local SAD scores of 4 pixel groups
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
BOOL RowSadScan( PP_INSTANCE *ppi, UINT8 * YuvPtr1, UINT8 * YuvPtr2, INT8 *  DispFragPtr )
{
	INT32    i, j;
	UINT32   GrpSad;
	UINT32   LocalGrpLowSadThresh = ppi->ModifiedGrpLowSadThresh;
	UINT32   LocalGrpHighSadThresh = ppi->ModifiedGrpHighSadThresh;
	INT8   * LocalDispFragPtr;
	UINT32 * LocalYuvPtr1;
	UINT32 * LocalYuvPtr2;

    BOOL     InterestingBlocksInRow = FALSE;

    // For each row of pixels in the row of blocks
    for ( j = 0; j < ppi->VFragPixels; j++ )
    {
		// Set local block map pointer.
		LocalDispFragPtr = DispFragPtr;

		// Set the local pixel data pointers for this row.
		LocalYuvPtr1 = (UINT32 *)YuvPtr1;
		LocalYuvPtr2 = (UINT32 *)YuvPtr2;

		// Scan along the row of pixels
		// If the block to which a group of pixels belongs is already marked for update then do nothing.
		for ( i = 0; i < ppi->PlaneHFragments; i ++ )
		{
			if ( *LocalDispFragPtr <= BLOCK_NOT_CODED )
			{
				// Calculate the SAD score for the block row		    
				GrpSad = ppi->RowSAD((UINT8 *)LocalYuvPtr1,(UINT8 *)LocalYuvPtr2);

				// Now test the group SAD score
				if ( GrpSad > LocalGrpLowSadThresh )
				{
					// If SAD very high we must update else we have candidate block
					if ( GrpSad > LocalGrpHighSadThresh )
					{
						// Force update
						*LocalDispFragPtr = BLOCK_CODED;
					}
					else
					{
						// Possible Update required
						*LocalDispFragPtr = CANDIDATE_BLOCK;
					}
                    InterestingBlocksInRow = TRUE;
				}
			}
			/**********  PGW 27/APR/2001 ***********/
			else
                InterestingBlocksInRow = TRUE;

			LocalDispFragPtr++;

			LocalYuvPtr1 += 2;
			LocalYuvPtr2 += 2;
		}

		// Increment the base data pointers to the start of the next line.
		YuvPtr1 += ppi->PlaneStride;
		YuvPtr2 += ppi->PlaneStride;
	}

    // This code is PC specific
    if ( ppi->MmxEnabled )
    {
        ClearMmxState(ppi);
    }

    return InterestingBlocksInRow;

}

/****************************************************************************
 * 
 *  ROUTINE       :     ColSadScan
 *
 *  INPUTS        :     UINT8  * YuvPtr1, YuvPtr2 
 *								 Pointers into current and previous frame
 *
 *  OUTPUTS       :		INT8   * DispFragPtr
 *								 Fragment update map (-1 = ???, 0 = No, >0 = Yes)
 *
 *  RETURNS       :     TRUE if row contains Candidate or coded blocsk else FALSE
 *
 *  FUNCTION      :     Preliminary fast scan based upon local SAD scores of 4 pixel groups
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
BOOL ColSadScan( PP_INSTANCE *ppi, UINT8 * YuvPtr1, UINT8 * YuvPtr2, INT8 *  DispFragPtr )
{
	INT32    i;
	UINT32   MaxSad;
	UINT32   LocalGrpLowSadThresh = ppi->ModifiedGrpLowSadThresh;
	UINT32   LocalGrpHighSadThresh = ppi->ModifiedGrpHighSadThresh;
	INT8   * LocalDispFragPtr;
	
	UINT8  * LocalYuvPtr1;		
	UINT8  * LocalYuvPtr2;

    BOOL     InterestingBlocksInRow = FALSE;

	// Set the local pixel data pointers for this row.
	LocalYuvPtr1 = YuvPtr1;
	LocalYuvPtr2 = YuvPtr2;

	// Set local block map pointer.
	LocalDispFragPtr = DispFragPtr;

	// Scan along the row of blocks
	for ( i = 0; i < ppi->PlaneHFragments; i ++ )
	{
		// Skip if block already marked to be coded.
		if ( *LocalDispFragPtr <= BLOCK_NOT_CODED )
		{
			// Calculate the SAD score for the block column		    
			MaxSad = ppi->ColSAD( ppi, (UINT8 *)LocalYuvPtr1,(UINT8 *)LocalYuvPtr2 );

			// Now test the group SAD score
			if ( MaxSad > LocalGrpLowSadThresh )
			{
				// If SAD very high we must update else we have candidate block
				if ( MaxSad > LocalGrpHighSadThresh )
				{
					// Force update
					*LocalDispFragPtr = BLOCK_CODED;
				}
				else
				{
					// Possible Update required
					*LocalDispFragPtr = CANDIDATE_BLOCK;
				}
				InterestingBlocksInRow = TRUE;
			}
		}
		/**********  PGW 27/APR/2001 ***********/
		else
            InterestingBlocksInRow = TRUE;

		// Increment the block map pointer.
		LocalDispFragPtr++;			

		// Step data pointers on ready for next block
		LocalYuvPtr1 += ppi->HFragPixels;
		LocalYuvPtr2 += ppi->HFragPixels;
	}

    // This code is PC specific
    if ( ppi->MmxEnabled )
    {
        ClearMmxState(ppi);
    }

    return InterestingBlocksInRow;

}

/****************************************************************************
 * 
 *  ROUTINE       :     SadPass2
 *
 *  INPUTS        :     UINT32   RowNumber
 *								 Fragment row number
 *						INT8  *  DispFragPtr
 *								 Fragment update map (-1 = ???, 0 = No, >0 = Yes)
 *
 *  OUTPUTS       :		INT8  *  DispFragPtr
 *								 Fragment update map (-1 = ???, 0 = No, >0 = Yes)
 *  RETURNS       :     
 *
 *  FUNCTION      :     This second pass should only be used when speed is critical.
 *                      The function revisits the classification of CANDIDATE_BLOCKS
 *                      if they are adjacent to one or more CODED_BLOCKS.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
void SadPass2( PP_INSTANCE *ppi, INT32 RowNumber, INT8 *  DispFragPtr )
{
	INT32  i;

	// First row
	if ( RowNumber == 0 )
	{
		// First block in row.
		if ( DispFragPtr[0] == CANDIDATE_BLOCK )
		{
			if ( (DispFragPtr[1] == BLOCK_CODED) ||
				 (DispFragPtr[ppi->PlaneHFragments] == BLOCK_CODED) ||
				 (DispFragPtr[ppi->PlaneHFragments+1] == BLOCK_CODED) )
			{
				ppi->TmpCodedMap[0] =  BLOCK_CODED_LOW;
			}
			else
			{
				ppi->TmpCodedMap[0] = DispFragPtr[0];
			}
		}
		else
		{
				ppi->TmpCodedMap[0] = DispFragPtr[0];
		}

		// All but first and last in row
		for ( i = 1; (i < ppi->PlaneHFragments-1); i++ )
		{
			if ( DispFragPtr[i] == CANDIDATE_BLOCK )
			{
				if ( (DispFragPtr[i-1] == BLOCK_CODED) || 
					 (DispFragPtr[i+1] == BLOCK_CODED) ||
					 (DispFragPtr[i+ppi->PlaneHFragments] == BLOCK_CODED) ||
					 (DispFragPtr[i+ppi->PlaneHFragments-1] == BLOCK_CODED) ||
					 (DispFragPtr[i+ppi->PlaneHFragments+1] == BLOCK_CODED) )
				{
					ppi->TmpCodedMap[i] =  BLOCK_CODED_LOW;
				}
				else
				{
					ppi->TmpCodedMap[i] = DispFragPtr[i];
				}
			}
			else
			{
				ppi->TmpCodedMap[i] = DispFragPtr[i];
			}
		}

		// Last block in row.
		i = ppi->PlaneHFragments-1;
		if ( DispFragPtr[i] == CANDIDATE_BLOCK )
		{
			if ( (DispFragPtr[i-1] == BLOCK_CODED) || 
				 (DispFragPtr[i+ppi->PlaneHFragments] == BLOCK_CODED) ||
				 (DispFragPtr[i+ppi->PlaneHFragments-1] == BLOCK_CODED) )
			{
				ppi->TmpCodedMap[i] =  BLOCK_CODED_LOW;
			}
			else
			{
				ppi->TmpCodedMap[i] = DispFragPtr[i];
			}
		}
		else
		{
			ppi->TmpCodedMap[i] = DispFragPtr[i];
		}
	}

	// General case
	else if ( RowNumber < (ppi->PlaneVFragments - 1) )
	{
		// First block in row.
		if ( DispFragPtr[0] == CANDIDATE_BLOCK )
		{
			if ( (DispFragPtr[1] == BLOCK_CODED) ||
				 (DispFragPtr[(-ppi->PlaneHFragments)] == BLOCK_CODED) ||
				 (DispFragPtr[(-ppi->PlaneHFragments)+1] == BLOCK_CODED) ||
				 (DispFragPtr[ppi->PlaneHFragments] == BLOCK_CODED) ||
				 (DispFragPtr[ppi->PlaneHFragments+1] == BLOCK_CODED) )
			{
				ppi->TmpCodedMap[0] =  BLOCK_CODED_LOW;
			}
			else
			{
				ppi->TmpCodedMap[0] = DispFragPtr[0];
			}
		}
		else
		{
			ppi->TmpCodedMap[0] = DispFragPtr[0];
		}

		// All but first and last in row
		for ( i = 1; (i < ppi->PlaneHFragments-1); i++ )
		{
			if ( DispFragPtr[i] == CANDIDATE_BLOCK )
			{
				if ( (DispFragPtr[i-1] == BLOCK_CODED) || 
					 (DispFragPtr[i+1] == BLOCK_CODED) ||
					 (DispFragPtr[i-ppi->PlaneHFragments] == BLOCK_CODED) ||
					 (DispFragPtr[i-ppi->PlaneHFragments-1] == BLOCK_CODED) ||
					 (DispFragPtr[i-ppi->PlaneHFragments+1] == BLOCK_CODED) ||
					 (DispFragPtr[i+ppi->PlaneHFragments] == BLOCK_CODED) ||
					 (DispFragPtr[i+ppi->PlaneHFragments-1] == BLOCK_CODED) ||
					 (DispFragPtr[i+ppi->PlaneHFragments+1] == BLOCK_CODED) )
				{
					ppi->TmpCodedMap[i] =  BLOCK_CODED_LOW;
				}
				else
				{
					ppi->TmpCodedMap[i] = DispFragPtr[i];
				}
			}
			else
			{
				ppi->TmpCodedMap[i] = DispFragPtr[i];
			}
		}

		// Last block in row.
		i = ppi->PlaneHFragments-1;
		if ( DispFragPtr[i] == CANDIDATE_BLOCK )
		{
			if ( (DispFragPtr[i-1] == BLOCK_CODED) || 
				 (DispFragPtr[i-ppi->PlaneHFragments] == BLOCK_CODED) ||
				 (DispFragPtr[i-ppi->PlaneHFragments-1] == BLOCK_CODED) ||
				 (DispFragPtr[i+ppi->PlaneHFragments] == BLOCK_CODED) ||
				 (DispFragPtr[i+ppi->PlaneHFragments-1] == BLOCK_CODED) )
			{
				ppi->TmpCodedMap[i] =  BLOCK_CODED_LOW;
			}
			else
			{
				ppi->TmpCodedMap[i] = DispFragPtr[i];
			}
		}
		else
		{
			ppi->TmpCodedMap[i] = DispFragPtr[i];
		}
	}

    // Last row
	else
	{
		// First block in row.
		if ( DispFragPtr[0] == CANDIDATE_BLOCK )
		{
			if ( (DispFragPtr[1] == BLOCK_CODED) ||
				 (DispFragPtr[(-ppi->PlaneHFragments)] == BLOCK_CODED) ||
				 (DispFragPtr[(-ppi->PlaneHFragments)+1] == BLOCK_CODED))
			{
				ppi->TmpCodedMap[0] =  BLOCK_CODED_LOW;
			}
			else
			{
				ppi->TmpCodedMap[0] = DispFragPtr[0];
			}
		}
		else
		{
			ppi->TmpCodedMap[0] = DispFragPtr[0];
		}

		// All but first and last in row
		for ( i = 1; (i < ppi->PlaneHFragments-1); i++ )
		{
			if ( DispFragPtr[i] == CANDIDATE_BLOCK )
			{
				if ( (DispFragPtr[i-1] == BLOCK_CODED) || 
					 (DispFragPtr[i+1] == BLOCK_CODED) ||
					 (DispFragPtr[i-ppi->PlaneHFragments] == BLOCK_CODED) ||
					 (DispFragPtr[i-ppi->PlaneHFragments-1] == BLOCK_CODED) ||
					 (DispFragPtr[i-ppi->PlaneHFragments+1] == BLOCK_CODED) )
				{
					ppi->TmpCodedMap[i] =  BLOCK_CODED_LOW;
				}
				else
				{
					ppi->TmpCodedMap[i] = DispFragPtr[i];
				}
			}
			else
			{
				ppi->TmpCodedMap[i] = DispFragPtr[i];
			}
		}

		// Last block in row.
		i = ppi->PlaneHFragments-1;
		if ( DispFragPtr[i] == CANDIDATE_BLOCK )
		{
			if ( (DispFragPtr[i-1] == BLOCK_CODED) || 
				 (DispFragPtr[i-ppi->PlaneHFragments] == BLOCK_CODED) ||
				 (DispFragPtr[i-ppi->PlaneHFragments-1] == BLOCK_CODED) )
			{
				ppi->TmpCodedMap[i] =  BLOCK_CODED_LOW;
			}
			else
			{
				ppi->TmpCodedMap[i] = DispFragPtr[i];
			}
		}
		else
		{
			ppi->TmpCodedMap[i] = DispFragPtr[i];
		}
	}

    // Now copy back the modified Fragment data
	memcpy( &DispFragPtr[0], &ppi->TmpCodedMap[0], (ppi->PlaneHFragments) );

}

/****************************************************************************
 * 
 *  ROUTINE       :     RowDiffScan
 *
 *  INPUTS        :     UINT8  * YuvPtr1, YuvPtr2 
 *								 Pointers into current and previous frame
 *                      BOOL     EdgeRow
 *                               Is this row an edge row.
 *
 *  OUTPUTS       :		UINT16 * YUVDiffsPtr
 *								 Differnece map
 *                      UINT8  * bits_map_ptr
 *                               Pixels changed map
 *                      UINT8  * SgcPtr
 *								 Level change score.
 *                      INT8   * DispFragPtr
 *                               Block update map.
 *                      INT32  * RowDiffsPtr
 *								 Total sig changes for row
 *                      UINT8 *  ChLocalsPtr
 *                               Changed locals data structure
 *
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     Initial pixel differences scan
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
void RowDiffScan( PP_INSTANCE *ppi, UINT8 * YuvPtr1, UINT8 * YuvPtr2, 
                  INT16 * YUVDiffsPtr, UINT8 * bits_map_ptr, 
                  INT8  * SgcPtr, INT8  * DispFragPtr, 
				  UINT8 * FDiffPixels, INT32 * RowDiffsPtr, 
                  UINT8 * ChLocalsPtr, BOOL EdgeRow )
{
    INT32 i,j;
    INT32 FragChangedPixels;

    UINT32 ZeroData[2] = { 0,0 };
    UINT8  OneData[8] = { 1,1,1,1,1,1,1,1 };
    UINT8  ChlocalsDummyData[8] = { 8,8,8,8,8,8,8,8 };

    INT16 Diff;     // Temp local workspace.

    // Cannot use kernel if at edge or if PAK disabled
    if ( (!ppi->PAKEnabled) || EdgeRow )
    {
        for ( i = 0; i < ppi->PlaneWidth; i += ppi->HFragPixels )
        {
            // Reset count of pixels changed for the current fragment.
            FragChangedPixels = 0;

            // Test for break out conditions to save time. 
			if (*DispFragPtr == CANDIDATE_BLOCK)
			{
                // Clear down entries in changed locals array
                ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ZeroData)[0];
                ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ZeroData)[1];

                for ( j = 0; j < ppi->HFragPixels; j++ )
                {
                    // Take a local copy of the measured difference.
    			    Diff = ((INT16)YuvPtr1[j]) - ((INT16)YuvPtr2[j]);

                    // Store the actual difference value
                    YUVDiffsPtr[j] = Diff;

				    // Test against the Level thresholds and record the results
                    SgcPtr[0] += ppi->SgcThreshTablePtr[Diff];

                    // Test against the SRF thresholds
                    bits_map_ptr[j] = ppi->SrfThreshTablePtr[Diff];
                    FragChangedPixels += ppi->SrfThreshTablePtr[Diff];
                }
	        }
            else
            {
                // For EBO coded blocks mark all pixels as changed.
                if ( *DispFragPtr > BLOCK_NOT_CODED )
                {
                    ((UINT32 *)bits_map_ptr)[0] = ((UINT32 *)OneData)[0];
                    ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ChlocalsDummyData)[0];
                    ((UINT32 *)bits_map_ptr)[1] = ((UINT32 *)OneData)[1];
                    ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ChlocalsDummyData)[1];
                }
                else
                {
                    ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ZeroData)[0];
                    ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ZeroData)[1];
                }
            }

			*RowDiffsPtr += FragChangedPixels;
			*FDiffPixels += (UINT8)FragChangedPixels;

			YuvPtr1 += ppi->HFragPixels;
			YuvPtr2 += ppi->HFragPixels;
			bits_map_ptr += ppi->HFragPixels;
            ChLocalsPtr += ppi->HFragPixels;
			YUVDiffsPtr += ppi->HFragPixels;
			SgcPtr ++;
			FDiffPixels ++;

			// If we have a lot of changed pixels for this fragment on this row then 
			// the fragment is almost sure to be picked (e.g. through the line search) so we
			// can mark it as selected and then ignore it.
			if (FragChangedPixels >= 7)
			{
				*DispFragPtr = BLOCK_CODED_LOW;
			}
			DispFragPtr++;    
		}
    }
    else
    {
        
        //*************************************************************
        // First fragment of row !!
        
        i = 0;
        // Reset count of pixels changed for the current fragment.
        FragChangedPixels = 0;
        
        // Test for break out conditions to save time. 
        if (*DispFragPtr == CANDIDATE_BLOCK)
        {
            // Clear down entries in changed locals array
            ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ZeroData)[0];
            ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ZeroData)[1];
            
            for ( j = 0; j < ppi->HFragPixels; j++ )
            {
                // Take a local copy of the measured difference.
                Diff = ((INT16)YuvPtr1[j]) - ((INT16)YuvPtr2[j]);
                
                // Store the actual difference value
                YUVDiffsPtr[j] = Diff;
                
                // Test against the Level thresholds and record the results
                SgcPtr[0] += ppi->SgcThreshTablePtr[Diff];
                
                // jbb added i+j > 0 and i+j < ppi->HFragPixels - 1 check
                if (j>0 && ppi->SrfPakThreshTablePtr[Diff] )
                    Diff = (int)ApplyPakLowPass( ppi, &YuvPtr1[j] ) - 
                    (int)ApplyPakLowPass( ppi, &YuvPtr2[j] );
                
                
                // Test against the SRF thresholds
                bits_map_ptr[j] = ppi->SrfThreshTablePtr[Diff];
                FragChangedPixels += ppi->SrfThreshTablePtr[Diff];
            }
        }
        else
        {
            // For EBO coded blocks mark all pixels as changed.
            if ( *DispFragPtr > BLOCK_NOT_CODED )
            {
                ((UINT32 *)bits_map_ptr)[0] = ((UINT32 *)OneData)[0];
                ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ChlocalsDummyData)[0];
                
                ((UINT32 *)bits_map_ptr)[1] = ((UINT32 *)OneData)[1];
                ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ChlocalsDummyData)[1];
            }
            else
            {
                ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ZeroData)[0];
                ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ZeroData)[1];
            }
        }
        
        *RowDiffsPtr += FragChangedPixels;
        *FDiffPixels += (UINT8)FragChangedPixels;
        
        YuvPtr1 += ppi->HFragPixels;
        YuvPtr2 += ppi->HFragPixels;
        bits_map_ptr += ppi->HFragPixels;
        ChLocalsPtr += ppi->HFragPixels;
        YUVDiffsPtr += ppi->HFragPixels;
        SgcPtr ++;
        FDiffPixels ++;
        
        // If we have a lot of changed pixels for this fragment on this row then 
        // the fragment is almost sure to be picked (e.g. through the line search) so we
        // can mark it as selected and then ignore it.
        if (FragChangedPixels >= 7)
        {
            *DispFragPtr = BLOCK_CODED_LOW;
        }
        DispFragPtr++;    
        //*************************************************************
        // Fragment in between!!

        for ( i = ppi->HFragPixels ; i < ppi->PlaneWidth-ppi->HFragPixels; i += ppi->HFragPixels )
        {
            // Reset count of pixels changed for the current fragment.
            FragChangedPixels = 0;

            // Test for break out conditions to save time. 
			if (*DispFragPtr == CANDIDATE_BLOCK)
			{
                // Clear down entries in changed locals array
                ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ZeroData)[0];
                ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ZeroData)[1];

                for ( j = 0; j < ppi->HFragPixels; j++ )
                {
                    // Take a local copy of the measured difference.
    			    Diff = ((INT16)YuvPtr1[j]) - ((INT16)YuvPtr2[j]);

                    // Store the actual difference value
                    YUVDiffsPtr[j] = Diff;

				    // Test against the Level thresholds and record the results
                    SgcPtr[0] += ppi->SgcThreshTablePtr[Diff];

                    // jbb added i+j > 0 and i+j < ppi->HFragPixels - 1 check
                    if (ppi->SrfPakThreshTablePtr[Diff] )
						Diff = (int)ApplyPakLowPass( ppi, &YuvPtr1[j] ) - 
							   (int)ApplyPakLowPass( ppi, &YuvPtr2[j] );


                    // Test against the SRF thresholds
                    bits_map_ptr[j] = ppi->SrfThreshTablePtr[Diff];
                    FragChangedPixels += ppi->SrfThreshTablePtr[Diff];
				}
            }
            else
            {
                // For EBO coded blocks mark all pixels as changed.
                if ( *DispFragPtr > BLOCK_NOT_CODED )
                {
                    ((UINT32 *)bits_map_ptr)[0] = ((UINT32 *)OneData)[0];
                    ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ChlocalsDummyData)[0];

                    ((UINT32 *)bits_map_ptr)[1] = ((UINT32 *)OneData)[1];
                    ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ChlocalsDummyData)[1];
                }
                else
                {
                    ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ZeroData)[0];
                    ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ZeroData)[1];
                }
            }

			*RowDiffsPtr += FragChangedPixels;
			*FDiffPixels += (UINT8)FragChangedPixels;

            YuvPtr1 += ppi->HFragPixels;
            YuvPtr2 += ppi->HFragPixels;
            bits_map_ptr += ppi->HFragPixels;
            ChLocalsPtr += ppi->HFragPixels;
            YUVDiffsPtr += ppi->HFragPixels;
            SgcPtr ++;
			FDiffPixels ++;

			// If we have a lot of changed pixels for this fragment on this row then 
			// the fragment is almost sure to be picked (e.g. through the line search) so we
			// can mark it as selected and then ignore it.
			if (FragChangedPixels >= 7)
			{
				*DispFragPtr = BLOCK_CODED_LOW;
			}
			DispFragPtr++;    
        }
        //*************************************************************

        //*************************************************************
        // Last fragment of row !!

        // Reset count of pixels changed for the current fragment.
        FragChangedPixels = 0;
        
        // Test for break out conditions to save time. 
        if (*DispFragPtr == CANDIDATE_BLOCK)
        {
            // Clear down entries in changed locals array
            ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ZeroData)[0];
            ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ZeroData)[1];
            
            for ( j = 0; j < ppi->HFragPixels; j++ )
            {
                // Take a local copy of the measured difference.
                Diff = ((INT16)YuvPtr1[j]) - ((INT16)YuvPtr2[j]);
                
                // Store the actual difference value
                YUVDiffsPtr[j] = Diff;
                
                // Test against the Level thresholds and record the results
                SgcPtr[0] += ppi->SgcThreshTablePtr[Diff];
                
                // jbb added i+j > 0 and i+j < ppi->HFragPixels - 1 check
                if (j<7 && ppi->SrfPakThreshTablePtr[Diff] )
                    Diff = (int)ApplyPakLowPass( ppi, &YuvPtr1[j] ) - 
                    (int)ApplyPakLowPass( ppi, &YuvPtr2[j] );
                
                
                // Test against the SRF thresholds
                bits_map_ptr[j] = ppi->SrfThreshTablePtr[Diff];
                FragChangedPixels += ppi->SrfThreshTablePtr[Diff];
            }
        }
        else
        {
            // For EBO coded blocks mark all pixels as changed.
            if ( *DispFragPtr > BLOCK_NOT_CODED )
            {
                ((UINT32 *)bits_map_ptr)[0] = ((UINT32 *)OneData)[0];
                ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ChlocalsDummyData)[0];
                
                ((UINT32 *)bits_map_ptr)[1] = ((UINT32 *)OneData)[1];
                ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ChlocalsDummyData)[1];
            }
            else
            {
                ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ZeroData)[0];
                ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ZeroData)[1];
            }
        }
        // If we have a lot of changed pixels for this fragment on this row then 
        // the fragment is almost sure to be picked (e.g. through the line search) so we
        // can mark it as selected and then ignore it.
        *RowDiffsPtr += FragChangedPixels;
        *FDiffPixels += (UINT8)FragChangedPixels;
        
        // If we have a lot of changed pixels for this fragment on this row then 
        // the fragment is almost sure to be picked (e.g. through the line search) so we
        // can mark it as selected and then ignore it.
        if (FragChangedPixels >= 7)
        {
            *DispFragPtr = BLOCK_CODED_LOW;
        }
        DispFragPtr++;    
        //*************************************************************

    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     ConsolidateDiffScanResults
 *
 *  INPUTS        :     UINT8  * FDiffPixels
 *								 Fragment changed pixels records
 *                      UINT8  * SgcScoresPtr
 *								 Fragment SGC records
 *                      INT8   * DispFragPtr
 *                               Fragment update map (-1 = ???, 0 = No, 1 = Yes)
 *
 *  OUTPUTS       :		UINT8  * DispFragPtr
 *								 Fragment update map (-1 = ???, 0 = No, 1 = Yes)
 *  RETURNS       :     
 *
 *  FUNCTION      :     Considers new information from difference scan and, if necessary, 
 *                      upates output map.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
void ConsolidateDiffScanResults( PP_INSTANCE *ppi, UINT8 * FDiffPixels, INT8 * SgcScoresPtr, INT8 * DispFragPtr )
{
	INT32 i;

	for ( i = 0; i < ppi->PlaneHFragments; i ++ )
	{
		// Consider only those blocks that were candidates in the
		// difference scan. Ignore definite YES and NO cases.
		if ( DispFragPtr[i] == CANDIDATE_BLOCK )
		{
			if ( ((UINT32)abs(SgcScoresPtr[i]) > ppi->BlockSgcThresh) )
			{
				// Block marked for update due to Sgc change
				DispFragPtr[i] = BLOCK_CODED_SGC;
			}
			else if ( FDiffPixels[i] == 0 )
			{
				// Block marked for NO update as no/too few interesting pixels. 
				//DispFragPtr[i] = BLOCK_NOT_CODED;

				// Block is no longer a candidate for the main tests but will 
				// still be considered a candidate in RowBarEnhBlockMap()
				DispFragPtr[i] = CANDIDATE_BLOCK_LOW;
			}
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     RowChangedLocalsScan
 *
 *  INPUTS        :     UINT8  * PixelMapPtr.
 *                      UINT8  * ChLocalsPtr.
 *                      INT8   * DispFragPtr
 *                      UINT8  * RowType
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     Calculates changed locals for changed pixels
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
void RowChangedLocalsScan( PP_INSTANCE *ppi, UINT8 * PixelMapPtr, UINT8 * ChLocalsPtr, 
						   INT8  * DispFragPtr, UINT8   RowType )
{
    UINT8 ChlocalsDummyData[8] = { 8,8,8,8,8,8,8,8 };
    UINT8 changed_locals = 0; 
    UINT8 Score = 0;    
	UINT8 * PixelsChangedPtr0;
	UINT8 * PixelsChangedPtr1;
	UINT8 * PixelsChangedPtr2;
    INT32 i, j;
	INT32 LastRowIndex = ppi->PlaneWidth - 1;

	// Set up the line based pointers into the bits changed map.
	PixelsChangedPtr0 = PixelMapPtr - ppi->PlaneWidth;
	if ( PixelsChangedPtr0 < ppi->PixelChangedMap )
		PixelsChangedPtr0 += ppi->PixelMapCircularBufferSize;
	PixelsChangedPtr0 -= 1;	
	
	PixelsChangedPtr1 = PixelMapPtr - 1;

	PixelsChangedPtr2 = PixelMapPtr + ppi->PlaneWidth;
	if ( PixelsChangedPtr2 >= (ppi->PixelChangedMap + ppi->PixelMapCircularBufferSize) )
		PixelsChangedPtr2 -= ppi->PixelMapCircularBufferSize;
	PixelsChangedPtr2 -= 1;	

    if ( RowType == NOT_EDGE_ROW )
    {
        // Scan through the row of pixels and calculate changed locals.
        for ( i = 0; i < ppi->PlaneWidth; i += ppi->HFragPixels )
        {
            // Skip a group of 8 pixels if the assosciated fragment has no pixels of interest or
            // if EBO is enabled and a breakout condition is met.
            if ( *DispFragPtr == CANDIDATE_BLOCK )
            {
                for ( j = 0; j < ppi->HFragPixels; j++ )
                {
					changed_locals = 0;

                    // If the pixel itself has changed
                    if ( PixelsChangedPtr1[1] )
                    {
						if ( (i > 0) || (j > 0) )
						{
							changed_locals += PixelsChangedPtr0[0];
							changed_locals += PixelsChangedPtr1[0];
							changed_locals += PixelsChangedPtr2[0];
						}

						changed_locals += PixelsChangedPtr0[1];
		                changed_locals += PixelsChangedPtr2[1];

						if ( (i + j) < LastRowIndex )
						{
							changed_locals += PixelsChangedPtr0[2];
							changed_locals += PixelsChangedPtr1[2];
							changed_locals += PixelsChangedPtr2[2];
						}

                        // Store the number of changed locals
                        *ChLocalsPtr |= changed_locals;
                    }

                    // Increment to next pixel in the row
                    ChLocalsPtr++;
					PixelsChangedPtr0++;
					PixelsChangedPtr1++;
					PixelsChangedPtr2++;
                }
            }
            else
            {
                if ( *DispFragPtr > BLOCK_NOT_CODED )
                {
                    ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ChlocalsDummyData)[0];
                    ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ChlocalsDummyData)[1];
                }

                // Step pointers
                ChLocalsPtr += ppi->HFragPixels;
				PixelsChangedPtr0 += ppi->HFragPixels;
				PixelsChangedPtr1 += ppi->HFragPixels;
				PixelsChangedPtr2 += ppi->HFragPixels;
            }

            // Move on to next fragment.
			DispFragPtr++;    

        }
    }
    else 
    {
        // Scan through the row of pixels and calculate changed locals.
        for ( i = 0; i < ppi->PlaneWidth; i += ppi->HFragPixels )
        {
            // Skip a group of 8 pixels if the assosciated fragment has no pixels of interest or
            // if EBO is enabled and a breakout condition is met.
            if ( *DispFragPtr == CANDIDATE_BLOCK )
            {
                for ( j = 0; j < ppi->HFragPixels; j++ )
                {
					changed_locals = 0;

                    // If the pixel itself has changed
                    if ( PixelsChangedPtr1[1] )
                    {
						if ( RowType == FIRST_ROW )
						{
							if ( (i > 0) || (j > 0) )
							{
								changed_locals += PixelsChangedPtr1[0];
								changed_locals += PixelsChangedPtr2[0];
							}

							changed_locals += PixelsChangedPtr2[1];

							if ( (i + j) < LastRowIndex )
							{
								changed_locals += PixelsChangedPtr1[2];
								changed_locals += PixelsChangedPtr2[2];
							}
						}
						else	// Last row
						{
							if ( (i > 0) || (j > 0 ) )
							{
								changed_locals += PixelsChangedPtr0[0];
								changed_locals += PixelsChangedPtr1[0];
							}

							changed_locals += PixelsChangedPtr0[1];

							if ( (i + j) < LastRowIndex )
							{
								changed_locals += PixelsChangedPtr0[2];
								changed_locals += PixelsChangedPtr1[2];
							}
						}

                        // Store the number of changed locals
                        *ChLocalsPtr |= changed_locals;
                    }

                    // Increment to next pixel in the row
                    ChLocalsPtr++;
					PixelsChangedPtr0++;
					PixelsChangedPtr1++;
					PixelsChangedPtr2++;
                }
            }
            else
            {
                if ( *DispFragPtr > BLOCK_NOT_CODED )
                {
                    ((UINT32 *)ChLocalsPtr)[0] = ((UINT32 *)ChlocalsDummyData)[0];
                    ((UINT32 *)ChLocalsPtr)[1] = ((UINT32 *)ChlocalsDummyData)[1];
                }

                // Step pointers
                ChLocalsPtr += ppi->HFragPixels;
				PixelsChangedPtr0 += ppi->HFragPixels;
				PixelsChangedPtr1 += ppi->HFragPixels;
				PixelsChangedPtr2 += ppi->HFragPixels;
            }

            // Move on to next fragment.
			DispFragPtr++;    
        }
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     NoiseScoreRow
 *
 *  INPUTS        :     UINT8  * PixelMapPtr.
 *                      INT16  * YUVDiffsPtr,
 *                      UINT8  * PixelNoiseScorePtr
 *                      UINT32 * FragScorePtr
 *                      INT8   * DispFragPtr
 *                      INT32  * RowDiffsPtr 
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     Calculates the noise scores for a row of pixels.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
void NoiseScoreRow( PP_INSTANCE *ppi, UINT8  * PixelMapPtr, UINT8 * ChLocalsPtr, 
				    INT16  * YUVDiffsPtr, 
                    UINT8  * PixelNoiseScorePtr, 
                    UINT32 * FragScorePtr, 
					INT8   * DispFragPtr,
                    INT32  * RowDiffsPtr )
{ 
    INT32 i,j;
    UINT8  changed_locals = 0; 
    INT32  Score;
    UINT32 FragScore;
    INT32  AbsDiff;

    // For each pixel in the row
    for ( i = 0; i < ppi->PlaneWidth; i += ppi->HFragPixels )
    {
        // Skip a group of 8 pixels if the assosciated fragment has no pixels of interest or
        // if EBO is enabled and a breakout condition is met.
        if ( *DispFragPtr == CANDIDATE_BLOCK )
        {
            // Reset the cumulative fragment score.
            FragScore = 0;

            // Pixels grouped along the row into fragments
            for ( j = 0; j < ppi->HFragPixels; j++ )
            {
                if ( PixelMapPtr[j] )
                {
                    AbsDiff = (INT32)( abs(YUVDiffsPtr[j]) );
                    changed_locals = ChLocalsPtr[j];

                    // Give this pixel a score based on changed locals and level of its own change.
                    Score = (1 + ((INT32)(changed_locals + ppi->NoiseScoreBoostTable[AbsDiff]) - ppi->NoiseSupLevel));  

					// For no zero scores adjust by a level based score multiplier.
					if ( Score > 0 )
					{
						Score = (INT32)( (double)Score * ppi->AbsDiff_ScoreMultiplierTable[AbsDiff] );
						if ( Score < 1 )
							Score = 1;
					}
					else
					{
						// Set -ve values to 0
						Score = 0;

						// If there are no changed locals then clear the pixel changed flag and
						// decrement the pixels changed in fragment count to speed later stages.
						if ( changed_locals == 0 )
						{
							PixelMapPtr[j] = 0; 
							*RowDiffsPtr -= 1;
						}
					}

                    // Update the pixel scores etc.
                    PixelNoiseScorePtr[j] = (UINT8)Score;
                    FragScore += (UINT32)Score;
                }
            }

            // Add fragment score (with plane correction factor) into main data structure
            *FragScorePtr += (INT32)(FragScore * ppi->YUVPlaneCorrectionFactor);

			// If score is greater than trip threshold then mark blcok for update.
			if ( *FragScorePtr > ppi->BlockThreshold )
			{
				*DispFragPtr = BLOCK_CODED_LOW;
			}
        }

        // Increment the various pointers
        FragScorePtr++;
		DispFragPtr++;
        PixelNoiseScorePtr += ppi->HFragPixels;
        PixelMapPtr += ppi->HFragPixels;
        ChLocalsPtr += ppi->HFragPixels;
        YUVDiffsPtr += ppi->HFragPixels;
    }
}


/****************************************************************************
 * 
 *  ROUTINE       :     PrimaryEdgeScoreRow
 *
 *  INPUTS        :     UINT8  * PixelMapPtr.
 *                      INT16  * YUVDiffsPtr,
 *                      UINT32 * FragScorePtr
 *                      INT8   * DispFragPtr,
 *                      UINT8    RowType
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     Calculates the primary edge scores for a row of pixels.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
void PrimaryEdgeScoreRow( PP_INSTANCE *ppi, UINT8  * ChangedLocalsPtr, INT16 * YUVDiffsPtr, 
                          UINT8  * PixelNoiseScorePtr, 
                          UINT32 * FragScorePtr,
						  INT8   * DispFragPtr,
                          UINT8    RowType )
{ 
    UINT32 BodyNeighbours;
    UINT32 AbsDiff;
    UINT8  changed_locals = 0; 
    INT32  Score;
    UINT32 FragScore;
	UINT8  * CHLocalsPtr0;
	UINT8  * CHLocalsPtr1;
	UINT8  * CHLocalsPtr2;
    INT32  i,j;
	INT32  LastRowIndex = ppi->PlaneWidth - 1;

	// Set up  pointers into the current previous and next row of the changed locals data structure.
	CHLocalsPtr0 = ChangedLocalsPtr - ppi->PlaneWidth;
	if ( CHLocalsPtr0 < ppi->ChLocals )
		CHLocalsPtr0 += ppi->ChLocalsCircularBufferSize;
	CHLocalsPtr0 -= 1;	
	
	CHLocalsPtr1 = ChangedLocalsPtr - 1;
	
	CHLocalsPtr2 = ChangedLocalsPtr + ppi->PlaneWidth;
	if ( CHLocalsPtr2 >= (ppi->ChLocals + ppi->ChLocalsCircularBufferSize) )
		CHLocalsPtr2 -= ppi->ChLocalsCircularBufferSize;
	CHLocalsPtr2 -= 1;	


    /* The defining rule used here is as follows. */
    /* An edge pixels has 3-5 changed locals. */
    /* And one or more of these changed locals has itself got 7-8 changed locals. */

    if ( RowType == NOT_EDGE_ROW )
    {
		/* Loop for all pixels in the row. */
		for ( i = 0; i < ppi->PlaneWidth; i += ppi->HFragPixels )
		{
			// Does the fragment contain anything interesting to work with.
			if ( *DispFragPtr == CANDIDATE_BLOCK )
			{
				// Reset the cumulative fragment score.
				FragScore = 0;

				// Pixels grouped along the row into fragments
				for ( j = 0; j < ppi->HFragPixels; j++ )
				{
					// How many changed locals has the current pixel got.
					changed_locals = ChangedLocalsPtr[j];

					// Is the pixel a suitable candidate
					if ( (changed_locals > 2) && (changed_locals < 6) )                   
					{
						// The pixel may qualify... have a closer look. 
						BodyNeighbours = 0;

						// Count the number of "BodyNeighbours" .. Pixels
						//  that have 7 or more changed neighbours. 
						if ( (i > 0) || (j > 0 ) )
						{
							if ( CHLocalsPtr0[0] >= 7 )
								BodyNeighbours++;
							if ( CHLocalsPtr1[0] >= 7 )
								BodyNeighbours++;
							if ( CHLocalsPtr2[0] >= 7 )
								BodyNeighbours++;
						}

						if ( CHLocalsPtr0[1] >= 7 )
							BodyNeighbours++;
						if ( CHLocalsPtr2[1] >= 7 )
							BodyNeighbours++;

						if ( (i + j) < LastRowIndex )
						{
							if ( CHLocalsPtr0[2] >= 7 )
								BodyNeighbours++;
							if ( CHLocalsPtr1[2] >= 7 )
								BodyNeighbours++;
							if ( CHLocalsPtr2[2] >= 7 )
								BodyNeighbours++;
						}

						if ( BodyNeighbours > 0 )
						{
							AbsDiff = abs( YUVDiffsPtr[j] );
							Score = (INT32)( (double)(BodyNeighbours * BodyNeighbourScore) * 
								             ppi->AbsDiff_ScoreMultiplierTable[AbsDiff] );
							if ( Score < 1 )
								Score = 1;

							/* Increment the score by a value determined by the number of body neighbours. */
							PixelNoiseScorePtr[j] += (UINT8)Score;  
							FragScore += (UINT32)Score;
						}
					}

					// Increment pointers into changed locals buffer
					CHLocalsPtr0 ++;
					CHLocalsPtr1 ++;
					CHLocalsPtr2 ++;
				}

				// Add fragment score (with plane correction factor) into main data structure
				*FragScorePtr += (INT32)(FragScore * ppi->YUVPlaneCorrectionFactor);

				// If score is greater than trip threshold then mark blcok for update.
				if ( *FragScorePtr > ppi->BlockThreshold )
				{
					*DispFragPtr = BLOCK_CODED_LOW;
				}

			}
			else   // Nothing to do for this fragment group
			{
				// Advance pointers into changed locals buffer
				CHLocalsPtr0 += ppi->HFragPixels;
				CHLocalsPtr1 += ppi->HFragPixels;
				CHLocalsPtr2 += ppi->HFragPixels;
			}

			// Increment the various pointers
			FragScorePtr++;
			DispFragPtr++;
			PixelNoiseScorePtr += ppi->HFragPixels;
			ChangedLocalsPtr += ppi->HFragPixels;
			YUVDiffsPtr += ppi->HFragPixels;
		}  
	}
	else		// This is either the top or bottom row of pixels in a plane.
    {
		/* Loop for all pixels in the row. */
		for ( i = 0; i < ppi->PlaneWidth; i += ppi->HFragPixels )
		{
			// Does the fragment contain anything interesting to work with.
			if ( *DispFragPtr == CANDIDATE_BLOCK )
			{
				// Reset the cumulative fragment score.
				FragScore = 0;

				// Pixels grouped along the row into fragments
				for ( j = 0; j < ppi->HFragPixels; j++ )
				{
					// How many changed locals has the current pixel got.
					changed_locals = ChangedLocalsPtr[j];

					// Is the pixel a suitable candidate
					if ( (changed_locals > 2) && (changed_locals < 6) )                   
					{
						/* The pixel may qualify... have a closer look. */
						BodyNeighbours = 0;

						/* Count the number of "BodyNeighbours" .. Pixels
						*  that have 7 or more changed neighbours. */
						if ( RowType == LAST_ROW )
						{
							// Test for cases where it could be the first pixel on the line
							if ( (i > 0) || (j > 0) )
							{
								if ( CHLocalsPtr0[0] >= 7 )
									BodyNeighbours++;
								if ( CHLocalsPtr1[0] >= 7 )
									BodyNeighbours++;
							}

							if ( CHLocalsPtr0[1] >= 7 )
								BodyNeighbours++;

							// Test for the end of line case
 							if ( (i + j) < LastRowIndex )
							{
								if ( CHLocalsPtr0[2] >= 7 )
									BodyNeighbours++;

								if ( CHLocalsPtr1[2] >= 7 )
									BodyNeighbours++;
							}
						}
						else  // FIRST ROW
						{
							// Test for cases where it could be the first pixel on the line
							if ( (i > 0) || (j > 0) )
							{
								if ( CHLocalsPtr1[0] >= 7 )
									BodyNeighbours++;
								if ( CHLocalsPtr2[0] >= 7 )
									BodyNeighbours++;
							}

							// Test for the end of line case
							if ( CHLocalsPtr2[1] >= 7 )
								BodyNeighbours++;
                    
 							if ( (i + j) < LastRowIndex )
							{
								if ( CHLocalsPtr1[2] >= 7 )
									BodyNeighbours++;
								if ( CHLocalsPtr2[2] >= 7 )
									BodyNeighbours++;
							}
						}

						// Allocate a score according to the number of Body neighbours.
						if ( BodyNeighbours > 0 )
						{
							AbsDiff = abs( YUVDiffsPtr[j] );
							Score = (INT32)( (double)(BodyNeighbours * BodyNeighbourScore) * 
								             ppi->AbsDiff_ScoreMultiplierTable[AbsDiff] );
							if ( Score < 1 )
								Score = 1;

							PixelNoiseScorePtr[j] += (UINT8)Score;  
							FragScore += (UINT32)Score;
						}
					}

					// Increment pointers into changed locals buffer
					CHLocalsPtr0 ++;
					CHLocalsPtr1 ++;
					CHLocalsPtr2 ++;
				}

				// Add fragment score (with plane correction factor) into main data structure
				*FragScorePtr += (INT32)(FragScore * ppi->YUVPlaneCorrectionFactor);

				// If score is greater than trip threshold then mark blcok for update.
				if ( *FragScorePtr > ppi->BlockThreshold )
				{
					*DispFragPtr = BLOCK_CODED_LOW;
				}

			}
			else   // Nothing to do for this fragment group
			{
				// Advance pointers into changed locals buffer
				CHLocalsPtr0 += ppi->HFragPixels;
				CHLocalsPtr1 += ppi->HFragPixels;
				CHLocalsPtr2 += ppi->HFragPixels;
			}

			// Increment the various pointers
			FragScorePtr++;
			DispFragPtr++;
			PixelNoiseScorePtr += ppi->HFragPixels;
			ChangedLocalsPtr += ppi->HFragPixels;
			YUVDiffsPtr += ppi->HFragPixels;
		}  
	}
}



/****************************************************************************
 * 
 *  ROUTINE       :     LineSearchScoreRow
 *
 *  INPUTS        :     UINT8  * ChangedLocalsPtr.
 *                      INT16  * YUVDiffsPtr,
 *                      UINT32 * FragScorePtr
 *                      UINT8    RowNumber
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     Calculates the line match scores for a row of pixels.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
void LineSearchScoreRow( PP_INSTANCE *ppi, UINT8  * ChangedLocalsPtr, INT16 * YUVDiffsPtr, 
                         UINT8  * PixelNoiseScorePtr, 
                         UINT32 * FragScorePtr, 
						 INT8   * DispFragPtr,
                         INT32    RowNumber )
{ 
    UINT32 AbsDiff;
    UINT8  changed_locals = 0; 
    INT32  Score;
    UINT32 FragScore;
    INT32  i,j;

    /* The defining rule used here is as follows. */
    /* An edge pixels has 2-5 changed locals. */
    /* And one or more of these changed locals has itself got 7-8 changed locals. */

    /* Loop for all pixels in the row. */
    for ( i = 0; i < ppi->PlaneWidth; i += ppi->HFragPixels )
    {
        // Does the fragment contain anything interesting to work with.
        if ( *DispFragPtr == CANDIDATE_BLOCK )
        {
            // Reset the cumulative fragment score.
            FragScore = 0;

            // Pixels grouped along the row into fragments
            for ( j = 0; j < ppi->HFragPixels; j++ )
            {
                // How many changed locals has the current pixel got.
                changed_locals = ChangedLocalsPtr[j];

                // Is the pixel a suitable candidate for edge enhancement
                if ( (changed_locals > 1) && (changed_locals < 6) &&
                     (PixelNoiseScorePtr[j] < LineSearchTripTresh) )                   
                {
                    Score = (INT32)LineSearchScorePixel( ppi, &ChangedLocalsPtr[j], RowNumber, i+j );

                    if ( Score )
                    {
						AbsDiff = abs( YUVDiffsPtr[j] );
						Score = (INT32)( (double)Score * ppi->AbsDiff_ScoreMultiplierTable[AbsDiff] );
						if ( Score < 1 )
							Score = 1;

                        PixelNoiseScorePtr[j] += (UINT8)Score;  
                        FragScore += (UINT32)Score;
                    }
                }
            }
               
            // Add fragment score (with plane correction factor) into main data structure
            *FragScorePtr += (INT32)(FragScore * ppi->YUVPlaneCorrectionFactor);

			// If score is greater than trip threshold then mark blcok for update.
			if ( *FragScorePtr > ppi->BlockThreshold )
			{
				*DispFragPtr = BLOCK_CODED_LOW;
			}
        }

        // Increment the various pointers
        FragScorePtr++;
		DispFragPtr++;
        PixelNoiseScorePtr += ppi->HFragPixels;
        ChangedLocalsPtr += ppi->HFragPixels;
        YUVDiffsPtr += ppi->HFragPixels;

    }
}


/****************************************************************************
 * 
 *  ROUTINE       :     LineSearchScorePixel 
 *
 *  INPUTS        :     UINT32  ChangedLocalsPtr     (this pixels index.)
 *                      INT32   RowNumber			 (Row number)
 *                      INT32   ColNumber            (Column number within a row)
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     A pixel line search score
 *
 *  FUNCTION      :     Returns a Line Search score for a pixel.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
UINT8 LineSearchScorePixel( PP_INSTANCE *ppi, UINT8 * ChangedLocalsPtr, INT32 RowNumber, INT32 ColNumber )
{                   
    UINT32 line_length = 0; 
    UINT32 line_length2 = 0; 
    UINT32 line_length_score = 0; 
    UINT32 tmp_line_length = 0; 
    UINT32 tmp_line_length2 = 0;  

	// Look UP and Down
    PixelLineSearch( ppi, ChangedLocalsPtr, RowNumber, ColNumber, UP, &tmp_line_length );

	if (tmp_line_length < ppi->MaxLineSearchLen) 
	{
		// Look DOWN
		PixelLineSearch( ppi, ChangedLocalsPtr, RowNumber, ColNumber, DOWN, &tmp_line_length2 );
	    line_length = tmp_line_length + tmp_line_length2 - 1; 

	    if ( line_length > ppi->MaxLineSearchLen )
	        line_length = ppi->MaxLineSearchLen;
	}    
	else
	    line_length = tmp_line_length; 

	// If no max length line found then look left and right                
	if ( line_length < ppi->MaxLineSearchLen )
	{   
	    tmp_line_length = 0;
	    tmp_line_length2 = 0;
    
	    PixelLineSearch( ppi, ChangedLocalsPtr, RowNumber, ColNumber, LEFT,  &tmp_line_length );
	    if (tmp_line_length < ppi->MaxLineSearchLen)
	    {
	        PixelLineSearch( ppi, ChangedLocalsPtr, RowNumber, ColNumber, RIGHT,  &tmp_line_length2 ); 
	        line_length2 = tmp_line_length + tmp_line_length2 - 1; 

	        if ( line_length2 > ppi->MaxLineSearchLen )
	            line_length2 = ppi->MaxLineSearchLen;
	    }    
	    else
	        line_length2 = tmp_line_length; 

	}

	/* Take the largest line length */
	if ( line_length2 > line_length )
	    line_length = line_length2;

	/* Create line length score */
   	line_length_score = LineLengthScores[line_length];

    return (UINT8)line_length_score;
}



/****************************************************************************
 *                                  
 *  ROUTINE       :     PixelLineSearch
 *
 *  INPUTS        :     UINT8 * ChangedLocalsPtr  (Map entry for this pixel)
 *                      INT32   RowNumber		  (Row number)
 *                      INT32   ColNumber         (Column number within a row)
 *                      UINT8   direction
 *
 *  OUTPUTS       :     UINT8 * line_length
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Recursive function for tracking along a line of pixels
 *                      obeying a specific set of rules
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void PixelLineSearch( PP_INSTANCE *ppi, UINT8 * ChangedLocalsPtr, INT32 RowNumber, INT32 ColNumber, UINT8 direction, UINT32 * line_length )
{   
    // Exit if the pixel does not qualify or we have fallen off the edge    
	// of either the image plane or the row.
    if ( ((*ChangedLocalsPtr) <= 1) ||
         ((*ChangedLocalsPtr) >= 6) ||
         (RowNumber < 0) ||
         (RowNumber >= ppi->PlaneHeight) ||
		 (ColNumber < 0) ||
		 (ColNumber >= ppi->PlaneWidth) )
    {
        // If not then it isn't part of any line.
        return;
    }

    if (*line_length < ppi->MaxLineSearchLen)
    {   
        UINT32 TmpLineLength; 
        UINT32 BestLineLength;
		UINT8 * search_ptr;

        // Increment the line length to include this pixel. 
        *line_length += 1;
        BestLineLength = *line_length;
         
        // Continue search 
        // up  
        if ( direction == UP )
        {
            TmpLineLength = *line_length;

			search_ptr = ChangedLocalsPtr - ppi->PlaneWidth;
			if ( search_ptr < ppi->ChLocals )
				search_ptr += ppi->ChLocalsCircularBufferSize;

            PixelLineSearch( ppi, search_ptr, RowNumber - 1, ColNumber, direction, &TmpLineLength );    
        
            if ( TmpLineLength > BestLineLength )
                BestLineLength = TmpLineLength;
        }
        
        // up and left    
        if ( (BestLineLength < ppi->MaxLineSearchLen) && ((direction == UP) || (direction == LEFT)) )
        {   
            TmpLineLength = *line_length;
            
			search_ptr = ChangedLocalsPtr - ppi->PlaneWidth;
			if ( search_ptr < ppi->ChLocals )
				search_ptr += ppi->ChLocalsCircularBufferSize;
			search_ptr -= 1;

			PixelLineSearch( ppi, search_ptr, RowNumber - 1, ColNumber - 1, direction,  &TmpLineLength );    
            
            if ( TmpLineLength > BestLineLength )
                BestLineLength = TmpLineLength;
        } 
        
        // up and right
        if ( (BestLineLength < ppi->MaxLineSearchLen) && ((direction == UP) || (direction == RIGHT)) )
        {   
            TmpLineLength = *line_length;

			search_ptr = ChangedLocalsPtr - ppi->PlaneWidth;
			if ( search_ptr < ppi->ChLocals )
				search_ptr += ppi->ChLocalsCircularBufferSize;
			search_ptr += 1;

            PixelLineSearch( ppi, search_ptr, RowNumber - 1, ColNumber + 1, direction, &TmpLineLength );   
            
            if ( TmpLineLength > BestLineLength )
                BestLineLength = TmpLineLength;
        }
        
        // left
        if ( (BestLineLength < ppi->MaxLineSearchLen) && ( direction == LEFT ) )
        {   
            TmpLineLength = *line_length;
            PixelLineSearch( ppi, ChangedLocalsPtr - 1, RowNumber, ColNumber - 1, direction, &TmpLineLength );    
                
            if ( TmpLineLength > BestLineLength )
                BestLineLength = TmpLineLength;
        }      
              
        // right
        if ( (BestLineLength < ppi->MaxLineSearchLen) && ( direction == RIGHT ) )
        {   
            TmpLineLength = *line_length;
            PixelLineSearch( ppi, ChangedLocalsPtr + 1, RowNumber, ColNumber + 1, direction, &TmpLineLength );    
                
            if ( TmpLineLength > BestLineLength )
                BestLineLength = TmpLineLength;
        }
        
        // Down...            
        if ( BestLineLength < ppi->MaxLineSearchLen )
        {   
            TmpLineLength = *line_length;
            // down
            if ( direction == DOWN )
            {
				search_ptr = ChangedLocalsPtr + ppi->PlaneWidth;
				if ( search_ptr >= (ppi->ChLocals + ppi->ChLocalsCircularBufferSize) )
					search_ptr -= ppi->ChLocalsCircularBufferSize;

                PixelLineSearch( ppi, search_ptr, RowNumber + 1, ColNumber, direction, &TmpLineLength );    
                
                if ( TmpLineLength > BestLineLength )
                    BestLineLength = TmpLineLength;
            }
            

            // down and left    
            if ( (BestLineLength < ppi->MaxLineSearchLen) && ((direction == DOWN) || (direction == LEFT)) )
            {   
                TmpLineLength = *line_length;
				
				search_ptr = ChangedLocalsPtr + ppi->PlaneWidth;
				if ( search_ptr >= (ppi->ChLocals + ppi->ChLocalsCircularBufferSize) )
					search_ptr -= ppi->ChLocalsCircularBufferSize;
				search_ptr -= 1;

                PixelLineSearch( ppi, search_ptr, RowNumber + 1, ColNumber - 1, direction, &TmpLineLength );    
                
                if ( TmpLineLength > BestLineLength )
                    BestLineLength = TmpLineLength;
            } 
            
            // down and right
            if ( (BestLineLength < ppi->MaxLineSearchLen) && ((direction == DOWN) || (direction == RIGHT)) )
            {   
                TmpLineLength = *line_length;

				search_ptr = ChangedLocalsPtr + ppi->PlaneWidth;
				if ( search_ptr >= (ppi->ChLocals + ppi->ChLocalsCircularBufferSize) )
					search_ptr -= ppi->ChLocalsCircularBufferSize;
				search_ptr += 1;
                
				PixelLineSearch( ppi, search_ptr, RowNumber + 1, ColNumber + 1, direction, &TmpLineLength );   
                
                if ( TmpLineLength > BestLineLength )
                    BestLineLength = TmpLineLength;
            }
        }    
        
        // Note the search value for this pixel.  
        *line_length = BestLineLength;
    }

}



/****************************************************************************
 * 
 *  ROUTINE       :     ScanCalcPixelIndexTable
 *
 *  INPUTS        :     Nonex.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Initialises the pixel index table used in the scan module.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ScanCalcPixelIndexTable(PP_INSTANCE *ppi)
{
    UINT32 i;
    UINT32 * PixelIndexTablePtr = ppi->ScanPixelIndexTable;
    
    /* If appropriate add on extra inices for U and V planes. */
    for ( i = 0; i < (ppi->ScanYPlaneFragments); i++ )
    {
        PixelIndexTablePtr[ i ] = ((i / ppi->ScanHFragments) * ppi->VFragPixels * ppi->ScanConfig.VideoFrameWidth);  
        PixelIndexTablePtr[ i ] += ((i % ppi->ScanHFragments) * ppi->HFragPixels);
    }

    PixelIndexTablePtr = &ppi->ScanPixelIndexTable[ppi->ScanYPlaneFragments];

    for ( i = 0; i < (ppi->ScanUVPlaneFragments * 2); i++ )
    {
        PixelIndexTablePtr[ i ] =  ((i / (ppi->ScanHFragments >> 1) ) * 
                                   (ppi->VFragPixels * (ppi->ScanConfig.VideoFrameWidth >> 1)) );   
        PixelIndexTablePtr[ i ] += ((i % (ppi->ScanHFragments >> 1) ) * ppi->HFragPixels) + ppi->YFramePixels;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     SetVcapLevelOffset
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Configures VCAP parameters to one of a set of pre-defined
 *                      alternatives.
 *
 *  SPECIAL NOTES :     None.  
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void SetVcapLevelOffset( PP_INSTANCE *ppi, INT32 Level ) 
{
    switch ( Level )
    {
    case 0:
	    ppi->SRFGreyThresh = 1; 
	    ppi->SRFColThresh = 1; 
	    ppi->NoiseSupLevel = 2;
	    ppi->SgcLevelThresh = 1; 
	    ppi->SuvcLevelThresh = 1; 
	    ppi->GrpLowSadThresh = 6;
	    ppi->GrpHighSadThresh = 24;
	    ppi->PrimaryBlockThreshold = 2;
	    ppi->SgcThresh = 10;
	    
		ppi->PAKEnabled = FALSE;
        break;

    case 1:
	    ppi->SRFGreyThresh = 2; 
	    ppi->SRFColThresh = 2; 
	    ppi->NoiseSupLevel = 2;
	    ppi->SgcLevelThresh = 2; 
	    ppi->SuvcLevelThresh = 2; 
	    ppi->GrpLowSadThresh = 8; 
	    ppi->GrpHighSadThresh = 32;
	    ppi->PrimaryBlockThreshold = 5;
	    ppi->SgcThresh = 12; 

		ppi->PAKEnabled = TRUE;
        break;
        
    case 2:                         // Default VP3 settings
	    ppi->SRFGreyThresh = 3; 
	    ppi->SRFColThresh = 3;
	    ppi->NoiseSupLevel = 2;
	    ppi->SgcLevelThresh = 2;
	    ppi->SuvcLevelThresh = 2;
	    ppi->GrpLowSadThresh = 8;
	    ppi->GrpHighSadThresh = 32;		
	    ppi->PrimaryBlockThreshold = 5;
	    ppi->SgcThresh = 16;

		ppi->PAKEnabled = TRUE;
        break;

    case 3:
		ppi->SRFGreyThresh = 4;
	    ppi->SRFColThresh = 4;
	    ppi->NoiseSupLevel = 3;
	    ppi->SgcLevelThresh = 3;
	    ppi->SuvcLevelThresh = 3;
	    ppi->GrpLowSadThresh = 10;
	    ppi->GrpHighSadThresh = 48; 
	    ppi->PrimaryBlockThreshold = 5;
	    ppi->SgcThresh = 18;

		ppi->PAKEnabled = TRUE;
        break;

    case 4:
	    ppi->SRFGreyThresh = 5;
	    ppi->SRFColThresh = 5;
	    ppi->NoiseSupLevel = 3;
	    ppi->SgcLevelThresh = 4;
	    ppi->SuvcLevelThresh = 4;
	    ppi->GrpLowSadThresh = 12;
	    ppi->GrpHighSadThresh = 48;
	    ppi->PrimaryBlockThreshold = 5;
	    ppi->SgcThresh = 20;

		ppi->PAKEnabled = TRUE;
        break;

    case 5:                         // Default live narrow band settings                            
	    ppi->SRFGreyThresh = 6;
	    ppi->SRFColThresh = 6;
	    ppi->NoiseSupLevel = 3;
	    ppi->SgcLevelThresh = 4;
	    ppi->SuvcLevelThresh = 4;
	    ppi->GrpLowSadThresh = 12;
	    ppi->GrpHighSadThresh = 64;
	    ppi->PrimaryBlockThreshold = 10;
	    ppi->SgcThresh = 24;

		ppi->PAKEnabled = TRUE;
        break;

    case 6:                         // Default live narrow band settings                            
	    ppi->SRFGreyThresh = 6;
	    ppi->SRFColThresh = 7;
	    ppi->NoiseSupLevel = 3;
	    ppi->SgcLevelThresh = 4;
	    ppi->SuvcLevelThresh = 4;
	    ppi->GrpLowSadThresh = 12;
	    ppi->GrpHighSadThresh = 64;
	    ppi->PrimaryBlockThreshold = 10;
	    ppi->SgcThresh = 24;

		ppi->PAKEnabled = TRUE;
        break;

    default:
	    ppi->SRFGreyThresh = 3; 
	    ppi->SRFColThresh = 3;
	    ppi->NoiseSupLevel = 2;
	    ppi->SgcLevelThresh = 2;
	    ppi->SuvcLevelThresh = 2;
	    ppi->GrpLowSadThresh = 10;
	    ppi->GrpHighSadThresh = 32;		
	    ppi->PrimaryBlockThreshold = 5;
	    ppi->SgcThresh = 16;
		ppi->PAKEnabled = TRUE;
        break;
    }
}


/****************************************************************************
 * 
 *  ROUTINE       :     GetLocalVarianceMultiplier
 *
 *  INPUTS        :     INT16 *   MasterYUVDiffPtr.
 *                      UINT32    PlaneLineLength
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Pixel variance
 *
 *  FUNCTION      :     Calculates a score correction based on local variance
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
double GetLocalVarianceMultiplier( PP_INSTANCE *ppi, INT16 * MasterYUVDiffPtr, UINT32 PlaneLineLength )
{
	INT32	XSum=0;
	INT32	XXSum=0;
	INT32	DiffVal;
    double  LocalVariance;
    double  VarMultiplier;
    INT16 * YUVDiffPtr;

    // Previous row (wrap back to top of buffer if necessary
    YUVDiffPtr = MasterYUVDiffPtr - PlaneLineLength;
    if ( YUVDiffPtr < ppi->yuv_differences )
        YUVDiffPtr += ppi->YuvDiffsCircularBufferSize;
        
    DiffVal = YUVDiffPtr[-1];
	XSum += DiffVal;
	XXSum += DiffVal * DiffVal;
    
    DiffVal = YUVDiffPtr[0];
	XSum += DiffVal;
	XXSum += DiffVal * DiffVal;

    DiffVal = YUVDiffPtr[1];
	XSum += DiffVal;
	XXSum += DiffVal * DiffVal;

    // Current row
    YUVDiffPtr = MasterYUVDiffPtr;
    DiffVal = YUVDiffPtr[-1];
	XSum += DiffVal;
	XXSum += DiffVal * DiffVal;

    DiffVal = YUVDiffPtr[0];
	XSum += DiffVal;
	XXSum += DiffVal * DiffVal;

    DiffVal = YUVDiffPtr[1];
	XSum += DiffVal;
	XXSum += DiffVal * DiffVal;

    // Last row (wrap back around if neeeded
    YUVDiffPtr = MasterYUVDiffPtr + PlaneLineLength;
	if ( YUVDiffPtr > &ppi->yuv_differences[ppi->YuvDiffsCircularBufferSize] )
		YUVDiffPtr -= ppi->YuvDiffsCircularBufferSize;

    DiffVal = YUVDiffPtr[-1];
	XSum += DiffVal;
	XXSum += DiffVal * DiffVal;

    DiffVal = YUVDiffPtr[0];
	XSum += DiffVal;
	XXSum += DiffVal * DiffVal;

    DiffVal = YUVDiffPtr[1];
	XSum += DiffVal;
	XXSum += DiffVal * DiffVal;

	// Compute and return population variance as mis-match metric.
	LocalVariance = ((double)XXSum * 0.1111) - ((double)XSum * (double)XSum * 0.012346);

    if ( LocalVariance > 2 * LowVarianceThresh )
    {
        VarMultiplier = 1.5;
    }
    else if ( LocalVariance > LowVarianceThresh )
    {
        VarMultiplier = 1.0;
    }
    else
    {
        VarMultiplier = 0.5;
    }

    return VarMultiplier;
}

/****************************************************************************
 * 
 *  ROUTINE       :     ScalarRowSAD
 *
 *  INPUTS        :     UINT8 * Src1
 *                      UINT8 * Src2
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     A Sum of the absolute difference value for a row of 4 pixels
 *
 *  FUNCTION      :     Calculates a sum of the absolute difference for one or two groups of
 *                      of 4 pixels. If two groups it returns the larger of the two values.
 *
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
UINT32 ScalarRowSAD( UINT8 * Src1, UINT8 * Src2 )
{
	UINT32 SadValue;
	UINT32 SadValue1;

	SadValue    = abs( Src1[0] - Src2[0] ) + abs( Src1[1] - Src2[1] ) + 
                  abs( Src1[2] - Src2[2] ) + abs( Src1[3] - Src2[3] );

	SadValue1   = abs( Src1[4] - Src2[4] ) + abs( Src1[5] - Src2[5] ) + 
                  abs( Src1[6] - Src2[6] ) + abs( Src1[7] - Src2[7] );

	SadValue = ( SadValue > SadValue1 ) ? SadValue : SadValue1;

	return SadValue;
}


/****************************************************************************
 * 
 *  ROUTINE       :     ScalarColSAD
 *
 *  INPUTS        :     PP_INSTANCE *ppi
 *						UINT8 * Src1
 *                      UINT8 * Src2
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     The maximum 4 pixel column SAD for an 8x8 block.
 *
 *  FUNCTION      :     Calculates a SAD for each 4 pixel column in a block and 
 *						returns the MAX value.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/   
UINT32 ScalarColSAD( PP_INSTANCE *ppi, UINT8 * Src1, UINT8 * Src2 )
{
	UINT32 SadValue[8] = {0,0,0,0,0,0,0,0};
	UINT32 SadValue2[8] = {0,0,0,0,0,0,0,0};
	UINT32 MaxSad = 0;
	UINT32 i;

	for ( i = 0; i < 4; i++ )
	{
		SadValue[0] += abs(Src1[0] - Src2[0]);
		SadValue[1] += abs(Src1[1] - Src2[1]);
		SadValue[2] += abs(Src1[2] - Src2[2]);
		SadValue[3] += abs(Src1[3] - Src2[3]);
		SadValue[4] += abs(Src1[4] - Src2[4]);
		SadValue[5] += abs(Src1[5] - Src2[5]);
		SadValue[6] += abs(Src1[6] - Src2[6]);
		SadValue[7] += abs(Src1[7] - Src2[7]);
		
		Src1 += ppi->PlaneStride;
		Src2 += ppi->PlaneStride;
	}

	for ( i = 0; i < 4; i++ )
	{
		SadValue2[0] += abs(Src1[0] - Src2[0]);
		SadValue2[1] += abs(Src1[1] - Src2[1]);
		SadValue2[2] += abs(Src1[2] - Src2[2]);
		SadValue2[3] += abs(Src1[3] - Src2[3]);
		SadValue2[4] += abs(Src1[4] - Src2[4]);
		SadValue2[5] += abs(Src1[5] - Src2[5]);
		SadValue2[6] += abs(Src1[6] - Src2[6]);
		SadValue2[7] += abs(Src1[7] - Src2[7]);
		
		Src1 += ppi->PlaneStride;
		Src2 += ppi->PlaneStride;
	}

	for ( i = 0; i < 8; i++ )
	{
		if ( SadValue[i] > MaxSad )
			MaxSad = SadValue[i];
		if ( SadValue2[i] > MaxSad )
			MaxSad = SadValue2[i];
	}

	return MaxSad;
}

/****************************************************************************
 * 
 *  ROUTINE       :     ApplyPakLowPass
 *
 *  INPUTS        :     UINT8 * SrcPtr
 *                              central point in kernel.
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Filtered value.
 *
 *  FUNCTION      :     Applies a moderate low pass filter at the given location. 
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
UINT8 ApplyPakLowPass( PP_INSTANCE *ppi, UINT8 * SrcPtr )
{
	UINT8 * SrcPtr1 = SrcPtr - 1;
	UINT8 * SrcPtr0 = SrcPtr1 - ppi->PlaneStride;        // Note the use of stride not width.
	UINT8 * SrcPtr2 = SrcPtr1 + ppi->PlaneStride;

	return  (UINT8)( ( (UINT32)SrcPtr0[0] + (UINT32)SrcPtr0[1] + (UINT32)SrcPtr0[2] +
                       (UINT32)SrcPtr1[0] + (UINT32)SrcPtr1[2] +
                       (UINT32)SrcPtr2[0] + (UINT32)SrcPtr2[1] + (UINT32)SrcPtr2[2] ) >> 3 );

}

