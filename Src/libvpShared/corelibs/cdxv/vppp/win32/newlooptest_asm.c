/****************************************************************************
 *
 *   Module Title :     newlooptest_asm.c 
 *
 *   Description  :     Codec specific functions
 *
 ***************************************************************************/ 
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include <math.h>
#include "postp.h"

/****************************************************************************
*  Macros
****************************************************************************/        
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

/****************************************************************************
*  Imports
****************************************************************************/ 
extern UINT32 *LoopFilterLimitValuesV2;             

/****************************************************************************
*  Exports
****************************************************************************/ 
INT16 LoopFilterLimitValuesV2_MMX[64*4];             

/****************************************************************************
 * 
 *  ROUTINE       : FillLoopFilterLimitValues_MMX
 *
 *  INPUTS        : None.
 *                  
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Sets-up array of limit values for use in loop-filter.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/                       
void FillLoopFilterLimitValues_MMX ( void )
{
	int i;

    for ( i=0; i<64; i++ )
	{
		LoopFilterLimitValuesV2_MMX[i*4+0] = LoopFilterLimitValuesV2[i];
		LoopFilterLimitValuesV2_MMX[i*4+1] = LoopFilterLimitValuesV2[i];
		LoopFilterLimitValuesV2_MMX[i*4+2] = LoopFilterLimitValuesV2[i];
		LoopFilterLimitValuesV2_MMX[i*4+3] = LoopFilterLimitValuesV2[i];
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : FilteringHoriz
 *
 *  INPUTS        : UINT32 QIndex : Quantization index.
 *                  UINT8 *Src    : Pointer to source block.
 *                  INT32 Pitch   : Pitch of input image.
 *                  
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Filters the vertical block edge inside a prediction
 *                  block.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/                       
void FilteringHoriz_8_MMX ( UINT32 QIndex, UINT8 *Src, INT32 Pitch ) 
{    
    INT16 *FLimitPtr = &LoopFilterLimitValuesV2_MMX[QIndex*4];
    
    __declspec(align(16)) const short fourFours[] = {4, 4, 4, 4};
    __declspec(align(16)) const short fourOnes[] = { 1, 1, 1, 1};
    __declspec(align(16)) unsigned char Temp[32];

    __asm 
    {
        mov         eax,    FLimitPtr
        mov         edx,    Pitch
        
        mov         esi,    Src
        lea         edi,    Temp

        mov         ecx,    edx                     //stride
        movd        mm0,    [esi + -4]              //xx xx xx xx 01 00 xx xx

        movd        mm4,    [esi]                   //xx xx xx xx xx xx 03 02
        psrld       mm0,    16                      //xx xx xx xx 00 00 01 00

        movd        mm1,    [esi + ecx + -4]        //xx xx xx xx 11 10 xx xx
        punpcklwd   mm0,    mm4                     //xx xx xx xx 03 02 01 00

        movd        mm4,    [esi + ecx]             //xx xx xx xx xx xx 13 12
        psrld       mm1,    16                      //xx xx xx xx 00 00 11 10

        punpcklwd   mm1,    mm4                     //xx xx xx xx 13 12 11 10
        lea         edx,    [edx + edx*2]           //stride * 3

        movd        mm2,    [esi + ecx*2 + -4]      //xx xx xx xx 21 20 xx xx
        punpcklbw   mm0,    mm1                     //13 03 12 02 11 01 10 00

        movd        mm4,    [esi + ecx*2]           //xx xx xx xx xx xx 23 22
        psrld       mm2,    16                      //xx xx xx xx 00 00 21 20

        movd        mm1,    [esi + edx + -4]        //xx xx xx xx 31 30 xx xx
        punpcklwd   mm2,    mm4                     //xx xx xx xx 23 22 21 20

        movd        mm4,    [esi + edx]             //xx xx xx xx xx xx 33 32
        psrld       mm1,    16                      //xx xx xx xx 00 00 31 30

        punpcklwd   mm1,    mm4                     //xx xx xx xx 33 32 31 30
        pxor        mm4,    mm4                     // clear mm4

        punpcklbw   mm2,    mm1                     //33 23 32 22 31 21 30 20
        movq        mm1,    mm0                     //13 03 12 03 11 01 10 00

        punpcklwd   mm0,    mm2                     //31 21 11 01 30 20 10 00
        lea         esi,    [esi + ecx*4]           //base + (stride * 4)

        punpckhwd   mm1,    mm2                     //33 23 13 03 32 22 12 02
        movq        mm6,    mm0                     //xx xx xx xx 30 20 10 00

        movq        [edi],  mm0                     // save to memory
        movq        mm2,    mm1                     // make a copy 

        movq        [edi+8],  mm1                   // save to memory
        psrlq       mm0,    32                      //xx xx xx xx 31 21 11 01

        movd        mm7,    [esi + -4]              //xx xx xx xx 41 40 xx xx
        punpcklbw   mm1,    mm4                     //xx 32 xx 22 xx 12 xx 02

        movd        mm4,    [esi]                   //xx xx xx xx xx xx 43 42
        psrld       mm7,    16                      //xx xx xx xx 00 00 41 40

        movd        mm5,    [esi + ecx + -4]        //xx xx xx xx 51 50 xx xx
        punpcklwd   mm7,    mm4                     //xx xx xx xx 43 42 41 40

        movd        mm4,    [esi + ecx]             //xx xx xx xx xx xx 53 52
        psrld       mm5,    16                      //xx xx xx xx xx xx 51 50

        punpcklwd   mm5,    mm4                     //xx xx xx xx 53 52 51 50
        pxor        mm4,    mm4                     // clear mm4

        punpcklbw   mm0,    mm4                     //xx 31 xx 21 xx 11 xx 01

        psrlq       mm2,    32                      //xx xx xx xx 33 23 13 03
        psubw       mm1,    mm0                     //x = p[0] - p[ms]

        punpcklbw   mm7,    mm5                     //53 43 52 42 51 41 50 40
        movq        mm3,    mm1                     // make a copy of x

        punpcklbw   mm6,    mm4                     //xx 30 xx 20 xx 10 xx 00    
        paddw       mm3,    mm1                     //x = 2*(p[0] - p[ms])

        punpcklbw   mm2,    mm4                     //xx 33 xx 23 xx 13 xx 03
        paddw       mm1,    mm3                     //mm1 = 3*(p[0] - p[-1])

        paddw       mm1,    fourFours               //mm1 += LoopFilterAdjustBeforeShift
        psubw       mm6,    mm2                     //mm6 = (p[-2]-p[1])

        movd        mm2,    [esi + ecx*2 + -4]      //xx xx xx xx 61 60 xx xx
        paddw       mm6,    mm1                     //mm6 = 3*(p[0] - p[-1]) +(p[-2]-p[1]) + 4

        movd        mm4,    [esi + ecx*2]           //xx xx xx xx xx xx 63 62
        psrld       mm2,    16                      //xx xx xx xx xx xx 61 60

        movd        mm5,    [esi + edx + -4]        //xx xx xx xx 71 70 xx xx
        punpcklwd   mm2,    mm4                     //xx xx xx xx 63 62 61 60

        movd        mm4,    [esi + edx]             //xx xx xx xx xx xx 73 72
        psrld       mm5,    16                      //xx xx xx xx 00 00 71 70

        mov         esi,    Src                     //restore PixelPtr
        punpcklwd   mm5,    mm4                     //xx xx xx xx 73 72 71 70

        psraw       mm6,    3                       //values to be clipped
        pxor        mm4,    mm4                     // clear mm4

        punpcklbw   mm2,    mm5                     //73 63 72 62 71 61 70 60
        movq        mm5,    mm7                     //53 43 52 42 51 41 50 40

        movq        mm1,    mm6                     // make a copy of results
        punpckhwd   mm5,    mm2                     //73 63 53 43 72 62 52 42


        movq        [edi+24],  mm5                  //save for later
        punpcklwd   mm7,    mm2                     //71 61 51 41 70 60 50 40

        movq        [edi+16],  mm7                  //save for later
        psraw       mm6,    15                      // FFFF or 0000 

        movq        mm2,    [eax]                   //get the limit value
        movq        mm0,    mm7                     //xx xx xx xx 70 60 50 41

        psrlq       mm7,    32                      //xx xx xx xx 71 61 51 41
        pxor        mm1,    mm6

        psubsw      mm1,    mm6                     //abs(i)
        punpcklbw   mm5,    mm4

        por         mm6,    fourOnes                //now have -1 or 1 
        movq        mm3,    mm2

        punpcklbw   mm7,    mm4
        psubw       mm3,    mm1                     //limit - abs(i)

        movq        mm4,    mm3
        psraw       mm3,    15

        psubw       mm5,    mm7                     //x = p[0] - p[ms]
        pxor        mm4,    mm3

        psubsw      mm4,    mm3                     //abs(limit - abs(i))
        pxor        mm3,    mm3

        movd        mm1,    [edi + 28]              //xx xx xx xx 73 63 53 43
        psubusw     mm2,    mm4                     //limit - abs(limit - abs(i))

        punpcklbw   mm0,    mm3
        movq        mm7,    mm5

        paddw       mm7,    mm5
        pmullw      mm2,    mm6                     //new y -- wait 3 cycles

        punpcklbw   mm1,    mm3
        paddw       mm5,    mm7

        paddw       mm5,    fourFours               //x += LoopFilterAdjustBeforeShift
        psubw       mm0,    mm1

        paddw       mm0,    mm5
        pxor        mm6,    mm6     

        movd        mm7,    [edi + 8]               //xx xx xx xx 32 22 12 02
        psraw       mm0,    3                       //values to be clipped

        movd        mm3,    [edi + 4]               //xx xx xx xx 31 21 11 01
        punpcklbw   mm7,    mm6

        psubw       mm7,    mm2                     //p[ms] + y
        punpcklbw   mm3,    mm6

        paddw       mm3,    mm2                     //p[0] - y
        packuswb    mm7,    mm7                     //clamp[ p[ms] + y]

        packuswb    mm3,    mm3                     //clamp[ p[0] - y]
        movq        mm1,    mm0

        movq        mm2,    [eax]                   //get the limit value
        psraw       mm0,    15

        punpcklbw   mm3,    mm7                     //32 31 22 21 12 11 02 01                    
        movq        mm7,    mm0                     //save sign

        movd        eax,    mm3                     //12 11 02 01
        pxor        mm1,    mm0


        mov         WORD PTR [esi - 1],ax           //02 01
        psubsw      mm1,    mm0                     //abs(i)

        shr         eax,    16
        movq        mm5,    mm2

        mov         WORD PTR [esi + ecx - 1],ax
        psrlq       mm3,    32                      //xx xx xx xx 32 31 22 21

        por         mm7,    fourOnes                //now have -1 or 1 
        psubw       mm5,    mm1                     //limit - abs(i)

        movd        eax,    mm3                     //32 31 22 21
        movq        mm4,    mm5

        mov         [esi + ecx*2 - 1],ax
        psraw       mm5,    15

        shr         eax,    16
        pxor        mm4,    mm5

        mov         [esi + edx - 1],ax
        psubsw      mm4,    mm5                     //abs(limit - abs(i))

        movd        mm5,    [edi + 24]              //xx xx xx xx 72 62 52 42
        psubusw     mm2,    mm4                     //limit - abs(limit - abs(i))

        pmullw      mm2,    mm7                     //new y
        pxor        mm6,    mm6

        movd        mm3,    [edi + 20]              //xx xx xx xx 71 61 51 41
        punpcklbw   mm5,    mm6

        lea         esi,    [esi + ecx*4]
        punpcklbw   mm3,    mm6

        paddw       mm3,    mm2                     //p[ms] + y
        psubw       mm5,    mm2                     //p[0] - y

        packuswb    mm3,    mm3                     //clamp[ p[ms] + y]


        packuswb    mm5,    mm5                     //clamp[ p[0] - y]
        punpcklbw   mm3,    mm5                     //72 71 62 61 52 51 42 41

        movd        eax,    mm3                     //52 51 42 41
        psrlq       mm3,    32                      //xx xx xx xx 72 71 62 61

        mov         [esi - 1],ax
        shr         eax,    16

        mov         [esi + ecx - 1],ax
        movd        eax,    mm3

        mov         [esi + ecx*2 - 1],ax
        shr         eax,16

        mov         [esi + edx - 1],ax
    
    } 
/*
    INT32 j;
	INT32 FiltVal;
    UINT8 *LimitTable = &LimitVal_VP31[VAL_RANGE];
    UINT32 FLimit;

    FLimit = LoopFilterLimitValuesV2[QValue];

	for ( j=0; j<Length; j++ )
	{            
        // set up blur kernel for differences
		FiltVal =  (( Src[-2]     ) - 
			        ( Src[-1] * 3 ) +
			        ( Src[ 0] * 3 ) - 
			        ( Src[ 1]     ) + 4 ) >> 3;

        FiltVal = Bound ( FLimit, FiltVal );

		Dest[-1] = LimitTable[(INT32)Src[-1] + FiltVal];
		Dest[ 0] = LimitTable[(INT32)Src[ 0] - FiltVal];
		
        Src  += SrcPitch;
        Dest += DestPitch;
	}
*/
}

