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
; **-TrashXMMReg
;
; This function is meant to be run on a Windows NT system to
; try and determine if the OS supports the XMM registers or
; not.
;
; This function is number 2 in a set of three.  The other
; functions are...
;
;    InitXMMReg
;    VerifyXMMReg
;
; Assumptions:
;   No necessary for this function to work properly but
;   IntiXMMReg should have been called to initilize the
;   XMM registers to a predetermined value
;
; Input:
;   None
;
; Output:
;  No return value.  But XMM registers 
;  0, 1, 2 written to 0's
;
;

        .686P
		.XMM
        .MODEL  flat, SYSCALL, os_dos
        .DATA 

TORQ_CX_DATA SEGMENT PAGE PUBLIC USE32 'DATA' 

         ALIGN 32

    Zeros       REAL4   0.0
                REAL4   0.0
                REAL4   0.0
                REAL4   0.0


NAME TrashXMMReg

PUBLIC TrashXMMReg_
PUBLIC _TrashXMMReg

        .CODE

; void TrashXMMReg( void )
TrashXMMReg_:
_TrashXMMReg:
    push    esi ;safety sh*&
    push    edi
    push    ebp
    push    ebx 
    push    ecx
    push    edx

    movaps  xmm0,Zeros
    movaps  xmm1,Zeros
    movaps  xmm2,Zeros

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
