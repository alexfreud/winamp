/****************************************************************************
*
*   Module Title :     PreProcIf.c
*
*   Description  :     Pre-processor dll interface module.
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
*   1.09 PGW 27 Apr 01 Changes to use last frame coded list passed in from codec.
*					   Removed code to set Y from UV.
*   1.08 PGW 28 Feb 01 Removal of history buffer functionality.
*   1.07 PGW 28 Feb 01 Removal of pre-processor output buffer.
*   1.06 JBB 03 Aug 00 Added Malloc Checks
*   1.05 PGW 27 Jul 00 Removed SetVcapParams() plus other housekeeping.
*   1.04 PGW 10 Jul 00 Removed unused functions GetBlockStats(), BlockChangeVariance() 
*					   and GetBlockCategories().
*					   Change interface to YUVAnalyseFrame() to include KF indicator.
*   1.03 PGW 22/06/00  Removed speed specific code.
*   1.02 JBB 30/05/00  Removed hard coded size limits
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

#include <string.h> 
#include "type_aliases.h"
#include "preproc.h"


/****************************************************************************
*  Module constants.
*****************************************************************************
*/        
#define MIN_STEP_THRESH 6

#define VARIANCE_THRESH			200
#define LOW_VARIANCE_THRESH		100
#define HIGH_SCORE				400


/****************************************************************************
*  Explicit Imports
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
 *  ROUTINE       :     ScanYUVInit
 *
 *  INPUTS        :     SCAN_CONFIG_DATA * ScanConfigPtr
 *                          Configuration data.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Initialises the scan process. 
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
extern BOOL PAllocateFrameInfo(PP_INSTANCE * ppi);
BOOL ScanYUVInit( PP_INSTANCE *  ppi, SCAN_CONFIG_DATA * ScanConfigPtr)
{  
    // Test machine specific features such as MMX support
    MachineSpecificConfig(ppi);

	/* Set up the various imported data structure pointers. */
	ppi->ScanConfig.Yuv0ptr = ScanConfigPtr->Yuv0ptr;
	ppi->ScanConfig.Yuv1ptr = ScanConfigPtr->Yuv1ptr;
	ppi->ScanConfig.FragInfo 			=	ScanConfigPtr->FragInfo;		
	ppi->ScanConfig.FragInfoElementSize = 	ScanConfigPtr->FragInfoElementSize; 	
	ppi->ScanConfig.FragInfoCodedMask 	=	ScanConfigPtr->FragInfoCodedMask ;
	
    ppi->ScanConfig.RegionIndex = ScanConfigPtr->RegionIndex;
	ppi->ScanConfig.HFragPixels = ScanConfigPtr->HFragPixels;
	ppi->ScanConfig.VFragPixels = ScanConfigPtr->VFragPixels;

	ppi->ScanConfig.VideoFrameWidth = ScanConfigPtr->VideoFrameWidth;
	ppi->ScanConfig.VideoFrameHeight = ScanConfigPtr->VideoFrameHeight;

	// UV plane sizes.
	ppi->VideoUVPlaneWidth = ScanConfigPtr->VideoFrameWidth / 2;
	ppi->VideoUVPlaneHeight = ScanConfigPtr->VideoFrameHeight / 2;

    /* Note the size of the entire frame and plaes in pixels. */
    ppi->YFramePixels = ppi->ScanConfig.VideoFrameWidth * ppi->ScanConfig.VideoFrameHeight;
    ppi->UVFramePixels = ppi->VideoUVPlaneWidth * ppi->VideoUVPlaneHeight;
    ppi->TotFramePixels = ppi->YFramePixels + (2 * ppi->UVFramePixels);

	/* Work out various fragment related values. */
	ppi->ScanYPlaneFragments = ppi->YFramePixels / (ppi->HFragPixels * ppi->VFragPixels);
	ppi->ScanUVPlaneFragments = ppi->UVFramePixels / (ppi->HFragPixels * ppi->VFragPixels);;
    ppi->ScanHFragments = ppi->ScanConfig.VideoFrameWidth / ppi->HFragPixels;
    ppi->ScanVFragments = ppi->ScanConfig.VideoFrameHeight / ppi->VFragPixels;
	ppi->ScanFrameFragments = ppi->ScanYPlaneFragments + (2 * ppi->ScanUVPlaneFragments);

    if(!PAllocateFrameInfo(ppi))
        return FALSE;

	/* Set up the scan pixel index table. */
	ScanCalcPixelIndexTable(ppi);

	/* Initialise scan arrays */
	InitScanMapArrays(ppi);

	return TRUE;
}


