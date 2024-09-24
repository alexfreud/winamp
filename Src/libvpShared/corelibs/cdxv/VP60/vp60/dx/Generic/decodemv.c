/****************************************************************************
*        
*   Module Title :	   Decodemv.c     
*
*   Description  :     Functions for decoding modes and motion vectors.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "pbdll.h"					
#include "decodemode.h" 
#include "decodemv.h"

/****************************************************************************
*  Macros
****************************************************************************/        


/****************************************************************************
*  Exports
****************************************************************************/        
const UINT8 VP6_MvUpdateProbs[2][MV_NODES] = 
{ 
	{ 237, 246, 253, 253, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 250, 250, 252 }, 
	{ 231, 243, 245, 253, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 251, 251, 254 }
};

const UINT8 DefaultMvShortProbs[2][7] = 
{ 
    { 225, 146, 172, 147, 214,  39, 156 }, 
    { 204, 170, 119, 235, 140, 230, 228 }
};

const UINT8 DefaultMvLongProbs[2][LONG_MV_BITS] =  
{ 
    { 247, 210, 135,  68, 138, 220, 239, 246 }, 
    { 244, 184, 201,  44, 173, 221, 239, 253 } 
};

const UINT8 DefaultIsShortProbs[2] = { 162, 164 };
const UINT8 DefaultSignProbs[2]    = { 128, 128 };

