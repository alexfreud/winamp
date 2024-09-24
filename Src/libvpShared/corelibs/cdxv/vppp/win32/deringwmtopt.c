/****************************************************************************
 *
 *   Module Title :     DeRingingWmtOpt.c
 *
 *   Description  :     Optimized functions for PostProcessor
 *
 ***************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "postp.h"

/****************************************************************************
*  MAcros
****************************************************************************/        
#pragma warning(disable:4305)
#pragma warning(disable:4731)

/****************************************************************************
*  Module Statics
****************************************************************************/        
#if defined(_WIN32_WCE)
#else
__declspec(align(16)) static unsigned short eight128s []= { 128, 128, 128, 128, 128, 128, 128, 128};
__declspec(align(16)) static unsigned short eight64s[]  = { 64,  64,  64,  64, 64,  64,  64,  64};
__declspec(align(16)) static char eight64c [] = { 64, 64, 64,64,64,64,64,64};
__declspec(align(16)) static char eight32c [] = { 32,32,32,32,32,32,32,32};
__declspec(align(16)) static char eight127c []= { 127, 127, 127, 127, 127, 127, 127, 127};
__declspec(align(16)) static char eight128c []= { 128, 128, 128, 128, 128, 128, 128, 128};
__declspec(align(16)) static unsigned char eight223c[] = { 223,223,223,223,223,223,223,223};
__declspec(align(16)) static unsigned char eight231c[] = { 231,231,231,231,231,231,231,231};
#endif
/****************************************************************************
*  Imports
****************************************************************************/              
extern UINT32 SharpenModifier[];

/****************************************************************************
 * 
 *  ROUTINE       : DeRingBlockStrong_WMT
 *
 *  INPUTS        : const POSTPROC_INSTANCE *pbi : Pointer to post-processor instance.
 *                  const UINT8 *SrcPtr          : Pointer to input image.
 *                  UINT8 *DstPtr                : Pointer to output image.
 *                  const INT32 Pitch            : Image stride.
 *                  UINT32 FragQIndex            : Q-index block encoded with.
 *                  UINT32 *QuantScale           : Array of quantization scale factors.
 *                               
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Filtering a block for de-ringing purpose.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void DeringBlockStrong_WMT
( 
    const POSTPROC_INSTANCE *pbi, 
    const UINT8 *SrcPtr,
    UINT8 *DstPtr,
    const INT32 Pitch,
    UINT32 FragQIndex,
    UINT32 *QuantScale
)
{
#if defined(_WIN32_WCE)
	return;
#else

	__declspec(align(16)) short UDMod[72];
	__declspec(align(16)) short	LRMod[128];

	unsigned int PlaneLineStep = Pitch;
	const unsigned char *Src   = SrcPtr;
	unsigned char *Des         = DstPtr;
    
	short *UDPointer = UDMod;
	short *LRPointer = LRMod;
    
    UINT32 QStep  = QuantScale[FragQIndex];
	INT32 Sharpen = SharpenModifier[FragQIndex];
	(void) pbi;

	__asm 
	{
		push		esi
		push		edi
		
		mov			esi,	Src						/* Source Pointer */
		mov			edi,	UDPointer				/* UD modifier pointer */

		push		ecx
		push		edx

		mov			ecx,	PlaneLineStep			/* Pitch Step */
        xor         edx,    edx

		push		eax
		push		ebx

		mov			eax,	QStep					/* QValue */
		mov			ebx,	Sharpen					/* Sharpen */

		movd		mm0,	eax						/* QValue */
		movd		mm2,	ebx						/* sharpen */

        push        ebp

		punpcklbw	mm0,	mm0						/* 00 00 00 QQ */
        sub         edx,    ecx                     /* Negative Pitch */

		punpcklbw	mm2,	mm2						/* 00 00 00 SS */
        pxor        mm7,    mm7                     /* clear mm7 for unpacks */

		punpcklbw	mm0,	mm0						/* 00 00 qq qq */
		mov			eax,	LRPointer				/* Left and Right Modifier */                

		punpcklbw	mm2,	mm2						/* 00 00 ss ss */
		lea         ebx,    [esi+ecx*8]             /* Source Pointer of last row */        

		punpcklbw	mm0,	mm0						/* qq qq qq qq */
		movq        mm1,    mm0;                    /* make a copy */
		
		punpcklbw	mm2,	mm2						/* ss ss ss ss */
		paddb		mm1,	mm0						/* QValue * 2 */

        paddb       mm1,    mm0                     /* High = 3 * Qvalue */
        paddusb		mm1,	eight223c				/* clamping high to 32 */	

		paddb       mm0,    eight32c                /* 32+QValues */
		psubusb		mm1,	eight223c				/* Get the real value back */

        movq		mm3,	eight127c				/* 7f 7f 7f 7f 7f 7f 7f 7f */
        pandn       mm1,    mm3                     /* ClampHigh */

        /* mm0,mm1,mm2,mm7 are in use  */
        /* mm0---> QValue+32           */
        /* mm1---> ClampHigh		   */
		/* mm2---> Sharpen             */
		/* mm7---> Cleared for unpack  */

