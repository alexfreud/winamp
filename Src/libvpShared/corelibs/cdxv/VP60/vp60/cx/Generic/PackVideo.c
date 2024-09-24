/****************************************************************************
*
*   Module Title :     PackVideo.c
*
*   Description  :     Bitstream Packing Routines for VP6.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "compdll.h"
#include "encodemode.h"
#include "encodemv.h"
#include "TokenEntropy.h"
#include "systemdependant.h"

/****************************************************************************
*  Macros
****************************************************************************/
#define PROB_UPDATE_CORECTION	(-1)		

#define MAX_DC_ZRL  74      // Maximum run of zeros at DC position (11 + 63)

/****************************************************************************
*  Exports
****************************************************************************/
UINT32 scanupdates[64][2];

const UINT8 DcZrlHuffCode[5]    = { 0, 1, 2, 6, 7  };
const UINT8 DcZrlHuffLength[5]  = { 2, 2, 2, 3, 3  };
const UINT8 DcZrlExtraOffset[5] = { 1, 2, 3, 7, 11 };
const UINT8 DcZrlExtraLength[5] = { 0, 0, 2, 2, 6  };

const UINT8 DcZrlHuffBand[MAX_DC_ZRL+1] = 
{
    0, 0, 1, 2, 2, 2, 2, 3,
    3, 3, 3, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4
};

/****************************************************************************
*  Imports
****************************************************************************/ 
extern void ConvertBoolTrees ( PB_INSTANCE *pbi );

/****************************************************************************
 * 
 *  ROUTINE       : GetOptimalFrameZrlProbs
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       :	None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Calculate optimal ZRL node probabilities and
 *                  hit counts from ZRL distribution data.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void GetOptimalFrameZrlProbs ( CP_INSTANCE *cpi )
{
	UINT32 i,j;
	UINT32 Sum, Sum2;
	UINT32 BitSums[ZRL_BANDS][6][2];
	UINT32 RunLength;
	UINT32 Count;
	UINT32 Index;

	// Clear down BitSums workspace
	memset ( BitSums, 0, sizeof(BitSums) );

	// Work out the optimised nodes probabilities relating to explicit values
	for ( i=0; i<ZRL_BANDS; i++ )
	{
		// branch hits and probility for the top node ( is Run > 4 )
		Sum = cpi->FrameZeroCount[i];
		Sum2 = cpi->FrameZrlDist[i][1] + cpi->FrameZrlDist[i][2] + cpi->FrameZrlDist[i][3] + cpi->FrameZrlDist[i][4];
		cpi->FrameZrlBranchHits[i][0][0] = Sum2;
		cpi->FrameZrlBranchHits[i][0][1] = Sum - Sum2;
		if ( Sum )
			cpi->FrameZrlProbs[i][0] = (Sum2 * 255) / Sum;

		// Second Node 1,2 vs 3,4
		Sum = cpi->FrameZrlDist[i][1] + cpi->FrameZrlDist[i][2] + cpi->FrameZrlDist[i][3] + cpi->FrameZrlDist[i][4];
		Sum2 = cpi->FrameZrlDist[i][1] + cpi->FrameZrlDist[i][2];
		cpi->FrameZrlBranchHits[i][1][0] = Sum2;
		cpi->FrameZrlBranchHits[i][1][1] = Sum - Sum2;
		if ( Sum )
			cpi->FrameZrlProbs[i][1] = (Sum2 * 255) / Sum;

		// Third Node 1 vs 2
		Sum = cpi->FrameZrlDist[i][1] + cpi->FrameZrlDist[i][2];
		Sum2 = cpi->FrameZrlDist[i][1];
		cpi->FrameZrlBranchHits[i][2][0] = Sum2;
		cpi->FrameZrlBranchHits[i][2][1] = Sum - Sum2;
		if ( Sum )
			cpi->FrameZrlProbs[i][2] = (Sum2 * 255) / Sum;

		// Fourth Node 3 vs 4
		Sum = cpi->FrameZrlDist[i][3] + cpi->FrameZrlDist[i][4];
		Sum2 = cpi->FrameZrlDist[i][3];
		cpi->FrameZrlBranchHits[i][3][0] = Sum2;
		cpi->FrameZrlBranchHits[i][3][1] = Sum - Sum2;
		if ( Sum )
			cpi->FrameZrlProbs[i][3] = (Sum2 * 255) / Sum;

		// Fifth Node 5-8 vs >8
		Sum = cpi->FrameZeroCount[i] - 
			  (cpi->FrameZrlDist[i][1] + cpi->FrameZrlDist[i][2] + cpi->FrameZrlDist[i][3] + cpi->FrameZrlDist[i][4]);
		Sum2 = cpi->FrameZrlDist[i][5] + cpi->FrameZrlDist[i][6] + cpi->FrameZrlDist[i][7] + cpi->FrameZrlDist[i][8];
		cpi->FrameZrlBranchHits[i][4][0] = Sum2;
		cpi->FrameZrlBranchHits[i][4][1] = Sum - Sum2;
		if ( Sum )
			cpi->FrameZrlProbs[i][4] = (Sum2 * 255) / Sum;

		// Sixth Node 5,6 vs 7,8
		Sum = cpi->FrameZrlDist[i][5] + cpi->FrameZrlDist[i][6] + cpi->FrameZrlDist[i][7] + cpi->FrameZrlDist[i][8];
		Sum2 = cpi->FrameZrlDist[i][5] + cpi->FrameZrlDist[i][6];
		cpi->FrameZrlBranchHits[i][5][0] = Sum2;
		cpi->FrameZrlBranchHits[i][5][1] = Sum - Sum2;
		if ( Sum )
			cpi->FrameZrlProbs[i][5] = (Sum2 * 255) / Sum;

  		// Seventh Node 5 vs 6
		Sum = cpi->FrameZrlDist[i][5] + cpi->FrameZrlDist[i][6];
		Sum2 = cpi->FrameZrlDist[i][5];
		cpi->FrameZrlBranchHits[i][6][0] = Sum2;
		cpi->FrameZrlBranchHits[i][6][1] = Sum - Sum2;
		if ( Sum )
			cpi->FrameZrlProbs[i][6] = (Sum2 * 255) / Sum;

  		// Eighth Node 7 vs 8
		Sum = cpi->FrameZrlDist[i][7] + cpi->FrameZrlDist[i][8];
		Sum2 = cpi->FrameZrlDist[i][7];
		cpi->FrameZrlBranchHits[i][7][0] = Sum2;
		cpi->FrameZrlBranchHits[i][7][1] = Sum - Sum2;
		if ( Sum )
			cpi->FrameZrlProbs[i][7] = (Sum2 * 255) / Sum;
	}

	// Work out the bit probabilities for the remaining nodes
	for ( i=0; i<ZRL_BANDS; i++ )
	{
		for ( j=9; j<64; j++ )
		{
			RunLength = j - 9;
			Count = cpi->FrameZrlDist[i][j];

			BitSums[i][5][((RunLength >> 5) & 1)] += Count;
			BitSums[i][4][((RunLength >> 4) & 1)] += Count;
			BitSums[i][3][((RunLength >> 3) & 1)] += Count;
			BitSums[i][2][((RunLength >> 2) & 1)] += Count;
			BitSums[i][1][((RunLength >> 1) & 1)] += Count;
			BitSums[i][0][(RunLength & 1)] += Count;
		}

		for ( j=0; j<6; j++ )
		{
			Index = j + 8;					// Index into FrameZrlProbs[] etc. 
			Sum = BitSums[i][j][0] + BitSums[i][j][1];
			Sum2 = BitSums[i][j][0];
			cpi->FrameZrlBranchHits[i][Index][0] = Sum2;
			cpi->FrameZrlBranchHits[i][Index][1] = Sum - Sum2;
			if ( Sum )
				cpi->FrameZrlProbs[i][Index] = (Sum2 * 255) / Sum;
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : ConvertDistribution
 *
 *  INPUTS        : CP_INSTANCE *cpi          : Pointer to encoder instance (NOT USED).
 *                  UINT32 *Distribution      : Token histogram array.
 *
 *  OUTPUTS       :	UINT8 *Probabilities      : Pointer to array of node probs.
 *                  UINT32 BranchChoices[][2] : Histogram of 1/0 branch decisions.
 * 
 *  RETURNS       : void
 *
 *  FUNCTION      : Converts a token distribution array into a set of tree
 *                  node probabilities.
 *
 *  SPECIAL NOTES : The format of the binary decision tree is fixed. 
 *
 ****************************************************************************/
