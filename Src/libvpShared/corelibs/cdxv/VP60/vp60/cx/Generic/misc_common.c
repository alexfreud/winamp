/****************************************************************************
*
*   Module Title :     MiscCommon.c
*
*   Description  :     Miscellaneous common routines
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "compdll.h"

/****************************************************************************
*  Macros
****************************************************************************/        
#define KF_WORST_Q_INDEX	20

/****************************************************************************
*  Module Static
****************************************************************************/        
// Provisional data for interpolated positions (x.xx00)     
static const LINE_EQ2 InterBpBEquations[Q_TABLE_SIZE] = 
{
	{ 0.00115,   445.98890}, { 0.00132,   406.83041}, { 0.00148,    400.18762}, { 0.00160,    363.68569},
	{ 0.00174,   378.33470}, { 0.00199,   377.42412}, { 0.00237,    300.00652}, { 0.00262,    266.74763},
	{ 0.00280,   252.69107}, { 0.00312,   205.72084}, { 0.00351,    183.14721}, { 0.00386,    155.88815},
	{ 0.00432,    95.74501}, { 0.00447,    91.53841}, { 0.00469,     69.65309}, { 0.00481,     80.08054},
	{ 0.00496,    63.44023}, { 0.00520,   110.00485}, { 0.00542,    108.04172}, { 0.00558,    165.23727},
	{ 0.00585,   154.10530}, { 0.00600,   176.84087}, { 0.00621,    169.06892}, { 0.00641,    157.49036},
	{ 0.00664,   148.93471}, { 0.00713,   199.24375}, { 0.00752,    210.01239}, { 0.00816,    195.86514},
	{ 0.00883,   352.16439}, { 0.00920,   354.57230}, { 0.00958,    393.60319}, { 0.00999,    420.30206},
	{ 0.01063,   529.24195}, { 0.01118,   538.52879}, { 0.01170,    651.23813}, { 0.01218,    713.79800},
	{ 0.01263,   788.52303}, { 0.01321,   871.46329}, { 0.01393,   1078.68114}, { 0.01459,   1180.46989},
	{ 0.01529,  1309.93961}, { 0.01597,  1366.39052}, { 0.01677,   1627.17452}, { 0.01762,   1826.38865},
	{ 0.01859,  2010.00287}, { 0.01963,  2388.91757}, { 0.02070,   2683.36530}, { 0.02178,   2875.49060},
	{ 0.02260,  3178.16923}, { 0.02418,  3572.88801}, { 0.02531,   4062.37227}, { 0.02709,   4921.59728},
	{ 0.02918,  5592.29649}, { 0.03107,  6186.93245}, { 0.03372,   7376.13311}, { 0.03768,   9534.78915},
	{ 0.04197, 11906.09757}, { 0.04691, 15241.79652}, { 0.05157,  18904.29545}, { 0.05953,  27091.47553},
	{ 0.07025, 41522.27709}, { 0.08343, 67789.86180}, { 0.11547, 124265.97640}, { 0.13380, 210301.81305},
};

static const LINE_EQ2 IntraBpBEquations[Q_TABLE_SIZE] = 
{
	{ 0.00106,  2288.83435}, { 0.00111,  2381.24321}, { 0.00116,   2484.21594}, { 0.00120,   2536.01662},
	{ 0.00127,  2674.68182}, { 0.00136,  2835.12286}, { 0.00146,   2946.60819}, { 0.00154,   3034.48115},
	{ 0.00163,  3117.20084}, { 0.00172,  3233.89966}, { 0.00184,   3407.24634}, { 0.00195,   3543.03650},
	{ 0.00210,  3699.64900}, { 0.00215,  3793.02049}, { 0.00220,   3854.74475}, { 0.00224,   3915.99566},
	{ 0.00227,  3959.82316}, { 0.00233,  4204.84699}, { 0.00237,   4276.08365}, { 0.00242,   4387.12774},
	{ 0.00246,  4452.87571}, { 0.00251,  4578.78112}, { 0.00256,   4642.65467}, { 0.00261,   4710.56167},
	{ 0.00267,  4780.30368}, { 0.00279,  5030.71570}, { 0.00288,   5170.75293}, { 0.00303,   5374.83851},
	{ 0.00315,  5872.91562}, { 0.00324,  6002.40178}, { 0.00331,   6163.13111}, { 0.00341,   6330.88665},
	{ 0.00356,  6638.13056}, { 0.00367,  6813.20389}, { 0.00378,   7073.27347}, { 0.00391,   7264.41977},
	{ 0.00401,  7464.35187}, { 0.00414,  7686.68885}, { 0.00427,   8222.38307}, { 0.00442,   8469.27069},
	{ 0.00459,  8750.44432}, { 0.00472,  8961.97754}, { 0.00492,   9406.63273}, { 0.00513,   9784.70928},
	{ 0.00531, 10199.58953}, { 0.00556, 10786.82064}, { 0.00582,  11271.52430}, { 0.00606,  11694.10222},
	{ 0.00631, 12147.95242}, { 0.00664, 12808.92178}, { 0.00695,  13528.07213}, { 0.00732,  14860.00245},
	{ 0.00779, 15815.03822}, { 0.00822, 16685.69714}, { 0.00884,  18214.89132}, { 0.00972,  20431.29266},
	{ 0.01063, 22995.09970}, { 0.01169, 26309.59450}, { 0.01275,  29857.49766}, { 0.01436,  37027.81351},
	{ 0.01637, 49621.40625}, { 0.01873, 72068.47846}, { 0.02150, 123873.67566}, { 0.02488, 208511.43171},
};   