FillModLoop1:
        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */
        pxor        xmm7,   xmm7                    /* clear xmm7 */ 

        movq        mm4,    QWORD PTR [esi+edx]     /* Pixels on top pu */
        movq        mm5,    mm3                     /* make a copy of p */

        psubusb     mm3,    mm4                     /* p-pu */       
        psubusb     mm4,    mm5                     /* pu-p */

        por         mm3,    mm4                     /* abs(p-pu) */
        movq        mm6,    mm0                     /* 32+QValues */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128c				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64c				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */

        movq2dq     xmm0,   mm6                     
        movq        mm4,    QWORD PTR [esi-1]       /* Pixels on top pu */

        punpcklbw	xmm7,	xmm0					/* extended to words */
        movq        mm5,    mm3                     /* make a copy of p */

        psraw		xmm7,	8						/* sign extended */
        psubusb     mm3,    mm4                     /* p-pu */

        movdqa      [edi],  xmm7                    /* writeout UDmod*/
        psubusb     mm4,    mm5                     /* pu-p */

        por         mm3,    mm4                     /* abs(p-pu) */
        movq        mm6,    mm0                     /* 32+QValues */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128c				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64c				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */

        pxor        xmm7,   xmm7                    /* clear xmm7 */
        movq        mm4,    QWORD PTR [esi+1]       /* Pixels on top pu */

   		movq2dq 	xmm0,	mm6						
        movq        mm5,    mm3                     /* make a copy of p */

        punpcklbw   xmm7,   xmm0                    /* extened  to shorts */
        psubusb     mm3,    mm4                     /* p-pu */

		psraw		xmm7,	8						/* sign extended */
        psubusb     mm4,    mm5                     /* pu-p */

        movdqa      [eax],  xmm7                    /* writeout UDmod*/
        por         mm3,    mm4                     /* abs(p-pu) */

        movq        mm6,    mm0                     /* 32+QValues */
        pxor        xmm7,   xmm7                    /* clear xmm7 */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128c				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64c				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
        add         esi,    ecx
        
        movq2dq     xmm0,   mm6
        add         edi,    16                  

        punpcklbw	xmm7,	mm0						/* extended to shorts */
        add         eax,    16      

        psraw		xmm7,	8						/* sign extended */
        cmp         esi,    ebx

        movdqa      [eax+112], xmm7                 /* writeout UDmod*/		
        jne         FillModLoop1
        
        /* last UDMod */

        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */
        pxor        xmm7,   xmm7                    /* clear xmm7 */


        movq        mm4,    QWORD PTR [esi+edx]     /* Pixels on top pu */
        movq        mm5,    mm3                     /* make a copy of p */
        
        psubusb     mm3,    mm4                     /* p-pu */
        psubusb     mm4,    mm5                     /* pu-p */

        por         mm3,    mm4                     /* abs(p-pu) */
        movq        mm6,    mm0                     /* 32+QValues */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128c				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64c				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
        movq2dq     xmm6,   mm6                     

        punpcklbw   xmm7,	xmm6					/* 03 xx 02 xx 01 xx 00 xx */

		psraw		xmm7,	8						/* sign extended */
		movdqa      [edi],  xmm7                    /* writeout UDmod */

		mov			esi,	Src
		mov			edi,	Des
		
		mov			eax,	UDPointer
		mov			ebx,	LRPointer

        mov         ebp,    8

