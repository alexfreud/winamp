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
; **-Does9xOSSupportXMM
;
; This function will verify if the operating system supports the XMM
; instructions.  According to Intel documentation 
;
;       Intel Architecture
;       Software Developer
;       Manual
;       Volume 1:
;       Basic Architecture
;
; The following needs to be true for the OS to suppor the XMM instructions
;
;   CR0.EM(bit 2) = 0 (emulation disabled)
;   CR4.OSFXSR(bit 9) = 1 (OS supports saving SIMD floating-point state during context
;                          switches)
;
;  * * * N O T E * * * * * * N O T E * * * * * * N O T E * * * * * * N O T E * * * * * * N O T E * * * * * * N O T E * * *
; 
; This function will NOT run on windows NT systems.  The function reads control registers
; which are protected under Windows NT.  If you attempt to run this function under Windows NT a
; protected mode access violation will be generated.
;
;  * * * N O T E * * * * * * N O T E * * * * * * N O T E * * * * * * N O T E * * * * * * N O T E * * * * * * N O T E * * *
;
; Assumptions:
;  Access to system control registers CR0 and CR4 are not protected
;
; Input:
;   None
;
; Output:
;   1 Returned if OS supports XMM instructions
;   0 Returned if OS does not support XMM instructions
;
;


        .586
        .MODEL  flat, SYSCALL, os_dos
        .DATA 

NAME x86cpuid

PUBLIC Does9xOSSupportXMM_
PUBLIC _Does9xOSSupportXMM

        .CODE

; int Does9xOSSupportXMM( void )
Does9xOSSupportXMM_:
_Does9xOSSupportXMM:
    push    esi ;safety sh*&
    push    edi
    push    ebp
    push    ebx 
    push    ecx
    push    edx

; check to see if OS supports SIMD instructions
    mov     edx,cr0
    bt      edx,2                           ; ensure no emulation
    jnae    NoXMMSupport

    mov     edx,cr4
    bt      edx,9                           ; OS support SIMD
    jnc     NoXMMSupport

; we support XMM instructions
    mov     eax,1
    jmp     Exit

NoXMMSupport:
;    mov     eax,0                           ; OS does not support XMM instructions

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
