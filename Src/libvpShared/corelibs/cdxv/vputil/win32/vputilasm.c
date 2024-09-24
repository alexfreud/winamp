/****************************************************************************
 *
 *   Module Title :     newLoopTest_asm.c 
 *
 *   Description  :     Codec specific functions
 *
 *   AUTHOR       :     Yaowu Xu
 *
 *****************************************************************************
 *   Revision History
 *
 *   1.02 YWX 03-Nov-00 Changed confusing variable name
 *   1.01 YWX 02-Nov-00 Added the set of functions
 *   1.00 YWX 19-Oct-00 configuration baseline
 *****************************************************************************
 */ 

/****************************************************************************
 *  Header Frames
 *****************************************************************************
 */


#define STRICT              /* Strict type checking. */
#include "codec_common.h"
#include <math.h>

 /****************************************************************************
 *  Module constants.
 *****************************************************************************
 */        

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))


/****************************************************************************
 *  Explicit Imports
 *****************************************************************************
 */ 
extern void SatUnsigned8( UINT8 * ResultPtr, INT16 * DataBlock, 
                         UINT32 ResultLineStep, UINT32 DataLineStep );

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
 *  Foreward References
 *****************************************************************************
 */       


/****************************************************************************
 * 
 *  ROUTINE       :     ClearMmx()
 *
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     Clears down the MMX state
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ClearMmx(void)
{
	__asm
	{
		emms									; Clear the MMX state.
	}
}
       
/****************************************************************************
 * 
 *  ROUTINE       :     CopyBlockUsingMMX
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Copies a block from source to destination
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void CopyBlockMMX(unsigned char *src, unsigned char *dest, unsigned int srcstride)
{
	unsigned char *s = src;
	unsigned char *d = dest;
	unsigned int stride = srcstride;
	// recon copy 
	_asm
	{
			mov		ecx, [stride]
			mov		eax, [s]
			mov		ebx, [d]
			lea		edx, [ecx + ecx * 2]

			movq	mm0, [eax]
			movq	mm1, [eax + ecx]
			movq	mm2, [eax + ecx*2]
			movq	mm3, [eax + edx]

			lea		eax, [eax + ecx*4]

			movq	[ebx], mm0
			movq	[ebx + ecx], mm1
			movq	[ebx + ecx*2], mm2
			movq	[ebx + edx], mm3

			lea		ebx, [ebx + ecx * 4]

			movq	mm0, [eax]
			movq	mm1, [eax + ecx]
			movq	mm2, [eax + ecx*2]
			movq	mm3, [eax + edx]

			movq	[ebx], mm0
			movq	[ebx + ecx], mm1
			movq	[ebx + ecx*2], mm2
			movq	[ebx + edx], mm3
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     CopyBlockUsingMMX
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Copies a block from source to destination
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void Copy12x12_MMX(
    const unsigned char *src, 
    unsigned char *dest, 
    unsigned int srcstride,
    unsigned int deststride)
{


	int j=0;
	do
	{
		((UINT32*)dest)[0] = ((UINT32*)src)[0];
		((UINT32*)dest)[1] = ((UINT32*)src)[1];
		((UINT32*)dest)[2] = ((UINT32*)src)[2];
		src+=srcstride;
		dest+=deststride;
	}
	while(++j<12);

}

/****************************************************************************

/****************************************************************************
 * 
 *  ROUTINE       :     AverageBlock_MMX
 *  
 *  INPUTS        :     Two block data to be averaged
 *						
 *  OUTPUTS       :     block with the average values
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Do pixel averages on two reference blocks 
 *
 *  SPECIAL NOTES :     This functions has a mmx version in newlooptest_asm.c
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void AverageBlock_MMX( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 ReconPixelsPerLine)
{
    
    __asm 
    {
        mov         esi,    ReconPtr1 
        mov         eax,    ReconPtr2

        mov         edi,    ReconRefPtr
        mov         ecx,    BLOCK_HEIGHT_WIDTH

        mov         edx,    ReconPixelsPerLine
        pxor        mm7,    mm7

AverageBlock_Loop:

        movq        mm0,    [esi]
        movq        mm1,    [eax]

        movq        mm2,    mm0
        punpcklbw   mm0,    mm7

        movq        mm3,    mm1
        punpcklbw   mm1,    mm7

        paddw       mm0,    mm1
        punpckhbw   mm2,    mm7

        psraw       mm0,    1
        punpckhbw   mm3,    mm7

        paddw       mm2,    mm3
        movq        [edi],  mm0

        psraw       mm2,    1
        add         esi,    edx

        add         eax,    edx
        add         edi,    16

        movq        [edi-8], mm2
        dec         ecx

        jnz         AverageBlock_Loop
    }
    /*    
    UINT32 i;

    // For each block row
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {
        ReconRefPtr[0] = (INT16)((INT32)(ReconPtr1[0])+ ((INT32)ReconPtr2[0]))>>1;
        ReconRefPtr[1] = (INT16)((INT32)(ReconPtr1[1])+ ((INT32)ReconPtr2[1]))>>1;
        ReconRefPtr[2] = (INT16)((INT32)(ReconPtr1[2])+ ((INT32)ReconPtr2[2]))>>1;
        ReconRefPtr[3] = (INT16)((INT32)(ReconPtr1[3])+ ((INT32)ReconPtr2[3]))>>1;
        ReconRefPtr[4] = (INT16)((INT32)(ReconPtr1[4])+ ((INT32)ReconPtr2[4]))>>1;
        ReconRefPtr[5] = (INT16)((INT32)(ReconPtr1[5])+ ((INT32)ReconPtr2[5]))>>1;
        ReconRefPtr[6] = (INT16)((INT32)(ReconPtr1[6])+ ((INT32)ReconPtr2[6]))>>1;
        ReconRefPtr[7] = (INT16)((INT32)(ReconPtr1[7])+ ((INT32)ReconPtr2[7]))>>1;
        
        // Start next row
        ReconPtr1 += ReconPixelsPerLine;
        ReconPtr2 += ReconPixelsPerLine;

        ReconRefPtr += BLOCK_HEIGHT_WIDTH;
    }
    */
}


