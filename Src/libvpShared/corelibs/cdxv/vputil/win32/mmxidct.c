/****************************************************************************
*
*   Module Title :     IDCTPart.c
*
*   Description  :     IDCT with multiple versions based on # of non 0 coeffs
*
*   AUTHOR       :     Scott Lavarnway, Tim Murphy
*
*****************************************************************************
*   Revision History
*	
*   1.02 JBB 15 Nov 00 Cleaned out unused ifdefs
*	1.01 YWX  15/05/00  Added MMX_idct3 for use in PostProcesser
*   1.00 YWX  14/05/00  Configuration baseline from Scott Lavarnway
*
*****************************************************************************
*/

// Dequantization + inverse discrete cosine transform.
// Timothy S. Murphy   14 July 1999.

#pragma warning(disable:4005)
#include "codec_common.h"
#include <math.h>
#include <memory.h>
#undef PI
#define	PI		3.14159265358979323846


// Constants used in MMX implementation of dequantization and idct.
// All the MMX stuff works with 4 16-bit quantities at a time and
// we create 11 constants of size 4 x 16 bits.
// The first 4 are used to mask the individual 16-bit words within a group
// and are used in the address-shuffling part of the dequantization.
// The last 7 are fixed-point approximations to the cosines of angles
// occurring in the DCT; each of these contains 4 copies of the same value.

// There is only one (statically initialized) instance of this object
// wrapped in an allocator object that forces its starting address
// to be evenly divisible by 32.  Hence the actual object occupies 2.75
// cache lines on a Pentium processor.

// Offsets in bytes used by the assembler code below
// must of course agree with the idctConstants constructor.

#define MaskOffset 0		// 4 masks come in order low word to high
#define CosineOffset 32		// 7 cosines come in order pi/16 * (1 ... 7)
#define EightOffset 88
#define IdctAdjustBeforeShift 8
#pragma warning( disable : 4799 )  // Disable no emms instruction warning!

UINT16 idctconstants[(4+7+1) * 4];
UINT16 idctcosTbl[ 7] = 
{
	64277, 60547, 54491, 46341, 36410, 25080, 12785
};


/* Dequantization + inverse DCT.

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

   We fold the first transpose into the storage of the expanded coefficients.
   We don't actually do a full transpose because this would require doubling
   the size of the idct buffer; rather, we just transpose each of the 4x4
   subblocks.  Using slightly varying addressing schemes in each of the
   four 4x8 idcts then allows these transforms to be done in place.

   Transposing the 4x4 subblocks in the matrix above gives

	00 02 03 11  16 20 31 37
	01 04 10 13  17 32 36 50
	05 07 14 22  33 35 51 54
	06 15 21 30  34 52 53 65

	12 24 25 43  47 56 62 71
	23 26 42 44  55 63 70 72
	27 41 45 60  64 67 73 76
	40 46 57 61  66 74 75 77

   Finally, we reverse the words in each 4 word group to clarify
   direction of shifts.

	11 03 02 00  37 31 20 16
	13 10 04 01  50 36 32 17
	22 14 07 05	 54 51 35 33
	30 21 15 06	 65 53 52 34

	43 25 24 12	 71 62 56 47
	44 42 26 23  72 70 63 55
	60 45 41 27	 76 73 67 64
	61 57 46 40  77 75 74 66

   This matrix then shows the 16 4x16 destination words in terms of
   the 16 4x16 input words.

   We implement this algorithm by manipulation of mmx registers,
   which seems to be the fastest way to proceed.  It is completely
   hand-written; there does not seem to be enough recurrence to
   reasonably compartmentalize any of it.  Hence the resulting
   program is ugly and bloated.  Furthermore, due to the absence of
   register pressure, it is boring and artless.	 I hate it.

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
   Since MMX only provides a signed multiply, C1 ... C5 appear to be
   negative and multiplies involving them must be adjusted to compensate
   for this.  C6 and C7 do not require this adjustment since
   they are < 1/2 and are correctly treated as positive numbers.

   Following macro does four 8-sample one-dimensional idcts in parallel.
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
*/

/**************************************************************************************
 *
 *		Routine:		BeginIDCT
 *		
 *		Description:	The Macro does IDct on 4 1-D Dcts
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

#define Dump	__asm  call MMX_dump

#define BeginIDCT __asm { \
	\
	__asm	movq		r2, I(3)  \
	 \
	__asm	movq		r6, C(3) \
	__asm	 movq		r4, r2 \
	__asm	movq		r7, J(5) \
	__asm	 pmulhw		r4, r6		/* r4 = c3*i3 - i3 */ \
	__asm	movq		r1, C(5) \
	__asm	 pmulhw		r6, r7		/* r6 = c3*i5 - i5 */ \
	__asm	movq		r5, r1 \
	__asm	 pmulhw		r1, r2		/* r1 = c5*i3 - i3 */ \
	__asm	movq		r3, I(1) \
	__asm	 pmulhw		r5, r7		/* r5 = c5*i5 - i5 */ \
	__asm	movq		r0, C(1)	/* (all registers are in use) */ \
	__asm	 paddw		r4, r2		/* r4 = c3*i3 */ \
	__asm	paddw		r6, r7		/* r6 = c3*i5 */ \
	__asm	 paddw		r2, r1		/* r2 = c5*i3 */ \
	__asm	movq		r1, J(7) \
	__asm	 paddw		r7, r5		/* r7 = c5*i5 */ \
	__asm	movq		r5, r0		/* r5 = c1 */ \
	__asm	 pmulhw		r0, r3		/* r0 = c1*i1 - i1 */ \
	__asm	paddsw		r4, r7		/* r4 = C = c3*i3 + c5*i5 */ \
	__asm	 pmulhw		r5, r1		/* r5 = c1*i7 - i7 */ \
	__asm	movq		r7, C(7) \
	__asm	 psubsw		r6, r2		/* r6 = D = c3*i5 - c5*i3  (done w/r2) */ \
	__asm	paddw		r0, r3		/* r0 = c1*i1 */ \
	__asm	 pmulhw		r3, r7		/* r3 = c7*i1 */ \
	__asm	movq		r2, I(2) \
	__asm	 pmulhw		r7, r1		/* r7 = c7*i7 */ \
	__asm	paddw		r5, r1		/* r5 = c1*i7 */ \
	__asm	 movq		r1, r2		/* r1 = i2 */ \
	__asm	pmulhw		r2, C(2)	/* r2 = c2*i2 - i2 */ \
	__asm	 psubsw		r3, r5		/* r3 = B = c7*i1 - c1*i7 */ \
	__asm	movq		r5, J(6) \
	__asm	 paddsw		r0, r7		/* r0 = A = c1*i1 + c7*i7 */ \
	__asm	movq		r7, r5		/* r7 = i6 */ \
	__asm	 psubsw		r0, r4		/* r0 = A - C */ \
	__asm	pmulhw		r5, C(2)	/* r5 = c2*i6 - i6 */ \
	__asm	 paddw		r2, r1		/* r2 = c2*i2 */ \
	__asm	pmulhw		r1, C(6)	/* r1 = c6*i2 */ \
	__asm	 paddsw		r4, r4		/* r4 = C + C */ \
	__asm	paddsw		r4, r0		/* r4 = C. = A + C */ \
	__asm	 psubsw		r3, r6		/* r3 = B - D */ \
	__asm	paddw		r5, r7		/* r5 = c2*i6 */ \
	__asm	 paddsw		r6, r6		/* r6 = D + D */ \
	__asm	pmulhw		r7, C(6)	/* r7 = c6*i6 */ \
	__asm	 paddsw		r6, r3		/* r6 = D. = B + D */ \
	__asm	movq		I(1), r4	/* save C. at I(1) */ \
	__asm	 psubsw		r1, r5		/* r1 = H = c6*i2 - c2*i6 */ \
	__asm	movq		r4, C(4) \
	__asm	 movq		r5, r3		/* r5 = B - D */ \
	__asm	pmulhw		r3, r4		/* r3 = (c4 - 1) * (B - D) */ \
	__asm	 paddsw		r7, r2		/* r7 = G = c6*i6 + c2*i2 */ \
	__asm	movq		I(2), r6	/* save D. at I(2) */ \
	__asm	 movq		r2, r0		/* r2 = A - C */ \
	__asm	movq		r6, I(0) \
	__asm	 pmulhw		r0, r4		/* r0 = (c4 - 1) * (A - C) */ \
	__asm	paddw		r5, r3		/* r5 = B. = c4 * (B - D) */ \
	 \
	__asm	movq		r3, J(4) \
	__asm	 psubsw		r5, r1		/* r5 = B.. = B. - H */ \
	__asm	paddw		r2, r0		/* r0 = A. = c4 * (A - C) */ \
	__asm	 psubsw		r6, r3		/* r6 = i0 - i4 */ \
	__asm	movq		r0, r6 \
	__asm	 pmulhw		r6, r4		/* r6 = (c4 - 1) * (i0 - i4) */ \
	__asm	paddsw		r3, r3		/* r3 = i4 + i4 */ \
	__asm	 paddsw		r1, r1		/* r1 = H + H */ \
	__asm	paddsw		r3, r0		/* r3 = i0 + i4 */ \
	__asm	 paddsw		r1, r5		/* r1 = H. = B + H */ \
	__asm	pmulhw		r4, r3		/* r4 = (c4 - 1) * (i0 + i4) */ \
	__asm	 paddsw		r6, r0		/* r6 = F = c4 * (i0 - i4) */ \
	__asm	psubsw		r6, r2		/* r6 = F. = F - A. */ \
	__asm	 paddsw		r2, r2		/* r2 = A. + A. */ \
	__asm	movq		r0, I(1)	/* r0 = C. */ \
	__asm	 paddsw		r2, r6		/* r2 = A.. = F + A. */ \
	__asm	paddw		r4, r3		/* r4 = E = c4 * (i0 + i4) */ \
	__asm	 psubsw		r2, r1		/* r2 = R2 = A.. - H. */ \
}
// end BeginIDCT macro (38 cycles).


// Two versions of the end of the idct depending on whether we're feeding
// into a transpose or dividing the final results by 16 and storing them.

