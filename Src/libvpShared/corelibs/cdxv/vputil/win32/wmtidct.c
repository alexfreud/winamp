/****************************************************************************
 *
 *   Module Title :     wmtidct.c
 *
 *   Description  :     IDct functions optimized specifically for willamette 
 *						processor
 *					
 *	 Special Notes:		
 *
 *   AUTHOR       :     YaoWu Xu
 *
 ***************************************************************************** 
 *   Revision History
 *		
 *   1.02 YWX   07-dec-00 Removed code not in use and added push pop ebx
 *	 1.01 YWX	29/06/00  Added Wmt_IDCT_Dx and Wmt_IDCT10_Dx
 *   1.00 YWX	31/05/00  Configuration baseline
 *
 *****************************************************************************
 */


/*******************************************************************************
 * Module Constants
 *******************************************************************************
 */
	

/* constants for rounding */
__declspec(align(32)) static unsigned int Eight[]=
{ 
	0x00080008, 
	0x00080008,
	0x00080008, 
	0x00080008 
}; 
/* cosine constants, cosine ( i * pi / 8 ) */
__declspec(align(32)) static unsigned short WmtIdctConst[7 * 8]=
{
	64277,64277,64277,64277,64277,64277,64277,64277, 
	60547,60547,60547,60547,60547,60547,60547,60547, 
	54491,54491,54491,54491,54491,54491,54491,54491, 
	46341,46341,46341,46341,46341,46341,46341,46341, 
	36410,36410,36410,36410,36410,36410,36410,36410, 
	25080,25080,25080,25080,25080,25080,25080,25080, 
	12785,12785,12785,12785,12785,12785,12785,12785
};
/* Mask constant for dequantization */
__declspec(align(32)) static unsigned short WmtDequantConst[]=
{
	0,65535,65535,0,0,0,0,0,	//0x0000 0000 0000 0000 0000 FFFF FFFF 0000
	0,0,0,0,65535,65535,0,0,	//0x0000 0000 FFFF FFFF 0000 0000 0000 0000
	65535,65535,65535,0,0,0,0,0,//0x0000 0000 0000 0000 0000 FFFF FFFF FFFF
	0,0,0,65535,0,0,0,0,		//0x0000 0000 0000 0000 FFFF 0000 0000 0000
	0,0,0,65535,65535,0,0,0,	//0x0000 0000 0000 FFFF FFFF 0000 0000 0000
	65535,0,0,0,0,65535,0,0,	//0x0000 0000 FFFF 0000 0000 0000 0000 FFFF
	0,0,65535,65535, 0,0,0,0	//0x0000 0000 0000 0000 FFFF FFFF 0000 0000
};


/*******************************************************************************
 * Forward Reference
 *******************************************************************************
 */

/********************************************************************************
 *	Description of Inverse DCT algorithm.
 ********************************************************************************
 *

   Dequantization multiplies user's 16-bit signed indices (range -512 to +511)
   by unsigned 16-bit quantization table entries.
   These table entries are upscaled by 4, max is 30 * 128 * 4 < 2^14.
   Result is scaled signed DCT coefficients (abs value < 2^15).

   In the data stream, the coefficients are sent in order of increasing
   total (horizontal + vertical) frequency.  The exact picture is as follows:

	00 01 05 06  16 17 33 34
	02 04 07 15  20 32 35 52
	03 10 14 21  31 36 51 53
	11 13 22 30  37 50 54 65

	12 23 27 40  47 55 64 66
	24 26 41 46	 56 63 67 74
	25 42 45 57  62 70 73 75
	43 44 60 61  71 72 76 77

   Here the position in the matrix corresponds to the (horiz,vert)
   freqency indices and the octal entry in the matrix is the position
   of the coefficient in the data stream.  Thus the coefficients are sent
   in sort of a diagonal "snake".

   The dequantization stage "uncurls the snake" and stores the expanded
   coefficients in more convenient positions.  These are not exactly the
   natural positions given above but take into account our implementation
   of the idct, which basically requires two one-dimensional idcts and
   two transposes.


   Transposing the 8x8 matrix above gives

	00 02 03 11  12 24 25 43  
	01 04 10 13  23 26 42 44  
	05 07 14 22  27 41 45 60  
	06 15 21 30  40 46 57 61  

	16 20 31 37  47 56 62 71
	17 32 36 50  55 63 70 72
	33 35 51 54  64 67 73 76
	34 52 53 65  66 74 75 77


   The idct itself is more interesting.  Since the two-dimensional dct
   basis functions are products of the one-dimesional dct basis functions,
   we can compute an inverse (or forward) dct via two 1-D transforms,
   on rows then on columns.  To exploit MMX parallelism, we actually do
   both operations on columns, interposing a (partial) transpose between
   the two 1-D transforms, the first transpose being done by the expansion
   described above.

   The 8-sample one-dimensional DCT is a standard orthogonal expansion using
   the (unnormalized) basis functions

	b[k]( i) = cos( pi * k * (2i + 1) / 16);

   here k = 0 ... 7 is the frequency and i = 0 ... 7 is the spatial coordinate.
   To normalize, b[0] should be multiplied by 1/sqrt( 8) and the other b[k]
   should be multiplied by 1/2.

   The 8x8 two-dimensional DCT is just the product of one-dimensional DCTs
   in each direction.  The (unnormalized) basis functions are

	B[k,l]( i, j) = b[k]( i) * b[l]( j);

   this time k and l are the horizontal and vertical frequencies,
   i and j are the horizontal and vertical spatial coordinates;
   all indices vary from 0 ... 7 (as above)
   and there are now 4 cases of normalization.
  
   Our 1-D idct expansion uses constants C1 ... C7 given by

   	(*)  Ck = C(-k) = cos( pi * k/16) = S(8-k) = -S(k-8) = sin( pi * (8-k)/16) 

   and the following 1-D algorithm transforming I0 ... I7  to  R0 ... R7 :
  
   A = (C1 * I1) + (C7 * I7)		B = (C7 * I1) - (C1 * I7)
   C = (C3 * I3) + (C5 * I5)		D = (C3 * I5) - (C5 * I3)
   A. = C4 * (A - C)				B. = C4 * (B - D)
   C. = A + C						D. = B + D
   
   E = C4 * (I0 + I4)				F = C4 * (I0 - I4)
   G = (C2 * I2) + (C6 * I6)		H = (C6 * I2) - (C2 * I6)
   E. = E - G
   G. = E + G
   
   A.. = F + A.					B.. = B. - H
   F.  = F - A. 				H.  = B. + H
   
   R0 = G. + C.	R1 = A.. + H.	R3 = E. + D.	R5 = F. + B..
   R7 = G. - C.	R2 = A.. - H.	R4 = E. - D.	R6 = F. - B..

   This algorithm was also used by Paul Wilkins in his C implementation;
   it is due to Vetterli and Lightenberg and may be found in the JPEG
   reference book by Pennebaker and Mitchell.

   Correctness of the algorithm follows from (*) together with the
   addition formulas for sine and cosine:

	cos( A + B) = cos( A) * cos( B)  -  sin( A) * sin( B)
	sin( A + B) = sin( A) * cos( B)  +  cos( A) * sin( B)

   Note that this implementation absorbs the difference in normalization
   between the 0th and higher frequencies, although the results produced
   are actually twice as big as they should be.  Since we do this for each
   dimension, the 2-D idct results are 4x the desired results.  Finally,
   taking into account that the dequantization multiplies by 4 as well,
   our actual results are 16x too big.  We fix this by shifting the final
   results right by 4 bits.

   High precision version approximates C1 ... C7 to 16 bits.
   Since there is not multiply taking one unsigned and one signed,
   we have to use the signed multiplay, therefore C1 ... C5 appear to be
   negative and multiplies involving them must be adjusted to compensate
   for this.  C6 and C7 do not require this adjustment since
   they are < 1/2 and are correctly treated as positive numbers.

   Following macro does Eight 8-sample one-dimensional idcts in parallel.
   This is actually not such a difficult program to write once you
   make a couple of observations (I of course was unable to make these
   observations until I'd half-written a couple of other versions).

	1. Everything is easy once you are done with the multiplies.
	   This is because, given X and Y in registers, one may easily
	   calculate X+Y and X-Y using just those 2 registers.

	2. You always need at least 2 extra registers to calculate products,
	   so storing 2 temporaries is inevitable.  C. and D. seem to be
	   the best candidates.   

	3. The products should be calculated in decreasing order of complexity
	   (which translates into register pressure).  Since C1 ... C5 require
	   adjustment (and C6, C7 do not), we begin by calculating C and D.

********************************************************************************/


/**************************************************************************************
 *
 *		Macro:			Wmt_Column_IDCT
 *		
 *		Description:	The Macro does 1-D IDct on 8 columns. 
 *
 *		Input:			None
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	None
 *
 *		Error:			None
 *
 ***************************************************************************************
 */

/*	
	The major difference between Willamette processor and other IA32 processors is that 
	all of the simd integer instructions now support the 128 bit xmm registers instead 
	of 64 bit mmx registers. By using these instructions, we can do 8 1-D coloumn idcts 
	that takes shorts as input and outputs shorts at once

*/

