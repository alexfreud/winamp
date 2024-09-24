;//==========================================================================
;//
;//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
;//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
;//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
;//  PURPOSE.
;//
;//  Copyright (c) 1999 - 2001  On2 Technologies Inc. All Rights Reserved.
;//
;//--------------------------------------------------------------------------



;
; **-VerifyXMMReg
;
; This function is meant to be run on a Windows NT system to
; try and determine if the OS supports the XMM registers or
; not.
;
; This function is number 3 in a set of three.  The other
; functions are...
;
;    InitXMMReg
;    TrashXMMReg
;
; Assumptions:
;   Assumes that InitXMMReg was called to initilize the XMM registers.
;   Assumes that TrashXMMReg was called from a different thread to clear
;   the values in the XMM registers.
;
; Input:
;   None
;
; Output:
;   Return 1 (True) if the XMM registers are at the correct values.
;   (os supports XMM registers)
;
;   Return 0 (False) if the XMM registers are not at the correct values.
;   (os does not support the XMM registers)
;

        .686P
		.XMM
        .MODEL  flat, SYSCALL, os_dos
        .DATA 

TORQ_CX_DATA SEGMENT PAGE PUBLIC USE32 'DATA' 

         ALIGN 32


NAME VerifyXMMReg

PUBLIC VerifyXMMReg_
PUBLIC _VerifyXMMReg


EXTERN XMM0Init:REAL4
EXTERN XMM1Init:REAL4
EXTERN XMM2Init:REAL4


        .CODE

; int VerifyXMMReg( void )
VerifyXMMReg_:
_VerifyXMMReg:
    push    esi ;safety sh*&
    push    edi
    push    ebp
    push    ebx 
    push    ecx
    push    edx

    mov     eax,0                       ; assume will fail

    comiss  xmm0,XMM0Init               ; check XMM0
    jne     Exit

    comiss  xmm1,XMM1Init
    jne     Exit

    comiss  xmm2,XMM2Init
    jne     Exit

    mov     eax,1                       ; OS supports XMM registers

Exit:
    pop     edx ;safety sh*&
    pop     ecx
    pop     ebx
    pop     ebp
    pop     edi
    pop     esi
    ret

;************************************************
         END
