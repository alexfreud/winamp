/****************************************************************************
 *
 *   Module Title :     simpledeblock_asm.c
 *
 *   Description  :     Simple deblocking filter for low end machines
 *
 ***************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
* Header Files
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "postp.h"

/****************************************************************************
* Module Statics
****************************************************************************/        
__declspec(align(16)) const unsigned char eightNOnes[]= {255, 255, 255, 255, 255, 255, 255, 255};
__declspec(align(16)) const short fourFours[] = {4, 4, 4, 4};
__declspec(align(16)) const short fourOnes[] = { 1, 1, 1, 1};
__declspec(align(16)) const unsigned char eightFours[] = {4, 4, 4, 4, 4, 4, 4, 4};
__declspec(align(16)) const unsigned char eightOnes[] = {1, 1, 1, 1, 1, 1, 1, 1};
__declspec(align(16)) const unsigned char eight128s[] = {128, 128, 128, 128, 128, 128, 128, 128};

/****************************************************************************
* Imports
****************************************************************************/              
extern UINT32 LoopFilterLimitValuesV1[];
extern UINT32 *DeblockLimitValuesV2;

/****************************************************************************
 * 
 *  ROUTINE       :     FilterHoriz_Simple_MMX
 *
 *  INPUTS        :     None
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Applies a loop filter to the vertical edge horizontally
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void FilterHoriz_Simple_MMX(
						POSTPROC_INSTANCE *pbi, 
						UINT8 * PixelPtr, 
						INT32 LineLength, 
						INT32 *BoundingValuePtr
						)
{	
	/*************************************************************	
		The following code in comments is the C version of the 
		function, provided here for reference  
	 *************************************************************

	INT32 j;
	INT32 FiltVal;
    UINT8 * LimitTable = &LimitVal_VP31[VAL_RANGE];



	for ( j = 0; j < 8; j++ )
	{            
        INT32 UseHighVariance;
		
		FiltVal =  ( PixelPtr[2] * 3 ) - 
			( PixelPtr[1] * 3 );

        UseHighVariance =  abs(PixelPtr[0] - PixelPtr[1]) > 1 ||
                 abs(PixelPtr[2] - PixelPtr[3]) > 1;

        if(UseHighVariance)
        {
            FiltVal +=  ( PixelPtr[0] ) - 
	    		( PixelPtr[3] );
        }

        FiltVal = BoundingValuePtr[(FiltVal + 4) >> 3];
		
		PixelPtr[1] = LimitTable[(INT32)PixelPtr[1] + FiltVal];
		PixelPtr[2] = LimitTable[(INT32)PixelPtr[2] - FiltVal];

        if(!UseHighVariance)
        {
            FiltVal >>= 1;

            PixelPtr[0] = LimitTable[(INT32)PixelPtr[0] + FiltVal];
		    PixelPtr[3] = LimitTable[(INT32)PixelPtr[3] - FiltVal];
        }
		
		PixelPtr += LineLength;
	}
	************************************************************/

	UINT32 FLimit = DeblockLimitValuesV2[pbi->FrameQIndex];
	__declspec(align(16)) unsigned char WorkingBuffer[32];
	(void)BoundingValuePtr;	

	__asm 
	{
		mov			eax,		FLimit					// Flimit
		xor			ecx,		ecx						// clear ecx

		mov			edx,		LineLength				// pitch
		mov			esi,		PixelPtr				// src and des pointer

		sub			ecx,		edx						// negative pitch
		lea			esi,		[esi + edx]				// next line

		movd		mm0,		[esi + ecx + -2]		// xx xx xx xx 01 00 xx xx	
		movd		mm4,		[esi + -2]				// xx xx xx xx 11 10 xx xx

		movd		mm2,		[esi + ecx + 2]			// xx xx xx xx xx xx 03 02
		punpcklbw   mm0,		mm4						// 11 01 10 00 xx xx xx xx

		movd		mm3,		[esi +  2]				// xx xx xx xx xx xx 13 12
		punpcklbw	mm2,		mm3						// xx xx xx	xx 13 03 12 02

		movd		mm1,		[esi+ edx + -2]			// xx xx xx xx 21 20 xx xx
		movd		mm5,		[esi+ edx *2 + -2]		// xx xx xx xx 31 30 xx xx

		movd		mm6,		[esi+ edx + 2]			// xx xx xx xx xx xx 23 22 	
		punpcklbw	mm1,		mm5						// 31 21 30 20 xx xx xx xx 
		
		movd		mm7,		[esi+ edx*2 + 2]		// xx xx xx xx xx xx 33 32
		punpckhwd	mm0,		mm1						// 31 21 11 01 30 20 10 00
		
		punpcklbw	mm6,		mm7						// xx xx xx xx 33 23 32 22
		lea			edi,		WorkingBuffer

		punpcklwd	mm2,		mm6						// 33 23 13 03 32 22 12 02
		lea			esi,		[esi+edx*4]				// four lines below

		movd		mm4,		[esi+ecx + -2]			// xx xx xx xx 41 40 xx xx
		movd		mm1,		[esi + -2]				// xx xx xx xx 51 50 xx xx
		
		movd		mm3,		[esi+ecx + 2]			// xx xx xx xx xx xx 43 42
		punpcklbw	mm4,		mm1						// 51 41 50 40 xx xx xx xx

		movd		mm6,		[esi + 2]				// xx xx xx xx xx xx 53 52
		movd		mm1,		[esi + edx + -2]		// xx xx xx xx 61 60 xx xx

		punpcklbw	mm3,		mm6						// xx xx xx xx 53 43 52 42
		movq		mm5,		[esi + edx*2 -2]		// xx xx xx xx 71 70 xx xx

		movq		mm6,		[esi + edx +2]			// xx xx xx xx xx xx 63 62
		punpcklbw	mm1,		mm5						// 71 61 70 60 xx xx xx xx
		
		movq		mm7,		[esi + edx*2 + 2]		// xx xx xx xx xx xx 73 72
		punpckhwd	mm4,		mm1						// 71 61 51 41 70 60 50 40

		punpcklbw	mm6,		mm7						// xx xx xx xx 73 63 72 62
		movq		mm1,		mm0						// 31 21 11 01 30 20 10 00

		punpcklwd	mm3,		mm6						// 73 63 53 43 72 62 52 42
		movq		mm7,		mm2						// 33 23 13 03 32 22 12 02

		punpckldq	mm0,		mm4						// 70 60 50 40 30 20 10 00
		movq		[edi],		mm0						// save	p[0]					

		punpckhdq	mm1,		mm4						// 71 61 51 41 31 21 11 01
		movq		mm4,		mm0						// copy of p[0]

		movq		[edi+8],	mm1						// save p[1]
		punpckldq	mm2,		mm3						// 72 62 52 42 32 22 12 02

		movq		mm5,		mm1						// copy of p[1]
		movq		[edi+16],	mm2						// save p[2]

		punpckhdq	mm7,		mm3						// 73 63 53 43 33 23 13 03
		movq		mm6,		mm2						// copy of p[2]

		movq		[edi+24],	mm7						// save p[3]

		//	mm0, 4 ---> p[0]
		//  mm1, 5 ---> p[1]		
		//  mm2, 6 ---> p[2]
		//  mm7, 3 ---> p[3]

		movq		mm1,		eightNOnes				// mm1 = FFFFFFFFFFFFFFFFF
		psubb		mm0,		mm5						// p[0]-p[1]

		movq		mm7,		eightOnes				// mm7 = 0101010101010101
		pcmpgtb		mm1,		mm0						// p[0]-p[1]<-1?

		pcmpgtb		mm0,		mm7						// p[0]-p[1]>1?
		movq		mm3,		eightNOnes				// mm1 = FFFFFFFFFFFFFFFFF

		por			mm0,		mm1						// abs(p[0]-p[1])>1?
		movq		mm1,		mm7						// mm1 = 0101010101010101

		movq		mm7,		[edi+24]				// p[3]
		psubb		mm2,		mm7						// p[2]-p[3]

		pcmpgtb		mm3,		mm2						// p[2]-p[3]<-1?
		pcmpgtb		mm2,		mm1						// p[2]-p[3]>1?

		por			mm2,		mm3						// abs(p[3]-p[2])>1?
		movq		mm3,		eight128s				// mm3 = 8080808080808080

		por			mm0,		mm2						// mm0 = UseHighVariance

		// mm0 = UseHighVariance
		// mm4 = P[0]
		// mm5 = P[1]
		// mm6 = P[2]
		// mm7 = P[3]
		// mm3 = 8080808080808080

		pxor		mm1,		mm1						// clear mm1 for unpack
		movq		mm2,		mm5						// copy p[1]

		movq		mm3,		mm6						// ocpy of p[2]
		punpcklbw	mm2,		mm1						// low four p[1]

		punpcklbw	mm3,		mm1						// low four p[2]
		psubw		mm3,		mm2						// low four p[2]-p[1]

		punpckhbw	mm5,		mm1						// high four p[1]
		movq		mm2,		mm3						// low p[2]-p[1]

		punpckhbw	mm6,		mm1						// high four p[2]
		paddw		mm3,		mm3						// 2*(p[2]-p[1]) low four

		psubw		mm6,		mm5						// high four p[2]-p[1]
		paddw		mm2,		mm3						// 3*(p[2]-p[1]) low four

		movq		mm5,		mm6						// high four p[2]-p[1]
		movq		mm3,		mm4						// copy of p[0]

		paddw		mm6,		mm6						// 2*(p[2]-p[1]) highfour
		punpcklbw	mm3,		mm1						// low four p[0]
		
		paddw		mm5,		mm6						// 3*(p[2]-p[1]) highfour
		punpckhbw	mm4,		mm1						// high four p[0]

		movq		mm6,		mm7						// copy of p[3]		
		punpcklbw	mm7,		mm1						// low four p[3]

		punpckhbw	mm6,		mm1						// high four p[3]
		psubw		mm3,		mm7						// low four p[0]-p[3]

		punpcklbw	mm1,		mm0						// UseHighVariance Low four
		pxor		mm7,		mm7						// clear mm7 for unpack 

		psraw		mm1,		8						// FFFF or 0000
		punpckhbw	mm7,		mm0						// UseHighVaraince high four

		psubw		mm4,		mm6						// high four p[0]-p[3]
		psraw		mm7,		8						// FFFF or 0000

		pand		mm3,		mm1						// And UseHighVariance
		pand		mm4,		mm7						// And UseHighVariance

		paddw		mm2,		mm3						// Low four 3*(p[2]-p(1)+ (p[0]-p[3])*Flag
		paddw		mm4,		mm5						// High four 3*(p[2]-p(1)+ (p[0]-p[3])*Flag

		paddw		mm2,		fourFours				// adjust before shift
		movd		mm1,		eax						// Flimit

		paddw		mm4,		fourFours				// adjust before shift
		psraw		mm2,		3						// shift
		
		psraw		mm4,		3						// shift
		movq		mm3,		mm2						// copy of low four

		punpcklwd	mm1,		mm1						// Flimit Flimit
		movq		mm5,		mm4						// copy of Highfour

		punpckldq	mm1,		mm1						// Four Flimit
		psraw		mm2,		15						// FFFF or 0000

		movq		mm6,		mm1						// copy of FLimit
		psraw		mm4,		15						// FFFF or 0000

		pxor		mm3,		mm2						
		psubsw		mm3,		mm2						// abs(FiltVal) for Low

		pxor		mm5,		mm4
		psubsw		mm5,		mm4						// abs(FiltVal) for Low

		por			mm2,		fourOnes				// -1 or -1 for sign
		por			mm4,		fourOnes				// -1 or +1 for sign

		//   mm0 = UseHIghVariance?
		//   mm1 = FLimit in shorts
		//	 mm2 = sign for lower four FiltVal
		//	 mm3 = abs for lower four FiltVal
		//	 mm4 = sign for higher four FiltVal
		//	 mm5 = abs for higher four FiltVal
		movq		mm6,		mm1						// copy of Flimit
		psubusw		mm1,		mm3						// Flimit - abs(FiltVal)

		psubusw		mm3,		mm6						// abs(Filtval) -FLimit
		por			mm3,		mm1						// abs(Flimit-abs(FiltVal)

		movq		mm1,		mm6						// Flimit
		psubusw		mm1,		mm3						// Flimit-abs(FLimit-abs(FiltVal)

		movq		mm3,		mm6						// copy of the Flimit
		pmullw		mm1,		mm2						// Get the sign back

		psubusw		mm3,		mm5						// Flimit-abs(Filtval)
		psubusw		mm5,		mm6						// abs(Filtval)-Flimit)

		por			mm5,		mm3						// abs(Flimit-abs(FiltVal)
		movq		mm3,		mm6						// Flimit

		psubusw		mm3,		mm5						// Flimit-abs(FLimit-abs(FiltVal)
		pmullw		mm4,		mm3						// Get the sign back

		movq		mm2,		mm4

		// mm0 = UseHighVariance
		// mm1 = low four
		// mm2 = high four

		movq		mm5,		[edi+8]					// p[1]
		movq		mm3,		mm1						// copy of low four

		movq		mm4,		eight128s				// 128 for offset
		packsswb	mm1,		mm2						// pack to chars

		movq		mm6,		[edi+16]				// p[2]
		psubb		mm5,		mm4						// unsigned -> signed

		psubb		mm6,		mm4						// unsigned -> signed
		paddsb		mm5,		mm1						// p[1]+delta

		psubsb		mm6,		mm1						// p[1]-delta
		paddb		mm5,		mm4						// offset back
		
		paddb		mm6,		mm4						// offset back
		movq		mm1,		[edi]					// p[0]
		
		psraw		mm3,		1						// delta/2
		psraw		mm2,		1						// delta/2
		
		movq		mm7,		[edi+24]				// p[3]
		packsswb	mm3,		mm2						// pack to chars

		psubb		mm1,		mm4						// unsigned -> signed
		pandn		mm0,		mm3						// and !UseHighVariance
		
		psubb		mm7,		mm4						// unsigned -> signed
		psubsb		mm7,		mm0						// 

		paddsb		mm0,		mm1						// 
		paddb		mm7,		mm4						// offset back

		paddb		mm0,		mm4						// offset back
		lea			esi,		[esi+ecx*4]				// esi now point to the second line

		//done with calculation, now write back the resutls
		// mm0 -> 7060504030201000
		// mm5 -> 7161514131211101
		// mm6 -> 7262524232221202
		// mm7 -> 7363534333231303

		movq		mm4,		mm0						// 7060504030201000
		punpcklbw	mm0,		mm5						// 3130212011100100

		punpckhbw	mm4,		mm5						// 7170616051504140
		movq		mm2,		mm6						// 7262524232221202

		punpcklbw	mm2,		mm7						// 3332232213120302
		punpckhbw	mm6,		mm7						// 7372636253524342

		movq		mm1,		mm0						// 3130212011100100
		punpcklwd	mm0,		mm2						// 1312111003020100

		movd		[esi+ecx],	mm0						// write 03020100
		punpckhwd	mm1,		mm2						// 3332313023222120

		psrlq		mm0,		32						// xxxxxxxx13121110
		movd		[esi],		mm0						// write 13121110

		movq		mm5,		mm4						// 7170717051504140		
		punpcklwd	mm4,		mm6						// 5352515043424140
		
		movd		[esi+edx],	mm1						// write 23222120
		psrlq		mm1,		32						// xxxxxxxx33323130

		punpckhwd	mm5,		mm6						// 7372717063626160
		movd		[esi+edx*2],mm1						// write 33323130

		lea			esi,		[esi+edx*4]				// fifth line
		movd		[esi+ecx],	mm4						// write 43424140

		psrlq		mm4,		32						// xxxxxxxx53525150
		movd		[esi],		mm4						// write 53525150

		movd		[esi+edx],	mm5						// write 63626160
		psrlq		mm5,		32						// xxxxxxxx73727170

		movd		[esi+edx*2], mm5					// write 73727170
	
	}

}

