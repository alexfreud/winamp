/****************************************************************************
 *
 *   Module Title :     DeblockOpt.c
 *
 *   Description  :     Optimized functions for deblocking 
 *
 *   AUTHOR       :     Yaowu Xu
 *
 *****************************************************************************
 *  Revision History
 * 
 *      1.04 YWX 21-Mar-02 bug fixed in functions using abs diff criteria
 *      1.03 YWX 15-Jun-01 Added new 7 tap filter in deblocking 
 *      1.02 YWX 02-May-01 Changed to use sum of abs diff to replace variance
 *	1.01 YWX 17-Nov-00 Re-arranged loop inside deblockNonFilteredBand()
 *	1.00 YWX 02-Nov-00 Configuration baseline from old PPoptfunctions.c
 *
 *****************************************************************************
 */


/****************************************************************************
 *  Header Frames
 *****************************************************************************
 */

#ifdef _MSC_VER 
#pragma warning(disable:4799)
#pragma warning(disable:4731)
#endif


#define STRICT              /* Strict type checking. */

#include "postp.h"
#include <stdio.h>
#include <stdlib.h>

/****************************************************************************
 *  Module constants.
 *****************************************************************************
 */        
#if defined(_WIN32_WCE)
#pragma pack(16)
static short Eight128s[] = {128, 128, 128, 128,128, 128, 128, 128 };
static short Eight64s[] = {64, 64, 64, 64, 64, 64, 64, 64  };
static short EightThrees[]= {3, 3, 3, 3, 3, 3, 3, 3};
static short EightFours[]= {4, 4, 4, 4, 4, 4, 4, 4};
static short Four128s[] = {128, 128, 128, 128};
static short Four64s[] = {64, 64, 64, 64 };
static short FourThrees[]= {3, 3, 3, 3};
static short FourFours[]= {4, 4, 4, 4};
static short FourOnes[]= { 1, 1, 1, 1};
static unsigned char  Eight128c[] = {128, 128, 128, 128,128, 128, 128, 128 };
#pragma pack()
#else
__declspec(align(16)) static short Eight128s[] = {128, 128, 128, 128,128, 128, 128, 128 };
__declspec(align(16)) static short Eight64s[] = {64, 64, 64, 64, 64, 64, 64, 64  };
__declspec(align(16)) static short EightThrees[]= {3, 3, 3, 3, 3, 3, 3, 3};
__declspec(align(16)) static short EightFours[]= {4, 4, 4, 4, 4, 4, 4, 4};
__declspec(align(16)) static short Four128s[] = {128, 128, 128, 128};
__declspec(align(16)) static short Four64s[] = {64, 64, 64, 64 };
__declspec(align(16)) static short FourThrees[]= {3, 3, 3, 3};
__declspec(align(16)) static short FourFours[]= {4, 4, 4, 4};
__declspec(align(16)) static short FourOnes[]= { 1, 1, 1, 1};
__declspec(align(16)) static unsigned char  Eight128c[] = {128, 128, 128, 128,128, 128, 128, 128 };
#endif

/****************************************************************************
 *  Explicit Imports
 *****************************************************************************
 */              

extern UINT32 *DeblockLimitValuesV2;
/****************************************************************************
 *  Exported Global Variables
 *****************************************************************************
 */

/****************************************************************************
 *  Exported Functions
 *****************************************************************************
 */              
extern double gaussian(double sigma, double mu, double x);

/****************************************************************************
 *  Module Statics
 *****************************************************************************
 */
/****************************************************************************
 * 
 *  ROUTINE       :     SetupBoundingValueArray_ForMMX
 *
 *  INPUTS        :      
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Applies a loop filter to the edge pixels of coded blocks.
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
INT32 *SetupDeblockValueArray_ForMMX(POSTPROC_INSTANCE *pbi, INT32 FLimit)
{
    INT32 * BoundingValuePtr;

    /* 
        Since the FiltBoundingValue array is currently only used in the generic version, we are going
        to reuse this memory for our own purposes.
        2 longs for limit, 2 longs for _4ONES, 2 longs for LFABS_MMX, and 8 longs for temp work storage 
    */
   BoundingValuePtr = (INT32 *)((UINT32)(&pbi->DeblockBoundingValue[256]) & 0xffffffe0);    

    //expand for mmx code
    BoundingValuePtr[0] = BoundingValuePtr[1] = FLimit * 0x00010001;
    BoundingValuePtr[2] = BoundingValuePtr[3] = 0x00010001;
    BoundingValuePtr[4] = BoundingValuePtr[5] = 0x00040004;

    return BoundingValuePtr;
}

