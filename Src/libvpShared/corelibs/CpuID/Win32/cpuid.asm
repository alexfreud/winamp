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
; **-getCPUType
;
; This function will return a code indicating the type of the processor
; that is in the system.  If the processor type is unknown the generic
; x86 (Intel 486) type is returned
;
; parts taken from intel's AP-485 
;
;put checks for cmov and mmx support ????
;
; Assumptions:
;  None
;
; Input:
;  None
;
; Output:
;  Code for CPU type returned.  See cpuidlib.h for the supported
;  types.
;



        .586
        .MODEL  flat, SYSCALL, os_dos
        .DATA 

NAME x86cpuid

PUBLIC getCPUType_
PUBLIC _getCPUType

CPU_ID MACRO 
    db 0fh                      ; Hardcoded CPUID instruction 
    db 0a2h 
ENDM

;see cpuidlib.h
X86         EQU 0                   ; /* 486, Pentium plain, or any other x86 compatible */
PMMX        EQU 1                   ; /* Pentium with MMX */
PPRO        EQU 2                   ; /* Pentium Pro */
PII         EQU 3                   ; /* Pentium II */
C6X86       EQU 4					
C6X86MX     EQU 5
AMDK63D     EQU 6
AMDK6       EQU 7
AMDK5       EQU 8
XMM         EQU 11
WMT			EQU 12					;/* Willamette */


_486        EQU 4h
PENT        EQU 50h
PENTMMX     EQU 54h
PENTPRO     EQU 61h
PENTII      EQU 63h
SIMD        EQU 25

AMD_K63D    EQU 58h
AMD_K6      EQU 56h
AMD_K5      EQU 50h             ; K5 has models 0 - 6

_6X86       EQU 52h
_6X86MX     EQU 60h


_vendor_id      db "------------" 
intel_id        db "GenuineIntel" 
amd_id          db "AuthenticAMD" 
cyrix_id        db "CyrixInstead" 

        .CODE

getCPUType_:
_getCPUType:
    push    esi ;safety sh*&
    push    edi
    push    ebp
    push    ebx 
    push    ecx
    push    edx

;------------------------------------------------
; Intel486 processor check 
; Checking for ability to set/clear ID flag (Bit 21) in EFLAGS 
; which indicates the presence of a processor with the CPUID 
; instruction.
;------------------------------------------------
check_80486: 
    pushfd                                  ; push original EFLAGS
    pop     eax                             ; get original EFLAGS 
    mov     ebp,X86                         ; rv
    mov     ecx, eax                        ; save original EFLAGS 
    xor     eax, 200000h                    ; flip ID bit in EFLAGS 
    push    eax                             ; save new EFLAGS value on stack 
    popfd                                   ; replace current EFLAGS value 
    pushfd                                  ; get new EFLAGS 
    pop     eax                             ; store new EFLAGS in EAX 
    xor     eax, ecx                        ; can not toggle ID bit, 
    je      end_cpu_type486                 ; processor=80486

;------------------------------------------------
; Execute CPUID instruction to not determine vendor, family, 
; model, stepping and features. For the purpose of this 
; code, only the initial set of CPUID information is saved.
;------------------------------------------------
;    push    ebx                             ; save registers 
;    push    esi 
;    push    edi 
;    push    edx
;    push    ecx

;    mov     ebp,X86                         ; rv

    mov     eax, 0                          ; set up for CPUID instruction 
    CPU_ID                                  ; get and save vendor ID

    mov     DWORD PTR _vendor_id, ebx 
    mov     DWORD PTR _vendor_id[+4], edx 
    mov     DWORD PTR _vendor_id[+8], ecx

    cmp     DWORD PTR intel_id, ebx 
    jne     IsProc_AMD
    cmp     DWORD PTR intel_id[+4], edx 
    jne     end_cpuid_type 
    cmp     DWORD PTR intel_id[+8], ecx 
    jne     end_cpuid_type                  ; if not equal, not an Intel processor

    cmp     eax, 1                          ; make sure 1 is valid input for CPUID 
    jl      end_cpuid_type                  ; if not, jump to end 

    mov     eax, 1 
    CPU_ID                                  ; get family/model/stepping/features 

    mov     ebp,XMM                         ; assume PIII

    bt      edx,SIMD                        ; check for SIMD support
    jnae    end_cpuid_type

SIMDContinue:
    shr     eax, 4                          ; isolate family and model
    mov     ebp,PII                         ; assume PII

    and     eax,0ffh                        ;mask out type and reserved
    nop

    cmp     eax,PENTII
    jge     end_cpuid_type

    mov     ebp,PPRO
    
    cmp     eax,PENTPRO
    je      end_cpuid_type

    mov     ebp,PMMX
    
    cmp     eax,PENTMMX
    je      end_cpuid_type

    mov     ebp,X86
    
    cmp     eax,PENT
    jge     end_cpuid_type

;    mov     ebp,X86

end_cpuid_type: 
    mov     eax,ebp

;remove these pops ???

;    pop     edi                             ; restore registers 
;    pop     esi 
;    pop     ebx 
;    pop     edx
;    pop     ecx
   
end_cpu_type:
    pop     edx ;safety sh*&
    pop     ecx
    pop     ebx
    pop     ebp
    pop     edi
    pop     esi
    ret

end_cpu_type486:
    mov     eax,ebp
    pop     edx ;safety sh*&
    pop     ecx
    pop     ebx
    pop     ebp
    pop     edi
    pop     esi
    ret

;------------------------------------------------
IsProc_AMD:
    cmp     DWORD PTR amd_id, ebx 
    jne     IsProc_CYRIX

    cmp     DWORD PTR amd_id[+4], edx 
    jne     end_cpuid_type 

    cmp     DWORD PTR amd_id[+8], ecx 
    jne     end_cpuid_type                  ; if not equal, not an AMD processor

    cmp     eax, 1                          ; make sure 1 is valid input for CPUID 
    jl      end_cpuid_type                  ; if not, jump to end 

    mov     eax, 1 
    CPU_ID                                  ; get family/model/stepping/features 

    shr     eax, 4                          ; isolate family and model
    mov     ebp,AMDK63D    

    and     eax,0ffh                        ;mask out type and reserved
    nop

    cmp     eax,AMD_K63D
    jge     end_cpuid_type

    mov     ebp,AMDK6    
    nop

    cmp     eax,AMD_K6
    jge     end_cpuid_type

    mov     ebp,X86
    nop

    cmp     eax,AMD_K5
    jge     end_cpuid_type

    mov     ebp,X86
    jmp     end_cpuid_type

;------------------------------------------------
IsProc_CYRIX:
    cmp     DWORD PTR cyrix_id, ebx 
    jne     end_cpuid_type

    cmp     DWORD PTR cyrix_id[+4], edx 
    jne     end_cpuid_type 

    cmp     DWORD PTR cyrix_id[+8], ecx 
    jne     end_cpuid_type                  ; if not equal, not an CYRIX processor

    cmp     eax, 1                          ; make sure 1 is valid input for CPUID 
    jl      end_cpuid_type                  ; if not, jump to end 

    mov     eax, 1 
    CPU_ID                                  ; get family/model/stepping/features 

    shr     eax, 4                          ; isolate family and model
    mov     ebp,C6X86MX

    and     eax,0ffh                        ;mask out type and reserved
    nop

    cmp     eax,_6X86MX
    je      end_cpuid_type

    mov     ebp,X86
    jmp     end_cpuid_type
;************************************************
         END
