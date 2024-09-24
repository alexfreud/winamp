/****************************************************************************
 *
 *   Module Title :     DeRingingOpt.c
 *
 *   Description  :     Optimized functions for PostProcessor
 *
 ***************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "postp.h"

/****************************************************************************
*  Macros
****************************************************************************/
#pragma warning(disable:4799)
#pragma warning(disable:4731)
#pragma warning(disable:4305)

/****************************************************************************
*  Module constants.
****************************************************************************/        
#if defined(_WIN32_WCE)
#pragma pack(16)
static unsigned short FourOnes[] = {  1,   1,   1,   1 };
static unsigned short Four128s[] = { 128, 128, 128, 128 };
static unsigned short Four64s[]  = { 64,  64,  64,  64};

static char eight64s [] = { 64, 64, 64, 64, 64, 64, 64, 64 };
static char eight32s [] = { 32, 32, 32, 32, 32, 32, 32, 32 };
static char eight127s []= { 127, 127, 127, 127, 127, 127, 127, 127 };
static char eight128s []= { 128, 128, 128, 128, 128, 128, 128, 128 };
static unsigned char eight223s[] = { 223, 223, 223, 223, 223, 223, 223, 223 };
static unsigned char eight231s[] = { 231, 231, 231, 231, 231, 231, 231, 231 };
#pragma pack()
#else

__declspec(align(16)) static unsigned short FourOnes[] = {  1,   1,   1,   1 };
__declspec(align(16)) static unsigned short Four128s[] = { 128, 128, 128, 128 };
__declspec(align(16)) static unsigned short Four64s[]  = { 64,  64,  64,  64};

__declspec(align(16)) static char eight64s [] = { 64, 64, 64, 64, 64, 64, 64, 64 };
__declspec(align(16)) static char eight32s [] = { 32, 32, 32, 32, 32, 32, 32, 32 };
__declspec(align(16)) static char eight127s []= { 127, 127, 127, 127, 127, 127, 127, 127 };
__declspec(align(16)) static char eight128s []= { 128, 128, 128, 128, 128, 128, 128, 128 };
__declspec(align(16)) static unsigned char eight223s[] = { 223, 223, 223, 223, 223, 223, 223, 223 };
__declspec(align(16)) static unsigned char eight231s[] = { 231, 231, 231, 231, 231, 231, 231, 231 };

#endif
/****************************************************************************
*  Imports
****************************************************************************/              
extern UINT32 SharpenModifier[];

/****************************************************************************
 * 
 *  ROUTINE       : DeRingBlockStrong_MMX
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
void DeringBlockStrong_MMX
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
	#pragma pack(16)
	short UDMod[72];
	short	LRMod[128];
	#pragma pack()
#else
	__declspec(align(16)) short UDMod[72];
	__declspec(align(16)) short	LRMod[128];
#endif
	unsigned int PlaneLineStep = Pitch;
	const unsigned char *Src = SrcPtr;
	unsigned char *Des       = DstPtr;
    
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
        paddusb		mm1,	eight223s				/* clamping high to 32 */	

		paddb       mm0,    eight32s                /* 32+QValues */
		psubusb		mm1,	eight223s				/* Get the real value back */

        movq		mm3,	eight127s				/* 7f 7f 7f 7f 7f 7f 7f 7f */
        pandn       mm1,    mm3                     /* ClampHigh */

        /* mm0,mm1,mm2,mm7 are in use  */
        /* mm0---> QValue+32           */
        /* mm1---> ClampHigh		   */
		/* mm2---> Sharpen             */
		/* mm7---> Cleared for unpack  */

