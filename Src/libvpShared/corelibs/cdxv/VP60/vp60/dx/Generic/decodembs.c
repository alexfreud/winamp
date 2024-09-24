/****************************************************************************
*
*   Module Title :     Decodembs.c
*
*   Description  :     Compressor functions for block order transmittal
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
*  Typedefs
****************************************************************************/     

// For details of tokens and extra bit breakdown see token definitions in huffman.h
typedef struct 
{    
    UINT16  MinVal;
    INT16   Length;
    UINT8   Probs[11];
} TOKENEXTRABITS;

/****************************************************************************
*  Module constants
****************************************************************************/     
static const UINT32 VP6_HuffTokenMinVal[MAX_ENTROPY_TOKENS] = { 0,1, 2, 3, 4, 5, 7, 11, 19, 35, 67, 0};

static const TOKENEXTRABITS VP6_TokenExtraBits2[MAX_ENTROPY_TOKENS] =
{
    {  0,-1, { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //ZERO_TOKEN
    {  1, 0, { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //ONE_TOKEN
    {  2, 0, { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //TWO_TOKEN
    {  3, 0, { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //THREE_TOKEN
    {  4, 0, { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //FOUR_TOKEN
    {  5, 0, { 159,0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //DCT_VAL_CATEGORY1
    {  7, 1, { 145,165,0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //DCT_VAL_CATEGORY2
    { 11, 2, { 140,148,173,0,  0,  0,  0,  0,  0,  0,  0   } },   //DCT_VAL_CATEGORY3
    { 19, 3, { 135,140,155,176,0,  0,  0,  0,  0,  0,  0   } },   //DCT_VAL_CATEGORY4
    { 35, 4, { 130,134,141,157,180,0,  0,  0,  0,  0,  0   } },   //DCT_VAL_CATEGORY5
    { 67,10, { 129,130,133,140,153,177,196,230,243,254,254 } },   //DCT_VAL_CATEGORY6
    {  0,-1, { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   // EOB TOKEN
};

// Definition of AC coefficient banding
const INT32 VP6_CoeffToBand[65] = 
{  
   -1,0,1,1,1,2,2,2,
	2,2,2,3,3,3,3,3,
	3,3,3,3,3,3,4,4,
	4,4,4,4,4,4,4,4,
	4,4,4,4,4,5,5,5,
	5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,7
};

static const INT32 VP6_CoeffToHuffBand[65] = 
{  
   -1,0,1,1,1,2,2,2,
	2,2,2,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3
};

// Default scan bands for non-interlaced frames
const UINT8 DefaultNonInterlacedScanBands[BLOCK_SIZE] = 
{
   0, 0, 1, 1, 1, 2, 2, 2, 
   2, 2, 2, 3, 3, 4, 4, 4, 
   5, 5, 5, 5, 6, 6, 7, 7, 
   7, 7, 7, 8, 8, 9, 9, 9, 
   9, 9, 9,10,10,11,11,11,
  11,11,11,12,12,12,12,12,
  12,13,13,13,13,13,14,14,
  14,14,15,15,15,15,15,15
};

// Default scan badns for interlaced frames
const UINT8 DefaultInterlacedScanBands[BLOCK_SIZE] = 
{
   0, 1, 0, 1, 1, 2, 5, 3, 
   2, 2, 2, 2, 4, 7, 8,10, 
   9, 7, 5, 4, 2, 3, 5, 6, 
   8, 9,11,12,13,12,11,10, 
   9, 7, 5, 4, 6, 7, 9,11, 
  12,12,13,13,14,12,11, 9, 
   7, 9,11,12,14,14,14,15, 
  13,11,13,15,15,15,15,15, 
};

// AWG Should export this in decodembs.h rather than pbdll.h
const int VP6_Mode2Frame[] =
{
	1,	// CODE_INTER_NO_MV		0 => Encoded diff from same MB last frame 
	0,	// CODE_INTRA			1 => DCT Encoded Block
	1,	// CODE_INTER_PLUS_MV	2 => Encoded diff from included MV MB last frame
	1,	// CODE_INTER_LAST_MV	3 => Encoded diff from MRU MV MB last frame
	1,	// CODE_INTER_PRIOR_MV	4 => Encoded diff from included 4 separate MV blocks
	2,	// CODE_USING_GOLDEN	5 => Encoded diff from same MB golden frame
	2,	// CODE_GOLDEN_MV		6 => Encoded diff from included MV MB golden frame
	1,  // CODE_INTER_FOUR_MV	7 => Encoded diff from included 4 separate MV blocks
	2,	// CODE_GOLD_NEAREST_MV 8 => Encoded diff from MRU MV MB last frame
	2,	// CODE_GOLD_NEAR_MV	9 => Encoded diff from included 4 separate MV blocks
};

// For Bitread functions
static const UINT32 loMaskTbl_VP60[] = 
{   
    0x00000000,
    0x00000001, 0x00000003, 0x00000007, 0x0000000F,
    0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
	0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
	0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
	0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
	0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
	0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
	0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF
};

/****************************************************************************
 * 
 *  ROUTINE       :     NextWord (MACRO)
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Reads 32 bits from the input buffer for processing and
 *                      reverts data to little endian.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
#	define BitsAreBigEndian 1
#	if BitsAreBigEndian
#		define NextWord \
{ br->remainder = (br->position[0] << 24) + (br->position[1] << 16) + (br->position[2] << 8) + br->position[3];  br->position += 4;}
#	else
#		define NextWord \
{ br->remainder = (br->position[3] << 24) + (br->position[2] << 16) + (br->position[1] << 8) + br->position[0];  br->position += 4;}
#	endif

/****************************************************************************
 * 
 *  ROUTINE       :     bitread
 *
 *  INPUTS        :     BITREADER *br : Wrapper for the encoded data buffer.
 *                      int bits      : Number of bits to read.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Value of the number of bits requested (as UINT32)
 *
 *  FUNCTION      :     Extracts requested number of bits from the encoded data buffer.
 *
 *  SPECIAL NOTES :     Uses the NextWord macro. 
 *
 ****************************************************************************/
FORCEINLINE
UINT32 bitread ( BITREADER *br, int bits )
{
	UINT32 z = 0;

	br->remainder &= loMaskTbl_VP60[br->bitsinremainder];
	
	if( (bits -= br->bitsinremainder) > 0) 
	{
		z |= br->remainder << bits;
		NextWord
			bits -= 32;
	}
	return z | br->remainder >> (br->bitsinremainder = -bits);
}

/****************************************************************************
 * 
 *  ROUTINE       :     bitreadonly
 *
 *  INPUTS        :     BITREADER *br : Wrapper for the encoded data buffer.
 *                      int bits      : Number of bits to read.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Value of the number of bits requested (as UINT32)
 *
 *  FUNCTION      :     Extracts requested number of bits from the encoded data buffer.
 *
 *  SPECIAL NOTES :     This reader variant will only read a further byte from the
 *                      encoded data buffer. 
 *
 ****************************************************************************/
FORCEINLINE
UINT32 bitreadonly ( BITREADER *br, UINT32 bits )
{
    UINT32 x = br->bitsinremainder;
    UINT32 z = (1<<x)-1;

    z &= br->remainder;
    if ( x >= bits )
    {        
        return z>>(x-bits);
    }    
    z <<= 8;
    z  |= br->position[0];
	return (z>>(8+x-bits));
}

/****************************************************************************
 * 
 *  ROUTINE       :     bitShift
 *
 *  INPUTS        :     BITREADER *br : Wrapper for the encoded data buffer.
 *                      int bits      : Number of bits to discard (shift off).
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Discards requested number of bits from the encoded data buffer.
 *
 *  SPECIAL NOTES :     Uses the NextWord macro.
 *
 ****************************************************************************/
FORCEINLINE
void bitShift ( BITREADER *br, int bits )
{			
	br->bitsinremainder -= bits;
    if ( br->bitsinremainder < 0 ) 
	{
		NextWord
			br->bitsinremainder += 32;
	}	
}

/****************************************************************************
 * 
 *  ROUTINE       :     bitread1
 *
 *  INPUTS        :     BITREADER *br : Wrapper for the encoded data buffer.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     A single bit (as UINT32)
 *
 *  FUNCTION      :     Extracts a single bit  from the encoded data buffer.
 *
 *  SPECIAL NOTES :     Uses the NextWord macro. 
 *
 ****************************************************************************/
FORCEINLINE
UINT32 bitread1 ( BITREADER *br ) 
{
	if( br->bitsinremainder)
		return (br->remainder >> --br->bitsinremainder) & 1;
	NextWord
		return br->remainder  >> (br->bitsinremainder = 31);
}

#undef NextWord

/****************************************************************************
 * 
 *  ROUTINE       :     nDecodeBool
 *
 *  INPUTS        :     BITREADER *br   : Wrapper for the encoded data buffer.
 *                      int probability : Probability that next symbol in Boolean 
 *                                        Coded buffer is a 0.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     Value of the next encoded token 0 or 1 (as int)
 *
 *  FUNCTION      :     Extracts next token (0 or 1) from the Boolean encoded data buffer.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
FORCEINLINE
int nDecodeBool ( BOOL_CODER *br, int probability ) 
{
	unsigned int split;
    int count = br->count;
    unsigned int range = br->range;
    unsigned int value = br->value;

	// perform the actual decoding
	split = 1 +  (((range-1) * probability) >> 8);	

	if ( value >= split<<24 )
	{
		value -= (split<<24);
		range = range - split;

        while(range < 0x80 )
        {
	        range += range;
	        value += value;
	        
	        if ( !--count ) 
	        {
		        count  = 8;
		        value |= br->buffer[br->pos];
		        br->pos++;
	        }
        }

        br->count = count;
        br->value = value;
        br->range = range;

        return 1;

	}
	range = split;

    while(range < 0x80 )
	{
		range += range;
		value += value;
		
		if ( !--count ) 
		{
			count  = 8;
			value |= br->buffer[br->pos];
			br->pos++;
		}
	}
    br->count = count;
    br->value = value;
    br->range = range;
	return 0;
} 

/****************************************************************************
 * 
 *
 ****************************************************************************/
#define APPLYSIGN(dest, valueToSign) \
{ \
	unsigned int split; \
    split    = (range + 1) >> 1; \
	if ( value >= split<<24 ) \
	{ \
		value = value - (split<<24); \
        value += value; \
		range = range - split; \
        range += range; \
        if( !--count ) \
        { \
            count  = 8; \
            value |= *brBuffer; \
            brBuffer++; \
        } \
        dest = -valueToSign; \
    } \
    else \
    { \
        range = split; \
        range += range; \
        value += value; \
        if( !--count ) \
        { \
            count  = 8; \
            value |= *brBuffer; \
            brBuffer++; \
        } \
        dest = valueToSign; \
    } \
}

/****************************************************************************
 * 
 *
 ****************************************************************************/
//    register int count = _mm_cvtsi64_si32(m64_brCount); 
#define NDECODEBOOL_AND_BRANCH_IF_ONE(probability, branch) \
{ \
	unsigned int split; \
	split = 1 +  (((range-1) * probability) >> 8); \
	if ( value >= split<<24 ) \
	{ \
		value -= (split<<24); \
		range = range - split; \
        while(range < 0x80 ) \
        { \
	        range += range; \
	        value += value; \
	        if ( !--count ) \
	        { \
		        count  = 8; \
		        value |= *brBuffer; \
		        brBuffer++; \
	        } \
        } \
        goto branch; \
	} \
	range = split; \
    while(range < 0x80 ) \
	{ \
		range += range; \
		value += value; \
		if ( !--count ) \
		{ \
			count  = 8; \
			value |= *brBuffer; \
			brBuffer++; \
		} \
	} \
}
/****************************************************************************
 * 
 *
 ****************************************************************************/
#define NDECODEBOOL_AND_BRANCH_IF_ZERO(probability, branch) \
{ \
	unsigned int split; \
	split = 1 +  (((range-1) * probability) >> 8); \
	if ( value < split<<24 ) \
	{ \
    	range = split; \
        while(range < 0x80 ) \
        { \
	        range += range; \
	        value += value; \
	        if ( !--count ) \
	        { \
		        count  = 8; \
		        value |= *brBuffer; \
		        brBuffer++; \
	        } \
        } \
        goto branch; \
	} \
	value -= (split<<24); \
	range = range - split; \
    while(range < 0x80 ) \
	{ \
		range += range; \
		value += value; \
		if ( !--count ) \
		{ \
			count  = 8; \
			value |= *brBuffer; \
			brBuffer++; \
		} \
	} \
}


/****************************************************************************
* 
*  ROUTINE       :     BuildScanOrder
*
*  INPUTS        :     PB_INSTANCE *pbi : Pointer to instance of a decoder.
*                      UINT8 *ScanBands : Pointer to array containing band for 
*                                         each DCT coeff position. 
*
*  OUTPUTS       :     None
*
*  RETURNS       :     void
*
*  FUNCTION      :     Builds a custom dct scan order from a set of band data.
*
*  SPECIAL NOTES :     None. 
*
****************************************************************************/
void BuildScanOrder( PB_INSTANCE *pbi, UINT8 *ScanBands )
{
	UINT32 i, j;
	UINT32 ScanOrderIndex = 1;
	UINT32 MaxOffset;
	UINT32     *TransIndex = pbi->quantizer->transIndex; 

	// DC is fixed
	pbi->ModifiedScanOrder[0] = 0;

	// Create a scan order where within each band the coefs are in ascending order
	// (in terms of their original zig-zag positions).
	for ( i = 0; i < SCAN_ORDER_BANDS; i++ )
	{
		for ( j = 1; j < BLOCK_SIZE; j++ )
		{
			if ( ScanBands[j] == i )
			{
				pbi->ModifiedScanOrder[ScanOrderIndex] = j;
				ScanOrderIndex++;
			}
		}
	}

	// For each of the positions in the modified scan order work out the 
	// worst case EOB offset in zig zag order. This is used in selecting
    // the appropriate idct variant
	for ( i = 0; i < BLOCK_SIZE; i++ )
	{
		MaxOffset = 0;
		for ( j = 0; j <= i; j++ )
		{
			if ( pbi->ModifiedScanOrder[j] > MaxOffset )
				MaxOffset = pbi->ModifiedScanOrder[j];
		}

		pbi->EobOffsetTable[i] = MaxOffset;

		if(pbi->Vp3VersionNo > 6)
            pbi->EobOffsetTable[i] = MaxOffset+1;

    }
}

/****************************************************************************
* 
*  ROUTINE       :     BoolTreeToHuffCodes
*
*  INPUTS        :     UINT8  *BoolTreeProbs : Dct coeff tree node probabilities
*
*  OUTPUTS       :     UINT32 *HuffProbs     : Dct coeff probability distribution
*
*  RETURNS       :     void
*
*  FUNCTION      :     Convert set of internal tree node probabilities to set of
*                      token probabilities (run lengths 1--8, and >8 are the tokens).
*
*  SPECIAL NOTES :     None. 
*
****************************************************************************/
void BoolTreeToHuffCodes ( UINT8 *BoolTreeProbs, UINT32 *HuffProbs )
{
    UINT32 Prob;
    UINT32 Prob1;

    HuffProbs[DCT_EOB_TOKEN]       = ((UINT32)BoolTreeProbs[0] * (UINT32)BoolTreeProbs[1]) >> 8;
    HuffProbs[ZERO_TOKEN]          = ((UINT32)BoolTreeProbs[0] * (255 - (UINT32)BoolTreeProbs[1])) >> 8;

    Prob = (255 - (UINT32)BoolTreeProbs[0]);
    HuffProbs[ONE_TOKEN]           = (Prob * (UINT32)BoolTreeProbs[2]) >> 8;

    Prob = (Prob*(255 - (UINT32)BoolTreeProbs[2])) >> 8;
    Prob1 = (Prob * (UINT32)BoolTreeProbs[3]) >> 8;
    HuffProbs[TWO_TOKEN]           = (Prob1 * (UINT32)BoolTreeProbs[4]) >> 8; 
    Prob1 = (Prob1 * (255 - (UINT32)BoolTreeProbs[4])) >> 8;
    HuffProbs[THREE_TOKEN]         = (Prob1 * (UINT32)BoolTreeProbs[5]) >> 8;
    HuffProbs[FOUR_TOKEN]          = (Prob1 * (255 - (UINT32)BoolTreeProbs[5])) >> 8;

    Prob = (Prob * (255 - (UINT32)BoolTreeProbs[3])) >> 8;
    Prob1 = (Prob * (UINT32)BoolTreeProbs[6]) >> 8;
    HuffProbs[DCT_VAL_CATEGORY1]   = (Prob1 * (UINT32)BoolTreeProbs[7]) >> 8;
    HuffProbs[DCT_VAL_CATEGORY2]   = (Prob1 * (255 - (UINT32)BoolTreeProbs[7])) >> 8;
    
    Prob = (Prob * (255 - (UINT32)BoolTreeProbs[6])) >> 8;
    Prob1 = (Prob * (UINT32)BoolTreeProbs[8]) >> 8; 
    HuffProbs[DCT_VAL_CATEGORY3]   = (Prob1 * (UINT32)BoolTreeProbs[9]) >> 8;
    HuffProbs[DCT_VAL_CATEGORY4]   = (Prob1 * (255 - (UINT32)BoolTreeProbs[9])) >> 8;

    Prob = (Prob * (255 - (UINT32)BoolTreeProbs[8])) >> 8;
    HuffProbs[DCT_VAL_CATEGORY5]   = (Prob * (UINT32)BoolTreeProbs[10]) >> 8;
    HuffProbs[DCT_VAL_CATEGORY6]   = (Prob * (255 - (UINT32)BoolTreeProbs[10])) >> 8;
}

/****************************************************************************
* 
*  ROUTINE       :     ZerosBoolTreeToHuffCodes
*
*  INPUTS        :     UINT8  *BoolTreeProbs : Zrl tree node probabilities
*
*  OUTPUTS       :     UINT32 *HuffProbs     : Zrl run-length distribution
*
*  RETURNS       :     void
*
*  FUNCTION      :     Convert zero run-length tree node probs to set 
*                      of run-length probs (run lengths 1--8, and >8
*                      are the tokens).
*
*  SPECIAL NOTES :     None. 
*
****************************************************************************/
void ZerosBoolTreeToHuffCodes ( UINT8 *BoolTreeProbs, UINT32 *HuffProbs )
{
    UINT32 Prob;

    Prob  = ((UINT32)BoolTreeProbs[0] * (UINT32)BoolTreeProbs[1]) >> 8;
    HuffProbs[0] = (Prob * (UINT32)BoolTreeProbs[2]) >> 8;
    HuffProbs[1] = (Prob * (UINT32)(255 - BoolTreeProbs[2])) >> 8;

    Prob = ((UINT32)BoolTreeProbs[0] * (UINT32)(255 - BoolTreeProbs[1])) >> 8;
    HuffProbs[2] = (Prob * (UINT32)BoolTreeProbs[3]) >> 8;
    HuffProbs[3] = (Prob * (UINT32)(255 - BoolTreeProbs[3])) >> 8;

    Prob = ((UINT32)(255 - BoolTreeProbs[0]) * (UINT32)BoolTreeProbs[4]) >> 8;
    Prob = (Prob * (UINT32)BoolTreeProbs[5]) >> 8;
    HuffProbs[4] = (Prob * (UINT32)BoolTreeProbs[6]) >> 8;
    HuffProbs[5] = (Prob * (UINT32)(255 - BoolTreeProbs[6])) >> 8;

    Prob = ((UINT32)(255 - BoolTreeProbs[0]) * (UINT32)BoolTreeProbs[4]) >> 8;
    Prob = (Prob * (UINT32)(255 - BoolTreeProbs[5])) >> 8;
    HuffProbs[6] = (Prob * (UINT32)BoolTreeProbs[7]) >> 8;
    HuffProbs[7] = (Prob * (UINT32)(255 - BoolTreeProbs[7])) >> 8;

    Prob = ((UINT32)(255 - BoolTreeProbs[0]) * (UINT32)(255 - BoolTreeProbs[4])) >> 8;
    HuffProbs[8] = Prob;
}


/****************************************************************************
* 
*  ROUTINE       :     ConvertBoolTrees
*
*  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
*
*  OUTPUTS       :     None.
*
*  RETURNS       :     void
*
*  FUNCTION      :     Create set of Huffman codes for tokens from a set of
*                      internal binary tree node probabilities.
*
*  SPECIAL NOTES :     None. 
*
****************************************************************************/
void ConvertBoolTrees ( PB_INSTANCE *pbi )
{
    UINT32  i;
	UINT32	Plane;
	UINT32	Band;
	INT32   Prec;

    // Convert bool tree node probabilities into array of token 
    // probabilities. Use these to create a set of Huffman codes

	// DC
    for ( Plane = 0; Plane < 2; Plane++ )
    {
        BoolTreeToHuffCodes ( pbi->DcProbs+DCProbOffset(Plane,0), pbi->DcHuffProbs[Plane] );
        VP6_BuildHuffTree ( pbi->DcHuffTree[Plane], pbi->DcHuffProbs[Plane], MAX_ENTROPY_TOKENS );
        VP6_BuildHuffLookupTable(pbi->DcHuffTree[Plane], pbi->DcHuffLUT[Plane]);        
        VP6_CreateCodeArray ( pbi->DcHuffTree[Plane], 0, pbi->DcHuffCode[Plane], pbi->DcHuffLength[Plane], 0, 0 );
    }
    
    // ZEROS
    for ( i = 0; i < ZRL_BANDS; i++ )
	{
        ZerosBoolTreeToHuffCodes ( pbi->ZeroRunProbs[i], pbi->ZeroHuffProbs[i] );
        VP6_BuildHuffTree ( pbi->ZeroHuffTree[i], pbi->ZeroHuffProbs[i], 9 );
        VP6_BuildHuffLookupTable(pbi->ZeroHuffTree[i], pbi->ZeroHuffLUT[i]);
        VP6_CreateCodeArray ( pbi->ZeroHuffTree[i], 0, pbi->ZeroHuffCode[i], pbi->ZeroHuffLength[i], 0, 0 );
    }

    // AC
    for ( Prec = 0; Prec < PREC_CASES; Prec++ )
	{
		// Baseline probabilities for each AC band.
		for ( Plane = 0; Plane < 2; Plane++ )
		{
			for ( Band = 0; Band < VP6_AC_BANDS; Band++ )
            {
                BoolTreeToHuffCodes ( pbi->AcProbs+ACProbOffset(Plane,Prec,Band,0), pbi->AcHuffProbs[Prec][Plane][Band] );
                VP6_BuildHuffTree ( pbi->AcHuffTree[Prec][Plane][Band], pbi->AcHuffProbs[Prec][Plane][Band], MAX_ENTROPY_TOKENS );
                VP6_BuildHuffLookupTable(pbi->AcHuffTree[Prec][Plane][Band],pbi->AcHuffLUT[Prec][Plane][Band]);
                VP6_CreateCodeArray ( pbi->AcHuffTree[Prec][Plane][Band], 0, pbi->AcHuffCode[Prec][Plane][Band], pbi->AcHuffLength[Prec][Plane][Band], 0, 0 );
            }
        }
    }
}

/****************************************************************************
* 
*  ROUTINE       :     VP6_ConfigureEntropyDecoder
*
*  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
*                      UINT8 FrameType  : Type of frame.
*
*  OUTPUTS       :     None.
*
*  RETURNS       :     void
*
*  FUNCTION      :     Configure entropy subsystem ready for decode
*
*  SPECIAL NOTES :     None. 
*
****************************************************************************/
void VP6_ConfigureEntropyDecoder( PB_INSTANCE *pbi, UINT8 FrameType )
{
	UINT32	i,j;
	UINT32  Plane;
	UINT32  Band;
	INT32   Prec;
	UINT8   PrecNonZero;
	UINT8   LastProb[MAX_ENTROPY_TOKENS-1];
	
	// Clear down Last Probs data structure
	memset( LastProb, 128, MAX_ENTROPY_TOKENS-1 );

	// Read in the Baseline DC probabilities and initialise the DC context for Y and then UV plane
	for ( Plane = 0; Plane < 2; Plane++ )
	{
		// If so then read them in.
		for ( i = 0; i < MAX_ENTROPY_TOKENS-1; i++ )
		{
			if ( nDecodeBool(&pbi->br, VP6_DcUpdateProbs[Plane][i] ) )
			{
				// 0 is not a legal value, clip to 1.
				LastProb[i] = VP6_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				LastProb[i] += ( LastProb[i] == 0 );
				pbi->DcProbs[DCProbOffset(Plane,i)] = LastProb[i];

			}
			else if ( FrameType == BASE_FRAME )
			{
				pbi->DcProbs[DCProbOffset(Plane,i)] = LastProb[i];
			}
		}
	}

	// Set Zero run probabilities to defaults if this is a key frame
	if ( FrameType == BASE_FRAME )
	{
		memcpy( pbi->ZeroRunProbs, ZeroRunProbDefaults, sizeof(pbi->ZeroRunProbs) );
	}

	// If this frame contains updates to the scan order then read them
	if ( nDecodeBool( &pbi->br, 128 ) )
	{
		// Read in the AC scan bands and build the custom scan order
		for ( i = 1; i < BLOCK_SIZE; i++ )
		{
			// Has the band for this coef been updated ?
			if ( nDecodeBool( &pbi->br, ScanBandUpdateProbs[i] ) )
				pbi->ScanBands[i] = VP6_bitread( &pbi->br, SCAN_BAND_UPDATE_BITS );
		}
		// Build the scan order
		BuildScanOrder( pbi, pbi->ScanBands );
	}

	// Update the Zero Run probabilities
	for ( i = 0; i < ZRL_BANDS; i++ )
	{
		for ( j = 0; j < ZERO_RUN_PROB_CASES; j++ )
		{
			if ( nDecodeBool( &pbi->br, ZrlUpdateProbs[i][j] )  )
			{
				// Probabilities sent
				pbi->ZeroRunProbs[i][j] = VP6_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				pbi->ZeroRunProbs[i][j] += ( pbi->ZeroRunProbs[i][j] == 0 );
			}
		}		
	}

	// Read in the Baseline AC band probabilities and initialise the appropriate contexts
	// Prec=0 means last token in current block was 0: Prec=1 means it was 1. Prec=2 means it was > 1
	for ( Prec = 0; Prec < PREC_CASES; Prec++ )
	{
		PrecNonZero = ( Prec > 0 ) ? 1 : 0;
		for ( Plane = 0; Plane < 2; Plane++ )
		{
			for ( Band = 0; Band < VP6_AC_BANDS; Band++ )
			{
				// If so then read them in.
				for ( i = 0; i < MAX_ENTROPY_TOKENS-1; i++ )
				{
					if ( nDecodeBool(&pbi->br, VP6_AcUpdateProbs[Prec][Plane][Band][i] ) )
					{
						// Probabilities transmitted at reduced resolution. 
						// 0 is not a legal value, clip to 1.
						LastProb[i] = VP6_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
						LastProb[i] += ( LastProb[i] == 0 );                        
						pbi->AcProbs[ACProbOffset(Plane,Prec,Band,i)] = LastProb[i];
					}
					else if ( FrameType == BASE_FRAME )
					{
						pbi->AcProbs[ACProbOffset(Plane,Prec,Band,i)] = LastProb[i];
					}
				}
			}
		}
	} 

	// Create all the context specific propabilities based upon the new baseline data
	VP6_ConfigureContexts(pbi);

}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_ResetLeftContext
 *
 *  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Updates the left contexts.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_ResetLeftContext ( PB_INSTANCE *pbi)
{
	memset((void *) &pbi->fc.LeftY[0], 0, sizeof(BLOCK_CONTEXT));
	memset((void *) &pbi->fc.LeftY[1], 0, sizeof(BLOCK_CONTEXT));
	memset((void *) &pbi->fc.LeftU,    0, sizeof(BLOCK_CONTEXT));
	memset((void *) &pbi->fc.LeftV,    0, sizeof(BLOCK_CONTEXT));
		
	pbi->fc.LeftY[0].Mode = (CODING_MODE)-1;
	pbi->fc.LeftY[1].Mode = (CODING_MODE)-1;
	pbi->fc.LeftU.Mode    = (CODING_MODE)-1;
	pbi->fc.LeftV.Mode    = (CODING_MODE)-1;
		
	pbi->fc.LeftY[0].Frame = 4;
	pbi->fc.LeftY[1].Frame = 4;
	pbi->fc.LeftU.Frame    = 4;
	pbi->fc.LeftV.Frame    = 4;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_ResetAboveContext
 *
 *  INPUTS        :     PB_INSTANCE *pbi : Pointer to decoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Updates the above contexts.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_ResetAboveContext ( PB_INSTANCE *pbi )
{
	UINT32 i;

    for ( i = 0 ; i < pbi->HFragments+8;i++)
	{
        pbi->fc.AboveY[i].Mode = -1;
        pbi->fc.AboveY[i].Frame = 4;
		pbi->fc.AboveY[i].Dc =0;
		pbi->fc.AboveY[i].Token=0;
	}
	for ( i = 0 ; i < pbi->HFragments/2 + 8;i++)
	{        
        pbi->fc.AboveU[i].Mode = -1;
        pbi->fc.AboveU[i].Frame = 4;
		pbi->fc.AboveU[i].Token=0;
		pbi->fc.AboveU[i].Dc=0;
        pbi->fc.AboveV[i].Mode = -1;
        pbi->fc.AboveV[i].Frame = 4;  
		pbi->fc.AboveV[i].Token=0;
		pbi->fc.AboveV[i].Dc=0;
	}

	if(pbi->Vp3VersionNo < 6)
	{
        pbi->fc.AboveU[1].Mode = 0;
        pbi->fc.AboveU[1].Frame = 0;
        pbi->fc.AboveV[1].Mode = 0;
        pbi->fc.AboveV[1].Frame = 0;                
	}

	pbi->fc.LastDcY[0] = 0;
	pbi->fc.LastDcU[0] = 128;
	pbi->fc.LastDcV[0] = 128;
	for ( i = 1 ; i < 3 ; i++)
	{
		pbi->fc.LastDcY[i] = 0;
		pbi->fc.LastDcU[i] = 0;
		pbi->fc.LastDcV[i] = 0;
	}
}


/****************************************************************************
 * 
 *  ROUTINE       :     VP6_UpdateContext
 *
 *  INPUTS        :     PB_INSTANCE *pbi  : Pointer to decoder instance.
 *                      BLOCK_CONTEXT *c  : Pointer to 
 *                      BLOCK_POSITION bp : Position of the block in the containing MB.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Updates the context for a particular block within a MB.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_UpdateContext ( PB_INSTANCE *pbi, BLOCK_CONTEXT *c, BLOCK_POSITION bp )
{
	c->Mode  = pbi->mbi.BlockMode[bp];
	c->Dc    = pbi->mbi.blockDxInfo[bp].coeffsPtr[0]; //pbi->mbi.Coeffs[bp][0];
	c->Frame = VP6_Mode2Frame[pbi->mbi.Mode];
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_UpdateContextA
 *
 *  INPUTS        :     PB_INSTANCE *pbi  : Pointer to decoder instance.
 *                      BLOCK_CONTEXT *c  : Pointer to 
 *                      BLOCK_POSITION bp : Position of the block in the containing MB.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Updates the context for a particular block within a MB.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_UpdateContextA ( PB_INSTANCE *pbi, BLOCK_CONTEXT *c, BLOCK_POSITION bp )
{
	c->Mode  = pbi->mbi.BlockMode[bp];
	c->Dc    = pbi->mbi.blockDxInfo[bp].coeffsPtr[0]; //pbi->mbi.Coeffs[bp][0];
	c->Frame = VP6_Mode2Frame[pbi->mbi.Mode];
}

#define HIGHBITDUPPED(X) (((signed short) X)  >> 15)

/****************************************************************************
 * 
 *
 ****************************************************************************/
void VP6_PredictDC
( 
	PB_INSTANCE *pbi,
	BLOCK_POSITION bp
)
{
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
            Avg += Left->Dc;
            Avg += (HIGHBITDUPPED(Avg)&1);
			Avg >>= 1;

        }
	}

	pbi->mbi.blockDxInfo[bp].coeffsPtr[0] += Avg;
	LastDC[Frame] = pbi->mbi.blockDxInfo[bp].coeffsPtr[0];

	return;
}

/****************************************************************************
 * 
 *  ROUTINE       :     VP6_PredictDC_MB
 *
 *  INPUTS        :     PB_INSTANCE *pbi     : Pointer to decoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Predicts coefficients in this macroblock based on the 
 *                      contexts provided.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void 
VP6_PredictDC_MB(PB_INSTANCE *pbi)
{
	UINT8 Frame = VP6_Mode2Frame[pbi->mbi.Mode];
	Q_LIST_ENTRY *  LastDC;
	BLOCK_CONTEXT*  Above;
	BLOCK_CONTEXT *  Left;

    BLOCK_DX_INFO *bdi = pbi->mbi.blockDxInfo;
    BLOCK_DX_INFO *bdiEnd = bdi + 6;

	do
    {
    	INT32 Avg;
	
    	LastDC = bdi->LastDc;
		Above = bdi->Above;
 		Left = bdi->Left;

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
            	Avg += Left->Dc;
                Avg += (HIGHBITDUPPED(Avg)&1);
			    Avg >>= 1;

            }
		}

		bdi->coeffsPtr[0] += Avg;
		LastDC[Frame] = bdi->coeffsPtr[0];
		
        Above->Dc = bdi->coeffsPtr[0];
		Above->Frame = Frame;

		Left->Dc = bdi->coeffsPtr[0];
		Left->Frame = Frame;

	} while(++bdi < bdiEnd);	
}
 

/****************************************************************************
* 
*  ROUTINE       :     VP6_ExtractTokenN
*
*  INPUTS        :     BITREADER *br : Pointer to bitreader to grab the bits from.
*                      HUFF_NODE *hn : Pointer to root of huffman tree to use for decoding.
*                      UINT16* hlt   : Pointer to Huffman table node.
*
*  OUTPUTS       :     None.
*
*  RETURNS       :     The number of bits decoded.
*
*  FUNCTION      :     Unpacks and expands a DCT token.
*
*  SPECIAL NOTES :     PROBLEM !!!!!!!!!!!   right now handles only left 
*                      justified bits in bitreader. The C version keeps every
*                      thing in place so I can't use it!!
*
****************************************************************************/
FORCEINLINE
int VP6_ExtractTokenN ( BITREADER *br, HUFF_NODE *hn, UINT16* hlt )
{
    tokenorptr torp;
    HUFF_TABLE_NODE *htptr = (HUFF_TABLE_NODE *)hlt;
    UINT32 x = bitreadonly(br, HUFF_LUT_LEVELS);
    
    bitShift(br, (htptr[x].length));
    if(htptr[x].flag)
    {        
        return htptr[x].value;
    }
        
    torp.value = htptr[x].value;
    do
    {
        if( bitread1(br) )
        {
            torp = hn[torp.value].rightunion.right;
        }
        else
        {
            torp = hn[torp.value].leftunion.left;
        }
    }
    while ( !(torp.selector));
    
    return torp.value;
    
}

/****************************************************************************
****************************************************************************/
void
ReadHuffTokensPredictA_MB(PB_INSTANCE *pbi)
{
    BITREADER *br = &pbi->br3;
    INT32   SignBit;
	UINT32  Prec;

	UINT32   token;
    UINT32  blockIndex;

    UINT32  Plane = 0;

    INT16 *CoeffData;
    MACROBLOCK_INFO *mbi = &pbi->mbi;

    UINT8 *MergedScanOrderPtr;

    //BLOCK_DX_INFO *bdi = pbi->mbi.blockDxInfo;
    //BLOCK_DX_INFO *bdiEnd = bdi + 6;

    for(blockIndex = 0; blockIndex < 6; blockIndex++)
    {
        MergedScanOrderPtr = pbi->MergedScanOrder;

        CoeffData = pbi->mbi.blockDxInfo[blockIndex].coeffsPtr; //mbi->Coeffs[blockIndex];


        if(blockIndex > 3)
        {
            Plane = 1;
        }

        if ( pbi->CurrentDcRunLen[Plane] > 0 )
        {
            // DC -- run of zeros in progress
            --pbi->CurrentDcRunLen[Plane];
            Prec = 0;        
        }
        else
        {
            // DC -- no current run of zeros
            token = VP6_ExtractTokenN(br, pbi->DcHuffTree[Plane], pbi->DcHuffLUT[Plane]);                                                      

            if(token == DCT_EOB_TOKEN)
                goto Finished;

            if(token == ZERO_TOKEN)
            {   
                // Read zero run-length
                {
                    // Run of zeros at DC is coded as a tree
                    UINT32 val = 1 + bitread(br, 2);

                    if ( val == 3 )
                        val += bitread(br, 2);
                    else if ( val == 4 )
                    {
                        if ( bitread1(br) )
                            val = 11 + bitread(br, 6);
                        else
                            val = 7 + bitread(br, 2); 
                    }
                    pbi->CurrentDcRunLen[Plane] = val - 1;
                }
                Prec = 0;
            }
            else
            {
                register INT32 value;
            
                value = VP6_HuffTokenMinVal[token];
    
                if(token <=FOUR_TOKEN)
                {
                    SignBit = bitread1(br);
                }
                else if(token <=DCT_VAL_CATEGORY5)
                {
                    value   += bitread(br, (token-4));
                    SignBit = bitread1(br);
                }
                else
                {
                    value   += bitread(br, 11);
                    SignBit = bitread1(br);
                
                }
                CoeffData[0] = (Q_LIST_ENTRY)((value ^ -SignBit) + SignBit); 
                Prec = (value>1)?2:1;
            }

        }
        //first AC
    
        MergedScanOrderPtr++;

        if ( pbi->CurrentAc1RunLen[Plane] > 0 )
        {
            // First AC in scan order -- run of EOBs in progress
            --pbi->CurrentAc1RunLen[Plane];
            goto Finished;
        }

        do
	    {
		    
            UINT32 Band = *(MergedScanOrderPtr + 64); //VP6_CoeffToHuffBand[EncodedCoeffs];        
                
            token = VP6_ExtractTokenN(br, pbi->AcHuffTree[Prec][Plane][Band], pbi->AcHuffLUT[Prec][Plane][Band]);              

            if(token == ZERO_TOKEN)
            {
                {
                    //UINT32 ZrlBand;
                    //UINT32 ZrlToken;
                    #define ZrlBand Band
                    #define ZrlToken token

                    // Read zero run-length
                    ZrlBand  = (MergedScanOrderPtr >= (pbi->MergedScanOrder + ZRL_BAND2));
                    
                    ZrlToken = VP6_ExtractTokenN(br, pbi->ZeroHuffTree[ZrlBand], pbi->ZeroHuffLUT[ZrlBand]);
              
                    if ( ZrlToken<8 )
                        MergedScanOrderPtr += ZrlToken;             // Zero run <= 8
                    else
                        MergedScanOrderPtr += 8 + bitread(br, 6);   // Zero run > 8
                }
                Prec =0;
                MergedScanOrderPtr ++;			
                continue;
            }
        
            if(token == DCT_EOB_TOKEN)
            {
                if ( MergedScanOrderPtr == (pbi->MergedScanOrder + 1) )
                {
                    // Read run of EOB at first AC position
                    UINT32 val = 1 + bitread(br, 2);
                
                    if ( val == 3 )
                        val += bitread(br, 2);
                    else if ( val == 4 )
                    {
                        if ( bitread1(br) )
                            val = 11 + bitread(br, 6);
                        else
                            val = 7 + bitread(br, 2); 
                    }
                    pbi->CurrentAc1RunLen[Plane] = val - 1;
                }
                goto Finished;

            }

            {
                register INT32 value;

                value = VP6_HuffTokenMinVal[token];
        
                if(token <=FOUR_TOKEN)
                {
                    SignBit = bitread1(br);
                }
                else if(token <=DCT_VAL_CATEGORY5)
                {
                    value   += bitread(br, (token-4));
                    SignBit = bitread1(br);
                }
                else
                {
                    value   += bitread(br, 11);
                    SignBit = bitread1(br);
            
                }
            
                CoeffData[*(MergedScanOrderPtr)] = (Q_LIST_ENTRY)((value ^ -SignBit) + SignBit);             
                Prec = (value>1)?2:1;        
                MergedScanOrderPtr ++;			
            }

        } while (MergedScanOrderPtr < (pbi->MergedScanOrder + BLOCK_SIZE));

	    MergedScanOrderPtr--;

    Finished:
	    //EobArray[blockIndex] =  pbi->EobOffsetTable[(UINT32)(MergedScanOrderPtr - (pbi->MergedScanOrder))];
	    pbi->mbi.blockDxInfo[blockIndex].EobPos =  (unsigned int)(MergedScanOrderPtr - pbi->MergedScanOrder);

    } //for(blockIndex = 0; blockIndex < 6; blockIndex++)
    //}while(++bdi < bdiEnd);

}

/****************************************************************************
****************************************************************************/

void
VP6_ReadTokensPredictA_MB(PB_INSTANCE *pbi) 
{
    BLOCK_DX_INFO *bdi = pbi->mbi.blockDxInfo;
    BLOCK_DX_INFO *bdiEnd = bdi + 6;

    INT32 token;

    int count = pbi->mbi.br->count;
    unsigned int range = pbi->mbi.br->range;
    unsigned int value = pbi->mbi.br->value;

    UINT8 *brBuffer = pbi->mbi.br->buffer;

    UINT8 *MergedScanOrder = pbi->MergedScanOrder;
    UINT8 *MergedScanOrderEnd = pbi->MergedScanOrder + BLOCK_SIZE;
    UINT8 *MergedScanOrderPtr;

    //bdi->br->buffer += bdi->br->pos;
    brBuffer += pbi->mbi.br->pos;

    //register __m64 m64_brCount;
    //__m64 m64_brBuffer;

    //{
      //  BOOL_CODER *br = pbi->mbi.blockDxInfo[0].br;
        //m64_brCount =  _mm_cvtsi32_si64((int)br->count);
    //}

#define BaselineProbsPtr bdi->BaselineProbsPtr
#define ContextProbsPtr bdi->ContextProbsPtr
#define AcProbsPtr bdi->AcProbsBasePtr
//#define token bdi->token

    do
    {
        MergedScanOrderPtr = MergedScanOrder;

        ContextProbsPtr = bdi->DcNodeContextsBasePtr + DcNodeOffset(0, (bdi->Left->Token + bdi->Above->Token), 0);
        BaselineProbsPtr = bdi->DcProbsBasePtr;

	    // Decode the dc token -- first test to see if it is zero
        NDECODEBOOL_AND_BRANCH_IF_ONE(ContextProbsPtr[ZERO_CONTEXT_NODE], DC_NON_ZERO_);

		// Zero is implicit for DC token
        //*(bdi->PrecTokenIndexPtr) = 0;
		bdi->Left->Token = 0;					// Update the above and left token contexts to indicate a zero
		bdi->Above->Token = 0;

        MergedScanOrderPtr++;
        BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 0, *(MergedScanOrderPtr + 64), 0 );

        goto AC_DO_WHILE;

DC_NON_ZERO_:
	    // A non zero DC value
		bdi->Left->Token = 1;					// Update the above and left token contexts to indicate non zero
		bdi->Above->Token = 1;

		// Was the value a 1
        NDECODEBOOL_AND_BRANCH_IF_ZERO(ContextProbsPtr[ONE_CONTEXT_NODE], ONE_CONTEXT_NODE_0_);

		//PrecTokenIndex = 2;		
        //*(bdi->PrecTokenIndexPtr) = 2;

		// Value token > 1
        NDECODEBOOL_AND_BRANCH_IF_ZERO(ContextProbsPtr[LOW_VAL_CONTEXT_NODE], LOW_VAL_CONTEXT_NODE_0_);
								
		// High value (value category) token
        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[HIGH_LOW_CONTEXT_NODE], HIGH_LOW_CONTEXT_NODE_0_);

		// Cat3,Cat4 or Cat5
        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[CAT_THREEFOUR_CONTEXT_NODE], CAT_THREEFOUR_CONTEXT_NODE_0_);

		token = DCT_VAL_CATEGORY5;

        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[CAT_FIVE_CONTEXT_NODE], DC_EXTRA_BITS_);

        token += 1;

        goto DC_EXTRA_BITS_;
                
CAT_THREEFOUR_CONTEXT_NODE_0_:
		token = DCT_VAL_CATEGORY3;
        
        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[CAT_THREE_CONTEXT_NODE], DC_EXTRA_BITS_);

        token += 1;

        goto DC_EXTRA_BITS_;

HIGH_LOW_CONTEXT_NODE_0_:
		// Either Cat1 or Cat2
		token = DCT_VAL_CATEGORY1;

        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[CAT_ONE_CONTEXT_NODE], DC_EXTRA_BITS_);

        token += 1;

DC_EXTRA_BITS_:
        {
            INT32       tValue;
            INT32		BitsCount;

	        unsigned int split;
	
        	tValue = VP6_TokenExtraBits2[token].MinVal;	

			// Read the extra bits
			BitsCount = VP6_TokenExtraBits2[token].Length;
			do
			{
				//value += (NDECODEBOOL(VP6_TokenExtraBits2[token].Probs[BitsCount]) << BitsCount );
	            // perform the actual decoding
	            split = 1 +  (((range-1) * VP6_TokenExtraBits2[token].Probs[BitsCount] ) >> 8);	

	            if ( value >= split<<24 )
	            {
		            value -= (split<<24);
		            split = range - split;
        
            		tValue += (1 << BitsCount);

	            }

                while(split < 0x80 )
	            {
		            split += split;
		            value += value;
		            
		            if ( !--count ) 
		            {
			            count  = 8;
			            value |= *brBuffer;
			            brBuffer++;
    	            }
                }
	            range = split;

			}
			while(--BitsCount >= 0);


			// apply the sign to the value
            APPLYSIGN(bdi->coeffsPtr[0], tValue);

		    MergedScanOrderPtr++;
            BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 2, *(MergedScanOrderPtr + 64), 0 );

            goto AC_DO_WHILE;
        }

LOW_VAL_CONTEXT_NODE_0_:
		// Low value token
        NDECODEBOOL_AND_BRANCH_IF_ZERO(ContextProbsPtr[TWO_CONTEXT_NODE], TWO_CONTEXT_NODE_0_);

		// Either a 3 or a 4
		token = THREE_TOKEN;

        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[THREE_CONTEXT_NODE], THREE_CONTEXT_NODE_0_);
        
        token += 1;

THREE_CONTEXT_NODE_0_:
		// apply the sign to the value
        APPLYSIGN(bdi->coeffsPtr[0], token);

		MergedScanOrderPtr++;
        BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 2, *(MergedScanOrderPtr + 64), 0 );

        goto AC_DO_WHILE;

TWO_CONTEXT_NODE_0_:
		// Is it a  2
		// apply the sign to the value
        APPLYSIGN(bdi->coeffsPtr[0], TWO_TOKEN);

		MergedScanOrderPtr++;
        BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 2, *(MergedScanOrderPtr + 64), 0 );

        goto AC_DO_WHILE;

ONE_CONTEXT_NODE_0_:
        MergedScanOrderPtr++;
        BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 1, *(MergedScanOrderPtr + 64), 0 );

		// apply the sign to the value
        APPLYSIGN(bdi->coeffsPtr[0], 1);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AC_DO_WHILE:
		// calculate the context for the next token. 
        NDECODEBOOL_AND_BRANCH_IF_ONE(BaselineProbsPtr[ZERO_CONTEXT_NODE], NON_ZERO_RUN_);

//ZERO_RUN_:
		// Is the token a Zero or EOB
        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[EOB_CONTEXT_NODE], BLOCK_FINISHED_1);

		// Select the appropriate Zero run context
		BaselineProbsPtr = bdi->ZeroRunProbsBasePtr;
		
        if(MergedScanOrderPtr >= (pbi->MergedScanOrder + ZRL_BAND2))
            BaselineProbsPtr += ZERO_RUN_PROB_CASES;

		// Now decode the zero run length
		// Run lenght 1-4
        NDECODEBOOL_AND_BRANCH_IF_ONE(BaselineProbsPtr[0], ZERO_RUN_5_8);

//ZERO_RUN_1_4:
        NDECODEBOOL_AND_BRANCH_IF_ONE(BaselineProbsPtr[1], ZERO_RUN_1_4_a);

		MergedScanOrderPtr += 1;

        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[2], ZERO_RUN_1_4_done);

		MergedScanOrderPtr += 1;

ZERO_RUN_1_4_done:
    	BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 0, *(MergedScanOrderPtr + 64), 0 );
        if( MergedScanOrderPtr < MergedScanOrderEnd)
            goto NON_ZERO_RUN_;

        goto BLOCK_FINISHED;
    
ZERO_RUN_1_4_a:
		MergedScanOrderPtr += 3;
        
        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[3], ZERO_RUN_1_4_a_done);

		MergedScanOrderPtr += 1;

ZERO_RUN_1_4_a_done:
    	BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 0, *(MergedScanOrderPtr + 64), 0 );
        if( MergedScanOrderPtr < MergedScanOrderEnd)
            goto NON_ZERO_RUN_;

        goto BLOCK_FINISHED;

ZERO_RUN_5_8:
		// Run length 5-8
        NDECODEBOOL_AND_BRANCH_IF_ONE(BaselineProbsPtr[4], ZERO_RUN_gt_8);

        NDECODEBOOL_AND_BRANCH_IF_ONE(BaselineProbsPtr[5], ZERO_RUN_5_8_a);

		MergedScanOrderPtr += 5;
        
        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[6], ZERO_RUN_5_8_done);

		MergedScanOrderPtr += 1;