/**************************************************************************************
 *
 *		Routine:		RowIDCT
 *		
 *		Description:	The Macro does 1-D IDct on 4 Rows
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

// RowIDCT gets ready to transpose.

#define RowIDCT __asm { \
	\
	BeginIDCT \
	\
	__asm	movq		r3, I(2)	/* r3 = D. */ \
	__asm	 psubsw		r4, r7		/* r4 = E. = E - G */ \
	__asm	paddsw		r1, r1		/* r1 = H. + H. */ \
	__asm	 paddsw		r7, r7		/* r7 = G + G */ \
	__asm	paddsw		r1, r2		/* r1 = R1 = A.. + H. */ \
	__asm	 paddsw		r7, r4		/* r7 = G. = E + G */ \
	__asm	psubsw		r4, r3		/* r4 = R4 = E. - D. */ \
	__asm	 paddsw		r3, r3 \
	__asm	psubsw		r6, r5		/* r6 = R6 = F. - B.. */ \
	__asm	 paddsw		r5, r5 \
	__asm	paddsw		r3, r4		/* r3 = R3 = E. + D. */ \
	__asm	 paddsw		r5, r6		/* r5 = R5 = F. + B.. */ \
	__asm	psubsw		r7, r0		/* r7 = R7 = G. - C. */ \
	__asm	 paddsw		r0, r0 \
	__asm	movq		I(1), r1	/* save R1 */ \
	__asm	 paddsw		r0, r7		/* r0 = R0 = G. + C. */ \
}
// end RowIDCT macro (8 + 38 = 46 cycles)


/**************************************************************************************
 *
 *		Routine:		ColumnIDCT
 *		
 *		Description:	The Macro does 1-D IDct on 4 columns
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
// Column IDCT normalizes and stores final results.

#define ColumnIDCT __asm { \
	\
	BeginIDCT \
	\
	__asm	paddsw		r2, Eight	/* adjust R2 (and R1) for shift */ \
	__asm	 paddsw		r1, r1		/* r1 = H. + H. */ \
	__asm	paddsw		r1, r2		/* r1 = R1 = A.. + H. */ \
	__asm	 psraw		r2, 4		/* r2 = NR2 */ \
	__asm	psubsw		r4, r7		/* r4 = E. = E - G */ \
	__asm	 psraw		r1, 4		/* r1 = NR1 */ \
	__asm	movq		r3, I(2)	/* r3 = D. */ \
	__asm	 paddsw		r7, r7		/* r7 = G + G */ \
	__asm	movq		I(2), r2	/* store NR2 at I2 */ \
	__asm	 paddsw		r7, r4		/* r7 = G. = E + G */ \
	__asm	movq		I(1), r1	/* store NR1 at I1 */ \
	__asm	 psubsw		r4, r3		/* r4 = R4 = E. - D. */ \
	__asm	paddsw		r4, Eight	/* adjust R4 (and R3) for shift */ \
	__asm	 paddsw		r3, r3		/* r3 = D. + D. */ \
	__asm	paddsw		r3, r4		/* r3 = R3 = E. + D. */ \
	__asm	 psraw		r4, 4		/* r4 = NR4 */ \
	__asm	psubsw		r6, r5		/* r6 = R6 = F. - B.. */ \
	__asm	 psraw		r3, 4		/* r3 = NR3 */ \
	__asm	paddsw		r6, Eight	/* adjust R6 (and R5) for shift */ \
	__asm	 paddsw		r5, r5		/* r5 = B.. + B.. */ \
	__asm	paddsw		r5, r6		/* r5 = R5 = F. + B.. */ \
	__asm	 psraw		r6, 4		/* r6 = NR6 */ \
	__asm	movq		J(4), r4	/* store NR4 at J4 */ \
	__asm	 psraw		r5, 4		/* r5 = NR5 */ \
	__asm	movq		I(3), r3	/* store NR3 at I3 */ \
	__asm	 psubsw		r7, r0		/* r7 = R7 = G. - C. */ \
	__asm	paddsw		r7, Eight	/* adjust R7 (and R0) for shift */ \
	__asm	 paddsw		r0, r0 		/* r0 = C. + C. */ \
	__asm	paddsw		r0, r7		/* r0 = R0 = G. + C. */ \
	__asm	 psraw		r7, 4		/* r7 = NR7 */ \
	__asm	movq		J(6), r6	/* store NR6 at J6 */ \
	__asm	 psraw		r0, 4		/* r0 = NR0 */ \
	__asm	movq		J(5), r5	/* store NR5 at J5 */ \
	 \
	__asm	movq		J(7), r7	/* store NR7 at J7 */ \
	 \
	__asm	movq		I(0), r0	/* store NR0 at I0 */ \
	 \
}
// end ColumnIDCT macro (38 + 19 = 57 cycles)

/**************************************************************************************
 *
 *		Routine:		Transpose
 *		
 *		Description:	The Macro does two 4x4 transposes in place.
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

/* Following macro does two 4x4 transposes in place.

  At entry (we assume):

	r0 = a3 a2 a1 a0
	I(1) = b3 b2 b1 b0
	r2 = c3 c2 c1 c0
	r3 = d3 d2 d1 d0

	r4 = e3 e2 e1 e0
	r5 = f3 f2 f1 f0
	r6 = g3 g2 g1 g0
	r7 = h3 h2 h1 h0

   At exit, we have:

	I(0) = d0 c0 b0 a0
	I(1) = d1 c1 b1 a1
	I(2) = d2 c2 b2 a2
	I(3) = d3 c3 b3 a3
	
	J(4) = h0 g0 f0 e0
	J(5) = h1 g1 f1 e1
	J(6) = h2 g2 f2 e2
	J(7) = h3 g3 f3 e3

   I(0) I(1) I(2) I(3)  is the transpose of r0 I(1) r2 r3.
   J(4) J(5) J(6) J(7)  is the transpose of r4 r5 r6 r7.

   Since r1 is free at entry, we calculate the Js first. */


#define Transpose __asm { \
	\
	__asm	movq		r1, r4			/* r1 = e3 e2 e1 e0 */ \
	__asm	 punpcklwd	r4, r5			/* r4 = f1 e1 f0 e0 */ \
	__asm	movq		I(0), r0		/* save a3 a2 a1 a0 */ \
	__asm	 punpckhwd	r1, r5			/* r1 = f3 e3 f2 e2 */ \
	__asm	movq		r0, r6			/* r0 = g3 g2 g1 g0 */ \
	__asm	 punpcklwd	r6, r7			/* r6 = h1 g1 h0 g0 */ \
	__asm	movq		r5, r4			/* r5 = f1 e1 f0 e0 */ \
	__asm	 punpckldq	r4, r6			/* r4 = h0 g0 f0 e0 = R4 */ \
	__asm	punpckhdq	r5, r6			/* r5 = h1 g1 f1 e1 = R5 */ \
	__asm	 movq		r6, r1			/* r6 = f3 e3 f2 e2 */ \
	__asm	movq		J(4), r4 \
	__asm	 punpckhwd	r0, r7			/* r0 = h3 g3 h2 g2 */ \
	__asm	movq		J(5), r5 \
	__asm	 punpckhdq	r6, r0			/* r6 = h3 g3 f3 e3 = R7 */ \
	__asm	movq		r4, I(0)		/* r4 = a3 a2 a1 a0 */ \
	__asm	 punpckldq	r1, r0			/* r1 = h2 g2 f2 e2 = R6 */ \
	__asm	movq		r5, I(1)		/* r5 = b3 b2 b1 b0 */ \
	__asm	 movq		r0, r4			/* r0 = a3 a2 a1 a0 */ \
	__asm	movq		J(7), r6 \
	__asm	 punpcklwd	r0, r5			/* r0 = b1 a1 b0 a0 */ \
	__asm	movq		J(6), r1 \
	__asm	 punpckhwd	r4, r5			/* r4 = b3 a3 b2 a2 */ \
	__asm	movq		r5, r2			/* r5 = c3 c2 c1 c0 */ \
	__asm	 punpcklwd	r2, r3			/* r2 = d1 c1 d0 c0 */ \
	__asm	movq		r1, r0			/* r1 = b1 a1 b0 a0 */ \
	__asm	 punpckldq	r0, r2			/* r0 = d0 c0 b0 a0 = R0 */ \
	__asm	punpckhdq	r1, r2			/* r1 = d1 c1 b1 a1 = R1 */ \
	__asm	 movq		r2, r4			/* r2 = b3 a3 b2 a2 */ \
	__asm	movq		I(0), r0 \
	__asm	 punpckhwd	r5, r3			/* r5 = d3 c3 d2 c2 */ \
	__asm	movq		I(1), r1 \
	__asm	 punpckhdq	r4, r5			/* r4 = d3 c3 b3 a3 = R3 */ \
	__asm	punpckldq	r2, r5			/* r2 = d2 c2 b2 a2 = R2 */ \
	 \
	__asm	movq		I(3), r4 \
	 \
	__asm	movq		I(2), r2 \
	 \
}
// end Transpose macro (19 cycles).

/*
__declspec( naked) static void MMX_dump() 
{
	__asm 
	{
		movq	[edi],	mm0
		movq	[edi+8], mm1
		movq	[edi+16], mm2
		movq	[edi+24], mm3
		movq	[edi+32], mm4
		movq	[edi+40], mm5
		movq	[edi+48], mm6
		movq	[edi+56], mm7
		ret
	}
}
*/
/**************************************************************************************
 *
 *		Routine:		MMX_idct
 *		
 *		Description:	Perform IDCT on a 8x8 block
 *
 *		Input:			Pointer to input and output buffer				
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	The input coefficients are in ZigZag order
 *
 *		Error:			None
 *
 ***************************************************************************************
 */
