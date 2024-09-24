/****************************************************************************
 *
 *   Module Title :     DeblockwmtOpt.c
 *
 *   Description  :     Optimized functions for deblocking 
 *
 *   AUTHOR       :     Yaowu Xu
 *
 *****************************************************************************
 *  Revision History
 *
 *  1.02 YWX 08-Dec-00 Configuration baseline from deblockopt.c
 *
 *****************************************************************************
 */
 

/****************************************************************************
 *  Header Frames
 *****************************************************************************
 */



#include "postp.h"
#include "stdlib.h"
#include <math.h>

/****************************************************************************
 *  Module constants.
 *****************************************************************************
 */        

#if defined(_WIN32_WCE)
#else
__declspec(align(16)) static short Eight128s[] = {128, 128, 128, 128,128, 128, 128, 128 };
__declspec(align(16)) static short Eight64s[] = {64, 64, 64, 64, 64, 64, 64, 64  };
__declspec(align(16)) static short EightThrees[]= {3, 3, 3, 3, 3, 3, 3, 3};
__declspec(align(16)) static short EightFours[]= {4, 4, 4, 4, 4, 4, 4, 4};
__declspec(align(16)) static short Four128s[] = {128, 128, 128, 128};
__declspec(align(16)) static short Four64s[] = {64, 64, 64, 64 };
__declspec(align(16)) static short FourThrees[]= {3, 3, 3, 3};
__declspec(align(16)) static short FourFours[]= {4, 4, 4, 4};
__declspec(align(16)) static short EightOnes[]= { 1, 1, 1, 1, 1, 1, 1, 1};
#endif

/****************************************************************************
 *  Explicit Imports
 *****************************************************************************
 */              

extern double gaussian(double sigma, double mu, double x);
extern UINT32 *DeblockLimitValuesV2;

/****************************************************************************
 *  Exported Global Variables
 *****************************************************************************
 */

/****************************************************************************
 *  Exported Functions
 *****************************************************************************
 */              

/****************************************************************************
 *  Module Statics
 *****************************************************************************
 */

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

