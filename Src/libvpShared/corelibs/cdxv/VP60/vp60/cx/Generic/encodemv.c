/****************************************************************************
*        
*   Module Title :	   encodemv.c
*
*   Description  :     Functions for encoding modes and motion vectors
*
****************************************************************************/
#define STRICT              /* Strict type checking */
 
/****************************************************************************
*  Header Files
****************************************************************************/
#include "compdll.h"
#include "boolhuff.h"
#include "decodemv.h"

/****************************************************************************
*  Macros
****************************************************************************/
// This small correction allows for the fact that an update to an MV probability
// may have benefit in subsequent frames as well as the current one.
#define MV_PROB_UPDATE_CORECTION	-1				

/****************************************************************************
*  Imports
****************************************************************************/        
extern void AddBitsToBuffer ( BOOL_CODER *bc, UINT32 data, UINT32 bits );

/****************************************************************************
 * 
 *  ROUTINE       :     encodeMotionVectorComponent
 *
 *  INPUTS        :     CP_INSTANCE *cpi : Pointer to encoder instance.
 *                      int i            : Selector as to what set of probs to use.						
 *                      INT32 Vector     : MV component to be coded.
 *                      INT32 MvOffset   : Reference value to code Vector from.
 *
 *  OUTPUTS       :     None.    
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Encodes a motion vector component either outputting
 *                      bits to the bitstream _or_ updating BitCounter in 
 *                      cpi->bc with the estimated cost.
 *
 *  SPECIAL NOTES :     cpi->bc.MeasureCost determines whether bits are
 *                      generated to the bitstream or not. 
 *
 ****************************************************************************/
