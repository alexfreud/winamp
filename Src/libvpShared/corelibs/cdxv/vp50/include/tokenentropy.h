/****************************************************************************
*
*   Module Title :     TokenEntropy.h
*
*   Description  :     Video CODEC: Coefficient toke entropy header.
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
*   1.01 PGW 27 Jun 01  Module created.
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/


/****************************************************************************
*  Header Frames
*****************************************************************************
*/


#ifndef TOKEN_ENTROPY_H
#define TOKEN_ENTROPY_H

#include "type_aliases.h"
#include "boolhuff.h"
#include "codec_common.h"
#include "huffman.h"


/****************************************************************************
*  Constants
*****************************************************************************
*/

// VP5 hufman table AC bands
#define VP5_AC_BANDS			6

// Tokens								Value		Extra Bits (range + sign)
#define ZERO_TOKEN              0		//0			Extra Bits 0+0
#define ONE_TOKEN               1		//1			Extra Bits 0+1       
#define TWO_TOKEN               2		//2			Extra Bits 0+1 
#define THREE_TOKEN             3		//3			Extra Bits 0+1
#define FOUR_TOKEN              4		//4			Extra Bits 0+1
#define DCT_VAL_CATEGORY1		5		//5-6		Extra Bits 1+1
#define DCT_VAL_CATEGORY2		6		//7-10		Extra Bits 2+1
#define DCT_VAL_CATEGORY3		7		//11-26		Extra Bits 4+1
#define DCT_VAL_CATEGORY4		8		//11-26		Extra Bits 5+1
#define DCT_VAL_CATEGORY5		9		//27-58		Extra Bits 5+1
#define DCT_VAL_CATEGORY6		10		//59+		Extra Bits 11+1	
#define DCT_EOB_TOKEN           11		//EOB		Extra Bits 0+0
#define MAX_ENTROPY_TOKENS      (DCT_EOB_TOKEN + 1)  
#define ILLEGAL_TOKEN			255


#define TOKEN_CONTEXTS			6 // EOB, 0, 1, 2, 3-4, x
#define CONTEXT_NODES			(MAX_ENTROPY_TOKENS-7)

#define PREC_CASES				3

#define DC_PROBABILITY_UPDATE_THRESH	100

#define ZERO_CONTEXT_NODE		0
#define EOB_CONTEXT_NODE		1
#define ONE_CONTEXT_NODE		2
#define LOW_VAL_CONTEXT_NODE	3
#define TWO_CONTEXT_NODE		4
#define THREE_CONTEXT_NODE		5
#define HIGH_LOW_CONTEXT_NODE	6
#define CAT_ONE_CONTEXT_NODE	7
#define CAT_THREEFOUR_CONTEXT_NODE	8
#define CAT_THREE_CONTEXT_NODE	9
#define CAT_FIVE_CONTEXT_NODE	10

#define PROB_UPDATE_BASELINE_COST	7

#define MAX_PROB				254
#define DCT_MAX_VALUE			2048


/****************************************************************************
*  Types
*****************************************************************************
*/        

extern const UINT32 ProbCost[256];

extern const UINT8  ExtraBitLengths_VP5[MAX_ENTROPY_TOKENS];
extern const UINT32 DctRangeMinVals[MAX_ENTROPY_TOKENS];

typedef struct LineEq
{
    INT32	M;
    INT32	C;
} LINE_EQ;


extern const UINT8 DcUpdateProbs[2][MAX_ENTROPY_TOKENS-1];
extern const UINT8 AcUpdateProbs[PREC_CASES][2][VP5_AC_BANDS][MAX_ENTROPY_TOKENS-1];
extern const UINT8 PrevTokenIndex[MAX_ENTROPY_TOKENS];

extern UINT8 PrecZeroRunLength[BLOCK_SIZE];

/****************************************************************************
*   Data structures
*****************************************************************************
*/


// These table contains the normailized probabilities required to traverse the
// entropy tree for DC and AC value tokens representing values >= 2
// Probabilities are normalized to 8 bits and represent the likelyhood of a zero branch.


/****************************************************************************
*  Functions
*****************************************************************************
*/


#endif

