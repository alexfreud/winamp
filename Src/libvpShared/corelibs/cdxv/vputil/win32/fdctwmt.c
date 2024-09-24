/****************************************************************************
 *
 *   Module Title :     Fdctwmt.c
 *
 *   Description  :     Forward DCT optimized specifically for Intel  P4
 *						processor
 *
 *   AUTHOR       :     YaoWu Xu
 *
 ***************************************************************************** 
 *   Revision History
 *	
 *   1.00 YWX  03/11/02  Configuration baseline
 *
 *****************************************************************************
 */


/*******************************************************************************
 * Module Constants
 *******************************************************************************
 */
	

__declspec(align(16)) static unsigned short TIRY[8];

__declspec(align(16)) static unsigned short WmtIdctConst[8 * 8] =
{
    0,    0,    0,    0,    0,    0,    0,    0, 
	64277,64277,64277,64277,64277,64277,64277,64277, 
	60547,60547,60547,60547,60547,60547,60547,60547, 
	54491,54491,54491,54491,54491,54491,54491,54491, 
	46341,46341,46341,46341,46341,46341,46341,46341, 
	36410,36410,36410,36410,36410,36410,36410,36410, 
	25080,25080,25080,25080,25080,25080,25080,25080, 
	12785,12785,12785,12785,12785,12785,12785,12785
};

 
/**************************************************************************************
 *
 *		Macro:			FDct_WMT
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
void  fdct_WMT(short *InputData, short *OutputData)
{

	__asm 
	{
		mov		eax, InputData
		mov		ebx, OutputData
		lea		edx, WmtIdctConst

#define I(i) [eax + 16 * i ]
#define O(i) [ebx + 16 * i ]
#define C(i) [edx + 16 * i ]

/******************************************************/
/* Do 8x8 Transpose                                   */
/******************************************************/

    	movdqa		xmm4, I(4)		/* xmm4=e7e6e5e4e3e2e1e0 */	
        movdqa		xmm0, I(5)		/* xmm4=f7f6f5f4f3f2f1f0 */	
        
        psllw       xmm4, 1
        psllw       xmm0, 1

        movdqa		xmm5, xmm4		/* make a copy */			
        punpcklwd	xmm4, xmm0		/* xmm4=f3e3f2e2f1e1f0e0 */	
        
        punpckhwd	xmm5, xmm0		/* xmm5=f7e7f6e6f5e5f4e4 */	
        movdqa		xmm6, I(6)		/* xmm6=g7g6g5g4g3g2g1g0 */ 
        
        movdqa		xmm0, I(7)		/* xmm0=h7h6h5h4h3h2h1h0 */ 

        psllw       xmm6, 1
        psllw       xmm0, 1

        movdqa		xmm7, xmm6		/* make a copy */			
        
        punpcklwd	xmm6, xmm0		/* xmm6=h3g3h3g2h1g1h0g0 */ 
        punpckhwd	xmm7, xmm0		/* xmm7=h7g7h6g6h5g5h4g4 */ 
        
        movdqa		xmm3, xmm4		/* make a copy */			
        punpckldq	xmm4, xmm6		/* xmm4=h1g1f1e1h0g0f0e0 */	
        
        punpckhdq	xmm3, xmm6		/* xmm3=h3g3g3e3h2g2f2e2 */	
        movdqa		I(6), xmm3		/* save h3g3g3e3h2g2f2e2 */	
        /* Free xmm6 */ 
        movdqa		xmm6, xmm5		/* make a copy */			
        punpckldq	xmm5, xmm7		/* xmm5=h5g5f5e5h4g4f4e4 */ 
        
        punpckhdq	xmm6, xmm7		/* xmm6=h7g7f7e7h6g6f6e6 */ 
        movdqa		xmm0, I(0)		/* xmm0=a7a6a5a4a3a2a1a0 */	
        /* Free xmm7 */ 
        movdqa		xmm1, I(1)		/* xmm1=b7b6b5b4b3b2b1b0 */	

        psllw       xmm0, 1
        psllw       xmm1, 1
        
        movdqa		xmm7, xmm0		/* make a copy */			
        
        punpcklwd	xmm0, xmm1		/* xmm0=b3a3b2a2b1a1b0a0 */	
        punpckhwd	xmm7, xmm1		/* xmm7=b7a7b6a6b5a5b4a4 */ 
        /* Free xmm1 */ 
        movdqa		xmm2, I(2)		/* xmm2=c7c6c5c4c3c2c1c0 */ 
        movdqa		xmm3, I(3)	    /* xmm3=d7d6d5d4d3d2d1d0 */ 
        
        psllw       xmm2, 1
        psllw       xmm3, 1

        movdqa		xmm1, xmm2		/* make a copy */			
        punpcklwd	xmm2, xmm3		/* xmm2=d3c3d2c2d1c1d0c0 */ 
        
        punpckhwd	xmm1, xmm3		/* xmm1=d7c7d6c6d5c5d4c4 */ 
        movdqa		xmm3, xmm0		/* make a copy	*/			
        
        punpckldq	xmm0, xmm2		/* xmm0=d1c1b1a1d0c0b0a0 */ 
        punpckhdq	xmm3, xmm2		/* xmm3=d3c3b3a3d2c2b2a2 */ 
        /* Free xmm2 */ 
        movdqa		xmm2, xmm7		/* make a copy */			
        punpckldq	xmm2, xmm1		/* xmm2=d5c5b5a5d4c4b4a4 */	
        
        punpckhdq	xmm7, xmm1		/* xmm7=d7c7b7a7d6c6b6a6 */ 
        movdqa		xmm1, xmm0		/* make a copy */			
        
        punpcklqdq	xmm0, xmm4		/* xmm0=h0g0f0e0d0c0b0a0 */	
        punpckhqdq	xmm1, xmm4		/* xmm1=h1g1g1e1d1c1b1a1 */ 
        
        movdqa		I(0), xmm0		/* save I(0) */				
        movdqa		I(1), xmm1		/* save I(1) */				
        
        movdqa		xmm0, I(6)		/* load h3g3g3e3h2g2f2e2 */ 
        movdqa		xmm1, xmm3		/* make a copy */			
        
        punpcklqdq	xmm1, xmm0		/* xmm1=h2g2f2e2d2c2b2a2 */ 
        punpckhqdq	xmm3, xmm0		/* xmm3=h3g3f3e3d3c3b3a3 */	
        
        movdqa		xmm4, xmm2		/* make a copy */			
        punpcklqdq	xmm4, xmm5		/* xmm4=h4g4f4e4d4c4b4a4 */	
        
        punpckhqdq	xmm2, xmm5		/* xmm2=h5g5f5e5d5c5b5a5 */	
        movdqa		I(2), xmm1		/* save I(2) */				
        
        movdqa		I(3), xmm3		/* save I(3) */				
        movdqa		I(4), xmm4		/* save I(4) */				
        
        movdqa		I(5), xmm2		/* save I(5) */				
        movdqa		xmm5, xmm7		/* make a copy */			
        
        punpcklqdq	xmm5, xmm6		/* xmm5=h6g6f6e6d6c6b6a6 */	
        punpckhqdq	xmm7, xmm6		/* xmm7=h7g7f7e7d7c7b7a7 */	
        
        movdqa		I(6), xmm5		/* save I(6) */				
        movdqa		I(7), xmm7		/* save I(7) */				