ZERO_RUN_5_8_done:
    	BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 0, *(MergedScanOrderPtr + 64), 0 );
        if( MergedScanOrderPtr < MergedScanOrderEnd)
            goto NON_ZERO_RUN_;

        goto BLOCK_FINISHED;

ZERO_RUN_5_8_a:
		MergedScanOrderPtr += 7;
        
        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[7], ZERO_RUN_5_8_a_done);

		MergedScanOrderPtr += 1;

ZERO_RUN_5_8_a_done:
    	BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 0, *(MergedScanOrderPtr + 64), 0 );
        if( MergedScanOrderPtr < MergedScanOrderEnd)
            goto NON_ZERO_RUN_;

        goto BLOCK_FINISHED;

ZERO_RUN_gt_8:
		// Run length > 8
        {
	        unsigned int decodeCount;
	        unsigned int split;

            decodeCount = 0;
            do
            {
	            // perform the actual decoding
	            split = 1 +  (((range-1) * BaselineProbsPtr[8 + decodeCount]) >> 8);	

	            if ( value >= split<<24 )
	            {
		            value -= (split<<24);
		            split = range - split;
        
            		MergedScanOrderPtr += (1 << decodeCount);

	            }

                while(split < 0x80 )
	            {
		            split += split;
		            value += value;
		            
		            if ( !--count ) 
		            {
			            count  = 8;
			            value |= *brBuffer;
			            brBuffer++;
		            }
	            }

	            range = split;

            } while (++decodeCount < 6);

    		MergedScanOrderPtr += 9;

        } 

        if( MergedScanOrderPtr >= MergedScanOrderEnd)
            goto BLOCK_FINISHED;
        
    	BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 0, *(MergedScanOrderPtr + 64), 0 );


