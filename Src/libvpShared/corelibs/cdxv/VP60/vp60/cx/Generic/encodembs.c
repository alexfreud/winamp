/****************************************************************************
*
*   Module Title :     Encodembs.c 
*
*   Description  :     Compressor functions for block order transmittal
*
*   AUTHOR       :     Paul Wilkins
*
****************************************************************************/
#define STRICT               /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "compdll.h"
#include "misc_common.h"
#include "decodemode.h"
#include "decodemv.h"
#include "quantize.h"


/****************************************************************************
 * 
 *  ROUTINE       :     PredictBlock
 *
 *  INPUTS        :     CP_INSTANCE *cpi  : Pointer to encoder instance.
 *                      BLOCK_POSITION bp : Position of block in MB (0-5)
 *                      UINT32 MBrow      : MB row (NOT USED).
 *                      UINT32 MBcol      : MB column (NOT USED).
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Creates a prediction for an 8x8 block given a coding 
 *                      mode and other data stored at the MB level.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void PredictBlock ( CP_INSTANCE *cpi, BLOCK_POSITION bp)
{
	MACROBLOCK_INFO *mbi=&cpi->pb.mbi;
    INT32  CurrentReconStride = cpi->pb.mbi.blockDxInfo[bp].CurrentReconStride;
    INT32  CurrentSourceStride = cpi->pb.mbi.blockDxInfo[bp].CurrentSourceStride;
    UINT32 thisRecon = cpi->pb.mbi.blockDxInfo[bp].thisRecon;
    UINT32 Source = cpi->pb.mbi.blockDxInfo[bp].Source;

	if ( VP6_ModeUsesMC[mbi->Mode] )
	{
		VP6_PredictFilteredBlock ( &cpi->pb, cpi->DCTDataBuffer, bp );

		SubtractBlock ( &cpi->yuv1ptr[Source], cpi->DCTDataBuffer, CurrentSourceStride );
	}
	else if ( mbi->Mode==CODE_INTER_NO_MV ) 
	{
		Sub8 ( &cpi->yuv1ptr[Source], &cpi->pb.LastFrameRecon[thisRecon], cpi->DCTDataBuffer, 0, 0, CurrentSourceStride, CurrentReconStride );
	}
	else if ( mbi->Mode==CODE_USING_GOLDEN )
	{
		Sub8 ( &cpi->yuv1ptr[Source], &cpi->pb.GoldenFrame[thisRecon], cpi->DCTDataBuffer, 0, 0, CurrentSourceStride, CurrentReconStride );
	}
	else if ( mbi->Mode==CODE_INTRA )
	{
		Sub8_128 ( &cpi->yuv1ptr[Source], cpi->DCTDataBuffer, 0, 0, CurrentSourceStride );
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     PredictDCE
 *
 *  INPUTS        :     CP_INSTANCE *cpi     : Pointer to encoder instance.
 *	                    BLOCK_POSITION bp    : Position of block in MB (0-5)
 *	                    Q_LIST_ENTRY *LastDC : Pointer to array of DC values last used (one per prediction frame type)
 *	                    BLOCK_CONTEXT *Above : Pointer to above context for block.
 *	                    BLOCK_CONTEXT *Left  : Pointer to left context for block.
 *	
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Computes a DC predictor for the block based on two
 *                      supplied contexts, one above and one to the left.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void PredictDCE
( 
	CP_INSTANCE *cpi,
	BLOCK_POSITION bp
)
{
	PB_INSTANCE *pbi = &cpi->pb;
	UINT8 Frame = VP6_Mode2Frame[pbi->mbi.Mode];

	Q_LIST_ENTRY *LastDC = pbi->mbi.blockDxInfo[bp].LastDc;
	BLOCK_CONTEXT *Above = pbi->mbi.blockDxInfo[bp].Above;
	BLOCK_CONTEXT *Left = pbi->mbi.blockDxInfo[bp].Left;

	INT32 Avg;

	Avg = LastDC[Frame];

	if(Frame == Left->Frame) 
	{
		Avg = Left->Dc;
	}
	if(Frame == Above->Frame) 
	{
		Avg = Above->Dc;
        if(Frame == Left->Frame)
        {
            #define HIGHBITDUPPED(X) (((signed short) X)  >> 15)
            Avg += Left->Dc;
            Avg += (HIGHBITDUPPED(Avg)&1);
			Avg >>= 1;

        }
	}

//Jim says that y,u,v all use the same quantizer so we probably do not need to have a separate dequant ptr
	// make sure the last dc is updated for next time
	cpi->DCT_codes[0] -= ((Avg * pbi->mbi.blockDxInfo[bp].dequantPtr[0]));
}
/****************************************************************************
 * 
 *  ROUTINE       :     EncodeMacroBlock
 *
 *  INPUTS        :     CP_INSTANCE *cpi  : Pointer to encoder instance.
 *                      UINT32 MBrow      : MB row.
 *                      UINT32 MBcol      : MB column.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Encodes a single macro-block by coding each of
 *                      it's six constituent blocks in turn.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void EncodeMacroBlock (	CP_INSTANCE *cpi, UINT32 MBrow, UINT32 MBcol )
{
	UINT32 bp;
    UINT32 fragCoefEOB;
//	UINT32 MBPointer;
//	UINT32 MBSourcePointer;
	//INT32  NextBlock;
	//INT32  NextLineInBlock;
    UINT32 FragsToCheck[6];
	PB_INSTANCE *pbi = &cpi->pb;
    UINT32 FragIndex = 2*(MBrow-BORDER_MBS) * pbi->HFragments + 2*(MBcol-BORDER_MBS);

	pbi->mbi.Interlaced = pbi->MBInterlaced[MBOffset(MBrow,MBcol)];

	//NextBlock = 8;
	//NextLineInBlock = 1;

	if ( pbi->mbi.Interlaced == 1 )
	{
		//NextBlock = 1;
		//NextLineInBlock = 2;
		pbi->mbi.blockDxInfo[0].CurrentReconStride = 
		pbi->mbi.blockDxInfo[1].CurrentReconStride = 
		pbi->mbi.blockDxInfo[2].CurrentReconStride = 
		pbi->mbi.blockDxInfo[3].CurrentReconStride = pbi->Configuration.YStride * 2;

	    pbi->mbi.blockDxInfo[2].thisRecon -= (pbi->Configuration.YStride * 7);
	    pbi->mbi.blockDxInfo[3].thisRecon -= (pbi->Configuration.YStride * 7);

	    pbi->mbi.blockDxInfo[0].CurrentSourceStride =
	    pbi->mbi.blockDxInfo[1].CurrentSourceStride =
	    pbi->mbi.blockDxInfo[2].CurrentSourceStride =
	    pbi->mbi.blockDxInfo[3].CurrentSourceStride = pbi->Configuration.VideoFrameWidth * 2;

        pbi->mbi.blockDxInfo[2].Source -= (pbi->Configuration.VideoFrameWidth * 7);
        pbi->mbi.blockDxInfo[3].Source -= (pbi->Configuration.VideoFrameWidth * 7);
	}


//note: should be able to move FragsToCheck into the blockDxInfo struct
//then in the MB loop, we should be able to inc the values instead of doing these multiplies
//it may not affect the pc performance, but it may help other processors
    FragsToCheck[0] = FragIndex;
    FragsToCheck[1] = FragIndex+1;
    FragsToCheck[2] = FragIndex+cpi->pb.HFragments;
    FragsToCheck[3] = FragIndex+cpi->pb.HFragments+1;
    FragsToCheck[4] = cpi->pb.YPlaneFragments + (MBrow-BORDER_MBS) * (cpi->pb.HFragments / 2) + MBcol-BORDER_MBS;
    FragsToCheck[5] = cpi->pb.YPlaneFragments + cpi->pb.UVPlaneFragments + (MBrow-BORDER_MBS) * ( cpi->pb.HFragments / 2 ) + MBcol-BORDER_MBS;

    cpi->pb.mbi.Mode = -1;
    
    for( bp=0 ; bp<6 ; bp++ )
    {
        cpi->pb.mbi.Mode = cpi->pb.FragInfo[FragsToCheck[bp]].FragCodingMode;
        cpi->pb.mbi.Mv[bp].x = cpi->pb.FragInfo[FragsToCheck[bp]].MVectorX;
        cpi->pb.mbi.Mv[bp].y = cpi->pb.FragInfo[FragsToCheck[bp]].MVectorY;
    }

    for( bp=0 ; bp<6 ; bp++ )
    {
	    // Build a block predictor, subtract from source to get prediction error for block
	    PredictBlock ( cpi, bp );
	    
	    // Transform the error signal using the forward DCT to get set of transform coefficients
	    fdct_short ( cpi->DCTDataBuffer, cpi->DCT_codes );

	    // Predict the DCT DC value from those in surrounding blocks
	    PredictDCE ( cpi, bp );

	    // Quantize the resulting DCT coefficients at prevailing Q
	    VP6_quantize ( cpi->pb.quantizer, cpi->DCT_codes, cpi->pb.mbi.blockDxInfo[bp].coeffsPtr, (UINT8)bp );   

	    // Tokenize the resulting quantized coefficients
	    fragCoefEOB = (UINT8)TokenizeFrag ( cpi, 
                                                    cpi->pb.mbi.blockDxInfo[bp].coeffsPtr, 
                                                    cpi->pb.mbi.blockDxInfo[bp].Plane, 
                                                    pbi->mbi.blockDxInfo[bp].Above, 
                                                    pbi->mbi.blockDxInfo[bp].Left );

        // Produce reconstructed block so encoder has __exactly__ the same
        // data for last frame reconstruction as the decoder
	    
        // Re-form the DC value from the prediction
	    VP6_PredictDC ( &cpi->pb, bp );
	    
	    // Invert the transform to re-create the prediction error
	    cpi->pb.idct[fragCoefEOB]( cpi->pb.mbi.blockDxInfo[bp].coeffsPtr, 
                                   cpi->pb.mbi.blockDxInfo[bp].dequantPtr, 
                                   cpi->pb.ReconDataBuffer[bp] );
	    
	    // Add prediction error to predictor to re-create block as it appears at decoder
	    VP6_ReconstructBlock(&cpi->pb, bp);

	    // DEBUG Code: Store prediction block in Post-processing buffer 
	    //PredictBlockToPostProcessBuffer ( &cpi->pb, bp );

	    // Update the context info for the next block 
	    cpi->pb.CodedBlockIndex++;
	    VP6_UpdateContextA ( &cpi->pb, pbi->mbi.blockDxInfo[bp].Above, bp );
	    VP6_UpdateContext  ( &cpi->pb, pbi->mbi.blockDxInfo[bp].Left,  bp );
    }

	if ( pbi->mbi.Interlaced == 1 )
	{
        /* reset to non interlaced */
    	pbi->mbi.blockDxInfo[0].CurrentReconStride =
	    pbi->mbi.blockDxInfo[1].CurrentReconStride =
	    pbi->mbi.blockDxInfo[2].CurrentReconStride =
	    pbi->mbi.blockDxInfo[3].CurrentReconStride = pbi->Configuration.YStride;

	    pbi->mbi.blockDxInfo[2].thisRecon += (pbi->Configuration.YStride * 7);
	    pbi->mbi.blockDxInfo[3].thisRecon += (pbi->Configuration.YStride * 7);

	    pbi->mbi.blockDxInfo[0].CurrentSourceStride =
	    pbi->mbi.blockDxInfo[1].CurrentSourceStride =
	    pbi->mbi.blockDxInfo[2].CurrentSourceStride =
	    pbi->mbi.blockDxInfo[3].CurrentSourceStride = pbi->Configuration.VideoFrameWidth;

        pbi->mbi.blockDxInfo[2].Source += (pbi->Configuration.VideoFrameWidth * 7);
        pbi->mbi.blockDxInfo[3].Source += (pbi->Configuration.VideoFrameWidth * 7);
	}

}