/******************************************************/
/* Done with transpose - Let's do the forward DCT     */
/******************************************************/

        movdqa		xmm0, I(0)      /* xmm0 = ip0 */
        movdqa      xmm1, I(1)      /* xmm1 = ip1 */

        movdqa      xmm2, I(3)      /* xmm2 = ip3 */
        movdqa      xmm3, I(5)      /* xmm3 = ip5 */

        movdqa      xmm4, xmm0      /* xmm4 = ip0 */
        movdqa      xmm5, xmm1      /* xmm5 = ip1 */      
        
        movdqa      xmm6, xmm2      /* xmm6 = ip3 */      
        movdqa      xmm7, xmm3      /* xmm7 = ip5 */      	

        paddsw      xmm0, I(7)      /* xmm0 = ip0 + ip7 */
        paddsw      xmm1, I(2)      /* xmm1 = ip1 + ip2 */

        paddsw      xmm2, I(4)      /* xmm2 = ip3 + ip4 */
        paddsw      xmm3, I(6)      /* xmm3 = ip5 + ip6 */

        psubsw      xmm4, I(7)      /* xmm4 = ip0 - ip7 */
        psubsw      xmm5, I(2)      /* xmm5 = ip1 - ip2 */       

        psubsw		xmm0, xmm2      /* xmm0 = is07 - is34 */			
        paddsw		xmm2, xmm2		/* xmm2 = is34 * 2    */	
        
        psubsw      xmm6, I(4)      /* xmm6 = ip3 - ip4 */               
        paddsw		xmm2, xmm0		/* xmm2 = is07 + is34 */	

        psubsw		xmm1, xmm3		/* xmm1 = is12 - is56 */	
        movdqa		TIRY, xmm0		/* save is07-is34 */	

        paddsw		xmm3, xmm3		/* xmm3 = is56 * 2 */	
        paddsw		xmm3, xmm1	    /* xmm3 = is12 + is56 */
        
        psubsw      xmm7, I(6)      /* xmm7 = ip5 -ip6 */
        psubsw		xmm5, xmm7		/* xmm5 = id12 - id56 */
	    
        paddsw		xmm7, xmm7		/* xmm7 = id56 * 2 */		
	    paddsw		xmm7, xmm5	    /* xmm7 = id12 + id56 */