NON_ZERO_RUN_:
		// The token codes a non zero value

        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[ONE_CONTEXT_NODE], AC_ONE_CONTEXT_0_);

 		// Value token > 1
        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[LOW_VAL_CONTEXT_NODE], AC_LOW_VAL_CONTEXT_0_);
                   								
    	// High value (value category) token
        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[HIGH_LOW_CONTEXT_NODE], AC_HIGH_LOW_CONTEXT_0_);

		// Cat3,Cat4
        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[CAT_THREEFOUR_CONTEXT_NODE], AC_CAT_THREEFOUR_CONTEXT_0_);

		token = DCT_VAL_CATEGORY5;

		// Cat5,Cat6
        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[CAT_FIVE_CONTEXT_NODE], AC_EXTRA_BITS_);

        //It is Cat6
        token += 1;

        goto AC_EXTRA_BITS_;

AC_CAT_THREEFOUR_CONTEXT_0_:
		token = DCT_VAL_CATEGORY3;

        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[CAT_THREE_CONTEXT_NODE], AC_EXTRA_BITS_);

        //It is Cat4
        token += 1;

        goto AC_EXTRA_BITS_;

AC_HIGH_LOW_CONTEXT_0_:
		// Either Cat1 or Cat2
		token = DCT_VAL_CATEGORY1;

        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[CAT_ONE_CONTEXT_NODE], AC_EXTRA_BITS_);
        
        //It is Cat2
        token += 1;

