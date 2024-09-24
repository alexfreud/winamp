/****************************************************************************
 *
 *   Module Title :     fdctmmx.c
 *
 *   Description  :     Forward DCT optimized specifically for mmx or compatible
 *						processor
 *
 *   AUTHOR       :     Yaowu Xu
 *
 ***************************************************************************** 
 *   Revision History
 *	
 *   1.00 YWX  07/11/11 Configuration baseline
 *
 *****************************************************************************
 */


/*******************************************************************************
 * Module Constants
 *******************************************************************************
 */
	

__declspec(align(16)) static unsigned short TIRY[8];

__declspec(align(16)) static unsigned short MmxIdctConst[8 * 4] =
{
    0,    0,    0,    0,    
	64277,64277,64277,64277, 
	60547,60547,60547,60547, 
	54491,54491,54491,54491, 
	46341,46341,46341,46341, 
	36410,36410,36410,36410, 
	25080,25080,25080,25080, 
	12785,12785,12785,12785
};

 
/**************************************************************************************
 *
 *		Macro:			fdct_MMX
 *		
 *		Description:	The Macro does 1-D IDct on 8 columns. 
 *
 *		Input:			None
 *
 *		Output:			None
 *		
 *		Return:			None			
 *
 *		Special Note:	The inputdata is limited to 9 bits [-256, 255]
 *
 *		Error:			None
 *
 ***************************************************************************************
 */