/*---------------------------------------------------------*/
/* op0 and op4 
/*---------------------------------------------------------*/
        psubsw		xmm2, xmm3		/* xmm2 = is0734 - is1256 */
        paddsw		xmm3, xmm3		/* xmm3 = is1256 * 2 */		

        movdqa		xmm0, xmm2	    /* xmm0 = is0734 - is1256 */
        paddsw		xmm3, xmm2		/* xmm3 = is0734 + is1256 */

        pmulhw		xmm0, C(4)	    /* xmm0 = xC4S4 * ( is0734 - is1256 ) - ( is0734 - is1256 ) */
        paddw		xmm0, xmm2		/* xmm0 = xC4S4 * ( is0734 - is1256 ) */

        psrlw		xmm2, 15			
        paddw		xmm0, xmm2		/* Truncate xmm0, now it is op[4] */
            
        movdqa		xmm2, xmm3		/* xmm2 = is0734 + is1256 */
        movdqa		O(4), xmm0		/*	op4, now xmm0,xmm2 are free */
            
        movdqa		xmm0, xmm3		/* xmm0 = is0734 + is1256 */
        pmulhw		xmm3, C(4)		/* xmm3 = xC4S4 * ( is0734 +is1256 ) - ( is0734 +is1256 ) */            
        
        psrlw		xmm2, 15			
        paddw		xmm3, xmm0		/* xmm3 = xC4S4 * ( is0734 +is1256 ) */
        
        paddw		xmm3, xmm2		/* Truncate xmm3, now it is op[0] */     
        movdqa		O(0), xmm3		/* save op0 */
/*---------------------------------------------------------*/
/* op2 and op6 
/*---------------------------------------------------------*/
 	    movdqa		xmm3, TIRY		/* xmm3 = irot_input_y */
        pmulhw		xmm3, C(2)		/* xmm3 = xC2S6 * irot_input_y - irot_input_y */
        
        movdqa		xmm2, TIRY		/* xmm2 = irot_input_y */
        movdqa		xmm0, xmm2		/* xmm0 = irot_input_y */
        
        psrlw		xmm2, 15		
        paddw		xmm3, xmm0      /* xmm3 = xC2S6 * irot_input_y */
            
        paddw       xmm3, xmm2		/* Truncated */
        movdqa		xmm0, xmm5		/* xmm0 = id12 - id56 */
        
        
        movdqa		xmm2, xmm5      /* xmm2 = id12 - id56 */
        pmulhw		xmm0, C(6)		/* xmm0 = xC6S2 * irot_input_x */
            
        psrlw		xmm2, 15			
        paddw		xmm0, xmm2		/* Truncated */
        
        paddsw		xmm3, xmm0		/* op[2] */
        movdqa		O(2), xmm3		/* save op[2] */
        
        
        movdqa		xmm0, xmm5		/* xmm0 = id12 - id56 */
        movdqa		xmm2, xmm5		/* xmm0 = id12 - id56 */
        
        pmulhw		xmm5, C(2)		/* xmm5 = xC2S6 * irot_input_x - irot_input_x */
        psrlw		xmm2, 15		
        
        movdqa		xmm3, TIRY		/* xmm3 = irot_input_y */
        paddw		xmm5, xmm0		/* xmm5 = xC2S6 * irot_input_x */
            
        paddw		xmm5, xmm2		/* Truncated */
        movdqa		xmm2, xmm3		/* xmm2 = irot_input_y */	
        
        pmulhw		xmm3, C(6)	    /* mm3 = xC6S2 * irot_input_y */
        psrlw		xmm2, 15        
        
        paddw		xmm3, xmm2		/* Truncated */
        psubsw		xmm3, xmm5		/* xmm3 = op[6] */
        
        movdqa		O(6), xmm3		
/*-----------------------------------------------------------------------*/
/* icommon_product1, icommon_product2                                    */
/*-----------------------------------------------------------------------*/
	    movdqa		xmm0, C(4)      /* xmm0 = xC4s4 */
	    movdqa		xmm2, xmm1      /* xmm2 = is12 - is56 */	
	
        movdqa		xmm3, xmm1      /* xmm3 = is12 - is56 */	
	    pmulhw		xmm1, xmm0		/* xmm0 = xC4S4 * ( is12 - is56 ) - ( is12 - is56 ) */
	
        psrlw		xmm2, 15				
	    paddw		xmm1, xmm3	    /* xmm1 = xC4S4 * ( is12 - is56 ) */
	    
        paddw		xmm1, xmm2      /* Truncate xmm1, now it is icommon_product1 */
	    movdqa		xmm2, xmm7      /* xmm2 = id12 + id56 */
	    
        movdqa		xmm3, xmm7		/* xmm3 = id12 + id56 */
        pmulhw		xmm7, xmm0		/* xmm7 = xC4S4 * ( id12 + id56 ) - ( id12 + id56 ) */
	
        psrlw		xmm2, 15		/* For trucation */	
	    paddw		xmm7, xmm3		/* xmm7 = xC4S4 * ( id12 + id56 ) */

	    paddw		xmm7, xmm2		/* Truncate xmm7, now it is icommon_product2 */