/****************************************************************************
 * 
 *  ROUTINE       : FilteringVert_8_MMX
 *
 *  INPUTS        : UINT32 QIndex   : Quantization index.
 *                  UINT8 *PixelPtr : Pointer to source block.
 *                  INT32 Pitch     : Pitch of input image.
 *                  
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Filters the horizontal block edge inside a prediction
 *                  block.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/                       
void FilteringVert_8_MMX ( UINT32 QIndex, UINT8 *PixelPtr, INT32 Pitch ) 
{    
    INT16 *FLimitPtr = &LoopFilterLimitValuesV2_MMX[QIndex*4];
    
    __declspec(align(16)) const short fourFours[] = { 4, 4, 4, 4 };
    __declspec(align(16)) const short fourOnes[]  = { 1, 1, 1, 1 };

    __asm
    {  
        mov         eax,    FLimitPtr               // 4 FLimit Values in shorts
        mov         edx,    Pitch                   // Pitch
   
        xor         ecx,    ecx                     // clear ecx to get negative Pitch
        sub         ecx,    edx                     // Negative Pitch

        mov         esi,    PixelPtr                // Src and Dest pointer
        movd        mm0,    [esi]                   // p[0], four pixels

        pxor        mm7,    mm7                     // clear mm7
        movd        mm1,    [esi+ecx]               // p[-1], four pixels

        punpcklbw   mm0,    mm7                     // unpack to short
        movd        mm2,    [esi+edx]               // p[1], four pixels

        punpcklbw   mm1,    mm7                     // unpack p[-1] to shorts
        movd        mm3,    [esi+ecx*2]             // p[-2], four pixels

        movq        mm5,    mm0                     // copy of unpacked p[0]
        movq        mm6,    mm1                     // copy of unpacked p[-1]

        psubw       mm0,    mm1                     // p[0] - p[-1]
        punpcklbw   mm2,    mm7                     // unpack p[1]

        movq        mm1,    mm0                     // make a copy of p[0]-p[-1]
        punpcklbw   mm3,    mm7                     // unpack p[-2]

        paddw       mm0,    mm1                     // (p[0]-p[-1]) * 2
        psubw       mm3,    mm2                     // (p[-2]-p[1])

        paddw       mm1,    mm0                     // (p[0]-p[-1]) * 3
        paddw       mm3,    mm1                     // p[-2]-3*p[-1]+3*p[0]-p[1]

        paddw       mm3,    fourFours               // p[-2]-3*p[-1]+3*p[0]-p[1]+4
        movq        mm0,    [eax]                   // FLimit

        psraw       mm3,    3                       // FiltVal
        movq        mm1,    mm3                     // FiltVal

        psraw       mm3,    15                      // FFFF-> Neg, 0->Pos
        pxor        mm1,    mm3                     //  

        psubsw      mm1,    mm3                     // abs(FiltVal)
        por         mm3,    fourOnes                // -1 or 1, corresponding the sign

        movq        mm2,    mm0                     // Copy of FLimit
        psubw       mm0,    mm1                     // FLimit - abs(FiltVal)

        movq        mm4,    mm0                     // copy FLimit-abs(FiltVal)
        psraw       mm0,    15                      // FFFF->Neg, 0->Pos

        pxor        mm4,    mm0                     //
        psubsw      mm4,    mm0                     // abs(FLimit-abs(FiltVal))

        psubusw     mm2,    mm4                     // FLimit-abs(FLimit-abs(FiltVal))
        pmullw      mm2,    mm3                     // Get the sign back

        psubw       mm5,    mm2                     // p[0] - FiltVal
        paddw       mm6,    mm2                     // p[-1] + FiltVal

        packuswb    mm5,    mm5                     // clamping
        packuswb    mm6,    mm6                     // clamping
        
        movd        [esi],  mm5                     // write p[0]
        movd        [esi+ecx], mm6                  // write p[-1]

        movd        mm0,    [esi+4]                   // p[0], four pixels
        movd        mm1,    [esi+ecx+4]               // p[-1], four pixels

        punpcklbw   mm0,    mm7                     // unpack to short
        movd        mm2,    [esi+edx+4]               // p[1], four pixels

        punpcklbw   mm1,    mm7                     // unpack p[-1] to shorts
        movd        mm3,    [esi+ecx*2+4]             // p[-2], four pixels

        movq        mm5,    mm0                     // copy of unpacked p[0]
        movq        mm6,    mm1                     // copy of unpacked p[-1]

        psubw       mm0,    mm1                     // p[0] - p[-1]
        punpcklbw   mm2,    mm7                     // unpack p[1]

        movq        mm1,    mm0                     // make a copy of p[0]-p[-1]
        punpcklbw   mm3,    mm7                     // unpack p[-2]

        paddw       mm0,    mm1                     // (p[0]-p[-1]) * 2
        psubw       mm3,    mm2                     // (p[-2]-p[1])

        paddw       mm1,    mm0                     // (p[0]-p[-1]) * 3
        paddw       mm3,    mm1                     // p[-2]-3*p[-1]+3*p[0]-p[1]

        paddw       mm3,    fourFours               // p[-2]-3*p[-1]+3*p[0]-p[1]+4
        movq        mm0,    [eax]                   // FLimit

        psraw       mm3,    3                       // FiltVal
        movq        mm1,    mm3                     // FiltVal

        psraw       mm3,    15                      // FFFF-> Neg, 0->Pos
        pxor        mm1,    mm3                     //  

        psubsw      mm1,    mm3                     // abs(FiltVal)
        por         mm3,    fourOnes                // -1 or 1, corresponding the sign

        movq        mm2,    mm0                     // Copy of FLimit
        psubw       mm0,    mm1                     // FLimit - abs(FiltVal)

        movq        mm4,    mm0                     // copy FLimit-abs(FiltVal)
        psraw       mm0,    15                      // FFFF->Neg, 0->Pos

        pxor        mm4,    mm0                     //
        psubsw      mm4,    mm0                     // abs(FLimit-abs(FiltVal))

        psubusw     mm2,    mm4                     // FLimit-abs(FLimit-abs(FiltVal))
        pmullw      mm2,    mm3                     // Get the sign back

        psubw       mm5,    mm2                     // p[0] - FiltVal
        paddw       mm6,    mm2                     // p[-1] + FiltVal

        packuswb    mm5,    mm5                     // clamping
        packuswb    mm6,    mm6                     // clamping
        
        movd        [esi+4],  mm5                   // write p[0]
        movd        [esi+ecx+4], mm6                // write p[-1]

    }
        
}

/****************************************************************************
 * 
 *  ROUTINE       : FilteringHoriz
 *
 *  INPUTS        : UINT32 QIndex : Quantization index.
 *                  UINT8 *Src    : Pointer to source block.
 *                  INT32 Pitch   : Pitch of input image.
 *                  
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Filters the vertical block edge inside a prediction
 *                  block.
 *
 *  SPECIAL NOTES : None.
 *
 *
 ****************************************************************************/                       
