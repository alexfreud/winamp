/****************************************************************************
*
*   Module Title :     Quantise
*
*   Description  :     Quantisation and dequanitsation of an 8x8 dct block. .
*
****************************************************************************/						
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Frames
****************************************************************************/
#include "quantize.h"
#include "duck_mem.h"
#include <stddef.h>
/****************************************************************************
*  Module Statics
****************************************************************************/ 
#define MIN16 ((1<<16)-1)
      
// Scale factors used to improve precision of DCT/IDCT
#define IDCT_SCALE_FACTOR       2       // Shift left bits to improve IDCT precision

// AC Quantizer Tables
static const UINT32 VP6_QThreshTable[Q_TABLE_SIZE] = 
{   94, 92, 90, 88, 86, 82, 78, 74,
    70, 66, 62, 58, 54, 53, 52, 51,
	50, 49, 48, 47, 46, 45, 44, 43,
	42,	40, 39, 37, 36, 35, 34, 33,
    32, 31, 30, 29, 28, 27, 26, 25, 
    24, 23, 22, 21, 20, 19, 18, 17,
    16, 15, 14, 13, 12, 11, 10,  9,  
    8,   7,  6,  5,  4,  3,  2,  1
};

static const UINT32 VP6_UvQThreshTable[Q_TABLE_SIZE] = 
{   94, 92, 90, 88, 86, 82, 78, 74,
    70, 66, 62, 58, 54, 53, 52, 51,
	50, 49, 48, 47, 46, 45, 44, 43,
	42,	40, 39, 37, 36, 35, 34, 33,
    32, 31, 30, 29, 28, 27, 26, 25, 
    24, 23, 22, 21, 20, 19, 18, 17,
    16, 15, 14, 13, 12, 11, 10,  9,  
    8,   7,  6,  5,  4,  3,  2,  1
};

// AC Zero Bin and Rounding Tables (include fdct normalisation)
static const UINT32 VP6_ZBinTable[Q_TABLE_SIZE] = 
{
	330,314,298,284,264,246,228,213,
	201,190,178,167,156,153,149,146,
	144,141,138,135,132,130,127,124,
	121,115,110,104, 99, 96, 94, 90,
	 85, 82, 79, 76, 74, 71, 69, 66,
	 63, 61, 58, 55, 53, 50, 47, 45,
	 43, 40, 38, 36, 33, 31, 28, 24,
	 21, 18, 16, 13, 10,  7,  4,  2
};

static const UINT32 VP6_UvZBinTable[Q_TABLE_SIZE] = 
{
	330,314,298,284,264,246,228,213,
	201,190,178,167,156,153,149,146,
	144,141,138,135,132,130,127,124,
	121,115,110,104, 99, 96, 94, 90,
     85, 82, 79, 76, 74, 71, 69, 66,
	 63, 61, 58, 55, 53, 50, 47, 45,
	 43, 40, 38, 36, 33, 31, 28, 24,
	 21, 18, 16, 13, 10,  7,  4,  2
};

static const UINT32 VP6_RTable[Q_TABLE_SIZE] = 
{
	48, 56, 64, 70, 78, 82, 86, 88,
	91, 92, 94, 94, 99,103,102,100,
	99, 97,	95, 93, 91, 89, 87, 85, 
	83, 79, 77, 73, 71, 69, 67, 65,
	64, 62, 60, 58, 56, 54, 52, 50,
	48, 46, 44, 42, 40, 38, 36, 34,
	32, 30, 28, 26, 24, 22, 20, 18, 
	16, 14, 12, 10,  8,  6,  4,  2
};

static const UINT32 VP6_UvRTable[Q_TABLE_SIZE] = 
{
	48, 56, 64, 70, 78, 82, 86, 88,
	91, 92, 94, 94, 99,103,102,100,
	99, 97,	95, 93, 91, 89, 87, 85, 
	83, 79, 77, 73, 71, 69, 67, 65,
	64, 62, 60, 58, 56, 54, 52, 50,
	48, 46, 44, 42, 40, 38, 36, 34,
	32, 30, 28, 26, 24, 22, 20, 18, 
	16, 14, 12, 10,  8,  6,  4,  2
};

