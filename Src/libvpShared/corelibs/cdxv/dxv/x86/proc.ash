;/***********************************************\
;??? proc.ash   
;\***********************************************/ 


CPU_ID MACRO 
    db 0fh                      ; Hardcoded CPUID instruction 
    db 0a2h 
ENDM

;see proc.h
X86         EQU 0                   ; /* 486, Pentium plain, or any other x86 compatible */
PMMX        EQU 1                   ; /* Pentium with MMX */
PPRO        EQU 2                   ; /* Pentium Pro */
PII         EQU 3                   ; /* Pentium II */
C6X86       EQU 4
C6X86MX     EQU 5
AMDK63D     EQU 6
AMDK6       EQU 7
AMDK5       EQU 8


