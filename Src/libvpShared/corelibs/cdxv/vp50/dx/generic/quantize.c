/****************************************************************************
*
*   Module Title :     Quantise
*
*   Description  :     Quantisation and dequanitsation of an 8x8 dct block. .
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
* 
*
*   1.18 PGW 03 Dec 01 Changes to available Q values.
*   1.17 PGW 14 Sep 01 Added support for ZB varying on zero-run.	
*   1.16 JBX 22-Mar-01 Merged with vp4-mapca bitstream
*   1.15 PGW 19 Oct 00 Added select_InterUV_quantiser and related data structures
*	  				   to support use of different DC  behaviour for UV.
*   1.11 PGW 18 Sep 00 QThreshTable[] and Inter_coeffs[] made instacne specific.
*	1.10 PGW 14 Sep 00 Added support for different Q, ZB and Rounding tables 
*					   in different encoder versions.
*   1.09 PGW 04 Sep 00 Fixed bugs in code to set up rounding and zero bins
*					   Added support for ZB to change with Q and coefficient.
*   1.08 PGW 29 Aug 00 Correction to UpdateQ() and UpdateQC() re. Q limits.
*					   Changes to rounding and ZBF.
*   1.08 JBB 22 Aug 00 Ansi C conversion
*   1.07 SJL 14/04/00  Added the BuildQuantIndex function.
*   1.06 PGW 18/02/00  Rate targeting changes.
*	1.05 JBB 27/01/99  Globals Removed, use of QUANTIZER, Dequant no longer 
*                      used
*   1.04 PGW 05/11/99  Changes to support AC range entropy tables
*   1.03 PGW 12/10/99  Removal of spurious windows dependancies.
*   1.02 PGW 14/09/99  Removal of some floating point code.
*   1.01 PGW 13/07/99  Changes to keep dequant output to 16 bit
*   1.01 PGW 07/07/99  Tweaks to baseline matrix.
*   1.00 PGW 18/06/99  Configuration baseline
*
*****************************************************************************
*/						

/****************************************************************************
*  Header Frames
*****************************************************************************
*/
#define STRICT              /* Strict type checking. */
#include <string.h>  
#include "quantize.h"
#include "duck_mem.h"

/****************************************************************************
*  Module constants.
*****************************************************************************
*/ 
#define MIN16 ((1<<16)-1)
      
// DC quantizer characteristics
#define VP5_MIN_QUANT		1
	
#define UV_Q_ADJUSTMENT		0

// Scale factors used to improve precision of DCT/IDCT
#define IDCT_SCALE_FACTOR       2       // Shift left bits to improve IDCT precision

/****************************************************************************
*  Imported Functions
*****************************************************************************
*/

/****************************************************************************
*  Imported Global Variables
*****************************************************************************
*/

/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/
void  (*VP5_BuildQuantIndex)( QUANTIZER * pbi);

UINT8 QTableSelect[6] = { 0,0,0,0,1,1 };	// Controls selection of Q Table,rounding,zero bin etc for Y, U & V blocks

/****************************************************************************
*  Foreward References
*****************************************************************************
*/    
void VP5_InitQTables( QUANTIZER *pbi, UINT8 Vp3VersionNo );
void VP5_BuildQuantIndex_Generic(QUANTIZER *pbi);
void VP5_UpdateQ( QUANTIZER *pbi, UINT8 Vp3VersionNo  );
void VP5_UpdateQC( QUANTIZER *pbi,UINT8 Vp3VersionNo  );
void VP5_init_quantizer ( QUANTIZER *pbi, UINT8 Vp3VersionNo );
void (*VP5_quantize)( QUANTIZER *pbi, INT16 * DCT_block, Q_LIST_ENTRY * quantized_list, UINT8 bp );
void VP5_init_dequantizer ( QUANTIZER *pbi, UINT8 Vp3VersionNo );
QUANTIZER * VP5_CreateQuantizer(void);
void VP5_DeleteQuantizer(QUANTIZER **pbi);          

/****************************************************************************
*  Module Statics
*****************************************************************************
*/      

