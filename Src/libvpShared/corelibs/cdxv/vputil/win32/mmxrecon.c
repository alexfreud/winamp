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
#define USING_TIMS 1

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              // Strict type checking. 

#include "codec_common.h"

#include "reconstruct.h"

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
void MMXReconIntra( INT16 *TmpDataBuffer, UINT8 * dest, UINT16 * idct, UINT32 stride )
{
	(void) TmpDataBuffer;
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
#if USING_TIMS
void MmxReconInter( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep )
{
    (void) TmpDataBuffer;

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
#else
void MmxReconInter( INT16 *TmpDataBuffer, UINT8 * ReconPtr, UINT8 * RefPtr, INT16 * ChangePtr, UINT32 LineStep )
{

    // Note that the line step for the change data is assumed to be 8 * 32 bits.
__asm
    {
        // Set up data pointers
        mov         eax,dword ptr [ReconPtr]  
        mov         ebx,dword ptr [RefPtr]      
        mov         ecx,dword ptr [ChangePtr]   
		mov         edx,dword ptr [LineStep]
		pxor        mm6, mm6					; Blank mmx6

        // Row 1
        // Load the data values. The change data needs to be unpacked to words
        movq        mm0,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data
        paddsw      mm0, mm2                    ; First 4 values
        paddsw      mm1, mm4                    ; Second 4 values

        // Pack and store
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [eax],mm0         ; Write the data out to the results buffer

		add         ebx,edx						; Step the reference pointer.
        add         ecx,16                      ; Step the change pointer.
        add         eax,edx                     ; Step the reconstruction pointer

        // Row 2
        // Load the data values. The change data needs to be unpacked to words
        movq        mm0,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data
        paddsw      mm0, mm2                    ; First 4 values
        paddsw      mm1, mm4                    ; Second 4 values

        // Pack and store
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [eax],mm0         ; Write the data out to the results buffer

		add         ebx,edx						; Step the reference pointer.
        add         ecx,16                      ; Step the change pointer.
        add         eax,edx                     ; Step the reconstruction pointer

        // Row 3
        // Load the data values. The change data needs to be unpacked to words
        movq        mm0,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data
        paddsw      mm0, mm2                    ; First 4 values
        paddsw      mm1, mm4                    ; Second 4 values

        // Pack and store
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [eax],mm0         ; Write the data out to the results buffer

		add         ebx,edx						; Step the reference pointer.
        add         ecx,16                      ; Step the change pointer.
        add         eax,edx                     ; Step the reconstruction pointer

        // Row 4
        // Load the data values. The change data needs to be unpacked to words
        movq        mm0,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data
        paddsw      mm0, mm2                    ; First 4 values
        paddsw      mm1, mm4                    ; Second 4 values

        // Pack and store
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [eax],mm0         ; Write the data out to the results buffer

		add         ebx,edx						; Step the reference pointer.
        add         ecx,16                      ; Step the change pointer.
        add         eax,edx                     ; Step the reconstruction pointer

        // Row 5
        // Load the data values. The change data needs to be unpacked to words
        movq        mm0,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data
        paddsw      mm0, mm2                    ; First 4 values
        paddsw      mm1, mm4                    ; Second 4 values

        // Pack and store
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [eax],mm0         ; Write the data out to the results buffer

		add         ebx,edx						; Step the reference pointer.
        add         ecx,16                      ; Step the change pointer.
        add         eax,edx                     ; Step the reconstruction pointer

        // Row 6
        // Load the data values. The change data needs to be unpacked to words
        movq        mm0,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data
        paddsw      mm0, mm2                    ; First 4 values
        paddsw      mm1, mm4                    ; Second 4 values

        // Pack and store
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [eax],mm0         ; Write the data out to the results buffer

		add         ebx,edx						; Step the reference pointer.
        add         ecx,16                      ; Step the change pointer.
        add         eax,edx                     ; Step the reconstruction pointer

        // Row 7
        // Load the data values. The change data needs to be unpacked to words
        movq        mm0,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data
        paddsw      mm0, mm2                    ; First 4 values
        paddsw      mm1, mm4                    ; Second 4 values

        // Pack and store
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [eax],mm0         ; Write the data out to the results buffer

		add         ebx,edx						; Step the reference pointer.
        add         ecx,16                      ; Step the change pointer.
        add         eax,edx                     ; Step the reconstruction pointer

        // Row 8
        // Load the data values. The change data needs to be unpacked to words
        movq        mm0,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data
        paddsw      mm0, mm2                    ; First 4 values
        paddsw      mm1, mm4                    ; Second 4 values

        // Pack and store
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [eax],mm0         ; Write the data out to the results buffer
   
        //emms									; Clear the MMX state.
    }
}
#endif

/****************************************************************************
 * 
 *  ROUTINE       :     MmxReconInterHalfPixel2
 *
 *  INPUTS        :     UINT8 *  RefPtr1, RefPtr2
 *                               The last frame reference
 *
 *                      INT16 *  ChangePtr
 *                               Pointer to the change data
 *
 *                      UINT32   LineStep
 *                               Line Length in pixels in recon and ref images
 *                               
 *
 *  OUTPUTS       :     UINT8 *  ReconPtr
 *                               The reconstruction
 *
 *  RETURNS       :     None
 *
 *  FUNCTION      :     Reconstructs data from half pixel reference data and change. 
 *                      Half pixel data interpolated from 2 references.
 *
 *  SPECIAL NOTES :     
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
#if USING_TIMS

#define A 0

void MmxReconInterHalfPixel2( INT16 *TmpDataBuffer, UINT8 * ReconPtr, 
		    	              UINT8 * RefPtr1, UINT8 * RefPtr2, 
						      INT16 * ChangePtr, UINT32 LineStep )
{
#	if A
		static culong FourOnes[2] = { 65537, 65537};	// only read once
#	endif
	(void) TmpDataBuffer;

 _asm {
	push	esi
	 push	edi

;;	mov		ecx, [diff]
;;	 mov	esi, [ref1]
;;	mov		edi, [ref2]
;;	 mov	ebx, [dest]
;;	mov		edx, [stride]

	mov		ecx, [ChangePtr]
	 mov	esi, [RefPtr1]
	mov		edi, [RefPtr2]
	 mov	ebx, [ReconPtr]
	mov		edx, [LineStep]

	 lea	eax, [ecx+128]

#	if A
		movq	mm1, [FourOnes]
#	endif

	 pxor	mm0, mm0
  L:
	movq	mm2, [esi]		; (+3 misaligned) mm2 = row from ref1
	 ;
	movq	mm4, [edi]		; (+3 misaligned) mm4 = row from ref2
	 movq	mm3, mm2
	punpcklbw mm2, mm0		; mm2 = start ref1 as positive 16-bit #s
	 movq	mm5, mm4
	movq	mm6, [ecx]		; mm6 = first 4 changes
	 punpckhbw mm3, mm0		; mm3 = end ref1 as positive 16-bit #s
	movq	mm7, [ecx+8]	; mm7 = last 4 changes
	 punpcklbw mm4, mm0		; mm4 = start ref2 as positive 16-bit #s
	punpckhbw mm5, mm0		; mm5 = end ref2 as positive 16-bit #s
	 paddw	mm2, mm4		; mm2 = start (ref1 + ref2)
	paddw	mm3, mm5		; mm3 = end (ref1 + ref2)

#	if A
		 paddw	mm2, mm1		; rounding adjustment
		paddw	mm3, mm1
#	endif

	 psrlw	mm2, 1			; mm2 = start (ref1 + ref2)/2
	psrlw	mm3, 1			; mm3 = end (ref1 + ref2)/2
	 paddw	mm2, mm6		; add changes to start
	paddw	mm3, mm7		; add changes to end
	 lea	ecx, [ecx+16]	; next row idct
	packuswb mm2, mm3		; pack start|end to unsigned 8-bit
	 add	esi, edx		; next row ref1
	add		edi, edx		; next row ref2
	 cmp	ecx, eax
	movq	[ebx], mm2		; store result
	 ;
	lea		ebx, [ebx+edx]
	 jc		L				; 22c / 8 elts = 33c / 8 pixels = 4.125 c/pix

	pop		edi
	 pop	esi
 }
}

#undef A

#else
void MmxReconInterHalfPixel2( INT16 *TmpDataBuffer, UINT8 * ReconPtr, 
		    	              UINT8 * RefPtr1, UINT8 * RefPtr2, 
						      INT16 * ChangePtr, UINT32 LineStep )
{
    UINT8 * TmpDataPtr = (UINT8 *)TmpDataBuffer->TmpReconBuffer;

    // Note that the line step for the change data is assumed to be 8 * 32 bits.
    __asm
    {
		pxor        mm6, mm6					; Blank mmx6

        // Set up data pointers
        mov         eax,dword ptr [RefPtr1]      
        mov         ebx,dword ptr [RefPtr2]      
        mov         edx,dword ptr [LineStep]

        // Row 1
        // Load the change pointer
        mov         ecx,dword ptr [ChangePtr]   

        // Load the data values (Ref1 and Ref2) and unpack to signed 16 bit values
        movq        mm0,dword ptr [eax]         ; Load 8 elements of source data
        movq        mm2,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
        movq        mm3, mm2                    ; Copy data

        punpcklbw   mm0, mm6					; Low bytes to words
		punpcklbw   mm2, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words
		punpckhbw   mm3, mm6					; High bytes to words

        // Average Ref1 and Ref2
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm3                    ; Second 4 values
        psrlw       mm0, 1
        psrlw       mm1, 1

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data reference and difference data
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm4                    ; Second 4 values

        // Pack and store
        mov         ecx,dword ptr [TmpDataPtr]  ; Load the temp results pointer 
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [ecx],mm0         ; Write the data out to the temporary results buffer
        add         eax,edx                     ; Step the reference pointers
        add         ebx,edx                    

        // Row 2
        // Load the change pointer
        mov         ecx,dword ptr [ChangePtr]   
        add         ecx,16                    

        // Load the data values (Ref1 and Ref2). 
        movq        mm0,dword ptr [eax]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words

        movq        mm2,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm3, mm2                    ; Copy data
		punpcklbw   mm2, mm6					; Low bytes to words
		punpckhbw   mm3, mm6					; High bytes to words

        // Average Ref1 and Ref2
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm3                    ; Second 4 values
        psrlw       mm0, 1
        psrlw       mm1, 1

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data reference and difference data
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm4                    ; Second 4 values

        // Pack and store
        mov         ecx,dword ptr [TmpDataPtr]  ; Load the temp results pointer 
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [ecx+8],mm0       ; Write the data out to the temporary results buffer
        add         eax,edx                     ; Step the reference pointers
        add         ebx,edx                    

        // Row 3
        // Load the change pointer
        mov         ecx,dword ptr [ChangePtr]   
        add         ecx,32                    

        // Load the data values (Ref1 and Ref2). 
        movq        mm0,dword ptr [eax]         ; Load 8 elements of source data
        movq        mm2,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
        movq        mm3, mm2                    ; Copy data

		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words
		punpcklbw   mm2, mm6					; Low bytes to words
		punpckhbw   mm3, mm6					; High bytes to words

        // Average Ref1 and Ref2
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm3                    ; Second 4 values
        psrlw       mm0, 1
        psrlw       mm1, 1

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data reference and difference data
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm4                    ; Second 4 values

        // Pack and store
        mov         ecx,dword ptr [TmpDataPtr]   
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [ecx+16],mm0         ; Write the data out to the temporary results buffer
        add         eax,edx                     ; Step the reference pointers
        add         ebx,edx                    

        // Row 4
        // Load the change pointer
        mov         ecx,dword ptr [ChangePtr]   
        add         ecx,48                    

        // Load the data values (Ref1 and Ref2). 
        movq        mm0,dword ptr [eax]         ; Load 8 elements of source data
        movq        mm2,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
        movq        mm3, mm2                    ; Copy data

		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words
		punpcklbw   mm2, mm6					; Low bytes to words
		punpckhbw   mm3, mm6					; High bytes to words

        // Average Ref1 and Ref2
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm3                    ; Second 4 values
        psrlw       mm0, 1
        psrlw       mm1, 1

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data reference and difference data
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm4                    ; Second 4 values

        // Pack and store
        mov         ecx,dword ptr [TmpDataPtr]   
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [ecx+24],mm0      ; Write the data out to the temporary results buffer
        add         eax,edx                     ; Step the reference pointers
        add         ebx,edx                    

        // Row 5
        // Load the change pointer
        mov         ecx,dword ptr [ChangePtr]   
        add         ecx,64                 

        // Load the data values (Ref1 and Ref2). 
        movq        mm0,dword ptr [eax]         ; Load 8 elements of source data
        movq        mm2,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
        movq        mm3, mm2                    ; Copy data

		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words
		punpcklbw   mm2, mm6					; Low bytes to words
		punpckhbw   mm3, mm6					; High bytes to words

        // Average Ref1 and Ref2
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm3                    ; Second 4 values
        psrlw       mm0, 1
        psrlw       mm1, 1

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data reference and difference data
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm4                    ; Second 4 values

        // Pack and store
        mov         ecx,dword ptr [TmpDataPtr]   
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [ecx+32],mm0      ; Write the data out to the temporary results buffer
        add         eax,edx                     ; Step the reference pointers
        add         ebx,edx                    

        // Row 6
        // Load the change pointer
        mov         ecx,dword ptr [ChangePtr]   
        add         ecx,80                    

        // Load the data values (Ref1 and Ref2). 
        movq        mm0,dword ptr [eax]         ; Load 8 elements of source data
        movq        mm2,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
        movq        mm3, mm2                    ; Copy data

		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words
		punpcklbw   mm2, mm6					; Low bytes to words
		punpckhbw   mm3, mm6					; High bytes to words

        // Average Ref1 and Ref2
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm3                    ; Second 4 values
        psrlw       mm0, 1
        psrlw       mm1, 1

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data reference and difference data
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm4                    ; Second 4 values

        // Pack and store
        mov         ecx,dword ptr [TmpDataPtr]   
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [ecx+40],mm0      ; Write the data out to the temporary results buffer
        add         eax,edx                     ; Step the reference pointers
        add         ebx,edx                    

        // Row 7
        // Load the change pointer
        mov         ecx,dword ptr [ChangePtr]   
        add         ecx,96                    

        // Load the data values (Ref1 and Ref2). 
        movq        mm0,dword ptr [eax]         ; Load 8 elements of source data
        movq        mm2,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
        movq        mm3, mm2                    ; Copy data

		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words
		punpcklbw   mm2, mm6					; Low bytes to words
		punpckhbw   mm3, mm6					; High bytes to words

        // Average Ref1 and Ref2
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm3                    ; Second 4 values
        psrlw       mm0, 1
        psrlw       mm1, 1

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data reference and difference data
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm4                    ; Second 4 values

        // Pack and store
        mov         ecx,dword ptr [TmpDataPtr]   
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [ecx+48],mm0      ; Write the data out to the temporary results buffer
        add         eax,edx                     ; Step the reference pointers
        add         ebx,edx                    

        // Row 8
        // Load the change pointer
        mov         ecx,dword ptr [ChangePtr]   
        add         ecx,112                    

        // Load the data values (Ref1 and Ref2). 
        movq        mm0,dword ptr [eax]         ; Load 8 elements of source data
        movq        mm2,dword ptr [ebx]         ; Load 8 elements of source data
        movq        mm1, mm0                    ; Copy data
        movq        mm3, mm2                    ; Copy data

		punpcklbw   mm0, mm6					; Low bytes to words
		punpckhbw   mm1, mm6					; High bytes to words
		punpcklbw   mm2, mm6					; Low bytes to words
		punpckhbw   mm3, mm6					; High bytes to words

        // Average Ref1 and Ref2
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm3                    ; Second 4 values
        psrlw       mm0, 1
        psrlw       mm1, 1

        // Load 8 elements of 16 bit change data
        movq        mm2,dword ptr [ecx]         ; Load 4 elements of change data
        movq        mm4,dword ptr [ecx+8]       ; Load next 4 elements of change data

        // Sum the data reference and difference data
        paddw       mm0, mm2                    ; First 4 values
        paddw       mm1, mm4                    ; Second 4 values

        // Pack and store
        mov         ecx,dword ptr [TmpDataPtr]   
        packuswb    mm0, mm1                    ; Then pack and saturate to unsigned bytes
        movq        dword ptr [ecx+56],mm0      ; Write the data out to the temporary results buffer


        // Now copy the results back to the reconstruction buffer.
        mov         eax,dword ptr [ReconPtr]    ; Load the reconstruction Pointer  
        mov         ecx,dword ptr [TmpDataPtr]  ; Load the temp results pointer 
        // Row 1
        movq        mm0,dword ptr [ecx]         ; Load 8 elements of results data
        movq        dword ptr [eax],mm0         ; Write the data tot he reconstruction buffer.
        add         eax,edx                     ; Step the reconstruction pointer
        // Row 2
        movq        mm0,dword ptr [ecx+8]       ; Load 8 elements of results data
        movq        dword ptr [eax],mm0         ; Write the data tot he reconstruction buffer.
        add         eax,edx                     ; Step the reconstruction pointer
        // Row 3
        movq        mm0,dword ptr [ecx+16]      ; Load 8 elements of results data
        movq        dword ptr [eax],mm0         ; Write the data tot he reconstruction buffer.
        add         eax,edx                     ; Step the reconstruction pointer
        // Row 4
        movq        mm0,dword ptr [ecx+24]      ; Load 8 elements of results data
        movq        dword ptr [eax],mm0         ; Write the data tot he reconstruction buffer.
        add         eax,edx                     ; Step the reconstruction pointer
        // Row 5
        movq        mm0,dword ptr [ecx+32]      ; Load 8 elements of results data
        movq        dword ptr [eax],mm0         ; Write the data tot he reconstruction buffer.
        add         eax,edx                     ; Step the reconstruction pointer
        // Row 6
        movq        mm0,dword ptr [ecx+40]      ; Load 8 elements of results data
        movq        dword ptr [eax],mm0         ; Write the data tot he reconstruction buffer.
        add         eax,edx                     ; Step the reconstruction pointer
        // Row 7
        movq        mm0,dword ptr [ecx+48]      ; Load 8 elements of results data
        movq        dword ptr [eax],mm0         ; Write the data tot he reconstruction buffer.
        add         eax,edx                     ; Step the reconstruction pointer
        // Row 8
        movq        mm0,dword ptr [ecx+56]      ; Load 8 elements of results data
        movq        dword ptr [eax],mm0         ; Write the data tot he reconstruction buffer.
        add         eax,edx                     ; Step the reconstruction pointer

        //emms
    }
}
#endif