FillModLoop1:
        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */
        movq        mm4,    QWORD PTR [esi+edx]     /* Pixels on top pu */

        movq        mm5,    mm3                     /* make a copy of p */
        psubusb     mm3,    mm4                     /* p-pu */
        
        psubusb     mm4,    mm5                     /* pu-p */
        por         mm3,    mm4                     /* abs(p-pu) */

        movq        mm6,    mm0                     /* 32+QValues */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128s				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64s				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
		pxor		mm5,	mm5						/* clear mm5 */

		pxor		mm4,	mm4						/* clear mm4 */
   		punpcklbw	mm5,	mm6						/* 03 xx 02 xx 01 xx 00 xx */

		psraw		mm5,	8						/* sign extended */
		movq        QWORD PTR [edi], mm5            /* writeout UDmod, low four */
		
		punpckhbw	mm4,	mm6
		psraw		mm4,	8

        movq        QWORD PTR [edi+8], mm4          /* writeout UDmod, high four */

        
        /* left Mod */
        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */
        movq        mm4,    QWORD PTR [esi-1]     /* Pixels on top pu */

        movq        mm5,    mm3                     /* make a copy of p */
        psubusb     mm3,    mm4                     /* p-pu */
        
        psubusb     mm4,    mm5                     /* pu-p */
        por         mm3,    mm4                     /* abs(p-pu) */

        movq        mm6,    mm0                     /* 32+QValues */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128s				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64s				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
		pxor		mm5,	mm5						/* clear mm5 */

		pxor		mm4,	mm4						/* clear mm4 */
   		punpcklbw	mm5,	mm6						/* 03 xx 02 xx 01 xx 00 xx */

		psraw		mm5,	8						/* sign extended */
		movq        QWORD PTR [eax], mm5            /* writeout UDmod, low four */
		
		punpckhbw	mm4,	mm6
		psraw		mm4,	8

        movq        QWORD PTR [eax+8], mm4          /* writeout UDmod, high four */



        /* Right Mod */
        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */
        movq        mm4,    QWORD PTR [esi+1]       /* Pixels on top pu */

        movq        mm5,    mm3                     /* make a copy of p */
        psubusb     mm3,    mm4                     /* p-pu */
        
        psubusb     mm4,    mm5                     /* pu-p */
        por         mm3,    mm4                     /* abs(p-pu) */

        movq        mm6,    mm0                     /* 32+QValues */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128s				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64s				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
		pxor		mm5,	mm5						/* clear mm5 */

		pxor		mm4,	mm4						/* clear mm4 */
   		punpcklbw	mm5,	mm6						/* 03 xx 02 xx 01 xx 00 xx */

		psraw		mm5,	8						/* sign extended */
		movq        QWORD PTR [eax+128], mm5            /* writeout UDmod, low four */
		
		punpckhbw	mm4,	mm6
		psraw		mm4,	8

        movq        QWORD PTR [eax+136], mm4          /* writeout UDmod, high four */
        add         esi,    ecx
        
        
        add         edi,    16                  
        add         eax,    16      

        cmp         esi,    ebx
        jne         FillModLoop1
        
        /* last UDMod */

        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */
        movq        mm4,    QWORD PTR [esi+edx]     /* Pixels on top pu */

        movq        mm5,    mm3                     /* make a copy of p */
        psubusb     mm3,    mm4                     /* p-pu */
        
        psubusb     mm4,    mm5                     /* pu-p */
        por         mm3,    mm4                     /* abs(p-pu) */

        movq        mm6,    mm0                     /* 32+QValues */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128s				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64s				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
		pxor		mm5,	mm5						/* clear mm5 */

		pxor		mm4,	mm4						/* clear mm4 */
   		punpcklbw	mm5,	mm6						/* 03 xx 02 xx 01 xx 00 xx */

		psraw		mm5,	8						/* sign extended */
		movq        QWORD PTR [edi], mm5            /* writeout UDmod, low four */
		
		punpckhbw	mm4,	mm6
		psraw		mm4,	8

        movq        QWORD PTR [edi+8], mm4          /* writeout UDmod, high four */

		mov			esi,	Src
		mov			edi,	Des
		
		mov			eax,	UDPointer
		mov			ebx,	LRPointer

		/* First Row */
		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		

		/* Second Row */
		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		

        /* Third Row */
		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		



        /* Fourth Row */
		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		

        /* Fifth Row */

		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		

        /* Sixth Row */

		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		

        /* Seventh Row */

		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		
        /* Eighth Row */

		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

        pop         ebx
        pop         eax

        pop         edx
        pop         ecx

        pop         edi
        pop         esi
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : DeRingBlockWeak_MMX
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
 *  FUNCTION      : Filters a block for de-ringing purpose.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void DeringBlockWeak_MMX
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
	#pragma pack(16)
	short UDMod[72];
	short	LRMod[128];
	#pragma pack()