#define Wmt_Column_IDCT __asm {		\
	\
	__asm	movdqa	xmm2, I(3)		/* xmm2 = i3 */				\
	__asm	movdqa	xmm6, C(3)		/* xmm6 = c3 */				\
	\
	__asm	movdqa	xmm4, xmm2		/* xmm4 = i3 */				\
	__asm	movdqa	xmm7, I(5)		/* xmm7 = i5 */				\
	\
	__asm	pmulhw xmm4, xmm6		/* xmm4 = c3 * i3 - i3 */	\
	__asm	movdqa  xmm1, C(5)		/* xmm1 = c5 */				\
	\
	__asm	pmulhw xmm6, xmm7		/* xmm6 = c3 * i5 - i5 */	\
	__asm	movdqa	xmm5, xmm1		/* xmm5 = c5 */				\
	\
	__asm	pmulhw	xmm1, xmm2		/* xmm1 = c5 * i3 - i3 */	\
	__asm	movdqa  xmm3, I(1)		/* xmm3 = i1 */				\
	\
	__asm	pmulhw	xmm5, xmm7		/* xmm5 = c5 * i5 - i5 */	\
	__asm	movdqa	xmm0, C(1)		/* xmm0 = c1 */				\
	\
	/* all registers are in use */								\
	\
	__asm	paddw   xmm4, xmm2		/* xmm4 = c3 * i3 */		\
	__asm	paddw	xmm6, xmm7		/* xmm6 = c3 * i5 */		\
	\
	__asm	paddw	xmm2, xmm1		/* xmm2 = c5 * i3 */		\
	__asm	movdqa	xmm1, I(7)		/* xmm1 = i7 */				\
	\
	__asm	paddw	xmm7, xmm5		/* xmm7 = c5 * i5 */		\
	__asm	movdqa	xmm5, xmm0		/* xmm5 = c1 */				\
	\
	__asm	pmulhw	xmm0, xmm3		/* xmm0 = c1 * i1 - i1 */				\
	__asm	paddsw	xmm4, xmm7		/* xmm4 = c3 * i3 + c5 * i5 = C */		\
	\
	__asm	pmulhw	xmm5, xmm1		/* xmm5 = c1 * i7 - i7 */				\
	__asm   movdqa	xmm7, C(7)		/* xmm7 = c7 */							\
	\
	__asm	psubsw	xmm6, xmm2		/* xmm6 = c3 * i5 - c5 * i3 = D */		\
	__asm	paddw	xmm0, xmm3		/* xmm0 = c1 * i1 */					\
	\
	__asm	pmulhw	xmm3, xmm7		/* xmm3 = c7 * i1 */		\
	__asm	movdqa	xmm2, I(2)		/* xmm2 = i2 */				\
	\
	__asm	pmulhw  xmm7, xmm1		/* xmm7 = c7 * i7 */		\
	__asm	paddw	xmm5, xmm1		/* xmm5 = c1 * i7 */		\
	\
	__asm	movdqa	xmm1, xmm2		/* xmm1 = i2 */				\
	__asm	pmulhw	xmm2, C(2)		/* xmm2 = i2 * c2 -i2 */	\
	\
	__asm	psubsw	xmm3, xmm5		/* xmm3 = c7 * i1 - c1 * i7 = B */		\
	__asm	movdqa	xmm5, I(6)		/* xmm5 = i6 */							\
	\
	__asm	paddsw	xmm0, xmm7		/* xmm0 = c1 * i1 + c7 * i7	= A */		\
	__asm	movdqa	xmm7, xmm5		/* xmm7 = i6 */							\
	\
	__asm	psubsw	xmm0, xmm4		/* xmm0 = A - C */			\
	__asm	pmulhw	xmm5, C(2)		/* xmm5 = c2 * i6 - i6 */	\
	\
	__asm	paddw	xmm2, xmm1		/* xmm2 = i2 * c2 */		\
	__asm	pmulhw	xmm1, C(6)		/* xmm1 = c6 * i2 */		\
	\
	__asm	paddsw	xmm4, xmm4		/* xmm4 = C + C */			\
	__asm	paddsw	xmm4, xmm0		/* xmm4 = A + C = C. */		\
	\
	__asm	psubsw	xmm3, xmm6		/* xmm3 = B - D */			\
	__asm	paddw	xmm5, xmm7		/* xmm5 = c2 * i6 */		\
	\
	__asm	paddsw	xmm6, xmm6		/* xmm6 = D + D */			\
	__asm	pmulhw  xmm7, C(6)		/* xmm7 = c6 * i6 */		\
	\
	__asm	paddsw	xmm6, xmm3		/* xmm6 = B + D = D. */		\
	__asm	movdqa	I(1), xmm4		/* Save C. at I(1)	*/		\
	\
	__asm	psubsw	xmm1, xmm5		/* xmm1 = c6 * i2 - c2 * i6 = H */		\
	__asm	movdqa	xmm4, C(4)		/* xmm4 = c4 */							\
	\
	__asm	movdqa  xmm5, xmm3		/* xmm5 = B - D */						\
	__asm	pmulhw	xmm3, xmm4		/* xmm3 = ( c4 -1 ) * ( B - D ) */		\
	\
	__asm	paddsw	xmm7, xmm2		/* xmm7 = c2 * i2 + c6 * i6 = G */		\
	__asm	movdqa	I(2), xmm6		/* Save D. at I(2) */		\
	\
	__asm	movdqa	xmm2, xmm0		/* xmm2 = A - C */			\
	__asm	movdqa	xmm6, I(0)		/* xmm6 = i0 */				\
	\
	__asm	pmulhw	xmm0, xmm4		/* xmm0 = ( c4 - 1 ) * ( A - C ) = A. */\
	__asm	paddw	xmm5, xmm3		/* xmm5 = c4 * ( B - D ) = B. */		\
	\
	__asm	movdqa	xmm3, I(4)		/* xmm3 = i4 */				\
	__asm	psubsw	xmm5, xmm1		/* xmm5 = B. - H = B.. */	\
	\
	__asm	paddw	xmm2, xmm0		/* xmm2 = c4 * ( A - C) = A. */			\
	__asm	psubsw	xmm6, xmm3		/* xmm6 = i0 - i4 */					\
	\
	__asm	movdqa	xmm0, xmm6		/* xmm0 = i0 - i4 */					\
	__asm	pmulhw	xmm6, xmm4		/* xmm6 = (c4 - 1) * (i0 - i4) = F */	\
	\
	__asm	paddsw	xmm3, xmm3		/* xmm3 = i4 + i4 */		\
	__asm	paddsw	xmm1, xmm1		/* xmm1 = H + H */			\
	\
	__asm	paddsw	xmm3, xmm0		/* xmm3 = i0 + i4 */		\
	__asm	paddsw	xmm1, xmm5		/* xmm1 = B. + H = H. */	\
	\
	__asm	pmulhw	xmm4, xmm3		/* xmm4 = ( c4 - 1 ) * ( i0 + i4 )  */	\
	__asm	paddw	xmm6, xmm0		/* xmm6 = c4 * ( i0 - i4 ) */			\
	\
	__asm	psubsw	xmm6, xmm2		/* xmm6 = F - A. = F. */	\
	__asm	paddsw	xmm2, xmm2		/* xmm2 = A. + A. */		\
	\
	__asm	movdqa	xmm0, I(1)		/* Load	C. from I(1) */		\
	__asm	paddsw	xmm2, xmm6		/* xmm2 = F + A. = A.. */	\
	\
	__asm	paddw	xmm4, xmm3		/* xmm4 = c4 * ( i0 + i4 ) = 3 */		\
	__asm	psubsw  xmm2, xmm1		/* xmm2 = A.. - H. = R2 */				\
	\
	__asm	paddsw	xmm2, Eight		/* Adjust R2 and R1 before shifting */	\
	__asm	paddsw  xmm1, xmm1		/* xmm1 = H. + H. */					\
	\
	__asm	paddsw  xmm1, xmm2		/* xmm1 = A.. + H. = R1 */	\
	__asm	psraw	xmm2, 4			/* xmm2 = op2 */			\
	\
	__asm	psubsw	xmm4, xmm7		/* xmm4 = E - G = E. */		\
	__asm	psraw	xmm1, 4			/* xmm1 = op1 */			\
	\
	__asm   movdqa	xmm3, I(2)		/* Load D. from I(2) */		\
	__asm	paddsw	xmm7, xmm7		/* xmm7 = G + G */			\
	\
	__asm	movdqa	O(2), xmm2		/* Write out op2 */			\
	__asm	paddsw  xmm7, xmm4		/* xmm7 = E + G = G. */		\
	\
	__asm	movdqa	O(1), xmm1		/* Write out op1 */			\
	__asm	psubsw  xmm4, xmm3		/* xmm4 = E. - D. = R4 */	\
	\
	__asm	paddsw	xmm4, Eight		/* Adjust R4 and R3 before shifting */	\
	__asm	paddsw  xmm3, xmm3		/* xmm3 = D. + D. */					\
	\
	__asm	paddsw	xmm3, xmm4		/* xmm3 = E. + D. = R3 */	\
	__asm	psraw	xmm4, 4			/* xmm4 = op4 */			\
	\
	__asm	psubsw	xmm6, xmm5		/* xmm6 = F. - B..= R6 */	\
	__asm	psraw	xmm3, 4			/* xmm3 = op3 */			\
	\
	__asm	paddsw	xmm6, Eight		/* Adjust R6 and R5 before shifting */	\
	__asm	paddsw	xmm5, xmm5		/* xmm5 = B.. + B.. */					\
	\
	__asm	paddsw	xmm5, xmm6		/* xmm5 = F. + B.. = R5 */	\
	__asm	psraw	xmm6, 4			/* xmm6 = op6 */			\
	\
	__asm	movdqa	O(4), xmm4		/* Write out op4 */			\
	__asm	psraw	xmm5, 4			/* xmm5 = op5 */			\
	\
	__asm 	movdqa	O(3), xmm3		/* Write out op3 */			\
	__asm	psubsw	xmm7, xmm0		/* xmm7 = G. - C. = R7 */	\
	\
	__asm	paddsw  xmm7, Eight		/* Adjust R7 and R0 before shifting */	\
	__asm	paddsw  xmm0, xmm0		/* xmm0 = C. + C. */					\
	\
	__asm	paddsw  xmm0, xmm7		/* xmm0 = G. + C. */		\
	__asm	psraw	xmm7, 4			/* xmm7 = op7 */			\
	\
	__asm	movdqa	O(6), xmm6		/* Write out op6 */			\
	__asm	psraw	xmm0, 4			/* xmm0 = op0 */			\
	\
	__asm	movdqa	O(5), xmm5		/* Write out op5 */			\
	__asm	movdqa	O(7), xmm7		/* Write out op7 */			\
	\
	__asm	movdqa	O(0), xmm0		/* Write out op0 */			\
	\
	} /* End of Wmt_Column_IDCT macro */


/**************************************************************************************
 *
 *		Macro:			Wmt_Row_IDCT
 *		
 *		Description:	The Macro does 1-D IDct on 8 columns. 
 *
 *		Input:			None
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	None
 *
 *		Error:			None
 *
 ***************************************************************************************
 */

/*	
	The major difference between Willamette processor and other IA32 processors is that 
	all of the simd integer instructions now support the 128 bit xmm registers instead 
	of 64 bit mmx registers. By using these instructions, we can do 8 1-D coloumn idcts 
	that takes shorts as input and outputs shorts at once

*/