// AC Quantizer Tables
static UINT32 VP5_QThreshTable[Q_TABLE_SIZE] = 
{   94, 92, 90, 88, 86, 82, 78, 74,
    70, 66, 62, 58, 54, 53, 52, 51,
	50, 49, 48, 47, 46, 45, 44, 43,
	42,	40, 39, 37, 36, 35, 34, 33,
    32, 31, 30, 29, 28, 27, 26, 25, 
    24, 23, 22, 21, 20, 19, 18, 17,
    16, 15, 14, 13, 12, 11, 10,  9,  
    8,   7,  6,  5,  4,  3,  2,  1
};
static UINT32 VP5_UvQThreshTable[Q_TABLE_SIZE] = 
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
UINT32 VP5_ZBinTable[Q_TABLE_SIZE] = 
{
	330,314,298,284,264,246,228,213,
	201,190,178,167,156,153,149,146,
	144,141,138,135,132,130,127,124,
	121,115,110,104,99, 96, 94, 90,
	85, 82, 79, 76, 74, 71, 69, 66,
	63, 61, 58, 55, 53, 50, 47, 45,
	43, 40, 38, 36, 33, 31, 28, 24,
	21, 18, 16, 13, 10, 7,  4,  2,
};
UINT32 VP5_UvZBinTable[Q_TABLE_SIZE] = 
{
	330,314,298,284,264,246,228,213,
	201,190,178,167,156,153,149,146,
	144,141,138,135,132,130,127,124,
	121,115,110,104,99, 96, 94, 90,
	85, 82, 79, 76, 74, 71, 69, 66,
	63, 61, 58, 55, 53, 50, 47, 45,
	43, 40, 38, 36, 33, 31, 28, 24,
	21, 18, 16, 13, 10, 7,  4,  2,
};
UINT32 VP5_RTable[Q_TABLE_SIZE] = 
{
	48, 56, 64, 70, 78, 82, 86, 88,
	91, 92, 94, 94, 99,103,102,100,
	99, 97,	95, 93, 91, 89, 87, 85, 
	83, 79, 77, 73, 71, 69, 67, 65,
	64, 62, 60, 58, 56, 54, 52, 50,
	48, 46, 44, 42, 40, 38, 36, 34,
	32, 30, 28, 26, 24, 22, 20, 18, 
	16, 14, 12, 10,  8,  6,  4,  2,
};
UINT32 VP5_UvRTable[Q_TABLE_SIZE] = 
{
	48, 56, 64, 70, 78, 82, 86, 88,
	91, 92, 94, 94, 99,103,102,100,
	99, 97,	95, 93, 91, 89, 87, 85, 
	83, 79, 77, 73, 71, 69, 67, 65,
	64, 62, 60, 58, 56, 54, 52, 50,
	48, 46, 44, 42, 40, 38, 36, 34,
	32, 30, 28, 26, 24, 22, 20, 18, 
	16, 14, 12, 10,  8,  6,  4,  2,
};