/****************************************************************************
 * 
 *  ROUTINE       :     DeblockLoopFilteredBand_MMX
 *
 *  INPUTS        :     None
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Filter both horizontal and vertical edge in a band
 *
 *  SPECIAL NOTES :     
 *
 *	REFERENCE	  :		
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

void DeblockLoopFilteredBand_MMX(
                                 POSTPROC_INSTANCE *pbi, 
                                 UINT8 *SrcPtr, 
                                 UINT8 *DesPtr,
                                 UINT32 PlaneLineStep, 
                                 UINT32 FragAcross,
                                 UINT32 StartFrag,
                                 UINT32 *QuantScale
							    )
{
	UINT32 j;
	UINT32 CurrentFrag=StartFrag;
	UINT32 QStep;
	UINT8 *Src, *Des;
	UINT32 Var1, Var2;

#if defined(_WIN32_WCE)
#pragma pack(16)
short QStepMmx[4];
short FLimitMmx[4];
short Rows[80];
short NewRows[64];

unsigned short Variance11[4];
unsigned short Variance12[4];
unsigned short Variance21[4];
unsigned short Variance22[4];
#pragma pack()
#else
__declspec(align(16)) short QStepMmx[4];
__declspec(align(16)) short FLimitMmx[4];
__declspec(align(16)) short Rows[80];
__declspec(align(16)) short NewRows[64];

__declspec(align(16)) unsigned short Variance11[4];
__declspec(align(16)) unsigned short Variance12[4];
__declspec(align(16)) unsigned short Variance21[4];
__declspec(align(16)) unsigned short Variance22[4];
#endif

	Src=SrcPtr;
	Des=DesPtr;

	while(CurrentFrag < StartFrag + FragAcross )
    {
        
        QStep = QuantScale[ pbi->FragQIndex[CurrentFrag+FragAcross]];
        if( QStep > 3 )
        {
            QStepMmx[0] = (INT16)QStep;
            QStepMmx[1] = (INT16)QStep;
            QStepMmx[2] = (INT16)QStep;
            QStepMmx[3] = (INT16)QStep;
			__asm 
			{
				
				/* Save the registers */
				push		eax
				push		ebp
				push		ecx			
				push		edx
				push		esi
				push		edi
				
				
				/* Calculate the FLimit and store FLimit and QStep */					
				
				movq		mm0,	QStepMmx			/* mm0 = QStep */				
				movq		mm1,	FourThrees			/* mm1 = 03030303 */			

                pmullw		mm1,	mm0					/* mm1 = QStep * 3 */			
				pmullw		mm1,	mm0					/* mm1 = QStep * QStep * 3 */	
				
				psrlw		mm1,	5					/* mm1 = FLimit */				
				movq		[FLimitMmx], mm1			/* Save FLimit */				
				
				/* Copy the data to the intermediate buffer */							
				
				mov			eax,	Src					/* eax = Src */					
				xor			edx,	edx					/* clear edx */					
				
				lea			esi,	NewRows				/* esi = NewRows */

				lea			edi,	Rows				/* edi = Rows */				
				mov			ecx,	PlaneLineStep		/* ecx = Pitch */				
				
				pxor		mm7,	mm7					/* Clear mm7 */					
				sub			edx,	ecx					/* edx = -Pitch */				
				
				lea			eax,	[eax + edx * 4 ]	/* eax = Src - 4*Pitch */		
				movq		mm0,	[eax + edx]			/* mm0 = Src[-5*Pitch] */		
				
				movq		mm1,	mm0					/* mm1 = mm0 */					
				punpcklbw	mm0,	mm7					/* Lower Four -5 */				
				
				movq		mm2,	[eax]				/* mm2 = Src[-4*Pitch] */		
				movq		mm3,	mm2					/* mm3 = mm2 */					
				
				punpckhbw	mm1,	mm7					/* Higher Four -5 */			
				movq		[edi],	mm0					/* Write Lower Four of -5 */	
				
				punpcklbw	mm2,	mm7					/* Lower Four -4 */				
				punpckhbw	mm3,	mm7					/* higher Four -4 */			
				
				movq		[edi+8], mm1				/* Write Higher Four of -5 */	
				movq		mm4,	[eax + ecx]			/* mm4 = Src[-3*Pitch] */		
				
				movq		[edi+16], mm2				/* Write Lower -4 */			
				movq		[edi+24], mm3				/* write hight -4 */			
				
				movq		mm5,	mm4					/* mm5 = mm4 */					
				punpcklbw	mm4,	mm7					/* lower four -3 */				
				
				movq		mm0,	[eax + ecx *2]		/* mm0 = Src[-2*Pitch] */		
				punpckhbw	mm5,	mm7					/* higher four -3 */			
				
				movq		mm1,	mm0					/* mm1 = mm0 */					
				movq		[edi+32], mm4				/* write Lower -3 */			
				
				punpcklbw	mm0,	mm7					/* lower four -2 */				
				lea			eax,	[eax + ecx *4]		/* eax = Src */					
				
				movq		[edi+40], mm5				/* write Higher -3 */			
				punpckhbw	mm1,	mm7					/* higher four -2 */			
				
				movq		mm2,	[eax + edx]			/* mm2 = Src[-Pitch] */			
				movq		[edi+48], mm0				/* lower -2	*/					
				
				movq		mm3,	mm2					/* mm3 = mm2 */					
				punpcklbw	mm2,	mm7					/* lower -1 */					
				
				movq		[edi+56], mm1				/* higher -2 */					
				punpckhbw	mm3,	mm7					/* Higher -1 */					
				
				movq		mm4,	[eax]				/* mm4 = Src[0] */				
				movq		[edi+64], mm2				/* Lower -1 */					
				
				movq		mm5,	mm4					/* mm5 = mm4 */					
				movq		[edi+72], mm3				/* Higher -1 */					
				
				punpcklbw	mm4,	mm7					/* lower 0 */					
				punpckhbw	mm5,	mm7					/* higher 0 */					
				
				movq		mm0,	[eax + ecx]			/* mm0 = Src[Pitch] */			
				movq		[edi+80], mm4				/* write lower 0 */				
				
				movq		mm1,	mm0					/* mm1 = mm0 */					
				movq		[edi+88], mm5				/* write higher 0 */			
				
				punpcklbw	mm0,	mm7					/* lower 1 */					
				punpckhbw	mm1,	mm7					/* higher 1 */					
				
				movq		mm2,	[eax + ecx *2 ]     /* mm2 = Src[2*Pitch] */		
				lea			eax,	[eax + ecx *4]		/* eax = Src + 4 * Pitch  */	
				
				movq		mm3,	mm2					/* mm3 = mm2 */					
				movq		[edi+96], mm0				/* write lower 1 */				
				
				punpcklbw	mm2,	mm7					/* lower 2 */					
				punpckhbw	mm3,	mm7					/* higher 2 */					
				
				movq		mm4,	[eax + edx ]		/* mm4 = Src[3*pitch] */		
				movq		[edi+104], mm1				/* wirte higher 1 */			
				
				movq		mm5,	mm4					/* mm5 = mm4 */					
				punpcklbw	mm4,	mm7					/* Low 3	*/					
				
				movq		[edi+112], mm2				/* write lower 2 */				
				movq		[edi+120], mm3				/* write higher 2 */			
				
				movq		mm0,	[eax]				/* mm0 = Src[4*pitch] */		
				punpckhbw	mm5,	mm7					/* high 3 */					
				
				movq		mm1,	mm0					/* mm1=mm0 */					
				movq		[edi+128], mm4				/* low 3 */						
				
				punpcklbw	mm0,	mm7					/* low 4 */						
				punpckhbw	mm1,	mm7					/* high 4 */					
				
				movq		[edi+136], mm5				/* high 3 */					
				movq		[edi+144], mm0				/* low 4 */						
				
				movq		[edi+152], mm1				/* high 4 */					
				
				/* done with copying everything to intermediate buffer */				
				/* Now, compute the variances for Pixel  1-4 and 5-8 */					
		
				/* we use mm0,mm1,mm2 for 1234 and mm4, mm5, mm6 for 5-8 */				
				/* mm7 = 0, mm3 = {128, 128, 128, 128} */								
				
				pcmpeqw		mm3,	mm3					/* mm3 = FFFFFFFFFFFFFFFF */	
				psllw		mm3,	15					/* mm3 = 8000800080008000 */	
				psrlw		mm3,	8					/* mm3 = 0080008000800080 */
				
				movq		mm2,	[edi+16]			/* Pixel 1 */					
				movq		mm6,	[edi+80]			/* Pixel 5 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				movq		mm0,	mm2					/* mm0 = pixel 1 */				
				movq		mm4,	mm6					/* mm4 = pixel 5 */				
				
				pmullw		mm2,	mm2					/* mm2 = pixel1 * pixel1 */		
				pmullw		mm6,	mm6					/* mm6 = pixel5 * pixel5 */		
				
				movq		mm1,	mm2					/* mm1 = pixel1^2 */			
				movq		mm5,	mm6					/* mm5 = pixel5^2 */			
				
				movq		mm2,	[edi+32]			/* Pixel 2 */					
				movq		mm6,	[edi+96]			/* Pixel 6 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 2 */			
				paddw		mm4,	mm6					/* mm4 += pixel 6 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel2^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel6^2 */			
				
				paddw		mm1,	mm2					/* mm1 += pixel2^2 */			
				paddw		mm5,	mm6					/* mm5 += pixel6^2 */			
				
				movq		mm2,	[edi+48]			/* Pixel 3 */					
				movq		mm6,	[edi+112]			/* Pixel 7 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 3 */			
				paddw		mm4,	mm6					/* mm4 += pixel 7 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel3^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel7^2 */			
				
				paddw		mm1,	mm2					/* mm1 += pixel3^2 */			
				paddw		mm5,	mm6					/* mm5 += pixel7^2 */			
				
				movq		mm2,	[edi+64]			/* Pixel 4 */					
				movq		mm6,	[edi+128]			/* Pixel 8 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 4 */			
				paddw		mm4,	mm6					/* mm4 += pixel 8 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel4^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel8^2 */			
				
				paddw		mm1,	mm2					/* mm1 = pixel4^2 */			
				paddw		mm5,	mm6					/* mm5 = pixel8^2 */			
				
				/* mm0 = x1^2 + x2^2 + x3^2 + x4^2 */									
				/* mm1 = x1 + x2 + x3 + x4 */											
				/* mm4 = x5^2 + x6^2 + x7^2 + x8^2 */									
				/* mm5 = x5 + x6 + x7 + x8 */											
				
				movq		mm7,	mm3					/* mm7 = mm3 */					
				psrlw		mm7,	7					/* mm7 = 0001000100010001 */	
				
				movq		mm2,	mm0					/* make copy of sum1 */			
				movq		mm6,	mm4					/* make copy of sum2 */			
				
				paddw		mm0,	mm7					/* (sum1 + 1) */				
				paddw		mm4,	mm7					/* (sum2 + 1) */				
				
				psraw		mm2,	1					/* sum1 /2 */					
				psraw		mm6,	1					/* sum2 /2 */					
				
				psraw		mm0,	1					/* (sum1 + 1)/2 */				
				psraw		mm4,	1					/* (sum2 + 1)/2 */				
				
				pmullw		mm2,	mm0					/* (sum1)/2*(sum1+1)/2 */		
				pmullw		mm6,	mm4					/* (sum2)/2*(sum2+1)/2 */		
				
				psubw		mm1,	mm2					/* Variance 1 */				
				psubw		mm5,	mm6					/* Variance 2 */				
				
				movq		mm7,	FLimitMmx			/* mm7 = FLimit */				
				movq		mm2,	mm1					/* copy of Varinace 1*/

				movq		mm6,	mm5					/* Variance 2 */
				movq		[Variance11], mm1			/* Save Variance1 */

				movq		[Variance21], mm5			/* Save Variance2 */
				psubw		mm1,	mm7					/* Variance 1 < Flimit? */		
				
				psubw		mm5,	mm7					/* Variance 2 < Flimit? */		
				psraw		mm2,	15					/* Variance 1 > 32768? */

				psraw		mm6,	15					/* Vaiance  2 > 32768? */	
				psraw		mm1,	15					/* FFFF/0000 for true/false */	
				
				psraw		mm5,	15					/* FFFF/0000 for true/false */	
				movq		mm7,	[edi+64]			/* mm0 = Pixel 4			*/	

				pandn		mm2,	mm1					/* Variance1<32678 && 
														   Variance1<Limit			*/
				pandn		mm6,	mm5					/* Variance2<32678 && 
														   Variance1<Limit			*/
				
				movq		mm4,	[edi+80]			/* mm4 = Pixel 5			*/	
				pand		mm6,	mm2					/* mm6 = Variance1 < Flimit */	
														/*     &&Variance2 < Flimit */	

				movq		mm2,	mm7					/* make copy of Pixel4		*/	

				psubusw		mm7,	mm4					/* 4 - 5 */						
				psubusw		mm4,	mm2					/* 5 - 4 */						
				
				por			mm7,	mm4					/* abs(4 - 5) */				
				psubw		mm7,	QStepMmx			/* abs(4-5)<QStepMmx ? */		
				
				psraw		mm7,	15					/* FFFF/0000 for True/Flase */
				pand		mm7,	mm6													
				
				/* mm7 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* now lets look at the right four colomn */							
				
				add			edi,	8					/* offset 8 to right 4 cols */	
				
				movq		mm2,	[edi+16]			/* Pixel 1 */					
				movq		mm6,	[edi+80]			/* Pixel 5 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				movq		mm0,	mm2					/* mm0 = pixel 1 */				
				movq		mm4,	mm6					/* mm4 = pixel 5 */				
				
				pmullw		mm2,	mm2					/* mm2 = pixel1 * pixel1 */		
				pmullw		mm6,	mm6					/* mm6 = pixel5 * pixel5 */		
				
				movq		mm1,	mm2					/* mm1 = pixel1^2 */			
				movq		mm5,	mm6					/* mm5 = pixel5^2 */			
				
				movq		mm2,	[edi+32]			/* Pixel 2 */					
				movq		mm6,	[edi+96]			/* Pixel 6 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 2 */			
				paddw		mm4,	mm6					/* mm4 += pixel 6 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel2^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel6^2 */			
				
				paddw		mm1,	mm2					/* mm1 += pixel2^2 */			
				paddw		mm5,	mm6					/* mm5 += pixel6^2 */			
				
				movq		mm2,	[edi+48]			/* Pixel 3 */					
				movq		mm6,	[edi+112]			/* Pixel 7 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 3 */			
				paddw		mm4,	mm6					/* mm4 += pixel 7 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel3^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel7^2 */			
				
				paddw		mm1,	mm2					/* mm1 += pixel3^2 */			
				paddw		mm5,	mm6					/* mm5 += pixel7^2 */			
				
				movq		mm2,	[edi+64]			/* Pixel 4 */					
				movq		mm6,	[edi+128]			/* Pixel 8 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 4 */			
				paddw		mm4,	mm6					/* mm4 += pixel 8 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel4^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel8^2 */			
				
				paddw		mm1,	mm2					/* mm1 = pixel4^2 */			
				paddw		mm5,	mm6					/* mm5 = pixel8^2 */			
				
				/* mm0 = x1^2 + x2^2 + x3^2 + x4^2 */									
				/* mm1 = x1 + x2 + x3 + x4 */											
				/* mm4 = x5^2 + x6^2 + x7^2 + x8^2 */									
				/* mm5 = x5 + x6 + x7 + x8 */											
				
				psrlw		mm3,	7					/* mm3 = 0001000100010001 */	
				
				movq		mm2,	mm0					/* make copy of sum1 */			
				movq		mm6,	mm4					/* make copy of sum2 */			
				
				paddw		mm0,	mm3					/* (sum1 + 1) */				
				paddw		mm4,	mm3					/* (sum2 + 1) */				
				
				psraw		mm2,	1					/* sum1 /2 */					
				psraw		mm6,	1					/* sum2 /2 */					
				
				psraw		mm0,	1					/* (sum1 + 1)/2 */				
				psraw		mm4,	1					/* (sum2 + 1)/2 */				
				
				pmullw		mm2,	mm0					/* (sum1)/2*(sum1+1)/2 */		
				pmullw		mm6,	mm4					/* (sum2)/2*(sum2+1)/2 */		
				
				psubw		mm1,	mm2					/* Variance 1 */				
				psubw		mm5,	mm6					/* Variance 2 */				

				movq		[Variance12], mm1			/* Save Variance1 */
				movq		[Variance22], mm5			/* Save Variance2 */
				
				movq		mm3,	FLimitMmx			/* mm3 = FLimit */				
				movq		mm2,	mm1					/* copy of Varinace 1*/

				movq		mm6,	mm5					/* Variance 2 */
				psubw		mm1,	mm3					/* Variance 1 < Flimit? */		
				
				psubw		mm5,	mm3					/* Variance 2 < Flimit? */		
				psraw		mm2,	15					/* Variance 1 > 32768? */

				psraw		mm6,	15					/* Vaiance  2 > 32768? */	
				psraw		mm1,	15					/* FFFF/0000 for true/false */	
				
				psraw		mm5,	15					/* FFFF/0000 for true/false */	
				movq		mm0,	[edi+64]			/* mm0 = Pixel 4			*/	

				pandn		mm2,	mm1					/* Variance1<32678 && 
														   Variance1<Limit			*/
				pandn		mm6,	mm5					/* Variance2<32678 && 
														   Variance1<Limit			*/

				movq		mm4,	[edi+80]			/* mm4 = Pixel 5			*/	
				pand		mm6,	mm2					/* mm6 = Variance1 < Flimit */	
														/*     &&Variance2 < Flimit */	
				movq		mm2,	mm0					/* make copy of Pixel4		*/	
														
				psubusw		mm0,	mm4					/* 4 - 5 */						
				psubusw		mm4,	mm2					/* 5 - 4 */						
				
				por			mm0,	mm4					/* abs(4 - 5) */				
				psubw		mm0,	QStepMmx			/* abs(4-5)<QStepMmx ? */		
				
				psraw		mm0,	15					/* FFFF/0000 for True/False */
				pand		mm0,	mm6				
				
				sub			edi,	8					/* offset edi back */			
				
				/* mm0 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* mm0 and mm7 now are in use  */										
				/* Let's do the filtering now */										
				/* p1 = (abs(Src[-4] - Src[-5]) < QStep ) ?  Src[-5] : Src[-4]; */		
				/* p2 = (abs(Src[+3] - Src[+4]) < QStep ) ?  Src[+4] : Src[+3]; */		
				
				movq		mm5,	[edi]				/* mm5 = -5 */					
				movq		mm4,	[edi + 16]			/* mm4 = -4 */					
				
				movq		mm3,	mm4					/* copy of -4 */				
				movq		mm6,	mm5					/* copy of -5 */				
				
				psubusw		mm4,	mm6					/* mm4 = [-4] - [-5] */			
				psubusw		mm5,	mm3					/* mm5 = [-5] - [-4] */			
				
				por			mm4,	mm5					/* abs([-4]-[-5] ) */			
				psubw		mm4,	QStepMmx			/* abs([-4]-[-5] )<QStep? */	
				
				psraw		mm4,	15					/* FFFF/0000 for True/False */	
				movq		mm1,	mm4					/* copy of the mm4 */			
				
				pand		mm4,	mm6					/*							*/	
				pandn		mm1,	mm3					/*							*/	
				
				por			mm1,	mm4					/* mm1 = p1					*/	
				
				/* now find P2 */														
				
				movq		mm4,	[edi+128]			/* mm4 = [3] */					
				movq		mm5,	[edi+144]			/* mm5 = [4] */					
				
				movq		mm3,	mm4					/* copy of 3 */					
				movq		mm6,	mm5					/* copy of 4 */					
				
				psubusw		mm4,	mm6					/* mm4 = [3] - [4] */			
				psubusw		mm5,	mm3					/* mm5 = [4] - [3] */			
				
				por			mm4,	mm5					/* abs([3]-[4] ) */				
				psubw		mm4,	QStepMmx			/* abs([3]-[4] )<QStep? */		
				
				psraw		mm4,	15					/* FFFF/0000 for True/False */	
				movq		mm2,	mm4					/* copy of the mm4 */			
				
				pand		mm4,	mm6					/*							*/	
				pandn		mm2,	mm3					/*							*/	
				
				por			mm2,	mm4					/* mm2 = p2					*/	
				
				/* sum = p1 + p1 + p1 + x1 + x2 + x3 + x4 + 4; */				
				/* Des[-w4] = (((sum + x1) << 1) - (x4 - x5)) >> 4; */			
				/* Des[-w4] = Src[-w4]; */												
				/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	
				
				movq		mm3,	mm1					/* mm3 = p1 */					
				paddw		mm3,	mm3					/* mm3 = p1 + p1 */				
				
				paddw		mm3,	mm1					/* mm3 = p1 + p1 + p1 */		
				movq		mm4,	[edi+16]			/* mm4 = x1 */					
				
				paddw		mm3,	[edi+32]			/* mm3 = p1+p1+p1+ x2 */		
				paddw		mm4,	[edi+48]			/* mm4 = x1+x3 */				
				
				paddw		mm3,	[edi+64]			/* mm3 += x4 */					
				paddw		mm4,	FourFours			/* mm4 = x1 + x3 + 4 */			
				
				paddw		mm3,	mm4					/* mm3 = 3*p1+x1+x2+x3+x4+4 */	
				movq		mm4,	mm3					/* mm4 = mm3 */					
				
				movq		mm5,	[edi+16]			/* mm5 = x1 */					
				paddw		mm4,	mm5					/* mm4 = sum+x1 */				
				
				psllw		mm4,	1					/* mm4 = (sum+x1)<<1 */			
				psubw		mm4,	[edi+64]			/* mm4 = (sum+x1)<<1-x4 */		
				
				paddw		mm4,	[edi+80]			/* mm4 = (sum+x1)<<1-x4+x5 */	
				psraw		mm4,	4					/* mm4 >>=4 */					
				
				psubw		mm4,	mm5					/* New Value - old Value */		
				pand		mm4,	mm7					/* And the flag */				
				
				paddw		mm4,	mm5					/* add the old value back */	
				movq		[esi],	mm4					/* Write new x1 */				
				
				/* sum += x5 -p1 */														
				/* Des[-w3]=((sum+x2)<<1-x5+x6)>>4 */									
				
				movq		mm5,	[edi+32]			/* mm5= x2 */					
				psubw		mm3,	mm1					/* sum=sum-p1 */				
				
				paddw		mm3,    [edi+80]			/* sum=sum+x5 */				
				movq		mm4,	mm5					/* copy sum */					
				
				paddw		mm4,	mm3					/* mm4=sum+x2 */				
				paddw		mm4,	mm4					/* mm4 <<= 1 */					
				
				psubw		mm4,	[edi+80]			/* mm4 =(sum+x2)<<1-x5 */		
				paddw		mm4,	[edi+96]			/* mm4 =(sum+x2)<<1-x5+x6 */	
				
				psraw		mm4,	4					/* mm4=((sum+x2)<<1-x5+x6)>>4 */
				psubw		mm4,	mm5					/* new value - old value	*/	
				
				pand		mm4,	mm7					/* And the flag */				
				paddw		mm4,	mm5					/* add the old value back */	
				
				movq		[esi+16], mm4				/* write new x2 */				
				
				/* sum += x6 - p1 */													
				/* Des[-w2]=((sum+x[3])<<1-x[6]+x[7])>>4 */								
				
				movq		mm5,	[edi+48]			/* mm5= x3 */					
				psubw		mm3,	mm1					/* sum=sum-p1 */				
				
				paddw		mm3,    [edi+96]			/* sum=sum+x6 */				
				movq		mm4,	mm5					/* copy x3 */					
				
				paddw		mm4,	mm3					/* mm4=sum+x3 */				
				paddw		mm4,	mm4					/* mm4 <<= 1 */					
				
				psubw		mm4,	[edi+96]			/* mm4 =(sum+x3)<<1-x6 */		
				paddw		mm4,	[edi+112]			/* mm4 =(sum+x3)<<1-x6+x7 */	
				
				psraw		mm4,	4					/* mm4=((sum+x3)<<1-x6+x7)>>4 */
				psubw		mm4,	mm5					/* new value - old value	*/	
				
				pand		mm4,	mm7					/* And the flag */				
				paddw		mm4,	mm5					/* add the old value back */	
				
				movq		[esi+32], mm4				/* write new x3 */				
				
				/* sum += x7 - p1 */													
				/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
				
				movq		mm5,	[edi+64]			/* mm5 = x4 */					
				psubw		mm3,	mm1					/* sum = sum-p1 */				
				
				paddw		mm3,	[edi+112]			/* sum = sum+x7 */				
				movq		mm4,	mm5					/* mm4 = x4 */					
				
				paddw		mm4,	mm3					/* mm4 = sum + x4 */			
				paddw		mm4,	mm4					/* mm4 *=2 */					
				
				paddw		mm4,	mm1					/* += p1 */						
				psubw		mm4,	[edi+16]			/* -= x1 */						
				
				psubw		mm4,	[edi+112]			/* -= x7 */						
				paddw		mm4,	[edi+128]			/* += x8 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x4 */						
				
				pand		mm4,	mm7					/* and flag */					
				paddw		mm4,	mm5					/* += x4 */						
				
				movq		[esi+48], mm4				/* write new x4 */				
				
				/* sum+= x8-x1 */														
				/* Des[0]=((sum+x5)<<1+x1-x2-x8+p2)>>4 */								
				
				movq		mm5,	[edi+80]			/* mm5 = x5 */					
				psubw		mm3,	[edi+16]			/* sum -= x1 */					
				
				paddw		mm3,	[edi+128]			/* sub += x8 */					
				movq		mm4,	mm5					/* mm4 = x5 */					
				
				paddw		mm4,	mm3					/* mm4= sum+x5 */				
				paddw		mm4,	mm4					/* mm4 *= 2 */					
				
				paddw		mm4,	[edi+16]			/* += x1 */						
				psubw		mm4,	[edi+32]			/* -= x2 */						
				
				psubw		mm4,	[edi+128]			/* -= x8 */						
				paddw		mm4,	mm2					/* += p2 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x5 */						
				
				pand		mm4,	mm7					/* and flag */					
				paddw		mm4,	mm5					/* += x5 */						
				
				movq		[esi+64], mm4				/* write new x5 */				
				
				/* sum += p2 - x2 */													
				/* Des[w1] = ((sum+x6)<<1 + x2-x3)>>4 */								
				
				movq		mm5,	[edi+96]			/* mm5 = x6 */					
				psubw		mm3,	[edi+32]			/* -= x2 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x6 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x6 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+32]			/* +=x2 */						
				psubw		mm4,	[edi+48]			/* -=x3 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x6 */						
				
				pand		mm4,	mm7					/* and flag */					
				paddw		mm4,	mm5					/* += x6 */						
				
				movq		[esi+80], mm4				/* write new x6 */				
				
				/* sum += p2 - x3 */													
				/* Des[w2] = ((sum+x7)<<1 + x3-x4)>>4 */								
				
				movq		mm5,	[edi+112]			/* mm5 = x7 */					
				psubw		mm3,	[edi+48]			/* -= x3 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x7 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x7 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+48]			/* +=x3 */						
				psubw		mm4,	[edi+64]			/* -=x4 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x7 */						
				
				pand		mm4,	mm7					/* and flag */					
				paddw		mm4,	mm5					/* += x7 */						
				
				movq		[esi+96], mm4				/* write new x7 */				
				
				/* sum += p2 - x4 */													
				/* Des[w3] = ((sum+x8)<<1 + x4-x5)>>4 */								
				
				movq		mm5,	[edi+128]			/* mm5 = x8 */					
				psubw		mm3,	[edi+64]			/* -= x4 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x8 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x8 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+64]			/* +=x4 */						
				psubw		mm4,	[edi+80]			/* -=x5 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x8 */						
				
				pand		mm4,	mm7					/* and flag */					
				paddw		mm4,	mm5					/* += x8 */						
				
				movq		[esi+112], mm4				/* write new x8 */				
				
				/* done with left four columns */										
				/* now do the righ four columns */										
				
				add			edi,	8					/* shift to right four column */
				add			esi,	8					/* shift to right four column */
				
				/* mm0 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* mm0 now are in use  */										
				/* Let's do the filtering now */										
				/* p1 = (abs(Src[-4] - Src[-5]) < QStep ) ?  Src[-5] : Src[-4]; */		
				/* p2 = (abs(Src[+3] - Src[+4]) < QStep ) ?  Src[+4] : Src[+3]; */		
				
				movq		mm5,	[edi]				/* mm5 = -5 */					
				movq		mm4,	[edi + 16]			/* mm4 = -4 */					
				
				movq		mm3,	mm4					/* copy of -4 */				
				movq		mm6,	mm5					/* copy of -5 */				
				
				psubusw		mm4,	mm6					/* mm4 = [-4] - [-5] */			
				psubusw		mm5,	mm3					/* mm5 = [-5] - [-4] */			
				
				por			mm4,	mm5					/* abs([-4]-[-5] ) */			
				psubw		mm4,	QStepMmx			/* abs([-4]-[-5] )<QStep? */	
				
				psraw		mm4,	15					/* FFFF/0000 for True/False */	
				movq		mm1,	mm4					/* copy of the mm4 */			
				
				pand		mm4,	mm6					/*							*/	
				pandn		mm1,	mm3					/*							*/	
				
				por			mm1,	mm4					/* mm1 = p1					*/	
				
				/* now find P2 */														
				
				movq		mm4, [edi+128]				/* mm4 = [3] */					
				movq		mm5, [edi+144]				/* mm5 = [4] */					
				
				movq		mm3, mm4					/* copy of 3 */					
				movq		mm6, mm5					/* copy of 4 */					
				
				psubusw		mm4, mm6					/* mm4 = [3] - [4] */			
				psubusw		mm5, mm3					/* mm5 = [4] - [3] */			
				
				por			mm4, mm5					/* abs([3]-[4] ) */				
				psubw		mm4, QStepMmx				/* abs([3]-[4] )<QStep? */		
				
				psraw		mm4, 15						/* FFFF/0000 for True/False */	
				movq		mm2, mm4					/* copy of the mm4 */			
				
				pand		mm4, mm6					/*							*/	
				pandn		mm2, mm3					/*							*/	
				
				por			mm2, mm4					/* mm2 = p2					*/	
				
				/* psum = p1 + p1 + p1 + v[1] + v[2] + v[3] + v[4] + 4; */				
				/* Des[-w4] = (((psum + v[1]) << 1) - (v[4] - v[5])) >> 4; */			
				/* Des[-w4]=Src[-w4]; */												
				/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	
				
				movq		mm3,	mm1					/* mm3 = p1 */					
				paddw		mm3,	mm3					/* mm3 = p1 + p1 */				
				
				paddw		mm3,	mm1					/* mm3 = p1 + p1 + p1 */		
				movq		mm4,	[edi+16]			/* mm4 = x1 */					
				
				paddw		mm3,	[edi+32]			/* mm3 = p1+p1+p1+ x2 */		
				paddw		mm4,	[edi+48]			/* mm4 = x1+x3 */				
				
				paddw		mm3,	[edi+64]			/* mm3 += x4 */					
				paddw		mm4,	FourFours			/* mm4 = x1 + x3 + 4 */			
				
				paddw		mm3,	mm4					/* mm3 = 3*p1+x1+x2+x3+x4+4 */	
				movq		mm4,	mm3					/* mm4 = mm3 */					
				
				movq		mm5,	[edi+16]			/* mm5 = x1 */					
				paddw		mm4,	mm5					/* mm4 = sum+x1 */				
				
				psllw		mm4,	1					/* mm4 = (sum+x1)<<1 */			
				psubw		mm4,	[edi+64]			/* mm4 = (sum+x1)<<1-x4 */		
				
				paddw		mm4,	[edi+80]			/* mm4 = (sum+x1)<<1-x4+x5 */	
				psraw		mm4,	4					/* mm4 >>=4 */					
				
				psubw		mm4,	mm5					/* New Value - old Value */		
				pand		mm4,	mm0					/* And the flag */				
				
				paddw		mm4,	mm5					/* add the old value back */	
				movq		[esi],	mm4					/* Write new x1 */				
				
				/* sum += x5 -p1 */														
				/* Des[-w3]=((sum+x2)<<1-x5+x6)>>4 */									
				
				movq		mm5,	[edi+32]			/* mm5= x2 */					
				psubw		mm3,	mm1					/* sum=sum-p1 */				
				
				paddw		mm3,    [edi+80]			/* sum=sum+x5 */				
				movq		mm4,	mm5					/* copy sum */					
				
				paddw		mm4,	mm3					/* mm4=sum+x2 */				
				paddw		mm4,	mm4					/* mm4 <<= 1 */					
				
				psubw		mm4,	[edi+80]			/* mm4 =(sum+x2)<<1-x5 */		
				paddw		mm4,	[edi+96]			/* mm4 =(sum+x2)<<1-x5+x6 */	
				
				psraw		mm4,	4					/* mm4=((sum+x2)<<1-x5+x6)>>4 */
				psubw		mm4,	mm5					/* new value - old value	*/	
				
				pand		mm4,	mm0					/* And the flag */				
				paddw		mm4,	mm5					/* add the old value back */	
				
				movq		[esi+16], mm4				/* write new x2 */				
				
				/* sum += x6 - p1 */													
				/* Des[-w2]=((sum+x[3])<<1-x[6]+x[7])>>4 */								
				
				movq		mm5,	[edi+48]			/* mm5= x3 */					
				psubw		mm3,	mm1					/* sum=sum-p1 */				
				
				paddw		mm3,    [edi+96]			/* sum=sum+x6 */				
				movq		mm4,	mm5					/* copy x3 */					
				
				paddw		mm4,	mm3					/* mm4=sum+x3 */				
				paddw		mm4,	mm4					/* mm4 <<= 1 */					
				
				psubw		mm4,	[edi+96]			/* mm4 =(sum+x3)<<1-x6 */		
				paddw		mm4,	[edi+112]			/* mm4 =(sum+x3)<<1-x6+x7 */	
				
				psraw		mm4,	4					/* mm4=((sum+x3)<<1-x6+x7)>>4 */
				psubw		mm4,	mm5					/* new value - old value	*/	
				
				pand		mm4,	mm0					/* And the flag */				
				paddw		mm4,	mm5					/* add the old value back */	
				
				movq		[esi+32], mm4				/* write new x3 */				
				
				/* sum += x7 - p1 */													
				/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
				
				movq		mm5,	[edi+64]			/* mm5 = x4 */					
				psubw		mm3,	mm1					/* sum = sum-p1 */				
				
				paddw		mm3,	[edi+112]			/* sum = sum+x7 */				
				movq		mm4,	mm5					/* mm4 = x4 */					
				
				paddw		mm4,	mm3					/* mm4 = sum + x4 */			
				paddw		mm4,	mm4					/* mm4 *=2 */					
				
				paddw		mm4,	mm1					/* += p1 */						
				psubw		mm4,	[edi+16]			/* -= x1 */						
				
				psubw		mm4,	[edi+112]			/* -= x7 */						
				paddw		mm4,	[edi+128]			/* += x8 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x4 */						
				
				pand		mm4,	mm0					/* and flag */					
				paddw		mm4,	mm5					/* += x4 */						
				
				movq		[esi+48], mm4				/* write new x4 */				
				
				/* sum+= x8-x1 */														
				/* Des[0]=((sum+x5)<<1+x1-x2-x8+p2)>>4 */								
				
				movq		mm5,	[edi+80]			/* mm5 = x5 */					
				psubw		mm3,	[edi+16]			/* sum -= x1 */					
				
				paddw		mm3,	[edi+128]			/* sub += x8 */					
				movq		mm4,	mm5					/* mm4 = x5 */					
				
				paddw		mm4,	mm3					/* mm4= sum+x5 */				
				paddw		mm4,	mm4					/* mm4 *= 2 */					
				
				paddw		mm4,	[edi+16]			/* += x1 */						
				psubw		mm4,	[edi+32]			/* -= x2 */						
				
				psubw		mm4,	[edi+128]			/* -= x8 */						
				paddw		mm4,	mm2					/* += p2 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x5 */						
				
				pand		mm4,	mm0					/* and flag */					
				paddw		mm4,	mm5					/* += x5 */						
				
				movq		[esi+64], mm4				/* write new x5 */				
				
				/* sum += p2 - x2 */													
				/* Des[w1] = ((sum+x6)<<1 + x2-x3)>>4 */								
				
				movq		mm5,	[edi+96]			/* mm5 = x6 */					
				psubw		mm3,	[edi+32]			/* -= x2 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x6 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x6 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+32]			/* +=x2 */						
				psubw		mm4,	[edi+48]			/* -=x3 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x6 */						
				
				pand		mm4,	mm0					/* and flag */					
				paddw		mm4,	mm5					/* += x6 */						
				
				movq		[esi+80], mm4				/* write new x6 */				
				
				/* sum += p2 - x3 */													
				/* Des[w2] = ((sum+x7)<<1 + x3-x4)>>4 */								
				
				movq		mm5,	[edi+112]			/* mm5 = x7 */					
				psubw		mm3,	[edi+48]			/* -= x3 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x7 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x7 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+48]			/* +=x3 */						
				psubw		mm4,	[edi+64]			/* -=x4 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x7 */						
				
				pand		mm4,	mm0					/* and flag */					
				paddw		mm4,	mm5					/* += x7 */						
				
				movq		[esi+96], mm4				/* write new x7 */				
				
				/* sum += p2 - x4 */													
				/* Des[w3] = ((sum+x8)<<1 + x4-x5)>>4 */								
				
				movq		mm5,	[edi+128]			/* mm5 = x8 */					
				psubw		mm3,	[edi+64]			/* -= x4 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x8 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x8 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+64]			/* +=x4 */						
				psubw		mm4,	[edi+80]			/* -=x5 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x8 */						
				
				pand		mm4,	mm0					/* and flag */					
				paddw		mm4,	mm5					/* += x8 */						
				
				movq		[esi+112], mm4				/* write new x8 */				
				
				/* done with right four column */										
				add			edi,	8					/* shift edi to point x1 */
				sub			esi,	8					/* shift esi back to x1 */

				mov			ebp, Des					/* the destination */							
				lea			ebp, [ebp + edx *4]			/* point to des[-w4] */			
				
				movq		mm0, [esi]													
				packuswb	mm0, [esi + 8]												
				
				movq		[ebp], mm0					/* write des[-w4] */			
				
				movq		mm1, [esi + 16]												
				packuswb	mm1, [esi + 24]												
				
				movq		[ebp+ecx ], mm1				/* write des[-w3] */			
				
				movq		mm2, [esi + 32]												
				packuswb	mm2, [esi + 40]												
				
				movq		[ebp+ecx*2 ], mm2			/* write des[-w2] */			
				
				movq		mm3, [esi + 48]												
				packuswb	mm3, [esi + 56]												
				
				lea			ebp, [ebp+ecx*4]			/* point to des[0] */			
				movq		[ebp+edx], mm3				/* write des[-w1] */			
				
				movq		mm0, [esi + 64]												
				packuswb	mm0, [esi + 72]												
				
				movq		[ebp ], mm0					/* write des[0] */				
				
				movq		mm1, [esi + 80]												
				packuswb	mm1, [esi + 88]												
				
				movq		[ebp+ecx], mm1				/* write des[w1] */				
				
				movq		mm2, [esi + 96]												
				packuswb	mm2, [esi + 104]											
				
				movq		[ebp+ecx*2], mm2			/* write des[w2] */				
				
				movq		mm3, [esi + 112]											
				packuswb	mm3, [esi + 120]											
				
				lea			ebp, [ebp+ecx*2]			/* point to des[w4] */			
				movq		[ebp+ecx], mm3				/* write des[w3] */				
				
				pop			edi
				pop			esi
				pop			edx
				pop			ecx
				pop			ebp
				pop			eax
				
				
		    } /* end of the macro */
		
		    Var1 = Variance11[0]+ Variance11[1]+Variance11[2]+Variance11[3];
		    Var1 += Variance12[0]+ Variance12[1]+Variance12[2]+Variance12[3];
		    pbi->FragmentVariances[CurrentFrag] += Var1;

		    Var2 = Variance21[0]+ Variance21[1]+Variance21[2]+Variance21[3];
		    Var2 += Variance22[0]+ Variance22[1]+Variance22[2]+Variance22[3];
		    pbi->FragmentVariances[CurrentFrag + FragAcross] += Var2;
        }
        else
        {

			/* copy from src to des */
			__asm	
			{
				push		esi
				push		edi
				push		ecx
				
				mov			esi,	Src					/* esi = Src */					
				mov			edi,	Des					/* edi = Des */				

				push		edx

				mov			ecx,	PlaneLineStep		/* ecx = Pitch */				
				xor			edx,	edx					/* clear edx */					
				
				sub			edx,	ecx					/* edx = -Pitch */				
				lea			esi,	[esi+edx*4]			/* esi=Src-4*Pitch*/
				
				movq		mm0,	[esi]				/* first row */
				movq		[edi+edx*4],	mm0			/* write first row */
				
				lea			edi,	[edi+edx*4]			/* edi=Des-4*Pitch*/
				movq		mm1,	[esi+ecx]			/* Src-3*Pitch */

				movq		[edi+ecx],	mm1				/* write second row */
				movq		mm2,	[esi+ecx*2]			/* Src-2*Pitch */

				lea			esi,	[esi+ecx*4]			/* Src */
				movq		[edi+ecx*2], mm2			/* write third row */

				lea			edi,	[edi+ecx*4]			/* Des */
				movq		mm3,	[esi+edx]			/* Src-Pitch */
				
				movq		[edi+edx],	mm3				/* write fourth row */				
				movq		mm4,	[esi]				/* Src */

				movq		mm5,	[esi+ecx]			/* Src+Pitch */
				movq		[edi],	mm4					/* write fifth rwo */

				movq		mm6,	[esi+ecx*2]
				lea			esi,	[esi+ecx*4]			/* Src+pitch*4 */

				movq		[edi+ecx], mm5				/* write the sixth rwo */
				movq		[edi+ecx*2], mm6			/* write the seventh row */

				movq		mm7,	[esi+edx]
				lea			edi,	[edi+ecx*4]			/* Des+Pitch*4 */

				movq		[edi+edx], mm7				/* write the last row */

				pop			edx
				pop			ecx
				pop			edi
				pop			esi				
			}

        }
		
		Src += 8;
		Des += 8;
		CurrentFrag ++;
	}

	Des -= ((PlaneLineStep + FragAcross)<<3);
	Des += 8;
	Src = Des;

	CurrentFrag = StartFrag ;

	while(CurrentFrag < StartFrag + FragAcross - 1)
	{

        QStep = QuantScale[pbi->FragQIndex[CurrentFrag+1]];		

        if( QStep > 3 )
        {
            QStepMmx[0] = (INT16)QStep;
            QStepMmx[1] = (INT16)QStep;
            QStepMmx[2] = (INT16)QStep;
            QStepMmx[3] = (INT16)QStep;

			for( j=0; j<8;j++)
		    {
    			Rows[j] = (short) (Src[-5 +j*PlaneLineStep]);
	    		Rows[72+j] = (short)(Src[4+j*PlaneLineStep]);		
    		}

	    	__asm
    		{
				/* Save the registers */
				push		eax
				push		ebp
				push		ecx			
				push		edx
				push		esi
				push		edi
				
				/* Calculate the FLimit and store FLimit and QStep */					
				
				movq		mm0,	QStepMmx			/* mm0 = QStep */				
				movq		mm1,	FourThrees			/* mm1 = 03030303 */			

                pmullw		mm1,	mm0					/* mm1 = QStep * 3 */							
				pmullw		mm1,	mm0					/* mm1 = QStep * QStep * 3 */	
				
				psrlw		mm1,	5					/* mm1 = FLimit */				
				movq		[FLimitMmx], mm1			/* Save FLimit */				

				/* setup the pointers to data */

				mov			eax,	Src					/* eax = Src */
				xor			edx,	edx					/* clear edx */
				
				sub			eax,	4					/* eax = Src-4 */
				lea			esi,	NewRows				/* esi = NewRows */
				lea			edi,	Rows				/* edi = Rows */				

				mov			ecx,	PlaneLineStep		/* ecx = Pitch */				
				sub			edx,	ecx					/* edx = -Pitch */				

				/* Get the data to the intermediate buffer */

				movq		mm0,	[eax]				/* mm0 = 07 06 05 04 03 02 01 00 */
				movq		mm1,	[eax+ecx]			/* mm1 = 17 16 15 14 13 12 11 10 */

				movq		mm2,	[eax+ecx*2]			/* mm2 = 27 26 25 24 23 22 21 20 */
				lea			eax,	[eax+ecx*4]			/* Go down four Rows */	

				movq		mm3,	[eax+edx]			/* mm3 = 37 36 35 34 33 32 31 30 */
				movq		mm4,	mm0					/* mm4 = 07 06 05 04 03 02 01 00 */
			
				punpcklbw	mm0,	mm1					/* mm0 = 13 03 12 02 11 01 10 00 */
				punpckhbw	mm4,	mm1					/* mm4 = 17 07 16 06 15 05 14 04 */

				movq		mm5,	mm2					/* mm5 = 27 26 25 24 23 22 21 20 */
				punpcklbw	mm2,	mm3					/* mm2 = 33 23 32 22 31 21 30 20 */

				punpckhbw	mm5,	mm3					/* mm5 = 37 27 36 26 35 25 34 24 */
				movq		mm1,	mm0					/* mm1 = 13 03 12 02 11 01 10 00 */

				punpcklwd	mm0,	mm2					/* mm0 = 31 21 11 01 30 20 10 00 */
				punpckhwd	mm1,	mm2					/* mm1 = 33 23 13 03 32 22 12 02 */
				
				movq		mm2,	mm4					/* mm2 = 17 07 16 06 15 05 14 04 */
				punpckhwd	mm4,	mm5					/* mm4 = 37 27 17 07 36 26 16 06 */

				punpcklwd	mm2,	mm5					/* mm2 = 35 25 15 05 34 24 14 04 */
				pxor		mm7,	mm7					/* clear mm7 */

				movq		mm5,	mm0					/* make a copy */
				punpcklbw	mm0,	mm7					/* mm0 = 30 20 10 00 */

				movq		[edi+16], mm0				/* write 00 10 20 30 */
				punpckhbw	mm5,	mm7					/* mm5 = 31 21 11 01 */

				movq		mm0,	mm1					/* mm0 =33 23 13 03 32 22 12 02 */
				movq		[edi+32], mm5				/* write 01 11 21 31 */
				
				punpcklbw	mm1,	mm7					/* mm1 = 32 22 12 02 */
				punpckhbw	mm0,	mm7					/* mm0 = 33 23 12 03 */

				movq		[edi+48], mm1				/* write 02 12 22 32 */
				movq		mm3,	mm2					/* mm3 = 35 25 15 05 34 24 14 04 */
				
				movq		mm5,	mm4					/* mm5 = 37 27 17 07 36 26 16 06 */
				movq		[edi+64], mm0				/* write 03 13 23 33 */

				punpcklbw	mm2,	mm7					/* mm2 = 34 24 14 04 */
				punpckhbw	mm3,	mm7					/* mm3 = 35 25 15 05 */

				movq		[edi+80], mm2				/* write 04 14 24 34 */
				punpcklbw	mm4,	mm7					/* mm4 = 36 26 16 06 */

				punpckhbw	mm5,	mm7					/* mm5 = 37 27 17 07 */
				movq		[edi+96], mm3				/* write 05 15 25 35 */
			
				movq		mm0,	[eax]				/* mm0 = 47 46 45 44 43 42 41 40 */
				movq		mm1,	[eax + ecx ]		/* mm1 = 57 56 55 54 53 52 51 50 */

				movq		[edi+112], mm4				/* write 06 16 26 37 */
				movq		mm2,	[eax+ecx*2]			/* mm2 = 67 66 65 64 63 62 61 60 */

				lea			eax,	[eax+ ecx*4]		/* Go down four rows */
				movq		[edi+128], mm5				/* write 07 17 27 37 */

				movq		mm4,	mm0					/* mm4 = 47 46 45 44 43 42 41 40 */
				movq		mm3,	[eax+edx]			/* mm3 = 77 76 75 74 73 72 71 70 */

				punpcklbw	mm0,	mm1					/* mm0 = 53 43 52 42 51 41 50 40 */
				punpckhbw	mm4,	mm1					/* mm4 = 57 57 56 46 55 45 54 44 */

				movq		mm5,	mm2					/* mm5 = 67 66 65 64 63 62 61 60 */
				punpcklbw	mm2,	mm3					/* mm2 = 73 63 72 62 71 61 70 60 */

				punpckhbw	mm5,	mm3					/* mm5 = 77 67 76 66 75 65 74 64 */
				movq		mm1,	mm0					/* mm1 = 53 43 52 42 51 41 50 40 */

				punpcklwd	mm0,	mm2					/* mm0 = 71 61 51 41 70 60 50 40 */
				punpckhwd	mm1,	mm2					/* mm1 = 73 63 53 43 72 62 52 42 */
				
				movq		mm2,	mm4					/* mm2 = 57 57 56 46 55 45 54 44 */
				punpckhwd	mm4,	mm5					/* mm4 = 77 67 57 47 76 66 56 46 */

				punpcklwd	mm2,	mm5					/* mm2 = 75 65 55 45 74 64 54 44 */

				movq		mm5,	mm0					/* make a copy */
				punpcklbw	mm0,	mm7					/* mm0 = 70 60 50 40 */

				movq		[edi+24], mm0				/* write 40 50 60 70 */
				punpckhbw	mm5,	mm7					/* mm5 = 71 61 51 41 */

				movq		mm0,	mm1					/* mm0 = 73 63 53 43 72 62 52 42 */
				movq		[edi+40], mm5				/* write 41 51 61 71 */
				
				punpcklbw	mm1,	mm7					/* mm1 = 72 62 52 42 */
				punpckhbw	mm0,	mm7					/* mm0 = 73 63 53 43 */

				movq		[edi+56], mm1				/* write 42 52 62 72 */
				movq		mm3,	mm2					/* mm3 = 75 65 55 45 74 64 54 44 */
				
				movq		mm5,	mm4					/* mm5 = 77 67 57 47 76 66 56 46 */
				movq		[edi+72], mm0				/* write 43 53 63 73 */

				punpcklbw	mm2,	mm7					/* mm2 = 74 64 54 44 */
				punpckhbw	mm3,	mm7					/* mm3 = 75 65 55 45 */

				movq		[edi+88], mm2				/* write 44 54 64 74 */
				punpcklbw	mm4,	mm7					/* mm4 = 76 66 56 46 */

				punpckhbw	mm5,	mm7					/* mm5 = 77 67 57 47 */
				movq		[edi+104], mm3				/* write 45 55 65 75 */
			
				movq		[edi+120], mm4				/* write 46 56 66 76 */
				movq		[edi+136], mm5				/* write 47 57 67 77 */


				/* Now, compute the variances for Pixel  1-4 and 5-8 */					
				
				/* we use mm0,mm1,mm2 for 1234 and mm4, mm5, mm6 for 5-8 */				
				/* mm7 = 0, mm3 = {128, 128, 128, 128} */								
				
				pcmpeqw		mm3,	mm3					/* mm3 = FFFFFFFFFFFFFFFF */	
				psllw		mm3,	15					/* mm3 = 8000800080008000 */	
				psrlw		mm3,	8					/* mm3 = 0080008000800080 */
				
				movq		mm2,	[edi+16]			/* Pixel 1 */					
				movq		mm6,	[edi+80]			/* Pixel 5 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				movq		mm0,	mm2					/* mm0 = pixel 1 */				
				movq		mm4,	mm6					/* mm4 = pixel 5 */				
				
				pmullw		mm2,	mm2					/* mm2 = pixel1 * pixel1 */		
				pmullw		mm6,	mm6					/* mm6 = pixel5 * pixel5 */		
				
				movq		mm1,	mm2					/* mm1 = pixel1^2 */			
				movq		mm5,	mm6					/* mm5 = pixel5^2 */			
				
				movq		mm2,	[edi+32]			/* Pixel 2 */					
				movq		mm6,	[edi+96]			/* Pixel 6 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 2 */			
				paddw		mm4,	mm6					/* mm4 += pixel 6 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel2^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel6^2 */			
				
				paddw		mm1,	mm2					/* mm1 += pixel2^2 */			
				paddw		mm5,	mm6					/* mm5 += pixel6^2 */			
				
				movq		mm2,	[edi+48]			/* Pixel 3 */					
				movq		mm6,	[edi+112]			/* Pixel 7 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 3 */			
				paddw		mm4,	mm6					/* mm4 += pixel 7 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel3^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel7^2 */			
				
				paddw		mm1,	mm2					/* mm1 += pixel3^2 */			
				paddw		mm5,	mm6					/* mm5 += pixel7^2 */			
				
				movq		mm2,	[edi+64]			/* Pixel 4 */					
				movq		mm6,	[edi+128]			/* Pixel 8 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 4 */			
				paddw		mm4,	mm6					/* mm4 += pixel 8 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel4^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel8^2 */			
				
				paddw		mm1,	mm2					/* mm1 = pixel4^2 */			
				paddw		mm5,	mm6					/* mm5 = pixel8^2 */			
				
				/* mm0 = x1^2 + x2^2 + x3^2 + x4^2 */									
				/* mm1 = x1 + x2 + x3 + x4 */											
				/* mm4 = x5^2 + x6^2 + x7^2 + x8^2 */									
				/* mm5 = x5 + x6 + x7 + x8 */											
				
				movq		mm7,	mm3					/* mm7 = mm3 */					
				psrlw		mm7,	7					/* mm7 = 0001000100010001 */	
				
				movq		mm2,	mm0					/* make copy of sum1 */			
				movq		mm6,	mm4					/* make copy of sum2 */			
				
				paddw		mm0,	mm7					/* (sum1 + 1) */				
				paddw		mm4,	mm7					/* (sum2 + 1) */				
				
				psraw		mm2,	1					/* sum1 /2 */					
				psraw		mm6,	1					/* sum2 /2 */					
				
				psraw		mm0,	1					/* (sum1 + 1)/2 */				
				psraw		mm4,	1					/* (sum2 + 1)/2 */				
				
				pmullw		mm2,	mm0					/* (sum1)/2*(sum1+1)/2 */		
				pmullw		mm6,	mm4					/* (sum2)/2*(sum2+1)/2 */		
				
				psubw		mm1,	mm2					/* Variance 1 */				
				psubw		mm5,	mm6					/* Variance 2 */				
				
				movq		[Variance11], mm1				/* Save Variance1 */
				movq		[Variance21], mm5				/* Save Variance2 */

				movq		mm7,	FLimitMmx			/* mm7 = FLimit */
				movq		mm2,	mm1					/* copy of Variance 1*/

				movq		mm6,	mm5					/* copy of Variance 2*/
				psubw		mm1,	mm7					/* Variance 1 < Flimit? */		
				
				psubw		mm5,	mm7					/* Variance 2 < Flimit? */		
				psraw		mm1,	15					/* FFFF/0000 for true/false */	
				
				psraw		mm5,	15					/* FFFF/0000 for true/false */	
				psraw		mm2,	15					/* Variance 1 > 32768 ? */

				psraw		mm6,	15					/* Variance 2 > 32768 ? */
				movq		mm7,	[edi+64]			/* mm0 = Pixel 4			*/	

				pandn		mm2,	mm1					/* Variance 1 < Flimit &&
														   Variance 1 < 32768		*/
				pandn		mm6,	mm5					/* Variance 2 < Flimit &&
														   Variance 2 < 32768		*/
				movq		mm4,	[edi+80]			/* mm4 = Pixel 5			*/				
				pand		mm6,	mm2					/* mm1 = Variance1 < Flimit */	
														/*     &&Variance2 < Flimit */	
				movq		mm2,	mm7					/* make copy of Pixel4		*/	

				psubusw		mm7,	mm4					/* 4 - 5 */						
				psubusw		mm4,	mm2					/* 5 - 4 */						
				
				por			mm7,	mm4					/* abs(4 - 5) */				
				psubw		mm7,	QStepMmx			/* abs(4-5)<QStepMmx ? */		
				
				psraw		mm7,	15					/* FFFF/0000 for True/Flase */
				pand		mm7,	mm6													
				
				/* mm7 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* now lets look at the right four colomn */							
				
				add			edi,	8					/* offset 8 to right 4 cols */	
				
				movq		mm2,	[edi+16]			/* Pixel 1 */					
				movq		mm6,	[edi+80]			/* Pixel 5 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				movq		mm0,	mm2					/* mm0 = pixel 1 */				
				movq		mm4,	mm6					/* mm4 = pixel 5 */				
				
				pmullw		mm2,	mm2					/* mm2 = pixel1 * pixel1 */		
				pmullw		mm6,	mm6					/* mm6 = pixel5 * pixel5 */		
				
				movq		mm1,	mm2					/* mm1 = pixel1^2 */			
				movq		mm5,	mm6					/* mm5 = pixel5^2 */			
				
				movq		mm2,	[edi+32]			/* Pixel 2 */					
				movq		mm6,	[edi+96]			/* Pixel 6 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 2 */			
				paddw		mm4,	mm6					/* mm4 += pixel 6 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel2^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel6^2 */			
				
				paddw		mm1,	mm2					/* mm1 += pixel2^2 */			
				paddw		mm5,	mm6					/* mm5 += pixel6^2 */			
				
				movq		mm2,	[edi+48]			/* Pixel 3 */					
				movq		mm6,	[edi+112]			/* Pixel 7 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 3 */			
				paddw		mm4,	mm6					/* mm4 += pixel 7 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel3^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel7^2 */			
				
				paddw		mm1,	mm2					/* mm1 += pixel3^2 */			
				paddw		mm5,	mm6					/* mm5 += pixel7^2 */			
				
				movq		mm2,	[edi+64]			/* Pixel 4 */					
				movq		mm6,	[edi+128]			/* Pixel 8 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 4 */			
				paddw		mm4,	mm6					/* mm4 += pixel 8 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel4^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel8^2 */			
				
				paddw		mm1,	mm2					/* mm1 = pixel4^2 */			
				paddw		mm5,	mm6					/* mm5 = pixel8^2 */			
				
				/* mm0 = x1^2 + x2^2 + x3^2 + x4^2 */									
				/* mm1 = x1 + x2 + x3 + x4 */											
				/* mm4 = x5^2 + x6^2 + x7^2 + x8^2 */									
				/* mm5 = x5 + x6 + x7 + x8 */											
				
				psrlw		mm3,	7					/* mm3 = 0001000100010001 */	
				
				movq		mm2,	mm0					/* make copy of sum1 */			
				movq		mm6,	mm4					/* make copy of sum2 */			
				
				paddw		mm0,	mm3					/* (sum1 + 1) */				
				paddw		mm4,	mm3					/* (sum2 + 1) */				
				
				psraw		mm2,	1					/* sum1 /2 */					
				psraw		mm6,	1					/* sum2 /2 */					
				
				psraw		mm0,	1					/* (sum1 + 1)/2 */				
				psraw		mm4,	1					/* (sum2 + 1)/2 */				
				
				pmullw		mm2,	mm0					/* (sum1)/2*(sum1+1)/2 */		
				pmullw		mm6,	mm4					/* (sum2)/2*(sum2+1)/2 */		
				
				psubw		mm1,	mm2					/* Variance 1 */				
				psubw		mm5,	mm6					/* Variance 2 */				
				
				movq		[Variance12], mm1				/* Save Variance1 */
				movq		[Variance22], mm5				/* Save Variance2 */
	
				movq		mm3,	FLimitMmx			/* mm3 = FLimit */				
				movq		mm2,	mm1					/* copy of Varinace 1*/

				movq		mm6,	mm5					/* Variance 2 */
				psubw		mm1,	mm3					/* Variance 1 < Flimit? */		
				
				psubw		mm5,	mm3					/* Variance 2 < Flimit? */		
				psraw		mm6,	15					/* Variance 1 > 32768 */
				
				psraw		mm2,	15					/* Variance 2 > 32768 */
				psraw		mm1,	15					/* FFFF/0000 for true/false */	
				
				psraw		mm5,	15					/* FFFF/0000 for true/false */	
				movq		mm0,	[edi+64]			/* mm0 = Pixel 4			*/	

				pandn		mm2,	mm1					/* Variance1<32678 && 
														   Variance1<Limit			*/
				pandn		mm6,	mm5					/* Variance2<32678 && 
														   Variance1<Limit			*/
				
				movq		mm4,	[edi+80]			/* mm4 = Pixel 5			*/	
				pand		mm6,	mm2					/* mm1 = Variance1 < Flimit */	
														/*     &&Variance2 < Flimit */	
				movq		mm2,	mm0					/* make copy of Pixel4		*/	
														
				psubusw		mm0,	mm4					/* 4 - 5 */						
				psubusw		mm4,	mm2					/* 5 - 4 */						
				
				por			mm0,	mm4					/* abs(4 - 5) */				
				psubw		mm0,	QStepMmx			/* abs(4-5)<QStepMmx ? */		
				
				psraw		mm0,	15					/* FFFF/0000 for True/False */
				pand		mm0,	mm6				
				
				sub			edi,	8					/* offset edi back */			
				
				/* mm0 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* mm0 and mm7 now are in use  */										
				/* Let's do the filtering now */										
				/* p1 = (abs(Src[-4] - Src[-5]) < QStep ) ?  Src[-5] : Src[-4]; */		
				/* p2 = (abs(Src[+3] - Src[+4]) < QStep ) ?  Src[+4] : Src[+3]; */		
				
				movq		mm5,	[edi]				/* mm5 = -5 */					
				movq		mm4,	[edi + 16]			/* mm4 = -4 */					
				
				movq		mm3,	mm4					/* copy of -4 */				
				movq		mm6,	mm5					/* copy of -5 */				
				
				psubusw		mm4,	mm6					/* mm4 = [-4] - [-5] */			
				psubusw		mm5,	mm3					/* mm5 = [-5] - [-4] */			
				
				por			mm4,	mm5					/* abs([-4]-[-5] ) */			
				psubw		mm4,	QStepMmx			/* abs([-4]-[-5] )<QStep? */	
				
				psraw		mm4,	15					/* FFFF/0000 for True/False */	
				movq		mm1,	mm4					/* copy of the mm4 */			
				
				pand		mm4,	mm6					/*							*/	
				pandn		mm1,	mm3					/*							*/	
				
				por			mm1,	mm4					/* mm1 = p1					*/	
				
				/* now find P2 */														
				
				movq		mm4,	[edi+128]			/* mm4 = [3] */					
				movq		mm5,	[edi+144]			/* mm5 = [4] */					
				
				movq		mm3,	mm4					/* copy of 3 */					
				movq		mm6,	mm5					/* copy of 4 */					
				
				psubusw		mm4,	mm6					/* mm4 = [3] - [4] */			
				psubusw		mm5,	mm3					/* mm5 = [4] - [3] */			
				
				por			mm4,	mm5					/* abs([3]-[4] ) */				
				psubw		mm4,	QStepMmx			/* abs([3]-[4] )<QStep? */		
				
				psraw		mm4,	15					/* FFFF/0000 for True/False */	
				movq		mm2,	mm4					/* copy of the mm4 */			
				
				pand		mm4,	mm6					/*							*/	
				pandn		mm2,	mm3					/*							*/	
				
				por			mm2,	mm4					/* mm2 = p2					*/	
				
				/* sum = p1 + p1 + p1 + x1 + x2 + x3 + x4 + 4; */				
				/* Des[-w4] = (((sum + x1) << 1) - (x4 - x5)) >> 4; */			
				/* Des[-w4] = Src[-w4]; */												
				/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	
				
				movq		mm3,	mm1					/* mm3 = p1 */					
				paddw		mm3,	mm3					/* mm3 = p1 + p1 */				
				
				paddw		mm3,	mm1					/* mm3 = p1 + p1 + p1 */		
				movq		mm4,	[edi+16]			/* mm4 = x1 */					
				
				paddw		mm3,	[edi+32]			/* mm3 = p1+p1+p1+ x2 */		
				paddw		mm4,	[edi+48]			/* mm4 = x1+x3 */				
				
				paddw		mm3,	[edi+64]			/* mm3 += x4 */					
				paddw		mm4,	FourFours			/* mm4 = x1 + x3 + 4 */			
				
				paddw		mm3,	mm4					/* mm3 = 3*p1+x1+x2+x3+x4+4 */	
				movq		mm4,	mm3					/* mm4 = mm3 */					
				
				movq		mm5,	[edi+16]			/* mm5 = x1 */					
				paddw		mm4,	mm5					/* mm4 = sum+x1 */				
				
				psllw		mm4,	1					/* mm4 = (sum+x1)<<1 */			
				psubw		mm4,	[edi+64]			/* mm4 = (sum+x1)<<1-x4 */		
				
				paddw		mm4,	[edi+80]			/* mm4 = (sum+x1)<<1-x4+x5 */	
				psraw		mm4,	4					/* mm4 >>=4 */					
				
				psubw		mm4,	mm5					/* New Value - old Value */		
				pand		mm4,	mm7					/* And the flag */				
				
				paddw		mm4,	mm5					/* add the old value back */	
				movq		[esi],	mm4					/* Write new x1 */				
				
				/* sum += x5 -p1 */														
				/* Des[-w3]=((sum+x2)<<1-x5+x6)>>4 */									
				
				movq		mm5,	[edi+32]			/* mm5= x2 */					
				psubw		mm3,	mm1					/* sum=sum-p1 */				
				
				paddw		mm3,    [edi+80]			/* sum=sum+x5 */				
				movq		mm4,	mm5					/* copy sum */					
				
				paddw		mm4,	mm3					/* mm4=sum+x2 */				
				paddw		mm4,	mm4					/* mm4 <<= 1 */					
				
				psubw		mm4,	[edi+80]			/* mm4 =(sum+x2)<<1-x5 */		
				paddw		mm4,	[edi+96]			/* mm4 =(sum+x2)<<1-x5+x6 */	
				
				psraw		mm4,	4					/* mm4=((sum+x2)<<1-x5+x6)>>4 */
				psubw		mm4,	mm5					/* new value - old value	*/	
				
				pand		mm4,	mm7					/* And the flag */				
				paddw		mm4,	mm5					/* add the old value back */	
				
				movq		[esi+16], mm4				/* write new x2 */				
				
				/* sum += x6 - p1 */													
				/* Des[-w2]=((sum+x[3])<<1-x[6]+x[7])>>4 */								
				
				movq		mm5,	[edi+48]			/* mm5= x3 */					
				psubw		mm3,	mm1					/* sum=sum-p1 */				
				
				paddw		mm3,    [edi+96]			/* sum=sum+x6 */				
				movq		mm4,	mm5					/* copy x3 */					
				
				paddw		mm4,	mm3					/* mm4=sum+x3 */				
				paddw		mm4,	mm4					/* mm4 <<= 1 */					
				
				psubw		mm4,	[edi+96]			/* mm4 =(sum+x3)<<1-x6 */		
				paddw		mm4,	[edi+112]			/* mm4 =(sum+x3)<<1-x6+x7 */	
				
				psraw		mm4,	4					/* mm4=((sum+x3)<<1-x6+x7)>>4 */
				psubw		mm4,	mm5					/* new value - old value	*/	
				
				pand		mm4,	mm7					/* And the flag */				
				paddw		mm4,	mm5					/* add the old value back */	
				
				movq		[esi+32], mm4				/* write new x3 */				
				
				/* sum += x7 - p1 */													
				/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
				
				movq		mm5,	[edi+64]			/* mm5 = x4 */					
				psubw		mm3,	mm1					/* sum = sum-p1 */				
				
				paddw		mm3,	[edi+112]			/* sum = sum+x7 */				
				movq		mm4,	mm5					/* mm4 = x4 */					
				
				paddw		mm4,	mm3					/* mm4 = sum + x4 */			
				paddw		mm4,	mm4					/* mm4 *=2 */					
				
				paddw		mm4,	mm1					/* += p1 */						
				psubw		mm4,	[edi+16]			/* -= x1 */						
				
				psubw		mm4,	[edi+112]			/* -= x7 */						
				paddw		mm4,	[edi+128]			/* += x8 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x4 */						
				
				pand		mm4,	mm7					/* and flag */					
				paddw		mm4,	mm5					/* += x4 */						
				
				movq		[esi+48], mm4				/* write new x4 */				
				
				/* sum+= x8-x1 */														
				/* Des[0]=((sum+x5)<<1+x1-x2-x8+p2)>>4 */								
				
				movq		mm5,	[edi+80]			/* mm5 = x5 */					
				psubw		mm3,	[edi+16]			/* sum -= x1 */					
				
				paddw		mm3,	[edi+128]			/* sub += x8 */					
				movq		mm4,	mm5					/* mm4 = x5 */					
				
				paddw		mm4,	mm3					/* mm4= sum+x5 */				
				paddw		mm4,	mm4					/* mm4 *= 2 */					
				
				paddw		mm4,	[edi+16]			/* += x1 */						
				psubw		mm4,	[edi+32]			/* -= x2 */						
				
				psubw		mm4,	[edi+128]			/* -= x8 */						
				paddw		mm4,	mm2					/* += p2 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x5 */						
				
				pand		mm4,	mm7					/* and flag */					
				paddw		mm4,	mm5					/* += x5 */						
				
				movq		[esi+64], mm4				/* write new x5 */				
				
				/* sum += p2 - x2 */													
				/* Des[w1] = ((sum+x6)<<1 + x2-x3)>>4 */								
				
				movq		mm5,	[edi+96]			/* mm5 = x6 */					
				psubw		mm3,	[edi+32]			/* -= x2 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x6 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x6 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+32]			/* +=x2 */						
				psubw		mm4,	[edi+48]			/* -=x3 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x6 */						
				
				pand		mm4,	mm7					/* and flag */					
				paddw		mm4,	mm5					/* += x6 */						
				
				movq		[esi+80], mm4				/* write new x6 */				
				
				/* sum += p2 - x3 */													
				/* Des[w2] = ((sum+x7)<<1 + x3-x4)>>4 */								
				
				movq		mm5,	[edi+112]			/* mm5 = x7 */					
				psubw		mm3,	[edi+48]			/* -= x3 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x7 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x7 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+48]			/* +=x3 */						
				psubw		mm4,	[edi+64]			/* -=x4 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x7 */						
				
				pand		mm4,	mm7					/* and flag */					
				paddw		mm4,	mm5					/* += x7 */						
				
				movq		[esi+96], mm4				/* write new x7 */				
				
				/* sum += p2 - x4 */													
				/* Des[w3] = ((sum+x8)<<1 + x4-x5)>>4 */								
				
				movq		mm5,	[edi+128]			/* mm5 = x8 */					
				psubw		mm3,	[edi+64]			/* -= x4 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x8 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x8 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+64]			/* +=x4 */						
				psubw		mm4,	[edi+80]			/* -=x5 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x8 */						
				
				pand		mm4,	mm7					/* and flag */					
				paddw		mm4,	mm5					/* += x8 */						
				
				movq		[esi+112], mm4				/* write new x8 */				
				
				/* done with left four columns */										
				/* now do the righ four columns */										
				
				add			edi,	8					/* shift to right four column */
				add			esi,	8					/* shift to right four column */
				
				/* mm0 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* mm0 now are in use  */										
				/* Let's do the filtering now */										
				/* p1 = (abs(Src[-4] - Src[-5]) < QStep ) ?  Src[-5] : Src[-4]; */		
				/* p2 = (abs(Src[+3] - Src[+4]) < QStep ) ?  Src[+4] : Src[+3]; */		
				
				movq		mm5,	[edi]				/* mm5 = -5 */					
				movq		mm4,	[edi + 16]			/* mm4 = -4 */					
				
				movq		mm3,	mm4					/* copy of -4 */				
				movq		mm6,	mm5					/* copy of -5 */				
				
				psubusw		mm4,	mm6					/* mm4 = [-4] - [-5] */			
				psubusw		mm5,	mm3					/* mm5 = [-5] - [-4] */			
				
				por			mm4,	mm5					/* abs([-4]-[-5] ) */			
				psubw		mm4,	QStepMmx			/* abs([-4]-[-5] )<QStep? */	
				
				psraw		mm4,	15					/* FFFF/0000 for True/False */	
				movq		mm1,	mm4					/* copy of the mm4 */			
				
				pand		mm4,	mm6					/*							*/	
				pandn		mm1,	mm3					/*							*/	
				
				por			mm1,	mm4					/* mm1 = p1					*/	
				
				/* now find P2 */														
				
				movq		mm4, [edi+128]				/* mm4 = [3] */					
				movq		mm5, [edi+144]				/* mm5 = [4] */					
				
				movq		mm3, mm4					/* copy of 3 */					
				movq		mm6, mm5					/* copy of 4 */					
				
				psubusw		mm4, mm6					/* mm4 = [3] - [4] */			
				psubusw		mm5, mm3					/* mm5 = [4] - [3] */			
				
				por			mm4, mm5					/* abs([3]-[4] ) */				
				psubw		mm4, QStepMmx				/* abs([3]-[4] )<QStep? */		
				
				psraw		mm4, 15						/* FFFF/0000 for True/False */	
				movq		mm2, mm4					/* copy of the mm4 */			
				
				pand		mm4, mm6					/*							*/	
				pandn		mm2, mm3					/*							*/	
				
				por			mm2, mm4					/* mm2 = p2					*/	
				
				/* psum = p1 + p1 + p1 + v[1] + v[2] + v[3] + v[4] + 4; */				
				/* Des[-w4] = (((psum + v[1]) << 1) - (v[4] - v[5])) >> 4; */			
				/* Des[-w4]=Src[-w4]; */												
				/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	
				
				movq		mm3,	mm1					/* mm3 = p1 */					
				paddw		mm3,	mm3					/* mm3 = p1 + p1 */				
				
				paddw		mm3,	mm1					/* mm3 = p1 + p1 + p1 */		
				movq		mm4,	[edi+16]			/* mm4 = x1 */					
				
				paddw		mm3,	[edi+32]			/* mm3 = p1+p1+p1+ x2 */		
				paddw		mm4,	[edi+48]			/* mm4 = x1+x3 */				
				
				paddw		mm3,	[edi+64]			/* mm3 += x4 */					
				paddw		mm4,	FourFours			/* mm4 = x1 + x3 + 4 */			
				
				paddw		mm3,	mm4					/* mm3 = 3*p1+x1+x2+x3+x4+4 */	
				movq		mm4,	mm3					/* mm4 = mm3 */					
				
				movq		mm5,	[edi+16]			/* mm5 = x1 */					
				paddw		mm4,	mm5					/* mm4 = sum+x1 */				
				
				psllw		mm4,	1					/* mm4 = (sum+x1)<<1 */			
				psubw		mm4,	[edi+64]			/* mm4 = (sum+x1)<<1-x4 */		
				
				paddw		mm4,	[edi+80]			/* mm4 = (sum+x1)<<1-x4+x5 */	
				psraw		mm4,	4					/* mm4 >>=4 */					
				
				psubw		mm4,	mm5					/* New Value - old Value */		
				pand		mm4,	mm0					/* And the flag */				
				
				paddw		mm4,	mm5					/* add the old value back */	
				movq		[esi],	mm4					/* Write new x1 */				
				
				/* sum += x5 -p1 */														
				/* Des[-w3]=((sum+x2)<<1-x5+x6)>>4 */									
				
				movq		mm5,	[edi+32]			/* mm5= x2 */					
				psubw		mm3,	mm1					/* sum=sum-p1 */				
				
				paddw		mm3,    [edi+80]			/* sum=sum+x5 */				
				movq		mm4,	mm5					/* copy sum */					
				
				paddw		mm4,	mm3					/* mm4=sum+x2 */				
				paddw		mm4,	mm4					/* mm4 <<= 1 */					
				
				psubw		mm4,	[edi+80]			/* mm4 =(sum+x2)<<1-x5 */		
				paddw		mm4,	[edi+96]			/* mm4 =(sum+x2)<<1-x5+x6 */	
				
				psraw		mm4,	4					/* mm4=((sum+x2)<<1-x5+x6)>>4 */
				psubw		mm4,	mm5					/* new value - old value	*/	
				
				pand		mm4,	mm0					/* And the flag */				
				paddw		mm4,	mm5					/* add the old value back */	
				
				movq		[esi+16], mm4				/* write new x2 */				
				
				/* sum += x6 - p1 */													
				/* Des[-w2]=((sum+x[3])<<1-x[6]+x[7])>>4 */								
				
				movq		mm5,	[edi+48]			/* mm5= x3 */					
				psubw		mm3,	mm1					/* sum=sum-p1 */				
				
				paddw		mm3,    [edi+96]			/* sum=sum+x6 */				
				movq		mm4,	mm5					/* copy x3 */					
				
				paddw		mm4,	mm3					/* mm4=sum+x3 */				
				paddw		mm4,	mm4					/* mm4 <<= 1 */					
				
				psubw		mm4,	[edi+96]			/* mm4 =(sum+x3)<<1-x6 */		
				paddw		mm4,	[edi+112]			/* mm4 =(sum+x3)<<1-x6+x7 */	
				
				psraw		mm4,	4					/* mm4=((sum+x3)<<1-x6+x7)>>4 */
				psubw		mm4,	mm5					/* new value - old value	*/	
				
				pand		mm4,	mm0					/* And the flag */				
				paddw		mm4,	mm5					/* add the old value back */	
				
				movq		[esi+32], mm4				/* write new x3 */				
				
				/* sum += x7 - p1 */													
				/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
				
				movq		mm5,	[edi+64]			/* mm5 = x4 */					
				psubw		mm3,	mm1					/* sum = sum-p1 */				
				
				paddw		mm3,	[edi+112]			/* sum = sum+x7 */				
				movq		mm4,	mm5					/* mm4 = x4 */					
				
				paddw		mm4,	mm3					/* mm4 = sum + x4 */			
				paddw		mm4,	mm4					/* mm4 *=2 */					
				
				paddw		mm4,	mm1					/* += p1 */						
				psubw		mm4,	[edi+16]			/* -= x1 */						
				
				psubw		mm4,	[edi+112]			/* -= x7 */						
				paddw		mm4,	[edi+128]			/* += x8 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x4 */						
				
				pand		mm4,	mm0					/* and flag */					
				paddw		mm4,	mm5					/* += x4 */						
				
				movq		[esi+48], mm4				/* write new x4 */				
				
				/* sum+= x8-x1 */														
				/* Des[0]=((sum+x5)<<1+x1-x2-x8+p2)>>4 */								
				
				movq		mm5,	[edi+80]			/* mm5 = x5 */					
				psubw		mm3,	[edi+16]			/* sum -= x1 */					
				
				paddw		mm3,	[edi+128]			/* sub += x8 */					
				movq		mm4,	mm5					/* mm4 = x5 */					
				
				paddw		mm4,	mm3					/* mm4= sum+x5 */				
				paddw		mm4,	mm4					/* mm4 *= 2 */					
				
				paddw		mm4,	[edi+16]			/* += x1 */						
				psubw		mm4,	[edi+32]			/* -= x2 */						
				
				psubw		mm4,	[edi+128]			/* -= x8 */						
				paddw		mm4,	mm2					/* += p2 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x5 */						
				
				pand		mm4,	mm0					/* and flag */					
				paddw		mm4,	mm5					/* += x5 */						
				
				movq		[esi+64], mm4				/* write new x5 */				
				
				/* sum += p2 - x2 */													
				/* Des[w1] = ((sum+x6)<<1 + x2-x3)>>4 */								
				
				movq		mm5,	[edi+96]			/* mm5 = x6 */					
				psubw		mm3,	[edi+32]			/* -= x2 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x6 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x6 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+32]			/* +=x2 */						
				psubw		mm4,	[edi+48]			/* -=x3 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x6 */						
				
				pand		mm4,	mm0					/* and flag */					
				paddw		mm4,	mm5					/* += x6 */						
				
				movq		[esi+80], mm4				/* write new x6 */				
				
				/* sum += p2 - x3 */													
				/* Des[w2] = ((sum+x7)<<1 + x3-x4)>>4 */								
				
				movq		mm5,	[edi+112]			/* mm5 = x7 */					
				psubw		mm3,	[edi+48]			/* -= x3 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x7 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x7 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+48]			/* +=x3 */						
				psubw		mm4,	[edi+64]			/* -=x4 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x7 */						
				
				pand		mm4,	mm0					/* and flag */					
				paddw		mm4,	mm5					/* += x7 */						
				
				movq		[esi+96], mm4				/* write new x7 */				
				
				/* sum += p2 - x4 */													
				/* Des[w3] = ((sum+x8)<<1 + x4-x5)>>4 */								
				
				movq		mm5,	[edi+128]			/* mm5 = x8 */					
				psubw		mm3,	[edi+64]			/* -= x4 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x8 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x8 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+64]			/* +=x4 */						
				psubw		mm4,	[edi+80]			/* -=x5 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x8 */						
				
				pand		mm4,	mm0					/* and flag */					
				paddw		mm4,	mm5					/* += x8 */						
				
				movq		[esi+112], mm4				/* write new x8 */				
				
				/* done with right four column */	
				/* transpose */
				mov			eax,	Des					/* the destination */			
				add			edi,	8					/* shift edi to point x1 */

				sub			esi,	8					/* shift esi back to left x1 */
				sub			eax,	4

				movq		mm0,	[esi]				/* mm0 = 30 20 10 00 */
				movq		mm1,	[esi+16]			/* mm1 = 31 21 11 01 */

				movq		mm4,	mm0					/* mm4 = 30 20 10 00 */
				punpcklwd	mm0,	mm1					/* mm0 = 11 10 01 00 */

				punpckhwd	mm4,	mm1					/* mm4 = 31 30 21 20 */
				movq		mm2,	[esi+32]			/* mm2 = 32 22 12 02 */

				movq		mm3,	[esi+48]			/* mm3 = 33 23 13 03 */
				movq		mm5,	mm2					/* mm5 = 32 22 12 02 */

				punpcklwd	mm2,	mm3					/* mm2 = 13 12 03 02 */
				punpckhwd	mm5,	mm3					/* mm5 = 33 32 23 22 */

				movq		mm1,	mm0					/* mm1 = 11 10 01 00 */
				punpckldq	mm0,	mm2					/* mm0 = 03 02 01 00 */

				movq		[edi],	mm0					/* write 00 01 02 03 */
				punpckhdq	mm1,	mm2					/* mm1 = 13 12 11 10 */
				
				movq		mm0,	mm4					/* mm0 = 31 30 21 20 */
				movq		[edi+16], mm1				/* write 10 11 12 13 */

				punpckldq	mm0,	mm5					/* mm0 = 23 22 21 20 */
				punpckhdq	mm4,	mm5					/* mm4 = 33 32 31 30 */

				movq		mm1,	[esi+64]			/* mm1 = 34 24 14 04 */
				movq		mm2,	[esi+80]			/* mm2 = 35 25 15 05 */				

				movq		mm5,	[esi+96]			/* mm5 = 36 26 16 06 */
				movq		mm6,	[esi+112]			/* mm6 = 37 27 17 07 */
								
				movq		mm3,	mm1					/* mm3 = 34 24 14 04 */
				movq		mm7,	mm5					/* mm7 = 36 26 16 06 */

				punpcklwd	mm1,	mm2					/* mm1 = 15 14 05 04 */
				punpckhwd	mm3,	mm2					/* mm3 = 35 34 25 24 */

				punpcklwd	mm5,	mm6					/* mm5 = 17 16 07 06 */
				punpckhwd	mm7,	mm6					/* mm7 = 37 36 27 26 */

				movq		mm2,	mm1					/* mm2 = 15 14 05 04 */
				movq		mm6,	mm3					/* mm6 = 35 34 25 24 */

				punpckldq	mm1,	mm5					/* mm1 = 07 06 05 04 */
				punpckhdq	mm2,	mm5					/* mm2 = 17 16 15 14 */

				punpckldq	mm3,	mm7					/* mm3 = 27 26 25 24 */
				punpckhdq	mm6,	mm7					/* mm6 = 37 36 35 34 */
			
				movq		mm5,	[edi]				/* mm5 = 03 02 01 00 */
				packuswb	mm5,	mm1					/* mm5 = 07 06 05 04 03 02 01 00 */
				
				movq		[eax],	mm5					/* write 00 01 02 03 04 05 06 07 */
				movq		mm7,	[edi+16]			/* mm7 = 13 12 11 10 */

				packuswb	mm7,	mm2					/* mm7 = 17 16 15 14 13 12 11 10 */
				movq		[eax+ecx], mm7				/* write 10 11 12 13 14 15 16 17 */

				packuswb	mm0,	mm3					/* mm0 = 27 26 25 24 23 22 21 20 */
				packuswb	mm4,	mm6					/* mm4 = 37 36 35 34 33 32 31 30 */
				
				movq		[eax+ecx*2], mm0			/* write 20 21 22 23 24 25 26 27 */
				lea			eax,	[eax+ecx*4]			/* mov forward the desPtr */

				movq		[eax+edx],	mm4				/* write 30 31 32 33 34 35 36 37 */
				add			edi, 8						/* move to right four column */
				add			esi, 8						/* move to right x1 */

				movq		mm0,	[esi]				/* mm0 = 70 60 50 40 */
				movq		mm1,	[esi+16]			/* mm1 = 71 61 51 41 */

				movq		mm4,	mm0					/* mm4 = 70 60 50 40 */
				punpcklwd	mm0,	mm1					/* mm0 = 51 50 41 40 */

				punpckhwd	mm4,	mm1					/* mm4 = 71 70 61 60 */
				movq		mm2,	[esi+32]			/* mm2 = 72 62 52 42 */

				movq		mm3,	[esi+48]			/* mm3 = 73 63 53 43 */
				movq		mm5,	mm2					/* mm5 = 72 62 52 42 */

				punpcklwd	mm2,	mm3					/* mm2 = 53 52 43 42 */
				punpckhwd	mm5,	mm3					/* mm5 = 73 72 63 62 */

				movq		mm1,	mm0					/* mm1 = 51 50 41 40 */
				punpckldq	mm0,	mm2					/* mm0 = 43 42 41 40 */

				movq		[edi],	mm0					/* write 40 41 42 43 */
				punpckhdq	mm1,	mm2					/* mm1 = 53 52 51 50 */
				
				movq		mm0,	mm4					/* mm0 = 71 70 61 60 */
				movq		[edi+16], mm1				/* write 50 51 52 53 */

				punpckldq	mm0,	mm5					/* mm0 = 63 62 61 60 */
				punpckhdq	mm4,	mm5					/* mm4 = 73 72 71 70 */

				movq		mm1,	[esi+64]			/* mm1 = 74 64 54 44 */
				movq		mm2,	[esi+80]			/* mm2 = 75 65 55 45 */				

				movq		mm5,	[esi+96]			/* mm5 = 76 66 56 46 */
				movq		mm6,	[esi+112]			/* mm6 = 77 67 57 47 */
								
				movq		mm3,	mm1					/* mm3 = 74 64 54 44 */
				movq		mm7,	mm5					/* mm7 = 76 66 56 46 */

				punpcklwd	mm1,	mm2					/* mm1 = 55 54 45 44 */
				punpckhwd	mm3,	mm2					/* mm3 = 75 74 65 64 */

				punpcklwd	mm5,	mm6					/* mm5 = 57 56 47 46 */
				punpckhwd	mm7,	mm6					/* mm7 = 77 76 67 66 */

				movq		mm2,	mm1					/* mm2 = 55 54 45 44 */
				movq		mm6,	mm3					/* mm6 = 75 74 65 64 */

				punpckldq	mm1,	mm5					/* mm1 = 47 46 45 44 */
				punpckhdq	mm2,	mm5					/* mm2 = 57 56 55 54 */

				punpckldq	mm3,	mm7					/* mm3 = 67 66 65 64 */
				punpckhdq	mm6,	mm7					/* mm6 = 77 76 75 74 */
			
				movq		mm5,	[edi]				/* mm5 = 43 42 41 40 */
				packuswb	mm5,	mm1					/* mm5 = 47 46 45 44 43 42 41 40 */
				
				movq		[eax],	mm5					/* write 40 41 42 43 44 45 46 47 */
				movq		mm7,	[edi+16]			/* mm7 = 53 52 51 50 */

				packuswb	mm7,	mm2					/* mm7 = 57 56 55 54 53 52 51 50 */
				movq		[eax+ecx], mm7				/* write 50 51 52 53 54 55 56 57 */

				packuswb	mm0,	mm3					/* mm0 = 67 66 65 64 63 62 61 60 */
				packuswb	mm4,	mm6					/* mm4 = 77 76 75 74 73 72 71 70 */
				
				movq		[eax+ecx*2], mm0			/* write 60 61 62 63 64 65 66 67 */
				lea			eax,	[eax+ecx*4]			/* mov forward the desPtr */

				movq		[eax+edx],	mm4				/* write 70 71 72 73 74 75 76 77 */
				
				pop			edi
				pop			esi
				pop			edx
				pop			ecx
				pop			ebp
				pop			eax
	    	}	

    		Var1 = Variance11[0]+ Variance11[1]+Variance11[2]+Variance11[3];
		    Var1 += Variance12[0]+ Variance12[1]+Variance12[2]+Variance12[3];
		    pbi->FragmentVariances[CurrentFrag] += Var1;

    		Var2 = Variance21[0]+ Variance21[1]+Variance21[2]+Variance21[3];
    		Var2 += Variance22[0]+ Variance22[1]+Variance22[2]+Variance22[3];
		    pbi->FragmentVariances[CurrentFrag + 1] += Var2;
        }
		CurrentFrag ++;
		Src += 8;
		Des += 8;		
	}

}