FilterLoop1:        

        movq		xmm0,	QWORD PTR [esi+edx]		/* mm0 = Pixels above */
		pxor		xmm7,	xmm7				    /* clear mm7 */

		movdqa		xmm4,	[eax]			        /* au */
        punpcklbw	xmm0,	xmm7				    /* extended to shorts */
		
		movq		xmm2,	QWORD PTR [esi+ecx]		/* mm2 = pixels below */
        pmullw		xmm0,	xmm4				    /* pu*au */
		
		movdqa		xmm6,	[eax+16]		        /* ad */
        punpcklbw	xmm2,	xmm7				    /* extened to shorts*/
		
		movq		xmm1,	QWORD PTR [esi-1]		/* pixel to the left */
        pmullw		xmm2,	xmm6				    /* ad*pd */
        
        movdqa      xmm3,   [ebx]                   /* al */
        punpcklbw   xmm1,   xmm7                    /* extended to shorts */

        movq        xmm5,   QWORD PTR [esi+1]       /* pixel to the right */
        pmullw      xmm1,   xmm3                    /* al * pl */

        paddw		xmm4,	xmm6				    /* au+ad */
        punpcklbw   xmm5,   xmm7                    /* extends to shorts */
        
        movdqa      xmm6,   [ebx+128]               /* ar */
        pmullw      xmm5,   xmm6                    /* ar * pr */
        
        paddw		xmm0,	xmm2			        /* au*pu + ad*pd */
        paddw       xmm4,   xmm3                    /* au+ad+al */

        paddw       xmm0,   xmm1                    /* au*pu+ad*pd+al*pl */
        paddw       xmm4,   xmm6                    /* au+ad+al+ar */

        movq		xmm2,	QWORD PTR [esi]			/* p */
        paddw       xmm0,   xmm5                    /* au*pu+ad*pd+al*pl+ar*pr */

		
		/* xmm0 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* xmm4 ---	 au + ad + al + ar */
		
		movdqa		xmm1,	eight128s		        /* 0080 0080 0080 0080 0080 0080 0080 0080 */
        punpcklbw	xmm2,	xmm7				    /* extended to shorts */

		psubw		xmm1,	xmm4				    /* 128-(au+ad+al+ar) */		
		pmullw		xmm2,	xmm1				    /* p*(128-(au+ad+al+ar)) */
        
		add			esi,	ecx				        /* Src += Pitch */
		movdqa		xmm6,	eight64s			    /* 64, 64, 64, 64, 64, 64, 64, 64 */

		movdqa      xmm7,   xmm6                    /* 64, 64, 64, 64, 64, 64, 64, 64 */
        add			eax,	16				        /* UDPointer += 8 */

        psllw		xmm7,	8				        /* {16384, .. } */
        paddw		xmm0,	xmm2				    /* sum */

        add			edi,	ecx				        /* Des += Pitch */
        paddw		xmm0,	xmm6				    /* sum+B */

        add         ebx,    16                      /* LPointer +=8 */
		paddw		xmm0,	xmm7				    /* clamping */

		psubusw		xmm0,	xmm7				    /* clamping */
		dec         ebp

        psrlw		xmm0,	7				        /* (sum+B)>>7 */
		packuswb	xmm0,	xmm7				    /* pack to 8 bytes */		

        movq		QWORD PTR [edi+edx],	xmm0	/* write to destination */
        jnz         FilterLoop1
        

        pop         ebp

        pop         ebx
        pop         eax

        pop         edx
        pop         ecx

        pop         edi
        pop         esi
    }
#endif
}