/****************************************************************************
*  Exports
****************************************************************************/   

// For FixedQ helps choos appropriate key frame quality.          
const UINT8 FixedQKfBoostTable[64] = 
{
	22, 24, 26, 26, 26, 26, 27, 28,
	28, 27, 27, 26, 26, 25, 25, 24,
	24, 23, 23, 22, 22, 21, 21, 20,
	20, 19, 18, 18, 17, 16, 16, 15,
	15, 14, 14, 13, 13, 13, 12, 12,
	12, 12, 12, 11, 11, 11, 10,  9,
	 8,  7,  7,  6,  5,  4,  3,  2,
	 1,  1,  0,  0,  0,  0,  0,  0
};

const UINT8 GfFixedQKfBoostTable[64] = 
{
	20, 22, 23, 23, 23, 24, 25, 26,
	27, 27, 28, 28, 29, 29, 28, 28,
	28, 27, 27, 27, 26, 26, 26, 26,
	25, 25, 25, 25, 24, 24, 23, 23,
	22, 21, 21, 20, 20, 19, 18, 17,
	16, 15, 14, 13, 12, 11, 10,  9,
	 8,  7,  6,  6,  5,  5,  4,  4,
	 4,  3,  3,  2,  1,  0,  0,  0
};

/****************************************************************************
 * 
 *  ROUTINE       :     GetEstimatedBpb
 *
 *  INPUTS        :     CP_INSTANCE *cpi    : Pointer to encoder instance.
 *						UINT32 TargetQIndex : Q Index to estimate for.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     double: The current estimate for the number of bits per block
 *                      at the current Q.
 *
 *  FUNCTION      :     Computes estimate of the number of bits per block 
 *                      that will be produced if coded at the specified Q.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
double GetEstimatedBpb ( CP_INSTANCE *cpi, UINT32 TargetQIndex )
{
    double BitsPerBlock;
	double Complexity = (double)cpi->InterError;	

    // NOTE 1: Inter and Intra error are the same for key frames.
    // NOTE 2: It may prove necessary to clip the complexity value.
    
    // Adjust according to currently active correction factor
    if ( VP6_GetFrameType(&cpi->pb) == BASE_FRAME )
    {
		// TEMP: use inter equations * 2 until Key frame values worked out
		//BitsPerBlock = ((InterBpBEquations[TargetQIndex].M * Complexity) + InterBpBEquations[TargetQIndex].C)/(double)cpi->pb.UnitFragments;
		BitsPerBlock = ((IntraBpBEquations[TargetQIndex].M * Complexity) + IntraBpBEquations[TargetQIndex].C)/(double)cpi->pb.UnitFragments;
	    BitsPerBlock = BitsPerBlock * cpi->KeyFrameBpbCorrectionFactor;
    }
    else 
    {
        // Get primary prediction
		BitsPerBlock = ((InterBpBEquations[TargetQIndex].M * Complexity) + InterBpBEquations[TargetQIndex].C)/(double)cpi->pb.UnitFragments;

		// Apply the correction factor that is based upon recent observations of overshoot and undershoot
		// Note that if we are coding a GF update frame we expect overshoot because we are jumping to
		// a higher quality from a lower quality (the tables were caluclated using fixed Q). Hence the 
		// additional correction for this case.
		if ( cpi->pb.RefreshGoldenFrame )
			BitsPerBlock = BitsPerBlock * (cpi->BpbCorrectionFactor * cpi->GfuBpbCorrectionFactor);
		else
			BitsPerBlock = BitsPerBlock * cpi->BpbCorrectionFactor;
    }

	return BitsPerBlock;
}

void UpdateBpbCorrectionFactor2 ( CP_INSTANCE *cpi, UINT32 FrameSize )
{
	double BpbCorrectionFactor;

#if defined(_MSC_VER)
	// NOTE: This function uses floating point
	ClearSysState();
#endif

	if ( VP6_GetFrameType(&cpi->pb) == BASE_FRAME )
		BpbCorrectionFactor = cpi->KeyFrameBpbCorrectionFactor;
	else
	{
		if ( cpi->pb.RefreshGoldenFrame )
			BpbCorrectionFactor = cpi->GfuBpbCorrectionFactor;
		else
			BpbCorrectionFactor = cpi->BpbCorrectionFactor;
	}

	// Work out a size correction factor.
	BpbCorrectionFactor *= (3+(2.0 * FrameSize) / cpi->ThisFrameTarget) /5;

	if ( VP6_GetFrameType(&cpi->pb) == BASE_FRAME )
		cpi->KeyFrameBpbCorrectionFactor = BpbCorrectionFactor;
	else
	{
		if ( cpi->pb.RefreshGoldenFrame )
			cpi->GfuBpbCorrectionFactor = BpbCorrectionFactor;
		else
			cpi->BpbCorrectionFactor = BpbCorrectionFactor;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     UpdateBpbCorrectionFactor
 *
 *  INPUTS        :     CP_INSTANCE *cpi : Pointer to encoder instance.
 *                      UINT32 FrameSize : Size of coded frame.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Adjusts the Bits Per Block correction factor used
 *                      for rate prediction.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void UpdateBpbCorrectionFactor ( CP_INSTANCE *cpi, UINT32 FrameSize )
{
    INT32  CorrectionFactor=100;
	double BpbCorrectionFactor;

#if defined(_MSC_VER)
	// NOTE: This function uses floating point
	ClearSysState();
#endif

	if ( VP6_GetFrameType(&cpi->pb) == BASE_FRAME )
		BpbCorrectionFactor = cpi->KeyFrameBpbCorrectionFactor;
	else
	{
		if ( cpi->pb.RefreshGoldenFrame )
			BpbCorrectionFactor = cpi->GfuBpbCorrectionFactor;
		else
			BpbCorrectionFactor = cpi->BpbCorrectionFactor;
	}

	// Work out a size correction factor.
    if(cpi->ThisFrameTarget > 0 )
    	CorrectionFactor = (100 * FrameSize) / cpi->ThisFrameTarget;
    
	if ( (CorrectionFactor > 101) && 
		 (cpi->pb.quantizer->FrameQIndex > cpi->Configuration.ActiveWorstQuality ) )
    {
        // We are not already at the worst allowable quality
		CorrectionFactor = 100 + ((CorrectionFactor - 100)/4);
		if ( CorrectionFactor > 125 )   // Damp the adjustment
			BpbCorrectionFactor = (BpbCorrectionFactor * 125)/100;
		else
			BpbCorrectionFactor = (BpbCorrectionFactor * CorrectionFactor) / 100;

		// Keep BpbCorrectionFactor within limits
		if ( BpbCorrectionFactor > MAX_BPB_FACTOR )
			 BpbCorrectionFactor = MAX_BPB_FACTOR;
	}
	else if ( (CorrectionFactor < 99) && 
		   	  (cpi->pb.quantizer->FrameQIndex < cpi->Configuration.ActiveBestQuality ) )
	{
        // We are not already at the best allowable quality
		CorrectionFactor = 100 - ((100 - CorrectionFactor)/4);
		if ( CorrectionFactor < 80 )    // Damp the adjustment
			BpbCorrectionFactor = (BpbCorrectionFactor * 80)/100;
		else
			BpbCorrectionFactor = (BpbCorrectionFactor * CorrectionFactor) / 100;

		// Keep BpbCorrectionFactor within limits
		if ( BpbCorrectionFactor < MIN_BPB_FACTOR )
			 BpbCorrectionFactor = MIN_BPB_FACTOR;
	}

	if ( VP6_GetFrameType(&cpi->pb) == BASE_FRAME )
		cpi->KeyFrameBpbCorrectionFactor = BpbCorrectionFactor;
	else
	{
		if ( cpi->pb.RefreshGoldenFrame )
			cpi->GfuBpbCorrectionFactor = BpbCorrectionFactor;
		else
			cpi->BpbCorrectionFactor = BpbCorrectionFactor;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     ClampAndUpdateQ 
 *
 *  INPUTS        :     CP_INSTANCE *cpi : Pointer to encoder instance.
 *						UINT32 QIndex    : Current Q Index.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Ensures that the specified Q index is within current
 *						active range and applies other constraints.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void ClampAndUpdateQ ( CP_INSTANCE *cpi, UINT32 QIndex ) 
{   
	// Apply limits to the value of QIndex
	// NOTE: Bigger QIndex ==> Higher Quality (Lower Quantizer)!!!!
	if ( QIndex > cpi->Configuration.ActiveBestQuality )
		QIndex = cpi->Configuration.ActiveBestQuality;
	else if ( QIndex < cpi->Configuration.ActiveWorstQuality )
		QIndex = cpi->Configuration.ActiveWorstQuality;

    // Apply range restrictions for key frames.
    if ( VP6_GetFrameType(&cpi->pb) == BASE_FRAME )
    {
		// Fixed Q Stuff for key frames
		if ( cpi->FixedQ >= 0 )
		{
			UINT8 Q;

			// Set an appropriate Key frame Q to match the recent ambient quality
			if ( (cpi->LastKeyFrame >= cpi->ForceKeyFrameEvery) )
				Q = cpi->FixedQ + (FixedQKfBoostTable[cpi->FixedQ]/2);
			else
				Q = cpi->FixedQ + FixedQKfBoostTable[cpi->FixedQ];

			cpi->pb.quantizer->FrameQIndex = Q;
		}   
		else
		{
			// Additional QIndex limits for Key frames
            if( cpi->pass != 2) 
            {
			    if ( QIndex < KF_WORST_Q_INDEX )
				    QIndex = KF_WORST_Q_INDEX;
			    else if ( QIndex > 60 )
				    QIndex = 60;
            }

			cpi->pb.quantizer->FrameQIndex = QIndex;
		}

		// We are going to update GF this frame so reset counter till next update due.
        if(cpi->pass < 2)
    		cpi->GfUpdateInterval = DEFAULT_GF_UPDATE_INTERVAL;
        else
            cpi->GfUpdateInterval = DEFAULT_2PASS_GF_UPDATE_INTERVAL;

		cpi->FramesTillGfUpdateDue = cpi->GfUpdateInterval;
		
		if ( cpi->GfUpdateInterval )
			cpi->GfuMotionSpeed = GF_UPDATE_MOTION_INTERVAL / cpi->GfUpdateInterval;
		else
			cpi->GfuMotionSpeed = 0;

		cpi->GfuMotionComplexity = GF_DEFAULT_MOTION_CMPLX;
		cpi->GfuBoost = 0;
	}
	else 
	{
		if(cpi->FixedQ >= 0) 
		{
			// We want KFs to count as GF updates
			cpi->pb.quantizer->FrameQIndex = cpi->FixedQ;

            if(!cpi->DisableGolden)
            {
				if ( cpi->FramesTillGfUpdateDue == 0 )
                {
					UINT32 Sum = 0;
					UINT32 Sum2 = 0;
					UINT32 Sum3 = 0;
					UINT32 i;
					UINT32 VarianceX = 0;
					UINT32 VarianceY = 0;
					UINT32 MaxVariance = 0;

					// Check the level of MV reuse as a measure of how valuable a GF update is likely to be.
					for ( i = 0; i < MAX_MODES; i++ )
						Sum += cpi->ModeDist[i];

					if ( Sum )
					{
						Sum2 = Sum - (cpi->ModeDist[CODE_INTRA] + cpi->ModeDist[CODE_INTER_PLUS_MV] + cpi->ModeDist[CODE_INTER_FOURMV]);
						Sum3 = Sum2 - cpi->ModeDist[CODE_INTER_NO_MV];			

						// Convert Sum2 and Sum3 to %
						Sum2 = (Sum2 * 100 / Sum);						
						Sum3 = (Sum3 * 100 / Sum);							
					}

					// Calculate various motion metrics
					if ( cpi->FrameMvStats.NumMvs )
					{
						cpi->GfuMotionSpeed = (cpi->FrameMvStats.SumAbsX > cpi->FrameMvStats.SumAbsY) ? (cpi->FrameMvStats.SumAbsX/cpi->FrameMvStats.NumMvs) : (cpi->FrameMvStats.SumAbsY/cpi->FrameMvStats.NumMvs);
						VarianceX = ((cpi->FrameMvStats.NumMvs * cpi->FrameMvStats.SumXSq) - (cpi->FrameMvStats.SumX*cpi->FrameMvStats.SumX)) / (cpi->FrameMvStats.NumMvs * cpi->FrameMvStats.NumMvs);
						VarianceY = ((cpi->FrameMvStats.NumMvs * cpi->FrameMvStats.SumYSq) - (cpi->FrameMvStats.SumY*cpi->FrameMvStats.SumY)) / (cpi->FrameMvStats.NumMvs * cpi->FrameMvStats.NumMvs);
						MaxVariance = (VarianceX > VarianceY) ? VarianceX : VarianceY;
						cpi->GfuMotionComplexity = cpi->GfuMotionSpeed + ((VarianceX)/4) + ((VarianceY)/4);
						if ( cpi->GfuMotionComplexity > 31 )
							cpi->GfuMotionComplexity = 31;
					}	
					else
					{
						cpi->GfuMotionSpeed = 0;
						cpi->GfuMotionComplexity = 0;
					}

					// Should we even consider a GF update or is there no point
					if ( (Sum2 > GF_MODE_DIST_THRESH1) && (Sum3 > GF_MODE_DIST_THRESH2) &&
						 (cpi->GfuMotionSpeed <= MAX_GF_UPDATE_MOTION) && 
						 (MaxVariance <= GF_MAX_VAR_THRESH) )
					{
						cpi->pb.quantizer->FrameQIndex = cpi->FixedQ + GfFixedQKfBoostTable[cpi->FixedQ];

						cpi->pb.RefreshGoldenFrame = TRUE;
					}
					else
					{
						cpi->pb.quantizer->FrameQIndex = cpi->FixedQ;
					}
	            }
                else
                {

                    cpi->pb.quantizer->FrameQIndex = cpi->FixedQ;
                }
            }
		}
		else
		{
			cpi->pb.quantizer->FrameQIndex = QIndex;
		}
	}
    
    // If necessary re-initialise the quantiser
    VP6_UpdateQC( cpi->pb.quantizer, cpi->pb.Vp3VersionNo );
}

/****************************************************************************
 * 
 *  ROUTINE       :     RegulateQ
 *
 *  INPUTS        :     CP_INSTANCE *cpi : Pointer to encoder instance.
 *						INT32 TargetBits : Target number of bits for frame.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     This function tries to regulate quanitzer level
 *                      to produce the specified target number of bits.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void RegulateQ ( CP_INSTANCE *cpi, INT32 TargetBits ) 
{   
    UINT32 i;
    double Predbpb;

    UINT32 QIndex = Q_TABLE_SIZE - 1;
    double Targetbpb = (double)TargetBits / (double)cpi->pb.UnitFragments;
    double LastBitError = 10000.0;       // Infeasibly high number to initialize

    // Search for the best Q for the target bitrate.
	for ( i=0; i<Q_TABLE_SIZE; i++ )
	{
        Predbpb = GetEstimatedBpb( cpi, i );
        if ( Predbpb > Targetbpb )
        {
            if ( (Predbpb - Targetbpb) <= LastBitError )
                QIndex = i;
            else
                QIndex = i - 1;
            break;
        }
        else
            LastBitError = Targetbpb - Predbpb;
    }

    ClampAndUpdateQ ( cpi, QIndex );
}

/****************************************************************************
 * 
 *  ROUTINE       :     ConfigureQuality
 *
 *  INPUTS        :     CP_INSTANCE *cpi    : Pointer to encoder instance.
 *						UINT32 QualityValue : Quality value.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Sets maximum operating Q value for specified
 *                      quality level.
 *
 *  SPECIAL NOTES :     None.
 *
 ****************************************************************************/
void ConfigureQuality ( CP_INSTANCE *cpi, UINT32 QualityValue )
{
    // Set the worst case quality value.
    // Note that the actual quality is determined by lookup into the quantiser table QThreshTable[]
    cpi->Configuration.WorstQuality = 63 - QualityValue;

    // Set the default Active WorstQuality.
    cpi->Configuration.ActiveWorstQuality = cpi->Configuration.WorstQuality;
}
