/****************************************************************************
*        
*   Module Title :	   Decodemv.c     
*
*   Description  :     functions for decoding modes and motionvectors 
*
*   AUTHOR       :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
*   1.00 JBB 30OCT01  New Configuration baseline.
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/
#include "pbdll.h"
#include "boolhuff.h"
#include "huffman.h"
#include "stdio.h"
#include "decodemode.h" 
#include "decodemv.h"

/****************************************************************************
*  Implicit Imports
*****************************************************************************
*/        
#define STRICT              /* Strict type checking. */

#ifdef MAPCA
    #include <eti/mm.h>
#endif

 

/****************************************************************************
*  Exported data structures.
*****************************************************************************
*/        


/****************************************************************************
*  Module statics.
*****************************************************************************
*/        

UINT8 MvUpdateProbs[2][MV_NODES] = 
{ 
	{ 243, 220, 251, 253, 237, 232, 241, 245, 247, 251, 253 },
	{ 235, 211, 246, 249, 234, 231, 248, 249, 252, 252, 254 }
};

/****************************************************************************
 * 
 *  ROUTINE       :     ConfigureMvEntropyDecoder
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     Build the MV entropy decoding tree
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
***************************************************************************/
void ConfigureMvEntropyDecoder( PB_INSTANCE *pbi, UINT8 FrameType )
{
	int i;
    
	//This funciton is not called at all if it is a BASE_FRAME
    /*    
	if ( FrameType == BASE_FRAME)
	{
		// Set up the default values for each of the MV probabilities
		// For now these are just 128
		memset ( pbi->MvSignProbs, 128, sizeof(pbi->MvSignProbs) );
		memset ( pbi->MvZeroProbs, 128, sizeof(pbi->MvZeroProbs) );
		memset ( pbi->MvHalfPixelProbs, DEFAULT_HALF_PIXEL_PROB, sizeof(pbi->MvHalfPixelProbs) );
		memset ( pbi->MvLowBitProbs, 128, sizeof(pbi->MvLowBitProbs) );
		memset ( pbi->MvSizeProbs, 128, sizeof(pbi->MvSizeProbs) );
	}
	else
    */
	{
		// Calculate and if necessary send the Zero, sign, half pixel and Low order probabilities.
		for ( i = 0; i < 2; i++ )
		{
			// Zero probability
			if ( DecodeBool(&pbi->br, MvUpdateProbs[i][0]) )
			{
				pbi->MvZeroProbs[i] = VP5_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				if ( pbi->MvZeroProbs[i] == 0 )
					pbi->MvZeroProbs[i] = 1;
			}

			// Sign probability
			if ( DecodeBool(&pbi->br, MvUpdateProbs[i][1]) )
			{
				pbi->MvSignProbs[i] = VP5_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				if ( pbi->MvSignProbs[i] == 0 )
					pbi->MvSignProbs[i] = 1;
			}

			// Half pixel bit probability
			if ( DecodeBool(&pbi->br, MvUpdateProbs[i][2]) )
			{
				pbi->MvHalfPixelProbs[i] = VP5_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				if ( pbi->MvHalfPixelProbs[i] == 0 )
					pbi->MvHalfPixelProbs[i] = 1;
			}

			// Low order magnitude bit Probability
			if ( DecodeBool(&pbi->br, MvUpdateProbs[i][3]) )
			{
				pbi->MvLowBitProbs[i] = VP5_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				if ( pbi->MvLowBitProbs[i] == 0 )
					pbi->MvLowBitProbs[i] = 1;
			}
		}

		// Now vector magnitude Probabilities
		for ( i = 0; i < 2; i++ )
		{
			if ( DecodeBool(&pbi->br, MvUpdateProbs[i][4]) )
			{
				pbi->MvSizeProbs[i][0] = VP5_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				if ( pbi->MvSizeProbs[i][0] == 0 )
					pbi->MvSizeProbs[i][0] = 1;
			}

			if ( DecodeBool(&pbi->br, MvUpdateProbs[i][5]) )
			{
				pbi->MvSizeProbs[i][1] = VP5_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				if ( pbi->MvSizeProbs[i][1] == 0 )
					pbi->MvSizeProbs[i][1] = 1;
			}
			
			if ( DecodeBool(&pbi->br, MvUpdateProbs[i][6]) )
			{
				pbi->MvSizeProbs[i][2] = VP5_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				if ( pbi->MvSizeProbs[i][2] == 0 )
					pbi->MvSizeProbs[i][2] = 1;
			}
			
			if ( DecodeBool(&pbi->br, MvUpdateProbs[i][7]) )
			{
				pbi->MvSizeProbs[i][3] = VP5_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				if ( pbi->MvSizeProbs[i][3] == 0 )
					pbi->MvSizeProbs[i][3] = 1;
			}
			
			if ( DecodeBool(&pbi->br, MvUpdateProbs[i][8]) )
			{
				pbi->MvSizeProbs[i][4] = VP5_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				if ( pbi->MvSizeProbs[i][4] == 0 )
					pbi->MvSizeProbs[i][4] = 1;
			}

			if ( DecodeBool(&pbi->br, MvUpdateProbs[i][9]) )
			{
				pbi->MvSizeProbs[i][5] = VP5_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				if ( pbi->MvSizeProbs[i][5] == 0 )
					pbi->MvSizeProbs[i][5] = 1;
			}

			if ( DecodeBool(&pbi->br, MvUpdateProbs[i][10]) )
			{
				pbi->MvSizeProbs[i][6] = VP5_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				if ( pbi->MvSizeProbs[i][6] == 0 )
					pbi->MvSizeProbs[i][6] = 1;
			}
		}
	}
}


