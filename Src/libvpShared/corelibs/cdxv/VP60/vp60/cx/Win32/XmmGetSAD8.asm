;------------------------------------------------
XmmGetSAD8Params  STRUC
                    dd  6 dup (?)   ;6 pushed regs
                    dd  ?           ;return address
    NewDataPtr      dd  ?
    RefDataPtr      dd  ?
	OffsetN			dd  ?
	OffsetR			dd	?
XmmGetSAD8Params  ENDS
;------------------------------------------------

 
        .686P
        .387
        .MODEL  flat, SYSCALL, os_dos
        .XMM

; macros


        .DATA
TORQ_CX_DATA SEGMENT PAGE PUBLIC USE32 'DATA' 

        ALIGN 32


        .CODE

NAME XmmGetSAD8

PUBLIC XmmGetSAD8_
PUBLIC _XmmGetSAD8


;------------------------------------------------
; local vars
LOCAL_SPACE     EQU 0


;------------------------------------------------
;INT32 XmmGetSAD8( UINT8 * NewDataPtr, UINT8  * RefDataPtr, 
;					INT32 OffsetN, INT32 OffsetR) 
;
XmmGetSAD8_:
_XmmGetSAD8:

   push    ecx
    push    ebx 
    push    edx

    push    esi
	mov         ecx,(XmmGetSAD8Params PTR [esp-8]).OffsetN
	mov         eax,(XmmGetSAD8Params PTR [esp-8]).NewDataPtr	; Load base addresses

    push    edi
	mov         ebx,(XmmGetSAD8Params PTR [esp-4]).RefDataPtr
    mov         edx,(XmmGetSAD8Params PTR [esp-4]).OffsetR

    push    ebp
    

;
; ESP = Stack Pointer                      MM0 = Free
; ESI = Free                               MM1 = Free
; EDI = Free                               MM2 = Free
; EBP = Free                               MM3 = Free
; EBX = RefDataPtr                         MM4 = Free
; ECX = OffsetN		                       MM5 = Free
; EDX =	OffsetR							   MM6 = Free
; EAX = NewDataPtr                         MM7 = Free
;


        ; Row 1
		movq		mm0, [eax]					; Copy eight bytes to mm0
		add         eax,ecx						; Inc pointer into the new data
        psadbw      mm0, [ebx]

		add         ebx,edx						; Inc pointer into ref data

        ; Row 2
		movq		mm1, [eax]					; Copy eight bytes to mm0
		add         eax,ecx						; Inc pointer into the new data
        psadbw      mm1, [ebx]

		add         ebx,edx						; Inc pointer into ref data

        ; Row 3
		movq		mm2, [eax]					; Copy eight bytes to mm0
		add         eax,ecx						; Inc pointer into the new data
        psadbw      mm2, [ebx]

		add         ebx,edx						; Inc pointer into ref data

        ; Row 4
		movq		mm3, [eax]					; Copy eight bytes to mm0
		add         eax,ecx						; Inc pointer into the new data
        psadbw      mm3, [ebx]

		add         ebx,edx						; Inc pointer into ref data

        ; Row 5
		movq		mm4, [eax]					; Copy eight bytes to mm0
		add         eax,ecx						; Inc pointer into the new data
        psadbw      mm4, [ebx]

		add         ebx,edx						; Inc pointer into ref data

        ; Row 6
		movq		mm5, [eax]					; Copy eight bytes to mm0
		add         eax,ecx						; Inc pointer into the new data
        psadbw      mm5, [ebx]

		add         ebx,edx						; Inc pointer into ref data

        ; Row 7
		movq		mm6, [eax]					; Copy eight bytes to mm0
		add         eax,ecx						; Inc pointer into the new data
        psadbw      mm6, [ebx]

		add         ebx,edx						; Inc pointer into ref data

        ; Row 8
		movq		mm7, [eax]					; Copy eight bytes to mm0
        psadbw      mm7, [ebx]

        ; start accumulating differences
        paddd       mm0,mm1
        paddd       mm2,mm3

        pop     ebp
        paddd       mm4,mm5
        paddd       mm6,mm7

        pop     edi
        paddd       mm0,mm2
        paddd       mm4,mm6

        pop     esi
        paddd       mm0,mm4
        movd        ecx,mm0

theExit:
        pop     edx
    	mov         eax, ecx                     ; add in calculated error

        pop     ebx
        pop     ecx

	    ret

;************************************************
        END