// DC Quantizer tables
const Q_LIST_ENTRY VP6_DcQuant[ Q_TABLE_SIZE ] = 
{
	47, 47, 47, 47, 45, 43, 43, 43, 
	43, 43,	42, 41, 41, 40, 40, 40, 
	40, 35,	35, 35, 35, 33, 33, 33, 
	33, 32, 32, 32, 27, 27, 26, 26,
	25, 25, 24, 24, 23, 23,	19, 19,
    19, 19, 18, 18, 17, 16, 16, 16, 
    16, 16, 15, 11, 11, 11, 10, 10,
     9,  8,  7,  5,  3,  3,  2,  2    
};

static const Q_LIST_ENTRY VP6_UvDcQuant[ Q_TABLE_SIZE ] = 
{
	47, 47, 47, 47, 45, 43, 43, 43, 
	43, 43,	42, 41, 41, 40, 40, 40, 
	40, 35,	35, 35, 35, 33, 33, 33, 
	33,	32, 32, 32, 27, 27, 26, 26,
	25, 25, 24, 24, 23, 23,	19, 19,
    19, 19, 18, 18, 17, 16, 16, 16,
    16, 16, 15, 11, 11, 11, 10, 10,
     9,  8,  7,  5,  3,  3,  2,  2    
};

// DC Zero Bin and Rounding Tables (include fdct normalisation)
static const UINT32 VP6_DcZBinTable[Q_TABLE_SIZE] = 
{
	170,162,152,150,140,130,125,121,
	121,118,113,111,110,108,108,106,
	105,96, 93, 87, 86, 83, 83, 83, 
	83, 78, 78, 78, 66, 66, 63, 63,
	61, 61, 58, 58, 56, 56, 46, 46,
	46, 46, 43, 43, 41, 38, 38, 38,
    38, 38, 35, 24, 24, 24, 23, 23, 
	20, 19, 16, 13,  6,  6,  4,  4
};

static const UINT32 VP6_UvDcZBinTable[Q_TABLE_SIZE] = 
{
	170,162,152,150,140,130,125,121,
	121,118,113,111,110,108,108,106,
	105,96, 93, 87, 86, 83, 83, 83, 
	83, 78, 78, 78, 66, 66, 63, 63,
	61, 61, 58, 58, 56, 56, 46, 46,
	46, 46, 43, 43, 41, 38, 38, 38,
    38, 38, 35, 24, 24, 24, 23, 23, 
	20, 19, 16, 13,  6,  6,  4,  4
};

static const UINT32 VP6_DcRTable[Q_TABLE_SIZE] = 
{
	20, 28, 38, 40, 44, 46, 50, 50, 
	51, 57,	59, 61, 62, 64, 66, 67, 
	67, 62,	63, 64, 64, 62, 62, 62, 
	62,	62, 62, 62, 54, 54, 52, 52,
	50, 50, 48, 48, 46, 46, 38, 38,
	38, 38, 36, 36, 34, 32, 32, 32,
	32, 32, 30, 22, 22, 22, 20, 20, 
	18, 16, 14, 10,  6,  6,  4,  4
};

static const UINT32 VP6_UvDcRTable[Q_TABLE_SIZE] = 
{
	20, 30, 38, 40, 44, 46, 50, 50, 
	51, 57,	59, 61, 62, 64, 66, 67, 
	67, 62,	63, 64, 64, 62, 62, 62, 
	62,	62, 62, 62, 54, 54, 52, 52, 
	50, 50, 48, 48, 46, 46, 38, 38,
	38, 38, 36, 36, 34, 32, 32, 32,
	32, 32, 30, 22, 22, 22, 20, 20, 
	18, 16, 14, 10,  6,  6,  4,  4
};