/*---------------------------------------------------------*/
	    pxor		xmm0, xmm0		/* Clear xmm0 */
	    psubsw		xmm0, xmm6		/* xmm0 = - id34 */

	    psubsw		xmm0, xmm7	    /* xmm0 = - ( id34 + idcommon_product2 ) = irot_input_y for 17*/
	    paddsw		xmm6, xmm6	    /* xmm6 = id34 * 2 */

	    paddsw		xmm6, xmm0		/* xmm6 = id34 - icommon_product2 = irot_input_x for 35 */
	    psubsw		xmm4, xmm1		/* xmm4 = id07 - icommon_product1 = irot_input_x for 35*/

	    paddsw		xmm1, xmm1		/* xmm1 = icommon_product1 * 2 */	    
        paddsw		xmm1, xmm4		/* xmm1 = id07 + icommon_product1 = irot_input_x for 17*/

/*---------------------------------------------------------*/
/* op1 and op7              
/*---------------------------------------------------------*/

	    movdqa		xmm7, C(1)     /* xC1S7 */
        movdqa		xmm2, xmm1      /* xmm2 = irot_input_x */
        
        movdqa		xmm3, xmm1;     /* xmm3 = irot_input_x */
        pmulhw		xmm1, xmm7		/* xmm1 = xC1S7 * irot_input_x - irot_input_x */
            
        movdqa		xmm7, C(7)		/* xC7S1 */
        psrlw		xmm2, 15		/* for trucation */		
            
        paddw		xmm1, xmm3		/* xmm1 = xC1S7 * irot_input_x */
        paddw		xmm1, xmm2		/* Trucated */
            
        pmulhw		xmm3, xmm7		/* xmm3 = xC7S1 * irot_input_x */
        paddw		xmm3, xmm2		/* Truncated */
            
        movdqa		xmm5, xmm0		/* xmm5 = irot_input_y */	
        movdqa	    xmm2, xmm0      /* xmm2 = irot_input_y */	
            
        movdqa		xmm7, C(1)      /* xC1S7 */			
        pmulhw		xmm0, xmm7	    /* xmm0 = xC1S7 * irot_input_y - irot_input_y */
        
        movdqa		xmm7, C(7)		/* xC7S1 */	
        psrlw		xmm2, 15		/* for trucation */	
        
        paddw		xmm0, xmm5		/* xmm0 = xC1S7 * irot_input_y */
        paddw		xmm0, xmm2		/* Truncated */
        
        pmulhw		xmm5, xmm7		/* xmm5 = xC7S1 * irot_input_y */
        paddw		xmm5, xmm2		/* Truncated */
        
        psubsw		xmm1, xmm5		/* xmm1 = xC1S7 * irot_input_x - xC7S1 * irot_input_y = op[1] */
        paddsw		xmm3, xmm0		/* xmm3 = xC7S1 * irot_input_x - xC1S7 * irot_input_y = op[7] */
        
        movdqa		O(1), xmm1
        movdqa		O(7), xmm3
/*---------------------------------------------------------*/
/* op3 and op5 
/*---------------------------------------------------------*/
	    movdqa		xmm0, C(3)      /* xC3S5 */
	    movdqa		xmm1, C(5)      /* xC5S3 */

	    movdqa		xmm5,xmm6       /* irot_input_x */
	    movdqa		xmm7,xmm6       /* irot_input_x */

	    movdqa		xmm2,xmm4       /* irot_input_y */
	    movdqa		xmm3,xmm4       /* irot_input_y */

	    pmulhw		xmm4,xmm0       /* xmm4 = xC3S5 * irot_input_x - irot_input_x */
	    pmulhw		xmm6,xmm1		/* xmm6 = xC5S3 * irot_input_y - irot_input_y */

	    psrlw		xmm2,15         /* for trucation */
	    psrlw		xmm5,15         /* for trucation */

	    paddw		xmm4,xmm3		/* xmm4 = xC3S5 * irot_input_x */
	    paddw		xmm6,xmm7		/* xmm6 = xC5S3 * irot_input_y */

	    paddw		xmm4,xmm2		/* Truncated */
	    paddw		xmm6,xmm5		/* Truncated */

	    psubsw		xmm4,xmm6		/* op [3] */
	    movdqa		O(3),xmm4		/* Save Op[3] */

	    movdqa		xmm4,xmm3		/* irot_input_y */
	    movdqa		xmm6,xmm7		/* irot_input_x */

	    pmulhw		xmm3,xmm1		/* mm3 = xC5S3 * irot_input_x - irot_input_x */
	    pmulhw		xmm7,xmm0		/* mm7 = xC3S5 * irot_input_y - irot_input_y */

	    paddw		xmm4,xmm2       /* Trucated */
	    paddw		xmm6,xmm5       /* Trucated */

	    paddw		xmm3,xmm4		/* xmm3 = xC5S3 * irot_input_x */
	    paddw		xmm7,xmm6		/*  mm7 = xC3S5 * irot_input_y */

	    paddw		xmm3,xmm7		/* Op[5] */
	    movdqa		O(5),xmm3		/* Save Op[5] */
/*---------------------------------------------------------*/
/* End of 8 1-D FDCT                                       */        
/*---------------------------------------------------------*/
#undef I
#undef O
#define I(i) [ebx + 16 * i ]
#define O(i) [ebx + 16 * i ]

