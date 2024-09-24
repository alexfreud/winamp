
; Pentium utilities.  Timothy S. Murphy 1/11/97.
; This is a Borland i586 TASM source file.
; Works (at least) with Watcom C++ and Visual C++ using "cdecl" linkage.

	.586
	.MODEL	flat, c, os_dos
	.CODE

;------------------------------------------------
PUBLIC c pentiumKiloCycles, pentiumTime

pentiumKiloCycles:
		push	edx
		; rdtsc					; get 64-bit cycle count in edx:eax
		db		0Fh, 31h		; (tasm 4.0 doesn't have rdtsc opcode)
		shrd	eax, edx, 10	; divide by 1024
		pop		edx
		ret						; value in eax

pentiumTime:
		push	ebx
			push	edx

		; rdtsc					; get 64-bit cycle count in edx:eax
		db		0Fh, 31h		; (tasm 4.0 doesn't have rdtsc opcode)
		shrd	eax, edx, 10	; divide by 1024
		mov		ebx, eax
	
		mov		eax, 12[esp]
		shr		eax, 1

lup:	shr		edx, 16
			dec		eax
		nop
			jns		lup

		; rdtsc					; get 64-bit cycle count in edx:eax
		db		0Fh, 31h		; (tasm 4.0 doesn't have rdtsc opcode)
		shrd	eax, edx, 10	; divide by 1024

		sub		eax, ebx

		pop		edx
		pop		ebx
		ret						; value in eax
;------------------------------------------------
; void Get_scc(&preciseU32,&lessPreciseU32);
x86_Get_sccParams    STRUC 
                        dd  3 dup (?)   ;3 pushed regs
                        dd  ?           ;return address
        preciseU32      dd  ? 
        lessPreciseU32  dd  ?
x86_Get_sccParams    ENDS

PUBLIC c Get_scc

Get_scc:
    push	edx
    push    esi
    push    edi

    mov     esi,[esp].preciseU32
    mov     edi,[esp].lessPreciseU32

    ; rdtsc					; get 64-bit cycle count in edx:eax
    db		0Fh, 31h		; (tasm 4.0 doesn't have rdtsc opcode)

    mov     [edi],edx
    mov     [esi],eax

    pop     edi
    pop     esi
    pop		edx
    ret						; value in eax

END