void DeblockLoopFilteredBand_WMT(
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
	return;
#else

__declspec(align(16)) short QStepWMT[8];
__declspec(align(16)) short FLimitWMT[8];
__declspec(align(16)) short Rows[80];

__declspec(align(16)) unsigned short Variance1[8];
__declspec(align(16)) unsigned short Variance2[8];


	Src=SrcPtr;
	Des=DesPtr;

	while(CurrentFrag < StartFrag + FragAcross )
    {
        
        QStep = QuantScale[ pbi->FragQIndex[CurrentFrag+FragAcross]];
        if( QStep > 3 )
        {
            QStepWMT[0] = (INT16)QStep;
            QStepWMT[1] = (INT16)QStep;
            QStepWMT[2] = (INT16)QStep;
            QStepWMT[3] = (INT16)QStep;
            QStepWMT[4] = (INT16)QStep;
            QStepWMT[5] = (INT16)QStep;
            QStepWMT[6] = (INT16)QStep;
            QStepWMT[7] = (INT16)QStep;

			__asm 
			{
				
				/* Save the registers */
				push		eax
				push		ecx			
				push		edx
				push		esi
				push		edi
				
				
				/* Calculate the FLimit and store FLimit and QStep */					
				
				movdqa		xmm0,	QStepWMT			/* xmm0 = QStep */				
				movdqa		xmm1,	EightThrees			/* mm1 = 03030303 */			

                pmullw		xmm1,	xmm0				/* mm1 = QStep * 3 */			
				pmullw		xmm1,	xmm0				/* mm1 = QStep * QStep * 3 */	
				
				psrlw		xmm1,	5					/* mm1 = FLimit */				
				movdqa		[FLimitWMT], xmm1			/* Save FLimit */				
				
				/* setup the pointers */
				mov			eax,	Src					/* eax = Src */					
				xor			edx,	edx					/* clear edx */					

				mov			esi,	Des					/* esi = Des */
				lea			edi,	Rows				/* edi = Rows */				

				mov			ecx,	PlaneLineStep		/* ecx = Pitch */								
				pxor		xmm7,	xmm7				/* Clear xmm7 */				
				
				sub			edx,	ecx					/* edx = -Pitch */								
				
				lea			eax,	[eax + edx * 4 ]	/* eax = Src - 4*Pitch */		
				lea			esi,	[esi + edx * 2 ]	/* esi = Des - 2 * Pitch */

				/* Copy the data to the intermediate buffer */							
				
				movq		xmm0,	QWORD PTR [eax + edx]/* xmm0 = Src[-5*Pitch] */		
				movq		xmm1,	QWORD PTR [eax ]	/* xmm1 = Src[-4*Pitch */
				
				punpcklbw	xmm0,	xmm7				/* expand to words */
				punpcklbw	xmm1,	xmm7				/* expand to words */

				movdqa		[edi],	xmm0				/* write 8 words */
				movdqa		[edi+16], xmm1				/* write 8 words */

				movq		xmm2,	QWORD PTR [eax+ecx]	/* xmm2 = Src[-3*Pitch] */		
				movq		xmm3,	QWORD PTR [eax+ecx*2]/* xmm3 = Src[-2*Pitch] */

				punpcklbw	xmm2,	xmm7				/* expand to words */
				punpcklbw	xmm3,	xmm7				/* expand to words */
				
				movdqa		[edi+32], xmm2				/* write 8 words */
				movdqa		[edi+48], xmm3				/* write 8 words */

				lea			eax,	[eax+ecx*4]			/* eax= Src */

				movq		xmm0,	QWORD PTR [eax + edx]/* xmm0 = Src[-Pitch] */		
				movq		xmm1,	QWORD PTR [eax ]	/* xmm1 = Src[0] */
				
				punpcklbw	xmm0,	xmm7				/* expand to words */
				punpcklbw	xmm1,	xmm7				/* expand to words */

				movdqa		[edi+64], xmm0				/* write 8 words */
				movdqa		[edi+80], xmm1				/* write 8 words */

				movq		xmm2,	QWORD PTR [eax+ecx]	/* xmm2 = Src[Pitch] */		
				movq		xmm3,	QWORD PTR [eax+ecx*2]/* xmm3 = Src[2*Pitch] */

				punpcklbw	xmm2,	xmm7				/* expand to words */
				punpcklbw	xmm3,	xmm7				/* expand to words */
				
				movdqa		[edi+96],  xmm2				/* write 8 words */
				movdqa		[edi+112], xmm3				/* write 8 words */

				lea			eax,	[eax+ecx*4]			/* eax= Src+4*Pitch */

				movq		xmm0,	QWORD PTR [eax + edx]/* xmm0 = Src[3*Pitch] */		
				movq		xmm1,	QWORD PTR [eax ]	/* xmm1 = Src[4*Pitch] */
				
				punpcklbw	xmm0,	xmm7				/* expand to words */
				punpcklbw	xmm1,	xmm7				/* expand to words */

				movdqa		[edi+128], xmm0				/* write 8 words */
				movdqa		[edi+144], xmm1				/* write 8 words */

				
				/* done with copying everything to intermediate buffer */				
				/* Now, compute the variances for Pixel  1-4 and 5-8 */					
		
				/* we use xmm0,xmm1,xmm2 for 1234 and xmm4, xmm5, xmm6 for 5-8 */				
				/* xmm7 = 0, xmm3 = {128, 128, 128, 128, 128, 128, 128, 128} */								
				
				pcmpeqw		xmm3,	xmm3				/* xmm3 = FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */	
				psllw		xmm3,	15					/* xmm3 = 80008000800080008000800080008000 */	
				psrlw		xmm3,	8					/* xmm3 = 00800080008000800080008000800080 */
				
				movdqa		xmm2,	[edi+16]			/* Pixel 1 */					
				movdqa		xmm6,	[edi+80]			/* Pixel 5 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				movdqa		xmm0,	xmm2				/* xmm0 = pixel 1 */				
				movdqa		xmm4,	xmm6				/* xmm4 = pixel 5 */				
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel1 * pixel1 */		
				pmullw		xmm6,	xmm6				/* xmm6 = pixel5 * pixel5 */		
				
				movdqa		xmm1,	xmm2				/* xmm1 = pixel1^2 */			
				movdqa		xmm5,	xmm6				/* xmm5 = pixel5^2 */			
				
				movdqa		xmm2,	[edi+32]			/* Pixel 2 */					
				movdqa		xmm6,	[edi+96]			/* Pixel 6 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				paddw		xmm0,	xmm2				/* xmm0 += pixel 2 */			
				paddw		xmm4,	xmm6				/* xmm4 += pixel 6 */			
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel2^2 */			
				pmullw		xmm6,	xmm6				/* xmm6 = pixel6^2 */			
				
				paddw		xmm1,	xmm2				/* xmm1 += pixel2^2 */			
				paddw		xmm5,	xmm6				/* xmm5 += pixel6^2 */			
				
				movdqa		xmm2,	[edi+48]			/* Pixel 3 */					
				movdqa		xmm6,	[edi+112]			/* Pixel 7 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				paddw		xmm0,	xmm2				/* xmm0 += pixel 3 */			
				paddw		xmm4,	xmm6				/* xmm4 += pixel 7 */			
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel3^2 */			
				pmullw		xmm6,	xmm6				/* xmm6 = pixel7^2 */			
				
				paddw		xmm1,	xmm2				/* xmm1 += pixel3^2 */			
				paddw		xmm5,	xmm6				/* xmm5 += pixel7^2 */			
				
				movdqa		xmm2,	[edi+64]			/* Pixel 4 */					
				movdqa		xmm6,	[edi+128]			/* Pixel 8 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				paddw		xmm0,	xmm2				/* xmm0 += pixel 4 */			
				paddw		xmm4,	xmm6				/* xmm4 += pixel 8 */			
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel4^2 */			
				pmullw		xmm6,	xmm6				/* xmm6 = pixel8^2 */			
				
				paddw		xmm1,	xmm2				/* xmm1 = pixel4^2 */			
				paddw		xmm5,	xmm6				/* xmm5 = pixel8^2 */			
				
				/* xmm0 = x1^2 + x2^2 + x3^2 + x4^2 */									
				/* xmm1 = x1 + x2 + x3 + x4 */											
				/* xmm4 = x5^2 + x6^2 + x7^2 + x8^2 */									
				/* xmm5 = x5 + x6 + x7 + x8 */											
				
				movdqa		xmm7,	xmm3				/* xmm7 = xmm3 */					
				psrlw		xmm7,	7					/* xmm7 = 00010001000100010001000100010001 */	
				
				movdqa		xmm2,	xmm0				/* make copy of sum1 */			
				movdqa		xmm6,	xmm4				/* make copy of sum2 */			
				
				paddw		xmm0,	xmm7				/* (sum1 + 1) */				
				paddw		xmm4,	xmm7				/* (sum2 + 1) */				
				
				psraw		xmm2,	1					/* sum1 /2 */					
				psraw		xmm6,	1					/* sum2 /2 */					
				
				psraw		xmm0,	1					/* (sum1 + 1)/2 */				
				psraw		xmm4,	1					/* (sum2 + 1)/2 */				
				
				pmullw		xmm2,	xmm0				/* (sum1)/2*(sum1+1)/2 */		
				pmullw		xmm6,	xmm4				/* (sum2)/2*(sum2+1)/2 */		
				
				psubw		xmm1,	xmm2				/* Variance 1 */				
				psubw		xmm5,	xmm6				/* Variance 2 */				
				
				movdqa		xmm7,	FLimitWMT			/* xmm7 = FLimit */				
				movdqa		xmm2,	xmm1				/* copy of Varinace 1*/

				movdqa		[Variance1], xmm1			/* save the varinace1 */
				movdqa		[Variance2], xmm5			/* save the varinace2 */

				movdqa		xmm6,	xmm5				/* Variance 2 */
				psubw		xmm1,	xmm7				/* Variance 1 < Flimit? */		
				
				psubw		xmm5,	xmm7				/* Variance 2 < Flimit? */		
				psraw		xmm2,	15					/* Variance 1 > 32768? */

				psraw		xmm6,	15					/* Vaiance  2 > 32768? */	
				psraw		xmm1,	15					/* FFFF/0000 for true/false */	
				
				psraw		xmm5,	15					/* FFFF/0000 for true/false */	
				movdqa		xmm7,	[edi+64]			/* xmm0 = Pixel 4			*/	

				pandn		xmm2,	xmm1				/* Variance1<32678 && 
															Variance1<Limit			*/
				pandn		xmm6,	xmm5				/* Variance2<32678 && 
														   Variance1<Limit			*/
				
				movdqa		xmm4,	[edi+80]			/* xmm4 = Pixel 5			*/	
				pand		xmm6,	xmm2				/* xmm6 = Variance1 < Flimit */	
														/*     &&Variance2 < Flimit */	

				movdqa		xmm2,	xmm7				/* make copy of Pixel4		*/	

				psubusw		xmm7,	xmm4				/* 4 - 5 */						
				psubusw		xmm4,	xmm2				/* 5 - 4 */						
				
				por			xmm7,	xmm4				/* abs(4 - 5) */				
				psubw		xmm7,	QStepWMT			/* abs(4-5)<QStepxmmx ? */		
				
				psraw		xmm7,	15					/* FFFF/0000 for True/Flase */
				pand		xmm7,	xmm6													
				
				/* xmm7 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* xmm7 now are in use  */										
				/* Let's do the filtering now */										
				/* p1 = (abs(Src[-4] - Src[-5]) < QStep ) ?  Src[-5] : Src[-4]; */		
				/* p2 = (abs(Src[+3] - Src[+4]) < QStep ) ?  Src[+4] : Src[+3]; */		
				
				movdqa		xmm5,	[edi]				/* xmm5 = -5 */					
				movdqa		xmm4,	[edi + 16]			/* xmm4 = -4 */					
				
				movdqa		xmm3,	xmm4				/* copy of -4 */				
				movdqa		xmm6,	xmm5				/* copy of -5 */				
				
				psubusw		xmm4,	xmm6				/* xmm4 = [-4] - [-5] */			
				psubusw		xmm5,	xmm3				/* xmm5 = [-5] - [-4] */			
				
				por			xmm4,	xmm5				/* abs([-4]-[-5] ) */			
				psubw		xmm4,	QStepWMT			/* abs([-4]-[-5] )<QStep? */	
				
				psraw		xmm4,	15					/* FFFF/0000 for True/False */	
				movdqa		xmm1,	xmm4				/* copy of the xmm4 */			
				
				pand		xmm4,	xmm6				/*							*/	
				pandn		xmm1,	xmm3				/*							*/	
				
				por			xmm1,	xmm4				/* xmm1 = p1				*/	
				
				/* now find P2 */														
				
				movdqa		xmm4,	[edi+128]			/* xmm4 = [3] */					
				movdqa		xmm5,	[edi+144]			/* xmm5 = [4] */					
				
				movdqa		xmm3,	xmm4				/* copy of 3 */					
				movdqa		xmm6,	xmm5				/* copy of 4 */					
				
				psubusw		xmm4,	xmm6				/* xmm4 = [3] - [4] */			
				psubusw		xmm5,	xmm3				/* xmm5 = [4] - [3] */			
				
				por			xmm4,	xmm5				/* abs([3]-[4] ) */				
				psubw		xmm4,	QStepWMT			/* abs([3]-[4] )<QStep? */		
				
				psraw		xmm4,	15					/* FFFF/0000 for True/False */	
				movdqa		xmm2,	xmm4				/* copy of the xmm4 */			
				
				pand		xmm4,	xmm6				/*							*/	
				pandn		xmm2,	xmm3				/*							*/	
				
				por			xmm2,	xmm4				/* xmm2 = p2				*/	

				/* Data is ready, now do the filtering */
				
				pxor		xmm0,	xmm0				/* clear xmm0 */

				/* sum = p1 + p1 + p1 + x1 + x2 + x3 + x4 + 4; */				
				/* Des[-w4] = (((sum + x1) << 1) - (x4 - x5)) >> 4; */			
				/* Des[-w4] = Src[-w4]; */												
				/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	

				
				movdqa		xmm3,	xmm1				/* xmm3 = p1 */					
				paddw		xmm3,	xmm3				/* xmm3 = p1 + p1 */				
				
				paddw		xmm3,	xmm1				/* xmm3 = p1 + p1 + p1 */		
				movdqa		xmm4,	[edi+16]			/* xmm4 = x1 */					
				
				paddw		xmm3,	[edi+32]			/* xmm3 = p1+p1+p1+ x2 */		
				paddw		xmm4,	[edi+48]			/* xmm4 = x1+x3 */				
				
				paddw		xmm3,	[edi+64]			/* xmm3 += x4 */					
				paddw		xmm4,	EightFours			/* xmm4 = x1 + x3 + 4 */			
				
				paddw		xmm3,	xmm4				/* xmm3 = 3*p1+x1+x2+x3+x4+4 */	
				movdqa		xmm4,	xmm3				/* xmm4 = xmm3 */					
				
				movdqa		xmm5,	[edi+16]			/* xmm5 = x1 */					
				paddw		xmm4,	xmm5				/* xmm4 = sum+x1 */				
				
				psllw		xmm4,	1					/* xmm4 = (sum+x1)<<1 */			
				psubw		xmm4,	[edi+64]			/* xmm4 = (sum+x1)<<1-x4 */		
				
				paddw		xmm4,	[edi+80]			/* xmm4 = (sum+x1)<<1-x4+x5 */	
				psraw		xmm4,	4					/* xmm4 >>=4 */					
				
				psubw		xmm4,	xmm5				/* New Value - old Value */		
				pand		xmm4,	xmm7				/* And the flag */				
				
				paddw		xmm4,	xmm5				/* add the old value back */	
				packuswb	xmm4,	xmm0				/* pack it to bytes */
				
				movq		QWORD PTR [esi+edx*2], xmm4	/* Write new x1 */				
				
				/* sum += x5 -p1 */														
				/* Des[-w3]=((sum+x2)<<1-x5+x6)>>4 */									
				
				movdqa		xmm5,	[edi+32]			/* xmm5= x2 */					
				psubw		xmm3,	xmm1				/* sum=sum-p1 */				
				
				paddw		xmm3,    [edi+80]			/* sum=sum+x5 */				
				movdqa		xmm4,	xmm5				/* copy sum */					
				
				paddw		xmm4,	xmm3				/* xmm4=sum+x2 */				
				paddw		xmm4,	xmm4				/* xmm4 <<= 1 */					
				
				psubw		xmm4,	[edi+80]			/* xmm4 =(sum+x2)<<1-x5 */		
				paddw		xmm4,	[edi+96]			/* xmm4 =(sum+x2)<<1-x5+x6 */	
				
				psraw		xmm4,	4					/* xmm4=((sum+x2)<<1-x5+x6)>>4 */
				psubw		xmm4,	xmm5				/* new value - old value	*/	
				
				pand		xmm4,	xmm7				/* And the flag */				
				paddw		xmm4,	xmm5				/* add the old value back */	

				packuswb	xmm4,	xmm0				/* pack it to bytes */
				movq		QWORD PTR [esi+edx], xmm4	/* write new x2 */				
				
				/* sum += x6 - p1 */													
				/* Des[-w2]=((sum+x[3])<<1-x[6]+x[7])>>4 */								
				
				movdqa		xmm5,	[edi+48]			/* xmm5= x3 */					
				psubw		xmm3,	xmm1				/* sum=sum-p1 */				
				
				paddw		xmm3,    [edi+96]			/* sum=sum+x6 */				
				movdqa		xmm4,	xmm5				/* copy x3 */					
				
				paddw		xmm4,	xmm3				/* xmm4=sum+x3 */				
				paddw		xmm4,	xmm4				/* xmm4 <<= 1 */					
				
				psubw		xmm4,	[edi+96]			/* xmm4 =(sum+x3)<<1-x6 */		
				paddw		xmm4,	[edi+112]			/* xmm4 =(sum+x3)<<1-x6+x7 */	
				
				psraw		xmm4,	4					/* xmm4=((sum+x3)<<1-x6+x7)>>4 */
				psubw		xmm4,	xmm5				/* new value - old value	*/	
				
				pand		xmm4,	xmm7				/* And the flag */				
				paddw		xmm4,	xmm5				/* add the old value back */	
				
				packuswb	xmm4,	xmm0				/* pack it to bytes */
				movq		QWORD PTR [esi],xmm4		/* write new x3 */				
				
				/* sum += x7 - p1 */													
				/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
				
				movdqa		xmm5,	[edi+64]			/* xmm5 = x4 */					
				psubw		xmm3,	xmm1				/* sum = sum-p1 */				
				
				paddw		xmm3,	[edi+112]			/* sum = sum+x7 */				
				movdqa		xmm4,	xmm5				/* xmm4 = x4 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum + x4 */			
				paddw		xmm4,	xmm4				/* xmm4 *=2 */					
				
				paddw		xmm4,	xmm1				/* += p1 */						
				psubw		xmm4,	[edi+16]			/* -= x1 */						
				
				psubw		xmm4,	[edi+112]			/* -= x7 */						
				paddw		xmm4,	[edi+128]			/* += x8 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x4 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x4 */						
				
				packuswb	xmm4,	xmm0				/* pack it to bytes */
				movq	    QWORD PTR [esi+ecx], xmm4	/* write new x4 */				
				
				/* sum+= x8-x1 */														
				/* Des[0]=((sum+x5)<<1+x1-x2-x8+p2)>>4 */								
				
				movdqa		xmm5,	[edi+80]			/* xmm5 = x5 */					
				psubw		xmm3,	[edi+16]			/* sum -= x1 */					
				
				paddw		xmm3,	[edi+128]			/* sub += x8 */					
				movdqa		xmm4,	xmm5				/* xmm4 = x5 */					
				
				paddw		xmm4,	xmm3				/* xmm4= sum+x5 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2 */					
				
				paddw		xmm4,	[edi+16]			/* += x1 */						
				psubw		xmm4,	[edi+32]			/* -= x2 */						
				
				psubw		xmm4,	[edi+128]			/* -= x8 */						
				paddw		xmm4,	xmm2				/* += p2 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x5 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x5 */						
				
				lea			esi,	[esi+ecx*4]			/* esi=des + 2*pitch */
				packuswb	xmm4,	xmm0				/* pack to bytes */

				movq		QWORD PTR [esi+edx*2], xmm4	/* write new x5 */				
				
				/* sum += p2 - x2 */													
				/* Des[w1] = ((sum+x6)<<1 + x2-x3)>>4 */								
				
				movdqa		xmm5,	[edi+96]			/* xmm5 = x6 */					
				psubw		xmm3,	[edi+32]			/* -= x2 */						
				
				paddw		xmm3,	xmm2				/* += p2 */						
				movdqa		xmm4,	xmm5				/* xmm4 = x6 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum+x6 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2*/					
				
				paddw		xmm4,	[edi+32]			/* +=x2 */						
				psubw		xmm4,	[edi+48]			/* -=x3 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x6 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x6 */						
				
				packuswb	xmm4,	xmm0				/* pack to bytes */
				movq		QWORD PTR [esi+edx], xmm4	/* write new x6 */				
				
				/* sum += p2 - x3 */													
				/* Des[w2] = ((sum+x7)<<1 + x3-x4)>>4 */								
				
				movdqa		xmm5,	[edi+112]			/* xmm5 = x7 */					
				psubw		xmm3,	[edi+48]			/* -= x3 */						
				
				paddw		xmm3,	xmm2				/* += p2 */						
				movdqa		xmm4,	xmm5				/* xmm4 = x7 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum+x7 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2*/					
				
				paddw		xmm4,	[edi+48]			/* +=x3 */						
				psubw		xmm4,	[edi+64]			/* -=x4 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x7 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x7 */						

				packuswb	xmm4,	xmm0				/* pack to bytes */				
				movq		QWORD PTR [esi],xmm4		/* write new x7 */				
				
				/* sum += p2 - x4 */													
				/* Des[w3] = ((sum+x8)<<1 + x4-x5)>>4 */								
				
				movdqa		xmm5,	[edi+128]			/* xmm5 = x8 */					
				psubw		xmm3,	[edi+64]			/* -= x4 */						
				
				paddw		xmm3,	xmm2				/* += p2 */						
				movdqa		xmm4,	xmm5				/* xmm4 = x8 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum+x8 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2*/					
				
				paddw		xmm4,	[edi+64]			/* +=x4 */						
				psubw		xmm4,	[edi+80]			/* -=x5 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x8 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x8 */						

				packuswb	xmm4,	xmm0				/* pack to bytes */				
				movq		QWORD PTR [esi+ecx], xmm4				/* write new x8 */				

				pop			edi
				pop			esi
				pop			edx
				pop			ecx
				pop			eax

                } /* end of the macro */
    		Var1=Variance1[0]+Variance1[1]+Variance1[2]+Variance1[3]+Variance1[4]+Variance1[5]+Variance1[6]+Variance1[7];
	    	Var2=Variance2[0]+Variance2[1]+Variance2[2]+Variance2[3]+Variance2[4]+Variance2[5]+Variance2[6]+Variance2[7];

		    pbi->FragmentVariances[CurrentFrag] += Var1;
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
            QStepWMT[0] = (INT16)QStep;
            QStepWMT[1] = (INT16)QStep;
            QStepWMT[2] = (INT16)QStep;
            QStepWMT[3] = (INT16)QStep;
            QStepWMT[4] = (INT16)QStep;
            QStepWMT[5] = (INT16)QStep;
            QStepWMT[6] = (INT16)QStep;
            QStepWMT[7] = (INT16)QStep;

		    for( j=0; j<8;j++)
		    {
    			Rows[j] = (short) (Src[-5 +j*PlaneLineStep]);
	    		Rows[72+j] = (short)(Src[4+j*PlaneLineStep]);		
    		}

	    	__asm
    		{
				/* Save the registers */
				push		eax
				push		ecx			
				push		edx
				push		esi
				push		edi
				
				/* Calculate the FLimit and store FLimit and QStep */					
				
				movdqa		xmm0,	QStepWMT            /* Get QStep */
				movdqa		xmm1,	EightThrees			/* mm1 = 03030303 */			

                pmullw		xmm1,	xmm0				/* mm1 = QStep * 3 */							
				pmullw		xmm1,	xmm0				/* mm1 = QStep * QStep * 3 */					
				
                psrlw		xmm1,	5					/* mm1 = FLimit */				
				movdqa		[FLimitWMT], xmm1			/* Save FLimit */				

				/* setup the pointers to data */

				mov			eax,	Src					/* eax = Src */
				xor			edx,	edx					/* clear edx */
				
				mov			esi,	Des					/* esi = Des */
				sub			eax,	4					/* eax = Src-4 */

				sub			esi,	4					/* esi = Des-4 */
				lea			edi,	Rows				/* edi = Rows */				

				mov			ecx,	PlaneLineStep		/* ecx = Pitch */				
				sub			edx,	ecx					/* edx = -Pitch */				

				lea			esi,	[esi+ecx*2]			/* esi = Des-4 + 2 * Pitch */
				
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

				/* we use xmm0,xmm1,xmm2 for 1234 and xmm4, xmm5, xmm6 for 5-8 */				
				/* xmm7 = 0, xmm3 = {128, 128, 128, 128, 128, 128, 128, 128} */								
				
				pcmpeqw		xmm3,	xmm3				/* xmm3 = FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */	
				psllw		xmm3,	15					/* xmm3 = 80008000800080008000800080008000 */	
				psrlw		xmm3,	8					/* xmm3 = 00800080008000800080008000800080 */
				
				movdqa		xmm2,	[edi+16]			/* Pixel 1 */					
				movdqa		xmm6,	[edi+80]			/* Pixel 5 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				movdqa		xmm0,	xmm2				/* xmm0 = pixel 1 */				
				movdqa		xmm4,	xmm6				/* xmm4 = pixel 5 */				
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel1 * pixel1 */		
				pmullw		xmm6,	xmm6				/* xmm6 = pixel5 * pixel5 */		
				
				movdqa		xmm1,	xmm2				/* xmm1 = pixel1^2 */			
				movdqa		xmm5,	xmm6				/* xmm5 = pixel5^2 */			
				
				movdqa		xmm2,	[edi+32]			/* Pixel 2 */					
				movdqa		xmm6,	[edi+96]			/* Pixel 6 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				paddw		xmm0,	xmm2				/* xmm0 += pixel 2 */			
				paddw		xmm4,	xmm6				/* xmm4 += pixel 6 */			
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel2^2 */			
				pmullw		xmm6,	xmm6				/* xmm6 = pixel6^2 */			
				
				paddw		xmm1,	xmm2				/* xmm1 += pixel2^2 */			
				paddw		xmm5,	xmm6				/* xmm5 += pixel6^2 */			
				
				movdqa		xmm2,	[edi+48]			/* Pixel 3 */					
				movdqa		xmm6,	[edi+112]			/* Pixel 7 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				paddw		xmm0,	xmm2				/* xmm0 += pixel 3 */			
				paddw		xmm4,	xmm6				/* xmm4 += pixel 7 */			
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel3^2 */			
				pmullw		xmm6,	xmm6				/* xmm6 = pixel7^2 */			
				
				paddw		xmm1,	xmm2				/* xmm1 += pixel3^2 */			
				paddw		xmm5,	xmm6				/* xmm5 += pixel7^2 */			
				
				movdqa		xmm2,	[edi+64]			/* Pixel 4 */					
				movdqa		xmm6,	[edi+128]			/* Pixel 8 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				paddw		xmm0,	xmm2				/* xmm0 += pixel 4 */			
				paddw		xmm4,	xmm6				/* xmm4 += pixel 8 */			
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel4^2 */			
				pmullw		xmm6,	xmm6				/* xmm6 = pixel8^2 */			
				
				paddw		xmm1,	xmm2				/* xmm1 = pixel4^2 */			
				paddw		xmm5,	xmm6				/* xmm5 = pixel8^2 */			
				
				/* xmm0 = x1^2 + x2^2 + x3^2 + x4^2 */									
				/* xmm1 = x1 + x2 + x3 + x4 */											
				/* xmm4 = x5^2 + x6^2 + x7^2 + x8^2 */									
				/* xmm5 = x5 + x6 + x7 + x8 */											
				
				movdqa		xmm7,	xmm3				/* xmm7 = xmm3 */					
				psrlw		xmm7,	7					/* xmm7 = 00010001000100010001000100010001 */	
				
				movdqa		xmm2,	xmm0				/* make copy of sum1 */			
				movdqa		xmm6,	xmm4				/* make copy of sum2 */			
				
				paddw		xmm0,	xmm7				/* (sum1 + 1) */				
				paddw		xmm4,	xmm7				/* (sum2 + 1) */				
				
				psraw		xmm2,	1					/* sum1 /2 */					
				psraw		xmm6,	1					/* sum2 /2 */					
				
				psraw		xmm0,	1					/* (sum1 + 1)/2 */				
				psraw		xmm4,	1					/* (sum2 + 1)/2 */				
				
				pmullw		xmm2,	xmm0				/* (sum1)/2*(sum1+1)/2 */		
				pmullw		xmm6,	xmm4				/* (sum2)/2*(sum2+1)/2 */		
				
				psubw		xmm1,	xmm2				/* Variance 1 */				
				psubw		xmm5,	xmm6				/* Variance 2 */				
				
				movdqa		xmm7,	FLimitWMT			/* xmm7 = FLimit */				
				movdqa		xmm2,	xmm1				/* copy of Varinace 1*/

                movdqa		[Variance1], xmm1			/* save the varinace1 */
				movdqa		[Variance2], xmm5			/* save the varinace2 */

				movdqa		xmm6,	xmm5				/* Variance 2 */
				psubw		xmm1,	xmm7				/* Variance 1 < Flimit? */		
				
				psubw		xmm5,	xmm7				/* Variance 2 < Flimit? */		
				psraw		xmm2,	15					/* Variance 1 > 32768? */

				psraw		xmm6,	15					/* Vaiance  2 > 32768? */	
				psraw		xmm1,	15					/* FFFF/0000 for true/false */	
				
				psraw		xmm5,	15					/* FFFF/0000 for true/false */	
				movdqa		xmm7,	[edi+64]			/* xmm0 = Pixel 4			*/	

				pandn		xmm2,	xmm1				/* Variance1<32678 && 
															Variance1<Limit			*/
				pandn		xmm6,	xmm5				/* Variance2<32678 && 
														   Variance1<Limit			*/
				
				movdqa		xmm4,	[edi+80]			/* xmm4 = Pixel 5			*/	
				pand		xmm6,	xmm2				/* xmm6 = Variance1 < Flimit */	
														/*     &&Variance2 < Flimit */	

				movdqa		xmm2,	xmm7				/* make copy of Pixel4		*/	

				psubusw		xmm7,	xmm4				/* 4 - 5 */						
				psubusw		xmm4,	xmm2				/* 5 - 4 */						
				
				por			xmm7,	xmm4				/* abs(4 - 5) */				
				psubw		xmm7,	QStepWMT			/* abs(4-5)<QStepxmmx ? */		
				
				psraw		xmm7,	15					/* FFFF/0000 for True/Flase */
				pand		xmm7,	xmm6													
				
				/* xmm7 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* xmm7 now are in use  */										
				/* Let's do the filtering now */										
				/* p1 = (abs(Src[-4] - Src[-5]) < QStep ) ?  Src[-5] : Src[-4]; */		
				/* p2 = (abs(Src[+3] - Src[+4]) < QStep ) ?  Src[+4] : Src[+3]; */		
				
				movdqa		xmm5,	[edi]				/* xmm5 = -5 */					
				movdqa		xmm4,	[edi + 16]			/* xmm4 = -4 */					
				
				movdqa		xmm3,	xmm4				/* copy of -4 */				
				movdqa		xmm6,	xmm5				/* copy of -5 */				
				
				psubusw		xmm4,	xmm6				/* xmm4 = [-4] - [-5] */			
				psubusw		xmm5,	xmm3				/* xmm5 = [-5] - [-4] */			
				
				por			xmm4,	xmm5				/* abs([-4]-[-5] ) */			
				psubw		xmm4,	QStepWMT			/* abs([-4]-[-5] )<QStep? */	
				
				psraw		xmm4,	15					/* FFFF/0000 for True/False */	
				movdqa		xmm1,	xmm4				/* copy of the xmm4 */			
				
				pand		xmm4,	xmm6				/*							*/	
				pandn		xmm1,	xmm3				/*							*/	
				
				por			xmm1,	xmm4				/* xmm1 = p1				*/	
				
				/* now find P2 */														
				
				movdqa		xmm4,	[edi+128]			/* xmm4 = [3] */					
				movdqa		xmm5,	[edi+144]			/* xmm5 = [4] */					
				
				movdqa		xmm3,	xmm4				/* copy of 3 */					
				movdqa		xmm6,	xmm5				/* copy of 4 */					
				
				psubusw		xmm4,	xmm6				/* xmm4 = [3] - [4] */			
				psubusw		xmm5,	xmm3				/* xmm5 = [4] - [3] */			
				
				por			xmm4,	xmm5				/* abs([3]-[4] ) */				
				psubw		xmm4,	QStepWMT			/* abs([3]-[4] )<QStep? */		
				
				psraw		xmm4,	15					/* FFFF/0000 for True/False */	
				movdqa		xmm2,	xmm4				/* copy of the xmm4 */			
				
				pand		xmm4,	xmm6				/*							*/	
				pandn		xmm2,	xmm3				/*							*/	
				
				por			xmm2,	xmm4				/* xmm2 = p2				*/	

				/* Data is ready, now do the filtering */
				
				pxor		xmm0,	xmm0				/* clear xmm0 */

				/* sum = p1 + p1 + p1 + x1 + x2 + x3 + x4 + 4; */				
				/* Des[-w4] = (((sum + x1) << 1) - (x4 - x5)) >> 4; */			
				/* Des[-w4] = Src[-w4]; */												
				/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	

				
				movdqa		xmm3,	xmm1				/* xmm3 = p1 */					
				paddw		xmm3,	xmm3				/* xmm3 = p1 + p1 */				
				
				paddw		xmm3,	xmm1				/* xmm3 = p1 + p1 + p1 */		
				movdqa		xmm4,	[edi+16]			/* xmm4 = x1 */					
				
				paddw		xmm3,	[edi+32]			/* xmm3 = p1+p1+p1+ x2 */		
				paddw		xmm4,	[edi+48]			/* xmm4 = x1+x3 */				
				
				paddw		xmm3,	[edi+64]			/* xmm3 += x4 */					
				paddw		xmm4,	EightFours			/* xmm4 = x1 + x3 + 4 */			
				
				paddw		xmm3,	xmm4				/* xmm3 = 3*p1+x1+x2+x3+x4+4 */	
				movdqa		xmm4,	xmm3				/* xmm4 = xmm3 */					
				
				movdqa		xmm5,	[edi+16]			/* xmm5 = x1 */					
				paddw		xmm4,	xmm5				/* xmm4 = sum+x1 */				
				
				psllw		xmm4,	1					/* xmm4 = (sum+x1)<<1 */			
				psubw		xmm4,	[edi+64]			/* xmm4 = (sum+x1)<<1-x4 */		
				
				paddw		xmm4,	[edi+80]			/* xmm4 = (sum+x1)<<1-x4+x5 */	
				psraw		xmm4,	4					/* xmm4 >>=4 */					
				
				psubw		xmm4,	xmm5				/* New Value - old Value */		
				pand		xmm4,	xmm7				/* And the flag */				
				
				paddw		xmm4,	xmm5				/* add the old value back */	
				packuswb	xmm4,	xmm0				/* pack it to bytes */
				
				movdq2q		mm0,	xmm4				/* Write new x1 */				
				
				/* sum += x5 -p1 */														
				/* Des[-w3]=((sum+x2)<<1-x5+x6)>>4 */									
				
				movdqa		xmm5,	[edi+32]			/* xmm5= x2 */					
				psubw		xmm3,	xmm1				/* sum=sum-p1 */				
				
				paddw		xmm3,    [edi+80]			/* sum=sum+x5 */				
				movdqa		xmm4,	xmm5				/* copy sum */					
				
				paddw		xmm4,	xmm3				/* xmm4=sum+x2 */				
				paddw		xmm4,	xmm4				/* xmm4 <<= 1 */					
				
				psubw		xmm4,	[edi+80]			/* xmm4 =(sum+x2)<<1-x5 */		
				paddw		xmm4,	[edi+96]			/* xmm4 =(sum+x2)<<1-x5+x6 */	
				
				psraw		xmm4,	4					/* xmm4=((sum+x2)<<1-x5+x6)>>4 */
				psubw		xmm4,	xmm5				/* new value - old value	*/	
				
				pand		xmm4,	xmm7				/* And the flag */				
				paddw		xmm4,	xmm5				/* add the old value back */	

				packuswb	xmm4,	xmm0				/* pack it to bytes */
				movdq2q		mm1,	xmm4				/* write new x2 */				
				
				/* sum += x6 - p1 */													
				/* Des[-w2]=((sum+x[3])<<1-x[6]+x[7])>>4 */								
				
				movdqa		xmm5,	[edi+48]			/* xmm5= x3 */					
				psubw		xmm3,	xmm1				/* sum=sum-p1 */				
				
				paddw		xmm3,    [edi+96]			/* sum=sum+x6 */				
				movdqa		xmm4,	xmm5				/* copy x3 */					
				
				paddw		xmm4,	xmm3				/* xmm4=sum+x3 */				
				paddw		xmm4,	xmm4				/* xmm4 <<= 1 */					
				
				psubw		xmm4,	[edi+96]			/* xmm4 =(sum+x3)<<1-x6 */		
				paddw		xmm4,	[edi+112]			/* xmm4 =(sum+x3)<<1-x6+x7 */	
				
				psraw		xmm4,	4					/* xmm4=((sum+x3)<<1-x6+x7)>>4 */
				psubw		xmm4,	xmm5				/* new value - old value	*/	
				
				pand		xmm4,	xmm7				/* And the flag */				
				paddw		xmm4,	xmm5				/* add the old value back */	
				
				packuswb	xmm4,	xmm0				/* pack it to bytes */
				movdq2q		mm2,	xmm4				/* write new x3 */				
				
				/* sum += x7 - p1 */													
				/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
				
				movdqa		xmm5,	[edi+64]			/* xmm5 = x4 */					
				psubw		xmm3,	xmm1				/* sum = sum-p1 */				
				
				paddw		xmm3,	[edi+112]			/* sum = sum+x7 */				
				movdqa		xmm4,	xmm5				/* xmm4 = x4 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum + x4 */			
				paddw		xmm4,	xmm4				/* xmm4 *=2 */					
				
				paddw		xmm4,	xmm1				/* += p1 */						
				psubw		xmm4,	[edi+16]			/* -= x1 */						
				
				psubw		xmm4,	[edi+112]			/* -= x7 */						
				paddw		xmm4,	[edi+128]			/* += x8 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x4 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x4 */						
				
				packuswb	xmm4,	xmm0				/* pack it to bytes */
				movdq2q		mm3,	xmm4				/* write new x4 */				
				

				/* sum+= x8-x1 */														
				/* Des[0]=((sum+x5)<<1+x1-x2-x8+p2)>>4 */								
				
				movdqa		xmm5,	[edi+80]			/* xmm5 = x5 */					
				psubw		xmm3,	[edi+16]			/* sum -= x1 */					
				
				paddw		xmm3,	[edi+128]			/* sub += x8 */					
				movdqa		xmm4,	xmm5				/* xmm4 = x5 */					
				
				paddw		xmm4,	xmm3				/* xmm4= sum+x5 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2 */					
				
				paddw		xmm4,	[edi+16]			/* += x1 */						
				psubw		xmm4,	[edi+32]			/* -= x2 */						
				
				psubw		xmm4,	[edi+128]			/* -= x8 */						
				paddw		xmm4,	xmm2				/* += p2 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x5 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x5 */						
				
				packuswb	xmm4,	xmm0				/* pack to bytes */
				movdq2q		mm4,	xmm4				/* write new x5 */				
				
				/* sum += p2 - x2 */													
				/* Des[w1] = ((sum+x6)<<1 + x2-x3)>>4 */								
				
				movdqa		xmm5,	[edi+96]			/* xmm5 = x6 */					
				psubw		xmm3,	[edi+32]			/* -= x2 */						
				
				paddw		xmm3,	xmm2				/* += p2 */						
				movdqa		xmm4,	xmm5				/* xmm4 = x6 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum+x6 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2*/					
				
				paddw		xmm4,	[edi+32]			/* +=x2 */						
				psubw		xmm4,	[edi+48]			/* -=x3 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x6 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x6 */						
				
				packuswb	xmm4,	xmm0				/* pack to bytes */
				movdq2q		mm5,	xmm4				/* write new x6 */				
				
				/* sum += p2 - x3 */													
				/* Des[w2] = ((sum+x7)<<1 + x3-x4)>>4 */								
				
				movdqa		xmm5,	[edi+112]			/* xmm5 = x7 */					
				psubw		xmm3,	[edi+48]			/* -= x3 */						
				
				paddw		xmm3,	xmm2				/* += p2 */						
				movdqa		xmm4,	xmm5				/* xmm4 = x7 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum+x7 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2*/					
				
				paddw		xmm4,	[edi+48]			/* +=x3 */						
				psubw		xmm4,	[edi+64]			/* -=x4 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x7 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x7 */						

				packuswb	xmm4,	xmm0				/* pack to bytes */				
				movdq2q		mm6,	xmm4				/* write new x7 */				
				
				/* sum += p2 - x4 */													
				/* Des[w3] = ((sum+x8)<<1 + x4-x5)>>4 */								
				
				movdqa		xmm5,	[edi+128]			/* xmm5 = x8 */					
				psubw		xmm3,	[edi+64]			/* -= x4 */						
				
				paddw		xmm3,	xmm2				/* += p2 */						
				movdqa		xmm4,	xmm5				/* xmm4 = x8 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum+x8 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2*/					
				
				paddw		xmm4,	[edi+64]			/* +=x4 */						
				psubw		xmm4,	[edi+80]			/* -=x5 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x8 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x8 */						

				packuswb	xmm4,	xmm0				/* pack to bytes */				
				movdq2q		mm7,	xmm4				/* write new x8 */				


				/* transpose */
				movq2dq		xmm0,	mm0					/* xmm0 = 70 60 50 40 30 20 10 00 */
				movq2dq		xmm1,	mm1					/* xmm1 = 71 61 51 41 31 21 11 01 */

				movq2dq		xmm2,	mm2					/* xmm2 = 72 62 52 42 32 22 12 02 */
				movq2dq		xmm3,	mm3					/* xmm3 = 73 63 53 43 33 23 13 03 */

				punpcklbw	xmm0,	xmm1				/* xmm0 = 7170 6160 5150 4140 3130 2120 1110 0100 */
				punpcklbw	xmm2,	xmm3				/* xmm2 = 7372 6362 5352 4342 3332 2322 1312 0302 */

				movdqa		xmm1,	xmm0				/* xmm1 = 7170 6160 5150 4140 3130 2120 1110 0100 */
				punpcklwd	xmm0,	xmm2				/* xmm0 = 33323130 23222120 13121110 03020100 */

				punpckhwd	xmm1,	xmm2				/* xmm1 = 73727170 63626160 53525150 43424140 */
				
				movq2dq		xmm4,	mm4					/* xmm4 = 74 64 54 44 34 24 14 04 */
				movq2dq		xmm5,	mm5					/* xmm5 = 75 65 55 45 35 25 15 05 */				

				movq2dq		xmm6,	mm6 				/* xmm6 = 76 66 56 46 36 26 16 06 */
				movq2dq		xmm7,	mm7					/* xmm7 = 77 67 57 47 37 27 17 07 */
								
				punpcklbw	xmm4,	xmm5				/* xmm4 = 7574 6564 5554 4544 3534 2524 1514 0504 */
				punpcklbw	xmm6,	xmm7				/* xmm6 = 7776 6766 5756 4746 3736 2726 1716 0706 */

				movdqa		xmm5,	xmm4				/* xmm5 = 7574 6564 5554 4544 3534 2524 1514 0504 */
				punpcklwd	xmm4,	xmm6				/* xmm4 = 37363534 27262524 17161514 07060504 */

				punpckhwd	xmm5,	xmm6				/* xmm5 = 77767574 67666564 57565554 47464544 */
				movdqa		xmm2,	xmm0				/* xmm2 = 33323130 23222120 13121110 03020100 */

				punpckldq	xmm0,	xmm4				/* xmm0 = 1716151413121110	0706050403020100 */
				movq		QWORD PTR [esi+edx*2],xmm0	/* write 00 01 02 03 04 05 06 07 */

				psrldq		xmm0,	8					/* xmm0 = 1716151413121110 */
				punpckhdq	xmm2,	xmm4				/* xmm2 = 3736353433323130	2726252423222120 */

				movq		QWORD PTR [esi+edx], xmm0	/* write 10 11 12 13 14 15 16 17 */
				movdqa		xmm3,	xmm1				/* xmm3 = 73727170 63626160 53525150 43424140 */
				
				punpckldq	xmm1,	xmm5				/* xmm1 = 5756555453525150 4746454443424140 */
				movq		QWORD PTR [esi],	xmm2	/* write 20 21 22 23 24 25 26 27 */
				
				psrldq		xmm2,	8					/* xmm2 = 3736353433323130 */
				punpckhdq	xmm3,	xmm5				/* xmm3 = 7776757473727170 6766656463626160 */

				movq		QWORD PTR [esi+ecx], xmm2	/* write 30 31 32 33 34 35 36 37 */
				lea			esi,	[esi+ecx*4]			/* esi= Des - 4 + 4 *pitch */
				
				movq		QWORD PTR [esi+edx*2], xmm1	/* write 40 41 42 43 44 45 46 47 */
				movq		QWORD PTR [esi],	xmm3	/* write 60 61 62 63 64 65 66 67 */

				psrldq		xmm1,	8					/* xmm1 = 5756555453525150 */
				psrldq		xmm3,	8					/* xmm3 = 7776757473727170 */

				movq		QWORD PTR [esi+edx], xmm1	/* write 50 51 52 53 54 55 56 57 */
				movq		QWORD PTR [esi+ecx], xmm3	/* write 70 71 72 73 74 75 76 77 */


				pop			edi
				pop			esi
				pop			edx
				pop			ecx
				pop			eax
	    	}// end of __asm	

            Var1=Variance1[0]+Variance1[1]+Variance1[2]+Variance1[3]+Variance1[4]+Variance1[5]+Variance1[6]+Variance1[7];
	    	Var2=Variance2[0]+Variance2[1]+Variance2[2]+Variance2[3]+Variance2[4]+Variance2[5]+Variance2[6]+Variance2[7];

		    pbi->FragmentVariances[CurrentFrag] += Var1;
		    pbi->FragmentVariances[CurrentFrag + 1] += Var2;
        }// end of if
		CurrentFrag ++;
		Src += 8;
		Des += 8;		
	}//end of while
#endif
}