/****************************************************************************
 * 
 *  ROUTINE       :     DeblockNonFilteredBand_MMX
 *
 *  INPUTS        :     None
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Filter both horizontal and vertical edge in a band
 *
 *  SPECIAL NOTES :     
 *
 *	REFERENCE	  :		
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

void DeblockNonFilteredBand_MMX(
                                 POSTPROC_INSTANCE *pbi, 
                                 UINT8 *SrcPtr, 
                                 UINT8 *DesPtr,
                                 UINT32 PlaneLineStep, 
                                 UINT32 FragAcross,
                                 UINT32 StartFrag,
                                 UINT32 *QuantScale
							    )
{
	UINT32 j;
	UINT32 CurrentFrag=StartFrag;
	UINT32 QStep;
    UINT32 LoopFLimit;
	UINT8 *Src, *Des;
	UINT32 Var1, Var2;

#if defined(_WIN32_WCE)
#pragma pack(16)
short QStepMmx[4];
short FLimitMmx[4];
short LoopFLimitMmx[4];
short Rows[80];
short NewRows[64];
short LoopFilteredValuesUp[4];
short LoopFilteredValuesDown[4];

unsigned short Variance11[4];
unsigned short Variance12[4];
unsigned short Variance21[4];
unsigned short Variance22[4];
#pragma pack()
#else
__declspec(align(16)) short QStepMmx[4];
__declspec(align(16)) short FLimitMmx[4];
__declspec(align(16)) short LoopFLimitMmx[4];
__declspec(align(16)) short Rows[80];
__declspec(align(16)) short NewRows[64];
__declspec(align(16)) short LoopFilteredValuesUp[4];
__declspec(align(16)) short LoopFilteredValuesDown[4];

__declspec(align(16)) unsigned short Variance11[4];
__declspec(align(16)) unsigned short Variance12[4];
__declspec(align(16)) unsigned short Variance21[4];
__declspec(align(16)) unsigned short Variance22[4];
#endif

    LoopFLimit = DeblockLimitValuesV2[pbi->FrameQIndex];
    LoopFLimitMmx[0] = (INT16)LoopFLimit;
    LoopFLimitMmx[1] = (INT16)LoopFLimit;
    LoopFLimitMmx[2] = (INT16)LoopFLimit;
    LoopFLimitMmx[3] = (INT16)LoopFLimit;

	while(CurrentFrag < StartFrag + FragAcross )
	{

		Src=SrcPtr+8*(CurrentFrag-StartFrag);
		Des=DesPtr+8*(CurrentFrag-StartFrag);

		QStep = QuantScale[ pbi->FragQIndex[CurrentFrag+FragAcross]];


		__asm 
		{
			
		push		eax

		push		ebp
			
		push		ecx			

		push		edx

		push		esi

		push		edi

			/* Calculate the FLimit and store FLimit and QStep */					
			/* Copy the data to the intermediate buffer */							
			mov			eax,	QStep
			xor			edx,	edx					/* clear edx */					

			mov			ecx,	PlaneLineStep		/* ecx = Pitch */				
			pcmpeqw		mm6,	mm6					
			
			
			movd		mm5,	eax
			mov			eax,	Src					/* eax = Src */					
							
			psrlw		mm6,	14					/* mm6 = 3, 3, 3, 3*/
			punpcklwd	mm5,	mm5					

			lea			esi,	NewRows				/* esi = NewRows */
			punpckldq	mm5,	mm5

			sub			edx,	ecx					/* edx = - Pitch */
			pmullw		mm6,	mm5					/* Qstep * 3 */

			movq		QStepMmx,	mm5
			lea			edi,	Rows				/* edi = Rows */				

			pxor		mm7,	mm7					/* Clear mm7 */					
			pmullw		mm6,	mm5
			
			lea			eax,	[eax + edx * 4 ]	/* eax = Src - 4*Pitch */		
			movq		mm0,	[eax + edx]			/* mm0 = Src[-5*Pitch] */		
			
			movq		mm1,	mm0					/* mm1 = mm0 */					
			punpcklbw	mm0,	mm7					/* Lower Four -5 */				

			psrlw		mm6,	5
			movq		[FLimitMmx], mm6		
			
			movq		mm2,	[eax]				/* mm2 = Src[-4*Pitch] */		
			punpckhbw	mm1,	mm7					/* Higher Four -5 */			

			movq		mm3,	mm2					/* mm3 = mm2 */					
			punpcklbw	mm2,	mm7					/* Lower Four -4 */				

			movq		[edi],	mm0					/* Write Lower Four of -5 */					
			punpckhbw	mm3,	mm7					/* higher Four -4 */			
			
			movq		[edi+8], mm1				/* Write Higher Four of -5 */	
			movq		mm4,	[eax + ecx]			/* mm4 = Src[-3*Pitch] */		
			
			movq		[edi+16], mm2				/* Write Lower -4 */			
			movq		[edi+24], mm3				/* write hight -4 */			
			
			movq		mm5,	mm4					/* mm5 = mm4 */					
			punpcklbw	mm4,	mm7					/* lower four -3 */				
			
			movq		mm0,	[eax + ecx *2]		/* mm0 = Src[-2*Pitch] */		
			punpckhbw	mm5,	mm7					/* higher four -3 */			
			
			movq		mm1,	mm0					/* mm1 = mm0 */					
			movq		[edi+32], mm4				/* write Lower -3 */			
			
			punpcklbw	mm0,	mm7					/* lower four -2 */				
			lea			eax,	[eax + ecx *4]		/* eax = Src */					
			
			movq		[edi+40], mm5				/* write Higher -3 */			
			punpckhbw	mm1,	mm7					/* higher four -2 */			
			
			movq		mm2,	[eax + edx]			/* mm2 = Src[-Pitch] */			
			movq		[edi+48], mm0				/* lower -2	*/					
			
			movq		mm3,	mm2					/* mm3 = mm2 */					
			punpcklbw	mm2,	mm7					/* lower -1 */					
			
			movq		[edi+56], mm1				/* higher -2 */					
			punpckhbw	mm3,	mm7					/* Higher -1 */					
			
			movq		mm4,	[eax]				/* mm4 = Src[0] */				
			movq		[edi+64], mm2				/* Lower -1 */					
			
			movq		mm5,	mm4					/* mm5 = mm4 */					
			movq		[edi+72], mm3				/* Higher -1 */					
			
			punpcklbw	mm4,	mm7					/* lower 0 */					
			punpckhbw	mm5,	mm7					/* higher 0 */					
			
			movq		mm0,	[eax + ecx]			/* mm0 = Src[Pitch] */			
			movq		[edi+80], mm4				/* write lower 0 */				
			
			movq		mm1,	mm0					/* mm1 = mm0 */					
			movq		[edi+88], mm5				/* write higher 0 */			
			
			punpcklbw	mm0,	mm7					/* lower 1 */					
			punpckhbw	mm1,	mm7					/* higher 1 */					
			
			movq		mm2,	[eax + ecx *2 ]     /* mm2 = Src[2*Pitch] */		
			lea			eax,	[eax + ecx *4]		/* eax = Src + 4 * Pitch  */	
			
			movq		mm3,	mm2					/* mm3 = mm2 */					
			movq		[edi+96], mm0				/* write lower 1 */				
			
			punpcklbw	mm2,	mm7					/* lower 2 */					
			punpckhbw	mm3,	mm7					/* higher 2 */					
			
			movq		mm4,	[eax + edx ]		/* mm4 = Src[3*pitch] */		
			movq		[edi+104], mm1				/* wirte higher 1 */			
			
			movq		mm5,	mm4					/* mm5 = mm4 */					
			punpcklbw	mm4,	mm7					/* Low 3	*/					
			
			movq		[edi+112], mm2				/* write lower 2 */				
			movq		[edi+120], mm3				/* write higher 2 */			
			
			movq		mm0,	[eax]				/* mm0 = Src[4*pitch] */		
			punpckhbw	mm5,	mm7					/* high 3 */					
			
			movq		mm1,	mm0					/* mm1=mm0 */					
			movq		[edi+128], mm4				/* low 3 */						
			
			punpcklbw	mm0,	mm7					/* low 4 */						
			punpckhbw	mm1,	mm7					/* high 4 */					
			
			movq		[edi+136], mm5				/* high 3 */					
			movq		[edi+144], mm0				/* low 4 */						
			
			movq		[edi+152], mm1				/* high 4 */					
			