// DC Quantizer tables
Q_LIST_ENTRY VP5_DcQuant[ Q_TABLE_SIZE ] = 
{
	47, 47, 47, 47, 45, 43, 43, 43, 
	43, 43,	42, 41, 41, 40, 40, 40, 
	40, 35,	35, 35, 35, 33, 33, 33, 
	33, 32, 32, 32, 27, 27, 26, 26,
	25, 25, 24, 24, 23, 23,	19, 19,
    19, 19, 18, 18, 17, 16, 16, 16,
    16, 16, 15, 11, 11, 11, 10, 10,
     9,  8,  7,  5,  3,  3,  2,  2,    
};
Q_LIST_ENTRY VP5_UvDcQuant[ Q_TABLE_SIZE ] = 
{
	47, 47, 47, 47, 45, 43, 43, 43, 
	43, 43,	42, 41, 41, 40, 40, 40, 
	40, 35,	35, 35, 35, 33, 33, 33, 
	33,	32, 32, 32, 27, 27, 26, 26,
	25, 25, 24, 24, 23, 23,	19, 19,
    19, 19, 18, 18, 17, 16, 16, 16,
    16, 16, 15, 11, 11, 11, 10, 10,
     9,  8,  7,  5,  3,  3,  2,  2,    
};
// DC Zero Bin and Rounding Tables (include fdct normalisation)
UINT32 VP5_DcZBinTable[Q_TABLE_SIZE] = 
{
	170,162,152,150,140,130,125,121,
	121,118,113,111,110,108,108,106,
	105,96, 93, 87, 86, 83, 83, 83, 
	83, 78, 78, 78, 66, 66, 63, 63,
	61, 61, 58, 58, 56, 56, 46, 46,
	46, 46, 43, 43, 41, 38, 38, 38,
	38, 38, 35, 24, 24, 24, 23, 23, 
	20, 19, 16, 13,  6,  6,  4,  4,
};
UINT32 VP5_UvDcZBinTable[Q_TABLE_SIZE] = 
{
	170,162,152,150,140,130,125,121,
	121,118,113,111,110,108,108,106,
	105,96, 93, 87, 86, 83, 83, 83, 
	83, 78, 78, 78, 66, 66, 63, 63,
	61, 61, 58, 58, 56, 56, 46, 46,
	46, 46, 43, 43, 41, 38, 38, 38,
	38, 38, 35, 24, 24, 24, 23, 23, 
	20, 19, 16, 13,  6,  6,  4,  4,
};

UINT32 VP5_DcRTable[Q_TABLE_SIZE] = 
{
	20, 28, 38, 40, 44, 46, 50, 50, 
	51, 57,	59, 61, 62, 64, 66, 67, 
	67, 62,	63, 64, 64, 62, 62, 62, 
	62,	62, 62, 62, 54, 54, 52, 52,
	50, 50, 48, 48, 46, 46, 38, 38,
	38, 38, 36, 36, 34, 32, 32, 32,
	32, 32, 30, 22, 22, 22, 20, 20, 
	18, 16, 14, 10,  6,  6,  4,  4,
};
UINT32 VP5_UvDcRTable[Q_TABLE_SIZE] = 
{
	20, 30, 38, 40, 44, 46, 50, 50, 
	51, 57,	59, 61, 62, 64, 66, 67, 
	67, 62,	63, 64, 64, 62, 62, 62, 
	62,	62, 62, 62, 54, 54, 52, 52,
	50, 50, 48, 48, 46, 46, 38, 38,
	38, 38, 36, 36, 34, 32, 32, 32,
	32, 32, 30, 22, 22, 22, 20, 20, 
	18, 16, 14, 10,  6,  6,  4,  4,
};

/*	Inverse fast DCT index											*/
/*	This contains the offsets needed to convert zigzag order into	*/
/*	x, y order for decoding. It is generated from the input zigzag	*/
/*	indexat run time.												*/

/*	For maximum speed during both quantisation and dequantisation	*/
/*	we maintain separate quantisation and zigzag tables for each	*/
/*	operation.														*/

/*	pbi->quant_index:	the zigzag index used during quantisation			*/
/*	dequant_index:	zigzag index used during dequantisation					*/
/*					the pbi->quant_index is the inverse of dequant_index	*/
/*					and is calculated during initialisation					*/