#define Wmt_Row_IDCT __asm {		\
	\
	__asm	movdqa	xmm2, I(3)		/* xmm2 = i3 */		\
	__asm	movdqa	xmm6, C(3)		/* xmm6 = c3 */		\
	\
	__asm	movdqa	xmm4, xmm2		/* xmm4 = i3 */		\
	__asm	movdqa	xmm7, I(5)		/* xmm7 = i5 */		\
	\
	__asm	pmulhw xmm4, xmm6		/* xmm4 = c3 * i3 - i3 */	\
	__asm	movdqa  xmm1, C(5)		/* xmm1 = c5 */		\
	\
	__asm	pmulhw xmm6, xmm7		/* xmm6 = c3 * i5 - i5 */	\
	__asm	movdqa	xmm5, xmm1		/* xmm5 = c5 */		\
	\
	__asm	pmulhw	xmm1, xmm2		/* xmm1 = c5 * i3 - i3 */	\
	__asm	movdqa  xmm3, I(1)		/* xmm3 = i1 */		\
	\
	__asm	pmulhw	xmm5, xmm7		/* xmm5 = c5 * i5 - i5 */	\
	__asm	movdqa	xmm0, C(1)		/* xmm0 = c1 */		\
	\
	/* all registers are in use */ \
	\
	__asm	paddw   xmm4, xmm2		/* xmm4 = c3 * i3 */	\
	__asm	paddw	xmm6, xmm7		/* xmm6 = c3 * i5 */	\
	\
	__asm	paddw	xmm2, xmm1		/* xmm2 = c5 * i3 */	\
	__asm	movdqa	xmm1, I(7)		/* xmm1 = i7 */		\
	\
	__asm	paddw	xmm7, xmm5		/* xmm7 = c5 * i5 */	\
	__asm	movdqa	xmm5, xmm0		/* xmm5 = c1 */		\
	\
	__asm	pmulhw	xmm0, xmm3		/* xmm0 = c1 * i1 - i1 */	\
	__asm	paddsw	xmm4, xmm7		/* xmm4 = c3 * i3 + c5 * i5 = C */		\
	\
	__asm	pmulhw	xmm5, xmm1		/* xmm5 = c1 * i7 - i7 */	\
	__asm   movdqa	xmm7, C(7)		/* xmm7 = c7 */		\
	\
	__asm	psubsw	xmm6, xmm2		/* xmm6 = c3 * i5 - c5 * i3 = D */		\
	__asm	paddw	xmm0, xmm3		/* xmm0 = c1 * i1 */	\
	\
	__asm	pmulhw	xmm3, xmm7		/* xmm3 = c7 * i1 */	\
	__asm	movdqa	xmm2, I(2)		/* xmm2 = i2 */		\
	\
	__asm	pmulhw  xmm7, xmm1		/* xmm7 = c7 * i7 */	\
	__asm	paddw	xmm5, xmm1		/* xmm5 = c1 * i7 */	\
	\
	__asm	movdqa	xmm1, xmm2		/* xmm1 = i2 */		\
	__asm	pmulhw	xmm2, C(2)		/* xmm2 = i2 * c2 -i2 */	\
	\
	__asm	psubsw	xmm3, xmm5		/* xmm3 = c7 * i1 - c1 * i7 = B */		\
	__asm	movdqa	xmm5, I(6)		/* xmm5 = i6 */		\
	\
	__asm	paddsw	xmm0, xmm7		/* xmm0 = c1 * i1 + c7 * i7	= A */		\
	__asm	movdqa	xmm7, xmm5		/* xmm7 = i6 */		\
	\
	__asm	psubsw	xmm0, xmm4		/* xmm0 = A - C */	\
	__asm	pmulhw	xmm5, C(2)		/* xmm5 = c2 * i6 - i6 */	\
	\
	__asm	paddw	xmm2, xmm1		/* xmm2 = i2 * c2 */	\
	__asm	pmulhw	xmm1, C(6)		/* xmm1 = c6 * i2 */	\
	\
	__asm	paddsw	xmm4, xmm4		/* xmm4 = C + C */		\
	__asm	paddsw	xmm4, xmm0		/* xmm4 = A + C = C. */	\
	\
	__asm	psubsw	xmm3, xmm6		/* xmm3 = B - D */		\
	__asm	paddw	xmm5, xmm7		/* xmm5 = c2 * i6 */	\
	\
	__asm	paddsw	xmm6, xmm6		/* xmm6 = D + D */		\
	__asm	pmulhw  xmm7, C(6)		/* xmm7 = c6 * i6 */	\
	\
	__asm	paddsw	xmm6, xmm3		/* xmm6 = B + D = D. */	\
	__asm	movdqa	I(1), xmm4		/* Save C. at I(1)	*/	\
	\
	__asm	psubsw	xmm1, xmm5		/* xmm1 = c6 * i2 - c2 * i6 = H */	\
	__asm	movdqa	xmm4, C(4)		/* xmm4 = c4 */		\
	\
	__asm	movdqa  xmm5, xmm3		/* xmm5 = B - D */	\
	__asm	pmulhw	xmm3, xmm4		/* xmm3 = ( c4 -1 ) * ( B - D ) */		\
	\
	__asm	paddsw	xmm7, xmm2		/* xmm7 = c2 * i2 + c6 * i6 = G */	\
	__asm	movdqa	I(2), xmm6		/* Save D. at I(2) */	\
	\
	__asm	movdqa	xmm2, xmm0		/* xmm2 = A - C */	\
	__asm	movdqa	xmm6, I(0)		/* xmm6 = i0 */		\
	\
	__asm	pmulhw	xmm0, xmm4		/* xmm0 = ( c4 - 1 ) * ( A - C ) = A. */	\
	__asm	paddw	xmm5, xmm3		/* xmm5 = c4 * ( B - D ) = B. */	\
	\
	__asm	movdqa	xmm3, I(4)		/* xmm3 = i4 */		\
	__asm	psubsw	xmm5, xmm1		/* xmm5 = B. - H = B.. */	\
	\
	__asm	paddw	xmm2, xmm0		/* xmm2 = c4 * ( A - C) = A. */		\
	__asm	psubsw	xmm6, xmm3		/* xmm6 = i0 - i4 */	\
	\
	__asm	movdqa	xmm0, xmm6		/* xmm0 = i0 - i4 */	\
	__asm	pmulhw	xmm6, xmm4		/* xmm6 = ( c4 - 1 ) * ( i0 - i4 ) = F */	\
	\
	__asm	paddsw	xmm3, xmm3		/* xmm3 = i4 + i4 */	\
	__asm	paddsw	xmm1, xmm1		/* xmm1 = H + H */	\
	\
	__asm	paddsw	xmm3, xmm0		/* xmm3 = i0 + i4 */	\
	__asm	paddsw	xmm1, xmm5		/* xmm1 = B. + H = H. */	\
	\
	__asm	pmulhw	xmm4, xmm3		/* xmm4 = ( c4 - 1 ) * ( i0 + i4 )  */	\
	__asm	paddw	xmm6, xmm0		/* xmm6 = c4 * ( i0 - i4 ) */	\
	\
	__asm	psubsw	xmm6, xmm2		/* xmm6 = F - A. = F. */	\
	__asm	paddsw	xmm2, xmm2		/* xmm2 = A. + A. */	\
	\
	__asm	movdqa	xmm0, I(1)		/* Load	C. from I(1) */		\
	__asm	paddsw	xmm2, xmm6		/* xmm2 = F + A. = A.. */	\
	\
	__asm	paddw	xmm4, xmm3		/* xmm4 = c4 * ( i0 + i4 ) = 3 */	\
	__asm	psubsw  xmm2, xmm1		/* xmm2 = A.. - H. = R2 */	\
	\
	__asm	paddsw  xmm1, xmm1		/* xmm1 = H. + H. */	\
	__asm	paddsw  xmm1, xmm2		/* xmm1 = A.. + H. = R1 */	\
	\
	__asm	psubsw	xmm4, xmm7		/* xmm4 = E - G = E. */		\
	\
	__asm   movdqa	xmm3, I(2)		/* Load D. from I(2) */		\
	__asm	paddsw	xmm7, xmm7		/* xmm7 = G + G */	\
	\
	__asm	movdqa	I(2), xmm2		/* Write out op2 */		\
	__asm	paddsw  xmm7, xmm4		/* xmm7 = E + G = G. */		\
	\
	__asm	movdqa	I(1), xmm1		/* Write out op1 */		\
	__asm	psubsw  xmm4, xmm3		/* xmm4 = E. - D. = R4 */	\
	\
	__asm	paddsw  xmm3, xmm3		/* xmm3 = D. + D. */	\
	\
	__asm	paddsw	xmm3, xmm4		/* xmm3 = E. + D. = R3 */	\
	\
	__asm	psubsw	xmm6, xmm5		/* xmm6 = F. - B..= R6 */	\
	\
	__asm	paddsw	xmm5, xmm5		/* xmm5 = B.. + B.. */	\
	\
	__asm	paddsw	xmm5, xmm6		/* xmm5 = F. + B.. = R5 */	\
	\
	__asm	movdqa	I(4), xmm4		/* Write out op4 */		\
	\
	__asm 	movdqa	I(3), xmm3		/* Write out op3 */		\
	__asm	psubsw	xmm7, xmm0		/* xmm7 = G. - C. = R7 */	\
	\
	__asm	paddsw  xmm0, xmm0		/* xmm0 = C. + C. */	\
	\
	__asm	paddsw  xmm0, xmm7		/* xmm0 = G. + C. */	\
	\
	__asm	movdqa	I(6), xmm6		/* Write out op6 */		\
	\
	__asm	movdqa	I(5), xmm5		/* Write out op5 */		\
	__asm	movdqa	I(7), xmm7		/* Write out op7 */		\
	\
	__asm	movdqa	I(0), xmm0		/* Write out op0 */		\
	\
	} /* End of Wmt_Row_IDCT macro */

/**************************************************************************************
 *
 *		Macro:			Transpose
 *		
 *		Description:	The Macro does 8x8 transpose
 *
 *		Input:			None
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	None
 *
 *		Error:			None
 *
 ***************************************************************************************
 */


#define Transpose __asm {	\
	\
	__asm	movdqa		xmm4, I(4)		/* xmm4=e7e6e5e4e3e2e1e0 */	\
	__asm	movdqa		xmm0, I(5)		/* xmm4=f7f6f5f4f3f2f1f0 */	\
	\
	__asm	movdqa		xmm5, xmm4		/* make a copy */			\
	__asm	punpcklwd	xmm4, xmm0		/* xmm4=f3e3f2e2f1e1f0e0 */	\
	\
	__asm	punpckhwd	xmm5, xmm0		/* xmm5=f7e7f6e6f5e5f4e4 */	\
	__asm	movdqa		xmm6, I(6)		/* xmm6=g7g6g5g4g3g2g1g0 */ \
	\
	__asm	movdqa		xmm0, I(7)		/* xmm0=h7h6h5h4h3h2h1h0 */ \
	__asm	movdqa		xmm7, xmm6		/* make a copy */			\
	\
	__asm	punpcklwd	xmm6, xmm0		/* xmm6=h3g3h3g2h1g1h0g0 */ \
	__asm	punpckhwd	xmm7, xmm0		/* xmm7=h7g7h6g6h5g5h4g4 */ \
	\
	__asm	movdqa		xmm3, xmm4		/* make a copy */			\
	__asm	punpckldq	xmm4, xmm6		/* xmm4=h1g1f1e1h0g0f0e0 */	\
	\
	__asm	punpckhdq	xmm3, xmm6		/* xmm3=h3g3g3e3h2g2f2e2 */	\
	__asm	movdqa		I(6), xmm3		/* save h3g3g3e3h2g2f2e2 */	\
	/* Free xmm6 */ \
	__asm	movdqa		xmm6, xmm5		/* make a copy */			\
	__asm	punpckldq	xmm5, xmm7		/* xmm5=h5g5f5e5h4g4f4e4 */ \
	\
	__asm	punpckhdq	xmm6, xmm7		/* xmm6=h7g7f7e7h6g6f6e6 */ \
	__asm	movdqa		xmm0, I(0)		/* xmm0=a7a6a5a4a3a2a1a0 */	\
	/* Free xmm7 */ \
	__asm	movdqa		xmm1, I(1)		/* xmm1=b7b6b5b4b3b2b1b0 */	\
	__asm	movdqa		xmm7, xmm0		/* make a copy */			\
	\
	__asm	punpcklwd	xmm0, xmm1		/* xmm0=b3a3b2a2b1a1b0a0 */	\
	__asm	punpckhwd	xmm7, xmm1		/* xmm7=b7a7b6a6b5a5b4a4 */ \
	/* Free xmm1 */ \
	__asm	movdqa		xmm2, I(2)		/* xmm2=c7c6c5c4c3c2c1c0 */ \
	__asm	movdqa		xmm3, I(3)	    /* xmm3=d7d6d5d4d3d2d1d0 */ \
	\
	__asm	movdqa		xmm1, xmm2		/* make a copy */			\
	__asm	punpcklwd	xmm2, xmm3		/* xmm2=d3c3d2c2d1c1d0c0 */ \
	\
	__asm	punpckhwd	xmm1, xmm3		/* xmm1=d7c7d6c6d5c5d4c4 */ \
	__asm	movdqa		xmm3, xmm0		/* make a copy	*/			\
	\
	__asm	punpckldq	xmm0, xmm2		/* xmm0=d1c1b1a1d0c0b0a0 */ \
	__asm	punpckhdq	xmm3, xmm2		/* xmm3=d3c3b3a3d2c2b2a2 */ \
	/* Free xmm2 */ \
	__asm	movdqa		xmm2, xmm7		/* make a copy */			\
	__asm	punpckldq	xmm2, xmm1		/* xmm2=d5c5b5a5d4c4b4a4 */	\
	\
	__asm	punpckhdq	xmm7, xmm1		/* xmm7=d7c7b7a7d6c6b6a6 */ \
	__asm	movdqa		xmm1, xmm0		/* make a copy */			\
	\
	__asm	punpcklqdq	xmm0, xmm4		/* xmm0=h0g0f0e0d0c0b0a0 */	\
	__asm	punpckhqdq	xmm1, xmm4		/* xmm1=h1g1g1e1d1c1b1a1 */ \
	\
	__asm	movdqa		I(0), xmm0		/* save I(0) */				\
	__asm	movdqa		I(1), xmm1		/* save I(1) */				\
	\
	__asm	movdqa		xmm0, I(6)		/* load h3g3g3e3h2g2f2e2 */ \
	__asm	movdqa		xmm1, xmm3		/* make a copy */			\
	\
	__asm	punpcklqdq	xmm1, xmm0		/* xmm1=h2g2f2e2d2c2b2a2 */ \
	__asm	punpckhqdq	xmm3, xmm0		/* xmm3=h3g3f3e3d3c3b3a3 */	\
	\
	__asm	movdqa		xmm4, xmm2		/* make a copy */			\
	__asm	punpcklqdq	xmm4, xmm5		/* xmm4=h4g4f4e4d4c4b4a4 */	\
	\
	__asm	punpckhqdq	xmm2, xmm5		/* xmm2=h5g5f5e5d5c5b5a5 */	\
	__asm	movdqa		I(2), xmm1		/* save I(2) */				\
	\
	__asm	movdqa		I(3), xmm3		/* save I(3) */				\
	__asm	movdqa		I(4), xmm4		/* save I(4) */				\
	\
	__asm	movdqa		I(5), xmm2		/* save I(5) */				\
	__asm	movdqa		xmm5, xmm7		/* make a copy */			\
	\
	__asm	punpcklqdq	xmm5, xmm6		/* xmm5=h6g6f6e6d6c6b6a6 */	\
	__asm	punpckhqdq	xmm7, xmm6		/* xmm7=h7g7f7e7d7c7b7a7 */	\
	\
	__asm	movdqa		I(6), xmm5		/* save I(6) */				\
	__asm	movdqa		I(7), xmm7		/* save I(7) */				\
	\
	}/* End of Transpose Macro */


/**************************************************************************************
 *
 *		Macro:			Wmt_Dequant
 *		
 *		Description:	The Macro does dequantzation and reorder the coefficents to avoid 
 *						the first transpose before Wmt_Row_IDCT
 *
 *		Input:			[eax], quantized input, 
 *						[ebx], quantizaiton table,
 *
 *		Output:			[eax]
 *		
 *		Return:			None			
 *
 *		Special Note:	None
 *
 *		Error:			None
 *
 ***************************************************************************************
 */
