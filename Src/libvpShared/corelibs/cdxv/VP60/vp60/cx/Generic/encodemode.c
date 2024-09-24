/****************************************************************************
*        
*   Module Title :	   encodemode.c
*
*   Description  :     Functions for encoding modes and motion vectors.
*
****************************************************************************/
#define STRICT              /* Strict type checking */
 
/****************************************************************************
*  Header Files
****************************************************************************/
#include <math.h>           // For abs()
#include "compdll.h"
#include "boolhuff.h"
#include "decodemode.h"
#include "encodemv.h"
#include "decodemv.h"

/****************************************************************************
*  Explicit Imports
****************************************************************************/        
extern void AddBitsToBuffer( BOOL_CODER *bc, UINT32 data, UINT32 bits );

/****************************************************************************
*  Module statics.
****************************************************************************/        
static const HNODE CodingMode[9] =
{
	{ // 0 
		{	0,	1	},   
		{   0,	2	},
	},
	{ // 1
		{	0,	3	},
		{   0,	4	},
	},
	{ // 2
		{	0,	5	},
		{   0,	6	},
	},
	{ // 3
		{	1,	CODE_INTER_NO_MV	},
		{   1,	CODE_INTER_PLUS_MV	},
	},
	{ // 4
		{	1,	CODE_INTER_NEAREST_MV	},
		{   1,	CODE_INTER_NEAR_MV	},
	},
	{ // 5 
		{	1,	CODE_INTRA	},
		{   1,	CODE_INTER_FOURMV	},
	},
	{ // 6
		{	0,	7	},
		{   0,	8	},
	},
	{ // 7
		{	1,	CODE_USING_GOLDEN	},
		{   1,	CODE_GOLDEN_MV	},
	},
	{ // 8
		{	1,	CODE_GOLD_NEAREST_MV},
		{   1,	CODE_GOLD_NEAR_MV	},
	},
};

// NOTE: 
// ModeCodeArray contains information required to traverse a binary
// decision tree for coding the coding mode. The form of the tree is
// documented in decodemode.c. Each entry corresponds to a decision
// as to whether to take the 0 or one branch at a particular node.
// An entry whose value is 9 indicates that we have reached a leaf node.
// Each row corresponds to the value of the previously coded mode
// and each column to the succesive node decisions.
static const UINT32 ModeCodeArray[MAX_MODES][7] =
{
	0, 0, 0, 9, 9, 9, 9,   // CODE_INTER_NO_MV		
	1, 0, 0, 9, 9, 9, 9,   // CODE_INTRA				
	0, 0, 1, 9, 9, 9, 9,   // CODE_INTER_PLUS_MV		
    0, 1, 0, 9, 9, 9, 9,   // CODE_INTER_NEAREST_MV	
	0, 1, 1, 9, 9, 9, 9,   // CODE_INTER_NEAR_MV		
	1, 1, 0, 0, 9, 9, 9,   // CODE_USING_GOLDEN		
	1, 1, 0, 1, 9, 9, 9,   // CODE_GOLDEN_MV			
	1, 0, 1, 9, 9, 9, 9,   // CODE_INTER_FOURMV		
    1, 1, 1, 0, 9, 9, 9,   // CODE_GOLD_NEAREST_MV	
	1, 1, 1, 1, 9, 9, 9    // CODE_GOLD_NEAR_MV		
};

/****************************************************************************
 * 
 *  ROUTINE       :     encodeBlockMode
 *
 *  INPUTS        :     CP_INSTANCE *cpi : Pointer to encoder instance.
 *                      CODING_MODE mode : Mode we are trying to encode.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Encodes a block mode into the bitstream using 2 bits.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void encodeBlockMode ( CP_INSTANCE *cpi, CODING_MODE mode )
{
	int choice = 0;

// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = MODE_SECTION;
#endif

	switch ( mode )
	{
	case CODE_INTER_NO_MV:      choice = 0; break;
	case CODE_INTER_PLUS_MV:    choice = 1; break;
	case CODE_INTER_NEAREST_MV: choice = 2; break;
	case CODE_INTER_NEAR_MV:    choice = 3; break;
	} 
	AddBitsToBuffer ( &cpi->bc, choice, 2 );				
}

/****************************************************************************
 * 
 *  ROUTINE       :     encodeMode
 *
 *  INPUTS        :     CP_INSTANCE *cpi     : Pointer to encoder instance.
 *                      CODING_MODE lastmode : Mode of the last coded macroblock.
 *                      CODING_MODE mode     : Mode we are trying to encode.
 *                      UINT32 type          : MODE_TYPE (all modes available, nonearest
 *                                             no near macroblock)
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Encodes coding mode for MB into the bitstream using a tree
 *                      traversal algorithm: 
 *                      -- First decision is whether mode==lastmode: code a 0 or 1
 *                         using probability from probModeSame.
 *                      -- If mode!=lastmode step down the tree using ModeCodeArray
 *                         to decide whether to code a 0 or 1 decision at each node,
 *                         and probMode to determine the probability of coding a 0 
 *                         decision (1 decision probability is then computed as
 *                         (1 minus zero-decision-prob)).
 *
 *  SPECIAL NOTES :     Uses VP6_EncodeBool to encode the bits to the bitstream.
 *
 ****************************************************************************/