/*
			mov			eax,	Des					
			lea			eax,	[eax+edx*4]
			movq		mm2,	[eax]
			movq		mm2,	[eax+ecx]
			movq		mm2,	[eax+ecx*2]
			lea			eax,	[eax+ecx*4]
			movq		mm2,	[eax+edx]
			movq		mm2,	[eax]
			movq		mm2,	[eax+ecx]
			movq		mm2,	[eax+ecx*2]
			lea			eax,	[eax+ecx*4]
			movq		mm2,	[eax+edx]
			movq		mm2,	[eax]
			
*/
	
			/* done with copying everything to intermediate buffer */				
			/* Now, compute the variances for Pixel  1-4 and 5-8 */					
	
			/* we use mm0,mm1,mm2 for 1234 and mm4, mm5, mm6 for 5-8 */				
			/* mm7 = 0, mm3 = {128, 128, 128, 128} */								
			

			pcmpeqw		mm3,	mm3					/* mm3 = FFFFFFFFFFFFFFFF */	
			psllw		mm3,	15					/* mm3 = 8000800080008000 */	
			psrlw		mm3,	8					/* mm3 = 0080008000800080 */
			
			movq		mm2,	[edi+16]			/* Pixel 1 */					
			movq		mm6,	[edi+80]			/* Pixel 5 */					
			
			psubw		mm2,	mm3					/* mm2 -=128 */					
			psubw		mm6,	mm3					/* mm6 -=128 */					
			
			movq		mm0,	mm2					/* mm0 = pixel 1 */				
			movq		mm4,	mm6					/* mm4 = pixel 5 */				
			
			pmullw		mm2,	mm2					/* mm2 = pixel1 * pixel1 */		
			pmullw		mm6,	mm6					/* mm6 = pixel5 * pixel5 */		
			
			movq		mm1,	mm2					/* mm1 = pixel1^2 */			
			movq		mm5,	mm6					/* mm5 = pixel5^2 */			
			
			movq		mm2,	[edi+32]			/* Pixel 2 */					
			movq		mm6,	[edi+96]			/* Pixel 6 */					
			
			psubw		mm2,	mm3					/* mm2 -=128 */					
			psubw		mm6,	mm3					/* mm6 -=128 */					
			
			paddw		mm0,	mm2					/* mm0 += pixel 2 */			
			paddw		mm4,	mm6					/* mm4 += pixel 6 */			
			
			pmullw		mm2,	mm2					/* mm2 = pixel2^2 */			
			pmullw		mm6,	mm6					/* mm6 = pixel6^2 */			
			
			paddw		mm1,	mm2					/* mm1 += pixel2^2 */			
			paddw		mm5,	mm6					/* mm5 += pixel6^2 */			
			
			movq		mm2,	[edi+48]			/* Pixel 3 */					
			movq		mm6,	[edi+112]			/* Pixel 7 */					
			
			psubw		mm2,	mm3					/* mm2 -=128 */					
			psubw		mm6,	mm3					/* mm6 -=128 */					
			
			paddw		mm0,	mm2					/* mm0 += pixel 3 */			
			paddw		mm4,	mm6					/* mm4 += pixel 7 */			
			
			pmullw		mm2,	mm2					/* mm2 = pixel3^2 */			
			pmullw		mm6,	mm6					/* mm6 = pixel7^2 */			
			
			paddw		mm1,	mm2					/* mm1 += pixel3^2 */			
			paddw		mm5,	mm6					/* mm5 += pixel7^2 */			
			
			movq		mm2,	[edi+64]			/* Pixel 4 */					
			movq		mm6,	[edi+128]			/* Pixel 8 */					
			
			psubw		mm2,	mm3					/* mm2 -=128 */					
			psubw		mm6,	mm3					/* mm6 -=128 */					
			
			paddw		mm0,	mm2					/* mm0 += pixel 4 */			
			paddw		mm4,	mm6					/* mm4 += pixel 8 */			
			
			pmullw		mm2,	mm2					/* mm2 = pixel4^2 */			
			pmullw		mm6,	mm6					/* mm6 = pixel8^2 */			
			
			paddw		mm1,	mm2					/* mm1 = pixel4^2 */			
			paddw		mm5,	mm6					/* mm5 = pixel8^2 */			
			
			
			/* mm0 = x1^2 + x2^2 + x3^2 + x4^2 */									
			/* mm1 = x1 + x2 + x3 + x4 */											
			/* mm4 = x5^2 + x6^2 + x7^2 + x8^2 */									
			/* mm5 = x5 + x6 + x7 + x8 */											
			
			movq		mm7,	mm3					/* mm7 = mm3 */					
			psrlw		mm7,	7					/* mm7 = 0001000100010001 */	
			
			movq		mm2,	mm0					/* make copy of sum1 */			
			movq		mm6,	mm4					/* make copy of sum2 */			
			
			paddw		mm0,	mm7					/* (sum1 + 1) */				
			paddw		mm4,	mm7					/* (sum2 + 1) */				
			
			psraw		mm2,	1					/* sum1 /2 */					
			psraw		mm6,	1					/* sum2 /2 */					
			
			psraw		mm0,	1					/* (sum1 + 1)/2 */				
			psraw		mm4,	1					/* (sum2 + 1)/2 */				
			
			pmullw		mm2,	mm0					/* (sum1)/2*(sum1+1)/2 */		
			pmullw		mm6,	mm4					/* (sum2)/2*(sum2+1)/2 */		
			
			psubw		mm1,	mm2					/* Variance 1 */				
			psubw		mm5,	mm6					/* Variance 2 */				
			
			movq		mm7,	FLimitMmx			/* mm7 = FLimit */				
			movq		mm2,	mm1					/* copy of Varinace 1*/

			movq		mm6,	mm5					/* Variance 2 */
			movq		[Variance11], mm1			/* Save Variance1 */

			movq		[Variance21], mm5			/* Save Variance2 */
			psubw		mm1,	mm7					/* Variance 1 < Flimit? */		
			
			psubw		mm5,	mm7					/* Variance 2 < Flimit? */		
			psraw		mm2,	15					/* Variance 1 > 32768? */

			psraw		mm6,	15					/* Vaiance  2 > 32768? */	
			psraw		mm1,	15					/* FFFF/0000 for true/false */	
			
			psraw		mm5,	15					/* FFFF/0000 for true/false */	
			movq		mm7,	[edi+64]			/* mm0 = Pixel 4			*/	

			pandn		mm2,	mm1					/* Variance1<32678 && 
													   Variance1<Limit			*/
			pandn		mm6,	mm5					/* Variance2<32678 && 
													   Variance1<Limit			*/
			
			movq		mm4,	[edi+80]			/* mm4 = Pixel 5			*/	
			pand		mm6,	mm2					/* mm6 = Variance1 < Flimit */	
													/*     &&Variance2 < Flimit */	

			movq		mm2,	mm7					/* make copy of Pixel4		*/	

			psubusw		mm7,	mm4					/* 4 - 5 */						
			psubusw		mm4,	mm2					/* 5 - 4 */						
			
			por			mm7,	mm4					/* abs(4 - 5) */				
			psubw		mm7,	QStepMmx			/* abs(4-5)<QStepMmx ? */		
			
			psraw		mm7,	15					/* FFFF/0000 for True/Flase */
			pand		mm7,	mm6													
			
			/* mm7 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
			/* now lets look at the right four colomn */							
			
			add			edi,	8					/* offset 8 to right 4 cols */	
			
			movq		mm2,	[edi+16]			/* Pixel 1 */					
			movq		mm6,	[edi+80]			/* Pixel 5 */					
			
			psubw		mm2,	mm3					/* mm2 -=128 */					
			psubw		mm6,	mm3					/* mm6 -=128 */					
			
			movq		mm0,	mm2					/* mm0 = pixel 1 */				
			movq		mm4,	mm6					/* mm4 = pixel 5 */				
			
			pmullw		mm2,	mm2					/* mm2 = pixel1 * pixel1 */		
			pmullw		mm6,	mm6					/* mm6 = pixel5 * pixel5 */		
			
			movq		mm1,	mm2					/* mm1 = pixel1^2 */			
			movq		mm5,	mm6					/* mm5 = pixel5^2 */			
			
			movq		mm2,	[edi+32]			/* Pixel 2 */					
			movq		mm6,	[edi+96]			/* Pixel 6 */					
			
			psubw		mm2,	mm3					/* mm2 -=128 */					
			psubw		mm6,	mm3					/* mm6 -=128 */					
			
			paddw		mm0,	mm2					/* mm0 += pixel 2 */			
			paddw		mm4,	mm6					/* mm4 += pixel 6 */			
			
			pmullw		mm2,	mm2					/* mm2 = pixel2^2 */			
			pmullw		mm6,	mm6					/* mm6 = pixel6^2 */			
			
			paddw		mm1,	mm2					/* mm1 += pixel2^2 */			
			paddw		mm5,	mm6					/* mm5 += pixel6^2 */			
			
			movq		mm2,	[edi+48]			/* Pixel 3 */					
			movq		mm6,	[edi+112]			/* Pixel 7 */					
			
			psubw		mm2,	mm3					/* mm2 -=128 */					
			psubw		mm6,	mm3					/* mm6 -=128 */					
			
			paddw		mm0,	mm2					/* mm0 += pixel 3 */			
			paddw		mm4,	mm6					/* mm4 += pixel 7 */			
			
			pmullw		mm2,	mm2					/* mm2 = pixel3^2 */			
			pmullw		mm6,	mm6					/* mm6 = pixel7^2 */			
			
			paddw		mm1,	mm2					/* mm1 += pixel3^2 */			
			paddw		mm5,	mm6					/* mm5 += pixel7^2 */			
			
			movq		mm2,	[edi+64]			/* Pixel 4 */					
			movq		mm6,	[edi+128]			/* Pixel 8 */					
			
			psubw		mm2,	mm3					/* mm2 -=128 */					
			psubw		mm6,	mm3					/* mm6 -=128 */					
			
			paddw		mm0,	mm2					/* mm0 += pixel 4 */			
			paddw		mm4,	mm6					/* mm4 += pixel 8 */			
			
			pmullw		mm2,	mm2					/* mm2 = pixel4^2 */			
			pmullw		mm6,	mm6					/* mm6 = pixel8^2 */			
			
			paddw		mm1,	mm2					/* mm1 = pixel4^2 */			
			paddw		mm5,	mm6					/* mm5 = pixel8^2 */			
			
			/* mm0 = x1^2 + x2^2 + x3^2 + x4^2 */									
			/* mm1 = x1 + x2 + x3 + x4 */											
			/* mm4 = x5^2 + x6^2 + x7^2 + x8^2 */									
			/* mm5 = x5 + x6 + x7 + x8 */											
			
			psrlw		mm3,	7					/* mm3 = 0001000100010001 */	
			
			movq		mm2,	mm0					/* make copy of sum1 */			
			movq		mm6,	mm4					/* make copy of sum2 */			
			
			paddw		mm0,	mm3					/* (sum1 + 1) */				
			paddw		mm4,	mm3					/* (sum2 + 1) */				
			
			psraw		mm2,	1					/* sum1 /2 */					
			psraw		mm6,	1					/* sum2 /2 */					
			
			psraw		mm0,	1					/* (sum1 + 1)/2 */				
			psraw		mm4,	1					/* (sum2 + 1)/2 */				
			
			pmullw		mm2,	mm0					/* (sum1)/2*(sum1+1)/2 */		
			pmullw		mm6,	mm4					/* (sum2)/2*(sum2+1)/2 */		
			
			psubw		mm1,	mm2					/* Variance 1 */				
			psubw		mm5,	mm6					/* Variance 2 */				

			movq		[Variance12], mm1			/* Save Variance1 */
			movq		[Variance22], mm5			/* Save Variance2 */
			
			movq		mm3,	FLimitMmx			/* mm3 = FLimit */				
			movq		mm2,	mm1					/* copy of Varinace 1*/

			movq		mm6,	mm5					/* Variance 2 */
			psubw		mm1,	mm3					/* Variance 1 < Flimit? */		
			
			psubw		mm5,	mm3					/* Variance 2 < Flimit? */		
			psraw		mm2,	15					/* Variance 1 > 32768? */

			psraw		mm6,	15					/* Vaiance  2 > 32768? */	
			psraw		mm1,	15					/* FFFF/0000 for true/false */	
			
			psraw		mm5,	15					/* FFFF/0000 for true/false */	
			movq		mm0,	[edi+64]			/* mm0 = Pixel 4			*/	

			pandn		mm2,	mm1					/* Variance1<32678 && 
													   Variance1<Limit			*/
			pandn		mm6,	mm5					/* Variance2<32678 && 
													   Variance1<Limit			*/

			movq		mm4,	[edi+80]			/* mm4 = Pixel 5			*/	
			pand		mm6,	mm2					/* mm6 = Variance1 < Flimit */	
													/*     &&Variance2 < Flimit */	
			movq		mm2,	mm0					/* make copy of Pixel4		*/	
													
			psubusw		mm0,	mm4					/* 4 - 5 */						
			psubusw		mm4,	mm2					/* 5 - 4 */						
			
			por			mm0,	mm4					/* abs(4 - 5) */				
			psubw		mm0,	QStepMmx			/* abs(4-5)<QStepMmx ? */		
			
			psraw		mm0,	15					/* FFFF/0000 for True/False */
			pand		mm0,	mm6				
			
			sub			edi,	8					/* offset edi back */			
			
			/* mm0 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
			/* mm0 and mm7 now are in use  */										
            
            /* find the loop filtered values for the pixels on block boundary */
            movq        mm1,    LoopFLimitMmx;      /* Get the Flimit values for loop filter */
            movq        mm3,    [edi + 48]          /* mm3 = x3 = p[-2] */

            movq        mm4,    [edi + 64]          /* mm4 = x4 = p[-1] */
            movq        mm5,    [edi + 80]          /* mm5 = x5 = p[ 0] */

            movq        mm6,    [edi + 96]          /* mm6 = x6 = p[ 1] */
            psubw       mm5,    mm4                 /* mm5 = p[ 0] - p[-1] */

            psubw       mm3,    mm6                 /* mm3 = p[-2] - p[ 1] */
            movq        mm4,    mm5                 /* make a copy */

            paddw       mm4,    mm5                 /* 2 * ( p[0] - p[-1] ) */
            paddw       mm3,    FourFours           /* mm3 + 4 */

            paddw       mm5,    mm4                 /* 3 * ( p[0] - p[-1] ) */
            paddw       mm3,    mm5                 /* Filtval before shift */

            psraw       mm3,    3                   /* FiltVal */
            movq        mm2,    mm3                 /* make a copy */

            psraw       mm3,    15                  /* FFFF->Neg, 0000->Pos */
            pxor        mm2,    mm3

            psubsw      mm2,    mm3                 /* mm2 = abs(FiltVal) */
            por         mm3,    FourOnes            /* -1 and 1 for + and - */

            movq        mm4,    mm1                 /* make a copy of Flimit */
            psubw       mm1,    mm2                 /* mm1= Flimit - abs(FiltVal) */

            movq        mm5,    mm1                 /* copy Flimit - abs(FiltVal) */
            psraw       mm1,    15                  /* FFFF or 0000 */

            pxor        mm5,    mm1                 
            psubsw      mm5,    mm1                 /* abs(Flimit - abs(FiltVal)) */

            psubusw     mm4,    mm5                 /* Flimit-abs(Flimit - abs(FiltVal)) */
            pmullw      mm4,    mm3                 /* get the sign back */

            movq        mm1,    [edi+64]            /* p[-1] */
            movq        mm2,    [edi+80]            /* p[0] */
            
            paddw       mm1,    mm4                 /* p[-1] + NewFiltVal */
            psubw       mm2,    mm4                 /* p[0] - NewFiltVal */

            pxor        mm6,    mm6                 /* clear mm6 */
            
            packuswb    mm1,    mm1                 /* clamping */
            packuswb    mm2,    mm2                 /* clamping */

            punpcklbw   mm1,    mm6                 /* unpack to word */
            movq        LoopFilteredValuesUp, mm1   /* save the values */

            punpcklbw   mm2,    mm6                 /* unpack to word */
            movq        LoopFilteredValuesDown, mm2 /* save the values */
            

			/* Let's do the filtering now */										
			/* p1 = (abs(Src[-4] - Src[-5]) < QStep ) ?  Src[-5] : Src[-4]; */		
			/* p2 = (abs(Src[+3] - Src[+4]) < QStep ) ?  Src[+4] : Src[+3]; */		
			
			movq		mm5,	[edi]				/* mm5 = -5 */					
			movq		mm4,	[edi + 16]			/* mm4 = -4 */					
			
			movq		mm3,	mm4					/* copy of -4 */				
			movq		mm6,	mm5					/* copy of -5 */				
			
			psubusw		mm4,	mm6					/* mm4 = [-4] - [-5] */			
			psubusw		mm5,	mm3					/* mm5 = [-5] - [-4] */			
			
			por			mm4,	mm5					/* abs([-4]-[-5] ) */			
			psubw		mm4,	QStepMmx			/* abs([-4]-[-5] )<QStep? */	
			
			psraw		mm4,	15					/* FFFF/0000 for True/False */	
			movq		mm1,	mm4					/* copy of the mm4 */			
			
			pand		mm4,	mm6					/*							*/	
			pandn		mm1,	mm3					/*							*/	
			
			por			mm1,	mm4					/* mm1 = p1					*/	
			
			/* now find P2 */														
			
			movq		mm4,	[edi+128]			/* mm4 = [3] */					
			movq		mm5,	[edi+144]			/* mm5 = [4] */					
			
			movq		mm3,	mm4					/* copy of 3 */					
			movq		mm6,	mm5					/* copy of 4 */					
			
			psubusw		mm4,	mm6					/* mm4 = [3] - [4] */			
			psubusw		mm5,	mm3					/* mm5 = [4] - [3] */			
			
			por			mm4,	mm5					/* abs([3]-[4] ) */				
			psubw		mm4,	QStepMmx			/* abs([3]-[4] )<QStep? */		
			
			psraw		mm4,	15					/* FFFF/0000 for True/False */	
			movq		mm2,	mm4					/* copy of the mm4 */			
			
			pand		mm4,	mm6					/*							*/	
			pandn		mm2,	mm3					/*							*/	
			
			por			mm2,	mm4					/* mm2 = p2					*/	
			
			/* sum = p1 + p1 + p1 + x1 + x2 + x3 + x4 + 4; */				
			/* Des[-w4] = (((sum + x1) << 1) - (x4 - x5)) >> 4; */			
			/* Des[-w4] = Src[-w4]; */												
			/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	
			
			movq		mm3,	mm1					/* mm3 = p1 */					
			paddw		mm3,	mm3					/* mm3 = p1 + p1 */				
			
			paddw		mm3,	mm1					/* mm3 = p1 + p1 + p1 */		
			movq		mm4,	[edi+16]			/* mm4 = x1 */					
			
			paddw		mm3,	[edi+32]			/* mm3 = p1+p1+p1+ x2 */		
			paddw		mm4,	[edi+48]			/* mm4 = x1+x3 */				
			
			paddw		mm3,	[edi+64]			/* mm3 += x4 */					
			paddw		mm4,	FourFours			/* mm4 = x1 + x3 + 4 */			
			
			paddw		mm3,	mm4					/* mm3 = 3*p1+x1+x2+x3+x4+4 */	
			movq		mm4,	mm3					/* mm4 = mm3 */					
			
			movq		mm5,	[edi+16]			/* mm5 = x1 */					
			paddw		mm4,	mm5					/* mm4 = sum+x1 */				
			
			psllw		mm4,	1					/* mm4 = (sum+x1)<<1 */			
			psubw		mm4,	[edi+64]			/* mm4 = (sum+x1)<<1-x4 */		
			
			paddw		mm4,	[edi+80]			/* mm4 = (sum+x1)<<1-x4+x5 */	
			psraw		mm4,	4					/* mm4 >>=4 */					
			
			psubw		mm4,	mm5					/* New Value - old Value */		
			pand		mm4,	mm7					/* And the flag */				
			
			paddw		mm4,	mm5					/* add the old value back */	
			movq		[esi],	mm4					/* Write new x1 */				
			
			/* sum += x5 -p1 */														
			/* Des[-w3]=((sum+x2)<<1-x5+x6)>>4 */									
			
			movq		mm5,	[edi+32]			/* mm5= x2 */					
			psubw		mm3,	mm1					/* sum=sum-p1 */				
			
			paddw		mm3,    [edi+80]			/* sum=sum+x5 */				
			movq		mm4,	mm5					/* copy sum */					
			
			paddw		mm4,	mm3					/* mm4=sum+x2 */				
			paddw		mm4,	mm4					/* mm4 <<= 1 */					
			
			psubw		mm4,	[edi+80]			/* mm4 =(sum+x2)<<1-x5 */		
			paddw		mm4,	[edi+96]			/* mm4 =(sum+x2)<<1-x5+x6 */	
			
			psraw		mm4,	4					/* mm4=((sum+x2)<<1-x5+x6)>>4 */
			psubw		mm4,	mm5					/* new value - old value	*/	
			
			pand		mm4,	mm7					/* And the flag */				
			paddw		mm4,	mm5					/* add the old value back */	
			
			movq		[esi+16], mm4				/* write new x2 */				
			
			/* sum += x6 - p1 */													
			/* Des[-w2]=((sum+x[3])<<1-x[6]+x[7])>>4 */								
			
			movq		mm5,	[edi+48]			/* mm5= x3 */					
			psubw		mm3,	mm1					/* sum=sum-p1 */				
			
			paddw		mm3,    [edi+96]			/* sum=sum+x6 */				
			movq		mm4,	mm5					/* copy x3 */					
			
			paddw		mm4,	mm3					/* mm4=sum+x3 */				
			paddw		mm4,	mm4					/* mm4 <<= 1 */					
			
			psubw		mm4,	[edi+96]			/* mm4 =(sum+x3)<<1-x6 */		
			paddw		mm4,	[edi+112]			/* mm4 =(sum+x3)<<1-x6+x7 */	
			
			psraw		mm4,	4					/* mm4=((sum+x3)<<1-x6+x7)>>4 */
			psubw		mm4,	mm5					/* new value - old value	*/	
			
			pand		mm4,	mm7					/* And the flag */				
			paddw		mm4,	mm5					/* add the old value back */	
			
			movq		[esi+32], mm4				/* write new x3 */				
			
			/* sum += x7 - p1 */													
			/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
			
			movq		mm5,	[edi+64]			/* mm5 = x4 */					
			psubw		mm3,	mm1					/* sum = sum-p1 */				
			
			paddw		mm3,	[edi+112]			/* sum = sum+x7 */				
			movq		mm4,	mm5					/* mm4 = x4 */					
			
			paddw		mm4,	mm3					/* mm4 = sum + x4 */			
			paddw		mm4,	mm4					/* mm4 *=2 */					
			
			paddw		mm4,	mm1					/* += p1 */						
			psubw		mm4,	[edi+16]			/* -= x1 */						
			
			psubw		mm4,	[edi+112]			/* -= x7 */						
			paddw		mm4,	[edi+128]			/* += x8 */						
			
			movq        mm5,    LoopFilteredValuesUp/* Read the loopfiltered value of x4 */
            psraw		mm4,	4					/* >>=4 */						
            
            psubw		mm4,	mm5					/* -=x4 */						
			pand		mm4,	mm7					/* and flag */					

            paddw		mm4,	mm5					/* += x4 */						
			movq		[esi+48], mm4				/* write new x4 */				
			
			/* sum+= x8-x1 */														
			/* Des[0]=((sum+x5)<<1+x1-x2-x8+p2)>>4 */								
			
			movq		mm5,	[edi+80]			/* mm5 = x5 */					
			psubw		mm3,	[edi+16]			/* sum -= x1 */					
			
			paddw		mm3,	[edi+128]			/* sub += x8 */					
			movq		mm4,	mm5					/* mm4 = x5 */					
			
			paddw		mm4,	mm3					/* mm4= sum+x5 */				
			paddw		mm4,	mm4					/* mm4 *= 2 */					
			
			paddw		mm4,	[edi+16]			/* += x1 */						
			psubw		mm4,	[edi+32]			/* -= x2 */						
			
			psubw		mm4,	[edi+128]			/* -= x8 */						
			paddw		mm4,	mm2					/* += p2 */						
			
			movq        mm5,    LoopFilteredValuesDown/* Read the loopfiltered value of x4 */
            psraw		mm4,	4					/* >>=4 */						

            psubw		mm4,	mm5					/* -=x5 */						
			pand		mm4,	mm7					/* and flag */					

            paddw		mm4,	mm5					/* += x5 */										
			movq		[esi+64], mm4				/* write new x5 */				
			
			/* sum += p2 - x2 */													
			/* Des[w1] = ((sum+x6)<<1 + x2-x3)>>4 */								
			
			movq		mm5,	[edi+96]			/* mm5 = x6 */					
			psubw		mm3,	[edi+32]			/* -= x2 */						
			
			paddw		mm3,	mm2					/* += p2 */						
			movq		mm4,	mm5					/* mm4 = x6 */					
			
			paddw		mm4,	mm3					/* mm4 = sum+x6 */				
			paddw		mm4,	mm4					/* mm4 *= 2*/					
			
			paddw		mm4,	[edi+32]			/* +=x2 */						
			psubw		mm4,	[edi+48]			/* -=x3 */						
			
			psraw		mm4,	4					/* >>=4 */						
			psubw		mm4,	mm5					/* -=x6 */						
			
			pand		mm4,	mm7					/* and flag */					
			paddw		mm4,	mm5					/* += x6 */						
			
			movq		[esi+80], mm4				/* write new x6 */				
			
			/* sum += p2 - x3 */													
			/* Des[w2] = ((sum+x7)<<1 + x3-x4)>>4 */								
			
			movq		mm5,	[edi+112]			/* mm5 = x7 */					
			psubw		mm3,	[edi+48]			/* -= x3 */						
			
			paddw		mm3,	mm2					/* += p2 */						
			movq		mm4,	mm5					/* mm4 = x7 */					
			
			paddw		mm4,	mm3					/* mm4 = sum+x7 */				
			paddw		mm4,	mm4					/* mm4 *= 2*/					
			
			paddw		mm4,	[edi+48]			/* +=x3 */						
			psubw		mm4,	[edi+64]			/* -=x4 */						
			
			psraw		mm4,	4					/* >>=4 */						
			psubw		mm4,	mm5					/* -=x7 */						
			
			pand		mm4,	mm7					/* and flag */					
			paddw		mm4,	mm5					/* += x7 */						
			
			movq		[esi+96], mm4				/* write new x7 */				
			
			/* sum += p2 - x4 */													
			/* Des[w3] = ((sum+x8)<<1 + x4-x5)>>4 */								
			
			movq		mm5,	[edi+128]			/* mm5 = x8 */					
			psubw		mm3,	[edi+64]			/* -= x4 */						
			
			paddw		mm3,	mm2					/* += p2 */						
			movq		mm4,	mm5					/* mm4 = x8 */					
			
			paddw		mm4,	mm3					/* mm4 = sum+x8 */				
			paddw		mm4,	mm4					/* mm4 *= 2*/					
			
			paddw		mm4,	[edi+64]			/* +=x4 */						
			psubw		mm4,	[edi+80]			/* -=x5 */						
			
			psraw		mm4,	4					/* >>=4 */						
			psubw		mm4,	mm5					/* -=x8 */						
			
			pand		mm4,	mm7					/* and flag */					
			paddw		mm4,	mm5					/* += x8 */						
			
			movq		[esi+112], mm4				/* write new x8 */				
			
			/* done with left four columns */										
			/* now do the righ four columns */										
			
			add			edi,	8					/* shift to right four column */
			add			esi,	8					/* shift to right four column */
			
			/* mm0 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
			/* mm0 now are in use  */										
			
            /* find the loop filtered values for the pixels on block boundary */

            movq        mm1,    LoopFLimitMmx;      /* Get the Flimit values for loop filter */
            movq        mm3,    [edi + 48]          /* mm3 = x3 = p[-2] */

            movq        mm4,    [edi + 64]          /* mm4 = x4 = p[-1] */
            movq        mm5,    [edi + 80]          /* mm5 = x5 = p[ 0] */

            movq        mm6,    [edi + 96]          /* mm6 = x6 = p[ 1] */
            psubw       mm5,    mm4                 /* mm5 = p[ 0] - p[-1] */

            psubw       mm3,    mm6                 /* mm3 = p[-2] - p[ 1] */
            movq        mm4,    mm5                 /* make a copy */

            paddw       mm3,    FourFours           /* mm3 + 4 */
            paddw       mm4,    mm4                 /* 2 * ( p[0] - p[-1] ) */

            paddw       mm3,    mm4                 /* 3 * ( p[0] - p[-1] ) */
            paddw       mm3,    mm5                 /* Filtval before shift */

            psraw       mm3,    3                   /* FiltVal */
            movq        mm2,    mm3                 /* make a copy */

            psraw       mm3,    15                  /* FFFF->Neg, 0000->Pos */
            pxor        mm2,    mm3

            psubsw      mm2,    mm3                 /* mm2 = abs(FiltVal) */
            por         mm3,    FourOnes            /* -1 and 1 for + and - */

            movq        mm4,    mm1                 /* make a copy of Flimit */
            psubw       mm1,    mm2                 /* mm1= Flimit - abs(FiltVal) */

            movq        mm5,    mm1                 /* copy Flimit - abs(FiltVal) */
            psraw       mm1,    15                  /* FFFF or 0000 */

            pxor        mm5,    mm1                 
            psubsw      mm5,    mm1                 /* abs(Flimit - abs(FiltVal)) */

            psubusw     mm4,    mm5                 /* Flimit-abs(Flimit - abs(FiltVal)) */
            pmullw      mm4,    mm3                 /* get the sign back */

            movq        mm1,    [edi+64]            /* p[-1] */
            movq        mm2,    [edi+80]            /* p[0] */
            
            paddw       mm1,    mm4                 /* p[-1] + NewFiltVal */
            psubw       mm2,    mm4                 /* p[0] - NewFiltVal */

            pxor        mm6,    mm6                 /* clear mm6 */
            
            packuswb    mm1,    mm1                 /* clamping */
            packuswb    mm2,    mm2                 /* clamping */

            punpcklbw   mm1,    mm6                 /* unpack to word */
            movq        LoopFilteredValuesUp, mm1   /* save the values */

            punpcklbw   mm2,    mm6                 /* unpack to word */
            movq        LoopFilteredValuesDown, mm2 /* save the values */
            
            
            /* Let's do the filtering now */										
			/* p1 = (abs(Src[-4] - Src[-5]) < QStep ) ?  Src[-5] : Src[-4]; */		
			/* p2 = (abs(Src[+3] - Src[+4]) < QStep ) ?  Src[+4] : Src[+3]; */		
			
			movq		mm5,	[edi]				/* mm5 = -5 */					
			movq		mm4,	[edi + 16]			/* mm4 = -4 */					
			
			movq		mm3,	mm4					/* copy of -4 */				
			movq		mm6,	mm5					/* copy of -5 */				
			
			psubusw		mm4,	mm6					/* mm4 = [-4] - [-5] */			
			psubusw		mm5,	mm3					/* mm5 = [-5] - [-4] */			
			
			por			mm4,	mm5					/* abs([-4]-[-5] ) */			
			psubw		mm4,	QStepMmx			/* abs([-4]-[-5] )<QStep? */	
			
			psraw		mm4,	15					/* FFFF/0000 for True/False */	
			movq		mm1,	mm4					/* copy of the mm4 */			
			
			pand		mm4,	mm6					/*							*/	
			pandn		mm1,	mm3					/*							*/	
			
			por			mm1,	mm4					/* mm1 = p1					*/	
			
			/* now find P2 */														
			
			movq		mm4, [edi+128]				/* mm4 = [3] */					
			movq		mm5, [edi+144]				/* mm5 = [4] */					
			
			movq		mm3, mm4					/* copy of 3 */					
			movq		mm6, mm5					/* copy of 4 */					
			
			psubusw		mm4, mm6					/* mm4 = [3] - [4] */			
			psubusw		mm5, mm3					/* mm5 = [4] - [3] */			
			
			por			mm4, mm5					/* abs([3]-[4] ) */				
			psubw		mm4, QStepMmx				/* abs([3]-[4] )<QStep? */		
			
			psraw		mm4, 15						/* FFFF/0000 for True/False */	
			movq		mm2, mm4					/* copy of the mm4 */			
			
			pand		mm4, mm6					/*							*/	
			pandn		mm2, mm3					/*							*/	
			
			por			mm2, mm4					/* mm2 = p2					*/	
			
			/* psum = p1 + p1 + p1 + v[1] + v[2] + v[3] + v[4] + 4; */				
			/* Des[-w4] = (((psum + v[1]) << 1) - (v[4] - v[5])) >> 4; */			
			/* Des[-w4]=Src[-w4]; */												
			/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	
			
			movq		mm3,	mm1					/* mm3 = p1 */					
			paddw		mm3,	mm3					/* mm3 = p1 + p1 */				
			
			paddw		mm3,	mm1					/* mm3 = p1 + p1 + p1 */		
			movq		mm4,	[edi+16]			/* mm4 = x1 */					
			
			paddw		mm3,	[edi+32]			/* mm3 = p1+p1+p1+ x2 */		
			paddw		mm4,	[edi+48]			/* mm4 = x1+x3 */				
			
			paddw		mm3,	[edi+64]			/* mm3 += x4 */					
			paddw		mm4,	FourFours			/* mm4 = x1 + x3 + 4 */			
			
			paddw		mm3,	mm4					/* mm3 = 3*p1+x1+x2+x3+x4+4 */	
			movq		mm4,	mm3					/* mm4 = mm3 */					
			
			movq		mm5,	[edi+16]			/* mm5 = x1 */					
			paddw		mm4,	mm5					/* mm4 = sum+x1 */				
			
			psllw		mm4,	1					/* mm4 = (sum+x1)<<1 */			
			psubw		mm4,	[edi+64]			/* mm4 = (sum+x1)<<1-x4 */		
			
			paddw		mm4,	[edi+80]			/* mm4 = (sum+x1)<<1-x4+x5 */	
			psraw		mm4,	4					/* mm4 >>=4 */					
			
			psubw		mm4,	mm5					/* New Value - old Value */		
			pand		mm4,	mm0					/* And the flag */				
			
			paddw		mm4,	mm5					/* add the old value back */	
			movq		[esi],	mm4					/* Write new x1 */				
			
			/* sum += x5 -p1 */														
			/* Des[-w3]=((sum+x2)<<1-x5+x6)>>4 */									
			
			movq		mm5,	[edi+32]			/* mm5= x2 */					
			psubw		mm3,	mm1					/* sum=sum-p1 */				
			
			paddw		mm3,    [edi+80]			/* sum=sum+x5 */				
			movq		mm4,	mm5					/* copy sum */					
			
			paddw		mm4,	mm3					/* mm4=sum+x2 */				
			paddw		mm4,	mm4					/* mm4 <<= 1 */					
			
			psubw		mm4,	[edi+80]			/* mm4 =(sum+x2)<<1-x5 */		
			paddw		mm4,	[edi+96]			/* mm4 =(sum+x2)<<1-x5+x6 */	
			
			psraw		mm4,	4					/* mm4=((sum+x2)<<1-x5+x6)>>4 */
			psubw		mm4,	mm5					/* new value - old value	*/	
			
			pand		mm4,	mm0					/* And the flag */				
			paddw		mm4,	mm5					/* add the old value back */	
			
			movq		[esi+16], mm4				/* write new x2 */				
			
			/* sum += x6 - p1 */													
			/* Des[-w2]=((sum+x[3])<<1-x[6]+x[7])>>4 */								
			
			movq		mm5,	[edi+48]			/* mm5= x3 */					
			psubw		mm3,	mm1					/* sum=sum-p1 */				
			
			paddw		mm3,    [edi+96]			/* sum=sum+x6 */				
			movq		mm4,	mm5					/* copy x3 */					
			
			paddw		mm4,	mm3					/* mm4=sum+x3 */				
			paddw		mm4,	mm4					/* mm4 <<= 1 */					
			
			psubw		mm4,	[edi+96]			/* mm4 =(sum+x3)<<1-x6 */		
			paddw		mm4,	[edi+112]			/* mm4 =(sum+x3)<<1-x6+x7 */	
			
			psraw		mm4,	4					/* mm4=((sum+x3)<<1-x6+x7)>>4 */
			psubw		mm4,	mm5					/* new value - old value	*/	
			
			pand		mm4,	mm0					/* And the flag */				
			paddw		mm4,	mm5					/* add the old value back */	
			
			movq		[esi+32], mm4				/* write new x3 */				
			
			/* sum += x7 - p1 */													
			/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
			
			movq		mm5,	[edi+64]			/* mm5 = x4 */					
			psubw		mm3,	mm1					/* sum = sum-p1 */				
			
			paddw		mm3,	[edi+112]			/* sum = sum+x7 */				
			movq		mm4,	mm5					/* mm4 = x4 */					
			
			paddw		mm4,	mm3					/* mm4 = sum + x4 */			
			paddw		mm4,	mm4					/* mm4 *=2 */					
			
			paddw		mm4,	mm1					/* += p1 */						
			psubw		mm4,	[edi+16]			/* -= x1 */						
			
			psubw		mm4,	[edi+112]			/* -= x7 */						
			paddw		mm4,	[edi+128]			/* += x8 */						
			
			movq        mm5,    LoopFilteredValuesUp/* Read the loopfiltered value of x4 */
            psraw		mm4,	4					/* >>=4 */						

			psubw		mm4,	mm5					/* -=x4 */						
			pand		mm4,	mm0					/* and flag */					

            paddw		mm4,	mm5					/* += x4 */						
			movq		[esi+48], mm4				/* write new x4 */				

			/* sum+= x8-x1 */														
			/* Des[0]=((sum+x5)<<1+x1-x2-x8+p2)>>4 */								
			
			movq		mm5,	[edi+80]			/* mm5 = x5 */					
			psubw		mm3,	[edi+16]			/* sum -= x1 */					
			
			paddw		mm3,	[edi+128]			/* sub += x8 */					
			movq		mm4,	mm5					/* mm4 = x5 */					
			
			paddw		mm4,	mm3					/* mm4= sum+x5 */				
			paddw		mm4,	mm4					/* mm4 *= 2 */					
			
			paddw		mm4,	[edi+16]			/* += x1 */						
			psubw		mm4,	[edi+32]			/* -= x2 */						
			
			psubw		mm4,	[edi+128]			/* -= x8 */						
			paddw		mm4,	mm2					/* += p2 */						

			movq        mm5,    LoopFilteredValuesDown/* Read the loopfiltered value of x4 */
			psraw		mm4,	4					/* >>=4 */						

            psubw		mm4,	mm5					/* -=x5 */						
			pand		mm4,	mm0					/* and flag */					

            paddw		mm4,	mm5					/* += x5 */						
			movq		[esi+64], mm4				/* write new x5 */				
			
			/* sum += p2 - x2 */													
			/* Des[w1] = ((sum+x6)<<1 + x2-x3)>>4 */								
			
			movq		mm5,	[edi+96]			/* mm5 = x6 */					
			psubw		mm3,	[edi+32]			/* -= x2 */						
			
			paddw		mm3,	mm2					/* += p2 */						
			movq		mm4,	mm5					/* mm4 = x6 */					
			
			paddw		mm4,	mm3					/* mm4 = sum+x6 */				
			paddw		mm4,	mm4					/* mm4 *= 2*/					
			
			paddw		mm4,	[edi+32]			/* +=x2 */						
			psubw		mm4,	[edi+48]			/* -=x3 */						
			
			psraw		mm4,	4					/* >>=4 */						
			psubw		mm4,	mm5					/* -=x6 */						
			
			pand		mm4,	mm0					/* and flag */					
			paddw		mm4,	mm5					/* += x6 */						
			
			movq		[esi+80], mm4				/* write new x6 */				
			
			/* sum += p2 - x3 */													
			/* Des[w2] = ((sum+x7)<<1 + x3-x4)>>4 */								
			
			movq		mm5,	[edi+112]			/* mm5 = x7 */					
			psubw		mm3,	[edi+48]			/* -= x3 */						
			
			paddw		mm3,	mm2					/* += p2 */						
			movq		mm4,	mm5					/* mm4 = x7 */					
			
			paddw		mm4,	mm3					/* mm4 = sum+x7 */				
			paddw		mm4,	mm4					/* mm4 *= 2*/					
			
			paddw		mm4,	[edi+48]			/* +=x3 */						
			psubw		mm4,	[edi+64]			/* -=x4 */						
			
			psraw		mm4,	4					/* >>=4 */						
			psubw		mm4,	mm5					/* -=x7 */						
			
			pand		mm4,	mm0					/* and flag */					
			paddw		mm4,	mm5					/* += x7 */						
			
			movq		[esi+96], mm4				/* write new x7 */				
			
			/* sum += p2 - x4 */													
			/* Des[w3] = ((sum+x8)<<1 + x4-x5)>>4 */								
			
			movq		mm5,	[edi+128]			/* mm5 = x8 */					
			psubw		mm3,	[edi+64]			/* -= x4 */						
			
			paddw		mm3,	mm2					/* += p2 */						
			movq		mm4,	mm5					/* mm4 = x8 */					
			
			paddw		mm4,	mm3					/* mm4 = sum+x8 */				
			paddw		mm4,	mm4					/* mm4 *= 2*/					
			
			paddw		mm4,	[edi+64]			/* +=x4 */						
			psubw		mm4,	[edi+80]			/* -=x5 */						
			
			psraw		mm4,	4					/* >>=4 */						
			psubw		mm4,	mm5					/* -=x8 */						
			
			pand		mm4,	mm0					/* and flag */					
			paddw		mm4,	mm5					/* += x8 */						
			
			movq		[esi+112], mm4				/* write new x8 */				
			
			/* done with right four column */										
			add			edi,	8					/* shift edi to point x1 */
			sub			esi,	8					/* shift esi back to x1 */

			mov			ebp, Des					/* the destination */							
			lea			ebp, [ebp + edx *4]			/* point to des[-w4] */			
			
			movq		mm0, [esi]													
			packuswb	mm0, [esi + 8]												
			
			movq		[ebp], mm0					/* write des[-w4] */			
			
			movq		mm1, [esi + 16]												
			packuswb	mm1, [esi + 24]												
			
			movq		[ebp+ecx ], mm1				/* write des[-w3] */			
			
			movq		mm2, [esi + 32]												
			packuswb	mm2, [esi + 40]												
			
			movq		[ebp+ecx*2 ], mm2			/* write des[-w2] */			
			
			movq		mm3, [esi + 48]												
			packuswb	mm3, [esi + 56]												
			
			lea			ebp, [ebp+ecx*4]			/* point to des[0] */			
			movq		[ebp+edx], mm3				/* write des[-w1] */			
			
			movq		mm0, [esi + 64]												
			packuswb	mm0, [esi + 72]												
			
			movq		[ebp ], mm0					/* write des[0] */				
			
			movq		mm1, [esi + 80]												
			packuswb	mm1, [esi + 88]												
			
			movq		[ebp+ecx], mm1				/* write des[w1] */				
			
			movq		mm2, [esi + 96]												
			packuswb	mm2, [esi + 104]											
			
			movq		[ebp+ecx*2], mm2			/* write des[w2] */				
			
			movq		mm3, [esi + 112]											
			packuswb	mm3, [esi + 120]											
			
			lea			ebp, [ebp+ecx*2]			/* point to des[w4] */			
			movq		[ebp+ecx], mm3				/* write des[w3] */				


			pop			edi
			pop			esi
			pop			edx
			pop			ecx
			pop			ebp
			pop			eax
			
		} /* end of the macro */
		
		Var1 = Variance11[0]+ Variance11[1]+Variance11[2]+Variance11[3];
		Var1 += Variance12[0]+ Variance12[1]+Variance12[2]+Variance12[3];
		pbi->FragmentVariances[CurrentFrag] += Var1;

		Var2 = Variance21[0]+ Variance21[1]+Variance21[2]+Variance21[3];
		Var2 += Variance22[0]+ Variance22[1]+Variance22[2]+Variance22[3];
		pbi->FragmentVariances[CurrentFrag + FragAcross] += Var2;
		

        if(CurrentFrag==StartFrag)
			CurrentFrag++;
		else
		{
			
			Des=DesPtr-8*PlaneLineStep+8*(CurrentFrag-StartFrag);
			Src=Des;

			QStep = QuantScale[pbi->FragQIndex[CurrentFrag]];		
			for( j=0; j<8;j++)
			{
				Rows[j] = (short) (Src[-5 +j*PlaneLineStep]);
				Rows[72+j] = (short)(Src[4+j*PlaneLineStep]);		
			}

			__asm
			{
			/* Save the registers */
			push		eax
			push		ebp
				/* Calculate the FLimit and store FLimit and QStep */					
				mov			eax,	QStep				/* get QStep */
				movd		mm0,	eax					/* mm0 = 0, 0, 0, Q */

			push		ecx			
				
				punpcklwd	mm0,	mm0					/* mm0 = 0, 0, Q, Q */
				movq		mm1,	FourThrees			/* mm1 = 03 03 03 03 */

			push		edx
				
				punpckldq	mm0,	mm0					/* mm0 = Q, Q, Q, Q */
				movq		QStepMmx,	mm0				/* write the Q step */

			push		esi

                pmullw		mm1,	mm0					/* mm1 = QStep * 3 */							
				pmullw		mm1,	mm0					/* mm1 = QStep * QStep * 3 */	

			push		edi
				
				
				psrlw		mm1,	5					/* mm1 = FLimit */				
				movq		[FLimitMmx], mm1			/* Save FLimit */				

				/* setup the pointers to data */

				mov			eax,	Src					/* eax = Src */
				xor			edx,	edx					/* clear edx */
				
				sub			eax,	4					/* eax = Src-4 */
				lea			esi,	NewRows				/* esi = NewRows */
				lea			edi,	Rows				/* edi = Rows */				

				mov			ecx,	PlaneLineStep		/* ecx = Pitch */				
				sub			edx,	ecx					/* edx = -Pitch */				

				/* Get the data to the intermediate buffer */

				movq		mm0,	[eax]				/* mm0 = 07 06 05 04 03 02 01 00 */
				movq		mm1,	[eax+ecx]			/* mm1 = 17 16 15 14 13 12 11 10 */

				movq		mm2,	[eax+ecx*2]			/* mm2 = 27 26 25 24 23 22 21 20 */
				lea			eax,	[eax+ecx*4]			/* Go down four Rows */	

				movq		mm3,	[eax+edx]			/* mm3 = 37 36 35 34 33 32 31 30 */
				movq		mm4,	mm0					/* mm4 = 07 06 05 04 03 02 01 00 */
			
				punpcklbw	mm0,	mm1					/* mm0 = 13 03 12 02 11 01 10 00 */
				punpckhbw	mm4,	mm1					/* mm4 = 17 07 16 06 15 05 14 04 */

				movq		mm5,	mm2					/* mm5 = 27 26 25 24 23 22 21 20 */
				punpcklbw	mm2,	mm3					/* mm2 = 33 23 32 22 31 21 30 20 */

				punpckhbw	mm5,	mm3					/* mm5 = 37 27 36 26 35 25 34 24 */
				movq		mm1,	mm0					/* mm1 = 13 03 12 02 11 01 10 00 */

				punpcklwd	mm0,	mm2					/* mm0 = 31 21 11 01 30 20 10 00 */
				punpckhwd	mm1,	mm2					/* mm1 = 33 23 13 03 32 22 12 02 */
				
				movq		mm2,	mm4					/* mm2 = 17 07 16 06 15 05 14 04 */
				punpckhwd	mm4,	mm5					/* mm4 = 37 27 17 07 36 26 16 06 */

				punpcklwd	mm2,	mm5					/* mm2 = 35 25 15 05 34 24 14 04 */
				pxor		mm7,	mm7					/* clear mm7 */

				movq		mm5,	mm0					/* make a copy */
				punpcklbw	mm0,	mm7					/* mm0 = 30 20 10 00 */

				movq		[edi+16], mm0				/* write 00 10 20 30 */
				punpckhbw	mm5,	mm7					/* mm5 = 31 21 11 01 */

				movq		mm0,	mm1					/* mm0 =33 23 13 03 32 22 12 02 */
				movq		[edi+32], mm5				/* write 01 11 21 31 */
				
				punpcklbw	mm1,	mm7					/* mm1 = 32 22 12 02 */
				punpckhbw	mm0,	mm7					/* mm0 = 33 23 12 03 */

				movq		[edi+48], mm1				/* write 02 12 22 32 */
				movq		mm3,	mm2					/* mm3 = 35 25 15 05 34 24 14 04 */
				
				movq		mm5,	mm4					/* mm5 = 37 27 17 07 36 26 16 06 */
				movq		[edi+64], mm0				/* write 03 13 23 33 */

				punpcklbw	mm2,	mm7					/* mm2 = 34 24 14 04 */
				punpckhbw	mm3,	mm7					/* mm3 = 35 25 15 05 */

				movq		[edi+80], mm2				/* write 04 14 24 34 */
				punpcklbw	mm4,	mm7					/* mm4 = 36 26 16 06 */

				punpckhbw	mm5,	mm7					/* mm5 = 37 27 17 07 */
				movq		[edi+96], mm3				/* write 05 15 25 35 */
			
				movq		mm0,	[eax]				/* mm0 = 47 46 45 44 43 42 41 40 */
				movq		mm1,	[eax + ecx ]		/* mm1 = 57 56 55 54 53 52 51 50 */

				movq		[edi+112], mm4				/* write 06 16 26 37 */
				movq		mm2,	[eax+ecx*2]			/* mm2 = 67 66 65 64 63 62 61 60 */

				lea			eax,	[eax+ ecx*4]		/* Go down four rows */
				movq		[edi+128], mm5				/* write 07 17 27 37 */

				movq		mm4,	mm0					/* mm4 = 47 46 45 44 43 42 41 40 */
				movq		mm3,	[eax+edx]			/* mm3 = 77 76 75 74 73 72 71 70 */

				punpcklbw	mm0,	mm1					/* mm0 = 53 43 52 42 51 41 50 40 */
				punpckhbw	mm4,	mm1					/* mm4 = 57 57 56 46 55 45 54 44 */

				movq		mm5,	mm2					/* mm5 = 67 66 65 64 63 62 61 60 */
				punpcklbw	mm2,	mm3					/* mm2 = 73 63 72 62 71 61 70 60 */

				punpckhbw	mm5,	mm3					/* mm5 = 77 67 76 66 75 65 74 64 */
				movq		mm1,	mm0					/* mm1 = 53 43 52 42 51 41 50 40 */

				punpcklwd	mm0,	mm2					/* mm0 = 71 61 51 41 70 60 50 40 */
				punpckhwd	mm1,	mm2					/* mm1 = 73 63 53 43 72 62 52 42 */
				
				movq		mm2,	mm4					/* mm2 = 57 57 56 46 55 45 54 44 */
				punpckhwd	mm4,	mm5					/* mm4 = 77 67 57 47 76 66 56 46 */

				punpcklwd	mm2,	mm5					/* mm2 = 75 65 55 45 74 64 54 44 */

				movq		mm5,	mm0					/* make a copy */
				punpcklbw	mm0,	mm7					/* mm0 = 70 60 50 40 */

				movq		[edi+24], mm0				/* write 40 50 60 70 */
				punpckhbw	mm5,	mm7					/* mm5 = 71 61 51 41 */

				movq		mm0,	mm1					/* mm0 = 73 63 53 43 72 62 52 42 */
				movq		[edi+40], mm5				/* write 41 51 61 71 */
				
				punpcklbw	mm1,	mm7					/* mm1 = 72 62 52 42 */
				punpckhbw	mm0,	mm7					/* mm0 = 73 63 53 43 */

				movq		[edi+56], mm1				/* write 42 52 62 72 */
				movq		mm3,	mm2					/* mm3 = 75 65 55 45 74 64 54 44 */
				
				movq		mm5,	mm4					/* mm5 = 77 67 57 47 76 66 56 46 */
				movq		[edi+72], mm0				/* write 43 53 63 73 */

				punpcklbw	mm2,	mm7					/* mm2 = 74 64 54 44 */
				punpckhbw	mm3,	mm7					/* mm3 = 75 65 55 45 */

				movq		[edi+88], mm2				/* write 44 54 64 74 */
				punpcklbw	mm4,	mm7					/* mm4 = 76 66 56 46 */

				punpckhbw	mm5,	mm7					/* mm5 = 77 67 57 47 */
				movq		[edi+104], mm3				/* write 45 55 65 75 */
			
				movq		[edi+120], mm4				/* write 46 56 66 76 */
				movq		[edi+136], mm5				/* write 47 57 67 77 */


				/* Now, compute the variances for Pixel  1-4 and 5-8 */					
				
				/* we use mm0,mm1,mm2 for 1234 and mm4, mm5, mm6 for 5-8 */				
				/* mm7 = 0, mm3 = {128, 128, 128, 128} */								
				
				pcmpeqw		mm3,	mm3					/* mm3 = FFFFFFFFFFFFFFFF */	
				psllw		mm3,	15					/* mm3 = 8000800080008000 */	
				psrlw		mm3,	8					/* mm3 = 0080008000800080 */
				
				movq		mm2,	[edi+16]			/* Pixel 1 */					
				movq		mm6,	[edi+80]			/* Pixel 5 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				movq		mm0,	mm2					/* mm0 = pixel 1 */				
				movq		mm4,	mm6					/* mm4 = pixel 5 */				
				
				pmullw		mm2,	mm2					/* mm2 = pixel1 * pixel1 */		
				pmullw		mm6,	mm6					/* mm6 = pixel5 * pixel5 */		
				
				movq		mm1,	mm2					/* mm1 = pixel1^2 */			
				movq		mm5,	mm6					/* mm5 = pixel5^2 */			
				
				movq		mm2,	[edi+32]			/* Pixel 2 */					
				movq		mm6,	[edi+96]			/* Pixel 6 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 2 */			
				paddw		mm4,	mm6					/* mm4 += pixel 6 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel2^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel6^2 */			
				
				paddw		mm1,	mm2					/* mm1 += pixel2^2 */			
				paddw		mm5,	mm6					/* mm5 += pixel6^2 */			
				
				movq		mm2,	[edi+48]			/* Pixel 3 */					
				movq		mm6,	[edi+112]			/* Pixel 7 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 3 */			
				paddw		mm4,	mm6					/* mm4 += pixel 7 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel3^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel7^2 */			
				
				paddw		mm1,	mm2					/* mm1 += pixel3^2 */			
				paddw		mm5,	mm6					/* mm5 += pixel7^2 */			
				
				movq		mm2,	[edi+64]			/* Pixel 4 */					
				movq		mm6,	[edi+128]			/* Pixel 8 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 4 */			
				paddw		mm4,	mm6					/* mm4 += pixel 8 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel4^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel8^2 */			
				
				paddw		mm1,	mm2					/* mm1 = pixel4^2 */			
				paddw		mm5,	mm6					/* mm5 = pixel8^2 */			
				
				/* mm0 = x1^2 + x2^2 + x3^2 + x4^2 */									
				/* mm1 = x1 + x2 + x3 + x4 */											
				/* mm4 = x5^2 + x6^2 + x7^2 + x8^2 */									
				/* mm5 = x5 + x6 + x7 + x8 */											
				
				movq		mm7,	mm3					/* mm7 = mm3 */					
				psrlw		mm7,	7					/* mm7 = 0001000100010001 */	
				
				movq		mm2,	mm0					/* make copy of sum1 */			
				movq		mm6,	mm4					/* make copy of sum2 */			
				
				paddw		mm0,	mm7					/* (sum1 + 1) */				
				paddw		mm4,	mm7					/* (sum2 + 1) */				
				
				psraw		mm2,	1					/* sum1 /2 */					
				psraw		mm6,	1					/* sum2 /2 */					
				
				psraw		mm0,	1					/* (sum1 + 1)/2 */				
				psraw		mm4,	1					/* (sum2 + 1)/2 */				
				
				pmullw		mm2,	mm0					/* (sum1)/2*(sum1+1)/2 */		
				pmullw		mm6,	mm4					/* (sum2)/2*(sum2+1)/2 */		
				
				psubw		mm1,	mm2					/* Variance 1 */				
				psubw		mm5,	mm6					/* Variance 2 */				
				
				movq		[Variance11], mm1				/* Save Variance1 */
				movq		[Variance21], mm5				/* Save Variance2 */

				movq		mm7,	FLimitMmx			/* mm7 = FLimit */
				movq		mm2,	mm1					/* copy of Variance 1*/

				movq		mm6,	mm5					/* copy of Variance 2*/
				psubw		mm1,	mm7					/* Variance 1 < Flimit? */		
				
				psubw		mm5,	mm7					/* Variance 2 < Flimit? */		
				psraw		mm1,	15					/* FFFF/0000 for true/false */	
				
				psraw		mm5,	15					/* FFFF/0000 for true/false */	
				psraw		mm2,	15					/* Variance 1 > 32768 ? */

				psraw		mm6,	15					/* Variance 2 > 32768 ? */
				movq		mm7,	[edi+64]			/* mm0 = Pixel 4			*/	

				pandn		mm2,	mm1					/* Variance 1 < Flimit &&
														   Variance 1 < 32768		*/
				pandn		mm6,	mm5					/* Variance 2 < Flimit &&
														   Variance 2 < 32768		*/
				movq		mm4,	[edi+80]			/* mm4 = Pixel 5			*/				
				pand		mm6,	mm2					/* mm1 = Variance1 < Flimit */	
														/*     &&Variance2 < Flimit */	
				movq		mm2,	mm7					/* make copy of Pixel4		*/	

				psubusw		mm7,	mm4					/* 4 - 5 */						
				psubusw		mm4,	mm2					/* 5 - 4 */						
				
				por			mm7,	mm4					/* abs(4 - 5) */				
				psubw		mm7,	QStepMmx			/* abs(4-5)<QStepMmx ? */		
				
				psraw		mm7,	15					/* FFFF/0000 for True/Flase */
				pand		mm7,	mm6													
				
				/* mm7 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* now lets look at the right four colomn */							
				
				add			edi,	8					/* offset 8 to right 4 cols */	
				
				movq		mm2,	[edi+16]			/* Pixel 1 */					
				movq		mm6,	[edi+80]			/* Pixel 5 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				movq		mm0,	mm2					/* mm0 = pixel 1 */				
				movq		mm4,	mm6					/* mm4 = pixel 5 */				
				
				pmullw		mm2,	mm2					/* mm2 = pixel1 * pixel1 */		
				pmullw		mm6,	mm6					/* mm6 = pixel5 * pixel5 */		
				
				movq		mm1,	mm2					/* mm1 = pixel1^2 */			
				movq		mm5,	mm6					/* mm5 = pixel5^2 */			
				
				movq		mm2,	[edi+32]			/* Pixel 2 */					
				movq		mm6,	[edi+96]			/* Pixel 6 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 2 */			
				paddw		mm4,	mm6					/* mm4 += pixel 6 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel2^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel6^2 */			
				
				paddw		mm1,	mm2					/* mm1 += pixel2^2 */			
				paddw		mm5,	mm6					/* mm5 += pixel6^2 */			
				
				movq		mm2,	[edi+48]			/* Pixel 3 */					
				movq		mm6,	[edi+112]			/* Pixel 7 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 3 */			
				paddw		mm4,	mm6					/* mm4 += pixel 7 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel3^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel7^2 */			
				
				paddw		mm1,	mm2					/* mm1 += pixel3^2 */			
				paddw		mm5,	mm6					/* mm5 += pixel7^2 */			
				
				movq		mm2,	[edi+64]			/* Pixel 4 */					
				movq		mm6,	[edi+128]			/* Pixel 8 */					
				
				psubw		mm2,	mm3					/* mm2 -=128 */					
				psubw		mm6,	mm3					/* mm6 -=128 */					
				
				paddw		mm0,	mm2					/* mm0 += pixel 4 */			
				paddw		mm4,	mm6					/* mm4 += pixel 8 */			
				
				pmullw		mm2,	mm2					/* mm2 = pixel4^2 */			
				pmullw		mm6,	mm6					/* mm6 = pixel8^2 */			
				
				paddw		mm1,	mm2					/* mm1 = pixel4^2 */			
				paddw		mm5,	mm6					/* mm5 = pixel8^2 */			
				
				/* mm0 = x1^2 + x2^2 + x3^2 + x4^2 */									
				/* mm1 = x1 + x2 + x3 + x4 */											
				/* mm4 = x5^2 + x6^2 + x7^2 + x8^2 */									
				/* mm5 = x5 + x6 + x7 + x8 */											
				
				psrlw		mm3,	7					/* mm3 = 0001000100010001 */	
				
				movq		mm2,	mm0					/* make copy of sum1 */			
				movq		mm6,	mm4					/* make copy of sum2 */			
				
				paddw		mm0,	mm3					/* (sum1 + 1) */				
				paddw		mm4,	mm3					/* (sum2 + 1) */				
				
				psraw		mm2,	1					/* sum1 /2 */					
				psraw		mm6,	1					/* sum2 /2 */					
				
				psraw		mm0,	1					/* (sum1 + 1)/2 */				
				psraw		mm4,	1					/* (sum2 + 1)/2 */				
				
				pmullw		mm2,	mm0					/* (sum1)/2*(sum1+1)/2 */		
				pmullw		mm6,	mm4					/* (sum2)/2*(sum2+1)/2 */		
				
				psubw		mm1,	mm2					/* Variance 1 */				
				psubw		mm5,	mm6					/* Variance 2 */				
				
				movq		[Variance12], mm1				/* Save Variance1 */
				movq		[Variance22], mm5				/* Save Variance2 */
	
				movq		mm3,	FLimitMmx			/* mm3 = FLimit */				
				movq		mm2,	mm1					/* copy of Varinace 1*/

				movq		mm6,	mm5					/* Variance 2 */
				psubw		mm1,	mm3					/* Variance 1 < Flimit? */		
				
				psubw		mm5,	mm3					/* Variance 2 < Flimit? */		
				psraw		mm6,	15					/* Variance 1 > 32768 */
				
				psraw		mm2,	15					/* Variance 2 > 32768 */
				psraw		mm1,	15					/* FFFF/0000 for true/false */	
				
				psraw		mm5,	15					/* FFFF/0000 for true/false */	
				movq		mm0,	[edi+64]			/* mm0 = Pixel 4			*/	

				pandn		mm2,	mm1					/* Variance1<32678 && 
														   Variance1<Limit			*/
				pandn		mm6,	mm5					/* Variance2<32678 && 
														   Variance1<Limit			*/
				
				movq		mm4,	[edi+80]			/* mm4 = Pixel 5			*/	
				pand		mm6,	mm2					/* mm1 = Variance1 < Flimit */	
														/*     &&Variance2 < Flimit */	
				movq		mm2,	mm0					/* make copy of Pixel4		*/	
														
				psubusw		mm0,	mm4					/* 4 - 5 */						
				psubusw		mm4,	mm2					/* 5 - 4 */						
				
				por			mm0,	mm4					/* abs(4 - 5) */				
				psubw		mm0,	QStepMmx			/* abs(4-5)<QStepMmx ? */		
				
				psraw		mm0,	15					/* FFFF/0000 for True/False */
				pand		mm0,	mm6				
				
				sub			edi,	8					/* offset edi back */			
				
				/* mm0 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* mm0 and mm7 now are in use  */										
                /* find the loop filtered values for the pixels on block boundary */
                movq        mm1,    LoopFLimitMmx;      /* Get the Flimit values for loop filter */
                movq        mm3,    [edi + 48]          /* mm3 = x3 = p[-2] */

                movq        mm4,    [edi + 64]          /* mm4 = x4 = p[-1] */
                movq        mm5,    [edi + 80]          /* mm5 = x5 = p[ 0] */

                movq        mm6,    [edi + 96]          /* mm6 = x6 = p[ 1] */
                psubw       mm5,    mm4                 /* mm5 = p[ 0] - p[-1] */

                psubw       mm3,    mm6                 /* mm3 = p[-2] - p[ 1] */
                movq        mm4,    mm5                 /* make a copy */

                paddw       mm4,    mm5                 /* 2 * ( p[0] - p[-1] ) */
                paddw       mm3,    FourFours           /* mm3 + 4 */

                paddw       mm5,    mm4                 /* 3 * ( p[0] - p[-1] ) */
                paddw       mm3,    mm5                 /* Filtval before shift */

                psraw       mm3,    3                   /* FiltVal */
                movq        mm2,    mm3                 /* make a copy */

                psraw       mm3,    15                  /* FFFF->Neg, 0000->Pos */
                pxor        mm2,    mm3

                psubsw      mm2,    mm3                 /* mm2 = abs(FiltVal) */
                por         mm3,    FourOnes            /* -1 and 1 for + and - */

                movq        mm4,    mm1                 /* make a copy of Flimit */
                psubw       mm1,    mm2                 /* mm1= Flimit - abs(FiltVal) */

                movq        mm5,    mm1                 /* copy Flimit - abs(FiltVal) */
                psraw       mm1,    15                  /* FFFF or 0000 */

                pxor        mm5,    mm1                 
                psubsw      mm5,    mm1                 /* abs(Flimit - abs(FiltVal)) */

                psubusw     mm4,    mm5                 /* Flimit-abs(Flimit - abs(FiltVal)) */
                pmullw      mm4,    mm3                 /* get the sign back */

                movq        mm1,    [edi+64]            /* p[-1] */
                movq        mm2,    [edi+80]            /* p[0] */
                
                paddw       mm1,    mm4                 /* p[-1] + NewFiltVal */
                psubw       mm2,    mm4                 /* p[0] - NewFiltVal */

                pxor        mm6,    mm6                 /* clear mm6 */
                
                packuswb    mm1,    mm1                 /* clamping */
                packuswb    mm2,    mm2                 /* clamping */

                punpcklbw   mm1,    mm6                 /* unpack to word */
                movq        LoopFilteredValuesUp, mm1   /* save the values */

                punpcklbw   mm2,    mm6                 /* unpack to word */
                movq        LoopFilteredValuesDown, mm2 /* save the values */

                /* Let's do the filtering now */										
				/* p1 = (abs(Src[-4] - Src[-5]) < QStep ) ?  Src[-5] : Src[-4]; */		
				/* p2 = (abs(Src[+3] - Src[+4]) < QStep ) ?  Src[+4] : Src[+3]; */		
				
				movq		mm5,	[edi]				/* mm5 = -5 */					
				movq		mm4,	[edi + 16]			/* mm4 = -4 */					
				
				movq		mm3,	mm4					/* copy of -4 */				
				movq		mm6,	mm5					/* copy of -5 */				
				
				psubusw		mm4,	mm6					/* mm4 = [-4] - [-5] */			
				psubusw		mm5,	mm3					/* mm5 = [-5] - [-4] */			
				
				por			mm4,	mm5					/* abs([-4]-[-5] ) */			
				psubw		mm4,	QStepMmx			/* abs([-4]-[-5] )<QStep? */	
				
				psraw		mm4,	15					/* FFFF/0000 for True/False */	
				movq		mm1,	mm4					/* copy of the mm4 */			
				
				pand		mm4,	mm6					/*							*/	
				pandn		mm1,	mm3					/*							*/	
				
				por			mm1,	mm4					/* mm1 = p1					*/	
				
				/* now find P2 */														
				
				movq		mm4,	[edi+128]			/* mm4 = [3] */					
				movq		mm5,	[edi+144]			/* mm5 = [4] */					
				
				movq		mm3,	mm4					/* copy of 3 */					
				movq		mm6,	mm5					/* copy of 4 */					
				
				psubusw		mm4,	mm6					/* mm4 = [3] - [4] */			
				psubusw		mm5,	mm3					/* mm5 = [4] - [3] */			
				
				por			mm4,	mm5					/* abs([3]-[4] ) */				
				psubw		mm4,	QStepMmx			/* abs([3]-[4] )<QStep? */		
				
				psraw		mm4,	15					/* FFFF/0000 for True/False */	
				movq		mm2,	mm4					/* copy of the mm4 */			
				
				pand		mm4,	mm6					/*							*/	
				pandn		mm2,	mm3					/*							*/	
				
				por			mm2,	mm4					/* mm2 = p2					*/	
				
				/* sum = p1 + p1 + p1 + x1 + x2 + x3 + x4 + 4; */				
				/* Des[-w4] = (((sum + x1) << 1) - (x4 - x5)) >> 4; */			
				/* Des[-w4] = Src[-w4]; */												
				/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	
				
				movq		mm3,	mm1					/* mm3 = p1 */					
				paddw		mm3,	mm3					/* mm3 = p1 + p1 */				
				
				paddw		mm3,	mm1					/* mm3 = p1 + p1 + p1 */		
				movq		mm4,	[edi+16]			/* mm4 = x1 */					
				
				paddw		mm3,	[edi+32]			/* mm3 = p1+p1+p1+ x2 */		
				paddw		mm4,	[edi+48]			/* mm4 = x1+x3 */				
				
				paddw		mm3,	[edi+64]			/* mm3 += x4 */					
				paddw		mm4,	FourFours			/* mm4 = x1 + x3 + 4 */			
				
				paddw		mm3,	mm4					/* mm3 = 3*p1+x1+x2+x3+x4+4 */	
				movq		mm4,	mm3					/* mm4 = mm3 */					
				
				movq		mm5,	[edi+16]			/* mm5 = x1 */					
				paddw		mm4,	mm5					/* mm4 = sum+x1 */				
				
				psllw		mm4,	1					/* mm4 = (sum+x1)<<1 */			
				psubw		mm4,	[edi+64]			/* mm4 = (sum+x1)<<1-x4 */		
				
				paddw		mm4,	[edi+80]			/* mm4 = (sum+x1)<<1-x4+x5 */	
				psraw		mm4,	4					/* mm4 >>=4 */					
				
				psubw		mm4,	mm5					/* New Value - old Value */		
				pand		mm4,	mm7					/* And the flag */				
				
				paddw		mm4,	mm5					/* add the old value back */	
				movq		[esi],	mm4					/* Write new x1 */				
				
				/* sum += x5 -p1 */														
				/* Des[-w3]=((sum+x2)<<1-x5+x6)>>4 */									
				
				movq		mm5,	[edi+32]			/* mm5= x2 */					
				psubw		mm3,	mm1					/* sum=sum-p1 */				
				
				paddw		mm3,    [edi+80]			/* sum=sum+x5 */				
				movq		mm4,	mm5					/* copy sum */					
				
				paddw		mm4,	mm3					/* mm4=sum+x2 */				
				paddw		mm4,	mm4					/* mm4 <<= 1 */					
				
				psubw		mm4,	[edi+80]			/* mm4 =(sum+x2)<<1-x5 */		
				paddw		mm4,	[edi+96]			/* mm4 =(sum+x2)<<1-x5+x6 */	
				
				psraw		mm4,	4					/* mm4=((sum+x2)<<1-x5+x6)>>4 */
				psubw		mm4,	mm5					/* new value - old value	*/	
				
				pand		mm4,	mm7					/* And the flag */				
				paddw		mm4,	mm5					/* add the old value back */	
				
				movq		[esi+16], mm4				/* write new x2 */				
				
				/* sum += x6 - p1 */													
				/* Des[-w2]=((sum+x[3])<<1-x[6]+x[7])>>4 */								
				
				movq		mm5,	[edi+48]			/* mm5= x3 */					
				psubw		mm3,	mm1					/* sum=sum-p1 */				
				
				paddw		mm3,    [edi+96]			/* sum=sum+x6 */				
				movq		mm4,	mm5					/* copy x3 */					
				
				paddw		mm4,	mm3					/* mm4=sum+x3 */				
				paddw		mm4,	mm4					/* mm4 <<= 1 */					
				
				psubw		mm4,	[edi+96]			/* mm4 =(sum+x3)<<1-x6 */		
				paddw		mm4,	[edi+112]			/* mm4 =(sum+x3)<<1-x6+x7 */	
				
				psraw		mm4,	4					/* mm4=((sum+x3)<<1-x6+x7)>>4 */
				psubw		mm4,	mm5					/* new value - old value	*/	
				
				pand		mm4,	mm7					/* And the flag */				
				paddw		mm4,	mm5					/* add the old value back */	
				
				movq		[esi+32], mm4				/* write new x3 */				
				
				/* sum += x7 - p1 */													
				/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
				
				movq		mm5,	[edi+64]			/* mm5 = x4 */					
				psubw		mm3,	mm1					/* sum = sum-p1 */				
				
				paddw		mm3,	[edi+112]			/* sum = sum+x7 */				
				movq		mm4,	mm5					/* mm4 = x4 */					
				
				paddw		mm4,	mm3					/* mm4 = sum + x4 */			
				paddw		mm4,	mm4					/* mm4 *=2 */					
				
				paddw		mm4,	mm1					/* += p1 */						
				psubw		mm4,	[edi+16]			/* -= x1 */						
				
				psubw		mm4,	[edi+112]			/* -= x7 */						
				paddw		mm4,	[edi+128]			/* += x8 */						

                movq        mm5,    LoopFilteredValuesUp/* Read the loopfiltered value of x4 */
				psraw		mm4,	4					/* >>=4 */						

				psubw		mm4,	mm5					/* -=x4 */						
				pand		mm4,	mm7					/* and flag */					

				paddw		mm4,	mm5					/* += x4 */						
				movq		[esi+48], mm4				/* write new x4 */				
				
				/* sum+= x8-x1 */														
				/* Des[0]=((sum+x5)<<1+x1-x2-x8+p2)>>4 */								
				
				movq		mm5,	[edi+80]			/* mm5 = x5 */					
				psubw		mm3,	[edi+16]			/* sum -= x1 */					
				
				paddw		mm3,	[edi+128]			/* sub += x8 */					
				movq		mm4,	mm5					/* mm4 = x5 */					
				
				paddw		mm4,	mm3					/* mm4= sum+x5 */				
				paddw		mm4,	mm4					/* mm4 *= 2 */					
				
				paddw		mm4,	[edi+16]			/* += x1 */						
				psubw		mm4,	[edi+32]			/* -= x2 */						
				
				psubw		mm4,	[edi+128]			/* -= x8 */						
				paddw		mm4,	mm2					/* += p2 */						

                movq        mm5,    LoopFilteredValuesDown/* Read the loopfiltered value of x4 */
				psraw		mm4,	4					/* >>=4 */						

                psubw		mm4,	mm5					/* -=x5 */						
				pand		mm4,	mm7					/* and flag */					

                paddw		mm4,	mm5					/* += x5 */						
				movq		[esi+64], mm4				/* write new x5 */				
				
				/* sum += p2 - x2 */													
				/* Des[w1] = ((sum+x6)<<1 + x2-x3)>>4 */								
				
				movq		mm5,	[edi+96]			/* mm5 = x6 */					
				psubw		mm3,	[edi+32]			/* -= x2 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x6 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x6 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+32]			/* +=x2 */						
				psubw		mm4,	[edi+48]			/* -=x3 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x6 */						
				
				pand		mm4,	mm7					/* and flag */					
				paddw		mm4,	mm5					/* += x6 */						
				
				movq		[esi+80], mm4				/* write new x6 */				
				
				/* sum += p2 - x3 */													
				/* Des[w2] = ((sum+x7)<<1 + x3-x4)>>4 */								
				
				movq		mm5,	[edi+112]			/* mm5 = x7 */					
				psubw		mm3,	[edi+48]			/* -= x3 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x7 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x7 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+48]			/* +=x3 */						
				psubw		mm4,	[edi+64]			/* -=x4 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x7 */						
				
				pand		mm4,	mm7					/* and flag */					
				paddw		mm4,	mm5					/* += x7 */						
				
				movq		[esi+96], mm4				/* write new x7 */				
				
				/* sum += p2 - x4 */													
				/* Des[w3] = ((sum+x8)<<1 + x4-x5)>>4 */								
				
				movq		mm5,	[edi+128]			/* mm5 = x8 */					
				psubw		mm3,	[edi+64]			/* -= x4 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x8 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x8 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+64]			/* +=x4 */						
				psubw		mm4,	[edi+80]			/* -=x5 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x8 */						
				
				pand		mm4,	mm7					/* and flag */					
				paddw		mm4,	mm5					/* += x8 */						
				
				movq		[esi+112], mm4				/* write new x8 */				
				
				/* done with left four columns */										
				/* now do the righ four columns */										
				
				add			edi,	8					/* shift to right four column */
				add			esi,	8					/* shift to right four column */
				
				/* mm0 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* mm0 now are in use  */										
                /* find the loop filtered values for the pixels on block boundary */
                movq        mm1,    LoopFLimitMmx;      /* Get the Flimit values for loop filter */
                movq        mm3,    [edi + 48]          /* mm3 = x3 = p[-2] */

                movq        mm4,    [edi + 64]          /* mm4 = x4 = p[-1] */
                movq        mm5,    [edi + 80]          /* mm5 = x5 = p[ 0] */

                movq        mm6,    [edi + 96]          /* mm6 = x6 = p[ 1] */
                psubw       mm5,    mm4                 /* mm5 = p[ 0] - p[-1] */

                psubw       mm3,    mm6                 /* mm3 = p[-2] - p[ 1] */
                movq        mm4,    mm5                 /* make a copy */

                paddw       mm4,    mm5                 /* 2 * ( p[0] - p[-1] ) */
                paddw       mm3,    FourFours           /* mm3 + 4 */

                paddw       mm5,    mm4                 /* 3 * ( p[0] - p[-1] ) */
                paddw       mm3,    mm5                 /* Filtval before shift */

                psraw       mm3,    3                   /* FiltVal */
                movq        mm2,    mm3                 /* make a copy */

                psraw       mm3,    15                  /* FFFF->Neg, 0000->Pos */
                pxor        mm2,    mm3

                psubsw      mm2,    mm3                 /* mm2 = abs(FiltVal) */
                por         mm3,    FourOnes            /* -1 and 1 for + and - */

                movq        mm4,    mm1                 /* make a copy of Flimit */
                psubw       mm1,    mm2                 /* mm1= Flimit - abs(FiltVal) */

                movq        mm5,    mm1                 /* copy Flimit - abs(FiltVal) */
                psraw       mm1,    15                  /* FFFF or 0000 */

                pxor        mm5,    mm1                 
                psubsw      mm5,    mm1                 /* abs(Flimit - abs(FiltVal)) */

                psubusw     mm4,    mm5                 /* Flimit-abs(Flimit - abs(FiltVal)) */
                pmullw      mm4,    mm3                 /* get the sign back */

                movq        mm1,    [edi+64]            /* p[-1] */
                movq        mm2,    [edi+80]            /* p[0] */
                
                paddw       mm1,    mm4                 /* p[-1] + NewFiltVal */
                psubw       mm2,    mm4                 /* p[0] - NewFiltVal */

                pxor        mm6,    mm6                 /* clear mm6 */
                
                packuswb    mm1,    mm1                 /* clamping */
                packuswb    mm2,    mm2                 /* clamping */

                punpcklbw   mm1,    mm6                 /* unpack to word */
                movq        LoopFilteredValuesUp, mm1   /* save the values */

                punpcklbw   mm2,    mm6                 /* unpack to word */
                movq        LoopFilteredValuesDown, mm2 /* save the values */

                /* Let's do the filtering now */										
				/* p1 = (abs(Src[-4] - Src[-5]) < QStep ) ?  Src[-5] : Src[-4]; */		
				/* p2 = (abs(Src[+3] - Src[+4]) < QStep ) ?  Src[+4] : Src[+3]; */		
				
				movq		mm5,	[edi]				/* mm5 = -5 */					
				movq		mm4,	[edi + 16]			/* mm4 = -4 */					
				
				movq		mm3,	mm4					/* copy of -4 */				
				movq		mm6,	mm5					/* copy of -5 */				
				
				psubusw		mm4,	mm6					/* mm4 = [-4] - [-5] */			
				psubusw		mm5,	mm3					/* mm5 = [-5] - [-4] */			
				
				por			mm4,	mm5					/* abs([-4]-[-5] ) */			
				psubw		mm4,	QStepMmx			/* abs([-4]-[-5] )<QStep? */	
				
				psraw		mm4,	15					/* FFFF/0000 for True/False */	
				movq		mm1,	mm4					/* copy of the mm4 */			
				
				pand		mm4,	mm6					/*							*/	
				pandn		mm1,	mm3					/*							*/	
				
				por			mm1,	mm4					/* mm1 = p1					*/	
				
				/* now find P2 */														
				
				movq		mm4, [edi+128]				/* mm4 = [3] */					
				movq		mm5, [edi+144]				/* mm5 = [4] */					
				
				movq		mm3, mm4					/* copy of 3 */					
				movq		mm6, mm5					/* copy of 4 */					
				
				psubusw		mm4, mm6					/* mm4 = [3] - [4] */			
				psubusw		mm5, mm3					/* mm5 = [4] - [3] */			
				
				por			mm4, mm5					/* abs([3]-[4] ) */				
				psubw		mm4, QStepMmx				/* abs([3]-[4] )<QStep? */		
				
				psraw		mm4, 15						/* FFFF/0000 for True/False */	
				movq		mm2, mm4					/* copy of the mm4 */			
				
				pand		mm4, mm6					/*							*/	
				pandn		mm2, mm3					/*							*/	
				
				por			mm2, mm4					/* mm2 = p2					*/	
				
				/* psum = p1 + p1 + p1 + v[1] + v[2] + v[3] + v[4] + 4; */				
				/* Des[-w4] = (((psum + v[1]) << 1) - (v[4] - v[5])) >> 4; */			
				/* Des[-w4]=Src[-w4]; */												
				/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	
				
				movq		mm3,	mm1					/* mm3 = p1 */					
				paddw		mm3,	mm3					/* mm3 = p1 + p1 */				
				
				paddw		mm3,	mm1					/* mm3 = p1 + p1 + p1 */		
				movq		mm4,	[edi+16]			/* mm4 = x1 */					
				
				paddw		mm3,	[edi+32]			/* mm3 = p1+p1+p1+ x2 */		
				paddw		mm4,	[edi+48]			/* mm4 = x1+x3 */				
				
				paddw		mm3,	[edi+64]			/* mm3 += x4 */					
				paddw		mm4,	FourFours			/* mm4 = x1 + x3 + 4 */			
				
				paddw		mm3,	mm4					/* mm3 = 3*p1+x1+x2+x3+x4+4 */	
				movq		mm4,	mm3					/* mm4 = mm3 */					
				
				movq		mm5,	[edi+16]			/* mm5 = x1 */					
				paddw		mm4,	mm5					/* mm4 = sum+x1 */				
				
				psllw		mm4,	1					/* mm4 = (sum+x1)<<1 */			
				psubw		mm4,	[edi+64]			/* mm4 = (sum+x1)<<1-x4 */		
				
				paddw		mm4,	[edi+80]			/* mm4 = (sum+x1)<<1-x4+x5 */	
				psraw		mm4,	4					/* mm4 >>=4 */					
				
				psubw		mm4,	mm5					/* New Value - old Value */		
				pand		mm4,	mm0					/* And the flag */				
				
				paddw		mm4,	mm5					/* add the old value back */	
				movq		[esi],	mm4					/* Write new x1 */				
				
				/* sum += x5 -p1 */														
				/* Des[-w3]=((sum+x2)<<1-x5+x6)>>4 */									
				
				movq		mm5,	[edi+32]			/* mm5= x2 */					
				psubw		mm3,	mm1					/* sum=sum-p1 */				
				
				paddw		mm3,    [edi+80]			/* sum=sum+x5 */				
				movq		mm4,	mm5					/* copy sum */					
				
				paddw		mm4,	mm3					/* mm4=sum+x2 */				
				paddw		mm4,	mm4					/* mm4 <<= 1 */					
				
				psubw		mm4,	[edi+80]			/* mm4 =(sum+x2)<<1-x5 */		
				paddw		mm4,	[edi+96]			/* mm4 =(sum+x2)<<1-x5+x6 */	
				
				psraw		mm4,	4					/* mm4=((sum+x2)<<1-x5+x6)>>4 */
				psubw		mm4,	mm5					/* new value - old value	*/	
				
				pand		mm4,	mm0					/* And the flag */				
				paddw		mm4,	mm5					/* add the old value back */	
				
				movq		[esi+16], mm4				/* write new x2 */				
				
				/* sum += x6 - p1 */													
				/* Des[-w2]=((sum+x[3])<<1-x[6]+x[7])>>4 */								
				
				movq		mm5,	[edi+48]			/* mm5= x3 */					
				psubw		mm3,	mm1					/* sum=sum-p1 */				
				
				paddw		mm3,    [edi+96]			/* sum=sum+x6 */				
				movq		mm4,	mm5					/* copy x3 */					
				
				paddw		mm4,	mm3					/* mm4=sum+x3 */				
				paddw		mm4,	mm4					/* mm4 <<= 1 */					
				
				psubw		mm4,	[edi+96]			/* mm4 =(sum+x3)<<1-x6 */		
				paddw		mm4,	[edi+112]			/* mm4 =(sum+x3)<<1-x6+x7 */	
				
				psraw		mm4,	4					/* mm4=((sum+x3)<<1-x6+x7)>>4 */
				psubw		mm4,	mm5					/* new value - old value	*/	
				
				pand		mm4,	mm0					/* And the flag */				
				paddw		mm4,	mm5					/* add the old value back */	
				
				movq		[esi+32], mm4				/* write new x3 */				
				
				/* sum += x7 - p1 */													
				/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
				
				movq		mm5,	[edi+64]			/* mm5 = x4 */					
				psubw		mm3,	mm1					/* sum = sum-p1 */				
				
				paddw		mm3,	[edi+112]			/* sum = sum+x7 */				
				movq		mm4,	mm5					/* mm4 = x4 */					
				
				paddw		mm4,	mm3					/* mm4 = sum + x4 */			
				paddw		mm4,	mm4					/* mm4 *=2 */					
				
				paddw		mm4,	mm1					/* += p1 */						
				psubw		mm4,	[edi+16]			/* -= x1 */						
				
				psubw		mm4,	[edi+112]			/* -= x7 */				
				paddw		mm4,	[edi+128]			/* += x8 */						

                movq        mm5,    LoopFilteredValuesUp/* Read the loopfiltered value of x4 */
				psraw		mm4,	4					/* >>=4 */						

                psubw		mm4,	mm5					/* -=x4 */						
				pand		mm4,	mm0					/* and flag */					

                paddw		mm4,	mm5					/* += x4 */						
				movq		[esi+48], mm4				/* write new x4 */				
				
				/* sum+= x8-x1 */														
				/* Des[0]=((sum+x5)<<1+x1-x2-x8+p2)>>4 */								
				
				movq		mm5,	[edi+80]			/* mm5 = x5 */					
				psubw		mm3,	[edi+16]			/* sum -= x1 */					
				
				paddw		mm3,	[edi+128]			/* sub += x8 */					
				movq		mm4,	mm5					/* mm4 = x5 */					
				
				paddw		mm4,	mm3					/* mm4= sum+x5 */				
				paddw		mm4,	mm4					/* mm4 *= 2 */					
				
				paddw		mm4,	[edi+16]			/* += x1 */						
				psubw		mm4,	[edi+32]			/* -= x2 */						
				
				psubw		mm4,	[edi+128]			/* -= x8 */						
				paddw		mm4,	mm2					/* += p2 */						

                movq        mm5,    LoopFilteredValuesDown/* Read the loopfiltered value of x5 */
				psraw		mm4,	4					/* >>=4 */						

                psubw		mm4,	mm5					/* -=x5 */						
				pand		mm4,	mm0					/* and flag */					

                paddw		mm4,	mm5					/* += x5 */						
				movq		[esi+64], mm4				/* write new x5 */				
				
				/* sum += p2 - x2 */													
				/* Des[w1] = ((sum+x6)<<1 + x2-x3)>>4 */								
				
				movq		mm5,	[edi+96]			/* mm5 = x6 */					
				psubw		mm3,	[edi+32]			/* -= x2 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x6 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x6 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+32]			/* +=x2 */						
				psubw		mm4,	[edi+48]			/* -=x3 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x6 */						
				
				pand		mm4,	mm0					/* and flag */					
				paddw		mm4,	mm5					/* += x6 */						
				
				movq		[esi+80], mm4				/* write new x6 */				
				
				/* sum += p2 - x3 */													
				/* Des[w2] = ((sum+x7)<<1 + x3-x4)>>4 */								
				
				movq		mm5,	[edi+112]			/* mm5 = x7 */					
				psubw		mm3,	[edi+48]			/* -= x3 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x7 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x7 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+48]			/* +=x3 */						
				psubw		mm4,	[edi+64]			/* -=x4 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x7 */						
				
				pand		mm4,	mm0					/* and flag */					
				paddw		mm4,	mm5					/* += x7 */						
				
				movq		[esi+96], mm4				/* write new x7 */				
				
				/* sum += p2 - x4 */													
				/* Des[w3] = ((sum+x8)<<1 + x4-x5)>>4 */								
				
				movq		mm5,	[edi+128]			/* mm5 = x8 */					
				psubw		mm3,	[edi+64]			/* -= x4 */						
				
				paddw		mm3,	mm2					/* += p2 */						
				movq		mm4,	mm5					/* mm4 = x8 */					
				
				paddw		mm4,	mm3					/* mm4 = sum+x8 */				
				paddw		mm4,	mm4					/* mm4 *= 2*/					
				
				paddw		mm4,	[edi+64]			/* +=x4 */						
				psubw		mm4,	[edi+80]			/* -=x5 */						
				
				psraw		mm4,	4					/* >>=4 */						
				psubw		mm4,	mm5					/* -=x8 */						
				
				pand		mm4,	mm0					/* and flag */					
				paddw		mm4,	mm5					/* += x8 */						
				
				movq		[esi+112], mm4				/* write new x8 */				
				
				/* done with right four column */	
				/* transpose */
				mov			eax,	Des					/* the destination */			
				add			edi,	8					/* shift edi to point x1 */

				sub			esi,	8					/* shift esi back to left x1 */
				sub			eax,	4

				movq		mm0,	[esi]				/* mm0 = 30 20 10 00 */
				movq		mm1,	[esi+16]			/* mm1 = 31 21 11 01 */

				movq		mm4,	mm0					/* mm4 = 30 20 10 00 */
				punpcklwd	mm0,	mm1					/* mm0 = 11 10 01 00 */

				punpckhwd	mm4,	mm1					/* mm4 = 31 30 21 20 */
				movq		mm2,	[esi+32]			/* mm2 = 32 22 12 02 */

				movq		mm3,	[esi+48]			/* mm3 = 33 23 13 03 */
				movq		mm5,	mm2					/* mm5 = 32 22 12 02 */

				punpcklwd	mm2,	mm3					/* mm2 = 13 12 03 02 */
				punpckhwd	mm5,	mm3					/* mm5 = 33 32 23 22 */

				movq		mm1,	mm0					/* mm1 = 11 10 01 00 */
				punpckldq	mm0,	mm2					/* mm0 = 03 02 01 00 */

				movq		[edi],	mm0					/* write 00 01 02 03 */
				punpckhdq	mm1,	mm2					/* mm1 = 13 12 11 10 */
				
				movq		mm0,	mm4					/* mm0 = 31 30 21 20 */
				movq		[edi+16], mm1				/* write 10 11 12 13 */

				punpckldq	mm0,	mm5					/* mm0 = 23 22 21 20 */
				punpckhdq	mm4,	mm5					/* mm4 = 33 32 31 30 */

				movq		mm1,	[esi+64]			/* mm1 = 34 24 14 04 */
				movq		mm2,	[esi+80]			/* mm2 = 35 25 15 05 */				

				movq		mm5,	[esi+96]			/* mm5 = 36 26 16 06 */
				movq		mm6,	[esi+112]			/* mm6 = 37 27 17 07 */
								
				movq		mm3,	mm1					/* mm3 = 34 24 14 04 */
				movq		mm7,	mm5					/* mm7 = 36 26 16 06 */

				punpcklwd	mm1,	mm2					/* mm1 = 15 14 05 04 */
				punpckhwd	mm3,	mm2					/* mm3 = 35 34 25 24 */

				punpcklwd	mm5,	mm6					/* mm5 = 17 16 07 06 */
				punpckhwd	mm7,	mm6					/* mm7 = 37 36 27 26 */

				movq		mm2,	mm1					/* mm2 = 15 14 05 04 */
				movq		mm6,	mm3					/* mm6 = 35 34 25 24 */

				punpckldq	mm1,	mm5					/* mm1 = 07 06 05 04 */
				punpckhdq	mm2,	mm5					/* mm2 = 17 16 15 14 */

				punpckldq	mm3,	mm7					/* mm3 = 27 26 25 24 */
				punpckhdq	mm6,	mm7					/* mm6 = 37 36 35 34 */
			
				movq		mm5,	[edi]				/* mm5 = 03 02 01 00 */
				packuswb	mm5,	mm1					/* mm5 = 07 06 05 04 03 02 01 00 */
				
				movq		[eax],	mm5					/* write 00 01 02 03 04 05 06 07 */
				movq		mm7,	[edi+16]			/* mm7 = 13 12 11 10 */

				packuswb	mm7,	mm2					/* mm7 = 17 16 15 14 13 12 11 10 */
				movq		[eax+ecx], mm7				/* write 10 11 12 13 14 15 16 17 */

				packuswb	mm0,	mm3					/* mm0 = 27 26 25 24 23 22 21 20 */
				packuswb	mm4,	mm6					/* mm4 = 37 36 35 34 33 32 31 30 */
				
				movq		[eax+ecx*2], mm0			/* write 20 21 22 23 24 25 26 27 */
				lea			eax,	[eax+ecx*4]			/* mov forward the desPtr */

				movq		[eax+edx],	mm4				/* write 30 31 32 33 34 35 36 37 */
				add			edi, 8						/* move to right four column */
				add			esi, 8						/* move to right x1 */

				movq		mm0,	[esi]				/* mm0 = 70 60 50 40 */
				movq		mm1,	[esi+16]			/* mm1 = 71 61 51 41 */

				movq		mm4,	mm0					/* mm4 = 70 60 50 40 */
				punpcklwd	mm0,	mm1					/* mm0 = 51 50 41 40 */

				punpckhwd	mm4,	mm1					/* mm4 = 71 70 61 60 */
				movq		mm2,	[esi+32]			/* mm2 = 72 62 52 42 */

				movq		mm3,	[esi+48]			/* mm3 = 73 63 53 43 */
				movq		mm5,	mm2					/* mm5 = 72 62 52 42 */

				punpcklwd	mm2,	mm3					/* mm2 = 53 52 43 42 */
				punpckhwd	mm5,	mm3					/* mm5 = 73 72 63 62 */

				movq		mm1,	mm0					/* mm1 = 51 50 41 40 */
				punpckldq	mm0,	mm2					/* mm0 = 43 42 41 40 */

				movq		[edi],	mm0					/* write 40 41 42 43 */
				punpckhdq	mm1,	mm2					/* mm1 = 53 52 51 50 */
				
				movq		mm0,	mm4					/* mm0 = 71 70 61 60 */
				movq		[edi+16], mm1				/* write 50 51 52 53 */

				punpckldq	mm0,	mm5					/* mm0 = 63 62 61 60 */
				punpckhdq	mm4,	mm5					/* mm4 = 73 72 71 70 */

				movq		mm1,	[esi+64]			/* mm1 = 74 64 54 44 */
				movq		mm2,	[esi+80]			/* mm2 = 75 65 55 45 */				

				movq		mm5,	[esi+96]			/* mm5 = 76 66 56 46 */
				movq		mm6,	[esi+112]			/* mm6 = 77 67 57 47 */
								
				movq		mm3,	mm1					/* mm3 = 74 64 54 44 */
				movq		mm7,	mm5					/* mm7 = 76 66 56 46 */

				punpcklwd	mm1,	mm2					/* mm1 = 55 54 45 44 */
				punpckhwd	mm3,	mm2					/* mm3 = 75 74 65 64 */

				punpcklwd	mm5,	mm6					/* mm5 = 57 56 47 46 */
				punpckhwd	mm7,	mm6					/* mm7 = 77 76 67 66 */

				movq		mm2,	mm1					/* mm2 = 55 54 45 44 */
				movq		mm6,	mm3					/* mm6 = 75 74 65 64 */

				punpckldq	mm1,	mm5					/* mm1 = 47 46 45 44 */
				punpckhdq	mm2,	mm5					/* mm2 = 57 56 55 54 */

				punpckldq	mm3,	mm7					/* mm3 = 67 66 65 64 */
				punpckhdq	mm6,	mm7					/* mm6 = 77 76 75 74 */
			
				movq		mm5,	[edi]				/* mm5 = 43 42 41 40 */
				packuswb	mm5,	mm1					/* mm5 = 47 46 45 44 43 42 41 40 */
				
				movq		[eax],	mm5					/* write 40 41 42 43 44 45 46 47 */
				movq		mm7,	[edi+16]			/* mm7 = 53 52 51 50 */

				packuswb	mm7,	mm2					/* mm7 = 57 56 55 54 53 52 51 50 */
				movq		[eax+ecx], mm7				/* write 50 51 52 53 54 55 56 57 */

				packuswb	mm0,	mm3					/* mm0 = 67 66 65 64 63 62 61 60 */
				packuswb	mm4,	mm6					/* mm4 = 77 76 75 74 73 72 71 70 */
				
				movq		[eax+ecx*2], mm0			/* write 60 61 62 63 64 65 66 67 */
				lea			eax,	[eax+ecx*4]			/* mov forward the desPtr */

				movq		[eax+edx],	mm4				/* write 70 71 72 73 74 75 76 77 */
				
				pop			edi
				pop			esi
				pop			edx
				pop			ecx
				pop			ebp
				pop			eax
			}//__asm	

			Var1 = Variance11[0]+ Variance11[1]+Variance11[2]+Variance11[3];
			Var1 += Variance12[0]+ Variance12[1]+Variance12[2]+Variance12[3];
			pbi->FragmentVariances[CurrentFrag-1] += Var1;

			Var2 = Variance21[0]+ Variance21[1]+Variance21[2]+Variance21[3];
			Var2 += Variance22[0]+ Variance22[1]+Variance22[2]+Variance22[3];
			pbi->FragmentVariances[CurrentFrag] += Var2;
		
			CurrentFrag ++;

		}//else
			
	}//while

}