#define Wmt_Dequant __asm {		\
	__asm	lea		ecx, WmtDequantConst										\
	__asm	movdqa	xmm0, [eax]													\
	\
	__asm	pmullw	xmm0, [ebx]			/* xmm0 = 07 06 05 04 03 02 01 00 */	\
	__asm	movdqa	xmm1, [eax + 16]											\
	\
	__asm	pmullw	xmm1, [ebx + 16]	/* xmm1 = 17 16 15 14 13 12 11 10 */	\
	__asm	pshuflw xmm3, xmm0,	078h		/* xmm3 = 07 06 05 04 01 03 02 00 */	\
	\
	__asm	movdqa	xmm2, xmm1			/* xmm2 = 17 16 15 14 13 12 11 10 */	\
	__asm	movdqa	xmm7, [ecx]			/* xmm7 = -- -- -- -- -- FF FF -- */	\
	\
	__asm	movdqa	xmm4, [eax + 32]											\
	__asm	movdqa	xmm5, [eax + 64]											\
	\
	__asm	pmullw	xmm4, [ebx + 32]	/* xmm4 = 27 26 25 24 23 22 21 20 */	\
	__asm	pmullw	xmm5, [ebx + 64]	/* xmm5	= 47 46 45 44 43 42 41 40 */	\
	\
	__asm	movdqa	xmm6, [ecx + 16]	/* xmm6 = -- -- FF FF -- -- -- -- */	\
	__asm	pand	xmm7, xmm2			/* xmm7 = -- -- -- -- -- 12 11 -- */	\
	\
	__asm	pand	xmm6, xmm4			/* xmm6 = -- -- 25 24 -- -- -- -- */	\
	__asm	pxor	xmm2, xmm7			/* xmm2 = 17 16 15 14 13 -- -- 10 */	\
	\
	__asm	pxor	xmm4, xmm6			/* xmm4 = 27 26 -- -- 23 22 21 20 */	\
	__asm	pslldq  xmm7, 4				/* xmm7 = -- -- -- 12 11 -- -- -- */	\
	\
	__asm	pslldq	xmm6, 2				/* xmm6 = -- 25 24 -- -- -- -- -- */	\
	__asm	por		xmm7, xmm6			/* xmm7 = -- 25 24 12 11 -- -- -- */	\
	\
	__asm	movdqa	xmm0, [ecx + 32]	/* xmm0 = -- -- -- -- -- FF FF FF */	\
	__asm	movdqa	xmm6, [ecx + 48]	/* xmm6 = -- -- -- -- FF -- -- -- */	\
	\
	__asm	pand	xmm0, xmm3			/* xmm0 = -- -- -- -- -- 03 02 00 */	\
	__asm   pand	xmm6, xmm5			/* xmm6 = -- -- -- -- 43 -- -- -- */	\
	\
	__asm   pxor	xmm3, xmm0			/* xmm3 = 07 06 05 04 01 -- -- -- */	\
	__asm	pxor	xmm5, xmm6			/* xmm5 = 47 46 45 44 -- 42 41 40 */	\
	\
	__asm	por		xmm0, xmm7			/* xmm0 = -- 25 24 12 11 03 02 00 */	\
	__asm	pslldq	xmm6, 8			    /* xmm6 = 43 -- -- -- -- -- -- -- */	\
	\
 	__asm	por		xmm0, xmm6		/* O0 =xmm0 = 43 25 24 12 11 03 02 00 */	\
	/* 02345 in use */ \
	\
	__asm	movdqa	xmm1, [ecx + 64 ]	/* xmm1 = -- -- -- FF FF -- -- -- */	\
	__asm	pshuflw	xmm5, xmm5, 0B4h		/* xmm5 = 47 46 45 44 42 -- 41 40 */	\
	\
	__asm	movdqa	xmm7, xmm1			/* xmm7 = -- -- -- FF FF -- -- -- */	\
	__asm	movdqa	xmm6, xmm1			/* xmm6 = -- -- -- FF FF -- -- -- */	\
	\
	__asm	movdqa	[eax], xmm0			/* write  43 25 24 12 11 03 02 00 */	\
	__asm	pshufhw	xmm4, xmm4, 0C2h		/* xmm4 = 27 -- -- 26 23 22 21 20 */	\
	\
	__asm	pand	xmm7, xmm4			/* xmm7 = -- -- -- 26 23 -- -- -- */	\
	__asm	pand	xmm1, xmm5			/* xmm1 = -- -- -- 44 42 -- -- -- */	\
	\
	__asm	pxor	xmm4, xmm7			/* xmm4 = 27 -- -- -- -- 22 21 20 */	\
	__asm	pxor	xmm5, xmm1			/* xmm5 = 47 46 45 -- -- -- 41 40 */	\
	\
	__asm	pshuflw	xmm2, xmm2, 0C6h		/* xmm2 = 17 16 15 14 13 10 -- -- */	\
	__asm	movdqa	xmm0, xmm6			/* xmm0 = -- -- -- FF FF -- -- -- */	\
	\
	__asm	pslldq	xmm7, 2				/* xmm7 = -- -- 26 23 -- -- -- -- */	\
	__asm	pslldq  xmm1, 6				/* xmm1 = 44 42 -- -- -- -- -- -- */	\
	\
	__asm	psrldq	xmm0, 2				/* xmm0 = -- -- -- -- FF FF -- -- */	\
	__asm	pand	xmm6, xmm3			/* xmm6 = -- -- -- 04 01 -- -- -- */	\
	\
	__asm	pand	xmm0, xmm2			/* xmm0 = -- -- -- -- 13 10 -- -- */	\
	__asm	pxor	xmm3, xmm6			/* xmm3 = 07 06 05 -- -- -- -- -- */	\
	\
	__asm	pxor	xmm2, xmm0			/* xmm2 = 17 16 15 14 -- -- -- -- */	\
	__asm	psrldq	xmm6, 6				/* xmm0 = -- -- -- -- -- -- 04 01 */	\
	\
	__asm	por		xmm1, xmm7			/* xmm1 = 44 42 26 23 -- -- -- -- */	\
	__asm	por		xmm0, xmm6			/* xmm1 = -- -- -- -- 13 10 04 01 */	\
	/* 12345 in use */	\
	__asm   por		xmm1, xmm0		/* o1 =xmm1 = 44 42 26 23 13 10 04 01 */	\
	__asm   pshuflw	xmm4, xmm4, 093h		/* xmm4 = 27 -- -- -- 22 21 20 -- */	\
	\
	__asm	pshufhw	xmm4, xmm4, 093h		/* xmm4 = -- -- -- 27 22 21 20 -- */	\
	__asm	movdqa	[eax + 16], xmm1	/* write  44 42 26 23 13 10 04 01 */	\
	\
	__asm	pshufhw	xmm3, xmm3, 0D2h		/* xmm3 = 07 05 -- 06 -- -- -- -- */	\
	__asm	movdqa	xmm0, [ecx + 64]	/* xmm0 = -- -- -- FF FF -- -- -- */	\
	\
	__asm	pand	xmm0, xmm3			/* xmm0 = -- -- -- 06 -- -- -- -- */	\
	__asm	psrldq	xmm3, 12			/* xmm3 = -- -- -- -- -- -- 07 05 */	\
	\
	__asm	psrldq	xmm0, 8				/* xmm0 = -- -- -- -- -- -- -- 06 */	\
	\
	__asm	movdqa	xmm6, [ecx + 64]	/* xmm6 = -- -- -- FF FF -- -- -- */	\
	__asm	movdqa	xmm7, [ecx + 96]	/* xmm7 = -- -- -- -- FF FF -- -- */	\
	\
	__asm	pand	xmm6, xmm4			/* xmm6 = -- -- -- 27 22 -- -- -- */	\
	__asm   pxor	xmm4, xmm6			/* xmm4 = -- -- -- -- -- 21 20 -- */	\
	\
	__asm	por		xmm3, xmm6			/* xmm3 = -- -- -- 27 22 -- 07 05 */	\
	__asm	pand	xmm7, xmm4		    /* xmm7 = -- -- -- -- -- 21 -- -- */	\
	\
	__asm	por		xmm0, xmm7			/* xmm0 = -- -- -- -- -- 21 -- 06 */	\
	__asm	pxor	xmm4, xmm7			/* xmm4 = -- -- -- -- -- -- 20 -- */	\
	\
	__asm	movdqa	xmm6, [ecx + 16 ]	/* xmm6 = -- -- FF FF -- -- -- -- */	\
	__asm	movdqa	xmm1, [ecx + 64 ]	/* xmm1 = -- -- -- FF FF -- -- -- */	\
	\
	__asm	pand	xmm6, xmm2			/* xmm6 = -- -- 15 14 -- -- -- -- */	\
	__asm	pand	xmm1, xmm6			/* xmm1 = -- -- -- 14 -- -- -- -- */	\
	\
	__asm	pxor	xmm2, xmm6			/* xmm2 = 17 16 -- -- -- -- -- -- */	\
	__asm	pxor	xmm6, xmm1			/* xmm6 = -- -- 15 -- -- -- -- -- */	\
	\
	__asm	psrldq	xmm1, 4				/* xmm1 = -- -- -- -- -- 14 -- -- */	\
	\
	__asm	psrldq	xmm6, 8				/* xmm6 = -- -- -- -- -- -- 15 -- */	\
	__asm	por		xmm3, xmm1			/* xmm3 = -- -- -- 27 22 14 07 05 */	\
	\
	__asm	por		xmm0, xmm6			/* xmm0 = -- -- -- -- -- 21 15 06 */	\
	__asm	pshufhw	xmm5, xmm5, 0E1h		/* xmm5 = 47 46 -- 45 -- -- 41 40 */	\
	\
	__asm	movdqa	xmm1, [ecx + 64]	/* xmm1 = -- -- -- FF FF -- -- -- */	\
	__asm	pshuflw	xmm5, xmm5, 072h		/* xmm5 = 47 46 -- 45 41 -- 40 -- */	\
	\
	__asm	movdqa	xmm6, xmm1			/* xmm6 = -- -- -- FF FF -- -- -- */	\
	__asm	pand	xmm1, xmm5			/* xmm1 = -- -- -- 45 41 -- -- -- */	\
	\
	__asm	pxor	xmm5, xmm1			/* xmm5 = 47 46 -- -- -- -- 40 -- */	\
	__asm	pslldq	xmm1, 4				/* xmm1 = -- 45 41 -- -- -- -- -- */	\
	\
	__asm	pshufd	xmm5, xmm5, 09Ch		/* xmm5 = -- -- -- -- 47 46 40 -- */	\
	__asm	por		xmm3, xmm1			/* xmm3 = -- 45 41 27 22 14 07 05 */	\
	\
	__asm	movdqa	xmm1, [eax + 96]	/* xmm1 = 67 66 65 64 63 62 61 60 */	\
	__asm	pmullw	xmm1, [ebx + 96]											\
	\
	__asm	movdqa	xmm7, [ecx]		    /* xmm7 = -- -- -- -- -- FF FF -- */	\
	\
	__asm	psrldq	xmm6, 8				/* xmm6 = -- -- -- -- -- -- -- FF */	\
	__asm	pand	xmm7, xmm5			/* xmm7 = -- -- -- -- -- 46 40 -- */	\
	\
	__asm	pand	xmm6, xmm1			/* xmm6 = -- -- -- -- -- -- -- 60 */	\
	__asm	pxor	xmm5, xmm7		    /* xmm5 = -- -- -- -- 47 -- -- -- */	\
	\
	__asm	pxor	xmm1, xmm6			/* xmm1 = 67 66 65 64 63 62 61 -- */	\
	__asm	pslldq	xmm5, 2				/* xmm5 = -- -- -- 47 -- -- -- -- */	\
	\
	__asm	pslldq	xmm6, 14			/* xmm6 = 60 -- -- -- -- -- -- -- */	\
	__asm	por		xmm4, xmm5			/* xmm4 = -- -- -- 47 -- -- 20 -- */	\
	\
	__asm	por		xmm3, xmm6		/* O2 = xmm3= 60 45 41 27 22 14 07 05 */	\
	__asm	pslldq	xmm7, 6				/* xmm7 = -- -- 46 40 -- -- -- -- */	\
	\
	__asm	movdqa	[eax+32], xmm3		/* write  60 45 41 27 22 14 07 05 */	\
	__asm	por		xmm0, xmm7			/* xmm0 = -- -- 46 40 -- 21 15 06 */	\
	/* 0, 1, 2, 4 in use */	\
	__asm	movdqa	xmm3, [eax + 48]	/* xmm3 = 37 36 35 34 33 32 31 30 */	\
	__asm	movdqa	xmm5, [eax + 80]	/* xmm5 = 57 56 55 54 53 52 51 50 */	\
	\
	__asm	pmullw	xmm3, [ebx + 48]											\
	__asm	pmullw	xmm5, [ebx + 80]											\
	\
	__asm	movdqa	xmm6, [ecx + 64]	/* xmm6 = -- -- -- FF FF -- -- -- */	\
	__asm	movdqa	xmm7, [ecx + 64]	/* xmm7 = -- -- -- FF FF -- -- -- */	\
	\
	__asm	psrldq	xmm6, 8				/* xmm6 = -- -- -- -- -- -- -- FF */	\
	__asm	pslldq	xmm7, 8				/* xmm7 = FF -- -- -- -- -- -- -- */	\
	\
	__asm	pand	xmm6, xmm3			/* xmm6 = -- -- -- -- -- -- -- 30 */	\
	__asm	pand	xmm7, xmm5			/* xmm7 = 57 -- -- -- -- -- -- -- */	\
	\
	__asm	pxor	xmm3, xmm6			/* xmm3 = 37 36 35 34 33 32 31 -- */	\
	__asm	pxor	xmm5, xmm7			/* xmm5 = __ 56 55 54 53 52 51 50 */	\
	\
	__asm	pslldq	xmm6, 6				/* xmm6 = -- -- -- -- 30 -- -- -- */	\
	__asm	psrldq	xmm7, 2				/* xmm7 = -- 57 -- -- -- -- -- -- */	\
	\
	__asm	por		xmm6, xmm7			/* xmm6 = -- 57 -- -- 30 -- -- -- */	\
	__asm	movdqa	xmm7, [ecx]			/* xmm7 = -- -- -- -- -- FF FF -- */	\
	\
	__asm	por		xmm0, xmm6			/* xmm0 = -- 57 46 40 30 21 15 06 */	\
	__asm	psrldq	xmm7, 2				/* xmm7 = -- -- -- -- -- -- FF FF */	\
	\
	__asm	movdqa	xmm6, xmm2			/* xmm6 = 17 16 -- -- -- -- -- -- */	\
	__asm	pand	xmm7, xmm1			/* xmm7 = -- -- -- -- -- -- 61 -- */	\
	\
	__asm	pslldq	xmm6, 2				/* xmm6 = 16 -- -- -- -- -- -- -- */	\
	__asm	psrldq	xmm2, 14			/* xmm2 = -- -- -- -- -- -- -- 17 */	\
	\
	__asm	pxor	xmm1, xmm7			/* xmm1 = 67 66 65 64 63 62 -- -- */	\
	__asm	pslldq	xmm7, 12			/* xmm7 = 61 -- -- -- -- -- -- -- */	\
	\
	__asm	psrldq	xmm6, 14			/* xmm6 = -- -- -- -- -- -- -- 16 */	\
	__asm	por		xmm4, xmm6			/* xmm4 = -- -- -- 47 -- -- 20 16 */	\
	\
	__asm	por		xmm0, xmm7			/* xmm0 = 61 57 46 40 30 21 15 06 */	\
	__asm	movdqa	xmm6, [ecx]			/* xmm6 = -- -- -- -- -- FF FF -- */	\
	\
	__asm	psrldq	xmm6, 2				/* xmm6 = -- -- -- -- -- -- FF FF */	\
	__asm	movdqa	[eax+48], xmm0		/* write  61 57 46 40 30 21 15 06 */	\
	/* 1, 2, 3, 4, 5 in use */\
	__asm	movdqa	xmm0, [ecx]			/* xmm0	= -- -- -- -- -- FF FF -- */	\
	__asm	pand	xmm6, xmm3			/* xmm6 = -- -- -- -- -- -- 31 -- */	\
	\
	__asm	movdqa	xmm7, xmm3			/* xmm7 = 37 36 35 34 33 32 31 -- */	\
	__asm	pxor	xmm3, xmm6			/* xmm3 = 37 36 35 34 33 32 -- -- */	\
	\
	__asm	pslldq	xmm3, 2				/* xmm3 = 36 35 34 33 32 -- -- -- */	\
	__asm	pand	xmm0, xmm1			/* xmm0 = -- -- -- -- -- 62 -- -- */	\
	\
	__asm	psrldq	xmm7, 14			/* xmm7 = -- -- -- -- -- -- -- 37 */	\
	__asm	pxor	xmm1, xmm0			/* xmm1 = 67 66 65 64 63 -- -- -- */	\
	\
	__asm	por		xmm6, xmm7			/* xmm6 = -- -- -- -- -- -- 31 37 */	\
	__asm	movdqa  xmm7, [ecx + 64]	/* xmm7 = -- -- -- FF FF -- -- -- */	\
	\
	__asm	pshuflw	xmm6, xmm6, 01Eh		/* xmm6	= -- -- -- -- 37 31 -- -- */	\
	__asm	pslldq	xmm7, 6				/* xmm7 = FF FF -- -- -- -- -- -- */	\
	\
	__asm	por		xmm4, xmm6			/* xmm4 = -- -- -- 47 37 31 20 16 */	\
	__asm	pand	xmm7, xmm5			/* xmm7 = -- 56 -- -- -- -- -- -- */	\
	\
	__asm	pslldq	xmm0, 8				/* xmm0 = -- 62 -- -- -- -- -- -- */	\
	__asm	pxor	xmm5, xmm7			/* xmm5 = -- -- 55 54 53 52 51 50 */	\
	\
	__asm	psrldq	xmm7, 2				/* xmm7 = -- -- 56 -- -- -- -- -- */	\
	\
	__asm	pshufhw	xmm3, xmm3, 087h		/* xmm3 = 35 33 34 36 32 -- -- -- */	\
	__asm	por		xmm0, xmm7			/* xmm0 = -- 62 56 -- -- -- -- -- */	\
	\
	__asm	movdqa	xmm7, [eax + 112]	/* xmm7 = 77 76 75 74 73 72 71 70 */	\
	__asm	pmullw	xmm7, [ebx + 112]											\
	\
	__asm	movdqa	xmm6, [ecx + 64]	/* xmm6 = -- -- -- FF FF -- -- -- */	\
	__asm	por		xmm4, xmm0			/* xmm4 = -- 62 56 47 37 31 20 16 */	\
	\
	__asm	pshuflw	xmm7, xmm7, 0E1h		/* xmm7 = 77 76 75 74 73 72 70 71 */	\
	__asm	psrldq	xmm6, 8				/* xmm6 = -- -- -- -- -- -- -- FF */	\
	\
	__asm	movdqa	xmm0, [ecx + 64]	/* xmm0 = -- -- -- FF FF -- -- -- */	\
	__asm	pand	xmm6, xmm7			/* xmm6 = -- -- -- -- -- -- -- 71 */	\
	\
	__asm	pand	xmm0, xmm3			/* xmm0 = -- -- -- 36 32 -- -- -- */	\
	__asm	pxor	xmm7, xmm6			/* xmm7 = 77 76 75 74 73 72 70 -- */	\
	\
	__asm	pxor	xmm3, xmm0			/* xmm3 = 35 33 34 -- -- -- -- -- */	\
	__asm	pslldq	xmm6, 14			/* xmm6 = 71 -- -- -- -- -- -- -- */	\
	\
	__asm	psrldq	xmm0, 4				/* xmm0 = -- -- -- -- -- 36 32 -- */	\
	__asm	por		xmm4, xmm6			/* xmm4 = 71 62 56 47 37 31 20 16 */	\
	\
	__asm	por		xmm2, xmm0			/* xmm2 = -- -- -- -- -- 36 32 17 */	\
	__asm	movdqa	[eax + 64], xmm4	/* write  71 62 56 47 37 31 20 16 */	\
	/* 1, 2, 3, 5, 7 in use */ \
	__asm	movdqa	xmm6, [ecx + 80]	/* xmm6 = -- -- FF -- -- -- -- FF */	\
	__asm	pshufhw	xmm7, xmm7,	0D2h		/* xmm7 = 77 75 74 76 73 72 70 __ */	\
	\
	__asm	movdqa	xmm4, [ecx]			/* xmm4 = -- -- -- -- -- FF FF -- */	\
	__asm	movdqa	xmm0, [ecx+48]		/* xmm0 = -- -- -- -- FF -- -- -- */	\
	\
	__asm	pand	xmm6, xmm5			/* xmm6 = -- -- 55 -- -- -- -- 50 */	\
	__asm	pand	xmm4, xmm7			/* xmm4 = -- -- -- -- -- 72 70 -- */	\
	\
	__asm	pand	xmm0, xmm1			/* xmm0 = -- -- -- -- 63 -- -- -- */	\
	__asm	pxor	xmm5, xmm6			/* xmm5 = -- -- -- 54 53 52 51 -- */	\
	\
	__asm	pxor	xmm7, xmm4			/* xmm7 = 77 75 74 76 73 -- -- -- */	\
	__asm	pxor	xmm1, xmm0			/* xmm1 = 67 66 65 64 -- -- -- -- */	\
	\
	__asm	pshuflw	xmm6, xmm6, 02Bh		/* xmm6 = -- -- 55 -- 50 -- -- -- */	\
	__asm	pslldq	xmm4, 10				/* xmm4 = 72 20 -- -- -- -- -- -- */	\
	\
	__asm	pshufhw	xmm6, xmm6, 0B1h		/* xmm6 = -- -- -- 55 50 -- -- -- */	\
	__asm	pslldq	xmm0, 4			/* xmm0 = -- -- 63 -- -- -- -- -- */	\
	\
	__asm	por		xmm6, xmm4			/* xmm6 = 72 70 -- 55 50 -- -- -- */	\
	__asm	por		xmm2, xmm0			/* xmm2 = -- -- 63 -- -- 36 32 17 */	\
	\
	__asm	por		xmm2, xmm6			/* xmm2 = 72 70 64 55 50 36 32 17 */	\
	__asm	pshufhw xmm1, xmm1, 0C9h		/* xmm1 = 67 64 66 65 -- -- -- -- */	\
	\
	__asm	movdqa	xmm6, xmm3			/* xmm6 = 35 33 34 -- -- -- -- -- */	\
	__asm	movdqa  [eax+80], xmm2		/* write  72 70 64 55 50 36 32 17 */	\
	\
	__asm	psrldq	xmm6, 12			/* xmm6 = -- -- -- -- -- -- 35 33 */	\
	__asm	pslldq	xmm3, 4				/* xmm3 = 34 -- -- -- -- -- -- -- */	\
	\
	__asm	pshuflw	xmm5, xmm5, 04Eh		/* xmm5 = -- -- -- 54 51 -- 53 52 */	\
	__asm	movdqa	xmm4, xmm7			/* xmm4 = 77 75 74 76 73 -- -- -- */	\
	\
	__asm	movdqa	xmm2, xmm5			/* xmm2 = -- -- -- 54 51 -- 53 52 */	\
	__asm	psrldq	xmm7, 10			/* xmm7 = -- -- -- -- -- 77 75 74 */	\
	\
	__asm	pslldq	xmm4, 6				/* xmm4 = 76 73 -- -- -- -- -- -- */	\
	__asm   pslldq	xmm2, 12			/* xmm2 = 53 52 -- -- -- -- -- -- */	\
	\
	__asm	movdqa	xmm0, xmm1			/* xmm0 = 67 64 66 65 -- -- -- -- */	\
	__asm	psrldq	xmm1, 12			/* xmm1 = -- -- -- -- -- -- 67 64 */	\
	\
	__asm	psrldq	xmm5, 6				/* xmm5 = -- -- -- -- -- -- 54 51 */	\
	__asm	psrldq	xmm3, 14			/* xmm3 = -- -- -- -- -- -- -- 34 */	\
	\
	__asm	pslldq	xmm7, 10			/* xmm7 = 77 75 74 -- -- -- -- -- */	\
	__asm	por		xmm4, xmm6			/* xmm4 = 76 73 -- -- -- -- 35 33 */	\
	\
	__asm	psrldq	xmm2, 10			/* xmm2 = -- -- -- -- -- 53 52 -- */	\
	__asm	pslldq	xmm0, 4				/* xmm0 = 66 65 -- -- -- -- -- -- */	\
	\
	__asm	pslldq	xmm1, 8				/* xmm1 = -- -- 67 64 -- -- -- -- */	\
	__asm	por		xmm3, xmm7			/* xmm3 = 77 75 74 -- -- -- -- 34 */	\
	\
	__asm	psrldq	xmm0, 6				/* xmm0 = -- -- -- 66 65 -- -- -- */	\
	__asm	pslldq	xmm5, 4				/* xmm5 = -- -- -- -- 54 51 -- -- */	\
	\
	__asm	por		xmm4, xmm1			/* xmm4 = 76 73 67 64 -- -- 35 33 */	\
	__asm	por		xmm3, xmm2			/* xmm3 = 77 75 74 -- -- 53 52 34 */	\
	\
	__asm	por		xmm4, xmm5			/* xmm4 = 76 73 67 64 54 51 35 33 */	\
	__asm	por		xmm3, xmm0			/* xmm3 = 77 75 74 66 65 53 52 34 */	\
	\
	__asm	movdqa	[eax+96], xmm4		/* write  76 73 67 64 54 51 35 33 */	\
	__asm	movdqa	[eax+112], xmm3		/* write  77 75 74 66 65 53 52 34 */	\
	\
	}/* end of Wmt_Dequant Macro */


