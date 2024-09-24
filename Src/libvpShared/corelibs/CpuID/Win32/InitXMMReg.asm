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
; **-InitXMMReg
;
; This function is meant to be run on a Windows NT system to
; try and determine if the OS supports the XMM registers or
; not.
;
; This function is number 1 in a set of three.  The other
; functions are...
;
;    TrashXMMReg
;    VerifyXMMReg
;
; Assumptions:
;   None
;
; Input:
;   None
;
; Output:
;  No return value.  But XMM registers 
;  0, 1, 2 initilized to a predetermined
;  value
;
;
        .686P
		.XMM
        .MODEL  flat, SYSCALL, os_dos
        .DATA 

TORQ_CX_DATA SEGMENT PAGE PUBLIC USE32 'DATA' 

        ALIGN 32

PUBLIC XMM0Init
PUBLIC XMM1Init
PUBLIC XMM2Init


    XMM0Init    REAL4   1.1
                REAL4   2.2
                REAL4   3.3
                REAL4   4.4
                        
    XMM1Init    REAL4   5.5
                REAL4   6.6
                REAL4   7.7
                REAL4   8.8
                        
    XMM2Init    REAL4   9.9
                REAL4   10.10
                REAL4   11.11
                REAL4   12.12


NAME InitXMMReg

PUBLIC InitXMMReg_
PUBLIC _InitXMMReg

        .CODE

; void InitXMMReg( void )
InitXMMReg_:
_InitXMMReg:
    push    esi ;safety sh*&
    push    edi
    push    ebp
    push    ebx 
    push    ecx
    push    edx

    movaps  xmm0,XMM0Init
    movaps  xmm1,XMM1Init
    movaps  xmm2,XMM2Init

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
