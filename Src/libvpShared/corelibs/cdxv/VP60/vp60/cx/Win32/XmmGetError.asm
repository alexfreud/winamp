; structures
XmmGetErrorParams  STRUC
                    dd  6 dup (?)   ;6 pushed regs
                    dd  ?           ;return address
    NewDataPtr		dd  ?
    PixelsPerLine   dd  ?
    ReconPtr1       dd  ?
    ReconPixelsPerLine   dd  ?
	XSum			dd	?
	XXSum			dd	?
XmmGetErrorParams  ENDS


 
        .686P
        .387
        .MODEL  flat, SYSCALL, os_dos
        .XMM

; macros

        .DATA
TORQ_CX_DATA SEGMENT PAGE PUBLIC USE32 'DATA' 

        ALIGN 32


        .CODE

NAME XmmGetError

PUBLIC XmmGetError_
PUBLIC _XmmGetError
 

;------------------------------------------------
; local vars
LOCAL_SPACE     EQU 0

;------------------------------------------------
; 		XmmGetError(UINT8*	NewDataPtr, 
;					UINT32	PixelsPerLine, 
;					UINT8*	RefDataPtr1,
;					UINT32	RefPixelsPerLine, 
;					INT32*	XSum, 
;					INT32*	XXSum)
		
XmmGetError_:
_XmmGetError:

		push    ecx
	    push    ebx 
	    push    edx
	    push    esi

	    mov         ecx,(XmmGetErrorParams PTR [esp-8]).PixelsPerLine
		mov         eax,(XmmGetErrorParams PTR [esp-8]).NewDataPtr

	    push    edi

	    mov	        ebx,(XmmGetErrorParams PTR [esp-4]).ReconPtr1
		mov			edx,(XmmGetErrorParams PTR [esp-4]).ReconPixelsPerLine

	    push    ebp
	
		mov			esi,(XmmGetErrorParams PTR [esp]).XSum
		mov			edi,(XmmGetErrorParams PTR [esp]).XXSum

	    prefetcht0	[eax+ecx]
		prefetcht0	[ebx+edx]
		
		pxor        mm5, mm5					; Blank mmx6
	    pxor        mm6, mm6					; Blank mmx7

		;Row 1
		
		movq		mm1, [ebx]					; Copy eight bytes to mm1
		movq		mm0, [eax]					; Copy eight bytes to mm0

	    pxor        mm7, mm7					; Blank mmx7
		
		prefetcht0	[eax+ecx*2]
		prefetcht0	[ebx+edx*2]
		movq		mm2, mm0					; Take copies
		movq		mm3, mm1					; Take copies

		punpcklbw   mm0, mm6					; unpack to higher precision
		punpcklbw   mm1, mm6					
		punpckhbw   mm2, mm6					; unpack to higher precision
		punpckhbw   mm3, mm6					
        psubsw		mm0, mm1					; A-B (low order) to MM0
        psubsw		mm2, mm3					; A-B (high order) to MM2

		paddw       mm5, mm0					; accumulate differences in mm5
		paddw       mm5, mm2					; accumulate differences in mm5

		pmaddwd     mm0, mm0					; square and accumulate
		pmaddwd     mm2, mm2					; square and accumulate
		add         ebx,edx						; Inc pointer into ref data
	    add         eax,ecx						; Inc pointer into the new data
	    movq		mm1, [ebx]					; Copy eight bytes to mm1
		prefetcht0	[ebx+edx*2]
	    paddd       mm7, mm0					; accumulate in mm7
	    paddd       mm7, mm2					; accumulate in mm7


        ; Row 2
	    movq		mm0, [eax]					; Copy eight bytes to mm0
	    prefetcht0	[eax+ecx*2]
		movq		mm2, mm0					; Take copies
	    movq		mm3, mm1					; Take copies

	    punpcklbw   mm0, mm6					; unpack to higher precision
	    punpcklbw   mm1, mm6					
	    punpckhbw   mm2, mm6					; unpack to higher precision
	    punpckhbw   mm3, mm6					
        psubsw		mm0, mm1					; A-B (low order) to MM0
        psubsw		mm2, mm3					; A-B (high order) to MM2

	    paddw       mm5, mm0					; accumulate differences in mm5
	    paddw       mm5, mm2					; accumulate differences in mm5

	    pmaddwd     mm0, mm0					; square and accumulate
	    pmaddwd     mm2, mm2					; square and accumulate
	    add         ebx,edx						; Inc pointer into ref data
	    add         eax,ecx						; Inc pointer into the new data
	    movq		mm1, [ebx]					; Copy eight bytes to mm1
		prefetcht0	[ebx+edx*2]
	    paddd       mm7, mm0					; accumulate in mm7
	    paddd       mm7, mm2					; accumulate in mm7

        ; Row 3
	    movq		mm0, [eax]					; Copy eight bytes to mm0
		prefetcht0	[eax+ecx*2]
	    movq		mm2, mm0					; Take copies
	    movq		mm3, mm1					; Take copies

	    punpcklbw   mm0, mm6					; unpack to higher precision
	    punpcklbw   mm1, mm6					
	    punpckhbw   mm2, mm6					; unpack to higher precision
	    punpckhbw   mm3, mm6					
        psubsw		mm0, mm1					; A-B (low order) to MM0
        psubsw		mm2, mm3					; A-B (high order) to MM2

	    paddw       mm5, mm0					; accumulate differences in mm5
	    paddw       mm5, mm2					; accumulate differences in mm5

	    pmaddwd     mm0, mm0					; square and accumulate
	    pmaddwd     mm2, mm2					; square and accumulate
	    add         ebx,edx						; Inc pointer into ref data
	    add         eax,ecx						; Inc pointer into the new data
	    movq		mm1, [ebx]					; Copy eight bytes to mm1
		prefetcht0	[ebx+edx*2]
	    paddd       mm7, mm0					; accumulate in mm7
	    paddd       mm7, mm2					; accumulate in mm7

        ; Row 4
	    movq		mm0, [eax]					; Copy eight bytes to mm0
		prefetcht0	[eax+ecx*2]
	    movq		mm2, mm0					; Take copies
	    movq		mm3, mm1					; Take copies

	    punpcklbw   mm0, mm6					; unpack to higher precision
	    punpcklbw   mm1, mm6					
	    punpckhbw   mm2, mm6					; unpack to higher precision
	    punpckhbw   mm3, mm6					
        psubsw		mm0, mm1					; A-B (low order) to MM0
        psubsw		mm2, mm3					; A-B (high order) to MM2

	    paddw       mm5, mm0					; accumulate differences in mm5
	    paddw       mm5, mm2					; accumulate differences in mm5

	    pmaddwd     mm0, mm0					; square and accumulate
	    pmaddwd     mm2, mm2					; square and accumulate
	    add         ebx,edx						; Inc pointer into ref data
	    add         eax,ecx						; Inc pointer into the new data
	    movq		mm1, [ebx]					; Copy eight bytes to mm1
		prefetcht0 [ebx+edx*2]
	    paddd       mm7, mm0					; accumulate in mm7
	    paddd       mm7, mm2					; accumulate in mm7

        ; Row 5
	    movq		mm0, [eax]					; Copy eight bytes to mm0
	    prefetcht0	[eax+ecx*2]
		movq		mm2, mm0					; Take copies
	    movq		mm3, mm1					; Take copies

	    punpcklbw   mm0, mm6					; unpack to higher precision
    	punpcklbw   mm1, mm6					
    	punpckhbw   mm2, mm6					; unpack to higher precision
	    punpckhbw   mm3, mm6					
        psubsw		mm0, mm1					; A-B (low order) to MM0
        psubsw		mm2, mm3					; A-B (high order) to MM2

	    paddw       mm5, mm0					; accumulate differences in mm5
	    paddw       mm5, mm2					; accumulate differences in mm5

	    pmaddwd     mm0, mm0					; square and accumulate
	    pmaddwd     mm2, mm2					; square and accumulate
	    add         ebx, edx						; Inc pointer into ref data
	    add         eax, ecx						; Inc pointer into the new data
	    movq		mm1, [ebx]					; Copy eight bytes to mm1
		prefetcht0 [ebx+edx*2]
	    paddd       mm7, mm0					; accumulate in mm7
	    paddd       mm7, mm2					; accumulate in mm7

        ; Row 6
	    movq		mm0, [eax]					; Copy eight bytes to mm0
	    prefetcht0	[eax+ecx*2]
		movq		mm2, mm0					; Take copies
	    movq		mm3, mm1					; Take copies

	    punpcklbw   mm0, mm6					; unpack to higher precision
	    punpcklbw   mm1, mm6					
	    punpckhbw   mm2, mm6					; unpack to higher precision
	    punpckhbw   mm3, mm6					
        psubsw		mm0, mm1					; A-B (low order) to MM0
        psubsw		mm2, mm3					; A-B (high order) to MM2

	    paddw       mm5, mm0					; accumulate differences in mm5
	    paddw       mm5, mm2					; accumulate differences in mm5

	    pmaddwd     mm0, mm0					; square and accumulate
	    pmaddwd     mm2, mm2					; square and accumulate
	    add         ebx,edx						; Inc pointer into ref data
	    add         eax,ecx						; Inc pointer into the new data
	    movq		mm1, [ebx]					; Copy eight bytes to mm1
		prefetcht0 [ebx+edx]
	    paddd       mm7, mm0					; accumulate in mm7
	    paddd       mm7, mm2					; accumulate in mm7

        ; Row 7
	    movq		mm0, [eax]					; Copy eight bytes to mm0
		prefetcht0	[eax+ecx]
		
	    movq		mm2, mm0					; Take copies
	    movq		mm3, mm1					; Take copies

	    punpcklbw   mm0, mm6					; unpack to higher precision
	    punpcklbw   mm1, mm6					
	    punpckhbw   mm2, mm6					; unpack to higher precision
	    punpckhbw   mm3, mm6					
        psubsw		mm0, mm1					; A-B (low order) to MM0
        psubsw		mm2, mm3					; A-B (high order) to MM2

	    paddw       mm5, mm0					; accumulate differences in mm5
	    paddw       mm5, mm2					; accumulate differences in mm5

	    pmaddwd     mm0, mm0					; square and accumulate
	    pmaddwd     mm2, mm2					; square and accumulate
	    add         ebx,edx						; Inc pointer into ref data
	    add         eax,ecx						; Inc pointer into the new data
	    movq		mm1, [ebx]					; Copy eight bytes to mm1
	    paddd       mm7, mm0					; accumulate in mm7
	    paddd       mm7, mm2					; accumulate in mm7

        ; Row 8
	    movq		mm0, [eax]					; Copy eight bytes to mm0
	    movq		mm2, mm0					; Take copies
	    movq		mm3, mm1					; Take copies

	    punpcklbw   mm0, mm6					; unpack to higher precision
	    punpcklbw   mm1, mm6					
	    punpckhbw   mm2, mm6					; unpack to higher precision
	    punpckhbw   mm3, mm6					
        psubsw		mm0, mm1					; A-B (low order) to MM0
        psubsw		mm2, mm3					; A-B (high order) to MM2

	    paddw       mm5, mm0					; accumulate differences in mm5
	    paddw       mm5, mm2					; accumulate differences in mm5

	    pmaddwd     mm0, mm0					; square and accumulate
	    pmaddwd     mm2, mm2					; square and accumulate
	    paddd       mm7, mm0					; accumulate in mm7
	    paddd       mm7, mm2					; accumulate in mm7


	    ; Now accumulate the final results.
		
		movq		mm4, mm5					; 
		punpcklwd	mm5, mm6		
		punpckhwd	mm4, mm6
		movq		mm0, mm7
		paddw		mm5, mm4

		punpckhdq	mm0, mm6
		punpckldq	mm7, mm6
		movq		mm4, mm5
		paddd		mm0, mm7	
		punpckhdq	mm4, mm6
		punpckldq	mm5, mm6
		movd		eax, mm0
		paddw	    mm4, mm5
		movd		ebp, mm4
		movsx		ebx, bp;

        pop     ebp
		mov		DWORD PTR [edi], eax			;XXSum
		mov		DWORD PTR [esi], ebx;			;XSum
        pop     edi
    	emms									; Clear the MMX state.        
		pop     esi
        pop     edx
        pop     ebx
        pop     ecx
		ret
;------------------------------------------------------------------------
		END