/****************************************************************************
 * 
 *  ROUTINE       :     decodeMotionVector 
 *
 *  INPUTS        :     *mv -> returned motion vector
						*nearestMv -> passed in mv acting as context 
						
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     decodes a motion vector from the bitstream 
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void decodeMotionVector
(
	PB_INSTANCE *pbi,
	MOTION_VECTOR *mv,
	MOTION_VECTOR *nearestMv
)
{
	UINT32 i;
	INT32  Vector = 0;
	INT32  SignBit;
	INT32  HpBit;
	INT32  LowBit;

	for ( i = 0; i < 2; i++ )
	{
		Vector = 0;

		// Is the vector non-zero
		if ( DecodeBool(&pbi->br, pbi->MvZeroProbs[i]) )
		{
			// Read the sign, half pixel and low order bits
			SignBit = DecodeBool(&pbi->br, pbi->MvSignProbs[i]);

			// Read half pixel and low order bits
			HpBit = DecodeBool(&pbi->br, pbi->MvHalfPixelProbs[i]);
			LowBit = DecodeBool(&pbi->br, pbi->MvLowBitProbs[i]);

			// Now read the magnitude bits
			if ( DecodeBool(&pbi->br, pbi->MvSizeProbs[i][0] ) )
			{
				Vector = 1 << 4;
				if ( DecodeBool(&pbi->br, pbi->MvSizeProbs[i][4]) )
				{
					Vector |= (1 << 3);
					Vector |= DecodeBool(&pbi->br, pbi->MvSizeProbs[i][6]) << 2;
				}
				else
				{
					Vector |= DecodeBool(&pbi->br, pbi->MvSizeProbs[i][5]) << 2;
				}
			}
			else
			{
				if ( DecodeBool(&pbi->br, pbi->MvSizeProbs[i][1]) )
				{
					Vector |= (1 << 3);
					Vector |= DecodeBool(&pbi->br, pbi->MvSizeProbs[i][3]) << 2;
				}
				else
				{
					Vector |= DecodeBool(&pbi->br, pbi->MvSizeProbs[i][2]) << 2;
				}
			}
		
			// Now Add in the low order and sign bits
			Vector |= HpBit;
			Vector |= (LowBit << 1);
			if ( SignBit )
				Vector = -Vector;
		}

		if ( i )
			mv->y = Vector;
		else
			mv->x = Vector;

    }
}


/****************************************************************************
 * 
 *  ROUTINE       :     FindNearestandNextNearest
 *
 *  INPUTS        :     
						MBrow row of macroblock to check
						MBcol col of macroblock to check
						*nearest returns nearest motion vector if found 0,0 otherwise
						*near returns next nearest motion vector if found 0,0 otherwise
						frame which frame motion vector should come from (gold or last)
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     true if motion vector differs 
                        false otherwise
 *
 *  FUNCTION      :     search through the existing motion vectors for two different MVs
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

void FindNearestandNextNearest
(
	PB_INSTANCE *pbi,
	UINT32 MBrow,
	UINT32 MBcol,
	MOTION_VECTORA *nearest,
	MOTION_VECTORA *nextnearest,	
	UINT8 Frame,
	int *type
)
{
	UINT32 BaseMB = MBOffset(MBrow,MBcol);
	UINT32 OffsetMB;
	int i;

	nearest->x=0;
	nearest->y=0;
	nextnearest->x=0;
	nextnearest->y=0;
	*type = NONEAREST_MACROBLOCK;

	for(i=0;i<12;i++)
	{

		OffsetMB = pbi->mvNearOffset[i]+BaseMB;

		if(VP5_Mode2Frame[pbi->predictionMode[OffsetMB]] != Frame)
			continue;

		if(*((unsigned int *) &pbi->MBMotionVector[OffsetMB]) == 0) 
			continue;

		*((unsigned int *) nearest) = *((unsigned int *) &pbi->MBMotionVector[OffsetMB]);
		*type = NONEAR_MACROBLOCK;

		break;

	}

	if(*((unsigned int *) nearest))
	{
		for(i=i+1;i<12;i++)
		{

			OffsetMB = pbi->mvNearOffset[i]+BaseMB;

			if(VP5_Mode2Frame[pbi->predictionMode[OffsetMB]] != Frame)
				continue;
			
			if(    *((unsigned int *) &pbi->MBMotionVector[OffsetMB])
				== *((unsigned int *) nearest) )
				continue;
			
			if(*((unsigned int *) &pbi->MBMotionVector[OffsetMB]) == 0) 
				continue;
			
			*((unsigned int *) nextnearest) = *((unsigned int *) &pbi->MBMotionVector[OffsetMB]);
			*type = MACROBLOCK;

			break;
		}
	
	}
}