/******************************************************/
/* Do 8x8 Transpose                                   */
/******************************************************/

    	movdqa		xmm4, I(4)		/* xmm4=e7e6e5e4e3e2e1e0 */	
        movdqa		xmm0, I(5)		/* xmm4=f7f6f5f4f3f2f1f0 */	
        
        movdqa		xmm5, xmm4		/* make a copy */			
        punpcklwd	xmm4, xmm0		/* xmm4=f3e3f2e2f1e1f0e0 */	
        
        punpckhwd	xmm5, xmm0		/* xmm5=f7e7f6e6f5e5f4e4 */	
        movdqa		xmm6, I(6)		/* xmm6=g7g6g5g4g3g2g1g0 */ 
        
        movdqa		xmm0, I(7)		/* xmm0=h7h6h5h4h3h2h1h0 */ 
        movdqa		xmm7, xmm6		/* make a copy */			
        
        punpcklwd	xmm6, xmm0		/* xmm6=h3g3h3g2h1g1h0g0 */ 
        punpckhwd	xmm7, xmm0		/* xmm7=h7g7h6g6h5g5h4g4 */ 
        
        movdqa		xmm3, xmm4		/* make a copy */			
        punpckldq	xmm4, xmm6		/* xmm4=h1g1f1e1h0g0f0e0 */	
        
        punpckhdq	xmm3, xmm6		/* xmm3=h3g3g3e3h2g2f2e2 */	
        movdqa		I(6), xmm3		/* save h3g3g3e3h2g2f2e2 */	
        /* Free xmm6 */ 
        movdqa		xmm6, xmm5		/* make a copy */			
        punpckldq	xmm5, xmm7		/* xmm5=h5g5f5e5h4g4f4e4 */ 
        
        punpckhdq	xmm6, xmm7		/* xmm6=h7g7f7e7h6g6f6e6 */ 
        movdqa		xmm0, I(0)		/* xmm0=a7a6a5a4a3a2a1a0 */	
        /* Free xmm7 */ 
        movdqa		xmm1, I(1)		/* xmm1=b7b6b5b4b3b2b1b0 */	
        movdqa		xmm7, xmm0		/* make a copy */			
        
        punpcklwd	xmm0, xmm1		/* xmm0=b3a3b2a2b1a1b0a0 */	
        punpckhwd	xmm7, xmm1		/* xmm7=b7a7b6a6b5a5b4a4 */ 
        /* Free xmm1 */ 
        movdqa		xmm2, I(2)		/* xmm2=c7c6c5c4c3c2c1c0 */ 
        movdqa		xmm3, I(3)	    /* xmm3=d7d6d5d4d3d2d1d0 */ 
        
        movdqa		xmm1, xmm2		/* make a copy */			
        punpcklwd	xmm2, xmm3		/* xmm2=d3c3d2c2d1c1d0c0 */ 
        
        punpckhwd	xmm1, xmm3		/* xmm1=d7c7d6c6d5c5d4c4 */ 
        movdqa		xmm3, xmm0		/* make a copy	*/			
        
        punpckldq	xmm0, xmm2		/* xmm0=d1c1b1a1d0c0b0a0 */ 
        punpckhdq	xmm3, xmm2		/* xmm3=d3c3b3a3d2c2b2a2 */ 
        /* Free xmm2 */ 
        movdqa		xmm2, xmm7		/* make a copy */			
        punpckldq	xmm2, xmm1		/* xmm2=d5c5b5a5d4c4b4a4 */	
        
        punpckhdq	xmm7, xmm1		/* xmm7=d7c7b7a7d6c6b6a6 */ 
        movdqa		xmm1, xmm0		/* make a copy */			
        
        punpcklqdq	xmm0, xmm4		/* xmm0=h0g0f0e0d0c0b0a0 */	
        punpckhqdq	xmm1, xmm4		/* xmm1=h1g1g1e1d1c1b1a1 */ 
        
        movdqa		I(0), xmm0		/* save I(0) */				
        movdqa		I(1), xmm1		/* save I(1) */				
        
        movdqa		xmm0, I(6)		/* load h3g3g3e3h2g2f2e2 */ 
        movdqa		xmm1, xmm3		/* make a copy */			
        
        punpcklqdq	xmm1, xmm0		/* xmm1=h2g2f2e2d2c2b2a2 */ 
        punpckhqdq	xmm3, xmm0		/* xmm3=h3g3f3e3d3c3b3a3 */	
        
        movdqa		xmm4, xmm2		/* make a copy */			
        punpcklqdq	xmm4, xmm5		/* xmm4=h4g4f4e4d4c4b4a4 */	
        
        punpckhqdq	xmm2, xmm5		/* xmm2=h5g5f5e5d5c5b5a5 */	
        movdqa		I(2), xmm1		/* save I(2) */				
        
        movdqa		I(3), xmm3		/* save I(3) */				
        movdqa		I(4), xmm4		/* save I(4) */				
        
        movdqa		I(5), xmm2		/* save I(5) */				
        movdqa		xmm5, xmm7		/* make a copy */			
        
        punpcklqdq	xmm5, xmm6		/* xmm5=h6g6f6e6d6c6b6a6 */	
        punpckhqdq	xmm7, xmm6		/* xmm7=h7g7f7e7d7c7b7a7 */	
        
        movdqa		I(6), xmm5		/* save I(6) */				
        movdqa		I(7), xmm7		/* save I(7) */				