/****************************************************************************
 * 
 *  ROUTINE       :     DeblockNonFilteredBand_WMT
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

void DeblockNonFilteredBand_WMT(
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
	return;
#else
__declspec(align(16)) short QStepWMT[8];
__declspec(align(16)) short FLimitWMT[8];
__declspec(align(16)) short Rows[80];
__declspec(align(16)) short LoopFLimitWMT[8];
__declspec(align(16)) short LoopFilteredValuesUp[8];
__declspec(align(16)) short LoopFilteredValuesDown[8];

__declspec(align(16)) unsigned short Variance1[8];
__declspec(align(16)) unsigned short Variance2[8];


    LoopFLimit = DeblockLimitValuesV2[pbi->FrameQIndex];
    LoopFLimitWMT[0] = (INT16)LoopFLimit;
    LoopFLimitWMT[1] = (INT16)LoopFLimit;
    LoopFLimitWMT[2] = (INT16)LoopFLimit;
    LoopFLimitWMT[3] = (INT16)LoopFLimit;
    LoopFLimitWMT[4] = (INT16)LoopFLimit;
    LoopFLimitWMT[5] = (INT16)LoopFLimit;
    LoopFLimitWMT[6] = (INT16)LoopFLimit;
    LoopFLimitWMT[7] = (INT16)LoopFLimit;


	while(CurrentFrag < StartFrag + FragAcross )
	{

		Src=SrcPtr+8*(CurrentFrag-StartFrag);
		Des=DesPtr+8*(CurrentFrag-StartFrag);

		QStep = QuantScale[ pbi->FragQIndex[CurrentFrag+FragAcross]];


		__asm 
		{
			
		    	push		eax
		        push		ecx			
		    	push		edx
		    	push		esi
		        push		edi
	
				/* Calculate the FLimit and store FLimit and QStep */					
				/* Copy the data to the intermediate buffer */							
				mov			eax,	    QStep
				xor			edx,	    edx					/* clear edx */					

				mov			ecx,	    PlaneLineStep		/* ecx = Pitch */				
			    pcmpeqw		xmm6,	    xmm6				/* xmm6 = FFFFFF... */	
				
				
				movd		mm5,	    eax                 /* mm5 = QStep */
				psrlw		xmm6,	    14					/* xmm6 = 3, 3, 3, 3, 3, 3, 3, 3*/
	
			    punpcklwd	mm5,	    mm5					/* mm5 = QQ */
            	mov			eax,	    Src					/* eax = Src */												
	
                punpckldq	mm5,	    mm5                 /* mm5 = QQQQ */
            	sub			edx,	    ecx					/* edx = - Pitch */
				
                movq2dq     xmm5,       mm5                 /* xmm5 = QQQQ */
            	punpcklqdq  xmm5,       xmm5                /* xmm5 = QQQQQQQQ */
	
            	pmullw		xmm6,	    xmm5			    /* Qstep * 3 */
				movdqa      QStepWMT,	xmm5
	
                lea			edi,	    Rows				/* edi = Rows */				
				pxor		xmm7,	    xmm7				/* Clear mm7 */					

            	mov         esi,        Des                 /* esi = des */
				pmullw		xmm6,	    xmm5
	
				lea			eax,	    [eax + edx * 4 ]	/* eax = Src - 4*Pitch */		
            	lea         esi,        [esi + edx * 2]     /* esi = Des - 2*Pitch */

                psraw       xmm6,       5
                movdqa      FLimitWMT,  xmm6

            	/* Copy the data to the intermediate buffer */
            	
				movq		xmm0,	    QWORD PTR [eax + edx]/* xmm0 = Src[-5*Pitch] */		
				movq		xmm1,	    QWORD PTR [eax ]	/* xmm1 = Src[-4*Pitch */
				
				punpcklbw	xmm0,	    xmm7				/* expand to words */
				punpcklbw	xmm1,	    xmm7				/* expand to words */

				movdqa		[edi],	    xmm0				/* write 8 words */
				movdqa		[edi+16],   xmm1				/* write 8 words */

				movq		xmm2,	    QWORD PTR [eax+ecx]	/* xmm2 = Src[-3*Pitch] */		
				movq		xmm3,	    QWORD PTR [eax+ecx*2]/* xmm3 = Src[-2*Pitch] */

				punpcklbw	xmm2,	    xmm7				/* expand to words */
				punpcklbw	xmm3,	    xmm7				/* expand to words */
				
				movdqa		[edi+32],   xmm2				/* write 8 words */
				movdqa		[edi+48],   xmm3				/* write 8 words */

				lea			eax,	    [eax+ecx*4]			/* eax= Src */

				movq		xmm0,	    QWORD PTR [eax + edx]/* xmm0 = Src[-Pitch] */		
				movq		xmm1,	    QWORD PTR [eax ]	/* xmm1 = Src[0] */
				
				punpcklbw	xmm0,	    xmm7				/* expand to words */
				punpcklbw	xmm1,	    xmm7				/* expand to words */

				movdqa		[edi+64],   xmm0				/* write 8 words */
				movdqa		[edi+80],   xmm1				/* write 8 words */

				movq		xmm2,	    QWORD PTR [eax+ecx]	/* xmm2 = Src[Pitch] */		
				movq		xmm3,	    QWORD PTR [eax+ecx*2]/* xmm3 = Src[2*Pitch] */

				punpcklbw	xmm2,	    xmm7				/* expand to words */
				punpcklbw	xmm3,	    xmm7				/* expand to words */
				
				movdqa		[edi+96],   xmm2				/* write 8 words */
				movdqa		[edi+112],  xmm3				/* write 8 words */

				lea			eax,	    [eax+ecx*4]			/* eax= Src+4*Pitch */

				movq		xmm0,	    QWORD PTR [eax + edx]/* xmm0 = Src[3*Pitch] */		
				movq		xmm1,	    QWORD PTR [eax ]	/* xmm1 = Src[4*Pitch] */
				
				punpcklbw	xmm0,	    xmm7				/* expand to words */
				punpcklbw	xmm1,	    xmm7				/* expand to words */

				movdqa		[edi+128],  xmm0				/* write 8 words */
				movdqa		[edi+144],  xmm1				/* write 8 words */

	
				/* done with copying everything to intermediate buffer */				
				/* Now, compute the variances for Pixel  1-4 and 5-8 */					
		
				/* we use xmm0,xmm1,xmm2 for 1234 and xmm4, xmm5, xmm6 for 5-8 */				
				/* xmm7 = 0, xmm3 = {128, 128, 128, 128, 128, 128, 128, 128} */								
				
				pcmpeqw		xmm3,	xmm3				/* xmm3 = FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */	
				psllw		xmm3,	15					/* xmm3 = 80008000800080008000800080008000 */	
				psrlw		xmm3,	8					/* xmm3 = 00800080008000800080008000800080 */
				
				movdqa		xmm2,	[edi+16]			/* Pixel 1 */					
				movdqa		xmm6,	[edi+80]			/* Pixel 5 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				movdqa		xmm0,	xmm2				/* xmm0 = pixel 1 */				
				movdqa		xmm4,	xmm6				/* xmm4 = pixel 5 */				
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel1 * pixel1 */		
				pmullw		xmm6,	xmm6				/* xmm6 = pixel5 * pixel5 */		
				
				movdqa		xmm1,	xmm2				/* xmm1 = pixel1^2 */			
				movdqa		xmm5,	xmm6				/* xmm5 = pixel5^2 */			
				
				movdqa		xmm2,	[edi+32]			/* Pixel 2 */					
				movdqa		xmm6,	[edi+96]			/* Pixel 6 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				paddw		xmm0,	xmm2				/* xmm0 += pixel 2 */			
				paddw		xmm4,	xmm6				/* xmm4 += pixel 6 */			
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel2^2 */			
				pmullw		xmm6,	xmm6				/* xmm6 = pixel6^2 */			
				
				paddw		xmm1,	xmm2				/* xmm1 += pixel2^2 */			
				paddw		xmm5,	xmm6				/* xmm5 += pixel6^2 */			
				
				movdqa		xmm2,	[edi+48]			/* Pixel 3 */					
				movdqa		xmm6,	[edi+112]			/* Pixel 7 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				paddw		xmm0,	xmm2				/* xmm0 += pixel 3 */			
				paddw		xmm4,	xmm6				/* xmm4 += pixel 7 */			
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel3^2 */			
				pmullw		xmm6,	xmm6				/* xmm6 = pixel7^2 */			
				
				paddw		xmm1,	xmm2				/* xmm1 += pixel3^2 */			
				paddw		xmm5,	xmm6				/* xmm5 += pixel7^2 */			
				
				movdqa		xmm2,	[edi+64]			/* Pixel 4 */					
				movdqa		xmm6,	[edi+128]			/* Pixel 8 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				paddw		xmm0,	xmm2				/* xmm0 += pixel 4 */			
				paddw		xmm4,	xmm6				/* xmm4 += pixel 8 */			
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel4^2 */			
				pmullw		xmm6,	xmm6				/* xmm6 = pixel8^2 */			
				
				paddw		xmm1,	xmm2				/* xmm1 = pixel4^2 */			
				paddw		xmm5,	xmm6				/* xmm5 = pixel8^2 */			
				
				/* xmm0 = x1^2 + x2^2 + x3^2 + x4^2 */									
				/* xmm1 = x1 + x2 + x3 + x4 */											
				/* xmm4 = x5^2 + x6^2 + x7^2 + x8^2 */									
				/* xmm5 = x5 + x6 + x7 + x8 */											
				
				movdqa		xmm7,	xmm3				/* xmm7 = xmm3 */					
				psrlw		xmm7,	7					/* xmm7 = 00010001000100010001000100010001 */	
				
				movdqa		xmm2,	xmm0				/* make copy of sum1 */			
				movdqa		xmm6,	xmm4				/* make copy of sum2 */			
				
				paddw		xmm0,	xmm7				/* (sum1 + 1) */				
				paddw		xmm4,	xmm7				/* (sum2 + 1) */				
				
				psraw		xmm2,	1					/* sum1 /2 */					
				psraw		xmm6,	1					/* sum2 /2 */					
				
				psraw		xmm0,	1					/* (sum1 + 1)/2 */				
				psraw		xmm4,	1					/* (sum2 + 1)/2 */				
				
				pmullw		xmm2,	xmm0				/* (sum1)/2*(sum1+1)/2 */		
				pmullw		xmm6,	xmm4				/* (sum2)/2*(sum2+1)/2 */		
				
				psubw		xmm1,	xmm2				/* Variance 1 */				
				psubw		xmm5,	xmm6				/* Variance 2 */				
				
				movdqa		xmm7,	FLimitWMT			/* xmm7 = FLimit */				
				movdqa		xmm2,	xmm1				/* copy of Varinace 1*/

				movdqa		[Variance1], xmm1			/* save the varinace1 */
				movdqa		[Variance2], xmm5			/* save the varinace2 */

				movdqa		xmm6,	xmm5				/* Variance 2 */
				psubw		xmm1,	xmm7				/* Variance 1 < Flimit? */		
				
				psubw		xmm5,	xmm7				/* Variance 2 < Flimit? */		
				psraw		xmm2,	15					/* Variance 1 > 32768? */

				psraw		xmm6,	15					/* Vaiance  2 > 32768? */	
				psraw		xmm1,	15					/* FFFF/0000 for true/false */	
				
				psraw		xmm5,	15					/* FFFF/0000 for true/false */	
				movdqa		xmm7,	[edi+64]			/* xmm0 = Pixel 4			*/	

				pandn		xmm2,	xmm1				/* Variance1<32678 && 
															Variance1<Limit			*/
				pandn		xmm6,	xmm5				/* Variance2<32678 && 
														   Variance1<Limit			*/
				
				movdqa		xmm4,	[edi+80]			/* xmm4 = Pixel 5			*/	
				pand		xmm6,	xmm2				/* xmm6 = Variance1 < Flimit */	
														/*     &&Variance2 < Flimit */	

				movdqa		xmm2,	xmm7				/* make copy of Pixel4		*/	

				psubusw		xmm7,	xmm4				/* 4 - 5 */						
				psubusw		xmm4,	xmm2				/* 5 - 4 */						
				
				por			xmm7,	xmm4				/* abs(4 - 5) */				
				psubw		xmm7,	QStepWMT			/* abs(4-5)<QStepxmmx ? */		
				
				psraw		xmm7,	15					/* FFFF/0000 for True/Flase */
				pand		xmm7,	xmm6													
		
                /* xmm7 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* xmm7 now are in use  */										

                
                /* find the loop filtered values for the pixels on block boundary */
                movdqa      xmm1,       LoopFLimitWMT;   /* Get the Flimit values for loop filter */
                movdqa      xmm3,       [edi + 48]       /* xmm3 = x3 = p[-2] */

                movdqa      xmm4,       [edi + 64]       /* mm4 = x4 = p[-1] */
                movdqa      xmm5,       [edi + 80]       /* mm5 = x5 = p[ 0] */

                movdqa      xmm6,       [edi + 96]       /* mm6 = x6 = p[ 1] */
                psubw       xmm5,       xmm4             /* mm5 = p[ 0] - p[-1] */

                psubw       xmm3,       xmm6             /* mm3 = p[-2] - p[ 1] */
                movdqa      xmm4,       xmm5             /* make a copy */

                paddw       xmm4,       xmm5             /* 2 * ( p[0] - p[-1] ) */
                paddw       xmm3,       EightFours       /* mm3 + 4 */

                paddw       xmm5,       xmm4             /* 3 * ( p[0] - p[-1] ) */
                paddw       xmm3,       xmm5             /* Filtval before shift */

                psraw       xmm3,       3                /* FiltVal */
                movdqa      xmm2,       xmm3             /* make a copy */

                psraw       xmm3,       15               /* FFFF->Neg, 0000->Pos */
                pxor        xmm2,       xmm3

                psubsw      xmm2,       xmm3             /* mm2 = abs(FiltVal) */
                por         xmm3,       EightOnes        /* -1 and 1 for + and - */

                movdqa      xmm4,       xmm1             /* make a copy of Flimit */
                psubw       xmm1,       xmm2             /* mm1= Flimit - abs(FiltVal) */

                movdqa      xmm5,       xmm1             /* copy Flimit - abs(FiltVal) */
                psraw       xmm1,       15               /* FFFF or 0000 */

                pxor        xmm5,       xmm1                 
                psubsw      xmm5,       xmm1             /* abs(Flimit - abs(FiltVal)) */

                psubusw     xmm4,       xmm5             /* Flimit-abs(Flimit - abs(FiltVal)) */
                pmullw      xmm4,       xmm3             /* get the sign back */

                movdqa      xmm1,       [edi+64]         /* p[-1] */
                movdqa      xmm2,       [edi+80]         /* p[0] */
            
                paddw       xmm1,       mm4              /* p[-1] + NewFiltVal */
                psubw       xmm2,       mm4              /* p[0] - NewFiltVal */

                pxor        xmm6,       xmm6             /* clear mm6 */
                packuswb    xmm1,       xmm1             /* clamping */

                packuswb    xmm2,       xmm2
                punpcklbw   xmm1,       xmm6             /* unpack to word */
            
                movdqa      LoopFilteredValuesUp, xmm1   /* save the values */
                punpcklbw   xmm2,       xmm6                 /* unpack to word */

                movdqa      LoopFilteredValuesDown, xmm2 /* save the values */
                
				/* Let's do the filtering now */										
				/* p1 = (abs(Src[-4] - Src[-5]) < QStep ) ?  Src[-5] : Src[-4]; */		
				/* p2 = (abs(Src[+3] - Src[+4]) < QStep ) ?  Src[+4] : Src[+3]; */		
				
				movdqa		xmm5,	[edi]				/* xmm5 = -5 */					
				movdqa		xmm4,	[edi + 16]			/* xmm4 = -4 */					
				
				movdqa		xmm3,	xmm4				/* copy of -4 */				
				movdqa		xmm6,	xmm5				/* copy of -5 */				
				
				psubusw		xmm4,	xmm6				/* xmm4 = [-4] - [-5] */			
				psubusw		xmm5,	xmm3				/* xmm5 = [-5] - [-4] */			
				
				por			xmm4,	xmm5				/* abs([-4]-[-5] ) */			
				psubw		xmm4,	QStepWMT			/* abs([-4]-[-5] )<QStep? */	
				
				psraw		xmm4,	15					/* FFFF/0000 for True/False */	
				movdqa		xmm1,	xmm4				/* copy of the xmm4 */			
				
				pand		xmm4,	xmm6				/*							*/	
				pandn		xmm1,	xmm3				/*							*/	
				
				por			xmm1,	xmm4				/* xmm1 = p1				*/	
				
				/* now find P2 */														
				
				movdqa		xmm4,	[edi+128]			/* xmm4 = [3] */					
				movdqa		xmm5,	[edi+144]			/* xmm5 = [4] */					
				
				movdqa		xmm3,	xmm4				/* copy of 3 */					
				movdqa		xmm6,	xmm5				/* copy of 4 */					
				
				psubusw		xmm4,	xmm6				/* xmm4 = [3] - [4] */			
				psubusw		xmm5,	xmm3				/* xmm5 = [4] - [3] */			
				
				por			xmm4,	xmm5				/* abs([3]-[4] ) */				
				psubw		xmm4,	QStepWMT			/* abs([3]-[4] )<QStep? */		
				
				psraw		xmm4,	15					/* FFFF/0000 for True/False */	
				movdqa		xmm2,	xmm4				/* copy of the xmm4 */			
				
				pand		xmm4,	xmm6				/*							*/	
				pandn		xmm2,	xmm3				/*							*/	
				
				por			xmm2,	xmm4				/* xmm2 = p2				*/	

				/* Data is ready, now do the filtering */
				
				pxor		xmm0,	xmm0				/* clear xmm0 */

				/* sum = p1 + p1 + p1 + x1 + x2 + x3 + x4 + 4; */				
				/* Des[-w4] = (((sum + x1) << 1) - (x4 - x5)) >> 4; */			
				/* Des[-w4] = Src[-w4]; */												
				/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	

				
				movdqa		xmm3,	xmm1				/* xmm3 = p1 */					
				paddw		xmm3,	xmm3				/* xmm3 = p1 + p1 */				
				
				paddw		xmm3,	xmm1				/* xmm3 = p1 + p1 + p1 */		
				movdqa		xmm4,	[edi+16]			/* xmm4 = x1 */					
				
				paddw		xmm3,	[edi+32]			/* xmm3 = p1+p1+p1+ x2 */		
				paddw		xmm4,	[edi+48]			/* xmm4 = x1+x3 */				
				
				paddw		xmm3,	[edi+64]			/* xmm3 += x4 */					
				paddw		xmm4,	EightFours			/* xmm4 = x1 + x3 + 4 */			
				
				paddw		xmm3,	xmm4				/* xmm3 = 3*p1+x1+x2+x3+x4+4 */	
				movdqa		xmm4,	xmm3				/* xmm4 = xmm3 */					
				
				movdqa		xmm5,	[edi+16]			/* xmm5 = x1 */					
				paddw		xmm4,	xmm5				/* xmm4 = sum+x1 */				
				
				psllw		xmm4,	1					/* xmm4 = (sum+x1)<<1 */			
				psubw		xmm4,	[edi+64]			/* xmm4 = (sum+x1)<<1-x4 */		
				
				paddw		xmm4,	[edi+80]			/* xmm4 = (sum+x1)<<1-x4+x5 */	
				psraw		xmm4,	4					/* xmm4 >>=4 */					
				
				psubw		xmm4,	xmm5				/* New Value - old Value */		
				pand		xmm4,	xmm7				/* And the flag */				
				
				paddw		xmm4,	xmm5				/* add the old value back */	
				packuswb	xmm4,	xmm0				/* pack it to bytes */
				
				movq		QWORD PTR [esi+edx*2], xmm4	/* Write new x1 */				
				
				/* sum += x5 -p1 */														
				/* Des[-w3]=((sum+x2)<<1-x5+x6)>>4 */									
				
				movdqa		xmm5,	[edi+32]			/* xmm5= x2 */					
				psubw		xmm3,	xmm1				/* sum=sum-p1 */				
				
				paddw		xmm3,    [edi+80]			/* sum=sum+x5 */				
				movdqa		xmm4,	xmm5				/* copy sum */					
				
				paddw		xmm4,	xmm3				/* xmm4=sum+x2 */				
				paddw		xmm4,	xmm4				/* xmm4 <<= 1 */					
				
				psubw		xmm4,	[edi+80]			/* xmm4 =(sum+x2)<<1-x5 */		
				paddw		xmm4,	[edi+96]			/* xmm4 =(sum+x2)<<1-x5+x6 */	
				
				psraw		xmm4,	4					/* xmm4=((sum+x2)<<1-x5+x6)>>4 */
				psubw		xmm4,	xmm5				/* new value - old value	*/	
				
				pand		xmm4,	xmm7				/* And the flag */				
				paddw		xmm4,	xmm5				/* add the old value back */	

				packuswb	xmm4,	xmm0				/* pack it to bytes */
				movq		QWORD PTR [esi+edx], xmm4	/* write new x2 */				
				
				/* sum += x6 - p1 */													
				/* Des[-w2]=((sum+x[3])<<1-x[6]+x[7])>>4 */								
				
				movdqa		xmm5,	[edi+48]			/* xmm5= x3 */					
				psubw		xmm3,	xmm1				/* sum=sum-p1 */				
				
				paddw		xmm3,    [edi+96]			/* sum=sum+x6 */				
				movdqa		xmm4,	xmm5				/* copy x3 */					
				
				paddw		xmm4,	xmm3				/* xmm4=sum+x3 */				
				paddw		xmm4,	xmm4				/* xmm4 <<= 1 */					
				
				psubw		xmm4,	[edi+96]			/* xmm4 =(sum+x3)<<1-x6 */		
				paddw		xmm4,	[edi+112]			/* xmm4 =(sum+x3)<<1-x6+x7 */	
				
				psraw		xmm4,	4					/* xmm4=((sum+x3)<<1-x6+x7)>>4 */
				psubw		xmm4,	xmm5				/* new value - old value	*/	
				
				pand		xmm4,	xmm7				/* And the flag */				
				paddw		xmm4,	xmm5				/* add the old value back */	
				
				packuswb	xmm4,	xmm0				/* pack it to bytes */
				movq		QWORD PTR [esi],xmm4		/* write new x3 */				
				
				/* sum += x7 - p1 */													
				/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
				
				movdqa		xmm5,	[edi+64]			/* xmm5 = x4 */					
				psubw		xmm3,	xmm1				/* sum = sum-p1 */				
				
				paddw		xmm3,	[edi+112]			/* sum = sum+x7 */				
				movdqa		xmm4,	xmm5				/* xmm4 = x4 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum + x4 */			
				paddw		xmm4,	xmm4				/* xmm4 *=2 */					
				
				paddw		xmm4,	xmm1				/* += p1 */						
				psubw		xmm4,	[edi+16]			/* -= x1 */						
				
				psubw		xmm4,	[edi+112]			/* -= x7 */						
				paddw		xmm4,	[edi+128]			/* += x8 */						
				
				movdqa      xmm5,   LoopFilteredValuesUp /* Read the loop filtered value of x4 */
                psraw		xmm4,	4					/* >>=4 */						

				psubw		xmm4,	xmm5				/* -=x4 */						
				pand		xmm4,	xmm7				/* and flag */					

				paddw		xmm4,	xmm5				/* += x4 */										
				packuswb	xmm4,	xmm0				/* pack it to bytes */

				movq	    QWORD PTR [esi+ecx], xmm4	/* write new x4 */				
				
				/* sum+= x8-x1 */														
				/* Des[0]=((sum+x5)<<1+x1-x2-x8+p2)>>4 */								
				
				movdqa		xmm5,	[edi+80]			/* xmm5 = x5 */					
				psubw		xmm3,	[edi+16]			/* sum -= x1 */					
				
				paddw		xmm3,	[edi+128]			/* sub += x8 */					
				movdqa		xmm4,	xmm5				/* xmm4 = x5 */					
				
				paddw		xmm4,	xmm3				/* xmm4= sum+x5 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2 */					
				
				paddw		xmm4,	[edi+16]			/* += x1 */						
				psubw		xmm4,	[edi+32]			/* -= x2 */						
				
				psubw		xmm4,	[edi+128]			/* -= x8 */						
				paddw		xmm4,	xmm2				/* += p2 */						
				
                movdqa      xmm5,   LoopFilteredValuesDown /* Read the loop filtered value of x5 */
				psraw		xmm4,	4					/* >>=4 */						

				psubw		xmm4,	xmm5				/* -=x5 */						
				pand		xmm4,	xmm7				/* and flag */					

				paddw		xmm4,	xmm5				/* += x5 */						
				lea			esi,	[esi+ecx*4]			/* esi=des + 2*pitch */

				packuswb	xmm4,	xmm0				/* pack to bytes */
				movq		QWORD PTR [esi+edx*2], xmm4	/* write new x5 */				
				
				/* sum += p2 - x2 */													
				/* Des[w1] = ((sum+x6)<<1 + x2-x3)>>4 */								
				
				movdqa		xmm5,	[edi+96]			/* xmm5 = x6 */					
				psubw		xmm3,	[edi+32]			/* -= x2 */						
				
				paddw		xmm3,	xmm2				/* += p2 */						
				movdqa		xmm4,	xmm5				/* xmm4 = x6 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum+x6 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2*/					
				
				paddw		xmm4,	[edi+32]			/* +=x2 */						
				psubw		xmm4,	[edi+48]			/* -=x3 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x6 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x6 */						
				
				packuswb	xmm4,	xmm0				/* pack to bytes */
				movq		QWORD PTR [esi+edx], xmm4	/* write new x6 */				
				
				/* sum += p2 - x3 */													
				/* Des[w2] = ((sum+x7)<<1 + x3-x4)>>4 */								
				
				movdqa		xmm5,	[edi+112]			/* xmm5 = x7 */					
				psubw		xmm3,	[edi+48]			/* -= x3 */						
				
				paddw		xmm3,	xmm2				/* += p2 */						
				movdqa		xmm4,	xmm5				/* xmm4 = x7 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum+x7 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2*/					
				
				paddw		xmm4,	[edi+48]			/* +=x3 */						
				psubw		xmm4,	[edi+64]			/* -=x4 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x7 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x7 */						

				packuswb	xmm4,	xmm0				/* pack to bytes */				
				movq		QWORD PTR [esi],xmm4		/* write new x7 */				
				
				/* sum += p2 - x4 */													
				/* Des[w3] = ((sum+x8)<<1 + x4-x5)>>4 */								
				
				movdqa		xmm5,	[edi+128]			/* xmm5 = x8 */					
				psubw		xmm3,	[edi+64]			/* -= x4 */						
				
				paddw		xmm3,	xmm2				/* += p2 */						
				movdqa		xmm4,	xmm5				/* xmm4 = x8 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum+x8 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2*/					
				
				paddw		xmm4,	[edi+64]			/* +=x4 */						
				psubw		xmm4,	[edi+80]			/* -=x5 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x8 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x8 */						

				packuswb	xmm4,	xmm0				/* pack to bytes */				
				movq		QWORD PTR [esi+ecx], xmm4				/* write new x8 */				

				pop			edi
				pop			esi
				pop			edx
				pop			ecx
				pop			eax

			
        } /* end of the macro */
        
    	Var1=Variance1[0]+Variance1[1]+Variance1[2]+Variance1[3]+Variance1[4]+Variance1[5]+Variance1[6]+Variance1[7];
	    Var2=Variance2[0]+Variance2[1]+Variance2[2]+Variance2[3]+Variance2[4]+Variance2[5]+Variance2[6]+Variance2[7];
        pbi->FragmentVariances[CurrentFrag] += Var1;        
        pbi->FragmentVariances[CurrentFrag + FragAcross] += Var2;
        

        if(CurrentFrag==StartFrag)
			CurrentFrag++;
		else
		{
			
			Des=DesPtr-8*PlaneLineStep+8*(CurrentFrag-StartFrag);
			Src=Des;

			QStep = QuantScale[pbi->FragQIndex[CurrentFrag]];		
			QStepWMT[0] = (INT16)QStep;
            QStepWMT[1] = (INT16)QStep;
            QStepWMT[2] = (INT16)QStep;
            QStepWMT[3] = (INT16)QStep;
            QStepWMT[4] = (INT16)QStep;
            QStepWMT[5] = (INT16)QStep;
            QStepWMT[6] = (INT16)QStep;
            QStepWMT[7] = (INT16)QStep;

		    for( j=0; j<8;j++)
		    {
    			Rows[j] = (short) (Src[-5 +j*PlaneLineStep]);
	    		Rows[72+j] = (short)(Src[4+j*PlaneLineStep]);		
    		}

			__asm
			{
				/* Save the registers */
				push		eax
				push		ecx			
				push		edx
				push		esi
				push		edi
				
				/* Calculate the FLimit and store FLimit and QStep */					
				
				movdqa		xmm0,	QStepWMT            /* Get QStep */
				movdqa		xmm1,	EightThrees			/* mm1 = 03030303 */			

                pmullw		xmm1,	xmm0				/* mm1 = QStep * 3 */							
				pmullw		xmm1,	xmm0				/* mm1 = QStep * QStep * 3 */					
				
                psrlw		xmm1,	5					/* mm1 = FLimit */				
				movdqa		[FLimitWMT], xmm1			/* Save FLimit */				

				/* setup the pointers to data */

				mov			eax,	Src					/* eax = Src */
				xor			edx,	edx					/* clear edx */
				
				mov			esi,	Des					/* esi = Des */
				sub			eax,	4					/* eax = Src-4 */

				sub			esi,	4					/* esi = Des-4 */
				lea			edi,	Rows				/* edi = Rows */				

				mov			ecx,	PlaneLineStep		/* ecx = Pitch */				
				sub			edx,	ecx					/* edx = -Pitch */				

				lea			esi,	[esi+ecx*2]			/* esi = Des-4 + 2 * Pitch */
				
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

				/* we use xmm0,xmm1,xmm2 for 1234 and xmm4, xmm5, xmm6 for 5-8 */				
				/* xmm7 = 0, xmm3 = {128, 128, 128, 128, 128, 128, 128, 128} */								
				
				pcmpeqw		xmm3,	xmm3				/* xmm3 = FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */	
				psllw		xmm3,	15					/* xmm3 = 80008000800080008000800080008000 */	
				psrlw		xmm3,	8					/* xmm3 = 00800080008000800080008000800080 */
				
				movdqa		xmm2,	[edi+16]			/* Pixel 1 */					
				movdqa		xmm6,	[edi+80]			/* Pixel 5 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				movdqa		xmm0,	xmm2				/* xmm0 = pixel 1 */				
				movdqa		xmm4,	xmm6				/* xmm4 = pixel 5 */				
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel1 * pixel1 */		
				pmullw		xmm6,	xmm6				/* xmm6 = pixel5 * pixel5 */		
				
				movdqa		xmm1,	xmm2				/* xmm1 = pixel1^2 */			
				movdqa		xmm5,	xmm6				/* xmm5 = pixel5^2 */			
				
				movdqa		xmm2,	[edi+32]			/* Pixel 2 */					
				movdqa		xmm6,	[edi+96]			/* Pixel 6 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				paddw		xmm0,	xmm2				/* xmm0 += pixel 2 */			
				paddw		xmm4,	xmm6				/* xmm4 += pixel 6 */			
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel2^2 */			
				pmullw		xmm6,	xmm6				/* xmm6 = pixel6^2 */			
				
				paddw		xmm1,	xmm2				/* xmm1 += pixel2^2 */			
				paddw		xmm5,	xmm6				/* xmm5 += pixel6^2 */			
				
				movdqa		xmm2,	[edi+48]			/* Pixel 3 */					
				movdqa		xmm6,	[edi+112]			/* Pixel 7 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				paddw		xmm0,	xmm2				/* xmm0 += pixel 3 */			
				paddw		xmm4,	xmm6				/* xmm4 += pixel 7 */			
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel3^2 */			
				pmullw		xmm6,	xmm6				/* xmm6 = pixel7^2 */			
				
				paddw		xmm1,	xmm2				/* xmm1 += pixel3^2 */			
				paddw		xmm5,	xmm6				/* xmm5 += pixel7^2 */			
				
				movdqa		xmm2,	[edi+64]			/* Pixel 4 */					
				movdqa		xmm6,	[edi+128]			/* Pixel 8 */					
				
				psubw		xmm2,	xmm3				/* xmm2 -=128 */					
				psubw		xmm6,	xmm3				/* xmm6 -=128 */					
				
				paddw		xmm0,	xmm2				/* xmm0 += pixel 4 */			
				paddw		xmm4,	xmm6				/* xmm4 += pixel 8 */			
				
				pmullw		xmm2,	xmm2				/* xmm2 = pixel4^2 */			
				pmullw		xmm6,	xmm6				/* xmm6 = pixel8^2 */			
				
				paddw		xmm1,	xmm2				/* xmm1 = pixel4^2 */			
				paddw		xmm5,	xmm6				/* xmm5 = pixel8^2 */			
				
				/* xmm0 = x1^2 + x2^2 + x3^2 + x4^2 */									
				/* xmm1 = x1 + x2 + x3 + x4 */											
				/* xmm4 = x5^2 + x6^2 + x7^2 + x8^2 */									
				/* xmm5 = x5 + x6 + x7 + x8 */											
				
				movdqa		xmm7,	xmm3				/* xmm7 = xmm3 */					
				psrlw		xmm7,	7					/* xmm7 = 00010001000100010001000100010001 */	
				
				movdqa		xmm2,	xmm0				/* make copy of sum1 */			
				movdqa		xmm6,	xmm4				/* make copy of sum2 */			
				
				paddw		xmm0,	xmm7				/* (sum1 + 1) */				
				paddw		xmm4,	xmm7				/* (sum2 + 1) */				
				
				psraw		xmm2,	1					/* sum1 /2 */					
				psraw		xmm6,	1					/* sum2 /2 */					
				
				psraw		xmm0,	1					/* (sum1 + 1)/2 */				
				psraw		xmm4,	1					/* (sum2 + 1)/2 */				
				
				pmullw		xmm2,	xmm0				/* (sum1)/2*(sum1+1)/2 */		
				pmullw		xmm6,	xmm4				/* (sum2)/2*(sum2+1)/2 */		
				
				psubw		xmm1,	xmm2				/* Variance 1 */				
				psubw		xmm5,	xmm6				/* Variance 2 */				
				
				movdqa		xmm7,	FLimitWMT			/* xmm7 = FLimit */				
				movdqa		xmm2,	xmm1				/* copy of Varinace 1*/

                movdqa		[Variance1], xmm1			/* save the varinace1 */
				movdqa		[Variance2], xmm5			/* save the varinace2 */

				movdqa		xmm6,	xmm5				/* Variance 2 */
				psubw		xmm1,	xmm7				/* Variance 1 < Flimit? */		
				
				psubw		xmm5,	xmm7				/* Variance 2 < Flimit? */		
				psraw		xmm2,	15					/* Variance 1 > 32768? */

				psraw		xmm6,	15					/* Vaiance  2 > 32768? */	
				psraw		xmm1,	15					/* FFFF/0000 for true/false */	
				
				psraw		xmm5,	15					/* FFFF/0000 for true/false */	
				movdqa		xmm7,	[edi+64]			/* xmm0 = Pixel 4			*/	

				pandn		xmm2,	xmm1				/* Variance1<32678 && 
															Variance1<Limit			*/
				pandn		xmm6,	xmm5				/* Variance2<32678 && 
														   Variance1<Limit			*/
				
				movdqa		xmm4,	[edi+80]			/* xmm4 = Pixel 5			*/	
				pand		xmm6,	xmm2				/* xmm6 = Variance1 < Flimit */	
														/*     &&Variance2 < Flimit */	

				movdqa		xmm2,	xmm7				/* make copy of Pixel4		*/	

				psubusw		xmm7,	xmm4				/* 4 - 5 */						
				psubusw		xmm4,	xmm2				/* 5 - 4 */						
				
				por			xmm7,	xmm4				/* abs(4 - 5) */				
				psubw		xmm7,	QStepWMT			/* abs(4-5)<QStepxmmx ? */		
				
				psraw		xmm7,	15					/* FFFF/0000 for True/Flase */
				pand		xmm7,	xmm6													
				
				/* xmm7 = Variance 1< Flimit && Variance 2<Flimit && abs(4-5)<QStep */	
				/* xmm7 now are in use  */										
                /* find the loop filtered values for the pixels on block boundary */
                movdqa      xmm1,       LoopFLimitWMT;   /* Get the Flimit values for loop filter */
                movdqa      xmm3,       [edi + 48]       /* xmm3 = x3 = p[-2] */

                movdqa      xmm4,       [edi + 64]       /* mm4 = x4 = p[-1] */
                movdqa      xmm5,       [edi + 80]       /* mm5 = x5 = p[ 0] */

                movdqa      xmm6,       [edi + 96]       /* mm6 = x6 = p[ 1] */
                psubw       xmm5,       xmm4             /* mm5 = p[ 0] - p[-1] */

                psubw       xmm3,       xmm6             /* mm3 = p[-2] - p[ 1] */
                movdqa      xmm4,       xmm5             /* make a copy */

                paddw       xmm4,       xmm5             /* 2 * ( p[0] - p[-1] ) */
                paddw       xmm3,       EightFours       /* mm3 + 4 */

                paddw       xmm5,       xmm4             /* 3 * ( p[0] - p[-1] ) */
                paddw       xmm3,       xmm5             /* Filtval before shift */

                psraw       xmm3,       3                /* FiltVal */
                movdqa      xmm2,       xmm3             /* make a copy */

                psraw       xmm3,       15               /* FFFF->Neg, 0000->Pos */
                pxor        xmm2,       xmm3

                psubsw      xmm2,       xmm3             /* mm2 = abs(FiltVal) */
                por         xmm3,       EightOnes        /* -1 and 1 for + and - */

                movdqa      xmm4,       xmm1             /* make a copy of Flimit */
                psubw       xmm1,       xmm2             /* mm1= Flimit - abs(FiltVal) */

                movdqa      xmm5,       xmm1             /* copy Flimit - abs(FiltVal) */
                psraw       xmm1,       15               /* FFFF or 0000 */

                pxor        xmm5,       xmm1                 
                psubsw      xmm5,       xmm1             /* abs(Flimit - abs(FiltVal)) */

                psubusw     xmm4,       xmm5             /* Flimit-abs(Flimit - abs(FiltVal)) */
                pmullw      xmm4,       xmm3             /* get the sign back */

                movdqa      xmm1,       [edi+64]         /* p[-1] */
                movdqa      xmm2,       [edi+80]         /* p[0] */
            
                paddw       xmm1,       mm4              /* p[-1] + NewFiltVal */
                psubw       xmm2,       mm4              /* p[0] - NewFiltVal */

                pxor        xmm6,       xmm6             /* clear mm6 */
                packuswb    xmm1,       xmm1             /* clamping */

                packuswb    xmm2,       xmm2
                punpcklbw   xmm1,       xmm6             /* unpack to word */
            
                movdqa      LoopFilteredValuesUp, xmm1   /* save the values */
                punpcklbw   xmm2,       xmm6                 /* unpack to word */

                movdqa      LoopFilteredValuesDown, xmm2 /* save the values */

                /* Let's do the filtering now */										
				/* p1 = (abs(Src[-4] - Src[-5]) < QStep ) ?  Src[-5] : Src[-4]; */		
				/* p2 = (abs(Src[+3] - Src[+4]) < QStep ) ?  Src[+4] : Src[+3]; */		
				
				movdqa		xmm5,	[edi]				/* xmm5 = -5 */					
				movdqa		xmm4,	[edi + 16]			/* xmm4 = -4 */					
				
				movdqa		xmm3,	xmm4				/* copy of -4 */				
				movdqa		xmm6,	xmm5				/* copy of -5 */				
				
				psubusw		xmm4,	xmm6				/* xmm4 = [-4] - [-5] */			
				psubusw		xmm5,	xmm3				/* xmm5 = [-5] - [-4] */			
				
				por			xmm4,	xmm5				/* abs([-4]-[-5] ) */			
				psubw		xmm4,	QStepWMT			/* abs([-4]-[-5] )<QStep? */	
				
				psraw		xmm4,	15					/* FFFF/0000 for True/False */	
				movdqa		xmm1,	xmm4				/* copy of the xmm4 */			
				
				pand		xmm4,	xmm6				/*							*/	
				pandn		xmm1,	xmm3				/*							*/	
				
				por			xmm1,	xmm4				/* xmm1 = p1				*/	
				
				/* now find P2 */														
				
				movdqa		xmm4,	[edi+128]			/* xmm4 = [3] */					
				movdqa		xmm5,	[edi+144]			/* xmm5 = [4] */					
				
				movdqa		xmm3,	xmm4				/* copy of 3 */					
				movdqa		xmm6,	xmm5				/* copy of 4 */					
				
				psubusw		xmm4,	xmm6				/* xmm4 = [3] - [4] */			
				psubusw		xmm5,	xmm3				/* xmm5 = [4] - [3] */			
				
				por			xmm4,	xmm5				/* abs([3]-[4] ) */				
				psubw		xmm4,	QStepWMT			/* abs([3]-[4] )<QStep? */		
				
				psraw		xmm4,	15					/* FFFF/0000 for True/False */	
				movdqa		xmm2,	xmm4				/* copy of the xmm4 */			
				
				pand		xmm4,	xmm6				/*							*/	
				pandn		xmm2,	xmm3				/*							*/	
				
				por			xmm2,	xmm4				/* xmm2 = p2				*/	

				/* Data is ready, now do the filtering */
				
				pxor		xmm0,	xmm0				/* clear xmm0 */

				/* sum = p1 + p1 + p1 + x1 + x2 + x3 + x4 + 4; */				
				/* Des[-w4] = (((sum + x1) << 1) - (x4 - x5)) >> 4; */			
				/* Des[-w4] = Src[-w4]; */												
				/* which is equivalent to Src[-w4] + flag * ( newvalue - Src[-w4] */	

				
				movdqa		xmm3,	xmm1				/* xmm3 = p1 */					
				paddw		xmm3,	xmm3				/* xmm3 = p1 + p1 */				
				
				paddw		xmm3,	xmm1				/* xmm3 = p1 + p1 + p1 */		
				movdqa		xmm4,	[edi+16]			/* xmm4 = x1 */					
				
				paddw		xmm3,	[edi+32]			/* xmm3 = p1+p1+p1+ x2 */		
				paddw		xmm4,	[edi+48]			/* xmm4 = x1+x3 */				
				
				paddw		xmm3,	[edi+64]			/* xmm3 += x4 */					
				paddw		xmm4,	EightFours			/* xmm4 = x1 + x3 + 4 */			
				
				paddw		xmm3,	xmm4				/* xmm3 = 3*p1+x1+x2+x3+x4+4 */	
				movdqa		xmm4,	xmm3				/* xmm4 = xmm3 */					
				
				movdqa		xmm5,	[edi+16]			/* xmm5 = x1 */					
				paddw		xmm4,	xmm5				/* xmm4 = sum+x1 */				
				
				psllw		xmm4,	1					/* xmm4 = (sum+x1)<<1 */			
				psubw		xmm4,	[edi+64]			/* xmm4 = (sum+x1)<<1-x4 */		
				
				paddw		xmm4,	[edi+80]			/* xmm4 = (sum+x1)<<1-x4+x5 */	
				psraw		xmm4,	4					/* xmm4 >>=4 */					
				
				psubw		xmm4,	xmm5				/* New Value - old Value */		
				pand		xmm4,	xmm7				/* And the flag */				
				
				paddw		xmm4,	xmm5				/* add the old value back */	
				packuswb	xmm4,	xmm0				/* pack it to bytes */
				
				movdq2q		mm0,	xmm4				/* Write new x1 */				
				
				/* sum += x5 -p1 */														
				/* Des[-w3]=((sum+x2)<<1-x5+x6)>>4 */									
				
				movdqa		xmm5,	[edi+32]			/* xmm5= x2 */					
				psubw		xmm3,	xmm1				/* sum=sum-p1 */				
				
				paddw		xmm3,    [edi+80]			/* sum=sum+x5 */				
				movdqa		xmm4,	xmm5				/* copy sum */					
				
				paddw		xmm4,	xmm3				/* xmm4=sum+x2 */				
				paddw		xmm4,	xmm4				/* xmm4 <<= 1 */					
				
				psubw		xmm4,	[edi+80]			/* xmm4 =(sum+x2)<<1-x5 */		
				paddw		xmm4,	[edi+96]			/* xmm4 =(sum+x2)<<1-x5+x6 */	
				
				psraw		xmm4,	4					/* xmm4=((sum+x2)<<1-x5+x6)>>4 */
				psubw		xmm4,	xmm5				/* new value - old value	*/	
				
				pand		xmm4,	xmm7				/* And the flag */				
				paddw		xmm4,	xmm5				/* add the old value back */	

				packuswb	xmm4,	xmm0				/* pack it to bytes */
				movdq2q		mm1,	xmm4				/* write new x2 */				
				
				/* sum += x6 - p1 */													
				/* Des[-w2]=((sum+x[3])<<1-x[6]+x[7])>>4 */								
				
				movdqa		xmm5,	[edi+48]			/* xmm5= x3 */					
				psubw		xmm3,	xmm1				/* sum=sum-p1 */				
				
				paddw		xmm3,    [edi+96]			/* sum=sum+x6 */				
				movdqa		xmm4,	xmm5				/* copy x3 */					
				
				paddw		xmm4,	xmm3				/* xmm4=sum+x3 */				
				paddw		xmm4,	xmm4				/* xmm4 <<= 1 */					
				
				psubw		xmm4,	[edi+96]			/* xmm4 =(sum+x3)<<1-x6 */		
				paddw		xmm4,	[edi+112]			/* xmm4 =(sum+x3)<<1-x6+x7 */	
				
				psraw		xmm4,	4					/* xmm4=((sum+x3)<<1-x6+x7)>>4 */
				psubw		xmm4,	xmm5				/* new value - old value	*/	
				
				pand		xmm4,	xmm7				/* And the flag */				
				paddw		xmm4,	xmm5				/* add the old value back */	
				
				packuswb	xmm4,	xmm0				/* pack it to bytes */
				movdq2q		mm2,	xmm4				/* write new x3 */				
				
				/* sum += x7 - p1 */													
				/* Des[-w1]=((sum+x4)<<1+p1-x1-x7+x8]>>4 */						
				
				movdqa		xmm5,	[edi+64]			/* xmm5 = x4 */					
				psubw		xmm3,	xmm1				/* sum = sum-p1 */				
				
				paddw		xmm3,	[edi+112]			/* sum = sum+x7 */				
				movdqa		xmm4,	xmm5				/* xmm4 = x4 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum + x4 */			
				paddw		xmm4,	xmm4				/* xmm4 *=2 */					
				
				paddw		xmm4,	xmm1				/* += p1 */						
				psubw		xmm4,	[edi+16]			/* -= x1 */						
				
				psubw		xmm4,	[edi+112]			/* -= x7 */						
				paddw		xmm4,	[edi+128]			/* += x8 */						
				
				movdqa      xmm5,   LoopFilteredValuesUp /* Read the loop filtered value of x4 */
                psraw		xmm4,	4					/* >>=4 */						

                psubw		xmm4,	xmm5				/* -=x4 */						
				pand		xmm4,	xmm7				/* and flag */					

                paddw		xmm4,	xmm5				/* += x4 */						
				packuswb	xmm4,	xmm0				/* pack it to bytes */

                movdq2q		mm3,	xmm4				/* write new x4 */				
				

				/* sum+= x8-x1 */														
				/* Des[0]=((sum+x5)<<1+x1-x2-x8+p2)>>4 */								
				
				movdqa		xmm5,	[edi+80]			/* xmm5 = x5 */					
				psubw		xmm3,	[edi+16]			/* sum -= x1 */					
				
				paddw		xmm3,	[edi+128]			/* sub += x8 */					
				movdqa		xmm4,	xmm5				/* xmm4 = x5 */					
				
				paddw		xmm4,	xmm3				/* xmm4= sum+x5 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2 */					
				
				paddw		xmm4,	[edi+16]			/* += x1 */						
				psubw		xmm4,	[edi+32]			/* -= x2 */						
				
				psubw		xmm4,	[edi+128]			/* -= x8 */						
				paddw		xmm4,	xmm2				/* += p2 */						

				movdqa      xmm5,   LoopFilteredValuesDown /*  Read the loop filtered value of x4 */
   				psraw		xmm4,	4					/* >>=4 */						

                psubw		xmm4,	xmm5				/* -=x5 */						
				pand		xmm4,	xmm7				/* and flag */					

                paddw		xmm4,	xmm5				/* += x5 */						
				packuswb	xmm4,	xmm0				/* pack to bytes */

                movdq2q		mm4,	xmm4				/* write new x5 */				
				
				/* sum += p2 - x2 */													
				/* Des[w1] = ((sum+x6)<<1 + x2-x3)>>4 */								
				
				movdqa		xmm5,	[edi+96]			/* xmm5 = x6 */					
				psubw		xmm3,	[edi+32]			/* -= x2 */						
				
				paddw		xmm3,	xmm2				/* += p2 */						
				movdqa		xmm4,	xmm5				/* xmm4 = x6 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum+x6 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2*/					
				
				paddw		xmm4,	[edi+32]			/* +=x2 */						
				psubw		xmm4,	[edi+48]			/* -=x3 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x6 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x6 */						
				
				packuswb	xmm4,	xmm0				/* pack to bytes */
				movdq2q		mm5,	xmm4				/* write new x6 */				
				
				/* sum += p2 - x3 */													
				/* Des[w2] = ((sum+x7)<<1 + x3-x4)>>4 */								
				
				movdqa		xmm5,	[edi+112]			/* xmm5 = x7 */					
				psubw		xmm3,	[edi+48]			/* -= x3 */						
				
				paddw		xmm3,	xmm2				/* += p2 */						
				movdqa		xmm4,	xmm5				/* xmm4 = x7 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum+x7 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2*/					
				
				paddw		xmm4,	[edi+48]			/* +=x3 */						
				psubw		xmm4,	[edi+64]			/* -=x4 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x7 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x7 */						

				packuswb	xmm4,	xmm0				/* pack to bytes */				
				movdq2q		mm6,	xmm4				/* write new x7 */				
				
				/* sum += p2 - x4 */													
				/* Des[w3] = ((sum+x8)<<1 + x4-x5)>>4 */								
				
				movdqa		xmm5,	[edi+128]			/* xmm5 = x8 */					
				psubw		xmm3,	[edi+64]			/* -= x4 */						
				
				paddw		xmm3,	xmm2				/* += p2 */						
				movdqa		xmm4,	xmm5				/* xmm4 = x8 */					
				
				paddw		xmm4,	xmm3				/* xmm4 = sum+x8 */				
				paddw		xmm4,	xmm4				/* xmm4 *= 2*/					
				
				paddw		xmm4,	[edi+64]			/* +=x4 */						
				psubw		xmm4,	[edi+80]			/* -=x5 */						
				
				psraw		xmm4,	4					/* >>=4 */						
				psubw		xmm4,	xmm5				/* -=x8 */						
				
				pand		xmm4,	xmm7				/* and flag */					
				paddw		xmm4,	xmm5				/* += x8 */						

				packuswb	xmm4,	xmm0				/* pack to bytes */				
				movdq2q		mm7,	xmm4				/* write new x8 */				


				/* transpose */
				movq2dq		xmm0,	mm0					/* xmm0 = 70 60 50 40 30 20 10 00 */
				movq2dq		xmm1,	mm1					/* xmm1 = 71 61 51 41 31 21 11 01 */

				movq2dq		xmm2,	mm2					/* xmm2 = 72 62 52 42 32 22 12 02 */
				movq2dq		xmm3,	mm3					/* xmm3 = 73 63 53 43 33 23 13 03 */

				punpcklbw	xmm0,	xmm1				/* xmm0 = 7170 6160 5150 4140 3130 2120 1110 0100 */
				punpcklbw	xmm2,	xmm3				/* xmm2 = 7372 6362 5352 4342 3332 2322 1312 0302 */

				movdqa		xmm1,	xmm0				/* xmm1 = 7170 6160 5150 4140 3130 2120 1110 0100 */
				punpcklwd	xmm0,	xmm2				/* xmm0 = 33323130 23222120 13121110 03020100 */

				punpckhwd	xmm1,	xmm2				/* xmm1 = 73727170 63626160 53525150 43424140 */
				
				movq2dq		xmm4,	mm4					/* xmm4 = 74 64 54 44 34 24 14 04 */
				movq2dq		xmm5,	mm5					/* xmm5 = 75 65 55 45 35 25 15 05 */				

				movq2dq		xmm6,	mm6 				/* xmm6 = 76 66 56 46 36 26 16 06 */
				movq2dq		xmm7,	mm7					/* xmm7 = 77 67 57 47 37 27 17 07 */
								
				punpcklbw	xmm4,	xmm5				/* xmm4 = 7574 6564 5554 4544 3534 2524 1514 0504 */
				punpcklbw	xmm6,	xmm7				/* xmm6 = 7776 6766 5756 4746 3736 2726 1716 0706 */

				movdqa		xmm5,	xmm4				/* xmm5 = 7574 6564 5554 4544 3534 2524 1514 0504 */
				punpcklwd	xmm4,	xmm6				/* xmm4 = 37363534 27262524 17161514 07060504 */

				punpckhwd	xmm5,	xmm6				/* xmm5 = 77767574 67666564 57565554 47464544 */
				movdqa		xmm2,	xmm0				/* xmm2 = 33323130 23222120 13121110 03020100 */

				punpckldq	xmm0,	xmm4				/* xmm0 = 1716151413121110	0706050403020100 */
				movq		QWORD PTR [esi+edx*2],xmm0	/* write 00 01 02 03 04 05 06 07 */

				psrldq		xmm0,	8					/* xmm0 = 1716151413121110 */
				punpckhdq	xmm2,	xmm4				/* xmm2 = 3736353433323130	2726252423222120 */

				movq		QWORD PTR [esi+edx], xmm0	/* write 10 11 12 13 14 15 16 17 */
				movdqa		xmm3,	xmm1				/* xmm3 = 73727170 63626160 53525150 43424140 */
				
				punpckldq	xmm1,	xmm5				/* xmm1 = 5756555453525150 4746454443424140 */
				movq		QWORD PTR [esi],	xmm2	/* write 20 21 22 23 24 25 26 27 */
				
				psrldq		xmm2,	8					/* xmm2 = 3736353433323130 */
				punpckhdq	xmm3,	xmm5				/* xmm3 = 7776757473727170 6766656463626160 */

				movq		QWORD PTR [esi+ecx], xmm2	/* write 30 31 32 33 34 35 36 37 */
				lea			esi,	[esi+ecx*4]			/* esi= Des - 4 + 4 *pitch */
				
				movq		QWORD PTR [esi+edx*2], xmm1	/* write 40 41 42 43 44 45 46 47 */
				movq		QWORD PTR [esi],	xmm3	/* write 60 61 62 63 64 65 66 67 */

				psrldq		xmm1,	8					/* xmm1 = 5756555453525150 */
				psrldq		xmm3,	8					/* xmm3 = 7776757473727170 */

				movq		QWORD PTR [esi+edx], xmm1	/* write 50 51 52 53 54 55 56 57 */
				movq		QWORD PTR [esi+ecx], xmm3	/* write 70 71 72 73 74 75 76 77 */


				pop			edi
				pop			esi
				pop			edx
				pop			ecx
				pop			eax
	    	}// end of __asm	

    		Var1=Variance1[0]+Variance1[1]+Variance1[2]+Variance1[3]+Variance1[4]+Variance1[5]+Variance1[6]+Variance1[7];
	    	Var2=Variance2[0]+Variance2[1]+Variance2[2]+Variance2[3]+Variance2[4]+Variance2[5]+Variance2[6]+Variance2[7];
			pbi->FragmentVariances[CurrentFrag-1] += Var1;
			pbi->FragmentVariances[CurrentFrag] += Var2;
			CurrentFrag ++;

		}//else
			
	}//while
