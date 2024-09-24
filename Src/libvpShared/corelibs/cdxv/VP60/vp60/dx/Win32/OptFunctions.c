/****************************************************************************
*
*   Module Title :     OptFunctions.c
*
*   Description  :     MMX or otherwise processor specific 
*                      optimised versions of functions
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*
 *  1.08 JBB 13 Jun 01 VP4 Code Clean Out
*   1.07 JBB 26/01/01  Removed unused function
*	1.06 YWX 23/05/00  Remove the clamping in MmxReconPostProcess()
*	1.05 YWX 15/05/00  Added MmxReconPostProcess()
*   1.04 SJL 03/14/00  Added in Tim's versions of MmxReconInter and MmxReconInterHalfPixel2. 
*   1.03 PGW 12/10/99  Changes to reduce uneccessary dependancies. 
*   1.02 PGW 30/08/99  Minor changes to MmxReconInterHalfPixel2().
*   1.01 PGW 13/07/99  Changes to keep reconstruction data to 16 bit
*   1.00 PGW 14/06/99  Configuration baseline
*
*****************************************************************************
*/

/* 
    Use Tim's optimized version.
*/
/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              // Strict type checking. 

#include "codec_common.h"

#include "pbdll.h"

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        

/****************************************************************************
*  Imports.
*****************************************************************************
*/   

extern INT32 * XX_LUT;

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

INT16 Ones[4]               = {1,1,1,1};
INT16 OneTwoEight[4]        = {128,128,128,128};
UINT8 Eight128s[8]          = {128,128,128,128,128,128,128,128};

#pragma warning( disable : 4799 )  // Disable no emms instruction warning!
                                      
/****************************************************************************
*  Forward References
*****************************************************************************
*/  
/****************************************************************************
 * 
 *  ROUTINE       :     ClearSysState()
 *
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :    
 * 
 *
 *  FUNCTION      :     DoesNothing
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void ClearSysStateC(void)
{
}

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
 *  ROUTINE       :     MMXReconIntra
 *
 *  INPUTS        :     INT16 *  idct
 *                               Pointer to the output from the idct for this block
 *
 *                      UINT32   stride
 *                               Line Length in pixels in recon and reference images
 *                               
 *
 *                     
 *
 *  OUTPUTS       :     UINT8 *  dest
 *                               The reconstruction buffer
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Reconstructs an intra block - MMX version
 *
 *  SPECIAL NOTES :     Tim Murphy's optimized version 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void MMXReconIntra( PB_INSTANCE *pbi, UINT8 * dest, INT16 * idct, INT32 stride )
{
    __asm
    {
        // u    pipe
        //   v  pipe
        mov         eax,[idct]              ; Signed 16 bit inputs
          mov         edx,[dest]            ; Signed 8 bit outputs
        movq        mm0,[Eight128s]         ; Set mm0 to 0x8080808080808080
          ;
        mov         ebx,[stride]            ; Line stride in output buffer
          lea         ecx,[eax+128]         ; Endpoint in input buffer
loop_label:                                 ;
        movq        mm2,[eax]               ; First four input values
          ;
        packsswb    mm2,[eax+8]             ; pack with next(high) four values
          por         mm0,mm0               ; stall
        pxor        mm2,mm0                 ; Convert result to unsigned (same as add 128)
          lea         eax,[eax + 16]        ; Step source buffer
        cmp         eax,ecx                 ; are we done
          ;
        movq        [edx],mm2               ; store results
          ;
        lea         edx,[edx+ebx]           ; Step output buffer
          jc          loop_label            ; Loop back if we are not done
    }
    // 6c/8 elts = 9c/8 = 1.125 c/pix

}

/****************************************************************************
 * 
 *  ROUTINE       :     MmxReconInter
 *
 *  INPUTS        :     UINT8 *  RefPtr
 *                               The last frame reference
 *
 *                      INT16 *  ChangePtr
 *                               Pointer to the change data
 *
 *                      UINT32   LineStep
 *                               Line Length in pixels in recon and ref images
 *
 *  OUTPUTS       :     UINT8 *  ReconPtr
 *                               The reconstruction
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Reconstructs data from last data and change
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void MmxReconInter( PB_INSTANCE *pbi, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep )
{
    (void) pbi;

 _asm {
	push	edi
;;	 mov	ebx, [ref]
;;	mov		ecx, [diff]
;;	 mov	eax, [dest]
;;	mov		edx, [stride]
	 mov	ebx, [RefPtr]
	mov		ecx, [ChangePtr]
	 mov	eax, [ReconPtr]
	mov		edx, [LineStep]
	 pxor	mm0, mm0
	lea		edi, [ecx + 128]
	 ;
  L:
	movq	mm2, [ebx]			; (+3 misaligned) 8 reference pixels
	 ;
	movq	mm4, [ecx]			; first 4 changes
	 movq	mm3, mm2
	movq	mm5, [ecx + 8]		; last 4 changes
	 punpcklbw mm2, mm0			; turn first 4 refs into positive 16-bit #s
	paddsw	mm2, mm4			; add in first 4 changes
	 punpckhbw mm3, mm0			; turn last 4 refs into positive 16-bit #s
	paddsw	mm3, mm5			; add in last 4 changes
	 add	ebx, edx			; next row of reference pixels
	packuswb mm2, mm3			; pack result to unsigned 8-bit values
	 lea	ecx, [ecx + 16]		; next row of changes
	cmp		ecx, edi			; are we done?
	 ;
	movq	[eax], mm2			; store result
	 ;
	lea		eax, [eax+edx]		; next row of output
	 jc		L					; 12c / 8 elts = 18c / 8 pixels = 2.25 c/pix

	pop		edi
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