// Correction factors for ZBin size.based upon zero run length leading up to the current coef
// The factor is A % of the bin width to be added to the existing zero bin.
static const INT32 VP6_ZlrZbinCorrection[Q_TABLE_SIZE] = 
{
    -8,  0,  5, 10, 10, 10, 10, 10, 
	15, 15, 15, 15, 20, 20, 20, 20, 
	20, 20, 20, 20, 20, 20, 20, 20, 
	20, 20, 20, 20, 20, 20, 20, 20, 
	20, 20, 20, 20, 20, 20, 20, 20, 
	25, 25, 25, 25, 25, 25, 25, 25,  
	25, 25, 25, 25, 25, 25, 25, 25, 
	30, 30, 30, 30, 30, 30, 30, 30, 
};


/****************************************************************************
 *
 *	Inverse fast DCT index:
 *
 *	This contains the offsets needed to convert zigzag order into x, y order
 *  for decoding. It is generated from the input zigzag	index at at run time.												
 *
 *	For maximum speed during both quantisation and dequantisation we maintain
 *  separate quantisation and zigzag tables for each operation.														
 *
 *	qi->quant_index  :	zigzag index used during quantisation	
 *	dequant_index    :	zigzag index used during dequantisation			
 *					
 *  qi->quant_index is the inverse of dequant_index and is calculated during
 *  initialisation.
 *
 ****************************************************************************/