/****************************************************************************
 * 
 *  ROUTINE       :     UnpackBlock
 *  
 *  INPUTS        :     Block of char data to be converted to short
 *						
 *  OUTPUTS       :     converted output
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Converted char block data to short
 *
 *  SPECIAL NOTES :     This functions has a mmx version in newlooptest_asm.c
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void UnpackBlock_MMX( UINT8 *ReconPtr, INT16 *ReconRefPtr, UINT32 ReconPixelsPerLine)
{
    
    __asm 
    {
        mov         esi,    ReconPtr 
        mov         edi,    ReconRefPtr

        mov         ecx,    BLOCK_HEIGHT_WIDTH
        mov         edx,    ReconPixelsPerLine

        pxor        mm7,    mm7

UnpackBlock_Loop:

        movq        mm0,    [esi] 
        movq        mm2,    mm0

        punpcklbw   mm0,    mm7
        movq        [edi],  mm0

        punpckhbw   mm2,    mm7
        add         esi,    edx

        movq        [edi+8], mm2
        add         edi,    16

        dec         ecx
        jnz         UnpackBlock_Loop
    }
    
    /*
    UINT32 i;

    // For each block row
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {

        ReconRefPtr[0] = (INT16)(ReconPtr[0]);
        ReconRefPtr[1] = (INT16)(ReconPtr[1]);
        ReconRefPtr[2] = (INT16)(ReconPtr[2]);
        ReconRefPtr[3] = (INT16)(ReconPtr[3]);
        ReconRefPtr[4] = (INT16)(ReconPtr[4]);
        ReconRefPtr[5] = (INT16)(ReconPtr[5]);
        ReconRefPtr[6] = (INT16)(ReconPtr[6]);
        ReconRefPtr[7] = (INT16)(ReconPtr[7]);
        
        // Start next row
        ReconPtr += ReconPixelsPerLine;
        ReconRefPtr += BLOCK_HEIGHT_WIDTH;
    }
    */
}

