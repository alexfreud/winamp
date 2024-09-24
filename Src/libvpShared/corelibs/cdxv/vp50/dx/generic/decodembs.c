/****************************************************************************
*
*   Module Title :     Decodembs.c
*
*   Description  :     Compressor functions for block order transmittal
*
*   AUTHOR       :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*   1.28 YWX 27-Dec-01 Rewrote ReadTokensPredictA()
*   1.27 YWX 06-Nov-01 Removed Warning errors
*   1.26 JBB 13 Jun 01 VP4 Code Clean Out
*	1.25 AWG 08-JUN-01 Added support for DCT16.
*	1.24 AWG 22-MAY-01 Removed HExtra/VExtra from call to QuadCodeComponent2
*   1.23 JBB 01-MAY-01 VP5 Functionality
*   1.22 JBB 09-Apr-01 first pass file clean up 
*   1.21 JBB 23-Mar-01 New DC preidction
*   1.20 JBB 30 NOV 00 Configuration BaseLine
*****************************************************************************
*/
#define STRICT              /* Strict type checking. */
/****************************************************************************
*  Header Files
*****************************************************************************
*/

//#include "compdll.h"
//#include "misc_common.h"
#include "pbdll.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "codec_common_interface.h"
#include "tokenentropy.h"
#include "decodemode.h"
#include "decodemv.h"
/****************************************************************************
 *  Module constants.
 *****************************************************************************
 */     

#define DCT_MAX_VALUE	2048

// For details of tokens and extra bit breakdown see token definitions in huffman.h
typedef struct 
{    
    UINT16  MinVal;
    INT16   Length;
    UINT8   Probs[11];
} TOKENEXTRABITS;