static const UINT32 dequant_index[64] = 
{	0,  1,  8,  16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

static const UINT32 transIndexC[64] = 
{
	 0,	 1,	 2,	 3,	   4,  5,  6,  7,
	 8,	 9, 10,	11,	  12, 13, 14, 15,
	16, 17, 18, 19,   20, 21, 22, 23,
	24, 25, 26, 27,   28, 29, 30, 31,

	32, 33, 34, 35,   36, 37, 38, 39,
	40, 41, 42, 43,   44, 45, 46, 47,
	48, 49, 50, 51,   52, 53, 54, 55, 
	56, 57, 58, 59,   60, 61, 62, 63
};

static const UINT32 quant_indexC[64] = 
{
    0,  1,   5,  6, 14, 15, 27, 28,
    2,  4,   7, 13, 16, 26, 29, 42,
    3,  8,  12, 17, 25, 30, 41, 43,
    9,  11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54, 
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

/****************************************************************************
*  Imports
****************************************************************************/
void (*VP6_BuildQuantIndex)( QUANTIZER * qi);
void (*VP6_quantize)( QUANTIZER *qi, INT16 * DCT_block, Q_LIST_ENTRY * quantized_list, UINT8 bp );

/****************************************************************************
*  Exports
****************************************************************************/
const UINT8 VP6_QTableSelect[6] = { 0,0,0,0,1,1 };	// Controls selection of Q Table,rounding,zero bin etc for Y, U & V blocks

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_InitQTables
 *
 *  INPUTS        :     QUANTIZER *qi     : Pointer to quantizer instance.
 *                      UINT8 Vp3VersionNo : Decoder version number (NOT USED).
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Initialises Q table.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_InitQTables ( QUANTIZER *qi, UINT8 Vp3VersionNo )
{  
	memcpy ( qi->QThreshTable, VP6_QThreshTable, sizeof(qi->QThreshTable) );
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_BuildQuantIndex_Generic
 *
 *  INPUTS        :     QUANTIZER *qi : Pointer to quantizer instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Builds the quant_index table.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_BuildQuantIndex_Generic ( QUANTIZER *qi )
{
    INT32 i,j;

    qi->transIndex = (UINT32 *)transIndexC;

    // invert the dequant index into the quant index
	for ( i=0; i<BLOCK_SIZE; i++ )
	{	
        j = dequant_index[i];
		qi->quant_index[j] = i;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_init_dequantizer
 *
 *  INPUTS        :     QUANTIZER *qi      : Pointer to quantizer instance.
 *                      UINT8 Vp3VersionNo : Decoder version number (NOT USED)
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Performs initialization of the dequantizer.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_init_dequantizer ( QUANTIZER *qi, UINT8 Vp3VersionNo )
{
    int	i, j;						 

	// *************** Y ******************/
	
    // AC: set-up the dequant values and then place in the zig-zag/transposed order.
	for ( i=1; i<64; i++ )
	{	
        j = qi->quant_index[i];
		qi->dequant_coeffs[0][j] = VP6_QThreshTable[qi->FrameQIndex] << IDCT_SCALE_FACTOR;
    }
	
    // DC
    qi->dequant_coeffs[0][0] = VP6_DcQuant[qi->FrameQIndex] << IDCT_SCALE_FACTOR;

	// *************** UV ******************/

    // AC: set-up the dequant values and then place in the zig-zag/transposed order.
	for ( i=1; i<64; i++ )
	{	
        j = qi->quant_index[i];
		qi->dequant_coeffs[1][j] = VP6_UvQThreshTable[qi->FrameQIndex] << IDCT_SCALE_FACTOR;
    }
	
    // DC
    qi->dequant_coeffs[1][0] = VP6_UvDcQuant[qi->FrameQIndex] << IDCT_SCALE_FACTOR;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_UpdateQ
 *
 *  INPUTS        :     QUANTIZER *qi      : Pointer to quantizer instance.
 *                      UINT8 Vp3VersionNo : Decoder version number.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Updates the quantisation tables for a new Q.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_UpdateQ ( QUANTIZER *qi, UINT8 Vp3VersionNo )
{  
	if ( qi->FrameQIndex == qi->LastFrameQIndex )
		return;

	// Update the record of last Q index.
    qi->LastFrameQIndex = qi->FrameQIndex;

	// Invert the dequant index into the quant index --
    // the decoder has a different order than the encoder.
    VP6_BuildQuantIndex(qi);

	// Re-initialise the q tables for forward and reverse transforms.    
	VP6_init_dequantizer ( qi, Vp3VersionNo );
}

/********************* COMPRESSOR SPECIFIC **********************************/

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_init_quantizer
 *
 *  INPUTS        :     QUANTIZER *qi      : Pointer to quantizer instance.
 *                      UINT8 Vp3VersionNo : Decoder version number (NOT USED).
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Updates the quantisation tables for a new Q.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
#define SHIFT16 (1<<16)
void VP6_init_quantizer ( QUANTIZER *qi, UINT8 Vp3VersionNo )
{
    int i;
    double temp_fp_quant_coeffs;

    // Notes on setup of quantisers:
    // The "* 4" is a normalisation factor for the forward DCT transform.
	
	// ******************* Y *********************

    // Calculate DC quant values (Include a *4 for FDCT normalization)
	temp_fp_quant_coeffs =  (double)( VP6_DcQuant[qi->FrameQIndex] * 4 ); 

	// 1/X (Y)
	temp_fp_quant_coeffs = 1.0 / temp_fp_quant_coeffs;
	qi->QuantCoeffs[0][0] = (INT32) (0.5 + SHIFT16 * temp_fp_quant_coeffs);

	// DC rounding (Y)
	qi->QuantRound[0][0] = VP6_DcRTable[qi->FrameQIndex];

	// Set DC zero Bin (Y)
	qi->ZeroBinSize[0][0] = VP6_DcZBinTable[qi->FrameQIndex];
   
	// AC for Y
	for ( i=1; i<64; i++ )
	{
		// Normalize the quantizer (* 4 for fdct normalisation)
		temp_fp_quant_coeffs =  (double)(VP6_QThreshTable[qi->FrameQIndex] * 4);

		// Convert to 1/x
		temp_fp_quant_coeffs = 1.0 / temp_fp_quant_coeffs;
		qi->QuantCoeffs[0][i] = (INT32) (0.5 + SHIFT16 * temp_fp_quant_coeffs);

		// AC rounding
		qi->QuantRound[0][i] = VP6_RTable[qi->FrameQIndex];

		// Zero Bins
		qi->ZeroBinSize[0][i] = VP6_ZBinTable[qi->FrameQIndex];
	}

	// ******************* UV *********************

    // Calculate DC quant values (Include a *4 for FDCT normalization)
	temp_fp_quant_coeffs =  (double)( VP6_UvDcQuant[qi->FrameQIndex] * 4 ); 

	// 1/X (UV)
	temp_fp_quant_coeffs = 1.0 / temp_fp_quant_coeffs;
	qi->QuantCoeffs[1][0] = (INT32) (0.5 + SHIFT16 * temp_fp_quant_coeffs);

	// DC rounding (UV)
	qi->QuantRound[1][0] = VP6_UvDcRTable[qi->FrameQIndex];

	// Set DC zero Bin (UV)
	qi->ZeroBinSize[1][0] = VP6_UvDcZBinTable[qi->FrameQIndex];
   
	// AC for UV
	for ( i=1; i<64; i++ )
	{
		// Normalize the quantizer (* 4 for fdct normalisation)
		temp_fp_quant_coeffs =  (double)(VP6_UvQThreshTable[qi->FrameQIndex] * 4);

		// 1/x
		temp_fp_quant_coeffs = 1.0 / temp_fp_quant_coeffs;
		qi->QuantCoeffs[1][i] = (INT32) (0.5 + SHIFT16 * temp_fp_quant_coeffs);

		// AC rounding
		qi->QuantRound[1][i] = VP6_UvRTable[qi->FrameQIndex];

		// Zero Bins
		qi->ZeroBinSize[1][i] = VP6_UvZBinTable[qi->FrameQIndex];
	}

	for ( i=0; i<8; i++ )
	{
		qi->round[i] = qi->QuantRound[0][1];
		qi->mult[i] = qi->QuantCoeffs[0][1];
		qi->zbin[i] = qi->ZeroBinSize[0][1]-1;
	}


	// Work out the ZRL correction factors for ZBIN
	for ( i = 0; i < 64; i++ )
	{
		qi->ZlrZbinCorrections[0][i] = ((INT32)VP6_QThreshTable[qi->FrameQIndex] * 4 * VP6_ZlrZbinCorrection[i]) / 100;
		qi->ZlrZbinCorrections[1][i] = ((INT32)VP6_UvQThreshTable[qi->FrameQIndex] * 4 * VP6_ZlrZbinCorrection[i]) / 100;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     UpdateQC (compressor's update q)
 *
 *  INPUTS        :     QUANTIZER *qi      : Pointer to quantizer instance.
 *                      UINT8 Vp3VersionNo : Decoder version number.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Updates the quantisation tables for a new Q
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_UpdateQC ( QUANTIZER *qi, UINT8 Vp3VersionNo )
{  
	if ( qi->FrameQIndex == qi->LastFrameQIndex )
		return;

	// Update the record of last Q index.
    qi->LastFrameQIndex = qi->FrameQIndex;

	// Invert the dequant index into the quant index --
    // the decoder has a different order than the encoder.
    VP6_BuildQuantIndex_Generic(qi);

    // Re-initialise the q tables for forward and reverse transforms.    
    VP6_init_quantizer ( qi, Vp3VersionNo );
	VP6_init_dequantizer ( qi, Vp3VersionNo );
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_quantize_c
 *
 *  INPUTS        :     QUANTIZER *qi               : Pointer to quantizer instance.
 *                      INT16 *DCT_block             : List of 64 DCT coefficients.
 *                      UINT8 bp                     : Position of block within MB.
 *
 *  OUTPUTS       :     Q_LIST_ENTRY *quantized_list : List of 64 quantized DCT coefficients.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Quantizes the DCT coefficients wrt the current 
 *                      quantization level.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
#define HIGHBITDUPPED(X) (((signed short) X)  >> 15)

void VP6_quantize_c( QUANTIZER *qi, INT16 *DCT_block, Q_LIST_ENTRY *quantized_list, UINT8 bp )
{
    UINT32  i, j;
	INT32	temp;
	UINT32	ColourPlane = VP6_QTableSelect[bp]; 
    
    INT32 * QuantRoundPtr  = qi->QuantRound[ColourPlane];
    INT32 * QuantCoeffsPtr = qi->QuantCoeffs[ColourPlane];
    INT32 * ZBinPtr        = qi->ZeroBinSize[ColourPlane];
	INT32 * ZrlCorrection = qi->ZlrZbinCorrections[ColourPlane];
    INT16 * DCT_blockPtr   = DCT_block;

	UINT8   Zrl = 0;

    // Set the quantized_list to default to 0
    memset( quantized_list, 0, 64 * sizeof(Q_LIST_ENTRY) );

	// DC quantization 
	if ( DCT_blockPtr[0] >= ZBinPtr[0] )
	{
		temp = QuantCoeffsPtr[0] * ( DCT_blockPtr[0] + QuantRoundPtr[0] );
		quantized_list[0] = (Q_LIST_ENTRY) (temp>>16);
	}
    else if ( DCT_blockPtr[0] <= -ZBinPtr[0] )
	{
		temp = QuantCoeffsPtr[0] * ( DCT_blockPtr[0] - QuantRoundPtr[0] ) + MIN16;
		quantized_list[0] = (Q_LIST_ENTRY) (temp>>16);
	}
	else
		Zrl++;

    // Quantize AC 
    for( i=1; i<64; i++ )
    {
		// Zig Zag order... 
		j = dequant_index[i];

        if ( DCT_blockPtr[j] >= (ZBinPtr[j] + ZrlCorrection[Zrl]) )
        {
			temp = QuantCoeffsPtr[j] * ( DCT_blockPtr[j] + QuantRoundPtr[j] );
			quantized_list[i] = (Q_LIST_ENTRY) (temp>>16);
			Zrl = 0;
        }
        else if ( DCT_blockPtr[j] <= -(ZBinPtr[j] + ZrlCorrection[Zrl]) )
        {
			temp = QuantCoeffsPtr[j] * ( DCT_blockPtr[j] - QuantRoundPtr[j] ) + MIN16;
			quantized_list[i] = (Q_LIST_ENTRY) (temp>>16);
			Zrl = 0;
        }
		else
			Zrl++;
    }

}
/**************************** END COMPRESSOR SPECIFIC **********************************/

/****************************************************************************
 * 
 *  ROUTINE       :     DeleteQuantizerBuffers
 *
 *  INPUTS        :     QUANTIZER *qi : Pointer to quantizer instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 * 
 *  FUNCTION      :     De-allocates buffers associated with the quantizer.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
static void DeleteQuantizerBuffers ( QUANTIZER *qi )
{
	if ( qi->dequant_coeffsAlloc[0] )
		duck_free(qi->dequant_coeffsAlloc[0]);
	qi->dequant_coeffsAlloc[0]	= 0;
	qi->dequant_coeffs[0]		= 0;

	if ( qi->dequant_coeffsAlloc[1] )
		duck_free(qi->dequant_coeffsAlloc[1]);
	qi->dequant_coeffsAlloc[1]	= 0;
	qi->dequant_coeffs[1]		= 0;
}

/****************************************************************************
 * 
 *  ROUTINE       :     AllocateQuantizerBuffers
 *
 *  INPUTS        :     QUANTIZER *qi : Pointer to quantizer instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     INT32: Always TRUE.
 *
 *  FUNCTION      :     Allocates buffers associated with quantization.
 *
 *  SPECIAL NOTES :     Uses ROUNDUP32 to ensure that allocated buffers are
 *                      aligned on 32-byte boundaries to improve cache performance. 
 *
 ****************************************************************************/

// TODO: benski> need better checks for other compilers
#if defined(_M_AMD64) || defined(__LP64__)
#define ROUNDUP32(X) ( ( ( (uintptr_t) X ) + 31 )&( 0xFFFFFFFFFFFFFFE0 ) )
#else //#elif //defined(_M_IX86) 
#define ROUNDUP32(X) ( ( ( (unsigned long) X ) + 31 )&( 0xFFFFFFE0 ) )
#endif


static INT32 AllocateQuantizerBuffers ( QUANTIZER *qi )
{
	DeleteQuantizerBuffers(qi);

	qi->dequant_coeffsAlloc[0]		    = (INT16 *)duck_malloc(32+64*sizeof(INT16), DMEM_GENERAL);
    if ( !qi->dequant_coeffsAlloc[0] ) { DeleteQuantizerBuffers(qi); return FALSE; };
	qi->dequant_coeffs[0]			    = (INT16 *)ROUNDUP32(qi->dequant_coeffsAlloc[0]);

	qi->dequant_coeffsAlloc[1]		    = (INT16 *)duck_malloc(32+64*sizeof(INT16), DMEM_GENERAL);
    if ( !qi->dequant_coeffsAlloc[1] ) { DeleteQuantizerBuffers(qi); return FALSE; };
	qi->dequant_coeffs[1]			    = (INT16 *)ROUNDUP32(qi->dequant_coeffsAlloc[1]);

	return TRUE;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_DeleteQuantizer
 *
 *  INPUTS        :     QUANTIZER **qi : Pointer to pointer to quantizer instance.
 *
 *  OUTPUTS       :     QUANTIZER **qi : Pointer to pointer to quantizer instance,
 *                                       set to NULL on exit.
 *
 *  RETURNS       :     void.
 *
 *  FUNCTION      :     De-allocates memory associated with the quantizer.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_DeleteQuantizer ( QUANTIZER **qi )
{
    if ( *qi )
    {
        // Delete any other dynamically allocaed temporary buffers
		DeleteQuantizerBuffers(*qi);

        // De-allocate the quantizer
        duck_free(*qi);
		*qi=0;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_CreateQuantizer
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Pointer to allocated quantizer instance.
 *
 *  FUNCTION      :     Allocated memory for and initializes a quantizer instance.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
QUANTIZER *VP6_CreateQuantizer ( void )
{
	QUANTIZER *qi = 0;
	int quantizer_size = sizeof(QUANTIZER);
	
    qi = (QUANTIZER *) duck_malloc(quantizer_size, DMEM_GENERAL); 
    if ( !qi )
        return 0;

	// initialize whole structure to 0
	memset ( (unsigned char *)qi, 0, quantizer_size );
	
	if ( !AllocateQuantizerBuffers(qi) )
		VP6_DeleteQuantizer(&qi);

	return qi;
}


/****************************************************************************
 * 
 *  ROUTINE       : GetQuantizedCoeffsMSE_RD
 *
 *  INPUTS        : CP_INSTANCE *cpi        : Pointer to encoder instance.
 *                  INT16 * DctCodes        : Result of Forward DCT
 *	                INT16 * Coeffs,         : Quantized Coeffs
 *	                INT16 * DequantMatrix,  : Dequantizaton Matrix
 *	                
 *
 *  OUTPUTS       : UINT32 *MSE             : Mean Square Error
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Computer MSE in transform domain.
 *
 *  SPECIAL NOTES : From the arguement that the mse in frequency domain
 *                  is same as the mse in spatial domain, this routine 
 *                  calculate the mse in transform domain to saving the
 *                  idct and recon operations for distortion measurement.
 *
 ****************************************************************************/
void GetQuantizedCoeffsMSE_RD
( 
    INT16 * DctCodes,
    INT16 * Coeffs,
    INT16 * DequantMatrix,
    UINT32 *MSE
)
{
    UINT32 Error=0;
    INT32 i;    
    INT32 diff;


    for(i=0;i<64;i++)
    {
        int j = dequant_index[i];
        diff = Coeffs[i] * DequantMatrix [i] - DctCodes[j];        
        Error += diff*diff;
    }

    *MSE = (Error<<2);
}