/****************************************************************************
 * 
 *  ROUTINE       :     SubtractBlock
 *  
 *  INPUTS        :     Get the residue data for the block
 *						
 *  OUTPUTS       :     Source block data and ref block data
 *
 *  RETURNS       :     residue block data
 *
 *  FUNCTION      :     do pixel subtraction of ref block from source block
 *
 *  SPECIAL NOTES :     This functions has a mmx version in newlooptest_asm.c
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void SubtractBlock_MMX( UINT8 *SrcBlock, INT16 *DestPtr, UINT32 LineStep )
{

    __asm 
    {

        mov         esi,    SrcBlock
        mov         edi,    DestPtr

        mov         edx,    LineStep
        mov         ecx,    8

        pxor        mm7,    mm7

SubtractBlock_Loop:

        movq        mm0,    [esi]
        movq        mm1,    [edi]

        movq        mm2,    mm0
        punpcklbw   mm0,    mm7

        movq        mm3,    [edi+8]
        psubw       mm0,    mm1
        
        punpckhbw   mm2,    mm7
        movq        [edi],  mm0

        psubw       mm2,    mm3
        add         esi,    edx

        movq        [edi+8], mm2
        add         edi,    16

        dec         ecx
        jnz         SubtractBlock_Loop
    }

    /*    
    UINT32 i;

    // For each block row
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {

        DestPtr[0] = (INT16)((INT32)SrcBlock[0] - (INT32)DestPtr[0]);
        DestPtr[1] = (INT16)((INT32)SrcBlock[1] - (INT32)DestPtr[1]);
        DestPtr[2] = (INT16)((INT32)SrcBlock[2] - (INT32)DestPtr[2]);
        DestPtr[3] = (INT16)((INT32)SrcBlock[3] - (INT32)DestPtr[3]);
        DestPtr[4] = (INT16)((INT32)SrcBlock[4] - (INT32)DestPtr[4]);
        DestPtr[5] = (INT16)((INT32)SrcBlock[5] - (INT32)DestPtr[5]);
        DestPtr[6] = (INT16)((INT32)SrcBlock[6] - (INT32)DestPtr[6]);
        DestPtr[7] = (INT16)((INT32)SrcBlock[7] - (INT32)DestPtr[7]);
        
        // Start next row
        SrcBlock += LineStep;
        DestPtr += BLOCK_HEIGHT_WIDTH;
    }
    */
}

/****************************************************************************
 * 
 *  ROUTINE       :     ReconBlock
 *  
 *  INPUTS        :     
 *						
 *  OUTPUTS       :     
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     Reconstrut a block using ref blocka and change data
 *
 *  SPECIAL NOTES :     This functions has a mmx version in newlooptest_asm.c
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ReconBlock_MMX( INT16 *SrcBlock, INT16 *ReconRefPtr, UINT8 *DestBlock, UINT32 LineStep)
{

    __asm 
    {
    
        mov         esi,    SrcBlock
        mov         eax,    ReconRefPtr

        mov         edi,    DestBlock
        mov         ecx,    8

        mov         edx,    LineStep
        pxor        mm7,    mm7

ReconBlock_Loop:

        movq        mm0,    [esi]
        movq        mm1,    [eax]
    
        movq        mm2,    [esi+8]
        movq        mm3,    [eax+8]

        paddw       mm0,    mm1
        paddw       mm2,    mm3

        packuswb    mm0,    mm2
        movq        [edi],  mm0
        
        add         esi,    16
        add         eax,    16

        add         edi,    edx
        dec         ecx

        jnz         ReconBlock_Loop
        
    }
    
    /*    
    UINT32 i;
    INT16 *SrcBlockPtr = SrcBlock;

    // For each block row
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {
        SrcBlock[0] += ReconRefPtr[0];
        SrcBlock[1] += ReconRefPtr[1];
        SrcBlock[2] += ReconRefPtr[2];
        SrcBlock[3] += ReconRefPtr[3];
        SrcBlock[4] += ReconRefPtr[4];
        SrcBlock[5] += ReconRefPtr[5];
        SrcBlock[6] += ReconRefPtr[6];
        SrcBlock[7] += ReconRefPtr[7];
        
        // Start next row
        SrcBlock += BLOCK_HEIGHT_WIDTH;
        ReconRefPtr += BLOCK_HEIGHT_WIDTH;
    }
    // Saturated the block and write to the output
    SatUnsigned8( DestBlock, SrcBlockPtr, LineStep, BLOCK_HEIGHT_WIDTH );
    */

}