void  fdct_MMX(short *InputData, short *OutputData)
{

	__asm 
	{
		mov		eax, InputData
		mov		ebx, OutputData
        lea     ecx, [eax+8]
        lea     edi, [ebx+8]

		lea		edx, MmxIdctConst

#define IL(i)   [eax + 16 * i]
#define IH(i)   [ecx + 16 * i]
#define OL(i)   [ebx + 16 * i]
#define OH(i)   [edi + 16 * i]
#define C(i)    [edx + 8  * i]

/******************************************************/
/* Do 4x8 Transpose  is done through 2 4x4 Transpose  */
/******************************************************/

    	movq		mm4, IH(0)		/* mm4=e3e2e1e0 */	
        movq		mm0, IH(1)		/* mm4=f3f2f1f0 */	
        
        psllw       mm4, 1          /* up precision */
        psllw       mm0, 1          /* up precision */

        movq		mm5, mm4		/* make a copy  */			
        punpcklwd	mm4, mm0		/* mm4=f1e1f0e0 */	
        
        punpckhwd	mm5, mm0		/* mm5=f3e3f2e2 */        
        movq		mm6, IH(2)		/* mm6=g3g2g1g0 */         

        movq		mm0, IH(3)		/* mm0=h3h2h1h0 */ 
        psllw       mm6, 1          /* up precision */

        psllw       mm0, 1          /* up precision */
        movq		mm7, mm6		/* mm7=g3g2g1g0 */         
        
        punpcklwd	mm6, mm0		/* mm6=h1g1h0g0 */ 
        punpckhwd	mm7, mm0		/* mm7=h3g3h2g2 */ 
        
        movq		mm3, mm4		/* mm4=f1e1f0e0 */	
        punpckldq	mm4, mm6		/* mm4=h0g0f0e0 */	
        
        punpckhdq	mm3, mm6		/* mm3=h1g1f1e1 */	
        movq		mm6, mm5		/* mm5=f3e3f2e2 */

        punpckldq	mm5, mm7		/* mm5=h2g2f2e2 */ 
        movq        IH(0), mm4      /* saveh0g0f0e0 */       
        
        punpckhdq	mm6, mm7		/* mm6=h3g3f3e3 */        
        movq        IH(2), mm5      /* saveh2g2f2e2 */

        movq        IH(3), mm6      /* saveh3g3f3e3 */

/*----------------------------------------------------*/        
/*    mm3 in use for IH(1)                            */
/*----------------------------------------------------*/

        movq		mm4, IL(0)		/* mm4=a3a2a1a0 */	
        movq		mm0, IL(1)		/* mm0=b3b2b1b0 */	

        psllw       mm4, 1          /* up precision */
        psllw       mm0, 1          /* up precision */
        
        movq        mm5, mm4        /* mm5=a3a2a1a0 */
        punpcklwd   mm4, mm0        /* mm4=b1a1b0a0 */
        
        punpckhwd	mm5, mm0		/* mm5=b3a3b2a2 */	                
        movq		mm6, IL(2)		/* mm6=c3c2c1c0 */ 
        
        
        movq		mm0, IL(3)	    /* mm0=d3d2d1d0 */         
        psllw       mm6, 1          /* up precision */

        psllw       mm0, 1          /* up precision */
        movq        mm7, mm6        /* mm7=c3c2c1c0 */

        punpcklwd	mm6, mm0		/* mm6=d1c1d0c0 */ 
        punpckhwd	mm7, mm0		/* mm7=c3c3d2c2 */ 
        
        movq		mm1, mm4		/* mm4=b1a1b0a0 */	
        punpckldq	mm4, mm6		/* mm4=d0c0b0a0 */	
        
        punpckhdq	mm1, mm6		/* mm1=d1c1b1a1 */	
        movq		mm2, mm5		/* mm5=b3a3b2a2 */

        punpckldq	mm5, mm7		/* mm5=d2c2b2a2 */ 
        punpckhdq	mm2, mm7		/* mm6=d3c3b3a3 */
        
        movq        IL(2), mm5       /* saved2c2b2a2 */

/*----------------------------------------------------*/        
/*    mm1 in use for IL(1)                            */
/*    mm2 in use for IL(3)                            */
/*    mm3 in use for IH(1)                            */
/*    mm4 in use for IH(0)                            */
/*----------------------------------------------------*/

/******************************************************/
/* Let's do the 4x8 forward DCT                       */
/******************************************************/
        movq        mm0, mm4        /* mm4 = ip0 */
        movq        mm5, mm1        /* mm5 = ip1 */      
        
        movq        mm6, mm2        /* mm6 = ip3 */      
        movq        mm7, mm3        /* mm7 = ip5 */      	

        paddsw      mm0, IH(3)      /* mm0 = ip0 + ip7 */
        paddsw      mm1, IL(2)      /* mm1 = ip1 + ip2 */

        paddsw      mm2, IH(0)      /* mm2 = ip3 + ip4 */
        paddsw      mm3, IH(2)      /* mm3 = ip5 + ip6 */

        psubsw      mm4, IH(3)      /* mm4 = ip0 - ip7 */
        psubsw      mm5, IL(2)      /* mm5 = ip1 - ip2 */       

        psubsw		mm0, mm2        /* mm0 = is07 - is34 */			
        paddsw		mm2, mm2		/* mm2 = is34 * 2    */	
        
        psubsw      mm6, IH(0)      /* mm6 = ip3 - ip4 */               
        paddsw		mm2, mm0		/* mm2 = is07 + is34 */	

        psubsw		mm1, mm3		/* mm1 = is12 - is56 */	
        movq		TIRY, mm0		/* save is07-is34 */	

        paddsw		mm3, mm3		/* mm3 = is56 * 2 */	
        paddsw		mm3, mm1	    /* mm3 = is12 + is56 */
        
        psubsw      mm7, IH(2)      /* mm7 = ip5 -ip6 */
        psubsw		mm5, mm7		/* mm5 = id12 - id56 */
	    
        paddsw		mm7, mm7		/* mm7 = id56 * 2 */		
	    paddsw		mm7, mm5	    /* mm7 = id12 + id56 */
/*---------------------------------------------------------*/
/* op0 and op4 
/*---------------------------------------------------------*/
        psubsw		mm2, mm3		/* mm2 = is0734 - is1256 */
        paddsw		mm3, mm3		/* mm3 = is1256 * 2 */		

        movq		mm0, mm2	    /* mm0 = is0734 - is1256 */
        paddsw		mm3, mm2		/* mm3 = is0734 + is1256 */

        pmulhw		mm0, C(4)	    /* mm0 = xC4S4 * ( is0734 - is1256 ) - ( is0734 - is1256 ) */
        paddw		mm0, mm2		/* mm0 = xC4S4 * ( is0734 - is1256 ) */

        psrlw		mm2, 15			
        paddw		mm0, mm2		/* Truncate mm0, now it is op[4] */
            
        movq		mm2, mm3		/* mm2 = is0734 + is1256 */
        movq		OH(0), mm0		/*	op4, now mm0,mm2 are free */
            
        movq		mm0, mm3		/* mm0 = is0734 + is1256 */
        pmulhw		mm3, C(4)		/* mm3 = xC4S4 * ( is0734 +is1256 ) - ( is0734 +is1256 ) */            
        
        psrlw		mm2, 15			
        paddw		mm3, mm0		/* mm3 = xC4S4 * ( is0734 +is1256 ) */
        
        paddw		mm3, mm2		/* Truncate mm3, now it is op[0] */     
        movq		OL(0), mm3		/* save op0 */
/*---------------------------------------------------------*/
/* op2 and op6 
/*---------------------------------------------------------*/
 	    movq		mm3, TIRY		/* mm3 = irot_input_y */
        pmulhw		mm3, C(2)		/* mm3 = xC2S6 * irot_input_y - irot_input_y */
        
        movq		mm2, TIRY		/* mm2 = irot_input_y */
        movq		mm0, mm2		/* mm0 = irot_input_y */
        
        psrlw		mm2, 15		
        paddw		mm3, mm0        /* mm3 = xC2S6 * irot_input_y */
            
        paddw       mm3, mm2		/* Truncated */
        movq		mm0, mm5		/* mm0 = id12 - id56 */
        
        
        movq		mm2, mm5        /* mm2 = id12 - id56 */
        pmulhw		mm0, C(6)		/* mm0 = xC6S2 * irot_input_x */
            
        psrlw		mm2, 15			
        paddw		mm0, mm2		/* Truncated */
        
        paddsw		mm3, mm0		/* op[2] */
        movq		OL(2), mm3		/* save op[2] */
        
        
        movq		mm0, mm5		/* mm0 = id12 - id56 */
        movq		mm2, mm5		/* mm0 = id12 - id56 */
        
        pmulhw		mm5, C(2)		/* mm5 = xC2S6 * irot_input_x - irot_input_x */
        psrlw		mm2, 15		
        
        movq		mm3, TIRY		/* mm3 = irot_input_y */
        paddw		mm5, mm0		/* mm5 = xC2S6 * irot_input_x */
            
        paddw		mm5, mm2		/* Truncated */
        movq		mm2, mm3		/* mm2 = irot_input_y */	
        
        pmulhw		mm3, C(6)	    /* mm3 = xC6S2 * irot_input_y */
        psrlw		mm2, 15        
        
        paddw		mm3, mm2		/* Truncated */
        psubsw		mm3, mm5		/* mm3 = op[6] */
        
        movq		OH(2), mm3		
/*-----------------------------------------------------------------------*/
/* icommon_product1, icommon_product2                                    */
/*-----------------------------------------------------------------------*/
	    movq		mm0, C(4)       /* mm0 = xC4s4 */
	    movq		mm2, mm1        /* mm2 = is12 - is56 */	
	
        movq		mm3, mm1        /* mm3 = is12 - is56 */	
	    pmulhw		mm1, mm0		/* mm0 = xC4S4 * ( is12 - is56 ) - ( is12 - is56 ) */
	
        psrlw		mm2, 15				
	    paddw		mm1, mm3	    /* mm1 = xC4S4 * ( is12 - is56 ) */
	    
        paddw		mm1, mm2        /* Truncate mm1, now it is icommon_product1 */
	    movq		mm2, mm7        /* mm2 = id12 + id56 */
	    
        movq		mm3, mm7		/* mm3 = id12 + id56 */
        pmulhw		mm7, mm0		/* mm7 = xC4S4 * ( id12 + id56 ) - ( id12 + id56 ) */
	
        psrlw		mm2, 15		    /* For trucation */	
	    paddw		mm7, mm3		/* mm7 = xC4S4 * ( id12 + id56 ) */

	    paddw		mm7, mm2		/* Truncate mm7, now it is icommon_product2 */
/*---------------------------------------------------------*/
	    pxor		mm0, mm0		/* Clear mm0 */
	    psubsw		mm0, mm6		/* mm0 = - id34 */

	    psubsw		mm0, mm7	    /* mm0 = - ( id34 + idcommon_product2 ) = irot_input_y for 17*/
	    paddsw		mm6, mm6	    /* mm6 = id34 * 2 */

	    paddsw		mm6, mm0		/* mm6 = id34 - icommon_product2 = irot_input_x for 35 */
	    psubsw		mm4, mm1		/* mm4 = id07 - icommon_product1 = irot_input_x for 35*/

	    paddsw		mm1, mm1		/* mm1 = icommon_product1 * 2 */	    
        paddsw		mm1, mm4		/* mm1 = id07 + icommon_product1 = irot_input_x for 17*/

/*---------------------------------------------------------*/
/* op1 and op7              
/*---------------------------------------------------------*/

	    movq		mm7, C(1)       /* xC1S7 */
        movq		mm2, mm1        /* mm2 = irot_input_x */
        
        movq		mm3, mm1;       /* mm3 = irot_input_x */
        pmulhw		mm1, mm7		/* mm1 = xC1S7 * irot_input_x - irot_input_x */
            
        movq		mm7, C(7)		/* xC7S1 */
        psrlw		mm2, 15		    /* for trucation */		
            
        paddw		mm1, mm3		/* mm1 = xC1S7 * irot_input_x */
        paddw		mm1, mm2		/* Trucated */
            
        pmulhw		mm3, mm7		/* mm3 = xC7S1 * irot_input_x */
        paddw		mm3, mm2		/* Truncated */
            
        movq		mm5, mm0		/* mm5 = irot_input_y */	
        movq	    mm2, mm0        /* mm2 = irot_input_y */	
            
        movq		mm7, C(1)       /* xC1S7 */			
        pmulhw		mm0, mm7	    /* mm0 = xC1S7 * irot_input_y - irot_input_y */
        
        movq		mm7, C(7)		/* xC7S1 */	
        psrlw		mm2, 15		    /* for trucation */	
        
        paddw		mm0, mm5		/* mm0 = xC1S7 * irot_input_y */
        paddw		mm0, mm2		/* Truncated */
        
        pmulhw		mm5, mm7		/* mm5 = xC7S1 * irot_input_y */
        paddw		mm5, mm2		/* Truncated */
        
        psubsw		mm1, mm5		/* mm1 = xC1S7 * irot_input_x - xC7S1 * irot_input_y = op[1] */
        paddsw		mm3, mm0		/* mm3 = xC7S1 * irot_input_x - xC1S7 * irot_input_y = op[7] */
        
        movq		OL(1), mm1
        movq		OH(3), mm3
/*---------------------------------------------------------*/
/* op3 and op5 
/*---------------------------------------------------------*/
	    movq		mm0, C(3)       /* xC3S5 */
	    movq		mm1, C(5)       /* xC5S3 */

	    movq		mm5,mm6         /* irot_input_x */
	    movq		mm7,mm6         /* irot_input_x */

	    movq		mm2,mm4         /* irot_input_y */
	    movq		mm3,mm4         /* irot_input_y */

	    pmulhw		mm4,mm0         /* mm4 = xC3S5 * irot_input_x - irot_input_x */
	    pmulhw		mm6,mm1		    /* mm6 = xC5S3 * irot_input_y - irot_input_y */

	    psrlw		mm2,15          /* for trucation */
	    psrlw		mm5,15          /* for trucation */

	    paddw		mm4,mm3		    /* mm4 = xC3S5 * irot_input_x */
	    paddw		mm6,mm7		    /* mm6 = xC5S3 * irot_input_y */

	    paddw		mm4,mm2		    /* Truncated */
	    paddw		mm6,mm5		    /* Truncated */

	    psubsw		mm4,mm6		    /* op [3] */
	    movq		OL(3),mm4		/* Save Op[3] */

	    movq		mm4,mm3		    /* irot_input_y */
	    movq		mm6,mm7		    /* irot_input_x */

	    pmulhw		mm3,mm1		    /* mm3 = xC5S3 * irot_input_x - irot_input_x */
	    pmulhw		mm7,mm0		    /* mm7 = xC3S5 * irot_input_y - irot_input_y */

	    paddw		mm4,mm2         /* Trucated */
	    paddw		mm6,mm5         /* Trucated */

	    paddw		mm3,mm4		    /* mm3 = xC5S3 * irot_input_x */
	    paddw		mm7,mm6		    /*  mm7 = xC3S5 * irot_input_y */

	    paddw		mm3,mm7		    /* Op[5] */
	    movq		OH(1),mm3		/* Save Op[5] */
/*---------------------------------------------------------*/
/* End of 4x8 1-D FDCT                                     */        
/*---------------------------------------------------------*/

/******************************************************/
/* Do 4x8 Transpose  is done through 2 4x4 Transpose  */
/******************************************************/

        lea         eax, [eax+64]
        lea         ecx, [ecx+64]
        lea         ebx, [ebx+64]
        lea         edi, [edi+64]

    	movq		mm4, IH(0)		/* mm4=e3e2e1e0 */	
        movq		mm0, IH(1)		/* mm4=f3f2f1f0 */	
        
        psllw       mm4, 1          /* up precision */
        psllw       mm0, 1          /* up precision */

        movq		mm5, mm4		/* make a copy  */			
        punpcklwd	mm4, mm0		/* mm4=f1e1f0e0 */	
        
        punpckhwd	mm5, mm0		/* mm5=f3e3f2e2 */        
        movq		mm6, IH(2)		/* mm6=g3g2g1g0 */         

        movq		mm0, IH(3)		/* mm0=h3h2h1h0 */ 
        psllw       mm6, 1          /* up precision */

        psllw       mm0, 1          /* up precision */
        movq		mm7, mm6		/* mm7=g3g2g1g0 */         
        
        punpcklwd	mm6, mm0		/* mm6=h1g1h0g0 */ 
        punpckhwd	mm7, mm0		/* mm7=h3g3h2g2 */ 
        
        movq		mm3, mm4		/* mm4=f1e1f0e0 */	
        punpckldq	mm4, mm6		/* mm4=h0g0f0e0 */	
        
        punpckhdq	mm3, mm6		/* mm3=h1g1f1e1 */	
        movq		mm6, mm5		/* mm5=f3e3f2e2 */

        punpckldq	mm5, mm7		/* mm5=h2g2f2e2 */ 
        movq        IH(0), mm4      /* saveh0g0f0e0 */       
        
        punpckhdq	mm6, mm7		/* mm6=h3g3f3e3 */        
        movq        IH(2), mm5      /* saveh2g2f2e2 */

        movq        IH(3), mm6      /* saveh3g3f3e3 */

/*----------------------------------------------------*/        
/*    mm3 in use for IH(1)                            */
/*----------------------------------------------------*/

        movq		mm4, IL(0)		/* mm4=a3a2a1a0 */	
        movq		mm0, IL(1)		/* mm0=b3b2b1b0 */	

        psllw       mm4, 1          /* up precision */
        psllw       mm0, 1          /* up precision */
        
        movq        mm5, mm4        /* mm5=a3a2a1a0 */
        punpcklwd   mm4, mm0        /* mm4=b1a1b0a0 */
        
        punpckhwd	mm5, mm0		/* mm5=b3a3b2a2 */	                
        movq		mm6, IL(2)		/* mm6=c3c2c1c0 */ 
        
        
        movq		mm0, IL(3)	    /* mm0=d3d2d1d0 */         
        psllw       mm6, 1          /* up precision */

        psllw       mm0, 1          /* up precision */
        movq        mm7, mm6        /* mm7=c3c2c1c0 */

        punpcklwd	mm6, mm0		/* mm6=d1c1d0c0 */ 
        punpckhwd	mm7, mm0		/* mm7=c3c3d2c2 */ 
        
        movq		mm1, mm4		/* mm4=b1a1b0a0 */	
        punpckldq	mm4, mm6		/* mm4=d0c0b0a0 */	
        
        punpckhdq	mm1, mm6		/* mm1=d1c1b1a1 */	
        movq		mm2, mm5		/* mm5=b3a3b2a2 */

        punpckldq	mm5, mm7		/* mm5=d2c2b2a2 */ 
        punpckhdq	mm2, mm7		/* mm6=d3c3b3a3 */
        
        movq        IL(2), mm5       /* saved2c2b2a2 */

/*----------------------------------------------------*/        
/*    mm1 in use for IL(1)                            */
/*    mm2 in use for IL(3)                            */
/*    mm3 in use for IH(1)                            */
/*    mm4 in use for IH(0)                            */
/*----------------------------------------------------*/

/******************************************************/
/* Let's do the 4x8 forward DCT                       */
/******************************************************/
        movq        mm0, mm4        /* mm4 = ip0 */
        movq        mm5, mm1        /* mm5 = ip1 */      
        
        movq        mm6, mm2        /* mm6 = ip3 */      
        movq        mm7, mm3        /* mm7 = ip5 */      	

        paddsw      mm0, IH(3)      /* mm0 = ip0 + ip7 */
        paddsw      mm1, IL(2)      /* mm1 = ip1 + ip2 */

        paddsw      mm2, IH(0)      /* mm2 = ip3 + ip4 */
        paddsw      mm3, IH(2)      /* mm3 = ip5 + ip6 */

        psubsw      mm4, IH(3)      /* mm4 = ip0 - ip7 */
        psubsw      mm5, IL(2)      /* mm5 = ip1 - ip2 */       

        psubsw		mm0, mm2        /* mm0 = is07 - is34 */			
        paddsw		mm2, mm2		/* mm2 = is34 * 2    */	
        
        psubsw      mm6, IH(0)      /* mm6 = ip3 - ip4 */               
        paddsw		mm2, mm0		/* mm2 = is07 + is34 */	

        psubsw		mm1, mm3		/* mm1 = is12 - is56 */	
        movq		TIRY, mm0		/* save is07-is34 */	

        paddsw		mm3, mm3		/* mm3 = is56 * 2 */	
        paddsw		mm3, mm1	    /* mm3 = is12 + is56 */
        
        psubsw      mm7, IH(2)      /* mm7 = ip5 -ip6 */
        psubsw		mm5, mm7		/* mm5 = id12 - id56 */
	    
        paddsw		mm7, mm7		/* mm7 = id56 * 2 */		
	    paddsw		mm7, mm5	    /* mm7 = id12 + id56 */
/*---------------------------------------------------------*/
/* op0 and op4 
/*---------------------------------------------------------*/
        psubsw		mm2, mm3		/* mm2 = is0734 - is1256 */
        paddsw		mm3, mm3		/* mm3 = is1256 * 2 */		

        movq		mm0, mm2	    /* mm0 = is0734 - is1256 */
        paddsw		mm3, mm2		/* mm3 = is0734 + is1256 */

        pmulhw		mm0, C(4)	    /* mm0 = xC4S4 * ( is0734 - is1256 ) - ( is0734 - is1256 ) */
        paddw		mm0, mm2		/* mm0 = xC4S4 * ( is0734 - is1256 ) */

        psrlw		mm2, 15			
        paddw		mm0, mm2		/* Truncate mm0, now it is op[4] */
            
        movq		mm2, mm3		/* mm2 = is0734 + is1256 */
        movq		OH(0), mm0		/*	op4, now mm0,mm2 are free */
            
        movq		mm0, mm3		/* mm0 = is0734 + is1256 */
        pmulhw		mm3, C(4)		/* mm3 = xC4S4 * ( is0734 +is1256 ) - ( is0734 +is1256 ) */            
        
        psrlw		mm2, 15			
        paddw		mm3, mm0		/* mm3 = xC4S4 * ( is0734 +is1256 ) */
        
        paddw		mm3, mm2		/* Truncate mm3, now it is op[0] */     
        movq		OL(0), mm3		/* save op0 */
/*---------------------------------------------------------*/
/* op2 and op6 
/*---------------------------------------------------------*/
 	    movq		mm3, TIRY		/* mm3 = irot_input_y */
        pmulhw		mm3, C(2)		/* mm3 = xC2S6 * irot_input_y - irot_input_y */
        
        movq		mm2, TIRY		/* mm2 = irot_input_y */
        movq		mm0, mm2		/* mm0 = irot_input_y */
        
        psrlw		mm2, 15		
        paddw		mm3, mm0        /* mm3 = xC2S6 * irot_input_y */
            
        paddw       mm3, mm2		/* Truncated */
        movq		mm0, mm5		/* mm0 = id12 - id56 */
        
        
        movq		mm2, mm5        /* mm2 = id12 - id56 */
        pmulhw		mm0, C(6)		/* mm0 = xC6S2 * irot_input_x */
            
        psrlw		mm2, 15			
        paddw		mm0, mm2		/* Truncated */
        
        paddsw		mm3, mm0		/* op[2] */
        movq		OL(2), mm3		/* save op[2] */
        
        
        movq		mm0, mm5		/* mm0 = id12 - id56 */
        movq		mm2, mm5		/* mm0 = id12 - id56 */
        
        pmulhw		mm5, C(2)		/* mm5 = xC2S6 * irot_input_x - irot_input_x */
        psrlw		mm2, 15		
        
        movq		mm3, TIRY		/* mm3 = irot_input_y */
        paddw		mm5, mm0		/* mm5 = xC2S6 * irot_input_x */
            
        paddw		mm5, mm2		/* Truncated */
        movq		mm2, mm3		/* mm2 = irot_input_y */	
        
        pmulhw		mm3, C(6)	    /* mm3 = xC6S2 * irot_input_y */
        psrlw		mm2, 15        
        
        paddw		mm3, mm2		/* Truncated */
        psubsw		mm3, mm5		/* mm3 = op[6] */
        
        movq		OH(2), mm3		
/*-----------------------------------------------------------------------*/
/* icommon_product1, icommon_product2                                    */
/*-----------------------------------------------------------------------*/
	    movq		mm0, C(4)       /* mm0 = xC4s4 */
	    movq		mm2, mm1        /* mm2 = is12 - is56 */	
	
        movq		mm3, mm1        /* mm3 = is12 - is56 */	
	    pmulhw		mm1, mm0		/* mm0 = xC4S4 * ( is12 - is56 ) - ( is12 - is56 ) */
	
        psrlw		mm2, 15				
	    paddw		mm1, mm3	    /* mm1 = xC4S4 * ( is12 - is56 ) */
	    
        paddw		mm1, mm2        /* Truncate mm1, now it is icommon_product1 */
	    movq		mm2, mm7        /* mm2 = id12 + id56 */
	    
        movq		mm3, mm7		/* mm3 = id12 + id56 */
        pmulhw		mm7, mm0		/* mm7 = xC4S4 * ( id12 + id56 ) - ( id12 + id56 ) */
	
        psrlw		mm2, 15		    /* For trucation */	
	    paddw		mm7, mm3		/* mm7 = xC4S4 * ( id12 + id56 ) */

	    paddw		mm7, mm2		/* Truncate mm7, now it is icommon_product2 */
/*---------------------------------------------------------*/
	    pxor		mm0, mm0		/* Clear mm0 */
	    psubsw		mm0, mm6		/* mm0 = - id34 */

	    psubsw		mm0, mm7	    /* mm0 = - ( id34 + idcommon_product2 ) = irot_input_y for 17*/
	    paddsw		mm6, mm6	    /* mm6 = id34 * 2 */

	    paddsw		mm6, mm0		/* mm6 = id34 - icommon_product2 = irot_input_x for 35 */
	    psubsw		mm4, mm1		/* mm4 = id07 - icommon_product1 = irot_input_x for 35*/

	    paddsw		mm1, mm1		/* mm1 = icommon_product1 * 2 */	    
        paddsw		mm1, mm4		/* mm1 = id07 + icommon_product1 = irot_input_x for 17*/

/*---------------------------------------------------------*/
/* op1 and op7              
/*---------------------------------------------------------*/

	    movq		mm7, C(1)       /* xC1S7 */
        movq		mm2, mm1        /* mm2 = irot_input_x */
        
        movq		mm3, mm1;       /* mm3 = irot_input_x */
        pmulhw		mm1, mm7		/* mm1 = xC1S7 * irot_input_x - irot_input_x */
            
        movq		mm7, C(7)		/* xC7S1 */
        psrlw		mm2, 15		    /* for trucation */		
            
        paddw		mm1, mm3		/* mm1 = xC1S7 * irot_input_x */
        paddw		mm1, mm2		/* Trucated */
            
        pmulhw		mm3, mm7		/* mm3 = xC7S1 * irot_input_x */
        paddw		mm3, mm2		/* Truncated */
            
        movq		mm5, mm0		/* mm5 = irot_input_y */	
        movq	    mm2, mm0        /* mm2 = irot_input_y */	
            
        movq		mm7, C(1)       /* xC1S7 */			
        pmulhw		mm0, mm7	    /* mm0 = xC1S7 * irot_input_y - irot_input_y */
        
        movq		mm7, C(7)		/* xC7S1 */	
        psrlw		mm2, 15		    /* for trucation */	
        
        paddw		mm0, mm5		/* mm0 = xC1S7 * irot_input_y */
        paddw		mm0, mm2		/* Truncated */
        
        pmulhw		mm5, mm7		/* mm5 = xC7S1 * irot_input_y */
        paddw		mm5, mm2		/* Truncated */
        
        psubsw		mm1, mm5		/* mm1 = xC1S7 * irot_input_x - xC7S1 * irot_input_y = op[1] */
        paddsw		mm3, mm0		/* mm3 = xC7S1 * irot_input_x - xC1S7 * irot_input_y = op[7] */
        
        movq		OL(1), mm1
        movq		OH(3), mm3
/*---------------------------------------------------------*/
/* op3 and op5 
/*---------------------------------------------------------*/
	    movq		mm0, C(3)       /* xC3S5 */
	    movq		mm1, C(5)       /* xC5S3 */

	    movq		mm5,mm6         /* irot_input_x */
	    movq		mm7,mm6         /* irot_input_x */

	    movq		mm2,mm4         /* irot_input_y */
	    movq		mm3,mm4         /* irot_input_y */

	    pmulhw		mm4,mm0         /* mm4 = xC3S5 * irot_input_x - irot_input_x */
	    pmulhw		mm6,mm1		    /* mm6 = xC5S3 * irot_input_y - irot_input_y */

	    psrlw		mm2,15          /* for trucation */
	    psrlw		mm5,15          /* for trucation */

	    paddw		mm4,mm3		    /* mm4 = xC3S5 * irot_input_x */
	    paddw		mm6,mm7		    /* mm6 = xC5S3 * irot_input_y */

	    paddw		mm4,mm2		    /* Truncated */
	    paddw		mm6,mm5		    /* Truncated */

	    psubsw		mm4,mm6		    /* op [3] */
	    movq		OL(3),mm4		/* Save Op[3] */

	    movq		mm4,mm3		    /* irot_input_y */
	    movq		mm6,mm7		    /* irot_input_x */

	    pmulhw		mm3,mm1		    /* mm3 = xC5S3 * irot_input_x - irot_input_x */
	    pmulhw		mm7,mm0		    /* mm7 = xC3S5 * irot_input_y - irot_input_y */

	    paddw		mm4,mm2         /* Trucated */
	    paddw		mm6,mm5         /* Trucated */

	    paddw		mm3,mm4		    /* mm3 = xC5S3 * irot_input_x */
	    paddw		mm7,mm6		    /*  mm7 = xC3S5 * irot_input_y */

	    paddw		mm3,mm7		    /* Op[5] */
	    movq		OH(1),mm3		/* Save Op[5] */
/*---------------------------------------------------------*/
/* End of Horizontal FDCT                                  */        
/*---------------------------------------------------------*/
        lea         eax, [ebx-64]
        lea         esi, [edi-64]

#undef  IL
#undef  IH
#undef  OL
#undef  OH
#define IL(i)   [eax + 16 * i]
#define IH(i)   [ebx + 16 * i]
#define OL(i)   [eax + 16 * i]
#define OH(i)   [ebx + 16 * i]

/******************************************************/
/* Do 4x8 Transpose  is done through 2 4x4 Transpose  */
/******************************************************/
    	movq		mm4, IH(0)		/* mm4=e3e2e1e0 */	
        movq		mm0, IH(1)		/* mm4=f3f2f1f0 */	
        
        movq		mm5, mm4		/* make a copy  */			
        punpcklwd	mm4, mm0		/* mm4=f1e1f0e0 */	
        
        punpckhwd	mm5, mm0		/* mm5=f3e3f2e2 */        
        movq		mm6, IH(2)		/* mm6=g3g2g1g0 */         

        movq		mm0, IH(3)		/* mm0=h3h2h1h0 */ 
        movq		mm7, mm6		/* mm7=g3g2g1g0 */         
        
        punpcklwd	mm6, mm0		/* mm6=h1g1h0g0 */ 
        punpckhwd	mm7, mm0		/* mm7=h3g3h2g2 */ 
        
        movq		mm3, mm4		/* mm4=f1e1f0e0 */	
        punpckldq	mm4, mm6		/* mm4=h0g0f0e0 */	
        
        punpckhdq	mm3, mm6		/* mm3=h1g1f1e1 */	
        movq		mm6, mm5		/* mm5=f3e3f2e2 */

        punpckldq	mm5, mm7		/* mm5=h2g2f2e2 */ 
        movq        IH(0), mm4      /* saveh0g0f0e0 */       
        
        punpckhdq	mm6, mm7		/* mm6=h3g3f3e3 */        
        movq        IH(2), mm5      /* saveh2g2f2e2 */

        movq        IH(3), mm6      /* saveh3g3f3e3 */

/*----------------------------------------------------*/        
/*    mm3 in use for IH(1)                            */
/*----------------------------------------------------*/

        movq		mm4, IL(0)		/* mm4=a3a2a1a0 */	
        movq		mm0, IL(1)		/* mm0=b3b2b1b0 */	
        
        movq        mm5, mm4        /* mm5=a3a2a1a0 */
        punpcklwd   mm4, mm0        /* mm4=b1a1b0a0 */
        
        punpckhwd	mm5, mm0		/* mm5=b3a3b2a2 */	                
        movq		mm6, IL(2)		/* mm6=c3c2c1c0 */ 
                
        movq		mm0, IL(3)	    /* mm0=d3d2d1d0 */         
        movq        mm7, mm6        /* mm7=c3c2c1c0 */

        punpcklwd	mm6, mm0		/* mm6=d1c1d0c0 */ 
        punpckhwd	mm7, mm0		/* mm7=c3c3d2c2 */ 
        
        movq		mm1, mm4		/* mm4=b1a1b0a0 */	
        punpckldq	mm4, mm6		/* mm4=d0c0b0a0 */	
        
        punpckhdq	mm1, mm6		/* mm1=d1c1b1a1 */	
        movq		mm2, mm5		/* mm5=b3a3b2a2 */

        punpckldq	mm5, mm7		/* mm5=d2c2b2a2 */ 
        punpckhdq	mm2, mm7		/* mm6=d3c3b3a3 */
    
        movq        IL(2), mm5       /* saved2c2b2a2 */

/*----------------------------------------------------*/        
/*    mm1 in use for IL(1)                            */
/*    mm2 in use for IL(3)                            */
/*    mm3 in use for IH(1)                            */
/*    mm4 in use for IH(0)                            */
/*----------------------------------------------------*/

/******************************************************/
/* Let's do the 4x8 forward DCT                       */
/******************************************************/
        movq        mm0, mm4        /* mm4 = ip0 */
        movq        mm5, mm1        /* mm5 = ip1 */      
        
        movq        mm6, mm2        /* mm6 = ip3 */      
        movq        mm7, mm3        /* mm7 = ip5 */      	

        paddsw      mm0, IH(3)      /* mm0 = ip0 + ip7 */
        paddsw      mm1, IL(2)      /* mm1 = ip1 + ip2 */

        paddsw      mm2, IH(0)      /* mm2 = ip3 + ip4 */
        paddsw      mm3, IH(2)      /* mm3 = ip5 + ip6 */

        psubsw      mm4, IH(3)      /* mm4 = ip0 - ip7 */
        psubsw      mm5, IL(2)      /* mm5 = ip1 - ip2 */       

        psubsw		mm0, mm2        /* mm0 = is07 - is34 */			
        paddsw		mm2, mm2		/* mm2 = is34 * 2    */	
        
        psubsw      mm6, IH(0)      /* mm6 = ip3 - ip4 */               
        paddsw		mm2, mm0		/* mm2 = is07 + is34 */	

        psubsw		mm1, mm3		/* mm1 = is12 - is56 */	
        movq		TIRY, mm0		/* save is07-is34 */	

        paddsw		mm3, mm3		/* mm3 = is56 * 2 */	
        paddsw		mm3, mm1	    /* mm3 = is12 + is56 */
        
        psubsw      mm7, IH(2)      /* mm7 = ip5 -ip6 */
        psubsw		mm5, mm7		/* mm5 = id12 - id56 */
	    
        paddsw		mm7, mm7		/* mm7 = id56 * 2 */		
	    paddsw		mm7, mm5	    /* mm7 = id12 + id56 */
/*---------------------------------------------------------*/
/* op0 and op4 
/*---------------------------------------------------------*/
        psubsw		mm2, mm3		/* mm2 = is0734 - is1256 */
        paddsw		mm3, mm3		/* mm3 = is1256 * 2 */		

        movq		mm0, mm2	    /* mm0 = is0734 - is1256 */
        paddsw		mm3, mm2		/* mm3 = is0734 + is1256 */

        pmulhw		mm0, C(4)	    /* mm0 = xC4S4 * ( is0734 - is1256 ) - ( is0734 - is1256 ) */
        paddw		mm0, mm2		/* mm0 = xC4S4 * ( is0734 - is1256 ) */

        psrlw		mm2, 15			
        paddw		mm0, mm2		/* Truncate mm0, now it is op[4] */
       
        movq        mm2, mm0      
        psrlw       mm0, 15
        
        paddw       mm0, mm2
        psraw       mm0, 1        

        movq		OH(0), mm0		/*	op4, now mm0,mm2 are free */
        movq		mm2, mm3		/* mm2 = is0734 + is1256 */
            

        movq		mm0, mm3		/* mm0 = is0734 + is1256 */
        pmulhw		mm3, C(4)		/* mm3 = xC4S4 * ( is0734 +is1256 ) - ( is0734 +is1256 ) */            
        
        psrlw		mm2, 15			
        paddw		mm3, mm0		/* mm3 = xC4S4 * ( is0734 +is1256 ) */
        
        paddw		mm3, mm2		/* Truncate mm3, now it is op[0] */     
        movq        mm2, mm3

        psrlw       mm3, 15
        paddw       mm3, mm2
        
        psraw       mm3, 1
        movq		OL(0), mm3		/* save op0 */
/*---------------------------------------------------------*/
/* op2 and op6 
/*---------------------------------------------------------*/
 	    movq		mm3, TIRY		/* mm3 = irot_input_y */
        pmulhw		mm3, C(2)		/* mm3 = xC2S6 * irot_input_y - irot_input_y */
        
        movq		mm2, TIRY		/* mm2 = irot_input_y */
        movq		mm0, mm2		/* mm0 = irot_input_y */
        
        psrlw		mm2, 15		
        paddw		mm3, mm0        /* mm3 = xC2S6 * irot_input_y */
            
        paddw       mm3, mm2		/* Truncated */
        movq		mm0, mm5		/* mm0 = id12 - id56 */
        
        
        movq		mm2, mm5        /* mm2 = id12 - id56 */
        pmulhw		mm0, C(6)		/* mm0 = xC6S2 * irot_input_x */
            
        psrlw		mm2, 15			
        paddw		mm0, mm2		/* Truncated */
        
        paddsw		mm3, mm0		/* op[2] */
        movq        mm0, mm3

        psrlw       mm3, 15
        paddw       mm3, mm0
        
        psraw       mm3, 1                
        movq		OL(2), mm3		/* save op[2] */        
        
        movq		mm0, mm5		/* mm0 = id12 - id56 */
        movq		mm2, mm5		/* mm0 = id12 - id56 */
        
        pmulhw		mm5, C(2)		/* mm5 = xC2S6 * irot_input_x - irot_input_x */
        psrlw		mm2, 15		
        
        movq		mm3, TIRY		/* mm3 = irot_input_y */
        paddw		mm5, mm0		/* mm5 = xC2S6 * irot_input_x */
            
        paddw		mm5, mm2		/* Truncated */
        movq		mm2, mm3		/* mm2 = irot_input_y */	
        
        pmulhw		mm3, C(6)	    /* mm3 = xC6S2 * irot_input_y */
        psrlw		mm2, 15        
        
        paddw		mm3, mm2		/* Truncated */
        psubsw		mm3, mm5		/* mm3 = op[6] */

        movq        mm5, mm3
        psrlw       mm3,  15
        
        paddw       mm3, mm5
        psraw       mm3, 1

        movq		OH(2), mm3		
/*-----------------------------------------------------------------------*/
/* icommon_product1, icommon_product2                                    */
/*-----------------------------------------------------------------------*/
	    movq		mm0, C(4)       /* mm0 = xC4s4 */
	    movq		mm2, mm1        /* mm2 = is12 - is56 */	
	
        movq		mm3, mm1        /* mm3 = is12 - is56 */	
	    pmulhw		mm1, mm0		/* mm0 = xC4S4 * ( is12 - is56 ) - ( is12 - is56 ) */
	
        psrlw		mm2, 15				
	    paddw		mm1, mm3	    /* mm1 = xC4S4 * ( is12 - is56 ) */
	    
        paddw		mm1, mm2        /* Truncate mm1, now it is icommon_product1 */
	    movq		mm2, mm7        /* mm2 = id12 + id56 */
	    
        movq		mm3, mm7		/* mm3 = id12 + id56 */
        pmulhw		mm7, mm0		/* mm7 = xC4S4 * ( id12 + id56 ) - ( id12 + id56 ) */
	
        psrlw		mm2, 15		    /* For trucation */	
	    paddw		mm7, mm3		/* mm7 = xC4S4 * ( id12 + id56 ) */

	    paddw		mm7, mm2		/* Truncate mm7, now it is icommon_product2 */
/*---------------------------------------------------------*/
	    pxor		mm0, mm0		/* Clear mm0 */
	    psubsw		mm0, mm6		/* mm0 = - id34 */

	    psubsw		mm0, mm7	    /* mm0 = - ( id34 + idcommon_product2 ) = irot_input_y for 17*/
	    paddsw		mm6, mm6	    /* mm6 = id34 * 2 */

	    paddsw		mm6, mm0		/* mm6 = id34 - icommon_product2 = irot_input_x for 35 */
	    psubsw		mm4, mm1		/* mm4 = id07 - icommon_product1 = irot_input_x for 35*/

	    paddsw		mm1, mm1		/* mm1 = icommon_product1 * 2 */	    
        paddsw		mm1, mm4		/* mm1 = id07 + icommon_product1 = irot_input_x for 17*/

/*---------------------------------------------------------*/
/* op1 and op7              
/*---------------------------------------------------------*/
	    movq		mm7, C(1)       /* xC1S7 */
        movq		mm2, mm1        /* mm2 = irot_input_x */
        
        movq		mm3, mm1;       /* mm3 = irot_input_x */
        pmulhw		mm1, mm7		/* mm1 = xC1S7 * irot_input_x - irot_input_x */
            
        movq		mm7, C(7)		/* xC7S1 */
        psrlw		mm2, 15		    /* for trucation */		
            
        paddw		mm1, mm3		/* mm1 = xC1S7 * irot_input_x */
        paddw		mm1, mm2		/* Trucated */
            
        pmulhw		mm3, mm7		/* mm3 = xC7S1 * irot_input_x */
        paddw		mm3, mm2		/* Truncated */
            
        movq		mm5, mm0		/* mm5 = irot_input_y */	
        movq	    mm2, mm0        /* mm2 = irot_input_y */	
            
        movq		mm7, C(1)       /* xC1S7 */			
        pmulhw		mm0, mm7	    /* mm0 = xC1S7 * irot_input_y - irot_input_y */
        
        movq		mm7, C(7)		/* xC7S1 */	
        psrlw		mm2, 15		    /* for trucation */	
        
        paddw		mm0, mm5		/* mm0 = xC1S7 * irot_input_y */
        paddw		mm0, mm2		/* Truncated */
        
        pmulhw		mm5, mm7		/* mm5 = xC7S1 * irot_input_y */
        paddw		mm5, mm2		/* Truncated */
        
        psubsw		mm1, mm5		/* mm1 = xC1S7 * irot_input_x - xC7S1 * irot_input_y = op[1] */
        paddsw		mm3, mm0		/* mm3 = xC7S1 * irot_input_x - xC1S7 * irot_input_y = op[7] */

        movq        mm5, mm1
        movq        mm0, mm3        

        psrlw       mm1, 15
        psrlw       mm3, 15

        paddw       mm1, mm5
        paddw       mm3, mm0

        psraw       mm1, 1
        psraw       mm3, 1
            
        movq		OL(1), mm1
        movq		OH(3), mm3
/*---------------------------------------------------------*/
/* op3 and op5 
/*---------------------------------------------------------*/
	    movq		mm0, C(3)       /* xC3S5 */
	    movq		mm1, C(5)       /* xC5S3 */

	    movq		mm5,mm6         /* irot_input_x */
	    movq		mm7,mm6         /* irot_input_x */

	    movq		mm2,mm4         /* irot_input_y */
	    movq		mm3,mm4         /* irot_input_y */

	    pmulhw		mm4,mm0         /* mm4 = xC3S5 * irot_input_x - irot_input_x */
	    pmulhw		mm6,mm1		    /* mm6 = xC5S3 * irot_input_y - irot_input_y */

	    psrlw		mm2,15          /* for trucation */
	    psrlw		mm5,15          /* for trucation */

	    paddw		mm4,mm3		    /* mm4 = xC3S5 * irot_input_x */
	    paddw		mm6,mm7		    /* mm6 = xC5S3 * irot_input_y */

	    paddw		mm4,mm2		    /* Truncated */
	    paddw		mm6,mm5		    /* Truncated */

	    psubsw		mm4,mm6		    /* op [3] */
        movq        mm6,mm4

        psrlw       mm4,15        
        paddw       mm4,mm6

        psraw       mm4,1
        movq		OL(3),mm4		/* Save Op[3] */

	    movq		mm4,mm3		    /* irot_input_y */
	    movq		mm6,mm7		    /* irot_input_x */

	    pmulhw		mm3,mm1		    /* mm3 = xC5S3 * irot_input_x - irot_input_x */
	    pmulhw		mm7,mm0		    /* mm7 = xC3S5 * irot_input_y - irot_input_y */

	    paddw		mm4,mm2         /* Trucated */
	    paddw		mm6,mm5         /* Trucated */

	    paddw		mm3,mm4		    /* mm3 = xC5S3 * irot_input_x */
	    paddw		mm7,mm6		    /*  mm7 = xC3S5 * irot_input_y */

	    paddw		mm3,mm7		    /* Op[5] */
        movq        mm7,mm3

        psrlw       mm3,15        
        paddw       mm3,mm7

        psraw       mm3,1
        movq		OH(1),mm3		/* Save Op[5] */
/*---------------------------------------------------------*/
/* End of 4x8 1-D FDCT                                     */        
/*---------------------------------------------------------*/
        lea         eax, [eax+8]
        lea         ebx, [ebx+8]

/******************************************************/
/* Do 4x8 Transpose  is done through 2 4x4 Transpose  */
/******************************************************/
    	movq		mm4, IH(0)		/* mm4=e3e2e1e0 */	
        movq		mm0, IH(1)		/* mm4=f3f2f1f0 */	
        
        movq		mm5, mm4		/* make a copy  */			
        punpcklwd	mm4, mm0		/* mm4=f1e1f0e0 */	
        
        punpckhwd	mm5, mm0		/* mm5=f3e3f2e2 */        
        movq		mm6, IH(2)		/* mm6=g3g2g1g0 */         

        movq		mm0, IH(3)		/* mm0=h3h2h1h0 */ 
        movq		mm7, mm6		/* mm7=g3g2g1g0 */         
        
        punpcklwd	mm6, mm0		/* mm6=h1g1h0g0 */ 
        punpckhwd	mm7, mm0		/* mm7=h3g3h2g2 */ 
        
        movq		mm3, mm4		/* mm4=f1e1f0e0 */	
        punpckldq	mm4, mm6		/* mm4=h0g0f0e0 */	
        
        punpckhdq	mm3, mm6		/* mm3=h1g1f1e1 */	
        movq		mm6, mm5		/* mm5=f3e3f2e2 */

        punpckldq	mm5, mm7		/* mm5=h2g2f2e2 */ 
        movq        IH(0), mm4      /* saveh0g0f0e0 */       
        
        punpckhdq	mm6, mm7		/* mm6=h3g3f3e3 */        
        movq        IH(2), mm5      /* saveh2g2f2e2 */

        movq        IH(3), mm6      /* saveh3g3f3e3 */

/*----------------------------------------------------*/        
/*    mm3 in use for IH(1)                            */
/*----------------------------------------------------*/

        movq		mm4, IL(0)		/* mm4=a3a2a1a0 */	
        movq		mm0, IL(1)		/* mm0=b3b2b1b0 */	
        
        movq        mm5, mm4        /* mm5=a3a2a1a0 */
        punpcklwd   mm4, mm0        /* mm4=b1a1b0a0 */
        
        punpckhwd	mm5, mm0		/* mm5=b3a3b2a2 */	                
        movq		mm6, IL(2)		/* mm6=c3c2c1c0 */ 
                
        movq		mm0, IL(3)	    /* mm0=d3d2d1d0 */         
        movq        mm7, mm6        /* mm7=c3c2c1c0 */

        punpcklwd	mm6, mm0		/* mm6=d1c1d0c0 */ 
        punpckhwd	mm7, mm0		/* mm7=c3c3d2c2 */ 
        
        movq		mm1, mm4		/* mm4=b1a1b0a0 */	
        punpckldq	mm4, mm6		/* mm4=d0c0b0a0 */	
        
        punpckhdq	mm1, mm6		/* mm1=d1c1b1a1 */	
        movq		mm2, mm5		/* mm5=b3a3b2a2 */

        punpckldq	mm5, mm7		/* mm5=d2c2b2a2 */ 
        punpckhdq	mm2, mm7		/* mm6=d3c3b3a3 */
    
        movq        IL(2), mm5       /* saved2c2b2a2 */

/*----------------------------------------------------*/        
/*    mm1 in use for IL(1)                            */
/*    mm2 in use for IL(3)                            */
/*    mm3 in use for IH(1)                            */
/*    mm4 in use for IH(0)                            */
/*----------------------------------------------------*/

/******************************************************/
/* Let's do the 4x8 forward DCT                       */
/******************************************************/
        movq        mm0, mm4        /* mm4 = ip0 */
        movq        mm5, mm1        /* mm5 = ip1 */      
        
        movq        mm6, mm2        /* mm6 = ip3 */      
        movq        mm7, mm3        /* mm7 = ip5 */      	

        paddsw      mm0, IH(3)      /* mm0 = ip0 + ip7 */
        paddsw      mm1, IL(2)      /* mm1 = ip1 + ip2 */

        paddsw      mm2, IH(0)      /* mm2 = ip3 + ip4 */
        paddsw      mm3, IH(2)      /* mm3 = ip5 + ip6 */

        psubsw      mm4, IH(3)      /* mm4 = ip0 - ip7 */
        psubsw      mm5, IL(2)      /* mm5 = ip1 - ip2 */       

        psubsw		mm0, mm2        /* mm0 = is07 - is34 */			
        paddsw		mm2, mm2		/* mm2 = is34 * 2    */	
        
        psubsw      mm6, IH(0)      /* mm6 = ip3 - ip4 */               
        paddsw		mm2, mm0		/* mm2 = is07 + is34 */	

        psubsw		mm1, mm3		/* mm1 = is12 - is56 */	
        movq		TIRY, mm0		/* save is07-is34 */	

        paddsw		mm3, mm3		/* mm3 = is56 * 2 */	
        paddsw		mm3, mm1	    /* mm3 = is12 + is56 */
        
        psubsw      mm7, IH(2)      /* mm7 = ip5 -ip6 */
        psubsw		mm5, mm7		/* mm5 = id12 - id56 */
	    
        paddsw		mm7, mm7		/* mm7 = id56 * 2 */		
	    paddsw		mm7, mm5	    /* mm7 = id12 + id56 */
/*---------------------------------------------------------*/
/* op0 and op4 
/*---------------------------------------------------------*/
        psubsw		mm2, mm3		/* mm2 = is0734 - is1256 */
        paddsw		mm3, mm3		/* mm3 = is1256 * 2 */		

        movq		mm0, mm2	    /* mm0 = is0734 - is1256 */
        paddsw		mm3, mm2		/* mm3 = is0734 + is1256 */

        pmulhw		mm0, C(4)	    /* mm0 = xC4S4 * ( is0734 - is1256 ) - ( is0734 - is1256 ) */
        paddw		mm0, mm2		/* mm0 = xC4S4 * ( is0734 - is1256 ) */

        psrlw		mm2, 15			
        paddw		mm0, mm2		/* Truncate mm0, now it is op[4] */
       
        movq        mm2, mm0      
        psrlw       mm0, 15
        
        paddw       mm0, mm2
        psraw       mm0, 1        

        movq		OH(0), mm0		/*	op4, now mm0,mm2 are free */
        movq		mm2, mm3		/* mm2 = is0734 + is1256 */
            

        movq		mm0, mm3		/* mm0 = is0734 + is1256 */
        pmulhw		mm3, C(4)		/* mm3 = xC4S4 * ( is0734 +is1256 ) - ( is0734 +is1256 ) */            
        
        psrlw		mm2, 15			
        paddw		mm3, mm0		/* mm3 = xC4S4 * ( is0734 +is1256 ) */
        
        paddw		mm3, mm2		/* Truncate mm3, now it is op[0] */     
        movq        mm2, mm3

        psrlw       mm3, 15
        paddw       mm3, mm2
        
        psraw       mm3, 1
        movq		OL(0), mm3		/* save op0 */
/*---------------------------------------------------------*/
/* op2 and op6 
/*---------------------------------------------------------*/
 	    movq		mm3, TIRY		/* mm3 = irot_input_y */
        pmulhw		mm3, C(2)		/* mm3 = xC2S6 * irot_input_y - irot_input_y */
        
        movq		mm2, TIRY		/* mm2 = irot_input_y */
        movq		mm0, mm2		/* mm0 = irot_input_y */
        
        psrlw		mm2, 15		
        paddw		mm3, mm0        /* mm3 = xC2S6 * irot_input_y */
            
        paddw       mm3, mm2		/* Truncated */
        movq		mm0, mm5		/* mm0 = id12 - id56 */
        
        
        movq		mm2, mm5        /* mm2 = id12 - id56 */
        pmulhw		mm0, C(6)		/* mm0 = xC6S2 * irot_input_x */
            
        psrlw		mm2, 15			
        paddw		mm0, mm2		/* Truncated */
        
        paddsw		mm3, mm0		/* op[2] */
        movq        mm0, mm3

        psrlw       mm3, 15
        paddw       mm3, mm0
        
        psraw       mm3, 1                
        movq		OL(2), mm3		/* save op[2] */        
        
        movq		mm0, mm5		/* mm0 = id12 - id56 */
        movq		mm2, mm5		/* mm0 = id12 - id56 */
        
        pmulhw		mm5, C(2)		/* mm5 = xC2S6 * irot_input_x - irot_input_x */
        psrlw		mm2, 15		
        
        movq		mm3, TIRY		/* mm3 = irot_input_y */
        paddw		mm5, mm0		/* mm5 = xC2S6 * irot_input_x */
            
        paddw		mm5, mm2		/* Truncated */
        movq		mm2, mm3		/* mm2 = irot_input_y */	
        
        pmulhw		mm3, C(6)	    /* mm3 = xC6S2 * irot_input_y */
        psrlw		mm2, 15        
        
        paddw		mm3, mm2		/* Truncated */
        psubsw		mm3, mm5		/* mm3 = op[6] */

        movq        mm5, mm3
        psrlw       mm3,  15
        
        paddw       mm3, mm5
        psraw       mm3, 1

        movq		OH(2), mm3		
/*-----------------------------------------------------------------------*/
/* icommon_product1, icommon_product2                                    */
/*-----------------------------------------------------------------------*/
	    movq		mm0, C(4)       /* mm0 = xC4s4 */
	    movq		mm2, mm1        /* mm2 = is12 - is56 */	
	
        movq		mm3, mm1        /* mm3 = is12 - is56 */	
	    pmulhw		mm1, mm0		/* mm0 = xC4S4 * ( is12 - is56 ) - ( is12 - is56 ) */
	
        psrlw		mm2, 15				
	    paddw		mm1, mm3	    /* mm1 = xC4S4 * ( is12 - is56 ) */
	    
        paddw		mm1, mm2        /* Truncate mm1, now it is icommon_product1 */
	    movq		mm2, mm7        /* mm2 = id12 + id56 */
	    
        movq		mm3, mm7		/* mm3 = id12 + id56 */
        pmulhw		mm7, mm0		/* mm7 = xC4S4 * ( id12 + id56 ) - ( id12 + id56 ) */
	
        psrlw		mm2, 15		    /* For trucation */	
	    paddw		mm7, mm3		/* mm7 = xC4S4 * ( id12 + id56 ) */

	    paddw		mm7, mm2		/* Truncate mm7, now it is icommon_product2 */
/*---------------------------------------------------------*/
	    pxor		mm0, mm0		/* Clear mm0 */
	    psubsw		mm0, mm6		/* mm0 = - id34 */

	    psubsw		mm0, mm7	    /* mm0 = - ( id34 + idcommon_product2 ) = irot_input_y for 17*/
	    paddsw		mm6, mm6	    /* mm6 = id34 * 2 */

	    paddsw		mm6, mm0		/* mm6 = id34 - icommon_product2 = irot_input_x for 35 */
	    psubsw		mm4, mm1		/* mm4 = id07 - icommon_product1 = irot_input_x for 35*/

	    paddsw		mm1, mm1		/* mm1 = icommon_product1 * 2 */	    
        paddsw		mm1, mm4		/* mm1 = id07 + icommon_product1 = irot_input_x for 17*/

/*---------------------------------------------------------*/
/* op1 and op7              
/*---------------------------------------------------------*/
	    movq		mm7, C(1)       /* xC1S7 */
        movq		mm2, mm1        /* mm2 = irot_input_x */
        
        movq		mm3, mm1;       /* mm3 = irot_input_x */
        pmulhw		mm1, mm7		/* mm1 = xC1S7 * irot_input_x - irot_input_x */
            
        movq		mm7, C(7)		/* xC7S1 */
        psrlw		mm2, 15		    /* for trucation */		
            
        paddw		mm1, mm3		/* mm1 = xC1S7 * irot_input_x */
        paddw		mm1, mm2		/* Trucated */
            
        pmulhw		mm3, mm7		/* mm3 = xC7S1 * irot_input_x */
        paddw		mm3, mm2		/* Truncated */
            
        movq		mm5, mm0		/* mm5 = irot_input_y */	
        movq	    mm2, mm0        /* mm2 = irot_input_y */	
            
        movq		mm7, C(1)       /* xC1S7 */			
        pmulhw		mm0, mm7	    /* mm0 = xC1S7 * irot_input_y - irot_input_y */
        
        movq		mm7, C(7)		/* xC7S1 */	
        psrlw		mm2, 15		    /* for trucation */	
        
        paddw		mm0, mm5		/* mm0 = xC1S7 * irot_input_y */
        paddw		mm0, mm2		/* Truncated */
        
        pmulhw		mm5, mm7		/* mm5 = xC7S1 * irot_input_y */
        paddw		mm5, mm2		/* Truncated */
        
        psubsw		mm1, mm5		/* mm1 = xC1S7 * irot_input_x - xC7S1 * irot_input_y = op[1] */
        paddsw		mm3, mm0		/* mm3 = xC7S1 * irot_input_x - xC1S7 * irot_input_y = op[7] */

        movq        mm5, mm1
        movq        mm0, mm3        

        psrlw       mm1, 15
        psrlw       mm3, 15

        paddw       mm1, mm5
        paddw       mm3, mm0

        psraw       mm1, 1
        psraw       mm3, 1
            
        movq		OL(1), mm1
        movq		OH(3), mm3
/*---------------------------------------------------------*/
/* op3 and op5 
/*---------------------------------------------------------*/
	    movq		mm0, C(3)       /* xC3S5 */
	    movq		mm1, C(5)       /* xC5S3 */

	    movq		mm5,mm6         /* irot_input_x */
	    movq		mm7,mm6         /* irot_input_x */

	    movq		mm2,mm4         /* irot_input_y */
	    movq		mm3,mm4         /* irot_input_y */

	    pmulhw		mm4,mm0         /* mm4 = xC3S5 * irot_input_x - irot_input_x */
	    pmulhw		mm6,mm1		    /* mm6 = xC5S3 * irot_input_y - irot_input_y */

	    psrlw		mm2,15          /* for trucation */
	    psrlw		mm5,15          /* for trucation */

	    paddw		mm4,mm3		    /* mm4 = xC3S5 * irot_input_x */
	    paddw		mm6,mm7		    /* mm6 = xC5S3 * irot_input_y */

	    paddw		mm4,mm2		    /* Truncated */
	    paddw		mm6,mm5		    /* Truncated */

	    psubsw		mm4,mm6		    /* op [3] */
        movq        mm6,mm4

        psrlw       mm4,15        
        paddw       mm4,mm6

        psraw       mm4,1
        movq		OL(3),mm4		/* Save Op[3] */

	    movq		mm4,mm3		    /* irot_input_y */
	    movq		mm6,mm7		    /* irot_input_x */

	    pmulhw		mm3,mm1		    /* mm3 = xC5S3 * irot_input_x - irot_input_x */
	    pmulhw		mm7,mm0		    /* mm7 = xC3S5 * irot_input_y - irot_input_y */

	    paddw		mm4,mm2         /* Trucated */
	    paddw		mm6,mm5         /* Trucated */

	    paddw		mm3,mm4		    /* mm3 = xC5S3 * irot_input_x */
	    paddw		mm7,mm6		    /*  mm7 = xC3S5 * irot_input_y */

	    paddw		mm3,mm7		    /* Op[5] */
        movq        mm7,mm3

        psrlw       mm3,15        
        paddw       mm3,mm7

        psraw       mm3,1
        movq		OH(1),mm3		/* Save Op[5] */
/*---------------------------------------------------------*/
/* End of 4x8 1-D FDCT                                     */        
/*---------------------------------------------------------*/


    }/* end of _asm code section */
}



