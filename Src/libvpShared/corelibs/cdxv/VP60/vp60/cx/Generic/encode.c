/****************************************************************************
*
*   Module Title :     Encode.c
*
*   Description  :     Main encode function.
*
****************************************************************************/

/****************************************************************************
*  Header Files
****************************************************************************/
#include <stdio.h>
#include "compdll.h"
#include "misc_common.h"
#include "encodemv.h"
#include "encodemode.h"

/****************************************************************************
*  Explicit imports
****************************************************************************/ 
extern void PackCodedVideo ( CP_INSTANCE *cpi );
extern void InitLoopDeringThresholds ( PB_INSTANCE *pbi );

#if defined FULLFRAMEFDCT
extern void BuildFrameMbs ( CP_INSTANCE *cpi );
extern void FDCTFrameMbs ( CP_INSTANCE *cpi );
#endif 

extern const UINT32 VP6_QThreshTable[Q_TABLE_SIZE];
extern const UINT32 VP6_ZBinTable[Q_TABLE_SIZE];
extern const UINT32 VP6_RTable[Q_TABLE_SIZE];



/****************************************************************************
 * 
 *  ROUTINE       :     VP6_ShannonCost
 *
 *  INPUTS        :     CP_INSTANCE *cpi  : Pointer to encoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     UINT32: Computed Shannon cost.
 *
 *  FUNCTION      :     Computes the Shannon cost of coding the frame based
 *                      on the observed distribution of tokens for the frame.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
UINT32 VP6_ShannonCost ( CP_INSTANCE *cpi )
{
	UINT32 Cost = 0;
	UINT32 i, j;
	UINT32 Sum;
	UINT32 Band;
	UINT32 Plane;
	UINT32 Prob;

	// First cost the DC tokens...
	for ( Plane=0; Plane<2; Plane++ )
	{
		Sum = 0;
		for ( i=0; i<MAX_ENTROPY_TOKENS; i++ )
		{
			Sum += cpi->FrameDcTokenDist[Plane][i];
		}

		if ( Sum>0 )
		{
			for ( i=0; i<MAX_ENTROPY_TOKENS; i++ )
			{
				Prob = (cpi->FrameDcTokenDist[Plane][i] * 255) / Sum;
				if ( Prob > 254 )
					Prob = 254;
				else if ( Prob == 0 )
					Prob = 1;

				Cost += (VP6_ProbCost[Prob] * cpi->FrameDcTokenDist[Plane][i])/256;
				Cost += cpi->FrameDcTokenDist[Plane][i] * ExtraBitLengths_VP6[i];

                // Save individual token costs for use in next frames RD code
                // Cost in bits x 265.... convert to bits x 64
                cpi->EstDcTokenCosts[Plane][i] = VP6_ProbCost[Prob] >> 2; 
				if ( cpi->EstDcTokenCosts[Plane][i] == 0 )
					cpi->EstDcTokenCosts[Plane][i] = 1;
			}
		}
		// Set defaults for predictive cost tables used in RD code
        else
        {
			for ( i=0; i<MAX_ENTROPY_TOKENS; i++ )
			{
                cpi->EstDcTokenCosts[Plane][i] = 4 << 6;  
            }
        }
	}

	// Then cost the AC tokens...
	for ( Plane=0; Plane<2; Plane++ )
	{
		for ( Band=0; Band<VP6_AC_BANDS; Band++ )
		{
			for ( j=0; j<PREC_CASES; j++ )
			{
				Sum = 0;
				for ( i=0; i<MAX_ENTROPY_TOKENS; i++ )
				{
					Sum += cpi->FrameAcTokenDist[j][Plane][Band][i];
				}

				if ( Sum>0 )
				{
					for ( i=0; i<MAX_ENTROPY_TOKENS; i++ )
					{
						Prob = (cpi->FrameAcTokenDist[j][Plane][Band][i] * 255) / Sum;
						if ( Prob > 254 )
							Prob = 254;
						else if ( Prob == 0 )
							Prob = 1;

						Cost += (VP6_ProbCost[Prob] * cpi->FrameAcTokenDist[j][Plane][Band][i])/256;
						Cost += cpi->FrameAcTokenDist[j][Plane][Band][i] * ExtraBitLengths_VP6[i];
                    
                        // Save individual token costs for use in next frames RD code
                        // Cost in bits x 265.... convert to bits x 64
                        cpi->EstAcTokenCosts[j][Plane][Band][i] = VP6_ProbCost[Prob] >> 2; 
						if ( cpi->EstAcTokenCosts[j][Plane][Band][i] == 0 )
							cpi->EstAcTokenCosts[j][Plane][Band][i] = 1;
					}
				}
				// Set defaults for predictive cost tables used in RD code
                else
                {
			        for ( i=0; i<MAX_ENTROPY_TOKENS; i++ )
			        {
                        cpi->EstAcTokenCosts[j][Plane][Band][i] = 4 << 6;  
                    }
                }
			}
		}
	}

	// Finally cost the zero run lengths...
    for ( i=0; i<ZRL_BANDS; i++ )
	{
		Sum = 0;
		for ( j=0; j<64; j++ )
		{
			Sum += cpi->FrameZrlDist[i][j];
		}

		// Now work out Shannon cost approximations for each run length
		if ( Sum>0 )
		{
			for ( j=0; j<64; j++ )
			{
				Prob = (cpi->FrameZrlDist[i][j] * 255) / Sum;
				if ( Prob > 255 )
					Prob = 255;
				else if ( Prob == 0 )
					Prob = 1;

				// Add in to our total cost estimate
				Cost += (VP6_ProbCost[Prob] * cpi->FrameZrlDist[i][j])/256;

                // Cost in bits x 265.... convert to bits x 64
				cpi->EstZrlCosts[i][j] = VP6_ProbCost[Prob] >> 2;
			}
		}
		// Set a default for predictive cost tables used in RD code
		else
		{
			cpi->EstZrlCosts[i][j] = 3 << 6;
		}
	}
	return Cost;
}

/****************************************************************************
 * 
 *  ROUTINE       :     EncodeData
 *
 *  INPUTS        :     CP_INSTANCE *cpi  : Pointer to encoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     UINT32: Always TRUE (This needs fixing!)
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  SPECIAL NOTES :     Applies rate targetting heuristics. 
 *
 ****************************************************************************/
