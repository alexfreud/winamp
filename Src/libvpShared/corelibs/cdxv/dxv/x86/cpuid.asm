;/***********************************************\
;??? cpuid.asm   
; checks for cpuid
; if an id is not found, the program assumes a x86
;\***********************************************/ 

; parts taken from intel's AP-485 



;put checks for cmov and mmx support ????



        .486
        .MODEL  flat, SYSCALL, os_dos
        .CODE

IDEAL
NAME x86cpuid
MASM

PUBLIC getCPUID_
PUBLIC _getCPUID

INCLUDE proc.ash

EXTRN c cpuFeatures:DWORD


_486        EQU 4h
PENT        EQU 50h
PENTMMX     EQU 54h
PENTPRO     EQU 61h
PENTII      EQU 63h

AMD_K63D    EQU 58h
AMD_K6      EQU 56h
AMD_K5      EQU 50h             ; K5 has models 0 - 6

_6X86       EQU 52h
_6X86MX     EQU 60h

.DATA 

_vendor_id      db "------------" 
intel_id        db "GenuineIntel" 
amd_id          db "AuthenticAMD" 
cyrix_id        db "CyrixInstead" 

getCPUID_:
_getCPUID:
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
.486 
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
    
;    cmp     eax,PENT
;    jge     end_cpuid_type

end_cpuid_type: 
    mov     eax,ebp
    mov     [cpuFeatures],edx

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
