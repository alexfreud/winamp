/****************************************************************************
*
*   Module Title :     BlockMap.c
*
*   Description  :     Contains functions used to create the block map
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
*   1.08 PGW 28 Feb 01 Removal of history buffer mechanism.
*   1.07 PGW 04 Oct 00 Changes to RowBarEnhBlockMap()
*   1.06 JBB 03 Aug 00 Fixed Problem in which rownumber was compared to 
*                      PlaneHFragments instead of PlaneVFragments, added 
*                      statistic output functions
*   1.05 PGW 27/07/00  Experiments with motion score.
*   1.04 JBB 30/05/00  Removed hard coded size limits
*   1.03 PGW 18/02/00  Changed weighting for History blocks. 
*                      Redundant functions deleted.
*					   Deglobalization.
*   1.02 PGW 12/07/99  Changes to reduce uneccessary dependancies. 
*   1.01 PGW 21/06/99  Alter function of RowBarEnhBlockMap() for VFW codec.
*   1.00 PGW 14/06/99  Configuration baseline
*
*****************************************************************************
*/						

/****************************************************************************
*  Header Frames
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include <string.h>

#include "preproc.h"


/****************************************************************************
*  Module constants.
*****************************************************************************
*/  

/****************************************************************************
*  Module Types
*****************************************************************************
*/              

/****************************************************************************
*  Imported Global Variables
*****************************************************************************
*/


/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/

/****************************************************************************
*  Foreward References
*****************************************************************************
*/              


/****************************************************************************
*  Module Statics
*****************************************************************************
*/              


