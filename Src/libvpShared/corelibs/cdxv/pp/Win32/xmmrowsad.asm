;------------------------------------------------
XmmRowSADParams  STRUC
                    dd  ?			;1 pushed regs
                    dd  ?           ;return address
    NewDataPtr      dd  ?
    RefDataPtr      dd  ?
XmmRowSADParams  ENDS
;------------------------------------------------

INCLUDE iaxmm.inc
 
        .586
        .387
        .MODEL  flat, SYSCALL, os_dos
        .MMX

; macros


        .DATA
TORQ_CX_DATA SEGMENT PAGE PUBLIC USE32 'DATA' 

        ALIGN 32


        .CODE

NAME XmmRowSAD

PUBLIC XmmRowSAD_
PUBLIC _XmmRowSAD
 
 

;------------------------------------------------
; local vars
LOCAL_SPACE     EQU 0


;------------------------------------------------
;UINT32 XmmRowSAD( UINT8 * NewDataPtr, UINT8  * RefDataPtr) 
;
XmmRowSAD_:
_XmmRowSAD:

    push    ebx 
	mov         eax,(XmmRowSADParams PTR [esp]).NewDataPtr	; Load base addresses
	mov         ebx,(XmmRowSADParams PTR [esp]).RefDataPtr
    
;
; ESP = Stack Pointer                      MM0 = Free
; ESI = Free                               MM1 = Free
; EDI = Free                               MM2 = Free
; EBP = Free                               MM3 = Free
; EBX = RefDataPtr                         MM4 = Free
; ECX = PixelsPerLine                      MM5 = Free
; EDX = PixelsPerLine + STRIDE_EXTRA       MM6 = Free
; EAX = NewDataPtr                         MM7 = Free
;


		movq		mm0, QWORD PTR [eax]		; copy eight bytes from NewDataPtr to mm0
		movq		mm3, QWORD PTR [ebx]		; copy eight bytes from ReconDataPtr to mm3
		
		pxor		mm1, mm1					; clear mm1 for unpacking

		movq		mm2, mm0					; make a copy
		movq		mm4, mm3					; make a copy 

		punpcklbw	mm0, mm1					; unpack the lower four bytes
		punpcklbw   mm3, mm1					; unpack the lower four bytes

		psadbw		mm0, mm3					; sum of absolute difference of four bytes
		punpckhbw   mm2, mm1					; unpack the higher four bytes
		punpckhbw   mm4, mm1					; unpack the higher four bytes

		psadbw		mm2, mm4					; sum of absolute difference of another four

        pop     ebx
		pmaxsw		mm0, mm2					; get the max
		movd		eax, mm0					; return value

    ret

;************************************************
        END

END