void FilteringHoriz_12_MMX ( UINT32 QIndex, UINT8 *Src, INT32 Pitch ) 
{    
    INT16 *FLimitPtr = &LoopFilterLimitValuesV2_MMX[QIndex*4];

    __declspec(align(16)) const short fourFours[] = {4, 4, 4, 4};
    __declspec(align(16)) const short fourOnes[] = { 1, 1, 1, 1};
    __declspec(align(16)) unsigned char Temp[32];

    __asm 
    {
        mov         eax,    FLimitPtr
        mov         edx,    Pitch
        
        mov         esi,    Src
        lea         edi,    Temp

        mov         ecx,    edx                     //stride
        movd        mm0,    [esi + -4]              //xx xx xx xx 01 00 xx xx

        movd        mm4,    [esi]                   //xx xx xx xx xx xx 03 02
        psrld       mm0,    16                      //xx xx xx xx 00 00 01 00

        movd        mm1,    [esi + ecx + -4]        //xx xx xx xx 11 10 xx xx
        punpcklwd   mm0,    mm4                     //xx xx xx xx 03 02 01 00

        movd        mm4,    [esi + ecx]             //xx xx xx xx xx xx 13 12
        psrld       mm1,    16                      //xx xx xx xx 00 00 11 10

        punpcklwd   mm1,    mm4                     //xx xx xx xx 13 12 11 10
        lea         edx,    [edx + edx*2]           //stride * 3

        movd        mm2,    [esi + ecx*2 + -4]      //xx xx xx xx 21 20 xx xx
        punpcklbw   mm0,    mm1                     //13 03 12 02 11 01 10 00

        movd        mm4,    [esi + ecx*2]           //xx xx xx xx xx xx 23 22
        psrld       mm2,    16                      //xx xx xx xx 00 00 21 20

        movd        mm1,    [esi + edx + -4]        //xx xx xx xx 31 30 xx xx
        punpcklwd   mm2,    mm4                     //xx xx xx xx 23 22 21 20

        movd        mm4,    [esi + edx]             //xx xx xx xx xx xx 33 32
        psrld       mm1,    16                      //xx xx xx xx 00 00 31 30

        punpcklwd   mm1,    mm4                     //xx xx xx xx 33 32 31 30
        pxor        mm4,    mm4

        punpcklbw   mm2,    mm1                     //33 23 32 22 31 21 30 20
        movq        mm1,    mm0

        punpcklwd   mm0,    mm2                     //31 21 11 01 30 20 10 00
        lea         esi,    [esi + ecx*4]           //base + (stride * 4)

        punpckhwd   mm1,    mm2                     //33 23 13 03 32 22 12 02
        movq        mm6,    mm0                     //xx xx xx xx 30 20 10 00

        movq        [edi],  mm0
        movq        mm2,    mm1

        movq        [edi+8],  mm1
        psrlq       mm0,    32                      //xx xx xx xx 31 21 11 01

        movd        mm7,    [esi + -4]              //xx xx xx xx 41 40 xx xx
        punpcklbw   mm1,    mm4                     //convert to words

        movd        mm4,    [esi]                   //xx xx xx xx xx xx 43 42
        psrld       mm7,    16                      //xx xx xx xx 00 00 41 40

        movd        mm5,    [esi + ecx + -4]        //xx xx xx xx 51 50 xx xx
        punpcklwd   mm7,    mm4                     //xx xx xx xx 43 42 41 40

        movd        mm4,    [esi + ecx]             //xx xx xx xx xx xx 53 52
        psrld       mm5,    16

        punpcklwd   mm5,    mm4
        pxor        mm4,    mm4

        punpcklbw   mm0,    mm4

        psrlq       mm2,    32                      //xx xx xx xx 33 23 13 03
        psubw       mm1,    mm0                     //x = p[0] - p[ms]

        punpcklbw   mm7,    mm5                     //53 43 52 42 51 41 50 40
        movq        mm3,    mm1

        punpcklbw   mm6,    mm4
        paddw       mm3,    mm1

        punpcklbw   mm2,    mm4
        paddw       mm1,    mm3

        paddw       mm1,    fourFours               //x += LoopFilterAdjustBeforeShift
        psubw       mm6,    mm2

        movd        mm2,    [esi + ecx*2 + -4]      //xx xx xx xx 61 60 xx xx
        paddw       mm6,    mm1

        movd        mm4,    [esi + ecx*2]           //xx xx xx xx xx xx 63 62
        psrld       mm2,    16

        movd        mm5,    [esi + edx + -4]        //xx xx xx xx 71 70 xx xx
        punpcklwd   mm2,    mm4                     //xx xx xx xx 63 62 61 60

        movd        mm4,    [esi + edx]             //xx xx xx xx xx xx 73 72
        psrld       mm5,    16                      //xx xx xx xx 00 00 71 70

        mov         esi,    Src                     //restore PixelPtr
        punpcklwd   mm5,    mm4                     //xx xx xx xx 73 72 71 70

        psraw       mm6,    3                       //values to be clipped
        pxor        mm4,    mm4

        punpcklbw   mm2,    mm5                     //73 63 72 62 71 61 70 60
        movq        mm5,    mm7                     //53 43 52 42 51 41 50 40

        movq        mm1,    mm6
        punpckhwd   mm5,    mm2                     //73 63 53 43 72 62 52 42


        movq        [edi+24],  mm5                  //save for later
        punpcklwd   mm7,    mm2                     //71 61 51 41 70 60 50 40

        movq        [edi+16],  mm7                  //save for later
        psraw       mm6,    15

        movq        mm2,    [eax]                   //get the limit value
        movq        mm0,    mm7                     //xx xx xx xx 70 60 50 41

        psrlq       mm7,    32                      //xx xx xx xx 71 61 51 41
        pxor        mm1,    mm6

        psubsw      mm1,    mm6                     //abs(i)
        punpcklbw   mm5,    mm4

        por         mm6,    fourOnes                //now have -1 or 1 
        movq        mm3,    mm2

        punpcklbw   mm7,    mm4
        psubw       mm3,    mm1                     //limit - abs(i)

        movq        mm4,    mm3
        psraw       mm3,    15

        psubw       mm5,    mm7                     //x = p[0] - p[ms]
        pxor        mm4,    mm3

        psubsw      mm4,    mm3                     //abs(limit - abs(i))
        pxor        mm3,    mm3

        movd        mm1,    [edi + 28]              //xx xx xx xx 73 63 53 43
        psubusw     mm2,    mm4                     //limit - abs(limit - abs(i))

        punpcklbw   mm0,    mm3
        movq        mm7,    mm5

        paddw       mm7,    mm5
        pmullw      mm2,    mm6                     //new y -- wait 3 cycles

        punpcklbw   mm1,    mm3
        paddw       mm5,    mm7

        paddw       mm5,    fourFours               //x += LoopFilterAdjustBeforeShift
        psubw       mm0,    mm1

        paddw       mm0,    mm5
        pxor        mm6,    mm6     

        movd        mm7,    [edi + 8]               //xx xx xx xx 32 22 12 02
        psraw       mm0,    3                       //values to be clipped

        movd        mm3,    [edi + 4]               //xx xx xx xx 31 21 11 01
        punpcklbw   mm7,    mm6

        psubw       mm7,    mm2                     //p[ms] + y
        punpcklbw   mm3,    mm6

        paddw       mm3,    mm2                     //p[0] - y
        packuswb    mm7,    mm7                     //clamp[ p[ms] + y]

        packuswb    mm3,    mm3                     //clamp[ p[0] - y]
        movq        mm1,    mm0

        movq        mm2,    [eax]                   //get the limit value
        psraw       mm0,    15

        punpcklbw   mm3,    mm7                     //32 31 22 21 12 11 02 01                    
        movq        mm7,    mm0                     //save sign

        movd        eax,    mm3                     //12 11 02 01
        pxor        mm1,    mm0

        mov         [esi - 1],ax                    //02 01
        psubsw      mm1,    mm0                     //abs(i)

        shr         eax,    16
        movq        mm5,    mm2

        mov         [esi + ecx - 1],ax
        psrlq       mm3,    32                      //xx xx xx xx 32 31 22 21

        por         mm7,    fourOnes                //now have -1 or 1 
        psubw       mm5,    mm1                     //limit - abs(i)

        movd        eax,    mm3                     //32 31 22 21
        movq        mm4,    mm5

        mov         [esi + ecx*2 - 1],ax
        psraw       mm5,    15

        shr         eax,    16
        pxor        mm4,    mm5

        mov         [esi + edx - 1],ax
        psubsw      mm4,    mm5                     //abs(limit - abs(i))

        movd        mm5,    [edi + 24]              //xx xx xx xx 72 62 52 42
        psubusw     mm2,    mm4                     //limit - abs(limit - abs(i))

        pmullw      mm2,    mm7                     //new y
        pxor        mm6,    mm6

        movd        mm3,    [edi + 20]              //xx xx xx xx 71 61 51 41
        punpcklbw   mm5,    mm6

        lea         esi,    [esi + ecx*4]
        punpcklbw   mm3,    mm6

        paddw       mm3,    mm2                     //p[ms] + y
        psubw       mm5,    mm2                     //p[0] - y

        packuswb    mm3,    mm3                     //clamp[ p[ms] + y]
        packuswb    mm5,    mm5                     //clamp[ p[0] - y]

        punpcklbw   mm3,    mm5                     //72 71 62 61 52 51 42 41
        movd        eax,    mm3                     //52 51 42 41

        psrlq       mm3,    32                      //xx xx xx xx 72 71 62 61
        mov         [esi - 1],ax

        shr         eax,    16
        mov         [esi + ecx - 1],ax

        movd        eax,    mm3
        mov         [esi + ecx*2 - 1],ax

        shr         eax,16
        mov         [esi + edx - 1],ax

        mov         eax,    FLimitPtr               //
        lea         esi,    [esi+ ecx * 4]          // four line below

        movd        mm0,    [esi + -4]              //xx xx xx xx 01 00 xx xx
        movd        mm4,    [esi]                   //xx xx xx xx xx xx 03 02

        psrld       mm0,    16                      //xx xx xx xx 00 00 01 00
        movd        mm1,    [esi + ecx + -4]        //xx xx xx xx 11 10 xx xx

        punpcklwd   mm0,    mm4                     //xx xx xx xx 03 02 01 00
        movd        mm4,    [esi + ecx]             //xx xx xx xx xx xx 13 12

        psrld       mm1,    16                      //xx xx xx xx 00 00 11 10
        punpcklwd   mm1,    mm4                     //xx xx xx xx 13 12 11 10

        movd        mm2,    [esi + ecx*2 + -4]      //xx xx xx xx 21 20 xx xx
        punpcklbw   mm0,    mm1                     //13 03 12 02 11 01 10 00

        movd        mm4,    [esi + ecx*2]           //xx xx xx xx xx xx 23 22
        psrld       mm2,    16                      //xx xx xx xx 00 00 21 20

        movd        mm1,    [esi + edx + -4]        //xx xx xx xx 31 30 xx xx
        punpcklwd   mm2,    mm4                     //xx xx xx xx 23 22 21 20

        movd        mm4,    [esi + edx]             //xx xx xx xx xx xx 33 32
        psrld       mm1,    16                      //xx xx xx xx 00 00 31 30

        punpcklwd   mm1,    mm4                     //xx xx xx xx 33 32 31 30
        pxor        mm4,    mm4                     //clear mm4 for unpacking

        punpcklbw   mm2,    mm1                     //33 23 32 22 31 21 30 20
        movq        mm1,    mm0                     //13 03 12 02 11 01 10 00

        punpcklwd   mm0,    mm2                     //31 21 11 01 30 20 10 00
        punpckhwd   mm1,    mm2                     //33 23 13 03 32 22 12 02

        movq        mm6,    mm0                     //xx xx xx xx 30 20 10 00
        movq        [edi],  mm0
        
        movq        mm2,    mm1
        movq        [edi+8],  mm1

        psrlq       mm0,    32                      //xx xx xx xx 31 21 11 01
        punpcklbw   mm1,    mm4                     //-- 32 -- 22 -- 12 -- 02

        punpcklbw   mm0,    mm4                     //-- 31 -- 21 -- 11 -- 01
        psrlq       mm2,    32                      //xx xx xx xx 33 23 13 03

        psubw       mm1,    mm0                     // mm1 = p[0] - p[ms]
        movq        mm3,    mm1                     // mm3 = p[0] - p[ms]

        punpcklbw   mm6,    mm4                     //-- 30 -- 20 -- 10 -- 00
        paddw       mm3,    mm1                     // mm3 = (p[0] - p[ms])*2

        punpcklbw   mm2,    mm4                     //-- 33 -- 23 -- 13 -- 03
        paddw       mm1,    mm3                     // mm1 = (p[0] - p[ms])*3

        paddw       mm1,    fourFours               // mm1 = (p[0] - p[ms])*3 + 4
        psubw       mm6,    mm2                     // mm6 = (p[ms2]-p[1])
        
        paddw       mm6,    mm1                     // mm6 = (p[0] - p[ms])*3 + 4 + (p[ms2]-p[1])
        psraw       mm6,    3                       // mm6 = mm6 / 8

        movq        mm1,    mm6                     // make a copy of initial FiltVal
        psraw       mm6,    15                      // FFFF for negative, 0000 for positive

        pxor        mm1,    mm6                     // 
        psubsw      mm1,    mm6                     // abs(FiltVal)

        por         mm6,    fourOnes                // -1 or 1 for negative or positive
        movq        mm2,    [eax]                   // mm2 = FLimit
        
        movq        mm3,    mm2                     // mm3 = FLimit
        psubw       mm3,    mm1                     // mm3 = FLimit - abs(FiltVal)

        movq        mm4,    mm3                     // Make a copy of FLimit - abs(FiltVal)
        psraw       mm3,    15                      // FFFF and 0000 for - and +

        pxor        mm4,    mm3                     // 
        psubsw      mm4,    mm3                     // abs(Limit-abs(FiltVal))
    
        psubusw     mm2,    mm4                     // Limit - abs(Limit-abs(FiltVal)
        pmullw      mm2,    mm6                     // get the sign back

        pxor        mm5,    mm5                     // clear mm5 for unpacking
        movd        mm7,    [edi+8]                 // xx xx xx xx 32 22 12 02

        punpcklbw   mm7,    mm5                     // -- 32 -- 22 -- 12 -- 02
        movd        mm3,    [edi+4]                 // xx xx xx xx 31 21 11 01        

        psubw       mm7,    mm2                     // p[ms] - FiltVal
        punpcklbw   mm3,    mm5                     // -- 31 -- 21 -- 11 -- 01
        
        paddw       mm3,    mm2                     // p[0] + FiltVal
        packuswb    mm7,    mm7                     // clamping

        packuswb    mm3,    mm3                     // clamping
        punpcklbw   mm3,    mm7                     // 32 31 22 21 12 11 02 01

        movd        eax,    mm3                     // 12 11 02 01
        psrlq       mm3,    32                      // xx xx xx xx 32 31 22 21

        mov         [esi-1], ax                     // write 01 02
        shr         eax,    16                      // xx xx 12 11
        
        mov         [esi+ecx -1], ax                // write 11 12
        movd        eax,    mm3                     // 32 31 22 21

        mov         [esi+ecx*2 -1], ax              // write 21 22
        shr         eax,    16                      // xx xx 32 31

        mov         [esi+edx-1], ax                 // write 31 32

    } 
    
/*
    INT32 j;
	INT32 FiltVal;
    UINT8 *LimitTable = &LimitVal_VP31[VAL_RANGE];
    UINT32 FLimit;
    FLimit = LoopFilterLimitValuesV2[QValue];
	for ( j=0; j<Length; j++ )
	{            
        // set up blur kernel for differences
		FiltVal =  (( Src[-2]     ) - 
			        ( Src[-1] * 3 ) +
			        ( Src[ 0] * 3 ) - 
			        ( Src[ 1]     ) + 4 ) >> 3;

        FiltVal = Bound ( FLimit, FiltVal );

		Dest[-1] = LimitTable[(INT32)Src[-1] + FiltVal];
		Dest[ 0] = LimitTable[(INT32)Src[ 0] - FiltVal];
		
        Src  += SrcPitch;
        Dest += DestPitch;
	}
*/
}