const TOKENEXTRABITS TokenExtraBits2[ MAX_ENTROPY_TOKENS]=
{
    { 0, -1,{   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //ZERO_TOKEN
    { 1, 0, {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //ONE_TOKEN
    { 2, 0, {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //TWO_TOKEN
    { 3, 0, {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //THREE_TOKEN
    { 4, 0, {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //FOUR_TOKEN
    { 5, 0, {   159,0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //DCT_VAL_CATEGORY1
    { 7, 1, {   145,165,0,  0,  0,  0,  0,  0,  0,  0,  0   } },   //DCT_VAL_CATEGORY2
    { 11,2, {   140,148,173,0,  0,  0,  0,  0,  0,  0,  0   } },   //DCT_VAL_CATEGORY3
    { 19,3, {   135,140,155,176,0,  0,  0,  0,  0,  0,  0   } },   //DCT_VAL_CATEGORY4
    { 35,4, {   130,134,141,157,180,0,  0,  0,  0,  0,  0   } },   //DCT_VAL_CATEGORY5
    { 67,10,{   129,130,133,140,153,177,196,230,243,254,254 } },   //DCT_VAL_CATEGORY6
    { 0, -1,{   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   } },   // EOB TOKEN
};

const UINT32 LTIndex[MAX_ENTROPY_TOKENS] = { 0,1, 2, 3,3,4,4,4,4,4,4, 5 }; 
const INT32 CoeffToBand[65] = 
{	-1,0,1,1,2,1,1,2,
	2,1,1,2,2,2,1,2, 
	2,2,2,2,1,1,2,2,
	3,3,4,3,4,4,4,3,
	3,3,3,3,4,3,3,3,
	4,4,4,4,4,3,3,4,
	4,4,3,4,4,4,4,4,
	4,4,5,5,5,5,5,5,7
};


const UINT32 toggleBand3[]= { 4,5,7,9,11,14,15,20,22 };

const int VP5_Mode2Frame[] =
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
/****************************************************************************
*  Explicit imports
*****************************************************************************
*/ 
extern UINT32 LoopFilterLimitValuesV2[Q_TABLE_SIZE];
extern void decodeModeAndMotionVector(PB_INSTANCE *pbi,UINT32 MBrow,UINT32 MBcol);


INLINE 
int nDecodeBool128
(
	BOOL_CODER	* br
) 
{
    unsigned int bit;
	unsigned int split;
	unsigned int bigsplit;
    unsigned int count = br->count;
    unsigned int range = br->range;
    unsigned int value = br->value;
    
    split = ( range + 1) >> 1;
    bigsplit = (split<<24);
    bit = (value >= bigsplit);
    range  =  bit?range-split:split;    
    value  =  bit?value-bigsplit:value;        
    value += value;
    range += range;
    if(!--count)
    {
        count=8;
        value |= br->buffer[br->pos];
        br->pos++;
        
    }
    br->count = count;
    br->value = value;
    br->range = range;
    return bit;
        
}    

INLINE
int nDecodeBool
(
	BOOL_CODER	* br,
	int probability
) 
{

    unsigned int bit=0;
	unsigned int split;
	unsigned int bigsplit;
    int count = br->count;
    unsigned int range = br->range;
    unsigned int value = br->value;

	// perform the actual encoding
	split = 1 +  (((range-1) * probability) >> 8);	
    bigsplit = (split<<24);

	if(value >= bigsplit)
	{
		range = range-split;
		value = value-bigsplit;
		bit = 1;
	}
	else
	{	
		range = split;
	}
    while(range < 0x80 )
	{
		range +=range;
		value +=value;
		
		if (!--count) 
		{
			count = 8;
			value |= br->buffer[br->pos];
			br->pos++;
		}
	}
    br->count = count;
    br->value = value;
    br->range = range;
	return bit;
} 


/****************************************************************************
* 
*  ROUTINE       :     ConfigureEntropyDecoder
*
*  INPUTS        :     None
*
*  OUTPUTS       :     None
*
*  RETURNS       :     None.
*
*  FUNCTION      :     Configure entropy subsystem for decode
*
*  SPECIAL NOTES :     None. 
*
*
*  ERRORS        :     None.
*
****************************************************************************/
void ConfigureEntropyDecoder( PB_INSTANCE *pbi, UINT8 FrameType )
{
	UINT32	i;
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
			if ( nDecodeBool(&pbi->br, DcUpdateProbs[Plane][i] ) )
			{
				// 0 is not a legal value.
				LastProb[i] = VP5_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
				LastProb[i] += ( LastProb[i] == 0 );
				pbi->DcProbs[DCProbOffset(Plane,i)] = LastProb[i];

			}
			else if ( FrameType == BASE_FRAME )
			{
				pbi->DcProbs[DCProbOffset(Plane,i)] = LastProb[i];
			}
		}
	}


	// Read in the Baseline AC band probabilities and initialise the appropriate contexts
	// Prec=0 means last token in current block was 0: Prec=1 means it was !0
	for ( Prec = 0; Prec < PREC_CASES; Prec++ )
	{
		PrecNonZero = ( Prec > 0 ) ? 1 : 0;
		for ( Plane = 0; Plane < 2; Plane++ )
		{
			for ( Band = 0; Band < VP5_AC_BANDS; Band++ )
			{
				// If so then read them in.
				for ( i = 0; i < MAX_ENTROPY_TOKENS-1; i++ )
				{
					if ( nDecodeBool(&pbi->br, AcUpdateProbs[Prec][Plane][Band][i] ) )
					{
						// Probabilities transmitted at reduced resolution. 
						// 0 is not a legal value.
						LastProb[i] = VP5_bitread( &pbi->br, PROB_UPDATE_BASELINE_COST ) << 1;
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
	ConfigureContexts(pbi);

}

/****************************************************************************
 * 
 *  ROUTINE       :     ResetLeftContext
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Updates the left contexts
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ResetLeftContext
( 
	PB_INSTANCE *pbi
)
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

	pbi->fc.LeftY[0].EOBPos = 24;
	pbi->fc.LeftY[1].EOBPos = 24;
	pbi->fc.LeftU.EOBPos = 24;
	pbi->fc.LeftV.EOBPos = 24;
   
	
}

/****************************************************************************
 * 
 *  ROUTINE       :     ResetAboveContext
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Updates the above contexts
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ResetAboveContext
( 
	PB_INSTANCE *pbi
)
{
	UINT32 i;

	/*
    memset ((void *) pbi->fc.AboveY, 0, (pbi->HFragments+2)*sizeof(BLOCK_CONTEXT));
    memset ((void *) pbi->fc.AboveU, 0, (pbi->HFragments/2+2)*sizeof(BLOCK_CONTEXT));
    memset ((void *) pbi->fc.AboveV, 0, (pbi->HFragments/2+2)*sizeof(BLOCK_CONTEXT));
    */
    for ( i = 0 ; i < pbi->HFragments+8;i++)
	{
        pbi->fc.AboveY[i].Mode = -1;
        pbi->fc.AboveY[i].Frame = 4;
		pbi->fc.AboveY[i].Dc =0;
		pbi->fc.AboveY[i].Tokens[0]=0;


	}
	for ( i = 0 ; i < pbi->HFragments/2 + 8;i++)
	{        
        pbi->fc.AboveU[i].Mode = -1;
        pbi->fc.AboveU[i].Frame = 4;
		pbi->fc.AboveU[i].Tokens[0]=0;
		pbi->fc.AboveU[i].Dc=0;
        pbi->fc.AboveV[i].Mode = -1;
        pbi->fc.AboveV[i].Frame = 4;  
		pbi->fc.AboveV[i].Tokens[0]=0;
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
 *  ROUTINE       :     UpdateContext
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Updates the frame context
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void UpdateContext
( 
	PB_INSTANCE *pbi,
	BLOCK_CONTEXT *c,
	BLOCK_POSITION bp
)
{
	c->Mode = pbi->mbi.BlockMode[bp];
	c->Dc = pbi->mbi.Coeffs[bp][0];
	c->Frame = VP5_Mode2Frame[pbi->mbi.Mode];
}

/****************************************************************************
 * 
 *  ROUTINE       :     UpdateContext
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Updates the frame context
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void UpdateContextA
( 
	PB_INSTANCE *pbi,
	BLOCK_CONTEXTA *c,
	BLOCK_POSITION bp
)
{
	c->Mode = pbi->mbi.BlockMode[bp];
	c->Dc = pbi->mbi.Coeffs[bp][0];
	c->Frame = VP5_Mode2Frame[pbi->mbi.Mode];
}



/****************************************************************************
 * 
 *  ROUTINE       :     PredictDc
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Predicts coefficients in this block based on the 
 *                      contexts we have
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
#define HIGHBITDUPPED(X) (((signed short) X)  >> 15)
void PredictDC
( 
	PB_INSTANCE *pbi,
	BLOCK_POSITION bp,
	Q_LIST_ENTRY *LastDC,
	BLOCK_CONTEXTA *Above,
	BLOCK_CONTEXT *Left
)
{
	UINT8 Frame = VP5_Mode2Frame[pbi->mbi.Mode];
	UINT8 Count = 0;
#if 0
	INT32 Avg = 0;
	if( Frame==Left->Frame) 
	{
		Avg += Left->Dc;
		Count ++;
	}
	if( Frame==Above->Frame) 
	{
		Avg += Above->Dc;
		Count ++;
	}

	if( Count < 2 && Frame == Above[-1].Frame)
	{
		Avg += Above[-1].Dc;
		Count ++;
	}

	if( Count < 2 && Frame == Above[+1].Frame)
	{
		Avg += Above[+1].Dc;
		Count ++;
	}
	if(Count==0)
	{
		Avg = LastDC[Frame];
	}
	else if(Count==2)
	{
		// trick to determine when to add 1 if negative (for proper truncation)
		Avg += (HIGHBITDUPPED(Avg)&1);
		Avg >>= 1;
	}

#else
INT32 Avg ;
//state_L:
    if( Frame != Left->Frame) 
		goto state_A0;
	Avg = Left->Dc;
//	goto state_A1;

//state_A1:
	if(Frame != Above->Frame)
		goto state_AM1;
	Avg += Above->Dc;
//	goto state_TWO;

state_TWO:
    Avg += (HIGHBITDUPPED(Avg)&1);
	Avg >>= 1;
	goto state_done;

state_A0:
	if(Frame != Above->Frame)
		goto state_AM0;
	Avg = Above->Dc;
//	goto state_AM1;

state_AM1:
	if(Frame == Above[-1].Frame)
	{
		Avg += Above[-1].Dc;
		goto state_TWO;
	}
//	goto state_AP1;

state_AP1:
	if(Frame != Above[+1].Frame)
		goto state_done;
	Avg += Above[+1].Dc;
	goto state_TWO;


state_AM0:
	if(Frame == Above[-1].Frame)
	{
		Avg = Above[-1].Dc;
		goto state_AP1;
	}
	//goto state_AP0;

//state_AP0:
	if(Frame != Above[+1].Frame)
		Avg = LastDC[Frame];
	else
		Avg = Above[+1].Dc;

state_done:

#endif 

	pbi->mbi.Coeffs[bp][0] += Avg;
	LastDC[Frame] = pbi->mbi.Coeffs[bp][0];

	return ;
}


#define TI(x) (TransIndex[x]) 

/****************************************************************************
* 
*  ROUTINE       :     ReadTokensPredictA
*
*  INPUTS        :     None
*                               
*  OUTPUTS       :     None
*
*  RETURNS       :     None
*
*  FUNCTION      :     Fills CoeffData with one blocks worth of coefficients
*                      decoded from the bitstream.
*
*  SPECIAL NOTES :     
*
*
*  ERRORS        :     None.
*
****************************************************************************/
UINT8 ReadTokensPredictA(
	PB_INSTANCE *pbi,
	INT16 * CoeffData,
	UINT32 BlockSize,
	UINT32 Plane,
	BLOCK_CONTEXTA *Above,
	BLOCK_CONTEXT *Left
)
{
	INT32		token;
	BOOL_CODER	* br = &pbi->br;
	UINT8		EncodedCoeffs = 0;
	UINT8		LeftContext;
	UINT8		AboveContext;
	UINT8		*BaselineProbsPtr;
	UINT8		*ContextProbsPtr;
    BOOL        LastTokenNonZero;   // Was last token in this block non-zero
    UINT8       PrecTokenIndex;		// Preceeding token index
    UINT32      Band;
	INT32		SignBit;
	INT32		BitsCount ;
	UINT8		*AcProbsPtr = pbi->AcProbs + ACProbOffset(Plane,0,0,0);
	UINT8		*AcContextPtr = pbi->AcNodeContexts + ACContextOffset(Plane,0,0,0,0);
	BOOL        EOB = FALSE;
	UINT32      *TransIndex = pbi->quantizer->transIndex; 
	INT32       value;


	// determine the contexts for dc
	LastTokenNonZero = TRUE;
	LeftContext  = Left->Tokens[EncodedCoeffs];
	AboveContext = Above->Tokens[EncodedCoeffs];

	BaselineProbsPtr = pbi->DcProbs+DCProbOffset(Plane,0);
	ContextProbsPtr = pbi->DcNodeContexts+DCContextOffset(Plane,LeftContext,AboveContext,0);
	
	do
	{
		// First test for the ! ZeroContext
		if ( !nDecodeBool(br, ContextProbsPtr[ZERO_CONTEXT_NODE] ) )	 		
		{
			// Zero or EOB
			if ( LastTokenNonZero )	
			{
				if ( nDecodeBool(br, ContextProbsPtr[EOB_CONTEXT_NODE]) )
				{
					PrecTokenIndex = 0;
					Left->Tokens[EncodedCoeffs] = 0;
				}
				else 
				{
					EncodedCoeffs++;
					break;
				}
			}
			else
			{
				PrecTokenIndex = 0;
				Left->Tokens[EncodedCoeffs] = 0;
			}
			LastTokenNonZero = FALSE;
		}
		else
        {													
			
			// Was the value a 1
			if ( nDecodeBool(br, ContextProbsPtr[ONE_CONTEXT_NODE]) )
			{
				// Value token > 1
				if ( nDecodeBool(br, ContextProbsPtr[LOW_VAL_CONTEXT_NODE]) )
				{												
					// High value (value category) token
					Left->Tokens[EncodedCoeffs] = 4;
					if ( nDecodeBool(br, BaselineProbsPtr[HIGH_LOW_CONTEXT_NODE]) )
					{								
						// Cat3,Cat4 or Cat5
						if ( nDecodeBool(br, BaselineProbsPtr[CAT_THREEFOUR_CONTEXT_NODE]) )
						{
							token = DCT_VAL_CATEGORY5 + nDecodeBool(br, BaselineProbsPtr[CAT_FIVE_CONTEXT_NODE]);
						}
						else									
						{
							token = DCT_VAL_CATEGORY3 + nDecodeBool(br, BaselineProbsPtr[CAT_THREE_CONTEXT_NODE]);
						}
					}
					else
					{								
						// Either Cat1 or Cat2
						token = DCT_VAL_CATEGORY1 + nDecodeBool(br, BaselineProbsPtr[CAT_ONE_CONTEXT_NODE]);
					}


					// Get the Sign Bit
					SignBit = nDecodeBool128(br);

					value = TokenExtraBits2[token].MinVal;	

					// Read the extra bits
					BitsCount = TokenExtraBits2[token].Length;

					do
					{
						value += (nDecodeBool(br, TokenExtraBits2[token].Probs[BitsCount])<<BitsCount);
						BitsCount -- ;
					}
					while( BitsCount >= 0);


					// Combine the signa and value
					CoeffData[TI(EncodedCoeffs)] =(Q_LIST_ENTRY)((value ^ -SignBit) + SignBit); 

				}
				else
				{									
					// Low value token
					if ( nDecodeBool(br, ContextProbsPtr[TWO_CONTEXT_NODE]) )
					{											
						// Either a 3 or a 4
						Left->Tokens[EncodedCoeffs] = 3;
						token = THREE_TOKEN + nDecodeBool(br, BaselineProbsPtr[THREE_CONTEXT_NODE]);
					}
					else			
					{											
						// Is it a  2
						token = TWO_TOKEN;	
						Left->Tokens[EncodedCoeffs] = 2;
					}

					// Get the Sign Bit and store the result in our coeff array
			        SignBit = nDecodeBool128(br);
					CoeffData[TI(EncodedCoeffs)] =(Q_LIST_ENTRY)((token ^ -SignBit) + SignBit); 

				}
				PrecTokenIndex = 2;
			}
			else
			{
				PrecTokenIndex = 1;
				Left->Tokens[EncodedCoeffs] = 1;

				// Get the Sign Bit
		        SignBit = nDecodeBool128(br);

			    // Combine the signa and value
				CoeffData[TI(EncodedCoeffs)] =(Q_LIST_ENTRY)((1 ^ -SignBit) + SignBit); 
			}
			LastTokenNonZero = TRUE;

		}
		
		// calculate the context for the next token. 
        EncodedCoeffs ++;			
        Band = CoeffToBand [ EncodedCoeffs ];
        BaselineProbsPtr = AcProbsPtr + ACProbOffset(0,PrecTokenIndex,Band,0);
		if(Band < 3)
		{
			ContextProbsPtr = AcContextPtr + ACContextOffset(0,PrecTokenIndex,Band,Left->Tokens[EncodedCoeffs],0);			
		}
		else
		{
			if(EncodedCoeffs >= BlockSize)
				break;

			ContextProbsPtr = BaselineProbsPtr;
		}

        
	} while ( 1 );
	EncodedCoeffs --;
				
	return EncodedCoeffs;
}

/****************************************************************************
 * 
 *  ROUTINE       :     DecodeBlock
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Decodes A Block
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void DecodeBlock
( 
	PB_INSTANCE *pbi,
	UINT32 MBrow,
	UINT32 MBcol,
	BLOCK_POSITION bp
)
{

	unsigned int lastEOB = pbi->mbi.Left->EOBPos;

	if(lastEOB >24)
		lastEOB =24;

	// read tokens from the bitstream and convert to coefficients.
	pbi->mbi.Left->EOBPos = ReadTokensPredictA(pbi, pbi->mbi.Coeffs[bp], 64, (pbi->mbi.Plane!=0), pbi->mbi.Above, pbi->mbi.Left);

	// Update LEFT and ABOVE Contexts
	if(pbi->mbi.Left->EOBPos < lastEOB )
		memset (&pbi->mbi.Left->Tokens[pbi->mbi.Left->EOBPos], LTIndex[DCT_EOB_TOKEN], lastEOB - pbi->mbi.Left->EOBPos);

	pbi->mbi.Above->Tokens[0] = pbi->mbi.Left->Tokens[0];

	// predict our dc values from the surrounding guys
	PredictDC(pbi, bp, pbi->mbi.LastDc, pbi->mbi.Above, pbi->mbi.Left);

	// do the inverse transform
	pbi->idct[pbi->mbi.Left->EOBPos]( pbi->mbi.Coeffs[bp], pbi->quantizer->dequant_coeffs[QTableSelect[bp]], pbi->ReconDataBuffer );

	// put it into our reconstruction buffer
	ReconstructBlock(pbi,bp);
	
	// update the context info for the next block 
	UpdateContextA(pbi,pbi->mbi.Above,bp);
	UpdateContext(pbi,pbi->mbi.Left,bp);

	// Default clear data area down to 0s
	if(pbi->mbi.Left->EOBPos <= 1)
	{
		pbi->mbi.Coeffs[bp][0] = 0;
	}
	else if(pbi->mbi.Left->EOBPos <= 10)
	{
	    memset(pbi->mbi.Coeffs[bp], 0,8*sizeof(Q_LIST_ENTRY));
	    memset(pbi->mbi.Coeffs[bp]+8, 0,4*sizeof(Q_LIST_ENTRY));
	    memset(pbi->mbi.Coeffs[bp]+16, 0,4*sizeof(Q_LIST_ENTRY));
	    memset(pbi->mbi.Coeffs[bp]+24, 0,4*sizeof(Q_LIST_ENTRY));
	}
	else 
	{
	    memset(pbi->mbi.Coeffs[bp], 0,64*sizeof(Q_LIST_ENTRY));
	}

}




/****************************************************************************
 * 
 *  ROUTINE       :     DecodeMacroBlock
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Decodes A MacroBlock
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void DecodeMacroBlock
(
	PB_INSTANCE *pbi,
	UINT32 MBrow,
	UINT32 MBcol
)
{
	UINT32 MBPointer;
	INT32  NextBlock;

    //***********************************************************************
    // Copy the existing structures into what we have now I'll fix this next.

	// dumb way to encode the interlaced decision but it works!!!

	if(pbi->Configuration.Interlaced)
	{
		UINT8 prob = pbi->probInterlaced;
		// super simple context adjustment
		if(MBcol>2)
		{
			// adjust the probability per the last one we did 
			if(pbi->mbi.Interlaced)
				prob=prob-(prob>>1);
			else 
				prob=prob+((256-prob)>>1);
		}
		pbi->mbi.Interlaced = nDecodeBool(	&pbi->br, prob);
	}
	else
		pbi->mbi.Interlaced = 0;

	if(pbi->FrameType == BASE_FRAME )
	{
		pbi->mbi.Mode = CODE_INTRA;
	}
	else
	{
		decodeModeAndMotionVector(pbi, MBrow, MBcol );
	}

	if(pbi->mbi.Interlaced == 0)
	{
		NextBlock = 8;
		pbi->mbi.CurrentReconStride = pbi->Configuration.YStride ;
	}
	else
	{
		NextBlock = 1;
		pbi->mbi.CurrentReconStride = pbi->Configuration.YStride * 2;
	}

	// y plane values
	pbi->mbi.FrameReconStride = pbi->Configuration.YStride;
	pbi->mbi.MvShift = 1;
	pbi->mbi.MvModMask = 1;
	pbi->mbi.LastDc = pbi->fc.LastDcY;
	pbi->mbi.Plane = 0;
	pbi->mbi.SourceY = MBrow * 16;
	pbi->mbi.SourceX = MBcol * 16;
	MBPointer = pbi->ReconYDataOffset 
		+ pbi->mbi.SourceY * pbi->Configuration.YStride
		+ pbi->mbi.SourceX;
	
	// Block 0 
	pbi->mbi.Recon = MBPointer;
	pbi->mbi.Above = &pbi->fc.AboveY[MBcol*2];
	pbi->mbi.Left  = &pbi->fc.LeftY[0];
	DecodeBlock(pbi, MBrow, MBcol, (BLOCK_POSITION)0);
	
	// Block 1 
	pbi->mbi.Recon += 8;
	pbi->mbi.Above = &pbi->fc.AboveY[MBcol*2+1];
	pbi->mbi.Left  = &pbi->fc.LeftY[0];
	pbi->mbi.SourceX += 8;
	DecodeBlock(pbi, MBrow, MBcol, (BLOCK_POSITION)1);
	
	// Block 2 
	pbi->mbi.Recon = MBPointer + NextBlock * pbi->Configuration.YStride;
	pbi->mbi.Above = &pbi->fc.AboveY[MBcol*2];
	pbi->mbi.Left  = &pbi->fc.LeftY[1];
	pbi->mbi.SourceX -= 8;
	pbi->mbi.SourceY += NextBlock;
	DecodeBlock(pbi, MBrow, MBcol, 2);
	
	// Block 3
	pbi->mbi.Recon += 8;
	pbi->mbi.Above = &pbi->fc.AboveY[MBcol*2+1];
	pbi->mbi.Left  = &pbi->fc.LeftY[1];
	pbi->mbi.SourceX += 8;
	DecodeBlock(pbi, MBrow, MBcol, (BLOCK_POSITION)3);
	
	// uv plane values
	pbi->mbi.FrameReconStride = pbi->Configuration.UVStride;
	pbi->mbi.CurrentReconStride = pbi->Configuration.UVStride;
	pbi->mbi.SourceY = MBrow * 8;
	pbi->mbi.SourceX = MBcol * 8;
	pbi->mbi.MvShift = 2;
	pbi->mbi.MvModMask = 3;
	
	// Block 4
	pbi->mbi.Recon = pbi->ReconUDataOffset + pbi->mbi.SourceY * pbi->mbi.CurrentReconStride + pbi->mbi.SourceX;
	pbi->mbi.Above = &pbi->fc.AboveU[MBcol];
	pbi->mbi.Left = &pbi->fc.LeftU;
	pbi->mbi.LastDc = pbi->fc.LastDcU;
	pbi->mbi.Plane = 1;
	DecodeBlock(pbi, MBrow, MBcol, (BLOCK_POSITION)4);
	
	// Block 5
	pbi->mbi.Above = &pbi->fc.AboveV[MBcol];
	pbi->mbi.Left = &pbi->fc.LeftV;
	pbi->mbi.Recon = pbi->ReconVDataOffset + pbi->mbi.SourceY * pbi->mbi.CurrentReconStride + pbi->mbi.SourceX;
	pbi->mbi.LastDc = pbi->fc.LastDcV;
	pbi->mbi.Plane = 2;
	DecodeBlock(pbi, MBrow, MBcol, (BLOCK_POSITION)5);
	
}



/****************************************************************************
 * 
 *  ROUTINE       :     DecodeFrame
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Decodes MacroBlocks of a Frame
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void DecodeFrameMbs
( 
	PB_INSTANCE *pbi
)
{
	UINT32 MBrow, MBcol;
	UINT32 MBRows = pbi->MBRows; 
	UINT32 MBCols = pbi->MBCols;
	UINT32 MB = 0;

	if(pbi->FrameType != BASE_FRAME )
	{
		DecodeModeProbs(pbi);
		ConfigureMvEntropyDecoder( pbi, pbi->FrameType );
        pbi->LastMode = CODE_INTER_NO_MV;
	}
	else
	{
		memcpy ( pbi->probXmitted,BaselineXmittedProbs,sizeof(pbi->probXmitted));
		// For now these are just 128
		memset ( pbi->MvSignProbs, 128, sizeof(pbi->MvSignProbs) );
		memset ( pbi->MvZeroProbs, 128, sizeof(pbi->MvZeroProbs) );
		memset ( pbi->MvHalfPixelProbs, DEFAULT_HALF_PIXEL_PROB, sizeof(pbi->MvHalfPixelProbs) );
		memset ( pbi->MvLowBitProbs, 128, sizeof(pbi->MvLowBitProbs) );
		memset ( pbi->MvSizeProbs, 128, sizeof(pbi->MvSizeProbs) );
		memset ( pbi->MBModeProb,128,sizeof(pbi->MBModeProb));
		memset ( pbi->BModeProb,128,sizeof(pbi->MBModeProb));
		memset ( pbi->predictionMode,1,sizeof(char)*pbi->MacroBlocks );
	}

	ConfigureEntropyDecoder( pbi, pbi->FrameType ); 

	if(pbi->Configuration.Interlaced == 1)
		pbi->probInterlaced = ((UINT8)VP5_bitread( &pbi->br,   8 ));  

	// since we are on a new frame reset the above contexts 
	ResetAboveContext(pbi);

	// Default clear data area down to 0s
    memset(pbi->mbi.Coeffs, 0,6*72*sizeof(Q_LIST_ENTRY));

	// for each row of macroblocks 
	for ( MBrow=2; MBrow<MBRows-2; MBrow++ )
	{

		ResetLeftContext(pbi);

		// for each macroblock within a row of macroblocks
		for ( MBcol=2; MBcol<MBCols-2; MBcol++,MB++ )
		{

			// Decode the macroblock
			DecodeMacroBlock(pbi,MBrow,MBcol);
            
		} // mb col


	} // mbrow

//	printmodes(pbi);
}

