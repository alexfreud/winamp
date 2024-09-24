;
; **-MmxEncodeMath.asm
;
; MMX versions of SUB8, SUB8_AV2, SUB8 with fixed subtract of 128
;
;******************************************************************
;	Revision History
;	
;	1.01	JBB	 23-Mar-01  Fixed frame	updating for preprocessor
;	1.00	YWX	 dd-mmm-yy	Configuration baseline from Jong Chen's code
;
;******************************************************************



        .586
        .387
        .MODEL  flat, SYSCALL, os_dos
        .MMX

; macros

        .DATA
TORQ_CX_DATA SEGMENT PAGE PUBLIC USE32 'DATA' 
        ALIGN 32

; local constants go here
OneTwentyEight    QWORD  00080008000800080h

@CurSeg ENDS


;
; external variables
;

; external variables go here

 

; structures
SUB8Params  STRUC
                    dd  6 dup (?)   ;6 pushed regs
                    dd  ?           ;return address
    FiltPtr         dd  ?
    ReconPtr        dd  ?
    DctInputPtr     dd  ?
    old_ptr1        dd  ?
    new_ptr1        dd  ?
    PixelsPerLine   dd  ?
    ReconPixelsPerLine dd ?
SUB8Params  ENDS

SUB8_128Params  STRUC
                    dd  6 dup (?)   ;6 pushed regs
                    dd  ?           ;return address
    FiltPtr2        dd  ?
    DctInputPtr2    dd  ?
    old_ptr12       dd  ?
    new_ptr12       dd  ?
    PixelsPerLine2  dd  ?
SUB8_128Params  ENDS

SUB8AV2Params  STRUC
                    dd  6 dup (?)   ;6 pushed regs
                    dd  ?           ;return address
    FiltPtr         dd  ?
    ReconPtr1       dd  ?
    ReconPtr2       dd  ?
    DctInputPtr     dd  ?
    old_ptr1        dd  ?
    new_ptr1        dd  ?
    PixelsPerLine   dd  ?
    ReconPixelsPerLine dd ?
SUB8AV2Params  ENDS

;
; macro functions
;
SUB8Calc8Bytes MACRO Index:REQ
    movq        mm0,[eax]                   ; mm0 = FiltPtr
    movq        mm1,[ebx]                   ; mm1 = ReconPtr
    movq        mm2,mm0                     ; dup to prepare for up conversion
    movq        mm3,mm1                     ; dup to prepare for up conversion

    ; convert from UINT8 to INT16
;    movq        mm6,[esi]
    punpcklbw   mm0,mm7                     ; mm0 = INT16(FiltPtr)
    punpcklbw   mm1,mm7                     ; mm1 = INT16(ReconPtr)
    punpckhbw   mm2,mm7                     ; mm2 = INT16(FiltPtr)
    punpckhbw   mm3,mm7                     ; mm3 = INT16(ReconPtr)

    ; start calculation
    psubw       mm0,mm1                     ; mm0 = FiltPtr - ReconPtr
    psubw       mm2,mm3                     ; mm2 = FiltPtr - ReconPtr

    ; Update the screen canvas in one step
    ;memcpy( old_ptr1, new_ptr1, BLOCK_HEIGHT_WIDTH ); 
;    movq        [edx],mm6
;     add         edx,edi
;     add         esi,edi

    movq        [ecx+Index],mm0             ; write answer out
    movq        [ecx+Index+8],mm2           ; write answer out

    ; Increment pointers
    add         eax,edi
    add         ebx,ebp
ENDM

;
; **-SUB8_128Calc8Bytes
;
SUB8_128Calc8Bytes MACRO Index:REQ
    movq        mm0,[eax]                   ; mm0 = FiltPtr
    movq        mm2,mm0                     ; dup to prepare for up conversion

    ; convert from UINT8 to INT16