/****************************************************************************
 * 
 *  ROUTINE       :     YUVAnalyseFrame
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Number of "output" blocks to be updated.
 *
 *  FUNCTION      :     Scores the fragments for the YUV planes 
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
UINT32 YUVAnalyseFrame( PP_INSTANCE *ppi, UINT32 * KFIndicator ) 
{  
    UINT32 UpdatedYBlocks = 0;
    UINT32 UpdatedUVBlocks = 0;
	UINT32 i;

	/* Initialise the map arrays. */
	InitScanMapArrays(ppi);

	/**********  PGW 27/APR/2001 ***********/
	// 	If the block is already marked as coded in the input block map then 
	//  mark it as coded here to avoid unnecessary pre-processor work.
	for ( i = 0; i < ppi->ScanFrameFragments; i++ )
	{

		if ( blockCoded(i) )
			ppi->ScanDisplayFragments[i] = BLOCK_ALREADY_MARKED_FOR_CODING;
	}

    // If the motion level in the previous frame was high then adjust the high and low SAD 
    // thresholds to speed things up.
    ppi->ModifiedGrpLowSadThresh = ppi->GrpLowSadThresh;
    ppi->ModifiedGrpHighSadThresh = ppi->GrpHighSadThresh;
    // testing force every block with any change to get coded
    //ppi->ModifiedGrpHighSadThresh = 0;

    // Set up the internal plane height and width variables.
    ppi->VideoYPlaneWidth = ppi->ScanConfig.VideoFrameWidth;
    ppi->VideoYPlaneHeight = ppi->ScanConfig.VideoFrameHeight;
	ppi->VideoUVPlaneWidth = ppi->ScanConfig.VideoFrameWidth / 2;
	ppi->VideoUVPlaneHeight = ppi->ScanConfig.VideoFrameHeight / 2;

    // To start with *** TBD **** the stides will be set from the widths
    ppi->VideoYPlaneStride = ppi->VideoYPlaneWidth;
    ppi->VideoUPlaneStride = ppi->VideoUVPlaneWidth;
    ppi->VideoVPlaneStride = ppi->VideoUVPlaneWidth;
    
    // Set up the plane pointers
    ppi->YPlanePtr0 = ppi->ScanConfig.Yuv0ptr;
    ppi->YPlanePtr1 = ppi->ScanConfig.Yuv1ptr;
    ppi->UPlanePtr0 = (ppi->ScanConfig.Yuv0ptr + ppi->YFramePixels);
    ppi->UPlanePtr1 = (ppi->ScanConfig.Yuv1ptr + ppi->YFramePixels);
    ppi->VPlanePtr0 = (ppi->ScanConfig.Yuv0ptr + ppi->YFramePixels + ppi->UVFramePixels);
    ppi->VPlanePtr1 = (ppi->ScanConfig.Yuv1ptr + ppi->YFramePixels + ppi->UVFramePixels);

    // Ananlyse the U and V palnes. 
    AnalysePlane( ppi, ppi->UPlanePtr0, ppi->UPlanePtr1, ppi->ScanYPlaneFragments, ppi->VideoUVPlaneWidth, ppi->VideoUVPlaneHeight, ppi->VideoUPlaneStride );
    AnalysePlane( ppi, ppi->VPlanePtr0, ppi->VPlanePtr1, (ppi->ScanYPlaneFragments + ppi->ScanUVPlaneFragments), ppi->VideoUVPlaneWidth, ppi->VideoUVPlaneHeight, ppi->VideoVPlaneStride );

    // Now analyse the Y plane.
    AnalysePlane( ppi, ppi->YPlanePtr0, ppi->YPlanePtr1, 0, ppi->VideoYPlaneWidth, ppi->VideoYPlaneHeight, ppi->VideoYPlaneStride );
    
    // Create an output block map for the calling process. 
	CreateOutputDisplayMap( ppi, ppi->ScanDisplayFragments);
	
	// Set the candidate key frame indicator (0-100)
	*KFIndicator = ppi->KFIndicator;

	// Return the normalised block count (this is actually a motion level 
    // weighting not a true block count).
	return ppi->OutputBlocksUpdated;
}

/****************************************************************************
 * 
 *  ROUTINE       :     SetScanParam
 *
 *  INPUTS        :     ParamID
 *                      ParamValue
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Sets a scan parameter. 
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void SetScanParam( PP_INSTANCE *ppi, UINT32 ParamId, INT32 ParamValue ) 
{  
	switch (ParamId)
	{

	case SCP_SET_VCAP_LEVEL_OFFSET:
		SetVcapLevelOffset(ppi, ParamValue);
        break;

	}

}