/**************************************************************************************
 *
 *		Macro:			Wmt_Dequant_Dx
 *		
 *		Description:	The Macro does dequantzation 
 *
 *		Input:			[eax], quantized input, 
 *						[ebx], quantizaiton table,
 *
 *		Output:			[eax]
 *		
 *		Return:			None			
 *
 *		Special Note:	None
 *
 *		Error:			None
 *
 ***************************************************************************************
 */
#define Wmt_Dequant_Dx __asm {		\
	__asm	movdqa	xmm0, [eax]													\
	__asm	movdqa	xmm1, [eax + 16]											\
	\
	__asm	pmullw	xmm0, [ebx]			/* xmm0 = 07 06 05 04 03 02 01 00 */	\
	__asm	pmullw	xmm1, [ebx + 16]	/* xmm1 = 17 16 15 14 13 12 11 10 */	\
	\
	__asm	movdqa	xmm2, [eax + 32]											\
	__asm	movdqa	xmm3, [eax + 48]	/* xmm3 = 37 36 35 34 33 32 31 30 */	\
	\
	__asm	pmullw	xmm2, [ebx + 32]	/* xmm4 = 27 26 25 24 23 22 21 20 */	\
	__asm	pmullw	xmm3, [ebx + 48]											\
	\
	__asm	movdqa	[edx], xmm0			/* write  43 25 24 12 11 03 02 00 */	\
	__asm	movdqa	[edx + 16], xmm1	/* write  44 42 26 23 13 10 04 01 */	\
	\
	__asm	movdqa	xmm4, [eax + 64]											\
	__asm	movdqa	xmm5, [eax + 80]	/* xmm5 = 57 56 55 54 53 52 51 50 */	\
	\
	__asm	pmullw	xmm4, [ebx + 64]	/* xmm5	= 47 46 45 44 43 42 41 40 */	\
	__asm	pmullw	xmm5, [ebx + 80]											\
	\
	__asm	movdqa	[edx+32], xmm2		/* write  60 45 41 27 22 14 07 05 */	\
	__asm	movdqa	[edx+48], xmm3		/* write  61 57 46 40 30 21 15 06 */	\
	\
	__asm	movdqa	xmm6, [eax + 96]	/* xmm1 = 67 66 65 64 63 62 61 60 */	\
	__asm	movdqa	xmm7, [eax + 112]	/* xmm7 = 77 76 75 74 73 72 71 70 */	\
	\
	__asm	pmullw	xmm6, [ebx + 96]											\
	__asm	pmullw	xmm7, [ebx + 112]											\
	\
	__asm	movdqa	[edx+64], xmm4		/* write  71 62 56 47 37 31 20 16 */		\
	__asm	movdqa  [edx+80], xmm5		/* write  72 70 64 55 50 36 32 17 */	\
	\
	__asm	movdqa	[edx+96], xmm6		/* write  76 73 67 64 54 51 35 33 */	\
	__asm	movdqa	[edx+112], xmm7		/* write  77 75 74 66 65 53 52 34 */	\
	\
	}/* end of Wmt_Dequant Macro */