void encodeMode ( CP_INSTANCE *cpi, CODING_MODE lastmode, CODING_MODE mode, UINT32 type )
{
    UINT8 Stat;
	UINT8 i = 0;
    UINT8 node = 0;

// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = MODE_SECTION;
#endif

	if ( mode == lastmode ) 
	{
		VP6_EncodeBool ( &cpi->bc, 1, cpi->pb.probModeSame[type][lastmode] );
	}
	else
	{
		VP6_EncodeBool(	&cpi->bc, 0, cpi->pb.probModeSame[type][lastmode] );
		
		while ( ModeCodeArray[mode][i] != 9 )
		{
			Stat = cpi->pb.probMode[type][lastmode][node];
			
			VP6_EncodeBool ( &cpi->bc, ModeCodeArray[mode][i], (int)Stat );
			
			if ( ModeCodeArray[mode][i] == 0 )
				node = CodingMode[node].left.value;
			else
				node = CodingMode[node].right.value;
			i++;
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     encodeModeTest
 *
 *  INPUTS        :     CP_INSTANCE *cpi     : Pointer to encoder instance.
 *                      CODING_MODE lastmode : Mode of the last coded macroblock.
 *                      CODING_MODE mode     : Mode we are trying to encode.
 *                      UINT32 type          : MODE_TYPE (all modes available, nonearest
 *                                             no near macroblock)
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Exactly the same functionality as encodeMode above,
 *                      _but_ rather than outputting bits to the bitstream,
 *                      BitCounter in cpi->pb in incremented by an estimate
 *                      of the number of bits required.
 *
 *  SPECIAL NOTES :     Uses VP6_EncodeBool2 to get an estimate of the number
 *                      of bits that will be generated. 
 *
 ****************************************************************************/
void encodeModeTest ( CP_INSTANCE *cpi, CODING_MODE lastmode, CODING_MODE mode, UINT32 type )
{
    UINT8 Stat;
	UINT8 i = 0;
    UINT8 node = 0;

	if ( mode==lastmode ) 
	{
		VP6_EncodeBool2 ( &cpi->bc, 1, cpi->pb.probModeSame[type][lastmode] );
	}
	else
	{
		VP6_EncodeBool2 ( &cpi->bc, 0, cpi->pb.probModeSame[type][lastmode] );
		
		while ( ModeCodeArray[mode][i] != 9 )
		{
			Stat = cpi->pb.probMode[type][lastmode][node];
			
			VP6_EncodeBool2 ( &cpi->bc, ModeCodeArray[mode][i], (int)Stat );
			
			if ( ModeCodeArray[mode][i] == 0 )
				node = CodingMode[node].left.value;
			else
				node = CodingMode[node].right.value;
			i++;
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : encodeModeDiff
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *                  int diff         : Probability difference value to encode.
 *						
 *  OUTPUTS       : None.    
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Takes a differential probability value in the range 
 *                  -256 to +256 in steps of 4 and encodes it using a fixed
 *                  tree with hard-coded probabilities.
 *
 *  SPECIAL NOTES : The hard coded probabilities for the difference tree
 *                  were calcualated by taking the average number of times a 
 *                  branch was taken on some sample material i.e. 
 *                  (bond, bike, beautifulmind).
 *
 ****************************************************************************/
void encodeModeDiff ( CP_INSTANCE *cpi, int diff )
{
	if ( diff==0 )
	{
		// 0 difference
		VP6_EncodeBool ( &cpi->bc, 0, 205 );
	}
	else
	{
		// Non-0 
		VP6_EncodeBool ( &cpi->bc, 1, 205 );

		// transmit sign of difference 
		VP6_EncodeBool ( &cpi->bc, diff<0, 128 );

		// go to abs value
		diff = abs(diff);

		if ( diff<12 )
		{
			VP6_EncodeBool ( &cpi->bc, 0, 171 );
			VP6_EncodeBool ( &cpi->bc, diff==4, 83 );
		}
		else
		{
			VP6_EncodeBool ( &cpi->bc, 1, 171 );

			if ( diff<28 ) 
			{
				VP6_EncodeBool ( &cpi->bc, 0, 199 );
				VP6_EncodeBool ( &cpi->bc, diff==12, 140 );
				if ( diff>12 ) 
				{
					VP6_EncodeBool ( &cpi->bc, diff==16, 125 );
					if ( diff>16 )
						VP6_EncodeBool ( &cpi->bc, diff==20, 104 );
				}
			}
			else 
			{
				VP6_EncodeBool ( &cpi->bc, 1, 199 );
				AddBitsToBuffer ( &cpi->bc, diff>>2, 7 );
			}
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : estimateModeDiffCost
 *
 *  INPUTS        : int diff : Probability difference value to encode.
 *						
 *  OUTPUTS       : None.   
 *
 *  RETURNS       : UINT32: Number of bits required to code diff.    
 *
 *  FUNCTION      : Same as encodeModeDiff above but rather than outputting
 *                  bits to the bitstream it estimates the number of bits
 *                  that will be generated.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
int estimateModeDiffCost ( int diff )
{
	int cost = 0;

	if ( diff==0 )
	{
		cost += (VP6_ProbCost[205]+128) >> 8;
	}
	else
	{
		cost += (VP6_ProbCost[255-205]+128) >> 8;
		cost += 64;

		// go to abs value
		diff = abs(diff);
		if ( diff<12 )
		{
			// < 12
    		cost += (VP6_ProbCost[171]+128) >> 8;

			if ( diff==4 )
        		cost += (VP6_ProbCost[255-83]+128) >> 8;
			else
        		cost += (VP6_ProbCost[83]+128) >> 8;
		}
		else
		{
			// >= 12
    		cost += (VP6_ProbCost[255-171]+128) >> 8;

			if ( diff<28 ) 
			{
				// < 28
          		cost += (VP6_ProbCost[199]+128) >> 8;

				if ( diff==12 )
            		cost += (VP6_ProbCost[255-140]+128) >> 8;
				else
				{
    		        cost += (VP6_ProbCost[140]+128) >> 8;

					if ( diff==16 )
                		cost += (VP6_ProbCost[255-125]+128) >> 8;
					else
					{
                		cost += (VP6_ProbCost[125]+128) >> 8;
						if ( diff==20 )
                    		cost += (VP6_ProbCost[255-104]+128) >> 8;
						else
                    		cost += (VP6_ProbCost[104]+128) >> 8;
					}
				}
			}
			else 
			{
				// >= 28 just send the bits
        		cost += (VP6_ProbCost[255-199]+128) >> 8;
				cost += 7*64;
			}
        }
    }
	return cost;
}

/****************************************************************************
 * 
 *  ROUTINE       : UpdateModeProbs
 *
 *  INPUTS        : CP_INSTANCE *cpi : Pointer to encoder instance.
 *						
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void  
 *
 *  FUNCTION      : Determines which probabilities to transmit and 
 *                  use for encoding macroblock modes, and then 
 *                  transmits the information necessary to decode the 
 *                  probabilities.
 *
 *                  a) Pick the lowest cost vector we have available
 *                  b) Compare it to what we used in the last frame
 *                  c) Determine if it makes sense to update the vector
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void UpdateModeProbs ( CP_INSTANCE *cpi )
{
	int i, j, k;
	int diff;
    int costToIdeal;
	unsigned int thisCost, bestCost;
	unsigned int lowestCost = 0x7fffffff;
    unsigned int lastCost   = 0x7fffffff; 
	unsigned int whichVector = 0;
	UINT32 total, round; 
	UINT8 proposedProb[2][10];
	PB_INSTANCE *pbi = &cpi->pb;

// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = MODE_SECTION;
#endif

	// For each mode type (all modes available, no nearest, no near mode)
	for ( j=0; j<MODETYPES; j++ )
	{
		// sum the totals for each of the modes
		cpi->MBModeCount[j][MAX_MODES]         = 0;
		cpi->CountModeSameAsLast[j][MAX_MODES] = 0;
		for ( i=0; i<MAX_MODES; i++ )
		{
			cpi->MBModeCount[j][MAX_MODES]         += cpi->MBModeCount[j][i];
			cpi->CountModeSameAsLast[j][MAX_MODES] += cpi->CountModeSameAsLast[j][i];
			cpi->CountModeDiffFrLast[j][MAX_MODES] += cpi->CountModeDiffFrLast[j][i];
		}

		// estimate the cost of using the cheapest vector from our vq codebook 
		whichVector = 0;
		lowestCost  = 0x7fffffff;
		for ( k=0; k<MODEVECTORS; k++ )
		{
			thisCost = 0;
			for ( i=0; i<MAX_MODES; i++ )
			{
				thisCost += cpi->CountModeSameAsLast[j][i]*((VP6_ProbCost[VP6_ModeVq[j][k][i*2]]  +128)>>8);
				thisCost += cpi->CountModeDiffFrLast[j][i]*((VP6_ProbCost[VP6_ModeVq[j][k][i*2+1]]+128)>>8);
			}
			if ( thisCost<lowestCost )
			{
				whichVector = k;
				lowestCost  = thisCost;
			}
		}

		// In the error resilliant mode / VC mode we discount the "last frame values" as 
		// a candidate vector in order to improve the resilliance to dropped/corrupt frames.
		if ( !cpi->ErrorResilliantMode )
		{
			// estimate the cost of using the vector we have from the last frame
			lastCost = 0;
			for ( i=0; i<MAX_MODES; i++ )
			{
				lastCost += cpi->CountModeSameAsLast[j][i]*((VP6_ProbCost[pbi->probXmitted[j][1][i]] + 128)>>8);
				lastCost += cpi->CountModeDiffFrLast[j][i]*((VP6_ProbCost[pbi->probXmitted[j][0][i]] + 128)>>8);
			}
		}

		// if the best from our vq book + the cost of transmitting the vector is cheaper
		// than our current vector use it. OR... if we are running in error resilliant mode.
		if( cpi->ErrorResilliantMode ||
			 ( (lastCost / 64) > (((VP6_ProbCost[255-PROBVECTORXMIT]+128)>>8) + lowestCost) / 64 + 4 ) /* for the vector itself */ ) 
		{
			// transmit that we are transmitting a new vector 
			VP6_EncodeBool ( &cpi->bc,1,PROBVECTORXMIT );

			// transmit which vector to use here
			AddBitsToBuffer ( &cpi->bc, whichVector, 4 );				

			// adjust the vector
			for ( i=0; i<MAX_MODES; i++ )
			{
				pbi->probXmitted[j][1][i] = VP6_ModeVq[j][whichVector][i*2];
				pbi->probXmitted[j][0][i] = VP6_ModeVq[j][whichVector][i*2+1];
			}
		}
		else 
		{
			lowestCost = lastCost;

			// transmit that we are reusing the last vector
			VP6_EncodeBool ( &cpi->bc, 0, PROBVECTORXMIT );
		}

		// calculate the ideal vector and how much it would cost to go to it.
		bestCost    = 0;
		costToIdeal = 0;
		total = 1 + cpi->CountModeSameAsLast[j][MAX_MODES]+cpi->CountModeDiffFrLast[j][MAX_MODES];
		round = total/2;
		for ( i=0; i<10; i++ )
		{
			// what's the ideal probability
			proposedProb[1][i] = (round+256*cpi->CountModeSameAsLast[j][i]) / total;

			// calculate the truncated difference between the ideal and where we are now
			diff = 4*((proposedProb[1][i] - pbi->probXmitted[j][1][i]) / 4);
			costToIdeal += estimateModeDiffCost(diff);
			diff += pbi->probXmitted[j][1][i];
			proposedProb[1][i] = ( diff<0 ? 0 : (diff>255 ? 255 : diff) );

			// update the cost of our ideal choice and of moving to our ideal values
			bestCost += cpi->CountModeSameAsLast[j][i]*((VP6_ProbCost[proposedProb[1][i]]+128)>>8);

			// what's the ideal probability
			proposedProb[0][i] = (round+256*cpi->CountModeDiffFrLast[j][i]) / total;

			// calculate the truncated difference between the ideal and where we are now 
			diff = 4*((proposedProb[0][i] - pbi->probXmitted[j][0][i]) / 4);
			costToIdeal += estimateModeDiffCost(diff);
			diff += pbi->probXmitted[j][0][i];
			proposedProb[0][i] = ( diff<0 ? 0 : (diff>255 ? 255 : diff) );

			// update the cost of our ideal choice and of moving to our ideal values
			bestCost += cpi->CountModeDiffFrLast[j][i]*((VP6_ProbCost[proposedProb[0][i]]+128)>>8);
		}

		// if updating our vector to be closer to the ideal is cheaper than going with what we have now
		if ( (costToIdeal + bestCost + ((VP6_ProbCost[255-PROBIDEALXMIT]+128)>>8)) / 64 < lowestCost / 64 )
		{
			// transmit that we are updating the mode probabilities
			VP6_EncodeBool ( &cpi->bc, 1, PROBIDEALXMIT );

			// encode the differences and adjust the ideal values
			for ( i=0; i<10; i++ )
			{
				diff = proposedProb[1][i]-pbi->probXmitted[j][1][i];
				encodeModeDiff(cpi,diff);
				diff += pbi->probXmitted[j][1][i];
				pbi->probXmitted[j][1][i] = ( diff<0 ? 0 : (diff>255 ? 255 : diff) );

				diff = proposedProb[0][i]- pbi->probXmitted[j][0][i];
				encodeModeDiff(cpi,diff);
				diff += pbi->probXmitted[j][0][i];
				pbi->probXmitted[j][0][i] = ( diff<0 ? 0 : (diff>255 ? 255 : diff) );
			}
		}
		else
		{
			// transmit that we are not updating the mode probabilities
			VP6_EncodeBool ( &cpi->bc, 0, PROBIDEALXMIT );
		}
	}
	
	VP6_BuildModeTree ( &cpi->pb );
}

/****************************************************************************
 * 
 *  ROUTINE       :     encodeModeandMotionVector
 *
 *  INPUTS        :     CP_INSTANCE *cpi  : Pointer to encoder instance.
 *                      UINT32 MBrow      : MB row.
 *                      UINT32 MBcol      : MB column.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Encodes a macroblock's mode and motion vectors to 
 *                      the bitstream.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void encodeModeAndMotionVector ( CP_INSTANCE *cpi, UINT32 MBrow, UINT32 MBcol )
{
	UINT32 k;
	int type, type2;
	CODING_MODE mode;
	UINT32 FragsToCheck[4];
	PB_INSTANCE *pbi = &cpi->pb;
 	int width  = pbi->HFragments;
	UINT32 FragIndex = (MBrow-BORDER_MBS)*width*2 + (MBcol-BORDER_MBS)*2;

	VP6_FindNearestandNextNearest ( &cpi->pb, MBrow, MBcol, 1, &type  );
	VP6_FindNearestandNextNearest ( &cpi->pb, MBrow, MBcol, 2, &type2 );
	
	FragsToCheck[0] = FragIndex;
	FragsToCheck[1] = FragIndex+1;
	FragsToCheck[2] = FragIndex+pbi->HFragments;
	FragsToCheck[3] = FragIndex+pbi->HFragments+1;
	
	mode = pbi->predictionMode[MBOffset(MBrow,MBcol)];

	encodeMode ( cpi, pbi->LastMode, mode, type );
    pbi->LastMode = mode;

	// check to see if we need to encode mvs or more sub modes
	switch ( mode )
	{
	case CODE_INTER_PLUS_MV:
		encodeMotionVector ( cpi, pbi->FragInfo[FragIndex].MVectorX, pbi->FragInfo[FragIndex].MVectorY, mode );
		break;

	case CODE_GOLDEN_MV:
		encodeMotionVector ( cpi, pbi->FragInfo[FragIndex].MVectorX, pbi->FragInfo[FragIndex].MVectorY, mode);
		break;

	case CODE_INTER_FOURMV:
		// encode sub mode decisions
		encodeBlockMode ( cpi, pbi->FragInfo[FragsToCheck[0]].FragCodingMode );
		encodeBlockMode ( cpi, pbi->FragInfo[FragsToCheck[1]].FragCodingMode );
		encodeBlockMode ( cpi, pbi->FragInfo[FragsToCheck[2]].FragCodingMode );
		encodeBlockMode ( cpi, pbi->FragInfo[FragsToCheck[3]].FragCodingMode );

		// encode the 4 motion vectors
		for ( k=0; k<4; k++ )
			if ( pbi->FragInfo[FragsToCheck[k]].FragCodingMode==CODE_INTER_PLUS_MV )
				encodeMotionVector ( cpi, pbi->FragInfo[FragsToCheck[k]].MVectorX, pbi->FragInfo[FragsToCheck[k]].MVectorY, CODE_INTER_PLUS_MV );
		break;
	}
}


/****************************************************************************
 * 
 *  ROUTINE       :     VP6_EstimateCost
 *
 *  INPUTS        :     BOOL_CODER *bc : Pointer to a BoolCoder (UNUSED). 
 *                      HUFF_NODE *hn  : Pointer to a Huffman tree.
 *                      int value      : Value to be encoded.
 *                      int length     : Length in bits of value.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     UINT32: Cost of coding value (in bits).
 *
 *  FUNCTION      :     Computes the cost of coding value bit-by-bit using
 *                      the Huffman tree specified.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
UINT32 VP6_EstimateCost ( BOOL_CODER *bc, HUFF_NODE *hn, int value, int length )
{
    int i;
    int node = 0;
	UINT32 total = 0;

    for ( i=length-1; i>=0; i-- )
    {
        int v = (value>>i) & 1;

        if ( v )
        {
			total += (VP6_ProbCost[255-hn[node].freq]+128)>>8;
            node  = hn[node].rightunion.right.value;
        }
        else
        {
			total += (VP6_ProbCost[hn[node].freq]+128)>>8;
            node  = hn[node].leftunion.left.value;
        }
    }
	return total; 
}

/****************************************************************************
 * 
 *  ROUTINE       :     modeCost
 *
 *  INPUTS        :     CP_INSTANCE *cpi : Pointer to encoder instance.
 *                      UINT32 MBrow     : MB row.
 *                      UINT32 MBcol     : MB column.
 *                      CODING_MODE mode : Mode to be costed.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     UINT32: Cost of coding mode (in bits*64).
 *
 *  FUNCTION      :     Computes the cost of coding mode (in bits*64).
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
UINT32 modeCost ( CP_INSTANCE *cpi, UINT32 MBrow, UINT32 MBcol, CODING_MODE mode )
{
	int type;
	CODING_MODE lastmode;
	PB_INSTANCE *pbi = &cpi->pb;
 	int width  = pbi->HFragments;

	VP6_FindNearestandNextNearest ( &cpi->pb, MBrow, MBcol, 1, &type );

	if ( MBcol==BORDER_MBS && MBrow==BORDER_MBS )
		lastmode = CODE_INTER_NO_MV;
	else if ( MBcol==BORDER_MBS )
		lastmode = pbi->predictionMode[MBOffset(MBrow-1,pbi->MBCols - (BORDER_MBS+1))];
	else 
		lastmode = pbi->predictionMode[MBOffset(MBrow,MBcol-1)];

	return cpi->EstModeCost[(lastmode==mode) ? 0 : 1][mode];
}

/****************************************************************************
 * 
 *  ROUTINE       :     blockModeCost
 *
 *  INPUTS        :     CP_INSTANCE *cpi : Pointer to encoder instance (NOT USED).
 *                      UINT32 i         : Undefined (NOT USED).
 *                      UINT32 j         : Undefined (NOT USED).
 *                      CODING_MODE mode : Mode to be costed (NOT USED).
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     UINT32: Cost of coding mode (in bits*64).
 *
 *  FUNCTION      :     Computes the cost of coding mode (in bits*64).
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
UINT32 blockModeCost ( CP_INSTANCE *cpi, UINT32 i, UINT32 j, CODING_MODE mode )
{
    // All modes within 4 mode mode cost 2 bits (cost specified as bits * 64)
	return 128;
}

/****************************************************************************
 * 
 *  ROUTINE       :     BuildModeCostEstimates
 *
 *  INPUTS        :     CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void.
 *
 *  FUNCTION      :     Compute an estimate of the cost of encoding each mode.
 *
 *  SPECIAL NOTES :     None. 
 *
 ***************************************************************************/
void BuildModeCostEstimates ( CP_INSTANCE *cpi )
{
	int i;

	for ( i=0; i<MAX_MODES; i++ )
	{
		cpi->bc.BitCounter = 0;
		encodeModeTest ( cpi, i, i, MACROBLOCK );
		cpi->EstModeCost[0][i] = (cpi->bc.BitCounter) >> 2;	

		// Non matching last mode case
		cpi->bc.BitCounter = 0;
		if ( i==0 )
			encodeModeTest ( cpi, 1, i, MACROBLOCK );
		else
			encodeModeTest ( cpi, 0, i, MACROBLOCK );
		cpi->EstModeCost[1][i] = (cpi->bc.BitCounter) >> 2;	
	}
}