__declspec ( naked ) void MMX_idct (	INT16 * input, INT16 * qtbl, INT16 * output) 
{

//	uINT16 *constants = idctconstants;
#	define M(I)		[ecx + MaskOffset + I*8]
#	define C(I)		[ecx + CosineOffset + (I-1)*8]
#	define Eight	[ecx + EightOffset]
#   undef Arg
#	define Arg(I)	[esp + 1*4 + 3*4 + I*4] // 1 return address + 3 pushes prior to args

#	define r0	mm0
#	define r1	mm1
#	define r2	mm2
#	define r3	mm3
#	define r4	mm4
#	define r5	mm5
#	define r6	mm6
#	define r7	mm7
	(void) output;
	(void) qtbl;
	(void) input;


	__asm {

	push	edx
	push	ecx
	push	ebx

;; Label:
	mov		eax, Arg( 0)	; eax = quantized input
	 mov	edx, Arg( 2)	; edx = destination (= idct buffer)

	mov		ecx, [edx]		; (+1 at least) preload the cache before writing
	 mov	ebx, [edx+28]   ; in case proc doesn't cache on writes
	mov		ecx, [edx+56]	; gets all the cache lines
	 mov	ebx, [edx+84]	; regardless of alignment (beyond 32-bit)
	mov		ecx, [edx+112]	; also avoids address contention stalls
	 mov	ebx, [edx+124]

	mov		ebx, Arg( 1)	; ebx = quantization table
	 lea    ecx, idctconstants ;;[0];

	movq	r0, [eax]
	 ;
	pmullw	r0, [ebx]		; r0 = 03 02 01 00
	 ;
	movq	r1, [eax+16]
	 ;
	pmullw	r1, [ebx+16]	; r1 = 13 12 11 10
	 ;
	movq	r2, M(0)		; r2 = __ __ __ FF
	 movq	r3, r0			; r3 = 03 02 01 00
	movq	r4, [eax+8]
	 psrlq	r0, 16			; r0 = __ 03 02 01
	pmullw	r4, [ebx+8]		; r4 = 07 06 05 04
	 pand	r3, r2			; r3 = __ __ __ 00
	movq	r5, r0			; r5 = __ 03 02 01
	 movq	r6, r1			; r6 = 13 12 11 10
	pand	r5, r2			; r5 = __ __ __ 01
	 psllq	r6, 32			; r6 = 11 10 __ __
	movq	r7, M(3)		; r7 = FF __ __ __
	 pxor	r0, r5			; r0 = __ 03 02 __
	pand	r7, r6			; r7 = 11 __ __ __
	 por	r0, r3			; r0 = __ 03 02 00
	pxor	r6, r7			; r6 = __ 10 __ __
	 por	r0, r7			; r0 = 11 03 02 00 = R0
	movq	r7, M(3)		; r7 = FF __ __ __
	 movq	r3, r4			; r3 = 07 06 05 04
	movq	[edx], r0		; write R0 = r0
	 pand	r3, r2			; r3 = __ __ __ 04
	movq	r0, [eax+32]
	 psllq	r3, 16			; r3 = __ __ 04 __
	pmullw	r0, [ebx+32]	; r0 = 23 22 21 20
	 pand	r7, r1			; r7 = 13 __ __ __
	por		r5, r3			; r5 = __ __ 04 01
	 por	r7, r6			; r7 = 13 10 __ __
	movq	r3, [eax+24]
	 por	r7, r5			; r7 = 13 10 04 01 = R1
	pmullw	r3, [ebx+24]	; r3 = 17 16 15 14
	 psrlq	r4, 16			; r4 = __ 07 06 05
	movq	[edx+16], r7	; write R1 = r7
	 movq	r5, r4			; r5 = __ 07 06 05
	movq	r7, r0			; r7 = 23 22 21 20
	 psrlq	r4, 16			; r4 = __ __ 07 06
	psrlq	r7, 48			; r7 = __ __ __ 23
	 movq	r6, r2			; r6 = __ __ __ FF
	pand	r5, r2			; r5 = __ __ __ 05
	 pand	r6, r4			; r6 = __ __ __ 06
	movq	[edx+80], r7	; partial R9 = __ __ __ 23
	 pxor	r4, r6			; r4 = __ __ 07 __
	psrlq	r1, 32			; r1 = __ __ 13 12
	 por	r4, r5			; r4 = __ __ 07 05
	movq	r7, M(3)		; r7 = FF __ __ __
	 pand	r1, r2			; r1 = __ __ __ 12
	movq	r5, [eax+48]
	 psllq	r0, 16			; r0 = 22 21 20 __
	pmullw	r5, [ebx+48]	; r5 = 33 32 31 30
	 pand	r7, r0			; r7 = 22 __ __ __
	movq	[edx+64], r1	; partial R8 = __ __ __ 12
	 por	r7, r4			; r7 = 22 __ 07 05
	movq	r4, r3			; r4 = 17 16 15 14
	 pand	r3, r2			; r3 = __ __ __ 14
	movq	r1, M(2)		; r1 = __ FF __ __
	 psllq	r3, 32			; r3 = __ 14 __ __
	por		r7, r3			; r7 = 22 14 07 05 = R2
	 movq	r3, r5			; r3 = 33 32 31 30
	psllq	r3, 48			; r3 = 30 __ __ __
	 pand	r1, r0			; r1 = __ 21 __ __
	movq	[edx+32], r7	; write R2 = r7
	 por	r6, r3			; r6 = 30 __ __ 06
	movq	r7, M(1)		; r7 = __ __ FF __
	 por	r6, r1			; r6 = 30 21 __ 06
	movq	r1, [eax+56]
	 pand	r7, r4			; r7 = __ __ 15 __
	pmullw	r1, [ebx+56]	; r1 = 37 36 35 34
	 por	r7, r6			; r7 = 30 21 15 06 = R3
	pand	r0, M(1)		; r0 = __ __ 20 __
	 psrlq	r4, 32			; r4 = __ __ 17 16
	movq	[edx+48], r7	; write R3 = r7
	 movq	r6, r4			; r6 = __ __ 17 16
	movq	r7, M(3)		; r7 = FF __ __ __
	 pand	r4, r2			; r4 = __ __ __ 16
	movq	r3, M(1)		; r3 = __ __ FF __
	 pand	r7, r1			; r7 = 37 __ __ __
	pand	r3, r5			; r3 = __ __ 31 __
	 por	r0, r4			; r0 = __ __ 20 16
	psllq	r3, 16			; r3 = __ 31 __ __
	 por	r7, r0			; r7 = 37 __ 20 16
	movq	r4, M(2)		; r4 = __ FF __ __
	 por	r7, r3			; r7 = 37 31 20 16 = R4
	movq	r0, [eax+80]
	 movq	r3, r4			; r3 = __ __ FF __
	pmullw	r0, [ebx+80]	; r0 = 53 52 51 50
	 pand	r4, r5			; r4 = __ 32 __ __
	movq	[edx+8], r7		; write R4 = r7
	 por	r6, r4			; r6 = __ 32 17 16
	movq	r4, r3			; r4 = __ FF __ __
	 psrlq	r6, 16			; r6 = __ __ 32 17
	movq	r7, r0			; r7 = 53 52 51 50
	 pand	r4, r1			; r4 = __ 36 __ __
	psllq	r7, 48			; r7 = 50 __ __ __
	 por	r6, r4			; r6 = __ 36 32 17
	movq	r4, [eax+88]
	 por	r7, r6			; r7 = 50 36 32 17 = R5
	pmullw	r4, [ebx+88]	; r4 = 57 56 55 54
	 psrlq	r3, 16			; r3 = __ __ FF __
	movq	[edx+24], r7	; write R5 = r7	 
	 pand	r3, r1			; r3 = __ __ 35 __
	psrlq	r5, 48			; r5 = __ __ __ 33
	 pand	r1, r2			; r1 = __ __ __ 34
	movq	r6, [eax+104]
	 por	r5, r3			; r5 = __ __ 35 33
	pmullw	r6, [ebx+104]	; r6 = 67 66 65 64
	 psrlq	r0, 16			; r0 = __ 53 52 51
	movq	r7, r4			; r7 = 57 56 55 54
	 movq	r3, r2			; r3 = __ __ __ FF
	psllq	r7, 48			; r7 = 54 __ __ __
	 pand	r3, r0			; r3 = __ __ __ 51
	pxor	r0, r3			; r0 = __ 53 52 __
	 psllq	r3, 32			; r3 = __ 51 __ __
	por		r7, r5			; r7 = 54 __ 35 33
	 movq	r5, r6			; r5 = 67 66 65 64
	pand	r6, M(1)		; r6 = __ __ 65 __
	 por	r7, r3			; r7 = 54 51 35 33 = R6
	psllq	r6, 32			; r6 = 65 __ __ __
	 por	r0, r1			; r0 = __ 53 52 34
	movq	[edx+40], r7	; write R6 = r7
	 por	r0, r6			; r0 = 65 53 52 34 = R7
	movq	r7, [eax+120]
	 movq	r6, r5			; r6 = 67 66 65 64
	pmullw	r7, [ebx+120]	; r7 = 77 76 75 74
	 psrlq	r5, 32			; r5 = __ __ 67 66
	pand	r6, r2			; r6 = __ __ __ 64
	 movq	r1, r5			; r1 = __ __ 67 66
	movq	[edx+56], r0	; write R7 = r0
	 pand	r1, r2			; r1 = __ __ __ 66
	movq	r0, [eax+112]
	 movq	r3, r7			; r3 = 77 76 75 74
	pmullw	r0, [ebx+112]	; r0 = 73 72 71 70
	 psllq	r3, 16			; r3 = 76 75 74 __
	pand	r7, M(3)		; r7 = 77 __ __ __
	 pxor	r5, r1			; r5 = __ __ 67 __
	por		r6, r5			; r6 = __ __ 67 64
	 movq	r5, r3			; r5 = 76 75 74 __
	pand	r5, M(3)		; r5 = 76 __ __ __
	 por	r7, r1			; r7 = 77 __ __ 66
	movq	r1, [eax+96]
	 pxor	r3, r5			; r3 = __ 75 74 __
	pmullw	r1, [ebx+96] 	; r1 = 63 62 61 60
	 por	r7, r3			; r7 = 77 75 74 66 = R15
	por		r6, r5			; r6 = 76 __ 67 64
	 movq	r5, r0			; r5 = 73 72 71 70
	movq	[edx+120], r7	; store R15 = r7
	 psrlq	r5, 16			; r5 = __ 73 72 71
	pand	r5, M(2)		; r5 = __ 73 __ __
	 movq	r7, r0			; r7 = 73 72 71 70
	por		r6, r5			; r6 = 76 73 67 64 = R14
	 pand	r0, r2			; r0 = __ __ __ 70
	pxor	r7, r0			; r7 = 73 72 71 __
	 psllq	r0, 32			; r0 = __ 70 __ __	
	movq	[edx+104], r6	; write R14 = r6
	 psrlq	r4, 16			; r4 = __ 57 56 55
	movq	r5, [eax+72]
	 psllq	r7, 16			; r7 = 72 71 __ __
	pmullw	r5, [ebx+72]	; r5 = 47 46 45 44
	 movq	r6, r7			; r6 = 72 71 __ __
	movq	r3, M(2)		; r3 = __ FF __ __
	 psllq	r6, 16			; r6 = 71 __ __ __
	pand	r7, M(3)		; r7 = 72 __ __ __
	 pand	r3, r1			; r3 = __ 62 __ __
	por		r7, r0			; r7 = 72 70 __ __
	 movq	r0, r1			; r0 = 63 62 61 60
	pand	r1, M(3)		; r1 = 63 __ __ __
	 por	r6, r3			; r6 = 71 62 __ __
	movq	r3, r4			; r3 = __ 57 56 55
	 psrlq	r1, 32			; r1 = __ __ 63 __
	pand	r3, r2			; r3 = __ __ __ 55
	 por	r7, r1			; r7 = 72 70 63 __
	por		r7, r3			; r7 = 72 70 63 55 = R13
	 movq	r3, r4			; r3 = __ 57 56 55
	pand	r3, M(1)		; r3 = __ __ 56 __
	 movq	r1, r5			; r1 = 47 46 45 44
	movq	[edx+88], r7	; write R13 = r7
	 psrlq	r5, 48			; r5 = __ __ __ 47
	movq	r7, [eax+64]
	 por	r6, r3			; r6 = 71 62 56 __	 
	pmullw	r7, [ebx+64]	; r7 = 43 42 41 40
	 por	r6, r5			; r6 = 71 62 56 47 = R12
	pand	r4, M(2)		; r4 = __ 57 __ __
	 psllq	r0, 32			; r0 = 61 60 __ __
	movq	[edx+72], r6	; write R12 = r6
	 movq	r6, r0			; r6 = 61 60 __ __
	pand	r0, M(3)		; r0 = 61 __ __ __
	 psllq	r6, 16			; r6 = 60 __ __ __
	movq	r5, [eax+40]
	 movq	r3, r1			; r3 = 47 46 45 44
	pmullw	r5, [ebx+40]	; r5 = 27 26 25 24
	 psrlq	r1, 16			; r1 = __ 47 46 45
	pand	r1, M(1)		; r1 = __ __ 46 __
	 por	r0, r4			; r0 = 61 57 __ __
	pand	r2, r7			; r2 = __ __ __ 40
	 por	r0, r1			; r0 = 61 57 46 __
	por		r0, r2			; r0 = 61 57 46 40 = R11
	 psllq	r3, 16			; r3 = 46 45 44 __
	movq	r4, r3			; r4 = 46 45 44 __
	 movq	r2, r5			; r2 = 27 26 25 24
	movq	[edx+112], r0	; write R11 = r0
	 psrlq	r2, 48			; r2 = __ __ __ 27
	pand	r4, M(2)		; r4 = __ 45 __ __
	 por	r6, r2			; r6 = 60 __ __ 27
	movq	r2, M(1)		; r2 = __ __ FF __
	 por	r6, r4			; r6 = 60 45 __ 27
	pand	r2, r7			; r2 = __ __ 41 __	 
	 psllq	r3, 32			; r3 = 44 __ __ __
	por		r3, [edx+80]	; r3 = 44 __ __ 23
	 por	r6, r2			; r6 = 60 45 41 27 = R10
	movq	r2, M(3)		; r2 = FF __ __ __
	 psllq	r5, 16			; r5 = 26 25 24 __
	movq	[edx+96], r6	; store R10 = r6
	 pand	r2, r5			; r2 = 26 __ __ __
	movq	r6, M(2)		; r6 = __ FF __ __
	 pxor	r5, r2			; r5 = __ 25 24 __
	pand	r6, r7			; r6 = __ 42 __ __
	 psrlq	r2, 32			; r2 = __ __ 26 __
	pand	r7, M(3)		; r7 = 43 __ __ __
	 por	r3, r2			; r3 = 44 __ 26 23
	por		r7, [edx+64]	; r7 = 43 __ __ 12
	 por	r6, r3			; r6 = 44 42 26 23 = R9
	por		r7, r5			; r7 = 43 25 24 12 = R8
	 ;
	movq	[edx+80], r6	; store R9 = r6
	 ;
	movq	[edx+64], r7	; store R8 = r7
	 ;
	; 123c  ( / 64 coeffs  < 2c / coeff)
#	undef M

; Done w/dequant + descramble + partial transpose; now do the idct itself.

#	define I( K)	[edx + (  K      * 16)]
#	define J( K)	[edx + ( (K - 4) * 16) + 8]

	RowIDCT			; 46 c
	Transpose		; 19 c

#	undef I
#	undef J
#	define I( K)	[edx + (  K      * 16) + 64]
#	define J( K)	[edx + ( (K - 4) * 16) + 72]

	RowIDCT			; 46 c
	Transpose		; 19 c

#	undef I
#	undef J
#	define I( K)	[edx + (K * 16)]
#	define J( K)	I( K)

	ColumnIDCT		; 57 c

#	undef I
#	undef J
#	define I( K)	[edx + (K * 16) + 8]
#	define J( K)	I( K)

	ColumnIDCT		; 57 c

#	undef I
#	undef J
	pop ebx
	pop ecx
	pop edx
	ret
	; 368 cycles  ( / 64 coeff  <  6 c / coeff)
 }
}

