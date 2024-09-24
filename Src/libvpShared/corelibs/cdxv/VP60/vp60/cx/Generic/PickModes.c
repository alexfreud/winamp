/****************************************************************************
*
*   Module Title :     PickModes.c
*
*   Description  :     Coding mode selection functions.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include <math.h>           // For abs()
#include "mcomp.h"
#include "tokenentropy.h"
#include "compdll.h"
#include "decodemode.h"
#include "decodemv.h"
#include "encodemode.h"

/****************************************************************************
*  Imports
****************************************************************************/ 
extern void PredictBlock ( 	CP_INSTANCE *cpi, 	BLOCK_POSITION bp );
extern UINT8 TokenizeFrag_RD (	CP_INSTANCE *cpi, INT16 * RawData, UINT32 Plane, UINT32 *MbCost );
extern INT32 *XX_LUT;
extern void GetQuantizedCoeffsMSE_RD( INT16 * DctCodes,INT16 * Coeffs,INT16 * DequantMatrix,UINT32 *MSE);

extern void PredictDCE 
( 
	CP_INSTANCE *cpi,
	BLOCK_POSITION bp
);

/****************************************************************************
*  Macros
****************************************************************************/        
#define KF_INDICATOR_THRESH (5 << 12)           //was 12800 (3 << 12)

#define EPB					(cpi->ErrorPerBit)

#define MIN_ERR             100
#define MAX_ERR             20000

/****************************************************************************
*  Module Statics
****************************************************************************/         
static const UINT32 IntraThreshTable[Q_TABLE_SIZE] =
{
    47, 46, 45, 40, 39, 38, 37, 36,
    35, 34, 33, 32, 31, 30, 29, 28,
    27, 26, 25, 25, 24, 24, 23, 23,
    22,	21, 21, 20,	19, 19, 18, 18,  
    17, 17, 17, 16, 16, 15, 15, 14,  
    14, 13, 13, 12, 12, 11, 11, 10,
     9,  9,  9,  7,  6,  6,  5,  4, 
     4,  3,  3,  2,  1,  0,  0,  0 
};

static const UINT32 IntraFactors[Q_TABLE_SIZE] = 
{
    128, 128, 128, 128, 128, 128, 128, 128,		
    128, 128, 128, 128, 128, 128, 128, 128,	
    128, 128, 128, 128, 128, 128, 128, 128,	
    128, 128, 128, 128, 128, 128, 128, 128,	
    128, 128, 128, 128, 128, 128, 126, 122,	
    120, 118, 116, 114, 112, 110, 108, 106,
    104, 102, 100,  98,  94,  90,  88,  84,
     80,  76,  72,  64,  56,  48,  32,  32
};

static const UINT32 ErrorPerBit[Q_TABLE_SIZE] = 
{
    300, 250, 200, 180, 170, 160, 150, 145,
    140, 130, 120, 114, 110, 102,  98,  95,
     90,  85,  80,  78,  76,  74,  72,  70,
     68,  64,  62,  58,  56,  54,  52,  50,
     49,  48,  47,  46,  45,  44,  43,  42, 	 
     41,  40,  39,  38,  37,  36,  35,  34,
     33,  33,  32,  31,  30,  27,  24,  19, 
     17,  15,  12,   9,   7,   4,   2,   1
};

static const UINT32 FourModeImprovement[Q_TABLE_SIZE] = 
{
    250, 225, 210, 200, 195, 180, 165, 150,   
    140, 130, 120, 114, 110, 102,  98,  95,
     90,  85,  80,  78,  76,  74,  72,  70,
     68,  64,  62,  58,  56,  54,  52,  50,
     49,  48,  47,  46,  45,  44,  43,  42, 	 
     41,  40,  39,  38,  37,  36,  35,  34,
     33,  33,  32,  31,  30,  27,  24,  19, 
     17,  15,  12,   9,   7,   4,   2,   1
};

static const UINT32 MvEpbCorrectionTable[10] = 
{
	650, 500, 400, 300, 250, 200, 150, 100, 75, 50 
};

/***************** RATE DISTORTION STATIC TABLES *****************/
static const UINT32 RateMult[Q_TABLE_SIZE] = 
{
     700, 650, 600, 550, 450, 450, 400, 375,
     350, 325, 300, 275, 250, 225, 200, 190,
	 180, 170, 160, 151, 142, 134, 126, 119,
	 112, 106, 100,  95,  90,  85,  80,  75,
	  70,  66,  62,  58,  54,  50,  47,  44,
	  41,  38,  35,  33,  31,  29,  27,  25,
	  23,  21,  19,  17,  15,  13,  11,   9,
	  7,    5,   3,   2,   3,   1,   1,   1
};


static const UINT32 RateDiv[Q_TABLE_SIZE] = 
{
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 2, 1, 2, 4
};

// Using the proportion of new mvs in the last frame as a measure of complexity
// this table is used to apply a correction to the rate multiplier used in RD.
// 128 is neutral, higher prefers rate, lower prefers dist.
static const UINT32 RateMultCorrection[10] = 
{
	120,  125,  130,   140,   150,   165,   180,   195,   200,   220
};

static const INT32 RdMvCostCorrection[10] = 
{

	36,  15,  12,   4,   3,   2,   1,   0,   0,   0
};

/****************************************************************************
*
*			RD SPECIFIC CODE
*
*****************************************************************************/

/****************************************************************************
 * 
 *  ROUTINE       : ComputeBlockReconError
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Distortion metric for the block 
 *
 *  FUNCTION      : Computes a reconstruction distortion metric for a block.
 *
 *  SPECIAL NOTES : None 

 *
 ****************************************************************************/
UINT32 ComputeBlockReconError ( CP_INSTANCE *cpi, UINT32 bp)
{
    UINT32 i, j;

	UINT8 *NewDataPtr   = &cpi->yuv1ptr[cpi->pb.mbi.blockDxInfo[bp].Source];
	UINT8 *RefDataPtr1  = &cpi->pb.ThisFrameRecon[cpi->pb.mbi.blockDxInfo[bp].thisRecon];
    INT32  SourceStride = cpi->pb.mbi.blockDxInfo[bp].CurrentSourceStride;
	INT32  ReconStride  = cpi->pb.mbi.blockDxInfo[bp].CurrentReconStride;

    INT32  XXDiff;
    INT32  XXSum = 0;
	INT32  MaxXXDiff = 0;

	static UINT32 MaxDiff = 0;

    // Mode of interpolation chosen based upon on the offset of the second reference pointer
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {
		for ( j=0; j<BLOCK_HEIGHT_WIDTH; j++ )
		{
			XXDiff =  XX_LUT[(int)NewDataPtr[j] - (int)RefDataPtr1[j]];
	        XXSum += XXDiff;

			if ( XXDiff > MaxXXDiff )
				MaxXXDiff = XXDiff;
		}

        // Step to next row of block.
        NewDataPtr  += SourceStride;
        RefDataPtr1 += ReconStride;
    }

	// Compute distortion value
	return  (UINT32)(XXSum + (2 * MaxXXDiff)) << 6;    
}