/******************************************************/
/* Done with transpose - Let's do the forward DCT     */
/******************************************************/

        movdqa		xmm0, I(0)      /* xmm0 = ip0 */
        movdqa      xmm1, I(1)      /* xmm1 = ip1 */

        movdqa      xmm2, I(3)      /* xmm2 = ip3 */
        movdqa      xmm3, I(5)      /* xmm3 = ip5 */

        movdqa      xmm4, xmm0      /* xmm4 = ip0 */
        movdqa      xmm5, xmm1      /* xmm5 = ip1 */      
        
        movdqa      xmm6, xmm2      /* xmm6 = ip3 */      
        movdqa      xmm7, xmm3      /* xmm7 = ip5 */      	

        paddsw      xmm0, I(7)      /* xmm0 = ip0 + ip7 */
        paddsw      xmm1, I(2)      /* xmm1 = ip1 + ip2 */

        paddsw      xmm2, I(4)      /* xmm2 = ip3 + ip4 */
        paddsw      xmm3, I(6)      /* xmm3 = ip5 + ip6 */

        psubsw      xmm4, I(7)      /* xmm4 = ip0 - ip7 */
        psubsw      xmm5, I(2)      /* xmm5 = ip1 - ip2 */       

        psubsw		xmm0, xmm2      /* xmm0 = is07 - is34 */			
        paddsw		xmm2, xmm2		/* xmm2 = is34 * 2    */	
        
        psubsw      xmm6, I(4)      /* xmm6 = ip3 - ip4 */               
        paddsw		xmm2, xmm0		/* xmm2 = is07 + is34 */	

        psubsw		xmm1, xmm3		/* xmm1 = is12 - is56 */	
        movdqa		TIRY, xmm0		/* save is07-is34 */	

        paddsw		xmm3, xmm3		/* xmm3 = is56 * 2 */	
        paddsw		xmm3, xmm1	    /* xmm3 = is12 + is56 */
        
        psubsw      xmm7, I(6)      /* xmm7 = ip5 -ip6 */
        psubsw		xmm5, xmm7		/* xmm5 = id12 - id56 */
	    
        paddsw		xmm7, xmm7		/* xmm7 = id56 * 2 */		
	    paddsw		xmm7, xmm5	    /* xmm7 = id12 + id56 */
/*---------------------------------------------------------*/
/* op0 and op4 
/*---------------------------------------------------------*/
#if 0        
        movdqa      xmm0, xmm2      /* xmm0 =xmm2= is0734  */
        pmulhw      xmm2, C(4)      /* xC4S4 * is0734 - is0734 */
    
        paddw       xmm2, xmm0      /* XC4S4 * is0734  */
        movdqa      xmm0, xmm3      /* xmm0 =xmm3= is1256 */

        pmulhw      xmm3, C(4)      /* xC4S4 * is1256 - is1256 */
        paddw       xmm3, xmm0      /* xC4S4 * is1256 */


        movdqa      xmm0, xmm2      
        paddsw      xmm2, xmm3      /* xC4S4 * ( is0734 +is1256 ) */

        psubsw      xmm0, xmm3      /* xC4S4 * ( is0734 -is1256 ) */
        movdqa      xmm3, xmm2      
        
        psrlw       xmm2, 15        
        paddsw      xmm3, xmm2      

        movdqa      xmm2, xmm0
        movdqa      O(0), xmm3
        
        psrlw       xmm0, 15
        paddsw      xmm2, xmm0

        movdqa      O(4), xmm2


#else


        psubsw		xmm2, xmm3		/* xmm2 = is0734 - is1256 */
        paddsw		xmm3, xmm3		/* xmm3 = is1256 * 2 */		

        movdqa		xmm0, xmm2	    /* xmm0 = is0734 - is1256 */
        paddsw		xmm3, xmm2		/* xmm3 = is0734 + is1256 */

        pmulhw		xmm0, C(4)	    /* xmm0 = xC4S4 * ( is0734 - is1256 ) - ( is0734 - is1256 ) */
        paddw		xmm0, xmm2		/* xmm0 = xC4S4 * ( is0734 - is1256 ) */

        psrlw		xmm2, 15			
        paddw		xmm0, xmm2		/* Truncate xmm0, now it is op[4] */
        
        movdqa      xmm2, xmm0      
        psrlw       xmm0, 15
        
        paddw       xmm0, xmm2
        psraw       xmm0, 1        
        
        movdqa		O(4), xmm0		/*	op4, now xmm0,xmm2 are free */        
        movdqa		xmm2, xmm3		/* xmm2 = is0734 + is1256 */
        
            
        movdqa		xmm0, xmm3		/* xmm0 = is0734 + is1256 */
        pmulhw		xmm3, C(4)		/* xmm3 = xC4S4 * ( is0734 +is1256 ) - ( is0734 +is1256 ) */            
        
        psrlw		xmm2, 15			
        paddw		xmm3, xmm0		/* xmm3 = xC4S4 * ( is0734 +is1256 ) */
        
        paddw		xmm3, xmm2		/* Truncate xmm3, now it is op[0] */     
        movdqa      xmm2, xmm3

        psrlw       xmm3, 15
        paddw       xmm3, xmm2
        
        psraw       xmm3, 1
        movdqa		O(0), xmm3		/* save op0 */