/****************************************************************************
 * 
 *  ROUTINE       : FilteringVert_12_MMX
 *
 *  INPUTS        : UINT32 QIndex   : Quantization index.
 *                  UINT8 *PixelPtr : Pointer to source block.
 *                  INT32 Pitch     : Pitch of input image.
 *                  
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Filters the horizontal block edge inside a prediction
 *                  block.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/                       

void FilteringVert_12_MMX ( UINT32 QIndex, UINT8 *PixelPtr, INT32 Pitch ) 
{    
    INT16 *FLimitPtr = &LoopFilterLimitValuesV2_MMX[QIndex*4];

    __declspec(align(16)) const short fourFours[] = { 4, 4, 4, 4 };
    __declspec(align(16)) const short fourOnes[]  = { 1, 1, 1, 1 };

    __asm
    {
        mov         eax,    FLimitPtr               // 4 FLimit Values in shorts
        mov         edx,    Pitch                   // Pitch
   
        xor         ecx,    ecx                     // clear ecx to get negative Pitch
        sub         ecx,    edx                     // Negative Pitch

        mov         esi,    PixelPtr                // Src and Dest pointer
        movd        mm0,    [esi]                   // p[0], four pixels

        pxor        mm7,    mm7                     // clear mm7
        movd        mm1,    [esi+ecx]               // p[-1], four pixels

        punpcklbw   mm0,    mm7                     // unpack to short
        movd        mm2,    [esi+edx]               // p[1], four pixels

        punpcklbw   mm1,    mm7                     // unpack p[-1] to shorts
        movd        mm3,    [esi+ecx*2]             // p[-2], four pixels

        movq        mm5,    mm0                     // copy of unpacked p[0]
        movq        mm6,    mm1                     // copy of unpacked p[-1]

        psubw       mm0,    mm1                     // p[0] - p[-1]
        punpcklbw   mm2,    mm7                     // unpack p[1]

        movq        mm1,    mm0                     // make a copy of p[0]-p[-1]
        punpcklbw   mm3,    mm7                     // unpack p[-2]

        paddw       mm0,    mm1                     // (p[0]-p[-1]) * 2
        psubw       mm3,    mm2                     // (p[-2]-p[1])

        paddw       mm1,    mm0                     // (p[0]-p[-1]) * 3
        paddw       mm3,    mm1                     // p[-2]-3*p[-1]+3*p[0]-p[1]

        paddw       mm3,    fourFours               // p[-2]-3*p[-1]+3*p[0]-p[1]+4
        movq        mm0,    [eax]                   // FLimit

        psraw       mm3,    3                       // FiltVal
        movq        mm1,    mm3                     // FiltVal

        psraw       mm3,    15                      // FFFF-> Neg, 0->Pos
        pxor        mm1,    mm3                     //  

        psubsw      mm1,    mm3                     // abs(FiltVal)
        por         mm3,    fourOnes                // -1 or 1, corresponding the sign

        movq        mm2,    mm0                     // Copy of FLimit
        psubw       mm0,    mm1                     // FLimit - abs(FiltVal)

        movq        mm4,    mm0                     // copy FLimit-abs(FiltVal)
        psraw       mm0,    15                      // FFFF->Neg, 0->Pos

        pxor        mm4,    mm0                     //
        psubsw      mm4,    mm0                     // abs(FLimit-abs(FiltVal))

        psubusw     mm2,    mm4                     // FLimit-abs(FLimit-abs(FiltVal))
        pmullw      mm2,    mm3                     // Get the sign back

        psubw       mm5,    mm2                     // p[0] - FiltVal
        paddw       mm6,    mm2                     // p[-1] + FiltVal

        packuswb    mm5,    mm5                     // clamping
        packuswb    mm6,    mm6                     // clamping
        
        movd        [esi],  mm5                     // write p[0]
        movd        [esi+ecx], mm6                  // write p[-1]

        movd        mm0,    [esi+4]                   // p[0], four pixels
        movd        mm1,    [esi+ecx+4]               // p[-1], four pixels

        punpcklbw   mm0,    mm7                     // unpack to short
        movd        mm2,    [esi+edx+4]               // p[1], four pixels

        punpcklbw   mm1,    mm7                     // unpack p[-1] to shorts
        movd        mm3,    [esi+ecx*2+4]             // p[-2], four pixels

        movq        mm5,    mm0                     // copy of unpacked p[0]
        movq        mm6,    mm1                     // copy of unpacked p[-1]

        psubw       mm0,    mm1                     // p[0] - p[-1]
        punpcklbw   mm2,    mm7                     // unpack p[1]

        movq        mm1,    mm0                     // make a copy of p[0]-p[-1]
        punpcklbw   mm3,    mm7                     // unpack p[-2]

        paddw       mm0,    mm1                     // (p[0]-p[-1]) * 2
        psubw       mm3,    mm2                     // (p[-2]-p[1])

        paddw       mm1,    mm0                     // (p[0]-p[-1]) * 3
        paddw       mm3,    mm1                     // p[-2]-3*p[-1]+3*p[0]-p[1]

        paddw       mm3,    fourFours               // p[-2]-3*p[-1]+3*p[0]-p[1]+4
        movq        mm0,    [eax]                   // FLimit

        psraw       mm3,    3                       // FiltVal
        movq        mm1,    mm3                     // FiltVal

        psraw       mm3,    15                      // FFFF-> Neg, 0->Pos
        pxor        mm1,    mm3                     //  

        psubsw      mm1,    mm3                     // abs(FiltVal)
        por         mm3,    fourOnes                // -1 or 1, corresponding the sign

        movq        mm2,    mm0                     // Copy of FLimit
        psubw       mm0,    mm1                     // FLimit - abs(FiltVal)

        movq        mm4,    mm0                     // copy FLimit-abs(FiltVal)
        psraw       mm0,    15                      // FFFF->Neg, 0->Pos

        pxor        mm4,    mm0                     //
        psubsw      mm4,    mm0                     // abs(FLimit-abs(FiltVal))

        psubusw     mm2,    mm4                     // FLimit-abs(FLimit-abs(FiltVal))
        pmullw      mm2,    mm3                     // Get the sign back

        psubw       mm5,    mm2                     // p[0] - FiltVal
        paddw       mm6,    mm2                     // p[-1] + FiltVal

        packuswb    mm5,    mm5                     // clamping
        packuswb    mm6,    mm6                     // clamping
        
        movd        [esi+4],  mm5                   // write p[0]
        movd        [esi+ecx+4], mm6                // write p[-1]

        movd        mm0,    [esi+8]                   // p[0], four pixels
        movd        mm1,    [esi+ecx+8]               // p[-1], four pixels

        punpcklbw   mm0,    mm7                     // unpack to short
        movd        mm2,    [esi+edx+8]               // p[1], four pixels

        punpcklbw   mm1,    mm7                     // unpack p[-1] to shorts
        movd        mm3,    [esi+ecx*2+8]             // p[-2], four pixels

        movq        mm5,    mm0                     // copy of unpacked p[0]
        movq        mm6,    mm1                     // copy of unpacked p[-1]

        psubw       mm0,    mm1                     // p[0] - p[-1]
        punpcklbw   mm2,    mm7                     // unpack p[1]

        movq        mm1,    mm0                     // make a copy of p[0]-p[-1]
        punpcklbw   mm3,    mm7                     // unpack p[-2]

        paddw       mm0,    mm1                     // (p[0]-p[-1]) * 2
        psubw       mm3,    mm2                     // (p[-2]-p[1])

        paddw       mm1,    mm0                     // (p[0]-p[-1]) * 3
        paddw       mm3,    mm1                     // p[-2]-3*p[-1]+3*p[0]-p[1]

        paddw       mm3,    fourFours               // p[-2]-3*p[-1]+3*p[0]-p[1]+4
        movq        mm0,    [eax]                   // FLimit

        psraw       mm3,    3                       // FiltVal
        movq        mm1,    mm3                     // FiltVal

        psraw       mm3,    15                      // FFFF-> Neg, 0->Pos
        pxor        mm1,    mm3                     //  

        psubsw      mm1,    mm3                     // abs(FiltVal)
        por         mm3,    fourOnes                // -1 or 1, corresponding the sign

        movq        mm2,    mm0                     // Copy of FLimit
        psubw       mm0,    mm1                     // FLimit - abs(FiltVal)

        movq        mm4,    mm0                     // copy FLimit-abs(FiltVal)
        psraw       mm0,    15                      // FFFF->Neg, 0->Pos

        pxor        mm4,    mm0                     //
        psubsw      mm4,    mm0                     // abs(FLimit-abs(FiltVal))

        psubusw     mm2,    mm4                     // FLimit-abs(FLimit-abs(FiltVal))
        pmullw      mm2,    mm3                     // Get the sign back

        psubw       mm5,    mm2                     // p[0] - FiltVal
        paddw       mm6,    mm2                     // p[-1] + FiltVal

        packuswb    mm5,    mm5                     // clamping
        packuswb    mm6,    mm6                     // clamping
        
        movd        [esi+8],  mm5                   // write p[0]
        movd        [esi+ecx+8], mm6                // write p[-1]
    }
}