/****************************************************************************
 * 
 *  ROUTINE       :     FilterVert_Simple_MMX
 *
 *  INPUTS        :     None
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Applies a loop filter to a horizontal edge vertically
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void FilterVert_Simple_MMX(
						   POSTPROC_INSTANCE *pbi, 
						   UINT8 * PixelPtr,
						   INT32 Pitch,
							INT32 *BoundingValuePtr

						   )
{

	/************************************************************	
		The following code in comments is the C version of the 
		function, provided here for reference  
	 ************************************************************
	
	INT32 j;
	INT32 FiltVal;
    UINT8 * LimitTable = &LimitVal_VP31[VAL_RANGE];
	for ( j = 0; j < 8; j++ )
	{            
        INT32 UseHighVariance;

        FiltVal = ( ( (INT32) PixelPtr[0] * 3 ) - 
			( (INT32)PixelPtr[- LineLength] * 3 ));

        UseHighVariance =  abs(PixelPtr[- (2 * LineLength)] - PixelPtr[- LineLength]) > 1 ||
                 abs(PixelPtr[0] - PixelPtr[LineLength]) > 1;

        if(UseHighVariance)
        {
		    FiltVal += ( (INT32)PixelPtr[- (2 * LineLength)] ) - 
			    ( (INT32)PixelPtr[LineLength] );
        }

        
		FiltVal = BoundingValuePtr[(FiltVal + 4) >> 3];
		
		PixelPtr[- LineLength] = LimitTable[(INT32)PixelPtr[- LineLength] + FiltVal];
		PixelPtr[0] = LimitTable[(INT32)PixelPtr[0] - FiltVal];
		
        if(!UseHighVariance)
        {
            FiltVal >>=1 ;
            
            PixelPtr[- 2* LineLength] = LimitTable[(INT32)PixelPtr[- 2 * LineLength] + FiltVal];
            PixelPtr[LineLength] = LimitTable[(INT32)PixelPtr[LineLength] - FiltVal];
        }

        PixelPtr ++;
	}
	*************************************************************/


	UINT32 FLimit = DeblockLimitValuesV2[pbi->FrameQIndex];
	(void)BoundingValuePtr;	
    __asm
    {
    
		mov			eax,		FLimit					// Flimit Values
		xor			ecx,		ecx						// clear ecx for negative pitch

		mov			edx,		Pitch					// Pitch 
		mov			esi,		PixelPtr				// Pointer to Src and Destination

		sub			ecx,		edx						// negative pitch
		movq		mm2,		[esi]					// p[2]

		movq		mm7,		eightOnes				// mm7 = 0101010101010101
		movq		mm0,		[esi+ecx*2]				// p[0]

		movq		mm6,		mm2						// Make a copy 
		movq		mm5,		[esi+ecx]				// p[1]

		movq		mm4,		mm0						// Make a copy
		movq		mm1,		eightNOnes				// mm1 = FFFFFFFFFFFFFFFFF

		psubb		mm0,		mm5						// p[0]-p[1]
		pcmpgtb		mm1,		mm0						// p[0]-p[1]<-1?

		pcmpgtb		mm0,		mm7						// p[0]-p[1]>1?
		movq		mm3,		eightNOnes				// mm1 = FFFFFFFFFFFFFFFFF

		por			mm0,		mm1						// abs(p[0]-p[1])>1?
		movq		mm1,		mm7						// mm1 = 0101010101010101

		movq		mm7,		[esi+edx]				// p[3]
		psubb		mm2,		mm7						// p[2]-p[3]

		pcmpgtb		mm3,		mm2						// p[2]-p[3]<-1?
		pcmpgtb		mm2,		mm1						// p[2]-p[3]>1?

		por			mm2,		mm3						// abs(p[3]-p[2])>1?
		movq		mm3,		eight128s				// mm3 = 8080808080808080

		por			mm0,		mm2						// mm0 = UseHighVariance

		// mm0 = UseHighVariance
		// mm4 = P[0]
		// mm5 = P[1]
		// mm6 = P[2]
		// mm7 = P[3]
		// mm3 = 8080808080808080

		pxor		mm1,		mm1						// clear mm1 for unpack
		movq		mm2,		mm5						// copy p[1]

		movq		mm3,		mm6						// ocpy of p[2]
		punpcklbw	mm2,		mm1						// low four p[1]

		punpcklbw	mm3,		mm1						// low four p[2]
		psubw		mm3,		mm2						// low four p[2]-p[1]

		punpckhbw	mm5,		mm1						// high four p[1]
		movq		mm2,		mm3						// low p[2]-p[1]

		punpckhbw	mm6,		mm1						// high four p[2]
		paddw		mm3,		mm3						// 2*(p[2]-p[1]) low four

		psubw		mm6,		mm5						// high four p[2]-p[1]
		paddw		mm2,		mm3						// 3*(p[2]-p[1]) low four

		movq		mm5,		mm6						// high four p[2]-p[1]
		movq		mm3,		mm4						// copy of p[0]

		paddw		mm6,		mm6						// 2*(p[2]-p[1]) highfour
		punpcklbw	mm3,		mm1						// low four p[0]
		
		paddw		mm5,		mm6						// 3*(p[2]-p[1]) highfour
		punpckhbw	mm4,		mm1						// high four p[0]

		movq		mm6,		mm7						// copy of p[3]		
		punpcklbw	mm7,		mm1						// low four p[3]

		punpckhbw	mm6,		mm1						// high four p[3]
		psubw		mm3,		mm7						// low four p[0]-p[3]

		punpcklbw	mm1,		mm0						// UseHighVariance Low four
		pxor		mm7,		mm7						// clear mm7 for unpack 

		psraw		mm1,		8						// FFFF or 0000
		punpckhbw	mm7,		mm0						// UseHighVaraince high four

		psubw		mm4,		mm6						// high four p[0]-p[3]
		psraw		mm7,		8						// FFFF or 0000

		pand		mm3,		mm1						// And UseHighVariance
		pand		mm4,		mm7						// And UseHighVariance

		paddw		mm2,		mm3						// Low four 3*(p[2]-p(1)+ (p[0]-p[3])*Flag
		paddw		mm4,		mm5						// High four 3*(p[2]-p(1)+ (p[0]-p[3])*Flag

		paddw		mm2,		fourFours				// adjust before shift
		paddw		mm4,		fourFours				// adjust before shift
		
		movd		mm1,		eax						// Flimit
		psraw		mm2,		3						// shift
		
		psraw		mm4,		3						// shift
		movq		mm3,		mm2						// copy of low four

		punpcklwd	mm1,		mm1						// Flimit Flimit
		movq		mm5,		mm4						// copy of Highfour

		punpckldq	mm1,		mm1						// Four Flimit
		psraw		mm2,		15						// FFFF or 0000

		movq		mm6,		mm1						// copy of FLimit
		psraw		mm4,		15						// FFFF or 0000

		pxor		mm3,		mm2						
		psubsw		mm3,		mm2						// abs(FiltVal) for Low

		pxor		mm5,		mm4
		psubsw		mm5,		mm4						// abs(FiltVal) for Low

		por			mm2,		fourOnes				// -1 or -1 for sign
		por			mm4,		fourOnes				// -1 or +1 for sign
	
		/*
		THE FOLLOWING CODE TRIED TO DO IT IN CHARS, BUT GENERATES DIFFERENT RESULTS
		THAN THE C VERSION BECAUSE OF OVERFLOW IN VERY RARE CASES

		pxor		mm4,		mm3						// offset all the pixels by 128		
		pxor		mm5,		mm3

		pxor		mm6,		mm3
		pxor		mm7,		mm3
		
		psubsb		mm6,		mm5						// p[2]-p[1]
		psubsb		mm4,		mm7						// p[0]-p[3]

		movq		mm2,		mm6						// Make a copy p[2] - p[1]
		paddsb		mm6,		mm6						// 2 * p[2] - p[1]

		pand		mm4,		mm0						// UseHighVariance * (p[0]-p[3])
		paddsb		mm2,		mm6						// 3*(p[2]-p[1])

		paddsb		mm4,		mm2						// 3*(p[2]-p(1)+ (p[0]-p[3])*Flag
		paddsb		mm4,		eightFours				// adjust before shift
		
		pxor		mm7,		mm7						// clear mm7 for unpack
		movd		mm1,		eax						// FLimit

		pxor 		mm2,		mm2						// make a copy
		punpcklwd	mm1,		mm1						// FLimit FLimit

		punpcklbw	mm2,		mm4						// Unpack to shorts
		punpckldq	mm1,		mm1						// 4 Flimit in short

		punpckhbw	mm7,		mm4						// Unpcak to shorts
		psraw		mm2,		11						// >> 3-> FiltVal low four

		psraw		mm7,		11						// >> 3-> FiltVal High four
		movq		mm3,		mm2						// make a copy of Low 4

		movq		mm4,		mm7
		pxor		mm7,		mm7

		movq		mm5,		mm4						// make a copy of high 4
		psraw		mm2,		15						// FFFF or 0000

		movq		mm6,		mm1						// copy of FLimit
		psraw		mm4,		15						// FFFF or 0000

		pxor		mm3,		mm2						
		psubsw		mm3,		mm2						// abs(FiltVal) for Low

		pxor		mm5,		mm4
		psubsw		mm5,		mm4						// abs(FiltVal) for Low

		por			mm2,		fourOnes				// -1 or -1 for sign
		por			mm4,		fourOnes				// -1 or +1 for sign
	
		*/
		//   mm0 = UseHIghVariance?
		//   mm1 = FLimit in shorts
		//	 mm2 = sign for lower four FiltVal
		//	 mm3 = abs for lower four FiltVal
		//	 mm4 = sign for higher four FiltVal
		//	 mm5 = abs for higher four FiltVal
		
		movq		mm6,		mm1						// copy of Flimit
		psubusw		mm1,		mm3						// Flimit - abs(FiltVal)

		psubusw		mm3,		mm6						// abs(Filtval) -FLimit
		por			mm3,		mm1						// abs(Flimit-abs(FiltVal)

		movq		mm1,		mm6						// Flimit
		psubusw		mm1,		mm3						// Flimit-abs(FLimit-abs(FiltVal)

		movq		mm3,		mm6						// copy of the Flimit
		pmullw		mm2,		mm1						// Get the sign back

		psubusw		mm3,		mm5						// Flimit-abs(Filtval)
		psubusw		mm5,		mm6						// abs(Filtval)-Flimit)

		por			mm5,		mm3						// abs(Flimit-abs(FiltVal)
		movq		mm3,		mm6						// Flimit

		psubusw		mm3,		mm5						// Flimit-abs(FLimit-abs(FiltVal)
		pmullw		mm4,		mm3						// Get the sign back

		// mm0 = UserHighVaraince
		// mm2 = Final value with sign for lower four
		// mm4 = Final value with sing for higher four
		movq		mm5,		[esi+ecx]				// p[1]
		movq		mm1,		mm2						// make a copy of low four

		movq		mm7,		eight128s				// 128 for offset
		packsswb	mm2,		mm4						// pack to chars for operation

		movq		mm6,		[esi]					// p[2]
		psubb		mm5,		mm7						// unsigned -> signed

		psubb		mm6,		mm7						// unsgined -> signed
		paddsb		mm5,		mm2						// p[1] + Delta
						
		psubsb		mm6,		mm2						// p[2] - Delta
		paddb		mm5,		mm7						// offset back

		paddb		mm6,		mm7						// offset back 
		movq		[esi+ecx],	mm5						// write out p[1]
		psraw		mm1,		1						// Delta/2

		psraw		mm4,		1						// Delta/2
		movq		[esi],		mm6						// write out p[2]

		movq		mm2,		[esi+ecx*2]				// p[0]
		packsswb	mm1,		mm4						// pack to chars
		
		movq		mm3,		[esi+edx]				// p[3]
		pandn		mm0,		mm1						// and !UseHighVaraince

		psubb		mm2,		mm7						// unsigned -> signed 
		psubb		mm3,		mm7						// unsigned -> signed 

		paddsb		mm2,		mm0						//  
		paddb		mm2,		mm7						// offset back

		movq		[esi+ecx*2],	mm2					// write p[0]
		psubsb		mm3,		mm0						//

		paddb		mm3,		mm7						// offset back
		movq		[esi+edx],	mm3						// write p[3]

	}

}