#endif

}

/****************************************************************************
 * 
 *  ROUTINE       : PlaneAddNoise_wmt
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
void PlaneAddNoise_wmt( UINT8 *Start, UINT32 Width, UINT32 Height, INT32 Pitch, int q)
{
    unsigned int i;
    INT32 Pitch4 = Pitch * 4;
    const int noiseAmount = 2;
    const int noiseAdder = 2 * noiseAmount + 1;
#if defined(_WIN32_WCE)
	return;
#else

	__declspec(align(16)) unsigned char blackclamp[16];
	__declspec(align(16)) unsigned char whiteclamp[16];
	__declspec(align(16)) unsigned char bothclamp[16];
    char CharDist[300];
    char Rand[2048];
    double sigma;
//    return;
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
            movdqu      xmm1,[esi+eax]         // get the source

			psubusb     xmm1,blackclamp        // clamp both sides so we don't outrange adding noise
			paddusb     xmm1,bothclamp          
			psubusb     xmm1,whiteclamp

            movdqu      xmm2,[edi+eax]         // get the noise for this line
            paddb       xmm1,xmm2              // add it in 
            movdqu      [esi+eax],xmm1         // store the result

            add         eax,16                 // move to the next line

			cmp         eax, ecx
			jl			nextset


        }

    }
#endif
}