/****************************************************************************
 * 
 *  ROUTINE       :     RowBarEnhBlockMap
 *
 *  INPUTS        :     UINT32 * FragNoiseScorePtr 
 *                      INT8   * FragSgcPtr
 *                      UINT32   RowNumber
 *
 *  OUTPUTS       :     INT8   * UpdatedBlockMapPtr 
 *                      INT8   * BarBlockMapPtr
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     BAR Enhances block map on a row by row basis.
 *
 *  SPECIAL NOTES :     Note special cases for first and last row and first and last
 *                      block in each row. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void RowBarEnhBlockMap( PP_INSTANCE *ppi, 
					    UINT32 * FragScorePtr, 
						INT8   * FragSgcPtr,
						INT8   * UpdatedBlockMapPtr,
						INT8   * BarBlockMapPtr,
						UINT32 RowNumber )
{
	// For boundary blocks relax thresholds
	UINT32 BarBlockThresh = ppi->BlockThreshold / 10;
	UINT32 BarSGCThresh = ppi->BlockSgcThresh / 2;

	INT32 i;

    // Start by blanking the row in the bar block map structure.
	memset( BarBlockMapPtr, BLOCK_NOT_CODED, ppi->PlaneHFragments );

	// First row
	if ( RowNumber == 0 )
	{
        
		// For each fragment in the row.
		for ( i = 0; i < ppi->PlaneHFragments; i ++ )
		{
			// Test for CANDIDATE_BLOCK or CANDIDATE_BLOCK_LOW
			// Uncoded or coded blocks will be ignored.
            if ( UpdatedBlockMapPtr[i] <= CANDIDATE_BLOCK )
			{
				// Is one of the immediate neighbours updated in the main map.
				// Note special cases for blocks at the start and end of rows.
				if ( i == 0 )
				{
                    
					if ( (UpdatedBlockMapPtr[i+1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments+1] > BLOCK_NOT_CODED ) )
					{
						BarBlockMapPtr[i] = BLOCK_CODED_BAR;
					}
                    
				}
				else if ( i == (ppi->PlaneHFragments - 1) )
				{
                    
					if ( (UpdatedBlockMapPtr[i-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments] > BLOCK_NOT_CODED ) )
					{
						BarBlockMapPtr[i] = BLOCK_CODED_BAR;
					}
                    
				}
				else
				{
					if ( (UpdatedBlockMapPtr[i-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments+1] > BLOCK_NOT_CODED ) )
					{
						BarBlockMapPtr[i] = BLOCK_CODED_BAR;
					}
				}
			}
		}
        
	}
	// Last row
    //   Used to read PlaneHFragments
	else if ( RowNumber == (UINT32)(ppi->PlaneVFragments-1))
	{
        
		// For each fragment in the row.
		for ( i = 0; i < ppi->PlaneHFragments; i ++ )
		{
			// Test for CANDIDATE_BLOCK or CANDIDATE_BLOCK_LOW
			// Uncoded or coded blocks will be ignored.
            if ( UpdatedBlockMapPtr[i] <= CANDIDATE_BLOCK )
			{
				// Is one of the immediate neighbours updated in the main map.
				// Note special cases for blocks at the start and end of rows.
				if ( i == 0 )
				{
					if ( (UpdatedBlockMapPtr[i+1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments+1] > BLOCK_NOT_CODED ) )
					{
						BarBlockMapPtr[i] = BLOCK_CODED_BAR;
					}
                
				}
				else if ( i == (ppi->PlaneHFragments - 1) )
				{
					if ( (UpdatedBlockMapPtr[i-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments] > BLOCK_NOT_CODED ) )
					{
						BarBlockMapPtr[i] = BLOCK_CODED_BAR;
					}
				}
				else
				{
					if ( (UpdatedBlockMapPtr[i-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments+1] > BLOCK_NOT_CODED ) )
					{
						BarBlockMapPtr[i] = BLOCK_CODED_BAR;
					}
				}
			}
		}
        
	}
	// All other rows
	else
	{
		// For each fragment in the row.
		for ( i = 0; i < ppi->PlaneHFragments; i ++ )
		{
			// Test for CANDIDATE_BLOCK or CANDIDATE_BLOCK_LOW
			// Uncoded or coded blocks will be ignored.
            if ( UpdatedBlockMapPtr[i] <= CANDIDATE_BLOCK )
			{
				// Is one of the immediate neighbours updated in the main map.
				// Note special cases for blocks at the start and end of rows.
				if ( i == 0 )
				{
                    
					if ( (UpdatedBlockMapPtr[i+1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments+1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments+1] > BLOCK_NOT_CODED ) )
					{
						BarBlockMapPtr[i] = BLOCK_CODED_BAR;
					}
                    
				}
				else if ( i == (ppi->PlaneHFragments - 1) )
				{
                    
					if ( (UpdatedBlockMapPtr[i-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments] > BLOCK_NOT_CODED ) )
					{
						BarBlockMapPtr[i] = BLOCK_CODED_BAR;
					}
                    
				}
				else
				{
					if ( (UpdatedBlockMapPtr[i-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i-ppi->PlaneHFragments+1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments-1] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
						 (UpdatedBlockMapPtr[i+ppi->PlaneHFragments+1] > BLOCK_NOT_CODED ) )
                         
					{
						BarBlockMapPtr[i] = BLOCK_CODED_BAR;
					}
				}
			}
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     BarCopyBack
 *
 *  INPUTS        :     INT8  * BarBlockMapPtr
 *
 *  OUTPUTS       :     INT8  * UpdatedBlockMapPtr 
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Copies BAR blocks back into main block map.
 *
 *  SPECIAL NOTES :     None.
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void BarCopyBack( PP_INSTANCE *ppi, 
				  INT8  * UpdatedBlockMapPtr,
				  INT8  * BarBlockMapPtr )
{
	INT32 i;

	// For each fragment in the row.
	for ( i = 0; i < ppi->PlaneHFragments; i ++ )
	{
		if ( BarBlockMapPtr[i] > BLOCK_NOT_CODED )
		{
			UpdatedBlockMapPtr[i] = BarBlockMapPtr[i];
		}
	}
}


/****************************************************************************
 * 
 *  ROUTINE       :     CreateOutputDisplayMap
 *
 *  INPUTS        :     INT8 *  InternalFragmentsPtr 
 *                              Fragment list using internal format.
 *                      INT8 *  RecentHistoryPtr
 *                              List of blocks that have been marked for update int he last few frames.
 * 
 *                      UINT8 * ExternalFragmentsPtr
 *                              Fragment list using external format.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Creates a block update map in the format expected by the caller.
 *
 *  SPECIAL NOTES :     The output block height and width must be an integer
 *                      multiple of the internal value.  
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void CreateOutputDisplayMap
(
 PP_INSTANCE *ppi, 
 INT8		 *InternalFragmentsPtr
)
{ 
    UINT32 i;
	UINT32 KFScore = 0;
	UINT32 YBand = 	(ppi->ScanYPlaneFragments/8);	// 1/8th of Y image.	

//#define DISPLAY_STATS
#ifdef DISPLAY_STATS
#include <stdio.h>
	{

		FILE * StatsFilePtr;
		StatsFilePtr = fopen( "c:\\display_stats.stt", "a" ); 
		if ( StatsFilePtr )
		{
            int i;
            for(i=0;i<ppi->ScanYPlaneFragments;i++)
            {
                if(i%ppi->ScanHFragments  == 0 )
                    fprintf( StatsFilePtr , "\n");

                fprintf( StatsFilePtr, "%2d", 
                    InternalFragmentsPtr[i]);
            }
            fprintf( StatsFilePtr , "\n");
			fclose( StatsFilePtr );

		}
	}
#endif    
    
	ppi->OutputBlocksUpdated = 0;
    for ( i = 0; i < ppi->ScanFrameFragments; i++ )
    {
		if ( InternalFragmentsPtr[i] > BLOCK_NOT_CODED ) 
        {
            ppi->OutputBlocksUpdated ++;
			setBlockCoded(i)
        }
		else
		{
			setBlockUncoded(i);
		}
    }

	// Now calculate a key frame candidate indicator.
	// This is based upon Y data only and only ignores the top and bottom 1/8 of the image.
	// Also ignore history blocks and BAR blocks.
    ppi->KFIndicator = 0;
    for ( i = YBand; i < (ppi->ScanYPlaneFragments - YBand); i++ )
    {
		if ( InternalFragmentsPtr[i] > BLOCK_CODED_BAR ) 
        {
            ppi->KFIndicator ++;
        }
    }

	// Convert the KF score to a range 0-100
	ppi->KFIndicator = ((ppi->KFIndicator*100)/((ppi->ScanYPlaneFragments*3)/4));
}