AC_EXTRA_BITS_:
		{
            INT32 BitsCount;
            INT32 tValue;

	        unsigned int split;

    		tValue = VP6_TokenExtraBits2[token].MinVal;	

            // Read the extra bits
			BitsCount = VP6_TokenExtraBits2[token].Length;

			do
			{
				//tValue += (NDECODEBOOL(VP6_TokenExtraBits2[token].Probs[BitsCount]) << BitsCount);
	            split = 1 +  (((range-1) * VP6_TokenExtraBits2[token].Probs[BitsCount] ) >> 8);	

	            if ( value >= split<<24 )
	            {
		            value -= (split<<24);
		            split = range - split;
        
            		tValue += (1 << BitsCount);

	            }

                while(split < 0x80 )
	            {
		            split += split;
		            value += value;
		            
		            if ( !--count ) 
		            {
			            count  = 8;
			            value |= *brBuffer;
			            brBuffer++;
		            }
                }

	            range = split;
			}
			while(--BitsCount >= 0);


        	// apply the sign to the value
            APPLYSIGN(bdi->coeffsPtr[*(MergedScanOrderPtr)], tValue);
            MergedScanOrderPtr++;
        }


    	//*(bdi->PrecTokenIndexPtr) = 2;
        BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 2, *(MergedScanOrderPtr + 64), 0 );

        if( MergedScanOrderPtr < MergedScanOrderEnd)
            goto AC_DO_WHILE;
        
        goto BLOCK_FINISHED;