/**************************************************************************************
 *
 *		Routine:		MMX_idct10
 *		
 *		Description:	Perform IDCT on a 8x8 block with at most 10 nonzero coefficients
 *
 *		Input:			Pointer to input and output buffer				
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	The input coefficients are in transposed ZigZag order
 *
 *		Error:			None
 *
 ***************************************************************************************
 */
/* --------------------------------------------------------------- */
// This macro does four 4-sample one-dimensional idcts in parallel.  Inputs
// 4 thru 7 are assumed to be zero.
#define BeginIDCT_10 __asm { \
\
	__asm	movq		r2, I(3)  \
__asm   nop \
\
	__asm	movq		r6, C(3) \
	__asm	movq		r4, r2 \
\
	__asm	movq		r1, C(5) \
	__asm	pmulhw		r4, r6		/* r4 = c3*i3 - i3 */ \
\
	__asm	movq		r3, I(1) \
	__asm	pmulhw		r1, r2		/* r1 = c5*i3 - i3 */ \
\
	__asm	movq		r0, C(1)	/* (all registers are in use) */ \
	__asm	paddw		r4, r2		/* r4 = C = c3*i3 */ \
\
    __asm   pxor        r6,r6       /* used to get -(c5*i3) */ \
	__asm	paddw		r2, r1		/* r2 = c5*i3 */ \
\
	__asm	movq		r5, I(2) \
	__asm	pmulhw		r0, r3		/* r0 = c1*i1 - i1 */ \
\
	__asm	movq		r1, r5 \
	__asm	paddw		r0, r3		/* r0 = A = c1*i1 */ \
\
	__asm	pmulhw		r3, C(7)	/* r3 = B = c7*i1 */ \
	__asm	psubsw		r6, r2		/* r6 = D = -c5*i3 */ \
\
	__asm	pmulhw		r5, C(2)	/* r1 = c2*i2 - i2 */ \
	__asm	psubsw		r0, r4		/* r0 = A - C */ \
\
    __asm   movq        r7,I(2) \
	__asm	paddsw		r4, r4		/* r4 = C + C */ \
\
	__asm	paddw		r7, r5		/* r7 = G = c2*i2 */ \
	__asm	paddsw		r4, r0		/* r4 = C. = A + C */ \
\
	__asm	pmulhw		r1, C(6)	/* r1 = H = c6*i2 */ \
	__asm	psubsw		r3, r6		/* r3 = B - D */ \
\
	__asm	movq		I(1), r4	/* save C. at I(1) */ \
	__asm	paddsw		r6, r6		/* r6 = D + D */ \
\
    __asm	movq		r4, C(4) \
	__asm	paddsw		r6, r3		/* r6 = D. = B + D */ \
\
	__asm	movq		r5, r3		/* r5 = B - D */ \
	__asm	pmulhw		r3, r4		/* r3 = (c4 - 1) * (B - D) */ \
\
	__asm	movq		I(2), r6	/* save D. at I(2) */ \
	__asm	movq		r2, r0		/* r2 = A - C */ \
\
	__asm	movq		r6, I(0) \
	__asm	pmulhw		r0, r4		/* r0 = (c4 - 1) * (A - C) */ \
\
	__asm	paddw		r5, r3		/* r5 = B. = c4 * (B - D) */ \
	__asm	paddw		r2, r0		/* r0 = A. = c4 * (A - C) */ \
\
	__asm	psubsw		r5, r1		/* r5 = B.. = B. - H */ \
	__asm	pmulhw		r6, r4		/* r6 = c4*i0 - i0 */ \
\
    __asm   paddw       r6, I(0)    /* r6 = E = c4*i0 */ \
	__asm	paddsw		r1, r1		/* r1 = H + H */ \
\
	__asm	movq		r4, r6      /* r4 = E */ \
	__asm	paddsw		r1, r5		/* r1 = H. = B + H */ \
\
	__asm	psubsw		r6, r2		/* r6 = F. = E - A. */ \
	__asm	paddsw		r2, r2		/* r2 = A. + A. */ \
\
	__asm	movq		r0, I(1)	/* r0 = C. */ \
	__asm	paddsw		r2, r6		/* r2 = A.. = E + A. */ \
\
	__asm	psubsw		r2, r1		/* r2 = R2 = A.. - H. */ \
__asm   nop \
}
// end BeginIDCT_10 macro (25 cycles).

#define RowIDCT_10 __asm { \
	\
	BeginIDCT_10 \
	\
	__asm	movq		r3, I(2)	/* r3 = D. */ \
	__asm	 psubsw		r4, r7		/* r4 = E. = E - G */ \
	__asm	paddsw		r1, r1		/* r1 = H. + H. */ \
	__asm	 paddsw		r7, r7		/* r7 = G + G */ \
	__asm	paddsw		r1, r2		/* r1 = R1 = A.. + H. */ \
	__asm	 paddsw		r7, r4		/* r7 = G. = E + G */ \
	__asm	psubsw		r4, r3		/* r4 = R4 = E. - D. */ \
	__asm	 paddsw		r3, r3 \
	__asm	psubsw		r6, r5		/* r6 = R6 = F. - B.. */ \
	__asm	 paddsw		r5, r5 \
	__asm	paddsw		r3, r4		/* r3 = R3 = E. + D. */ \
	__asm	 paddsw		r5, r6		/* r5 = R5 = F. + B.. */ \
	__asm	psubsw		r7, r0		/* r7 = R7 = G. - C. */ \
	__asm	 paddsw		r0, r0 \
	__asm	movq		I(1), r1	/* save R1 */ \
	__asm	 paddsw		r0, r7		/* r0 = R0 = G. + C. */ \
}
// end RowIDCT macro (8 + 38 = 46 cycles)

// Column IDCT normalizes and stores final results.