/****************************************************************************
 * 
 *  ROUTINE       :     DeblockNonFilteredBandNewFilter_MMX(
 *
 *  INPUTS        :     None
 *                               
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Filter both horizontal and vertical edge in a band
 *
 *  SPECIAL NOTES :     Using Sum of abs to determine where to apply the 
 *                      new 7 tap filter
 *
 *	REFERENCE	  :		
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
 void DeblockNonFilteredBandNewFilter_MMX(
                                          POSTPROC_INSTANCE *pbi, 
                                          UINT8 *SrcPtr, 
                                          UINT8 *DesPtr,
                                          UINT32 PlaneLineStep, 
                                          UINT32 FragAcross,
                                          UINT32 StartFrag,
                                          UINT32 *QuantScale
                                          )
{
	UINT32 j;
	UINT32 CurrentFrag=StartFrag;
	UINT32 QStep;
    UINT32 LoopFLimit;
	UINT8 *Src, *Des;

#if defined(_WIN32_WCE)
	#pragma pack(16)
	short QStepMmx[4];
	short FLimitMmx[4];
	short LoopFLimitMmx[4];
	short Rows[80];
	short NewRows[64];
	short LoopFilteredValuesUp[4];
	short LoopFilteredValuesDown[4];
	unsigned char Variance11[8];
	unsigned char Variance21[8];
    UINT32 Var1, Var2;
	#pragma pack()
#else
	__declspec(align(16)) short QStepMmx[4];
	__declspec(align(16)) short FLimitMmx[4];
	__declspec(align(16)) short LoopFLimitMmx[4];
	__declspec(align(16)) short Rows[80];
	__declspec(align(16)) short NewRows[64];
	__declspec(align(16)) short LoopFilteredValuesUp[4];
	__declspec(align(16)) short LoopFilteredValuesDown[4];
	__declspec(align(16)) unsigned char Variance11[8];
	__declspec(align(16)) unsigned char Variance21[8];
    UINT32 Var1, Var2;
#endif


	QStep = QuantScale[pbi->FrameQIndex];
	QStepMmx[0] = (INT16)QStep;
    QStepMmx[1] = (INT16)QStep;
    QStepMmx[2] = (INT16)QStep;
    QStepMmx[3] = (INT16)QStep;
    LoopFLimit = DeblockLimitValuesV2[pbi->FrameQIndex];
    LoopFLimitMmx[0] = (INT16)LoopFLimit;
    LoopFLimitMmx[1] = (INT16)LoopFLimit;
    LoopFLimitMmx[2] = (INT16)LoopFLimit;
    LoopFLimitMmx[3] = (INT16)LoopFLimit;

	while(CurrentFrag < StartFrag + FragAcross )
	{

		Src=SrcPtr+8*(CurrentFrag-StartFrag);
		Des=DesPtr+8*(CurrentFrag-StartFrag);
    	__asm 
		{
			
			push		eax

			push		ebp
			
			push		ecx			

			push		edx

			push		esi

			push		edi

			/* Calculate the FLimit and store FLimit and QStep */					
			/* Copy the data to the intermediate buffer */							
			mov			eax,	QStep
			xor			edx,	edx					/* clear edx */					

			mov			ecx,	PlaneLineStep		/* ecx = Pitch */				
			movd		mm5,	eax

            mov			eax,	Src					/* eax = Src */					
			punpcklwd	mm5,	mm5					

			lea			esi,	NewRows				/* esi = NewRows */
			punpckldq	mm5,	mm5
			
            sub			edx,	ecx					/* edx = - Pitch */
            movq        mm6,    mm5                 /*  Q Q Q Q */

            paddw       mm6,    mm5                 
            paddw       mm6,    mm5                 /* 3Q3Q3Q3Q */

            packuswb    mm5,    mm5                 /* QQQQQQQQ */            
			movq		QStepMmx,	mm5

            psraw       mm6,    2                   /*  F F F F */           
            packuswb    mm6,    mm6                 /* FFFFFFFF */

			lea			edi,	Rows				/* edi = Rows */				
            pxor		mm7,	mm7					/* Clear mm7 */					

            psubb       mm6,    Eight128c           /* Eight (F-128)s */        
    
			lea			eax,	[eax + edx * 4 ]	/* eax = Src - 4*Pitch */		
			movq		mm0,	[eax + edx]			/* mm0 = Src[-5*Pitch] */		

			movq		mm1,	mm0					/* mm1 = mm0 */					
			punpcklbw	mm0,	mm7					/* Lower Four -5 */				

            movq        mm4,    mm1                 /* mm4 = Src[-5*Pitch] */
            movq		[FLimitMmx], mm6            /* FFFF FFFF */		
			
			movq		mm2,	[eax]				/* mm2 = Src[-4*Pitch] */		
			punpckhbw	mm1,	mm7					/* Higher Four -5 */	
            
			movq		[edi],	mm0					/* Write Lower Four of -5 */					
            movq        mm5,    mm2                 /* mm5 = S_4 */
            
            movq		mm3,	mm2					/* mm3 = S_4 */					
			movq		[edi+8], mm1				/* Write Higher Four of -5 */	

            movq		mm0,	[eax + ecx]			/* mm0 = Src[-3*Pitch] */		
            psubusb     mm5,    mm4                 /* S_4 - S_5 */
            
            psubusb     mm4,    mm2                 /* S_5 - S_4 */
            punpcklbw	mm2,	mm7					/* Lower Four -4 */				

            por         mm4,    mm5                 /* abs(S_4-S_5) */
            movq		[edi+16], mm2				/* Write Lower -4 */			

            movq        mm6,    mm3                 /* mm6 = S_4 */
			punpckhbw	mm3,	mm7					/* higher Four -4 */			

            movq		[edi+24], mm3				/* write hight -4 */						
            movq		mm1,	mm0					/* mm1 = S_3 */					

			punpcklbw	mm0,	mm7					/* lower four -3 */				
			movq		[edi+32], mm0				/* write Lower -3 */			

			movq		mm2,	[eax + ecx *2]		/* mm2 = Src[-2*Pitch] */		
            movq        mm5,    mm1                 /* mm5 = S_3 */

            psubusb     mm5,    mm6                 /* S_3 - S_4 */            
            psubusb     mm6,    mm1                 /* S_4 - S_3 */

            por         mm5,    mm6                 /* abs(S_4-S_3) */
            movq        mm6,    mm1                 /* mm6 = S_3 */
			
			punpckhbw	mm1,	mm7					/* higher four -3 */						
			movq		mm3,	mm2					/* mm3 = S_2 */					
			
			movq		[edi+40], mm1				/* write Higher -3 */			
            paddusb      mm4,    mm5                 /* abs(S_5-S_4)+abs(S_4-S_3) */
    
            movq        mm5,    mm2                 /* mm5 = S_2 */
            psubusb     mm5,    mm6                 /* S_2 - S_3 */

            psubusb     mm6,    mm2                 /* S_3 - S_2 */
            por         mm5,    mm6                 /* abs(S_3 - S_2) */

            movq        mm6,    mm2                 /* mm6 = S_2 */

			punpcklbw	mm2,	mm7					/* lower four -2 */				
			lea			eax,	[eax + ecx *4]		/* eax = Src */					
			
			punpckhbw	mm3,	mm7					/* higher four -2 */			

			movq		mm0,	[eax + edx]			/* mm2 = Src[-Pitch] */			
			movq		[edi+48], mm2				/* lower -2	*/					
			
            paddusb     mm4,    mm5                 /* abs(S_5-S_4)+abs(S_4-S_3)+abs(S_3-S_2) */
            movq        mm5,    mm0                 /* mm5 = S_1 */

			movq		[edi+56], mm3				/* higher -2 */					
            movq		mm1,	mm0					/* mm1 = S_1 */					

            psubusb     mm5,    mm6                 /* S_1 - S_2 */
            psubusb     mm6,    mm1                 /* S_2 - S_1 */
			
            punpcklbw	mm0,	mm7					/* lower -1 */					
            por         mm5,    mm6                 /* abs(S_2 - S_1) */

            movq		[edi+64], mm0				/* Lower -1 */					
            movq        mm6,    mm1                 /* mm6 = S_1 */

            punpckhbw	mm1,	mm7					/* Higher -1 */					
			movq		[edi+72], mm1				/* Higher -1 */					

			movq		mm0,	[eax]				/* mm0 = Src[0] */				
            paddusb       mm4,    mm5               /* abs(S_5-S_4)+abs(S_4-S_3)+abs(S_3-S_2)+abs(S_2 - S_1) */

            movq        [Variance11], mm4;          /* save the variance */

            movq        mm5,    FLimitMmx           /* mm5 = FFFF FFFF */
            psubb       mm4,    Eight128c           /* abs(..) - 128 */

            pcmpgtb     mm5,    mm4                 /* abs(S_5-S_4)+abs(S_4-S_3)+abs(S_3-S_2)+abs(S_2 - S_1) < FLimit ? */
			            
            movq		mm1,	mm0					/* mm1 = S0 */					
			punpcklbw	mm0,	mm7					/* lower 0 */					
    
            movq        mm4,    mm1                 /* mm4 = S0 */
			movq		[edi+80], mm0				/* write lower 0 */				

            psubusb     mm4,    mm6                 /* S0 - S_1 */
            psubusb     mm6,    mm1                 /* S_1 - S0 */

			movq		mm0,	[eax + ecx]			/* mm0 = Src[Pitch] */			
            movq        mm3,    QStepMmx            /* mm3 = QQQQQQQQQ */

            por         mm4,    mm6                 /* abs(S0 - S_1) */            
            movq        mm6,    mm1                 /* mm6 = S0 */
            
            psubb       mm3,    Eight128c           /* -128 for using signed compare*/
            psubb       mm4,    Eight128c           /* -128 for using signed compare*/

            pcmpgtb     mm3,    mm4                 /* abs(S0-S_1) < QStep */
			punpckhbw	mm1,	mm7					/* higher 0 */			
            
            movq        mm4,    mm0                 /* mm4 = S1 */
            pand        mm5,    mm3                 /* abs(S_5-S_4)+abs(S_4-S_3)+abs(S_3-S_2)+abs(S_2 - S_1) < FLimit &&
                                                       abs(S0-S_1) < QStep */

            movq		[edi+88], mm1				/* write higher 0 */			
			
			movq		mm1,	mm0					/* mm1 = S1 */					
            psubusb     mm4,    mm6                 /* S1 - S0 */

			punpcklbw	mm0,	mm7					/* lower 1 */					
            psubusb     mm6,    mm1                 /* S0 - S1 */

            movq		[edi+96], mm0				/* write lower 1 */		
            por         mm4,    mm6                 /* mm4 = abs(S1-S0) */

			movq		mm2,	[eax + ecx *2 ]     /* mm2 = Src[2*Pitch] */		
            movq        mm6,    mm1                 /* mm6 = S1 */

            lea			eax,	[eax + ecx *4]		/* eax = Src + 4 * Pitch  */	
            punpckhbw	mm1,	mm7					/* higher 1 */					
			
			
			movq		mm0,	mm2					/* mm0 = S2 */					
			movq		[edi+104], mm1				/* wirte higher 1 */			


            movq        mm3,    mm0                 /* mm3 = S2 */
			movq		mm1,	[eax + edx ]		/* mm4 = Src[3*pitch] */		
            
            punpcklbw	mm2,	mm7					/* lower 2 */					
            psubusb     mm3,    mm6                 /* S2 - S1 */
            
            psubusb     mm6,    mm0                 /* S1 - S2 */
            por         mm3,    mm6                 /* abs(S1-S2) */

            movq		[edi+112], mm2				/* write lower 2 */				
            movq        mm6,    mm0                 /* mm6 = S2 */

			punpckhbw	mm0,	mm7					/* higher 2 */					
            paddusb       mm4,    mm3                 /* abs(S0-S1)+abs(S1-S2) */
			
            movq        mm2,    mm1                 /* mm2 = S3 */            
            movq        mm3,    mm1                 /* mm3 = S3 */
			
			movq		[edi+120], mm0				/* write higher 2 */			
			punpcklbw	mm1,	mm7					/* Low 3	*/					

			movq		mm0,	[eax]				/* mm0 = Src[4*pitch] */		
            psubusb     mm3,    mm6                 /* S3 - S2 */

            psubusb     mm6,    mm2                 /* S2 - S3 */
            por         mm3,    mm6                 /* abs(S2-S3) */
            
            movq		[edi+128], mm1				/* low 3 */						
            movq        mm6,    mm2                 /* mm6 = S3 */
            
			punpckhbw	mm2,	mm7					/* high 3 */					
			paddusb       mm4,    mm3                 /* abs(S0-S1)+abs(S1-S2)+abs(S2-S3) */


			movq		mm1,	mm0					/* mm1 = S4 */					
            movq        mm3,    mm0                 /* mm3 = S4 */
			
            movq		[edi+136], mm2				/* high 3 */					
            punpcklbw	mm0,	mm7					/* low 4 */						
            
            psubusb     mm3,    mm6                 /* S4 - S3 */
   			movq		[edi+144], mm0				/* low 4 */						
   
            psubusb     mm6,    mm1                 /* S3 - S4 */
            por         mm3,    mm6                 /* abs(S3-S4) */

            punpckhbw	mm1,	mm7					/* high 4 */							
			paddusb     mm4,    mm3                 /* abs((S0-S1)+abs(S1-S2)+abs(S2-S3)+abs(S3-S4) */
        
            movq        [Variance21], mm4;          /* save the variance */

            movq        mm6,    FLimitMmx           /* mm6 = FFFFFFFFF */
			psubb        mm4,    Eight128c           /* abs(..) - 128 */

            movq		[edi+152], mm1				/* high 4 */					
			
	        pcmpgtb     mm6,    mm4                 /* abs((S0-S1)+abs(S1-S2)+abs(S2-S3)+abs(S3-S4)<FLimit? */
            pand        mm6,    mm5                 /* Flag */

			/* done with copying everything to intermediate buffer */				
            /* mm7 = 0, mm6 = Flag */								
            movq        mm0,    mm6
            movq        mm7,    mm6 
            
            punpckhbw   mm0,    mm6
            punpcklbw   mm7,    mm6
            
			/* mm0 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
			/* mm0 and mm7 now are in use  */										
            
            /* find the loop filtered values for the pixels on block boundary */
            movq        mm1,    LoopFLimitMmx;      /* Get the Flimit values for loop filter */
            movq        mm3,    [edi + 48]          /* mm3 = x3 = p[-2] */

            movq        mm4,    [edi + 64]          /* mm4 = x4 = p[-1] */
            movq        mm5,    [edi + 80]          /* mm5 = x5 = p[ 0] */

            movq        mm6,    [edi + 96]          /* mm6 = x6 = p[ 1] */
            psubw       mm5,    mm4                 /* mm5 = p[ 0] - p[-1] */

            psubw       mm3,    mm6                 /* mm3 = p[-2] - p[ 1] */
            movq        mm4,    mm5                 /* make a copy */

            paddw       mm4,    mm5                 /* 2 * ( p[0] - p[-1] ) */
            paddw       mm3,    FourFours           /* mm3 + 4 */

            paddw       mm5,    mm4                 /* 3 * ( p[0] - p[-1] ) */
            paddw       mm3,    mm5                 /* Filtval before shift */

            psraw       mm3,    3                   /* FiltVal */
            movq        mm2,    mm3                 /* make a copy */

            psraw       mm3,    15                  /* FFFF->Neg, 0000->Pos */
            pxor        mm2,    mm3

            psubsw      mm2,    mm3                 /* mm2 = abs(FiltVal) */
            por         mm3,    FourOnes            /* -1 and 1 for + and - */

            movq        mm4,    mm1                 /* make a copy of Flimit */
            psubw       mm1,    mm2                 /* mm1= Flimit - abs(FiltVal) */

            movq        mm5,    mm1                 /* copy Flimit - abs(FiltVal) */
            psraw       mm1,    15                  /* FFFF or 0000 */

            pxor        mm5,    mm1                 
            psubsw      mm5,    mm1                 /* abs(Flimit - abs(FiltVal)) */

            psubusw     mm4,    mm5                 /* Flimit-abs(Flimit - abs(FiltVal)) */
            pmullw      mm4,    mm3                 /* get the sign back */

            movq        mm1,    [edi+64]            /* p[-1] */
            movq        mm2,    [edi+80]            /* p[0] */
            
            paddw       mm1,    mm4                 /* p[-1] + NewFiltVal */
            psubw       mm2,    mm4                 /* p[0] - NewFiltVal */

            pxor        mm6,    mm6                 /* clear mm6 */
            
            packuswb    mm1,    mm1                 /* clamping */
            packuswb    mm2,    mm2                 /* clamping */

            punpcklbw   mm1,    mm6                 /* unpack to word */
            movq        LoopFilteredValuesUp, mm1   /* save the values */

            punpcklbw   mm2,    mm6                 /* unpack to word */
            movq        LoopFilteredValuesDown, mm2 /* save the values */

            /* Let's do the filtering now */										
            /* p1 = Src[-5] */		
            /* p2 = Src[+4] */		
            /* sum = p1 + p1 + p1 + x1 + x2 + x3 + x4 + 4; */				
            
            movq		mm3,	[edi]			    /* mm3 = [-5] */
            movq		mm2,	[edi+144]			/* mm2 = [4] */					

            movq		mm1,	mm3					/* p1 = [-4] */					
			paddw		mm3,	mm3					/* mm3 = p1 + p1 */				

			movq		mm4,	[edi+16]			/* mm4 = x1 */					
			paddw		mm3,	mm1					/* mm3 = p1 + p1 + p1 */		
            
			paddw		mm3,	[edi+32]			/* mm3 = p1+p1+p1+ x2 */		
			paddw		mm4,	[edi+48]			/* mm4 = x1+x3 */				
			
			paddw		mm3,	[edi+64]			/* mm3 += x4 */					
			paddw		mm4,	FourFours			/* mm4 = x1 + x3 + 4 */			
			
			paddw		mm3,	mm4					/* mm3 = 3*p1+x1+x2+x3+x4+4 */	

            /* Des[-w4] = (((sum + x1) >> 3; */			
			/* Des[-w4] = Src[-w4]; */												
			/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	
			
			movq		mm4,	mm3					/* mm4 = mm3 */					
			movq		mm5,	[edi+16]			/* mm5 = x1 */					

            paddw		mm4,	mm5					/* mm4 = sum+x1 */				
			psraw		mm4,	3					/* mm4 >>=4 */					

            psubw		mm4,	mm5					/* New Value - old Value */		
			pand		mm4,	mm7					/* And the flag */				
			
			paddw		mm4,	mm5					/* add the old value back */	
			movq		[esi],	mm4					/* Write new x1 */				
			
			/* sum += x5 -p1 */														
			/* Des[-w3]=((sum+x2)>>3 */									
			
			movq		mm5,	[edi+32]			/* mm5= x2 */					
			psubw		mm3,	mm1					/* sum=sum-p1 */				
			
			paddw		mm3,    [edi+80]			/* sum=sum+x5 */				
			movq		mm4,	mm5					/* copy sum */					
			
			paddw		mm4,	mm3					/* mm4=sum+x2 */				
			psraw		mm4,	3					/* mm4=((sum+x2)<<1-x5+x6)>>4 */
			psubw		mm4,	mm5					/* new value - old value	*/	
			
			pand		mm4,	mm7					/* And the flag */				
			paddw		mm4,	mm5					/* add the old value back */	
			
			movq		[esi+16], mm4				/* write new x2 */				
			
			/* sum += x6 - p1 */													
			/* Des[-w2]=((sum+x[3])>>3 */								
			
			movq		mm5,	[edi+48]			/* mm5= x3 */					
			psubw		mm3,	mm1					/* sum=sum-p1 */				
			
			paddw		mm3,    [edi+96]			/* sum=sum+x6 */				
			movq		mm4,	mm5					/* copy x3 */					
			
			paddw		mm4,	mm3					/* mm4=sum+x3 */				
			psraw		mm4,	3					/* mm4=((sum+x3)<<1-x6+x7)>>4 */

            psubw		mm4,	mm5					/* new value - old value	*/	
			pand		mm4,	mm7					/* And the flag */				

            paddw		mm4,	mm5					/* add the old value back */	
			movq		[esi+32], mm4				/* write new x3 */				
			
			/* sum += x7 - p1 */													
			/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
			
			movq		mm5,	[edi+64]			/* mm5 = x4 */					
			psubw		mm3,	mm1					/* sum = sum-p1 */				
			
			paddw		mm3,	[edi+112]			/* sum = sum+x7 */				
			movq		mm4,	mm5					/* mm4 = x4 */					
			
			paddw		mm4,	mm3					/* mm4 = sum + x4 */			
			movq        mm5,    LoopFilteredValuesUp/* Read the loopfiltered value of x4 */

            psraw		mm4,	3					/* >>=4 */						
            psubw		mm4,	mm5					/* -=x4 */						

            pand		mm4,	mm7					/* and flag */					
            paddw		mm4,	mm5					/* += x4 */						

            movq		[esi+48], mm4				/* write new x4 */				
			
			/* sum+= x8-x1 */														
			/* Des[0]=((sum+x5)>>3 */								
			
			movq		mm5,	[edi+80]			/* mm5 = x5 */					
			psubw		mm3,	[edi+16]			/* sum -= x1 */					
			
			paddw		mm3,	[edi+128]			/* sub += x8 */					
			movq		mm4,	mm5					/* mm4 = x5 */					
			
			paddw		mm4,	mm3					/* mm4= sum+x5 */				
			movq        mm5,    LoopFilteredValuesDown/* Read the loopfiltered value of x4 */

            psraw		mm4,	3					/* >>=4 */						
            psubw		mm4,	mm5					/* -=x5 */						

            pand		mm4,	mm7					/* and flag */					
            paddw		mm4,	mm5					/* += x5 */										

            movq		[esi+64], mm4				/* write new x5 */				
			
			/* sum += p2 - x2 */													
			/* Des[w1] = ((sum+x6)>>3 */								
			
			movq		mm5,	[edi+96]			/* mm5 = x6 */					
			psubw		mm3,	[edi+32]			/* -= x2 */						
			
			paddw		mm3,	mm2					/* += p2 */						
			movq		mm4,	mm5					/* mm4 = x6 */					
			
			paddw		mm4,	mm3					/* mm4 = sum+x6 */				
			psraw		mm4,	3					/* >>=3 */						

			psubw		mm4,	mm5					/* -=x6 */						
			pand		mm4,	mm7					/* and flag */					

            paddw		mm4,	mm5					/* += x6 */						
			movq		[esi+80], mm4				/* write new x6 */				
			
			/* sum += p2 - x3 */													
			/* Des[w2] = (sum+x7)>>3 */								
			
			movq		mm5,	[edi+112]			/* mm5 = x7 */					
			psubw		mm3,	[edi+48]			/* -= x3 */						
			
			paddw		mm3,	mm2					/* += p2 */						
			movq		mm4,	mm5					/* mm4 = x7 */					
			
			paddw		mm4,	mm3					/* mm4 = sum+x7 */				
			psraw		mm4,	3					/* >>=3 */						

			psubw		mm4,	mm5					/* -=x7 */						
			pand		mm4,	mm7					/* and flag */					

            paddw		mm4,	mm5					/* += x7 */						
			movq		[esi+96], mm4				/* write new x7 */				
			
			/* sum += p2 - x4 */													
			/* Des[w3] = ((sum+x8)>>3 */								
			
			movq		mm5,	[edi+128]			/* mm5 = x8 */					
			psubw		mm3,	[edi+64]			/* -= x4 */						
			
			paddw		mm3,	mm2					/* += p2 */						
			movq		mm4,	mm5					/* mm4 = x8 */					
			
			paddw		mm4,	mm3					/* mm4 = sum+x8 */				
			psraw		mm4,	3					/* >>=3 */						
			
            psubw		mm4,	mm5					/* -=x8 */						
			pand		mm4,	mm7					/* and flag */					

            paddw		mm4,	mm5					/* += x8 */						
			movq		[esi+112], mm4				/* write new x8 */				
						
			/* done with left four columns */										
			/* now do the righ four columns */										
			
			add			edi,	8					/* shift to right four column */
			add			esi,	8					/* shift to right four column */
			
			/* mm0 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
			/* mm0 now are in use  */										
			

            /* find the loop filtered values for the pixels on block boundary */
            movq        mm1,    LoopFLimitMmx;      /* Get the Flimit values for loop filter */
            movq        mm3,    [edi + 48]          /* mm3 = x3 = p[-2] */

            movq        mm4,    [edi + 64]          /* mm4 = x4 = p[-1] */
            movq        mm5,    [edi + 80]          /* mm5 = x5 = p[ 0] */

            movq        mm6,    [edi + 96]          /* mm6 = x6 = p[ 1] */
            psubw       mm5,    mm4                 /* mm5 = p[ 0] - p[-1] */

            psubw       mm3,    mm6                 /* mm3 = p[-2] - p[ 1] */
            movq        mm4,    mm5                 /* make a copy */

            paddw       mm3,    FourFours           /* mm3 + 4 */
            paddw       mm4,    mm4                 /* 2 * ( p[0] - p[-1] ) */

            paddw       mm3,    mm4                 /* 3 * ( p[0] - p[-1] ) */
            paddw       mm3,    mm5                 /* Filtval before shift */

            psraw       mm3,    3                   /* FiltVal */
            movq        mm2,    mm3                 /* make a copy */

            psraw       mm3,    15                  /* FFFF->Neg, 0000->Pos */
            pxor        mm2,    mm3

            psubsw      mm2,    mm3                 /* mm2 = abs(FiltVal) */
            por         mm3,    FourOnes            /* -1 and 1 for + and - */

            movq        mm4,    mm1                 /* make a copy of Flimit */
            psubw       mm1,    mm2                 /* mm1= Flimit - abs(FiltVal) */

            movq        mm5,    mm1                 /* copy Flimit - abs(FiltVal) */
            psraw       mm1,    15                  /* FFFF or 0000 */

            pxor        mm5,    mm1                 
            psubsw      mm5,    mm1                 /* abs(Flimit - abs(FiltVal)) */

            psubusw     mm4,    mm5                 /* Flimit-abs(Flimit - abs(FiltVal)) */
            pmullw      mm4,    mm3                 /* get the sign back */

            movq        mm1,    [edi+64]            /* p[-1] */
            movq        mm2,    [edi+80]            /* p[0] */
            
            paddw       mm1,    mm4                 /* p[-1] + NewFiltVal */
            psubw       mm2,    mm4                 /* p[0] - NewFiltVal */

            pxor        mm6,    mm6                 /* clear mm6 */
            
            packuswb    mm1,    mm1                 /* clamping */
            packuswb    mm2,    mm2                 /* clamping */

            punpcklbw   mm1,    mm6                 /* unpack to word */
            movq        LoopFilteredValuesUp, mm1   /* save the values */

            punpcklbw   mm2,    mm6                 /* unpack to word */
            movq        LoopFilteredValuesDown, mm2 /* save the values */
            
            
            /* Let's do the filtering now */										
            /* p1 = Src[-5] */		
            /* p2 = Src[+4] */		
            /* sum = p1 + p1 + p1 + x1 + x2 + x3 + x4 + 4; */				
            
            movq		mm3,	[edi]			    /* mm3 = [-5] */
            movq		mm2,	[edi+144]			/* mm2 = [4] */					
            
            movq		mm1,	mm3					/* p1 = [-4] */					
			paddw		mm3,	mm3					/* mm3 = p1 + p1 */				

			movq		mm4,	[edi+16]			/* mm4 = x1 */					
			paddw		mm3,	mm1					/* mm3 = p1 + p1 + p1 */		
            
			paddw		mm3,	[edi+32]			/* mm3 = p1+p1+p1+ x2 */		
			paddw		mm4,	[edi+48]			/* mm4 = x1+x3 */				
			
			paddw		mm3,	[edi+64]			/* mm3 += x4 */					
			paddw		mm4,	FourFours			/* mm4 = x1 + x3 + 4 */			
			
			paddw		mm3,	mm4					/* mm3 = 3*p1+x1+x2+x3+x4+4 */	

            /* Des[-w4] = (((sum + x1) >> 3; */			
			/* Des[-w4] = Src[-w4]; */												
			/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	
			movq		mm4,	mm3					/* mm4 = mm3 */					
			movq		mm5,	[edi+16]			/* mm5 = x1 */					

            paddw		mm4,	mm5					/* mm4 = sum+x1 */				
			psraw		mm4,	3					/* mm4 >>=4 */					

            psubw		mm4,	mm5					/* New Value - old Value */		
			pand		mm4,	mm0					/* And the flag */				
			
			paddw		mm4,	mm5					/* add the old value back */	
			movq		[esi],	mm4					/* Write new x1 */				
			
			/* sum += x5 -p1 */														
			/* Des[-w3]=((sum+x2)>>3 */									
			
			movq		mm5,	[edi+32]			/* mm5= x2 */					
			psubw		mm3,	mm1					/* sum=sum-p1 */				
			
			paddw		mm3,    [edi+80]			/* sum=sum+x5 */				
			movq		mm4,	mm5					/* copy sum */					
			
			paddw		mm4,	mm3					/* mm4=sum+x2 */				
			psraw		mm4,	3					/* mm4=((sum+x2)<<1-x5+x6)>>4 */
			psubw		mm4,	mm5					/* new value - old value	*/	
			
			pand		mm4,	mm0					/* And the flag */				
			paddw		mm4,	mm5					/* add the old value back */	
			
			movq		[esi+16], mm4				/* write new x2 */				
			
			/* sum += x6 - p1 */													
			/* Des[-w2]=((sum+x[3])>>3 */								
			
			movq		mm5,	[edi+48]			/* mm5= x3 */					
			psubw		mm3,	mm1					/* sum=sum-p1 */				
			
			paddw		mm3,    [edi+96]			/* sum=sum+x6 */				
			movq		mm4,	mm5					/* copy x3 */					
			
			paddw		mm4,	mm3					/* mm4=sum+x3 */				
			psraw		mm4,	3					/* mm4=((sum+x3)<<1-x6+x7)>>4 */

            psubw		mm4,	mm5					/* new value - old value	*/	
			pand		mm4,	mm0					/* And the flag */				

            paddw		mm4,	mm5					/* add the old value back */	
			movq		[esi+32], mm4				/* write new x3 */				
			
			/* sum += x7 - p1 */													
			/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
			
			movq		mm5,	[edi+64]			/* mm5 = x4 */					
			psubw		mm3,	mm1					/* sum = sum-p1 */				
			
			paddw		mm3,	[edi+112]			/* sum = sum+x7 */				
			movq		mm4,	mm5					/* mm4 = x4 */					
			
			paddw		mm4,	mm3					/* mm4 = sum + x4 */			
			movq        mm5,    LoopFilteredValuesUp/* Read the loopfiltered value of x4 */

            psraw		mm4,	3					/* >>=4 */						
            psubw		mm4,	mm5					/* -=x4 */						

            pand		mm4,	mm0					/* and flag */					
            paddw		mm4,	mm5					/* += x4 */						

            movq		[esi+48], mm4				/* write new x4 */				
			
			/* sum+= x8-x1 */														
			/* Des[0]=((sum+x5)>>3 */								
			
			movq		mm5,	[edi+80]			/* mm5 = x5 */					
			psubw		mm3,	[edi+16]			/* sum -= x1 */					
			
			paddw		mm3,	[edi+128]			/* sub += x8 */					
			movq		mm4,	mm5					/* mm4 = x5 */					
			
			paddw		mm4,	mm3					/* mm4= sum+x5 */				
			movq        mm5,    LoopFilteredValuesDown/* Read the loopfiltered value of x4 */

            psraw		mm4,	3					/* >>=4 */						
            psubw		mm4,	mm5					/* -=x5 */						

            pand		mm4,	mm0					/* and flag */					
            paddw		mm4,	mm5					/* += x5 */										

            movq		[esi+64], mm4				/* write new x5 */				
			
			/* sum += p2 - x2 */													
			/* Des[w1] = ((sum+x6)>>3 */								
			
			movq		mm5,	[edi+96]			/* mm5 = x6 */					
			psubw		mm3,	[edi+32]			/* -= x2 */						
			
			paddw		mm3,	mm2					/* += p2 */						
			movq		mm4,	mm5					/* mm4 = x6 */					
			
			paddw		mm4,	mm3					/* mm4 = sum+x6 */				
			psraw		mm4,	3					/* >>=3 */						

			psubw		mm4,	mm5					/* -=x6 */						
			pand		mm4,	mm0					/* and flag */					

            paddw		mm4,	mm5					/* += x6 */						
			movq		[esi+80], mm4				/* write new x6 */				
			
			/* sum += p2 - x3 */													
			/* Des[w2] = (sum+x7)>>3 */								
			
			movq		mm5,	[edi+112]			/* mm5 = x7 */					
			psubw		mm3,	[edi+48]			/* -= x3 */						
			
			paddw		mm3,	mm2					/* += p2 */						
			movq		mm4,	mm5					/* mm4 = x7 */					
			
			paddw		mm4,	mm3					/* mm4 = sum+x7 */				
			psraw		mm4,	3					/* >>=3 */						

			psubw		mm4,	mm5					/* -=x7 */						
			pand		mm4,	mm0					/* and flag */					

            paddw		mm4,	mm5					/* += x7 */						
			movq		[esi+96], mm4				/* write new x7 */				
			
			/* sum += p2 - x4 */													
			/* Des[w3] = ((sum+x8)>>3 */								
			
			movq		mm5,	[edi+128]			/* mm5 = x8 */					
			psubw		mm3,	[edi+64]			/* -= x4 */						
			
			paddw		mm3,	mm2					/* += p2 */						
			movq		mm4,	mm5					/* mm4 = x8 */					
			
			paddw		mm4,	mm3					/* mm4 = sum+x8 */				
			psraw		mm4,	3					/* >>=3 */						
			
            psubw		mm4,	mm5					/* -=x8 */						
			pand		mm4,	mm0					/* and flag */					

            paddw		mm4,	mm5					/* += x8 */						
			movq		[esi+112], mm4				/* write new x8 */				

			
			/* done with right four column */										
			add			edi,	8					/* shift edi to point x1 */
			sub			esi,	8					/* shift esi back to x1 */

			mov			ebp, Des					/* the destination */							
			lea			ebp, [ebp + edx *4]			/* point to des[-w4] */			
			
			movq		mm0, [esi]													
			packuswb	mm0, [esi + 8]												
			
			movq		[ebp], mm0					/* write des[-w4] */			
			
			movq		mm1, [esi + 16]												
			packuswb	mm1, [esi + 24]												
			
			movq		[ebp+ecx ], mm1				/* write des[-w3] */			
			
			movq		mm2, [esi + 32]												
			packuswb	mm2, [esi + 40]												
			
			movq		[ebp+ecx*2 ], mm2			/* write des[-w2] */			
			
			movq		mm3, [esi + 48]												
			packuswb	mm3, [esi + 56]												
			
			lea			ebp, [ebp+ecx*4]			/* point to des[0] */			
			movq		[ebp+edx], mm3				/* write des[-w1] */			
			
			movq		mm0, [esi + 64]												
			packuswb	mm0, [esi + 72]												
			
			movq		[ebp ], mm0					/* write des[0] */				
			
			movq		mm1, [esi + 80]												
			packuswb	mm1, [esi + 88]												
			
			movq		[ebp+ecx], mm1				/* write des[w1] */				
			
			movq		mm2, [esi + 96]												
			packuswb	mm2, [esi + 104]											
			
			movq		[ebp+ecx*2], mm2			/* write des[w2] */				
			
			movq		mm3, [esi + 112]											
			packuswb	mm3, [esi + 120]											
			
			lea			ebp, [ebp+ecx*2]			/* point to des[w4] */			
			movq		[ebp+ecx], mm3				/* write des[w3] */				


			pop			edi
			pop			esi
			pop			edx
			pop			ecx
			pop			ebp
			pop			eax
			
		} /* end of the macro */
		
		Var1 = Variance11[0]+ Variance11[1]+Variance11[2]+Variance11[3];
		Var1 += Variance11[4]+ Variance11[5]+Variance11[6]+Variance11[7];
		pbi->FragmentVariances[CurrentFrag] += Var1;

		Var2 = Variance21[0]+ Variance21[1]+Variance21[2]+Variance21[3];
		Var2 += Variance21[4]+ Variance21[5]+Variance21[6]+Variance21[7];
		pbi->FragmentVariances[CurrentFrag + FragAcross] += Var2;

        if(CurrentFrag==StartFrag)
			CurrentFrag++;
		else 
		{

			Des=DesPtr-8*PlaneLineStep+8*(CurrentFrag-StartFrag);
			Src=Des;

			for( j=0; j<8;j++)
			{
				Rows[j] = (short) (Src[-5+j*PlaneLineStep]);
				Rows[72+j] = (short)(Src[4+j*PlaneLineStep]);		
			}

            __asm
			{
			/* Save the registers */
			push		eax
			push		ebp
				/* Calculate the FLimit and store FLimit and QStep */					
				mov			eax,	QStep				/* get QStep */
				movd		mm0,	eax					/* mm0 = 0, 0, 0, Q */

			push		ecx			
				
				punpcklwd	mm0,	mm0					/* mm0 = 0, 0, Q, Q */
				punpckldq	mm0,	mm0					/* mm0 = Q, Q, Q, Q */

			push		edx
				
                movq        mm1,    mm0                 /* mm1 = Q, Q, Q, Q */
                paddw       mm1,    mm0                                                       
				

			push		esi

               paddw        mm1,    mm0
               packuswb     mm0,    mm0
   
			push		edi
				
                movq		QStepMmx,	mm0				/* write the Q step */
				psraw		mm1,	2					/* mm1 = FLimit */				
		
                packuswb    mm1,    mm1                 /* mm1 = FFFF FFFF */
                psubb       mm1,    Eight128c           /* F-128 */

                movq		[FLimitMmx], mm1			/* Save FLimit */				

				/* setup the pointers to data */

				mov			eax,	Src					/* eax = Src */
				xor			edx,	edx					/* clear edx */
				
				sub			eax,	4					/* eax = Src-4 */
				lea			esi,	NewRows				/* esi = NewRows */
				lea			edi,	Rows				/* edi = Rows */				

				mov			ecx,	PlaneLineStep		/* ecx = Pitch */				
				sub			edx,	ecx					/* edx = -Pitch */				

				/* Get the data to the intermediate buffer */

				movq		mm0,	[eax]				/* mm0 = 07 06 05 04 03 02 01 00 */
				movq		mm1,	[eax+ecx]			/* mm1 = 17 16 15 14 13 12 11 10 */

				movq		mm2,	[eax+ecx*2]			/* mm2 = 27 26 25 24 23 22 21 20 */
				lea			eax,	[eax+ecx*4]			/* Go down four Rows */	

				movq		mm3,	[eax+edx]			/* mm3 = 37 36 35 34 33 32 31 30 */
				movq		mm4,	mm0					/* mm4 = 07 06 05 04 03 02 01 00 */
			
				punpcklbw	mm0,	mm1					/* mm0 = 13 03 12 02 11 01 10 00 */
				punpckhbw	mm4,	mm1					/* mm4 = 17 07 16 06 15 05 14 04 */

				movq		mm5,	mm2					/* mm5 = 27 26 25 24 23 22 21 20 */
				punpcklbw	mm2,	mm3					/* mm2 = 33 23 32 22 31 21 30 20 */

				punpckhbw	mm5,	mm3					/* mm5 = 37 27 36 26 35 25 34 24 */
				movq		mm1,	mm0					/* mm1 = 13 03 12 02 11 01 10 00 */

				punpcklwd	mm0,	mm2					/* mm0 = 31 21 11 01 30 20 10 00 */
				punpckhwd	mm1,	mm2					/* mm1 = 33 23 13 03 32 22 12 02 */
				
				movq		mm2,	mm4					/* mm2 = 17 07 16 06 15 05 14 04 */
				punpckhwd	mm4,	mm5					/* mm4 = 37 27 17 07 36 26 16 06 */

				punpcklwd	mm2,	mm5					/* mm2 = 35 25 15 05 34 24 14 04 */
				pxor		mm7,	mm7					/* clear mm7 */

				movq		mm5,	mm0					/* make a copy */
				punpcklbw	mm0,	mm7					/* mm0 = 30 20 10 00 */

				movq		[edi+16], mm0				/* write 00 10 20 30 */

				punpckhbw	mm5,	mm7					/* mm5 = 31 21 11 01 */

				movq		mm0,	mm1					/* mm0 =33 23 13 03 32 22 12 02 */
				movq		[edi+32], mm5				/* write 01 11 21 31 */
				
				punpcklbw	mm1,	mm7					/* mm1 = 32 22 12 02 */
				punpckhbw	mm0,	mm7					/* mm0 = 33 23 12 03 */

				movq		[edi+48], mm1				/* write 02 12 22 32 */
				movq		mm3,	mm2					/* mm3 = 35 25 15 05 34 24 14 04 */
				
				movq		mm5,	mm4					/* mm5 = 37 27 17 07 36 26 16 06 */
				movq		[edi+64], mm0				/* write 03 13 23 33 */


				punpcklbw	mm2,	mm7					/* mm2 = 34 24 14 04 */
				punpckhbw	mm3,	mm7					/* mm3 = 35 25 15 05 */

				movq		[edi+80], mm2				/* write 04 14 24 34 */
				punpcklbw	mm4,	mm7					/* mm4 = 36 26 16 06 */

				punpckhbw	mm5,	mm7					/* mm5 = 37 27 17 07 */
				movq		[edi+96], mm3				/* write 05 15 25 35 */
			
				movq		mm0,	[eax]				/* mm0 = 47 46 45 44 43 42 41 40 */
				movq		mm1,	[eax + ecx ]		/* mm1 = 57 56 55 54 53 52 51 50 */

				movq		[edi+112], mm4				/* write 06 16 26 37 */
				movq		mm2,	[eax+ecx*2]			/* mm2 = 67 66 65 64 63 62 61 60 */

				lea			eax,	[eax+ ecx*4]		/* Go down four rows */
				movq		[edi+128], mm5				/* write 07 17 27 37 */

				movq		mm4,	mm0					/* mm4 = 47 46 45 44 43 42 41 40 */
				movq		mm3,	[eax+edx]			/* mm3 = 77 76 75 74 73 72 71 70 */

				punpcklbw	mm0,	mm1					/* mm0 = 53 43 52 42 51 41 50 40 */
				punpckhbw	mm4,	mm1					/* mm4 = 57 57 56 46 55 45 54 44 */

				movq		mm5,	mm2					/* mm5 = 67 66 65 64 63 62 61 60 */
				punpcklbw	mm2,	mm3					/* mm2 = 73 63 72 62 71 61 70 60 */

				punpckhbw	mm5,	mm3					/* mm5 = 77 67 76 66 75 65 74 64 */
				movq		mm1,	mm0					/* mm1 = 53 43 52 42 51 41 50 40 */

				punpcklwd	mm0,	mm2					/* mm0 = 71 61 51 41 70 60 50 40 */
				punpckhwd	mm1,	mm2					/* mm1 = 73 63 53 43 72 62 52 42 */
				
				movq		mm2,	mm4					/* mm2 = 57 57 56 46 55 45 54 44 */
				punpckhwd	mm4,	mm5					/* mm4 = 77 67 57 47 76 66 56 46 */

				punpcklwd	mm2,	mm5					/* mm2 = 75 65 55 45 74 64 54 44 */

				movq		mm5,	mm0					/* make a copy */
				punpcklbw	mm0,	mm7					/* mm0 = 70 60 50 40 */

				movq		[edi+24], mm0				/* write 40 50 60 70 */
				punpckhbw	mm5,	mm7					/* mm5 = 71 61 51 41 */

				movq		mm0,	mm1					/* mm0 = 73 63 53 43 72 62 52 42 */
				movq		[edi+40], mm5				/* write 41 51 61 71 */
				
				punpcklbw	mm1,	mm7					/* mm1 = 72 62 52 42 */
				punpckhbw	mm0,	mm7					/* mm0 = 73 63 53 43 */

				movq		[edi+56], mm1				/* write 42 52 62 72 */
				movq		mm3,	mm2					/* mm3 = 75 65 55 45 74 64 54 44 */
				
				movq		mm5,	mm4					/* mm5 = 77 67 57 47 76 66 56 46 */
				movq		[edi+72], mm0				/* write 43 53 63 73 */

				punpcklbw	mm2,	mm7					/* mm2 = 74 64 54 44 */
				punpckhbw	mm3,	mm7					/* mm3 = 75 65 55 45 */

				movq		[edi+88], mm2				/* write 44 54 64 74 */
				punpcklbw	mm4,	mm7					/* mm4 = 76 66 56 46 */

				punpckhbw	mm5,	mm7					/* mm5 = 77 67 57 47 */
				movq		[edi+104], mm3				/* write 45 55 65 75 */
			
				movq		[edi+120], mm4				/* write 46 56 66 76 */
				movq		[edi+136], mm5				/* write 47 57 67 77 */


			    /* Now, compute the variances for Pixel  1-4 and 5-8 */					

                
                movq        mm0,    [edi]               /* S_5 */
                movq        mm1,    [edi+16]            /* S_4 */

                movq        mm2,    [edi+32]            /* S_3 */
                packuswb    mm0,    [edi+8]     

                packuswb    mm1,    [edi+24]
                packuswb    mm2,    [edi+40]

                movq        mm3,    [edi+48]            /* S_2 */
                movq        mm4,    [edi+64]            /* S_1 */

                packuswb    mm3,    [edi+56]
                packuswb    mm4,    [edi+72]

                movq        mm5,    mm1                 /* S_4 */
                movq        mm6,    mm2                 /* S_3 */

                psubusb     mm5,    mm0                 /* S_4 - S_5 */
                psubusb     mm0,    mm1                 /* S_5 - S_4 */

                por         mm0,    mm5                 /* abs(S_5-S_4) */
                psubusb     mm6,    mm1                 /* S_3 - S_4 */

                psubusb     mm1,    mm2                 /* S_4 - S_3 */
                movq        mm5,    mm3                 /* S_2 */

                por         mm1,    mm6                 /* abs(S_4-S_3) */
                psubusb     mm5,    mm2                 /* S_2 - S_3 */
                
                psubusb     mm2,    mm3                 /* S_3 - S_2 */
                movq        mm6,    mm4                 /* S_1 */

                por         mm2,    mm5                 /* abs(S_3-S_2) */
                psubusb     mm6,    mm3                 /* S_1 - S_2 */

                psubusb     mm3,    mm4                 /* S_2 - S_1 */
                por         mm3,    mm6                 /* abs(S_2-S_1) */

                paddusb      mm0,    mm1                 /* abs(S_5-S_4)+abs(S_4-S_3) */
                paddusb      mm2,    mm3                 /* abs(S_3-S_2)+abs(S_2-S_1) */

                movq        mm7,    FLimitMmx              /* FFFFF FFFF */
                paddusb      mm0,    mm2                 /* abs(S_5-S_4)+abs(S_4-S_3)+abs(S_3-S_2)+abs(S_2-S_1) */
                
                movq        [Variance11], mm0           /* Save the variance */

                movq        mm6,    mm4                 /* S_1 */
                psubb       mm0,    Eight128c           /* abs(..) - 128 */
                pcmpgtb     mm7,    mm0                 /* abs(S_5-S_4)+abs(S_4-S_3)+abs(S_3-S_2)+abs(S_2-S_1)<? */
                
				movq        mm5,    [edi+80]            /* S0 */
                movq        mm1,    [edi+96]            /* S1 */

                movq        mm2,    [edi+112]           /* S2 */
                packuswb    mm5,    [edi+88]     

                packuswb    mm1,    [edi+104]
                packuswb    mm2,    [edi+120]

                movq        mm3,    [edi+128]           /* S3 */
                movq        mm4,    [edi+144]           /* S4 */

                packuswb    mm3,    [edi+136]
                packuswb    mm4,    [edi+152]

                movq        mm0,    mm5                 /* S0 */
                psubusb     mm5,    mm6                 /* S0-S_1 */

                psubusb     mm6,    mm0                 /* S_1-S0 */
                por         mm5,    mm6                 /* abs(S_1-S0) */

                movq        mm6,    QStepMmx            /* QQQQ QQQQ */                
                psubb       mm5,    Eight128c           /* -128 for using signed compare*/

                psubb       mm6,    Eight128c           /* -128 for using signed compare*/
                pcmpgtb     mm6,    mm5                 /* abs(S_1-S0)<QStep? */

                movq        mm5,    mm1                 /* S1 */
                pand        mm7,    mm6                 /* abs(S_1-S0)<QStep &&
                                                            abs(S_5-S_4)+abs(S_4-S_3)+abs(S_3-S_2)+abs(S_2-S_1)<FLimit? */
                movq        mm6,    mm2                 /* S2 */
                psubusb     mm5,    mm0                 /* S1 - S0 */

                psubusb     mm0,    mm1                 /* S0 - S1*/

                por         mm0,    mm5                 /* abs(S0-S1) */
                psubusb     mm6,    mm1                 /* S2 - S1 */

                psubusb     mm1,    mm2                 /* S1 - S2*/
                movq        mm5,    mm3                 /* S3 */

                por         mm1,    mm6                 /* abs(S1-S2) */
                psubusb     mm5,    mm2                 /* S3 - S2 */
                
                psubusb     mm2,    mm3                 /* S2 - S3 */
                movq        mm6,    mm4                 /* S4 */

                por         mm2,    mm5                 /* abs(S2-S3) */
                psubusb     mm6,    mm3                 /* S4 - S3 */

                psubusb     mm3,    mm4                 /* S3 - S4 */
                por         mm3,    mm6                 /* abs(S3-S4) */

                paddusb      mm0,    mm1                 /* abs(S0-S1)+abs(S1-S2) */
                paddusb      mm2,    mm3                 /* abs(S2-S3)+abs(S3-S4) */

                movq        mm6,    FLimitMmx           /* FFFFF FFFF */
                paddusb      mm0,    mm2                 /* abs(S0-S1)+abs(S1-S2)+abs(S2-S3)+abs(S3-S4) */
                
                movq        [Variance21], mm0           /* Save the variance */
                
                psubb        mm0,    Eight128c            /* abs(..) - 128 */
                pcmpgtb     mm6,    mm0                 /* abs(S0-S1)+abs(S1-S2)+abs(S2-S3)+abs(S3-S4)<FLimit */
                pand        mm6,    mm7                 /* Flag */

                movq        mm0,    mm6
                movq        mm7,    mm6 
            
                punpckhbw   mm0,    mm6
                punpcklbw   mm7,    mm6

				/* mm0 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* mm0 and mm7 now are in use  */										
                /* find the loop filtered values for the pixels on block boundary */
                movq        mm1,    LoopFLimitMmx;      /* Get the Flimit values for loop filter */
                movq        mm3,    [edi + 48]          /* mm3 = x3 = p[-2] */

                movq        mm4,    [edi + 64]          /* mm4 = x4 = p[-1] */
                movq        mm5,    [edi + 80]          /* mm5 = x5 = p[ 0] */

                movq        mm6,    [edi + 96]          /* mm6 = x6 = p[ 1] */
                psubw       mm5,    mm4                 /* mm5 = p[ 0] - p[-1] */

                psubw       mm3,    mm6                 /* mm3 = p[-2] - p[ 1] */
                movq        mm4,    mm5                 /* make a copy */

                paddw       mm4,    mm5                 /* 2 * ( p[0] - p[-1] ) */
                paddw       mm3,    FourFours           /* mm3 + 4 */

                paddw       mm5,    mm4                 /* 3 * ( p[0] - p[-1] ) */
                paddw       mm3,    mm5                 /* Filtval before shift */

                psraw       mm3,    3                   /* FiltVal */
                movq        mm2,    mm3                 /* make a copy */

                psraw       mm3,    15                  /* FFFF->Neg, 0000->Pos */
                pxor        mm2,    mm3

                psubsw      mm2,    mm3                 /* mm2 = abs(FiltVal) */
                por         mm3,    FourOnes            /* -1 and 1 for + and - */

                movq        mm4,    mm1                 /* make a copy of Flimit */
                psubw       mm1,    mm2                 /* mm1= Flimit - abs(FiltVal) */

                movq        mm5,    mm1                 /* copy Flimit - abs(FiltVal) */
                psraw       mm1,    15                  /* FFFF or 0000 */

                pxor        mm5,    mm1                 
                psubsw      mm5,    mm1                 /* abs(Flimit - abs(FiltVal)) */

                psubusw     mm4,    mm5                 /* Flimit-abs(Flimit - abs(FiltVal)) */
                pmullw      mm4,    mm3                 /* get the sign back */

                movq        mm1,    [edi+64]            /* p[-1] */
                movq        mm2,    [edi+80]            /* p[0] */
                
                paddw       mm1,    mm4                 /* p[-1] + NewFiltVal */
                psubw       mm2,    mm4                 /* p[0] - NewFiltVal */

                pxor        mm6,    mm6                 /* clear mm6 */
                
                packuswb    mm1,    mm1                 /* clamping */
                packuswb    mm2,    mm2                 /* clamping */

                punpcklbw   mm1,    mm6                 /* unpack to word */
                movq        LoopFilteredValuesUp, mm1   /* save the values */

                punpcklbw   mm2,    mm6                 /* unpack to word */
                movq        LoopFilteredValuesDown, mm2 /* save the values */

                /* Let's do the filtering now */										
                /* p1 = Src[-5] */		
                /* p2 = Src[+4] */		
                /* sum = p1 + p1 + p1 + x1 + x2 + x3 + x4 + 4; */				
                
                movq		mm3,	[edi]			    /* mm3 = [-5] */
                movq		mm2,	[edi+144]			/* mm2 = [4] */					
                
                movq		mm1,	mm3					/* p1 = [-4] */					
                paddw		mm3,	mm3					/* mm3 = p1 + p1 */				
                
                movq		mm4,	[edi+16]			/* mm4 = x1 */					
                paddw		mm3,	mm1					/* mm3 = p1 + p1 + p1 */		
                
                paddw		mm3,	[edi+32]			/* mm3 = p1+p1+p1+ x2 */		
                paddw		mm4,	[edi+48]			/* mm4 = x1+x3 */				
                
                paddw		mm3,	[edi+64]			/* mm3 += x4 */					
                paddw		mm4,	FourFours			/* mm4 = x1 + x3 + 4 */			
                
                paddw		mm3,	mm4					/* mm3 = 3*p1+x1+x2+x3+x4+4 */	
                
                /* Des[-w4] = (((sum + x1) >> 3; */			
                /* Des[-w4] = Src[-w4]; */												
                /* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	
                
                movq		mm4,	mm3					/* mm4 = mm3 */					
                movq		mm5,	[edi+16]			/* mm5 = x1 */					
                
                paddw		mm4,	mm5					/* mm4 = sum+x1 */				
                psraw		mm4,	3					/* mm4 >>=3 */					
                
                psubw		mm4,	mm5					/* New Value - old Value */		
                pand		mm4,	mm7					/* And the flag */				
                
                paddw		mm4,	mm5					/* add the old value back */	
                movq		[esi],	mm4					/* Write new x1 */				
                
                /* sum += x5 -p1 */														
                /* Des[-w3]=((sum+x2)>>3 */									
                
                movq		mm5,	[edi+32]			/* mm5= x2 */					
                psubw		mm3,	mm1					/* sum=sum-p1 */				
                
                paddw		mm3,    [edi+80]			/* sum=sum+x5 */				
                movq		mm4,	mm5					/* copy sum */					
                
                paddw		mm4,	mm3					/* mm4=sum+x2 */				
                psraw		mm4,	3					/* mm4=((sum+x2)<<1-x5+x6)>>4 */
                psubw		mm4,	mm5					/* new value - old value	*/	
                
                pand		mm4,	mm7					/* And the flag */				
                paddw		mm4,	mm5					/* add the old value back */	
                
                movq		[esi+16], mm4				/* write new x2 */				
                
                /* sum += x6 - p1 */													
                /* Des[-w2]=((sum+x[3])>>3 */								
                
                movq		mm5,	[edi+48]			/* mm5= x3 */					
                psubw		mm3,	mm1					/* sum=sum-p1 */				
                
                paddw		mm3,    [edi+96]			/* sum=sum+x6 */				
                movq		mm4,	mm5					/* copy x3 */					
                
                paddw		mm4,	mm3					/* mm4=sum+x3 */				
                psraw		mm4,	3					/* mm4=((sum+x3)<<1-x6+x7)>>4 */
                
                psubw		mm4,	mm5					/* new value - old value	*/	
                pand		mm4,	mm7					/* And the flag */				
                
                paddw		mm4,	mm5					/* add the old value back */	
                movq		[esi+32], mm4				/* write new x3 */				
                
                /* sum += x7 - p1 */													
                /* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
                
                movq		mm5,	[edi+64]			/* mm5 = x4 */					
                psubw		mm3,	mm1					/* sum = sum-p1 */				
                
                paddw		mm3,	[edi+112]			/* sum = sum+x7 */				
                movq		mm4,	mm5					/* mm4 = x4 */					
                
                paddw		mm4,	mm3					/* mm4 = sum + x4 */			
                movq        mm5,    LoopFilteredValuesUp/* Read the loopfiltered value of x4 */
                
                psraw		mm4,	3					/* >>=4 */						
                psubw		mm4,	mm5					/* -=x4 */						
                
                pand		mm4,	mm7					/* and flag */					
                paddw		mm4,	mm5					/* += x4 */						
                
                movq		[esi+48], mm4				/* write new x4 */				
                
                /* sum+= x8-x1 */														
                /* Des[0]=((sum+x5)>>3 */								
                
                movq		mm5,	[edi+80]			/* mm5 = x5 */					
                psubw		mm3,	[edi+16]			/* sum -= x1 */					
                
                paddw		mm3,	[edi+128]			/* sub += x8 */					
                movq		mm4,	mm5					/* mm4 = x5 */					
                
                paddw		mm4,	mm3					/* mm4= sum+x5 */				
                movq        mm5,    LoopFilteredValuesDown/* Read the loopfiltered value of x4 */
                
                psraw		mm4,	3					/* >>=4 */						
                psubw		mm4,	mm5					/* -=x5 */						
                
                pand		mm4,	mm7					/* and flag */					
                paddw		mm4,	mm5					/* += x5 */										
                
                movq		[esi+64], mm4				/* write new x5 */				
                
                /* sum += p2 - x2 */													
                /* Des[w1] = ((sum+x6)>>3 */								
                
                movq		mm5,	[edi+96]			/* mm5 = x6 */					
                psubw		mm3,	[edi+32]			/* -= x2 */						
                
                paddw		mm3,	mm2					/* += p2 */						
                movq		mm4,	mm5					/* mm4 = x6 */					
                
                paddw		mm4,	mm3					/* mm4 = sum+x6 */				
                psraw		mm4,	3					/* >>=3 */						
                
                psubw		mm4,	mm5					/* -=x6 */						
                pand		mm4,	mm7					/* and flag */					
                
                paddw		mm4,	mm5					/* += x6 */						
                movq		[esi+80], mm4				/* write new x6 */				
                
                /* sum += p2 - x3 */													
                /* Des[w2] = (sum+x7)>>3 */								
                
                movq		mm5,	[edi+112]			/* mm5 = x7 */					
                psubw		mm3,	[edi+48]			/* -= x3 */						
                
                paddw		mm3,	mm2					/* += p2 */						
                movq		mm4,	mm5					/* mm4 = x7 */					
                
                paddw		mm4,	mm3					/* mm4 = sum+x7 */				
                psraw		mm4,	3					/* >>=3 */						
                
                psubw		mm4,	mm5					/* -=x7 */						
                pand		mm4,	mm7					/* and flag */					
                
                paddw		mm4,	mm5					/* += x7 */						
                movq		[esi+96], mm4				/* write new x7 */				
                
                /* sum += p2 - x4 */													
                /* Des[w3] = ((sum+x8)>>3 */								
                
                movq		mm5,	[edi+128]			/* mm5 = x8 */					
                psubw		mm3,	[edi+64]			/* -= x4 */						
                
                paddw		mm3,	mm2					/* += p2 */						
                movq		mm4,	mm5					/* mm4 = x8 */					
                
                paddw		mm4,	mm3					/* mm4 = sum+x8 */				
                psraw		mm4,	3					/* >>=3 */						
                
                psubw		mm4,	mm5					/* -=x8 */						
                pand		mm4,	mm7					/* and flag */					
                
                paddw		mm4,	mm5					/* += x8 */						
                movq		[esi+112], mm4				/* write new x8 */				
                
                /* done with left four columns */										
                /* now do the righ four columns */										
				add			edi,	8					/* shift to right four column */
				add			esi,	8					/* shift to right four column */
				
				/* mm0 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* mm0 now are in use  */										
                /* find the loop filtered values for the pixels on block boundary */
                movq        mm1,    LoopFLimitMmx;      /* Get the Flimit values for loop filter */
                movq        mm3,    [edi + 48]          /* mm3 = x3 = p[-2] */

                movq        mm4,    [edi + 64]          /* mm4 = x4 = p[-1] */
                movq        mm5,    [edi + 80]          /* mm5 = x5 = p[ 0] */

                movq        mm6,    [edi + 96]          /* mm6 = x6 = p[ 1] */
                psubw       mm5,    mm4                 /* mm5 = p[ 0] - p[-1] */

                psubw       mm3,    mm6                 /* mm3 = p[-2] - p[ 1] */
                movq        mm4,    mm5                 /* make a copy */

                paddw       mm4,    mm5                 /* 2 * ( p[0] - p[-1] ) */
                paddw       mm3,    FourFours           /* mm3 + 4 */

                paddw       mm5,    mm4                 /* 3 * ( p[0] - p[-1] ) */
                paddw       mm3,    mm5                 /* Filtval before shift */

                psraw       mm3,    3                   /* FiltVal */
                movq        mm2,    mm3                 /* make a copy */

                psraw       mm3,    15                  /* FFFF->Neg, 0000->Pos */
                pxor        mm2,    mm3

                psubsw      mm2,    mm3                 /* mm2 = abs(FiltVal) */
                por         mm3,    FourOnes            /* -1 and 1 for + and - */

                movq        mm4,    mm1                 /* make a copy of Flimit */
                psubw       mm1,    mm2                 /* mm1= Flimit - abs(FiltVal) */

                movq        mm5,    mm1                 /* copy Flimit - abs(FiltVal) */
                psraw       mm1,    15                  /* FFFF or 0000 */

                pxor        mm5,    mm1                 
                psubsw      mm5,    mm1                 /* abs(Flimit - abs(FiltVal)) */

                psubusw     mm4,    mm5                 /* Flimit-abs(Flimit - abs(FiltVal)) */
                pmullw      mm4,    mm3                 /* get the sign back */

                movq        mm1,    [edi+64]            /* p[-1] */
                movq        mm2,    [edi+80]            /* p[0] */
                
                paddw       mm1,    mm4                 /* p[-1] + NewFiltVal */
                psubw       mm2,    mm4                 /* p[0] - NewFiltVal */

                pxor        mm6,    mm6                 /* clear mm6 */
                
                packuswb    mm1,    mm1                 /* clamping */
                packuswb    mm2,    mm2                 /* clamping */

                punpcklbw   mm1,    mm6                 /* unpack to word */
                movq        LoopFilteredValuesUp, mm1   /* save the values */

                punpcklbw   mm2,    mm6                 /* unpack to word */
                movq        LoopFilteredValuesDown, mm2 /* save the values */
                
                
                /* Let's do the filtering now */										
                /* p1 = Src[-5] */		
                /* p2 = Src[+4] */		
                /* sum = p1 + p1 + p1 + x1 + x2 + x3 + x4 + 4; */				
                
                movq		mm3,	[edi]			    /* mm3 = [-5] */
                movq		mm2,	[edi+144]			/* mm2 = [4] */					
                
                movq		mm1,	mm3					/* p1 = [-4] */					
                paddw		mm3,	mm3					/* mm3 = p1 + p1 */				
                
                movq		mm4,	[edi+16]			/* mm4 = x1 */					
                paddw		mm3,	mm1					/* mm3 = p1 + p1 + p1 */		
                
                paddw		mm3,	[edi+32]			/* mm3 = p1+p1+p1+ x2 */		
                paddw		mm4,	[edi+48]			/* mm4 = x1+x3 */				
                
                paddw		mm3,	[edi+64]			/* mm3 += x4 */					
                paddw		mm4,	FourFours			/* mm4 = x1 + x3 + 4 */			
                
                paddw		mm3,	mm4					/* mm3 = 3*p1+x1+x2+x3+x4+4 */	
                
                /* Des[-w4] = (((sum + x1) >> 3; */			
                /* Des[-w4] = Src[-w4]; */												
                /* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	
                
                movq		mm4,	mm3					/* mm4 = mm3 */					
                movq		mm5,	[edi+16]			/* mm5 = x1 */					
                
                paddw		mm4,	mm5					/* mm4 = sum+x1 */				
                psraw		mm4,	3					/* mm4 >>=4 */					
                
                psubw		mm4,	mm5					/* New Value - old Value */		
                pand		mm4,	mm0					/* And the flag */				
                
                paddw		mm4,	mm5					/* add the old value back */	
                movq		[esi],	mm4					/* Write new x1 */				
                
                /* sum += x5 -p1 */														
                /* Des[-w3]=((sum+x2)>>3 */									
                
                movq		mm5,	[edi+32]			/* mm5= x2 */					
                psubw		mm3,	mm1					/* sum=sum-p1 */				
                
                paddw		mm3,    [edi+80]			/* sum=sum+x5 */				
                movq		mm4,	mm5					/* copy sum */					
                
                paddw		mm4,	mm3					/* mm4=sum+x2 */				
                psraw		mm4,	3					/* mm4=((sum+x2)<<1-x5+x6)>>4 */
                psubw		mm4,	mm5					/* new value - old value	*/	
                
                pand		mm4,	mm0					/* And the flag */				
                paddw		mm4,	mm5					/* add the old value back */	
                
                movq		[esi+16], mm4				/* write new x2 */				
                
                /* sum += x6 - p1 */													
                /* Des[-w2]=((sum+x[3])>>3 */								
                
                movq		mm5,	[edi+48]			/* mm5= x3 */					
                psubw		mm3,	mm1					/* sum=sum-p1 */				
                
                paddw		mm3,    [edi+96]			/* sum=sum+x6 */				
                movq		mm4,	mm5					/* copy x3 */					
                
                paddw		mm4,	mm3					/* mm4=sum+x3 */				
                psraw		mm4,	3					/* mm4=((sum+x3)<<1-x6+x7)>>4 */
                
                psubw		mm4,	mm5					/* new value - old value	*/	
                pand		mm4,	mm0					/* And the flag */				
                
                paddw		mm4,	mm5					/* add the old value back */	
                movq		[esi+32], mm4				/* write new x3 */				
                
                /* sum += x7 - p1 */													
                /* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
                
                movq		mm5,	[edi+64]			/* mm5 = x4 */					
                psubw		mm3,	mm1					/* sum = sum-p1 */				
                
                paddw		mm3,	[edi+112]			/* sum = sum+x7 */				
                movq		mm4,	mm5					/* mm4 = x4 */					
                
                paddw		mm4,	mm3					/* mm4 = sum + x4 */			
                movq        mm5,    LoopFilteredValuesUp/* Read the loopfiltered value of x4 */
                
                psraw		mm4,	3					/* >>=4 */						
                psubw		mm4,	mm5					/* -=x4 */						
                
                pand		mm4,	mm0					/* and flag */					
                paddw		mm4,	mm5					/* += x4 */						
                
                movq		[esi+48], mm4				/* write new x4 */				
                
                /* sum+= x8-x1 */														
                /* Des[0]=((sum+x5)>>3 */								
                
                movq		mm5,	[edi+80]			/* mm5 = x5 */					
                psubw		mm3,	[edi+16]			/* sum -= x1 */					
                
                paddw		mm3,	[edi+128]			/* sub += x8 */					
                movq		mm4,	mm5					/* mm4 = x5 */					
                
                paddw		mm4,	mm3					/* mm4= sum+x5 */				
                movq        mm5,    LoopFilteredValuesDown/* Read the loopfiltered value of x4 */
                
                psraw		mm4,	3					/* >>=4 */						
                psubw		mm4,	mm5					/* -=x5 */						
                
                pand		mm4,	mm0					/* and flag */					
                paddw		mm4,	mm5					/* += x5 */										
                
                movq		[esi+64], mm4				/* write new x5 */				
                
                /* sum += p2 - x2 */													
                /* Des[w1] = ((sum+x6)>>3 */								
                
                movq		mm5,	[edi+96]			/* mm5 = x6 */					
                psubw		mm3,	[edi+32]			/* -= x2 */						
                
                paddw		mm3,	mm2					/* += p2 */						
                movq		mm4,	mm5					/* mm4 = x6 */					
                
                paddw		mm4,	mm3					/* mm4 = sum+x6 */				
                psraw		mm4,	3					/* >>=3 */						
                
                psubw		mm4,	mm5					/* -=x6 */						
                pand		mm4,	mm0					/* and flag */					
                
                paddw		mm4,	mm5					/* += x6 */						
                movq		[esi+80], mm4				/* write new x6 */				
                
                /* sum += p2 - x3 */													
                /* Des[w2] = (sum+x7)>>3 */								
                
                movq		mm5,	[edi+112]			/* mm5 = x7 */					
                psubw		mm3,	[edi+48]			/* -= x3 */						
                
                paddw		mm3,	mm2					/* += p2 */						
                movq		mm4,	mm5					/* mm4 = x7 */					
                
                paddw		mm4,	mm3					/* mm4 = sum+x7 */				
                psraw		mm4,	3					/* >>=3 */						
                
                psubw		mm4,	mm5					/* -=x7 */						
                pand		mm4,	mm0					/* and flag */					
                
                paddw		mm4,	mm5					/* += x7 */						
                movq		[esi+96], mm4				/* write new x7 */				
                
                /* sum += p2 - x4 */													
                /* Des[w3] = ((sum+x8)>>3 */								
                
                movq		mm5,	[edi+128]			/* mm5 = x8 */					
                psubw		mm3,	[edi+64]			/* -= x4 */						
                
                paddw		mm3,	mm2					/* += p2 */						
                movq		mm4,	mm5					/* mm4 = x8 */					
                
                paddw		mm4,	mm3					/* mm4 = sum+x8 */				
                psraw		mm4,	3					/* >>=3 */						
                
                psubw		mm4,	mm5					/* -=x8 */						
                pand		mm4,	mm0					/* and flag */					
                
                paddw		mm4,	mm5					/* += x8 */						
                movq		[esi+112], mm4				/* write new x8 */				
				
				/* done with right four column */	
				/* transpose */
				mov			eax,	Des					/* the destination */			
				add			edi,	8					/* shift edi to point x1 */

				sub			esi,	8					/* shift esi back to left x1 */
				sub			eax,	4

				movq		mm0,	[esi]				/* mm0 = 30 20 10 00 */
				movq		mm1,	[esi+16]			/* mm1 = 31 21 11 01 */

				movq		mm4,	mm0					/* mm4 = 30 20 10 00 */
				punpcklwd	mm0,	mm1					/* mm0 = 11 10 01 00 */

				punpckhwd	mm4,	mm1					/* mm4 = 31 30 21 20 */
				movq		mm2,	[esi+32]			/* mm2 = 32 22 12 02 */

				movq		mm3,	[esi+48]			/* mm3 = 33 23 13 03 */
				movq		mm5,	mm2					/* mm5 = 32 22 12 02 */

				punpcklwd	mm2,	mm3					/* mm2 = 13 12 03 02 */
				punpckhwd	mm5,	mm3					/* mm5 = 33 32 23 22 */

				movq		mm1,	mm0					/* mm1 = 11 10 01 00 */
				punpckldq	mm0,	mm2					/* mm0 = 03 02 01 00 */

				movq		[edi],	mm0					/* write 00 01 02 03 */
				punpckhdq	mm1,	mm2					/* mm1 = 13 12 11 10 */
				
				movq		mm0,	mm4					/* mm0 = 31 30 21 20 */
				movq		[edi+16], mm1				/* write 10 11 12 13 */

				punpckldq	mm0,	mm5					/* mm0 = 23 22 21 20 */
				punpckhdq	mm4,	mm5					/* mm4 = 33 32 31 30 */

				movq		mm1,	[esi+64]			/* mm1 = 34 24 14 04 */
				movq		mm2,	[esi+80]			/* mm2 = 35 25 15 05 */				

				movq		mm5,	[esi+96]			/* mm5 = 36 26 16 06 */
				movq		mm6,	[esi+112]			/* mm6 = 37 27 17 07 */
								
				movq		mm3,	mm1					/* mm3 = 34 24 14 04 */
				movq		mm7,	mm5					/* mm7 = 36 26 16 06 */

				punpcklwd	mm1,	mm2					/* mm1 = 15 14 05 04 */
				punpckhwd	mm3,	mm2					/* mm3 = 35 34 25 24 */

				punpcklwd	mm5,	mm6					/* mm5 = 17 16 07 06 */
				punpckhwd	mm7,	mm6					/* mm7 = 37 36 27 26 */

				movq		mm2,	mm1					/* mm2 = 15 14 05 04 */
				movq		mm6,	mm3					/* mm6 = 35 34 25 24 */

				punpckldq	mm1,	mm5					/* mm1 = 07 06 05 04 */
				punpckhdq	mm2,	mm5					/* mm2 = 17 16 15 14 */

				punpckldq	mm3,	mm7					/* mm3 = 27 26 25 24 */
				punpckhdq	mm6,	mm7					/* mm6 = 37 36 35 34 */
			
				movq		mm5,	[edi]				/* mm5 = 03 02 01 00 */
				packuswb	mm5,	mm1					/* mm5 = 07 06 05 04 03 02 01 00 */
				
				movq		[eax],	mm5					/* write 00 01 02 03 04 05 06 07 */
				movq		mm7,	[edi+16]			/* mm7 = 13 12 11 10 */

				packuswb	mm7,	mm2					/* mm7 = 17 16 15 14 13 12 11 10 */
				movq		[eax+ecx], mm7				/* write 10 11 12 13 14 15 16 17 */

				packuswb	mm0,	mm3					/* mm0 = 27 26 25 24 23 22 21 20 */
				packuswb	mm4,	mm6					/* mm4 = 37 36 35 34 33 32 31 30 */
				
				movq		[eax+ecx*2], mm0			/* write 20 21 22 23 24 25 26 27 */
				lea			eax,	[eax+ecx*4]			/* mov forward the desPtr */

				movq		[eax+edx],	mm4				/* write 30 31 32 33 34 35 36 37 */
				add			edi, 8						/* move to right four column */
				add			esi, 8						/* move to right x1 */

				movq		mm0,	[esi]				/* mm0 = 70 60 50 40 */
				movq		mm1,	[esi+16]			/* mm1 = 71 61 51 41 */

				movq		mm4,	mm0					/* mm4 = 70 60 50 40 */
				punpcklwd	mm0,	mm1					/* mm0 = 51 50 41 40 */

				punpckhwd	mm4,	mm1					/* mm4 = 71 70 61 60 */
				movq		mm2,	[esi+32]			/* mm2 = 72 62 52 42 */

				movq		mm3,	[esi+48]			/* mm3 = 73 63 53 43 */
				movq		mm5,	mm2					/* mm5 = 72 62 52 42 */

				punpcklwd	mm2,	mm3					/* mm2 = 53 52 43 42 */
				punpckhwd	mm5,	mm3					/* mm5 = 73 72 63 62 */

				movq		mm1,	mm0					/* mm1 = 51 50 41 40 */
				punpckldq	mm0,	mm2					/* mm0 = 43 42 41 40 */

				movq		[edi],	mm0					/* write 40 41 42 43 */
				punpckhdq	mm1,	mm2					/* mm1 = 53 52 51 50 */
				
				movq		mm0,	mm4					/* mm0 = 71 70 61 60 */
				movq		[edi+16], mm1				/* write 50 51 52 53 */

				punpckldq	mm0,	mm5					/* mm0 = 63 62 61 60 */
				punpckhdq	mm4,	mm5					/* mm4 = 73 72 71 70 */

				movq		mm1,	[esi+64]			/* mm1 = 74 64 54 44 */
				movq		mm2,	[esi+80]			/* mm2 = 75 65 55 45 */				

				movq		mm5,	[esi+96]			/* mm5 = 76 66 56 46 */
				movq		mm6,	[esi+112]			/* mm6 = 77 67 57 47 */
								
				movq		mm3,	mm1					/* mm3 = 74 64 54 44 */
				movq		mm7,	mm5					/* mm7 = 76 66 56 46 */

				punpcklwd	mm1,	mm2					/* mm1 = 55 54 45 44 */
				punpckhwd	mm3,	mm2					/* mm3 = 75 74 65 64 */

				punpcklwd	mm5,	mm6					/* mm5 = 57 56 47 46 */
				punpckhwd	mm7,	mm6					/* mm7 = 77 76 67 66 */

				movq		mm2,	mm1					/* mm2 = 55 54 45 44 */
				movq		mm6,	mm3					/* mm6 = 75 74 65 64 */

				punpckldq	mm1,	mm5					/* mm1 = 47 46 45 44 */
				punpckhdq	mm2,	mm5					/* mm2 = 57 56 55 54 */

				punpckldq	mm3,	mm7					/* mm3 = 67 66 65 64 */
				punpckhdq	mm6,	mm7					/* mm6 = 77 76 75 74 */
			
				movq		mm5,	[edi]				/* mm5 = 43 42 41 40 */
				packuswb	mm5,	mm1					/* mm5 = 47 46 45 44 43 42 41 40 */
				
				movq		[eax],	mm5					/* write 40 41 42 43 44 45 46 47 */
				movq		mm7,	[edi+16]			/* mm7 = 53 52 51 50 */

				packuswb	mm7,	mm2					/* mm7 = 57 56 55 54 53 52 51 50 */
				movq		[eax+ecx], mm7				/* write 50 51 52 53 54 55 56 57 */

				packuswb	mm0,	mm3					/* mm0 = 67 66 65 64 63 62 61 60 */
				packuswb	mm4,	mm6					/* mm4 = 77 76 75 74 73 72 71 70 */
				
				movq		[eax+ecx*2], mm0			/* write 60 61 62 63 64 65 66 67 */
				lea			eax,	[eax+ecx*4]			/* mov forward the desPtr */

				movq		[eax+edx],	mm4				/* write 70 71 72 73 74 75 76 77 */
				
				pop			edi
				pop			esi
				pop			edx
				pop			ecx
				pop			ebp
				pop			eax
			}//__asm	
		Var1 = Variance11[0]+ Variance11[1]+Variance11[2]+Variance11[3];
		Var1 += Variance11[4]+ Variance11[5]+Variance11[6]+Variance11[7];
		pbi->FragmentVariances[CurrentFrag-1] += Var1;

		Var2 = Variance21[0]+ Variance21[1]+Variance21[2]+Variance21[3];
		Var2 += Variance21[4]+ Variance21[5]+Variance21[6]+Variance21[7];
		pbi->FragmentVariances[CurrentFrag] += Var2;


        CurrentFrag ++;
		}//else
			
	}//while

}