/**************************************************************************************
 *
 *		Routine:		Wmt_IDct_Dx
 *		
 *		Description:	Perform IDCT on a 8x8 block
 *
 *		Input:			Pointer to input and output buffer				
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	The input coefficients are in raster order
 *
 *		Error:			None
 *
 ***************************************************************************************
 */

void  Wmt_IDct_Dx(short *InputData, short *QuantizationTable, short *OutputData)
{

	
	__asm 
	{

        push    ebx

        mov		eax, InputData
		mov		ebx, QuantizationTable
		mov		edx, OutputData
		lea		ecx, WmtIdctConst
		
		Wmt_Dequant_Dx

#undef	I
#undef	O
#undef	C
#define I(i) [edx + 16 * i ]
#define O(i) [edx + 16 * i ]
#define C(i) [ecx + 16 * (i-1) ]

		
		/* Transpose - absorbed by the Wmt_dequant */

		Wmt_Row_IDCT

		Transpose
		
		Wmt_Column_IDCT

        pop     ebx
	}

}

/**************************************************************************************
 **************  Wmt_IDCT10_Dx   ******************************************************
 **************************************************************************************
 

	In IDCT10, we are dealing with only ten Non-Zero coefficients in the 8x8 block. 
	In the case that we work in the fashion RowIDCT -> ColumnIDCT, we only have to 
	do 1-D row idcts on the first four rows, the rest four rows remain zero anyway. 
	After row IDCTs, since every column could have nonzero coefficients, we need do
	eight 1-D column IDCT. However, for each column, there are at most two nonzero
	coefficients, coefficient 0 to coefficient 3. Same for the coefficents for the 
	two 1-d row idcts. For this reason, the process of a 1-D IDCT is simplified 
	
	from a full version:
	
	A = (C1 * I1) + (C7 * I7)		B = (C7 * I1) - (C1 * I7)
	C = (C3 * I3) + (C5 * I5)		D = (C3 * I5) - (C5 * I3)
	A. = C4 * (A - C)				B. = C4 * (B - D)
    C. = A + C						D. = B + D
   
    E = C4 * (I0 + I4)				F = C4 * (I0 - I4)
    G = (C2 * I2) + (C6 * I6)		H = (C6 * I2) - (C2 * I6)
    E. = E - G
    G. = E + G
   
    A.. = F + A.					B.. = B. - H
    F.  = F - A. 					H.  = B. + H
   
    R0 = G. + C.	R1 = A.. + H.	R3 = E. + D.	R5 = F. + B..
    R7 = G. - C.	R2 = A.. - H.	R4 = E. - D.	R6 = F. - B..


	To:

  	A = (C1 * I1)					B = (C7 * I1) 
	C = (C3 * I3)					D = - (C5 * I3)
	A. = C4 * (A - C)				B. = C4 * (B - D)
    C. = A + C						D. = B + D
   
    E = C4 * I0						F = E
    G = (C2 * I2)					H = (C6 * I2)
    E. = E - G
    G. = E + G
   
    A.. = F + A.					B.. = B. - H
    F.  = F - A. 					H.  = B. + H
   
    R0 = G. + C.	R1 = A.. + H.	R3 = E. + D.	R5 = F. + B..
    R7 = G. - C.	R2 = A.. - H.	R4 = E. - D.	R6 = F. - B..

	
******************************************************************************************/


/**************************************************************************************
 *
 *		Macro:			Wmt_Column_IDCT10
 *		
 *		Description:	The Macro does 1-D IDct on 8 columns. 
 *
 *		Input:			None
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	None
 *
 *		Error:			None
 *
 ***************************************************************************************
 */

/*	
	The major difference between Willamette processor and other IA32 processors is that 
	all of the simd integer instructions now support the 128 bit xmm registers instead 
	of 64 bit mmx registers. By using these instructions, we can do 8 1-D coloumn idcts 
	that takes shorts as input and outputs shorts at once

*/

#define Wmt_Column_IDCT10 __asm {		\
	\
	__asm	movdqa	xmm2, I(3)		/* xmm2 = i3 */				\
	__asm	movdqa	xmm6, C(3)		/* xmm6 = c3 */				\
	\
	__asm	movdqa	xmm4, xmm2		/* xmm4 = i3 */				\
	__asm	pmulhw  xmm4, xmm6		/* xmm4 = c3 * i3 - i3 */	\
	\
	__asm	movdqa  xmm1, C(5)		/* xmm1 = c5 */				\
	__asm	movdqa	xmm5, xmm1		/* xmm5 = c5 */				\
	\
	__asm	pmulhw	xmm1, xmm2		/* xmm1 = c5 * i3 - i3 */	\
	__asm	movdqa  xmm3, I(1)		/* xmm3 = i1 */				\
	\
	__asm	movdqa	xmm0, C(1)		/* xmm0 = c1 */				\
	__asm	paddw   xmm4, xmm2		/* xmm4 = c3 * i3 = C */	\
	\
	__asm	movdqa	xmm7, C(7)		/* xmm7 = c7 */				\
	\
	__asm	paddw	xmm2, xmm1		/* xmm2 = c5 * i3 */		\
	__asm	movdqa	xmm5, xmm0		/* xmm5 = c1 */				\
	\
	__asm	pmulhw	xmm0, xmm3		/* xmm0 = c1 * i1 - i1 */	\
	__asm	pxor	xmm6, xmm6		/* clear xmm6 */			\
	\
	__asm	psubsw	xmm6, xmm2		/* xmm6 = - c5 * i3 = D */	\
	__asm	paddw	xmm0, xmm3		/* xmm0 = c1 * i1 = A */	\
	\
	__asm	pmulhw	xmm3, xmm7		/* xmm3 = c7 * i1 = B */		\
	__asm	movdqa	xmm2, I(2)		/* xmm2 = i2 */				\
	\
	__asm	movdqa	xmm1, xmm2		/* xmm1 = i2 */				\
	__asm	pmulhw	xmm2, C(2)		/* xmm2 = i2 * c2 -i2 */	\
	\
	__asm	psubsw	xmm0, xmm4		/* xmm0 = A - C */			\
	\
	__asm	paddw	xmm2, xmm1		/* xmm2 = i2 * c2 */		\
	__asm	pmulhw	xmm1, C(6)		/* xmm1 = c6 * i2 */		\
	\
	__asm	paddsw	xmm4, xmm4		/* xmm4 = C + C */			\
	__asm	paddsw	xmm4, xmm0		/* xmm4 = A + C = C. */		\
	\
	__asm	psubsw	xmm3, xmm6		/* xmm3 = B - D */			\
	__asm	paddsw	xmm6, xmm6		/* xmm6 = D + D */			\
	\
	__asm	paddsw	xmm6, xmm3		/* xmm6 = B + D = D. */		\
	__asm	movdqa	I(1), xmm4		/* Save C. at I(1)	*/		\
	\
	__asm	movdqa	xmm4, C(4)		/* xmm4 = c4 */							\
	__asm	movdqa  xmm5, xmm3		/* xmm5 = B - D */						\
	\
	__asm	pmulhw	xmm3, xmm4		/* xmm3 = ( c4 -1 ) * ( B - D ) */		\
	\
	__asm	movdqa	xmm7, xmm2		/* xmm7 = c2 * i2 + c6 * i6 = G */		\
	__asm	movdqa	I(2), xmm6		/* Save D. at I(2) */		\
	\
	__asm	movdqa	xmm2, xmm0		/* xmm2 = A - C */			\
	__asm	movdqa	xmm6, I(0)		/* xmm6 = i0 */				\
	\
	__asm	pmulhw	xmm0, xmm4		/* xmm0 = ( c4 - 1 ) * ( A - C ) = A. */\
	__asm	paddw	xmm5, xmm3		/* xmm5 = c4 * ( B - D ) = B. */		\
	\
	__asm	psubsw	xmm5, xmm1		/* xmm5 = B. - H = B.. */	\
	__asm	paddw	xmm2, xmm0		/* xmm2 = c4 * ( A - C) = A. */			\
	\
	__asm	movdqa	xmm0, xmm6		/* xmm0 = i0 */					\
	__asm	pmulhw	xmm6, xmm4		/* xmm6 = (c4 - 1) * i0 = E = F */	\
	\
	__asm	paddsw	xmm1, xmm1		/* xmm1 = H + H */			\
	__asm	paddsw	xmm1, xmm5		/* xmm1 = B. + H = H. */	\
	\
	__asm	paddw	xmm6, xmm0		/* xmm6 = c4 *  i0  */			\
	__asm	movdqa	xmm4, xmm6		/* xmm4 = c4 *  i0 = E */		\
	\
	__asm	psubsw	xmm6, xmm2		/* xmm6 = F - A. = F. */	\
	__asm	paddsw	xmm2, xmm2		/* xmm2 = A. + A. */		\
	\
	__asm	movdqa	xmm0, I(1)		/* Load	C. from I(1) */		\
	__asm	paddsw	xmm2, xmm6		/* xmm2 = F + A. = A.. */	\
	\
	__asm	psubsw  xmm2, xmm1		/* xmm2 = A.. - H. = R2 */				\
	\
	__asm	paddsw	xmm2, Eight		/* Adjust R2 and R1 before shifting */	\
	__asm	paddsw  xmm1, xmm1		/* xmm1 = H. + H. */					\
	\
	__asm	paddsw  xmm1, xmm2		/* xmm1 = A.. + H. = R1 */	\
	__asm	psraw	xmm2, 4			/* xmm2 = op2 */			\
	\
	__asm	psubsw	xmm4, xmm7		/* xmm4 = E - G = E. */		\
	__asm	psraw	xmm1, 4			/* xmm1 = op1 */			\
	\
	__asm   movdqa	xmm3, I(2)		/* Load D. from I(2) */		\
	__asm	paddsw	xmm7, xmm7		/* xmm7 = G + G */			\
	\
	__asm	movdqa	O(2), xmm2		/* Write out op2 */			\
	__asm	paddsw  xmm7, xmm4		/* xmm7 = E + G = G. */		\
	\
	__asm	movdqa	O(1), xmm1		/* Write out op1 */			\
	__asm	psubsw  xmm4, xmm3		/* xmm4 = E. - D. = R4 */	\
	\
	__asm	paddsw	xmm4, Eight		/* Adjust R4 and R3 before shifting */	\
	__asm	paddsw  xmm3, xmm3		/* xmm3 = D. + D. */					\
	\
	__asm	paddsw	xmm3, xmm4		/* xmm3 = E. + D. = R3 */	\
	__asm	psraw	xmm4, 4			/* xmm4 = op4 */			\
	\
	__asm	psubsw	xmm6, xmm5		/* xmm6 = F. - B..= R6 */	\
	__asm	psraw	xmm3, 4			/* xmm3 = op3 */			\
	\
	__asm	paddsw	xmm6, Eight		/* Adjust R6 and R5 before shifting */	\
	__asm	paddsw	xmm5, xmm5		/* xmm5 = B.. + B.. */					\
	\
	__asm	paddsw	xmm5, xmm6		/* xmm5 = F. + B.. = R5 */	\
	__asm	psraw	xmm6, 4			/* xmm6 = op6 */			\
	\
	__asm	movdqa	O(4), xmm4		/* Write out op4 */			\
	__asm	psraw	xmm5, 4			/* xmm5 = op5 */			\
	\
	__asm 	movdqa	O(3), xmm3		/* Write out op3 */			\
	__asm	psubsw	xmm7, xmm0		/* xmm7 = G. - C. = R7 */	\
	\
	__asm	paddsw  xmm7, Eight		/* Adjust R7 and R0 before shifting */	\
	__asm	paddsw  xmm0, xmm0		/* xmm0 = C. + C. */					\
	\
	__asm	paddsw  xmm0, xmm7		/* xmm0 = G. + C. */		\
	__asm	psraw	xmm7, 4			/* xmm7 = op7 */			\
	\
	__asm	movdqa	O(6), xmm6		/* Write out op6 */			\
	__asm	psraw	xmm0, 4			/* xmm0 = op0 */			\
	\
	__asm	movdqa	O(5), xmm5		/* Write out op5 */			\
	__asm	movdqa	O(7), xmm7		/* Write out op7 */			\
	\
	__asm	movdqa	O(0), xmm0		/* Write out op0 */			\
	\
	} /* End of Wmt_Column_IDCT10 macro */