#define ColumnIDCT_10 __asm { \
	\
	BeginIDCT_10 \
	\
	__asm	paddsw		r2, Eight	/* adjust R2 (and R1) for shift */ \
	__asm	 paddsw		r1, r1		/* r1 = H. + H. */ \
	__asm	paddsw		r1, r2		/* r1 = R1 = A.. + H. */ \
	__asm	 psraw		r2, 4		/* r2 = NR2 */ \
	__asm	psubsw		r4, r7		/* r4 = E. = E - G */ \
	__asm	 psraw		r1, 4		/* r1 = NR1 */ \
	__asm	movq		r3, I(2)	/* r3 = D. */ \
	__asm	 paddsw		r7, r7		/* r7 = G + G */ \
	__asm	movq		I(2), r2	/* store NR2 at I2 */ \
	__asm	 paddsw		r7, r4		/* r7 = G. = E + G */ \
	__asm	movq		I(1), r1	/* store NR1 at I1 */ \
	__asm	 psubsw		r4, r3		/* r4 = R4 = E. - D. */ \
	__asm	paddsw		r4, Eight	/* adjust R4 (and R3) for shift */ \
	__asm	 paddsw		r3, r3		/* r3 = D. + D. */ \
	__asm	paddsw		r3, r4		/* r3 = R3 = E. + D. */ \
	__asm	 psraw		r4, 4		/* r4 = NR4 */ \
	__asm	psubsw		r6, r5		/* r6 = R6 = F. - B.. */ \
	__asm	 psraw		r3, 4		/* r3 = NR3 */ \
	__asm	paddsw		r6, Eight	/* adjust R6 (and R5) for shift */ \
	__asm	 paddsw		r5, r5		/* r5 = B.. + B.. */ \
	__asm	paddsw		r5, r6		/* r5 = R5 = F. + B.. */ \
	__asm	 psraw		r6, 4		/* r6 = NR6 */ \
	__asm	movq		J(4), r4	/* store NR4 at J4 */ \
	__asm	 psraw		r5, 4		/* r5 = NR5 */ \
	__asm	movq		I(3), r3	/* store NR3 at I3 */ \
	__asm	 psubsw		r7, r0		/* r7 = R7 = G. - C. */ \
	__asm	paddsw		r7, Eight	/* adjust R7 (and R0) for shift */ \
	__asm	 paddsw		r0, r0 		/* r0 = C. + C. */ \
	__asm	paddsw		r0, r7		/* r0 = R0 = G. + C. */ \
	__asm	 psraw		r7, 4		/* r7 = NR7 */ \
	__asm	movq		J(6), r6	/* store NR6 at J6 */ \
	__asm	 psraw		r0, 4		/* r0 = NR0 */ \
	__asm	movq		J(5), r5	/* store NR5 at J5 */ \
	 \
	__asm	movq		J(7), r7	/* store NR7 at J7 */ \
	 \
	__asm	movq		I(0), r0	/* store NR0 at I0 */ \
	 \
}
// end ColumnIDCT macro (38 + 19 = 57 cycles)
/* --------------------------------------------------------------- */