#endif
/*---------------------------------------------------------*/
/* op2 and op6 
/*---------------------------------------------------------*/
 	    movdqa		xmm3, TIRY		/* xmm3 = irot_input_y */
        pmulhw		xmm3, C(2)		/* xmm3 = xC2S6 * irot_input_y - irot_input_y */
        
        movdqa		xmm2, TIRY		/* xmm2 = irot_input_y */
        movdqa		xmm0, xmm2		/* xmm0 = irot_input_y */
        
        psrlw		xmm2, 15		
        paddw		xmm3, xmm0      /* xmm3 = xC2S6 * irot_input_y */
            
        paddw       xmm3, xmm2		/* Truncated */
        movdqa		xmm0, xmm5		/* xmm0 = id12 - id56 */
        
        
        movdqa		xmm2, xmm5      /* xmm2 = id12 - id56 */
        pmulhw		xmm0, C(6)		/* xmm0 = xC6S2 * irot_input_x */
            
        psrlw		xmm2, 15			
        paddw		xmm0, xmm2		/* Truncated */
        
        paddsw		xmm3, xmm0		/* op[2] */
        movdqa      xmm0, xmm3

        psrlw       xmm3, 15
        paddw       xmm3, xmm0

        psraw       xmm3, 1
        movdqa		O(2), xmm3		/* save op[2] */
        
        
        movdqa		xmm0, xmm5		/* xmm0 = id12 - id56 */
        movdqa		xmm2, xmm5		/* xmm0 = id12 - id56 */
        
        pmulhw		xmm5, C(2)		/* xmm5 = xC2S6 * irot_input_x - irot_input_x */
        psrlw		xmm2, 15		
        
        movdqa		xmm3, TIRY		/* xmm3 = irot_input_y */
        paddw		xmm5, xmm0		/* xmm5 = xC2S6 * irot_input_x */
            
        paddw		xmm5, xmm2		/* Truncated */
        movdqa		xmm2, xmm3		/* xmm2 = irot_input_y */	
        
        pmulhw		xmm3, C(6)	    /* mm3 = xC6S2 * irot_input_y */
        psrlw		xmm2, 15        
        
        paddw		xmm3, xmm2		/* Truncated */
        psubsw		xmm3, xmm5		/* xmm3 = op[6] */
        
        movdqa      xmm5, xmm3
        psrlw       xmm3,  15
        
        paddw       xmm3, xmm5
        psraw       xmm3, 1
        
        movdqa		O(6), xmm3		
/*-----------------------------------------------------------------------*/
/* icommon_product1, icommon_product2                                    */
/*-----------------------------------------------------------------------*/
	    movdqa		xmm0, C(4)      /* xmm0 = xC4s4 */
	    movdqa		xmm2, xmm1      /* xmm2 = is12 - is56 */	
	
        movdqa		xmm3, xmm1      /* xmm3 = is12 - is56 */	
	    pmulhw		xmm1, xmm0		/* xmm0 = xC4S4 * ( is12 - is56 ) - ( is12 - is56 ) */
	
        psrlw		xmm2, 15				
	    paddw		xmm1, xmm3	    /* xmm1 = xC4S4 * ( is12 - is56 ) */
	    
        paddw		xmm1, xmm2      /* Truncate xmm1, now it is icommon_product1 */
	    movdqa		xmm2, xmm7      /* xmm2 = id12 + id56 */
	    
        movdqa		xmm3, xmm7		/* xmm3 = id12 + id56 */
        pmulhw		xmm7, xmm0		/* xmm7 = xC4S4 * ( id12 + id56 ) - ( id12 + id56 ) */
	
        psrlw		xmm2, 15		/* For trucation */	
	    paddw		xmm7, xmm3		/* xmm7 = xC4S4 * ( id12 + id56 ) */

	    paddw		xmm7, xmm2		/* Truncate xmm7, now it is icommon_product2 */
/*---------------------------------------------------------*/
	    pxor		xmm0, xmm0		/* Clear xmm0 */
	    psubsw		xmm0, xmm6		/* xmm0 = - id34 */

	    psubsw		xmm0, xmm7	    /* xmm0 = - ( id34 + idcommon_product2 ) = irot_input_y for 17*/
	    paddsw		xmm6, xmm6	    /* xmm6 = id34 * 2 */

	    paddsw		xmm6, xmm0		/* xmm6 = id34 - icommon_product2 = irot_input_x for 35 */
	    psubsw		xmm4, xmm1		/* xmm4 = id07 - icommon_product1 = irot_input_x for 35*/

	    paddsw		xmm1, xmm1		/* xmm1 = icommon_product1 * 2 */	    
        paddsw		xmm1, xmm4		/* xmm1 = id07 + icommon_product1 = irot_input_x for 17*/