/****************************************************************************
 * 
 *  ROUTINE       : RdSaveMbContext
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *                  UINT32 MBcol     : Macroblock column number.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Saves the context information for a macro-block.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void RdSaveMbContext ( CP_INSTANCE *cpi, UINT32 MBcol )
{
	PB_INSTANCE *pbi = &cpi->pb;

	// Save mbi to restore later
	memcpy ( &cpi->CopyMbi, &pbi->mbi, sizeof(MACROBLOCK_INFO) );

	// Save the frame dc context
	memcpy (  cpi->AboveCopyY, &pbi->fc.AboveY[MBcol*2], sizeof(BLOCK_CONTEXT)*2 );
	memcpy ( &cpi->AboveCopyU, &pbi->fc.AboveU[MBcol],   sizeof(BLOCK_CONTEXT) );
	memcpy ( &cpi->AboveCopyV, &pbi->fc.AboveV[MBcol],   sizeof(BLOCK_CONTEXT) );

	memcpy (  cpi->LeftYCopy,  pbi->fc.LeftY, sizeof(BLOCK_CONTEXT)*2 );
	memcpy ( &cpi->LeftUCopy, &pbi->fc.LeftU, sizeof(BLOCK_CONTEXT) );
	memcpy ( &cpi->LeftVCopy, &pbi->fc.LeftV, sizeof(BLOCK_CONTEXT) );

	memcpy ( cpi->LastDcYCopy, pbi->fc.LastDcY, sizeof(Q_LIST_ENTRY)*3 );
	memcpy ( cpi->LastDcUCopy, pbi->fc.LastDcU, sizeof(Q_LIST_ENTRY)*3 );
	memcpy ( cpi->LastDcVCopy, pbi->fc.LastDcV, sizeof(Q_LIST_ENTRY)*3 );
}

/****************************************************************************
 * 
 *  ROUTINE       : RdRestoresMbContext
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *                  UINT32 MBcol     : Macroblock column number.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Restores the contexts for a macro-block.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void RdRestoresMbContext( CP_INSTANCE *cpi, UINT32 MBcol )
{
	PB_INSTANCE *pbi = &cpi->pb;

    // Restore the dc context data structures to how they were before the call to this function.
	memcpy( &pbi->fc.AboveY[MBcol*2], cpi->AboveCopyY, sizeof(BLOCK_CONTEXT)*2 );
	memcpy( &pbi->fc.AboveU[MBcol],  &cpi->AboveCopyU, sizeof(BLOCK_CONTEXT) );
	memcpy( &pbi->fc.AboveV[MBcol],  &cpi->AboveCopyV, sizeof(BLOCK_CONTEXT) );

	memcpy(  pbi->fc.LeftY,  cpi->LeftYCopy, sizeof(BLOCK_CONTEXT)*2 );
	memcpy( &pbi->fc.LeftU, &cpi->LeftUCopy, sizeof(BLOCK_CONTEXT) );
	memcpy( &pbi->fc.LeftV, &cpi->LeftVCopy, sizeof(BLOCK_CONTEXT) );

	memcpy( pbi->fc.LastDcY, cpi->LastDcYCopy, sizeof(Q_LIST_ENTRY)*3 );
	memcpy( pbi->fc.LastDcU, cpi->LastDcUCopy, sizeof(Q_LIST_ENTRY)*3 );
	memcpy( pbi->fc.LastDcV, cpi->LastDcVCopy, sizeof(Q_LIST_ENTRY)*3 );

	// Restore mbi values to their Y defaults for use in the rest of pickmodes
	memcpy( &pbi->mbi, &cpi->CopyMbi, sizeof(MACROBLOCK_INFO) );
}



/****************************************************************************
 * 
 *  ROUTINE       : EncodeBlock_RD
 *
 *  INPUTS        : CP_INSTANCE *cpi        : Pointer to encoder instance.
 *	                UINT32 MBrow            : Macro-block row number.
 *	                UINT32 MBcol            : Macro-block column number.
 *	                BLOCK_POSITION bp       : Position of block in MB (0-5).
 *	                BOOL SaveBlockDcContext : Flag whether to save block context.
 *
 *  OUTPUTS       : UINT32 *Rate            : Approximation of number of bits required to code block.
 *                  UINT32 *Dist            : Distortion of the encoded block.   
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Encodes a block in rate-distortion mode.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void EncodeBlock_RD
( 
	CP_INSTANCE *cpi,
	UINT32 MBrow,
	UINT32 MBcol,
	BLOCK_POSITION bp,
    UINT32 *Rate,
    UINT32 *Dist,
	BOOL SaveBlockDcContext
)
{
	PB_INSTANCE *pbi = &cpi->pb;

    UINT32 T_Error1;

    // build a block predictor & subtract predictor from source we are trying to compress
	PredictBlock ( cpi, bp );
	
    // forward DCT
    fdct_short(cpi->DCTDataBuffer, cpi->DCT_codes); 
    
    // predict our dc values from the surrounding guys
	PredictDCE (cpi, bp);

	// quantize the coefficients
	VP6_quantize ( pbi->quantizer, cpi->DCT_codes, pbi->mbi.blockDxInfo[bp].coeffsPtr, (UINT8)bp );   

	// convert coefficients to tokens
	//pbi->FragCoefEOB = (UINT8)
    TokenizeFrag_RD ( cpi, pbi->mbi.blockDxInfo[bp].coeffsPtr, pbi->mbi.blockDxInfo[bp].Plane, Rate );

    
    GetQuantizedCoeffsMSE_RD(cpi->DCT_codes, 
                                pbi->mbi.blockDxInfo[bp].coeffsPtr, 
                                pbi->mbi.blockDxInfo[bp].dequantPtr,
                                &T_Error1);
    *Dist += T_Error1;
    
    // predict our dc values from the surrounding guys
	VP6_PredictDC ( pbi, bp );

	// update the context info for the next block
	VP6_UpdateContextA ( pbi, pbi->mbi.blockDxInfo[bp].Above, bp );
	VP6_UpdateContext  ( pbi, pbi->mbi.blockDxInfo[bp].Left,  bp );

    
	// If requested then save the DC context for this block in a data structure indexed by mode and block position.
	// The saved values are used to update the DC context once the best coding method has been decided.
	if ( SaveBlockDcContext )
	{
		memcpy ( &cpi->MbDcContexts[pbi->mbi.Mode][bp].Above, pbi->mbi.blockDxInfo[bp].Above, sizeof(BLOCK_CONTEXT) );
		cpi->MbDcContexts[pbi->mbi.Mode][bp].AbovePtr = pbi->mbi.blockDxInfo[bp].Above;

		memcpy ( &cpi->MbDcContexts[pbi->mbi.Mode][bp].Left, pbi->mbi.blockDxInfo[bp].Left, sizeof(BLOCK_CONTEXT) );
		cpi->MbDcContexts[pbi->mbi.Mode][bp].LeftPtr = pbi->mbi.blockDxInfo[bp].Left;

		memcpy ( &cpi->MbDcContexts[pbi->mbi.Mode][bp].LastDc, pbi->mbi.blockDxInfo[bp].LastDc, sizeof(Q_LIST_ENTRY) );
		cpi->MbDcContexts[pbi->mbi.Mode][bp].LastDcPtr = pbi->mbi.blockDxInfo[bp].LastDc;
	}
    
}
/****************************************************************************
 * 
 *  ROUTINE       : EncodeMacroBlock_RD
 *
 *  INPUTS        : CP_INSTANCE *cpi     : Pointer to encoder instance.
 *	                UINT32 *FragsToCheck : Pointer to list of blocks in the MB.
 *                  UINT32 MBrow         : Macro-block row number.
 *	                UINT32 MBcol         : Macro-block column number.
 *
 *  OUTPUTS       : UINT32 *Rate         : Pointer to Rate value (in bits).
 *                  UINT32 *Dist         : Pointer to Distortion value.
 *
 *  RETURNS       : None.
 *
 *  FUNCTION      : Encodes the macro-block to the point where an estimate
 *                  of the cost of coding and reconstruction error may be
 *                  made.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void EncodeMacroBlock_RD
(
	CP_INSTANCE *cpi,
	UINT32 *FragsToCheck,
	UINT32 MBrow,
	UINT32 MBcol,
    UINT32 *Rate,
    UINT32 *Dist
)
{
	UINT32 Block;
	PB_INSTANCE *pbi = &cpi->pb;

	// Save the Macro Block and DC context
	RdSaveMbContext ( cpi, MBcol );

	// Clear down MB rate and distortion accumulators 
	*Rate = 0;
	*Dist = 0;

	// Set up the Mb mode and Mv values
    for ( Block=0; Block<6; Block++ )
    {
        pbi->mbi.Mv[Block].x = pbi->FragInfo[FragsToCheck[Block]].MVectorX;
        pbi->mbi.Mv[Block].y = pbi->FragInfo[FragsToCheck[Block]].MVectorY;
    } 

	pbi->mbi.blockDxInfo[0].Above = &pbi->fc.AboveY[MBcol*2];
	pbi->mbi.blockDxInfo[1].Above = &pbi->fc.AboveY[MBcol*2+1];
	pbi->mbi.blockDxInfo[2].Above = &pbi->fc.AboveY[MBcol*2];
	pbi->mbi.blockDxInfo[3].Above = &pbi->fc.AboveY[MBcol*2+1];
	pbi->mbi.blockDxInfo[4].Above = &pbi->fc.AboveU[MBcol];
	pbi->mbi.blockDxInfo[5].Above = &pbi->fc.AboveV[MBcol];

	EncodeBlock_RD ( cpi, MBrow, MBcol, 0, Rate, Dist, TRUE );

	EncodeBlock_RD ( cpi, MBrow, MBcol, 1, Rate, Dist, TRUE );

	EncodeBlock_RD ( cpi, MBrow, MBcol, 2, Rate, Dist, TRUE );
	
	EncodeBlock_RD ( cpi, MBrow, MBcol, 3, Rate, Dist, TRUE );

	EncodeBlock_RD ( cpi, MBrow, MBcol, 4, Rate, Dist, TRUE );

	EncodeBlock_RD ( cpi, MBrow, MBcol, 5, Rate, Dist, TRUE );

	// Restore the MB and dc context
	RdRestoresMbContext ( cpi, MBcol );
}

/****************************************************************************
 * 
 *  ROUTINE       : RdFunction
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *                  UINT32 Rate      : Rate value (in bits).
 *                  UINT32 Dist      : Distortion value.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: The computed rate-distortion value.
 *
 *  FUNCTION      : Evaluates a Rate-Distortion function for specified rate
 *                  and distortion.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
UINT32 RdFunction ( CP_INSTANCE *cpi, UINT32 Rate, UINT32 Dist )
{
	UINT32 RdValue;
	UINT32 A = RateMult[cpi->pb.quantizer->FrameQIndex];
	UINT32 B = RateDiv[cpi->pb.quantizer->FrameQIndex];

	// Apply a correction to the rate multiplier according to an estimate
    // of complexity derived from last frame MV useage.
	A = (A*RateMultCorrection[cpi->LastFrameNewMvUsage]) >> 7;
	if ( A < 1 )
		A = 1;
	
	RdValue = Dist + ((A * Rate) / B);
	
	return RdValue;
}

/****************************************************************************
 * 
 *  ROUTINE       : RdModeCost
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *	                UINT32 MBrow     : Macro-block row number.
 *	                UINT32 MBcol     : Macro-block column number.
 *                  UINT8 Mode       : Coding mode for MB.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Approximate cost of coding mode (in bits).
 *
 *  FUNCTION      : Estimates the cost (in bits) of coding a mode.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
UINT32 RdModeCost ( CP_INSTANCE *cpi, UINT32 MBrow, UINT32 MBcol, UINT8 Mode )
{
	return modeCost ( cpi, MBrow, MBcol, Mode );
}

/****************************************************************************
 * 
 *  ROUTINE       : SetFragMotionVectorAndMode
 *
 *  INPUTS        : PB_INSTANCE *pbi                : Pointer to decoder instance.
 *                  INT32 FragIndex                 : Block to set Mode & MV for.
 *                  MOTION_VECTOR *ThisMotionVector : MV for the block.
 *	                CODING_MODE mode                : Coding mode for the block.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Sets specified coding mode & motion vector for a block.
 *
 ****************************************************************************/
void SetFragMotionVectorAndMode
( 
    PB_INSTANCE *pbi,
    INT32 FragIndex,
    MOTION_VECTOR *ThisMotionVector,
	CODING_MODE mode
)
{
    // Note the coding mode and vector for the block
    pbi->FragInfo[FragIndex].FragCodingMode = mode;
    pbi->FragInfo[FragIndex].MVectorX       = ThisMotionVector->x;
    pbi->FragInfo[FragIndex].MVectorY       = ThisMotionVector->y;
}

/****************************************************************************
 * 
 *  ROUTINE       : PickIntra
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Total intra-error for the frame.
 *
 *  FUNCTION      : Selects INTRA coding mode for all macro-blocks in the
 *                  frame. This is a suitable way to code key-frames as
 *                  there is then no dependency on previously decoded data.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
UINT32 PickIntra ( CP_INSTANCE *cpi )
{
    UINT32  Temp;
	UINT32  i;
	UINT32	B;
    UINT32	MBrow;
    UINT32	MBcol;
    UINT32	UVRow;
    UINT32	UVColumn;
	INT32	FragIndex;
	UINT32  IntraError;
    UINT32	UVFragOffset;
	INT32   TopLeftIndex = 0;
	UINT32  TotIntraError = 0;
	UINT32  CountInterlaced = 0;
	PB_INSTANCE *pbi = &cpi->pb;
    //UINT32	BlockOffset[4] = { 0, 1, pbi->HFragments, pbi->HFragments+1 };
    UINT32	BlockOffset[4];

    BlockOffset[0] = 0;
    BlockOffset[1] = 1;
    BlockOffset[2] = pbi->HFragments;
    BlockOffset[3] = pbi->HFragments+1;

	for ( i=0; i<128; i++ )
		cpi->ErrorBins[i] = pbi->UnitFragments / 4;

	// Reset the mode+mv frame cost estimate (no modes or mvs for a key frame).
	cpi->ModeMvCostEstimate = 0;

	for ( MBrow=BORDER_MBS; MBrow<pbi->MBRows-BORDER_MBS; MBrow++ )
	{
		for ( MBcol=BORDER_MBS; MBcol<pbi->MBCols-BORDER_MBS; MBcol++ )
		{
			cpi->MBCodingMode = CODE_INTRA;
            
            pbi->mbi.blockDxInfo[0].Source = pbi->YDataOffset + 16*(MBrow-BORDER_MBS) *pbi->Configuration.VideoFrameWidth + 16*(MBcol-BORDER_MBS);
			pbi->mbi.blockDxInfo[0].thisRecon = pbi->ReconYDataOffset + 16*MBrow * pbi->Configuration.YStride + 16*MBcol;

			if ( pbi->Configuration.Interlaced /*&& GetMBFrameVertVar(cpi) > GetMBFieldVertVar(cpi)*/ )
			{
                // Code MB as two separate fields
				pbi->mbi.Interlaced = 1;
				pbi->MBInterlaced[MBOffset(MBrow,MBcol)] = 1;

				pbi->mbi.blockDxInfo[0].CurrentSourceStride =
                pbi->mbi.blockDxInfo[1].CurrentSourceStride =
                pbi->mbi.blockDxInfo[2].CurrentSourceStride =
                pbi->mbi.blockDxInfo[3].CurrentSourceStride = 2 * pbi->Configuration.VideoFrameWidth;

                pbi->mbi.blockDxInfo[1].Source = pbi->mbi.blockDxInfo[0].Source + 8;
                pbi->mbi.blockDxInfo[2].Source = pbi->mbi.blockDxInfo[0].Source + pbi->Configuration.VideoFrameWidth;
                pbi->mbi.blockDxInfo[3].Source = pbi->mbi.blockDxInfo[2].Source + 8;

				CountInterlaced++;
			}
			else
			{
                // Code MB as a single progressive-scan MB
				pbi->MBInterlaced[MBOffset(MBrow,MBcol)] = 0;
				pbi->mbi.Interlaced = 0;

				pbi->mbi.blockDxInfo[0].CurrentSourceStride =
                pbi->mbi.blockDxInfo[1].CurrentSourceStride =
                pbi->mbi.blockDxInfo[2].CurrentSourceStride =
                pbi->mbi.blockDxInfo[3].CurrentSourceStride = pbi->Configuration.VideoFrameWidth;

                pbi->mbi.blockDxInfo[1].Source = pbi->mbi.blockDxInfo[0].Source + 8;
                pbi->mbi.blockDxInfo[2].Source = pbi->mbi.blockDxInfo[0].Source + (pbi->Configuration.VideoFrameWidth << 3);
                pbi->mbi.blockDxInfo[3].Source = pbi->mbi.blockDxInfo[2].Source + 8;

			}

			for ( B=0; B<4; B++ )
			{
				FragIndex = TopLeftIndex + BlockOffset[B];
				pbi->FragInfo[FragIndex].FragCodingMode = cpi->MBCodingMode;
			}

			// Matching fragments in the U and V planes
			UVRow        = (FragIndex / (pbi->HFragments * 2));
			UVColumn     = (FragIndex % pbi->HFragments) / 2;
			UVFragOffset = (UVRow * (pbi->HFragments / 2)) + UVColumn;
        
			pbi->FragInfo[pbi->YPlaneFragments + UVFragOffset].FragCodingMode = cpi->MBCodingMode;
			pbi->FragInfo[pbi->YPlaneFragments + pbi->UVPlaneFragments + UVFragOffset].FragCodingMode  = cpi->MBCodingMode;

			// Keep a note of the total error score for the Y macro blocks for rate targeting purposes
			IntraError = GetMBIntraError( cpi );

        	Temp = (IntraError>>8);
        	if ( Temp < MIN_ERR )
        		Temp = MIN_ERR;
	        else if (  Temp > MAX_ERR )
		        Temp = MAX_ERR;
	        TotIntraError += Temp;

			TopLeftIndex += 2;
		}

		TopLeftIndex += pbi->HFragments;
	}
	
    pbi->probInterlaced = 256-(1+254*CountInterlaced/pbi->MacroBlocks);

    return TotIntraError;
}