/**************************************************************************************
 *
 *		Macro:			Wmt_Row_IDCT10
 *		
 *		Description:	The Macro does 1-D IDct on 8 columns. 
 *
 *		Input:			None
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	None
 *
 *		Error:			None
 *
 ***************************************************************************************
 */

/*	
	The major difference between Willamette processor and other IA32 processors is that 
	all of the simd integer instructions now support the 128 bit xmm registers instead 
	of 64 bit mmx registers. By using these instructions, we can do 8 1-D coloumn idcts 
	that takes shorts as input and outputs shorts at once

*/

#define Wmt_Row_IDCT10 __asm {		\
	\
	__asm	movdqa	xmm2, I(3)		/* xmm2 = i3 */		\
	__asm	movdqa	xmm6, C(3)		/* xmm6 = c3 */		\
	\
	__asm	movdqa	xmm4, xmm2		/* xmm4 = i3 */		\
	__asm	pmulhw xmm4, xmm6		/* xmm4 = c3 * i3 - i3 */	\
	\
	__asm	movdqa  xmm1, C(5)		/* xmm1 = c5 */		\
	__asm	movdqa	xmm5, xmm1		/* xmm5 = c5 */		\
	\
	__asm	pmulhw	xmm1, xmm2		/* xmm1 = c5 * i3 - i3 */	\
	__asm	movdqa  xmm3, I(1)		/* xmm3 = i1 */		\
	\
	__asm	movdqa	xmm0, C(1)		/* xmm0 = c1 */		\
	__asm	paddw   xmm4, xmm2		/* xmm4 = c3 * i3 =C */	\
	\
	__asm	movdqa	xmm7, C(7)		/* xmm7 = c7 */				\
	\
	__asm	paddw	xmm2, xmm1		/* xmm2 = c5 * i3 */	\
	__asm	movdqa	xmm5, xmm0		/* xmm5 = c1 */		\
	\
	__asm	pmulhw	xmm0, xmm3		/* xmm0 = c1 * i1 - i1 */	\
	__asm	pxor	xmm6, xmm6		/* clear xmm6 */	\
	\
	__asm	psubsw	xmm6, xmm2		/* xmm6 = - c5 * i3 = D */		\
	__asm	paddw	xmm0, xmm3		/* xmm0 = c1 * i1 = A */	\
	\
	__asm	pmulhw	xmm3, xmm7		/* xmm3 = c7 * i1 = B */	\
	__asm	movdqa	xmm2, I(2)		/* xmm2 = i2 */		\
	\
	__asm	movdqa	xmm1, xmm2		/* xmm1 = i2 */		\
	__asm	pmulhw	xmm2, C(2)		/* xmm2 = i2 * c2 -i2 */	\
	\
	__asm	psubsw	xmm0, xmm4		/* xmm0 = A - C */	\
	\
	__asm	paddw	xmm2, xmm1		/* xmm2 = i2 * c2 = G */	\
	__asm	pmulhw	xmm1, C(6)		/* xmm1 = c6 * i2 = H */	\
	\
	__asm	paddsw	xmm4, xmm4		/* xmm4 = C + C */			\
	__asm	paddsw	xmm4, xmm0		/* xmm4 = A + C = C. */		\
	\
	__asm	psubsw	xmm3, xmm6		/* xmm3 = B - D */			\
	__asm	paddsw	xmm6, xmm6		/* xmm6 = D + D */			\
	\
	__asm	paddsw	xmm6, xmm3		/* xmm6 = B + D = D. */		\
	__asm	movdqa	I(1), xmm4		/* Save C. at I(1)	*/		\
	\
	__asm	movdqa	xmm4, C(4)		/* xmm4 = c4 */				\
	\
	__asm	movdqa  xmm5, xmm3		/* xmm5 = B - D */			\
	__asm	pmulhw	xmm3, xmm4		/* xmm3 = ( c4 -1 ) * ( B - D ) */		\
	\
	__asm	movdqa	xmm7, xmm2		/* xmm7 = c2 * i2 = G */				\
	__asm	movdqa	I(2), xmm6		/* Save D. at I(2) */					\
	\
	__asm	movdqa	xmm2, xmm0		/* xmm2 = A - C */	\
	__asm	movdqa	xmm6, I(0)		/* xmm6 = i0 */		\
	\
	__asm	pmulhw	xmm0, xmm4		/* xmm0 = ( c4 - 1 ) * ( A - C ) = A. */	\
	__asm	paddw	xmm5, xmm3		/* xmm5 = c4 * ( B - D ) = B. */			\
	\
	__asm	psubsw	xmm5, xmm1		/* xmm5 = B. - H = B.. */			\
	__asm	paddw	xmm2, xmm0		/* xmm2 = c4 * ( A - C) = A. */		\
	\
	__asm	movdqa	xmm0, xmm6		/* xmm0 = i0  */	\
	__asm	pmulhw	xmm6, xmm4		/* xmm6 = ( c4 - 1 ) *  i0 = E = F */	\
	\
	__asm	paddsw	xmm1, xmm1		/* xmm1 = H + H */			\
	__asm	paddsw	xmm1, xmm5		/* xmm1 = B. + H = H. */	\
	\
	__asm	paddw	xmm6, xmm0		/* xmm6 = c4 * i0  */	\
	__asm	movdqa	xmm4, xmm6		/* xmm4 = c4 * i0  */	\
	\
	__asm	psubsw	xmm6, xmm2		/* xmm6 = F - A. = F. */	\
	__asm	paddsw	xmm2, xmm2		/* xmm2 = A. + A. */	\
	\
	__asm	movdqa	xmm0, I(1)		/* Load	C. from I(1) */		\
	__asm	paddsw	xmm2, xmm6		/* xmm2 = F + A. = A.. */	\
	\
	__asm	psubsw  xmm2, xmm1		/* xmm2 = A.. - H. = R2 */	\
	\
	__asm	paddsw  xmm1, xmm1		/* xmm1 = H. + H. */	\
	__asm	paddsw  xmm1, xmm2		/* xmm1 = A.. + H. = R1 */	\
	\
	__asm	psubsw	xmm4, xmm7		/* xmm4 = E - G = E. */		\
	\
	__asm   movdqa	xmm3, I(2)		/* Load D. from I(2) */		\
	__asm	paddsw	xmm7, xmm7		/* xmm7 = G + G */	\
	\
	__asm	movdqa	I(2), xmm2		/* Write out op2 */		\
	__asm	paddsw  xmm7, xmm4		/* xmm7 = E + G = G. */		\
	\
	__asm	movdqa	I(1), xmm1		/* Write out op1 */		\
	__asm	psubsw  xmm4, xmm3		/* xmm4 = E. - D. = R4 */	\
	\
	__asm	paddsw  xmm3, xmm3		/* xmm3 = D. + D. */	\
	\
	__asm	paddsw	xmm3, xmm4		/* xmm3 = E. + D. = R3 */	\
	\
	__asm	psubsw	xmm6, xmm5		/* xmm6 = F. - B..= R6 */	\
	\
	__asm	paddsw	xmm5, xmm5		/* xmm5 = B.. + B.. */	\
	\
	__asm	paddsw	xmm5, xmm6		/* xmm5 = F. + B.. = R5 */	\
	\
	__asm	movdqa	I(4), xmm4		/* Write out op4 */		\
	\
	__asm 	movdqa	I(3), xmm3		/* Write out op3 */		\
	__asm	psubsw	xmm7, xmm0		/* xmm7 = G. - C. = R7 */	\
	\
	__asm	paddsw  xmm0, xmm0		/* xmm0 = C. + C. */	\
	\
	__asm	paddsw  xmm0, xmm7		/* xmm0 = G. + C. */	\
	\
	__asm	movdqa	I(6), xmm6		/* Write out op6 */		\
	\
	__asm	movdqa	I(5), xmm5		/* Write out op5 */		\
	__asm	movdqa	I(7), xmm7		/* Write out op7 */		\
	\
	__asm	movdqa	I(0), xmm0		/* Write out op0 */		\
	\
	} /* End of Wmt_Row_IDCT10 macro */

/**************************************************************************************
 *
 *		Macro:			Transpose
 *		
 *		Description:	The Macro does 8x8 transpose
 *
 *		Input:			None
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	None
 *
 *		Error:			None
 *
 ***************************************************************************************
 */