UINT32 EncodeData ( CP_INSTANCE *cpi )
{                                                       
    unsigned char *tmp;
	BOOL	RedoY = FALSE;
	UINT32	FrameOverShootLimit;
	UINT32  FrameUnderShootLimit;
	UINT32  ShannonBits;
	UINT32  TopIndex;
	UINT32  BottomIndex;
	INT32   ModeMvCost = cpi->ModeMvCostEstimate/64;		// Estimated overhed in bits for modes and MVs
	INT32   QuantizePasses = 0;
	PB_INSTANCE *pbi = &cpi->pb;

	// Set tolerance values for frame overshoot and undershoot.
	if ( VP6_GetFrameType(pbi) == BASE_FRAME )
	{
		if ( cpi->BufferedMode )
		{
			if ( cpi->BufferLevel < cpi->OptimalBufferLevel )
			{
				FrameOverShootLimit = cpi->ThisFrameTarget * 10/8;
				FrameUnderShootLimit = 0;
			}
			else
			{
				FrameOverShootLimit = cpi->ThisFrameTarget * 14/8;
				FrameUnderShootLimit = 0;
			}
		}
		// Unbuffered video mode (eg video conferencing)
		else
		{
			FrameOverShootLimit = cpi->ThisFrameTarget * 10/8;
			FrameUnderShootLimit = 0;
		}

		// Limit Q range for the adaptive loop.
		BottomIndex = cpi->Configuration.ActiveWorstQuality;
		if ( BottomIndex < 20)
			BottomIndex = 20;
		TopIndex = cpi->Configuration.ActiveBestQuality;
		if ( TopIndex > 60 )
			TopIndex = 60;
	}
	else
	{
		// Normal streamed video mode
		if ( cpi->BufferedMode && cpi->pass != 2 )
		{
			if ( cpi->BufferLevel < cpi->OptimalBufferLevel )
			{
				// Looser frame size constraints for local file playback
				if ( cpi->EndUsage == USAGE_LOCAL_FILE_PLAYBACK )
				{
					if ( cpi->ThisFrameTarget > cpi->PerFrameBandwidth )
						FrameOverShootLimit = cpi->ThisFrameTarget * 2;
					else
						FrameOverShootLimit = cpi->PerFrameBandwidth * 2;

					FrameUnderShootLimit = cpi->ThisFrameTarget * 3/8;
				}
				else
				{
					if ( cpi->MaxAllowedDatarate > 125 )
						FrameOverShootLimit = (cpi->ThisFrameTarget * cpi->MaxAllowedDatarate)/100;
					else 
						FrameOverShootLimit = cpi->ThisFrameTarget * 125/100;

					FrameUnderShootLimit = cpi->ThisFrameTarget * 3/8;
				}
			}
			else
			{
				// Looser frame size constraints for local file playback
				if ( cpi->EndUsage == USAGE_LOCAL_FILE_PLAYBACK)
				{
					if ( cpi->ThisFrameTarget > cpi->PerFrameBandwidth )
						FrameOverShootLimit = cpi->ThisFrameTarget * 2;
					else
						FrameOverShootLimit = cpi->PerFrameBandwidth * 2;

					FrameUnderShootLimit = cpi->ThisFrameTarget * 4/8;
				}
				else
				{
					if ( cpi->MaxAllowedDatarate > 150 )
						FrameOverShootLimit = (cpi->ThisFrameTarget * cpi->MaxAllowedDatarate)/100;
					else 
						FrameOverShootLimit = cpi->ThisFrameTarget * 150/100;

					FrameUnderShootLimit = cpi->ThisFrameTarget * 5/8;
				}
			}
		}

		// Unbuffered video mode (eg video conferencing)
		// jbb upped this from 10/8 to 14/8 and shut off lower 
        //   limit.  This basically eliminated the multiple 
        //   go round issue?
        else
		{
			FrameOverShootLimit = cpi->ThisFrameTarget * 14/8;
			FrameUnderShootLimit = cpi->ThisFrameTarget * 0/8;
		}

		// Limit Q range for the adaptive loop.
		BottomIndex = cpi->Configuration.ActiveWorstQuality;
		TopIndex = cpi->Configuration.ActiveBestQuality;
		if ( TopIndex > 60)
			TopIndex = 60;
	}


	// Q adjustment loop (Only loops around if our rate targeting huristic is badly off). 
	do 
	{
        #if defined FULLFRAMEFDCT
            FDCTFrameMbs ( cpi );
        #endif

		// Zero down the structures used to count token distributions
		memset ( cpi->FrameDcTokenDist,  0, sizeof(cpi->FrameDcTokenDist)  );	
		memset ( cpi->FrameDcTokenDist2, 0, sizeof(cpi->FrameDcTokenDist2) );	
		memset ( cpi->FrameAcTokenDist,  0, sizeof(cpi->FrameAcTokenDist)  );
		memset ( cpi->FrameAcTokenDist2, 0, sizeof(cpi->FrameAcTokenDist2) );
		memset ( cpi->FrameNzCount,      0, sizeof(cpi->FrameNzCount)      );
	
		// Zero down run distribution counts
		memset( cpi->FrameZrlDist, 0, sizeof(cpi->FrameZrlDist) );
		cpi->FrameZeroCount[0] = 0;
		cpi->FrameZeroCount[1] = 0;

		// Pack DC tokens and adjust the ones we couldn't predict 2d
		pbi->CodedBlockIndex = 0;

		// reset our token list
		cpi->CoeffTokenPtr = cpi->CoeffTokens;

		// Set loop/predictionfilter thresholds based upon Q
		if ( pbi->UseLoopFilter == LOOP_FILTER_DERING )
			InitLoopDeringThresholds( pbi );

#if defined FULLFRAMEFDCT
        BuildFrameMbs ( cpi );
#else
        // Encode frame MB-by-MB
		EncodeFrameMbs(cpi);
#endif
		// Increment the counter on the number of passes through the dct quantize loop
		QuantizePasses++;

		// Clear MMX state so floating point can work again
#if defined(_MSC_VER)
	    ClearSysState();
#endif

		// If we are in buffered (streaming) mode and have selected fastest speed 
		// then disallow the re-code loop
		if ( (cpi->QuickCompress == 2) && (cpi->BufferedMode) )
			break;

		// Test for severe over-run or under-run conditions. If necessary adjust Q and try again.
		ShannonBits = VP6_ShannonCost(cpi) + ModeMvCost;


		// Are we are overshooting and up against the limit of active max Q.
		if ( (pbi->quantizer->FrameQIndex == cpi->Configuration.ActiveWorstQuality) &&
			 (cpi->Configuration.ActiveWorstQuality > cpi->Configuration.WorstQuality) &&
			 (ShannonBits > FrameOverShootLimit) )
		{
			INT32 OverSizePercent = ((ShannonBits - FrameOverShootLimit) * 100) / FrameOverShootLimit;

			// If so is there any scope for relaxing it
			while ( (cpi->Configuration.ActiveWorstQuality > cpi->Configuration.WorstQuality) &&
				    (OverSizePercent > 0) )
			{

				cpi->Configuration.ActiveWorstQuality --;
				BottomIndex = cpi->Configuration.ActiveWorstQuality;

				OverSizePercent -= 6;		// Assume 1 qstep = about 65 on frame size.
			}
		}

		// Should we try and recode
		if ( ((ShannonBits > FrameOverShootLimit) && (pbi->quantizer->FrameQIndex > BottomIndex)) || 
			 ((ShannonBits < FrameUnderShootLimit) && (pbi->quantizer->FrameQIndex < TopIndex)) )
		{
			UINT32 LastQIndex = pbi->quantizer->FrameQIndex;
			
			if ( ShannonBits > FrameOverShootLimit )
			{  
				// Truncate TmpBottomIndex
				UINT32 TmpBottomIndex = (pbi->quantizer->FrameQIndex + BottomIndex) >> 1;

				if ( pbi->quantizer->FrameQIndex > 0 )
					TopIndex = pbi->quantizer->FrameQIndex - 1;
				else
					TopIndex = 0;

  				// Tweak the appropriate BpbCorrectionFactor.
				UpdateBpbCorrectionFactor( cpi, ShannonBits );
				
				if ( VP6_GetFrameType(pbi) == BASE_FRAME )
					RegulateQ(cpi, FrameOverShootLimit );
				else
					RegulateQ(cpi, cpi->ThisFrameTarget );

				// Do not allow jumps to be to large and to go out of range.
				if ( pbi->quantizer->FrameQIndex < TmpBottomIndex )
					ClampAndUpdateQ ( cpi, (UINT32)TmpBottomIndex );
				else if ( pbi->quantizer->FrameQIndex > TopIndex )
					ClampAndUpdateQ ( cpi, (UINT32)TopIndex );
			}
			else
			{
				// Round TmpTopIndex Up
				UINT32 TmpTopIndex = (TopIndex + pbi->quantizer->FrameQIndex + 1) >> 1;

				if ( pbi->quantizer->FrameQIndex < (Q_TABLE_SIZE-1) )
					BottomIndex = pbi->quantizer->FrameQIndex + 1;
				else
					BottomIndex = (Q_TABLE_SIZE-1);

  				// Tweak the appropriate BpbCorrectionFactor.
				UpdateBpbCorrectionFactor( cpi, ShannonBits );
				RegulateQ(cpi, cpi->ThisFrameTarget );

				// Clamp Q to upper and lower limits
				if ( pbi->quantizer->FrameQIndex < BottomIndex )
					ClampAndUpdateQ ( cpi, (UINT32)BottomIndex );
				else if ( pbi->quantizer->FrameQIndex > TmpTopIndex )
					ClampAndUpdateQ ( cpi, (UINT32)TmpTopIndex );
			}

			// If we were able to adjust Q index 
			// given current constraints, then cycle round again.
			if ( pbi->quantizer->FrameQIndex != LastQIndex )	
			{
				// Loop round and try again at the modified Q
				RedoY = TRUE;
			}
			else
			{
				RedoY = FALSE;
			}
		}
		else
			RedoY = FALSE;
	}
    while ( RedoY );


	// Optimize the scan order and then repeat dct and tokenize phases
	if ( ( (cpi->pb.Configuration.Interlaced) || (cpi->AllowScanOrderUpdates) ) && 
		   (!cpi->ErrorResilliantMode) &&
		   (cpi->QuickCompress !=2) )
	{
		// Work out the optimal scan bands based upon the frame zero counts for this frame
		PredictScanOrder( cpi );

		// Build the scan order
		BuildScanOrder( &(cpi->pb), cpi->NewScanOrderBands );

		// Zero down the structures used to count token distributions
		memset ( cpi->FrameDcTokenDist,  0, sizeof(cpi->FrameDcTokenDist)  );	
		memset ( cpi->FrameDcTokenDist2, 0, sizeof(cpi->FrameDcTokenDist2) );	
		memset ( cpi->FrameAcTokenDist,  0, sizeof(cpi->FrameAcTokenDist)  );
		memset ( cpi->FrameAcTokenDist2, 0, sizeof(cpi->FrameAcTokenDist2) );
		memset ( cpi->FrameNzCount,      0, sizeof(cpi->FrameNzCount)      );
	
		// Zero run distribution counts
		memset( cpi->FrameZrlDist, 0, sizeof(cpi->FrameZrlDist) );
		cpi->FrameZeroCount[0] = 0;
		cpi->FrameZeroCount[1] = 0;

		// Pack DC tokens and adjust the ones we couldn't predict 2d
		pbi->CodedBlockIndex = 0;

		// reset our token list
		cpi->CoeffTokenPtr = cpi->CoeffTokens;

		// Set loop/prediction filter thresholds based upon Q
		if ( pbi->UseLoopFilter == LOOP_FILTER_DERING )
			InitLoopDeringThresholds( pbi );
		
        // Encode frame MB-by-MB
#if defined FULLFRAMEFDCT
        BuildFrameMbs ( cpi );
#else
		EncodeFrameMbs(cpi);
#endif

		// Clear MMX state so floating point can work again
#if defined(_MSC_VER)
	    ClearSysState();
#endif
	}
    // Decide whether to drop back to using Huffman entropy coding or not
    if ( cpi->pb.VpProfile == SIMPLE_PROFILE )
	{
        if( ShannonBits > 9000*8 )
            pbi->UseHuffman = TRUE;
        else 
            pbi->UseHuffman = FALSE; 
	}

    // Entropy code the tokens generated & output bits to the bitstream
    PackCodedVideo(cpi);

    // switch pointers so that this frame recon becomes last frame recon
    tmp = pbi->LastFrameRecon;
    pbi->LastFrameRecon = pbi->ThisFrameRecon;
    pbi->ThisFrameRecon = tmp;
	
	// update UMV border 
	UpdateUMVBorder ( pbi->postproc, pbi->LastFrameRecon );
	
	// Update the golden frame buffer.
	if( (pbi->FrameType == BASE_FRAME) || pbi->RefreshGoldenFrame )
		memcpy ( pbi->GoldenFrame, pbi->LastFrameRecon, pbi->ReconYPlaneSize + 2* pbi->ReconUVPlaneSize ); 

#if defined(_MSC_VER)
	ClearSysState();
#endif

    BuildMVCostEstimates(cpi);
	BuildModeCostEstimates(cpi);

    // AWG This function returns a UINT32 __NOT__ a BOOL !!
	return TRUE;
}