/****************************************************************************
 * 
 *  ROUTINE       : DeRingBlockWeak_WMT
 *
 *  INPUTS        : const POSTPROC_INSTANCE *pbi : Pointer to post-processor instance.
 *                  const UINT8 *SrcPtr          : Pointer to input image.
 *                  UINT8 *DstPtr                : Pointer to output image.
 *                  const INT32 Pitch            : Image stride.
 *                  UINT32 FragQIndex            : Q-index block encoded with.
 *                  UINT32 *QuantScale           : Array of quantization scale factors.
 *                               
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Filtering a block for de-ringing purpose.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void DeringBlockWeak_WMT
( 
    const POSTPROC_INSTANCE *pbi, 
    const UINT8 *SrcPtr,
    UINT8 *DstPtr,
    const INT32 Pitch,
    UINT32 FragQIndex,
    UINT32 *QuantScale
)
{
#if defined(_WIN32_WCE)
	return;
#else

	__declspec(align(16)) short UDMod[72];
	__declspec(align(16)) short	LRMod[128];
    
	unsigned int PlaneLineStep = Pitch;
	const unsigned char *Src   = SrcPtr;
	unsigned char *Des         = DstPtr;
    
	short *UDPointer = UDMod;
	short *LRPointer = LRMod;
    
    UINT32 QStep  = QuantScale[FragQIndex];
	INT32 Sharpen = SharpenModifier[FragQIndex];
	(void) pbi;

	__asm 
	{
		push		esi
		push		edi
		
		mov			esi,	Src						/* Source Pointer */
		mov			edi,	UDPointer				/* UD modifier pointer */

		push		ecx
		push		edx

		mov			ecx,	PlaneLineStep			/* Pitch Step */
        xor         edx,    edx

		push		eax
		push		ebx

		mov			eax,	QStep					/* QValue */
		mov			ebx,	Sharpen					/* Sharpen */

		movd		mm0,	eax						/* QValue */
		movd		mm2,	ebx						/* sharpen */

        push        ebp

		punpcklbw	mm0,	mm0						/* 00 00 00 QQ */
        sub         edx,    ecx                     /* Negative Pitch */

		punpcklbw	mm2,	mm2						/* 00 00 00 SS */
        pxor        mm7,    mm7                     /* clear mm7 for unpacks */

		punpcklbw	mm0,	mm0						/* 00 00 qq qq */
		mov			eax,	LRPointer				/* Left and Right Modifier */                

		punpcklbw	mm2,	mm2						/* 00 00 ss ss */
		lea         ebx,    [esi+ecx*8]             /* Source Pointer of last row */        

		punpcklbw	mm0,	mm0						/* qq qq qq qq */
		movq        mm1,    mm0;                    /* make a copy */
		
		punpcklbw	mm2,	mm2						/* ss ss ss ss */
		paddb		mm1,	mm0						/* QValue * 2 */

        paddb       mm1,    mm0                     /* High = 3 * Qvalue */
        paddusb		mm1,	eight231c				/* clamping high to 24 */	

		paddb       mm0,    eight32c                /* 32+QValues */
		psubusb		mm1,	eight231c				/* Get the real value back */

        movq		mm3,	eight127c				/* 7f 7f 7f 7f 7f 7f 7f 7f */
        pandn       mm1,    mm3                     /* ClampHigh */

        /* mm0,mm1,mm2,mm7 are in use  */
        /* mm0---> QValue+32           */
        /* mm1---> ClampHigh		   */
		/* mm2---> Sharpen             */
		/* mm7---> Cleared for unpack  */

FillModLoop1:
        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */
        pxor        xmm7,   xmm7                    /* clear xmm7 */ 

        movq        mm4,    QWORD PTR [esi+edx]     /* Pixels on top pu */
        movq        mm5,    mm3                     /* make a copy of p */

        psubusb     mm3,    mm4                     /* p-pu */       
        psubusb     mm4,    mm5                     /* pu-p */

        por         mm3,    mm4                     /* abs(p-pu) */
        movq        mm6,    mm0                     /* 32+QValues */

        paddusb     mm3,    mm3                     /* 2*abs(p-pu) */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128c				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64c				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */

        movq2dq     xmm0,   mm6                     
        movq        mm4,    QWORD PTR [esi-1]       /* Pixels on top pu */

        punpcklbw	xmm7,	xmm0					/* extended to words */
        movq        mm5,    mm3                     /* make a copy of p */

        psraw		xmm7,	8						/* sign extended */
        psubusb     mm3,    mm4                     /* p-pu */

        movdqa      [edi],  xmm7                    /* writeout UDmod*/
        psubusb     mm4,    mm5                     /* pu-p */

        por         mm3,    mm4                     /* abs(p-pu) */
        movq        mm6,    mm0                     /* 32+QValues */

        paddusb     mm3,    mm3                     /* 2*abs(p-pu) */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128c				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64c				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */

        pxor        xmm7,   xmm7                    /* clear xmm7 */
        movq        mm4,    QWORD PTR [esi+1]       /* Pixels on top pu */

   		movq2dq 	xmm0,	mm6						
        movq        mm5,    mm3                     /* make a copy of p */

        punpcklbw   xmm7,   xmm0                    /* extened  to shorts */
        psubusb     mm3,    mm4                     /* p-pu */

		psraw		xmm7,	8						/* sign extended */
        psubusb     mm4,    mm5                     /* pu-p */

        movdqa      [eax],  xmm7                    /* writeout UDmod*/
        por         mm3,    mm4                     /* abs(p-pu) */

        movq        mm6,    mm0                     /* 32+QValues */
        paddusb     mm3,    mm3                     /* 2*abs(p-pu) */

        pxor        xmm7,   xmm7                    /* clear xmm7 */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128c				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64c				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
        add         esi,    ecx
        
        movq2dq     xmm0,   mm6
        add         edi,    16                  

        punpcklbw	xmm7,	mm0						/* extended to shorts */
        add         eax,    16      

        psraw		xmm7,	8						/* sign extended */
        cmp         esi,    ebx

        movdqa      [eax+112], xmm7                 /* writeout UDmod*/		
        jne         FillModLoop1
        
        /* last UDMod */

        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */
        pxor        xmm7,   xmm7                    /* clear xmm7 */


        movq        mm4,    QWORD PTR [esi+edx]     /* Pixels on top pu */
        movq        mm5,    mm3                     /* make a copy of p */
        
        psubusb     mm3,    mm4                     /* p-pu */
        psubusb     mm4,    mm5                     /* pu-p */

        por         mm3,    mm4                     /* abs(p-pu) */
        movq        mm6,    mm0                     /* 32+QValues */

        paddusb     mm3,    mm3                     /* 2*abs(p-pu) */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128c				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64c				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
        movq2dq     xmm6,   mm6                     

        punpcklbw   xmm7,	xmm6					/* 03 xx 02 xx 01 xx 00 xx */

		psraw		xmm7,	8						/* sign extended */
		movdqa      [edi],  xmm7                    /* writeout UDmod */

		mov			esi,	Src
		mov			edi,	Des
		
		mov			eax,	UDPointer
		mov			ebx,	LRPointer

        mov         ebp,    8