/* --------------------------------------------------------------- */
/* IDCT 10 */
__declspec ( naked ) void MMX_idct10 (	INT16 * input, INT16 * qtbl, INT16 * output) 
{

#	define M(I)		[ecx + MaskOffset + I*8]
#	define C(I)		[ecx + CosineOffset + (I-1)*8]
#	define Eight	[ecx + EightOffset]
#   undef Arg
#	define Arg(I)	[esp + 16 + I*4]

#	define r0	mm0
#	define r1	mm1
#	define r2	mm2
#	define r3	mm3
#	define r4	mm4
#	define r5	mm5
#	define r6	mm6
#	define r7	mm7
	(void) output;
	(void) qtbl;
	(void) input;

 __asm {
	push	edx				
	push	ecx
	push	ebx

// Label:
	mov		eax, Arg( 0)	; eax = quantized input
	 mov	edx, Arg( 2)	; edx = destination (= idct buffer)

	mov		ecx, [edx]		; (+1 at least) preload the cache before writing
	 mov	ebx, [edx+28]   ; in case proc doesn't cache on writes
	mov		ecx, [edx+56]	; gets all the cache lines
	 mov	ebx, [edx+84]	; regardless of alignment (beyond 32-bit)
	mov		ecx, [edx+112]	; also avoids address contention stalls
	 mov	ebx, [edx+124]

	mov		ebx, Arg( 1)	; ebx = quantization table
	lea     ecx, idctconstants ;; [0];

	movq	r0, [eax]
	 ;
	pmullw	r0, [ebx]		; r0 = 03 02 01 00
	 ;
	movq	r1, [eax+16]
	 ;
	pmullw	r1, [ebx+16]	; r1 = 13 12 11 10
	 ;
	movq	r2, M(0)		; r2 = __ __ __ FF
	 movq	r3, r0			; r3 = 03 02 01 00
	movq	r4, [eax+8]
	 psrlq	r0, 16			; r0 = __ 03 02 01
	pmullw	r4, [ebx+8]		; r4 = 07 06 05 04
	 pand	r3, r2			; r3 = __ __ __ 00
	movq	r5, r0			; r5 = __ 03 02 01
	 movq	r6, r1			; r6 = 13 12 11 10
	pand	r5, r2			; r5 = __ __ __ 01
	 psllq	r6, 32			; r6 = 11 10 __ __
	movq	r7, M(3)		; r7 = FF __ __ __
	 pxor	r0, r5			; r0 = __ 03 02 __
	pand	r7, r6			; r7 = 11 __ __ __
	 por	r0, r3			; r0 = __ 03 02 00
	pxor	r6, r7			; r6 = __ 10 __ __
	 por	r0, r7			; r0 = 11 03 02 00 = R0
	movq	r7, M(3)		; r7 = FF __ __ __
	 movq	r3, r4			; r3 = 07 06 05 04
	movq	[edx], r0		; write R0 = r0
	 pand	r3, r2			; r3 = __ __ __ 04
	movq	r0, [eax+32]
	 psllq	r3, 16			; r3 = __ __ 04 __
	pmullw	r0, [ebx+32]	; r0 = 23 22 21 20
	 pand	r7, r1			; r7 = 13 __ __ __
	por		r5, r3			; r5 = __ __ 04 01
	 por	r7, r6			; r7 = 13 10 __ __
	movq	r3, [eax+24]
	 por	r7, r5			; r7 = 13 10 04 01 = R1
	pmullw	r3, [ebx+24]	; r3 = 17 16 15 14
	 psrlq	r4, 16			; r4 = __ 07 06 05
	movq	[edx+16], r7	; write R1 = r7
	 movq	r5, r4			; r5 = __ 07 06 05
	movq	r7, r0			; r7 = 23 22 21 20
	 psrlq	r4, 16			; r4 = __ __ 07 06
	psrlq	r7, 48			; r7 = __ __ __ 23
	 movq	r6, r2			; r6 = __ __ __ FF
	pand	r5, r2			; r5 = __ __ __ 05
	 pand	r6, r4			; r6 = __ __ __ 06
	movq	[edx+80], r7	; partial R9 = __ __ __ 23
	 pxor	r4, r6			; r4 = __ __ 07 __
	psrlq	r1, 32			; r1 = __ __ 13 12
	 por	r4, r5			; r4 = __ __ 07 05
	movq	r7, M(3)		; r7 = FF __ __ __
	 pand	r1, r2			; r1 = __ __ __ 12
	movq	r5, [eax+48]
	 psllq	r0, 16			; r0 = 22 21 20 __
	pmullw	r5, [ebx+48]	; r5 = 33 32 31 30
	 pand	r7, r0			; r7 = 22 __ __ __
	movq	[edx+64], r1	; partial R8 = __ __ __ 12
	 por	r7, r4			; r7 = 22 __ 07 05
	movq	r4, r3			; r4 = 17 16 15 14
	 pand	r3, r2			; r3 = __ __ __ 14
	movq	r1, M(2)		; r1 = __ FF __ __
	 psllq	r3, 32			; r3 = __ 14 __ __
	por		r7, r3			; r7 = 22 14 07 05 = R2
	 movq	r3, r5			; r3 = 33 32 31 30
	psllq	r3, 48			; r3 = 30 __ __ __
	 pand	r1, r0			; r1 = __ 21 __ __
	movq	[edx+32], r7	; write R2 = r7
	 por	r6, r3			; r6 = 30 __ __ 06
	movq	r7, M(1)		; r7 = __ __ FF __
	 por	r6, r1			; r6 = 30 21 __ 06
	movq	r1, [eax+56]
	 pand	r7, r4			; r7 = __ __ 15 __
	pmullw	r1, [ebx+56]	; r1 = 37 36 35 34
	 por	r7, r6			; r7 = 30 21 15 06 = R3
	pand	r0, M(1)		; r0 = __ __ 20 __
	 psrlq	r4, 32			; r4 = __ __ 17 16
	movq	[edx+48], r7	; write R3 = r7
	 movq	r6, r4			; r6 = __ __ 17 16
	movq	r7, M(3)		; r7 = FF __ __ __
	 pand	r4, r2			; r4 = __ __ __ 16
	movq	r3, M(1)		; r3 = __ __ FF __
	 pand	r7, r1			; r7 = 37 __ __ __
	pand	r3, r5			; r3 = __ __ 31 __
	 por	r0, r4			; r0 = __ __ 20 16
	psllq	r3, 16			; r3 = __ 31 __ __
	 por	r7, r0			; r7 = 37 __ 20 16
	movq	r4, M(2)		; r4 = __ FF __ __
	 por	r7, r3			; r7 = 37 31 20 16 = R4
	movq	r0, [eax+80]
	 movq	r3, r4			; r3 = __ __ FF __
	pmullw	r0, [ebx+80]	; r0 = 53 52 51 50
	 pand	r4, r5			; r4 = __ 32 __ __
	movq	[edx+8], r7		; write R4 = r7
	 por	r6, r4			; r6 = __ 32 17 16
	movq	r4, r3			; r4 = __ FF __ __
	 psrlq	r6, 16			; r6 = __ __ 32 17
	movq	r7, r0			; r7 = 53 52 51 50
	 pand	r4, r1			; r4 = __ 36 __ __
	psllq	r7, 48			; r7 = 50 __ __ __
	 por	r6, r4			; r6 = __ 36 32 17
	movq	r4, [eax+88]
	 por	r7, r6			; r7 = 50 36 32 17 = R5
	pmullw	r4, [ebx+88]	; r4 = 57 56 55 54
	 psrlq	r3, 16			; r3 = __ __ FF __
	movq	[edx+24], r7	; write R5 = r7	 
	 pand	r3, r1			; r3 = __ __ 35 __
	psrlq	r5, 48			; r5 = __ __ __ 33
	 pand	r1, r2			; r1 = __ __ __ 34
	movq	r6, [eax+104]
	 por	r5, r3			; r5 = __ __ 35 33
	pmullw	r6, [ebx+104]	; r6 = 67 66 65 64
	 psrlq	r0, 16			; r0 = __ 53 52 51
	movq	r7, r4			; r7 = 57 56 55 54
	 movq	r3, r2			; r3 = __ __ __ FF
	psllq	r7, 48			; r7 = 54 __ __ __
	 pand	r3, r0			; r3 = __ __ __ 51
	pxor	r0, r3			; r0 = __ 53 52 __
	 psllq	r3, 32			; r3 = __ 51 __ __
	por		r7, r5			; r7 = 54 __ 35 33
	 movq	r5, r6			; r5 = 67 66 65 64
	pand	r6, M(1)		; r6 = __ __ 65 __
	 por	r7, r3			; r7 = 54 51 35 33 = R6
	psllq	r6, 32			; r6 = 65 __ __ __
	 por	r0, r1			; r0 = __ 53 52 34
	movq	[edx+40], r7	; write R6 = r7
	 por	r0, r6			; r0 = 65 53 52 34 = R7
	movq	r7, [eax+120]
	 movq	r6, r5			; r6 = 67 66 65 64
	pmullw	r7, [ebx+120]	; r7 = 77 76 75 74
	 psrlq	r5, 32			; r5 = __ __ 67 66
	pand	r6, r2			; r6 = __ __ __ 64
	 movq	r1, r5			; r1 = __ __ 67 66
	movq	[edx+56], r0	; write R7 = r0
	 pand	r1, r2			; r1 = __ __ __ 66
	movq	r0, [eax+112]
	 movq	r3, r7			; r3 = 77 76 75 74
	pmullw	r0, [ebx+112]	; r0 = 73 72 71 70
	 psllq	r3, 16			; r3 = 76 75 74 __
	pand	r7, M(3)		; r7 = 77 __ __ __
	 pxor	r5, r1			; r5 = __ __ 67 __
	por		r6, r5			; r6 = __ __ 67 64
	 movq	r5, r3			; r5 = 76 75 74 __
	pand	r5, M(3)		; r5 = 76 __ __ __
	 por	r7, r1			; r7 = 77 __ __ 66
	movq	r1, [eax+96]
	 pxor	r3, r5			; r3 = __ 75 74 __
	pmullw	r1, [ebx+96] 	; r1 = 63 62 61 60
	 por	r7, r3			; r7 = 77 75 74 66 = R15
	por		r6, r5			; r6 = 76 __ 67 64
	 movq	r5, r0			; r5 = 73 72 71 70
	movq	[edx+120], r7	; store R15 = r7
	 psrlq	r5, 16			; r5 = __ 73 72 71
	pand	r5, M(2)		; r5 = __ 73 __ __
	 movq	r7, r0			; r7 = 73 72 71 70
	por		r6, r5			; r6 = 76 73 67 64 = R14
	 pand	r0, r2			; r0 = __ __ __ 70
	pxor	r7, r0			; r7 = 73 72 71 __
	 psllq	r0, 32			; r0 = __ 70 __ __	
	movq	[edx+104], r6	; write R14 = r6
	 psrlq	r4, 16			; r4 = __ 57 56 55
	movq	r5, [eax+72]
	 psllq	r7, 16			; r7 = 72 71 __ __
	pmullw	r5, [ebx+72]	; r5 = 47 46 45 44
	 movq	r6, r7			; r6 = 72 71 __ __
	movq	r3, M(2)		; r3 = __ FF __ __
	 psllq	r6, 16			; r6 = 71 __ __ __
	pand	r7, M(3)		; r7 = 72 __ __ __
	 pand	r3, r1			; r3 = __ 62 __ __
	por		r7, r0			; r7 = 72 70 __ __
	 movq	r0, r1			; r0 = 63 62 61 60
	pand	r1, M(3)		; r1 = 63 __ __ __
	 por	r6, r3			; r6 = 71 62 __ __
	movq	r3, r4			; r3 = __ 57 56 55
	 psrlq	r1, 32			; r1 = __ __ 63 __
	pand	r3, r2			; r3 = __ __ __ 55
	 por	r7, r1			; r7 = 72 70 63 __
	por		r7, r3			; r7 = 72 70 63 55 = R13
	 movq	r3, r4			; r3 = __ 57 56 55
	pand	r3, M(1)		; r3 = __ __ 56 __
	 movq	r1, r5			; r1 = 47 46 45 44
	movq	[edx+88], r7	; write R13 = r7
	 psrlq	r5, 48			; r5 = __ __ __ 47
	movq	r7, [eax+64]
	 por	r6, r3			; r6 = 71 62 56 __	 
	pmullw	r7, [ebx+64]	; r7 = 43 42 41 40
	 por	r6, r5			; r6 = 71 62 56 47 = R12
	pand	r4, M(2)		; r4 = __ 57 __ __
	 psllq	r0, 32			; r0 = 61 60 __ __
	movq	[edx+72], r6	; write R12 = r6
	 movq	r6, r0			; r6 = 61 60 __ __
	pand	r0, M(3)		; r0 = 61 __ __ __
	 psllq	r6, 16			; r6 = 60 __ __ __
	movq	r5, [eax+40]
	 movq	r3, r1			; r3 = 47 46 45 44
	pmullw	r5, [ebx+40]	; r5 = 27 26 25 24
	 psrlq	r1, 16			; r1 = __ 47 46 45
	pand	r1, M(1)		; r1 = __ __ 46 __
	 por	r0, r4			; r0 = 61 57 __ __
	pand	r2, r7			; r2 = __ __ __ 40
	 por	r0, r1			; r0 = 61 57 46 __
	por		r0, r2			; r0 = 61 57 46 40 = R11
	 psllq	r3, 16			; r3 = 46 45 44 __
	movq	r4, r3			; r4 = 46 45 44 __
	 movq	r2, r5			; r2 = 27 26 25 24
	movq	[edx+112], r0	; write R11 = r0
	 psrlq	r2, 48			; r2 = __ __ __ 27
	pand	r4, M(2)		; r4 = __ 45 __ __
	 por	r6, r2			; r6 = 60 __ __ 27
	movq	r2, M(1)		; r2 = __ __ FF __
	 por	r6, r4			; r6 = 60 45 __ 27
	pand	r2, r7			; r2 = __ __ 41 __	 
	 psllq	r3, 32			; r3 = 44 __ __ __
	por		r3, [edx+80]	; r3 = 44 __ __ 23
	 por	r6, r2			; r6 = 60 45 41 27 = R10
	movq	r2, M(3)		; r2 = FF __ __ __
	 psllq	r5, 16			; r5 = 26 25 24 __
	movq	[edx+96], r6	; store R10 = r6
	 pand	r2, r5			; r2 = 26 __ __ __
	movq	r6, M(2)		; r6 = __ FF __ __
	 pxor	r5, r2			; r5 = __ 25 24 __
	pand	r6, r7			; r6 = __ 42 __ __
	 psrlq	r2, 32			; r2 = __ __ 26 __
	pand	r7, M(3)		; r7 = 43 __ __ __
	 por	r3, r2			; r3 = 44 __ 26 23
	por		r7, [edx+64]	; r7 = 43 __ __ 12
	 por	r6, r3			; r6 = 44 42 26 23 = R9
	por		r7, r5			; r7 = 43 25 24 12 = R8
	 ;
	movq	[edx+80], r6	; store R9 = r6
	 ;
	movq	[edx+64], r7	; store R8 = r7
	 ;
	; 123c  ( / 64 coeffs  < 2c / coeff)

#	undef M

; Done w/dequant + descramble + partial transpose; now do the idct itself.

#	define I( K)	[edx + (  K      * 16)]
#	define J( K)	[edx + ( (K - 4) * 16) + 8]

	RowIDCT_10		; 33 c
	Transpose		; 19 c

#	undef I
#	undef J
#	define I( K)	[edx + (  K      * 16) + 64]
#	define J( K)	[edx + ( (K - 4) * 16) + 72]

//	RowIDCT			; 46 c
//	Transpose		; 19 c

#	undef I
#	undef J
#	define I( K)	[edx + (K * 16)]
#	define J( K)	I( K)

	ColumnIDCT_10		; 44 c

#	undef I
#	undef J
#	define I( K)	[edx + (K * 16) + 8]
#	define J( K)	I( K)

	ColumnIDCT_10		; 44 c

#	undef I
#	undef J



	pop	ebx
	pop ecx
	pop	edx
	 ret		
 }
}

/**************************************************************************************
 *
 *		Routine:		MMX_idct1
 *		
 *		Description:	Perform IDCT on a 8x8 block with at most 1 nonzero coefficients
 *
 *		Input:			Pointer to input and output buffer				
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

/* --------------------------------------------------------------- */
/* IDCT 1 */
void MMX_idct1 (INT16 * input, INT16 * qtbl, INT16 * output) 
{
        if(input[0])
        {
            int i;
            INT32 temp = (INT32)input[0];
			INT32 *iBuf=(INT32 *)output;
        
            temp *= qtbl[0];
            
            //necessary in order to match tim's
            temp += 15;

            temp >>= 5;

            temp &= 0xffff;

            temp += temp << 16;

            for(i = 0; i < 32; i += 4)
            {
                iBuf[i] = temp;
                iBuf[i+1] = temp;
                iBuf[i+2] = temp;
                iBuf[i+3] = temp;
            }
        }
        else
        {
	        /* special case where there is only a 0 dc coeff */
    	    memset( output, 0, 128);
        }

}

/* --------------------------------------------------------------- */
/*
    The following functions (MMX_idct_DX and MMX_idct10_DX) are only
    used by the dxer.  The coeffs are written into a transposed order 
    during the unpack stage.  
*/
/* --------------------------------------------------------------- */
/**************************************************************************************
 *
 *		Routine:		MMX_idct_DX
 *		
 *		Description:	Perform IDCT on a 8x8 block
 *
 *		Input:			Pointer to input and output buffer				
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	The input coefficients are in transposed ZigZag order
 *
 *		Error:			None
 *
 ***************************************************************************************
 */