#define Transpose10 __asm {	\
	\
	__asm	movdqa		xmm4, I(4)		/* xmm4=e7e6e5e4e3e2e1e0 */	\
	__asm	movdqa		xmm0, I(5)		/* xmm4=f7f6f5f4f3f2f1f0 */	\
	\
	__asm	movdqa		xmm5, xmm4		/* make a copy */			\
	__asm	punpcklwd	xmm4, xmm0		/* xmm4=f3e3f2e2f1e1f0e0 */	\
	\
	__asm	punpckhwd	xmm5, xmm0		/* xmm5=f7e7f6e6f5e5f4e4 */	\
	__asm	movdqa		xmm6, I(6)		/* xmm6=g7g6g5g4g3g2g1g0 */ \
	\
	__asm	movdqa		xmm0, I(7)		/* xmm0=h7h6h5h4h3h2h1h0 */ \
	__asm	movdqa		xmm7, xmm6		/* make a copy */			\
	\
	__asm	punpcklwd	xmm6, xmm0		/* xmm6=h3g3h3g2h1g1h0g0 */ \
	__asm	punpckhwd	xmm7, xmm0		/* xmm7=h7g7h6g6h5g5h4g4 */ \
	\
	__asm	movdqa		xmm3, xmm4		/* make a copy */			\
	__asm	punpckldq	xmm4, xmm6		/* xmm4=h1g1f1e1h0g0f0e0 */	\
	\
	__asm	punpckhdq	xmm3, xmm6		/* xmm3=h3g3g3e3h2g2f2e2 */	\
	__asm	movdqa		I(6), xmm3		/* save h3g3g3e3h2g2f2e2 */	\
	/* Free xmm6 */ \
	__asm	movdqa		xmm6, xmm5		/* make a copy */			\
	__asm	punpckldq	xmm5, xmm7		/* xmm5=h5g5f5e5h4g4f4e4 */ \
	\
	__asm	punpckhdq	xmm6, xmm7		/* xmm6=h7g7f7e7h6g6f6e6 */ \
	__asm	movdqa		xmm0, I(0)		/* xmm0=a7a6a5a4a3a2a1a0 */	\
	/* Free xmm7 */ \
	__asm	movdqa		xmm1, I(1)		/* xmm1=b7b6b5b4b3b2b1b0 */	\
	__asm	movdqa		xmm7, xmm0		/* make a copy */			\
	\
	__asm	punpcklwd	xmm0, xmm1		/* xmm0=b3a3b2a2b1a1b0a0 */	\
	__asm	punpckhwd	xmm7, xmm1		/* xmm7=b7a7b6a6b5a5b4a4 */ \
	/* Free xmm1 */ \
	__asm	movdqa		xmm2, I(2)		/* xmm2=c7c6c5c4c3c2c1c0 */ \
	__asm	movdqa		xmm3, I(3)	    /* xmm3=d7d6d5d4d3d2d1d0 */ \
	\
	__asm	movdqa		xmm1, xmm2		/* make a copy */			\
	__asm	punpcklwd	xmm2, xmm3		/* xmm2=d3c3d2c2d1c1d0c0 */ \
	\
	__asm	punpckhwd	xmm1, xmm3		/* xmm1=d7c7d6c6d5c5d4c4 */ \
	__asm	movdqa		xmm3, xmm0		/* make a copy	*/			\
	\
	__asm	punpckldq	xmm0, xmm2		/* xmm0=d1c1b1a1d0c0b0a0 */ \
	__asm	punpckhdq	xmm3, xmm2		/* xmm3=d3c3b3a3d2c2b2a2 */ \
	/* Free xmm2 */ \
	__asm	movdqa		xmm2, xmm7		/* make a copy */			\
	__asm	punpckldq	xmm2, xmm1		/* xmm2=d5c5b5a5d4c4b4a4 */	\
	\
	__asm	punpckhdq	xmm7, xmm1		/* xmm7=d7c7b7a7d6c6b6a6 */ \
	__asm	movdqa		xmm1, xmm0		/* make a copy */			\
	\
	__asm	punpcklqdq	xmm0, xmm4		/* xmm0=h0g0f0e0d0c0b0a0 */	\
	__asm	punpckhqdq	xmm1, xmm4		/* xmm1=h1g1g1e1d1c1b1a1 */ \
	\
	__asm	movdqa		I(0), xmm0		/* save I(0) */				\
	__asm	movdqa		I(1), xmm1		/* save I(1) */				\
	\
	__asm	movdqa		xmm0, I(6)		/* load h3g3g3e3h2g2f2e2 */ \
	__asm	movdqa		xmm1, xmm3		/* make a copy */			\
	\
	__asm	punpcklqdq	xmm1, xmm0		/* xmm1=h2g2f2e2d2c2b2a2 */ \
	__asm	punpckhqdq	xmm3, xmm0		/* xmm3=h3g3f3e3d3c3b3a3 */	\
	\
	__asm	movdqa		xmm4, xmm2		/* make a copy */			\
	__asm	punpcklqdq	xmm4, xmm5		/* xmm4=h4g4f4e4d4c4b4a4 */	\
	\
	__asm	punpckhqdq	xmm2, xmm5		/* xmm2=h5g5f5e5d5c5b5a5 */	\
	__asm	movdqa		I(2), xmm1		/* save I(2) */				\
	\
	__asm	movdqa		I(3), xmm3		/* save I(3) */				\
	__asm	movdqa		I(4), xmm4		/* save I(4) */				\
	\
	__asm	movdqa		I(5), xmm2		/* save I(5) */				\
	__asm	movdqa		xmm5, xmm7		/* make a copy */			\
	\
	__asm	punpcklqdq	xmm5, xmm6		/* xmm5=h6g6f6e6d6c6b6a6 */	\
	__asm	punpckhqdq	xmm7, xmm6		/* xmm7=h7g7f7e7d7c7b7a7 */	\
	\
	__asm	movdqa		I(6), xmm5		/* save I(6) */				\
	__asm	movdqa		I(7), xmm7		/* save I(7) */				\
	\
	}/* End of Transpose10 Macro */


/**************************************************************************************
 *
 *		Macro:			Wmt_Dequant10_Dx
 *		
 *		Description:	The Macro does dequantzation 
 *
 *		Input:			[eax], quantized input, 
 *						[ebx], quantizaiton table,
 *
 *		Output:			[eax]
 *		
 *		Return:			None			
 *
 *		Special Note:	None
 *
 *		Error:			None
 *
 ***************************************************************************************
 */
#define Wmt_Dequant10_Dx __asm {		\
	__asm	movdqa	xmm0, [eax]													\
	__asm	movdqa	xmm1, [eax + 16]											\
	\
	__asm	pmullw	xmm0, [ebx]			/* xmm0 = 07 06 05 04 03 02 01 00 */	\
	__asm	pmullw	xmm1, [ebx + 16]	/* xmm1 = 17 16 15 14 13 12 11 10 */	\
	\
	__asm	movdqa	xmm2, [eax + 32]											\
	__asm	movdqa	xmm3, [eax + 48]	/* xmm3 = 37 36 35 34 33 32 31 30 */	\
	\
	__asm	pmullw	xmm2, [ebx + 32]	/* xmm2 = 27 26 25 24 23 22 21 20 */	\
	__asm	pmullw	xmm3, [ebx + 48]											\
	\
	__asm	movdqa	[edx], xmm0			/* write  */	\
	__asm	movdqa	[edx + 16], xmm1	/* write  */	\
	\
	__asm	movdqa	[edx+32], xmm2		/* write  */	\
	__asm	movdqa	[edx+48], xmm3		/* write  */	\
	\
	}/* end of Wmt_Dequant10_Dx Macro */




/**************************************************************************************
 *
 *		Routine:		Wmt_IDct10_Dx
 *		
 *		Description:	Perform IDCT on a 8x8 block where only the first 10 coeffs are 
 *						non-zero coefficients.
 *
 *		Input:			Pointer to input and output buffer				
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	The input coefficients are in raster order
 *
 *		Error:			None
 *
 ***************************************************************************************
 */
void  Wmt_IDct10_Dx(short *InputData, short *QuantizationTable, short *OutputData)
{

	
	__asm 
	{
        push    ebx

		mov		eax, InputData
		mov		ebx, QuantizationTable
		mov		edx, OutputData
		lea		ecx, WmtIdctConst
		
		Wmt_Dequant10_Dx

#define I(i) [edx + 16 * i ]
#define O(i) [edx + 16 * i ]
#define C(i) [ecx + 16 * (i-1) ]

		
		/* Transpose - absorbed by the Wmt_dequant */

		Wmt_Row_IDCT10

		Transpose10
		
		Wmt_Column_IDCT10

        pop     ebx
	}

}
/**************************************************************************************
 *
 *		Routine:		Wmt_IDct1
 *		
 *		Description:	Perform IDCT on a 8x8 block where only the first 1 coeff
 *
 *		Input:			Pointer to input and output buffer				
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	We only have one coefficient
 *
 *		Error:			None
 *
 ***************************************************************************************
 */

void Wmt_idct1 (short * input, short * qtbl, short * output) 
{
    __asm
    {
        mov         eax,    [input]
        mov         edx,    0xf

        movd        xmm2,   edx

        mov         ecx,    [qtbl]
        mov         edx,    [output]
        
        movq        xmm0,   QWORD ptr [eax]
        movq        xmm1,   QWORD ptr [ecx]

        pmullw      xmm0,   xmm1;
        paddw       xmm0,   xmm2

        psraw       xmm0,   5;        
        punpcklwd   xmm0,   xmm0;
        
        punpckldq   xmm0,   xmm0;
        punpcklqdq  xmm0,   xmm0;

        movdqa      xmm1,   xmm0
        
        movdqa      [edx],  xmm0;        
        movdqa      [edx+16], xmm1;

        movdqa      [edx+32],  xmm0;        
        movdqa      [edx+48], xmm1;

        movdqa      [edx+64],  xmm0;        
        movdqa      [edx+80], xmm1;
        
        movdqa      [edx+96],  xmm0;        
        movdqa      [edx+112], xmm1;

    }
}
/**************************************************************************************
 **************  Wmt_IDCT3       ******************************************************
 **************************************************************************************
 */

/**************************************************************************************
 *
 *		Routine:		Wmt_IDCT3
 *		
 *		Description:	Perform IDCT on a 8x8 block with at most 3 nonzero coefficients
 *
 *		Input:			Pointer to input and output buffer				
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	Intel Compiler, Please
 *
 *		Error:			None
 *
 ***************************************************************************************
 */

/***************************************************************************************
	In IDCT 3, we are dealing with only three Non-Zero coefficients in the 8x8 block. 
	In the case that we work in the fashion RowIDCT -> ColumnIDCT, we only have to 
	do 1-D row idcts on the first two rows, the rest six rows remain zero anyway. 
	After row IDCTs, since every column could have nonzero coefficients, we need do
	eight 1-D column IDCT. However, for each column, there are at most two nonzero
	coefficients, coefficient 0 and coefficient 1. Same for the coefficents for the 
	two 1-d row idcts. For this reason, the process of a 1-D IDCT is simplified 
	
	from a full version:
	
	A = (C1 * I1) + (C7 * I7)		B = (C7 * I1) - (C1 * I7)
	C = (C3 * I3) + (C5 * I5)		D = (C3 * I5) - (C5 * I3)
	A. = C4 * (A - C)				B. = C4 * (B - D)
    C. = A + C						D. = B + D
   
    E = C4 * (I0 + I4)				F = C4 * (I0 - I4)
    G = (C2 * I2) + (C6 * I6)		H = (C6 * I2) - (C2 * I6)
    E. = E - G
    G. = E + G
   
    A.. = F + A.					B.. = B. - H
    F.  = F - A. 					H.  = B. + H
   
    R0 = G. + C.	R1 = A.. + H.	R3 = E. + D.	R5 = F. + B..
    R7 = G. - C.	R2 = A.. - H.	R4 = E. - D.	R6 = F. - B..

	To:


	A = (C1 * I1)					B = (C7 * I1)
	C = 0							D = 0
	A. = C4 * A 					B. = C4 * B 
    C. = A							D. = B 
   
    E = C4 * I0 					F = E
    G = 0							H = 0
    E. = E 
    G. = E 

    A.. = E + A.					B.. = B. 
    F.  = E - A. 					H.  = B. 
   
    R0 = E + A		R1 = E + A. + B.	R3 = E + B		R5 = E - A. + B.
    R7 = E - A		R2 = E + A. - B.	R4 = E - B		R6 = F - A. - B.
	
******************************************************************************************/