FilterLoop1:        

        movq		xmm0,	QWORD PTR [esi+edx]		/* mm0 = Pixels above */
		pxor		xmm7,	xmm7				    /* clear mm7 */

		movdqa		xmm4,	[eax]			        /* au */
        punpcklbw	xmm0,	xmm7				    /* extended to shorts */
		
		movq		xmm2,	QWORD PTR [esi+ecx]		/* mm2 = pixels below */
        pmullw		xmm0,	xmm4				    /* pu*au */
		
		movdqa		xmm6,	[eax+16]		        /* ad */
        punpcklbw	xmm2,	xmm7				    /* extened to shorts*/
		
		movq		xmm1,	QWORD PTR [esi-1]		/* pixel to the left */
        pmullw		xmm2,	xmm6				    /* ad*pd */
        
        movdqa      xmm3,   [ebx]                   /* al */
        punpcklbw   xmm1,   xmm7                    /* extended to shorts */

        movq        xmm5,   QWORD PTR [esi+1]       /* pixel to the right */
        pmullw      xmm1,   xmm3                    /* al * pl */

        paddw		xmm4,	xmm6				    /* au+ad */
        punpcklbw   xmm5,   xmm7                    /* extends to shorts */
        
        movdqa      xmm6,   [ebx+128]               /* ar */
        pmullw      xmm5,   xmm6                    /* ar * pr */
        
        paddw		xmm0,	xmm2			        /* au*pu + ad*pd */
        paddw       xmm4,   xmm3                    /* au+ad+al */

        paddw       xmm0,   xmm1                    /* au*pu+ad*pd+al*pl */
        paddw       xmm4,   xmm6                    /* au+ad+al+ar */

        movq		xmm2,	QWORD PTR [esi]			/* p */
        paddw       xmm0,   xmm5                    /* au*pu+ad*pd+al*pl+ar*pr */

		
		/* xmm0 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* xmm4 ---	 au + ad + al + ar */
		
		movdqa		xmm1,	eight128s		        /* 0080 0080 0080 0080 0080 0080 0080 0080 */
        punpcklbw	xmm2,	xmm7				    /* extended to shorts */

		psubw		xmm1,	xmm4				    /* 128-(au+ad+al+ar) */		
		pmullw		xmm2,	xmm1				    /* p*(128-(au+ad+al+ar)) */
        
		add			esi,	ecx				        /* Src += Pitch */
		movdqa		xmm6,	eight64s			    /* 64, 64, 64, 64, 64, 64, 64, 64 */

		movdqa      xmm7,   xmm6                    /* 64, 64, 64, 64, 64, 64, 64, 64 */
        add			eax,	16				        /* UDPointer += 8 */

        psllw		xmm7,	8				        /* {16384, .. } */
        paddw		xmm0,	xmm2				    /* sum */

        add			edi,	ecx				        /* Des += Pitch */
        paddw		xmm0,	xmm6				    /* sum+B */

        add         ebx,    16                      /* LPointer +=8 */
		paddw		xmm0,	xmm7				    /* clamping */

		psubusw		xmm0,	xmm7				    /* clamping */
		dec         ebp

        psrlw		xmm0,	7				        /* (sum+B)>>7 */
		packuswb	xmm0,	xmm7				    /* pack to 8 bytes */		

        movq		QWORD PTR [edi+edx],	xmm0	/* write to destination */
        jnz         FilterLoop1
        

        pop         ebp

        pop         ebx
        pop         eax

        pop         edx
        pop         ecx

        pop         edi
        pop         esi
    }
#endif
}