__declspec ( naked ) void MMX_idct_DX (	INT16 * input, INT16 * qtbl, INT16 * output) 
{

//	uINT16 *constants = idctconstants;
#	define M(I)		[ecx + MaskOffset + I*8]
#	define C(I)		[ecx + CosineOffset + (I-1)*8]
#	define Eight	[ecx + EightOffset]
#   undef Arg
#	define Arg(I)	[esp + 1*4 + 3*4 + I*4] // 1 return address + 3 pushes prior to args

#	define r0	mm0
#	define r1	mm1
#	define r2	mm2
#	define r3	mm3
#	define r4	mm4
#	define r5	mm5
#	define r6	mm6
#	define r7	mm7
	(void) output;
	(void) qtbl;
	(void) input;

	__asm {

	push	edx
	push	ecx
	push	ebx

;; Label:
	mov		eax, Arg( 0)	; eax = quantized input
	 mov	edx, Arg( 2)	; edx = destination (= idct buffer)

	mov		ecx, [edx]		; (+1 at least) preload the cache before writing
	 mov	ebx, [edx+28]   ; in case proc doesn't cache on writes
	mov		ecx, [edx+56]	; gets all the cache lines
	 mov	ebx, [edx+84]	; regardless of alignment (beyond 32-bit)
	mov		ecx, [edx+112]	; also avoids address contention stalls
	 mov	ebx, [edx+124]

	mov		ebx, Arg( 1)	; ebx = quantization table
	 lea    ecx, idctconstants ;;[0];

//dequantization    
//try to optimize better
	movq	r0, [eax+0]
	 ;
	pmullw	r0, [ebx+0]		; r0 = 03 02 01 00
	 ;
	movq	r1, [eax+8]
	 ;
	pmullw	r1, [ebx+8]
	 ;
	movq	r2, [eax+16]
	 ;
	pmullw	r2, [ebx+16]
	 ;
	movq	r3, [eax+24]
	 ;
	pmullw	r3, [ebx+24]
	 ;
	movq	r4, [eax+32]
	 ;
	pmullw	r4, [ebx+32]
	 ;
	movq	r5, [eax+40]
	 ;
	pmullw	r5, [ebx+40]
	 ;
	movq	r6, [eax+48]
	 ;
	pmullw	r6, [ebx+48]
	 ;
	movq	r7, [eax+56]
	 ;
	pmullw	r7, [ebx+56]
	 ;
    movq    [edx+0],r0
     ;
    movq    [edx+8],r1
     ;
    movq    [edx+16],r2
     ;
    movq    [edx+24],r3
     ;
    movq    [edx+32],r4
     ;
    movq    [edx+40],r5
     ;
    movq    [edx+48],r6
     ;
    movq    [edx+56],r7
     ;
;;;;;;;;;;;    
	movq	r0, [eax+64]
	 ;
	pmullw	r0, [ebx+64]		; r0 = 03 02 01 00
	 ;
	movq	r1, [eax+72]
	 ;
	pmullw	r1, [ebx+72]
	 ;
	movq	r2, [eax+80]
	 ;
	pmullw	r2, [ebx+80]
	 ;
	movq	r3, [eax+88]
	 ;
	pmullw	r3, [ebx+88]
	 ;
	movq	r4, [eax+96]
	 ;
	pmullw	r4, [ebx+96]
	 ;
	movq	r5, [eax+104]
	 ;
	pmullw	r5, [ebx+104]
	 ;
	movq	r6, [eax+112]
	 ;
	pmullw	r6, [ebx+112]
	 ;
	movq	r7, [eax+120]
	 ;
	pmullw	r7, [ebx+120]
	 ;
    movq    [edx+64],r0
     ;
    movq    [edx+72],r1
     ;
    movq    [edx+80],r2
     ;
    movq    [edx+88],r3
     ;
    movq    [edx+96],r4
     ;
    movq    [edx+104],r5
     ;
    movq    [edx+112],r6
     ;
    movq    [edx+120],r7
     ;

#	undef M

; Done w/dequant + descramble + partial transpose; now do the idct itself.

#	define I( K)	[edx + (  K      * 16)]
#	define J( K)	[edx + ( (K - 4) * 16) + 8]

	RowIDCT			; 46 c
	Transpose		; 19 c

#	undef I
#	undef J
#	define I( K)	[edx + (  K      * 16) + 64]
#	define J( K)	[edx + ( (K - 4) * 16) + 72]

	RowIDCT			; 46 c
	Transpose		; 19 c

#	undef I
#	undef J
#	define I( K)	[edx + (K * 16)]
#	define J( K)	I( K)

	ColumnIDCT		; 57 c

#	undef I
#	undef J
#	define I( K)	[edx + (K * 16) + 8]
#	define J( K)	I( K)

	ColumnIDCT		; 57 c

#	undef I
#	undef J
	pop ebx
	pop ecx
	pop edx
	ret
	; 368 cycles  ( / 64 coeff  <  6 c / coeff)
 }
}

/**************************************************************************************
 *
 *		Routine:		MMX_idct10_DX
 *		
 *		Description:	Perform IDCT on a 8x8 block with at most 10 nonzero coefficients
 *
 *		Input:			Pointer to input and output buffer				
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	The input coefficients are in transposed ZigZag order
 *
 *		Error:			None
 *
 ***************************************************************************************
 */
/* --------------------------------------------------------------- */
/* IDCT 10 */
__declspec ( naked ) void MMX_idct10_DX (	INT16 * input, INT16 * qtbl, INT16 * output) 
{

#	define M(I)		[ecx + MaskOffset + I*8]
#	define C(I)		[ecx + CosineOffset + (I-1)*8]
#	define Eight	[ecx + EightOffset]
#	undef Arg
#	define Arg(I)	[esp + 16 + I*4]

#	define r0	mm0
#	define r1	mm1
#	define r2	mm2
#	define r3	mm3
#	define r4	mm4
#	define r5	mm5
#	define r6	mm6
#	define r7	mm7
	(void) output;
	(void) qtbl;
	(void) input;

 __asm {
	push	edx				
	push	ecx
	push	ebx

// Label:
	mov		eax, Arg( 0)	; eax = quantized input
	 mov	edx, Arg( 2)	; edx = destination (= idct buffer)

	mov		ecx, [edx]		; (+1 at least) preload the cache before writing
	 mov	ebx, [edx+28]   ; in case proc doesn't cache on writes
	mov		ecx, [edx+56]	; gets all the cache lines
	 mov	ebx, [edx+84]	; regardless of alignment (beyond 32-bit)
	mov		ecx, [edx+112]	; also avoids address contention stalls
	 mov	ebx, [edx+124]

	mov		ebx, Arg( 1)	; ebx = quantization table
	lea     ecx, idctconstants ;; [0];

//dequantization    
	movq	r0, [eax+0]
	 ;
	pmullw	r0, [ebx+0]	
	 ;
	movq	r1, [eax+16]
	 ;
	pmullw	r1, [ebx+16]
	 ;
	movq	r2, [eax+32]
	 ;
	pmullw	r2, [ebx+32]
	 ;
	movq	r3, [eax+48]
	 ;
	pmullw	r3, [ebx+48]
	 ;
    movq    [edx+0],r0
    pxor    r5,r5

    movq    [edx+8],r5
     ;
    movq    [edx+16],r1
     ;
    movq    [edx+24],r5
     ;
    movq    [edx+32],r2
     ;
    movq    [edx+40],r5
     ;
    movq    [edx+48],r3
     ;
    movq    [edx+56],r5
     ;
    movq    [edx+64],r5
     ;
    movq    [edx+72],r5
     ;
    movq    [edx+80],r5
     ;
    movq    [edx+88],r5
     ;
    movq    [edx+96],r5
     ;
    movq    [edx+104],r5
     ;
    movq    [edx+112],r5
     ;
    movq    [edx+120],r5
     ;

#	undef M

; Done w/dequant + descramble + partial transpose; now do the idct itself.

#	define I( K)	[edx + (  K      * 16)]
#	define J( K)	[edx + ( (K - 4) * 16) + 8]

	RowIDCT_10		; 33 c
	Transpose		; 19 c

#	undef I
#	undef J
#	define I( K)	[edx + (  K      * 16) + 64]
#	define J( K)	[edx + ( (K - 4) * 16) + 72]

//	RowIDCT			; 46 c
//	Transpose		; 19 c

#	undef I
#	undef J
#	define I( K)	[edx + (K * 16)]
#	define J( K)	I( K)

	ColumnIDCT_10		; 44 c

#	undef I
#	undef J
#	define I( K)	[edx + (K * 16) + 8]
#	define J( K)	I( K)

	ColumnIDCT_10		; 44 c

#	undef I
#	undef J



	pop	ebx
	pop ecx
	pop	edx
	 ret		
 }
}



/**************************************************************************************
 *
 *		Routine:		MMX_idct3
 *		
 *		Description:	Perform IDCT on a 8x8 block with at most 3 nonzero coefficients
 *
 *		Input:			Pointer to input and output buffer				
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	Only works for three nonzero coefficients.
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

#define RowIDCT_3 __asm {			\
\
	__asm	movq		r7, I(1)	/* r7 = I1						*/  \
	__asm	movq		r0, C(1)	/* r0 = C1						*/  \
\
	__asm	movq		r3, C(7)	/* r3 = C7						*/  \
	__asm	pmulhw		r0, r7		/* r0 = C1 * I1 - I1			*/	\
\
	__asm   pmulhw		r3, r7		/* r3 = C7 * I1	= B, D.			*/  \
	__asm	movq		r6, I(0)	/* r6 = I0						*/  \
\
	__asm	movq		r4, C(4)	/* r4 = C4						*/  \
	__asm	paddw		r0, r7		/* r0 = C1 * I1 = A, C.			*/  \
\
	__asm	movq		r1, r6		/* make a copy of I0			*/	\
	__asm	pmulhw		r6, r4      /* r2 = C4 * I0 - I0			*/	\
\
	__asm	movq		r2, r0		/* make a copy of A				*/  \
	__asm   movq		r5, r3		/* make a copy of B				*/	\
\
	__asm	pmulhw		r2, r4      /* r2 = C4 * A - A				*/  \
	__asm   pmulhw		r5, r4		/* r5 = C4 * B - B				*/  \
\
	__asm	paddw		r6, r1		/* r2 = C4 * I0	= E, F			*/	\
	__asm	movq		r4, r6		/* r4 = E						*/  \
\
	__asm   paddw		r2, r0		/* r2 = A.   					*/  \
	__asm	paddw		r5, r3		/* r5 = B.						*/  \
\
	__asm	movq		r7, r6		/* r7 = E						*/  \
	__asm	movq		r1, r5		/* r1 = B.						*/  \
\
	/*  r0 = A		*/   \
	/*	r3 = B		*/	 \
	/*  r2 = A.		*/   \
	/*  r5 = B.		*/   \
	/*  r6 = E		*/   \
	/*  r4 = E		*/   \
	/*  r7 = E		*/   \
	/*  r1 = B.		*/   \
\
	__asm	psubw		r6, r2		/* r6 = E - A.					*/  \
	__asm   psubw		r4, r3		/* r4 = E - B ----R4			*/  \