void ConvertDistribution
(
    CP_INSTANCE *cpi,
    UINT32 *Distribution,
    UINT8 *Probabilities, 
    UINT32 BranchChoices[][2] 
)
{
	UINT32 i;
	UINT32 Tmp1;
	UINT32 Tmp2;
	UINT32 SumTokens = 0;

	// Count the token
	for ( i=0; i<MAX_ENTROPY_TOKENS; i++ )
		SumTokens += Distribution[i];

	// Set the default output probabilities
	for ( i=0; i<MAX_ENTROPY_TOKENS-1; i++ )
	{
		Probabilities[i]    = 128;
		BranchChoices[i][0] = 0;
		BranchChoices[i][1] = 0;
	}

	// Trap cases where there are no tokens
	if ( SumTokens > 0 )
	{
		// The first probability we are interested in is the 0 context
		Probabilities[ZERO_CONTEXT_NODE] = (UINT8)(((Distribution[DCT_EOB_TOKEN]+Distribution[ZERO_TOKEN]) * 255)/SumTokens);
		BranchChoices[ZERO_CONTEXT_NODE][0] = (Distribution[DCT_EOB_TOKEN] + Distribution[ZERO_TOKEN]);
		BranchChoices[ZERO_CONTEXT_NODE][1] = SumTokens - (Distribution[DCT_EOB_TOKEN] + Distribution[ZERO_TOKEN]);
		if ( Probabilities[ZERO_CONTEXT_NODE] == 0 )
			Probabilities[ZERO_CONTEXT_NODE] = 1;
		else if ( Probabilities[ZERO_CONTEXT_NODE] > MAX_PROB )
			Probabilities[ZERO_CONTEXT_NODE] = MAX_PROB;

		// Next the Zero/EOB split
		Tmp1 = Distribution[DCT_EOB_TOKEN];
		Tmp2 = Distribution[DCT_EOB_TOKEN]+Distribution[ZERO_TOKEN];
		BranchChoices[EOB_CONTEXT_NODE][0] = Tmp1;
		BranchChoices[EOB_CONTEXT_NODE][1] = Tmp2-Tmp1;
		if ( Tmp2  )
		{
			Probabilities[EOB_CONTEXT_NODE] = (UINT8)(((Tmp1 * 255) + (Tmp2 >> 1))/Tmp2);
			if ( Probabilities[EOB_CONTEXT_NODE] == 0 )
				Probabilities[EOB_CONTEXT_NODE] = 1;
			else if ( Probabilities[EOB_CONTEXT_NODE] > MAX_PROB )
				Probabilities[EOB_CONTEXT_NODE] = MAX_PROB;
		}

		// Now the One Context
		Tmp1 = Distribution[ONE_TOKEN];
		Tmp2 = SumTokens - (Distribution[DCT_EOB_TOKEN]+Distribution[ZERO_TOKEN]);
		BranchChoices[ONE_CONTEXT_NODE][0] = Tmp1;
		BranchChoices[ONE_CONTEXT_NODE][1] = Tmp2-Tmp1;
		if ( Tmp2 )
		{
			Probabilities[ONE_CONTEXT_NODE] = (UINT8)(((Tmp1 * 255) + (Tmp2 >> 1))/Tmp2);
			if ( Probabilities[ONE_CONTEXT_NODE] == 0 )
				Probabilities[ONE_CONTEXT_NODE] = 1;
			else if ( Probabilities[ONE_CONTEXT_NODE] > MAX_PROB )
				Probabilities[ONE_CONTEXT_NODE] = MAX_PROB;
		}

		// Now the LowVal Context
		Tmp1 = Distribution[TWO_TOKEN] + Distribution[THREE_TOKEN] + Distribution[FOUR_TOKEN];
		Tmp2 = Tmp2 - Distribution[ONE_TOKEN];
		BranchChoices[LOW_VAL_CONTEXT_NODE][0] = Tmp1;
		BranchChoices[LOW_VAL_CONTEXT_NODE][1] = Tmp2-Tmp1;
		if ( Tmp2 )
		{
			Probabilities[LOW_VAL_CONTEXT_NODE] = (UINT8)(((Tmp1 * 255) + (Tmp2 >> 1))/Tmp2);
			if ( Probabilities[LOW_VAL_CONTEXT_NODE] == 0 )
				Probabilities[LOW_VAL_CONTEXT_NODE] = 1;
			else if ( Probabilities[LOW_VAL_CONTEXT_NODE] > MAX_PROB )
				Probabilities[LOW_VAL_CONTEXT_NODE] = MAX_PROB;
		}

		// Now the TWO Context
		Tmp1 = Distribution[TWO_TOKEN];
		Tmp2 = Distribution[TWO_TOKEN] + Distribution[THREE_TOKEN] + Distribution[FOUR_TOKEN];
		BranchChoices[TWO_CONTEXT_NODE][0] = Tmp1;
		BranchChoices[TWO_CONTEXT_NODE][1] = Tmp2-Tmp1;
		if ( Tmp2 )
		{
			Probabilities[TWO_CONTEXT_NODE] = (UINT8)(((Tmp1 * 255) + (Tmp2 >> 1))/Tmp2);
			if ( Probabilities[TWO_CONTEXT_NODE] == 0 )
				Probabilities[TWO_CONTEXT_NODE] = 1;
			else if ( Probabilities[TWO_CONTEXT_NODE] > MAX_PROB )
				Probabilities[TWO_CONTEXT_NODE] = MAX_PROB;
		}

		// Now the Three Context
		Tmp1 = Distribution[THREE_TOKEN];
		Tmp2 = Distribution[THREE_TOKEN] + Distribution[FOUR_TOKEN];
		BranchChoices[THREE_CONTEXT_NODE][0] = Tmp1;
		BranchChoices[THREE_CONTEXT_NODE][1] = Tmp2-Tmp1;
		if ( Tmp2 )
		{
			Probabilities[THREE_CONTEXT_NODE] = (UINT8)(((Tmp1 * 255) + (Tmp2 >> 1))/Tmp2);
			if ( Probabilities[THREE_CONTEXT_NODE] == 0 )
				Probabilities[THREE_CONTEXT_NODE] = 1;
			else if ( Probabilities[THREE_CONTEXT_NODE] > MAX_PROB )
				Probabilities[THREE_CONTEXT_NODE] = MAX_PROB;
		}

		// Now the HighLowVal Context
		Tmp1 = Distribution[DCT_VAL_CATEGORY1] + Distribution[DCT_VAL_CATEGORY2];
		Tmp2 = Distribution[DCT_VAL_CATEGORY1] + Distribution[DCT_VAL_CATEGORY2] + Distribution[DCT_VAL_CATEGORY3] + Distribution[DCT_VAL_CATEGORY4] + Distribution[DCT_VAL_CATEGORY5] + Distribution[DCT_VAL_CATEGORY6];
		BranchChoices[HIGH_LOW_CONTEXT_NODE][0] = Tmp1;
		BranchChoices[HIGH_LOW_CONTEXT_NODE][1] = Tmp2-Tmp1;
		if ( Tmp2 )
		{
			Probabilities[HIGH_LOW_CONTEXT_NODE] = (UINT8)(((Tmp1 * 255) + (Tmp2 >> 1))/Tmp2);
			if ( Probabilities[HIGH_LOW_CONTEXT_NODE] == 0 )
				Probabilities[HIGH_LOW_CONTEXT_NODE] = 1;
			else if ( Probabilities[HIGH_LOW_CONTEXT_NODE] > MAX_PROB )
				Probabilities[HIGH_LOW_CONTEXT_NODE] = MAX_PROB;
		}	

		// Now the Cat1 Context
		Tmp1 = Distribution[DCT_VAL_CATEGORY1];
		Tmp2 = Distribution[DCT_VAL_CATEGORY1] + Distribution[DCT_VAL_CATEGORY2];
		BranchChoices[CAT_ONE_CONTEXT_NODE][0] = Tmp1;
		BranchChoices[CAT_ONE_CONTEXT_NODE][1] = Tmp2-Tmp1;
		if ( Tmp2 )
		{
			Probabilities[CAT_ONE_CONTEXT_NODE] = (UINT8)(((Tmp1 * 255) + (Tmp2 >> 1))/Tmp2);
			if ( Probabilities[CAT_ONE_CONTEXT_NODE] == 0 )
				Probabilities[CAT_ONE_CONTEXT_NODE] = 1;
			else if ( Probabilities[CAT_ONE_CONTEXT_NODE] > MAX_PROB )
				Probabilities[CAT_ONE_CONTEXT_NODE] = MAX_PROB;
		}
		
		// Now the Cat3/4 Context
		Tmp1 = Distribution[DCT_VAL_CATEGORY3] + Distribution[DCT_VAL_CATEGORY4];
		Tmp2 = Distribution[DCT_VAL_CATEGORY3] + Distribution[DCT_VAL_CATEGORY4] + Distribution[DCT_VAL_CATEGORY5] + Distribution[DCT_VAL_CATEGORY6];
		BranchChoices[CAT_THREEFOUR_CONTEXT_NODE][0] = Tmp1;
		BranchChoices[CAT_THREEFOUR_CONTEXT_NODE][1] = Tmp2-Tmp1;
		if ( Tmp2 )
		{
			Probabilities[CAT_THREEFOUR_CONTEXT_NODE] = (UINT8)(((Tmp1 * 255) + (Tmp2 >> 1))/Tmp2);
			if ( Probabilities[CAT_THREEFOUR_CONTEXT_NODE] == 0 )
				Probabilities[CAT_THREEFOUR_CONTEXT_NODE] = 1;
			else if ( Probabilities[CAT_THREEFOUR_CONTEXT_NODE] > MAX_PROB )
				Probabilities[CAT_THREEFOUR_CONTEXT_NODE] = MAX_PROB;
		}

		// Now the Cat3 Context
		Tmp1 = Distribution[DCT_VAL_CATEGORY3];
		Tmp2 = Distribution[DCT_VAL_CATEGORY3] + Distribution[DCT_VAL_CATEGORY4];
		BranchChoices[CAT_THREE_CONTEXT_NODE][0] = Tmp1;
		BranchChoices[CAT_THREE_CONTEXT_NODE][1] = Tmp2-Tmp1;
		if ( Tmp2 )
		{
			Probabilities[CAT_THREE_CONTEXT_NODE] = (UINT8)(((Tmp1 * 255) + (Tmp2 >> 1))/Tmp2);
			if ( Probabilities[CAT_THREE_CONTEXT_NODE] == 0 )
				Probabilities[CAT_THREE_CONTEXT_NODE] = 1;
			else if ( Probabilities[CAT_THREE_CONTEXT_NODE] > MAX_PROB )
				Probabilities[CAT_THREE_CONTEXT_NODE] = MAX_PROB;
		}

		// Now the Cat5 Context
		Tmp1 = Distribution[DCT_VAL_CATEGORY5];
		Tmp2 = Distribution[DCT_VAL_CATEGORY5] + Distribution[DCT_VAL_CATEGORY6];
		BranchChoices[CAT_FIVE_CONTEXT_NODE][0] = Tmp1;
		BranchChoices[CAT_FIVE_CONTEXT_NODE][1] = Tmp2-Tmp1;
		if ( Tmp2 )
		{
			Probabilities[CAT_FIVE_CONTEXT_NODE] = (UINT8)(((Tmp1 * 255) + (Tmp2 >> 1))/Tmp2);
			if ( Probabilities[CAT_FIVE_CONTEXT_NODE] == 0 )
				Probabilities[CAT_FIVE_CONTEXT_NODE] = 1;
			else if ( Probabilities[CAT_FIVE_CONTEXT_NODE] > MAX_PROB )
				Probabilities[CAT_FIVE_CONTEXT_NODE] = MAX_PROB;
		}
	}

	// Adjust the probabilities to a 7 bit resolution
	for ( i=0; i<MAX_ENTROPY_TOKENS-1; i++ )
	{
		Probabilities[i] &= ~1;
		if ( Probabilities[i] == 0 )	// 0 not legal.
			Probabilities[i] = 1;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : AddBitsToBuffer
 *
 *  INPUTS        :	BOOL_CODER *bc : Pointer to a bool coder instance.
 * 					UINT32 data    : Data value to be encoder by bc.
 *                  UINT32 bits    : Number of bits of data to be encoded.
 *                  
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Uses the specified Bool Coder to encode the specified
 *                  data value which has the specified number of bits.
 *
 *  SPECIAL NOTES : Fixed probability of 128 (0x80) is used to encode
 *                  each bit in turn. The least-significant bit is 
 *                  encoded first.
 *
 ****************************************************************************/
void AddBitsToBuffer ( BOOL_CODER *bc, UINT32 data, UINT32 bits )
{
	int bit;

    for( bit=bits-1; bit>=0; bit-- )
		VP6_EncodeBool ( bc, (1&(data>>bit)), 0x80 );
}

/****************************************************************************
 * 
 *  ROUTINE       : WriteFrameHeader
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Writes a frame header to the bitstream.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void WriteFrameHeader ( CP_INSTANCE *cpi )
{
    RAW_BUFFER *Buffer = &cpi->RawBuffer;
	BOOL_CODER  *bc  = &cpi->bc;
    PB_INSTANCE *pbi = &cpi->pb;

// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = HEADER_SECTION;
#endif

    // Output the frame type (base/key frame or inter frame)
    AddRawBitsToBuffer( Buffer, (UINT32)pbi->FrameType, 1 );
    
	// Quantizer
	AddRawBitsToBuffer( Buffer, pbi->quantizer->FrameQIndex, 6 );

    // Flag to indicate if we are using two bool coder streams.
	// Note that this flag is ignored by the decoder in SIMPLE_PROFILE
	// where the use of two streams is implicit
	AddRawBitsToBuffer ( Buffer, (UINT32)((pbi->MultiStream || (pbi->VpProfile == SIMPLE_PROFILE)) ? 1 : 0), 1);
  
	// If the frame was a base frame then write out the frame dimensions. 
	if ( pbi->FrameType == BASE_FRAME )
	{
		// Add the version and profile details
		AddRawBitsToBuffer ( Buffer, (UINT32)pbi->Vp3VersionNo, 5 );
		AddRawBitsToBuffer ( Buffer, (UINT32)pbi->VpProfile, 2 );

		// is this keyframe section of the file interlaced
		AddRawBitsToBuffer ( Buffer, (UINT32)(pbi->Configuration.Interlaced), 1);

		// encoded size vertical and horizontal
		AddBitsToBuffer( bc, (UINT32)(pbi->VFragments>>1), 8 );             
		AddBitsToBuffer( bc, (UINT32)(pbi->HFragments>>1), 8 );         
		
		if( ( pbi->Configuration.HScale > 1 || pbi->Configuration.VScale > 1 ) &&
			( cpi->AllowSpatialResampling == 0 && !cpi->ForceInternalSize ))
		{
			// scaled and cropped output size in macroblocks
			AddBitsToBuffer( bc, (UINT32)(cpi->YuvInputData.YHeight * pbi->Configuration.VScale / pbi->Configuration.VRatio >> 4), 8 );         
			AddBitsToBuffer( bc, (UINT32)(cpi->YuvInputData.YWidth * pbi->Configuration.HScale / pbi->Configuration.HRatio >> 4), 8 );             
		}
		else
		{
			// scaled and cropped output size in macroblocks
			AddBitsToBuffer( bc, (UINT32)(cpi->YuvInputData.YHeight >> 4), 8 );         
			AddBitsToBuffer( bc, (UINT32)(cpi->YuvInputData.YWidth >> 4), 8 ); 
		}

		// scaling mode
		AddBitsToBuffer( bc, (UINT32)(pbi->Configuration.ScalingMode), 2);

		// Unless in SIMPLE_PROFILE transmit data to describe the filter 
        // strategy for fractional pels (Applies until next key frame)
		if ( pbi->VpProfile != SIMPLE_PROFILE )
		{
			// Indicate what type of filtering we should use in motion prediction.
			// Applies until next key frame.
			if ( pbi->PredictionFilterMode == AUTO_SELECT_PM )
			{
				AddBitsToBuffer( bc,  (UINT32)1, 1 );
				AddBitsToBuffer( bc,  (UINT32)(pbi->PredictionFilterVarThresh >> ((cpi->pb.Vp3VersionNo  > 7) ? 0 : 5)), 5 );
				AddBitsToBuffer( bc,  (UINT32)(pbi->PredictionFilterMvSizeThresh), 3 );
			}
			else
			{
				AddBitsToBuffer( bc,  (UINT32)0, 1 );
				AddBitsToBuffer( bc,  (UINT32)(pbi->PredictionFilterMode == BICUBIC_ONLY_PM) ? 1 : 0, 1 );
			}

			// If the ENCODER VERSION is > 7 then we add the VP6.2 specific stuff
			if ( cpi->pb.Vp3VersionNo > 7 )
				AddBitsToBuffer( bc,  (UINT32)pbi->PredictionFilterAlpha, 4 );

			cpi->LastPredictionFilterMode = pbi->PredictionFilterMode;
			cpi->LastPredictionFilterVarThresh = pbi->PredictionFilterVarThresh;
			cpi->LastPredictionFilterMvSizeThresh = pbi->PredictionFilterMvSizeThresh;
			cpi->LastPredictionFilterAlpha = pbi->PredictionFilterAlpha;
		}
    }
	// Non key frame specific stuff
	else
	{
		// Flag whether or not the golden frame should be updated this frame
		AddBitsToBuffer( bc, (pbi->RefreshGoldenFrame) ? 1 : 0, 1 );

		// Indicate whether loop filter is to be used. 
		// This flag is ignored if we are in SIMPLE_PROFILE	
		if ( pbi->VpProfile != SIMPLE_PROFILE )
		{
			if ( pbi->UseLoopFilter == NO_LOOP_FILTER )
			{
				AddBitsToBuffer( bc, 0, 1 );
			}
			else if ( pbi->UseLoopFilter == LOOP_FILTER_BASIC )
			{
				AddBitsToBuffer( bc, 1, 1 );
				AddBitsToBuffer( bc, 0, 1 );
			}
			else // LOOP_FILTER_DERING
			{
				AddBitsToBuffer( bc, 1, 1 );
				AddBitsToBuffer( bc, 1, 1 );
			}

			// Should we update prediction modes etc. VP6.2 and later
			if ( cpi->pb.Vp3VersionNo > 7 )
			{
				if ( (pbi->PredictionFilterMode != cpi->LastPredictionFilterMode) ||
					 (pbi->PredictionFilterVarThresh != cpi->LastPredictionFilterVarThresh) ||
					 (pbi->PredictionFilterMvSizeThresh != cpi->LastPredictionFilterMvSizeThresh) ||
					 (pbi->PredictionFilterAlpha != cpi->LastPredictionFilterAlpha) )
				{
					// Idicate a change
					AddBitsToBuffer( bc, 1, 1 );

					// Indicate what type of filtering we should use in motion prediction.
					// Applies until next key frame.
					if ( pbi->PredictionFilterMode == AUTO_SELECT_PM )
					{
						AddBitsToBuffer( bc,  (UINT32)1, 1 );
						AddBitsToBuffer( bc,  (UINT32)pbi->PredictionFilterVarThresh, 5 );
						AddBitsToBuffer( bc,  (UINT32)(pbi->PredictionFilterMvSizeThresh), 3 );
					}
					else
					{
						AddBitsToBuffer( bc,  (UINT32)0, 1 );
						AddBitsToBuffer( bc,  (UINT32)(pbi->PredictionFilterMode == BICUBIC_ONLY_PM) ? 1 : 0, 1 );
					}

					AddBitsToBuffer( bc,  (UINT32)pbi->PredictionFilterAlpha, 4 );

					cpi->LastPredictionFilterMode = pbi->PredictionFilterMode;
					cpi->LastPredictionFilterVarThresh = pbi->PredictionFilterVarThresh;
					cpi->LastPredictionFilterMvSizeThresh = pbi->PredictionFilterMvSizeThresh;
					cpi->LastPredictionFilterAlpha = pbi->PredictionFilterAlpha;
				}
				else
				{
					// No change this frame
					AddBitsToBuffer( bc, 0, 1 );
				}
			}
		}
	}

	// All frames (key frame and inter)
	if ( pbi->UseHuffman )
        AddBitsToBuffer( bc, 1, 1 );
    else
	    AddBitsToBuffer( bc, 0, 1 );
}

/****************************************************************************
 * 
 *  ROUTINE       : VP6AddHuffmanToken
 *
 *  INPUTS        : CP_INSTANCE *cpi       : Pointer to encoder instance.
 *	                TOKENEXTRA *TokenExtra : Token & extrabits to be encoded.
 *                  UINT32 *HuffCode       : Array of Huffman codes for tokens.
 *                  UINT8  *HuffLength     : Array of lengths of each HuffCode entry.
 *                  UINT32 *ZeroCode       : Array of Huffman codes for zero runs.
 *                  UINT8  *ZeroLength     : Array of lengths of each ZeroLength entry.
 *	                UINT8  *CoefIndex      : DCT coeff position token occurs at.
 * 
 *  OUTPUTS       : None
 * 
 *  RETURNS       : void
 *
 *  FUNCTION      : Adds a single token any any associated extra-bits
 *                  to the bitstream using Huffman tokens.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
INLINE 
void VP6AddHuffmanToken
(
	CP_INSTANCE *cpi,
	TOKENEXTRA *TokenExtra,
    UINT32 *HuffCode,
    UINT8  *HuffLength,
    UINT32 *ZeroCode,
    UINT8  *ZeroLength,
	UINT8 *CoefIndex
)
{
    INT32 Token = TokenExtra->Token;
    UINT32 Extra = TokenExtra->Extra;
    PB_INSTANCE *pbi = &cpi->pb;

    // Output Huffman code for zero run length
    if ( Token == ZERO_TOKEN )
    {
        if ( *CoefIndex > 0 ) 
        {
            // Output Huffman code for Token
            AddRawBitsToBuffer ( &pbi->HuffBuffer, HuffCode[Token], HuffLength[Token] );

            if ( Extra >= 8 )
            {
                // Zero run greater than 8 coded with extra bits greater than 8
                AddRawBitsToBuffer ( &pbi->HuffBuffer, ZeroCode[8], ZeroLength[8] );

                // Zero run of 8 or more coded with fixed 6-bits
                AddRawBitsToBuffer ( &pbi->HuffBuffer, Extra-8, 6 );
            }
            else
            {
                // Zero run less than 8 coded with Huffman code
                AddRawBitsToBuffer ( &pbi->HuffBuffer, ZeroCode[Extra], ZeroLength[Extra] );
            }

            // Step the coefindex on by run length - 1 for AC zero runs
		    // Note that TokenExtra->Extra = run length - 1
            *CoefIndex += Extra;
        }
        else if ( Extra > 0 )
        {
            // Zero at DC
            UINT32 DcZrlBand = DcZrlHuffBand[Extra];

            // Zero token
            AddRawBitsToBuffer ( &pbi->HuffBuffer, HuffCode[Token], HuffLength[Token] );

            // Run length token & extra bits
            AddRawBitsToBuffer ( &pbi->HuffBuffer, DcZrlHuffCode[DcZrlBand], DcZrlHuffLength[DcZrlBand] );
            if ( DcZrlExtraLength[DcZrlBand] )
                AddRawBitsToBuffer ( &pbi->HuffBuffer, Extra-DcZrlExtraOffset[DcZrlBand], DcZrlExtraLength[DcZrlBand] );
        }
    }
    else if ( Token == DCT_EOB_TOKEN )
    {
        if ( *CoefIndex > 1 )
        {
            // EOB token beyond first AC in scan
            AddRawBitsToBuffer ( &pbi->HuffBuffer, HuffCode[Token], HuffLength[Token] );
        }
        else if ( Extra > 0 )
        {
            // Temp use same codes for EOB runs as for DC zero runs        
            UINT32 DcZrlBand = DcZrlHuffBand[Extra];

            // EOB token at first AC in scan
            AddRawBitsToBuffer ( &pbi->HuffBuffer, HuffCode[Token], HuffLength[Token] );

            // Run length token & extra bits
            AddRawBitsToBuffer ( &pbi->HuffBuffer, DcZrlHuffCode[DcZrlBand], DcZrlHuffLength[DcZrlBand] );
            if ( DcZrlExtraLength[DcZrlBand] )
                AddRawBitsToBuffer ( &pbi->HuffBuffer, Extra-DcZrlExtraOffset[DcZrlBand], DcZrlExtraLength[DcZrlBand] );
        }
    }
    else 
    {
        // Output Huffman code for Token
        AddRawBitsToBuffer ( &pbi->HuffBuffer, HuffCode[Token], HuffLength[Token] );

        // Output Extra bits
	    if ( ExtraBitLengths_VP6[Token] )
            AddRawBitsToBuffer ( &pbi->HuffBuffer, Extra, ExtraBitLengths_VP6[Token] );
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : VP6AddToken
 *
 *  INPUTS        : CP_INSTANCE *cpi        : Pointer to encoder instance (NOT USED).
 *                  BOOL_CODER *bc          : Pointer to Bool Coder to be used.
 *	                TOKENEXTRA *TokenExtra  : Token & extrabits to be encoded.
 *	                UINT8 *BaselineProbsPtr : 
 *	                UINT8 *ContextProbsPtr  : Array of tree node probs
 *	                UINT8 *ZeroRunProbsPtr  : Array of probs for aero run lengths.
 *	                UINT8 *CoefIndex        : DCT coeff position token occurs at.
 *	                BOOL  NonZeroImplicit   : Flag indicating whether a zero token
 *                                            is prohibited due to context.
 *
 *  OUTPUTS       : None
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Adds a single token any any associated extra-bits
 *                  to the bitstream using a Bool Coder.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void VP6AddToken
(
	CP_INSTANCE *cpi,
    BOOL_CODER *bc,
	TOKENEXTRA *TokenExtra,
	UINT8 *BaselineProbsPtr,
	UINT8 *ContextProbsPtr,
	UINT8 *ZeroRunProbsPtr,
	UINT8 *CoefIndex,
	BOOL  NonZeroImplicit
)
{
	// Case statement to output code patterns for the token.
	switch ( TokenExtra->Token )
	{
	case DCT_EOB_TOKEN:		// 00
		VP6_EncodeBool ( bc, 0, ContextProbsPtr[ZERO_CONTEXT_NODE] );	// Zero value branch
		VP6_EncodeBool ( bc, 0, ContextProbsPtr[EOB_CONTEXT_NODE ] );	// EOB vs 0 branch
		break;

	case ZERO_TOKEN:	 	// 01
		VP6_EncodeBool ( bc, 0, ContextProbsPtr[ZERO_CONTEXT_NODE] );	// Zero value branch

		// For DC there is no run length and EOB is not allowed
		if ( *CoefIndex > 0 )
		{
			VP6_EncodeBool ( bc, 1, ContextProbsPtr[EOB_CONTEXT_NODE] );// EOB vs 0 branch

			// Step the coefindex on by run length - 1 
			// Note that TokenExtra->Extra = run length - 1
			*CoefIndex += TokenExtra->Extra;

			// Now code the zero run length
			if ( TokenExtra->Extra < 8 )								// run lengths 2, 3, 4
			{
				switch ( TokenExtra->Extra )
				{
				case 0:
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[0] );
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[1] );
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[2] );
					break;
				case 1:
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[0] );
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[1] );
					VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[2] );
					break;
				case 2:
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[0] );
					VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[1] );
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[3] );
					break;
				case 3:
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[0] );
					VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[1] );
					VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[3] );
					break;
				case 4:
					VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[0] );
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[4] );
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[5] );
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[6] );
					break;
				case 5:
					VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[0] );
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[4] );
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[5] );
					VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[6] );
					break;
				case 6:
					VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[0] );
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[4] );
					VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[5] );
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[7] );
					break;
				case 7:
					VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[0] );
					VP6_EncodeBool ( bc, 0, ZeroRunProbsPtr[4] );
					VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[5] );
					VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[7] );
					break;
				}
			}
			else
			{
				TokenExtra->Extra -= 8;

				// Run length > 8
				VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[0] );	
				VP6_EncodeBool ( bc, 1, ZeroRunProbsPtr[4] );	

				// Code run length -8
				VP6_EncodeBool ( bc, (1&TokenExtra->Extra), ZeroRunProbsPtr[8] );			    // Bit 0
				VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>1)), ZeroRunProbsPtr[9] );		// Bit 1
				VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>2)), ZeroRunProbsPtr[10] );		// Bit 2
				VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>3)), ZeroRunProbsPtr[11] );		// Bit 3
				VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>4)), ZeroRunProbsPtr[12] );		// Bit 4
				VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>5)), ZeroRunProbsPtr[13] );		// Bit 5
			}
		}
		break;

	case ONE_TOKEN:			// 10 X	
		if ( !NonZeroImplicit )
			VP6_EncodeBool ( bc, 1, ContextProbsPtr[ZERO_CONTEXT_NODE] ); // Zero value branch
		VP6_EncodeBool ( bc, 0, ContextProbsPtr[ONE_CONTEXT_NODE] );	  // One Branch
	
		VP6_EncodeBool ( bc, TokenExtra->Extra, 128 );					  // Sign
		break;

	case TWO_TOKEN:			// 1100 X				
		if ( !NonZeroImplicit )
			VP6_EncodeBool ( bc, 1, ContextProbsPtr[ZERO_CONTEXT_NODE] ); // Zero value branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[ONE_CONTEXT_NODE] );	  // One Branch

		VP6_EncodeBool ( bc, 0, ContextProbsPtr[LOW_VAL_CONTEXT_NODE] );  // Low Value Branch
		VP6_EncodeBool ( bc, 0, ContextProbsPtr[TWO_CONTEXT_NODE] );	  // 2 Branch
	
		VP6_EncodeBool ( bc, TokenExtra->Extra, 128);					  // Sign
		break;

	case THREE_TOKEN:			// 11010 X			
		if ( !NonZeroImplicit )
			VP6_EncodeBool ( bc, 1, ContextProbsPtr[ZERO_CONTEXT_NODE] ); // Zero value branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[ONE_CONTEXT_NODE] );	  // One Branch
		VP6_EncodeBool ( bc, 0, ContextProbsPtr[LOW_VAL_CONTEXT_NODE] );  // Low Value Branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[TWO_CONTEXT_NODE] );	  // 2 Branch
		VP6_EncodeBool ( bc, 0, BaselineProbsPtr[THREE_CONTEXT_NODE] );	  // Three Branch

		VP6_EncodeBool ( bc, TokenExtra->Extra, 128 );					  // Sign
		break;

	case FOUR_TOKEN:			// 11011 X			
		if ( !NonZeroImplicit )
			VP6_EncodeBool ( bc, 1, ContextProbsPtr[ZERO_CONTEXT_NODE] ); // Zero value branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[ONE_CONTEXT_NODE] );	  // One Branch
		VP6_EncodeBool ( bc, 0, ContextProbsPtr[LOW_VAL_CONTEXT_NODE] );  // Low Value Branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[TWO_CONTEXT_NODE] );	  // 2 Branch
		VP6_EncodeBool ( bc, 1, BaselineProbsPtr[THREE_CONTEXT_NODE] );	  // Three Branch

		VP6_EncodeBool ( bc, TokenExtra->Extra, 128 );					  // Sign
		break;

	case DCT_VAL_CATEGORY1:		// 11100 XX
		if ( !NonZeroImplicit )
			VP6_EncodeBool ( bc, 1, ContextProbsPtr[ZERO_CONTEXT_NODE] ); // Zero value branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[ONE_CONTEXT_NODE] );	  // One Branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[LOW_VAL_CONTEXT_NODE] );  // Low Value Branch

		VP6_EncodeBool ( bc, 0, BaselineProbsPtr[HIGH_LOW_CONTEXT_NODE] );// HighLow Value Branch
		VP6_EncodeBool ( bc, 0, BaselineProbsPtr[CAT_ONE_CONTEXT_NODE] ); // Cat1 Value Branch

		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>1)), 159 );			  // Data Bit
		VP6_EncodeBool ( bc, (1&TokenExtra->Extra), 128 );				  // Sign Bit
		break;

	case DCT_VAL_CATEGORY2:		// 11101	XXX	
		if ( !NonZeroImplicit )
			VP6_EncodeBool ( bc, 1, ContextProbsPtr[ZERO_CONTEXT_NODE] ); // Zero value branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[ONE_CONTEXT_NODE] );	  // One Branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[LOW_VAL_CONTEXT_NODE] );  // Low Value Branch 

		VP6_EncodeBool ( bc, 0, BaselineProbsPtr[HIGH_LOW_CONTEXT_NODE] );// HighLow Value Branch
		VP6_EncodeBool ( bc, 1, BaselineProbsPtr[CAT_ONE_CONTEXT_NODE] ); // Cat1 Value Branch

		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>2)), 165 );			  // Data Bits
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>1)), 145 );
		VP6_EncodeBool ( bc, (1&TokenExtra->Extra), 128 );				  // Sign Bit
		break;

	case DCT_VAL_CATEGORY3:		// 111 100	XXXXX
		if ( !NonZeroImplicit )
			VP6_EncodeBool ( bc, 1, ContextProbsPtr[ZERO_CONTEXT_NODE] );		// Zero value branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[ONE_CONTEXT_NODE] );			// One Branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[LOW_VAL_CONTEXT_NODE] );		// Low Value Branch

		VP6_EncodeBool ( bc, 1, BaselineProbsPtr[HIGH_LOW_CONTEXT_NODE] );		// HighLow Value Branch
		VP6_EncodeBool ( bc, 0, BaselineProbsPtr[CAT_THREEFOUR_CONTEXT_NODE] );	// Cat3/4 Value Branch
		VP6_EncodeBool ( bc, 0, BaselineProbsPtr[CAT_THREE_CONTEXT_NODE] );		// Cat3 Value Branch

		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>3)), 173 );					// Data Bits
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>2)), 148 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>1)), 140 );
		VP6_EncodeBool ( bc, (1&TokenExtra->Extra), 128 );						// Sign Bit
		break;

	case DCT_VAL_CATEGORY4:		// 111101	XXXXX
		if ( !NonZeroImplicit )
			VP6_EncodeBool ( bc, 1, ContextProbsPtr[ZERO_CONTEXT_NODE] );		// Zero value branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[ONE_CONTEXT_NODE] );			// One Branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[LOW_VAL_CONTEXT_NODE] );		// Low Value Branch

		VP6_EncodeBool ( bc, 1, BaselineProbsPtr[HIGH_LOW_CONTEXT_NODE] );		// HighLow Value Branch
		VP6_EncodeBool ( bc, 0, BaselineProbsPtr[CAT_THREEFOUR_CONTEXT_NODE] );	// Cat3/4 Value Branch
		VP6_EncodeBool ( bc, 1, BaselineProbsPtr[CAT_THREE_CONTEXT_NODE] );		// Cat3 Value Branch

		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>4)), 176 );					// More significant bits more likely to be 0
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>3)), 155 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>2)), 140 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>1)), 135 );
		VP6_EncodeBool ( bc, (1&TokenExtra->Extra), 128 );						// Sign Bit
		break;

	case DCT_VAL_CATEGORY5:		// 111110	XXXXXX		
		if ( !NonZeroImplicit )
			VP6_EncodeBool ( bc, 1, ContextProbsPtr[ZERO_CONTEXT_NODE] );		// Zero value branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[ONE_CONTEXT_NODE] );			// One Branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[LOW_VAL_CONTEXT_NODE] );		// Low Value Branch

		VP6_EncodeBool ( bc, 1, BaselineProbsPtr[HIGH_LOW_CONTEXT_NODE] );		// HighLow Value Branch
		VP6_EncodeBool ( bc, 1, BaselineProbsPtr[CAT_THREEFOUR_CONTEXT_NODE] );	// Cat3/4 Value Branch
		VP6_EncodeBool ( bc, 0, BaselineProbsPtr[CAT_FIVE_CONTEXT_NODE] );		// Cat5/6 Value Branch

		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>5)), 180 );					// Data Bits
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>4)), 157 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>3)), 141 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>2)), 134 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>1)), 130 );
		VP6_EncodeBool ( bc, (1&TokenExtra->Extra), 128);						// Sign Bit
		break;

	case DCT_VAL_CATEGORY6:		// 111111 XXXXXXXXXXXX	
		if ( !NonZeroImplicit )
			VP6_EncodeBool ( bc, 1, ContextProbsPtr[ZERO_CONTEXT_NODE] );		// Zero value branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[ONE_CONTEXT_NODE] );			// One Branch
		VP6_EncodeBool ( bc, 1, ContextProbsPtr[LOW_VAL_CONTEXT_NODE] );		// Low Value Branch

		VP6_EncodeBool ( bc, 1, BaselineProbsPtr[HIGH_LOW_CONTEXT_NODE] );		// HighLow Value Branch
		VP6_EncodeBool ( bc, 1, BaselineProbsPtr[CAT_THREEFOUR_CONTEXT_NODE] );	// Cat3/4 Value Branch
		VP6_EncodeBool ( bc, 1, BaselineProbsPtr[CAT_FIVE_CONTEXT_NODE] );		// Cat5/6 Value Branch

		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>11)), 254 );				// Data Bits
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>10)), 254 );		
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>9)), 243 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>8)), 230 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>7)), 196 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>6)), 177 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>5)), 153 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>4)), 140 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>3)), 133 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>2)), 130 );
		VP6_EncodeBool ( bc, (1&(TokenExtra->Extra>>1)), 129 );
		VP6_EncodeBool ( bc, (1&TokenExtra->Extra), 128 );						// Sign Bit
		break;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : UpdateContextProbs
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Determines which context probabilities to update and 
 *                  encodes the changes to the bitstream.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void UpdateContextProbs ( CP_INSTANCE *cpi )
{
	UINT32	i,j;
	UINT32	Plane;
	UINT32	Band;
	INT32   Prec;
	INT32   OldBits;
	INT32   NewBits;
	UINT8   PrecNonZero;
	INT32   ProbUpdateCost;
	UINT8   Probs[MAX_ENTROPY_TOKENS-1];
	UINT8   LastProb[MAX_ENTROPY_TOKENS-1];
	UINT32  BranchChoices[MAX_ENTROPY_TOKENS-1][2];
	
	BOOL_CODER  *bc  = &cpi->bc;
    PB_INSTANCE *pbi = &cpi->pb;
	UINT8 FrameType = VP6_GetFrameType ( pbi );

	// Clear down last prob structure
	memset ( LastProb, 128, MAX_ENTROPY_TOKENS-1 );

	// Baseline DC probabilities for Y and then UV Planes.
	for ( Plane=0; Plane<2; Plane++ )
	{
		if ( FrameType == BASE_FRAME ) 
			memcpy ( pbi->DcProbs+DCProbOffset(Plane,0), LastProb, MAX_ENTROPY_TOKENS-1 ); 

	    ConvertDistribution ( cpi, cpi->FrameDcTokenDist[Plane], Probs, BranchChoices );

		// Are there any updates for this set.
		for ( i=0; i<MAX_ENTROPY_TOKENS-1; i++ )
		{
			OldBits = ((BranchChoices[i][0] * VP6_ProbCost[pbi->DcProbs[DCProbOffset(Plane,i)]])/256) +
					  ((BranchChoices[i][1] * VP6_ProbCost[255 - pbi->DcProbs[DCProbOffset(Plane,i)]])/256);
			NewBits = ((BranchChoices[i][0] * VP6_ProbCost[Probs[i]])/256) +
					  ((BranchChoices[i][1] * VP6_ProbCost[255 - Probs[i]])/256);

			ProbUpdateCost = PROB_UPDATE_BASELINE_COST + PROB_UPDATE_CORECTION + (VP6_ProbCost[255 - VP6_DcUpdateProbs[Plane][i]]/256);

            if ( (OldBits - NewBits) > ProbUpdateCost )
			{
				// Probabilities sent
				VP6_EncodeBool ( bc, 1, VP6_DcUpdateProbs[Plane][i] );

				AddBitsToBuffer ( bc, Probs[i] >> 1, PROB_UPDATE_BASELINE_COST );

				// Update the last probability records
				pbi->DcProbs[DCProbOffset(Plane,i)] = Probs[i];
				LastProb[i] = Probs[i];
			}
			else
			{
				// Probabilities not sent
				VP6_EncodeBool ( bc, 0, VP6_DcUpdateProbs[Plane][i] );
			}
		}
	}

	// If we are in Error resilliant mode and this was the first frame then take a copy of the 
	// entropy probabilities used for re-use on subsequent key frames.
	if ( (cpi->ErrorResilliantMode) && (cpi->CurrentFrame == 1) )
	{
		memcpy( cpi->FirstFrameDcProbs, pbi->DcProbs, sizeof(cpi->FirstFrameDcProbs) );
	}

	// Are we supporting dynamic scan order updates
	if ( ( (pbi->Configuration.Interlaced) || (cpi->AllowScanOrderUpdates) ) &&
		 ( !cpi->ErrorResilliantMode ) )
	{
		VP6_EncodeBool ( bc, 1, 128 );

		// Transmit changes to the AC scan order banding
		for ( i=1; i<BLOCK_SIZE; i++ )
		{
			// Should we update the ceoffs band
			if ( cpi->NewScanOrderBands[i] != pbi->ScanBands[i] )
			{
				VP6_EncodeBool ( bc, 1, ScanBandUpdateProbs[i] );
				AddBitsToBuffer ( bc, cpi->NewScanOrderBands[i], SCAN_BAND_UPDATE_BITS );
				pbi->ScanBands[i] = cpi->NewScanOrderBands[i];
				scanupdates[i][1]++;
			}
			else
			{
				VP6_EncodeBool ( bc, 0, ScanBandUpdateProbs[i] );
				scanupdates[i][0]++;
			}
		}
	}
	else
	{
		VP6_EncodeBool ( bc, 0, 128 );
	}

	// Reset Zero run probabilities to defaults values for key frames
	if ( FrameType == BASE_FRAME )
	{
		memcpy ( pbi->ZeroRunProbs, ZeroRunProbDefaults, sizeof(pbi->ZeroRunProbs) );
	}

	// Update the Zero Run probabilities
	memcpy ( cpi->FrameZrlProbs, pbi->ZeroRunProbs, sizeof(cpi->FrameZrlProbs) );
	if ( !cpi->ErrorResilliantMode )
		GetOptimalFrameZrlProbs( cpi );

	// Transmit any changes needed
	for ( i=0; i<ZRL_BANDS; i++ )
	{
		for ( j=0; j<ZERO_RUN_PROB_CASES; j++ )
		{
			// Work out if saving enough to justify update TBD,
			OldBits = ((cpi->FrameZrlBranchHits[i][j][0] * VP6_ProbCost[pbi->ZeroRunProbs[i][j]])/256) +
					  ((cpi->FrameZrlBranchHits[i][j][1] * VP6_ProbCost[255 - pbi->ZeroRunProbs[i][j]])/256);
			NewBits = ((cpi->FrameZrlBranchHits[i][j][0] * VP6_ProbCost[cpi->FrameZrlProbs[i][j]])/256) +
					  ((cpi->FrameZrlBranchHits[i][j][1] * VP6_ProbCost[255 - cpi->FrameZrlProbs[i][j]])/256);

			ProbUpdateCost = PROB_UPDATE_BASELINE_COST + (VP6_ProbCost[255 - ZrlUpdateProbs[i][j]]/256);

			if ( (OldBits - NewBits) > ProbUpdateCost )
			{
				// Probabilities sent
				VP6_EncodeBool ( bc, 1, ZrlUpdateProbs[i][j] );
				AddBitsToBuffer( bc, cpi->FrameZrlProbs[i][j] >> 1, PROB_UPDATE_BASELINE_COST );
				pbi->ZeroRunProbs[i][j] = (cpi->FrameZrlProbs[i][j] & ~1);
				pbi->ZeroRunProbs[i][j] += (pbi->ZeroRunProbs[i][j] == 0) ? 1 : 0;
			}
			else 
			{
				// Probability not sent
				VP6_EncodeBool ( bc, 0, ZrlUpdateProbs[i][j] );
			}
		}		
	}

	// Baseline probabilities for each AC band.
	// Prec=0 means last token in current block was 0: Prec=1 means it was !0
	for ( Prec=0; Prec<PREC_CASES; Prec++ )
	{
		PrecNonZero = (Prec > 0) ? 1 : 0;

		// Baseline probabilities for each AC band.
		for ( Plane=0; Plane<2; Plane++ )
		{
			for ( Band=0; Band<VP6_AC_BANDS; Band++ )
			{
				// Decide whether to transmit probability data based upon number of tokens represented
				ConvertDistribution ( cpi, cpi->FrameAcTokenDist[Prec][Plane][Band], Probs, BranchChoices );

				if ( FrameType == BASE_FRAME )
					memcpy( pbi->AcProbs+ACProbOffset(Plane,Prec,Band,0), LastProb, MAX_ENTROPY_TOKENS-1 ); 

				for ( i=0; i<MAX_ENTROPY_TOKENS-1; i++ )
				{
					OldBits = ((BranchChoices[i][0] * VP6_ProbCost[pbi->AcProbs[ACProbOffset(Plane,Prec,Band,i)]])/256) +
							  ((BranchChoices[i][1] * VP6_ProbCost[255 - pbi->AcProbs[ACProbOffset(Plane,Prec,Band,i)]])/256);
					NewBits = ((BranchChoices[i][0] * VP6_ProbCost[Probs[i]])/256) +
							  ((BranchChoices[i][1] * VP6_ProbCost[255 - Probs[i]])/256);

					ProbUpdateCost = PROB_UPDATE_BASELINE_COST + PROB_UPDATE_CORECTION + (VP6_ProbCost[255 - VP6_AcUpdateProbs[Prec][Plane][Band][i]]/256);

                    if ( (OldBits - NewBits) > ProbUpdateCost )
					{
						// Probabilities sent
						VP6_EncodeBool ( bc, 1, VP6_AcUpdateProbs[Prec][Plane][Band][i] );
						AddBitsToBuffer ( bc, Probs[i] >> 1, PROB_UPDATE_BASELINE_COST );

						// Update the last probability records
						pbi->AcProbs[ACProbOffset(Plane,Prec,Band,i)] = Probs[i];
						LastProb[i] = Probs[i];
					}
					else
					{
						// Probabilities not sent
						VP6_EncodeBool ( bc, 0, VP6_AcUpdateProbs[Prec][Plane][Band][i] );
					}
				}
			}
		}
	}

	// If we are in Error resilliant mode and this was the first frame then take a copy of the 
	// entropy probabilities used for re-use on subsequent key frames.
	if ( cpi->ErrorResilliantMode && (cpi->CurrentFrame == 1) )
		memcpy ( cpi->FirstFrameAcProbs, pbi->AcProbs, sizeof(cpi->FirstFrameAcProbs) );
}

/****************************************************************************
 * 
 *  ROUTINE       : UpdateContextProbs2
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Sends a selected set of updated context info to the bitstream.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void UpdateContextProbs2 ( CP_INSTANCE *cpi )
{
	UINT32 i,j;
	UINT32 Plane;
	UINT32 Band;
	INT32  Prec;
	UINT8  PrecNonZero;
	UINT8  Probs[MAX_ENTROPY_TOKENS-1];
	UINT32 BranchChoices[MAX_ENTROPY_TOKENS-1][2];

	BOOL_CODER  *bc  = &cpi->bc;
	PB_INSTANCE *pbi = &cpi->pb;
	UINT8  ActiveDcNodes[2]    = { 3, 2 };		                  // Y, UV
	UINT8  ActiveAcNodes[3][2] = { { 1, 1 },{ 2, 0 },{ 1, 0 } };  // {Y, UV} for each prec case
	UINT8  ActiveAcBands[3][2] = { { 1, 1 },{ 1, 0 },{ 1, 0 } };  // {Y, UV} for each prec case

	// Baseline DC probabilities for Y and then UV Planes.
	for ( Plane=0; Plane<2; Plane++ )
	{
	    ConvertDistribution ( cpi, cpi->FrameDcTokenDist[Plane], Probs, BranchChoices );

		// Some nodes are always updated
		// The rest are never updated but are left at the key frame values
		for ( i=0; i<ActiveDcNodes[Plane]; i++ )
		{
			// Probabilities sent
			VP6_EncodeBool ( bc, 1, VP6_DcUpdateProbs[Plane][i] );
			AddBitsToBuffer ( bc, Probs[i] >> 1, PROB_UPDATE_BASELINE_COST );

			// Update the last probability records
			pbi->DcProbs[DCProbOffset(Plane,i)] = Probs[i];
		}
		for ( i=ActiveDcNodes[Plane]; i<MAX_ENTROPY_TOKENS-1; i++ )
		{
			// Probabilities not sent
			VP6_EncodeBool ( bc, 0, VP6_DcUpdateProbs[Plane][i] );
		}
	}

	// Do not change the scan order banding in error resilient mode
    VP6_EncodeBool ( bc, 0, 128 );

	// For now do not update ZRL probabilities in error resilient mode
	for ( i=0; i<2; i++ )
	{
		for ( j=0; j<ZERO_RUN_PROB_CASES; j++ )
		{
			// Probability not sent
			VP6_EncodeBool ( bc, 0, ZrlUpdateProbs[i][j] );
		}		
	}

	// Baseline probabilities for each AC band.
	// Prec=0 means last token in current block was 0: Prec=1 means it was !0
	for ( Prec=0; Prec<PREC_CASES; Prec++ )
	{
		PrecNonZero = (Prec > 0) ? 1 : 0;

		// Baseline probabilities for each AC band.
		for ( Plane=0; Plane<2; Plane++ )
		{
			// For the first couple of AC bands we always update the first few probabilities.
			// For the higher AC bands we never update probabilities
			for ( Band=0; Band<ActiveAcBands[Prec][Plane]; Band++ )
			{
				// Decide whether to transmit probability data based upon number of tokens represented
				ConvertDistribution ( cpi, cpi->FrameAcTokenDist[Prec][Plane][Band], Probs, BranchChoices );

				for ( i=0; i<ActiveAcNodes[Prec][Plane]; i++ )
				{
					// Probabilities sent
					VP6_EncodeBool ( bc, 1, VP6_AcUpdateProbs[Prec][Plane][Band][i] );
					AddBitsToBuffer ( bc, Probs[i] >> 1, PROB_UPDATE_BASELINE_COST );

					// Update the last probability records
					pbi->AcProbs[ACProbOffset(Plane,Prec,Band,i)] = Probs[i];
				}

				for ( i=ActiveAcNodes[Prec][Plane]; i<MAX_ENTROPY_TOKENS-1; i++ )
				{
					// Probabilities not sent
					VP6_EncodeBool ( bc, 0, VP6_AcUpdateProbs[Prec][Plane][Band][i] );
				}
			}
			
            for ( Band=ActiveAcBands[Prec][Plane]; Band<VP6_AC_BANDS; Band++ )
			{
				for ( i=0; i<MAX_ENTROPY_TOKENS-1; i++ )
				{
					// Probabilities not sent
					VP6_EncodeBool ( bc, 0, VP6_AcUpdateProbs[Prec][Plane][Band][i] );
				}
			}
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : UpdateContextProbs3
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Updates all baseline probabilities (except first frame
 *					when in error resilliant mode).
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void UpdateContextProbs3(CP_INSTANCE *cpi)
{
	UINT32	i,j;
	UINT32	Plane;
	UINT32	Band;
	INT32   Prec;
	UINT8	Prob;
	UINT8   LastProb[MAX_ENTROPY_TOKENS-1];

	BOOL_CODER  *bc  = &cpi->bc;
    PB_INSTANCE *pbi = &cpi->pb;

	// Clear down last prob structure
	memset ( LastProb, 128, MAX_ENTROPY_TOKENS-1 );

	// Copy over the DC probabilities used for the first frame
	memcpy ( pbi->DcProbs, cpi->FirstFrameDcProbs, sizeof(pbi->DcProbs) );

	// Baseline DC probabilities for Y and then UV Planes.
	for ( Plane=0; Plane<2; Plane++ )
	{
		// Are there any updates for this set.
		for ( i=0; i<MAX_ENTROPY_TOKENS-1; i++ )
		{
			Prob = pbi->DcProbs[DCProbOffset(Plane,i)];
			if ( Prob != LastProb [i] )
			{
				// Send probabilities
				VP6_EncodeBool ( bc, 1, VP6_DcUpdateProbs[Plane][i] );
				AddBitsToBuffer ( bc, Prob >> 1, PROB_UPDATE_BASELINE_COST );

				LastProb[i] = Prob;
			}
			else
			{
				// Probabilities not sent
				VP6_EncodeBool ( bc, 0, VP6_DcUpdateProbs[Plane][i] );
			}
		}
	}

	// Reset Zero run probabilities to defaults values for key frames
	memcpy ( pbi->ZeroRunProbs, ZeroRunProbDefaults, sizeof(pbi->ZeroRunProbs) );

	// Do not change the scan order banding in error resilient mode
	VP6_EncodeBool ( bc, 0, 128 );

	// For now do not update ZRL probabilities in error resilient mode
	for ( i=0; i<2; i++ )
	{
		for ( j=0; j<ZERO_RUN_PROB_CASES; j++ )
		{
			// Probability not sent
			VP6_EncodeBool ( bc, 0, ZrlUpdateProbs[i][j] );
		}		
	}

	// Copy over the AC probabilities used for the first frame
	memcpy ( pbi->AcProbs, cpi->FirstFrameAcProbs, sizeof(pbi->AcProbs) );

	// Baseline probabilities for each AC band.
	// Prec=0 means last token in current block was 0: Prec=1 means it was !0
	for ( Prec=0; Prec<PREC_CASES; Prec++ )
	{
		// Baseline probabilities for each AC band.
		for ( Plane=0; Plane<2; Plane++ )
		{
			for ( Band=0; Band<VP6_AC_BANDS; Band++ )
			{
				for ( i=0; i<MAX_ENTROPY_TOKENS-1; i++ )
				{
					Prob = pbi->AcProbs[ACProbOffset(Plane,Prec,Band,i)];
					if ( Prob != LastProb [i] )
					{
						// Probabilities sent
						VP6_EncodeBool ( bc, 1, VP6_AcUpdateProbs[Prec][Plane][Band][i] );
						AddBitsToBuffer ( bc, Prob >> 1, PROB_UPDATE_BASELINE_COST );

						LastProb [i] = Prob;
					}
					else
					{
						// Probabilities not sent
						VP6_EncodeBool ( bc, 0, VP6_AcUpdateProbs[Prec][Plane][Band][i] );
					}
				}
			}
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : PackHuffmanCoeffs
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Outputs the list of tokens generated for the frame
 *                  using Huffman coding.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void PackHuffmanCoeffs ( CP_INSTANCE *cpi )
{
	UINT32	Plane;
	UINT8 PrecTokenIndex;
	TOKENEXTRA *j;
	TOKENEXTRA *First;
	TOKENEXTRA *Last;
	
    BOOL_CODER  *bc  = &cpi->bc;
	PB_INSTANCE *pbi = &cpi->pb;
    UINT8 FrameType = VP6_GetFrameType ( pbi );

// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = HEADER_SECTION;
#endif

	// Work out which context probabilities need to be updated
	// and output the changes to the bitstream.
	//
	// Error resilliant mode uses a fixed probability update pattern to make the entropy 
	// code more resilliant to dropped frames
	if ( cpi->ErrorResilliantMode )
	{
		// In "error resilliant / VC" mode use an update mechanism that is more tolerant of dropped frames.
		if ( FrameType == BASE_FRAME )
		{
			if ( cpi->CurrentFrame == 1 )
				UpdateContextProbs( cpi );
			else
				UpdateContextProbs3( cpi );
		}
		else
			UpdateContextProbs2( cpi );
	}
	else
	{
		UpdateContextProbs( cpi );
	}

	// Create all the context specific propabilities
	VP6_ConfigureContexts ( pbi );

	// probability that the macroblock is interlaced
	if(pbi->Configuration.Interlaced)
		AddBitsToBuffer ( bc, (UINT32)(pbi->probInterlaced), 8 );

    // Create Huffman codes for tokens based on tree probabilities
    ConvertBoolTrees ( pbi );

	// encode coefficients 
	First=cpi->CoeffTokens;
	Last=cpi->CoeffTokenPtr;
	{
		UINT8 coef;
		UINT32 now;
		unsigned int MBrow, MBcol, block;

        j = First;
		now = bc->pos * 8 - 4;
		for ( MBrow=BORDER_MBS; MBrow<pbi->MBRows - BORDER_MBS; MBrow++ )
		{
			for ( MBcol=BORDER_MBS; MBcol<pbi->MBCols-BORDER_MBS; MBcol++ )
			{
				// dumb way to encode the interlaced decision but it works!!!
				{
					UINT8 prob = pbi->probInterlaced;

					// super simple context adjustment
					if(MBcol>BORDER_MBS)
					{
						if(pbi->MBInterlaced[MBOffset(MBrow,MBcol-1)])
							prob = prob - (prob>>1);
						else 
							prob = prob + ((256-prob)>>1);
					}
					
// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = HEADER_SECTION;
#endif

					if ( pbi->Configuration.Interlaced )
						VP6_EncodeBool ( bc, pbi->MBInterlaced[MBOffset(MBrow,MBcol)], prob );
				}

				if ( pbi->FrameType != BASE_FRAME )
					encodeModeAndMotionVector ( cpi, MBrow, MBcol );

				for ( block=0 ; block<6 ; block++ )
				{
					Plane = block>3;

// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = DC_SECTION;
#endif

					// DC Token
					coef = 0;
                    VP6AddHuffmanToken ( cpi, j, 
                        pbi->DcHuffCode[Plane], 
                        pbi->DcHuffLength[Plane],  
                        pbi->ZeroHuffCode[0], 
                        pbi->ZeroHuffLength[0], 
                        &coef );

					PrecTokenIndex = VP6_PrevTokenIndex[j->Token];
					j++;

                    for ( coef=1; coef<64; coef++ )
                    {
                        UINT32 ZrlBand = (coef >= ZRL_BAND2) ? 1 : 0;
                        
                        // Restrict to 4 AC bands when using Huffman                        
                        UINT32 AcBand = VP6_CoeffToBand[coef];
                        AcBand = (AcBand < 4) ? AcBand : 3;

// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = AC_SECTION;
#endif

                        VP6AddHuffmanToken ( cpi, j, 
                            pbi->AcHuffCode[PrecTokenIndex][Plane][AcBand], 
                            pbi->AcHuffLength[PrecTokenIndex][Plane][AcBand],  
                            pbi->ZeroHuffCode[ZrlBand], 
                            pbi->ZeroHuffLength[ZrlBand], &coef );
                        
                        PrecTokenIndex = VP6_PrevTokenIndex[j->Token];
                        
                        if( j->Token == DCT_EOB_TOKEN )
                            coef=64;
                        j++;
                    } 
				}
			}
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : PackArithmeticCoeffs
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Outputs the list of tokens generated for the frame
 *                  using a Bool Coder.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void PackArithmeticCoeffs ( CP_INSTANCE *cpi )
{
	UINT32	Plane;
	UINT8   PrecTokenIndex;
	TOKENEXTRA *j;
	TOKENEXTRA *First;
	TOKENEXTRA *Last;
    BOOL_CODER *nbc;
	
    BOOL_CODER  *bc  = &cpi->bc;
	PB_INSTANCE *pbi = &cpi->pb;
	UINT8	FrameType = VP6_GetFrameType ( pbi );

// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = CONTEXT_OVERHEADS_SECTION;
#endif

	// Select which bool coder partition to use
	if ( pbi->MultiStream || (pbi->VpProfile == SIMPLE_PROFILE) )
		nbc = &cpi->bc2;
	else
		nbc = &cpi->bc;

	// Work out which context probabilities need to be updated
	// and output the changes to the bitstream.
	//
	// Error resilliant mode uses a fixed probability update pattern to make the entropy 
	// code more resilliant to dropped frames
	if ( cpi->ErrorResilliantMode )
	{
		// In "error resilliant / VC" mode use an update mechanism that is more tolerant of dropped frames.
		if ( FrameType == BASE_FRAME )
		{
			if ( cpi->CurrentFrame == 1 )
				UpdateContextProbs( cpi );
			else
				UpdateContextProbs3( cpi );
		}
		else
			UpdateContextProbs2( cpi );
	}
	else
	{
		UpdateContextProbs( cpi );
	}

	// Create all the context specific propabilities
	VP6_ConfigureContexts ( pbi );

	// probability that the macroblock is interlaced
	if ( pbi->Configuration.Interlaced )
		AddBitsToBuffer ( bc, (UINT32)(pbi->probInterlaced), 8 );

	// encode coefficients 
	First=cpi->CoeffTokens;
	Last=cpi->CoeffTokenPtr;
	{
		UINT8 coef;
		UINT32 now;
		unsigned int MBrow,MBcol,block;

        j = First;

		now = bc->pos * 8 - 4;
		for ( MBrow=BORDER_MBS; MBrow<pbi->MBRows - BORDER_MBS; MBrow++ )
		{
			for ( MBcol=BORDER_MBS; MBcol<pbi->MBCols-BORDER_MBS; MBcol++ )
			{
				// dumb way to encode the interlaced decision but it works!!!
				{
					UINT8 prob = pbi->probInterlaced;

					// super simple context adjustment
					if ( MBcol>BORDER_MBS )
					{
						if ( pbi->MBInterlaced[MBOffset(MBrow,MBcol-1)] )
							prob = prob - (prob>>1);
						else 
							prob = prob + ((256-prob)>>1);
					}
					
// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = CONTEXT_OVERHEADS_SECTION;
#endif

					if ( pbi->Configuration.Interlaced )
						VP6_EncodeBool(	bc, pbi->MBInterlaced[MBOffset(MBrow,MBcol)], prob );
				}

				if ( pbi->FrameType != BASE_FRAME )
					encodeModeAndMotionVector ( cpi, MBrow, MBcol );

				for ( block=0 ; block<6 ; block++ )
				{
					Plane = block>3;

// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = DC_SECTION;
#endif

					// DC Token
                    coef = 0;
                    VP6AddToken ( cpi, nbc,
                        j, 
                        pbi->DcProbs+DCProbOffset(Plane,0), 
//                        pbi->DcNodeContexts[Plane][j->LastTokenL + j->LastTokenA], 
				        (pbi->DcNodeContexts + DcNodeOffset(Plane, (j->LastTokenL + j->LastTokenA), 0)),
                        pbi->ZeroRunProbs[0], &coef, FALSE );
             		
                    PrecTokenIndex = VP6_PrevTokenIndex[j->Token];
					j++;

                    for ( coef=1; coef<64; coef++ )
                    {
                     
                        UINT32   band = VP6_CoeffToBand[coef];
                        UINT8	*AcProbsPtr = pbi->AcProbs + ACProbOffset(Plane,PrecTokenIndex,band,0 );
                        
// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = AC_SECTION;
#endif

                        VP6AddToken ( cpi, nbc,
                            j, AcProbsPtr, AcProbsPtr, pbi->ZeroRunProbs[(coef >= ZRL_BAND2) ? 1 : 0], 
                            &coef, ((coef>1) && (PrecTokenIndex == 0)) );
                        
                        PrecTokenIndex = VP6_PrevTokenIndex[j->Token];
                        
                        if ( j->Token == DCT_EOB_TOKEN )
                            coef=64;
                        j++;                      
                    } 
				}
			}
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : PackCodedVideo
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Outputs the compressed frame to the bitstream: writes
 *                  a frame header and entropy codes associated lists
 *                  of tokens.
 *
 *  SPECIAL NOTES : Uses either Huffman or Bool coding depending on 
 *                  pbi->UseHuffman flag. 
 *
 ****************************************************************************/
extern double ModeBits;
extern double ModeBits2;

void PackCodedVideo ( CP_INSTANCE *cpi )
{
    UINT32 Buffer2Offset;
	unsigned int duration;
	unsigned int starttsc,endtsc;
	
	BOOL_CODER  *bc  = &cpi->bc;
    PB_INSTANCE *pbi = &cpi->pb;
	BOOL KeyFrame = (pbi->FrameType == BASE_FRAME);

	VP6_readTSC ( &starttsc );

	// Initialise the raw buffer i/o and the two bool coders.
    InitAddRawBitsToBuffer ( &cpi->RawBuffer, pbi->DataOutputInPtr );

	// Start the bool coder or coders
	if ( pbi->MultiStream || (pbi->VpProfile == SIMPLE_PROFILE) )
	{
		// Start the first bool coder: Allow for the raw header bytes.
		VP6_StartEncode ( bc, (pbi->DataOutputInPtr + ((KeyFrame) ? 4 : 3)) );		

		// Create either second Bool or Huffman coded partition
		if ( pbi->UseHuffman )
			InitAddRawBitsToBuffer ( &pbi->HuffBuffer, cpi->OutputBuffer2 );
		else
			VP6_StartEncode ( &cpi->bc2, cpi->OutputBuffer2 );
	}
	else
	{
		// Start the first bool coder: Allow for the raw header bytes.
		VP6_StartEncode( bc, (pbi->DataOutputInPtr + ((KeyFrame) ? 2 : 1)) );		
	}

	// Set flag to insure ouput to the bitstream rather than simulated cost analysis
	bc->MeasureCost = FALSE;

    if ( pbi->UseHuffman )
    {
        // AWG Using runs so copy correct distribution
        memcpy ( cpi->FrameDcTokenDist, cpi->FrameDcTokenDist2, sizeof(cpi->FrameDcTokenDist2) );
        memcpy ( cpi->FrameAcTokenDist, cpi->FrameAcTokenDist2, sizeof(cpi->FrameAcTokenDist2) );
    }

    // Write out the frame header information including size. 
    WriteFrameHeader ( cpi );

    // The tree is not needed (implicit) for key frames
    if ( !KeyFrame ) 
    {
		// Error resilliant mode uses a fixed probability update pattern to make the entropy 
		// code more resilliant to dropped frames
		if ( cpi->ErrorResilliantMode )
		{
			UpdateModeProbs(cpi);
			BuildandPackMvTree2( cpi );
		}
		else
		{
			UpdateModeProbs(cpi);
			BuildandPackMvTree( cpi );
		}
    }
   
	if ( pbi->UseHuffman )
        PackHuffmanCoeffs ( cpi );
    else
        PackArithmeticCoeffs ( cpi );

	// Stop the bool coders and work out this frame size.
	VP6_StopEncode ( bc );

    // ThisFrameSize is measured in bits
	if ( pbi->MultiStream || (pbi->VpProfile == SIMPLE_PROFILE) )
	{
		// Offset to second bitstream partition from start of buffer
		Buffer2Offset = 4 + bc->pos;

		// Write offset to third bitstream partition 
		AddRawBitsToBuffer ( &cpi->RawBuffer, Buffer2Offset, 16 );

		if ( pbi->UseHuffman )
		{
			// Flush buffer for second Huffman coded output partition
			EndAddRawBitsToBuffer ( &pbi->HuffBuffer );   

	        // ThisFrameSize is measured in bits
			cpi->ThisFrameSize = (Buffer2Offset + pbi->HuffBuffer.pos)*8;

			memcpy ( &cpi->RawBuffer.Buffer[Buffer2Offset], pbi->HuffBuffer.Buffer, pbi->HuffBuffer.pos );
		}
 		else
		{
			// Stop the second bool coder
			VP6_StopEncode ( &cpi->bc2);

			// Work out the frame size
			cpi->ThisFrameSize = (Buffer2Offset + cpi->bc2.pos)*8;

			// Assemble output bitstream from two bitstream partitions
			memcpy ( &pbi->DataOutputInPtr[Buffer2Offset], cpi->bc2.buffer, cpi->bc2.pos );
		}
	}
	else
	{
		// Raw header bits + coded bits
        cpi->ThisFrameSize = ((KeyFrame ? 2 : 1) + bc->pos)*8;
	}

	// Stop and flush the raw bits encoder used for the frist part of the header
    EndAddRawBitsToBuffer ( &cpi->RawBuffer );

    // Get time & compute duration
	VP6_readTSC ( &endtsc );
	duration = ( endtsc - starttsc )/ pbi->ProcessorFrequency ;

	if( cpi->avgPackVideoTime == 0)
		cpi->avgPackVideoTime = duration;
	else
		cpi->avgPackVideoTime = ( 7 * cpi->avgPackVideoTime + duration ) >> 3;
}