/*---------------------------------------------------------*/
/* op1 and op7              
/*---------------------------------------------------------*/

	    movdqa		xmm7, C(1)     /* xC1S7 */
        movdqa		xmm2, xmm1      /* xmm2 = irot_input_x */
        
        movdqa		xmm3, xmm1;     /* xmm3 = irot_input_x */
        pmulhw		xmm1, xmm7		/* xmm1 = xC1S7 * irot_input_x - irot_input_x */
            
        movdqa		xmm7, C(7)		/* xC7S1 */
        psrlw		xmm2, 15		/* for trucation */		
            
        paddw		xmm1, xmm3		/* xmm1 = xC1S7 * irot_input_x */
        paddw		xmm1, xmm2		/* Trucated */
            
        pmulhw		xmm3, xmm7		/* xmm3 = xC7S1 * irot_input_x */
        paddw		xmm3, xmm2		/* Truncated */
            
        movdqa		xmm5, xmm0		/* xmm5 = irot_input_y */	
        movdqa	    xmm2, xmm0      /* xmm2 = irot_input_y */	
            
        movdqa		xmm7, C(1)      /* xC1S7 */			
        pmulhw		xmm0, xmm7	    /* xmm0 = xC1S7 * irot_input_y - irot_input_y */
        
        movdqa		xmm7, C(7)		/* xC7S1 */	
        psrlw		xmm2, 15		/* for trucation */	
        
        paddw		xmm0, xmm5		/* xmm0 = xC1S7 * irot_input_y */
        paddw		xmm0, xmm2		/* Truncated */
        
        pmulhw		xmm5, xmm7		/* xmm5 = xC7S1 * irot_input_y */
        paddw		xmm5, xmm2		/* Truncated */
        
        psubsw		xmm1, xmm5		/* xmm1 = xC1S7 * irot_input_x - xC7S1 * irot_input_y = op[1] */
        paddsw		xmm3, xmm0		/* xmm3 = xC7S1 * irot_input_x - xC1S7 * irot_input_y = op[7] */

        movdqa      xmm5, xmm1
        movdqa      xmm0, xmm3

        psrlw       xmm1, 15
        psrlw       xmm3, 15

        paddw       xmm1, xmm5
        paddw       xmm3, xmm0

        psraw       xmm1, 1
        psraw       xmm3, 1

        
        movdqa		O(1), xmm1
        movdqa		O(7), xmm3
/*---------------------------------------------------------*/
/* op3 and op5 
/*---------------------------------------------------------*/
	    movdqa		xmm0, C(3)      /* xC3S5 */
	    movdqa		xmm1, C(5)      /* xC5S3 */

	    movdqa		xmm5,xmm6       /* irot_input_x */
	    movdqa		xmm7,xmm6       /* irot_input_x */

	    movdqa		xmm2,xmm4       /* irot_input_y */
	    movdqa		xmm3,xmm4       /* irot_input_y */

	    pmulhw		xmm4,xmm0       /* xmm4 = xC3S5 * irot_input_x - irot_input_x */
	    pmulhw		xmm6,xmm1		/* xmm6 = xC5S3 * irot_input_y - irot_input_y */

	    psrlw		xmm2,15         /* for trucation */
	    psrlw		xmm5,15         /* for trucation */

	    paddw		xmm4,xmm3		/* xmm4 = xC3S5 * irot_input_x */
	    paddw		xmm6,xmm7		/* xmm6 = xC5S3 * irot_input_y */

	    paddw		xmm4,xmm2		/* Truncated */
	    paddw		xmm6,xmm5		/* Truncated */

	    psubsw		xmm4,xmm6		/* op [3] */
        movdqa      xmm6,xmm4

        psrlw       xmm4,15        
        paddw       xmm4,xmm6

        psraw       xmm4,1
	    movdqa		O(3),xmm4		/* Save Op[3] */

	    movdqa		xmm4,xmm3		/* irot_input_y */
	    movdqa		xmm6,xmm7		/* irot_input_x */

	    pmulhw		xmm3,xmm1		/* mm3 = xC5S3 * irot_input_x - irot_input_x */
	    pmulhw		xmm7,xmm0		/* mm7 = xC3S5 * irot_input_y - irot_input_y */

	    paddw		xmm4,xmm2       /* Trucated */
	    paddw		xmm6,xmm5       /* Trucated */

	    paddw		xmm3,xmm4		/* xmm3 = xC5S3 * irot_input_x */
	    paddw		xmm7,xmm6		/*  mm7 = xC3S5 * irot_input_y */

	    paddw		xmm3,xmm7		/* Op[5] */        
        movdqa      xmm7,xmm3

        psrlw       xmm3,15        
        paddw       xmm3,xmm7

        psraw       xmm3,1
	    movdqa		O(5),xmm3		/* Save Op[5] */
/*---------------------------------------------------------*/
/* End of 8 1-D FDCT                                       */        
/*---------------------------------------------------------*/

    }/* end of _asm code section */
}