AC_LOW_VAL_CONTEXT_0_:
		// Low value token
        NDECODEBOOL_AND_BRANCH_IF_ZERO(BaselineProbsPtr[TWO_CONTEXT_NODE], AC_TWO_CONTEXT_0_);

		// Either a 3 or a 4
		token = THREE_TOKEN + 1;
        
        NDECODEBOOL_AND_BRANCH_IF_ONE(BaselineProbsPtr[THREE_CONTEXT_NODE], AC_THREE_CONTEXT_1_);

        //It is a 3
        token = token - 1;

AC_THREE_CONTEXT_1_:
        // apply the sign to the value
        APPLYSIGN(bdi->coeffsPtr[*(MergedScanOrderPtr)], token);
    	MergedScanOrderPtr++;

    	//*(bdi->PrecTokenIndexPtr) = 2;
        BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 2, *(MergedScanOrderPtr + 64), 0 );

        if( MergedScanOrderPtr < MergedScanOrderEnd)
            goto AC_DO_WHILE;
        
        goto BLOCK_FINISHED;


AC_TWO_CONTEXT_0_:
		// Is it a  2
        // apply the sign to the TWO_TOKEN
        APPLYSIGN(bdi->coeffsPtr[*(MergedScanOrderPtr)], TWO_TOKEN);
    	MergedScanOrderPtr++;

    	//*(bdi->PrecTokenIndexPtr) = 2;
        BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 2, *(MergedScanOrderPtr + 64), 0 );

        if( MergedScanOrderPtr < MergedScanOrderEnd)
            goto AC_DO_WHILE;
        
        goto BLOCK_FINISHED;