/**************************************************************************** 
 * 
 *  ROUTINE       :     VP6_ConfigureMvEntropyDecoder
 *
 *  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
 *                      UINT8 FrameType  : Type of the frame.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Builds the MV entropy decoding tree.
 *
 *  SPECIAL NOTES :     None. 
 *
***************************************************************************/
void VP6_ConfigureMvEntropyDecoder( PB_INSTANCE *pbi, UINT8 FrameType )
{
	int i;
    
	// This funciton is not called at all for a BASE_FRAME
	// Read any changes to mv probabilities.
	for ( i = 0; i < 2; i++ )
	{
		// Short vector probability
		if ( VP6_DecodeBool(&pbi->br, VP6_MvUpdateProbs[i][0]) )
		{
			pbi->IsMvShortProb[i] = VP6_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
			if ( pbi->IsMvShortProb[i] == 0 )
				pbi->IsMvShortProb[i] = 1;
		}

		// Sign probability
		if ( VP6_DecodeBool(&pbi->br, VP6_MvUpdateProbs[i][1]) )
		{
			pbi->MvSignProbs[i] = VP6_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
			if ( pbi->MvSignProbs[i] == 0 )
				pbi->MvSignProbs[i] = 1;
		}
	}

	// Short vector tree node probabilities
	for ( i = 0; i < 2; i++ )
	{
		UINT32 j;
		UINT32 MvUpdateProbsOffset = 2;				// Offset into MvUpdateProbs[i][]

		for ( j = 0; j < 7; j++ )
		{
			if ( VP6_DecodeBool(&pbi->br, VP6_MvUpdateProbs[i][MvUpdateProbsOffset]) )
			{
				pbi->MvShortProbs[i][j] = VP6_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				if ( pbi->MvShortProbs[i][j] == 0 )
					pbi->MvShortProbs[i][j] = 1;
			}
			MvUpdateProbsOffset++;
		}
	}

	// Long vector tree node probabilities
	for ( i = 0; i < 2; i++ )
	{
		UINT32 j;
		UINT32 MvUpdateProbsOffset = 2 + 7;

		for ( j = 0; j < LONG_MV_BITS; j++ )
		{
			if ( VP6_DecodeBool(&pbi->br, VP6_MvUpdateProbs[i][MvUpdateProbsOffset]) )
			{
				pbi->MvSizeProbs[i][j] = VP6_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				if ( pbi->MvSizeProbs[i][j] == 0 )
					pbi->MvSizeProbs[i][j] = 1;
			}
			MvUpdateProbsOffset++;
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_decodeMotionVector 
 *
 *  INPUTS        :     PB_INSTANCE *pbi  : Pointer to decoder instance.
 *						CODING_MODE Mode  : MV coding mode.
 *						
 *  OUTPUTS       :     MOTION_VECTOR *mv : Returned motion vector.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Decodes a motion vector from the bitstream.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_decodeMotionVector
(
	PB_INSTANCE *pbi,
	MOTION_VECTOR *mv,
	CODING_MODE Mode 
)
{
	UINT32 i;
	INT32  Vector = 0;
	INT32  SignBit = 0;
	INT32  MvOffsetX = 0;
	INT32  MvOffsetY = 0;

	// Work out how the MV was coded so that the appropriate origin offset can be applied
	if ( Mode == CODE_INTER_PLUS_MV )
	{	
        // Normal Inter MV
		if ( pbi->mbi.NearestMvIndex < MAX_NEAREST_ADJ_INDEX )
		{
			MvOffsetX = pbi->mbi.NearestInterMVect.x;
			MvOffsetY = pbi->mbi.NearestInterMVect.y;
		}
	}
	else
	{	
        // Golden Frame MV
		if ( pbi->mbi.NearestGMvIndex < MAX_NEAREST_ADJ_INDEX )
		{
			MvOffsetX = pbi->mbi.NearestGoldMVect.x;
			MvOffsetY = pbi->mbi.NearestGoldMVect.y;
		}
	}

	for ( i = 0; i < 2; i++ )
	{
		Vector = 0;

		// Is the vector a small vector or a large vector
		if ( !VP6_DecodeBool(&pbi->br, pbi->IsMvShortProb[i]) )
		{
			// Small magnitude vector
			if ( VP6_DecodeBool(&pbi->br, pbi->MvShortProbs[i][0] ) )
			{
				Vector += (1 << 2);
				if ( VP6_DecodeBool(&pbi->br, pbi->MvShortProbs[i][4]) )
				{
					Vector += (1 << 1);
					Vector += VP6_DecodeBool(&pbi->br, pbi->MvShortProbs[i][6]);
				}
				else
				{
					Vector += VP6_DecodeBool(&pbi->br, pbi->MvShortProbs[i][5]);
				}
			}
			else
			{
				if ( VP6_DecodeBool(&pbi->br, pbi->MvShortProbs[i][1]) )
				{
					Vector += (1 << 1);
					Vector += VP6_DecodeBool(&pbi->br, pbi->MvShortProbs[i][3]);
				}
				else
				{
					Vector = VP6_DecodeBool(&pbi->br, pbi->MvShortProbs[i][2]);
				}
			}
		}
		else
		{
			// Large magnitude vector
			Vector = VP6_DecodeBool( &pbi->br, pbi->MvSizeProbs[i][0] );
			Vector += (VP6_DecodeBool( &pbi->br, pbi->MvSizeProbs[i][1] ) << 1);
			Vector += (VP6_DecodeBool( &pbi->br, pbi->MvSizeProbs[i][2] ) << 2);

			Vector += (VP6_DecodeBool( &pbi->br, pbi->MvSizeProbs[i][7] ) << 7);
			Vector += (VP6_DecodeBool( &pbi->br, pbi->MvSizeProbs[i][6] ) << 6);
			Vector += (VP6_DecodeBool( &pbi->br, pbi->MvSizeProbs[i][5] ) << 5);
			Vector += (VP6_DecodeBool( &pbi->br, pbi->MvSizeProbs[i][4] ) << 4);

			// If none of the higher order bits are set then this bit is implicit
			if ( Vector & 0xF0 )
				Vector += (VP6_DecodeBool( &pbi->br, pbi->MvSizeProbs[i][3] ) << 3);
			else
				Vector += 0x08;
		}

		// Read the sign bit if needed.
		if ( Vector != 0 )
		{
			SignBit = VP6_DecodeBool(&pbi->br, pbi->MvSignProbs[i]);

			if ( SignBit )
				Vector = -Vector;
		}

		if ( i )
			mv->y = Vector + MvOffsetY;
		else
			mv->x = Vector + MvOffsetX;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_FindNearestandNextNearest
 *
 *  INPUTS        :     PB_INSTANCE *pbi  : Pointer to decoder instance.
 *						UINT32 MBrow      : Row of macroblock to check.
 *						UINT32 MBcol      : Col of macroblock to check.
 *						UINT8 Frame       : Frame type which MV should come 
 *                                          from (Golden or Last).
 *
 *  OUTPUTS       :     int *type         : Type of the vector returned.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Find a Nearest and NextNearest MV in nearby MBs in
 *                      frames having the same type (Golden or Last).
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_FindNearestandNextNearest
(
	PB_INSTANCE *pbi,
	UINT32 MBrow,
	UINT32 MBcol,
	UINT8 Frame,
	int *type
)
{
	int i;
	UINT32 OffsetMB;
	UINT32 BaseMB = MBOffset(MBrow,MBcol);
	INT32 Nearest = 0;
    INT32 NextNearest = 0;
    INT32 nearestIndex;
    UINT32 thisMv;
    INT32 typet;
    
    typet = NONEAREST_MACROBLOCK;

	// BEWARE:
    // The use of (unsigned int *) casting here is potentially dangerous 
	// and will only work if the motion vector structure consists of 
	// two 16 bit values and is 32 bit aligned.
	for ( i=0; i<12 ; i++ )
	{ 
		OffsetMB = pbi->mvNearOffset[i] + BaseMB;

		if ( VP6_Mode2Frame[pbi->predictionMode[OffsetMB]] != Frame )
			continue;

		thisMv = *((unsigned int *) &pbi->MBMotionVector[OffsetMB]);

		if ( thisMv ) 
        {
	
		    *((unsigned int *) &Nearest) = thisMv;
		    typet = NONEAR_MACROBLOCK;
    		break;
        }		     
   	}

    nearestIndex = i;

    for ( i=i+1; i<12; i++ )
    {
        OffsetMB = pbi->mvNearOffset[i] + BaseMB;
        
        if ( VP6_Mode2Frame[pbi->predictionMode[OffsetMB]] != Frame )
            continue;
        
		thisMv = *((unsigned int *) &pbi->MBMotionVector[OffsetMB]);
        if( thisMv == *((unsigned int *) &Nearest) )
			continue;
		
		if( thisMv ) 
        {
		    *((unsigned int *) &NextNearest) = thisMv;
		    typet = MACROBLOCK;
		    break;
        }
    }

	// Only update type if normal frame
	if ( Frame == 1 )
	{
        *type = typet;
        pbi->mbi.NearestMvIndex = nearestIndex;
		*((unsigned int *) &pbi->mbi.NearestInterMVect)  = *((unsigned int *) &Nearest);
		*((unsigned int *) &pbi->mbi.NearInterMVect)  = *((unsigned int *) &NextNearest);
	}
	else
	{
        pbi->mbi.NearestGMvIndex = nearestIndex;
		*((unsigned int *) &pbi->mbi.NearestGoldMVect)  = *((unsigned int *) &Nearest);
		*((unsigned int *) &pbi->mbi.NearGoldMVect)  = *((unsigned int *) &NextNearest);
	}
}
