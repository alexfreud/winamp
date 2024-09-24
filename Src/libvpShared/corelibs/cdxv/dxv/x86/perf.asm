;/***********************************************\
;??? perf.asm   
;\***********************************************/ 
        .586
        .MODEL  flat, SYSCALL, os_dos
        .CODE

IDEAL
NAME tsc
MASM

PUBLIC DUCK_sti_   
PUBLIC _DUCK_sti

PUBLIC DUCK_cli_   
PUBLIC _DUCK_cli

PUBLIC rdtsc_Start_   
PUBLIC _rdtsc_Start

PUBLIC rdtsc_End_   
PUBLIC _rdtsc_End

PUBLIC addTSC_   
PUBLIC _addTSC

; typedef struct tsc_cnt {
;     unsigned long low;
;     unsigned long high;
; } *TSC_HANDLE, TSC;

DUCK_sti_:   
_DUCK_sti:
    sti
    ret

DUCK_cli_:   
_DUCK_cli:
    cli
    ret

;------------------------------------------------
; void rdtsc_Start(low, high)
;
rdtsc_StartParams    STRUC
                dd  3 dup (?)   ;3 pushed regs
                dd  ?           ;return address
    low         dd  ?       
    high        dd  ?       
rdtsc_StartParams    ENDS
;------------------------------------------------
rdtsc_Start_:
_rdtsc_Start:
    push    ebx 
    push    ecx
    push    edx
nop

    mov     ebx,[esp].low               ;pointer to low
    mov     ecx,[esp].high             ;pointer to high
        
;    RDTSC
    db 0fh, 31h

    mov     [ebx],eax               ;return values
    mov     [ecx],edx

nop
    pop     edx
    pop     ecx
    pop     ebx
    ret

;------------------------------------------------
; void rdtsc_End(unsigned long *)
;
rdtsc_EndParams    STRUC
                dd  6 dup (?)   ;6 pushed regs
                dd  ?           ;return address
    elow         dd  ?       
    ehigh        dd  ?       
rdtsc_EndParams    ENDS
;------------------------------------------------
rdtsc_End_:
_rdtsc_End:
    push    esi
    push    edi
    push    ebp
    push    ebx 
    push    ecx
    push    edx

    mov		edi,[esp].elow               ;pointer to low var
    mov		esi,[esp].ehigh               ;pointer to high var

;    RDTSC
    db 0fh, 31h

    mov         ebx,[edi]           ;get start values
    mov         ecx,[esi]
    sub         eax,ebx
    sbb         edx,ecx

    mov         [edi],eax           ;return values
    mov         [esi],edx

    pop     edx
    pop     ecx
    pop     ebx
    pop     ebp
    pop     edi
    pop     esi
    ret

;------------------------------------------------
; adds time stamped counts and passes back average
;------------------------------------------------
; void addTSC(unsigned long *, unsigned long, unsigned long *);
;
addTSCParams    STRUC
                dd  6 dup (?)   ;6 pushed regs
                dd  ?           ;return address
    dkTimes     dd  ?
    dkCount     dd  ?       
    rv          dd  ?       
addTSCParams    ENDS

addTSC_:
_addTSC:
    push    esi
    push    edi
    push    ebp
    push    ebx 
    push    ecx
    push    edx

    xor     ebp,ebp             ;used for adc
    mov     eax,[esp].dkTimes   ;pointer to array of TSC's

    mov     edi,[esp].dkCount   ;array count
    mov     esi,[esp].rv        ;pointer to result

    xor     edx,edx
    mov     ebx,[eax]           ;get first TSC

    mov     ecx,[eax+4]         ;get next TSC
    add     eax,8
    
    adc     edx,ebp
    add     ebx,ecx

add_loop:
    dec     edi
    jz      averageVal

    mov     ecx,[eax]
    add     eax,4

    adc     edx,ebp
    add     ebx,ecx

    jmp     add_loop

averageVal:
    mov     eax,ebx
    mov     ebx,[esp].dkCount   ;array count

    div     ebx                 ;div edx:eax by ebx (eax=quo, edx=rem)

    mov    [esi],eax            ;get average of counts

the_exit:
    pop     edx
    pop     ecx
    pop     ebx
    pop     ebp
    pop     edi
    pop     esi
    ret

;************************************************
         END