/****************************************************************************
 * 
 *  ROUTINE       : SetMBMotionVectorsAndMode
 *
 *  INPUTS        : CP_INSTANCE *cpi            : Pointer to encoder instance.
 *                  INT32 *FragIndexes          : Pointer to list of blocks in the MB.
 *                  MOTION_VECTOR *MotionVector : MV for the MB.
 *	                CODING_MODE mode            : Coding mode for the MB.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Sets the coding mode for the macro-block and coding mode
 *                  and motion vector for each its 6 constituent blocks.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void SetMBMotionVectorsAndMode
( 
    CP_INSTANCE *cpi,     
    UINT32 *FragIndexes,
    UINT32 Mode,
    MOTION_VECTOR *MotionVector
)
{
	PB_INSTANCE *pbi = &cpi->pb;

	pbi->mbi.Mode = Mode;
    SetFragMotionVectorAndMode ( pbi, FragIndexes[0], MotionVector, Mode );
    SetFragMotionVectorAndMode ( pbi, FragIndexes[1], MotionVector, Mode );
    SetFragMotionVectorAndMode ( pbi, FragIndexes[2], MotionVector, Mode );
    SetFragMotionVectorAndMode ( pbi, FragIndexes[3], MotionVector, Mode );
    SetFragMotionVectorAndMode ( pbi, FragIndexes[4], MotionVector, Mode );
    SetFragMotionVectorAndMode ( pbi, FragIndexes[5], MotionVector, Mode );
}

/****************************************************************************
 * 
 *  ROUTINE       : PickBetterMBMode
 *
 *  INPUTS        : CP_INSTANCE *cpi           : Pointer to encoder instance.
 *	                UINT32 *FragsToCheck       : Pointer to list of 6 blocks in this MB.
 *	                CODING_MODE mode           : Coding mode to evaluate.
 *	                MOTION_VECTOR *ThisMVector : Pointer to MV associated with this mode.
 *	                UINT32 MBrow               : MB row.
 *	                UINT32 MBcol               : MB column.
 *	                UINT8 *Frame               : Pointer to MB in previous frame reconstruction.
 *	                CODING_MODE *BestMode      : Pointer to best mode found so far.
 *	                UINT32 *Error              : Best error found so far.
 *	                MOTION_VECTOR *mv          : Pointer to MV for best mode found so far.
 *	                UINT32 *FourError          : Pointer to errors for 4 Y-blocks in MB.
 *	                UINT32 *BestRate           : Pointer to best rate found so far.
 *	                UINT32 *BestDist           : Pointer to best distortion found so far.
 *	                UINT32 *BestRd             : Pointer to best RD-value found so far.
 *
 *  OUTPUTS       : CODING_MODE *BestMode      : Pointer to best mode found so far.
 *                  UINT32 *Error              : Best error found so far.
 *                  MOTION_VECTOR *mv          : Pointer to MV for best mode found so far.
 *                  UINT32 *FourError          : Pointer to errors for 4 Y-blocks in MB.
 *                  UINT32 *BestRate           : Pointer to best rate found so far.
 *                  UINT32 *BestDist           : Pointer to best distortion found so far.
 *                  UINT32 *BestRd             : Pointer to best RD-value found so far.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Evaluates the specified coding mode and if better than
 *                  the best mode found so far, updates the relevant variables.
 *
 *  SPECIAL NOTES : If rate-distortion mode is enabled then an estimate of
 *                  the rate & distortion is found by a dummy coding of the
 *                  MB that does not output to the bitstream.
 *
 ****************************************************************************/