/****************************************************************************
 * 
 *  ROUTINE       : PlaneAddNoise_mmx
 *
 *  INPUTS        : UINT8 *Start    starting address of buffer to add gaussian
 *                                  noise to
 *                  UINT32 Width    width of plane
 *                  UINT32 Height   height of plane
 *                  INT32  Pitch    distance between subsequent lines of frame
 *                  INT32  q        quantizer used to determine amount of noise 
 *                                  to add
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void.   
 * 
 *  FUNCTION      : adds gaussian noise to a plane of pixels
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void PlaneAddNoise_mmx( UINT8 *Start, UINT32 Width, UINT32 Height, INT32 Pitch, int q)
{
    unsigned int i;
    INT32 Pitch4 = Pitch * 4;
    const int noiseAmount = 2;
    const int noiseAdder = 2 * noiseAmount + 1;

#if defined(_WIN32_WCE)
#pragma pack(16)
	unsigned char blackclamp[16];
	unsigned char whiteclamp[16];
	unsigned char bothclamp[16];
#pragma pack()
#else
	__declspec(align(16)) unsigned char blackclamp[16];
	__declspec(align(16)) unsigned char whiteclamp[16];
	__declspec(align(16)) unsigned char bothclamp[16];
#endif 
    char CharDist[300];
    char Rand[2048] = 
    {
    -2,0,-2,2,0,0,-1,2,2,1,-2,2,1,0,-1,-2,-2,-1,-2,-2,2,0,-2,-2,-2,-1,0,0,1,1,-2,1,0,-1,-2,1,1,2,0,-1,2,1,2,2,0,-2,0,-1,2,-1,1,2,2,2,1,-1,-1,-1,2,-2,-1,-2,1,-2,-2,2,-1,-1,0,1,2,1,0,-1,1,0,0,2,1,-2,0,-1,1,1,0,-1,-2,-1,0,2,0,2,1,-1,-2,1,0,-2,1,0,-2,2,-2,2,1,-1,0,-2,2,1,-2,2,2,0,-2,-2,2,0,-2,0,1,0,-1,0,1,1,1,0,-2,-1,2,-2,0,1,0,-2,2,2,0,-1,0,-1,2,-1,0,-1,2,-1,1,0,-2,1,2,-1,0,2,-2,2,0,-2,0,-2,2,1,1,-2,2,-2,-2,1,-1,2,-1,-1,-2,1,2,1,1,1,-1,-2,-2,-2,2,2,-1,-2,0,-2,-2,0,1,1,0,-2,0,-1,1,-1,0,-1,0,0,1,-2,0,2,1,2,-2,-1,-2,2,0,2,-2,1,-2,0,2,-2,2,-1,-1,1,0,-1,1,1,0,0,0,1,2,2,1,1,0,-1,-2,1,0,2,-1,-2,1,1,0,-1,0,-2,1,1,1,1,2,-2,0,2,2,1,1,-2,1,2,-1,0,-1,-2,-2,2,2,1,-2,-1,-2,-2,1,2,0,0,0,-1,0,0,-2,-1,1,-1,2,2,2,1,-1,2,-2,-2,1,0,1,2,-2,2,1,-1,-2,0,-1,-1,2,0,1,-2,0,-1,0,1,0,-1,1,0,1,-1,-2,1,-2,1,2,0,1,0,-1,1,0,-1,2,1,-2,-1,-2,1,2,1,-2,-1,-2,1,-2,2,2,0,1,2,-2,-2,1,1,-1,-2,-2,1,-1,-1,-1,1,2,2,0,1,1,2,-2,1,0,-1,-2,2,-2,0,0,-1,0,-1,-1,-2,2,-2,-1,1,2,1,1,1,-1,2,-1,2,-1,-1,0,2,-2,-2,0,0,-2,-1,2,-1,-2,-2,2,-2,-2,-2,-1,2,-1,0,2,2,0,2,1,-1,-1,-2,0,2,-1,-1,0,-1,1,2,0,2,-2,2,1,1,0,-2,-1,-1,-2,0,-2,1,2,-2,2,1,1,2,0,1,-2,1,1,1,-2,2,1,1,-2,0,2,-2,-1,-2,2,1,-1,2,-1,1,-1,-2,-1,0,2,-2,2,0,-2,1,-2,2,1,2,-1,0,-2,1,-2,0,-1,2,-2,-1,-2,-1,-2,1,2,2,-2,1,1,1,2,0,2,1,-2,1,0,0,2,0,0,0,-1,-1,-1,-2,1,-2,-2,-1,0,-2,
    -2,-2,1,0,1,1,0,1,-1,2,0,-2,2,2,-1,2,-2,2,0,0,1,1,-2,-1,-1,0,2,1,1,2,-1,-1,2,-1,-1,0,-1,1,1,1,1,-2,-1,-1,1,2,-1,0,-2,2,-1,0,1,0,1,-2,-2,-2,-2,-1,-1,1,-2,-1,-2,1,1,-2,1,1,1,0,-2,0,-2,2,0,2,1,0,1,1,-1,-1,-2,2,-2,-2,-1,1,-1,-1,0,-2,0,0,1,1,0,-1,2,2,1,2,-2,0,2,-1,-1,-1,-2,1,-1,-2,-2,0,2,2,0,1,1,2,2,0,0,-2,1,0,0,0,0,2,1,-1,-2,-1,-1,-1,1,-1,2,-2,1,1,2,-2,0,2,1,2,-2,2,1,2,2,2,1,-2,1,-1,-1,1,1,-2,1,0,-2,2,2,-2,-1,0,0,1,-2,1,2,-2,1,1,-2,-2,-1,1,2,0,-1,1,-1,1,-1,-1,2,-1,-2,1,-2,-2,-2,-1,1,-1,0,0,-2,0,1,-1,1,2,0,0,-2,0,-1,0,2,0,-2,0,1,1,2,2,-1,2,1,1,2,1,2,2,2,0,0,-2,-1,2,0,-2,-2,1,1,-2,-2,-1,1,2,-2,-2,-2,-1,-2,2,1,-2,2,1,0,-2,-1,-1,1,1,-2,2,-2,1,0,2,0,-1,-1,1,-1,0,1,-2,2,1,-2,0,1,2,1,1,1,2,1,-1,0,-1,0,1,-1,0,0,2,1,1,1,0,1,1,2,-1,1,2,0,2,0,0,0,2,2,-2,-1,-1,1,2,1,-2,1,-2,0,0,0,-2,2,-2,1,-2,-2,1,-1,-1,1,0,0,-1,1,-2,0,0,2,0,-2,-1,-1,-2,2,1,2,1,1,0,1,1,2,0,-1,-2,2,2,0,-2,2,1,-2,0,2,-2,-2,-1,-2,0,-2,1,0,1,1,2,1,-1,2,-1,2,1,-1,-2,-1,-2,0,-2,2,-2,-1,-1,-2,-2,-2,1,1,2,-2,0,0,2,0,0,1,-1,0,-2,2,2,2,-2,0,1,1,1,-1,2,1,-2,0,-2,0,1,1,-2,1,0,2,2,1,-1,-1,0,-2,1,-2,1,1,-1,-2,-2,1,-2,-1,1,1,0,2,1,-1,0,2,-2,-2,-2,-2,2,-1,-1,2,-2,2,-1,2,-1,-1,-1,-1,2,2,2,2,1,-2,-2,-2,-1,0,-2,2,1,0,2,0,1,2,2,2,2,-2,-1,-1,-2,2,1,1,-2,1,2,1,2,-2,1,-1,1,2,2,-2,1,0,-2,-1,0,-2,2,0,-1,1,2,-1,-2,1,-1,0,2,2,-1,0,2,2,1,
    -1,2,-1,-1,-2,0,-1,-2,-1,2,-1,2,-2,2,2,0,-1,1,0,1,0,-2,2,-2,-1,-1,1,0,2,1,1,0,2,1,-2,0,-2,-2,1,-1,2,0,1,-2,1,-2,1,2,0,1,-1,2,1,0,-1,2,0,1,-1,-2,0,1,0,-1,-2,-1,0,2,0,2,-1,0,-2,2,2,0,1,-1,1,0,0,-2,-1,-1,2,2,2,1,0,-2,0,-1,0,-2,2,-1,1,2,0,-1,-1,0,2,-1,-1,1,2,-1,-2,0,2,0,-2,2,-2,1,-1,-2,-2,-1,0,2,-2,-2,-1,-1,0,0,0,2,1,-1,0,0,2,0,2,1,2,0,2,-1,2,-1,2,1,-2,1,0,-2,-2,-2,0,2,-2,-2,-1,2,1,1,1,-1,1,2,2,-1,0,-2,-2,-2,-1,1,0,-2,-1,-2,1,-2,-2,0,-1,2,-2,2,-2,-2,-2,2,-1,0,-1,0,1,2,2,2,-2,-2,0,2,2,-2,2,2,-1,0,1,0,-1,2,2,1,0,-1,-2,-2,1,0,-1,-1,0,1,2,1,2,-1,0,-1,2,0,-1,0,0,-1,-1,-2,-1,-1,2,1,2,1,1,-1,1,-2,1,2,-1,-2,0,-2,2,1,0,1,0,1,1,1,1,2,-2,0,1,-2,0,-2,0,-1,-2,-1,2,0,1,-2,-1,2,2,-1,-1,-1,-2,2,-2,-2,-1,-1,1,1,-2,-1,-2,-1,0,-2,1,-2,0,1,-1,-2,-1,1,2,0,2,-2,1,2,1,1,0,0,-2,2,-1,-2,-1,-1,0,1,-1,2,-1,1,-1,-2,1,-1,-1,1,2,-1,2,-1,2,1,-1,-1,-1,0,-1,-1,-2,-2,1,2,1,2,-2,0,1,2,-1,1,1,2,2,2,1,-1,1,-2,0,1,-1,2,-2,0,-2,1,-1,-2,-1,-2,2,1,-2,0,-2,2,-2,0,2,0,2,0,0,0,1,2,2,-1,-2,1,-2,1,0,2,1,-1,0,-1,1,2,-2,-2,-1,-1,-1,2,2,-1,-2,0,0,2,0,-1,0,-1,0,2,-1,-1,2,0,0,1,1,-2,-2,-1,-2,-1,0,1,-1,-2,1,-2,-1,2,0,2,-1,-2,0,-1,-2,0,1,-2,2,-1,2,0,-1,-1,0,-1,0,1,2,-1,0,1,1,-2,-2,1,2,1,-1,0,-2,0,-2,-1,2,-1,-1,-2,-1,-2,-1,-1,-2,-1,-2,0,2,2,0,2,-2,0,0,1,-1,2,-1,-1,2,2,1,1,-2,-1,-1,2,2,0,1,-1,2,0,-2,2,-2,-1,-1,1,0,0,-2,
    2,-2,-2,2,0,1,-2,-2,0,1,0,2,2,-1,0,2,-2,2,0,-1,-2,-1,-2,-2,-2,2,0,1,-1,1,1,2,2,2,-1,-2,-2,2,-2,2,-1,2,-1,-1,1,2,-1,0,1,-1,0,0,2,1,1,0,2,0,-1,-1,-2,2,1,-1,-1,-1,-1,-2,2,-1,0,-2,2,1,1,-2,0,1,0,1,2,-2,-1,2,1,-2,2,-2,1,-2,-2,-2,0,0,0,-1,-2,-1,-2,0,-2,-1
    };

    double sigma;
    __asm emms
    sigma = 1 + .8*(63-q) / 63.0;

    // set up a lookup table of 256 entries that matches 
    // a gaussian distribution with sigma determined by q.
    // 
    {
        double i,sum=0;
        int next,j;

        next=0;
        for(i=-32;i<32;i++)
        {
            int a = (int)(.5+256*gaussian(sigma,0,i));

            if(a)
            {
                for(j=0;j<a;j++)
                {
                    CharDist[next+j]=(char) i;
                }
                next = next+j;
            }

        }
        for(next=next;next<256;next++)
            CharDist[next] = 0;

    }

    for(i=0;i<2048;i++)
    {
        Rand[i]=CharDist[rand() & 0xff];
    }

	for(i=0;i<16;i++)
	{
		blackclamp[i]=-CharDist[0];
		whiteclamp[i]=-CharDist[0];
		bothclamp[i]=-2*CharDist[0];
	}

    for(i=0;i<Height;i++)
    {
        UINT8 *Pos = Start + i *Pitch;
        INT8  *Ref = Rand + (rand() & 0xff);

        __asm
        {
			mov ecx, [Width]
            mov esi,Pos
            mov edi,Ref
			xor		    eax,eax

    		nextset:
            movq        mm1,[esi+eax]         // get the source

			psubusb     mm1,blackclamp        // clamp both sides so we don't outrange adding noise
			paddusb     mm1,bothclamp          
			psubusb     mm1,whiteclamp

            movq        mm2,[edi+eax]         // get the noise for this line
            paddb       mm1,mm2              // add it in 
            movq        [esi+eax],mm1         // store the result

            add         eax,8                 // move to the next line

			cmp         eax, ecx
			jl			nextset


        }

    }
}
