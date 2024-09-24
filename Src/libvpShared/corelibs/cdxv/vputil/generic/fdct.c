/****************************************************************************
*
*   Module Title :     fdct.c
*
*   Description  :     Fast 8x8 DCT C-Implementation.
*
****************************************************************************/

/****************************************************************************
*  Header Files
****************************************************************************/
#include "dct.h"

/****************************************************************************
*  Macros
****************************************************************************/
#define SIGNBITDUPPED(X) ( (signed )((X & 0x80000000)) >> 31 )
#define DOROUND(X) X = ( (SIGNBITDUPPED(X) & (0xffff)) + X ); 

/****************************************************************************
*  Module statics
****************************************************************************/
static INT32 xC1S7 = 64277;
static INT32 xC2S6 = 60547;
static INT32 xC3S5 = 54491;
static INT32 xC4S4 = 46341;
static INT32 xC5S3 = 36410;
static INT32 xC6S2 = 25080;
static INT32 xC7S1 = 12785;

/****************************************************************************
 * 
 *  ROUTINE       : fdct_short_C_orig
 *
 *  INPUTS        : INT16 *InputData  : 16-bit input data.
 *
 *  OUTPUTS       : INT16 *OutputData : 16-bit transform coefficients.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Performs an 8x8 2-D fast DCT.
 *
 *                  The algorithm used is derived from the flowgraph for
 *                  the Vetterli and Ligtenberg fast 1-D dct given in the
 *                  JPEG reference book by Pennebaker and Mitchell.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void fdct_short_C_orig ( INT16 *InputData, INT16 *OutputData )
{
	int    loop;
	INT32  is07, is12, is34, is56;
	INT32  is0734, is1256;
	INT32  id07, id12, id34, id56; 
	INT32  irot_input_x, irot_input_y;
	INT32  icommon_product1;            // Re-used product  (c4s4 * (s12 - s56)). 
	INT32  icommon_product2;            // Re-used product  (c4s4 * (d12 + d56)).
	INT32  temp1, temp2;	            // intermediate variable for computation
	INT32  InterData[64];

    INT32 *ip = InterData;
	INT16 *op = OutputData;
	
    for ( loop=0; loop<8; loop++ )
	{
		// Pre calculate some common sums and differences.
		is07 = InputData[0] + InputData[7];
		is12 = InputData[1] + InputData[2];
		is34 = InputData[3] + InputData[4];
		is56 = InputData[5] + InputData[6];

		id07 = InputData[0] - InputData[7];
		id12 = InputData[1] - InputData[2];
		id34 = InputData[3] - InputData[4];
		id56 = InputData[5] - InputData[6];
	
		is0734 = is07 + is34;
		is1256 = is12 + is56;
		
		// Pre-Calculate some common product terms.
		icommon_product1 = xC4S4*(is12 - is56); 
		DOROUND ( icommon_product1 )
		icommon_product1 >>= 16;
		
		icommon_product2 = xC4S4*(id12 + id56);
		DOROUND ( icommon_product2 )
		icommon_product2 >>= 16;

		ip[0] = (xC4S4*(is0734 + is1256));
		DOROUND ( ip[0] );
		ip[0] >>= 16;

		ip[4] = (xC4S4*(is0734 - is1256));
		DOROUND ( ip[4] );
		ip[4] >>= 16;

		// Define inputs to rotation for outputs 2 and 6 
		irot_input_x = id12 - id56;
		irot_input_y = is07 - is34;

		// Apply rotation for outputs 2 and 6. 
		temp1 = xC6S2*irot_input_x;
		DOROUND ( temp1 );
		temp1 >>= 16;
		temp2 = xC2S6*irot_input_y;
		DOROUND ( temp2 );
		temp2 >>= 16;
		ip[2] = temp1 + temp2;

		temp1 = xC6S2*irot_input_y;
		DOROUND ( temp1 );
		temp1 >>= 16;
		temp2 = xC2S6*irot_input_x;
		DOROUND ( temp2 );
		temp2 >>= 16;
		ip[6] = temp1 -temp2;

		// Define inputs to rotation for outputs 1 and 7 
		irot_input_x = icommon_product1 + id07;
		irot_input_y = -( id34 + icommon_product2 );

		// Apply rotation for outputs 1 and 7. 
		temp1 = xC1S7*irot_input_x;
		DOROUND ( temp1 );
		temp1 >>= 16;
		temp2 = xC7S1*irot_input_y;
		DOROUND ( temp2 );
		temp2 >>= 16;
		ip[1] = temp1 - temp2;

		temp1 = xC7S1*irot_input_x;
		DOROUND ( temp1 );
		temp1 >>= 16;
		temp2 = xC1S7*irot_input_y;
		DOROUND ( temp2 );
		temp2 >>= 16;
		ip[7] = temp1 + temp2;
		
		// Define inputs to rotation for outputs 3 and 5 
		irot_input_x = id07 - icommon_product1;
		irot_input_y = id34 - icommon_product2;

		// Apply rotation for outputs 3 and 5. 
		temp1 = xC3S5 * irot_input_x;
		DOROUND ( temp1 );
		temp1 >>= 16;
		temp2 = xC5S3*irot_input_y;
		DOROUND ( temp2 );
		temp2 >>= 16;
		ip[3] = temp1 - temp2;

		temp1 = xC5S3*irot_input_x;
		DOROUND ( temp1 );
		temp1 >>= 16;
		temp2 = xC3S5*irot_input_y;
		DOROUND ( temp2 );
		temp2 >>= 16;
		ip[5] = temp1 + temp2;
		
		// Increment data pointer for next row. 
		InputData += 8;
		ip += 8;		// advance pointer to next row 
	}

	//	Performed DCT on rows, now transform the columns	
	ip = InterData;
	for ( loop=0; loop<8; loop++ )
	{
		// Pre calculate some common sums and differences. 
		is07 = ip[0 * 8] + ip[7 * 8];
		is12 = ip[1 * 8] + ip[2 * 8];
		is34 = ip[3 * 8] + ip[4 * 8];
		is56 = ip[5 * 8] + ip[6 * 8];

		id07 = ip[0 * 8] - ip[7 * 8];
		id12 = ip[1 * 8] - ip[2 * 8];
		id34 = ip[3 * 8] - ip[4 * 8];
		id56 = ip[5 * 8] - ip[6 * 8];
	
		is0734 = is07 + is34;
		is1256 = is12 + is56;
		
		// Pre-Calculate some common product terms.
		icommon_product1 = xC4S4*(is12 - is56); 
		icommon_product2 = xC4S4*(id12 + id56);
		DOROUND ( icommon_product1 )
		DOROUND ( icommon_product2 )
		icommon_product1 >>= 16;
		icommon_product2 >>= 16;

		temp1 = xC4S4*(is0734 + is1256);
		temp2 = xC4S4*(is0734 - is1256);
		DOROUND ( temp1 );
		DOROUND ( temp2 );
		temp1 >>= 16;
		temp2 >>= 16;
		op[0*8] = (INT16)temp1;
		op[4*8] = (INT16)temp2;

		// Define inputs to rotation for outputs 2 and 6 
		irot_input_x = id12 - id56;
		irot_input_y = is07 - is34;

		// Apply rotation for outputs 2 and 6. 
		temp1 = xC6S2*irot_input_x;
		DOROUND ( temp1 );
		temp1 >>= 16;
		temp2 = xC2S6*irot_input_y;
		DOROUND ( temp2 );
		temp2 >>= 16;
		op[2*8] = (INT16)(temp1 + temp2);

		temp1 = xC6S2*irot_input_y;
		DOROUND ( temp1 );
		temp1 >>= 16;
		temp2 = xC2S6*irot_input_x;
		DOROUND ( temp2 );
		temp2 >>= 16;
		op[6*8] = (INT16)(temp1 -temp2);

		// Define inputs to rotation for outputs 1 and 7 
		irot_input_x = icommon_product1 + id07;
		irot_input_y = -( id34 + icommon_product2 );

		// Apply rotation for outputs 1 and 7. 
		temp1 = xC1S7*irot_input_x;
		DOROUND ( temp1 );
		temp1 >>= 16;
		temp2 = xC7S1*irot_input_y;
		DOROUND ( temp2 );
		temp2 >>= 16;
		op[1*8] = (INT16) (temp1 - temp2);

		temp1 = xC7S1*irot_input_x;
		DOROUND ( temp1 );
		temp1 >>= 16;
		temp2 = xC1S7*irot_input_y;
		DOROUND ( temp2 );
		temp2 >>= 16;
		op[7*8] = (INT16)(temp1 + temp2);

		// Define inputs to rotation for outputs 3 and 5 
		irot_input_x = id07 - icommon_product1;
		irot_input_y = id34 - icommon_product2;

		// Apply rotation for outputs 3 and 5. 
		temp1 = xC3S5*irot_input_x;
		DOROUND ( temp1 );
		temp1 >>= 16;
		temp2 = xC5S3*irot_input_y;
		DOROUND ( temp2 );
		temp2 >>= 16;
		op[3*8] = (INT16)(temp1 - temp2);

		temp1 = xC5S3*irot_input_x;
		DOROUND ( temp1 );
		temp1 >>= 16;
		temp2 = xC3S5*irot_input_y;
		DOROUND ( temp2 );
		temp2 >>= 16;
		op[5*8] = (INT16) (temp1 + temp2);

		// Increment data pointer for next column. 
		ip ++;
		op ++;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : fdct_short_C
 *
 *  INPUTS        : INT16 *InputData  : 16-bit input data.
 *
 *  OUTPUTS       : INT16 *OutputData : 16-bit transform coefficients.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Performs an 8x8 2-D fast DCT.
 *
 *                  The function to up the precision of FDCT by number of bits 
 *                  defined by FDCT_PRECISION_BITS.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void fdct_short_C ( INT16 *DCTDataBuffer, INT16 *DCT_codes )
{

    INT32 i;

	// Increase precision on input to fdct
	for ( i = 0; i < 64; i++ )
		DCTDataBuffer[i] = DCTDataBuffer[i] << FDCT_PRECISION_BITS;

	// Transform the error signal using the forward DCT to get set of transform coefficients
	fdct_short_C_orig ( DCTDataBuffer, DCT_codes );

	// Strip off the extra bits from the DCT output.
	// This should ultimately be merged into the quantize process but there are also
	// implications for DC prediction that would then need to be sorted
	for ( i = 0; i < 64; i++ )
	{	
		// signed shift modified so behaves like "/" (truncates towards 0 for + and -)
		if ( DCT_codes[i]  >= 0 )
			DCT_codes[i] = (DCT_codes[i]) >> FDCT_PRECISION_BITS;
		else
			DCT_codes[i] = (DCT_codes[i] + FDCT_PRECISION_NEG_ADJ) >> FDCT_PRECISION_BITS;
	}

}