void PickBetterMBMode
(
	CP_INSTANCE *cpi,
	UINT32 * FragsToCheck,
	CODING_MODE mode,
	MOTION_VECTOR *ThisMVector, 
	UINT32 MBrow,
	UINT32 MBcol,
	UINT8 *Frame,
	CODING_MODE *BestMode,
	UINT32 *Error,
	MOTION_VECTOR *mv,
	UINT32 *FourError,
	UINT32 *BestRate,
	UINT32 *BestDist,
	UINT32 *BestRd
)
{
	UINT32 ThisError;
	UINT32 EstModeCost;

	// Get an estimate of the mode cost
	if ( cpi->RdOpt ) 
		EstModeCost = RdModeCost ( cpi, MBrow, MBcol, mode );
	else
		EstModeCost = modeCost ( cpi, MBrow, MBcol, mode );
	ThisError = EstModeCost * EPB;
 
	// Trap for cases where mode cost alone rules this mode out
	if( !cpi->RdOpt && (ThisError > *Error))
		return;

	ThisError += GetMBInterError ( cpi, cpi->yuv1ptr, Frame, ThisMVector, FourError );

	// Are we using RD
	if ( cpi->RdOpt )
	{
		UINT32 Rate;
		UINT32 Dist;
		UINT32 RdValue;

		// RD Code TBD
		SetMBMotionVectorsAndMode ( cpi, FragsToCheck, mode, ThisMVector );
		EncodeMacroBlock_RD ( cpi, FragsToCheck, MBrow, MBcol, &Rate, &Dist );
		Rate += EstModeCost;

		// Calculate Best RD value
		RdValue = RdFunction ( cpi, Rate, Dist );

		if ( (RdValue < *BestRd) || ( (ThisError<(*Error >> 1)) && (Dist<(*BestDist >> 1)) ) )
		{
			*BestRd   = RdValue;
			*BestRate = Rate;
			*BestDist = Dist;
			*BestMode = mode;
			*Error    = ThisError;
			mv->x     = ThisMVector->x;
			mv->y     = ThisMVector->y;
		}
	}
	else
	{
		if ( ThisError < *Error )
		{
			*BestMode = mode;
			*Error    = ThisError;
			mv->x     = ThisMVector->x;
			mv->y     = ThisMVector->y;
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : PickBetterMBModeandMV
 *
 *  INPUTS        : CP_INSTANCE *cpi           : Pointer to encoder instance.
 *	                UINT32 *FragsToCheck       : Pointer to list of 6 blocks in this MB.
 *	                CODING_MODE mode           : Coding mode to evaluate.
 *	                UINT8 *Frame               : Pointer to MB in previous frame reconstruction.
 *	                UINT32 MBrow               : MB row.
 *	                UINT32 MBcol               : MB column.
 *	                CODING_MODE *BestMode      : Pointer to best mode found so far.
 *	                UINT32 *Error              : Best error found so far.
 *	                MOTION_VECTOR *BestMV      : Pointer to MV for best mode found so far.
 *	                UINT32 *FourErrors         : Pointer to errors for 4 Y-blocks in MB.
 *	                UINT32 *BestRate           : Pointer to best rate found so far.
 *	                UINT32 *BestDist           : Pointer to best distortion found so far.
 *	                UINT32 *BestRd             : Pointer to best RD-value found so far.
 *
 *  OUTPUTS       : CODING_MODE *BestMode      : Pointer to best mode found so far.
 *                  UINT32 *Error              : Best error found so far.
 *                  MOTION_VECTOR *BestMV      : Pointer to MV for best mode found so far.
 *                  UINT32 *FourErrors         : Pointer to errors for 4 Y-blocks in MB.
 *                  UINT32 *BestRate           : Pointer to best rate found so far.
 *                  UINT32 *BestDist           : Pointer to best distortion found so far.
 *                  UINT32 *BestRd             : Pointer to best RD-value found so far.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Evaluates the specified coding mode and if better than
 *                  the best mode found so far, updates the relevant variables.
 *                  As part of the evaluation of the mode a motion vector
 *                  search is carried out.
 *
 *  SPECIAL NOTES : If rate-distortion mode is enabled then an estimate of
 *                  the rate & distortion is found by a dummy coding of the
 *                  MB that does not output to the bitstream.
 *
 ****************************************************************************/
void PickBetterMBModeAndMV
(
	CP_INSTANCE *cpi,
	UINT32 *FragsToCheck,
	CODING_MODE mode,
	UINT8 *Frame,
	UINT32 MBrow,
	UINT32 MBcol,
	CODING_MODE *BestMode,
	UINT32 *Error,
	MOTION_VECTOR *BestMV,
	BOOL FullSearchEnabled,
	UINT32 *FourErrors,
	UINT32 *BestRate,
	UINT32 *BestDist,
	UINT32 *BestRd
)
{
	UINT32 ThisError;
	UINT32 EstMvCost;	
	UINT32 EstModeCost;
	MOTION_VECTOR ThisMV;
	MOTION_VECTOR InterMVectEx;
	MOTION_VECTOR DifferentialVector;

	// Get an estimate of the mode cost
	if ( cpi->RdOpt )
		EstModeCost = RdModeCost ( cpi, MBrow, MBcol, mode );
	else
		EstModeCost = modeCost ( cpi, MBrow, MBcol, mode );

	if ( !cpi->RdOpt && ((EstModeCost * EPB) > *Error) )
		return;

	// If the best error is above the required threshold search for a new inter MV
	// Use a mix of heirachical and exhaustive searches for quick mode.
	ThisError = GetMBMVInterError ( cpi, mode, Frame, &ThisMV, FourErrors );
	
	// If we still do not have a good match try an exhaustive MBMV search
	if ( FullSearchEnabled &&
		 (ThisError > cpi->ExhaustiveSearchThresh) && 
		 (*Error > cpi->ExhaustiveSearchThresh) ) 
	{
		UINT32 NewError;
		NewError = GetMBMVExhaustiveSearch ( cpi, mode, Frame, &InterMVectEx, FourErrors );
		
		// Is the Variance measure for the EX search better... If so then use it.
		if ( NewError < ThisError )
		{
			ThisError = NewError;   
			ThisMV.x = InterMVectEx.x;
			ThisMV.y = InterMVectEx.y;
		}
	}
	
	cpi->bc.BitCounter = 0;

	// Convert the motion vector to a differential vector relative to "nearest"
	DifferentialVector.x = ThisMV.x;
	DifferentialVector.y = ThisMV.y;
	if ( mode == CODE_INTER_PLUS_MV )
	{
		if ( cpi->pb.mbi.NearestMvIndex < MAX_NEAREST_ADJ_INDEX )
		{
			DifferentialVector.x -= cpi->pb.mbi.NearestInterMVect.x;
			DifferentialVector.y -= cpi->pb.mbi.NearestInterMVect.y;
		}
	}
	else	// Golden frame
	{
		if ( cpi->pb.mbi.NearestGMvIndex < MAX_NEAREST_ADJ_INDEX )
		{
			DifferentialVector.x -= cpi->pb.mbi.NearestGoldMVect.x;
			DifferentialVector.y -= cpi->pb.mbi.NearestGoldMVect.y;
		}
		else
		{
			DifferentialVector.x = ThisMV.x;
			DifferentialVector.y = ThisMV.y;
		}
	}


	// The error MV error adjustment coprises a MVEPB which is a constant set according 
	// to the number of new motion vectors in the last frame and an estimate of the cost 
	// in bits(*64) of the vector.
	EstMvCost = cpi->EstMvCostPtrX[DifferentialVector.x] + cpi->EstMvCostPtrY[DifferentialVector.y];
	ThisError += (cpi->MVErrorPerBit + (ThisError >> 13)) * EstMvCost;
	ThisError += EstModeCost * EPB;

	// Are we using RD
	if ( cpi->RdOpt )
	{
		UINT32 Rate;
		UINT32 Dist;
		UINT32 RdValue;

		// RD Code TBD
		SetMBMotionVectorsAndMode ( cpi, FragsToCheck, mode, &ThisMV );
		EncodeMacroBlock_RD ( cpi, FragsToCheck, MBrow, MBcol, &Rate, &Dist );
		Rate += EstModeCost;
		Rate += EstMvCost;	
		Rate -= RdMvCostCorrection[cpi->LastFrameNewMvUsage];   // Apply mv re-use estimate correction

		// Calculate Best RD value
		RdValue = RdFunction ( cpi, Rate, Dist );

		if ( (RdValue < *BestRd) || ( (ThisError<(*Error >> 1)) && (Dist<(*BestDist >> 1)) ) )
		{
			*BestRd   = RdValue;
			*BestRate = Rate;
			*BestDist = Dist;
			*BestMode = mode;
			*Error    = ThisError;
			BestMV->x = ThisMV.x;
			BestMV->y = ThisMV.y;
		}
	}
	else
	{
		// Is the improvement, if any, good enough to justify a new MV
		if ( ThisError < *Error )
		{
			*BestMode = mode;
			*Error    = ThisError;
			BestMV->x = ThisMV.x;
			BestMV->y = ThisMV.y;
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : PickBetterBMode
 *
 *  INPUTS        : CP_INSTANCE *cpi           : Pointer to encoder instance.
 *	                UINT8 *Frame               : Pointer to block in previous frame reconstruction (NOT USED).
 *	                UINT32 MBrow               : MB row of parent MB.
 *	                UINT32 MBcol               : MB column of parent MB.
 *	                UINT32 Block               : Block number in its parant MB (0-3).
 *	                CODING_MODE ThisMode       : Coding mode to evaluate.
 *	                MOTION_VECTOR *ThisMv      : Pointer to MV for best mode found so far.
 *	                UINT32 *ThisError          : Best error found so far.
 *	                CODING_MODE *BestMode      : Pointer to best mode found so far.
 *	                UINT32 *BestError          : Pointer to best error found so far.
 *	                MOTION_VECTOR *BestMv      : Pointer to MV for best mode found so far.
 *	                UINT32 *BestRdValue        : Pointer to best RD-value found so far.
 *
 *  OUTPUTS       : CODING_MODE *BestMode      : Pointer to best mode found so far.
 *	                UINT32 *BestError          : Pointer to best error found so far.
 *	                MOTION_VECTOR *BestMv      : Pointer to MV for best mode found so far.
 *	                UINT32 *BestRdValue        : Pointer to best RD-value found so far.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Evaluates the specified coding mode for coding the block
 *                  and if better than the best mode found so far, updates 
 *                  the relevant variables.
 *
 *  SPECIAL NOTES : If rate-distortion mode is enabled then an estimate of
 *                  the rate & distortion is found by a dummy coding of the
 *                  block that does not output to the bitstream.
 *
 ****************************************************************************/
void PickBetterBMode
(
	CP_INSTANCE *cpi,
	UINT8 *Frame,
	UINT32 MBrow,
	UINT32 MBcol,
	UINT32 Block,
	CODING_MODE ThisMode,
	MOTION_VECTOR *ThisMv,
	UINT32 ThisError,
	CODING_MODE *BestMode,
	UINT32 *BestError,
	MOTION_VECTOR *BestMv,
	UINT32 *BestRdValue
)
{
	UINT32 EstModeCost;

	EstModeCost = blockModeCost ( cpi, MBrow, MBcol, ThisMode );
	ThisError += EstModeCost * EPB;

	// Are we using RD or modified prediction error
	if ( cpi->RdOpt > 1 )
	{
		UINT32 Rate    = 0;
		UINT32 Dist    = 0;
		UINT32 RdValue = 0;
		PB_INSTANCE *pbi = &cpi->pb;
		
		// Save the Macro Block and DC context
		RdSaveMbContext ( cpi, MBcol );

		// Set up relevant parts of the mbi structure
		pbi->mbi.Mode        = ThisMode;
		pbi->mbi.Mv[Block].x = ThisMv->x;
		pbi->mbi.Mv[Block].y = ThisMv->y;

		switch ( Block )
		{
		case 0:
			pbi->mbi.blockDxInfo[Block].Above = &pbi->fc.AboveY[1+MBcol*2];
			pbi->mbi.blockDxInfo[Block].Left  = &pbi->fc.LeftY[0];
			break;
		case 1:
			pbi->mbi.blockDxInfo[Block].Above = &pbi->fc.AboveY[1+MBcol*2+1];
			pbi->mbi.blockDxInfo[Block].Left  = &pbi->fc.LeftY[0];
			break;
		case 2:
			pbi->mbi.blockDxInfo[Block].Above = &pbi->fc.AboveY[1+MBcol*2];
			pbi->mbi.blockDxInfo[Block].Left  = &pbi->fc.LeftY[1];
			break;
		case 3:
			pbi->mbi.blockDxInfo[Block].Above = &pbi->fc.AboveY[1+MBcol*2+1];
			pbi->mbi.blockDxInfo[Block].Left  = &pbi->fc.LeftY[1];
			break;
		default:
//sjlhack -- what the heck is this??????  If it is an error then return one... don't fake it out!!!!!!!!!!
			// Error - Block should always be in range 0-3
			pbi->mbi.blockDxInfo[0].Above = &pbi->fc.AboveY[1+MBcol*2];
			pbi->mbi.blockDxInfo[0].Left  = &pbi->fc.LeftY[0];
			break;
		}


		// Encode the block to get a rate and a distortion value
		EncodeBlock_RD ( cpi, MBrow, MBcol, Block, &Rate, &Dist, FALSE );

		// Restore the MB and dc context
		RdRestoresMbContext ( cpi, MBcol );

		// Add in the mode cost to the rate.
		Rate += EstModeCost;

		// Calculate Best RD value
		RdValue = RdFunction ( cpi, Rate, Dist );

        // Does this mode give an improvement in RD
		if ( (RdValue < *BestRdValue) || (ThisError < (*BestError >> 1)) )
		{
			*BestMode    = ThisMode;
			*BestError = ThisError;
			*BestError   = Dist;
			*BestRdValue = RdValue;
			BestMv->x    = ThisMv->x;
			BestMv->y    = ThisMv->y;
		}
	}
	else
	{
		// Non RD case.
		if ( ThisError < *BestError )
		{
			*BestMode  = ThisMode;
			*BestError = ThisError;
			BestMv->x  = ThisMv->x;
			BestMv->y  = ThisMv->y;
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : PickBetterBModeandMV
 *
 *  INPUTS        : CP_INSTANCE *cpi           : Pointer to encoder instance.
 *	                UINT8 *Frame               : Pointer to block in previous frame reconstruction.
 *	                UINT32 MBrow               : MB row of parent MB.
 *	                UINT32 MBcol               : MB column of parent MB.
 *	                UINT32 Block               : Block number in its parant MB (0-3).
 *	                CODING_MODE ThisMode       : Coding mode to evaluate.
 *	                CODING_MODE *BestMode      : Coding mode to evaluate.
 *	                UINT32 *BestError          : Pointer to best error found so far.
 *	                MOTION_VECTOR *BestMv      : Pointer to MV for best mode found so far.
 *	                UINT32 *BestRdValue        : Pointer to best RD-value found so far.
 *                  BOOL FullSearchEnabled     : Flag as to whether exhaustive MV search is enabled (NOT USED).
 *
 *  OUTPUTS       : CODING_MODE *BestMode      : Coding mode to evaluate.
 *	                UINT32 *BestError          : Pointer to best error found so far.
 *	                MOTION_VECTOR *BestMv      : Pointer to MV for best mode found so far.
 *	                UINT32 *BestRdValue        : Pointer to best RD-value found so far.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Evaluates the specified coding mode for coding the block
 *                  and if better than the best mode found so far, updates 
 *                  the relevant variables. As part of the evaluation of the
 *                  mode a motion vector search is carried out.
 *
 *  SPECIAL NOTES : If rate-distortion mode is enabled then an estimate of
 *                  the rate & distortion is found by a dummy coding of the
 *                  block that does not output to the bitstream.
 *
 ****************************************************************************/
void PickBetterBModeAndMV
(
	CP_INSTANCE *cpi,
	UINT8 *Frame,
	UINT32 MBrow,
	UINT32 MBcol,
	UINT32 Block,
	CODING_MODE ThisMode,
	CODING_MODE *BestMode,
	UINT32 *BestError,
	MOTION_VECTOR *BestMV,
	UINT32 *BestRdValue,
	BOOL FullSearchEnabled
)
{
	UINT32 ThisError;
	UINT32 EstMvCost;
	UINT32 EstModeCost;
	MOTION_VECTOR ThisMV;
	MOTION_VECTOR DifferentialVector;

	PB_INSTANCE *pbi = &cpi->pb;

	// Weight the mode according to last mode
	EstModeCost = blockModeCost ( cpi, MBrow, MBcol, ThisMode );
	ThisError = EstModeCost * EPB;
	
    if ( !cpi->RdOpt && (ThisError > *BestError) )
		return;

    // If the best error is above the required threshold search for a new inter MV
	if ( *BestError > cpi->BlockExhaustiveSearchThresh )
	{
		ThisError += GetBMVExhaustiveSearch( cpi, Frame, &ThisMV, Block);
    }
    else
    {
		ThisError += GetBMVSearch( cpi, Frame, &ThisMV, Block );
    }
	//else

	DifferentialVector.x = ThisMV.x;
	DifferentialVector.y = ThisMV.y;
	if ( pbi->mbi.NearestMvIndex < MAX_NEAREST_ADJ_INDEX )
	{
		DifferentialVector.x -= pbi->mbi.NearestInterMVect.x;
		DifferentialVector.y -= pbi->mbi.NearestInterMVect.y;
	}

	EstMvCost  = cpi->EstMvCostPtrX[DifferentialVector.x] + cpi->EstMvCostPtrY[DifferentialVector.y];
	ThisError += (cpi->MVErrorPerBit + (ThisError >> 13)) * EstMvCost;

	// Are we using RD or modified prediction error
	if ( cpi->RdOpt > 1 )
	{
		UINT32 Rate    = 0;
		UINT32 Dist    = 0;
		UINT32 RdValue = 0;
		
		// Save the Macro Block and DC context
		RdSaveMbContext ( cpi, MBcol );

		// Set up relevant parts of the mbi structure
		pbi->mbi.Mode        = ThisMode;
		pbi->mbi.Mv[Block].x = ThisMV.x;
		pbi->mbi.Mv[Block].y = ThisMV.y;

		switch ( Block )
		{
		case 0:
			pbi->mbi.blockDxInfo[Block].Above = &pbi->fc.AboveY[1+MBcol*2];
			pbi->mbi.blockDxInfo[Block].Left  = &pbi->fc.LeftY[0];
			break;
		case 1:
			pbi->mbi.blockDxInfo[Block].Above = &pbi->fc.AboveY[1+MBcol*2+1];
			pbi->mbi.blockDxInfo[Block].Left  = &pbi->fc.LeftY[0];
			break;
		case 2:
			pbi->mbi.blockDxInfo[Block].Above = &pbi->fc.AboveY[1+MBcol*2];
			pbi->mbi.blockDxInfo[Block].Left  = &pbi->fc.LeftY[1];
			break;
		case 3:
			pbi->mbi.blockDxInfo[Block].Above = &pbi->fc.AboveY[1+MBcol*2+1];
			pbi->mbi.blockDxInfo[Block].Left  = &pbi->fc.LeftY[1];
			break;
		}

		// Encode the block to get a rate and a distortion value
		EncodeBlock_RD ( cpi, MBrow, MBcol, Block, &Rate, &Dist, FALSE );

		// Restore the MB and dc context
		RdRestoresMbContext ( cpi, MBcol );

		// Add in the mode and mv costs to the rate.
		Rate += EstModeCost;
		Rate += EstMvCost;

		// Calculate Best RD value
		RdValue = RdFunction ( cpi, Rate, Dist );

        // Does this mode give an improvement in RD
		if ( (RdValue < *BestRdValue) || (ThisError < (*BestError >> 1)) )
		{
   			*BestError = ThisError;
			*BestMode    = ThisMode;
			*BestError   = Dist;
			*BestRdValue = RdValue;
			BestMV->x    = ThisMV.x;
			BestMV->y    = ThisMV.y;
		}
	}
	else
	{
		// Non RD case.
		if ( ThisError < *BestError )
		{
			*BestMode  = ThisMode;
			*BestError = ThisError;
			BestMV->x  = ThisMV.x;
			BestMV->y  = ThisMV.y;
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : PickBlockMode
 *
 *  INPUTS        : CP_INSTANCE *cpi           : Pointer to encoder instance.
 *	                UINT32 MBrow               : MB row of parent MB.
 *	                UINT32 MBcol               : MB column of parent MB.
 *	                UINT32 Block               : Block number in its parant MB (0-3).
 *	                CODING_MODE *BestMode      : Coding mode to evaluate.
 *	                MOTION_VECTOR *BestMVect   : Pointer to MV for best mode found so far.
 *	                UINT32 *BestError          : Pointer to best error found so far.
 *
 *  OUTPUTS       : CODING_MODE *BestMode      : Coding mode to evaluate.
 *	                MOTION_VECTOR *BestMVect   : Pointer to MV for best mode found so far.
 *	                UINT32 *BestError          : Pointer to best error found so far.
 *
 *  RETURNS       : void 
 *
 *  FUNCTION      : Picks the best coding mode for a block.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void PickBlockMode
( 
	CP_INSTANCE	  *cpi, 
	UINT32        MBrow,
	UINT32        MBcol,
	UINT32        Block,
	CODING_MODE	  *BestMode,
	MOTION_VECTOR *BestMVect,
	UINT32        *BestError
)
{
	UINT32		  BestSoFarError   = HUGE_ERROR;
	CODING_MODE	  BestSoFarMode    = CODE_INTER_NO_MV;
	UINT32        BestSoFarRdValue = HUGE_ERROR;
   	MOTION_VECTOR BestSoFarMVect   = { 0, 0 };       
    MOTION_VECTOR ZeroMVect        = { 0, 0 };  
	PB_INSTANCE   *pbi = &cpi->pb;
	
	// To start with I have chosen to pick the best mode and mv for the block based upon prediction error even when using RD
	// and only do the rate and distortion stuff for the chosen best mode and MV.
	PickBetterBMode ( cpi, pbi->LastFrameRecon, MBrow, MBcol, Block, CODE_INTER_NO_MV, &ZeroMVect, cpi->ZeroError[Block], &BestSoFarMode, &BestSoFarError, &BestSoFarMVect, &BestSoFarRdValue );

	if ( pbi->mbi.NearestInterMVect.x || pbi->mbi.NearestInterMVect.y )
		PickBetterBMode ( cpi, pbi->LastFrameRecon, MBrow, MBcol, Block, CODE_INTER_NEAREST_MV, &pbi->mbi.NearestInterMVect, cpi->NearestError[Block], &BestSoFarMode, &BestSoFarError, &BestSoFarMVect, &BestSoFarRdValue );

	if ( pbi->mbi.NearInterMVect.x || pbi->mbi.NearInterMVect.y )
		PickBetterBMode ( cpi, pbi->LastFrameRecon, MBrow, MBcol, Block, CODE_INTER_NEAR_MV, &pbi->mbi.NearInterMVect, cpi->NearError[Block], &BestSoFarMode, &BestSoFarError, &BestSoFarMVect, &BestSoFarRdValue  );

	if ( (cpi->RdOpt > 1) || (BestSoFarError > cpi->MinErrorForBlockMVSearch) ) 
		PickBetterBModeAndMV ( cpi, pbi->LastFrameRecon, MBrow, MBcol, Block, CODE_INTER_PLUS_MV,&BestSoFarMode,&BestSoFarError,&BestSoFarMVect, &BestSoFarRdValue, TRUE );

	*BestMode    = BestSoFarMode;
	*BestError   = BestSoFarError;
	BestMVect->x = BestSoFarMVect.x;
	BestMVect->y = BestSoFarMVect.y;
}

/****************************************************************************
 * 
 *  ROUTINE       : PickMacroBlockMode
 *
 *  INPUTS        : CP_INSTANCE	*cpi   : Pointer to encoder instance.
 *	                UINT32 MBrow       : MB row number.
 *	                UINT32 MBcol       : MB column number.
 *	                
 *  OUTPUTS       : UINT32 *InterError : Pointer to best inter-mode error.
 *	                UINT32 *IntraError : Pointer to intra-mode error.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Picks the best coding mode for a macro-block.
 *                  
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void PickMacroBlockMode
( 
	CP_INSTANCE	  *cpi, 
	UINT32		  MBrow, 
	UINT32		  MBcol,
	UINT32        *InterError,
	UINT32        *IntraError
)
{
	UINT32 i;
	UINT32 Temp;
	UINT32 TempError[4];
	UINT32 BestRate;    // The "rate" of the current best mode choice (when RD enabled else unused)
	UINT32 BestDist;    // The "distortion" of the current best mode choice (when RD enabled else unused)
	UINT32 BestRd;	    // The best RD compromise so far
	int type, type2;
	UINT32 EstModeCost;
	UINT32 ThisError;
	UINT32 ThisIntraError;
	UINT32 FragsToCheck[6];
    MOTION_VECTOR FourMVect[6];
	CODING_MODE   FourMode[6];
	MOTION_VECTOR DifferentialVector;

    MOTION_VECTOR MVect     = { 0, 0 };
    MOTION_VECTOR ZeroMVect = { 0, 0 };
    UINT32		  BestError = HUGE_ERROR;
	CODING_MODE	  BestMode = CODE_INTRA;
	PB_INSTANCE   *pbi = &cpi->pb;
 
	UINT32 YFragIndex   = (MBrow-BORDER_MBS) * pbi->HFragments * 2 + (MBcol-BORDER_MBS)*2;
	UINT32 UVFragOffset = (MBrow-BORDER_MBS) * pbi->HFragments / 2 + (MBcol-BORDER_MBS);
	UINT32 UFragIndex   = pbi->YPlaneFragments + UVFragOffset;
	UINT32 VFragIndex   = pbi->YPlaneFragments + pbi->UVPlaneFragments + UVFragOffset;

	// Intra and inter errors for this mb ignoring mode cost corrections etc.
	UINT32 ThisMbIntraErr;
	UINT32 ThisMbInterErr;

//note: should be able to move FragsToCheck into the blockDxInfo struct
//then in the MB loop, we should be able to inc the values instead of doing these multiplies
//it may not affect the pc performance, but it may help other processors
	FragsToCheck[0] = YFragIndex;
	FragsToCheck[1] = YFragIndex+1;
	FragsToCheck[2] = YFragIndex+pbi->HFragments;
	FragsToCheck[3] = YFragIndex+pbi->HFragments+1;
	FragsToCheck[4] = UFragIndex;
	FragsToCheck[5] = VFragIndex;

	// Root offsets for this MB
    pbi->mbi.blockDxInfo[0].Source = pbi->YDataOffset + 16*(MBrow-BORDER_MBS) *pbi->Configuration.VideoFrameWidth + 16*(MBcol-BORDER_MBS);
	pbi->mbi.blockDxInfo[0].thisRecon = pbi->ReconYDataOffset + 16*MBrow * pbi->Configuration.YStride + 16*MBcol;

    // AWG Add function here to compute variance for each block in MB
    // in progressive & interlaced mode. Use the resulting values to 
    // determine which coding pattern to use from (initially):
    // (P,P,P,P), (P,I,P,I), (I,P,I,P), (I,I,I,I)
    // Selected pattern encoded instead of interlaced flag.

	// Values that depend on whether or not we are coding an interlaced block.
	if ( pbi->Configuration.Interlaced /*&& GetMBFrameVertVar(cpi) > GetMBFieldVertVar(cpi)*/ )
	{
		pbi->mbi.Interlaced = 1;
		pbi->MBInterlaced[MBOffset(MBrow,MBcol)] = 1;

	    pbi->mbi.blockDxInfo[0].CurrentReconStride =
        pbi->mbi.blockDxInfo[1].CurrentReconStride =
        pbi->mbi.blockDxInfo[2].CurrentReconStride =
        pbi->mbi.blockDxInfo[3].CurrentReconStride = 2 * pbi->Configuration.YStride;

	    pbi->mbi.blockDxInfo[0].CurrentSourceStride =
        pbi->mbi.blockDxInfo[1].CurrentSourceStride =
        pbi->mbi.blockDxInfo[2].CurrentSourceStride =
        pbi->mbi.blockDxInfo[3].CurrentSourceStride = 2 * pbi->Configuration.VideoFrameWidth;

        pbi->mbi.blockDxInfo[1].thisRecon = pbi->mbi.blockDxInfo[0].thisRecon + 8;
        pbi->mbi.blockDxInfo[2].thisRecon = pbi->mbi.blockDxInfo[0].thisRecon + pbi->Configuration.YStride;
        pbi->mbi.blockDxInfo[3].thisRecon = pbi->mbi.blockDxInfo[2].thisRecon + 8;

        pbi->mbi.blockDxInfo[1].Source = pbi->mbi.blockDxInfo[0].Source + 8;
        pbi->mbi.blockDxInfo[2].Source = pbi->mbi.blockDxInfo[0].Source + pbi->Configuration.VideoFrameWidth;
        pbi->mbi.blockDxInfo[3].Source = pbi->mbi.blockDxInfo[2].Source + 8;
	} 
	else	
	{
		pbi->mbi.Interlaced = 0;
		pbi->MBInterlaced[MBOffset(MBrow,MBcol)] = 0;

	    pbi->mbi.blockDxInfo[0].CurrentReconStride =
        pbi->mbi.blockDxInfo[1].CurrentReconStride =
        pbi->mbi.blockDxInfo[2].CurrentReconStride =
        pbi->mbi.blockDxInfo[3].CurrentReconStride = pbi->Configuration.YStride;

	    pbi->mbi.blockDxInfo[0].CurrentSourceStride =
        pbi->mbi.blockDxInfo[1].CurrentSourceStride =
        pbi->mbi.blockDxInfo[2].CurrentSourceStride =
        pbi->mbi.blockDxInfo[3].CurrentSourceStride = pbi->Configuration.VideoFrameWidth;

        pbi->mbi.blockDxInfo[1].thisRecon = pbi->mbi.blockDxInfo[0].thisRecon + 8;
        pbi->mbi.blockDxInfo[2].thisRecon = pbi->mbi.blockDxInfo[0].thisRecon + (pbi->Configuration.YStride << 3);
        pbi->mbi.blockDxInfo[3].thisRecon = pbi->mbi.blockDxInfo[2].thisRecon + 8;

        pbi->mbi.blockDxInfo[1].Source = pbi->mbi.blockDxInfo[0].Source + 8;
        pbi->mbi.blockDxInfo[2].Source = pbi->mbi.blockDxInfo[0].Source + (pbi->Configuration.VideoFrameWidth << 3);
        pbi->mbi.blockDxInfo[3].Source = pbi->mbi.blockDxInfo[2].Source + 8;
	}

	// Calculate the U and V pointers (not affected by interlaced mode) for use in Rd code.
	if ( cpi->RdOpt )
	{
        pbi->mbi.blockDxInfo[4].Source = pbi->UDataOffset + ((MBrow-BORDER_MBS) * 8) * (pbi->Configuration.VideoFrameWidth/2) + ((MBcol-BORDER_MBS) * 8);
        pbi->mbi.blockDxInfo[5].Source = pbi->VDataOffset + ((MBrow-BORDER_MBS) * 8) * (pbi->Configuration.VideoFrameWidth/2) + ((MBcol-BORDER_MBS) * 8);

        pbi->mbi.blockDxInfo[4].thisRecon = pbi->ReconUDataOffset + (MBrow * 8) * pbi->Configuration.UVStride + (MBcol * 8);
        pbi->mbi.blockDxInfo[5].thisRecon = pbi->ReconVDataOffset + (MBrow * 8) * pbi->Configuration.UVStride + (MBcol * 8);
	}

	// What are the two nearest motion vectors.
	VP6_FindNearestandNextNearest ( pbi, MBrow, MBcol, 1, &type  );
	VP6_FindNearestandNextNearest ( pbi, MBrow, MBcol, 2, &type2 );

	// Look at the intra coding error. 
	ThisIntraError = GetMBIntraError ( cpi );

	// Keep a cumulative Intra error score for the frame (clip individual values to an allowed range)
	Temp = ThisIntraError >> 8;
	if ( Temp < MIN_ERR )
		Temp = MIN_ERR;
	else if (  Temp > MAX_ERR )
		Temp = MAX_ERR;
	*IntraError += Temp;
	ThisMbIntraErr = Temp;

	EstModeCost = RdModeCost ( cpi, MBrow, MBcol, CODE_INTRA );
	ThisError = EstModeCost*EPB;
	ThisIntraError += ThisError;

	// To start with set best mode etc to Intra values
	BestMode  = CODE_INTRA;
	BestError = ThisIntraError;

	// Apply Intra weighting factors to best error
	BestError = (ThisIntraError >> 7) * IntraFactors[pbi->quantizer->FrameQIndex];
	if ( cpi->MBCodingMode != CODE_INTRA ) 
		BestError += (cpi->IntraThresh); 
	else
		BestError += (cpi->IntraThresh >> 1);

	// Set Best Rate and Dist if appropriate.
	if ( cpi->RdOpt )
	{
		SetMBMotionVectorsAndMode ( cpi, FragsToCheck, CODE_INTRA, &ZeroMVect ); 
		EncodeMacroBlock_RD ( cpi, FragsToCheck, MBrow, MBcol, &BestRate, &BestDist );
		BestRate += EstModeCost;
		
		// Calculate a BestRd value for Intra
		BestRd = RdFunction ( cpi, BestRate, BestDist );
	}
 
	// pick the best of the set of inter modes with known motion vectors
	if ( !cpi->GfRecoveryFrame )
    {
        PickBetterMBMode ( cpi, 
                           FragsToCheck, 
                           CODE_INTER_NO_MV, 
                           &ZeroMVect, 
                           MBrow, 
                           MBcol, 
                           pbi->LastFrameRecon, 
                           &BestMode, 
                           &BestError, 
                           &MVect, 
                           cpi->ZeroError, 
                           &BestRate, 
                           &BestDist, 
                           &BestRd );
    }

	if( (!cpi->GfRecoveryFrame) && (pbi->mbi.NearestInterMVect.x || pbi->mbi.NearestInterMVect.y) )
	{
		PickBetterMBMode ( cpi,
                          FragsToCheck,
                          CODE_INTER_NEAREST_MV, 
                          &pbi->mbi.NearestInterMVect, 
                          MBrow, 
                          MBcol, 
                          pbi->LastFrameRecon, 
                          &BestMode, 
                          &BestError, 
                          &MVect, 
                          cpi->NearestError, 
                          &BestRate, 
                          &BestDist, 
                          &BestRd );
	}

	if( (!cpi->GfRecoveryFrame) && ( pbi->mbi.NearInterMVect.x || pbi->mbi.NearInterMVect.y) )
	{
		PickBetterMBMode ( cpi,
                           FragsToCheck,
                           CODE_INTER_NEAR_MV,
                           &pbi->mbi.NearInterMVect,
                           MBrow,
                           MBcol,
                           pbi->LastFrameRecon,
                           &BestMode,
                           &BestError,
                           &MVect,
                           cpi->NearError, 
                           &BestRate, 
                           &BestDist, 
                           &BestRd );
	}

	PickBetterMBMode ( cpi,
                       FragsToCheck,
                       CODE_USING_GOLDEN,
                       &ZeroMVect,
                       MBrow,
                       MBcol,
                       pbi->GoldenFrame,
                       &BestMode,
                       &BestError,
                       &MVect,
                       TempError, 
                       &BestRate, 
                       &BestDist, 
                       &BestRd );

	if(pbi->mbi.NearestGoldMVect.x || pbi->mbi.NearestGoldMVect.y)
	{
		PickBetterMBMode ( cpi,
                           FragsToCheck,
                           CODE_GOLD_NEAREST_MV,
                           &pbi->mbi.NearestGoldMVect,
                           MBrow,
                           MBcol,
                           pbi->GoldenFrame,
                           &BestMode,
                           &BestError,
                           &MVect,
                           TempError, 
                           &BestRate, 
                           &BestDist, 
                           &BestRd );
	}

	if ( pbi->mbi.NearGoldMVect.x || pbi->mbi.NearGoldMVect.y )
	{
		PickBetterMBMode ( cpi,
                           FragsToCheck,
                           CODE_GOLD_NEAR_MV,
                           &pbi->mbi.NearGoldMVect,
                           MBrow,
                           MBcol,
                           pbi->GoldenFrame,
                           &BestMode,
                           &BestError,
                           &MVect,
                           TempError, 
                           &BestRate, 
                           &BestDist, 
                           &BestRd );
	}

    // DEBUG Code...
	{
		int a = (BestError >> 17);
		if ( a>127 )
			cpi->ErrorBins[127]++;
		else
			cpi->ErrorBins[a]++;
	}

	// (Note: ignoring this threshold for RD doesn't seem to help much)
	if ( (!cpi->GfRecoveryFrame) && (BestError > cpi->MinErrorForMacroBlockMVSearch) ) 
	{
		PickBetterMBModeAndMV ( cpi,
                                FragsToCheck,
                                CODE_INTER_PLUS_MV,
                                pbi->LastFrameRecon,
                                MBrow,
                                MBcol,
                                &BestMode,
                                &BestError, 
                                &MVect, 
                                TRUE, 
                                cpi->BestError, 
                                &BestRate, 
                                &BestDist, 
                                &BestRd );
	}
	
	// (Note: ignoring this threshold for RD doesn't seem to help much)
	if ( BestError > cpi->MinErrorForGoldenMVSearch ) 
	{
		PickBetterMBModeAndMV ( cpi,
                                FragsToCheck,
                                CODE_GOLDEN_MV,
                                pbi->GoldenFrame,
                                MBrow,
                                MBcol,
                                &BestMode,
                                &BestError, 
                                &MVect, 
                                FALSE, 
                                TempError, 
                                &BestRate, 
                                &BestDist, 
                                &BestRd );
	}
 	
	// Finaly... If the best error is still to high then consider the 4MV mode
	EstModeCost = RdModeCost(cpi,MBrow,MBcol,CODE_INTER_FOURMV);
	ThisError   = EstModeCost * EPB;

	// Only consider 4-Mode mode if the best prediction error so far is above a threshold
	// (Note that ignoring this threshold for RD doesn't seem to help much)
	if ( (!cpi->GfRecoveryFrame) && ((ThisError + cpi->MinImprovementForFourMV) < BestError) )
	{
		UINT32 Error;
		UINT32 RdValue;
		UINT32 Rate = 0;
		UINT32 Dist = 0;

		for ( i=0; i<4; i++ )
		{
			PickBlockMode ( cpi, MBrow, MBcol, i, &FourMode[i], &FourMVect[i], &Error );
			ThisError += Error;
		}
		
		// Calculate the UV vectors as the average of the Y plane ones.
		// First .x component
		FourMVect[4].x = FourMVect[0].x + FourMVect[1].x + FourMVect[2].x + FourMVect[3].x;
		if ( FourMVect[4].x >= 0 )
			FourMVect[4].x = (FourMVect[4].x + 2) / 4;
		else
			FourMVect[4].x = (FourMVect[4].x - 2) / 4;
		FourMVect[5].x = FourMVect[4].x;
		
		// Then .y component
		FourMVect[4].y = FourMVect[0].y + FourMVect[1].y + FourMVect[2].y + FourMVect[3].y;
		if ( FourMVect[4].y >= 0 )
			FourMVect[4].y = (FourMVect[4].y + 2) / 4;
		else
			FourMVect[4].y = (FourMVect[4].y - 2) / 4;
		FourMVect[5].y = FourMVect[4].y;

		// Do Rd for selected modes
		if ( cpi->RdOpt )
		{
			// Set up the individual block modes and motion vector structures
			pbi->mbi.Mode = CODE_INTER_FOURMV;
			SetFragMotionVectorAndMode ( pbi, FragsToCheck[0], &FourMVect[0], FourMode[0] );
			SetFragMotionVectorAndMode ( pbi, FragsToCheck[1], &FourMVect[1], FourMode[1] );
			SetFragMotionVectorAndMode ( pbi, FragsToCheck[2], &FourMVect[2], FourMode[2] );
			SetFragMotionVectorAndMode ( pbi, FragsToCheck[3], &FourMVect[3], FourMode[3] );
			SetFragMotionVectorAndMode ( pbi, FragsToCheck[4], &FourMVect[4], CODE_INTER_FOURMV );
			SetFragMotionVectorAndMode ( pbi, FragsToCheck[5], &FourMVect[5], CODE_INTER_FOURMV );

			// Now calculate Rate and distortion
			EncodeMacroBlock_RD ( cpi, FragsToCheck, MBrow, MBcol, &Rate, &Dist );
			Rate += EstModeCost;    // Add in the cost of specifying 4-Mode mode in the first place
			
			// Add in the cost of the 4 individual modes and Mvs
			for ( i=0; i<4; i++ )
			{
				Rate += blockModeCost ( cpi, MBrow, MBcol, FourMode[i] );
				if ( FourMode[i] == CODE_INTER_PLUS_MV )
				{
					DifferentialVector.x = FourMVect[i].x;
					DifferentialVector.y = FourMVect[i].y;
					if ( pbi->mbi.NearestMvIndex < MAX_NEAREST_ADJ_INDEX )
					{
						DifferentialVector.x -= pbi->mbi.NearestInterMVect.x;
						DifferentialVector.y -= pbi->mbi.NearestInterMVect.y;
					}

					Rate += cpi->EstMvCostPtrX[DifferentialVector.x] + cpi->EstMvCostPtrY[DifferentialVector.y];
				}
			}

			// Finaly plug the combined Rate and distortion values into the RD function.
			RdValue = RdFunction ( cpi, Rate, Dist );

			if ( RdValue < BestRd )
			{
				BestRd    = RdValue;
				BestRate  = Rate;
				BestDist  = Dist;
				BestError = ThisError;
				BestMode  = CODE_INTER_FOURMV;
			}
		}
		else if ( (ThisError + cpi->MinImprovementForFourMV) < BestError )
		{
			BestError = ThisError;
			BestMode  = CODE_INTER_FOURMV;
		}
	}

	// Keep a cumulative best error score for the frame (clip individual values to an allowed range)
	// For The Intra mode case use ThisIntraError not BestError because BestError has been modified
	// by intra weighting factors and could be less than mode cost.
	if ( BestMode != CODE_INTRA )
		Temp = ( (BestError - (modeCost( cpi, MBrow, MBcol, BestMode )*EPB)) >>8 );
	else 
		Temp = ( (ThisIntraError - (modeCost( cpi, MBrow, MBcol, BestMode )*EPB)) >>8 );

	if ( Temp < MIN_ERR )
		Temp = MIN_ERR;
	else if ( Temp > MAX_ERR )
		Temp = MAX_ERR;
	*InterError += Temp;
	ThisMbInterErr = Temp;

	// Record of intra and inter error for motion modes
	if ( (BestMode != CODE_INTRA) && (BestMode != CODE_INTER_NO_MV) && (BestMode != CODE_USING_GOLDEN) )
	{
		// Keep a record of motion related inta and intra prediction errors
		cpi->MotionIntraErr += ThisMbIntraErr;
		cpi->MotionInterErr += ThisMbInterErr;
	}

	// Keep running total of the approximate cost of the chosen mode / MVs etc
	cpi->ModeMvCostEstimate += modeCost ( cpi, MBrow, MBcol, BestMode );

	// keep track of how many times this mode is the same as the last one we encountered
	if ( (pbi->mbi.NearestInterMVect.x == 0) && (pbi->mbi.NearestInterMVect.y == 0) )
		type = NONEAREST_MACROBLOCK;
	else if ( (pbi->mbi.NearInterMVect.x == 0) && (pbi->mbi.NearInterMVect.y == 0) )
		type = NONEAR_MACROBLOCK;
	else
		type = MACROBLOCK;

	//type = 0;
	cpi->CountModeSameAsLast[type][BestMode] += (cpi->MBCodingMode == BestMode);
	cpi->CountModeDiffFrLast[type][BestMode] += (cpi->MBCodingMode != BestMode);
	cpi->MBModeCount[type][BestMode]++;
	cpi->MBCodingMode = BestMode;

	switch ( BestMode )
	{
	case CODE_INTER_FOURMV:

		for ( i=0; i<4; i++ )
		{
			cpi->BModeCount[FourMode[i]]++;

			// Running total modeMv costs
			cpi->ModeMvCostEstimate += blockModeCost ( cpi, MBrow, MBcol, FourMode[i] );
		}

		// Set up mb mode and mv structures for four mv
		SetFragMotionVectorAndMode ( pbi, FragsToCheck[0], &FourMVect[0], FourMode[0] );
		SetFragMotionVectorAndMode ( pbi, FragsToCheck[1], &FourMVect[1], FourMode[1] );
		SetFragMotionVectorAndMode ( pbi, FragsToCheck[2], &FourMVect[2], FourMode[2] );
		SetFragMotionVectorAndMode ( pbi, FragsToCheck[3], &FourMVect[3], FourMode[3] );
		SetFragMotionVectorAndMode ( pbi, FragsToCheck[4], &FourMVect[4], CODE_INTER_FOURMV );
		SetFragMotionVectorAndMode ( pbi, FragsToCheck[5], &FourMVect[5], CODE_INTER_FOURMV );
		
		for ( i=0; i<4; i++ )
		{
			if ( FourMode[i] == CODE_INTER_PLUS_MV )
			{
				DifferentialVector.x = FourMVect[i].x;
				DifferentialVector.y = FourMVect[i].y;
				if ( pbi->mbi.NearestMvIndex < MAX_NEAREST_ADJ_INDEX )
				{
					DifferentialVector.x -= pbi->mbi.NearestInterMVect.x;
					DifferentialVector.y -= pbi->mbi.NearestInterMVect.y;
				}

				cpi->MvBaselineDist[0][(MV_ENTROPY_TOKENS >> 1) + DifferentialVector.x]++;
				cpi->MvBaselineDist[1][(MV_ENTROPY_TOKENS >> 1) + DifferentialVector.y]++;

				// Running total of estimated mode+mv costs
				cpi->ModeMvCostEstimate += (cpi->EstMvCostPtrX[DifferentialVector.x] + cpi->EstMvCostPtrY[DifferentialVector.y]);

				// Store mv stats 
				cpi->FrameMvStats.NumMvs++;
				cpi->FrameMvStats.SumAbsX += abs(FourMVect[i].x);
				cpi->FrameMvStats.SumAbsY += abs(FourMVect[i].y);
				cpi->FrameMvStats.SumX    += FourMVect[i].x;
				cpi->FrameMvStats.SumY    += FourMVect[i].y;
				cpi->FrameMvStats.SumXSq  += FourMVect[i].x * FourMVect[i].x;
				cpi->FrameMvStats.SumYSq  += FourMVect[i].y * FourMVect[i].y;
			}
		}
		
		// Update the new MV and Mode counters
		cpi->FrameNewMvCounter += 4;
		cpi->FrameModeCounter += 4;

		// Update KeyFrameIndicator
		if ( (MBrow >= cpi->FirstSixthBoundary) && (MBrow < cpi->LastSixthBoundary) &&						// Exclude top and bottome for "letterbox in 4:3" video
			 (BestError > KF_INDICATOR_THRESH) && ((ThisIntraError * 2) < (BestError * 5)) )
			cpi->MotionScore ++;
		break;

	case CODE_INTRA:
		SetMBMotionVectorsAndMode ( cpi, FragsToCheck, BestMode, &ZeroMVect );

		// Update KeyFrameIndicator
		if ( (MBrow >= cpi->FirstSixthBoundary) && (MBrow < cpi->LastSixthBoundary) )						// Exclude top and bottome for "letterbox in 4:3" video
			cpi->MotionScore++;

		// Update the Mode counter
		cpi->FrameModeCounter++;

		break;

	case CODE_INTER_PLUS_MV:
	case CODE_GOLDEN_MV:

		DifferentialVector.x = MVect.x;
		DifferentialVector.y = MVect.y;
		if ( BestMode == CODE_INTER_PLUS_MV )
		{
			if ( pbi->mbi.NearestMvIndex < MAX_NEAREST_ADJ_INDEX )
			{
				DifferentialVector.x -= pbi->mbi.NearestInterMVect.x;
				DifferentialVector.y -= pbi->mbi.NearestInterMVect.y;
			}
			else
			{
				DifferentialVector.x = MVect.x;
				DifferentialVector.y = MVect.y;
			}
		}
		else
		{
			if ( pbi->mbi.NearestGMvIndex < MAX_NEAREST_ADJ_INDEX )
			{
				DifferentialVector.x -= pbi->mbi.NearestGoldMVect.x;
				DifferentialVector.y -= pbi->mbi.NearestGoldMVect.y;
			}
			else
			{
				DifferentialVector.x = MVect.x;
				DifferentialVector.y = MVect.y;
			}
		}

		SetMBMotionVectorsAndMode ( cpi, FragsToCheck, BestMode, &MVect );

		// Update KeyFrameIndicator
		if ( (MBrow >= cpi->FirstSixthBoundary) && (MBrow < cpi->LastSixthBoundary) &&						// Exclude top and bottome for "letterbox in 4:3" video
			 (BestError > KF_INDICATOR_THRESH) && ((ThisIntraError * 2) < (BestError * 5)) )
			cpi->MotionScore++;

		cpi->MvBaselineDist[0][(MV_ENTROPY_TOKENS >> 1) + DifferentialVector.x]++;
		cpi->MvBaselineDist[1][(MV_ENTROPY_TOKENS >> 1) + DifferentialVector.y]++;

		// Update the new MV and Mode counters
		cpi->FrameNewMvCounter++;
		cpi->FrameModeCounter++;

		// Running total of estimated mode+mv costs
		cpi->ModeMvCostEstimate += (cpi->EstMvCostPtrX[DifferentialVector.x] + cpi->EstMvCostPtrY[DifferentialVector.y]);

		// Store mv stats (exclude GF)
		if ( BestMode == CODE_INTER_PLUS_MV)
		{
			cpi->FrameMvStats.NumMvs++;
			cpi->FrameMvStats.SumAbsX += abs(MVect.x);
			cpi->FrameMvStats.SumAbsY += abs(MVect.y);
			cpi->FrameMvStats.SumX    += MVect.x;
			cpi->FrameMvStats.SumY    += MVect.y;
			cpi->FrameMvStats.SumXSq  += MVect.x * MVect.x;
			cpi->FrameMvStats.SumYSq  += MVect.y * MVect.y;
		}
		break;

	default:
		SetMBMotionVectorsAndMode ( cpi, FragsToCheck, BestMode, &MVect );
		
        // Update KeyFrameIndicator
		if ( (MBrow >= cpi->FirstSixthBoundary) && (MBrow < cpi->LastSixthBoundary) &&						// Exclude top and bottome for "letterbox in 4:3" video
			 (BestError > KF_INDICATOR_THRESH) && ((ThisIntraError * 2) < (BestError * 5)) )
			cpi->MotionScore++;

		// Update the Mode counters
		cpi->FrameModeCounter++;

		// Store mv stats (exclude GF mv modes)
		if ( (BestMode == CODE_INTER_NEAREST_MV) || (BestMode == CODE_INTER_NEAR_MV) )
		{
			cpi->FrameMvStats.NumMvs++;
			cpi->FrameMvStats.SumAbsX += abs(MVect.x);
			cpi->FrameMvStats.SumAbsY += abs(MVect.y);
			cpi->FrameMvStats.SumX    += MVect.x;
			cpi->FrameMvStats.SumY    += MVect.y;
			cpi->FrameMvStats.SumXSq  += MVect.x * MVect.x;
			cpi->FrameMvStats.SumYSq  += MVect.y * MVect.y;
		}

		break;
	}

	// Keep a record of the distribution of mode choices in this frame
	cpi->ModeDist[BestMode]++;
	pbi->predictionMode[MBOffset(MBrow,MBcol)]   = BestMode;
	pbi->MBMotionVector[MBOffset(MBrow,MBcol)].x = pbi->FragInfo[FragsToCheck[3]].MVectorX;
	pbi->MBMotionVector[MBOffset(MBrow,MBcol)].y = pbi->FragInfo[FragsToCheck[3]].MVectorY;

	// If Rd Opt is enabled then restore the macro block Dc Prediction context for chosen mode.
	if ( cpi->RdOpt )
	{
		for ( i=0; i<6; i++ )
		{
			memcpy ( cpi->MbDcContexts[BestMode][i].AbovePtr,  &cpi->MbDcContexts[BestMode][i].Above,  sizeof(BLOCK_CONTEXT) );
			memcpy ( cpi->MbDcContexts[BestMode][i].LeftPtr,   &cpi->MbDcContexts[BestMode][i].Left,   sizeof(BLOCK_CONTEXT) );
			memcpy ( cpi->MbDcContexts[BestMode][i].LastDcPtr, &cpi->MbDcContexts[BestMode][i].LastDc, sizeof(Q_LIST_ENTRY)  );

		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : PickModes
 *
 *  INPUTS        : CP_INSTANCE	*cpi   : Pointer to encoder instance.
 *	                
 *  OUTPUTS       : UINT32 *InterError : Pointer to inter-mode error.
 *	                UINT32 *IntraError : Pointer to intra-mode error.
 *
 *  RETURNS       : UINT32: 0 Always.
 *
 *  FUNCTION      : Picks the best coding mode for each macro-block in
 *                  the frame.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
UINT32 PickModes ( CP_INSTANCE *cpi, UINT32 *InterError, UINT32 *IntraError )
{
    UINT8   QIndex;
	UINT32  MBrow, MBcol;
	unsigned int duration;
	unsigned int starttsc,endtsc;
	UINT32 CountInterlaced = 0;
	PB_INSTANCE *pbi = &cpi->pb;

    // Record start time
    VP6_readTSC ( &starttsc );

	// Work new motion vector cost weighting based upon the frequency of new motion vectors in the last frame.
	if ( cpi->FrameModeCounter )
	{
		cpi->LastFrameNewMvUsage = (cpi->FrameNewMvCounter * 10)/cpi->FrameModeCounter; 
		cpi->MvEpbCorrection     = MvEpbCorrectionTable[cpi->LastFrameNewMvUsage];
	}
	else
	{
		cpi->LastFrameNewMvUsage = 0;
		cpi->MvEpbCorrection = MvEpbCorrectionTable[0];
	}

	cpi->FrameModeCounter  = 0;
	cpi->FrameNewMvCounter = 0;

	// Set flag to allow bit cost anlaylsis without actual output.
	cpi->bc.MeasureCost = TRUE;
    QIndex = pbi->quantizer->FrameQIndex;

	memset ( (void *)cpi->MvBaselineDist,      0, sizeof(cpi->MvBaselineDist) );
	memset ( (void *)cpi->MBModeCount,         0, sizeof(cpi->MBModeCount) );
	memset ( (void *)cpi->CountModeSameAsLast, 0, sizeof(cpi->CountModeSameAsLast) );
	memset ( (void *)cpi->CountModeDiffFrLast, 0, sizeof(cpi->CountModeDiffFrLast) );
	memset ( (void *)cpi->BModeCount,          0, sizeof(cpi->BModeCount) );

	// Clear down record of frame coding mode distribution
	memset ( cpi->ModeDist, 0, sizeof(cpi->ModeDist) );

	// Clear down frame average abs MV data structure
	memset ( &cpi->FrameMvStats, 0, sizeof(cpi->FrameMvStats) );

	// Clear the mode+mv frame cost estimate
	cpi->ModeMvCostEstimate = 0;

	cpi->ErrorPerBit = ErrorPerBit[QIndex];

	if ( cpi->ErrorPerBit < 1 )
		cpi->ErrorPerBit = 1;

	// Calculate a provisional mv-epb using epb and a correction that depends on 
	// frequency of mv's in last frame.
	cpi->MVErrorPerBit = (ErrorPerBit[QIndex] << 8) / cpi->MvEpbCorrection;

    // initialize error scores
    *InterError = 0;
    *IntraError = 0;

	// Initialise key frame indicator.
	cpi->MotionScore = 0;

	// Initialise mode variable for use in mode weighting tests
	cpi->MBCodingMode = CODE_INTER_NO_MV;

	// Error threshold where we consider forcing INTRA mode.
    cpi->InterTripOutThresh = (5000<<12);

	// Test Values
	cpi->IntraThresh = (IntraThreshTable[QIndex] << 12); 

    switch ( cpi->QuickCompress )
    {
	case 2:  
		{
			// this auto speed selection code needs some work !!!
			UINT32 millisecondsForCompress = 1000000 / cpi->Configuration.OutputFrameRate;
			millisecondsForCompress = millisecondsForCompress * (16-cpi->CPUUsed) / 16;
			
			if ( cpi->avgEncodeTime+cpi->avgPackVideoTime < millisecondsForCompress )
			{
				millisecondsForCompress -= cpi->avgEncodeTime + cpi->avgPackVideoTime;
			
				if ( cpi->avgPickModeTime == 0 )
				{
					cpi->Speed = 4;
				}
				else
				{
					// why just go up by 1 and not try to calculate the value 
					// that would compress fast enough (etc)??
				    if ( millisecondsForCompress <  cpi->avgPickModeTime )
                    {
                        cpi->Speed          += 3;
                        cpi->avgPickModeTime = 0;                        
                    }                    
                    else if ( millisecondsForCompress*100 > cpi->avgPickModeTime*130 )
                    {
                        cpi->Speed          -= 1;
                        cpi->avgPickModeTime = 0;
                    }

                    if ( cpi->Speed < 4 )
                        cpi->Speed = 4;
					else if ( cpi->Speed > 16 )
						cpi->Speed = 16;
				}
			}
			else
			{
				cpi->Speed = 16;
			}

            cpi->MinErrorForMacroBlockMVSearch = 25   << 12;
            cpi->MinErrorForGoldenMVSearch     = 40   << 12;
            cpi->ExhaustiveSearchThresh        = 1000 << 12;
            cpi->MinErrorForBlockMVSearch      = 50   << 12;
            cpi->FindMvViaSearch        = FindMvVia3StepSearch;
            cpi->FindBestHalfPixelMv    = FindBestFractionalPixelStep;
            cpi->FindBestQuarterPixelMv = FindBestFractionalPixelStep;
            cpi->BlockExhaustiveSearchThresh   = HUGE_ERROR;
			
            if ( cpi->Speed >= 1 )
                cpi->FindMvViaSearch = FindMvViaDiamondSearch;
            if ( cpi->Speed >= 2 )
                cpi->FindBestQuarterPixelMv = SkipFractionalPixelStep;
            if ( cpi->Speed >= 3 )
                cpi->MinErrorForGoldenMVSearch = HUGE_ERROR;
            if ( cpi->Speed >= 4 )
                cpi->MinErrorForBlockMVSearch = HUGE_ERROR;
            if ( cpi->Speed >= 14 )
                cpi->FindBestHalfPixelMv = SkipFractionalPixelStep;
            if ( cpi->Speed >= 5 )
            {
                unsigned int i, sum=0;

				for ( i=0; i<128; i++ )
				{
					sum += cpi->ErrorBins[i];
					if ( 10*sum>(cpi->Speed-6)*(pbi->MBRows-4)*(pbi->MBCols-4) )
						break;
				}
				++i;
				cpi->MinErrorForMacroBlockMVSearch = i << 17;
                cpi->ExhaustiveSearchThresh        = i << 23;
			}
            if ( cpi->Speed >= 12 )
                cpi->ExhaustiveSearchThresh = HUGE_ERROR;

            memset ( cpi->ErrorBins, 0, sizeof(cpi->ErrorBins) );
		}
		break;
	
    case 1: 
		cpi->MinErrorForMacroBlockMVSearch = 25   << 12;
		cpi->MinErrorForGoldenMVSearch     = 25   << 12;
		cpi->ExhaustiveSearchThresh        = 1000  << 12;
		cpi->MinErrorForBlockMVSearch      = 50   << 12;
        cpi->BlockExhaustiveSearchThresh   = HUGE_ERROR;
		break;

	case 3: 
		cpi->MinErrorForMacroBlockMVSearch = 25   << 12;
		cpi->MinErrorForGoldenMVSearch     = 25   << 12;
		cpi->ExhaustiveSearchThresh        = 1000 << 12;
		cpi->MinErrorForBlockMVSearch      = 50   << 12;
        cpi->BlockExhaustiveSearchThresh   = HUGE_ERROR;
		cpi->RdOpt = 2;				
        break;

	case 0: 
		cpi->MinErrorForMacroBlockMVSearch = 25   << 12;
		cpi->MinErrorForGoldenMVSearch     = 25   << 12;
		cpi->ExhaustiveSearchThresh        = 300   << 12;
        cpi->BlockExhaustiveSearchThresh   = 40  << 12;
		cpi->MinErrorForBlockMVSearch      = 20  << 12;
		cpi->RdOpt = 2;				
		break;
  } 

	// Extra cost penalty to prevent spurious use of 4mv mode.
	// The reason this is needed probably has something to do with 
	// poorer dc prediction with a 4mv macro block than within a 
	// macro block where all are coded with the same mode.
	cpi->MinImprovementForFourMV = FourModeImprovement[QIndex]<<12;

	// Define boundaries to be used in key frame selection process
	cpi->FirstSixthBoundary = (pbi->MBRows-(2*BORDER_MBS))/6+2;			// Macro block index marking the first sixth of the image
	cpi->LastSixthBoundary  = ((pbi->MBRows-(2*BORDER_MBS))*5)/6+2;		// Macro block index marking the last sixth of the image

	// If we are using RdOpt then reset the Above dc context data structure
	if ( cpi->RdOpt )
		VP6_ResetAboveContext ( pbi );

	// decide what block type and motion vectors to use on all of the frames
	for ( MBrow=BORDER_MBS; MBrow<pbi->MBRows-BORDER_MBS; MBrow++ )
	{
		// If we are using RdOpt then reset the Left dc context data structure for each row of MBs
		if ( cpi->RdOpt )
			VP6_ResetLeftContext ( pbi );

		for ( MBcol=BORDER_MBS; MBcol < pbi->MBCols-BORDER_MBS; MBcol++ )
		{
			// Try to pick the best mode for the macro block
			PickMacroBlockMode ( cpi, MBrow, MBcol,InterError, IntraError );

			if ( pbi->MBInterlaced[MBOffset(MBrow,MBcol)] )
				CountInterlaced++;
		}
	}

	pbi->probInterlaced = 256-(1+254*CountInterlaced/((pbi->MBRows-(2*BORDER_MBS))*(pbi->MBCols-(2*BORDER_MBS))));

    // system state should be cleared here....
#if defined(_MSC_VER)
	ClearSysState();
#endif

    // Recored end time & compute duration
	VP6_readTSC(&endtsc);
	duration = (endtsc - starttsc) / pbi->ProcessorFrequency;
	
    if ( cpi->avgPickModeTime == 0)
		cpi->avgPickModeTime = duration;
	else
		cpi->avgPickModeTime = (7*cpi->avgPickModeTime+duration)>>3;

	return 0;
}