\
	__asm	psubw		r7, r0		/* r7 = E - A ----R7			*/  \
	__asm	paddw		r2, r2		/* r2 = A. + A.					*/  \
\
	__asm	paddw		r3, r3		/* r3 = B + B					*/  \
	__asm	paddw		r0, r0      /* r0 = A + A					*/  \
\
	__asm	paddw		r2, r6		/* r2 = E + A.					*/  \
	__asm	paddw	    r3, r4		/* r3 = E + B ----R3			*/  \
\
	__asm	psubw		r2, r1		/* r2 = E + A. - B.	----R2  	*/  \
	__asm	psubw		r6, r5		/* r6 = E - A. - B.	----R6		*/  \
\
	__asm	paddw		r1, r1      /* r1 = B. + B.					*/  \
	__asm	paddw		r5, r5		/* r5 = B. + B.					*/  \
\
	__asm	paddw	    r0, r7		/* r0 = E + A ----R0			*/  \
	__asm	paddw		r1, r2		/* r1 = E + A. + B. -----R1     */  \
\
	__asm	movq		I(1), r1    /* save r1						*/	\
	__asm	paddw		r5, r6		/* r5 = E - A. + B.	-----R5     */  \
\
}
//End of RowIDCT_3

#define ColumnIDCT_3 __asm {			\
\
	__asm	movq		r7, I(1)	/* r7 = I1						*/  \
	__asm	movq		r0, C(1)	/* r0 = C1						*/  \
\
	__asm	movq		r3, C(7)	/* r3 = C7						*/  \
	__asm	pmulhw		r0, r7		/* r0 = C1 * I1 - I1			*/	\
\
	__asm   pmulhw		r3, r7		/* r3 = C7 * I1	= B, D.			*/  \
	__asm	movq		r6, I(0)	/* r6 = I0						*/  \
\
	__asm	movq		r4, C(4)	/* r4 = C4						*/  \
	__asm	paddw		r0, r7		/* r0 = C1 * I1 = A, C.			*/  \
\
	__asm	movq		r1, r6		/* make a copy of I0			*/	\
	__asm	pmulhw		r6, r4      /* r2 = C4 * I0 - I0			*/	\
\
	__asm	movq		r2, r0		/* make a copy of A				*/  \
	__asm   movq		r5, r3		/* make a copy of B				*/	\
\
	__asm	pmulhw		r2, r4      /* r2 = C4 * A - A				*/  \
	__asm   pmulhw		r5, r4		/* r5 = C4 * B - B				*/  \
\
	__asm	paddw		r6, r1		/* r2 = C4 * I0	= E, F			*/	\
	__asm	movq		r4, r6		/* r4 = E						*/  \
\
	__asm	paddw		r6, Eight	/* +8 for shift					*/  \
	__asm   Paddw		r4, Eight   /* +8 for shift					*/  \
\
	__asm   paddw		r2, r0		/* r2 = A.   					*/  \
	__asm	paddw		r5, r3		/* r5 = B.						*/  \
\
	__asm	movq		r7, r6		/* r7 = E						*/  \
	__asm	movq		r1, r5		/* r1 = B.						*/  \
\
/*  r0 = A		*/   \
/*	r3 = B		*/	 \
/*  r2 = A.		*/   \
/*  r5 = B.		*/   \
/*  r6 = E		*/   \
/*  r4 = E		*/   \
/*  r7 = E		*/   \
/*  r1 = B.		*/   \
\
	__asm	psubw		r6, r2		/* r6 = E - A.					*/  \
	__asm   psubw		r4, r3		/* r4 = E - B ----R4			*/  \
\
	__asm	psubw		r7, r0		/* r7 = E - A ----R7			*/  \
	__asm	paddw		r2, r2		/* r2 = A. + A.					*/  \
\
	__asm	paddw		r3, r3		/* r3 = B + B					*/  \
	__asm	paddw		r0, r0      /* r0 = A + A					*/  \
\
	__asm	paddw		r2, r6		/* r2 = E + A.					*/  \
	__asm	paddw	    r3, r4		/* r3 = E + B ----R3			*/  \
\
	__asm	psraw		r4, 4		/* shift						*/  \
	__asm   movq		J(4), r4	/* store R4 at J4				*/  \
\
	__asm	psraw		r3, 4		/* shift						*/  \
	__asm   movq		I(3), r3	/* store R3 at I3				*/  \
\
	__asm	psubw		r2, r1		/* r2 = E + A. - B.	----R2  	*/  \
	__asm	psubw		r6, r5		/* r6 = E - A. - B.	----R6		*/  \
\
	__asm	paddw		r1, r1      /* r1 = B. + B.					*/  \
	__asm	paddw		r5, r5		/* r5 = B. + B.					*/  \
\
	__asm	paddw	    r0, r7		/* r0 = E + A ----R0			*/  \
	__asm	paddw		r1, r2		/* r1 = E + A. + B. -----R1     */  \
\
	__asm	psraw		r7, 4		/* shift						*/  \
	__asm	psraw		r2, 4		/* shift						*/	\
\
	__asm	psraw		r0, 4		/* shift						*/	\
	__asm   psraw		r1, 4		/* shift						*/  \
\
	__asm   movq		J(7), r7	/* store R7 to J7				*/  \
	__asm	movq		I(0), r0	/* store R0 to I0				*/  \
\
	__asm   movq		I(1), r1    /* store R1 to I1				*/  \
	__asm	movq		I(2), r2	/* store R2 to I2				*/  \
\
	__asm	movq		I(1), r1    /* save r1						*/	\
	__asm	paddw		r5, r6		/* r5 = E - A. + B.	-----R5     */  \
\
	__asm	psraw		r5, 4		/* shift						*/  \
	__asm   movq		J(5), r5	/* store R5 at J5				*/  \
\
	__asm	psraw		r6, 4		/* shift						*/  \
	__asm   movq		J(6), r6	/* store R6 at J6				*/  \
\
}
//End of ColumnIDCT_3

__declspec ( naked ) void MMX_idct3 (	INT16 * input, INT16 * output ) 
{

#	define M(I)		[ecx + MaskOffset + I*8]
#	define C(I)		[ecx + CosineOffset + (I-1)*8]
#	define Eight	[ecx + EightOffset]
#   undef Arg
#	define Arg(I)	[esp + 16 + I*4]

#	define r0	mm0
#	define r1	mm1
#	define r2	mm2
#	define r3	mm3
#	define r4	mm4
#	define r5	mm5
#	define r6	mm6
#	define r7	mm7
	(void) output;
	(void) input;

 __asm {
	push	edx				
	push	ecx
	push	ebx

// Label:
	mov		eax, Arg( 0)	; eax = quantized input
	 mov	edx, Arg( 1)	; edx = destination (= idct buffer)

	mov		ecx, [edx]		; (+1 at least) preload the cache before writing
	 mov	ebx, [edx+28]   ; in case proc doesn't cache on writes
	mov		ecx, [edx+56]	; gets all the cache lines
	 mov	ebx, [edx+84]	; regardless of alignment (beyond 32-bit)
	mov		ecx, [edx+112]	; also avoids address contention stalls
	 mov	ebx, [edx+124]

	lea     ecx, idctconstants ;; [0];

	movq	r0, [eax]		; r0 = 03 02 01 00
	 ;
	pxor	r1, r1			; r1 = 13 12 11 10; all zero
	 ;
	movq	r2, M(0)		; r2 = __ __ __ FF
	 movq	r3, r0			; r3 = 03 02 01 00
	pxor	r4, r4
	 psrlq	r0, 16			; r0 = __ 03 02 01
	 pand	r3, r2			; r3 = __ __ __ 00
	movq	r5, r0			; r5 = __ 03 02 01
	 movq	r6, r1			; r6 = 13 12 11 10;all zero
	pand	r5, r2			; r5 = __ __ __ 01
	;psllq	r6, 32			; r6 = 11 10 __ __
	movq	r7, M(3)		; r7 = FF __ __ __
	 pxor	r0, r5			; r0 = __ 03 02 __
	pand	r7, r6			; r7 = 11 __ __ __
	 por	r0, r3			; r0 = __ 03 02 00
	pxor	r6, r7			; r6 = __ 10 __ __
	 por	r0, r7			; r0 = 11 03 02 00 = R0
	movq	r7, M(3)		; r7 = FF __ __ __
	 movq	r3, r4			; r3 = 07 06 05 04
	movq	[edx], r0		; write R0 = r0
	 pand	r3, r2			; r3 = __ __ __ 04
	psllq	r3, 16			; r3 = __ __ 04 __
	 pand	r7, r1			; r7 = 13 __ __ __
	por		r5, r3			; r5 = __ __ 04 01
	 por	r7, r6			; r7 = 13 10 __ __
	por	r7, r5				; r7 = 13 10 04 01 = R1
	 psrlq	r4, 16			; r4 = __ 07 06 05
	movq	[edx+16], r7	; write R1 = r7
	movq	[edx+32], r4	; write R2 = r7
	movq	[edx+48], r4	; write R3 = r7
	movq	[edx+8], r4		; write R4 = r7
	movq	[edx+24], r4	; write R5 = r7	 
	movq	[edx+40], r4	; write R6 = r7
	movq	[edx+56], r4	; write R7 = r0
	movq	[edx+120], r4	; store R15 = r7
	movq	[edx+104], r4	; write R14 = r6
	movq	[edx+88], r4	; write R13 = r7
	movq	[edx+72], r4	; write R12 = r6
	movq	[edx+112], r4	; write R12 = r6
	movq	[edx+96], r4	; store R10 = r6
	movq	[edx+80], r4	; store R9 = r6
	movq	[edx+64], r4	; store R8 = r7
	 ;
	; 123c  ( / 64 coeffs  < 2c / coeff)

#	undef M

; Donepartial transpose; now do the idct itself.

#	define I( K)	[edx + (  K      * 16)]
#	define J( K)	[edx + ( (K - 4) * 16) + 8]

	RowIDCT_3		; 33 c
	Transpose		; 19 c

#	undef I
#	undef J
#	define I( K)	[edx + (  K      * 16) + 64]
#	define J( K)	[edx + ( (K - 4) * 16) + 72]

//	RowIDCT			; 46 c
//	Transpose		; 19 c

#	undef I
#	undef J
#	define I( K)	[edx + (K * 16)]
#	define J( K)	I( K)

	ColumnIDCT_3		; 44 c

#	undef I
#	undef J
#	define I( K)	[edx + (K * 16) + 8]
#	define J( K)	I( K)

	ColumnIDCT_3		; 44 c

#	undef I
#	undef J

	pop	ebx
	pop ecx
	pop	edx
	 ret		
 }
}