;    movq        mm6,[esi]
    punpcklbw   mm0,mm7                     ; mm0 = INT16(FiltPtr)
    punpckhbw   mm2,mm7                     ; mm2 = INT16(FiltPtr)

    ; start calculation
    psubw       mm0,mm1                     ; mm0 = FiltPtr - 128
    psubw       mm2,mm1                     ; mm2 = FiltPtr - 128

    ; Update the screen canvas in one step
    ;memcpy( old_ptr1, new_ptr1, BLOCK_HEIGHT_WIDTH ); 
;    movq        [edx],mm6
;    add         edx,edi
;    add         esi,edi

    movq        [ecx+Index],mm0             ; write answer out
    movq        [ecx+Index+8],mm2           ; write answer out

    ; Increment pointers
    add         eax,edi
ENDM

;
; **-SUB8AV2Calc8Bytes
;
SUB8AV2Calc8Bytes MACRO Index:REQ
    movq        mm0,[eax]                   ; mm0 = FiltPtr
    movq        mm1,[ebx]                   ; mm1 = ReconPtr1
    movq        mm4,[ebp]                   ; mm4 = ReconPtr2
    movq        mm2,mm0                     ; dup to prepare for up conversion
    movq        mm3,mm1                     ; dup to prepare for up conversion
    movq        mm5,mm4                     ; dup to prepere for up conversion

    ; convert from UINT8 to INT16
;    movq        mm6,[esi]
    punpcklbw   mm0,mm7                     ; mm0 = INT16(FiltPtr)
    punpcklbw   mm1,mm7                     ; mm1 = INT16(ReconPtr1)
    punpcklbw   mm4,mm7                     ; mm4 = INT16(ReconPtr2)

    punpckhbw   mm2,mm7                     ; mm2 = INT16(FiltPtr)
    punpckhbw   mm3,mm7                     ; mm3 = INT16(ReconPtr1)
    punpckhbw   mm5,mm7                     ; mm5 = INT16(ReconPtr2)

    ; average ReconPtr1 and ReconPtr2
    paddw       mm1,mm4                     ; mm1 = ReconPtr1 + ReconPtr2
    paddw       mm3,mm5                     ; mm3 = ReconPtr1 + ReconPtr2
    psrlw       mm1,1                       ; mm1 = (ReconPtr1 + ReconPtr2) / 2
    psrlw       mm3,1                       ; mm3 = (ReconPtr1 + ReconPtr2) / 2

    psubw       mm0,mm1                     ; mm0 = FiltPtr - ((ReconPtr1 + ReconPtr2) / 2)
    psubw       mm2,mm3                     ; mm2 = FiltPtr - ((ReconPtr1 + ReconPtr2) / 2)

    ; Update the screen canvas in one step
    ;memcpy( old_ptr1, new_ptr1, BLOCK_HEIGHT_WIDTH ); 
;    movq        [edx],mm6
;    add         edx,edi
;    add         esi,edi

    movq        [ecx+Index],mm0             ; write answer out
    movq        [ecx+Index+8],mm2           ; write answer out

    ; Increment pointers
    add         eax,edi
    add         ebx,(SUB8AV2Params PTR [esp]).ReconPixelsPerLine
    add         ebp,(SUB8AV2Params PTR [esp]).ReconPixelsPerLine
ENDM

;------------------------------------------------
; local vars
LOCAL_SPACE     EQU 0


;
; **-MmxSUB8
;
; Input:
;   FiltPtr
;   ReconPtr
;   DctInputPtr
;   old_ptr1
;   new_ptr1
;
; Output:
;
;------------------------------------------------
; void MmxSUB8( UINT8 *FiltPtr, UINT8 *ReconPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, 
;               INT32 PixelsPerLine, INT32 ReconPixelsPerLine )
;
        .CODE

NAME MmxSUB8

PUBLIC MmxSUB8_
PUBLIC _MmxSUB8
MmxSUB8_:
_MmxSUB8:

    push    ecx
    push    ebx 
    push    edx
    push    esi
    push    edi
    push    ebp
    

	mov         eax,(SUB8Params PTR [esp]).FiltPtr
    mov         ebx,(SUB8Params PTR [esp]).ReconPtr
    mov         ecx,(SUB8Params PTR [esp]).DctInputPtr
