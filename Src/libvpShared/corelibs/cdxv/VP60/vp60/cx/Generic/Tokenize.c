/****************************************************************************
*
*   Module Title :     Tokenize.C
*
*   Description  :     Tokenizing fragments for output by pack video
*
****************************************************************************/
#define STRICT         /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include <math.h>       // For abs()
#include "compdll.h"

/****************************************************************************
*  Module Statics
****************************************************************************/
static TOKENEXTRA DctValueTokens[DCT_MAX_VALUE*2];

/****************************************************************************
 * 
 *  ROUTINE       :     FillValueTokens
 *
 *  INPUTS        :     None.
 *						
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Fills in the DctValueTokens array used during
 *                      compression for fast look-up of token and eatra-bits
 *                      information.
 *
 *  SPECIAL NOTES :     None.
 *
 ****************************************************************************/
void FillValueTokens ( void )
{
	INT32 i;

	for ( i=-2048; i<2047; i++ )
	{
		UINT32 AbsDataVal = abs ( i );
		TOKENEXTRA *TokenExtra = DctValueTokens + 2048 + i;

		// Values are tokenised as category value and a number of 
		// additional bits that define the position within the category.
		if ( i == 0 )
		{
			TokenExtra->Token = 0;
		} 
		else if ( AbsDataVal < VP6_DctRangeMinVals[DCT_VAL_CATEGORY1] )
		{
			TokenExtra->Token = AbsDataVal; 
            TokenExtra->Extra = (i < 0);
		}
		// Extra Bit 1 determines sign, Bit 0 the value
		else if ( AbsDataVal < VP6_DctRangeMinVals[DCT_VAL_CATEGORY2] )
		{
			TokenExtra->Token = DCT_VAL_CATEGORY1; 
            TokenExtra->Extra = (AbsDataVal - VP6_DctRangeMinVals[DCT_VAL_CATEGORY1]);
            TokenExtra->Extra <<=1;
            TokenExtra->Extra |= (i < 0);
		}
		// Extra Bit 2 determines sign, Bit 0-1 the value
		else if ( AbsDataVal < VP6_DctRangeMinVals[DCT_VAL_CATEGORY3] )
		{
			TokenExtra->Token = DCT_VAL_CATEGORY2; 
            TokenExtra->Extra = (AbsDataVal - VP6_DctRangeMinVals[DCT_VAL_CATEGORY2]);
            TokenExtra->Extra <<=1;
            TokenExtra->Extra |= (i < 0);
		}
		// Extra Bit 3 determines sign, Bit 0-2 the value
		else if ( AbsDataVal < VP6_DctRangeMinVals[DCT_VAL_CATEGORY4] )
		{
			TokenExtra->Token = DCT_VAL_CATEGORY3; 
            TokenExtra->Extra = (AbsDataVal - VP6_DctRangeMinVals[DCT_VAL_CATEGORY3]);
            TokenExtra->Extra <<=1;
            TokenExtra->Extra |= (i < 0);
		}
		// Extra Bit 4 determines sign, Bit 0-3 the value
		else if ( AbsDataVal < VP6_DctRangeMinVals[DCT_VAL_CATEGORY5] )
		{
			TokenExtra->Token = DCT_VAL_CATEGORY4; 
            TokenExtra->Extra = (AbsDataVal - VP6_DctRangeMinVals[DCT_VAL_CATEGORY4]);
            TokenExtra->Extra <<=1;
            TokenExtra->Extra |= (i < 0);
		}
		// Extra Bit 5 determines sign, Bit 0-4 the value
		else if ( AbsDataVal < VP6_DctRangeMinVals[DCT_VAL_CATEGORY6] )
		{
			TokenExtra->Token = DCT_VAL_CATEGORY5; 
            TokenExtra->Extra = (AbsDataVal - VP6_DctRangeMinVals[DCT_VAL_CATEGORY5]);
            TokenExtra->Extra <<=1;
            TokenExtra->Extra |= (i < 0);
		}
		// Extra Bit 11 determines sign, Bit 0-10 the value
		else 
		{
			TokenExtra->Token = DCT_VAL_CATEGORY6; 
            TokenExtra->Extra = (AbsDataVal - VP6_DctRangeMinVals[DCT_VAL_CATEGORY6]);
            TokenExtra->Extra <<=1;
            TokenExtra->Extra |= (i < 0);
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : TokenizeFrag
 *
 *  INPUTS        : CP_INSTANCE *cpi     : Pointer to encoder instance.
 *	                INT16 *RawData       : Array of quantized DCT coefficients.
 *	                UINT32 Plane         : Plane block belongs to (Y=0, UV=1)
 *	                BLOCK_CONTEXT *Above : Pointer to an above context.
 *	                BLOCK_CONTEXT *Left  : Pointer to a left context.
 *						
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT16: Index of the EOB token for the block.
 *
 *  FUNCTION      : Takes a set of quantized DCT coefficients for a block
 *                  and produces a set of representative tokens. Each token
 *                  consists of a token identifier and, for most tokens, a 
 *                  set of 'extra-bits'.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
UINT32 TokenizeFrag
(
	CP_INSTANCE *cpi, 
	INT16 *RawData, 
	UINT32 Plane,
	BLOCK_CONTEXT *Above,
	BLOCK_CONTEXT *Left
)
{
    INT32 i;
	UINT32 Token;
	INT32  Offset;
	INT32  ZeroCount;
    INT32  LastNonZeroCoeff=0;          
    
    UINT32 token_pos = 0;
	UINT32 PlaneX = Plane;
	UINT32 PrevTokenIndex;
    UINT32 LastTokenNonZero;


	for ( i=1; i<64; i++ )
	{
        // j is coeff number in zig-zag order
        int j = cpi->pb.ModifiedScanOrder[i];

		if ( RawData[j] )
        {
		    LastNonZeroCoeff = i;
            cpi->FrameNzCount[j][1]++;
        }
		else
			cpi->FrameNzCount[j][0]++;
	}

	// Tokenize the DC value
	if ( RawData[0] )
	{
        if ( cpi->CurrentDcZeroRun[PlaneX] > 0 )
        {
            // Termination of run of zeros in DC positions
            cpi->DcZeroRunStartPtr[PlaneX]->Extra = cpi->CurrentDcZeroRun[PlaneX];
            cpi->CurrentDcZeroRun[PlaneX] = 0;
        }

        Offset = DCT_MAX_VALUE + RawData[0];
		
		Token = DctValueTokens[Offset].Token;
		cpi->CoeffTokenPtr->Token = Token;
		cpi->CoeffTokenPtr->Extra = DctValueTokens[Offset].Extra;

        cpi->FrameDcTokenDist2[Plane][Token]++;
	}
	else
	{
		Token = ZERO_TOKEN;
		cpi->CoeffTokenPtr->Token = Token;
        cpi->CoeffTokenPtr->Extra = 0;

        // Check for run of zeros at DC position (Huffman mode)
        if  ( cpi->CurrentDcZeroRun[PlaneX] == 0 )
        {
            //  New run starts
            cpi->DcZeroRunStartPtr[PlaneX] = cpi->CoeffTokenPtr;
            cpi->FrameDcTokenDist2[Plane][Token]++;
        }
        
        cpi->CurrentDcZeroRun[PlaneX]++;

        if ( cpi->CurrentDcZeroRun[PlaneX] >= 74/*11+63*/ )
        {
            // Maximum run-length is 11+63
            cpi->DcZeroRunStartPtr[PlaneX]->Extra = cpi->CurrentDcZeroRun[PlaneX];
            cpi->CurrentDcZeroRun[PlaneX] = 0;
        }
	}

    cpi->CoeffTokenPtr->LastTokenL = Left->Token;
	cpi->CoeffTokenPtr->LastTokenA = Above->Token;
	cpi->FrameDcTokenDist[Plane][Token]++;
	PrevTokenIndex = VP6_PrevTokenIndex[Token];
	cpi->CoeffTokenPtr++; 
	token_pos++;

	// Update the context
    LastTokenNonZero = (Token != ZERO_TOKEN);
	Left->Token  = LastTokenNonZero;
	Above->Token = LastTokenNonZero;

	// Tokenize the rest of the block 
	for ( i=1; i<=LastNonZeroCoeff; i++ )
	{   
		
        UINT32 Band;        
		ZeroCount = 0;
		while ( !RawData[cpi->pb.ModifiedScanOrder[i]] )
		{
			i++;
			ZeroCount++;

		}
		//  Trap the end of a run of EOBs at AC1
        if ( cpi->CurrentAc1EobRun[PlaneX] > 0 )
        {
            // End of run of EOBs at first AC position
            cpi->Ac1EobRunStartPtr[PlaneX]->Extra = cpi->CurrentAc1EobRun[PlaneX];
            cpi->CurrentAc1EobRun[PlaneX] = 0;
        }

        // Code the zero token and zero run length
		if ( ZeroCount > 0 )
		{
            int ZeroBand;

            Band = VP6_CoeffToBand[token_pos];
			cpi->CoeffTokenPtr->Token = ZERO_TOKEN;
			cpi->CoeffTokenPtr->Extra = ZeroCount - 1;

			cpi->FrameAcTokenDist [PrevTokenIndex][Plane][Band][ZERO_TOKEN]++;
            cpi->FrameAcTokenDist2[PrevTokenIndex][Plane][Band][ZERO_TOKEN]++;

			PrevTokenIndex = VP6_PrevTokenIndex[ZERO_TOKEN];

            // ZeroBand = 0:1
            ZeroBand = (token_pos >= ZRL_BAND2);    

			cpi->FrameZrlDist[ZeroBand][ZeroCount]++;
			cpi->FrameZeroCount[ZeroBand]++;
			
			// Update token_pos 
			token_pos += ZeroCount;

			// Step on to next token
			cpi->CoeffTokenPtr++;
		}

		// Code the non zero value
		Offset = DCT_MAX_VALUE + RawData[cpi->pb.ModifiedScanOrder[i]];
		cpi->CoeffTokenPtr->Token = DctValueTokens[Offset].Token;
		cpi->CoeffTokenPtr->Extra = DctValueTokens[Offset].Extra;
        Band = VP6_CoeffToBand[token_pos];

		cpi->FrameAcTokenDist [PrevTokenIndex][Plane][Band][cpi->CoeffTokenPtr->Token]++;
        cpi->FrameAcTokenDist2[PrevTokenIndex][Plane][Band][cpi->CoeffTokenPtr->Token]++;
		PrevTokenIndex = VP6_PrevTokenIndex [cpi->CoeffTokenPtr->Token];

		cpi->CoeffTokenPtr++; 
		token_pos++;
	}

    // If we have reached the end of the block then code EOB 
    if ( i < BLOCK_SIZE  )
    {
        UINT32 Band;
        cpi->CoeffTokenPtr->Token = DCT_EOB_TOKEN;
        cpi->CoeffTokenPtr->Extra = 0;
        Band = VP6_CoeffToBand[token_pos];
        
        // if EOB at first AC pos
        if ( token_pos == 1 )
        {
            // The start of an EOB run
            if ( cpi->CurrentAc1EobRun[PlaneX] == 0 )
            {
                cpi->Ac1EobRunStartPtr[PlaneX] = cpi->CoeffTokenPtr;
                cpi->FrameAcTokenDist2[PrevTokenIndex][Plane][Band][DCT_EOB_TOKEN]++;
            }
            
            cpi->CurrentAc1EobRun[PlaneX]++;
            
            if ( cpi->CurrentAc1EobRun[PlaneX] >= 74 /*11+63*/ )
            {
                cpi->Ac1EobRunStartPtr[PlaneX]->Extra = cpi->CurrentAc1EobRun[PlaneX];
                cpi->CurrentAc1EobRun[PlaneX] = 0;
            }
        }
        else
        {
            cpi->FrameAcTokenDist2[PrevTokenIndex][Plane][Band][DCT_EOB_TOKEN]++;
        }
        
        cpi->FrameAcTokenDist[PrevTokenIndex][Plane][Band][DCT_EOB_TOKEN]++;
        PrevTokenIndex = VP6_PrevTokenIndex [DCT_EOB_TOKEN];
        
        cpi->CoeffTokenPtr++;
        token_pos++;
        
    }


	token_pos--;

    // Return the position of the last token. 
	return cpi->pb.EobOffsetTable[token_pos];
}

/****************************************************************************
*
*	Rate Distortion Specific Code...
*
****************************************************************************/

/****************************************************************************
 * 
 *  ROUTINE       : TokenCost_RD
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *	                UINT8 Token      : Token to be costed.
 *	                int Band         : Band that the token belongs in.
 *	                UINT8 Plane      : Plane that the token belogs in.
 *	                UINT8 PrecCase   : Previous token context type.
 *						
 *  OUTPUTS       : None.
 *
 *  RETURNS       : UINT32: Estimated cost in bits of coding this token.
 *
 *  FUNCTION      : Produces an estimate of the cost, i.e. number of bits
 *                  required to code, the token using statistics derived
 *                  from the distribution of tokens in the previous frame.
 *
 *  SPECIAL NOTES : None.
 *
 ***************************************************************************/
UINT32 TokenCost_RD ( CP_INSTANCE *cpi, UINT8 Token, int Band, UINT8 Plane, UINT8 PrecCase )
{
    if ( Band == -1 ) 
		return cpi->EstDcTokenCosts[Plane][Token] + (ExtraBitLengths_VP6[Token] << 6);
    else
        return cpi->EstAcTokenCosts[PrecCase][Plane][Band][Token] + (ExtraBitLengths_VP6[Token] << 6);
}         
/****************************************************************************
 * 
 *  ROUTINE       : TokenizeFrag_RD
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *	                INT16 *RawData   : Array of quantized DCT coeffs to be tokenized.
 *	                UINT32 Plane     : Plane that the block belongs to.
 *	                						
 *  OUTPUTS       : UINT32 *MbCost   : Pointer to variable that will hold the 
 *                                     cost of tokenizing the block.
 *
 *  RETURNS       : UINT8: Estimated cost in bits of coding this token.
 *
 *  FUNCTION      : Cut down RD version of tokenize function of tokenize block
 *					that does not update all the context stuff.
 *
 *  SPECIAL NOTES :     
 *
 ****************************************************************************/
UINT8 TokenizeFrag_RD
(
	CP_INSTANCE *cpi, 
	INT16 *RawData, 
	UINT32 Plane,
    UINT32 *MbCost
)
{
    UINT32	i;
	UINT8   Token;
	INT32	ZeroCount;

    UINT8   TokenPos = 1;	
    INT32   Band;
    INT32   PrevTokenCase ;
    

	// Tokenize the DC value	
    Token = DctValueTokens[DCT_MAX_VALUE + RawData[0]].Token;
    *MbCost += cpi->EstDcTokenCosts[Plane][Token] + (ExtraBitLengths_VP6[Token] << 6);    
	PrevTokenCase =VP6_PrevTokenIndex[Token];    


	// Tokenize the rest of the block 
	for ( i=1; i<BLOCK_SIZE; i++ )
	{   
		// Test for EOB condition 
		ZeroCount = 0;
		while ( !RawData[cpi->pb.ModifiedScanOrder[i]] && (i < BLOCK_SIZE) )
		{
			i++;
			ZeroCount++;
		}

		// If we have reached the end of the block then code EOB 
		if ( i == BLOCK_SIZE  )
		{
			Token = DCT_EOB_TOKEN;
            Band  = VP6_CoeffToBand[TokenPos];
		    *MbCost += cpi->EstAcTokenCosts[PrevTokenCase][Plane][Band][Token] + (ExtraBitLengths_VP6[Token] << 6);             
            PrevTokenCase =VP6_PrevTokenIndex[Token];    
			TokenPos++;
		}
		else
		{
			INT32 Offset = DCT_MAX_VALUE + RawData[cpi->pb.ModifiedScanOrder[i]];

			if ( ZeroCount > 0 )
			{
                //0:1
				UINT8 ZBand = (TokenPos >= ZRL_BAND2);

				Token = ZERO_TOKEN;
                Band  = VP6_CoeffToBand[TokenPos];
		        *MbCost += cpi->EstAcTokenCosts[PrevTokenCase][Plane][Band][Token] + (ExtraBitLengths_VP6[Token] << 6);             
                PrevTokenCase =VP6_PrevTokenIndex[Token];
				TokenPos += ZeroCount;

				// Get estimated cost of zero run bits (based upon previous frame stats
				*MbCost += cpi->EstZrlCosts[ZBand][ZeroCount];
			}

			Token = DctValueTokens[Offset].Token;
            Band  = VP6_CoeffToBand[TokenPos];
            *MbCost += cpi->EstAcTokenCosts[PrevTokenCase][Plane][Band][Token] + (ExtraBitLengths_VP6[Token] << 6);             
            PrevTokenCase =VP6_PrevTokenIndex[Token];
            TokenPos++;
		}
	}

	TokenPos--;

    // Return the position of the last token. 
    return TokenPos;
}
