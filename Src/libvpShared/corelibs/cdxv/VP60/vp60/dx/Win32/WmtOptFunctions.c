 /****************************************************************************
 *
 *   Module Title :     WmtOptFunctions.c
 *
 *   Description  :     willamette processor specific 
 *                      optimised versions of functions
 *
 *   AUTHOR      :		Yaowu Xu
 *
 *	 Special Note:		
 *
 *****************************************************************************
 *   Revision History
 *
 *
 *  1.04 JBB 13 Jun 01 VP4 Code Clean Out
 *   1.03 YWX 07-Dec-00 Removed constants and functions that are not in use
 * 			Added push and pop ebx in WmtReconIntra
 *   1.02 YWX 30 Aug 00 changed to be compatible with Microsoft compiler
 *   1.01 YWX 13 JUL 00 New Willamette Optimized Functions
 *   1.00 YWX 14/06/00  Configuration baseline from OptFunctions.c
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

#if defined(_WIN32_WCE)
#pragma pack(16)
static  UINT8 Eight128s[8] =  {128,128,128,128,128,128,128,128};
#pragma pack()
#else
_declspec(align(16)) static  UINT8 Eight128s[8] =  {128,128,128,128,128,128,128,128};
#endif

#pragma warning( disable : 4799 )  // Disable no emms instruction warning!
                                      
/****************************************************************************
*  Forward References
*****************************************************************************
*/  


/****************************************************************************
 * 
 *  ROUTINE       :     WmtReconIntra
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
 *  FUNCTION      :     Reconstructs an intra block - wmt version
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void WmtReconIntra( PB_INSTANCE *pbi, UINT8 * dest, INT16 * idct, INT32 stride )
{
    __asm
    {
	
		push		ebx

        mov         eax,[idct]						; Signed 16 bit inputs
        mov         edx,[dest]						; Unsigned 8 bit outputs

        movq		xmm0,QWORD PTR [Eight128s]		; Set xmm0 to 0x000000000000008080808080808080
		pxor		xmm3, xmm3						; set xmm3 to 0
													;
        mov         ebx,[stride]					; Line stride in output buffer
        lea         ecx,[eax+128]					; Endpoint in input buffer

loop_label:                                 

        movdqa		xmm2,XMMWORD PTR [eax]			; Read the eight inputs
		packsswb	xmm2,xmm3						;		
		
		pxor        xmm2,xmm0						; Convert result to unsigned (same as add 128)
        lea         eax,[eax + 16]					; Step source buffer

        cmp         eax,ecx							; are we done
        movq		QWORD PTR [edx],xmm2			; store results

        lea         edx,[edx+ebx]					; Step output buffer
        jc          loop_label						; Loop back if we are not done

		pop			ebx
    }

}

/****************************************************************************
 * 
 *  ROUTINE       :     WmtReconInter
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
void WmtReconInter( PB_INSTANCE *pbi, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep )
{
    (void) pbi;

 _asm {
		push	edi
		
		mov		ebx, [RefPtr]
		mov		ecx, [ChangePtr]

		mov		eax, [ReconPtr]
		mov		edx, [LineStep]

		pxor	xmm0, xmm0
		lea		edi, [ecx + 128]
  L:
		movq	xmm2, QWORD ptr [ebx]		; (+3 misaligned) 8 reference pixels
		movdqa	xmm4, XMMWORD ptr [ecx]		; 8 changes
		
		punpcklbw xmm2, xmm0				; 

		add	ebx, edx						; next row of reference pixels
		paddsw	xmm2, xmm4					; add in first 4 changes

		lea		ecx, [ecx + 16]				; next row of changes
		packuswb xmm2, xmm0					; pack result to unsigned 8-bit values

		cmp		ecx, edi					; are we done?
		movq	QWORD PTR [eax], xmm2		; store result

		lea		eax, [eax+edx]				; next row of output
		jc		L							; 12c / 8 elts = 18c / 8 pixels = 2.25 c/pix

		pop		edi
 }

}