AC_ONE_CONTEXT_0_:
		// apply the sign to the value
        APPLYSIGN(bdi->coeffsPtr[*(MergedScanOrderPtr)], 1);

        MergedScanOrderPtr++;

		//*(bdi->PrecTokenIndexPtr) = 1;

        BaselineProbsPtr = AcProbsPtr + ACProbOffset(0, 1, *(MergedScanOrderPtr + 64), 0 );
    
        if( MergedScanOrderPtr < MergedScanOrderEnd)
            goto AC_DO_WHILE;
	
BLOCK_FINISHED:
        MergedScanOrderPtr--;

BLOCK_FINISHED_1:				    
	    bdi->EobPos =  (unsigned int)(MergedScanOrderPtr - MergedScanOrder);
    }while(++bdi < bdiEnd); 

    //bdi = pbi->mbi.blockDxInfo;
    brBuffer -= pbi->mbi.br->pos;
    pbi->mbi.br->pos += (unsigned int)(brBuffer - pbi->mbi.br->buffer);
    //bdi->br->buffer = brBuffer;

    pbi->mbi.br->count = count;
    pbi->mbi.br->value = value;
    pbi->mbi.br->range = range;

}


/****************************************************************************
 * 
 *  ROUTINE       :     VP6_DecodeMacroBlock
 *
 *  INPUTS        :     PB_INSTANCE *pbi  : Pointer to decoder instance.
 *                      UINT32 MBrow      : Row of MBs that block is in.
 *                  	UINT32 MBcol      : Col of MBs that block is in.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Decodes a single MacroBlock.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
INLINE
void 
VP6_DecodeMacroBlock ( PB_INSTANCE *pbi, UINT32 MBrow, UINT32 MBcol )
{
    UINT32 thisRecon;
    UINT32 bp;

    MACROBLOCK_INFO *mbi = &pbi->mbi;

    //***********************************************************************
    // Copy the existing structures into what we have now I'll fix this next.

	pbi->mbi.Mode = CODE_INTRA;
//	pbi->mbi.Interlaced = 0;

	// dumb way to encode the interlaced decision but it works!!!
	if(pbi->Configuration.Interlaced)
	{
		UINT8 prob = pbi->probInterlaced;
		
        // super simple context adjustment
		if(MBcol>BORDER_MBS)
		{
			// adjust the probability per the last one we did 
			if(pbi->mbi.Interlaced)
				prob = prob-(prob>>1);
			else 
				prob = prob+((256-prob)>>1);
		}
		pbi->mbi.Interlaced = nDecodeBool(	&pbi->br, prob);

	    if ( pbi->mbi.Interlaced == 1 )
	    {
		    pbi->mbi.blockDxInfo[0].CurrentReconStride = 
		    pbi->mbi.blockDxInfo[1].CurrentReconStride = 
		    pbi->mbi.blockDxInfo[2].CurrentReconStride = 
		    pbi->mbi.blockDxInfo[3].CurrentReconStride = pbi->Configuration.YStride * 2;

	        pbi->mbi.blockDxInfo[2].thisRecon -= (pbi->Configuration.YStride * 7);
	        pbi->mbi.blockDxInfo[3].thisRecon -= (pbi->Configuration.YStride * 7);

	    }
	}

	if(pbi->FrameType != BASE_FRAME )
	{
		VP6_decodeModeAndMotionVector ( pbi, MBrow, MBcol );
	}

	// read tokens from the bitstream and convert to coefficients.
    if ( pbi->UseHuffman )
    {
        ReadHuffTokensPredictA_MB(pbi);
    }
    else
    {
        VP6_ReadTokensPredictA_MB(pbi);
    }

    VP6_PredictDC_MB(pbi);

    bp = 0;
    do    
	{	
//note: maybe offset table can contain a func ptr and the amount to meset
//we can then get rid of the if then else....
        UINT32 EOBPos = pbi->EobOffsetTable[mbi->blockDxInfo[bp].EobPos];

        // Default clear data area down to 0s
        if ( EOBPos <= 1 )
        {
		    idct[1]( mbi->blockDxInfo[bp].coeffsPtr, mbi->blockDxInfo[bp].dequantPtr, pbi->ReconDataBuffer[bp] );
	        mbi->blockDxInfo[bp].coeffsPtr[0] = 0;
        }
        else if ( EOBPos <= 10 )
        {
		    idct[9]( mbi->blockDxInfo[bp].coeffsPtr, mbi->blockDxInfo[bp].dequantPtr, pbi->ReconDataBuffer[bp] );
	        memset(mbi->blockDxInfo[bp].coeffsPtr,    0,8*sizeof(Q_LIST_ENTRY));
	        memset(mbi->blockDxInfo[bp].coeffsPtr+8,  0,4*sizeof(Q_LIST_ENTRY));
	        memset(mbi->blockDxInfo[bp].coeffsPtr+16, 0,4*sizeof(Q_LIST_ENTRY));
	        memset(mbi->blockDxInfo[bp].coeffsPtr+24, 0,4*sizeof(Q_LIST_ENTRY));
            //if(mbi->Coeffs[bp][32] )
                mbi->blockDxInfo[bp].coeffsPtr[32] =0;
        }
        else 
        {
		    idct[63]( mbi->blockDxInfo[bp].coeffsPtr, mbi->blockDxInfo[bp].dequantPtr, pbi->ReconDataBuffer[bp] );
	        memset(mbi->blockDxInfo[bp].coeffsPtr, 0, 64*sizeof(Q_LIST_ENTRY));
        }

    } while(++bp < 6);



//note:all of the recon function should be written for mb's not blocks
//also lets create a func table that selects the recon based on mode
//i hate if then elses........

    bp = 0;
    // Action depends on decode mode.
	if ( pbi->mbi.Mode == CODE_INTER_NO_MV )       // Inter with no motion vector
	{
        do
        {
            thisRecon = pbi->mbi.blockDxInfo[bp].thisRecon;
		    ReconInter( pbi->TmpDataBuffer, 
                            (UINT8 *)&pbi->ThisFrameRecon[thisRecon], 
			                (UINT8 *)&pbi->LastFrameRecon[thisRecon], 
			                pbi->ReconDataBuffer[bp], 
                            pbi->mbi.blockDxInfo[bp].CurrentReconStride);
        } while(++bp < 6);
    }
	else if ( VP6_ModeUsesMC[pbi->mbi.Mode] )          // The mode uses a motion vector.
	{
        do
        {
            thisRecon = pbi->mbi.blockDxInfo[bp].thisRecon;
		    // For the compressor we did this already ( possible optimization).
		    VP6_PredictFilteredBlock( pbi, pbi->TmpDataBuffer,bp);

		    ReconBlock( pbi->TmpDataBuffer,
			                pbi->ReconDataBuffer[bp],
			                (UINT8 *)&pbi->ThisFrameRecon[thisRecon],
			                pbi->mbi.blockDxInfo[bp].CurrentReconStride);
        } while(++bp < 6);
	}
	else if ( pbi->mbi.Mode == CODE_USING_GOLDEN )     // Golden frame with motion vector
	{
        do
        {
            thisRecon = pbi->mbi.blockDxInfo[bp].thisRecon;
		    // Reconstruct the pixel data using the golden frame reconstruction and change data
		    ReconInter( pbi->TmpDataBuffer, 
                            (UINT8 *)&pbi->ThisFrameRecon[thisRecon], 
			                (UINT8 *)&pbi->GoldenFrame[thisRecon], 
			                pbi->ReconDataBuffer[bp], 
                            pbi->mbi.blockDxInfo[bp].CurrentReconStride );
        } while(++bp < 6);
	}
	else                                            // Simple Intra coding
	{
        do
        {
            thisRecon = pbi->mbi.blockDxInfo[bp].thisRecon;
		    // Get the pixel index for the first pixel in the fragment.
		    ReconIntra( pbi->TmpDataBuffer, 
                            (UINT8 *)&pbi->ThisFrameRecon[thisRecon], 
                            (UINT16 *)pbi->ReconDataBuffer[bp], 
                            pbi->mbi.blockDxInfo[bp].CurrentReconStride);
        } while(++bp < 6);
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
    }


}

/****************************************************************************
 * 
 *  ROUTINE       :     DecodeFrameMbs
 *
 *  INPUTS        :     PB_INSTANCE *pbi  : Pointer to decoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     void
 *
 *  FUNCTION      :     Decodes all the MacroBlocks of a frame.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
void VP6_DecodeFrameMbs ( PB_INSTANCE *pbi )
{
    //UINT32 blockIndex;

	if(pbi->FrameType != BASE_FRAME )
	{
		VP6_DecodeModeProbs(pbi);
		VP6_ConfigureMvEntropyDecoder( pbi, pbi->FrameType );
        pbi->LastMode = CODE_INTER_NO_MV;
	}
	else
	{
		memcpy ( pbi->probXmitted,VP6_BaselineXmittedProbs,sizeof(pbi->probXmitted));

		memcpy ( pbi->IsMvShortProb, DefaultIsShortProbs, sizeof(pbi->IsMvShortProb) );
		memcpy ( pbi->MvShortProbs, DefaultMvShortProbs, sizeof(pbi->MvShortProbs) );
		memcpy ( pbi->MvSignProbs, DefaultSignProbs, sizeof(pbi->MvSignProbs) );
		memcpy ( pbi->MvSizeProbs, DefaultMvLongProbs, sizeof(pbi->MvSizeProbs) );

		memset ( pbi->MBModeProb,128,sizeof(pbi->MBModeProb));
		memset ( pbi->BModeProb,128,sizeof(pbi->MBModeProb));
		memset ( pbi->predictionMode,1,sizeof(char)*pbi->MacroBlocks );

		// Set up default scan order banding
		if( pbi->Configuration.Interlaced == 1 )
			memcpy( pbi->ScanBands, DefaultInterlacedScanBands, sizeof(pbi->ScanBands) );
		else
			memcpy( pbi->ScanBands, DefaultNonInterlacedScanBands, sizeof(pbi->ScanBands) );

		// Build the scan order
		BuildScanOrder( pbi, pbi->ScanBands );


	}

	VP6_ConfigureEntropyDecoder( pbi, pbi->FrameType ); 

    {
        UINT32  i;

        for(i=0;i<64;i++)
        {
            pbi->MergedScanOrder[i] = pbi->quantizer->transIndex[pbi->ModifiedScanOrder[i]];
        }


        // Create Huffman codes for tokens based on tree probabilities
        if ( pbi->UseHuffman )
        {
            ConvertBoolTrees ( pbi );

            for(i = 64; i < 64+65; i++)
            {
                pbi->MergedScanOrder[i] = VP6_CoeffToHuffBand[i - 64];
            }

            // Reset Dc zero & Ac EOB run counters
            pbi->CurrentDcRunLen[0]  = 0;
            pbi->CurrentDcRunLen[1]  = 0;
            pbi->CurrentAc1RunLen[0] = 0;
            pbi->CurrentAc1RunLen[1] = 0;
        }
        else
        {
            for(i = 64; i < 64+65; i++)
            {
                pbi->MergedScanOrder[i] = VP6_CoeffToBand[i - 64];
            }
        }
    }

	if(pbi->Configuration.Interlaced == 1)
		pbi->probInterlaced = ((UINT8)VP6_bitread( &pbi->br,   8 ));  

	// since we are on a new frame reset the above contexts 
	VP6_ResetAboveContext(pbi);

    {
	    UINT32 MBrow;
	    UINT32 MBRows = pbi->MBRows; 
	    UINT32 MBCols = pbi->MBCols;

        MBCols -= BORDER_MBS;
        MBRows -= BORDER_MBS;

        // for each row of macroblocks 
	    MBrow=BORDER_MBS;
        do
	    {
            MACROBLOCK_INFO *mbi = &pbi->mbi;
            UINT32 MBcol;

		    VP6_ResetLeftContext(pbi);

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


            MBcol=BORDER_MBS;
            do
            {
			    // Decode the macroblock
			    VP6_DecodeMacroBlock(pbi, MBrow, MBcol);   


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

		    } while(++MBcol < MBCols);


	    } while(++MBrow < MBRows);
    }
}