static UINT32 dequant_index[64] = 
{	0,  1,  8,  16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

static UINT32 transIndexC[64] = 
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

static UINT32 quant_indexC[64] = 
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
 * 
 *  ROUTINE       :     InitQTables
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Initialises Q tables based upon version number
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void VP5_InitQTables( QUANTIZER *pbi, UINT8 Vp3VersionNo )
{  
	// Make version specific assignments.
	memcpy ( pbi->QThreshTable, VP5_QThreshTable, sizeof( pbi->QThreshTable ) );
}


/****************************************************************************
 * 
 *  ROUTINE       :     BuildQuantIndex_Generic
 *
 *  INPUTS        :     
 *                      
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Builds the quant_index table.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void VP5_BuildQuantIndex_Generic(QUANTIZER *pbi)
{
    INT32 i,j;

    pbi->transIndex = transIndexC;

    // invert the dequant index into the quant index
	for ( i = 0; i < BLOCK_SIZE; i++ )
	{	
        j = dequant_index[i];
		pbi->quant_index[j] = i;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     UpdateQ
 *
 *  INPUTS        :     UINT32  NewQ
 *                              (A New Q value (50 - 1000))
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Updates the quantisation tables for a new Q
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void VP5_UpdateQ( QUANTIZER *pbi, UINT8 Vp3VersionNo )
{  
	if ( pbi->QThreshTable[pbi->FrameQIndex] == pbi->LastQuantizerValue )
		return;

	// Update the record of last Q and last Q index.
    pbi->LastQuantizerValue = pbi->ThisFrameQuantizerValue;

	// invert the dequant index into the quant index
    // the dxer has a different order than the cxer.
    VP5_BuildQuantIndex(pbi);

	// Re-initialise the q tables for forward and reverse transforms.    
	VP5_init_dequantizer ( pbi, Vp3VersionNo );
}

/********************* COMPRESSOR SPECIFIC **********************************/

/****************************************************************************
 * 
 *  ROUTINE       :     UpdateQC (compressor's update q)
 *
 *  INPUTS        :     UINT32  NewQ
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Updates the quantisation tables for a new Q
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void VP5_UpdateQC( QUANTIZER *pbi, UINT8 Vp3VersionNo )
{  
	if ( pbi->QThreshTable[pbi->FrameQIndex] == pbi->LastQuantizerValue )
		return;

    // Update the record of last Q.
    pbi->LastQuantizerValue = pbi->ThisFrameQuantizerValue;

	// invert the dequant index into the quant index
    // the dxer has a different order than the cxer.
    VP5_BuildQuantIndex_Generic(pbi);

    // Re-initialise the q tables for forward and reverse transforms.    
    VP5_init_quantizer ( pbi, Vp3VersionNo );
	VP5_init_dequantizer ( pbi, Vp3VersionNo );
}

/****************************************************************************
* 
*   Routine:	init_quantizer
*
*   Purpose:    Used to initialize the encoding/decoding data structures
*				and to select DCT algorithm	
*
*   Parameters :
*       Input :
*           UINT32          scale_factor
*                           Defines the factor by which to scale QUANT_ARRAY to
*                           produce quantization_array
*
*           UINT8           QIndex          :: 
*                           Index into Q table for current quantiser value.
*   Return value :
*       None.
*
****************************************************************************
*/
#define SHIFT16 (1<<16)
void VP5_init_quantizer ( QUANTIZER *pbi, UINT8 Vp3VersionNo )
{
    int i;                   // Loop counters

    double temp_fp_quant_coeffs;


    // Notes on setup of quantisers.
    // The "* 4" is a normalisation factor for the forward DCT transform.
	
	// ******************* Y *********************

    // Calculate DC quant values (Include a *4 for FDCT normalization)
	temp_fp_quant_coeffs =  ( VP5_DcQuant[pbi->FrameQIndex] * 4 ); 

	// 1/X (Y)
	temp_fp_quant_coeffs = 1.0 / temp_fp_quant_coeffs;
	pbi->QuantCoeffs[0][0] = (INT32) (0.5 + SHIFT16 * temp_fp_quant_coeffs);

	// DC rounding (Y)
	pbi->QuantRound[0][0] = VP5_DcRTable[pbi->FrameQIndex];

	// Set DC zero Bin (Y)
	pbi->ZeroBinSize[0][0] = VP5_DcZBinTable[pbi->FrameQIndex];
   

	// AC for Y
	for ( i = 1; i < 64; i++ )
	{
		// Normalize the quantizer (* 4 for fdct normalisation)
		temp_fp_quant_coeffs =  (double)(VP5_QThreshTable[pbi->FrameQIndex] * 4);

		// Convert to 1/x
		temp_fp_quant_coeffs = 1.0 / temp_fp_quant_coeffs;
		pbi->QuantCoeffs[0][i] = (INT32) (0.5 + SHIFT16 * temp_fp_quant_coeffs);

		// AC rounding
		pbi->QuantRound[0][i] = VP5_RTable[pbi->FrameQIndex];

		// Zero Bins
		pbi->ZeroBinSize[0][i] = VP5_ZBinTable[pbi->FrameQIndex];
	}


	// ******************* UV *********************
    // Calculate DC quant values (Include a *4 for FDCT normalization)
	temp_fp_quant_coeffs =  ( VP5_UvDcQuant[pbi->FrameQIndex] * 4 ); 

	// 1/X (UV)
	temp_fp_quant_coeffs = 1.0 / temp_fp_quant_coeffs;
	pbi->QuantCoeffs[1][0] = (INT32) (0.5 + SHIFT16 * temp_fp_quant_coeffs);

	// DC rounding (UV)
	pbi->QuantRound[1][0] = VP5_UvDcRTable[pbi->FrameQIndex];

	// Set DC zero Bin (UV)
	pbi->ZeroBinSize[1][0] = VP5_UvDcZBinTable[pbi->FrameQIndex];
   

	// AC for UV
	for ( i = 1; i < 64; i++ )
	{
		// Normalize the quantizer (* 4 for fdct normalisation)
		temp_fp_quant_coeffs =  (double)(VP5_UvQThreshTable[pbi->FrameQIndex] * 4);

		// 1/x
		temp_fp_quant_coeffs = 1.0 / temp_fp_quant_coeffs;
		pbi->QuantCoeffs[1][i] = (INT32) (0.5 + SHIFT16 * temp_fp_quant_coeffs);

		// AC rounding
		pbi->QuantRound[1][i] = VP5_UvRTable[pbi->FrameQIndex];

		// Zero Bins
		pbi->ZeroBinSize[1][i] = VP5_UvZBinTable[pbi->FrameQIndex];
	}
	for(i=0;i<8;i++)
	{
		pbi->round[i] = pbi->QuantRound[0][1];
		pbi->mult[i] = pbi->QuantCoeffs[0][1];
		pbi->zbin[i] = pbi->ZeroBinSize[0][1]-1;
	}

}


/***************************************************************************
* 
*   Routine:    quantize
*
*   Purpose:    Quantizes a block of pixels by dividing 
*               each element by the corresponding entry in the quantization
*               array. Output is in a list of values in the zig-zag order.
*
*   Parameters :
*       Input :
*           DCT_block        -- The block to by quantized
*       Output :
*           quantized_list   -- The quantized values in zig-zag order
*
*   Return value :
*       None.
*
*   Persistent data referenced :
*       quantization_array   Module static array read
*       zig_zag_index        Module static array read
* 
****************************************************************************
*/
#define HIGHBITDUPPED(X) (((signed short) X)  >> 15)
void VP5_quantize_c( QUANTIZER *pbi, INT16 * DCT_block, Q_LIST_ENTRY * quantized_list, UINT8 bp )
{
    UINT32  i, j;
    
    INT32 * QuantRoundPtr = pbi->QuantRound[QTableSelect[bp]];
    INT32 * QuantCoeffsPtr = pbi->QuantCoeffs[QTableSelect[bp]];
    INT32 * ZBinPtr = pbi->ZeroBinSize[QTableSelect[bp]];

    INT16 * DCT_blockPtr = DCT_block;
	INT32 temp;
	INT32 NonZeroACs = 0;
	INT16 *round = &pbi->round[0];
	INT16 *mult = &pbi->mult[0];
	INT16 *zbin = &pbi->zbin[0];

    // Set the quantized_list to default to 0
    memset( quantized_list, 0, 64 * sizeof(Q_LIST_ENTRY) );

	// dc quantization (disabled the zerobinning!!)
	temp = 0;
	if ( DCT_blockPtr[0] >= QuantRoundPtr[0] )
		temp = QuantCoeffsPtr[0] * ( DCT_blockPtr[0] + QuantRoundPtr[0] ) ;
	else if ( DCT_blockPtr[0] <= -QuantRoundPtr[0] )
		temp = QuantCoeffsPtr[0] * ( DCT_blockPtr[0] - QuantRoundPtr[0] ) + MIN16;
	quantized_list[0] = (Q_LIST_ENTRY) (temp>>16);

    // Note that we add in a value to effect rounding.
	// AC Quantization
    for( i = 1; i < 64; i++)
    {
		// Zig Zag order
		j = dequant_index[i];

        if ( DCT_blockPtr[j] >= ZBinPtr[j] )
        {
			temp = QuantCoeffsPtr[j] * ( DCT_blockPtr[j] + QuantRoundPtr[j] ) ;
			quantized_list[i] = (Q_LIST_ENTRY) (temp>>16);
			//NonZeroACs += quantized_list[i];;
        }
        else if ( DCT_blockPtr[j] <= -ZBinPtr[j] )
        {
			temp = QuantCoeffsPtr[j] * ( DCT_blockPtr[j] - QuantRoundPtr[j] ) + MIN16;
			quantized_list[i] = (Q_LIST_ENTRY) (temp>>16);
			//NonZeroACs -= quantized_list[i];
        }
    }


	// Now the DC quantization
/*
	if ( NonZeroACs > 0 )
	{
        if ( DCT_blockPtr[0] >= QuantRoundPtr[0] )
        {
			temp = QuantCoeffsPtr[0] * ( DCT_blockPtr[0] + QuantRoundPtr[0] ) ;
			quantized_list[0] = (Q_LIST_ENTRY) (temp>>16);
        }
        else if ( DCT_blockPtr[0] <= -QuantRoundPtr[0] )
        {
			temp = QuantCoeffsPtr[0] * ( DCT_blockPtr[0] - QuantRoundPtr[0] ) + MIN16;
			quantized_list[0] = (Q_LIST_ENTRY) (temp>>16);
        }
	}
	// Use larger Zero Bin only if there are no ACs as this will help us get an EOB
	else
	{
        if ( DCT_blockPtr[0] >= ZBinPtr[0] )
        {
			temp = QuantCoeffsPtr[0] * ( DCT_blockPtr[0] + QuantRoundPtr[0] ) ;
			quantized_list[0] = (Q_LIST_ENTRY) (temp>>16);
        }
        else if ( DCT_blockPtr[0] <= -ZBinPtr[0] )
        {
			temp = QuantCoeffsPtr[0] * ( DCT_blockPtr[0] - QuantRoundPtr[0] ) + MIN16;
			quantized_list[0] = (Q_LIST_ENTRY) (temp>>16);
        }
	}
*/
}
/**************************** END COMPRESSOR SPECIFIC **********************************/
/***************************************************************************************
*  Dequantiser code for decode loop
/***************************************************************************************/

/****************************************************************************
* 
*   Routine:	init_pbi->dequantizer
*
*   Purpose:    Used to initialize the encoding/decoding data structures
*				and to select DCT algorithm	
*
*   Parameters :
*       Input :
*           UINT32          scale_factor
*                           Defines the factor by which to scale QUANT_ARRAY to
*                           produce quantization_array
*
*           UINT8           QIndex          :: 
*                           Index into Q table for current quantiser value.
*   Return value :
*       None.
*
****************************************************************************
*/

void VP5_init_dequantizer ( QUANTIZER *pbi, UINT8 Vp3VersionNo )
{
    int		i, j;						 


	// *************** Y ******************/
	// Set up the Ac dequant values and then place in the zig-zag/transposed order as appropriate.
	for ( i = 1; i < 64; i++ )
	{	
        j = pbi->quant_index[i];

		pbi->dequant_coeffs[0][j] = VP5_QThreshTable[pbi->FrameQIndex] << IDCT_SCALE_FACTOR;
    }
	
    // DC
    pbi->dequant_coeffs[0][0] = VP5_DcQuant[pbi->FrameQIndex] << IDCT_SCALE_FACTOR;

	// *************** UV ******************/
	// Set up the Ac dequant values and then place in the zig-zag/transposed order as appropriate.
	for ( i = 1; i < 64; i++ )
	{	
        j = pbi->quant_index[i];

		pbi->dequant_coeffs[1][j] = VP5_UvQThreshTable[pbi->FrameQIndex] << IDCT_SCALE_FACTOR;
    }
	
    // DC
    pbi->dequant_coeffs[1][0] = VP5_UvDcQuant[pbi->FrameQIndex] << IDCT_SCALE_FACTOR;

}

/****************************************************************************/
/*																			*/
/*		Select Quantisation Parameters										*/
/*																			*/
/*		void select_Y_dequantiser ( void )									*/
/*			sets dequantiser to use for intra Y         					*/
/*																			*/
/*		void select_Inter_dequantiser ( void )								*/
/*			sets dequantiser to use for inter Y         					*/
/*																			*/
/*		void select_UV_dequantiser ( void )									*/
/*			sets dequantiser to use UV compression constants				*/
/*																			*/
/****************************************************************************/



/****************************************************************************
 * 
 *  ROUTINE       :     DeleteQuantizerBuffers
 *
 *
 *  INPUTS        :     Instance of PB to be cleared
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Initializes the Playback instance passed in
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
static void DeleteQuantizerBuffers(QUANTIZER *pbi)
{

	if(pbi->dequant_coeffsAlloc[0])
		duck_free(pbi->dequant_coeffsAlloc[0]);
	pbi->dequant_coeffsAlloc[0]		= 0;
	pbi->dequant_coeffs[0]				= 0;

	if(pbi->dequant_coeffsAlloc[1])
		duck_free(pbi->dequant_coeffsAlloc[1]);
	pbi->dequant_coeffsAlloc[1]		= 0;
	pbi->dequant_coeffs[1]				= 0;

}

/****************************************************************************
 * 
 *  ROUTINE       :     AllocateQuantizerBuffers
 *
 *
 *  INPUTS        :     Instance of PB to be initialized
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Initializes the Playback instance passed in
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
#define ROUNDUP32(X) ( ( ( (unsigned long) X ) + 31 )&( 0xFFFFFFE0 ) )
static INT32 AllocateQuantizerBuffers(QUANTIZER *pbi)
{
	DeleteQuantizerBuffers(pbi);

	pbi->dequant_coeffsAlloc[0]				= (INT16 *)duck_malloc(32+64*sizeof(INT16), DMEM_GENERAL);
    if(!pbi->dequant_coeffsAlloc[0])		{ DeleteQuantizerBuffers(pbi); return FALSE;};
	pbi->dequant_coeffs[0]					= (INT16 *)ROUNDUP32(pbi->dequant_coeffsAlloc[0]);

	pbi->dequant_coeffsAlloc[1]				= (INT16 *)duck_malloc(32+64*sizeof(INT16), DMEM_GENERAL);
    if(!pbi->dequant_coeffsAlloc[1])		{ DeleteQuantizerBuffers(pbi); return FALSE;};
	pbi->dequant_coeffs[1]					= (INT16 *)ROUNDUP32(pbi->dequant_coeffsAlloc[1]);

	return TRUE;
}


/****************************************************************************
 * 
 *  ROUTINE       :     DeleteQuantizer
 *
 *
 *  INPUTS        :     Instance of POSTPROC to be deleted
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     frees the Playback instance passed in
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void VP5_DeleteQuantizer(QUANTIZER **pbi)
{
	// clear any existing info
    if(*pbi)
    {
        // Delete any other dynamically allocaed temporary buffers

		DeleteQuantizerBuffers(*pbi);
		duck_free(*pbi);
		*pbi=0;
    }

}

/****************************************************************************
 * 
 *  ROUTINE       :     CreateQuantizer
 *
 *
 *  INPUTS        :     Instance of PB to be initialized
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Initializes the Playback instance passed in
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
QUANTIZER * VP5_CreateQuantizer(void)
{
	QUANTIZER *pbi=0;
	int postproc_size = sizeof(QUANTIZER);
	pbi=(QUANTIZER *) duck_malloc(postproc_size, DMEM_GENERAL);
    if(!pbi)
    {
        return 0;
    }

	// initialize whole structure to 0
	memset((unsigned char *) pbi, 0, sizeof(QUANTIZER));
	
	if(!AllocateQuantizerBuffers(pbi))
		VP5_DeleteQuantizer(&pbi);

	return pbi;
}