/****************************************************************************
 * 
 *  ROUTINE       :     EncodeFrameMbs
 *
 *  INPUTS        :     CP_INSTANCE *cpi  : Pointer to encoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Encodes a frame by encoding each of it's constituent
 *                      macro-blocks in turn.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void EncodeFrameMbs ( CP_INSTANCE *cpi )
{
	PB_INSTANCE *pbi = &cpi->pb;

	unsigned int duration;
	unsigned int starttsc;
    unsigned int endtsc;
	
    // Record start time
    VP6_readTSC ( &starttsc );

    // Reset Dc zero & Ac EOB run counters
    cpi->CurrentDcZeroRun[0]  = 0;
    cpi->CurrentDcZeroRun[1]  = 0;
	cpi->DcZeroRunStartPtr[0] = NULL;
	cpi->DcZeroRunStartPtr[1] = NULL;
    cpi->CurrentAc1EobRun[0]  = 0;
    cpi->CurrentAc1EobRun[1]  = 0;
    cpi->Ac1EobRunStartPtr[0] = NULL;
    cpi->Ac1EobRunStartPtr[1] = NULL;

	if ( cpi->pb.FrameType == BASE_FRAME )
	{
        // Initialise probability distributions with baseline default values
		memcpy ( cpi->pb.IsMvShortProb,    DefaultIsShortProbs,      sizeof(cpi->pb.IsMvShortProb) );
		memcpy ( cpi->pb.MvShortProbs,     DefaultMvShortProbs,      sizeof(cpi->pb.MvShortProbs) );
		memcpy ( cpi->pb.MvSignProbs,      DefaultSignProbs,         sizeof(cpi->pb.MvSignProbs) );
		memcpy ( cpi->pb.MvSizeProbs,      DefaultMvLongProbs,       sizeof(cpi->pb.MvSizeProbs) );
		memcpy ( cpi->pb.probXmitted,      VP6_BaselineXmittedProbs, sizeof(cpi->pb.probXmitted) );
		memset ( cpi->pb.MBModeProb,       128,                      sizeof(cpi->pb.MBModeProb) );
		memset ( cpi->pb.BModeProb,        128,                      sizeof(cpi->pb.BModeProb) );
		memset ( cpi->pb.probModeSame,     128,                      sizeof(cpi->pb.probModeSame) );
		memset ( cpi->pb.probMode,         128,                      sizeof(cpi->pb.probMode) );
		memset ( cpi->pb.predictionMode,   1,                        sizeof(char)*cpi->pb.MacroBlocks );

        memset ( cpi->MBModeCostNoNearest, 0,                        sizeof(cpi->MBModeCostNoNearest) );
		memset ( cpi->MBModeCostNoNear,    0,                        sizeof(cpi->MBModeCostNoNear) );
		memset ( cpi->MBModeCostBoth,      0,                        sizeof(cpi->MBModeCostBoth) );
		memset ( cpi->BModeCost,           0,                        sizeof(cpi->BModeCost) );
	}
	else
	{
	    cpi->pb.LastMode = CODE_INTER_NO_MV;  
	}

	// since we are on a new frame reset the above contexts 
	VP6_ResetAboveContext( &cpi->pb );

    {
	    UINT32 MBrow;
	    UINT32 MBRows = cpi->pb.MBRows; 
	    UINT32 MBCols = cpi->pb.MBCols;

        MBCols -= BORDER_MBS;
        MBRows -= BORDER_MBS;

        // AWG Code Added: Initialize strides for source & recon
    	pbi->mbi.blockDxInfo[0].CurrentReconStride =
	    pbi->mbi.blockDxInfo[1].CurrentReconStride =
	    pbi->mbi.blockDxInfo[2].CurrentReconStride =
	    pbi->mbi.blockDxInfo[3].CurrentReconStride = pbi->Configuration.YStride;

	    pbi->mbi.blockDxInfo[0].CurrentSourceStride =
	    pbi->mbi.blockDxInfo[1].CurrentSourceStride =
	    pbi->mbi.blockDxInfo[2].CurrentSourceStride =
	    pbi->mbi.blockDxInfo[3].CurrentSourceStride = pbi->Configuration.VideoFrameWidth;
        // AWG End Added Code

        // for each row of macroblocks 
	    MBrow=BORDER_MBS;
        do
	    {
            MACROBLOCK_INFO *mbi = &cpi->pb.mbi;
            UINT32 MBcol;

		    VP6_ResetLeftContext(&cpi->pb);

		    // for each macroblock within a row of macroblocks

	        mbi->blockDxInfo[0].Above = &pbi->fc.AboveY[BORDER_MBS*2];
	        mbi->blockDxInfo[1].Above = &pbi->fc.AboveY[BORDER_MBS*2+1];
	        mbi->blockDxInfo[2].Above = &pbi->fc.AboveY[BORDER_MBS*2];
	        mbi->blockDxInfo[3].Above = &pbi->fc.AboveY[BORDER_MBS*2+1];
	        mbi->blockDxInfo[4].Above = &pbi->fc.AboveU[BORDER_MBS];
	        mbi->blockDxInfo[5].Above = &pbi->fc.AboveV[BORDER_MBS];


	        mbi->blockDxInfo[0].thisRecon = pbi->ReconYDataOffset + ((MBrow * pbi->Configuration.YStride) << 4) + (BORDER_MBS * 16);
	        mbi->blockDxInfo[1].thisRecon = mbi->blockDxInfo[0].thisRecon + 8;
	        mbi->blockDxInfo[2].thisRecon = mbi->blockDxInfo[0].thisRecon + (pbi->Configuration.YStride << 3);
	        mbi->blockDxInfo[3].thisRecon = mbi->blockDxInfo[1].thisRecon + (pbi->Configuration.YStride << 3);
	        mbi->blockDxInfo[4].thisRecon = pbi->ReconUDataOffset + ((MBrow * pbi->Configuration.UVStride) << 3) + (BORDER_MBS * 8);
	        mbi->blockDxInfo[5].thisRecon = pbi->ReconVDataOffset + ((MBrow * pbi->Configuration.UVStride) << 3) + (BORDER_MBS * 8);


            mbi->blockDxInfo[0].Source = pbi->YDataOffset + ((MBrow * 16) - UMV_BORDER) * pbi->Configuration.VideoFrameWidth;
            mbi->blockDxInfo[1].Source = mbi->blockDxInfo[0].Source + 8;
	        mbi->blockDxInfo[2].Source = mbi->blockDxInfo[0].Source + (pbi->Configuration.VideoFrameWidth << 3);
	        mbi->blockDxInfo[3].Source = mbi->blockDxInfo[1].Source + (pbi->Configuration.VideoFrameWidth << 3);
	        mbi->blockDxInfo[4].Source = pbi->UDataOffset + ((MBrow * 8) - (UMV_BORDER>>1)) * (pbi->Configuration.VideoFrameWidth/2);
	        mbi->blockDxInfo[5].Source = pbi->VDataOffset + ((MBrow * 8) - (UMV_BORDER>>1)) * (pbi->Configuration.VideoFrameWidth/2);

            MBcol=BORDER_MBS;
            do
            {

			    // Decode the macroblock
			    EncodeMacroBlock(cpi, MBrow, MBcol);   


	            mbi->blockDxInfo[0].Above += 2;
	            mbi->blockDxInfo[1].Above += 2;
	            mbi->blockDxInfo[2].Above += 2;
	            mbi->blockDxInfo[3].Above += 2;
	            mbi->blockDxInfo[4].Above += 1;
	            mbi->blockDxInfo[5].Above += 1;

                mbi->blockDxInfo[0].thisRecon += 16;
                mbi->blockDxInfo[1].thisRecon += 16;
                mbi->blockDxInfo[2].thisRecon += 16;
                mbi->blockDxInfo[3].thisRecon += 16;
                mbi->blockDxInfo[4].thisRecon += 8;
                mbi->blockDxInfo[5].thisRecon += 8;

                mbi->blockDxInfo[0].Source += 16;
                mbi->blockDxInfo[1].Source += 16;
                mbi->blockDxInfo[2].Source += 16;
                mbi->blockDxInfo[3].Source += 16;
                mbi->blockDxInfo[4].Source += 8;
                mbi->blockDxInfo[5].Source += 8;

		    } while(++MBcol < MBCols);


	    } while(++MBrow < MBRows);
    }


    // Terminate current DC run of zeros or AC run of EOB
    if ( cpi->CurrentDcZeroRun[0] > 0 )
    {
        cpi->DcZeroRunStartPtr[0]->Extra = cpi->CurrentDcZeroRun[0];
        cpi->CurrentDcZeroRun[0] = 0;
    }
    if ( cpi->CurrentDcZeroRun[1] > 0 )
    {
        cpi->DcZeroRunStartPtr[1]->Extra = cpi->CurrentDcZeroRun[1];
        cpi->CurrentDcZeroRun[1] = 0;
    }
    if ( cpi->CurrentAc1EobRun[0] > 0 )
    {
        cpi->Ac1EobRunStartPtr[0]->Extra = cpi->CurrentAc1EobRun[0];
        cpi->CurrentAc1EobRun[0] = 0;
    }
    if ( cpi->CurrentAc1EobRun[1] > 0 )
    {
        cpi->Ac1EobRunStartPtr[1]->Extra = cpi->CurrentAc1EobRun[1];
        cpi->CurrentAc1EobRun[1] = 0;
    }

    // Record end time and compute duration
    VP6_readTSC ( &endtsc );
	duration = (endtsc - starttsc)/cpi->pb.ProcessorFrequency;

	if( cpi->avgEncodeTime==0 )
		cpi->avgEncodeTime = duration;
	else
		cpi->avgEncodeTime = ( 7 * cpi->avgEncodeTime + duration ) >> 3;
}
