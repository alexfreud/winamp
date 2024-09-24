/****************************************************************************
*
*   Module Title :     quantindexmmx.c
*
*   Description  :     
*
****************************************************************************/						
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "codec_common.h"
#include "quantize.h"
#include "math.h"
/****************************************************************************
*  Macros
****************************************************************************/ 
#define MIN16 ((1<<16)-1)
       
/****************************************************************************
*  Module Statics
****************************************************************************/
static UINT32 dequant_index[64] = 
{	
     0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};
 
static UINT32 dequant_indexMMX[64] = 
{
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54, 
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

/* Used to unravel the coeffs in the proper order required */
/* by MMX_idct (see mmxidct.cxx)                           */
static UINT32 transIndexMMX[64] = 
{
     0,  8,  1,  2,    9, 16, 24, 17,
    10,  3, 32, 11,   18, 25,  4, 12,
     5, 26, 19, 40,   33, 34, 41, 48,
    27,  6, 13, 20,   28, 21, 14,  7,

    56, 49, 42, 35,   43, 50, 57, 36, 
    15, 22, 29, 30,   23, 44, 37, 58,
    51, 59, 38, 45,   52, 31, 60, 53,
    46, 39, 47, 54,   61, 62, 55, 63
};

static UINT32 transIndexWMT[64] = 
{	
	 0,  8,  1,  2,   9, 16, 24, 17,
	10,  3,  4, 11,	 18, 25, 32, 40,
    33, 26, 19, 12,   5,  6, 13, 20,
    27, 34, 41, 48,  56, 49, 42, 35,
    28, 21, 14,  7,  15, 22, 29, 36, 
    43, 50, 57, 58,  51, 44, 37, 30,
    23, 31, 38, 45,  52, 59, 60, 53,
    46, 39, 47, 54,  61, 62, 55, 63
};

/****************************************************************************
 * 
 *  ROUTINE       : VP6_BuildQuantIndex_ForMMX
 *
 *  INPUTS        : QUANTIZER *pbi : Pointer to quantizer instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Builds the quant_index table in a transposed order.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void VP6_BuildQuantIndex_ForMMX ( QUANTIZER *pbi )
{
    INT32 i, j;

    pbi->transIndex = transIndexMMX;

    // invert the dequant index into the quant index
	for ( i=0; i<BLOCK_SIZE; i++ )
	{	
        j = transIndexMMX[dequant_indexMMX[i]];
		pbi->quant_index[j] = i;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : BuildQuantIndex_ForWMT
 *
 *  INPUTS        : QUANTIZER *pbi : Pointer to quantizer instance.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Builds the quant_index table in a transposed order.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void VP6_BuildQuantIndex_ForWMT ( QUANTIZER *pbi )
{
    INT32 i, j;

    pbi->transIndex = transIndexWMT;

    // invert the dequant index into the quant index
	for ( i=0; i<BLOCK_SIZE; i++ )
	{	
        j = transIndexWMT[dequant_indexMMX[i]];
		pbi->quant_index[j] = i;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : VP6_quantize_wmt
 *
 *  INPUTS        : QUANTIZER *pbi               : Pointer to quantizer instance.
 *                  INT16 *DCT_block             : Pointer to block of DCT coeffs.
 *                  UINT8 bp                     : Position of blockin MB.
 *
 *  OUTPUTS       : Q_LIST_ENTRY *quantized_list : Pointer to block of quantized DCT coeffs.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Quantizes an 8x8 blockof DCT coefficients.  
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void VP6_quantize_wmt
( 
    QUANTIZER *qi, 
    INT16 *DCT_block, 
    Q_LIST_ENTRY *quantized_list, 
    UINT8 bp 
)
{
    UINT32  i, j;
	INT32   temp;
    INT32 ThisDC;
    
    UINT32	ColourPlane   = VP6_QTableSelect[bp]; 
    INT32 *QuantRoundPtr  = qi->QuantRound[ColourPlane];
    INT32 QuantCoeffsDC   = qi->QuantCoeffs[ColourPlane][0];
    INT32 *ZBinPtr        = qi->ZeroBinSize[ColourPlane];
	INT16 *round          = &qi->round[0];
	INT16 *mult           = &qi->mult[0];
    INT32  Zrl            = 0;
    INT32 * ZrlCorrection = qi->ZlrZbinCorrections[ColourPlane];
__declspec(align(16)) unsigned short xyw[64];  
  

	// this quantizer stores its results back in the source!!
	__asm
	{
		// setup and collect registers
		mov			esi, DCT_block
		xor         ecx, ecx        // index ptr
		mov			edi, round
		movdqu      xmm2, [edi]		// get the round values
		mov         edi, mult
		movdqu      xmm3, [edi]     // get the quantizer values
        lea         edi,  xyw      
        
        mov         eax,  quantized_list
        pxor        xmm7, xmm7
	
		// 8 coefficients at a time loop 
next8:
		movdqa      xmm0, [esi+ecx]	// get source values
		movdqa      xmm1, xmm0		// sign bits of the abs values 
		psraw		xmm1, 15		// negative all 1's postive all 0's

        // get the absolute value of the input values
		pxor        xmm0, xmm1      // one's complement of negatives 
		psubw       xmm0, xmm1      // xmm0 = abs coeffs

		// calculate & round quantizer
		paddw		xmm0, xmm2      // Coeff + Quant Round
        pmulhuw     xmm0, xmm3      // *QuantCoeffs >> 16


		// get back the sign bit
        pxor        xmm0, xmm1      // ones complement of negatives
        psubw       xmm0, xmm1      // negatives are back as negative

		// output the results
		movdqa      [edi+ecx], xmm0 
        movdqa      [eax+ecx], xmm7

		// loop back to the next set
		add         ecx, 16			
		cmp			ecx, 128
		jl          next8
	}


	// DC quantization 
    ThisDC = DCT_block[0];
	if ( ThisDC >= ZBinPtr[0] )
	{
		temp = QuantCoeffsDC * ( ThisDC + QuantRoundPtr[0] );
		quantized_list[0] = (Q_LIST_ENTRY) (temp>>16);
	}
    else if ( ThisDC <= -ZBinPtr[0] )
	{
		temp = QuantCoeffsDC * ( ThisDC - QuantRoundPtr[0] ) + MIN16;
		quantized_list[0] = (Q_LIST_ENTRY) (temp>>16);
	}
	else
    {    
		Zrl++;
    }


	// zig-zagify 
    for ( i=1; i<64; i++ )
    {
		INT32 x;
        INT32 y;
        // Zig Zag order
		j = dequant_index[i];
        x = xyw[j];
        y= abs( DCT_block[j]);
        if(y<ZBinPtr[j] + ZrlCorrection[Zrl]) 
        {
            Zrl ++;
        }
        else
        {
            Zrl = 0;
            quantized_list[i] = x;
        }
    }

}


/****************************************************************************
 * 
 *  ROUTINE       : VP6_quantize_mmx
 *
 *  INPUTS        : QUANTIZER *pbi               : Pointer to quantizer instance.
 *                  INT16 *DCT_block             : Pointer to block of DCT coeffs.
 *                  UINT8 bp                     : Position of blockin MB.
 *
 *  OUTPUTS       : Q_LIST_ENTRY *quantized_list : Pointer to block of quantized DCT coeffs.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Quantizes an 8x8 blockof DCT coefficients.  
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void VP6_quantize_mmx
( 
    QUANTIZER *qi, 
    INT16 *DCT_block, 
    Q_LIST_ENTRY *quantized_list, 
    UINT8 bp 
)
{
    UINT32  i, j;
	INT32   temp;
    INT32 ThisDC;
    
    UINT32	ColourPlane   = VP6_QTableSelect[bp]; 
    INT32 *QuantRoundPtr  = qi->QuantRound[ColourPlane];
    INT32 QuantCoeffsDC   = qi->QuantCoeffs[ColourPlane][0];
    INT32 *ZBinPtr        = qi->ZeroBinSize[ColourPlane];
	INT16 *round          = &qi->round[0];
	INT16 *mult           = &qi->mult[0];
    INT32  Zrl            = 0;
    INT32 * ZrlCorrection = qi->ZlrZbinCorrections[ColourPlane];
__declspec(align(16)) unsigned short xyw[64];  
  

	// this quantizer stores its results back in the source!!
	__asm
	{
		// setup and collect registers
		mov			esi, DCT_block
		xor         ecx, ecx        // index ptr
		mov			edi, round
		movq        mm2, [edi]		// get the round values
		mov         edi,  mult
		movq        mm3, [edi]     // get the quantizer values
        lea         edi,  xyw      
        
        mov         eax,  quantized_list
        pxor        mm7, mm7
	
		// 8 coefficients at a time loop 
next8:
		movq        mm0, [esi+ecx]	// get source values
		movq        mm1, mm0		// sign bits of the abs values 
		psraw		mm1, 15		// negative all 1's postive all 0's

        // get the absolute value of the input values
		pxor        mm0, mm1      // one's complement of negatives 
		psubw       mm0, mm1      // xmm0 = abs coeffs

		// calculate & round quantizer
		paddw		mm0, mm2      // Coeff + Quant Round
        pmulhuw     mm0, mm3      // *QuantCoeffs >> 16


		// get back the sign bit
        pxor        mm0, mm1      // ones complement of negatives
        psubw       mm0, mm1      // negatives are back as negative

		// output the results
		movq        [edi+ecx], mm0 
        movq        [eax+ecx], mm7

		// loop back to the next set
		add         ecx, 8			
		cmp			ecx, 128
		jl          next8
	}


	// DC quantization 
    ThisDC = DCT_block[0];
	if ( ThisDC >= ZBinPtr[0] )
	{
		temp = QuantCoeffsDC * ( ThisDC + QuantRoundPtr[0] );
		quantized_list[0] = (Q_LIST_ENTRY) (temp>>16);
	}
    else if ( ThisDC <= -ZBinPtr[0] )
	{
		temp = QuantCoeffsDC * ( ThisDC - QuantRoundPtr[0] ) + MIN16;
		quantized_list[0] = (Q_LIST_ENTRY) (temp>>16);
	}
	else
    {    
		Zrl++;
    }


	// zig-zagify 
    for ( i=1; i<64; i++ )
    {
		INT32 x;
        INT32 y;
        // Zig Zag order
		j = dequant_index[i];
        x = xyw[j];
        y= abs( DCT_block[j]);
        if(y<ZBinPtr[j] + ZrlCorrection[Zrl]) 
        {
            Zrl ++;
        }
        else
        {
            Zrl = 0;
            quantized_list[i] = x;
        }
    }
}