;    mov         edx,(SUB8Params PTR [esp]).old_ptr1
;    mov         esi,(SUB8Params PTR [esp]).new_ptr1
    mov         edi,(SUB8Params PTR [esp]).PixelsPerLine
    mov         ebp,(SUB8Params PTR [esp]).ReconPixelsPerLine

    pxor        mm7,mm7                     ; clear mm7 for up precision conversion

    LoopCtr = 0
WHILE LoopCtr LT 128
    SUB8Calc8Bytes <LoopCtr>
    LoopCtr = LoopCtr + 16
ENDM

theExit1:
        pop     ebp
        pop     edi
        pop     esi
        pop     edx
        pop     ebx
        pop     ecx


    ret

;
; **-MmxSUB8_128
;
; Input:
;   FiltPtr
;   ReconPtr
;   DctInputPtr
;   old_ptr1
;   new_ptr1
;
; Output:
;
;------------------------------------------------
; void MmxSUB8_128( UINT8 *FiltPtr, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, 
;               INT32 PixelsPerLine )
;
        .CODE

NAME MmxSUB8_128

PUBLIC MmxSUB8_128_
PUBLIC _MmxSUB8_128
MmxSUB8_128_:
_MmxSUB8_128:

    push    ecx
    push    ebx 
    push    edx
    push    esi
    push    edi
    push    ebp
    

	mov         eax,(SUB8_128Params PTR [esp]).FiltPtr2
    mov         ecx,(SUB8_128Params PTR [esp]).DctInputPtr2
;    mov         edx,(SUB8_128Params PTR [esp]).old_ptr12
;    mov         esi,(SUB8_128Params PTR [esp]).new_ptr12
    mov         edi,(SUB8_128Params PTR [esp]).PixelsPerLine2

    movq        mm1,OneTwentyEight          ; load value to subtract with
    pxor        mm7,mm7                     ; clear mm7 for up precision conversion

    LoopCtr = 0
WHILE LoopCtr LT 128
    SUB8_128Calc8Bytes <LoopCtr>
    LoopCtr = LoopCtr + 16
ENDM

theExit3:
        pop     ebp
        pop     edi
        pop     esi
        pop     edx
        pop     ebx
        pop     ecx


    ret

;
; **-MmxSUB8AV2
;
; Input:
;   FiltPtr
;   ReconPtr
;   DctInputPtr
;   old_ptr1
;   new_ptr1
;
; Output:
;
;------------------------------------------------
; void MmxSUB8AV2( UINT8 *FiltPtr, UINT8 *ReconPtr1, UINT8 *ReconPtr1, INT16 *DctInputPtr, UINT8 *old_ptr1, UINT8 *new_ptr1, 
;                  INT32 PixelsPerLine, INT32 ReconPixelsPerLine )
;
        .CODE

NAME MmxSUB8AV2

PUBLIC MmxSUB8AV2_
PUBLIC _MmxSUB8AV2
MmxSUB8AV2_:
_MmxSUB8AV2:

    push    ecx
    push    ebx 
    push    edx
    push    esi
    push    edi
    push    ebp
    

	mov         eax,(SUB8AV2Params PTR [esp]).FiltPtr
    mov         ebx,(SUB8AV2Params PTR [esp]).ReconPtr1
    mov         ecx,(SUB8AV2Params PTR [esp]).DctInputPtr
;   mov         edx,(SUB8AV2Params PTR [esp]).old_ptr1
;   mov         esi,(SUB8AV2Params PTR [esp]).new_ptr1
    mov         edi,(SUB8AV2Params PTR [esp]).PixelsPerLine
    mov         ebp,(SUB8AV2Params PTR [esp]).ReconPtr2

    pxor        mm7,mm7                     ; clear mm7 for up precision conversion

    LoopCtr = 0
WHILE LoopCtr LT 128
    SUB8AV2Calc8Bytes <LoopCtr>
    LoopCtr = LoopCtr + 16
ENDM

theExit2:
        pop     ebp
        pop     edi
        pop     esi
        pop     edx
        pop     ebx
        pop     ecx


    ret

;************************************************
        END