#else
	__declspec(align(16)) short UDMod[72];
	__declspec(align(16)) short	LRMod[128];
#endif
    
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
        paddusb		mm1,	eight231s				/* clamping high to 24 */	

		paddb       mm0,    eight32s                /* 32+QValues */
		psubusb		mm1,	eight231s				/* Get the real value back */

        movq		mm3,	eight127s				/* 7f 7f 7f 7f 7f 7f 7f 7f */
        pandn       mm1,    mm3                     /* ClampHigh */

        /* mm0,mm1,mm2,mm7 are in use  */
        /* mm0---> QValue+32           */
        /* mm1---> ClampHigh		   */
		/* mm2---> Sharpen             */
		/* mm7---> Cleared for unpack  */

FillModLoop1:
        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */
        movq        mm4,    QWORD PTR [esi+edx]     /* Pixels on top pu */

        movq        mm5,    mm3                     /* make a copy of p */
        psubusb     mm3,    mm4                     /* p-pu */
        
        psubusb     mm4,    mm5                     /* pu-p */
        por         mm3,    mm4                     /* abs(p-pu) */

        movq        mm6,    mm0                     /* 32+QValues */
		paddusb		mm3,	mm3						/* 2*abs(p-pu) */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128s				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64s				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
		pxor		mm5,	mm5						/* clear mm5 */

		pxor		mm4,	mm4						/* clear mm4 */
   		punpcklbw	mm5,	mm6						/* 03 xx 02 xx 01 xx 00 xx */

		psraw		mm5,	8						/* sign extended */
		movq        QWORD PTR [edi], mm5            /* writeout UDmod, low four */
		
		punpckhbw	mm4,	mm6
		psraw		mm4,	8

        movq        QWORD PTR [edi+8], mm4          /* writeout UDmod, high four */

        
        /* left Mod */
        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */
        movq        mm4,    QWORD PTR [esi-1]     /* Pixels on top pu */

        movq        mm5,    mm3                     /* make a copy of p */
        psubusb     mm3,    mm4                     /* p-pu */
        
        psubusb     mm4,    mm5                     /* pu-p */
        por         mm3,    mm4                     /* abs(p-pu) */

        movq        mm6,    mm0                     /* 32+QValues */
		paddusb		mm3,	mm3						/* 2*abs(p-pu) */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128s				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64s				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
		pxor		mm5,	mm5						/* clear mm5 */

		pxor		mm4,	mm4						/* clear mm4 */
   		punpcklbw	mm5,	mm6						/* 03 xx 02 xx 01 xx 00 xx */

		psraw		mm5,	8						/* sign extended */
		movq        QWORD PTR [eax], mm5            /* writeout UDmod, low four */
		
		punpckhbw	mm4,	mm6
		psraw		mm4,	8

        movq        QWORD PTR [eax+8], mm4          /* writeout UDmod, high four */



        /* Right Mod */
        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */
        movq        mm4,    QWORD PTR [esi+1]       /* Pixels on top pu */

        movq        mm5,    mm3                     /* make a copy of p */
        psubusb     mm3,    mm4                     /* p-pu */
        
        psubusb     mm4,    mm5                     /* pu-p */
        por         mm3,    mm4                     /* abs(p-pu) */

        movq        mm6,    mm0                     /* 32+QValues */
		paddusb		mm3,	mm3						/* 2*abs(p-pu) */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128s				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64s				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
		pxor		mm5,	mm5						/* clear mm5 */

		pxor		mm4,	mm4						/* clear mm4 */
   		punpcklbw	mm5,	mm6						/* 03 xx 02 xx 01 xx 00 xx */

		psraw		mm5,	8						/* sign extended */
		movq        QWORD PTR [eax+128], mm5            /* writeout UDmod, low four */
		
		punpckhbw	mm4,	mm6
		psraw		mm4,	8

        movq        QWORD PTR [eax+136], mm4          /* writeout UDmod, high four */
        add         esi,    ecx
        
        
        add         edi,    16                  
        add         eax,    16      

        cmp         esi,    ebx
        jne         FillModLoop1
        
        /* last UDMod */

        movq        mm3,    QWORD PTR [esi]         /* read 8 pixels p  */
        movq        mm4,    QWORD PTR [esi+edx]     /* Pixels on top pu */

        movq        mm5,    mm3                     /* make a copy of p */
        psubusb     mm3,    mm4                     /* p-pu */
        
        psubusb     mm4,    mm5                     /* pu-p */
        por         mm3,    mm4                     /* abs(p-pu) */

        movq        mm6,    mm0                     /* 32+QValues */
		paddusb		mm3,	mm3						/* 2*abs(p-pu) */

        movq		mm4,	mm0						/* 32+QValues */
		psubusb		mm6,    mm3                     /* zero clampled TmpMod */

		movq		mm5,	eight128s				/* 80 80 80 80 80 80 80 80 */
		paddb		mm4,	eight64s				/* 32+QValues + 64 */

		pxor		mm4,	mm5						/* convert to a sign number */
		pxor		mm3,	mm5						/* convert to a sign number */

		pcmpgtb		mm3,	mm4						/* 32+QValue- 2*abs(p-pu) <-64 ? */
		pand		mm3,	mm2						/* use sharpen */

        paddsb		mm6,    mm1						/* clamping to high */
		psubsb		mm6,	mm1						/* offset back */

		por			mm6,	mm3						/* Mod value to be stored */
		pxor		mm5,	mm5						/* clear mm5 */

		pxor		mm4,	mm4						/* clear mm4 */
   		punpcklbw	mm5,	mm6						/* 03 xx 02 xx 01 xx 00 xx */

		psraw		mm5,	8						/* sign extended */
		movq        QWORD PTR [edi], mm5            /* writeout UDmod, low four */
		
		punpckhbw	mm4,	mm6
		psraw		mm4,	8

        movq        QWORD PTR [edi+8], mm4          /* writeout UDmod, high four */

		mov			esi,	Src
		mov			edi,	Des
		
		mov			eax,	UDPointer
		mov			ebx,	LRPointer

		/* First Row */
		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		

		/* Second Row */
		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		

        /* Third Row */
		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		



        /* Fourth Row */
		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		

        /* Fifth Row */

		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		

        /* Sixth Row */

		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		

        /* Seventh Row */

		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

		add			esi,	ecx				/* Src += Pitch */
		add			edi,	ecx				/* Des += Pitch */

		add			eax,	16				/* UDPointer += 8 */
        add         ebx,    16              /* LPointer +=8 */
		
        /* Eighth Row */

		movq		mm0,	[esi+edx]		/* mm0 = Pixels above */
		pxor		mm7,	mm7				/* clear mm7 */

		movq		mm1,	mm0				/* make a copy of mm0 */			
		punpcklbw	mm0,	mm7				/* lower four pixels */
		
		movq		mm4,	[eax]			/* au */
		punpckhbw	mm1,	mm7				/* high four pixels */
		
		movq		mm5,	[eax+8]			/* au */
		
		pmullw		mm0,	mm4				/* pu*au */
		movq		mm2,	[esi+ecx]		/* mm2 = pixels below */
		
		pmullw		mm1,	mm5				/* pu*au */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* lower four */
		movq		mm6,	[eax+16]		/* ad */

		punpckhbw	mm3,	mm7				/* higher four */			
		paddw		mm4,	mm6				/* au+ad */
		
		pmullw		mm2,	mm6				/* au*pu+ad*pd */
		movq		mm6,	[eax+24]		/* ad */

		paddw		mm0,	mm2			
		paddw		mm5,	mm6				/* au+ad */
		
		pmullw		mm3,	mm6				/* ad*pd */
		movq		mm2,	[esi-1]			/* pixel to the left */

		paddw		mm1,	mm3				/* au*pu+ad*pd */
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx]			/* al */

		punpckhbw	mm3,	mm7				/* four right pixels */
		paddw		mm4,	mm6				/* au + ad + al */
		
		pmullw		mm2,	mm6				/* pl * al */
		movq		mm6,	[ebx+8]			/* al */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl */
		paddw		mm5,	mm6				/* au+ad+al */
		
		pmullw		mm3,	mm6				/* al*pl */
		movq		mm2,	[esi+1]			/* pixel to the right */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl */			
		movq		mm3,	mm2				/* make a copy of mm2 */
		
		punpcklbw	mm2,	mm7				/* four left pixels */
		movq		mm6,	[ebx+128]			/* ar */

		punpckhbw	mm3,	mm7				/* four right pixels */			
		paddw		mm4,	mm6				/* au + ad + al + ar */
		
		pmullw		mm2,	mm6				/* pr * ar */
		movq		mm6,	[ebx+136]		/* ar */

		paddw		mm0,	mm2				/* au*pu+ad*pd+al*pl+pr*ar */
		paddw		mm5,	mm6				/* au+ad+al+ar */
		
		pmullw		mm3,	mm6				/* ar*pr */
		movq		mm2,	[esi]			/* p */

		paddw		mm1,	mm3				/* au*pu+ad*pd+al*pl+ar*pr */
		movq		mm3,	mm2				/* make a copy of the pixel */
		
		/* mm0, mm1 ---  au*pu+ad*pd+al*pl+ar*pr */
		/* mm4, mm5	---	 au + ad + al + ar */
		
		punpcklbw	mm2,	mm7				/* left four pixels */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		punpckhbw	mm3,	mm7				/* right four pixels */
		psubw		mm6,	mm4				/* 128-(au+ad+al+ar) */
		
		pmullw		mm2,	mm6				/* p*(128-(au+ad+al+ar)) */
		movq		mm6,	Four128s		/* 0080  0080 0080 0080 */

		paddw		mm0,	mm2				/* sum */
		psubw		mm6,	mm5				/* 128-(au+ad+al+ar) */
		
		pmullw		mm3,	mm6				/* p*(128-(au+ad+al+ar)) */ 
		movq		mm6,	Four64s			/* {64, 64, 64, 64 } */

		movq		mm7,	mm6				/* {64, 64, 64, 64} */
		paddw		mm0,	mm6				/* sum+B */

		paddw		mm1,	mm3				/* sum */
		psllw		mm7,	8				/* {16384, .. } */

		paddw		mm0,	mm7				/* clamping */
		paddw		mm1,	mm6				/* sum+B */

		paddw		mm1,	mm7				/* clamping */
		psubusw		mm0,	mm7				/* clamping */

		psubusw		mm1,	mm7				/* clamping */
		psrlw		mm0,	7				/* (sum+B)>>7 */

		psrlw		mm1,	7				/* (sum+B)>>7 */
		packuswb	mm0,	mm1				/* pack to 8 bytes */
		
		movq		[edi],	mm0				/* write to destination */

        pop         ebx
        pop         eax

        pop         edx
        pop         ecx

        pop         edi
        pop         esi
    }
}