void encodeMotionVectorComponent ( CP_INSTANCE *cpi, int i, INT32 Vector, INT32 MvOffset )
{
	UINT8 SignBit;
	INT32 TmpVector;
	void (*CodeBool)( BOOL_CODER *, int, int );

    // Are we outputting bits to the bitstream or just estimating cost?
	if ( cpi->bc.MeasureCost )
		CodeBool = VP6_EncodeBool2;
	else
		CodeBool = VP6_EncodeBool;

	// Code vector differentially
	TmpVector = Vector - MvOffset;

	// Convert vector to sign bit and magnitude
	if ( TmpVector < 0 )
	{
		TmpVector = - TmpVector;
		SignBit = 1;
	}
	else
		SignBit = 0;

	// Is the vector a small vector componet (currently < 2 whole pixels)
	if ( TmpVector <= 7 )
	{
		// Small vector
		CodeBool ( &cpi->bc, 0, cpi->pb.IsMvShortProb[i] );

		// Code up the magnitude value
		switch ( TmpVector )
		{
		case 0:
			CodeBool ( &cpi->bc, 0, cpi->pb.MvShortProbs[i][0] );
			CodeBool ( &cpi->bc, 0, cpi->pb.MvShortProbs[i][1] );
			CodeBool ( &cpi->bc, 0, cpi->pb.MvShortProbs[i][2] );
			break;
		case 1:
			CodeBool ( &cpi->bc, 0, cpi->pb.MvShortProbs[i][0] );
			CodeBool ( &cpi->bc, 0, cpi->pb.MvShortProbs[i][1] );
			CodeBool ( &cpi->bc, 1, cpi->pb.MvShortProbs[i][2] );
			break;
		case 2:
			CodeBool ( &cpi->bc, 0, cpi->pb.MvShortProbs[i][0] );
			CodeBool ( &cpi->bc, 1, cpi->pb.MvShortProbs[i][1] );
			CodeBool ( &cpi->bc, 0, cpi->pb.MvShortProbs[i][3] );
			break;
		case 3:
			CodeBool ( &cpi->bc, 0, cpi->pb.MvShortProbs[i][0] );
			CodeBool ( &cpi->bc, 1, cpi->pb.MvShortProbs[i][1] );
			CodeBool ( &cpi->bc, 1, cpi->pb.MvShortProbs[i][3] );
			break;
		case 4:
			CodeBool ( &cpi->bc, 1, cpi->pb.MvShortProbs[i][0] );
			CodeBool ( &cpi->bc, 0, cpi->pb.MvShortProbs[i][4] );
			CodeBool ( &cpi->bc, 0, cpi->pb.MvShortProbs[i][5] );
			break;
		case 5:
			CodeBool ( &cpi->bc, 1, cpi->pb.MvShortProbs[i][0] );
			CodeBool ( &cpi->bc, 0, cpi->pb.MvShortProbs[i][4] );
			CodeBool ( &cpi->bc, 1, cpi->pb.MvShortProbs[i][5] );
			break;
		case 6:
			CodeBool ( &cpi->bc, 1, cpi->pb.MvShortProbs[i][0] );
			CodeBool ( &cpi->bc, 1, cpi->pb.MvShortProbs[i][4] );
			CodeBool ( &cpi->bc, 0, cpi->pb.MvShortProbs[i][6] );
			break;
		case 7:
			CodeBool ( &cpi->bc, 1, cpi->pb.MvShortProbs[i][0] );
			CodeBool ( &cpi->bc, 1, cpi->pb.MvShortProbs[i][4] );
			CodeBool ( &cpi->bc, 1, cpi->pb.MvShortProbs[i][6] );
			break;
		}

		// Code the sign bit
        if ( TmpVector > 0 )			
			CodeBool ( &cpi->bc, SignBit, cpi->pb.MvSignProbs[i] );
	}
	else
	{
		// Indicate that we have a larger vector
		CodeBool ( &cpi->bc, 1, cpi->pb.IsMvShortProb[i] );

		// Code the magnitude
		CodeBool ( &cpi->bc, ((TmpVector & 0x01) ? 1 : 0), cpi->pb.MvSizeProbs[i][0] );	// QPel
		CodeBool ( &cpi->bc, ((TmpVector & 0x02) ? 1 : 0), cpi->pb.MvSizeProbs[i][1] );	// HPel
		CodeBool ( &cpi->bc, ((TmpVector & 0x04) ? 1 : 0), cpi->pb.MvSizeProbs[i][2] );	// Pel

		// At least one of the following must be non zero (or we would have coded a short vector)
		// We code from least likely to be set to most likely. The last bit is thus implicit 
		// if none of the others are set
		CodeBool ( &cpi->bc, ((TmpVector & 0x80) ? 1 : 0), cpi->pb.MvSizeProbs[i][7] );
		CodeBool ( &cpi->bc, ((TmpVector & 0x40) ? 1 : 0), cpi->pb.MvSizeProbs[i][6] );
		CodeBool ( &cpi->bc, ((TmpVector & 0x20) ? 1 : 0), cpi->pb.MvSizeProbs[i][5] );
		CodeBool ( &cpi->bc, ((TmpVector & 0x10) ? 1 : 0), cpi->pb.MvSizeProbs[i][4] );

		// Only need to code if at least one of the others was set else it is implicit
		if ( TmpVector & 0xF0 )
			CodeBool ( &cpi->bc, ((TmpVector & 0x08) ? 1 : 0), cpi->pb.MvSizeProbs[i][3] );

		// Code the sign bit
		CodeBool ( &cpi->bc, SignBit, cpi->pb.MvSignProbs[i] );
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     encodeMotionVector 
 *
 *  INPUTS        :     CP_INSTANCE *cpi : Pointer to encoder instance.
 *                      INT32 MVectorX   : MV x-component to be coded.
 *                      INT32 MVectorY   : MV y-component to be coded.
 *  					CODING_MODE Mode : Coding mode for corresponding MB/Block.
 *
 *  OUTPUTS       :     None.     
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Encodes a motion vector to the bitstream.
 *
 *  SPECIAL NOTES :     None. 
 * 
 ****************************************************************************/
void encodeMotionVector ( CP_INSTANCE *cpi, INT32 MVectorX, INT32 MVectorY, CODING_MODE Mode )
{  
	INT32  MvOffsetX = 0;
	INT32  MvOffsetY = 0;
	PB_INSTANCE *pbi = &cpi->pb; 

// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = MV_SECTION;
#endif

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

	encodeMotionVectorComponent ( cpi, 0, MVectorX, MvOffsetX );
	encodeMotionVectorComponent ( cpi, 1, MVectorY, MvOffsetY );
}

/****************************************************************************
 * 
 *  ROUTINE       :     CalculateMvNodeProbabilities
 *
 *  INPUTS        :     CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Build the MV entropy coding tree.
 *
 *  SPECIAL NOTES :     None. 
 *
***************************************************************************/
void CalculateMvNodeProbabilities ( CP_INSTANCE *cpi )
{
	UINT32 Sum;
	UINT32 Sum2;
	UINT32 Count;
	INT32  AbsVector;
	INT32  DistributionOffset;
	INT32  NewProb;
	INT32  i;
	INT32  j;

	UINT32 MvShortDist[2][2];
	UINT32 MvShortSizeDist[2][8];
	UINT32 MvSignDist[2][2];
	UINT32 MvSizeDist[2][LONG_MV_BITS][2];

	memset( MvShortDist,     0, sizeof(MvShortDist) );
	memset( MvShortSizeDist, 0, sizeof(MvShortSizeDist) );
	memset( MvSizeDist,      0, sizeof(MvSizeDist) );
	memset( MvSignDist,      0, sizeof(MvSignDist) );

	// Calculate the distributions for the MV nodes.
	cpi->FrameMvCount = 0;
	for ( i=0; i<2; i++ )
	{
		Sum = 0;
		for ( j=-(MV_ENTROPY_TOKENS >> 1); j<0; j++ )
		{
            // -ve vectors
			DistributionOffset = (MV_ENTROPY_TOKENS >> 1) + j;
			Count = cpi->MvBaselineDist[i][DistributionOffset];
			AbsVector = -j;
			MvSignDist[i][1] += Count;

            if ( AbsVector < 8 )
			{
				MvShortDist[i][0] += Count;			    // Short vector
				MvShortSizeDist[i][AbsVector] += Count;	// Magnitude distribution
			}
			else 
			{
				MvShortDist[i][1] += Count;				// Long vector

				MvSizeDist[i][0][(AbsVector & 0x01) ? 1 : 0] += Count;	// QPel
				MvSizeDist[i][1][(AbsVector & 0x02) ? 1 : 0] += Count;	// HPel
				MvSizeDist[i][2][(AbsVector & 0x04) ? 1 : 0] += Count;	// Bit1

				MvSizeDist[i][3][(AbsVector & 0x08) ? 1 : 0] += Count;	// Bit2
				MvSizeDist[i][4][(AbsVector & 0x10) ? 1 : 0] += Count;	// Bit3
				MvSizeDist[i][5][(AbsVector & 0x20) ? 1 : 0] += Count;	// Bit4 
				MvSizeDist[i][6][(AbsVector & 0x40) ? 1 : 0] += Count;	// Bit5
				MvSizeDist[i][7][(AbsVector & 0x80) ? 1 : 0] += Count;	// Bit6
			}
			Sum += Count;
		}

		// Zero Vector component
		Count = cpi->MvBaselineDist[i][(MV_ENTROPY_TOKENS >> 1)];
		MvShortDist[i][0] += Count;							
		MvShortSizeDist[i][0] += Count;
		Sum += Count;

		for ( j=1; j<(MV_ENTROPY_TOKENS >> 1); j++ )
		{
            // +ve vectors
			DistributionOffset = (MV_ENTROPY_TOKENS >> 1) + j;
			Count = cpi->MvBaselineDist[i][DistributionOffset];
			AbsVector = j;
			MvSignDist[i][0] += Count;							

			if ( AbsVector < 8 )
			{
				MvShortDist[i][0] += Count;						// Short vector
				MvShortSizeDist[i][AbsVector] += Count;			// Magnitude distribution
			}
			else
			{
				MvShortDist[i][1] += Count;						// Long vector

				MvSizeDist[i][0][(AbsVector & 0x01) ? 1 : 0] += Count;	// QPel
				MvSizeDist[i][1][(AbsVector & 0x02) ? 1 : 0] += Count;	// HPel
				MvSizeDist[i][2][(AbsVector & 0x04) ? 1 : 0] += Count;	// Bit1

				MvSizeDist[i][3][(AbsVector & 0x08) ? 1 : 0] += Count;	// Bit2
				MvSizeDist[i][4][(AbsVector & 0x10) ? 1 : 0] += Count;	// Bit3
				MvSizeDist[i][5][(AbsVector & 0x20) ? 1 : 0] += Count;	// Bit4 
				MvSizeDist[i][6][(AbsVector & 0x40) ? 1 : 0] += Count;	// Bit5
				MvSizeDist[i][7][(AbsVector & 0x80) ? 1 : 0] += Count;	// Bit6
			}
			Sum += Count;
		}
	}
	cpi->FrameMvCount = Sum;		// Note that Sum is reset to 0 for each "i" above		

	for ( i=0; i<2; i++ )		// X and Y
	{
		// Convert the distributions to optimal node probabilities
		Sum = MvShortDist[i][0] + MvShortDist[i][1];
		
        if ( Sum>0 )
		{
			Sum2 = MvShortDist[i][0];

			NewProb = (Sum2 * 255) / Sum;
			NewProb &= ~0x01;
			if ( NewProb < 1 )
				NewProb = 1;
			cpi->NewIsMvShortProb[i] = NewProb;
			cpi->NewIsMvShortHits[i][0] = Sum2;
			cpi->NewIsMvShortHits[i][1] = Sum - Sum2;
		}

		// Sign
		Sum = ( MvSignDist[i][0] + MvSignDist[i][1] );
		if ( Sum>0 )
		{
			Sum2 = MvSignDist[i][0];

			NewProb = (Sum2 * 255) / Sum;
			NewProb &= ~0x01;
			if ( NewProb < 1 )
				NewProb = 1;
			cpi->NewMvSignProbs[i] = NewProb;
			cpi->NewMvSignHits[i][0] = Sum2;
			cpi->NewMvSignHits[i][1] = Sum - Sum2;
		}

		// Tree nodes for short vectors
		for ( j=0; j<7; j++ )
		{
			// Node specific
			switch ( j )
			{
			case 0:
				// Node 0 Low
				Sum =  MvShortSizeDist[i][0] + MvShortSizeDist[i][1] + MvShortSizeDist[i][2] + MvShortSizeDist[i][3] +
					   MvShortSizeDist[i][4] + MvShortSizeDist[i][5] + MvShortSizeDist[i][6] + MvShortSizeDist[i][7];
				Sum2 = MvShortSizeDist[i][0] + MvShortSizeDist[i][1] + MvShortSizeDist[i][2] + MvShortSizeDist[i][3];
				break;
			case 1:
				// Node 1 LowLow
				Sum = Sum2;
				Sum2 = MvShortSizeDist[i][0] + MvShortSizeDist[i][1];
				break;
			case 2:
				// Node 2 LowLowLow
				Sum = Sum2;
				Sum2 = MvShortSizeDist[i][0];
				break;
			case 3:
				// Node 3 LowHighLow
				Sum = MvShortSizeDist[i][2] + MvShortSizeDist[i][3];
				Sum2 = MvShortSizeDist[i][2];
				break;
			case 4:
				// Node 4 HighLow
				Sum = MvShortSizeDist[i][4] + MvShortSizeDist[i][5] + MvShortSizeDist[i][6] + MvShortSizeDist[i][7];
				Sum2 = MvShortSizeDist[i][4] + MvShortSizeDist[i][5];
				break;
			case 5:
				// Node 5 HighLowLow 
				Sum = MvShortSizeDist[i][4] + MvShortSizeDist[i][5];
				Sum2 = MvShortSizeDist[i][4];
				break;
			case 6:
				// Node 6 HighLowHigh
				Sum = MvShortSizeDist[i][6] + MvShortSizeDist[i][7];
				Sum2 = MvShortSizeDist[i][6];
				break;
			}

			if ( Sum )
			{ 
				NewProb = (Sum2 * 255)/Sum;
				NewProb &= ~0x01;
				if ( NewProb < 1 )
					NewProb = 1;
				cpi->NewMvShortProbs[i][j] = NewProb;
				cpi->NewMvShortHits[i][j][0] = Sum2;
				cpi->NewMvShortHits[i][j][1] = Sum - Sum2;
			}
		}

		// Long vectors 
		for ( j=0; j<LONG_MV_BITS; j++ )
		{
			Sum	 = MvSizeDist[i][j][0] + MvSizeDist[i][j][1];
			Sum2 = MvSizeDist[i][j][0];

			if ( Sum )
			{
				NewProb = (Sum2 * 255)/Sum;
				NewProb &= ~0x01;
				if ( NewProb < 1 )
					NewProb = 1;
				cpi->NewMvSizeProbs[i][j] = NewProb;
				cpi->NewMvSizeHits[i][j][0] = Sum2;
				cpi->NewMvSizeHits[i][j][1] = Sum - Sum2;
			}
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     BuildandPackMvTree
 *
 *  INPUTS        :     CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       :     None.     
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Build the MV entropy coding tree.
 *
 *  SPECIAL NOTES :     None. 
 *
***************************************************************************/
void BuildandPackMvTree ( CP_INSTANCE *cpi )
{
	INT32  i;
	INT32  NewProb;
	INT32  OldProb;
	INT32  NewBits;
	INT32  OldBits;
	INT32  ProbUpdateCost;

// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = MV_SECTION;
#endif

	// calculate the MV node Probabilities
	CalculateMvNodeProbabilities ( cpi );

	// If appropriate update short, sign, qpel, half pixel and low order bit probabilities.
	for ( i=0; i<2; i++ )
	{
		// Update the Short vector probability.
		NewProb = cpi->NewIsMvShortProb[i];
		OldProb = cpi->pb.IsMvShortProb[i];
		OldBits = ((cpi->NewIsMvShortHits[i][0] * VP6_ProbCost[OldProb])/256) +
				  ((cpi->NewIsMvShortHits[i][1] * VP6_ProbCost[255 - OldProb])/256);
		NewBits = ((cpi->NewIsMvShortHits[i][0] * VP6_ProbCost[NewProb])/256) +
				  ((cpi->NewIsMvShortHits[i][1] * VP6_ProbCost[255 - NewProb])/256);

		ProbUpdateCost = PROB_UPDATE_BASELINE_COST + MV_PROB_UPDATE_CORECTION;
		ProbUpdateCost += (VP6_ProbCost[255 - VP6_MvUpdateProbs[i][0]] + 128) / 256;
		ProbUpdateCost -= (VP6_ProbCost[VP6_MvUpdateProbs[i][0]] + 128) / 256;

		if ( (OldBits - NewBits) > ProbUpdateCost )
		{
			cpi->pb.IsMvShortProb[i] = NewProb;
			VP6_EncodeBool  ( &cpi->bc, 1, VP6_MvUpdateProbs[i][0] );
			AddBitsToBuffer ( &cpi->bc, NewProb>>1, PROB_UPDATE_BASELINE_COST );
		}
		else
		{
			VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][0] );
		}

		// Sign
		NewProb = cpi->NewMvSignProbs[i];
		OldProb = cpi->pb.MvSignProbs[i];

		OldBits = ((cpi->NewMvSignHits[i][0] * VP6_ProbCost[OldProb])/256) +
				  ((cpi->NewMvSignHits[i][1] * VP6_ProbCost[255 - OldProb])/256);
		NewBits = ((cpi->NewMvSignHits[i][0] * VP6_ProbCost[NewProb])/256) +
				  ((cpi->NewMvSignHits[i][1] * VP6_ProbCost[255 - NewProb])/256);

		ProbUpdateCost = PROB_UPDATE_BASELINE_COST + MV_PROB_UPDATE_CORECTION;
		ProbUpdateCost += (VP6_ProbCost[255 - VP6_MvUpdateProbs[i][1]] + 128) / 256;
		ProbUpdateCost -= (VP6_ProbCost[VP6_MvUpdateProbs[i][1]] + 128) / 256;

		if ( (OldBits - NewBits) > ProbUpdateCost )
		{
			cpi->pb.MvSignProbs[i] = NewProb;
			VP6_EncodeBool ( &cpi->bc, 1, VP6_MvUpdateProbs[i][1] );
			AddBitsToBuffer ( &cpi->bc, NewProb >> 1, PROB_UPDATE_BASELINE_COST );
		}
		else
		{
			VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][1] );
		}
	}

	// If appropriate update the tree probabilities for short vector
	for ( i = 0; i < 2; i++ )   // X then Y
	{
		INT32  j;
		UINT32 MvUpdateProbsOffset = 2;				// Offset into VP6_MvUpdateProbs[i][]

		// For each node in the tree
		for ( j=0; j<7; j++ )
		{
			NewProb = cpi->NewMvShortProbs[i][j];
			OldProb = cpi->pb.MvShortProbs[i][j];

			OldBits = ((cpi->NewMvShortHits[i][j][0] * VP6_ProbCost[OldProb])/256) +
					  ((cpi->NewMvShortHits[i][j][1] * VP6_ProbCost[255 - OldProb])/256);
			NewBits = ((cpi->NewMvShortHits[i][j][0] * VP6_ProbCost[NewProb])/256) +
					  ((cpi->NewMvShortHits[i][j][1] * VP6_ProbCost[255 - NewProb])/256);

			ProbUpdateCost = PROB_UPDATE_BASELINE_COST + MV_PROB_UPDATE_CORECTION;
			ProbUpdateCost += (VP6_ProbCost[255 - VP6_MvUpdateProbs[i][MvUpdateProbsOffset]] + 128) / 256;
			ProbUpdateCost -= (VP6_ProbCost[VP6_MvUpdateProbs[i][MvUpdateProbsOffset]] + 128) / 256;

			if ( (OldBits - NewBits) > ProbUpdateCost )
			{
				cpi->pb.MvShortProbs[i][j] = NewProb;
				VP6_EncodeBool(&cpi->bc, 1, VP6_MvUpdateProbs[i][MvUpdateProbsOffset] );
				AddBitsToBuffer( &cpi->bc, NewProb >> 1, PROB_UPDATE_BASELINE_COST );
			}
			else
			{
				VP6_EncodeBool(&cpi->bc, 0, VP6_MvUpdateProbs[i][MvUpdateProbsOffset] );
			}

			// Increment to next offset in VP6_MvUpdateProbs[];
			MvUpdateProbsOffset++;
		}
	}

	// If appropriate update the bit probabilities for long vectors
	for ( i=0; i<2; i++ )   // X then Y
	{
		INT32  j;
		UINT32 MvUpdateProbsOffset = 2 + 7;
	
		// For each bit
		for ( j=0; j<LONG_MV_BITS; j++ )
		{
			NewProb = cpi->NewMvSizeProbs[i][j];
			OldProb = cpi->pb.MvSizeProbs[i][j];

			OldBits = ((cpi->NewMvSizeHits[i][j][0] * VP6_ProbCost[OldProb])/256) +
					  ((cpi->NewMvSizeHits[i][j][1] * VP6_ProbCost[255 - OldProb])/256);
			NewBits = ((cpi->NewMvSizeHits[i][j][0] * VP6_ProbCost[NewProb])/256) +
					  ((cpi->NewMvSizeHits[i][j][1] * VP6_ProbCost[255 - NewProb])/256);

			ProbUpdateCost = PROB_UPDATE_BASELINE_COST + MV_PROB_UPDATE_CORECTION;
			ProbUpdateCost += (VP6_ProbCost[255 - VP6_MvUpdateProbs[i][MvUpdateProbsOffset]] + 128) / 256;
			ProbUpdateCost -= (VP6_ProbCost[VP6_MvUpdateProbs[i][MvUpdateProbsOffset]] + 128) / 256;

			if ( (OldBits - NewBits) > ProbUpdateCost )
			{
				cpi->pb.MvSizeProbs[i][j] = NewProb;
				VP6_EncodeBool(&cpi->bc, 1, VP6_MvUpdateProbs[i][MvUpdateProbsOffset] );
				AddBitsToBuffer( &cpi->bc, NewProb >> 1, PROB_UPDATE_BASELINE_COST );
			}
			else
			{
				VP6_EncodeBool(&cpi->bc, 0, VP6_MvUpdateProbs[i][MvUpdateProbsOffset] );
			}

			// Increment to next offset in VP6_MvUpdateProbs[];
			MvUpdateProbsOffset++;
		}
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     BuildandPackMvTree2
 *
 *  INPUTS        :     CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       :     None.     
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Build the MV entropy coding tree. This version is
 *                      used when in unbuffered / VC mode to improve tolerance
 *                      to dropped frames.
 *
 *  SPECIAL NOTES :     None. 
 *
 ***************************************************************************/
void BuildandPackMvTree2 ( CP_INSTANCE *cpi )
{
	INT32  i;

// Stats to measure section costs
#if defined MEASURE_SECTION_COSTS
ActiveSection = MV_SECTION;
#endif
	
	// calculate the MV node Probabilities
	CalculateMvNodeProbabilities ( cpi );

	// Send short and sign probabilities
	for ( i=0; i<2; i++ )
	{
		cpi->pb.IsMvShortProb[i] = cpi->NewIsMvShortProb[i];
		VP6_EncodeBool ( &cpi->bc, 1, VP6_MvUpdateProbs[i][0] );
		AddBitsToBuffer( &cpi->bc, cpi->pb.IsMvShortProb[i] >> 1, PROB_UPDATE_BASELINE_COST );

		cpi->pb.MvSignProbs[i] = cpi->NewMvSignProbs[i];
		VP6_EncodeBool ( &cpi->bc, 1, VP6_MvUpdateProbs[i][1] );
		AddBitsToBuffer( &cpi->bc, cpi->pb.MvSignProbs[i] >> 1, PROB_UPDATE_BASELINE_COST );
	}

	// Short vector tree nodes
	for ( i=0; i<2; i++ )
	{
		// Node 0 Low
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][2] );

		// Node 1 LowLow
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][3] );

		// Node 2 LowLowLow
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][4] );

		// Node 3 LowHighLow
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][5] );

		// Node 4 HighLow
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][6] );

		// Node 5 HighLowLow
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][7] );

		// Node 6 HighHighLow
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][8] );
	}

	// Long vector Probabilities
	for ( i=0; i<2; i++ )
	{
		// QPel
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][9] );

		// HPel
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][10] );

		// Bit1
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][11] );

		// Bit2
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][12] );

		// Bit3
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][13] );

		// Bit4
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][14] );

		// Bit5
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][15] );

		// Bit6
		VP6_EncodeBool ( &cpi->bc, 0, VP6_MvUpdateProbs[i][16] );
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     BuildMVCostEstimates
 *
 *  INPUTS        :     CP_INSTANCE *cpi : Pointer to encoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Calculate a cost in bits of encoding a motion vector.
 *
 *  SPECIAL NOTES :     None. 
 *
 ***************************************************************************/
void BuildMVCostEstimates ( CP_INSTANCE *cpi )
{
	int i;
	int vect;

	cpi->bc.MeasureCost = TRUE;

    for ( i=0; i<MV_ENTROPY_TOKENS; i++ )
	{
		cpi->bc.BitCounter = 0;
		vect = i - (MV_ENTROPY_TOKENS/2);

		encodeMotionVectorComponent ( cpi, 0, vect, 0 );
		
		// keep all costs at 64 * actual number of bits
		cpi->EstMvCostPtrX[vect] = (cpi->bc.BitCounter ) >> 2;	

		cpi->bc.BitCounter = 0;
		encodeMotionVectorComponent ( cpi, 1, vect, 0 );

		// keep all costs at 64 * actual number of bits
		cpi->EstMvCostPtrY[vect] = (cpi->bc.BitCounter) >> 2;
	}
	
    cpi->bc.MeasureCost = FALSE;
}
